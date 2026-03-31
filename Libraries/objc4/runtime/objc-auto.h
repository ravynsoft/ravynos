/*
 * Copyright (c) 2004-2007 Apple Inc. All rights reserved.
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

#ifndef _OBJC_AUTO_H_
#define _OBJC_AUTO_H_

#include <objc/objc.h>
#include <malloc/malloc.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <Availability.h>
#include <TargetConditionals.h>

#include <sys/types.h>
#include <libkern/OSAtomic.h>


// Define OBJC_SILENCE_GC_DEPRECATIONS=1 to temporarily 
// silence deprecation warnings for GC functions.

#if OBJC_SILENCE_GC_DEPRECATIONS
#   define OBJC_GC_DEPRECATED(message)
#elif __has_extension(attribute_deprecated_with_message)
#   define OBJC_GC_DEPRECATED(message) __attribute__((deprecated(message ". Define OBJC_SILENCE_GC_DEPRECATIONS=1 to temporarily silence this diagnostic.")))
#else
#   define OBJC_GC_DEPRECATED(message) __attribute__((deprecated))
#endif


enum {
    OBJC_RATIO_COLLECTION        = (0 << 0),
    OBJC_GENERATIONAL_COLLECTION = (1 << 0),
    OBJC_FULL_COLLECTION         = (2 << 0),
    OBJC_EXHAUSTIVE_COLLECTION   = (3 << 0),
    
    OBJC_COLLECT_IF_NEEDED       = (1 << 3),
    OBJC_WAIT_UNTIL_DONE         = (1 << 4)
};

enum {
    OBJC_CLEAR_RESIDENT_STACK = (1 << 0)
};


#if !defined(OBJC_NO_GC)  ||  \
    (OBJC_DECLARE_SYMBOLS && !defined(OBJC_NO_GC_API))


/* Out-of-line declarations */

OBJC_EXPORT void objc_collect(unsigned long options)
    OBJC_OSX_DEPRECATED_OTHERS_UNAVAILABLE(10.6, 10.8, "it does nothing");
OBJC_EXPORT BOOL objc_collectingEnabled(void)
    OBJC_OSX_DEPRECATED_OTHERS_UNAVAILABLE(10.5, 10.8, "it always returns NO");
OBJC_EXPORT malloc_zone_t *objc_collectableZone(void) 
    OBJC_OSX_DEPRECATED_OTHERS_UNAVAILABLE(10.7, 10.8, "it always returns nil");
OBJC_EXPORT void objc_setCollectionThreshold(size_t threshold)
    OBJC_OSX_DEPRECATED_OTHERS_UNAVAILABLE(10.5, 10.8, "it does nothing");
OBJC_EXPORT void objc_setCollectionRatio(size_t ratio)
    OBJC_OSX_DEPRECATED_OTHERS_UNAVAILABLE(10.5, 10.8, "it does nothing");
OBJC_EXPORT BOOL objc_atomicCompareAndSwapPtr(id predicate, id replacement, volatile id *objectLocation) 
    OBJC_OSX_DEPRECATED_OTHERS_UNAVAILABLE(10.6, 10.8, "use OSAtomicCompareAndSwapPtr instead");
OBJC_EXPORT BOOL objc_atomicCompareAndSwapPtrBarrier(id predicate, id replacement, volatile id *objectLocation) 
    OBJC_OSX_DEPRECATED_OTHERS_UNAVAILABLE(10.6, 10.8, "use OSAtomicCompareAndSwapPtrBarrier instead");
OBJC_EXPORT BOOL objc_atomicCompareAndSwapGlobal(id predicate, id replacement, volatile id *objectLocation)
    OBJC_OSX_DEPRECATED_OTHERS_UNAVAILABLE(10.6, 10.8, "use OSAtomicCompareAndSwapPtr instead");
