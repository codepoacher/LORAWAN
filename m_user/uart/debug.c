#include "uart_driver.h"
#include "config_driver.h"
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
#include "loopbuf.h"
void debug(char *fmt, ...)
{
	uint8_t p[64];
	va_list args;
	int n;

	va_start(args, fmt);
	n = vsnprintf((char*)p, 64, fmt, args);
	if (n < 0)
		return;
	va_end(args);

	fdrive_write(DRIVER_LPUART, p, strlen(p));
}

//加入以下代码,支持printf函数,不需要use MicroLIB 
#if 0 
#pragma import(__use_no_semihosting)
//标准库需要的支持函数                  
struct __FILE
{
	int handle;
};

FILE __stdout;
//定义_sys_exit()以避免使用半主机模式     
void _sys_exit(int x)
{
	x = x;
}
//重定义fputc函数  
int fputc(int ch, FILE *f)
{
	uint8_t temp[1]={ch};

	fdrive_write(DRIVER_LPUART, temp, 1);

	return ch;
}
#endif
