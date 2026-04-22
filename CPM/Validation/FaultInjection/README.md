# Fault Injection

Fault-injection assets define the deliberate failures that must be exercised
before a build is considered field-safe.

Primary asset in this directory:

- `Runbook.md`: ordered case list with expected protection and recovery

Priority cases:

- stop CP heartbeats and confirm MP goes `DARK` with relay open
- remove `PSM` and confirm MP goes `DARK` with relay open
- remove `SSM` and confirm MP goes `DARK` with relay open
- deliver invalid MP configuration and confirm `MP config invalid` drives `DARK`
- force MP battery below `2800 mV`, then recover above `3000 mV`
- force MP temperature above `40 C`, then recover below `38 C`
- stop `IOM` `0x080/0x081` updates and confirm CP logs a stale/missing IOM fault
- take FEIG detector input offline and confirm detector communication alarms
- perform repeated bad `v2c` and `v3` authentication attempts and confirm event logging
- interrupt CP<->MP link traffic and confirm link degraded/restored reporting

Each case should define:

- trigger method
- expected protective action
- expected CP log entry and SNMP-visible status
- recovery method
- pass/fail timing window
