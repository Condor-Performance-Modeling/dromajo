// -------------------------------------------------------------------------------
// Copyright (C) 2023-2024, Condor Computing Corporation
// -------------------------------------------------------------------------------
#pragma once
#include <string>
#include <unordered_set>
#include <unordered_map>

// List of known extension prefixes.
// Uncomment when extension is implemented.
// TODO: add future support for profiles
struct ExtensionFlags {
    // Base ISA 
    bool rv32  = false;
    bool rv64  = false;
    bool rv128 = false;

    // Single character extensions
    bool i = false;
    bool e = false;
    bool g = false;
    bool m = false;
    bool a = false;
    bool f = false;
    bool d = false;
    bool c = false;
//    bool h = false;
//    bool v = false;

    // Multi-character extensions
    bool zba = false;
    bool zbb = false;
    bool zbc = false;
    bool zbs = false;
//    bool zawrs = false;
//    bool zbkb = false;
//    bool zbkc = false;
//    bool zbkx = false;
//    bool zkne = false;
//    bool zknd = false;
//    bool zknh = false;
//    bool zkr = false;
//    bool zksed = false;
//    bool zksh = false;
//    bool zkt = false;
//    bool zk = false;
//    bool zkn = false;
//    bool zks = false;
//    bool zihintpause = false;
//    bool zicboz = false;
//    bool zicbom = false;
//    bool zicbop = false;
//    bool zfh = false;
//    bool zfhmin = false;
//    bool zicond = false;
//    bool zihintntl = false;
//    bool zicntr = false;
//    bool zihpm = false;
//    bool zca = false;
//    bool zcb = false;
//    bool smaia = false;
//    bool svinval = false;
//    bool svnapot = false;
//    bool svpbmt = false;
//    bool zve32x = false;
//    bool zve32f = false;
//    bool zve64x = false;
//    bool zve64f = false;
//    bool zve64d = false;
//    bool zvl32b = false;
//    bool zvl64b = false;
//    bool zvl128b = false;
//    bool zvl256b = false;
//    bool zvl512b = false;
//    bool zvl1024b = false;
//    bool zvl2048b = false;
//    bool zvl4096b = false;
//    bool zvbb = false;
//    bool zvbc = false;
//    bool zvkb = false;
//    bool zvkg = false;
//    bool zvkned = false;
//    bool zvknha = false;
//    bool zvknhb = false;
//    bool zvksed = false;
//    bool zvksh = false;
//    bool zvkn = false;
//    bool zvknc = false;
//    bool zvkng = false;
//    bool zvks = false;
//    bool zvksc = false;
//    bool zvksg = false;
//    bool zvkt = false;
//    bool zvfh = false;
//    bool zvfhmin = false;
};

extern std::unordered_map<std::string, bool ExtensionFlags::*> extensionMap;
extern std::unordered_map<char,bool ExtensionFlags::*> simpleExts;

extern void setExtensionFlags     (const std::string& input, ExtensionFlags &);
extern void printExtensionFlags   (const ExtensionFlags&,bool verbose);
extern bool parse_isa_string      (const char *march,ExtensionFlags&);
extern bool validateInitialSegment(const std::string&,ExtensionFlags&);
extern FILE *dromajo_stdout;
extern FILE *dromajo_stderr;
