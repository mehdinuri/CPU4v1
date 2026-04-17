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
Use C11 and follow the repo's MISRA-oriented Uncrustify rules in `Tools/Format/MISRA-C.cfg`: 2-space indentation, spaces not tabs, Allman braces, and explicit braces for control blocks. Prefer PascalCase for module and API names (`SignalOutputSetLamp`, `FeatureAdapterInit`), `IFeaturePort_t` for port structs, and `FeatureAdapterCtx_t` for adapter context types. Keep `App/Domain/` free of HAL, FreeRTOS, and LwIP dependencies.

## Testing Guidelines
Tests use Unity via CMake. Name test files `Test_<Module>.c` and register them with `Add_Unity_Test(...)` in `Tests/Unit/CMakeLists.txt` or `Tests/Integration/CMakeLists.txt`. Coverage is measured only for `App/Domain/`; `gcovr.cfg` currently fails CI below 50% line coverage.

## Commit & Pull Request Guidelines
Recent history favors short, imperative commit subjects such as `add ntcip mibs` or `gcov threshold to config file`; keep subjects concise, action-first, and scoped to one change. Pull requests should explain the affected layer(s), list the commands run (`ctest`, coverage, firmware build), and include screenshots or field notes only when UI, hardware behavior, or flashing steps change.

## Configuration Tips
`CP.ioc` and CubeMX outputs can overwrite code outside `USER CODE` guards. Put new behavior in `App/` whenever possible, and treat formatting or vendor updates as separate changes from domain logic.
