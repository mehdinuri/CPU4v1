# Fault Injection Runbook

Run these cases only after the HIL bring-up pass and nominal engine service are
stable.

## Common Rules

- Inject one fault at a time.
- Record start time, trigger method, observed reaction time, and recovery time.
- Retain the matching CAN capture, SNMP log, and CP event log export for every
  case.
- Reset the bench to nominal service between cases.

## Case Matrix

| Case | Trigger | Expected protective action | Expected visible result | Recovery |
| --- | --- | --- | --- | --- |
| CP missing | stop CP heartbeats or power down CP only | MP goes `DARK`, relay opens | `cpMpLinkPeerHealthy` degrades, safety action changes, CP-side log after return | restore CP power and heartbeat |
| PSM missing | remove `PSM` from field bus | MP goes `DARK`, relay opens | `driverModuleStatus` or module health changes, CP logs module loss | reconnect `PSM` |
| SSM missing | remove `SSM` from field bus | MP goes `DARK`, relay opens | `driverModuleStatus` changes, CP logs module loss | reconnect `SSM` |
| MP config invalid | load an invalid MP image or invalid mapping | MP goes `DARK`, relay opens | `cpMpLinkConfigState` invalid, safety action dark | restore valid config |
| MP battery low | force battery below `2800 mV` | report-only MP self-health event | MP event count increments, telemetry shows low battery | restore above `3000 mV` |
| MP temperature high | force temperature above `40 C` | report-only MP self-health event | MP event count increments, telemetry shows high temperature | cool below `38 C` |
| IOM stale | stop `0x080` and `0x081` traffic | no cabinet-wide protective action | CP pedestrian communications-fault path shows stale input behavior | restore `IOM` traffic |
| FEIG offline | disconnect FEIG detector source | no cabinet-wide protective action | detector communications alarms appear | reconnect FEIG source |
| CP↔MP link degraded | interrupt private control bus | safety follows cause and MP reports degraded link | `cpMpLink` status changes and event counters advance | restore control bus |
| SNMP bad authentication | repeated bad `v2c` and bad `v3` credentials | no protective action | authentication failures are logged and access remains denied | reconnect with valid credentials |

## Minimum Acceptance

- No fault causes a weaker action than defined above.
- No fault latches after its recovery condition is cleared unless the design
  explicitly requires a power cycle.
- No unrelated module drops out during a single injected fault.
- Every case leaves enough evidence to reconstruct trigger, reaction, and
  recovery.
