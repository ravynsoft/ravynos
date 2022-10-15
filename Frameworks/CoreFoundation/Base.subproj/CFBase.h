/*	CFBase.h
	Copyright (c) 1998-2019, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
*/

#if !defined(__COREFOUNDATION_CFBASE__)
#define __COREFOUNDATION_CFBASE__ 1

#if __has_include(<CoreFoundation/TargetConditionals.h>)
#include <CoreFoundation/TargetConditionals.h>
#else
#include <TargetConditionals.h>
#endif

#include <CoreFoundation/CFAvailability.h>

#if (defined(__CYGWIN32__) || defined(_WIN32)) && !defined(__WIN32__)
#define __WIN32__ 1
#endif

#if defined(_WIN64) && !defined(__WIN64__)
#define __WIN64__ 1
#endif

#if defined(__WIN64__) && !defined(__LLP64__)
#define __LLP64__ 1
#endif

#if defined(_MSC_VER) && defined(_M_IX86)
#define __i386__ 1
#endif

#if (defined(__i386__) || defined(__x86_64__)) && !defined(__LITTLE_ENDIAN__)
#define __LITTLE_ENDIAN__ 1
#endif

#if !defined(__BIG_ENDIAN__) && !defined(__LITTLE_ENDIAN__)
#error Do not know the endianess of this architecture
#endif

#if !__BIG_ENDIAN__ && !__LITTLE_ENDIAN__
#error Both __BIG_ENDIAN__ and __LITTLE_ENDIAN__ cannot be false
#endif

#if __BIG_ENDIAN__ && __LITTLE_ENDIAN__
#error Both __BIG_ENDIAN__ and __LITTLE_ENDIAN__ cannot be true
#endif

// Some compilers provide the capability to test if certain features are available. This macro provides a compatibility path for other compilers.
#ifndef __has_feature
#define __has_feature(x) 0
#endif

// Some compilers provide the capability to test if certain attributes are available. This macro provides a compatibility path for other compilers.
#ifndef __has_attribute
#define __has_attribute(x) 0
#endif

#ifndef __has_extension
#define __has_extension(x) 0
#endif

#if defined(__GNUC__) || TARGET_OS_WIN32
#include <stdint.h>
#include <stdbool.h>
#endif

#if __BLOCKS__ && (TARGET_OS_OSX || TARGET_OS_IPHONE)
#include <Block.h>
#endif

  #if (TARGET_OS_OSX || TARGET_OS_IPHONE) && !DEPLOYMENT_RUNTIME_SWIFT
    #include <libkern/OSTypes.h>
  #endif

#if !defined(__MACTYPES__)
#if !defined(_OS_OSTYPES_H)
#if DEPLOYMENT_RUNTIME_SWIFT
    typedef _Bool                   Boolean;
#else
    typedef unsigned char           Boolean;
#endif
    typedef unsigned char           UInt8;
    typedef signed char             SInt8;
    typedef unsigned short          UInt16;
    typedef signed short            SInt16;
    typedef unsigned int            UInt32;
    typedef signed int              SInt32;
    typedef uint64_t		    UInt64;
    typedef int64_t		    SInt64;
    typedef SInt32                  OSStatus;
#endif
    typedef float                   Float32;
    typedef double                  Float64;
    typedef unsigned short          UniChar;
    typedef unsigned long           UniCharCount;
    typedef unsigned char *         StringPtr;
    typedef const unsigned char *   ConstStringPtr;
    typedef unsigned char           Str255[256];
    typedef const unsigned char *   ConstStr255Param;
    typedef SInt16                  OSErr;
    typedef SInt16                  RegionCode;
    typedef SInt16                  LangCode;
    typedef SInt16                  ScriptCode;
    typedef UInt32                  FourCharCode;
    typedef FourCharCode            OSType;
    typedef UInt8                   Byte;
    typedef SInt8                   SignedByte;
#endif
#if !defined(__MACTYPES__) || (defined(UNIVERSAL_INTERFACES_VERSION) && UNIVERSAL_INTERFACES_VERSION < 0x0340)
    typedef UInt32                  UTF32Char;
    typedef UInt16                  UTF16Char;
    typedef UInt8                   UTF8Char;
#endif

