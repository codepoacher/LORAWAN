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

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include <math.h>

//#include "debug.h"
//#include "hw.h"
//#include "low_power_manager.h"
//#include "bsp.h"
#include "time.h"
//#include "vcom.h"

#include "version.h"
#include "board.h"
#include "utilities.h"
#include "Commissioning.h"

#include "LoRaMac.h"
#include "LoRaMacFCntHandler.h"

#include "lora_module_controller.h"
#include "lora_config.h"
#include "connection.h"
#include "lora_multicast_control.h"
#include "lora_module_certification.h"
#include "lora_module_device_stat_flash_manager.h"
#include "lora_module_ica_at.h"
//#include "gpio.h"
//#include "cmsis_os.h"
//#include "lite-log.h"
//#include "battery.h"

#define NUM_MS_PER_SECOND       1000
#define NUM_SECOND_PER_MINUTE   60
#define NUM_MINUTE_PER_HOUR     60

#define TIMER_SWITCH_CLASSB_RESULT_CHECK (150000) //150s
#define TIMER_RADIO_STATUS_CHECK         (180000) //180s
#define LORAWAN_FCNT_MAX                 (65535)
#define LORAWAN_NODE_REPEATER_MODE_REPORT_DEVADDR_FLAG (0x1A2B3C4D)

static int g_num_send_data_failed_to_rejoin = 0;
static int g_num_of_join_failed             = 0;
static int g_num_of_store_join_failed       = 0;
static int g_num_of_def_join_failed         = 0;
static int g_num_of_scan_round_join_failed  = 0;
static int g_num_of_scan_join_failed        = 0;
uint16_t g_lora_num_send_uplink             = 0;
uint16_t g_lora_num_received_data_ack       = 0;
static int g_num_of_class_switch_failed     = 0;
static bool g_lora_class_switch_succes_flag  = true;
static unsigned long g_random_joindelay     = 0;

static int g_num_of_pingslotack_failed      = 0;
static int g_num_beacon_missed              = 0;

DeviceClass_t g_current_target_lora_class_type = LORA_NODE_TARGET_CLASS_TYPE;
DeviceClass_t g_lora_current_class        = CLASS_A;

static esl_connection_ops_cb_t *g_lora_esl_connection_ops_cb = NULL;
static void                    *g_lora_cm_data               = NULL;

LoRaModuleStatus_t g_lora_control_current_status = LoRa_Initing;

int g_lora_rx_rssi = 0;
int g_lora_rx_snr  = 0;

typedef struct {
    uint8_t port;
    rx_cb_t rx_cb;
} app_rx_cb_t;

typedef enum join_method_s {
    STORED_JOIN_METHOD = 0,
    DEF_JOIN_METHOD    = 1,
    SCAN_JOIN_METHOD   = 2,
    JOIN_METHOD_NUM
} join_method_t;

typedef enum MonitorNodeDlCmdStatus_s {
    MonitorNode_NoDlCmd          = 0,
    MonitorNode_DlCmdExecuteOK   = 1,
    MonitorNode_DlCmdExecuteFail = 2
} MonitorNodeDlCmdStatus_t;

#define MAX_APP_RX_CB_LIST (5)
static app_rx_cb_t g_app_rx_cb_list[MAX_APP_RX_CB_LIST];

#if LORA_MODULE_GET_FLASH_EUI_ENABLE
uint8_t DevEui[8] = {0};//LORAWAN_DEVICE_EUI;
uint8_t JoinEui[8] = {0};//LORAWAN_JOIN_EUI;
uint8_t AppKey[16] = {0};//LORAWAN_APP_KEY;
uint8_t NwkKey[16] = {0};//LORAWAN_NWK_KEY;
static int8_t g_store_freqband = -1;
#else
uint8_t DevEui[] = LORAWAN_DEVICE_EUI;
uint8_t JoinEui[] = LORAWAN_JOIN_EUI;
uint8_t AppKey[] = LORAWAN_APP_KEY;
uint8_t NwkKey[] = LORAWAN_NWK_KEY;
static int8_t g_store_freqband = LORA_STORE_FREQBAND;
#endif

/* preset multicast parameter */
mcChannel_t g_mc_channels[LORAMAC_MAX_MC_CTX] = {
    { .AddrID = MULTICAST_0_ADDR, .Address = LORAWAN_MC_ADDR_0, .IsEnabled = 1, .Frequency = 0, .Datarate = LORAWAN_MC_DR_0, .Periodicity = LORAWAN_MC_PERIOD_0 },
    { .AddrID = MULTICAST_1_ADDR, .Address = LORAWAN_MC_ADDR_1, .IsEnabled = 1, .Frequency = 0, .Datarate = LORAWAN_MC_DR_1, .Periodicity = LORAWAN_MC_PERIOD_1 },
    { .AddrID = MULTICAST_2_ADDR, .Address = LORAWAN_MC_ADDR_2, .IsEnabled = 1, .Frequency = 0, .Datarate = LORAWAN_MC_DR_2, .Periodicity = LORAWAN_MC_PERIOD_2 },
    { .AddrID = MULTICAST_3_ADDR, .Address = LORAWAN_MC_ADDR_3, .IsEnabled = 1, .Frequency = 0, .Datarate = LORAWAN_MC_DR_3, .Periodicity = LORAWAN_MC_PERIOD_3 }
};

mcKey_t g_mc_keys[LORAMAC_MAX_MC_CTX] = {
    { MIB_MC_KEY_0, LORAWAN_MC_KEY_0 },
    { MIB_MC_KEY_1, LORAWAN_MC_KEY_1 },
    { MIB_MC_KEY_2, LORAWAN_MC_KEY_2 },
    { MIB_MC_KEY_3, LORAWAN_MC_KEY_3 }
};

static uint8_t mcGenAppKey[] = LORAWAN_MC_GEN_APP_KEY;
uint8_t FNwkSIntKey[] = LORAWAN_F_NWK_S_INT_KEY;
uint8_t SNwkSIntKey[] = LORAWAN_S_NWK_S_INT_KEY;
uint8_t NwkSEncKey[] = LORAWAN_NWK_S_ENC_KEY;
uint8_t AppSKey[] = LORAWAN_APP_S_KEY;

/*!
 * Device address
 */
uint32_t DevAddr = LORAWAN_DEVICE_ADDRESS;

#define LORAWAN_CONTROLLER_PORT                             (2)
#define LORAWAN_CONTROLLER_DATA_LEN                         (0)

/*!
 * Application port
 */
uint8_t g_lora_app_port = LORAWAN_APP_PORT;

/*!
 * port in use
 */
uint8_t g_lora_port = LORAWAN_CONTROLLER_PORT;

/*!
 * User application data size
 */
uint8_t g_lora_app_data_size = 1;
uint8_t g_lora_app_data_size_backup = 1;
uint8_t g_lora_app_rx_data_size = 0;

/*!
 * User application data
 */
uint8_t g_lora_app_data_buffer[LORAWAN_APP_DATA_MAX_SIZE];

/*!
 * User application rx data
 */
uint8_t g_lora_app_rx_data_buffer[LORAWAN_APP_DATA_MAX_SIZE];

/*!
 * Indicates if the node is sending confirmed or unconfirmed messages
 */
uint8_t g_lora_is_confirmup = LORAWAN_CONFIRMED_MSG_ON;

/*!
 * Defines Tx confirm retry count
 */
uint8_t g_lora_nb_confirmup_trails = LORA_NODE_CONFIRM_RETX_NUM;

/*!
 * Defines auto join or not
 */
bool g_lora_auto_join = true;

/*!
 * Defines the default application data transmission duty cycle (ms)
 */
uint32_t g_lora_app_tx_dutycycle = APP_TX_DUTYCYCLE;

/*!
 * Defines the tx cycle mode
 */
uint8_t g_lora_app_tx_report_mode = LORA_MODULE_SEND_HEARTBEAT;

/*!
 * Defines the join method
 */
static join_method_t g_join_method = STORED_JOIN_METHOD;

/*!
 * Defines join scan left band mask
 */
static uint16_t g_join_scan_left_band_mask;

/*!
 * Defines join freqband
 */
static uint8_t g_join_freqband = LORA_STORE_FREQBAND;

/*!
 * Defines default datarate
 */
uint8_t g_default_datarate = LORAWAN_DEFAULT_DATARATE;

/*!
 * Defines join datarate
 */
static uint8_t g_join_datarate = LORAWAN_DEFAULT_DATARATE;

/*!
 * Defines freqband mask
 */
uint16_t g_lora_freqband_mask = LORA_JOIN_FREQBAND_MASK;

/*!
 * Defines freqband mask
 */
uint8_t g_lora_pingslot_period = LORAWAN_DEFAULT_PING_SLOT_PERIODICITY;

/*!
 * Defines the application data transmission duty cycle
 */
static uint32_t TxDutyCycleTime;

/*!
 * Timer to handle the application data transmission duty cycle
 */
static TimerEvent_t TxNextPacketTimer;

/*!
 * Specifies the state of the application LED
 */
static bool AppLedStateOn = false;

/*!
 * Specifies the otaa join mode
 */
static bool g_lora_otaa_mode = OVER_THE_AIR_ACTIVATION;

/*!
 * Indicates if a new packet can be sent
 */
static bool NextTx = true;

static DeviceState_t DeviceState, WakeUpState;

/*!
 * Defines the default work mode
 */
NodeWorkMode_t g_lora_current_work_mode = LORA_NODE_WORKMODE;
bool           g_lora_is_work_mode_autoswitch = false;
//ONLY for Repeater mode
static uint8_t g_cntSendedDevAddrToRepeater = 0;
static bool    g_isReceivedRepeaterModeReportDevAddrAck = false;

static MonitorNodeDlCmdStatus_t g_monitor_node_dl_cmd_status = MonitorNode_NoDlCmd;
static uint8_t g_lora_dl_fct = 0;

/*!
 * Defines if linkcheck for each tx
 */
bool           g_auto_linkcheck = false;

/*!
 * Defines rssi channel list
 */
ChannelRssiSnr_t g_lora_channel_rssi_snr[8];

/*!
 * Defines if proprietary msg should be sent to uppper layer
 */
bool g_lora_proprietary_enable;

/*!
 * Defines the default freq mode
 */
static LoRaNodeFreqMode_t g_freq_mode = LORA_NODE_FREQ_TYPE;

/*!
 * Defines if enable monitor node function
 */
bool           g_monitor_node_feature_open = false;

typedef struct LinkCheck_s {
    uint8_t Result;
    uint8_t DemodMargin;
    uint8_t NbGateways;
} LinkCheck_t;

static LinkCheck_t g_linkcheck;

/*!
 *
 */
typedef enum {
    LORAMAC_HANDLER_UNCONFIRMED_MSG = 0,
    LORAMAC_HANDLER_CONFIRMED_MSG = !LORAMAC_HANDLER_UNCONFIRMED_MSG
} LoRaMacHandlerMsgTypes_t;

typedef enum {
    LORA_ERROR = -1,
    LORA_SUCCESS = 0
} LoraErrorStatus;

/*!
 * Application data structure
 */
typedef struct LoRaMacHandlerAppData_s {
    LoRaMacHandlerMsgTypes_t MsgType;
    uint8_t Port;
    uint8_t BufferSize;
    uint8_t *Buffer;
} LoRaMacHandlerAppData_t;

static LoRaMacHandlerAppData_t AppData = {
    .MsgType = LORAMAC_HANDLER_CONFIRMED_MSG,
    .Buffer = NULL,
    .BufferSize = 0,
    .Port = 0
};

/*!
 * LoRaMAC DevInfoToRepeater structure
 */
typedef struct sDevInfoToRepeaterReq {
    /*!
     * App Msg Type
     */
    uint32_t AppMsgType;

    struct sDevInfo {
        /*!
         * Device IEEE EUI
         */
        uint8_t DevEui[8];

        /*!
         * Mote Address
         */
        uint32_t DevAddr;
    } DevInfo;
} DevInfoToRepeaterReq;

#define HEX16(X)  X[0],X[1], X[2],X[3], X[4],X[5], X[6],X[7],X[8],X[9], X[10],X[11], X[12],X[13], X[14],X[15]
#define HEX8(X)   X[0],X[1], X[2],X[3], X[4],X[5], X[6],X[7]

/*!
 * MAC status strings
 */
static const char *MacStatusStrings[] = {
    "OK", "Busy", "Service unknown", "Parameter invalid", "Frequency invalid",
    "Datarate invalid", "Frequency or datarate invalid", "No network joined",
    "Length error", "Device OFF", "Region not supported", "Skipped APP data",
    "Duty-cycle restricted", "No channel found", "No free channel found",
    "Busy beacon reserved time", "Busy ping-slot window time",
    "Busy uplink collision", "Crypto error", "FCnt handler error",
    "MAC command error", "ERROR"
};

/*!
 * MAC event info status strings.
 */
static const char *EventInfoStatusStrings[] = {
    "OK", "Error", "Tx timeout", "Rx 1 timeout",
    "Rx 2 timeout", "Rx1 error", "Rx2 error",
    "Join failed", "Downlink repeated", "Tx DR payload size error",
    "Downlink too many frames loss", "dynamic multicast fcnt error",
    "Address fail", "MIC fail", "Multicast fail", "Beacon locked",
    "Beacon lost", "Beacon not found"
};

