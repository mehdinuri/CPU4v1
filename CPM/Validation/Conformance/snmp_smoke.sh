#!/usr/bin/env bash
set -euo pipefail

usage()
{
  cat <<'EOF'
Usage:
  snmp_smoke.sh --ip <cp-ip> --mode <mode> [options]

Modes:
  debug-v2c
  debug-v3
  release-v2c
  release-v3

Options:
  --read-community <value>  Required for release-v2c
  --write-community <value> Required for release-v2c
  --username <value>        Required for release-v3
  --auth-pass <value>       Required for release-v3
  --priv-pass <value>       Required for release-v3; defaults to auth pass
  --auth-proto <value>      Default: SHA
  --priv-proto <value>      Default: AES
  --timeout <seconds>       Default: 2
  --retries <count>         Default: 1
  --help                    Show this message
EOF
}

require_cmd()
{
  if ! command -v "$1" >/dev/null 2>&1; then
    printf 'Missing required command: %s\n' "$1" >&2
    exit 1
  fi
}

run_get()
{
  local label="$1"
  local oid="$2"

  printf '==> GET %s\n' "$label"
  "${SNMP_GET_CMD[@]}" "$TARGET_IP" "$oid"
}

run_walk()
{
  local label="$1"
  local oid="$2"

  printf '==> WALK %s\n' "$label"
  "${SNMP_WALK_CMD[@]}" "$TARGET_IP" "$oid"
}

expect_failure()
{
  local label="$1"
  shift

  printf '==> EXPECT FAIL %s\n' "$label"
  if "$@"; then
    printf 'Unexpected success: %s\n' "$label" >&2
    exit 1
  fi
}

TARGET_IP=""
MODE=""
READ_COMMUNITY=""
WRITE_COMMUNITY=""
USERNAME=""
AUTH_PASS=""
PRIV_PASS=""
AUTH_PROTO="SHA"
PRIV_PROTO="AES"
TIMEOUT="2"
RETRIES="1"

while [[ $# -gt 0 ]]; do
  case "$1" in
    --ip)
      TARGET_IP="${2:?missing value for --ip}"
      shift 2
      ;;
    --mode)
      MODE="${2:?missing value for --mode}"
      shift 2
      ;;
    --read-community)
      READ_COMMUNITY="${2:?missing value for --read-community}"
      shift 2
      ;;
    --write-community)
      WRITE_COMMUNITY="${2:?missing value for --write-community}"
      shift 2
      ;;
    --username)
      USERNAME="${2:?missing value for --username}"
      shift 2
      ;;
    --auth-pass)
      AUTH_PASS="${2:?missing value for --auth-pass}"
      shift 2
      ;;
    --priv-pass)
      PRIV_PASS="${2:?missing value for --priv-pass}"
      shift 2
      ;;
    --auth-proto)
      AUTH_PROTO="${2:?missing value for --auth-proto}"
      shift 2
      ;;
    --priv-proto)
      PRIV_PROTO="${2:?missing value for --priv-proto}"
      shift 2
      ;;
    --timeout)
      TIMEOUT="${2:?missing value for --timeout}"
      shift 2
      ;;
    --retries)
      RETRIES="${2:?missing value for --retries}"
      shift 2
      ;;
    --help)
      usage
      exit 0
      ;;
    *)
      printf 'Unknown option: %s\n' "$1" >&2
      usage
      exit 1
      ;;
  esac
done

if [[ -z "$TARGET_IP" || -z "$MODE" ]]; then
  usage
  exit 1
fi

case "$MODE" in
  debug-v2c)
    READ_COMMUNITY="public"
    WRITE_COMMUNITY="private"
    ;;
  debug-v3)
    USERNAME="maester"
    AUTH_PASS="maester-debug-key"
    PRIV_PASS="maester-debug-key"
    ;;
  release-v2c)
    if [[ -z "$READ_COMMUNITY" || -z "$WRITE_COMMUNITY" ]]; then
      printf 'release-v2c requires --read-community and --write-community\n' >&2
      exit 1
    fi
    ;;
  release-v3)
    if [[ -z "$USERNAME" || -z "$AUTH_PASS" ]]; then
      printf 'release-v3 requires --username and --auth-pass\n' >&2
      exit 1
    fi
    ;;
  *)
    printf 'Unknown mode: %s\n' "$MODE" >&2
    usage
    exit 1
    ;;
esac

if [[ -z "$PRIV_PASS" && -n "$AUTH_PASS" ]]; then
  PRIV_PASS="$AUTH_PASS"
fi

require_cmd snmpget
require_cmd snmpwalk
require_cmd snmpset

BASE_GET_OPTS=(-OQ -On -r "$RETRIES" -t "$TIMEOUT")
BASE_WALK_OPTS=(-OQ -On -r "$RETRIES" -t "$TIMEOUT")

if [[ "$MODE" == *v2c ]]; then
  SNMP_GET_CMD=(snmpget "${BASE_GET_OPTS[@]}" -v2c -c "$READ_COMMUNITY")
  SNMP_WALK_CMD=(snmpwalk "${BASE_WALK_OPTS[@]}" -v2c -c "$READ_COMMUNITY")
  SNMP_SET_CMD=(snmpset "${BASE_GET_OPTS[@]}" -v2c -c "$WRITE_COMMUNITY")
