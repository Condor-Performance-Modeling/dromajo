/data/tools/riscv-embecosm-embedded-ubuntu2204-20240407-14.0.1/bin/riscv64-unknown-elf-gcc -O3 -flto -march=rv64imacdf_zicond_zba_zbb_zbc_zbs_zfa_zfh -mabi=lp64d -mcmodel=medany -nostdlib -nostartfiles -ffast-math -funsafe-math-optimizations -finline-functions -fno-common -fno-builtin-printf -fno-tree-loop-distribute-patterns -DPREALLOCATE=1 -D__riscv=1  -static -std=gnu99 -I./inc -I../riscv-tests/env/p -I../riscv-tests/isa/macros/scalar  -T/data/users/jeffnye/condor/benchmarks/linker/common_proposed.ld \
  -I./src/decoder src/decoder/*.c src/decoder/*.S \
  -o bin/decoder/decoder.riscv -lm -lgcc

/data/tools/riscv-embecosm-embedded-ubuntu2204-20240407-14.0.1/bin/riscv64-unknown-elf-objdump --disassemble-all --disassemble-zeroes --section=.text --section=.text.startup --section=.text.init --section=.data -Mnumeric,no-aliases \
  ./bin/decoder/decoder.riscv > ./logs/decoder/decoder.riscv.dump

/data/users/jeffnye/condor/cpm.dromajo/bin/cpm_dromajo --ctrlc --trace 0 --march=rv64g_zba_zbb_zbc_zbs_xandes_zfa \
  ./bin/decoder/decoder.riscv 

