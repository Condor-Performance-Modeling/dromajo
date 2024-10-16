/*
 * RISCV emulator
 *
 * Copyright (c) 2016-2017 Fabrice Bellard
 * Copyright (C) 2017,2018,2019, Esperanto Technologies Inc.
 * Copyright (C) 2023-2024, Condor Computing Corporation
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
#include "cutils.h"
#include "elf64.h"
#include "iomem.h"
#include "network.h"
#include "options.h"
#include "riscv_machine.h"
#include "term_io.h"
#include "virtio.h"

#include "dromajo.h"
#include "dromajo_sha.h"
#include "dromajo_stf.h"
#include "dromajo_isa.h"

#ifdef CONFIG_FS_NET
#include "fs_utils.h"
#include "fs_wget.h"
#endif

#include <getopt.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <csignal>

FILE *dromajo_trace;
FILE *dromajo_stdout;
FILE *dromajo_stderr;

BOOL virt_machine_run(RISCVMachine *s, int hartid, int n_cycles) {
    (void)virt_machine_get_sleep_duration(s, hartid, MAX_SLEEP_TIME);

    riscv_cpu_interp64(s->cpu_state[hartid], n_cycles);
    RISCVCPUState *cpu = s->cpu_state[hartid];
    if (s->htif_tohost_addr) {
        uint32_t tohost;
        bool     fail = true;
        tohost        = riscv_phys_read_u32(s->cpu_state[hartid], s->htif_tohost_addr, &fail);
        if (!fail && tohost & 1) {
            if (tohost != 1)
                cpu->benchmark_exit_code = tohost;
            return false;
        }
    }

    return !riscv_terminated(s->cpu_state[hartid]) && s->common.maxinsns > 0;
}

void launch_alternate_executable(char **argv) {
    char        filename[1024];
    char        new_exename[64];
    const char *p, *exename;
    int         len;

    snprintf(new_exename, sizeof(new_exename), "dromajo64");
    exename = argv[0];
    p       = strrchr(exename, '/');
    if (p) {
        len = p - exename + 1;
    } else {
        len = 0;
    }
    if (len + strlen(new_exename) > sizeof(filename) - 1) {
        fprintf(dromajo_stderr, "%s: filename too long\n", exename);
        exit(1);
    }
    memcpy(filename, exename, len);
    filename[len] = '\0';
    strcat(filename, new_exename);
    argv[0] = filename;

    if (execvp(argv[0], argv) < 0) {
        perror(argv[0]);
        exit(1);
    }
}

#ifdef CONFIG_FS_NET
static BOOL net_completed;
static void net_start_cb(void *arg) { net_completed = TRUE; }
static BOOL net_poll_cb(void *arg) { return net_completed; }
#endif

static bool load_elf_and_fake_the_config(VirtMachineParams *p, const char *path) {
    uint8_t *buf;
    int      buf_len = load_file(&buf, path);

    if (elf64_is_riscv64(buf, buf_len) || isxdigit(buf[0]) && isxdigit(buf[1])) {
        /* Fake the corresponding config file */
        p->files[VM_FILE_BIOS].filename = strdup(path);
        p->files[VM_FILE_BIOS].buf      = buf;
        p->files[VM_FILE_BIOS].len      = buf_len;
        p->ram_size    = (size_t)256 << 20;  // Default to 256 MiB
        p->ram_base_addr                = RAM_BASE_ADDR;
        elf64_find_global(buf, buf_len, "tohost", &p->htif_base_addr);

        return true;
    }

    free(buf);

    return false;
}

RISCVMachine *virt_machine_main(int argc, char **argv) {
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

    long        memory_size_override       = 0;
    uint64_t    memory_addr_override       = 0;
    bool        memory_addr_override_flag  = false;
    bool        ignore_sbi_shutdown        = false;
    bool        dump_memories              = false;

    char *      bootrom_name               = 0;
    char *      dtb_name                   = 0;
    bool        compact_bootrom            = false;
    uint64_t    reset_vector_override      = 0;
    uint64_t    plic_base_addr_override    = 0;
    uint64_t    plic_size_override         = 0;
    uint64_t    clint_base_addr_override   = 0;
    uint64_t    clint_size_override        = 0;
    bool        custom_extension           = false;
    const char *simpoint_file              = 0;
    bool        clear_ids                  = false;
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

    if(!parse_isa_string(march_string,s->common.ext_flags)) {
      fprintf(stderr, "Parsing --march string failed\n");
      exit(1);
    }

    if(show_enabled_extensions) {
      printExtensionFlags(s->common.ext_flags,false); //not verbose
      exit(1);
    }

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
