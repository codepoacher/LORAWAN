/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : frtos_time.c
 * Author        :
 * Date          : 2017-08-17
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#include "frtos_app.h"
#include "frtos_mem.h"
#include "frtos_utils.h"
#include "frtos_errno.h"
#include "frtos_time.h"

/**************************************************************************************
* Description    : 数据上锁
**************************************************************************************/
#define TIME_MUTEX_PEND()             vTaskSuspendAll()     // 上锁
#define TIME_MUTEX_POST()             xTaskResumeAll()      // 解锁

/**************************************************************************************
* Description    : 模块内部数据定义
**************************************************************************************/
static __const uint8_t time_amonday[] =                     // 平年每月天数
    {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static __const uint8_t time_lmonday[] =                     // 闰年每月天数
    {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static __const uint16_t time_amonaftday[] =                 // 平年每月经过的天数
    {0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

/**************************************************************************************
* FunctionName   : time_setstime()
* Description    : 设置系统时间(1970到现在的秒数, 需要外部实现)
* EntryParameter : systime,系统时间
* ReturnValue    : 返回错误码
**************************************************************************************/
int8_t __default time_setstime(time_sys_t systime)
{
    (void)systime;
    return 0;
}

/**************************************************************************************
* FunctionName   : time_getstime()
* Description    : 获取系统时间(1970到现在的秒数, 需要外部实现)
* EntryParameter : None
* ReturnValue    : 返回秒钟数
**************************************************************************************/
time_sys_t __default time_getstime(void)
{
    return 0;
}

/**************************************************************************************
* FunctionName   : time_leapyear()
* Description    : 判断闰年
* EntryParameter : year,年份
* ReturnValue    : true,闰年, false,平年
**************************************************************************************/
bool time_leapyear(const uint16_t year)
{
    if((year % 4)    > 0) return false;
    if((year % 100) > 0) return true;
    if((year % 400) > 0) return false;

    return true;
}

/**************************************************************************************
* FunctionName   : time_tick2ms()
* Description    : 时钟节拍转毫秒
* EntryParameter : tick,时钟节拍
* ReturnValue    : 毫秒值
**************************************************************************************/
uint32_t time_tick2ms(uint32_t tick)
{
    return (uint32_t)((tick/configTICK_RATE_HZ) * (uint32_t)1000);
}

/**************************************************************************************
* FunctionName   : time_ms2tick()
* Description    : 毫秒转时钟节拍
* EntryParameter : ms,毫秒
* ReturnValue    : 节拍值
**************************************************************************************/
uint32_t time_ms2tick(uint32_t ms)
{
    return pdMS_TO_TICKS(ms);
}

/**************************************************************************************
* FunctionName   : time_stm2dtm()
* Description    : 系统时间转日期时间
* EntryParameter : stime,系统时间, stime,日期时间指针
* ReturnValue    : 返回错误码
**************************************************************************************/
int8_t time_stm2dtm(time_sys_t stime, time_date_t *dtime)
{
    uint8_t i = 0;
    uint16_t year_days = TIME_DAY_YEAR;
    uint32_t day_num = stime / TIME_SEC_DAY;
    uint32_t sec_num =  stime % TIME_SEC_DAY;
    uint32_t month_num = 0;
    bool leap_year = false;

    if(unlikely(NULL == dtime)) return -EINVAL;

    dtime->year = TIME_YEAR_START;
    dtime->hour = (uint16_t)(sec_num / TIME_SEC_HOUR);
    sec_num = sec_num % TIME_SEC_HOUR;
    dtime->min = (uint16_t)(sec_num / TIME_SEC_MINUTE);
    dtime->sec = (uint8_t)(sec_num % TIME_SEC_MINUTE);

    while(day_num >= year_days){
        dtime->year++;
        day_num -= year_days;
        if(!time_leapyear(dtime->year)){
            year_days = TIME_DAY_YEAR;
        }else{
            year_days = TIME_DAY_YEAR_LEAP;
        }
    }

    day_num += 1;
    leap_year = time_leapyear(dtime->year);
    for(i = 1; i <= 12; i++){
        month_num = ((leap_year == true) ? \
            (uint32_t)time_lmonday[i] : (uint32_t)time_amonday[i]);
        if(day_num <= month_num){
            dtime->mon = (uint16_t)i;
            break;
        }else{
            day_num -= month_num;
        }
    }

    dtime->day = (uint16_t)day_num;

    return 0;
}

/**************************************************************************************
* FunctionName   : time_dtm2stm()
* Description    : 日期时间转系统时间
* EntryParameter : dtime,日期时间, stime,转换后的系统时间
* ReturnValue    : 返回错误码
**************************************************************************************/
int8_t time_dtm2stm(time_date_t *dtime, time_sys_t *stime)
{
    uint16_t year = 0;

    if(unlikely(NULL == stime || NULL == dtime)) return -EINVAL;

    if(dtime->mon > 12 || dtime->day > 31 || dtime->hour > 23 \
        || dtime->min > 59 || dtime->sec > 59){
        return -EINVAL;
    }

    (*stime) = (uint32_t)(TIME_DAY_YEAR * (uint32_t)(TIME_SEC_DAY));
    (*stime) *= ((uint32_t)dtime->year - TIME_YEAR_START);
    for(year = TIME_YEAR_START; year < dtime->year; year++){
        if(time_leapyear(year)){
            (*stime) += TIME_SEC_DAY;
        }
    }

    if((time_leapyear(year)) && (dtime->mon > 2U)){
        (*stime) += TIME_SEC_DAY;
    }

    (*stime) += time_amonaftday[dtime->mon] * TIME_SEC_DAY;
    (*stime) += (uint32_t)(((uint32_t)dtime->day - 1U) * (uint32_t)TIME_SEC_DAY);
    (*stime) += (uint32_t)(((uint32_t)dtime->hour * TIME_SEC_HOUR) + \
        ((uint32_t)dtime->min * TIME_SEC_MINUTE) + (uint32_t)dtime->sec);

    return 0;
}

/**************************************************************************************
* FunctionName   : time_btm2dtm()
* Description    : BCD时间转日期时间
* EntryParameter : btime,BCD格式时间, dtime,转换后的日期时间
* ReturnValue    : 返回错误码
**************************************************************************************/
int8_t time_btm2dtm(uint8_t btime[6], time_date_t *dtime)
{
    if(NULL == btime || NULL == dtime)return -EMEM;

    dtime->year  = utils_bcd2bin(btime[0]) + 2000;
    dtime->mon   = utils_bcd2bin(btime[1]);
    dtime->day   = utils_bcd2bin(btime[2]);
    dtime->hour  = utils_bcd2bin(btime[3]);
    dtime->min   = utils_bcd2bin(btime[4]);
    dtime->sec   = utils_bcd2bin(btime[5]);

    return 0;
}

/**************************************************************************************
* FunctionName   : time_dtm2btm()
* Description    : 日期时间转BCD时间
* EntryParameter : dtime,日期时间, btime,转换后的BCD格式时间,
* ReturnValue    : 返回错误码
**************************************************************************************/
int8_t time_dtm2btm(time_date_t *dtime, uint8_t btime[6])
{
    if(NULL == dtime || NULL == btime)return -EMEM;

    btime[0] = utils_bin2bcd(dtime->year - 2000);
    btime[1] = utils_bin2bcd(dtime->mon );
    btime[2] = utils_bin2bcd(dtime->day );
    btime[3] = utils_bin2bcd(dtime->hour);
    btime[4] = utils_bin2bcd(dtime->min );
    btime[5] = utils_bin2bcd(dtime->sec );

    return 0;
}

/**************************************************************************************
* FunctionName   : time_btm2stm()
* Description    : BCD时间转系统时间
* EntryParameter : btime,BCD格式时间, stime,转换后的系统时间
* ReturnValue    : 返回错误码
**************************************************************************************/
int8_t time_btm2stm(uint8_t btime[6], time_sys_t *stime)
{
    time_date_t dtime;

    if(NULL == btime || NULL == stime) return -EMEM;

    // 1.BCD时间转日期时间
    time_btm2dtm(btime, &dtime);

    // 2.日期时间转系统时间
    time_dtm2stm(&dtime, stime);

    return 0;
}

/**************************************************************************************
* FunctionName   : time_stm2btm()
* Description    : 系统时间转BCD时间
* EntryParameter : stime,转换后的系统时间, btime,BCD格式时间
* ReturnValue    : 返回错误码
**************************************************************************************/
int8_t time_stm2btm(time_sys_t stime, uint8_t btime[6])
{
    time_date_t dtime = {0,0,0,0,0,0};

    if(NULL == btime) return -EMEM;

    // 1.系统时间转日期时间
    if(0 != time_stm2dtm(stime, &dtime)) return -EMEM;

    // 2.日期时间转BCD时间
    time_dtm2btm(&dtime, btime);

    return 0;
}

/**************************************************************************************
* FunctionName   : time_setbtime()
* Description    : 设置BCD时间
* EntryParameter : btime,BCD时间
* ReturnValue    : None
**************************************************************************************/
void time_setbtime(uint8_t btime[6])
{
    time_sys_t stime = 0;

    // 1.BCD时间转系统时间
    time_btm2stm(btime, &stime);

    // 2.设置系统时间
    time_setstime(stime);
}

/**************************************************************************************
* FunctionName   : time_setdtime()
* Description    : 设置日期时间
* EntryParameter : dtime,日期时间结构指针
* ReturnValue    : None
**************************************************************************************/
void time_setdtime(time_date_t *dtime)
{
    time_sys_t stime = 0;

    // 1.将日期时间转换成系统时间
    time_dtm2stm(dtime, &stime);

    // 2.设置系统时间
    time_setstime(stime);
}

/**************************************************************************************
* FunctionName   : time_gettick()
* Description    : 获取系统时钟节拍
* EntryParameter : None
* ReturnValue    : 返回节拍数
**************************************************************************************/
uint32_t time_gettick(void)
{
    return xTaskGetTickCount();
}

/**************************************************************************************
* FunctionName   : time_getutctime()
* Description    : 获取UTC时间
* EntryParameter : None
* ReturnValue    : 返回秒钟数
**************************************************************************************/
time_sys_t time_getutctime(void)
{
    return time_getstime() - (8 * 60 * 60);
}

/**************************************************************************************
* FunctionName   : time_getdtime()
* Description    : 获取日期时间
* EntryParameter : dtime,返回日期时间结构指针
* ReturnValue    : None
**************************************************************************************/
void time_getdtime(time_date_t *dtime)
{
    // 1.读取系统时间,并转换成日期类型
    time_stm2dtm(time_getstime(), dtime);
}

/**************************************************************************************
* FunctionName   : time_getbtime()
* Description    : 读取BCD时间
* EntryParameter : time,返回BCD码时间(YY-MM-DD-hh-mm-ss)
* ReturnValue    : None
**************************************************************************************/
void time_getbtime(uint8_t btime[6])
{
    time_sys_t stime = 0;

    // 1.获取系统时间
    stime = time_getstime();

    // 2.将系统时间转换成BCD时间
    time_stm2btm(stime, btime);
}

/**************************************************************************************
* FunctionName   : time_chkexpire()
* Description    : 检查时间到期
* EntryParameter : *oldtick,上次时间tick(如果超时,会被更新到当前tick值)
                   ms,间隔毫秒, update,是否更新时间
* ReturnValue    : true,到期, false,未到期
**************************************************************************************/
bool time_chkexpire(uint32_t *oldtick, uint32_t ms, bool update)
{
    if(NULL == oldtick)return false;

    // 1.到期
    if(time_after(int32_t, time_gettick(), (*oldtick) + time_ms2tick(ms))){
        if(true == update)*oldtick = time_gettick();
        return true;
    }

    return false;
}

/**************************************************************************************
* FunctionName   : time_delayms()
* Description    : 循环延时
* EntryParameter : ms,延时毫秒
* ReturnValue    : None
**************************************************************************************/
void time_delayms(uint32_t ms)
{
    uint32_t oldtick = time_gettick();

    // 1.等待到期
    while(true != time_chkexpire(&oldtick, ms, false));
}

/**************************************************************************************
* FunctionName   : time_btmcmp()
* Description    : BCD时间比较
* EntryParameter : btm1,BCD时间1, btm2,BCD时间2
* ReturnValue    : (< 0), btm1 < btm2, (= 0), btm1 == btm2, (> 0), btm1 > btm2
**************************************************************************************/
int8_t time_btmcmp(uint8_t btm1[6], uint8_t btm2[6])
{
    uint8_t i = 0;
    uint8_t tm1 = 0, tm2 = 0;

    for(i = 0; i < 6; i++){
        tm1 = utils_bcd2bin(btm1[i]);
        tm2 = utils_bcd2bin(btm2[i]);
        if(tm1 != tm2){
            return (tm1 - tm2);
        }
    }

    return 0;
}

/**************************************************************************************
* FunctionName   : time_dtmcmp()
* Description    : 日期时间比较
* EntryParameter : dtm1,时间1, dtm2,时间2
* ReturnValue    : (< 0), dtm1 < dtm2, (= 0), dtm1 == dtm2, (> 0), dtm1 > dtm2
**************************************************************************************/
int8_t time_dtmcmp(time_date_t *dtm1, time_date_t *dtm2)
{
    if(dtm1->year  != dtm2->year  )return dtm1->year  - dtm2->year;
    if(dtm1->mon   != dtm2->mon   )return dtm1->mon   - dtm2->mon;
    if(dtm1->day   != dtm2->day   )return dtm1->day   - dtm2->day;
    if(dtm1->hour  != dtm2->hour  )return dtm1->hour  - dtm2->hour;
    if(dtm1->min   != dtm2->min   )return dtm1->min   - dtm2->min;
    if(dtm1->sec   != dtm2->sec   )return dtm1->sec   - dtm2->sec;

    return 0;
}
