# --------------------------------------------------------------------
RVTST_INC=-I./inc -I../riscv-tests/env/p -I../riscv-tests/isa/macros/scalar
RVTST_LNK=-T../riscv-tests/env/p/link.ld
# --------------------------------------------------------------------
OBJD_OPTS=--disassemble-all --disassemble-zeroes --section=.text --section=.text.startup --section=.text.init --section=.data -Mnumeric,no-aliases
DROM=$(CPM_DROMAJO)/bin/cpm_dromajo
DROM_OPTS=--ctrlc --trace 0 --march=rv64g_zba_zbb_zbc_zbs_xandes_zfa
COMMON_LINK=-T$(BENCHMARKS)/linker/common_proposed.ld
# --------------------------------------------------------------------
ANDES_CC=/data/tools/AndeSight_STD_v530/toolchains/nds64le-elf-newlib-v5f/bin/riscv64-unknown-elf-gcc
ANDES_CC_OPTS =-mcpu=ax65 -march=rv64g_zba_zbb_zbc_zbs_xandes -mcmodel=medany -static -fvisibility=hidden -nostdlib -nostartfiles $(RVTST_INC) $(RVTST_LNK)
# --------------------------------------------------------------------
ANDES_OBJD=/data/tools/AndeSight_STD_v530/toolchains/nds64le-elf-newlib-v5f/bin/riscv64-unknown-elf-objdump
ANDES_OBJD_OPTS=$(OBJD_OPTS)
# --------------------------------------------------------------------
ANDES_SIM=$(DROM)
ANDES_SIM_OPTS=$(DROM_OPTS)
# --------------------------------------------------------------------
CC_ROOT=/data/tools/riscv-embecosm-embedded-ubuntu2204-20240407-14.0.1
CONDOR_CC=$(CC_ROOT)/bin/riscv64-unknown-elf-gcc

DECODER_CC_OPTS=  \
 -march=rv64imacdf_zba_zbb_zbc_zbs_zfa -mabi=lp64d -mcmodel=medany \
 -nostdlib -nostartfiles -ffast-math -funsafe-math-optimizations \
 -finline-functions -fno-common -fno-builtin-printf \
 -fno-tree-loop-distribute-patterns \
  -DPREALLOCATE=1 -D__riscv=1  -static -std=gnu99

CONDOR_CC_OPTS= -O3 -flto $(DECODER_CC_OPTS)

CONDOR_OBJD=$(CC_ROOT)/bin/riscv64-unknown-elf-objdump
CONDOR_OBJD_OPTS=$(OBJD_OPTS)

CONDOR_SIM=$(DROM)
CONDOR_SIM_OPTS=$(DROM_OPTS)
# --------------------------------------------------------------------
