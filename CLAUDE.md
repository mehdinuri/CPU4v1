# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

CPU4v1 is an intersection controller system being refactored from EU stage-based logic to **NEMA TS2 dual-ring barrier logic** with full **NTCIP 1201/1202 compliance**. Each module has its own `CLAUDE.md` with module-specific details; this file covers the system-wide architecture.

## Hardware Modules

| Module | MCU | Role |
|--------|-----|------|
| **CPM/CP** (Controller Processor) | STM32H743VIT6 (Cortex-M7 @ 480 MHz) | NEMA TS2 intersection engine, NTCIP SNMP, coordinator |
| **CPM/MP** (Malfunction Processor) | STM32G473 (Cortex-M4 @ 160 MHz) | Signal card safety, malfunction detection, conflict watchdog |
| **PSM** (Power Supply Module) | STM32G473 | AC/DC voltage/frequency monitoring, calibration storage |
| **SSM** (Signal Switching Module) | STM32G473 | Red/Yellow/Green channel switching, per-output measurement |

Modules communicate over **CAN/FDCAN** using a snapshot protocol. CP is the bus master; MP, PSM, and SSM are peripherals that publish telemetry and receive commands via CAN frames.

## Build Commands

All modules follow the same CMake pattern. Presets are used in CP; direct CMake invocation in MP/PSM/SSM.

### CP (STM32H743 firmware)
```bash
cd CPM/CP
cmake --preset STM32-Debug   && cmake --build --preset STM32-Debug
cmake --preset STM32-Release && cmake --build --preset STM32-Release
```

### CP (host unit tests + coverage)
```bash
cd CPM/CP
cmake --preset Host-Test && cmake --build --preset Host-Test
cd build/Host-Test && ctest --output-on-failure
./Tools/Scripts/build-test.sh   # generates build/coverage-report/index.html
```

### Run a single test
```bash
cd CPM/CP/build/Host-Test
./Tests/Unit/Test_Phase          # run one test binary directly
ctest -R Test_Phase              # or via ctest filter
```

### Code formatting (must pass CI)
```bash
cd CPM/CP
cmake --build build/Host-Test --target Format-Check   # check only
cmake --build build/Host-Test --target Format         # apply in-place
```

### MP / PSM / SSM firmware
```bash
cd CPM/MP   # or PSM, SSM
cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/gcc-arm-none-eabi.cmake
cmake --build build
```

### Docker builds (mirrors CI)
```bash
cd CPM/CP
docker compose -f Tools/Docker/compose.yml run build-arm    # STM32 firmware
docker compose -f Tools/Docker/compose.yml run build-test   # host tests + coverage
```

### Flash to hardware
```bash
cd CPM/CP
./Tools/Scripts/flash.sh STM32-Release   # OpenOCD or STM32_Programmer_CLI
```

## Architecture

### Hexagonal Architecture (CP — the reference pattern)

CP follows strict ports-and-adapters. MP is adopting the same pattern; PSM/SSM are simpler task-oriented firmware not yet fully hexagonalised.

```
┌────────────────────────────────────────────────────┐
│           App/Domain/  (pure C11, zero deps)        │
│   Intersection engine · NTCIP 1201/1202 objects     │
│   Services · LCD logic                              │
└──────────────┬─────────────────────────────────────┘
               │ calls via inline vtable dispatch
    ┌──────────▼──────────┐
    │   App/Ports/         │   ← 26 C vtable interface headers, zero impl
    └──────────┬──────────┘
               │ implemented by
   ┌───────────┴───────────┐
   │  App/Adapters/STM32/  │   App/Platform/STM32/Tasks/
   │  (HAL, LwIP, FreeRTOS │   (FreeRTOS task wrappers,
   │   bindings)           │    adapter wiring in main_stm32.c)
   └───────────────────────┘
```

**Port pattern** — all 26 ports follow this exact structure:
```c
typedef struct {
    void *ctx;
    void (*Method)(void *ctx, /* args */);
} IFeaturePort_t;

static inline void Feature_Method(IFeaturePort_t *p, /* args */) {
    if (p != NULL) p->Method(p->ctx, /* args */);
}
```

Domain code never calls HAL or any platform API directly.

### NEMA TS2 Dual-Ring Engine (`CPM/CP/App/Domain/Intersection/`)

Core objects and their roles:

| Type | Role |
|------|------|
| `IntersectionEngine_t` | Master state machine, owns all rings/phases/detectors |
| `Phase_t` | Timing parameters: minGreen, maxGreen, passage, yellow, redClear, walk, pedClear |
| `Detector_t` | Demand/extension/occupancy tracking at 100 ms tick |
| `SignalGroup_t` | Maps output channels to phase green/yellow/red |
| `Sequence_t` | Barrier sync: both rings must reach their barrier before crossing |
| `Conflict_t` | Safety matrix; any violation forces `STATE_ALL_RED` |
| `Program_t` | Cycle, offset, splits, coordination |

