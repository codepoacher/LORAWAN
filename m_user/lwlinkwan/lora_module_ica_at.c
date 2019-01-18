/*
 * Copyright (c) 2018 Alibaba Group. All rights reserved.
 * License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <string.h>
#include <stdlib.h>
#include "utilities.h"
#include "lora_module_ica_at.h"
#include "lora_module_export.h"

#define ATCMD_SIZE   160
#define ATQUERY_SIZE 128

#define AT_ERRNO_NOSUPP   (1)
#define AT_ERRNO_NOALLOW  (2)
#define AT_ERRNO_PARA_VAL (5)
#define AT_ERRNO_PARA_NUM (6)
#define AT_ERRNO_SYS      (8)
#define AT_CB_PRINT       (0xFF)

static char atcmd[ATCMD_SIZE];
static uint16_t atcmd_index = 0;
static char g_at_data_buf[LORAWAN_APP_DATA_MAX_SIZE];
static char g_at_query_buf[ATQUERY_SIZE];

typedef struct atcmd_s {
    const char *cmd_name;
    const char *test_cmd_str;
    int (*query_cmd)(void);
    int (*exec_cmd)(char *str);
    int (*exec_cmd_no_para)(void);
} atcmd_t;

static int hex2bin(const char *hex, uint8_t *bin, uint16_t bin_length)
{
    uint16_t hex_length = strlen(hex);
    const char *hex_end    = hex + hex_length;
    uint8_t *cur        = bin;
    uint8_t num_chars  = hex_length & 1;
    uint8_t byte       = 0;

    if (hex_length % 2 != 0) {
        return -1;
    }

    if (hex_length / 2 > bin_length) {
        return -1;
    }

    while (hex < hex_end) {
        if ('A' <= *hex && *hex <= 'F') {
            byte |= 10 + (*hex - 'A');
        } else if ('a' <= *hex && *hex <= 'f') {
            byte |= 10 + (*hex - 'a');
        } else if ('0' <= *hex && *hex <= '9') {
            byte |= *hex - '0';
        } else {
            return -1;
        }
        hex++;
        num_chars++;

        if (num_chars >= 2) {
            num_chars = 0;
            *cur++    = byte;
            byte      = 0;
        } else {
            byte <<= 4;
        }
    }
    return cur - bin;
}

char *my_strtok(char *str,char *dl)  //TODO::使用strtok(NULL,",")会导致死机，原因不明
{
	static char *tmp;
	char *ret;
	
	if(str == NULL)
	{
		if(*tmp == '\0') 
			return  NULL;
		str = tmp;
		ret = tmp;
	} else {
		ret = str; 
	}
	while(*str != '\0')
	{
		if (*str == *dl) 
		{
			*str = '\0';
			str++;
			tmp = str;
			break;
		}
		str++;
	}
	return ret;
}
static int lora_tx_data_payload(char *payload, uint8_t len)
{
    return lorawan_module_send(lorawan_module_get_tx_cfm_flag(), lorawan_module_get_app_port(), payload, len, NULL);
}

static int at_query_cjoinmode(void)
{
    int mode;
    mode = lorawan_module_get_join_mode();

    snprintf(g_at_query_buf, ATQUERY_SIZE, "%d", mode);
    return 0;
}

static int at_query_cdeveui(void)
{
    uint8_t eui[8];
    lorawan_module_get_dev_eui(eui);
    snprintf(g_at_query_buf, ATQUERY_SIZE,
             "%02x%02x%02x%02x%02x%02x%02x%02x",
             eui[0], eui[1], eui[2], eui[3], eui[4],
             eui[5], eui[6], eui[7]);
    return 0;
}

static int at_query_cappeui(void)
{
    uint8_t eui[8];
    lorawan_module_get_app_eui(eui);
    snprintf(g_at_query_buf, ATQUERY_SIZE,
             "%02x%02x%02x%02x%02x%02x%02x%02x",
             eui[0], eui[1], eui[2], eui[3], eui[4],
             eui[5], eui[6], eui[7]);
    return 0;
}

static int at_query_cappkey(void)
{
    uint8_t i;
    uint8_t len = 0;
    uint8_t key[16];
    lorawan_module_get_app_key(key);

    for (i = 0; i < 16; i++) {
        len += snprintf(g_at_query_buf + len, ATQUERY_SIZE, "%02x", key[i]);
    }
    return 0;
}

static int at_query_cdevaddr(void)
{
    uint8_t *devaddr = lorawan_module_get_devaddr();
    snprintf(g_at_query_buf, ATQUERY_SIZE, "%02x%02x%02x%02x",
             devaddr[0], devaddr[1],
             devaddr[2], devaddr[3]);
    return 0;
}

static int at_query_cappskey(void)
{
    uint8_t i;
    uint8_t len = 0;
    uint8_t *appskey = lorawan_module_get_appskey();

    for (i = 0; i < 16; i++) {
        len += snprintf(g_at_query_buf + len, ATQUERY_SIZE, "%02x", appskey[i]);
    }

    return 0;
}

static int at_query_cnwkskey(void)
{
    uint8_t *nwkskey = lorawan_module_get_nwkskey();
    uint8_t i;
    uint8_t len = 0;

    for (i = 0; i < 16; i++) {
        len += snprintf(atcmd + len, ATQUERY_SIZE, "%02x", nwkskey[i]);
    }

    return 0;
}

static int at_query_caddmulticast(void)
{
    mcChannel_t *mCastInfo = lorawan_module_get_cur_multicast();
    mcKey_t *mKeyInfo = lorawan_module_get_cur_mc_key();
    int i;
    int hit = 0;
    char c;

    if (mCastInfo == NULL) {
        return AT_ERRNO_SYS;
    }

    for (i = 0; i < 4; i++) {

        if (mCastInfo[i].Address != 0) {

            if (!hit) {
                LORA_AT_PRINTF("\r\n%s", LORA_AT_CADDMULTICAST);
                c = ':';
            } else {
                c = ',';
            }

            LORA_AT_PRINTF(
                "%c%08lx,%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x,%lx,%x,%x",
                c, mCastInfo[i].Address, mKeyInfo[i].val[0],
                mKeyInfo[i].val[1], mKeyInfo[i].val[2],
                mKeyInfo[i].val[3], mKeyInfo[i].val[4],
                mKeyInfo[i].val[5], mKeyInfo[i].val[6],
                mKeyInfo[i].val[7], mKeyInfo[i].val[8],
                mKeyInfo[i].val[9], mKeyInfo[i].val[10],
                mKeyInfo[i].val[11], mKeyInfo[i].val[12],
                mKeyInfo[i].val[13], mKeyInfo[i].val[14],
                mKeyInfo[i].val[15],
                mCastInfo[i].Frequency, mCastInfo[i].Datarate, mCastInfo[i].Periodicity);
            hit = 1;
        }
    }

    if (!hit) {
        LORA_AT_PRINTF("\r\n%s:NO INFO\r\n", LORA_AT_CADDMULTICAST);
    } else {
        LORA_AT_PRINTF("\r\nOK\r\n");
    }

    return AT_CB_PRINT;
}

static int at_query_cdelmulticast(void)
{
    mcChannel_t *mCastInfo = lorawan_module_get_cur_multicast();
    int i;
    int hit = 0;
    char c;

    if (mCastInfo == NULL) {
        return AT_ERRNO_SYS;
    }

    for (i = 0; i < 4; i++) {

        if (mCastInfo[i].Address != 0) {

            if (!hit) {
                LORA_AT_PRINTF("\r\n%s", LORA_AT_CDELMULTICAST);
                c = ':';
            } else {
                c = ',';
            }

            LORA_AT_PRINTF("%c%08lx", c, mCastInfo[i].Address);
            hit = 1;
        }
    }

    if (!hit) {
        LORA_AT_PRINTF("\r\n%s:NO INFO\r\n", LORA_AT_CDELMULTICAST);
    } else {
        LORA_AT_PRINTF("\r\nOK\r\n");
    }

    return AT_CB_PRINT;
}

static int at_query_cnummulticast(void)
{
    uint8_t multiNum = lorawan_module_get_mulitcast_num();
    snprintf(g_at_query_buf, ATQUERY_SIZE, "%d", multiNum);
    return 0;
}

static int at_query_cfreqbandmask(void)
{
    snprintf(g_at_query_buf, ATQUERY_SIZE, "%04x",
             lorawan_module_get_freqband_mask());
    return 0;
}

static int at_query_culdlmode(void)
{
    snprintf(g_at_query_buf, ATQUERY_SIZE, "%d",
             lorawan_module_get_freq_mode());
    return 0;
}

static int at_query_cworkmode(void)
{
    int mode;
    mode = lorawan_module_get_work_mode();
    snprintf(g_at_query_buf, ATQUERY_SIZE, "%d", mode);
    return 0;
}

static int at_query_cclass(void)
{
    int8_t class;
    class = lorawan_module_get_class();
    if (class == CLASS_B) {
        snprintf(g_at_query_buf, ATQUERY_SIZE, "%d,0,%d",
                 class, lorawan_module_get_pingslot_period());
    } else {
        snprintf(g_at_query_buf, ATQUERY_SIZE, "%d", class);
    }
    return 0;
}

static int at_query_cstatus(void)
{
    LoRaModuleStatus_t status;
    status = lorawan_module_get_status();
    snprintf(g_at_query_buf, ATQUERY_SIZE, "%d", status);
    return 0;
}

static int at_query_cjoin(void)
{
    uint8_t bJoin;
    int8_t autoJoin;
    bJoin = lorawan_module_get_join_status();
    autoJoin = lorawan_module_get_auto_join();
    snprintf(g_at_query_buf, ATQUERY_SIZE, "%d,%d", bJoin, autoJoin);
    return 0;
}

static int at_query_drx(void)
{
    uint8_t *rx_data;
    int len;

    lorawan_module_get_rx_data(&rx_data, &len);
    LORA_AT_PRINTF("\r\n%s:%d", LORA_AT_DRX, len);

    if (len > 0) {
        LORA_AT_PRINTF(",");
        char str[3] = {'\0'};
        for (int i = 0; i < len; i++) {
            snprintf(str, sizeof(str), "%02x", rx_data[i]);
            LORA_AT_PRINTF(str);
        }
    }

    LORA_AT_PRINTF("\r\nOK\r\n");

    return AT_CB_PRINT;
}

static int at_query_cconfirm(void)
{
    snprintf(g_at_query_buf, ATQUERY_SIZE, "%d", lorawan_module_get_tx_cfm_flag());
    return 0;
}

static int at_query_cappport(void)
{
    snprintf(g_at_query_buf, ATQUERY_SIZE, "%d", lorawan_module_get_app_port());
    return 0;
}

static int at_query_cdatarate(void)
{
    snprintf(g_at_query_buf, ATQUERY_SIZE, "%d", lorawan_module_get_tx_datarate());
    return 0;
}

static int at_query_crssi(void)
{
    int i;
    int len = 0;
    int16_t channel_rssi[8];
    int8_t channel_snr[8];

    lorawan_module_get_channel_rssi_snr(channel_rssi, channel_snr);

    for (i = 0; i < 8; i++) {
        len += snprintf(g_at_query_buf + len, ATQUERY_SIZE, "\r\n%d:%d", i, channel_rssi[i]);
    }

    for (i = 0; i < 8; i++) {
        len += snprintf(g_at_query_buf + len, ATQUERY_SIZE, "\r\n%d:%d", i, channel_snr[i]);
    }

    return 0;
}

static int at_query_cnbtrials(void)
{
    int8_t confirm_val;
    int8_t unconfirm_val;
    unconfirm_val = lorawan_module_get_tx_cfm_trials(0);
    confirm_val = lorawan_module_get_tx_cfm_trials(1);
    snprintf(g_at_query_buf, ATQUERY_SIZE, "0,%d,1,%d",
             unconfirm_val, confirm_val);

    return 0;
}

static int at_query_crm(void)
{
    int8_t reportMode;
    uint32_t reportInterval;
    reportMode     = lorawan_module_get_report_mode();
    reportInterval = lorawan_module_get_duty_cycle() / 1000;
    snprintf(g_at_query_buf, ATQUERY_SIZE, "%d,%ld",
             reportMode, reportInterval);

    return 0;
}

static int at_query_ctxp(void)
{
    snprintf(g_at_query_buf, ATQUERY_SIZE, "%d", lorawan_module_get_tx_power());
    return 0;
}

static int at_query_cadr(void)
{
    snprintf(g_at_query_buf, ATQUERY_SIZE, "%d", lorawan_module_get_adr());
    return 0;
}

static int at_query_crxp(void)
{
    uint8_t RX1DRoffset;
    uint8_t RX2DataRate;
    uint32_t RX2Frequency;
    lorawan_module_get_rx_window_params(&RX1DRoffset, &RX2DataRate,
                                        &RX2Frequency);
    snprintf(g_at_query_buf, ATQUERY_SIZE, "%d,%d",
             RX1DRoffset, RX2DataRate);

    return 0;
}

static int at_query_crx1delay(void)
{
    uint32_t rx1delay;
    rx1delay = lorawan_module_get_mac_rx1_delay() / 1000;
    snprintf(g_at_query_buf, ATQUERY_SIZE, "%ld", rx1delay);

    return 0;
}

static int at_query_clbt(void)
{
    snprintf(g_at_query_buf, ATQUERY_SIZE, "%d", lorawan_module_get_lbt());
    return 0;
}

static int at_query_cproprietary(void)
{
    snprintf(g_at_query_buf, ATQUERY_SIZE, "%d", lorawan_module_get_proprietary());
    return 0;
}

static int at_query_cmonitornode(void)
{
    snprintf(g_at_query_buf, ATQUERY_SIZE, "%d", lorawan_module_get_monitor_node());
    return 0;
}

static int at_query_crepeaterfreq(void)
{
    uint32_t freq = 0;

    lorawan_module_get_repeater_freq(&freq);
    snprintf(g_at_query_buf, ATQUERY_SIZE, "%ld", freq);
    return 0;
}

static int at_exec_ireboot(char *str)
{
    int8_t mode = strtol(str, NULL, 0);

    if (mode != 0 && mode != 1) {
        return AT_ERRNO_PARA_VAL;
    }

    lorawan_module_reboot(mode);

    return 0;
}

static int at_exec_cjoinmode(char *str)
{
    int ret;
    int mode = strtol(str, NULL, 0);

    if (mode != 0 && mode != 1) {
        return AT_ERRNO_PARA_VAL;
    }

    ret = lorawan_module_set_join_mode(mode);
    return ret;
}

static int at_exec_cdeveui(char *str)
{
    int ret;
    uint8_t len;
    uint8_t buf[8];

    len = hex2bin(str, buf, 8);

    if (len != 8) {
        return AT_ERRNO_PARA_VAL;
    }

    ret = lorawan_module_set_dev_eui(buf);
    return ret;
}

static int at_exec_cappeui(char *str)
{
    int ret;
    uint8_t len;
    uint8_t buf[8];

    len = hex2bin(str, buf, 8);
    if (len != 8) {
        return AT_ERRNO_PARA_VAL;
    }

    ret = lorawan_module_set_app_eui(buf);
    return ret;
}

static int at_exec_cappkey(char *str)
{
    int ret;
    uint8_t buf[16];
    uint8_t len;

    len = hex2bin(str, buf, 16);
    if (len != 16) {
        return AT_ERRNO_PARA_VAL;
    }

    ret = lorawan_module_set_app_key(buf);
    return ret;
}

static int at_exec_cdevaddr(char *str)
{
    int ret;
    uint8_t len;
    uint8_t buf[4];

    len = hex2bin(str, buf, 4);
    if (len != 4) {
        return AT_ERRNO_PARA_VAL;
    }

    ret = lorawan_module_set_devaddr(buf);
    return ret;
}

static int at_exec_cappskey(char *str)
{
    int ret;
    uint8_t len;
    uint8_t buf[16];

    len = hex2bin(str, buf, 16);
    if (len != 16) {
        return AT_ERRNO_PARA_VAL;
    }

    ret = lorawan_module_set_appskey(buf);
    return ret;
}

static int at_exec_cnwkskey(char *str)
{
    int ret;
    uint8_t len;
    uint8_t buf[16];

    len = hex2bin(str, buf, 16);
    if (len != 16) {
        return AT_ERRNO_PARA_VAL;
    }

    ret = lorawan_module_set_nwkskey(buf);
    return ret;
}

static int at_exec_caddmulticast(char *str)
{
    uint32_t dev_addr;
    uint8_t mc_key[16];
    uint32_t frequency;
    int8_t dr;
    uint16_t period;
    int ret;
    char *param;

    param = my_strtok(str, ",");
    hex2bin(param, (uint8_t *)&dev_addr, 4);
    dev_addr =
        ((dev_addr << 24) & (0xFF000000)) |
        ((dev_addr << 8) & (0x00FF0000)) |
        ((dev_addr >> 8) & (0x0000FF00)) |
        ((dev_addr >> 24) & (0x000000FF));

    param = my_strtok(NULL, ",");
    if (param == NULL) {
        return AT_ERRNO_PARA_NUM;
    }

    hex2bin(param, mc_key, 16);

    param = my_strtok(NULL, ",");
    if (param == NULL) {
        return AT_ERRNO_PARA_NUM;
    }
    frequency = strtol(param, NULL, 0);

    param = my_strtok(NULL, ",");
    if (param == NULL) {
        return AT_ERRNO_PARA_NUM;
    }
    dr = strtol(param, NULL, 0);

    param = my_strtok(NULL, ",");
    if (param == NULL) {
        return AT_ERRNO_PARA_NUM;
    }

    period = strtol(param, NULL, 0);

    ret = lorawan_module_set_multicast(dev_addr, mc_key, frequency, dr, period);

    return ret;
}

static int at_exec_cdelmulticast(char *str)
{
    int ret;
    uint32_t devAddr;

    hex2bin(str, (uint8_t *)&devAddr, 4);
    devAddr =
        ((devAddr << 24) & (0xFF000000)) | ((devAddr << 8) & (0x00FF0000)) |
        ((devAddr >> 8) & (0x0000FF00)) | ((devAddr >> 24) & (0x000000FF));

    ret = lorawan_module_del_multicast(devAddr);

    return ret;
}

static int at_exec_cfreqbandmask(char *str)
{
    int ret;
    uint8_t mask[2];
    uint8_t len;

    len = hex2bin(str, (uint8_t *)mask, 2);
    if (len != 2) {
        return AT_ERRNO_PARA_VAL;
    }

    ret = lorawan_module_set_freqband_mask(mask[1] | (mask[0] << 8));
    return ret;
}

static int at_exec_culdlmode(char *str)
{
    int ret;
    int mode;

    mode = strtol(str, NULL, 0);
    ret  = lorawan_module_set_freq_mode(mode);

    return ret;
}

static int at_exec_cworkmode(char *str)
{
    int mode;
    int ret;

    mode = strtol(str, NULL, 0);
    if (mode != 1 && mode != 2 && mode != 3) {
        return AT_ERRNO_PARA_VAL;
    }

    ret = lorawan_module_set_work_mode(mode);
    return ret;
}

static int at_exec_cclass(char *str)
{
    int ret;
    int branch;
    int period;
    int8_t class;
    char *param;

    param = my_strtok(str, ",");
    class = strtol(param, NULL, 0);

    /* only branch 0 support */
    if (class == CLASS_B) {
        param = my_strtok(NULL, ",");
        if (param != NULL) {
            branch = strtol(param, NULL, 0);

            if (branch != 0) {
                return AT_ERRNO_PARA_NUM;
            }

            param = my_strtok(NULL, ",");
            if (param == NULL) {
                return AT_ERRNO_PARA_NUM;
            }

            period = strtol(param, NULL, 0);
            ret = lorawan_module_set_pingslot_period(period);
            if (ret != 0) {
                return AT_ERRNO_PARA_NUM;
            }
        }
    }

    ret = lorawan_module_set_class(class);

    return ret;
}

