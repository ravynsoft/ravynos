/*
 * Copyright (c) 2009 Apple Inc.  All Rights Reserved.
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


#ifndef _OBJC_INTERNAL_H
#define _OBJC_INTERNAL_H

/* 
 * WARNING  DANGER  HAZARD  BEWARE  EEK
 * 
 * Everything in this file is for Apple Internal use only.
 * These will change in arbitrary OS updates and in unpredictable ways.
 * When your program breaks, you get to keep both pieces.
 */

/*
 * objc-internal.h: Private SPI for use by other system frameworks.
 */

#include <objc/objc.h>
#include <objc/runtime.h>
#include <Availability.h>
#include <malloc/malloc.h>
#include <mach-o/loader.h>
#include <dispatch/dispatch.h>


// Termination reasons in the OS_REASON_OBJC namespace.
#define OBJC_EXIT_REASON_UNSPECIFIED 1
#define OBJC_EXIT_REASON_GC_NOT_SUPPORTED 2

// This is the allocation size required for each of the class and the metaclass 
// with objc_initializeClassPair() and objc_readClassPair().
// The runtime's class structure will never grow beyond this.
#define OBJC_MAX_CLASS_SIZE (32*sizeof(void*))


__BEGIN_DECLS

// This symbol is exported only from debug builds of libobjc itself.
#if defined(OBJC_IS_DEBUG_BUILD)
OBJC_EXPORT void _objc_isDebugBuild(void);
#endif

// In-place construction of an Objective-C class.
// cls and metacls must each be OBJC_MAX_CLASS_SIZE bytes.
// Returns nil if a class with the same name already exists.
// Returns nil if the superclass is under construction.
// Call objc_registerClassPair() when you are done.
OBJC_EXPORT Class _Nullable
objc_initializeClassPair(Class _Nullable superclass, const char * _Nonnull name,
                         Class _Nonnull cls, Class _Nonnull metacls) 
    OBJC_AVAILABLE(10.6, 3.0, 9.0, 1.0, 2.0);

// Class and metaclass construction from a compiler-generated memory image.
// cls and cls->isa must each be OBJC_MAX_CLASS_SIZE bytes. 
// Extra bytes not used the the metadata must be zero.
// info is the same objc_image_info that would be emitted by a static compiler.
// Returns nil if a class with the same name already exists.
// Returns nil if the superclass is nil and the class is not marked as a root.
// Returns nil if the superclass is under construction.
// Do not call objc_registerClassPair().
#if __OBJC2__
struct objc_image_info;
OBJC_EXPORT Class _Nullable
objc_readClassPair(Class _Nonnull cls,
                   const struct objc_image_info * _Nonnull info)
    OBJC_AVAILABLE(10.10, 8.0, 9.0, 1.0, 2.0);
#endif

// Batch object allocation using malloc_zone_batch_malloc().
OBJC_EXPORT unsigned
class_createInstances(Class _Nullable cls, size_t extraBytes, 
                      id _Nonnull * _Nonnull results, unsigned num_requested)
    OBJC_AVAILABLE(10.7, 4.3, 9.0, 1.0, 2.0)
    OBJC_ARC_UNAVAILABLE;

// Get the isa pointer written into objects just before being freed.
OBJC_EXPORT Class _Nonnull
_objc_getFreedObjectClass(void)
    OBJC_AVAILABLE(10.0, 2.0, 9.0, 1.0, 2.0);

// env NSObjCMessageLoggingEnabled
OBJC_EXPORT void
instrumentObjcMessageSends(BOOL flag)
    OBJC_AVAILABLE(10.0, 2.0, 9.0, 1.0, 2.0);

// Initializer called by libSystem
OBJC_EXPORT void
_objc_init(void)
#if __OBJC2__
    OBJC_AVAILABLE(10.8, 6.0, 9.0, 1.0, 2.0);
#else
    OBJC_AVAILABLE(10.12, 10.0, 10.0, 3.0, 2.0);
#endif

// fork() safety called by libSystem
OBJC_EXPORT void
_objc_atfork_prepare(void)
    OBJC_AVAILABLE(10.12, 10.0, 10.0, 3.0, 2.0);

OBJC_EXPORT void
_objc_atfork_parent(void)
    OBJC_AVAILABLE(10.12, 10.0, 10.0, 3.0, 2.0);

OBJC_EXPORT void
_objc_atfork_child(void)
    OBJC_AVAILABLE(10.12, 10.0, 10.0, 3.0, 2.0);

// Return YES if GC is on and `object` is a GC allocation.
OBJC_EXPORT BOOL
objc_isAuto(id _Nullable object) 
    OBJC_OSX_DEPRECATED_OTHERS_UNAVAILABLE(10.4, 10.8, "it always returns NO");

// GC debugging
OBJC_EXPORT BOOL
objc_dumpHeap(char * _Nonnull filename, unsigned long length)
    OBJC_OSX_DEPRECATED_OTHERS_UNAVAILABLE(10.4, 10.8, "it always returns NO");

// GC startup callback from Foundation
OBJC_EXPORT malloc_zone_t * _Nullable
objc_collect_init(int (* _Nonnull callback)(void))
    OBJC_OSX_DEPRECATED_OTHERS_UNAVAILABLE(10.4, 10.8, "it does nothing");

#if __OBJC2__
// Copies the list of currently realized classes
// intended for introspection only
// most users will want objc_copyClassList instead.
OBJC_EXPORT
Class _Nonnull * _Nullable
objc_copyRealizedClassList(unsigned int *_Nullable outCount)
	OBJC_AVAILABLE(10.15, 13.0, 13.0, 6.0, 5.0);

