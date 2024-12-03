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

#define STF_TRACE_DEBUG(s)	\
    if((s)->machine->common.stf_insn_tracing_enabled){\
        if((s)->machine->common.stf_num_traced % 1000000 == 0){\
            fprintf(dromajo_stderr, "\tSATP: %lx  ASID: %lx>>>>> Traced Instr Count : %ld / exe:%ld\n",\
                (s)->satp, ((s)->satp >> 4) & 0xFFFF, (s)->machine->common.stf_num_traced, (s)->machine->common.num_executed);\
        }\
    }

void stf_record_state(RISCVMachine * m, int hartid, uint64_t last_pc)
{
    RISCVCPUState * cpu = m->cpu_state[hartid];

    stf_writer << stf::ForcePCRecord(last_pc);

    if(m->common.stf_trace_register_state == true) {
        // Record integer registers
        for(int rn = 0; rn < 32; ++rn) {
            stf_writer << stf::InstRegRecord(rn,
              stf::Registers::STF_REG_TYPE::INTEGER,
              stf::Registers::STF_REG_OPERAND_TYPE::REG_STATE,
              riscv_get_reg(cpu, rn));
        }
        #if FLEN > 0
        // Record floating point registers
        for(int rn = 0; rn < 32; ++rn) {
            stf_writer << stf::InstRegRecord(rn,
              stf::Registers::STF_REG_TYPE::FLOATING_POINT,
              stf::Registers::STF_REG_OPERAND_TYPE::REG_STATE,
              riscv_get_fpreg(cpu, rn));
        }
        #endif
    } 
    // TODO: CSRs
}

bool stf_trace_trigger(RISCVCPUState *s,target_ulong PC,uint32_t insn) 
{
    s->machine->common.stf_is_start_opc     = insn == START_TRACE_OPC;
    s->machine->common.stf_is_stop_opc      = insn == STOP_TRACE_OPC;
    s->machine->common.stf_has_exit_pending = insn == STOP_TRACE_OPC
                               && s->machine->common.stf_exit_on_stop_opc;

    bool isStop =  (s->machine->common.stf_tracing_enabled
                &&  s->machine->common.stf_is_stop_opc)
                ||  s->machine->common.stf_has_exit_pending;

    if(s->machine->common.stf_has_exit_pending) {
        fprintf(dromajo_stderr, "@@@ DROMAJO: STOP OPC \n");
    }

    if(s->machine->common.stf_is_start_opc) {
        stf_trace_open(s, PC);
    } else if(isStop) {
        stf_trace_close(s, PC);
        if(s->machine->common.stf_exit_on_stop_opc){
           s->terminate_simulation = 1;
        }
    }
    
    STF_TRACE_DEBUG(s);
    return s->machine->common.stf_tracing_enabled;
}

void stf_trace_open(RISCVCPUState *s, target_ulong PC)
{
    int hartid = s->mhartid;
    //RISCVCPUState *cpu = s->machine->cpu_state[hartid];

    s->machine->common.stf_tracing_enabled = true;
    fprintf(dromajo_stderr, ">>> DROMAJO: Tracing Started at 0x%lx\n", PC);

    s->machine->common.stf_prog_asid = (s->satp >> 4) & 0xFFFF;

    //Do not re-open stf_writer
    if((bool)stf_writer == false) {
        stf_writer.open(s->machine->common.stf_trace);
        std::string drom_sha,stflib_sha;
        uint32_t vMajor,vMinor,vPatch;
        if(s->machine->common.stf_force_zero_sha) {
          drom_sha   = "DROMAJO SHA:0";
          stflib_sha = "STF_LIB SHA:0";
          vMajor = 0;
          vMinor = 0;
          vPatch = 0;
        } else {
          drom_sha   = std::string("DROMAJO SHA:")+std::string(DROMAJO_GIT_SHA);
          stflib_sha = std::string("STF_LIB SHA:")+std::string(STF_LIB_GIT_SHA);
          vMajor = VERSION_MAJOR;
          vMinor = VERSION_MINOR;
          vPatch = VERSION_PATCH;
        }

        stf_writer.addTraceInfo(stf::TraceInfoRecord(
           stf::STF_GEN::STF_GEN_DROMAJO,vMajor,vMinor,vPatch,drom_sha)
        );

        stf_writer.addHeaderComment(stflib_sha);
        stf_writer.setISA(stf::ISA::RISCV);
        stf_writer.setHeaderIEM(stf::INST_IEM::STF_INST_IEM_RV64);
        stf_writer.setTraceFeature(stf::TRACE_FEATURES::STF_CONTAIN_RV64);
        stf_writer.setTraceFeature(
           stf::TRACE_FEATURES::STF_CONTAIN_PHYSICAL_ADDRESS);

        stf_writer.setHeaderPC(PC);
        stf_writer.finalizeHeader();
    }

    stf_record_state(s->machine, hartid, PC);
}

void stf_trace_close(RISCVCPUState *s, target_ulong PC)
{
    if (stf_writer) {
        s->machine->common.stf_insn_tracing_enabled = false;

        fprintf(dromajo_stderr, "\n\t>>> DROMAJO: Tracing Stopped at 0x%lx @INST_num %ld\n", 
                                PC, s->machine->common.num_executed);
        fprintf(dromajo_stderr, "\n\t>>> DROMAJO: Traced %ld insts in %ld executed instructions\n",
                                s->machine->common.stf_num_traced, s->machine->common.num_executed);
        stf_writer.close();
    }
}

