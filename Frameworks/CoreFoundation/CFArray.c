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

/*	CFArray.c
	Copyright (c) 1998-2014, Apple Inc. All rights reserved.
	Responsibility: Christopher Kane
*/

#include <CoreFoundation/CFArray.h>
#include <CoreFoundation/CFPriv.h>
#include "CFInternal.h"
#include <string.h>


const CFArrayCallBacks kCFTypeArrayCallBacks = {0, __CFTypeCollectionRetain, __CFTypeCollectionRelease, CFCopyDescription, CFEqual};
static const CFArrayCallBacks __kCFNullArrayCallBacks = {0, NULL, NULL, NULL, NULL};

struct __CFArrayBucket {
    const void *_item;
};

enum {
    __CF_MAX_BUCKETS_PER_DEQUE = LONG_MAX
};

CF_INLINE CFIndex __CFArrayDequeRoundUpCapacity(CFIndex capacity) {
    if (capacity < 4) return 4;
    return __CFMin((1 << flsl(capacity)), __CF_MAX_BUCKETS_PER_DEQUE);
}

struct __CFArrayDeque {
    uintptr_t _leftIdx;
    uintptr_t _capacity;
    /* struct __CFArrayBucket buckets follow here */
};

struct __CFArray {
    CFRuntimeBase _base;
    CFIndex _count;		/* number of objects */
    CFIndex _mutations;
    int32_t _mutInProgress;
    __strong void *_store;           /* can be NULL when MutableDeque */
};

/* Flag bits */
enum {		/* Bits 0-1 */
    __kCFArrayImmutable = 0,
    __kCFArrayDeque = 2,
};

enum {		/* Bits 2-3 */
    __kCFArrayHasNullCallBacks = 0,
    __kCFArrayHasCFTypeCallBacks = 1,
    __kCFArrayHasCustomCallBacks = 3	/* callbacks are at end of header */
};

/*
    Bits 4 & 5 are reserved for GC use.
    Bit 4, if set, indicates that the array is weak.
    Bit 5 marks whether finalization has occured and, thus, whether to continue to do special retain/release processing of elements.
 */

CF_INLINE bool isStrongMemory(CFTypeRef collection) {
    return __CFBitfieldGetValue(((const CFRuntimeBase *)collection)->_cfinfo[CF_INFO_BITS], 4, 4) == 0;
}

CF_INLINE bool isWeakMemory(CFTypeRef collection) {
    return __CFBitfieldGetValue(((const CFRuntimeBase *)collection)->_cfinfo[CF_INFO_BITS], 4, 4) != 0;
}

CF_INLINE bool hasBeenFinalized(CFTypeRef collection) {
    return __CFBitfieldGetValue(((const CFRuntimeBase *)collection)->_cfinfo[CF_INFO_BITS], 5, 5) != 0;
}
#if DEPLOYMENT_TARGET_MACOSX
CF_INLINE void markFinalized(CFTypeRef collection) {
    __CFBitfieldSetValue(((CFRuntimeBase *)collection)->_cfinfo[CF_INFO_BITS], 5, 5, 1);
}
#endif

CF_INLINE CFIndex __CFArrayGetType(CFArrayRef array) {
    return __CFBitfieldGetValue(((const CFRuntimeBase *)array)->_cfinfo[CF_INFO_BITS], 1, 0);
}

CF_INLINE CFIndex __CFArrayGetSizeOfType(CFIndex t) {
    CFIndex size = 0;
        size += sizeof(struct __CFArray);
    if (__CFBitfieldGetValue(t, 3, 2) == __kCFArrayHasCustomCallBacks) {
	size += sizeof(CFArrayCallBacks);
    }
    return size;
}

CF_INLINE CFIndex __CFArrayGetCount(CFArrayRef array) {
    return array->_count;
}

CF_INLINE void __CFArraySetCount(CFArrayRef array, CFIndex v) {
    ((struct __CFArray *)array)->_count = v;
}

/* Only applies to immutable and mutable-deque-using arrays;
 * Returns the bucket holding the left-most real value in the latter case. */
CF_INLINE struct __CFArrayBucket *__CFArrayGetBucketsPtr(CFArrayRef array) {
    switch (__CFArrayGetType(array)) {
    case __kCFArrayImmutable:
	return (struct __CFArrayBucket *)((uint8_t *)array + __CFArrayGetSizeOfType(((CFRuntimeBase *)array)->_cfinfo[CF_INFO_BITS]));
    case __kCFArrayDeque: {
	struct __CFArrayDeque *deque = (struct __CFArrayDeque *)array->_store;
        return (struct __CFArrayBucket *)((uint8_t *)deque + sizeof(struct __CFArrayDeque) + deque->_leftIdx * sizeof(struct __CFArrayBucket));
    }
    }
    return NULL;
}

/* This shouldn't be called if the array count is 0. */
CF_INLINE struct __CFArrayBucket *__CFArrayGetBucketAtIndex(CFArrayRef array, CFIndex idx) {
    switch (__CFArrayGetType(array)) {
    case __kCFArrayImmutable:
    case __kCFArrayDeque:
	return __CFArrayGetBucketsPtr(array) + idx;
    }
    return NULL;
}

CF_PRIVATE CFArrayCallBacks *__CFArrayGetCallBacks(CFArrayRef array) {
    CFArrayCallBacks *result = NULL;
    switch (__CFBitfieldGetValue(((const CFRuntimeBase *)array)->_cfinfo[CF_INFO_BITS], 3, 2)) {
    case __kCFArrayHasNullCallBacks:
	return (CFArrayCallBacks *)&__kCFNullArrayCallBacks;
    case __kCFArrayHasCFTypeCallBacks:
	return (CFArrayCallBacks *)&kCFTypeArrayCallBacks;
    case __kCFArrayHasCustomCallBacks:
	break;
    }
    switch (__CFArrayGetType(array)) {
    case __kCFArrayImmutable:
	result = (CFArrayCallBacks *)((uint8_t *)array + sizeof(struct __CFArray));
	break;
    case __kCFArrayDeque:
	result = (CFArrayCallBacks *)((uint8_t *)array + sizeof(struct __CFArray));
	break;
    }
    return result;
}

CF_INLINE bool __CFArrayCallBacksMatchNull(const CFArrayCallBacks *c) {
    return (NULL == c ||
	(c->retain == __kCFNullArrayCallBacks.retain &&
	 c->release == __kCFNullArrayCallBacks.release &&
	 c->copyDescription == __kCFNullArrayCallBacks.copyDescription &&
	 c->equal == __kCFNullArrayCallBacks.equal));
}

CF_INLINE bool __CFArrayCallBacksMatchCFType(const CFArrayCallBacks *c) {
    return (&kCFTypeArrayCallBacks == c ||
	(c->retain == kCFTypeArrayCallBacks.retain &&
	 c->release == kCFTypeArrayCallBacks.release &&
	 c->copyDescription == kCFTypeArrayCallBacks.copyDescription &&
	 c->equal == kCFTypeArrayCallBacks.equal));
}

#if 0
#define CHECK_FOR_MUTATION(A) do { if ((A)->_mutInProgress) CFLog(3, CFSTR("*** %s: function called while the array (%p) is being mutated in this or another thread"), __PRETTY_FUNCTION__, (A)); } while (0)
#define BEGIN_MUTATION(A) do { OSAtomicAdd32Barrier(1, &((struct __CFArray *)(A))->_mutInProgress); } while (0)
#define END_MUTATION(A) do { OSAtomicAdd32Barrier(-1, &((struct __CFArray *)(A))->_mutInProgress); } while (0)
#else
#define CHECK_FOR_MUTATION(A) do { } while (0)
#define BEGIN_MUTATION(A) do { } while (0)
#define END_MUTATION(A) do { } while (0)
#endif

