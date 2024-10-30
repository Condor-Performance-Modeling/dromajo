#!/bin/bash

export OPT='--ctrlc --stf_force_zero_sha --stf_priv_modes USHM'
export RESET_VECTOR_OPT='--reset_vector 0x00000000 --memory_addr 0x00000000 --bootrom ./common/bootrom.elf'
export DRO=../../bin/cpm_dromajo
BIN_DIR=./elf
GOLDEN_DIR=./golden

echo "Cleaning previous traces..."
mkdir -p traces
rm -f traces/*
echo "Previous traces cleaned."

fail_count=0

echo "Creating the bare metal traces..."
for INPUT_FILE in $BIN_DIR/*.riscv; do
    base_name=$(basename "$INPUT_FILE")
    
    echo ""
    echo "Processing file: $INPUT_FILE"
    $DRO $RESET_VECTOR_OPT $OPT --exe_trace 0 --exe_trace_log traces/"$base_name".log "$INPUT_FILE"
    
    if [ -f "$GOLDEN_DIR/$base_name".log ]; then
        echo "Comparing binary trace files for $base_name..."
        if diff traces/"$base_name".log "$GOLDEN_DIR/$base_name".log > /dev/null; then
            echo "Comparison successful for $base_name: binary traces match the golden traces."
        else
            echo "Comparison failed for $base_name: binary traces do not match the golden traces."
            fail_count=$((fail_count+1))
        fi
    else
        echo "Warning: No golden trace file found for $base_name"
        fail_count=$((fail_count+1))
    fi
done

echo "All files processed."
echo "Number of failed comparisons: $fail_count"

if [ $fail_count -eq 0 ]; then
    echo "All traces matched successfully."
    exit 0
else
    echo "Some traces did not match. Number of failed comparisons: $fail_count"
    exit 1
fi
