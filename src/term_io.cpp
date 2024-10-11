// -------------------------------------------------------------------------
// Copyright (c) 2016-2017 Fabrice Bellard
// Copyright (C) 2017,2018,2019, Esperanto Technologies Inc.
// Copyright (C) 2023-2024, Condor Computing Corporation
//
// See dromajo_main.cpp for license notice.
// -------------------------------------------------------------------------
// Split from the original dromajo_main
// -------------------------------------------------------------------------

#include "term_io.h"

#include <termios.h>

extern FILE *dromajo_stderr;

static struct termios oldtty;
static int            old_fd0_flags;
static STDIODevice *  global_stdio_device;

void term_exit(void)
{
  tcsetattr(0, TCSANOW, &oldtty);
  fcntl(0, F_SETFL, old_fd0_flags);
}

void term_init(bool allow_ctrlc)
{
  struct termios tty;

  memset(&tty, 0, sizeof(tty));
  tcgetattr(0, &tty);
  oldtty        = tty;
  old_fd0_flags = fcntl(0, F_GETFL);

  tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP 
                  | INLCR | IGNCR | ICRNL | IXON);
  tty.c_oflag |= OPOST;
  tty.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN);
  if (!allow_ctrlc) {
    tty.c_lflag &= ~ISIG;
  }
  tty.c_cflag &= ~(CSIZE | PARENB);
  tty.c_cflag |= CS8;
  tty.c_cc[VMIN]  = 1;
  tty.c_cc[VTIME] = 0;

  tcsetattr(0, TCSANOW, &tty);
  atexit(term_exit);
}

void console_write(void *opaque, const uint8_t *buf, int len)
{
  STDIODevice *s = (STDIODevice *)opaque;
  fwrite(buf, 1, len, s->out);
  fflush(s->out);
}

int console_read(void *opaque, uint8_t *buf, int len)
{
    STDIODevice *s = (STDIODevice *)opaque;

    if (len <= 0)
        return 0;

    int ret = fread(buf, len, 1, s->stdin);
    if (ret <= 0)
        return 0;

    int j = 0;
    for (int i = 0; i < ret; i++) {
        uint8_t ch = buf[i];
        if (s->console_esc_state) {
            s->console_esc_state = 0;
            switch (ch) {
                case 'x': fprintf(dromajo_stderr, "Terminated\n"); exit(0);
                case 'h':
                    fprintf(dromajo_stderr,
                            "\n"
                            "C-b h   print this help\n"
                            "C-b x   exit emulator\n"
                            "C-b C-b send C-b\n");
                    break;
                case 1: goto output_char;
                default: break;
            }
        } else {
            if (ch == 2) {  // Change to work with tmux
                s->console_esc_state = 1;
            } else {
            output_char:
                buf[j++] = ch;
            }
        }
    }

    return j;
}

void term_resize_handler(int sig)
{
  if (global_stdio_device) global_stdio_device->resize_pending = TRUE;
}

CharacterDevice *console_init(bool allow_ctrlc, FILE *stdin, FILE *out)
{
  term_init(allow_ctrlc);

  CharacterDevice *dev = (CharacterDevice *)mallocz(sizeof *dev);
  STDIODevice *    s   = (STDIODevice *)mallocz(sizeof *s);
  s->stdin             = stdin;
  s->out               = out;

  /* Note: the glibc does not properly tests the return value of
     write() in printf, so some messages on out may be lost */
  fcntl(fileno(s->stdin), F_SETFL, O_NONBLOCK);

  s->resize_pending   = TRUE;
  global_stdio_device = s;

  /* use a signal to get the host terminal resize events */
  struct sigaction sig;
  sig.sa_handler = term_resize_handler;
  sigemptyset(&sig.sa_mask);
  sig.sa_flags = 0;
  sigaction(SIGWINCH, &sig, NULL);

  dev->opaque     = s;
  dev->write_data = console_write;
  dev->read_data  = console_read;
  return dev;
}

