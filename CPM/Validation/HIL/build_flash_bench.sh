#!/usr/bin/env bash
set -euo pipefail

usage()
{
  cat <<'EOF'
Usage:
  build_flash_bench.sh [build|flash|build-flash] [options]

Options:
  --cp-preset <preset>   Default: STM32-Debug
  --mp-preset <preset>   Default: STM32-Release
  --psm-preset <preset>  Default: STM32-Release
  --ssm-preset <preset>  Default: STM32-Release
  --help                 Show this message

Notes:
  - The wrapper builds and flashes PSM, SSM, MP, then CP.
  - IOM is not automated here; build and flash it separately from
    ../IOM/project/iom.uvproj.
EOF
}

run_build()
{
  local module_name="$1"
  local module_dir="$2"
  local preset="$3"

  printf '==> Building %s with %s\n' "$module_name" "$preset"
  (
    cd "$module_dir"
    ./Tools/Scripts/build-arm.sh "$preset"
  )
}

run_flash()
{
  local module_name="$1"
  local module_dir="$2"
  local preset="$3"

  printf '==> Flashing %s with %s\n' "$module_name" "$preset"
  (
    cd "$module_dir"
    ./Tools/Scripts/flash.sh "$preset"
  )
}

ACTION="build-flash"
CP_PRESET="STM32-Debug"
MP_PRESET="STM32-Release"
PSM_PRESET="STM32-Release"
SSM_PRESET="STM32-Release"

case "${1:-}" in
  build|flash|build-flash)
    ACTION="$1"
    shift
    ;;
  ""|--*)
    ;;
  *)
    usage
    exit 1
    ;;
esac

while [[ $# -gt 0 ]]; do
  case "$1" in
    --cp-preset)
      CP_PRESET="${2:?missing value for --cp-preset}"
      shift 2
      ;;
    --mp-preset)
      MP_PRESET="${2:?missing value for --mp-preset}"
      shift 2
      ;;
    --psm-preset)
      PSM_PRESET="${2:?missing value for --psm-preset}"
      shift 2
      ;;
    --ssm-preset)
      SSM_PRESET="${2:?missing value for --ssm-preset}"
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

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CPM_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
PROJECT_ROOT="$(cd "$CPM_ROOT/.." && pwd)"

CP_DIR="$CPM_ROOT/CP"
MP_DIR="$CPM_ROOT/MP"
PSM_DIR="$PROJECT_ROOT/PSM"
SSM_DIR="$PROJECT_ROOT/SSM"
IOM_PROJECT="$PROJECT_ROOT/IOM/project/iom.uvproj"

if [[ "$ACTION" == "build" || "$ACTION" == "build-flash" ]]; then
  run_build "PSM" "$PSM_DIR" "$PSM_PRESET"
  run_build "SSM" "$SSM_DIR" "$SSM_PRESET"
  run_build "MP" "$MP_DIR" "$MP_PRESET"
  run_build "CP" "$CP_DIR" "$CP_PRESET"
fi

if [[ "$ACTION" == "flash" || "$ACTION" == "build-flash" ]]; then
  run_flash "PSM" "$PSM_DIR" "$PSM_PRESET"
  run_flash "SSM" "$SSM_DIR" "$SSM_PRESET"
  run_flash "MP" "$MP_DIR" "$MP_PRESET"
  run_flash "CP" "$CP_DIR" "$CP_PRESET"
fi

printf '\nIOM reminder:\n'
printf '  Build and flash manually from %s\n' "$IOM_PROJECT"
