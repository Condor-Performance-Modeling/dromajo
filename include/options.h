/*
 * Copyright (C) 2024, Jeff Nye
 *
 * This file is part of jnutils, made public 2023, (c) Jeff Nye
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

#pragma once
#include "riscv_machine.h"
#include <boost/program_options.hpp>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>

extern FILE* dromajo_stderr;
extern void usage_isa();
extern void usage_interactive();
extern void usage(const char *prog, const char *msg);

namespace po = boost::program_options;

struct BaseAndSize
{
  uint64_t base;
  uint64_t size;
};

struct Options
{
  // ----------------------------------------------------------------
  // singleton 
  // ----------------------------------------------------------------
  static Options* getInstance() {
    if(!instance) instance = new Options();
    return instance;
  }
  // ----------------------------------------------------------------
  // support methods
  // ----------------------------------------------------------------
  void build_options(po::options_description&,
                     po::options_description&,
                     po::options_description&,
                     po::options_description&,
                     po::options_description&,
                     po::options_description&,
                     po::options_description&,
                     po::positional_options_description&);

  bool check_options(po::variables_map&,
                     po::options_description&,
                     po::options_description&,
                     po::positional_options_description&,
                     bool);

  void setup_options(int,char**);

  void usage(po::options_description&,
             po::positional_options_description&);

  void validate(boost::any& v,
                const std::vector<std::string>& values,
                BaseAndSize*, int);

  void emit_version_info();
  void emit_credits_info();

  bool is_elf_file(const std::string& file_path);
  std::string detect_file_type(const std::string& file_path);

  // ----------------------------------------------------------------
  // place holders
  // ----------------------------------------------------------------
  void version();
  void query_options();
  // ----------------------------------------------------------------
  // ----------------------------------------------------------------

  std::string prog{""};
  std::string snapshot_load_name{""};
  std::string snapshot_save_name{""};
  std::string path{""};
  std::string cmdline{""};
  std::string simpoint_file{""};

  uint32_t    ncpus{0};
  uint64_t    maxinsns{0};
  uint64_t    heartbeat{0};

  uint64_t    exe_trace{UINT64_MAX};
  std::string exe_trace_log{""};

  bool        interactive{false};

  std::string stf_trace{""};
  bool        stf_exit_on_stop_opc{false};
  bool        stf_memrecord_size_in_bits{false};
  bool        stf_trace_register_state{false};
  bool        stf_disable_memory_records{false};
  std::string stf_priv_modes{"USHM"};
  bool        stf_force_zero_sha{false};
  bool        stf_insn_num_tracing{false};
  uint64_t    stf_insn_start{0};
  uint64_t    stf_insn_length{0};

  bool        simpoint_en_bbv{false};
  std::string simpoint_bb_file;
  uint64_t    simpoint_size{0};

  uint64_t    memory_size_override{0};
  uint64_t    memory_addr_override{0};
  bool        memory_addr_override_flag{false};

  bool        ignore_sbi_shutdown{false};
  bool        dump_memories{false};

  std::string bootrom_name{""};
  std::string dtb_name{""};

  bool        compact_bootrom{false};
  uint64_t    plic_base_addr{0};
  uint64_t    plic_base_addr_override{0};
  uint64_t    plic_size_override{0};

  uint64_t    clint_base_addr_override{0};
  uint64_t    clint_size_override{0};

  bool        custom_extension{false};
  bool        clear_ids{false};

//#ifdef LIVECACHE
//  uint64_t    live_cache_size{8*1024*1024};
//#endif

  bool elf_based{false};
  bool allow_ctrlc{false};
  bool show_enabled_extensions{false};

  //Control warning messages
  bool en_unimpl_csr_msg{false};

  std::string march_string{"rv64gc"};
  std::string cfgfile;
  std::string _xlens;
  std::string _vlens;

  std::vector<uint32_t> xlens;
  std::vector<uint32_t> vlens;

  uint32_t physical_addr_len{PHYSICAL_ADDR_LEN_DEFAULT};

  uint64_t ram_size{0};
  uint64_t ram_base_addr        {RAM_BASE_ADDR};
  uint64_t reset_vector_override{BOOT_BASE_ADDR};
 
  BaseAndSize plic_range {(uint64_t)PLIC_BASE_ADDR, 
                          (uint64_t)PLIC_SIZE};

  BaseAndSize clint_range{(uint64_t)CLINT_BASE_ADDR, 
                          (uint64_t)CLINT_SIZE};

  //This is only valid when -DLIVECACHE is supplied during compile
  uint64_t live_cache_size{0x800000};//8MB

  std::string positional_argument;

  static const std::string model_desc;
  // ----------------------------------------------------------------
  bool notify_error{false};

  po::variables_map vm;

  static Options *instance;
private:
  // ----------------------------------------------------------------
  // more singleton 
  // ----------------------------------------------------------------
  Options() {} //default
  Options(const Options&) = delete; //copy
  Options(Options&&)      = delete; //move
  Options& operator=(const Options&) = delete; //assignment
};

extern std::shared_ptr<Options> opts;