#if !defined(CF_EXTERN_C_BEGIN)
#if defined(__cplusplus)
#define CF_EXTERN_C_BEGIN extern "C" {
#define CF_EXTERN_C_END   }
#else
#define CF_EXTERN_C_BEGIN
#define CF_EXTERN_C_END
#endif
#endif

#if TARGET_OS_WIN32
    #if defined(__cplusplus)
        #define _CF_EXTERN extern "C"
    #else
        #define _CF_EXTERN extern
    #endif

    #if defined(_WINDLL)
        #if defined(CoreFoundation_EXPORTS) || defined(CF_BUILDING_CF)
            #define CF_EXPORT _CF_EXTERN __declspec(dllexport)
        #else
            #define CF_EXPORT _CF_EXTERN __declspec(dllimport)
        #endif
    #else
        #define CF_EXPORT _CF_EXTERN
    #endif
#else
#define CF_EXPORT extern
#endif

CF_EXTERN_C_BEGIN

#if !defined(NULL)
#if defined(__GNUG__)
    #define NULL __null
#elif defined(__cplusplus)
    #define NULL 0
#else
    #define NULL ((void *)0)
#endif
#endif

#if !defined(TRUE)
    #define TRUE	1
#endif

#if !defined(FALSE)
    #define FALSE	0
#endif

#if !defined(CF_INLINE)
    #if defined(__GNUC__) && (__GNUC__ == 4) && !defined(DEBUG)
        #define CF_INLINE static __inline__ __attribute__((always_inline))
    #elif defined(__GNUC__)
        #define CF_INLINE static __inline__
    #elif defined(__cplusplus)
	#define CF_INLINE static inline
    #elif defined(_MSC_VER)
        #define CF_INLINE static __inline
    #elif TARGET_OS_WIN32
	#define CF_INLINE static __inline__
    #endif
#endif

// Marks functions which return a CF type that needs to be released by the caller but whose names are not consistent with CoreFoundation naming rules. The recommended fix to this is to rename the functions, but this macro can be used to let the clang static analyzer know of any exceptions that cannot be fixed.
// This macro is ONLY to be used in exceptional circumstances, not to annotate functions which conform to the CoreFoundation naming rules.
#ifndef CF_RETURNS_RETAINED
#if __has_feature(attribute_cf_returns_retained)
#define CF_RETURNS_RETAINED __attribute__((cf_returns_retained))
#else
#define CF_RETURNS_RETAINED
#endif
#endif

// Marks functions which return a CF type that may need to be retained by the caller but whose names are not consistent with CoreFoundation naming rules. The recommended fix to this is to rename the functions, but this macro can be used to let the clang static analyzer know of any exceptions that cannot be fixed. 
// This macro is ONLY to be used in exceptional circumstances, not to annotate functions which conform to the CoreFoundation naming rules.
#ifndef CF_RETURNS_NOT_RETAINED
#if __has_feature(attribute_cf_returns_not_retained)
#define CF_RETURNS_NOT_RETAINED __attribute__((cf_returns_not_retained))
#else
#define CF_RETURNS_NOT_RETAINED
#endif
#endif

// Marks function arguments which are released by the callee.
#ifndef CF_RELEASES_ARGUMENT
#if __has_feature(attribute_cf_consumed)
#define CF_RELEASES_ARGUMENT __attribute__((cf_consumed))
#else
#define CF_RELEASES_ARGUMENT
#endif
#endif

// Compatibility
#ifndef CF_CONSUMED
#if __has_feature(attribute_cf_consumed)
#define CF_CONSUMED __attribute__((cf_consumed))
#else
#define CF_CONSUMED
#endif
#endif

// Marks functions which cannot be used when compiling in automatic reference counting mode.
#if __has_feature(objc_arc)
#define CF_AUTOMATED_REFCOUNT_UNAVAILABLE __attribute__((unavailable("not available in automatic reference counting mode")))
#else
#define CF_AUTOMATED_REFCOUNT_UNAVAILABLE
#endif

#ifndef CF_IMPLICIT_BRIDGING_ENABLED
#if __has_feature(arc_cf_code_audited)
#define CF_IMPLICIT_BRIDGING_ENABLED _Pragma("clang arc_cf_code_audited begin")
#else
#define CF_IMPLICIT_BRIDGING_ENABLED
#endif
#endif

