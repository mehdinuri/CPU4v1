# Repository Guidelines

## Project Overview
CPU4v1 is an intersection controller system — refactored from EU stage-based logic to **NEMA TS2 dual-ring barrier** with full **NTCIP 1201/1202** compliance. The repo is a multi-module embedded C workspace; each module has its own `AGENTS.md` and `CLAUDE.md` with module-specific details.

| Module | MCU | Role |
|--------|-----|------|
| **CPM/CP** (Controller Processor) | STM32H743VIT6 (Cortex-M7 @ 480 MHz) | NEMA TS2 intersection engine, NTCIP SNMP, coordinator |
| **CPM/MP** (Malfunction Processor) | STM32G473 (Cortex-M4 @ 160 MHz) | Signal-card safety, malfunction detection, conflict watchdog |
| **PSM** (Power Supply Module) | STM32G473 | AC/DC voltage/frequency monitoring, calibration storage |
| **SSM** (Signal Switching Module) | STM32G473 | Red/Yellow/Green channel switching, per-output measurement |

Modules communicate over **CAN/FDCAN** using a snapshot protocol; CP is the bus master. Shared types and CAN-wire enums live in `CPM/Libs/`.

## Build, Test, and Development Commands
All modules use CMake. CP has named presets; MP/PSM/SSM use direct CMake invocation.

```
# CP firmware + host tests
cd CPM/CP
cmake --preset STM32-Debug   && cmake --build --preset STM32-Debug
cmake --preset STM32-Release && cmake --build --preset STM32-Release
cmake --preset Host-Test     && cmake --build --preset Host-Test
cd build/Host-Test && ctest --output-on-failure

# PSM (also has named presets)
cd PSM
cmake --preset STM32-Debug && cmake --build --preset STM32-Debug
cmake --preset Host-Test   && cmake --build --preset Host-Test
ctest --preset Host-Test --output-on-failure

# MP / SSM
cd CPM/MP   # or SSM
cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/gcc-arm-none-eabi.cmake
cmake --build build

# Formatter (applies to every module individually)
cmake --build build/STM32-Debug --target Format        # apply
cmake --build build/STM32-Debug --target Format-Check  # CI-safe check
```

Docker parity: each module ships a `Tools/Docker/compose.yml` with `build-arm` (STM32 firmware) and `build-test` (host + coverage) services.

## Coding Style

- **C11** throughout, no C++.
- **Domain-layer rules** (`App/Domain/` in every module): no `malloc` (static pools only), no global mutable state (state lives in context structs), no HAL / FreeRTOS / network API calls.
- **MISRA-C** enforced by Uncrustify + `-Werror`:
  - Explicit `{ }` braces on every control-flow body (Rule 15.6)
  - Every `case` ends with `break` or `/* fallthrough */` (Rule 16.3)
  - Every `switch` has a `default:` clause last (Rules 16.4/16.5)
  - Array indexing only — no pointer arithmetic (Rule 17.4)
  - No `//` comments — use `/* */`
- **Formatting** (Uncrustify 0.72+, config at `CPM/CP/Tools/Format/MISRA-C.cfg`): 2-space indentation, no tabs, 80-column max, Allman braces (`{` on its own line), pointer star attached to variable name: `uint8_t *ptr`.

## Naming Conventions

**Do not reintroduce Hungarian-style prefixes** — `b` / `s` / `l` / `f` / `p` / `ba` / `sa` / `tS` / `tE` / `tp` / `S` (as struct-instance prefix). They are retired across the project. Type information comes from the declaration, not the identifier.

| Category | Style | Example |
|---|---|---|
| Functions | `PascalCase_WithModulePrefix` | `MeasurementService_Init`, `Eeprom_Read` |
| Struct / typedef'd types | `PascalCase_t` | `MeasurementServiceCtx_t`, `FdcanRxMsg_t`, `Phase_t` |
| Enum typedefs | `PascalCase_e` | `OffsetOperation_e`, `PhaseState_e` |
| Macros, enum constants | `SCREAMING_SNAKE_CASE` | `EEPROM_ADDR_PERIOD`, `OFFSET_OPERATION_SUM` |
| Variables, struct fields, parameters | `camelCase` | `flashPeriod`, `netVoltage` |
| Global storage-duration variables | `g_camelCase` | `g_canTxOverflowCount`, `g_eepromPort` |
| Static file-scope variables | `s_camelCase` | `s_svc`, `s_pendingOps` |

