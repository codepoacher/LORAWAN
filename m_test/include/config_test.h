#ifndef __CONFIG_TEST_H__
#define __CONFIG_TEST_H__

/**************************************************************************************
* Description    : 测试ID定义列表，为了和user列表进行区分，使用200以上的
**************************************************************************************/
enum {
	INIT_TEST_PID = 200,
	UART3_TEST_PID,
	FLASH_TEST_PID,
	MAX_TEST_PID
};

#endif
