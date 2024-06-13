#!/bin/bash

# Check if the argument is provided
if [ -z "$1" ]; then
  echo "Usage: $0 <path_to_stf_load_store.linux.riscv>"
  exit 1
fi

if [ -z "$TOP" ]; then
    echo "Required environment variable TOP is not set."
    echo "To set the required environment variables, cd into your work area and run: source how-to/env/setuprc.sh"
    exit 1
fi

export OPT='--ctrlc --stf_exit_on_stop_opc --stf_tracepoint --stf_priv_modes USHM'
export DRO=../../bin/cpm_dromajo
export STF_RECORD_DUMP=/data/tools/bin/stf_record_dump
export SCRIPTS_DIR=scripts
export COLLATERAL_SCRIPT=scripts/create_linux_collateral.sh
INPUT_FILE=$1

echo "Cleaning previous traces..."
mkdir -p traces temp_traces
rm -f traces/* temp_traces/*
echo "Previous traces cleaned."

echo "Creating the linux collateral..."
bash $COLLATERAL_SCRIPT $INPUT_FILE
echo "Linux collateral created."

echo "Creating the linux traces..."
pwd
$DRO $OPT --stf_trace traces/linux.zstf $SCRIPTS_DIR/trace.boot.cfg
echo "Linux traces created."

echo "Dumping traces to text files..."
$STF_RECORD_DUMP traces/linux.zstf > temp_traces/new_trace.txt
$STF_RECORD_DUMP golden/linux.zstf > temp_traces/golden_trace.txt
echo "Traces dumped to text files."

echo "Comparing text traces..."
if diff temp_traces/new_trace.txt temp_traces/golden_trace.txt > temp_traces/diff_output.txt; then
  echo "Comparison successful: traces match the golden traces."
  exit 0
else
  echo "Comparison failed: traces do not match the golden traces."
  echo "Differences:"
  cat temp_traces/diff_output.txt
  exit 1
fi