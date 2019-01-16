/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : uart4.c
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#if 1
#include "uart_driver.h"
#include "gpio_driver.h"
#include "frtos_delay.h"
/*************************************************************************************
* Description    : 模块内部全局变量定义
*************************************************************************************/
static struct uart g_uart4;
static UART_HandleTypeDef g_huart4_handle;
extern UART_HandleTypeDef g_huart5_handle;
/*************************************************************************************
* FunctionName   : UART4_5_IRQHandler()
* Description    : UART4、5中断
* EntryParameter : None
* ReturnValue    : None
*************************************************************************************/
void USART4_5_IRQHandler(void)
{
	HAL_UART_IRQHandler(&g_huart4_handle);
	HAL_UART_IRQHandler(&g_huart5_handle);
}

/*************************************************************************************
* FunctionName   : uart4_data_add()
* Description    : 添加uart4串口数据
* EntryParameter : None
* ReturnValue    : None
*************************************************************************************/
void uart4_data_add(void)
{
	loop_buffer_put(g_uart4.comm_huart->rx_buf, &g_uart4.recv_data, 1);
	HAL_UART_Receive_IT(g_uart4.comm_huart->uart, &g_uart4.recv_data, 1);
}

/*************************************************************************************
* FunctionName   : uart4_wakeup()
* Description    : UART唤醒后恢复引脚
* EntryParameter : None
* ReturnValue    : 返回错误码
*************************************************************************************/
static int32_t uart4_wakeup(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF6_USART4;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	return 0;
}

/*************************************************************************************
* FunctionName   : uart4_phy_init()
* Description    : UART物理初始化
* EntryParameter : None
* ReturnValue    : 返回错误码
*************************************************************************************/
static inline int8_t uart4_phy_init(UART_HandleTypeDef *huart4)
{
    /* 1.uart4 初始化 */
	huart4->Instance					= USART4;
	huart4->Init.BaudRate				= UART4_BAUD;
	huart4->Init.WordLength				= UART_WORDLENGTH_8B;
	huart4->Init.StopBits				= UART_STOPBITS_1;
	huart4->Init.Parity					= UART_PARITY_NONE;
	huart4->Init.Mode					= UART_MODE_TX_RX;
	huart4->Init.HwFlowCtl				= UART_HWCONTROL_NONE;
	huart4->Init.OverSampling			= UART_OVERSAMPLING_16;
	huart4->Init.OneBitSampling			= UART_ONE_BIT_SAMPLE_DISABLE;
	huart4->AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init(huart4) != HAL_OK)
		return -EINVAL;

    return 0;
}

/*************************************************************************************
* FunctionName   : uart4_phy_timer()
* Description    : UART4物理监视定时器
* EntryParameter : None
* ReturnValue    : 返回错误码
*************************************************************************************/
static void uart4_phy_timer(softimer_t timer)
{
	uart_comm_timer_handle(UART4_RXBUF_LEN, 
							  g_uart4.comm_huart->rx_buf, 
							  g_uart4.comm_huart->rx_queue);
    (void)timer;
}

/*************************************************************************************
* FunctionName   : uart4_write()
* Description    : 写
* EntryParameter : *args,参数, len,参数长度
* ReturnValue    : 返回写入的字节数, 返回错误码
*************************************************************************************/
static int32_t uart4_write(uint8_t idx, void *data, int32_t len)
{
	return comm_uart_write_it(g_uart4.comm_huart, data, len);
}

/*************************************************************************************
* FunctionName   : uart4_ioctrl()
* Description    : 控制
* EntryParameter : *args,参数, len,参数长度
* ReturnValue    : 返回错误码
*************************************************************************************/
static int32_t uart4_ioctrl(uint8_t idx, int32_t cmd,  void *args,int32_t len)
{
	return comm_uart_ioctl(g_uart4.comm_huart, cmd, args, len);
}

/*************************************************************************************
* FunctionName   : uart4_rx()
* Description    : UART驱动任务
* EntryParameter : None
* ReturnValue    : 返回错误码
*************************************************************************************/
static int32_t uart4_rx(void)
{
	return comm_uart_read(g_uart4.comm_huart);
}

/*************************************************************************************
* FunctionName   : uart_init()
* Description    : 设备初始化
* EntryParameter : None
* ReturnValue    : 返回错误码
*************************************************************************************/
static int32_t __init uart4_init(void)
{
	int8_t ret = 0;

	memset(&g_uart4, 0, sizeof(g_uart4));
	g_uart4.comm_huart = create_comm_huart(UART4_RXBUF_LEN, UART4_RXQUEUE_LEN, 
										  UART4_RXQUEUE_SIZE, UART4_TIMER_TIME, 
										  uart4_phy_timer);
	if (NULL == g_uart4.comm_huart)
		return -EMEM;

	g_uart4.comm_huart->idx = DRIVER_UART4;

	ret = uart4_phy_init(&g_huart4_handle);
	g_uart4.comm_huart->uart = &g_huart4_handle;
	HAL_UART_Receive_IT(g_uart4.comm_huart->uart, &g_uart4.recv_data, 1);

	return ret;
}

static __const struct driver uart4 = {
    .idx  = DRIVER_UART4,
    .init = uart4_init,
    .run = uart4_rx,
	.write = uart4_write,
    .ioctl = uart4_ioctrl,
	.wakeup = uart4_wakeup,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
MODULE_INIT(uart4);
#endif
