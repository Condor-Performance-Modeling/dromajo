RISCV_TESTS_DIR = ../riscv-tests
INSTALL_DIR = $(shell pwd)/riscv_isa_test

all: rebuild_riscv_tests

rebuild_riscv_tests:
	@rm -rf $(INSTALL_DIR)
	@echo "Reuilding riscv-tests..."
	@export RISCV_PREFIX=/tools/riscv64-unknown-elf/bin/riscv64-unknown-elf- && \
	cd $(RISCV_TESTS_DIR) && autoupdate && autoconf && \
	./configure --prefix=$(INSTALL_DIR) && make -j32 clean && make -j32 && make -j32 install

clean:
	@echo "Cleaning up..."
	@rm -rf $(INSTALL_DIR)
	@cd $(RISCV_TESTS_DIR) && make clean

.PHONY: all rebuild_riscv_tests clean