static int at_exec_cjoin(char *str)
{
    uint8_t bJoin;
    int8_t autoJoin;
    int ret;
    char *param;

    param = my_strtok(str, ",");
	/* check start or stop join parameter */
    bJoin = strtol(param, NULL, 0);
    //bJoin = 1;
    if (bJoin != 1 && bJoin != 0) {
        return AT_ERRNO_PARA_VAL;
    }
	
	/* check auto join parameter */
	param = my_strtok(NULL, ",");

	if (param != NULL) {
		autoJoin = strtol(param, NULL, 0);
		if (autoJoin != 1 && autoJoin != 0) {
			return AT_ERRNO_PARA_VAL;
		}

		ret = lorawan_module_set_auto_join(autoJoin);
		if (ret != 0) {
            return ret;
        }
    }
    ret = lorawan_module_join(bJoin);

    return ret;
}

static int at_exec_dtx(char *str)
{
    uint8_t str_len;
    uint8_t bin_len;
    uint8_t ret_len;
    int ret;
    char *param;

    param = my_strtok(str, ",");
    str_len = strtol(param, NULL, 0);
    param = my_strtok(NULL, ",");
    if (param == NULL) {
        return AT_ERRNO_PARA_NUM;
    }

    if (str_len % 2 != 0) {
        return AT_ERRNO_PARA_VAL;
    }

    bin_len = str_len / 2;

    ret_len = hex2bin(param, (uint8_t *)g_at_data_buf, bin_len);
    if (ret_len != bin_len) {
        return AT_ERRNO_PARA_VAL;
    }

    ret = lora_tx_data_payload(g_at_data_buf, bin_len);
    if (ret == 0) {
        int cfm;
        cfm = lorawan_module_get_tx_cfm_flag();
        if (cfm == 0) {
            LORA_AT_PRINTF("\r\nOK+SEND:%d\r\n", bin_len);
        } else {
            LORA_AT_PRINTF("\r\nOK+SEND:%d\r\n", bin_len);
        }
    } else {
        LORA_AT_PRINTF("\r\nERR+SEND:%d\r\n", ret - 1);
    }

    return AT_CB_PRINT;
}

