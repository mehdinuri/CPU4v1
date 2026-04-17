# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**MP (Maestro Platform)** is embedded firmware for the **C0502-P251207-CPU4 Maestro Intersection Controlling System** — a traffic signal controller running on an STM32G473 (ARM Cortex-M4 @ 160 MHz, 1 MB Flash, 128 KB SRAM) with FreeRTOS and FDCAN communication.

Current firmware version: **3.4.0.1B** (defined in `Core/Inc/data.h`).

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
Core/          STM32CubeMX-generated HAL init + application data model
  Inc/data.h   All domain type definitions and constants
  Src/data.c   All domain state, helper functions, and business logic
  Src/app_freertos.c  Task creation, queues, and memory pools
Tasks/         Application task implementations (one .c per FreeRTOS task)
Drivers/       STM32G4xx HAL + CMSIS (do not modify)
Middlewares/   FreeRTOS v10 + CMSIS-RTOS V2 wrapper (do not modify)
cmake/         Toolchain and STM32CubeMX CMake wrappers
```

### FreeRTOS Task Structure

All inter-task communication uses FreeRTOS queues and static memory pools defined in `app_freertos.c`.

| Task | File | Role |
|---|---|---|
| `APP_TASK_CAN_MSG_PARSER` | `Tasks/Src/CANRxTx.c` | Parses incoming CAN messages from CP |
| `APP_TASK_CAN_MSG_SENDER` | `Tasks/Src/CANRxTx.c` | Transmits CAN responses to CP |
| `APP_TASK_SIGNAL_OUTPUT_CATCH` | `Tasks/Src/signalOutputCatch.c` | Monitors SSM/PSM lamp output states |
| `APP_TASK_SIGNAL_CHECK` | `Tasks/Src/signalCheck.c` | Checks lamp voltage/current; detects failures |
| `APP_TASK_MAINTENANCE` | `Tasks/Src/maintenance.c` | Watchdog feed, device maintenance |

### Domain Model (Core/Inc/data.h, Core/Src/data.c)

All domain types and global state live here. Key structures:

- **Signal Sets** (`SIGNAL_SETS_MAX = 3`): Independent logical signal groups, each with its own signaling mode.
- **Signals** (`SIGNALS_MAX = 16`): User-defined signal programs composed of three subsignals (Red/Yellow/Green), each with a `sPeriod`.
- **Signal Outputs** (`SIGNAL_OUTPUTS_MAX = 96`): Physical lamp outputs mapped to SSM/PSM devices.
- **`tSMPProgramData`**: Full program configuration (sets, signals, outputs).
- **`tSMPProgramRuntimes`**: Live runtime state across all sets.

Signaling modes: `SIGNALING_MODE_NORMAL`, `SIGNALING_MODE_FLASH`, `SIGNALING_MODE_EMERGENCY_FLASH`, `SIGNALING_MODE_EMERGENCY_DARK`. The emergency flag bit is `0x08` — use `SIGNALING_MODE_EMERGENCY_FLAG` to test.

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
- Naming: existing code uses `snake_case` with `t`/`tp` prefixes for typedefs (`tSSetRuntime`, `tpSSetRuntime`) and `SCREAMING_SNAKE` for constants
- `common.h` provides bit manipulation macros (`GetBitValue`, `SetBitValue`, `ClearBitValue`) and byte-extraction macros (`mMsb`, `mLsb`, `mMsw`, `mLsw`) — use these instead of raw bit operations
- All peripherals use STM32 HAL (not LL) except where already using LL
- `__attribute__((packed))` is applied to all structs stored in EEPROM or sent over CAN
