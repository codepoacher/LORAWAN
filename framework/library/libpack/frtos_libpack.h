/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : frtos_libpack.h
 * Author        :
 * Date          : 2017-08-17
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#ifndef __FRTOS_LIBPACK_H__
#define __FRTOS_LIBPACK_H__

#include "frtos_utils.h"
#include "frtos_time.h"

#if !defined(__BYTE_ORDER__) || !defined(__ORDER_BIG_ENDIAN__)
#error "The compiler does not support the libpack library."
#endif

/**************************************************************************************
* Description    : 字节交换函数
**************************************************************************************/
#define BYTE_SWAP16(x)      __builtin_bswap16(x)
#define BYTE_SWAP32(x)      __builtin_bswap32(x)
#define BYTE_SWAP64(x)      __builtin_bswap64(x)

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************
* FunctionName   : pack_be8()
* Description    : 8位大端格式打包
* EntryParameter : src,原始数据, *dst,目标数据指针
* Returnsrcue    : 返回打包后数据长度
**************************************************************************************/
static inline int8_t pack_be8(const uint8_t src, uint8_t *dst)
{
    *dst = src;
    return sizeof(uint8_t);
}

/**************************************************************************************
* FunctionName   : pack_be8()
* Description    : 8位小端格式打包
* EntryParameter : src,原始数据, *dst,目标数据指针
* Returnsrcue    : 返回打包后数据长度
**************************************************************************************/
static inline int8_t pack_le8(const uint8_t src, uint8_t *dst)
{
    *dst = src;
    return sizeof(uint8_t);
}

/**************************************************************************************
* FunctionName   : pack_be16()
* Description    : 16位大端格式打包
* EntryParameter : src,原始数据, *dst,目标数据指针
* Returnsrcue    : 返回打包后数据长度
**************************************************************************************/
static inline int8_t pack_be16(const uint16_t src, uint16_t *dst)
{
    #if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    *dst = src;
    #else
    *dst = BYTE_SWAP16(src);
    #endif

    return sizeof(uint16_t);
}

/**************************************************************************************
* FunctionName   : pack_le16()
* Description    : 16位小端格式打包
* EntryParameter : src,原始数据, *dst,目标数据指针
* Returnsrcue    : 返回打包后数据长度
**************************************************************************************/
static inline int8_t pack_le16(const uint16_t src, uint16_t *dst)
{
    #if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    *dst = BYTE_SWAP16(src);
    #else
    *dst =  src;
    #endif

    return sizeof(uint16_t);
}

/**************************************************************************************
* FunctionName   : pack_be32()
* Description    : 32位大端格式打包
* EntryParameter : src,原始数据, *dst,目标数据指针
* Returnsrcue    : 返回打包后数据长度
**************************************************************************************/
static inline int8_t pack_be32(const uint32_t src, uint32_t *dst)
{
    #if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    *dst = src;
    #else
    *dst = BYTE_SWAP32(src);
    #endif

    return sizeof(uint32_t);
}

/**************************************************************************************
* FunctionName   : pack_le32()
* Description    : 32位小端格式打包
* EntryParameter : src,原始数据, *dst,目标数据指针
* Returnsrcue    : 返回打包后数据长度
**************************************************************************************/
static inline int8_t pack_le32(const uint32_t src, uint32_t *dst)
{
    #if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    *dst =  BYTE_SWAP32(src);
    #else
    *dst = src;
    #endif

    return sizeof(uint32_t);
}

/**************************************************************************************
* FunctionName   : pack_be64()
* Description    : 64位大端格式打包
* EntryParameter : src,原始数据, *dst,目标数据指针
* Returnsrcue    : 返回打包后数据长度
**************************************************************************************/
// FIXME:对结构体中packed对其的64位数据打包会崩溃
static inline int8_t pack_be64(const uint64_t src, uint64_t *dst)
{
    #if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    *dst = src;
    #else
    *dst = BYTE_SWAP64(src);
    #endif

    return sizeof(uint64_t);
}

/**************************************************************************************
* FunctionName   : pack_le64()
* Description    : 64位小端格式打包
* EntryParameter : src,原始数据, *dst,目标数据指针
* Returnsrcue    : 返回打包后数据长度
**************************************************************************************/
static inline int8_t pack_le64(const uint64_t src, uint64_t *dst)
{
    #if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    *dst = BYTE_SWAP64(src);
    #else
    *dst = src;
    #endif

    return sizeof(uint64_t);
}

/**************************************************************************************
* FunctionName   : pack_simno()
* Description    : 终端手机号打包
* EntryParameter : src,原始数据, *dst,加密后的数据
* ReturnValue    : 返回加打包后的数据长度
**************************************************************************************/
static inline int8_t pack_simno(const uint8_t src[6], uint8_t dst[6])
{
    if(src != dst){
        memcpy(dst, src, 6);
    }
    return 6;
}

