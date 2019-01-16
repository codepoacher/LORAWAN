/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : rtc_driver.c
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#if 0
#include "frtos_errno.h"
#include "frtos_drivers.h"
#include "frtos_lock.h"
#include "config_driver.h"
#include "rtc_driver.h"

/**************************************************************************************
* Description    : 模块内部数据定义
**************************************************************************************/
static mutex_lock_t rtc_mutex = NULL;    // 内部RTC访问锁
static RTC_HandleTypeDef hrtc;

/**************************************************************************************
* FunctionName   : RTC_WKUP_IRQHandler()
* Description    : RTC唤醒中断处理函数
* EntryParameter : None
* ReturnValue    : None 
* **************************************************************************************/
void RTC_IRQHandler(void)
{
	HAL_RTCEx_WakeUpTimerIRQHandler(&hrtc);
}
/**************************************************************************************
* FunctionName   : rtc_time_print()
* Description    : 打印时间
* EntryParameter : None.
* ReturnValue    : None.
**************************************************************************************/
static inline int8_t rtc_getdtime(struct tm_conf *dtime);
void rtc_time_print()
{
	struct tm_conf time;
	RTC_TimeTypeDef rtc_time;
	RTC_DateTypeDef rtc_date;

    // 2.获取当前时间
	memset(&rtc_time, 0, sizeof(rtc_time));
	memset(&rtc_date, 0, sizeof(rtc_date));
	HAL_RTC_GetTime(&hrtc, &rtc_time, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &rtc_date, RTC_FORMAT_BIN);
	time.tm_year = rtc_date.Year + 2000;
	time.tm_mon  = rtc_date.Month;
	time.tm_mday = rtc_date.Date;

	time.tm_hour = rtc_time.Hours;
	time.tm_min  = rtc_time.Minutes;
	time.tm_sec  = rtc_time.Seconds;
	time.tm_subsec  = rtc_time.SubSeconds;
	
	debug("\r\n%d-%d-%d %d:%d:%d:%d\r\n", time.tm_year, time.tm_mon, time.tm_mday, \
			time.tm_hour, time.tm_min, time.tm_sec, time.tm_subsec);
}
/**************************************************************************************
* FunctionName   : rtc_getdtime()
* Description    : 获取物理设备RTC时间
* EntryParameter : *dtime,返回获取的日期时间
* ReturnValue    : 返回错误码
**************************************************************************************/
static inline int8_t rtc_getdtime(struct tm_conf *dtime)
{
    int8_t err = 0;
	RTC_TimeTypeDef rtc_time;
	RTC_DateTypeDef rtc_date;

    // 1.上锁
    mutex_lock(rtc_mutex);

    // 2.获取当前时间
	memset(&rtc_time, 0, sizeof(rtc_time));
	memset(&rtc_date, 0, sizeof(rtc_date));
	HAL_RTC_GetTime(&hrtc, &rtc_time, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &rtc_date, RTC_FORMAT_BIN);
	dtime->tm_year = rtc_date.Year + 2000;
	dtime->tm_mon  = rtc_date.Month;
	dtime->tm_mday = rtc_date.Date;

	dtime->tm_hour = rtc_time.Hours;
	dtime->tm_min  = rtc_time.Minutes;
	dtime->tm_sec  = rtc_time.Seconds;
	dtime->tm_subsec  = rtc_time.SubSeconds;
    
	// 3.解锁
    mutex_unlock(rtc_mutex);

    return err;
}

/**************************************************************************************
* FunctionName   : rtc_setdtime()
* Description    : 设置物理设备RTC时间
* EntryParameter : *dtime,带设置的日期时间指针
* ReturnValue    : 返回错误码
**************************************************************************************/
static inline int8_t rtc_setdtime(const struct tm_conf *dtime)
{
	HAL_StatusTypeDef status;
	RTC_TimeTypeDef rtc_time;
	RTC_DateTypeDef rtc_date;

    // 1.上锁
	mutex_lock(rtc_mutex);


    // 2.设置日期时间
	memset(&rtc_time, 0, sizeof(rtc_time));
	memset(&rtc_date, 0, sizeof(rtc_date));

	rtc_time.Hours   = dtime->tm_hour;
	rtc_time.Minutes = dtime->tm_min;
	rtc_time.Seconds = dtime->tm_sec;
	rtc_time.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	rtc_time.StoreOperation = RTC_STOREOPERATION_RESET;
	status = HAL_RTC_SetTime(&hrtc, &rtc_time, RTC_FORMAT_BIN);
	if (status != HAL_OK)
		return -EINVAL;
	
	rtc_date.Year    = dtime->tm_year - 2000;
	rtc_date.Month   = dtime->tm_mon;
	rtc_date.Date    = dtime->tm_mday;
	rtc_date.WeekDay = dtime->tm_week;
	HAL_RTC_SetDate(&hrtc, &rtc_date, RTC_FORMAT_BIN);
	if (status != HAL_OK)
		return -EINVAL;

    // 5.解锁
    mutex_unlock(rtc_mutex);

    return 0;
}

/**************************************************************************************
* FunctionName   : rtc_read()
* Description    : 读
* EntryParameter : *args,参数, len,参数长度
* ReturnValue    : 返回读取的字节数, 返回错误码
**************************************************************************************/
static int32_t rtc_read(uint8_t idx, void *data, int32_t len)
{
    int16_t err = 0;
    struct tm_conf dtime;

    if(unlikely(NULL == data || sizeof(uint32_t) != len)){
        return -EINVAL;
    }

    // 2.获取RTC时间
    err = rtc_getdtime(&dtime);
    if(0 == err) err = len;

    // 3.时间转换
	*(struct tm_conf*)data = dtime;

    (void)idx;
    return err;
}

