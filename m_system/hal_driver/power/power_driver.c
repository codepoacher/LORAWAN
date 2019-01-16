/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : power_driver.c
 * Author        :
 * Date          : 2017-07-13
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#if 1
#include "frtos_utils.h"
#include "frtos_errno.h"
#include "frtos_drivers.h"
#include "frtos_tasklet.h"
#include "frtos_lock.h"
#include "frtos_sys.h"
#include "config_driver.h"
#include "power_driver.h"
#include "gpio_driver.h"
#include "stm32l0xx_hal.h"
#include "adc_driver.h"
#include "uart_driver.h"
#include "cmsis_os.h"

/**************************************************************************************
* FunctionName   : mpu_pwrkey_off()
* Description    : EC20手册中提供的pwrkey关机时序,并没有真正关闭电源开关
* EntryParameter : 空
* ReturnValue    : 空
**************************************************************************************/
static void mpu_pwrkey_off(void)
{
	struct gpio io;
	
	io.gpio = POWER_PIN_MPU_PWRKEY; 
	io.value = GPIO_PIN_SET;
	lprint("mpu_pwrkey_off\r\n");
	fdrive_write(DRIVER_GPIO, &io, sizeof(io));
	osDelay(1000);

	io.value = GPIO_PIN_RESET;
	fdrive_write(DRIVER_GPIO, &io, sizeof(io));
	osDelay(1000);
	io.value = GPIO_PIN_SET;
	fdrive_write(DRIVER_GPIO, &io, sizeof(io));
}

/**************************************************************************************
* FunctionName   : mpu_pwr_on()
* Description    : EC20手册中提供的上电时序，包括打开电源,pwrkey开机,SIM卡切换
* EntryParameter : 空
* ReturnValue    : 空
**************************************************************************************/
static void mpu_pwr_on(void)
{
	struct gpio io;

	//打开MPU电源开关
	io.gpio = POWER_PIN_MPU;
	io.value = GPIO_PIN_SET;
	lprint("mpu_pwr_on\r\n");
	fdrive_write(DRIVER_GPIO, &io, sizeof(io));

	//PWRKEY上电时序
	io.gpio = POWER_PIN_MPU_PWRKEY; 
	io.value = GPIO_PIN_SET;
	lprint("mpu_pwrkey_on\r\n");
	fdrive_write(DRIVER_GPIO, &io, sizeof(io));
	osDelay(500);

	io.value = GPIO_PIN_RESET;
	fdrive_write(DRIVER_GPIO, &io, sizeof(io));
	osDelay(500);
	io.value = GPIO_PIN_SET;
	fdrive_write(DRIVER_GPIO, &io, sizeof(io));
}

/**************************************************************************************
* FunctionName   : mpu_pwr_off()
* Description    : MPU模块下电
* EntryParameter : 空
* ReturnValue    : 空
**************************************************************************************/
static void mpu_pwr_off(void)
{
	struct gpio io;

	io.gpio = POWER_PIN_MPU;
	io.value = GPIO_PIN_SET;
	lprint("mpu_pwr_off\r\n");
	fdrive_write(DRIVER_GPIO, &io, sizeof(io));
}

/**************************************************************************************
* FunctionName   : v3_3_pwr_process()
* Description    : 控制3.3V的开关电源
* EntryParameter : 上电或者下电
* ReturnValue    : 空
**************************************************************************************/
static void v3_3_pwr_process(uint8_t status)
{
	struct gpio io;

	io.gpio = POWER_PIN_3_3V;
	if(status == POWER_DOWN) {
		io.value = GPIO_PIN_RESET;
		lprint("3.3V power off\r\n");
	}
	else {
		io.value = GPIO_PIN_SET;
		lprint("3.3V power on\r\n");
	}
	fdrive_write(DRIVER_GPIO, &io, sizeof(io));
}

/**************************************************************************************
* FunctionName   : board_power_init()
* Description    : 打开所有电源开关
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
static void board_power_init(void)
{
	//初始化外设的电源开关
	GPIO_COMM_INIT(POWER_PIN_3_3V, GPIO_SPEED_FREQ_LOW, GPIO_MODE_OUTPUT_PP);
	GPIO_COMM_INIT(POWER_PIN_MPU, GPIO_SPEED_FREQ_LOW, GPIO_MODE_OUTPUT_PP);
	GPIO_COMM_INIT(POWER_PIN_MPU_PWRKEY, GPIO_SPEED_FREQ_LOW, GPIO_MODE_OUTPUT_PP);

	GPIO_COMM_WRITE(POWER_PIN_3_3V, GPIO_PIN_SET);
	GPIO_COMM_WRITE(POWER_PIN_MPU, GPIO_PIN_SET);
	GPIO_COMM_WRITE(POWER_PIN_MPU_PWRKEY, GPIO_PIN_RESET);

	//初始化mpu准备就绪引脚
	GPIO_COMM_INIT(STATUS_PIN_MPU_READY, 0, GPIO_MODE_INPUT);
}

/**************************************************************************************
* FunctionName   : power_ioctrl()
* Description    : 电源模块对外提供的电源管理控制接口
* EntryParameter : *args,参数, len,参数长度
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t power_ioctrl(uint8_t idx, int32_t cmd, void *args, int32_t len)
{
	uint8_t val = 0;
	struct adc_samp_s samp;

    if(unlikely((NULL == args && len != 0) || \
        (NULL != args && 0 == len) || len < 0)){
        return -EINVAL;
    }

	val = *(uint8_t*)args;
    // 1.执行命令序列
	switch(cmd){
	case POWER_CMD_MPU_ON:
		mpu_pwr_on();
		break;
	case POWER_CMD_MPU_OFF:
		mpu_pwrkey_off();
		tasklet_schedule(mpu_pwr_off, NULL, MPU_PWR_DOWN_DELAY);
		break;
	case POWER_CMD_REINIT:
		board_power_init();
		break;
	case STATUS_CMD_MPU_READY:
		*(uint8_t*)args = (uint8_t)gpio_pin_get(GPIOC, STATUS_PIN_MPU_READY);
		break;
	case STATUS_CMD_VOLTAGE_GET:
		memset(&samp, 0, sizeof(samp));
		samp.scale = 1;
		samp.zero = 0;
		fdrive_read(DRIVER_ADC, &samp, sizeof(samp));
		*(uint8_t*)args = samp.result * 66 / 4096;
		break;
    default:
		return -EINVAL;
	}

	(void)idx;
	return 0;
}

/**************************************************************************************
* FunctionName   : power_read()
* Description    : 读
* EntryParameter : *args,参数, len,参数长度
* ReturnValue    : 返回读取的字节数, 返回错误码
**************************************************************************************/
static int32_t power_read(uint8_t idx, void *data, int32_t len)
{
	if(len < (int16_t)sizeof(uint32_t)) return -EIO;
	//CLOCK_SYS_GetFreq(CORE_CLOCK, (uint32_t *)data);
	*(uint32_t*)data = SystemCoreClock;

	(void)idx;
	return sizeof(uint32_t);
}

/**************************************************************************************
* FunctionName   : power_init()
* Description    : 设备初始化
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
static int32_t __init power_init(void)
{
	board_power_init();

	return 0;
}

static __const struct driver power = {
    .idx  = DRIVER_POWER,
    .init = power_init,
    //.read = power_read,
    .ioctl = power_ioctrl,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
//CORE_INIT(power);
#endif