#ifndef CF_IMPLICIT_BRIDGING_DISABLED
#if __has_feature(arc_cf_code_audited)
#define CF_IMPLICIT_BRIDGING_DISABLED _Pragma("clang arc_cf_code_audited end")
#else
#define CF_IMPLICIT_BRIDGING_DISABLED
#endif
#endif

#if __has_attribute(objc_bridge) && __has_feature(objc_bridge_id) && __has_feature(objc_bridge_id_on_typedefs)

#ifdef __OBJC__
@class NSArray;
@class NSAttributedString;
@class NSString;
@class NSNull;
@class NSCharacterSet;
@class NSData;
@class NSDate;
@class NSTimeZone;
@class NSDictionary;
@class NSError;
@class NSLocale;
@class NSNumber;
@class NSSet;
@class NSURL;
#endif

#define CF_BRIDGED_TYPE(T)		__attribute__((objc_bridge(T)))
#define CF_BRIDGED_MUTABLE_TYPE(T)	__attribute__((objc_bridge_mutable(T)))
#define CF_RELATED_TYPE(T,C,I)		__attribute__((objc_bridge_related(T,C,I)))
#else
#define CF_BRIDGED_TYPE(T)
#define CF_BRIDGED_MUTABLE_TYPE(T)
#define CF_RELATED_TYPE(T,C,I)
#endif


#if __has_feature(assume_nonnull)
#define CF_ASSUME_NONNULL_BEGIN _Pragma("clang assume_nonnull begin")
#define CF_ASSUME_NONNULL_END   _Pragma("clang assume_nonnull end")
#else
#define CF_ASSUME_NONNULL_BEGIN
#define CF_ASSUME_NONNULL_END
#endif


#if !__has_feature(nullability)
#ifndef _Nullable
#define _Nullable
#endif
#ifndef _Nonnull
#define _Nonnull
#endif
#ifndef _Null_unspecified
#define _Null_unspecified
#endif
#endif


#if __has_attribute(swift_private)
# define CF_REFINED_FOR_SWIFT __attribute__((swift_private))
#else
# define CF_REFINED_FOR_SWIFT
#endif


#if __has_attribute(swift_name)
# define CF_SWIFT_NAME(_name) __attribute__((swift_name(#_name)))
#else
# define CF_SWIFT_NAME(_name)
#endif

#if __has_attribute(noescape)
#define CF_NOESCAPE __attribute__((noescape))
#else
#define CF_NOESCAPE
#endif

#if __has_attribute(not_tail_called)
#define CF_NO_TAIL_CALL __attribute__((not_tail_called))
#else
#define CF_NO_TAIL_CALL
#endif

#if __has_attribute(warn_unused_result)
#define CF_WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#else
#define CF_WARN_UNUSED_RESULT
#endif

#if !__has_feature(objc_generics_variance)
#ifndef __covariant
#define __covariant
#endif
#ifndef __contravariant
#define __contravariant
#endif
#endif


CF_EXPORT double kCFCoreFoundationVersionNumber;

