// --------------------------------------------------------------------------
// This file is part of jnutils, made public 2023, (c) Jeff Nye
// ---------------------------------------------------------------------------
#pragma once
#include <boost/program_options.hpp>
#include <cstdio>


extern FILE* dromajo_stderr;
extern void usage_isa();
extern void usage_interactive();
extern void usage(const char *prog, const char *msg);

namespace po = boost::program_options;
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
  void build_options(po::options_description&);

  bool check_options(po::variables_map&,
                     po::options_description&,
                     bool);

  void setup_options(int,char**);
  void usage(po::options_description&);

  // ----------------------------------------------------------------
  // place holders
  // ----------------------------------------------------------------
  void version();
  void query_options();
  // ----------------------------------------------------------------
//  std::vector<std::string> isa_files;
//
//  // ----------------------------------------------------------------
//  // Options for stf info
//  // ----------------------------------------------------------------
//  bool stf_info{false};
//  bool mnem_only{false};
//  // ----------------------------------------------------------------
//  // Options for fusion tuple sorting
//  // ----------------------------------------------------------------
//  bool sort_tuples{false};
//  std::string stf_file{""};
//  std::string sorted_file{""};
//  std::string disasm_file{""};
//  std::string objdump_file{""};
//  std::string stats_file{""};
//  std::string search_file{""};
//  std::string stack_access_file{""};
//
//  uint64_t seq_min_len{2};
//  uint64_t seq_max_len{16};
//  uint64_t seq_min_occurrence{1024};
//
//  uint64_t chunks{1};
//  uint64_t chunk_size{0};
//  uint64_t overlap{0};
//  std::vector<std::string> chunk_files;
//
//  double   redux_pct{5.0};
//  uint64_t redux_length{2};
//  int64_t  max_records{1000000};
//  int64_t  mem_limit_gb{45};
//
//  bool filter_nonldst{false};
//  // ----------------------------------------------------------------
//  // Options for fusion tuple filtering
//  // ----------------------------------------------------------------
//  bool filter_tuples{false};
//  std::vector<std::string> input_json_files;
//  std::string filtered_json_file{""};
//  // ----------------------------------------------------------------
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

