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

/*	CFBinaryHeap.c
	Copyright (c) 1998-2014, Apple Inc. All rights reserved.
	Responsibility: Christopher Kane
*/

#include <CoreFoundation/CFBinaryHeap.h>
#include <CoreFoundation/CFPriv.h>
#include "CFInternal.h"

const CFBinaryHeapCallBacks kCFStringBinaryHeapCallBacks = {0, __CFTypeCollectionRetain, __CFTypeCollectionRelease, CFCopyDescription, (CFComparisonResult (*)(const void *, const void *, void *))CFStringCompare};

struct __CFBinaryHeapBucket {
    void *_item;
};

CF_INLINE CFIndex __CFBinaryHeapRoundUpCapacity(CFIndex capacity) {
    if (capacity < 4) return 4;
    return (1 << flsl(capacity));
}

CF_INLINE CFIndex __CFBinaryHeapNumBucketsForCapacity(CFIndex capacity) {
    return capacity;
}

struct __CFBinaryHeap {
    CFRuntimeBase _base;
    CFIndex _count;	/* number of objects */
    CFIndex _capacity;	/* maximum number of objects */
    CFBinaryHeapCallBacks _callbacks;
    CFBinaryHeapCompareContext _context;
    struct __CFBinaryHeapBucket *_buckets;
};

CF_INLINE CFIndex __CFBinaryHeapCount(CFBinaryHeapRef heap) {
    return heap->_count;
}

CF_INLINE void __CFBinaryHeapSetCount(CFBinaryHeapRef heap, CFIndex v) {
    /* for a CFBinaryHeap, _bucketsUsed == _count */
}

CF_INLINE CFIndex __CFBinaryHeapCapacity(CFBinaryHeapRef heap) {
    return heap->_capacity;
}

CF_INLINE void __CFBinaryHeapSetCapacity(CFBinaryHeapRef heap, CFIndex v) {
    /* for a CFBinaryHeap, _bucketsNum == _capacity */
}

CF_INLINE CFIndex __CFBinaryHeapNumBucketsUsed(CFBinaryHeapRef heap) {
    return heap->_count;
}

CF_INLINE void __CFBinaryHeapSetNumBucketsUsed(CFBinaryHeapRef heap, CFIndex v) {
    heap->_count = v;
}

CF_INLINE CFIndex __CFBinaryHeapNumBuckets(CFBinaryHeapRef heap) {
    return heap->_capacity;
}

CF_INLINE void __CFBinaryHeapSetNumBuckets(CFBinaryHeapRef heap, CFIndex v) {
    heap->_capacity = v;
}

enum {      /* bits 1-0 */
    kCFBinaryHeapMutable = 0x1,		/* changeable and variable capacity */
};

/* Bits 4-5 are used by GC */

CF_INLINE bool isStrongMemory_Heap(CFTypeRef collection) {
    return __CFBitfieldGetValue(((const CFRuntimeBase *)collection)->_cfinfo[CF_INFO_BITS], 4, 4) == 0;
}

CF_INLINE UInt32 __CFBinaryHeapMutableVariety(const void *cf) {
    return __CFBitfieldGetValue(((const CFRuntimeBase *)cf)->_cfinfo[CF_INFO_BITS], 3, 2);
}

CF_INLINE void __CFBinaryHeapSetMutableVariety(void *cf, UInt32 v) {
    __CFBitfieldSetValue(((CFRuntimeBase *)cf)->_cfinfo[CF_INFO_BITS], 3, 2, v);
}

CF_INLINE UInt32 __CFBinaryHeapMutableVarietyFromFlags(UInt32 flags) {
    return __CFBitfieldGetValue(flags, 1, 0);
}

