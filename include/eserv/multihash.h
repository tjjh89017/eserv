#ifndef __ESERV_MULTIHASH_H__
#define __ESERV_MULTIHASH_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "mempool.h"
#include "hash.h"
#include <string.h>

struct _mlist{
	void *value;
	struct _mlist *next;
}

typedef struct{
	ex_hashlist **buckets;
	size_t size;
	ex_mpool *mpool;
	int (*hashfun)(const void *key);
	int (*hashcmp)(const void *lkey, const void *rkey);
} ex_multihash;

void ex_multihash_init(ex_multihash *mh, ex_mpool *mp, size_t _size);
ex_multihash* ex_multihash_new(ex_mpool *mp, size_t _size);
void ex_multihash_clear(ex_multihash *mh);

int ex_multihash_add(ex_multihash *mh, const void *key, const void *value);
void* ex_multihash_find(const ex_multihash *mh, const void *key);

#ifdef _cplusplus
}
#endif
#endif
