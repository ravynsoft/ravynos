/*	CoreFoundation_Prefix.h
	Copyright (c) 2005-2019, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
*/

#ifndef __COREFOUNDATION_PREFIX_H__
#define __COREFOUNDATION_PREFIX_H__ 1

#if __has_include(<CoreFoundation/TargetConditionals.h>)
#include <CoreFoundation/TargetConditionals.h>
#define __TARGETCONDITIONALS__ // Prevent loading the macOS TargetConditionals.h at all.
#else
#include <TargetConditionals.h>
#endif

#include <CoreFoundation/CFAvailability.h>

#if TARGET_OS_WASI
#define __HAS_DISPATCH__ 0
#else
#define __HAS_DISPATCH__ 1
#endif

// Darwin may or may not define these macros, but we rely on them for building in Swift; define them privately.
#ifndef TARGET_OS_LINUX
#define TARGET_OS_LINUX 0
#endif
#ifndef TARGET_OS_BSD
#define TARGET_OS_BSD 0
#endif
#ifndef TARGET_OS_ANDROID
#define TARGET_OS_ANDROID 0
#endif
#ifndef TARGET_OS_CYGWIN
#define TARGET_OS_CYGWIN 0
#endif
#ifndef TARGET_OS_WASI
#define TARGET_OS_WASI 0
#endif
#ifndef TARGET_OS_MAC
#define TARGET_OS_MAC 0
#endif
#ifndef TARGET_OS_IPHONE
#define TARGET_OS_IPHONE 0
#endif
#ifndef TARGET_OS_OSX
#define TARGET_OS_OSX 0
#endif
#ifndef TARGET_OS_IOS
#define TARGET_OS_IOS 0
#endif
#ifndef TARGET_OS_TV
#define TARGET_OS_TV 0
#endif
#ifndef TARGET_OS_WATCH
#define TARGET_OS_WATCH 0
#endif

#include <CoreFoundation/CFBase.h>


