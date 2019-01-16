#if 0
#include "can_driver.h"
#include "frtos_mem.h"
#include "config_driver.h"


static void HAL_CAN_Comm_Receive(CAN_HandleTypeDef *hcan, uint32_t fifo,
								 void (*cb)(CAN_RxHeaderTypeDef *head, uint8_t *data));

/**************************************************************************************
* FunctionName   : can_recv_create()
* Description    : 根据CAN_RxHeaderTypeDef结构获取can消息信息
* EntryParameter : idx,can的id号, header,CAN_RxHeaderTypeDef结构指针 
*				   data,接收到的数据
* ReturnValue    : 成功返回can_recv_s结构指针，失败返回NULL
**************************************************************************************/
struct can_recv_s *can_recv_create(uint8_t idx, 
									  CAN_RxHeaderTypeDef *header, 
									  uint8_t *data)
{
	struct can_recv_s *recv_buf = NULL;

	if (!header)
		return NULL;

	recv_buf = mem_malloc(sizeof(struct can_recv_s));
	if (!recv_buf) 
		return NULL;
	memset(recv_buf, 0, sizeof(struct can_recv_s));
	recv_buf->idx = idx;
	if (header->IDE == CAN_ID_STD) {
		recv_buf->msg.msgid = header->StdId;
		recv_buf->msg.id_type = CAN_STDID;
	} else {
		recv_buf->msg.msgid = header->ExtId;
		recv_buf->msg.id_type = CAN_EXTID;
	}
	if (header->RTR = CAN_RTR_DATA)
		recv_buf->msg.msg_type = CAN_DATA;
	else
		recv_buf->msg.msg_type = CAN_REMOTE;
	recv_buf->msg.len = header->DLC;
	if (header->DLC > 0)
		memcpy(recv_buf->msg.data, data, header->DLC);

	return recv_buf;
}

/**************************************************************************************
* FunctionName   : HAL_CAN_MspInit()
* Description    : 初始化can的gpio引脚，注册can中断
* EntryParameter : hcan,CAN_HandleTypeDef结构指针
* ReturnValue    : None
**************************************************************************************/
void HAL_CAN_MspInit(CAN_HandleTypeDef* hcan)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  if(hcan->Instance==CAN1)
  {
	/* USER CODE BEGIN CAN1_MspInit 0 */

	/* USER CODE END CAN1_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_CAN1_CLK_ENABLE();
  
    /**CAN1 GPIO Configuration    
    PA11     ------> CAN1_RX
    PA12     ------> CAN1_TX 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_CAN1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* CAN1 interrupt Init */
    HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
    HAL_NVIC_SetPriority(CAN1_RX1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(CAN1_RX1_IRQn);
	/* USER CODE BEGIN CAN1_MspInit 1 */

	/* USER CODE END CAN1_MspInit 1 */
  }
}

/**************************************************************************************
* FunctionName   : HAL_CAN_MspDeInit()
* Description    : 反初始化can的gpio引脚，注销的can中断
* EntryParameter : hcan,CAN_HandleTypeDef结构指针
* ReturnValue    : None
**************************************************************************************/
void HAL_CAN_MspDeInit(CAN_HandleTypeDef* hcan)
{
	if(hcan->Instance==CAN1)
  	{
  	/* USER CODE BEGIN CAN1_MspDeInit 0 */

  	/* USER CODE END CAN1_MspDeInit 0 */
  	  /* Peripheral clock disable */
  	  __HAL_RCC_CAN1_CLK_DISABLE();
  	
  	  /**CAN1 GPIO Configuration    
  	  PA11     ------> CAN1_RX
  	  PA12     ------> CAN1_TX 
  	  */
  	  HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11|GPIO_PIN_12);

  	  /* CAN1 interrupt DeInit */
  	  HAL_NVIC_DisableIRQ(CAN1_RX0_IRQn);
  	  HAL_NVIC_DisableIRQ(CAN1_RX1_IRQn);
  	/* USER CODE BEGIN CAN1_MspDeInit 1 */

  	/* USER CODE END CAN1_MspDeInit 1 */
  	}
}

