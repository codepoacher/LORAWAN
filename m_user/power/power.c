/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : power.c
 * Author        :
 * Date          : 2017-07-09
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#if 0
#include "frtos_app.h"
#include "frtos_mem.h"
#include "frtos_errno.h"
#include "frtos_lock.h"
#include "config_user.h"
#include "config_driver.h"
#include "frtos_ioctl.h"
#include "gpio_driver.h"
#include "frtos_sys.h"
#include "frtos_tasklet.h"
#include "frtos_gpio.h"
#include "frtos_irq.h"
#include "data.pb-c.h"
#include "power_driver.h"
#include "debug.h"

/**************************************************************************************
* Description    : 定义GPIO数据最大长度
**************************************************************************************/
#define GPIO_SIGNAL_ACC              GPIO(A,0)               // ACC
#define PULSE_POWER_12V_IRQ          129
/**************************************************************************************
* Description    : 模块内部数据定义
**************************************************************************************/
static uint32_t interval = 200;                             // 电源检查间隔，默认200ms
static uint32_t power_12v_status = 0;                       // 摩托车电瓶上电状态
static uint32_t power_mpu_status = 0;                       // MPU模块上电状态
static uint32_t power_3_3v_status = 0;                      // 3.3V电源开关状态
static uint32_t power_gps_status = 0;                       // GPS模块上电状态
static uint32_t power_sensor_status = 0;                    // 六轴传感器工作状态，0-低功耗模式， 1-正常工作模式
static uint32_t power_wifi_tf_status = 0;                   // WIFI模块上电状态
static uint32_t power_can_status = 0;                       // CAN/K模块上电状态
static uint32_t power_gprs_status = 0;                      // GPRS模块上电状态
static uint32_t ready_for_sleep = 0;                        // 是否准备好休眠
static uint32_t sleep_interval = 720;                       // 休眠的时间长度，单位为分钟
/**************************************************************************************
* FunctionName   : power_config()
* Description    : 电源配置
* EntryParameter : data，指向发送的数据， len,指向发送数据长度
* ReturnValue    : 返回发送状态或者长度
**************************************************************************************/
static int32_t power_config(uint8_t idx, void *data, int32_t len)
{
    uint32_t i = 0;
    Subid *msg = NULL;

    // 1.解开通用子协议数据头
    msg = subid__unpack(NULL,len, data);
    if(unlikely(msg == NULL)) return -EFAULT;

    // 2.释放内存
    subid__free_unpacked(msg, NULL);

    (void)idx;
    return len;
}

/**************************************************************************************
* FunctionName   : mpu_is_powered_off()
* Description    : 在MPU完成关机操作之后允许系统进入休眠
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
static void mpu_is_powered_off(void)
{
	ready_for_sleep = 1;
}

/**************************************************************************************
* FunctionName   : set_wakeup_interval()
* Description    : MCU进入低功耗休眠模式
* EntryParameter : 期望的休眠时间长度，单位为秒
* ReturnValue    : None
**************************************************************************************/
static int32_t set_wakeup_interval(void)
{
	uint32_t interval = sleep_interval * 60;
	return fdrive_ioctl(DRIVER_RTC, _IOC_RTC_SLEEP, &interval, sizeof(interval));
}

