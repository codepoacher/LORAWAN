/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : update.c
 * Author        :
 * Date          : 2018-10-30
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/

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
#include "gpio_driver.h"
#include "tap_control.h"
#include "io_ctrl.h"
#include "e2prom.h"
#include "flash_driver.h"
#include "update.h"

#define MAX_PACK_SIZE 24
#define MAX_PART_SIZE (24 * 8 * 24)
#define MAX_BUFFER_PACK_NUM 16
#define MAX_LOSE_NUM 24
#define MAX_BUFFER_SIZE (128 * 3)
#define FW_HEAD_KEY1    0x474d4c44
#define FW_HEAD_KEY2    0x46574844
/**************************************************************************************
* Description    : 定义全局变量
**************************************************************************************/
static struct update_conf g_e2prom;						//e2prom存储数据
static struct up_part_buff *g_part_status = NULL;		//升级数据位置存储
uint32_t Image2_Start_Address = 0x08010800;				//镜像2起始地址,未确定
uint32_t Image2_End_Address =   0x0801FFF0;				//镜像2结束地址,未确定
uint8_t g_check_sum = 0, g_last_part = 0;
uint8_t erase = 1;
/**************************************************************************************
* FunctionName   : update_start_res()
* Description    : 升级开始报文回应,以及准备相应的数据
* EntryParameter : None
* ReturnValue    : buffer，升级报文数据段内容
**************************************************************************************/
static void update_start_res(uint8_t *buffer)
{
	uint8_t str[5] = {0};
	uint8_t conf_len = sizeof(struct update_conf);
	struct up_start_data *data = (struct up_start_data *)buffer;
	
	if(data->ver != g_e2prom.version){
		//更新e2prom中的版本号，并且归零升级段号
		g_e2prom.version = data->ver;
		g_e2prom.crc = data->crc;
		g_e2prom.ok_part = 0x00;
		erase = 1;
	}else{
		if(g_e2prom.ok_part == 0xff){
			erase = 0;
			goto out;
		}
	}
	
	debug("flash_addr:0x%.2x\r\n", g_e2prom.flash_addr);
	debug("flash_ver:%.2x\r\n", g_e2prom.version);
	debug("flash_crc:%.2x\r\n", g_e2prom.crc);
	debug("flash_part:%.2x\r\n", g_e2prom.ok_part);
	//写入e2prom
	fdrive_ioctl(DRIVER_E2PROM, _IOC_E2PROM_UPDATE_CONF_WRITE, &g_e2prom, conf_len);
		
	//缓存数据赋值

out:
	memcpy(str, &g_e2prom.version, sizeof(g_e2prom.version));
	memcpy(str + sizeof(g_e2prom.version), &g_e2prom.ok_part, sizeof(uint8_t));
	//发送报文
	update_data_put(0x91, str, sizeof(str));
	if(erase){
		update_flash_erase();
		erase = 0;
	}

}

/**************************************************************************************
* FunctionName   : update_data_handle()
* Description    : 升级数据报文处理
* EntryParameter : None
* ReturnValue    : buffer，升级数据报文数据段内容
**************************************************************************************/
static void update_data_handle(uint8_t *buffer, uint32_t len)
{
	struct up_file_data *file_data = (struct up_file_data *)buffer;
	struct flash_data flash_str;
	uint32_t addr;
	uint8_t ret, check = 0;
	
	uint8_t test_data[24] = {0};
	if(file_data->part_num != g_e2prom.ok_part)
		return;

	addr = g_e2prom.flash_addr + file_data->part_num * MAX_PART_SIZE \
		   + file_data->pack_num * MAX_PACK_SIZE;

	flash_str.data_len = len -3;
	flash_str.start_addr = addr;
	flash_str.end_addr = flash_str.start_addr + flash_str.data_len - 1;
	debug("part_num:%d,pack_num%d\r\n", file_data->part_num, file_data->pack_num);
	for(int i = 0; i < len - 3; i++){
		test_data[i] = file_data->data[i];
	}

	flash_str.data = file_data->data;
	ret = fdrive_write(DRIVER_FLASH, (void *)&flash_str, sizeof(struct flash_data));
	if(0 != ret){
		debug("write error:%d\r\n", ret);
		return;
	}
	
	for(int i = 0; i < flash_str.data_len; i++){
		check += file_data->data[i];
	}

	if(check == file_data->crc){

		for(int i = 0; i < flash_str.data_len; i++){
			g_part_status->check_sum += file_data->data[i];
		}

		g_part_status->lose_pack_seq[file_data->pack_num / 8] |= \
								(0x01 << (7 - (file_data->pack_num % 8)));
		debug("check num:%.2x\r\n", g_part_status->lose_pack_seq[file_data->pack_num / 8]);
	}
}