#if TARGET_OS_MAC
#define kCFCoreFoundationVersionNumber10_0	196.40
#define kCFCoreFoundationVersionNumber10_0_3	196.50
#define kCFCoreFoundationVersionNumber10_1	226.00
#define kCFCoreFoundationVersionNumber10_1_1	226.00
/* Note the next three do not follow the usual numbering policy from the base release */
#define kCFCoreFoundationVersionNumber10_1_2	227.20
#define kCFCoreFoundationVersionNumber10_1_3	227.20
#define kCFCoreFoundationVersionNumber10_1_4	227.30
#define kCFCoreFoundationVersionNumber10_2	263.00
#define kCFCoreFoundationVersionNumber10_2_1	263.10
#define kCFCoreFoundationVersionNumber10_2_2	263.10
#define kCFCoreFoundationVersionNumber10_2_3	263.30
#define kCFCoreFoundationVersionNumber10_2_4	263.30
#define kCFCoreFoundationVersionNumber10_2_5	263.50
#define kCFCoreFoundationVersionNumber10_2_6	263.50
#define kCFCoreFoundationVersionNumber10_2_7	263.50
#define kCFCoreFoundationVersionNumber10_2_8	263.50
#define kCFCoreFoundationVersionNumber10_3	299.00
#define kCFCoreFoundationVersionNumber10_3_1	299.00
#define kCFCoreFoundationVersionNumber10_3_2	299.00
#define kCFCoreFoundationVersionNumber10_3_3	299.30
#define kCFCoreFoundationVersionNumber10_3_4	299.31
#define kCFCoreFoundationVersionNumber10_3_5	299.31
#define kCFCoreFoundationVersionNumber10_3_6	299.32
#define kCFCoreFoundationVersionNumber10_3_7	299.33
#define kCFCoreFoundationVersionNumber10_3_8	299.33
#define kCFCoreFoundationVersionNumber10_3_9	299.35
#define kCFCoreFoundationVersionNumber10_4	368.00
#define kCFCoreFoundationVersionNumber10_4_1	368.10
#define kCFCoreFoundationVersionNumber10_4_2	368.11
#define kCFCoreFoundationVersionNumber10_4_3	368.18
#define kCFCoreFoundationVersionNumber10_4_4_Intel	368.26
#define kCFCoreFoundationVersionNumber10_4_4_PowerPC	368.25
#define kCFCoreFoundationVersionNumber10_4_5_Intel	368.26
#define kCFCoreFoundationVersionNumber10_4_5_PowerPC	368.25
#define kCFCoreFoundationVersionNumber10_4_6_Intel	368.26
#define kCFCoreFoundationVersionNumber10_4_6_PowerPC	368.25
#define kCFCoreFoundationVersionNumber10_4_7	368.27
#define kCFCoreFoundationVersionNumber10_4_8	368.27
#define kCFCoreFoundationVersionNumber10_4_9	368.28
#define kCFCoreFoundationVersionNumber10_4_10	368.28
#define kCFCoreFoundationVersionNumber10_4_11	368.31
#define kCFCoreFoundationVersionNumber10_5	476.00
#define kCFCoreFoundationVersionNumber10_5_1	476.00
#define kCFCoreFoundationVersionNumber10_5_2	476.10
#define kCFCoreFoundationVersionNumber10_5_3	476.13
#define kCFCoreFoundationVersionNumber10_5_4	476.14
#define kCFCoreFoundationVersionNumber10_5_5	476.15
#define kCFCoreFoundationVersionNumber10_5_6	476.17
#define kCFCoreFoundationVersionNumber10_5_7	476.18
#define kCFCoreFoundationVersionNumber10_5_8	476.19
#define kCFCoreFoundationVersionNumber10_6	550.00
#define kCFCoreFoundationVersionNumber10_6_1	550.00
#define kCFCoreFoundationVersionNumber10_6_2	550.13
#define kCFCoreFoundationVersionNumber10_6_3	550.19
#define kCFCoreFoundationVersionNumber10_6_4	550.29
#define kCFCoreFoundationVersionNumber10_6_5	550.42
#define kCFCoreFoundationVersionNumber10_6_6	550.42
#define kCFCoreFoundationVersionNumber10_6_7	550.42
#define kCFCoreFoundationVersionNumber10_6_8	550.43
#define kCFCoreFoundationVersionNumber10_7      635.00
#define kCFCoreFoundationVersionNumber10_7_1    635.00
#define kCFCoreFoundationVersionNumber10_7_2    635.15
#define kCFCoreFoundationVersionNumber10_7_3    635.19
#define kCFCoreFoundationVersionNumber10_7_4    635.21
#define kCFCoreFoundationVersionNumber10_7_5    635.21
#define kCFCoreFoundationVersionNumber10_8      744.00
#define kCFCoreFoundationVersionNumber10_8_1    744.00
#define kCFCoreFoundationVersionNumber10_8_2    744.12
#define kCFCoreFoundationVersionNumber10_8_3    744.18
#define kCFCoreFoundationVersionNumber10_8_4    744.19
#define kCFCoreFoundationVersionNumber10_9      855.11
#define kCFCoreFoundationVersionNumber10_9_1    855.11
#define kCFCoreFoundationVersionNumber10_9_2    855.14
#define kCFCoreFoundationVersionNumber10_10     1151.16
#define kCFCoreFoundationVersionNumber10_10_1   1151.16
#define kCFCoreFoundationVersionNumber10_10_2   1152
#define kCFCoreFoundationVersionNumber10_10_3   1153.18
#define kCFCoreFoundationVersionNumber10_10_4   1153.18
#define kCFCoreFoundationVersionNumber10_10_5   1153.18
#define kCFCoreFoundationVersionNumber10_10_Max 1199
#define kCFCoreFoundationVersionNumber10_11     1253
#define kCFCoreFoundationVersionNumber10_11_1   1255.1
#define kCFCoreFoundationVersionNumber10_11_2   1256.14
#define kCFCoreFoundationVersionNumber10_11_3   1256.14
#define kCFCoreFoundationVersionNumber10_11_4   1258.1
#define kCFCoreFoundationVersionNumber10_11_Max 1299
#endif