static int at_exec_cconfirm(char *str)
{
    int cfm;
    int ret;

    cfm = strtol(str, NULL, 0);
    if (cfm != 0 && cfm != 1) {
        return AT_ERRNO_PARA_VAL;
    }

    ret = lorawan_module_set_tx_cfm_flag(cfm);

    return ret;
}

static int at_exec_cappport(char *str)
{
    int ret;
    uint8_t port;

    port = (uint8_t)strtol(str,  NULL, 0);
    ret  = lorawan_module_set_app_port(port);

    return ret;
}

static int at_exec_cdatarate(char *str)
{
    int ret;
    int8_t datarate;

    datarate = strtol(str, NULL, 0);
    ret      = lorawan_module_set_tx_datarate(datarate);
    return ret;
}

static int at_exec_cnbtrials(char *str)
{
    int ret;
    int8_t m_type;
    int8_t value;
    char *param;

    param = my_strtok(str, ",");
    m_type = strtol(param, NULL, 0);
    param = my_strtok(NULL, ",");
    if (param == NULL) {
        return AT_ERRNO_PARA_NUM;
    }

    value  = strtol(param, NULL, 0);
    if ((m_type == 0 || m_type == 1) && (value >= 1 && value <= 15)) {
        ret = lorawan_module_set_tx_cfm_trials(m_type, value);
    } else {
        ret = AT_ERRNO_PARA_VAL;
    }

    return ret;
}

