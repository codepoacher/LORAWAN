#include "uart_driver.h"
#include "stm32l0xx_hal.h"

/*************************************************************************************
* FunctionName   : HAL_UART_MspInit()
* Description    : uart 硬件相关的初始化
* EntryParameter : huart,需要初始化的串口
* ReturnValue    : None
*************************************************************************************/
void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{

	GPIO_InitTypeDef GPIO_InitStruct;
	if (huart->Instance==LPUART1) {
		__HAL_RCC_LPUART1_CLK_ENABLE();

		GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF6_LPUART1;
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

		HAL_NVIC_SetPriority(AES_RNG_LPUART1_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(AES_RNG_LPUART1_IRQn);
	}
	else if(huart->Instance==USART4)
	{
		__HAL_RCC_USART4_CLK_ENABLE();

		GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF6_USART4;
		HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

		HAL_NVIC_SetPriority(USART4_5_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(USART4_5_IRQn);
	}
	else if(huart->Instance==USART5)
	{
		__HAL_RCC_USART5_CLK_ENABLE();

		GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF6_USART5;
		HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

		HAL_NVIC_SetPriority(USART4_5_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(USART4_5_IRQn);
	}
}

/*************************************************************************************
* FunctionName   : HAL_UART_MspDeInit()
* Description    : uart 硬件相关的反初始化
* EntryParameter : huart,需要反初始化的串口
* ReturnValue    : None
*************************************************************************************/
void HAL_UART_MspDeInit(UART_HandleTypeDef* huart)
{

	if (huart->Instance == LPUART1) {

		__HAL_RCC_LPUART1_CLK_DISABLE();

		HAL_GPIO_DeInit(GPIOC, GPIO_PIN_0|GPIO_PIN_1);

		HAL_NVIC_DisableIRQ(AES_RNG_LPUART1_IRQn);
	}
	else if(huart->Instance==USART4)
	{
		__HAL_RCC_USART4_CLK_DISABLE();

		HAL_GPIO_DeInit(GPIOE, GPIO_PIN_8|GPIO_PIN_9);

		HAL_NVIC_DisableIRQ(USART4_5_IRQn);
	}
	else if(huart->Instance==USART5)
	{
		__HAL_RCC_USART5_CLK_DISABLE();

		HAL_GPIO_DeInit(GPIOE, GPIO_PIN_10|GPIO_PIN_11);

		HAL_NVIC_DisableIRQ(USART4_5_IRQn);
	}
}

/*************************************************************************************
* FunctionName   : HAL_UART_RxCpltCallback()
* Description    : uart 中断回调
* EntryParameter : huart,产生中断的串口
* ReturnValue    : None
*************************************************************************************/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == LPUART1) {
		lpuart_data_add();
	} 
	else if (huart->Instance == USART4) {
		uart4_data_add();
	}
	else if (huart->Instance == USART5) {
		uart5_data_add();
	}
}

/*************************************************************************************
* FunctionName   : HAL_UART_TxCpltCallback()
* Description    : uart 中断回调
* EntryParameter : huart,产生中断的串口
* ReturnValue    : None
*************************************************************************************/
//void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
//{
//	if (huart->Instance == USART3)
//		uart3_tx_end();
//}

/*************************************************************************************
* FunctionName   : HAL_UART_ErrorCallback()
* Description    : uart 错误中断回调
* EntryParameter : huart,产生中断的串口
* ReturnValue    : None
*************************************************************************************/
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{

	/*if (huart->Instance == USART3){
		uart3_data_add();
	}*/
}

/*************************************************************************************
* FunctionName   : comm_uart_read()
* Description    : uart的通用读接口
* EntryParameter : comm_uart,通用uart结构指针
* ReturnValue    : 返回错误码
*************************************************************************************/
int32_t comm_uart_read(struct comm_huart *comm_uart)
{
	struct huart_rx_s *qdata = NULL;

	if (comm_uart->rx_queue) {
		while (0 == fqueue_pop(comm_uart->rx_queue, &qdata, 0, true)) {
			if (NULL != comm_uart->rx_handler) 
				comm_uart->rx_handler(comm_uart->idx, qdata->data, qdata->len);
			mem_free(qdata);
		}
	}

	return 0;
}

/*************************************************************************************
* FunctionName   : comm_uart_write()
* Description    : uart的通用轮询写接口
* EntryParameter : comm_uart,通用uart结构指针, data,需要写的数据， len,数据长度
* ReturnValue    : 返回错误码
*************************************************************************************/
int32_t comm_uart_write(struct comm_huart *comm_uart, void *data, int32_t len)
{
	 /* 1.上锁 */
    mutex_lock(comm_uart->tx_mutex);

    /* 2.发送数据 */
	HAL_UART_Transmit(comm_uart->uart, (uint8_t*)data, len, 0xff);

    /* 3.解锁 */
    mutex_unlock(comm_uart->tx_mutex);

	return 0;	
}

/*************************************************************************************
* FunctionName   : comm_uart_write_it()
* Description    : uart的通用中断写接口
* EntryParameter : comm_uart,通用uart结构指针, data,需要写的数据， len,数据长度
* ReturnValue    : 返回错误码
*************************************************************************************/
int32_t comm_uart_write_it(struct comm_huart *comm_uart, void *data, int32_t len)
{
	uint8_t *send_data = NULL;
	uint32_t ret;
	/* 1.上锁 */
    mutex_lock(comm_uart->tx_mutex);

	if (comm_uart->send_buf_cnt + len > UART_TXBUF_LEN)
		comm_uart->send_buf_cnt = 0;
	send_data = comm_uart->send_buf + comm_uart->send_buf_cnt;
	comm_uart->send_buf_cnt += len;	
	memcpy(send_data, data, len);

    /* 2.发送数据 */
	while (1) {
		ret = HAL_UART_Transmit_IT(comm_uart->uart, (uint8_t*)send_data, len);
		if (ret == HAL_OK)
			break;
	}
 
	//while (tx_end != UART_TX_END);

    /* 3.解锁 */
    mutex_unlock(comm_uart->tx_mutex);

	return len;
}

/*************************************************************************************
* FunctionName   : comm_uart_ioctl()
* Description    : uart的通用ioctl接口
* EntryParameter : comm_uart,通用uart结构指针, cmd,控制命令 
*				   args,命令参数, len,命令参数长度
* ReturnValue    : 返回错误码
*************************************************************************************/
int32_t comm_uart_ioctl(struct comm_huart *comm_uart, int32_t cmd, void *args, int32_t len)
{
   if(unlikely((NULL == args && len != 0) ||
        (NULL != args && 0 == len) || len < 0)){
        return -EINVAL;
    }

    /* 1.执行命令 */
    switch(cmd){
    case _IOC_SET_CB:
        if(unlikely(NULL == args))return -EINVAL;
		comm_uart->rx_handler = (ioctl_cb_t)args;
//        rx_handler = (ioctl_cb_t)args;
        break;
    default:
        return -EINVAL;
    }

	return 0;
}

/*************************************************************************************
* FunctionName   : uart_comm_timer_handle()
* Description    : uart的通用定时器处理接口
* EntryParameter : buf_len,需要读取的数据长度, lbuf,环形缓冲区, queue,接收队列
* ReturnValue    : None
*************************************************************************************/
void uart_comm_timer_handle(int buf_len, struct loop_buffer *lbuf, fqueue_t queue)
{
	uint8_t *buf = NULL; //[UART2_RXBUF_LEN] = { 0 };
    uint16_t recv_data_len = 0;
    struct huart_rx_s *qdata = NULL;

	buf = (uint8_t *)mem_malloc(buf_len);
	if (!buf)
		goto NEXT_RECV;
	memset(buf, 0, buf_len);

	/* 1.获取环形缓冲区数据 */
	recv_data_len = loop_buffer_get(lbuf, buf, buf_len);
	
     /* 2.检查是否接收到数据 */
    if(unlikely(0 == recv_data_len)) goto NEXT_RECV;

    /* 3.申请数据存储空间 */
    qdata = mem_malloc(sizeof(struct huart_rx_s) + recv_data_len);
    if(unlikely(NULL == qdata)) goto NEXT_RECV;

    /* 4.填充数据 */
    qdata->len = recv_data_len;
    memcpy(qdata->data, buf, recv_data_len);

    /* 5.将数据添加到队列 */
    if(0 == fqueue_push(queue, &qdata, 0)) goto NEXT_RECV;

    /* 6.释放入队列失败的数据 */
    mem_free(qdata);

    NEXT_RECV:
	if (buf)
		mem_free(buf);
}

/*************************************************************************************
* FunctionName   : destroy_comm_uart()
* Description    : 销毁通用uart结构
* EntryParameter : uart,通用uart结构指针
* ReturnValue    : None
*************************************************************************************/
void destroy_comm_uart(struct comm_huart *uart)
{
	if (uart && uart->tx_mutex)
		vQueueDelete(uart->tx_mutex);
	if (uart && uart->rx_queue)
		vQueueDelete(uart->rx_queue);
	//if (uart && uart->rx_timer)
		//sorftimer_delete(uart->rx_timer);
	if (uart)
		mem_free(uart);

	return;
}

/*************************************************************************************
* FunctionName   : cureate_comm_uart()
* Description    : 创建通用uart结构
* EntryParameter : buf_len,环形缓冲区长度， queue_len,接收队列长度
*				   queue_size,队列元素大小， timer_time,定时器时间 
*				   timer_cb,定时器回调函数
* ReturnValue    : 通用uart结构指针
*************************************************************************************/
struct comm_huart *create_comm_huart(int buf_len, int queue_len, 
									int queue_size, int timer_time, 
									void (*timer_cb)(softimer_t timer))
{
	struct comm_huart *comm_uart = NULL;

	/* 1. 创建comm_huart */
	comm_uart = (struct comm_huart*)mem_malloc(sizeof(struct comm_huart));
	if (!comm_uart)
		return NULL;
	memset(comm_uart, 0, sizeof(struct comm_huart));

	/* 2. 创建锁 */
	comm_uart->tx_mutex = mutex_lock_init();
	if (NULL == comm_uart->tx_mutex)
		goto err;

	/* 3. 创建队列 */
	comm_uart->rx_queue = fqueue_create(queue_len, queue_size);
	if (NULL == comm_uart->rx_queue)
		goto err;

	/* 4. 创建循环buf */
	comm_uart->rx_buf = init_loop_buffer(buf_len);
	if (NULL == comm_uart->rx_buf)
		goto err;

	/* 5. 创建定时器 */
	if (timer_cb) 
	{
		comm_uart->rx_timer = softimer_create(timer_cb, 
										  SOFTIMER_RELOAD_ENABLE, 
										  timer_time == 0 ? 10 : timer_time);
		if (NULL == comm_uart->rx_timer)
			goto err;

		softimer_start(comm_uart->rx_timer, 0);
	}

	/* 6. 创建发送缓冲区 */
	comm_uart->send_buf = (uint8_t *)mem_malloc(UART_TXBUF_LEN);
	if (NULL == comm_uart->send_buf) 
		goto err;

	return comm_uart;

err:
	destroy_comm_uart(comm_uart);
	return NULL;
}


