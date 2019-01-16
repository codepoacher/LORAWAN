#if 1
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
#include "gpio_driver.h"
#include "config_driver.h"
#include "config_user.h"
#include "debug.h"
#include "flash_driver.h"

#define STM32L0XX_FLASH_SIZE		(128 * 1024)
#define STM32L0XX_FLASH_START_ADDR  (FLASH_BASE + 10 * 1024)
#define STM32L0XX_FLASH_END_ADDR	(FLASH_BASE + STM32L0XX_FLASH_SIZE)

/************************************************************************************* 
* FunctionName   : flash_page_get()
* Description    : 通过地址获取所在的页号
* EntryParameter : Addr,flash地址 
* ReturnValue    : 错误码
*************************************************************************************/
//static uint32_t flash_page_get(uint32_t Addr)
//{
//	uint32_t page = 0;
//	
//	if (Addr < (FLASH_BASE + FLASH_BANK_SIZE))  /* Bank 1 */
//	  page = (Addr - FLASH_BASE) / FLASH_PAGE_SIZE;
//	else /* Bank 2 */
//	  page = (Addr - (FLASH_BASE + FLASH_BANK_SIZE)) / FLASH_PAGE_SIZE;
//	
//	return page;
//}

/************************************************************************************* 
* FunctionName   : flash_write()
* Description    : flash 写
* EntryParameter : idx,flash 驱动号，data,struct flash_data结构指针，
*				   len,struct flash_data结构大小
* ReturnValue    : 错误码
*************************************************************************************/
static int32_t flash_write(uint8_t idx, void *data, int32_t len)
{
	int8_t ret = 0;
	uint8_t *write_data = NULL;
	uint8_t dw_len = sizeof(uint32_t);
	uint32_t Address = 0;
	uint64_t d = 0;
	struct flash_data *fdata = (struct flash_data *)data;

	if (!data || len <= 0)	
		return -EINVAL;

	if (fdata->end_addr > STM32L0XX_FLASH_END_ADDR)
		return -EINVAL;
	if (fdata->start_addr < STM32L0XX_FLASH_START_ADDR)
		return -EINVAL;
	if (fdata->start_addr + fdata->data_len > STM32L0XX_FLASH_END_ADDR)
		return -EINVAL;

	Address = fdata->start_addr;

	HAL_FLASH_Unlock();
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR);
	len = fdata->data_len;
	write_data = fdata->data;
	while (len > 0) {
		if (len < dw_len) {
			memcpy(&d, write_data, len);
			len = 0;
		} else {
			memcpy(&d, write_data, dw_len);
			len -= dw_len;
		}
		
		ret = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, Address, d);
		if (ret != HAL_OK) {
			debug("HAL_FLASH_Program error: addr = %x, d = %x\r\n", Address, d);
			debug("3, SR = %x\r\n", FLASH->SR);
			HAL_FLASH_Lock();	
			return -EFAULT;
		}
		Address += dw_len;
		write_data = write_data + dw_len;
	}
	HAL_FLASH_Lock();

	return 0;	
}

/************************************************************************************* 
* FunctionName   : flash_read()
* Description    : flash 读
* EntryParameter : idx,flash 驱动号，data,struct flash_data结构指针，
*				   len,struct flash_data结构大小
* ReturnValue    : 错误码
*************************************************************************************/
static int32_t flash_read(uint8_t idx, void *data, int32_t len)
{
	uint8_t dlen = sizeof(uint32_t);
	uint8_t read_len = 0;
	uint8_t *read_data = NULL;
	uint32_t Address = 0;
	struct flash_data *fdata = (struct flash_data*)data;

	if (!data || len <= 0)
		return 0;
	if (fdata->end_addr > STM32L0XX_FLASH_END_ADDR)
		return -EINVAL;
	if (fdata->start_addr < STM32L0XX_FLASH_START_ADDR)
		return -EINVAL;
	if (fdata->start_addr + fdata->data_len > STM32L0XX_FLASH_END_ADDR)
		return -EINVAL;
	if (!fdata->data || fdata->data_len < 0)
		return -EINVAL;

	Address = fdata->start_addr;
	read_data = fdata->data;
	len = fdata->data_len;

	while (len > 0) {
		if (len < dlen) {
			read_len = len;
			len = 0;
		} else  {
			read_len = dlen;
			len -= dlen;
		}
		*(uint32_t*)(read_data) = *(__IO uint32_t*)Address;

		read_data += dlen;
		Address += dlen;
	}
	return 0;
}

/************************************************************************************* 
* FunctionName   : flash_erase()
* Description    : flash 擦除
* EntryParameter : flash_data,指明需要擦除的flash地址
* ReturnValue    : 错误码
*************************************************************************************/
static int32_t flash_erase(struct flash_data *flash_data)
{

	uint32_t PageError = 0;
	HAL_StatusTypeDef ret = HAL_OK;
	FLASH_EraseInitTypeDef erase_init;

	HAL_FLASH_Unlock();
	//__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR);
	memset(&erase_init, 0, sizeof(erase_init));
	erase_init.TypeErase	= FLASH_TYPEERASE_PAGES;
	erase_init.NbPages		= flash_data->data_len / FLASH_PAGE_SIZE; //flash_page_get(flash_data->start_addr);
	erase_init.NbPages	   += flash_data->data_len % FLASH_PAGE_SIZE > 0 ? 1 : 0;
	erase_init.PageAddress  = flash_data->start_addr;
	//erase_init.NbPages		= flash_page_get(flash_data->end_addr + FLASH_PAGE_SIZE - 1);
	//erase_init.NbPages		-= erase_init.Page + 1;
	//erase_init.Banks		= FLASH_BANK_1;
	ret = HAL_FLASHEx_Erase(&erase_init, &PageError);
	if (ret != HAL_OK) {
		HAL_FLASH_Lock();
		debug("flash erase error\r\n");
		return -EFAULT;
	}
	HAL_FLASH_Lock();

	return 0;
}

/************************************************************************************* 
* FunctionName   : flash_ioctl()
* Description    : flash 控制
* EntryParameter : idx,flash驱动id, cmd,flash控制命令, args,flash命令参数， 
*				   len,flash命令参数长度
* ReturnValue    : 错误码
*************************************************************************************/
static int32_t flash_ioctl(uint8_t idx, int32_t cmd, void *args, int32_t len)
{
	int32_t ret = 0;
	struct flash_data *fdata = (struct flash_data*)args;

	if (!args || len <= 0 || (args && len <=0) || (!args && len != 0))
		return -EINVAL;

	switch (cmd) {
		case FLASH_ERASE:
			ret = flash_erase(fdata);
			break;
		default:
			break;
	}

	return ret;
}

/*************************************************************************************
* FunctionName   : flash_init()
* Description    : 初始化flash
* EntryParameter : None
* ReturnValue    : 错误码
*************************************************************************************/
static int32_t __init flash_init(void)
{
	return 0;
}

static __const struct driver flash = {
	.idx	= DRIVER_FLASH,
	.init	= flash_init,
	.write	= flash_write,
	.read	= flash_read,
	.ioctl	= flash_ioctl
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
POSTCORE_INIT(flash);
#endif
