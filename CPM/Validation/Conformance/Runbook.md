# Conformance Runbook

This runbook captures the minimum SNMP and NTCIP conformance work needed after
the HIL bring-up pass is stable.

## 1. Tools

- `snmpget`, `snmpwalk`, and `snmpset` from Net-SNMP on the manager laptop
- Teknotel vendor MIB loaded from `CP/Docs/TEKNOTEL-CPU4-MIB.mib`
- NTCIP `1201`, `1202`, and `1103` MIBs loaded into the MIB browser used for
  the standard-object transaction tests

## 2. Vendor SNMP Smoke

Run the scripted vendor smoke test first:

```bash
./Validation/Conformance/snmp_smoke.sh --ip <cp-ip> --mode debug-v2c
./Validation/Conformance/snmp_smoke.sh --ip <cp-ip> --mode debug-v3
```

For the release rerun, supply the commissioned credentials:

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

Accept this stage only if:

- the vendor subtree walk succeeds
- `cpMpLinkPeerHealthy` is healthy
- `cpMpLinkConfigState` is applied
- release `v2c` cannot access the protected security paths checked by the
  script

## 3. Standard NTCIP 1202 Transaction

Use a MIB browser with the standard MIBs loaded. Run the same transaction flow
covered by `CP/Tests/Integration/Test_NTCIP1202Annex4.c`:

1. Select one safe phase row that is not actively serving traffic.
2. Open a database transaction.
3. Change `phaseMinimumGreen`.
4. Change the paired `phaseMaximumInitial` value so verify can succeed.
5. Run verify and confirm the status completes with no error.
6. Commit and read both values back from the active configuration.
7. Open a new transaction, change one writable value again, then rollback.
8. Confirm the active configuration remains at the committed values.

Retain the full set and get transcript.

## 4. Remote Write Lock

Still in the MIB browser:

1. Enable the remote write lock path.
2. Confirm ordinary reads still succeed.
3. Attempt one standard writable object update and confirm it is blocked.
4. Unlock using the documented unlock path.
5. Confirm writes succeed again.

## 5. Vendor SNMPv3 Rotation

Perform this only over `v3`, never over `v2c`.

The vendor objects are:

- `unitSnmpV3ActiveUsername.0`
- `unitSnmpV3NewAuthPassphrase.0`
- `unitSnmpV3NewPrivPassphrase.0`
- `unitSnmpV3Apply.0`

Recommended sequence:

1. Read `unitSnmpV3ActiveUsername.0`.
2. Set the staged username if it must change.
3. Set the new auth passphrase.
4. Set the new priv passphrase. If you want the same value for both, use the
   same text for both writes.
5. Set `unitSnmpV3Apply.0 = 1`.
6. Reconnect using the new credentials.

Retain the exact set transcript and the post-apply successful reconnect.

## 6. Traps

Capture at least these on the manager host:

- coldstart
- door open
- driver module missing

Record the destination IP, trap version, and receive timestamp for each.
