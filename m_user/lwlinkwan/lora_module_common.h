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
#ifndef LORA_MODULE_COMMON_H
#define LORA_MODULE_COMMON_H

/*!
 * LoRa Module Status type
 */
typedef enum {
    LoRa_Idle = 0,       //being ready to send data
    LoRa_Sending,
    LoRa_Send_Failed,
    LoRa_Send_Success,
    LoRa_Join_Success,
    LoRa_Join_Failed,
    LoRa_Network_Abnormal,
    LoRa_Joining,
    LoRa_Class_Switching,//valid only for switching to classB
    LoRa_Class_SwitchOK,
    LoRa_Class_SwitchNOK,
    LoRa_ClassC_Listen,
    LoRa_Initing,
    LoRa_Init_Done
} LoRaModuleStatus_t;

typedef enum eJoinModeType {
    JOIN_MODE_OTAA = 0,
    JOIN_MODE_ABP
} JoinMode_t;

typedef int (*rx_cb_t)(uint8_t *data, int len);

#endif
