/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : system.c
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/

#include "frtos_mem.h"
#include "frtos_list.h"
#include "frtos_app.h"
#include "frtos_errno.h"
#include "frtos_sys.h"
#include "frtos_drivers.h"
#include "stm32l0xx_hal.h"
#include "cmsis_os.h"
#include "config_driver.h"

#define APPLITE_DEPTH_MGR                   1024                // 管理任务栈深度

/**************************************************************************************
* Description    : 驱动模块安装地址
**************************************************************************************/
extern struct driver *__DRIVER_LIST0_S__[];
extern struct driver *__DRIVER_LIST0_E__[];
extern struct driver *__DRIVER_LIST1_S__[];
extern struct driver *__DRIVER_LIST7_E__[];

/**************************************************************************************
* Description    : 轻量级任务安装地址
**************************************************************************************/
extern struct applite * __APP_LIST_CHAIN_S__[];
extern struct applite * __APP_LIST_CHAIN_E__[];

/**************************************************************************************
* Description    : 普通任务安装地址
**************************************************************************************/
extern struct task * __TASK_LIST_CHAIN_S__[];
extern struct task * __TASK_LIST_CHAIN_E__[];

/**************************************************************************************
* Description    : 模块内部数据定义
**************************************************************************************/
static TaskHandle_t mgrhdle = NULL;           // 系统管理任务堆栈
static osThreadId app_handle;

/**************************************************************************************
* Description    : 定义系统堆内存
**************************************************************************************/
#if( configAPPLICATION_ALLOCATED_HEAP == 1 )
uint8_t __attribute__((section(".ucheap")))  ucHeap[FRTOS_UCHEAP_SIZE];
#endif

/**************************************************************************************
* FunctionName   : vApplicationStackOverflowHook()
* Description    : 栈溢出回调函数
* EntryParameter : xTask,任务, pcTaskName,任务名称
* ReturnValue    : None
**************************************************************************************/
void __default vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;(void)pcTaskName;while(1);
}

/**************************************************************************************
* FunctionName   : vApplicationMallocFailedHook()
* Description    : Malloc失败回调函数
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
void __default vApplicationMallocFailedHook(void)
{
//	fdrive_write(DRIVER_LPUART, "failed hook\r\n", strlen("failed hook\r\n"));
    while(1);
}

/**************************************************************************************
* FunctionName   : StartIdleMonitor()
* Description    : 启动空闲模拟器
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
void __default StartIdleMonitor (void)
{
}

/**************************************************************************************
* FunctionName   : EndIdleMonitor()
* Description    : 结束空闲模拟器
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
void __default EndIdleMonitor (void)
{
}

/**************************************************************************************
* MacroName      : arch_suspend
* Description    : 系统CPU进入休眠模式
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
int32_t __default arch_suspend(void)
{
    return 0;
}


/**************************************************************************************
* MacroName      : arch_reboot
* Description    : 系统重启
* EntryParameter : magic,幻术
* ReturnValue    : 返回错误码
**************************************************************************************/
int32_t __default arch_reboot(uint32_t magic)
{
    (void)magic;
    return 0;
}

/**************************************************************************************
* MacroName      : fuser_data_set
* Description    : 设置数据
* EntryParameter : idx,应用ID，data,设置应用数据， len数据长度
* ReturnValue    : 返回错误码
**************************************************************************************/
int32_t fuser_data_set(uint8_t idx, void *data, int32_t len)
{
    struct applite **applites = NULL;
    struct task **tasks = NULL;

    // 1.查找轻量级任务应用程序
    for (applites = __APP_LIST_CHAIN_S__; applites < __APP_LIST_CHAIN_E__; applites++){
        if(unlikely((*applites)->idx == idx)) {
            if(likely((*applites)->set)) return (*applites)->set(idx, data, len);
            else return -EACCES;
        }
    }

    // 2.查找普通任务
    for(tasks = __TASK_LIST_CHAIN_S__; tasks < __TASK_LIST_CHAIN_E__; tasks++){
        if(unlikely((*tasks)->idx == idx)) {
            if(likely((*tasks)->set)) return (*tasks)->set(idx, data, len);
            else return -EACCES;
        }
    }
    return -ESRCH;
}

