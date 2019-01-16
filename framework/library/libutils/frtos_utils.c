/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : frtos_utils.c
 * Author        :
 * Date          : 2017-08-17
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#include "frtos_errno.h"
#include "frtos_mem.h"
#include "frtos_utils.h"

/**************************************************************************************
* FunctionName   : utils_itoa()
* Description    : 数字转字符串
* EntryParameter : val,待转换数字, buf,字符BUF, radix,(进制:2,8,10,16)
* ReturnValue    : 返回错误码, 返回转换后的字符串长度
**************************************************************************************/
int16_t utils_itoa(uint32_t val, void *buf, uint8_t radix)
{
    uint8_t *p = (uint8_t *)buf;
    uint8_t *firstdig = p;
    uint8_t temp = 0;
    uint8_t digval = 0;

    do{
        digval = (uint8_t)(val % radix);
        val /= radix;
        if(digval > 9)
            *p++ = (uint8_t)(digval - 10 + 'a');
        else
            *p++ = (uint8_t)(digval + '0');
    }while(val > 0);

    *p-- = '\0';
    do{
        temp = *p;
        *p = *firstdig;
        *firstdig = temp;
        --p;
        ++firstdig;
    }while(firstdig < p);

    return strlen(buf);
}

/**************************************************************************************
* FunctionName   : utils_calc_bcc()
* Description    : 计算BCC检验码
* EntryParameter : dat,数据, size,数据长度
* ReturnValue    : 返回BCC校验码
**************************************************************************************/
uint8_t utils_calc_bcc(const void *dat, uint16_t size)
{
    uint8_t bcc = *(uint8_t *)dat;
    uint8_t *p = (uint8_t *)dat + 1;

    if(NULL == dat || size <= 0){
        return 0;
    }
    while(--size){
        bcc ^= *p;
        p++;
    }

    return bcc;
}

/**************************************************************************************
* FunctionName   : utils_calc_lrc()
* Description    : 计算LRC检验码
* EntryParameter : dat,数据, size,数据长度
* ReturnValue    : 返回LRCC校验码
**************************************************************************************/
uint8_t utils_calc_lrc(const void *dat, uint16_t size)
{
    uint8_t lrc = *(uint8_t *)dat;
    uint8_t *p = (uint8_t *)dat + 1;

    if(NULL == dat || size <= 0){
        return 0;
    }
    while(--size){
        lrc ^= *p;
        p++;
    }

    return ~lrc;
}

