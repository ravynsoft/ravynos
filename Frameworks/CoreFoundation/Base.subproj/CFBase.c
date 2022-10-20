/*	CFBase.c
	Copyright (c) 1998-2019, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
	Responsibility: Michael LeHew
*/

#include <CoreFoundation/CFBase.h>
#include "CFInternal.h"
#include "CFRuntime_Internal.h"
#if __has_include(<unistd.h>)
#include <unistd.h>
#endif
#if _POSIX_THREADS
#include <pthread.h>
#endif
#if TARGET_OS_MAC && !__RAVYNOS__
#include <malloc/malloc.h>
#include <mach/mach.h>
#include <dlfcn.h>
#endif
#include <stdlib.h>
#include <string.h>

// -------- -------- -------- -------- -------- -------- -------- --------

struct __CFAllocator {
    CFRuntimeBase _base;
#if TARGET_OS_MAC && !__RAVYNOS__
    // CFAllocator structure must match struct _malloc_zone_t!
    // The first two reserved fields in struct _malloc_zone_t are for us with CFRuntimeBase
    size_t 	(*size)(struct _malloc_zone_t *zone, const void *ptr); /* returns the size of a block or 0 if not in this zone; must be fast, especially for negative answers */
    void 	*(*malloc)(struct _malloc_zone_t *zone, size_t size);
    void 	*(*calloc)(struct _malloc_zone_t *zone, size_t num_items, size_t size); /* same as malloc, but block returned is set to zero */
    void 	*(*valloc)(struct _malloc_zone_t *zone, size_t size); /* same as malloc, but block returned is set to zero and is guaranteed to be page aligned */
    void 	(*free)(struct _malloc_zone_t *zone, void *ptr);
    void 	*(*realloc)(struct _malloc_zone_t *zone, void *ptr, size_t size);
    void 	(*destroy)(struct _malloc_zone_t *zone); /* zone is destroyed and all memory reclaimed */
    const char	*zone_name;

    /* Optional batch callbacks; these may be NULL */
    unsigned	(*batch_malloc)(struct _malloc_zone_t *zone, size_t size, void **results, unsigned num_requested); /* given a size, returns pointers capable of holding that size; returns the number of pointers allocated (maybe 0 or less than num_requested) */
    void	(*batch_free)(struct _malloc_zone_t *zone, void **to_be_freed, unsigned num_to_be_freed); /* frees all the pointers in to_be_freed; note that to_be_freed may be overwritten during the process */

    const struct malloc_introspection_t	*introspect;
    unsigned	version;
    
    /* aligned memory allocation. The callback may be NULL. */
	void *(*memalign)(struct _malloc_zone_t *zone, size_t alignment, size_t size);
    
    /* free a pointer known to be in zone and known to have the given size. The callback may be NULL. */
    void (*free_definite_size)(struct _malloc_zone_t *zone, void *ptr, size_t size);
#endif
    CFAllocatorRef _allocator;
    CFAllocatorContext _context;
};

CF_INLINE uintptr_t __CFISAForCFAllocator(void) {
    return _GetCFRuntimeObjcClassAtIndex(_kCFRuntimeIDCFAllocator);
}

CF_INLINE CFAllocatorRetainCallBack __CFAllocatorGetRetainFunction(const CFAllocatorContext *context) {
    CFAllocatorRetainCallBack retval = NULL;
	retval = context->retain;
    return retval;
}

CF_INLINE CFAllocatorReleaseCallBack __CFAllocatorGetReleaseFunction(const CFAllocatorContext *context) {
    CFAllocatorReleaseCallBack retval = NULL;
	retval = context->release;
    return retval;
}

CF_INLINE CFAllocatorCopyDescriptionCallBack __CFAllocatorGetCopyDescriptionFunction(const CFAllocatorContext *context) {
    CFAllocatorCopyDescriptionCallBack retval = NULL;
	retval = context->copyDescription;
    return retval;
}

CF_INLINE CFAllocatorAllocateCallBack __CFAllocatorGetAllocateFunction(const CFAllocatorContext *context) {
    CFAllocatorAllocateCallBack retval = NULL;
	retval = context->allocate;
    return retval;
}

CF_INLINE CFAllocatorReallocateCallBack __CFAllocatorGetReallocateFunction(const CFAllocatorContext *context) {
    CFAllocatorReallocateCallBack retval = NULL;
	retval = context->reallocate;
    return retval;
}

CF_INLINE CFAllocatorDeallocateCallBack __CFAllocatorGetDeallocateFunction(const CFAllocatorContext *context) {
    CFAllocatorDeallocateCallBack retval = NULL;
	retval = context->deallocate;
    return retval;
}

CF_INLINE CFAllocatorPreferredSizeCallBack __CFAllocatorGetPreferredSizeFunction(const CFAllocatorContext *context) {
    CFAllocatorPreferredSizeCallBack retval = NULL;
	retval = context->preferredSize;
    return retval;
}

