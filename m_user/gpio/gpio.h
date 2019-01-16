/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : gpio.h
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#ifndef __GPIO_H__
#define __GPIO_H__

#include "stm32l4xx_hal.h"

/**************************************************************************************
* Description    : GPIO号定义
**************************************************************************************/
#define GPIO_SIGNAL_ACC              129         // ACC
#define GPIO_SIGNAL_TURN_RIGHT       8           // 右转
#define GPIO_SIGNAL_TURN_LEFT        9           // 左转
#define GPIO_SIGNAL_CLOSING_LID      142         // 盒盖
#define GPIO_SIGNAL_ALARM            144         // 报警
#define GPIO_SIGNAL_LOW_BEAM         143         // 近光
#define GPIO_SIGNAL_HIGH_BEAM        97          // 远光
#define GPIO_SIGNAL_BRAKE            96          // 制动
#define GPIO_SIGNAL_LIFT             10          // 举升
#define GPIO_SIGNAL_LIGHT            69          // 空车

#define GPIO_SIGNAL_ICDET            136         // IC卡DET

#endif /* __GPIO_H__ */

