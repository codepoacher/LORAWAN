/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : frtos_dlyrun.c
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#include "frtos_app.h"
#include "frtos_mem.h"
#include "frtos_time.h"
#include "frtos_errno.h"
#include "frtos_softimer.h"
#include "frtos_lock.h"
#include "frtos_tasklet.h"
#include "frtos_drivers.h"

/**************************************************************************************
* Description    : 模块内部数据定义
**************************************************************************************/
//static mutex_lock_t tasklet_mutex = NULL;              // 互斥访问锁
static uint16_t tasklet_total = 0;
static softimer_t tasklet_timer = NULL;                /* 定时器 */

/**************************************************************************************
* Description    : 模块内部数据定义
**************************************************************************************/
static struct list_head tasklet_list = LIST_HEAD_INIT(tasklet_list);

/**************************************************************************************
* FunctionName   : tasklet_cancel()
* Description    : 取消延后执行函数
* EntryParameter : work_fn,延时执行回调, ms,延时毫秒数
* ReturnValue    : 返回错误码
**************************************************************************************/
int32_t tasklet_cancel(workqueue_fn_t work_fn)
{
    struct workqueue *work, *tmp;

    list_for_each_entry_safe(work, tmp, &tasklet_list, head) {
    	if(work_fn != work->run) continue;
    	list_del(&(work->head));
        mem_free(work);
        tasklet_total--;
        return 0;
	}

    return -ENODEV;
}

/**************************************************************************************
* FunctionName   : tasklet_schedule()
* Description    : 调度函数
* EntryParameter : work_fn,延时执行回调, ms,延时毫秒数
* ReturnValue    : 返回错误码
**************************************************************************************/
int32_t tasklet_schedule(workqueue_fn_t work_fn, void *args, uint32_t ms)
{
    struct workqueue *work = NULL;

    // 1.申请延时链表节点
    work = mem_malloc(sizeof(struct workqueue));
    if(NULL == work) return -ENOMEM;

    // 2.填充数据
    work->jffies = time_gettick() + time_ms2tick(ms);
    work->run = work_fn;
    work->args = args;

    // 3.上锁
    //mutex_lock(tasklet_mutex);

    // 4.添加到发送链表
    list_add_tail(&work->head, &tasklet_list);
    tasklet_total++;

    // 5.解锁
    //mutex_unlock(tasklet_mutex);

    return 0;
}

/*************************************************************************************
* FunctionName   : tasklet_isr()
* Description    : tasklet中断回调
* EntryParameter : None
* ReturnValue    : 返回错误码
*************************************************************************************/
static void tasklet_isr(softimer_t timer)
{
    struct workqueue *work = NULL, *tmp = NULL;

    list_for_each_entry_safe(work, tmp, &tasklet_list, head) {
        if(true != time_after(int32_t, time_gettick(), work->jffies))
            continue;

        list_del(&(work->head));
        work->run(work->args);
        mem_free(work);
        tasklet_total--;
	}
    (void)timer;
}

/**************************************************************************************
* FunctionName   : tasklet_init()
* Description    : 初始化tasklet功能
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t __init tasklet_init(void)
{
    //tasklet_mutex = mutex_lock_init();
    tasklet_timer = softimer_create(tasklet_isr, SOFTIMER_RELOAD_ENABLE, 10);
    if(NULL == tasklet_timer) return -EMEM;

    return softimer_start(tasklet_timer, 0);
}

static __const struct driver frtos_tasklet = {
    .init = tasklet_init,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
EARLY_INIT(frtos_tasklet);
