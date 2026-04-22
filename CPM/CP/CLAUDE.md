# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Fully NTCIP 1201/1202 compliant intersection controller firmware for the **STM32H743VIT6** (ARM Cortex-M7 @ 480 MHz, 2 MB Flash, 1056 KB RAM). Implements a **NEMA TS2** actuated traffic signal controller with NTCIP-compliant SNMP management, using hexagonal architecture to decouple domain logic from hardware, RTOS, and network stack.

## Build Commands

### Prerequisites
- `arm-none-eabi-gcc` 13.x (for STM32 builds)
- `gcc` + `gcovr` (for host test builds)
- CMake 3.22+, Ninja
- `uncrustify` (for `Format`/`Format-Check` targets — `sudo apt install uncrustify`)

### STM32 firmware
```bash
cmake --preset STM32-Debug && cmake --build --preset STM32-Debug    # → build/STM32-Debug/CP.elf
cmake --preset STM32-Release && cmake --build --preset STM32-Release  # → build/STM32-Release/CP.elf
```

### Host unit tests (x86-64, no hardware needed)
```bash
cmake --preset Host-Test && cmake --build --preset Host-Test
cd build/Host-Test && ctest --output-on-failure
# With coverage:
./Tools/Scripts/build-test.sh   # runs tests + generates build/coverage/index.html
```

### Via Docker (mirrors CI exactly)
```bash
docker compose -f Tools/Docker/compose.yml run build-arm    # STM32 firmware
docker compose -f Tools/Docker/compose.yml run build-test   # host tests + coverage
```

### Flash to hardware
```bash
./Tools/Scripts/flash.sh STM32-Release   # uses STM32_Programmer_CLI or openocd
```

### CI/CD
Three GitHub Actions workflows: `build.yml` (STM32 firmware on push to main), `test.yml` (host tests on every push), `release.yml` (triggered by `v*.*.*` tags → attaches .elf + .bin to GitHub release). Coverage must stay ≥ 80% on `App/Domain/`.

## Architecture

### Hexagonal Architecture (Ports & Adapters)

The codebase is organized so `App/Domain/` has **zero** dependencies on HAL, FreeRTOS, or LwIP. Platform-specific code lives in adapters, which are injected at startup:

```
App/Domain/            Pure C11 business logic — intersection engine, NTCIP objects
App/Ports/             C vtable port interfaces (structs of function pointers)
App/Adapters/STM32/    Concrete STM32H743 port implementations (HAL/FreeRTOS/LwIP allowed)
App/Adapters/Mock/     In-memory test doubles for host unit testing
App/Platform/STM32/    Thin FreeRTOS task wrappers + adapter wiring (main_stm32.c)
App/Platform/Host/     Host entry point (smoke test runner)
Tests/                 Unity-based unit and integration tests
```

**Port interface pattern** (all ports follow this):
```c
typedef struct
{
    void *ctx;
    void (*SetLampState)(void *ctx, uint8_t outputId, SignalColor_t color);
    void (*Flush)(void *ctx);
} ISignalOutputPort_t;

/* Inline dispatch helpers (zero overhead with -O2): */
static inline void SignalOutputSetLamp(ISignalOutputPort_t *p, uint8_t id, SignalColor_t c)
{
    p->SetLampState(p->ctx, id, c);
}
static inline void SignalOutputFlush(ISignalOutputPort_t *p)
{
    p->Flush(p->ctx);
}
```

**Naming convention (port-specific examples — full rules in root `CLAUDE.md` § "Naming Conventions (system-wide)"):**
- Port structs: `IFeaturePort_t`
- Function pointer members: `PascalCase` verbs (`SetLampState`, `Flush`)
- Inline dispatch helpers: `FeatureVerb()` — no underscore separators (`SignalOutputSetLamp`)
- Adapter init/create: `FeatureAdapterInit()`, `FeatureAdapterCreatePort()`
- Context structs: `FeatureAdapterCtx_t`

Domain code only ever calls the inline helpers — no `#ifdef HARDWARE` anywhere.

### Intersection Engine (App/Domain/Intersection/)

NEMA TS2 ring-barrier actuated controller model with full NTCIP 1201/1202 compliance:

