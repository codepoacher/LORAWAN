/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : tap_control.c
 * Author        :
 * Date          : 2018-10-18
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/

#include "frtos_errno.h"
#include "frtos_lock.h"
#include "frtos_sys.h"
#include "frtos_ioctl.h"
#include "frtos_drivers.h"
#include "frtos_delay.h"
#include "frtos_irq.h"
#include "frtos_tasklet.h"
#include "config_user.h"
#include "gpio_driver.h"
#include "uart_driver.h"
#include "tap_control.h"
#include "e2prom.h"

static struct tap_ctl_pin g_tap01_pin = {
	.d1 = TAP_01_CTRL_D1,
	.d2 = TAP_01_CTRL_D2,
	.a  = TAP_01_OUT_A,
	.b	= TAP_01_OUT_B
},
g_tap02_pin = {
	.d1	= TAP_02_CTRL_D1,
	.d2	= TAP_02_CTRL_D2,
	.a	= TAP_02_OUT_A,
	.b	= TAP_02_OUT_B
};

static int32_t tap_init(void);
static fqueue_t g_tap1_ctrl_queue;
static fqueue_t g_tap2_ctrl_queue;

uint8_t g_tap01_stats = TAP_UNCTRL, g_tap02_stats = TAP_UNCTRL;

/**************************************************************************************
* Description    : 唤醒
**************************************************************************************/
static int32_t tap_wakeup(void)
{
	/*	TAP1	
	 *  PA7		---->   CTRL_SF //暂时不处理	PC4		---->	output_A
	 *	PC5		---->	output_B				PB0     ---->   CTRL_D1
	 *  PB1     ---->   CTRL_D2					
	 */
	GPIO_COMM_INIT(TAP_01_OUT_A, GPIO_SPEED_FREQ_LOW, GPIO_MODE_OUTPUT_PP);
	GPIO_COMM_INIT(TAP_01_OUT_B, GPIO_SPEED_FREQ_LOW, GPIO_MODE_OUTPUT_PP);
	GPIO_COMM_INIT(TAP_01_CTRL_D1, GPIO_SPEED_FREQ_LOW, GPIO_MODE_OUTPUT_PP);
	GPIO_COMM_INIT(TAP_01_CTRL_D2, GPIO_SPEED_FREQ_LOW, GPIO_MODE_OUTPUT_PP);

	/*	TAP2	
	 *  PE0		---->   CTRL_SF //暂时不处理	PB9		---->	output_A
	 *	PB8		---->	output_B				PE1     ---->   CTRL_D1
	 *  PE2     ---->   CTRL_D2					
	 */
	GPIO_COMM_INIT(TAP_02_OUT_A, GPIO_SPEED_FREQ_LOW, GPIO_MODE_OUTPUT_PP);
	GPIO_COMM_INIT(TAP_02_OUT_B, GPIO_SPEED_FREQ_LOW, GPIO_MODE_OUTPUT_PP);
	GPIO_COMM_INIT(TAP_02_CTRL_D1, GPIO_SPEED_FREQ_LOW, GPIO_MODE_OUTPUT_PP);
	GPIO_COMM_INIT(TAP_02_CTRL_D2, GPIO_SPEED_FREQ_LOW, GPIO_MODE_OUTPUT_PP);

	/*电容充电引脚PE3*/
	GPIO_COMM_INIT(TAP_CHARGE_PIN, GPIO_SPEED_FREQ_LOW, GPIO_MODE_OUTPUT_PP);

	return 0;
}

/**************************************************************************************
* Description    : 充电
**************************************************************************************/
static void tap_charge(void)
{
	GPIO_COMM_WRITE(TAP_CHARGE_PIN, 1);
	frtos_delay_ms(CHARGE_TIME);
	GPIO_COMM_WRITE(TAP_CHARGE_PIN, 0);
}

