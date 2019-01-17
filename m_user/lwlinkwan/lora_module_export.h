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
#ifndef LORA_MODULE_EXPORT_H
#define LORA_MODULE_EXPORT_H

#include "connection.h"
#include "LoRaMac.h"
#include "lora_module_common.h"
#include "lora_config.h"
#include "LoRaMacMulticast.h"

/*!
 * LoRa_Semaph_Release_Source type
 */
typedef enum {
    LoRa_Task = 0,
    RTC_IRQ,
    Radio_IRQ
} LoRaSemaphRelSource;

typedef enum eRepeaterFilterType {
    REPEATER_FILTER_TPYE_DEVEUI_DEVADDR = 0,
    REPEATER_FILTER_TPYE_DEVEOUI_NETID = 1
} RepeaterFilterType_t;

typedef enum eRepeaterFilterActType {
    REPEATER_FILTER_ADD_ONE_ITERM = 0,
    REPEATER_FILTER_DEL_ONE_ITERM = 1,
    REPEATER_FILTER_ADD_ALL_ITERM = 2,//add all iterm once
    REPEATER_FILTER_DEL_ALL_ITERM = 3 //delete all iterm once
} RepeaterFilterActType_t;

/*  function prototypes -----------------------------------------------*/

/*!
 * \brief   lorawan module initialization
 *
 * \details In addition to the initialization of the lorawan module, this
 *          function initializes the callback of APP services. Must be
 *          set to a valid callback function.
 * \param   [IN] .
 *
 * \retval  Possible returns are:
 *          -1:init failed.
 *           0:init succesful.
 */
extern int  lorawan_module_init(esl_connection_ops_cb_t *_esl_ops_cb, void *data);

/*!
 * \brief   lorawan module deinitialization
 *
 * \param   [IN] .
 *
 * \retval  Possible returns are:
 *          -1:deinit failed.
 *           0:deinit succesful.
 */
extern int  lorawan_module_deinit(void);

/*!
 * \brief   lorawan module send data, must loar module status be connected,see on_connect() CB funtion.
 *
 * \param   [IN] .
 *
 * \retval  Possible returns are:
 *          -1:send failed.
 *           0:send succesful.
 */
extern int  lorawan_module_send(int is_confirm, int app_port, const char *data, int len, handle_send_cb_fp_t cb);

/*!
 * \brief   get rx data.
 *
 * \param   [OUT] buffer
 * \param   [OUT] len
 *
 * \retval  Possible returns are:
 *           0:get succesful.
 */
extern int lorawan_module_get_rx_data(uint8_t **buffer, int *len);

/*!
 * \brief   get the latest downlink rssi and snr from lorawan module.
 *
 * \param   [IN] .
 *
 * \retval  Possible returns are:
 *          -1:get failed.
 *           0:get succesful.
 */
extern int  lorawan_module_get_rssi_snr(char *rssi, char *snr);

/*!
 * \brief   get the latest number of Uplink and Downlink package from lorawan module.
 *
 * \param   [IN] .
 *
 * \retval  Possible returns are:
 *          -1:get failed.
 *           0:get succesful.
 */
extern int lorawan_module_get_num_tx_rx(uint16_t *tx_num, uint16_t *rx_num);

/*!
 * \brief   trigger lorawan module to join or stop join.
 *          Note:
 *
 * \param   [IN] mode 0: stop join, 1: start join.
 *
 * \retval  Possible returns are:
 *          -1:trigger failed.
 *           0:trigger succesful, join result will be know later.
 */
extern int  lorawan_module_join(int mode);

/*!
 * \brief   trigger lorawan module to switch class type.
 *          Note:Should be Join successful and module status is LoRa_Join_Success or LoRa_Idle before starting to switch class
 *
 * \param   [IN] .
 *
 * \retval  Possible returns are:
 *          success:
 *          LoRa_Class_Switching,//valid only for switching to classB
 *          LoRa_Class_SwitchOK, //valid only for switching to classC/A
 *          failed:
 *          other status
 */
extern LoRaModuleStatus_t lorawan_module_switch_class(DeviceClass_t newClass);

/*!
 * \brief   get current lorawan module status.
 *          Note:
 *
 * \param   [IN] .
 *
 * \retval  Possible returns are:
 *          see LoRaModuleStatus_t.
 */
