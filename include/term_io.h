// -------------------------------------------------------------------------
// Copyright (C) 2023-2024, Condor Computing Corporation
// See dromajo_main.cpp for license notice.
// -------------------------------------------------------------------------
// Split from the original dromajo_main
// -------------------------------------------------------------------------
#pragma once
#include "virtio.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <termios.h>

typedef struct {
    FILE *stdin, *out;
    int   console_esc_state;
    BOOL  resize_pending;
} STDIODevice;

extern void term_exit(void);
extern void term_init(bool allow_ctrlc);
extern void console_write(void *opaque, const uint8_t *buf, int len);
extern int console_read(void *opaque, uint8_t *buf, int len);
extern void term_resize_handler(int sig);
extern CharacterDevice *console_init(bool allow_ctrlc, FILE *stdin, FILE *out);
