/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

/*	CFBitVector.c
	Copyright (c) 1998-2014, Apple Inc. All rights reserved.
	Responsibility: Christopher Kane
*/

#include <CoreFoundation/CFBitVector.h>
#include "CFInternal.h"
#include <string.h>

/* The bucket type must be unsigned, at least one byte in size, and
   a power of 2 in number of bits; bits are numbered from 0 from left
   to right (bit 0 is the most significant) */
typedef uint8_t __CFBitVectorBucket;

#define __CFBITVECTORBUCKET_SIZE 8
#define __CF_BITS_PER_BYTE 8

enum {
    __CF_BITS_PER_BYTE_MASK = 0x07
};

enum {
    __CF_BITS_PER_BUCKET = (__CF_BITS_PER_BYTE * sizeof(__CFBitVectorBucket)),
    __CF_BITS_PER_BUCKET_MASK = 0x07    // __CF_BITS_PER_BYTE_MASK * lg2(sizeof(__CFBitVectorBucket))
};

CF_INLINE CFIndex __CFBitVectorRoundUpCapacity(CFIndex capacity) {
    if (0 == capacity) capacity = 1;
    return ((capacity + 63) / 64) * 64;
}

CF_INLINE CFIndex __CFBitVectorNumBucketsForCapacity(CFIndex capacity) {
    return capacity / __CF_BITS_PER_BUCKET + ((capacity & __CF_BITS_PER_BUCKET_MASK) ? 1 : 0);
}

struct __CFBitVector {
    CFRuntimeBase _base;
    CFIndex _count;	/* number of bits */
    CFIndex _capacity;	/* maximum number of bits */
    __CFBitVectorBucket *_buckets;
};

CF_INLINE UInt32 __CFBitVectorMutableVariety(const void *cf) {
    return __CFBitfieldGetValue(((const CFRuntimeBase *)cf)->_cfinfo[CF_INFO_BITS], 3, 2);
}

CF_INLINE void __CFBitVectorSetMutableVariety(void *cf, UInt32 v) {
    __CFBitfieldSetValue(((CFRuntimeBase *)cf)->_cfinfo[CF_INFO_BITS], 3, 2, v);
}

CF_INLINE UInt32 __CFBitVectorMutableVarietyFromFlags(UInt32 flags) {
    return __CFBitfieldGetValue(flags, 1, 0);
}

// ensure that uses of these inlines are correct, bytes vs. buckets vs. bits
CF_INLINE CFIndex __CFBitVectorCount(CFBitVectorRef bv) {
    return bv->_count;
}

CF_INLINE void __CFBitVectorSetCount(CFMutableBitVectorRef bv, CFIndex v) {
    bv->_count = v;
}

CF_INLINE CFIndex __CFBitVectorCapacity(CFBitVectorRef bv) {
    return bv->_capacity;
}

CF_INLINE void __CFBitVectorSetCapacity(CFMutableBitVectorRef bv, CFIndex v) {
    bv->_capacity = v;
}

CF_INLINE void __CFBitVectorSetNumBucketsUsed(CFMutableBitVectorRef bv, CFIndex v) {
    /* for a CFBitVector, _bucketsUsed == _count / __CF_BITS_PER_BUCKET + 1 */
}

CF_INLINE CFIndex __CFBitVectorNumBuckets(CFBitVectorRef bv) {
    return bv->_capacity / __CF_BITS_PER_BUCKET + 1;
}

CF_INLINE void __CFBitVectorSetNumBuckets(CFMutableBitVectorRef bv, CFIndex v) {
    /* for a CFBitVector, _bucketsNum == _capacity / __CF_BITS_PER_BUCKET + 1 */
}

static __CFBitVectorBucket __CFBitBucketMask(CFIndex bottomBit, CFIndex topBit) {
    CFIndex shiftL = __CF_BITS_PER_BUCKET - topBit + bottomBit - 1;
    __CFBitVectorBucket result = ~(__CFBitVectorBucket)0;
    result = (result << shiftL);
    result = (result >> bottomBit);
    return result;
}

