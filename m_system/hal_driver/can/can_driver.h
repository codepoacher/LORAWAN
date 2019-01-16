#ifndef __CAN_DRIVER_H__
#define __CAN_DRVIER_H__

#if 1
#include "frtos_ioctl.h"
#include "stm32l0xx_hal.h"
#include "gpio_driver.h"
//#include "stm32l4xx_hal_can.h"


/**************************************************************************************
* Description    : 模块内部配置宏定义
**************************************************************************************/
#define CAN_SEND_OVERTIME                1           // CAN发送超时配置
#define CAN_BUFFER_SIZE					512

/**************************************************************************************
* Description    : 消息结构定义
**************************************************************************************/
struct can_msg_s{
    uint32_t msgid;                                     // CAN消息ID
	uint8_t id_type;									// CAN消息ID类型
	uint8_t msg_type;									// CAN消息类型
    uint8_t len;                                        // CAN消息长度
    uint8_t data[8];                                    // CAN消息数据
};
#define CAN_DATA		1				//can 数据帧
#define CAN_REMOTE		2				//can 远程帧


#define CAN_SUSPEND		1				//休眠回调
#define CAN_WAKEUP		2				//唤醒回调
#define CAN_RECEIVE     3				//接收回调
struct can_cb_s {
	uint8_t cb_type;	
	ioctl_cb_t cb;
};

/**************************************************************************************
* Description    : 设备端口配置
**************************************************************************************/

#define CAN_STB_GPIO  GPIOB                       // can STB模式gpio引脚的寄存器组
#define CAN_STB_PIN   GPIO_PIN_14				 // can STB模式gpio引脚
#define CAN_PRESCALER 4							 // can 的预分频数
#define CAN_STB_HIGH_SET()  \
	gpio_pin_set(CAN_STB_GPIO, CAN_STB_PIN, GPIO_PIN_RESET)				 // can STB模式设置为高速模式
#define CAN_STB_STOP_SET  \
	gpio_pin_set(CAN_STB_GPIO, CAN_STB_PIN, GPIO_PIN_SET)				 // can STB模式设置为禁止接受模式

#define CAN_IRQPRIO                           1                       // can0中断优先级
#define CAN_QUEUE_LEN                         20                       // can0接收队列长度
/**************************************************************************************
* Description    : 唤醒设置结构
**************************************************************************************/
struct can_wakeup_t{
    uint32_t msgid;                                     // 唤醒消息ID
    uint32_t mask;                                      // 唤醒屏蔽码
};

/**************************************************************************************
* Description    : 消息结构定义
**************************************************************************************/
struct can_recv_s{
    uint8_t idx;                                        // 设备索引
    struct can_msg_s msg;                               // 消息
}__attribute__((packed));

/**************************************************************************************
* Description    : id类型
**************************************************************************************/
#define CAN_STDID				1						//标准id
#define CAN_EXTID				2						//扩展id

/**************************************************************************************
* Description    : id 位数
**************************************************************************************/
#define CAN_ID_16BIT			1						//id是16位的, 只有标准id
#define CAN_ID_32BIT			2						//id是32位的

/**************************************************************************************
* Description    : id列表支持的最大id数
**************************************************************************************/
#define CAN_ID_MAX				56						//可过滤的最大id数

/**************************************************************************************
* Description    : id列表支持的最大id数
**************************************************************************************/
#define CAN_INTERRUPT  (CAN_IT_RX_FIFO0_MSG_PENDING | \
						CAN_IT_RX_FIFO0_FULL | \
						CAN_IT_RX_FIFO1_MSG_PENDING | \
						CAN_IT_RX_FIFO1_FULL | \
						CAN_IT_ERROR)

/**************************************************************************************
* Description    : can id列表结构定义
**************************************************************************************/
struct can_filter_s{
	uint8_t cnt;										//需要过滤的id个数
	uint8_t id_type;									//id的类型，是标准id还是扩展id
	uint8_t bit_type;									//id的位数，是16位id，还是32位id
	uint32_t can_id[CAN_ID_MAX];						//需要过滤的id, 16位标准id最大支持56个，
														//32位标准或扩展id最大支持28个, id需要
														//与id_type 和 bit_type 所设置的对应
}__attribute__((packed));


//void can_recv_cb(CAN_RxHeaderTypeDef *header, uint8_t *data);
//struct can_recv_s *can_recv_create(uint8_t idx, CAN_RxHeaderTypeDef *header, 
//									  uint8_t *data);
#endif

#endif
