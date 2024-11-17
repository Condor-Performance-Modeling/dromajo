/*
 * RISCV machine
 *
 * Copyright (c) 2016-2017 Fabrice Bellard
 * Copyright (C) 2018,2019, Esperanto Technologies Inc.
 * Contribution (C) 2024, Jeff Nye
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
 *
 * THIS FILE IS BASED ON THE RISCVEMU SOURCE CODE WHICH IS DISTRIBUTED
 * UNDER THE FOLLOWING LICENSE:
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "block_device.h"
#include "dromajo_protos.h"
#include "dw_apb_uart.h"
#include "elf64.h"
#include "options.h"
#include "riscv_machine.h"
#include "termio.h"
#include "virtio.h"

#include <string>
#include <cstdarg>
#include <err.h>
#include <getopt.h>
using namespace std;

RISCVMachine *virt_machine_main(int argc, char **argv) {

//FIXME: full integration of boost options is in progress.
//commenting this turns of boost options but keeps the 
//other code
//    opts->setup_options(argc,argv);

    const char *prog                = argv[0];
    char *      snapshot_load_name  = 0;
    char *      snapshot_save_name  = 0;
    const char *path                = NULL;
    const char *cmdline             = NULL;
    long        ncpus               = 0;
    uint64_t    maxinsns            = 0;

    uint64_t    exe_trace           = UINT64_MAX;
    const char *exe_trace_file_name = nullptr;
    bool        interactive         = false;

    const char *stf_trace                  = nullptr;
    bool        stf_exit_on_stop_opc       = false;
    bool        stf_memrecord_size_in_bits = false;
    bool        stf_trace_register_state   = false;
    bool        stf_disable_memory_records = false;
    const char *stf_priv_modes             = "USHM";
    bool        stf_force_zero_sha         = false;

    long        memory_size_override      = 0;
    uint64_t    memory_addr_override      = 0;
    bool        memory_addr_override_flag = false;
    bool        ignore_sbi_shutdown       = false;
    bool        dump_memories             = false;
    char *      bootrom_name              = 0;
    char *      dtb_name                  = 0;
    bool        compact_bootrom           = false;
    uint64_t    reset_vector_override     = 0;
    uint64_t    plic_base_addr_override   = 0;
    uint64_t    plic_size_override        = 0;
    uint64_t    clint_base_addr_override  = 0;
    uint64_t    clint_size_override       = 0;
    bool        custom_extension          = false;
    const char *simpoint_file             = 0;
    bool        clear_ids                 = false;

#ifdef LIVECACHE
    uint64_t    live_cache_size            = 8*1024*1024;
#endif
    bool        elf_based                  = false;
    bool        allow_ctrlc                = false;
    bool        show_enabled_extensions    = false;
    const char *march_string               = "rv64gc";

    dromajo_stdout    = stdout;
    dromajo_stderr    = stderr;
    dromajo_trace     = stderr;

    optind = 0;

    for (;;) {
        int option_index = 0;
        // clang-format off
        // available: k, v, E, F, G, J, K, N, O, Q, R, U, V, W, Y, w, x
        static struct option long_options[] = {
            {"help",                              no_argument, 0,  'h' },
            {"help-march",                        no_argument, 0,  'g' },
            {"show-march",                        no_argument, 0,  'j' },
            {"help-interactive",                  no_argument, 0,  'H' },

            {"cmdline",                     required_argument, 0,  'c' }, // CFG
            {"ncpus",                       required_argument, 0,  'n' }, // CFG
            {"load",                        required_argument, 0,  'l' },
            {"save",                        required_argument, 0,  's' },
            {"simpoint",                    required_argument, 0,  'S' },
            {"maxinsns",                    required_argument, 0,  'm' }, // CFG

            {"march   ",                    required_argument, 0,  'i' },
            {"custom_extension",                  no_argument, 0,  'u' }, // CFG

            {"trace",                       required_argument, 0,  't' },
            {"exe_trace",                   required_argument, 0,  'T' },
            {"exe_trace_log",               required_argument, 0,  'q' },
            {"interactive",                       no_argument, 0,  'I' },

            {"stf_trace",                   required_argument, 0,  'z' },
            {"stf_exit_on_stop_opc",              no_argument, 0,  'e' },
            {"stf_memrecord_size_in_bits",        no_argument, 0,  'B' },
            {"stf_trace_register_state",          no_argument, 0,  'y' },
            {"stf_disable_memory_records",        no_argument, 0,  'f' },
            {"stf_priv_modes",              required_argument, 0,  'a' },
            {"stf_force_zero_sha",                no_argument, 0,  'Z' },

            {"ignore_sbi_shutdown",         required_argument, 0,  'P' }, // CFG
            {"dump_memories",                     no_argument, 0,  'D' }, // CFG
            {"memory_size",                 required_argument, 0,  'M' }, // CFG
            {"memory_addr",                 required_argument, 0,  'A' }, // CFG
            {"bootrom",                     required_argument, 0,  'b' }, // CFG
            {"compact_bootrom",                   no_argument, 0,  'o' },
            {"reset_vector",                required_argument, 0,  'r' }, // CFG
            {"dtb",                         required_argument, 0,  'd' }, // CFG
            {"plic",                        required_argument, 0,  'p' }, // CFG
            {"clint",                       required_argument, 0,  'C' }, // CFG
            {"clear_ids",                         no_argument, 0,  'L' }, // CFG
            {"ctrlc",                             no_argument, 0,  'X' },
#ifdef LIVECACHE
            {"live_cache_size",          required_argument, 0,  'w' }, // CFG
#endif
            {0,                         0,                 0,  0 }
        };
        // clang-format on

        int c = getopt_long(argc, argv, "", long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
            case 'c':
                if (cmdline)
                    usage(prog, "already had a kernel command line");
                cmdline = strdup(optarg);
                break;

            case 'g': //Show supported extensions
                usage_isa();
                break;

            case 'h': // List options
                usage(prog, "Show usage");
                break;

            case 'H': // Show interactive command help
                usage_interactive();
                break;

            case 'i':
                march_string = strdup(optarg);
                break;

            case 'I':
                interactive = true;
                break;

            case 'j':
                show_enabled_extensions= true;
                break;

            case 'l':
                if (snapshot_load_name)
                    usage(prog, "already had a snapshot to load");
                snapshot_load_name = strdup(optarg);
                break;

            case 'n':
                if (ncpus != 0)
                    usage(prog, "already had a ncpus set");
                ncpus = atoll(optarg);
                break;

            case 's':
                if (snapshot_save_name)
                    usage(prog, "already had a snapshot to save");
                snapshot_save_name = strdup(optarg);
                break;

            case 'S':
                if (simpoint_file)
                    usage(prog, "already had a simpoint file");
                simpoint_file = strdup(optarg);
                break;

            case 'm':
                if (maxinsns)
                    usage(prog, "already had a max instructions");
                maxinsns = (uint64_t)atoll(optarg);
                {
                    char last = optarg[strlen(optarg) - 1];
                    if (last == 'k' || last == 'K')
                        maxinsns *= 1000;
                    else if (last == 'm' || last == 'M')
                        maxinsns *= 1000000;
                    else if (last == 'g' || last == 'G')
                        maxinsns *= 1000000000;
                }
                break;

            case 'T':
            case 't':
                if (exe_trace != UINT64_MAX)
                    usage(prog, "already had a trace set");
                exe_trace = (uint64_t)atoll(optarg);
                break;
            case 'q': exe_trace_file_name = strdup(optarg); break;

            case 'B': stf_memrecord_size_in_bits = true; break;
            case 'Z': stf_force_zero_sha = true; break;
            case 'a': stf_priv_modes = strdup(optarg); break;
            case 'e': stf_exit_on_stop_opc = true; break;
            case 'f': stf_disable_memory_records = true; break;
            case 'y': stf_trace_register_state = true; break;
            case 'z': stf_trace = strdup(optarg); break;

            case 'P': ignore_sbi_shutdown = true; break;
            case 'D': dump_memories = true; break;

            case 'M':
                if (optarg[0] == '0' && optarg[1] == 'x')
                    memory_size_override = strtoll(optarg + 2, NULL, 16);
                else
                    memory_size_override = atoi(optarg);
                break;

            case 'A':
                if (optarg[0] != '0' || optarg[1] != 'x')
                    usage(prog, "--memory_addr expects argument to start with 0x... ");
                memory_addr_override = strtoll(optarg + 2, NULL, 16);
                memory_addr_override_flag = true;
                break;

            case 'b':
                if (bootrom_name)
                    usage(prog, "already had a bootrom to load");
                bootrom_name = strdup(optarg);
                break;

            case 'd':
                if (dtb_name)
                    usage(prog, "already had a dtb to load");
                dtb_name = strdup(optarg);
                break;

            case 'o': compact_bootrom = true; break;

            case 'r':
                if (optarg[0] != '0' || optarg[1] != 'x')
                    usage(prog, "--reset_vector expects argument to start with 0x... ");
                reset_vector_override = strtoll(optarg + 2, NULL, 16);
                break;

            case 'p': {
                if (!strchr(optarg, ':'))
                    usage(prog, "--plic expects an argument like START:SIZE");

                char *copy           = strdup(optarg);
                char *plic_base_addr = strtok(copy, ":");
                char *plic_size      = strtok(NULL, ":");

                if (plic_base_addr[0] != '0' || plic_base_addr[1] != 'x')
                    usage(prog, "--plic START address must begin with 0x...");
                plic_base_addr_override = strtoll(plic_base_addr + 2, NULL, 16);

                if (plic_size[0] != '0' || plic_size[1] != 'x')
                    usage(prog, "--plic SIZE must begin with 0x...");
                plic_size_override = strtoll(plic_size + 2, NULL, 16);

                free(copy);
            } break;

            case 'C': {
                if (!strchr(optarg, ':'))
                    usage(prog, "--clint expects an argument like START:SIZE");

                char *copy            = strdup(optarg);
                char *clint_base_addr = strtok(copy, ":");
                char *clint_size      = strtok(NULL, ":");

                if (clint_base_addr[0] != '0' || clint_base_addr[1] != 'x')
                    usage(prog, "--clint START address must begin with 0x...");
                clint_base_addr_override = strtoll(clint_base_addr + 2, NULL, 16);

                if (clint_size[0] != '0' || clint_size[1] != 'x')
                    usage(prog, "--clint SIZE must begin with 0x...");
                clint_size_override = strtoll(clint_size + 2, NULL, 16);

                free(copy);
            } break;

            case 'u': custom_extension = true; break;

            case 'L': clear_ids = true; break;

#ifdef LIVECACHE
            case 'w':
                if (live_cache_size)
                    usage(prog, "already had a live_cache_size");
                live_cache_size = (uint64_t)atoll(optarg);
                {
                    char last = optarg[strlen(optarg) - 1];
                    if (last == 'k' || last == 'K')
                        live_cache_size *= 1000;
                    else if (last == 'm' || last == 'M')
                        live_cache_size *= 1000000;
                    else if (last == 'g' || last == 'G')
                        live_cache_size *= 1000000000;
                }
                break;
#endif
            case 'X':
                allow_ctrlc = true;
                break;

            default: usage(prog, "Unknown command line argument");
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "optin %d argc %d\n",optind,argc);
        usage(prog, "missing config file");
    } else {
        path = argv[optind++];
    }

/*
    if (optind < argc)
        usage(prog, "too many arguments");
*/

    assert(path);
    BlockDeviceModeEnum drive_mode = BF_MODE_SNAPSHOT;
    VirtMachineParams   p_s, *p = &p_s;

    virt_machine_set_defaults(p);
