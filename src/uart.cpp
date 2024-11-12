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

#include "block_device.h"
#include "cutils.h"
#include "dromajo.h"
#include "dromajo_protos.h"
#include "dw_apb_uart.h"
#include "elf64.h"
#include "iomem.h"
#include "options.h"
#include "riscv_machine.h"
#include "termio.h"
#include "uart.h"
#include "virtio.h"

#include <assert.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>


void uart_update_irq(SiFiveUARTState *s) {
    int cond = 0;
    if ((s->ie & SIFIVE_UART_IE_RXWM) && s->rx_fifo_len) {
        cond = 1;
    }
    if (cond) {
        vm_error("uart_update_irq: FIXME we should raise IRQ saying that there is new data\n");
    }
}

uint32_t uart_read(void *opaque, uint32_t offset, int size_log2) {
    SiFiveUARTState *s = (SiFiveUARTState *)opaque;

#ifdef DUMP_UART
    vm_error("uart_read: offset=%x size_log2=%d\n", offset, size_log2);
#endif
    switch (offset) {
        case SIFIVE_UART_RXFIFO: {
            CharacterDevice *cs = s->cs;
            unsigned char    r;
            int              ret = cs->read_data(cs->opaque, &r, 1);
            if (ret) {
#ifdef DUMP_UART
                vm_error("uart_read: val=%x\n", r);
#endif
                return r;
            }
            return 0x80000000;
        }
        case SIFIVE_UART_TXFIFO: return 0; /* Should check tx fifo */
        case SIFIVE_UART_IE: return s->ie;
        case SIFIVE_UART_IP: return s->rx_fifo_len ? SIFIVE_UART_IP_RXWM : 0;
        case SIFIVE_UART_TXCTRL: return s->txctrl;
        case SIFIVE_UART_RXCTRL: return s->rxctrl;
        case SIFIVE_UART_DIV: return s->div;
    }

    vm_error("%s: bad read: offset=0x%x\n", __func__, (int)offset);
    return 0;
}

void uart_write(void *opaque, uint32_t offset, uint32_t val, int size_log2) {
    SiFiveUARTState *s  = (SiFiveUARTState *)opaque;
    CharacterDevice *cs = s->cs;
    unsigned char    ch = val;

#ifdef DUMP_UART
    vm_error("uart_write: offset=%x val=%x size_log2=%d\n", offset, val, size_log2);
#endif

    switch (offset) {
        case SIFIVE_UART_TXFIFO: cs->write_data(cs->opaque, &ch, 1); return;
        case SIFIVE_UART_IE:
            s->ie = val;
            uart_update_irq(s);
            return;
        case SIFIVE_UART_TXCTRL: s->txctrl = val; return;
        case SIFIVE_UART_RXCTRL: s->rxctrl = val; return;
        case SIFIVE_UART_DIV: s->div = val; return;
    }

    vm_error("%s: bad write: addr=0x%x v=0x%x\n", __func__, (int)offset, (int)val);
}

