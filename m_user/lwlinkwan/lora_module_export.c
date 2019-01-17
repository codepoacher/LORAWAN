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

#include <string.h>
#include "LoRaMac.h"
#include "lora_module_export.h"
#include "lora_module_controller.h"
#include "lora_module_device_stat_flash_manager.h"
#include "Commissioning.h"
#include "board.h"

static int is_lora_module_init(void)
{
    LoRaModuleStatus_t status;

    status = lorawan_module_get_status();

    if (status == LoRa_Init_Done ||
        status == LoRa_Initing ||
        status == LoRa_Join_Failed) {
        return 1;
    } else {
        return 0;
    }
}

int lorawan_module_get_join_status(void)
{
    return lorawan_control_get_join_status();
}

int lorawan_module_set_duty_cycle(uint32_t duty_cycle)
{
    if (duty_cycle < 5000) {
        duty_cycle = 5000;
    }

    g_lora_app_tx_dutycycle = duty_cycle;
    return 0;
}

uint32_t lorawan_module_get_duty_cycle(void)
{
    return g_lora_app_tx_dutycycle;
}

int lorawan_module_set_dev_eui(uint8_t *eui)
{
    memcpy(DevEui, eui, 8);
    return 0;
}

void lorawan_module_get_dev_eui(uint8_t *eui)
{
    memcpy(eui, DevEui, 8);
}

int lorawan_module_set_app_eui(uint8_t *eui)
{
    memcpy(JoinEui, eui, 8);
    return 0;
}

void lorawan_module_get_app_eui(uint8_t *eui)
{
    memcpy(eui, JoinEui, 8);
}

int lorawan_module_set_app_key(uint8_t *key)
{
    memcpy(AppKey, key, 16);
    memcpy(NwkKey, key, 16);
    return 0;
}

void lorawan_module_get_app_key(uint8_t *key)
{
    memcpy(key, AppKey, 16);
}

int lorawan_module_get_rx_window_params(uint8_t *RX1DRoffset, uint8_t *RX2DataRate,
                                        uint32_t *RX2Frequency)
{
    MibRequestConfirm_t mib_req;
    LoRaMacStatus_t     status;

    mib_req.Type = MIB_RX1_DROFFSET;
    status       = LoRaMacMibGetRequestConfirm(&mib_req);
    if (status == LORAMAC_STATUS_OK) {
        *RX1DRoffset = mib_req.Param.Rx1DrOffset;
    } else {
        return -1;
    }
    mib_req.Type = MIB_RX2_CHANNEL;
    status       = LoRaMacMibGetRequestConfirm(&mib_req);
    if (status == LORAMAC_STATUS_OK) {
        *RX2DataRate  = mib_req.Param.Rx2Channel.Datarate;
        *RX2Frequency = mib_req.Param.Rx2Channel.Frequency;
    } else {
        return -1;
    }
    return 0;
}

int lorawan_module_set_rx_window_params(uint8_t RX1DRoffset, uint8_t RX2DR)
{
    MibRequestConfirm_t mib_req;
    LoRaMacStatus_t     status;
    uint32_t RX2Freq;

    mib_req.Type              = MIB_RX1_DROFFSET;
    mib_req.Param.Rx1DrOffset = RX1DRoffset;
    status                    = LoRaMacMibSetRequestConfirm(&mib_req);
    if (status != LORAMAC_STATUS_OK) {
        return -1;
    }

    mib_req.Type = MIB_RX2_CHANNEL;
    status       = LoRaMacMibGetRequestConfirm(&mib_req);
    if (status == LORAMAC_STATUS_OK) {
        RX2Freq = mib_req.Param.Rx2Channel.Datarate;
    } else {
        return -1;
    }

    mib_req.Type                       = MIB_RX2_CHANNEL;
    mib_req.Param.Rx2Channel.Datarate  = RX2DR;
    mib_req.Param.Rx2Channel.Frequency = RX2Freq;
    status                             = LoRaMacMibSetRequestConfirm(&mib_req);

    if (status != LORAMAC_STATUS_OK) {
        return -1;
    }

    return 0;
}