static const void * const __MallocDefaultZoneInfoPlaceholder = NULL;

#if TARGET_OS_MAC && !__RAVYNOS__

CF_PRIVATE void __CFAllocatorDeallocate(CFTypeRef cf);

static kern_return_t __CFAllocatorZoneIntrospectNoOp(void) {
    return 0;
}

static boolean_t __CFAllocatorZoneIntrospectTrue(void) {
    return 1;
}

static size_t __CFAllocatorCustomSize(malloc_zone_t *zone, const void *ptr) {
    return 0;

    // The only way to implement this with a version 0 allocator would be
    // for CFAllocator to keep track of all blocks allocated itself, which
    // could be done, but would be bad for performance, so we don't do it.
    //    size_t (*size)(struct _malloc_zone_t *zone, const void *ptr);
    /* returns the size of a block or 0 if not in this zone;
     * must be fast, especially for negative answers */
}

static void *__CFAllocatorCustomMalloc(malloc_zone_t *zone, size_t size) {
    CFAllocatorRef allocator = (CFAllocatorRef)zone;
    return CFAllocatorAllocate(allocator, size, 0);
}

static void *__CFAllocatorCustomCalloc(malloc_zone_t *zone, size_t num_items, size_t size) {
    CFAllocatorRef allocator = (CFAllocatorRef)zone;
    void *newptr = CFAllocatorAllocate(allocator, size, 0);
    if (newptr) memset(newptr, 0, size);
    return newptr;
}

static void *__CFAllocatorCustomValloc(malloc_zone_t *zone, size_t size) {
    CFAllocatorRef allocator = (CFAllocatorRef)zone;
    if (size >= ULONG_MAX - 2 * vm_page_size) return NULL; // avoid integer overflow plus don't allow all pages to be allocated either
    void *newptr = CFAllocatorAllocate(allocator, size + vm_page_size, 0);
    newptr = (void *)round_page((uintptr_t)newptr);
    return newptr;
}

static void __CFAllocatorCustomFree(malloc_zone_t *zone, void *ptr) {
    CFAllocatorRef allocator = (CFAllocatorRef)zone;
    CFAllocatorDeallocate(allocator, ptr);
}

static void *__CFAllocatorCustomRealloc(malloc_zone_t *zone, void *ptr, size_t size) {
    CFAllocatorRef allocator = (CFAllocatorRef)zone;
    return CFAllocatorReallocate(allocator, ptr, size, 0);
}

static void __CFAllocatorCustomDestroy(malloc_zone_t *zone) {
    CFAllocatorRef allocator = (CFAllocatorRef)zone;
    // !!! we do it, and caller of malloc_destroy_zone() assumes
    // COMPLETE responsibility for the result; NO Apple library
    // code should be modified as a result of discovering that
    // some activity results in inconveniences to developers
    // trying to use malloc_destroy_zone() with a CFAllocatorRef;
    // that's just too bad for them.
    __CFAllocatorDeallocate(allocator);
}

static size_t __CFAllocatorCustomGoodSize(malloc_zone_t *zone, size_t size) {
    CFAllocatorRef allocator = (CFAllocatorRef)zone;
    return CFAllocatorGetPreferredSizeForSize(allocator, size, 0);
}

static const struct malloc_introspection_t __CFAllocatorZoneIntrospect = {
    (void *)__CFAllocatorZoneIntrospectNoOp,
    (void *)__CFAllocatorCustomGoodSize,
    (void *)__CFAllocatorZoneIntrospectTrue,
    (void *)__CFAllocatorZoneIntrospectNoOp,
    (void *)__CFAllocatorZoneIntrospectNoOp,
    (void *)__CFAllocatorZoneIntrospectNoOp,
    (void *)__CFAllocatorZoneIntrospectNoOp,
    (void *)__CFAllocatorZoneIntrospectNoOp
};

static size_t __CFAllocatorNullSize(malloc_zone_t *zone, const void *ptr) {
    return 0;
}

static void * __CFAllocatorNullMalloc(malloc_zone_t *zone, size_t size) {
    return NULL;
}

static void * __CFAllocatorNullCalloc(malloc_zone_t *zone, size_t num_items, size_t size) {
    return NULL;
}

static void * __CFAllocatorNullValloc(malloc_zone_t *zone, size_t size) {
    return NULL;
}

static void __CFAllocatorNullFree(malloc_zone_t *zone, void *ptr) {
}

static void * __CFAllocatorNullRealloc(malloc_zone_t *zone, void *ptr, size_t size) {
    return NULL;
}

static void __CFAllocatorNullDestroy(malloc_zone_t *zone) {
}

