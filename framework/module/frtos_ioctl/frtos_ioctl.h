/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : frtos_ioctl.h
 * Author        :
 * Date          : 2017-08-17
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#ifndef __FRTOS_IOCTL_H__
#define __FRTOS_IOCTL_H__

#include "FreeRTOS.h"
#include "frtos_types.h"

/**************************************************************************************
* Description    : 回调、过滤器设置
**************************************************************************************/
#define _IOC_SET_CB                               0x00000001
#define _IOC_SET_CB1                              0x00000002
#define _IOC_SET_CB2                              0x00000003
#define _IOC_SET_FILTER                           0x00000004
#define _IOC_GET_CB                               0x00000005
#define _IOC_GET_CB1                              0x00000006
#define _IOC_GET_CB2                              0x00000007
#define _IOC_GET_FILTER                           0x00000008
#define _IOC_WAKEUP                               0x00000009
#define _IOC_SUSPEND                              0x0000000A

/**************************************** **********************************************
* Description    : 数据设置
**************************************************************************************/
#define _IOC_SET_DATA                             0x00000011
#define _IOC_SET_DATA1                            0x00000012
#define _IOC_SET_DATA2                            0x00000013

/**************************************************************************************
* Description    : 数据获取
**************************************************************************************/
#define _IOC_GET_DATA                             0x00000021
#define _IOC_GET_DATA1                            0x00000022
#define _IOC_GET_DATA2                            0x00000023

/**************************************************************************************
* Description    : 总线处理相关
**************************************************************************************/
#define _IOC_BUS_INIT                             0x00000031
#define _IOC_BUS_TRANSPORTS                       0x00000032
#define _IOC_BUS_RESET                            0x00000033

/**************************************************************************************
* Description    : RTC处理相关
**************************************************************************************/
#define _IOC_RTC_SETTIME                          0x00000041
#define _IOC_RTC_SETIRQ                           0x00000042
#define _IOC_RTC_SETALARM                         0x00000043
#define _IOC_RTC_SETINT                           0x00000044
#define _IOC_RTC_GETTIME                          0x00000045
#define _IOC_RTC_GETIRQ                           0x00000046
#define _IOC_RTC_WAKEUP	                          0x00000047
#define _IOC_RTC_SLEEP	                          0x00000048

/**************************************************************************************
* Description    : IO处理相关
**************************************************************************************/
#define _IOC_IO_ON                                0x00000051
#define _IOC_IO_OFF                               0x00000052

/**************************************************************************************
* Description    : 电源处理相关
**************************************************************************************/
#define _IOC_PWR_ON                               0x00000061
#define _IOC_PWR_OFF                              0x00000062

/**************************************************************************************
* Description    : 灯处理相关
**************************************************************************************/
#define _IOC_LED_ON                               0x00000071
#define _IOC_LED_OFF                              0x00000072

/**************************************************************************************
* Description    : 设置、清除
**************************************************************************************/
#define _IOC_CLEAR                                0x00000081
#define _IOC_SET                                  0x00000082
#define _IOC_GET                                  0x00000083

/**************************************************************************************
* Description    : 网络处理相关
**************************************************************************************/
#define _IOC_RX_CB                                0x00000091
#define _IOC_CONNECT_CB                           0x00000092
#define _IOC_CLOSE_CB                             0x00000093


/**************************************************************************************
* Description    : gpio
**************************************************************************************/
#define _IOC_GPIO_DIRECTION                       0x000000A1   //对应结构struct gpio_direct_s
#define _IOC_GPIO_GET                             0x000000A2   //对应结构struct gpio
#define _IOC_GPIO_SET                             0x000000A3   //对应结构struct gpio

/**************************************************************************************
* Description    : 其他
**************************************************************************************/
#define _IOC_SUB_IOCTL                            0x000000B1
#define _IOC_USER1                                0x000000B2
#define _IOC_TAP1_HANDLE                          0x000000B3
#define _IOC_TAP2_HANDLE                          0x000000B4
#define _IOC_TAP_GZCS                             0x000000B5
#define _IOC_TAP01_CTRL							  0x000000B6
#define _IOC_TAP02_CTRL							  0x000000B7
#define _IOC_GET_TAP01							  0x000000B8
#define _IOC_GET_TAP02							  0x000000B9
#define _IOC_GET_TAP_ALL						  0x000000BA
#define _IOC_GET_BAT_VOL						  0x000000BB
#define _IOC_GET_SUN_VOL						  0x000000BC
#define _IOC_GET_INPUT_VOL						  0x000000BD	
#define _IOC_GET_PRESS_VOL						  0x000000BE
/**************************************************************************************
* Description    : lora
**************************************************************************************/
#define _IOC_LORA_SET_CONF                       0x000000C1   //对应结构struct lora_conf 
#define _IOC_LORA_GET_TIME                       0x000000C2
#define _IOC_LORA_GZCS_TX						 0x000000C3
#define _IOC_LORA_GZCS_RX						 0x000000C4

/**************************************************************************************
* Description    : e2prom
**************************************************************************************/
#define _IOC_E2PROM_PRESS_WRITE					 0x000000D1
#define _IOC_E2PROM_PRESS_READ					 0x000000D2
#define _IOC_E2PROM_OID_WRITE					 0x000000D3
#define _IOC_E2PROM_OID_READ					 0x000000D4
#define _IOC_E2PROM_TAP01_WRITE					 0x000000D5
#define _IOC_E2PROM_TAP01_READ					 0x000000D6
#define _IOC_E2PROM_TAP02_WRITE					 0x000000D7
#define _IOC_E2PROM_TAP02_READ					 0x000000D8
#define _IOC_E2PROM_485_WRITE					 0x000000D9
#define _IOC_E2PROM_485_READ					 0x000000DA
#define _IOC_E2PROM_UPDATE_CONF_READ			 0X000000DB
#define _IOC_E2PROM_UPDATE_CONF_WRITE			 0X000000DC
#define _IOC_E2PROM_LORA_CONF_WRITE              0x000000DD
#define _IOC_E2PROM_LORA_CONF_READ				 0x000000DE
/**************************************************************************************
* FunctionName   : ioctl_cb()
* Description    : ioctl通用回调函数类型
* EntryParameter : idx, 回调标记, data,数据指针, len,数据长度
* ReturnValue    : None
**************************************************************************************/
typedef int32_t (*ioctl_cb_t)(int32_t , void *, int32_t);

/**************************************************************************************
* FunctionName   : ioctl_cb1()
* Description    : ioctl通用回调函数1类型
* EntryParameter : idx, 回调标记
* ReturnValue    : None
**************************************************************************************/
typedef int32_t (*ioctl_cb1_t)(int32_t);

#endif /*__FRTOS_IOCTL_H__ */