#if TARGET_OS_IPHONE
#define kCFCoreFoundationVersionNumber_iPhoneOS_2_0 478.23
#define kCFCoreFoundationVersionNumber_iPhoneOS_2_1 478.26
#define kCFCoreFoundationVersionNumber_iPhoneOS_2_2 478.29
#define kCFCoreFoundationVersionNumber_iPhoneOS_3_0 478.47
#define kCFCoreFoundationVersionNumber_iPhoneOS_3_1 478.52
#define kCFCoreFoundationVersionNumber_iPhoneOS_3_2 478.61
#define kCFCoreFoundationVersionNumber_iOS_4_0 550.32
#define kCFCoreFoundationVersionNumber_iOS_4_1 550.38
#define kCFCoreFoundationVersionNumber_iOS_4_2 550.52
#define kCFCoreFoundationVersionNumber_iOS_4_3 550.52
#define kCFCoreFoundationVersionNumber_iOS_5_0 675.00
#define kCFCoreFoundationVersionNumber_iOS_5_1 690.10
#define kCFCoreFoundationVersionNumber_iOS_6_0 793.00
#define kCFCoreFoundationVersionNumber_iOS_6_1 793.00
#define kCFCoreFoundationVersionNumber_iOS_7_0 847.20
#define kCFCoreFoundationVersionNumber_iOS_7_1 847.24
#define kCFCoreFoundationVersionNumber_iOS_8_0 1140.1
#define kCFCoreFoundationVersionNumber_iOS_8_1 1141.14
#define kCFCoreFoundationVersionNumber_iOS_8_2 1142.16
#define kCFCoreFoundationVersionNumber_iOS_8_3 1144.17
#define kCFCoreFoundationVersionNumber_iOS_8_4 1145.15
#define kCFCoreFoundationVersionNumber_iOS_8_x_Max 1199
#define kCFCoreFoundationVersionNumber_iOS_9_0 1240.1
#define kCFCoreFoundationVersionNumber_iOS_9_1 1241.11
#define kCFCoreFoundationVersionNumber_iOS_9_2 1242.13
#define kCFCoreFoundationVersionNumber_iOS_9_3 1242.13
#define kCFCoreFoundationVersionNumber_iOS_9_4 1280.38
#define kCFCoreFoundationVersionNumber_iOS_9_x_Max 1299
#endif

#if __LLP64__
typedef unsigned long long CFTypeID;
typedef unsigned long long CFOptionFlags;
typedef unsigned long long CFHashCode;
typedef signed long long CFIndex;
#else
typedef unsigned long CFTypeID;
typedef unsigned long CFOptionFlags;
typedef unsigned long CFHashCode;
typedef signed long CFIndex;
#endif

/* Base "type" of all "CF objects", and polymorphic functions on them */
typedef const CF_BRIDGED_TYPE(id) void * CFTypeRef;

typedef const struct CF_BRIDGED_TYPE(NSString) __CFString * CFStringRef;
typedef struct CF_BRIDGED_MUTABLE_TYPE(NSMutableString) __CFString * CFMutableStringRef;

/*
        Type to mean any instance of a property list type;
        currently, CFString, CFData, CFNumber, CFBoolean, CFDate,
        CFArray, and CFDictionary.
*/
typedef CF_BRIDGED_TYPE(id) CFTypeRef CFPropertyListRef;

/* Values returned from comparison functions */
typedef CF_ENUM(CFIndex, CFComparisonResult) {
    kCFCompareLessThan = -1L,
    kCFCompareEqualTo = 0,
    kCFCompareGreaterThan = 1
};

/* A standard comparison function */
typedef CFComparisonResult (*CFComparatorFunction)(const void *val1, const void *val2, void *context);

/* Constant used by some functions to indicate failed searches. */
static const CFIndex kCFNotFound = -1;


/* Range type */
typedef struct {
    CFIndex location;
    CFIndex length;
} CFRange;