/**************************************************************************************
* Description    : 工装测试
**************************************************************************************/
static void tap_gzcs(void)
{
	tap_charge();

	GPIO_COMM_WRITE(TAP_01_CTRL_D1, 0);	//D1拉低
	GPIO_COMM_WRITE(TAP_02_CTRL_D1, 0);	//D1拉低
	GPIO_COMM_WRITE(TAP_01_CTRL_D2, 1);	//D2拉高
	GPIO_COMM_WRITE(TAP_02_CTRL_D2, 1);	//D2拉高
	GPIO_COMM_WRITE(TAP_01_OUT_A, 1);	//a拉高
	GPIO_COMM_WRITE(TAP_02_OUT_A, 1);	//a拉高
	GPIO_COMM_WRITE(TAP_01_OUT_B, 0);	//b拉低
	GPIO_COMM_WRITE(TAP_02_OUT_B, 0);	//b拉低
	frtos_delay_ms(2000);
	GPIO_COMM_WRITE(TAP_01_OUT_A, 0);	//a拉低
	GPIO_COMM_WRITE(TAP_02_OUT_A, 0);	//a拉低
	GPIO_COMM_WRITE(TAP_01_CTRL_D1, 1);	//D1拉高
	GPIO_COMM_WRITE(TAP_02_CTRL_D1, 1);	//D1拉高
	GPIO_COMM_WRITE(TAP_01_CTRL_D2, 0);	//D2拉低
	GPIO_COMM_WRITE(TAP_02_CTRL_D2, 0);	//D2拉低

	GPIO_COMM_WRITE(TAP_01_CTRL_D1, 0);	//D1拉低
	GPIO_COMM_WRITE(TAP_02_CTRL_D1, 0);	//D1拉低
	GPIO_COMM_WRITE(TAP_01_CTRL_D2, 1);	//D2拉高
	GPIO_COMM_WRITE(TAP_02_CTRL_D2, 1);	//D2拉高
	GPIO_COMM_WRITE(TAP_01_OUT_A, 0);	//a拉低
	GPIO_COMM_WRITE(TAP_02_OUT_A, 0);	//a拉低
	GPIO_COMM_WRITE(TAP_01_OUT_B, 1);	//b拉高
	GPIO_COMM_WRITE(TAP_02_OUT_B, 1);	//b拉高
	frtos_delay_ms(2000);
	GPIO_COMM_WRITE(TAP_01_OUT_B, 0);	//b拉低
	GPIO_COMM_WRITE(TAP_02_OUT_B, 0);	//b拉低
	GPIO_COMM_WRITE(TAP_01_CTRL_D1, 1);	//D1拉高
	GPIO_COMM_WRITE(TAP_02_CTRL_D1, 1);	//D1拉高
	GPIO_COMM_WRITE(TAP_01_CTRL_D2, 0);	//D2拉低
	GPIO_COMM_WRITE(TAP_02_CTRL_D2, 0);	//D2拉低
}

/**************************************************************************************
* Description    : 开阀门时序
**************************************************************************************/
static void open_tap_action(struct tap_ctl_pin *pin)
{
	GPIO_COMM_WRITE(pin->d1, 0);	//D1拉低
	GPIO_COMM_WRITE(pin->d2, 1);	//D2拉高
	GPIO_COMM_WRITE(pin->a, 1);		//a拉高
	GPIO_COMM_WRITE(pin->b, 0);		//b拉低
	frtos_delay_ms(50);
	GPIO_COMM_WRITE(pin->a, 0);		//a拉低
	GPIO_COMM_WRITE(pin->d1, 1);	//D1拉高
	GPIO_COMM_WRITE(pin->d2, 0);	//D2拉低
}

/**************************************************************************************
* Description    : 关阀门时序
**************************************************************************************/
static void close_tap_action(struct tap_ctl_pin *pin)
{
	GPIO_COMM_WRITE(pin->d1, 0);	//D1拉低
	GPIO_COMM_WRITE(pin->d2, 1);	//D2拉高
	GPIO_COMM_WRITE(pin->a, 0);		//a拉低
	GPIO_COMM_WRITE(pin->b, 1);		//b拉高
	frtos_delay_ms(50);
	GPIO_COMM_WRITE(pin->b, 0);		//b拉低
	GPIO_COMM_WRITE(pin->d1, 1);	//D1拉高
	GPIO_COMM_WRITE(pin->d2, 0);	//D2拉低
}

