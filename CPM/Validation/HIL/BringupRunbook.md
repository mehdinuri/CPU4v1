# HIL Bring-Up Runbook

This runbook is the first integrated bench session for `CP`, `MP`, `PSM`,
`SSM`, and `IOM`.

## 1. Prepare the Bench

- Use simulated signal loads or a lamp load bank, not live field heads.
- Connect `CP + MP + PSM + SSM + IOM` on the production CAN or FDCAN wiring.
- Connect FEIG `VEKM4D` detector source or detector emulator.
- Connect pedestrian inputs into `IOM`; the module should publish two CAN
  frames on `0x080` and `0x081`.
- Observe relay feedback, cabinet door input, DC rails, and private `CP↔MP`
  CAN traffic.
- Put the SNMP manager laptop on `192.168.10.226/24`. The current CP default
  trap destination points there.

## 2. Build and Flash the Bench

Use the helper wrapper from the `CPM` root:

```bash
./Validation/HIL/build_flash_bench.sh build-flash
```

Default behavior is:

- `CP`: `STM32-Debug`
- `MP`: `STM32-Release`
- `PSM`: `STM32-Release`
- `SSM`: `STM32-Release`

The wrapper intentionally leaves `IOM` manual because it is still the legacy
Keil project in `../IOM/project/iom.uvproj`.

After the debug bench pass is clean, rerun only the CP release subset:

```bash
./Validation/HIL/build_flash_bench.sh build-flash --cp-preset STM32-Release
```

## 3. Power-Up Acceptance

- Apply power with only simulated loads connected.
- Confirm the controller does not enter unexpected `DARK` or `FLASH`.
- Confirm private `CP↔MP` traffic is present:
  - CP frames `0x100` to `0x104`
  - MP frames `0x180` to `0x186`
- Confirm `IOM` is publishing `0x080` and `0x081`.

## 4. Discover the CP Ethernet Address

- Prefer the local UI or diagnostics page first.
- If needed, scan `192.168.10.0/24`; the CP defaults to a static address in
  that subnet.
- Record the discovered IP in the lab run record before any SNMP work.

## 5. Bring Up SNMP on the Debug Pass

Debug credentials are deterministic:

- `v2c` read community: `public`
- `v2c` write community: `private`
- `v2c` trap community: `reports`
- `v3` username: `maester`
- `v3` auth and priv passphrase: `maester-debug-key`

Run the smoke checks:

```bash
./Validation/Conformance/snmp_smoke.sh --ip <cp-ip> --mode debug-v2c
./Validation/Conformance/snmp_smoke.sh --ip <cp-ip> --mode debug-v3
```

Accept this step only if `cpMpLinkPeerHealthy`, `cpMpLinkAuthorityReady`, and
`cpMpLinkConfigState` all show the normal applied state.

## 6. Validate the Engine

Start with a minimal known-good `4-phase / 2-ring` NEMA plan, then move to the
full cabinet plan.

Verify in order:

- startup behavior reaches normal service
- minimum green timing
- yellow and red clearance timing
- legal concurrent phase service only
- barrier crossing between rings
- gap-out and max-out behavior
- no conflict pair is energized together

Retain a CAN capture for the full startup-to-service window.

## 7. Validate Detectors and Pedestrian Inputs

- Drive one FEIG detector input at a time and confirm the intended phase call
  and service.
- Drive one `IOM` pedestrian input at a time and confirm the intended ped call
  and ped clearance behavior.
- Retain the matching `0x080` and `0x081` CAN capture while each ped input is
  actuated.

## 8. Validate Integrated Hardware

- Confirm `driverModuleStatus` stays healthy in nominal service.
- Confirm every `SSM` output drives the expected simulated load.
- Confirm relay feedback matches the safe state.
- Confirm door open and door close events are visible through the event path.
- Confirm MP battery and temperature telemetry remain nominal during the
  baseline run.

## 9. Run the Release SNMP Subset

After the debug pass is clean, reflash only the CP to `STM32-Release` and
rerun:

- `v2c` operational read access
- `v3` operational read access
- blocked `v2c` access to security-bearing objects
- vendor subtree walk
- trap reception

Use:

```bash
./Validation/Conformance/snmp_smoke.sh \
  --ip <cp-ip> \
  --mode release-v2c \
  --read-community <read-community> \
  --write-community <write-community>

./Validation/Conformance/snmp_smoke.sh \
  --ip <cp-ip> \
  --mode release-v3 \
  --username <username> \
  --auth-pass <auth-pass> \
  --priv-pass <priv-pass>
```

If the release unit does not yet have known commissioned credentials, stop the
release SNMP qualification and record commissioning as the blocker.

## 10. Hand Off to the Next Campaigns

Run these after bring-up passes:

- `Validation/Conformance/Runbook.md`
- `Validation/FaultInjection/Runbook.md`
- `Validation/Soak/Runbook.md`

Retain all evidence listed in `Validation/LabRunTemplate.md`.
