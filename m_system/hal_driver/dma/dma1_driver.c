/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : dma_driver.c
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#if 1
#include "frtos_errno.h"
#include "frtos_drivers.h"
#include "frtos_lock.h"
#include "dma_driver.h"
#include "stm32l0xx_hal.h"

/*************************************************************************************
* Description    : DMA通道7配置
*************************************************************************************/
static DMA_HandleTypeDef hdma_usart1_rx;		  /* usart1_dma */


void DMA1_Channel2_3_IRQHandler(void)
{
	HAL_DMA_IRQHandler(&hdma_usart1_rx);
}


/*************************************************************************************
* FunctionName   : dma_channel3_init()
* Description    : dma通道3初始化
* EntryParameter : None
* ReturnValue    : 错误码
*************************************************************************************/
static int8_t dma_channel3_init(void)
{
	hdma_usart1_rx.Instance = DMA1_Channel3;
	hdma_usart1_rx.Init.Request = DMA_REQUEST_3;
	hdma_usart1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
	hdma_usart1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
	hdma_usart1_rx.Init.MemInc = DMA_MINC_ENABLE;
	hdma_usart1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	hdma_usart1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
	hdma_usart1_rx.Init.Mode = DMA_CIRCULAR;
	hdma_usart1_rx.Init.Priority = DMA_PRIORITY_VERY_HIGH;
	if (HAL_DMA_Init(&hdma_usart1_rx) != HAL_OK)
		return -1;

	return 0;
	//__HAL_LINKDMA(huart,hdmarx,hdma_usart1_rx);
}

/*************************************************************************************
* FunctionName   : dma1_uart_link()
* Description    : dma1通道和串口绑定
* EntryParameter : huart,需要绑定的串口, chn,需要绑定的dma通道
* ReturnValue    : None
*************************************************************************************/
void dma_uart_link(UART_HandleTypeDef *huart, uint8_t chn)
{
	DMA_HandleTypeDef *dma = NULL;

	switch(chn) {
		case DMA1_CHANNEL1:
			__HAL_LINKDMA(huart, hdmarx, hdma_usart1_rx);
			break;
		default:
			break;
	}
}

/*************************************************************************************
* FunctionName   : dma1_init()
* Description    : dma1初始化
* EntryParameter : None
* ReturnValue    : None
*************************************************************************************/
static int32_t __init dma1_init(void)
{
    // 1.初始化DMA
	dma_channel3_init();

    return 0;
}

static __const struct driver dma1 = {
    .init = dma1_init,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
//EARLY_INIT(dma1);
#endif
