/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : frtos_tasklet.h
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#ifndef __FRTOS_TASKLET_H__
#define __FRTOS_TASKLET_H__

#include "frtos_types.h"
#include "frtos_list.h"
#include "frtos_errno.h"

/**************************************************************************************
* TypeName       : workqueue_fn_t()
* Description    : 工作队列执行函数
* EntryParameter : *args,参数
* ReturnValue    : None
**************************************************************************************/
typedef void (*workqueue_fn_t)(void *args);

/**************************************************************************************
* Description    : 延时执行链表结构
**************************************************************************************/
struct workqueue {
    struct list_head head;                              // 链表头
    uint32_t jffies;                                    // 执行时的TICK值
    workqueue_fn_t run;                                 // 执行回调函数
    void *args;                                         // 回调函数参数
};

/**************************************************************************************
* FunctionName   : tasklet_cancel()
* Description    : 取消延后执行函数
* EntryParameter : work_fn,延时执行回调, ms,延时毫秒数
* ReturnValue    : 返回错误码
**************************************************************************************/
int32_t tasklet_cancel(workqueue_fn_t work_fn);

/**************************************************************************************
* FunctionName   : tasklet_schedule()
* Description    : 调度函数
* EntryParameter : work_fn,延时执行回调, ms,延时毫秒数
* ReturnValue    : 返回错误码
**************************************************************************************/
int32_t tasklet_schedule(workqueue_fn_t work_fn, void *args, uint32_t ms);

#endif /* __FRTOS_TASKLET_H__ */

