#Execute from the build directory

#./dromajo \
#--ctrlc \
#--march=rv64gc \
#../scripts/cpm.boot.cfg

T=./bin/zfa/rv64ud-p-fli_s
../../build/dromajo --ctrlc --march=rv64gc $T

#./dromajo --ctrlc --march=rv64gc ../scripts/bmi_mm.bare.riscv

#./dromajo --ctrlc --march=rv64gc_zfa \
#../tests/condor/bin/zfa/rv64ud-p-fleq_d

