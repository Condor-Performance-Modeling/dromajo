SHELL:=/usr/bin/bash
.PHONY: default whisper dromajo drom_x drom_s whisper_x

DROM   = ../../build/dromajo

# Note for this march switch to work these benchmarks should be compiled
# with a similar MARCH.

#D_OPTS = --ctrlc --show-march --stf_tracepoint --stf_priv_modes USHM 
D_OPTS = --march=rv64gc_zba_zbb_zbc_zbs --ctrlc --stf_tracepoint --stf_priv_modes USHM 

#D_TR   = --stf_trace=trace.stf

WHSP   = /data/users/jeffnye/condor/tools/bin/whisper
W_OPTS = --newlib --log

default: clean_logs dromajo

dromajo:
	mkdir -p logs
	mkdir -p stfs
	$(MAKE) drom_s E=bmi_sanity.bare
	$(MAKE) drom_s E=coremark_1.bare
	$(MAKE) drom_s E=bmi_dhrystone.llvm.bare
	$(MAKE) drom_s E=bmi_median.bare
	$(MAKE) drom_s E=bmi_median.llvm.bare
	$(MAKE) drom_s E=bmi_mm.bare
	$(MAKE) drom_s E=bmi_mm.llvm.bare
	$(MAKE) drom_s E=bmi_multiply.bare
	$(MAKE) drom_s E=bmi_multiply.llvm.bare
	$(MAKE) drom_s E=bmi_pmp.bare
	$(MAKE) drom_s E=bmi_pmp.llvm.bare
	$(MAKE) drom_s E=bmi_qsort.bare
	$(MAKE) drom_s E=bmi_rsort.bare
	$(MAKE) drom_s E=bmi_rsort.llvm.bare
	$(MAKE) drom_s E=bmi_sanity.bare
	$(MAKE) drom_s E=bmi_sanity.llvm.bare
	$(MAKE) drom_s E=bmi_spmv.bare
	$(MAKE) drom_s E=bmi_spmv.llvm.bare
	$(MAKE) drom_s E=bmi_towers.bare
	$(MAKE) drom_s E=bmi_towers.llvm.bare
	$(MAKE) drom_s E=bmi_vvadd.bare
	$(MAKE) drom_s E=bmi_vvadd.llvm.bare
	$(MAKE) drom_s E=coremark_1.bare
	$(MAKE) drom_s E=coremark_1.llvm.bare
	$(MAKE) drom_s E=coremark_10.bare
	$(MAKE) drom_s E=coremark_10.llvm.bare
	$(MAKE) drom_s E=dhry_no_print.bare
	$(MAKE) drom_s E=dhry_no_print.llvm.bare
	$(MAKE) drom_s E=dhrystone_opt1.bare
	$(MAKE) drom_s E=dhrystone_opt1.llvm.bare
	$(MAKE) drom_s E=dhrystone_opt2.bare
	$(MAKE) drom_s E=dhrystone_opt2.llvm.bare
	$(MAKE) drom_s E=dhrystone_opt3.bare
	$(MAKE) drom_s E=dhrystone_opt3.llvm.bare

drom_x:
	$(DROM) $(D_OPTS) $(BENCHMARKS)/bin/$(E).riscv  \
	|| (echo "Command failed, value of E: $(E)" && false)

drom_s:
	$(DROM) $(D_OPTS) --stf_trace=stfs/$(E).zstf $(BENCHMARKS)/bin/$(E).riscv

whisper:
	mkdir -p logs
	$(MAKE) whisper_x E=bmi_qsort.llvm.bare

whisper_x:
	$(WHSP) $(W_OPTS) $(BENCHMARKS)/bin/$(E).riscv  \
	|| (echo "Command failed, value of E: $(E)" && false)

clean_logs:
	rm -f logs/*

# Do not run - multi-threaded benchmarks
#	$(MAKE) drom_x E=bmi_mt-matmul.bare
#	$(MAKE) drom_x E=bmi_mt-matmul.llvm.bare
#	$(MAKE) drom_x E=bmi_mt-matmul.bare
#	$(MAKE) drom_x E=bmi_mt-matmul.llvm.bare
#	$(MAKE) drom_x E=bmi_mt-vvadd.bare
#	$(MAKE) drom_x E=bmi_mt-vvadd.llvm.bare

# Do not run - take  too long
#	$(MAKE) drom_x E=coremark_100K.bare
#	$(MAKE) drom_x E=coremark_100K.llvm.bare
#	$(MAKE) drom_x E=coremark_10K.bare
#	$(MAKE) drom_x E=coremark_10K.llvm.bare
#	$(MAKE) drom_x E=coremark_1K.bare
#	$(MAKE) drom_x E=coremark_1K.llvm.bare

# Unexplained error
#	$(MAKE) drom_x E=bmi_qsort.llvm.bare

# Known good
#	$(MAKE) drom_x E=bmi_median.bare
#	$(MAKE) drom_x E=bmi_dhrystone.llvm.bare
#	$(MAKE) drom_x E=bmi_median.llvm.bare
#	$(MAKE) drom_x E=bmi_mm.bare
#	$(MAKE) drom_x E=bmi_mm.llvm.bare
#	$(MAKE) drom_x E=bmi_multiply.bare
#	$(MAKE) drom_x E=bmi_multiply.llvm.bare
#	$(MAKE) drom_x E=bmi_pmp.bare
#	$(MAKE) drom_x E=bmi_pmp.llvm.bare
#	$(MAKE) drom_x E=bmi_qsort.bare
#	$(MAKE) drom_x E=bmi_rsort.bare
#	$(MAKE) drom_x E=bmi_rsort.llvm.bare
#	$(MAKE) drom_x E=bmi_sanity.bare
#	$(MAKE) drom_x E=bmi_sanity.llvm.bare
#	$(MAKE) drom_x E=bmi_spmv.bare
#	$(MAKE) drom_x E=bmi_spmv.llvm.bare
#	$(MAKE) drom_x E=bmi_towers.bare
#	$(MAKE) drom_x E=bmi_towers.llvm.bare
#	$(MAKE) drom_x E=bmi_vvadd.bare
#	$(MAKE) drom_x E=bmi_vvadd.llvm.bare
#	$(MAKE) drom_x E=coremark_1.bare
#	$(MAKE) drom_x E=coremark_1.llvm.bare
#	$(MAKE) drom_x E=coremark_10.bare
#	$(MAKE) drom_x E=coremark_10.llvm.bare
#	$(MAKE) drom_x E=dhry_no_print.bare
#	$(MAKE) drom_x E=dhry_no_print.llvm.bare
#	$(MAKE) drom_x E=dhrystone_opt1.bare
#	$(MAKE) drom_x E=dhrystone_opt1.llvm.bare
#	$(MAKE) drom_x E=dhrystone_opt2.bare
#	$(MAKE) drom_x E=dhrystone_opt2.llvm.bare
#	$(MAKE) drom_x E=dhrystone_opt3.bare
#	$(MAKE) drom_x E=dhrystone_opt3.llvm.bare
