#!/usr/bin/env bash
# Flash firmware to STM32G473 via OpenOCD or STM32CubeProgrammer.
# Usage: ./flash.sh [STM32-Debug|STM32-Release]
set -euo pipefail

PRESET="${1:-STM32-Release}"
ELF="build/$PRESET/MP.elf"

if [[ ! -f "$ELF" ]]; then
    echo "ERROR: $ELF not found. Run build-arm.sh first."
    exit 1
fi

echo "==> Flashing $ELF"

# Try STM32CubeProgrammer first, fall back to OpenOCD
if command -v STM32_Programmer_CLI &>/dev/null; then
    STM32_Programmer_CLI \
        --connect port=SWD \
        --write "$ELF" \
        --verify \
        --go
elif command -v openocd &>/dev/null; then
    openocd \
        -f interface/stlink.cfg \
        -f target/stm32g4x.cfg \
        -c "program $ELF verify reset exit"
else
    echo "ERROR: Neither STM32_Programmer_CLI nor openocd found in PATH."
    exit 1
fi

echo "==> Flash complete."
