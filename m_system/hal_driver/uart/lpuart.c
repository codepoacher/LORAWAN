/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : l451_lpuart.c
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#include "uart_driver.h"
#include "frtos_app.h"
#include "frtos_mem.h"
#include "frtos_errno.h"
#include "frtos_lock.h"
#include "frtos_queue.h"
#include "frtos_ioctl.h"
#include "frtos_sys.h"
#include "config_user.h"

/*************************************************************************************
* Description    : 模块内部全局变量定义
*************************************************************************************/
static struct uart g_lpuart;
static UART_HandleTypeDef g_lphuart_handle;

/*************************************************************************************
* FunctionName   : AES_RNG_LPUART1_IRQHandler()
* Description    : LPUART中断
* EntryParameter : None
* ReturnValue    : None
*************************************************************************************/
void AES_RNG_LPUART1_IRQHandler(void)
{
	HAL_UART_IRQHandler(g_lpuart.comm_huart->uart);
}


/*************************************************************************************
* FunctionName   : lpuart_data_add()
* Description    : 添加lpuart串口数据
* EntryParameter : None
* ReturnValue    : None
*************************************************************************************/
void lpuart_data_add(void)
{
	loop_buffer_put(g_lpuart.comm_huart->rx_buf, &g_lpuart.recv_data, 1);
	HAL_UART_Receive_IT(g_lpuart.comm_huart->uart, &g_lpuart.recv_data, 1);
}

/*************************************************************************************
* FunctionName   : lpuart_phy_init()
* Description    : UART物理初始化
* EntryParameter : None
* ReturnValue    : 返回错误码
*************************************************************************************/
static inline int8_t lpuart_phy_init(UART_HandleTypeDef *hlpuart)
{
    /* 1.lpuart初始化 */
	hlpuart->Instance					= LPUART1;
	hlpuart->Init.BaudRate				= LPUART_BAUD;
	hlpuart->Init.WordLength			= UART_WORDLENGTH_8B;
	hlpuart->Init.StopBits				= UART_STOPBITS_1;
	hlpuart->Init.Parity				= UART_PARITY_NONE;
	hlpuart->Init.Mode					= UART_MODE_TX_RX;
	hlpuart->Init.HwFlowCtl				= UART_HWCONTROL_NONE;
	hlpuart->Init.OneBitSampling		= UART_ONE_BIT_SAMPLE_DISABLE;
	hlpuart->AdvancedInit.AdvFeatureInit= UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init(hlpuart) != HAL_OK)
		return -EINVAL;

    return 0;
}

/*************************************************************************************
* FunctionName   : lpuart_phy_timer()
* Description    : UART物理监视定时器
* EntryParameter : None
* ReturnValue    : 返回错误码
*************************************************************************************/
static void lpuart_phy_timer(softimer_t timer)
{
    (void)timer;
	uart_comm_timer_handle(LPUART_RXBUF_LEN, 
							  g_lpuart.comm_huart->rx_buf, 
							  g_lpuart.comm_huart->rx_queue);
}

/*************************************************************************************
* FunctionName   : lpuart_write()
* Description    : 写
* EntryParameter : *args,参数, len,参数长度
* ReturnValue    : 返回写入的字节数, 返回错误码
*************************************************************************************/
static int32_t lpuart_write(uint8_t idx, void *data, int32_t len)
{
	return comm_uart_write_it(g_lpuart.comm_huart, data, len);
}

/*************************************************************************************
* FunctionName   : lpuart_ioctrl()
* Description    : 控制
* EntryParameter : *args,参数, len,参数长度
* ReturnValue    : 返回错误码
*************************************************************************************/
static int32_t lpuart_ioctrl(uint8_t idx, int32_t cmd,  void *args,int32_t len)
{
	return comm_uart_ioctl(g_lpuart.comm_huart, cmd, args, len);
}

/*************************************************************************************
* FunctionName   : lpuart_init()
* Description    : 设备初始化
* EntryParameter : None
* ReturnValue    : None
*************************************************************************************/
int32_t lpuart_init(void)
{
	int8_t ret = 0;

	memset(&g_lpuart, 0, sizeof(g_lpuart));
	g_lpuart.comm_huart = create_comm_huart(LPUART_RXBUF_LEN, LPUART_RXQUEUE_LEN, 
										   LPUART_RXQUEUE_SIZE, LPUART_TIMER_TIME, 
										   lpuart_phy_timer);
	if (NULL == g_lpuart.comm_huart)
		return -EMEM;

	g_lpuart.comm_huart->idx = DRIVER_LPUART; 

    ret = lpuart_phy_init(&g_lphuart_handle);
	g_lpuart.comm_huart->uart = &g_lphuart_handle;


	debug("build time: %s %s\r\n", __DATE__, __TIME__);
	HAL_UART_Receive_IT(g_lpuart.comm_huart->uart, &g_lpuart.recv_data, 1);

	return ret;
}

/*************************************************************************************
* FunctionName   : lpuart_rx()
* Description    : UART驱动任务
* EntryParameter : None
* ReturnValue    : 返回错误码
*************************************************************************************/
static void lpuart_rx(void* args)
{
	/*uart_comm_timer_handle(LPUART_RXBUF_LEN, 
							  g_lpuart.comm_huart->rx_buf, 
							  g_lpuart.comm_huart->rx_queue);*/
	while(1)
		comm_uart_read(g_lpuart.comm_huart);

	return 0;
}

/*************************************************************************************
* FunctionName   : lprint()
* Description    : 串口输出
* EntryParameter : ftm,输出格式
* ReturnValue    : None
*************************************************************************************/
void lprint(char *fmt, ...)
{
	uint8_t p[64];
	va_list args;
	int n;

	va_start(args, fmt);
	n = vsnprintf((char*)p, 64, fmt, args);
	if (n < 0)
		return;
	va_end(args);

	fdrive_write(DRIVER_LPUART, p, strlen(p));
}

/*************************************************************************************
* FunctionName   : uart4_wakeup()
* Description    : UART唤醒后恢复引脚
* EntryParameter : None
* ReturnValue    : 返回错误码
*************************************************************************************/
static int32_t lpuart_wakeup(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF6_LPUART1;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	return 0;
}

static __const struct driver lpuart = {
    .idx  = DRIVER_LPUART,
    .init = lpuart_init,
//    .run = lpuart_rx,
	.write = lpuart_write,
    .ioctl = lpuart_ioctrl, 
	.wakeup = lpuart_wakeup,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
EARLY_INIT(lpuart);

/**************************************************************************************
* description    : 定义任务结构
**************************************************************************************/
static __const struct task cmd_task  = {
    .idx   =  CMD_TASK_PID,
    .name  = "cmd_task",
	.pro   = 3,
	.depth = 256,
	.main  = lpuart_rx, 
};

/**************************************************************************************
* description    : 任务注册
**************************************************************************************/
TASK_REGISTER(cmd_task);
