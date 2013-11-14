#include "tpool.h"

ex_tpool* ex_tpool_new(int max_threads, int max_queue_size, int block_queue_full)
{
	int i = 0;
	int rtn = 0;
	ex_tpool *tpool = NULL;
	if((tpool = (ex_tpool*)malloc(sizeof(ex_tpool))) == NULL)
		perror("ex_tpool_new: tpool malloc error");
	tpool->max_threads = max_threads;
	tpool->max_queue_size = max_queue_size;
	tpool->block_queue_full = block_queue_full;
	
	if((tpool->threads = (pthread*)malloc(sizeof(pthread_t) * max_threads)) == NULL)
		perror("ex_tpool_new: threads malloc error");
	tpool->queue_size = 0;
	tpool->queue_head = NULL;
	tpool->queue_tail = NULL;
	tpool->queue_closed = 0;
	tpool->shutdown = 0;

	if((rtn = pthread_mutex_init(&(tpool->queue_lock), NULL)) != 0)
		perror("ex_tpool_new: queue_lock init error");
	if((rtn = othread_cond_init(&(tpool->queue_not_empty), NULL)) != 0)
		perror("ex_tpool_new: queue_not_epmty init error");
	if((rtn = othread_cond_init(&(tpool->queue_not_full), NULL)) != 0)
		perror("ex_tpool_new: queue_not_full init error");
	if((rtn = othread_cond_init(&(tpool->queue_empty), NULL)) != 0)
		perror("ex_tpool_new: queue_empty init error");

	for(i = 0; i < max_threads; i++){
		if((rtn = pthread_create(&(tpool->threads[i]), NULL, ex_tpool_work, (void*)tpool)) != 0)
			perror("ex_tpool_new: pthread create error");
	}

	return tpool;
}

int ex_tpool_add(ex_tpool *tpool, void (*routine)(void*), void *arg)
{
	ex_job *job = NULL;
	pthread_mutex_lock(&tpool->queue_lock);

	if((tpool->queue_size == tpool->max_queue_size) && !tpool->block_queue_full){
		pthread_mutex_unlock(&tpool->queue_lock);
		return 1;
	}

	while ((tpool->queue_size == tpool->max_queue_size) && (!(tpool->shutdown || tpool->queue_closed))){
		pthread_cond_wait(&tpool->queue_not_full, &tpool->queue_lock);
	}
	if (tpool->shutdown || tpool->queue_closed){
		pthread_mutex_unlock(&tpool->queue_lock);
		return 0;
	}



}
int ex_tpool_free(ex_tpool *tpool, int finish_queue);
void* ex_tpool_work(ex_tpool *tpool);


