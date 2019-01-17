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

#ifndef LORA_MODULE_DEVICE_STAT_FLASH_MANAGER_H
#define LORA_MODULE_DEVICE_STAT_FLASH_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdint.h>
#include "lora_config.h"

/* init */
extern int lora_module_dev_stat_init(void);

extern int lora_module_dev_stat_get_devEUI(uint8_t *buf, int buf_len);
extern int lora_module_dev_stat_get_appKey(uint8_t *buf, int buf_len);
extern int lora_module_dev_stat_get_appEUI(uint8_t *buf, int buf_len);
extern int lora_module_dev_stat_get_freqband_dr(int8_t *freqband, uint8_t *dr);
extern int lora_module_dev_stat_get_freqmode(LoRaNodeFreqMode_t *freq_mode);
extern int lora_module_dev_stat_get_freqband_mask(uint16_t *freqband_mask);
extern int lora_module_dev_stat_get_class(uint8_t *class);

extern int lora_module_dev_stat_set_devEUI(uint8_t *buf, int buf_len);
extern int lora_module_dev_stat_set_appKey(uint8_t *buf, int buf_len);
extern int lora_module_dev_stat_set_appEUI(uint8_t *buf, int buf_len);
extern int lora_module_dev_stat_set_freqband_dr(int8_t freqband, uint8_t dr);
extern int lora_module_dev_stat_set_freqmode(int8_t freq_mode);
extern int lora_module_dev_stat_set_freqband_mask(uint16_t freqband_mask);

extern int lora_module_dev_stat_set_lora_dev(void);
extern int lora_module_dev_stat_set_class(uint8_t class);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LORA_MODULE_DEVICE_STATUS_H */