static size_t __CFAllocatorNullGoodSize(malloc_zone_t *zone, size_t size) {
    return size;
}

static const struct malloc_introspection_t __CFAllocatorNullZoneIntrospect = {
    (void *)__CFAllocatorZoneIntrospectNoOp,
    (void *)__CFAllocatorNullGoodSize,
    (void *)__CFAllocatorZoneIntrospectTrue,
    (void *)__CFAllocatorZoneIntrospectNoOp,
    (void *)__CFAllocatorZoneIntrospectNoOp,
    (void *)__CFAllocatorZoneIntrospectNoOp,
    (void *)__CFAllocatorZoneIntrospectNoOp,
    (void *)__CFAllocatorZoneIntrospectNoOp
};

static void *__CFAllocatorSystemAllocate(CFIndex size, CFOptionFlags hint, void *info) {
    malloc_zone_t * const zone = (info == &__MallocDefaultZoneInfoPlaceholder) ? malloc_default_zone() : (malloc_zone_t *)info;
    void *result = NULL;
    if (hint == _CFAllocatorHintZeroWhenAllocating) {
        result = malloc_zone_calloc(zone, 1, size);
    } else {
        result = malloc_zone_malloc(zone, size);
    }
    return result;
}

static void *__CFAllocatorSystemReallocate(void *ptr, CFIndex newsize, CFOptionFlags hint, void *info) {
    malloc_zone_t * const zone = (info == &__MallocDefaultZoneInfoPlaceholder) ? malloc_default_zone() : (malloc_zone_t *)info;
    return malloc_zone_realloc(zone, ptr, newsize);
}

static void __CFAllocatorSystemDeallocate(void *ptr, void *info) {
    malloc_zone_t * const zone = (info == &__MallocDefaultZoneInfoPlaceholder) ? malloc_default_zone() : (malloc_zone_t *)info;
#if defined(DEBUG)
    size_t size = malloc_size(ptr);
    if (size) memset(ptr, 0xCC, size);
#endif
    malloc_zone_free(zone, ptr);
}

#else

static void *__CFAllocatorSystemAllocate(CFIndex size, CFOptionFlags hint, void *info) {
    if (hint == _CFAllocatorHintZeroWhenAllocating) {
        return calloc(1, size);
    } else {
        return malloc(size);
    }
}

static void *__CFAllocatorSystemReallocate(void *ptr, CFIndex newsize, CFOptionFlags hint, void *info) {
    return realloc(ptr, newsize);
}

static void __CFAllocatorSystemDeallocate(void *ptr, void *info) {
    free(ptr);
}
#endif

static void *__CFAllocatorNullAllocate(CFIndex size, CFOptionFlags hint, void *info) {
    return NULL;
}

static void *__CFAllocatorNullReallocate(void *ptr, CFIndex newsize, CFOptionFlags hint, void *info) {
    return NULL;
}

#if defined (__cplusplus)
static void * __CFAllocatorCPPMalloc(CFIndex allocSize, CFOptionFlags hint, void *info)
{
	return malloc(allocSize);	
}
static void * __CFAllocatorCPPReAlloc(void *ptr, CFIndex newsize, CFOptionFlags hint, void *info)
{
	return realloc(ptr, newsize);
}
static void __CFAllocatorCPPFree(void *ptr, void *info)
{
	free(ptr);
}
#endif // C++


static _CF_CONSTANT_OBJECT_BACKING struct __CFAllocator __kCFAllocatorSystemDefault;

DECLARE_STATIC_CLASS_REF(__NSCFType);

static _CF_CONSTANT_OBJECT_BACKING struct __CFAllocator __kCFAllocatorMalloc = {
    INIT_CFRUNTIME_BASE_WITH_CLASS(__NSCFType, _kCFRuntimeIDCFAllocator),
#if TARGET_OS_MAC && !__RAVYNOS__
    __CFAllocatorCustomSize,
    __CFAllocatorCustomMalloc,
    __CFAllocatorCustomCalloc,
    __CFAllocatorCustomValloc,
    __CFAllocatorCustomFree,
    __CFAllocatorCustomRealloc,
    __CFAllocatorNullDestroy,
    "kCFAllocatorMalloc",
    NULL,
    NULL,
    &__CFAllocatorZoneIntrospect,
    6,
    NULL,
    NULL,
#endif
    &__kCFAllocatorSystemDefault,	// _allocator
    // Using the malloc functions directly is a total cheat, but works (in C)
    // because the function signatures match in their common prefix of arguments.
    // This saves us one hop through an adaptor function.
#if !defined (__cplusplus)
	{0, NULL, NULL, NULL, NULL, (void *)malloc, (void *)realloc, (void *)free, NULL}
#else
	{0, NULL, NULL, NULL, NULL, __CFAllocatorCPPMalloc,__CFAllocatorCPPReAlloc, __CFAllocatorCPPFree, NULL}
#endif // __cplusplus
};

