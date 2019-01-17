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

#include "LoRaMac.h"
#include "lora_module_export.h"
#include "lora_module_controller.h"
#include "lora_module_certification.h"

/*!
 * LoRaWAN compliance tests support data
 */
static struct ComplianceTest_s {
    bool Running;
    uint8_t State;
    bool IsTxConfirmed;
    uint8_t AppPort;
    uint8_t AppDataSize;
    uint8_t *AppDataBuffer;
    uint16_t DownLinkCounter;
    bool LinkCheck;
    uint8_t DemodMargin;
    uint8_t NbGateways;
    uint8_t uplink_cycle;
} ComplianceTest;

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

/* Exported functions definition---------------------------------------------------------*/
bool lorawan_certif_running(void)
{
    return ComplianceTest.Running;
}

uint8_t lorawan_certif_uplink_cycle(void)
{
    return ComplianceTest.uplink_cycle;
}


void lorawan_certif_DownLinkIncrement( void )
{
    ComplianceTest.DownLinkCounter++;
}

void lorawan_certif_linkCheck(MlmeConfirm_t *mlmeConfirm)
{
    ComplianceTest.LinkCheck = true;
    ComplianceTest.DemodMargin = mlmeConfirm->DemodMargin;
    ComplianceTest.NbGateways = mlmeConfirm->NbGateways;
}

void lorawan_certif_tx( void )
{
    if ( ComplianceTest.LinkCheck == true ) {
        ComplianceTest.LinkCheck = false;
        g_lora_app_data_size = 3;
        g_lora_app_data_buffer[0] = 5;
        g_lora_app_data_buffer[1] = ComplianceTest.DemodMargin;
        g_lora_app_data_buffer[2] = ComplianceTest.NbGateways;
        ComplianceTest.State = 1;
    } else {
        switch ( ComplianceTest.State ) {
            case 4:
                ComplianceTest.State = 1;
                break;
            case 1:
                g_lora_app_data_size = 2;
                g_lora_app_data_buffer[0] = ComplianceTest.DownLinkCounter >> 8;
                g_lora_app_data_buffer[1] = ComplianceTest.DownLinkCounter;
                break;
        }
    }
}

