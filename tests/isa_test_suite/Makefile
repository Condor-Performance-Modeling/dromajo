RISCV_TESTS_DIR = ../riscv-tests
CONDOR_TESTS_DIR = ./build_isa
INSTALL_DIR = $(shell pwd)/test_files

# RISCV environment variable must be set
ifndef RISCV
    $(error RISCV environment variable is not set. \
    Please set it to the RISC-V toolchain installation directory. \
    Example: export RISCV=/data/tools/riscv64-unknown-elf)
endif

all: build_riscv_tests build_condor_tests

build_riscv_tests:
	@rm -rf $(INSTALL_DIR)
	@export RISCV_PREFIX=$(RISCV)/bin/riscv64-unknown-elf- && \
	cd $(RISCV_TESTS_DIR) && autoupdate && autoconf && \
	./configure --prefix=$(INSTALL_DIR) && make -j32 && make -j32 install
	
build_condor_tests:
	@rm -rf $(INSTALL_DIR)
	@export RISCV_PREFIX=$(RISCV)/bin/riscv64-unknown-elf- && \
	cd $(CONDOR_TESTS_DIR) && autoupdate && autoconf && \
	./configure --prefix=$(INSTALL_DIR) && make -j32 && make -j32 install

clean_install:
	@echo "Cleaning up install..."
	@rm -rf $(INSTALL_DIR)

clean_all:
	@echo "Cleaning up all..."
	@rm -rf $(INSTALL_DIR)
	@cd $(RISCV_TESTS_DIR) && make clean
	@cd $(CONDOR_TESTS_DIR) && make clean

.PHONY: all build_riscv_tests build_condor_tests clean_install clean_all