/**************************************************************************************
* FunctionName   : utils_char2bcd()
* Description    : 字符转BCD码(Binary-Coded Decimal)
* EntryParameter : ch,要转换的字符
* ReturnValue    : 返回转换后的BCD码
**************************************************************************************/
uint8_t utils_char2bcd(const uint8_t ch)
{
    if(ch >= '0' && ch <= '9') return ch - '0';
    if(ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
    if(ch >= 'A' && ch <= 'F') return ch - 'A' + 10;

    return 0;
}

/**************************************************************************************
* FunctionName   : utils_str2bcd()
* Description    : 字符串转BCD码(Binary-Coded Decimal)
* EntryParameter : *str,要转换的字符串, bcd_ar,返回转换后的BCD码数组,
                   arr_len,数组长度
* ReturnValue    : 返回转换后的数据个数
**************************************************************************************/
int16_t utils_str2bcd(const char *str, uint8_t *bcd_arr, int16_t arr_len)
{
    int16_t i = 0, j = 0;
    uint8_t high = 0, low = 0;

    if(NULL == bcd_arr || 0 == arr_len){
        return 0;
    }
    for(i = 0; i < (int16_t)strlen(str); i += 2){
        high=utils_char2bcd(str[i]);
        if((i + 1) < (int16_t)strlen(str)){
            low = utils_char2bcd(str[i+1]);
        }else{
            low = 0xF;
        }
        if(j < arr_len){
            bcd_arr[j++] = (high << 4) + low;
        }else{
            return j;
        }
    }

    return j;
}

/**************************************************************************************
* FunctionName   : utils_mem_test()
* Description    : 测试一段内存是否为指定字符
* EntryParameter : ch,待检测的字节, p_mem,待检测内存, len,长度
* ReturnValue    : 返回错误码
**************************************************************************************/
int8_t utils_mem_test(char ch, void *mem, uint16_t len)
{
    uint16_t i = 0;

    for(i = 0; i < len; i++){
        if(((char *)mem)[i] != ch)return -EIO;
    }
    return 0;
}

/**************************************************************************************
* FunctionName   : mem_scannf
* Description    : 数据解析函数，类似于sscanf，但不需要开辟新的内存空间
* EntryParameter : 同sscanf
* ReturnValue    : 返回解析到的位置:0，表示解析失败
**************************************************************************************/
int __optimize("O3") mem_scannf(char *buffer, int len, char *fmt,...)
{
    int i = 0;
    va_list arg;

    va_start(arg,fmt);
    while (unlikely(len > 0 && fmt[i] != '\0')) {
        if (unlikely(fmt[i] == '%' && fmt[i+1] == 'd')) {
            int *intp = va_arg(arg,int *);
            *intp = atoi(buffer);
            i++;
        } else if (unlikely(fmt[i] == '%' && fmt[i+1] == 's')) {
            char **buf = va_arg(arg, char **);
            *buf = buffer;
            i++;
        } else {
           while (likely(len > 0 && *buffer != fmt[i])) {
               buffer++;
               len --;
           }
           if(len > 0 && *buffer == fmt[i]) {
               *buffer++ = '\0';
               len--;
           }
        }
        i++;
    }
    va_end(arg);
    return len;
}

/**************************************************************************************
* FunctionName   : memstr
* Description    : 字符串内存查找，类似于strstr，但支持长度传入，避免越界
* EntryParameter : 同strstr
* ReturnValue    : 返回找到的子串位置，未找到返回NULL
**************************************************************************************/
char __optimize("O3") *memstr(char *src, int len, char *substr)
{
    if (unlikely(src == NULL || len <= 0 || substr == NULL)) {
        return NULL;
    }

    if (unlikely(*substr == '\0')) {
        return NULL;
    }

    int sublen = strlen(substr);

    int i;
    char* cur = src;
    int last_possible = len - sublen + 1;
    for (i = 0; i < last_possible; i++) {
        if (unlikely(*cur == *substr)) {
            if (unlikely(memcmp(cur, substr, sublen) == 0)) {
                return cur;
            }
        }
        cur++;
    }

    return NULL;
}

/**************************************************************************************
* FunctionName   : chksum_xor()
* Description    : 计算校验
* EntryParameter : data，指向待校验数据， len,送数据长度
* ReturnValue    : 返回校验码
**************************************************************************************/
uint8_t chksum_xor(uint8_t *data, int32_t len)
{
    int32_t i = 1;
    uint8_t csum = data[0];

    for (i = 1; i < len; i++) {
        csum ^= data[i];
    }
    return csum;
}

static const uint8_t aucCRCHi[] = {
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40
};

static const uint8_t aucCRCLo[] = {
    0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7,
    0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E,
    0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9,
    0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC,
    0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
    0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32,
    0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D,
    0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A, 0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 
    0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF,
    0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
    0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1,
    0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4,
    0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 
    0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA,
    0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
    0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0,
    0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97,
    0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C, 0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E,
    0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89,
    0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
    0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83,
    0x41, 0x81, 0x80, 0x40
};

/**************************************************************************************
* FunctionName   : crc16()
* Description    : 计算CRC16校验
* EntryParameter : data，指向待校验数据， len,送数据长度
* ReturnValue    : 返回校验码
**************************************************************************************/
uint16_t crc16(uint8_t * pucFrame, uint16_t usLen)
{
    uint8_t ucCRCHi = 0xFF;
    uint8_t ucCRCLo = 0xFF;
    int iIndex;

    while(usLen--)
    {
        iIndex = ucCRCLo ^ *(pucFrame++);
        ucCRCLo = (uint8_t)(ucCRCHi ^ aucCRCHi[iIndex]);
        ucCRCHi = aucCRCLo[iIndex];
    }
    return (uint16_t)(ucCRCHi << 8 | ucCRCLo);
}

 /******************************************************************************
*
* mask_set -
*
* set a bit in the given mask
* 
* ARGUMENTS: 
* mask - the mask to manipulate.
* length - the length of the mask.
* num - the number to set,base on 0
* 
* RETURNS: new mask after set the num.
*
* NOMANUAL
*/
uint8_t *mask_set(uint8_t *mask, uint8_t length, uint8_t num)
{
	uint8_t nBytes;
	uint8_t nBits;

	nBytes = num/8;
	nBits = num%8;

	if ( nBytes >= length )
		return NULL;
	
	mask[nBytes] |= 1<<(7-nBits);
	
	return mask;

}

/******************************************************************************
*
* mask_clear -
*
* clear a bit from the given mask
* 
* ARGUMENTS: 
* mask - the mask to manipulate.
* length - the length of the mask.
* num - the number to clear,base on 0.
* 
* RETURNS: new mask after clear the num.
*
* NOMANUAL
*/
uint8_t *mask_clear(uint8_t *mask, uint8_t length, uint8_t num)
{
	uint8_t nBytes;
	uint8_t nBits;

	nBytes = num/8;
	nBits = num%8;

	if ( nBytes >= length )
		return NULL;
	
	mask[nBytes] &= ~(1<<(7-nBits));
	
	return mask;
}

/******************************************************************************
*
* mask_test -
*
* test a bit in the given mask
* 
* ARGUMENTS: 
* mask - the mask to manipulate.
* length - the length of the mask.
* num - the number to test, based on 0.
* 
* RETURNS: 1 if the num in the mask, else 0.
*
* NOMANUAL
*/
int mask_test(uint8_t *mask, uint8_t length, uint8_t num )
{
	uint8_t nBytes;
	uint8_t nBits;
	int ret = 0;
	
	nBytes = num/8;
	nBits = num%8;

	if (nBytes >= length )
		return 0;
	
	if (mask[nBytes] & (1<<(7-nBits)) )
		ret = 1;

	return ret;
}


/******************************************************************************
 * *
 * * mask_count -
 * *
 * * get the count of bits set 1 in the given mask
 * * 
 * * ARGUMENTS: 
 * * mask - the mask to manipulate.
 * * length - the length of the mask.
 * * num - end of count
 * * 
 * * RETURNS: the count of bits set 1.
 * *
 * * NOMANUAL
 * */

int mask_count(uint8_t *mask, uint8_t length, uint8_t num)
{
	int i = 0, j = 0;
	int count = 0;
	
	for( i=0; i<length; i++ ) {
		for( j=7; j>=0; j--) {
								    
			if ( (i*8 + (7-j)) >= num )
				return count;			

			if ( mask[i] & (1<<j) )
				count++;	
		}
	}

	return count;	
}

