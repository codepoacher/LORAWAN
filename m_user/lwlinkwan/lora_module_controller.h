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
#ifndef LORA_MODULE_CONTROLLER_H
#define LORA_MODULE_CONTROLLER_H

#include "connection.h"
#include "lora_module_common.h"
#include "LoRaMacMulticast.h"

#define osOK                    (1) //cmsis_os.h

/*!
 * Device states
 */
typedef enum eDeviceState {
    DEVICE_STATE_REINIT,
    DEVICE_STATE_RESTORE,
    DEVICE_STATE_START,
    DEVICE_STATE_JOIN,
    DEVICE_STATE_REJOIN,
    DEVICE_STATE_SEND,
    DEVICE_STATE_REQ_DEVICE_TIME,
    DEVICE_STATE_REQ_PINGSLOT_ACK,
    DEVICE_STATE_REQ_BEACON_TIMING,
    DEVICE_STATE_BEACON_ACQUISITION,
    DEVICE_STATE_SWITCH_CLASS,
    DEVICE_STATE_CYCLE,
    DEVICE_STATE_SLEEP
} DeviceState_t;

typedef enum eErrNo {
    ERRNO_NOT_JOIN = 1,
    ERRNO_SEND_FAIL = 2,
    ERRNO_DATA_EXCEED = 3
} ErrNo_t;

extern uint8_t DevEui[8];
extern uint8_t JoinEui[8];
extern uint8_t AppKey[16];
extern uint8_t NwkKey[16];

extern uint8_t FNwkSIntKey[];
extern uint8_t SNwkSIntKey[];
extern uint8_t NwkSEncKey[];
extern uint8_t AppSKey[];
extern uint32_t DevAddr;

extern uint16_t       g_lora_freqband_mask;
extern uint8_t        g_lora_pingslot_period;
extern DeviceClass_t  g_lora_current_class;
extern DeviceClass_t  g_current_target_lora_class_type;
extern NodeWorkMode_t g_lora_current_work_mode;
extern bool           g_lora_is_work_mode_autoswitch;
extern bool           g_monitor_node_feature_open;

extern uint8_t        g_lora_app_tx_report_mode;
extern uint32_t       g_lora_app_tx_dutycycle;

extern uint16_t       g_lora_num_send_uplink;
extern uint16_t       g_lora_num_received_data_ack;
extern LoRaModuleStatus_t    g_lora_control_current_status;

extern int            g_lora_rx_rssi;
extern int            g_lora_rx_snr;

extern uint8_t        g_lora_is_confirmup;
extern uint8_t        g_lora_nb_confirmup_trails;
extern bool           g_lora_auto_join;
extern uint8_t        g_lora_app_port;
extern uint8_t        g_lora_port;
extern uint8_t        g_lora_app_data_size;
extern uint8_t        g_lora_app_data_size_backup;
extern uint8_t        g_lora_app_rx_data_size;
extern uint8_t        g_lora_app_data_buffer[LORAWAN_APP_DATA_MAX_SIZE];
extern uint8_t        g_lora_app_rx_data_buffer[LORAWAN_APP_DATA_MAX_SIZE];
extern uint8_t        g_default_datarate;
extern bool           g_auto_linkcheck;
extern int16_t        g_lora_channel_rssi[];
extern bool           g_lora_proprietary_enable;

extern mcChannel_t    g_mc_channels[LORAMAC_MAX_MC_CTX];
extern mcKey_t        g_mc_keys[LORAMAC_MAX_MC_CTX];

typedef struct ChannelRssiSnr_s {
    int16_t rssi;
    int8_t snr;
} ChannelRssiSnr_t;

extern ChannelRssiSnr_t g_lora_channel_rssi_snr[];

/*!
 * \brief   register APP's callback functions
 *
 * \param   [in] pointer to callback
 *
 * \retval  Possible returns are:
 *           0: succesful.
 */
extern int lorawan_control_init(esl_connection_ops_cb_t *_esl_cm_ops_cb, void *data);
/*!
 * \brief   unregister APP's callback functions
 * \retval  Possible returns are:
 *           0: succesful.
 */
