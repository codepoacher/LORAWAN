/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : i2c_driver.h
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#if 0
#ifndef __I2C_DRIVER_H__
#define __I2C_DRIVER_H__

#include "frtos_types.h"
#include "frtos_ioctl.h"
#include "stm32l4xx_hal.h"

/**************************************************************************************
* Description    : 模块配置宏数据定义
**************************************************************************************/
#define I2C_RW_WAIT_TIMES       255         // I2C发送超时时间

/**************************************************************************************
* Description    : 设备I2C传输消息标志
**************************************************************************************/
#define I2C_M_WT                0           // 写标志
#define I2C_M_RD                1           // 读标志
#define I2C_M_TEN               2           // 10比特地址

/**************************************************************************************
* Description    : 设备I2C传输消息体
**************************************************************************************/
struct i2c_msg_s{
    uint16_t addr;                          // 从设备地址
	uint16_t reg_addr;						// i2c设备寄存器地址
    uint8_t *data;                          // 数据指针
    int16_t len;                            // 数据长度
    uint8_t flags;                          // 消息传输标记
};

/**************************************************************************************
* FunctionName   : i2c_transfer()
* Description    : I2C设备阻塞式收发数据
* EntryParameter : msgs,消息数组, cnt,传输Msgs个数
* ReturnValue    : 返回发送成功的个数
**************************************************************************************/
int16_t i2c_transfer(struct i2c_msg_s msgs[], int16_t cnt);

/**************************************************************************************
* FunctionName   : i2c_read_block_data()
* Description    : 读取寄存器内容
* EntryParameter : addr,芯片地址, reg,寄存器地址, length,缓存区长度, buf,缓冲区
* ReturnValue    : 返回错误码
**************************************************************************************/
int8_t i2c_read_block_data(uint16_t addr, uint8_t reg, int16_t len, uint8_t *buf);

/**************************************************************************************
* FunctionName   : i2c_write_block_data()
* Description    : 写寄存器内容
* EntryParameter : addr,芯片地址, reg,寄存器地址, len,缓存区长度，buf,缓冲区
* ReturnValue    : 返回错误码
**************************************************************************************/
int8_t i2c_write_block_data(uint8_t addr, uint8_t reg, int16_t len, uint8_t *data);

/**************************************************************************************
* FunctionName   : i2c_smbus_write_data()
* Description    : i2c写数据
* EntryParameter : addr,芯片地址, len,缓存区长度，data,缓冲区
* ReturnValue    : 返回错误码
**************************************************************************************/
int8_t i2c_smbus_write_data(uint8_t addr, int16_t len, uint8_t *data,uint8_t flag);

/**************************************************************************************
* FunctionName   : i2c_smbus_read_data()
* Description    : 写寄存器内容
* EntryParameter : addr,芯片地址, len,缓存区长度，data,缓冲区
* ReturnValue    : 返回错误码
**************************************************************************************/
int8_t i2c_smbus_read_data(uint8_t addr, int16_t len, uint8_t *data,uint8_t flag);

#endif /* __I2C_DRIVER_H__ */
#endif