uint32_t lorawan_module_get_mac_rx1_delay(void)
{
    MibRequestConfirm_t mib_req;
    LoRaMacStatus_t     status;

    mib_req.Type = MIB_RECEIVE_DELAY_1;
    status = LoRaMacMibGetRequestConfirm(&mib_req);
    if (status == LORAMAC_STATUS_OK) {
        return mib_req.Param.ReceiveDelay1;
    } else {
        return 0;
    }
}

int lorawan_module_set_mac_rx1_delay(long rx1delay)
{
    MibRequestConfirm_t mib_req;
    LoRaMacStatus_t     status;

    mib_req.Type                = MIB_RECEIVE_DELAY_1;
    mib_req.Param.ReceiveDelay1 = rx1delay;
    status                      = LoRaMacMibSetRequestConfirm(&mib_req);
    if (status != LORAMAC_STATUS_OK) {
        return -1;
    }

    mib_req.Type                = MIB_RECEIVE_DELAY_2;
    mib_req.Param.ReceiveDelay2 = rx1delay + 1000;
    status                      = LoRaMacMibSetRequestConfirm(&mib_req);
    if (status == LORAMAC_STATUS_OK) {
        return 0;
    } else {
        return -1;
    }
}

int lorawan_module_set_devaddr(uint8_t *devaddr)
{
    memcpy(&DevAddr, devaddr, 4);
    return 0;
}

uint8_t *lorawan_module_get_devaddr(void)
{
    return (uint8_t *)&DevAddr;
}

int lorawan_module_set_appskey(uint8_t *buf)
{
    memcpy(AppSKey, buf, 16);
    return 0;
}

uint8_t *lorawan_module_get_appskey(void)
{
    return AppSKey;
}

int lorawan_module_set_nwkskey(uint8_t *buf)
{
    memcpy(FNwkSIntKey, buf, 16);
    memcpy(SNwkSIntKey, buf, 16);
    memcpy(NwkSEncKey, buf, 16);
    return 0;
}

uint8_t *lorawan_module_get_nwkskey(void)
{
    return FNwkSIntKey;
}

int lorawan_module_set_freqband_mask(uint16_t mask)
{
    if ((mask | 0xF00F) != 0xF00F) {
        return -1;
    }

    g_lora_freqband_mask = mask;
    return 0;
}

uint16_t lorawan_module_get_freqband_mask(void)
{
    return g_lora_freqband_mask;
}

int lorawan_module_set_app_port(uint8_t port)
{
    if (port < 1 || port > 199) {
        return -1;
    }

    g_lora_app_port = port;

    return 0;
}

uint8_t lorawan_module_get_app_port(void)
{
    return g_lora_app_port;
}

mcChannel_t *lorawan_module_get_cur_multicast(void)
{
    return g_mc_channels;
}

mcKey_t *lorawan_module_get_cur_mc_key(void)
{
    return g_mc_keys;
}

int lorawan_module_set_multicast(uint32_t dev_addr, uint8_t *mc_key, uint32_t freq, int8_t dr, uint16_t period)
{
    int i;
    mcChannel_t *multiCastNode = lorawan_module_get_cur_multicast();

    if (multiCastNode == NULL) {
        return -1;
    }

    /* if same addr found, return -1 */
    for (i = 0; i < LORAMAC_MAX_MC_CTX; i++) {
        if (multiCastNode[i].Address != 0 && multiCastNode[i].Address == dev_addr) {
            return -1;
        }
    }

    for (i = 0; i < LORAMAC_MAX_MC_CTX; i++) {
        if (multiCastNode[i].Address == 0)  {
            multiCastNode[i].Address = dev_addr;
            multiCastNode[i].Frequency = freq;
            multiCastNode[i].Datarate = dr;
            multiCastNode[i].Periodicity = period;
            memcpy(g_mc_keys[i].val, mc_key, 16);
            lorawan_control_mc_setup();
            return 0;
        }
    }

    return -1;
}