CF_INLINE CFBit __CFBitVectorBit(__CFBitVectorBucket *buckets, CFIndex idx) {
    CFIndex bucketIdx = idx / __CF_BITS_PER_BUCKET;
    CFIndex bitOfBucket = idx & (__CF_BITS_PER_BUCKET - 1);
    return (buckets[bucketIdx] >> (__CF_BITS_PER_BUCKET - 1 - bitOfBucket)) & 0x1;
}

CF_INLINE void __CFSetBitVectorBit(__CFBitVectorBucket *buckets, CFIndex idx, CFBit value) {
    CFIndex bucketIdx = idx / __CF_BITS_PER_BUCKET;
    CFIndex bitOfBucket = idx & (__CF_BITS_PER_BUCKET - 1);
    if (value) {
	buckets[bucketIdx] |= (1 << (__CF_BITS_PER_BUCKET - 1 - bitOfBucket));
    } else {
	buckets[bucketIdx] &= ~(1 << (__CF_BITS_PER_BUCKET - 1 - bitOfBucket));
    }
}

CF_INLINE void __CFFlipBitVectorBit(__CFBitVectorBucket *buckets, CFIndex idx) {
    CFIndex bucketIdx = idx / __CF_BITS_PER_BUCKET;
    CFIndex bitOfBucket = idx & (__CF_BITS_PER_BUCKET - 1);
    buckets[bucketIdx] ^= (1 << (__CF_BITS_PER_BUCKET - 1 - bitOfBucket));
}

#if defined(DEBUG)
CF_INLINE void __CFBitVectorValidateRange(CFBitVectorRef bv, CFRange range, const char *func) {
    CFAssert2(0 <= range.location && range.location < __CFBitVectorCount(bv), __kCFLogAssertion, "%s(): range.location index (%d) out of bounds", func, range.location);
    CFAssert2(0 <= range.length, __kCFLogAssertion, "%s(): range.length (%d) cannot be less than zero", func, range.length);
    CFAssert2(range.location + range.length <= __CFBitVectorCount(bv), __kCFLogAssertion, "%s(): ending index (%d) out of bounds", func, range.location + range.length);
}
#else
#define __CFBitVectorValidateRange(bf,r,f)
#endif

static Boolean __CFBitVectorEqual(CFTypeRef cf1, CFTypeRef cf2) {
    CFBitVectorRef bv1 = (CFBitVectorRef)cf1;
    CFBitVectorRef bv2 = (CFBitVectorRef)cf2;
    CFIndex idx, cnt;
    cnt = __CFBitVectorCount(bv1);
    if (cnt != __CFBitVectorCount(bv2)) return false;
    if (0 == cnt) return true;
    for (idx = 0; idx < (cnt / __CF_BITS_PER_BUCKET) + 1; idx++) {
	__CFBitVectorBucket val1 = bv1->_buckets[idx];
	__CFBitVectorBucket val2 = bv2->_buckets[idx];
	if (val1 != val2) return false;
    }
    return true;
}

static CFHashCode __CFBitVectorHash(CFTypeRef cf) {
    CFBitVectorRef bv = (CFBitVectorRef)cf;
    return __CFBitVectorCount(bv);
}

