// -------------------------------------------------------------------------
// Copyright (c) 2016-2017 Fabrice Bellard
// Copyright (C) 2017,2018,2019, Esperanto Technologies Inc.
// Copyright (C) 2023-2024, Condor Computing Corporation
//
// See dromajo_main.cpp for license notice.
// -------------------------------------------------------------------------
// Split from the original dromajo_main
// -------------------------------------------------------------------------
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

