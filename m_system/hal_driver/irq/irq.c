/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : irq.c
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#if 0
#include "frtos_app.h"
#include "frtos_mem.h"
#include "frtos_errno.h"
#include "frtos_drivers.h"
#include "frtos_lock.h"
#include "config_driver.h"
#include "gpio_driver.h"
//#include "port_hal.h"
#include "frtos_irq.h"
//#include "interrupt_manager.h"

void EXTI0_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}

void EXTI15_10_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
}

void EXTI9_5_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_5);
}


/**************************************************************************************
* FunctionName   : irq_cvrt_trig()
* Description    : 触发模式转换
* EntryParameter : gpio,gpio组， pin,gpio的pin脚，trig,触发模式
* ReturnValue    : 返回错误码
**************************************************************************************/
int8_t irq_cvrt_trig(GPIO_TypeDef *gpio, uint16_t pin, uint8_t trig)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	HAL_GPIO_DeInit(gpio, pin);

//	memset(&GPIO_InitStruct, 0, sizeof(GPIO_InitStruct));
	GPIO_InitStruct.Pin = pin;
	GPIO_InitStruct.Pull = GPIO_NOPULL;

    switch(trig){
    case IRQ_TRIG_DISABLE:
		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
		break;
    case IRQ_TRIG_UP:
		GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
		break;
    case IRQ_TRIG_DOWN:
		GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
		break;
    case IRQ_TRIG_EDGE:
		GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
		break;
    default:
        return -EINVAL;
    }

	HAL_GPIO_Init(gpio, &GPIO_InitStruct);

    return 0;
}

/**************************************************************************************
* FunctionName   : irq_set_trigger()
* Description    : 中断触发模式设置(需要外部实现)
* EntryParameter : irq,中断号, trig,触发模式
* ReturnValue    : 返回错误码
**************************************************************************************/
int8_t irq_set_trigger(uint8_t irq, uint8_t trig)
{
    uint16_t pin, group;
    GPIO_TypeDef *gpio = NULL;

    // 1.计算GPIO组
	pin = IRQ_TO_GPIOPIN(irq);
	group = IRQ_TO_GPIOGROUP(irq);
	switch (group) {
		case 0:
			gpio = GPIOA;
			break;
		case 1:
			gpio = GPIOB;
			break;
		case 2:
			gpio = GPIOC;
			break;
		case 3:
			gpio = GPIOD;
			break;
		case 4:
			gpio = GPIOE;
			break;
		default:
			gpio = GPIOA;
	}

    // 3.转换触发模式
    if(0 != irq_cvrt_trig(gpio, pin, trig)){
        return -EINVAL;
    }

    return 0;
}

/**************************************************************************************
* FunctionName   : irq_enable()
* Description    : 使能中断
* EntryParameter : 中断号
* ReturnValue    : 返回错误码
**************************************************************************************/
int8_t irq_enable(uint8_t irq)
{
	uint16_t gpio_pin;
	gpio_pin = GPIOSEQ_TO_GPIOPIN(irq);

	switch(gpio_pin){
		case GPIO_PIN_0:
			HAL_NVIC_SetPriority(EXTI0_IRQn, 5, 0);
			HAL_NVIC_EnableIRQ(EXTI0_IRQn);
			break;
		case GPIO_PIN_1:
			HAL_NVIC_SetPriority(EXTI1_IRQn, 5, 0);
		  	HAL_NVIC_EnableIRQ(EXTI1_IRQn);
			break;
		case GPIO_PIN_2:
			HAL_NVIC_SetPriority(EXTI2_IRQn, 5, 0);
			HAL_NVIC_EnableIRQ(EXTI2_IRQn);
			break;
		case GPIO_PIN_3:
			HAL_NVIC_SetPriority(EXTI3_IRQn, 5, 0);
			HAL_NVIC_EnableIRQ(EXTI3_IRQn);
			break;
		case GPIO_PIN_4:
			HAL_NVIC_SetPriority(EXTI4_IRQn, 5, 0);
			HAL_NVIC_EnableIRQ(EXTI4_IRQn);
			break;
		default:
			if (gpio_pin >= GPIO_PIN_5 && gpio_pin <= GPIO_PIN_9) {
				HAL_NVIC_SetPriority(EXTI9_5_IRQn, 5, 0);
				HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
			}
			else if (gpio_pin >= GPIO_PIN_10 && gpio_pin <= GPIO_PIN_15) {
				HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
				HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
			}
			break;
	}
	return 0;
}
/**************************************************************************************
* FunctionName   : irq_ioctrl()
* Description    : irq应用控制
* EntryParameter : *args,参数, len,参数长度
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t irq_ioctrl(uint8_t idx, int32_t cmd, void *args, int32_t len)
{
    struct irq_reg_s *irq;

    if(unlikely((NULL == args && len > 0) || \
        (NULL != args && len != sizeof(struct irq_reg_s)) || len < 0)){
        return -EINVAL;
    }
    irq = (struct irq_reg_s *)args;

    (void)idx;
    (void)cmd;
    return request_irq(irq->irq, irq->trig, irq->handler);
}

/*************************************************************************************
* FunctionName   : HAL_GPIO_EXTICallback()
* Description    : 外部中断句柄
* EntryParameter : pin 产生中断的引脚
* ReturnValue    : None
*************************************************************************************/
extern int8_t g_cmd_sensor_key;
void HAL_GPIO_EXTI_Callback(uint16_t pin)
{
    uint16_t irq = 99;

    //1、pin脚转为中断号
	switch (pin) {
		case GPIO_PIN_0://PA0 12V上电中断
			irq = IRQ(A,0);
			break;
		case GPIO_PIN_5://PC5 6轴中断
			irq = IRQ(C,5);
			break;
		case GPIO_PIN_13://PC13 光敏电阻中断
			irq = IRQ(C,13);
			break;
		default:
			return;
	}
	irq_fast_dispatch_by_pin(irq);
}

/*************************************************************************************
* FunctionName   : irq_init()
* Description    : 设备初始化
* EntryParameter : None
* ReturnValue    : 返回错误码
*************************************************************************************/
static int32_t __init irq_init(void)
{
	/* 开启外部中断 */
	HAL_NVIC_SetPriority(EXTI0_IRQn, 5, 0);
  	HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  	HAL_NVIC_SetPriority(EXTI1_IRQn, 5, 0);
  	HAL_NVIC_EnableIRQ(EXTI1_IRQn);

  	HAL_NVIC_SetPriority(EXTI2_IRQn, 5, 0);
  	HAL_NVIC_EnableIRQ(EXTI2_IRQn);

  	HAL_NVIC_SetPriority(EXTI3_IRQn, 5, 0);
  	HAL_NVIC_EnableIRQ(EXTI3_IRQn);

  	HAL_NVIC_SetPriority(EXTI4_IRQn, 5, 0);
  	HAL_NVIC_EnableIRQ(EXTI4_IRQn);

  	HAL_NVIC_SetPriority(EXTI9_5_IRQn, 5, 0);
  	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  	HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
  	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

    return 0;
}

/*************************************************************************************
* Description    : 模块初始化
*************************************************************************************/
static __const struct driver irq = {
    .idx = DRIVER_GPIOINT,
//  .init  = irq_init,
    .ioctl = irq_ioctrl,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
EARLY_INIT(irq);
#endif