static CFStringRef __CFBitVectorCopyDescription(CFTypeRef cf) {
    CFBitVectorRef bv = (CFBitVectorRef)cf;
    CFMutableStringRef result;
    CFIndex idx, cnt;
    __CFBitVectorBucket *buckets;
    cnt = __CFBitVectorCount(bv);
    buckets = bv->_buckets;
    result = CFStringCreateMutable(kCFAllocatorSystemDefault, 0);
    CFStringAppendFormat(result, NULL, CFSTR("<CFBitVector %p [%p]>{count = %lu, capacity = %lu, objects = (\n"), cf, CFGetAllocator(bv), (unsigned long)cnt, __CFBitVectorCapacity(bv));
    for (idx = 0; idx < (cnt / 64); idx++) {	/* Print groups of 64 */
	CFIndex idx2;
	CFStringAppendFormat(result, NULL, CFSTR("\t%lu : "), (unsigned long)(idx * 64));
	for (idx2 = 0; idx2 < 64; idx2 += 4) {
	    CFIndex bucketIdx = (idx << 6) + idx2;
	    CFStringAppendFormat(result, NULL, CFSTR("%u%u%u%u"),
		(unsigned int)__CFBitVectorBit(buckets, bucketIdx + 0),
		(unsigned int)__CFBitVectorBit(buckets, bucketIdx + 1),
		(unsigned int)__CFBitVectorBit(buckets, bucketIdx + 2),
		(unsigned int)__CFBitVectorBit(buckets, bucketIdx + 3));
	}
	CFStringAppend(result, CFSTR("\n"));
    }
    if (idx * 64 < cnt) {
	CFStringAppendFormat(result, NULL, CFSTR("\t%lu : "), (unsigned long)(idx * 64));
	for (idx = (idx * 64); idx < cnt; idx++) {	/* Print remainder */
	    CFStringAppendFormat(result, NULL, CFSTR("%u"), (unsigned int)__CFBitVectorBit(buckets, idx));
	}
    }
    CFStringAppend(result, CFSTR("\n)}"));
    return result;
}

enum {
    kCFBitVectorImmutable = 0x0,	/* unchangable and fixed capacity; default */
    kCFBitVectorMutable = 0x1,		/* changeable and variable capacity */
};

static void __CFBitVectorDeallocate(CFTypeRef cf) {
    CFMutableBitVectorRef bv = (CFMutableBitVectorRef)cf;
    CFAllocatorRef allocator = CFGetAllocator(bv);
    if (bv->_buckets) _CFAllocatorDeallocateGC(allocator, bv->_buckets);
}

static CFTypeID __kCFBitVectorTypeID = _kCFRuntimeNotATypeID;

static const CFRuntimeClass __CFBitVectorClass = {
    _kCFRuntimeScannedObject,
    "CFBitVector",
    NULL,	// init
    NULL,	// copy
    __CFBitVectorDeallocate,
    __CFBitVectorEqual,
    __CFBitVectorHash,
    NULL,	// 
    __CFBitVectorCopyDescription
};

CFTypeID CFBitVectorGetTypeID(void) {
    static dispatch_once_t initOnce;
    dispatch_once(&initOnce, ^{ __kCFBitVectorTypeID = _CFRuntimeRegisterClass(&__CFBitVectorClass); });
    return __kCFBitVectorTypeID;
}

static CFMutableBitVectorRef __CFBitVectorInit(CFAllocatorRef allocator, CFOptionFlags flags, CFIndex capacity, const uint8_t *bytes, CFIndex numBits) {
    CFMutableBitVectorRef memory;
    CFIndex size;
    CFAssert2(0 <= capacity, __kCFLogAssertion, "%s(): capacity (%d) cannot be less than zero", __PRETTY_FUNCTION__, capacity);
    CFAssert2(0 <= numBits, __kCFLogAssertion, "%s(): numValues (%d) cannot be less than zero", __PRETTY_FUNCTION__, numBits);
    size = sizeof(struct __CFBitVector) - sizeof(CFRuntimeBase);
    memory = (CFMutableBitVectorRef)_CFRuntimeCreateInstance(allocator, CFBitVectorGetTypeID(), size, NULL);
    if (NULL == memory) {
	return NULL;
    }
    __CFBitVectorSetCapacity(memory, __CFBitVectorRoundUpCapacity(numBits));
    __CFBitVectorSetNumBuckets(memory, __CFBitVectorNumBucketsForCapacity(__CFBitVectorRoundUpCapacity(numBits)));
    __CFAssignWithWriteBarrier((void **)&memory->_buckets, _CFAllocatorAllocateGC(allocator, __CFBitVectorNumBuckets(memory) * sizeof(__CFBitVectorBucket), 0));
    if (__CFOASafe) __CFSetLastAllocationEventName(memory->_buckets, "CFBitVector (store)");
    if (NULL == memory->_buckets) {
	CFRelease(memory);
	return NULL;
    }
    memset(memory->_buckets, 0, __CFBitVectorNumBuckets(memory) * sizeof(__CFBitVectorBucket));
    __CFBitVectorSetNumBucketsUsed(memory, numBits / __CF_BITS_PER_BUCKET + 1);
    __CFBitVectorSetCount(memory, numBits);
    if (bytes) {
	/* This move is possible because bits are numbered from 0 on the left */
	memmove(memory->_buckets, bytes, numBits / __CF_BITS_PER_BYTE + (numBits & __CF_BITS_PER_BYTE_MASK ? 1 : 0));
    }
    __CFBitVectorSetMutableVariety(memory, __CFBitVectorMutableVarietyFromFlags(flags));
    return memory;
}

