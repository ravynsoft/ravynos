#if defined(__clang__) && !defined(__OBJC_RUNTIME_INTERNAL__)
#pragma clang system_header
#endif
#include "objc-visibility.h"

#ifndef __OBJC_ARC_INCLUDED__
#define __OBJC_ARC_INCLUDED__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Autoreleases the argument.  Equivalent to [obj autorelease].
 */
OBJC_PUBLIC id objc_autorelease(id obj);
/**
 * Autoreleases a return value.  This is equivalent to [obj autorelease], but
 * may also store the object somewhere where it can be quickly removed without
 * the need for any message sending.
 */
OBJC_PUBLIC id objc_autoreleaseReturnValue(id obj);
/**
 * Initializes object as a weak pointer and stores value in it, or nil if value
 * has already begun deallocation.
 */
OBJC_PUBLIC id objc_initWeak(id *object, id value);
/**
 * Loads the object.  Returns nil if the object stored at this address has
 * already begun deallocation.
 */
OBJC_PUBLIC id objc_loadWeak(id* object);
/**
 * Loads a weak value and retains it.
 */
OBJC_PUBLIC id objc_loadWeakRetained(id* obj);
/**
 * Retains the argument.  Equivalent to [obj retain].
 */
OBJC_PUBLIC id objc_retain(id obj);
/**
 * Retains the argument, assuming that the argument is a normal object and has
 * its reference count managed by the runtime.
 * This is intended to implement `-retain` in ARC-compatible root classes.
 */
OBJC_PUBLIC id objc_retain_fast_np(id obj) OBJC_NONPORTABLE;
/**
 * Retains and autoreleases an object.  Equivalent to [[obj retain] autorelease].
 */
OBJC_PUBLIC id objc_retainAutorelease(id obj);
/**
 * Retains and releases a return value.  Equivalent to
 * objc_retain(objc_autoreleaseReturnValue(obj)).
 */
OBJC_PUBLIC id objc_retainAutoreleaseReturnValue(id obj);
/**
 * Retains a return value that has previously been autoreleased and returned.
 * This is equivalent to objc_retainAutoreleaseReturnValue(), but may support a
 * fast path, skipping the autorelease pool entirely.
 */
OBJC_PUBLIC id objc_retainAutoreleasedReturnValue(id obj);
/**
 * Retains a block.
 */
OBJC_PUBLIC id objc_retainBlock(id b);
/**
 * Stores value in addr.  This first retains value, then releases the old value
 * at addr, and stores the retained value in the address.
 */
OBJC_PUBLIC id objc_storeStrong(id *addr, id value);
/**
 * Stores obj in zeroing weak pointer addr.  If obj has begun deallocation,
 * then this stores nil.
 */
OBJC_PUBLIC id objc_storeWeak(id *addr, id obj);
/**
 * Allocates an autorelease pool and pushes it onto the top of the autorelease
 * pool stack.  Note that the returned autorelease pool is not required to be
 * an object.
 */
OBJC_PUBLIC void *objc_autoreleasePoolPush(void);
/**
 * Pops the specified autorelease pool from the stack, sending release messages
 * to every object that has been autreleased since the pool was created.
 */
OBJC_PUBLIC void objc_autoreleasePoolPop(void *pool);
/**
 * Initializes dest as a weak pointer and stores the value stored in src into
 * it.  
 */
OBJC_PUBLIC void objc_copyWeak(id *dest, id *src);
/**
 * Destroys addr as a weak pointer.
 */
OBJC_PUBLIC void objc_destroyWeak(id* addr);
/**
 * Equivalent to objc_copyWeak(), but may also set src to nil.
 */
OBJC_PUBLIC void objc_moveWeak(id *dest, id *src);
/**
 * Releases the argument, assuming that the argument is a normal object and has
 * its reference count managed by the runtime.  If the retain count reaches
 * zero then all weak references will be zeroed and the object will be
 * destroyed.
 *
 * This is intended to implement `-release` in ARC-compatible root
 * classes.
 */
OBJC_PUBLIC void objc_release_fast_np(id obj) OBJC_NONPORTABLE;
/**
 * Releases the argument, assuming that the argument is a normal object and has
 * its reference count managed by the runtime.  If the retain count reaches
 * zero then all weak references will be zeroed but the object will *NOT* be
 * destroyed.
 *
 * This is intended to implement `NSDecrementExtraRefCountWasZero` for use with
 * ARC-compatible classes.
 */
OBJC_PUBLIC BOOL objc_release_fast_no_destroy_np(id obj) OBJC_NONPORTABLE;
/**
 * Returns the retain count of an object.
 */
OBJC_PUBLIC size_t object_getRetainCount_np(id obj) OBJC_NONPORTABLE;
/**
 * Releases an object.  Equivalent to [obj release].
 */
OBJC_PUBLIC void objc_release(id obj);
/**
 * Mark the object as about to begin deallocation.  All subsequent reads of
 * weak pointers will return 0.  This function should be called in -release,
 * before calling [self dealloc].
 *
 * This will return `YES` if the weak references were deleted, `NO` otherwise.
 *
 * Nonstandard extension.
 */
OBJC_PUBLIC BOOL objc_delete_weak_refs(id obj);
/**
 * Returns the total number of objects in the ARC-managed autorelease pool.
 */
OBJC_PUBLIC unsigned long objc_arc_autorelease_count_np(void);
/**
 * Returns the total number of times that an object has been autoreleased in
 * this thread.
 */
OBJC_PUBLIC unsigned long objc_arc_autorelease_count_for_object_np(id);

#ifdef __cplusplus
}
#endif

#endif // __OBJC_ARC_INCLUDED__

