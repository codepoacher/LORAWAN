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

/* lora_config.h Define to prevent recursive inclusion -----------------------*/

#ifndef __LORA_CONFIG_H__
#define __LORA_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "LoRaMac.h"

/* Exported constants --------------------------------------------------------*/

#ifndef ACTIVE_REGION
#define ACTIVE_REGION LORAMAC_REGION_CN470
#endif

/*!
 * LoRaWAN application port
 */
#define LORAWAN_APP_PORT                            (100)

/*!
 * LoRaWAN application max size
 */
#define LORAWAN_APP_DATA_MAX_SIZE                   (242)

/*!
 * dynamic multicast command port
 */
#define LORA_MULTICAST_CMD_PORT                     (200)

/*!
 * monitor node command port
 */
#define LORA_MONITOR_NODE_CMD_PORT                   (199)

/*!
 * Uncomment to use the deprecated BeaconTiming MAC command
 */
//#define USE_BEACON_TIMING

/*!
 * Defines the application data transmission duty cycle. 10 minutes, value in [ms].
 */
#define APP_TX_DUTYCYCLE                            (60*1000*10) //APP send data in every 10 minutes, must be greater than beacon period 128s.

/*!
 * Defines a random delay for application data transmission duty cycle. 5s,
 * value in [ms].
 */
#define APP_TX_DUTYCYCLE_RND                        5000

/*!
 * Default datarate
 */
#define LORAWAN_DEFAULT_DATARATE                    DR_3

/*!
 * Lowest datarate
 */
#define LORAWAN_LOWEST_DATARATE                     DR_2

/*!
 * Default ping slots periodicity
 *
 * \remark periodicity is equal to 2^LORAWAN_DEFAULT_PING_SLOT_PERIODICITY seconds
 *         example: 2^3 = 8 seconds. The end-device will open an Rx slot every 8 seconds.
 */
#define LORAWAN_DEFAULT_PING_SLOT_PERIODICITY       2

/*!
 * LoRaWAN confirmed messages
 */
#define LORAWAN_CONFIRMED_MSG_ON                    false

/*!
 * LoRaWAN Adaptive Data Rate
 *
 * \remark Please note that when ADR is enabled the end-device should be static
 */
#define LORAWAN_ADR_ON                              true //default: false;true:just for LinkWAN Certification pass

#define LORAWAN_DUTY_CYCLE_RESTRICT_ON              false
//=====================Commissioning.h=========================
typedef enum node_freq_mode_s {
    FREQ_MODE_INTRA = 1, // uplink and downlink use same frequencies
    FREQ_MODE_INTER = 2, // uplink and downlink use different frequencies
} LoRaNodeFreqMode_t;

#define LORA_NODE_WORKMODE                           NODE_WORK_MODE_NORMAL
#define LORA_NODE_TARGET_CLASS_TYPE                  CLASS_B
#define LORA_NODE_FREQ_TYPE                          FREQ_MODE_INTRA

#define LORA_JOIN_FREQBAND_MASK                      (0x0003)//Default:0x0003(1A1+1A2),0x000C(2A1+2A2),0x3000(3B1+3B2),0xC000(4B1+4B2).
#define LORA_STORE_FREQBAND                          (1) //1A2
#define LORA_DEF_FREQBAND                            (1) //1A2

//===========start radio parameters config==================

#define LORA_JOIN_TIMEOUT_VALUE                      (35000) //35s

#define LORA_MODULE_SEND_HEARTBEAT                    1 //1: enable cycle data send, 0:disable

#define LORA_NODE_CONFIRM_RETX_NUM                   (2)

#define LORA_NODE_JOIN_MIN_INTERVAL                  (8)
#define LORA_NODE_JOIN_MAX_INTERVAL                  (10)
#define LORA_NODE_NUM_SEND_FAILED_TO_REJOIN          (5)
#define LORA_NODE_NUM_REJOIN_FAILED_TO_REINIT        (6)
#define LORA_NODE_NUM_CLASS_SWITCH_RETRY_CNT         (5)

#define LORA_NODE_NUM_PINGSLOTINFORACK_FAILED_TO_REINIT (5)
#define LORA_NODE_NUM_BEACONMISS_TO_SEND_DEVICETIMEREQ  (3)

#define LORA_NODE_DISABLE_JOIN_BACKOFF               (1)
#define LORA_NODE_DISABLE_CLASS_SWITCH_BACKOFF       (1)
#define LORA_MODULE_GET_FLASH_EUI_ENABLE             (0)

#define LORA_NODE_STROED_JOIN_RETRY_CNT              (3)
#define LORA_NODE_DEF_JOIN_RETRY_CNT                 (3)
#define LORA_NODE_SCAN_JOIN_RETRY_CNT                (3)
#define LORAWAN_LBT_ENABLE                           (false)
#define LORAWAN_LBT_MAX_RETRY                        (8)

//Work Mode Repeater Param Config
#define LORA_NODE_REPEATER_MODE_RX_IQINVERTED        (false)
#define LORA_NODE_REPEATER_MODE_TX_IQINVERTED        (true)
#define LORA_NODE_REPEATER_MODE_NUM_REPORT_DEVADDR   (5)
#define LORA_NODE_REPEATER_CHANNEL_DR                DR_3
#define LORA_NODE_REPEATER_MODE_RX1DELAY_PLUS         (1000) //ms
#define LORA_NODE_REPEATER_MODE_RX2DELAY_PLUS         (2000) //ms

//===========start radio parameters config end==================

/* Exported types ------------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

#endif
