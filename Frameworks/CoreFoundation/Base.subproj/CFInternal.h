/*	CFInternal.h
	Copyright (c) 1998-2019, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
*/

/*
        NOT TO BE USED OUTSIDE CF!
*/

#if !CF_BUILDING_CF
    #error The header file CFInternal.h is for the exclusive use of CoreFoundation. No other project should include it.
#endif

#if !defined(__COREFOUNDATION_CFINTERNAL__)
#define __COREFOUNDATION_CFINTERNAL__ 1

#if __has_include(<CoreFoundation/TargetConditionals.h>)
#include <CoreFoundation/TargetConditionals.h>
#else
#include <TargetConditionals.h>
#endif


#define __CF_COMPILE_YEAR__	(__DATE__[7] * 1000 + __DATE__[8] * 100 + __DATE__[9] * 10 + __DATE__[10] - 53328)
#define __CF_COMPILE_MONTH__	((__DATE__[1] + __DATE__[2] == 207) ? 1 : \
				 (__DATE__[1] + __DATE__[2] == 199) ? 2 : \
				 (__DATE__[1] + __DATE__[2] == 211) ? 3 : \
				 (__DATE__[1] + __DATE__[2] == 226) ? 4 : \
				 (__DATE__[1] + __DATE__[2] == 218) ? 5 : \
				 (__DATE__[1] + __DATE__[2] == 227) ? 6 : \
				 (__DATE__[1] + __DATE__[2] == 225) ? 7 : \
				 (__DATE__[1] + __DATE__[2] == 220) ? 8 : \
				 (__DATE__[1] + __DATE__[2] == 213) ? 9 : \
				 (__DATE__[1] + __DATE__[2] == 215) ? 10 : \
				 (__DATE__[1] + __DATE__[2] == 229) ? 11 : \
				 (__DATE__[1] + __DATE__[2] == 200) ? 12 : 0)
#define __CF_COMPILE_DAY__	(__DATE__[4] * 10 + __DATE__[5] - (__DATE__[4] == ' ' ? 368 : 528))
#define __CF_COMPILE_DATE__	(__CF_COMPILE_YEAR__ * 10000 + __CF_COMPILE_MONTH__ * 100 + __CF_COMPILE_DAY__)

#define __CF_COMPILE_HOUR__	(__TIME__[0] * 10 + __TIME__[1] - 528)
#define __CF_COMPILE_MINUTE__	(__TIME__[3] * 10 + __TIME__[4] - 528)
#define __CF_COMPILE_SECOND__	(__TIME__[6] * 10 + __TIME__[7] - 528)
#define __CF_COMPILE_TIME__	(__CF_COMPILE_HOUR__ * 10000 + __CF_COMPILE_MINUTE__ * 100 + __CF_COMPILE_SECOND__)

#define __CF_COMPILE_SECOND_OF_DAY__	(__CF_COMPILE_HOUR__ * 3600 + __CF_COMPILE_MINUTE__ * 60 + __CF_COMPILE_SECOND__)

// __CF_COMPILE_DAY_OF_EPOCH__ works within Gregorian years 2001 - 2099; the epoch is of course CF's epoch
#define __CF_COMPILE_DAY_OF_EPOCH__	((__CF_COMPILE_YEAR__ - 2001) * 365 + (__CF_COMPILE_YEAR__ - 2001) / 4 \
					+ ((__DATE__[1] + __DATE__[2] == 207) ? 0 : \
					   (__DATE__[1] + __DATE__[2] == 199) ? 31 : \
					   (__DATE__[1] + __DATE__[2] == 211) ? 59 + (__CF_COMPILE_YEAR__ % 4 == 0) : \
					   (__DATE__[1] + __DATE__[2] == 226) ? 90 + (__CF_COMPILE_YEAR__ % 4 == 0) : \
					   (__DATE__[1] + __DATE__[2] == 218) ? 120 + (__CF_COMPILE_YEAR__ % 4 == 0) : \
					   (__DATE__[1] + __DATE__[2] == 227) ? 151 + (__CF_COMPILE_YEAR__ % 4 == 0) : \
					   (__DATE__[1] + __DATE__[2] == 225) ? 181 + (__CF_COMPILE_YEAR__ % 4 == 0) : \
					   (__DATE__[1] + __DATE__[2] == 220) ? 212 + (__CF_COMPILE_YEAR__ % 4 == 0) : \
					   (__DATE__[1] + __DATE__[2] == 213) ? 243 + (__CF_COMPILE_YEAR__ % 4 == 0) : \
					   (__DATE__[1] + __DATE__[2] == 215) ? 273 + (__CF_COMPILE_YEAR__ % 4 == 0) : \
					   (__DATE__[1] + __DATE__[2] == 229) ? 304 + (__CF_COMPILE_YEAR__ % 4 == 0) : \
					   (__DATE__[1] + __DATE__[2] == 200) ? 334 + (__CF_COMPILE_YEAR__ % 4 == 0) : \
					    365 + (__CF_COMPILE_YEAR__ % 4 == 0)) \
					+ __CF_COMPILE_DAY__)


#if __has_feature(attribute_ns_returns_retained)
#define _CF_RETURNS_RETAINED_OBJ __attribute__((ns_returns_retained))
#else
#define _CF_RETURNS_RETAINED_OBJ
#endif

#if __has_feature(attribute_ns_returns_not_retained)
#define _CF_RETURNS_NOT_RETAINED_OBJ __attribute__((ns_returns_not_retained))
#else
#define _CF_RETURNS_NOT_RETAINED_OBJ
#endif

#if __has_feature(attribute_ns_consumed)
#if !__OBJC__
#define _CF_RELEASES_ARGUMENT_OBJ __attribute__((cf_consumed))
#else
#define _CF_RELEASES_ARGUMENT_OBJ __attribute__((ns_consumed))
#endif
#else
#define _CF_RELEASES_ARGUMENT_OBJ
#endif

// For places where we need to return a +1 to satisfy the analyzer but the returned value is actually an un-deallocable singleton, use this macro.
#define _CF_RETURNS_SINGLETON CF_RETURNS_RETAINED

CF_EXTERN_C_BEGIN

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFURL.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFDate.h>
#include <CoreFoundation/CFArray.h>
#include <CoreFoundation/CFLogUtilities.h>
#include <CoreFoundation/CFRuntime.h>
#include "CFRuntime_Internal.h"
#include <limits.h>
#include <stdatomic.h>
#include <Block.h>

#if TARGET_OS_MAC || TARGET_OS_LINUX || TARGET_OS_BSD || TARGET_OS_WASI

#if TARGET_OS_MAC || (TARGET_OS_BSD && !defined(__OpenBSD__)) || TARGET_OS_ANDROID
#include <xlocale.h>
#endif // TARGET_OS_MAC || (TARGET_OS_BSD && !defined(__OpenBSD__)) || TARGET_OS_ANDROID

#include <sys/time.h>
#include <signal.h>
#include <stdio.h>
#endif // TARGET_OS_MAC || TARGET_OS_LINUX || TARGET_OS_BSD || TARGET_OS_WASI

#if __has_include(<unistd.h>)
#include <unistd.h>
#endif
#if _POSIX_THREADS
#include <pthread.h>
#endif
#if __has_include(<pthread_np.h>)
#include <pthread_np.h>
#endif

#if !DEPLOYMENT_RUNTIME_SWIFT && __has_include(<os/log.h>)
#include <os/log.h>
#else
typedef struct os_log_s *os_log_t;
#define os_log(...) do { } while (0)
#define os_log_info(...) do { } while (0)
#define os_log_debug(...) do { } while (0)
#define os_log_error(...) do { } while (0)
#define os_log_fault(...) do { } while (0)
#define os_log_create(...) (NULL)
#define os_log_debug_enabled(...) (0)
#endif

// We want to eventually note that some objects are immortal to the Swift runtime, but this stopgap lets things work while we work to make an ABI for them.
#if DEPLOYMENT_RUNTIME_SWIFT
#define _CF_CONSTANT_OBJECT_BACKING // We don't support this on Swift
#else
#define _CF_CONSTANT_OBJECT_BACKING const
#endif

#if TARGET_OS_OSX && DEPLOYMENT_RUNTIME_SWIFT
// This target configuration some how misses the availability macros to let these be defined, so this works-around the missing definitions
#ifndef PTHREAD_ERRORCHECK_MUTEX_INITIALIZER
#define PTHREAD_ERRORCHECK_MUTEX_INITIALIZER {_PTHREAD_ERRORCHECK_MUTEX_SIG_init, {0}}
#endif
#ifndef PTHREAD_RECURSIVE_MUTEX_INITIALIZER
#define PTHREAD_RECURSIVE_MUTEX_INITIALIZER {_PTHREAD_RECURSIVE_MUTEX_SIG_init, {0}}
#endif
#endif

#if defined(__BIG_ENDIAN__)
#define __CF_BIG_ENDIAN__ 1
#define __CF_LITTLE_ENDIAN__ 0
#endif

