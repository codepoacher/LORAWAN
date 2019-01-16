#ifndef _IA_PROTO_H_
#define _IA_PROTO_H_

#include "stm32l0xx_hal.h"

#define PROTO_UPLINK_MASK				0x80		// 节点回应报文均带上此掩码

/*lora通信 控制协议相关*/
#define	PROTO_REG			            0xFF	// 入网
#define PROTO_KEEPALIVE1	            0X01	// 报文报文1
#define PROTO_KEEPALIVE2	            0X02	// 报文报文2
#define PROTO_KEEPALIVE3	            0X03	// 报文报文3
#define PROTO_KEEPALIVE4	            0X04	// 报文报文4
#define PROTO_POWER			            0X08	// 电量查询
#define PROTO_VALVE_CONTROL	            0x09	// 阀门控制
#define PROTO_VERSION	                0x0A	// 软件版本查询
#define PROTO_LORA_SET	                0x0B	// LORA 参数设置
#define PROTO_PRESSURE_CONFIG		    0x0C	// 设置阀控压力配置
#define PROTO_RS485_CONFIG				0x0D	// 设置阀控rs485配置
#define PROTO_CHANGE_NETWORK            0x0E    // 修改网络号 

#define PROTO_LORA_TEST                 0x20

/*阀控升级协议相关*/
#define PROTO_UPDATE_START				0x11	//升级开始
#define PROTO_UPDATE_DATA				0x12	//升级数据
#define PROTO_UPDATE_PRAT				0x13	//升级段查询
#define PROTO_UPDATE_DATA_RE			0x14	//升级数据重传
#define PROTO_UPDATE_PRAT_CRC			0x15	//升级段校验
#define PROTO_UPDATE_OVER				0x16	//升级结束

/*偏移量相关*/
#define PROTO_TYPE_OFFSET				0x2		// type偏移
#define PROTO_CRC_OFFSET				0x4		// crc偏移
#define PROTO_NETWORK_OFFSET			0x5		// 网络号偏移
#define PROTO_NODECOUNT_OFFSET			0x6		// 组网情况下通告本次入网节点个数
#define PROTO_NODEBIT_OFFSET			0x6		// 组播情况下通告节点位图


/* 其他 */
#define MAX_NODE_BYTES					31
#define PROTO_BROADCAST					0xFF
#define PROTO_MULTICAST_MASK			0xC0


#define ENCAP_PACKET_HEAD(p, addr, t, net, l) \
	do { \
		struct proto_head *psend = (struct proto_head *)(p);	\
		uint8_t crc = 0;	\
		psend->id = (addr);	\
		psend->type = (t);	\
		psend->len = (l);	\
		psend->crc = 0;	\
		psend->network = (net);	\
		crc = (uint8_t)crc16((p), (l)); \
		psend->crc = crc; \
	}while(0)

// 报文头
struct proto_head {	
	uint8_t id;		//   节点ID, 下行报文时可能是ff，表示广播；fe，表示组播	
	uint8_t type;	
	uint8_t len;	
	uint8_t crc;	
	uint8_t network;	
	uint8_t data[0]; 	//数据
}__attribute__((packed));

struct proto_lora {	
	uint8_t channel_index;
	uint8_t channel_step;	
	uint8_t speed_index;	
	uint8_t timeout;	
}__attribute__((packed));

// keepalive 应答
struct proto_keepalive1 {	
	uint8_t valve_status;
	float pluse;	
	uint16_t adc1;	
	uint16_t adc2;	
}__attribute__((packed));

struct proto_keepalive2 {	
	uint8_t valve_status;
	float pluse;	
	uint16_t adc1;	
	uint16_t adc2;
	uint32_t rs485_1;
	uint32_t rs485_2;
}__attribute__((packed));

struct proto_keepalive3 {	
	uint8_t valve_status;
	float pluse;	
	uint16_t adc1;	
	uint16_t adc2;
	uint32_t rs485_1;
	uint32_t rs485_2;	
	uint16_t batter;
	uint16_t solar;
}__attribute__((packed));

struct lora_data {
	uint8_t type;
	uint8_t data[0];
}__attribute__((packed));

#endif
