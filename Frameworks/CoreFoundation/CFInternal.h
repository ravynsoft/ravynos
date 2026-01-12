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

/*	CFInternal.h
	Copyright (c) 1998-2014, Apple Inc. All rights reserved.
*/

/*
        NOT TO BE USED OUTSIDE CF!
*/

#if !CF_BUILDING_CF
    #error The header file CFInternal.h is for the exclusive use of CoreFoundation. No other project should include it.
#endif

#if !defined(__COREFOUNDATION_CFINTERNAL__)
#define __COREFOUNDATION_CFINTERNAL__ 1


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


CF_EXTERN_C_BEGIN

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFURL.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFDate.h>
#include <CoreFoundation/CFArray.h>
#include <CoreFoundation/CFLogUtilities.h>
#include <CoreFoundation/CFRuntime.h>
#include <limits.h>
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI || DEPLOYMENT_TARGET_LINUX
#include <xlocale.h>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>
#include <stdio.h>
#endif
#include <pthread.h>


#if defined(__BIG_ENDIAN__)
#define __CF_BIG_ENDIAN__ 1
#define __CF_LITTLE_ENDIAN__ 0
#endif

#if defined(__LITTLE_ENDIAN__)
#define __CF_LITTLE_ENDIAN__ 1
#define __CF_BIG_ENDIAN__ 0
#endif


#include <CoreFoundation/ForFoundationOnly.h>

CF_EXPORT const char *_CFProcessName(void);
CF_EXPORT CFStringRef _CFProcessNameString(void);

CF_EXPORT Boolean _CFGetCurrentDirectory(char *path, int maxlen);

CF_EXPORT CFArrayRef _CFGetWindowsBinaryDirectories(void);

CF_EXPORT CFStringRef _CFStringCreateHostName(void);

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
#include <CoreFoundation/CFRunLoop.h>
CF_EXPORT void _CFMachPortInstallNotifyPort(CFRunLoopRef rl, CFStringRef mode);
#endif


CF_PRIVATE CFIndex __CFActiveProcessorCount();

#ifndef CLANG_ANALYZER_NORETURN
#if __has_feature(attribute_analyzer_noreturn)
#define CLANG_ANALYZER_NORETURN __attribute__((analyzer_noreturn))
#else
#define CLANG_ANALYZER_NORETURN
#endif
#endif

#if DEPLOYMENT_TARGET_WINDOWS
#define __builtin_unreachable() do { } while (0)
#endif

#if defined(__i386__) || defined(__x86_64__)
    #if defined(__GNUC__)
        #define HALT do {asm __volatile__("int3"); kill(getpid(), 9); __builtin_unreachable(); } while (0)
    #elif defined(_MSC_VER)
        #define HALT do { DebugBreak(); abort(); __builtin_unreachable(); } while (0)
    #else
        #error Compiler not supported
    #endif
#endif


#if defined(DEBUG)
    #define __CFAssert(cond, prio, desc, a1, a2, a3, a4, a5)	\
	do {			\
	    if (!(cond)) {	\
		CFLog(prio, CFSTR(desc), a1, a2, a3, a4, a5); \
		/* HALT; */		\
	    }			\
	} while (0)
#else
    #define __CFAssert(cond, prio, desc, a1, a2, a3, a4, a5)	\
	do {} while (0)
#endif

#define CFAssert(condition, priority, description)			\
    __CFAssert((condition), (priority), description, 0, 0, 0, 0, 0)
#define CFAssert1(condition, priority, description, a1)			\
    __CFAssert((condition), (priority), description, (a1), 0, 0, 0, 0)
#define CFAssert2(condition, priority, description, a1, a2)		\
    __CFAssert((condition), (priority), description, (a1), (a2), 0, 0, 0)
#define CFAssert3(condition, priority, description, a1, a2, a3)		\
    __CFAssert((condition), (priority), description, (a1), (a2), (a3), 0, 0)
#define CFAssert4(condition, priority, description, a1, a2, a3, a4)	\
    __CFAssert((condition), (priority), description, (a1), (a2), (a3), (a4), 0)

#define __kCFLogAssertion	3

// This CF-only log function uses no CF functionality, so it may be called anywhere within CF - including thread teardown or prior to full CF setup
CF_PRIVATE void _CFLogSimple(int32_t lev, char *format, ...);

