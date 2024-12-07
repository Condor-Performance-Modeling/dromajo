/*
 * RISCV emulator
 *
 * Copyright (c) 2016-2017 Fabrice Bellard
 * Copyright (C) 2017,2018,2019, Esperanto Technologies Inc.
 * Contribution (C) 2023-2024, Jeff Nye
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

#include "elf64.h"
#include "network.h"
#include "riscv_machine.h"

#ifdef CONFIG_FS_NET
#include "fs_utils.h"
#include "fs_wget.h"
#endif

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

bool load_elf_and_fake_the_config(VirtMachineParams *p, const char *path) {
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

