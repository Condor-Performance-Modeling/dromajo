#!/bin/bash

# Check if the argument is provided
if [ -z "$1" ]; then
  echo "Usage: $0 <path_to_stf_load_store.linux.riscv>"
  exit 1
fi

INPUT_FILE=$1
export COMMON_DIR=common

if [ -f "$COMMON_DIR/rootfs.cpio" ]; then
  echo "rootfs.cpio already exists in $COMMON_DIR. Exiting."
  exit 0
fi

if [ -z "$BUILDROOT" ]; then
    echo "Required environment variable BUILDROOT is not set."
    echo "To set the required environment variables, cd into your work area and run: source how-to/env/setuprc.sh"
    exit 1
fi

sudo cp $COMMON_DIR/inittab $BUILDROOT/output/target/etc
sudo cp $COMMON_DIR/run_benchmark.sh $BUILDROOT/output/target/etc/init.d
sudo rm -rf $BUILDROOT/output/target/root/benchfs
sudo mkdir -p $BUILDROOT/output/target/root/benchfs
sudo cp $INPUT_FILE $BUILDROOT/output/target/root/benchfs
sudo make -C $BUILDROOT
sudo cp $BUILDROOT/output/images/rootfs.cpio $COMMON_DIR