extern LoRaModuleStatus_t lorawan_module_get_status(void);

/*!
 * \brief   get the join result from lorawan module.
 *
 * \param   [IN] .
 *
 * \retval  Possible returns are:
 *           0:join not ok.
 *           1:join successful.
 */
extern int lorawan_module_get_join_status(void);

/*!
 * \brief    lorawan module main process, get CPU to process lora event;
 *
 * \param   [IN] .
 *
 * \retval
 */
extern void lorawan_module_main(void const *arg);

/*!
 * \brief    [RTOS] other task activates lorawan module task
 *
 * \param   [IN] activate: 0: de-activate, non-0：activate
 *
 * \retval  Possible returns are:
 *          -1: fail.
 *           0: succesful.
 */
extern int lorawan_module_activate(int activate);


/*!
 * \brief    [RTOS] Radio/RTC interrupt trigger lorawan module task to work
 *
 * \param   [IN] interrupt Source
 *
 * \retval  Possible returns are:
 *          OS_OK.
 *          not OS_OK.
 */
extern int lora_interrupt_semaphore_release(LoRaSemaphRelSource releaseSource);

/*!
 * \brief   get work mode
 * \retval  Possible returns are:
 *          NODE_WORK_MODE_NORMAL
 *          NODE_WORK_MODE_REPEATER
 *          NODE_WORK_MODE_AUTOSWITCH
 */
extern NodeWorkMode_t lorawan_module_get_work_mode(void);

/*!
 * \brief   set work mode
 *
 * \param   [IN] work mode
 *
 * \details Check if NODE_WORK_MODE_NORMAL or NODE_WORK_MODE_REPEATER or NODE_WORK_MODE_AUTOSWITCH
 *
 * \retval  Possible returns are:
 *          -1: fail.
 *           0: succesful.
 */
extern int lorawan_module_set_work_mode(NodeWorkMode_t mode);

/*!
 * \brief   get join mode
 *
 * \details Check if OTAA or ABP
 *
 * \retval  Possible returns are:
 *          JOIN_MODE_OTAA
 *          JOIN_MODE_ABP
 */
extern JoinMode_t lorawan_module_get_join_mode(void);

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
extern int lorawan_module_set_join_mode(JoinMode_t mode);

/*!
 * \brief   get lbt mode
 *
 * \details Check if lbt enable or not
 *
 * \retval  Possible returns are:
 *          0: disable
 *          1: enable
 */
extern int lorawan_module_get_lbt(void);

/*!
 * \brief   set lbt mode
 *
 * \param   [IN] lbt mode 0: disable, 1: enable
 *
 * \details enable or disable lbt
 *
 * \retval  Possible returns are:
 *           0: succesful.
 */
extern int lorawan_module_set_lbt(int mode);

/*!
 * \brief   get proprietary mode
 *
 * \details Check if proprietary enable or not
 *
 * \retval  Possible returns are:
 *          0: disable
 *          1: enable
 */
extern int lorawan_module_get_proprietary(void);

/*!
 * \brief   set proprietary mode
 *
 * \param   [IN] proprietary mode 0: disable, 1: enable
 *
 * \details enable or disable proprietary
 *
 * \retval  Possible returns are:
 *           0: succesful.
 */
extern int lorawan_module_set_proprietary(int mode);

/*!
 * \brief   set freqband mask
 *
 * \param   [IN] mask: mask to be set
 *
 * \details freqband mask should be in F00F
 *
 * \retval  Possible returns are:
 *          -1: fail.
 *           0: succesful.
 */
extern int lorawan_module_set_freqband_mask(uint16_t mask);

/*!
 * \brief   get freqband mask
 *
 * \retval  freqband mask
 */
extern uint16_t lorawan_module_get_freqband_mask(void);

/*!
 * \brief   get deveui
 *
 * \param   [OUT] eui
 *
 */
extern void lorawan_module_get_dev_eui(uint8_t *eui);

/*!
 * \brief   get appeui (joineui)
 *
 * \param   [OUT] eui
 *
 */
extern void lorawan_module_get_app_eui(uint8_t *eui);

/*!
 * \brief   get appkey
 *
 * \param   [OUT] appkey
 *
 */
extern void lorawan_module_get_app_key(uint8_t *key);

