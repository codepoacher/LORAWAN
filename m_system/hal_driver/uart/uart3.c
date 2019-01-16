/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : uart3.c
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

static struct uart g_uart3;
static HAL_StatusTypeDef g_recv_status = 0;


/*************************************************************************************
* FunctionName   : UART5_IRQHandler()
* Description    : UART5中断
* EntryParameter : None
* ReturnValue    : None
*************************************************************************************/
void USART3_IRQHandler(void)
{
	HAL_UART_IRQHandler(&g_uart3.comm_huart->uart);
}

/*************************************************************************************
* FunctionName   : uart3_data_add()
* Description    : 添加uart3串口数据
* EntryParameter : None
* ReturnValue    : None
*************************************************************************************/
void uart3_data_add(void)
{
	loop_buffer_put(g_uart3.comm_huart->rx_buf, &g_uart3.recv_data, 1);
	g_uart3.recv_data = 0;
	g_uart3.comm_huart->uart.RxState = HAL_UART_STATE_READY;
	g_recv_status = HAL_UART_Receive_IT(&g_uart3.comm_huart->uart, &g_uart3.recv_data, 1);
}

/*************************************************************************************
* FunctionName   : uart3_tx_end()
* Description    : uart3 发送结束
* EntryParameter : None
* ReturnValue    : None
*************************************************************************************/
//void uart3_tx_end() 
//{
//	//g_queue_cnt++;
//	//end_flag = 1;
//}

/*************************************************************************************
* FunctionName   : uart3_phy_init()
* Description    : UART物理初始化
* EntryParameter : None
* ReturnValue    : 返回错误码
*************************************************************************************/
static inline int8_t uart3_phy_init(UART_HandleTypeDef *huart3)
{
    /* 1.uart3 初始化 */
	huart3->Instance					= USART3;
	huart3->Init.BaudRate				= UART3_BAUD;
	huart3->Init.WordLength				= UART_WORDLENGTH_8B;
	huart3->Init.StopBits				= UART_STOPBITS_1;
	huart3->Init.Parity					= UART_PARITY_NONE;
	huart3->Init.Mode					= UART_MODE_TX_RX;
	huart3->Init.HwFlowCtl				= UART_HWCONTROL_NONE;
	huart3->Init.OverSampling			= UART_OVERSAMPLING_16;
	huart3->Init.OneBitSampling			= UART_ONE_BIT_SAMPLE_DISABLE;
	huart3->AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init(huart3) != HAL_OK)
		return -EINVAL;

    return 0;
}

/*************************************************************************************
* FunctionName   : uart3_phy_timer()
* Description    : UART3物理监视定时器
* EntryParameter : None
* ReturnValue    : 返回错误码
*************************************************************************************/
static void uart3_phy_timer(softimer_t timer)
{
	static uint8_t cnt = 0;
	int32_t buf_len = UART3_RXBUF_LEN;
	uint8_t *data = NULL;
	uint8_t *buf = NULL;
    uint16_t recv_data_len = 0;
    struct huart_rx_s *qdata = NULL;

	/* 1. 判断接收道德数据长度 */
	buf_len = loop_buffer_use(g_uart3.comm_huart->rx_buf);
	if (buf_len <= 0 || buf_len > UART3_TXBUF_LEN) 
		goto NEXT_RECV;

	/* 2. 获取接收到的数据 */
	buf = (uint8_t *)mem_malloc(buf_len);
	if (!buf) {
		debug("malloc %d error: %d\r\n", buf_len, xPortGetFreeHeapSize());
		goto NEXT_RECV;
	}
	memset(buf, 0, buf_len);
	recv_data_len = loop_buffer_get(g_uart3.comm_huart->rx_buf, buf, buf_len);

	/* 3. 调用接收回调函数 */
	if (g_uart3.comm_huart->rx_handler) 
		g_uart3.comm_huart->rx_handler(g_uart3.comm_huart->idx, buf, recv_data_len);

	NEXT_RECV:
	if (buf)
		mem_free(buf);
    (void)timer;

#if 0
    /* 3.申请数据存储空间 */
    qdata = mem_malloc(sizeof(struct huart_rx_s) + recv_data_len);
    if(unlikely(NULL == qdata)) goto NEXT_RECV;

    /* 4.填充数据 */
    qdata->len = recv_data_len;
    memcpy(qdata->data, buf, recv_data_len);

    /* 5.将数据添加到队列 */
	if (g_uart3.comm_huart->rx_handler) 
		g_uart3.comm_huart->rx_handler(g_uart3.comm_huart->idx, qdata->data, qdata->len);
    //if(0 == fqueue_push(queue, &qdata, 0)) goto NEXT_RECV;

    /* 6.释放入队列失败的数据 */
    mem_free(qdata);
#endif
}

