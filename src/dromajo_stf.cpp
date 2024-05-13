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
    s->machine->common.stf_is_start_opc = insn == START_TRACE_OPC;
    s->machine->common.stf_is_stop_opc  = insn == STOP_TRACE_OPC;

    if(s->machine->common.stf_is_start_opc) {
        stf_trace_open(s, PC);
        return true;

    } else if(s->machine->common.stf_tracing_enabled
              && s->machine->common.stf_is_stop_opc)
    {
        s->machine->common.stf_tracing_enabled = false;
        fprintf(dromajo_stderr, ">>> DROMAJO: Tracing Stopped at 0x%lx\n", PC);
        fprintf(dromajo_stderr, ">>> DROMAJO: Traced %ld insts\n\n",
                             s->machine->common.stf_count);
        //Let main decide to close the file if we are done
        //stf_writer.close();
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

    //Do not re-open stf_writer
    if((bool)stf_writer == false) {
        stf_writer.open(s->machine->common.stf_trace);
        std::string sha;
        uint32_t vMajor,vMinor,vPatch;
        if(s->machine->common.stf_force_zero_sha) {
          sha = "SHA:0";
          vMajor = 0;
          vMinor = 0;
          vPatch = 0;
        } else {
          sha = std::string("SHA:")+std::string(DROMAJO_GIT_SHA);
          vMajor = VERSION_MAJOR;
          vMinor = VERSION_MINOR;
          vPatch = VERSION_PATCH;
        }

        stf_writer.addTraceInfo(stf::TraceInfoRecord(
           stf::STF_GEN::STF_GEN_DROMAJO,vMajor,vMinor,vPatch,sha)
        );

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

void stf_trace_close()
{
    if(stf_writer) {
        stf_writer.close();
        fprintf(dromajo_stderr,"-I: closed STF file\n");
    }
}

void stf_trace_element(RISCVMachine *m,int hartid,int priv,uint64_t last_pc,uint32_t insn_raw)
{
    RISCVCPUState *cpu = m->cpu_state[hartid];

    bool traceable_priv_level = priv <= m->common.stf_highest_priv_mode;

    // Traps or exceptions
    if(cpu->pending_exception != -1) {
        stf_writer << stf::EventRecord(stf::EventRecord::TYPE(cpu->pending_exception), (uint64_t)0);
    }

    if(traceable_priv_level && (cpu->pending_exception == -1)
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

//FIXME: I'm keeping this code to make is simpler when features are restored
//       see dromajo_main for the disabled features
//void stf_trace_element(RISCVMachine * m, int hartid, int priv, uint64_t last_pc, uint32_t insn)
//{
//    RISCVCPUState * cpu = m->cpu_state[hartid];
//    const uint64_t current_pc = cpu->pc;
//
//    ++(m->common.stf_count);
//    const uint32_t inst_width = ((insn & 0x3) == 0x3) ? 4 : 2;
//
//    // PC target (change of flow)
//    if(current_pc != last_pc + inst_width) {
//        stf_writer << stf::InstPCTargetRecord(virt_machine_get_pc(m, 0));
//    }
//
//    if(m->common.stf_essential_mode == false) {
//        // Source registers
//        for(auto int_reg_src : cpu->stf_read_regs) {
//            stf_writer << stf::InstRegRecord(int_reg_src,
//                                             stf::Registers::STF_REG_TYPE::INTEGER,
//                                             stf::Registers::STF_REG_OPERAND_TYPE::REG_SOURCE,
//                                             riscv_get_reg(cpu, int_reg_src));
//        }
//#if FLEN > 0
//        for(auto fp_reg_src : cpu->stf_read_fp_regs) {
//            stf_writer << stf::InstRegRecord(fp_reg_src,
//                                         stf::Registers::STF_REG_TYPE::FLOATING_POINT,
//                                         stf::Registers::STF_REG_OPERAND_TYPE::REG_SOURCE,
//                                         riscv_get_reg(cpu, fp_reg_src));
//        }
//#endif
//        // Destination registers
//        for(auto int_reg_dst : cpu->stf_write_regs) {
//            stf_writer << stf::InstRegRecord(int_reg_dst,
//                                             stf::Registers::STF_REG_TYPE::INTEGER,
//                                             stf::Registers::STF_REG_OPERAND_TYPE::REG_DEST,
//                                             riscv_get_reg(cpu, int_reg_dst));
//            }
//#if FLEN > 0
//        for(auto fp_reg_dst : cpu->stf_write_fp_regs) {
//            stf_writer << stf::InstRegRecord(fp_reg_dst,
//                                             stf::Registers::STF_REG_TYPE::FLOATING_POINT,
//                                             stf::Registers::STF_REG_OPERAND_TYPE::REG_DEST,
//                                             riscv_get_fpreg(cpu, fp_reg_dst));
//        }
//#endif
//        // Memory reads
//        for(auto mem_read : cpu->stf_mem_reads) {
//            stf_writer << stf::InstMemAccessRecord(mem_read.vaddr,
//                                                   mem_read.size,
//                                                   0,
//                                                   stf::INST_MEM_ACCESS::READ);
//            stf_writer << stf::InstMemContentRecord(mem_read.value);
//        }
//        // Memory writes
//        for(auto mem_write : cpu->stf_mem_writes) {
//            stf_writer << stf::InstMemAccessRecord(mem_write.vaddr,
//                                                   mem_write.size,
//                                                   0,
//                                                   stf::INST_MEM_ACCESS::WRITE);
//            stf_writer << stf::InstMemContentRecord(mem_write.value); // empty content for now
//        }
//    }
//
//    // Privilege mode change
//    if(cpu->stf_prev_priv_mode != cpu->priv) {
//        stf_writer << stf::EventRecord(stf::EventRecord::TYPE::MODE_CHANGE, (uint64_t)cpu->priv);
//        cpu->stf_prev_priv_mode = cpu->priv;
//    }
//    // Traps or exceptions
//    if(cpu->pending_exception != -1) {
//        stf_writer << stf::EventRecord(stf::EventRecord::TYPE(cpu->pending_exception), (uint64_t)0);
//    }
//    // Opcode (instruction)
//    if(inst_width == 4) {
//       stf_writer << stf::InstOpcode32Record(insn);
//    }
//    else {
//       stf_writer << stf::InstOpcode16Record(insn & 0xFFFF);
//    }
//
//    // Reset
//    riscv_stf_reset(cpu);
//}
//
//bool stf_trace_trigger(RISCVMachine * m, int hartid, uint32_t insn)
//{
//    uint64_t pc = virt_machine_get_pc(m, hartid);
//
//    // Determine if we're in a traceable region of the workload. All conditions
//    // must be met (true) to begin/continue tracing.
//
//    // If tracepoints are enabled, open the trace and start tracing when the
//    // start tracepoint is detected
//    if(m->common.stf_tracepoints_enabled) {
//        if(insn == START_TRACE_OPC) {
//            m->common.stf_in_tracepoint_region = true;
//        }
//        else if(insn == STOP_TRACE_OPC) {
//            m->common.stf_in_tracepoint_region = false;
//        }
//    }
//    bool in_traceable_region = m->common.stf_in_tracepoint_region;
//
//    // Are we in a traceable privilege mode?
//    in_traceable_region &= 
//        riscv_get_priv_level(m->cpu_state[hartid]) <= m->common.stf_highest_priv_mode;
//
//    // Are we executing the expected thread?
//    const uint64_t asid = (m->cpu_state[hartid]->satp >> 4) & 0xFFFF;
//    in_traceable_region &= m->common.stf_prog_asid == asid;
//
//    const bool entering_traceable_region = 
//        in_traceable_region && (m->common.stf_in_traceable_region == false);
//    const bool exiting_traceable_region = 
//        (in_traceable_region == false) && m->common.stf_in_traceable_region;
//    m->common.stf_in_traceable_region = in_traceable_region;
//
//    /* If entering the traceable region for the first time, open the trace.
//     * If the trace is already open, it means we're reentering the traceable
//     * region after leaving it. Record the state to capture all the program
//     * state changes that occurred in the non-traceable region. The current
//     * instruction will not be traced.
//     */
//    if(entering_traceable_region) {
//        if(m->common.stf_trace_open == false) {
//            stf_trace_open(m, hartid, pc);
//        }
//	else {
//            stf_record_state(m, hartid, pc);
//        }
//    }
//
//    // Returns true if current instruction should be traced
//    const bool exclude_stop_tracepoint = m->common.stf_tracepoints_enabled && (m->common.stf_include_stop_tracepoint == false) && (insn = STOP_TRACE_OPC);
//    return (m->common.stf_in_traceable_region && !entering_traceable_region) || (exiting_traceable_region && !exclude_stop_tracepoint);
//}
//
//void stf_trace_open(RISCVMachine * m, int hartid, target_ulong pc)
//{
//    m->common.stf_trace_open = true;
//    fprintf(dromajo_stderr, ">>> DROMAJO: Tracing Started at 0x%llx\n",
//       (long long unsigned int) pc);
//
//    RISCVCPUState * s = m->cpu_state[hartid];
//    m->common.stf_prog_asid = (s->satp >> 4) & 0xFFFF;
//    s->stf_prev_priv_mode = s->priv;
//
//    if((bool)stf_writer == false) {
//        std::string SHA = m->common.stf_force_zero_sha
//                        ? "SHA:0"
//                        : std::string("SHA:")+std::string(DROMAJO_GIT_SHA);
////        std::string SHA(std::string("SHA:")+std::string(DROMAJO_GIT_SHA));
////        if(m->common.stf_force_zero_sha) SHA = "SHA:0";
//        stf_writer.open(m->common.stf_trace);
//        stf_writer.addTraceInfo(stf::TraceInfoRecord(
//                   stf::STF_GEN::STF_GEN_DROMAJO,
//                   VERSION_MAJOR,
//                   VERSION_MINOR,
//                   VERSION_PATCH,
//                   SHA));
//        stf_writer.setISA(stf::ISA::RISCV);
//        stf_writer.setHeaderIEM(stf::INST_IEM::STF_INST_IEM_RV64);
//        stf_writer.setTraceFeature(stf::TRACE_FEATURES::STF_CONTAIN_RV64);
//        stf_writer.setTraceFeature(stf::TRACE_FEATURES::STF_CONTAIN_PHYSICAL_ADDRESS);
//	// TODO: Update trace features
//        stf_writer.setHeaderPC(pc);
//        stf_writer.finalizeHeader();
//    }
//
//    stf_record_state(m, hartid, pc);
//}
//
//void stf_trace_close(RISCVMachine * m, target_ulong pc)
//{
//    m->common.stf_trace_open = false;
//    fprintf(dromajo_stderr, ">>> DROMAJO: Tracing Stopped at 0x%llx\n",
//       (long long unsigned int) pc);
//    fprintf(dromajo_stderr, ">>> DROMAJO: Traced %llu insts\n",
//       (long long unsigned int) m->common.stf_count);
//    stf_writer.close();
//}
//