#if defined(__LITTLE_ENDIAN__)
#define __CF_LITTLE_ENDIAN__ 1
#define __CF_BIG_ENDIAN__ 0
#endif

#include <CoreFoundation/ForFoundationOnly.h>
#if DEPLOYMENT_RUNTIME_SWIFT
#include <CoreFoundation/ForSwiftFoundationOnly.h>
#include <CoreFoundation/CFString.h>
#endif

CF_EXPORT const char *_CFProcessName(void);
CF_PRIVATE CFStringRef _CFProcessNameString(void);

CF_EXPORT Boolean _CFGetCurrentDirectory(char *path, int maxlen);

CF_EXPORT CFArrayRef _CFGetWindowsBinaryDirectories(void);

CF_EXPORT CFStringRef _CFStringCreateHostName(void);

#if TARGET_OS_MAC
#include <CoreFoundation/CFRunLoop.h>
CF_EXPORT void _CFMachPortInstallNotifyPort(CFRunLoopRef rl, CFStringRef mode);
#endif


CF_PRIVATE os_log_t _CFOSLog(void);
CF_PRIVATE os_log_t _CFMethodSignatureROMLog(void);

// Messages logged with os_log_fault() to this log will show in the IDE as
// Runtime Issues warnings to framework clients, including external developers,
// and will be attributed to CoreFoundation.
// This log should only be used for warnings that the client can take action to
// address, such as API misuse, for APIs that are conceptually part of
// CoreFoundation from the client's perspective.
CF_PRIVATE os_log_t _CFRuntimeIssuesLog(void);

// Messages logged with os_log_fault() to this log will show in the IDE as
// Runtime Issues warnings to framework clients, including external developers.
// The warnings will be attributed to Foundation not CoreFoundation.
// This log should only be used for warnings that the client can take action to
// address, such as API misuse, for APIs that are conceptually part of Foundation
// from the client's perspective.
CF_PRIVATE os_log_t _CFFoundationRuntimeIssuesLog(void);

CF_PRIVATE CFIndex __CFActiveProcessorCount(void);

#define HALT __builtin_trap()
#define HALT_MSG(str) do { CRSetCrashLogMessage(str); HALT; } while (0)

#ifndef CLANG_ANALYZER_NORETURN
#if __has_feature(attribute_analyzer_noreturn)
#define CLANG_ANALYZER_NORETURN __attribute__((analyzer_noreturn))
#else
#define CLANG_ANALYZER_NORETURN
#endif
#endif

// Use this in places where the result may be nil but the function is marked as nonnull (http://clang-analyzer.llvm.org/faq.html#nullability_intentional_violation)
// e.g. `return _CLANG_ANALYZER_IGNORE_NONNULL(nil);` or `id x = nil; return _CLANG_ANALYZER_IGNORE_NONNULL(x);`
#define _CLANG_ANALYZER_IGNORE_NONNULL(x) ((id _Nonnull)x)

// For places where we need to return a +1 to satisfy the analyzer but the returned value is actually an un-deallocable singleton, use this macro.
#ifdef __clang_analyzer__
#define _CLANG_ANALYZER_IGNORE_RETAIN(x) do { CFAutorelease(x); } while (0)
#else
#define _CLANG_ANALYZER_IGNORE_RETAIN(x)
#endif

// For places where we want to assert something is true, but only for analyzer purposes.
#ifdef __clang_analyzer__
#define _CLANG_ANALYZER_ASSERT(x) do { if (!(x)) HALT_MSG("Analyzer-only assert failed"); } while (0)
#else
#define _CLANG_ANALYZER_ASSERT(x)
#endif

#ifdef __clang_analyzer__
#define _CLANG_ANALYZER_IGNORE_UNINITIALIZED_BUFFER(buf, size) do { memset((buf), 0, (size)); } while (0)
#else
#define _CLANG_ANALYZER_IGNORE_UNINITIALIZED_BUFFER(buf, size)
#endif


#if TARGET_OS_WIN32
#define __builtin_unreachable() do { } while (0)
#endif

#if defined(DEBUG)
    #define CFAssert(cond, prio, desc) do { if (!(cond)) { CFLog(prio, CFSTR(desc)); /* HALT; */ } } while (0)
    #define CFAssert1(cond, prio, desc, a1) do { if (!(cond)) { CFLog(prio, CFSTR(desc), a1); /* HALT; */ } } while (0)
    #define CFAssert2(cond, prio, desc, a1, a2) do { if (!(cond)) { CFLog(prio, CFSTR(desc), a1, a2); /* HALT; */ } } while (0)
    #define CFAssert3(cond, prio, desc, a1, a2, a3) do { if (!(cond)) { CFLog(prio, CFSTR(desc), a1, a2, a3); /* HALT; */ } } while (0)
    #define CFAssert4(cond, prio, desc, a1, a2, a3, a4) do { if (!(cond)) { CFLog(prio, CFSTR(desc), a1, a2, a3, a4); /* HALT; */ } } while (0)
#else
    #define CFAssert(cond, prio, desc) do {} while (0)
    #define CFAssert1(cond, prio, desc, a1) do {} while (0)
    #define CFAssert2(cond, prio, desc, a1, a2) do {} while (0)
    #define CFAssert3(cond, prio, desc, a1, a2, a3) do {} while (0)
    #define CFAssert4(cond, prio, desc, a1, a2, a3, a4) do {} while (0)
#endif

#define __kCFLogAssertion	3

// This CF-only log function uses no CF functionality, so it may be called anywhere within CF - including thread teardown or prior to full CF setup
CF_PRIVATE void _CFLogSimple(int32_t lev, char *format, ...);

#if defined(DEBUG)
extern void __CFGenericValidateType_(CFTypeRef cf, CFTypeID type, const char *func);
#define __CFGenericValidateType(cf, type) __CFGenericValidateType_(cf, type, __PRETTY_FUNCTION__)
#else
#define __CFGenericValidateType(cf, type) ((void)0)
#endif

/* Bit manipulation macros */
/* Bits are numbered from 31 on left to 0 on right */
/* May or may not work if you use them on bitfields in types other than UInt32, bitfields the full width of a UInt32, or anything else for which they were not designed. */
/* In the following, N1 and N2 specify an inclusive range N2..N1 with N1 >= N2 */
#define __CFBitfieldMask(N1, N2)	((((UInt32)~0UL) << (31UL - (N1) + (N2))) >> (31UL - N1))
#define __CFBitfieldGetValue(V, N1, N2)	(((V) & __CFBitfieldMask(N1, N2)) >> (N2))
#define __CFBitfieldSetValue(V, N1, N2, X)	((V) = ((V) & ~__CFBitfieldMask(N1, N2)) | (((X) << (N2)) & __CFBitfieldMask(N1, N2)))

#define __CFBitfield64Mask(N1, N2)	((((uint64_t)~0ULL) << (63ULL - (N1) + (N2))) >> (63ULL - N1))
#define __CFBitfield64GetValue(V, N1, N2)	(((V) & __CFBitfield64Mask(N1, N2)) >> (N2))
#define __CFBitfield64SetValue(V, N1, N2, X)	((V) = ((V) & ~__CFBitfield64Mask(N1, N2)) | ((((uint64_t)X) << (N2)) & __CFBitfield64Mask(N1, N2)))

#if TARGET_RT_64_BIT || TARGET_OS_ANDROID
typedef uint64_t __CFInfoType;
#define __CFInfoMask(N1, N2) __CFBitfield64Mask(N1, N2)
#else
typedef uint32_t __CFInfoType;
#define __CFInfoMask(N1, N2) __CFBitfieldMask(N1, N2)
#endif

/// Get a value from a CFTypeRef info bitfield.
///
/// Bits are numbered from 6 on left to 0 on right. n1 and n2 specify an inclusive range n1..n2 with n1 >= n2.
/// For example:
///  n1 == 6, n2 == 4 will result in using the mask 0x0070. The value must fit inside 3 bits (6 - 4 + 1).
///  n1 == 0, n2 == 0 will result in using the mask 0x0001. The value must be 1 bit (0 - 0 + 1).
static inline uint8_t __CFRuntimeGetValue(CFTypeRef cf, uint8_t n1, uint8_t n2) {
    __CFInfoType info = atomic_load(&(((CFRuntimeBase *)cf)->_cfinfoa));
    return (info & __CFInfoMask(n1, n2)) >> n2;
}

/// Get a flag from a CFTypeRef info bitfield.
///
/// Bits are numbered from 7 on left to 0 on right.
static inline Boolean __CFRuntimeGetFlag(CFTypeRef cf, uint8_t n) {
    return __CFRuntimeGetValue(cf, n, n) == 1;
}