#if defined(DEBUG)
extern void __CFGenericValidateType_(CFTypeRef cf, CFTypeID type, const char *func);
#define __CFGenericValidateType(cf, type) __CFGenericValidateType_(cf, type, __PRETTY_FUNCTION__)
#else
#define __CFGenericValidateType(cf, type) ((void)0)
#endif

#define CF_INFO_BITS (!!(__CF_BIG_ENDIAN__) * 3)
#define CF_RC_BITS (!!(__CF_LITTLE_ENDIAN__) * 3)

/* Bit manipulation macros */
/* Bits are numbered from 31 on left to 0 on right */
/* May or may not work if you use them on bitfields in types other than UInt32, bitfields the full width of a UInt32, or anything else for which they were not designed. */
/* In the following, N1 and N2 specify an inclusive range N2..N1 with N1 >= N2 */
#define __CFBitfieldMask(N1, N2)	((((UInt32)~0UL) << (31UL - (N1) + (N2))) >> (31UL - N1))
#define __CFBitfieldGetValue(V, N1, N2)	(((V) & __CFBitfieldMask(N1, N2)) >> (N2))
#define __CFBitfieldSetValue(V, N1, N2, X)	((V) = ((V) & ~__CFBitfieldMask(N1, N2)) | (((X) << (N2)) & __CFBitfieldMask(N1, N2)))
#define __CFBitfieldMaxValue(N1, N2)	__CFBitfieldGetValue(0xFFFFFFFFUL, (N1), (N2))

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
	// autorelease pool stuff must be higher than run loop constants
	__CFTSDKeyAutoreleaseData2 = 61,
	__CFTSDKeyAutoreleaseData1 = 62,
	__CFTSDKeyExceptionData = 63,
};

#define __kCFAllocatorTypeID_CONST	2

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
#define __kCFAllocatorGCScannedMemory 0x200     /* GC:  memory should be scanned. */
#define __kCFAllocatorGCObjectMemory 0x400      /* GC:  memory needs to be finalized. */

CF_INLINE auto_memory_type_t CF_GET_GC_MEMORY_TYPE(CFOptionFlags flags) {
	auto_memory_type_t type = (flags & __kCFAllocatorGCScannedMemory ? 0 : AUTO_UNSCANNED) | (flags & __kCFAllocatorGCObjectMemory ? AUTO_OBJECT : 0);
    return type;
}

CF_INLINE void __CFAssignWithWriteBarrier(void **location, void *value) {
    if (kCFUseCollectableAllocator) {
        objc_assign_strongCast((id)value, (id *)location);
    } else {
        *location = value;
    }
}

// Zero-retain count CFAllocator functions, i.e. memory that will be collected, no dealloc necessary
CF_EXPORT void *_CFAllocatorAllocateGC(CFAllocatorRef allocator, CFIndex size, CFOptionFlags hint);
CF_EXPORT void *_CFAllocatorReallocateGC(CFAllocatorRef allocator, void *ptr, CFIndex newsize, CFOptionFlags hint);
CF_EXPORT void _CFAllocatorDeallocateGC(CFAllocatorRef allocator, void *ptr);

CF_EXPORT CFAllocatorRef _CFTemporaryMemoryAllocator(void);

extern uint64_t __CFTimeIntervalToTSR(CFTimeInterval ti);
extern CFTimeInterval __CFTSRToTimeInterval(uint64_t tsr);
// use this instead of attempting to subtract mach_absolute_time() directly, because that can underflow and give an unexpected answer
CF_PRIVATE CFTimeInterval __CFTimeIntervalUntilTSR(uint64_t tsr);
CF_PRIVATE dispatch_time_t __CFTSRToDispatchTime(uint64_t tsr);
CF_PRIVATE uint64_t __CFTSRToNanoseconds(uint64_t tsr);

extern CFStringRef __CFCopyFormattingDescription(CFTypeRef cf, CFDictionaryRef formatOptions);

/* Enhanced string formatting support
 */
CF_PRIVATE CFDictionaryRef _CFStringGetFormatSpecifierConfiguration(CFStringRef aFormatString);
CF_PRIVATE CFStringRef _CFStringCopyWithFomatStringConfiguration(CFStringRef aFormatString, CFDictionaryRef formatConfiguration);
CF_PRIVATE CFStringRef _CFCopyResolvedFormatStringWithConfiguration(CFTypeRef anObject, CFDictionaryRef aConfiguration, CFDictionaryRef formatOptions);

