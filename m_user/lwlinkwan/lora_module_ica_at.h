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

#ifndef LINKWAN_AT_H
#define LINKWAN_AT_H

#include "uart_driver.h"
#define AT_CMD "AT"

#define AT_ERROR "+CME ERROR:"

// mandatory
#define LORA_AT_CJOINMODE "+CJOINMODE"  // join mode
#define LORA_AT_CFREQBAND "+CFREQBAND"  // freq band (OTAA)
#define LORA_AT_CDEVEUI "+CDEVEUI"  // dev eui (OTAA)
#define LORA_AT_CAPPEUI "+CAPPEUI"  // app eui (OTAA)
#define LORA_AT_CAPPKEY "+CAPPKEY"  // app key (OTAA)
#define LORA_AT_CDEVADDR "+CDEVADDR"  // dev addr (ABP)
#define LORA_AT_CAPPSKEY "+CAPPSKEY"  // sapp key (ABP)
#define LORA_AT_CNWKSKEY "+CNWKSKEY"  // nwk skey (ABP)
#define LORA_AT_CADDMULTICAST "+CADDMULTICAST"  // add mcast
#define LORA_AT_CDELMULTICAST "+CDELMULTICAST"  // del mcast
#define LORA_AT_CNUMMULTICAST "+CNUMMULTICAST"  // mcast num
#define LORA_AT_CFREQBANDMASK "+CFREQBANDMASK"  // freqband mask
#define LORA_AT_CULDLMODE "+CULDLMODE"  // ul and dl
#define LORA_AT_CLBT "+CLBT"  // LBT
#define LORA_AT_CPROPRIETARY "+CPROPRIETARY"  // PROPRIETARY
#define LORA_AT_CMONITORNODE "+CMONITORNODE"  // monitor node

#define LORA_AT_CWORKMODE "+CWORKMODE"  // work mode
#define LORA_AT_CREPEATERFREQ "+CREPEATERFREQ"  // repeater freq
#define LORA_AT_CCLASS "+CCLASS"  // class
#define LORA_AT_CBL "+CBL"  // battery level
#define LORA_AT_CSTATUS "+CSTATUS"  // cstatus
#define LORA_AT_CJOIN "+CJOIN"  // OTTA join

#define LORA_AT_DTX "+DTX"  // tx
#define LORA_AT_DRX "+DRX"  // rx

#define LORA_AT_CCONFIRM "+CCONFIRM"  //cfm or uncfm
#define LORA_AT_CAPPPORT "+CAPPPORT"  // app port
#define LORA_AT_CDATARATE "+CDATARATE"  // data rate
#define LORA_AT_CRSSI "+CRSSI"  // rssi
#define LORA_AT_CNBTRIALS "+CNBTRIALS"  // nb trans
#define LORA_AT_CRM "+CRM"  // report mode
#define LORA_AT_CTXP "+CTXP"  // tx power
#define LORA_AT_CLINKCHECK "+CLINKCHECK"  // link check
#define LORA_AT_CADR "+CADR"  // ADR
#define LORA_AT_CRXP "+CRXP"  // rx win params
#define LORA_AT_CFREQLIST "+CFREQLIST"  // freq list
#define LORA_AT_CRX1DELAY "+CRX1DELAY"  // rx1 win delay
#define LORA_AT_CSAVE "+CSAVE"  // save cfg
#define LORA_AT_CRESTORE "+CRESTORE"  // restore def cfg


// repeater
#define LORA_AT_CREPEATERFILTER "+CREPEATERFILTER"

// optional
#define LORA_AT_CGMI "+CGMI"  // manufacture identification
#define LORA_AT_CGMM "+CGMM"  // model identification
#define LORA_AT_CGMR "+CGMR"  // revision info
#define LORA_AT_CGSN "+CGSN"  // product serial number id
#define LORA_AT_CGBR "+CGBR"  // baud rate on UART interface

#define LORA_AT_ILOGLVL "+ILOGLVL"  // log level
#define LORA_AT_IREBOOT "+IREBOOT"
#define LORA_AT_IDEFAULT "+IDEFAULT"

#define LORA_AT_PRINTF(format, ...) lprint(format,  ##__VA_ARGS__)
void lorawan_at_serial_input(uint8_t cmd);
void lorawan_at_process(void);
void lorawan_at_init(void);

#endif