struct _releaseContext {
    void (*release)(CFAllocatorRef, const void *);
    CFAllocatorRef allocator; 
};

static void __CFArrayReleaseValues(CFArrayRef array, CFRange range, bool releaseStorageIfPossible) {
    const CFArrayCallBacks *cb = __CFArrayGetCallBacks(array);
    CFAllocatorRef allocator;
    CFIndex idx;
    switch (__CFArrayGetType(array)) {
    case __kCFArrayImmutable:
	if (NULL != cb->release && 0 < range.length && !hasBeenFinalized(array)) {
            // if we've been finalized then we know that
            //   1) we're using the standard callback on GC memory
            //   2) the slots don't' need to be zeroed
	    struct __CFArrayBucket *buckets = __CFArrayGetBucketsPtr(array);
	    allocator = __CFGetAllocator(array);
	    for (idx = 0; idx < range.length; idx++) {
		INVOKE_CALLBACK2(cb->release, allocator, buckets[idx + range.location]._item);
		buckets[idx + range.location]._item = NULL; // GC:  break strong reference.
	    }
	}
	break;
    case __kCFArrayDeque: {
	struct __CFArrayDeque *deque = (struct __CFArrayDeque *)array->_store;
	if (0 < range.length && NULL != deque && !hasBeenFinalized(array)) {
	    struct __CFArrayBucket *buckets = __CFArrayGetBucketsPtr(array);
	    if (NULL != cb->release) {
		allocator = __CFGetAllocator(array);
		for (idx = 0; idx < range.length; idx++) {
		    INVOKE_CALLBACK2(cb->release, allocator, buckets[idx + range.location]._item);
		    buckets[idx + range.location]._item = NULL; // GC:  break strong reference.
		}
            } else {
		for (idx = 0; idx < range.length; idx++) {
		    buckets[idx + range.location]._item = NULL; // GC:  break strong reference.
		}
	    }
	}
	if (releaseStorageIfPossible && 0 == range.location && __CFArrayGetCount(array) == range.length) {
	    allocator = __CFGetAllocator(array);
	    if (NULL != deque) if (!CF_IS_COLLECTABLE_ALLOCATOR(allocator)) CFAllocatorDeallocate(allocator, deque);
	    __CFArraySetCount(array, 0);  // GC: _count == 0 ==> _store == NULL.
	    ((struct __CFArray *)array)->_store = NULL;
	}
	break;
    }
    }
}

#if defined(DEBUG)
CF_INLINE void __CFArrayValidateRange(CFArrayRef array, CFRange range, const char *func) {
    CFAssert3(0 <= range.location && range.location <= CFArrayGetCount(array), __kCFLogAssertion, "%s(): range.location index (%d) out of bounds (0, %d)", func, range.location, CFArrayGetCount(array));
    CFAssert2(0 <= range.length, __kCFLogAssertion, "%s(): range.length (%d) cannot be less than zero", func, range.length);
    CFAssert3(range.location + range.length <= CFArrayGetCount(array), __kCFLogAssertion, "%s(): ending index (%d) out of bounds (0, %d)", func, range.location + range.length, CFArrayGetCount(array));
}
#else
#define __CFArrayValidateRange(a,r,f)
#endif

static Boolean __CFArrayEqual(CFTypeRef cf1, CFTypeRef cf2) {
    CFArrayRef array1 = (CFArrayRef)cf1;
    CFArrayRef array2 = (CFArrayRef)cf2;
    const CFArrayCallBacks *cb1, *cb2;
    CFIndex idx, cnt;
    if (array1 == array2) return true;
    cnt = __CFArrayGetCount(array1);
    if (cnt != __CFArrayGetCount(array2)) return false;
    cb1 = __CFArrayGetCallBacks(array1);
    cb2 = __CFArrayGetCallBacks(array2);
    if (cb1->equal != cb2->equal) return false;
    if (0 == cnt) return true;	/* after function comparison! */
    for (idx = 0; idx < cnt; idx++) {
	const void *val1 = __CFArrayGetBucketAtIndex(array1, idx)->_item;
	const void *val2 = __CFArrayGetBucketAtIndex(array2, idx)->_item;
	if (val1 != val2) {
	    if (NULL == cb1->equal) return false;
	    if (!INVOKE_CALLBACK2(cb1->equal, val1, val2)) return false;
	}
    }
    return true;
}

static CFHashCode __CFArrayHash(CFTypeRef cf) {
    CFArrayRef array = (CFArrayRef)cf;
    return __CFArrayGetCount(array);
}

static CFStringRef __CFArrayCopyDescription(CFTypeRef cf) {
    CFArrayRef array = (CFArrayRef)cf;
    CFMutableStringRef result;
    const CFArrayCallBacks *cb;
    CFAllocatorRef allocator;
    CFIndex idx, cnt;
    cnt = __CFArrayGetCount(array);
    allocator = CFGetAllocator(array);
    result = CFStringCreateMutable(allocator, 0);
    switch (__CFArrayGetType(array)) {
    case __kCFArrayImmutable:
	CFStringAppendFormat(result, NULL, CFSTR("<CFArray %p [%p]>{type = immutable, count = %lu, values = (%s"), cf, allocator, (unsigned long)cnt, cnt ? "\n" : "");
	break;
    case __kCFArrayDeque:
	CFStringAppendFormat(result, NULL, CFSTR("<CFArray %p [%p]>{type = mutable-small, count = %lu, values = (%s"), cf, allocator, (unsigned long)cnt, cnt ? "\n" : "");
	break;
    }
    cb = __CFArrayGetCallBacks(array);
    for (idx = 0; idx < cnt; idx++) {
	CFStringRef desc = NULL;
	const void *val = __CFArrayGetBucketAtIndex(array, idx)->_item;
	if (NULL != cb->copyDescription) {
	    desc = (CFStringRef)INVOKE_CALLBACK1(cb->copyDescription, val);
	}
	if (NULL != desc) {
	    CFStringAppendFormat(result, NULL, CFSTR("\t%lu : %@\n"), (unsigned long)idx, desc);
	    CFRelease(desc);
	} else {
	    CFStringAppendFormat(result, NULL, CFSTR("\t%lu : <%p>\n"), (unsigned long)idx, val);
	}
    }
    CFStringAppend(result, CFSTR(")}"));
    return result;
}


static void __CFArrayDeallocate(CFTypeRef cf) {
    CFArrayRef array = (CFArrayRef)cf;
    BEGIN_MUTATION(array);
#if DEPLOYMENT_TARGET_MACOSX
    // Under GC, keep contents alive when we know we can, either standard callbacks or NULL
    // if (__CFBitfieldGetValue(cf->info, 5, 4)) return; // bits only ever set under GC
    CFAllocatorRef allocator = __CFGetAllocator(array);
    if (CF_IS_COLLECTABLE_ALLOCATOR(allocator)) {
	// XXX_PCB keep array intact during finalization.
	const CFArrayCallBacks *cb = __CFArrayGetCallBacks(array);
	if (cb->retain == NULL && cb->release == NULL) {
	    END_MUTATION(array);
            return;
	}
        if (cb == &kCFTypeArrayCallBacks || cb->release == kCFTypeArrayCallBacks.release) {
            markFinalized(cf);
            for (CFIndex idx = 0; idx < __CFArrayGetCount(array); idx++) {
                const void *item = CFArrayGetValueAtIndex(array, 0 + idx);
    	        kCFTypeArrayCallBacks.release(kCFAllocatorSystemDefault, item);
            }
	    END_MUTATION(array);
            return;
        }
    }
#endif
    __CFArrayReleaseValues(array, CFRangeMake(0, __CFArrayGetCount(array)), true);
    END_MUTATION(array);
}

