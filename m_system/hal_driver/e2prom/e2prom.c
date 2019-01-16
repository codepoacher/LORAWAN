#include "frtos_drivers.h"
#include "frtos_mem.h"
#include "frtos_errno.h"
#include "config_driver.h"
#include "e2prom.h"
#include "tap_control.h"
#include "stm32l0xx_hal.h"

/**************************************************************************************
* TypeName       : e2prom_write()
* Description    : 驱动写数据类型，驱动id号， 写入数据缓冲， 写入数据长度
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t e2prom_write(uint8_t idx, void *data, int32_t len)
{
	if (NULL == data) {
		return -EINVAL;
	}

	int8_t ret = 0;
	struct e2prom_s *e_str = (struct e2prom_s*)data;

	HAL_FLASHEx_DATAEEPROM_Unlock();	
	for(uint32_t t = 0; t < e_str->len; t++){
		ret = HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_BYTE, (DATA_EEPROM_BASE + e_str->addr + t), \
				*(e_str->data + t));
		if (ret != HAL_OK) {
			debug("\r\neeprom write error!!\r\n");
			break;
		}
	}
	HAL_FLASHEx_DATAEEPROM_Lock();

	return ret;
}

/**************************************************************************************
* TypeName       : e2prom_read()
* Description    : 驱动读数据类型，驱动id号， 读入数据缓冲， 读入buffer最大长度
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t e2prom_read(uint8_t idx, void *data, int32_t len)
{
	struct e2prom_s *e_str = (struct e2prom_s*)data;

	if (e_str->data == NULL || e_str->len == 0) {
		return -EINVAL;
	}

	for (uint32_t t = 0; t < e_str->len; t++) {
		*(e_str->data + t) = *(uint8_t*)(DATA_EEPROM_BASE + e_str->addr + t);
	}

	return 0;
}

/**************************************************************************************
* FunctionName   : e2prom_ioctrl()
* Description    : 控制
* EntryParameter : *args,参数, len,参数长度
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t e2prom_ioctrl(int8_t idx, int32_t cmd, void *args, int32_t len)
{
    if(unlikely((NULL == args && len > 0) || \
        (NULL != args && len <= 0) || len < 0)){
        return -EINVAL;
    }
	struct e2prom_s edata;
	uint8_t addr;

    // 1.执行控制命令
    switch(cmd){
    case _IOC_E2PROM_PRESS_WRITE:
		addr = E2PROM_PRESS_CONF_ADDR;
        COMM_WRITE(edata,args,addr,sizeof(struct e2prom_press_dev));
		break;
	case _IOC_E2PROM_PRESS_READ:
		addr = E2PROM_PRESS_CONF_ADDR;
        COMM_READ(edata,args,addr,sizeof(struct e2prom_press_dev));
		break;
	case _IOC_E2PROM_OID_WRITE:
		addr = E2PROM_OID_ADDR;
		COMM_WRITE(edata,args,addr,OID_LEN);
		break;
	case _IOC_E2PROM_OID_READ:	
		addr = E2PROM_OID_ADDR;
		COMM_READ(edata,args,addr,OID_LEN);
		break;
	case _IOC_E2PROM_TAP01_WRITE:
		addr = E2PROM_TAP01_STAT_ADDR;
		COMM_WRITE(edata,args,addr,1);
		break;
	case _IOC_E2PROM_TAP01_READ:
		addr = E2PROM_TAP01_STAT_ADDR;
		COMM_READ(edata,args,addr,1);
		break;
	case _IOC_E2PROM_TAP02_WRITE:
		addr = E2PROM_TAP02_STAT_ADDR;
		COMM_WRITE(edata,args,addr,1);
		break;
	case _IOC_E2PROM_TAP02_READ:
		addr = E2PROM_TAP02_STAT_ADDR;
		COMM_READ(edata,args,addr,1);
		break;
	case _IOC_E2PROM_485_WRITE:
		addr = E2PROM_485_CONF_ADDR;
		COMM_WRITE(edata,args,addr,4);
		break;
	case _IOC_E2PROM_485_READ:
		addr = E2PROM_485_CONF_ADDR;
		COMM_READ(edata,args,addr,4);
		break;
	case _IOC_E2PROM_UPDATE_CONF_WRITE:
		addr = E2PROM_UPDATE_CONF_ADDR;
		COMM_WRITE(edata,args,addr,sizeof(struct update_conf));
		break;
	case _IOC_E2PROM_UPDATE_CONF_READ:
		addr = E2PROM_UPDATE_CONF_ADDR;
		COMM_READ(edata,args,addr,sizeof(struct update_conf));
		break;
	default:
        return -EINVAL;
    }

    (void)idx;
    return 0;
}

/**************************************************************************************
* TypeName       : e2prom_init()
* Description    : 驱动初始化类型
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t e2prom_init(void)
{
	struct e2prom_press_dev press_conf;
	uint8_t press_conf_len = sizeof(struct e2prom_press_dev);
	int32_t ret = 0;
	
	/* 读取阀控状态 */
	//阀门1
	fdrive_ioctl(DRIVER_E2PROM, _IOC_E2PROM_TAP01_READ, &g_tap01_stats, 4);
	//阀门2
	fdrive_ioctl(DRIVER_E2PROM, _IOC_E2PROM_TAP02_READ, &g_tap02_stats, 4);
	
	/* 判断压力流量一体计默认值 */
	fdrive_ioctl(DRIVER_E2PROM, _IOC_E2PROM_PRESS_READ, &press_conf, press_conf_len);
	if (0x0a != press_conf.conf_key) {
		float high_vol = 3.0, low_vol = 0.3, press_range = 0.5, pulse_scale = 0.021;
		press_conf.conf_key = 0x0a;
		press_conf.dev_mount = 0;
		press_conf.high_vol = *(int32_t*)&high_vol;
		press_conf.low_vol = *(int32_t*)&low_vol;
		press_conf.press_range = *(int32_t*)&press_range;
		press_conf.pulse_scale = *(int32_t*)&pulse_scale;

		fdrive_ioctl(DRIVER_E2PROM, _IOC_E2PROM_PRESS_WRITE, &press_conf, press_conf_len);
	}

	return ret;
}

static __const struct driver e2prom = {
	.idx	 = DRIVER_E2PROM,
	.write	 = e2prom_write,
	.read	 = e2prom_read,
	.init	 = e2prom_init,
	.ioctl = e2prom_ioctrl,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
//POSTCORE_INIT(e2prom);