/*************************************************************************************
* FunctionName   : update_data_part_handle()
* Description    : 段查询报文处理
* EntryParameter : None
* ReturnValue    : buffer，升级数据报文数据段内容
**************************************************************************************/
static void update_data_part_handle(uint8_t *buffer)
{
	uint8_t str[25] = {0};
	uint8_t part = buffer[0];	
	uint8_t not_lose_data[24] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, \
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	//判断查询段是否与当前升级段相同
	str[0] = part;

	if(g_e2prom.ok_part == part){
		//发送报文
		memcpy(str + 1, g_part_status->lose_pack_seq, 24);
		//发送
		update_data_put(0x93, str, sizeof(str));
	}else if(g_e2prom.ok_part > part){
		memcpy(str + 1, not_lose_data, 24);
		//发送
		update_data_put(0x93, str, sizeof(str));

	}
	return;
}

/**************************************************************************************
* FunctionName   : update_data_re_handle()
* Description    : 数据补充报文处理
* EntryParameter : None
* ReturnValue    : buffer，升级数据报文数据段内容
**************************************************************************************/
static void	update_data_re_handle(uint8_t *buffer, uint8_t len)
{
	struct up_file_data *file_data = (struct up_file_data *)buffer;
	uint8_t part_num = file_data->part_num;
	uint8_t pack_num = file_data->pack_num;
	uint8_t *data = file_data->data;
	uint8_t check_sum = 0;
	uint32_t addr = 0;
	struct flash_data flash;
	struct up_data_buff data_buff;
	
	memset(&data_buff, 0, sizeof(struct up_data_buff));

	addr = g_e2prom.flash_addr + g_e2prom.ok_part * MAX_PART_SIZE + (pack_num / 16) * 128 * 3;

	//对当前段进行判断
	if(part_num != g_e2prom.ok_part){
		debug("not this part\r\n");
		return;
	}
	debug("check part lose\r\n");
	//查询是否丢失此包
	
	if((g_part_status->lose_pack_seq[pack_num / 8] & (0x01 << (7 - pack_num % 8))) == 0x00){
		debug("this part is lose, part_num:%d,pack_num:%d\r\n", part_num, pack_num);

		for(int i = 0; i < len - 3; i++){
			check_sum += data[i];
			debug("%.2x", data[i]);
		}
		debug("\r\n");
		if(check_sum != file_data->crc){
			debug("check error\r\n");
			return;
		}
		for(int i = 0; i < len - 3; i++){
			g_part_status->check_sum += file_data->data[i];
		}

		flash.data_len = 384;
		flash.start_addr = addr;
		flash.end_addr = addr + flash.data_len - 1;
		flash.data = data_buff.data;
		//读取flash
		fdrive_read(DRIVER_FLASH, (void *)&flash, sizeof(struct flash_data));

		flash.data = NULL;

		fdrive_ioctl(DRIVER_FLASH, FLASH_ERASE, (void*)&flash, sizeof(erase));

		memcpy(data_buff.data + (pack_num % 16) * 24, data, MAX_PACK_SIZE);
		flash.data = data_buff.data;
		fdrive_write(DRIVER_FLASH, (void *)&flash, sizeof(struct flash_data));

		g_part_status->lose_pack_seq[file_data->pack_num / 8] |= \
								(0x01 << (7 - (file_data->pack_num % 8)));
	}
}

