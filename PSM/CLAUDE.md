# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

PSM (Power Supply Module) — embedded firmware for the **STM32G473xx** (Cortex-M4 with FPU). The firmware monitors AC grid voltage/frequency and DC regulator voltages, and communicates over CAN bus. It is part of the CPU4v1 board family alongside SSM (Signal Switching Module).

## Build Commands

### STM32 firmware (requires `arm-none-eabi-gcc` on PATH)

```bash
cmake --preset STM32-Debug   && cmake --build --preset STM32-Debug
cmake --preset STM32-Release && cmake --build --preset STM32-Release
```

Output: `build/STM32-{Debug,Release}/PSM.elf`. Linker script: `STM32G473XX_FLASH.ld`.

### Host unit tests (x86-64, no hardware required)

```bash
cmake --preset Host-Test && cmake --build --preset Host-Test
ctest --preset Host-Test --output-on-failure
```

### Coverage (after running tests)

```bash
gcovr --root . build/Host-Test    # uses gcovr.cfg (filter = App/Domain/, fail-under-line = 80)
```

### Docker (self-contained, no local toolchain needed)

```bash
cd Tools/Docker
docker compose run --rm build-arm    # STM32 Release build
docker compose run --rm build-test   # host tests + coverage
docker compose run --rm shell-arm    # interactive ARM shell
docker compose run --rm shell-test   # interactive test shell
```

### Code formatting (requires `uncrustify`)

```bash
cmake --build build/STM32-Debug --target Format        # reformat in-place
cmake --build build/STM32-Debug --target Format-Check  # CI-safe check
```

User application sources are added in the **top-level** `CMakeLists.txt` (not in `cmake/stm32cubemx/CMakeLists.txt`, which is CubeMX-managed and should not be edited by hand).

The `BUILD_TARGET` CMake variable controls the build mode: `STM32` (default, cross-compile) or `Host` (x86-64 unit testing).

## Architecture

### Layer separation

PSM follows the same hexagonal (ports + adapters) architecture as CPM/CP.

- `App/Domain/` — Pure C11 computation; **no HAL, no FreeRTOS**. Only layer covered by unit tests. Add new testable logic here.
- `App/Ports/` — Six vtable interface headers (zero implementation). Domain code only calls via these.
- `App/Adapters/STM32/` — STM32 adapter implementations; included only in the `STM32` build target.
- `App/Adapters/Mock/` — In-memory mock adapters for host unit tests; included only in the `Host` build target.
- `App/Platform/STM32/Tasks/` — FreeRTOS task wrappers (STM32-only); bridge CubeMX ISRs to domain services. Compiled into `PSM_Platform_STM32` for the STM32 build; omitted from Host builds.
- `App/Platform/Host/` — Host test runner entry point (`main_host.c`); linked only for `BUILD_TARGET=Host`.
- `Core/` — STM32CubeMX-generated peripheral init code (HAL + FreeRTOS scaffolding). User logic lives inside `/* USER CODE BEGIN/END */` guards so CubeMX regeneration is safe.
- `Tests/` — Unity unit tests; compiled only for `BUILD_TARGET=Host`. Each test file has its own `main()` with explicit `RUN_TEST()` calls.
- `Tools/` — Docker (`Tools/Docker/`), MISRA-C format config (`Tools/Format/`), and helper scripts (`Tools/Scripts/`).
- `Drivers/` / `Middlewares/` — Vendor-supplied STM32 HAL and FreeRTOS; do not modify.

### Port interfaces (`App/Ports/`)

Six vtable structs, each a `void *ctx` + function pointer(s) with an inline dispatch helper:

| Header | Inline helper(s) | Abstracts |
|--------|-----------------|-----------|
| `IIndicatorLEDPort.h` | `IndicatorLED_SetState`, `IndicatorLED_Toggle` | GPIO LED outputs |
| `ISignalInputPort.h` | `SignalInput_GetState` | GPIO digital inputs (ACOK, DCOK, COMMOK pins) |
| `IEepromPort.h` | `Eeprom_Read`, `Eeprom_Write` | I2C EEPROM via StorageTask queue |
| `ICANTxPort.h` | `CANTx_Send` | FDCAN1 transmit queue |
| `IVoltageSensorPort.h` | `VoltageSensor_GetNetVoltage/GetRegVIn/GetRegVOut` | ADC DMA voltage results |
| `IFrequencySensorPort.h` | `FrequencySensor_GetNetFrequency` | TIM2 frequency measurement |

