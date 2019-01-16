#include "can_driver.h"
#include "frtos_drivers.h"
#include "frtos_errno.h"
#include "config_driver.h"
#include "frtos_softimer.h"
#include "frtos_mem.h"
#include "frtos_queue.h"
#include "frtos_lock.h"
#include "frtos_delay.h"
#include "gpio_driver.h"
#include "loopbuf.h"

#if  0
#define CAN_FILTER_MAX  14

struct can_recv_data_s {
	CAN_RxHeaderTypeDef header;
	uint8_t data[8];
};

CAN_HandleTypeDef hcan;
static mutex_lock_t can_mutex = NULL;
static fqueue_t can_rxqueue = NULL;
static ioctl_cb_t can_phy_rx_handler = NULL;
static ioctl_cb_t can_suspend_handler = NULL;
static ioctl_cb_t can_wakeup_handler = NULL;
static softimer_t can_rxtimer = NULL;
static struct loop_buffer *rx_buf = NULL;	  /* 串口数据接收缓冲区 */

static uint8_t g_filter_num = 0;

/**************************************************************************************
* FunctionName   : USB_LP_CAN_RX0_IRQHandler(void)
* Description    : can 接收中断函数
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
void CAN1_RX0_IRQHandler(void)
{
	HAL_CAN_IRQHandler(&hcan);
}

void CAN1_RX1_IRQHandler(void)
{
	HAL_CAN_IRQHandler(&hcan);
}


/**************************************************************************************
* FunctionName   : can_recv_cb
* Description    : can 接收中断回调函数
* EntryParameter : recv_buf,can接收的数据
* ReturnValue    : None
**************************************************************************************/
void can_recv_cb(CAN_RxHeaderTypeDef *header, uint8_t *data)
{
	if (!header)
		return;
	loop_buffer_put(rx_buf, (unsigned char*)header, 
					sizeof(CAN_RxHeaderTypeDef));
	loop_buffer_put(rx_buf, data, 8);
}

/**************************************************************************************
* FunctionName   : can_phy_init()
* Description    : can 硬件初始化
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static uint32_t can_phy_init(void)
{
	hcan.Instance							= CAN;	
	hcan.Init.Prescaler						= CAN_PRESCALER;
	hcan.Init.Mode							= CAN_MODE_NORMAL;
	hcan.Init.SyncJumpWidth					= CAN_SJW_1TQ;
	hcan.Init.TimeSeg1						= CAN_BS1_7TQ;
	hcan.Init.TimeSeg2						= CAN_BS2_8TQ;
	//hcan.Init.SyncJumpWidth					= CAN_SJW_1TQ;
	//hcan.Init.TimeSeg1						= CAN_BS1_11TQ;
	//hcan.Init.TimeSeg2						= CAN_BS2_6TQ;
	hcan.Init.TimeTriggeredMode				= DISABLE;
  	hcan.Init.AutoBusOff					= DISABLE;
  	hcan.Init.AutoWakeUp					= DISABLE;
    hcan.Init.ReceiveFifoLocked				= DISABLE; 
  	hcan.Init.TransmitFifoPriority 			= DISABLE;
	if (HAL_CAN_Init(&hcan) != HAL_OK)
	{
		return -EINVAL;
	}

	return 0;
}

/*************************************************************************************
* FunctionName   : can_phy_rxtimer()
* Description    : 物理接收定时器
* EntryParameter : timer,定时器
* ReturnValue    : None
*************************************************************************************/
static void can_phy_rxtimer(softimer_t timer)
{
#if 1
	int recv_len = 0, d_len = 0;
	uint8_t data[8] = { 0 };
	//CAN_RxHeaderTypeDef *msg = NULL;
	HAL_StatusTypeDef status = HAL_OK;
	struct can_recv_data_s *msg;
    struct can_recv_s *recvbuf = NULL;

    /* 1.接收数据 */
	/*status = HAL_CAN_Receive(&hcan, CAN_FIFO0, 0x0);
	if (status != HAL_OK)
		return;*/
	msg = (struct can_recv_data_s *)mem_malloc(sizeof(struct can_recv_data_s));
	if (!msg)
		goto RETURN_1;
	memset(msg, 0, sizeof(*msg));
	
	recv_len = loop_buffer_get(rx_buf, (unsigned char*)msg, 
								sizeof(struct can_recv_data_s));
	if (unlikely(recv_len != sizeof(*msg))) 
		goto RETURN_1;
	d_len = msg->header.DLC;

	/* 2.创建can接收结构 */
	recvbuf = can_recv_create(DRIVER_CAN0, &msg->header, msg->data);
	if (NULL == recvbuf)
		goto RETURN_1;


	/* 3.调用接收回调 */
	if (NULL != can_phy_rx_handler) 
		can_phy_rx_handler(DRIVER_CAN0, &recvbuf->msg, sizeof(struct can_msg_s));

    /* 3.添加到队列 */
    /*if(0 == fqueue_push(can_rxqueue, &recvbuf, 0))
        goto RETURN_1;*/

    /* 4.释放发送失败的消息 */
    mem_free(recvbuf);

    RETURN_1:
	if (msg)
		mem_free(msg);
#endif

    (void)timer;
}

