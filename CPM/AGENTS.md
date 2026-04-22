# Repository Guidelines

## Scope

**CPM (Controller Processor Module)** is the brain pair of the C0502-P251207-CPU4 intersection controller: a **CP** (Control Processor, STM32H743) supervised by an **MP** (Malfunction Processor, STM32G473), linked by a private CAN FD control bus. This directory groups both module firmwares plus the shared library. Sibling modules `PSM/` and `SSM/` live one level up.

| Subdir | MCU | Role | Module guide |
|---|---|---|---|
| `CP/` | STM32H743VIT6 (Cortex-M7 @ 480 MHz) | NEMA TS2 engine, NTCIP 1201/1202 SNMP, coordinator (hexagonal-arch reference) | `CP/AGENTS.md`, `CP/CLAUDE.md` |
| `MP/` | STM32G473 (Cortex-M4 @ 160 MHz) | Safety/malfunction monitor, conflict watchdog, lamp supervision | `MP/AGENTS.md`, `MP/CLAUDE.md` |
| `Libs/` | — | Shared headers — CP↔MP protocol, intersection config, cross-module ports | (this file) |
| `Docs/` | — | NEMA TS2 2003, ATC 5301 v02.03, schematics | — |

Always consult the per-module guide in `CP/` or `MP/` before editing inside those trees; they cover build presets, FreeRTOS task tables, and MISRA rules in detail.

## Shared Library (`Libs/`)

`Libs/` is the single source of truth for anything that crosses the CP↔MP boundary. Both firmwares `#include` from here; do **not** duplicate these types inside a module.

- **`Libs/CpMpProtocolShared.h`** — CAN FD control-link protocol between CP and MP. Defines frame IDs `CPMP_FRAME_ID_CP_*` (0x100–0x104) and `CPMP_FRAME_ID_MP_*` (0x180–0x186), protocol version (`CPMP_PROTOCOL_VERSION`), heartbeat timeout, config-image transfer (`CpMpMmuConfigImageV1_t`, chunked), fault/safety reporting (`CpMpFaultStatusImageV1_t`, `CpMpSafetyAction_t`). Field-bus PSM/SSM traffic stays on the legacy wire protocol — this header is only for controller-local supervision. Bump `CPMP_PROTOCOL_VERSION` whenever frame layout or enum semantics change, and update both CP and MP parsers in the same commit.
- **`Libs/Intersection/IntersectionConfig.{h,c}`** — canonical persisted intersection configuration (phases, rings, barriers, channels, overlaps, patterns, timebase, preempts, detectors). Sized by `INTERSECTION_*_COUNT_MAX` caps; shared so CP authors the config and MP validates/mirrors it.
- **`Libs/Ports/IUnitClockPort.h`, `IUnitAlarmPort.h`** — port interfaces shared across modules (same C-vtable pattern documented in `CP/CLAUDE.md`).

## Build, Test, and Development Commands

Each module is built from its own directory. Full preset list and Docker flows are in the per-module guides — essentials:

```bash
# CP firmware + host tests
cd CP
cmake --preset STM32-Release && cmake --build --preset STM32-Release
cmake --preset Host-Test     && cmake --build --preset Host-Test && \
  (cd build/Host-Test && ctest --output-on-failure)

# MP firmware + host tests
cd MP
cmake --preset STM32-Release && cmake --build --preset STM32-Release
cmake --preset Host-Test     && cmake --build --preset Host-Test && \
  (cd build/Host-Test && ctest --output-on-failure)
```

Shared toolchain: `arm-none-eabi-gcc` 13.x, CMake 3.22+/Ninja, Uncrustify 0.72+ for `Format` / `Format-Check` targets, Docker compose files under each module's `Tools/Docker/`.

## Cross-Module Change Checklist

When a change touches both modules (most changes to `Libs/` will), do all of the following in **one** PR:

1. Update the shared header(s) in `Libs/`.
2. Rebuild CP **and** MP against the changed header — `Host-Test` preset in each module catches API drift fastest.
3. If frame layout or enum semantics changed, bump `CPMP_PROTOCOL_VERSION` and update both the CP sender and the MP parser.
4. Re-run host unit tests in **both** modules; CI runs them independently and a one-sided change will not be caught locally otherwise.

## Architecture Direction