/*!
 * join method strings.
 */
static const char *JoinMethodStrings[] = {
    "stored freq band", "default freq band", "multi freq band"
};
//==============================Local Variable============================
static LoRaMacPrimitives_t LoRaMacPrimitives;
static LoRaMacCallback_t LoRaMacCallbacks;
static TimerEvent_t JoinBackoffTimer;
static TimerEvent_t ClassSwitchBackoffTimer;
static int ClassSwitchBackoffTimerInterval;
static TimerEvent_t ClassSwitchRespTimeoutTimer;
static TimerEvent_t RadioStatusTimer;
/*=========================================================================
                       Local Funtions
==========================================================================*/
static void LORA_ResetParameters(void);
static int  LORA_SwitchClass(DeviceClass_t newClass);
static void LORA_InitFinished(void);
static void LORA_SwitchClassResult(int successful);
static void LORA_ClassBLostSync(void);
static void LORA_SendDataOkWithoutRx(void);
static void LORA_SendDataFailed(void);
static void LORA_SetRssiSnr(int rssi, int snr);

/* call back when LoRa endNode has received a frame*/
static void LORA_RxData(Mcps_t type, uint8_t *Buff, uint8_t BuffSize, uint8_t Port);

/* call back when LoRa endNode has just joined*/
static void LORA_JoinResult( int successful );

/* call back when need to set the device state */
static int LORA_SetDeviceState ( DeviceState_t state);
static void LORA_ReInit(void);
static void LORA_ReJoin(void);

/* tx timer callback function*/
static void StartJoinTimer(void);
static void OnJoinBackoffTimerEvent(void);
static void OnClassSwitchRespTimeoutTimerEvent( void );
/* start to join*/
static void JoinNetwork( void );
/* join backoff handle*/
static void LORA_ResetJoinParam(bool joined);
static void LORA_PrepJoinParam(void);
static void LORA_UpdateJoinScanMask(void);
static void LORA_MonitorNodeProcessCmd(uint8_t *Buff, uint8_t BuffSize);
/*!
 * \brief Function to Report DevAddrTo Repeater
 */
static void LORA_RepeaterModeReportDevAddr(void);
/*!
 * \brief    register lorawan app protocol rx callback
 *
 * \param   [IN] port: port number.
 * \param   [IN] rx_cb_t: rx callback.
 *
 * \retval  Possible returns are:
 *          -1: fail.
 *           0: succesful.
 */
static int lorawan_module_reg_app_rx_cb(uint8_t port, rx_cb_t rx_cb);
/*!
 * \brief    get lorawan app protocol rx callback
 *
 * \param   [IN] port: port number.
 * \param   [IN] rx_cb_t: rx callback.
 *
 * \retval  Possible returns are:
 *          -1: fail.
 *           0: succesful.
 */
static int lorawan_module_get_app_rx_cb(uint8_t port, rx_cb_t *rx_cb);

/* load Main call backs structure*/
/* Lora Main callbacks*/
typedef struct sLoRaControllerCallback {
    void (*LORA_InitFinished)(void);
    void (*LORA_SwitchClassResult)(int successful);
    void (*LORA_ClassBLostSync)(void);
    void (*LORA_SendDataOkWithoutRx)(void);
    void (*LORA_SendDataFailed)(void);
    void (*LORA_SetRssiSnr)(int rssi, int snr);
    void (*LORA_RxData ) (Mcps_t type, uint8_t *Buff, uint8_t BuffSize, uint8_t Port);
    void (*LORA_JoinResult)( int successful);
} LoRaControllerCallback_t;

static LoRaControllerCallback_t LoRaControllerCallbacks = {
    LORA_InitFinished,
    LORA_SwitchClassResult,
    LORA_ClassBLostSync,
    LORA_SendDataOkWithoutRx,
    LORA_SendDataFailed,
    LORA_SetRssiSnr,
    LORA_RxData,
    LORA_JoinResult,
};

/*!
 * Prints the provided buffer in HEX
 *
 * \param buffer Buffer to be printed
 * \param size   Buffer size to be printed
 */
static void PrintHexBuffer( uint8_t *buffer, uint8_t size )
{
    uint8_t newline = 0;

    for ( uint8_t i = 0; i < size; i++ ) {
        if ( newline != 0 ) {
            PRINTF( "\r\n" );
            newline = 0;
        }

        PRINTF( "%02X ", buffer[i] );

        if ( ( ( i + 1 ) % 16 ) == 0 ) {
            newline = 1;
        }
    }
    PRINTF( "\r\n" );
}

static bool LORA_RepeaterModeReportDevAddrIsNeed()
{
    if ((g_lora_current_work_mode == NODE_WORK_MODE_REPEATER) &&
        (g_cntSendedDevAddrToRepeater < LORA_NODE_REPEATER_MODE_NUM_REPORT_DEVADDR) &&
        (g_isReceivedRepeaterModeReportDevAddrAck == false)) {
        return true;
    }
    return false;
}

static bool LORA_RepeaterModeReportDevAddrAckToReJoin()
{
    if ((g_lora_current_work_mode == NODE_WORK_MODE_REPEATER) &&
        (g_cntSendedDevAddrToRepeater  >= LORA_NODE_REPEATER_MODE_NUM_REPORT_DEVADDR) &&
        (g_isReceivedRepeaterModeReportDevAddrAck == false)) {
        return true;
    }
    return false;
}

static void LORA_RepeaterModeReportDevAddr()
{
    TimerTime_t curTime = TimerGetCurrentTime( );
    McpsReq_t mcpsReq;
    DevInfoToRepeaterReq  devInfoToRepeaterReq;
    LoRaMacStatus_t status;

    MibRequestConfirm_t mibReqJoin;
    MibRequestConfirm_t mibReqAddr;


    //get if Joined
    mibReqJoin.Type = MIB_NETWORK_ACTIVATION;
    LoRaMacMibGetRequestConfirm( &mibReqJoin );

    mibReqAddr.Type = MIB_DEV_ADDR;
    LoRaMacMibGetRequestConfirm( &mibReqAddr );

    if ( mibReqJoin.Param.NetworkActivation != ACTIVATION_TYPE_NONE ) {
        devInfoToRepeaterReq.AppMsgType = LORAWAN_NODE_REPEATER_MODE_REPORT_DEVADDR_FLAG;
        devInfoToRepeaterReq.DevInfo.DevAddr = mibReqAddr.Param.DevAddr;
        memcpyr(devInfoToRepeaterReq.DevInfo.DevEui, DevEui, 8);

        mcpsReq.Type = MCPS_PROPRIETARY;
        mcpsReq.Req.Proprietary.fBuffer = &devInfoToRepeaterReq;
        mcpsReq.Req.Proprietary.fBufferSize = sizeof(DevInfoToRepeaterReq);
        mcpsReq.Req.Proprietary.Datarate = LORA_NODE_REPEATER_CHANNEL_DR;

        LoRaMacStart();
        status = LoRaMacMcpsRequest( &mcpsReq );

        if ( status != LORAMAC_STATUS_OK ) {

        }
        PRINTF("LORA_RepeaterModeSendDevAddr,Status=%d,curTime=%d.\r\n", status, curTime);
    }
}

static void LORA_RepeaterModeReportDevAddrAck( DevInfoToRepeaterReq *reportDevAddrMsgAck)
{
    TimerTime_t curTime = TimerGetCurrentTime( );
    DevInfoToRepeaterReq  devInfoToRepeaterReq;

    MibRequestConfirm_t mibReqJoin;
    MibRequestConfirm_t mibReqAddr;

    //get if Joined
    mibReqJoin.Type = MIB_NETWORK_ACTIVATION;
    LoRaMacMibGetRequestConfirm( &mibReqJoin );

    mibReqAddr.Type = MIB_DEV_ADDR;
    LoRaMacMibGetRequestConfirm( &mibReqAddr );

    if ( mibReqJoin.Param.NetworkActivation != ACTIVATION_TYPE_NONE ) {

        if ( (reportDevAddrMsgAck->AppMsgType == LORAWAN_NODE_REPEATER_MODE_REPORT_DEVADDR_FLAG) &&
             (reportDevAddrMsgAck->DevInfo.DevAddr == mibReqAddr.Param.DevAddr) &&
             (reportDevAddrMsgAck->DevInfo.DevEui[0] == DevEui[7]) &&
             (reportDevAddrMsgAck->DevInfo.DevEui[1] == DevEui[6]) &&
             (reportDevAddrMsgAck->DevInfo.DevEui[2] == DevEui[5]) &&
             (reportDevAddrMsgAck->DevInfo.DevEui[3] == DevEui[4]) &&
             (reportDevAddrMsgAck->DevInfo.DevEui[4] == DevEui[3]) &&
             (reportDevAddrMsgAck->DevInfo.DevEui[5] == DevEui[2]) &&
             (reportDevAddrMsgAck->DevInfo.DevEui[6] == DevEui[1]) &&
             (reportDevAddrMsgAck->DevInfo.DevEui[7] == DevEui[0])) {

            g_isReceivedRepeaterModeReportDevAddrAck = true;
        }
    }
    PRINTF("\r\nLoRa: Received LORA_RepeaterModeReportDevAddrAck,ischeckResultSame=%d,curTime=%d.\r\n",
           g_isReceivedRepeaterModeReportDevAddrAck, curTime);

}

/*!
 * Executes the network Join request
 */
void JoinNetwork( void )
{
    MibRequestConfirm_t mibReq;
    LoRaMacStatus_t status;
    MlmeReq_t mlmeReq;

    LoRaModuleStatus_t ret;

    /* if already join, switch back to class A firstly */
    if (g_lora_current_class != CLASS_A) {
        ret = lorawan_control_switch_class(CLASS_A);
        if (ret == LoRa_Class_SwitchNOK) {
            return;
        }
    }

    if (g_monitor_node_feature_open == true) {
        if ((g_num_of_join_failed % 2) == 0) {
            g_join_datarate = DR_5;
        } else {
            g_join_datarate = DR_4;
        }
    }

    mlmeReq.Type = MLME_JOIN;
    mlmeReq.Req.Join.DevEui = DevEui;
    mlmeReq.Req.Join.JoinEui = JoinEui;
    mlmeReq.Req.Join.Datarate = g_join_datarate;

    StartJoinTimer();
    LoRaMacStart( );
    g_lora_control_current_status = LoRa_Joining;

    // Starts the join procedure
    status = LoRaMacMlmeRequest( &mlmeReq );
    PRINTF( "\r\nLoRa:MLME-Request - MLME_JOIN,STATUS: %s\r\n", MacStatusStrings[status] );

    if ( status == LORAMAC_STATUS_OK ) {
        PRINTF( "LoRa:JOINING\r\n" );
        LORA_SetDeviceState( DEVICE_STATE_SLEEP );
    } else {
        LORA_SetDeviceState( DEVICE_STATE_CYCLE );
    }
}

/*!
 * \brief   Prepares the payload of the frame
 */
static void PrepareTxFrame( uint8_t port )
{
#if 0
    /*!
     * Low battery threshold definition.
     */
#define LOW_BAT_THRESHOLD   3450 // mV

    MibRequestConfirm_t mibReq;

    if ( BoardGetBatteryVoltage( ) < LOW_BAT_THRESHOLD ) {
        mibReq.Type = MIB_CHANNELS_TX_POWER;
        LoRaMacMibGetRequestConfirm( &mibReq );
        // 30 dBm = TX_POWER_0, 28 dBm = TX_POWER_1, ..., 20 dBm = TX_POWER_5, ..., 10 dBm = TX_POWER_10
        // The if condition is then "less than" to check if the power is greater than 20 dBm
        if ( mibReq.Param.ChannelsTxPower < TX_POWER_5 ) {
            mibReq.Param.ChannelsTxPower = TX_POWER_5;
            LoRaMacMibSetRequestConfirm( &mibReq );
        }
    }
#endif

    switch ( port ) {
        case LORAWAN_APP_PORT: {
            g_lora_app_data_size_backup = g_lora_app_data_size = 1;
            g_lora_app_data_buffer[0] = 0xAB;
        }
        break;
        case LORA_MONITOR_NODE_CMD_PORT: {
            g_lora_app_data_size_backup = g_lora_app_data_size = 10;

            g_lora_app_data_buffer[0] = 0x00; //MsgType
            g_lora_app_data_buffer[1] = g_lora_app_data_size;   //MSgLength
            g_lora_app_data_buffer[2] = 0x00; //PageID
            g_lora_app_data_buffer[3] = BoardGetBatteryLevel();    //BatteryLevel
            g_lora_app_data_buffer[4] = g_monitor_node_dl_cmd_status;    //CMD Response
            g_lora_app_data_buffer[5] = (g_lora_rx_rssi & 0xFF00) >> 8;
            g_lora_app_data_buffer[6] = g_lora_rx_rssi & 0x00FF;
            g_lora_app_data_buffer[7] = g_lora_rx_snr;

            g_lora_app_data_buffer[8] = (g_lora_dl_fct & 0xFF00) >> 8;
            g_lora_app_data_buffer[9] = (g_lora_dl_fct & 0xFF);

            g_monitor_node_dl_cmd_status = MonitorNode_NoDlCmd;
        }
        break;
        case LORAWAN_CONTROLLER_PORT: {
            g_lora_app_data_size_backup = g_lora_app_data_size = LORAWAN_CONTROLLER_DATA_LEN;
            g_lora_app_data_buffer[0] = 0xDE;
        }
        break;
        case CERTIF_PORT:
            lorawan_certif_tx();
            break;
        default:
            break;
    }
}