int lorawan_module_del_multicast(uint32_t dev_addr)
{
    int i;
    mcChannel_t *multiCastNode = lorawan_module_get_cur_multicast();

    if (multiCastNode == NULL) {
        return -1;
    }

    for (i = 0; i < LORAMAC_MAX_MC_CTX; i++) {
        if (multiCastNode[i].Address == dev_addr)  {
            multiCastNode[i].Address = 0;
            lorawan_control_mc_setup();
            return 0;
        }
    }

    return -1;
}

uint8_t lorawan_module_get_mulitcast_num(void)
{
    uint8_t num = 0;
    int i;

    mcChannel_t *multiCastNode = lorawan_module_get_cur_multicast();
    if (multiCastNode == NULL) {
        return 0;
    }

    for (i = 0; i <  LORAMAC_MAX_MC_CTX; i++) {
        if (multiCastNode[i].Address != 0)  {
            num++;
        }
    }

    return num;
}

int lorawan_module_get_report_mode(void)
{
    return g_lora_app_tx_report_mode;
}

int lorawan_module_set_report_mode(int8_t mode)
{
    if (mode != 0 && mode != 1) {
        return -1;
    }

    g_lora_app_tx_report_mode = mode;

    if (g_lora_app_tx_report_mode == 1) {
        lorawan_control_trigger_send();
    }

    return 0;
}

int8_t lorawan_module_get_tx_power(void)
{
    MibRequestConfirm_t mib_req;
    LoRaMacStatus_t     status;

    mib_req.Type = MIB_CHANNELS_TX_POWER;
    status       = LoRaMacMibGetRequestConfirm(&mib_req);
    if (status == LORAMAC_STATUS_OK) {
        return mib_req.Param.ChannelsTxPower;
    } else {
        return -1;
    }
}

int lorawan_module_set_tx_power(int8_t pwr)
{
    MibRequestConfirm_t mib_req;
    LoRaMacStatus_t     status;

    mib_req.Type                  = MIB_CHANNELS_TX_POWER;
    mib_req.Param.ChannelsTxPower = pwr;
    status                        = LoRaMacMibSetRequestConfirm(&mib_req);
    if (status == LORAMAC_STATUS_OK) {
        return 0;
    } else {
        return -1;
    }
}

int lorawan_module_set_tx_cfm_flag(int confirmed)
{
    g_lora_is_confirmup = confirmed;
    return 0;
}

int lorawan_module_get_tx_cfm_flag(void)
{
    return g_lora_is_confirmup;
}

int lorawan_module_set_tx_cfm_trials(uint8_t type, uint8_t trials)
{
    if (type == 0) {
        MibRequestConfirm_t mib_req;
        LoRaMacStatus_t status;

        mib_req.Type = MIB_CHANNELS_NB_TRANS;
        mib_req.Param.ChannelsNbTrans = trials;
        status       = LoRaMacMibSetRequestConfirm(&mib_req);
        if (status == LORAMAC_STATUS_OK) {
            return 0;
        } else {
            return -1;
        }
    } else {
        g_lora_nb_confirmup_trails = trials;
        return 0;
    }
}

uint8_t lorawan_module_get_tx_cfm_trials(uint8_t type)
{
    if (type == 0) {
        MibRequestConfirm_t mib_req;
        LoRaMacStatus_t status;
        uint8_t unconfirm_trails = 0;

        mib_req.Type = MIB_CHANNELS_NB_TRANS;
        status       = LoRaMacMibGetRequestConfirm(&mib_req);
        if (status == LORAMAC_STATUS_OK) {
            unconfirm_trails = mib_req.Param.ChannelsNbTrans;
        }
        return unconfirm_trails;
    } else if (type == 1) {
        return g_lora_nb_confirmup_trails;
    } else {
        return 0;
    }
}

