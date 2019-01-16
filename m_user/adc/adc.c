/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : adc.c
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
#include "gpio_driver.h"
#include "frtos_sys.h"
#include "math.h"
#include "data.pb-c.h"
#include "adc_driver.h"

/**************************************************************************************
* Description    : 定义ADC数据最大长度
**************************************************************************************/
#define MAX_DATA_LEN                 100
#define MAX_SUBPROTO_LEN             50

#define MAIN_VOL_ID                  1

/**************************************************************************************
* Description    : 定义ADC采样参数
**************************************************************************************/
#if 1
static struct {
    uint32_t change;                            // 满足变化值，则上报, 单位v
    int32_t voltage;                            // 当前电压值
} adc_sample = {
    1000,                                      // 电压变化值超过默认1000mv的时候上报
    0                                          // 启动电压值
};

/**************************************************************************************
* FunctionName   : adc_config()
* Description    : ADC配置
* EntryParameter : data，指向发送的数据， len,指向发送数据长度
* ReturnValue    : 返回发送状态或者长度
**************************************************************************************/
static int32_t adc_config(uint8_t idx, void *data, int32_t len)
{
    uint32_t i = 0;
    Subid *msg = NULL;
    Voltage *adc = NULL;

    // 1.解开通用子协议数据头
    msg = subid__unpack(NULL,len, data);
    if(unlikely(msg == NULL)) return -EFAULT;

    for (i = 0; i < msg->n_subdata; i++) {
        adc = voltage__unpack(NULL, msg->subdata[i].len, msg->subdata[i].data);
        if(unlikely(adc == NULL)) break;

        if(msg->id == IOC__SET && adc->has_value && adc->id == MAIN_VOL_ID) {
            adc_sample.change = adc->value;
        } else if(msg->id == IOC__GET && adc->id == MAIN_VOL_ID) {
            adc_sample.voltage = 0; // 这里设置成0， 后面周期运行任务里面会自动上报一次电压
        }
        voltage__free_unpacked(adc, NULL);
    }
    // 2.释放内存
    subid__free_unpacked(msg, NULL);

    (void)idx;
    return len;
}

/**************************************************************************************
* FunctionName   : adc_run()
* Description    : adc周期任务
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t adc_run(void)
{
    Subid msg = SUBID__INIT;
    ProtobufCBinaryData proto;
    uint8_t buffer[MAX_DATA_LEN];
    uint8_t bytedata[MAX_SUBPROTO_LEN];
    struct piddata *pidata = (struct piddata *)buffer;

    Voltage vol = VOLTAGE__INIT;
    struct adc_samp_s adc = {1, VBAT_ADC, 271, 15, 0};//需要确定0值、最小刻度值！！

    // 1.读取需要上报的信息
    if(fdrive_read(DRIVER_ADC, &adc, sizeof(struct adc_samp_s)) < 0) return -EFAULT;

    // 2.不满足条件不上报
    if(adc_sample.change > fabs(adc.result - adc_sample.voltage)) return 0;

    vol.value = adc.result;
    vol.has_value = 1;
    vol.id = MAIN_VOL_ID;
    adc_sample.voltage = adc.result;

    // 3.填充应答protobuf 子ID信息
    msg.id = IOC__DATA;
    msg.n_subdata = 1;
    msg.subdata = &proto;
    proto.len = voltage__get_packed_size(&vol);
    proto.data = bytedata;
    voltage__pack(&vol, bytedata);

    // 4.准备发送protobuf打包
    pidata->id = ADC_PID;
    pidata->len = subid__get_packed_size(&msg);
    subid__pack(&msg, pidata->data);
    fuser_data_set(INIT_PID, pidata, pidata->len + sizeof(struct piddata));

    return 0;
}

/**************************************************************************************
* Description    : 定义ADC任务结构
**************************************************************************************/
static __const struct applite adc = {
    .idx   = ADC_PID,
    .name  = "adc",
    .run   = adc_run,
    .set   = adc_config,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
//APP_REGISTER(adc);
#endif
