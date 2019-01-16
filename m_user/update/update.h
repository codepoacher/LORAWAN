#ifndef __UPDATE_H__
#define __UPDATE_H__

/**************************************************************************************
* Description    : 升级位置存储格式定义
**************************************************************************************/
struct up_part_buff
{
	uint8_t lose_pack_seq[24];	//丢失报文序列号(按位存储，默认全为0)
	uint8_t check_sum;			//累加和校验码
};

/**************************************************************************************
* Description    : 升级缓存数据存储格式定义
**************************************************************************************/
struct up_data_buff
{
	uint32_t buff_start_addr;		//缓存数据在flash的写入地址
	uint8_t data[384];				//缓存数据
};

/**************************************************************************************
* Description	 : 升级报文数据原始数据格式定义
**************************************************************************************/
struct update_data_ori
{
	uint8_t head;					//头报文
	uint8_t *data;					//数据
};

struct up_start_data
{
	uint32_t ver;
	uint16_t crc;
}__attribute__((packed));

struct up_file_data
{
	uint8_t part_num;
	uint8_t pack_num;
	uint8_t crc;
	char data[24];
}__attribute__((packed));

struct up_query_data
{
	uint8_t part_num;
}__attribute__((packed));


typedef struct fwhead_s
{
	uint32_t key1;           //固件头标识1
	uint8_t type;            //固件类型，0:网关,1:节点
	uint8_t ver;             //固件版本
	uint16_t crc;             //固件校验
	uint32_t total;          //固件长度，包括固件头
	uint32_t key2;           //固件头标识2
}__attribute__((packed));

/**************************************************************************************
* Description	 : 升级相关函数定义
**************************************************************************************/
void update_part_check_handle(uint8_t *buffer);
void update_flash_check_handle(uint8_t *buffer);
void update_flash_write(struct up_data_buff *pack_buff);
void update_flash_read(struct up_data_buff *pack_buff);
void update_flash_erase(void);
uint16_t UpdateCRC16(uint16_t crcIn, uint8_t byte);
uint16_t Cal_CRC16(uint32_t addr, uint16_t size);
void update_data_put(uint8_t put_type, uint8_t *ori_data, uint8_t len);
int32_t update_set(uint8_t idx, void *data, int32_t len);
int32_t update_init(void);

#endif /*__UPDATE_H__*/