CFBitVectorRef CFBitVectorCreate(CFAllocatorRef allocator, const uint8_t *bytes, CFIndex numBits) {
   return __CFBitVectorInit(allocator, kCFBitVectorImmutable, numBits, bytes, numBits);
}

CFMutableBitVectorRef CFBitVectorCreateMutable(CFAllocatorRef allocator, CFIndex capacity) {
   return __CFBitVectorInit(allocator, kCFBitVectorMutable, capacity, NULL, 0);
}

CFBitVectorRef CFBitVectorCreateCopy(CFAllocatorRef allocator, CFBitVectorRef bv) {
   __CFGenericValidateType(bv, CFBitVectorGetTypeID());
    return __CFBitVectorInit(allocator, kCFBitVectorImmutable, __CFBitVectorCount(bv), (const uint8_t *)bv->_buckets, __CFBitVectorCount(bv));
}

CFMutableBitVectorRef CFBitVectorCreateMutableCopy(CFAllocatorRef allocator, CFIndex capacity, CFBitVectorRef bv) {
   __CFGenericValidateType(bv, CFBitVectorGetTypeID());
    return __CFBitVectorInit(allocator, kCFBitVectorMutable, capacity, (const uint8_t *)bv->_buckets, __CFBitVectorCount(bv));
}

CFIndex CFBitVectorGetCount(CFBitVectorRef bv) {
    __CFGenericValidateType(bv, CFBitVectorGetTypeID());
    return __CFBitVectorCount(bv);
}

typedef __CFBitVectorBucket (*__CFInternalMapper)(__CFBitVectorBucket bucketValue, __CFBitVectorBucket bucketValueMask, void *context);

static void __CFBitVectorInternalMap(CFMutableBitVectorRef bv, CFRange range, __CFInternalMapper mapper, void *context) {
    CFIndex bucketIdx, bitOfBucket;
    CFIndex nBuckets;
    __CFBitVectorBucket bucketValMask, newBucketVal;
    if (0 == range.length) return;
    bucketIdx = range.location / __CF_BITS_PER_BUCKET;
    bitOfBucket = range.location & (__CF_BITS_PER_BUCKET - 1);
    /* Follow usual pattern of ramping up to a bit bucket boundary ...*/
    if (bitOfBucket + range.length < __CF_BITS_PER_BUCKET) {
	bucketValMask = __CFBitBucketMask(bitOfBucket, bitOfBucket + range.length - 1);
	range.length = 0;
    } else {
	bucketValMask = __CFBitBucketMask(bitOfBucket, __CF_BITS_PER_BUCKET - 1);
	range.length -= __CF_BITS_PER_BUCKET - bitOfBucket;
    }
    newBucketVal = mapper(bv->_buckets[bucketIdx], bucketValMask, context);
    bv->_buckets[bucketIdx] = (bv->_buckets[bucketIdx] & ~bucketValMask) | (newBucketVal & bucketValMask);
    bucketIdx++;
    /* ... clipping along with entire bit buckets ... */
    nBuckets = range.length / __CF_BITS_PER_BUCKET;
    range.length -= nBuckets * __CF_BITS_PER_BUCKET;
    while (nBuckets--) {
	newBucketVal = mapper(bv->_buckets[bucketIdx], ~0, context);
	bv->_buckets[bucketIdx] = newBucketVal;
	bucketIdx++;
    }
    /* ... and ramping down with the last fragmentary bit bucket. */
    if (0 != range.length) {
	bucketValMask = __CFBitBucketMask(0, range.length - 1);
	newBucketVal = mapper(bv->_buckets[bucketIdx], bucketValMask, context);
	bv->_buckets[bucketIdx] = (bv->_buckets[bucketIdx] & ~bucketValMask) | (newBucketVal & bucketValMask);
    }
}

