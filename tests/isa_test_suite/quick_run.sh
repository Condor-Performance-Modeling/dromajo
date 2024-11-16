#Execute from the build directory

#./dromajo \
#--ctrlc \
#--march=rv64gc \
#../scripts/cpm.boot.cfg

T=../riscv-tests/isa/rv64uzfh-p-fadd
../../build/dromajo --ctrlc --march=rv64gc $T

#../scripts/bmi_mm.bare.riscv

#./dromajo --ctrlc --march=rv64gc_zfa \
#../tests/condor/bin/zfa/rv64ud-p-fleq_d

