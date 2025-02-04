/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Hash functions.
 */
#if (OMV_ENABLE_HASH == 1)
#include STM32_HAL_H
#include <stdbool.h>
#include <ff_wrapper.h>
#include <stdio.h>
#include "fb_alloc.h"
#include "hash.h"

#define BLOCK_SIZE    (512)
static HASH_HandleTypeDef hhash;

int hash_start() {
    if (HAL_HASH_DeInit(&hhash) != HAL_OK) {
        return -1;
    }
    hhash.Init.DataType = HASH_DATATYPE_8B;
    HAL_HASH_Init(&hhash);
}

int hash_update(uint8_t *buffer, uint32_t size) {
    if ((size % 4) != 0) {
        return -1;
    }

    if (HAL_HASH_MD5_Accumulate(&hhash, buffer, size) != HAL_OK) {
        return -1;
    }

    return 0;
}

int hash_digest(uint8_t *buffer, uint32_t size, uint8_t *digest) {
    if (HAL_HASH_MD5_Start(&hhash, buffer, size, digest, 0xFF) != HAL_OK) {
        return -1;
    }

    return 0;
}

int hash_from_file(const char *path, uint8_t *digest) {
    FIL fp;
    uint32_t offset = 0;
    UINT bytes = 0;
    UINT bytes_out = 0;

    int ret = -1;
    uint8_t *buf = fb_alloc(BLOCK_SIZE, FB_ALLOC_NO_HINT);

    if (f_open_helper(&fp, path, FA_READ | FA_OPEN_EXISTING) != FR_OK) {
        goto error;
    }

    // File size
    uint32_t size = f_size(&fp);

    while (size) {
        // Read a block.
        bytes = MIN(size, BLOCK_SIZE);
        if (f_read(&fp, buf, bytes, &bytes_out) != FR_OK || bytes != bytes_out) {
            printf("hash_from_file: file read error!\n");
            goto error;
        }

        // Accumulate buffer.
        if ((size - bytes) > 0) {
            ret = hash_update(buf, bytes);
        } else {
            // last block, get digest.
            ret = hash_digest(buf, bytes, digest);
        }

        if (ret != 0) {
            printf("hash_from_file: hash_update/digest failed!\n");
            goto error;
        }

        size -= bytes;
        offset += bytes;
    }

    ret = 0;

error:
    fb_free();
    f_close(&fp);
    return ret;
}
#endif //OMV_ENABLE_HASH == 1