/*************************************************************************************
* FunctionName   : uart3_write()
* Description    : 写
* EntryParameter : *args,参数, len,参数长度
* ReturnValue    : 返回写入的字节数, 返回错误码
*************************************************************************************/
static int32_t uart3_write(uint8_t idx, void *data, int32_t len)
{
	int32_t ret = 0;
	char *send_data = NULL;

	/* 1. 把数据存入发送缓冲区 */
	/* 2. 使用中断发送数据 */
	ret = comm_uart_write_it(g_uart3.comm_huart, data, len);

	/* 3. 开启接收中断 */
	while (g_recv_status != HAL_OK) {
		g_uart3.recv_data = 0;
		g_uart3.comm_huart->uart.RxState = HAL_UART_STATE_READY;
		g_recv_status = HAL_UART_Receive_IT(&g_uart3.comm_huart->uart, &g_uart3.recv_data, 1);
	}

 	return ret;
}

/*************************************************************************************
* FunctionName   : uart3_ioctrl()
* Description    : 控制
* EntryParameter : *args,参数, len,参数长度
* ReturnValue    : 返回错误码
*************************************************************************************/
static int32_t uart3_ioctrl(uint8_t idx, int32_t cmd,  void *args,int32_t len)
{
	return comm_uart_ioctl(g_uart3.comm_huart, cmd, args, len);
}

/*************************************************************************************
* FunctionName   : uart3_rx()
* Description    : UART驱动任务
* EntryParameter : None
* ReturnValue    : 返回错误码
*************************************************************************************/
static int32_t uart3_rx(void)
{

#if 0
	void *qdata = NULL;
	//int cnt = g_queue_cnt / 4 * 3;
	if (g_queue_cnt > 20)
		cnt = 10;
	else 
		return 0;

	while (cnt-- > 0) {
		while (0 == fqueue_pop(g_uart3.tx_queue, &qdata, 0, true)) {
			mem_free(qdata);
		}
	}
	g_queue_cnt -= cnt;
#endif
	//return comm_uart_read(g_uart3.comm_huart);
	return 0;
}

/*************************************************************************************
* FunctionName   : uart_init()
* Description    : 设备初始化
* EntryParameter : None
* ReturnValue    : 返回错误码
*************************************************************************************/
static int32_t __init uart3_init(void)
{
	int8_t ret = 0;

	/* 1. 创建通用串口结构 */
	g_uart3.comm_huart = create_comm_huart(UART3_RXBUF_LEN, UART3_RXQUEUE_LEN, 
										  UART3_RXQUEUE_SIZE, UART3_TIMER_TIME, 
										  uart3_phy_timer);
	if (NULL == g_uart3.comm_huart)
		return -EMEM;

	/* 2. 设置窗口序号 */
	g_uart3.comm_huart->idx = DRIVER_UART3;

	/* 3. 初始化串口硬件相关 */
	ret = uart3_phy_init(&g_uart3.comm_huart->uart);

	/* 4. 开启接收中断 */
	HAL_UART_Receive_IT(&g_uart3.comm_huart->uart, &g_uart3.recv_data, 1);

	return ret;
}

static __const struct driver uart3 = {
    .idx  = DRIVER_UART3,
    .init = uart3_init,
    .run = uart3_rx,
	.write = uart3_write,
    .ioctl = uart3_ioctrl,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
POSTCORE_INIT(uart3);
#endif
