# Validation Assets

This tree captures the non-firmware evidence needed before a controller build
is called field-ready. It is split by validation intent so lab runs, conformance
captures, endurance runs, and injected-fault campaigns can be versioned with
the firmware that they qualify.

Current subtrees:

- `HIL/`: bench topology, harness expectations, and repeatable hardware-in-loop scenarios.
- `Conformance/`: protocol and MIB behavior checks, including NTCIP 1202 Annex 4 and SNMP profile validation.
- `Soak/`: long-duration endurance runs and the logs required to accept them.
- `FaultInjection/`: scripted loss-of-peer, loss-of-module, bad-auth, stale-input, and configuration-fault cases.

Use this directory for lab procedures, runner scripts, capture templates, and
acceptance reports. Each run should record:

- firmware revision for `CP`, `MP`, `PSM`, `SSM`, and `IOM`
- hardware identifiers and bench wiring revision
- date, operator, and environmental conditions
- exact scenario set executed
- pass/fail outcome and retained logs

Recommended starting points:

- `HIL/BringupRunbook.md`: first integrated bench session from flash to phase service
- `HIL/build_flash_bench.sh`: wrapper for the standard CP/MP/PSM/SSM build and flash order
- `Conformance/Runbook.md`: SNMP and NTCIP conformance sequence
- `Conformance/snmp_smoke.sh`: vendor-tree SNMP smoke test for `v2c` and `v3`
- `FaultInjection/Runbook.md`: deliberate failure campaign
- `Soak/Runbook.md`: `24 h` and `72 h` endurance procedure
- `LabRunTemplate.md`: reusable operator record for every bench session
