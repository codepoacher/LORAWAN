#if 0
#include "i2c_driver.h"
#include "stm32l0xx_hal_i2c.h"

/*************************************************************************************
* FunctionName   : HAL_I2C_MspInit()
* Description    : i2c初始化
* EntryParameter : hi2c,需要初始化的i2c对象
* ReturnValue    : None
*************************************************************************************/
void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c)
{

  GPIO_InitTypeDef GPIO_InitStruct;
  if(hi2c->Instance==I2C1){
	  /**I2C1 GPIO Configuration    
	   * PB6     ------> I2C1_SCL
	   * PB7     ------> I2C1_SDA 
	   */
	  GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
	  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
	  GPIO_InitStruct.Pull = GPIO_PULLUP;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	  GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
	  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	  /* I2C1 clock enable */
	  __HAL_RCC_I2C1_CLK_ENABLE();
  }

}

/*************************************************************************************
* FunctionName   : HAL_I2C_MspDeInit()
* Description    : i2c反初始化
* EntryParameter : hi2c,需要反初始化的i2c对象
* ReturnValue    : None
*************************************************************************************/
void HAL_I2C_MspDeInit(I2C_HandleTypeDef* hi2c)
{

  if(hi2c->Instance==I2C1){

	  __HAL_RCC_I2C1_CLK_DISABLE();

	  /**I2C1 GPIO Configuration    
	   * PB6     ------> I2C1_SCL
	   * PB7     ------> I2C1_SDA 
	   */
	  HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6|GPIO_PIN_7);
  }
}

#endif
