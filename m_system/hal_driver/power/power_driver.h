/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : power_driver.h
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#if 1
#ifndef __POWER_DEMO_H__
#define __POWER_DEMO_H__

#include "frtos_types.h"
#include "gpio_driver.h"

/**************************************************************************************
* Description    : 设备驱动名称(设备驱动唯一标识符)
**************************************************************************************/
#define POWER_DRIVER_NAME    "/dev/power"

/**************************************************************************************
* Description    : 电源控制管脚定义
**************************************************************************************/
#define POWER_PIN_MPU                   GPIO(B,0)    //mpu 开关电引脚
#define POWER_PIN_MPU_PWRKEY			GPIO(D,9)	 //mpu pwrkey引脚
#define POWER_PIN_MPU_RESET				GPIO(D,8)	 //mpu 重启引脚
#define POWER_PIN_3_3V					GPIO(E,2)	 //3.3v 开关电引脚	

/**************************************************************************************
* Description    : 其他控制和状态管脚定义
**************************************************************************************/
#define STATUS_PIN_MPU_READY			GPIO(B,10)   //mpu 启动完成引脚
/**************************************************************************************
* Description    : 电源控制命令定义
**************************************************************************************/
#define POWER_CMD_MPU_ON				0x01			//mpu电源开命令
#define POWER_CMD_MPU_OFF				0x02			//mpu电源关命令
#define POWER_CMD_3_3V_ON				0x05			//3.3v电源开命令
#define POWER_CMD_3_3V_OFF				0x06			//3.3v电源关命令
#define STATUS_CMD_MPU_READY			0x0D			//mpu状态查询
#define STATUS_CMD_VOLTAGE_GET			0x0E			//获取电池电压
#define POWER_CMD_REINIT                0x0F            //重新初始化电源引脚，用于MCU唤醒之后调用
//电源控制开关
#define POWER_DOWN						1			//关闭相关电源
#define POWER_UP						2			//开启相关电源
#define MPU_PWR_DOWN_DELAY              30000       //MPU pwrkey操作之后延迟30s再给MPU下电
#define GPRS_PWR_DOWN_DELAY             12000       //2G模块 pwrkey操作之后延迟15秒再给2G模块下电
/*#define POWER_CMD_HSRUN               0                 // 高速运行模式
#define POWER_CMD_RUN                 1                 // 普通运行模式
#define POWER_CMD_VLPR                2                 // 极低功耗运行模式
#define POWER_CMD_STOP                3                 // 停止模式
#define POWER_CMD_STOP1               4                 // 停止选项1
#define POWER_CMD_STOP2               5                 // 停止选项2
#define POWER_CMD_VLPS                6                 // 极低功耗停止

#define POWER_ACC3V_BOARD             PTD,11            // 板子3.3V电压开关
#define POWER_5V_BOARD                PTD,16            // 板子5V电压开关
#define POWER_7V_BOARD                PTD,15            // 板子7.2V电压开关
#define POWER_12V_BOARD               PTE,3             // 板子12V电压开关
*/

#endif /* __POWER_DEMO_H__ */
#endif
