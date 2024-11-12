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
#include <cassert>
#include <cstdlib>
#include <string.h>

#define SECTOR_SIZE 512UL

int64_t bf_get_sector_count(BlockDevice *bs) {
    BlockDeviceFile *bf = (BlockDeviceFile *)bs->opaque;
    return bf->nb_sectors;
}

//#define DUMP_BLOCK_READ

int bf_read_async(BlockDevice *bs, uint64_t sector_num, uint8_t *buf, int n, BlockDeviceCompletionFunc *cb, void *opaque) {
    BlockDeviceFile *bf = (BlockDeviceFile *)bs->opaque;
#ifdef DUMP_BLOCK_READ
    {
        static FILE *f;
        if (!f)
            f = fopen("/tmp/read_sect.txt", "wb");
        fprintf(f, "%" PRId64 " %d\n", sector_num, n);
    }
#endif
    if (!bf->f)
        return -1;
    if (bf->mode == BF_MODE_SNAPSHOT) {
        int i;
        for (i = 0; i < n; i++) {
            if (!bf->sector_table[sector_num]) {
                fseek(bf->f, sector_num * SECTOR_SIZE, SEEK_SET);
                size_t got = fread(buf, 1, SECTOR_SIZE, bf->f);
                (void) got; // Make GCC happy
                assert(got == SECTOR_SIZE);
            } else {
                memcpy(buf, bf->sector_table[sector_num], SECTOR_SIZE);
            }
            sector_num++;
            buf += SECTOR_SIZE;
        }
    } else {
        fseek(bf->f, sector_num * SECTOR_SIZE, SEEK_SET);
        size_t got = fread(buf, 1, n * SECTOR_SIZE, bf->f);
        (void) got; // Make GCC happy
        assert(got == n * SECTOR_SIZE);
    }
    /* synchronous read */
    return 0;
}

int bf_write_async(BlockDevice *bs, uint64_t sector_num, const uint8_t *buf, int n, BlockDeviceCompletionFunc *cb,
                          void *opaque) {
    BlockDeviceFile *bf = (BlockDeviceFile *)bs->opaque;
    int              ret;

    switch (bf->mode) {
        case BF_MODE_RO:
            ret = -1; /* error */
            break;
        case BF_MODE_RW:
            fseek(bf->f, sector_num * SECTOR_SIZE, SEEK_SET);
            fwrite(buf, 1, n * SECTOR_SIZE, bf->f);
            ret = 0;
            break;
        case BF_MODE_SNAPSHOT: {
            if ((unsigned int)(sector_num + n) > bf->nb_sectors)
                return -1;
            for (int i = 0; i < n; i++) {
                if (!bf->sector_table[sector_num]) {
                    bf->sector_table[sector_num] = (uint8_t *)malloc(SECTOR_SIZE);
                }
                memcpy(bf->sector_table[sector_num], buf, SECTOR_SIZE);
                sector_num++;
                buf += SECTOR_SIZE;
            }
            ret = 0;
        } break;
        default: abort();
    }

    return ret;
}

BlockDevice *block_device_init(const char *filename, BlockDeviceModeEnum mode) {
    const char *mode_str;

    if (mode == BF_MODE_RW) {
        mode_str = "r+b";
    } else {
        mode_str = "rb";
    }

    FILE *f = fopen(filename, mode_str);
    if (!f) {
        perror(filename);
        exit(1);
    }
    fseek(f, 0, SEEK_END);
    int64_t file_size = ftello(f);

    BlockDevice *    bs = (BlockDevice *)mallocz(sizeof *bs);
    BlockDeviceFile *bf = (BlockDeviceFile *)mallocz(sizeof *bf);

    bf->mode       = mode;
    bf->nb_sectors = file_size / 512;
    bf->f          = f;

    if (mode == BF_MODE_SNAPSHOT) {
        bf->sector_table = (uint8_t **)mallocz(sizeof(bf->sector_table[0]) * bf->nb_sectors);
    }

    bs->opaque           = bf;
    bs->get_sector_count = bf_get_sector_count;
    bs->read_async       = bf_read_async;
    bs->write_async      = bf_write_async;
    return bs;
}

