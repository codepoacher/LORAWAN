/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : frtos_time.h
 * Author        :
 * Date          : 2017-08-17
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#ifndef __FRTOS_TIME_H__
#define __FRTOS_TIME_H__

#include "frtos_types.h"

/**************************************************************************************
* Description    : 秒参数定义
**************************************************************************************/
#define TIME_SEC_DAY            86400       // 1天     = 86400秒
#define TIME_SEC_HOUR           3600        // 1小时    = 3600秒
#define TIME_SEC_MINUTE         60          // 1分钟    = 60秒
#define TIME_MIN_HOUR           60          // 1小时    = 60分钟
#define TIME_HUR_DAY            24          // 1天     = 24小时
#define TIME_DAY_YEAR           365         // 1年     = 365天
#define TIME_DAY_YEAR_LEAP      366         // 1闰年    = 366天
#define TIME_YEAR_START         1970        // 开始计时年份
#define TIME_YEAR_END           2099        // 结束计时年份

/**************************************************************************************
* Description    : 系统时间类型定义
**************************************************************************************/
typedef uint32_t time_sys_t;                // 从1970年至今的秒数

/**************************************************************************************
* Description    : 日期时间类型定义
**************************************************************************************/
typedef struct{
    uint16_t year;                          // 年
    uint8_t mon;                            // 月
    uint8_t day;                            // 日
    uint8_t hour;                           // 时
    uint8_t min;                            // 分
    uint8_t sec;                            // 秒
}time_date_t;

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************
* MacroName      : time_after()
* Description    : 时间a在时间b之后
* EntryParameter : type,时间值的有符号类型, a,b,时间值
* ReturnValue    : true,成立, false,不成立
**************************************************************************************/
#define time_after(type,a,b)        ((type)(b) - (type)(a) < 0)

/**************************************************************************************
* MacroName      : time_before()
* Description    : 时间a在时间b之前
* EntryParameter : type,时间值的有符号类型, a,b,时间值
* ReturnValue    : true,成立, false,不成立
**************************************************************************************/
#define time_before(type,a,b)       time_after(type, b, a)

/**************************************************************************************
* FunctionName   : time_leapyear()
* Description    : 判断闰年
* EntryParameter : year,年份
* ReturnValue    : true,闰年, false,平年
**************************************************************************************/
bool time_leapyear(const uint16_t year);

/**************************************************************************************
* FunctionName   : time_tick2ms()
* Description    : 时钟节拍转毫秒
* EntryParameter : tick,时钟节拍
* ReturnValue    : 毫秒值
**************************************************************************************/
uint32_t time_tick2ms(uint32_t tick);

/**************************************************************************************
* FunctionName   : time_ms2tick()
* Description    : 毫秒转时钟节拍
* EntryParameter : ms,毫秒
* ReturnValue    : 节拍值
**************************************************************************************/
uint32_t time_ms2tick(uint32_t ms);

/**************************************************************************************
* FunctionName   : time_stm2dtm()
* Description    : 系统时间转日期时间
* EntryParameter : stime,系统时间, stime,日期时间指针
* ReturnValue    : 返回错误码
**************************************************************************************/
int8_t time_stm2dtm(time_sys_t stime, time_date_t *dtime);

/**************************************************************************************
* FunctionName   : time_dtm2stm()
* Description    : 日期时间转系统时间
* EntryParameter : dtime,日期时间, stime,转换后的系统时间
* ReturnValue    : 返回错误码
**************************************************************************************/
int8_t time_dtm2stm(time_date_t *dtime, time_sys_t *stime);

/**************************************************************************************
* FunctionName   : time_btm2dtm()
* Description    : BCD时间转日期时间
* EntryParameter : btime,BCD格式时间, dtime,转换后的日期时间
* ReturnValue    : 返回错误码
**************************************************************************************/
int8_t time_btm2dtm(uint8_t btime[6], time_date_t *dtime);

/**************************************************************************************
* FunctionName   : time_dtm2btm()
* Description    : 日期时间转BCD时间
* EntryParameter : dtime,日期时间, btime,转换后的BCD格式时间,
* ReturnValue    : 返回错误码
**************************************************************************************/
int8_t time_dtm2btm(time_date_t *dtime, uint8_t btime[6]);

