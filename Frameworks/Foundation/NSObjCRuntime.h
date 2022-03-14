/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <objc/objc.h>
#include <stdarg.h>
#include <stdint.h>
#include <limits.h>

#ifdef __clang__
#define FOUNDATION_DLLEXPORT
#define FOUNDATION_DLLIMPORT
#else
#define FOUNDATION_DLLEXPORT __declspec(dllexport)
#define FOUNDATION_DLLIMPORT __declspec(dllimport)
#endif

#ifdef __cplusplus

#if defined(__WIN32__)
#if defined(FOUNDATION_INSIDE_BUILD)
#define FOUNDATION_EXPORT extern "C" FOUNDATION_DLLEXPORT
#else
#define FOUNDATION_EXPORT extern "C" FOUNDATION_DLLIMPORT
#endif
#else
#define FOUNDATION_EXPORT extern "C"
#endif

#else

#if defined(__WIN32__)
#if defined(FOUNDATION_INSIDE_BUILD)
#define FOUNDATION_EXPORT FOUNDATION_DLLEXPORT extern
#else
#define FOUNDATION_EXPORT FOUNDATION_DLLIMPORT extern
#endif
#else
#define FOUNDATION_EXPORT extern
#endif

#endif

#define NS_INLINE static inline

#ifdef __clang__
#define NS_ROOT_CLASS __attribute__((objc_root_class))
#else
#define NS_ROOT_CLASS
#endif

#ifndef __has_feature
#define __has_feature(x) 0
#endif

#ifndef __has_extension
#define __has_extension(x) 0
#endif

#ifndef __has_attribute
#define __has_attribute(x) 0
#endif

#if __has_feature(attribute_ns_returns_retained)
#define NS_RETURNS_RETAINED __attribute__((ns_returns_retained))
#else
#define NS_RETURNS_RETAINED
#endif

#if __has_feature(attribute_ns_returns_not_retained)
#define NS_RETURNS_NOT_RETAINED __attribute__((ns_returns_not_retained))
#else
#define NS_RETURNS_NOT_RETAINED
#endif

#ifndef CF_RETURNS_RETAINED
#if __has_feature(attribute_cf_returns_retained)
#define CF_RETURNS_RETAINED __attribute__((cf_returns_retained))
#else
#define CF_RETURNS_RETAINED
#endif
#endif

@class NSString;

#define NSINTEGER_DEFINED 1

#if defined(__LP64__)
typedef long NSInteger;
typedef unsigned long NSUInteger;
#define NSIntegerMax LONG_MAX
#define NSIntegerMin LONG_MIN
#define NSUIntegerMax ULONG_MAX
#define NSIntegerFormat "%ld"
#define NSUIntegerFormat "%lu"
#else
typedef int NSInteger;
typedef unsigned int NSUInteger;
#define NSIntegerMax INT_MAX
#define NSIntegerMin INT_MIN
#define NSUIntegerMax UINT_MAX
#define NSIntegerFormat "%d"
#define NSUIntegerFormat "%u"
#endif

enum {
    NSOrderedAscending = -1,
    NSOrderedSame = 0,
    NSOrderedDescending = 1
};

typedef NSInteger NSComparisonResult;

#define NSNotFound NSIntegerMax

#ifndef MIN
#define MIN(a, b) ({__typeof__(a) _a = (a); __typeof__(b) _b = (b); (_a < _b) ? _a : _b; })
//#else
//#warning MIN is already defined, MIN(a, b) may not behave as expected.
#endif

#ifndef MAX
#define MAX(a, b) ({__typeof__(a) _a = (a); __typeof__(b) _b = (b); (_a > _b) ? _a : _b; })
//#else
//#warning MAX is already defined, MAX(a, b) may not not behave as expected.
#endif

#ifndef ABS
#define ABS(a) ({__typeof__(a) _a = (a); (_a < 0) ? -_a : _a; })
#else
#warning ABS is already defined, ABS(a) may not behave as expected.
#endif

#ifndef NS_ENUM
#define NS_ENUM(_type, _name) \
    _type _name;              \
    enum
#endif

#ifndef NS_OPTIONS
#define NS_OPTIONS(_type, _name) \
    _type _name;                 \
    enum
#endif

FOUNDATION_EXPORT void NSLog(NSString *format, ...);
FOUNDATION_EXPORT void NSLogv(NSString *format, va_list args);

FOUNDATION_EXPORT const char *NSGetSizeAndAlignment(const char *type, NSUInteger *size, NSUInteger *alignment);

FOUNDATION_EXPORT SEL NSSelectorFromString(NSString *selectorName);
FOUNDATION_EXPORT NSString *NSStringFromSelector(SEL selector);

FOUNDATION_EXPORT Class NSClassFromString(NSString *className);
FOUNDATION_EXPORT NSString *NSStringFromClass(Class aClass);
