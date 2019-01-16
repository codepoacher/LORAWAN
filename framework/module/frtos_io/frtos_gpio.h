/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : frtos_gpio.h
 * Author        :
 * Date          : 2017-08-17
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#ifndef __FRTOS_GPIO_H__
#define __FRTOS_GPIO_H__

#include "FreeRTOS.h"
#include "frtos_types.h"

/**************************************************************************************
* Description    : 定义通用GPIO头部
**************************************************************************************/
//gpio 号从0 - 79, 每16个对应一个gpio组
#define GPIO_GROUP_INTERVAL   16
struct gpio {
    uint32_t gpio;
    uint32_t value;
}; 

/**************************************************************************************
* Description    : GPIO宏控制
**************************************************************************************/
#define GPIO_A 0			//gpio组 A
#define GPIO_B 1			//gpio组 B
#define GPIO_C 2			//gpio组 C
#define GPIO_D 3			//gpio组 D
#define GPIO_E 4			//gpio组 E
#define GPIO_H 5

#define GPIO_0	0			//gpio脚 1
#define GPIO_1 	1			//gpio脚 2
#define GPIO_2 	2			//gpio脚 3
#define GPIO_3 	3			//gpio脚 4
#define GPIO_4 	4			//gpio脚 5
#define GPIO_5 	5			//gpio脚 6
#define GPIO_6 	6			//gpio脚 7
#define GPIO_7 	7			//gpio脚 8
#define GPIO_8 	8			//gpio脚 9
#define GPIO_9  9			//gpio脚 10
#define GPIO_10 10			//gpio脚 11
#define GPIO_11 11			//gpio脚 12
#define GPIO_12	12			//gpio脚 13
#define GPIO_13 13			//gpio脚 14
#define GPIO_14 14			//gpio脚 15
#define GPIO_15 15			//gpio脚 16

#define GPIO_GROUP_INTERVAL		16										//GPIO组之间的间隔
/**************************************************************************************
* FunctionName   : GPIOSEQ_TO_GPIOGROUP()
* Description    : gpio序号转换为gpio的组号
* EntryParameter : pin,gpio的pin脚序号(0-79)
* ReturnValue    : gpio的组号(0-4)
**************************************************************************************/
#define GPIOSEQ_TO_GPIOGROUP(pin)		((pin) / GPIO_GROUP_INTERVAL)		//gpio序号转gpio组
/**************************************************************************************
* FunctionName   : GPIOSEQ_TO_GPIOPIN()
* Description    : gpio序号转换为gpio的pin脚号
* EntryParameter : pin,gpio的pin脚序号(0-79)
* ReturnValue    : gpio的pin脚号
**************************************************************************************/
#define GPIOSEQ_TO_GPIOPIN(pin) (1 << ((pin) % GPIO_GROUP_INTERVAL))	//gpio序号转gpio引脚

/**************************************************************************************
* FunctionName   : GPIO()
* Description    : 通过gpio的组号和pin脚号转换为gpio序号
* EntryParameter : a,gpio的组号(A-E), b,gpio的引脚号(0-15)
* ReturnValue    : gpio的序号(0-79)
**************************************************************************************/
#define GPIO(a, b) (GPIO_##a * GPIO_GROUP_INTERVAL + GPIO_##b)			//gpio组与序号之间的转换

#endif /*__FRTOS_GPIO_H__ */

