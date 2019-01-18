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
#include <math.h>

//#include "debug.h"
//#include "hw.h"
//#include "bsp.h"
//#include "vcom.h"

#include "version.h"
#include "utilities.h"
#include "board.h"

//#include "cmsis_os.h"
//#include "lite-log.h"
#include "lora_module_ica_at.h"
#include "LoRaMac.h"
#include "lora_module_controller.h"
#include "lora_multicast_control.h"
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

static esl_connection_ops_cb_t *g_lora_esl_connection_ops_cb = NULL;
static void                    *g_lora_cm_data               = NULL;

int lorawan_module_init(esl_connection_ops_cb_t *_esl_cm_ops_cb, void *data)
{
#ifdef USED_RTOS
    PRINTF("creat lora task.\r\n");

    osThreadDef(linkwanTask, lorawan_module_main, osPriorityHigh, 1, 512);
    if (NULL == osThreadCreate(osThread(linkwanTask), NULL)) {
        log_err("task create failed\r\n");
    }
#endif
    g_lora_esl_connection_ops_cb = _esl_cm_ops_cb;
    g_lora_cm_data = data;
    //assert_param(g_lora_esl_connection_ops_cb);

    lorawan_control_init(_esl_cm_ops_cb, data);
    lorawan_at_init();
    return 0;
}

int lorawan_module_deinit(void)
{
    lorawan_control_deinit();
    //osThreadTerminate(osThreadGetId());
    return 0;
}

void lorawan_module_main(void const *arg )
{
    DeviceState_t devicestate;

    while (true) {

#ifdef USED_RTOS
        //suspend
#endif

        // Process Radio IRQ
        if ( Radio.IrqProcess != NULL ) {
            //Radio.IrqProcess( );
        }

        lorawan_at_process();
        LoRaMacProcess( );
        devicestate = lorawan_control_process( );

        if (DEVICE_STATE_SLEEP != devicestate) {
#ifdef USED_RTOS
            //release
#endif
        } else {
#ifdef USED_RTOS

#else
            // Wake up through events
            BoardLowPowerHandler( );
#endif
        }
    }
}