### Adapters

**STM32 adapters** (`App/Adapters/STM32/`) — wrap HAL/storage calls. `VoltageSensorAdapterCtx_t` and `FrequencySensorAdapterCtx_t` have writable fields populated by ISR callback forwarders in `App/Platform/STM32/Tasks/Measurement.c` (`MeasurementNetVoltageSet`, etc.). Float reads/writes are 32-bit word-aligned on Cortex-M4 and therefore atomic — no mutex needed.

**Mock adapters** (`App/Adapters/Mock/`) — in-memory implementations. Tests pre-set input fields and inspect output fields after calling service methods. `MockEepromAdapterCtx_t` has a 256-byte backing buffer plus configurable `bReadResult`/`bWriteResult` flags.

### FreeRTOS tasks (`Core/Src/app_freertos.c`)

| Task | Priority | Role |
|---|---|---|
| `MaintenanceTask` | Low | Watchdog keeper; verifies all tasks signal `MaintenanceEvent` within 500 ms, resets on 2 consecutive failures. IWDG is only initialized here in Release builds (`#ifndef DEBUG`). |
| `CANMsgParserTask` | Normal | Dequeues received CAN frames from `CANRxReqsQueue`, dispatches to `CANMsgParse()`. Also calls `CANStart()` on init. |
| `CANMsgSenderTask` | Normal | Serialises all outgoing CAN transmissions from `CANTxReqsQueue` to avoid concurrent TX FIFO access. |
| `MeasurementTask` | AboveNormal | Waits on `THREAD_FLAGS_MEASUREMENT_DONE` (set by ADC DMA complete callback), computes voltages/frequency, drives LEDs, sends periodic measurement frames and flash-sync frames. |
| `StorageTask` | Normal | Synchronous EEPROM read/write over I2C; signals calling thread via `THREAD_FLAGS_STORAGE_REQ_PROCESS_OK/ERROR`. |

### Inter-task communication

All queues pass **pointers** to objects allocated from static memory pools, keeping queue items small (pointer-sized).

| Queue | Memory pool | Producer | Consumer |
|---|---|---|---|
| `CANRxReqsQueue` | `CANRxReqsMemPool` | `HAL_FDCAN_RxFifo0Callback` (ISR) | `CANMsgParserTask` |
| `CANTxReqsQueue` | `CANTxReqsMemPool` | `CANTxRequest()` (any task) | `CANMsgSenderTask` |
| `StorageReqsQueue` | `StorageReqsMemPool` | `StorageRequest()` (any task) | `StorageTask` |

Global RTOS handles and the `MaintenanceTaskSignal()` helper are declared in `Core/Inc/utilities.h` and defined in `Core/Src/app_freertos.c`.

### CAN bus

- **FDCAN1** (PA11/PA12): Classic CAN, standard IDs only. Accepts IDs in range `[FDCAN_CP_FLASH_SIGNALS_1_STD_ID … FDCAN_CP_OFFSET_2_STD_ID]`; all others rejected.
- **FDCAN2** (PB12/PB13): FDCAN with BRS, standard + extended IDs, accepts all frames (reserved for future use).

Incoming message IDs parsed in `App/Platform/STM32/Tasks/CanMsgParser.c`:
- `FDCAN_CP_DATE_TIME_STD_ID` — resets flash-state and comm-error counter.
- `FDCAN_CP_FLASH_SIGNALS_1_STD_ID` — sets measurement/flash period from byte 6.
- `FDCAN_CP_OFFSET_1/2_STD_ID` — sets AC voltage calibration offset (operation + value).

Outgoing:
- `FDCAN_PSM_MEASUREMENT_STD_ID` — periodic voltage/frequency measurement frame.
- `FDCAN_PSM_FLASH_SYNC_1_STD_ID` — flash-sync signal broadcast.

### Measurement pipeline

1. TIM3 triggers ADC1 DMA → grid (AC) voltage samples.
2. TIM2 input-capture/output-compare → grid frequency.
3. ADC2/ADC3 DMA → regulator Vin and Vout.
4. ADC DMA complete → `MeasurementThreadFlagSet()` → unblocks `MeasurementTask`.
5. Task applies calibration coefficient + optional offset, converts to scaled integers, evaluates OK ranges, drives LEDs, sends CAN frame.

