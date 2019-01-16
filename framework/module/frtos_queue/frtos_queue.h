/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : frtos_queue.h
 * Author        :
 * Date          : 2017-08-17
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#ifndef __FRTOS_QUEUE_H__
#define __FRTOS_QUEUE_H__

#include "frtos_types.h"
#include "frtos_errno.h"

/**************************************************************************************
* Description    : 自定义数据类型定义
**************************************************************************************/
typedef xQueueHandle fqueue_t;                  // 队列数据类型

/**************************************************************************************
* FunctionName   : fqueue_create()
* Description    : 创建队列
* EntryParameter : len,队列长度, item_size,项目字节数
* ReturnValue    : 返回创建好的队列指针, NULL,创建失败
**************************************************************************************/
static inline fqueue_t fqueue_create(uint32_t len, uint32_t item_size)
{
    return xQueueCreate(len, item_size);
}

/**************************************************************************************
* FunctionName   : fqueue_push()
* Description    : 入队列
* EntryParameter : queue,队列指针, *txitem,入队列项目指针, ms,等待毫秒
* ReturnValue    : 返回错误码
**************************************************************************************/
static inline int8_t fqueue_push(fqueue_t queue, void *txitem, uint32_t ms)
{
    if(NULL == queue || NULL == txitem) return -EMEM;
    if(pdPASS != xQueueSend(queue, txitem, ms)) return -ENOMEM;

    return 0;
}

/**************************************************************************************
* FunctionName   : fqueue_pop()
* Description    : 出队列
* EntryParameter : queue,队列指针, *rxitem,出队列项目指针, ms,等待毫秒,
                   del,从队列删除(true,删除, false,不删除)
* ReturnValue    : 返回错误码
**************************************************************************************/
static inline int8_t fqueue_pop(fqueue_t queue, void *rxitem, uint32_t ms, bool del)
{
    if(NULL == queue || NULL == rxitem) return -EMEM;

    if(true == del){
        if(pdPASS != xQueueReceive(queue, rxitem, ms)) return -EEMPTY;
    }else{
        if(pdPASS != xQueuePeek(queue, rxitem, ms))    return -EEMPTY;
    }

    return 0;
}

#endif /* __FRTOS_QUEUE_H__ */

