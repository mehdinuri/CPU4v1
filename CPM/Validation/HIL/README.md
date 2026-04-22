# HIL

Hardware-in-loop validation proves the integrated controller stack against real
or emulated field wiring.

Primary assets in this directory:

- `BringupRunbook.md`: first bench session from flashing to nominal service
- `build_flash_bench.sh`: helper wrapper for the standard CP/MP/PSM/SSM order

Minimum bench content:

- `CP + MP + PSM + SSM + IOM` populated on the same CAN/FDCAN topology used in production
- signal load simulation for every driven channel, including conflict-pair observation
- FEIG `VEKM4D` detector source or detector emulator
- pedestrian-input source into `IOM` `0x080` / `0x081`
- relay feedback, cabinet-door input, and DC rail observation
- SNMP management station capable of `v2c` and `v3`

Baseline scenarios:

- startup to normal operation with valid configuration load into MP
- phase service with detector calls and pedestrian calls active
- CP restart while MP, PSM, SSM, and IOM remain powered
- local dark action for `CP missing`, `PSM missing`, `SSM missing`, and `MP config invalid`
- MP battery and temperature telemetry crossing trip and clear thresholds

Artifacts to retain per run:

- bench wiring/topology diagram
- CAN capture for the scenario window
- SNMP walk/set logs
- controller event log export
- operator notes for any manual intervention