static CFTypeID __kCFArrayTypeID = _kCFRuntimeNotATypeID;

static const CFRuntimeClass __CFArrayClass = {
    _kCFRuntimeScannedObject,
    "CFArray",
    NULL,	// init
    NULL,	// copy
    __CFArrayDeallocate,
    __CFArrayEqual,
    __CFArrayHash,
    NULL,	// 
    __CFArrayCopyDescription
};

CFTypeID CFArrayGetTypeID(void) {
    static dispatch_once_t initOnce;
    dispatch_once(&initOnce, ^{ __kCFArrayTypeID = _CFRuntimeRegisterClass(&__CFArrayClass); });
    return __kCFArrayTypeID;
}

static CFArrayRef __CFArrayInit(CFAllocatorRef allocator, UInt32 flags, CFIndex capacity, const CFArrayCallBacks *callBacks) {
    struct __CFArray *memory;
    UInt32 size;
    __CFBitfieldSetValue(flags, 31, 2, 0);
    if (CF_IS_COLLECTABLE_ALLOCATOR(allocator)) {
	if (!callBacks || (callBacks->retain == NULL && callBacks->release == NULL)) {
	    __CFBitfieldSetValue(flags, 4, 4, 1); // setWeak
	}
    }
    if (__CFArrayCallBacksMatchNull(callBacks)) {
	__CFBitfieldSetValue(flags, 3, 2, __kCFArrayHasNullCallBacks);
    } else if (__CFArrayCallBacksMatchCFType(callBacks)) {
	__CFBitfieldSetValue(flags, 3, 2, __kCFArrayHasCFTypeCallBacks);
    } else {
	__CFBitfieldSetValue(flags, 3, 2, __kCFArrayHasCustomCallBacks);
    }
    size = __CFArrayGetSizeOfType(flags) - sizeof(CFRuntimeBase);
    switch (__CFBitfieldGetValue(flags, 1, 0)) {
    case __kCFArrayImmutable:
	size += capacity * sizeof(struct __CFArrayBucket);
	break;
    case __kCFArrayDeque:
	break;
    }
    memory = (struct __CFArray*)_CFRuntimeCreateInstance(allocator, CFArrayGetTypeID(), size, NULL);
    if (NULL == memory) {
	return NULL;
    }
    __CFBitfieldSetValue(memory->_base._cfinfo[CF_INFO_BITS], 6, 0, flags);
    __CFArraySetCount((CFArrayRef)memory, 0);
    switch (__CFBitfieldGetValue(flags, 1, 0)) {
    case __kCFArrayImmutable:
        if (isWeakMemory(memory)) {  // if weak, don't scan
            auto_zone_set_unscanned(objc_collectableZone(), memory);
        }
	if (__CFOASafe) __CFSetLastAllocationEventName(memory, "CFArray (immutable)");
	break;
    case __kCFArrayDeque:
	if (__CFOASafe) __CFSetLastAllocationEventName(memory, "CFArray (mutable-variable)");
	((struct __CFArray *)memory)->_mutations = 1;
	((struct __CFArray *)memory)->_mutInProgress = 0;
	((struct __CFArray*)memory)->_store = NULL;
	break;
    }
    if (__kCFArrayHasCustomCallBacks == __CFBitfieldGetValue(flags, 3, 2)) {
	CFArrayCallBacks *cb = (CFArrayCallBacks *)__CFArrayGetCallBacks((CFArrayRef)memory);
	*cb = *callBacks;
	FAULT_CALLBACK((void **)&(cb->retain));
	FAULT_CALLBACK((void **)&(cb->release));
	FAULT_CALLBACK((void **)&(cb->copyDescription));
	FAULT_CALLBACK((void **)&(cb->equal));
    }
    return (CFArrayRef)memory;
}

CF_PRIVATE CFArrayRef __CFArrayCreateTransfer(CFAllocatorRef allocator, const void **values, CFIndex numValues) {
    CFAssert2(0 <= numValues, __kCFLogAssertion, "%s(): numValues (%d) cannot be less than zero", __PRETTY_FUNCTION__, numValues);
    UInt32 flags = __kCFArrayImmutable;
    __CFBitfieldSetValue(flags, 31, 2, 0);
    __CFBitfieldSetValue(flags, 3, 2, __kCFArrayHasCFTypeCallBacks);
    UInt32 size = __CFArrayGetSizeOfType(flags) - sizeof(CFRuntimeBase);
    size += numValues * sizeof(struct __CFArrayBucket);
    struct __CFArray *memory = (struct __CFArray*)_CFRuntimeCreateInstance(allocator, CFArrayGetTypeID(), size, NULL);
    if (NULL == memory) {
	return NULL;
    }
    __CFBitfieldSetValue(memory->_base._cfinfo[CF_INFO_BITS], 6, 0, flags);
    __CFArraySetCount(memory, numValues);
    memmove(__CFArrayGetBucketsPtr(memory), values, sizeof(void *) * numValues);
    if (__CFOASafe) __CFSetLastAllocationEventName(memory, "CFArray (immutable)");
    return (CFArrayRef)memory;
}

CF_PRIVATE CFArrayRef __CFArrayCreate0(CFAllocatorRef allocator, const void **values, CFIndex numValues, const CFArrayCallBacks *callBacks) {
    CFArrayRef result;
    const CFArrayCallBacks *cb;
    struct __CFArrayBucket *buckets;
    CFAllocatorRef bucketsAllocator;
    void* bucketsBase;
    CFIndex idx;
    CFAssert2(0 <= numValues, __kCFLogAssertion, "%s(): numValues (%d) cannot be less than zero", __PRETTY_FUNCTION__, numValues);
    result = __CFArrayInit(allocator, __kCFArrayImmutable, numValues, callBacks);
    cb = __CFArrayGetCallBacks(result);
    buckets = __CFArrayGetBucketsPtr(result);
    bucketsAllocator = isStrongMemory(result) ? allocator : kCFAllocatorNull;
    bucketsBase = CF_IS_COLLECTABLE_ALLOCATOR(bucketsAllocator) ? (void *)auto_zone_base_pointer(objc_collectableZone(), buckets) : NULL;
    if (NULL != cb->retain) {
        for (idx = 0; idx < numValues; idx++) {
	    __CFAssignWithWriteBarrier((void **)&buckets->_item, (void *)INVOKE_CALLBACK2(cb->retain, allocator, *values));
            values++;
            buckets++;
        }
    }
    else {
        for (idx = 0; idx < numValues; idx++) {
            __CFAssignWithWriteBarrier((void **)&buckets->_item, (void *)*values);
            values++;
            buckets++;
        }
    }
    __CFArraySetCount(result, numValues);
    return result;
}

CF_PRIVATE CFMutableArrayRef __CFArrayCreateMutable0(CFAllocatorRef allocator, CFIndex capacity, const CFArrayCallBacks *callBacks) {
    CFAssert2(0 <= capacity, __kCFLogAssertion, "%s(): capacity (%d) cannot be less than zero", __PRETTY_FUNCTION__, capacity);
    CFAssert2(capacity <= LONG_MAX / sizeof(void *), __kCFLogAssertion, "%s(): capacity (%d) is too large for this architecture", __PRETTY_FUNCTION__, capacity);
    return (CFMutableArrayRef)__CFArrayInit(allocator, __kCFArrayDeque, capacity, callBacks);
}

