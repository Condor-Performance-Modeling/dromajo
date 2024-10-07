#pragma once
#include "riscv_machine.h"
extern FILE *dromajo_trace;
extern FILE *dromajo_stdout;
extern FILE *dromajo_stderr;
extern void execution_trace(RISCVMachine *m,int hartid,uint32_t insn_raw);
