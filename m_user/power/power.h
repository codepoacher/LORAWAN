/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : power.h
 * Author        :
 * Date          : 2017-07-09
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#ifndef __POWER_H__
#define __POWER_H__

#include "S32K144.h"

/**************************************************************************************
* Description    : GPIO号定义
**************************************************************************************/
#define POWER_MODE_PERF              0         // 摩托车点火后的正常工作模式
#define POWER_MODE_LOW               1         // 摩托车熄火以后的低功耗模式
#define POWER_MODE_GPS        9           // 左转
#define GPIO_SIGNAL_CLOSING_LID      142         // 盒盖
#define GPIO_SIGNAL_ALARM            144         // 报警
#define GPIO_SIGNAL_LOW_BEAM         143         // 近光
#define GPIO_SIGNAL_HIGH_BEAM        97          // 远光
#define GPIO_SIGNAL_BRAKE            96          // 制动
#define GPIO_SIGNAL_LIFT             10          // 举升
#define GPIO_SIGNAL_LIGHT            69          // 空车

#define GPIO_SIGNAL_ICDET            136         // IC卡DET
#define POWERED_ON                     1         // 电源打开
#define POWERED_OFF                    0         // 电源关闭

#endif /* __GPIO_H__ */

