/*
 * Copyright (c) 2018 Alibaba Group. All rights reserved.
 * License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "utilities.h"
#include "lora_module_device_stat_flash_manager.h"
#include "Commissioning.h"
#include "lora_config.h"
#include "nvmm.h"

#define DEVEUI_LEN     (8)
#define APPEUI_LEN     (8)
#define APPKEY_LEN     (16)

typedef struct {
    /* do NOT modify */
    uint8_t dev_eui[DEVEUI_LEN];
    uint8_t app_eui[APPEUI_LEN];
    uint8_t app_key[APPKEY_LEN];
    int8_t store_freqband;
    int8_t data_rate;
    int8_t freq_mode;
    uint16_t freqband_mask;
    uint8_t class;
    //int8_t to_used[3];
} lora_dev_t;

static lora_dev_t g_lora_dev = {  LORAWAN_DEVICE_EUI,
                                  LORAWAN_JOIN_EUI,
                                  LORAWAN_APP_KEY,
                                  LORA_STORE_FREQBAND,
                                  LORAWAN_DEFAULT_DATARATE,
                                  LORA_NODE_FREQ_TYPE,
                                  LORA_JOIN_FREQBAND_MASK,
                                  LORA_NODE_TARGET_CLASS_TYPE
                               };

static NvmmDataBlock_t g_lora_dev_data_block;
static int write_lora_dev(lora_dev_t *lora_dev);

/******************************************************************************/
/*  init */
/******************************************************************************/
int lora_module_dev_stat_init(void)
{
    int ret;

    ret = NvmmDeclare(&g_lora_dev_data_block, sizeof(lora_dev_t));

    if (ret == NVMM_FAIL_CHECKSUM) {
        PRINTF("lora: NVMM fail checksum, reset state\r\n");
        ret = write_lora_dev(&g_lora_dev);
    } else if (ret != NVMM_SUCCESS) {
        PRINTF("lora: NvmmDeclare lora dev ret = %d\r\n", ret);
        ret = -1;
    }

    return ret;
}

static int _check_parameter(const uint8_t *buf, int buf_len)
{
    return (buf == NULL || buf_len == 0) ? -1 : 0;
}


static int read_lora_dev(lora_dev_t *lora_dev)
{
    int ret;

    memset(lora_dev, 0, sizeof(lora_dev_t));

    ret = NvmmRead(&g_lora_dev_data_block, lora_dev, sizeof(lora_dev_t));
    if (ret != NVMM_SUCCESS) {
        memcpy(lora_dev, &g_lora_dev, sizeof(lora_dev_t));
        return -1;
    }

    return 0;
}

static int write_lora_dev(lora_dev_t *lora_dev)
{
    int ret;

    ret = NvmmWrite(&g_lora_dev_data_block, lora_dev, sizeof(lora_dev_t));

    if (ret != NVMM_SUCCESS) {
        PRINTF("lora: NvmmWrite ret = %d\r\n", ret);
        return -1;
    }

    return 0;
}

/******************************************************************************/
/*  triple */
/******************************************************************************/
static int dev_stat_set_triple(uint8_t *from, int from_len, int dst_offset, int dst_len)
{
    int ret;
    lora_dev_t lora_dev;

    if (from_len != dst_len) {
        return -1;
    }

    if (_check_parameter(from, from_len) != 0) {
        return -1;
    }

    read_lora_dev(&lora_dev);
    memcpy((uint8_t *)&lora_dev + dst_offset, from, from_len);
    ret = write_lora_dev(&lora_dev);

    return ret;

}

static int dev_stat_get_triple(uint8_t *to, int to_len, int from_offset, int from_len)
{
    lora_dev_t lora_dev;

    if (to_len != from_len) {
        return -1;
    }

    if (_check_parameter(to, to_len) != 0) {
        return -1;
    }

    read_lora_dev(&lora_dev);
    memcpy(to, (uint8_t *)&lora_dev + from_offset, to_len);

    return 0;
}

int lora_module_dev_stat_set_devEUI(uint8_t *buf, int buf_len)
{
    return dev_stat_set_triple(buf, buf_len, 0, DEVEUI_LEN);
}

int lora_module_dev_stat_set_appEUI(uint8_t *buf, int buf_len)
{
    return dev_stat_set_triple(buf, buf_len, DEVEUI_LEN, APPEUI_LEN);
}

int lora_module_dev_stat_set_appKey(uint8_t *buf, int buf_len)
{
    return dev_stat_set_triple(buf, buf_len, DEVEUI_LEN + APPEUI_LEN, APPKEY_LEN);
}

int lora_module_dev_stat_get_devEUI(uint8_t *buf, int buf_len)
{
    return dev_stat_get_triple(buf, buf_len, 0, DEVEUI_LEN);
}

int lora_module_dev_stat_get_appEUI(uint8_t *buf, int buf_len)
{
    return dev_stat_get_triple(buf, buf_len, DEVEUI_LEN, APPEUI_LEN);
}

