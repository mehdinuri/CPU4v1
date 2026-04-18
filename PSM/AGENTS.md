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
Follow C11 and keep domain code warning-clean under `-Wall -Wextra -Werror -Wpedantic`. Formatting is driven by `Tools/Format/MISRA-C.cfg` through `Tools/Scripts/format-user-code.sh`.

Match the existing style: 2-space indentation, braces on their own line, and naming patterns such as `bReady`, `pOffset`, `tSMeasurementOffset`, and `tEOffsetOperation`. Preserve current file naming conventions: domain modules use names like `MeasurementService.c`, while task modules use names like `can_msg_sender.c`.

## Testing Guidelines
Add new unit tests under `Tests/Unit/` with names like `Test_MeasurementService.c`. Each Unity test file should define its own `main()` and explicit `RUN_TEST(...)` list. Coverage is enforced only for `App/Domain/`, with a minimum 80% line threshold in `gcovr.cfg`.

If logic becomes hard to test inside `Tasks/`, move the computation into `App/Domain/` and test it there with mock adapters.

## Commit & Pull Request Guidelines
Recent history uses short, lowercase commit subjects such as `structure and tooling for ssm and psm`. Keep commits focused, use concise imperative summaries, and avoid mixing unrelated refactors.

Pull requests should say whether the change affects `Host`, `STM32`, or both, list the commands you ran, and note any hardware or Docker validation. Attach logs or coverage output when behavior changes make them useful.
