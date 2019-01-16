/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : timer_driver.c
 * Author        :
 * Date          : 2018-10-23
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#include "frtos_errno.h"
#include "frtos_drivers.h"
#include "frtos_ioctl.h"
#include "frtos_lock.h"
#include "config_driver.h"
#include "timer_driver.h"
#include "stm32l0xx_hal.h"

/**************************************************************************************
* Description    : 模块内部变量
**************************************************************************************/
#define TIM_PRESCALER	31			//2MHz主频，31分频可检测最低1Hz脉冲

static TIM_HandleTypeDef htim3;
static uint32_t pulse_uwIC2Value1		= 0;
static uint32_t pulse_uwIC2Value2		= 0;
static uint32_t pulse_uwDiffCapture		= 0;
static uint16_t pulse_uhCaptureIndex	= 0;
static uint32_t pulse_frequency			= 0;

void TIM3_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&htim3);
}

void HAL_TIM_IC_MspInit(TIM_HandleTypeDef* tim_icHandle)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	if (tim_icHandle->Instance==TIM3) {

		__HAL_RCC_TIM3_CLK_ENABLE();

		/*    
		 * PB5 ------> TIM3_CH2 
		 */
		GPIO_InitStruct.Pin = GPIO_PIN_5;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		GPIO_InitStruct.Alternate = GPIO_AF4_TIM3;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(TIM3_IRQn);
	}
}

void HAL_TIM_IC_MspDeInit(TIM_HandleTypeDef* tim_icHandle)
{
	if (tim_icHandle->Instance==TIM3) {

		__HAL_RCC_TIM3_CLK_DISABLE();

		/*    
		 * PB5 ------> TIM3_CH2 
		 */
		HAL_GPIO_DeInit(GPIOB, GPIO_PIN_5);
		HAL_NVIC_DisableIRQ(TIM3_IRQn);
	}
} 

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM3){
		if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2){
			if(pulse_uhCaptureIndex == 0){
				/* Get the 1st Input Capture value */
				pulse_uwIC2Value1 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
				pulse_uhCaptureIndex = 1;
			}else if(pulse_uhCaptureIndex == 1){
				/* Get the 2nd Input Capture value */
				pulse_uwIC2Value2 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2); 

				/* Capture computation */
				if (pulse_uwIC2Value2 > pulse_uwIC2Value1){
					pulse_uwDiffCapture = (pulse_uwIC2Value2 - pulse_uwIC2Value1); 
				}else if (pulse_uwIC2Value2 < pulse_uwIC2Value1){
					pulse_uwDiffCapture = ((0xFFFF - pulse_uwIC2Value1) + pulse_uwIC2Value2) + 1;
				}else{
					/* If capture values are equal, we have reached the limit of frequency
					 *	measures */
					return;
				}
				/* Frequency computation: for this example TIMx (TIM22) is clocked by
				 *					 APB2Clk */      
				pulse_frequency = (HAL_RCC_GetPCLK2Freq() / TIM_PRESCALER) / pulse_uwDiffCapture;
				pulse_uhCaptureIndex = 0;
			}
		}
	}
}

/*************************************************************************************
* FunctionName   : timer_wakeup()
* Description    : 唤醒后恢复引脚
* EntryParameter : None
* ReturnValue    : 返回错误码
*************************************************************************************/
static int32_t timer_wakeup(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	GPIO_InitStruct.Pin = GPIO_PIN_5;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Alternate = GPIO_AF4_TIM3;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	return 0;
}

/**************************************************************************************
* TypeName       : timer_read()
* Description    : 驱动读数据类型，驱动id号， 读入数据缓冲， 读入buffer最大长度
* EntryParameter : None
* ReturnValue    : 返回错误码或者长度
**************************************************************************************/
static int32_t timer_read(uint8_t idx, void *data, int32_t len)
{
	*(uint32_t*)data = pulse_frequency;
	pulse_frequency = 0;

	return 0;
}

/*************************************************************************************
* FunctionName   : timer_phy_init()
* Description    : 定时器物理初始化
* EntryParameter : None
* ReturnValue    : 返回错误码
*************************************************************************************/
static int32_t timer_phy_init()
{
	 TIM_MasterConfigTypeDef sMasterConfig;
	 TIM_IC_InitTypeDef sConfigIC;

	 htim3.Instance = TIM3;
	 htim3.Init.Prescaler = TIM_PRESCALER;
	 htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	 htim3.Init.Period = 0xffff;
	 htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	 if (HAL_TIM_IC_Init(&htim3) != HAL_OK)
	 {
		 return -EFAULT;
	 }

	 sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	 sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	 if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
	 {
	 	 return -EFAULT;
	 }

	 sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
	 sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
	 sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
	 sConfigIC.ICFilter = 0;
	 if (HAL_TIM_IC_ConfigChannel(&htim3, &sConfigIC, TIM_CHANNEL_2) != HAL_OK)
	 {
		 return -EFAULT;
	 }

	 if (HAL_TIMEx_RemapConfig(&htim3, TIM3_TI2_GPIOB5_AF4) != HAL_OK)
	 {
		 return -EFAULT;
	 }
}

/**************************************************************************************
* TypeName       : module_ioctl_t()
* Description    : 驱动控制，驱动ID号，驱动命令字，驱动控制结构，驱动控制结构长度
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
//extern float g_pulse;
static int32_t timer_ioctl(uint8_t idx, int32_t cmd, void* arg, int32_t len)
{
	if (unlikely(arg == NULL && len != 0 ) || (arg != NULL && len == 0)) {
		return -EINVAL;
	}
	
	//1、执行命令
	switch(cmd) {
		case _IOC_GET_DATA:
//			*(float*)arg = g_pulse;
			break;
		default:
			return -EINVAL;
	}

	return 0;
}
/*************************************************************************************
* FunctionName   : timer_init()
* Description    : 设备初始化
* EntryParameter : None
* ReturnValue    : None
*************************************************************************************/
static int32_t timer_init(void)
{
	timer_phy_init();
	HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_2);
}

static __const struct driver timer1 = {
    .idx	= DRIVER_TIMER1,
    .init	= timer_init,
	.wakeup = timer_wakeup,
	.read	= timer_read,
	.ioctl	= timer_ioctl,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
MODULE_INIT(timer1);