int lorawan_module_set_class(int8_t class)
{
    if (class != CLASS_A && class != CLASS_B && class != CLASS_C) {
        return -1;
    }

    g_current_target_lora_class_type = class;

    /* if not join, don't switch class */
    if (lorawan_module_get_join_status() == 0) {
        return 0;
    }

    if (lorawan_module_switch_class(class) == LoRa_Class_SwitchNOK) {
        return -1;
    } else {
        return 0;
    }
}

int8_t lorawan_module_get_class(void)
{
    return g_current_target_lora_class_type;
}

void lorawan_module_get_channel_rssi_snr(int16_t *channel_rssi, int8_t *channel_snr)
{
    for (uint8_t i = 0; i < 8; i++) {
        channel_rssi[i] = g_lora_channel_rssi_snr[i].rssi;
        channel_snr[i] = g_lora_channel_rssi_snr[i].snr;
    }
}

void lorawan_module_reboot(int8_t mode)
{
    BoardResetMcu();
}

LoRaNodeFreqMode_t lorawan_module_get_freq_mode(void)
{
    return lorawan_control_get_freq_mode();
}

int lorawan_module_set_freq_mode(LoRaNodeFreqMode_t mode)
{
    if (!is_lora_module_init()) {
        return -1;
    }

    return lorawan_control_set_freq_mode(mode);
}

int lorawan_module_set_tx_datarate(int8_t datarate)
{
    int min_tx_dr;
    int max_tx_dr;
    GetPhyParams_t getPhy;
    PhyParam_t phyParam;

    getPhy.Attribute = PHY_MIN_TX_DR;
    phyParam = RegionGetPhyParam( ACTIVE_REGION, &getPhy );
    min_tx_dr = phyParam.Value;

    getPhy.Attribute = PHY_MAX_TX_DR;
    phyParam = RegionGetPhyParam( ACTIVE_REGION, &getPhy );
    max_tx_dr = phyParam.Value;

    if (datarate >= min_tx_dr &&
        datarate <= max_tx_dr && lorawan_module_get_adr() == 0) {
        g_default_datarate = datarate;
        return 0;
    } else {
        return -1;
    }
}

int8_t lorawan_module_get_tx_datarate(void)
{
    return g_default_datarate;
}

int lorawan_module_set_repeater_filter(RepeaterFilterActType_t act_type, RepeaterFilterType_t filt_type,
                                       uint8_t *deveui_oui, uint32_t netid)
{
    return -1;
}

int lorawan_module_get_repeater_filter(uint8_t *nb_iterm, RepeaterFilterType_t *filt_type)
{
    return -1;
}

int lorawan_module_set_repeater_freq(uint32_t frequency)
{
    LoRaMacStatus_t     status;
    MibRequestConfirm_t mib_req;
    int                ret = -1;

    mib_req.Type = MIB_REPEATER_CHANNEL_PARAM;
    mib_req.Param.RepeaterChannelParam.Frequency = frequency;
    mib_req.Param.RepeaterChannelParam.Datarate  = LORA_NODE_REPEATER_CHANNEL_DR;

    status       = LoRaMacMibSetRequestConfirm(&mib_req);
    if (status == LORAMAC_STATUS_OK) {
        ret = 0;
    }
    return ret;
}

int lorawan_module_get_repeater_freq(uint32_t *frequency)
{
    LoRaMacStatus_t     status;
    MibRequestConfirm_t mib_req;
    int                ret = -1;

    mib_req.Type = MIB_REPEATER_CHANNEL_PARAM;
    status       = LoRaMacMibGetRequestConfirm(&mib_req);
    if (status == LORAMAC_STATUS_OK) {
        *frequency = mib_req.Param.RepeaterChannelParam.Frequency;
        ret = 0;
    }
    return ret;
}