/*!
 * \brief   Prepares the payload of the frame
 *
 * \retval  [0: frame could be send, 1: error]
 */
static bool SendFrame( void )
{
    McpsReq_t mcpsReq;
    LoRaMacTxInfo_t txInfo;

    if ( LoRaMacQueryTxPossible( g_lora_app_data_size, &txInfo ) != LORAMAC_STATUS_OK ) {
        // Send empty frame in order to flush MAC commands
        mcpsReq.Type = MCPS_UNCONFIRMED;
        mcpsReq.Req.Unconfirmed.fBuffer = NULL;
        mcpsReq.Req.Unconfirmed.fBufferSize = 0;
        mcpsReq.Req.Unconfirmed.Datarate = g_default_datarate;
    } else {
        if ( g_lora_is_confirmup == false ) {
            mcpsReq.Type = MCPS_UNCONFIRMED;
            mcpsReq.Req.Unconfirmed.fPort = g_lora_port;
            mcpsReq.Req.Unconfirmed.fBuffer = g_lora_app_data_buffer;
            mcpsReq.Req.Unconfirmed.fBufferSize = g_lora_app_data_size;
            mcpsReq.Req.Unconfirmed.Datarate = g_default_datarate;
        } else {
            mcpsReq.Type = MCPS_CONFIRMED;
            mcpsReq.Req.Confirmed.fPort = g_lora_port;
            mcpsReq.Req.Confirmed.fBuffer = g_lora_app_data_buffer;
            mcpsReq.Req.Confirmed.fBufferSize = g_lora_app_data_size;
            mcpsReq.Req.Confirmed.NbTrials = g_lora_nb_confirmup_trails;
            mcpsReq.Req.Confirmed.Datarate = g_default_datarate;
        }
    }

    // Update global variable
    AppData.MsgType = ( mcpsReq.Type == MCPS_CONFIRMED ) ? LORAMAC_HANDLER_CONFIRMED_MSG : LORAMAC_HANDLER_UNCONFIRMED_MSG;
    AppData.Port = mcpsReq.Req.Unconfirmed.fPort;
    AppData.Buffer = mcpsReq.Req.Unconfirmed.fBuffer;
    AppData.BufferSize = mcpsReq.Req.Unconfirmed.fBufferSize;

    LoRaMacStatus_t status;
    status = LoRaMacMcpsRequest( &mcpsReq );
    PRINTF( "\r\nLoRa:MCPS-Request-DATA,STATUS:%s\r\n", MacStatusStrings[status] );

    if (g_lora_class_switch_succes_flag) {
        g_lora_control_current_status = LoRa_Sending;
    }

    if ( status == LORAMAC_STATUS_OK ) {
        return false;
    }
    return true;
}

static void StartJoinTimer(void)
{
    int JoinInterval;

    if (LORA_NODE_DISABLE_JOIN_BACKOFF) {
        JoinInterval = randr(LORA_NODE_JOIN_MIN_INTERVAL * NUM_MS_PER_SECOND,
                             LORA_NODE_JOIN_MAX_INTERVAL * NUM_MS_PER_SECOND);
    } else {
        // Starts the join backoff timer
        JoinInterval = (1 << g_num_of_join_failed) * NUM_MS_PER_SECOND * NUM_SECOND_PER_MINUTE +
                       g_random_joindelay; //unit =ms; 1000*60s
    }
    //check Min value
    ////Must be greater than 9s for repeat mode,due to joinAcceptDelay+= 2s;
    if (g_lora_current_work_mode == NODE_WORK_MODE_REPEATER) {
        if (JoinInterval < LORA_NODE_JOIN_MAX_INTERVAL * NUM_MS_PER_SECOND) {
            JoinInterval = LORA_NODE_JOIN_MAX_INTERVAL * NUM_MS_PER_SECOND + LORA_NODE_REPEATER_MODE_RX2DELAY_PLUS;
        }
    }

    //check Max value
    if (JoinInterval > (NUM_MS_PER_SECOND * NUM_SECOND_PER_MINUTE * NUM_MINUTE_PER_HOUR)) {
        JoinInterval = NUM_MS_PER_SECOND * NUM_SECOND_PER_MINUTE * NUM_MINUTE_PER_HOUR;
    }
    TimerSetValue(&JoinBackoffTimer, JoinInterval);
    TimerStart(&JoinBackoffTimer);

    PRINTF("LoRa: Next Join Delay Seconds =%d, NumofJoin=%d\r\n", JoinInterval / NUM_MS_PER_SECOND, g_num_of_join_failed);
}

static void OnJoinBackoffTimerEvent( void )
{
    LORA_SetDeviceState(DEVICE_STATE_JOIN);
}

/*!
 * \brief Function executed on TxNextPacket Timeout event
 */
void OnTxNextPacketTimerEvent( void )
{
    MibRequestConfirm_t mibReq;
    LoRaMacStatus_t status;

    TimerStop( &TxNextPacketTimer );

    mibReq.Type = MIB_NETWORK_ACTIVATION;
    status = LoRaMacMibGetRequestConfirm( &mibReq );

    if ( status == LORAMAC_STATUS_OK ) {
        if ( mibReq.Param.NetworkActivation == ACTIVATION_TYPE_NONE ) {
            // Network not joined yet. Try to join again
            JoinNetwork( );
        } else {
            DeviceState = WakeUpState;
            NextTx = true;
        }
    }
}

/*!
 * \brief Function executed on StartNextClassSwitch Timeout event
 */
static void OnStartNextClassSwitchTimerEvent( void )
{
    TimerStop( &ClassSwitchBackoffTimer );

    //start to send
    NextTx = true;
    lorawan_control_switch_class(g_current_target_lora_class_type);

    PRINTF("LoRa: switching to ClassB from ClassA,waiting 2 minutes...\r\n");
}

/*!
 * \brief Function executed on ClassSwitchRespTimeoutTimer Timeout event
 */
static void OnClassSwitchRespTimeoutTimerEvent( void )
{
    //timer_set(&TimerSwitchClassBResultCheck, TIMER_SWITCH_CLASSB_RESULT_CHECK);
    PRINTF("LoRa: happen ClassBSwitchRespTimeout.\r\n");

    TimerStop( &ClassSwitchRespTimeoutTimer);

    //start classB switch again
    OnStartNextClassSwitchTimerEvent();
}

static void OnRadioStatusEvent(void)
{
    static int pre_cnt = 0;
    int cur_cnt = LoRaMacGetEventCnt();
    PRINTNOW();
    PRINTF("LoRa: Radio status checked.\r\n");
    if (cur_cnt == pre_cnt) {
        PRINTF("LoRa: Critical error, reinit lora\r\n");
        LORA_ReJoin();
    }

    pre_cnt = cur_cnt;
    TimerStart(&RadioStatusTimer);
}

static void ATPrintSendData(const char *header, Mcps_t mcpsreq, uint8_t nbretries)
{
    switch (mcpsreq) {
        case MCPS_UNCONFIRMED: {
            MibRequestConfirm_t mib_req;
            LoRaMacStatus_t status;
            uint8_t unconfirm_trails = 0;

            mib_req.Type = MIB_CHANNELS_NB_TRANS;
            status       = LoRaMacMibGetRequestConfirm(&mib_req);
            if (status == LORAMAC_STATUS_OK) {
                unconfirm_trails = mib_req.Param.ChannelsNbTrans;
            }
            LORA_AT_PRINTF("%s+SENT:%d\r\n", header, unconfirm_trails);
            break;
        }
        case MCPS_CONFIRMED: {
            LORA_AT_PRINTF("%s+SENT:%d\r\n", header, nbretries);
            break;
        }
        default:
            break;
    }

    return;
}

/*!
 * \brief   MCPS-Confirm event function
 *
 * \param   [IN] mcpsConfirm - Pointer to the confirm structure,
 *               containing confirm attributes.
 */
static void McpsConfirm( McpsConfirm_t *mcpsConfirm )
{
    PRINTF( "\r\nLoRa:MCPS-Confirm,STATUS: %s\r\n", EventInfoStatusStrings[mcpsConfirm->Status] );
    if ( mcpsConfirm->Status != LORAMAC_EVENT_INFO_STATUS_OK ) {
        ATPrintSendData("ERR", mcpsConfirm->McpsRequest, mcpsConfirm->NbRetries);
        /*notify upper layer */
        LoRaControllerCallbacks.LORA_SendDataFailed();
    } else {
        switch ( mcpsConfirm->McpsRequest ) {
            case MCPS_UNCONFIRMED: {
                // Check Datarate
                // Check TxPower

                ATPrintSendData("OK", MCPS_UNCONFIRMED, mcpsConfirm->NbRetries);
                /*notify upper layer */
                LoRaControllerCallbacks.LORA_SendDataOkWithoutRx();
                break;
            }
            case MCPS_CONFIRMED: {
                // Check Datarate
                // Check TxPower
                // Check AckReceived
                // Check NbTrials

                /*notify upper layer */
                if (mcpsConfirm->AckReceived) {
                    ATPrintSendData("OK", MCPS_CONFIRMED, mcpsConfirm->NbRetries);
                    LoRaControllerCallbacks.LORA_SendDataOkWithoutRx();
                } else {
                    ATPrintSendData("ERR", MCPS_CONFIRMED, mcpsConfirm->NbRetries);
                    LORA_AT_PRINTF("ERR+SENT:%d\r\n", mcpsConfirm->NbRetries);
                    LoRaControllerCallbacks.LORA_SendDataFailed();
                }

                break;
            }
            case MCPS_PROPRIETARY: {
                if (true == LORA_RepeaterModeReportDevAddrAckToReJoin()) {
                    //rejoin
                    PRINTF("\r\nLoRa: LORA_RepeaterModeReportDevAddrAckToReJoin is true, start reJoin.\r\n");
                    LORA_ReJoin();
                }
                break;
            }
            default:
                break;
        }
    }

#if 0

    MibRequestConfirm_t mibGet;
    MibRequestConfirm_t mibReq;

    mibReq.Type = MIB_DEVICE_CLASS;
    LoRaMacMibGetRequestConfirm( &mibReq );

    PRINTF( "\r\n###### ===== UPLINK FRAME %lu ==== ######\r\n", mcpsConfirm->UpLinkCounter );
    PRINTF( "\r\n" );

    PRINTF( "CLASS       : %c\r\n", "ABC"[mibReq.Param.Class] );
    PRINTF( "\r\n" );
    PRINTF( "TX PORT     : %d\r\n", AppData.Port );

    if ( AppData.BufferSize != 0 ) {
        PRINTF( "TX DATA     : " );
        if ( AppData.MsgType == LORAMAC_HANDLER_CONFIRMED_MSG ) {
            PRINTF( "CONFIRMED - %s\r\n", ( mcpsConfirm->AckReceived != 0 ) ? "ACK" : "NACK" );
        } else {
            PRINTF( "UNCONFIRMED\r\n" );
        }
        PrintHexBuffer( AppData.Buffer, AppData.BufferSize );
    }

    PRINTF( "\r\n" );
    PRINTF( "DATA RATE   : DR_%d\r\n", mcpsConfirm->Datarate );

    mibGet.Type  = MIB_CHANNELS;
    if ( LoRaMacMibGetRequestConfirm( &mibGet ) == LORAMAC_STATUS_OK ) {
        PRINTF( "U/L FREQ    : %lu\r\n", mibGet.Param.ChannelList[mcpsConfirm->Channel].Frequency );
    }

    PRINTF( "TX POWER    : %d\r\n", mcpsConfirm->TxPower );

    mibGet.Type  = MIB_CHANNELS_MASK;
    if ( LoRaMacMibGetRequestConfirm( &mibGet ) == LORAMAC_STATUS_OK ) {
        PRINTF("CHANNEL MASK: ");
#if defined( REGION_AS923 ) || defined( REGION_CN779 ) || \
    defined( REGION_EU868 ) || defined( REGION_IN865 ) || \
    defined( REGION_KR920 ) || defined( REGION_EU433 ) || \
    defined( REGION_RU864 )

        for ( uint8_t i = 0; i < 1; i++)

#elif defined( REGION_AU915 ) || defined( REGION_US915 ) || defined( REGION_CN470 )

        for ( uint8_t i = 0; i < 5; i++)
#else

#error "Please define a region in the compiler options."

#endif
        {
            PRINTF("%04X ", mibGet.Param.ChannelsMask[i] );
        }
        PRINTF("\r\n");
    }

    PRINTF( "\r\n" );
#endif
}