/* result is long long or int, depending on doLonglong
*/
extern Boolean __CFStringScanInteger(CFStringInlineBuffer *buf, CFTypeRef locale, SInt32 *indexPtr, Boolean doLonglong, void *result);
extern Boolean __CFStringScanDouble(CFStringInlineBuffer *buf, CFTypeRef locale, SInt32 *indexPtr, double *resultPtr); 
extern Boolean __CFStringScanHex(CFStringInlineBuffer *buf, SInt32 *indexPtr, unsigned *result);

extern const char *__CFgetenv(const char *n);

CF_PRIVATE Boolean __CFProcessIsRestricted();

// This is really about the availability of C99. We don't have that on Windows, but we should everywhere else.
#if DEPLOYMENT_TARGET_WINDOWS
#define STACK_BUFFER_DECL(T, N, C) T *N = (T *)_alloca((C) * sizeof(T))
#else
#define STACK_BUFFER_DECL(T, N, C) T N[C]
#endif


CF_EXPORT void * __CFConstantStringClassReferencePtr;

#ifdef __CONSTANT_CFSTRINGS__

#define CONST_STRING_DECL(S, V) const CFStringRef S = (const CFStringRef)__builtin___CFStringMakeConstantString(V);
#define PE_CONST_STRING_DECL(S, V) CF_PRIVATE const CFStringRef S = (const CFStringRef)__builtin___CFStringMakeConstantString(V);

#else

struct CF_CONST_STRING {
    CFRuntimeBase _base;
    uint8_t *_ptr;
    uint32_t _length;
};

CF_EXPORT int __CFConstantStringClassReference[];

/* CFNetwork also has a copy of the CONST_STRING_DECL macro (for use on platforms without constant string support in cc); please warn cfnetwork-core@group.apple.com of any necessary changes to this macro. -- REW, 1/28/2002 */

#if __CF_BIG_ENDIAN__

#define CONST_STRING_DECL(S, V)			\
static struct CF_CONST_STRING __ ## S ## __ = {{(uintptr_t)&__CFConstantStringClassReference, {0x00, 0x00, 0x07, 0xc8}}, (uint8_t *)V, sizeof(V) - 1}; \
const CFStringRef S = (CFStringRef) & __ ## S ## __;
#define PE_CONST_STRING_DECL(S, V)			\
static struct CF_CONST_STRING __ ## S ## __ = {{(uintptr_t)&__CFConstantStringClassReference, {0x00, 0x00, 0x07, 0xc8}}, (uint8_t *)V, sizeof(V) - 1}; \
CF_PRIVATE const CFStringRef S = (CFStringRef) & __ ## S ## __;

#elif __CF_LITTLE_ENDIAN__

#define CONST_STRING_DECL(S, V)			\
static struct CF_CONST_STRING __ ## S ## __ = {{(uintptr_t)&__CFConstantStringClassReference, {0xc8, 0x07, 0x00, 0x00}}, (uint8_t *)(V), sizeof(V) - 1}; \
const CFStringRef S = (CFStringRef) & __ ## S ## __;
#define PE_CONST_STRING_DECL(S, V)			\
static struct CF_CONST_STRING __ ## S ## __ = {{(uintptr_t)&__CFConstantStringClassReference, {0xc8, 0x07, 0x00, 0x00}}, (uint8_t *)(V), sizeof(V) - 1}; \
CF_PRIVATE const CFStringRef S = (CFStringRef) & __ ## S ## __;

#endif

#endif // __CONSTANT_CFSTRINGS__

CF_EXPORT bool __CFOASafe;
CF_EXPORT void __CFSetLastAllocationEventName(void *ptr, const char *classname);



/* Comparators are passed the address of the values; this is somewhat different than CFComparatorFunction is used in public API usually. */
CF_EXPORT CFIndex	CFBSearch(const void *element, CFIndex elementSize, const void *list, CFIndex count, CFComparatorFunction comparator, void *context);

CF_EXPORT CFHashCode	CFHashBytes(UInt8 *bytes, CFIndex length);

CF_EXPORT CFStringEncoding CFStringFileSystemEncoding(void);