int lorawan_module_set_adr(int state)
{
    LoRaMacStatus_t     status;
    MibRequestConfirm_t mib_req;
    int                ret = -1;

    if (state == 0) {
        mib_req.Param.AdrEnable = false;
    } else {
        mib_req.Param.AdrEnable = true;
    }
    mib_req.Type = MIB_ADR;
    status       = LoRaMacMibSetRequestConfirm(&mib_req);
    if (status == LORAMAC_STATUS_OK) {
        ret = 0;
    }
    return ret;
}

int lorawan_module_get_adr(void)
{
    MibRequestConfirm_t mib_req;

    mib_req.Type = MIB_ADR;
    LoRaMacMibGetRequestConfirm(&mib_req);
    if (mib_req.Param.AdrEnable == true) {
        return 1;
    }
    return 0;
}

int lorawan_module_send_link_check(int check_value)
{
    int ret = 0;

    MlmeReq_t mlmeReq;

    switch ( check_value ) {
        case 0: {
            g_auto_linkcheck = false;
            ret = 0;
            break;
        }
        case 1 : {
            mlmeReq.Type = MLME_LINK_CHECK;
            if (LoRaMacMlmeRequest(&mlmeReq) == LORAMAC_STATUS_OK) {
                lorawan_control_trigger_send();
                ret = 0;
            } else {
                ret = -1;
            }
            break;
        }
        case 2: {
            g_auto_linkcheck = true;
            ret = 0;
            break;
        }
        default: {
            ret = -1;
            break;
        }
    }

    return ret;

}

JoinMode_t lorawan_module_get_join_mode(void)
{
    return lorawan_control_get_join_mode();
}

int lorawan_module_set_join_mode(JoinMode_t mode)
{
    if (!is_lora_module_init()) {
        return -1;
    }

    return lorawan_control_set_join_mode(mode);
}

int lorawan_module_get_num_tx_rx(uint16_t *tx_num, uint16_t *rx_num)
{
    *tx_num = g_lora_num_send_uplink;
    *rx_num = g_lora_num_received_data_ack;

    return 0;
}

LoRaModuleStatus_t lorawan_module_get_status(void)
{
    return g_lora_control_current_status;
}

int lorawan_module_get_rssi_snr(char *rssi, char *snr)
{
    if ((g_lora_rx_rssi == 0) && (g_lora_rx_snr == 0)) {
        return -1;
    }
    *rssi = g_lora_rx_rssi;
    *snr =  g_lora_rx_snr;

    return 0;
}

int lorawan_module_join(int mode)
{
    if (mode == 1 || mode == 0) {
        return lorawan_control_join(mode);
    } else {
        return -1;
    }
}

NodeWorkMode_t lorawan_module_get_work_mode(void)
{
    return lorawan_control_get_work_mode();
}

int lorawan_module_set_work_mode(NodeWorkMode_t mode)
{
    if (!is_lora_module_init()) {
        return -1;
    }

    return lorawan_control_set_work_mode(mode);
}

LoRaModuleStatus_t lorawan_module_switch_class(DeviceClass_t newClass)
{
    return lorawan_control_switch_class(newClass);
}

int lorawan_module_send(int is_confirm, int app_port, const char *data, int len, handle_send_cb_fp_t cb)
{
    return lorawan_control_send(is_confirm, app_port, data, len, cb);
}

int lorawan_module_get_rx_data(uint8_t **buffer, int *len)
{
    *buffer = g_lora_app_rx_data_buffer;
    *len = g_lora_app_rx_data_size;
    g_lora_app_rx_data_size = 0;

    return 0;
}