/**************************************************************************************
* MacroName      : fuser_data_get
* Description    : 获取数据
* EntryParameter : idx,应用ID，data,获取应用数据， len数据长度
* ReturnValue    : 返回错误码
**************************************************************************************/
int32_t fuser_data_get(uint8_t idx, void *data, int32_t len)
{
    struct applite **applites = NULL;
    struct task **tasks = NULL;

    // 2.查找轻量级任务应用程序
    for (applites = __APP_LIST_CHAIN_S__; applites < __APP_LIST_CHAIN_E__; applites++){
        if(unlikely((*applites)->idx == idx)) {
            if(likely((*applites)->get)) return (*applites)->get(idx, data, len);
            else return -EACCES;
        }
    }

    // 3.查找普通任务
    for(tasks = __TASK_LIST_CHAIN_S__; tasks < __TASK_LIST_CHAIN_E__; tasks++){
        if(unlikely((*tasks)->idx == idx)) {
            if(likely((*tasks)->get)) return (*tasks)->get(idx, data, len);
            else return -EACCES;
        }
    }
    return -ESRCH;
}

/**************************************************************************************
* MacroName      : fdrive_read
* Description    : 读取驱动数据
* EntryParameter : idx,驱动ID，data,数据， len数据长度
* ReturnValue    : 返回错误码或者长度
**************************************************************************************/
int32_t fdrive_read(uint8_t idx, void *data, int32_t len)
{
    struct driver **drivers = NULL;

    // 1.查找所有內核驱动模块
    for (drivers = __DRIVER_LIST0_S__; drivers < __DRIVER_LIST7_E__; drivers++){
        if(unlikely((*drivers)->idx == idx)) {
            if(likely((*drivers)->read)) return (*drivers)->read(idx, data, len);
            else return -EACCES;
        }
    }
    return -ENODEV;
}

/**************************************************************************************
* MacroName      : fdrive_write
* Description    : 写驱动数据
* EntryParameter : idx,驱动ID，data,数据， len数据长度
* ReturnValue    : 返回错误码或者写入长度
**************************************************************************************/
int32_t fdrive_write(uint8_t idx, void *data, int32_t len)
{
    struct driver **drivers = NULL;

    // 1.查找所有內核驱动模块
    for (drivers = __DRIVER_LIST0_S__; drivers < __DRIVER_LIST7_E__; drivers++){
        if(unlikely((*drivers)->idx == idx)) {
            if(likely((*drivers)->write)) return (*drivers)->write(idx, data, len);
            else return -EACCES;
        }
    }
    return -ENODEV;
}

/**************************************************************************************
* MacroName      : fdrive_ioctl
* Description    : 控制驱动程序
* EntryParameter : idx,驱动ID，cmd,控制命令字，data,数据， len数据长度
* ReturnValue    : 返回错误码
**************************************************************************************/
int32_t fdrive_ioctl(uint8_t idx,int32_t cmd, void *data, int32_t len)
{
    struct driver **drivers = NULL;

    // 1.查找所有內核驱动模块
    for (drivers = __DRIVER_LIST0_S__; drivers < __DRIVER_LIST7_E__; drivers++){
        if(unlikely((*drivers)->idx == idx)) {
            if(likely((*drivers)->ioctl)) return (*drivers)->ioctl(idx, cmd, data, len);
            else return -EACCES;
        }
    }
    return -ENODEV;
}

/**************************************************************************************
* MacroName      : fdrive_suspend
* Description    : 控制驱动进入休眠
* EntryParameter : idx,驱动ID
* ReturnValue    : 返回错误码
**************************************************************************************/
int32_t fdrive_suspend(void)
{
    struct driver **drivers = NULL;

    // 1.查找所有內核驱动模块
    for (drivers = __DRIVER_LIST0_S__; drivers < __DRIVER_LIST7_E__; drivers++){
        if((*drivers)->suspend) (*drivers)->suspend();
    }
    arch_suspend();

    return 0;
}