/**************************************************************************************
* FunctionName   : can_init()
* Description    : can 初始化
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t __init can_init(void)
{
	uint32_t ret = 0;

	/* 1.初始化物理设备 */
	ret = can_phy_init();
	if (ret != 0)
		return ret;

	/* 2.创建TX_MUTEX */
	can_mutex = mutex_lock_init();
	if (NULL == can_mutex) {
		ret = -EMEM;
		goto err;
	}

	/* 3.创建rx接收队列 */
	can_rxqueue = fqueue_create(CAN_QUEUE_LEN, 
								   sizeof(struct can_recv_s *));
	if (!can_rxqueue) {
		ret = -EMEM;
		goto err;
	}
	rx_buf = init_loop_buffer(CAN_BUFFER_SIZE);
	if (!rx_buf) {
		ret = -EMEM;
		goto err;
	}

   /* 4.创建数据接收定时器 */
    can_rxtimer = softimer_create(can_phy_rxtimer,
        SOFTIMER_RELOAD_ENABLE, 1);
    if(NULL == can_rxtimer) return -EMEM;

    /* 5.启动接收定时器 */
    softimer_start(can_rxtimer, 0);
	
	/* 6.设置can芯片位高速模式 */
	gpio_comm_init(CAN_STB_GPIO, CAN_STB_PIN, 
				   GPIO_SPEED_FREQ_LOW, 
				   GPIO_MODE_OUTPUT_PP);
	CAN_STB_HIGH_SET();

	HAL_CAN_Start(&hcan);

	return 0;
err:
	if (can_mutex)
		vSemaphoreDelete(can_mutex);
	if (can_rxqueue)
		vQueueDelete(can_rxqueue);
	if (rx_buf)
		destroy_loop_buffer(rx_buf);

	return ret;
}

/**************************************************************************************
* FunctionName   : can_rx()
* Description    : can 接收数据
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t can_rx(void)
{
	/*struct can_recv_s *recv_buf = NULL;

	while (0 == fqueue_pop(can_rxqueue, &recv_buf, 0, true)) {
		if (NULL != can_phy_rx_handler) 
			can_phy_rx_handler(DRIVER_CAN0, &recv_buf->msg, sizeof(struct can_msg_s));
		mem_free(recv_buf);
	}*/

	return 0;
}

/*************************************************************************************
* FunctionName   : can_send_init()
* Description    : 初始化发送的can数据
* EntryParameter : msg,can消息
* ReturnValue    : 返回发送的数据长度
*************************************************************************************/
static int16_t can_send_init(struct can_msg_s *msg, CAN_TxHeaderTypeDef *header)
{
	if (CAN_STDID == msg->id_type) {
		header->IDE = CAN_ID_STD;
		header->StdId = msg->msgid;
	}
	else if (CAN_EXTID == msg->id_type) {
		header->IDE = CAN_ID_EXT;
		header->ExtId = msg->msgid;
	}
	else 
		return -EINVAL;

	if (CAN_DATA == msg->msg_type)
		header->RTR = CAN_RTR_DATA;
	else if (CAN_REMOTE == msg->msg_type)
		header->RTR = CAN_RTR_REMOTE;
	else 
		return -EINVAL;

	if (unlikely(msg->len > 8 || msg->len <= 0)) 
		return -EINVAL;

	header->DLC = msg->len;

	return 0;
}


/*************************************************************************************
* FunctionName   : can_phy_send()
* Description    : 设备物理发送
* EntryParameter : msgid,消息ID, *data,发送数据指针, len,发送数据长度(<=8)
* ReturnValue    : 返回发送的数据长度
*************************************************************************************/
static int16_t can_phy_send(struct can_msg_s *msg)
{
	int16_t ret = 0;
	uint32_t tx_box = 0;
	CAN_TxHeaderTypeDef header;
	HAL_StatusTypeDef status;
 
    /* 1.上锁 */
    mutex_lock(can_mutex);

    /* 2.配置发送消息邮箱 */
	memset(&header, 0, sizeof(header));
	ret = can_send_init(msg, &header);
	if (ret != 0)
		goto err;

	/* 3. 发送数据 */
	//status = HAL_CAN_Transmit(&hcan, 0xff);
	status = HAL_CAN_AddTxMessage(&hcan, &header, msg->data, &tx_box);
	if (HAL_OK != status) {
		ret = -EIO;
		goto err;
	}

    /* 4.发送延时 */
    frtos_delay_ms(CAN_SEND_OVERTIME);
	
	ret = msg->len;
err:
    /* 5.解锁 */
    mutex_unlock(can_mutex);

    return ret;
}

