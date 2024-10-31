# --------------------------------------------------------------------
RVTST_INC=-I./inc -I../riscv-tests/env/p -I../riscv-tests/isa/macros/scalar
RVTST_LNK=-T../riscv-tests/env/p/link.ld
# --------------------------------------------------------------------
ANDES_CC=/data/tools/AndeSight_STD_v530/toolchains/nds64le-elf-newlib-v5f/bin/riscv64-unknown-elf-gcc

ANDES_CC_OPTS =-mcpu=ax65 -march=rv64g_zba_zbb_zbc_zbs_xandes -mcmodel=medany -static -fvisibility=hidden -nostdlib -nostartfiles $(RVTST_INC) $(RVTST_LNK)
#ANDES_CC_OPTS =-mcpu=ax65 -march=rv64gc_zba_zbb_zbc_zbs -mabi=lp64d -mcmodel=medany -static -fvisibility=hidden -nostdlib -nostartfiles $(RVTST_INC) $(RVTST_LNK)
#ANDES_CC_OPTS=-static -mcmodel=medany -fvisibility=hidden -nostdlib -nostartfiles $(RVTST_INC) $(RVTST_LNK)
#ANDES_CC_OPTS=-static -mcmodel=medany -fvisibility=hidden -nostdlib -nostartfiles -I../riscv-tests/env/p -I../riscv-tests/isa/macros/scalar -T../riscv-tests/env/p/link.ld

# --------------------------------------------------------------------
ANDES_OBJD=/data/tools/AndeSight_STD_v530/toolchains/nds64le-elf-newlib-v5f/bin/riscv64-unknown-elf-objdump

ANDES_OBJD_OPTS=--disassemble-all --disassemble-zeroes --section=.text --section=.text.startup --section=.text.init --section=.data -Mnumeric,no-aliases

# --------------------------------------------------------------------
ANDES_SIM=$(CPM_DROMAJO)/bin/cpm_dromajo

ANDES_SIM_OPTS=--ctrlc --trace 0 --march=rv64g_zba_zbb_zbc_zbs_xandes

# --------------------------------------------------------------------
# --------------------------------------------------------------------