static int at_exec_crm(char *str)
{
    int8_t reportMode;
    uint32_t reportInterval;
    char *param;

    param = my_strtok(str, ",");
    reportMode = strtol(param, NULL, 0);

    param = my_strtok(NULL, ",");
    if (param != NULL) {
        reportInterval = strtol(param, NULL, 0);
    } else if (param == NULL && reportMode == 1) {
        return AT_ERRNO_PARA_NUM;
    }

    if (reportMode == 0) {
        lorawan_module_set_report_mode(reportMode);
    } else if (reportMode == 1 && reportInterval >= 1) {
        lorawan_module_set_duty_cycle(reportInterval * 1000);
        lorawan_module_set_report_mode(reportMode);
    } else {
        return AT_ERRNO_PARA_VAL;
    }

    return 0;
}

static int at_exec_ctxp(char *str)
{
    int ret;
    int8_t txpwr_idx;

    txpwr_idx = strtol(str, NULL, 0);
    ret = lorawan_module_set_tx_power(txpwr_idx);

    return ret;
}

static int at_exec_clinkcheck(char *str)
{
    uint8_t checkValue;
    int ret;

    checkValue = strtol(str, NULL, 0);
    if (checkValue <= 2) {
        ret = lorawan_module_send_link_check(checkValue);
    } else {
        ret = AT_ERRNO_PARA_VAL;
    }

    return ret;
}

