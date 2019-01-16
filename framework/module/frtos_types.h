/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : frtos_types.h
 * Author        :
 * Date          : 2017-08-17
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#ifndef __FRTOS_TYPES_H__
#define __FRTOS_TYPES_H__

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <math.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"

/**************************************************************************************
* Description    : 宏参数定义
**************************************************************************************/
#ifndef __const
#define __const     const
#endif
#ifndef __data
#define __data      __attribute__((__section__(".data")))
#endif
#ifndef __init
#define __init      __attribute__((__section__(".text.init")))
#endif
#ifndef __unused
#define __unused    __attribute__((unused))
#endif
#ifndef __used
#define __used    __attribute__((used))
#endif
#ifndef __default
#define __default   __attribute__((weak))
#endif
#ifndef __packed
#define __packed    __attribute__((packed))
#endif
#ifndef __barrier
#define __barrier()   __asm__ __volatile__("":::"memory")
#endif
#ifndef __optimize
#define __optimize(level) __attribute__((optimize(level)))
#endif
#ifndef __get_caller
#define __get_caller(i)              __builtin_return_address(i)
#endif
#ifndef __get_frame_pointer
#define __get_frame_pointer(i)       __builtin_frame_address(i)
#endif

/**************************************************************************************
* Description    : 变长数据结构定义
**************************************************************************************/
typedef struct{
    uint8_t len;                                            // 数据长度
    uint8_t *data;                                          // 数据指针
}nbytes;

/**************************************************************************************
* Description    : 8字节长度数组
**************************************************************************************/
typedef struct{
    uint8_t len;                                            // 数组长度
    uint8_t data[8];                                        // 数组内容
}array8_t;

/**************************************************************************************
* Description    : 16字节长度数组
**************************************************************************************/
typedef struct{
    uint8_t len;                                            // 数组长度
    uint8_t data[16];                                       // 数组内容
}array16_t;

/**************************************************************************************
* Description    : 32字节长度数组
**************************************************************************************/
typedef struct{
    uint8_t len;                                            // 数组长度
    uint8_t data[32];                                       // 数组内容
}array32_t;

/**************************************************************************************
* Description    : 64字节长度数组
**************************************************************************************/
typedef struct{
    uint8_t len;                                            // 数组长度
    uint8_t data[64];                                       // 数组内容
}array64_t;

/**************************************************************************************
* Description    : 128字节长度数组
**************************************************************************************/
typedef struct{
    uint8_t len;                                            // 数组长度
    uint8_t data[128];                                      // 数组内容
}array128_t;

/**************************************************************************************
* Description    : 256字节长度数组
**************************************************************************************/
typedef struct{
    uint8_t len;                                            // 数组长度
    uint8_t data[256];                                      // 数组内容
}array256_t;

/**************************************************************************************
* Description    : 512字节长度数组
**************************************************************************************/
typedef struct{
    uint8_t len;                                            // 数组长度
    uint8_t data[512];                                      // 数组内容
}array512_t;

/**************************************************************************************
* Description    : 默认标记定义
**************************************************************************************/
#define STRBR               "\r\n"                          // 默认换行

/**************************************************************************************
* Description    : 默认数据定义
**************************************************************************************/
#define E10_1               ((double)10)                    // 10的1次方
#define E10_2               ((double)100)                   // 10的2次方
#define E10_3               ((double)1000)                  // 10的3次方
#define E10_4               ((double)10000)                 // 10的4次方
#define E10_5               ((double)100000)                // 10的5次方
#define E10_6               ((double)1000000)               // 10的6次方
#define PI                  ((double)3.14159265358979323846)// 圆周率

/**************************************************************************************
* Description    : 期望值优化
**************************************************************************************/
#define likely(exp)         __builtin_expect(!!(exp), 1)    // 期望位真
#define unlikely(Exp)       __builtin_expect(!!(Exp), 0)    // 期望为假

/**************************************************************************************
* Description    : 宏功能函数
**************************************************************************************/
#define SETBIT(val,bit)     (val |= (1 << bit))             // 设置BIT位
#define CLRBIT(val,bit)     (val &= ~(1 << bit))            // 清除BIT位
#define ISSETBIT(val,bit)   (val & (1 << bit))              // 判断是否置位
#define ARRAY_LEN(x)        (sizeof(x) / sizeof((x)[0]))    // 获取一个数组元素个数

#endif /*__FRTOS_TYPES_H__ */