OBJC_EXPORT BOOL objc_atomicCompareAndSwapGlobalBarrier(id predicate, id replacement, volatile id *objectLocation)
    OBJC_OSX_DEPRECATED_OTHERS_UNAVAILABLE(10.6, 10.8, "use OSAtomicCompareAndSwapPtrBarrier instead");
OBJC_EXPORT BOOL objc_atomicCompareAndSwapInstanceVariable(id predicate, id replacement, volatile id *objectLocation)
    OBJC_OSX_DEPRECATED_OTHERS_UNAVAILABLE(10.6, 10.8, "use OSAtomicCompareAndSwapPtr instead");
OBJC_EXPORT BOOL objc_atomicCompareAndSwapInstanceVariableBarrier(id predicate, id replacement, volatile id *objectLocation)
    OBJC_OSX_DEPRECATED_OTHERS_UNAVAILABLE(10.6, 10.8, "use OSAtomicCompareAndSwapPtrBarrier instead");
OBJC_EXPORT id objc_assign_strongCast(id val, id *dest)
    OBJC_OSX_DEPRECATED_OTHERS_UNAVAILABLE(10.4, 10.8, "use a simple assignment instead");
OBJC_EXPORT id objc_assign_global(id val, id *dest)
    OBJC_OSX_DEPRECATED_OTHERS_UNAVAILABLE(10.4, 10.8, "use a simple assignment instead");
OBJC_EXPORT id objc_assign_threadlocal(id val, id *dest)
    OBJC_OSX_DEPRECATED_OTHERS_UNAVAILABLE(10.7, 10.8, "use a simple assignment instead");
OBJC_EXPORT id objc_assign_ivar(id value, id dest, ptrdiff_t offset)
    OBJC_OSX_DEPRECATED_OTHERS_UNAVAILABLE(10.4, 10.8, "use a simple assignment instead");
OBJC_EXPORT void *objc_memmove_collectable(void *dst, const void *src, size_t size)
    OBJC_OSX_DEPRECATED_OTHERS_UNAVAILABLE(10.4, 10.8, "use memmove instead");
OBJC_EXPORT id objc_read_weak(id *location)
    OBJC_OSX_DEPRECATED_OTHERS_UNAVAILABLE(10.5, 10.8, "use a simple read instead, or convert to zeroing __weak");
OBJC_EXPORT id objc_assign_weak(id value, id *location)
    OBJC_OSX_DEPRECATED_OTHERS_UNAVAILABLE(10.5, 10.8, "use a simple assignment instead, or convert to zeroing __weak");
OBJC_EXPORT void objc_registerThreadWithCollector(void)
    OBJC_OSX_DEPRECATED_OTHERS_UNAVAILABLE(10.6, 10.8, "it does nothing");
OBJC_EXPORT void objc_unregisterThreadWithCollector(void)
    OBJC_OSX_DEPRECATED_OTHERS_UNAVAILABLE(10.6, 10.8, "it does nothing");
OBJC_EXPORT void objc_assertRegisteredThreadWithCollector(void)
    OBJC_OSX_DEPRECATED_OTHERS_UNAVAILABLE(10.6, 10.8, "it does nothing");
OBJC_EXPORT void objc_clear_stack(unsigned long options)
    OBJC_OSX_DEPRECATED_OTHERS_UNAVAILABLE(10.5, 10.8, "it does nothing");
OBJC_EXPORT BOOL objc_is_finalized(void *ptr)
    OBJC_OSX_DEPRECATED_OTHERS_UNAVAILABLE(10.4, 10.8, "it always returns NO");
OBJC_EXPORT void objc_finalizeOnMainThread(Class cls)
    OBJC_OSX_DEPRECATED_OTHERS_UNAVAILABLE(10.5, 10.5, "it does nothing");
OBJC_EXPORT BOOL objc_collecting_enabled(void)
    OBJC_OSX_DEPRECATED_OTHERS_UNAVAILABLE(10.4, 10.5, "it always returns NO");
