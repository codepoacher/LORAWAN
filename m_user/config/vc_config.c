#include "frtos_app.h"
#include "frtos_mem.h"
#include "frtos_errno.h"
#include "config_user.h"
#include "config_driver.h"
#include "frtos_sys.h"
#include "debug.h"
#include "config_user.h"
#include "e2prom.h"
#include "vc_config.h"
#include "proto.h"

static int32_t vc_config_set(uint8_t idx, void *data, int32_t len)
{
	uint8_t pdata[128] = {0};
	struct lora_data *ldata = pdata;
	struct e2prom_s edata;
	struct e2prom_press_dev dev;
	struct valve_config *vc = (struct valve_config *)data;
	struct valve_config vc_rep;

	if (len <= 0) 
		return -1;

	//读取当前配置
	memset(&edata, 0, sizeof(edata));
	memset(&dev, 0, sizeof(dev));
	edata.addr = E2PROM_PRESS_CONF_ADDR;
	edata.len = sizeof(dev);
	edata.data = (uint8_t*)&dev;
	fdrive_read(DRIVER_E2PROM, &edata, sizeof(struct e2prom_s));


	//设置配置
	dev.dev_mount = vc->pfm;
	dev.high_vol = vc->pressure_max;
	dev.low_vol = vc->pressure_min;
	dev.press_range = vc->pressure_range;	
	dev.pulse_scale = vc->impulse;
	fdrive_write(DRIVER_E2PROM, &edata, sizeof(struct e2prom_s));

	//发送应答
	memset(&dev, 0, sizeof(dev));
	fdrive_read(DRIVER_E2PROM, &edata, sizeof(struct e2prom_s));
	memset(&vc_rep, 0, sizeof(vc_rep));
	vc_rep.pfm = dev.dev_mount;
	vc_rep.pressure_max = dev.high_vol;
	vc_rep.pressure_min = dev.low_vol;
	vc_rep.pressure_range = dev.press_range;
	vc_rep.impulse = dev.pulse_scale;
	
	ldata->type = PROTO_PRESSURE_CONFIG;
	memcpy(ldata->data, &vc_rep, sizeof(vc_rep));

	fuser_data_set(LORA_PID, ldata, sizeof(vc_rep) + sizeof(struct lora_data));

	return 0;
}

//static int32_t vc_config_get(uint8_t idx, void *data, int32_t len)
//{
//	return 0;
//}

static int32_t vc_config_init()
{
	return 0;
}

static __const struct applite vc_config = {
	.idx = VC_CONFIG_PID,
	.name = "vc_config",
	.init = vc_config_init,
	.set = vc_config_set,
};

APP_REGISTER(vc_config);