struct _occursContext {
    CFBit value;
    CFIndex count;
};

static __CFBitVectorBucket __CFBitVectorCountBits(__CFBitVectorBucket bucketValue, __CFBitVectorBucket bucketValueMask, struct _occursContext *context) {
    static const __CFBitVectorBucket __CFNibbleBitCount[16] = {0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};
    __CFBitVectorBucket val;
    CFIndex idx;
    val = (context->value) ? (bucketValue & bucketValueMask) : (~bucketValue & bucketValueMask);
    for (idx = 0; idx < (CFIndex)sizeof(__CFBitVectorBucket) * 2; idx++) {
	context->count += __CFNibbleBitCount[val & 0xF];
	val = val >> 4;
    }
    return bucketValue;
}

CFIndex CFBitVectorGetCountOfBit(CFBitVectorRef bv, CFRange range, CFBit value) {
    struct _occursContext context;
    __CFGenericValidateType(bv, CFBitVectorGetTypeID());
    __CFBitVectorValidateRange(bv, range, __PRETTY_FUNCTION__);
    if (0 == range.length) return 0;
    context.value = value;
    context.count = 0;
    __CFBitVectorInternalMap((CFMutableBitVectorRef)bv, range, (__CFInternalMapper)__CFBitVectorCountBits, &context);
    return context.count;
}

Boolean CFBitVectorContainsBit(CFBitVectorRef bv, CFRange range, CFBit value) {
    __CFGenericValidateType(bv, CFBitVectorGetTypeID());
    __CFBitVectorValidateRange(bv, range, __PRETTY_FUNCTION__);
    return (CFBitVectorGetCountOfBit(bv, range, value) != 0) ? true : false;
}

CFBit CFBitVectorGetBitAtIndex(CFBitVectorRef bv, CFIndex idx) {
    __CFGenericValidateType(bv, CFBitVectorGetTypeID());
    CFAssert2(0 <= idx && idx < __CFBitVectorCount(bv), __kCFLogAssertion, "%s(): index (%d) out of bounds", __PRETTY_FUNCTION__, idx);
    return __CFBitVectorBit(bv->_buckets, idx);
}

struct _getBitsContext {
    uint8_t *curByte;
    CFIndex initBits;	/* Bits to extract off the front for the prev. byte */
    CFIndex totalBits;	/* This is for stopping at the end */
    bool ignoreFirstInitBits;
};

static __CFBitVectorBucket __CFBitVectorGetBits(__CFBitVectorBucket bucketValue, __CFBitVectorBucket bucketValueMask, void *ctx) {
    struct _getBitsContext *context = (struct _getBitsContext *)ctx;
    __CFBitVectorBucket val;
    CFIndex nBits;
    val = bucketValue & bucketValueMask;
    nBits = __CFMin(__CF_BITS_PER_BUCKET - context->initBits, context->totalBits);
    /* First initBits bits go in *curByte ... */
    if (0 < context->initBits) {
	if (!context->ignoreFirstInitBits) {
	    *context->curByte |= (uint8_t)(val >> (__CF_BITS_PER_BUCKET - context->initBits));
	    context->curByte++;
	    context->totalBits -= context->initBits;
	    context->ignoreFirstInitBits = false;
	}
	val <<= context->initBits;
    }
    /* ... then next groups of __CF_BITS_PER_BYTE go in *curByte ... */
    while (__CF_BITS_PER_BYTE <= nBits) {
	*context->curByte = (uint8_t)(val >> (__CF_BITS_PER_BUCKET - __CF_BITS_PER_BYTE));
	context->curByte++;
	context->totalBits -= context->initBits;
	nBits -= __CF_BITS_PER_BYTE;
#if __CFBITVECTORBUCKET_SIZE > __CF_BITS_PER_BYTE
	val <<= __CF_BITS_PER_BYTE;
#else
        val = 0;
#endif
    }
    /* ... then remaining bits go in *curByte */
    if (0 < nBits) {
	*context->curByte = (uint8_t)(val >> (__CF_BITS_PER_BUCKET - __CF_BITS_PER_BYTE));
	context->totalBits -= nBits;
    }
    return bucketValue;
}

