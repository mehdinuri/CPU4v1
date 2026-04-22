# Soak

Soak testing looks for slow faults, state leaks, stale communications, and
deferred timing drift under realistic field activity.

Primary asset in this directory:

- `Runbook.md`: `24 h` and `72 h` endurance procedure

Recommended runs:

- `24 h` nominal operation with recurring detector and pedestrian activity
- `72 h` endurance run before first field release candidate
- power-cycle campaign during the run at scheduled intervals

Monitor continuously:

- CP/MP heartbeat continuity
- module presence for `PSM`, `SSM`, and `IOM`
- detector and pedestrian input freshness
- SNMP responsiveness over both `v2c` and `v3`
- event-log growth and reset-cause counters
- cabinet temperature and battery telemetry

Acceptance criteria:

- no unexpected reset or watchdog event
- no latched safety action without the corresponding injected cause
- no growth in communication-error counters beyond the injected scenario plan
- no stale-input alarms under nominal traffic generation