CF_PRIVATE CFArrayRef __CFArrayCreateCopy0(CFAllocatorRef allocator, CFArrayRef array) {
    CFArrayRef result;
    const CFArrayCallBacks *cb;
    struct __CFArrayBucket *buckets;
    CFAllocatorRef bucketsAllocator;
    void* bucketsBase;
    CFIndex numValues = CFArrayGetCount(array);
    CFIndex idx;
    if (CF_IS_OBJC(CFArrayGetTypeID(), array)) {
	cb = &kCFTypeArrayCallBacks;
    } else {
	cb = __CFArrayGetCallBacks(array);
	    }
    result = __CFArrayInit(allocator, __kCFArrayImmutable, numValues, cb);
    cb = __CFArrayGetCallBacks(result); // GC: use the new array's callbacks so we don't leak.
    buckets = __CFArrayGetBucketsPtr(result);
    bucketsAllocator = isStrongMemory(result) ? allocator : kCFAllocatorNull;
	bucketsBase = CF_IS_COLLECTABLE_ALLOCATOR(bucketsAllocator) ? (void *)auto_zone_base_pointer(objc_collectableZone(), buckets) : NULL;
    for (idx = 0; idx < numValues; idx++) {
	const void *value = CFArrayGetValueAtIndex(array, idx);
	if (NULL != cb->retain) {
	    value = (void *)INVOKE_CALLBACK2(cb->retain, allocator, value);
	}
	__CFAssignWithWriteBarrier((void **)&buckets->_item, (void *)value);
	buckets++;
    }
    __CFArraySetCount(result, numValues);
    return result;
}

CF_PRIVATE CFMutableArrayRef __CFArrayCreateMutableCopy0(CFAllocatorRef allocator, CFIndex capacity, CFArrayRef array) {
    CFMutableArrayRef result;
    const CFArrayCallBacks *cb;
    CFIndex idx, numValues = CFArrayGetCount(array);
    UInt32 flags;
    if (CF_IS_OBJC(CFArrayGetTypeID(), array)) {
	cb = &kCFTypeArrayCallBacks;
    }
    else {
	cb = __CFArrayGetCallBacks(array);
    }
    flags = __kCFArrayDeque;
    result = (CFMutableArrayRef)__CFArrayInit(allocator, flags, capacity, cb);
    if (0 == capacity) _CFArraySetCapacity(result, numValues);
    for (idx = 0; idx < numValues; idx++) {
	const void *value = CFArrayGetValueAtIndex(array, idx);
	CFArrayAppendValue(result, value);
    }
    return result;
}

#define DEFINE_CREATION_METHODS 1

#if DEFINE_CREATION_METHODS

CFArrayRef CFArrayCreate(CFAllocatorRef allocator, const void **values, CFIndex numValues, const CFArrayCallBacks *callBacks) {
    return __CFArrayCreate0(allocator, values, numValues, callBacks);
}

CFMutableArrayRef CFArrayCreateMutable(CFAllocatorRef allocator, CFIndex capacity, const CFArrayCallBacks *callBacks) {
    return __CFArrayCreateMutable0(allocator, capacity, callBacks);
}

CFArrayRef CFArrayCreateCopy(CFAllocatorRef allocator, CFArrayRef array) {
    return __CFArrayCreateCopy0(allocator, array);
}

CFMutableArrayRef CFArrayCreateMutableCopy(CFAllocatorRef allocator, CFIndex capacity, CFArrayRef array) {
    return __CFArrayCreateMutableCopy0(allocator, capacity, array);
}

#endif

CFIndex CFArrayGetCount(CFArrayRef array) {
    CF_OBJC_FUNCDISPATCHV(CFArrayGetTypeID(), CFIndex, (NSArray *)array, count);
    __CFGenericValidateType(array, CFArrayGetTypeID());
    CHECK_FOR_MUTATION(array);
    return __CFArrayGetCount(array);
}


CFIndex CFArrayGetCountOfValue(CFArrayRef array, CFRange range, const void *value) {
    CFIndex idx, count = 0;
    __CFGenericValidateType(array, CFArrayGetTypeID());    
    __CFArrayValidateRange(array, range, __PRETTY_FUNCTION__);
    CHECK_FOR_MUTATION(array);
    const CFArrayCallBacks *cb = CF_IS_OBJC(CFArrayGetTypeID(), array) ? &kCFTypeArrayCallBacks : __CFArrayGetCallBacks(array);
    for (idx = 0; idx < range.length; idx++) {
	const void *item = CFArrayGetValueAtIndex(array, range.location + idx);
	if (value == item || (cb->equal && INVOKE_CALLBACK2(cb->equal, value, item))) {
	    count++;
	}
    }
    return count;
}

Boolean CFArrayContainsValue(CFArrayRef array, CFRange range, const void *value) {
    CFIndex idx;
    __CFGenericValidateType(array, CFArrayGetTypeID());
    __CFArrayValidateRange(array, range, __PRETTY_FUNCTION__);
    CHECK_FOR_MUTATION(array);
    const CFArrayCallBacks *cb = CF_IS_OBJC(CFArrayGetTypeID(), array) ? &kCFTypeArrayCallBacks : __CFArrayGetCallBacks(array);
    for (idx = 0; idx < range.length; idx++) {
	const void *item = CFArrayGetValueAtIndex(array, range.location + idx);
	if (value == item || (cb->equal && INVOKE_CALLBACK2(cb->equal, value, item))) {
	    return true;
	}
    }
    return false;
}

const void *CFArrayGetValueAtIndex(CFArrayRef array, CFIndex idx) {
    CF_OBJC_FUNCDISPATCHV(CFArrayGetTypeID(), const void *, (NSArray *)array, objectAtIndex:idx);
    __CFGenericValidateType(array, CFArrayGetTypeID());
    CFAssert2(0 <= idx && idx < __CFArrayGetCount(array), __kCFLogAssertion, "%s(): index (%d) out of bounds", __PRETTY_FUNCTION__, idx);
    CHECK_FOR_MUTATION(array);
    return __CFArrayGetBucketAtIndex(array, idx)->_item;
}

// This is for use by NSCFArray; it avoids ObjC dispatch, and checks for out of bounds
const void *_CFArrayCheckAndGetValueAtIndex(CFArrayRef array, CFIndex idx) {
    CHECK_FOR_MUTATION(array);
    if (0 <= idx && idx < __CFArrayGetCount(array)) return __CFArrayGetBucketAtIndex(array, idx)->_item;
    return (void *)(-1);
}


void CFArrayGetValues(CFArrayRef array, CFRange range, const void **values) {
    CF_OBJC_FUNCDISPATCHV(CFArrayGetTypeID(), void, (NSArray *)array, getObjects:(id *)values range:NSMakeRange(range.location, range.length));
    __CFGenericValidateType(array, CFArrayGetTypeID());
    __CFArrayValidateRange(array, range, __PRETTY_FUNCTION__);
    CFAssert1(NULL != values, __kCFLogAssertion, "%s(): pointer to values may not be NULL", __PRETTY_FUNCTION__);
    CHECK_FOR_MUTATION(array);
    if (0 < range.length) {
	switch (__CFArrayGetType(array)) {
	case __kCFArrayImmutable:
	case __kCFArrayDeque:
	    objc_memmove_collectable(values, __CFArrayGetBucketsPtr(array) + range.location, range.length * sizeof(struct __CFArrayBucket));
	    break;
	}
    }
}