typedef struct objc_imp_cache_entry {
    SEL _Nonnull sel;
    IMP _Nonnull imp;
} objc_imp_cache_entry;

OBJC_EXPORT
objc_imp_cache_entry *_Nullable
class_copyImpCache(Class _Nonnull cls, int * _Nullable outCount)
	OBJC_AVAILABLE(10.15, 13.0, 13.0, 6.0, 5.0);
#endif

// Plainly-implemented GC barriers. Rosetta used to use these.
OBJC_EXPORT id _Nullable
objc_assign_strongCast_generic(id _Nullable value, id _Nullable * _Nonnull dest)
    UNAVAILABLE_ATTRIBUTE;

OBJC_EXPORT id _Nullable
objc_assign_global_generic(id _Nullable value, id _Nullable * _Nonnull dest)
    UNAVAILABLE_ATTRIBUTE;

OBJC_EXPORT id _Nullable
objc_assign_threadlocal_generic(id _Nullable value,
                                id _Nullable * _Nonnull dest)
    UNAVAILABLE_ATTRIBUTE;

OBJC_EXPORT id _Nullable
objc_assign_ivar_generic(id _Nullable value, id _Nonnull dest, ptrdiff_t offset)
    UNAVAILABLE_ATTRIBUTE;

// GC preflight for an app executable.
// 1: some slice requires GC
// 0: no slice requires GC
// -1: I/O or file format error
OBJC_EXPORT int
objc_appRequiresGC(int fd)
    __OSX_AVAILABLE(10.11) 
    __IOS_UNAVAILABLE __TVOS_UNAVAILABLE
    __WATCHOS_UNAVAILABLE
#ifndef __APPLE_BLEACH_SDK__
    __BRIDGEOS_UNAVAILABLE
#endif
;

// Install missing-class callback. Used by the late unlamented ZeroLink.
OBJC_EXPORT void
_objc_setClassLoader(BOOL (* _Nonnull newClassLoader)(const char * _Nonnull))
    OBJC2_UNAVAILABLE;

#if !(TARGET_OS_OSX && !TARGET_OS_IOSMAC && __i386__)
// Add a class copy fixup handler. The name is a misnomer, as
// multiple calls will install multiple handlers. Older versions
// of the Swift runtime call it by name, and it's only used by Swift
// so it's not worth deprecating this name in favor of a better one.
OBJC_EXPORT void
_objc_setClassCopyFixupHandler(void (* _Nonnull newFixupHandler)
    (Class _Nonnull oldClass, Class _Nonnull newClass))
    OBJC_AVAILABLE(10.14, 12.0, 12.0, 5.0, 3.0);
#endif

// Install handler for allocation failures. 
// Handler may abort, or throw, or provide an object to return.
OBJC_EXPORT void
_objc_setBadAllocHandler(id _Nullable (* _Nonnull newHandler)
                           (Class _Nullable isa))
    OBJC_AVAILABLE(10.8, 6.0, 9.0, 1.0, 2.0);

// Used by ExceptionHandling.framework
#if !__OBJC2__
OBJC_EXPORT void
_objc_error(id _Nullable rcv, const char * _Nonnull fmt, va_list args)
    __attribute__((noreturn, cold))
    OBJC_OSX_DEPRECATED_OTHERS_UNAVAILABLE(10.0, 10.5, "use other logging facilities instead");

#endif


/**
 * Returns the names of all the classes within a library.
 *
 * @param image The mach header for library or framework you are inquiring about.
 * @param outCount The number of class names returned.
 *
 * @return An array of C strings representing the class names.
 */
OBJC_EXPORT const char * _Nonnull * _Nullable
objc_copyClassNamesForImageHeader(const struct mach_header * _Nonnull mh,
                                  unsigned int * _Nullable outCount)
    OBJC_AVAILABLE(10.14, 12.0, 12.0, 5.0, 3.0);

// Tagged pointer objects.

#if __LP64__
#define OBJC_HAVE_TAGGED_POINTERS 1
#endif

#if OBJC_HAVE_TAGGED_POINTERS

// Tagged pointer layout and usage is subject to change on different OS versions.

// Tag indexes 0..<7 have a 60-bit payload.
// Tag index 7 is reserved.
// Tag indexes 8..<264 have a 52-bit payload.
// Tag index 264 is reserved.

#if __has_feature(objc_fixed_enum)  ||  __cplusplus >= 201103L
enum objc_tag_index_t : uint16_t
#else
typedef uint16_t objc_tag_index_t;
enum
#endif
{
    // 60-bit payloads
    OBJC_TAG_NSAtom            = 0, 
    OBJC_TAG_1                 = 1, 
    OBJC_TAG_NSString          = 2, 
    OBJC_TAG_NSNumber          = 3, 
    OBJC_TAG_NSIndexPath       = 4, 
    OBJC_TAG_NSManagedObjectID = 5, 
    OBJC_TAG_NSDate            = 6,

    // 60-bit reserved
    OBJC_TAG_RESERVED_7        = 7, 

    // 52-bit payloads
    OBJC_TAG_Photos_1          = 8,
    OBJC_TAG_Photos_2          = 9,
    OBJC_TAG_Photos_3          = 10,
    OBJC_TAG_Photos_4          = 11,
    OBJC_TAG_XPC_1             = 12,
    OBJC_TAG_XPC_2             = 13,
    OBJC_TAG_XPC_3             = 14,
    OBJC_TAG_XPC_4             = 15,
    OBJC_TAG_NSColor           = 16,
    OBJC_TAG_UIColor           = 17,
    OBJC_TAG_CGColor           = 18,
    OBJC_TAG_NSIndexSet        = 19,
    OBJC_TAG_NSMethodSignature = 20,
    OBJC_TAG_UTTypeRecord      = 21,

    OBJC_TAG_First60BitPayload = 0, 
    OBJC_TAG_Last60BitPayload  = 6, 
    OBJC_TAG_First52BitPayload = 8, 
    OBJC_TAG_Last52BitPayload  = 263, 

    OBJC_TAG_RESERVED_264      = 264
};
#if __has_feature(objc_fixed_enum)  &&  !defined(__cplusplus)
typedef enum objc_tag_index_t objc_tag_index_t;
#endif


