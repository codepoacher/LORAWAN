/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : rtc_driver.h
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#if 1
#ifndef __RTC_DRIVER_H__
#define __RTC_DRIVER_H__

#include "frtos_types.h"
#include "frtos_ioctl.h"
#include "stm32l0xx_hal.h"
#include "stm32l0xx_hal_rtc.h"
#include "time.h"

/**************************************************************************************
* Description    : 內核RTC秒中断参数设置
**************************************************************************************/
#define RTC_ALARMA		0
#define RTC_ALARMB		1
struct rtc_setsecirq {
	uint32_t seconds;			//需要产生中断的时间
	void (*irq_func)();			//中断回调函数
}__attribute__((packed));
typedef struct rtc_setsecirq rtc_setsecirq_t;

struct tm_conf {
	uint32_t tm_year;
	uint32_t tm_mon;
	uint32_t tm_week;
	uint32_t tm_mday;
	uint32_t tm_hour;
	uint32_t tm_min;
	uint32_t tm_sec;
	uint32_t tm_subsec;
};
void rtc_time_print();
#endif /* __RTC_DRIVER_H__ */
#endif
