/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : transport.c
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#include "frtos_app.h"
#include "frtos_mem.h"
#include "frtos_errno.h"
#include "frtos_lock.h"
#include "config_user.h"
#include "config_driver.h"
#include "frtos_ioctl.h"
#include "frtos_libpack.h"
#include "frtos_list.h"
#include "frtos_sys.h"
#include "frtos_lock.h"
#include "frtos_tasklet.h"
#include "transport.h"
#include "loopbuf.h"
#include "debug.h"

/**************************************************************************************
* Description    : 模块内部数据定义
**************************************************************************************/
#define TRANS_UART                       DRIVER_UART3 		//设置发送串口
#define TRANS_RXBUF_LEN					 1024				//设置接收缓冲区长度
#define TRANS_RXDATA_LEN 				 256 				//设置一次接收数据的长度

#define TRANS_DATA_ERROR				 -1					//接收数据有错
#define TRANS_DATA_NOT_ENOUGH			 -2					//接收数据不全

static struct loop_buffer *g_trans_buf = NULL; 				//接收数据环形缓冲区
static mutex_lock_t g_mutex; 								//接收数据锁
static void trans_data_recv(void *args); 

#if 0
/**************************************************************************************
* FunctionName   : trans_data_check()
* Description    : 检查是否接收完数据
* EntryParameter : data，指向接收的数据， start,实际数据的起始位置, len,送数据长度
* ReturnValue    : 返回校验码
**************************************************************************************/
static int32_t trans_data_check(uint8_t *data, int32_t *start, int32_t *len)
{
	int32_t start_len = 0;
	int32_t pos = 0;
    uint32_t magic = 0;
	struct transport *tdata = NULL;

	start_len = loop_buffer_use(g_trans_buf);
	while (likely((start_len - pos) > (int32_t)sizeof(struct transport))) {
        tdata = (struct transport *)((uint8_t *)data + pos);
        unpack_be32(tdata->magic, &magic);
        // 1.1 判断是否为头部
        if(likely(magic != TRANS_MAGIC)) { pos++;continue;}

		*start = pos;
        unpack_be32(tdata->length, &tdata->length);
		if (start_len - pos < tdata->length + sizeof(struct transport))
			return TRANS_DATA_NOT_ENOUGH;

		(*len) = tdata->length + sizeof(struct transport);

		return 0;
    }

	return TRANS_DATA_ERROR;
}

/**************************************************************************************
* FunctionName   : trans_recv()
* Description    : MCU接收到MPU的数据
* EntryParameter : data，指向MPU发送的数据， len,指向MPU发送数据长度
* ReturnValue    : 返回None
**************************************************************************************/
int32_t trans_recv(int32_t idx, void *data, int32_t len)
{
    int32_t pos = 0;

	//debug("trans_recv\r\n");

    if(unlikely(data == NULL)) return -EINVAL;

	/* 1. 把数据存入环形缓冲区 */
	mutex_lock(g_mutex);
	pos = loop_buffer_put(g_trans_buf, data, len);
	mutex_unlock(g_mutex);

	(void)idx;
	return len;
#if 0
    // 1.遍历整个数据区，寻找数据头部
    while (likely((len - pos) > (int32_t)sizeof(struct transport))) {
        tdata = (struct transport *)((uint8_t *)data + pos);
        unpack_be32(tdata->magic, &magic);

        // 1.1 判断是否为头部
        if(likely(magic != TRANS_MAGIC)) { pos++;continue;}

        // 头部数据，解析数据长度和ID
        unpack_be8(tdata->id, &tdata->id);
        unpack_be32(tdata->length, &tdata->length);

        // 检测数据长度有效性, 长度异常, 跳出循环
        if(unlikely(tdata->length > (len - pos - sizeof(struct transport)))) {
            break;
        }
        // 数据校验可靠性检测， 错误就重新尝试
        if(tdata->csum == chksum_xor((uint8_t *)tdata->data, tdata->length)) {
            fuser_data_set(tdata->id, tdata->data, tdata->length);
            pos = pos + tdata->length;         //去掉数据部分长度
        }

        // 数据去掉头部数据
        pos += sizeof(struct transport);
    }

    (void)idx;
    return len;
#endif
}

