#ifndef _HASH_H
#define _HASH_H

#include "stddef.h"
#include "list.h"

struct hash_elem 
{
    struct list_elem list_elem;
};

typedef unsigned hash_hash_func (const struct hash_elem *e, void *aux);

typedef bool hash_less_func (const struct hash_elem *a,
                             const struct hash_elem *b,
                             void *aux);

struct hash 
{
    size_t elem_cnt;            /* Number of elements in table. */
    size_t bucket_cnt;          /* Number of buckets, a power of 2. */
    struct list *buckets;       /* Array of `bucket_cnt' lists. */
    hash_hash_func *hash;       /* Hash function. */
    hash_less_func *less;       /* Comparison function. */
    void *aux;                  /* Auxiliary data for `hash' and `less'. */
};

#endif