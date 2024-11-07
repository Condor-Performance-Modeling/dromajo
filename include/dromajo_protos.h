#pragma once
#include "machine.h"
#include "network.h"
#include "uart.h"
#include "riscv_machine.h"
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