CF_EXPORT unsigned long _CFArrayFastEnumeration(CFArrayRef array, struct __objcFastEnumerationStateEquivalent *state, void *stackbuffer, unsigned long count) {
    CHECK_FOR_MUTATION(array);
    if (array->_count == 0) return 0;
    enum { ATSTART = 0, ATEND = 1 };
    switch (__CFArrayGetType(array)) {
    case __kCFArrayImmutable:
        if (state->state == ATSTART) { /* first time */
            static const unsigned long const_mu = 1;
            state->state = ATEND;
            state->mutationsPtr = (unsigned long *)&const_mu;
            state->itemsPtr = (unsigned long *)__CFArrayGetBucketsPtr(array);
            return array->_count;
        }
        return 0;			
    case __kCFArrayDeque:
        if (state->state == ATSTART) { /* first time */
            state->state = ATEND;
            state->mutationsPtr = (unsigned long *)&array->_mutations;
            state->itemsPtr = (unsigned long *)__CFArrayGetBucketsPtr(array);
            return array->_count;
        }
        return 0;
    }
    return 0;
}


void CFArrayApplyFunction(CFArrayRef array, CFRange range, CFArrayApplierFunction applier, void *context) {
    CFIndex idx;
    FAULT_CALLBACK((void **)&(applier));
    __CFGenericValidateType(array, CFArrayGetTypeID());
    __CFArrayValidateRange(array, range, __PRETTY_FUNCTION__);
    CFAssert1(NULL != applier, __kCFLogAssertion, "%s(): pointer to applier function may not be NULL", __PRETTY_FUNCTION__);
    CHECK_FOR_MUTATION(array);
    for (idx = 0; idx < range.length; idx++) {
	const void *item = CFArrayGetValueAtIndex(array, range.location + idx);
	INVOKE_CALLBACK2(applier, item, context);
    }
}

CFIndex CFArrayGetFirstIndexOfValue(CFArrayRef array, CFRange range, const void *value) {
    CFIndex idx;
    __CFGenericValidateType(array, CFArrayGetTypeID());
    __CFArrayValidateRange(array, range, __PRETTY_FUNCTION__);
    CHECK_FOR_MUTATION(array);
    const CFArrayCallBacks *cb = CF_IS_OBJC(CFArrayGetTypeID(), array) ? &kCFTypeArrayCallBacks : __CFArrayGetCallBacks(array);
    for (idx = 0; idx < range.length; idx++) {
	const void *item = CFArrayGetValueAtIndex(array, range.location + idx);
	if (value == item || (cb->equal && INVOKE_CALLBACK2(cb->equal, value, item)))
	    return idx + range.location;
    }
    return kCFNotFound;
}

CFIndex CFArrayGetLastIndexOfValue(CFArrayRef array, CFRange range, const void *value) {
    CFIndex idx;
    __CFGenericValidateType(array, CFArrayGetTypeID());
    __CFArrayValidateRange(array, range, __PRETTY_FUNCTION__);
    CHECK_FOR_MUTATION(array);
    const CFArrayCallBacks *cb = CF_IS_OBJC(CFArrayGetTypeID(), array) ? &kCFTypeArrayCallBacks : __CFArrayGetCallBacks(array);
    for (idx = range.length; idx--;) {
	const void *item = CFArrayGetValueAtIndex(array, range.location + idx);
	if (value == item || (cb->equal && INVOKE_CALLBACK2(cb->equal, value, item)))
	    return idx + range.location;
    }
    return kCFNotFound;
}

void CFArrayAppendValue(CFMutableArrayRef array, const void *value) {
    CF_OBJC_FUNCDISPATCHV(CFArrayGetTypeID(), void, (NSMutableArray *)array, addObject:(id)value);
    
    __CFGenericValidateType(array, CFArrayGetTypeID());
    CFAssert1(__CFArrayGetType(array) != __kCFArrayImmutable, __kCFLogAssertion, "%s(): array is immutable", __PRETTY_FUNCTION__);
    CHECK_FOR_MUTATION(array);
    _CFArrayReplaceValues(array, CFRangeMake(__CFArrayGetCount(array), 0), &value, 1);
}

void CFArraySetValueAtIndex(CFMutableArrayRef array, CFIndex idx, const void *value) {
    CF_OBJC_FUNCDISPATCHV(CFArrayGetTypeID(), void, (NSMutableArray *)array, setObject:(id)value atIndex:(NSUInteger)idx);
    __CFGenericValidateType(array, CFArrayGetTypeID());
    CFAssert1(__CFArrayGetType(array) != __kCFArrayImmutable, __kCFLogAssertion, "%s(): array is immutable", __PRETTY_FUNCTION__);
    CFAssert2(0 <= idx && idx <= __CFArrayGetCount(array), __kCFLogAssertion, "%s(): index (%d) out of bounds", __PRETTY_FUNCTION__, idx);
    CHECK_FOR_MUTATION(array);
    if (idx == __CFArrayGetCount(array)) {
	_CFArrayReplaceValues(array, CFRangeMake(idx, 0), &value, 1);
    } else {
	BEGIN_MUTATION(array);
	const void *old_value;
	const CFArrayCallBacks *cb = __CFArrayGetCallBacks(array);
	CFAllocatorRef allocator = __CFGetAllocator(array);
	struct __CFArrayBucket *bucket = __CFArrayGetBucketAtIndex(array, idx);
	if (NULL != cb->retain && !hasBeenFinalized(array)) {
	    value = (void *)INVOKE_CALLBACK2(cb->retain, allocator, value);
	}
	old_value = bucket->_item;
	__CFAssignWithWriteBarrier((void **)&bucket->_item, (void *)value); // GC: handles deque/CFStorage cases.
	if (NULL != cb->release && !hasBeenFinalized(array)) {
	    INVOKE_CALLBACK2(cb->release, allocator, old_value);
	}
	array->_mutations++;
        END_MUTATION(array);
    }
}

void CFArrayInsertValueAtIndex(CFMutableArrayRef array, CFIndex idx, const void *value) {
    CF_OBJC_FUNCDISPATCHV(CFArrayGetTypeID(), void, (NSMutableArray *)array, insertObject:(id)value atIndex:(NSUInteger)idx);
    __CFGenericValidateType(array, CFArrayGetTypeID());
    CFAssert1(__CFArrayGetType(array) != __kCFArrayImmutable, __kCFLogAssertion, "%s(): array is immutable", __PRETTY_FUNCTION__);
    CFAssert2(0 <= idx && idx <= __CFArrayGetCount(array), __kCFLogAssertion, "%s(): index (%d) out of bounds", __PRETTY_FUNCTION__, idx);
    CHECK_FOR_MUTATION(array);
    _CFArrayReplaceValues(array, CFRangeMake(idx, 0), &value, 1);
}

// NB: AddressBook on the Phone is a fragile flower, so this function cannot do anything
// that causes the values to be retained or released.
void CFArrayExchangeValuesAtIndices(CFMutableArrayRef array, CFIndex idx1, CFIndex idx2) {
    const void *tmp;
    struct __CFArrayBucket *bucket1, *bucket2;
    CF_OBJC_FUNCDISPATCHV(CFArrayGetTypeID(), void, (NSMutableArray *)array, exchangeObjectAtIndex:(NSUInteger)idx1 withObjectAtIndex:(NSUInteger)idx2);
    __CFGenericValidateType(array, CFArrayGetTypeID());
    CFAssert2(0 <= idx1 && idx1 < __CFArrayGetCount(array), __kCFLogAssertion, "%s(): index #1 (%d) out of bounds", __PRETTY_FUNCTION__, idx1);
    CFAssert2(0 <= idx2 && idx2 < __CFArrayGetCount(array), __kCFLogAssertion, "%s(): index #2 (%d) out of bounds", __PRETTY_FUNCTION__, idx2);
    CFAssert1(__CFArrayGetType(array) != __kCFArrayImmutable, __kCFLogAssertion, "%s(): array is immutable", __PRETTY_FUNCTION__);
    CHECK_FOR_MUTATION(array);
    BEGIN_MUTATION(array);
    bucket1 = __CFArrayGetBucketAtIndex(array, idx1);
    bucket2 = __CFArrayGetBucketAtIndex(array, idx2);
    tmp = bucket1->_item;
    // XXX these aren't needed.
    __CFAssignWithWriteBarrier((void **)&bucket1->_item, (void *)bucket2->_item);
    __CFAssignWithWriteBarrier((void **)&bucket2->_item, (void *)tmp);
    array->_mutations++;
    END_MUTATION(array);
}

