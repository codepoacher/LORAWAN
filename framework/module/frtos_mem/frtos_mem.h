/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : frtos_mem.h
 * Author        :
 * Date          : 2017-08-17
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#ifndef __FRTOS_MEM_H__
#define __FRTOS_MEM_H__

#include "FreeRTOS.h"
#include "frtos_types.h"

/**************************************************************************************
* MacroName      : mem_malloc()
* Description    : 申请内存空间
* EntryParameter : size,申请的字节数
* ReturnValue    : 返回内存指针, NULL,申请失败
**************************************************************************************/
#define mem_malloc(size)        pvPortMalloc(size)

/**************************************************************************************
* MacroName      : mem_free()
* Description    : 释放内存空间
* EntryParameter : pv,内存指针
* ReturnValue    : None
**************************************************************************************/
#define mem_free(pv)            do {vPortFree(pv);pv = NULL;}while(0)

#endif /*__FRTOS_MEM_H__ */

