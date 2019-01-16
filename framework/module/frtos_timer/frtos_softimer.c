/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : frtos_softimer.c
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#if 1
#include "frtos_errno.h"
#include "frtos_softimer.h"

/**************************************************************************************
* FunctionName   : softimer_create()
* Description    : 创建定时器
* EntryParameter : handle,定时器回调, reload,重复加载标记(1,重复, 0,一次), ms,定时毫秒数
* ReturnValue    : 返回定时器指针, NULL,失败
**************************************************************************************/
softimer_t softimer_create(softimer_handle_t handle, uint8_t reload, uint16_t ms)
{
    TimerHandle_t timer = NULL;

    if(0 != reload)reload = pdTRUE;
    timer = xTimerCreate(NULL, pdMS_TO_TICKS(ms), reload, NULL, handle);
    if(NULL == timer)return NULL;

    return timer;
}

/**************************************************************************************
* FunctionName   : softimer_start()
* Description    : 启动定时器
* EntryParameter : timer,定时器指针, isr,中断调用使能(1,中断, 0,非中中断)
* ReturnValue    : 返回错误码
**************************************************************************************/
int8_t softimer_start(softimer_t timer, uint8_t isr)
{
    if(0 != isr){
        if(pdPASS != xTimerStartFromISR(timer, pdFALSE)) return -EMEM;
    }else{
        if(pdPASS != xTimerStart(timer, 0))return -EMEM;
    }
    return 0;
}

/**************************************************************************************
* FunctionName   : softimer_fstart()
* Description    : 快速启动定时器
* EntryParameter : handle,定时器回调, reload,重复加载标记(1,重复, 0,一次), ms,定时毫秒数
* ReturnValue    : 返回定时器指针, NULL,失败
**************************************************************************************/
softimer_t softimer_fstart(softimer_handle_t handle, uint8_t reload, uint16_t ms)
{
    softimer_t timer = NULL;

    // 1.创建定时
    timer = softimer_create(handle, reload, ms);
    if(NULL == timer)return NULL;

    // 2.启动定时器
    softimer_start(timer, 0);

    return timer;
}

/**************************************************************************************
* FunctionName   : softimer_stop()
* Description    : 停止定时器
* EntryParameter : timer,定时器指针, isr,中断调用使能(1,中断, 0,非中中断)
* ReturnValue    : 返回错误码
**************************************************************************************/
int8_t softimer_stop(softimer_t timer, uint8_t isr)
{
    if(0 != isr){
        if(pdTRUE != xTimerStopFromISR(timer, pdFALSE))return -EMEM;
    }else{
        if(pdTRUE != xTimerStop(timer, 0))return -EMEM;
    }

    return 0;
}

/**************************************************************************************
* FunctionName   : softimer_reset()
* Description    : 重置定时器
* EntryParameter : timer,定时器指针, isr,中断调用使能(1,中断, 0,非中中断)
* ReturnValue    : 返回错误码
**************************************************************************************/
int8_t softimer_reset(softimer_t timer, uint8_t isr)
{
    if(0 != isr){
        if(pdTRUE != xTimerResetFromISR(timer, pdFALSE))return -EMEM;
    }else{
        if(pdTRUE != xTimerReset(timer, 0))return -EMEM;
    }

    return 0;
}

/**************************************************************************************
* FunctionName   : sorftimer_delete()
* Description    : 删除定时器
* EntryParameter : timer,定时器指针
* ReturnValue    : None
**************************************************************************************/
void sorftimer_delete(softimer_t timer)
{
    xTimerDelete(timer, 0);
}
#endif
