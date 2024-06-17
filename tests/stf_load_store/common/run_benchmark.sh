#!/bin/sh

# Read the kernel command line
cmdline=$(cat /proc/cmdline)

# Extract the value of the custom_param parameter
benchmark=$(echo "$cmdline" | sed -n 's/.*benchmark=\([^ ]*\).*/\1/p')

# If benchmark is 'none', skip everything and continue booting
if [ "$benchmark" = "none" ]; then
    echo "Benchmark is set to 'none'. Skipping benchmark execution."
    exit 0
fi

# Define the programs to run based on the custom_param value
case "$benchmark" in
    stf_load_store.linux)
        program="/root/benchfs/stf_load_store.linux.riscv"
        ;;

    *)
        echo "No valid program specified"
        program=""
        ;;
esac

# Check if the program exists and is executable
if [ -n "$program" ]; then
    if [ -e "$program" ]; then
        if [ -x "$program" ]; then
            $program
        else
            echo "Program $program is not executable"
            exit 1
        fi
    else
        echo "Program $program not found"
        exit 1
    fi
fi

sync
# Halt or power off the system after the program finishes
halt