OBJC_EXPORT void objc_set_collection_threshold(size_t threshold)
    OBJC_OSX_DEPRECATED_OTHERS_UNAVAILABLE(10.4, 10.5, "it does nothing");
OBJC_EXPORT void objc_set_collection_ratio(size_t ratio)
    OBJC_OSX_DEPRECATED_OTHERS_UNAVAILABLE(10.4, 10.5, "it does nothing");
OBJC_EXPORT void objc_start_collector_thread(void)
    OBJC_OSX_DEPRECATED_OTHERS_UNAVAILABLE(10.4, 10.5, "it does nothing");
OBJC_EXPORT void objc_startCollectorThread(void)
    OBJC_OSX_DEPRECATED_OTHERS_UNAVAILABLE(10.5, 10.7, "it does nothing");
OBJC_EXPORT id objc_allocate_object(Class cls, int extra)
    OBJC_OSX_DEPRECATED_OTHERS_UNAVAILABLE(10.4, 10.4, "use class_createInstance instead");


/* !defined(OBJC_NO_GC) */
#else
/* defined(OBJC_NO_GC) */


/* Inline declarations */

OBJC_GC_DEPRECATED("it does nothing")
static OBJC_INLINE void objc_collect(unsigned long options __unused) { }
OBJC_GC_DEPRECATED("it always returns NO")
static OBJC_INLINE BOOL objc_collectingEnabled(void) { return NO; }
#if TARGET_OS_OSX
OBJC_GC_DEPRECATED("it always returns nil")
static OBJC_INLINE malloc_zone_t *objc_collectableZone(void) { return nil; }
#endif
OBJC_GC_DEPRECATED("it does nothing")
static OBJC_INLINE void objc_setCollectionThreshold(size_t threshold __unused) { }
OBJC_GC_DEPRECATED("it does nothing")
static OBJC_INLINE void objc_setCollectionRatio(size_t ratio __unused) { }
OBJC_GC_DEPRECATED("it does nothing")
static OBJC_INLINE void objc_startCollectorThread(void) { }

#if __has_feature(objc_arc)

/* Covers for GC memory operations are unavailable in ARC */

#else

OBJC_GC_DEPRECATED("use OSAtomicCompareAndSwapPtr instead")
static OBJC_INLINE BOOL objc_atomicCompareAndSwapPtr(id predicate, id replacement, volatile id *objectLocation) 
    { return OSAtomicCompareAndSwapPtr((void *)predicate, (void *)replacement, (void * volatile *)objectLocation); }

OBJC_GC_DEPRECATED("use OSAtomicCompareAndSwapPtrBarrier instead")
static OBJC_INLINE BOOL objc_atomicCompareAndSwapPtrBarrier(id predicate, id replacement, volatile id *objectLocation) 
    { return OSAtomicCompareAndSwapPtrBarrier((void *)predicate, (void *)replacement, (void * volatile *)objectLocation); }

OBJC_GC_DEPRECATED("use OSAtomicCompareAndSwapPtr instead")
static OBJC_INLINE BOOL objc_atomicCompareAndSwapGlobal(id predicate, id replacement, volatile id *objectLocation) 
    { return objc_atomicCompareAndSwapPtr(predicate, replacement, objectLocation); }

OBJC_GC_DEPRECATED("use OSAtomicCompareAndSwapPtrBarrier instead")
static OBJC_INLINE BOOL objc_atomicCompareAndSwapGlobalBarrier(id predicate, id replacement, volatile id *objectLocation) 
    { return objc_atomicCompareAndSwapPtrBarrier(predicate, replacement, objectLocation); }

OBJC_GC_DEPRECATED("use OSAtomicCompareAndSwapPtr instead")
static OBJC_INLINE BOOL objc_atomicCompareAndSwapInstanceVariable(id predicate, id replacement, volatile id *objectLocation) 
    { return objc_atomicCompareAndSwapPtr(predicate, replacement, objectLocation); }

