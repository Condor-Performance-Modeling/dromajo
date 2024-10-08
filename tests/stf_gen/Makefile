.PHONY: default clean run unpack pack check
SHELL=/bin/bash

DRO=../../build/dromajo

OPT=

bm_elfs = illegal bmi_mm.bare bmi_towers.bare

default: unpack run check

run:
	@echo "Generating traces"
	mkdir -p traces
	for i in $(bm_elfs); do \
		$(DRO) $(OPT) --stf_trace traces/$$i.zstf  elf/$$i.riscv; \
	done

check:
	@echo "Checking traces"
	@for i in $(bm_elfs); do \
		diff traces/$$i.zstf golden/$$i.zstf; \
	done;
	@echo "Checks complete"

unpack:
	cd elf;    tar xf *.bz2

pack:
	cd elf; tar jcvf elf.bz2 *.riscv
	$(MAKE) clean

clean:
	-rm -f traces/*.zstf elf/*.riscv
