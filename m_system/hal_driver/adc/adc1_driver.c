/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : adc_driver.c
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#include "frtos_errno.h"
#include "frtos_drivers.h"
#include "frtos_lock.h"
#include "frtos_delay.h"
#include "config_driver.h"
#include "adc_driver.h"
#include "gpio_driver.h"

#if 0
/**************************************************************************************
* Description    : 模块内部数据定义
**************************************************************************************/
static mutex_lock_t adc_mutex = NULL;        // 互斥访问锁
static ADC_HandleTypeDef hadc;

/**************************************************************************************
* FunctionName   : HAL_ADC_MspInit()
* Description    : 设备硬件初始化
* EntryParameter : adc句柄
* ReturnValue    : None.
**************************************************************************************/
void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	if(adcHandle->Instance==ADC1)
	{
		__HAL_RCC_ADC1_CLK_ENABLE();

		/**ADC GPIO Configuration    
		 *     PC2     ------> ADC_IN12
		 *     PC3     ------> ADC_IN13
		 *     PA2     ------> ADC_IN2
		 *     PA3     ------> ADC_IN3 
		 */
		GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;
		GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

		GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;
		GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	}
}

/**************************************************************************************
* FunctionName   : HAL_ADC_MspDeInit()
* Description    : 设备硬件反初始化
* EntryParameter : adc句柄
* ReturnValue    : None.
**************************************************************************************/
void HAL_ADC_MspDeInit(ADC_HandleTypeDef* adcHandle)
{
	if(adcHandle->Instance==ADC1)
	{
		__HAL_RCC_ADC1_CLK_DISABLE();

		/**ADC GPIO Configuration    
		 *    PC2     ------> ADC_IN12
		 *    PC3     ------> ADC_IN13
		 *    PA2     ------> ADC_IN2
		 *    PA3     ------> ADC_IN3 
		 */
		HAL_GPIO_DeInit(GPIOC, GPIO_PIN_2|GPIO_PIN_3);

		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2|GPIO_PIN_3);
	}
} 


/**************************************************************************************
* FunctionName   : adc1_get()
* Description    : 获取adc1采样
* EntryParameter : channel,adc1对应的通道
* ReturnValue    : 返回AD值
**************************************************************************************/
static uint32_t adc1_get(uint16_t channel)
{
	uint32_t ADC_Convert_Temp = 0;
	ADC_ChannelConfTypeDef sConfig;

	hadc.Instance->CHSELR = 0;
	switch(channel){
		case PW_ADC:
			sConfig.Channel = ADC_CHANNEL_2;	//PW+_ADC
			break;
		case INPUT_VOL1:
			sConfig.Channel = ADC_CHANNEL_3;	//INPUT_VOL1
			break;
		case VBATIN_ADC:
			GPIO_COMM_WRITE(GPIO(D,9), GPIO_PIN_SET);
			frtos_delay_ms(5);
            sConfig.Channel = ADC_CHANNEL_12;	//VBATIN_ADC
			break;
		case VSUNIN_ADC:
            sConfig.Channel = ADC_CHANNEL_13;	//VSUNIN_ADC
			break;
	}
//	sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
	HAL_ADC_ConfigChannel(&hadc, &sConfig);
	HAL_ADC_Start(&hadc);
	if(HAL_ADC_PollForConversion(&hadc, 0xffff) == HAL_OK){
		ADC_Convert_Temp = HAL_ADC_GetValue(&hadc);
	}
	HAL_ADC_Stop(&hadc);

	if (3 == channel)
		GPIO_COMM_WRITE(GPIO(D,9), GPIO_PIN_RESET);

   	return ADC_Convert_Temp;
}

/**************************************************************************************
* FunctionName   : adc1_adjust_value()
* Description    : adc多次采样,去掉最高值和最低值后去居中的值。
* EntryParameter : channel,adc1通道值
* ReturnValue    : 返回AD值
**************************************************************************************/
#define N 7
static uint32_t adc1_adjust_value(uint16_t channel)
{
	uint8_t count, i, j;
	uint32_t value_buf[N], sum = 0, temp;

	for(count = 0; count < N; count++){
		value_buf[count] = adc1_get(channel);
	}
	for(j = 0; j < N - 1; j++){
		for(i = 0; i < N - j - 1; i++){
			if(value_buf[i] > value_buf[i + 1]){
				temp = value_buf[i];
				value_buf[i] = value_buf[i + 1];
				value_buf[i + 1] = temp;
			}
		}
	}
	for(count = 1; count < N - 1; count++){
		sum += value_buf[count];
	}

	//总数取平均值
	return (sum / (N - 2));
	
	//从小到大排序后，取中间的值
	//return value_buf[N / 2];
}

