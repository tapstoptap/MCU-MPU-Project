#!/bin/bash
set -e
ELF=$(ls build/Debug/*.elf 2>/dev/null | head -n 1)
if [ -z "$ELF" ]; then
  echo "No .elf found - build first (F7 or cmake --build --preset Debug)"
  exit 1
fi
arm-none-eabi-objcopy -O binary "$ELF" build/Debug/firmware.bin
st-flash --reset write build/Debug/firmware.bin 0x08000000