static void ATPrintRxData(Mcps_t frame_type, bool recv, uint8_t Port, uint8_t *Buff, uint8_t BuffSize)
{
    uint8_t type = 0;

    if (frame_type == MCPS_CONFIRMED) {
        type |= 1;
    }

    if (recv) {
        type |= (1 << 1);
    }

    LORA_AT_PRINTF("\r\nOK+RECV:%02x,%02x,%02x", type, Port, BuffSize);

    for (uint8_t i = 0; i < BuffSize; i++) {

        if (i == 0) {
            LORA_AT_PRINTF(",");
        }

        LORA_AT_PRINTF("%02x", Buff[i]);
    }

    LORA_AT_PRINTF("\r\n");
}

/*!
 * \brief   MCPS-Indication event function
 *
 * \param   [IN] mcpsIndication - Pointer to the indication structure,
 *               containing indication attributes.
 */
static void McpsIndication( McpsIndication_t *mcpsIndication )
{
    PRINTF( "\r\nLoRa:FPending=%d,MCPS-Indication STATUS: %s\r\n", mcpsIndication->FramePending,
            EventInfoStatusStrings[mcpsIndication->Status] );
    if ( mcpsIndication->Status != LORAMAC_EVENT_INFO_STATUS_OK ) {
        return;
    }


    switch ( mcpsIndication->McpsIndication ) {
        case MCPS_UNCONFIRMED: {
            break;
        }
        case MCPS_CONFIRMED: {
            break;
        }
        case MCPS_PROPRIETARY: {
            break;
        }
        case MCPS_MULTICAST: {
            break;
        }
        default:
            break;
    }

    // Check Multicast
    // Check Port
    // Check Datarate
    // Check FramePending
    if (( mcpsIndication->FramePending == true ) && (CLASS_A == g_current_target_lora_class_type) &&
        (( MCPS_CONFIRMED == mcpsIndication->McpsIndication ))) {
        // The server signals that it has pending data to be sent.
        // We schedule an uplink as soon as possible to flush the server.
        OnTxNextPacketTimerEvent( );
    }
    // Check Buffer
    // Check BufferSize
    // Check Rssi
    // Check Snr
    // Check RxSlot

    if (lorawan_certif_running() == true) {
        lorawan_certif_DownLinkIncrement( );
    }

    DevInfoToRepeaterReq reportDevAddrMsg;

    if ( mcpsIndication->RxData == true ) {
        /*notify upper layer */
        if (mcpsIndication->McpsIndication != MCPS_PROPRIETARY || g_lora_proprietary_enable) {
            LoRaControllerCallbacks.LORA_RxData(mcpsIndication->McpsIndication,
                                                mcpsIndication->Buffer,
                                                mcpsIndication->BufferSize,
                                                mcpsIndication->Port);
        }

        switch ( mcpsIndication->Port ) {
            case 1: // The application LED can be controlled on port 1 or 2
            case 2:
                if ( mcpsIndication->BufferSize == 1 ) {
                    AppLedStateOn = mcpsIndication->Buffer[0] & 0x01;
                }
                break;
            case CERTIF_PORT:
                lorawan_certif_rx( mcpsIndication );
                break;

            default:
                break;
        }
    } else if (mcpsIndication->McpsIndication == MCPS_PROPRIETARY) {
        //only for repeater mode,not notify upper layer
        memcpy(&reportDevAddrMsg, mcpsIndication->Buffer, sizeof(DevInfoToRepeaterReq));

        PRINTF( "\r\nLoRa:reportDevAddrMsg.AppMsgType= %x\r\n", reportDevAddrMsg.AppMsgType);
        if ((g_lora_current_work_mode == NODE_WORK_MODE_REPEATER) &&
            (mcpsIndication->BufferSize == sizeof(DevInfoToRepeaterReq)) &&
            (reportDevAddrMsg.AppMsgType == LORAWAN_NODE_REPEATER_MODE_REPORT_DEVADDR_FLAG)) {

            LORA_RepeaterModeReportDevAddrAck( &reportDevAddrMsg);
        }
    } else {
        PRINTF("LoRa:McpsIndication: no RxData\r\n");
    }

    const char *slotStrings[] = { "1", "2", "C", "Ping-Slot", "Multicast Ping-Slot" };
    int8_t snr = mcpsIndication->Snr;

    /*notify upper layer */
    LoRaControllerCallbacks.LORA_SetRssiSnr(mcpsIndication->Rssi, snr);
    g_lora_dl_fct = mcpsIndication->DownLinkCounter;

    PRINTF( "\r\nLoRa:DOWNLINK FRAME %lu ==== ######\r\n", mcpsIndication->DownLinkCounter );

    PRINTNOW();
    PRINTF( "RX WINDOW   : %s\r\n", slotStrings[mcpsIndication->RxSlot] );

    PRINTF( "RX PORT     : %d\r\n", mcpsIndication->Port );

    if ( mcpsIndication->BufferSize != 0 ) {
        PRINTF( "RX DATA     : \r\n" );
        PrintHexBuffer( mcpsIndication->Buffer, mcpsIndication->BufferSize );
    }

    PRINTF( "\r\n" );
    PRINTF( "DATA RATE   : DR_%d\r\n", mcpsIndication->RxDatarate );
    PRINTF( "RX RSSI     : %d\r\n", mcpsIndication->Rssi );
    PRINTF( "RX SNR      : %d\r\n", snr );

    PRINTF( "\r\n" );

    /* send AT event */
    ATPrintRxData(mcpsIndication->McpsIndication, mcpsIndication->AckReceived,
                  mcpsIndication->Port, mcpsIndication->Buffer, mcpsIndication->BufferSize);
    if (g_linkcheck.Result) {
        LORA_AT_PRINTF("\r\n+CLINKCHECK:%d,%d,%d,%d,%d\r\n", g_linkcheck.Result, g_linkcheck.DemodMargin,
                       g_linkcheck.NbGateways, mcpsIndication->Rssi, snr);
        g_linkcheck.Result = 0;
    }


    GetPhyParams_t getPhy;
    PhyParam_t phyParam;
    int8_t rx_channel;

    getPhy.Attribute = PHY_RX_CHANNEL;
    phyParam = RegionGetPhyParam( ACTIVE_REGION, &getPhy );

    rx_channel = (int8_t)( phyParam.Value );
    if (rx_channel >= 0 && rx_channel < 8) {
        g_lora_channel_rssi_snr[rx_channel].rssi = mcpsIndication->Rssi;
        g_lora_channel_rssi_snr[rx_channel].snr = snr;
    }

    //if FCntDown Rollback, need to join
    if ( mcpsIndication->DownLinkCounter >= LORAWAN_FCNT_MAX) {
        //rejoin
        PRINTF("LoRa: FCntUp Rollback happen, start reJoin.\r\n");
        LORA_ReJoin();
    }

}

/*!
 * \brief   MLME-Confirm event function
 *
 * \param   [IN] mlmeConfirm - Pointer to the confirm structure,
 *               containing confirm attributes.
 */
static void MlmeConfirm( MlmeConfirm_t *mlmeConfirm )
{
    MibRequestConfirm_t mibReq;

    PRINTF( "\r\nLoRa:MLME-Confirm,STATUS: %s\r\n", EventInfoStatusStrings[mlmeConfirm->Status] );
    if ( mlmeConfirm->Status != LORAMAC_EVENT_INFO_STATUS_OK ) {
    }
    switch ( mlmeConfirm->MlmeRequest ) {
        case MLME_JOIN: {
            if ( mlmeConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK ) {
                MibRequestConfirm_t mibGet;
                //PRINTF( "LoRa:JOINED Successful,OTAA.\r\n\r\n" );

                mibGet.Type = MIB_DEV_ADDR;
                LoRaMacMibGetRequestConfirm( &mibGet );
                PRINTF( "DevAddr     : %08lX\r\n", mibGet.Param.DevAddr );

                PRINTF( "\n\r\n" );
                mibGet.Type = MIB_CHANNELS_DATARATE;
                LoRaMacMibGetRequestConfirm( &mibGet );
                PRINTF( "DATA RATE   : DR_%d\r\n", mibGet.Param.ChannelsDatarate );
                PRINTF( "\r\n" );
                // Status is OK, node has joined the network
                /*
                #if defined( USE_BEACON_TIMING )
                                DeviceState = DEVICE_STATE_REQ_BEACON_TIMING;
                #else
                                DeviceState = DEVICE_STATE_REQ_DEVICE_TIME;
                #endif
                */
                /*notify upper layer */
                LoRaControllerCallbacks.LORA_JoinResult(true);

            } else {
                // Join was not successful. Try to join again
                //PRINTF( "LoRa:JOIN failed,Try to join later ######\r\n" );
                //DelayMs(10*1000);
                //JoinNetwork( );

                /*notify upper layer */
                LoRaControllerCallbacks.LORA_JoinResult(false);
            }
            break;
        }
        case MLME_LINK_CHECK: {
            if ( mlmeConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK ) {
                // Check DemodMargin
                // Check NbGateways
                g_linkcheck.Result = 1;
                g_linkcheck.DemodMargin = mlmeConfirm->DemodMargin;
                g_linkcheck.NbGateways = mlmeConfirm->NbGateways;

                if (lorawan_certif_running() == true) {
                    lorawan_certif_linkCheck(mlmeConfirm);
                }
            } else {
                g_lora_control_current_status = LoRa_Network_Abnormal;
                LORA_AT_PRINTF("\r\n+CLINKCHEK:-1\r\n");
            }
            break;
        }
        case MLME_DEVICE_TIME: {
            if (g_lora_current_class != CLASS_B) { //not need when under classB sending DEVICE_TIME_Req for Beacon Miss
                if ( mlmeConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK ) {
                    // Setup the WakeUpState to DEVICE_STATE_SEND. This allows the
                    // application to initiate MCPS requests during a beacon acquisition
                    WakeUpState = DEVICE_STATE_SEND;
                    // Switch to the next state immediately
                    LORA_SetDeviceState( DEVICE_STATE_BEACON_ACQUISITION );
                    NextTx = true;
                    PRINTF( "LoRa:Received MLME_DEVICE_TIME_ANS,start BEACON_ACQUISITION. \n\r\n" );
                } else {
                    PRINTF( "LoRa:DEVICE_TIME_ANS Not Received.Switch Class failed.\n\r\n" );
                    /*notify upper layer */
                    NextTx = true;
                    LoRaControllerCallbacks.LORA_SwitchClassResult(false);
                }
            } else {
                g_num_beacon_missed = 0;
            }
            break;
        }
        case MLME_BEACON_TIMING: {
            // Setup the WakeUpState to DEVICE_STATE_SEND. This allows the
            // application to initiate MCPS requests during a beacon acquisition
            WakeUpState = DEVICE_STATE_SEND;
            // Switch to the next state immediately
            LORA_SetDeviceState( DEVICE_STATE_BEACON_ACQUISITION );
            NextTx = true;
            break;
        }
        case MLME_BEACON_ACQUISITION: {
            if ( mlmeConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK ) {
                if (g_lora_current_class != CLASS_B) { //not need when under classB sending DEVICE_TIME_Req for Beacon Miss
                    WakeUpState = DEVICE_STATE_SEND;
                    // Switch to the next state immediately
                    LORA_SetDeviceState( DEVICE_STATE_REQ_PINGSLOT_ACK );
                    NextTx = true;
                    PRINTF( "LoRa:Beacon Found,start PINGSLOT_REQ.\n\r\n" );
                }
            } else {
                PRINTF( "LoRa:Beacon Not Found.Switch Class failed.\n\r\n" );
                /*
                #if defined( USE_BEACON_TIMING )
                                WakeUpState = DEVICE_STATE_REQ_BEACON_TIMING;
                #else
                                WakeUpState = DEVICE_STATE_REQ_DEVICE_TIME;
                #endif
                */
                /*notify upper layer */
                NextTx = true;
                LoRaControllerCallbacks.LORA_SwitchClassResult(false);
            }
            break;
        }
        case MLME_PING_SLOT_INFO: {
            if ( mlmeConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK ) {
                mibReq.Type = MIB_DEVICE_CLASS;
                mibReq.Param.Class = CLASS_B;
                LoRaMacMibSetRequestConfirm( &mibReq );

                //PRINTF( "\r\n\r\n###### ===== Switch to Class B done. ==== ######\r\n\r\n" );

                WakeUpState = DEVICE_STATE_SEND;
                LORA_SetDeviceState( WakeUpState );
                NextTx = true;
                g_num_of_pingslotack_failed = 0;

                /*notify upper layer */
                LoRaControllerCallbacks.LORA_SwitchClassResult(true);
            } else {
                //WakeUpState = DEVICE_STATE_REQ_PINGSLOT_ACK;

                g_num_of_pingslotack_failed++;

                if (g_num_of_pingslotack_failed < LORA_NODE_NUM_PINGSLOTINFORACK_FAILED_TO_REINIT) {
                    WakeUpState = DEVICE_STATE_SEND;
                    // Switch to the next state immediately
                    LORA_SetDeviceState( DEVICE_STATE_REQ_PINGSLOT_ACK );
                    NextTx = true;
                    PRINTF( "LoRa:PINGSLOT_ACK Not Received,start PINGSLOT_REQ again.\n\r\n" );
                } else {
                    PRINTF( "LoRa:PINGSLOT_ACK Not Received. Switch Class failed.\n\r\n" );
                    /*notify upper layer */
                    NextTx = true;
                    g_num_of_pingslotack_failed = 0;

                    LoRaControllerCallbacks.LORA_SwitchClassResult(false);
                }
            }
            break;
        }
        default:
            break;
    }
}