/**************************************************************************************
* MacroName      : fdrive_wakeup
* Description    : 控制驱动唤醒
* EntryParameter : idx,驱动ID
* ReturnValue    : 返回错误码
**************************************************************************************/
int32_t fdrive_wakeup(void)
{
    struct driver **drivers = NULL;

    // 1.查找所有內核驱动模块
    for (drivers = __DRIVER_LIST0_S__; drivers < __DRIVER_LIST7_E__; drivers++){
        if((*drivers)->wakeup) (*drivers)->wakeup();
    }
    return 0;
}

/**************************************************************************************
* MacroName      : fsystem_reboot
* Description    : 系统重启
* EntryParameter : magic,幻术
* ReturnValue    : 返回错误码
**************************************************************************************/
int32_t fsystem_reboot(uint32_t magic)
{
    struct driver **drivers = NULL;

    // 1.查找所有內核驱动模块
    for (drivers = __DRIVER_LIST0_S__; drivers < __DRIVER_LIST7_E__; drivers++){
        if(likely((*drivers)->stop)) (*drivers)->stop();
    }
    arch_reboot(magic);

    return -ENODEV;
}

/**************************************************************************************
* FunctionName   : app_master()
* Description    : 主任务
* EntryParameter : args,参数指针
* ReturnValue    : None
**************************************************************************************/

static void app_master(void const* args)
{
	int i = 0;
    struct driver **drivers = NULL;
    struct applite **applites = NULL;
    struct task **tasks = NULL;


    // 1.初始化所有內核驱动模块
    for (drivers = __DRIVER_LIST1_S__; drivers < __DRIVER_LIST7_E__; drivers++) {
        if(likely((*drivers)->init)) { 
			((*drivers)->init)();
	//		debug("driver:%d\r\n", (*drivers)->idx);
		}
    }

    // 2.初始化轻量级任务应用程序
    for (applites = __APP_LIST_CHAIN_S__; applites < __APP_LIST_CHAIN_E__; applites++) {
        if(likely((*applites)->init)) {
			((*applites)->init)();
	//		debug("app init:%s id:%d\r\n", (*applites)->name, (*applites)->idx);
		}
    }

    // 3.创建普通任务
    for(tasks = __TASK_LIST_CHAIN_S__; tasks < __TASK_LIST_CHAIN_E__; tasks++) {
        if(likely((*tasks)->main)) {
            xTaskCreate((*tasks)->main, (*tasks)->name, (*tasks)->depth, NULL, (*tasks)->pro, NULL);
        }
    }
	vTaskDelay(pdMS_TO_TICKS(5000));	
    // 6.执行管理任务
    while(1) {
        // 1.驱动程序周期执行
        for (drivers = __DRIVER_LIST0_S__; drivers < __DRIVER_LIST7_E__; drivers++) {
            if(unlikely((*drivers)->run)) {
				((*drivers)->run)();
		//		debug("driver run:%d\r\n", (*drivers)->idx);
			}
        }
        // 2.初始化轻量级任务应用程序
        for (applites = __APP_LIST_CHAIN_S__; applites < __APP_LIST_CHAIN_E__; applites++) {
            if(likely((*applites)->run)) {
				((*applites)->run)();
		//		debug("app run:%d name:%s\r\n", (*applites)->idx, (*applites)->name);
			}
        }
		//vTaskDelay(pdMS_TO_TICKS(APP_MGR_TASK_TIME));
    }
    (void)args;
}

/**************************************************************************************
* FunctionName   : os_start()
* Description    : 主程序入口
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
int os_start(void)
{
    struct driver **drivers = NULL;

	//1.hal库初始化
	HAL_Init();

	//2.体系结构相关初始化, 例如系统时钟
	for (drivers = __DRIVER_LIST0_S__; drivers < __DRIVER_LIST0_E__; drivers++) {
        if(likely((*drivers)->init)) ((*drivers)->init)();
    }

    // 4.创建管理任务
	osThreadDef(app_task, app_master, osPriorityNormal, 0, 256);
	app_handle = osThreadCreate(osThread(app_task), NULL);

	// 5.启动任务调度
	osKernelStart();
    while(1) ;
}

