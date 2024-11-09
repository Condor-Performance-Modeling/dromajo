/*
 * Copyright (c) 2016-2017 Fabrice Bellard
 * Copyright (C) 2018,2019, Esperanto Technologies Inc.
 * Contribution (C) 2024, Jeff Nye
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

#include "riscv_machine.h"
#include "dromajo_protos.h"

void execution_trace(RISCVMachine *m,int hartid,uint32_t insn_raw) {
  RISCVCPUState *cpu = m->cpu_state[hartid];
  uint64_t last_pc  = virt_machine_get_pc(m, hartid);
  int      priv     = riscv_get_priv_level(cpu);

  fprintf(dromajo_trace, "%d %d 0x%016" PRIx64 " (0x%08x)",
          hartid, priv, last_pc,
          (insn_raw & 3) == 3 ? insn_raw : (uint16_t)insn_raw);

      int iregno = riscv_get_most_recently_written_reg(cpu);
      int fregno = riscv_get_most_recently_written_fp_reg(cpu);

      if (cpu->pending_exception != -1) {
          fprintf(dromajo_trace, " exception %d, tval %016" PRIx64,
                  cpu->pending_exception,
                  riscv_get_priv_level(cpu) == PRV_M ? cpu->mtval : cpu->stval);
      } else if (iregno > 0) {
          fprintf(dromajo_trace, " x%2d 0x%016" PRIx64,
                  iregno, virt_machine_get_reg(m, hartid, iregno));
      } else if (fregno >= 0) {
          fprintf(dromajo_trace, " f%2d 0x%016" PRIx64,
                  fregno, virt_machine_get_fpreg(m, hartid, fregno));
      } else {
          for (int i = 31; i >= 0; i--) {
              if (cpu->most_recently_written_vregs[i]) {
                  fprintf(dromajo_trace, " v%2d 0x", i);
                  for (int j = VLEN / 8 - 1; j >= 0; j--) {
                      fprintf(dromajo_stderr, "%02" PRIx8, cpu->v_reg[i][j]);
                  }
              }
          }
      }

      putc('\n', dromajo_trace);
}