/*!
 * \brief   set deveui
 *
 * \param   [in] deveui
 *
 * \retval  Possible returns are:
 *           0: succesful.
 */
extern int lorawan_module_set_dev_eui(uint8_t *eui);

/*!
 * \brief   set appeui
 *
 * \param   [in] appeui
 *
 * \retval  Possible returns are:
 *           0: succesful.
 */
extern int lorawan_module_set_app_eui(uint8_t *eui);

/*!
 * \brief   set appkey
 *
 * \param   [in] appkey
 *
 * \retval  Possible returns are:
 *           0: succesful.
 */
extern int lorawan_module_set_app_key(uint8_t *key);

/*!
 * \brief   set devaddr
 *
 * \param   [in] devaddr
 *
 * \retval  Possible returns are:
 *           0: succesful.
 */
int lorawan_module_set_devaddr(uint8_t *devaddr);

/*!
 * \brief   get devaddr
 *
 * \retval  devaddr pointer
 */
uint8_t *lorawan_module_get_devaddr(void);

/*!
 * \brief   set appskey
 *
 * \param   [in] appskey
 *
 * \retval  Possible returns are:
 *           0: succesful.
 */
int lorawan_module_set_appskey(uint8_t *buf);

/*!
 * \brief   get appskey
 *
 * \retval  appskey pointer
 */
uint8_t *lorawan_module_get_appskey(void);

/*!
 * \brief   set nwkskey
 *
 * \param   [in] nwkskey
 *
 * \retval  Possible returns are:
 *           0: succesful.
 */
int lorawan_module_set_nwkskey(uint8_t *buf);

/*!
 * \brief   get nwkskey
 *
 * \retval  nwkskey pointer
 */
uint8_t *lorawan_module_get_nwkskey(void);

/*!
 * \brief   get freq mode
 *
 * \retval  Possible returns are:
 *           FREQ_MODE_INTRA
 *           FREQ_MODE_INTER
 */
extern LoRaNodeFreqMode_t lorawan_module_get_freq_mode(void);

/*!
 * \brief   set freq mode
 *
 * \param   [in] mode
 *
 * \retval  Possible returns are:
 *          -1: fail.
 *           0: succesful.
 */
extern int lorawan_module_set_freq_mode(LoRaNodeFreqMode_t mode);

/*!
 * \brief   add or delete repeater filter's iterm of white list
 *
 * \param   [in] act_type: delete or add ,ref RepeaterFilterActType_t;
 * \param   [in] filt_type: filt by devEUI/devAddr or OUI/NetID ,ref RepeaterFilterType_t;
 * \param   [in] deveui_oui: pointer to devEUI for filter type of devEUI/devAddr,  or pointer to OUI for filter type of OUI/NetID;
 * \param   [in] netid: NetID, only valid for filter type of OUI/NetID;
 *
 * \retval  Possible returns are:
 *          -1: fail.
 *           0: succesful.
 */
extern int lorawan_module_set_repeater_filter(RepeaterFilterActType_t act_type, RepeaterFilterType_t filt_type,
                                              uint8_t *deveui_oui, uint32_t netid);
/*!
 * \brief   get the number of repeater filter's iterm of white list and filter type;
 *
 * \param   [in] nb_iterm:pointer to number of added iterm of white list;
 * \param   [in] filt_type:pointer to filter type;
 *
 * \retval  Possible returns are:
 *          -1: fail.
 *           0: succesful.
 */
extern int lorawan_module_get_repeater_filter(uint8_t *nb_iterm, RepeaterFilterType_t *filt_type);

/*!
 * \brief   set repeat mode's freq
 *
 * \param   [in] frequency in Hz
 *
 * \retval  Possible returns are:
 *          -1: fail.
 *           0: succesful.
 */
extern int lorawan_module_set_repeater_freq(uint32_t frequency);
/*!
 * \brief   get repeat mode's freq
 *
 * \param   [in] pointer to frequency in Hz
 *
 * \retval  Possible returns are:
 *          -1: fail.
 *           0: succesful.
 */
extern int lorawan_module_get_repeater_freq(uint32_t *frequency);

/*!
 * \brief   set ADR
 *
 * \param   [in] state
 *
 * \retval  Possible returns are:
 *          -1: fail.
 *           0: succesful.
 */
