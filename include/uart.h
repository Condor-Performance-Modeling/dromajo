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

//#define DUMP_UART
//#define DUMP_CLINT
//#define DUMP_HTIF
//#define DUMP_PLIC
//#define DUMP_DTB

//#define USE_SIFIVE_UART

enum {
    SIFIVE_UART_TXFIFO = 0,
    SIFIVE_UART_RXFIFO = 4,
    SIFIVE_UART_TXCTRL = 8,
    SIFIVE_UART_TXMARK = 10,
    SIFIVE_UART_RXCTRL = 12,
    SIFIVE_UART_RXMARK = 14,
    SIFIVE_UART_IE     = 16,
    SIFIVE_UART_IP     = 20,
    SIFIVE_UART_DIV    = 24,
    SIFIVE_UART_MAX    = 32
};

enum {
    SIFIVE_UART_IE_TXWM = 1, /* Transmit watermark interrupt enable */
    SIFIVE_UART_IE_RXWM = 2  /* Receive watermark interrupt enable */
};

enum {
    SIFIVE_UART_IP_TXWM = 1, /* Transmit watermark interrupt pending */
    SIFIVE_UART_IP_RXWM = 2  /* Receive watermark interrupt pending */
};

typedef struct SiFiveUARTState {
    CharacterDevice *cs;  // Console
    uint32_t         irq;
    uint8_t          rx_fifo[8];
    unsigned int     rx_fifo_len;
    uint32_t         ie;
    uint32_t         ip;
    uint32_t         txctrl;
    uint32_t         rxctrl;
    uint32_t         div;
} SiFiveUARTState;

// sifive,uart, same as qemu UART0 (qemu has 2 sifive uarts)
#ifdef ARIANE_UART
#define UART0_BASE_ADDR 0x10000000
#define UART0_SIZE      0x1000
#else
#define UART0_BASE_ADDR 0x54000000
#define UART0_SIZE      32
#endif
#define UART0_IRQ       3

