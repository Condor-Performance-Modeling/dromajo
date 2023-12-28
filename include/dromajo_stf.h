//
// STF gen protos.
//
#pragma once

#include "machine.h"
#include "riscv_cpu.h"
#include "trace_macros.h"
#include "stf-inc/stf_writer.hpp"
#include "stf-inc/stf_record_types.hpp"

extern stf::STFWriter stf_writer;
extern void stf_trace_element(RISCVMachine*,int hartid,int priv,
                              uint64_t last_pc,uint32_t insn);
extern bool stf_trace_trigger(RISCVCPUState*,uint64_t PC,uint32_t insn);
extern bool stf_trace_trigger_insn(RISCVCPUState *s,target_ulong PC, uint64_t insn_executed);
