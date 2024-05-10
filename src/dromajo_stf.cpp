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
#include "dromajo_sha.h"

#ifdef REGRESS_COSIM
#include "dromajo_cosim.h"
#endif

#include "dromajo_stf.h"
stf::STFWriter stf_writer;

bool stf_trace_trigger(RISCVCPUState *s,target_ulong PC,uint32_t insn) 
{
    s->machine->common.stf_is_start_opc = insn == START_TRACE_OPC;
    s->machine->common.stf_is_stop_opc  = insn == STOP_TRACE_OPC;

    if(s->machine->common.stf_is_start_opc) {
        stf_trace_open(s, PC);
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

void stf_trace_open(RISCVCPUState *s, target_ulong PC)
{
    int hartid = s->mhartid;
    RISCVCPUState *cpu = s->machine->cpu_state[hartid];

    s->machine->common.stf_tracing_enabled = true;
    fprintf(dromajo_stderr, ">>> DROMAJO: Tracing Started at 0x%lx\n", PC);

    s->machine->common.stf_prog_asid = (cpu->satp >> 4) & 0xFFFF;

    if((bool)stf_writer == false) {
        stf_writer.open(s->machine->common.stf_trace);
        std::string SHA(std::string("SHA:")+std::string(DROMAJO_GIT_SHA));

        stf_writer.addTraceInfo(stf::TraceInfoRecord(
           stf::STF_GEN::STF_GEN_DROMAJO, 
           VERSION_MAJOR,
           VERSION_MINOR,
           VERSION_PATCH,
           SHA)
        );

        stf_writer.setISA(stf::ISA::RISCV);

        stf_writer.setHeaderIEM(stf::INST_IEM::STF_INST_IEM_RV64);

        stf_writer.setTraceFeature(stf::TRACE_FEATURES::STF_CONTAIN_RV64);

        stf_writer.setTraceFeature(
           stf::TRACE_FEATURES::STF_CONTAIN_PHYSICAL_ADDRESS);

        stf_writer.setHeaderPC(PC);
        stf_writer.finalizeHeader();
    }
}

void stf_trace_close(RISCVCPUState *s, target_ulong PC)
{
    s->machine->common.stf_tracing_enabled = false;
    fprintf(dromajo_stderr,">>> DROMAJO: Tracing Stopped at 0x%lx\n", PC);
    fprintf(dromajo_stderr,">>> DROMAJO: Traced %ld insts\n",
                            s->machine->common.stf_count);
    stf_writer.close();
}

void stf_trace_element(RISCVMachine *m,int hartid,int priv,uint64_t last_pc,uint32_t insn_raw)
{
    RISCVCPUState *cpu = m->cpu_state[hartid];

    bool traceable_priv_level = priv <= m->common.stf_highest_priv_mode;

    if(traceable_priv_level
       && (cpu->pending_exception == -1)
       && (m->common.stf_prog_asid == ((cpu->satp >> 4) & 0xFFFF)))
    {

        ++m->common.stf_count;
        const uint32_t inst_width = ((insn_raw & 0x3) == 0x3) ? 4 : 2;
        bool skip_record = false;

        // See if the instruction changed control flow or a
        // possible not-taken branch conditional
        if(cpu->info != ctf_nop) {
            stf_writer << stf::InstPCTargetRecord(virt_machine_get_pc(m, 0));
        }
        else {
            // Not sure what's going on, but there's a
            // possibility that the current instruction will
            // cause a page fault or a timer interrupt or
            // process switch so the next instruction might
            // not be on the program's path
            if(cpu->pc != last_pc + inst_width) {
                skip_record = true;
            }
        }

        // Record the instruction trace record
        if(false == skip_record)
        {
            // If the last instruction were a load/store,
            // record the last vaddr, size, and if it were a
            // read or write.

            if(cpu->last_data_vaddr
                != std::numeric_limits<decltype(cpu->last_data_vaddr)>::max())
            {
                //If not a read to the tohost address then trace it
                if(cpu->last_data_type != 0 || cpu->last_data_vaddr != m->htif_tohost_addr)
                {
                    stf_writer << stf::InstMemAccessRecord(cpu->last_data_vaddr,
                                                           cpu->last_data_size,
                                                           0,
                                                           (cpu->last_data_type == 0) ?
                                                           stf::INST_MEM_ACCESS::READ :
                                                           stf::INST_MEM_ACCESS::WRITE);
                    stf_writer << stf::InstMemContentRecord(0); // empty content for now
                }
            }

            if(inst_width == 4) {
               stf_writer << stf::InstOpcode32Record(insn_raw);
            }
            else {
               stf_writer << stf::InstOpcode16Record(insn_raw & 0xFFFF);
            }
        }
    }
}

//void stf_record_state(RISCVMachine * m, int hartid, uint64_t last_pc)
//{
//    RISCVCPUState * cpu = m->cpu_state[hartid];
//
//    stf_writer << stf::ForcePCRecord(last_pc);
//
//    if(m->common.stf_trace_register_state == true) {
//        // Record integer registers
//        for(int rn = 0; rn < 32; ++rn) {
//            stf_writer << stf::InstRegRecord(rn,
//              stf::Registers::STF_REG_TYPE::INTEGER,
//              stf::Registers::STF_REG_OPERAND_TYPE::REG_STATE,
//              riscv_get_reg(cpu, rn));
//        }
//#if FLEN > 0
//        // Record floating point registers
//        for(int rn = 0; rn < 32; ++rn) {
//            stf_writer << stf::InstRegRecord(rn,
//              stf::Registers::STF_REG_TYPE::FLOATING_POINT,
//              stf::Registers::STF_REG_OPERAND_TYPE::REG_STATE,
//              riscv_get_fpreg(cpu, rn));
//        }
//#endif
//    }
//    // TODO: CSRs
//}

