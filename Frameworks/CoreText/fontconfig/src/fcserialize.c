/*
 * Copyright © 2006 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

#include "fcint.h"

intptr_t
FcAlignSize (intptr_t size)
{
    intptr_t	rem = size % sizeof (FcAlign);
    if (rem)
	size += sizeof (FcAlign) - rem;
    return size;
}

/*
 * Serialization helper object -- allocate space in the
 * yet-to-be-created linear array for a serialized font set
 */

FcSerialize *
FcSerializeCreate (void)
{
    FcSerialize	*serialize;

    serialize = malloc (sizeof (FcSerialize));
    if (!serialize)
	return NULL;
    serialize->size = 0;
    serialize->linear = NULL;
    serialize->cs_freezer = NULL;
    serialize->buckets = NULL;
    serialize->buckets_count = 0;
    serialize->buckets_used = 0;
    serialize->buckets_used_max = 0;
    return serialize;
}

void
FcSerializeDestroy (FcSerialize *serialize)
{
    free (serialize->buckets);
    if (serialize->cs_freezer)
	FcCharSetFreezerDestroy (serialize->cs_freezer);
    free (serialize);
}

static size_t
FcSerializeNextBucketIndex (const FcSerialize *serialize, size_t index)
{
    if (index == 0)
	index = serialize->buckets_count;
    --index;
    return index;
}

#if ((SIZEOF_VOID_P) * (CHAR_BIT)) == 32

/*
 * Based on triple32
 * https://github.com/skeeto/hash-prospector
 */
static uintptr_t
FcSerializeHashPtr (const void *object)
{
    uintptr_t x = (uintptr_t)object;
    x ^= x >> 17;
    x *= 0xed5ad4bbU;
    x ^= x >> 11;
    x *= 0xac4c1b51U;
    x ^= x >> 15;
    x *= 0x31848babU;
    x ^= x >> 14;
    return x ? x : 1; /* 0 reserved to mark empty, x starts out 0 */
}


#elif ((SIZEOF_VOID_P) * (CHAR_BIT)) == 64

/*
 * Based on splittable64/splitmix64 from Mix13
 * https://zimbry.blogspot.com/2011/09/better-bit-mixing-improving-on.html
 * https://prng.di.unimi.it/splitmix64.c
 */
static uintptr_t
FcSerializeHashPtr (const void *object)
{
    uintptr_t x = (uintptr_t)object;
    x ^= x >> 30;
    x *= 0xbf58476d1ce4e5b9U;
    x ^= x >> 27;
    x *= 0x94d049bb133111ebU;
    x ^= x >> 31;
    return x ? x : 1; /* 0 reserved to mark empty, x starts out 0 */
}

#endif

static FcSerializeBucket*
FcSerializeFind (const FcSerialize *serialize, const void *object)
{
    uintptr_t hash = FcSerializeHashPtr (object);
    size_t buckets_count = serialize->buckets_count;
    size_t index = hash & (buckets_count-1);
    for (size_t n = 0; n < buckets_count; ++n) {
	FcSerializeBucket* bucket = &serialize->buckets[index];
	if (bucket->hash == 0) {
	    return NULL;
	}
	if (object == bucket->object) {
	    return bucket;
	}
	index = FcSerializeNextBucketIndex (serialize, index);
    }
    return NULL;
}

static FcSerializeBucket*
FcSerializeUncheckedSet (FcSerialize *serialize, FcSerializeBucket* insert) {
    const void *object = insert->object;
    size_t buckets_count = serialize->buckets_count;
    size_t index = insert->hash & (buckets_count-1);
    for (size_t n = 0; n < buckets_count; ++n) {
	FcSerializeBucket* bucket = &serialize->buckets[index];
	if (bucket->hash == 0) {
	    *bucket = *insert;
	    ++serialize->buckets_used;
	    return bucket;
	}
	if (object == bucket->object) {
	    /* FcSerializeAlloc should not allow this to happen. */
	    assert (0);
	    *bucket = *insert;
	    return bucket;
	}
	index = FcSerializeNextBucketIndex (serialize, index);
    }
    assert (0);
    return NULL;
}

