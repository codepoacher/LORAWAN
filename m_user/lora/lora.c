
/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : lora.c
 * Author        :
 * Date          : 2018-09-20
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#include "frtos_app.h"
#include "frtos_mem.h"
#include "frtos_errno.h"
#include "frtos_lock.h"
#include "config_user.h"
#include "config_driver.h"
#include "frtos_ioctl.h"
#include "gpio_driver.h"
#include "frtos_sys.h"
#include "frtos_utils.h"
#include "data.pb-c.h"
#include "frtos_delay.h"
#include "task.h"
#include "frtos_drivers.h"
#include "stm32l0xx_hal.h"
#include "proto.h"
#include "lora_user.h"
#include "update.h"
#include "e2prom.h"

#if 0
//  全局lora配置参数
struct lora_conf g_lora_conf = {
	.channel_index = 0,
	.channel_step = 0,
	.speed_index = 1,
	.timeout = 20,
};

// 全局lora状态
struct lora_status g_lora_status = {
	.lora_ms    = LORA_MS_M1,
	.sleep_time = SLEEP_TIME,
	.value_ctl  = VALUE_CONTROL_FREE,
};

struct update_data_ori update_ori;

uint32_t g_my_oid;

uint8_t *g_send_buf = NULL ;

//休眠时喂狗标志
static bool  wdog_flag = false; 

//工装标志
extern uint8_t g_gzcs_key;

/**************************************************************************************
* FunctionName   : init_param()
* Description    : 初始化全局参数
* EntryParameter : 
* ReturnValue    : 无
**************************************************************************************/
static void init_param()
{
	//  全局lora配置参数初始化
	g_lora_conf.channel_index = 0;
	g_lora_conf.channel_step = 0;
	g_lora_conf.speed_index = 1;
	g_lora_conf.timeout = 20;

	// 全局lora状态初始化
	g_lora_status.lora_ms = LORA_MS_M1;
	g_lora_status.sleep_time = SLEEP_TIME;
}

static struct e2prom_lora_conf g_e2prom_lr_conf_bak;
static void e2prom_read()
{
    char oid[OID_LEN] = {0};
	
	// 获取自己的SN
	fdrive_ioctl(DRIVER_E2PROM, _IOC_E2PROM_OID_READ, oid, sizeof(oid));
	g_my_oid = atoi(oid);
	LRDEBUG("\r\n####oid: %d\r\n ",g_my_oid);

	fdrive_ioctl(DRIVER_E2PROM, _IOC_E2PROM_LORA_CONF_READ, &g_e2prom_lr_conf_bak, \
			sizeof(struct e2prom_lora_conf));

	if(g_e2prom_lr_conf_bak.read_flag == 1)
	{
		g_lora_conf				   = g_e2prom_lr_conf_bak.lr_conf;
		g_lora_status.last_rx_time = g_e2prom_lr_conf_bak.last_rx_time;
		g_lora_status.lora_ms	   = g_e2prom_lr_conf_bak.lora_ms;
		LRDEBUG("ms: %d, network: %d, idx: %d, timeout: %d\r\n", g_lora_status.lora_ms, \
			g_lora_conf.network, g_lora_conf.addr, g_lora_conf.timeout);
	}
}

static void e2prom_write()
{
	struct e2prom_lora_conf e2prom_lr_conf;

	e2prom_lr_conf.lr_conf	    =  g_lora_conf;					
	e2prom_lr_conf.last_rx_time =  g_lora_status.last_rx_time;
	e2prom_lr_conf.lora_ms	    =  g_lora_status.lora_ms;	

	if(g_lora_status.lora_ms != LORA_MS_M2){
		e2prom_lr_conf.read_flag    =  0;
	} else {
		e2prom_lr_conf.read_flag    =  1;
	}

	if(memcmp(&e2prom_lr_conf, &g_e2prom_lr_conf_bak, sizeof(struct e2prom_lora_conf)) != 0);
	{
		fdrive_ioctl(DRIVER_E2PROM, _IOC_E2PROM_LORA_CONF_WRITE, &e2prom_lr_conf, sizeof(struct e2prom_lora_conf));
		memcpy(&g_e2prom_lr_conf_bak, &e2prom_lr_conf, sizeof(struct e2prom_lora_conf));
	}
}

