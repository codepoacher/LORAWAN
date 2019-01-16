/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : frtos_irq.c
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#include "frtos_app.h"
#include "frtos_mem.h"
#include "frtos_errno.h"
#include "frtos_lock.h"
#include "frtos_irq.h"

/**************************************************************************************
* Description    : 模块内部数据定义
**************************************************************************************/
static uint8_t irq_total = 0;
static struct list_head irq_list = LIST_HEAD_INIT(irq_list);

/**************************************************************************************
* FunctionName   : irq_set_trigger()
* Description    : 中断触发模式设置(需要外部实现)
* EntryParameter : irq,中断号, trig,触发模式
* ReturnValue    : 返回错误码
**************************************************************************************/
int8_t __default irq_set_trigger(uint8_t irq, uint8_t trig)
{
    (void)irq;(void)trig;return 0;
}

/**************************************************************************************
* FunctionName   : irq_enable()
* Description    : 使能中断触发模式(需要外部实现)
* EntryParameter : irq,中断号
* ReturnValue    : 返回错误码
**************************************************************************************/
int8_t __default irq_enable(uint8_t irq)
{
    (void)irq;return 0;
}

/**************************************************************************************
* FunctionName   : irq_disable()
* Description    : 禁用中断触发模式(需要外部实现)
* EntryParameter : irq,中断号
* ReturnValue    : 返回错误码
**************************************************************************************/
int8_t __default irq_disable(uint8_t irq)
{
    (void)irq;return 0;
}

/**************************************************************************************
* FunctionName   : irq_fast_dispatch()
* Description    : 快速中断服务调度器
* EntryParameter : irq,中断号
* ReturnValue    : 返回错误码
**************************************************************************************/
int32_t irq_fast_dispatch(uint32_t irq)
{
    struct irq_reg_s *irq_reg = NULL, *tmp;

    list_for_each_entry_safe(irq_reg, tmp, &irq_list, list) {
        if(irq_reg->irq == irq && irq_reg->handler) {
            irq_reg->handler(irq);
            return 0;
        }
	}

    return -ENODEV;
}

int32_t irq_fast_dispatch_by_pin(uint16_t pin)
{
    struct irq_reg_s *irq_reg = NULL, *tmp;

    list_for_each_entry_safe(irq_reg, tmp, &irq_list, list) {
        if(irq_reg->irq == pin && irq_reg->handler) {
            irq_reg->handler(irq_reg->irq);
            return 0;
        }
	}

    return -ENODEV;
}

/**************************************************************************************
* FunctionName   : request_irq()
* Description    : 注册中断函数
* EntryParameter : irq,中断号, trig，触发模式，handler，中断函数
* ReturnValue    : 返回错误码
**************************************************************************************/
int32_t request_irq(uint32_t irq, uint8_t trig, irq_cb_t handler)
{
    struct irq_reg_s *irq_reg = mem_malloc(sizeof(struct irq_reg_s));

    if(irq_reg == NULL) return -ENOMEM;

    irq_reg->irq = irq;
    irq_reg->trig = trig;
    irq_reg->handler = handler;
    list_add_tail(&irq_reg->list, &irq_list);
    irq_set_trigger(irq, trig);
    irq_enable(irq);
    irq_total++;

    return 0;
}

/**************************************************************************************
* FunctionName   : free_irq()
* Description    : 释放中断函数
* EntryParameter : irq,中断号
* ReturnValue    : 返回错误码
**************************************************************************************/
int32_t free_irq(uint32_t irq)
{
    struct irq_reg_s *irq_reg = NULL, *tmp;

    irq_disable(irq);
    list_for_each_entry_safe(irq_reg, tmp, &irq_list, list) {
        if(irq_reg->irq == irq) {
            list_del(&(irq_reg->list));
            mem_free(irq_reg);
            irq_total--;
            return 0;
        }
	}
    return -ENODEV;
}

