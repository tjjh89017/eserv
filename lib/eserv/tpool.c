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
	
	if((tpool->threads = (pthread_t*)malloc(sizeof(pthread_t) * max_threads)) == NULL)
		perror("ex_tpool_new: threads malloc error");
	tpool->queue_size = 0;
	tpool->queue_head = NULL;
	tpool->queue_tail = NULL;
	tpool->queue_closed = 0;
	tpool->shutdown = 0;

	if((rtn = pthread_mutex_init(&(tpool->queue_lock), NULL)) != 0)
		perror("ex_tpool_new: queue_lock init error");
	if((rtn = pthread_cond_init(&(tpool->queue_not_empty), NULL)) != 0)
		perror("ex_tpool_new: queue_not_epmty init error");
	if((rtn = pthread_cond_init(&(tpool->queue_not_full), NULL)) != 0)
		perror("ex_tpool_new: queue_not_full init error");
	if((rtn = pthread_cond_init(&(tpool->queue_empty), NULL)) != 0)
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
		return 1;
	}

	job = (ex_job*)malloc(sizeof(ex_job));
	job->routine = routine;
	job->arg = arg;
	job->next = NULL;

	if(tpool->queue_size == 0){
		tpool->queue_tail = tpool->queue_head = job;
	}
	else{
		tpool->queue_tail->next = job;
		tpool->queue_tail = job;
	}

	tpool->queue_size++;
	pthread_cond_signal(&tpool->queue_not_empty);
	pthread_mutex_unlock(&tpool->queue_lock);

	return 0;
}

int ex_tpool_free(ex_tpool *tpool, int finish_queue)
{
	int i = 0;
	ex_job *job = NULL;

	if(pthread_mutex_lock(&(tpool->queue_lock)) != 0)
		perror("pthread lock fail");

	if(tpool->queue_closed || tpool->shutdown){
		pthread_mutex_unlock(&(tpool->queue_lock));
		return 0;
	}

	tpool->queue_closed = 1;

	if(finish_queue){
		while(tpool->queue_size != 0){
			if(pthread_cond_wait(&(tpool->queue_empty), &(tpool->queue_lock)) != 0){
				pthread_mutex_unlock(&tpool->queue_lock);
				perror("finish queue error");
			}
		}
	}

	tpool->shutdown = 1;
	if(pthread_mutex_unlock(&(tpool->queue_lock)) != 0){
		perror("unlock error");
		return 1;
	}

	if(pthread_cond_broadcast(&(tpool->queue_not_empty)) != 0){
		perror("broadcast error");
		return 1;
	}

	for(i = 0; i < tpool->max_threads; i++){
		if(pthread_join(tpool->threads[i], NULL) != 0){
			perror("pthread join error");
		}
	}

	free(tpool->threads);

	while(tpool->queue_head != NULL){
		job = tpool->queue_head;
		tpool->queue_head = tpool->queue_head->next;
		free(job);
	}

	pthread_mutex_destroy(&(tpool->queue_lock));
	pthread_cond_destroy(&(tpool->queue_not_empty));
	pthread_cond_destroy(&(tpool->queue_not_full));
	pthread_cond_destroy(&(tpool->queue_empty));

	free(tpool);

	return 0;
}

void* ex_tpool_work(ex_tpool *tpool)
{
	ex_job *job = NULL;
	
	while(1){
		pthread_mutex_lock(&(tpool->queue_lock));
		while(tpool->queue_size == 0 && !tpool->shutdown){
			pthread_cond_wait(&(tpool->queue_not_empty), &(tpool->queue_lock));
		}

		if(tpool->shutdown){
			pthread_mutex_unlock(&(tpool->queue_lock));
			pthread_exit(NULL);
		}

		job = tpool->queue_head;
		tpool->queue_size--;

		if(tpool->queue_size == 0){
			tpool->queue_head = tpool->queue_tail = NULL;
			pthread_cond_signal(&(tpool->queue_empty));
		}
		else{
			tpool->queue_head = job->next;
		}

		if(tpool->block_queue_full && tpool->queue_size == tpool->max_queue_size - 1){
			pthread_cond_signal(&(tpool->queue_not_full));
		}

		pthread_mutex_unlock(&(tpool->queue_lock));
		DBG("pid: %d", pthread_self());
		(*(job->routine))(job->arg);
		free(job);
	}
}