#ifdef CONFIG_FS_NET
    fs_wget_init();
#endif

    if (!load_elf_and_fake_the_config(p, path)) {
        virt_machine_load_config_file(p, path, NULL, NULL);
    } else {
        elf_based = true;
    }

    if (p->logfile) {
        FILE *log_out = fopen(p->logfile, "w");
        if (!log_out) {
            perror(p->logfile);
            exit(1);
        }
    }

#ifdef CONFIG_FS_NET
    fs_net_event_loop(NULL, NULL);
#endif

    /* override some config parameters */
    if (memory_addr_override_flag)
        p->ram_base_addr = memory_addr_override;
    if (memory_size_override)
        p->ram_size = memory_size_override << 20;

    if (ncpus)
        p->ncpus = ncpus;
    if (p->ncpus >= MAX_CPUS)
        usage(prog, "ncpus limit reached (MAX_CPUS). Increase MAX_CPUS");

    if (p->ncpus == 0)
        p->ncpus = 1;

    if (cmdline)
        vm_add_cmdline(p, cmdline);

    /* open the files & devices */
    for (int i = 0; i < p->drive_count; i++) {
        BlockDevice *drive;
        char *       fname;
        fname = get_file_path(p->cfg_filename, p->tab_drive[i].filename);
#ifdef CONFIG_FS_NET
        if (is_url(fname)) {
            net_completed = FALSE;
            drive = block_device_init_http(fname, 128 * 1024, 
                                           net_start_cb, NULL);
            /* wait until the drive is initialized */
            fs_net_event_loop(net_poll_cb, NULL);
        } else
#endif
        {
            drive = block_device_init(fname, drive_mode);
        }
        free(fname);
        p->tab_drive[i].block_dev = drive;
    }

    for (int i = 0; i < p->fs_count; i++) {
        FSDevice *  fs;
        const char *path;
        path = p->tab_fs[i].filename;
#ifdef CONFIG_FS_NET
        if (is_url(path)) {
            fs = fs_net_init(path, NULL, NULL);
            if (!fs)
                exit(1);
            fs_net_event_loop(NULL, NULL);
        } else
#endif
        {
#if defined(__APPLE__)
            fprintf(dromajo_stderr, "Filesystem access not supported yet\n");
            exit(1);
#else
            char *fname;
            fname = get_file_path(p->cfg_filename, path);
            fs    = fs_disk_init(fname);
            if (!fs) {
                fprintf(dromajo_stderr, "%s: must be a directory\n", fname);
                exit(1);
            }
            free(fname);
#endif
        }
        p->tab_fs[i].fs_dev = fs;
    }

    for (int i = 0; i < p->eth_count; i++) {
#ifdef CONFIG_SLIRP
        if (!strcmp(p->tab_eth[i].driver, "user")) {
            p->tab_eth[i].net = slirp_open();
            if (!p->tab_eth[i].net)
                exit(1);
        } else
#endif
#if !defined(__APPLE__)
            if (!strcmp(p->tab_eth[i].driver, "tap")) {
            p->tab_eth[i].net = tun_open(p->tab_eth[i].ifname);
            if (!p->tab_eth[i].net)
                exit(1);
        } else
#endif
        {
            fprintf(dromajo_stderr, "Unsupported network driver '%s'\n",
                    p->tab_eth[i].driver);
            exit(1);
        }
    }

    p->console       = console_init(allow_ctrlc, stdin, dromajo_stdout);
    p->dump_memories = dump_memories;

    // Setup bootrom params
    if (bootrom_name)
        p->bootrom_name = bootrom_name;
    if (dtb_name)
        p->dtb_name = dtb_name;
    p->compact_bootrom = compact_bootrom;

    // Setup particular reset vector
    if (reset_vector_override)
        p->reset_vector = reset_vector_override;

    // PLIC params
    if (plic_base_addr_override)
        p->plic_base_addr = plic_base_addr_override;
    if (plic_size_override)
        p->plic_size = plic_size_override;

    // CLINT params
    if (clint_base_addr_override)
        p->clint_base_addr = clint_base_addr_override;
    if (clint_size_override)
        p->clint_size = clint_size_override;

    // core modifications
    p->custom_extension = custom_extension;
    p->clear_ids        = clear_ids;

    RISCVMachine *s = virt_machine_init(p);
    if (!s)
        return NULL;

