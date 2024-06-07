#!/bin/bash

# Check if the argument is provided
if [ -z "$1" ]; then
  echo "Usage: $0 <path_to_stf_load_store.bare.riscv>"
  exit 1
fi

export OPT='--ctrlc --stf_force_zero_sha --stf_tracepoint --stf_priv_modes USHM'
export DRO=../../bin/cpm_dromajo
INPUT_FILE=$1

echo "clean previous traces"
mkdir -p traces
rm -f traces/*
echo "Previous traces cleaned."

echo "create the bare metal traces"
$DRO $OPT --stf_trace traces/stf_load_store.zstf $INPUT_FILE
echo "Bare metal traces created."

echo "compare to the golden traces"
if diff traces/stf_load_store.zstf golden/stf_load_store.zstf > /dev/null; then
  echo "Comparison successful: traces match the golden traces."
  exit 0
else
  echo "Comparison failed: traces do not match the golden traces."
  exit 1
fi
echo ""
