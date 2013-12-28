#ifndef __ESERV_TPOOL_H__
#define __ESERV_TPOOL_H__

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <event2/event.h>
#include <event2/bufferevent.h>

enum{
	EX_TMGR_SUCCESS = 0,
	EX_TMGR_ALLOCFAIL,
	EX_TMGR_LOCKFAIL,
	EX_TWORKER_ALLOCFAIL,
	EX_TWORKER_INITFAIL,
	EX_TWKR_ALLOCFAIL,
	EX_TWKR_THREAD_FAIL,
};

typedef int (*ex_tworker_job_func)(ex_tworker*, void*);

typedef struct{
	int id;
	int jobs;
	pthread_t pid;

	event_base *base;
	ex_tworker_job_func job_func;
	void *arg;
	ex_tmanager *manager;
}ex_tworker;

typedef struct{
	int max_threads;
	int shutdown;

	pthread_mutex_t heap_lock;
	int workers_num;
	ex_tworker **workers;
	int (*worker_compare)(void*, void*);
}ex_tmanager;

int ex_tmanager_init(ex_tmanager **m, int max_threads, int (*compare)(void*, void*));
ex_tworker* ex_tmanager_req_wkr(ex_tmanager *mgr);
int ex_tmanager_wkr_done(ex_tworker *mgr, ex_tworker *w);
int ex_tmanager_free(ex_tmanager *mgr);

int ex_tworker_init(ex_tworker **w, ex_tmanager *m, int id, ex_tworker_job_func fun, void *arg);
int ex_tworker_increase(ex_tworker *w);
int ex_tworker_decrease(ex_tworker *w);
void* ex_tworker_work(void *s);
int ex_tworker_exit(ex_tworker *w);
int ex_tworker_free(ex_tworker *w);

#endif