/**************************************************************************************
* FunctionName   : HAL_CAN_Comm_Receive()
* Description    : can中断中的公用接收方法
* EntryParameter : hcan,CAN_HandleTypeDef结构指针, fifo,can接收队列号
*				   cb,can中断回调函数
* ReturnValue    : None
**************************************************************************************/
static void HAL_CAN_Comm_Receive(CAN_HandleTypeDef *hcan, uint32_t fifo,
								 void (*cb)(CAN_RxHeaderTypeDef *head, uint8_t *data)) 
{
	uint8_t i = 0, num = 0;
	uint8_t data[8];
	CAN_RxHeaderTypeDef header;
	HAL_StatusTypeDef status = HAL_OK;
	struct can_recv_s *recv_buf = NULL;

	num = HAL_CAN_GetRxFifoFillLevel(hcan, fifo);

	for (i = 0; i < num; i++) {
		memset(&header, 0, sizeof(header));
		status = HAL_CAN_GetRxMessage(hcan, fifo, &header, data);
		if (status == HAL_OK) {
			cb(&header, data);
			//recv_buf = can_recv_create(DRIVER_CAN0, &header, data);
		//	if (recv_buf) {
		//		else
		//			mem_free(recv_buf);
		//	}
		}
	}
}

/**************************************************************************************
* FunctionName   : HAL_CAN_RxFifo0MsgPendingCallback()
* Description    : can的fifo0接收中断
* EntryParameter : hcan,CAN_HandleTypeDef结构指针
* ReturnValue    : None
**************************************************************************************/
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	if (hcan->Instance == CAN)
		HAL_CAN_Comm_Receive(hcan, CAN_RX_FIFO0, can_recv_cb);
}

/**************************************************************************************
* FunctionName   : HAL_CAN_RxFifo0FullCallback()
* Description    : can的fifo0接收队列满中断回调
* EntryParameter : hcan,CAN_HandleTypeDef结构指针
* ReturnValue    : None
**************************************************************************************/
void HAL_CAN_RxFifo0FullCallback(CAN_HandleTypeDef *hcan)
{
	if (hcan->Instance == CAN) {
		//1.关闭所有can中断
		HAL_CAN_DeactivateNotification(hcan, CAN_INTERRUPT);
		//2.接收缓冲区数据
		HAL_CAN_Comm_Receive(hcan, CAN_RX_FIFO0, can_recv_cb);
		//3.开启所有can中断
		HAL_CAN_ActivateNotification(hcan, CAN_INTERRUPT);
	}
}

/**************************************************************************************
* FunctionName   : HAL_CAN_RxFifo1MsgPendingCallback()
* Description    : can的fifo0接收中断回调
* EntryParameter : hcan,CAN_HandleTypeDef结构指针
* ReturnValue    : None
**************************************************************************************/
void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	if (hcan->Instance == CAN)
		HAL_CAN_Comm_Receive(hcan, CAN_RX_FIFO1, can_recv_cb);
}

/**************************************************************************************
* FunctionName   : HAL_CAN_RxFifo1MsgPendingCallback()
* Description    : can的fifo1接收队列满中断回调
* EntryParameter : hcan,CAN_HandleTypeDef结构指针
* ReturnValue    : None
**************************************************************************************/
void HAL_CAN_RxFifo1FullCallback(CAN_HandleTypeDef *hcan)
{
	if (hcan->Instance == CAN) {
		//1.关闭所有can中断
		HAL_CAN_DeactivateNotification(hcan, CAN_INTERRUPT);
		//2.接收缓冲区数据
		HAL_CAN_Comm_Receive(hcan, CAN_RX_FIFO1, can_recv_cb);
		//3.开启所有can中断
		HAL_CAN_ActivateNotification(hcan, CAN_INTERRUPT);
	}
}

/**************************************************************************************
* FunctionName   : HAL_CAN_ErrorCallback()
* Description    : can错误中断回调函数
* EntryParameter : hcan,CAN_HandleTypeDef结构指针
* ReturnValue    : None
**************************************************************************************/
void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan)
{
	return;
}

#if 1
//void HAL_CAN_RxCpltCallback(CAN_HandleTypeDef *hcan)
//{
//	struct can_recv_s *recv_buf = NULL;;
//	if (hcan->Instance == CAN) {
////		recv_buf = can_recv_create(DRIVER_CAN0, hcan->pRxMsg);
////		if (NULL == recv_buf)
////			return;
//		can_recv_cb(hcan->pRxMsg);
//	}
//}
//
//void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan)
//{
//	if (hcan->Instance == CAN) 
//		HAL_CAN_Receive_IT(hcan, CAN_FIFO0);
//}
#endif

#endif

