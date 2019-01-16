/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : frtos_utils.h
 * Author        :
 * Date          : 2017-08-17
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#ifndef __FRTOS_UTILS_H__
#define __FRTOS_UTILS_H__

#include "frtos_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************
* FunctionName   : utils_itoa()
* Description    : 数字转字符串
* EntryParameter : val,待转换数字, buf,字符BUF, radix,(进制:2,8,10,16)
* ReturnValue    : 返回错误码, 返回转换后的字符串长度
**************************************************************************************/
int16_t utils_itoa(uint32_t val, void *buf, uint8_t radix);

/**************************************************************************************
* FunctionName   : utils_calc_bcc()
* Description    : 计算BCC检验码
* EntryParameter : dat,数据, size,数据长度
* ReturnValue    : 返回BCC校验码
**************************************************************************************/
uint8_t utils_calc_bcc(const void *dat, uint16_t size);

/**************************************************************************************
* FunctionName   : utils_calc_lrc()
* Description    : 计算LRC检验码
* EntryParameter : dat,数据, size,数据长度
* ReturnValue    : 返回LRCC校验码
**************************************************************************************/
uint8_t utils_calc_lrc(const void *dat, uint16_t size);

/**************************************************************************************
* FunctionName   : utils_char2bcd()
* Description    : 字符转BCD码(Binary-Coded Decimal)
* EntryParameter : ch,要转换的字符
* ReturnValue    : 返回转换后的BCD码
**************************************************************************************/
uint8_t utils_char2bcd(const uint8_t ch);

/**************************************************************************************
* FunctionName   : utils_str2bcd()
* Description    : 字符串转BCD码(Binary-Coded Decimal)
* EntryParameter : *str,要转换的字符串, bcd_ar,返回转换后的BCD码数组,
                   arr_len,数组长度
* ReturnValue    : 返回转换后的数据个数
**************************************************************************************/
int16_t utils_str2bcd(const char *str, uint8_t *bcd_arr, int16_t arr_len);

/**************************************************************************************
* FunctionName   : utils_mem_test()
* Description    : 测试一段内存是否为指定字符
* EntryParameter : ch,待检测的字节, p_mem,待检测内存, len,长度
* ReturnValue    : 返回错误码
**************************************************************************************/
int8_t utils_mem_test(char ch, void *mem, uint16_t len);

/**************************************************************************************
* FunctionName   : utils_hex_dump()
* Description    : 十六进制dump
* EntryParameter : data,数据指针, len,数据长度
* ReturnValue    : None
**************************************************************************************/
void utils_hex_dump(void *data, uint16_t len);

/**************************************************************************************
* FunctionName   : utils_bcd2bin()
* Description    : bcd转换成bin
* EntryParameter : val,转换值
* ReturnValue    : 返回结果
**************************************************************************************/
static inline uint8_t utils_bcd2bin(uint8_t val)
{
    return (val & 0x0f) + (val >> 4) * 10;
}

/**************************************************************************************
* FunctionName   : mem_scannf
* Description    : 数据解析函数，类似于sscanf，但不需要开辟新的内存空间
* EntryParameter : 同sscanf
* ReturnValue    : 返回解析到的位置:0，表示解析失败
**************************************************************************************/
int mem_scannf(char *buffer, int len, char *fmt,...);

/**************************************************************************************
* FunctionName   : memstr
* Description    : 字符串内存查找，类似于strstr，但支持长度传入，避免越界
* EntryParameter : 同strstr
* ReturnValue    : 返回找到的子串位置，未找到返回NULL
**************************************************************************************/
char *memstr(char *src, int len, char *substr);

/**************************************************************************************
* FunctionName   : utils_bin2bcd()
* Description    : bin转换成bcd
* EntryParameter : val,转换值
* ReturnValue    : 返回结果
**************************************************************************************/
static inline uint8_t utils_bin2bcd(uint8_t val)
{
    return ((val / 10) << 4) + val % 10;
}

/**************************************************************************************
* FunctionName   : chksum_xor()
* Description    : 计算校验
* EntryParameter : data，指向待校验数据， len,送数据长度
* ReturnValue    : 返回校验码
**************************************************************************************/
uint8_t chksum_xor(uint8_t *data, int32_t len);

/**************************************************************************************
* FunctionName   : crc16()
* Description    : 计算CRC16校验
* EntryParameter : data，指向待校验数据， len,送数据长度
* ReturnValue    : 返回校验码
**************************************************************************************/
uint16_t crc16(uint8_t * pucFrame, uint16_t usLen);

uint8_t *mask_set(uint8_t *mask, uint8_t length, uint8_t num);
uint8_t *mask_clear(uint8_t *mask, uint8_t length, uint8_t num);
int mask_test(uint8_t *mask, uint8_t length, uint8_t num);
int mask_count(uint8_t *mask, uint8_t length, uint8_t num);

#ifdef __cplusplus
}
#endif

#endif /* __FRTOS_UTILS_H__ */