/// Set a value in a CFTypeRef info bitfield.
///
/// Bits are numbered from 6 on left to 0 on right. n1 and n2 specify an inclusive range n1..n2 with n1 >= n2.
/// For example:
///  n1 == 6, n2 == 4 will result in using the mask 0x0070. The value must fit inside 3 bits (6 - 4 + 1).
///  n1 == 0, n2 == 0 will result in using the mask 0x0001. The value must be 1 bit (0 - 0 + 1).
static inline void __CFRuntimeSetValue(CFTypeRef cf, uint8_t n1, uint8_t n2, uint8_t x) {
    __CFInfoType info = atomic_load(&(((CFRuntimeBase *)cf)->_cfinfoa));
    __CFInfoType newInfo;
    __CFInfoType mask = __CFInfoMask(n1, n2);
	
    #if !TARGET_OS_WASI
    do {
    #endif
        // maybe don't need to do the negation part because the right side promises that we are not going to touch the rest of the word
        newInfo = (info & ~mask) | ((x << n2) & mask);
    // Atomics are not supported on WASI, see https://bugs.swift.org/browse/SR-12097 for more details	
    #if !TARGET_OS_WASI
    } while (!atomic_compare_exchange_weak(&(((CFRuntimeBase *)cf)->_cfinfoa), &info, newInfo));
    #else
    ((CFRuntimeBase *)cf)->_cfinfoa = newInfo;
    #endif
}

/// Set a flag in a CFTypeRef info bitfield.
///
/// Bits are numbered from 7 on left to 0 on right.
static inline void __CFRuntimeSetFlag(CFTypeRef cf, uint8_t n, Boolean flag) {
    __CFRuntimeSetValue(cf, n, n, flag ? 1 : 0);
}

CF_PRIVATE Boolean __CFRuntimeIsConstant(CFTypeRef cf);
CF_PRIVATE void __CFRuntimeSetRC(CFTypeRef cf, uint32_t rc);

#if DEPLOYMENT_RUNTIME_SWIFT
#define _CFRUNTIME_BASE_INIT_SWIFT_RETAIN_COUNT ._swift_rc = _CF_CONSTANT_OBJECT_STRONG_RC
#else
#define _CFRUNTIME_BASE_INIT_SWIFT_RETAIN_COUNT
#endif

// A note on these macros.
// On systems where we have ObjC support (DEPLOYMENT_RUNTIME_OBJC), STATIC_CLASS_REF(…) can statically produce a reference to the ObjC class symbol that ties into this particular type.
// When compiling for Swift Foundation, STATIC_CLASS_REF returns a Swift class. There's a mapping of ObjC name classes to Swift symbols in the header that defines it that should be kept up to date if more constant objects are defined.
// On all other platforms, it returns NULL, which is okay; we only need the type ID if CF is to be used by itself.
#if TARGET_RT_64_BIT
    #define INIT_CFRUNTIME_BASE_WITH_CLASS(CLASS, TYPEID) {  ._cfisa = (uintptr_t)STATIC_CLASS_REF(CLASS) , ._cfinfoa = 0x0000000000000080ULL | ((TYPEID) << 8), _CFRUNTIME_BASE_INIT_SWIFT_RETAIN_COUNT }
    #define INIT_CFRUNTIME_BASE_WITH_CLASS_AND_FLAGS(CLASS, TYPEID, FLAGS) {  ._cfisa = (uintptr_t)STATIC_CLASS_REF(CLASS) , ._cfinfoa = 0x0000000000000080ULL | ((TYPEID) << 8) | (FLAGS), _CFRUNTIME_BASE_INIT_SWIFT_RETAIN_COUNT }
#else // if !TARGET_RT_64_BIT
    #define INIT_CFRUNTIME_BASE_WITH_CLASS(CLASS, TYPEID) { ._cfisa = (uintptr_t)STATIC_CLASS_REF(CLASS) , ._cfinfoa = 0x00000080UL | ((TYPEID) << 8), _CFRUNTIME_BASE_INIT_SWIFT_RETAIN_COUNT }
    #define INIT_CFRUNTIME_BASE_WITH_CLASS_AND_FLAGS(CLASS, TYPEID, FLAGS) {  ._cfisa = (uintptr_t)STATIC_CLASS_REF(CLASS), ._cfinfoa = 0x0000000000000080ULL | ((TYPEID) << 8) | (FLAGS), _CFRUNTIME_BASE_INIT_SWIFT_RETAIN_COUNT }
#endif // TARGET_RT_64_BIT

#define __CFBitIsSet(V, N)  (((V) & (1UL << (N))) != 0)
#define __CFBitSet(V, N)  ((V) |= (1UL << (N)))
#define __CFBitClear(V, N)  ((V) &= ~(1UL << (N)))

// Foundation uses 20-40
// Foundation knows about the value of __CFTSDKeyAutoreleaseData1
enum {
	__CFTSDKeyAllocator = 1,
	__CFTSDKeyIsInCFLog = 2,
	__CFTSDKeyIsInNSCache = 3,
	__CFTSDKeyIsInGCDMainQ = 4,
	__CFTSDKeyICUConverter = 7,
	__CFTSDKeyCollatorLocale = 8,
	__CFTSDKeyCollatorUCollator = 9,
	__CFTSDKeyRunLoop = 10,
	__CFTSDKeyRunLoopCntr = 11,
        __CFTSDKeyMachMessageBoost = 12, // valid only in the context of a CFMachPort callout
        __CFTSDKeyMachMessageHasVoucher = 13,
        __CFTSDKeyWeakReferenceHandler = 14,
        __CFTSDKeyIsInPreferences = 15,
        __CFTSDKeyPendingPreferencesKVONotifications = 16,
	// autorelease pool stuff must be higher than run loop constants
	__CFTSDKeyAutoreleaseData2 = 61,
	__CFTSDKeyAutoreleaseData1 = 62,
	__CFTSDKeyExceptionData = 63,

};

CF_INLINE CFAllocatorRef __CFGetDefaultAllocator(void) {
    CFAllocatorRef allocator = (CFAllocatorRef)_CFGetTSD(__CFTSDKeyAllocator);
    if (NULL == allocator) {
	allocator = kCFAllocatorSystemDefault;
    }
    return allocator;
}


#if !defined(LLONG_MAX)
    #if defined(_I64_MAX)
	#define LLONG_MAX	_I64_MAX
    #else
	#warning Arbitrarily defining LLONG_MAX
       #define LLONG_MAX	(int64_t)9223372036854775807
    #endif
#endif /* !defined(LLONG_MAX) */

#if !defined(LLONG_MIN)
    #if defined(_I64_MIN)
	#define LLONG_MIN	_I64_MIN
    #else
	#warning Arbitrarily defining LLONG_MIN
	#define LLONG_MIN	(-LLONG_MAX - (int64_t)1)
    #endif
#endif /* !defined(LLONG_MIN) */

#if defined(__GNUC__) && !defined(__STRICT_ANSI__)
    #define __CFMin(A,B) ({__typeof__(A) __a = (A); __typeof__(B) __b = (B); __a < __b ? __a : __b; })
    #define __CFMax(A,B) ({__typeof__(A) __a = (A); __typeof__(B) __b = (B); __a < __b ? __b : __a; })
#else /* __GNUC__ */
    #define __CFMin(A,B) ((A) < (B) ? (A) : (B))
    #define __CFMax(A,B) ((A) > (B) ? (A) : (B))
#endif /* __GNUC__ */

/* Secret CFAllocator hint bits */
#define __kCFAllocatorTempMemory	0x2
#define __kCFAllocatorNoPointers	0x10
#define __kCFAllocatorDoNotRecordEvent	0x100

CF_EXPORT CFAllocatorRef _CFTemporaryMemoryAllocator(void);

extern uint64_t __CFTimeIntervalToTSR(CFTimeInterval ti);
extern CFTimeInterval __CFTSRToTimeInterval(uint64_t tsr);
// use this instead of attempting to subtract mach_absolute_time() directly, because that can underflow and give an unexpected answer
CF_PRIVATE CFTimeInterval __CFTimeIntervalUntilTSR(uint64_t tsr);
#if __HAS_DISPATCH__
CF_PRIVATE dispatch_time_t __CFTSRToDispatchTime(uint64_t tsr);
#endif
CF_PRIVATE uint64_t __CFTSRToNanoseconds(uint64_t tsr);

extern CFStringRef __CFCopyFormattingDescription(CFTypeRef cf, CFDictionaryRef formatOptions);

/* Enhanced string formatting support
 */
CF_PRIVATE CFStringRef _CFStringCopyWithFomatStringConfiguration(CFStringRef aFormatString, CFDictionaryRef formatConfiguration);
CF_PRIVATE CFStringRef _CFCopyResolvedFormatStringWithConfiguration(CFTypeRef anObject, CFDictionaryRef aConfiguration, CFDictionaryRef formatOptions);
CF_PRIVATE CFStringRef _CFStringCreateWithWidthContexts(CFDictionaryRef widthContexts);
CF_PRIVATE CFStringRef _CFStringCreateWithMarkdownAndConfiguration(CFStringRef stringWithMarkup, CFDictionaryRef configuration, CFURLRef tableURL);
CF_PRIVATE Boolean _CFStringObjCFormatRequiresInflection(CFStringRef format);
CF_PRIVATE CFStringRef _CFStringCreateFormatWithInflectionAndArguments(CFAllocatorRef alloc, CFDictionaryRef formatOptions, CFStringRef format, va_list arguments);