#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#if TARGET_OS_WIN32 && defined(__cplusplus)
extern "C" {
#endif

#if TARGET_OS_IPHONE && TARGET_OS_SIMULATOR // work around <rdar://problem/16507706>
#include <pthread.h>
#include <pthread/qos.h>
#define qos_class_self() (QOS_CLASS_UTILITY)
#define qos_class_main() (QOS_CLASS_UTILITY)
#define pthread_set_qos_class_self_np(A, B) do {} while (0)
#define pthread_override_qos_class_start_np(A, B, C) (NULL)
#define pthread_override_qos_class_end_np(A) do {} while (0)
#elif TARGET_OS_MAC
#include <pthread.h>
#include <pthread/qos.h>
#endif

#define SystemIntegrityCheck(A, B)	do {} while (0)

    
#if INCLUDE_OBJC
#include <objc/objc.h>
#else
typedef signed char	BOOL; 
typedef char * id;
typedef char * Class;
#ifndef YES
#define YES (BOOL)1
#endif
#ifndef NO
#define NO (BOOL)0
#endif
#ifndef nil
#define nil NULL
#endif
#endif

#define CRSetCrashLogMessage(A) do {} while (0)
#define CRSetCrashLogMessage2(A) do {} while (0)

#if TARGET_OS_MAC
#include <libkern/OSAtomic.h>
#include <pthread.h>
#endif

#if TARGET_OS_WIN32
#define BOOL WINDOWS_BOOL

#define MAXPATHLEN MAX_PATH
#undef MAX_PATH
#undef INVALID_HANDLE_VALUE

#define WIN32_LEAN_AND_MEAN

#ifndef WINVER
#define WINVER  0x0601
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif

// The order of these includes is important
#define FD_SETSIZE 1024
#include <winsock2.h>
#include <windows.h>

#undef BOOL

#endif

    
/* This macro creates some helper functions which are useful in dealing with libdispatch:
 *  __ PREFIX Queue -- manages and returns a singleton serial queue
 *
 * Use the macro like this:
 *   DISPATCH_HELPER_FUNCTIONS(fh, NSFileHandle)
 */

#if __HAS_DISPATCH__
    
#define DISPATCH_HELPER_FUNCTIONS(PREFIX, QNAME)			\
static dispatch_queue_t __ ## PREFIX ## Queue(void) {			\
    static volatile dispatch_queue_t __ ## PREFIX ## dq = NULL;		\
    if (!__ ## PREFIX ## dq) {						\
        dispatch_queue_t dq = dispatch_queue_create("com.apple." # QNAME, NULL); \
        void * volatile *loc = (void * volatile *)&__ ## PREFIX ## dq;	\
        if (!OSAtomicCompareAndSwapPtrBarrier(NULL, dq, loc)) {		\
            dispatch_release(dq);					\
        }								\
    }									\
    return __ ## PREFIX ## dq;						\
}

#else

#define DISPATCH_HELPER_FUNCTIONS(PREFIX, QNAME)
    
#endif
    
    
// hint to the analyzer that the caller is no longer responsable for the object and that it will be transfered to the reciver that is opaque to the caller
#if __clang_analyzer__
#define CF_TRANSFER_OWNERSHIP(obj) (__typeof(obj))[(id)obj autorelease]
#else
#define CF_TRANSFER_OWNERSHIP(obj) obj
#endif
    
// hint to the analyzer that the retain/releases are balanced in other locations; the string should be searchable to identify the coorisponding location for the retain/release. These macros should be used with great caution in that they distort the actual retain/release nature of what is happening to the analyzer. Reasonable locations would be in the cases where a value needs to be retained over the lifespan of an external event like a remote machine/process etc.
// NOTE: these seem like they may be backwards - however they are intended to be promises to the analyzer of what will come to pass
#if __clang_analyzer__
#define CF_RELEASE_BALANCED_ELSEWHERE(obj, identified_location) if (obj) CFRetain(obj)
#else
#define CF_RELEASE_BALANCED_ELSEWHERE(obj, identified_location) do { } while (0)
#endif
    
#if __clang_analyzer__
#define CF_RETAIN_BALANCED_ELSEWHERE(obj, identified_location) if (obj) CFRelease(obj)
#else
#define CF_RETAIN_BALANCED_ELSEWHERE(obj, identified_location) do { } while (0)
#endif

#if (TARGET_OS_LINUX && !TARGET_OS_ANDROID && !TARGET_OS_CYGWIN) || TARGET_OS_WIN32
CF_INLINE size_t
strlcpy(char * dst, const char * src, size_t maxlen) {
    const size_t srclen = strlen(src);
    if (srclen < maxlen) {
        memcpy(dst, src, srclen+1);
    } else if (maxlen != 0) {
        memcpy(dst, src, maxlen-1);
        dst[maxlen-1] = '\0';
    }
    return srclen;
}

CF_INLINE size_t
strlcat(char * dst, const char * src, size_t maxlen) {
    const size_t srclen = strlen(src);
    const size_t dstlen = strnlen(dst, maxlen);
    if (dstlen == maxlen) return maxlen+srclen;
    if (srclen < maxlen-dstlen) {
        memcpy(dst+dstlen, src, srclen+1);
    } else {
        memcpy(dst+dstlen, src, maxlen-dstlen-1);
        dst[maxlen-1] = '\0';
    }
    return dstlen + srclen;
}
#endif

#if TARGET_OS_WIN32
// Compatibility with boolean.h
#if defined(__x86_64__)
typedef unsigned int	boolean_t;
#else
typedef int		boolean_t;
#endif
#endif
    
#if TARGET_OS_BSD
#include <string.h>
#include <sys/stat.h> // mode_t
#endif

#if TARGET_OS_LINUX || TARGET_OS_BSD || TARGET_OS_WIN32 || TARGET_OS_WASI
// Implemented in CFPlatform.c
CF_EXPORT bool OSAtomicCompareAndSwapPtr(void *oldp, void *newp, void *volatile *dst);
CF_EXPORT bool OSAtomicCompareAndSwapLong(long oldl, long newl, long volatile *dst);
CF_EXPORT bool OSAtomicCompareAndSwapPtrBarrier(void *oldp, void *newp, void *volatile *dst);
CF_EXPORT bool OSAtomicCompareAndSwap64Barrier( int64_t __oldValue, int64_t __newValue, volatile int64_t *__theValue );

CF_EXPORT int32_t OSAtomicDecrement32Barrier(volatile int32_t *dst);
CF_EXPORT int32_t OSAtomicIncrement32Barrier(volatile int32_t *dst);
CF_EXPORT int32_t OSAtomicIncrement32(volatile int32_t *theValue);
CF_EXPORT int32_t OSAtomicDecrement32(volatile int32_t *theValue);

CF_EXPORT int32_t OSAtomicAdd32( int32_t theAmount, volatile int32_t *theValue );
CF_EXPORT int32_t OSAtomicAdd32Barrier( int32_t theAmount, volatile int32_t *theValue );
CF_EXPORT bool OSAtomicCompareAndSwap32Barrier( int32_t oldValue, int32_t newValue, volatile int32_t *theValue );

CF_EXPORT void OSMemoryBarrier();

#include <time.h>

CF_INLINE uint64_t mach_absolute_time() {
#if TARGET_OS_WIN32
    ULONGLONG ullTime;
	QueryUnbiasedInterruptTimePrecise(&ullTime);
    return ullTime;
#elif TARGET_OS_LINUX || TARGET_OS_BSD || TARGET_OS_MAC
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_nsec + (uint64_t)ts.tv_sec * 1000000000UL;
#endif
}

#define malloc_default_zone() (void *)0
#endif // TARGET_OS_LINUX || TARGET_OS_BSD || TARGET_OS_WIN32 || TARGET_OS_WASI

#if TARGET_OS_LINUX || TARGET_OS_WIN32 || defined(__OpenBSD__) || TARGET_OS_WASI
#define strtod_l(a,b,locale) strtod(a,b)
#define strtoul_l(a,b,c,locale) strtoul(a,b,c)
#define strtol_l(a,b,c,locale) strtol(a,b,c)

#define fprintf_l(a,locale,b,...) fprintf(a, b, __VA_ARGS__)

CF_INLINE int flsl( long mask ) {
    int idx = 0;
    while (mask != 0) {
        mask = (unsigned long)mask >> 1;
        idx++;
    }
    return idx;
}
#endif // TARGET_OS_LINUX || TARGET_OS_WIN32 || defined(__OpenBSD__) || TARGET_OS_WASI

#if TARGET_OS_LINUX || TARGET_OS_WASI
    
#define CF_PRIVATE extern __attribute__((visibility("hidden")))
#define __weak

#define strtoll_l(a,b,c,locale) strtoll(a,b,c)
#define strncasecmp_l(a, b, c, d) strncasecmp(a, b, c)

#if !TARGET_OS_WASI
#include <pthread.h>
#endif

#if TARGET_OS_ANDROID
typedef unsigned long fd_mask;
#endif
    

#if !TARGET_OS_CYGWIN && !TARGET_OS_BSD
#define issetugid() 0
#endif

#if TARGET_OS_CYGWIN
#define HAVE_STRUCT_TIMESPEC 1
#define strncasecmp_l(a, b, c, d) strncasecmp(a, b, c)
#define _NO_BOOL_TYPEDEF
#undef interface
#endif

#if TARGET_OS_CYGWIN
#define HAVE_STRUCT_TIMESPEC 1
#define strncasecmp_l(a, b, c, d) strncasecmp(a, b, c)
#define _NO_BOOL_TYPEDEF
#undef interface
#endif

#include <malloc.h>
CF_INLINE size_t malloc_size(void *memblock) {
    return malloc_usable_size(memblock);
}
#endif
    
#if TARGET_OS_BSD
#define HAVE_STRUCT_TIMESPEC 1

#define CF_PRIVATE extern __attribute__((visibility("hidden")))
#define __strong
#define __weak

#if defined(__OpenBSD__)
#define strtoll_l(a,b,c,locale) strtoll(a,b,c)
#endif
#endif

#if TARGET_OS_LINUX || TARGET_OS_BSD
#include <sys/param.h>
#endif
#if TARGET_OS_WIN32 || TARGET_OS_LINUX
#if !defined(ABS)
#define ABS(A)	((A) < 0 ? (-(A)) : (A))
#endif
#endif

#if TARGET_OS_WIN32

// Defined for source compatibility
#define ino_t _ino_t
#define off_t _off_t
typedef int mode_t;

// This works because things aren't actually exported from the DLL unless they have a __declspec(dllexport) on them... so extern by itself is closest to __private_extern__ on Mac OS
#define CF_PRIVATE extern
    
#define __builtin_expect(P1,P2) P1

#include <sys/stat.h>

// These are replacements for POSIX calls on Windows, ensuring that the UTF8 parameters are converted to UTF16 before being passed to Windows
CF_EXPORT int _NS_stat(const char *name, struct _stat *st);
CF_EXPORT int _NS_mkdir(const char *name);
CF_EXPORT int _NS_rmdir(const char *name);
CF_EXPORT int _NS_chmod(const char *name, int mode);
CF_EXPORT int _NS_unlink(const char *name);
CF_EXPORT char *_NS_getcwd(char *dstbuf, size_t size);     // Warning: this doesn't support dstbuf as null even though 'getcwd' does
CF_EXPORT char *_NS_getenv(const char *name);
CF_EXPORT int _NS_rename(const char *oldName, const char *newName);
CF_EXPORT int _NS_open(const char *name, int oflag, int pmode);
CF_EXPORT int _NS_chdir(const char *name);
CF_EXPORT int _NS_mkstemp(char *name, int bufSize);
CF_EXPORT int _NS_access(const char *name, int amode);

#define __PRETTY_FUNCTION__ __FUNCTION__

#define malloc_zone_from_ptr(a) (void *)0
#define malloc_zone_malloc(zone,size) malloc(size)
#define malloc_zone_memalign(zone,align,size) malloc(size)
#define malloc_zone_calloc(zone,count,size) calloc(count,size)
#define bcopy(b1,b2,len) memmove(b2, b1, (size_t)(len))
typedef int malloc_zone_t;
typedef int uid_t;
typedef int gid_t;
#define geteuid() 0
#define getuid() 0
#define getegid() 0

#define fsync(a) _commit(a)
#define malloc_create_zone(a,b) 123
#define malloc_set_zone_name(zone,name)
#define malloc_zone_realloc(zone,ptr,size) realloc(ptr,size)
#define malloc_zone_free(zone,ptr) free(ptr)

// implemented in CFInternal.h
#define OSSpinLockLock(A) __CFLock(A)
#define OSSpinLockUnlock(A) __CFUnlock(A)
    
typedef int32_t OSSpinLock;

#define OS_SPINLOCK_INIT       0

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <malloc.h>

CF_INLINE size_t malloc_size(void *memblock) {
    return _msize(memblock);
}

CF_INLINE long long llabs(long long v) {
    if (v < 0) return -v;
    return v;
}

#define strtoll_l(a,b,c,locale) _strtoi64(a,b,c)
#define strncasecmp(a, b, c) _strnicmp(a, b, c)
#define strncasecmp_l(a, b, c, d) _strnicmp(a, b, c)
#define snprintf _snprintf

#define sleep(x) Sleep(1000*x)

#define issetugid() 0

#include <io.h>
#include <fcntl.h>
#include <errno.h>
    
CF_INLINE int popcountll(long long x) {
    int count = 0;
    while (x) {
        count++;
        x &= x - 1; // reset LS1B
    }
    return count;
}

#endif

#if !defined(CF_PRIVATE)
#define CF_PRIVATE __attribute__((__visibility__("hidden"))) extern
#endif
    
    // [FIXED_35517899] We can't currently support this, but would like to leave things annotated
#if !defined(CF_TEST_PRIVATE)
#define CF_TEST_PRIVATE CF_PRIVATE
#endif

#if TARGET_OS_WIN32 || (TARGET_OS_LINUX && !defined(_GNU_SOURCE))

#include <stdarg.h>

CF_PRIVATE int asprintf(char **ret, const char *format, ...);

#endif

#if TARGET_OS_WIN32 && defined(__cplusplus)
} // extern "C"
#endif

#endif // __COREFOUNDATION_PREFIX_H__
