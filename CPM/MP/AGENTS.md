# Repository Guidelines

## Project Structure & Module Organization

**MP (Maestro Platform / Malfunction Processor)** is embedded firmware for the C0502-P251207-CPU4 Maestro Intersection Controller — an STM32G473 (Cortex-M4 @ 160 MHz, 1 MB Flash, 128 KB SRAM) with FreeRTOS and FDCAN. Current firmware version is defined in `Core/Inc/MpVersion.h`.

The sibling `../CP` (Control Processor) is the peer over the CAN FD control bus. MP is being migrated toward CP's hexagonal architecture; see `../AGENTS.md` for the cross-module picture.

- `App/Domain/` — pure C11 domain logic (MMU orchestrator, fault/safety services, CP↔MP link service, configuration service). HAL/RTOS-free.
- `App/Ports/` — interface contracts used by the domain layer.
- `App/Adapters/STM32/` and `App/Adapters/Mock/` — production and test-double implementations.
- `App/Platform/STM32/Tasks/Tasks.c` — the active FreeRTOS task layer (`MP_Malfunction`, `MP_Maintenance`).
- `Core/` — STM32CubeMX-generated startup and HAL init. Only edit inside `/* USER CODE BEGIN/END */` guards.
- `Tests/Unit/` — Unity host tests.
- `Drivers/`, `Middlewares/` — STM32G4 HAL + FreeRTOS v10 vendor code; do not modify.
- `cmake/` — toolchain and CubeMX CMake wrappers.

## Build, Test, and Development Commands

Prerequisites: `arm-none-eabi-gcc` 13.x, `gcc` + `gcovr` for host coverage, CMake 3.22+/Ninja, Uncrustify 0.72+.

```bash
# STM32 firmware
cmake --preset STM32-Debug && cmake --build --preset STM32-Debug     # build/STM32-Debug/MP.elf
cmake --preset STM32-Release && cmake --build --preset STM32-Release # build/STM32-Release/MP.elf

# Host unit tests
cmake --preset Host-Test && cmake --build --preset Host-Test
(cd build/Host-Test && ctest --output-on-failure)

# Coverage report
./Tools/Scripts/build-test.sh

# Flash
./Tools/Scripts/flash.sh STM32-Release   # STM32_Programmer_CLI or openocd

# Formatter (MISRA-C, uncrustify)
cmake --build build/Host-Test --target Format        # apply in-place
cmake --build build/Host-Test --target Format-Check  # CI-safe check

# Docker parity (mirrors CI)
docker compose -f Tools/Docker/compose.yml run build-arm
docker compose -f Tools/Docker/compose.yml run build-test
```

`build/STM32-Debug/` must exist before clangd can resolve includes.

## FreeRTOS Tasks

Defined in `App/Platform/STM32/Tasks/Tasks.c`:

| Task | Role |
|---|---|
| `MP_Malfunction` | Steps control-bus RX, field-bus RX, MMU monitoring, CP↔MP service logic |
| `MP_Maintenance` | Lightweight periodic maintenance hook |

## Domain Model (`App/Domain/`)

The MMU domain centres on clean App-layer services and monitors:
- `MalfunctionEngine` — 10 ms MMU orchestrator for SSM/PSM health and channel-fault monitors
- `FaultMonitorService` — structured NEMA/MMU fault surface and event trace
- `SafetyDecisionService` — relay/safety action ownership
- `CpMpLinkService` — CP↔MP configuration, heartbeat, fault status, and fault-event transport
- `ConfigurationService` — applied MP config image and output mapping state

## CubeMX Integration

`MP.ioc` is the CubeMX project. Re-generating overwrites everything outside `/* USER CODE BEGIN/END */` guards. All peripheral init in `Core/Src/` (`gpio.c`, `adc.c`, `fdcan.c`, `i2c.c`, etc.) is generated — application logic belongs inside USER CODE blocks only.

## Architecture Direction

MP currently retains monolithic patches (global state in `data.c`, direct HAL calls in tasks). Port interface pattern for new hardware interactions:

