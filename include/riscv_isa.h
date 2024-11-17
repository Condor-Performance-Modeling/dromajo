/*
 * Contribution (C) 2024, Condor Computing
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
#include <memory>
#include <string>
#include <unordered_set>
#include <unordered_map>

// List of known extension prefixes.
// Uncomment when extension is implemented.
// TODO: add future support for profiles
// TODO: this needs to be reworked, this should
//       be unique to each CPU
// TODO: Add function that turns everything on
// TODO: make more use of the map, and add helper functions
// FIXME: enabling RV64, F64, and VLEN ?
struct IsaConfigFlags {

  // ----------------------------------------------------------------
  // singleton 
  // ----------------------------------------------------------------
  static IsaConfigFlags* getInstance() {
    if(!instance) instance = new IsaConfigFlags();
    return instance;
  }

//    bool has_zfh() { return f && flen > 0 && zfh; }
    bool has_zfh() { return true; }
    
    // Default EXT setting
    static constexpr bool DEFAULT_EXT_SETTING = true;

    // Base ISA 
    bool rv32{false};
    bool rv64{true};
    bool rv128{false};

    // XLEN
    uint32_t xlen{64};
    // FLEN
    uint32_t flen{64};
    // VLEN
    uint32_t vlen{64};

    // Single character extensions
    bool i{DEFAULT_EXT_SETTING};
    bool e{DEFAULT_EXT_SETTING};
    bool g{DEFAULT_EXT_SETTING};
    bool m{DEFAULT_EXT_SETTING};
    bool a{DEFAULT_EXT_SETTING};
    bool f{DEFAULT_EXT_SETTING};
    bool d{DEFAULT_EXT_SETTING};
    bool c{DEFAULT_EXT_SETTING};
//    bool h{DEFAULT_EXT_SETTING};
//    bool v{DEFAULT_EXT_SETTING};

    // Multi-character extensions
    bool zba{DEFAULT_EXT_SETTING};
    bool zbb{DEFAULT_EXT_SETTING};
    bool zbc{DEFAULT_EXT_SETTING};
    bool zbs{DEFAULT_EXT_SETTING};
//    bool zawrs{DEFAULT_EXT_SETTING};
//    bool zbkb{DEFAULT_EXT_SETTING};
//    bool zbkc{DEFAULT_EXT_SETTING};
//    bool zbkx{DEFAULT_EXT_SETTING};
//    bool zkne{DEFAULT_EXT_SETTING};
//    bool zknd{DEFAULT_EXT_SETTING};
//    bool zknh{DEFAULT_EXT_SETTING};
//    bool zkr{DEFAULT_EXT_SETTING};
//    bool zksed{DEFAULT_EXT_SETTING};
//    bool zksh{DEFAULT_EXT_SETTING};
//    bool zkt{DEFAULT_EXT_SETTING};
//    bool zk{DEFAULT_EXT_SETTING};
//    bool zkn{DEFAULT_EXT_SETTING};
//    bool zks{DEFAULT_EXT_SETTING};
//    bool zihintpause{DEFAULT_EXT_SETTING};
//    bool zicboz{DEFAULT_EXT_SETTING};
//    bool zicbom{DEFAULT_EXT_SETTING};
//    bool zicbop{DEFAULT_EXT_SETTING};
//    bool zfh{DEFAULT_EXT_SETTING};
//    bool zfhmin{DEFAULT_EXT_SETTING};
//    bool zicond{DEFAULT_EXT_SETTING};
//    bool zihintntl{DEFAULT_EXT_SETTING};
//    bool zicntr{DEFAULT_EXT_SETTING};
//    bool zihpm{DEFAULT_EXT_SETTING};
//    bool zca{DEFAULT_EXT_SETTING};
//    bool zcb{DEFAULT_EXT_SETTING};
//    bool smaia{DEFAULT_EXT_SETTING};
//    bool svinval{DEFAULT_EXT_SETTING};
//    bool svnapot{DEFAULT_EXT_SETTING};
//    bool svpbmt{DEFAULT_EXT_SETTING};
//    bool zve32x{DEFAULT_EXT_SETTING};
//    bool zve32f{DEFAULT_EXT_SETTING};
//    bool zve64x{DEFAULT_EXT_SETTING};
//    bool zve64f{DEFAULT_EXT_SETTING};
//    bool zve64d{DEFAULT_EXT_SETTING};
//    bool zvl32b{DEFAULT_EXT_SETTING};
//    bool zvl64b{DEFAULT_EXT_SETTING};
//    bool zvl128b{DEFAULT_EXT_SETTING};
//    bool zvl256b{DEFAULT_EXT_SETTING};
//    bool zvl512b{DEFAULT_EXT_SETTING};
//    bool zvl1024b{DEFAULT_EXT_SETTING};
//    bool zvl2048b{DEFAULT_EXT_SETTING};
//    bool zvl4096b{DEFAULT_EXT_SETTING};
//    bool zvbb{DEFAULT_EXT_SETTING};
//    bool zvbc{DEFAULT_EXT_SETTING};
//    bool zvkb{DEFAULT_EXT_SETTING};
//    bool zvkg{DEFAULT_EXT_SETTING};
//    bool zvkned{DEFAULT_EXT_SETTING};
//    bool zvknha{DEFAULT_EXT_SETTING};
//    bool zvknhb{DEFAULT_EXT_SETTING};
//    bool zvksed{DEFAULT_EXT_SETTING};
//    bool zvksh{DEFAULT_EXT_SETTING};
//    bool zvkn{DEFAULT_EXT_SETTING};
//    bool zvknc{DEFAULT_EXT_SETTING};
//    bool zvkng{DEFAULT_EXT_SETTING};
//    bool zvks{DEFAULT_EXT_SETTING};
//    bool zvksc{DEFAULT_EXT_SETTING};
//    bool zvksg{DEFAULT_EXT_SETTING};
//    bool zvkt{DEFAULT_EXT_SETTING};
    bool zfa {DEFAULT_EXT_SETTING};
    bool zfh {DEFAULT_EXT_SETTING};
//    bool zvfh{DEFAULT_EXT_SETTING};
//    bool zvfhmin{DEFAULT_EXT_SETTING};

  static IsaConfigFlags *instance;

private:
  // ----------------------------------------------------------------
  // more singleton 
  // ----------------------------------------------------------------
  IsaConfigFlags() {} //default
  IsaConfigFlags(const IsaConfigFlags&) = delete; //copy
  IsaConfigFlags(IsaConfigFlags&&)      = delete; //move
  IsaConfigFlags& operator=(const IsaConfigFlags&) = delete; //assignment
};

extern std::shared_ptr<IsaConfigFlags> isa_flags;

extern std::unordered_map<std::string, bool IsaConfigFlags::*> extensionMap;
extern std::unordered_map<char,bool IsaConfigFlags::*> simpleExts;

extern void setIsaConfigFlags     (const std::string& input, IsaConfigFlags&);
extern void printIsaConfigFlags   (const IsaConfigFlags&,bool verbose);
extern bool parse_isa_string      (const char *march,IsaConfigFlags&);
extern bool validateInitialSegment(const std::string&,IsaConfigFlags&);
extern FILE *dromajo_stdout;
extern FILE *dromajo_stderr;
