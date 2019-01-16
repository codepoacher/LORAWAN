/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : config_driver.h
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#ifndef __CONFIG_DRIVER_H__
#define __CONFIG_DRIVER_H__

//#include "S32K144.h"

/**************************************************************************************
* Description    : 设备驱动ID定义列表, ID必须大于0, 0作为保留字段
**************************************************************************************/
#define DRIVER_ADC                  1
#define DRIVER_PRINT                2
#define DRIVER_LCD                  3
#define DRIVER_I2C                  4
#define DRIVER_GPIO                 5
#define DRIVER_SENSOR               6
#define DRIVER_RTC                  7
#define DRIVER_POWER                8
#define DRIVER_LPUART	            9
#define DRIVER_UART4				10
#define DRIVER_UART3                11
#define DRIVER_UART2                12
#define DRIVER_UART1				13
#define DRIVER_FLEXUART             14
#define DRIVER_CAN0                 15
#define DRIVER_CAN1                 16
#define DRIVER_CAN2                 17
#define DRIVER_TIMER1               18
#define DRIVER_GPIOINT              19
#define DRIVER_GPS					20
#define DRIVER_FLASH				21
#define DRIVER_LORA                 22
#define DRIVER_SPI                  23
#define DRIVER_TAP					24
#define DRIVER_UART5				25
#define DRIVER_IOCTRL				26
#define DRIVER_E2PROM				27

#endif /* __CONFIG_DRIVER_H__ */

