#include "uart_driver.h"
#include "config_driver.h"
#include "frtos_app.h"
#include "frtos_mem.h"
#include "frtos_errno.h"
#include "frtos_lock.h"
#include "config_user.h"
#include "config_driver.h"
#include "frtos_ioctl.h"
#include "frtos_libpack.h"
#include "frtos_list.h"
#include "frtos_sys.h"
#include "frtos_delay.h"
#include "loopbuf.h"
#include "adc_driver.h"
#include "rtc_driver.h"
#include "gpio_driver.h"
#include "tap_control.h"
#include "io_ctrl.h"
#include "e2prom.h"
#include "lora_user.h"

/**************************************************************************************
* Description    : 定义命令行所需的全局变量
**************************************************************************************/
static uint8_t g_cmd_buf[128] = { 0 };			//命令缓冲区
static uint8_t g_buf_cnt = 0;					//缓冲区当前个数
#define TIP  "\r\n#"							//命令行提示符
//int8_t g_cmd_sensor_key = 0;
//int8_t g_cmd_gps_key = 0;
//
//uint8_t g_fakong[7] = "FAKONG";
//uint8_t g_success[8] = "success";
//uint8_t g_fail[5] = "fail";
//uint8_t g_time[13] = "1-2018-11-12";
//
//extern fqueue_t g_tap_ctrl_queue;
//extern uint8_t g_485_gzcs;
//extern struct lora_conf g_lora_conf;
//uint8_t g_gzcs_key = 0;
/**************************************************************************************
* FunctionName   : cmdline_dispatcher()
* Description    : 处理命令
* EntryParameter : data，接收的数据
* ReturnValue    : None.
**************************************************************************************/
char g_lr_buf[5] = {1, 2, 3, 3, 4};
static void cmdline_dispatcher(uint8_t *data)
{
	if (strncmp((const char *)data, "freq:", strlen("freq:")) == 0) {
		debug("\r\nFrequency(%dHz)\r\n "    , SX1276LoRaGetRFFrequency());
	}else if(strncmp((const char *)data, "send", strlen("send")) == 0 )
	{
		int AppPort = atoi(data + strlen("send") + 1); 
		debug("\r\nport %d\r\n",AppPort);
        lorawan_module_send(1, AppPort, g_lr_buf, sizeof(g_lr_buf), NULL);
	}
	/*
	if (strncmp((const char *)data, "sleep", strlen("sleep")) == 0) {
		uint32_t time = atoi(data + 6);
		debug("\r\nsleep %d second, start at:", time);
		//rtc_time_print();
		fdrive_ioctl(DRIVER_RTC, _IOC_RTC_SLEEP, &time, sizeof(&time));
		debug("wake up at:");
		//rtc_time_print();
	}
	else if (strncmp((const char *)data, "tap", strlen("tap")) == 0) {
		uint8_t tap = atoi(data + 3), ch;
		uint8_t action = *(data + 5), t, stat;
		if (strncmp((const char *)data + 5, "gzcs", strlen("gzcs")) == 0) {
			debug("%s:%s:tap:man::\r\n", g_fakong, g_time);
			fdrive_ioctl(DRIVER_TAP, _IOC_TAP_GZCS, NULL, 0);
		}
		else {
			if ('s' == action) {
				if (tap == 1) {
					fdrive_ioctl(DRIVER_TAP, _IOC_GET_TAP01, &stat, 1);
					debug("\r\ntap01 stat:%d \r\n", stat);
				}
				else if (tap == 2) {
					fdrive_ioctl(DRIVER_TAP, _IOC_GET_TAP02, &stat, 1);
					debug("\r\ntap02 stat:%d \r\n", stat);
				}
			}
			else {
				if (action == 'o')
					t = TAP_OPEN;
				if (action == 'c')
					t = TAP_CLOSE;
				if (tap == 1)
					fdrive_ioctl(DRIVER_TAP, _IOC_TAP01_CTRL, &t, 1);
				if (tap == 2)
					fdrive_ioctl(DRIVER_TAP, _IOC_TAP02_CTRL, &t, 1);
			}
		}
	}
	else if (strncmp((const char *)data, "485", strlen("485")) == 0) {
		if (strncmp((const char *)data + 5, "gzcs", strlen("gzcs")) == 0) {
			fdrive_write(DRIVER_UART5, "485 test", strlen("485 test"));
			g_485_gzcs = 1;
		}
	}
	else if (strncmp((const char*)data, "sysclock", strlen("sysclock")) == 0) {
		debug("\r\nclock:%dHz\r\n", SystemCoreClock);
	}
	else if (strncmp((const char *)data, "adc", strlen("adc")) == 0) {
		uint16_t adc;
		uint8_t ch;
		ch = atoi(data + 3);
		if (ch == 1) {
			fdrive_ioctl(DRIVER_ADC, _IOC_GET_INPUT_VOL, &adc, 2);
		}
		if (ch == 2) {
			fdrive_ioctl(DRIVER_ADC, _IOC_GET_BAT_VOL, &adc, 2);
		}
		if (ch == 3) {
			fdrive_ioctl(DRIVER_ADC, _IOC_GET_SUN_VOL, &adc, 2);
		}
		if (strncmp((const char *)data + 6, "gzcs", strlen("gzcs")) == 0) {
			if (1 == ch) {
				if (adc >= 270)
					debug("%s:%s:adc1:%s::\r\n", g_fakong, g_time, g_success);
				else
					debug("%s:%s:adc1:%s::\r\n", g_fakong, g_time, g_fail);
			}
			else if (2 == ch) {
				if (adc >= 280)
					debug("%s:%s:adc2:%s::\r\n", g_fakong, g_time, g_success);
				else
					debug("%s:%s:adc2:%s::\r\n", g_fakong, g_time, g_fail);
			}
		}
		else {
			debug("\r\nadc%d:%d\r\n", ch, adc);
		}
	}
	else if (strncmp((const char *)data, "pulse", strlen("pulse")) == 0) {
		uint32_t fre = 0;
		fdrive_read(DRIVER_TIMER1, &fre, sizeof(uint32_t));
		if (strncmp((const char *)data + 7, "gzcs", strlen("gzcs")) == 0) {
			if (fre > 0) {
				debug("%s:%s:pulse:%s::\r\n", g_fakong, g_time, g_success);
			}
			else {
				debug("%s:%s:pulse:%s::\r\n", g_fakong, g_time, g_fail);
			}
		}
		else {
			debug("\r\nfrequency:%d\r\n", fre);
		}
	}
	else if (strncmp((const char *)data, "onoff", strlen("onoff")) == 0) {
		struct gpio io;
		uint8_t t;	
		io.gpio = INPUT_ONOFF_1; 
		fdrive_read(DRIVER_GPIO, (void *)&io, sizeof(struct gpio));
		t = io.value;
		io.gpio = INPUT_ONOFF_2;
		fdrive_read(DRIVER_GPIO, (void *)&io, sizeof(struct gpio));
		if (0 == io.value && t == 0) {
			debug("%s:%s:onoff:%s::\r\n", g_fakong, g_time, g_success);
		}
		else {
			debug("%s:%s:onoff:%s::\r\n", g_fakong, g_time, g_fail);
		}
	}
	else if (strncmp((const char *)data, "out", strlen("out")) == 0) {
		uint32_t pin = atoi(data + 3);
		uint32_t value = atoi(data + 5);

		if (strncmp((const char *)data + 5, "gzcs", strlen("gzcs")) == 0) {
			debug("%s:%s:out:man::\r\n", g_fakong, g_time);
			GPIO_COMM_WRITE(OUTPUT_3_3V_EN_1, 1);
			GPIO_COMM_WRITE(OUTPUT_3_3V_EN_2, 1);
			GPIO_COMM_WRITE(PWR_12V_485_EN, 1);
			frtos_delay_ms(3000);
			GPIO_COMM_WRITE(OUTPUT_3_3V_EN_1, 0);
			GPIO_COMM_WRITE(OUTPUT_3_3V_EN_2, 0);
			GPIO_COMM_WRITE(PWR_12V_485_EN, 0);
		}
		else {
			if (1 == pin) {
				if (value)
					GPIO_COMM_WRITE(OUTPUT_3_3V_EN_1, 1);
				else
					GPIO_COMM_WRITE(OUTPUT_3_3V_EN_1, 0);
			}
			else if (2 == pin) {
				if (value)
					GPIO_COMM_WRITE(OUTPUT_3_3V_EN_2, 1);
				else
					GPIO_COMM_WRITE(OUTPUT_3_3V_EN_2, 0);
			}
			else if (3 == pin) {
				if (value)
					GPIO_COMM_WRITE(PWR_12V_485_EN, 1);
				else
					GPIO_COMM_WRITE(PWR_12V_485_EN, 0);
			}
			else {
				debug("\r\nout no ch!\r\n");
			}
		}
	}
	else if (strncmp((const char *)data, "e2prom", strlen("e2prom")) == 0) {
		if (strncmp((const char *)data + 7, "default", strlen("default")) == 0) {
			struct e2prom_press_dev press_conf;
			uint8_t press_conf_len = sizeof(struct e2prom_press_dev);

			fdrive_ioctl(DRIVER_E2PROM, _IOC_E2PROM_PRESS_READ, &press_conf, press_conf_len);

			debug("\r\nkey:%x\r\n", press_conf.conf_key);
			debug("dev_mount:%x\r\n", press_conf.dev_mount);
			debug("hi_vol:%x\r\n", press_conf.high_vol);
			debug("low_vol:%x\r\n", press_conf.low_vol);
			debug("range:%x\r\n", press_conf.press_range);
			debug("pulse:%x\r\n", press_conf.pulse_scale);
		}
		if (strncmp((const char *)data + 7, "oid", strlen("oid")) == 0) {
			uint8_t oid_read[OID_LEN + 1] = {0};
			fdrive_ioctl(DRIVER_E2PROM, _IOC_E2PROM_OID_READ, oid_read, OID_LEN);
			debug("\r\noid:%s\r\n", oid_read);
		}
	}
	else if (strncmp((const char *)data, "start::gzcs", strlen("start::gzcs")) == 0) {
		static uint8_t gzcs_key = 1;
		if (gzcs_key) {
			gzcs_key = 0;
			g_gzcs_key = 1;
			debug("%s:%s:start:%s::\r\n",g_fakong, g_time, g_success);
		}
	}
	else if (strncmp((const char *)data, "oid:", strlen("oid:")) == 0) {
		uint8_t oid[OID_LEN + 1] = {0}, oid_read[OID_LEN + 1] = {0};
		//extern uint32_t g_my_oid;

		memcpy(oid, data + 4, OID_LEN);
		fdrive_ioctl(DRIVER_E2PROM, _IOC_E2PROM_OID_WRITE, oid, OID_LEN);
		fdrive_ioctl(DRIVER_E2PROM, _IOC_E2PROM_OID_READ, oid_read, OID_LEN);
		//g_my_oid = atoi(oid_read);
		
		if (!strncmp(oid, oid_read, OID_LEN)) {
			debug("%s:%s:oid:%s::\r\n", g_fakong, g_time, g_success);
		}
		else {
			debug("%s:%s:oid:%s::\r\n", g_fakong, g_time, g_fail);
		}
		g_gzcs_key = 0;
	}
	else if (strncmp((const char *)data, "lora:", strlen("lora:")) == 0) {
		uint8_t ch = atoi(data + 5);
		struct lora_conf conf;
		conf.channel_index = ch;
		conf.channel_step = 100;
		conf.speed_index = 1;

		//初始化Lora频率，速率
		fdrive_ioctl(DRIVER_LORA, _IOC_LORA_SET_CONF, &conf, sizeof(struct lora_conf));
		fdrive_ioctl(DRIVER_LORA, _IOC_LORA_GZCS_TX, NULL, 0);
		fdrive_ioctl(DRIVER_LORA, _IOC_LORA_GZCS_RX, NULL, 0);
	}*/
}

