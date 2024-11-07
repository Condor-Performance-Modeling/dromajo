// -------------------------------------------------------------------------
// Copyright (C) 2023-2024, Jeff Nye, Condor Computing Corporation
// See dromajo_main.cpp for license notice.
// -------------------------------------------------------------------------
// Split from the original dromajo_main
// -------------------------------------------------------------------------
#include "dromajo_sha.h"
#include "options.h"
#include "options.h"
#include "riscv_machine.h"

#include <cstdlib>
#include <iostream>
using namespace std;

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

// --------------------------------------------------------------------
// Build the option set and check the options
// --------------------------------------------------------------------
void Options::setup_options(int ac,char **av)
{
  notify_error = false;

  po::options_description visibleOpts(
   "\nOpcode frequency analysis - opc_freq\n "
   "Usage:: test [--help|-h|--version|-v] { options }");

  po::options_description stdOpts("Standard options");
  build_options(stdOpts);

  try {
    po::store(
      po::command_line_parser(ac, av)
          .options(stdOpts).run(),vm
    );

    if (ac == 1) {
      usage(stdOpts);
      exit(0);
    }

    //Without positional option po::parse_command_line can be used
    //po::store(po::parse_command_line(ac, av, allOpts), vm);

  } catch(boost::program_options::error& e) {
//    msg->msg("");
//    msg->emsg("1st pass command line option parsing failed");
//    msg->emsg("What: " + string(e.what()));
//    usage(stdOpts);
    exit(1);
  }

  po::notify(vm);
  if(!check_options(vm,stdOpts,true)) exit(1);
}
// --------------------------------------------------------------------
// Construct the std, hidden and positional option descriptions
// --------------------------------------------------------------------
void Options::build_options(po::options_description &stdOpts)
{
  stdOpts.add_options()
    ("help,h", "...")

    ("version,v", "report version and exit")

//    //options used generally
//    ("isa_file", po::value<vector<string> >(&isa_files),
//     "Multiple --isa_file accepted")
//
//    //options for stf info mining
//    ("mnem_only", po::bool_switch(&mnem_only)->default_value(false),
//     "Emit only mnemonic info from STF")
//
//    ("stf_info", po::bool_switch(&stf_info)->default_value(false),
//     "Emit STF info")
//
//    //options for tuple sorting
//    ("sort_tuples", po::bool_switch(&sort_tuples)->default_value(false),
//     "Fusion analysis of STF")
//
//    ("stf_file",     po::value<string>(&stf_file),"in")
//    ("sorted_file,o",po::value<string>(&sorted_file),"sorted sequence file")
//    ("disasm_file",  po::value<string>(&disasm_file),"disassembled trace")
//    ("objdump_file", po::value<string>(&objdump_file),"results of objdump -D")
//    ("stats_file",   po::value<string>(&stats_file),"tbd")
//    ("search_file",  po::value<string>(&search_file),
//     "input for block search, file path must end in _search.py")
//    ("stack_access_file",  po::value<string>(&stack_access_file),
//     "stack access detection file")
//
//    ("redux_pct",    po::value<double>(&redux_pct),
//     "Minimum required cycle count reduction, "
//     "used to filter block search output")
//
//    ("redux_length", po::value<uint64_t>(&redux_length),
//     "Maximum sequence length, used to filter block search output")
//
//    ("seq_min_len", po::value<uint64_t>(&seq_min_len),
//     "Minimum sequence length")
//
//    ("max_records", po::value<int64_t>(&max_records),
//     "Maximum trace records to process")
//
//    ("seq_max_len", po::value<uint64_t>(&seq_max_len),
//     "Maximum sequence length")
//
//    ("seq_min_occur", po::value<uint64_t>(&seq_min_occurrence),
//     "Minimum sequence occurrence")
//
//    //options for filtering tuples
//    ("filter_tuples", po::bool_switch(&filter_tuples)->default_value(false),
//     "Filter fusion group tuples.")
//
//    ("input_json", po::value<vector<string> >(&input_json_files),
//     "Input, contains raw fusion tuples. Requires --filter_json.")
//
//    ("filtered_json", po::value<string>(&filtered_json_file),
//     "Output filtered fusion tuples. Requires --filter_json.")
//
//    //options for filtering stack accesses that are not LOAD/STORE
//    ("filter-nonldst", po::bool_switch(&filter_nonldst)->default_value(false),
//     "Exclude non-load/store instructions which access stack pointer.")
//
//    ("chunks", po::value<uint64_t>(&chunks)->default_value(1),
//     "Number of chunks for sequential cound processing. Default is 1.")
//
//    ("overlap", po::value<uint64_t>(&overlap)->default_value(0),
//     "Chunks overlap for sequential cound processing. Default is 0.")
  ;
}
// --------------------------------------------------------------------
// Check sanity on the options, handle --help, --version
// --------------------------------------------------------------------
bool Options::check_options(po::variables_map &vm,
                            po::options_description &stdOpts,
                            bool firstPass)
{
//  if(firstPass) {
//    if(vm.count("help"))    { usage(stdOpts); return false; }
//    if(vm.count("version")) { version(); return false; }
//  } else {
//    //insert option checks for 2nd pass only, usually ini file
//    //no ini file in this example so far.
//  }
//
//  //mode check
//  uint32_t more_than_one = static_cast<uint32_t>(filter_tuples)
//                         + static_cast<uint32_t>(sort_tuples)
//                         + static_cast<uint32_t>(stf_info);
//
//  if(more_than_one == 0) {
//    msg->emsg("At least one of --filter_tuples, --sort_tuples or --stf_info"
//              " must be set");
//    return false;
//  }
//
//  if(more_than_one > 1) {
//    msg->emsg("--filter_tuples, --sort_tuples and --stf_info are "
//              "exclusive flags"); 
//    msg->emsg("    --filter_tuples "+::to_string(filter_tuples));
//    msg->emsg("    --sort_tuples   "+::to_string(sort_tuples));
//    msg->emsg("    --stf_info      "+::to_string(stf_info));
//    return false;
//  }
//
//  if(!filter_tuples && search_file.find(".py") == string::npos) {
//    msg->emsg("search file name must end with .py "
//              "for proper import into python");
//    return false;
//  }
//
//  //filter_tuples mode
//  if(!input_json_files.empty() > 0 && !filter_tuples) {
//    msg->emsg("--input_json_files requires --filter_tuples");
//    return false;
//  }
//
//  if(filtered_json_file.length() > 0 && !filter_tuples) {
//    msg->emsg("--filtered_json requires --filter_tuples");
//    return false;
//  }
//
//  if(filter_tuples) {
//    if(filtered_json_file.length() == 0 
//       || input_json_files.empty())
//    {
//      msg->emsg("--filter_tuples requires both --input_json and"
//                " --filtered_json");
//      return false;
//    }
//  }
//
//  //this is a required option, not using ->required(),
//  //this message format is consistent with the others
//  if(isa_files.size() == 0) {
//    msg->emsg("At least one --isa_file option must be specified");
//    return false;
//  }
//  
//  // Validate chunks and overlap
//  if (chunks == 0) {
//      msg->emsg("The number of chunks (--chunks) must be greater than 0.");
//      return false;
//  }
//
//  if (overlap > seq_max_len) {
//      msg->emsg("--overlap cannot be greater than --seq_max_len.");
//      return false;
//  }
//
  return true;
}
// --------------------------------------------------------------------
// --------------------------------------------------------------------
void Options::usage(po::options_description &opts)
{
//  cout<<opts<<endl;
}
// --------------------------------------------------------------------
void Options::version()
{
//  msg->imsg("");
//  msg->imsg("Opcode usage frequency utility");
//  msg->imsg("Version: v"+std::string(VERSION));
//  msg->imsg("Slack jeff w/any questions");
//  msg->imsg("");
}