else
  SNMP_GET_CMD=(snmpget "${BASE_GET_OPTS[@]}" -v3 -l authPriv -u "$USERNAME" -a "$AUTH_PROTO" -A "$AUTH_PASS" -x "$PRIV_PROTO" -X "$PRIV_PASS")
  SNMP_WALK_CMD=(snmpwalk "${BASE_WALK_OPTS[@]}" -v3 -l authPriv -u "$USERNAME" -a "$AUTH_PROTO" -A "$AUTH_PASS" -x "$PRIV_PROTO" -X "$PRIV_PASS")
  SNMP_SET_CMD=(snmpset "${BASE_GET_OPTS[@]}" -v3 -l authPriv -u "$USERNAME" -a "$AUTH_PROTO" -A "$AUTH_PASS" -x "$PRIV_PROTO" -X "$PRIV_PASS")
fi

UNIT_FAILURE_FLASH_PERIOD_DS=".1.3.6.1.4.1.59748.4.2.1.3.1.0"
UNIT_SNMPV3_ACTIVE_USERNAME=".1.3.6.1.4.1.59748.4.2.1.3.2.0"
UNIT_SNMPV3_APPLY=".1.3.6.1.4.1.59748.4.2.1.3.5.0"
CPMP_PROTOCOL_VERSION=".1.3.6.1.4.1.59748.4.2.1.20.1.0"
CPMP_PEER_HEALTHY=".1.3.6.1.4.1.59748.4.2.1.20.2.0"
CPMP_AUTHORITY_READY=".1.3.6.1.4.1.59748.4.2.1.20.3.0"
CPMP_CONFIG_STATE=".1.3.6.1.4.1.59748.4.2.1.20.4.0"
CPMP_SAFETY_ACTION=".1.3.6.1.4.1.59748.4.2.1.20.5.0"
CPMP_STATUS_SEQUENCE=".1.3.6.1.4.1.59748.4.2.1.20.7.0"
CPMP_GLOBAL_FLAGS=".1.3.6.1.4.1.59748.4.2.1.20.8.0"
DRIVER_MODULE_STATUS=".1.3.6.1.4.1.59748.4.2.1.21.1.0"
EVENT_SOURCE_POWER_ON_COUNT=".1.3.6.1.4.1.59748.4.2.1.22.1.0"
EVENT_SOURCE_RESET_CAUSE=".1.3.6.1.4.1.59748.4.2.1.22.2.0"
EVENT_SOURCE_MP_EVENT_COUNT=".1.3.6.1.4.1.59748.4.2.1.22.8.0"
EVENT_SOURCE_MP_EVENT_DATA=".1.3.6.1.4.1.59748.4.2.1.22.9.0"
TEKNOTEL_VENDOR_TREE=".1.3.6.1.4.1.59748.4.2.1"
COMMUNITY_NAME_USER_ROW1=".1.3.6.1.4.1.1206.4.2.6.5.3.1.2.1"

printf 'Target IP: %s\n' "$TARGET_IP"
printf 'Mode: %s\n' "$MODE"

run_get "unitFailureFlashPeriodDs" "$UNIT_FAILURE_FLASH_PERIOD_DS"
run_get "cpMpLinkProtocolVersion" "$CPMP_PROTOCOL_VERSION"
run_get "cpMpLinkPeerHealthy" "$CPMP_PEER_HEALTHY"
run_get "cpMpLinkAuthorityReady" "$CPMP_AUTHORITY_READY"
run_get "cpMpLinkConfigState" "$CPMP_CONFIG_STATE"
run_get "cpMpLinkSafetyAction" "$CPMP_SAFETY_ACTION"
run_get "cpMpLinkStatusSequence" "$CPMP_STATUS_SEQUENCE"
run_get "cpMpLinkGlobalFlags" "$CPMP_GLOBAL_FLAGS"
run_get "driverModuleStatus" "$DRIVER_MODULE_STATUS"
run_get "eventSourcePowerOnCount" "$EVENT_SOURCE_POWER_ON_COUNT"
run_get "eventSourceResetCause" "$EVENT_SOURCE_RESET_CAUSE"
run_get "eventSourceMpEventCount" "$EVENT_SOURCE_MP_EVENT_COUNT"
run_get "eventSourceMpEventData" "$EVENT_SOURCE_MP_EVENT_DATA"
run_walk "Teknotel vendor subtree" "$TEKNOTEL_VENDOR_TREE"

if [[ "$MODE" == *v3 ]]; then
  run_get "unitSnmpV3ActiveUsername" "$UNIT_SNMPV3_ACTIVE_USERNAME"
fi

if [[ "$MODE" == "release-v2c" ]]; then
  expect_failure \
    "blocked v2c GET communityNameUser row 1" \
    "${SNMP_GET_CMD[@]}" "$TARGET_IP" "$COMMUNITY_NAME_USER_ROW1"
  expect_failure \
    "blocked v2c SET unitSnmpV3Apply" \
    "${SNMP_SET_CMD[@]}" "$TARGET_IP" "$UNIT_SNMPV3_APPLY" i 1
fi

printf 'SNMP smoke completed successfully.\n'