/*************************************************************************************
* FunctionName   : can_write()
* Description    : 写can数据
* EntryParameter : idx,设备编号, data,写的数据, len,数据长度
* ReturnValue    : 返回写入的字节数, 返回错误码
*************************************************************************************/
static int32_t can_write(uint8_t idx, void *data, int32_t len)
{
    uint16_t i = 0;
    struct can_msg_s *msgbuf = (struct can_msg_s *)data;

    if(unlikely(NULL == data || len <= 0 ||
        0 != (len % sizeof(struct can_msg_s)))){
        return -EINVAL;
    }

    /* 1.循环发送数据 */
    for(i = 0; i < (len / sizeof(struct can_msg_s)); i++){
        if(msgbuf[i].len != can_phy_send(&msgbuf[i]))
            return -EREMOTEIO;
    }

    (void)idx;
    return len;
}

/*************************************************************************************
* FunctionName   : __can_id_16bit_filter()
* Description    : 设备16位can id 过滤配置
* EntryParameter : filter_num, id1, id2, id3, id4 
* ReturnValue    : 返回错误码
*************************************************************************************/
static int16_t __can_id_16bit_filter(uint8_t filter_num, uint16_t id1, uint16_t id2, \
									 uint16_t id3, uint16_t id4)
{
	CAN_FilterTypeDef filter;

	memset(&filter, 0, sizeof(filter));
	filter.FilterBank = filter_num;
	filter.FilterIdHigh = id1 << 5;
	filter.FilterIdLow  = id2 << 5;
	filter.FilterMaskIdHigh = id3 << 5;
	filter.FilterMaskIdLow  = id4 << 5;
	filter.FilterScale = CAN_FILTERSCALE_16BIT;
	filter.FilterMode  = CAN_FILTERMODE_IDLIST;
	filter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
	filter.FilterActivation = ENABLE;
	filter.SlaveStartFilterBank = 0;
	if (HAL_CAN_ConfigFilter(&hcan, &filter) != HAL_OK)
    {
		return -EINVAL;
    }

	return 0;
}

/*************************************************************************************
* FunctionName   : can_id_16bit_filter()
* Description    : 设备16位can id 过滤配置
* EntryParameter : filter,过滤id结构
* ReturnValue    : 返回错误码
*************************************************************************************/
static int8_t can_id_16bit_filter(struct can_filter_s *filter)
{
	uint8_t i = 0;	
	uint8_t step = 4;
#define ID_SELECT(num, off, cnt) \
		(num + off) == cnt ? filter->can_id[num] : filter->can_id[num + off]

	for (i; i < filter->cnt && g_filter_num < CAN_FILTER_MAX; g_filter_num++) {
		if (filter->cnt - i < step) {
			__can_id_16bit_filter(
					g_filter_num, 
					filter->can_id[i], 
					ID_SELECT(i, 1, filter->cnt),
					ID_SELECT(i, 2, filter->cnt),
					ID_SELECT(i, 3, filter->cnt));
		} else {
			__can_id_16bit_filter( 
					g_filter_num, 
					filter->can_id[i], 
					filter->can_id[i + 1], 
					filter->can_id[i + 2], 
					filter->can_id[i + 3]);
		}
		i += step;		
	}

	return 0;
}

/*************************************************************************************
* FunctionName   : __can_id_32bit_filter()
* Description    : 设置32位的can id 过滤
* EntryParameter : filter_num,过滤块号, id_type,id类型, 
*				   can_id1,第一个id号， can_id2,第二个id号
* ReturnValue    : 返回错误码
*************************************************************************************/
static int8_t __can_id_32bit_filter(
							uint8_t filter_num,
							uint8_t  id_type,
							uint32_t can_id1,
							uint32_t can_id2)
{
	CAN_FilterTypeDef filter;

	memset(&filter, 0, sizeof(filter));
	filter.FilterBank = filter_num;
	filter.FilterIdHigh = can_id1 << 5;
	filter.FilterIdLow  = 0 | id_type;
	filter.FilterMaskIdHigh = can_id2 << 5;
	filter.FilterMaskIdLow  = 0 | id_type;
	filter.FilterScale = CAN_FILTERSCALE_32BIT;
	filter.FilterMode  = CAN_FILTERMODE_IDLIST;
	filter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
	filter.FilterActivation = ENABLE;
	filter.SlaveStartFilterBank = 0;
	if (HAL_CAN_ConfigFilter(&hcan, &filter) != HAL_OK)
    {
		return -EINVAL;
    }
	return 0;
}

