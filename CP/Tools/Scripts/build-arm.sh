#!/usr/bin/env bash
# Build STM32 firmware. Pass "Debug" or "Release" as first argument (default: Release).
set -euo pipefail

PRESET="${1:-STM32-Release}"

echo "==> Configuring: $PRESET"
cmake --preset "$PRESET"

echo "==> Building: $PRESET"
cmake --build --preset "$PRESET"

echo "==> Done. Output: build/$PRESET/CP.elf"