// Returns true if tagged pointers are enabled.
// The other functions below must not be called if tagged pointers are disabled.
static inline bool 
_objc_taggedPointersEnabled(void);

// Register a class for a tagged pointer tag.
// Aborts if the tag is invalid or already in use.
OBJC_EXPORT void
_objc_registerTaggedPointerClass(objc_tag_index_t tag, Class _Nonnull cls)
    OBJC_AVAILABLE(10.9, 7.0, 9.0, 1.0, 2.0);

// Returns the registered class for the given tag.
// Returns nil if the tag is valid but has no registered class.
// Aborts if the tag is invalid.
OBJC_EXPORT Class _Nullable
_objc_getClassForTag(objc_tag_index_t tag)
    OBJC_AVAILABLE(10.9, 7.0, 9.0, 1.0, 2.0);

// Create a tagged pointer object with the given tag and payload.
// Assumes the tag is valid.
// Assumes tagged pointers are enabled.
// The payload will be silently truncated to fit.
static inline void * _Nonnull
_objc_makeTaggedPointer(objc_tag_index_t tag, uintptr_t payload);

// Return true if ptr is a tagged pointer object.
// Does not check the validity of ptr's class.
static inline bool 
_objc_isTaggedPointer(const void * _Nullable ptr);

// Extract the tag value from the given tagged pointer object.
// Assumes ptr is a valid tagged pointer object.
// Does not check the validity of ptr's tag.
static inline objc_tag_index_t 
_objc_getTaggedPointerTag(const void * _Nullable ptr);

// Extract the payload from the given tagged pointer object.
// Assumes ptr is a valid tagged pointer object.
// The payload value is zero-extended.
static inline uintptr_t
_objc_getTaggedPointerValue(const void * _Nullable ptr);

// Extract the payload from the given tagged pointer object.
// Assumes ptr is a valid tagged pointer object.
// The payload value is sign-extended.
static inline intptr_t
_objc_getTaggedPointerSignedValue(const void * _Nullable ptr);

// Don't use the values below. Use the declarations above.

#if (TARGET_OS_OSX || TARGET_OS_IOSMAC) && __x86_64__
    // 64-bit Mac - tag bit is LSB
#   define OBJC_MSB_TAGGED_POINTERS 0
#else
    // Everything else - tag bit is MSB
#   define OBJC_MSB_TAGGED_POINTERS 1
#endif

#define _OBJC_TAG_INDEX_MASK 0x7
// array slot includes the tag bit itself
#define _OBJC_TAG_SLOT_COUNT 16
#define _OBJC_TAG_SLOT_MASK 0xf

#define _OBJC_TAG_EXT_INDEX_MASK 0xff
// array slot has no extra bits
#define _OBJC_TAG_EXT_SLOT_COUNT 256
#define _OBJC_TAG_EXT_SLOT_MASK 0xff

#if OBJC_MSB_TAGGED_POINTERS
#   define _OBJC_TAG_MASK (1UL<<63)
#   define _OBJC_TAG_INDEX_SHIFT 60
#   define _OBJC_TAG_SLOT_SHIFT 60
#   define _OBJC_TAG_PAYLOAD_LSHIFT 4
#   define _OBJC_TAG_PAYLOAD_RSHIFT 4
#   define _OBJC_TAG_EXT_MASK (0xfUL<<60)
#   define _OBJC_TAG_EXT_INDEX_SHIFT 52
#   define _OBJC_TAG_EXT_SLOT_SHIFT 52
#   define _OBJC_TAG_EXT_PAYLOAD_LSHIFT 12
#   define _OBJC_TAG_EXT_PAYLOAD_RSHIFT 12
#else
#   define _OBJC_TAG_MASK 1UL
#   define _OBJC_TAG_INDEX_SHIFT 1
#   define _OBJC_TAG_SLOT_SHIFT 0
#   define _OBJC_TAG_PAYLOAD_LSHIFT 0
#   define _OBJC_TAG_PAYLOAD_RSHIFT 4
#   define _OBJC_TAG_EXT_MASK 0xfUL
#   define _OBJC_TAG_EXT_INDEX_SHIFT 4
#   define _OBJC_TAG_EXT_SLOT_SHIFT 4
#   define _OBJC_TAG_EXT_PAYLOAD_LSHIFT 0
#   define _OBJC_TAG_EXT_PAYLOAD_RSHIFT 12
#endif

extern uintptr_t objc_debug_taggedpointer_obfuscator;