/**************************************************************************************
* FunctionName   : deinit_gpio()
* Description    : 配置大部分MCU管脚为输入模式用于降低MCU休眠功耗
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
static void deinit_gpio()
{
	GPIO_InitTypeDef GPIO_InitStruct;

	GPIO_InitStruct.Pin = GPIO_PIN_All ^ GPIO_PIN_0 ^ GPIO_PIN_13 ^ GPIO_PIN_14;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  
	GPIO_InitStruct.Pin = GPIO_PIN_All;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
   
	GPIO_InitStruct.Pin = GPIO_PIN_All ^ GPIO_PIN_5 ^ GPIO_PIN_13;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

/**************************************************************************************
* FunctionName   : enter_sleep()
* Description    : MCU进入低功耗休眠模式
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
static void enter_sleep(void)
{
	int ret = -1;

	if(ready_for_sleep == 0)
		return;
	deinit_gpio();
	ret = set_wakeup_interval();
	if(ret < 0 ){
		return;
	}
	//休眠醒来后从此处继续执行
//	__HAL_RCC_PWR_CLK_ENABLE();
	//此处需要恢复管脚的默认状态，重新初始化
	//recover_gpio_config();
	//fdrive_ioctl(DRIVER_POWER, POWER_CMD_REINIT, NULL, 0);
}

/**************************************************************************************
* FunctionName   : power_manage()
* Description    : 根据系统的供电状态进行电源管理
* EntryParameter : args, gpio结构信息
* ReturnValue    : None
**************************************************************************************/
/* 
*                      GPS电源开关
*                      --------/-------> 
*  3.3V电源开关      / 六轴电源开关
*  -----/----------------------/------->
*                    \ WIFI/TF电源开关
*                     -------/-------->
*                    \ CAN/K电源
*                     ---------------->
*/
static void power_manage(void *args)
{
    struct gpio io;
	
	io.gpio = GPIO(A,0); 
    // 1.检查12V管脚当前的状态(有12V电源为低，没有为高)
	if(fdrive_read(DRIVER_GPIO, (void *)&io, sizeof(struct gpio)) < 0) {
        return;
    }
	if(io.value == 0) {
		ready_for_sleep = 0;
		//电瓶供电状态
		if(power_3_3v_status == 0){
			fdrive_ioctl(DRIVER_POWER, POWER_CMD_3_3V_ON, NULL, 0);
			power_3_3v_status = 1;
		}
		if(power_mpu_status == 0){
			fdrive_ioctl(DRIVER_POWER, POWER_CMD_MPU_ON, NULL, 0);
			power_mpu_status = 1;
		}
		if(power_gps_status == 0){
			fdrive_ioctl(DRIVER_POWER, POWER_CMD_GPS_ON, NULL, 0);
			power_gps_status = 1;
		}
		if(power_sensor_status == 0){
			fdrive_ioctl(DRIVER_POWER, POWER_CMD_SENSOR_ON, NULL, 0);
			//fdrive_ioctl(DRIVER_SENSOR, _IOC_SET_PERF_MODE, NULL, 0);
			power_sensor_status = 1;
		}
		if(power_wifi_tf_status == 0){
			fdrive_ioctl(DRIVER_POWER, POWER_CMD_WIFI_TF_ON, NULL, 0);
			power_wifi_tf_status = 1;
		}
	}
	if(io.value == 1) {
		//电池供电状态
		ready_for_sleep = 1;
		if(power_3_3v_status == 1){
			fdrive_ioctl(DRIVER_POWER, POWER_CMD_3_3V_OFF, NULL, 0);
			power_3_3v_status = 0;
		}
		if(power_mpu_status == 1){
			fdrive_ioctl(DRIVER_POWER, POWER_CMD_MPU_OFF, NULL, 0);
			//注意此时的MPU电源并没有被关掉,有30s的时延
			power_mpu_status = 0;
			ready_for_sleep = 0;
			tasklet_schedule(mpu_is_powered_off, NULL, MPU_PWR_DOWN_DELAY);
		}
		if(power_gps_status == 1){
			fdrive_ioctl(DRIVER_POWER, POWER_CMD_GPS_OFF, NULL, 0);
			power_gps_status = 0;
		}
		if(power_sensor_status == 1){
			//fdrive_ioctl(DRIVER_SENSOR, _IOC_SET_LOW_MODE, NULL, 0);
			power_sensor_status = 0;
		}
		if(power_wifi_tf_status == 1){
			fdrive_ioctl(DRIVER_POWER, POWER_CMD_WIFI_TF_OFF, NULL, 0);
			power_wifi_tf_status = 0;
		}

	}
	
	enter_sleep();
}

/**************************************************************************************
* FunctionName   : wakeup_low_power()
* Description    : 唤醒低功耗模式下必须的子模块
* EntryParameter : irq,中断号
* ReturnValue    : None
**************************************************************************************/
static void wakeup_low_power(void)
{
	//打开CAN/K模块
	fdrive_ioctl(DRIVER_POWER, POWER_CMD_3_3V_ON, NULL, 0);
	power_3_3v_status = 1;
}

/**************************************************************************************
* FunctionName   : power_run()
* Description    : 电源模块周期任务
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static void power_run(void *args)
{
    power_manage(args);
    tasklet_schedule(power_run, args, interval);
}

/**************************************************************************************
* FunctionName   : power_isr()
* Description    : 摩托车电瓶上电中断回调
* EntryParameter : irq,中断号
* ReturnValue    : None
**************************************************************************************/
static void power_isr(uint32_t irq)
{
    (void)irq;
	tasklet_schedule(power_manage, NULL, 10);
}

/**************************************************************************************
* FunctionName   : timer_isr()
* Description    : 定时唤醒中断
* EntryParameter : irq,中断号
* ReturnValue    : None
**************************************************************************************/
static void timer_isr(uint32_t irq)
{
    (void)irq;
	tasklet_schedule(wakeup_low_power, NULL, 10);
}

/**************************************************************************************
* FunctionName   : power_init()
* Description    : 电源管理应用程序的初始化
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t power_init(void)
{
    struct irq_reg_s irq;

    // 1.设置12V上下电中断
    irq.trig = IRQ_TRIG_EDGE;
    irq.handler = power_isr;
    irq.irq = POWER_12V_INT;//IRQ(A,0);
    fdrive_ioctl(DRIVER_GPIOINT, 0, &irq, sizeof(struct irq_reg_s));

    // 2.启用调度
    tasklet_schedule(power_run, NULL, interval);
    return 0;
}

/**************************************************************************************
* Description    : 定义通信任务结构
**************************************************************************************/
static __const struct applite power = {
    .idx   = POWER_PID,
    .name  = "power",
    .init  = power_init,
    .set   = power_config,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
APP_REGISTER(power);
#endif