- **Phases** (up to 16, NTCIP 1202): Each phase represents an independent traffic movement with its own timing plan. NTCIP timing fields: `phaseMinimumGreen`, `phaseMaximumGreen1`, `phaseMaximumGreen2`, `phasePassage` (vehicle extension), `phaseYellowChange`, `phaseRedClear`, `phaseWalk`, `phasePedestrianClear`.
- **Ring-Barrier Structure**: Dual-ring (Ring 1: phases 1–4, Ring 2: phases 5–8) separated by a barrier at the end of phases 2/6 and 4/8. Both rings must reach the barrier simultaneously before crossing. `phaseOptions` bitmask controls dual-entry, max recall, rest-in-walk, and similar per-phase behaviours.
- **Signal Groups / Channels**: Individual output channels (up to 32) mapped to phase green/yellow/red via `channelControlSource`. Closing duration = `phaseYellowChange`; clearance = `phaseRedClear`.
- **Detector Actuation**: Up to 32 detectors with demand, extension, and occupancy tracking per 100 ms tick. Passage time (`phasePassage`) extends green on each actuation up to `phaseMaximumGreen`. Phase terminates at min green if no demand remains.
- **Coordination**: Cycle length, offset, and yield-point programming for arterial coordination. Split times distribute cycle between phases.
- **Overlaps**: Auxiliary output channels that remain green while any contributing phase is green.
- **Conflict Matrix**: Phase-to-phase conflict map; violation triggers `STATE_ALL_RED` safety fallback.
- **Unit Modes**: Pre-timed, actuated, coordinated, flash, and manual (`unitControl` object).

Top-level coordinator: `ProgramInit(ctx, signalOut, detectors, clock, snmp)` + `ProgramTick(ctx)` called every 100 ms.

### NTCIP Layer (App/Domain/NTCIP/)

Three files — `NTCIP1201.c`, `NTCIP1202.c`, `OidRegistry.c` — all pure domain, no SNMP transport coupling:
- `NTCIP1201.c`: GET/SET functions for global objects — system time, software info, data collection (`Ntcip1201_GetTime`, `Ntcip1201_SetTime`, etc.)
- `NTCIP1202.c`: GET/SET functions for actuated controller objects — phase timing, detector demand/occupancy, alarm table, `unitControl`, `channelControlSource`, coordination objects
- `OidRegistry.c`: Transport-agnostic dispatch table — maps `OidObjectId_t` enum values to NTCIP1201/1202 calls via `OidRegistry_Get()`/`OidRegistry_Set()`

The `LwIPSNMPAdapter` (in `App/Adapters/STM32/`) registers OID subtrees with LwIP's SNMP engine and translates OID paths → `OidObjectId_t`, then forwards to the registry. Domain code is completely unaware of SNMP wire format.

SNMP traps emitted by domain code via `ISNMPNotifierPort` on lamp failures, detector faults, and conflict events.

### FreeRTOS Task Structure

9 thin tasks in `App/Platform/STM32/Tasks/` — all business logic is in `App/Domain/`:

| Task | Priority | Period | What it does |
|---|---|---|---|
| `ProgramTask` | High | 100 ms | `ProgramTick()` |
| `CANRxTask` | High | event | FDCAN RxFIFO → detector/signal adapters |
| `CANTxTask` | AboveNormal | event | Drain CAN Tx queue |
| `NetworkTask` | Normal | event | LwIP, DHCP, TCP reconnect |
| `GPSTask` | BelowNormal | event | UART5 NMEA → GPS adapter |
| `UITask` | Low | event | LCD render + keypad scan |
| `StorageTask` | Low | event | Async EEPROM/Flash queue |
| `TimeTask` | Low | 1 s | RTC + GPS time sync |
| `MaintenanceTask` | Idle | 1 s | Watchdog feed |

### STM32CubeMX Generated Code

`CP.ioc` is the CubeMX project file. Re-generating overwrites code **outside** `/* USER CODE BEGIN */` / `/* USER CODE END */` guards. All HAL/middleware config lives in `Core/`, `LWIP/`, `MBEDTLS/`, `USB_DEVICE/` and is left untouched. The `cmake/STM32CubeMX/CMakeLists.txt` wires all generated sources into the build.

## Coding Standard

- **C11** throughout — no C++ for embedded compatibility
- **No global mutable state** in `App/Domain/` — all state in `Context_t` structs passed by pointer
- **No `malloc`** in `App/Domain/` — all pools are statically-sized arrays
- **Naming** — follow the system-wide rules in the root `CLAUDE.md` § "Naming Conventions (system-wide)":
  `PascalCase_t` types, `camelCase` variables/fields/params, `SCREAMING_SNAKE_CASE` macros, `g_` / `s_` for scope, **no Hungarian prefixes**. CP is **not yet fully migrated**; rename opportunistically as files are touched.
- `-Wall -Wextra -Werror -Wpedantic` enforced for `App/Domain/` and `App/Ports/` on all targets
- Legacy Keil project at `~/workspace/teknotel/C0502-P250514-CPU4/CP` is the reference implementation for domain logic being ported

## Code Formatting

Formatting is enforced with **Uncrustify** using `Tools/Format/MISRA-C.cfg` (compatible with Uncrustify 0.72+).