/**************************************************************************************
* FunctionName   : pack_bin()
* Description    : 二进制打包
* EntryParameter : src,原始数据, size,原始数据长度, *dst,打包后的消息
* ReturnValue    : 返回加打包后的数据长度
**************************************************************************************/
static inline int16_t pack_bin(const void *src, const int16_t size, void *dst)
{
    if(src != dst){
        memcpy((uint8_t *)dst, src, size);
    }

    return size;
}

/**************************************************************************************
* FunctionName   : pack_str()
* Description    : 字符串打包
* EntryParameter : src,原始数据, *dst,打包后的字符串
* ReturnValue    : 返回加打包后的数据长度
**************************************************************************************/
static inline int16_t pack_str(const uint8_t *src, void *dst)
{
    if(src != dst){
        strcpy((char *)dst, (char *)src);
    }

    return sizeof(src) + 1;
}

/**************************************************************************************
* FunctionName   : pack_bcd()
* Description    : BCD码打包
* EntryParameter : src,原始数据, size,原始数据长度, *dst,打包后的BCD码
* ReturnValue    : 返回加打包后的数据长度
**************************************************************************************/
static inline int16_t pack_bcd(const uint8_t *src, const int16_t size, \
    uint8_t *dst)
{
    if(src != dst){
        memcpy((uint8_t *)dst, src, size);
    }

    return size;
}

/**************************************************************************************
* FunctionName   : pack_time()
* Description    : 时间打包
* EntryParameter : *dst,打包后的时间
* ReturnValue    : 返回加打包后的数据长度
**************************************************************************************/
static inline int16_t pack_time(uint8_t dst[6])
{
    time_getbtime(dst);
    return 6;
}

/**************************************************************************************
* FunctionName   : pack_bcc()
* Description    : 消息BCC校验码
* EntryParameter : src,原始数据, size,原始数据长度, *dst,打包后的BCC码
* ReturnValue    : 返回加打包后的数据长度
**************************************************************************************/
static inline int16_t pack_bcc(const void *src, const int16_t size, void *dst)
{
    *((uint8_t *)dst) = utils_calc_bcc(src, size);
    return sizeof(uint8_t);
}

/**************************************************************************************
* FunctionName   : pack_lrc()
* Description    : 消息LRC校验码
* EntryParameter : src,原始数据, size,原始数据长度, *dst,打包后的BCC码
* ReturnValue    : 返回加打包后的数据长度
**************************************************************************************/
static inline int16_t pack_lrc(const void *src, const int16_t size, void *dst)
{
    *((uint8_t *)dst) = utils_calc_lrc(src, size);
    return sizeof(uint8_t);
}

/**************************************************************************************
* FunctionName   : pack_rsa()
* Description    : RSA加密打包
* EntryParameter : data,原始数据(目标数据也放这里), size,原始数据长度
* ReturnValue    : 返回加密后的数据长度
**************************************************************************************/
int16_t pack_rsa(void *data, const uint16_t size);

/**************************************************************************************
* FunctionName   : unpack_be8()
* Description    : 8位大端格式解包
* EntryParameter : src,原始数据, *dst,目标数据指针
* ReturnValue    : 返回解包后数据长度
**************************************************************************************/
static inline int8_t unpack_be8(const uint8_t src, uint8_t *dst)
{
    *dst = src;
    return sizeof(uint8_t);
}

/**************************************************************************************
* FunctionName   : unpack_le8()
* Description    : 8位小端格式解包
* EntryParameter : src,原始数据, *dst,目标数据指针
* ReturnValue    : 返回解包后数据长度
**************************************************************************************/
static inline int8_t unpack_le8(const uint8_t src, uint8_t *dst)
{
    *dst = src;
    return sizeof(uint8_t);
}

/**************************************************************************************
* FunctionName   : unpack_be16()
* Description    : 16位大端格式解包
* EntryParameter : src,原始数据, *dst,目标数据指针
* ReturnValue    : 返回解包后数据长度
**************************************************************************************/
static inline int8_t unpack_be16(const uint16_t src, uint16_t *dst)
{
    #if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    *dst = src;
    #else
    *dst = BYTE_SWAP16(src);
    #endif

    return sizeof(uint16_t);
}

/**************************************************************************************
* FunctionName   : unpack_le16()
* Description    : 16位小端格式解包
* EntryParameter : src,原始数据, *dst,目标数据指针
* ReturnValue    : 返回解包后数据长度
**************************************************************************************/
static inline int8_t unpack_le16(const uint16_t src, uint16_t *dst)
{
    #if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    *dst =  BYTE_SWAP16(src);
    #else
    *dst = src;
    #endif

    return sizeof(uint16_t);
}

/**************************************************************************************
* FunctionName   : unpack_be32()
* Description    : 32位大端格式解包
* EntryParameter : src,原始数据, *dst,目标数据指针
* ReturnValue    : 返回解包后数据长度
**************************************************************************************/
static inline int8_t unpack_be32(const uint32_t src, uint32_t *dst)
{
    #if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    *dst = src;
    #else
    *dst = BYTE_SWAP32(src);
    #endif

    return sizeof(uint32_t);
}