/**************************************************************************************
* Description    : 关阀门
**************************************************************************************/
static int32_t close_tap(uint8_t tap)
{
	struct e2prom_s edata;
	tap_charge();
	lprint("\r\nclose tap%02d\r\n", tap);
	switch(tap) {
		case TAP_01:
			close_tap_action(&g_tap01_pin);
			g_tap01_stats = TAP_CLOSE;
			edata.addr = E2PROM_TAP01_STAT_ADDR;
			edata.data = &g_tap01_stats;
			break;
		case TAP_02:
			close_tap_action(&g_tap02_pin);
			g_tap02_stats = TAP_CLOSE;
			edata.addr = E2PROM_TAP02_STAT_ADDR;
			edata.data = &g_tap02_stats;
			break;
		default:
			return -EINVAL;
	}
	edata.len = 1;
	fdrive_write(DRIVER_E2PROM, &edata, sizeof(struct e2prom_s));
}

/**************************************************************************************
* Description    : 开阀门
**************************************************************************************/
static int32_t open_tap(uint8_t tap)
{
	struct e2prom_s edata;
	tap_charge();
	lprint("\r\nopen tap%02d\r\n", tap);
	switch(tap) {
		case TAP_01:
			open_tap_action(&g_tap01_pin);
			g_tap01_stats = TAP_OPEN;
			edata.addr = E2PROM_TAP01_STAT_ADDR;
			edata.data = &g_tap01_stats;
			break;
		case TAP_02:
			open_tap_action(&g_tap02_pin);
			g_tap02_stats = TAP_OPEN;
			edata.addr = E2PROM_TAP02_STAT_ADDR;
			edata.data = &g_tap02_stats;
			break;
		default:
			return -EINVAL;
	}
	edata.len = 1;
	fdrive_write(DRIVER_E2PROM, &edata, sizeof(struct e2prom_s));
}