int lora_module_dev_stat_get_appKey(uint8_t *buf, int buf_len)
{
    return dev_stat_get_triple(buf, buf_len, DEVEUI_LEN + APPEUI_LEN, APPKEY_LEN);
}

/******************************************************************************/
/*  other */
/******************************************************************************/
int lora_module_dev_stat_get_freqband_dr(int8_t *freqband, uint8_t *dr)
{
    lora_dev_t lora_dev;

    if (freqband == NULL || dr == NULL) {
        return -1;
    }

    read_lora_dev(&lora_dev);

    if (lora_dev.store_freqband >= 0 && lora_dev.store_freqband < 16) {
        *freqband = lora_dev.store_freqband;
    } else {
        *freqband = LORA_STORE_FREQBAND;
    }

    if (lora_dev.data_rate >= LORAWAN_LOWEST_DATARATE && lora_dev.data_rate <= LORAWAN_DEFAULT_DATARATE) {
        *dr = lora_dev.data_rate;
    } else {
        *dr = LORAWAN_DEFAULT_DATARATE;
    }

    return 0;
}

int lora_module_dev_stat_set_freqband_dr(int8_t freqband, uint8_t dr)
{
    int ret;
    lora_dev_t lora_dev;

    if (read_lora_dev(&lora_dev) != 0) {
        return -1;
    }

    /* if freqband is same as result in flash, skip it */
    if (lora_dev.store_freqband == freqband && lora_dev.data_rate == dr) {
        return 0;
    }

    lora_dev.store_freqband = freqband;
    lora_dev.data_rate = dr;

    ret = NvmmWrite(&g_lora_dev_data_block, &lora_dev, sizeof(lora_dev_t));

    if (ret != NVMM_SUCCESS) {
        PRINTF("lora: NvmmWrite freqband ret = %d\r\n", ret);
        return -1;
    }
    return 0;
}

int lora_module_dev_stat_get_freqmode(LoRaNodeFreqMode_t *freq_mode)
{
    lora_dev_t lora_dev;

    if (freq_mode == NULL) {
        return -1;
    }

    read_lora_dev(&lora_dev);

    if (lora_dev.freq_mode == FREQ_MODE_INTRA && lora_dev.freq_mode == FREQ_MODE_INTER) {
        *freq_mode = lora_dev.freq_mode;
    } else {
        *freq_mode = LORA_NODE_FREQ_TYPE;
    }

    return 0;
}

int lora_module_dev_stat_set_freqmode(int8_t freq_mode)
{
    int ret;
    lora_dev_t lora_dev;

    if (read_lora_dev(&lora_dev) != 0) {
        return -1;
    }

    /* if freqmode is same as result in flash, skip it */
    if (lora_dev.freq_mode == freq_mode) {
        return 0;
    }

    lora_dev.freq_mode = freq_mode;

    ret = write_lora_dev(&lora_dev);

    return ret;
}

int lora_module_dev_stat_get_freqband_mask(uint16_t *freqband_mask)
{
    lora_dev_t lora_dev;

    if (freqband_mask == NULL) {
        return -1;
    }

    read_lora_dev(&lora_dev);

    if ((lora_dev.freqband_mask | 0xF00F) == 0xF00F) {
        *freqband_mask = lora_dev.freqband_mask;
    } else {
        *freqband_mask = LORA_JOIN_FREQBAND_MASK;
    }

    return 0;
}

int lora_module_dev_stat_set_freqband_mask(uint16_t freqband_mask)
{
    int ret;
    lora_dev_t lora_dev;

    if (read_lora_dev(&lora_dev) != 0) {
        return -1;
    }

    /* if freqband_mask is same as result in flash, skip it */
    if (lora_dev.freqband_mask == freqband_mask) {
        return 0;
    }

    lora_dev.freqband_mask = freqband_mask;

    ret = write_lora_dev(&lora_dev);
    return ret;
}

int lora_module_dev_stat_set_lora_dev(void)
{
    int ret;

    ret = write_lora_dev(&g_lora_dev);
    return ret;
}

int lora_module_dev_stat_get_class(uint8_t *class)
{
    lora_dev_t lora_dev;

    if (class == NULL) {
        return -1;
    }

    read_lora_dev(&lora_dev);

    if (lora_dev.class <= CLASS_C) {
        *class = lora_dev.class;
    } else {
        *class = LORA_NODE_TARGET_CLASS_TYPE;
    }

    return 0;
}

int lora_module_dev_stat_set_class(uint8_t class)
{
    int ret;
    lora_dev_t lora_dev;

    if (read_lora_dev(&lora_dev) != 0) {
        return -1;
    }

    /* if class is same as result in flash, skip it */
    if (lora_dev.class == class) {
        return 0;
    }

    lora_dev.class = class;

    ret = write_lora_dev(&lora_dev);
    return ret;
}
