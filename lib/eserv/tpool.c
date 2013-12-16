#include "tpool.h"

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
