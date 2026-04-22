# Repository Guidelines

## Project Structure & Module Organization
`App/Domain/` contains pure C11 business logic and should stay free of HAL and FreeRTOS dependencies. `App/Ports/` defines the interfaces the domain uses. `App/Adapters/STM32/` and `App/Adapters/Mock/` provide hardware and test implementations, while `App/Platform/STM32/` and `App/Platform/Host/` wire each build target together. `Tasks/` holds FreeRTOS task-level orchestration.

`Core/` is STM32CubeMX-generated startup and peripheral code; only edit user sections guarded for regeneration safety. `Tests/Unit/` contains Unity-based host tests. `Tools/` holds Docker, formatting, and helper scripts. `Drivers/` and `Middlewares/` are vendor code and should not be edited by hand.

## Build, Test, and Development Commands
Use the CMake presets in `CMakePresets.json`:

- `cmake --preset STM32-Debug && cmake --build --preset STM32-Debug` builds debug firmware for the STM32G473 target.
- `cmake --preset STM32-Release && cmake --build --preset STM32-Release` builds release firmware.
- `cmake --preset Host-Test && cmake --build --preset Host-Test` builds the x86-64 host test target.
- `ctest --preset Host-Test --output-on-failure` runs the Unity test suite.
- `gcovr --root . build/Host-Test` reports domain-layer coverage using `gcovr.cfg`.
- `cmake --build build/STM32-Debug --target Format` applies Uncrustify; use `Format-Check` for CI-safe validation.

For containerized builds, run `docker compose run --rm build-test` or `build-arm` from `Tools/Docker/`.

## Coding Style & Naming Conventions
Follow C11 and keep domain code warning-clean under `-Wall -Wextra -Werror -Wpedantic`. Formatting is driven by `Tools/Format/MISRA-C.cfg` through `Tools/Scripts/format-user-code.sh`: 2-space indentation, no tabs, 80-column max, Allman braces, pointer star attached to the variable name.

Naming follows the system-wide rules in the root `CLAUDE.md` / `AGENTS.md` § "Naming Conventions": `PascalCase_t` types, `PascalCase_e` enums, `camelCase` variables/fields/params, `SCREAMING_SNAKE_CASE` macros, `g_` / `s_` for scope, **no Hungarian prefixes** (`b` / `s` / `l` / `f` / `p` / `tS` / `tE` / `tp` / `S` struct-instance prefix).

**Migration status: PSM is fully migrated.** Do not reintroduce the retired Hungarian-style identifiers — any legacy example you may see in agent memory (`bReady`, `pOffset`, `tSMeasurementOffset`, `tEOffsetOperation`) is no longer present in PSM user code. File names use `PascalCase.c` (e.g. `MeasurementService.c`, `CanMsgSender.c`, `Storage.c`) — not `snake_case`.

## Testing Guidelines
Add new unit tests under `Tests/Unit/` with names like `Test_MeasurementService.c`. Each Unity test file should define its own `main()` and explicit `RUN_TEST(...)` list. Coverage is enforced only for `App/Domain/`, with a minimum 80% line threshold in `gcovr.cfg`.

If logic becomes hard to test inside `Tasks/`, move the computation into `App/Domain/` and test it there with mock adapters.

## Commit & Pull Request Guidelines
Recent history uses short, lowercase commit subjects such as `structure and tooling for ssm and psm`. Keep commits focused, use concise imperative summaries, and avoid mixing unrelated refactors.

Pull requests should say whether the change affects `Host`, `STM32`, or both, list the commands you ran, and note any hardware or Docker validation. Attach logs or coverage output when behavior changes make them useful.

## Vendor MIB Maintenance (1.3.6.1.4.1.59748)
PSM telemetry surfaces through the CP SNMP agent under the Teknotel enterprise arc `1.3.6.1.4.1.59748`. The canonical `.mib` (vendor "Teknotel Elektronik") lives at `../CPM/CP/Docs/TEKNOTEL-CPU4-MIB.mib`. Arc shape: `teknotel .59748 → transportation .4 → devices .2 → cpu4 .1 → { unit .3, channel .8, cpMpLink .20, driverModule .21 }`. Update the `.mib` in the **same commit** whenever PSM-side work changes the semantics of any `CpMpFaultGlobalFlags_t` bit driven from PSM (`AC_LINE`, `RAIL_24V`, `RAIL_5V`, `PSM_MISSING`) mirrored by `cpMpLinkGlobalFlags`, or the trigger for `teknotelPowerDownTrap` (specific-trap 1, `.59748.0.1`) emitted by `CPM/CP/LWIP/App/snmp_client.c`. New OIDs must be registered on the CP side (`CPM/CP/App/Domain/NTCIP/MibVendor59748/`, one file per functional group) and added to the `.mib` together.
