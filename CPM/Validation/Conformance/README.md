# Conformance

This subtree holds protocol-behavior evidence.

Primary assets in this directory:

- `Runbook.md`: manual conformance sequence and evidence expectations
- `snmp_smoke.sh`: scripted vendor-tree smoke test

Required conformance packs:

- `NTCIP 1202 Annex 4`
  - GET succeeds on every implemented mandatory object
  - out-of-range index and value handling returns the documented error class
  - writable objects require a valid database transaction where specified
  - verify/commit persists and rollback restores the active configuration
  - remote write lock blocks all writes except the unlock path
- `SNMP profile`
  - `Release`: `v1` disabled, `v2c + v3` enabled
  - `Release`: security-bearing objects blocked over `v1/v2c`
  - `Release`: legacy default communities rejected
  - `Debug` and `Host-Test`: deterministic lab communities remain available
- `Vendor MIB`
  - OID walk matches `TEKNOTEL-CPU4-MIB.mib`
  - `unitSnmpV3*` objects enforce staging/apply semantics
  - `cpMpLink` and `eventSource` payloads decode correctly

Expected retained evidence:

- MIB walk capture for standard and vendor subtrees
- SET transaction transcript for at least one writable 1202 object
- negative test transcript for blocked security writes over `v2c`
- summary report naming firmware and MIB revisions under test
