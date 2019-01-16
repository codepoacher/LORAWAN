/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : io_ctrl.h
 * Author        :
 * Date          : 2018-10-23
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#ifndef __IO_CTRL_H__
#define __IO_CTRL_H__

#include "stm32l0xx_hal.h"

/**************************************************************************************
* Description    : 模块内部数据定义
**************************************************************************************/
#define INPUT_ONOFF_1		GPIO(B,2)	//外部接高，读出为低；外部接低，读出为高。
#define INPUT_ONOFF_2		GPIO(B,4)	//外部接高，读出为低；外部接低，读出为高。
#define OUTPUT_3_3V_EN_1	GPIO(D,8)	
#define OUTPUT_3_3V_EN_2	GPIO(B,10)
#define PWR_12V_485_EN		GPIO(E,4)	//对外供12v电

#endif /* __S32ADC_DRIVER_H__ */

