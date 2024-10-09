// -----------------------------------------------------------------------------
// Copyright (C) 2023-2024, Condor Computing Corporation
// -----------------------------------------------------------------------------
#include "riscv_machine.h"
#include "dromajo_isa.h"
using namespace std;

//TODO: Test this first with various typo's etc in the -D
//#if XLEN == 32 || XLEN == 64 || XLEN == 128
//static uint32_t _XLEN = XLEN;
//#else
//#error unsupported XLEN
//#endif

// List of supported extensions
// See also dromajo_isa.h
std::unordered_map<char,bool ExtensionFlags::*> simpleExts = {
    {'i', &ExtensionFlags::i},
    {'e', &ExtensionFlags::e},
    {'g', &ExtensionFlags::g},
    {'m', &ExtensionFlags::m},
    {'a', &ExtensionFlags::a},
    {'f', &ExtensionFlags::f},
    {'d', &ExtensionFlags::d},
    {'c', &ExtensionFlags::c}
};

std::unordered_map<std::string, bool ExtensionFlags::*> extensionMap = {
    {"i", &ExtensionFlags::i},
    {"e", &ExtensionFlags::e},
    {"g", &ExtensionFlags::g},
    {"m", &ExtensionFlags::m},
    {"a", &ExtensionFlags::a},
    {"f", &ExtensionFlags::f},
    {"d", &ExtensionFlags::d},
    {"c", &ExtensionFlags::c},
    {"zba", &ExtensionFlags::zba},
    {"zbb", &ExtensionFlags::zbb},
    {"zbc", &ExtensionFlags::zbc},
    {"zbs", &ExtensionFlags::zbs}
};
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
void setExtensionFlags(const std::string& input, ExtensionFlags& flags)
{
    // Iterate over each character for single-character extensions
    for (char ch : input) {
        std::string ext(1, ch);
        if (extensionMap.find(ext) != extensionMap.end()) {
            flags.*extensionMap[ext] = true;
        }
    }

    // Iterate over each known multi-character extension
    for (const auto& [key, _] : extensionMap) {
        if (key.length() > 1 && input.find(key) != std::string::npos) {
            flags.*extensionMap[key] = true;
        }
    }
}
// -----------------------------------------------------------------------
// stderr because it is called from main
// -----------------------------------------------------------------------
void printExtensionFlags(const ExtensionFlags& flags,bool verbose) {

    fprintf(stderr,"Extension Flags State:\n");

    bool nothing_enabled = true;
    for (const auto& [key, flagPtr] : extensionMap) {
        if(flags.*flagPtr) nothing_enabled = false;
        if (verbose) {
            fprintf(dromajo_stderr, "  - %s: %s\n", key.c_str(),
                   (flags.*flagPtr ? "enabled" : "disabled"));
        } else if (flags.*flagPtr) {
            fprintf(dromajo_stderr, "  - %s: enabled\n", key.c_str());
        }
    }

    // This can happen if the command line over loads the default with
    // something like rv64 instead of rv64i, etc
    if(nothing_enabled) {
      fprintf(dromajo_stderr, "  - No extensions are enabled\n");
    }
}
// -----------------------------------------------------------------------
// Verify the common form and expand 'g' as needed
// -----------------------------------------------------------------------
bool validateInitialSegment(const std::string& segment,ExtensionFlags &flags)
{
    // Valid base prefixes
    std::unordered_set<std::string> validBases = {"rv32", "rv64", "rv128"};
    
    flags.rv32  = false;
    flags.rv64  = false;
    flags.rv128 = false;

    // Check if the segment starts with a valid base
    std::string base;
    if (segment.compare(0, 4, "rv32") == 0) {
        flags.rv32 = true;
        base = "rv32";
    } else if (segment.compare(0, 4, "rv64") == 0) {
        flags.rv64 = true;
        base = "rv64";
    } else if (segment.compare(0, 5, "rv128") == 0) {
        flags.rv128 = true;
        base = "rv128";
    } else {
        fprintf(dromajo_stderr,"Invalid base prefix. Must start "
                               "with 'rv32', 'rv64', or 'rv128'.\n");
        return false;
    }

    // Extract the remaining part after the base prefix
    std::string remaining = segment.substr(base.length());

    // Check if there is an underscore and remove it
    size_t underscorePos = remaining.find('_');
    if (underscorePos != std::string::npos) {
        // There is an underscore, ignore it for now
        remaining = remaining.substr(0, underscorePos);
    }

    for (char ch : remaining) {
        auto it = simpleExts.find(ch);
        if (it != simpleExts.end()) {
            // Set the corresponding flag in ExtensionFlags
            flags.*(it->second) = true;
            if(it->first == 'g') {
              flags.i = true;
              flags.m = true;
              flags.a = true;
              flags.f = true;
              flags.d = true;
            }
        } else {
            fprintf(dromajo_stderr,
                    "Invalid character in the initial segment: %c\n",ch);
            return false;
        }
    }

    return true;
}
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
bool parse_isa_string(const char *march,ExtensionFlags &flags)
{
  //Message will be emitted by sub-functions
  if(!validateInitialSegment(std::string(march),flags)) return false;
  setExtensionFlags(std::string(march), flags);
  return true;
}