#if defined(CF_INLINE)
CF_INLINE CFRange CFRangeMake(CFIndex loc, CFIndex len) {
    CFRange range;
    range.location = loc;
    range.length = len;
    return range;
}
#else
#define CFRangeMake(LOC, LEN) __CFRangeMake(LOC, LEN)
#endif

/* Private; do not use */
CF_EXPORT
CFRange __CFRangeMake(CFIndex loc, CFIndex len);


/* Null representant */

typedef const struct CF_BRIDGED_TYPE(NSNull) __CFNull * CFNullRef;

CF_EXPORT
CFTypeID CFNullGetTypeID(void);

CF_EXPORT
const CFNullRef kCFNull;	// the singleton null instance


/* Allocator API

   Most of the time when specifying an allocator to Create functions, the NULL
   argument indicates "use the default"; this is the same as using kCFAllocatorDefault
   or the return value from CFAllocatorGetDefault().  This assures that you will use
   the allocator in effect at that time.
*/
typedef const struct CF_BRIDGED_TYPE(id) __CFAllocator * CFAllocatorRef;

/* This is a synonym for NULL, if you'd rather use a named constant. */
CF_EXPORT
const CFAllocatorRef kCFAllocatorDefault;

/* Default system allocator; you rarely need to use this. */
CF_EXPORT
const CFAllocatorRef kCFAllocatorSystemDefault;

/* This allocator uses malloc(), realloc(), and free(). This should not be
   generally used; stick to kCFAllocatorDefault whenever possible. This
   allocator is useful as the "bytesDeallocator" in CFData or
   "contentsDeallocator" in CFString where the memory was obtained as a
   result of malloc() type functions.
*/
CF_EXPORT
const CFAllocatorRef kCFAllocatorMalloc;

/* This allocator explicitly uses the default malloc zone, returned by
   malloc_default_zone(). It should only be used when an object is
   safe to be allocated in non-scanned memory.
 */
CF_EXPORT
const CFAllocatorRef kCFAllocatorMallocZone;

/* Null allocator which does nothing and allocates no memory. This allocator
   is useful as the "bytesDeallocator" in CFData or "contentsDeallocator"
   in CFString where the memory should not be freed. 
*/
CF_EXPORT
const CFAllocatorRef kCFAllocatorNull;

/* Special allocator argument to CFAllocatorCreate() which means
   "use the functions given in the context to allocate the allocator
   itself as well". 
*/
CF_EXPORT
const CFAllocatorRef kCFAllocatorUseContext;

typedef const void *	(*CFAllocatorRetainCallBack)(const void *info);
typedef void		(*CFAllocatorReleaseCallBack)(const void *info);
typedef CFStringRef	(*CFAllocatorCopyDescriptionCallBack)(const void *info);
typedef void *		(*CFAllocatorAllocateCallBack)(CFIndex allocSize, CFOptionFlags hint, void *info);
typedef void *		(*CFAllocatorReallocateCallBack)(void *ptr, CFIndex newsize, CFOptionFlags hint, void *info);
typedef void		(*CFAllocatorDeallocateCallBack)(void *ptr, void *info);
typedef CFIndex		(*CFAllocatorPreferredSizeCallBack)(CFIndex size, CFOptionFlags hint, void *info);
typedef struct {
    CFIndex				version;
    void *				info;
    CFAllocatorRetainCallBack		retain;
    CFAllocatorReleaseCallBack		release;        
    CFAllocatorCopyDescriptionCallBack	copyDescription;
    CFAllocatorAllocateCallBack		allocate;
    CFAllocatorReallocateCallBack	reallocate;
    CFAllocatorDeallocateCallBack	deallocate;
    CFAllocatorPreferredSizeCallBack	preferredSize;
} CFAllocatorContext;

CF_EXPORT
CFTypeID	CFAllocatorGetTypeID(void);

/*
	CFAllocatorSetDefault() sets the allocator that is used in the current
	thread whenever NULL is specified as an allocator argument. This means
	that most, if not all allocations will go through this allocator. It
	also means that any allocator set as the default needs to be ready to
	deal with arbitrary memory allocation requests; in addition, the size
	and number of requests will change between releases.

	An allocator set as the default will never be released, even if later
	another allocator replaces it as the default. Not only is it impractical
	for it to be released (as there might be caches created under the covers
	that refer to the allocator), in general it's also safer and more
	efficient to keep it around.

	If you wish to use a custom allocator in a context, it's best to provide
	it as the argument to the various creation functions rather than setting
	it as the default. Setting the default allocator is not encouraged.

	If you do set an allocator as the default, either do it for all time in
	your app, or do it in a nested fashion (by restoring the previous allocator
	when you exit your context). The latter might be appropriate for plug-ins
	or libraries that wish to set the default allocator.
*/
CF_EXPORT
void CFAllocatorSetDefault(CFAllocatorRef allocator);