static FcBool
FcSerializeResize (FcSerialize *serialize, size_t new_count)
{
    size_t old_used = serialize->buckets_used;
    size_t old_count = serialize->buckets_count;
    FcSerializeBucket *old_buckets = serialize->buckets;
    FcSerializeBucket *old_buckets_end = old_buckets + old_count;

    FcSerializeBucket *new_buckets = malloc (new_count * sizeof (*old_buckets));
    if (!new_buckets)
	return FcFalse;
    FcSerializeBucket *new_buckets_end = new_buckets + new_count;
    for (FcSerializeBucket *b = new_buckets; b < new_buckets_end; ++b)
	b->hash = 0;

    serialize->buckets = new_buckets;
    serialize->buckets_count = new_count;
    serialize->buckets_used = 0;
    for (FcSerializeBucket *b = old_buckets; b < old_buckets_end; ++b)
	if (b->hash != 0 && !FcSerializeUncheckedSet (serialize, b))
	{
	    serialize->buckets = old_buckets;
	    serialize->buckets_count = old_count;
	    serialize->buckets_used = old_used;
	    free (new_buckets);
	    return FcFalse;
	}
    free (old_buckets);
    return FcTrue;
}

static FcSerializeBucket*
FcSerializeSet (FcSerialize *serialize, const void *object, intptr_t offset)
{
    if (serialize->buckets_used >= serialize->buckets_used_max)
    {
	size_t capacity = serialize->buckets_count;
	if (capacity == 0)
	    capacity = 4;
	else if (capacity > SIZE_MAX / 2u)
	    return NULL;
	else
	    capacity *= 2;

	if (!FcSerializeResize (serialize, capacity))
	    return NULL;

	serialize->buckets_used_max = capacity / 4u * 3u;
    }

    FcSerializeBucket bucket;
    bucket.object = object;
    bucket.offset = offset;
    bucket.hash = FcSerializeHashPtr (object);
    return FcSerializeUncheckedSet (serialize, &bucket);
}

/*
 * Allocate space for an object in the serialized array. Keep track
 * of where the object is placed and only allocate one copy of each object
 */
FcBool
FcSerializeAlloc (FcSerialize *serialize, const void *object, int size)
{
    FcSerializeBucket *bucket = FcSerializeFind (serialize, object);
    if (bucket)
	return FcTrue;

    if (!FcSerializeSet (serialize, object, serialize->size))
	return FcFalse;

    serialize->size += FcAlignSize (size);
    return FcTrue;
}

/*
 * Reserve space in the serialization array
 */
intptr_t
FcSerializeReserve (FcSerialize *serialize, int size)
{
    intptr_t	offset = serialize->size;
    serialize->size += FcAlignSize (size);
    return offset;
}

/*
 * Given an object, return the offset in the serialized array where
 * the serialized copy of the object is stored
 */
intptr_t
FcSerializeOffset (FcSerialize *serialize, const void *object)
{
    FcSerializeBucket *bucket = FcSerializeFind (serialize, object);
    return bucket ? bucket->offset : 0;
}

/*
 * Given a cache and an object, return a pointer to where
 * the serialized copy of the object is stored
 */
void *
FcSerializePtr (FcSerialize *serialize, const void *object)
{
    intptr_t	offset = FcSerializeOffset (serialize, object);

    if (!offset)
	return NULL;
    return (void *) ((char *) serialize->linear + offset);
}

FcBool
FcStrSerializeAlloc (FcSerialize *serialize, const FcChar8 *str)
{
    return FcSerializeAlloc (serialize, str, strlen ((const char *) str) + 1);
}

FcChar8 *
FcStrSerialize (FcSerialize *serialize, const FcChar8 *str)
{
    FcChar8 *str_serialize = FcSerializePtr (serialize, str);
    if (!str_serialize)
	return NULL;
    strcpy ((char *) str_serialize, (const char *) str);
    return str_serialize;
}
#include "fcaliastail.h"
#undef __fcserialize__