#ifdef LIVECACHE
    // LiveCache (should be ~2x larger than real LLC)
    s->llc = new LiveCache("LiveCache", live_cache_size, 
                           p->ram_base_addr, p->ram_size);
#endif

    if (elf_based) {
        for (int j = 0, i = optind - 1; i < argc; ++i, ++j) {
            uint8_t *buf;
            int      buf_len = load_file(&buf, argv[i]);

            if (elf64_is_riscv64(buf, buf_len)) {
                load_elf_image(s, buf, buf_len);
            } else
                load_hex_image(s, buf, buf_len);
        }
        for (int i = 0; i < (int)p->ncpus; ++i)
            s->cpu_state[i]->debug_mode = true;
    } else {
        s  = virt_machine_load(p, s);
        if (!s)
            return NULL;
    }

    // Overwrite the value specified in the configuration file
    if (snapshot_load_name) {
        s->common.snapshot_load_name = snapshot_load_name;
    }

    if (simpoint_file) {
#ifdef SIMPOINT_BB
        FILE *file = fopen(simpoint_file, "r");
        if (file == 0) {
            fprintf(stderr, "could not open simpoint file %s\n",
                    simpoint_file);
            exit(1);
        }
        int distance;
        int num;
        while (fscanf(file, "%d %d", &distance, &num) == 2) {
            uint64_t start = distance * SIMPOINT_SIZE;

            if (start == 0) {  // skip boot ROM
                start = ROM_SIZE;
            }

            s->common.simpoints.push_back({start, num});
        }

        std::sort(s->common.simpoints.begin(), s->common.simpoints.end());
        for (auto sp : s->common.simpoints) {
            printf("simpoint %d starts at %dK\n", sp.id, (int)sp.start / 1000);
        }

        if (s->common.simpoints.empty()) {
            fprintf(stderr, "simpoint file %s appears empty or invalid\n",
                    simpoint_file);
            exit(1);
        }
        s->common.simpoint_next = 0;
#else
        fprintf(stderr, "simpoint flag requires to recompile "
                        "with SIMPOINT_BB\n");
        exit(1);
#endif
    }

    (void) march_string;
