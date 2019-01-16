/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : config_user.h
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#ifndef __CONFIG_USER_H__
#define __CONFIG_USER_H__

//#include "S32K144.h"

/**************************************************************************************
* Description    : 设备驱动ID定义列表, ID必须大于0, 0作为保留字段
**************************************************************************************/
#define INIT_PID                     100
#define GPIO_PID                     1
#define ADC_PID                      2
#define GPS_PID                      3
#define CAN_PID                      4
#define ICCARD_PID                   5
#define LCM_PID                      6
#define PRINT_PID                    7
#define SENSOR_PID                   8
#define VEHIC_PID                    9
#define REBOOT_PID                   10
#define CMDLINE_PID					 11
#define POWER_PID                    12
#define UPDATE_PID					 13
#define LORA_PID					 14
#define TAP_PID						 15
#define RS485_1_PID					 16
#define RS485_2_PID					 17
#define TAP1_TASK_PID				 18
#define TAP2_TASK_PID				 19
#define CMD_TASK_PID				 20
#define RS485_CONFIG_PID             21
#define VC_CONFIG_PID                22


/*lorawan 相关*/
#define LORAWAN_PID                  50
#define LORAWAN_TX_PID               51
#define LORAWAN_RX_PID               52
#define LORAWAN_PP_PID               53
#define LINKWAN_PID					 54

struct piddata {
    uint8_t id;                           // id数据类型
    uint32_t len;                         // 数据长度
    uint8_t data[0];                      // 数据内容
} __packed;

#endif /* __CONFIG_USER_H__ */

