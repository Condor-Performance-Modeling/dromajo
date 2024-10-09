#!/bin/bash

export OPT='--ctrlc --stf_force_zero_sha --stf_exit_on_stop_opc --stf_priv_modes USHM'
export DRO=../../bin/cpm_dromajo
export STF_RECORD_DUMP=/data/tools/bin/stf_record_dump
export SCRIPTS_DIR=scripts
export COLLATERAL_SCRIPT=scripts/create_linux_collateral.sh
INPUT_FILE=common/stf_load_store.linux.riscv

echo "Cleaning previous traces..."
mkdir -p traces temp_traces
rm -f traces/* temp_traces/*
echo "Previous traces cleaned."

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
