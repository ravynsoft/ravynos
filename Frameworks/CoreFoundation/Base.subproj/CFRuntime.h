/*	CFRuntime.h
	Copyright (c) 1999-2019, Apple Inc. All rights reserved.
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
*/

#if !defined(__COREFOUNDATION_CFRUNTIME__)
#define __COREFOUNDATION_CFRUNTIME__ 1

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFDictionary.h>
#include <stddef.h>

#if __has_include(<ptrauth.h>)
#include <ptrauth.h>
#endif

#ifndef __ptrauth_cf_objc_isa_pointer
#define __ptrauth_cf_objc_isa_pointer
#endif

#ifndef __ptrauth_objc_isa_pointer
#define __ptrauth_objc_isa_pointer
#endif

CF_EXTERN_C_BEGIN

#if (TARGET_OS_MAC && !TARGET_OS_IPHONE && !__x86_64h__)

// Although we no longer support GC, we leave exported symbols in place for now to avoid any lockstep dependency issues.
CF_EXPORT bool kCFUseCollectableAllocator;
CF_EXPORT bool (*__CFObjCIsCollectable)(void *);

#else

#define kCFUseCollectableAllocator 0
#define __CFObjCIsCollectable 0

#endif


CF_INLINE Boolean _CFAllocatorIsSystemDefault(CFAllocatorRef allocator) {
    if (allocator == kCFAllocatorSystemDefault) return true;
    if (NULL == allocator || kCFAllocatorDefault == allocator) {
        return (kCFAllocatorSystemDefault == CFAllocatorGetDefault());
    }
    return false;
}

#define CF_USING_COLLECTABLE_MEMORY 0
#define CF_IS_COLLECTABLE_ALLOCATOR(allocator) (0 && allocator) // prevent allocator from being claimed to be un-used
#define CF_IS_COLLECTABLE(obj) (0 && obj) // prevent obj from being claimed to be un-used

enum {
    _kCFRuntimeNotATypeID = 0
};

enum { // Version field constants
    _kCFRuntimeScannedObject =     (1UL << 0),
    _kCFRuntimeResourcefulObject = (1UL << 2),  // tells CFRuntime to make use of the reclaim field
    _kCFRuntimeCustomRefCount =    (1UL << 3),  // tells CFRuntime to make use of the refcount field
    _kCFRuntimeRequiresAlignment = (1UL << 4),  // tells CFRuntime to make use of the requiredAlignment field
};

typedef struct __CFRuntimeClass {
    CFIndex version;
    const char *className; // must be a pure ASCII string, nul-terminated
    void (*init)(CFTypeRef cf);
    CFTypeRef (*copy)(CFAllocatorRef allocator, CFTypeRef cf);
    void (*finalize)(CFTypeRef cf);
    Boolean (*equal)(CFTypeRef cf1, CFTypeRef cf2);
    CFHashCode (*hash)(CFTypeRef cf);
    CFStringRef (*copyFormattingDesc)(CFTypeRef cf, CFDictionaryRef formatOptions);	// return str with retain
    CFStringRef (*copyDebugDesc)(CFTypeRef cf);	// return str with retain

#define CF_RECLAIM_AVAILABLE 1
    void (*reclaim)(CFTypeRef cf); // Or in _kCFRuntimeResourcefulObject in the .version to indicate this field should be used

#define CF_REFCOUNT_AVAILABLE 1
    uint32_t (*refcount)(intptr_t op, CFTypeRef cf); // Or in _kCFRuntimeCustomRefCount in the .version to indicate this field should be used
        // this field must be non-NULL when _kCFRuntimeCustomRefCount is in the .version field
        // - if the callback is passed 1 in 'op' it should increment the 'cf's reference count and return 0
        // - if the callback is passed 0 in 'op' it should return the 'cf's reference count, up to 32 bits
        // - if the callback is passed -1 in 'op' it should decrement the 'cf's reference count; if it is now zero, 'cf' should be cleaned up and deallocated; then return 0
        // remember to use saturation arithmetic logic and stop incrementing and decrementing when the ref count hits UINT32_MAX, or you will have a security bug
        // remember that reference count incrementing/decrementing must be done thread-safely/atomically
        // objects should be created/initialized with a custom ref-count of 1 by the class creation functions
        // do not attempt to use any bits within the CFRuntimeBase for your reference count; store that in some additional field in your CF object

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#define CF_REQUIRED_ALIGNMENT_AVAILABLE 1
    uintptr_t requiredAlignment; // Or in _kCFRuntimeRequiresAlignment in the .version field to indicate this field should be used; the allocator to _CFRuntimeCreateInstance() will be ignored in this case; if this is less than the minimum alignment the system supports, you'll get higher alignment; if this is not an alignment the system supports (e.g., most systems will only support powers of two, or if it is too high), the result (consequences) will be up to CF or the system to decide

} CFRuntimeClass;

