/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.
 * File Name     : frtos_trace.c
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#include "string.h"
#include "frtos_mem.h"
#include "mini-printf.h"
#include "config_driver.h"

/**************************************************************************************
* Description    : 定义打印需要相关
**************************************************************************************/
#define TRACE_MSG_LENGTH                   300               // trace消息体最大长度
#define DUMP_STACK_DEPTH                   72                // trace栈大小，一定要4的整数倍
#define FAULT_USER_STACK_DEPTH             26                // 用户栈压栈大小
#define FAULT_KERNEL_STACK_DEPTH           8                 // 内核栈压栈大小

/**************************************************************************************
* Description    : 定义压栈寄存器
**************************************************************************************/
static struct pt_regs {
    uint32_t *r0;                                // ARM-M的R0寄存器
    uint32_t *r1;                                // ARM-M的R1寄存器
    uint32_t *r2;                                // ARM-M的R2寄存器
    uint32_t *r3;                                // ARM-M的R3寄存器
    uint32_t *r12;                               // ARM-M的R12寄存器
    uint32_t *lr;                                // ARM-M的lr寄存器
    uint32_t *pc;                                // ARM-M的pc寄存器
    uint32_t *psr;                               // ARM-M的psr寄存器
} regs;

/**************************************************************************************
* FunctionName   : trace_msg_send()
* Description    : 打印MCU异常信息，需要外部函数实现
* EntryParameter : data，打印数据内容,len数据内容长度
* ReturnValue    : 返回None
**************************************************************************************/
void __default trace_msg_send(uint8_t *data, uint32_t len)
{
    (void)data;(void)len;
}

/**************************************************************************************
* FunctionName   : trace_finished()
* Description    : 外部函数，外部实现，实现设备重启或者其他功能
* EntryParameter : None
* ReturnValue    : 返回None
**************************************************************************************/
void __default trace_finished(void)
{
}

/**************************************************************************************
* FunctionName   : backtrace()
* Description    : 打印MCU异常的时候，程序状态值
* EntryParameter : None
* ReturnValue    : 返回None
**************************************************************************************/
void __used backtrace(uint32_t *stack,  uint32_t is_kernel)
{
    uint32_t depth;
    uint8_t *tcb_head = NULL;
    uint32_t *usr_stack = NULL;
    uint32_t *usr_stack_top = NULL, psr;
    char trace_msg[TRACE_MSG_LENGTH];

    //1.获取应用psr值
    __asm__ __volatile__("mrs %0, psr \n":"=r"(psr));
    tcb_head = (uint8_t *)xTaskGetCurrentTaskHandle();
    usr_stack = (uint32_t *)(*((uint32_t *)(tcb_head)));
    usr_stack_top = (uint32_t *)(*((uint32_t *)(tcb_head + 8 + 2*sizeof(ListItem_t))));

    //2.将sp中压栈的内容还原到regs当中
    memcpy((uint8_t *)&regs, (uint8_t *)stack, sizeof(struct pt_regs));

    if(is_kernel) depth = FAULT_KERNEL_STACK_DEPTH;
    else depth = FAULT_USER_STACK_DEPTH;

    //3.打印出关键寄存器值
    snprintf((char *)trace_msg, TRACE_MSG_LENGTH ,STRBR"Backtrace:"STRBR
            "\tstack\tpointer: 0x%X\tr0:0x%X\tr1:0x%X\tr2:0x%X\tr3:0x%X"STRBR
            "\tpc\tpointer: 0x%X\tr12:0x%X \tlr:0x%X\tpsr:0x%X\tfault: 0x%X"STRBR
			"\tusr stack\tpointer: 0x%X\tcurrent:0x%X"STRBR
			"\tusr stack\ttop    : 0x%X(0x%X,0x%X,0x%X,0x%X)"STRBR,
            stack+depth, regs.r0, regs.r1, regs.r2, regs.r3, regs.pc,
            regs.r12, regs.lr, regs.psr, psr, usr_stack, tcb_head, usr_stack_top, *usr_stack_top,
			*(usr_stack_top+1), *(usr_stack_top+2),*(usr_stack_top+3));

    trace_msg_send((uint8_t *)trace_msg, strlen(trace_msg));

    //4.104bytes是异常中断触发后， 自动压入的数据， 加上104就到了程序真正的栈位置
    snprintf((char *)trace_msg, TRACE_MSG_LENGTH, "dump %s stack:",is_kernel?"kernel":"user");
    stack = stack + depth + DUMP_STACK_DEPTH;
    for (depth = 0; depth < DUMP_STACK_DEPTH; depth = depth+4, stack -= 4) {
    	trace_msg_send((uint8_t *)trace_msg, strlen(trace_msg));
        snprintf((char *)trace_msg,TRACE_MSG_LENGTH, STRBR"\t0x%X: %X %X %X %X",
                stack, *stack, *(stack-1), *(stack-2), *(stack-3));
    }
    trace_msg_send((uint8_t *)trace_msg, strlen(trace_msg));
    trace_msg_send((uint8_t *)STRBR"done.", strlen(STRBR"done."));
    trace_finished();
    while(1);
}

/**************************************************************************************
* FunctionName   : frtos_trace()
* Description    : 获取当前CPU工作模式，取对应SP传入给backtrace
* EntryParameter : r0, sp指向的栈位置， r1,1表示内核模式， 2表示用户模式
* ReturnValue    : 返回None
**************************************************************************************/
void __attribute__((naked)) frtos_trace(void)
{
    __asm volatile (
    "     movs r0, #4                          \n"
    "     movs r1, lr                          \n"
    "     tst r0, r1                           \n"
    "     beq kernel_fault_isr                 \n"
	"     mov r1, #0                           \n"
    "     mrs r0, psp                          \n"
    "     b user_fault_isr                     \n"
    " kernel_fault_isr:                        \n"
    "     mov r1, #1                           \n"
    "     mrs r0, msp                          \n"
    " user_fault_isr:                          \n"
    "     b backtrace                          \n"
    "     bkpt #0                              \n"
    );
}
