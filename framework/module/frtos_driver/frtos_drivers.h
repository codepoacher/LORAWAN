/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : frtos_drivers.h
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#ifndef __FRTOS_DRIVERS_H__
#define __FRTOS_DRIVERS_H__

#include "frtos_types.h"

/**************************************************************************************
* TypeName       : module_init_t()
* Description    : 驱动初始化类型
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
typedef int32_t (*module_init_t)(void);

/**************************************************************************************
* TypeName       : module_thread_t()
* Description    : 驱动周期工作函数
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
typedef int32_t (*module_thread_t)(void);

/**************************************************************************************
* TypeName       : module_stop_t()
* Description    : 驱动卸载类型
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
typedef int32_t (*module_stop_t)(void);

/**************************************************************************************
* TypeName       : module_suspend_t()
* Description    : 驱动进入低功耗模式类型
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
typedef int32_t (*module_suspend_t)(void);

/**************************************************************************************
* TypeName       : module_wakeup_t()
* Description    : 驱动唤醒函数类型
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
typedef int32_t (*module_wakeup_t)(void);

/**************************************************************************************
* TypeName       : module_read_t()
* Description    : 驱动读数据类型，驱动id号， 读入数据缓冲， 读入buffer最大长度
* EntryParameter : None
* ReturnValue    : 返回错误码或者长度
**************************************************************************************/
typedef int32_t (*module_read_t)(uint8_t, void *, int32_t);

/**************************************************************************************
* TypeName       : module_write_t()
* Description    : 驱动写数据类型，驱动id号， 写入数据缓冲， 写入数据长度
* EntryParameter : None
* ReturnValue    : 返回错误码或者长度
**************************************************************************************/
typedef int32_t (*module_write_t)(uint8_t, void *, int32_t);

/**************************************************************************************
* TypeName       : module_ioctl_t()
* Description    : 驱动控制，驱动ID号，驱动命令字，驱动控制结构，驱动控制结构长度
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
typedef int32_t (*module_ioctl_t)(uint8_t, int32_t, void *, int32_t);

/**************************************************************************************
* Description    : 驱动初始化结构定义
**************************************************************************************/
struct driver {
    __const uint8_t idx;                        // 驱动id
    __const module_init_t init;                 // 驱动初始化函数
    __const module_thread_t run;                // 驱动线程
    __const module_stop_t stop;                 // 驱动卸载函数
    __const module_suspend_t suspend;           // 驱动休眠函数
    __const module_wakeup_t wakeup;             // 驱动唤醒函数
    __const module_read_t read;                 // 驱动读数据函数
    __const module_write_t write;               // 驱动写数据函数
    __const module_ioctl_t ioctl;               // 驱动控制函数
};

/**************************************************************************************
* TypeName       : MODULE_INSTALL()
* Description    : 驱动统一加载库
* EntryParameter : drivers,驱动结构体， pro驱动优先级
* ReturnValue    : None
**************************************************************************************/
#define MODULE_INSTALL(drivers, pro) \
    static __const struct driver* __attribute__((used,section(".module_driver_list_"#pro))) \
        __fdrivers_##drivers##__ = &drivers

/**************************************************************************************
* TypeName       : ARCH_INIT()
* Description    : 驱动初始化函数
* EntryParameter : drivers,驱动结构体
* ReturnValue    : None
**************************************************************************************/
#define ARCH_INIT(drivers)            MODULE_INSTALL(drivers, 0)

/**************************************************************************************
* TypeName       : EARLAY_INIT()
* Description    : 驱动初始化函数
* EntryParameter : drivers,驱动结构体
* ReturnValue    : None
**************************************************************************************/
#define EARLY_INIT(drivers)           MODULE_INSTALL(drivers, 1)

/**************************************************************************************
* TypeName       : CORE_INIT()
* Description    : 驱动初始化函数
* EntryParameter : drivers,驱动结构体
* ReturnValue    : None
**************************************************************************************/
#define CORE_INIT(drivers)            MODULE_INSTALL(drivers, 2)

/**************************************************************************************
* TypeName       : POSTCORE_INIT()
* Description    : 驱动初始化函数
* EntryParameter : drivers,驱动结构体
* ReturnValue    : None
**************************************************************************************/
#define POSTCORE_INIT(drivers)        MODULE_INSTALL(drivers, 3)

/**************************************************************************************
* TypeName       : SUBSYS_INIT()
* Description    : 驱动初始化函数
* EntryParameter : drivers,驱动结构体
* ReturnValue    : None
**************************************************************************************/
#define SUBSYS_INIT(drivers)          MODULE_INSTALL(drivers, 4)

/**************************************************************************************
* TypeName       : FSCORE_INIT()
* Description    : 驱动初始化函数
* EntryParameter : drivers,驱动结构体
* ReturnValue    : None
**************************************************************************************/
#define FSCORE_INIT(drivers)          MODULE_INSTALL(drivers, 5)

/**************************************************************************************
* TypeName       : MODULE_INIT()
* Description    : 驱动初始化函数
* EntryParameter : drivers,驱动结构体
* ReturnValue    : None
**************************************************************************************/
#define MODULE_INIT(drivers)          MODULE_INSTALL(drivers, 6)

/**************************************************************************************
* TypeName       : LATE_INIT()
* Description    : 驱动初始化函数
* EntryParameter : drivers,驱动结构体
* ReturnValue    : None
**************************************************************************************/
#define LATE_INIT(drivers)            MODULE_INSTALL(drivers, 7)

#endif /* __FRTOS_DRIVERS_H__ */