/*!
 * \brief   MLME-Indication event function
 *
 * \param   [IN] mlmeIndication - Pointer to the indication structure.
 */
static void MlmeIndication( MlmeIndication_t *mlmeIndication )
{
    MibRequestConfirm_t mibReq;

    if ( mlmeIndication->Status != LORAMAC_EVENT_INFO_STATUS_BEACON_LOCKED ) {
        PRINTF( "\r\nLoRa:MLME-Indication,STATUS: %s\r\n", EventInfoStatusStrings[mlmeIndication->Status] );
    }
    if ( mlmeIndication->Status != LORAMAC_EVENT_INFO_STATUS_OK ) {
    }
    switch ( mlmeIndication->MlmeIndication ) {
        case MLME_JOIN: {
            // The MAC signals that we shall provide an join as soon as possible
            PRINTF("LoRa:LoRaMac trigger a MLME_JOIN.\n\r");
            //rejoin
            LORA_SetDeviceState(DEVICE_STATE_REJOIN);
            break;
        }
        case MLME_SCHEDULE_UPLINK: {
            // The MAC signals that we shall provide an uplink as soon as possible
            PRINTF("LoRa:LoRaMac trigger a MLME_SCHEDULE_UPLINK,DeviceState=%d.\n\r", DeviceState);
            NextTx = true;//OnTxNextPacketTimerEvent( );
            LORA_SetDeviceState(DEVICE_STATE_SEND);
            break;
        }
        case MLME_BEACON_LOST: {
            mibReq.Type = MIB_DEVICE_CLASS;
            mibReq.Param.Class = CLASS_A;
            LoRaMacMibSetRequestConfirm( &mibReq );

            PRINTF( "\r\n\r\nLoRa: Switch to Class A done. ==== ######\r\n\r\n" );

            // Switch to class A again
#if defined( USE_BEACON_TIMING )
            WakeUpState = DEVICE_STATE_REQ_BEACON_TIMING;
#else
            WakeUpState = DEVICE_STATE_REQ_DEVICE_TIME;
#endif
            PRINTF( "\r\nLoRa: BEACON LOST ==== ######\r\n" );
            /*notify upper layer */
            LoRaControllerCallbacks.LORA_ClassBLostSync();

            break;
        }
        case MLME_BEACON: {
            if ( mlmeIndication->Status == LORAMAC_EVENT_INFO_STATUS_BEACON_LOCKED ) {
                PRINTF( "LoRa:BEACON RECEIVED.\n\r");
                PRINTF( "\r\n###### ===== BEACON %lu ==== ######\r\n", mlmeIndication->BeaconInfo.Time );
                PRINTF( "GW DESC     : %d\r\n", mlmeIndication->BeaconInfo.GwSpecific.InfoDesc );
                PRINTF( "GW INFO     : " );
                PrintHexBuffer( mlmeIndication->BeaconInfo.GwSpecific.Info, 6 );
                PRINTF( "\r\n" );
                PRINTF( "FREQ        : %lu\r\n", mlmeIndication->BeaconInfo.Frequency );
                PRINTF( "DATA RATE   : DR_%d\r\n", mlmeIndication->BeaconInfo.Datarate );
                PRINTF( "RX RSSI     : %d\r\n", mlmeIndication->BeaconInfo.Rssi );

                int32_t snr = 0;
                if ( mlmeIndication->BeaconInfo.Snr & 0x80 ) { // The SNR sign bit is 1
                    // Invert and divide by 4
                    snr = ( ( ~mlmeIndication->BeaconInfo.Snr + 1 ) & 0xFF ) >> 2;
                    snr = -snr;
                } else {
                    // Divide by 4
                    snr = ( mlmeIndication->BeaconInfo.Snr & 0xFF ) >> 2;
                }
                PRINTF( "RX SNR      : %ld\r\n", snr );
                PRINTF( "\r\n" );

                g_num_beacon_missed = 0;

            } else {
                PRINTF( "\r\nLoRa: BEACON NOT RECEIVED ==== ######\r\n" );
                g_num_beacon_missed++;
                if (g_num_beacon_missed >= LORA_NODE_NUM_BEACONMISS_TO_SEND_DEVICETIMEREQ) {
                    PRINTF("LoRa:BeaconMissedTimesIsGreaterThanDefined,willSendDeviceReq,currState=%d.\n\r", DeviceState);
                    NextTx = true;//OnTxNextPacketTimerEvent( );
                    LORA_SetDeviceState(DEVICE_STATE_REQ_DEVICE_TIME);
                }
            }
            break;
        }
        default:
            break;
    }
}

static void LORA_ConnectedProcess(void)
{
    bool cycleSendEnable = false;
    g_num_of_class_switch_failed = 0;
    g_lora_class_switch_succes_flag = true;
    g_lora_control_current_status = LoRa_Idle;

    //timer_stop(&TimerSwitchClassBResultCheck);

    g_lora_num_received_data_ack = 0;
    g_lora_num_send_uplink        = 0;

    if (g_lora_app_tx_report_mode) {
        PRINTF( "\r\nLoRa: Enter Data Cycle Send. ==== ######\r\n" );
        cycleSendEnable = true;
    }

    if ((g_lora_current_class == CLASS_B) || (cycleSendEnable)) {
        //classB:ensure NS receive the first package with classB bit.
        NextTx = true;
        LORA_SetDeviceState(DEVICE_STATE_SEND);
        WakeUpState = DEVICE_STATE_SEND;

        TimerStart( &TxNextPacketTimer );
    }
    //call CB to notify App
    if (g_lora_esl_connection_ops_cb->on_connect) {
        g_lora_esl_connection_ops_cb->on_connect(true, g_lora_cm_data);
    }

    if (g_monitor_node_feature_open) {
        //after join and classSwitch, default use monitor node port
        g_lora_port    = LORA_MONITOR_NODE_CMD_PORT;
    }

}

void LORA_ResetParameters(void)
{
    //reset counter
    g_lora_num_send_uplink = 0;
    g_lora_num_received_data_ack = 0;
    //join backoff
    g_lora_control_current_status = LoRa_Initing;

    g_num_of_join_failed = 0;
    g_num_send_data_failed_to_rejoin = 0;
    g_num_of_class_switch_failed = 0;
    g_lora_class_switch_succes_flag = true;
    g_lora_current_class = CLASS_A;

    g_num_beacon_missed = 0;
    g_num_of_pingslotack_failed = 0;

    g_cntSendedDevAddrToRepeater = 0;
    g_isReceivedRepeaterModeReportDevAddrAck = false;

}

static void LORA_MacReInit(void)
{
    LoRaMacStart( );

    TimerStop( &TxNextPacketTimer );
    NextTx = true;
}


static void LORA_ReInit(void)
{
    //call CB to notify App
    if (g_lora_esl_connection_ops_cb->on_disconnect) {
        g_lora_esl_connection_ops_cb->on_disconnect(g_lora_cm_data);
    }

    LORA_ResetParameters();
    //lorawan_module_join();
    LORA_SetDeviceState(DEVICE_STATE_REINIT);
}

static void LORA_ReJoin(void)
{
    LORA_ResetParameters();
    LORA_MacReInit();
    //lorawan_module_join();
    LORA_SetDeviceState(DEVICE_STATE_JOIN);
}

static void LoRaSetABPParam(void)
{
    MibRequestConfirm_t mibReq;
    // Choose a random device address if not already defined in Commissioning.h
    if ( DevAddr == 0 ) {
        // Random seed initialization
        srand1( BoardGetRandomSeed( ) );

        // Choose a random device address
        DevAddr = randr( 0, 0x01FFFFFF );
    }

    mibReq.Type = MIB_NET_ID;
    mibReq.Param.NetID = LORAWAN_NETWORK_ID;
    LoRaMacMibSetRequestConfirm( &mibReq );

    mibReq.Type = MIB_DEV_ADDR;
    mibReq.Param.DevAddr = DevAddr;
    LoRaMacMibSetRequestConfirm( &mibReq );

    mibReq.Type = MIB_F_NWK_S_INT_KEY;
    mibReq.Param.FNwkSIntKey = FNwkSIntKey;
    LoRaMacMibSetRequestConfirm( &mibReq );

    mibReq.Type = MIB_S_NWK_S_INT_KEY;
    mibReq.Param.SNwkSIntKey = SNwkSIntKey;
    LoRaMacMibSetRequestConfirm( &mibReq );

    mibReq.Type = MIB_NWK_S_ENC_KEY;
    mibReq.Param.NwkSEncKey = NwkSEncKey;
    LoRaMacMibSetRequestConfirm( &mibReq );

    mibReq.Type = MIB_APP_S_KEY;
    mibReq.Param.AppSKey = AppSKey;
    LoRaMacMibSetRequestConfirm( &mibReq );
    return;
}

