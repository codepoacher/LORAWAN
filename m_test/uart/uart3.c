#include "config_test.h"
#include "uart_driver.h"
#include "config_driver.h"
#include "frtos_app.h"
#include "frtos_mem.h"
#include "frtos_errno.h"
#include "frtos_lock.h"
#include "config_user.h"
#include "config_driver.h"
#include "frtos_ioctl.h"
#include "frtos_libpack.h"
#include "frtos_list.h"
#include "frtos_sys.h"
#include "loopbuf.h"
#include "transport.h"
#include "debug.h"
#include "frtos_tasklet.h"



#define UART3_INTERVAL 10

/**************************************************************************************
* FunctionName   : uart3_handle()
* Description    : uart3处理
* EntryParameter : idx,驱动序号， data，接收的数据， len,接收数据长度
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t uart3_set(uint8_t idx, void *data, int32_t len) { 
	debug("recvd_uart3: %s\r\n", (char*)data);
	return 0;	
}

/**************************************************************************************
* FunctionName   : uart3_timer()
* Description    : uart3定时器，用于定时向uart3发送数据
* EntryParameter : idx,驱动序号， data，接收的数据， len,接收数据长度
* ReturnValue    : 返回错误码
**************************************************************************************/
static void uart3_timer(void *args)
{
	char test_data[32] = { 0 };
	static int32_t cnt = 10000000;
	struct piddata *pdata = NULL;

	if (cnt >= 0xfffffff0)
		cnt = 100000000;
	sprintf(test_data, "uart3_test:%d", cnt++);
	//sprintf(test_data, "%d", cnt++);
	//debug("send: %s, len = %d\r\n", test_data, strlen(test_data) + 1);
	pdata = (struct piddata *)mem_malloc(sizeof(struct piddata) + strlen(test_data) + 1);
	if (!pdata)
		return;
	memset(pdata, 0, sizeof(struct piddata) + strlen(test_data) + 1);
	pdata->id = 201;
	pdata->len = strlen(test_data) + 1;
	memcpy(pdata->data, test_data, strlen(test_data) + 1);
	fuser_data_set(INIT_PID, pdata, pdata->len + sizeof(struct piddata));
	mem_free(pdata);
	tasklet_schedule(uart3_timer, NULL, UART3_INTERVAL);
}

/**************************************************************************************
* FunctionName   : uart3_init()
* Description    : CAN初始化初始化
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t uart3_init(void)
{
	debug("uart3_init\r\n");			
	tasklet_schedule(uart3_timer, NULL, UART3_INTERVAL);

	return 0;
}

/**************************************************************************************
* Description    : 定义uart3任务结构
**************************************************************************************/
static __const struct applite uart3 = {
	.idx	= UART3_TEST_PID,
	.name	= "uart3_test",
	.init	= uart3_init,
	.set	= uart3_set,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
//APP_REGISTER(uart3);