Scaling constants (defined in `App/Platform/STM32/Tasks/Measurement.c`):
- AC voltage: `CP_NET_VOLTAGE_COEFFICIENT = 0.73029`
- DC voltages: `CP_REG_VIN_COEFFICIENT = CP_REG_VOUT_COEFFICIENT = 10.0`

OK ranges: Vin 22–26 V, Vout 4.7–6.7 V, AC 165–265 V.

### EEPROM storage

Accessed via I2C through `StorageRequest(bReqId, address, data, size)`. Persisted values:
- Offset calibration struct at `I2C_E2PROM_ADD_OFFSET`
- Flash period at `I2C_E2PROM_ADD_PERIOD`

`STORAGE_REQ_EEPROM_WRITE_ASYNCH` returns immediately; `STORAGE_REQ_EEPROM_WRITE` and `STORAGE_REQ_EEPROM_READ` block until `StorageTask` signals completion.

## Naming Conventions

Follow the system-wide rules in the root `CLAUDE.md` § "Naming Conventions (system-wide)":
`PascalCase_t` types, `PascalCase_e` enums, `camelCase` variables/fields/params, `SCREAMING_SNAKE_CASE` macros, `g_` / `s_` for scope, **no Hungarian prefixes** (`b` / `s` / `l` / `f` / `p` / `tS` / `tE` / `tp` / `S` struct-instance prefix).

**Migration status**: PSM is **fully migrated** — the Hungarian-prefix
scheme inherited from the prior codebase has been retired. Do not
reintroduce any of those prefixes in PSM user code. CubeMX-generated
locals inside `Core/` HAL init blocks (e.g. `sConfig`, `sMasterConfig`) are
regenerated on each `.ioc` export and are intentionally left alone.

## GitHub Actions Workflows

Three workflows in `.github/workflows/`:

| Workflow | Trigger | What it does |
|---|---|---|
| `psm-build.yml` | push/PR to `main`, workflow_call | Builds STM32-Debug + STM32-Release; uploads `.elf` + `.map` (30 days) |
| `psm-test.yml` | any push/PR, workflow_call | Builds Host-Test, runs ctest, generates gcovr HTML/XML report (7 days) |
| `psm-release.yml` | tag `psm-v*.*.*` | Calls build + test, generates `.bin`, creates GitHub Release |

## Notes

- `Core/Src/can_util.c` is **not** included in the CMake build. It references old function names from a prior codebase version and can be ignored or removed.
- `MAINTENANCE_MAX_TASK_FAILURES = 2` and `MAINTENANCE_TASK_TIMEOUT_MS = 500 ms` — worst-case fault detection latency = 1000 ms (NEMA TS2 compliant). Adjust if tasks legitimately need more time to start.
- When adding new testable computation to `Tasks/`, extract it into `App/Domain/` first and add Unity tests in `Tests/Unit/`. Each test file must include its own `main()` with `UNITY_BEGIN()`, `RUN_TEST()` calls, and `UNITY_END()`.

## Vendor MIB Maintenance (1.3.6.1.4.1.59748)

PSM does not serve SNMP itself, but PSM-originated signals (AC line loss, rail voltage faults, PSM-missing heartbeats) feed the CP SNMP agent via the MP and surface under the Teknotel enterprise arc `1.3.6.1.4.1.59748`. The canonical `.mib` (vendor "Teknotel Elektronik") lives at `../CPM/CP/Docs/TEKNOTEL-CPU4-MIB.mib`. OID arc shape: `teknotel .59748 → transportation .4 → devices .2 → cpu4 .1 → { unit .3, channel .8, cpMpLink .20, driverModule .21 }`.

**Update that `.mib` in the same commit** whenever PSM work changes:

- The semantics of any `CpMpFaultGlobalFlags_t` bit driven from PSM telemetry (e.g. `AC_LINE`, `RAIL_24V`, `RAIL_5V`, `PSM_MISSING`) — these are mirrored by `cpMpLinkGlobalFlags` at `.59748.4.2.1.20.8`
- The trigger condition for `teknotelPowerDownTrap` (specific-trap 1, emitted at `.59748.0.1`) in `CPM/CP/LWIP/App/snmp_client.c`

Do not invent new enterprise OIDs from PSM — new objects must be added on the CP side via `CPM/CP/App/Domain/NTCIP/MibVendor59748/` (one file per functional group) and simultaneously in the `.mib`.