CF_PRIVATE CFStringRef __CFStringCreateImmutableFunnel3(CFAllocatorRef alloc, const void *bytes, CFIndex numBytes, CFStringEncoding encoding, Boolean possiblyExternalFormat, Boolean tryToReduceUnicode, Boolean hasLengthByte, Boolean hasNullByte, Boolean noCopy, CFAllocatorRef contentsDeallocator, UInt32 converterFlags);

extern const void *__CFStringCollectionCopy(CFAllocatorRef allocator, const void *ptr);
extern const void *__CFTypeCollectionRetain(CFAllocatorRef allocator, const void *ptr);
extern void __CFTypeCollectionRelease(CFAllocatorRef allocator, const void *ptr);

extern CFTypeRef CFMakeUncollectable(CFTypeRef cf);

CF_PRIVATE void _CFRaiseMemoryException(CFStringRef reason);

CF_PRIVATE Boolean __CFProphylacticAutofsAccess;


#if DEPLOYMENT_TARGET_MACOSX

typedef pthread_mutex_t CFLock_t;

#define CFLockInit ((pthread_mutex_t)PTHREAD_ERRORCHECK_MUTEX_INITIALIZER)
#define CF_LOCK_INIT_FOR_STRUCTS(X) (X = CFLockInit)

#define __CFLock(LP) ({ \
    (void)pthread_mutex_lock(LP); })

#define __CFUnlock(LP) ({ \
    (void)pthread_mutex_unlock(LP); })

#define __CFLockTry(LP) ({ \
    pthread_mutex_trylock(LP) == 0; })

#elif DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI

typedef pthread_mutex_t CFLock_t;

#define CFLockInit ((pthread_mutex_t)PTHREAD_ERRORCHECK_MUTEX_INITIALIZER)
#define CF_LOCK_INIT_FOR_STRUCTS(X) (X = CFLockInit)

#define __CFLock(LP) ({ \
    (void)pthread_mutex_lock(LP); })

#define __CFUnlock(LP) ({ \
    (void)pthread_mutex_unlock(LP); })

#define __CFLockTry(LP) ({ \
    pthread_mutex_trylock(LP) == 0; })

#elif DEPLOYMENT_TARGET_WINDOWS

typedef int32_t CFLock_t;
#define CFLockInit 0
#define CF_LOCK_INIT_FOR_STRUCTS(X) (X = CFLockInit)

CF_INLINE void __CFLock(volatile CFLock_t *lock) {
    while (InterlockedCompareExchange((LONG volatile *)lock, ~0, 0) != 0) {
	Sleep(0);
    }
}

CF_INLINE void __CFUnlock(volatile CFLock_t *lock) {
    MemoryBarrier();
    *lock = 0;
}

CF_INLINE Boolean __CFLockTry(volatile CFLock_t *lock) {
    return (InterlockedCompareExchange((LONG volatile *)lock, ~0, 0) == 0);
}

#elif DEPLOYMENT_TARGET_LINUX

typedef int32_t CFLock_t;
#define CFLockInit 0
#define CF_LOCK_INIT_FOR_STRUCTS(X) (X = CFLockInit)

CF_INLINE void __CFLock(volatile CFLock_t *lock) {
    while (__sync_val_compare_and_swap(lock, 0, ~0) != 0) {
	sleep(0);
    }
}

CF_INLINE void __CFUnlock(volatile CFLock_t *lock) {
    __sync_synchronize();
    *lock = 0;
}

CF_INLINE Boolean __CFLockTry(volatile CFLock_t *lock) {
    return (__sync_val_compare_and_swap(lock, 0, ~0) == 0);
}

#else

#warning CF locks not defined for this platform -- CF is not thread-safe
#define __CFLock(A)	do {} while (0)
#define __CFUnlock(A)	do {} while (0)

#endif


#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
extern uint8_t __CF120293;
extern uint8_t __CF120290;
extern void __THE_PROCESS_HAS_FORKED_AND_YOU_CANNOT_USE_THIS_COREFOUNDATION_FUNCTIONALITY___YOU_MUST_EXEC__(void);
#define CHECK_FOR_FORK() do { __CF120290 = true; if (__CF120293) __THE_PROCESS_HAS_FORKED_AND_YOU_CANNOT_USE_THIS_COREFOUNDATION_FUNCTIONALITY___YOU_MUST_EXEC__(); } while (0)
#define CHECK_FOR_FORK_RET(...) do { CHECK_FOR_FORK(); if (__CF120293) return __VA_ARGS__; } while (0)
#define HAS_FORKED() (__CF120293)
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


