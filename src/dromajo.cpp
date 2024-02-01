/*
 * Top-level driver
 *
 * Copyright (C) 2018,2019, Esperanto Technologies Inc.
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

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <inttypes.h>
#include <net/if.h>
#include <signal.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include <unordered_map>

#include "LiveCacheCore.h"
#include "cutils.h"
#include "iomem.h"
#include "riscv_machine.h"
#include "virtio.h"

//#define REGRESS_COSIM 1
#ifdef REGRESS_COSIM
#include "dromajo_cosim.h"
#endif

#include "dromajo_stf.h"
#include <limits>

//#ifdef SIMPOINT_BB
FILE *simpoint_bb_file = nullptr;
int   simpoint_roi     = 0;  // start without ROI enabled

int simpoint_step(RISCVMachine *m, int hartid) {
    assert(hartid == 0);  // Only single core for simpoint creation

    static uint64_t ninst = 0;  // ninst in BB
    ninst++;

    if (simpoint_bb_file == 0) {  // Creating checkpoints mode

        assert(!m->common.simpoints.empty());

        auto &sp = m->common.simpoints[m->common.simpoint_next];
        if (ninst > sp.start) {
            char str[100];
            sprintf(str, "sp%d", sp.id);
            virt_machine_serialize(m, str);

            m->common.simpoint_next++;
            if (m->common.simpoint_next == m->common.simpoints.size()) {
                return 0;  // notify to terminate nicely
            }
        }
        return 1;
    }

    // Creating bb trace mode
    assert(m->common.simpoints.empty());

    uint64_t                                 pc            = virt_machine_get_pc(m, hartid);
    static uint64_t                          next_bbv_dump = UINT64_MAX;
    static std::unordered_map<uint64_t, int> bbv;
    static std::unordered_map<uint64_t, int> pc2id;
    static int                               next_id = 1;
    if (m->common.maxinsns <= next_bbv_dump) {
        if (m->common.maxinsns > m->common.simpoint_size)
            next_bbv_dump = m->common.maxinsns - m->common.simpoint_size;
        else
            next_bbv_dump = 0;

        if (bbv.size()) {
            fprintf(simpoint_bb_file, "T");
            for (const auto ent : bbv) {
                auto it = pc2id.find(ent.first);
                int  id = 0;
                if (it == pc2id.end()) {
                    id = next_id;
                    next_id++;
                    pc2id[ent.first] = next_id;
                } else {
                    id = it->second;
                }

                fprintf(simpoint_bb_file, ":%d:%d ", id, ent.second);
            }
            fprintf(simpoint_bb_file, "\n");
            fflush(simpoint_bb_file);
            bbv.clear();
        }
    }

    static uint64_t last_pc = 0;
    if ((last_pc + 2) != pc && (last_pc + 4) != pc) {
        bbv[last_pc] += ninst;
        // fprintf(simpoint_bb_file,"xxxBB 0x%" PRIx64 " %d\n", pc, ninst);
        ninst = 0;
    }
    last_pc = pc;

    return 1;
}
//#endif

static int iterate_core(RISCVMachine *m, int hartid, int& n_cycles) {


    RISCVCPUState *cpu = m->cpu_state[hartid];

    /* Instruction that raises exceptions should be marked as such in
     * the trace of retired instructions.
     */
    uint64_t last_pc  = virt_machine_get_pc(m, hartid);
    int      priv     = riscv_get_priv_level(cpu);
    uint32_t insn_raw = -1;
    bool     do_trace = false;

    (void)riscv_read_insn(cpu, &insn_raw, last_pc);

    if(stf_trace_trigger_insn(cpu, last_pc, m->common.instruction_count)) {
    }

    //STF:The start OPC has been detected, throttle back n_cycles
    if(m->common.stf_tracing_enabled || m->common.stf_insn_tracing_enabled) {
      n_cycles = 1;
    }

    if (m->common.trace < (unsigned) n_cycles) {
        n_cycles = 1;
        do_trace = true;
    } else
      m->common.trace -= n_cycles;

    m->common.instruction_count = m->common.instruction_count + n_cycles;

    if(m->common.maxinsns  < uint64_t(n_cycles))
        m->common.maxinsns = 0;
    else
        m->common.maxinsns -= n_cycles;

    if (m->common.maxinsns <= 0)
        /* Succeed after N instructions without failure. */
        return 0;

    int keep_going = virt_machine_run(m, hartid, n_cycles);

    //STF:Trace the insn if the start OPC has been detected,
    //do not trace the start or stop insn's
    if((m->common.stf_tracing_enabled
       && !m->common.stf_is_start_opc
       && !m->common.stf_is_stop_opc) || (m->common.stf_insn_tracing_enabled && m->common.stf_insn_num_tracing))
    {
        RISCVCPUState *cpu = m->cpu_state[hartid];

        if((priv == 0 || m->common.stf_no_priv_check)
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
                    stf_writer << stf::InstMemAccessRecord(cpu->last_data_vaddr,
                                                           cpu->last_data_size,
                                                           0,
                                                           (cpu->last_data_type == 0) ?
                                                           stf::INST_MEM_ACCESS::READ :
                                                           stf::INST_MEM_ACCESS::WRITE);
                    stf_writer << stf::InstMemContentRecord(0); // empty content for now
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

	do_trace = false;
    if (!do_trace) {
        return keep_going;
    }

    fprintf(dromajo_stderr,
            "%d %d 0x%016" PRIx64 " (0x%08x)",
            hartid,
            priv,
            last_pc,
            (insn_raw & 3) == 3 ? insn_raw : (uint16_t)insn_raw);

    int iregno = riscv_get_most_recently_written_reg(cpu);
    int fregno = riscv_get_most_recently_written_fp_reg(cpu);

    if (cpu->pending_exception != -1)
        fprintf(dromajo_stderr,
                " exception %d, tval %016" PRIx64,
                cpu->pending_exception,
                riscv_get_priv_level(cpu) == PRV_M ? cpu->mtval : cpu->stval);
    else if (iregno > 0)
        fprintf(dromajo_stderr, " x%2d 0x%016" PRIx64, iregno, virt_machine_get_reg(m, hartid, iregno));
    else if (fregno >= 0)
        fprintf(dromajo_stderr, " f%2d 0x%016" PRIx64, fregno, virt_machine_get_fpreg(m, hartid, fregno));
    else
        for (int i = 31; i >= 0; i--)
            if (cpu->most_recently_written_vregs[i]) {
                fprintf(dromajo_stderr, " v%2d 0x", i);
                for (int j = VLEN / 8 - 1; j >= 0; j--) {
                    fprintf(dromajo_stderr, "%02" PRIx8, cpu->v_reg[i][j]);
                }
            }


    putc('\n', dromajo_stderr);

    return keep_going;
}

static double execution_start_ts;
static uint64_t *execution_progress_meassure;


static void sigintr_handler(int dummy) {
    double t = get_current_time_in_seconds();
    fprintf(dromajo_stderr, "Simulation speed: %5.2f MIPS (single-core)\n",
            1e-6 * *execution_progress_meassure / (t - execution_start_ts));
    exit(1);
}

int main(int argc, char **argv) {
#ifdef REGRESS_COSIM
    dromajo_cosim_state_t *costate = 0;
    costate                        = dromajo_cosim_init(argc, argv);

    if (!costate)
        return 1;

    while (!dromajo_cosim_step(costate, 0, 0, 0, 0, 0, false))
        ;
    dromajo_cosim_fini(costate);
#else
    RISCVMachine *m = virt_machine_main(argc, argv);

//#ifdef SIMPOINT_BB
    if (m->common.simpoints.empty() & m->common.en_bbv) {
        if (m->common.bb_file != nullptr){
             simpoint_bb_file = fopen(m->common.bb_file, "w");
        }
        else {
             simpoint_bb_file = fopen("dromajo_simpoint.bb", "w");
        }
        if (simpoint_bb_file == nullptr) {
            fprintf(dromajo_stderr, "\nerror: could not open dromajo_simpoint.bb for dumping trace\n");
            exit(-3);
        }
    }
//#endif

    if (!m)
        return 1;

    RISCVCPUState *cpu = m->cpu_state[0];

    int n_cycles = 10000;
    execution_start_ts = get_current_time_in_seconds();
    execution_progress_meassure = &m->cpu_state[0]->minstret;
    signal(SIGINT, sigintr_handler);

    uint64_t prev_prog_asid = 0;
    uint64_t inst_heart_beat = 0;
    uint64_t total_inst_count = 0;
    int keep_going;
    do {
        keep_going = 0;
        prev_prog_asid = (cpu->satp);
        for (int i = 0; i < m->ncpus; ++i) keep_going |= iterate_core(m, i, n_cycles);
        inst_heart_beat += n_cycles;
        total_inst_count += n_cycles;
        if(inst_heart_beat > m->common.heartbeat){
            fprintf(dromajo_stderr, "HeartBeat : %li / %li \n", inst_heart_beat, total_inst_count);
            inst_heart_beat = 0;
        }
        if((cpu->satp) != prev_prog_asid){
            fprintf(dromajo_stderr, "\n\t -- ASID ::  %lx --> %lx @%li \n", prev_prog_asid, (cpu->satp), total_inst_count);
        }
//#ifdef SIMPOINT_BB
        if (simpoint_roi & m->common.en_bbv) {
            if (!simpoint_step(m, 0))
                break;
        }
//#endif
    } while (keep_going);

    double t = get_current_time_in_seconds();

    for (int i = 0; i < m->ncpus; ++i) {
        int benchmark_exit_code = riscv_benchmark_exit_code(m->cpu_state[i]);
        if (benchmark_exit_code != 0) {
            fprintf(dromajo_stderr, "\nBenchmark exited with code: %i \n", benchmark_exit_code);
            return 1;
        }
    }

    fprintf(dromajo_stderr, "\nInstruction Count: %i \n", total_inst_count);
    fprintf(dromajo_stderr, "Simulation speed: %5.2f MIPS (single-core)\n",
            1e-6 * *execution_progress_meassure / (t - execution_start_ts));

    fprintf(dromajo_stderr, "\nPower off.\n");

    virt_machine_end(m);
#endif

#ifdef LIVECACHE
#if 0
    // LiveCache Dump
    uint64_t addr_size;
    uint64_t *addr = m->llc->traverse(addr_size);

    for (uint64_t i = 0u; i < addr_size; ++i) {
        printf("addr:%llx %s\n", (unsigned long long)addr[i], (addr[i] & 1) ? "ST" : "LD");
    }
#endif
    delete m->llc;
#endif

    return 0;
}
