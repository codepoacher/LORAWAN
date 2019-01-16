#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "frtos_mem.h"
#include "frtos_lock.h"

#include "loopbuf.h"
#define min(x, y) ((x) < (y) ? (x) : (y))

struct loop_buffer *init_loop_buffer(unsigned int size)
{
	int ret;
	struct loop_buffer *lb = NULL;

	ret = size & (size - 1);
	if (ret)
		return NULL;
	lb = (struct loop_buffer *)mem_malloc(sizeof(struct loop_buffer));
	if (!lb)
		return NULL;

	memset(lb, 0, sizeof(struct loop_buffer));
	lb->size = size;
	lb->in = lb->out = 0;
	//pthread_mutex_init(&lb->lock, NULL);
	lb->lock = mutex_lock_init();
	if (!lb->lock) {
		mem_free(lb);
		return NULL;
	}

	lb->buf = (unsigned char *)mem_malloc(size);
	if (!lb->buf) {
		mem_free(lb);
		return NULL;
	} else {
		memset(lb->buf, 0, size);
	}
	return lb;
}

void destroy_loop_buffer(struct loop_buffer * lb)
{
	vPortFree(lb->buf);
	vPortFree(lb);
}

unsigned int loop_buffer_free(struct loop_buffer * lb)
{
	return (lb->size - lb->in + lb->out);
}

unsigned int loop_buffer_use(struct loop_buffer *lb)
{
	return (lb->in - lb->out);
}

unsigned int loop_buffer_put(struct loop_buffer *lb, unsigned char *buf, unsigned int len)
{
	unsigned int l;
	if (lb->in>lb->size && lb->out>lb->size) {
		lb->in  -= lb->size;
		lb->out -= lb->size;
	}
	len = min(len, lb->size - lb->in + lb->out);
	l = min(len, lb->size - (lb->in & (lb->size - 1)));
	memcpy(lb->buf + (lb->in & (lb->size - 1)), buf, l);
	memcpy(lb->buf, buf + l, len - l);
	lb->in += len;
	return len;
}

unsigned int loop_buffer_get(struct loop_buffer *lb, unsigned char *buf, unsigned int len)
{
	unsigned int l;
	if (lb->in>lb->size && lb->out>lb->size) {
		lb->in  -= lb->size;
		lb->out -= lb->size;
	}
	len = min(len, lb->in - lb->out);
	l = min(len, lb->size - (lb->out & (lb->size - 1)));
	memcpy(buf, lb->buf + (lb->out & (lb->size - 1)), l);
	memcpy(buf + l, lb->buf, len - l);
	lb->out += len;
	return len;
}
unsigned int loop_buffer_pick(struct loop_buffer *lb, unsigned char *buf, unsigned int len)
{
	unsigned int l;
	if (lb->in>lb->size && lb->out>lb->size) {
		lb->in  -= lb->size;
		lb->out -= lb->size;
	}
	len = min(len, lb->in - lb->out);
	l = min(len, lb->size - (lb->out & (lb->size - 1)));
	memcpy(buf, lb->buf + (lb->out & (lb->size - 1)), l);
	memcpy(buf + l, lb->buf, len - l);
	//lb->out += len;
	return len;
}

unsigned int loop_buffer_drop(struct loop_buffer *lb, unsigned int len)
{
	unsigned int l;
	len = min(len, lb->in - lb->out);
	lb->out += len;
	return len;
}

