/*
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


#pragma once
#include "machine.h"
#include "network.h"
#include "uart.h"
#include "riscv_machine.h"
#include <cstdarg>

extern FILE *dromajo_trace;
extern FILE *dromajo_stdout;
extern FILE *dromajo_stderr;
extern void execution_trace(RISCVMachine *m,int hartid,uint32_t insn_raw);
//extern void riscv_flush_tlb_write_range(void *opaque, uint8_t *ram_addr, size_t ram_size);
extern void dromajo_default_debug_log(int hartid, const char *fmt, ...);
extern bool load_elf_and_fake_the_config(VirtMachineParams *p, const char *path);
extern CharacterDevice *console_init(bool allow_ctrlc, FILE *stdin, FILE *out);
extern EthernetDevice *tun_open(const char *ifname);

extern void uart_update_irq(SiFiveUARTState *s);
extern uint32_t uart_read(void *opaque, uint32_t offset, int size_log2);
extern void uart_write(void *opaque, uint32_t offset, uint32_t val, int size_log2);

extern uint32_t clint_read(void *opaque, uint32_t offset, int size_log2);
extern void clint_write(void *opaque, uint32_t offset, uint32_t val, int size_log2);
extern void plic_update_mip(RISCVMachine *s, int hartid);
extern uint32_t plic_read(void *opaque, uint32_t offset, int size_log2);
extern void plic_write(void *opaque, uint32_t offset, uint32_t val, int size_log2);
extern void plic_set_irq(void *opaque, int irq_num, int state);

extern int riscv_build_fdt(RISCVMachine *m, uint8_t *dst, const char *dtb_name, 
                           const char *cmd_line, uint64_t initrd_start,uint64_t initrd_end);
extern uint8_t *get_ram_ptr(RISCVMachine *s, uint64_t paddr);
extern void load_elf_image(RISCVMachine *s, const uint8_t *image, size_t image_len);
extern void load_hex_image(RISCVMachine *s, uint8_t *image, size_t image_len);
extern int load_bootrom(RISCVMachine *s, const char *bootrom_name);
extern int generate_bootrom(RISCVMachine *s);
extern int copy_kernel(RISCVMachine *s, uint8_t *fw_buf, size_t fw_buf_len, const uint8_t *kernel_buf, size_t kernel_buf_len, const uint8_t *initrd_buf, size_t initrd_buf_len, const char *bootrom_name, const char *dtb_name, const char *cmd_line);
extern void dump_dram(RISCVMachine *s, FILE *f[16], const char *region, uint64_t start, uint64_t len);

extern uint64_t rtc_get_time(RISCVMachine *m);
extern void dromajo_default_error_log(int hartid, const char *fmt, ...);
extern void dromajo_default_debug_log(int hartid, const char *fmt, ...);

extern uint64_t fli_h64(uint32_t rs);
extern uint64_t fli_s64(uint32_t rs);
extern uint64_t fli_d64(uint32_t rs);

extern uint64_t fround_h64(uint32_t rs1,uint32_t rm,uint32_t *flags);
extern uint64_t fround_s64(uint32_t rs1,uint32_t rm,uint32_t *flags);
extern uint64_t fround_d64(uint32_t rs1,uint32_t rm,uint32_t *flags);

extern uint64_t froundnx_h64(uint32_t rs1,uint32_t rm,uint32_t *flags);
extern uint64_t froundnx_s64(uint32_t rs1,uint32_t rm,uint32_t *flags);
extern uint64_t froundnx_d64(uint32_t rs1,uint32_t rm,uint32_t *flags);