void CFBitVectorGetBits(CFBitVectorRef bv, CFRange range, uint8_t *bytes) {
    struct _getBitsContext context;
    __CFGenericValidateType(bv, CFBitVectorGetTypeID());
    __CFBitVectorValidateRange(bv, range, __PRETTY_FUNCTION__);
    if (0 == range.length) return;
    context.curByte = bytes;
    context.initBits = range.location & (__CF_BITS_PER_BUCKET - 1);
    context.totalBits = range.length;
    context.ignoreFirstInitBits = true;
    __CFBitVectorInternalMap((CFMutableBitVectorRef)bv, range, __CFBitVectorGetBits, &context);
}

CFIndex CFBitVectorGetFirstIndexOfBit(CFBitVectorRef bv, CFRange range, CFBit value) {
    CFIndex idx;
    __CFGenericValidateType(bv, CFBitVectorGetTypeID());
    __CFBitVectorValidateRange(bv, range, __PRETTY_FUNCTION__);
    for (idx = 0; idx < range.length; idx++) {
	if (value == CFBitVectorGetBitAtIndex(bv, range.location + idx)) {
	    return range.location + idx;
	}
    }
    return kCFNotFound;
}

CFIndex CFBitVectorGetLastIndexOfBit(CFBitVectorRef bv, CFRange range, CFBit value) {
    CFIndex idx;
    __CFGenericValidateType(bv, CFBitVectorGetTypeID());
    __CFBitVectorValidateRange(bv, range, __PRETTY_FUNCTION__);
    for (idx = range.length; idx--;) {
	if (value == CFBitVectorGetBitAtIndex(bv, range.location + idx)) {
	    return range.location + idx;
	}
    }
    return kCFNotFound;
}

static void __CFBitVectorGrow(CFMutableBitVectorRef bv, CFIndex numNewValues) {
    CFIndex oldCount = __CFBitVectorCount(bv);
    CFIndex capacity = __CFBitVectorRoundUpCapacity(oldCount + numNewValues);
    CFAllocatorRef allocator = CFGetAllocator(bv);
    __CFBitVectorSetCapacity(bv, capacity);
    __CFBitVectorSetNumBuckets(bv, __CFBitVectorNumBucketsForCapacity(capacity));
    __CFAssignWithWriteBarrier((void **)&bv->_buckets, CFAllocatorReallocate(allocator, bv->_buckets, __CFBitVectorNumBuckets(bv) * sizeof(__CFBitVectorBucket), 0));
    if (__CFOASafe) __CFSetLastAllocationEventName(bv->_buckets, "CFBitVector (store)");
    if (NULL == bv->_buckets) HALT;
}

static __CFBitVectorBucket __CFBitVectorZeroBits(__CFBitVectorBucket bucketValue, __CFBitVectorBucket bucketValueMask, void *context) {
    return 0;
}

static __CFBitVectorBucket __CFBitVectorOneBits(__CFBitVectorBucket bucketValue, __CFBitVectorBucket bucketValueMask, void *context) {
    return ~(__CFBitVectorBucket)0;
}

void CFBitVectorSetCount(CFMutableBitVectorRef bv, CFIndex count) {
    CFIndex cnt;
    CFAssert1(__CFBitVectorMutableVariety(bv) == kCFBitVectorMutable, __kCFLogAssertion, "%s(): bit vector is immutable", __PRETTY_FUNCTION__);
    cnt = __CFBitVectorCount(bv);
    switch (__CFBitVectorMutableVariety(bv)) {
    case kCFBitVectorMutable:
	if (cnt < count) {
	    __CFBitVectorGrow(bv, count - cnt);
	}
	break;
    }
    if (cnt < count) {
	CFRange range = CFRangeMake(cnt, count - cnt);
        __CFBitVectorInternalMap(bv, range, __CFBitVectorZeroBits, NULL);
    }
    __CFBitVectorSetNumBucketsUsed(bv, count / __CF_BITS_PER_BUCKET + 1);
    __CFBitVectorSetCount(bv, count);
}

