#!/bin/bash

if [ -z "$RISCV" ]; then
    echo "Error: RISCV environment variable is not set."
    echo "Please set it to the RISC-V toolchain installation directory."
    echo "Example: export RISCV=/data/tools/riscv64-unknown-elf"
    echo "IMPORTANT! After setting the variable, remember to run cmake .. in your build directory!"
    exit 1
fi

echo "RISCV environment variable is set to: $RISCV"
echo "IMPORTANT! After setting the variable, remember to run cmake .. in your build directory!"
exit 0
