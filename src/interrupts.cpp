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


#include "dromajo_protos.h"
#include "riscv_machine.h"

#include <cstdint>
#include <err.h>

static uint32_t plic_priority[PLIC_NUM_SOURCES + 1];  // XXX migrate to VirtMachine!

/* CLINT registers
 * 0000 msip hart 0
 * 0004 msip hart 1
 * 4000 mtimecmp hart 0 lo
 * 4004 mtimecmp hart 0 hi
 * 4008 mtimecmp hart 1 lo
 * 400c mtimecmp hart 1 hi
 * bff8 mtime lo
 * bffc mtime hi
 */

uint32_t clint_read(void *opaque, uint32_t offset, int size_log2) {
    RISCVMachine *m = (RISCVMachine *)opaque;
    uint32_t      val;

    if (0 <= offset && offset < 0x4000) {
        int hartid = offset >> 2;
        if (m->ncpus <= hartid) {
            vm_error("%s: MSIP access for hartid:%d which is beyond ncpus\n", __func__, hartid);
            val = 0;
        } else {
            val = (riscv_cpu_get_mip(m->cpu_state[hartid]) & MIP_MSIP) != 0;
        }
    } else if (offset == 0xbff8) {
        uint64_t mtime = m->cpu_state[0]->mcycle / RTC_FREQ_DIV;  // WARNING: mcycle may need to move to RISCVMachine
        val            = mtime;
    } else if (offset == 0xbffc) {
        uint64_t mtime = m->cpu_state[0]->mcycle / RTC_FREQ_DIV;
        val            = mtime >> 32;
    } else if (0x4000 <= offset && offset < 0xbff8) {
        int hartid = (offset - 0x4000) >> 3;
        if (m->ncpus <= hartid) {
            vm_error("%s: MSIP access for hartid:%d which is beyond ncpus\n", __func__, hartid);
            val = 0;
        } else if ((offset >> 2) & 1) {
            val = m->cpu_state[hartid]->timecmp >> 32;
        } else {
            val = m->cpu_state[hartid]->timecmp;
        }
    } else {
        vm_error("clint_read to unmanaged address CLINT_BASE+0x%x\n", offset);
        val = 0;
    }

#ifdef DUMP_CLINT
    vm_error("clint_read: offset=%x val=%x\n", offset, val);
#endif

    switch (size_log2) {
        case 1: val = val & 0xffff; break;
        case 2: val = val & 0xffffffff; break;
        case 3:
        default: break;
    }

    return val;
}

void clint_write(void *opaque, uint32_t offset, uint32_t val, int size_log2) {
    RISCVMachine *m = (RISCVMachine *)opaque;

    switch (size_log2) {
        case 1: val = val & 0xffff; break;
        case 2: val = val & 0xffffffff; break;
        case 3:
        default: break;
    }

    if (0 <= offset && offset < 0x4000) {
        int hartid = offset >> 2;
        if (m->ncpus <= hartid) {
            vm_error("%s: MSIP access for hartid:%d which is beyond ncpus\n", __func__, hartid);
        } else if (val & 1)
            riscv_cpu_set_mip(m->cpu_state[hartid], MIP_MSIP);
        else
            riscv_cpu_reset_mip(m->cpu_state[hartid], MIP_MSIP);
    } else if (offset == 0xbff8) {
        uint64_t mtime          = m->cpu_state[0]->mcycle / RTC_FREQ_DIV;  // WARNING: move mcycle to RISCVMachine
        mtime                   = (mtime & 0xFFFFFFFF00000000L) + val;
        m->cpu_state[0]->mcycle = mtime * RTC_FREQ_DIV;
    } else if (offset == 0xbffc) {
        uint64_t mtime          = m->cpu_state[0]->mcycle / RTC_FREQ_DIV;
        mtime                   = (mtime & 0x00000000FFFFFFFFL) + ((uint64_t)val << 32);
        m->cpu_state[0]->mcycle = mtime * RTC_FREQ_DIV;
    } else if (0x4000 <= offset && offset < 0xbff8) {
        int hartid = (offset - 0x4000) >> 3;
        if (m->ncpus <= hartid) {
            vm_error("%s: MSIP access for hartid:%d which is beyond ncpus\n", __func__, hartid);
        } else if ((offset >> 2) & 1) {
            m->cpu_state[hartid]->timecmp = (m->cpu_state[hartid]->timecmp & 0xffffffff) | ((uint64_t)val << 32);
            riscv_cpu_reset_mip(m->cpu_state[hartid], MIP_MTIP);
        } else {
            m->cpu_state[hartid]->timecmp = (m->cpu_state[hartid]->timecmp & ~0xffffffff) | val;
            riscv_cpu_reset_mip(m->cpu_state[hartid], MIP_MTIP);
        }
    } else {
        vm_error("clint_write to unmanaged address CLINT_BASE+0x%x\n", offset);
        val = 0;
    }

#ifdef DUMP_CLINT
    vm_error("clint_write: offset=%x val=%x\n", offset, val);
#endif
}

void plic_update_mip(RISCVMachine *s, int hartid) {
    uint32_t       mask = s->plic_pending_irq & ~s->plic_served_irq;
    RISCVCPUState *cpu  = s->cpu_state[hartid];

    for (int ctx = 0; ctx < 2; ++ctx) {
        unsigned mip_mask = ctx == 0 ? MIP_SEIP : MIP_MEIP;

        if (mask & cpu->plic_enable_irq[ctx]) {
            riscv_cpu_set_mip(cpu, mip_mask);
        } else {
            riscv_cpu_reset_mip(cpu, mip_mask);
        }
    }
}

