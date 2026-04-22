# Local Admin Protocol V1 Plan

## Summary

Replace the legacy ASCII `UITask` protocol with a transport-neutral local admin channel for RS232 and USB. Reuse the same shared CP services and canonical NTCIP object layer already used by the LCD and MMI paths. Keep SNMP as the IP management interface only.

## Decisions

- Do not use SNMP over RS232 or USB.
- Do not use protobuf for v1.
- Use deterministic CBOR payloads inside a framed binary envelope.
- Use COBS framing with `0x00` delimiter on both RS232 and USB CDC.
- Keep USB-stick import/export separate from the live admin RPC framing.

## Transport Shape

- Replace the current ASCII packet parser with raw RX/TX transport tasks only.
- Keep:
  - RX queue and RX memory pool
  - TX queue and TX memory pool
  - parser/dispatch task shell
  - sender task shell
- Remove:
  - all `UI_COMM_PACKET_*` packet tables
  - ASCII framing/checksum logic
  - upload/download packet-specific parser behavior
  - old request/response text protocol semantics

## Framed Message Format

- Fixed binary header:
  - magic
  - version
  - message type
  - flags
  - request id
  - session id
  - payload length
- Deterministic CBOR payload
- CRC32 over header and payload
- COBS frame delimiter `0x00`

## Protocol Verbs

- `hello`
- `auth.login`
- `auth.logout`
- `resource.get`
- `resource.subscribe`
- `resource.unsubscribe`
- `object.begin_txn`
- `object.get`
- `object.set`
- `object.verify`
- `object.commit`
- `object.rollback`
- `event.get_page`
- `command.invoke`
- `bundle.import_start`
- `bundle.import_chunk`
- `bundle.import_verify`
- `bundle.activate`
- `bundle.export_info`
- `bundle.export_chunk`

## Resource Model

- `runtime`
  - summary
  - rings, phases, channels, overlaps
  - vehicle detectors
  - pedestrian detectors
  - safety summary and safety channels
  - power
  - comms identity and state
  - relay state
  - output-test state
  - clock
- `standard_object`
  - canonical NTCIP object-directory reads and writes
  - DB transaction flow identical to SNMP
- `local_settings`
  - modem type
  - GPS port + baud as one resource
  - unified user flags
  - auth actions
- `events`
  - cursor and page based event-log access
- `commands`
  - controller mode changes
  - relay request
  - output test control
  - detector reset
  - reboot and factory reset if retained

## Auth

- CP is the only auth source.
- Reuse the shared CP auth/session service.
- Guest may read logs, time, and runtime.
- Admin is required for config writes, mutating commands, relay control, and output test.

## Config Import / Export

- Small standards-backed edits continue through object transactions.
- Full engine configuration uses a canonical config image, not packet soup.
- Large serial/USB transfers use bundle import/export verbs.
- USB-stick import/export should use the same validation and activation path as streamed config import.

## Cleanup Targets

- Remove active UI dependencies on:
  - `data.h`
  - `program.h`
  - `gps.h`
  - `MCSAsynch.h`
- Keep `MCSTask` only behind clean comms/identity services.
- Treat USB CDC as the default future PC-admin transport, but do not change the USB class in the cleanup slice.

## Test Plan

- COBS framing and CRC failure handling
- identical RS232 and USB decode behavior
- RX/TX queue and pool exhaustion handling
- auth lifecycle
- request/response correlation
- subscribe/unsubscribe behavior
- NTCIP object transaction flow
- config import verify / activate / rollback
- event paging and latest door-log navigation
