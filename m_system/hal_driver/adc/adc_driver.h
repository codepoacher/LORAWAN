/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : adc_driver.c
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#ifndef __ADC_DRIVER_H__
#define __ADC_DRIVER_H__

#include "stm32l0xx_hal.h"

/**************************************************************************************
* Description    : ADC采样数据读取结果
**************************************************************************************/
struct adc_samp_s {
    uint8_t adc;                    // ADC索引(0,1)
    uint8_t ch;                     // ADC通道索引
    int16_t zero;                   // 校零AD值
    int32_t scale;                  // 最小刻度值(单位:mv)
    int32_t result;                 // 采样结果
};

/**************************************************************************************
* Description    : ADC通道索引值
**************************************************************************************/
enum {
	PW_ADC = 1,
	INPUT_VOL1,
	VBATIN_ADC,
	VSUNIN_ADC,
};

#define ADC_COMM_READ(a,c,s,z,arg) \
	do{\
		(a).ch = (c);\
		(a).scale = (s);\
		(a).zero = (z);\
		fdrive_read(DRIVER_ADC,&(a),sizeof(struct adc_samp_s));\
		*(uint16_t*)arg = (*(float*)&a.result)*100;\
	}while(0)

#endif /* __S32ADC_DRIVER_H__ */