//    //WTF: original code, s = RISCVMachine...
//    if(!parse_isa_string(march_string,*isa_flags)) {
//      fprintf(stderr, "Parsing --march string failed\n");
//      exit(1);
//    }
//
    (void) show_enabled_extensions;
//    if(show_enabled_extensions) {
//      printIsaConfigFlags(*isa_flags,false); //not verbose
//      exit(1);
//    }

    s->common.snapshot_save_name = snapshot_save_name;
    s->common.exe_trace          = exe_trace;

    if(exe_trace_file_name) {

      dromajo_trace = fopen(exe_trace_file_name,"w");

      if(dromajo_trace == NULL) {
            fprintf(stderr, "Could not open execution log file '%s'\n", 
                    exe_trace_file_name);
      }

    } 

    s->common.interactive = interactive;

    /* STF Trace Generation */
    auto get_stf_highest_priv_mode = [](const char * stf_priv_modes) -> int {
        if(strcmp(stf_priv_modes, "USHM") == 0) {
            return PRV_M;
        }
        else if(strcmp(stf_priv_modes, "USH") == 0) {
            return PRV_H;
        }
        else if(strcmp(stf_priv_modes, "US") == 0) {
            return PRV_S;
        }
        else if(strcmp(stf_priv_modes, "U") == 0) {
            return PRV_U;
        }
        else {
            fprintf(stderr, "invalid stf privilege modes '%s'\n", 
                    stf_priv_modes);
            exit(1);
        }
    };

    s->common.stf_trace                  = stf_trace;
    s->common.stf_exit_on_stop_opc       = stf_exit_on_stop_opc;
    s->common.stf_memrecord_size_in_bits = stf_memrecord_size_in_bits;
    s->common.stf_trace_register_state   = stf_trace_register_state;
    s->common.stf_disable_memory_records = stf_disable_memory_records;
    s->common.stf_highest_priv_mode      = get_stf_highest_priv_mode(stf_priv_modes);
    s->common.stf_force_zero_sha         = stf_force_zero_sha;

    s->common.stf_trace_open           = false;
    s->common.stf_in_traceable_region  = false;
    s->common.stf_tracing_enabled      = false;
    s->common.stf_is_start_opc         = false;
    s->common.stf_is_stop_opc          = false;
    s->common.stf_has_exit_pending     = false;

    s->common.stf_prog_asid = 0;
    s->common.stf_count     = 0;


    // Allow the command option argument to overwrite the value
    // specified in the configuration file
    if (maxinsns > 0) {
        s->common.maxinsns = maxinsns;
    }

    // If no value is specified in the configuration or the command line
    // then run indefinitely
    if (s->common.maxinsns == 0)
        s->common.maxinsns = UINT64_MAX;

    for (int i = 0; i < s->ncpus; ++i) {
        s->cpu_state[i]->ignore_sbi_shutdown = ignore_sbi_shutdown;
    }

    virt_machine_free_config(p);

    if (s->common.net)
        s->common.net->device_set_carrier(s->common.net, TRUE);

    if (s->common.snapshot_load_name)
        virt_machine_deserialize(s, s->common.snapshot_load_name);

    return s;
}
static void riscv_flush_tlb_write_range(void *opaque, uint8_t *ram_addr, size_t ram_size) {
    RISCVMachine *s = (RISCVMachine *)opaque;
    for (int i = 0; i < s->ncpus; ++i) riscv_cpu_flush_tlb_write_range_ram(s->cpu_state[i], ram_addr, ram_size);
}

