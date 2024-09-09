SHELL:=/usr/bin/bash
.PHONY: default whisper dromajo spike copy_files data_rv64gc_lp64d \
        data_rv64gc_zbb_lp64d set

DROM   = ../../build/dromajo
D_OPTS = --ctrlc --trace 0
#D_OPTS = --ctrlc --stf_essential_mode --stf_tracepoint --stf_priv_modes USHM 
#D_TR   = --stf_trace=trace.stf

WHSP   = /data/users/jeffnye/condor/tools/bin/whisper
W_OPTS = --newlib --log

B_GC  := $(BENCHMARKS)/rv64gc_lp64d
B_ZBB := $(BENCHMARKS)/rv64gc_zbb_lp64d

#default: clean_logs dromajo whisper
default: clean_logs dromajo

#	$(MAKE) drom_one R=rv64gc_zbb A=lp64d E=bmi_sanity.bare
#	$(MAKE) drom_one R=rv64gc     A=lp64d E=bmi_sanity.bare

#	$(MAKE) drom_x E=bmi_mt-matmul.bare
#	$(MAKE) drom_x E=bmi_mt-matmul.llvm.bare
#	$(MAKE) drom_x E=bmi_mt-matmul.bare
#	$(MAKE) drom_x E=bmi_mt-matmul.llvm.bare
#	$(MAKE) drom_x E=bmi_mt-vvadd.bare
#	$(MAKE) drom_x E=bmi_mt-vvadd.llvm.bare

dromajo:
	mkdir -p logs
	$(MAKE) drom_x E=bmi_pmp.bare

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
#	$(MAKE) drom_x E=bmi_qsort.llvm.bare
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
#	$(MAKE) drom_x E=coremark_100K.bare
#	$(MAKE) drom_x E=coremark_100K.llvm.bare
#	$(MAKE) drom_x E=coremark_10K.bare
#	$(MAKE) drom_x E=coremark_10K.llvm.bare
#	$(MAKE) drom_x E=coremark_1K.bare
#	$(MAKE) drom_x E=coremark_1K.llvm.bare
#	$(MAKE) drom_x E=dhry_no_print.bare
#	$(MAKE) drom_x E=dhry_no_print.llvm.bare
#	$(MAKE) drom_x E=dhrystone_opt1.bare
#	$(MAKE) drom_x E=dhrystone_opt1.llvm.bare
#	$(MAKE) drom_x E=dhrystone_opt2.bare
#	$(MAKE) drom_x E=dhrystone_opt2.llvm.bare
#	$(MAKE) drom_x E=dhrystone_opt3.bare
#	$(MAKE) drom_x E=dhrystone_opt3.llvm.bare

drom_x:
	 $(DROM) $(D_OPTS) $(BENCHMARKS)/bin/$(E).riscv \
			|| (echo "Command failed, value of E: $(E)" && false)

# &> logs/d_bin.$(E).txt

whisper:
	mkdir -p logs
	$(MAKE) whisper_one R=rv64gc     A=lp64d E=bmi_sanity.bare
	$(MAKE) whisper_one R=rv64gc_zbb A=lp64d E=bmi_sanity.bare

#	$(MAKE) whisper_one B=rv64gc_lp64d E=bmi_sanity.bare
#	$(MAKE) whisper_one B=rv64gc_lp64d E=coremark_1.bare
#	$(MAKE) whisper_one B=rv64gc_lp64d E=coremark_1.llvm.bare
#	$(MAKE) whisper_one B=rv64gc_lp64d E=dhrystone_opt3.bare
#	$(MAKE) whisper_one B=rv64gc_lp64d E=dhrystone_opt3.llvm.bare
#	\
#	$(MAKE) whisper_one B=rv64gc_zbb_lp64d E=coremark_1.bare
#	$(MAKE) whisper_one B=rv64gc_zbb_lp64d E=coremark_1.llvm.bare
#	$(MAKE) whisper_one B=rv64gc_zbb_lp64d E=dhrystone_opt3.bare
#	$(MAKE) whisper_one B=rv64gc_zbb_lp64d E=dhrystone_opt3.llvm.bare

drom_one:
	-$(DROM) $(D_OPTS) $(BENCHMARKS)/$(R)_$(A)/$(E).riscv \
           &> logs/d_$(R)_$(A).$(E).txt

whisper_one:
	-$(WHSP) $(W_OPTS) --isa=$(R) \
           --logfile logs/w_$(R)_$(A).$(E).txt $(R)_$(A)/$(E).riscv

whisper_show:

clean_logs:
	rm -f logs/*


#copy_files: data_rv64gc_lp64d
#
#data_rv64gc_lp64d:
#	cd $(BENCHMARKS); make clean; \
#  make RV_ARCH=-march=rv64gc binaries; \
#  make dump_all; \
#	cp $(BENCHMARKS)/bin/bmi_sanity.bare.riscv            rv64gc_lp64d
#	cp $(BENCHMARKS)/bin/coremark_1.bare.riscv.bare.riscv rv64gc_lp64d
#	cp $(BENCHMARKS)/bin/coremark_1.llvm.bare.riscv       rv64gc_lp64d
#	cp $(BENCHMARKS)/bin/rdhrystone_opt3.bare.riscv       rv64gc_lp64d
#
#data_rv64gc_zbb_lp64d:
#	cd $(BENCHMARKS); make clean; \
#  make RV_ARCH=-march=rv64gc_zbb binaries; \
#  make dump_all; \
#	cp $(BENCHMARKS)/bin/bmi_sanity.bare.riscv            rv64gc_zbb_lp64d
#	cp $(BENCHMARKS)/bin/coremark_1.bare.riscv.bare.riscv rv64gc_zbb_lp64d
#	cp $(BENCHMARKS)/bin/coremark_1.llvm.bare.riscv       rv64gc_zbb_lp64d
#	cp $(BENCHMARKS)/bin/rdhrystone_opt3.bare.riscv       rv64gc_zbb_lp64d
#

#	$(DROM) $(D_OPTS) $(D_TR) $(ELF)