```c
typedef struct
{
    void *ctx;
    void (*MethodName)(void *ctx, /* args */);
} IFeaturePort_t;
```

Domain code only calls the inline dispatch helpers — no `#ifdef HARDWARE` inside the domain.

## Coding Style & Naming Conventions

C11 throughout — no C++. Formatting follows the CP's Uncrustify MISRA config referenced by `cmake --build ... --target Format` (2-space indent, Allman braces, explicit braces on every control body, pointer star attached to variable name, no `//` comments).

- `common.h` provides bit macros (`GetBitValue`, `SetBitValue`, `ClearBitValue`) and byte-extraction macros (`mMsb`, `mLsb`, `mMsw`, `mLsw`) — use these instead of raw bit operations.
- All peripherals use STM32 HAL (not LL) except where already using LL.
- `__attribute__((packed))` is applied to all structs stored in EEPROM or sent over CAN.

Naming follows the system-wide rules in the root `CLAUDE.md` / `AGENTS.md` § "Naming Conventions": `PascalCase_t` types, `PascalCase_e` enums, `camelCase` variables/fields/params, `SCREAMING_SNAKE_CASE` macros, `g_` / `s_` for scope, **no Hungarian prefixes** (`b` / `s` / `l` / `f` / `p` / `tS` / `tE` / `tp` / `S` struct-instance prefix).

**Migration status: MP is not yet migrated.** Legacy Hungarian-style identifiers (e.g. `tSSetRuntime`, `tpSSetRuntime`, `bFoo`, `sFoo`) are still present in the existing codebase. New code follows the convention above; rename opportunistically when touching a file. Do not open blanket rename PRs that conflict with in-flight feature work.

## Testing Guidelines

Unit tests live under `Tests/Unit/` with names like `Test_<Module>.c`. Each Unity test file defines its own `main()` with explicit `RUN_TEST(...)` calls. Coverage is enforced on `App/Domain/` via `gcovr.cfg`. If logic in `Tasks/` becomes hard to test on hardware, extract it into `App/Domain/` and mock-adapter-test it on host first.

## Commit & Pull Request Guidelines

Short, imperative commit subjects. PRs should state which build targets were exercised (`STM32`, `Host`, both), include `ctest` output on domain-logic changes, and list any hardware validation performed. If the change touches `../Libs/CpMpProtocolShared.h` it is a cross-module change — rebuild and test CP too, and bump `CPMP_PROTOCOL_VERSION` in the same commit.

## Vendor MIB Maintenance (1.3.6.1.4.1.59748)

MP does not serve SNMP directly, but every field MP publishes over the CP/MP CAN FD link ends up exposed by the CP SNMP agent under the Teknotel enterprise arc `1.3.6.1.4.1.59748`. The canonical `.mib` (vendor "Teknotel Elektronik") lives at `../CP/Docs/TEKNOTEL-CPU4-MIB.mib`. OID arc shape: `teknotel .59748 → transportation .4 → devices .2 → cpu4 .1 → { unit .3, channel .8, cpMpLink .20, driverModule .21 }`.

**Update that `.mib` in the same commit** whenever MP work changes:

- The set of bits in `CpMpFaultGlobalFlags_t` or `CpMpFaultChannelFlags_t` (mirrored by `cpMpLinkGlobalFlags` and `channelFaultFlags`)
- Enumerations `CpMpConfigState_t` or `CpMpSafetyAction_t` (mirrored by `cpMpLinkConfigState` and `cpMpLinkSafetyAction`)
- The reason-code namespace reported via `cpMpLinkSafetyReasonCode`
- Any bump of `CPMP_PROTOCOL_VERSION` (mirrored by `cpMpLinkProtocolVersion`)

Do not add new OIDs under `59748` from MP-side work without a paired CP-side object registration in `CP/App/Domain/NTCIP/MibVendor59748/` (`UnitObjects.c`, `ChannelFaultObjects.c`, `CpMpLinkObjects.c`, `DriverModuleObjects.c`).