static Boolean __CFBinaryHeapEqual(CFTypeRef cf1, CFTypeRef cf2) {
    CFBinaryHeapRef heap1 = (CFBinaryHeapRef)cf1;
    CFBinaryHeapRef heap2 = (CFBinaryHeapRef)cf2;
    CFComparisonResult (*compare)(const void *, const void *, void *);
    CFIndex idx;
    CFIndex cnt;
    const void **list1, **list2, *buffer[256];
    cnt = __CFBinaryHeapCount(heap1);
    if (cnt != __CFBinaryHeapCount(heap2)) return false;
    compare = heap1->_callbacks.compare;
    if (compare != heap2->_callbacks.compare) return false;
    if (0 == cnt) return true;	/* after function comparison */
    list1 = (cnt <= 128) ? (const void **)buffer : (const void **)CFAllocatorAllocate(kCFAllocatorSystemDefault, 2 * cnt * sizeof(void *), 0); // GC OK
    if (__CFOASafe && list1 != buffer) __CFSetLastAllocationEventName(list1, "CFBinaryHeap (temp)");
    list2 = (cnt <= 128) ? buffer + 128 : list1 + cnt;
    CFBinaryHeapGetValues(heap1, list1);
    CFBinaryHeapGetValues(heap2, list2);
    for (idx = 0; idx < cnt; idx++) {
	const void *val1 = list1[idx];
	const void *val2 = list2[idx];
// CF: which context info should be passed in? both?
// CF: if the context infos are not equal, should the heaps not be equal?
        if (val1 != val2) { 
            if (NULL == compare) return false;
            if (!compare(val1, val2, heap1->_context.info)) return false;
        }
    }
    if (list1 != buffer) CFAllocatorDeallocate(CFGetAllocator(heap1), list1); // GC OK
    return true;
}

static CFHashCode __CFBinaryHeapHash(CFTypeRef cf) {
    CFBinaryHeapRef heap = (CFBinaryHeapRef)cf;
    return __CFBinaryHeapCount(heap);
}

static CFStringRef __CFBinaryHeapCopyDescription(CFTypeRef cf) {
    CFBinaryHeapRef heap = (CFBinaryHeapRef)cf;
    CFMutableStringRef result;
    CFIndex idx;
    CFIndex cnt;
    const void **list, *buffer[256];
    cnt = __CFBinaryHeapCount(heap);
    result = CFStringCreateMutable(CFGetAllocator(heap), 0);
    CFStringAppendFormat(result, NULL, CFSTR("<CFBinaryHeap %p [%p]>{count = %lu, capacity = %lu, objects = (\n"), cf, CFGetAllocator(heap), (unsigned long)cnt, (unsigned long)__CFBinaryHeapCapacity(heap));
    list = (cnt <= 128) ? (const void **)buffer : (const void **)CFAllocatorAllocate(kCFAllocatorSystemDefault, cnt * sizeof(void *), 0); // GC OK
    if (__CFOASafe && list != buffer) __CFSetLastAllocationEventName(list, "CFBinaryHeap (temp)");
    CFBinaryHeapGetValues(heap, list);
    for (idx = 0; idx < cnt; idx++) {
	CFStringRef desc = NULL;
	const void *item = list[idx];
	if (NULL != heap->_callbacks.copyDescription) {
	    desc = heap->_callbacks.copyDescription(item);
	}
	if (NULL != desc) {
	    CFStringAppendFormat(result, NULL, CFSTR("\t%lu : %@\n"), (unsigned long)idx, desc);
	    CFRelease(desc);
	} else {
	    CFStringAppendFormat(result, NULL, CFSTR("\t%lu : <%p>\n"), (unsigned long)idx, item);
	}
    }
    CFStringAppend(result, CFSTR(")}"));
    if (list != buffer) CFAllocatorDeallocate(CFGetAllocator(heap), list); // GC OK
    return result;
}

static void __CFBinaryHeapDeallocate(CFTypeRef cf) {
    CFBinaryHeapRef heap = (CFBinaryHeapRef)cf;
    CFAllocatorRef allocator = CFGetAllocator(heap);
    if (CF_IS_COLLECTABLE_ALLOCATOR(allocator)) {
	if (heap->_callbacks.retain == NULL && heap->_callbacks.release == NULL)
	    return; // GC: keep heap intact during finalization.
    }
// CF: should make the heap mutable here first, a la CFArrayDeallocate
    CFBinaryHeapRemoveAllValues(heap);
// CF: does not release the context info
    if (__CFBinaryHeapMutableVariety(heap) == kCFBinaryHeapMutable) {
	_CFAllocatorDeallocateGC(allocator, heap->_buckets);
    }
}

static CFTypeID __kCFBinaryHeapTypeID = _kCFRuntimeNotATypeID;

static const CFRuntimeClass __CFBinaryHeapClass = {
    _kCFRuntimeScannedObject,
    "CFBinaryHeap",
    NULL,	// init
    NULL,	// copy
    __CFBinaryHeapDeallocate,
    __CFBinaryHeapEqual,
    __CFBinaryHeapHash,
    NULL,	// 
    __CFBinaryHeapCopyDescription
};

CFTypeID CFBinaryHeapGetTypeID(void) {
    static dispatch_once_t initOnce;
    dispatch_once(&initOnce, ^{ __kCFBinaryHeapTypeID = _CFRuntimeRegisterClass(&__CFBinaryHeapClass); });
    return __kCFBinaryHeapTypeID;
}

