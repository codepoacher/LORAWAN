#ifndef __UART_DRIVER_H__
#define __UART_DRIVER_H__

#include "frtos_types.h"
#include "frtos_ioctl.h"
#include "frtos_queue.h"
#include "frtos_softimer.h"
#include "frtos_lock.h"
#include "frtos_drivers.h"
#include "frtos_mem.h"
#include "loopbuf.h"
#include "stm32l0xx_hal.h"
#include "config_driver.h"

/**************************************************************************************
* Description    : 串口配置
**************************************************************************************/
#define UART_TXBUF_LEN					256				//uart3发送缓冲区大小
#define LPUART_BAUD						9600			//lpuart波特率
#define LPUART_RXBUF_LEN				256			    //lpuart接收缓冲区长度
#define LPUART_RXQUEUE_LEN				10				//lpuart接收队列长度
#define LPUART_RXQUEUE_SIZE				4				//lpuart接收队列元素大小
#define LPUART_TIMER_TIME				10				//lpuart定时器时间

#define UART5_BAUD						9600			//uart5波特率
#define UART5_RXBUF_LEN					256			    //uart5接收缓冲区长度
#define UART5_RXQUEUE_LEN				10				//uart5接收队列长度
#define UART5_RXQUEUE_SIZE				4				//uart5每个元素的大小
#define UART5_TIMER_TIME				10				//uart5定时器时间

#define UART4_BAUD						9600			//uart4波特率
#define UART4_RXBUF_LEN					256			    //uart4接收缓冲区长度
#define UART4_RXQUEUE_LEN				10				//uart4接收队列长度
#define UART4_RXQUEUE_SIZE				4				//uart4每个元素的大小
#define UART4_TIMER_TIME				10				//uart4定时器时间

#define UART3_BAUD						115200			//uart3波特率
#define UART3_RXBUF_LEN					256			    //uart3接收缓冲区长度
#define UART3_RXQUEUE_LEN				10				//uart3接收队列长度
#define UART3_RXQUEUE_SIZE				4				//uart3每个接收元素的大小
#define UART3_TIMER_TIME				1				//uart3定时器时间, 根据实际发包情况设置

#define UART2_BAUD						9600			//uart2波特率
#define UART2_RXBUF_LEN					256			    //uart2接收缓冲区长度
#define UART2_RXQUEUE_LEN				10				//uart2接收队列长度
#define UART2_RXQUEUE_SIZE				4				//uart2每个元素的大小
#define UART2_TIMER_TIME				10				//uart2定时器时间

#define UART1_BAUD						10400			//uart1波特率
#define UART1_RXBUF_LEN					256			    //uart1接收缓冲区长度
#define UART1_RXQUEUE_LEN				10				//uart1接收队列长度
#define UART1_RXQUEUE_SIZE				4				//uart1每个元素的大小
#define UART1_TIMER_TIME				10				//uart1定时器时间
/**************************************************************************************
* Description    : 串口状态
**************************************************************************************/
#define UART_TX_IDLE					0				//串口发送空闲
#define UART_TX_END						1				//串口发送结束

#define UART_RX_IDLE					0				//串口接收空闲
#define UART_RX_HALF					1				//串口接收缓冲区达到一半
#define UART_RX_FULL					2				//串口接收缓冲区满	

/**************************************************************************************
* Description    : 接收数据结构定义
**************************************************************************************/
struct huart_rx_s{
    uint16_t len;                                           // 数据长度
    uint8_t data[0];                                        // 数据内容
};

struct comm_huart {
	int idx;												//串口驱动序号
	mutex_lock_t tx_mutex;									//发送锁
	softimer_t rx_timer;									//接收定时器
	fqueue_t rx_queue;										//接收队列
	ioctl_cb_t rx_handler;									//接收回调函数
	struct loop_buffer *rx_buf;								//接收环形缓冲区
	uint8_t *send_buf;										//发送缓冲区
	uint16_t send_buf_cnt;									//发送缓冲区个数
	UART_HandleTypeDef *uart;								//具体的串口
}__attribute__((packed));

struct uart {
	struct comm_huart *comm_huart;							//通用串口指针
	uint8_t recv_data;										//中断接收数据
}__attribute__((packed));

/**************************************************************************************
* Description    : 函数声明
**************************************************************************************/
void lpuart_data_add(void);
void lpuart_send_end(void);
void uart5_data_add(void);
void uart5_send_end(void);
void uart4_data_add(void);
void uart4_send_end(void);
void uart3_data_add(void);
void uart3_send_end(void);
void uart2_data_add(void);
void uart2_send_end(void);
void uart1_data_add(void);
void uart1_send_end(void);
void uart_comm_timer_handle(int buf_len, struct loop_buffer *lbuf, fqueue_t queue);
void destroy_comm_uart(struct comm_huart *uart);
struct comm_huart *create_comm_huart(int buf_len, int queue_len, 
									int que_size, int timer_time, 
									void (*timer_cb)(softimer_t timer));
int32_t comm_uart_read(struct comm_huart *comm_uart);
int32_t comm_uart_write(struct comm_huart *comm_uart, void *data, int32_t len);
int32_t comm_uart_write_it(struct comm_huart *comm_uart, void *data, int32_t len);
int32_t comm_uart_ioctl(struct comm_huart *comm_uart, int32_t cmd, void *args, int32_t len);
void lprint(char *fmt, ...);

#endif  /*__S32L083_UART_H__ */