void virt_machine_set_defaults(VirtMachineParams *p) {
    memset(p, 0, sizeof *p);
    p->physical_addr_len = PHYSICAL_ADDR_LEN_DEFAULT;
    p->ram_base_addr     = RAM_BASE_ADDR;
    p->reset_vector      = BOOT_BASE_ADDR;
    p->plic_base_addr    = PLIC_BASE_ADDR;
    p->plic_size         = PLIC_SIZE;
    p->clint_base_addr   = CLINT_BASE_ADDR;
    p->clint_size        = CLINT_SIZE;
}

RISCVMachine *global_virt_machine = 0;
uint8_t       dromajo_get_byte_direct(uint64_t paddr) {
    assert(global_virt_machine);  // needed to have a global map
    uint8_t *ptr = get_ram_ptr(global_virt_machine, paddr);
    if (ptr == NULL)
        return 0;

    return *ptr;
}

RISCVMachine *virt_machine_init(const VirtMachineParams *p) {

    VIRTIODevice *blk_dev;
    int           irq_num, i;
    VIRTIOBusDef  vbus_s, *vbus = &vbus_s;
    RISCVMachine *s = (RISCVMachine *)mallocz(sizeof *s);

    s->ram_size      = p->ram_size;
    s->ram_base_addr = p->ram_base_addr;

    s->mem_map = phys_mem_map_init();
    /* needed to handle the RAM dirty bits */
    s->mem_map->opaque                = s;
    s->mem_map->flush_tlb_write_range = riscv_flush_tlb_write_range;
    s->common.maxinsns                = p->maxinsns;
    s->common.snapshot_load_name      = p->snapshot_load_name;

    /* loggers are changed using install_new_loggers() in dromajo_cosim */
    s->common.debug_log = &dromajo_default_debug_log;
    s->common.error_log = &dromajo_default_error_log;

    s->ncpus = p->ncpus;

    /* setup reset vector for core
     * note: must be above riscv_cpu_init
     */
    s->reset_vector = p->reset_vector;

    /* have compact bootrom */
    s->compact_bootrom = p->compact_bootrom;

    /* add custom extension bit to misa */
    s->custom_extension = p->custom_extension;

    s->plic_base_addr  = p->plic_base_addr;
    s->plic_size       = p->plic_size;
    s->clint_base_addr = p->clint_base_addr;
    s->clint_size      = p->clint_size;
    /* clear mimpid, marchid, mvendorid */
    s->clear_ids = p->clear_ids;

    if (MAX_CPUS < s->ncpus) {
        vm_error("ERROR: ncpus:%d exceeds maximum MAX_CPU\n", s->ncpus);
        return NULL;
    }

    for (int i = 0; i < s->ncpus; ++i) {
        s->cpu_state[i] = riscv_cpu_init(s, i);
    }

    /* RAM */
    cpu_register_ram(s->mem_map, s->ram_base_addr, s->ram_size, 0);
    cpu_register_ram(s->mem_map, ROM_BASE_ADDR, ROM_SIZE, 0);

    for (int i = 0; i < s->ncpus; ++i) {
        s->cpu_state[i]->physical_addr_len = p->physical_addr_len;
    }

    SiFiveUARTState *uart = (SiFiveUARTState *)calloc(sizeof *uart, 1);
    uart->irq             = UART0_IRQ;
    uart->cs              = p->console;
    cpu_register_device(s->mem_map, UART0_BASE_ADDR, UART0_SIZE, uart, uart_read, uart_write, DEVIO_SIZE32);

    DW_apb_uart_state *dw_apb_uart = (DW_apb_uart_state *)calloc(sizeof *dw_apb_uart, 1);
    dw_apb_uart->irq               = &s->plic_irq[DW_APB_UART0_IRQ];
    dw_apb_uart->cs                = p->console;
    cpu_register_device(s->mem_map,
                        DW_APB_UART0_BASE_ADDR,
                        DW_APB_UART0_SIZE,
                        dw_apb_uart,
                        dw_apb_uart_read,
                        dw_apb_uart_write,
                        DEVIO_SIZE32 | DEVIO_SIZE16 | DEVIO_SIZE8);

    DW_apb_uart_state *dw_apb_uart1 = (DW_apb_uart_state *)calloc(sizeof *dw_apb_uart, 1);
    dw_apb_uart1->irq               = &s->plic_irq[DW_APB_UART1_IRQ];
    dw_apb_uart1->cs                = p->console;
    cpu_register_device(s->mem_map,
                        DW_APB_UART1_BASE_ADDR,
                        DW_APB_UART1_SIZE,
                        dw_apb_uart1,
                        dw_apb_uart_read,
                        dw_apb_uart_write,
                        DEVIO_SIZE32 | DEVIO_SIZE16 | DEVIO_SIZE8);

    cpu_register_device(s->mem_map,
                        p->clint_base_addr,
                        p->clint_size,
                        s,
                        clint_read,
                        clint_write,
                        DEVIO_SIZE32 | DEVIO_SIZE16 | DEVIO_SIZE8);
    cpu_register_device(s->mem_map, p->plic_base_addr, p->plic_size, s, plic_read, plic_write, DEVIO_SIZE32);

    for (int j = 1; j < 32; j++) {
        irq_init(&s->plic_irq[j], plic_set_irq, s, j);
    }

    s->htif_tohost_addr = p->htif_base_addr;

    s->common.console = p->console;

    memset(vbus, 0, sizeof(*vbus));
    vbus->mem_map = s->mem_map;
    vbus->addr    = VIRTIO_BASE_ADDR;
    irq_num       = VIRTIO_IRQ;

    /* virtio console */
    if (p->console && 0) {
        vbus->irq             = &s->plic_irq[irq_num];
        s->common.console_dev = virtio_console_init(vbus, p->console);
        vbus->addr += VIRTIO_SIZE;
        irq_num++;
        s->virtio_count++;
    }

    /* virtio net device */
    for (i = 0; i < p->eth_count; ++i) {
        vbus->irq = &s->plic_irq[irq_num];
        virtio_net_init(vbus, p->tab_eth[i].net);
        s->common.net = p->tab_eth[i].net;
        vbus->addr += VIRTIO_SIZE;
        irq_num++;
        s->virtio_count++;
    }

    /* virtio block device */
    for (i = 0; i < p->drive_count; ++i) {
        vbus->irq = &s->plic_irq[irq_num];
        blk_dev   = virtio_block_init(vbus, p->tab_drive[i].block_dev);
        (void)blk_dev;
        vbus->addr += VIRTIO_SIZE;
        irq_num++;
        s->virtio_count++;
        // virtio_set_debug(blk_dev, 1);
    }

    /* virtio filesystem */
    for (i = 0; i < p->fs_count; ++i) {
        VIRTIODevice *fs_dev;
        vbus->irq = &s->plic_irq[irq_num];
        fs_dev    = virtio_9p_init(vbus, p->tab_fs[i].fs_dev, p->tab_fs[i].tag);
        (void)fs_dev;
        vbus->addr += VIRTIO_SIZE;
        irq_num++;
        s->virtio_count++;
    }

    if (p->input_device) {
        if (!strcmp(p->input_device, "virtio")) {
            vbus->irq       = &s->plic_irq[irq_num];
            s->keyboard_dev = virtio_input_init(vbus, VIRTIO_INPUT_TYPE_KEYBOARD);
            vbus->addr += VIRTIO_SIZE;
            irq_num++;
            s->virtio_count++;

            vbus->irq    = &s->plic_irq[irq_num];
            s->mouse_dev = virtio_input_init(vbus, VIRTIO_INPUT_TYPE_TABLET);
            vbus->addr += VIRTIO_SIZE;
            irq_num++;
            s->virtio_count++;
        } else {
            vm_error("unsupported input device: %s\n", p->input_device);
            return NULL;
        }
    }

    if (!p->files[VM_FILE_BIOS].buf) {
        vm_error("No bios given\n");
        return NULL;
    } else if (copy_kernel(s,
                           p->files[VM_FILE_BIOS].buf,
                           p->files[VM_FILE_BIOS].len,
                           p->files[VM_FILE_KERNEL].buf,
                           p->files[VM_FILE_KERNEL].len,
                           p->files[VM_FILE_INITRD].buf,
                           p->files[VM_FILE_INITRD].len,
                           p->bootrom_name,
                           p->dtb_name,
                           p->cmdline))
        return NULL;

    /* interrupts and exception setup for cosim */
    s->common.cosim             = false;
    s->common.pending_exception = -1;
    s->common.pending_interrupt = -1;

    /* plic/clint setup */
    s->plic_base_addr  = p->plic_base_addr;
    s->plic_size       = p->plic_size;
    s->clint_base_addr = p->clint_base_addr;
    s->clint_size      = p->clint_size;

    return s;
}