static CFBinaryHeapRef __CFBinaryHeapInit(CFAllocatorRef allocator, UInt32 flags, CFIndex capacity, const void **values, CFIndex numValues, const CFBinaryHeapCallBacks *callBacks, const CFBinaryHeapCompareContext *compareContext) {
    CFBinaryHeapRef memory;
    CFIndex idx;
    CFIndex size;

    CFAssert2(0 <= capacity, __kCFLogAssertion, "%s(): capacity (%d) cannot be less than zero", __PRETTY_FUNCTION__, capacity);
    CFAssert2(0 <= numValues, __kCFLogAssertion, "%s(): numValues (%d) cannot be less than zero", __PRETTY_FUNCTION__, numValues);
    size = sizeof(struct __CFBinaryHeap) - sizeof(CFRuntimeBase);
    if (CF_IS_COLLECTABLE_ALLOCATOR(allocator)) {
	if (!callBacks || (callBacks->retain == NULL && callBacks->release == NULL)) {
	    __CFBitfieldSetValue(flags, 4, 4, 1); // setWeak
	}
    }

    memory = (CFBinaryHeapRef)_CFRuntimeCreateInstance(allocator, CFBinaryHeapGetTypeID(), size, NULL);
    if (NULL == memory) {
	return NULL;
    }
	__CFBinaryHeapSetCapacity(memory, __CFBinaryHeapRoundUpCapacity(1));
	__CFBinaryHeapSetNumBuckets(memory, __CFBinaryHeapNumBucketsForCapacity(__CFBinaryHeapRoundUpCapacity(1)));
	void *buckets = _CFAllocatorAllocateGC(allocator, __CFBinaryHeapNumBuckets(memory) * sizeof(struct __CFBinaryHeapBucket), isStrongMemory_Heap(memory) ? __kCFAllocatorGCScannedMemory : 0);
	__CFAssignWithWriteBarrier((void **)&memory->_buckets, buckets);
	if (__CFOASafe) __CFSetLastAllocationEventName(memory->_buckets, "CFBinaryHeap (store)");
	if (NULL == memory->_buckets) {
	    CFRelease(memory);
	    return NULL;
	}
    __CFBinaryHeapSetNumBucketsUsed(memory, 0);
    __CFBinaryHeapSetCount(memory, 0);
    if (NULL != callBacks) {
	memory->_callbacks.retain = callBacks->retain;
	memory->_callbacks.release = callBacks->release;
	memory->_callbacks.copyDescription = callBacks->copyDescription;
	memory->_callbacks.compare = callBacks->compare;
    } else {
	memory->_callbacks.retain = 0;
	memory->_callbacks.release = 0;
	memory->_callbacks.copyDescription = 0;
	memory->_callbacks.compare = 0;
    }
    if (compareContext) memcpy(&memory->_context, compareContext, sizeof(CFBinaryHeapCompareContext));
// CF: retain info for proper operation
    __CFBinaryHeapSetMutableVariety(memory, kCFBinaryHeapMutable);
    for (idx = 0; idx < numValues; idx++) {
	CFBinaryHeapAddValue(memory, values[idx]);
    }
    __CFBinaryHeapSetMutableVariety(memory, __CFBinaryHeapMutableVarietyFromFlags(flags));
    return memory;
}

CFBinaryHeapRef CFBinaryHeapCreate(CFAllocatorRef allocator, CFIndex capacity, const CFBinaryHeapCallBacks *callBacks, const CFBinaryHeapCompareContext *compareContext) {
   return __CFBinaryHeapInit(allocator, kCFBinaryHeapMutable, capacity, NULL, 0, callBacks, compareContext);
}

CFBinaryHeapRef CFBinaryHeapCreateCopy(CFAllocatorRef allocator, CFIndex capacity, CFBinaryHeapRef heap) {
   __CFGenericValidateType(heap, CFBinaryHeapGetTypeID());
    return __CFBinaryHeapInit(allocator, kCFBinaryHeapMutable, capacity, (const void **)heap->_buckets, __CFBinaryHeapCount(heap), &(heap->_callbacks), &(heap->_context));
}

CFIndex CFBinaryHeapGetCount(CFBinaryHeapRef heap) {
    __CFGenericValidateType(heap, CFBinaryHeapGetTypeID());
    return __CFBinaryHeapCount(heap);
}

