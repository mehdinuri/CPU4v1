#!/bin/bash
# Tools/Scripts/format-user-code.sh
#
# Format user code with Uncrustify using MISRA-C rules.
# Skips generated and third-party code (ST drivers, FreeRTOS, LWIP, mbedTLS).
#
# Usage:
#   ./Tools/Scripts/format-user-code.sh           # Format in-place
#   ./Tools/Scripts/format-user-code.sh --check   # verify only, exit 1 if any file needs reFormatting
#
# From CMake:
#   cmake --build <build-dir> --target Format
#   cmake --build <build-dir> --target Format-Check

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
CONFIG="${SCRIPT_DIR}/../Format/MISRA-C.cfg"
CHECK_ONLY=false

for arg in "$@"; do
    case "$arg" in
        --check) CHECK_ONLY=true ;;
        *) echo "Unknown argument: $arg" >&2; exit 1 ;;
    esac
done

# ---------------------------------------------------------------------------
# Locate uncrustify
# ---------------------------------------------------------------------------
if ! command -v uncrustify &>/dev/null; then
    echo "Error: uncrustify not found in PATH." >&2
    echo "Install:" >&2
    echo "  sudo apt install uncrustify   # Debian/Ubuntu" >&2
    echo "  brew install uncrustify       # macOS" >&2
    exit 1
fi

if [ ! -f "$CONFIG" ]; then
    echo "Error: config not found: $CONFIG" >&2
    exit 1
fi

# ---------------------------------------------------------------------------
# User code directories (relative to repo root)
# Third-party / CubeMX-generated paths are intentionally excluded.
# ---------------------------------------------------------------------------
USER_DIRS=(
    "App/Domain"
    "App/Ports"
    "App/Adapters"
    "App/Platform"
    "Tests"
)

# ---------------------------------------------------------------------------
# Collect .c / .h files
# ---------------------------------------------------------------------------
FILES=()
for dir in "${USER_DIRS[@]}"; do
    full_dir="${REPO_ROOT}/${dir}"
    if [ -d "$full_dir" ]; then
        while IFS= read -r -d '' file; do
            FILES+=("$file")
        done < <(find "$full_dir" \( -name "*.c" -o -name "*.h" \) -print0 | sort -z)
    fi
done

TOTAL=${#FILES[@]}

if [ "$TOTAL" -eq 0 ]; then
    echo "No C/H files found in user directories."
    exit 0
fi

# ---------------------------------------------------------------------------
# Check mode: report files that need reFormatting (no modifications)
# ---------------------------------------------------------------------------
if $CHECK_ONLY; then
    echo "Checking Formatting of ${TOTAL} files..."
    UNFORMATTED=0
    for file in "${FILES[@]}"; do
        if ! uncrustify -c "$CONFIG" -l C -f "$file" 2>/dev/null | diff -q "$file" - &>/dev/null; then
            echo "  NEEDS FORMAT: ${file#"${REPO_ROOT}/"}"
            UNFORMATTED=$((UNFORMATTED + 1))
        fi
    done
    echo ""
    if [ "$UNFORMATTED" -gt 0 ]; then
        echo "Format check FAILED: ${UNFORMATTED}/${TOTAL} files need reFormatting."
        echo "Run:  cmake --build <build-dir> --target Format"
        exit 1
    fi
    echo "Format check PASSED: all ${TOTAL} files conform to MISRA-C style."
    exit 0
fi

# ---------------------------------------------------------------------------
# Format mode: rewrite files in-place via a temporary file list
# ---------------------------------------------------------------------------
TMPLIST="$(mktemp)"
trap 'rm -f "$TMPLIST"' EXIT

for file in "${FILES[@]}"; do
    echo "$file" >> "$TMPLIST"
done

echo "Formatting ${TOTAL} user code files..."

# Two passes are required: the first pass adds long-block closing comments
# (mod_add_long_switch_closebrace_comment, mod_add_long_ifdef_else_comment),
# which shifts line counts; the second pass stabilises all positions.
for PASS in 1 2; do
    if ! uncrustify -c "$CONFIG" -l C --no-backup -F "$TMPLIST" 2>/dev/null; then
        echo "" >&2
        echo "Uncrustify failed on pass ${PASS}." >&2
        exit 1
    fi
done

echo "Done: ${TOTAL} files Formatted."