/**************************************************************************************
* FunctionName   : trans_send()
* Description    : MCU发送数据到MPU
* EntryParameter : data，指向发送的数据， len,指向发送数据长度
* ReturnValue    : 返回发送状态或者长度
**************************************************************************************/
static int32_t trans_send(uint8_t idx, void *data, int32_t len)
{
    int32_t tdata_len = 0;
    struct transport *tdata = NULL;
    struct piddata *pdata = (struct piddata *)data;

    if(pdata->len + sizeof(struct piddata) != (uint32_t)len || data == NULL) {
        return -EINVAL;
    }
    tdata_len = sizeof(struct transport) + pdata->len;

    tdata = mem_malloc(tdata_len);
    if(unlikely(tdata == NULL)) return -ENOMEM;

    pack_be8(pdata->id, &tdata->id);
    pack_be32(pdata->len, &tdata->length);
    pack_be32(TRANS_MAGIC, &tdata->magic);

    memcpy(tdata->data, pdata->data, pdata->len);
    tdata->csum = chksum_xor((uint8_t *)tdata->data, pdata->len);

    if (fdrive_write(TRANS_UART, tdata, tdata_len) <= 0) {
		mem_free(tdata);
        return -EREMOTEIO;
    }
    mem_free(tdata);

    (void)idx;
    return len;
}

static void trans_data_recv(void *args)
{
	int32_t pos = 0, ret = 0;
	int32_t recv_len = 0;
	int32_t real_len = 0;
	uint8_t *tmp_buf = NULL;
	struct transport *trans_data = NULL;


	mutex_lock(g_mutex);

	/* 1. 判断当前数据长度 */
	recv_len = loop_buffer_use(g_trans_buf);
	if (recv_len <= sizeof(struct transport)) {
		mutex_unlock(g_mutex);
		goto next;
	}

	/* 2. 获取接收数据 */
	recv_len = recv_len > TRANS_RXDATA_LEN ? TRANS_RXDATA_LEN : recv_len;
	tmp_buf = mem_malloc(recv_len);
	if (!tmp_buf) {
		debug("tmp_buf mamllco error: %d\r\n", recv_len);
		mutex_unlock(g_mutex);
		goto next;
	}
	loop_buffer_pick(g_trans_buf, (uint8_t*)tmp_buf, recv_len);
	/* 3. 判断当前环形缓冲区长度是否接收满数据 */
	ret = trans_data_check((uint8_t*)tmp_buf, &pos, &real_len);
	if (ret == TRANS_DATA_NOT_ENOUGH)  {
		/* 数据接收不全 */
		if (pos > 0)
			loop_buffer_get(g_trans_buf, tmp_buf, pos);
		mutex_unlock(g_mutex);
		goto next;
	} else if (ret == TRANS_DATA_ERROR) {
		/* 数据出错 */
		loop_buffer_get(g_trans_buf, tmp_buf, recv_len);
		mutex_unlock(g_mutex);
		goto next;
	} 
	if (pos > 0) 
		loop_buffer_get(g_trans_buf, tmp_buf, pos);
	mem_free(tmp_buf);
	tmp_buf = NULL;

	/* 4. 获取实际数据 */
	trans_data = (struct transport *)mem_malloc(real_len);
	if (!trans_data) {
		mutex_unlock(g_mutex);
		debug("trans_data, mem_malloc error:%d, %d\r\n", real_len, xPortGetFreeHeapSize());
		goto  next;
	}
	memset(trans_data, 0, real_len);	
	loop_buffer_get(g_trans_buf, (uint8_t*)trans_data, real_len);
	mutex_unlock(g_mutex);
	
	/* 5. 获取id和长度 */
	unpack_be32(trans_data->length, &trans_data->length);
	unpack_be8(trans_data->id, &trans_data->id);
	
	/* 6. 校验和判断 */
	if(trans_data->csum == chksum_xor((uint8_t *)trans_data->data, trans_data->length)) 
	    fuser_data_set(trans_data->id, trans_data->data, trans_data->length);
	else
		debug("check sum error\r\n");
next:
	if (trans_data)
		mem_free(trans_data);
	if (tmp_buf) 
		mem_free(tmp_buf);
	tmp_buf = NULL;

	/* 7. 重新开启定时器调度 */
	tasklet_schedule(trans_data_recv, NULL, 1);	
}

/**************************************************************************************
* FunctionName   : trans_init()
* Description    : MCU通信接口初始化
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t trans_init(void)
{
	g_trans_buf = init_loop_buffer(TRANS_RXBUF_LEN);
	if (NULL == g_trans_buf)
		return -EMEM;

	g_mutex = mutex_lock_init();
	if (!g_mutex) {
		destroy_loop_buffer(g_trans_buf);
		return -EMEM;
	}

	tasklet_schedule(trans_data_recv, NULL, 1);	
    return fdrive_ioctl(TRANS_UART, _IOC_SET_CB, trans_recv, sizeof(trans_recv));
}

/**************************************************************************************
* Description    : 定义通信任务结构
**************************************************************************************/
static __const struct applite app_transport = {
    .idx   = INIT_PID,
    .name  = "init",
    .init  = trans_init,
    .set   = trans_send,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
//APP_REGISTER(app_transport);
#endif