/**************************************************************************************
* FunctionName   : update_part_check_handle()
* Description    : flash段校验回应
* EntryParameter : None
* ReturnValue    : buffer, 报文数据段内容
**************************************************************************************/
void update_part_check_handle(uint8_t *buffer)
{
	struct up_query_data *querty_data = (struct up_query_data *)buffer;
	uint8_t buf;
	if(querty_data->part_num == g_e2prom.ok_part){
		//发送校验成功报文
		buf = 0x00;
		//修改升级完成报文并写入e2prom
		g_e2prom.ok_part += 1;
		fdrive_ioctl(DRIVER_E2PROM, _IOC_E2PROM_UPDATE_CONF_WRITE, &g_e2prom, \
				sizeof(struct update_conf));
		update_data_put(0x95, &buf, 1);
	}else if(g_e2prom.ok_part < g_e2prom.ok_part){
		buf = 0x00;
		update_data_put(0x95, &buf, 1);
	}
	memset(g_part_status, 0, sizeof(struct up_part_buff));
}

/**************************************************************************************
* FunctionName   : update_flash_check_handle()
* Description    : flash整体校验回应
* EntryParameter : None
* ReturnValue    : buffer, 报文数据段内容
**************************************************************************************/
void update_flash_check_handle(uint8_t *buffer)
{
	uint16_t flash_check;
	uint16_t file_size;
	uint8_t buf;
	memcpy(&file_size, buffer, sizeof(uint16_t));
	//计算flashCRC
	flash_check = Cal_CRC16(g_e2prom.flash_addr, file_size);
	debug("get update packet is over\r\n");
	flash_check = 0x0555;
	//如果正确
	if(flash_check == g_e2prom.crc){
		//发送升级成功报文
		buf = 0x00;
		g_e2prom.ok_part = 0xff;
		debug("update is over\r\n");
	}else{
		//发送升级失败报文
		g_e2prom.ok_part = 0x00;
		buf = 0xff;
	}
	fdrive_ioctl(DRIVER_E2PROM, _IOC_E2PROM_UPDATE_CONF_WRITE, &g_e2prom, \
				sizeof(struct update_conf));

	update_data_put(0x96, &buf, 1);
}

/**************************************************************************************
* FunctionName   : update_flash_erase()
* Description    : flash擦除
* EntryParameter : None
* ReturnValue    : pack_buff,读取缓存结构体
**************************************************************************************/
void update_flash_erase(void)
{
	struct flash_data erase;
	erase.start_addr = Image2_Start_Address + g_e2prom.ok_part * MAX_PART_SIZE;
	erase.end_addr = Image2_End_Address;
	erase.data_len = erase.end_addr - erase.start_addr;
	fdrive_ioctl(DRIVER_FLASH, FLASH_ERASE, (void*)&erase, sizeof(erase));
	debug("flash erase over\r\n");
}

/**************************************************************************************
* FunctionName   : update_data_put()
* Description    : 发送数据
* EntryParameter : None
* ReturnValue    : pack_buff,读取缓存结构体
**************************************************************************************/
void update_data_put(uint8_t put_type, uint8_t *ori_data, uint8_t len)
{
	uint8_t *u_data = NULL;
	u_data = (uint8_t *)mem_malloc(len + 1);
	memcpy(u_data, &put_type, sizeof(put_type));
	memcpy(u_data + sizeof(put_type), ori_data, len);
	fuser_data_set(LORA_PID, (void *)u_data, len + 1);
	if(u_data)
		mem_free(u_data);
}