/* result is long long or int, depending on doLonglong
*/
extern Boolean __CFStringScanInteger(CFStringInlineBuffer *buf, CFTypeRef locale, SInt32 *indexPtr, Boolean doLonglong, void *result);
extern Boolean __CFStringScanDouble(CFStringInlineBuffer *buf, CFTypeRef locale, SInt32 *indexPtr, double *resultPtr); 
extern Boolean __CFStringScanHex(CFStringInlineBuffer *buf, SInt32 *indexPtr, unsigned *result);

extern const char *__CFgetenv(const char *n);
extern const char *__CFgetenvIfNotRestricted(const char *n);    // Returns NULL in a restricted process

CF_PRIVATE Boolean __CFProcessIsRestricted(void);

// This is really about the availability of C99. We don't have that on Windows, but we should everywhere else.
#if TARGET_OS_WIN32
#define STACK_BUFFER_DECL(T, N, C) T *N = (T *)_alloca((C) * sizeof(T))
#else
#define STACK_BUFFER_DECL(T, N, C) T N[C]
#endif


#if TARGET_OS_WIN32
#define SAFE_STACK_BUFFER_DECL(Type, Name, Count, Max) Type *Name; BOOL __ ## Name ## WasMallocd = NO; if (sizeof(Type) * Count > Max) { Name = (Type *)malloc((Count) * sizeof(Type)); __ ## Name ## WasMallocd = YES; } else Name = (Count > 0) ? _alloca((Count) * sizeof(Type)) : NULL
#define SAFE_STACK_BUFFER_USE(Type, Name, Count, Max) if (sizeof(Type) * Count > Max) { Name = (Type *)malloc((Count) * sizeof(Type)); __ ## Name ## WasMallocd = YES; } else Name = (Count > 0) ? _alloca((Count) * sizeof(Type)) : NULL
#define SAFE_STACK_BUFFER_CLEANUP(Name) if (__ ## Name ## WasMallocd) free(Name)
#else
// Declare and allocate a stack buffer. Max is the max size (in bytes) before falling over to malloc.
#define SAFE_STACK_BUFFER_DECL(Type, Name, Count, Max) Type *Name; BOOL __ ## Name ## WasMallocd = NO; if (sizeof(Type) * Count > Max) { Name = (Type *)malloc((Count) * sizeof(Type)); __ ## Name ## WasMallocd = YES; } else Name = (Count > 0) ? alloca((Count) * sizeof(Type)) : NULL

// Allocate a pre-named stack buffer. Max is the max size (in bytes) before falling over to malloc.
#define SAFE_STACK_BUFFER_DEFINE(Type, Name) Type *Name = NULL; BOOL __ ## Name ## WasMallocd = NO;
#define SAFE_STACK_BUFFER_USE(Type, Name, Count, Max) if (sizeof(Type) * Count > Max) { Name = (Type *)malloc((Count) * sizeof(Type)); __ ## Name ## WasMallocd = YES; } else Name = (Count > 0) ? alloca((Count) * sizeof(Type)) : NULL

// Be sure to call this before your SAFE_STACK_BUFFER exits scope.
#define SAFE_STACK_BUFFER_CLEANUP(Name) if (__ ## Name ## WasMallocd) free(Name)
#endif // !TARGET_OS_WIN32


CF_EXPORT void * __CFConstantStringClassReferencePtr;

#if DEPLOYMENT_RUNTIME_SWIFT && TARGET_OS_MAC

#if TARGET_OS_LINUX
#define CONST_STRING_SECTION __attribute__((section(".cfstr.data")))
#else
#define CONST_STRING_SECTION
#endif

#if __BIG_ENDIAN__
#define _CF_CONST_STR_CFINFOA 0x00000000C8070000
#else // Little endian:
#define _CF_CONST_STR_CFINFOA 0x07C8
#endif // __BIG_ENDIAN__

#define _CF_CONST_STR_CONTENTS(cStr) {{(uintptr_t)&_CF_CONSTANT_STRING_SWIFT_CLASS, _CF_CONSTANT_OBJECT_STRONG_RC, _CF_CONST_STR_CFINFOA}, (uint8_t *)(cStr), sizeof(cStr) - 1}

#define CONST_STRING_DECL(S, V) \
_CF_CONSTANT_OBJECT_BACKING struct __CFConstStr __##S CONST_STRING_SECTION = _CF_CONST_STR_CONTENTS(V); \
const CFStringRef S = (CFStringRef)&__##S;

#define STATIC_CONST_STRING_DECL(S, V) \
static _CF_CONSTANT_OBJECT_BACKING struct __CFConstStr __##S CONST_STRING_SECTION = _CF_CONST_STR_CONTENTS(V); \
static const CFStringRef S = (CFStringRef)&__##S;

#define PE_CONST_STRING_DECL(S, V) \
static _CF_CONSTANT_OBJECT_BACKING struct __CFConstStr __##S CONST_STRING_SECTION = _CF_CONST_STR_CONTENTS(V); \
CF_PRIVATE const CFStringRef S = (CFStringRef)&__##S;

#elif defined(__CONSTANT_CFSTRINGS__)

#define CONST_STRING_DECL(S, V) const CFStringRef S = (const CFStringRef)__builtin___CFStringMakeConstantString(V);
#define STATIC_CONST_STRING_DECL(S, V) static const CFStringRef S = (const CFStringRef)__builtin___CFStringMakeConstantString(V);
#define PE_CONST_STRING_DECL(S, V) CF_PRIVATE const CFStringRef S = (const CFStringRef)__builtin___CFStringMakeConstantString(V);

#else

struct CF_CONST_STRING {
    CFRuntimeBase _base;
    uint8_t *_ptr;
    uint32_t _length;
};

CF_EXPORT int __CFConstantStringClassReference[];

/* CFNetwork also has a copy of the CONST_STRING_DECL macro (for use on platforms without constant string support in cc); please warn cfnetwork-core@group.apple.com of any necessary changes to this macro. -- REW, 1/28/2002 */

#define CONST_STRING_DECL(S, V)			\
static struct CF_CONST_STRING __ ## S ## __ = {{(uintptr_t)&__CFConstantStringClassReference, 0x000007c8U}, (uint8_t *)V, sizeof(V) - 1}; \
const CFStringRef S = (CFStringRef) & __ ## S ## __;
#define PE_CONST_STRING_DECL(S, V)			\
static struct CF_CONST_STRING __ ## S ## __ = {{(uintptr_t)&__CFConstantStringClassReference, 0x000007c8U}, (uint8_t *)V, sizeof(V) - 1}; \
CF_PRIVATE const CFStringRef S = (CFStringRef) & __ ## S ## __;

#endif // __CONSTANT_CFSTRINGS__

CF_EXPORT bool __CFOASafe;
CF_EXPORT void __CFSetLastAllocationEventName(void *ptr, const char *classname);



/* Comparators are passed the address of the values; this is somewhat different than CFComparatorFunction is used in public API usually. */
CF_EXPORT CFIndex	CFBSearch(const void *element, CFIndex elementSize, const void *list, CFIndex count, CFComparatorFunction comparator, void *context);

CF_EXPORT CFStringEncoding CFStringFileSystemEncoding(void);

CF_PRIVATE CFStringRef __CFStringCreateImmutableFunnel3(CFAllocatorRef alloc, const void *bytes, CFIndex numBytes, CFStringEncoding encoding, Boolean possiblyExternalFormat, Boolean tryToReduceUnicode, Boolean hasLengthByte, Boolean hasNullByte, Boolean noCopy, CFAllocatorRef contentsDeallocator, UInt32 converterFlags);

extern const void *__CFStringCollectionCopy(CFAllocatorRef allocator, const void *ptr);
extern const void *__CFTypeCollectionRetain(CFAllocatorRef allocator, const void *ptr);
extern void __CFTypeCollectionRelease(CFAllocatorRef allocator, const void *ptr);

extern CFTypeRef CFMakeUncollectable(CFTypeRef cf);

__attribute__((cold))
CF_PRIVATE void _CFRaiseMemoryException(CFStringRef reason);

CF_PRIVATE Boolean __CFProphylacticAutofsAccess;

#if __OBJC2__
CF_EXPORT id const __NSDictionary0__;
CF_EXPORT id const __NSArray0__;
#else
CF_EXPORT id __NSDictionary0__;
CF_EXPORT id __NSArray0__;
#endif

#include <CoreFoundation/CFLocking.h>

#if _POSIX_THREADS
typedef pthread_mutex_t _CFMutex;
#define _CF_MUTEX_STATIC_INITIALIZER PTHREAD_MUTEX_INITIALIZER
CF_INLINE int _CFMutexCreate(_CFMutex *lock) {
  return pthread_mutex_init(lock, NULL);
}
CF_INLINE int _CFMutexDestroy(_CFMutex *lock) {
  return pthread_mutex_destroy(lock);
}
CF_INLINE int _CFMutexLock(_CFMutex *lock) {
  return pthread_mutex_lock(lock);
}
CF_INLINE int _CFMutexUnlock(_CFMutex *lock) {
  return pthread_mutex_unlock(lock);
}