RISCVMachine *virt_machine_load(const VirtMachineParams *p, RISCVMachine *s) {
    if (!p->files[VM_FILE_BIOS].buf) {
        vm_error("No bios given\n");
        return NULL;
    } else if (copy_kernel(s,
                           p->files[VM_FILE_BIOS].buf,
                           p->files[VM_FILE_BIOS].len,
                           p->files[VM_FILE_KERNEL].buf,
                           p->files[VM_FILE_KERNEL].len,
                           p->files[VM_FILE_INITRD].buf,
                           p->files[VM_FILE_INITRD].len,
                           p->bootrom_name,
                           p->dtb_name,
                           p->cmdline))
        return NULL;

    if (p->dump_memories) {
        FILE *f = fopen("BootRAM.hex", "w+");
        if (!f) {
            vm_error("dromajo: %s: %s\n", "BootRAM.hex", strerror(errno));
            return NULL;
        }

        uint8_t *ram_ptr = get_ram_ptr(s, ROM_BASE_ADDR);
        for (int i = 0; i < ROM_SIZE / 4; ++i) {
            uint32_t *q_base = (uint32_t *)(ram_ptr + (BOOT_BASE_ADDR - ROM_BASE_ADDR));
            fprintf(f, "@%06x %08x\n", i, q_base[i]);
        }

        fclose(f);

        {
            FILE *f[16] = {0};

            char hexname[60];
            for (int i = 0; i < 16; ++i) {
                snprintf(hexname, sizeof hexname, "memImage_d%crow%d_%s.hex", "we"[i / 4 % 2], i % 4, i / 8 == 0 ? "even" : "odd");
                f[i] = fopen(hexname, "w");
                if (!f[i]) {
                    vm_error("dromajo: %s: %s\n", hexname, strerror(errno));
                    return NULL;
                }
            }

            dump_dram(s, f, "firmware", s->ram_base_addr, p->files[VM_FILE_BIOS].len);
            dump_dram(s, f, "kernel", s->ram_base_addr + KERNEL_OFFSET, p->files[VM_FILE_KERNEL].len);
            dump_dram(s, f, "initrd", s->initrd_start, p->files[VM_FILE_INITRD].len);

            for (int i = 0; i < 16; ++i) {
                fclose(f[i]);
            }
        }
    }

    global_virt_machine = s;

    return s;
}

