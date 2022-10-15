/*	CFBinaryHeap.c
	Copyright (c) 1998-2019, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
	Responsibility: Michael LeHew
*/

#include <CoreFoundation/CFBinaryHeap.h>
#include <CoreFoundation/CFPriv.h>
#include "CFInternal.h"
#include "CFRuntime_Internal.h"

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

CF_INLINE UInt32 __CFBinaryHeapMutableVariety(const void *cf) {
    return __CFRuntimeGetValue(cf, 3, 2);
}

CF_INLINE void __CFBinaryHeapSetMutableVariety(void *cf, UInt32 v) {
    __CFRuntimeSetValue(cf, 3, 2, v);
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
    list1 = (cnt <= 128) ? (const void **)buffer : (const void **)CFAllocatorAllocate(kCFAllocatorSystemDefault, 2 * cnt * sizeof(void *), 0);
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
    if (list1 != buffer) CFAllocatorDeallocate(CFGetAllocator(heap1), list1);
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
    list = (cnt <= 128) ? (const void **)buffer : (const void **)CFAllocatorAllocate(kCFAllocatorSystemDefault, cnt * sizeof(void *), 0);
    if (__CFOASafe && list != buffer) __CFSetLastAllocationEventName(list, "CFBinaryHeap (temp)");
    CFBinaryHeapGetValues(heap, list);
    _CLANG_ANALYZER_IGNORE_UNINITIALIZED_BUFFER(list, cnt * sizeof(void *)); // The analyzer doesn't understand that the 'cnt' values in both this function and CFBinaryHeapGetValues are identical.
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
    if (list != buffer) CFAllocatorDeallocate(CFGetAllocator(heap), list);
    return result;
}

static void __CFBinaryHeapDeallocate(CFTypeRef cf) {
    CFBinaryHeapRef heap = (CFBinaryHeapRef)cf;
    CFAllocatorRef allocator = CFGetAllocator(heap);
// CF: should make the heap mutable here first, a la CFArrayDeallocate
    CFBinaryHeapRemoveAllValues(heap);
// CF: does not release the context info
    if (__CFBinaryHeapMutableVariety(heap) == kCFBinaryHeapMutable) {
	CFAllocatorDeallocate(allocator, heap->_buckets);
    }
}

const CFRuntimeClass __CFBinaryHeapClass = {
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
    return _kCFRuntimeIDCFBinaryHeap;
}

static CFBinaryHeapRef __CFBinaryHeapCreateInit(CFAllocatorRef allocator, UInt32 flags, CFIndex capacity, const void **values, CFIndex numValues, const CFBinaryHeapCallBacks *callBacks, const CFBinaryHeapCompareContext *compareContext) {
    CFBinaryHeapRef memory;
    CFIndex idx;
    CFIndex size;

    CFAssert2(0 <= capacity, __kCFLogAssertion, "%s(): capacity (%ld) cannot be less than zero", __PRETTY_FUNCTION__, capacity);
    CFAssert2(0 <= numValues, __kCFLogAssertion, "%s(): numValues (%ld) cannot be less than zero", __PRETTY_FUNCTION__, numValues);
    size = sizeof(struct __CFBinaryHeap) - sizeof(CFRuntimeBase);

    memory = (CFBinaryHeapRef)_CFRuntimeCreateInstance(allocator, CFBinaryHeapGetTypeID(), size, NULL);
    if (NULL == memory) {
	return NULL;
    }
	__CFBinaryHeapSetCapacity(memory, __CFBinaryHeapRoundUpCapacity(1));
	__CFBinaryHeapSetNumBuckets(memory, __CFBinaryHeapNumBucketsForCapacity(__CFBinaryHeapRoundUpCapacity(1)));
	void *buckets = CFAllocatorAllocate(allocator, __CFBinaryHeapNumBuckets(memory) * sizeof(struct __CFBinaryHeapBucket), 0);
	*((void **)&memory->_buckets) = buckets;
	if (__CFOASafe) __CFSetLastAllocationEventName(memory->_buckets, "CFBinaryHeap (store)");
	if (NULL == memory->_buckets) {
	    CFRelease(memory);
	    return NULL;
	}
    if (NULL != callBacks) {
	memory->_callbacks.retain = callBacks->retain;
	memory->_callbacks.release = callBacks->release;
	memory->_callbacks.copyDescription = callBacks->copyDescription;
	memory->_callbacks.compare = callBacks->compare;
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
   return __CFBinaryHeapCreateInit(allocator, kCFBinaryHeapMutable, capacity, NULL, 0, callBacks, compareContext);
}

CFBinaryHeapRef CFBinaryHeapCreateCopy(CFAllocatorRef allocator, CFIndex capacity, CFBinaryHeapRef heap) {
   __CFGenericValidateType(heap, CFBinaryHeapGetTypeID());
    return __CFBinaryHeapCreateInit(allocator, kCFBinaryHeapMutable, capacity, (const void **)heap->_buckets, __CFBinaryHeapCount(heap), &(heap->_callbacks), &(heap->_context));
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
    if (NULL != value) *((void **)value) = heap->_buckets[0]._item;
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
    void *buckets = __CFSafelyReallocateWithAllocator(allocator, heap->_buckets, __CFBinaryHeapNumBuckets(heap) * sizeof(struct __CFBinaryHeapBucket), 0, NULL);
    *((void **)&heap->_buckets) = buckets;
    if (__CFOASafe) __CFSetLastAllocationEventName(heap->_buckets, "CFBinaryHeap (store)");
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
	*((void **)&heap->_buckets[idx]._item) = item;
	idx = pidx;
	pidx = (idx - 1) >> 1;
    }
    if (heap->_callbacks.retain) {
	*((void **)&heap->_buckets[idx]._item) = (void *)heap->_callbacks.retain(allocator, (void *)value);
    } else {
	*((void **)&heap->_buckets[idx]._item) = (void *)value;
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
	*((void **)&heap->_buckets[idx]._item) = item;
	idx = cidx;
	cidx = (idx << 1) + 1;
    }
    *((void **)&heap->_buckets[idx]._item) = val;
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

