#!/usr/bin/env bash
# Build STM32 firmware. Pass "STM32-Debug" or "STM32-Release" as first argument (default: STM32-Release).
set -euo pipefail

PRESET="${1:-STM32-Release}"

echo "==> Configuring: $PRESET"
cmake --preset "$PRESET"

echo "==> Building: $PRESET"
cmake --build --preset "$PRESET"

echo "==> Done. Output: build/$PRESET/PSM.elf"