void lorawan_certif_rx( McpsIndication_t *mcpsIndication)
{
    if ( ComplianceTest.Running == false ) {
        // Check compliance test enable command (i)
        if ( ( mcpsIndication->BufferSize == 4 ) &&
             ( mcpsIndication->Buffer[0] == 0x01 ) &&
             ( mcpsIndication->Buffer[1] == 0x01 ) &&
             ( mcpsIndication->Buffer[2] == 0x01 ) &&
             ( mcpsIndication->Buffer[3] == 0x01 ) ) {
            g_lora_is_confirmup = false;
            g_lora_port = CERTIF_PORT;
            g_lora_app_data_size_backup = g_lora_app_data_size;
            g_lora_app_data_size = 2;
            ComplianceTest.DownLinkCounter = 0;
            ComplianceTest.LinkCheck = false;
            ComplianceTest.DemodMargin = 0;
            ComplianceTest.NbGateways = 0;
            ComplianceTest.Running = true;
            ComplianceTest.uplink_cycle = 5;
            ComplianceTest.State = 1;

            MibRequestConfirm_t mibReq;
            mibReq.Type = MIB_ADR;
            mibReq.Param.AdrEnable = true;
            LoRaMacMibSetRequestConfirm( &mibReq );

#if defined( REGION_EU868 )
            LoRaMacTestSetDutyCycleOn( false );
#endif
            lorawan_control_trigger_send();
            PRINTF( "\r\n###### ===== Enter Certification Mode! ==== ######\r\n" );
        }
    }

    else {
        ComplianceTest.State = mcpsIndication->Buffer[0];
        switch ( ComplianceTest.State ) {
            case 0: // Check compliance test disable command (ii)
                g_lora_is_confirmup = LORAWAN_CONFIRMED_MSG_ON;
                g_lora_port = g_lora_app_port;
                g_lora_app_data_size = g_lora_app_data_size_backup;
                ComplianceTest.DownLinkCounter = 0;
                ComplianceTest.Running = false;

                MibRequestConfirm_t mibReq;
                mibReq.Type = MIB_ADR;
                mibReq.Param.AdrEnable = LORAWAN_ADR_ON;
                LoRaMacMibSetRequestConfirm( &mibReq );
#if defined( REGION_EU868 )
                LoRaMacTestSetDutyCycleOn( LORAWAN_DUTYCYCLE_ON );
#endif
                PRINTF( "\r\n###### ===== Exit Certification Mode! ==== ######\r\n" );
                break;
            case 1: // (iii, iv)
                g_lora_app_data_size = 2;
                break;
            case 2: // Enable confirmed messages (v)
                g_lora_is_confirmup = true;
                ComplianceTest.State = 1;
                break;
            case 3:  // Disable confirmed messages (vi)
                g_lora_is_confirmup = false;
                ComplianceTest.State = 1;
                break;
            case 4: // (vii)
                g_lora_app_data_size = mcpsIndication->BufferSize;

                g_lora_app_data_buffer[0] = 4;
                for ( uint8_t i = 1; i < MIN( g_lora_app_data_size, LORAWAN_APP_DATA_MAX_SIZE ); i++ ) {
                    g_lora_app_data_buffer[i] = mcpsIndication->Buffer[i] + 1;
                }
                break;
            case 5: { // (viii)
                MlmeReq_t mlmeReq;
                mlmeReq.Type = MLME_LINK_CHECK;
                LoRaMacStatus_t status = LoRaMacMlmeRequest( &mlmeReq );
                PRINTF( "\r\n###### ===== MLME-Request - MLME_LINK_CHECK ==== ######\r\n" );
                PRINTF( "STATUS      : %s\r\n", MacStatusStrings[status] );
            }
            break;
            case 6: { // (ix)
                // Disable TestMode and revert back to normal operation
                g_lora_is_confirmup = LORAWAN_CONFIRMED_MSG_ON;
                g_lora_port = g_lora_app_port;
                g_lora_app_data_size = g_lora_app_data_size_backup;
                ComplianceTest.DownLinkCounter = 0;
                ComplianceTest.Running = false;

                MibRequestConfirm_t mibReq;
                mibReq.Type = MIB_ADR;
                mibReq.Param.AdrEnable = LORAWAN_ADR_ON;
                LoRaMacMibSetRequestConfirm( &mibReq );
#if defined( REGION_EU868 )
                LoRaMacTestSetDutyCycleOn( LORAWAN_DUTYCYCLE_ON );
#endif

                lorawan_module_join(1);
            }
            break;
            case 7: { // (x)
                if ( mcpsIndication->BufferSize == 3 ) {
                    MlmeReq_t mlmeReq;
                    mlmeReq.Type = MLME_TXCW;
                    mlmeReq.Req.TxCw.Timeout = ( uint16_t )( ( mcpsIndication->Buffer[1] << 8 ) | mcpsIndication->Buffer[2] );
                    LoRaMacStatus_t status = LoRaMacMlmeRequest( &mlmeReq );
                    PRINTF( "\r\n###### ===== MLME-Request - MLME_TXCW ==== ######\r\n" );
                    PRINTF( "STATUS      : %s\r\n", MacStatusStrings[status] );
                } else if ( mcpsIndication->BufferSize == 7 ) {
                    MlmeReq_t mlmeReq;
                    mlmeReq.Type = MLME_TXCW_1;
                    mlmeReq.Req.TxCw.Timeout = ( uint16_t )( ( mcpsIndication->Buffer[1] << 8 ) | mcpsIndication->Buffer[2] );
                    mlmeReq.Req.TxCw.Frequency = ( uint32_t )( ( mcpsIndication->Buffer[3] << 16 ) | ( mcpsIndication->Buffer[4] << 8 ) |
                                                               mcpsIndication->Buffer[5] ) * 100;
                    mlmeReq.Req.TxCw.Power = mcpsIndication->Buffer[6];
                    LoRaMacStatus_t status = LoRaMacMlmeRequest( &mlmeReq );
                    PRINTF( "\r\n###### ===== MLME-Request - MLME_TXCW1 ==== ######\r\n" );
                    PRINTF( "STATUS      : %s\r\n", MacStatusStrings[status] );
                }
                ComplianceTest.State = 1;
            }
            break;
            case 8: { // Send DeviceTimeReq
                MlmeReq_t mlmeReq;

                mlmeReq.Type = MLME_DEVICE_TIME;

                LoRaMacStatus_t status = LoRaMacMlmeRequest( &mlmeReq );
                if (status != LORAMAC_STATUS_OK) {
                    PRINTF("LoRa:STATUS error: %s\r\n", MacStatusStrings[status]);
                } else {
                    lorawan_control_trigger_send();
                }
            }
            break;
            case 9: { // Switch end device Class
                // CLASS_A = 0, CLASS_B = 1, CLASS_C = 2
                DeviceClass_t class;
                class = ( DeviceClass_t )mcpsIndication->Buffer[1];

                if (class != CLASS_A && class != CLASS_B && class != CLASS_C) {
                    break;
                }

                g_current_target_lora_class_type = class;
                lorawan_control_switch_class(class);
                if (class == CLASS_A || class == CLASS_C) {
                    ComplianceTest.State = 1;
                } else if (class == CLASS_B) {
                    // Disable TestMode and revert back to normal operation
                    g_lora_is_confirmup = LORAWAN_CONFIRMED_MSG_ON;
                    g_lora_port = g_lora_app_port;
                    g_lora_app_data_size = g_lora_app_data_size_backup;
                    ComplianceTest.DownLinkCounter = 0;
                    ComplianceTest.Running = false;

                    MibRequestConfirm_t mibReq;
                    mibReq.Type = MIB_ADR;
                    mibReq.Param.AdrEnable = LORAWAN_ADR_ON;
                    LoRaMacMibSetRequestConfirm( &mibReq );
#if defined( REGION_EU868 )
                    LoRaMacTestSetDutyCycleOn( LORAWAN_DUTYCYCLE_ON );
#endif
                    PRINTF( "\r\n###### ===== Exit Certification Mode! ==== ######\r\n" );
                }
            }
            break;
            case 10: { // Send PingSlotInfoReq
                MlmeReq_t mlmeReq;

                mlmeReq.Type = MLME_PING_SLOT_INFO;

                mlmeReq.Req.PingSlotInfo.PingSlot.Value = mcpsIndication->Buffer[1];

                LoRaMacStatus_t status = LoRaMacMlmeRequest( &mlmeReq );
                if (status != LORAMAC_STATUS_OK) {
                    PRINTF("LoRa:STATUS error: %s\r\n", MacStatusStrings[status]);
                } else {
                    lorawan_control_trigger_send();
                }
            }
            break;
            case 11: { // Send BeaconTimingReq
                MlmeReq_t mlmeReq;

                mlmeReq.Type = MLME_BEACON_TIMING;

                LoRaMacStatus_t status = LoRaMacMlmeRequest( &mlmeReq );
                if (status != LORAMAC_STATUS_OK) {
                    PRINTF("LoRa:STATUS error: %s\r\n", MacStatusStrings[status]);
                } else {
                    lorawan_control_trigger_send();
                }
            }
            break;
            case 0x81: {
                if (mcpsIndication->BufferSize == 2 && mcpsIndication->Buffer[1] != 0) {
                    ComplianceTest.uplink_cycle = mcpsIndication->Buffer[1];
                    ComplianceTest.State = 1;
                } else {
                    PRINTF("LoRa:param error\r\n");
                }
            }
            break;
            case 0x82: {
                // Don't disable TestMode
                MibRequestConfirm_t mibReq;
                mibReq.Type = MIB_ADR;
                mibReq.Param.AdrEnable = LORAWAN_ADR_ON;
                LoRaMacMibSetRequestConfirm( &mibReq );
#if defined( REGION_EU868 )
                LoRaMacTestSetDutyCycleOn( LORAWAN_DUTYCYCLE_ON );
#endif

                lorawan_module_join(1);
                ComplianceTest.State = 1;
            }
            break;
            default:
                break;
        }
    }
}