void virt_machine_end(RISCVMachine *s) {
    if (s->common.snapshot_save_name)
        virt_machine_serialize(s, s->common.snapshot_save_name);

    /* XXX: stop all */
    for (int i = 0; i < s->ncpus; ++i) {
        riscv_cpu_end(s->cpu_state[i]);
    }

    phys_mem_map_end(s->mem_map);
    free(s);
}

void virt_machine_serialize(RISCVMachine *m, const char *dump_name) {
    RISCVCPUState *s = m->cpu_state[0];  // FIXME: MULTICORE

    vm_error("plic: %x %x timecmp=%llx\n", m->plic_pending_irq, m->plic_served_irq, (unsigned long long)s->timecmp);

    assert(m->ncpus == 1);  // FIXME: riscv_cpu_serialize must be patched for multicore
    riscv_cpu_serialize(s, dump_name, m->clint_base_addr);
}

void virt_machine_deserialize(RISCVMachine *m, const char *dump_name) {
    RISCVCPUState *s = m->cpu_state[0];  // FIXME: MULTICORE

    assert(m->ncpus == 1);  // FIXME: riscv_cpu_serialize must be patched for multicore
    riscv_cpu_deserialize(s, dump_name);
}

int virt_machine_get_sleep_duration(RISCVMachine *m, int hartid, int ms_delay) {
    RISCVCPUState *s = m->cpu_state[hartid];
    int64_t        ms_delay1;

    /* wait for an event: the only asynchronous event is the RTC timer */
    if (!(riscv_cpu_get_mip(s) & MIP_MTIP) && rtc_get_time(m) > 0) {
        ms_delay1 = s->timecmp - rtc_get_time(m);
        if (ms_delay1 <= 0) {
            riscv_cpu_set_mip(s, MIP_MTIP);
            ms_delay = 0;
        } else {
            /* convert delay to ms */
            ms_delay1 = ms_delay1 / (RTC_FREQ / 1000);
            if (ms_delay1 < ms_delay)
                ms_delay = ms_delay1;
        }
    }

    if (!riscv_cpu_get_power_down(s))
        ms_delay = 0;

    return ms_delay;
}

uint64_t virt_machine_get_pc(RISCVMachine *s, int hartid) { return riscv_get_pc(s->cpu_state[hartid]); }

uint64_t virt_machine_get_reg(RISCVMachine *s, int hartid, int rn) { return riscv_get_reg(s->cpu_state[hartid], rn); }

uint64_t virt_machine_get_fpreg(RISCVMachine *s, int hartid, int rn) { return riscv_get_fpreg(s->cpu_state[hartid], rn); }

const char *virt_machine_get_name(void) { return "riscv64"; }

void vm_send_key_event(RISCVMachine *s, BOOL is_down, uint16_t key_code) {
    if (s->keyboard_dev) {
        virtio_input_send_key_event(s->keyboard_dev, is_down, key_code);
    }
}

BOOL vm_mouse_is_absolute(RISCVMachine *s) { return TRUE; }

void vm_send_mouse_event(RISCVMachine *s, int dx, int dy, int dz, unsigned buttons) {
    if (s->mouse_dev) {
        virtio_input_send_mouse_event(s->mouse_dev, dx, dy, dz, buttons);
    }
}


