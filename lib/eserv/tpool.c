#include "tpool.h"

#define SWAP(a, b, size) \
{ \
	int _size = (size); \
	char *_a = (a), *_b = (b); \
	char _tmp; \
	while(_size--){ \
		_tmp = *_a; \
		*_a++ = *_b; \
		*_b++ = _tmp; \
	} \
}

static void reheap(void *array_base, int len, int size, int (*cmp)(void*, void*))
{
	char *array = array_base;
	int i = 0;
	int son = 0;
	int parent = 0;
	char *s_ptr = &array[son];
	char *p_ptr = &array[parent];

	for(i = 0; i < len; i++){
		son = i;
		parent = (son - 1) / 2;
		s_ptr = &array[son * size];
		p_ptr = &array[parent * size];
		while(parent >= 0 && (*cmp)((void*)s_ptr, (void*)p_ptr) > 0){
			SWAP(s_ptr, p_ptr, size);
			son = parent;
			parent = (son - 1) / 2;
			s_ptr = &array[son * size];
			p_ptr = &array[parent * size];
		}
	}
}

int default_compare(void *a, void *b)
{
	return ((ex_tworker*)a)->jobs - ((ex_tworker*)b)->jobs;
}

int ex_tmanager_init(ex_tmanager **m, int max_threads, int (*compare)(void*, void*))
{
	int rtn = 0;
	int i = 0;
	ex_tmanager *mgr;
	if((mgr = (ex_tmanager*)malloc(sizeof(ex_tmanager))) == NULL)
		return EX_TMGR_ALLOCFAIL;

	mgr->workers_num = mgr->max_threads = max_threads;
	mgr->shutdown = 0;
	mgr->worker_compare = compare;
	if(compare == NULL)
		mgr->worker_compare = default_compare;

	if((rtn = pthread_mutex_init(&mgr->heap_lock, NULL)) != 0){
		rtn = EX_TMGR_LOCKFAIL;
		goto TMGR_INIT_FAIL;
	}

	if((mgr->workers = (ex_tworker**)malloc(mgr->workers_num * sizeof(ex_tworker*))) == NULL){
		rtn = EX_TWORKER_ALLOCFAIL;
		goto TMGR_INIT_FAIL;
	}
	for(i = 0; i < mgr->workers_num; i++){
		if((rtn = ex_tworker_init(&mgr->workers[i], mgr)) != 0){
			rtn = EX_TWORKER_INITFAIL;
			goto TMGR_INIT_FAIL;
		}
	}

	*m = mgr;
	return EX_TMGR_SUCCESS;

	TMGR_INIT_FAIL:
		switch(rtn){
			EX_TWORKER_INITFAIL:
				while(i--){
					ex_tworker_free(&mgr->workers[i]);
				}
			EX_TWORKER_ALLOCFAIL:
				free(mgr->workers);
			EX_TMGR_LOCKFAIL:
			EX_TMGR_ALLOCFAIL:
				free(mgr);
		}
		return rtn;
}

ex_tworker* ex_tmanager_req_wkr(ex_tmanager *mgr)
{
	pthread_mutex_lock(&mgr->heap_lock);

	ex_tworker *rtn = mgr->workers[0];
	ex_tworker_increase(rtn);

	// TODO spilt heap to another function
	//re-heap the MGR workers
	reheap(mgr->workers, mgr->max_threads, sizeof(ex_tworker*), mgr->worker_compare);
	
	pthread_mutex_unlock(&mgr->heap_lock);
	return rtn;
}

int ex_tmanager_wkr_done(ex_tmanager *mgr, ex_tworker *w)
{
	pthread_mutex_lock(&mgr->heap_lock);

	ex_tworker_decrease(w);
	reheap(mgr->workers, mgr->max_threads, sizeof(ex_tworker*), mgr->worker_compare);

	pthread_mutex_unlock(&mgr->heap_lock);
	return 0;
}

int ex_tmanager_free(ex_tmanager *mgr)
{
	pthread_mutex_lock(&mgr->heap_lock);

	int rtn = 0;
	int i = 0;
	for(i = 0; i < mgr->max_threads; i++){
		event_del(mgr->workers[i]->persist_event);
		if((rtn = pthread_join(&mgr->workers[i]->pid, NULL)) != 0){
			ex_tworker_free(mgr->workers[i]);
		}
	}
	pthread_mutex_unlock(&mgr->heap_lock);

	pthread_mutex_destroy(&mgr->heap_lock);
	free(mgr);

	return 0;
}

int ex_tworker_init(ex_tworker **w, ex_tmanager *m)
{
	int rtn = 0;
	ex_tworker *wkr = NULL;
	if((wkr = (ex_tworker*)malloc(sizeof(ex_tworker))) == NULL)
		return EX_TWKR_ALLOCFAIL;

	wkr->jobs = 0;
	wkr->manager = m;

	if((rtn = pthread_create(&wkr->pid, NULL, ex_tworker_work, wkr)) != 0){
		free(wkr);
		return EX_TWKR_THREAD_FAIL;
	}

	*w = wkr;
	return 0;
}

int ex_tworker_increase(ex_tworker *w)
{
	w->jobs++;

	return 0;
}

int ex_tworker_decrease(ex_tworker *w)
{
	w->jobs--;

	return 0;
}

static void persist_cb(int sock, short which, void *arg)
{
	struct event *ev = *(struct event**)arg;
	event_active(ev, EV_WRITE, 0);
}

void* ex_tworker_work(void *s)
{
	ex_tworker *wkr = (ex_tworker*)s;
	struct event_config *cfg = event_config_new();
	// TODO something strange there
	//event_config_set_max_dispatch_interval(cfg, NULL, 16, 0);
	wkr->base = event_base_new_with_config(cfg);

	wkr->persist_event = event_new(wkr->base, -1, EV_PERSIST | EV_READ, persist_cb, &wkr->persist_event);
	event_add(wkr->persist_event, NULL);
	event_base_dispatch(wkr->base);

	return NULL;
}

void ex_tworker_exit(evutil_socket_t fd, short event_code, void *arg)
{
	pthread_exit(NULL);
}

int ex_tworker_free(ex_tworker *w)
{
	event_base_free(w->base);
	free(w);

	return 0;
}