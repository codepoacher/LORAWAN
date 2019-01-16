#include "stm32l0xx_hal.h"
#include "frtos_drivers.h"
#include "frtos_errno.h"

#if 0
static WWDG_HandleTypeDef g_wdog;
static int32_t wdog_refresh();
/**************************************************************************************
* FunctionName   : WWDG_IRQHandler()
* Description    : 看门狗中断函数
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
void WWDG_IRQHandler(void)
{
	HAL_WWDG_IRQHandler(&g_wdog);
}

/**************************************************************************************
* FunctionName   : HAL_WWDG_EarlyWakeupCallback() 
* Description    : 看门狗中断回调函数，用来喂狗
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
void HAL_WWDG_EarlyWakeupCallback(WWDG_HandleTypeDef *hwwdg)
{
	HAL_WWDG_Refresh(hwwdg);
}

/**************************************************************************************
* FunctionName   : wdog_init()
* Description    : 看门狗初始化
* EntryParameter : None
* ReturnValue    : 错误码
**************************************************************************************/
static int32_t __init wdog_init(void)
{
	g_wdog.Instance = WWDG;
	g_wdog.Init.Prescaler = WWDG_PRESCALER_8;
	g_wdog.Init.Window = 127;
	g_wdog.Init.Counter = 127;
	g_wdog.Init.EWIMode = WWDG_EWI_ENABLE;
	if (HAL_WWDG_Init(&g_wdog) != HAL_OK){
		return -1;
	}

	return 0;
}

/**************************************************************************************
* FunctionName   : wdog_refresh()
* Description    : 看门狗喂狗
* EntryParameter : None
* ReturnValue    : 错误码
**************************************************************************************/
static int32_t wdog_refresh()
{
	HAL_WWDG_Refresh(&g_wdog);

	return 0;
}

/**************************************************************************************
* FunctionName   : HAL_WWDG_MspInit()
* Description    : 反初始化
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
void HAL_WWDG_MspInit(WWDG_HandleTypeDef* wwdgHandle)
{
	if (wwdgHandle->Instance==WWDG) {
		__HAL_RCC_WWDG_CLK_ENABLE();

		HAL_NVIC_SetPriority(WWDG_IRQn, 5, 0);
		HAL_NVIC_EnableIRQ(WWDG_IRQn);
	}
}

static __const struct driver wdog = {
	.init = wdog_init,
//	.run  = wdog_refresh,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
CORE_INIT(wdog);
#endif