static _CF_CONSTANT_OBJECT_BACKING struct __CFAllocator __kCFAllocatorMallocZone = {
    INIT_CFRUNTIME_BASE_WITH_CLASS(__NSCFType, _kCFRuntimeIDCFAllocator),
#if TARGET_OS_MAC && !__RAVYNOS__
    __CFAllocatorCustomSize,
    __CFAllocatorCustomMalloc,
    __CFAllocatorCustomCalloc,
    __CFAllocatorCustomValloc,
    __CFAllocatorCustomFree,
    __CFAllocatorCustomRealloc,
    __CFAllocatorNullDestroy,
    "kCFAllocatorMallocZone",
    NULL,
    NULL,
    &__CFAllocatorZoneIntrospect,
    6,
    NULL,
    NULL,
#endif
    &__kCFAllocatorSystemDefault,	// _allocator
    {0, (void *)&__MallocDefaultZoneInfoPlaceholder, NULL, NULL, NULL, __CFAllocatorSystemAllocate, __CFAllocatorSystemReallocate, __CFAllocatorSystemDeallocate, NULL}
};

static _CF_CONSTANT_OBJECT_BACKING struct __CFAllocator __kCFAllocatorSystemDefault = {
    INIT_CFRUNTIME_BASE_WITH_CLASS(__NSCFType, _kCFRuntimeIDCFAllocator),
#if TARGET_OS_MAC && !__RAVYNOS__
    __CFAllocatorCustomSize,
    __CFAllocatorCustomMalloc,
    __CFAllocatorCustomCalloc,
    __CFAllocatorCustomValloc,
    __CFAllocatorCustomFree,
    __CFAllocatorCustomRealloc,
    __CFAllocatorNullDestroy,
    "kCFAllocatorSystemDefault",
    NULL,
    NULL,
    &__CFAllocatorZoneIntrospect,
    6,
    NULL,
    NULL,
#endif
    &__kCFAllocatorSystemDefault,	// _allocator
    {0, (void *)&__MallocDefaultZoneInfoPlaceholder, NULL, NULL, NULL, __CFAllocatorSystemAllocate, __CFAllocatorSystemReallocate, __CFAllocatorSystemDeallocate, NULL}
};

static _CF_CONSTANT_OBJECT_BACKING struct __CFAllocator __kCFAllocatorNull = {
    INIT_CFRUNTIME_BASE_WITH_CLASS(__NSCFType, _kCFRuntimeIDCFAllocator),
#if TARGET_OS_MAC && !__RAVYNOS__
    __CFAllocatorNullSize,
    __CFAllocatorNullMalloc,
    __CFAllocatorNullCalloc,
    __CFAllocatorNullValloc,
    __CFAllocatorNullFree,
    __CFAllocatorNullRealloc,
    __CFAllocatorNullDestroy,
    "kCFAllocatorNull",
    NULL,
    NULL,
    &__CFAllocatorNullZoneIntrospect,
    6,
    NULL,
    NULL,
#endif
    &__kCFAllocatorSystemDefault,	// _allocator
    {0, NULL, NULL, NULL, NULL, __CFAllocatorNullAllocate, __CFAllocatorNullReallocate, NULL, NULL}
};

const CFAllocatorRef kCFAllocatorDefault = NULL;
const CFAllocatorRef kCFAllocatorSystemDefault = &__kCFAllocatorSystemDefault;
const CFAllocatorRef kCFAllocatorMalloc = &__kCFAllocatorMalloc;
const CFAllocatorRef kCFAllocatorMallocZone = &__kCFAllocatorMallocZone;
const CFAllocatorRef kCFAllocatorNull = &__kCFAllocatorNull;
const CFAllocatorRef kCFAllocatorUseContext = (CFAllocatorRef)0x03ab;

// Even though we no longer support GC, leave in the definitions for exported symbols.
#undef kCFAllocatorSystemDefaultGCRefZero
#undef kCFAllocatorDefaultGCRefZero
const CFAllocatorRef kCFAllocatorSystemDefaultGCRefZero = (CFAllocatorRef)0x03ad;
const CFAllocatorRef kCFAllocatorDefaultGCRefZero = (CFAllocatorRef)0x03af;

static CFStringRef __CFAllocatorCopyDescription(CFTypeRef cf) {
    CFAllocatorRef self = (CFAllocatorRef)cf;
    CFAllocatorRef allocator = (kCFAllocatorUseContext == self->_allocator) ? self : self->_allocator;
    return CFStringCreateWithFormat(allocator, NULL, CFSTR("<CFAllocator %p [%p]>{info = %p}"), cf, allocator, self->_context.info);
// CF: should use copyDescription function here to describe info field
// remember to release value returned from copydescr function when this happens
}

