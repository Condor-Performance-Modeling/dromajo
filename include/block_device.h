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

typedef enum {
    BF_MODE_RO,
    BF_MODE_RW,
    BF_MODE_SNAPSHOT,
} BlockDeviceModeEnum;

typedef struct BlockDeviceFile {
    FILE *              f;
    int64_t             nb_sectors;
    BlockDeviceModeEnum mode;
    uint8_t **          sector_table;
} BlockDeviceFile;

extern int64_t bf_get_sector_count(BlockDevice *bs);

extern int bf_read_async(BlockDevice *bs, uint64_t sector_num, uint8_t *buf, 
                         int n, BlockDeviceCompletionFunc *cb, void *opaque);

extern int bf_write_async(BlockDevice *bs, uint64_t sector_num, 
                         const uint8_t *buf, int n,
                         BlockDeviceCompletionFunc *cb, void *opaque) ;

extern BlockDevice *block_device_init(const char *filename,
                         BlockDeviceModeEnum mode);