/**************************************************************************************
* FunctionName   : cmdline_rx()
* Description    : 接收命令输入
* EntryParameter : idx,驱动序号， data，接收的数据， len,接收数据长度
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t cmdline_rx(int32_t idx, void *data, int32_t len)
{
	int32_t i = 0;
	uint8_t *tmp = (uint8_t *)data;
	
	for (; i < len; i++) {
		if (tmp[i] != '\r') {
			fdrive_write(DRIVER_LPUART, &tmp[i], 1);
			/* 退格处理 */
			if (0x7f == tmp[i]) {
				g_cmd_buf[--g_buf_cnt] = 0;
				continue;
			}
			
			if (tmp[i] != '\n') {
				g_cmd_buf[g_buf_cnt] = tmp[i];
				g_buf_cnt++;
				if (g_buf_cnt >= 128) {
					g_buf_cnt = 0;
					memset(g_cmd_buf, 0, sizeof(g_cmd_buf));
				}
			}
		}
		else {
			cmdline_dispatcher(g_cmd_buf);
			memset(g_cmd_buf, 0, g_buf_cnt);
			g_buf_cnt = 0;
			fdrive_write(DRIVER_LPUART, TIP, strlen(TIP));
		}
	}

	return 0;
}

/**************************************************************************************
* FunctionName   : cmdline_init()
* Description    : 命令行初始化
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t cmdline_init(void)
{
	fdrive_ioctl(DRIVER_LPUART, _IOC_SET_CB, cmdline_rx, sizeof(ioctl_cb_t));

	fdrive_write(DRIVER_LPUART, TIP, strlen(TIP));
	return 0;
}

/**************************************************************************************
* Description    : 定义通信任务结构
**************************************************************************************/
static __const struct applite cmdline = {
	.idx	= CMDLINE_PID,
	.name   = "cmdline",
	.init   = cmdline_init
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
APP_REGISTER(cmdline);

