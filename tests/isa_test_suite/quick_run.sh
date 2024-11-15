#Execute from the build directory

#./dromajo \
#--ctrlc \
#--march=rv64gc \
#../scripts/cpm.boot.cfg

T=./riscv-test-files/share/riscv-tests/isa/rv64uzbb-p-rolw

../../build/dromajo --ctrlc --march=rv64gc $T

#../../build/dromajo --ctrlc --march=rv64gc ./riscv-test-files/share/riscv-tests/isa/rv64um-p-mulw
#../../build/dromajo --ctrlc --march=rv64gc ./riscv-test-files/share/riscv-tests/isa/rv64um-v-mulw

#../../build/dromajo --ctrlc --march=rv64gc ./riscv-test-files/share/riscv-tests/isa/rv64um-p-div
#../../build/dromajo --ctrlc --march=rv64gc ./riscv-test-files/share/riscv-tests/isa/rv64um-p-divu
#../../build/dromajo --ctrlc --march=rv64gc ./riscv-test-files/share/riscv-tests/isa/rv64um-p-divuw
#../../build/dromajo --ctrlc --march=rv64gc ./riscv-test-files/share/riscv-tests/isa/rv64um-p-divw
#../../build/dromajo --ctrlc --march=rv64gc ./riscv-test-files/share/riscv-tests/isa/rv64um-p-mul
#../../build/dromajo --ctrlc --march=rv64gc ./riscv-test-files/share/riscv-tests/isa/rv64um-p-mulh
#../../build/dromajo --ctrlc --march=rv64gc ./riscv-test-files/share/riscv-tests/isa/rv64um-p-mulhsu
#../../build/dromajo --ctrlc --march=rv64gc ./riscv-test-files/share/riscv-tests/isa/rv64um-p-mulhu
#../../build/dromajo --ctrlc --march=rv64gc ./riscv-test-files/share/riscv-tests/isa/rv64um-p-rem
#../../build/dromajo --ctrlc --march=rv64gc ./riscv-test-files/share/riscv-tests/isa/rv64um-p-remu
#../../build/dromajo --ctrlc --march=rv64gc ./riscv-test-files/share/riscv-tests/isa/rv64um-p-remuw
#../../build/dromajo --ctrlc --march=rv64gc ./riscv-test-files/share/riscv-tests/isa/rv64um-p-remw
#../../build/dromajo --ctrlc --march=rv64gc ./riscv-test-files/share/riscv-tests/isa/rv64um-v-div
#../../build/dromajo --ctrlc --march=rv64gc ./riscv-test-files/share/riscv-tests/isa/rv64um-v-divu
#../../build/dromajo --ctrlc --march=rv64gc ./riscv-test-files/share/riscv-tests/isa/rv64um-v-divuw
#../../build/dromajo --ctrlc --march=rv64gc ./riscv-test-files/share/riscv-tests/isa/rv64um-v-divw
#../../build/dromajo --ctrlc --march=rv64gc ./riscv-test-files/share/riscv-tests/isa/rv64um-v-mul
#../../build/dromajo --ctrlc --march=rv64gc ./riscv-test-files/share/riscv-tests/isa/rv64um-v-mulh
#../../build/dromajo --ctrlc --march=rv64gc ./riscv-test-files/share/riscv-tests/isa/rv64um-v-mulhsu
#../../build/dromajo --ctrlc --march=rv64gc ./riscv-test-files/share/riscv-tests/isa/rv64um-v-mulhu


#../../build/dromajo --ctrlc --march=rv64gc ./riscv-test-files/share/riscv-tests/isa/rv64um-v-rem
#../../build/dromajo --ctrlc --march=rv64gc ./riscv-test-files/share/riscv-tests/isa/rv64um-v-remu
#../../build/dromajo --ctrlc --march=rv64gc ./riscv-test-files/share/riscv-tests/isa/rv64um-v-remuw
#../../build/dromajo --ctrlc --march=rv64gc ./riscv-test-files/share/riscv-tests/isa/rv64um-v-remw

#../scripts/bmi_mm.bare.riscv

#./dromajo --ctrlc --march=rv64gc_zfa \
#../tests/condor/bin/zfa/rv64ud-p-fleq_d