/**************************************************************************************
* FunctionName   : set_sleep_time()
* Description    : 根据阀控所在节点设置睡眠时间
* EntryParameter : delay: 延时秒数 num 节点个数, len 每个节点发送字节长度
* ReturnValue    : 无
**************************************************************************************/
static void set_sleep_time(uint8_t delay, uint8_t num, uint8_t len)
{
	
	//lora 发送40bytes所需时间, 单位ms
	uint16_t lora_send_time;   
	
	fdrive_ioctl(DRIVER_LORA, _IOC_LORA_GET_TIME, &lora_send_time, len );
	
	// (实际发送时常补齐整秒 + 发送间隔) * 节点个数
	// 加1s提高可靠性
	// delay 主要是给开阀门留出充足的时间 
	g_lora_status.sleep_time = delay + 1 + (lora_send_time/1000 + 1 + SEND_INTERVAL) * num;

}

static int32_t proto_reg_process(uint8_t *pbuf)
{
	uint8_t i;
	uint8_t addr;
	uint32_t sn;
	int32_t rc;
	struct proto_lora *plora;
	uint8_t node_count;

	node_count =*(pbuf++);
	g_lora_status.version = *(pbuf++);

	LRDEBUG("Lora: recv REG, myoid %d\r\n", g_my_oid);

	for (i = 0; i < node_count; i++) {
		memcpy(&sn, pbuf + (i<<2), 4);
		LRDEBUG("\tSN: %d\r\n", sn);
		if (sn == g_my_oid) {
			addr = *(pbuf + (node_count<<2) + i);
			break;
		}
	}

	if (i == node_count)
		return -1;

	LRDEBUG("\tAddr %d\r\n", addr);
	
	//  指向全局配置段,加一个字节的协议号
	pbuf = pbuf + (node_count<<2) + node_count;
	plora = (struct proto_lora *)pbuf;

	//  获取参数，设置lora
	g_lora_conf.addr = addr;
	g_lora_conf.channel_index = plora->channel_index;
	g_lora_conf.channel_step = plora->channel_step;
	g_lora_conf.speed_index = plora->speed_index;
	g_lora_conf.timeout = plora->timeout;
	rc =  fdrive_ioctl(DRIVER_LORA, _IOC_LORA_SET_CONF, &g_lora_conf, sizeof(g_lora_conf));
	if (rc < 0)
		return -1;
	
	//  设置状态
	g_lora_status.lora_ms = LORA_MS_M2;

	return 0;
}

static int32_t proto_keepalive_process(uint8_t *pbuf, uint8_t type)
{
	uint8_t *pdata = g_send_buf + sizeof(struct proto_head);
	uint8_t len;
	uint8_t tap;
	uint16_t adc;
	uint32_t data;

	LRDEBUG("Lora: recv Keepalive, type %d\r\n", type);

	// 这里的动作都是非阻塞的
	// 取 电磁阀 状态
	fdrive_ioctl(DRIVER_TAP, _IOC_GET_TAP01, &tap, sizeof(tap));
	*pdata = tap;
	fdrive_ioctl(DRIVER_TAP, _IOC_GET_TAP02, &tap, sizeof(tap));
	*pdata = (tap<<1) | *pdata;
	pdata++;
	
	// 取 脉冲值
	fdrive_ioctl(DRIVER_TIMER1, _IOC_GET_DATA, &data, sizeof(data));
	memcpy(pdata, &data, 4);
	pdata += 4;

	// 取  ADC
	fdrive_ioctl(DRIVER_ADC, _IOC_GET_INPUT_VOL, &adc, sizeof(adc));
	memcpy(pdata, &adc, 2);
	pdata += 2;

	fdrive_ioctl(DRIVER_ADC, _IOC_GET_PRESS_VOL, &adc, sizeof(adc));
	memcpy(pdata, &adc, 2);
	pdata += 2;

	// 取 电压
	if (type == PROTO_KEEPALIVE3) {
		fdrive_ioctl(DRIVER_ADC, _IOC_GET_BAT_VOL, &adc, sizeof(adc));
		memcpy(pdata, &adc, 2);
		pdata += 2;

		fdrive_ioctl(DRIVER_ADC, _IOC_GET_SUN_VOL, &adc, sizeof(adc));
		memcpy(pdata, &adc, 2);
		pdata += 2;
	}

	// 取 485
	if (type == PROTO_KEEPALIVE3 || type == PROTO_KEEPALIVE2) {
		pdata += 4;
		pdata += 4;
	}

	len = pdata - g_send_buf;

	ENCAP_PACKET_HEAD(g_send_buf, g_lora_conf.addr, type | PROTO_UPLINK_MASK, g_lora_conf.network, len);

	g_lora_status.need_send_len = len;
	set_sleep_time(0, g_lora_status.send_index, len);

	return 0;
	
}