/**************************************************************************************
* FunctionName   : time_btm2stm()
* Description    : BCD时间转系统时间
* EntryParameter : btime,BCD格式时间, stime,转换后的系统时间
* ReturnValue    : 返回错误码
**************************************************************************************/
int8_t time_btm2stm(uint8_t btime[6], time_sys_t *stime);

/**************************************************************************************
* FunctionName   : time_stm2btm()
* Description    : 系统时间转BCD时间
* EntryParameter : stime,转换后的系统时间, btime,BCD格式时间
* ReturnValue    : 返回错误码
**************************************************************************************/
int8_t time_stm2btm(time_sys_t stime, uint8_t btime[6]);

/**************************************************************************************
* FunctionName   : time_setstime()
* Description    : 设置系统时间(1970到现在的秒数, 需要外部实现)
* EntryParameter : systime,系统时间
* ReturnValue    : 返回错误码
**************************************************************************************/
int8_t __default time_setstime(time_sys_t systime);

/**************************************************************************************
* FunctionName   : time_setbtime()
* Description    : 设置BCD时间
* EntryParameter : btime,BCD时间
* ReturnValue    : None
**************************************************************************************/
void time_setbtime(uint8_t btime[6]);

/**************************************************************************************
* FunctionName   : time_setdtime()
* Description    : 设置日期时间
* EntryParameter : dtime,日期时间结构指针
* ReturnValue    : None
**************************************************************************************/
void time_setdtime(time_date_t *dtime);

/**************************************************************************************
* FunctionName   : time_gettick()
* Description    : 获取系统时钟节拍
* EntryParameter : None
* ReturnValue    : 返回节拍数
**************************************************************************************/
uint32_t time_gettick(void);

/**************************************************************************************
* FunctionName   : time_getstime()
* Description    : 获取系统时间(1970到现在的秒数, 需要外部实现)
* EntryParameter : None
* ReturnValue    : 返回秒钟数
**************************************************************************************/
time_sys_t __default time_getstime(void);

/**************************************************************************************
* FunctionName   : time_getutctime()
* Description    : 获取UTC时间
* EntryParameter : None
* ReturnValue    : 返回秒钟数
**************************************************************************************/
time_sys_t time_getutctime(void);

/**************************************************************************************
* FunctionName   : time_getbtime()
* Description    : 读取BCD时间
* EntryParameter : time,返回BCD码时间(YY-MM-DD-hh-mm-ss)
* ReturnValue    : None
**************************************************************************************/
void time_getbtime(uint8_t btime[6]);

/**************************************************************************************
* FunctionName   : time_getdtime()
* Description    : 获取日期时间
* EntryParameter : dtime,返回日期时间结构指针
* ReturnValue    : None
**************************************************************************************/
void time_getdtime(time_date_t *dtime);

/**************************************************************************************
* FunctionName   : time_chkexpire()
* Description    : 检查时间到期
* EntryParameter : *oldtick,上次时间tick(如果超时,会被更新到当前tick值)
                   ms,间隔毫秒, update,是否更新时间
* ReturnValue    : true,到期, false,未到期
**************************************************************************************/
bool time_chkexpire(uint32_t *oldtick, uint32_t ms, bool update);

/**************************************************************************************
* FunctionName   : time_delayms()
* Description    : 循环延时
* EntryParameter : ms,延时毫秒
* ReturnValue    : None
**************************************************************************************/
void time_delayms(uint32_t ms);

/**************************************************************************************
* FunctionName   : time_btmcmp()
* Description    : BCD时间比较
* EntryParameter : btm1,BCD时间1, btm2,BCD时间2
* ReturnValue    : (< 0), btm1 < btm2, (= 0), btm1 == btm2, (> 0), btm1 > btm2
**************************************************************************************/
int8_t time_btmcmp(uint8_t btm1[6], uint8_t btm2[6]);

/**************************************************************************************
* FunctionName   : time_dtmcmp()
* Description    : 日期时间比较
* EntryParameter : dtm1,时间1, dtm2,时间2
* ReturnValue    : (< 0), dtm1 < dtm2, (= 0), dtm1 == dtm2, (> 0), dtm1 > dtm2
**************************************************************************************/
int8_t time_dtmcmp(time_date_t *dtm1, time_date_t *dtm2);

#endif /* __FRTOS_TIME_H__ */