static int at_exec_cadr(char *str)
{
    int adr;
    int ret;

    adr = strtol(str, NULL, 0);
    if (adr != 0 && adr != 1) {
        return AT_ERRNO_PARA_VAL;
    }

    ret = lorawan_module_set_adr(adr);
    return ret;
}

static int at_exec_crxp(char *str)
{
    uint8_t RX1DRoffset;
    uint8_t RX2DataRate;
    char *param;
    int ret;

    param = my_strtok(str, ",");
    RX1DRoffset  = strtol(param, NULL, 0);
    param = my_strtok(NULL, ",");
    if (param == NULL) {
        return AT_ERRNO_PARA_NUM;
    }

    RX2DataRate  = strtol(param, NULL, 0);
    ret = lorawan_module_set_rx_window_params(RX1DRoffset, RX2DataRate);

    return ret;
}

static int at_exec_crx1delay(char *str)
{
    int ret;
    uint32_t rx1delay;

    rx1delay = strtol(str, NULL, 0);
    ret = lorawan_module_set_mac_rx1_delay(rx1delay * 1000);

    return ret;
}

static int at_exec_csave(void)
{
    return lorawan_module_save();
}

static int at_exec_crestore(void)
{
    return lorawan_module_restore();
}

static int at_exec_clbt(char *str)
{
    int ret;
    int mode;

    mode = strtol(str, NULL, 0);
    if (mode == 0 || mode == 1) {
        ret = lorawan_module_set_lbt(mode);
    } else {
        ret = AT_ERRNO_PARA_VAL;
    }

    return ret;
}