**Ring-barrier rule:** Ring 1 (phases 1–4) and Ring 2 (phases 5–8) must both reach their barrier ends (after phases 2/4 and 6/8) before either can advance. This guarantees opposing movements cannot conflict across the barrier.

`ProgramTask` calls `ProgramTick()` at **100 ms** — the fundamental timing unit for the entire intersection engine.

### NTCIP Layer (`CPM/CP/App/Domain/NTCIP/`)

Three files form the compliance layer:
- **`NTCIP1201.c`** — global system objects (time, data collection, coordination)
- **`NTCIP1202.c`** — actuated controller objects (phase timing, detector status, unit control, alarms)
- **`OidRegistry.c`** — transport-agnostic OID dispatch; maps `OidObjectId_t` enum to getter/setter; domain has no knowledge of SNMP wire format

The LwIP SNMP adapter (`App/Adapters/STM32/LWIPSNMPAdapter.c`) bridges the SNMP wire protocol to `OidRegistry`, keeping all MIB logic inside the domain.

### FreeRTOS Tasks (CP)

9 tasks; `ProgramTask` is the timing heart:

| Task | Period | Role |
|------|--------|------|
| `ProgramTask` | 100 ms | `ProgramTick()` — intersection engine step |
| `CANRxTask` | event | FDCAN → detector/signal adapters |
| `CANTxTask` | event | Drain CAN Tx queue |
| `NetworkTask` | event | LwIP, DHCP, TCP |
| `GPSTask` | event | UART5 NMEA → GPS adapter |
| `UITask` | event | LCD + keypad |
| `StorageTask` | event | Async EEPROM/Flash writes |
| `TimeTask` | 1 s | RTC + GPS sync |
| `MaintenanceTask` | 1 s | Watchdog feed |

### STM32CubeMX Integration

Each module has a `.ioc` file (`CP.ioc`, `MP.ioc`, `PSM.ioc`, `SSM.ioc`). Re-generating from CubeMX overwrites everything outside `/* USER CODE BEGIN/END */` guards. The generated HAL/middleware lives in `Core/`, `LWIP/`, `MBEDTLS/`, `USB_DEVICE/`, `Drivers/`, `Middlewares/` — treat those directories as third-party and do not edit them.

## Coding Standards

**Language:** C11 throughout — no C++.

**Hard rules for `App/Domain/` (CP reference, extending to all modules):**
- No `malloc` — static pools only
- No global mutable state — all state lives in context structs passed by pointer
- Domain functions never touch HAL, FreeRTOS, or network APIs

**MISRA-C rules enforced by Uncrustify and `-Werror`:**
- All control flow bodies use `{ }` braces (Rule 15.6)
- Every `case` ends with `break` or `/* fallthrough */` (Rule 16.3)
- Every `switch` has a `default` clause last (Rules 16.4/16.5)
- Array indexing only — no pointer arithmetic (`a[i]`, not `p++`) (Rule 17.4)
- No `//` comments — use `/* */` only

**Formatting (Uncrustify 0.72+, config at `CPM/CP/Tools/Format/MISRA-C.cfg`):**
- 2-space indentation, no tabs
- 80-column maximum
- Allman brace style (`{` on its own line)
- Pointer star attached to variable name: `uint8_t *ptr`

### Naming Conventions (system-wide)

This is the canonical convention for every module. Individual module
CLAUDE.md files should reference this section; any per-module deviation
must be justified in that module's file.

**Do not reintroduce Hungarian-style prefixes** — `b` / `s` / `l` / `f` /
`p` / `ba` / `sa` / `tS` / `tE` / `tp` / `S` (as struct-instance prefix).
They are retired across the project. Type information comes from the
declaration, not the identifier.

| Category | Style | Example |
|---|---|---|
| Functions | `PascalCase_WithModulePrefix` | `MeasurementService_Init`, `Eeprom_Read` |
| Struct / typedef'd types | `PascalCase_t` | `MeasurementServiceCtx_t`, `FdcanRxMsg_t`, `Phase_t` |
| Enum typedefs | `PascalCase_e` | `OffsetOperation_e`, `PhaseState_e` |
| Macros, enum constants | `SCREAMING_SNAKE_CASE` | `EEPROM_ADDR_PERIOD`, `OFFSET_OPERATION_SUM` |
| Variables, struct fields, parameters | `camelCase` | `flashPeriod`, `netVoltage` |
| Global storage-duration variables | `g_camelCase` | `g_canTxOverflowCount`, `g_eepromPort` |
| Static file-scope variables | `s_camelCase` | `s_svc`, `s_pendingOps` |