/**************************************************************************************
* FunctionName   : adc1_read()
* Description    : 读
* EntryParameter : *args,参数, len,参数长度
* ReturnValue    : 返回读取的字节数, 返回错误码
**************************************************************************************/
static int32_t adc1_read(uint8_t idx, void *data, int32_t len)
{
    uint32_t adc_value = 0;
    struct adc_samp_s *samp = (struct adc_samp_s *)data;

    if (unlikely(NULL == data || len != sizeof(struct adc_samp_s))) {
        return -EINVAL;
    }

    // 1.上锁
    mutex_lock(adc_mutex);

    // 2.读取AD值
    adc_value = adc1_adjust_value(samp->ch);

    // 3.解锁
    mutex_unlock(adc_mutex);

    // 4.转换
    float t = ((adc_value - samp->zero) * 3.3 / 4096.0) * samp->scale;
	samp->result = *(int32_t *)&t;
    (void)idx;
    return len;
}

/**************************************************************************************
* FunctionName   : adc1_phy_init()
* Description    : 设备初始化
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
static int8_t adc1_phy_init(void)
{
	ADC_ChannelConfTypeDef sConfig;

	hadc.Instance = ADC1;
	hadc.Init.OversamplingMode = DISABLE;
	hadc.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV1;
	hadc.Init.Resolution = ADC_RESOLUTION_12B;
	hadc.Init.SamplingTime = ADC_SAMPLETIME_160CYCLES_5;
	hadc.Init.ScanConvMode = ADC_SCAN_DIRECTION_FORWARD;
	hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc.Init.ContinuousConvMode = DISABLE;
	hadc.Init.DiscontinuousConvMode = DISABLE;
	hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	hadc.Init.DMAContinuousRequests = DISABLE;
	hadc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
	hadc.Init.Overrun = ADC_OVR_DATA_PRESERVED;
	hadc.Init.LowPowerAutoWait = DISABLE;
	hadc.Init.LowPowerFrequencyMode = DISABLE;
	hadc.Init.LowPowerAutoPowerOff = DISABLE;
	if (HAL_ADC_Init(&hadc) != HAL_OK)
	{
		return -EINVAL;
	}
	
	return 0;
}

/*************************************************************************************
* FunctionName   : adc1_wakeup()
* Description    : adc唤醒后恢复引脚
* EntryParameter : None
* ReturnValue    : 返回错误码
*************************************************************************************/
static int32_t adc1_wakeup(void)
{
	/*1、硬件初始化*/
	adc1_phy_init();

	/*2、电池采集使能引脚*/
	GPIO_COMM_INIT(GPIO(D,9), GPIO_SPEED_FREQ_LOW, GPIO_MODE_OUTPUT_PP);

	return 0;
}

/**************************************************************************************
* MacroName      : adc1_sleep
* Description    : 控制驱动进入休眠
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t adc1_sleep(void)
{
	HAL_ADC_DeInit(&hadc);

	return 0;
}

/**************************************************************************************
* TypeName       : module_ioctl_t()
* Description    : 驱动控制，驱动ID号，驱动命令字，驱动控制结构，驱动控制结构长度
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
extern float g_pressure;
static int32_t adc1_ioctl(uint8_t idx, int32_t cmd, void* arg, int32_t len)
{
	if (unlikely(arg == NULL && len != 0 ) || (arg != NULL && len == 0)) {
		return -EINVAL;
	}
	
	struct adc_samp_s adc;
	//1、执行命令
	switch(cmd) {
		case _IOC_GET_BAT_VOL:
			ADC_COMM_READ(adc,3,2,0,arg);
			break;
		case _IOC_GET_SUN_VOL:
			ADC_COMM_READ(adc,4,2,0,arg);
			break;
		case _IOC_GET_INPUT_VOL:
			ADC_COMM_READ(adc,2,1,430,arg);
			break;
		case _IOC_GET_PRESS_VOL:
			*(uint16_t*)arg = g_pressure * 100;
			break;
		default:
			return -EINVAL;
	}

	return 0;
}
/**************************************************************************************
* FunctionName   : adc_init()
* Description    : 设备初始化
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t __init adc1_init(void)
{
	int8_t ret = 0;

	/* 1.adc 硬件初始化 */
	ret = adc1_phy_init();
	if (ret != 0)
		return ret;

    adc_mutex = mutex_lock_init();
    if(NULL == adc_mutex) return -EPERM;

	/*电池采集使能引脚*/
	GPIO_COMM_INIT(GPIO(D,9), GPIO_SPEED_FREQ_LOW, GPIO_MODE_OUTPUT_PP);

    return 0;
}

static __const struct driver adc1 = {
    .idx  = DRIVER_ADC,
    .init = adc1_init,
    .read = adc1_read,
	.wakeup = adc1_wakeup,
	.suspend = adc1_sleep,
	.ioctl = adc1_ioctl,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
MODULE_INIT(adc1);
#endif
