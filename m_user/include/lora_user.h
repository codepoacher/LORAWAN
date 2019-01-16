#ifndef _LORA_USER_H_
#define _LORA_USER_H_

#include "rtc_driver.h"

// 此文件记录lora协议的相关配置

/**************************************************************************************
* Description    : 定义lora数据最大长度
**************************************************************************************/
#define MAX_DATA_LEN                 100
#define MAX_SUBPROTO_LEN             50

#define MAIN_VOL_ID                  1

#define SEND_INTERVAL                1   //阀控发送间隔为1s

#define MAXIMUM_WDOG                 15  //最长为狗时间

#define SLEEP_TIME                   5   //阀控正常休眠周期5s

/* 阀门控制相关 */
#define VALUE_CONTROL_REPLY_DELAY	 6	// 阀门控制应答等待时间
#define VALUE_CONTROL_BUSY           1  
#define VALUE_CONTROL_FREE           0

/* 阀控状态 */
#define LORA_MS_M0  0
#define LORA_MS_M1	1
#define LORA_MS_M2	2

struct lora_conf {
	uint8_t network;
	uint8_t addr;
	uint8_t channel_index;
	uint8_t channel_step;
	uint8_t speed_index;
	uint8_t timeout;
};

struct lora_status {
	uint8_t lora_ms;
	uint8_t version;   //版本协议号 
	uint8_t node_count;
	uint8_t need_send_len;
	uint8_t send_index;
	uint8_t sleep_time;
	uint8_t value_ctl;
    struct tm_conf last_rx_time;	// 记录最后一次收到网关报文的时间
};

#if 1
#define LRDEBUG(format, ...) lprint(format,  ##__VA_ARGS__)
#else
#define LRDEBUG(format, ...) 
#endif

#endif
