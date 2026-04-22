# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Scope

**CPM (Controller Processor Module)** is the brain pair of the C0502-P251207-CPU4 intersection controller: a **CP** (Control Processor) supervised by an **MP** (Malfunction Processor), linked by a private CAN FD control bus. Everything inside this directory belongs to that subsystem; the sibling `PSM/` and `SSM/` trees live one level up.

| Subdir | MCU | Role | Module CLAUDE.md |
|---|---|---|---|
| `CP/` | STM32H743VIT6 (Cortex-M7 @ 480 MHz) | NEMA TS2 engine, NTCIP 1201/1202 SNMP, coordinator (hexagonal arch reference) | `CP/CLAUDE.md` |
| `MP/` | STM32G473 (Cortex-M4 @ 160 MHz) | Safety/malfunction monitor, conflict watchdog, lamp supervision | `MP/CLAUDE.md` |
| `Libs/` | — | Shared headers — CP↔MP protocol, intersection config, cross-module ports | (this file) |
| `Docs/` | — | NEMA TS2 2003, ATC 5301 v02.03, schematic PDFs | — |

Always consult the per-module CLAUDE.md in `CP/` or `MP/` before editing inside those trees; they cover build presets, FreeRTOS task tables, coding standards, and MISRA rules.

## Shared Library (`Libs/`)

`Libs/` is the single source of truth for anything that crosses the CP↔MP boundary. Both firmwares `#include` from here; do **not** duplicate these types inside a module.

- **`Libs/CpMpProtocolShared.h`** — CAN FD control-link protocol between CP and MP. Defines:
  - Frame IDs `CPMP_FRAME_ID_CP_*` (0x100–0x104) and `CPMP_FRAME_ID_MP_*` (0x180–0x186)
  - Protocol version (`CPMP_PROTOCOL_VERSION`), heartbeat peer-timeout (`CPMP_PEER_TIMEOUT_TICKS`)
  - Config-image transfer (`CpMpMmuConfigImageV1_t`, chunked over `CFG_BEGIN`/`CFG_CHUNK`/`CFG_COMMIT`)
  - Fault/safety reporting (`CpMpFaultStatusImageV1_t`, `CpMpSafetyAction_t`, fault-flag enums)
  - Field-bus PSM/SSM traffic stays on the legacy wire protocol — **this header is only for the controller-local supervision link.**
  - Bump `CPMP_PROTOCOL_VERSION` whenever frame layout or enum semantics change, and update both `CP/` and `MP/` parsers in the same commit.
- **`Libs/Intersection/IntersectionConfig.{h,c}`** — canonical persisted intersection configuration (phases, rings, barriers, channels, overlaps, patterns, timebase, preempts, detectors). Sized by `INTERSECTION_*_COUNT_MAX` caps; shared so that CP authors the config and MP validates/mirrors it from the same struct.
- **`Libs/Ports/IUnitClockPort.h`, `IUnitAlarmPort.h`** — port interfaces shared across modules (same C-vtable pattern documented in `CP/CLAUDE.md`).

## Build Quick Reference

Each module is built independently from its own directory. Full preset list and Docker flows are in the module CLAUDE.md files — the essentials:

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

Both modules share: `arm-none-eabi-gcc` 13.x toolchain, CMake 3.22+/Ninja, Uncrustify 0.72+ for `Format`/`Format-Check`, and Docker compose files under each module's `Tools/Docker/`.

## Cross-Module Change Checklist

When a change touches both modules (most changes to `Libs/` will), do all of the following in one PR:

1. Update the shared header(s) in `Libs/`.
2. Rebuild CP **and** MP against the changed header (`Host-Test` preset in each module catches API drift fastest).
3. If frame layout or enum semantics changed, bump `CPMP_PROTOCOL_VERSION` and update both the CP sender and MP parser — and vice versa.
4. Re-run host unit tests in **both** modules; CI runs them independently and a one-sided change will not be caught locally otherwise.

