/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted,free of charge,to any person obtaining a copy of this software and associated documentation files (the "Software"),to deal in the Software without restriction,including without limitation the rights to use,copy,modify,merge,publish,distribute,sublicense,and/or sell copies of the Software,and to permit persons to whom the Software is furnished to do so,subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS",WITHOUT WARRANTY OF ANY KIND,EXPRESS OR IMPLIED,INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,DAMAGES OR OTHER LIABILITY,WHETHER IN AN ACTION OF CONTRACT,TORT OR OTHERWISE,ARISING FROM,OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <sys/types.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <time.h>

#ifdef __clang__
#define COREFOUNDATION_DLLEXPORT
#define COREFOUNDATION_DLLIMPORT
#else
#define COREFOUNDATION_DLLEXPORT __declspec(dllexport)
#define COREFOUNDATION_DLLIMPORT __declspec(dllimport)
#endif

#ifdef __cplusplus

#if defined(__WIN32__)
#if defined(COREFOUNDATION_INSIDE_BUILD)
#define COREFOUNDATION_EXPORT extern "C" COREFOUNDATION_DLLEXPORT
#else
#define COREFOUNDATION_EXPORT extern "C" COREFOUNDATION_DLLIMPORT
#endif
#else
#define COREFOUNDATION_EXPORT extern "C"
#endif

#else

#if defined(__WIN32__)
#if defined(COREFOUNDATION_INSIDE_BUILD)
#define COREFOUNDATION_EXPORT COREFOUNDATION_DLLEXPORT extern
#else
#define COREFOUNDATION_EXPORT COREFOUNDATION_DLLIMPORT extern
#endif
#else
#define COREFOUNDATION_EXPORT extern
#endif

#endif // __cplusplus

/* Apple's Foundation imports CoreGraphics in order to get some of the basic CG* types, unfortunately
   this is a hassle on platforms where you just want to use Foundation, so we put them in CoreFoundation and see what happens
*/

enum {
    kCFNotFound = LONG_MAX
};

typedef float CGFloat;
#define CGFLOAT_MIN FLT_MIN
#define CGFLOAT_MAX FLT_MAX
#define CGFLOAT_SCAN "%g"

typedef struct CGPoint {
    CGFloat x;
    CGFloat y;
} CGPoint;

typedef struct CGSize {
    CGFloat width;
    CGFloat height;
} CGSize;

typedef struct CGRect {
    CGPoint origin;
    CGSize size;
} CGRect;

typedef unsigned short UniChar;
typedef unsigned int UTF32Char;
typedef float Float32;
typedef double Float64;
// ---

#if defined(__LP64__)
typedef long CFInteger;
typedef unsigned long CFUInteger;
#else
typedef int CFInteger;
typedef unsigned int CFUInteger;
#endif

typedef int8_t SInt8;
typedef uint8_t UInt8;
typedef int16_t SInt16;
typedef uint16_t UInt16;
typedef int32_t SInt32;
typedef uint32_t UInt32;
typedef int64_t SInt64;
typedef uint64_t UInt64;

typedef const void *CFTypeRef;
typedef CFUInteger CFTypeID;
typedef CFUInteger CFHashCode;
typedef char Boolean;
typedef CFInteger CFIndex;
typedef CFUInteger CFOptionFlags;

typedef SInt32 OSStatus;
typedef SInt16 OSErr;

typedef UInt32 FourCharCode;
typedef FourCharCode OSType;

typedef struct {
    CFIndex location;
    CFIndex length;
} CFRange;

static inline CFRange CFRangeMake(CFIndex loc, CFIndex len) {
    CFRange result = {loc, len};

    return result;
}

#ifndef TRUE
#define TRUE ((Boolean)1)
#endif

#ifndef FALSE
#define FALSE ((Boolean)0)
#endif

typedef enum {
    kCFCompareLessThan = -1,
    kCFCompareEqualTo = 0,
    kCFCompareGreaterThan = 1
} CFComparisonResult;

typedef CFComparisonResult (*CFComparatorFunction)(const void *value, const void *other, void *context);

typedef struct CFAllocator *CFAllocatorRef;

#import <CoreFoundation/CFString.h>

typedef void *(*CFAllocatorAllocateCallBack)(CFIndex size, CFOptionFlags hint, void *info);
typedef CFStringRef (*CFAllocatorCopyDescriptionCallBack)(const void *info);
typedef void (*CFAllocatorDeallocateCallBack)(void *ptr, void *info);
typedef CFIndex (*CFAllocatorPreferredSizeCallBack)(CFIndex size, CFOptionFlags hint, void *info);
typedef void *(*CFAllocatorReallocateCallBack)(void *ptr, CFIndex size, CFOptionFlags hint, void *info);
typedef void (*CFAllocatorReleaseCallBack)(const void *info);
typedef const void *(*CFAllocatorRetainCallBack)(const void *info);

typedef struct {
    CFIndex version;
    void *info;
    CFAllocatorRetainCallBack retain;
    CFAllocatorReleaseCallBack release;
    CFAllocatorCopyDescriptionCallBack copyDescription;
    CFAllocatorAllocateCallBack allocate;
    CFAllocatorReallocateCallBack reallocate;
    CFAllocatorDeallocateCallBack deallocate;
    CFAllocatorPreferredSizeCallBack preferredSize;
} CFAllocatorContext;