The `g_` / `s_` prefixes are kept as **scope** indicators (not type). C has no other way to mark linkage at a glance; everything else infers type from the declaration.

**Pointer typedefs** (`typedef Foo * FooPtr`) are avoided — they hide the indirection. Callers use `Foo_t *` explicitly.

**Boundary rule — CubeMX-generated code**: the STM32CubeMX HAL init blocks (everything outside `/* USER CODE BEGIN/END */` guards in `Core/`, plus the HAL types themselves such as `FDCAN_HandleTypeDef`, `GPIO_InitTypeDef`, `sMasterConfig` locals) still use ST's own `sFoo` / `h_foo` style. Leave those alone — they regenerate on every `.ioc` export. The convention above applies to `App/**`, `Tests/**`, `Libs/**`, and the interior of `USER CODE` guard blocks in `Core/**`.

**Migration status** (as of the commit that ships this AGENTS.md):
- **PSM** — fully migrated. All user code conforms.
- **CP / MP / SSM / CPM shared libs** — not yet migrated; legacy Hungarian style still in use. New code follows the convention above; existing code can be renamed opportunistically as files are touched.

## Testing Guidelines

Tests compile and run on x86-64 via each module's `Host-Test` CMake preset. Unity framework, one test binary per domain class, Unity `main()` per file with explicit `RUN_TEST(...)` calls.

- CP enforces **80 % minimum line coverage** on `App/Domain/` via `gcovr` (`CPM/CP/gcovr.cfg`).
- PSM enforces **80 %** similarly (`PSM/gcovr.cfg`).
- Mock adapters live in `App/Adapters/Mock/` and must exist before a new port can be host-tested.

When new logic in `Tasks/` becomes hard to test on hardware, extract the pure computation into `App/Domain/` and add a Unity test first.

## Commit & Pull Request Guidelines

- Short, imperative subjects (e.g. `add ntcip mibs`, `fix watchdog off-by-two`). No Conventional Commits prefix required by the project.
- PRs: state which module(s) are affected (`PSM`, `CP`, `MP`, `SSM`, `Libs`) and which build targets (`STM32`, `Host`, both), list the commands run (`ctest`, firmware build, format check), and attach coverage output when behavior shifts.
- Do not mix unrelated refactors with feature work. The migration rename is an example of "touch-when-you're-here" — batch it into the same commit as the feature change in that area, not as a separate sweep.

## CI/CD
Per-module GitHub Actions workflows live in `.github/workflows/`:
- `cp-build.yml`, `mp-build.yml`, `psm-build.yml`, `ssm-build.yml` — STM32 firmware builds on push/PR.
- `cp-test.yml`, `mp-test.yml`, `psm-test.yml` — host tests + gcovr coverage report.
- `cp-release.yml`, `mp-release.yml`, `psm-release.yml` — triggered by module-specific tags (e.g. `psm-v*.*.*`); attach `.elf` / `.bin` to GitHub Release.

Coverage is enforced in CI — a PR that drops `App/Domain/` coverage below the per-module threshold fails the build.

## Vendor MIB Maintenance (1.3.6.1.4.1.59748)

`CPM/CP/Docs/TEKNOTEL-CPU4-MIB.mib` (module `TEKNOTEL-CPU4-MIB`, vendor "Teknotel Elektronik", IANA PEN 59748) is the single source-of-truth for every OID under the Teknotel enterprise arc. Update it in the **same commit** as any change that:

- Adds, removes, or renumbers an OID (scalar, table, or table column)
- Changes the `SYNTAX`, `ACCESS`, or enumerated values of an existing object
- Adds or removes a specific-trap number emitted from `CPM/CP/LWIP/App/snmp_client.c`
- Touches the enums in `CPM/Libs/CpMpProtocolShared.h` that the MIB references (`CpMpConfigState_t`, `CpMpSafetyAction_t`, `CpMpFaultGlobalFlags_t`, `CpMpFaultChannelFlags_t`)

OID arc shape: `teknotel .59748 → transportation .4 → devices .2 → cpu4 .1 → { unit .3, channel .8, cpMpLink .20, driverModule .21 }`. Authoritative per-group C sources in `CPM/CP/App/Domain/NTCIP/MibVendor59748/`. Keep the "OID Quick Reference" block at the foot of the `.mib` in sync with the body.