CF_PRIVATE CFAllocatorRef __CFAllocatorGetAllocator(CFTypeRef cf) {
    CFAllocatorRef allocator = (CFAllocatorRef)cf;
    return (kCFAllocatorUseContext == allocator->_allocator) ? allocator : allocator->_allocator;
}

CF_PRIVATE void __CFAllocatorDeallocate(CFTypeRef cf) {
    CFAllocatorRef self = (CFAllocatorRef)cf;
    CFAllocatorRef allocator = self->_allocator;
    CFAllocatorReleaseCallBack releaseFunc = __CFAllocatorGetReleaseFunction(&self->_context);
    if (kCFAllocatorUseContext == allocator) {
	/* Rather a chicken and egg problem here, so we do things
	   in the reverse order from what was done at create time. */
	CFAllocatorDeallocateCallBack deallocateFunc = __CFAllocatorGetDeallocateFunction(&self->_context);
	void *info = self->_context.info;
	if (NULL != deallocateFunc) {
	    INVOKE_CALLBACK2(deallocateFunc, (void *)self, info);
	}
	if (NULL != releaseFunc) {
	    INVOKE_CALLBACK1(releaseFunc, info);
	}
    } else {
	if (NULL != releaseFunc) {
	    INVOKE_CALLBACK1(releaseFunc, self->_context.info);
	}
        CFAllocatorDeallocate(allocator, (void *)self);
    }
}

const CFRuntimeClass __CFAllocatorClass = {
    0,
    "CFAllocator",
    NULL,	// init
    NULL,	// copy
    NULL,
    NULL,	// equal
    NULL,	// hash
    NULL,	// 
    __CFAllocatorCopyDescription
};

static void _CFAllocatorSetInstanceTypeIDAndIsa(struct __CFAllocator *memory) {
    _CFRuntimeSetInstanceTypeID(memory, _kCFRuntimeIDCFAllocator);
    memory->_base._cfisa = __CFISAForCFAllocator();
}

CFTypeID CFAllocatorGetTypeID(void) {
    return _kCFRuntimeIDCFAllocator;
}

CFAllocatorRef CFAllocatorGetDefault(void) {
    return __CFGetDefaultAllocator();
}

void CFAllocatorSetDefault(CFAllocatorRef allocator) {
#ifndef __clang_analyzer__
    // clang doesn't like complexity of staticly laid out instances like the black magic we do here and  __CFGetDefaultAllocator
    CFAllocatorRef current = __CFGetDefaultAllocator();
#if defined(DEBUG) 
    if (NULL != allocator) {
	__CFGenericValidateType(allocator, _kCFRuntimeIDCFAllocator);
    }
#endif
#if TARGET_OS_MAC
    if (allocator && _CFTypeGetClass(allocator) != __CFISAForCFAllocator()) {	// malloc_zone_t *
	return;		// require allocator to this function to be an allocator
    }
#endif
    if (NULL != allocator && allocator != current) {
	if (current) CFRelease(current);
	CFRetain(allocator);
	// We retain an extra time so that anything set as the default
	// allocator never goes away.
	CFRetain(allocator);
        _CFSetTSD(__CFTSDKeyAllocator, (void *)allocator, NULL);
    }
#endif
}

