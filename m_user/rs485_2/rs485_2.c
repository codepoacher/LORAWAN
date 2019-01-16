/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : rs485_1.c
 * Author        :
 * Date          : 2018-10-19
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#include "frtos_app.h"
#include "frtos_mem.h"
#include "frtos_errno.h"
#include "frtos_lock.h"
#include "config_user.h"
#include "config_driver.h"
#include "frtos_ioctl.h"
#include "frtos_sys.h"
#include "data.pb-c.h"
#include "gpio_driver.h"

/**************************************************************************************
* FunctionName   : rs485_recv()
* Description    : 485接收函数
* EntryParameter : idx, 回调标记, data,数据指针, len,数据长度
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t rs485_recv(int32_t idx, void *data, int32_t len)
{
	uint32_t i = 0;
	uint8_t *recv_data = (uint8_t*)data;
	for (; i < len; i++) {
//		if (recv_data[i] != 0x00)
//			lprint("%c", recv_data[i]);
	}
}

/**************************************************************************************
* FunctionName   : rs485_init()
* Description    : 485初始化
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t rs485_init(void)
{
	fdrive_ioctl(DRIVER_UART4, _IOC_SET_CB, rs485_recv, sizeof(rs485_recv));
}

/**************************************************************************************
* Description    : 定义ADC任务结构
**************************************************************************************/
static __const struct applite rs485_2 = {
    .idx   = RS485_2_PID,
    .name  = "rs485_2",
    .init  = rs485_init,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
//APP_REGISTER(rs485_2);
