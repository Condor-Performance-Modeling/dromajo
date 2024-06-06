#!/bin/bash

# Check if the argument is provided
if [ -z "$1" ]; then
  echo "Usage: $0 <path_to_stf_load_store.bare.riscv>"
  exit 1
fi

# Set environment variables
export OPT='--ctrlc --stf_essential_mode --stf_tracepoint --stf_priv_modes USHM'
export DRO=../../bin/cpm_dromajo
INPUT_FILE=$1

# Clean previous traces
echo "Cleaning previous traces..."
mkdir -p traces
rm -f traces/*
echo "Previous traces cleaned."

# Create the bare metal traces
echo "Creating the bare metal traces..."
echo "Command: $DRO $OPT --stf_trace traces/stf_load_store.zstf $INPUT_FILE"
$DRO $OPT --stf_trace traces/stf_load_store.zstf $INPUT_FILE
echo "Bare metal traces created."

# Compare to the golden traces
echo "Comparing to the golden traces..."
echo "Command: diff traces/stf_load_store.zstf golden/stf_load_store.zstf"
diff_output=$(diff traces/stf_load_store.zstf golden/stf_load_store.zstf)
diff_exit_status=$?

# Output the result of the diff command
if [ $diff_exit_status -eq 0 ]; then
  echo "Comparison successful: traces match the golden traces."
else
  echo "Comparison failed: traces do not match the golden traces."
  echo "Diff output:"
  echo "$diff_output"
fi

exit $diff_exit_status