void stf_emit_memory_records(RISCVCPUState *cpu)
{
    // Memory reads
    for(auto mem_read : cpu->stf_mem_reads) {
        stf_writer << stf::InstMemAccessRecord(mem_read.vaddr,
                                               mem_read.size,
                                               0,
                                               stf::INST_MEM_ACCESS::READ);
        stf_writer << stf::InstMemContentRecord(mem_read.value);
    }

    cpu->stf_mem_reads.clear();

    // Memory writes
    for(auto mem_write : cpu->stf_mem_writes) {
        stf_writer << stf::InstMemAccessRecord(mem_write.vaddr,
                                               mem_write.size,
                                               0,
                                               stf::INST_MEM_ACCESS::WRITE);
        // empty content for now
        stf_writer << stf::InstMemContentRecord(mem_write.value);
    }

    cpu->stf_mem_writes.clear();
}

void stf_emit_register_records(RISCVCPUState *cpu)
{
    // general purpose source registers
    for(auto int_reg_src : cpu->stf_read_regs) {
        stf_writer << stf::InstRegRecord(int_reg_src,
                                         stf::Registers::STF_REG_TYPE::INTEGER,
                                         stf::Registers::STF_REG_OPERAND_TYPE::REG_SOURCE,
                                         riscv_get_reg(cpu, int_reg_src));
    }

    cpu->stf_read_regs.clear();

    // fp source regs - FLEN check is done in the macro
    // cpu->stf_read_fp_regs will be empty if no fp regs
    for(auto fp_reg_src : cpu->stf_read_fp_regs) {
        stf_writer << stf::InstRegRecord(fp_reg_src,
                                     stf::Registers::STF_REG_TYPE::FLOATING_POINT,
                                     stf::Registers::STF_REG_OPERAND_TYPE::REG_SOURCE,
                                     riscv_get_reg(cpu, fp_reg_src));
    }

    cpu->stf_read_fp_regs.clear();

    // general purpose destination regs
    for(auto int_reg_dst : cpu->stf_write_regs) {
        stf_writer << stf::InstRegRecord(int_reg_dst,
                                         stf::Registers::STF_REG_TYPE::INTEGER,
                                         stf::Registers::STF_REG_OPERAND_TYPE::REG_DEST,
                                         riscv_get_reg(cpu, int_reg_dst));
    }

    cpu->stf_write_regs.clear();

    // fp destination regs - FLEN check is done in the macro
    // cpu->stf_read_fp_regs will be empty if no fp regs
    for(auto fp_reg_dst : cpu->stf_write_fp_regs) {
        stf_writer << stf::InstRegRecord(fp_reg_dst,
                                         stf::Registers::STF_REG_TYPE::FLOATING_POINT,
                                         stf::Registers::STF_REG_OPERAND_TYPE::REG_DEST,
                                         riscv_get_fpreg(cpu, fp_reg_dst));
    }

    cpu->stf_write_fp_regs.clear();
}


void stf_trace_element(RISCVMachine *m,int hartid,int priv,
                       uint64_t last_pc,uint32_t insn_raw)
{
    RISCVCPUState *cpu = m->cpu_state[hartid];

    bool traceable_priv_level = priv <= m->common.stf_highest_priv_mode;

    if(traceable_priv_level && (cpu->pending_exception == -1)
       && (m->common.stf_prog_asid == ((cpu->satp >> 4) & 0xFFFF)))
    {
        ++m->common.stf_num_traced;

        const uint32_t inst_width = ((insn_raw & 0x3) == 0x3) ? 4 : 2;
        bool skip_record = false;

        // See if the instruction changed control flow or a
        // possible not-taken branch conditional
        if(cpu->info != ctf_nop) {
            stf_writer << stf::InstPCTargetRecord(virt_machine_get_pc(m, 0));
        }
        else {
            // JNYE: old comment from before my time:
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
            if(!m->common.stf_disable_memory_records) {
              stf_emit_memory_records(cpu);
            }

            if(m->common.stf_trace_register_state) {
              stf_emit_register_records(cpu);
            }

            // Instruction records 
            if(inst_width == 4) {
               stf_writer << stf::InstOpcode32Record(insn_raw);
            }
            else {
               stf_writer << stf::InstOpcode16Record(insn_raw & 0xFFFF);
            }
        }

        // Traps or exceptions
        if(cpu->pending_exception != -1) {
            stf_writer << stf::EventRecord(stf::EventRecord::TYPE(cpu->pending_exception), (uint64_t)0);
        }
    }

    cpu->stf_mem_reads.clear();
    cpu->stf_mem_writes.clear();
}

bool stf_trace_trigger_insn(RISCVCPUState *s, target_ulong PC) 
{
    //int hartid = s->mhartid;
    //RISCVCPUState *cpu = s->machine->cpu_state[hartid];

    if(!s->machine->common.stf_insn_tracing_enabled && (s->machine->common.stf_num_traced == 0)) {
       s->machine->common.stf_insn_started = (s->machine->common.num_executed >= s->machine->common.stf_insn_start);
    } else {
       s->machine->common.stf_insn_started = false;
    }

    if(s->machine->common.stf_insn_tracing_enabled){
       s->machine->common.stf_insn_stop  = (s->machine->common.stf_num_traced == s->machine->common.stf_insn_length);
    } else {
       s->machine->common.stf_insn_stop  = false;
    }

    if(s->machine->common.stf_insn_started) {
        stf_trace_open(s, PC);
    } else if(s->machine->common.stf_insn_stop) {
        stf_trace_close(s, PC);  
        if(s->machine->common.stf_exit_on_stop_opc){
           s->terminate_simulation = 1;
        }
    }

    STF_TRACE_DEBUG(s);
    return s->machine->common.stf_insn_tracing_enabled;
}