static inline void * _Nonnull
_objc_encodeTaggedPointer(uintptr_t ptr)
{
    return (void *)(objc_debug_taggedpointer_obfuscator ^ ptr);
}

static inline uintptr_t
_objc_decodeTaggedPointer(const void * _Nullable ptr)
{
    return (uintptr_t)ptr ^ objc_debug_taggedpointer_obfuscator;
}

static inline bool 
_objc_taggedPointersEnabled(void)
{
    extern uintptr_t objc_debug_taggedpointer_mask;
    return (objc_debug_taggedpointer_mask != 0);
}

static inline void * _Nonnull
_objc_makeTaggedPointer(objc_tag_index_t tag, uintptr_t value)
{
    // PAYLOAD_LSHIFT and PAYLOAD_RSHIFT are the payload extraction shifts.
    // They are reversed here for payload insertion.

    // ASSERT(_objc_taggedPointersEnabled());
    if (tag <= OBJC_TAG_Last60BitPayload) {
        // ASSERT(((value << _OBJC_TAG_PAYLOAD_RSHIFT) >> _OBJC_TAG_PAYLOAD_LSHIFT) == value);
        uintptr_t result =
            (_OBJC_TAG_MASK | 
             ((uintptr_t)tag << _OBJC_TAG_INDEX_SHIFT) | 
             ((value << _OBJC_TAG_PAYLOAD_RSHIFT) >> _OBJC_TAG_PAYLOAD_LSHIFT));
        return _objc_encodeTaggedPointer(result);
    } else {
        // ASSERT(tag >= OBJC_TAG_First52BitPayload);
        // ASSERT(tag <= OBJC_TAG_Last52BitPayload);
        // ASSERT(((value << _OBJC_TAG_EXT_PAYLOAD_RSHIFT) >> _OBJC_TAG_EXT_PAYLOAD_LSHIFT) == value);
        uintptr_t result =
            (_OBJC_TAG_EXT_MASK |
             ((uintptr_t)(tag - OBJC_TAG_First52BitPayload) << _OBJC_TAG_EXT_INDEX_SHIFT) |
             ((value << _OBJC_TAG_EXT_PAYLOAD_RSHIFT) >> _OBJC_TAG_EXT_PAYLOAD_LSHIFT));
        return _objc_encodeTaggedPointer(result);
    }
}

static inline bool 
_objc_isTaggedPointer(const void * _Nullable ptr)
{
    return ((uintptr_t)ptr & _OBJC_TAG_MASK) == _OBJC_TAG_MASK;
}

static inline objc_tag_index_t 
_objc_getTaggedPointerTag(const void * _Nullable ptr) 
{
    // ASSERT(_objc_isTaggedPointer(ptr));
    uintptr_t value = _objc_decodeTaggedPointer(ptr);
    uintptr_t basicTag = (value >> _OBJC_TAG_INDEX_SHIFT) & _OBJC_TAG_INDEX_MASK;
    uintptr_t extTag =   (value >> _OBJC_TAG_EXT_INDEX_SHIFT) & _OBJC_TAG_EXT_INDEX_MASK;
    if (basicTag == _OBJC_TAG_INDEX_MASK) {
        return (objc_tag_index_t)(extTag + OBJC_TAG_First52BitPayload);
    } else {
        return (objc_tag_index_t)basicTag;
    }
}

static inline uintptr_t
_objc_getTaggedPointerValue(const void * _Nullable ptr) 
{
    // ASSERT(_objc_isTaggedPointer(ptr));
    uintptr_t value = _objc_decodeTaggedPointer(ptr);
    uintptr_t basicTag = (value >> _OBJC_TAG_INDEX_SHIFT) & _OBJC_TAG_INDEX_MASK;
    if (basicTag == _OBJC_TAG_INDEX_MASK) {
        return (value << _OBJC_TAG_EXT_PAYLOAD_LSHIFT) >> _OBJC_TAG_EXT_PAYLOAD_RSHIFT;
    } else {
        return (value << _OBJC_TAG_PAYLOAD_LSHIFT) >> _OBJC_TAG_PAYLOAD_RSHIFT;
    }
}

static inline intptr_t
_objc_getTaggedPointerSignedValue(const void * _Nullable ptr) 
{
    // ASSERT(_objc_isTaggedPointer(ptr));
    uintptr_t value = _objc_decodeTaggedPointer(ptr);
    uintptr_t basicTag = (value >> _OBJC_TAG_INDEX_SHIFT) & _OBJC_TAG_INDEX_MASK;
    if (basicTag == _OBJC_TAG_INDEX_MASK) {
        return ((intptr_t)value << _OBJC_TAG_EXT_PAYLOAD_LSHIFT) >> _OBJC_TAG_EXT_PAYLOAD_RSHIFT;
    } else {
        return ((intptr_t)value << _OBJC_TAG_PAYLOAD_LSHIFT) >> _OBJC_TAG_PAYLOAD_RSHIFT;
    }
}

// OBJC_HAVE_TAGGED_POINTERS
#endif


/**
 * Returns the method implementation of an object.
 *
 * @param obj An Objective-C object.
 * @param name An Objective-C selector.
 *
 * @return The IMP corresponding to the instance method implemented by
 * the class of \e obj.
 * 
 * @note Equivalent to:
 *
 * class_getMethodImplementation(object_getClass(obj), name);
 */
OBJC_EXPORT IMP _Nonnull
object_getMethodImplementation(id _Nullable obj, SEL _Nonnull name)
    OBJC_AVAILABLE(10.9, 7.0, 9.0, 1.0, 2.0);