void CFArrayRemoveValueAtIndex(CFMutableArrayRef array, CFIndex idx) {
    CF_OBJC_FUNCDISPATCHV(CFArrayGetTypeID(), void, (NSMutableArray *)array, removeObjectAtIndex:(NSUInteger)idx);
    __CFGenericValidateType(array, CFArrayGetTypeID());
    CFAssert1(__CFArrayGetType(array) != __kCFArrayImmutable, __kCFLogAssertion, "%s(): array is immutable", __PRETTY_FUNCTION__);
    CFAssert2(0 <= idx && idx < __CFArrayGetCount(array), __kCFLogAssertion, "%s(): index (%d) out of bounds", __PRETTY_FUNCTION__, idx);
    CHECK_FOR_MUTATION(array);
    _CFArrayReplaceValues(array, CFRangeMake(idx, 1), NULL, 0);
}

void CFArrayRemoveAllValues(CFMutableArrayRef array) {
    CF_OBJC_FUNCDISPATCHV(CFArrayGetTypeID(), void, (NSMutableArray *)array, removeAllObjects);
    __CFGenericValidateType(array, CFArrayGetTypeID());
    CFAssert1(__CFArrayGetType(array) != __kCFArrayImmutable, __kCFLogAssertion, "%s(): array is immutable", __PRETTY_FUNCTION__);
    CHECK_FOR_MUTATION(array);
    BEGIN_MUTATION(array);
    __CFArrayReleaseValues(array, CFRangeMake(0, __CFArrayGetCount(array)), true);
    __CFArraySetCount(array, 0);
    array->_mutations++;
    END_MUTATION(array);
}

// may move deque storage, as it may need to grow deque
static void __CFArrayRepositionDequeRegions(CFMutableArrayRef array, CFRange range, CFIndex newCount) {
    // newCount elements are going to replace the range, and the result will fit in the deque
    struct __CFArrayDeque *deque = (struct __CFArrayDeque *)array->_store;
    struct __CFArrayBucket *buckets;
    CFIndex cnt, futureCnt, numNewElems;
    CFIndex L, A, B, C, R;

    buckets = (struct __CFArrayBucket *)((uint8_t *)deque + sizeof(struct __CFArrayDeque));
    cnt = __CFArrayGetCount(array);
    futureCnt = cnt - range.length + newCount;

    L = deque->_leftIdx;		// length of region to left of deque
    A = range.location;			// length of region in deque to left of replaced range
    B = range.length;			// length of replaced range
    C = cnt - B - A;			// length of region in deque to right of replaced range
    R = deque->_capacity - cnt - L;	// length of region to right of deque
    numNewElems = newCount - B;

    CFIndex wiggle = deque->_capacity >> 17;
    if (wiggle < 4) wiggle = 4;
    if (deque->_capacity < (uint32_t)futureCnt || (cnt < futureCnt && L + R < wiggle)) {
	// must be inserting or space is tight, reallocate and re-center everything
	CFIndex capacity = __CFArrayDequeRoundUpCapacity(futureCnt + wiggle);
	CFIndex size = sizeof(struct __CFArrayDeque) + capacity * sizeof(struct __CFArrayBucket);
	CFAllocatorRef allocator = __CFGetAllocator(array);
        Boolean collectableMemory = CF_IS_COLLECTABLE_ALLOCATOR(allocator);
	struct __CFArrayDeque *newDeque = (struct __CFArrayDeque *)CFAllocatorAllocate(allocator, size, isStrongMemory(array) ? __kCFAllocatorGCScannedMemory : 0);
	if (__CFOASafe) __CFSetLastAllocationEventName(newDeque, "CFArray (store-deque)");
	struct __CFArrayBucket *newBuckets = (struct __CFArrayBucket *)((uint8_t *)newDeque + sizeof(struct __CFArrayDeque));
	CFIndex oldL = L;
	CFIndex newL = (capacity - futureCnt) / 2;
	CFIndex oldC0 = oldL + A + B;
	CFIndex newC0 = newL + A + newCount;
	newDeque->_leftIdx = newL;
	newDeque->_capacity = capacity;
	if (0 < A) objc_memmove_collectable(newBuckets + newL, buckets + oldL, A * sizeof(struct __CFArrayBucket));
	if (0 < C) objc_memmove_collectable(newBuckets + newC0, buckets + oldC0, C * sizeof(struct __CFArrayBucket));
	__CFAssignWithWriteBarrier((void **)&array->_store, (void *)newDeque);
        if (!collectableMemory && deque) CFAllocatorDeallocate(allocator, deque);
        if (CF_IS_COLLECTABLE_ALLOCATOR(allocator)) auto_zone_release(objc_collectableZone(), newDeque);
//printf("3:  array %p store is now %p (%lx)\n", array, array->_store, *(unsigned long *)(array->_store));
	return;
    }

    if ((numNewElems < 0 && C < A) || (numNewElems <= R && C < A)) {	// move C
	// deleting: C is smaller
	// inserting: C is smaller and R has room
	CFIndex oldC0 = L + A + B;
	CFIndex newC0 = L + A + newCount;
	if (0 < C) objc_memmove_collectable(buckets + newC0, buckets + oldC0, C * sizeof(struct __CFArrayBucket));
	// GrP GC: zero-out newly exposed space on the right, if any
	if (oldC0 > newC0) memset(buckets + newC0 + C, 0, (oldC0 - newC0) * sizeof(struct __CFArrayBucket));
    } else if ((numNewElems < 0) || (numNewElems <= L && A <= C)) {	// move A
	// deleting: A is smaller or equal (covers remaining delete cases)
	// inserting: A is smaller and L has room
	CFIndex oldL = L;
	CFIndex newL = L - numNewElems;
	deque->_leftIdx = newL;
	if (0 < A) objc_memmove_collectable(buckets + newL, buckets + oldL, A * sizeof(struct __CFArrayBucket));
	// GrP GC: zero-out newly exposed space on the left, if any
	if (newL > oldL) memset(buckets + oldL, 0, (newL - oldL) * sizeof(struct __CFArrayBucket));
    } else {
	// now, must be inserting, and either:
	//    A<=C, but L doesn't have room (R might have, but don't care)
	//    C<A, but R doesn't have room (L might have, but don't care)
	// re-center everything
	CFIndex oldL = L;
	CFIndex newL = (L + R - numNewElems) / 2;
	newL = newL - newL / 2;
	CFIndex oldC0 = oldL + A + B;
	CFIndex newC0 = newL + A + newCount;
	deque->_leftIdx = newL;
	if (newL < oldL) {
	    if (0 < A) objc_memmove_collectable(buckets + newL, buckets + oldL, A * sizeof(struct __CFArrayBucket));
	    if (0 < C) objc_memmove_collectable(buckets + newC0, buckets + oldC0, C * sizeof(struct __CFArrayBucket));
	    // GrP GC: zero-out newly exposed space on the right, if any
	    if (oldC0 > newC0) memset(buckets + newC0 + C, 0, (oldC0 - newC0) * sizeof(struct __CFArrayBucket));
	} else {
	    if (0 < C) objc_memmove_collectable(buckets + newC0, buckets + oldC0, C * sizeof(struct __CFArrayBucket));
	    if (0 < A) objc_memmove_collectable(buckets + newL, buckets + oldL, A * sizeof(struct __CFArrayBucket));
	    // GrP GC: zero-out newly exposed space on the left, if any
	    if (newL > oldL) memset(buckets + oldL, 0, (newL - oldL) * sizeof(struct __CFArrayBucket));
	}
    }
}

