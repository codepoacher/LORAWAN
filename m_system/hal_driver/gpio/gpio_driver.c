/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : gpio_driver.c
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#if 1
#include "frtos_sys.h"
#include "frtos_errno.h"
#include "frtos_drivers.h"
#include "frtos_lock.h"
#include "config_driver.h"
#include "frtos_gpio.h"
#include "gpio_driver.h"
#include "uart_driver.h"

/**************************************************************************************
* FunctionName   : gpio_group_get()
* Description    : 根据gpio引脚序号获取gpio组
* EntryParameter : gpio,gpio序号
* ReturnValue    : 返回读取的字节数, 返回错误码
**************************************************************************************/
GPIO_TypeDef*  gpio_group_get(uint8_t gpio)
{
	GPIO_TypeDef *port = NULL;
	uint16_t group = GPIOSEQ_TO_GPIOGROUP(gpio);

	switch (group) {
		case 0:
			port = GPIOA;
			break;
		case 1:
			port = GPIOB;
			break;
		case 2:
			port = GPIOC;
			break;
		case 3:
			port = GPIOD;
			break;
		case 4:
			port = GPIOE;
			break;
		default:
			return NULL;
			break;
	}

	return port;
}

void gpio_comm_init(GPIO_TypeDef *gpio, uint16_t gpio_pin, uint8_t speed, uint8_t mode)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	memset(&GPIO_InitStruct, 0, sizeof(GPIO_InitStruct));
	GPIO_InitStruct.Pin = gpio_pin;
	GPIO_InitStruct.Mode = mode;
	if (mode != GPIO_MODE_INPUT)
		GPIO_InitStruct.Speed = speed;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(gpio, &GPIO_InitStruct);
}

/**************************************************************************************
* FunctionName   : gpio_read()
* Description    : 读
* EntryParameter : *args,参数, len,参数长度
* ReturnValue    : 返回读取的字节数, 返回错误码
**************************************************************************************/
static int32_t gpio_read(uint8_t idx, void *data, int32_t len)
{
    uint16_t pin;
    GPIO_TypeDef *port = NULL;
    struct gpio *gpio = (struct gpio *)data;

    (void)idx;
    if(unlikely(len < (int32_t)sizeof(struct gpio) || NULL == data)) return -EINVAL;

    // 1.计算GPIO组
	pin = GPIOSEQ_TO_GPIOPIN(gpio->gpio);
	port = gpio_group_get(gpio->gpio);
	gpio->value = gpio_pin_get(port, pin);
 
    return gpio->value;
}

/**************************************************************************************
* FunctionName   : gpio_write()
* Description    : 写
* EntryParameter : *args,参数, len,参数长度
* ReturnValue    : 返回写入的字节数, 返回错误码
**************************************************************************************/
static int32_t gpio_write(uint8_t idx, void *data, int32_t len)
{
    uint16_t pin;
    GPIO_TypeDef *port = NULL;
    struct gpio *gpio = (struct gpio *)data;

    if(unlikely(len < (int32_t)sizeof(struct gpio) || NULL == data)) return -EINVAL;

	pin = GPIOSEQ_TO_GPIOPIN(gpio->gpio);
	port = gpio_group_get(gpio->gpio);
	gpio_pin_set(port, pin, gpio->value);
	GPIO_PIN_6;

    (void)idx;
    return len;
}

/**************************************************************************************
* FunctionName   : gpio_ioctrl()
* Description    : 控制
* EntryParameter : idx,驱动序号(此处没用), cmd,控制命令, len,参数长度
*				   args, 具体的参数, 每个命令对应的参数结构:
*						_IOC_SET_DATA 和 _IOC_GET_DATA 的结构为 struct gpio
*						_IOC_GPIO_DIRECTION 的结构为 struct gpio_direct_s
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t gpio_ioctrl(uint8_t idx, int32_t cmd, void *args, int32_t len)
{
	gpio_port  group;
	uint16_t  pin = 0;
	struct gpio_args_s *gpio = (struct gpio_args_s*)args;
    struct gpio_direct_s *gpio_args = (struct gpio_direct_s*)args;

	if (unlikely(NULL == args))
			return -EINVAL;
 
    switch(cmd){
    case _IOC_GPIO_SET:
	case _IOC_SET_DATA:
		if(len != sizeof(struct gpio_args_s))
			return -EINVAL;
		group = gpio_group_get(gpio->gpio);
		if (!group)
			return -EINVAL;
		pin = GPIOSEQ_TO_GPIOPIN(gpio->gpio);
		gpio_pin_set(group, pin, gpio->value);
        break;
    case _IOC_GPIO_GET:
	case _IOC_GET_DATA:
		if(len != sizeof(struct gpio_args_s))
			return -EINVAL;
		group = gpio_group_get(gpio->gpio);
		if (!group)
			return -EINVAL;
		pin = GPIOSEQ_TO_GPIOPIN(gpio->gpio);
        gpio->value = gpio_pin_get(group, pin);
        break;
	case _IOC_GPIO_DIRECTION:
		if (len != sizeof(struct gpio_direct_s))
			return -EINVAL;
		group = gpio_group_get(gpio_args->pin);
		if (!group) 
			return -EINVAL;
		pin = GPIOSEQ_TO_GPIOPIN(gpio_args->pin);
		HAL_GPIO_DeInit(group, pin);
		gpio_comm_init(group, pin, gpio_args->speed, gpio_args->direct);
		break;	
    }

    (void)idx;
    return 0;
}

/**************************************************************************************
* FunctionName   : gpio_clock_enable()
* Description    : 是能gpio时钟
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
static void gpio_clock_enable()
{
	return;
}

/**************************************************************************************
* FunctionName   : gpio_init()
* Description    : 设备初始化
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t __init gpio_init(void)
{
	gpio_clock_enable();

    return 0;
}

static __const struct driver gpio = {
    .idx  = DRIVER_GPIO,
    .init = gpio_init,
    .read = gpio_read,
    .write = gpio_write,
    .ioctl = gpio_ioctrl,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
EARLY_INIT(gpio);
#endif
