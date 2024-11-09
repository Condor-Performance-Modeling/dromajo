/*
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
#pragma once
#include <cstdint>
#include <cstdio>
#include "virtio.h"

#ifndef MAX_EXEC_CYCLE
#define MAX_EXEC_CYCLE 1
#endif

#ifndef MAX_SLEEP_TIME
#define MAX_SLEEP_TIME 10 /* in ms */
#endif

extern FILE* dromajo_stderr;

#if !defined(__APPLE__)
typedef struct {
    int  fd;
    BOOL select_filled;
} TunState;

extern void tun_write_packet(EthernetDevice *net, const uint8_t *buf, int len);
extern void tun_select_fill(EthernetDevice *net, int *pfd_max, fd_set *rfds, fd_set *wfds, fd_set *efds, int *pdelay);
extern void tun_select_poll(EthernetDevice *net, fd_set *rfds, fd_set *wfds, fd_set *efds, int select_ret);
extern EthernetDevice *tun_open(const char *ifname);
#endif

#ifdef CONFIG_SLIRP
extern void slirp_write_packet(EthernetDevice *net, const uint8_t *buf, int len);
extern int slirp_can_output(void *opaque);
extern void slirp_output(void *opaque, const uint8_t *pkt, int pkt_len);
extern void slirp_select_fill1(EthernetDevice *net, int *pfd_max, fd_set *rfds, fd_set *wfds, fd_set *efds, int *pdelay);
extern void slirp_select_poll1(EthernetDevice *net, fd_set *rfds, fd_set *wfds, fd_set *efds, int select_ret);
extern EthernetDevice *slirp_open(void);

#endif

