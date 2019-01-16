/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : frtos_softimer.h
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#ifndef __FRTOS_SOFTIMER_H__
#define __FRTOS_SOFTIMER_H__

#include "frtos_types.h"

/**************************************************************************************
* Description    : 模块宏数据定义
**************************************************************************************/
#define SOFTIMER_RELOAD_ENABLE          pdTRUE      // 软定时器重复加载使能
#define SOFTIMER_RELOAD_DISABLE         pdFAIL      // 软定时器重复加载关闭

/**************************************************************************************
* Description    : 软定时器类型定义
**************************************************************************************/
typedef TimerHandle_t softimer_t;

/**************************************************************************************
* TypeName       : softimer_handle_t()
* Description    : 软定时器回调函数类型
* EntryParameter : timer,定时器指针
* ReturnValue    : None
**************************************************************************************/
typedef void (*softimer_handle_t)(softimer_t timer);

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************
* FunctionName   : softimer_create()
* Description    : 创建定时器
* EntryParameter : handle,定时器回调, reload,重复加载标记(1,重复, 0,一次), ms,定时毫秒数
* ReturnValue    : 返回定时器指针, NULL,失败
**************************************************************************************/
softimer_t softimer_create(softimer_handle_t handle, uint8_t reload, uint16_t ms);

/**************************************************************************************
* FunctionName   : softimer_start()
* Description    : 启动定时器
* EntryParameter : timer,定时器指针, isr,中断调用使能(1,中断, 0,非中中断)
* ReturnValue    : 返回错误码
**************************************************************************************/
int8_t softimer_start(softimer_t timer, uint8_t isr);

/**************************************************************************************
* FunctionName   : softimer_fstart()
* Description    : 快速启动定时器
* EntryParameter : handle,定时器回调, reload,重复加载标记(1,重复, 0,一次), ms,定时毫秒数
* ReturnValue    : 返回定时器指针, NULL,失败
**************************************************************************************/
softimer_t softimer_fstart(softimer_handle_t handle, uint8_t reload, uint16_t ms);

/**************************************************************************************
* FunctionName   : softimer_stop()
* Description    : 停止定时器
* EntryParameter : timer,定时器指针, isr,中断调用使能(1,中断, 0,非中中断)
* ReturnValue    : 返回错误码
**************************************************************************************/
int8_t softimer_stop(softimer_t timer, uint8_t isr);

/**************************************************************************************
* FunctionName   : softimer_reset()
* Description    : 重置定时器
* EntryParameter : timer,定时器指针, isr,中断调用使能(1,中断, 0,非中中断)
* ReturnValue    : 返回错误码
**************************************************************************************/
int8_t softimer_reset(softimer_t timer, uint8_t isr);

/**************************************************************************************
* FunctionName   : sorftimer_delete()
* Description    : 删除定时器
* EntryParameter : timer,定时器指针
* ReturnValue    : None
**************************************************************************************/
void sorftimer_delete(softimer_t timer);

#ifdef __cplusplus
}
#endif

#endif /* __FRTOS_SOFTIMER_H__ */