DeviceState_t lorawan_control_process(void)
{
    MibRequestConfirm_t mibReq;

    switch ( DeviceState ) {
        case DEVICE_STATE_REINIT: {
            PRINTF( "LoRa:start REINIT\n\r");
            LoRaMacInitialization( &LoRaMacPrimitives, &LoRaMacCallbacks, ACTIVE_REGION );
            lorawan_control_mc_setup();
            DeviceState = DEVICE_STATE_RESTORE;
            WakeUpState = DEVICE_STATE_START;
            LORA_MacReInit();

            break;
        }
        case DEVICE_STATE_RESTORE: {
            LORA_ResetJoinParam(false);
            LORA_PrepJoinParam();
            lorawan_control_set_work_mode(LORA_NODE_WORKMODE);
            /* region param should be updated */
            lorawan_control_set_freq_mode(g_freq_mode);

            // Try to restore from NVM and query the mac if possible.
            if (0) { //( NvmCtxMgmtRestore( ) == NVMCTXMGMT_STATUS_SUCCESS )
                PRINTF( "\r\nLoRa:###### ===== CTXS RESTORED ==== ######\r\n\r\n" );
            } else {
                // Initialize LoRaMac device unique ID if not already defined in Commissioning.h
                if ( ( DevEui[0] == 0 ) && ( DevEui[1] == 0 ) &&
                     ( DevEui[2] == 0 ) && ( DevEui[3] == 0 ) &&
                     ( DevEui[4] == 0 ) && ( DevEui[5] == 0 ) &&
                     ( DevEui[6] == 0 ) && ( DevEui[7] == 0 ) ) {
                    //BoardGetUniqueId( DevEui );
                }
            }

            DeviceState = DEVICE_STATE_START;
            break;
        }

        case DEVICE_STATE_START: {
            TimerInit( &TxNextPacketTimer, OnTxNextPacketTimerEvent );

            mibReq.Type = MIB_PUBLIC_NETWORK;
            mibReq.Param.EnablePublicNetwork = LORAWAN_PUBLIC_NETWORK;
            LoRaMacMibSetRequestConfirm( &mibReq );

            mibReq.Type = MIB_ADR;
            mibReq.Param.AdrEnable = LORAWAN_ADR_ON;
            LoRaMacMibSetRequestConfirm( &mibReq );

#if defined( REGION_EU868 )
            LoRaMacTestSetDutyCycleOn( LORAWAN_DUTYCYCLE_ON );
#endif
            mibReq.Type = MIB_SYSTEM_MAX_RX_ERROR;
            mibReq.Param.SystemMaxRxError = 15;
            LoRaMacMibSetRequestConfirm( &mibReq );

            LoRaMacStart( );

            DeviceState = DEVICE_STATE_SLEEP;
            break;
        }
        case DEVICE_STATE_JOIN: {
            if (g_lora_otaa_mode) {
                mibReq.Type = MIB_APP_KEY;
                mibReq.Param.AppKey = AppKey;
                LoRaMacMibSetRequestConfirm( &mibReq );

                mibReq.Type = MIB_NWK_KEY;
                mibReq.Param.NwkKey = NwkKey;
                LoRaMacMibSetRequestConfirm( &mibReq );

                PRINTF( "=======================================================\r\n" );
                PRINTF( "LoRa:OTAA\r\n");
                PRINTF( "LoRa:DevEui= %02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X\r\n", HEX8(DevEui));
                PRINTF( "LoRa:AppEui= %02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X\r\n", HEX8(JoinEui));
                //PRINTF( "LoRa:AppKey= %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\r\n", HEX16(AppKey));
                PRINTF( "=======================================================\r\n" );
                JoinNetwork( );
            } else {
                LoRaSetABPParam();
                PRINTF( "LoRa:###### ===== JOINED ==== ######\r\n" );
                PRINTF( "\r\nABP\r\n\r\n" );
                PRINTF( "DevAddr     : %08lX\r\n", DevAddr );
                PRINTF( "NwkSKey     : %02X", FNwkSIntKey[0] );
                for ( int i = 1; i < 16; i++ ) {
                    PRINTF( " %02X", FNwkSIntKey[i] );
                }
                PRINTF( "\r\n" );
                PRINTF( "AppSKey     : %02X", AppSKey[0] );
                for ( int i = 1; i < 16; i++ ) {
                    PRINTF( " %02X", AppSKey[i] );
                }
                PRINTF( "\n\r\n" );

                // Tell the MAC layer which network server version are we connecting too.
                mibReq.Type = MIB_ABP_LORAWAN_VERSION;
                mibReq.Param.AbpLrWanVersion.Value = ABP_ACTIVATION_LRWAN_VERSION;
                LoRaMacMibSetRequestConfirm( &mibReq );

                mibReq.Type = MIB_NETWORK_ACTIVATION;
                mibReq.Param.NetworkActivation = ACTIVATION_TYPE_ABP;
                LoRaMacMibSetRequestConfirm( &mibReq );

                LORA_JoinResult(true);
            }
            break;
        }

        case DEVICE_STATE_REJOIN: {
            PRINTF( "LoRa:start REJOIN\n\r");
            NextTx = true;
            TimerStop( &TxNextPacketTimer );
            JoinNetwork( );

            break;
        }

        case DEVICE_STATE_REQ_DEVICE_TIME: {
            MlmeReq_t mlmeReq;

            if ( NextTx == true ) {
                mlmeReq.Type = MLME_DEVICE_TIME;

                if ( LoRaMacMlmeRequest( &mlmeReq ) == LORAMAC_STATUS_OK ) {
                    PRINTF( "LoRa:Send MLME_DEVICE_TIME_Req\n\r\n" );
                    WakeUpState = DEVICE_STATE_SEND;
                }
            }
            DeviceState = DEVICE_STATE_SEND;
            break;
        }
        case DEVICE_STATE_REQ_BEACON_TIMING: {
            MlmeReq_t mlmeReq;

            if ( NextTx == true ) {
                mlmeReq.Type = MLME_BEACON_TIMING;

                if ( LoRaMacMlmeRequest( &mlmeReq ) == LORAMAC_STATUS_OK ) {
                    WakeUpState = DEVICE_STATE_SEND;
                }
            }
            DeviceState = DEVICE_STATE_SEND;
            break;
        }
        case DEVICE_STATE_BEACON_ACQUISITION: {
            MlmeReq_t mlmeReq;

            if ( NextTx == true ) {
                mlmeReq.Type = MLME_BEACON_ACQUISITION;

                LoRaMacMlmeRequest( &mlmeReq );
                NextTx = false;
                PRINTF( "LoRa:Send MLME_BEACON_ACQUISITION_Req\n\r\n" );
            }
            //DeviceState = DEVICE_STATE_SEND;
            //DeviceState = DEVICE_STATE_CYCLE;
            DeviceState = DEVICE_STATE_SLEEP; //TODO:tgl
            break;
        }
        case DEVICE_STATE_REQ_PINGSLOT_ACK: {
            MlmeReq_t mlmeReq;

            if ( NextTx == true ) {
                mlmeReq.Type = MLME_LINK_CHECK;
                LoRaMacMlmeRequest( &mlmeReq );

                mlmeReq.Type = MLME_PING_SLOT_INFO;
                mlmeReq.Req.PingSlotInfo.PingSlot.Fields.Periodicity = g_lora_pingslot_period;
                mlmeReq.Req.PingSlotInfo.PingSlot.Fields.RFU = 0;

                if ( LoRaMacMlmeRequest( &mlmeReq ) == LORAMAC_STATUS_OK ) {
                    WakeUpState = DEVICE_STATE_SEND;
                }
                PRINTF( "LoRa:Send MLME_PING_SLOT_INFO_Req\n\r\n" );
            }
            DeviceState = DEVICE_STATE_SEND;
            break;
        }
        case DEVICE_STATE_SEND: {
            //In ClassA Repeat Mode, need report DevAddr to repeater after Join ok;
            if (true == LORA_RepeaterModeReportDevAddrIsNeed()) {
                LORA_RepeaterModeReportDevAddr();
            } else if ( NextTx == true ) {
                PrepareTxFrame(g_lora_port);
                //NextTx = SendFrame( );
                lorawan_control_send(g_lora_is_confirmup, g_lora_port, (char *)g_lora_app_data_buffer, g_lora_app_data_size, NULL);
            }
            DeviceState = DEVICE_STATE_CYCLE;
            break;
        }
        case DEVICE_STATE_CYCLE: {
            int dutytime = 180 * NUM_MS_PER_SECOND; //180s=3minutes
            int nextNeedTx = false;
            WakeUpState = DEVICE_STATE_SEND;
            DeviceState = DEVICE_STATE_SLEEP;
            if (  lorawan_certif_running() == true ) {
                // Schedule next packet transmission
                TxDutyCycleTime = lorawan_certif_uplink_cycle() * 1000; // default 5000 ms
                nextNeedTx = true;
                g_lora_port = CERTIF_PORT;
            } else {
                // Schedule next packet transmission
                if (true == g_lora_class_switch_succes_flag) { //ClassA/C or Enter ClassB
                    if ((2 >= g_lora_num_received_data_ack) && (g_lora_current_class == CLASS_B)) {
                        //after enter classB, make sure node receive PingSlotChannelReq from NS to change DR.
                        dutytime = 15 * 1000; //10s
                        nextNeedTx = true;
                        g_lora_port    = LORAWAN_CONTROLLER_PORT;
                    } else {
                        //In ClassA Repeat Mode, need report DevEUI to repeater after Join ok;
                        if (true == LORA_RepeaterModeReportDevAddrIsNeed()) {
                            g_cntSendedDevAddrToRepeater++;
                            dutytime = 6000; //6s;
                            nextNeedTx = true;
                        } else if (g_lora_app_tx_report_mode) {
                            //after class switch succesfully, heatbeat will be send by lora module;
                            dutytime = g_lora_app_tx_dutycycle;
                            nextNeedTx = true;

                            if (g_monitor_node_feature_open) {
                                g_lora_port    = LORA_MONITOR_NODE_CMD_PORT;
                            } else {
                                g_lora_port    = g_lora_app_port;
                            }
                        }
                    }
                } else { //ClassB:before switch succesfully, MAC Cmd need be send to NS by lora module;
                    dutytime = 180 * NUM_MS_PER_SECOND; //180s=3minutes
                    nextNeedTx = true;
                    g_lora_port    = LORAWAN_CONTROLLER_PORT;
                }

                TxDutyCycleTime = dutytime + randr( -APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND );
            }

            if (nextNeedTx == true) {
                // Schedule next packet transmission
                TimerSetValue( &TxNextPacketTimer, TxDutyCycleTime );
                TimerStart( &TxNextPacketTimer );
            }
            break;
        }
        case DEVICE_STATE_SLEEP: {

#if 0
            DISABLE_IRQ( );
            /* if an interrupt has occurred after DISABLE_IRQ, it is kept pending
             * and cortex will not enter low power anyway  */
#ifndef LOW_POWER_DISABLE
            LPM_EnterLowPower( );
#endif

            ENABLE_IRQ();
#endif
            /* USER CODE BEGIN 2 */
            /* USER CODE END 2 */

            break;
        }
        default: {
            DeviceState = DEVICE_STATE_START;
            break;
        }
    }

    return DeviceState;

}

static void LORA_InitFinished(void)
{

}

static void LORA_SwitchClassResult(int successful) //only for switch to classB
{

    //support only classA->classB or classA->classC
    if (g_lora_current_class == CLASS_A) {
        TimerStop( &ClassSwitchRespTimeoutTimer);

        if (successful) {
            LORA_ConnectedProcess();
            TimerStart(&RadioStatusTimer);
            g_lora_control_current_status = LoRa_Class_SwitchOK;
            LORA_AT_PRINTF("+CCLASS:OK\r\n");

            //call CB to notify App
            if (g_lora_esl_connection_ops_cb->lora_classbswitch_result) {
                g_lora_esl_connection_ops_cb->lora_classbswitch_result(true, g_lora_cm_data);
            }

            g_lora_current_class = CLASS_B;
            PRINTF( "=======================================================\r\n" );
            PRINTF("LoRa:Switch ClassB successful, Send data to inform NS.\r\n");
            PRINTF( "=======================================================\r\n" );

        } else {
            PRINTF("LoRa: Switch ClassB failed,NumFailed=%d.\r\n", g_num_of_class_switch_failed);
            g_num_of_class_switch_failed++;
            g_lora_control_current_status = LoRa_Class_SwitchNOK;

            if (g_num_of_class_switch_failed < LORA_NODE_NUM_CLASS_SWITCH_RETRY_CNT) {

                /*1,set next class switch slot*/
                ClassSwitchBackoffTimerInterval = (1 << g_num_of_class_switch_failed) * NUM_MS_PER_SECOND * NUM_SECOND_PER_MINUTE +
                                                  g_random_joindelay;//unit =ms; 1000*60s

                if (LORA_NODE_DISABLE_CLASS_SWITCH_BACKOFF) {
                    ClassSwitchBackoffTimerInterval = 30 * 1000;    //30 seconds
                }

                if (ClassSwitchBackoffTimerInterval > (NUM_MS_PER_SECOND * NUM_SECOND_PER_MINUTE * NUM_MINUTE_PER_HOUR / 2)) {
                    ClassSwitchBackoffTimerInterval = NUM_MS_PER_SECOND * NUM_SECOND_PER_MINUTE * NUM_MINUTE_PER_HOUR / 2;
                }

                TimerSetValue( &ClassSwitchBackoffTimer, ClassSwitchBackoffTimerInterval);
                TimerStart( &ClassSwitchBackoffTimer);

                TimerStop( &TxNextPacketTimer );

                PRINTF("LoRa:Switch ClassB failed,Next switch Delay Seconds =%d,NumofSwitchFailed=%d.\r\n",
                       ClassSwitchBackoffTimerInterval / NUM_MS_PER_SECOND, g_num_of_class_switch_failed);

            } else {
                PRINTF("LoRa: Class Switch failed times is more than defined.\r\n");
                g_num_of_class_switch_failed = 0;
                LORA_AT_PRINTF("+CCLASS:FAIL\r\n");
            }
        }
    } else {
        PRINTF("LoRa: Class Switch happend unexpected, start reJoin.\r\n");
        LORA_ReJoin();
    }
}

static void LORA_ClassBLostSync(void)
{
    PRINTF("LoRa: ClassB Lost Sync(lost many Beacons), start reJoin.\r\n");
    LORA_ReJoin();
}

static void LORA_SendDataOkWithoutRx(void)
{
    PRINTF("LoRa: SendDataOkWithoutRx.\r\n");
    if ((true == g_lora_class_switch_succes_flag)) {
        g_lora_control_current_status = LoRa_Idle;
    }

    g_num_send_data_failed_to_rejoin = 0;
    g_lora_num_received_data_ack++;
#if 0 //Test code for class switch
    if (g_lora_current_class == CLASS_B) {
        lorawan_control_switch_class(CLASS_A);
    }

    if ((g_lora_current_class == CLASS_A) && (g_lora_num_received_data_ack == 4)) {
        lorawan_control_switch_class(CLASS_C);
    }

    if ((g_lora_current_class == CLASS_C) && (g_lora_num_received_data_ack == 6)) {
        lorawan_control_switch_class(CLASS_A);
    }

    if ((g_lora_current_class == CLASS_A) && (g_lora_num_received_data_ack == 8)) {
        lorawan_control_switch_class(CLASS_B);
    }
#endif

}


static void LORA_SendDataFailed(void)
{
    int num2reint = LORA_NODE_NUM_REJOIN_FAILED_TO_REINIT;
    PRINTF("LoRa: SendDataFailed.\r\n");
    if (true == g_lora_class_switch_succes_flag) {
        g_lora_control_current_status = LoRa_Idle;
    }

    if ((0 == g_lora_num_received_data_ack) && (true == g_lora_class_switch_succes_flag)) {
        //osDelay(1000);
        //SendHeartBeat();
        NextTx = true;
        LORA_SetDeviceState(DEVICE_STATE_SEND);
    } else {
        num2reint = LORA_NODE_NUM_SEND_FAILED_TO_REJOIN;
    }

    g_num_send_data_failed_to_rejoin++;

    if (g_num_send_data_failed_to_rejoin >= num2reint) {
        g_num_send_data_failed_to_rejoin = 0;

        //rejoin
        PRINTF("LoRa: SendDataFailed times is more than defined, start reJoin.\r\n");
        LORA_ReJoin();
    }
}

static void LORA_SetRssiSnr(int rssi, int snr)
{
    g_lora_rx_rssi = rssi;
    g_lora_rx_snr = snr;
}

int LORA_SetRepeaterSupport(int state)
{
    LoRaMacStatus_t     status;
    MibRequestConfirm_t mib_req;
    int                ret = -1;

    g_cntSendedDevAddrToRepeater = 0;

    mib_req.Type = MIB_REPEATER_SUPPORT;
    if (state == false) {
        mib_req.Param.EnableRepeaterSupport = false;
    } else {
        mib_req.Param.EnableRepeaterSupport = true;
    }

    status       = LoRaMacMibSetRequestConfirm(&mib_req);
    if (status == LORAMAC_STATUS_OK) {
        ret = 0;
    }
    return ret;
}