void CFBitVectorFlipBitAtIndex(CFMutableBitVectorRef bv, CFIndex idx) {
    __CFGenericValidateType(bv, CFBitVectorGetTypeID());
    CFAssert2(0 <= idx && idx < __CFBitVectorCount(bv), __kCFLogAssertion, "%s(): index (%d) out of bounds", __PRETTY_FUNCTION__, idx);
    CFAssert1(__CFBitVectorMutableVariety(bv) == kCFBitVectorMutable, __kCFLogAssertion, "%s(): bit vector is immutable", __PRETTY_FUNCTION__);
    __CFFlipBitVectorBit(bv->_buckets, idx);
}

static __CFBitVectorBucket __CFBitVectorFlipBits(__CFBitVectorBucket bucketValue, __CFBitVectorBucket bucketValueMask, void *context) {
    return (~(__CFBitVectorBucket)0) ^ bucketValue;
}

void CFBitVectorFlipBits(CFMutableBitVectorRef bv, CFRange range) {
    __CFGenericValidateType(bv, CFBitVectorGetTypeID());
    __CFBitVectorValidateRange(bv, range, __PRETTY_FUNCTION__);
    CFAssert1(__CFBitVectorMutableVariety(bv) == kCFBitVectorMutable, __kCFLogAssertion, "%s(): bit vector is immutable", __PRETTY_FUNCTION__);
    if (0 == range.length) return;
    __CFBitVectorInternalMap(bv, range, __CFBitVectorFlipBits, NULL);
}

void CFBitVectorSetBitAtIndex(CFMutableBitVectorRef bv, CFIndex idx, CFBit value) {
    __CFGenericValidateType(bv, CFBitVectorGetTypeID());
    CFAssert2(0 <= idx && idx < __CFBitVectorCount(bv), __kCFLogAssertion, "%s(): index (%d) out of bounds", __PRETTY_FUNCTION__, idx);
    CFAssert1(__CFBitVectorMutableVariety(bv) == kCFBitVectorMutable, __kCFLogAssertion, "%s(): bit vector is immutable", __PRETTY_FUNCTION__);
    __CFSetBitVectorBit(bv->_buckets, idx, value);
}

void CFBitVectorSetBits(CFMutableBitVectorRef bv, CFRange range, CFBit value) {
    __CFGenericValidateType(bv, CFBitVectorGetTypeID());
    __CFBitVectorValidateRange(bv, range, __PRETTY_FUNCTION__);
    CFAssert1(__CFBitVectorMutableVariety(bv) == kCFBitVectorMutable , __kCFLogAssertion, "%s(): bit vector is immutable", __PRETTY_FUNCTION__);
    if (0 == range.length) return;
    if (value) {
	__CFBitVectorInternalMap(bv, range, __CFBitVectorOneBits, NULL);
    } else {
	__CFBitVectorInternalMap(bv, range, __CFBitVectorZeroBits, NULL);
    }
}

void CFBitVectorSetAllBits(CFMutableBitVectorRef bv, CFBit value) {
    CFIndex nBuckets, leftover;
    __CFGenericValidateType(bv, CFBitVectorGetTypeID());
    CFAssert1(__CFBitVectorMutableVariety(bv) == kCFBitVectorMutable , __kCFLogAssertion, "%s(): bit vector is immutable", __PRETTY_FUNCTION__);
    nBuckets = __CFBitVectorCount(bv) / __CF_BITS_PER_BUCKET;
    leftover = __CFBitVectorCount(bv) - nBuckets * __CF_BITS_PER_BUCKET;
    if (0 < leftover) {
	CFRange range = CFRangeMake(nBuckets * __CF_BITS_PER_BUCKET, leftover);
	if (value) {
	    __CFBitVectorInternalMap(bv, range, __CFBitVectorOneBits, NULL);
	} else {
	    __CFBitVectorInternalMap(bv, range, __CFBitVectorZeroBits, NULL);
	}
    }
    memset(bv->_buckets, (value ? ~0 : 0), nBuckets);
}

#undef __CFBitVectorValidateRange