#if DEPLOYMENT_RUNTIME_SWIFT
// Custom allocators are unsupported for swift-corelibs-foundation
CFAllocatorRef CFAllocatorCreate(CFAllocatorRef allocator, CFAllocatorContext *context) {
    HALT;
}
#else
static CFAllocatorRef __CFAllocatorCreate(CFAllocatorRef allocator, CFAllocatorContext *context) {
    struct __CFAllocator *memory = NULL;
    CFAllocatorRetainCallBack retainFunc;
    CFAllocatorAllocateCallBack allocateFunc;
    void *retainedInfo;
#if TARGET_OS_MAC
    if (allocator && kCFAllocatorUseContext != allocator && _CFTypeGetClass(allocator) != __CFISAForCFAllocator()) {	// malloc_zone_t *
	return NULL;	// require allocator to this function to be an allocator
    }
#endif
    retainFunc = context->retain;
    FAULT_CALLBACK((void **)&retainFunc);
    allocateFunc = context->allocate;
    FAULT_CALLBACK((void **)&allocateFunc);
    if (NULL != retainFunc) {
	retainedInfo = (void *)INVOKE_CALLBACK1(retainFunc, context->info);
    } else {
	retainedInfo = context->info;
    }
    // We don't use _CFRuntimeCreateInstance()
    if (kCFAllocatorUseContext == allocator) {
	memory = NULL;
	if (allocateFunc) {
		memory = (struct __CFAllocator *)INVOKE_CALLBACK3(allocateFunc, sizeof(struct __CFAllocator), 0, retainedInfo);
	}
	if (NULL == memory) {
	    return NULL;
	}
    } else {
	allocator = (NULL == allocator) ? __CFGetDefaultAllocator() : allocator;
	memory = (struct __CFAllocator *)CFAllocatorAllocate(allocator, sizeof(struct __CFAllocator), 0);
	if (NULL == memory) {
	    return NULL;
	}
	if (__CFOASafe) __CFSetLastAllocationEventName(memory, "CFAllocator");
    }
    memset(memory, 0, sizeof(CFRuntimeBase));
    __CFRuntimeSetRC(memory, 1);
    _CFAllocatorSetInstanceTypeIDAndIsa(memory);
#if TARGET_OS_MAC && !__RAVYNOS__
    memory->size = __CFAllocatorCustomSize;
    memory->malloc = __CFAllocatorCustomMalloc;
    memory->calloc = __CFAllocatorCustomCalloc;
    memory->valloc = __CFAllocatorCustomValloc;
    memory->free = __CFAllocatorCustomFree;
    memory->realloc = __CFAllocatorCustomRealloc;
    memory->destroy = __CFAllocatorCustomDestroy;
    memory->zone_name = "Custom CFAllocator";
    memory->batch_malloc = NULL;
    memory->batch_free = NULL;
    memory->introspect = &__CFAllocatorZoneIntrospect;
    memory->version = 6;
    memory->memalign = NULL;
    memory->free_definite_size = NULL;
#endif
    memory->_allocator = allocator;
    memory->_context.version = context->version;
    memory->_context.info = retainedInfo;
    memory->_context.retain = retainFunc;
    memory->_context.release = context->release;
    FAULT_CALLBACK((void **)&(memory->_context.release));
    memory->_context.copyDescription = context->copyDescription;
    FAULT_CALLBACK((void **)&(memory->_context.copyDescription));
    memory->_context.allocate = allocateFunc;
    memory->_context.reallocate = context->reallocate;
    FAULT_CALLBACK((void **)&(memory->_context.reallocate));
    memory->_context.deallocate = context->deallocate;
    FAULT_CALLBACK((void **)&(memory->_context.deallocate));
    memory->_context.preferredSize = context->preferredSize;
    FAULT_CALLBACK((void **)&(memory->_context.preferredSize));

    return memory;
}


CFAllocatorRef CFAllocatorCreate(CFAllocatorRef allocator, CFAllocatorContext *context) {
    return __CFAllocatorCreate(allocator, context);
}
#endif

void *CFAllocatorAllocate(CFAllocatorRef allocator, CFIndex size, CFOptionFlags hint) {
    CFAllocatorAllocateCallBack allocateFunc;
    void *newptr = NULL;

    if (NULL == allocator) {
	allocator = __CFGetDefaultAllocator();
    }

#if defined(DEBUG) && TARGET_OS_MAC
    if (_CFTypeGetClass(allocator) == __CFISAForCFAllocator()) {
	__CFGenericValidateType(allocator, _kCFRuntimeIDCFAllocator);
    }
#else
    __CFGenericValidateType(allocator, _kCFRuntimeIDCFAllocator);
#endif
    if (0 == size) return NULL;
#if TARGET_OS_MAC && !__RAVYNOS__
    if (_CFTypeGetClass(allocator) != __CFISAForCFAllocator()) {	// malloc_zone_t *
	return malloc_zone_malloc((malloc_zone_t *)allocator, size);
    }
#endif
    newptr = NULL;
    allocateFunc = __CFAllocatorGetAllocateFunction(&allocator->_context);
    if (allocateFunc) {
        newptr = (void *)INVOKE_CALLBACK3(allocateFunc, size, hint, allocator->_context.info);
    }
    return newptr;
}

