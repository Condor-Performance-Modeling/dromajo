/*
 * STF gen trigger detection
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
 */
#include "dromajo.h"

#ifdef REGRESS_COSIM
#include "dromajo_cosim.h"
#endif

#include "dromajo_stf.h"
stf::STFWriter stf_writer;
bool stf_trace_trigger(RISCVCPUState *s,target_ulong PC,uint32_t insn) 
{
    int hartid = s->mhartid;
    RISCVCPUState *cpu = s->machine->cpu_state[hartid];

    s->machine->common.stf_is_start_opc = insn == START_TRACE_OPC;
    s->machine->common.stf_is_stop_opc  = insn == STOP_TRACE_OPC;

    if(s->machine->common.stf_is_start_opc) {

        s->machine->common.stf_tracing_enabled = true;
        fprintf(dromajo_stderr, ">>> DROMAJO: Tracing Started at 0x%lx\n", PC);

        s->machine->common.stf_prog_asid = (cpu->satp >> 4) & 0xFFFF;

        if((bool)stf_writer == false) {
            stf_writer.open(s->machine->common.stf_trace);
            stf_writer.addTraceInfo(stf::TraceInfoRecord(
                       stf::STF_GEN::STF_GEN_DROMAJO, 1, 1, 0,"Trace from Dromajo"));
            stf_writer.setISA(stf::ISA::RISCV);
            stf_writer.setHeaderIEM(stf::INST_IEM::STF_INST_IEM_RV64);
            stf_writer.setTraceFeature(stf::TRACE_FEATURES::STF_CONTAIN_RV64);
            stf_writer.setTraceFeature(stf::TRACE_FEATURES::STF_CONTAIN_PHYSICAL_ADDRESS);
            stf_writer.setHeaderPC(PC);
            stf_writer.finalizeHeader();
        }
        return true;

    } else if(s->machine->common.stf_is_stop_opc) {

        s->machine->common.stf_tracing_enabled = false;
        fprintf(dromajo_stderr, ">>> DROMAJO: Tracing Stopped at 0x%lx\n", PC);
        fprintf(dromajo_stderr, ">>> DROMAJO: Traced %ld insts\n",
                             s->machine->common.stf_count);
        stf_writer.close();
        return false;
    }

    return s->machine->common.stf_tracing_enabled;
}

#define STF_TRACE_DEBUG	\
    if(s->machine->common.stf_insn_tracing_enabled){\
        /**/ fprintf(dromajo_stderr, "\t\t>>>>> Traced Instr Count : %ld / exe:%ld\n", s->machine->common.stf_count, insn_executed); /* */\
    }

bool stf_trace_trigger_insn(RISCVCPUState *s,target_ulong PC, uint64_t insn_executed) 
{
    int hartid = s->mhartid;
    RISCVCPUState *cpu = s->machine->cpu_state[hartid];

    s->machine->common.stf_insn_start = (insn_executed == s->machine->common.stf_start);
    s->machine->common.stf_insn_stop  = s->machine->common.stf_insn_tracing_enabled && (s->machine->common.stf_count == s->machine->common.stf_length);//(insn_executed == (s->machine->common.stf_start + s->machine->common.stf_length - 1));

    if(s->machine->common.stf_insn_start) {

        s->machine->common.stf_insn_tracing_enabled = true;

        s->machine->common.stf_prog_asid = (cpu->satp >> 4) & 0xFFFF;

        if((bool)stf_writer == false) {
            stf_writer.open(s->machine->common.stf_trace);
            stf_writer.addTraceInfo(stf::TraceInfoRecord(
                       stf::STF_GEN::STF_GEN_DROMAJO, 1, 1, 0,"Trace from Dromajo"));
            stf_writer.setISA(stf::ISA::RISCV);
            stf_writer.setHeaderIEM(stf::INST_IEM::STF_INST_IEM_RV64);
            stf_writer.setTraceFeature(stf::TRACE_FEATURES::STF_CONTAIN_RV64);
            stf_writer.setTraceFeature(stf::TRACE_FEATURES::STF_CONTAIN_PHYSICAL_ADDRESS);
            stf_writer.setHeaderPC(PC);
            stf_writer.finalizeHeader();
        }
        fprintf(dromajo_stderr, "\n\t>>> DROMAJO: Tracing Started at 0x%lx @INST_num %ld - asid=%lx\n", PC, insn_executed, s->machine->common.stf_prog_asid);
        STF_TRACE_DEBUG
        return true;

    } else if(s->machine->common.stf_insn_stop) {

        s->machine->common.stf_insn_tracing_enabled = false;
        fprintf(dromajo_stderr, "\n\t>>> DROMAJO: Tracing Stopped at 0x%lx @INST_num %ld\n", PC, insn_executed);
        fprintf(dromajo_stderr, "\n\t>>> DROMAJO: Traced %ld insts in %ld executed instructions\n",
                             s->machine->common.stf_count, insn_executed);
        stf_writer.close();
        STF_TRACE_DEBUG
        return false;
    }

    STF_TRACE_DEBUG
    return s->machine->common.stf_insn_tracing_enabled;
}
