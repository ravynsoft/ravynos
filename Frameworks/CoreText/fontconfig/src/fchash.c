/*
 * Copyright © 2000 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of the author(s) not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  The authors make no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * THE AUTHOR(S) DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
#include "fcint.h"

#define FC_HASH_SIZE 227

typedef struct _FcHashBucket {
    struct _FcHashBucket  *next;
    void                  *key;
    void                  *value;
} FcHashBucket;

struct _FcHashTable {
    FcHashBucket  *buckets[FC_HASH_SIZE];
    FcHashFunc     hash_func;
    FcCompareFunc  compare_func;
    FcCopyFunc     key_copy_func;
    FcCopyFunc     value_copy_func;
    FcDestroyFunc  key_destroy_func;
    FcDestroyFunc  value_destroy_func;
};


FcBool
FcHashStrCopy (const void  *src,
	       void       **dest)
{
    *dest = FcStrdup (src);

    return *dest != NULL;
}

FcHashTable *
FcHashTableCreate (FcHashFunc    hash_func,
		   FcCompareFunc compare_func,
		   FcCopyFunc    key_copy_func,
		   FcCopyFunc    value_copy_func,
		   FcDestroyFunc key_destroy_func,
		   FcDestroyFunc value_destroy_func)
{
    FcHashTable *ret = malloc (sizeof (FcHashTable));

    if (ret)
    {
	memset (ret->buckets, 0, sizeof (FcHashBucket *) * FC_HASH_SIZE);
	ret->hash_func = hash_func;
	ret->compare_func = compare_func;
	ret->key_copy_func = key_copy_func;
	ret->value_copy_func = value_copy_func;
	ret->key_destroy_func = key_destroy_func;
	ret->value_destroy_func = value_destroy_func;
    }
    return ret;
}

void
FcHashTableDestroy (FcHashTable *table)
{
    int i;

    for (i = 0; i < FC_HASH_SIZE; i++)
    {
	FcHashBucket *bucket = table->buckets[i], *prev;

	while (bucket)
	{
	    if (table->key_destroy_func)
		table->key_destroy_func (bucket->key);
	    if (table->value_destroy_func)
		table->value_destroy_func (bucket->value);
	    prev = bucket;
	    bucket = bucket->next;
	    free (prev);
	}
	table->buckets[i] = NULL;
    }
    free (table);
}

FcBool
FcHashTableFind (FcHashTable  *table,
		 const void   *key,
		 void        **value)
{
    FcHashBucket *bucket;
    FcChar32 hash = table->hash_func (key);

    for (bucket = table->buckets[hash % FC_HASH_SIZE]; bucket; bucket = bucket->next)
    {
	if (!table->compare_func(bucket->key, key))
	{
	    if (table->value_copy_func)
	    {
		if (!table->value_copy_func (bucket->value, value))
		    return FcFalse;
	    }
	    else
		*value = bucket->value;
	    return FcTrue;
	}
    }
    return FcFalse;
}

static FcBool
FcHashTableAddInternal (FcHashTable *table,
			void        *key,
			void        *value,
			FcBool       replace)
{
    FcHashBucket **prev, *bucket, *b;
    FcChar32 hash = table->hash_func (key);
    FcBool ret = FcFalse;

    bucket = (FcHashBucket *) malloc (sizeof (FcHashBucket));
    if (!bucket)
	return FcFalse;
    memset (bucket, 0, sizeof (FcHashBucket));
    if (table->key_copy_func)
	ret |= !table->key_copy_func (key, &bucket->key);
    else
	bucket->key = key;
    if (table->value_copy_func)
	ret |= !table->value_copy_func (value, &bucket->value);
    else
	bucket->value = value;
    if (ret)
    {
    destroy:
	if (bucket->key && table->key_destroy_func)
	    table->key_destroy_func (bucket->key);
	if (bucket->value && table->value_destroy_func)
	    table->value_destroy_func (bucket->value);
	free (bucket);

	return !ret;
    }
  retry:
    for (prev = &table->buckets[hash % FC_HASH_SIZE];
	 (b = fc_atomic_ptr_get (prev)); prev = &(b->next))
    {
	if (!table->compare_func (b->key, key))
	{
	    if (replace)
	    {
		bucket->next = b->next;
		if (!fc_atomic_ptr_cmpexch (prev, b, bucket))
		    goto retry;
		bucket = b;
	    }
	    else
		ret = FcTrue;
	    goto destroy;
	}
    }
    bucket->next = NULL;
    if (!fc_atomic_ptr_cmpexch (prev, b, bucket))
	goto retry;

    return FcTrue;
}

FcBool
FcHashTableAdd (FcHashTable *table,
		void        *key,
		void        *value)
{
    return FcHashTableAddInternal (table, key, value, FcFalse);
}

FcBool
FcHashTableReplace (FcHashTable *table,
		    void        *key,
		    void        *value)
{
    return FcHashTableAddInternal (table, key, value, FcTrue);
}

FcBool
FcHashTableRemove (FcHashTable *table,
		   void        *key)
{
    FcHashBucket **prev, *bucket;
    FcChar32 hash = table->hash_func (key);
    FcBool ret = FcFalse;

retry:
    for (prev = &table->buckets[hash % FC_HASH_SIZE];
	 (bucket = fc_atomic_ptr_get (prev)); prev = &(bucket->next))
    {
	if (!table->compare_func (bucket->key, key))
	{
	    if (!fc_atomic_ptr_cmpexch (prev, bucket, bucket->next))
		goto retry;
	    if (table->key_destroy_func)
		table->key_destroy_func (bucket->key);
	    if (table->value_destroy_func)
		table->value_destroy_func (bucket->value);
	    free (bucket);
	    ret = FcTrue;
	    break;
	}
    }

    return ret;
}
