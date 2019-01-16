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
#include <stdio.h>
#include "utilities.h"
#include "board.h"
#include "connection.h"
#include "lora_module_export.h"
#include "Commissioning.h"

#include "frtos_app.h"
#include "frtos_mem.h"
#include "frtos_errno.h"
#include "frtos_lock.h"
#include "config_user.h"
#include "config_driver.h"
#include "frtos_ioctl.h"
#include "gpio_driver.h"
#include "frtos_sys.h"
#include "frtos_utils.h"
#include "data.pb-c.h"
#include "frtos_delay.h"
#include "task.h"
#include "frtos_drivers.h"
#include "proto.h"

#if MAIN_SDK
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
typedef struct _app_context
{
    int lora_conn_status;
} app_context_t;

/* Private functions ---------------------------------------------------------*/
static int lora_on_connect(int success, void* ctx)
{
    return 0;
}

static int lora_on_disconnect(void* ctx)
{
    return 0;
}

static int lora_data_arrived(esl_data_type_t type, uint8_t port, uint8_t* data, int len, void* ctx)
{
    return 0;
}

static esl_connection_ops_cb_t esl_lora_connection_ops_cb = {
    .data_arrived     = lora_data_arrived,
    .on_connect       = lora_on_connect,
    .on_disconnect    = lora_on_disconnect,
    .lora_classbswitch_result = NULL
};
static app_context_t g_app_ctx;

static rx_cb_t rx_cb(uint8_t *data, int len)
{
	printf("recv data from gw\r\n");
	//lwdump_data(data,len);
}

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
static int32_t linkwan_run( void )
{
    BoardInitMcu( );
    BoardInitPeriph( );

    printf( "\r\n**********************************************\r\n" );
    printf( "\r\n====== LinkWAN LoRa Node SDK Example App =====\r\n" );
    printf( "\r\n**********************************************\r\n" );

	//注册端口号10；
    lorawan_module_reg_app_rx_cb(10, rx_cb);
    
	lorawan_module_init(&esl_lora_connection_ops_cb, &g_app_ctx);

    #ifdef USED_RTOS
        osKernelStart();
    #else
        while( 1 )
        {
            lorawan_module_main(NULL);
        }
    #endif
}

static __const struct task linkwan = {
	.idx   = LORAWAN_PID,
	.name  = "linkwan",
	.pro   = 3,
	.depth = 512,
	.main  = linkwan_run,
};

TASK_REGISTER(linkwan);
#endif
