/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : frtos_sys.h
 * Author        :
 * Date          : 2017-08-17
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#ifndef __FRTOS_SYS_H__
#define __FRTOS_SYS_H__

#include "FreeRTOS.h"
#include "frtos_types.h"

/**************************************************************************************
* Description    : 定义关机幻术
**************************************************************************************/
#define SYS_REBOOT                                          0
#define SYS_POWEROFF                                        1

/**************************************************************************************
* Description    : 定义外部内存大小
**************************************************************************************/
#define FRTOS_UCHEAP_SIZE                                   0 //表示采用柔性数组

/**************************************************************************************
* MacroName      : fuser_data_set()
* Description    : 设置数据
* EntryParameter : idx,应用ID，data,设置应用数据， len数据长度
* ReturnValue    : 返回错误码
**************************************************************************************/
int32_t fuser_data_set(uint8_t idx, void *data, int32_t len);

/**************************************************************************************
* MacroName      : fuser_data_get()
* Description    : 获取数据
* EntryParameter : idx,应用ID，data,获取应用数据， len数据长度
* ReturnValue    : 返回错误码
**************************************************************************************/
int32_t fuser_data_get(uint8_t idx, void *data, int32_t len);

/**************************************************************************************
* MacroName      : fdrive_read()
* Description    : 读取驱动数据
* EntryParameter : idx,驱动ID，data,数据， len数据长度
* ReturnValue    : 返回错误码或者长度
**************************************************************************************/
int32_t fdrive_read(uint8_t idx, void *data, int32_t len);

/**************************************************************************************
* MacroName      : fdrive_write()
* Description    : 写驱动数据
* EntryParameter : idx,驱动ID，data,数据， len数据长度
* ReturnValue    : 返回错误码或者写入长度
**************************************************************************************/
int32_t fdrive_write(uint8_t idx, void *data, int32_t len);

/**************************************************************************************
* MacroName      : fdrive_ioctl()
* Description    : 控制驱动程序
* EntryParameter : idx,驱动ID，cmd,控制命令字，data,数据， len数据长度
* ReturnValue    : 返回错误码
**************************************************************************************/
int32_t fdrive_ioctl(uint8_t idx,int32_t cmd, void *data, int32_t len);

/**************************************************************************************
* MacroName      : fdrive_suspend
* Description    : 控制驱动进入休眠
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
int32_t fdrive_suspend(void);

/**************************************************************************************
* MacroName      : fdrive_wakeup
* Description    : 控制驱动唤醒
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
int32_t fdrive_wakeup(void);

/**************************************************************************************
* MacroName      : fsystem_reboot
* Description    : 系统重启
* EntryParameter : magic,幻术
* ReturnValue    : 返回错误码
**************************************************************************************/
int32_t fsystem_reboot(uint32_t);

#endif /*__FRTOS_SYS_H__ */

