/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : reboot.c
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#if 1
#include "frtos_drivers.h"
#include "stm32l0xx_hal.h"
//#include "core_cm4.h"
//#include "system_S32K144.h"

/**************************************************************************************
* MacroName      : arch_reboot
* Description    : 系统重启
* EntryParameter : magic,幻术
* ReturnValue    : 返回错误码
**************************************************************************************/
int32_t arch_reboot(uint32_t magic)
{
    (void)magic;
	taskENTER_CRITICAL();
	vTaskSuspendAll();
	NVIC_SystemReset();
	return 0;
}
#endif
