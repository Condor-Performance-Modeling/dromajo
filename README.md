
# Dromajo - Esperanto Technology's RISC-V Reference Model

Functional verification is key to have a strong RISC-V ecosystem.
Esperanto is releasing Dromajo to help the RISC-V community.  Dromajo
is the Esperanto translation for an emu bird. It is a RISC-V RV64GC
emulator designed for RTL co-simulation.  This is the emulator used
for cosimulation inside Esperanto, but it is designed with a simple
API that can be leveraged to other RTL RISC-V cores.

Dromajo enables executing application (such as benchmarks running on
Linux) under fast software simulation, generating checkpoints after a
given number of cycles, and resuming such checkpoints for HW/SW
co-simulation.  This has proven to be a very powerful way to capture
bugs, especially in combination with randomized tests.

Dromajo's semantic model is based on Fabrice Bellard's RISCVEMU (later
renamed TinyEMU), but extensively verified, bug-fixed, and enhanced to
take it to ISA 2.3/priv 1.11.

## Building

```
mkdir build
cd build
# Debug build
cmake ..
# Release build Ofast compile option
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

The resulting artifacts are the `dromajo` simulator and the
`libdromajo_cosim.a` library with associated `dromajo_cosim.h`
header file.

Check the [setup.md](doc/setup.md) for instructions how to compile tests like
booting Linux and baremetal for dromajo.

## Usage

The co-simulation environment will link with the libraries and usage
will depend on that, but the `src/dromajo.c` utility allows for standalone
simulation of RISC-V ELF binaries.

```
./dromajo
error: missing config file
usage: ./dromajo [--load snapshot_name] [--save snapshot_name] [--maxinsns N] [--memory_size MB] config
       --load resumes a previously saved snapshot
       --save saves a snapshot upon exit
       --maxinsns terminates execution after a number of instructions
       --terminate-event name of the validate event to terminate execution
       --trace start trace dump after a number of instructions
       --memory_size sets the memory size in MiB (default 256 MiB)

./dromajo path/to/your/coremark.riscv
...
```

### Running Dromajo with custom Start of Program Memory

By default, Dromajo uses 0x80000000 as the start of program memory. If you need to specify a different start address, you can do so using the specific set of program options. Here's how you can run Dromajo with a custom start address for the program memory:

1. **Create your .elf with custom start address:**

Modify your linker script (`.ld`) to set the desired start address. For example, to set the start address to `0x20000000`, include the following line:

```ld
/* set loc counter */
. = 0x20000000;
```

2. **Modify and build Bootrom**

You need to  provide a custom bootrom via the `--bootrom` option to run dromajo with custom base address. Modify the Makefile in the bootrom subdirectory to your desired start address. Update the MEMORY_START variable in the Makefile, for example:

```Makefile
MEMORY_START=0x15000000
```

Bootrom uses `CC=$(RISCV)/bin/riscv64-unknown-elf-gcc` to build custom bootrom. `RISCV` environment veriable is not set by default so it must be set manually before building bootrom. To build the bootrom:

```bash
cd bootrom
make
```

This wil generate `bootrom.elf` file to be used in `--bootrom` program option.

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

### Running the Tests

Some test targets depend on submodules, before you start, use the following command in the top level directory:

```
git submodule update --init --recursive
```

To run the regression tests, use the following command in the top level directory:

```
mkdir build
cd build
# Debug build
cmake ..
# Release build Ofast compile option
cmake -DCMAKE_BUILD_TYPE=Release ..
make regress
```