OBJC_EXPORT IMP _Nonnull
object_getMethodImplementation_stret(id _Nullable obj, SEL _Nonnull name)
    OBJC_AVAILABLE(10.9, 7.0, 9.0, 1.0, 2.0)
    OBJC_ARM64_UNAVAILABLE;


/**
 * Adds multiple methods to a class in bulk. This amortizes overhead that can be
 * expensive when adding methods one by one with class_addMethod.
 *
 * @param cls The class to which to add the methods.
 * @param names An array of selectors for the methods to add.
 * @param imps An array of functions which implement the new methods.
 * @param types An array of strings that describe the types of each method's
 *              arguments.
 * @param count The number of items in the names, imps, and types arrays.
 * @param outFiledCount Upon return, contains the number of failed selectors in
 *                      the returned array.
 *
 * @return A NULL-terminated C array of selectors which could not be added. A
 * method cannot be added when a method of that name already exists on that
 * class. When no failures occur, the return value is \c NULL. When a non-NULL
 * value is returned, the caller must free the array with \c free().
 *
 */
#if __OBJC2__
OBJC_EXPORT _Nullable SEL * _Nullable
class_addMethodsBulk(_Nullable Class cls, _Nonnull const SEL * _Nonnull names,
                     _Nonnull const IMP * _Nonnull imps,
                     const char * _Nonnull * _Nonnull types, uint32_t count,
                     uint32_t * _Nullable outFailedCount)
        OBJC_AVAILABLE(10.14, 12.0, 12.0, 5.0, 3.0);
#endif

/**
 * Replaces multiple methods in a class in bulk. This amortizes overhead that
 * can be expensive when adding methods one by one with class_replaceMethod.
 *
 * @param cls The class to modify.
 * @param names An array of selectors for the methods to replace.
 * @param imps An array of functions will be the new method implementantations.
 * @param types An array of strings that describe the types of each method's
 *              arguments.
 * @param count The number of items in the names, imps, and types arrays.
 */
#if __OBJC2__
OBJC_EXPORT void
class_replaceMethodsBulk(_Nullable Class cls,
                         _Nonnull const SEL * _Nonnull names,
                         _Nonnull const IMP * _Nonnull imps,
                         const char * _Nonnull * _Nonnull types,
                         uint32_t count)
        OBJC_AVAILABLE(10.14, 12.0, 12.0, 5.0, 3.0);
#endif


// Instance-specific instance variable layout. This is no longer implemented.

OBJC_EXPORT void
_class_setIvarLayoutAccessor(Class _Nullable cls,
                             const uint8_t* _Nullable (* _Nonnull accessor)
                               (id _Nullable object))
    UNAVAILABLE_ATTRIBUTE;

OBJC_EXPORT const uint8_t * _Nullable
_object_getIvarLayout(Class _Nullable cls, id _Nullable object)
    UNAVAILABLE_ATTRIBUTE;


/*
  "Unknown" includes non-object ivars and non-ARC non-__weak ivars
  "Strong" includes ARC __strong ivars
  "Weak" includes ARC and new MRC __weak ivars
  "Unretained" includes ARC __unsafe_unretained and old GC+MRC __weak ivars
*/
typedef enum {
    objc_ivar_memoryUnknown,     // unknown / unknown
    objc_ivar_memoryStrong,      // direct access / objc_storeStrong
    objc_ivar_memoryWeak,        // objc_loadWeak[Retained] / objc_storeWeak
    objc_ivar_memoryUnretained   // direct access / direct access
} objc_ivar_memory_management_t;

OBJC_EXPORT objc_ivar_memory_management_t
_class_getIvarMemoryManagement(Class _Nullable cls, Ivar _Nonnull ivar)
    OBJC_AVAILABLE(10.12, 10.0, 10.0, 3.0, 2.0);

OBJC_EXPORT BOOL _class_isFutureClass(Class _Nullable cls)
    OBJC_AVAILABLE(10.9, 7.0, 9.0, 1.0, 2.0);


// API to only be called by root classes like NSObject or NSProxy

OBJC_EXPORT
id _Nonnull
_objc_rootRetain(id _Nonnull obj)
    OBJC_AVAILABLE(10.7, 5.0, 9.0, 1.0, 2.0);

OBJC_EXPORT
void
_objc_rootRelease(id _Nonnull obj)
    OBJC_AVAILABLE(10.7, 5.0, 9.0, 1.0, 2.0);

OBJC_EXPORT
bool
_objc_rootReleaseWasZero(id _Nonnull obj)
    OBJC_AVAILABLE(10.7, 5.0, 9.0, 1.0, 2.0);

OBJC_EXPORT
bool
_objc_rootTryRetain(id _Nonnull obj)
    OBJC_AVAILABLE(10.7, 5.0, 9.0, 1.0, 2.0);

OBJC_EXPORT
bool
_objc_rootIsDeallocating(id _Nonnull obj)
    OBJC_AVAILABLE(10.7, 5.0, 9.0, 1.0, 2.0);

OBJC_EXPORT
id _Nonnull
_objc_rootAutorelease(id _Nonnull obj)
    OBJC_AVAILABLE(10.7, 5.0, 9.0, 1.0, 2.0);

OBJC_EXPORT
uintptr_t
_objc_rootRetainCount(id _Nonnull obj)
    OBJC_AVAILABLE(10.7, 5.0, 9.0, 1.0, 2.0);