static void proto_network_process(uint8_t *pbuf, uint8_t len)
{
	g_lora_conf.network = *pbuf;
	pbuf++;

	if(0 == mask_test(pbuf, len-1, g_lora_conf.addr))
	{
		init_param();
		fdrive_ioctl(DRIVER_LORA, _IOC_LORA_SET_CONF, &g_lora_conf, sizeof(struct lora_conf));
	}
	else
	{
		g_lora_conf.addr = mask_count(pbuf, len-1, g_lora_conf.addr);
	}
}

static void lora_send_test(uint8_t *pbuf ,uint8_t len )
{
	//原样返回	
	memcpy(g_send_buf, pbuf, len);
	g_send_buf[PROTO_CRC_OFFSET - 1] = 0;
	g_send_buf[PROTO_CRC_OFFSET - 1] = (uint8_t)crc16(g_send_buf, len); 
	
	g_lora_status.need_send_len = len;
	set_sleep_time(0, 0, len);
}


static int32_t proto_power_process(uint8_t *pbuf)
{
	uint8_t *pdata = g_send_buf + sizeof(struct proto_head);
	uint8_t len;
}

static int32_t proto_value_process(uint8_t *pbuf)
{
	uint8_t *pdata = g_send_buf + sizeof(struct proto_head);
	uint32_t index1 = 0, index2 = 0;
	uint8_t tap1 = 0, tap2 = 0;
	uint8_t count, start;

	count = *pbuf;
	start = *(pbuf + 1);

	pbuf += 2;
	
	// 根据自己的网络地址来判断状态
	if (g_lora_conf.addr < start) {
		return 0;
	}

	index1 = mask_test(pbuf, count, g_lora_conf.addr - start);
	index2 = mask_test(pbuf + count, count, g_lora_conf.addr - start);

	LRDEBUG("tap1 valid:%d, tap2 vaild:%d\r\n",index1, index2);
	if (index1 != 0) {
		tap1 = mask_test(pbuf + count*2, count, g_lora_conf.addr - start);	
		LRDEBUG("tap1 action:%d\r\n",tap1);
		fdrive_ioctl(DRIVER_TAP, _IOC_TAP01_CTRL, &tap1, 1);
	}
	if (index2 != 0) {		
		tap2 = mask_test(pbuf + count*3, count, g_lora_conf.addr - start);
		LRDEBUG("tap2 action:%d\r\n",tap2);
		fdrive_ioctl(DRIVER_TAP, _IOC_TAP02_CTRL, &tap2, 1);
	}

	// 组织应答报文，其中阀控的状态需要在发送的时候再读
	
	g_lora_status.need_send_len = sizeof(struct proto_head) + 1;
	ENCAP_PACKET_HEAD(g_send_buf, g_lora_conf.addr, PROTO_VALVE_CONTROL|PROTO_UPLINK_MASK, g_lora_conf.network, g_lora_status.need_send_len);	
	
	//延时6s,等待阀门执行完
	g_lora_status.value_ctl  = VALUE_CONTROL_BUSY;
	set_sleep_time(VALUE_CONTROL_REPLY_DELAY, g_lora_status.send_index, g_lora_status.need_send_len);
	
}

