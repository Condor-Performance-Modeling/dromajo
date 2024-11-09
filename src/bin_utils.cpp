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
#include "elf64.h"
#include "riscv_machine.h"

#include <cstdarg>
#include <err.h>

uint8_t *get_ram_ptr(RISCVMachine *s, uint64_t paddr) {
    PhysMemoryRange *pr = get_phys_mem_range(s->mem_map, paddr);
    if (!pr || !pr->is_ram)
        return NULL;
    return pr->phys_mem + (uintptr_t)(paddr - pr->addr);
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

int load_bootrom(RISCVMachine *s, const char *bootrom_name) {
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

int generate_bootrom(RISCVMachine *s) {
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
int copy_kernel(RISCVMachine *s, uint8_t *fw_buf, size_t fw_buf_len, const uint8_t *kernel_buf, size_t kernel_buf_len,
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

void dump_dram(RISCVMachine *s, FILE *f[16], const char *region, uint64_t start, uint64_t len) {
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