/**************************************************************************************
* TypeName       : module_write_t()
* Description    : 驱动写数据类型，驱动id号， 写入数据缓冲， 写入数据长度
* EntryParameter : None
* ReturnValue    : 返回错误码或者长度
**************************************************************************************/
static int32_t tap_write(uint8_t idx, void *arg, int32_t len)
{
	if (unlikely(arg == NULL && len != 0 ) || (arg != NULL && len == 0)) {
		return -EINVAL;
	}

	struct tap_action *tap_ctrl = (struct tap_action*)arg;
	if (TAP_OPEN == tap_ctrl->action) {
		open_tap(tap_ctrl->ch);
	}
	else if (TAP_CLOSE == tap_ctrl->action) {
		close_tap(tap_ctrl->ch);
	}

	return 0;
}
/**************************************************************************************
* TypeName       : module_ioctl_t()
* Description    : 驱动控制，驱动ID号，驱动命令字，驱动控制结构，驱动控制结构长度
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t tap_ioctl(uint8_t idx, int32_t cmd, void* arg, int32_t len)
{
	int32_t ret = 0;
	if (unlikely(arg == NULL && len != 0 ) || (arg != NULL && len == 0)) {
		return -EINVAL;
	}
	
	//1、执行命令
	switch(cmd) {
		case _IOC_TAP01_CTRL: {
				uint8_t action = *(uint8_t *)arg;
				if (TAP_OPEN == action) {
					struct tap_action *tap_ctrl = NULL;
					tap_ctrl = mem_malloc(sizeof(struct tap_action));
					tap_ctrl->ch = 1;
					tap_ctrl->action = 1;
					fqueue_push(g_tap1_ctrl_queue, &tap_ctrl, 0);
				}
				else if (TAP_CLOSE == action) {
					struct tap_action *tap_ctrl = NULL;
					tap_ctrl = mem_malloc(sizeof(struct tap_action));
					tap_ctrl->ch = 1;
					tap_ctrl->action = 0;
					fqueue_push(g_tap1_ctrl_queue, &tap_ctrl, 0);
				}
				break;
			}
		case _IOC_TAP02_CTRL: {
				uint8_t action = *(uint8_t *)arg;
				if (TAP_OPEN == action) {
					struct tap_action *tap_ctrl = NULL;
					tap_ctrl = mem_malloc(sizeof(struct tap_action));
					tap_ctrl->ch = 2;
					tap_ctrl->action = 1;
					fqueue_push(g_tap2_ctrl_queue, &tap_ctrl, 0);
				}
				else if (TAP_CLOSE == action) {
					struct tap_action *tap_ctrl = NULL;
					tap_ctrl = mem_malloc(sizeof(struct tap_action));
					tap_ctrl->ch = 2;
					tap_ctrl->action = 0;
					fqueue_push(g_tap2_ctrl_queue, &tap_ctrl, 0);
				}
				break;
			}
		case _IOC_TAP1_HANDLE: {
				struct tap_action *tap_ctrl = NULL;
				ret = fqueue_pop(g_tap1_ctrl_queue, &tap_ctrl, 0, true);
				if (0 == ret) {
					fdrive_write(DRIVER_TAP, tap_ctrl, sizeof(struct tap_action*));
					mem_free(tap_ctrl);
				}
				break;
			}
		case _IOC_TAP2_HANDLE: {
				struct tap_action *tap_ctrl = NULL;
				ret = fqueue_pop(g_tap2_ctrl_queue, &tap_ctrl, 0, true);
				if (0 == ret) {
					fdrive_write(DRIVER_TAP, tap_ctrl, sizeof(struct tap_action*));
					mem_free(tap_ctrl);
				}
				break;
			}
		case _IOC_GET_TAP01:
			*(uint8_t*)arg = g_tap01_stats;
			break;
		case _IOC_GET_TAP02:
			*(uint8_t*)arg = g_tap02_stats;
			break;
		case _IOC_GET_TAP_ALL:
			*(uint8_t*)arg = (g_tap02_stats << 1) | g_tap01_stats;
			break;
		case _IOC_TAP_GZCS:
			tap_gzcs();
			break;
		default:
			return -EINVAL;
	}

	return ret;
}

/**************************************************************************************
* Description    : 初始化
**************************************************************************************/
static int32_t tap_init(void)
{
	/*	TAP1	
	 *  PA7		---->   CTRL_SF //暂时不处理	PC4		---->	output_A
	 *	PC5		---->	output_B				PB0     ---->   CTRL_D1
	 *  PB1     ---->   CTRL_D2					
	 */
	GPIO_COMM_INIT(TAP_01_OUT_A, GPIO_SPEED_FREQ_LOW, GPIO_MODE_OUTPUT_PP);
	GPIO_COMM_INIT(TAP_01_OUT_B, GPIO_SPEED_FREQ_LOW, GPIO_MODE_OUTPUT_PP);
	GPIO_COMM_INIT(TAP_01_CTRL_D1, GPIO_SPEED_FREQ_LOW, GPIO_MODE_OUTPUT_PP);
	GPIO_COMM_INIT(TAP_01_CTRL_D2, GPIO_SPEED_FREQ_LOW, GPIO_MODE_OUTPUT_PP);

	/*	TAP2	
	 *  PE0		---->   CTRL_SF //暂时不处理	PB9		---->	output_A
	 *	PB8		---->	output_B				PE1     ---->   CTRL_D1
	 *  PE2     ---->   CTRL_D2					
	 */
	GPIO_COMM_INIT(TAP_02_OUT_A, GPIO_SPEED_FREQ_LOW, GPIO_MODE_OUTPUT_PP);
	GPIO_COMM_INIT(TAP_02_OUT_B, GPIO_SPEED_FREQ_LOW, GPIO_MODE_OUTPUT_PP);
	GPIO_COMM_INIT(TAP_02_CTRL_D1, GPIO_SPEED_FREQ_LOW, GPIO_MODE_OUTPUT_PP);
	GPIO_COMM_INIT(TAP_02_CTRL_D2, GPIO_SPEED_FREQ_LOW, GPIO_MODE_OUTPUT_PP);

	/*电容充电引脚PE3*/
	GPIO_COMM_INIT(TAP_CHARGE_PIN, GPIO_SPEED_FREQ_LOW, GPIO_MODE_OUTPUT_PP);

	g_tap1_ctrl_queue = fqueue_create(1, sizeof(struct tap_action*));
	g_tap2_ctrl_queue = fqueue_create(1, sizeof(struct tap_action*));

	fdrive_ioctl(DRIVER_E2PROM, _IOC_E2PROM_TAP01_READ, &g_tap01_stats, sizeof(g_tap01_stats));
	fdrive_ioctl(DRIVER_E2PROM, _IOC_E2PROM_TAP02_READ, &g_tap02_stats, sizeof(g_tap02_stats));
	return 0;
}

/**************************************************************************************
* Description    : 定义通信任务结构
**************************************************************************************/
static __const struct driver tap = {
    .idx	= DRIVER_TAP,
    .init	= tap_init,
    .ioctl	= tap_ioctl,
	.wakeup	= tap_wakeup,
	.write  = tap_write,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
LATE_INIT(tap);

