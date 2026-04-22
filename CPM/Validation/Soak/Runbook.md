# Soak Runbook

Run soak only after HIL bring-up, SNMP smoke, and fault injection all pass on
the same firmware set.

## 1. Pre-Run Setup

- Start from nominal service with all modules present.
- Enable recurring detector activity and recurring pedestrian calls.
- Confirm SNMP `v2c` and `v3` both respond before the timer starts.
- Start CAN capture, SNMP logging, and event-log retention before the run.

## 2. Required Runs

### 24 Hour Nominal Run

- Duration: `24 h`
- Purpose: first stability check after an integrated build
- Scheduled actions:
  - detector activity every few minutes
  - pedestrian activity every few minutes
  - one planned power cycle near the middle of the run

### 72 Hour Endurance Run

- Duration: `72 h`
- Purpose: release-candidate endurance qualification
- Scheduled actions:
  - recurring detector and pedestrian activity throughout
  - planned power cycles at fixed intervals
  - at least one SNMP poll cycle every `5 min`

## 3. What to Watch

- CP and MP heartbeat continuity
- module presence for `PSM`, `SSM`, and `IOM`
- detector and pedestrian freshness
- SNMP response time and reachability
- event-log growth
- reset-cause counters
- cabinet temperature and MP battery telemetry

## 4. Fail Conditions

- unexpected reset, watchdog, or crash
- spontaneous `DARK` or `FLASH` without a matching injected cause
- stale-input faults during nominal traffic generation
- growing communication-loss counters without a known bench event
- SNMP becoming unreachable while Ethernet remains physically up

## 5. Evidence to Keep

- run start and stop timestamps
- power-cycle timestamps
- CAN captures or segmented capture windows
- periodic SNMP walk logs
- final controller event-log export
- operator notes for any anomaly
