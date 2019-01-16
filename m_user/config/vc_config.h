#ifndef VC_CONFIG_H
#define VC_CONFIG_H

/* 阀控压力、脉冲配置 */
struct valve_config {
	uint8_t pfm;                //是否开启压力流量一体计(pressure and flow meter)
	uint32_t pressure_range;    //压力量程
	uint32_t pressure_max;      //压力最大值
	uint32_t pressure_min;      //压力最低值
	uint32_t impulse;           //脉冲
}__attribute__((packed));

struct rs485_config_data {
	uint8_t rs485_ch;            //rs485通道
	uint8_t rs485_addr;		     //rs485地址
	uint8_t rs485_type;          //rs485外设类型
}__attribute__((packed));

/* rs485 配置 */
struct valve_rs485_config {
	uint8_t rs485_group;         //rs485组数
	struct rs485_config_data data[0];  //rs485配置数据
}__attribute__((packed));

#endif
