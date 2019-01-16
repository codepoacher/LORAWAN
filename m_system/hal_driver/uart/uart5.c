/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : uart5.c
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
static struct uart g_uart5;
UART_HandleTypeDef g_huart5_handle;

/*************************************************************************************
* FunctionName   : UART4_5_IRQHandler()
* Description    : UART4、5中断
* EntryParameter : None
* ReturnValue    : None
*************************************************************************************/
//void USART4_5_IRQHandler(void)
//{
//	HAL_UART_IRQHandler(g_uart5.comm_huart->uart);
//	HAL_UART_IRQHandler(&huart5);
//}

/*************************************************************************************
* FunctionName   : uart5_data_add()
* Description    : 添加uart5串口数据
* EntryParameter : None
* ReturnValue    : None
*************************************************************************************/
void uart5_data_add(void)
{
	loop_buffer_put(g_uart5.comm_huart->rx_buf, &g_uart5.recv_data, 1);
	HAL_UART_Receive_IT(g_uart5.comm_huart->uart, &g_uart5.recv_data, 1);
}


/*************************************************************************************
* FunctionName   : uart5_phy_init()
* Description    : UART物理初始化
* EntryParameter : None
* ReturnValue    : 返回错误码
*************************************************************************************/
static inline int8_t uart5_phy_init(UART_HandleTypeDef *huart5)
{
    /* 1.uart5 初始化 */
	huart5->Instance					= USART5;
	huart5->Init.BaudRate				= UART5_BAUD;
	huart5->Init.WordLength				= UART_WORDLENGTH_8B;
	huart5->Init.StopBits				= UART_STOPBITS_1;
	huart5->Init.Parity					= UART_PARITY_NONE;
	huart5->Init.Mode					= UART_MODE_TX_RX;
	huart5->Init.HwFlowCtl				= UART_HWCONTROL_NONE;
	huart5->Init.OverSampling			= UART_OVERSAMPLING_16;
	huart5->Init.OneBitSampling			= UART_ONE_BIT_SAMPLE_DISABLE;
	huart5->AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init(huart5) != HAL_OK)
		return -EINVAL;

    return 0;
}

/*************************************************************************************
* FunctionName   : uart5_suspend()
* Description    : UART唤醒后恢复引脚
* EntryParameter : None
* ReturnValue    : 返回错误码
*************************************************************************************/
static int32_t uart5_suspend(void)
{
	HAL_UART_DeInit(&g_huart5_handle);

	return 0;
}

/*************************************************************************************
* FunctionName   : uart5_wakeup()
* Description    : UART唤醒后恢复引脚
* EntryParameter : None
* ReturnValue    : 返回错误码
*************************************************************************************/
static int32_t uart5_wakeup(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	/*打开3.3V 485电源*/
	GPIO_COMM_INIT(GPIO(A,15), GPIO_SPEED_FREQ_LOW, GPIO_MODE_OUTPUT_PP);
	GPIO_COMM_WRITE(GPIO(A,15), 1);

	uart5_phy_init(&g_huart5_handle);
	HAL_UART_Receive_IT(&g_huart5_handle, &g_uart5.recv_data, 1);

	return 0;
}
/*************************************************************************************
* FunctionName   : uart5_phy_timer()
* Description    : UART5物理监视定时器
* EntryParameter : None
* ReturnValue    : 返回错误码
*************************************************************************************/
static void uart5_phy_timer(softimer_t timer)
{
	uart_comm_timer_handle(UART5_RXBUF_LEN, 
							  g_uart5.comm_huart->rx_buf, 
							  g_uart5.comm_huart->rx_queue);
    (void)timer;
}

/*************************************************************************************
* FunctionName   : uart5_write()
* Description    : 写
* EntryParameter : *args,参数, len,参数长度
* ReturnValue    : 返回写入的字节数, 返回错误码
*************************************************************************************/
static int32_t uart5_write(uint8_t idx, void *data, int32_t len)
{
	return comm_uart_write_it(g_uart5.comm_huart, data, len);
}

/*************************************************************************************
* FunctionName   : uart5_ioctrl()
* Description    : 控制
* EntryParameter : *args,参数, len,参数长度
* ReturnValue    : 返回错误码
*************************************************************************************/
static int32_t uart5_ioctrl(uint8_t idx, int32_t cmd,  void *args,int32_t len)
{
	return comm_uart_ioctl(g_uart5.comm_huart, cmd, args, len);
}

/*************************************************************************************
* FunctionName   : uart5_rx()
* Description    : UART驱动任务
* EntryParameter : None
* ReturnValue    : 返回错误码
*************************************************************************************/
static int32_t uart5_rx(void)
{
	return comm_uart_read(g_uart5.comm_huart);
}

/*************************************************************************************
* FunctionName   : uart_init()
* Description    : 设备初始化
* EntryParameter : None
* ReturnValue    : 返回错误码
*************************************************************************************/
static int32_t __init uart5_init(void)
{
	int8_t ret = 0;

	/*打开3.3V 485电源*/
	GPIO_COMM_INIT(GPIO(A,15), GPIO_SPEED_FREQ_LOW, GPIO_MODE_OUTPUT_PP);
	GPIO_COMM_WRITE(GPIO(A,15), 1);

	memset(&g_uart5, 0, sizeof(g_uart5));
	g_uart5.comm_huart = create_comm_huart(UART5_RXBUF_LEN, UART5_RXQUEUE_LEN, 
										  UART5_RXQUEUE_SIZE, UART5_TIMER_TIME, 
										  uart5_phy_timer);
	if (NULL == g_uart5.comm_huart)
		return -EMEM;

	g_uart5.comm_huart->idx = DRIVER_UART5;

	ret = uart5_phy_init(&g_huart5_handle);
	g_uart5.comm_huart->uart = &g_huart5_handle;
	HAL_UART_Receive_IT(g_uart5.comm_huart->uart, &g_uart5.recv_data, 1);

	return ret;
}

static __const struct driver uart5 = {
    .idx  = DRIVER_UART5,
    .init = uart5_init,
    .run = uart5_rx,
	.write = uart5_write,
    .ioctl = uart5_ioctrl,
	.wakeup = uart5_wakeup,
	.suspend = uart5_suspend,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
FSCORE_INIT(uart5);
#endif
