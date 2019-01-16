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
#include "loopbuf.h"

#define MAX_BUF_LEN 32 
extern uint8_t g_fakong[7];
extern uint8_t g_success[8];
extern uint8_t g_fail[5];
extern uint8_t g_time[13];
static struct loop_buffer *rx_buf = NULL;		//接收环形缓冲区
uint8_t g_485_gzcs = 0;
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

	loop_buffer_put(rx_buf, recv_data, len);
	if (g_485_gzcs) {
		uint8_t loop_len = loop_buffer_use(rx_buf);
		if (loop_len >= 8) {
			uint8_t tmp_data[MAX_BUF_LEN] = {0}, *stat = NULL;
			loop_buffer_get(rx_buf, tmp_data, loop_len);
			if (strstr(tmp_data, "485 test") != NULL) {
				stat = g_success;
			}
			else {
				stat = g_fail;
			}
			debug("%s:%s:485:%s::\r\n", g_fakong, g_time, stat);
			g_485_gzcs = 0;
		}
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
	debug("rs485_1 init\r\n");
	fdrive_ioctl(DRIVER_UART5, _IOC_SET_CB, rs485_recv, sizeof(rs485_recv));
	rx_buf = init_loop_buffer(MAX_BUF_LEN);

	return 0;
}

/**************************************************************************************
* description    : 定义任务结构
**************************************************************************************/
static __const struct applite rs485_1 = {
    .idx   = RS485_1_PID,
    .name  = "rs485_1",
    .init  = rs485_init,
};

/**************************************************************************************
* description    : 模块初始化
**************************************************************************************/
APP_REGISTER(rs485_1);
