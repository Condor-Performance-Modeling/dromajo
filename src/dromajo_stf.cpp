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
        if(s->machine->common.stf_exit_on_stop_opc){
           s->terminate_simulation = 1;
        }
        return false;
    }

    return s->machine->common.stf_tracing_enabled;
}

stf::STFWriter stf_writer_simpoint;
#define STF_TRACE_DEBUG	\
    if(m->common.stf_insn_tracing_enabled){\
        if((m->common.stf_count > 0) && (m->common.stf_count % 100000 == 0)){\
        /**/ fprintf(dromajo_stderr, "\tSATP: %lx  ASID: %lx>>>>> Traced Instr Count : %ld / exe:%ld\n", cpu->satp, (cpu->satp >> 4) & 0xFFFF, m->common.stf_count, insn_executed); /* */\
        }\
    }

bool stf_trace_trigger_insn(RISCVMachine *m, int hartid, target_ulong PC, uint64_t insn_executed) 
{
    RISCVCPUState *cpu = m->cpu_state[hartid];

    static uint64_t stop_ninst = UINT64_MAX;
    static char stf_f[100];
	auto &sp = m->common.simpoints[m->common.simpoint_next];
	if(!m->common.simpoints.empty()){
		m->common.stf_insn_num_tracing = true;
		if(insn_executed >= sp.start) {
            char str[100];
            sprintf(str, "sp%d", sp.id);
            virt_machine_serialize(m, str);
            sprintf(stf_f, "simpoint_%d.zstf", sp.id);

            stop_ninst = sp.start + m->common.simpoint_size;
		    fprintf(dromajo_stderr, " STF [%d] %ld --> %ld == %d - %d", m->common.simpoint_next, sp.start, stop_ninst, m->common.stf_insn_start, m->common.stf_insn_stop);

			m->common.simpoint_next++;

            m->common.stf_insn_start = true;			
		} else {
            m->common.stf_insn_start = false;			
        }
		if(insn_executed == stop_ninst) {
            m->common.stf_insn_stop  = true;
        } else {
            m->common.stf_insn_stop  = false;
        }
	} else {
    		/*
            m->common.stf_insn_start = false;			
            m->common.stf_insn_stop  = false;
			// */
    		if((not (m->common.stf_insn_tracing_enabled) ) & (m->common.stf_count == 0)){
    		   m->common.stf_insn_start = (insn_executed >= m->common.stf_start);
    		} else {
    		   m->common.stf_insn_start = false;
    		}
    		if(m->common.stf_insn_tracing_enabled){
    		   m->common.stf_insn_stop  = (m->common.stf_count == m->common.stf_length);
    		} else {
    		   m->common.stf_insn_stop  = false;
    		}
    }
    if(m->common.stf_insn_start) {

        m->common.stf_insn_tracing_enabled = true;

        m->common.stf_prog_asid = (cpu->satp >> 4) & 0xFFFF;

        m->common.stf_count = 0;

        if((bool)stf_writer_simpoint == false) {
            stf_writer_simpoint.open(stf_f);//m->common.stf_trace);
            stf_writer_simpoint.addTraceInfo(stf::TraceInfoRecord(
                       stf::STF_GEN::STF_GEN_DROMAJO, 1, 1, 0,"Trace from Dromajo"));
            stf_writer_simpoint.setISA(stf::ISA::RISCV);
            stf_writer_simpoint.setHeaderIEM(stf::INST_IEM::STF_INST_IEM_RV64);
            stf_writer_simpoint.setTraceFeature(stf::TRACE_FEATURES::STF_CONTAIN_RV64);
            stf_writer_simpoint.setTraceFeature(stf::TRACE_FEATURES::STF_CONTAIN_PHYSICAL_ADDRESS);
            stf_writer_simpoint.setHeaderPC(PC);
            stf_writer_simpoint.finalizeHeader();
        	fprintf(dromajo_stderr, "\n\t>>> DROMAJO:STF HEADER Finalized - asid=%lx\n",  m->common.stf_prog_asid);
        }
        fprintf(dromajo_stderr, "\n\t>>> DROMAJO: Tracing Started at 0x%lx @INST_num %ld - asid=%lx\n", PC, insn_executed, m->common.stf_prog_asid);
        return true;

    } else if(m->common.stf_insn_stop) {

        m->common.stf_insn_tracing_enabled = false;
        fprintf(dromajo_stderr, "\n\t>>> DROMAJO: Tracing Stopped at 0x%lx @INST_num %ld\n", PC, insn_executed);
        fprintf(dromajo_stderr, "\n\t>>> DROMAJO: Traced %ld insts in %ld executed instructions\n",
                             m->common.stf_count, insn_executed);
        stf_writer_simpoint.close();
        STF_TRACE_DEBUG
        if (m->common.simpoint_next == m->common.simpoints.size()) {
           m->cpu_state[hartid]->terminate_simulation = 1;  // notify to terminate nicely
        }
        return false;
    }

    STF_TRACE_DEBUG
    return m->common.stf_insn_tracing_enabled;
}