/**************************************************************************************
* FunctionName   : rtc_write()
* Description    : 写
* EntryParameter : *args,参数, len,参数长度
* ReturnValue    : 返回写入的字节数, 返回错误码
**************************************************************************************/
static int32_t rtc_write(uint8_t idx, void *data, int32_t len)
{
    int16_t err = 0;
	struct tm_conf *tm = NULL;

    if(unlikely(NULL == data || sizeof(uint32_t) != len)){
        return -EINVAL;
    }

    // 1.设置RTC时间
    err = rtc_setdtime(data);
    if(0 == err) err = len;

    (void)idx;
    return len;
}

/**************************************************************************************
* FunctionName   : deinit_gpio()
* Description    : 进入低功耗，拉低所有管脚
* EntryParameter : 
* ReturnValue    : 无
**************************************************************************************/
static void deinit_gpio()
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.Pin = GPIO_PIN_All ^ GPIO_PIN_13 ^ GPIO_PIN_14;
	GPIO_InitStructure.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure); 
	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure); 
	HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);
	HAL_GPIO_Init(GPIOD, &GPIO_InitStructure); 
	HAL_GPIO_Init(GPIOH, &GPIO_InitStructure);
	GPIO_InitStructure.Pin = GPIO_PIN_All ^ GPIO_PIN_13 ^ GPIO_PIN_15;
	GPIO_InitStructure.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStructure);

	GPIO_InitStructure.Pin = GPIO_PIN_13 | GPIO_PIN_15;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStructure);
}

/**************************************************************************************
* FunctionName   : rtc_sleep()
* Description    : 进入休眠
* EntryParameter : 休眠时间
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t rtc_sleep(uint32_t *time)
{
	uint32_t sleep_time = 0;
	//1、关闭外设
	fdrive_suspend();	
	deinit_gpio();	

	//2、计算休眠时间
	if(*time >= 32) return;
	sleep_time = *time * 32768 / 16;
	
	HAL_PWREx_EnableUltraLowPower();
	HAL_PWREx_EnableFastWakeUp();

	//3、Disable all used wakeup source	
	HAL_RTCEx_DeactivateWakeUpTimer(&hrtc);
	
	//4、定时休眠时间
	if (HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, sleep_time, RTC_WAKEUPCLOCK_RTCCLK_DIV16) != HAL_OK) {
		debug("sleep error\r\n");
	}
	//5、进入休眠stop模式
	__HAL_RCC_PWR_CLK_ENABLE();
	HAL_PWR_EnterSTOPMode(PWR_MAINREGULATOR_ON, PWR_STOPENTRY_WFI);

	__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
	__HAL_RTC_WAKEUPTIMER_EXTI_CLEAR_FLAG();

	//6、唤醒后重新初始化时钟
	clock_init();
	__HAL_RCC_PWR_CLK_ENABLE();

	//7、唤醒外设
	fdrive_wakeup();

	return 0;
}

/**************************************************************************************
* FunctionName   : rtc_ioctrl()
* Description    : 控制
* EntryParameter : *args,参数, len,参数长度
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t rtc_ioctrl(uint8_t idx, int32_t cmd, void *args, int32_t len)
{
    if(unlikely((NULL == args && len > 0) || \
        (NULL != args && len <= 0) || len < 0)){
        return -EINVAL;
    }
	
    // 1.执行控制命令
    switch(cmd){
    case _IOC_RTC_SETTIME:
        return rtc_setdtime(args);
	case _IOC_RTC_SLEEP:
		return rtc_sleep(args);
    default:
        return -EINVAL;
    }

    (void)idx;
    return 0;
}

/**************************************************************************************
* FunctionName   : rtc_phy_init()
* Description    : rtc设备硬件初始化
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
static int32_t rtc_phy_init()
{
	RTC_TimeTypeDef sTime;
	RTC_DateTypeDef sDate;

	hrtc.Instance = RTC;
  	hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  	hrtc.Init.AsynchPrediv = 127;
  	hrtc.Init.SynchPrediv = 255;
  	hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
	hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  	hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  	hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  	if (HAL_RTC_Init(&hrtc) != HAL_OK)
		return -EINVAL;

	sDate.WeekDay = RTC_WEEKDAY_MONDAY;
	sDate.Month = RTC_MONTH_JANUARY;
	sDate.Date = 0x1;
	sDate.Year = 0x18;

	if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
		return -EINVAL;

	return 0;
}

/**************************************************************************************
* FunctionName   : rtc_init()
* Description    : 设备初始化
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
static int32_t __init rtc_init(void)
{
	/* 1.硬件初始化 */
	rtc_phy_init();

	/* 2.锁初始化 */
    rtc_mutex = mutex_lock_init();
    if(NULL == rtc_mutex) return -EPERM;

    return 0;
}
/*
static __const struct driver rtc = {
    .idx   = DRIVER_RTC,
    .init  = rtc_init,
    .read  = rtc_read,
    .write = rtc_write,
    .ioctl = rtc_ioctrl,
};
*/
/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
//MODULE_INIT(rtc);
#endif