## Hexagonal Architecture Direction

CP is the reference hexagonal implementation (ports in `CP/App/Ports/`, adapters in `CP/App/Adapters/{STM32,Mock}/`, domain in `CP/App/Domain/` with zero HAL/RTOS/LwIP deps). MP is mid-migration from a monolithic `Core/Src/data.c` model toward the same pattern — when adding new MP code, prefer the port/adapter layout over extending the legacy global state. Shared port contracts go in `Libs/Ports/`, not inside either module.

## Naming Conventions

Follow the system-wide rules in the root `CLAUDE.md` § "Naming Conventions (system-wide)" for everything new in `Libs/`, `CP/`, and `MP/`:
`PascalCase_t` types, `PascalCase_e` enums, `camelCase` variables/fields/params, `SCREAMING_SNAKE_CASE` macros, `g_` / `s_` for scope, **no Hungarian prefixes** (`b` / `s` / `l` / `f` / `p` / `tS` / `tE` / `tp`).

**Migration status**: CP and MP both still contain legacy Hungarian-style identifiers; the shared-library types in `Libs/` (e.g. `CpMpConfigState_t`, `CpMpSafetyAction_t`) already match the new convention and should stay that way. For everything else, rename opportunistically when touching a file — do not open blanket rename PRs that conflict with in-flight feature work.

**Shared-library types that cross the CP/MP link** (e.g. anything in `Libs/CpMpProtocolShared.h`) are part of the CAN wire contract: rename only in commits that also bump `CPMP_PROTOCOL_VERSION` and update both sides in lockstep.

## Vendor MIB Maintenance (1.3.6.1.4.1.59748)

`CP/Docs/TEKNOTEL-CPU4-MIB.mib` (module `TEKNOTEL-CPU4-MIB`, vendor "Teknotel Elektronik") is the single source-of-truth for every OID under the Teknotel enterprise arc. **Update it in the same commit** as any change that:

- Adds, removes, or renumbers an OID (scalar, table, or table column)
- Changes the `SYNTAX`, `ACCESS`, or enumerated values of an existing object
- Adds or removes a specific-trap number emitted from `CP/LWIP/App/snmp_client.c`
- Touches the enums in `Libs/CpMpProtocolShared.h` that the MIB references (`CpMpConfigState_t`, `CpMpSafetyAction_t`, `CpMpFaultGlobalFlags_t`, `CpMpFaultChannelFlags_t`) — these describe the bit layout of `cpMpLinkGlobalFlags` and the per-row `channelFaultFlags`, so a protocol-version bump must be mirrored in the MIB

OID arc shape (mirrors NTCIP 8004/1202): `teknotel .59748 → transportation .4 → devices .2 → cpu4 .1 → { unit .3, channel .8, cpMpLink .20, driverModule .21 }`.

Authoritative C sources, one file per functional group:
- `CP/App/Domain/NTCIP/MibVendor59748/UnitObjects.c` — `unit` (`.3`)
- `CP/App/Domain/NTCIP/MibVendor59748/ChannelFaultObjects.c` — `channel` (`.8`)
- `CP/App/Domain/NTCIP/MibVendor59748/CpMpLinkObjects.c` — `cpMpLink` (`.20`)
- `CP/App/Domain/NTCIP/MibVendor59748/DriverModuleObjects.c` — `driverModule` (`.21`)
- `CP/App/Adapters/STM32/LWIPSNMPRootMibs.c` — one-stop LwIP subtree registration (add a new `LWIP_SNMP_CREATE_DELEGATING_LEAF` node and append it to `kTeknotelNodes[]` when introducing a new top-level group under `cpu4`)
- `CP/LWIP/App/snmp_client.c` — enterprise-specific traps (`.59748.0.{1,2,3}`)

Any change to these files that is not accompanied by a matching `.mib` update should fail code review.
