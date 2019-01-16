/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : frtos_delay.h
 * Author        :
 * Date          : 2017-08-17
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#ifndef __FRTOS_DELAY_H__
#define __FRTOS_DELAY_H__

#include "FreeRTOS.h"
#include "frtos_types.h"

/**************************************************************************************
* FunctionName   : frtos_delay_ns()
* Description    : 纳秒延时函数
* EntryParameter : ns,延时纳秒
* ReturnValue    : None
**************************************************************************************/
static inline void frtos_delay_ns(uint16_t ns)
{
    for(uint16_t i = 0; i < ns; i++) __barrier();
}

/**************************************************************************************
* FunctionName   : frtos_delay_ms()
* Description    : 毫秒延时函数
* EntryParameter : ms,延时毫秒
* ReturnValue    : None
**************************************************************************************/
static inline void frtos_delay_ms(uint16_t ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
}

#endif /*__FRTOS_DELAY_H__ */

