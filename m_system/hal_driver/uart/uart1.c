/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : uart1.c
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
static struct uart g_uart1;
static UART_HandleTypeDef g_uart1_handle;

/*************************************************************************************
* FunctionName   : UART5_IRQHandler()
* Description    : UART5中断
* EntryParameter : None
* ReturnValue    : None
*************************************************************************************/
void USART1_IRQHandler(void)
{
	HAL_UART_IRQHandler(g_uart1.comm_huart->uart);
}

/*************************************************************************************
* FunctionName   : uart1_data_add()
* Description    : 添加uart1串口数据
* EntryParameter : None
* ReturnValue    : None
*************************************************************************************/
void uart1_data_add(void)
{
	loop_buffer_put(g_uart1.comm_huart->rx_buf, &g_uart1.recv_data, 1);
	HAL_UART_Receive_IT(g_uart1.comm_huart->uart, &g_uart1.recv_data, 1);
}

/************************************************************************************
 * FunctionName  : uart1_IT_ENABLE
 * Description   : 
 * EntryParameter: None
 * ReturnValue   : None
*************************************************************************************/ 
void uart1_IT_ENABLE(void)
{
	__HAL_UART_ENABLE_IT(g_uart1.comm_huart->uart, UART_IT_RXNE);
	HAL_UART_Receive_IT(g_uart1.comm_huart->uart, &g_uart1.recv_data, 1);
}

/************************************************************************************
 *
 *
 *
 *
*************************************************************************************/
void uart1_buffer(void)
{
	g_uart1.comm_huart->rx_buf = init_loop_buffer(512);
}

/*************************************************************************************
* FunctionName   : uart1_phy_init()
* Description    : UART物理初始化
* EntryParameter : None
* ReturnValue    : 返回错误码
*************************************************************************************/
static inline int8_t uart1_phy_init(UART_HandleTypeDef *huart1)
{
	/* 1.uart1 初始化 */
	huart1->Instance					= USART1;
	huart1->Init.BaudRate				= UART1_BAUD;
	huart1->Init.WordLength				= UART_WORDLENGTH_8B;
	huart1->Init.StopBits				= UART_STOPBITS_1;
	huart1->Init.Parity					= UART_PARITY_NONE;
	huart1->Init.Mode					= UART_MODE_TX_RX;
	huart1->Init.HwFlowCtl				= UART_HWCONTROL_NONE;
	huart1->Init.OverSampling			= UART_OVERSAMPLING_16;
	huart1->Init.OneBitSampling			= UART_ONE_BIT_SAMPLE_DISABLE;
	huart1->AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init(huart1) != HAL_OK)
		return -EINVAL;

    return 0;
}

/*************************************************************************************
* FunctionName   : uart1_phy_timer()
* Description    : UART1物理监视定时器
* EntryParameter : None
* ReturnValue    : 返回错误码
*************************************************************************************/
static void uart1_phy_timer(softimer_t timer)
{
	uart_comm_timer_handle(UART1_RXBUF_LEN, 
							  g_uart1.comm_huart->rx_buf, 
							  g_uart1.comm_huart->rx_queue);
    (void)timer;
}

/*************************************************************************************
* FunctionName   : uart1_write()
* Description    : 写
* EntryParameter : *args,参数, len,参数长度
* ReturnValue    : 返回写入的字节数, 返回错误码
*************************************************************************************/
static int32_t uart1_write(uint8_t idx, void *data, int32_t len)
{
	return comm_uart_write(g_uart1.comm_huart, data, len);
}

/*************************************************************************************
* FunctionName   : uart1_ioctrl()
* Description    : 控制
* EntryParameter : *args,参数, len,参数长度
* ReturnValue    : 返回错误码
*************************************************************************************/
static int32_t uart1_ioctrl(uint8_t idx, int32_t cmd,  void *args,int32_t len)
{
	return comm_uart_ioctl(g_uart1.comm_huart, cmd, args, len);
}

/*************************************************************************************
* FunctionName   : uart1_rx()
* Description    : UART驱动任务
* EntryParameter : None
* ReturnValue    : 返回错误码
*************************************************************************************/
int32_t uart1_rx(void)
{
	return comm_uart_read(g_uart1.comm_huart);
}

/*************************************************************************************
* FunctionName   : uart_init()
* Description    : 设备初始化
* EntryParameter : None
* ReturnValue    : 返回错误码
*************************************************************************************/
static int32_t __init uart1_init(void)
{
	int8_t ret = 0;

	g_uart1.comm_huart = create_comm_huart(UART1_RXBUF_LEN, UART1_RXQUEUE_LEN, 
										  UART1_RXQUEUE_SIZE, UART1_TIMER_TIME, 
										  uart1_phy_timer);
	if (NULL == g_uart1.comm_huart)
		return -EMEM;

	g_uart1.comm_huart->idx = DRIVER_UART1;

	ret = uart1_phy_init(&g_uart1_handle);
	g_uart1.comm_huart->uart = &g_uart1_handle;
	if (0 == ret)
		HAL_UART_Receive_IT(g_uart1.comm_huart->uart, 
							&g_uart1.recv_data, 1);

	return ret;
}

static __const struct driver uart1 = {
    .idx  = DRIVER_UART1,
    .init = uart1_init,
    .run = uart1_rx,
	.write = uart1_write,
    .ioctl = uart1_ioctrl,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
//POSTCORE_INIT(uart1);
#endif