OBJC_EXPORT
id _Nonnull
_objc_rootInit(id _Nonnull obj)
    OBJC_AVAILABLE(10.7, 5.0, 9.0, 1.0, 2.0);

OBJC_EXPORT
id _Nullable
_objc_rootAllocWithZone(Class _Nonnull cls, malloc_zone_t * _Nullable zone)
    OBJC_AVAILABLE(10.7, 5.0, 9.0, 1.0, 2.0);

OBJC_EXPORT
id _Nullable
_objc_rootAlloc(Class _Nonnull cls)
    OBJC_AVAILABLE(10.7, 5.0, 9.0, 1.0, 2.0);

OBJC_EXPORT
void
_objc_rootDealloc(id _Nonnull obj)
    OBJC_AVAILABLE(10.7, 5.0, 9.0, 1.0, 2.0);

OBJC_EXPORT
void
_objc_rootFinalize(id _Nonnull obj)
    OBJC_AVAILABLE(10.7, 5.0, 9.0, 1.0, 2.0);

OBJC_EXPORT
malloc_zone_t * _Nonnull
_objc_rootZone(id _Nonnull obj)
    OBJC_AVAILABLE(10.7, 5.0, 9.0, 1.0, 2.0);

OBJC_EXPORT
uintptr_t
_objc_rootHash(id _Nonnull obj)
    OBJC_AVAILABLE(10.7, 5.0, 9.0, 1.0, 2.0);

OBJC_EXPORT
void * _Nonnull
objc_autoreleasePoolPush(void)
    OBJC_AVAILABLE(10.7, 5.0, 9.0, 1.0, 2.0);

OBJC_EXPORT
void
objc_autoreleasePoolPop(void * _Nonnull context)
    OBJC_AVAILABLE(10.7, 5.0, 9.0, 1.0, 2.0);


OBJC_EXPORT id _Nullable
objc_alloc(Class _Nullable cls)
    OBJC_AVAILABLE(10.9, 7.0, 9.0, 1.0, 2.0);

OBJC_EXPORT id _Nullable
objc_allocWithZone(Class _Nullable cls)
    OBJC_AVAILABLE(10.9, 7.0, 9.0, 1.0, 2.0);

OBJC_EXPORT id _Nullable
objc_alloc_init(Class _Nullable cls)
    OBJC_AVAILABLE(10.14.4, 12.2, 12.2, 5.2, 3.2);

OBJC_EXPORT id _Nullable
objc_opt_new(Class _Nullable cls)
	OBJC_AVAILABLE(10.15, 13.0, 13.0, 6.0, 5.0);

OBJC_EXPORT id _Nullable
objc_opt_self(id _Nullable obj)
	OBJC_AVAILABLE(10.15, 13.0, 13.0, 6.0, 5.0);

OBJC_EXPORT Class _Nullable
objc_opt_class(id _Nullable obj)
	OBJC_AVAILABLE(10.15, 13.0, 13.0, 6.0, 5.0);

OBJC_EXPORT BOOL
objc_opt_respondsToSelector(id _Nullable obj, SEL _Nullable sel)
	OBJC_AVAILABLE(10.15, 13.0, 13.0, 6.0, 5.0);

OBJC_EXPORT BOOL
objc_opt_isKindOfClass(id _Nullable obj, Class _Nullable cls)
	OBJC_AVAILABLE(10.15, 13.0, 13.0, 6.0, 5.0);


OBJC_EXPORT BOOL
objc_sync_try_enter(id _Nonnull obj)
    OBJC_AVAILABLE(10.15, 13.0, 13.0, 6.0, 5.0);

OBJC_EXPORT id _Nullable
objc_retain(id _Nullable obj)
    __asm__("_objc_retain")
    OBJC_AVAILABLE(10.7, 5.0, 9.0, 1.0, 2.0);

OBJC_EXPORT void
objc_release(id _Nullable obj)
    __asm__("_objc_release")
    OBJC_AVAILABLE(10.7, 5.0, 9.0, 1.0, 2.0);

OBJC_EXPORT id _Nullable
objc_autorelease(id _Nullable obj)
    __asm__("_objc_autorelease")
    OBJC_AVAILABLE(10.7, 5.0, 9.0, 1.0, 2.0);

// Prepare a value at +1 for return through a +0 autoreleasing convention.
OBJC_EXPORT id _Nullable
objc_autoreleaseReturnValue(id _Nullable obj)
    OBJC_AVAILABLE(10.7, 5.0, 9.0, 1.0, 2.0);

// Prepare a value at +0 for return through a +0 autoreleasing convention.
OBJC_EXPORT id _Nullable
objc_retainAutoreleaseReturnValue(id _Nullable obj)
    OBJC_AVAILABLE(10.7, 5.0, 9.0, 1.0, 2.0);

// Accept a value returned through a +0 autoreleasing convention for use at +1.
OBJC_EXPORT id _Nullable
objc_retainAutoreleasedReturnValue(id _Nullable obj)
    OBJC_AVAILABLE(10.7, 5.0, 9.0, 1.0, 2.0);

// Accept a value returned through a +0 autoreleasing convention for use at +0.
OBJC_EXPORT id _Nullable
objc_unsafeClaimAutoreleasedReturnValue(id _Nullable obj)
    OBJC_AVAILABLE(10.11, 9.0, 9.0, 1.0, 2.0);

OBJC_EXPORT void
objc_storeStrong(id _Nullable * _Nonnull location, id _Nullable obj)
    OBJC_AVAILABLE(10.7, 5.0, 9.0, 1.0, 2.0);

