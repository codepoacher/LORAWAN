/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : frtos_libpack.c
 * Author        :
 * Date          : 2017-08-17
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#include "frtos_libpack.h"

/**************************************************************************************
* FunctionName   : pack_rsa()
* Description    : RSA加密打包
* EntryParameter : data,原始数据(目标数据也放这里), size,原始数据长度
* ReturnValue    : 返回加密后的数据长度
**************************************************************************************/
int16_t __default pack_rsa(void *data, const uint16_t size)
{
    (void)data;
    return size;
}

/**************************************************************************************
* FunctionName   : unpack_rsa()
* Description    : RSA加密解包
* EntryParameter : data,原始数据(目标数据也放这里), size,原始数据长度
* ReturnValue    : 返回解密后的数据长度
**************************************************************************************/
int16_t __default unpack_rsa(void *data, const uint16_t size)
{
    (void)data;
    return size;
}