typedef pthread_mutex_t _CFRecursiveMutex;
CF_INLINE int _CFRecursiveMutexCreate(_CFRecursiveMutex *mutex) {
  pthread_mutexattr_t attr;
  pthread_mutexattr_init(&attr);
  pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

  int result = pthread_mutex_init(mutex, &attr);

  pthread_mutexattr_destroy(&attr);

  return result;
}
CF_INLINE int _CFRecursiveMutexDestroy(_CFRecursiveMutex *mutex) {
  return pthread_mutex_destroy(mutex);
}
CF_INLINE int _CFRecursiveMutexLock(_CFRecursiveMutex *mutex) {
  return pthread_mutex_lock(mutex);
}
CF_INLINE int _CFRecursiveMutexUnlock(_CFRecursiveMutex *mutex) {
  return pthread_mutex_unlock(mutex);
}
#elif defined(_WIN32)
typedef SRWLOCK _CFMutex;
#define _CF_MUTEX_STATIC_INITIALIZER SRWLOCK_INIT
CF_INLINE int _CFMutexCreate(_CFMutex *lock) {
  InitializeSRWLock(lock);
  return 0;
}
CF_INLINE int _CFMutexDestroy(_CFMutex *lock) {
  (void)lock;
  return 0;
}
CF_INLINE int _CFMutexLock(_CFMutex *lock) {
  AcquireSRWLockExclusive(lock);
  return 0;
}
CF_INLINE int _CFMutexUnlock(_CFMutex *lock) {
  ReleaseSRWLockExclusive(lock);
  return 0;
}

typedef CRITICAL_SECTION _CFRecursiveMutex;
CF_INLINE int _CFRecursiveMutexCreate(_CFRecursiveMutex *mutex) {
  InitializeCriticalSection(mutex);
  return 0;
}
CF_INLINE int _CFRecursiveMutexDestroy(_CFRecursiveMutex *mutex) {
  DeleteCriticalSection(mutex);
  return 0;
}
CF_INLINE int _CFRecursiveMutexLock(_CFRecursiveMutex *mutex) {
  EnterCriticalSection(mutex);
  return 0;
}
CF_INLINE int _CFRecursiveMutexUnlock(_CFRecursiveMutex *mutex) {
  LeaveCriticalSection(mutex);
  return 0;
}
#else
#error "do not know how to define mutex and recursive mutex for this OS"
#endif

#if __has_include(<os/lock.h>)
#include <os/lock.h>
#if __has_include(<os/lock_private.h>)
#include <os/lock_private.h>
#define _CF_HAS_OS_UNFAIR_RECURSIVE_LOCK 1
#else
#define os_unfair_lock_lock_with_options(lock, options) os_unfair_lock_lock(lock)
#define OS_UNFAIR_LOCK_DATA_SYNCHRONIZATION (0)
#endif

#elif _POSIX_THREADS
#define OS_UNFAIR_LOCK_INIT PTHREAD_MUTEX_INITIALIZER
typedef pthread_mutex_t os_unfair_lock;
typedef pthread_mutex_t * os_unfair_lock_t;
typedef uint32_t os_unfair_lock_options_t;
#define OS_UNFAIR_LOCK_DATA_SYNCHRONIZATION (0)
static void os_unfair_lock_lock(os_unfair_lock_t lock) { pthread_mutex_lock(lock); }
static void os_unfair_lock_lock_with_options(os_unfair_lock_t lock, os_unfair_lock_options_t options) { pthread_mutex_lock(lock); }
static void os_unfair_lock_unlock(os_unfair_lock_t lock) { pthread_mutex_unlock(lock); }
#elif defined(_WIN32)
#define OS_UNFAIR_LOCK_INIT CFLockInit
#define os_unfair_lock CFLock_t
#define os_unfair_lock_lock __CFLock
#define os_unfair_lock_unlock __CFUnlock
#define os_unfair_lock_lock_with_options(lock, options) __CFLock(lock)
#define OS_UNFAIR_LOCK_DATA_SYNCHRONIZATION
#endif // __has_include(<os/lock.h>)

#if defined(_CF_HAS_OS_UNFAIR_RECURSIVE_LOCK)
#undef _CF_HAS_OS_UNFAIR_RECURSIVE_LOCK // Nothing to do here.
#define _CFPerformDynamicInitOfOSRecursiveLock(lock) do {} while (0)
#else
#define os_unfair_recursive_lock _CFRecursiveMutex
#define OS_UNFAIR_RECURSIVE_LOCK_INIT { 0 }
#define _CFPerformDynamicInitOfOSRecursiveLock _CFRecursiveMutexCreate
#define os_unfair_recursive_lock_lock _CFRecursiveMutexLock
#define os_unfair_recursive_lock_lock_with_options(lock, more) _CFRecursiveMutexLock(lock)
#define os_unfair_recursive_lock_unlock _CFRecursiveMutexUnlock
#endif


#if !__HAS_DISPATCH__

typedef volatile long dispatch_once_t;
CF_PRIVATE void _CF_dispatch_once(dispatch_once_t *, void (^)(void));
#define dispatch_once _CF_dispatch_once

#endif

#if TARGET_OS_MAC
#define __CF_FORK_STATE_FORKED_FLAG         (1 << 0)
#define __CF_FORK_STATE_CF_USED_FLAG        (1 << 1)
#define __CF_FORK_STATE_MULTITHREADED_FLAG  (1 << 2)
CF_PRIVATE _Atomic(uint8_t) __CF_FORK_STATE;
extern void __THE_PROCESS_HAS_FORKED_AND_YOU_CANNOT_USE_THIS_COREFOUNDATION_FUNCTIONALITY___YOU_MUST_EXEC__(void);
extern void __CF_USED(void);
#define CHECK_FOR_FORK() do { \
    /* Write __CF_FORK_STATE_CF_USED_FLAG only once, avoiding a memory barrier for subsequent reads. */ \
    if (0 == (atomic_load_explicit(&__CF_FORK_STATE, memory_order_relaxed) & __CF_FORK_STATE_CF_USED_FLAG)) { \
        __CF_USED(); \
    } \
    if (atomic_load_explicit(&__CF_FORK_STATE, memory_order_relaxed) & __CF_FORK_STATE_FORKED_FLAG) { \
        __THE_PROCESS_HAS_FORKED_AND_YOU_CANNOT_USE_THIS_COREFOUNDATION_FUNCTIONALITY___YOU_MUST_EXEC__(); \
    } \
} while (0)

#define HAS_FORKED() ({ CHECK_FOR_FORK(); (atomic_load_explicit(&__CF_FORK_STATE, memory_order_relaxed) & __CF_FORK_STATE_FORKED_FLAG) != 0;})
#define CHECK_FOR_FORK_RET(...) do { if (HAS_FORKED()) return __VA_ARGS__; } while (0)
#endif

#if !defined(CHECK_FOR_FORK)
#define CHECK_FOR_FORK() do { } while (0)
#endif

#if !defined(CHECK_FOR_FORK_RET)
#define CHECK_FOR_FORK_RET(...) do { } while (0)
#endif

#if !defined(HAS_FORKED)
#define HAS_FORKED() 0
#endif

#include <errno.h>

#define thread_errno() errno
#define thread_set_errno(V) do {errno = (V);} while (0)

extern void *__CFStartSimpleThread(void *func, void *arg);

/* ==================== Simple file access ==================== */
/* For dealing with abstract types.  MF:!!! These ought to be somewhere else and public. */
    
CF_EXPORT CFStringRef _CFCopyExtensionForAbstractType(CFStringRef abstractType);

/* ==================== Simple file access ==================== */
/* These functions all act on a c-strings which must be in the file system encoding. */
    
CF_PRIVATE Boolean _CFCreateDirectory(const char *path);
CF_PRIVATE Boolean _CFRemoveDirectory(const char *path);
CF_PRIVATE Boolean _CFDeleteFile(const char *path);

CF_PRIVATE CFDataRef _CFDataCreateFromURL(CFURLRef resourceURL, CFErrorRef *error);

CF_PRIVATE Boolean _CFReadBytesFromFile(CFAllocatorRef alloc, CFURLRef url, void **bytes, CFIndex *length, CFIndex maxLength, int extraOpenFlags);
    /* resulting bytes are allocated from alloc which MUST be non-NULL. */
    /* maxLength of zero means the whole file.  Otherwise it sets a limit on the number of bytes read. */

CF_EXPORT Boolean _CFWriteBytesToFile(CFURLRef url, const void *bytes, CFIndex length);

CF_PRIVATE CFMutableArrayRef _CFCreateContentsOfDirectory(CFAllocatorRef alloc, char *dirPath, void *dirSpec, CFURLRef dirURL, CFStringRef matchingAbstractType);
    /* On Mac OS 8/9, one of dirSpec, dirPath and dirURL must be non-NULL */
    /* On all other platforms, one of path and dirURL must be non-NULL */
    /* If both are present, they are assumed to be in-synch; that is, they both refer to the same directory.  */
    /* alloc may be NULL */
    /* return value is CFArray of CFURLs */

