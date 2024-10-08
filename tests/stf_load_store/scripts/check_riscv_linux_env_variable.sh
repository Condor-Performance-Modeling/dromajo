#!/bin/bash

if [ -z "$RISCV_LINUX" ]; then
    echo "Error: RISCV_LINUX environment variable is not set."
    echo "Please set it to the RISC-V Linux installation directory."
    echo "Example: export RISCV_LINUX=/data/tools/riscv64-unknown-linux-gnu"
    echo "IMPORTANT! After setting the variable, remember to run cmake .. in your build directory!"
    exit 1
fi

echo "RISCV_LINUX environment variable is set to: $RISCV_LINUX"
echo "IMPORTANT! After setting the variables, remember to run cmake .. in your build directory!"
exit 0