extern int lorawan_module_set_adr(int state);

/*!
 * \brief   get ADR
 *
 * \retval  Possible returns are:
 *          1: ADR enble
 *          0: ADR disable
 */
extern int lorawan_module_get_adr(void);

/*!
 * \brief   set class
 *
 * \param   [in] class: maybe CLASS_A / CLASS_B / CLASS_C
 *
 * \retval  Possible returns are:
 *          -1: fail.
 *           0: succesful.
 */
extern int lorawan_module_set_class(int8_t class);

/*!
 * \brief   get class
 *
 * \retval  Possible returns are:
 *          CLASS_A
 *          CLASS_B
 *          CLASS_C
 */
extern int8_t lorawan_module_get_class(void);

/*!
 * \brief   set confirm flag
 *
 * \param   [in] confirmed
 *
 * \retval  Possible returns are:
 *           0: succesful.
 */
extern int lorawan_module_set_tx_cfm_flag(int confirmed);

/*!
 * \brief   get confirm flag
 *
 * \retval  Possible returns are:
 *           0: unconfirmed
 *           1: confirmed
 */
extern int lorawan_module_get_tx_cfm_flag(void);

/*!
 * \brief   set tx trails time
 *
 * \param   [in] type 0: unconfirm 1:confirm
 * \param   [in] trials
 *
 * \retval  Possible returns are:
 *           0: succesful.
 */
extern int lorawan_module_set_tx_cfm_trials(uint8_t type, uint8_t trials);

/*!
 * \brief   get tx trails time
 *
 * \param   [in] type 0: unconfirm 1:confirm
 *
 * \retval  trails time
 */
extern uint8_t lorawan_module_get_tx_cfm_trials(uint8_t type);

/*!
 * \brief   get rssi
 *
 * \param   [in] channel_rssi: 8 channel to keep rssi
 * \param   [in] channel_snr: 8 channel to keep snr
 *
 */
extern void lorawan_module_get_channel_rssi_snr(int16_t *channel_rssi, int8_t *channel_snr);

/*!
 * \brief   get report mode
 *
 * \retval  Possible returns are:
 *           0: not cycle mode
 *           1: cycle mode
 */
extern int lorawan_module_get_report_mode(void);

/*!
 * \brief   set report mode
 *
 * \param   [in] mode
 *
 * \retval  Possible returns are:
 *          -1: fail.
 *           0: succesful.
 */
extern int lorawan_module_set_report_mode(int8_t mode);

/*!
 * \brief   get app port
 *
 * \retval  app port
 */
extern uint8_t lorawan_module_get_app_port(void);

/*!
 * \brief   set app port
 *
 * \param   [in] port
 *
 * \retval  Possible returns are:
 *          -1: fail.
 *           0: succesful.
 */
extern int lorawan_module_set_app_port(uint8_t port);

/*!
 * \brief   reboot board
 */
extern void lorawan_module_reboot(int8_t mode);


/*!
 * \brief   set duty cycle
 *
 * \param   [in] duty cycle
 *
 * \retval  Possible returns are:
 *           0: succesful.
 */
extern int lorawan_module_set_duty_cycle(uint32_t duty_cycle);

/*!
 * \brief   get duty cycle
 *
 * \retval  duty cycle
 */
extern uint32_t lorawan_module_get_duty_cycle(void);

/*!
 * \brief   return multicast info
 *
 * \retval  pointer to mcChanel
 */
extern mcChannel_t *lorawan_module_get_cur_multicast(void);

/*!
 * \brief   return mc key info
 *
 * \retval  pointer to mcKey
 */
extern mcKey_t *lorawan_module_get_cur_mc_key(void);

/*!
 * \brief   set one multicast
 *
 * \param   [in] dev_addr
 * \param   [in] mc_key
 * \param   [in] frequency
 * \param   [in] datarate
 * \param   [in] periodicity
 *
 * \retval  Possible returns are:
 *           0: succesful.
 *          -1: fail.
 */
extern int lorawan_module_set_multicast(uint32_t dev_addr, uint8_t *mc_key, uint32_t freq, int8_t dr, uint16_t period);

/*!
 * \brief   del one multicast
 *
 * \param   [in] devaddr
 *
 * \retval  Possible returns are:
 *           0: succesful.
 *          -1: fail.
 */
