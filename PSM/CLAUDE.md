# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

PSM (Power Supply Module) — embedded firmware for the **STM32G473xx** (Cortex-M4 with FPU). The firmware monitors AC grid voltage/frequency and DC regulator voltages, and communicates over CAN bus. It is part of the CPU4v1 board family alongside SSM (Signal Switching Module).

## Build Commands

### STM32 firmware (requires `arm-none-eabi-gcc` on PATH)

```bash
cmake --preset STM32-GCC-Debug   && cmake --build --preset STM32-GCC-Debug
cmake --preset STM32-GCC-Release && cmake --build --preset STM32-GCC-Release
```

Output: `build/STM32-GCC-{Debug,Release}/PSM.elf`. Linker script: `STM32G473XX_FLASH.ld`.

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
cmake --build build/STM32-GCC-Debug --target Format        # reformat in-place
cmake --build build/STM32-GCC-Debug --target Format-Check  # CI-safe check
```

User application sources are added in the **top-level** `CMakeLists.txt` (not in `cmake/stm32cubemx/CMakeLists.txt`, which is CubeMX-managed and should not be edited by hand).

The `BUILD_TARGET` CMake variable controls the build mode: `STM32` (default, cross-compile) or `Host` (x86-64 unit testing).

## Architecture

### Layer separation

- `App/Domain/` — Pure C11 computation; **no HAL, no FreeRTOS**. Only layer covered by unit tests. Add new testable logic here.
- `Tasks/` — FreeRTOS task implementations; calls Domain functions for computation, uses HAL/RTOS for I/O.
- `Core/` — STM32CubeMX-generated peripheral init code (HAL + FreeRTOS scaffolding). User logic lives inside `/* USER CODE BEGIN/END */` guards so CubeMX regeneration is safe.
- `Tests/` — Unity unit tests; compiled only for `BUILD_TARGET=Host`. Each test file has its own `main()` with explicit `RUN_TEST()` calls.
- `Tools/` — Docker (`Tools/Docker/`), MISRA-C format config (`Tools/Format/`), and helper scripts (`Tools/Scripts/`).
- `Drivers/` / `Middlewares/` — Vendor-supplied STM32 HAL and FreeRTOS; do not modify.

### FreeRTOS tasks (`Core/Src/app_freertos.c`)

| Task | Priority | Role |
|---|---|---|
| `MaitenanceTask` | Low | Watchdog keeper; verifies all tasks signal `MaintenanceEvent` within 3 s, resets on 3 consecutive failures. IWDG is only initialized here in Release builds (`#ifndef DEBUG`). |
| `CANMsgParserTask` | Normal | Dequeues received CAN frames from `CANRxReqsQueue`, dispatches to `CANMsgParse()`. Also calls `CANStart()` on init. |
| `CANMsgSenderTask` | Normal | Serialises all outgoing CAN transmissions from `CANTxReqsQueue` to avoid concurrent TX FIFO access. |
| `MeasurementTask` | Normal | Waits on `THREAD_FLAGS_MEASUREMENT_DONE` (set by ADC DMA complete callback), computes voltages/frequency, drives LEDs, sends periodic measurement frames and flash-sync frames. |
| `StorageTask` | Normal | Synchronous EEPROM read/write over I2C; signals calling thread via `THREAD_FLAGS_STORAGE_REQ_PROCESS_OK/ERROR`. |

### Inter-task communication

All queues pass **pointers** to objects allocated from static memory pools, keeping queue items small (pointer-sized).

| Queue | Memory pool | Producer | Consumer |
|---|---|---|---|
| `CANRxReqsQueue` | `CANRxReqsMemPool` | `HAL_FDCAN_RxFifo0Callback` (ISR) | `CANMsgParserTask` |
| `CANTxReqsQueue` | `CANTxReqsMemPool` | `CANTxRequest()` (any task) | `CANMsgSenderTask` |
| `StorageReqsQueue` | `StorageReqsMemPool` | `StorageRequest()` (any task) | `StorageTask` |

`VoltagesMutex` protects `SCANMeasurements` (the struct copied into outgoing CAN frames) between `MeasurementTask` and any future reader.

Global RTOS handles and the `MaintenanceTaskSignal()` helper are declared in `Core/Inc/utilities.h` and defined in `Core/Src/app_freertos.c`.

### CAN bus

- **FDCAN1** (PA11/PA12): Classic CAN, standard IDs only. Accepts IDs in range `[FDCAN_CP_FLASH_SIGNALS_1_STD_ID … FDCAN_CP_OFFSET_2_STD_ID]`; all others rejected.
- **FDCAN2** (PB12/PB13): FDCAN with BRS, standard + extended IDs, accepts all frames (reserved for future use).

Incoming message IDs parsed in `Tasks/Src/can_msg_parser.c`:
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

Scaling constants (defined in `Tasks/Src/measurement.c`):
- AC voltage: `CP_NET_VOLTAGE_COEFFICIENT = 0.73029`
- DC voltages: `CP_REG_VIN_COEFFICIENT = CP_REG_VOUT_COEFFICIENT = 10.0`

OK ranges: Vin 22–26 V, Vout 4.7–6.7 V, AC 165–265 V.

### EEPROM storage

Accessed via I2C through `StorageRequest(bReqId, address, data, size)`. Persisted values:
- Offset calibration struct at `I2C_E2PROM_ADD_OFFSET`
- Flash period at `I2C_E2PROM_ADD_PERIOD`

`STORAGE_REQ_EEPROM_WRITE_ASYNCH` returns immediately; `STORAGE_REQ_EEPROM_WRITE` and `STORAGE_REQ_EEPROM_READ` block until `StorageTask` signals completion.

## Naming Conventions

Hungarian-style prefixes used throughout:

| Prefix | Type |
|---|---|
| `b` | `uint8_t` / byte / bool flag |
| `s` | `uint16_t` / short |
| `l` | `uint32_t` / long |
| `f` | `float` or boolean flag (context-dependent) |
| `p` | pointer |
| `tS` | struct typedef |
| `tE` | enum typedef |
| `S` (uppercase) | struct instance |

## GitHub Actions Workflows

Three workflows in `.github/workflows/`:

| Workflow | Trigger | What it does |
|---|---|---|
| `psm-build.yml` | push/PR to `main`, workflow_call | Builds STM32-GCC-Debug + STM32-GCC-Release; uploads `.elf` + `.map` (30 days) |
| `psm-test.yml` | any push/PR, workflow_call | Builds Host-Test, runs ctest, generates gcovr HTML/XML report (7 days) |
| `psm-release.yml` | tag `psm-v*.*.*` | Calls build + test, generates `.bin`, creates GitHub Release |

## Notes

- `Core/Src/can_util.c` is **not** included in the CMake build. It references old function names from a prior codebase version and can be ignored or removed.
- `MAINTAINANCE_MAX_TASK_FAILURES = 3` and `MAINTAINANCE_TASK_MAX_TIMEOUT = 3000 ms` — adjust if tasks legitimately need more time to start.
- When adding new testable computation to `Tasks/`, extract it into `App/Domain/` first and add Unity tests in `Tests/Unit/`. Each test file must include its own `main()` with `UNITY_BEGIN()`, `RUN_TEST()` calls, and `UNITY_END()`.
