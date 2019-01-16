
#include <stdio.h>
#include <stdbool.h>
#include "spi.h"
#include "frtos_errno.h"
#include "frtos_drivers.h"
#include "frtos_lock.h"
#include "frtos_delay.h"
#include "config_driver.h"


#define SPI_BUF_SIZE_MAX   255

static mutex_lock_t spi_lock = NULL;
SPI_HandleTypeDef hspi1;


void HAL_SPI_MspDeInit(SPI_HandleTypeDef* spiHandle)
{

	if(spiHandle->Instance==SPI1)
	{
		__HAL_RCC_SPI1_CLK_DISABLE();

		/**SPI1 GPIO Configuration    
		 * PE13     ------> SPI1_SCK
		 * PE14     ------> SPI1_MISO
		 * PE15     ------> SPI1_MOSI 
		 */
		HAL_GPIO_DeInit(GPIOE, GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15);
	}
} 
void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	if(hspi->Instance==SPI1)
	{
		/* SPI1 clock enable */
		__HAL_RCC_SPI1_CLK_ENABLE();

		/**SPI1 GPIO Configuration    
		 * PE12     ------> SPI1_NSS
		 * PE13     ------> SPI1_SCK
		 * PE14     ------> SPI1_MISO
		 * PE15     ------> SPI1_MOSI 
		 */
		GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		GPIO_InitStruct.Alternate = GPIO_AF2_SPI1;
		HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
	}
}

void spi1_cs(uint8_t selected)
{
	if (selected) {
		SPI1_CLRCS; 
	} 
	else {
		SPI1_SETCS; 
	}
} 
void spi1_xfer(uint8_t *pTxData, uint8_t *pRxData, uint8_t bytes) 
{    
	if (!pRxData) {
		HAL_SPI_Transmit(&hspi1, (uint8_t*)pTxData, bytes, 5000);
	} else if (!pTxData) {
		HAL_SPI_Receive(&hspi1, (uint8_t*)pRxData, bytes, 5000);
	} else {
		HAL_SPI_TransmitReceive(&hspi1, (uint8_t*)pTxData, (uint8_t *)pRxData, bytes, 5000);
	}
}

static int32_t spi1_init(void)
{    
    spi_lock = mutex_lock_init();
	/* SPI1 parameter configuration*/
	hspi1.Instance = SPI1;
	hspi1.Init.Mode = SPI_MODE_MASTER;
	hspi1.Init.Direction = SPI_DIRECTION_2LINES;
	hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
	hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
	hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
	hspi1.Init.NSS = SPI_NSS_HARD_OUTPUT;
	hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
	hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi1.Init.CRCPolynomial = 7;
	if (HAL_SPI_Init(&hspi1) != HAL_OK){
		lprint("spi init error\r\n");
		return -EFAULT;
	};
	
	GPIO_InitTypeDef gpio;
	gpio.Mode = GPIO_MODE_OUTPUT_PP;
	gpio.Pin = GPIO_PIN_12;
	HAL_GPIO_Init(GPIOE, &gpio);
    
	return 0;
}

static __const struct driver spi = {
    .idx      = DRIVER_SPI,
    .init     = spi1_init,
   // .read     = spi1_read,
   // .write    = spi1_write,
};
    
MODULE_INIT(spi);


