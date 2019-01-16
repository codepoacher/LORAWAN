/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : dma_driver.h
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description * Version       : 1.0 **************************************************************************************/
#ifndef __DMA_DRIVER_H__
#define __DMA_DRIVER_H__

#include "frtos_types.h"
#include "stm32l0xx_hal.h"

/**************************************************************************************
* Description    : dam通道配置
**************************************************************************************/
#define DMA1_CHANNEL1			1			//dma1通道 1
#define DMA1_CHANNEL2			2			//dma1通道 2，可用于uart3的发送
#define DMA1_CHANNEL3			3			//dma1通道 3, 可用于uart3的接收

//关联dma和串口
void dma_uart_link(UART_HandleTypeDef *huart, uint8_t chn);

#endif /* __DMA_DRIVER_H__ */

