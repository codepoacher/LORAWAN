/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : uart2.c
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#if 0
#include "uart_driver.h"
/*************************************************************************************
* Description    : 模块内部全局变量定义
*************************************************************************************/
static struct uart g_uart2;

/*************************************************************************************
* FunctionName   : UART5_IRQHandler()
* Description    : UART5中断
* EntryParameter : None
* ReturnValue    : None
*************************************************************************************/
void USART2_IRQHandler(void)
{
	HAL_UART_IRQHandler(&g_uart2.comm_huart->uart);
}

/*************************************************************************************
* FunctionName   : uart2_data_add()
* Description    : 添加uart2串口数据
* EntryParameter : None
* ReturnValue    : None
*************************************************************************************/
void uart2_data_add(void)
{
	loop_buffer_put(g_uart2.comm_huart->rx_buf, &g_uart2.recv_data, 1);
	HAL_UART_Receive_IT(&g_uart2.comm_huart->uart, &g_uart2.recv_data, 1);
}

/*************************************************************************************
* FunctionName   : uart2_phy_init()
* Description    : UART物理初始化
* EntryParameter : None
* ReturnValue    : 返回错误码
*************************************************************************************/
static inline int8_t uart2_phy_init(UART_HandleTypeDef *huart2)
{
    /* 1.uart2 初始化 */
	huart2->Instance					= USART2;
	huart2->Init.BaudRate				= UART2_BAUD;
	huart2->Init.WordLength				= UART_WORDLENGTH_8B;
	huart2->Init.StopBits				= UART_STOPBITS_1;
	huart2->Init.Parity					= UART_PARITY_NONE;
	huart2->Init.Mode					= UART_MODE_TX_RX;
	huart2->Init.HwFlowCtl				= UART_HWCONTROL_NONE;
	huart2->Init.OverSampling			= UART_OVERSAMPLING_16;
	huart2->Init.OneBitSampling			= UART_ONE_BIT_SAMPLE_DISABLE;
	huart2->AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init(huart2) != HAL_OK)
		return -EINVAL;

    return 0;
}

/*************************************************************************************
* FunctionName   : uart2_phy_timer()
* Description    : UART2物理监视定时器
* EntryParameter : None
* ReturnValue    : 返回错误码
*************************************************************************************/
static void uart2_phy_timer(softimer_t timer)
{
	uart_comm_timer_handle(UART2_RXBUF_LEN, 
							  g_uart2.comm_huart->rx_buf, 
							  g_uart2.comm_huart->rx_queue);
    (void)timer;
}

/*************************************************************************************
* FunctionName   : uart2_write()
* Description    : 写
* EntryParameter : *args,参数, len,参数长度
* ReturnValue    : 返回写入的字节数, 返回错误码
*************************************************************************************/
static int32_t uart2_write(uint8_t idx, void *data, int32_t len)
{
	uint32_t ret;

	ret = comm_uart_write_it(g_uart2.comm_huart, data, len);

	return ret;
}

/*************************************************************************************
* FunctionName   : uart2_ioctrl()
* Description    : 控制
* EntryParameter : *args,参数, len,参数长度
* ReturnValue    : 返回错误码
*************************************************************************************/
static int32_t uart2_ioctrl(uint8_t idx, int32_t cmd,  void *args,int32_t len)
{
	return comm_uart_ioctl(g_uart2.comm_huart, cmd, args, len);
}

/*************************************************************************************
* FunctionName   : uart2_rx()
* Description    : UART驱动任务
* EntryParameter : None
* ReturnValue    : 返回错误码
*************************************************************************************/
static int32_t uart2_rx(void)
{
	return comm_uart_read(g_uart2.comm_huart);
}

/*************************************************************************************
* FunctionName   : uart_init()
* Description    : 设备初始化
* EntryParameter : None
* ReturnValue    : 返回错误码
*************************************************************************************/
static int32_t __init uart2_init(void)
{
	int8_t ret = 0;

	g_uart2.comm_huart = create_comm_huart(UART2_RXBUF_LEN, UART2_RXQUEUE_LEN, 
										  UART2_RXQUEUE_SIZE, UART2_TIMER_TIME, 
										  uart2_phy_timer);
	if (NULL == g_uart2.comm_huart)
		return -EMEM;

	g_uart2.comm_huart->idx = DRIVER_UART2;

	ret = uart2_phy_init(&g_uart2.comm_huart->uart);
	if (0 == ret)
		HAL_UART_Receive_IT(&g_uart2.comm_huart->uart, 
							&g_uart2.recv_data, 1);

	return ret;
}

static __const struct driver uart2 = {
    .idx  = DRIVER_UART2,
    .init = uart2_init,
    .run = uart2_rx,
	.write = uart2_write,
    .ioctl = uart2_ioctrl,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
POSTCORE_INIT(uart2);
#endif