COREFOUNDATION_EXPORT const CFAllocatorRef kCFAllocatorDefault;
COREFOUNDATION_EXPORT const CFAllocatorRef kCFAllocatorSystemDefault;
COREFOUNDATION_EXPORT const CFAllocatorRef kCFAllocatorMalloc;
COREFOUNDATION_EXPORT const CFAllocatorRef kCFAllocatorMallocZone;
COREFOUNDATION_EXPORT const CFAllocatorRef kCFAllocatorNull;
COREFOUNDATION_EXPORT const CFAllocatorRef kCFAllocatorUseContext;

COREFOUNDATION_EXPORT CFAllocatorRef CFAllocatorGetDefault(void);
COREFOUNDATION_EXPORT void CFAllocatorSetDefault(CFAllocatorRef self);

COREFOUNDATION_EXPORT CFTypeID CFAllocatorGetTypeID(void);

COREFOUNDATION_EXPORT CFAllocatorRef CFAllocatorCreate(CFAllocatorRef self, CFAllocatorContext *context);

COREFOUNDATION_EXPORT void CFAllocatorGetContext(CFAllocatorRef self, CFAllocatorContext *context);
COREFOUNDATION_EXPORT CFIndex CFAllocatorGetPreferredSizeForSize(CFAllocatorRef self, CFIndex size, CFOptionFlags hint);

COREFOUNDATION_EXPORT void *CFAllocatorAllocate(CFAllocatorRef self, CFIndex size, CFOptionFlags hint);
COREFOUNDATION_EXPORT void CFAllocatorDeallocate(CFAllocatorRef self, void *ptr);
COREFOUNDATION_EXPORT void *CFAllocatorReallocate(CFAllocatorRef self, void *ptr, CFIndex size, CFOptionFlags hint);

COREFOUNDATION_EXPORT CFTypeID CFGetTypeID(CFTypeRef self);

COREFOUNDATION_EXPORT CFTypeRef CFRetain(CFTypeRef self);
COREFOUNDATION_EXPORT void CFRelease(CFTypeRef self);
COREFOUNDATION_EXPORT CFIndex CFGetRetainCount(CFTypeRef self);

COREFOUNDATION_EXPORT CFAllocatorRef CFGetAllocator(CFTypeRef self);

COREFOUNDATION_EXPORT CFHashCode CFHash(CFTypeRef self);
COREFOUNDATION_EXPORT Boolean CFEqual(CFTypeRef self, CFTypeRef other);
COREFOUNDATION_EXPORT CFStringRef CFCopyTypeIDDescription(CFTypeID typeID);
COREFOUNDATION_EXPORT CFStringRef CFCopyDescription(CFTypeRef self);
COREFOUNDATION_EXPORT CFTypeRef CFMakeCollectable(CFTypeRef self);

#ifndef MACH

// mach/mach_types.h

typedef int kern_return_t;

#define KERN_SUCCESS 0
#define KERN_FAILURE 5

#ifndef _MACH_PORT_T
#define _MACH_PORT_T
typedef int mach_port_t;
#endif

// mach/mach_time.h

typedef struct mach_timebase_info {
    uint32_t numer;
    uint32_t denom;
} mach_timebase_info_data_t, *mach_timebase_info_t;

uint64_t mach_absolute_time(void);
kern_return_t mach_timebase_info(mach_timebase_info_t timebase);
#endif

#ifdef WINDOWS
COREFOUNDATION_EXPORT unsigned int sleep(unsigned int seconds);
//COREFOUNDATION_EXPORT int usleep(long useconds);
COREFOUNDATION_EXPORT size_t strlcpy(char *dst, const char *src, size_t size);
COREFOUNDATION_EXPORT char *strnstr(const char *s1, const char *s2, size_t n);
COREFOUNDATION_EXPORT void bzero(void *ptr, size_t size);
COREFOUNDATION_EXPORT void bcopy(const void *s1, void *s2, size_t n);
COREFOUNDATION_EXPORT int bcmp(const void *s1, void *s2, size_t n);
COREFOUNDATION_EXPORT int mkstemps(char *tmplt, int suffixlen);
COREFOUNDATION_EXPORT long random(void);
#endif

// Enums and Options
#if (__cplusplus && __cplusplus >= 201103L && (__has_extension(cxx_strong_enums) || __has_feature(objc_fixed_enum))) || (!__cplusplus && __has_feature(objc_fixed_enum))
#define CF_ENUM(_type, _name) enum _name : _type _name; enum _name : _type
#if (__cplusplus)
#define CF_OPTIONS(_type, _name) _type _name; enum : _type
#else
#define CF_OPTIONS(_type, _name) enum _name : _type _name; enum _name : _type
#endif
#else
#define CF_ENUM(_type, _name) _type _name; enum
#define CF_OPTIONS(_type, _name) _type _name; enum
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

#if __has_attribute(objc_bridge)

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