CF_PRIVATE SInt32 _CFGetPathProperties(CFAllocatorRef alloc, char *path, Boolean *exists, SInt32 *posixMode, SInt64 *size, CFDateRef *modTime, SInt32 *ownerID, CFArrayRef *dirContents);
    /* alloc may be NULL */
    /* any of exists, posixMode, size, modTime, and dirContents can be NULL.  Usually it is not a good idea to pass NULL for exists, since interpretting the other values sometimes requires that you know whether the file existed or not.  Except for dirContents, it is pretty cheap to compute any of these things as loing as one of them must be computed. */

CF_PRIVATE SInt32 _CFGetFileProperties(CFAllocatorRef alloc, CFURLRef pathURL, Boolean *exists, SInt32 *posixMode, SInt64 *size, CFDateRef *modTime, SInt32 *ownerID, CFArrayRef *dirContents);
    /* alloc may be NULL */
    /* any of exists, posixMode, size, modTime, and dirContents can be NULL.  Usually it is not a good idea to pass NULL for exists, since interpretting the other values sometimes requires that you know whether the file existed or not.  Except for dirContents, it is pretty cheap to compute any of these things as loing as one of them must be computed. */

CF_PRIVATE bool _CFURLExists(CFURLRef url);

/* ==================== Simple path manipulation ==================== */

CF_EXPORT UniChar _CFGetSlash(void);
CF_PRIVATE CFStringRef _CFGetSlashStr(void);
CF_EXPORT Boolean _CFIsAbsolutePath(UniChar *unichars, CFIndex length);
CF_PRIVATE void _CFAppendTrailingPathSlash2(CFMutableStringRef path);
CF_PRIVATE void _CFAppendConditionalTrailingPathSlash2(CFMutableStringRef path);
CF_EXPORT Boolean _CFAppendPathComponent(UniChar *unichars, CFIndex *length, CFIndex maxLength, UniChar *component, CFIndex componentLength);
CF_PRIVATE void _CFAppendPathComponent2(CFMutableStringRef path, CFStringRef component);
CF_PRIVATE Boolean _CFAppendPathExtension2(CFMutableStringRef path, CFStringRef extension);
CF_EXPORT Boolean _CFAppendPathExtension(UniChar *unichars, CFIndex *length, CFIndex maxLength, UniChar *extension, CFIndex extensionLength);
CF_EXPORT Boolean _CFTransmutePathSlashes(UniChar *unichars, CFIndex *length, UniChar replSlash);
CF_PRIVATE CFStringRef _CFCreateLastPathComponent(CFAllocatorRef alloc, CFStringRef path, CFIndex *slashIndex);
CF_EXPORT CFIndex _CFStartOfLastPathComponent(UniChar *unichars, CFIndex length);
CF_PRIVATE CFIndex _CFStartOfLastPathComponent2(CFStringRef path);
CF_EXPORT CFIndex _CFLengthAfterDeletingLastPathComponent(UniChar *unichars, CFIndex length);
CF_PRIVATE CFIndex _CFLengthAfterDeletingPathExtension2(CFStringRef path);
CF_EXPORT CFIndex _CFStartOfPathExtension(UniChar *unichars, CFIndex length);
CF_PRIVATE CFIndex _CFStartOfPathExtension2(CFStringRef path);
CF_EXPORT CFIndex _CFLengthAfterDeletingPathExtension(UniChar *unichars, CFIndex length);
CF_PRIVATE CFArrayRef _CFCreateCFArrayByTokenizingString(const char *values, char delimiter);

#if __BLOCKS__
#if TARGET_OS_WIN32
#define	DT_DIR		 4
#define	DT_REG		 8
#define DT_LNK          10
#define	DT_UNKNOWN	 0
#endif

/*
 Iterate the contents of a directory. If you want directories to have a / appended to their name, set appendSlashForDirectories. If you provide the stuffToPrefix argument, then this will prepend those to the file name in the fileNameWithPrefix block argument (and assume each is a directory).
 This function automatically skips '.' and '..', and '._' files.
*/
CF_PRIVATE void _CFIterateDirectory(CFStringRef directoryPath, Boolean appendSlashForDirectories, CFArrayRef stuffToPrefix, Boolean (^fileHandler)(CFStringRef fileName, CFStringRef fileNameWithPrefix, uint8_t fileType));
#endif

#define __CFMaxRuntimeTypes	65535
#define __CFRuntimeClassTableSize 1024

extern void _CFRuntimeSetInstanceTypeIDAndIsa(CFTypeRef cf, CFTypeID newTypeID);

#if DEPLOYMENT_RUNTIME_SWIFT
#define CF_IS_SWIFT(type, obj) (_CFIsSwift(type, (CFSwiftRef)obj))

#define CF_SWIFT_FUNCDISPATCHV_CHECK(check, type, ret, obj, fn, ...) do { \
    if (check(type, obj)) { \
        return (ret)__CFSwiftBridge.fn((CFSwiftRef)obj, ##__VA_ARGS__); \
    } \
} while (0)
#define CF_SWIFT_FUNCDISPATCHV(type, ret, obj, fn, ...) CF_SWIFT_FUNCDISPATCHV_CHECK(CF_IS_SWIFT, type, ret, obj, fn, ## __VA_ARGS__)

#define CF_SWIFT_CALLV(obj, fn, ...) __CFSwiftBridge.fn((CFSwiftRef)obj, ##__VA_ARGS__)
#else
#define CF_IS_SWIFT(type, obj) (0)
#define CF_SWIFT_FUNCDISPATCHV(type, ret, obj, fn, ...) do { } while (0)
#define CF_SWIFT_CALLV(obj, fn, ...) (0)
#endif

#ifndef __has_attribute
#define __has_attribute(...) 0
#endif

#if TARGET_OS_WIN32
#define _CF_VISIBILITY_HIDDEN_ATTRIBUTE
#elif __has_attribute(visibility)
#define _CF_VISIBILITY_HIDDEN_ATTRIBUTE __attribute__((visibility("hidden")))
#else
#define _CF_VISIBILITY_HIDDEN_ATTRIBUTE
#endif

typedef struct __CFClassTables {
    CFRuntimeClass const * classTable[__CFRuntimeClassTableSize];
    // This can be safely `_Atomic` because we just store the signed classes; you can't sign / auth _Atomic pointers
    _Atomic(uintptr_t) objCClassTable[__CFRuntimeClassTableSize];
} _CFClassTables;

// IMPORTANT: 'heap' and other memory tools look up this symbol by name. Even though it is not exported, the name is ABI. Changes must be coordinated with them.
CF_PRIVATE _CFClassTables __CFRuntimeClassTables;

#define __CFRuntimeClassTable __CFRuntimeClassTables.classTable
#define __CFRuntimeObjCClassTable __CFRuntimeClassTables.objCClassTable

#if __has_feature(ptrauth_intrinsics)
__attribute__((visibility("hidden")))
CF_INLINE uintptr_t ___CFRUNTIME_OBJC_CLASSTABLE_PTRAUTH_DISCRIMINATOR(void const * const tableSlotAddr) {
    return ptrauth_blend_discriminator(tableSlotAddr, ptrauth_string_discriminator("__CFRuntimeObjCClassTable"));
}
#endif

CF_INLINE uintptr_t _GetCFRuntimeObjcClassAtIndex(CFTypeID typeID) {
    uintptr_t obj = atomic_load_explicit(&__CFRuntimeObjCClassTable[typeID], memory_order_relaxed);

#if __has_feature(ptrauth_intrinsics)
    // Auth using a discriminator that uses the address of the slot
    // in __CFRuntimeObjCClassTable and a known string discriminator.
    void const * const slot = &__CFRuntimeObjCClassTable[typeID];
    return (uintptr_t)ptrauth_auth_data((void *)obj,
                                        ptrauth_key_process_dependent_data,
                                        ___CFRUNTIME_OBJC_CLASSTABLE_PTRAUTH_DISCRIMINATOR(slot));
#else
    return (uintptr_t)obj;
#endif
}

CF_INLINE void _SetCFRuntimeObjcClass(uintptr_t aClass, CFTypeID typeID) {
    uintptr_t classToStore = aClass;

#if __has_feature(ptrauth_intrinsics)
    // validate the current entry; ignore the return value we just want to ensure our table is in a valid state before mutation
    _GetCFRuntimeObjcClassAtIndex(typeID);
    // If we're using ptrauth, we'll sign using a discriminator that uses the address of the slot
    // in __CFRuntimeObjCClassTable and a known string discriminator.
    // Later we'll auth this using the the same discriminator to ensure the table hasn't been messed with
    // and that the class we've stored in the table is the one we expect it to be.
    void const * const slot = &__CFRuntimeObjCClassTable[typeID];
    classToStore = (uintptr_t)ptrauth_sign_unauthenticated((void *)classToStore,
                                                           ptrauth_key_process_dependent_data,
                                                           ___CFRUNTIME_OBJC_CLASSTABLE_PTRAUTH_DISCRIMINATOR(slot));
#endif

    atomic_store_explicit(&__CFRuntimeObjCClassTable[typeID], classToStore, memory_order_relaxed);
}