static void __CFArrayHandleOutOfMemory(CFTypeRef obj, CFIndex numBytes) {
    CFStringRef msg = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("Attempt to allocate %ld bytes for CFArray failed"), numBytes);
    {
        CFLog(kCFLogLevelCritical, CFSTR("%@"), msg);
        HALT;
    }
    CFRelease(msg);
}

// This function is for Foundation's benefit; no one else should use it.
void _CFArraySetCapacity(CFMutableArrayRef array, CFIndex cap) {
    if (CF_IS_OBJC(CFArrayGetTypeID(), array)) return;
    __CFGenericValidateType(array, CFArrayGetTypeID());
    CFAssert1(__CFArrayGetType(array) != __kCFArrayImmutable, __kCFLogAssertion, "%s(): array is immutable", __PRETTY_FUNCTION__);
    CFAssert3(__CFArrayGetCount(array) <= cap, __kCFLogAssertion, "%s(): desired capacity (%d) is less than count (%d)", __PRETTY_FUNCTION__, cap, __CFArrayGetCount(array));
    CHECK_FOR_MUTATION(array);
    BEGIN_MUTATION(array);
    // Currently, attempting to set the capacity of an array which is the CFStorage
    // variant, or set the capacity larger than __CF_MAX_BUCKETS_PER_DEQUE, has no
    // effect.  The primary purpose of this API is to help avoid a bunch of the
    // resizes at the small capacities 4, 8, 16, etc.
    if (__CFArrayGetType(array) == __kCFArrayDeque) {
	struct __CFArrayDeque *deque = (struct __CFArrayDeque *)array->_store;
	CFIndex capacity = __CFArrayDequeRoundUpCapacity(cap);
	CFIndex size = sizeof(struct __CFArrayDeque) + capacity * sizeof(struct __CFArrayBucket);
	CFAllocatorRef allocator = __CFGetAllocator(array);
	Boolean collectableMemory = CF_IS_COLLECTABLE_ALLOCATOR(allocator);
	if (NULL == deque) {
	    deque = (struct __CFArrayDeque *)CFAllocatorAllocate(allocator, size, isStrongMemory(array) ? __kCFAllocatorGCScannedMemory : 0);
	    if (NULL == deque) __CFArrayHandleOutOfMemory(array, size);
	    if (__CFOASafe) __CFSetLastAllocationEventName(deque, "CFArray (store-deque)");
	    deque->_leftIdx = capacity / 2; 
	} else {
	    struct __CFArrayDeque *olddeque = deque;
	    CFIndex oldcap = deque->_capacity;
	    deque = (struct __CFArrayDeque *)CFAllocatorAllocate(allocator, size, isStrongMemory(array) ? __kCFAllocatorGCScannedMemory : 0);
	    if (NULL == deque) __CFArrayHandleOutOfMemory(array, size);
	    objc_memmove_collectable(deque, olddeque, sizeof(struct __CFArrayDeque) + oldcap * sizeof(struct __CFArrayBucket));
	    if (!collectableMemory) CFAllocatorDeallocate(allocator, olddeque);
	    if (__CFOASafe) __CFSetLastAllocationEventName(deque, "CFArray (store-deque)");
	}
	deque->_capacity = capacity;
	__CFAssignWithWriteBarrier((void **)&array->_store, (void *)deque);
        if (collectableMemory) auto_zone_release(objc_collectableZone(), deque);
    }
    END_MUTATION(array);
}


void CFArrayReplaceValues(CFMutableArrayRef array, CFRange range, const void **newValues, CFIndex newCount) {
    CF_OBJC_FUNCDISPATCHV(CFArrayGetTypeID(), void, (NSMutableArray *)array, replaceObjectsInRange:NSMakeRange(range.location, range.length) withObjects:(id *)newValues count:(NSUInteger)newCount);
    __CFGenericValidateType(array, CFArrayGetTypeID());
    __CFArrayValidateRange(array, range, __PRETTY_FUNCTION__);
    CFAssert1(__CFArrayGetType(array) != __kCFArrayImmutable, __kCFLogAssertion, "%s(): array is immutable", __PRETTY_FUNCTION__);
    CFAssert2(0 <= newCount, __kCFLogAssertion, "%s(): newCount (%d) cannot be less than zero", __PRETTY_FUNCTION__, newCount);
    CHECK_FOR_MUTATION(array);
    return _CFArrayReplaceValues(array, range, newValues, newCount);
}

// This function does no ObjC dispatch or argument checking;
// It should only be called from places where that dispatch and check has already been done, or NSCFArray
void _CFArrayReplaceValues(CFMutableArrayRef array, CFRange range, const void **newValues, CFIndex newCount) {
    CHECK_FOR_MUTATION(array);
    BEGIN_MUTATION(array);
    const CFArrayCallBacks *cb;
    CFIndex idx, cnt, futureCnt;
    const void **newv, *buffer[256];
    cnt = __CFArrayGetCount(array);
    futureCnt = cnt - range.length + newCount;
    CFAssert1(newCount <= futureCnt, __kCFLogAssertion, "%s(): internal error 1", __PRETTY_FUNCTION__);
    cb = __CFArrayGetCallBacks(array);
    CFAllocatorRef allocator = __CFGetAllocator(array);

    /* Retain new values if needed, possibly allocating a temporary buffer for them */
    if (NULL != cb->retain && !hasBeenFinalized(array)) {
	newv = (newCount <= 256) ? (const void **)buffer : (const void **)CFAllocatorAllocate(kCFAllocatorSystemDefault, newCount * sizeof(void *), 0); // GC OK
	if (newv != buffer && __CFOASafe) __CFSetLastAllocationEventName(newv, "CFArray (temp)");
	for (idx = 0; idx < newCount; idx++) {
	    newv[idx] = (void *)INVOKE_CALLBACK2(cb->retain, allocator, (void *)newValues[idx]);
	}
    } else {
	newv = newValues;
    }
    array->_mutations++;

    /* Now, there are three regions of interest, each of which may be empty:
     *   A: the region from index 0 to one less than the range.location
     *   B: the region of the range
     *   C: the region from range.location + range.length to the end
     * Note that index 0 is not necessarily at the lowest-address edge
     * of the available storage. The values in region B need to get
     * released, and the values in regions A and C (depending) need
     * to get shifted if the number of new values is different from
     * the length of the range being replaced.
     */
    if (0 < range.length) {
	__CFArrayReleaseValues(array, range, false);
    }
    // region B elements are now "dead"
    if (0) {
    } else if (NULL == array->_store) {
	if (0) {
	} else if (0 <= futureCnt) {
	    struct __CFArrayDeque *deque;
	    CFIndex capacity = __CFArrayDequeRoundUpCapacity(futureCnt);
	    CFIndex size = sizeof(struct __CFArrayDeque) + capacity * sizeof(struct __CFArrayBucket);
	    deque = (struct __CFArrayDeque *)CFAllocatorAllocate((allocator), size, isStrongMemory(array) ? __kCFAllocatorGCScannedMemory : 0);
	    if (__CFOASafe) __CFSetLastAllocationEventName(deque, "CFArray (store-deque)");
	    deque->_leftIdx = (capacity - newCount) / 2;
	    deque->_capacity = capacity;
	    __CFAssignWithWriteBarrier((void **)&array->_store, (void *)deque);
            if (CF_IS_COLLECTABLE_ALLOCATOR(allocator)) auto_zone_release(objc_collectableZone(), deque); // GC: now safe to unroot the array body.
	}
    } else {		// Deque
	// reposition regions A and C for new region B elements in gap
	if (0) {
	} else if (range.length != newCount) {
	    __CFArrayRepositionDequeRegions(array, range, newCount);
	}
    }
    // copy in new region B elements
    if (0 < newCount) {
	if (0) {
	} else {	// Deque
	    struct __CFArrayDeque *deque = (struct __CFArrayDeque *)array->_store;
	    struct __CFArrayBucket *raw_buckets = (struct __CFArrayBucket *)((uint8_t *)deque + sizeof(struct __CFArrayDeque));
	    objc_memmove_collectable(raw_buckets + deque->_leftIdx + range.location, newv, newCount * sizeof(struct __CFArrayBucket));
	}
    }
    __CFArraySetCount(array, futureCnt);
    if (newv != buffer && newv != newValues) CFAllocatorDeallocate(kCFAllocatorSystemDefault, newv);
    END_MUTATION(array);
}

