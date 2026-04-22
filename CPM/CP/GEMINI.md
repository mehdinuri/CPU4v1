# Gemini CLI Context: Intersection Controller (CP)

This project is a fully NTCIP 1201/1202 compliant intersection controller firmware for the **STM32H743VIT6** microcontroller. It implements a **NEMA TS2** actuated traffic signal controller and uses a **Hexagonal Architecture** (Ports & Adapters) to decouple core traffic signal logic from hardware peripherals, RTOS, and network stacks.

## Project Overview

- **Main Technologies**: C11, CMake, FreeRTOS, LwIP (SNMP), mbedTLS, STM32 HAL.
- **Architecture**:
    - `App/Domain/`: Pure C11 business logic (NEMA TS2 intersection engine, NTCIP objects). No dependencies on HAL or RTOS.
    - `App/Ports/`: C vtable-style interface definitions (function pointer structs).
    - `App/Adapters/`: Concrete implementations of ports.
        - `App/Adapters/STM32/`: Production adapters using STM32 HAL, FreeRTOS, and LwIP.
        - `App/Adapters/Mock/`: In-memory test doubles for host-side unit testing.
    - `App/Platform/`: Entry points and system wiring.
        - `App/Platform/STM32/`: Main firmware entry and FreeRTOS task orchestration.
        - `App/Platform/Host/`: Host-side smoke test runner.
    - `Core/`, `Drivers/`, `Middlewares/`: STM32CubeMX generated code and vendor drivers.

## Building and Running

### Prerequisites
- `arm-none-eabi-gcc` 13.x (for STM32 builds)
- `gcc` + `gcovr` (for host test builds)
- CMake 3.22+, Ninja

### Build Commands
- **STM32 Firmware**:
  ```bash
  cmake --preset STM32-Debug && cmake --build --preset STM32-Debug
  cmake --preset STM32-Release && cmake --build --preset STM32-Release
  ```
- **Host Unit Tests**:
  ```bash
  cmake --preset Host-Test && cmake --build --preset Host-Test
  cd build/Host-Test && ctest --output-on-failure
  ```
- **Coverage Report**:
  ```bash
  ./Tools/Scripts/build-test.sh
  ```
- **Flash to Hardware**:
  ```bash
  ./Tools/Scripts/flash.sh STM32-Release
  ```

## Development Conventions

- **Language**: C11 (strictly enforced `-Wall -Wextra -Werror -Wpedantic` for `App/Domain/` and `App/Ports/`).
- **Architecture Mandate**: `App/Domain/` code MUST NOT include HAL, FreeRTOS, or LwIP headers. Use `App/Ports/` for all external interactions.
- **Memory Management**: No `malloc`/`free` in `App/Domain/`. Use static allocation or stack-based structures.
- **State Management**: No global mutable state in `App/Domain/`. All state must be contained within `Context_t` structs passed by pointer.
- **Concurrency**: The `ProgramTick()` function is called every 100 ms in a high-priority FreeRTOS task. Thread safety between Domain logic and NTCIP/UI tasks is managed via mutexes in the adapters.
- **Naming**: PascalCase throughout. Port function-pointer members use PascalCase verbs (`SetLampState`, `Flush`). Inline dispatch helpers are `FeatureVerb()` with no underscore separators (`SignalOutputSetLamp`). Adapter functions follow `FeatureAdapterInit()` / `FeatureAdapterCreatePort()`. Follow NTCIP 1202 terminology for NEMA TS2 phase objects (e.g., `phaseMinimumGreen`, `phasePassage`, `phaseYellowChange`, `phaseRedClear`, `unitControl`, `channelControlSource`).
- **Testing**: Every new feature in `App/Domain/` must have a corresponding unit test in `Tests/Unit/` using the Unity framework. Coverage for `App/Domain/` should be ≥ 80%.
- **STM32CubeMX**: Code outside `/* USER CODE BEGIN */` guards in `Core/` will be overwritten by CubeMX. Application logic should reside in `App/`.

## Vendor MIB Maintenance (1.3.6.1.4.1.59748)

`Docs/TEKNOTEL-CPU4-MIB.mib` (module `TEKNOTEL-CPU4-MIB`, vendor "Teknotel Elektronik", IANA PEN 59748) is the single source-of-truth for every OID under the Teknotel enterprise arc. **Update it in the same commit** as any change that:

- Adds, removes, or renumbers an OID (scalar, table, or table column)
- Changes the `SYNTAX`, `ACCESS`, or enumerated values of an existing object
- Adds or removes a specific-trap number emitted from `LWIP/App/snmp_client.c`
- Touches the enums in `../Libs/CpMpProtocolShared.h` referenced by the MIB (`CpMpConfigState_t`, `CpMpSafetyAction_t`, `CpMpFaultGlobalFlags_t`, `CpMpFaultChannelFlags_t`)

OID arc shape (mirrors NTCIP 8004/1202): `teknotel .59748 → transportation .4 → devices .2 → cpu4 .1 → { unit .3, channel .8, cpMpLink .20, driverModule .21 }`.

Authoritative C sources, one file per functional group:
- `App/Domain/NTCIP/MibVendor59748/UnitObjects.c` — `unit` (`.3`)
- `App/Domain/NTCIP/MibVendor59748/ChannelFaultObjects.c` — `channel` (`.8`)
- `App/Domain/NTCIP/MibVendor59748/CpMpLinkObjects.c` — `cpMpLink` (`.20`)
- `App/Domain/NTCIP/MibVendor59748/DriverModuleObjects.c` — `driverModule` (`.21`)
- `App/Adapters/STM32/LWIPSNMPRootMibs.c` — one-stop LwIP subtree registration
- `LWIP/App/snmp_client.c` — enterprise-specific traps (`.59748.0.{1,2,3}`)

Keep the "OID Quick Reference" block at the foot of the `.mib` in sync with the body.
