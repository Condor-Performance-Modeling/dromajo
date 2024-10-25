
# Dromajo - RISC-V Reference Model

This is the Condor Computing fork of the Dromajo RISC-V golden model.

This source is derived from the Esperanto source which was
in turn derived from Fabrice Bellard's RISCVEMU/TinyEMU.

The original contents of the Esperanto README.md are found in ESPERANTO_README.md.
Some of the original contents are also duplicated here.

The original repo is here: https://github.com/chipsalliance/dromajo.

This file is located here: https://github.com/Condor-Performance-Modeling/dromajo.


## Cloning

```
git clone --recurse-submodules git@github.com:Condor-Performance-Modeling/dromajo.git cpm.dromajo
```
Or
```
git clone git@github.com:Condor-Performance-Modeling/dromajo.git cpm.dromajo
cd cpm.dromajo
git submodule update --init --recursive
```

## Building

A debug build is built by default.  To create a release 
build use -DCMAKE_BUILD_TYPE=Release.

### Debug build
The name of the directory is cosmetic.
```
<cd cpm.dromajo>
mkdir debug; cd debug
cmake .. 
make -j
```

### Release build
The name of the directory is cosmetic.
```
<cd cpm.dromajo>
mkdir release; cd release
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j
```

## Build artifacts

The artifacts produced by a build are the `dromajo` simulator and the
`libdromajo_cosim.a` library with associated `dromajo_cosim.h` header file.

See ESPERANTO_README.md for documentation on co-simulation.

Check the [setup.md](doc/setup.md) for instructions how to compile tests like
booting Linux and baremetal for dromajo.

## Run simple regression.

We intend to release a regression suite using riscv-tests in the future.  

NOTE: At present the regression is very limited.

From a build directory
```
make regress
```

## Usage

There are multiple uses cases for dromajo. We show an example of dumping a trace to the console, and and example which generates STF lib trace file from the same elf.

These examples require a cross compile bare metal elf.

An sample is provided ../tests/elfs/bmi_sanity.bare.riscv.

This sample has also been instrumented to produce STF trace output.

### Standard use case examples(3) 

Emitting a text trace log
```
./dromajo \
--ctrlc \
--exe_trace 0 \
--exe_trace_log  this.log \
../tests/condor/bin/andestar/rv64ui-p-addigp
```

Emitting a compressed STF w/ march specification
```
./dromajo \
--ctrlc \
--march=rv64gc_rba_zbb_zbc_zbs \
--stf_priv_modes USHM \
--stf_trace=example.zstf \
$BENCHMARKS/bin/bmi_mm.bare.riscv
```

Booting linux
```
cp $TOOLS/riscv-linux/* .
cp $PATCHES/cpm.boot.cfg .

./dromajo \
--ctrlc \
--march=rv64gc \
--stf_priv_modes USHM \
--stf_trace example.stf \
cpm.boot.cfg
```

## STF trace options

Command line options are added to control STF trace generation.

```
    --stf_trace <file> Dump an STF trace to the given file
                  Use .zstf as the file extension for compressed trace
                  output. Use .stf for uncompressed output
    --stf_exit_on_stop_opc Terminate the simulation after 
                  detecting a STOP_TRACE opcode. Using this
                  switch will disable non-contiguous region
                  tracing. The first STOP_TRACE opcode will 
                  terminate the simulator.
    --stf_memrecord_size_in_bits write memory access size in bits
                   instead of bytes
    --stf_trace_register_state include register state in the STF
                   (default false)
    --stf_disable_memory_records Do not add memory records to 
                   STF trace. By default memory records are 
                   always traced.
                   (default false)
    --stf_priv_modes <USHM|USH|US|U> Specify which privilege 
                  modes to include for STF trace generation
    --stf_force_zero_sha Emit 0 for all SHA's in the STF header. This is a 
                  debug option. Also clears the dromajo version placed in
                  the STF header.
```

## Enabling RISC-V extensions

This version of dromajo has early support for RISC-V extension specification using the --march switch. The format is similar to the --march switched used by GCC and LLVM.

NOTE: This is an experimental feature with some caveats. 
    - This implementation supports enable/disable of zba zbb zbc zbs extensions
    - The zbx extensions are disabled by default.
    - Each variant of zbx is enabled by a separate string.
    - Enforcement is not complete, for example XLEN is currently a conditional compile setting
        - We intend to modify this behavior
        - We will retain the previous default behavior until we have a broader regression suite

To enable all zbx extensions use the --march switch as shown:
```
./dromajo --march=rv64gc_zba_zbb_zbc_zbs  <etc>
```

Commandline help shows other related options:
```
  ISA selection options EXPERIMENTAL
    --march <string> Specify the architecture string to enable
                  supported ISA extensions, default is rv64gc.
                  --help-march to see currently supported set.
    --show-march  Takes a complete option set and shows the
                  enabled extensions. Then exits. 
    --help-march  List the currently supported ISA extension set.
```

## Running Dromajo with custom start of program memory

By default, Dromajo uses 0x80000000 as the start of program memory. You can specify an alternative start address, through command line options, bootrom changes and a matching linker file.

1. **Create your .elf with custom start address:**

Modify your linker script (`.ld`) to set the desired start address. For example, to set the start address to `0x20000000`, include the following line:

```ld
/* set loc counter */
. = 0x20000000;
```