static void LORA_UpdateChannelsMask(uint8_t tx_freq)
{
#if (ACTIVE_REGION == LORAMAC_REGION_CN470)
    /* 1A1, 1A2, 2A1, 2A2, 3B1, 3B2, 4B1, 4B2 */
    uint8_t channelsMask[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    uint8_t mask_idx;
    MibRequestConfirm_t mibReq;

    mask_idx = tx_freq;
    /* 3B1: txfreq = 12, mask_idx = 4 */
    if (mask_idx >= 12) {
        mask_idx -= 8;
    }

    channelsMask[mask_idx] = 0xFF;

    mibReq.Type = MIB_CHANNELS_MASK;
    mibReq.Param.ChannelsMask = (uint16_t *) channelsMask;
    LoRaMacMibSetRequestConfirm( &mibReq );

    mibReq.Type = MIB_CHANNELS_DEFAULT_MASK;
    mibReq.Param.ChannelsMask = (uint16_t *) channelsMask;
    LoRaMacMibSetRequestConfirm( &mibReq );
#else
#error "region not support!"
#endif
}

static void LORA_UpdateJoinScanMask(void)
{
    g_join_scan_left_band_mask = g_lora_freqband_mask;
}

static int LORA_FindNextAvailableFreqband(uint8_t *avail_freqband)
{
    uint8_t freqband;

    if (g_join_scan_left_band_mask == 0) {
        return -1;
    }

    freqband = randr(0, 15);
    while ((g_join_scan_left_band_mask & (1 << freqband)) == 0) {
        freqband = ((freqband + 1) % 15);
    }

    g_join_scan_left_band_mask &= (~(1 << freqband));
    *avail_freqband = freqband;

    return 0;
}

static void LORA_ResetJoinParam(bool joined)
{
    if (g_store_freqband >= 0 && g_num_of_scan_round_join_failed != LORA_NODE_SCAN_JOIN_RETRY_CNT) {
        g_join_method = STORED_JOIN_METHOD;
    } else {
        g_join_method = DEF_JOIN_METHOD;
    }

    if (joined) {
        g_num_of_join_failed = 0;
    }

    /* for first time or after join success, DO NOT reset g_join_datarate */
    if (g_num_of_join_failed != 0) {
        g_join_datarate = g_default_datarate;
    }

    LORA_UpdateJoinScanMask();
    g_num_of_store_join_failed = 0;
    g_num_of_def_join_failed = 0;
    g_num_of_scan_join_failed = 0;
    g_num_of_scan_round_join_failed = 0;
    TimerStop( &JoinBackoffTimer);
}

static void LORA_PrepJoinParam(void)
{
    int join_interval;
    uint8_t freqband;
    int ret;
    join_method_t cur_join_method = g_join_method;
    bool is_try_join_after_one_hour = false;

    switch (g_join_method) {
        case STORED_JOIN_METHOD: {
            if (g_num_of_store_join_failed != 0) {
                g_join_datarate = LORAWAN_LOWEST_DATARATE;
            }

            if (g_num_of_store_join_failed < LORA_NODE_STROED_JOIN_RETRY_CNT) {
                g_join_freqband = g_store_freqband;
                LORA_UpdateChannelsMask(g_store_freqband);
                g_num_of_store_join_failed++;
                break;
            } else {
                g_join_method = DEF_JOIN_METHOD;
                __attribute__ ((fallthrough));
            }
        }
        case DEF_JOIN_METHOD: {
            if (g_num_of_def_join_failed == 0) {
                g_join_datarate = g_default_datarate;
            } else {
                g_join_datarate = LORAWAN_LOWEST_DATARATE;
            }

            if (g_num_of_def_join_failed < LORA_NODE_STROED_JOIN_RETRY_CNT) {
                g_join_freqband = LORA_DEF_FREQBAND;
                LORA_UpdateChannelsMask(LORA_DEF_FREQBAND);
                g_num_of_def_join_failed++;
                break;
            } else {
                g_join_method = SCAN_JOIN_METHOD;
                __attribute__ ((fallthrough));
            }
        }
        case SCAN_JOIN_METHOD: {
            while (g_num_of_scan_round_join_failed < LORA_NODE_SCAN_JOIN_RETRY_CNT) {
                if (g_num_of_scan_join_failed % 2 == 0) {
                    g_join_datarate = g_default_datarate;
                    ret = LORA_FindNextAvailableFreqband(&freqband);
                    if (ret == -1) {
                        LORA_UpdateJoinScanMask();
                        g_num_of_scan_round_join_failed++;
                    } else {
                        g_join_freqband = freqband;
                        LORA_UpdateChannelsMask(g_join_freqband);
                        g_num_of_scan_join_failed++;
                        break;
                    }
                } else {
                    g_join_datarate = LORAWAN_LOWEST_DATARATE;
                    LORA_UpdateChannelsMask(g_join_freqband);
                    g_num_of_scan_join_failed++;
                    break;
                }
            }

            if (g_num_of_scan_round_join_failed == LORA_NODE_SCAN_JOIN_RETRY_CNT) {
                LORA_ResetJoinParam(false);

                //check node current work mode
                if (g_lora_is_work_mode_autoswitch == true) {
                    if (g_lora_current_work_mode == NODE_WORK_MODE_REPEATER) {
                        is_try_join_after_one_hour = true;
                        LORA_SetRepeaterSupport(false); //set NodeWorkMode_Normal
                        g_lora_current_work_mode = NODE_WORK_MODE_NORMAL;
                    } else {
                        TimerStart( &JoinBackoffTimer);
                        is_try_join_after_one_hour = false;
                        LORA_SetRepeaterSupport(true); //set NodeWorkMode_repeater
                        g_lora_current_work_mode = NODE_WORK_MODE_REPEATER;
                        PRINTF("LoRa:Join fail in NodeNormalWorkMode, Switch to Repeat Mode!\r\n");
                    }
                } else {
                    is_try_join_after_one_hour = true;
                }

                //check if stop one hour
                if (is_try_join_after_one_hour == true) {
                    if (g_lora_auto_join) {
                        join_interval = NUM_MS_PER_SECOND * NUM_SECOND_PER_MINUTE * NUM_MINUTE_PER_HOUR;
                        TimerSetValue(&JoinBackoffTimer, join_interval);
                        TimerStart(&JoinBackoffTimer);
                        PRINTF("LoRa: join fail, wait for one hour!\r\n");
                    } else {
                        TimerStop(&JoinBackoffTimer);
                    }

                    LORA_AT_PRINTF("+CJOIN:FAIL\r\n");
                }
            }
        }
        default:
            break;
    }

    PRINTF("LoRa: current join method = %s, next join method = %s\r\n",
           JoinMethodStrings[cur_join_method],
           JoinMethodStrings[g_join_method]);

    return;
}

static void LORA_JoinResult( int successful )
{
    if (!successful) {
        PRINTF("\r\nLoRa: JoinResult failed, NumJoin=%d, try another FreqGroup.\r\n", g_num_of_join_failed);
        g_lora_control_current_status = LoRa_Join_Failed;

        g_num_of_join_failed++;
        LORA_PrepJoinParam();

        // wait the join backoff timer timeout, it has been started in JoinNetwork()
    } else {
        PRINTF("\r\n===============================================\r\n");
        PRINTF("\r\nLoRa: JoinResult is Successful,current Class=%d.\r\n", g_lora_current_class);
        PRINTF("\r\n===============================================\r\n");
        g_lora_control_current_status = LoRa_Join_Success;

        g_default_datarate = g_join_datarate;

        g_store_freqband = g_join_freqband;
#if LORA_MODULE_GET_FLASH_EUI_ENABLE
        lora_module_dev_stat_set_freqband_dr(g_store_freqband, g_join_datarate);
#endif
        LORA_ResetJoinParam(true);

        if ((CLASS_B == g_current_target_lora_class_type)) {
            lorawan_control_switch_class(g_current_target_lora_class_type);
        } else {
            if (!g_lora_otaa_mode || g_current_target_lora_class_type == CLASS_C) {
                lorawan_control_switch_class(g_current_target_lora_class_type);
            }
            LORA_ConnectedProcess();
        }

        LORA_AT_PRINTF("+CJOIN:OK\r\n");
    }
}

static int LORA_SwitchClass(DeviceClass_t newClass)
{
    LoraErrorStatus Errorstatus = LORA_SUCCESS;
    MibRequestConfirm_t mibReq;
    DeviceClass_t currentClass;

    mibReq.Type = MIB_DEVICE_CLASS;
    LoRaMacMibGetRequestConfirm( &mibReq );

    currentClass = mibReq.Param.Class;
    /*attempt to swicth only if class update*/
    if (currentClass != newClass) {
        switch (newClass) {
            case CLASS_A: {
                mibReq.Param.Class = CLASS_A;
                if ( LoRaMacMibSetRequestConfirm( &mibReq ) == LORAMAC_STATUS_OK ) {
                    /*switch is instantanuous*/
                    TimerStop(&RadioStatusTimer);
                } else {
                    Errorstatus = LORA_ERROR;
                }
                break;
            }
            case CLASS_B: {
                if (currentClass != CLASS_A) {
                    Errorstatus = LORA_ERROR;
                } else {
                    WakeUpState = DEVICE_STATE_REQ_DEVICE_TIME;
                    LORA_SetDeviceState(DEVICE_STATE_REQ_DEVICE_TIME);
                }
                break;
            }
            case CLASS_C: {
                if (currentClass != CLASS_A) {
                    Errorstatus = LORA_ERROR;
                }
                /*switch is instantanuous*/
                mibReq.Param.Class = CLASS_C;
                if ( LoRaMacMibSetRequestConfirm( &mibReq ) == LORAMAC_STATUS_OK ) {

                } else {
                    Errorstatus = LORA_ERROR;
                }

                //make a send to trigger classC Rx2 work
                NextTx = true;
                LORA_SetDeviceState(DEVICE_STATE_SEND);

                break;
            }
            default:
                break;
        }
    }
    return Errorstatus;
}

static void LORA_RxData(Mcps_t type, uint8_t *Buff, uint8_t BuffSize, uint8_t Port)
{
    int ret;

    /* USER CODE BEGIN 4 */
    if ((true == g_lora_class_switch_succes_flag)) {
        g_lora_control_current_status = LoRa_Idle;
    }

    PRINTF("LoRa: Received Data, type=%d port=%d,len=%d,first data=0x%02x 0x%02x 0x%02x 0x%02x\r\n",
           type,
           Port,
           BuffSize,
           Buff[0],
           Buff[1],
           Buff[2],
           Buff[3]);

    g_num_send_data_failed_to_rejoin = 0;
    g_lora_num_received_data_ack++;

    memcpy(g_lora_app_rx_data_buffer, Buff, BuffSize);
    g_lora_app_rx_data_size = BuffSize;

    //1,parse Application's package protocol.
    switch (Port) {
        case LORAWAN_APP_PORT: {
            if (g_lora_esl_connection_ops_cb->data_arrived) {
                if (type == MCPS_MULTICAST) {
                    g_lora_esl_connection_ops_cb->data_arrived(ESL_MULTICAST_DATA, Port, Buff, BuffSize, g_lora_cm_data);
                } else {
                    g_lora_esl_connection_ops_cb->data_arrived(ESL_UNICAST_DATA, Port, Buff, BuffSize, g_lora_cm_data);
                }
            }
            break;
        }
        case LORA_MULTICAST_CMD_PORT: {
            PRINTF("Dynamic multicast commands\r\n");
            lorawan_mc_process_cmd(Buff, BuffSize);
            lorawan_mc_cmd_ans();
            break;
        }

        case LORA_MONITOR_NODE_CMD_PORT: {
            PRINTF("monitor node received dl commands\r\n");
            LORA_MonitorNodeProcessCmd(Buff, BuffSize);
            break;
        }

        default: {
            rx_cb_t app_rx_cb = NULL;
            ret = lorawan_module_get_app_rx_cb(Port, &app_rx_cb);
            if (ret != 0) {
                PRINTF("LoRa: lorawan_module_get_app_rx_cb ret=%d\r\n", ret);
                return;
            }

            if (app_rx_cb == NULL) {
                PRINTF("LoRa: Port = %d not support!\r\n", Port);
            } else {
                app_rx_cb(Buff, BuffSize);
            }
            break;
        }
    }
}

static int LORA_SetDeviceState ( DeviceState_t state)
{
    LoraErrorStatus Errorstatus = LORA_SUCCESS;

    if (state == DEVICE_STATE_SEND) {
        if (g_lora_control_current_status == LoRa_Class_Switching || g_lora_control_current_status == LoRa_Joining) {
            Errorstatus = LORA_ERROR;
        } else {
            DeviceState = state;
        }
    } else if (state == DEVICE_STATE_REJOIN ) {
        if (g_lora_control_current_status == LoRa_Joining) {
            Errorstatus = LORA_ERROR;
        } else {
            DeviceState = state;
        }
    } else {
        DeviceState = state;
    }

    if (Errorstatus == LORA_ERROR) {
        PRINTF("\r\nLoRa: LORA_SetDeviceState() Error, target state=%d, current state=%d.\n\r", state,
               g_lora_control_current_status);
    }

    return Errorstatus;
}

static void LORA_MonitorNodeProcessCmd(uint8_t *Buff, uint8_t BuffSize)
{
    uint8_t reportInterval = 0;//minute

    g_monitor_node_dl_cmd_status = MonitorNode_DlCmdExecuteFail;

    //parse monitor node's DL Protocol
    if ((Buff[0] == 2) && (Buff[1] == 4) && (Buff[2] == 1)) {
        reportInterval = Buff[3];
    }

    if (reportInterval != 0) {
        g_lora_app_tx_dutycycle = 1000 * 60 * reportInterval;
        g_monitor_node_dl_cmd_status = MonitorNode_DlCmdExecuteOK;

        //reset report interval
        TimerStop( &TxNextPacketTimer );
        TimerSetValue( &TxNextPacketTimer, g_lora_app_tx_dutycycle );
        TimerStart( &TxNextPacketTimer );
        PRINTF( "lora:MonitorNodeProcessCmd,change Report Dutycycle to %ld ms \r\n", g_lora_app_tx_dutycycle );
    }
}

int lorawan_control_init(esl_connection_ops_cb_t *_esl_cm_ops_cb, void *data)
{
    g_lora_esl_connection_ops_cb = _esl_cm_ops_cb;
    g_lora_cm_data = data;
    //assert_param(g_lora_esl_connection_ops_cb);

#if LORA_MODULE_GET_FLASH_EUI_ENABLE
    lora_module_dev_stat_init();
    lora_module_dev_stat_get_devEUI(DevEui, sizeof(DevEui));
    lora_module_dev_stat_get_appEUI(JoinEui, sizeof(JoinEui));
    lora_module_dev_stat_get_appKey(AppKey, sizeof(AppKey));
    lora_module_dev_stat_get_appKey(NwkKey, sizeof(NwkKey));
    lora_module_dev_stat_get_freqband_dr(&g_store_freqband, &g_join_datarate);
    lora_module_dev_stat_get_freqmode(&g_freq_mode);
    lora_module_dev_stat_get_freqband_mask(&g_lora_freqband_mask);
    lora_module_dev_stat_get_class(&g_current_target_lora_class_type);

    PRINTF( "=======================================================\r\n" );
    PRINTF( "LoRa:get DevEui= %02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X\r\n", HEX8(DevEui));
    PRINTF( "LoRa:get AppEui= %02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X\r\n", HEX8(JoinEui));
    PRINTF( "LoRa:get AppKey= %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\r\n", HEX16(AppKey));
    PRINTF( "LoRa:get store freqband= %d, datarate= %d\r\n", g_store_freqband, g_join_datarate);
    PRINTF( "=======================================================\r\n" );
#endif

    LORA_ResetParameters();

    /* Configure and Init the LoRaWAN Stack*/
    LoRaMacPrimitives.MacMcpsConfirm = McpsConfirm;
    LoRaMacPrimitives.MacMcpsIndication = McpsIndication;
    LoRaMacPrimitives.MacMlmeConfirm = MlmeConfirm;
    LoRaMacPrimitives.MacMlmeIndication = MlmeIndication;
    LoRaMacCallbacks.GetBatteryLevel = NULL;
    LoRaMacCallbacks.GetTemperatureLevel = NULL;
    LoRaMacCallbacks.NvmContextChange = NULL;

    LoRaMacInitialization( &LoRaMacPrimitives, &LoRaMacCallbacks, ACTIVE_REGION );

    DeviceState = DEVICE_STATE_RESTORE;
    WakeUpState = DEVICE_STATE_START;
    g_random_joindelay = randr( 0, 2 * APP_TX_DUTYCYCLE_RND );

    TimerInit( &JoinBackoffTimer, OnJoinBackoffTimerEvent );
    TimerInit( &ClassSwitchBackoffTimer, OnStartNextClassSwitchTimerEvent);

    TimerInit( &ClassSwitchRespTimeoutTimer, OnClassSwitchRespTimeoutTimerEvent);
    TimerSetValue( &ClassSwitchRespTimeoutTimer, TIMER_SWITCH_CLASSB_RESULT_CHECK);

    TimerInit(&RadioStatusTimer, OnRadioStatusEvent);
    TimerSetValue(&RadioStatusTimer, TIMER_RADIO_STATUS_CHECK);

    g_lora_control_current_status = LoRa_Init_Done;

    lorawan_control_mc_setup();

    PRINTF("LoRa:LoRa_Init_Done,LoRaWAN Protocol MAC VERSION: %lX\r\n", VERSION);

    return 0;
}

int lorawan_control_deinit(void)
{
    g_lora_esl_connection_ops_cb = NULL;
    g_lora_cm_data = NULL;
    g_lora_control_current_status = LoRa_Initing;

    return 0;
}

int lorawan_control_send(int is_confirm, int app_port, const char *data, int len, handle_send_cb_fp_t cb)
{
    if (g_auto_linkcheck) {
        MlmeReq_t mlmeReq;
        mlmeReq.Type = MLME_LINK_CHECK;
        LoRaMacMlmeRequest(&mlmeReq);
    }

    if ( lorawan_control_get_join_status () != 1) {
        /*Not joined, try again later*/
        JoinNetwork();
        return ERRNO_NOT_JOIN;
    }

    if (len > LORAWAN_APP_DATA_MAX_SIZE) {
        return ERRNO_DATA_EXCEED;
    }

    //if FCntUp Rollback, need to join
    uint32_t fCntUp = 0;
    LoRaMacGetFCntUp( &fCntUp );
    if ( fCntUp >= LORAWAN_FCNT_MAX) {
        //rejoin
        PRINTF("LoRa: FCntUp Rollback happen, start reJoin.\r\n");
        LORA_ReJoin();

        return ERRNO_SEND_FAIL;
    }

    g_lora_port = app_port;
    g_lora_app_data_size = len;
    memcpy1(g_lora_app_data_buffer, (uint8_t *)data, len);
    g_lora_is_confirmup = is_confirm;

    if (false == SendFrame()) {
        g_lora_num_send_uplink++;
        return 0;
    } else {
        g_lora_control_current_status = LoRa_Send_Failed;
        return ERRNO_SEND_FAIL;
    }
}

LoRaModuleStatus_t lorawan_control_switch_class(DeviceClass_t newClass)
{
    int switchStatus;
    if ((g_lora_control_current_status == LoRa_Joining) ||
        (g_lora_control_current_status == LoRa_Sending) ||
        (g_lora_control_current_status == LoRa_Class_Switching) ||
        (g_lora_control_current_status == LoRa_Initing)) {
        PRINTF("LoRa:failed to start lorawan_control_switch_class(),because g_lora_control_current_status=%d.\r\n",
               g_lora_control_current_status);
        return LoRa_Class_SwitchNOK;
    }

    PRINTF("LoRa:lorawan_control_switch_class(),TargetClassType=%d,CurrentClass=%d,g_lora_control_current_status=%d.\r\n",
           newClass, g_lora_current_class, g_lora_control_current_status);

    if ((g_lora_current_class != newClass)) {
        if (newClass == CLASS_B) {
            switchStatus = LORA_SwitchClass(newClass);

            if (switchStatus == LORA_SUCCESS) {
                TimerStart( &ClassSwitchRespTimeoutTimer);
                g_lora_control_current_status = LoRa_Class_Switching;
                g_lora_class_switch_succes_flag = false;

                PRINTF("LoRa switching to ClassB from ClassA,waiting 2 minutes...\r\n");
                return LoRa_Class_Switching;
            }
        } else if ((newClass == CLASS_C) || (newClass == CLASS_A)) {
            switchStatus = LORA_SwitchClass(newClass);

            PRINTF("LoRa switching to ClassC or ClassA, switch finished,switchStatus=%d.\r\n", switchStatus);

            if (switchStatus == LORA_SUCCESS) {
                if (newClass == CLASS_A) {
                    g_lora_control_current_status = LoRa_Idle;
                } else {
                    g_lora_control_current_status = LoRa_ClassC_Listen;
                }
                g_lora_class_switch_succes_flag = true;
                g_lora_current_class = newClass;
                return LoRa_Class_SwitchOK;
            }
        }
    } else {
        return LoRa_Class_SwitchOK;
    }

    return LoRa_Class_SwitchNOK;
}

static int lorawan_module_reg_app_rx_cb(uint8_t port, rx_cb_t rx_cb)
{
    int i;

    if (rx_cb == NULL) {
        PRINTF("LoRa: app_rx_cb is NULL!\r\n");
        return -1;
    }

    if (port == 0) {
        PRINTF("LoRa: port is 0!\r\n");
        return -1;
    }

    for (i = 0; i < MAX_APP_RX_CB_LIST; i++) {
        if (g_app_rx_cb_list[i].port == 0) {
            g_app_rx_cb_list[i].port = port;
            g_app_rx_cb_list[i].rx_cb = rx_cb;
            break;
        }
    }

    if (i == MAX_APP_RX_CB_LIST) {
        PRINTF("LoRa: no enough cb list space left!\r\n");
        return -1;
    }

    return 0;
}

static int lorawan_module_get_app_rx_cb(uint8_t port, rx_cb_t *rx_cb)
{
    int i;

    for (i = 0; i < MAX_APP_RX_CB_LIST; i++) {
        if (g_app_rx_cb_list[i].port == port) {
            *rx_cb = g_app_rx_cb_list[i].rx_cb;
            return 0;
        }
    }

    PRINTF("LoRa: no app rx cb found!\r\n");
    return -1;
}

int lorawan_control_mc_setup(void)
{
    uint8_t mcID;

    lorawan_mc_init(LORA_MULTICAST_CMD_PORT, mcGenAppKey);

    for (mcID = 0; mcID < LORAMAC_MAX_MC_CTX; mcID++) {
        lorawan_mc_set_preset_channel(mcID, &g_mc_channels[mcID], &g_mc_keys[mcID]);
    }

    return 0;
}

int lorawan_control_join(int mode)
{
    if (mode == 1) {
        LORA_ReJoin();
        return 0;
    } else if (mode == 0) {
        if (g_lora_control_current_status != LoRa_Joining) {
            return -1;
        }

        LORA_ResetJoinParam(false);
        LORA_SetDeviceState(DEVICE_STATE_SLEEP);
        return 0;
    } else {
        return -1;
    }

}

NodeWorkMode_t lorawan_control_get_work_mode(void)
{
    if (g_lora_is_work_mode_autoswitch == true) {
        return NODE_WORK_MODE_AUTOSWITCH;
    } else {
        return g_lora_current_work_mode;
    }
}

int lorawan_control_set_work_mode(NodeWorkMode_t mode)
{
    int     status;

    if ((mode != NODE_WORK_MODE_NORMAL && mode != NODE_WORK_MODE_REPEATER && mode != NODE_WORK_MODE_AUTOSWITCH)) {
        return -1;
    }

    //monitor not support repeater mode
    if (g_monitor_node_feature_open) {
        if (mode != NODE_WORK_MODE_NORMAL) {
            return -1;
        }
    }

    if (NODE_WORK_MODE_AUTOSWITCH == mode) {
        g_lora_is_work_mode_autoswitch = true;
        g_lora_current_work_mode = NODE_WORK_MODE_NORMAL;
    } else {
        g_lora_is_work_mode_autoswitch = false;
        g_lora_current_work_mode = mode;
    }

    //update repeater support status
    if (NODE_WORK_MODE_REPEATER == g_lora_current_work_mode) {
        status = LORA_SetRepeaterSupport(true);
    } else {
        status = LORA_SetRepeaterSupport(false);
    }

    return status;
}

int lorawan_control_get_join_status(void)
{
    MibRequestConfirm_t mibReq;

    mibReq.Type = MIB_NETWORK_ACTIVATION;
    LoRaMacMibGetRequestConfirm( &mibReq );

    if ( mibReq.Param.NetworkActivation == ACTIVATION_TYPE_NONE ) {
        /*Not joined*/
        return 0;
    } else {
        return 1;
    }
}

void lorawan_control_trigger_send( void )
{
    DeviceState = DEVICE_STATE_SEND;
    WakeUpState = DEVICE_STATE_SEND;
    OnTxNextPacketTimerEvent();
}

LoRaNodeFreqMode_t lorawan_control_get_freq_mode(void)
{
    return g_freq_mode;
}

int lorawan_control_set_freq_mode(LoRaNodeFreqMode_t mode)
{
    LoRaMacStatus_t     status;
    MibRequestConfirm_t mib_req;

    if ((mode != FREQ_MODE_INTRA && mode != FREQ_MODE_INTER)) {
        return -1;
    }

    mib_req.Type = MIB_FREQ_MODE;
    mib_req.Param.FreqMode = mode;

    status = LoRaMacMibSetRequestConfirm(&mib_req);
    if (status != LORAMAC_STATUS_OK) {
        return -1;
    }

    g_freq_mode = mode;
    return 0;
}

JoinMode_t lorawan_control_get_join_mode(void)
{
    if (g_lora_otaa_mode) {
        return JOIN_MODE_OTAA;
    } else {
        return JOIN_MODE_ABP;
    }
}

int lorawan_control_set_join_mode(JoinMode_t mode)
{
    if (mode == JOIN_MODE_OTAA) {
        g_lora_otaa_mode = true;
    } else {
        g_lora_otaa_mode = false;
    }

    return 0;
}
