#include "stm32l0xx_hal.h"
#include "frtos_drivers.h"
#include "frtos_errno.h"
#include "stm32l0xx_hal_iwdg.h"

#if 1
static IWDG_HandleTypeDef g_wdog;
static int32_t wdog_refresh();


/**************************************************************************************
* FunctionName   : wdog_init()
* Description    : 看门狗初始化
* EntryParameter : None
* ReturnValue    : 错误码
**************************************************************************************/
static int32_t __init wdog_init(void)
{
	g_wdog.Instance = IWDG;
	g_wdog.Init.Prescaler = IWDG_PRESCALER_256;
	g_wdog.Init.Reload = 0x900;
	g_wdog.Init.Window = IWDG_WINDOW_DISABLE;
	HAL_IWDG_Init(&g_wdog);
	if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST) != RESET) {
		debug("watch dog reset\r\n");
	}
	__HAL_RCC_CLEAR_RESET_FLAGS();

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
	HAL_IWDG_Refresh(&g_wdog);

	return 0;
}


static __const struct driver wdog = {
	.init = wdog_init,
	.run  = wdog_refresh,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
CORE_INIT(wdog);
#endif