/**************************************************************************************
* FunctionName   : unpack_le32()
* Description    : 32位小端格式解包
* EntryParameter : src,原始数据, *dst,目标数据指针
* ReturnValue    : 返回解包后数据长度
**************************************************************************************/
static inline int8_t unpack_le32(const uint32_t src, uint32_t *dst)
{
    #if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    *dst = BYTE_SWAP32(src);
    #else
    *dst = src;
    #endif

    return sizeof(uint32_t);
}

/**************************************************************************************
* FunctionName   : unpack_be64()
* Description    : 64位大端格式解包
* EntryParameter : src,原始数据, *dst,目标数据指针
* ReturnValue    : 返回解包后数据长度
**************************************************************************************/
static inline int8_t unpack_be64(const uint64_t src, uint64_t *dst)
{
    #if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    *dst = src;
    #else
    *dst = BYTE_SWAP16(src);
    #endif

    return sizeof(uint64_t);
}

/**************************************************************************************
* FunctionName   : unpack_le64()
* Description    : 64位小端格式解包
* EntryParameter : src,原始数据, *dst,目标数据指针
* ReturnValue    : 返回解包后数据长度
**************************************************************************************/
static inline int8_t unpack_le64(const uint64_t src, uint64_t *dst)
{
    #if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    *dst = BYTE_SWAP16(src);
    #else
    *dst = src;
    #endif

    return sizeof(uint64_t);
}

/**************************************************************************************
* FunctionName   : unpack_simno()
* Description    : 终端手机号解包
* EntryParameter : src,原始数据, size,原始数据长度, *dst,加密后的数据
* ReturnValue    : 返回解包后数据长度
**************************************************************************************/
static inline int8_t unpack_simno(const uint8_t src[6], uint8_t dst[6])
{
    if(src != dst){
        memcpy(dst, src, 6);
    }

    return 6;
}

/**************************************************************************************
* FunctionName   : unpack_bin()
* Description    : 二进制解包
* EntryParameter : src,原始数据, size,原始数据长度, *dst,解包后的消息
* ReturnValue    : 返回解包后数据长度
**************************************************************************************/
static inline int16_t unpack_bin(const void *src, const int16_t size, void *dst)
{
    if(src != dst){
        memcpy((uint8_t *)dst, src, size);
    }

    return size;
}
/**************************************************************************************
* FunctionName   : unpack_str()
* Description    : 字符串解包
* EntryParameter : src,原始数据, *dst,解包后的消息
* ReturnValue    : 返回解包后数据长度
**************************************************************************************/
static inline int16_t unpack_str(const uint8_t *src, void *dst)
{
    if(src != dst){
        strcpy((char *)dst, (char *)src);
    }

    return strlen(dst) + 1;
}

/**************************************************************************************
* FunctionName   : unpack_bcd()
* Description    : BCD码解包
* EntryParameter : src,原始数据, size,原始数据长度, *dst,解包后的BCD码
* ReturnValue    : 返回解包后数据长度
**************************************************************************************/
static inline int16_t unpack_bcd(const uint8_t *src, const int16_t size, \
    uint8_t *dst)
{
    if(src != dst){
        memcpy((uint8_t *)dst, src, size);
    }

    return size;
}

/**************************************************************************************
* FunctionName   : unpack_time()
* Description    : 时间解包
* EntryParameter : src,原始数据, *dst,解包后的时间
* ReturnValue    : 返回解包后数据长度
**************************************************************************************/
static inline int8_t unpack_time(const uint8_t src[6], uint8_t dst[6])
{
    if(src != dst){
        memcpy((uint8_t *)dst, src, 6);
    }

    return 6;
}

/**************************************************************************************
* FunctionName   : unpack_bcc()
* Description    : 消息BCC校验码解包
* EntryParameter : src,原始数据, size,原始数据长度, *dst,目标数据指针
* ReturnValue    : 返回解包后数据长度
**************************************************************************************/
static inline int8_t unpack_bcc(const void *src, const int16_t size, uint8_t *dst)
{
    *dst = utils_calc_bcc(src, size);
    return sizeof(uint8_t);
}

/**************************************************************************************
* FunctionName   : unpack_lrc()
* Description    : 消息LRC校验码解包
* EntryParameter : src,原始数据, size,原始数据长度, *dst,目标数据指针
* ReturnValue    : 返回解包后数据长度
**************************************************************************************/
static inline int8_t unpack_lrc(const void *src, const int16_t size, uint8_t *dst)
{
    *dst = utils_calc_lrc(src, size);
    return sizeof(uint8_t);
}

/**************************************************************************************
* FunctionName   : unpack_rsa()
* Description    : RSA加密解包
* EntryParameter : data,原始数据(目标数据也放这里), size,原始数据长度
* ReturnValue    : 返回解密后的数据长度
**************************************************************************************/
int16_t unpack_rsa(void *data, const uint16_t size);

#ifdef __cplusplus
}
#endif

#endif /* __FRTOS_LIBPACK_H__ */