static int8_t can_id_32bit_filter(struct can_filter_s *filter)
{
	uint8_t i = 0;	
	uint8_t step = 2;
	uint8_t id_type = filter->id_type == CAN_STDID ? CAN_ID_STD : CAN_ID_EXT;

	for (; i < filter->cnt && g_filter_num < CAN_FILTER_MAX; g_filter_num++) {
		if (filter->cnt - i < step) {
			__can_id_32bit_filter(
							g_filter_num, 
							id_type,
							filter->can_id[i],
							filter->can_id[i]);
		} else {
			__can_id_32bit_filter(
							g_filter_num, 
							id_type,
							filter->can_id[i], 
							filter->can_id[i+1]);
		}
		i += step;
	}

	return 0;
}

/*************************************************************************************
* FunctionName   : can_phy_setrxfilter()
* Description    : 设备物理屏蔽字
* EntryParameter : mask,屏蔽字
* ReturnValue    : 返回错误码
*************************************************************************************/
static int8_t can_phy_setrxfilter(struct can_filter_s *filter)
{
	int8_t ret = 0;

	if (unlikely(filter->cnt == 0))
		return -EINVAL;
      /* 1.上锁 */
    mutex_lock(can_mutex);

	/* 2.配置过滤id */
	if (CAN_ID_16BIT == filter->bit_type)
		ret = can_id_16bit_filter(filter);
	else if (CAN_ID_32BIT == filter->bit_type){
			can_id_32bit_filter(filter);
	} else
		return -EINVAL;

    /* 3.解锁 */
    mutex_unlock(can_mutex);

    return 0;
}

static void can_cb_set(void *args)
{
	struct can_cb_s *cbs = (struct can_cb_s*)args;	

	switch (cbs->cb_type) {
		case CAN_SUSPEND:
			can_suspend_handler = cbs->cb;
			break;
		case CAN_WAKEUP:
			can_suspend_handler = cbs->cb;
			break;
		case CAN_RECEIVE:
			can_phy_rx_handler = cbs->cb;
			break;
		default:
			break;
	}
}

/*************************************************************************************
* FunctionName   : can_ioctrl()
* Description    : 控制
* EntryParameter : *args,参数, len,参数长度
* ReturnValue    : 返回错误码
*************************************************************************************/
static int32_t can_ioctrl(uint8_t idx, int32_t cmd, void *args, int32_t len)
{
    struct can_wakeup_t *wakeup = (void *)args;
    if(unlikely((NULL == args && len != 0) ||
        (NULL != args && 0 == len) || len < 0)){
        return -EINVAL;
    }

    /* 1.执行命令 */
    switch(cmd){
    case _IOC_SET_CB:
        if(unlikely(NULL == args)) return -EINVAL;
        if(unlikely(NULL != can_phy_rx_handler)){
            return -EBUSY;
        }
		can_cb_set(args);
        //can_phy_rx_handler = (ioctl_cb_t)args;

        break;
    case _IOC_SET_FILTER:
        if(unlikely(NULL == args)) return -EINVAL;
        can_phy_setrxfilter(((struct can_filter_s *)args));
		if (can_rxtimer) {
			softimer_start(can_rxtimer, 0);
			HAL_CAN_ActivateNotification(&hcan, CAN_INTERRUPT);
			//HAL_CAN_Receive_IT(&hcan, CAN_FIFO0);
		}
	
        break;
    /*case _IOC_WAKEUP:
        can_pncfg.idFilter1.id = wakeup->msgid;
        can_pncfg.idFilter2.id = wakeup->mask;
        FLEXCAN_DRV_ConfigPN(0, true, &can_pncfg);
        INT_SYS_SetPriority(CAN0_Wake_Up_IRQn, CAN0_IRQPRIO);
        break;*/
    default:
        return -EINVAL;
    }

    (void)idx;
    return 0;
}

static __const struct driver can = {
	.idx	= DRIVER_CAN0,
	.init	= can_init,
	.run	= can_rx,
	.write	= can_write,
	.ioctl	= can_ioctrl,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
//POSTCORE_INIT(can);
#endif
