/*
 * RISCV machine
 *
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

#include "block_device.h"
#include "dromajo_protos.h"
#include "dw_apb_uart.h"
#include "elf64.h"
#include "options.h"
#include "riscv_machine.h"
#include "termio.h"
#include "virtio.h"

#include <cstdarg>
#include <err.h>
#include <getopt.h>

static uint64_t rtc_get_time(RISCVMachine *m) { return m->cpu_state[0]->mcycle / RTC_FREQ_DIV; }

void dromajo_default_error_log(int hartid, const char *fmt, ...) {
    va_list args;

    va_start(args, fmt);
    vfprintf(dromajo_stderr, fmt, args);
    va_end(args);
}

void dromajo_default_debug_log(int hartid, const char *fmt, ...) {
    va_list args;

    va_start(args, fmt);
    vfprintf(dromajo_stderr, fmt, args);
    va_end(args);
}

/* RISCV machine */
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

static uint32_t clint_read(void *opaque, uint32_t offset, int size_log2) {
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

static void clint_write(void *opaque, uint32_t offset, uint32_t val, int size_log2) {
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

static void plic_update_mip(RISCVMachine *s, int hartid) {
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

static uint32_t plic_priority[PLIC_NUM_SOURCES + 1];  // XXX migrate to VirtMachine!

static uint32_t plic_read(void *opaque, uint32_t offset, int size_log2) {
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

static void plic_write(void *opaque, uint32_t offset, uint32_t val, int size_log2) {
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

static void plic_set_irq(void *opaque, int irq_num, int state) {
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

static uint8_t *get_ram_ptr(RISCVMachine *s, uint64_t paddr) {
    PhysMemoryRange *pr = get_phys_mem_range(s->mem_map, paddr);
    if (!pr || !pr->is_ram)
        return NULL;
    return pr->phys_mem + (uintptr_t)(paddr - pr->addr);
}

/* FDT machine description */

#define FDT_MAGIC   0xd00dfeed
#define FDT_VERSION 17

struct fdt_header {
    uint32_t magic;
    uint32_t totalsize;
    uint32_t off_dt_struct;
    uint32_t off_dt_strings;
    uint32_t off_mem_rsvmap;
    uint32_t version;
    uint32_t last_comp_version; /* <= 17 */
    uint32_t boot_cpuid_phys;
    uint32_t size_dt_strings;
    uint32_t size_dt_struct;
};

struct fdt_reserve_entry {
    uint64_t address;
    uint64_t size;
};

#define FDT_BEGIN_NODE 1
#define FDT_END_NODE   2
#define FDT_PROP       3
#define FDT_NOP        4
#define FDT_END        9

typedef struct {
    uint32_t *tab;
    int       tab_len;
    int       tab_size;
    int       open_node_count;

    char *string_table;
    int   string_table_len;
    int   string_table_size;
} FDTState;

static FDTState *fdt_init(void) {
    FDTState *s = (FDTState *)mallocz(sizeof *s);
    return s;
}

static void fdt_alloc_len(FDTState *s, int len) {
    if (unlikely(len > s->tab_size)) {
        int new_size = max_int(len, s->tab_size * 3 / 2);
        s->tab       = (uint32_t *)realloc(s->tab, new_size * sizeof(uint32_t));
        s->tab_size  = new_size;
    }
}

static void fdt_put32(FDTState *s, int v) {
    fdt_alloc_len(s, s->tab_len + 1);
    s->tab[s->tab_len++] = cpu_to_be32(v);
}

/* the data is zero padded */
static void fdt_put_data(FDTState *s, const uint8_t *data, int len) {
    int len1 = (len + 3) / 4;
    fdt_alloc_len(s, s->tab_len + len1);
    memcpy(s->tab + s->tab_len, data, len);
    memset((uint8_t *)(s->tab + s->tab_len) + len, 0, -len & 3);
    s->tab_len += len1;
}

static void fdt_begin_node(FDTState *s, const char *name) {
    fdt_put32(s, FDT_BEGIN_NODE);
    fdt_put_data(s, (const uint8_t *)name, strlen(name) + 1);
    s->open_node_count++;
}

static void fdt_begin_node_num(FDTState *s, const char *name, uint64_t n) {
    char buf[256];
    snprintf(buf, sizeof(buf), "%s@%" PRIx64, name, n);
    fdt_begin_node(s, buf);
}

static void fdt_end_node(FDTState *s) {
    fdt_put32(s, FDT_END_NODE);
    s->open_node_count--;
}

static int fdt_get_string_offset(FDTState *s, const char *name) {
    int pos, new_size, name_size, new_len;

    pos = 0;
    while (pos < s->string_table_len) {
        if (!strcmp(s->string_table + pos, name))
            return pos;
        pos += strlen(s->string_table + pos) + 1;
    }
    /* add a new string */
    name_size = strlen(name) + 1;
    new_len   = s->string_table_len + name_size;
    if (new_len > s->string_table_size) {
        new_size             = max_int(new_len, s->string_table_size * 3 / 2);
        s->string_table      = (char *)realloc(s->string_table, new_size);
        s->string_table_size = new_size;
    }
    pos = s->string_table_len;
    memcpy(s->string_table + pos, name, name_size);
    s->string_table_len = new_len;
    return pos;
}

static void fdt_prop(FDTState *s, const char *prop_name, const char *data, int data_len) {
    fdt_put32(s, FDT_PROP);
    fdt_put32(s, data_len);
    fdt_put32(s, fdt_get_string_offset(s, prop_name));
    fdt_put_data(s, (const uint8_t *)data, data_len);
}

static void fdt_prop_tab_u32(FDTState *s, const char *prop_name, uint32_t *tab, int tab_len) {
    int i;
    fdt_put32(s, FDT_PROP);
    fdt_put32(s, tab_len * sizeof(uint32_t));
    fdt_put32(s, fdt_get_string_offset(s, prop_name));
    for (i = 0; i < tab_len; i++) fdt_put32(s, tab[i]);
}

static void fdt_prop_u32(FDTState *s, const char *prop_name, uint32_t val) { fdt_prop_tab_u32(s, prop_name, &val, 1); }

static void fdt_prop_u64(FDTState *s, const char *prop_name, uint64_t val) {
    uint32_t tab[2];
    tab[0] = val >> 32;
    tab[1] = val;
    fdt_prop_tab_u32(s, prop_name, tab, 2);
}

static void fdt_prop_tab_u64_2(FDTState *s, const char *prop_name, uint64_t v0, uint64_t v1) {
    uint32_t tab[4];
    tab[0] = v0 >> 32;
    tab[1] = v0;
    tab[2] = v1 >> 32;
    tab[3] = v1;
    fdt_prop_tab_u32(s, prop_name, tab, 4);
}

static void fdt_prop_str(FDTState *s, const char *prop_name, const char *str) { fdt_prop(s, prop_name, str, strlen(str) + 1); }

/* NULL terminated string list */
static void fdt_prop_tab_str(FDTState *s, const char *prop_name, ...) {
    va_list ap;

    va_start(ap, prop_name);
    int size = 0;
    for (;;) {
        char *ptr = va_arg(ap, char *);
        if (!ptr)
            break;
        int str_size = strlen(ptr) + 1;
        size += str_size;
    }
    va_end(ap);

    char *tab = (char *)malloc(size);
    va_start(ap, prop_name);
    size = 0;
    for (;;) {
        char *ptr = va_arg(ap, char *);
        if (!ptr)
            break;
        int str_size = strlen(ptr) + 1;
        memcpy(tab + size, ptr, str_size);
        size += str_size;
    }
    va_end(ap);

    fdt_prop(s, prop_name, tab, size);
    free(tab);
}

/* write the FDT to 'dst1'. return the FDT size in bytes */
int fdt_output(FDTState *s, uint8_t *dst) {
    struct fdt_header *       h;
    struct fdt_reserve_entry *re;
    int                       dt_struct_size;
    int                       dt_strings_size;
    int                       pos;

    assert(s->open_node_count == 0);

    fdt_put32(s, FDT_END);

    dt_struct_size  = s->tab_len * sizeof(uint32_t);
    dt_strings_size = s->string_table_len;

    h                    = (struct fdt_header *)dst;
    h->magic             = cpu_to_be32(FDT_MAGIC);
    h->version           = cpu_to_be32(FDT_VERSION);
    h->last_comp_version = cpu_to_be32(16);
    h->boot_cpuid_phys   = cpu_to_be32(0);
    h->size_dt_strings   = cpu_to_be32(dt_strings_size);
    h->size_dt_struct    = cpu_to_be32(dt_struct_size);

    pos = sizeof(struct fdt_header);

    h->off_dt_struct = cpu_to_be32(pos);
    memcpy(dst + pos, s->tab, dt_struct_size);
    pos += dt_struct_size;

    /* align to 8 */
    while ((pos & 7) != 0) {
        dst[pos++] = 0;
    }
    h->off_mem_rsvmap = cpu_to_be32(pos);
    re                = (struct fdt_reserve_entry *)(dst + pos);
    re->address       = 0; /* no reserved entry */
    re->size          = 0;
    pos += sizeof(struct fdt_reserve_entry);

    h->off_dt_strings = cpu_to_be32(pos);
    memcpy(dst + pos, s->string_table, dt_strings_size);
    pos += dt_strings_size;

    /* align to 8, just in case */
    while ((pos & 7) != 0) {
        dst[pos++] = 0;
    }

    h->totalsize = cpu_to_be32(pos);
    return pos;
}

void fdt_end(FDTState *s) {
    free(s->tab);
    free(s->string_table);
    free(s);
}

static int riscv_build_fdt(RISCVMachine *m, uint8_t *dst, const char *dtb_name, const char *cmd_line, uint64_t initrd_start,
                           uint64_t initrd_end) {
    FDTState *s = 0;
    int       size;
    if (!dtb_name) {
        int       intc_phandle = 0;
        int       max_xlen, i, cur_phandle;
        char      isa_string[128], *q;
        uint32_t  misa;
        uint32_t  tab[4 * MAX_CPUS];
        FBDevice *fb_dev;

        s = fdt_init();

        cur_phandle = 1;

        fdt_begin_node(s, "");
        fdt_prop_u32(s, "#address-cells", 2);
        fdt_prop_u32(s, "#size-cells", 2);
        fdt_prop_str(s, "compatible", "ucbbar,dromajo-bar_dev");
        fdt_prop_str(s, "model", "ucbbar,dromajo-bare");

        /* CPU list */
        fdt_begin_node(s, "cpus");
        fdt_prop_u32(s, "#address-cells", 1);
        fdt_prop_u32(s, "#size-cells", 0);
        fdt_prop_u32(s, "timebase-frequency", RTC_FREQ);

        int hartid2handle[MAX_CPUS];

        for (int hartid = 0; hartid < m->ncpus; ++hartid) {
            /* cpu */
            fdt_begin_node_num(s, "cpu", hartid);
            fdt_prop_str(s, "device_type", "cpu");
            fdt_prop_u32(s, "reg", hartid);
            fdt_prop_str(s, "status", "okay");
            fdt_prop_str(s, "compatible", "riscv");

            max_xlen = 64;
            misa     = riscv_cpu_get_misa(m->cpu_state[hartid]);
            q        = isa_string;
            q += snprintf(isa_string, sizeof(isa_string), "rv%d", max_xlen);
            for (i = 0; i < 26; ++i) {
                if (misa & (1 << i))
                    *q++ = 'a' + i;
            }
            *q = '\0';
            fdt_prop_str(s, "riscv,isa", isa_string);

            fdt_prop_str(s, "mmu-type", max_xlen <= 32 ? "riscv,sv32" : "riscv,sv48");
            fdt_prop_u32(s, "clock-frequency", CPU_FREQUENCY);

            fdt_begin_node(s, "interrupt-controller");
            fdt_prop_u32(s, "#interrupt-cells", 1);
            fdt_prop(s, "interrupt-controller", NULL, 0);
            fdt_prop_str(s, "compatible", "riscv,cpu-intc");
            intc_phandle          = cur_phandle++;
            hartid2handle[hartid] = intc_phandle;
            fdt_prop_u32(s, "phandle", intc_phandle);
            fdt_prop_u32(s, "linux,phandle", intc_phandle);
            fdt_end_node(s); /* interrupt-controller */

            fdt_end_node(s); /* cpu */
        }

        fdt_end_node(s); /* cpus */

        fdt_begin_node_num(s, "memory", m->ram_base_addr);
        fdt_prop_str(s, "device_type", "memory");
        tab[0] = m->ram_base_addr >> 32;
        tab[1] = m->ram_base_addr;
        tab[2] = m->ram_size >> 32;
        tab[3] = m->ram_size;
        fdt_prop_tab_u32(s, "reg", tab, 4);

        fdt_end_node(s); /* memory */

        fdt_begin_node(s, "soc");
        fdt_prop_u32(s, "#address-cells", 2);
        fdt_prop_u32(s, "#size-cells", 2);
        fdt_prop_tab_str(s, "compatible", "ucbbar,dromajo-bar-soc", "simple-bus", NULL);
        fdt_prop(s, "ranges", NULL, 0);

        fdt_begin_node_num(s, "clint", m->clint_base_addr);
        fdt_prop_str(s, "compatible", "riscv,clint0");

        for (int hartid = 0; hartid < m->ncpus; ++hartid) {
            tab[hartid * 4 + 0] = hartid2handle[hartid];
            tab[hartid * 4 + 1] = 3; /* M IPI irq */
            tab[hartid * 4 + 2] = hartid2handle[hartid];
            tab[hartid * 4 + 3] = 7; /* M timer irq */
        }

        fdt_prop_tab_u32(s, "interrupts-extended", tab, 4 * m->ncpus);

        fdt_prop_tab_u64_2(s, "reg", m->clint_base_addr, m->clint_size);

        fdt_end_node(s); /* clint */

        fdt_begin_node_num(s, "plic", m->plic_base_addr);
        fdt_prop_u32(s, "#interrupt-cells", 1);

        fdt_prop(s, "interrupt-controller", NULL, 0);
        fdt_prop_str(s, "compatible", "riscv,plic0");
        fdt_prop_u32(s, "riscv,ndev", 31);
        fdt_prop_tab_u64_2(s, "reg", m->plic_base_addr, m->plic_size);

        for (int hartid = 0; hartid < m->ncpus; ++hartid) {
            tab[hartid * 4 + 0] = hartid2handle[hartid];
            tab[hartid * 4 + 1] = 9; /* S ext irq */
            tab[hartid * 4 + 2] = hartid2handle[hartid];
            tab[hartid * 4 + 3] = 11; /* M ext irq */
        }

        fdt_prop_tab_u32(s, "interrupts-extended", tab, m->ncpus * 4);

        int plic_phandle = cur_phandle++;
        fdt_prop_u32(s, "phandle", plic_phandle);

        fdt_end_node(s); /* plic */

        for (i = 0; i < m->virtio_count; ++i) {
            fdt_begin_node_num(s, "virtio", VIRTIO_BASE_ADDR + i * VIRTIO_SIZE);
            fdt_prop_str(s, "compatible", "virtio,mmio");
            fdt_prop_tab_u64_2(s, "reg", VIRTIO_BASE_ADDR + i * VIRTIO_SIZE, VIRTIO_SIZE);
            tab[0] = plic_phandle;
            tab[1] = VIRTIO_IRQ + i;
            fdt_prop_tab_u32(s, "interrupts-extended", tab, 2);
            fdt_end_node(s); /* virtio */
        }

#ifdef USE_SIFIVE_UART
        // SiFive UART
        fdt_begin_node_num(s, "uart", UART0_BASE_ADDR);
        fdt_prop_str(s, "compatible", "sifive,uart0");
        fdt_prop_tab_u64_2(s, "reg", UART0_BASE_ADDR, UART0_SIZE);
        fdt_end_node(s); /* uart */
#endif

        for (unsigned uart_no = 0; uart_no < 2; ++uart_no) {
            uint64_t base_addr = uart_no == 0 ? DW_APB_UART0_BASE_ADDR : DW_APB_UART1_BASE_ADDR;
            // Fake Synopsys™ DesignWare™ ABP™ UART (NS16550 compatible)
            fdt_begin_node_num(s, "uart", base_addr);
            // interrupts = <0x0a>;
            // interrupt-parent = <0x09>;

            fdt_prop_tab_u64_2(s, "reg", base_addr, DW_APB_UART0_SIZE);
            fdt_prop_u32(s, "current-speed", 115200);
            fdt_prop_u32(s, "clock-frequency", 25000000);
            fdt_prop_u32(s, "reg-shift", 2);
            fdt_prop_u32(s, "reg-io-width", 4);
            // fdt_prop_str(s, "compatible", "snps,dw-apb-uart");
            fdt_prop_str(s, "compatible", "ns16550a");
            /*
            tab[0] = plic_phandle;
            tab[1] = DW_APB_UART0_IRQ;
            fdt_prop_tab_u32(s, "interrupts-extended", tab, 2);
            */

            fdt_prop_u32(s, "interrupt-parent", plic_phandle);
            fdt_prop_u32(s, "interrupts", uart_no == 0 ? DW_APB_UART0_IRQ : DW_APB_UART1_IRQ);
            fdt_end_node(s);
        }

        fb_dev = m->common.fb_dev;
        if (fb_dev) {
            fdt_begin_node_num(s, "framebuffer", FRAMEBUFFER_BASE_ADDR);
            fdt_prop_str(s, "compatible", "simple-framebuffer");
            fdt_prop_tab_u64_2(s, "reg", FRAMEBUFFER_BASE_ADDR, fb_dev->fb_size);
            fdt_prop_u32(s, "width", fb_dev->width);
            fdt_prop_u32(s, "height", fb_dev->height);
            fdt_prop_u32(s, "stride", fb_dev->stride);
            fdt_prop_str(s, "format", "a8r8g8b8");
            fdt_end_node(s); /* framebuffer */
        }

        fdt_end_node(s); /* soc */

        fdt_begin_node(s, "chosen");
        fdt_prop_str(s, "bootargs", cmd_line ? cmd_line : "");
        if (initrd_start && initrd_start < initrd_end) {
            fdt_prop_u64(s, "linux,initrd-start", initrd_start);
            fdt_prop_u64(s, "linux,initrd-end", initrd_end);
        }

        fdt_end_node(s); /* chosen */

        fdt_end_node(s); /* / */

        size = fdt_output(s, dst);
        fdt_end(s);
    } else {
        FILE *f = fopen(dtb_name, "rb");
        fseek(f, 0, SEEK_END);
        size = ftell(f);
        rewind(f);

        if (fread((char *)dst, 1, size, f) != (size_t)size) {
            vm_error("dromajo: %s: %s\n", dtb_name, strerror(errno));
            return -1;
        }

        fclose(f);
    }

#ifdef DUMP_DTB
    {
        FILE *f = fopen("dromajo.dtb", "wb");
        if (!f) {
            vm_error("dromajo: %s: %s\n", "dromajo.dtb", strerror(errno));
            return -1;
        }
        fwrite(dst, 1, size, f);
        fclose(f);
    }
#endif

    return size;
}

void load_elf_image(RISCVMachine *s, const uint8_t *image, size_t image_len) {
    Elf64_Ehdr *      ehdr = (Elf64_Ehdr *)image;
    const Elf64_Phdr *ph   = (Elf64_Phdr *)(image + ehdr->e_phoff);

    for (int i = 0; i < ehdr->e_phnum; ++i, ++ph)
        if (ph->p_type == PT_LOAD) {
            size_t rounded_size = ph->p_memsz;
            rounded_size        = (rounded_size + DEVRAM_PAGE_SIZE - 1) & ~(DEVRAM_PAGE_SIZE - 1);
            if (ph->p_vaddr == BOOT_BASE_ADDR) {
                if (s->bootrom_loaded) {
                    vm_error("dromajo: WARNING multiple bootrams; last wins");
                }
                s->bootrom_loaded = true;
            } else if (ph->p_vaddr != s->ram_base_addr)
                /* XXX This is a kludge to taper over the fact that cpu_register_ram will
                   happily allocate mapping covering existing mappings.  Unfortunately we
                   can't fix this without a substantial rewrite as the handling of IO devices
                   depends on this. */
                cpu_register_ram(s->mem_map, ph->p_vaddr, rounded_size, 0);
            memcpy(get_ram_ptr(s, ph->p_vaddr), image + ph->p_offset, ph->p_filesz);
        }
}

void load_hex_image(RISCVMachine *s, uint8_t *image, size_t image_len) {
    char *p = (char *)image;

    for (;;) {
        long unsigned offset = 0;
        unsigned data = 0;
        if (p[0] == '0' && p[1] == 'x')
          p += 2;
        char *nl = strchr(p, '\n');
        if (nl)
            *nl = 0;
        int n = sscanf(p, "%lx %x", &offset, &data);
        if (n != 2)
            break;
        uint32_t *mem = (uint32_t *)get_ram_ptr(s, offset);
        if (!mem)
          errx(1, "dromajo: can't load hex file, no memory at 0x%lx", offset);

        *mem = data;

        if (!nl)
            break;
        p = nl + 1;
    }
}

static int load_bootrom(RISCVMachine *s, const char *bootrom_name) {
    uint8_t * ram_ptr  = get_ram_ptr(s, ROM_BASE_ADDR);
    uint32_t *location = (uint32_t *)(ram_ptr + (BOOT_BASE_ADDR - ROM_BASE_ADDR));
    FILE *    f        = fopen(bootrom_name, "rb");

    if (!f) {
        vm_error("dromajo: %s: %s\n", bootrom_name, strerror(errno));
        return -1;
    }

    size_t len = fread((char *)location, 1, ~0U, f);

    fclose(f);

    return len;
}

static int generate_bootrom(RISCVMachine *s) {
    uint8_t * ram_ptr        = get_ram_ptr(s, ROM_BASE_ADDR);
    uint32_t *q              = (uint32_t *)(ram_ptr + (BOOT_BASE_ADDR - ROM_BASE_ADDR));
    int32_t   bootromSzBytes = 0;

    /*
     * RISCVEMU upon which Dromajo is based used to generate the boot
     * rom and existing clients have dependencies on the exact
     * contents, so this is delicate.  Reliance on this is depricated
     * and future client are encouraged to pass in the boot ram as an
     * argument.
     */

    if (s->ram_base_addr != 0x0080000000 && s->ram_base_addr != 0x8000000000 && s->ram_base_addr != 0xC000000000) {
        vm_error("Dromajo doesn't support BOOTROM generation for base address 0x%0" PRIx64
                 " please provide a custom bootrom via the --bootrom option or the bootrom"
                 " config parameter\n",
                 s->ram_base_addr);
        return -1;
    }

    // use the hardcoded bootrom
    /* KEEP THIS IN SYNC WITH THE TARGET BOOTROM */
    *q++ = 0xf1402573;  // start:  csrr   a0, mhartid
    if (s->ncpus == 1) {
        *q++ = 0x00050663;  //         beqz   a0, 1f
        *q++ = 0x10500073;  // 0:      wfi
        *q++ = 0xffdff06f;  //         j      0b
    } else {
        *q++ = 0x00000013;  // nop
        *q++ = 0x00000013;  // nop
        *q++ = 0x00000013;  // nop
    }
    *q++ = 0x00000597;  // 1:      auipc  a1, 0x0
    *q++ = 0x0f058593;  //         addi   a1, a1, 240 # _start + 256
    *q++ = 0x60300413;  //         li     s0, 1539
    *q++ = 0x7b041073;  //         csrw   dcsr, s0
    if (s->ram_base_addr == 0xC000000000) {
        *q++ = 0x0030041b;  //         addiw  s0, zero, 3
        *q++ = 0x02641413;  //         slli   s0, s0, 38
    } else {
        *q++ = 0x0010041b;  //         addiw  s0, zero, 1
        if (s->ram_base_addr == 0x80000000)
            *q++ = 0x01f41413;  //     slli s0, s0, 31
        else
            *q++ = 0x02741413;  //         slli   s0, s0, 39
    }
    *q++           = 0x7b141073;  //         csrw   dpc, s0
    *q++           = 0x7b200073;  //         dret
    bootromSzBytes = 13 * sizeof(uint32_t);

    return bootromSzBytes;
}

/* Return non-zero on failure */
static int copy_kernel(RISCVMachine *s, uint8_t *fw_buf, size_t fw_buf_len, const uint8_t *kernel_buf, size_t kernel_buf_len,
                       const uint8_t *initrd_buf, size_t initrd_buf_len, const char *bootrom_name, const char *dtb_name,
                       const char *cmd_line) {
    uint64_t initrd_end = 0;
    s->initrd_start     = 0;

    if (fw_buf_len > s->ram_size) {
        vm_error("Firmware too big\n");
        return 1;
    }

    // load firmware into ram
    if (elf64_is_riscv64(fw_buf, fw_buf_len)) {
        // XXX if the ELF is given in the config file, then we don't get to set memory base based on that.

        load_elf_image(s, fw_buf, fw_buf_len);
        uint64_t fw_entrypoint = elf64_get_entrypoint(fw_buf);
        if (!s->bootrom_loaded && fw_entrypoint != s->ram_base_addr) {
            fprintf(dromajo_stderr,
                    "DROMAJO currently requires a 0x%" PRIx64 " starting address, image assumes 0x%0" PRIx64 "\n",
                    s->ram_base_addr,
                    fw_entrypoint);
            return 1;
        }

        load_elf_image(s, fw_buf, fw_buf_len);
    } else if (fw_buf_len > 2 && fw_buf[0] == '0' && fw_buf[0] == 'x') {
        load_hex_image(s, fw_buf, fw_buf_len);
    } else
        memcpy(get_ram_ptr(s, s->ram_base_addr), fw_buf, fw_buf_len);

    // load kernel into ram
    if (kernel_buf && kernel_buf_len) {
        if (s->ram_size <= KERNEL_OFFSET) {
            vm_error("Can't load kernel at ram offset 0x%x\n", KERNEL_OFFSET);
            return 1;
        }
        if (kernel_buf_len > (s->ram_size - KERNEL_OFFSET)) {
            vm_error("Kernel too big\n");
            return 1;
        }
        memcpy(get_ram_ptr(s, s->ram_base_addr + KERNEL_OFFSET), kernel_buf, kernel_buf_len);
    }

    // load initrd into ram
    if (initrd_buf && initrd_buf_len) {
        if (initrd_buf_len > s->ram_size) {
            vm_error("Initrd too big\n");
            return 1;
        }
        initrd_end      = s->ram_base_addr + s->ram_size;
        s->initrd_start = initrd_end - initrd_buf_len;
        s->initrd_start = (s->initrd_start >> 12) << 12;
        memcpy(get_ram_ptr(s, s->initrd_start), initrd_buf, initrd_buf_len);
    }

    if (!s->bootrom_loaded) {
        if (bootrom_name) {
            if (load_bootrom(s, bootrom_name) < 0)
                return -1;
        } else {
            int32_t bootromSzBytes = generate_bootrom(s);

            if (bootromSzBytes < 0)
                return -1;

            // setup the dtb
            uint32_t fdt_off = (BOOT_BASE_ADDR - ROM_BASE_ADDR);
            if (s->compact_bootrom)
                fdt_off += bootromSzBytes;
            else
                fdt_off += 256;

            uint8_t *ram_ptr = get_ram_ptr(s, ROM_BASE_ADDR);
            if (riscv_build_fdt(s, ram_ptr + fdt_off, dtb_name, cmd_line, s->initrd_start, initrd_end) < 0)
                return -1;
        }
    }

    for (int i = 0; i < s->ncpus; ++i) riscv_set_debug_mode(s->cpu_state[i], TRUE);

    return 0;
}

static void riscv_flush_tlb_write_range(void *opaque, uint8_t *ram_addr, size_t ram_size) {
    RISCVMachine *s = (RISCVMachine *)opaque;
    for (int i = 0; i < s->ncpus; ++i) riscv_cpu_flush_tlb_write_range_ram(s->cpu_state[i], ram_addr, ram_size);
}

void virt_machine_set_defaults(VirtMachineParams *p) {
    memset(p, 0, sizeof *p);
    p->physical_addr_len = PHYSICAL_ADDR_LEN_DEFAULT;
    p->ram_base_addr     = RAM_BASE_ADDR;
    p->reset_vector      = BOOT_BASE_ADDR;
    p->plic_base_addr    = PLIC_BASE_ADDR;
    p->plic_size         = PLIC_SIZE;
    p->clint_base_addr   = CLINT_BASE_ADDR;
    p->clint_size        = CLINT_SIZE;
}

RISCVMachine *global_virt_machine = 0;
uint8_t       dromajo_get_byte_direct(uint64_t paddr) {
    assert(global_virt_machine);  // needed to have a global map
    uint8_t *ptr = get_ram_ptr(global_virt_machine, paddr);
    if (ptr == NULL)
        return 0;

    return *ptr;
}

static void dump_dram(RISCVMachine *s, FILE *f[16], const char *region, uint64_t start, uint64_t len) {
    if (len == 0)
        return;

    assert(start % 1024 == 0);

    uint64_t end = start + len;

    fprintf(stderr, "Dumping %-10s [%016lx; %016lx) %6.2f MiB\n", region, start, end, len / (1024 * 1024.0));

    /*
      Bytes
      0 ..31   memImage_dwrow0_even.hex:0-7
      32..63   memImage_dwrow1_even.hex:0-7
               memImage_dwrow2_even.hex:0-7
               memImage_dwrow3_even.hex:0-7
               memImage_derow0_even.hex:0-7
               memImage_derow1_even.hex:0-7
               memImage_derow2_even.hex:0-7
               memImage_derow3_even.hex:0-7
               memImage_dwrow0_odd.hex:0-7

               memImage_dwrow0_even.hex:8-15? (Not verified, but that would be logical)

      IOW,  16 banks of 64-bit wide memories, striped in cache sized (64B) blocks.  16 * 64 = 1 KiB


      @00000000 0053c5634143b383
    */

    for (int line = (start - s->ram_base_addr) / 1024; start < end; ++line) {
        for (int bank = 0; bank < 16; ++bank) {
            for (int word = 0; word < 8; ++word) {
                fprintf(f[bank],
                        "@%08x %016lx\n",
                        // Yes, this is mental
                        (line % 8) * 0x01000000 + line / 8 * 8 + word,
                        *(uint64_t *)get_ram_ptr(s, start));
                start += sizeof(uint64_t);
            }
        }
    }
}

RISCVMachine *virt_machine_init(const VirtMachineParams *p) {
    VIRTIODevice *blk_dev;
    int           irq_num, i;
    VIRTIOBusDef  vbus_s, *vbus = &vbus_s;
    RISCVMachine *s = (RISCVMachine *)mallocz(sizeof *s);

    s->ram_size      = p->ram_size;
    s->ram_base_addr = p->ram_base_addr;

    s->mem_map = phys_mem_map_init();
    /* needed to handle the RAM dirty bits */
    s->mem_map->opaque                = s;
    s->mem_map->flush_tlb_write_range = riscv_flush_tlb_write_range;
    s->common.maxinsns                = p->maxinsns;
    s->common.snapshot_load_name      = p->snapshot_load_name;

    /* loggers are changed using install_new_loggers() in dromajo_cosim */
    s->common.debug_log = &dromajo_default_debug_log;
    s->common.error_log = &dromajo_default_error_log;

    s->ncpus = p->ncpus;

    /* setup reset vector for core
     * note: must be above riscv_cpu_init
     */
    s->reset_vector = p->reset_vector;

    /* have compact bootrom */
    s->compact_bootrom = p->compact_bootrom;

    /* add custom extension bit to misa */
    s->custom_extension = p->custom_extension;

    s->plic_base_addr  = p->plic_base_addr;
    s->plic_size       = p->plic_size;
    s->clint_base_addr = p->clint_base_addr;
    s->clint_size      = p->clint_size;
    /* clear mimpid, marchid, mvendorid */
    s->clear_ids = p->clear_ids;

    if (MAX_CPUS < s->ncpus) {
        vm_error("ERROR: ncpus:%d exceeds maximum MAX_CPU\n", s->ncpus);
        return NULL;
    }

    for (int i = 0; i < s->ncpus; ++i) {
        s->cpu_state[i] = riscv_cpu_init(s, i);
    }

    /* RAM */
    cpu_register_ram(s->mem_map, s->ram_base_addr, s->ram_size, 0);
    cpu_register_ram(s->mem_map, ROM_BASE_ADDR, ROM_SIZE, 0);

    for (int i = 0; i < s->ncpus; ++i) {
        s->cpu_state[i]->physical_addr_len = p->physical_addr_len;
    }

    SiFiveUARTState *uart = (SiFiveUARTState *)calloc(sizeof *uart, 1);
    uart->irq             = UART0_IRQ;
    uart->cs              = p->console;
    cpu_register_device(s->mem_map, UART0_BASE_ADDR, UART0_SIZE, uart, uart_read, uart_write, DEVIO_SIZE32);

    DW_apb_uart_state *dw_apb_uart = (DW_apb_uart_state *)calloc(sizeof *dw_apb_uart, 1);
    dw_apb_uart->irq               = &s->plic_irq[DW_APB_UART0_IRQ];
    dw_apb_uart->cs                = p->console;
    cpu_register_device(s->mem_map,
                        DW_APB_UART0_BASE_ADDR,
                        DW_APB_UART0_SIZE,
                        dw_apb_uart,
                        dw_apb_uart_read,
                        dw_apb_uart_write,
                        DEVIO_SIZE32 | DEVIO_SIZE16 | DEVIO_SIZE8);

    DW_apb_uart_state *dw_apb_uart1 = (DW_apb_uart_state *)calloc(sizeof *dw_apb_uart, 1);
    dw_apb_uart1->irq               = &s->plic_irq[DW_APB_UART1_IRQ];
    dw_apb_uart1->cs                = p->console;
    cpu_register_device(s->mem_map,
                        DW_APB_UART1_BASE_ADDR,
                        DW_APB_UART1_SIZE,
                        dw_apb_uart1,
                        dw_apb_uart_read,
                        dw_apb_uart_write,
                        DEVIO_SIZE32 | DEVIO_SIZE16 | DEVIO_SIZE8);

    cpu_register_device(s->mem_map,
                        p->clint_base_addr,
                        p->clint_size,
                        s,
                        clint_read,
                        clint_write,
                        DEVIO_SIZE32 | DEVIO_SIZE16 | DEVIO_SIZE8);
    cpu_register_device(s->mem_map, p->plic_base_addr, p->plic_size, s, plic_read, plic_write, DEVIO_SIZE32);

    for (int j = 1; j < 32; j++) {
        irq_init(&s->plic_irq[j], plic_set_irq, s, j);
    }

    s->htif_tohost_addr = p->htif_base_addr;

    s->common.console = p->console;

    memset(vbus, 0, sizeof(*vbus));
    vbus->mem_map = s->mem_map;
    vbus->addr    = VIRTIO_BASE_ADDR;
    irq_num       = VIRTIO_IRQ;

    /* virtio console */
    if (p->console && 0) {
        vbus->irq             = &s->plic_irq[irq_num];
        s->common.console_dev = virtio_console_init(vbus, p->console);
        vbus->addr += VIRTIO_SIZE;
        irq_num++;
        s->virtio_count++;
    }

    /* virtio net device */
    for (i = 0; i < p->eth_count; ++i) {
        vbus->irq = &s->plic_irq[irq_num];
        virtio_net_init(vbus, p->tab_eth[i].net);
        s->common.net = p->tab_eth[i].net;
        vbus->addr += VIRTIO_SIZE;
        irq_num++;
        s->virtio_count++;
    }

    /* virtio block device */
    for (i = 0; i < p->drive_count; ++i) {
        vbus->irq = &s->plic_irq[irq_num];
        blk_dev   = virtio_block_init(vbus, p->tab_drive[i].block_dev);
        (void)blk_dev;
        vbus->addr += VIRTIO_SIZE;
        irq_num++;
        s->virtio_count++;
        // virtio_set_debug(blk_dev, 1);
    }

    /* virtio filesystem */
    for (i = 0; i < p->fs_count; ++i) {
        VIRTIODevice *fs_dev;
        vbus->irq = &s->plic_irq[irq_num];
        fs_dev    = virtio_9p_init(vbus, p->tab_fs[i].fs_dev, p->tab_fs[i].tag);
        (void)fs_dev;
        vbus->addr += VIRTIO_SIZE;
        irq_num++;
        s->virtio_count++;
    }

    if (p->input_device) {
        if (!strcmp(p->input_device, "virtio")) {
            vbus->irq       = &s->plic_irq[irq_num];
            s->keyboard_dev = virtio_input_init(vbus, VIRTIO_INPUT_TYPE_KEYBOARD);
            vbus->addr += VIRTIO_SIZE;
            irq_num++;
            s->virtio_count++;

            vbus->irq    = &s->plic_irq[irq_num];
            s->mouse_dev = virtio_input_init(vbus, VIRTIO_INPUT_TYPE_TABLET);
            vbus->addr += VIRTIO_SIZE;
            irq_num++;
            s->virtio_count++;
        } else {
            vm_error("unsupported input device: %s\n", p->input_device);
            return NULL;
        }
    }

    if (!p->files[VM_FILE_BIOS].buf) {
        vm_error("No bios given\n");
        return NULL;
    } else if (copy_kernel(s,
                           p->files[VM_FILE_BIOS].buf,
                           p->files[VM_FILE_BIOS].len,
                           p->files[VM_FILE_KERNEL].buf,
                           p->files[VM_FILE_KERNEL].len,
                           p->files[VM_FILE_INITRD].buf,
                           p->files[VM_FILE_INITRD].len,
                           p->bootrom_name,
                           p->dtb_name,
                           p->cmdline))
        return NULL;

    /* interrupts and exception setup for cosim */
    s->common.cosim             = false;
    s->common.pending_exception = -1;
    s->common.pending_interrupt = -1;

    /* plic/clint setup */
    s->plic_base_addr  = p->plic_base_addr;
    s->plic_size       = p->plic_size;
    s->clint_base_addr = p->clint_base_addr;
    s->clint_size      = p->clint_size;

    return s;
}

RISCVMachine *virt_machine_load(const VirtMachineParams *p, RISCVMachine *s) {
    if (!p->files[VM_FILE_BIOS].buf) {
        vm_error("No bios given\n");
        return NULL;
    } else if (copy_kernel(s,
                           p->files[VM_FILE_BIOS].buf,
                           p->files[VM_FILE_BIOS].len,
                           p->files[VM_FILE_KERNEL].buf,
                           p->files[VM_FILE_KERNEL].len,
                           p->files[VM_FILE_INITRD].buf,
                           p->files[VM_FILE_INITRD].len,
                           p->bootrom_name,
                           p->dtb_name,
                           p->cmdline))
        return NULL;

    if (p->dump_memories) {
        FILE *f = fopen("BootRAM.hex", "w+");
        if (!f) {
            vm_error("dromajo: %s: %s\n", "BootRAM.hex", strerror(errno));
            return NULL;
        }

        uint8_t *ram_ptr = get_ram_ptr(s, ROM_BASE_ADDR);
        for (int i = 0; i < ROM_SIZE / 4; ++i) {
            uint32_t *q_base = (uint32_t *)(ram_ptr + (BOOT_BASE_ADDR - ROM_BASE_ADDR));
            fprintf(f, "@%06x %08x\n", i, q_base[i]);
        }

        fclose(f);

        {
            FILE *f[16] = {0};

            char hexname[60];
            for (int i = 0; i < 16; ++i) {
                snprintf(hexname, sizeof hexname, "memImage_d%crow%d_%s.hex", "we"[i / 4 % 2], i % 4, i / 8 == 0 ? "even" : "odd");
                f[i] = fopen(hexname, "w");
                if (!f[i]) {
                    vm_error("dromajo: %s: %s\n", hexname, strerror(errno));
                    return NULL;
                }
            }

            dump_dram(s, f, "firmware", s->ram_base_addr, p->files[VM_FILE_BIOS].len);
            dump_dram(s, f, "kernel", s->ram_base_addr + KERNEL_OFFSET, p->files[VM_FILE_KERNEL].len);
            dump_dram(s, f, "initrd", s->initrd_start, p->files[VM_FILE_INITRD].len);

            for (int i = 0; i < 16; ++i) {
                fclose(f[i]);
            }
        }
    }

    global_virt_machine = s;

    return s;
}

void virt_machine_end(RISCVMachine *s) {
    if (s->common.snapshot_save_name)
        virt_machine_serialize(s, s->common.snapshot_save_name);

    /* XXX: stop all */
    for (int i = 0; i < s->ncpus; ++i) {
        riscv_cpu_end(s->cpu_state[i]);
    }

    phys_mem_map_end(s->mem_map);
    free(s);
}

void virt_machine_serialize(RISCVMachine *m, const char *dump_name) {
    RISCVCPUState *s = m->cpu_state[0];  // FIXME: MULTICORE

    vm_error("plic: %x %x timecmp=%llx\n", m->plic_pending_irq, m->plic_served_irq, (unsigned long long)s->timecmp);

    assert(m->ncpus == 1);  // FIXME: riscv_cpu_serialize must be patched for multicore
    riscv_cpu_serialize(s, dump_name, m->clint_base_addr);
}

void virt_machine_deserialize(RISCVMachine *m, const char *dump_name) {
    RISCVCPUState *s = m->cpu_state[0];  // FIXME: MULTICORE

    assert(m->ncpus == 1);  // FIXME: riscv_cpu_serialize must be patched for multicore
    riscv_cpu_deserialize(s, dump_name);
}

int virt_machine_get_sleep_duration(RISCVMachine *m, int hartid, int ms_delay) {
    RISCVCPUState *s = m->cpu_state[hartid];
    int64_t        ms_delay1;

    /* wait for an event: the only asynchronous event is the RTC timer */
    if (!(riscv_cpu_get_mip(s) & MIP_MTIP) && rtc_get_time(m) > 0) {
        ms_delay1 = s->timecmp - rtc_get_time(m);
        if (ms_delay1 <= 0) {
            riscv_cpu_set_mip(s, MIP_MTIP);
            ms_delay = 0;
        } else {
            /* convert delay to ms */
            ms_delay1 = ms_delay1 / (RTC_FREQ / 1000);
            if (ms_delay1 < ms_delay)
                ms_delay = ms_delay1;
        }
    }

    if (!riscv_cpu_get_power_down(s))
        ms_delay = 0;

    return ms_delay;
}

uint64_t virt_machine_get_pc(RISCVMachine *s, int hartid) { return riscv_get_pc(s->cpu_state[hartid]); }

uint64_t virt_machine_get_reg(RISCVMachine *s, int hartid, int rn) { return riscv_get_reg(s->cpu_state[hartid], rn); }

uint64_t virt_machine_get_fpreg(RISCVMachine *s, int hartid, int rn) { return riscv_get_fpreg(s->cpu_state[hartid], rn); }

const char *virt_machine_get_name(void) { return "riscv64"; }

void vm_send_key_event(RISCVMachine *s, BOOL is_down, uint16_t key_code) {
    if (s->keyboard_dev) {
        virtio_input_send_key_event(s->keyboard_dev, is_down, key_code);
    }
}

BOOL vm_mouse_is_absolute(RISCVMachine *s) { return TRUE; }

void vm_send_mouse_event(RISCVMachine *s, int dx, int dy, int dz, unsigned buttons) {
    if (s->mouse_dev) {
        virtio_input_send_mouse_event(s->mouse_dev, dx, dy, dz, buttons);
    }
}


RISCVMachine *virt_machine_main(int argc, char **argv) {
    const char *prog                = argv[0];
    char *      snapshot_load_name  = 0;
    char *      snapshot_save_name  = 0;
    const char *path                = NULL;
    const char *cmdline             = NULL;
    long        ncpus               = 0;
    uint64_t    maxinsns            = 0;

    uint64_t    exe_trace           = UINT64_MAX;
    const char *exe_trace_file_name = nullptr;
    bool        interactive         = false;

    const char *stf_trace                  = nullptr;
    bool        stf_exit_on_stop_opc       = false;
    bool        stf_memrecord_size_in_bits = false;
    bool        stf_trace_register_state   = false;
    bool        stf_disable_memory_records = false;
    const char *stf_priv_modes             = "USHM";
    bool        stf_force_zero_sha         = false;

    long        memory_size_override      = 0;
    uint64_t    memory_addr_override      = 0;
    bool        memory_addr_override_flag = false;
    bool        ignore_sbi_shutdown       = false;
    bool        dump_memories             = false;
    char *      bootrom_name              = 0;
    char *      dtb_name                  = 0;
    bool        compact_bootrom           = false;
    uint64_t    reset_vector_override     = 0;
    uint64_t    plic_base_addr_override   = 0;
    uint64_t    plic_size_override        = 0;
    uint64_t    clint_base_addr_override  = 0;
    uint64_t    clint_size_override       = 0;
    bool        custom_extension          = false;
    const char *simpoint_file             = 0;
    bool        clear_ids                 = false;

#ifdef LIVECACHE
    uint64_t    live_cache_size            = 8*1024*1024;
#endif
    bool        elf_based                  = false;
    bool        allow_ctrlc                = false;
    bool        show_enabled_extensions    = false;
    const char *march_string               = "rv64gc";

    dromajo_stdout    = stdout;
    dromajo_stderr    = stderr;
    dromajo_trace     = stderr;

    optind = 0;

    for (;;) {
        int option_index = 0;
        // clang-format off
        // available: k, v, E, F, G, J, K, N, O, Q, R, U, V, W, Y, w, x
        static struct option long_options[] = {
            {"help",                              no_argument, 0,  'h' },
            {"help-march",                        no_argument, 0,  'g' },
            {"show-march",                        no_argument, 0,  'j' },
            {"help-interactive",                  no_argument, 0,  'H' },

            {"cmdline",                     required_argument, 0,  'c' }, // CFG
            {"ncpus",                       required_argument, 0,  'n' }, // CFG
            {"load",                        required_argument, 0,  'l' },
            {"save",                        required_argument, 0,  's' },
            {"simpoint",                    required_argument, 0,  'S' },
            {"maxinsns",                    required_argument, 0,  'm' }, // CFG

            {"march   ",                    required_argument, 0,  'i' },
            {"custom_extension",                  no_argument, 0,  'u' }, // CFG

            {"trace",                       required_argument, 0,  't' },
            {"exe_trace",                   required_argument, 0,  'T' },
            {"exe_trace_log",               required_argument, 0,  'q' },
            {"interactive",                       no_argument, 0,  'I' },

            {"stf_trace",                   required_argument, 0,  'z' },
            {"stf_exit_on_stop_opc",              no_argument, 0,  'e' },
            {"stf_memrecord_size_in_bits",        no_argument, 0,  'B' },
            {"stf_trace_register_state",          no_argument, 0,  'y' },
            {"stf_disable_memory_records",        no_argument, 0,  'f' },
            {"stf_priv_modes",              required_argument, 0,  'a' },
            {"stf_force_zero_sha",                no_argument, 0,  'Z' },

            {"ignore_sbi_shutdown",         required_argument, 0,  'P' }, // CFG
            {"dump_memories",                     no_argument, 0,  'D' }, // CFG
            {"memory_size",                 required_argument, 0,  'M' }, // CFG
            {"memory_addr",                 required_argument, 0,  'A' }, // CFG
            {"bootrom",                     required_argument, 0,  'b' }, // CFG
            {"compact_bootrom",                   no_argument, 0,  'o' },
            {"reset_vector",                required_argument, 0,  'r' }, // CFG
            {"dtb",                         required_argument, 0,  'd' }, // CFG
            {"plic",                        required_argument, 0,  'p' }, // CFG
            {"clint",                       required_argument, 0,  'C' }, // CFG
            {"clear_ids",                         no_argument, 0,  'L' }, // CFG
            {"ctrlc",                             no_argument, 0,  'X' },
#ifdef LIVECACHE
            {"live_cache_size",          required_argument, 0,  'w' }, // CFG
#endif
            {0,                         0,                 0,  0 }
        };
        // clang-format on

        int c = getopt_long(argc, argv, "", long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
            case 'c':
                if (cmdline)
                    usage(prog, "already had a kernel command line");
                cmdline = strdup(optarg);
                break;

            case 'g': //Show supported extensions
                usage_isa();
                break;

            case 'h': // List options
                usage(prog, "Show usage");
                break;

            case 'H': // Show interactive command help
                usage_interactive();
                break;

            case 'i':
                march_string = strdup(optarg);
                break;

            case 'I':
                interactive = true;
                break;

            case 'j':
                show_enabled_extensions= true;
                break;

            case 'l':
                if (snapshot_load_name)
                    usage(prog, "already had a snapshot to load");
                snapshot_load_name = strdup(optarg);
                break;

            case 'n':
                if (ncpus != 0)
                    usage(prog, "already had a ncpus set");
                ncpus = atoll(optarg);
                break;

            case 's':
                if (snapshot_save_name)
                    usage(prog, "already had a snapshot to save");
                snapshot_save_name = strdup(optarg);
                break;

            case 'S':
                if (simpoint_file)
                    usage(prog, "already had a simpoint file");
                simpoint_file = strdup(optarg);
                break;

            case 'm':
                if (maxinsns)
                    usage(prog, "already had a max instructions");
                maxinsns = (uint64_t)atoll(optarg);
                {
                    char last = optarg[strlen(optarg) - 1];
                    if (last == 'k' || last == 'K')
                        maxinsns *= 1000;
                    else if (last == 'm' || last == 'M')
                        maxinsns *= 1000000;
                    else if (last == 'g' || last == 'G')
                        maxinsns *= 1000000000;
                }
                break;

            case 'T':
            case 't':
                if (exe_trace != UINT64_MAX)
                    usage(prog, "already had a trace set");
                exe_trace = (uint64_t)atoll(optarg);
                break;
            case 'q': exe_trace_file_name = strdup(optarg); break;

            case 'B': stf_memrecord_size_in_bits = true; break;
            case 'Z': stf_force_zero_sha = true; break;
            case 'a': stf_priv_modes = strdup(optarg); break;
            case 'e': stf_exit_on_stop_opc = true; break;
            case 'f': stf_disable_memory_records = true; break;
            case 'y': stf_trace_register_state = true; break;
            case 'z': stf_trace = strdup(optarg); break;

            case 'P': ignore_sbi_shutdown = true; break;
            case 'D': dump_memories = true; break;

            case 'M':
                if (optarg[0] == '0' && optarg[1] == 'x')
                    memory_size_override = strtoll(optarg + 2, NULL, 16);
                else
                    memory_size_override = atoi(optarg);
                break;

            case 'A':
                if (optarg[0] != '0' || optarg[1] != 'x')
                    usage(prog, "--memory_addr expects argument to start with 0x... ");
                memory_addr_override = strtoll(optarg + 2, NULL, 16);
                memory_addr_override_flag = true;
                break;

            case 'b':
                if (bootrom_name)
                    usage(prog, "already had a bootrom to load");
                bootrom_name = strdup(optarg);
                break;

            case 'd':
                if (dtb_name)
                    usage(prog, "already had a dtb to load");
                dtb_name = strdup(optarg);
                break;

            case 'o': compact_bootrom = true; break;

            case 'r':
                if (optarg[0] != '0' || optarg[1] != 'x')
                    usage(prog, "--reset_vector expects argument to start with 0x... ");
                reset_vector_override = strtoll(optarg + 2, NULL, 16);
                break;

            case 'p': {
                if (!strchr(optarg, ':'))
                    usage(prog, "--plic expects an argument like START:SIZE");

                char *copy           = strdup(optarg);
                char *plic_base_addr = strtok(copy, ":");
                char *plic_size      = strtok(NULL, ":");

                if (plic_base_addr[0] != '0' || plic_base_addr[1] != 'x')
                    usage(prog, "--plic START address must begin with 0x...");
                plic_base_addr_override = strtoll(plic_base_addr + 2, NULL, 16);

                if (plic_size[0] != '0' || plic_size[1] != 'x')
                    usage(prog, "--plic SIZE must begin with 0x...");
                plic_size_override = strtoll(plic_size + 2, NULL, 16);

                free(copy);
            } break;

            case 'C': {
                if (!strchr(optarg, ':'))
                    usage(prog, "--clint expects an argument like START:SIZE");

                char *copy            = strdup(optarg);
                char *clint_base_addr = strtok(copy, ":");
                char *clint_size      = strtok(NULL, ":");

                if (clint_base_addr[0] != '0' || clint_base_addr[1] != 'x')
                    usage(prog, "--clint START address must begin with 0x...");
                clint_base_addr_override = strtoll(clint_base_addr + 2, NULL, 16);

                if (clint_size[0] != '0' || clint_size[1] != 'x')
                    usage(prog, "--clint SIZE must begin with 0x...");
                clint_size_override = strtoll(clint_size + 2, NULL, 16);

                free(copy);
            } break;

            case 'u': custom_extension = true; break;

            case 'L': clear_ids = true; break;

#ifdef LIVECACHE
            case 'w':
                if (live_cache_size)
                    usage(prog, "already had a live_cache_size");
                live_cache_size = (uint64_t)atoll(optarg);
                {
                    char last = optarg[strlen(optarg) - 1];
                    if (last == 'k' || last == 'K')
                        live_cache_size *= 1000;
                    else if (last == 'm' || last == 'M')
                        live_cache_size *= 1000000;
                    else if (last == 'g' || last == 'G')
                        live_cache_size *= 1000000000;
                }
                break;
#endif
            case 'X':
                allow_ctrlc = true;
                break;

            default: usage(prog, "Unknown command line argument");
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "optin %d argc %d\n",optind,argc);
        usage(prog, "missing config file");
    } else {
        path = argv[optind++];
    }

/*
    if (optind < argc)
        usage(prog, "too many arguments");
*/

    assert(path);
    BlockDeviceModeEnum drive_mode = BF_MODE_SNAPSHOT;
    VirtMachineParams   p_s, *p = &p_s;

    virt_machine_set_defaults(p);
#ifdef CONFIG_FS_NET
    fs_wget_init();
#endif

    if (!load_elf_and_fake_the_config(p, path)) {
        virt_machine_load_config_file(p, path, NULL, NULL);
    } else {
        elf_based = true;
    }

    if (p->logfile) {
        FILE *log_out = fopen(p->logfile, "w");
        if (!log_out) {
            perror(p->logfile);
            exit(1);
        }
    }

#ifdef CONFIG_FS_NET
    fs_net_event_loop(NULL, NULL);
#endif

    /* override some config parameters */
    if (memory_addr_override_flag)
        p->ram_base_addr = memory_addr_override;
    if (memory_size_override)
        p->ram_size = memory_size_override << 20;

    if (ncpus)
        p->ncpus = ncpus;
    if (p->ncpus >= MAX_CPUS)
        usage(prog, "ncpus limit reached (MAX_CPUS). Increase MAX_CPUS");

    if (p->ncpus == 0)
        p->ncpus = 1;

    if (cmdline)
        vm_add_cmdline(p, cmdline);

    /* open the files & devices */
    for (int i = 0; i < p->drive_count; i++) {
        BlockDevice *drive;
        char *       fname;
        fname = get_file_path(p->cfg_filename, p->tab_drive[i].filename);
#ifdef CONFIG_FS_NET
        if (is_url(fname)) {
            net_completed = FALSE;
            drive = block_device_init_http(fname, 128 * 1024, 
                                           net_start_cb, NULL);
            /* wait until the drive is initialized */
            fs_net_event_loop(net_poll_cb, NULL);
        } else
#endif
        {
            drive = block_device_init(fname, drive_mode);
        }
        free(fname);
        p->tab_drive[i].block_dev = drive;
    }

    for (int i = 0; i < p->fs_count; i++) {
        FSDevice *  fs;
        const char *path;
        path = p->tab_fs[i].filename;
#ifdef CONFIG_FS_NET
        if (is_url(path)) {
            fs = fs_net_init(path, NULL, NULL);
            if (!fs)
                exit(1);
            fs_net_event_loop(NULL, NULL);
        } else
#endif
        {
#if defined(__APPLE__)
            fprintf(dromajo_stderr, "Filesystem access not supported yet\n");
            exit(1);
#else
            char *fname;
            fname = get_file_path(p->cfg_filename, path);
            fs    = fs_disk_init(fname);
            if (!fs) {
                fprintf(dromajo_stderr, "%s: must be a directory\n", fname);
                exit(1);
            }
            free(fname);
#endif
        }
        p->tab_fs[i].fs_dev = fs;
    }

    for (int i = 0; i < p->eth_count; i++) {
#ifdef CONFIG_SLIRP
        if (!strcmp(p->tab_eth[i].driver, "user")) {
            p->tab_eth[i].net = slirp_open();
            if (!p->tab_eth[i].net)
                exit(1);
        } else
#endif
#if !defined(__APPLE__)
            if (!strcmp(p->tab_eth[i].driver, "tap")) {
            p->tab_eth[i].net = tun_open(p->tab_eth[i].ifname);
            if (!p->tab_eth[i].net)
                exit(1);
        } else
#endif
        {
            fprintf(dromajo_stderr, "Unsupported network driver '%s'\n",
                    p->tab_eth[i].driver);
            exit(1);
        }
    }

    p->console       = console_init(allow_ctrlc, stdin, dromajo_stdout);
    p->dump_memories = dump_memories;

    // Setup bootrom params
    if (bootrom_name)
        p->bootrom_name = bootrom_name;
    if (dtb_name)
        p->dtb_name = dtb_name;
    p->compact_bootrom = compact_bootrom;

    // Setup particular reset vector
    if (reset_vector_override)
        p->reset_vector = reset_vector_override;

    // PLIC params
    if (plic_base_addr_override)
        p->plic_base_addr = plic_base_addr_override;
    if (plic_size_override)
        p->plic_size = plic_size_override;

    // CLINT params
    if (clint_base_addr_override)
        p->clint_base_addr = clint_base_addr_override;
    if (clint_size_override)
        p->clint_size = clint_size_override;

    // core modifications
    p->custom_extension = custom_extension;
    p->clear_ids        = clear_ids;

    RISCVMachine *s = virt_machine_init(p);
    if (!s)
        return NULL;

#ifdef LIVECACHE
    // LiveCache (should be ~2x larger than real LLC)
    s->llc = new LiveCache("LiveCache", live_cache_size, 
                           p->ram_base_addr, p->ram_size);
#endif

    if (elf_based) {
        for (int j = 0, i = optind - 1; i < argc; ++i, ++j) {
            uint8_t *buf;
            int      buf_len = load_file(&buf, argv[i]);

            if (elf64_is_riscv64(buf, buf_len)) {
                load_elf_image(s, buf, buf_len);
            } else
                load_hex_image(s, buf, buf_len);
        }
        for (int i = 0; i < (int)p->ncpus; ++i)
            s->cpu_state[i]->debug_mode = true;
    } else {
        s  = virt_machine_load(p, s);
        if (!s)
            return NULL;
    }

    // Overwrite the value specified in the configuration file
    if (snapshot_load_name) {
        s->common.snapshot_load_name = snapshot_load_name;
    }

    if (simpoint_file) {
#ifdef SIMPOINT_BB
        FILE *file = fopen(simpoint_file, "r");
        if (file == 0) {
            fprintf(stderr, "could not open simpoint file %s\n",
                    simpoint_file);
            exit(1);
        }
        int distance;
        int num;
        while (fscanf(file, "%d %d", &distance, &num) == 2) {
            uint64_t start = distance * SIMPOINT_SIZE;

            if (start == 0) {  // skip boot ROM
                start = ROM_SIZE;
            }

            s->common.simpoints.push_back({start, num});
        }

        std::sort(s->common.simpoints.begin(), s->common.simpoints.end());
        for (auto sp : s->common.simpoints) {
            printf("simpoint %d starts at %dK\n", sp.id, (int)sp.start / 1000);
        }

        if (s->common.simpoints.empty()) {
            fprintf(stderr, "simpoint file %s appears empty or invalid\n",
                    simpoint_file);
            exit(1);
        }
        s->common.simpoint_next = 0;
#else
        fprintf(stderr, "simpoint flag requires to recompile "
                        "with SIMPOINT_BB\n");
        exit(1);
#endif
    }

    if(!parse_isa_string(march_string,s->common.ext_flags)) {
      fprintf(stderr, "Parsing --march string failed\n");
      exit(1);
    }

    if(show_enabled_extensions) {
      printExtensionFlags(s->common.ext_flags,false); //not verbose
      exit(1);
    }

    s->common.snapshot_save_name = snapshot_save_name;
    s->common.exe_trace          = exe_trace;

    if(exe_trace_file_name) {

      dromajo_trace = fopen(exe_trace_file_name,"w");

      if(dromajo_trace == NULL) {
            fprintf(stderr, "Could not open execution log file '%s'\n", 
                    exe_trace_file_name);
      }

    } 

    s->common.interactive = interactive;

    /* STF Trace Generation */
    auto get_stf_highest_priv_mode = [](const char * stf_priv_modes) -> int {
        if(strcmp(stf_priv_modes, "USHM") == 0) {
            return PRV_M;
        }
        else if(strcmp(stf_priv_modes, "USH") == 0) {
            return PRV_H;
        }
        else if(strcmp(stf_priv_modes, "US") == 0) {
            return PRV_S;
        }
        else if(strcmp(stf_priv_modes, "U") == 0) {
            return PRV_U;
        }
        else {
            fprintf(stderr, "invalid stf privilege modes '%s'\n", 
                    stf_priv_modes);
            exit(1);
        }
    };

    s->common.stf_trace                  = stf_trace;
    s->common.stf_exit_on_stop_opc       = stf_exit_on_stop_opc;
    s->common.stf_memrecord_size_in_bits = stf_memrecord_size_in_bits;
    s->common.stf_trace_register_state   = stf_trace_register_state;
    s->common.stf_disable_memory_records = stf_disable_memory_records;
    s->common.stf_highest_priv_mode      = get_stf_highest_priv_mode(stf_priv_modes);
    s->common.stf_force_zero_sha         = stf_force_zero_sha;

    s->common.stf_trace_open           = false;
    s->common.stf_in_traceable_region  = false;
    s->common.stf_tracing_enabled      = false;
    s->common.stf_is_start_opc         = false;
    s->common.stf_is_stop_opc          = false;
    s->common.stf_has_exit_pending     = false;

    s->common.stf_prog_asid = 0;
    s->common.stf_count     = 0;

    // Allow the command option argument to overwrite the value
    // specified in the configuration file
    if (maxinsns > 0) {
        s->common.maxinsns = maxinsns;
    }

    // If no value is specified in the configuration or the command line
    // then run indefinitely
    if (s->common.maxinsns == 0)
        s->common.maxinsns = UINT64_MAX;

    for (int i = 0; i < s->ncpus; ++i) {
        s->cpu_state[i]->ignore_sbi_shutdown = ignore_sbi_shutdown;
    }

    virt_machine_free_config(p);

    if (s->common.net)
        s->common.net->device_set_carrier(s->common.net, TRUE);

    if (s->common.snapshot_load_name)
        virt_machine_deserialize(s, s->common.snapshot_load_name);

    return s;
}
