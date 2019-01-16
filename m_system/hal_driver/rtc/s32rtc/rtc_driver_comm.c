#if 0
#include "rtc_driver.h"

/**************************************************************************************
* FunctionName   : HAL_RTC_MspInit()
* Description    : rtc初始化
* EntryParameter : hrtc,需要初始化的rtc对象
* ReturnValue    : None
**************************************************************************************/
void HAL_RTC_MspInit(RTC_HandleTypeDef* hrtc)
{

  if(hrtc->Instance==RTC)
  {
    __HAL_RCC_RTC_ENABLE();

	HAL_NVIC_SetPriority(RTC_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(RTC_IRQn);
  }

}

void HAL_RTC_MspDeInit(RTC_HandleTypeDef* hrtc)
{

  if(hrtc->Instance==RTC)
  {
    __HAL_RCC_RTC_DISABLE();
	HAL_NVIC_DisableIRQ(RTC_IRQn);
  }

}

#endif