CF_INLINE uintptr_t __CFISAForTypeID(CFTypeID typeID) {
    if (typeID < __CFRuntimeClassTableSize) {
        // There is a "race" here between CFRetain / CFRelease (which call CF_IS_OBJC)
        // and _CFRuntimeBridgeClasses. Except... that because this array is
        // pointer-sized, the only possible races are on access to the same index in
        // both cases.
        // So we have two cases:
        // - if you call CF_IS_OBJC on a CF object, it means that type has been registered
        //   previously: no race
        // - if you call CF_IS_OBJC on an objc object, and __CFGenericTypeID_inline
        //   interpreted some bits of the object as a type ID, we don't really care
        //   if the value we read is outdated or not, since we will fail the isa comparison
        //   in CF_IS_OBJC

        return _GetCFRuntimeObjcClassAtIndex(typeID);
    } else {
        return 0;
    }
}

#define CF_OBJC_FUNCDISPATCHV(typeID, obj, ...) do { } while (0)
#define CF_OBJC_RETAINED_FUNCDISPATCHV(typeID, obj, ...) do { } while (0)
#define CF_OBJC_CALLV(obj, ...) (0)
#define CF_IS_OBJC(typeID, obj) (0)
#define _CFTypeGetClass(obj) ((uintptr_t)((CFRuntimeBase *)obj)->_cfisa)

/* See comments in CFBase.c
*/
#define FAULT_CALLBACK(V)
#define INVOKE_CALLBACK1(P, A) (P)(A)
#define INVOKE_CALLBACK2(P, A, B) (P)(A, B)
#define INVOKE_CALLBACK3(P, A, B, C) (P)(A, B, C)
#define INVOKE_CALLBACK4(P, A, B, C, D) (P)(A, B, C, D)
#define INVOKE_CALLBACK5(P, A, B, C, D, E) (P)(A, B, C, D, E)
#define UNFAULT_CALLBACK(V) do { } while (0)

/* For the support of functionality which needs CarbonCore or other frameworks */
// These macros define an upcall or weak "symbol-lookup" wrapper function.
// The parameters are:
//   R : the return type of the function
//   N : the name of the function (in the other library)
//   P : the parenthesized parameter list of the function
//   A : the parenthesized actual argument list to be passed
//  FAILACTION: (only for the _FAIL macros) additional code to be
//       run when the function cannot be found.
//  opt: a fifth optional argument can be passed in which is the
//       return value of the wrapper when the function cannot be
//       found; should be of type R, & can be a function call
// The name of the resulting wrapper function is:
//    __CFCarbonCore_N (where N is the second parameter)
//    __CFNetwork_N (where N is the second parameter)
//
// Example:
//   DEFINE_WEAK_CARBONCORE_FUNC(void, DisposeHandle, (Handle h), (h))
//

#if TARGET_OS_MAC

extern void *__CFLookupCFNetworkFunction(const char *name);

#define DEFINE_WEAK_CFNETWORK_FUNC_FAIL(R, N, P, A, FAILACTION, ...)        \
    static R __CFNetwork_ ## N P {                                          \
        typedef R (*dyfuncptr)P;                                            \
        static dyfuncptr dyfunc = (dyfuncptr)(~(uintptr_t)0);               \
        static dispatch_once_t onceToken;                                   \
        dispatch_once(&onceToken, ^{                                        \
            dyfunc = (dyfuncptr)__CFLookupCFNetworkFunction(#N);            \
        });                                                                 \
        if (dyfunc) {                                                       \
            return dyfunc A ;                                               \
        }                                                                   \
        FAILACTION ;                                                        \
        return __VA_ARGS__ ;                                                \
    }

#else
#define DEFINE_WEAK_CFNETWORK_FUNC_FAIL(R, N, P, A, ...)
#endif

#define DEFINE_WEAK_CARBONCORE_FUNC(R, N, P, A, ...)

#if TARGET_OS_MAC

extern void *__CFLookupCoreServicesInternalFunction(const char *name);

#define DEFINE_WEAK_CORESERVICESINTERNAL_FUNC(R, N, P, A, ...)              \
    static R __CFCoreServicesInternal_ ## N P {                             \
        typedef R (*dyfuncptr)P;                                            \
        static dyfuncptr dyfunc = (dyfuncptr)(~(uintptr_t)0);               \
        static dispatch_once_t onceToken;                                   \
        dispatch_once(&onceToken, ^{                                        \
            dyfunc = (dyfuncptr)__CFLookupCoreServicesInternalFunction(#N); \
        });                                                                 \
        if (dyfunc) {                                                       \
            return dyfunc A ;                                               \
        }                                                                   \
        return __VA_ARGS__ ;                                                \
    }

#else
#define DEFINE_WEAK_CORESERVICESINTERNAL_FUNC(R, N, P, A, ...)
#endif

CF_PRIVATE CFComparisonResult _CFCompareStringsWithLocale(CFStringInlineBuffer *str1, CFRange str1Range, CFStringInlineBuffer *str2, CFRange str2Range, CFOptionFlags options, const void *compareLocale);


CF_PRIVATE CFArrayRef _CFBundleCopyUserLanguages(void);


// This should only be used in CF types, not toll-free bridged objects!
// It should not be used with CFAllocator arguments!
// Use CFGetAllocator() in the general case, and this inline function in a few limited (but often called) situations.
CF_INLINE CFAllocatorRef __CFGetAllocator(CFTypeRef cf) {	// !!! Use with CF types only, and NOT WITH CFAllocator!
#if OBJC_HAVE_TAGGED_POINTERS
    if (_objc_isTaggedPointer(cf)) {
        return kCFAllocatorSystemDefault;
    }
#endif
    if (__builtin_expect(__CFRuntimeGetFlag(cf, 7), true)) {
	return kCFAllocatorSystemDefault;
    }
    // To preserve 16 byte alignment when using custom allocators, we always place the CFAllocatorRef 16 bytes before the CFType
    return *(CFAllocatorRef *)((char *)cf - 16);
}

/* !!! Avoid #importing objc.h; e.g. converting this to a .m file */
struct __objcFastEnumerationStateEquivalent {
    unsigned long state;
    unsigned long *itemsPtr;
    unsigned long *mutationsPtr;
    unsigned long extra[5];
};

CF_PRIVATE CFSetRef __CFBinaryPlistCopyTopLevelKeys(CFAllocatorRef allocator, const uint8_t *databytes, uint64_t datalen, uint64_t startOffset, const CFBinaryPlistTrailer *trailer);
CF_PRIVATE bool __CFBinaryPlistIsDictionary(const uint8_t *databytes, uint64_t datalen, uint64_t startOffset, const CFBinaryPlistTrailer *trailer);
CF_PRIVATE bool __CFBinaryPlistIsArray(const uint8_t *databytes, uint64_t datalen, uint64_t startOffset, const CFBinaryPlistTrailer *trailer);

#if 0
#pragma mark -
#pragma mark Windows Compatibility
#endif

// Need to use the _O_BINARY flag on Windows to get the correct behavior
#if TARGET_OS_WIN32
#define CF_OPENFLGS	(_O_BINARY|_O_NOINHERIT)
#else
#define CF_OPENFLGS	(0)
#endif

#if TARGET_OS_WIN32

// These are replacements for pthread calls on Windows
CF_EXPORT int _NS_pthread_main_np();
CF_EXPORT int _NS_pthread_setspecific(_CFThreadSpecificKey key, const void *val);
CF_EXPORT void* _NS_pthread_getspecific(_CFThreadSpecificKey key);
CF_EXPORT int _NS_pthread_key_init_np(int key, void (*destructor)(void *));
CF_EXPORT void _NS_pthread_setname_np(const char *name);
CF_EXPORT bool _NS_pthread_equal(_CFThreadRef t1, _CFThreadRef t2);

// map use of pthread_set/getspecific to internal API
#define pthread_setspecific _NS_pthread_setspecific
#define pthread_getspecific _NS_pthread_getspecific
#define pthread_key_init_np _NS_pthread_key_init_np
#define pthread_main_np _NS_pthread_main_np
#define pthread_setname_np _NS_pthread_setname_np
#define pthread_equal _NS_pthread_equal

#define pthread_self() GetCurrentThread()

#endif

#if TARGET_OS_LINUX
#define pthread_main_np _CFIsMainThread
#endif

#if TARGET_OS_WIN32
CF_PRIVATE const wchar_t *_CFDLLPath(void);
#endif

