/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.  
 * File Name     : gpio_driver.h
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#if 1
#ifndef __GPIO_DRIVER_H__
#define __GPIO_DRIVER_H__

#include "frtos_types.h"
#include "frtos_ioctl.h"
#include "frtos_irq.h"
#include "frtos_gpio.h"
#include "stm32l0xx_hal.h"

/**************************************************************************************
* Description    : GPIO端口数据类型(PTA,PTB,PTC,...PTE)
**************************************************************************************/
typedef GPIO_TypeDef * gpio_port;

/**************************************************************************************
* Description    : GPIO控制参数结构
**************************************************************************************/
// gpio 输入输出配置
#define GPIO_DIRECT_INPUT			GPIO_MODE_INPUT		// GPIO输入
#define GPIO_DIRECT_OUTPUT_PP		GPIO_MODE_OUTPUT_PP	// GPIO推挽输出
#define GPIO_DIRECT_OUTPUT_OD		GPIO_MODE_OUTPUT_OD	// GPIO开漏输出

// gpio 速度配置， 具体速率参考相关芯片数据手册
#define GPIO_LOW_SPEED				GPIO_SPEED_FREQ_LOW		// GPIO低速，范围0MHz-2MHz
#define GPIO_MEDIUM_SPEED			GPIO_SPEED_FREQ_MEDIUM	// GPIO中速，范围4MHz-10MHz
#define GPIO_HIGH_SPEED				GPIO_SPEED_FREQ_HIGH	// GPIO高速，范围10MHz-50Hz 
// gpio 方向配置
struct gpio_direct_s{
    uint32_t pin;					// GPIO端口引脚编号
	uint32_t speed;					// GPIO输出速度,配置为输出时配置
    uint8_t direct;                 // GPIO端口方向,指明该gpio时输出还是输入
}__attribute__((packed));

// 定义 gpio_args_s 结构位 struct gpio
#define gpio_args_s  gpio

/**************************************************************************************
* Description    : GPIO外用函数声明
**************************************************************************************/
GPIO_TypeDef*  gpio_group_get(uint8_t gpio);
void gpio_comm_init(GPIO_TypeDef *gpio, uint16_t gpio_pin, uint8_t speed, uint8_t mode);

#if 1
/**************************************************************************************
* FunctionName   : gpio_pin_set()
* Description    : 设置GPIO端口
* EntryParameter : port,GPIO端口, pin,端口引脚编号, val,值(0,1)
* ReturnValue    : None
**************************************************************************************/
static inline void gpio_pin_set(GPIO_TypeDef *port, uint16_t pin, uint8_t val)
{
    HAL_GPIO_WritePin(port, pin, val);
}

/**************************************************************************************
* FunctionName   : gpio_pin_get()
* Description    : 设置GPIO端口
* EntryParameter : port,GPIO端口, pin,端口引脚编号
* ReturnValue    : 返回端口值(0,1)
**************************************************************************************/
static inline uint8_t gpio_pin_get(GPIO_TypeDef *port, uint16_t pin)
{
    return HAL_GPIO_ReadPin(port, pin);
}

/**************************************************************************************
* FunctionName   : gpio_comm_init
* Description    : 通过gpio的组号和pin脚号转换为gpio序号
* EntryParameter : a,gpio的组号(A-E), b,gpio的引脚号(0-15)
* ReturnValue    : gpio的序号(0-79)
**************************************************************************************/
#define GPIO_COMM_INIT(gpio_seq, sp, direc) do{ \
	struct gpio_direct_s direct; \
	memset(&direct, 0, sizeof(direct)); \
	direct.speed = sp; \
	direct.direct = direc; \
	direct.pin = gpio_seq; \
	fdrive_ioctl(DRIVER_GPIO, _IOC_GPIO_DIRECTION, (void*)&direct, sizeof(direct)); \
}while(0)
			
//gpio_comm_init(gpio_group_get(gpio_seq), GPIOSEQ_TO_GPIOPIN(gpio_seq), \
			//		speed, direc)

#define GPIO_COMM_WRITE(gpio_seq, val) do { \
	struct gpio gpio; \
	memset(&gpio, 0, sizeof(gpio)); \
	gpio.gpio = gpio_seq; \
	gpio.value = val; \
	fdrive_write(DRIVER_GPIO, &gpio, sizeof(gpio)); \
}while(0)

#define GPIO_COMM_READ(gpio_seq, val) do { \
	struct gpio gpio; \
	memset(&gpio, 0, sizeof(gpio)); \
	gpio.gpio = gpio_seq; \
	gpio.value = val; \
	fdrive_read(DRIVER_GPIO, &gpio, sizeof(gpio)); \
}while(0)
		

#endif
#endif
#endif
