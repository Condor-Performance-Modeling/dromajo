// -------------------------------------------------------------------------
// Copyright (C) 2023-2024, Condor Computing Corporation
// See dromajo_main.cpp for license notice.
// -------------------------------------------------------------------------
// Split from the original dromajo_main
// -------------------------------------------------------------------------
#include "dromajo_sha.h"
#include "options.h"
#include "riscv_machine.h"
#include <cstdlib>

void usage_isa()
{
  fprintf(dromajo_stderr,"\nSupported/Implemented ISA Extensions\n");
  for (const auto& [key, _] : extensionMap) {
      fprintf(stdout, "  -  %s\n", key.c_str());
  }
  exit(1);
}

void usage_interactive()
{
  fprintf(dromajo_stderr,"\nInteractive Command Help\n");
  fprintf(stdout, "Nothing so far\n");
  exit(1);
}

void usage(const char *prog, const char *msg) {
  fprintf(dromajo_stderr,
"\nmessage: %s\n\n"
"    Dromajo version:  %s\n"
"    Dromajo SHA:      %s\n"
"    STF_LIB SHA:      %s\n"
"\n"
"    Copyright (c) 2016-2017 Fabrice Bellard\n"
"    Copyright (c) 2018,2019 Esperanto Technologies\n"
"    Copyright (c) 2023-2024 Condor Computing\n"
"\n"
"usage: %s {options} [config|elf-file]\n\n"
"    --help              ...\n"
"    --help-march        List the currently supported ISA extension set.\n"
"    --help-interactive  List the interactive command set and exit.\n"

"\n"
"  ISA selection options EXPERIMENTAL\n"
"    --march <string>    Specify the architecture string to enable\n"
"                        supported ISA extensions, default is rv64gc.\n"
"                        Use --help-march to see currently supported set.\n"
"    --show-march        Takes a complete option set and shows the\n"
"                        enabled extensions. Then exits. \n"
"    --custom_extension  Set the custom extension bit in the misa \n"
"                        in all cores\n"
    
"\n"
"  STF options\n" 
"    --stf_trace <file> Dump an STF trace to the given file\n"
"                  Use .zstf as the file extension for compressed trace\n"
"                  output. Use .stf for uncompressed output\n"
"    --stf_exit_on_stop_opc Terminate the simulation after \n"
"                  detecting a STOP_TRACE opcode. Using this\n"
"                  switch will disable non-contiguous region\n"
"                  tracing. The first STOP_TRACE opcode will \n"
"                  terminate the simulator.\n"
"    --stf_memrecord_size_in_bits write memory access size in bits\n"
"                   instead of bytes\n"
"    --stf_trace_register_state include register state in the STF\n"
"                   (default false)\n"
"    --stf_disable_memory_records Do not add memory records to \n"
"                   STF trace. By default memory records are \n"
"                   always traced.\n"
"                   (default false)\n"
"    --stf_priv_modes <USHM|USH|US|U> Specify which privilege \n"
"                  modes to include for STF trace generation\n"
"    --stf_force_zero_sha Emit 0 for all SHA's in the STF header. This is a \n"
"                  debug option. Also clears the dromajo version placed in\n"
"                  the STF header.\n"

"\n"
"  Execution trace options\n"
"    --trace                Start trace dump after a number of instructions.\n"
"                           Alias for --exe_trace. Trace disabled by default\n"
"    --exe_trace            Start trace dump after a number of instructions.\n"
"                           Trace disabled by default\n"
"    --exe_trace_log <file> Write exe trace output to a file\n"
"                           Ignored without --exe_trace.\n"
"    --interactive          Initialize and enter interactive mode\n"

"\n"
"  Standard options\n"
"    --cmdline Kernel command line arguments to append\n"
"    --simpoint reads a simpoint file to create multiple checkpoints\n"
"    --ncpus number of cpus to simulate (default 1)\n"
"    --load resumes a previously saved snapshot\n"
"    --save saves a snapshot upon exit\n"
"    --maxinsns terminates execution after a number of instructions\n"
"    --terminate-event name of the validate event to terminate \n"
"                  execution\n"
"    --ignore_sbi_shutdown continue simulation even upon seeing \n"
"                  the SBI_SHUTDOWN call\n"
"    --dump_memories dump memories that could be used to load \n"
"                  a cosimulation\n"
"    --memory_size sets the memory size in MiB \n"
"                   (default 256 MiB)\n"
"    --memory_addr sets the memory start address \n"
"                   (default 0x%lx)\n"
"    --bootrom load in a bootrom img file \n"
"                   (default is dromajo bootrom)\n"
"    --dtb load in a dtb file (default is dromajo dtb)\n"
"    --compact_bootrom have dtb be directly after bootrom \n"
"                   (default 256B after boot base)\n"
"    --reset_vector set reset vector for all cores \n"
"                   (default 0x%lx)\n"
"    --plic START:SIZE set PLIC start address and size in B\n"
"                   (defaults to 0x%lx:0x%lx)\n"
"    --clint START:SIZE set CLINT start address and size in B\n"
"                   (defaults to 0x%lx:0x%lx)\n"
#ifdef LIVECACHE
"    --live_cache_size live cache warmup for checkpoint \n"
"                   (default 8M)\n"
#endif
"    --clear_ids clear mvendorid, marchid, mimpid for all cores\n\n"
        ,
        msg,
        DROMAJO_VERSION_STRING,
        DROMAJO_GIT_SHA,
        STF_LIB_GIT_SHA,
        prog,
        (long)RAM_BASE_ADDR,
        (long)RAM_BASE_ADDR,
        (long)PLIC_BASE_ADDR,
        (long)PLIC_SIZE,
        (long)CLINT_BASE_ADDR,
        (long)CLINT_SIZE);

    exit(EXIT_FAILURE);
}

