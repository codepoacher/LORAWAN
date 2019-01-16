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
#include "lora_user.h"
#include "proto.h"

static int32_t rs485_config_set(uint8_t idx, void *data, int32_t len)
{
	uint8_t pdata[128] = {0};
	struct lora_data *ldata = pdata;
	struct e2prom_s edata;
	struct rs485_conf rconf;
	struct valve_rs485_config *rs485 = (struct valve_rs485_config*)data;
	struct lora_conf lconf;
	uint8_t *send_data = NULL;

	//写入e2prom
	memset(&edata, 0, sizeof(edata));
	memset(&rconf, 0, sizeof(rconf));
	memcpy(&rconf, rs485, sizeof(rconf));
	edata.addr = E2PROM_485_CONF_ADDR;
	edata.len = sizeof(struct valve_rs485_config) + 
					rs485->rs485_group * sizeof(struct rs485_config_data);
	edata.data = (uint8_t*)rs485;
	fdrive_write(DRIVER_E2PROM, &edata, sizeof(struct e2prom_s));

	//读出e2prom
	rs485 = (struct valve_rs485_config*)mem_malloc(edata.len);
	if (!rs485) 
		return -1;
	edata.data = (uint8_t*)rs485;
	fdrive_read(DRIVER_E2PROM, &edata, sizeof(struct e2prom_s));
	
	//发送应答
	ldata->type = PROTO_RS485_CONFIG;
	memcpy(ldata->data, rs485, edata.len);
	fuser_data_set(LORA_PID, ldata, edata.len + sizeof(struct lora_data));

	return 0;
}

static int32_t rs485_config_init()
{
	return 0;
}

static __const struct applite vc_rs485_config = {
	.idx	= RS485_CONFIG_PID,	
	.name	= "rs485_config",
	.init	= rs485_config_init,
	.set	= rs485_config_set,
};

APP_REGISTER(vc_rs485_config);
