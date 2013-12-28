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
		s_ptr = &array[son];
		p_ptr = &array[parent];
		while(parent >= 0 && (*cmp)((void*)s_ptr, (void*)p_ptr) > 0){
			SWAP(s_ptr, s_ptr, size);
			son = parent;
			parent = (son - 1) / 2;
			s_ptr = &array[son];
			p_ptr = &array[parent];
		}
	}
}

int default_compare(void *a, void *b)
{
	return ((ex_tworker*)a->jobs - (ex_tworker*)b->jobs);
}

int ex_tmanger_init(ex_tmanager **m, int max_threads, int (*compare)(void*, void*))
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

	if((rtn = pthread_mutex_init(&mgr->heap_lock)) != 0){
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
	pthread_mutex_lock(mgr->heap_lock);

	ex_tworker *rtn = mgr->workers[0];
	ex_tworker_increase(rtn);

	// TODO spilt heap to another function
	//re-heap the MGR workers
	reheap(mgr->workers, mgr->max_threads, sizeof(ex_tworker*), mgr->worker_compare);
	
	pthread_mutex_unlock(mgr->heap_lock);
	return rtn;
}

int ex_tmanager_wkr_done(ex_tworker *mgr, ex_tworker *w)
{
	pthread_mutex_lock(mgr->heap_lock);

	ex_tworker_decrease(w);
	reheap(mgr->workers, mgr->max_threads, sizeof(ex_tworker*), mgr->worker_compare);

	pthread_mutex_unlock(mgr->heap_lock);
	return 0;
}