extern int lorawan_module_del_multicast(uint32_t dev_addr);

/*!
 * \brief   return mc num
 *
 * \retval  mc num
 */
extern uint8_t lorawan_module_get_mulitcast_num(void);

/*!
 * \brief   set tx datarate
 *
 * \param   [in] datarate
 *
 * \retval  Possible returns are:
 *           0: succesful.
 *          -1: fail.
 */
extern int lorawan_module_set_tx_datarate(int8_t datarate);

/*!
 * \brief   get tx datarate
 *
 * \retval  datarate
 */
extern int8_t lorawan_module_get_tx_datarate(void);

/*!
 * \brief   get tx power
 *
 * \retval  tx power
 */
extern int8_t lorawan_module_get_tx_power(void);

/*!
 * \brief   set tx power
 *
 * \param   [in] pwr
 *
 * \retval  Possible returns are:
 *           0: succesful.
 *          -1: fail.
 */
extern int lorawan_module_set_tx_power(int8_t pwr);

/*!
 * \brief   send link check
 *
 * \param   [in] check_value
 *
 * \retval  Possible returns are:
 *           0: succesful.
 *          -1: fail.
 */
extern int lorawan_module_send_link_check(int check_value);

/*!
 * \brief   get rx1 delay
 *
 * \retval  rx1 delay (s)
 */
extern uint32_t lorawan_module_get_mac_rx1_delay(void);

/*!
 * \brief   set rx1 delay
 *
 * \param   [in] rx1delay
 *
 * \retval  Possible returns are:
 *           0: succesful.
 *          -1: fail.
 */
extern int lorawan_module_set_mac_rx1_delay(long rx1delay);

/*!
 * \brief   get rx window parameters
 *
 * \param   [in] RX1DRoffset
 * \param   [in] RX2DataRate
 * \param   [in] RX2Frequency
 *
 * \retval  Possible returns are:
 *          -1: fail.
 *           0: succesful.
 */
extern int lorawan_module_get_rx_window_params(uint8_t *RX1DRoffset, uint8_t *RX2DataRate,
                                               uint32_t *RX2Frequency);

/*!
 * \brief   set rx window parameters
 *
 * \param   [in] RX1DRoffset
 * \param   [in] RX2DR
 *
 * \retval  Possible returns are:
 *          -1: fail.
 *           0: succesful.
 */
extern int lorawan_module_set_rx_window_params(uint8_t RX1DRoffset, uint8_t RX2DR);

/*!
 * \brief  save mac parameters
 *
 * \retval  Possible returns are:
 *          -1: fail.
 *           0: succesful.
 */
extern int lorawan_module_save(void);

/*!
 * \brief  restore mac parameters
 *
 * \retval  Possible returns are:
 *          -1: fail.
 *           0: succesful.
 */
extern int lorawan_module_restore(void);

/*!
 * \brief   get auto_join mode
 *
 * \retval  Possible returns are:
 *           0: not auto join
 *           1: auto join
 */
extern int lorawan_module_get_auto_join(void);

/*!
 * \brief   set auto_join mode
 *
 * \param   [in] mode 0: not auto join 1: auto join
 *
 * \retval  Possible returns are:
 *           0: succesful.
 */
extern int lorawan_module_set_auto_join(int8_t mode);

/*!
 * \brief   get pingslot period
 *
 * \retval  pingslot period
 */
extern int lorawan_module_get_pingslot_period(void);

/*!
 * \brief   set pingslot period
 *
 * \param   [in] mode 0: not auto join 1: auto join
 *
 * \retval  Possible returns are:
 *           0: succesful.
 */
extern int lorawan_module_set_pingslot_period(uint8_t period);

/*!
 * \brief   open or close the function of monitor node
 *
 * \param   [in] open
 *
 * \retval  Possible returns are:
 *          -1: fail.
 *           0: succesful.
 */
extern int lorawan_module_set_monitor_node(bool open);
/*!
 * \brief   get if the function of monitor node is opened.
 *
 * \param   [in]
 *
 * \retval  Possible returns are:
 *          0:  the function of monitor node is closed.
 *          1:  the function of monitor node is opened.
 */
extern int lorawan_module_get_monitor_node(void);

#endif /* LORA_MODULE_EXPORT_H */