CF_EXPORT
CFAllocatorRef CFAllocatorGetDefault(void);

CF_EXPORT
CFAllocatorRef CFAllocatorCreate(CFAllocatorRef allocator, CFAllocatorContext *context);

CF_EXPORT
void *CFAllocatorAllocate(CFAllocatorRef allocator, CFIndex size, CFOptionFlags hint);

CF_EXPORT
void *CFAllocatorReallocate(CFAllocatorRef allocator, void *ptr, CFIndex newsize, CFOptionFlags hint);

CF_EXPORT
void CFAllocatorDeallocate(CFAllocatorRef allocator, void *ptr);

CF_EXPORT
CFIndex CFAllocatorGetPreferredSizeForSize(CFAllocatorRef allocator, CFIndex size, CFOptionFlags hint);

CF_EXPORT
void CFAllocatorGetContext(CFAllocatorRef allocator, CFAllocatorContext *context);


/* Polymorphic CF functions */

CF_IMPLICIT_BRIDGING_ENABLED

CF_EXPORT
CFTypeID CFGetTypeID(CFTypeRef cf);

CF_EXPORT
CFStringRef CFCopyTypeIDDescription(CFTypeID type_id);

CF_EXPORT
CFTypeRef CFRetain(CFTypeRef cf);

CF_EXPORT
void CFRelease(CFTypeRef cf);

#if DEPLOYMENT_RUNTIME_SWIFT
#else
CF_EXPORT
CFTypeRef CFAutorelease(CFTypeRef CF_RELEASES_ARGUMENT arg) API_AVAILABLE(macos(10.9), ios(7.0), watchos(2.0), tvos(9.0));

CF_EXPORT
CFIndex CFGetRetainCount(CFTypeRef cf);
#endif

CF_EXPORT
Boolean CFEqual(CFTypeRef cf1, CFTypeRef cf2);

CF_EXPORT
CFHashCode CFHash(CFTypeRef cf);

CF_EXPORT
CFStringRef CFCopyDescription(CFTypeRef cf);

CF_EXPORT
CFAllocatorRef CFGetAllocator(CFTypeRef cf);

CF_IMPLICIT_BRIDGING_DISABLED

// This function is unavailable in ARC mode. On OS X 10.12 and later, this function simply returns the argument.
CF_EXPORT
CFTypeRef CFMakeCollectable(CFTypeRef cf) CF_AUTOMATED_REFCOUNT_UNAVAILABLE;

#if DEPLOYMENT_RUNTIME_SWIFT

#if TARGET_RT_64_BIT
#define _CF_SWIFT_RC_PINNED_FLAG (0x80000004ffffffff)
#else
#define _CF_SWIFT_RC_PINNED_FLAG (0x800004FF)
#endif
#define _CF_CONSTANT_OBJECT_STRONG_RC ((uintptr_t)_CF_SWIFT_RC_PINNED_FLAG)
#endif

#if __has_include(<ptrauth.h>)
#include <ptrauth.h>
#endif

#ifndef __ptrauth_objc_isa_pointer
#define __ptrauth_objc_isa_pointer
#endif

#define ISA_PTRAUTH_DISCRIMINATOR 0x6AE1
#if defined(__ptrauth_objc_isa_uintptr)
#define __ptrauth_cf_objc_isa_pointer __ptrauth_objc_isa_uintptr
#elif defined(__arm64e__) && defined(__PTRAUTH_INTRINSICS__) && __has_feature(ptrauth_objc_isa)
#define __ptrauth_cf_objc_isa_pointer __ptrauth_restricted_intptr(ptrauth_key_objc_isa_pointer, 1, ISA_PTRAUTH_DISCRIMINATOR)
#else
#define __ptrauth_cf_objc_isa_pointer
#endif

CF_EXTERN_C_END

#endif /* ! __COREFOUNDATION_CFBASE__ */

