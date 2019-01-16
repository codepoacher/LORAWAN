
/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : tap_control.c
 * Author        :
 * Date          : 2018-10-18
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#ifndef _TAP_CONTROL_H_
#define _TAP_CONTROL_H_

#include "gpio_driver.h"

/**************************************************************************************
* Description    : 定义GPIO引脚
**************************************************************************************/
//阀门1
#define TAP_01_SF						GPIO(A,7)		//暂时不用
#define TAP_01_OUT_A					GPIO(C,4)		//output_A
#define TAP_01_OUT_B					GPIO(C,5)		//output_B
#define TAP_01_CTRL_D1					GPIO(B,0)		//CTRL_D1
#define TAP_01_CTRL_D2					GPIO(B,1)		//CTRL_D2
//阀门2
#define TAP_02_SF						GPIO(E,0)		//暂时不用
#define	TAP_02_OUT_A					GPIO(B,9)		//output_A
#define TAP_02_OUT_B					GPIO(B,8)		//output_B
#define TAP_02_CTRL_D1					GPIO(E,1)		//CTRL_D1
#define TAP_02_CTRL_D2					GPIO(E,2)		//CTRL_D2
//电容充电
#define TAP_CHARGE_PIN					GPIO(E,3)		//电容充电引脚

/**************************************************************************************
* Description    : 定义GPIO数据最大长度
**************************************************************************************/
#define CHARGE_TIME                     4000	//电容充电时间ms
#define KEEP_TIME	                    50		//开关阀脉冲时间ms

#define TAP_01							1		//1号阀门
#define TAP_02							2		//2号阀门
#define TAP_CLOSE						0		//关阀门
#define TAP_OPEN						1		//开阀门
#define TAP_UNCTRL						2		//阀门未控制

extern uint8_t g_tap01_stats, g_tap02_stats;

struct tap_ctl_pin {
	uint32_t d1;
	uint32_t d2;
	uint32_t a;
	uint32_t b;
};

struct tap_action {
	uint8_t ch;
	uint8_t action;
};

#endif