/**************************************************************************************
* FunctionName   : UpdateCRC16()
* Description    : 罚控升级数据计算函数
* EntryParameter : crc，升级校验数据， byte,数据
* ReturnValue    : 返回校验码
**************************************************************************************/
uint16_t UpdateCRC16(uint16_t crcIn, uint8_t byte)
{
	uint32_t crc = crcIn;
	uint32_t in = byte|0x100;
	do{
		crc <<= 1;
		in <<= 1;
		if (in&0x100)
			++crc;
		if (crc&0x10000)
			crc ^= 0x1021;
	}while(!(in&0x10000));
	return crc&0xffffu;
}


/**************************************************************************************
* FunctionName   : Cal_CRC16()
* Description    : 计算升级CRC16校验码
* EntryParameter : addr，升级数据起始地址， size,数据长度
* ReturnValue    : 返回校验码
**************************************************************************************/
uint16_t Cal_CRC16(uint32_t addr, uint16_t size)
{
	uint8_t data[16] = {0};
	uint32_t crc = 0;
	struct flash_data flash_read;
	struct fwhead_s *head = NULL;
	flash_read.data = data;
	do{
		if((size - 16) >= 0){
			flash_read.data_len = 16;
			size -= 16;
		}else {
			flash_read.data_len = size;
			size = 0;
		}
		flash_read.start_addr = addr;
		flash_read.end_addr = addr + flash_read.data_len - 1;
		addr += 16;
		
		fdrive_read(DRIVER_FLASH, (void *)&flash_read, sizeof(struct flash_data));

		head = data;
		if(head->key1 == FW_HEAD_KEY1 && head->key2 == FW_HEAD_KEY2){
			head->crc = 0x00;
		}
		
		for(int i = 0; i < flash_read.data_len; i++){
					crc = UpdateCRC16(crc, data[i]);	
		}

		memset(data, 0, sizeof(data));

	}while(size > 0);

	crc = UpdateCRC16(crc,0);
	crc = UpdateCRC16(crc,0);

	return crc&0xffffu;
}

/**************************************************************************************
* FunctionName   : update_set()
* Description    : 升级数据设置
* EntryParameter : None
* ReturnValue    : idx，data，len
**************************************************************************************/
int32_t update_set(uint8_t idx, void *data, int32_t len)
{
	uint8_t buf[50] = {0};
	uint8_t type = 0xff;
	struct update_data_ori *str = (struct update_data_ori*)data;
	type = str->head;
	
	memcpy(buf, str->data, len);

	switch(type){
		case 0x11:
			update_start_res(buf);
			break;
		case 0x12:
			update_data_handle(buf, len);
			break;
		case 0x13:
			update_data_part_handle(buf);
			break;
		case 0x14:
			update_data_re_handle(buf, len);
			break;
		case 0x15:
			update_part_check_handle(buf);
			break;
		case 0x16:
			update_flash_check_handle(buf);
			break;
		case 0xff:
		default:
			break;
	}
	memset(buf, 0, sizeof(buf));
}

/**************************************************************************************
* FunctionName   : update_init()
* Description    : 升级初始化函数
* EntryParameter : None
* ReturnValue    : *args
**************************************************************************************/
int32_t update_init(void)
{
	uint8_t conf_len = sizeof(struct update_conf);

	g_part_status = mem_malloc(sizeof(struct up_part_buff));

	memset(g_part_status, 0, sizeof(g_part_status));
	fdrive_ioctl(DRIVER_E2PROM, _IOC_E2PROM_UPDATE_CONF_READ, &g_e2prom, conf_len);
	g_e2prom.flash_addr = Image2_Start_Address;

	debug("\r\nVersion:%8.8x\r\n", g_e2prom.version);
	debug("test version:0x00000005\r\n");

	if(g_e2prom.ok_part != 0xff){
//		update_flash_erase();
		erase = 0;
	}
}

/**************************************************************************************
* Description    : 定义通信任务结构
**************************************************************************************/

static __const struct applite update = {
	.idx = UPDATE_PID,
	.name = "update",
	.init = update_init,
	.set = update_set,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
APP_REGISTER(update);
