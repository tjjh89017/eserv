#ifndef __ESERV_TPOOL_H__
#define __ESERV_TPOOL_H__

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct _job{
	void (*routine)(void*);
	void *arg;
	struct _job *next;
} ex_job;

typedef struct{
	int max_threads;
	int max_queue_size;
	int block_queue_full;
	int queue_size;
	int queue_closed;
	int shutdown;
	ex_job *queue_head;
	ex_job *queue_tail;
	pthread_mutex_t queue_lock;
	pthread_cond_t queue_not_empty;
	pthread_cond_t queue_not_full;
	pthread_cond_t queue_empty;

	pthread_t *threads;
} ex_tpool;

ex_tpool* ex_tpool_new(int max_threads, int max_queue_size, int block_queue_full);
int ex_tpool_add(ex_tpool *tpool, void (*routine)(void*), void *arg);
int ex_tpool_free(ex_tpool *tpool, int finish_queue);
void* ex_tpool_work(ex_tpool *tpool);

#endif
