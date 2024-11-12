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
