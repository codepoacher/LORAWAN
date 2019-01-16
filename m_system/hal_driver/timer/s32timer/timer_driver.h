/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : timer_driver.h
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#if 0
#ifndef __TIMER_DRIVER_H__
#define __TIMER_DRIVER_H__

#include "frtos_types.h"
#include "frtos_ioctl.h"
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_tim.h"
#include "stm32l4xx_hal_tim_ex.h"


/**************************************************************************************
* Description    : 定时器配置
**************************************************************************************/
#define TIM1_PRESCLER_SEC	1000									//精确到s的时钟分频
#define TIM1_PRESCLER_MSEC  (TIM1_PRESCLER_SEC / 1000)				//精确到ms的时钟分频
#define TIM1_PERIOD_VALUE	1000									//定时器1计数值

/**************************************************************************************
* Description    : 定时器配置
**************************************************************************************/
struct timer_conf {
	uint16_t period;			//定时间隔
	ioctl_cb1_t timer_cb;		//定时回调
}__attribute__((packed));
typedef struct timer_conf timer_conf_t;


#endif /* __TIMER_DRIVER_H__ */
#endif