static int at_exec_cproprietary(char *str)
{
    int ret;
    int mode;

    mode = strtol(str, NULL, 0);
    if (mode == 0 || mode == 1) {
        ret = lorawan_module_set_proprietary(mode);
    } else {
        ret = AT_ERRNO_PARA_VAL;
    }

    return ret;
}

static int at_exec_cmonitornode(char *str)
{
    int ret;
    int mode;

    mode = strtol(str, NULL, 0);
    if (mode == 0 || mode == 1) {
        ret = lorawan_module_set_monitor_node(mode);
    } else {
        ret = AT_ERRNO_PARA_VAL;
    }

    return ret;
}

static int at_exec_crepeaterfreq(char *str)
{
    int ret;
    uint32_t freq;

    freq = strtol(str, NULL, 0);
    ret = lorawan_module_set_repeater_freq(freq);

    return ret;
}

static atcmd_t g_at_cmd_list[] = {
    { LORA_AT_IREBOOT, "OK", NULL, at_exec_ireboot, NULL },
    { LORA_AT_CJOINMODE, "mode", at_query_cjoinmode, at_exec_cjoinmode, NULL },
    { LORA_AT_CDEVEUI, "DevEUI:length is 16", at_query_cdeveui, at_exec_cdeveui, NULL },
    { LORA_AT_CAPPEUI, "AppEUI:length is 16", at_query_cappeui, at_exec_cappeui, NULL },
    { LORA_AT_CAPPKEY, "AppKey:length is 32", at_query_cappkey, at_exec_cappkey, NULL },
    { LORA_AT_CDEVADDR, "DevAddr:length is 8, Device address of ABP mode", at_query_cdevaddr, at_exec_cdevaddr, NULL },
    { LORA_AT_CAPPSKEY, "AppSKey:length is 32", at_query_cappskey, at_exec_cappskey, NULL },
    { LORA_AT_CNWKSKEY, "NwkSKey:length is 32", at_query_cnwkskey, at_exec_cnwkskey, NULL },
    { LORA_AT_CADDMULTICAST, "DevAddr\",\"McKey\",\"Frequency\",\"DataRate\",\"Periodicity", at_query_caddmulticast, at_exec_caddmulticast, NULL },
    { LORA_AT_CDELMULTICAST, "DevAddr", at_query_cdelmulticast, at_exec_cdelmulticast, NULL },
    { LORA_AT_CNUMMULTICAST, "number", at_query_cnummulticast, NULL, NULL },
    { LORA_AT_CFREQBANDMASK, "mask", at_query_cfreqbandmask, at_exec_cfreqbandmask, NULL },
    { LORA_AT_CULDLMODE, "mode", at_query_culdlmode, at_exec_culdlmode, NULL },
    { LORA_AT_CWORKMODE, "mode", at_query_cworkmode, at_exec_cworkmode, NULL },
    { LORA_AT_CCLASS, "class\",\"branch\",\"para1", at_query_cclass, at_exec_cclass, NULL },
    { LORA_AT_CSTATUS, "status", at_query_cstatus, NULL, NULL },
    { LORA_AT_CJOIN, "ParaTag1\",\"ParaTag2", at_query_cjoin, at_exec_cjoin, NULL },
    { LORA_AT_DTX, "Length\",\"Payload", NULL, at_exec_dtx, NULL },
    { LORA_AT_DRX, "Length\",\"Payload", at_query_drx, NULL, NULL },
    { LORA_AT_CCONFIRM, "value", at_query_cconfirm, at_exec_cconfirm, NULL },
    { LORA_AT_CAPPPORT, "value", at_query_cappport, at_exec_cappport, NULL },
    { LORA_AT_CDATARATE, "value", at_query_cdatarate, at_exec_cdatarate, NULL },
    { LORA_AT_CRSSI, NULL, at_query_crssi, NULL, NULL },
    { LORA_AT_CNBTRIALS, "MTypes\",\"value", at_query_cnbtrials, at_exec_cnbtrials, NULL },
    { LORA_AT_CRM, "reportMode\",\"reportInterval", at_query_crm, at_exec_crm, NULL },
    { LORA_AT_CTXP, "value", at_query_ctxp, at_exec_ctxp, NULL },
    { LORA_AT_CLINKCHECK, "value", NULL, at_exec_clinkcheck, NULL },
    { LORA_AT_CADR, "value", at_query_cadr, at_exec_cadr, NULL },
    { LORA_AT_CRXP, "RX1DRoffset\",\"RX2DataRate", at_query_crxp, at_exec_crxp, NULL },
    { LORA_AT_CRX1DELAY, "value", at_query_crx1delay, at_exec_crx1delay, NULL },
    { LORA_AT_CSAVE, NULL, NULL, NULL, at_exec_csave },
    { LORA_AT_CRESTORE, NULL, NULL, NULL, at_exec_crestore },
    { LORA_AT_CLBT, "value", at_query_clbt, at_exec_clbt, NULL },
    { LORA_AT_CPROPRIETARY, "value", at_query_cproprietary, at_exec_cproprietary, NULL },
    { LORA_AT_CMONITORNODE, "value", at_query_cmonitornode, at_exec_cmonitornode, NULL },
    { LORA_AT_CREPEATERFREQ, "freq", at_query_crepeaterfreq, at_exec_crepeaterfreq, NULL }
};

