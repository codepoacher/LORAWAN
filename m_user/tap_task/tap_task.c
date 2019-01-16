/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : tap_task.c
 * Author        :
 * Date          : 2018-10-26
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#include "cmsis_os.h"
#include "frtos_app.h"
#include "frtos_mem.h"
#include "frtos_errno.h"
#include "frtos_lock.h"
#include "frtos_queue.h"
#include "config_user.h"
#include "config_driver.h"
#include "frtos_ioctl.h"
#include "frtos_sys.h"
#include "gpio_driver.h"
#include "tap_control.h"
#include "e2prom.h"
#include "adc_driver.h"

float g_pulse = 0.0;
float g_pressure = 0.0;
/**************************************************************************************
* description    : 读e2prom压力流量一体计配置
**************************************************************************************/
static void press_conf_read(struct e2prom_press_dev *conf)
{
	struct e2prom_s edata;
	uint8_t conf_len = sizeof(struct e2prom_press_dev);

	edata.addr = E2PROM_PRESS_CONF_ADDR;
	edata.len  = conf_len;
	edata.data = conf;
	fdrive_read(DRIVER_E2PROM, &edata, sizeof(struct e2prom_s));
}

/**************************************************************************************
* description    : 任务主循环
**************************************************************************************/
static void tap1_task_run(void *args)
{
	struct e2prom_press_dev press_conf;

	while(1){
		/* 压力流量采集 */
		press_conf_read(&press_conf);
		//0、判断是否接压力流量计
		if (1 == press_conf.dev_mount) {
			//1、采集流量
			uint32_t fre = 0;
			fdrive_read(DRIVER_TIMER1, &fre, sizeof(uint32_t));
			g_pulse = fre * (*(float*)&press_conf.pulse_scale);

			//2、采集压力
			struct adc_samp_s adc_samp;
			adc_samp.ch = 2;
			adc_samp.scale = 1;
			adc_samp.zero = 430;
			fdrive_read(DRIVER_ADC, &adc_samp, sizeof(struct adc_samp_s));
			g_pressure = (*(float*)&adc_samp.result - *(float*)&press_conf.low_vol) / \
						 (*(float*)&press_conf.high_vol - *(float*)&press_conf.low_vol) * \
						 *(float*)&press_conf.press_range;
			if (g_pressure < 0.0) {
				g_pressure = 0.0;
			}

			//3、没有流量和压力值则更改阀门状态
			if (g_pulse < 0.021 && g_pressure < 0.05) {
				g_tap01_stats = 0;
				g_tap02_stats = 0;
			}
		}

		/* 阀门操作 */
	//	if (0 == fqueue_pop(g_tap_ctrl_queue, &tap_ctrl, 0, true)) {
	//		fdrive_write(DRIVER_TAP, tap_ctrl, sizeof(struct tap_action));
	//		mem_free(tap_ctrl);
	//	}
		fdrive_ioctl(DRIVER_TAP, _IOC_TAP1_HANDLE, NULL, 0);
		
	}
}

/**************************************************************************************
* description    : 任务主循环
**************************************************************************************/
static void tap2_task_run(void *args)
{

	while(1){
		fdrive_ioctl(DRIVER_TAP, _IOC_TAP2_HANDLE, NULL, 0);
	}
}

/**************************************************************************************
* description    : 定义任务结构
**************************************************************************************/
static __const struct task tap1_task  = {
    .idx   =  TAP1_TASK_PID,
    .name  = "tap1_task",
	.pro   = 3,
	.depth = 256,
	.main  = tap1_task_run, 
};

static __const struct task tap2_task  = {
    .idx   =  TAP2_TASK_PID,
    .name  = "tap2_task",
	.pro   = 3,
	.depth = 256,
	.main  = tap2_task_run, 
};
/**************************************************************************************
* description    : 任务注册
**************************************************************************************/
//TASK_REGISTER(tap1_task);
//TASK_REGISTER(tap2_task);
