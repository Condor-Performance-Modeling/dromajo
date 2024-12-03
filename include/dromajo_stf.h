/*
 * Contribution (C) 2023, various
 * Contribution (C) 2023-2024, Condor Computing
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


//
// STF gen protos.
//
#pragma once

#include "machine.h"
#include "riscv_cpu.h"
#include "trace_macros.h"
#include "stf-inc/stf_writer.hpp"
#include "stf-inc/stf_record_types.hpp"
#include "stf_lib_sha.h"

extern stf::STFWriter stf_writer;

extern void stf_trace_element(RISCVMachine*, int hartid, int priv,
                              uint64_t last_pc, uint32_t insn);
extern bool stf_trace_trigger(RISCVCPUState *s, uint64_t PC, uint32_t insn);
extern bool stf_trace_trigger_insn(RISCVCPUState *s, target_ulong PC);
extern void stf_record_state(RISCVMachine *m, int hartid, uint64_t last_pc);
extern void stf_trace_open(RISCVCPUState *s, target_ulong PC);
extern void stf_trace_close(RISCVCPUState *s, target_ulong PC);