OBJC_EXPORT id _Nullable
objc_retainAutorelease(id _Nullable obj)
    OBJC_AVAILABLE(10.7, 5.0, 9.0, 1.0, 2.0);

// obsolete.
OBJC_EXPORT id _Nullable
objc_retain_autorelease(id _Nullable obj)
    OBJC_AVAILABLE(10.7, 5.0, 9.0, 1.0, 2.0);

OBJC_EXPORT id _Nullable
objc_loadWeakRetained(id _Nullable * _Nonnull location)
    OBJC_AVAILABLE(10.7, 5.0, 9.0, 1.0, 2.0);

OBJC_EXPORT id _Nullable 
objc_initWeak(id _Nullable * _Nonnull location, id _Nullable val)
    OBJC_AVAILABLE(10.7, 5.0, 9.0, 1.0, 2.0);

// Like objc_storeWeak, but stores nil if the new object is deallocating 
// or the new object's class does not support weak references.
// Returns the value stored (either the new object or nil).
OBJC_EXPORT id _Nullable
objc_storeWeakOrNil(id _Nullable * _Nonnull location, id _Nullable obj)
    OBJC_AVAILABLE(10.11, 9.0, 9.0, 1.0, 2.0);

// Like objc_initWeak, but stores nil if the new object is deallocating 
// or the new object's class does not support weak references.
// Returns the value stored (either the new object or nil).
OBJC_EXPORT id _Nullable
objc_initWeakOrNil(id _Nullable * _Nonnull location, id _Nullable val) 
    OBJC_AVAILABLE(10.11, 9.0, 9.0, 1.0, 2.0);

OBJC_EXPORT void
objc_destroyWeak(id _Nullable * _Nonnull location) 
    OBJC_AVAILABLE(10.7, 5.0, 9.0, 1.0, 2.0);

OBJC_EXPORT void 
objc_copyWeak(id _Nullable * _Nonnull to, id _Nullable * _Nonnull from)
    OBJC_AVAILABLE(10.7, 5.0, 9.0, 1.0, 2.0);

OBJC_EXPORT void 
objc_moveWeak(id _Nullable * _Nonnull to, id _Nullable * _Nonnull from) 
    OBJC_AVAILABLE(10.7, 5.0, 9.0, 1.0, 2.0);


OBJC_EXPORT void
_objc_autoreleasePoolPrint(void)
    OBJC_AVAILABLE(10.7, 5.0, 9.0, 1.0, 2.0);

OBJC_EXPORT BOOL
objc_should_deallocate(id _Nonnull object)
    OBJC_AVAILABLE(10.7, 5.0, 9.0, 1.0, 2.0);

OBJC_EXPORT void
objc_clear_deallocating(id _Nonnull object)
    OBJC_AVAILABLE(10.7, 5.0, 9.0, 1.0, 2.0);

 
// to make CF link for now

OBJC_EXPORT void * _Nonnull
_objc_autoreleasePoolPush(void)
    OBJC_AVAILABLE(10.7, 5.0, 9.0, 1.0, 2.0);

OBJC_EXPORT void
_objc_autoreleasePoolPop(void * _Nonnull context)
    OBJC_AVAILABLE(10.7, 5.0, 9.0, 1.0, 2.0);


/**
 * Load a classref, which is a chunk of data containing a class
 * pointer. May perform initialization and rewrite the classref to
 * point to a new object, if needed. Returns the loaded Class.
 *
 * In particular, if the classref points to a stub class (indicated
 * by setting the bottom bit of the class pointer to 1), then this
 * will call the stub's initializer and then replace the classref
 * value with the value returned by the initializer.
 *
 * @param clsref The classref to load.
 * @return The loaded Class pointer.
 */
#if __OBJC2__
OBJC_EXPORT _Nullable Class
objc_loadClassref(_Nullable Class * _Nonnull clsref)
    OBJC_AVAILABLE(10.15, 13.0, 13.0, 6.0, 5.0);
#endif


// Extra @encode data for XPC, or NULL
OBJC_EXPORT const char * _Nullable
_protocol_getMethodTypeEncoding(Protocol * _Nonnull proto, SEL _Nonnull sel,
                                BOOL isRequiredMethod, BOOL isInstanceMethod)
    OBJC_AVAILABLE(10.8, 6.0, 9.0, 1.0, 2.0);


/**
 * Function type for a function that is called when a realized class
 * is about to be initialized.
 *
 * @param context The context pointer the function was registered with.
 * @param cls The class that's about to be initialized.
 */
struct mach_header;
typedef void (*_objc_func_willInitializeClass)(void * _Nullable context, Class _Nonnull cls);

/**
 * Add a function to be called when a realized class is about to be
 * initialized. The class can be queried and manipulated using runtime
 * functions. Don't message it.
 *
 * When adding a new function, that function is immediately called with all
 * realized classes that are already initialized or are in the process
 * of initialization.
 *
 * @param func The function to add.
 * @param context A context pointer that will be passed to the function when called.
 */
#define OBJC_WILLINITIALIZECLASSFUNC_DEFINED 1
OBJC_EXPORT void _objc_addWillInitializeClassFunc(_objc_func_willInitializeClass _Nonnull func, void * _Nullable context)
    OBJC_AVAILABLE(10.15, 13.0, 13.0, 6.0, 4.0);

// API to only be called by classes that provide their own reference count storage