void *CFAllocatorReallocate(CFAllocatorRef allocator, void *ptr, CFIndex newsize, CFOptionFlags hint) {
    CFAllocatorAllocateCallBack allocateFunc;
    CFAllocatorReallocateCallBack reallocateFunc;
    CFAllocatorDeallocateCallBack deallocateFunc;
    void *newptr;

    if (NULL == allocator) {
        allocator = __CFGetDefaultAllocator();
    }

#if defined(DEBUG) && TARGET_OS_MAC
    if (_CFTypeGetClass(allocator) == __CFISAForCFAllocator()) {
	__CFGenericValidateType(allocator, _kCFRuntimeIDCFAllocator);
    }
#else
    __CFGenericValidateType(allocator, _kCFRuntimeIDCFAllocator);
#endif
    if (NULL == ptr && 0 < newsize) {
#if TARGET_OS_MAC && !__RAVYNOS__
	if (_CFTypeGetClass(allocator) != __CFISAForCFAllocator()) {	// malloc_zone_t *
	    return malloc_zone_malloc((malloc_zone_t *)allocator, newsize);
	}
#endif
	newptr = NULL;
	allocateFunc = __CFAllocatorGetAllocateFunction(&allocator->_context);
	if (allocateFunc) {
		newptr = (void *)INVOKE_CALLBACK3(allocateFunc, newsize, hint, allocator->_context.info);
	}
	return newptr;
    }
    if (NULL != ptr && 0 == newsize) {
#if TARGET_OS_MAC && !__RAVYNOS__
	if (_CFTypeGetClass(allocator) != __CFISAForCFAllocator()) {	// malloc_zone_t *
#if defined(DEBUG)
	    size_t size = malloc_size(ptr);
	    if (size) memset(ptr, 0xCC, size);
#endif
	    malloc_zone_free((malloc_zone_t *)allocator, ptr);
	    return NULL;
	}
#endif
	deallocateFunc = __CFAllocatorGetDeallocateFunction(&allocator->_context);
	if (NULL != deallocateFunc) {
	    INVOKE_CALLBACK2(deallocateFunc, ptr, allocator->_context.info);
	}
	return NULL;
    }
    if (NULL == ptr && 0 == newsize) return NULL;
#if TARGET_OS_MAC && !__RAVYNOS__
    if (_CFTypeGetClass(allocator) != __CFISAForCFAllocator()) {	// malloc_zone_t *
	return malloc_zone_realloc((malloc_zone_t *)allocator, ptr, newsize);
    }
#endif
    reallocateFunc = __CFAllocatorGetReallocateFunction(&allocator->_context);
    if (NULL == reallocateFunc) return NULL;
    newptr = (void *)INVOKE_CALLBACK4(reallocateFunc, ptr, newsize, hint, allocator->_context.info);
    return newptr;
}

void CFAllocatorDeallocate(CFAllocatorRef allocator, void *ptr) {
    CFAllocatorDeallocateCallBack deallocateFunc;

    if (NULL == allocator) {
        allocator = __CFGetDefaultAllocator();
    }

#if defined(DEBUG) && TARGET_OS_MAC
    if (_CFTypeGetClass(allocator) == __CFISAForCFAllocator()) {
	__CFGenericValidateType(allocator, _kCFRuntimeIDCFAllocator);
    }
#else
    __CFGenericValidateType(allocator, _kCFRuntimeIDCFAllocator);
#endif
#if TARGET_OS_MAC && !__RAVYNOS__
    if (_CFTypeGetClass(allocator) != __CFISAForCFAllocator()) {	// malloc_zone_t *
#if defined(DEBUG)
	size_t size = malloc_size(ptr);
	if (size) memset(ptr, 0xCC, size);
#endif
	return malloc_zone_free((malloc_zone_t *)allocator, ptr);
    }
#endif
    deallocateFunc = __CFAllocatorGetDeallocateFunction(&allocator->_context);
    if (NULL != ptr && NULL != deallocateFunc) {
	INVOKE_CALLBACK2(deallocateFunc, ptr, allocator->_context.info);
    }
}

CFIndex CFAllocatorGetPreferredSizeForSize(CFAllocatorRef allocator, CFIndex size, CFOptionFlags hint) {
    CFAllocatorPreferredSizeCallBack prefFunc;
    CFIndex newsize = 0;

    if (NULL == allocator) {
        allocator = __CFGetDefaultAllocator();
    }

#if TARGET_OS_MAC && !__RAVYNOS__
    if (_CFTypeGetClass(allocator) == __CFISAForCFAllocator()) {
	__CFGenericValidateType(allocator, _kCFRuntimeIDCFAllocator);
    }
#else
    __CFGenericValidateType(allocator, _kCFRuntimeIDCFAllocator);
#endif
#if TARGET_OS_MAC && !__RAVYNOS__
    if (_CFTypeGetClass(allocator) != __CFISAForCFAllocator()) {	// malloc_zone_t *
	return malloc_good_size(size);
    }
#endif
    prefFunc = __CFAllocatorGetPreferredSizeFunction(&allocator->_context);
    if (0 < size && NULL != prefFunc) {
	newsize = (CFIndex)(INVOKE_CALLBACK3(prefFunc, size, hint, allocator->_context.info));
    }
    if (newsize < size) newsize = size;
    return newsize;
}

