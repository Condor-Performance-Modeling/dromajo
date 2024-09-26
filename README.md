
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

A debug build is built by default.  To create a release build use -DCMAKE_BUILD_TYPE=Release. Dromajo uses `RISCV` and `RISCV_LINUX` environment variables to build some of the targets. `RISCV` and `RISCV_LINUX` environment variables are not set by default so it must be set manually before building Dromajo.

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

### Text trace generation example
Text to console using the sample elf.
```
<cd to the build directory>
./dromajo --ctrlc --trace 0 --march=rv64gc ../tests/elfs/bmi_sanity.bare.riscv
```

### STF trace generation example
STF trace file creation. The bmi_sanity elf supports tracing over specified regions.

```
<cd to the build directory>
./dromajo --ctrlc --stf_tracepoint --stf_priv_modes USHM --stf_trace=bmi_sanity.stf ../tests/elfs/bmi_sanity.bare.riscv
```

## STF trace options

Command line options are added to control STF trace generation.

```
    --stf_trace <filename> Dump an STF trace to the given file
                  Use .zstf as the file extension for compressed trace
                  output. Use .stf for uncompressed output
    --stf_exit_on_stop_opc Terminate the simulation after 
                  detecting a STOP_TRACE opcode. Using this
                  switch will disable non-contiguous region
                  tracing. The first STOP_TRACE opcode will 
                  terminate the simulator.
    --stf_memrecord_size_in_bits write memory access size in bits instead of bytes
    --stf_trace_register_state include register state in the STF
                   (default false)
    --stf_disable_memory_records Do not add memory records to 
                   STF trace. By default memory records are 
                   always traced.
                   (default false)
    --stf_tracepoint Enable tracepoint detection for STF trace 
                  generation.
                  ALWAYS ENABLED IN THIS VERSION.
    --stf_include_tracepoints Include the start/stop tracepoints 
                  in the STF trace
                  DISABLED IN THIS VERSION.
    --stf_priv_modes <USHM|USH|US|U> Specify which privilege 
                  modes to include for STF trace generation
    --stf_force_zero_sha Emit 0 for all SHA's in the STF header. This is a 
                  debug option. Also clears the dromajo version placed in
                  the STF header.
    --stf_essential_mode DEPRECATED. NO LONGER NECESSARY. 
                  The behavior controlled by this switch is now on by default.
                  See: --stf_disable_memory_records, --stf_trace_register_state
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

This project includes a set of unit tests and custom targets to ensure the reliability and correctness of the Dromajo. Tests are defined in the `tests` directory and can be run using CTest.

### Test Targets

We have defined two custom targets to manage the testing process:

1. **regress**: This target runs the regression tests against the simulator.
2. **update_test_files**: This target updates the test files after changes to the project sources.

Important Note: The update_test_files target does not update the golden files. These files need to be manually regenerated before running the regression tests again.