OBJC_EXPORT void
_objc_deallocOnMainThreadHelper(void * _Nullable context)
    OBJC_AVAILABLE(10.7, 5.0, 9.0, 1.0, 2.0);

// On async versus sync deallocation and the _dealloc2main flag
//
// Theory:
//
// If order matters, then code must always: [self dealloc].
// If order doesn't matter, then always async should be safe.
//
// Practice:
//
// The _dealloc2main bit is set for GUI objects that may be retained by other
// threads. Once deallocation begins on the main thread, doing more async
// deallocation will at best cause extra UI latency and at worst cause
// use-after-free bugs in unretained delegate style patterns. Yes, this is
// extremely fragile. Yes, in the long run, developers should switch to weak
// references.
//
// Note is NOT safe to do any equality check against the result of
// dispatch_get_current_queue(). The main thread can and does drain more than
// one dispatch queue. That is why we call pthread_main_np().
//

typedef enum {
    _OBJC_RESURRECT_OBJECT = -1,        /* _logicBlock has called -retain, and scheduled a -release for later. */
    _OBJC_DEALLOC_OBJECT_NOW = 1,       /* call [self dealloc] immediately. */
    _OBJC_DEALLOC_OBJECT_LATER = 2      /* call [self dealloc] on the main queue. */
} _objc_object_disposition_t;

#define _OBJC_SUPPORTED_INLINE_REFCNT_LOGIC_BLOCK(_rc_ivar, _logicBlock)        \
    -(id)retain {                                                               \
        /* this will fail to compile if _rc_ivar is an unsigned type */         \
        int _retain_count_ivar_must_not_be_unsigned[0L - (__typeof__(_rc_ivar))-1] __attribute__((unused)); \
        __typeof__(_rc_ivar) _prev = __sync_fetch_and_add(&_rc_ivar, 2);        \
        if (_prev < -2) { /* specifically allow resurrection from logical 0. */ \
            __builtin_trap(); /* BUG: retain of over-released ref */            \
        }                                                                       \
        return self;                                                            \
    }                                                                           \
    -(oneway void)release {                                                     \
        __typeof__(_rc_ivar) _prev = __sync_fetch_and_sub(&_rc_ivar, 2);        \
        if (_prev > 0) {                                                        \
            return;                                                             \
        } else if (_prev < 0) {                                                 \
            __builtin_trap(); /* BUG: over-release */                           \
        }                                                                       \
        _objc_object_disposition_t fate = _logicBlock(self);                    \
        if (fate == _OBJC_RESURRECT_OBJECT) {                                   \
            return;                                                             \
        }                                                                       \
        /* mark the object as deallocating. */                                  \
        if (!__sync_bool_compare_and_swap(&_rc_ivar, -2, 1)) {                  \
            __builtin_trap(); /* BUG: dangling ref did a retain */              \
        }                                                                       \
        if (fate == _OBJC_DEALLOC_OBJECT_NOW) {                                 \
            [self dealloc];                                                     \
        } else if (fate == _OBJC_DEALLOC_OBJECT_LATER) {                        \
            dispatch_barrier_async_f(dispatch_get_main_queue(), self,           \
                _objc_deallocOnMainThreadHelper);                               \
        } else {                                                                \
            __builtin_trap(); /* BUG: bogus fate value */                       \
        }                                                                       \
    }                                                                           \
    -(NSUInteger)retainCount {                                                  \
        return (NSUInteger)(_rc_ivar + 2) >> 1;                                 \
    }                                                                           \
    -(BOOL)_tryRetain {                                                         \
        __typeof__(_rc_ivar) _prev;                                             \
        do {                                                                    \
            _prev = _rc_ivar;                                                   \
            if (_prev & 1) {                                                    \
                return 0;                                                       \
            } else if (_prev == -2) {                                           \
                return 0;                                                       \
            } else if (_prev < -2) {                                            \
                __builtin_trap(); /* BUG: over-release elsewhere */             \
            }                                                                   \
        } while ( ! __sync_bool_compare_and_swap(&_rc_ivar, _prev, _prev + 2)); \
        return 1;                                                               \
    }                                                                           \
    -(BOOL)_isDeallocating {                                                    \
        if (_rc_ivar == -2) {                                                   \
            return 1;                                                           \
        } else if (_rc_ivar < -2) {                                             \
            __builtin_trap(); /* BUG: over-release elsewhere */                 \
        }                                                                       \
        return (_rc_ivar & 1) != 0;                                             \
    }

#define _OBJC_SUPPORTED_INLINE_REFCNT_LOGIC(_rc_ivar, _dealloc2main)            \
    _OBJC_SUPPORTED_INLINE_REFCNT_LOGIC_BLOCK(_rc_ivar, (^(id _self_ __attribute__((unused))) { \
        if ((_dealloc2main) && !pthread_main_np()) {                            \
            return _OBJC_DEALLOC_OBJECT_LATER;                                  \
        } else {                                                                \
            return _OBJC_DEALLOC_OBJECT_NOW;                                    \
        }                                                                       \
    }))

#define _OBJC_SUPPORTED_INLINE_REFCNT(_rc_ivar) _OBJC_SUPPORTED_INLINE_REFCNT_LOGIC(_rc_ivar, 0)
#define _OBJC_SUPPORTED_INLINE_REFCNT_WITH_DEALLOC2MAIN(_rc_ivar) _OBJC_SUPPORTED_INLINE_REFCNT_LOGIC(_rc_ivar, 1)

__END_DECLS

#endif
