/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : frtos_lock.h
 * Author        :
 * Date          : 2017-08-17
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#ifndef __FRTOS_LOCK_H__
#define __FRTOS_LOCK_H__

#include "FreeRTOS.h"
#include "frtos_types.h"

/**************************************************************************************
* MacroName      : mutex_lock_t()
* Description    : 定义锁
**************************************************************************************/
#define mutex_lock_t xSemaphoreHandle

/**************************************************************************************
* MacroName      : mutex_lock_init()
* Description    : 初始化锁
* EntryParameter : None
* ReturnValue    : 返回锁
**************************************************************************************/
#define mutex_lock_init()          xSemaphoreCreateMutex()

/**************************************************************************************
* MacroName      : mutex_lock()
* Description    : 加锁
* EntryParameter : 锁
* ReturnValue    : None
**************************************************************************************/
#define mutex_lock(lock)           xSemaphoreTake(lock, pdMS_TO_TICKS(portMAX_DELAY))

/**************************************************************************************
* MacroName      : mutex_unlock()
* Description    : 解锁
* EntryParameter : 锁
* ReturnValue    : None
**************************************************************************************/
#define mutex_unlock(lock)            xSemaphoreGive(lock)

/**************************************************************************************
* MacroName      : mutex_lock_irq()
* Description    : 中断中加锁
* EntryParameter : 锁
* ReturnValue    : None
**************************************************************************************/
#define mutex_lock_irq(lock)    xSemaphoreTakeFromISR(lock, pdMS_TO_TICKS(portMAX_DELAY))

/**************************************************************************************
* MacroName      : mutex_unlock_irq()
* Description    : 中断中解锁
* EntryParameter : 锁
* ReturnValue    : None
**************************************************************************************/
#define mutex_unlock_irq(lock)            xSemaphoreGiveFromISR(lock)

/**************************************************************************************
* MacroName      : vTaskSuspendAll()
* Description    : 进入关键代码区
* EntryParameter : 锁
* ReturnValue    : None
**************************************************************************************/
#define vTaskSuspendAll()           vPortEnterCritical()    // 禁止调度

/**************************************************************************************
* MacroName      : xTaskResumeAll()
* Description    : 退出关键代码区
* EntryParameter : 锁
* ReturnValue    : None
**************************************************************************************/
#define xTaskResumeAll()            vPortExitCritical()     // 使能调度

#endif /*__FRTOS_LOCK_H__ */