static int32_t lora_proto_process(uint8_t *pbuf, uint8_t len) 
{
	struct proto_head *head;
	uint8_t *pdata = NULL;
	int32_t rc;
	uint8_t crc;

	head = (struct proto_head *)pbuf;

	if(head->type == PROTO_LORA_TEST)
	{
		lora_send_test((uint8_t *)head, head->len);
	}
	// 先校验长度和CRC
	// 设计上不会出现两个报文连续的情况
	// 万一出现干扰呢
	if (head->len > len) {
		LRDEBUG("Lora: recv pkt, len error pkg len %d, len %d\r\n", head->len, len);
		return -1;
	}
	
	// 校验包括整个报文
	crc = pbuf[PROTO_CRC_OFFSET - 1];
	pbuf[PROTO_CRC_OFFSET - 1] = 0x0;
	
	if (crc != (uint8_t)crc16(pbuf, len)) 
	{
		LRDEBUG("Lora: recv pkt, crc error pkg crc 0x%x, crc 0x%x\r\n", crc, (uint8_t)crc16(pbuf, len));
		return -1;
	}

	// rc4加解密功能后续再加

	LRDEBUG(" id 0x%x, type 0x%x, net 0x%x, my net 0x%x, ", head->id, head->type, head->network, g_lora_conf.network);
	LRDEBUG("my addr 0x%x, my ms 0x%x\r\n", g_lora_conf.addr,g_lora_status.lora_ms);
	
	// 检查网络号
	if (g_lora_status.lora_ms == LORA_MS_M2) {
		if (g_lora_conf.network != head->network)
			return -1;
	} else {
		// 没有组网成功的情况，只收广播
		if (head->id != PROTO_BROADCAST)
			return -1;
	}

	// 先处理组播的情况，看自己的地址是否在列表里面
	if (head->id == PROTO_BROADCAST) {
		// 广播
		//g_lora_status.node_count = pbuf[PROTO_NODECOUNT_OFFSET - 1];
		pdata = pbuf + PROTO_NETWORK_OFFSET;
		g_lora_status.send_index = g_lora_conf.addr;
	} else if ((head->id & PROTO_MULTICAST_MASK) == PROTO_MULTICAST_MASK ) {
		//  组播
		// 取出总共有多少字节来表示设备位图

		int8_t bytes = (head->id) & (~PROTO_MULTICAST_MASK);

		pdata = pbuf + sizeof(struct proto_head);
		if (mask_test(pdata, bytes, g_lora_conf.addr) == 0)
			return -1;
		
		g_lora_status.send_index = mask_count(pdata, bytes, g_lora_conf.addr);
		LRDEBUG("send_index : %d\r\n",g_lora_status.send_index);
		// 将指针指向数据
		pdata = pdata + bytes;
	} else {
		if (head->id == g_lora_conf.addr) {
			g_lora_status.send_index = 0;
			pdata = pbuf + sizeof(struct proto_head);
		} else {
			return -1;
		}
	}
	
	switch(head->type)
	{
		case PROTO_REG:
			// 如果自己在组网成功状态，收到给自己的注册报文，如何处理
			if (g_lora_status.lora_ms == LORA_MS_M2) {
				// 直接不理
			} else {
				//记录网络号
				g_lora_conf.network = head->network;	
				rc = proto_reg_process(pdata);
			}
			break;
		case PROTO_KEEPALIVE1:
		case PROTO_KEEPALIVE2:
		case PROTO_KEEPALIVE3:			
			if (g_lora_status.lora_ms == LORA_MS_M2) {
				rc = proto_keepalive_process(pdata, head->type);
			}
			break;
		case PROTO_POWER:			
			if (g_lora_status.lora_ms == LORA_MS_M2) {
				rc = proto_power_process(pdata);
			}
			break;
		case PROTO_VALVE_CONTROL:			
			if (g_lora_status.lora_ms == LORA_MS_M2) {
				rc = proto_value_process(pdata);
			}
			break;
		case PROTO_RS485_CONFIG:
			fuser_data_set(RS485_CONFIG_PID, pdata, head->len - sizeof(struct proto_head));
			break;
		case PROTO_PRESSURE_CONFIG:
			fuser_data_set(VC_CONFIG_PID, pdata, head->len - sizeof(struct proto_head));
			break;
		case PROTO_VERSION:
			break;
		case PROTO_UPDATE_START:
		case PROTO_UPDATE_DATA:
		case PROTO_UPDATE_PRAT:
		case PROTO_UPDATE_DATA_RE:
		case PROTO_UPDATE_PRAT_CRC:
		case PROTO_UPDATE_OVER:
			update_ori.head = head->type;
			update_ori.data = pdata;
			fuser_data_set(UPDATE_PID, &update_ori,head->len - sizeof(struct proto_head));
			break;
		case PROTO_CHANGE_NETWORK:
			proto_network_process(pdata,head->len - sizeof(struct proto_head));
			break;
		default:
			rc = -1;
			break;
	}

	//记录收到有效报文的时间
	fdrive_read(DRIVER_RTC, &g_lora_status.last_rx_time, sizeof(uint32_t));

	return rc;
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
* FunctionName   : is_timeout()
* Description    : 判断阀控是否该退网， 精度分钟
* EntryParameter : 无
* ReturnValue    : ture 超时；false 未超时
**************************************************************************************/
static bool is_timeout( void )
{
	struct tm_conf curr_time;
	uint8_t timer;

	fdrive_read(DRIVER_RTC, &curr_time, sizeof(uint32_t));	
	
	//超时时间计算
	if (curr_time.tm_hour !=  g_lora_status.last_rx_time.tm_hour)
	{
		timer = curr_time.tm_min + 60 - g_lora_status.last_rx_time.tm_min;
	} else {
		timer = curr_time.tm_min - g_lora_status.last_rx_time.tm_min;
	}

	if (timer > g_lora_conf.timeout)
	{
		return true;
	}
	return false;
}

/**************************************************************************************
* FunctionName   : enter_sleep()
* Description    : 进入睡眠模式，
* EntryParameter : 
* ReturnValue    : 无
**************************************************************************************/
static void enter_sleep()
{
	uint32_t time = 0;               //唤醒时间,这里必须使用uint32_t

	//阀门控制，阻塞6s,不能进入休眠
	if(g_lora_status.value_ctl  == VALUE_CONTROL_BUSY)
	{
		vTaskDelay(VALUE_CONTROL_REPLY_DELAY * 1000);
		g_lora_status.sleep_time -= VALUE_CONTROL_REPLY_DELAY;
		g_lora_status.value_ctl   = VALUE_CONTROL_FREE;
		wdog_flag = true;
		return;
	}

	//休眠时长超过喂狗时间,需要专门醒来喂狗
	if (g_lora_status.sleep_time > MAXIMUM_WDOG)
	{
		time = MAXIMUM_WDOG;	
		g_lora_status.sleep_time -= MAXIMUM_WDOG;
		wdog_flag = true;
	}
	else if(g_lora_status.sleep_time <= 2) //休眠时间小于2s，是否需要休眠？
	{
		vTaskDelay(g_lora_status.sleep_time * 1000);
		wdog_flag = false;
		return;
	}
	else
	{
		time = g_lora_status.sleep_time;
		wdog_flag = false;
	}
	fdrive_ioctl(DRIVER_RTC, _IOC_RTC_SLEEP, &time, sizeof(time));
}

/**************************************************************************************
* FunctionName   : lora_user_set()
* Description    : 
* EntryParameter : 
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t lora_user_set(uint8_t idx, void *data, int32_t len)
{
	struct proto_head *head = (struct proto_head *)g_send_buf;
	struct lora_data *ldata = (struct lora_data *)data;

	if(data == NULL)
		return -1;

	memcpy(head->data, ldata->data, len - sizeof(struct lora_data));

	g_lora_status.need_send_len = sizeof(struct proto_head) + len - \
								  sizeof(struct lora_data);

	ENCAP_PACKET_HEAD(g_send_buf, g_lora_conf.addr, ldata->type | \
			PROTO_UPLINK_MASK, g_lora_conf.network, g_lora_status.need_send_len);	
	
	set_sleep_time(0, g_lora_status.send_index, g_lora_status.need_send_len);
	
	return 0;
}

#endif
/**************************************************************************************
* FunctionName   : lora_run()
* Description    : adc周期任务
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t lora_run(void)
{
//	rtc_time_print();
	vTaskDelay(5000);
	return 0;
}

static int32_t lora_user_init(void)
{
//	e2prom_read(); //有没有必要记录参数
	
	//初始化Lora频率，速率
struct lora_conf g_lora_conf = {
	.channel_index = 0,
	.channel_step = 0,
	.speed_index = 1,
	.timeout = 20,
};
	fdrive_ioctl(DRIVER_LORA, _IOC_LORA_SET_CONF, &g_lora_conf, sizeof(struct lora_conf));
	// 初始化变量
//	g_send_buf = mem_malloc(MAX_DATA_LEN);
//	if(g_send_buf == NULL)
//	{
//		LRDEBUG("malloc error\r\n");
//		return -1;
//	}
	return 0;
}

/**************************************************************************************
* Description    : 定义通信任务结构
**************************************************************************************/
static __const struct applite lora = {
    .idx   = LORA_PID,
    .name  = "lora",
    .run   = lora_run,
    .init  = lora_user_init,
//	.set   = lora_user_set,
};

/**************************************************************************************
* Description    : lora模块初始化
**************************************************************************************/
//APP_REGISTER(lora);

