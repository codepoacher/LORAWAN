#include "stm32l0xx_hal.h"
#include "stm32l0xx_hal_flash.h"
#include "stm32l0xx_hal_uart.h"
#include "frtos_sys.h"
#include "frtos_drivers.h"
#include "frtos_errno.h"
#include "config_driver.h"
#include "frtos_softimer.h"
#include "frtos_mem.h"
#include "frtos_queue.h"
#include "frtos_lock.h"
#include "frtos_delay.h"
#include "frtos_app.h"
#include "gpio_driver.h"
#include "config_driver.h"
#include "config_test.h"
#include "config_user.h"
#include "debug.h"
#include "flash_driver.h"

static uint8_t run_init = 0;

static void flash_data_init(struct flash_data *flash, uint32_t start_addr, 
							void *data, int32_t len)
{
	memset(flash, 0, sizeof(flash));
	flash->data = (char*)data;
	flash->data_len = len;
	flash->end_addr = start_addr + len;
	flash->start_addr = start_addr;
}

static int32_t flash_test_run()
{
	char *data = "hello2";
	char save[16] = { 0 };
	struct flash_data flash_data;

	flash_data_init(&flash_data, 0x800fe00, save, sizeof(save));
	fdrive_read(DRIVER_FLASH, &flash_data, sizeof(flash_data));
	if (run_init == 1)   {
		run_init = 2;
		debug("save1 = %s\r\n", save);
		return 0;
	}

	if (run_init == 0) {
		debug("save data \r\n");
		run_init = 1;
		flash_data_init(&flash_data, 0x800fe00, data, strlen(data) + 1);
		debug("flash data_len = %d, %s, %s\r\n", flash_data.data_len, (char*)flash_data.data, data);
		fdrive_ioctl(DRIVER_FLASH, FLASH_ERASE, (void*)&flash_data, sizeof(flash_data));
		fdrive_write(DRIVER_FLASH, (void*)&flash_data, sizeof(flash_data));
	}
}

static int32_t flash_test_init(void)
{
	return 0;
}

static __const struct applite flash = {
	.idx = FLASH_TEST_PID,
	.name = "flash_test",
	.init = flash_test_init,
	.run  = flash_test_run
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
//APP_REGISTER(flash);