CP is the reference hexagonal implementation (ports in `CP/App/Ports/`, adapters in `CP/App/Adapters/{STM32,Mock}/`, domain in `CP/App/Domain/` with zero HAL/RTOS/LwIP deps). MP is mid-migration from a monolithic `Core/Src/data.c` model toward the same pattern — when adding new MP code, prefer the port/adapter layout over extending the legacy global state. Shared port contracts go in `Libs/Ports/`, not inside either module.

## Coding Style & Naming Conventions

C11 throughout, no C++. Domain layers (`CP/App/Domain/`, MP's in-progress domain services) must stay free of HAL/RTOS/LwIP. Uncrustify MISRA config in `CP/Tools/Format/MISRA-C.cfg` is the formatting source of truth: 2-space indentation, Allman braces, explicit braces on every control-flow body, pointer star attached to the variable name.

Naming follows the system-wide rules in the root `CLAUDE.md` / `AGENTS.md` § "Naming Conventions": `PascalCase_t` types, `PascalCase_e` enums, `camelCase` variables/fields/params, `SCREAMING_SNAKE_CASE` macros, `g_` / `s_` for scope, **no Hungarian prefixes** (`b` / `s` / `l` / `f` / `p` / `tS` / `tE` / `tp` / `S` struct-instance prefix).

**Migration status**: CP and MP both still contain legacy Hungarian-style identifiers; the shared-library types in `Libs/` (e.g. `CpMpConfigState_t`, `CpMpSafetyAction_t`) already match the new convention and should stay that way. For everything else, rename opportunistically when touching a file — do not open blanket rename PRs that conflict with in-flight feature work.

**Shared-library types that cross the CP/MP link** (anything in `Libs/CpMpProtocolShared.h`) are part of the CAN wire contract: rename only in commits that also bump `CPMP_PROTOCOL_VERSION` and update both sides in lockstep.

## Testing Guidelines

Both CP and MP ship a `Host-Test` CMake preset that compiles Unity-based unit tests on x86-64. CP's `gcovr.cfg` enforces ≥ 50 % line coverage on `App/Domain/` (moving to 80 %); MP follows the same model as its domain layer grows.

Naming: `Tests/Unit/Test_<Module>.c`, each file with its own `main()` and explicit `RUN_TEST(...)` list. Integration tests (CP only, in `Tests/Integration/`) exercise end-to-end phase cycles and conflict safety scenarios against mock adapters.

## Commit & Pull Request Guidelines

- Short, imperative subjects (e.g. `bump CPMP protocol version`, `add ntcip mibs`).
- PRs that touch `Libs/` must say so explicitly and confirm both CP and MP were rebuilt + tested locally.
- Keep `.mib` updates in the **same commit** as the code change that motivated them — see below.

## Vendor MIB Maintenance (1.3.6.1.4.1.59748)

`CP/Docs/TEKNOTEL-CPU4-MIB.mib` (module `TEKNOTEL-CPU4-MIB`, vendor "Teknotel Elektronik", IANA PEN 59748) is the single source-of-truth for every OID under the Teknotel enterprise arc. Update it in the **same commit** as any change that:

- Adds, removes, or renumbers an OID
- Changes the `SYNTAX` / `ACCESS` / enumerated values of an existing object
- Adds or removes a specific-trap number emitted from `CP/LWIP/App/snmp_client.c`
- Touches the enums in `Libs/CpMpProtocolShared.h` that the MIB references (`CpMpConfigState_t`, `CpMpSafetyAction_t`, `CpMpFaultGlobalFlags_t`, `CpMpFaultChannelFlags_t`) — these describe the bit layout of `cpMpLinkGlobalFlags` and per-row `channelFaultFlags`, so a protocol-version bump must be mirrored in the MIB

OID arc shape (mirrors NTCIP 8004/1202): `teknotel .59748 → transportation .4 → devices .2 → cpu4 .1 → { unit .3, channel .8, cpMpLink .20, driverModule .21 }`.

Authoritative per-group C sources in `CP/App/Domain/NTCIP/MibVendor59748/` (`UnitObjects.c`, `ChannelFaultObjects.c`, `CpMpLinkObjects.c`, `DriverModuleObjects.c`). One-stop LwIP subtree registration in `CP/App/Adapters/STM32/LWIPSNMPRootMibs.c`. Enterprise traps in `CP/LWIP/App/snmp_client.c`. Any change to these files that is not accompanied by a matching `.mib` update should fail code review.