extern int lorawan_control_deinit(void);
/*!
 * \brief   lorawan control core process
 *
 * \retval  Possible returns are:
 *           ref DeviceState_t
 */
extern DeviceState_t lorawan_control_process(void);
/*!
 * \brief   let lorawan_control to make a uplink send
 */
extern void lorawan_control_trigger_send( void );
/*!
 * \brief   let lorawan_control strat to join by OTAA
 * \param   [in] mode: 1->start join, 0->stop join
 * \retval  Possible returns are:
 *           0: succesful.
 *          -1: fail.
 */
extern int lorawan_control_join(int mode);
/*!
 * \brief   get join status
 * \retval  Possible returns are:
 *           0: not joined;
 *           1: joined;
 */
extern int lorawan_control_get_join_status(void);
/*!
 * \brief   let lorawan control to send a uplink data
 *
 * \param   [in] is_confirm: 1->confirmUp, 0->unconfirmUp
 *          [in] app_port: 1~199
 *          [in] data: pointer to data buffer to send
 *          [in] len: length of data to send
 *          [in] cb: not support now!!!
 *
 * \retval  Possible returns are:
 *           0: succesful.
 *           -1: fail
 */
extern int lorawan_control_send(int is_confirm, int app_port, const char *data, int len, handle_send_cb_fp_t cb);
/*!
 * \brief   set node work mode
 *
 * \param   [in] mode:NODE_WORK_MODE_NORMAL,NODE_WORK_MODE_REPEATER,NODE_WORK_MODE_AUTOSWITCH
 *
 * \retval  Possible returns are:
 *           0: succesful.
 *           other:fail
 */
extern int lorawan_control_set_work_mode(NodeWorkMode_t mode);
/*!
 * \brief   get node work mode
 *
 * \retval  Possible returns are:
 *           NODE_WORK_MODE_NORMAL,NODE_WORK_MODE_REPEATER,NODE_WORK_MODE_AUTOSWITCH.
 */
extern NodeWorkMode_t lorawan_control_get_work_mode(void);
/*!
 * \brief   start class type switch
 *
 * \param   [in] newClass:CLASS_A,CLASS_B,CLASS_C
 *
 * \retval  Possible returns are:
 *           ref LoRaModuleStatus_t.
 */
extern LoRaModuleStatus_t    lorawan_control_switch_class(DeviceClass_t newClass);

/*!
 * \brief   lorawan multicast initialization
 *
 * \details  Initialize the multicast SDK, and set preset multicast channels and
 *          dynamic multicast channels.User can change the function to voluntarily
 *          assign the multicast channel.
 *
 * \retval  Possible returns are:
 *          -1: fail.
 *           0: succesful.
 */
extern int lorawan_control_mc_setup(void);

/*!
 * \brief   get freq mode
 *
 * \retval  Possible returns are:
 *           FREQ_MODE_INTRA
 *           FREQ_MODE_INTER
 */
extern LoRaNodeFreqMode_t lorawan_control_get_freq_mode(void);

/*!
 * \brief   set freq mode
 *
 * \param   [in] mode
 *
 * \retval  Possible returns are:
 *          -1: fail.
 *           0: succesful.
 */
extern int lorawan_control_set_freq_mode(LoRaNodeFreqMode_t mode);

/*!
 * \brief   get join mode
 *
 * \details Check if OTAA or ABP
 *
 * \retval  Possible returns are:
 *          JOIN_MODE_OTAA
 *          JOIN_MODE_ABP
 */
extern JoinMode_t lorawan_control_get_join_mode(void);

/*!
 * \brief   set join mode
 *
 * \param   [IN] join mode
 *
 * \details set OTAA mode or ABP mode
 *
 * \retval  Possible returns are:
 *           0: succesful.
 */
extern int lorawan_control_set_join_mode(JoinMode_t mode);
#endif /* LORA_MODULE_CONTROLLER_H */
