#ifndef _E2PROM_H_
#define _E2PROM_H_

#define E2PROM_PRESS_CONF_ADDR	0
#define E2PROM_485_CONF_ADDR	32
#define E2PROM_TAP01_STAT_ADDR	48
#define E2PROM_TAP02_STAT_ADDR	49
#define E2PROM_OID_ADDR			61
#define E2PROM_UPDATE_CONF_ADDR 100
#define	E2PROM_LORA_CONF        200   //200~300 

#define OID_LEN					10

struct e2prom_s {
	uint32_t addr;	//存储地址
	uint32_t len;	//数据长度
	uint8_t *data;	//数据
};

/* 压力流量一体计配置 */
struct e2prom_press_dev {
	uint8_t conf_key;	//用于检测是否需要写入默认配置; 0x0a:表示以写入
	uint8_t dev_mount;	//是否安装压力流量一体计，0:未安装;1:安装
	uint32_t high_vol;		//压力计电压检测最大值
	uint32_t low_vol;		//压力计电压检测最小值
	uint32_t press_range;	//压力计压力检测范围
	uint32_t pulse_scale;	//脉冲系数
};

/* 485配置 */
struct rs485_conf {
	uint8_t group;		//配置组数
	uint8_t ch;			//通道
	uint8_t addr;		//485设备地址
	uint8_t dev_type;	//485设备类型
};

/* 阀控升级配置 */
struct update_conf {
	uint32_t version;				//正在升级的软件版本
	uint32_t flash_addr;			//当前升级写入起始地址
	uint8_t ok_part;				//当前已经升级完成的段号
	uint16_t crc;					//当前升级版本CRC
};

#define COMM_WRITE(a,b,add,l) \
	do{\
		(a).addr = (add); \
		(a).len  = (l); \
		(a).data = (b); \
		fdrive_write(DRIVER_E2PROM, &(a), sizeof(struct e2prom_s)); \
	}while(0)

#define COMM_READ(a,b,add,l) \
	do{\
		(a).addr = (add); \
		(a).len  = (l); \
		(a).data = (b); \
		fdrive_read(DRIVER_E2PROM, &(a), sizeof(struct e2prom_s)); \
	}while(0)

uint8_t EepromWriteBuffer( uint16_t addr, uint8_t *buffer, uint16_t size );
uint8_t EepromReadBuffer( uint16_t addr, uint8_t *buffer, uint16_t size );
#endif