#define RADAR_5115468_FIXED 1

/* Note that CF runtime class registration and unregistration is not currently
 * thread-safe, which should not currently be a problem, as long as unregistration
 * is done only when valid to do so.
 */

CF_EXPORT CFTypeID _CFRuntimeRegisterClass(const CFRuntimeClass * const cls);
	/* Registers a new class with the CF runtime.  Pass in a
	 * pointer to a CFRuntimeClass structure.  The pointer is
	 * remembered by the CF runtime -- the structure is NOT
	 * copied.
	 *
	 * - version field must be zero currently.
	 * - className field points to a null-terminated C string
	 *   containing only ASCII (0 - 127) characters; this field
	 *   may NOT be NULL.
	 * - init field points to a function which classes can use to
	 *   apply some generic initialization to instances as they
	 *   are created; this function is called by both
	 *   _CFRuntimeCreateInstance and _CFRuntimeInitInstance; if
	 *   this field is NULL, no function is called; the instance
	 *   has been initialized enough that the polymorphic funcs
	 *   CFGetTypeID(), CFRetain(), CFRelease(), CFGetRetainCount(),
	 *   and CFGetAllocator() are valid on it when the init
	 *   function if any is called.
         * - copy field should always be NULL. Generic copying of CF
         *   objects has never been defined (and is unlikely).
	 * - finalize field points to a function which destroys an
	 *   instance when the retain count has fallen to zero; if
	 *   this is NULL, finalization does nothing. Note that if
	 *   the class-specific functions which create or initialize
	 *   instances more fully decide that a half-initialized
	 *   instance must be destroyed, the finalize function for
	 *   that class has to be able to deal with half-initialized
	 *   instances.  The finalize function should NOT destroy the
	 *   memory for the instance itself; that is done by the
	 *   CF runtime after this finalize callout returns.
	 * - equal field points to an equality-testing function; this
	 *   field may be NULL, in which case only pointer/reference
	 *   equality is performed on instances of this class. 
	 *   Pointer equality is tested, and the type IDs are checked
	 *   for equality, before this function is called (so, the
	 *   two instances are not pointer-equal but are of the same
	 *   class before this function is called).
	 * NOTE: the equal function must implement an immutable
	 *   equality relation, satisfying the reflexive, symmetric,
	 *    and transitive properties, and remains the same across
	 *   time and immutable operations (that is, if equal(A,B) at
	 *   some point, then later equal(A,B) provided neither
	 *   A or B has been mutated).
	 * - hash field points to a hash-code-computing function for
	 *   instances of this class; this field may be NULL in which
	 *   case the pointer value of an instance is converted into
	 *   a hash.
	 * NOTE: the hash function and equal function must satisfy
	 *   the relationship "equal(A,B) implies hash(A) == hash(B)";
	 *   that is, if two instances are equal, their hash codes must
	 *   be equal too. (However, the converse is not true!)
	 * - copyFormattingDesc field points to a function returning a
	 *   CFStringRef with a human-readable description of the
	 *   instance; if this is NULL, the type does not have special
	 *   human-readable string-formats.
	 * - copyDebugDesc field points to a function returning a
	 *   CFStringRef with a debugging description of the instance;
	 *   if this is NULL, a simple description is generated.
	 *
	 * This function returns _kCFRuntimeNotATypeID on failure, or
	 * on success, returns the CFTypeID for the new class.  This
	 * CFTypeID is what the class uses to allocate or initialize
	 * instances of the class. It is also returned from the
	 * conventional *GetTypeID() function, which returns the
	 * class's CFTypeID so that clients can compare the
	 * CFTypeID of instances with that of a class.
	 *
	 * The function to compute a human-readable string is very
	 * optional, and is really only interesting for classes,
	 * like strings or numbers, where it makes sense to format
	 * the instance using just its contents.
	 */

CF_EXPORT const CFRuntimeClass * _CFRuntimeGetClassWithTypeID(CFTypeID typeID);
	/* Returns the pointer to the CFRuntimeClass which was
	 * assigned the specified CFTypeID.
	 */