int lorawan_module_save(void)
{
#if LORA_MODULE_GET_FLASH_EUI_ENABLE
    int ret;

    ret = lora_module_dev_stat_set_devEUI(DevEui, 8);
    if (ret != 0) {
        return -1;
    }

    ret = lora_module_dev_stat_set_appEUI(JoinEui, 8);
    if (ret != 0) {
        return -1;
    }

    ret = lora_module_dev_stat_set_appKey(AppKey, 16);
    if (ret != 0) {
        return -1;
    }

    ret = lora_module_dev_stat_set_freqmode(lorawan_module_get_freq_mode());
    if (ret != 0) {
        return -1;
    }

    ret = lora_module_dev_stat_set_freqband_mask(lorawan_module_get_freqband_mask());
    if (ret != 0) {
        return -1;
    }

    ret = lora_module_dev_stat_set_class(g_current_target_lora_class_type);
    if (ret != 0) {
        return -1;
    }

    return 0;
#else
    return 0;
#endif
}

int lorawan_module_restore(void)
{
#if LORA_MODULE_GET_FLASH_EUI_ENABLE
    int ret;

    ret = lora_module_dev_stat_set_lora_dev();
    return ret;
#else
    return 0;
#endif
}

int lorawan_module_set_lbt(int mode)
{
    LoRaMacStatus_t     status;
    MibRequestConfirm_t mib_req;
    int                ret = -1;

    if (!is_lora_module_init()) {
        return -1;
    }

    if (mode == 0) {
        mib_req.Param.LbtEnable = false;
    } else {
        mib_req.Param.LbtEnable = true;
    }

    mib_req.Type = MIB_LBT;
    status       = LoRaMacMibSetRequestConfirm(&mib_req);
    if (status == LORAMAC_STATUS_OK) {
        ret = 0;
    }

    return ret;
}

int lorawan_module_get_lbt(void)
{
    MibRequestConfirm_t mib_req;

    mib_req.Type = MIB_LBT;
    LoRaMacMibGetRequestConfirm(&mib_req);
    if (mib_req.Param.LbtEnable == true) {
        return 1;
    }

    return 0;
}

int lorawan_module_set_proprietary(int mode)
{
    if (mode == 0) {
        g_lora_proprietary_enable = false;
    } else {
        g_lora_proprietary_enable = true;
    }

    return 0;
}

int lorawan_module_get_proprietary(void)
{
    if (g_lora_proprietary_enable) {
        return 1;
    } else {
        return 0;
    }
}

int lorawan_module_set_auto_join(int8_t mode)
{
    g_lora_auto_join = mode;
    return 0;
}

int lorawan_module_get_auto_join(void)
{
    return g_lora_auto_join;
}

int lorawan_module_set_pingslot_period(uint8_t period)
{
    if (period > 7) {
        return -1;
    }

    g_lora_pingslot_period = period;
    return 0;
}

int lorawan_module_get_pingslot_period(void)
{
    return g_lora_pingslot_period;
}

int lorawan_module_set_monitor_node(bool open)
{
    g_monitor_node_feature_open = open;

    if (open) {
        //close ADR
        if (0 != lorawan_module_set_adr(0)) {
            return -1;
        }
        if (0 != lorawan_module_set_tx_datarate(DR_5)) {
            return -1;
        }
        //set confrim
        if (0 != lorawan_module_set_tx_cfm_flag(1)) {
            return -1;
        }
        if (0 != lorawan_module_set_tx_cfm_trials(1, 1)) {
            return -1;
        }
        //set report interval and start report
        if (0 != lorawan_module_set_duty_cycle(5 * 60 * 1000)) {
            return -1;
        }
        if (0 != lorawan_module_set_report_mode(1)) {
            return -1;
        }
        //set to classA
        if (0 != lorawan_module_set_class(CLASS_A)) {
            return -1;
        }
        //monitor not support repeater mode
        if (0 != lorawan_control_set_work_mode(NODE_WORK_MODE_NORMAL)) {
            return -1;
        }
        //start join
        if (0 != lorawan_module_join(1)) {
            return -1;
        }
    } else {
        if (0 != lorawan_module_set_adr(1)) {
            return -1;
        }
    }

    return 0;
}

int lorawan_module_get_monitor_node(void)
{
    return g_monitor_node_feature_open;
}