void CFAllocatorGetContext(CFAllocatorRef allocator, CFAllocatorContext *context) {
    if (NULL == allocator) {
        allocator = __CFGetDefaultAllocator();
    }

#if TARGET_OS_MAC && !__RAVYNOS__
    if (_CFTypeGetClass(allocator) == __CFISAForCFAllocator()) {
	__CFGenericValidateType(allocator, _kCFRuntimeIDCFAllocator);
    }
#else
    __CFGenericValidateType(allocator, _kCFRuntimeIDCFAllocator);
#endif
    CFAssert1(0 == context->version, __kCFLogAssertion, "%s(): context version not initialized to 0", __PRETTY_FUNCTION__);
#if TARGET_OS_MAC && !__RAVYNOS__
    if (_CFTypeGetClass(allocator) != __CFISAForCFAllocator()) {	// malloc_zone_t *
	return;
    }
#endif
    context->version = 0;
    context->info = (allocator->_context.info == &__MallocDefaultZoneInfoPlaceholder) ? (void *)malloc_default_zone() : allocator->_context.info;
    context->retain = __CFAllocatorGetRetainFunction(&allocator->_context);
    context->release = __CFAllocatorGetReleaseFunction(&allocator->_context);
    context->copyDescription = __CFAllocatorGetCopyDescriptionFunction(&allocator->_context);
    context->allocate = __CFAllocatorGetAllocateFunction(&allocator->_context);
    context->reallocate = __CFAllocatorGetReallocateFunction(&allocator->_context);
    context->deallocate = __CFAllocatorGetDeallocateFunction(&allocator->_context);
    context->preferredSize = __CFAllocatorGetPreferredSizeFunction(&allocator->_context);
}

// -------- -------- -------- -------- -------- -------- -------- --------


// Technically this function can return, but for analyzer purposes it's enough to claim it doesn't.
__attribute__((cold))
static void __CFReallocationFailed(void *ptr, CFStringRef reason, void (^reallocationFailureHandler)(void *original, bool *outRecovered)) CLANG_ANALYZER_NORETURN {
    bool recovered = false;
    if (reallocationFailureHandler) {
        reallocationFailureHandler(ptr, &recovered);
    }
    
    if (!recovered) {
        CRSetCrashLogMessage("Failed to grow buffer");
        HALT;
    }
}


void *__CFSafelyReallocate(void *destination, size_t newCapacity, void (^reallocationFailureHandler)(void *original, bool *outRecovered)) {
    void *const reallocated = realloc(destination, newCapacity);
    if (__builtin_expect(reallocated == NULL, false)) {
        __CFReallocationFailed(destination,  CFSTR("realloc"), reallocationFailureHandler);
    }
    return reallocated;
}


void *__CFSafelyReallocateWithAllocator(CFAllocatorRef allocator, void *destination, size_t newCapacity, CFOptionFlags options, void (^reallocationFailureHandler)(void *original, bool *outRecovered)) {
    void *reallocated = CFAllocatorReallocate(allocator, destination, newCapacity, options);
    // NOTE: important difference in behavior between realloc vs CFAllocateReallocate NULL+0 -> NULL for allocators!
    if (__builtin_expect(reallocated == NULL, false) && !(destination == NULL && newCapacity == 0)) {
        __CFReallocationFailed(destination,  CFSTR("realloc"), reallocationFailureHandler);
    }
    return _CLANG_ANALYZER_IGNORE_NONNULL(reallocated);;
}

Boolean __CFAllocatorRespectsHintZeroWhenAllocating(CFAllocatorRef allocator) {
    return allocator == kCFAllocatorSystemDefault || allocator == kCFAllocatorMallocZone;
}


CFRange __CFRangeMake(CFIndex loc, CFIndex len) {
    CFRange range;
    range.location = loc;
    range.length = len;
    return range;
}


struct __CFNull {
    CFRuntimeBase _base;
};

DECLARE_STATIC_CLASS_REF(NSNull);

struct __CFNull _CF_CONSTANT_OBJECT_BACKING __kCFNull = {
    INIT_CFRUNTIME_BASE_WITH_CLASS(NSNull, _kCFRuntimeIDCFNull)
};
const CFNullRef kCFNull = &__kCFNull;

static CFStringRef __CFNullCopyDescription(CFTypeRef cf) {
    return CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("<CFNull %p [%p]>"), cf, CFGetAllocator(cf));
}

static CFStringRef __CFNullCopyFormattingDescription(CFTypeRef cf, CFDictionaryRef formatOptions) {
    return (CFStringRef)CFRetain(CFSTR("null"));
}

static void __CFNullDeallocate(CFTypeRef cf) {
    CFAssert(false, __kCFLogAssertion, "Deallocated CFNull!");
}

const CFRuntimeClass __CFNullClass = {
    0,
    "CFNull",
    NULL,      // init
    NULL,      // copy
    __CFNullDeallocate,
    NULL,
    NULL,
    __CFNullCopyFormattingDescription,
    __CFNullCopyDescription
};

CFTypeID CFNullGetTypeID(void) {
    return _kCFRuntimeIDCFNull;
}

void CFCollection_non_gc_storage_error(void) { }


void _CFRuntimeSetCFMPresent(void *addr) {
}


// void __HALT(void);

/* Keep this assembly at the bottom of the source file! */


extern void __HALT(void);
void __HALT() {
    __builtin_trap();
}


