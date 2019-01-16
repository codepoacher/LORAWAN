/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : trace.c
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#if 1
#include "frtos_drivers.h"
#include "frtos_trace.h" 
#include "stm32l0xx_hal.h" 
/**************************************************************************************
* FunctionName   : trace_msg_send()
* Description    : 打印MCU异常信息
* EntryParameter : data，打印数据内容,len数据内容长度
* ReturnValue    : 返回None
**************************************************************************************/
void trace_msg_send(uint8_t *data, uint32_t len)
{
    (void)data;(void)len;
}

/**************************************************************************************
* FunctionName   : trace_finished()
* Description    : 实现设备重启或者其他功能
* EntryParameter : None
* ReturnValue    : 返回None
**************************************************************************************/
void trace_finished(void)
{
    while(1);
}

/**
* @brief This function handles Non maskable interrupt.
*/
void NMI_Handler(void)
{
	//while(1);
	frtos_trace();
}

/**
* @brief This function handles Hard fault interrupt.
*/
void HardFault_Handler(void)
{
//	while(1) ;
	frtos_trace(); 
}

/**
* @brief This function handles Memory management fault.
*/
void MemManage_Handler(void)
{
	frtos_trace();
}

/**
* @brief This function handles Pre-fetch fault, memory access fault.
*/
void BusFault_Handler(void)
{
	frtos_trace();
}

/**
* @brief This function handles Undefined instruction or illegal state.
*/
void UsageFault_Handler(void)
{
	frtos_trace();
}

/**
* @brief This function handles System service call via SWI instruction.
*/
#if 0
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */

  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}
#endif

/**
* @brief This function handles Debug monitor.
*/
//void DebugMon_Handler(void)
//{
//  /* USER CODE BEGIN DebugMonitor_IRQn 0 */
//
//  /* USER CODE END DebugMonitor_IRQn 0 */
//  /* USER CODE BEGIN DebugMonitor_IRQn 1 */
//
//  /* USER CODE END DebugMonitor_IRQn 1 */
//}

/**************************************************************************************
* FunctionName   : backtrace_init()
* Description    : 注册系统异常中断
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t __init trace_init(void)
{
    // 注册ARM-M系列不可屏蔽中断
    /*INT_SYS_InstallHandler(NonMaskableInt_IRQn, frtos_trace, (isr_t *)0);

    // 注册ARM-M系列硬件异常中断
    INT_SYS_InstallHandler(HardFault_IRQn, frtos_trace, (isr_t *)0);

    // 注册ARM-M系列内存异常中断
    INT_SYS_InstallHandler(MemoryManagement_IRQn, frtos_trace, (isr_t *)0);

    INT_SYS_InstallHandler(BusFault_IRQn, frtos_trace, (isr_t *)0);
    INT_SYS_InstallHandler(UsageFault_IRQn, frtos_trace, (isr_t *)0);*/

    return 0;
}

static __const struct driver trace = {
    .init = trace_init,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
ARCH_INIT(trace);
#endif
