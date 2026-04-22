# Repository Guidelines

## Project Structure & Module Organization
This repository builds STM32H743 firmware plus host-side tests. Keep new application logic in `App/`, not in generated vendor code.

- `App/Domain/`: pure C11 domain logic for intersection control and NTCIP behavior.
- `App/Ports/`: interface contracts used by the domain layer.
- `App/Adapters/STM32/` and `App/Adapters/Mock/`: production adapters and test doubles.
- `App/Platform/STM32/` and `App/Platform/Host/`: startup, task wiring, and host runner.
- `Tests/Unit/`, `Tests/Integration/`, `Tests/Fixtures/`: Unity tests and shared fixtures.
- `Core/`, `Drivers/`, `Middlewares/`, `LWIP/`, `MBEDTLS/`, `USB_DEVICE/`: CubeMX-generated or third-party code; avoid manual edits unless the change is truly platform/vendor related.

## Build, Test, and Development Commands
- `cmake --preset STM32-Debug && cmake --build --preset STM32-Debug`: build debug firmware to `build/STM32-Debug/CP.elf`.
- `cmake --preset STM32-Release && cmake --build --preset STM32-Release`: build release firmware.
- `cmake --preset Host-Test && cmake --build --preset Host-Test`: configure and build host tests.
- `cd build/Host-Test && ctest --output-on-failure`: run registered Unity tests.
- `./Tools/Scripts/build-test.sh`: build host tests, run CTest, and generate coverage in `build/coverage-report/`.
- `cmake --build build/Host-Test --target Format` or `--target Format-Check`: run or verify Uncrustify formatting.

## Coding Style & Naming Conventions
Use C11 and follow the repo's MISRA-oriented Uncrustify rules in `Tools/Format/MISRA-C.cfg`: 2-space indentation, spaces not tabs, Allman braces, explicit braces for every control block, pointer star attached to the variable name. Keep `App/Domain/` free of HAL, FreeRTOS, and LwIP dependencies.

Port-pattern conventions (still accurate, specific to CP's hexagonal layer): PascalCase for module and API names (`SignalOutputSetLamp`, `FeatureAdapterInit`), `IFeaturePort_t` for port structs, `FeatureAdapterCtx_t` for adapter context types.

Broader identifier conventions follow the system-wide rules in the root `CLAUDE.md` / `AGENTS.md` § "Naming Conventions": `PascalCase_t` types, `PascalCase_e` enums, `camelCase` variables/fields/params, `SCREAMING_SNAKE_CASE` macros, `g_` / `s_` for scope, **no Hungarian prefixes** (`b` / `s` / `l` / `f` / `p` / `tS` / `tE` / `tp` / `S` struct-instance prefix).

**Migration status: CP is not yet migrated** — legacy Hungarian-style identifiers are still present. New code follows the convention above; rename opportunistically when touching a file. Do not open blanket rename PRs that conflict with in-flight feature work.

## Testing Guidelines
Tests use Unity via CMake. Name test files `Test_<Module>.c` and register them with `Add_Unity_Test(...)` in `Tests/Unit/CMakeLists.txt` or `Tests/Integration/CMakeLists.txt`. Coverage is measured only for `App/Domain/`; `gcovr.cfg` currently fails CI below 50% line coverage.

## Commit & Pull Request Guidelines
Recent history favors short, imperative commit subjects such as `add ntcip mibs` or `gcov threshold to config file`; keep subjects concise, action-first, and scoped to one change. Pull requests should explain the affected layer(s), list the commands run (`ctest`, coverage, firmware build), and include screenshots or field notes only when UI, hardware behavior, or flashing steps change.

## Configuration Tips
`CP.ioc` and CubeMX outputs can overwrite code outside `USER CODE` guards. Put new behavior in `App/` whenever possible, and treat formatting or vendor updates as separate changes from domain logic.

## Vendor MIB Maintenance (1.3.6.1.4.1.59748)
`Docs/TEKNOTEL-CPU4-MIB.mib` (module `TEKNOTEL-CPU4-MIB`, vendor "Teknotel Elektronik") is the single source-of-truth for every OID under the Teknotel enterprise arc. Update it in the **same commit** as any change that adds/removes/renumbers an OID, alters `SYNTAX`/`ACCESS`/enums, changes a specific-trap number in `LWIP/App/snmp_client.c`, or modifies the enums in `../Libs/CpMpProtocolShared.h` referenced by the MIB (`CpMpConfigState_t`, `CpMpSafetyAction_t`, `CpMpFaultGlobalFlags_t`, `CpMpFaultChannelFlags_t`). Arc shape: `teknotel .59748 → transportation .4 → devices .2 → cpu4 .1 → { unit .3, channel .8, cpMpLink .20, driverModule .21 }`. Authoritative sources: `App/Domain/NTCIP/MibVendor59748/{UnitObjects,ChannelFaultObjects,CpMpLinkObjects,DriverModuleObjects}.c`, `App/Adapters/STM32/LWIPSNMPRootMibs.c` (one-stop subtree registration), and `LWIP/App/snmp_client.c`. Keep the "OID Quick Reference" block at the foot of the `.mib` in sync with the body.
