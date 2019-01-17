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

#ifndef __LORA_MODULE_CERTIFICATION_H__
#define __LORA_MODULE_CERTIFICATION_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Exported constants --------------------------------------------------------*/

#define CERTIF_PORT 224
/* Exported types ------------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
bool lorawan_certif_running(void);

void lorawan_certif_DownLinkIncrement( void );

void lorawan_certif_linkCheck(MlmeConfirm_t *mlmeConfirm);

void lorawan_certif_rx(McpsIndication_t *mcpsIndication);

void lorawan_certif_tx( void );

uint8_t lorawan_certif_uplink_cycle(void);
#ifdef __cplusplus
}
#endif

#endif /*__LORA_MODULE_CERTIFICATION_H__*/
