/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : io_ctrl.c
 * Author        :
 * Date          : 2018-10-23
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#include "frtos_errno.h"
#include "frtos_drivers.h"
#include "frtos_lock.h"
#include "frtos_delay.h"
#include "config_driver.h"
#include "adc_driver.h"
#include "gpio_driver.h"
#include "io_ctrl.h"

/**************************************************************************************
* FunctionName   : io_ctrl_init()
* Description    : 设备初始化
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t io_ctrl_init(void)
{
	GPIO_COMM_INIT(PWR_12V_485_EN, GPIO_SPEED_FREQ_LOW, GPIO_MODE_OUTPUT_PP);
	GPIO_COMM_INIT(OUTPUT_3_3V_EN_1, GPIO_SPEED_FREQ_LOW, GPIO_MODE_OUTPUT_PP);
	GPIO_COMM_INIT(OUTPUT_3_3V_EN_2, GPIO_SPEED_FREQ_LOW, GPIO_MODE_OUTPUT_PP);
	GPIO_COMM_INIT(INPUT_ONOFF_1, GPIO_SPEED_FREQ_LOW, GPIO_MODE_INPUT);
	GPIO_COMM_INIT(INPUT_ONOFF_2, GPIO_SPEED_FREQ_LOW, GPIO_MODE_INPUT);

	GPIO_COMM_WRITE(OUTPUT_3_3V_EN_1, 1);
	GPIO_COMM_WRITE(OUTPUT_3_3V_EN_2, 1);

	return 0;
}

static __const struct driver io_ctrl = {
    .idx  = DRIVER_IOCTRL,
    .init = io_ctrl_init,
	.wakeup = io_ctrl_init,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
MODULE_INIT(io_ctrl);