The `g_` / `s_` prefixes are kept as **scope** indicators (not type). C has
no other way to mark linkage at a glance; everything else infers type from
the declaration.

**Pointer typedefs** (`typedef Foo * FooPtr`) are avoided — they hide the
indirection. Callers use `Foo_t *` explicitly.

**Boundary rule — CubeMX-generated code.** The STM32CubeMX HAL init blocks
(everything outside `/* USER CODE BEGIN/END */` guards in `Core/`, plus the
HAL types themselves such as `FDCAN_HandleTypeDef`, `GPIO_InitTypeDef`,
`sMasterConfig` locals, etc.) still use ST's own `sFoo` / `h_foo` style.
Leave those alone — they regenerate on every `.ioc` export and mass-renames
there just create merge pain. The convention above applies only to our
user code: `App/**`, `Tests/**`, `Libs/**`, and the interior of `USER CODE`
guard blocks in `Core/**`.

**Migration status** (as of the commit that ships this CLAUDE.md):
- **PSM** — fully migrated. All user code conforms.
- **CP / MP / SSM / CPM shared libs** — not yet migrated; legacy Hungarian
  style still in use. New code should follow the convention above; existing
  code can be renamed opportunistically as files are touched.

## CI/CD

GitHub Actions workflows per module in `.github/workflows/`:
- `cp-build.yml` / `mp-build.yml` — STM32 firmware (push to main, PRs)
- `cp-test.yml` / `mp-test.yml` — host tests + **80% minimum coverage on `App/Domain/`**
- `cp-release.yml` / `mp-release.yml` — triggered by `v*.*.*` tags, attaches `.elf`/`.bin` to GitHub release

Coverage is measured by `gcovr` (config: `CPM/CP/gcovr.cfg`) and enforced in CI — PRs that drop `App/Domain/` coverage below 80% will fail.

## Test Architecture (CP)

Tests compile and run on x86-64 (no hardware needed) via the `Host-Test` preset:
- **Unit tests** (`Tests/Unit/`, 25+): Unity framework, one test binary per domain class
- **Integration tests** (`Tests/Integration/`): end-to-end phase cycle and conflict safety scenarios
- **Mock adapters** (`App/Adapters/Mock/`): in-memory implementations of all 26 ports — these are the test doubles used by host tests
- **Fixtures** (`Tests/Fixtures/`): shared `TimingPlan_4Phase.h`, `ConflictMatrix_6sg.h`

When adding a new port, a corresponding mock adapter in `App/Adapters/Mock/` is required before the domain code can be unit-tested on host.

## Vendor MIB Maintenance (1.3.6.1.4.1.59748)

`CPM/CP/Docs/TEKNOTEL-CPU4-MIB.mib` (module `TEKNOTEL-CPU4-MIB`, vendor "Teknotel Elektronik", IANA PEN 59748) is the single source-of-truth for every OID under the Teknotel enterprise arc. **Update it in the same commit** as any code change that:

- Adds, removes, or renumbers an OID (scalar, table, or table column)
- Changes the `SYNTAX`, `ACCESS`, or enumerated values of an existing object
- Adds or removes a specific-trap number emitted from `CPM/CP/LWIP/App/snmp_client.c`
- Touches the enums in `CPM/Libs/CpMpProtocolShared.h` that the MIB references (`CpMpConfigState_t`, `CpMpSafetyAction_t`, `CpMpFaultGlobalFlags_t`, `CpMpFaultChannelFlags_t`)

OID arc shape (mirrors NTCIP 8004/1202): `teknotel .59748 → transportation .4 → devices .2 → cpu4 .1 → { unit .3, channel .8, cpMpLink .20, driverModule .21 }`.

Authoritative C sources, one file per functional group:
- `CPM/CP/App/Domain/NTCIP/MibVendor59748/UnitObjects.c` — `unit` (`.3`)
- `CPM/CP/App/Domain/NTCIP/MibVendor59748/ChannelFaultObjects.c` — `channel` (`.8`)
- `CPM/CP/App/Domain/NTCIP/MibVendor59748/CpMpLinkObjects.c` — `cpMpLink` (`.20`)
- `CPM/CP/App/Domain/NTCIP/MibVendor59748/DriverModuleObjects.c` — `driverModule` (`.21`)
- `CPM/CP/App/Adapters/STM32/LWIPSNMPRootMibs.c` — one-stop LwIP subtree registration for every `cpu4.*` group
- `CPM/CP/LWIP/App/snmp_client.c` — enterprise-specific traps (`.59748.0.{1,2,3}`)

Keep the "OID Quick Reference" block at the bottom of the `.mib` in sync with the body.