void lorawan_at_process(void)
{
    uint8_t i;
    int ret = 0;
    const char *cmd_name;
    char *rxcmd = atcmd + 2;
    int16_t tmp = atcmd_index - 2;
    uint16_t rxcmd_index;

    if (tmp <= 0 || rxcmd[tmp] != '\0') {
        return;
    }

    rxcmd_index = tmp;

    for (i = 0; i < sizeof(g_at_cmd_list) / sizeof(atcmd_t); i++) {
        cmd_name = g_at_cmd_list[i].cmd_name;
        if (strncmp(rxcmd, cmd_name, strlen(cmd_name)) != 0) {
            continue;
        }

        if (rxcmd_index == (strlen(cmd_name) + 2) &&
            strcmp(&rxcmd[strlen(cmd_name)], "=?") == 0) {
            /* test cmd */
            if (g_at_cmd_list[i].test_cmd_str) {
                if (strncmp(g_at_cmd_list[i].test_cmd_str, "OK", 2) == 0) {
                    snprintf(atcmd, ATCMD_SIZE, "\r\nOK\r\n");
                } else {
                    snprintf(atcmd, ATCMD_SIZE, "\r\n%s:\"%s\"\r\nOK\r\n",
                             cmd_name, g_at_cmd_list[i].test_cmd_str);
                }
            } else {
                snprintf(atcmd, ATCMD_SIZE, "\r\n%s\r\nOK\r\n", cmd_name);
            }
        } else if (rxcmd_index == (strlen(cmd_name) + 1) &&
                   rxcmd[strlen(cmd_name)] == '?') {
            /* query cmd */
            if (g_at_cmd_list[i].query_cmd != NULL) {
                ret = g_at_cmd_list[i].query_cmd();

                if (ret == 0) {
                    snprintf(atcmd, ATCMD_SIZE, "\r\n%s:%s\r\nOK\r\n",
                             cmd_name, g_at_query_buf);
                }
            } else {
                ret = AT_ERRNO_NOALLOW;
            }
        } else if (rxcmd_index > (strlen(cmd_name) + 1) &&
                   rxcmd[strlen(cmd_name)] == '=') {
            /* exec cmd */
            if (g_at_cmd_list[i].exec_cmd != NULL) {
                ret = g_at_cmd_list[i].exec_cmd(rxcmd + strlen(cmd_name) + 1);
                if (ret == 0) {
                    snprintf(atcmd, ATCMD_SIZE, "\r\nOK\r\n");
                } else if (ret == -1) {
                    ret = AT_ERRNO_SYS;
                }
            } else {
                ret = AT_ERRNO_NOALLOW;
            }
        } else if (rxcmd_index == strlen(cmd_name)) {
            /* exec cmd without parameter*/
            if (g_at_cmd_list[i].exec_cmd_no_para != NULL) {
                ret = g_at_cmd_list[i].exec_cmd_no_para();
                if (ret == 0) {
                    snprintf(atcmd, ATCMD_SIZE, "\r\nOK\r\n");
                } else if (ret == -1) {
                    ret = AT_ERRNO_SYS;
                }
            } else {
                ret = AT_ERRNO_NOALLOW;
            }
        }
        break;
    }

    if (i == sizeof(g_at_cmd_list) / sizeof(atcmd_t)) {
        ret = AT_ERRNO_NOSUPP;
    }

    if (ret != 0 && ret != AT_CB_PRINT) {
        snprintf(atcmd, ATCMD_SIZE, "\r\n%s%x\r\n", AT_ERROR, ret);
    }

    if (ret != AT_CB_PRINT) {
        LORA_AT_PRINTF("\r\n%s\r\n", atcmd);
    }

    atcmd_index = 0;
    memset(atcmd, 0xff, ATCMD_SIZE);
    return;
}

// this can be in irq context
void lorawan_at_serial_input(uint8_t cmd)
{
    if ((cmd >= '0' && cmd <= '9') || (cmd >= 'a' && cmd <= 'z') ||
        (cmd >= 'A' && cmd <= 'Z') || cmd == '?' || cmd == '+' || cmd == ':' ||
        cmd == '=' || cmd == ' ' || cmd == ',') {
        atcmd[atcmd_index++] = cmd;
    } else if (cmd == '\r' || cmd == '\n') {
        atcmd[atcmd_index] = '\0';
    }

    if (atcmd_index > ATCMD_SIZE) {
        atcmd_index = 0;
    }
}

void lorawan_at_init(void)
{
    atcmd_index = 0;
    memset(atcmd, 0xff, ATCMD_SIZE);
}