CFIndex CFBinaryHeapGetCountOfValue(CFBinaryHeapRef heap, const void *value) {
    CFComparisonResult (*compare)(const void *, const void *, void *);
    CFIndex idx;
    CFIndex cnt = 0, length;
    __CFGenericValidateType(heap, CFBinaryHeapGetTypeID());
    compare = heap->_callbacks.compare;
    length = __CFBinaryHeapCount(heap);
    for (idx = 0; idx < length; idx++) {
	const void *item = heap->_buckets[idx]._item;
	if (value == item || (compare && kCFCompareEqualTo == compare(value, item, heap->_context.info))) {
	    cnt++;
	}
    }
    return cnt;
}

Boolean CFBinaryHeapContainsValue(CFBinaryHeapRef heap, const void *value) {
    CFComparisonResult (*compare)(const void *, const void *, void *);
    CFIndex idx;
    CFIndex length;
    __CFGenericValidateType(heap, CFBinaryHeapGetTypeID());
    compare = heap->_callbacks.compare;
    length = __CFBinaryHeapCount(heap);
    for (idx = 0; idx < length; idx++) {
	const void *item = heap->_buckets[idx]._item;
	if (value == item || (compare && kCFCompareEqualTo == compare(value, item, heap->_context.info))) {
	    return true;
	}
    }
    return false;
}

const void *CFBinaryHeapGetMinimum(CFBinaryHeapRef heap) {
    __CFGenericValidateType(heap, CFBinaryHeapGetTypeID());
    CFAssert1(0 < __CFBinaryHeapCount(heap), __kCFLogAssertion, "%s(): binary heap is empty", __PRETTY_FUNCTION__);
    return (0 < __CFBinaryHeapCount(heap)) ? heap->_buckets[0]._item : NULL;
}

Boolean CFBinaryHeapGetMinimumIfPresent(CFBinaryHeapRef heap, const void **value) {
    __CFGenericValidateType(heap, CFBinaryHeapGetTypeID());
    if (0 == __CFBinaryHeapCount(heap)) return false;
    if (NULL != value) __CFAssignWithWriteBarrier((void **)value, heap->_buckets[0]._item);
    return true;
}

void CFBinaryHeapGetValues(CFBinaryHeapRef heap, const void **values) {
    CFBinaryHeapRef heapCopy;
    CFIndex idx;
    CFIndex cnt;
    __CFGenericValidateType(heap, CFBinaryHeapGetTypeID());
    CFAssert1(NULL != values, __kCFLogAssertion, "%s(): pointer to values may not be NULL", __PRETTY_FUNCTION__);
    cnt = __CFBinaryHeapCount(heap);
    if (0 == cnt) return;
    heapCopy = CFBinaryHeapCreateCopy(CFGetAllocator(heap), cnt, heap);
    idx = 0;
    while (0 < __CFBinaryHeapCount(heapCopy)) {
	const void *value = CFBinaryHeapGetMinimum(heapCopy);
	CFBinaryHeapRemoveMinimumValue(heapCopy);
	values[idx++] = value;
    }
    CFRelease(heapCopy);
}

void CFBinaryHeapApplyFunction(CFBinaryHeapRef heap, CFBinaryHeapApplierFunction applier, void *context) {
    CFBinaryHeapRef heapCopy;
    CFIndex cnt;
    __CFGenericValidateType(heap, CFBinaryHeapGetTypeID());
    CFAssert1(NULL != applier, __kCFLogAssertion, "%s(): pointer to applier function may not be NULL", __PRETTY_FUNCTION__);
    cnt = __CFBinaryHeapCount(heap);
    if (0 == cnt) return;
    heapCopy = CFBinaryHeapCreateCopy(CFGetAllocator(heap), cnt, heap);
    while (0 < __CFBinaryHeapCount(heapCopy)) {
	const void *value = CFBinaryHeapGetMinimum(heapCopy);
	CFBinaryHeapRemoveMinimumValue(heapCopy);
	applier(value, context);
    }
    CFRelease(heapCopy);
}

static void __CFBinaryHeapGrow(CFBinaryHeapRef heap, CFIndex numNewValues) {
    CFIndex oldCount = __CFBinaryHeapCount(heap);
    CFIndex capacity = __CFBinaryHeapRoundUpCapacity(oldCount + numNewValues);
    CFAllocatorRef allocator = CFGetAllocator(heap);
    __CFBinaryHeapSetCapacity(heap, capacity);
    __CFBinaryHeapSetNumBuckets(heap, __CFBinaryHeapNumBucketsForCapacity(capacity));
    void *buckets = _CFAllocatorReallocateGC(allocator, heap->_buckets, __CFBinaryHeapNumBuckets(heap) * sizeof(struct __CFBinaryHeapBucket), isStrongMemory_Heap(heap) ? __kCFAllocatorGCScannedMemory : 0);
    __CFAssignWithWriteBarrier((void **)&heap->_buckets, buckets);
    if (__CFOASafe) __CFSetLastAllocationEventName(heap->_buckets, "CFBinaryHeap (store)");
    if (NULL == heap->_buckets) HALT;
}