/* ==================== Simple path manipulation ==================== */

CF_EXPORT UniChar _CFGetSlash();
CF_PRIVATE CFStringRef _CFGetSlashStr();
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

#if __BLOCKS__
#if DEPLOYMENT_TARGET_WINDOWS
#define	DT_DIR		 4
#define	DT_REG		 8
#define DT_LNK          10
#endif

// This function automatically skips '.' and '..', and '._' files
CF_PRIVATE void _CFIterateDirectory(CFStringRef directoryPath, Boolean (^fileHandler)(CFStringRef fileName, uint8_t fileType));
#endif

#define __CFMaxRuntimeTypes	65535
#define __CFRuntimeClassTableSize 1024

extern void _CFRuntimeSetInstanceTypeIDAndIsa(CFTypeRef cf, CFTypeID newTypeID);

#define CF_OBJC_FUNCDISPATCHV(typeID, obj, ...) do { } while (0)
#define CF_OBJC_CALLV(obj, ...) (0)
#define CF_IS_OBJC(typeID, obj) (0)
#define __CFISAForTypeID(t) (0)

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

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED

extern void *__CFLookupCFNetworkFunction(const char *name);

#define DEFINE_WEAK_CFNETWORK_FUNC(R, N, P, A, ...)             \
    typedef R (*dyfuncptr)P;                                    \
    static dyfuncptr dyfunc = (dyfuncptr)(~(uintptr_t)0);	\
    if ((dyfuncptr)(~(uintptr_t)0) == dyfunc) {			\
        dyfunc = (dyfuncptr)__CFLookupCFNetworkFunction(#N); }	\
    if (dyfunc) { return dyfunc A ; }				\
    return __VA_ARGS__ ;					\
}

#define DEFINE_WEAK_CFNETWORK_FUNC_FAIL(R, N, P, A, FAILACTION, ...)	\
static R __CFNetwork_ ## N P {                                  \
    typedef R (*dyfuncptr)P;                                    \
    static dyfuncptr dyfunc = (dyfuncptr)(~(uintptr_t)0);       \
    if ((dyfuncptr)(~(uintptr_t)0) == dyfunc) {                 \
        dyfunc = (dyfuncptr)__CFLookupCFNetworkFunction(#N); }  \
    if (dyfunc) { return dyfunc A ; }                           \
    FAILACTION ;                                                \
    return __VA_ARGS__ ;                                        \
}

#else
#define DEFINE_WEAK_CFNETWORK_FUNC(R, N, P, A, ...)
#define DEFINE_WEAK_CFNETWORK_FUNC_FAIL(R, N, P, A, ...)
#endif

#define DEFINE_WEAK_CARBONCORE_FUNC(R, N, P, A, ...)

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED

extern void *__CFLookupCoreServicesInternalFunction(const char *name);

#define DEFINE_WEAK_CORESERVICESINTERNAL_FUNC(R, N, P, A, ...)              \
    static R __CFCoreServicesInternal_ ## N P {                             \
        typedef R (*dyfuncptr)P;                                            \
        static dyfuncptr dyfunc = (dyfuncptr)(~(uintptr_t)0);               \
        if ((dyfuncptr)(~(uintptr_t)0) == dyfunc) {                         \
            dyfunc = (dyfuncptr)__CFLookupCoreServicesInternalFunction(#N); \
        }                                                                   \
        if (dyfunc) {                                                       \
            return dyfunc A ;                                               \
        }                                                                   \
        return __VA_ARGS__ ;                                                \
    }

#else
#define DEFINE_WEAK_CORESERVICESINTERNAL_FUNC(R, N, P, A, ...)
#endif

CF_PRIVATE CFComparisonResult _CFCompareStringsWithLocale(CFStringInlineBuffer *str1, CFRange str1Range, CFStringInlineBuffer *str2, CFRange str2Range, CFOptionFlags options, const void *compareLocale);


CF_PRIVATE CFArrayRef _CFBundleCopyUserLanguages();


// This should only be used in CF types, not toll-free bridged objects!
// It should not be used with CFAllocator arguments!
// Use CFGetAllocator() in the general case, and this inline function in a few limited (but often called) situations.
CF_INLINE CFAllocatorRef __CFGetAllocator(CFTypeRef cf) {	// !!! Use with CF types only, and NOT WITH CFAllocator!
#if OBJC_HAVE_TAGGED_POINTERS
    if (_objc_isTaggedPointer(cf)) {
        return kCFAllocatorSystemDefault;
    }
#endif
    if (__builtin_expect(__CFBitfieldGetValue(((const CFRuntimeBase *)cf)->_cfinfo[CF_INFO_BITS], 7, 7), 1)) {
	return kCFAllocatorSystemDefault;
    }
    return *(CFAllocatorRef *)((char *)cf - sizeof(CFAllocatorRef));
}

/* !!! Avoid #importing objc.h; e.g. converting this to a .m file */
struct __objcFastEnumerationStateEquivalent {
    unsigned long state;
    unsigned long *itemsPtr;
    unsigned long *mutationsPtr;
    unsigned long extra[5];
};

#if 0
#pragma mark -
#pragma mark Windows Compatability
#endif

// Need to use the _O_BINARY flag on Windows to get the correct behavior
#if DEPLOYMENT_TARGET_WINDOWS
#define CF_OPENFLGS	(_O_BINARY|_O_NOINHERIT)
#else
#define CF_OPENFLGS	(0)
#endif

#if DEPLOYMENT_TARGET_WINDOWS

// These are replacements for pthread calls on Windows
CF_EXPORT int _NS_pthread_main_np();
CF_EXPORT int _NS_pthread_setspecific(pthread_key_t key, const void *val);
CF_EXPORT void* _NS_pthread_getspecific(pthread_key_t key);
CF_EXPORT int _NS_pthread_key_init_np(int key, void (*destructor)(void *));
CF_EXPORT void _NS_pthread_setname_np(const char *name);

// map use of pthread_set/getspecific to internal API
#define pthread_setspecific _NS_pthread_setspecific
#define pthread_getspecific _NS_pthread_getspecific
#define pthread_key_init_np _NS_pthread_key_init_np
#define pthread_main_np _NS_pthread_main_np
#define pthread_setname_np _NS_pthread_setname_np
#endif

#if DEPLOYMENT_TARGET_WINDOWS
// replacement for DISPATCH_QUEUE_OVERCOMMIT until we get a bug fix in dispatch on Windows
// <rdar://problem/7923891> dispatch on Windows: Need queue_private.h
#define DISPATCH_QUEUE_OVERCOMMIT 2
#endif

#if DEPLOYMENT_TARGET_WINDOWS
CF_PRIVATE const wchar_t *_CFDLLPath(void);
#endif

/* Buffer size for file pathname */
#if DEPLOYMENT_TARGET_WINDOWS
#define CFMaxPathSize ((CFIndex)262)
#define CFMaxPathLength ((CFIndex)260)
#define PATH_SEP '\\'
#define PATH_SEP_STR CFSTR("\\")
#define PATH_MAX MAX_PATH
#else
#define CFMaxPathSize ((CFIndex)1026)
#define CFMaxPathLength ((CFIndex)1024)
#define PATH_SEP '/'
#define PATH_SEP_STR CFSTR("/")
#endif

CF_INLINE const char *CFPathRelativeToAppleFrameworksRoot(const char *path, Boolean *allocated) {
    if (path) {
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

#include <dispatch/dispatch.h>
#include <dispatch/private.h>

#if (DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED)

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

#else

CF_INLINE dispatch_queue_t __CFDispatchQueueGetGenericMatchingMain(void) {
    return dispatch_get_global_queue(QOS_CLASS_USER_INTERACTIVE, DISPATCH_QUEUE_OVERCOMMIT);
}

CF_INLINE dispatch_queue_t __CFDispatchQueueGetGenericMatchingCurrent(void) {
    return dispatch_get_global_queue(QOS_CLASS_USER_INTERACTIVE, 0); // DISPATCH_QUEUE_OVERCOMMIT left out intentionally at this point
}

CF_INLINE dispatch_queue_t __CFDispatchQueueGetGenericBackground(void) {
    return dispatch_get_global_queue(QOS_CLASS_UTILITY, DISPATCH_QUEUE_OVERCOMMIT);
}

#endif


CF_EXTERN_C_END

#endif /* ! __COREFOUNDATION_CFINTERNAL__ */