CF_EXPORT void _CFRuntimeUnregisterClassWithTypeID(CFTypeID typeID);
	/* Unregisters the class with the given type ID.  It is
	 * undefined whether type IDs are reused or not (expect
	 * that they will be).
	 *
	 * Whether or not unregistering the class is a good idea or
	 * not is not CF's responsibility.  In particular you must
	 * be quite sure all instances are gone, and there are no
	 * valid weak refs to such in other threads.
	 */

/* All CF "instances" start with this structure.  Never refer to
 * these fields directly -- they are for CF's use and may be added
 * to or removed or change format without warning.  Binary
 * compatibility for uses of this struct is not guaranteed from
 * release to release.
 */
#if DEPLOYMENT_RUNTIME_SWIFT

typedef struct __attribute__((__aligned__(8))) __CFRuntimeBase {
    // This matches the isa and retain count storage in Swift
    __ptrauth_cf_objc_isa_pointer uintptr_t _cfisa;
    uintptr_t _swift_rc;
    // This is for CF's use, and must match __NSCFType/_CFInfo layout
    _Atomic(uint64_t) _cfinfoa;
} CFRuntimeBase;

#define INIT_CFRUNTIME_BASE(...) {0, _CF_CONSTANT_OBJECT_STRONG_RC, 0x0000000000000080ULL}

#else
typedef struct __CFRuntimeBase {
    __ptrauth_cf_objc_isa_pointer uintptr_t _cfisa;
#if defined(__LP64__) || defined(__LLP64__)
    _Atomic(uint64_t) _cfinfoa;
#else
    _Atomic(uint32_t) _cfinfoa;
#endif
} CFRuntimeBase;

#if TARGET_RT_64_BIT
#define INIT_CFRUNTIME_BASE(...) {0, 0x0000000000000080ULL}
#else
#define INIT_CFRUNTIME_BASE(...) {0, 0x00000080UL}
#endif

#endif

CF_EXPORT CFTypeRef _CFRuntimeCreateInstance(CFAllocatorRef allocator, CFTypeID typeID, CFIndex extraBytes, unsigned char *category);
	/* Creates a new CF instance of the class specified by the
	 * given CFTypeID, using the given allocator, and returns it. 
	 * If the allocator returns NULL, this function returns NULL.
	 * A CFRuntimeBase structure is initialized at the beginning
	 * of the returned instance.  extraBytes is the additional
	 * number of bytes to allocate for the instance (BEYOND that
	 * needed for the CFRuntimeBase).  If the specified CFTypeID
	 * is unknown to the CF runtime, this function returns NULL.
	 * The base header is initialized and the extra bytes if
	 * requested will be zeroed.
	 * All instances created with this function must be destroyed
	 * only through use of the CFRelease() function -- instances
	 * must not be destroyed by using CFAllocatorDeallocate()
	 * directly, even in the initialization or creation functions
	 * of a class.  Pass NULL for the category parameter.
	 */

CF_EXPORT void _CFRuntimeSetInstanceTypeID(CFTypeRef cf, CFTypeID typeID);
	/* This function changes the typeID of the given instance.
	 * If the specified CFTypeID is unknown to the CF runtime,
	 * this function does nothing.  This function CANNOT be used
	 * to initialize an instance.  It is for advanced usages such
	 * as faulting. You cannot change the CFTypeID of an object
	 * of a _kCFRuntimeCustomRefCount class, or to a 
         * _kCFRuntimeCustomRefCount class.
	 */

#if DEPLOYMENT_RUNTIME_SWIFT
#else
CF_EXPORT void _CFRuntimeInitStaticInstance(void *memory, CFTypeID typeID);
	/* This function initializes a memory block to be a constant
	 * (unreleaseable) CF object of the given typeID.
	 * If the specified CFTypeID is unknown to the CF runtime,
	 * this function does nothing.  The memory block should
	 * be a chunk of in-binary writeable static memory, and at
	 * least as large as sizeof(CFRuntimeBase) on the platform
	 * the code is being compiled for.  The init function of the
	 * CFRuntimeClass is invoked on the memory as well, if the
	 * class has one. Static instances cannot be initialized to
	 * _kCFRuntimeCustomRefCount classes.
	 */
#define CF_HAS_INIT_STATIC_INSTANCE 1
#endif

CF_EXTERN_C_END

#endif /* ! __COREFOUNDATION_CFRUNTIME__ */