/* Buffer size for file pathname */
#if TARGET_OS_WIN32
/// Use this constant for the size (in characters) of a buffer in which to hold a path. This size adds space for at least a couple of null terminators at the end of a buffer into which you copy up to kCFMaxPathLength characters.
#define CFMaxPathSize ((CFIndex)262)
/// Use this constant for the maximum length (in characters) of a path you want to copy into a buffer. This should be the maximum number of characters before the null terminator(s).
#define CFMaxPathLength ((CFIndex)260)
#define PATH_SEP '\\'
#define PATH_SEP_STR CFSTR("\\")
#define PATH_MAX MAX_PATH
#else
/// Use this constant for the size (in characters) of a buffer in which to hold a path. This size adds space for at least a couple of null terminators at the end of a buffer into which you copy up to kCFMaxPathLength characters.
#define CFMaxPathSize ((CFIndex)1026)
/// Use this constant for the maximum length (in characters) of a path you want to copy into a buffer. This should be the maximum number of characters before the null terminator(s).
#define CFMaxPathLength ((CFIndex)1024)
#define PATH_SEP '/'
#define PATH_SEP_STR CFSTR("/")
#endif

CF_INLINE const char *CFPathRelativeToAppleFrameworksRoot(const char *path, Boolean *allocated) {
    if (!__CFProcessIsRestricted() && path) {
        const char *platformRoot = __CFgetenv("APPLE_FRAMEWORKS_ROOT");
        if (platformRoot) {
            char *newPath = NULL;
            asprintf(&newPath, "%s%s", platformRoot, path);
            if (allocated && newPath) {
                *allocated = true;
            }
            return newPath;
        }
    }
    if (allocated) {
        *allocated = false;
    }
    return path;
}

#if __HAS_DISPATCH__

#include <dispatch/dispatch.h>
#if __has_include(<dispatch/private.h>)
#include <dispatch/private.h>
#else
enum {
    DISPATCH_QUEUE_OVERCOMMIT = 0x2ull,
};
#endif

#if TARGET_OS_LINUX || TARGET_OS_WIN32 || TARGET_OS_BSD
#define QOS_CLASS_USER_INITIATED DISPATCH_QUEUE_PRIORITY_HIGH
#define QOS_CLASS_DEFAULT DISPATCH_QUEUE_PRIORITY_DEFAULT
#define QOS_CLASS_UTILITY DISPATCH_QUEUE_PRIORITY_LOW
#define QOS_CLASS_BACKGROUND DISPATCH_QUEUE_PRIORITY_BACKGROUND

CF_INLINE long qos_class_main() {
    return QOS_CLASS_USER_INITIATED;
}

CF_INLINE long qos_class_self() {
    return QOS_CLASS_DEFAULT;
}

#endif

// Returns a generic dispatch queue for when you want to just throw some work
// into the concurrent pile to execute, and don't care about specifics except
// to match the QOS of the main thread.
CF_INLINE dispatch_queue_t __CFDispatchQueueGetGenericMatchingMain(void) {
    return dispatch_get_global_queue(qos_class_main(), DISPATCH_QUEUE_OVERCOMMIT);
}

// Returns a generic dispatch queue for when you want to just throw some work
// into the concurrent pile to execute, and don't care about specifics except
// to match the QOS of the current thread.
CF_INLINE dispatch_queue_t __CFDispatchQueueGetGenericMatchingCurrent(void) {
    return dispatch_get_global_queue(qos_class_self(), 0); // DISPATCH_QUEUE_OVERCOMMIT left out intentionally at this point
}

// Returns a generic dispatch queue for when you want to just throw some work
// into the concurrent pile to execute, and don't care about specifics except
// that it should be in background QOS.
CF_INLINE dispatch_queue_t __CFDispatchQueueGetGenericBackground(void) {
    // Don't ACTUALLY use BACKGROUND, because of unknowable and unfavorable interactions like (<rdar://problem/16319229>)
    return dispatch_get_global_queue(QOS_CLASS_UTILITY, DISPATCH_QUEUE_OVERCOMMIT);
}

#endif

CF_PRIVATE CFStringRef _CFStringCopyBundleUnloadingProtectedString(CFStringRef str);

CF_PRIVATE uint8_t *_CFDataGetBytePtrNonObjC(CFDataRef data);
CF_PRIVATE dispatch_data_t _CFDataCreateDispatchData(CFDataRef data); //avoids copying in most cases

// Use this for functions that are intended to be breakpoint hooks. If you do not, the compiler may optimize them away.
// Based on: BREAKPOINT_FUNCTION in objc-os.h
// Example:
//   CF_BREAKPOINT_FUNCTION( void stop_on_error(void) ); */
#define CF_BREAKPOINT_FUNCTION(prototype) \
    extern __attribute__((noinline, used, visibility("hidden"))) \
    prototype { __asm(""); }

#define __CF_QUEUE_NAME(a) "com.apple." a

#pragma mark -
#pragma mark CF Instruments SPI

#if TARGET_OS_MAC
extern void __CFRecordAllocationEvent(int eventnum, void *ptr, int64_t size, uint64_t data, const char *classname);
#else
#define __CFRecordAllocationEvent(a, b, c, d, e) ((void)0)
#define __CFSetLastAllocationEventName(a, b) ((void)0)
#endif

enum {
    __kCFZombieMessagedEvent = 21,
};

#define _CFReleaseDeferred __attribute__((__cleanup__(_CFReleaseOnCleanup)))
static inline void _CFReleaseOnCleanup(void * CF_RELEASES_ARGUMENT ptr) {
    CFTypeRef cf = *(CFTypeRef *)ptr;
    if (cf) CFRelease(cf);
}

#pragma mark - CF Private Globals

CF_PRIVATE void *__CFAppleLanguages;
CF_PRIVATE uint8_t __CFZombieEnabled;
CF_PRIVATE uint8_t __CFDeallocateZombies;
CF_PRIVATE Boolean __CFInitialized;
CF_PRIVATE _Atomic(bool) __CFMainThreadHasExited;
CF_PRIVATE const CFStringRef __kCFLocaleCollatorID;

#if __OBJC__
#import <Foundation/NSArray.h>
@interface NSArray (CFBufferAdoption)
- (instancetype)_initByAdoptingBuffer:(id *)buffer count:(NSUInteger)count size:(size_t)size;
@end
#endif

CF_EXTERN_C_END


// Load 16,32,64 bit values from unaligned memory addresses. These need to be done bytewise otherwise
// it is undefined behaviour in C. On some architectures, eg x86, unaligned loads are allowed by the
// processor and the compiler will convert these byte accesses into the appropiate DWORD/QWORD memory
// access.

CF_INLINE uint32_t _CFUnalignedLoad32(const void *ptr) {
    uint8_t *bytes = (uint8_t *)ptr;
#if __LITTLE_ENDIAN__
    uint32_t result = (uint32_t)bytes[0];
    result |= ((uint32_t)bytes[1] << 8);
    result |= ((uint32_t)bytes[2] << 16);
    result |= ((uint32_t)bytes[3] << 24);
#else
    uint32_t result = (uint32_t)bytes[0] << 24;
    result |= ((uint32_t)bytes[1] << 16);
    result |= ((uint32_t)bytes[2] << 8);
    result |= (uint32_t)bytes[3];
#endif
    return result;
}


CF_INLINE void _CFUnalignedStore32(void *ptr, uint32_t value) {
    uint8_t *bytes = (uint8_t *)ptr;
#if __LITTLE_ENDIAN__
    bytes[0] = (uint8_t)(value & 0xff);
    bytes[1] = (uint8_t)((value >>  8) & 0xff);
    bytes[2] = (uint8_t)((value >> 16) & 0xff);
    bytes[3] = (uint8_t)((value >> 24) & 0xff);
#else
    bytes[0] = (uint8_t)((value >> 24) & 0xff);
    bytes[1] = (uint8_t)((value >> 16) & 0xff);
    bytes[2] = (uint8_t)((value >>  8) & 0xff);
    bytes[3] = (uint8_t)((value >>  0) & 0xff);
#endif
}


// Load values stored in Big Endian order in memory.
CF_INLINE uint16_t _CFUnalignedLoad16BE(const void *ptr) {
    uint8_t *bytes = (uint8_t *)ptr;
    uint16_t result = (uint16_t)bytes[0] << 8;
    result |= (uint16_t)bytes[1];

    return result;
}


CF_INLINE uint32_t _CFUnalignedLoad32BE(const void *ptr) {
    uint8_t *bytes = (uint8_t *)ptr;
    uint32_t result = (uint32_t)bytes[0] << 24;
    result |= ((uint32_t)bytes[1] << 16);
    result |= ((uint32_t)bytes[2] << 8);
    result |= (uint32_t)bytes[3];

    return result;
}


CF_INLINE uint64_t _CFUnalignedLoad64BE(const void *ptr) {
    uint8_t *bytes = (uint8_t *)ptr;
    uint64_t result = (uint64_t)bytes[0] << 56;
    result |= ((uint64_t)bytes[1] << 48);
    result |= ((uint64_t)bytes[2] << 40);
    result |= ((uint64_t)bytes[3] << 32);
    result |= ((uint64_t)bytes[4] << 24);
    result |= ((uint64_t)bytes[5] << 16);
    result |= ((uint64_t)bytes[6] << 8);
    result |= (uint64_t)bytes[7];

    return result;
}

#endif /* ! __COREFOUNDATION_CFINTERNAL__ */

