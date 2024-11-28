#!/bin/bash

set -e

export COMMON_DIR=$(pwd)/common
INPUT_FILE=$COMMON_DIR/stf_load_store.linux.riscv

if [ -z "$BUILDROOT" ]; then
    echo "Required environment variable BUILDROOT is not set."
    echo "To set the required environment variables, cd into your work area and run: source how-to/env/setuprc.sh"
    exit 1
fi

if [ -z "$OPENSBI" ]; then
    echo "Required environment variable OPENSBI is not set."
    echo "To set the required environment variables, cd into your work area and run: source how-to/env/setuprc.sh"
    exit 1
fi

if [ -z "$TOP" ]; then
    echo "Required environment variable TOP is not set."
    echo "To set the required environment variables, cd into your work area and run: source how-to/env/setuprc.sh"
    exit 1
fi

if [ -z "$RV_LINUX_TOOLS" ]; then
    echo "Required environment variable RV_LINUX_TOOLS is not set."
    echo "To set the required environment variables, cd into your work area and run: source how-to/env/setuprc.sh"
    exit 1
fi

if [ ! -L $TOP/riscv64-unknown-elf ]; then
    echo "Required environment variable riscv64-unknown-elf is not set."
    exit 1
fi

if [ ! -L $TOP/riscv64-unknown-linux-gnu ]; then
    echo "Required environment variable riscv64-unknown-linux-gnu is not set."
    exit 1
fi

export PATH="$RV_LINUX_TOOLS/bin:$PATH"

TMP_DIR=$(mktemp -d -t build-XXXXXX)
echo "Created temporary directory at $TMP_DIR"

echo "Building the Linux kernel"
cd $TMP_DIR
if [ ! -d "linux-5.8-rc4" ]; then
    wget --no-check-certificate -nc https://git.kernel.org/torvalds/t/linux-5.8-rc4.tar.gz
    tar -xf linux-5.8-rc4.tar.gz
fi

grep -qxF 'KBUILD_CFLAGS += -march=rv64imafdc_zicsr_zifencei' linux-5.8-rc4/Makefile \
|| echo 'KBUILD_CFLAGS += -march=rv64imafdc_zicsr_zifencei' >> linux-5.8-rc4/Makefile

make -C linux-5.8-rc4 ARCH=riscv defconfig
make -C linux-5.8-rc4 ARCH=riscv -j$(nproc)

if [ -f linux-5.8-rc4/arch/riscv/boot/Image ]; then
    echo "Updating Linux kernel image in $COMMON_DIR"
    sudo cp linux-5.8-rc4/arch/riscv/boot/Image $COMMON_DIR
else
    echo "Error: Linux kernel image (Image) build failed."
    exit 1
fi

echo "Cleaning up temporary directory"
rm -rf $TMP_DIR
echo "Temporary directory removed."

echo "Building OpenSBI firmware using $OPENSBI"
make -C $OPENSBI PLATFORM=generic -j$(nproc)

if [ -f $OPENSBI/build/platform/generic/firmware/fw_jump.bin ]; then
    echo "Updating OpenSBI firmware in $COMMON_DIR"
    sudo cp $OPENSBI/build/platform/generic/firmware/fw_jump.bin $COMMON_DIR
else
    echo "Error: OpenSBI firmware (fw_jump.bin) build failed."
    exit 1
fi

sudo cp $COMMON_DIR/inittab $BUILDROOT/output/target/etc
sudo cp $COMMON_DIR/run_benchmark.sh $BUILDROOT/output/target/etc/init.d
sudo rm -rf $BUILDROOT/output/target/root/benchfs
sudo mkdir -p $BUILDROOT/output/target/root/benchfs
sudo cp $INPUT_FILE $BUILDROOT/output/target/root/benchfs
sudo make -C $BUILDROOT
sudo cp $BUILDROOT/output/images/rootfs.cpio $COMMON_DIR

