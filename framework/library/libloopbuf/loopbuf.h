#ifndef __LOOP_BUF_H_
#define __LOOP_BUF_H_
#ifdef __cplusplus
extern "C" {
#endif
//#include <pthread.h>
#include "FreeRTOS.h"
#include "semphr.h"

struct loop_buffer {  
    unsigned char *buf;  
    unsigned int size;  
    unsigned int in;  
    unsigned int out;  
    SemaphoreHandle_t lock;
};

struct loop_buffer *init_loop_buffer(unsigned int size);
void destroy_loop_buffer(struct loop_buffer *lb);

unsigned int loop_buffer_free(struct loop_buffer *lb);
unsigned int loop_buffer_use(struct loop_buffer *lb);
unsigned int loop_buffer_put(struct loop_buffer *lb, unsigned char *buf, unsigned int len);
unsigned int loop_buffer_get(struct loop_buffer *lb, unsigned char *buf, unsigned int len);
unsigned int loop_buffer_pick(struct loop_buffer *lb, unsigned char *buf, unsigned int len);
unsigned int loop_buffer_drop(struct loop_buffer *lb, unsigned int len);

#endif
#ifdef __cplusplus
}
#endif
