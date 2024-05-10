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

extern void stf_trace_element(RISCVMachine*,int,int,uint64_t,uint32_t);
extern bool stf_trace_trigger(RISCVCPUState*,uint64_t,uint32_t);
extern void stf_record_state(RISCVMachine*,int,uint64_t);
extern void stf_trace_open(RISCVCPUState*,target_ulong);
extern void stf_trace_close(RISCVCPUState*,target_ulong);