### Run formatting
```bash
# Via CMake (any preset):
cmake --build build/Host-Test --target Format        # reformat in-place
cmake --build build/Host-Test --target Format-Check  # verify only — exits 1 if any file needs reformatting

# Direct script:
./Tools/Scripts/format-user-code.sh           # reformat in-place
./Tools/Scripts/format-user-code.sh --check   # CI-safe check

# Via Docker (uncrustify is pre-installed in both images):
docker compose -f Tools/Docker/compose.yml run build-test \
    bash -c "cmake --preset Docker-Host-Test && cmake --build /tmp/docker-host-test --target Format"
```

### Scope — user code only

| Formatted | Not formatted (libraries) |
|---|---|
| `App/Domain/`, `App/Ports/`, `App/Adapters/`, `App/Platform/`, `Tests/` | `Middlewares/`, `Drivers/` |
| `Core/Src/`, `Core/Inc/` | `LWIP/`, `MBEDTLS/`, `USB_DEVICE/` |

### MISRA C rules — mandatory for all agents and developers

Always follow these rules when writing or modifying C code in this project. Violations are caught by code review (`/review-pr`) and should be treated as blockers.

| Rule | Requirement |
|---|---|
| **15.6** | Every `if`, `for`, `while`, `do` body **must** use `{ }` braces, even single-statement bodies |
| **16.3** | Every `case` in a `switch` must end with `break` (or `/* fallthrough */` with justification) |
| **16.4** | Every `switch` must have a `default` clause |
| **16.5** | `default` label should appear last in a `switch` |
| **12.1** | Use explicit parentheses to clarify operator precedence in complex expressions |
| **8.1/8.2** | All functions must have explicit prototypes with complete parameter types; `void` for empty parameter lists |
| **14.3** | Null statements (lone `;`) must appear on their own line |
| **17.4** | Use only array indexing (`a[i]`) for pointer arithmetic — never `p + n` or `p++` on arbitrary pointers |

Additional formatting rules enforced by the Uncrustify configuration:

- **No `//` line comments** — the formatter converts them to `/* */` block comments; write `/* */` directly in new code
- **Indentation** — 2 spaces, no tabs
- **Line width** — maximum 80 columns
- **One statement per line** — never combine multiple statements on a single line
- **Braces on new line** — opening `{` goes on its own line (Allman style) for all control structures and function definitions
- **Spaces around operators** — `a = b + c`, not `a=b+c`; no space inside `()` or `[]`
- **Pointer star** — attached to the variable name: `uint8_t *ptr`, not `uint8_t* ptr`

## Vendor MIB Maintenance (1.3.6.1.4.1.59748)

`Docs/TEKNOTEL-CPU4-MIB.mib` (module `TEKNOTEL-CPU4-MIB`, vendor "Teknotel Elektronik", IANA PEN 59748) is the single source-of-truth for every OID under the Teknotel enterprise arc. **Update it in the same commit** as any code change that:

- Adds, removes, or renumbers an OID (scalar, table, or table column)
- Changes the `SYNTAX`, `ACCESS`, or enumerated values of an existing object
- Adds or removes a specific-trap number emitted from `LWIP/App/snmp_client.c`
- Touches the enums in `../Libs/CpMpProtocolShared.h` referenced by the MIB (`CpMpConfigState_t`, `CpMpSafetyAction_t`, `CpMpFaultGlobalFlags_t`, `CpMpFaultChannelFlags_t`)

OID arc shape (mirrors NTCIP 8004/1202): `teknotel .59748 → transportation .4 → devices .2 → cpu4 .1 → { unit .3, channel .8, cpMpLink .20, driverModule .21 }`. `unit` and `channel` reuse NTCIP 1202's asc.* slot numbers; vendor-only groups live at 20+ to stay clear of any future NTCIP assignment.

Authoritative C sources, one file per functional group:
- `App/Domain/NTCIP/MibVendor59748/UnitObjects.c` — `unit` (`.3`)
- `App/Domain/NTCIP/MibVendor59748/ChannelFaultObjects.c` — `channel` (`.8`)
- `App/Domain/NTCIP/MibVendor59748/CpMpLinkObjects.c` — `cpMpLink` (`.20`)
- `App/Domain/NTCIP/MibVendor59748/DriverModuleObjects.c` — `driverModule` (`.21`)
- `App/Adapters/STM32/LWIPSNMPRootMibs.c` — one-stop LwIP subtree registration for every `cpu4.*` group (extend `kTeknotelNodes[]` when adding a new group)
- `LWIP/App/snmp_client.c` — enterprise-specific traps (`.59748.0.{1,2,3}`); trap varbinds point into the arc defined above

Keep the "OID Quick Reference" block at the bottom of the `.mib` in sync with the body. `/review-pr` should block any change to the files above that does not also touch the `.mib`.