OBJC_GC_DEPRECATED("use OSAtomicCompareAndSwapPtrBarrier instead")
static OBJC_INLINE BOOL objc_atomicCompareAndSwapInstanceVariableBarrier(id predicate, id replacement, volatile id *objectLocation) 
    { return objc_atomicCompareAndSwapPtrBarrier(predicate, replacement, objectLocation); }


OBJC_GC_DEPRECATED("use a simple assignment instead")
static OBJC_INLINE id objc_assign_strongCast(id val, id *dest) 
    { return (*dest = val); }

OBJC_GC_DEPRECATED("use a simple assignment instead")
static OBJC_INLINE id objc_assign_global(id val, id *dest) 
    { return (*dest = val); }

OBJC_GC_DEPRECATED("use a simple assignment instead")
static OBJC_INLINE id objc_assign_threadlocal(id val, id *dest) 
    { return (*dest = val); }

OBJC_GC_DEPRECATED("use a simple assignment instead")
static OBJC_INLINE id objc_assign_ivar(id val, id dest, ptrdiff_t offset) 
    { return (*(id*)((intptr_t)(char *)dest+offset) = val); }

OBJC_GC_DEPRECATED("use a simple read instead, or convert to zeroing __weak")
static OBJC_INLINE id objc_read_weak(id *location) 
    { return *location; }

OBJC_GC_DEPRECATED("use a simple assignment instead, or convert to zeroing __weak")
static OBJC_INLINE id objc_assign_weak(id value, id *location) 
    { return (*location = value); }

/* MRC */
#endif

OBJC_GC_DEPRECATED("use memmove instead")
static OBJC_INLINE void *objc_memmove_collectable(void *dst, const void *src, size_t size) 
    { return memmove(dst, src, size); }

OBJC_GC_DEPRECATED("it does nothing")
static OBJC_INLINE void objc_finalizeOnMainThread(Class cls __unused) { }
OBJC_GC_DEPRECATED("it always returns NO")
static OBJC_INLINE BOOL objc_is_finalized(void *ptr __unused) { return NO; }
OBJC_GC_DEPRECATED("it does nothing")
static OBJC_INLINE void objc_clear_stack(unsigned long options __unused) { }
OBJC_GC_DEPRECATED("it always returns NO")
static OBJC_INLINE BOOL objc_collecting_enabled(void) { return NO; }
OBJC_GC_DEPRECATED("it does nothing")
static OBJC_INLINE void objc_set_collection_threshold(size_t threshold __unused) { } 
OBJC_GC_DEPRECATED("it does nothing")
static OBJC_INLINE void objc_set_collection_ratio(size_t ratio __unused) { } 
OBJC_GC_DEPRECATED("it does nothing")
static OBJC_INLINE void objc_start_collector_thread(void) { }

#if __has_feature(objc_arc)
extern id objc_allocate_object(Class cls, int extra) UNAVAILABLE_ATTRIBUTE;
#else
OBJC_EXPORT id class_createInstance(Class cls, size_t extraBytes)
    OBJC_AVAILABLE(10.0, 2.0, 9.0, 1.0, 2.0);
OBJC_GC_DEPRECATED("use class_createInstance instead")
static OBJC_INLINE id objc_allocate_object(Class cls, int extra) 
    { return class_createInstance(cls, (size_t)extra); }
#endif

OBJC_GC_DEPRECATED("it does nothing")
static OBJC_INLINE void objc_registerThreadWithCollector() { }
OBJC_GC_DEPRECATED("it does nothing")
static OBJC_INLINE void objc_unregisterThreadWithCollector() { }
OBJC_GC_DEPRECATED("it does nothing")
static OBJC_INLINE void objc_assertRegisteredThreadWithCollector() { }

/* defined(OBJC_NO_GC) */
#endif


#endif
