/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : frtos_errno.h
 * Author        :
 * Date          : 2017-08-17
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#ifndef __FRTOS_ERRNO_H__
#define __FRTOS_ERRNO_H__

#include "FreeRTOS.h"

#define EPERM                                       1   /* 操作不被允许 */
#define ENOENT                                      2   /* 没有该文件或目录 */
#define ESRCH                                       3   /* 没有该任务 */
#define ENXIO                                       4   /* 没有该设备或者设备文件 */
#define EEXIST                                      5   /* 文件已存在 */
#define ENOTDIR                                     6   /* 不是一个目录 */
#define EPROTOTYPE                                  7   /* 协议类型错误 */
#define ENOPROTOOPT                                 8   /* 协议不可用 */
#define EPROTONOSUPPORT                             9  /* 协议不能被操作 */
#define ENETDOWN                                    10  /* 网络关闭 */
#define ENETUNREACH                                 11  /* 网络不可达 */
#define ESHUTDOWN                                   12  /* 对方已关闭连接 */
#define ECONNREFUSED                                13  /* 连接被拒绝 */
#define EHOSTDOWN                                   14  /* 主机已经关闭 */
#define EALREADY                                    15  /* 该操作已经被处理 */
#define EINPROGRESS                                 16  /* 该操作正在被处理 */

#define ENODEV                                      17  /* 没有该设备 */
#define EREMOTEIO                                   18  /* 远端IO错误 */
#define EMEM                                        19  /* 内存错误 */
#define EEMPTY                                      20  /* 队列空 */
#define EFULL                                       21  /* 队列满 */
#define EAGAIN                                      22  /* 重试 */
#define ENOMEM                                      23  /* 内存溢出 */
#define EACCES                                      24  /* 没有操作权限 */
#define EFAULT                                      25  /* 错误 */
#define ECREATE                                     26  /* 创建结构失败 */
#define EIO                                         27  /* I/O错误 */
#define EBUSY                                       28  /* 设备或地址忙 */
#define EINVAL                                      29  /* 无效的参数引用 */
#define EMSGSIZE                                    30  /* 消息长度过长 */
#define ETIMEDOUT                                   31  /* 连接超时 */

#endif /*__FRTOS_ERRNO_H__ */
