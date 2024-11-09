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

#include "LiveCacheCore.h"
#include "cutils.h"
#include "dromajo.h"
#include "dromajo_protos.h"
#include "iomem.h"
#include "options.h"
#include "riscv_machine.h"
#include "virtio.h"

//#define REGRESS_COSIM 1
#ifdef REGRESS_COSIM
#include "dromajo_cosim.h"
#endif

#include "dromajo_stf.h"

#include <assert.h>
#include <signal.h>
#include <unordered_map>

using namespace std;

Options *Options::instance = 0;
std::shared_ptr<Options> opts(Options::getInstance());

#ifdef SIMPOINT_BB

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
        if (m->common.maxinsns > SIMPOINT_SIZE)
            next_bbv_dump = m->common.maxinsns - SIMPOINT_SIZE;
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
#endif

static int iterate_core(RISCVMachine *m, int hartid, int n_cycles) {
    m->common.maxinsns -= n_cycles;

    if (m->common.maxinsns <= 0)
        /* Succeed after N instructions without failure. */
        return 0;

    RISCVCPUState *cpu = m->cpu_state[hartid];

    /* Instruction that raises exceptions should be marked as such in
     * the trace of retired instructions.
     */
    uint64_t last_pc  = virt_machine_get_pc(m, hartid);
    int      priv     = riscv_get_priv_level(cpu);
    uint32_t insn_raw = -1;
    bool     en_trace       = false; //This is log or console tracing not STF
    bool     in_interactive = false;
    (void) in_interactive;

    (void)riscv_read_insn(cpu, &insn_raw, last_pc);

    //STF:The start OPC has been detected, throttle back n_cycles
    if(m->common.stf_tracing_enabled) {
      n_cycles = 1;
    }

    if (m->common.exe_trace < (unsigned) n_cycles) {
        n_cycles = 1;
        en_trace = true;
    } else if(m->common.interactive) {
        n_cycles = 1;
        in_interactive = true;
    } else {
        m->common.exe_trace -= n_cycles;
    }

    int keep_going = virt_machine_run(m, hartid, n_cycles);

    //STF:Trace the insn if the start OPC has been detected,
    //do not trace the start or stop insn's unless enabled
    if(m->common.stf_tracing_enabled)
    {
        if(!m->common.stf_is_start_opc && !m->common.stf_is_stop_opc) {
            stf_trace_element(m,hartid,priv,last_pc,insn_raw);
        }
    }

    if (!en_trace && !in_interactive) {
        return keep_going;
    }

    if(en_trace) { execution_trace(m,hartid,insn_raw); }

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
    if (!costate) return 1;
    while (!dromajo_cosim_step(costate, 0, 0, 0, 0, 0, false)) ;
    dromajo_cosim_fini(costate);

#else

    RISCVMachine *m = virt_machine_main(argc, argv);

    #ifdef SIMPOINT_BB
    if (m->common.simpoints.empty()) {
        simpoint_bb_file = fopen("dromajo_simpoint.bb", "w");
        if (simpoint_bb_file == nullptr) {
            fprintf(dromajo_stderr, "\nerror: could not "
                    "open dromajo_simpoint.bb for dumping trace\n");
            exit(-3);
        }
    }
    #endif

    if (!m) return 1;

    int n_cycles = 10000;
    execution_start_ts = get_current_time_in_seconds();
    execution_progress_meassure = &m->cpu_state[0]->minstret;
    signal(SIGINT, sigintr_handler);

    int keep_going;
    do {
        keep_going = 0;
        for (int i = 0; i < m->ncpus; ++i) {
            keep_going |= iterate_core(m, i, n_cycles);
        }

        #ifdef SIMPOINT_BB
        if (simpoint_roi) {
            if (!simpoint_step(m, 0)) break;
        }
        #endif
    } while (keep_going && !m->common.stf_has_exit_pending);

    double t = get_current_time_in_seconds();

    for (int i = 0; i < m->ncpus; ++i) {
        int benchmark_exit_code = riscv_benchmark_exit_code(m->cpu_state[i]);
        if (benchmark_exit_code != 0) {
            fprintf(dromajo_stderr, "\nBenchmark exited with code: %i \n",
                    benchmark_exit_code);
            return 1;
        }
    }

    if(stf_writer) {
        stf_writer.flush();
        stf_trace_close();
    }

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
