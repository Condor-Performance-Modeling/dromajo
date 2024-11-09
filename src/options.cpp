/*
 * Copyright (C) 2024, Jeff Nye, Condor Computing
 *
 * Licensed under the Apache License, Version 2.0 (the "License")
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * THIS FILE IS BASED ON THE RISCVEMU SOURCE CODE WHICH IS DISTRIBUTED
 * UNDER THE FOLLOWING LICENSE:
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "dromajo_sha.h"
#include "options.h"
#include "riscv_machine.h"

#include <cstdlib>
#include <iostream>
#include <fstream>
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
        (long)BOOT_BASE_ADDR,
        (long)RAM_BASE_ADDR,
        (long)PLIC_BASE_ADDR,
        (long)PLIC_SIZE,
        (long)CLINT_BASE_ADDR,
        (long)CLINT_SIZE);

    exit(EXIT_FAILURE);
}

// --------------------------------------------------------------------
const string Options::model_desc
                        = "Dromajo RISC-V Reference Model, Condor Fork";
// --------------------------------------------------------------------
// Build the option set and check the options
// --------------------------------------------------------------------
void Options::setup_options(int ac,char **av)
{
  notify_error = false;

  po::options_description visibleOpts(
//   string(model_desc +"\n" +
   string("\nUsage:: cpm.dromajo [--help|-h|--version|-v] {[options] "
                                                 "{positional_option}}")
  );

  po::options_description allOpts("All options");
  po::options_description stdOpts("Standard options");
  po::options_description isaOpts("Instruction/extension options");
  po::options_description stfOpts("STF options");
  po::options_description traceOpts("Trace log options");
  po::options_description cfgOpts("Configuration options");
  po::options_description hiddenOpts("Hidden options");

  //not implemented  po::options_description iniOpts("Config file options");
  po::positional_options_description posOpts;

  build_options(stdOpts,
                isaOpts,
                stfOpts,
                traceOpts,
                cfgOpts,
                hiddenOpts,posOpts);

  visibleOpts.add(stdOpts)
             .add(isaOpts)
             .add(stfOpts)
             .add(traceOpts)
             .add(cfgOpts);

  allOpts.add(visibleOpts).add(hiddenOpts);

  try {
    po::store(
      po::command_line_parser(ac, av)
          .options(allOpts).positional(posOpts).run(),vm
    );

    if (ac == 1) {
      usage(visibleOpts,posOpts);
      exit(0);
    }

    //Without positional option po::parse_command_line can be used
    //po::store(po::parse_command_line(ac, av, allOpts), vm);

  } catch(boost::program_options::error& e) {
    cout<<endl;
    cout<<"-E: 1st pass command line option parsing failed"<<endl;
    cout<<"-E: what: "<<e.what()<<endl;
    cout<<endl;
    usage(visibleOpts,posOpts);
    exit(1);
  }

  po::notify(vm);
  if(!check_options(vm,allOpts,visibleOpts,posOpts,true)) exit(1);
}
// --------------------------------------------------------------------
// Overload the input stream operator for BaseAndSize
// --------------------------------------------------------------------
std::istream& operator>>(std::istream& is, BaseAndSize& bs) {

  std::string input;
  is >> input;

  std::size_t colonPos = input.find(':');

  if (colonPos == std::string::npos) {
    throw po::validation_error(po::validation_error::invalid_option_value,
                               input, "Expected format START:SIZE");
  }

  try {
    bs.base = std::stoul(input.substr(0, colonPos), nullptr, 0);
    bs.size = std::stoul(input.substr(colonPos + 1), nullptr, 0);
  } catch (const std::exception&) {
    throw po::validation_error(po::validation_error::invalid_option_value,
                               input, "Invalid numeric values");
  }

  return is;
}
// --------------------------------------------------------------------
// --------------------------------------------------------------------
bool Options::is_elf_file(const std::string& file_path) {
    std::ifstream file(file_path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    // Check ELF magic numbers (first 4 bytes: 0x7F 'E' 'L' 'F')
    char magic[4];
    file.read(magic, 4);
    return (magic[0] == 0x7F && magic[1] == 'E' 
         && magic[2] == 'L'  && magic[3] == 'F');
}
// --------------------------------------------------------------------
// --------------------------------------------------------------------
std::string Options::detect_file_type(const std::string& file_path) {
    if (is_elf_file(file_path)) {
        return "ELF";
    } else if (file_path.find(".cfg") != std::string::npos
        || file_path.find(".conf") != std::string::npos) {
        return "CFG";
    } else {
        return "UNKNOWN";
    }
}
// --------------------------------------------------------------------
// Overload the output stream operator for BaseAndSize (for serialization)
// --------------------------------------------------------------------
std::ostream& operator<<(std::ostream& os, const BaseAndSize& bs) {
  os << "0x" << std::hex << std::setw(8) << std::setfill('0') << bs.base 
     << ":0x" << std::setw(8) << std::setfill('0') << bs.size;
  return os;
}
// --------------------------------------------------------------------
// Construct the std, hidden and positional option descriptions
// --------------------------------------------------------------------
void Options::validate(boost::any& v, const std::vector<std::string>& values,
                       BaseAndSize*, int)
{
    if (values.size() != 1) {
        throw po::validation_error(po::validation_error::invalid_option_value);
    }

    const std::string& input = values[0];
    std::size_t colonPos = input.find(':');

    if (colonPos == std::string::npos) {
        throw po::validation_error(
              po::validation_error::invalid_option_value, input,
              "Expected format START:SIZE");
    }

    try {
        std::string startStr = input.substr(0, colonPos);
        std::string sizeStr = input.substr(colonPos + 1);

        uint32_t start = std::stoul(startStr, nullptr, 0);
        uint32_t size = std::stoul(sizeStr, nullptr, 0);

        v = boost::any(BaseAndSize{start, size});

    } catch (const std::exception&) {
        throw po::validation_error(
              po::validation_error::invalid_option_value, input,
              "Invalid numeric values");
    }
}

// --------------------------------------------------------------------
// Construct the std/stf, hidden and positional option descriptions
// --------------------------------------------------------------------
void Options::build_options(po::options_description &stdOpts,
                            po::options_description &isaOpts,
                            po::options_description &stfOpts,
                            po::options_description &traceOpts,
                            po::options_description &cfgOpts,
                            po::options_description &hiddenOpts,
                            po::positional_options_description &posOpts)
{

  //Supply the help messages that include the defaults
// FIXME: This is the alternative method of doing this, keeping this for now
// until documented in jnutils
//
//  std::ostringstream plic_desc;
//  plic_desc <<"Set PLIC base/range as START:SIZE "
//            <<"(e.g. 0x1234:0x5678). Default is 0x"
//            <<hex<<plic_range.base<<":0x"<<plic_range.size;
//
//  std::ostringstream clint_desc;
//  clint_desc <<"Set CLINT base/range as START:SIZE "
//             <<"(e.g. 0x1234:0x5678). Default is 0x"
//             <<hex<<clint_range.base<<":0x"<<clint_range.size;

  std::ostringstream mem_start_desc;
  mem_start_desc <<"Sets the memory start address. "
					       <<"Default is 0x"<<hex<<RAM_BASE_ADDR;

  std::ostringstream reset_vector_desc;
  reset_vector_desc <<"Sets the memory start address. "
					          <<"Default is 0x"<<hex<<BOOT_BASE_ADDR;

  //Build the option set
  stdOpts.add_options()

    ("help,h", "...")

    ("help-march",
     "List the currently supported ISA extension set.")

    ("help-interactive",
     "List the interactive command set and exit. EXPERIMENTAL")

    ("version,v", "report version and exit")

    ("cmdline",
       po::value<string>(&cmdline),
       "Specify kernel command line arguments to append")

    ("ctrlc", 
       po::bool_switch(&allow_ctrlc)->default_value(false),
       "Allow control-c to exit simulation.")
  ;

  isaOpts.add_options()

    ("march",
       po::value<string>(&march_string),
       "Specify the architecture string to enable "
       "supported ISA extensions, default is rv64gc. "
       "Use --help-march to see currently supported set."
    )

    ("show-march",
     "This switch takes a complete option set and shows the "
     "enabled extensions, then exits.")

    ("custom_extension",
       po::bool_switch(&custom_extension)->default_value(false),
       "Set the custom extension bit in the misa in all cores")
  ;

  stfOpts.add_options()
    ("stf_trace",
       po::value<string>(&stf_trace), 
       "Dump an STF trace to the given file. Use .zstf as the file "
       "extension for compressed trace output. Use .stf for "
       "uncompressed output")

    ("stf_exit_on_stop_opc", 
     po::bool_switch(&stf_exit_on_stop_opc)->default_value(false),
       "Terminate the simulation after detecting a STOP_TRACE opcode. "
       "Using this switch will disable non-contiguous region tracing. "
       "The first STOP_TRACE opcode will terminate the simulator.")

    ("stf_memrecord_size_in_bits",
     po::bool_switch(&stf_memrecord_size_in_bits)->default_value(false),
       "write memory access size in bits instead of bytes in STF records")

    ("stf_trace_register_state",
     po::bool_switch(&stf_trace_register_state)->default_value(false),
       "Include register state in the STF output")

    ("stf_disable_memory_records", 
       po::bool_switch(&stf_disable_memory_records)->default_value(false),
       "Do not add memory records to STF trace. By default memory records "
       "are always traced (default false).")

    ("stf_priv_modes",
       po::value<string>(&stf_priv_modes), 
       "<USHM|USH|US|U> Specify which privilege modes to include for "
       "STF trace generation")

    ("stf_force_zero_sha", 
       po::bool_switch(&stf_force_zero_sha)->default_value(false),
       "Emit 0 for all SHA's in the STF header. This is a debug option. "
       "Also clears the dromajo version placed in the STF header used in "
       "regression results comparisons")
  ;

  traceOpts.add_options()

    ("trace",
      po::value<uint64_t>(&exe_trace),
     "Backward compatible alias for exe_trace")

    ("exe_trace",
       po::value<uint64_t>(&exe_trace), 
       "Start an exe trace dump after a number of instructions. Exe "
       "tracing is disabled by default")

    ("exe_trace_log",
       po::value<string>(&exe_trace_log), 
       "Write exe trace output to a file, Ignored without --exe_trace.")

    ("simpoint",
       po::value<string>(&simpoint_file),
       "Read a simpoint file to create multiple checkpoints")

    ("interactive", 
       po::bool_switch(&interactive)->default_value(false),
       "Initialize and enter interactive mode")

    ("ignore_sbi_shutdown", 
       po::bool_switch(&ignore_sbi_shutdown)->default_value(false),
       "...")

    ("load",
       po::value<string>(&snapshot_load_name),
       "Load a snapshot from a named file")

    ("save",
       po::value<string>(&snapshot_save_name),
       "Save a snapshot to named file")

    ("maxinsns",
       po::value<uint64_t>(&maxinsns),
       "Terminates execution after a number of instructions")

    ("dump_memories", 
       po::bool_switch(&dump_memories)->default_value(false),
       "dump memories that could be used to load a cosimulation")
  ;

  cfgOpts.add_options()

    ("ncpus",
       po::value<uint32_t>(&ncpus),
       "Number of cpus to simulate (default 1)")

    ("memory_size",
       po::value<uint64_t>(&memory_size_override), 
       "sets the memory size in MB, (default 256 MB)")

    ("memory_addr",
       po::value<uint64_t>(&memory_addr_override), 
        mem_start_desc.str().c_str())

    ("bootrom",
       po::value<string>(&bootrom_name), 
       "Load a bootrom imsg from file")

    ("compact_bootrom", 
       po::bool_switch(&compact_bootrom)->default_value(false),
       "...")

    ("reset_vector",
       po::value<uint64_t>(&reset_vector_override), 
        reset_vector_desc.str().c_str())

    ("dtb",
       po::value<string>(&dtb_name), 
       "Load a dtb file (default is dromajo dtb)")

    ("plic", po::value<BaseAndSize>(&plic_range)
                  ->default_value(plic_range)
                  ->value_name("START:SIZE"), 
     "Set PLIC base and size as START:SIZE (e.g., 0x1234:0x5678)")

    ("clint", po::value<BaseAndSize>(&clint_range)
                   ->default_value(clint_range)
                   ->value_name("START:SIZE"), 
     "Set CLINT base and size as START:SIZE (e.g., 0x1234:0x5678)")

    ("clear_ids", 
       po::bool_switch(&clear_ids)->default_value(false),
       "Clear mvendorid, marchid, mimpid for all cores")

    ("live_cache_size",po::value<uint64_t>(&live_cache_size),
     "Live cache warmup for checkpoint (default 8M). "
     "Dromajo must be compiled with -DLIVECACHE for this option to "
     "be valid")
  ; 

  //Add a placeholder for the positional option
  hiddenOpts.add_options()
    ("posopt",
       po::value<string>(&positional_argument), 
       "Capture the positional argument in a string")
  ;

  //Tell the vm that there is 1 positional argument
  //Add 1 positional option, either a CFG or an elf
  posOpts.add("posopt", 1);

}
// --------------------------------------------------------------------
// Check sanity on the options, handle --help, --version
// --------------------------------------------------------------------
bool Options::check_options(po::variables_map &vm,
                            po::options_description &allOpts,
                            po::options_description &visibleOpts,
                            po::positional_options_description &posOpts,
                            bool firstPass)
{
  if(firstPass) {
    if(vm.count("help"))    { usage(visibleOpts,posOpts); return false; }
    if(vm.count("version")) { version(); return false; }
  } else {
    //insert option checks for 2nd pass only, usually ini file
    //no ini file implemente so far.
  }

  bool ok = true;
  if(vm.count("live_cache_size")) {
    #ifndef LIVECACHE
    cout<<"-E: Dromajo must be compiled with -DLIVECACHE for "
        <<"--live_cache_size to have an effect"<<endl;
    ok = false;
    #endif
  }

//FIXME: more checks will be added


  return ok;
}
// --------------------------------------------------------------------
// --------------------------------------------------------------------
void Options::emit_credits_info()
{
  cout<<"  Copyright (c) 2016-2017 Fabrice Bellard"<<endl;
  cout<<"  Copyright (c) 2018,2019 Esperanto Technologies"<<endl;
  cout<<"  Copyright (c) 2023-2024 Condor Computing"<<endl;
}
// --------------------------------------------------------------------
// --------------------------------------------------------------------
void Options::emit_version_info()
{
  cout<<"  Dromajo version: "<<DROMAJO_VERSION_STRING<<endl;
  cout<<"  Dromajo SHA:     "<<DROMAJO_GIT_SHA<<endl;
  cout<<"  STF_LIB SHA:     "<<STF_LIB_GIT_SHA<<endl;
}
// --------------------------------------------------------------------
// --------------------------------------------------------------------
void Options::usage(po::options_description &opts,
                    po::positional_options_description &)
{
  cout<<endl<<model_desc<<endl;
  cout<<endl;
  emit_version_info();
  cout<<endl;
  emit_credits_info();
  cout<<endl;
  cout<<opts<<endl;
}
// --------------------------------------------------------------------
void Options::version()
{
  cout<<endl;
  cout<<model_desc<<endl;
  emit_version_info();
}