2. **Modify and build Bootrom**

You need to provide a custom bootrom via the `--bootrom` option to run dromajo with custom base address. Modify the Makefile in the bootrom subdirectory to your desired start address. Update the MEMORY_START variable in the Makefile, for example:

```Makefile
MEMORY_START=0x20000000
```

Bootrom uses `CC=$(RISCV)/bin/riscv64-unknown-elf-gcc` to build custom bootrom. `RISCV` environment veriable is not set by default so it must be set manually before building bootrom. To build the bootrom:

```bash
cd bootrom
make
```

This will generate the `bootrom.elf` file to be used in `--bootrom` program option.

3. **Run Dromajo with custom options**

Use the `--reset_vector`, `--memory_addr`, and `--bootrom` switches to run Dromajo with the desired start address. For example, to set the start address to `0x20000000`, run:

```bash
./dromajo --stf_trace /path/to/traces/trace.zstf --reset_vector "0x20000000" --memory_addr "0x20000000" --bootrom /path/to/bootrom/bootrom.elf /path/to/your/program.elf
```

**Note:** The start address in the linker script, the `--reset_vector` value, and the `--memory_addr` value **must** all be the same for Dromajo to run correctly.

## Testing

This project includes a set of unit tests and custom targets to ensure the reliability and correctness of the Dromajo. Tests are defined in the `tests` directory and can be run using CTest. Test related targets depend on `RISCV` and `RISCV_LINUX` environment variables `RISCV` and `RISCV_LINUX` environment variables are not set by default so it must be set manually before running below targets.

### Test targets

We have defined two custom targets to manage the testing process:

1. **regress**: This target runs the regression tests against the simulator.
2. **update_test_files**: This target updates the test files after changes to the project sources.

**Important Note:** The `update_test_files` target does not update the golden files. These files need to be manually regenerated before running the regression tests again.

### Test logs

Logs from running the `regress` target are available for review. Logs are located in the build directory and can be accessed for detailed information on the test results. The logs are stored at `build/tests/Testing/Temporary/LastTest.log`. The logs contain output from both passed and failed tests.

### Implemented tests

- **stf_load_store_baremetal:** This test runs a baremetal load/store operation, generating trace files for comparison. It checks the accuracy of the new traces against the golden (reference) traces to ensure that they match, confirming correct functionality.

- **stf_load_store_linux:** This test performs the same load/store operation in a Linux environment, following the same process of generating and comparing traces with the golden ones to ensure consistency and correctness.

- **stf_gen_ad_hoc:** This test generates and compares traces for various bare metal operations. It extracts ELF files, runs load/store operations, and compares the generated traces against golden (reference) traces to ensure they match. This process is done for both uncompressed and compressed traces, with the aim of validating the integrity of the STF (System Trace Format) files in different configurations.

- **riscv_isa_test:** This test runs a suite of RISC-V ISA compliance tests using a predefined list of enabled tests. It simulates each test and evaluates the results to verify correct functionality. Tests can be enabled/disabled in `isa_tests_list.txt` in the isa_test_suite directory. In this file, disabled tests are preceded by "x".

  This test can be run manually by navigating from dromajo directory:

  ```bash
  cd tests/isa_test_suite
  make
  bash run_riscv_isa_test_suite.sh 
  ```

  When `riscv-tests` submodule changes, new tests might be added. To update `isa_tests_list.txt` with those tests run:

  ```bash
  cd tests/isa_test_suite
  make
  bash update_isa_tests_list.sh
  ```

  General structure of isa test names:

  - **rv**: Stands for "RISC-V", indicating that these are RISC-V architecture tests.

  - **64 or 32**: Indicates the bit-width of the test.
    - `rv64`: Tests for 64-bit RISC-V systems.
    - `rv32`: Tests for 32-bit RISC-V systems.

  - **ISA or Instruction Set Extensions**: Represented by two-letter suffixes:
    - **`ui`**: User-level instructions (integer instructions).
    - **`ua`**: Atomic operations (A extension).
    - **`um`**: Multiplication and division instructions (M extension).
    - **`uz`**: Compressed instructions (Z extension).
    - **`uf`**: Floating-point operations (F extension).
    - **`uzbb`**: Bit manipulation (B extension).
    - **`uzba`**: Atomic bit manipulation.
    - **`uzbc`**: Cryptographic extension.

  - **Program-Level or Vector Tests**:
    - **`p`**: Indicates program-level tests (standard user instructions).
    - **`v`**: Indicates vector instruction set tests (vector extension).

  - **Mnemonic**: Represents the instruction or functionality being tested, such as:
    - `add`
    - `xor`
    - `fadd`
  
- **condor_isa_tests:** The `condor_isa_tests`  runs a collection of RISC-V ISA compliance tests specifically implemented by Condor. Unlike the `riscv_isa_test`, which may selectively run tests, the `condor_isa_tests` executes every test that has been implemented.

  To add new test, follow structure defined in `tests/isa_test_suite/condor_tests/isa` directory.

  This test can be run manually by navigating from dromajo directory:

  ```bash
  cd tests/isa_test_suite
  make
  bash run_condor_isa_test_suite.sh
  ```

- **andestar_tests:** This test runs a suite andestar tests: `addi`, `addigp`, `lwgp`, `lwugp`, `sdgp`.

# Semihosting

TDB
