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
	EX_TWORKER_INITFAIL
};

typedef struct{
	int id;
	int jobs;

	event_base *base;

	ex_tmanager *manager;
}ex_tworker;

typedef struct{
	int max_threads;
	int shutdown;

	pthread_mutex_t heap_lock;
	int workers_num;
	ex_tworker** workers;
	int (*worker_compare)(void*, void*);
} ex_tmanager;

int ex_tmanager_init(ex_tmanager **m, int max_threads, int (*compare)(void*, void*));
int ex_tmanager_add(ex_tmanager *mgr, void *s);
int ex_tmanager_free(ex_tmanager *mgr);

int ex_tworker_init(ex_tworker **w, ex_tmanager *m, int id);

#endif
