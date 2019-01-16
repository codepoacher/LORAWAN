/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : i2c_driver.c
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#if 0
#include "frtos_errno.h"
#include "frtos_drivers.h"
#include "frtos_lock.h"
#include "config_driver.h"
#include "i2c_driver.h"
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_i2c.h"
#include "stm32l4xx_hal_i2c_ex.h"

/**************************************************************************************
* Description    : 模块内部数据定义
**************************************************************************************/
static mutex_lock_t i2c_mutex = NULL;           // I2C访问锁
I2C_HandleTypeDef hi2c1;

/**************************************************************************************
* FunctionName   : i2c_transfer()
* Description    : I2C设备阻塞式收发数据
* EntryParameter : msgs,消息数组, cnt,传输Msgs个数
* ReturnValue    : 返回发送成功的个数
**************************************************************************************/
int16_t i2c_transfer(struct i2c_msg_s msgs[], int16_t cnt)
{
    int8_t i = 0;
    int8_t trans_cnt = 0;
    bool addr_10_bit = (msgs[i].flags & I2C_M_TEN) ? true : false;

    // 1.上锁
    mutex_lock(i2c_mutex);
    for (i = 0; i < cnt; i++){
        // 1.1.设置从设备地址
		if (msgs[i].flags & I2C_M_RD){
			HAL_I2C_Mem_Read(&hi2c1, msgs[i].addr, msgs[i].reg_addr, \
							I2C_MEMADD_SIZE_8BIT, msgs[i].data, msgs[i].len, 0xff);
            trans_cnt++;
        }
		else{ 
			HAL_I2C_Mem_Write(&hi2c1, msgs[i].addr, msgs[i].reg_addr, \
							I2C_MEMADD_SIZE_8BIT, msgs[i].data, msgs[i].len, 0xff); 
            trans_cnt++;
        }
    }
    // 2.解锁
    mutex_unlock(i2c_mutex);

    return trans_cnt;
}

/**************************************************************************************
* FunctionName   : i2c_read_block_data()
* Description    : 读取寄存器内容
* EntryParameter : addr,芯片地址, reg_addr,寄存器地址, reg,寄存器数据, 
*				   len,缓存区长度, buf,缓冲区
* ReturnValue    : 返回错误码
**************************************************************************************/
int8_t i2c_read_block_data(uint16_t addr, uint8_t reg, int16_t len, uint8_t *buf)
{
    struct i2c_msg_s msgs[] = {
        [0] = {addr, reg, buf, len, I2C_M_RD},
    };

    if (1 != i2c_transfer(msgs, 1))return -EIO;

    return 0;
}

/**************************************************************************************
* FunctionName   : i2c_write_block_data()
* Description    : 写寄存器内容
* EntryParameter : addr,芯片地址, reg,寄存器地址, len,缓存区长度，buf,缓冲区
* ReturnValue    : 返回错误码
**************************************************************************************/
int8_t i2c_write_block_data(uint8_t addr, uint8_t reg, int16_t len, uint8_t *data)
{
    struct i2c_msg_s msgs[] = {
        [0] = {addr, reg, data, len, I2C_M_WT},
    };

	if (1 != i2c_transfer(msgs, 1))return -EIO;

    return 0;
}

/**************************************************************************************
* FunctionName   : i2c_smbus_write_data()
* Description    : i2c写数据
* EntryParameter : addr,芯片地址, len,缓存区长度，data,缓冲区
* ReturnValue    : 返回错误码
**************************************************************************************/
/*int8_t i2c_smbus_write_data(uint8_t addr, int16_t len, uint8_t *data,uint8_t flag)
{
    struct i2c_msg_s msgs[] = {
        [0] = {addr, data, len, flag},
    };

    if ((i2c_transfer(msgs, 1)) != 1) {
        return -EIO;
    }

    return 0;
}*/

/**************************************************************************************
* FunctionName   : i2c_smbus_read_data()
* Description    : 写寄存器内容
* EntryParameter : addr,芯片地址, len,缓存区长度，data,缓冲区
* ReturnValue    : 返回错误码
**************************************************************************************/
/*int8_t i2c_smbus_read_data(uint8_t addr, int16_t len, uint8_t *data,uint8_t flag)
{
    struct i2c_msg_s msgs[] = {
        [0] = {addr, data, len, I2C_M_RD|flag},
    };

    if ((i2c_transfer(msgs, 1)) != 1) {
        return -EIO;
    }

    return 0;
}*/

/**************************************************************************************
* FunctionName   : i2c_phy_init()
* Description    : I2C物理接口初始化
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
static int32_t i2c_phy_init(void)
{
    // 1.初始化I2C, Master
	hi2c1.Instance = I2C1;
	hi2c1.Init.Timing = 0x00506682;
  	hi2c1.Init.OwnAddress1 = 0;
  	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  	hi2c1.Init.OwnAddress2 = 0;
  	hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  	if (HAL_I2C_Init(&hi2c1) != HAL_OK)
		return -EINVAL;

  	/**Configure Analogue filter 
  	*/
  	if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
		return -EINVAL;

  	/**Configure Digital filter 
  	*/
  	if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
		return -EINVAL;

	return 0;
}

/**************************************************************************************
* FunctionName   : i2c_ioctrl()
* Description    : I2C应用控制
* EntryParameter : *args,参数, len,参数长度
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t i2c_ioctrl(uint8_t idx, int32_t cmd, void *args, int32_t len)
{
    if(unlikely((NULL == args && len > 0) || \
        (NULL != args && len <= 0) || len < 0)){
        return -EINVAL;
    }

    // 1.执行命令序列
    switch(cmd){
    case _IOC_BUS_INIT:
        i2c_phy_init();
        break;
    case _IOC_BUS_TRANSPORTS:
        if((int16_t )(len / sizeof(struct i2c_msg_s)) != \
            i2c_transfer((void *)args, len / sizeof(struct i2c_msg_s))){
            return -EIO;
        }
        break;
    default:
        return -EINVAL;
    }

    (void)idx;
    return 0;
}

/**************************************************************************************
* FunctionName   : i2c_init()
* Description    : I2C设备初始化
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
static int32_t __init i2c_init(void)
{
	uint32_t  ret = 0;
    // 1.初始化物理接口
    ret = i2c_phy_init();
	if (ret != 0)
		return ret;

    // 2.创建互斥访问锁
    i2c_mutex = mutex_lock_init();
    if(NULL == i2c_mutex) return -EPERM;

    return 0;
}

static __const struct driver i2c = {
    .idx   = DRIVER_I2C,
    .init  = i2c_init,
    .ioctl = i2c_ioctrl,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
POSTCORE_INIT(i2c);
#endif