uint32_t plic_read(void *opaque, uint32_t offset, int size_log2) {
    uint32_t      val = 0;
    RISCVMachine *s   = (RISCVMachine *)opaque;

    assert(size_log2 == 2);
    if (PLIC_PRIORITY_BASE <= offset && offset < PLIC_PRIORITY_BASE + (PLIC_NUM_SOURCES << 2)) {
        uint32_t irq = (offset - PLIC_PRIORITY_BASE) >> 2;
        assert(irq < PLIC_NUM_SOURCES);
        val = plic_priority[irq];
    } else if (PLIC_PENDING_BASE <= offset && offset < PLIC_PENDING_BASE + (PLIC_NUM_SOURCES >> 3)) {
        if (offset == PLIC_PENDING_BASE)
            val = s->plic_pending_irq;
        else
            val = 0;
    } else if (PLIC_ENABLE_BASE <= offset && offset < PLIC_ENABLE_BASE + (PLIC_ENABLE_STRIDE * MAX_CPUS)) {
        int addrid = (offset - PLIC_ENABLE_BASE) / PLIC_ENABLE_STRIDE;
        int hartid = addrid / 2;  // PLIC_HART_CONFIG is "MS"
        if (hartid < s->ncpus) {
            // uint32_t wordid = (offset & (PLIC_ENABLE_STRIDE-1)) >> 2;
            RISCVCPUState *cpu = s->cpu_state[hartid];
            val                = cpu->plic_enable_irq[addrid % 2];
        } else {
            val = 0;
        }
    } else if (PLIC_CONTEXT_BASE <= offset && offset < PLIC_CONTEXT_BASE + PLIC_CONTEXT_STRIDE * MAX_CPUS) {
        uint32_t hartid = (offset - PLIC_CONTEXT_BASE) / PLIC_CONTEXT_STRIDE;
        uint32_t wordid = (offset & (PLIC_CONTEXT_STRIDE - 1)) >> 2;
        if (wordid == 0) {
            val = 0;  // target_priority in qemu
        } else if (wordid == 1) {
            uint32_t mask = s->plic_pending_irq & ~s->plic_served_irq;
            if (mask != 0) {
                int i = ctz32(mask);
                s->plic_served_irq |= 1 << i;
                s->plic_pending_irq &= ~(1 << i);
                plic_update_mip(s, hartid);
                val = i;
            } else {
                val = 0;
            }
        }
    } else {
        vm_error("plic_read: unknown offset=%x\n", offset);
        val = 0;
    }
#ifdef DUMP_PLIC
    vm_error("plic_read: offset=%x val=%x\n", offset, val);
#endif

    return val;
}

void plic_write(void *opaque, uint32_t offset, uint32_t val, int size_log2) {
    RISCVMachine *s = (RISCVMachine *)opaque;

    assert(size_log2 == 2);
    if (PLIC_PRIORITY_BASE <= offset && offset < PLIC_PRIORITY_BASE + (PLIC_NUM_SOURCES << 2)) {
        uint32_t irq = (offset - PLIC_PRIORITY_BASE) >> 2;
        assert(irq < PLIC_NUM_SOURCES);
        plic_priority[irq] = val & 7;

    } else if (PLIC_PENDING_BASE <= offset && offset < PLIC_PENDING_BASE + (PLIC_NUM_SOURCES >> 3)) {
        vm_error("plic_write: INVALID pending write to offset=0x%x\n", offset);
    } else if (PLIC_ENABLE_BASE <= offset && offset < PLIC_ENABLE_BASE + PLIC_ENABLE_STRIDE * MAX_CPUS) {
        int addrid = (offset - PLIC_ENABLE_BASE) / PLIC_ENABLE_STRIDE;
        int hartid = addrid / 2;  // PLIC_HART_CONFIG is "MS"
        if (hartid < s->ncpus) {
            // uint32_t wordid = (offset & (PLIC_ENABLE_STRIDE - 1)) >> 2;
            RISCVCPUState *cpu   = s->cpu_state[hartid];
            cpu->plic_enable_irq[addrid % 2] = val;
        }
    } else if (PLIC_CONTEXT_BASE <= offset && offset < PLIC_CONTEXT_BASE + PLIC_CONTEXT_STRIDE * MAX_CPUS) {
        uint32_t hartid = (offset - PLIC_CONTEXT_BASE) / PLIC_CONTEXT_STRIDE;
        uint32_t wordid = (offset & (PLIC_CONTEXT_STRIDE - 1)) >> 2;
        if (wordid == 0) {
            plic_priority[wordid] = val;
        } else if (wordid == 1) {
            int irq = val & 31;
            uint32_t mask = 1 << irq;
            s->plic_served_irq &= ~mask;
        } else {
            vm_error("plic_write: hartid=%d ERROR?? unexpected wordid=%d offset=%x val=%x\n", hartid, wordid, offset, val);
        }
    } else {
        vm_error("plic_write: ERROR: unexpected offset=%x val=%x\n", offset, val);
    }
#ifdef DUMP_PLIC
    vm_error("plic_write: offset=%x val=%x\n", offset, val);
#endif
}

void plic_set_irq(void *opaque, int irq_num, int state) {
    RISCVMachine *m = (RISCVMachine *)opaque;

    uint32_t mask = 1 << irq_num;

    if (state)
        m->plic_pending_irq |= mask;
    else
        m->plic_pending_irq &= ~mask;

    for (int hartid = 0; hartid < m->ncpus; ++hartid) {
        plic_update_mip(m, hartid);
    }
}

