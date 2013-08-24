#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "multihash.h"

void ex_multihash_init(ex_multihash *mh, ex_mpool *mp, size_t _size)
{
	assert(mp != NULL);
	mh->mpool = mp;
	mh->buckets = (ex_hashlist**)ex_mpool_malloc(mp, sizeof(long) * _size);
	memset(mh->buckets, 0, sizeof(long) * _size);
	mh->size = _size;

	mh->hashcmp = (void*)ex_hashcmp_str;
	mh->hashfun = (void*)ex_hashfun_str;
}

ex_multihash* ex_multihash_new(ex_mpool *mp, size_t _size)
{
	ex_multihash *mh = malloc(sizeof(ex_multihash));
	ex_multihash_init(mh, mp, _size);

	return mh;
}

void ex_hash_clear(ex_multihash *mh)
{
	ex_hashlist *nl, *cl;
	struct _mlist *nml, *ml;
	size_t i;

	do{
		if(mh->mpool == NULL || mh->mpool->cflag == 0)
			break;
		for(i = 0; i < hm->size; i++){
			cl = hm->buckets[i];
			while(cl != NULL){
				nl = cl->next;
				ml = (struct _mlist*)cl->value;
				while(ml != NULL){
					nml = ml->next;
					ex_mpool_free(mh->mpool, ml);
					ml = nml;
				}
				ex_mpool_free(mh->mpool, cl);
				cl = nl;
			}
		}
		ex_mpool_free(mh->mpool, mh->buckets);
	}while(0);

	memset(mh, 0, sizeof(*mh));
}

int ex_hash_add(ex_multihash *mh, const void *key, const void *value)
{
	int pos = mh->hashfun(key) % mh->size;
	ex_hashlist *nlh = hm->buckets[pos];
	struct _mlist *ml = malloc(sizeof(struct _mlist));
	ml->value = (void*)value;

	while(nlh != NULL){
		if(mh->hashcmp(nlh->key, key))
			break;
		nlh = nlh->next;
	}

	if(nlh == NULL){
		nlh = (ex_hashlist *)
			ex_mpool_malloc(hm->mpool, sizeof(ex_hashlist));
		nlh->value = NULL;
	}
	ml->next = (struct _mlist*)nlh->value;
	nlh->key = (void*)key, nlh->value = (void*)ml;
	nlh->next = mh->buckets[pos];
	mh->buckets[pos] = nlh;

	return pos;
}

void* ex_multihash_find(const ex_multihash *mh, const void *key)
{
	static ex_multihash *target = NULL;
	static struct _mlist *ml = NULL;

	if(key != NULL){
		target = mh;
		pos = mh->hashfun(key) % mh->size;
		nlh = mh->buckets[pos];
		while(nlh != NULL){
			if(mh->hashcmp(nlh->key, key)){
				ml = nlh->value;
				break;
			}
			nlh = nlh->next;
		}
	}

	if(ml != NULL){
		struct _mlist *ret = ml;
		ml = ml->next;
		return ret;
	}

	return ml;
}
