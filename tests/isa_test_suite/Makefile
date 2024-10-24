RISCV_TESTS_DIR = ../riscv-tests
CONDOR_TESTS_DIR = ./condor_tests
RISCV_TESTS_INSTALL_DIR = $(shell pwd)/riscv-test-files
CONDOR_TESTS_INSTALL_DIR = $(shell pwd)/condor-test-files

# RISCV environment variable must be set
ifndef RISCV
    $(error RISCV environment variable is not set. \
    Please set it to the RISC-V toolchain installation directory. \
    Example: export RISCV=/data/tools/riscv64-unknown-elf)
endif

all: build_riscv_tests build_condor_tests

build_riscv_tests:
	@rm -rf $(RISCV_TESTS_INSTALL_DIR)
	@export RISCV_PREFIX=$(RISCV)/bin/riscv64-unknown-elf- && \
	cd $(RISCV_TESTS_DIR) && autoupdate && autoconf && \
	./configure --prefix=$(RISCV_TESTS_INSTALL_DIR) && make -j32 && make -j32 install
	
build_condor_tests:
	@rm -rf $(CONDOR_TESTS_INSTALL_DIR)
	@export RISCV_PREFIX=$(RISCV)/bin/riscv64-unknown-elf- && \
	cd $(CONDOR_TESTS_DIR) && autoupdate && autoconf && \
	./configure --prefix=$(CONDOR_TESTS_INSTALL_DIR) && make -j32 && make -j32 install

clean_install:
	@echo "Cleaning up install..."
	@rm -rf $(RISCV_TESTS_INSTALL_DIR)
	@rm -rf $(CONDOR_TESTS_INSTALL_DIR)

clean_all:
	@echo "Cleaning up all..."
	@rm -rf $(RISCV_TESTS_INSTALL_DIR)
	@rm -rf $(CONDOR_TESTS_INSTALL_DIR)
	@cd $(RISCV_TESTS_DIR) && make clean
	@cd $(CONDOR_TESTS_DIR) && make clean

.PHONY: all build_riscv_tests build_condor_tests clean_install clean_all
