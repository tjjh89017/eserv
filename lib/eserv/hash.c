#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "hash.h"

static int ex_hashcmp_str(const char *s1, const char *s2)
{
	return (0 == strcmp(s1, s2));
}

void ex_hash_init(ex_hashmap *hm, ex_mpool *mp, size_t _size)
{
	assert(mp != NULL);
	hm->mpool = mp;
	hm->buckets = (ex_hashlist **)
		ex_mpool_malloc(mp,
		                sizeof(long) * _size);
	memset(hm->buckets, 0, sizeof(long) * _size);
	hm->size = _size;

	hm->hashcmp = (void *) ex_hashcmp_str;
	hm->hashfun = (void *) ex_hashfun_str;

	hm->recycle = NULL;
}

void ex_hash_clear(ex_hashmap *hm)
{
	ex_hashlist *nl, *cl;
	size_t i;

	do {
		if (hm->mpool == NULL || hm->mpool->cflag == 0)
			break;
		for (i = 0 ; i < hm->size ; ++i) {
			cl = hm->buckets[i];
			while (cl != NULL) {
				nl = cl->next;
				ex_mpool_free(hm->mpool, cl);
				cl = nl;
			}
		}
		ex_mpool_free(hm->mpool, hm->buckets);
	} while (0);

	memset(hm, 0, sizeof(*hm));
}

int ex_hash_add(ex_hashmap *hm, const void *key, const void *value)
{
	int pos = hm->hashfun(key) % hm->size;
	ex_hashlist *nhl;

	if(hm->recycle){
		nhl = hm->recycle;
		hm->recycle = nhl->next;
		nhl->next = NULL;
	}
	else{
		nhl = (ex_hashlist *)
			ex_mpool_malloc(hm->mpool,
			                sizeof(ex_hashlist));
	}

	nhl->key = (void *) key, nhl->value = (void *) value;
	nhl->next = hm->buckets[pos];
	hm->buckets[pos] = nhl;

	return pos;
}

void* ex_hash_find(const ex_hashmap *hm, const void *key)
{
	int pos = hm->hashfun(key) % hm->size;
	ex_hashlist *nlh = hm->buckets[pos];
	void *ret = NULL;
	while (nlh != NULL) {
		if (hm->hashcmp(nlh->key, key)) {
			ret = nlh->value;
			break;
		}
		nlh = nlh->next;
	}
	return ret;
}

void ex_hash_del(ex_hashmap *hm, const void *key)
{
	int pos = hm->hashfun(key) % hm->size;
	ex_hashlist *nlh = hm->buckets[pos], **last = &hm->buckets[pos];

	while(nlh != NULL){
		if(hm->hashcmp(nlh->key, key)){
			*last = nlh->next;
			free(nlh->key);
			free(nlh->value);
			// TODO find a better way to free memory
			// need to recycle
			if(ex_mpool_free(hm->mpool, nlh)){
				nlh->next = hm->recycle;
				hm->recycle = nlh;
			}
			break;
		}
		last = &nlh;
		nlh = nlh->next;
	}
}

int ex_hashfun_str(const char *s)
{
	int even , odd;
	int i = 0;
	int mask = 0x1F;
	int ret;
	even = odd = 0;
	while (*s != '\0') {
		if (i & 1) {
			odd ^= *s;
		}
		else {
			even ^= *s;
		}
		++s;
		++i;
	}
	ret = even & mask;
	ret <<= 5;
	ret += (odd & mask);
	return ret;
}

/*
 * TODO find all things in HashMap
 */
ex_hashlist* ex_hash_next(const ex_hashmap *hm){

	static int bucket_pos = 0;
	static ex_hashmap *target = NULL;
	static ex_hashlist *current = NULL;

	if(hm != NULL){
		target = hm;
		bucket_pos = 0;
		current = target->buckets[0];
	}

	if(current != NULL){
		if(current->next != NULL)
			current = current->next;
		else if(++bucket_pos < target->size)
			current = target->buckets[bucket_pos];
		else
			current = NULL;
	}

	return current;
}