struct _acompareContext {
    CFComparatorFunction func;
    void *context;
};

static CFComparisonResult __CFArrayCompareValues(const void *v1, const void *v2, struct _acompareContext *context) {
    const void **val1 = (const void **)v1;
    const void **val2 = (const void **)v2;
    return (CFComparisonResult)(INVOKE_CALLBACK3(context->func, *val1, *val2, context->context));
}

CF_INLINE void __CFZSort(CFMutableArrayRef array, CFRange range, CFComparatorFunction comparator, void *context) {
    CFIndex cnt = range.length;
    while (1 < cnt) {
	for (CFIndex idx = range.location; idx < range.location + cnt - 1; idx++) {
            const void *a = CFArrayGetValueAtIndex(array, idx);
            const void *b = CFArrayGetValueAtIndex(array, idx + 1);
            if ((CFComparisonResult)(INVOKE_CALLBACK3(comparator, b, a, context)) < 0) {
                CFArrayExchangeValuesAtIndices(array, idx, idx + 1);
            }
	}
	cnt--;
    }
}

CF_PRIVATE void _CFArraySortValues(CFMutableArrayRef array, CFComparatorFunction comparator, void *context) {
    CFRange range = {0, CFArrayGetCount(array)};
    if (range.length < 2) {
        return;
    }
    // implemented abstractly, careful!
    const void **values, *buffer[256];
    values = (range.length <= 256) ? (const void **)buffer : (const void **)CFAllocatorAllocate(kCFAllocatorSystemDefault, range.length * sizeof(void *), 0); // GC OK
    CFArrayGetValues(array, range, values);
    struct _acompareContext ctx;
    ctx.func = comparator;
    ctx.context = context;
    CFQSortArray(values, range.length, sizeof(void *), (CFComparatorFunction)__CFArrayCompareValues, &ctx);
    CFArrayReplaceValues(array, range, values, range.length);
    if (values != buffer) CFAllocatorDeallocate(kCFAllocatorSystemDefault, values);
}

void CFArraySortValues(CFMutableArrayRef array, CFRange range, CFComparatorFunction comparator, void *context) {
    FAULT_CALLBACK((void **)&(comparator));
    __CFArrayValidateRange(array, range, __PRETTY_FUNCTION__);
    CFAssert1(NULL != comparator, __kCFLogAssertion, "%s(): pointer to comparator function may not be NULL", __PRETTY_FUNCTION__);
    Boolean immutable = false;
    if (CF_IS_OBJC(CFArrayGetTypeID(), array)) {
        BOOL result;
        result = CF_OBJC_CALLV((NSMutableArray *)array, isKindOfClass:[NSMutableArray class]);
        immutable = !result;
    } else if (__kCFArrayImmutable == __CFArrayGetType(array)) {
        immutable = true;
    }
    const CFArrayCallBacks *cb = NULL;
    if (CF_IS_OBJC(CFArrayGetTypeID(), array)) {
        cb = &kCFTypeArrayCallBacks;
    } else {
        cb = __CFArrayGetCallBacks(array);
    }
    if (!immutable && ((cb->retain && !cb->release) || (!cb->retain && cb->release))) {
	__CFZSort(array, range, comparator, context);
	return;
    }
    if (range.length < 2) {
        return;
    }
    // implemented abstractly, careful!
    const void **values, *buffer[256];
    values = (range.length <= 256) ? (const void **)buffer : (const void **)CFAllocatorAllocate(kCFAllocatorSystemDefault, range.length * sizeof(void *), 0); // GC OK
    CFArrayGetValues(array, range, values);
    struct _acompareContext ctx;
    ctx.func = comparator;
    ctx.context = context;
    CFQSortArray(values, range.length, sizeof(void *), (CFComparatorFunction)__CFArrayCompareValues, &ctx);
    if (!immutable) CFArrayReplaceValues(array, range, values, range.length);
    if (values != buffer) CFAllocatorDeallocate(kCFAllocatorSystemDefault, values);
}

CFIndex CFArrayBSearchValues(CFArrayRef array, CFRange range, const void *value, CFComparatorFunction comparator, void *context) {
    FAULT_CALLBACK((void **)&(comparator));
    __CFArrayValidateRange(array, range, __PRETTY_FUNCTION__);
    CFAssert1(NULL != comparator, __kCFLogAssertion, "%s(): pointer to comparator function may not be NULL", __PRETTY_FUNCTION__);
    // implemented abstractly, careful!
    if (range.length <= 0) return range.location;
    const void *item = CFArrayGetValueAtIndex(array, range.location + range.length - 1);
    if ((CFComparisonResult)(INVOKE_CALLBACK3(comparator, item, value, context)) < 0) {
	return range.location + range.length;
    }
    item = CFArrayGetValueAtIndex(array, range.location);
    if ((CFComparisonResult)(INVOKE_CALLBACK3(comparator, value, item, context)) < 0) {
	return range.location;
    }
    SInt32 lg = flsl(range.length) - 1;	// lg2(range.length)
    item = CFArrayGetValueAtIndex(array, range.location + -1 + (1 << lg));
    // idx will be the current probe index into the range
    CFIndex idx = (comparator(item, value, context) < 0) ? range.length - (1 << lg) : -1;
    while (lg--) {
	item = CFArrayGetValueAtIndex(array, range.location + idx + (1 << lg));
	if (comparator(item, value, context) < 0) {
	    idx += (1 << lg);
	}
    }
    idx++;
    return idx + range.location;
}

void CFArrayAppendArray(CFMutableArrayRef array, CFArrayRef otherArray, CFRange otherRange) {
    __CFArrayValidateRange(otherArray, otherRange, __PRETTY_FUNCTION__);
    // implemented abstractly, careful!
    for (CFIndex idx = otherRange.location; idx < otherRange.location + otherRange.length; idx++) {
	CFArrayAppendValue(array, CFArrayGetValueAtIndex(otherArray, idx));
    }
}


