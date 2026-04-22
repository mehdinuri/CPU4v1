# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**MP (Maestro Platform)** is embedded firmware for the **C0502-P251207-CPU4 Maestro Intersection Controlling System** — a traffic signal controller running on an STM32G473 (ARM Cortex-M4 @ 160 MHz, 1 MB Flash, 128 KB SRAM) with FreeRTOS and FDCAN communication.

Current firmware version: **3.4.0.1B** (defined in `Core/Inc/MpVersion.h`).

The sibling project `../CP` (Control Processor) is the peer device that communicates with MP over CAN FD. CP's `CLAUDE.md` documents the hexagonal architecture pattern that MP is being migrated toward.

## Build Commands

### Prerequisites
- `arm-none-eabi-gcc` 13.x (for STM32 builds)
- `gcc` + `gcovr` (for host test builds)
- CMake 3.22+, Ninja
- `uncrustify` (for `Format`/`Format-Check` targets — `sudo apt install uncrustify`)

### STM32 firmware
```bash
cmake --preset STM32-Debug && cmake --build --preset STM32-Debug     # → build/STM32-Debug/MP.elf
cmake --preset STM32-Release && cmake --build --preset STM32-Release # → build/STM32-Release/MP.elf
```

The `build/STM32-Debug/` directory must exist before clangd can resolve includes (`.clangd` → `CompilationDatabase: build/Debug`; update this path after the first configure if needed).

### Host unit tests (x86-64, no hardware needed)
```bash
cmake --preset Host-Test && cmake --build --preset Host-Test
cd build/Host-Test && ctest --output-on-failure
# With coverage report:
./Tools/Scripts/build-test.sh
```

### Flash to hardware
```bash
./Tools/Scripts/flash.sh STM32-Release   # uses STM32_Programmer_CLI or openocd
```

### Code formatting (MISRA-C, uncrustify)
```bash
cmake --build build/Host-Test --target Format        # reformat Tasks/ and Tests/ in-place
cmake --build build/Host-Test --target Format-Check  # CI-safe check, exits 1 if reformatting needed
```

### Via Docker (mirrors CI exactly)
```bash
docker compose -f Tools/Docker/compose.yml run build-arm    # STM32 firmware
docker compose -f Tools/Docker/compose.yml run build-test   # host tests + coverage
```

## Architecture

### Directory Structure

```
Core/          STM32CubeMX-generated HAL init + low-level platform helpers
Tasks/         Application task implementations (one .c per FreeRTOS task)
Drivers/       STM32G4xx HAL + CMSIS (do not modify)
Middlewares/   FreeRTOS v10 + CMSIS-RTOS V2 wrapper (do not modify)
cmake/         Toolchain and STM32CubeMX CMake wrappers
```

### FreeRTOS Task Structure

The active runtime uses the new platform task layer in `App/Platform/STM32/Tasks/Tasks.c`.

| Task | File | Role |
|---|---|---|
| `MP_Malfunction` | `App/Platform/STM32/Tasks/Tasks.c` | Steps control-bus RX, field-bus RX, MMU monitoring, and CP↔MP service logic |
| `MP_Maintenance` | `App/Platform/STM32/Tasks/Tasks.c` | Lightweight periodic maintenance hook |

### Domain Model (App/Domain)

The MMU domain is now centered on the clean App-layer services and monitors:

- `MalfunctionEngine`: 10 ms MMU orchestrator for SSM/PSM health and channel-fault monitors
- `FaultMonitorService`: structured NEMA/MMU fault surface and event trace
- `SafetyDecisionService`: relay/safety action ownership
- `CpMpLinkService`: CP↔MP configuration, heartbeat, fault status, and fault-event transport
- `ConfigurationService`: applied MP config image and output mapping state

### STM32CubeMX Generated Code

`MP.ioc` is the CubeMX project. Re-generating overwrites all code **outside** `/* USER CODE BEGIN */` / `/* USER CODE END */` guards. All peripheral init in `Core/Src/` (gpio.c, adc.c, fdcan.c, i2c.c, etc.) is generated — application logic belongs inside USER CODE blocks only.

### Architecture Direction

MP currently uses a monolithic architecture (global state in `data.c`, direct HAL calls in tasks). The sibling CP project implements hexagonal architecture (ports/adapters); MP is being migrated toward that pattern. When adding new hardware interactions, follow CP's port interface pattern:

```c
typedef struct
{
    void *ctx;
    void (*MethodName)(void *ctx, /* args */);
} IFeaturePort_t;
```

## Coding Conventions

- **C11** throughout — no C++
- **Naming** — follow the system-wide rules in the root `CLAUDE.md` § "Naming Conventions (system-wide)":
  `PascalCase_t` types, `PascalCase_e` enums, `camelCase` variables/fields/params, `SCREAMING_SNAKE_CASE` macros, `g_` / `s_` for scope, **no Hungarian prefixes** (`b` / `s` / `l` / `f` / `p` / `tS` / `tE` / `tp`).
  MP is **not yet migrated**; legacy `tSSetRuntime` / `tpSSetRuntime` / `bFoo` / `sFoo` style is still present. New code follows the convention above; existing files can be renamed opportunistically when touched.
- `common.h` provides bit manipulation macros (`GetBitValue`, `SetBitValue`, `ClearBitValue`) and byte-extraction macros (`mMsb`, `mLsb`, `mMsw`, `mLsw`) — use these instead of raw bit operations
- All peripherals use STM32 HAL (not LL) except where already using LL
- `__attribute__((packed))` is applied to all structs stored in EEPROM or sent over CAN

## Vendor MIB Maintenance (1.3.6.1.4.1.59748)

MP does not serve SNMP directly, but every field MP publishes over the CP/MP CAN FD link ends up exposed by the CP SNMP agent under the Teknotel enterprise arc `1.3.6.1.4.1.59748`. The canonical `.mib` (vendor "Teknotel Elektronik") lives at `../CP/Docs/TEKNOTEL-CPU4-MIB.mib`. OID arc shape: `teknotel .59748 → transportation .4 → devices .2 → cpu4 .1 → { unit .3, channel .8, cpMpLink .20, driverModule .21 }`.

**Update that `.mib` in the same commit** whenever MP work changes:

- The set of bits in `CpMpFaultGlobalFlags_t` or `CpMpFaultChannelFlags_t` (mirrored by `cpMpLinkGlobalFlags` and `channelFaultFlags`)
- Enumerations `CpMpConfigState_t` or `CpMpSafetyAction_t` (mirrored by `cpMpLinkConfigState` and `cpMpLinkSafetyAction`)
- The reason-code namespace reported via `cpMpLinkSafetyReasonCode`
- Any bump of `CPMP_PROTOCOL_VERSION` (mirrored by `cpMpLinkProtocolVersion`)

Do not add new OIDs under `59748` from MP-side work without a paired CP-side object registration in `CP/App/Domain/NTCIP/MibVendor59748/` (one file per functional group: `UnitObjects.c`, `ChannelFaultObjects.c`, `CpMpLinkObjects.c`, `DriverModuleObjects.c`).