void CFBinaryHeapAddValue(CFBinaryHeapRef heap, const void *value) {
    CFIndex idx, pidx;
    CFIndex cnt;
    CFAllocatorRef allocator = CFGetAllocator(heap);
    __CFGenericValidateType(heap, CFBinaryHeapGetTypeID());
    switch (__CFBinaryHeapMutableVariety(heap)) {
    case kCFBinaryHeapMutable:
	if (__CFBinaryHeapNumBucketsUsed(heap) == __CFBinaryHeapCapacity(heap))
	    __CFBinaryHeapGrow(heap, 1);
	break;
    }
    cnt = __CFBinaryHeapCount(heap);
    idx = cnt;
    __CFBinaryHeapSetNumBucketsUsed(heap, cnt + 1);
    __CFBinaryHeapSetCount(heap, cnt + 1);
    CFComparisonResult (*compare)(const void *, const void *, void *) = heap->_callbacks.compare;
    pidx = (idx - 1) >> 1;
    while (0 < idx) {
	void *item = heap->_buckets[pidx]._item;
	if ((!compare && item <= value) || (compare && kCFCompareGreaterThan != compare(item, value, heap->_context.info))) break;
	__CFAssignWithWriteBarrier((void **)&heap->_buckets[idx]._item, item);
	idx = pidx;
	pidx = (idx - 1) >> 1;
    }
    if (heap->_callbacks.retain) {
	__CFAssignWithWriteBarrier((void **)&heap->_buckets[idx]._item, (void *)heap->_callbacks.retain(allocator, (void *)value));
    } else {
	__CFAssignWithWriteBarrier((void **)&heap->_buckets[idx]._item, (void *)value);
    }
}

void CFBinaryHeapRemoveMinimumValue(CFBinaryHeapRef heap) {
    void *val;
    CFIndex idx, cidx;
    CFIndex cnt;
    CFAllocatorRef allocator;
    __CFGenericValidateType(heap, CFBinaryHeapGetTypeID());
    cnt = __CFBinaryHeapCount(heap);
    if (0 == cnt) return;
    idx = 0;
    __CFBinaryHeapSetNumBucketsUsed(heap, cnt - 1);
    __CFBinaryHeapSetCount(heap, cnt - 1);
    CFComparisonResult (*compare)(const void *, const void *, void *) = heap->_callbacks.compare;
    allocator = CFGetAllocator(heap);
    if (heap->_callbacks.release)
	heap->_callbacks.release(allocator, heap->_buckets[idx]._item);
    val = heap->_buckets[cnt - 1]._item;
    cidx = (idx << 1) + 1;
    while (cidx < __CFBinaryHeapCount(heap)) {
	void *item = heap->_buckets[cidx]._item;
	if (cidx + 1 < __CFBinaryHeapCount(heap)) {
	    void *item2 = heap->_buckets[cidx + 1]._item;
	    if ((!compare && item > item2) || (compare && kCFCompareGreaterThan == compare(item, item2, heap->_context.info))) {
		cidx++;
		item = item2;
	    }
	}
	if ((!compare && item > val) || (compare && kCFCompareGreaterThan == compare(item, val, heap->_context.info))) break;
	__CFAssignWithWriteBarrier((void **)&heap->_buckets[idx]._item, item);
	idx = cidx;
	cidx = (idx << 1) + 1;
    }
    __CFAssignWithWriteBarrier((void **)&heap->_buckets[idx]._item, val);
}

void CFBinaryHeapRemoveAllValues(CFBinaryHeapRef heap) {
    CFIndex idx;
    CFIndex cnt;
    __CFGenericValidateType(heap, CFBinaryHeapGetTypeID());
    cnt = __CFBinaryHeapCount(heap);
    if (heap->_callbacks.release)
	for (idx = 0; idx < cnt; idx++)
	    heap->_callbacks.release(CFGetAllocator(heap), heap->_buckets[idx]._item);
    __CFBinaryHeapSetNumBucketsUsed(heap, 0);
    __CFBinaryHeapSetCount(heap, 0);
}

