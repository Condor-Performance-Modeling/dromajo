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

