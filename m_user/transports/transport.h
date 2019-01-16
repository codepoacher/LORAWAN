/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : transport.h
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#ifndef __TRANSPORT_H__
#define __TRANSPORT_H__

//#include "stm32l4xx_hal.h"
#include "frtos_list.h"
#include "frtos_libpack.h"
#include "protobuf-c.h"

/**************************************************************************************
* Description    : 定义头部幻术
**************************************************************************************/
#define TRANS_MAGIC                         0x55443322


/**************************************************************************************
* Description    : 传输数据头部定义
**************************************************************************************/
struct transport {
    uint32_t magic;                   // 传输头部标记，默认为固定值：0x55，0x44，0x33，0x22
    uint32_t length;                  // 后续所有字段的长度总和
    uint8_t id;                       // 标识数据类型
    uint8_t csum;                     // 数据段的校验码
    uint8_t data[0];                  // 通过protobuf生成的数据内容
}__packed;

#endif /* __TRANSPORT_H__ */

