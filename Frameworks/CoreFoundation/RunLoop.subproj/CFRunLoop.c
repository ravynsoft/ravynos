/*	CFRunLoop.c
	Copyright (c) 1998-2019, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
	Responsibility: Michael LeHew
*/

#include <CoreFoundation/CFRunLoop.h>
#include <CoreFoundation/CFSet.h>
#include <CoreFoundation/CFBag.h>
#include <CoreFoundation/CFNumber.h>
#include <CoreFoundation/CFPreferences.h>
#include "CFInternal.h"
#include "CFPriv.h"
#include "CFRuntime_Internal.h"
#include "CFMachPort_Internal.h"
#include <math.h>
#include <stdio.h>
#include <limits.h>

#if __has_include(<unistd.h>)
#include <unistd.h>
#endif
#if _POSIX_THREADS
#include <pthread.h>
#endif
#if __HAS_DISPATCH__
#include <dispatch/dispatch.h>
#endif

extern void objc_terminate(void);


#if TARGET_OS_WIN32
// typeinfo.h has been removed in VS2019 16.3
#if __has_include(<typeinfo.h>)
#include <typeinfo.h>
#endif
#endif
#include "CFOverflow.h"

#if DEPLOYMENT_RUNTIME_OBJC
#define USE_DISPATCH_SOURCE_FOR_TIMERS __HAS_DISPATCH__
#else
#define USE_DISPATCH_SOURCE_FOR_TIMERS 0
#endif

static inline uint64_t __CFNanosecondsToTSR(uint64_t ns) {
#if TARGET_OS_MAC || TARGET_OS_LINUX
    return ns;
#else
    CFTimeInterval ti = ns / 1.0E9;
    return __CFTimeIntervalToTSR(ti);
#endif
}

#if USE_DISPATCH_SOURCE_FOR_TIMERS
#if !TARGET_OS_MAC
typedef uint32_t mach_port_t;
typedef uint32_t mach_port_name_t;
#endif
#endif

#if __HAS_DISPATCH__ && __has_include(<dispatch/private.h>)
#include <dispatch/private.h>
#else
extern dispatch_queue_t _dispatch_runloop_root_queue_create_4CF(const char *_Nullable label, unsigned long flags);
#if TARGET_OS_MAC
extern mach_port_t _dispatch_runloop_root_queue_get_port_4CF(dispatch_queue_t queue);
#endif
#endif
extern void _dispatch_source_set_runloop_timer_4CF(dispatch_source_t source, dispatch_time_t start, uint64_t interval, uint64_t leeway);
extern bool _dispatch_runloop_root_queue_perform_4CF(dispatch_queue_t queue);

#if TARGET_OS_MAC
typedef mach_port_t dispatch_runloop_handle_t;
#elif defined(__linux__) || defined(__FreeBSD__)
typedef int dispatch_runloop_handle_t;
#elif TARGET_OS_WIN32
typedef HANDLE dispatch_runloop_handle_t;
#else
typedef uint64_t dispatch_runloop_handle_t;
#endif

#if TARGET_OS_MAC && !__RAVYNOS__
#include <sys/param.h>
#include <CoreFoundation/CFUserNotification.h>
#include <mach/mach.h>
#include <mach/clock_types.h>
#include <mach/clock.h>
#include <unistd.h>
#include <dlfcn.h>

extern void _dispatch_main_queue_callback_4CF(void *);

extern _CFThreadRef pthread_main_thread_np(void);
typedef struct voucher_s *voucher_t;

extern voucher_t _Nullable voucher_copy(void);
extern void os_release(void *object);

extern mach_port_t _dispatch_get_main_queue_port_4CF(void);

#elif TARGET_OS_WIN32 || TARGET_OS_CYGWIN
#include <process.h>
DISPATCH_EXPORT dispatch_runloop_handle_t _dispatch_get_main_queue_port_4CF(void);
DISPATCH_EXPORT void _dispatch_main_queue_callback_4CF(void * _Null_unspecified);

#define MACH_PORT_NULL 0
#define mach_port_name_t HANDLE
#define mach_port_t HANDLE

#elif TARGET_OS_LINUX

#include <dlfcn.h>
#include <poll.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/timerfd.h>

dispatch_runloop_handle_t _dispatch_get_main_queue_port_4CF(void);
extern void _dispatch_main_queue_callback_4CF(void *_Null_unspecified msg);

#else
dispatch_runloop_handle_t _dispatch_get_main_queue_port_4CF(void);
extern void _dispatch_main_queue_callback_4CF(void *_Null_unspecified msg);
#endif

#if TARGET_OS_WIN32 || TARGET_OS_LINUX || TARGET_OS_BSD
CF_EXPORT _CFThreadRef _CF_pthread_main_thread_np(void);
#define pthread_main_thread_np() _CF_pthread_main_thread_np()
#endif

#include <Block.h>
#if __has_include(<Block_private.h>)
#include <Block_private.h>
#elif __has_include("Block_private.h")
#include "Block_private.h"
#endif


// Open source CF may not have this defined.
#ifndef cf_trace
#define cf_trace(...) do {} while (0)
#endif

static int _LogCFRunLoop = 0;
static void _runLoopTimerWithBlockContext(CFRunLoopTimerRef timer, void *opaqueBlock);

// for conservative arithmetic safety, such that (TIMER_DATE_LIMIT + TIMER_INTERVAL_LIMIT + kCFAbsoluteTimeIntervalSince1970) * 10^9 < 2^63
#define TIMER_DATE_LIMIT	4039289856.0
#define TIMER_INTERVAL_LIMIT	504911232.0

#define HANDLE_DISPATCH_ON_BASE_INVOCATION_ONLY 0

#define CRASH(string, errcode) do { char msg[256]; snprintf(msg, 256, string, errcode); CRSetCrashLogMessage(msg); HALT; } while (0)

#if TARGET_OS_WIN32

static _CFThreadRef const kNilPthreadT = INVALID_HANDLE_VALUE;
#define pthreadPointer(a) (GetThreadId(a))
typedef	int kern_return_t;
#define KERN_SUCCESS 0

#elif TARGET_OS_LINUX

static _CFThreadRef const kNilPthreadT = (_CFThreadRef)0;
#define pthreadPointer(a) ((void*)a)
typedef int kern_return_t;
#define KERN_SUCCESS 0

#else

static _CFThreadRef const kNilPthreadT = (_CFThreadRef)0;
typedef int kern_return_t;
#define KERN_SUCCESS 0
#define pthreadPointer(a) a
#define lockCount(a) a
#endif

#pragma mark -

#define CF_RUN_LOOP_PROBES 0

#pragma mark CFRunLoopProbes
#if CF_RUN_LOOP_PROBES
#include "CFRunLoopProbes.h"
#else
#define	CFRUNLOOP_NEXT_TIMER_ARMED(arg0) do { } while (0)
#define	CFRUNLOOP_NEXT_TIMER_ARMED_ENABLED() (0)
#define	CFRUNLOOP_POLL() do { } while (0)
#define	CFRUNLOOP_POLL_ENABLED() (0)
#define	CFRUNLOOP_SLEEP() do { } while (0)
#define	CFRUNLOOP_SLEEP_ENABLED() (0)
#define	CFRUNLOOP_SOURCE_FIRED(arg0, arg1, arg2) do { } while (0)
#define	CFRUNLOOP_SOURCE_FIRED_ENABLED() (0)
#define	CFRUNLOOP_TIMER_CREATED(arg0, arg1, arg2, arg3, arg4, arg5, arg6) do { } while (0)
#define	CFRUNLOOP_TIMER_CREATED_ENABLED() (0)
#define	CFRUNLOOP_TIMER_FIRED(arg0, arg1, arg2, arg3, arg4) do { } while (0)
#define	CFRUNLOOP_TIMER_FIRED_ENABLED() (0)
#define	CFRUNLOOP_TIMER_RESCHEDULED(arg0, arg1, arg2, arg3, arg4, arg5) do { } while (0)
#define	CFRUNLOOP_TIMER_RESCHEDULED_ENABLED() (0)
#define	CFRUNLOOP_WAKEUP(arg0) do { } while (0)
#define	CFRUNLOOP_WAKEUP_ENABLED() (0)
#define	CFRUNLOOP_WAKEUP_FOR_DISPATCH() do { } while (0)
#define	CFRUNLOOP_WAKEUP_FOR_DISPATCH_ENABLED() (0)
#define	CFRUNLOOP_WAKEUP_FOR_NOTHING() do { } while (0)
#define	CFRUNLOOP_WAKEUP_FOR_NOTHING_ENABLED() (0)
#define	CFRUNLOOP_WAKEUP_FOR_SOURCE() do { } while (0)
#define	CFRUNLOOP_WAKEUP_FOR_SOURCE_ENABLED() (0)
#define	CFRUNLOOP_WAKEUP_FOR_TIMEOUT() do { } while (0)
#define	CFRUNLOOP_WAKEUP_FOR_TIMEOUT_ENABLED() (0)
#define	CFRUNLOOP_WAKEUP_FOR_TIMER() do { } while (0)
#define	CFRUNLOOP_WAKEUP_FOR_TIMER_ENABLED() (0)
#define	CFRUNLOOP_WAKEUP_FOR_WAKEUP() do { } while (0)
#define	CFRUNLOOP_WAKEUP_FOR_WAKEUP_ENABLED() (0)
#endif


// NOTE: this is locally defined rather than in CFInternal.h as on Linux,
// `linux/sysctl.h` defines `struct __sysctl_args` with an `__unused` member
// which breaks the build.
#if TARGET_OS_WIN32 || TARGET_OS_CYGWIN || TARGET_OS_LINUX
#ifndef __unused
    #if __has_attribute(unused)
        #define __unused __attribute__((unused))
    #else
        #define __unused
    #endif
#endif // !defined(__unused)
#endif

#define CFRUNLOOP_ARP_BEGIN(...)
#define CFRUNLOOP_ARP_END(...)

// In order to reuse most of the code across Mach and Windows v1 RunLoopSources, we define a
// simple abstraction layer spanning Mach ports and Windows HANDLES
#if TARGET_OS_MAC
typedef mach_port_t __CFPort;
#define CFPORT_NULL MACH_PORT_NULL
typedef mach_port_t __CFPortSet;

static void __THE_SYSTEM_HAS_NO_PORTS_AVAILABLE__(kern_return_t ret) __attribute__((noinline));
static void __THE_SYSTEM_HAS_NO_PORTS_AVAILABLE__(kern_return_t ret) { HALT; };

static __CFPort __CFPortAllocate(uintptr_t guard) {
    __CFPort result = CFPORT_NULL;

    mach_port_options_t options = {
        .flags = MPO_CONTEXT_AS_GUARD | MPO_QLIMIT | MPO_INSERT_SEND_RIGHT | MPO_STRICT,
        .mpl.mpl_qlimit = 1,
    };

    kern_return_t const ret = mach_port_construct(mach_task_self(), &options, (mach_port_context_t)guard, &result);

    if (KERN_SUCCESS != ret) {
        char msg[256];
        snprintf(msg, 256, "*** The system has no mach ports available. You may be able to diagnose which application(s) are using ports by using 'top' or Activity Monitor. (%d) ***", ret);
        CRSetCrashLogMessage(msg); 
        __THE_SYSTEM_HAS_NO_PORTS_AVAILABLE__(ret); 
        return CFPORT_NULL;
    }

    return result;
}

CF_INLINE void __CFPortFree(__CFPort port, uintptr_t guard) {
    kern_return_t const ret = mach_port_destruct(mach_task_self(), port, -1, (mach_port_context_t)guard);
    if (KERN_SUCCESS != ret) {
        
        char msg[256];
        snprintf(msg, 256, "*** Unable to destruct port. (0x%x, %d, %p) ***", port, ret, (void *)guard);
        CRSetCrashLogMessage(msg);
        HALT;
    }
}

static void __NO_SPACE__(kern_return_t ret) __attribute__((noinline));
static void __NO_SPACE__(kern_return_t ret) { HALT; };

static void __RESOURCE_SHORTAGE__(kern_return_t ret) __attribute__((noinline));
static void __RESOURCE_SHORTAGE__(kern_return_t ret) { HALT; };

static void __THE_SYSTEM_HAS_NO_PORT_SETS_AVAILABLE__(kern_return_t ret) __attribute__((noinline));
static void __THE_SYSTEM_HAS_NO_PORT_SETS_AVAILABLE__(kern_return_t ret) {
    if (ret == KERN_NO_SPACE) {
        __NO_SPACE__(ret);
    }
    else if (ret == KERN_RESOURCE_SHORTAGE) {
        __RESOURCE_SHORTAGE__(ret);
    }
    HALT;
};

CF_INLINE __CFPortSet __CFPortSetAllocate(void) {
    __CFPortSet result;
    kern_return_t ret = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_PORT_SET, &result);
    if (KERN_SUCCESS != ret) { __THE_SYSTEM_HAS_NO_PORT_SETS_AVAILABLE__(ret); }
    return (KERN_SUCCESS == ret) ? result : CFPORT_NULL;
}

CF_INLINE kern_return_t __CFPortSetInsert(__CFPort port, __CFPortSet portSet) {
    if (MACH_PORT_NULL == port) {
        return -1;
    }
    return mach_port_insert_member(mach_task_self(), port, portSet);
}

CF_INLINE kern_return_t __CFPortSetRemove(__CFPort port, __CFPortSet portSet) {
    if (MACH_PORT_NULL == port) {
        return -1;
    }
    return mach_port_extract_member(mach_task_self(), port, portSet);
}

CF_INLINE void __CFPortSetFree(__CFPortSet portSet) {
    // NOTE: we rely on the impl of mach_port destroy to extract each member, which it does
    // ALSO NOTE: Per CoreOS  port sets don't have ref counts, so this is equiv to mach_port_destroy, but faster/safer
    const kern_return_t ret = mach_port_mod_refs(mach_task_self(), portSet, MACH_PORT_RIGHT_PORT_SET, -1);
    if (ret != KERN_SUCCESS) {
        CFLog(kCFLogLevelError, CFSTR("error (%d - %s) while trying to free port set: %d"), ret, mach_error_string(ret), portSet);
    }
}

#elif TARGET_OS_WIN32 || TARGET_OS_CYGWIN

typedef HANDLE __CFPort;
#define CFPORT_NULL NULL

// A simple dynamic array of HANDLEs, which grows to a high-water mark
typedef struct ___CFPortSet {
    uint16_t	used;
    uint16_t	size;
    HANDLE	*handles;
    CFLock_t lock;		// insert and remove must be thread safe, like the Mach calls
} *__CFPortSet;

CF_INLINE __CFPort __CFPortAllocate(__unused uintptr_t guard) {
    return CreateEventA(NULL, true, false, NULL);
}

CF_INLINE void __CFPortFree(__CFPort port, __unused uintptr_t guard) {
    CloseHandle(port);
}

static __CFPortSet __CFPortSetAllocate(void) {
    __CFPortSet result = (__CFPortSet)CFAllocatorAllocate(kCFAllocatorSystemDefault, sizeof(struct ___CFPortSet), 0);
    result->used = 0;
    result->size = 4;
    result->handles = (HANDLE *)CFAllocatorAllocate(kCFAllocatorSystemDefault, result->size * sizeof(HANDLE), 0);
    CF_LOCK_INIT_FOR_STRUCTS(result->lock);
    return result;
}

static void __CFPortSetFree(__CFPortSet portSet) {
    CFAllocatorDeallocate(kCFAllocatorSystemDefault, portSet->handles);
    CFAllocatorDeallocate(kCFAllocatorSystemDefault, portSet);
}

// Returns portBuf if ports fit in that space, else returns another ptr that must be freed
static __CFPort *__CFPortSetGetPorts(__CFPortSet portSet, __CFPort *portBuf, uint32_t bufSize, uint32_t *portsUsed) {
    __CFLock(&(portSet->lock));
    __CFPort *result = portBuf;
    if (bufSize < portSet->used)
	result = (__CFPort *)CFAllocatorAllocate(kCFAllocatorSystemDefault, portSet->used * sizeof(HANDLE), 0);
    if (portSet->used > 1) {
	// rotate the ports to vaguely simulate round-robin behaviour
	uint16_t lastPort = portSet->used - 1;
	HANDLE swapHandle = portSet->handles[0];
	memmove(portSet->handles, &portSet->handles[1], lastPort * sizeof(HANDLE));
	portSet->handles[lastPort] = swapHandle;
    }
    memmove(result, portSet->handles, portSet->used * sizeof(HANDLE));
    *portsUsed = portSet->used;
    __CFUnlock(&(portSet->lock));
    return result;
}

static kern_return_t __CFPortSetInsert(__CFPort port, __CFPortSet portSet) {
    if (NULL == port) {
        return -1;
    }
    __CFLock(&(portSet->lock));
    if (portSet->used >= portSet->size) {
        portSet->size += 4;
        portSet->handles = __CFSafelyReallocateWithAllocator(kCFAllocatorSystemDefault, portSet->handles, portSet->size * sizeof(HANDLE), 0, NULL);
    }
    if (portSet->used >= MAXIMUM_WAIT_OBJECTS) {
        CFLog(kCFLogLevelWarning, CFSTR("*** More than MAXIMUM_WAIT_OBJECTS (%d) ports add to a port set.  The last ones will be ignored."), MAXIMUM_WAIT_OBJECTS);
    }
    portSet->handles[portSet->used++] = port;
    __CFUnlock(&(portSet->lock));
    return KERN_SUCCESS;
}

static kern_return_t __CFPortSetRemove(__CFPort port, __CFPortSet portSet) {
    int i, j;
    if (NULL == port) {
        return -1;
    }
    __CFLock(&(portSet->lock));
    for (i = 0; i < portSet->used; i++) {
        if (portSet->handles[i] == port) {
            for (j = i+1; j < portSet->used; j++) {
                portSet->handles[j-1] = portSet->handles[j];
            }
            portSet->used--;
            __CFUnlock(&(portSet->lock));
            return true;
        }
    }
    __CFUnlock(&(portSet->lock));
    return KERN_SUCCESS;
}

#elif TARGET_OS_LINUX
// eventfd/timerfd descriptor
typedef int __CFPort;
#define CFPORT_NULL -1
#define MACH_PORT_NULL CFPORT_NULL

// epoll file descriptor
typedef int __CFPortSet;
#define CFPORTSET_NULL -1

static __CFPort __CFPortAllocate(__unused uintptr_t guard) {
    return eventfd(0, EFD_CLOEXEC|EFD_NONBLOCK);
}

CF_INLINE void __CFPortFree(__CFPort port, __unused uintptr_t guard) {
    close(port);
}

CF_INLINE __CFPortSet __CFPortSetAllocate(void) {
    return epoll_create1(EPOLL_CLOEXEC);
}

CF_INLINE kern_return_t __CFPortSetInsert(__CFPort port, __CFPortSet portSet) {
    if (CFPORT_NULL == port) {
        return -1;
    }
    struct epoll_event event;
    memset(&event, 0, sizeof(event));
    event.data.fd = port;
    event.events = EPOLLIN|EPOLLET;
    
    return epoll_ctl(portSet, EPOLL_CTL_ADD, port, &event);
}

CF_INLINE kern_return_t __CFPortSetRemove(__CFPort port, __CFPortSet portSet) {
    if (CFPORT_NULL == port) {
        return -1;
    }
    return epoll_ctl(portSet, EPOLL_CTL_DEL, port, NULL);
}

CF_INLINE void __CFPortSetFree(__CFPortSet portSet) {
    close(portSet);
}
#elif TARGET_OS_BSD

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <poll.h>

typedef uint64_t __CFPort;
#define CFPORT_NULL ((__CFPort)-1)

// _dispatch_get_main_queue_port_4CF is a uint64_t, i.e., a __CFPort.
// That is, we can't use one type for the queue handle in Dispatch and a
// different type for __CFPort in CF.
#define __CFPORT_PACK(rfd, wfd) (((uint64_t)(rfd) << 32) | ((uint32_t)(wfd)))
#define __CFPORT_UNPACK_W(port) ((uint32_t)((port) & 0xffffffff))
#define __CFPORT_UNPACK_R(port) ((uint32_t)((port) >> 32))

typedef struct ___CFPortSet {
    int kq;
} *__CFPortSet;
#define CFPORTSET_NULL NULL

#define TIMEOUT_INFINITY UINT64_MAX

// Timers are not pipes; they are kevents on a parent kqueue.
// We must flag these to differentiate them from pipes, but we have
// to pack the (kqueue, timer ident) pair like a __CFPort.
#define __CFPORT_TIMER_PACK(ident, kq) \
    ((1ULL << 63) | ((uint64_t)(ident) << 32) | ((uint32_t)(kq)))
#define __CFPORT_IS_TIMER(port) ((port) & (1ULL << 63))

static __CFPort __CFPortAllocate(__unused uintptr_t guard) {
    __CFPort port;
    int fds[2];
    int r = pipe2(fds, O_CLOEXEC | O_NONBLOCK);
    if (r == -1) {
        return CFPORT_NULL;
    }

    uint32_t rfd = (uint32_t)fds[0], wfd = (uint32_t)fds[1];
    port = __CFPORT_PACK(rfd, wfd);

    if (__CFPORT_IS_TIMER(port)) {
      // This port is not distinguishable from a flagged packed timer.
      close((int)(__CFPORT_UNPACK_W(port)));
      close((int)(__CFPORT_UNPACK_R(port)));
      return CFPORT_NULL;
    }

    return port;
}

static void __CFPortTrigger(__CFPort port) {
    int wfd = (int)__CFPORT_UNPACK_W(port);
    ssize_t result;
    do {
        result = write(wfd, "x", 1);
    } while (result == -1 && errno == EINTR);
}

CF_INLINE void __CFPortFree(__CFPort port, __unused uintptr_t guard) {
    close((int)(__CFPORT_UNPACK_W(port)));
    close((int)(__CFPORT_UNPACK_R(port)));
}

#define __CFPORT_TIMER_UNPACK_ID(port) (((port) >> 32) & 0x7fffffff)
#define __CFPORT_TIMER_UNPACK_KQ(port) ((port) & 0xffffffff)
#define MAX_TIMERS 16
uintptr_t ident = 0;

static __CFPort mk_timer_create(__CFPortSet parent) {
    if (ident > MAX_TIMERS) return CFPORT_NULL;
    ident++;

    int kq = parent->kq;
    __CFPort port = __CFPORT_TIMER_PACK(ident, kq);

    return port;
}

static kern_return_t mk_timer_arm(__CFPort timer, int64_t expire_tsr) {
    uint64_t now = mach_absolute_time();
    uint64_t expire_time = __CFTSRToNanoseconds(expire_tsr);
    int64_t duration = 0;
    if (now <= expire_time) {
        duration = __CFTSRToTimeInterval(expire_time - now) * 1000;
    }

    int id = __CFPORT_TIMER_UNPACK_ID(timer);
    struct kevent tev;
    EV_SET(
        &tev,
        id,
        EVFILT_TIMER,
        EV_ADD | EV_ENABLE,
        0,
        duration,
        (void *)timer);

    int kq = __CFPORT_TIMER_UNPACK_KQ(timer);
    int r = kevent(kq, &tev, 1, NULL, 0, NULL);

    return KERN_SUCCESS;
}

static kern_return_t mk_timer_cancel(__CFPort timer, const void *unused) {
    int id = __CFPORT_TIMER_UNPACK_ID(timer);
    struct kevent tev;
    EV_SET(
        &tev,
        id,
        EVFILT_TIMER,
        EV_DISABLE,
        0,
        0,
        (void *)timer);

    int kq = __CFPORT_TIMER_UNPACK_KQ(timer);
    int r = kevent(kq, &tev, 1, NULL, 0, NULL);

    return KERN_SUCCESS;
}

static kern_return_t mk_timer_destroy(__CFPort timer) {
    int id = __CFPORT_TIMER_UNPACK_ID(timer);
    struct kevent tev;
    EV_SET(
        &tev,
        id,
        EVFILT_TIMER,
        EV_DELETE,
        0,
        0,
        (void *)timer);

    int kq = __CFPORT_TIMER_UNPACK_KQ(timer);
    int r = kevent(kq, &tev, 1, NULL, 0, NULL);

    ident--;
    return KERN_SUCCESS;
}

CF_INLINE __CFPortSet __CFPortSetAllocate(void) {
    struct ___CFPortSet *set = malloc(sizeof(struct ___CFPortSet));
    set->kq = kqueue();
    return set;
}

CF_INLINE kern_return_t __CFPortSetInsert(__CFPort port, __CFPortSet set) {
    if (__CFPORT_IS_TIMER(port)) {
        return 0;
    }

    struct kevent change;
    EV_SET(&change,
        __CFPORT_UNPACK_R(port),
        EVFILT_READ,
        EV_ADD | EV_ENABLE | EV_CLEAR | EV_RECEIPT,
        0,
        0,
        (void *)port);
    struct timespec timeout = {0, 0};
    int r = kevent(set->kq, &change, 1, NULL, 0, &timeout);

    return 0;
}

CF_INLINE kern_return_t __CFPortSetRemove(__CFPort port, __CFPortSet set) {
    if (__CFPORT_IS_TIMER(port)) {
        return 0;
    }

    struct kevent change;
    EV_SET(&change,
        __CFPORT_UNPACK_R(port),
        EVFILT_READ,
        EV_DELETE | EV_RECEIPT,
        0,
        0,
        (void *)port);
    struct timespec timeout = {0, 0};
    int r = kevent(set->kq, &change, 1, NULL, 0, &timeout);

    return 0;
}

CF_INLINE void __CFPortSetFree(__CFPortSet set) {
    close(set->kq);
    free(set);
}

static int __CFPollFileDescriptors(struct pollfd *fds, nfds_t nfds, uint64_t timeout) {
    uint64_t elapsed = 0;
    uint64_t start = mach_absolute_time();
    int result = 0;
    while (1) {
        struct timespec ts = {0};
        struct timespec *tsPtr = &ts;
        if (timeout == TIMEOUT_INFINITY) {
            tsPtr = NULL;
        } else if (elapsed < timeout) {
            uint64_t delta = timeout - elapsed;
            ts.tv_sec = delta / 1000000000UL;
            ts.tv_nsec = delta % 1000000000UL;
        }

        result = ppoll(fds, 1, tsPtr, NULL);

        if (result == -1 && errno == EINTR) {
            uint64_t end = mach_absolute_time();
            elapsed += (end - start);
            start = end;
        } else {
            return result;
        }
    }
}

static Boolean __CFRunLoopServiceFileDescriptors(__CFPortSet set, __CFPort port, uint64_t timeout, __CFPort *livePort) {
    __CFPort awokenPort = CFPORT_NULL;

    if (port != CFPORT_NULL) {
        int rfd = __CFPORT_UNPACK_R(port);
        struct pollfd fdInfo = {
            .fd = rfd,
            .events = POLLIN,
        };

        ssize_t result = __CFPollFileDescriptors(&fdInfo, 1, timeout);
        if (result == 0)
            return false;

        awokenPort = port;
    } else {
        struct kevent awake;
        struct timespec timeout = {0, 0};

        int r = kevent(set->kq, NULL, 0, &awake, 1, &timeout);

        if (r == 0) {
            return false;
        }

        if (awake.flags == EV_ERROR) {
            return false;
        }

        if (awake.filter == EVFILT_READ) {
            char x;
            r = read(awake.ident, &x, 1);
        }

        awokenPort = (__CFPort)awake.udata;
    }

    if (livePort)
        *livePort = awokenPort;

    return true;
}

#else
#error "CFPort* stubs for this platform must be implemented
#endif

#if !defined(__MACTYPES__) && !defined(_OS_OSTYPES_H)
#if defined(__BIG_ENDIAN__)
typedef	struct UnsignedWide {
    UInt32		hi;
    UInt32		lo;
} UnsignedWide;
#elif defined(__LITTLE_ENDIAN__)
typedef	struct UnsignedWide {
    UInt32		lo;
    UInt32		hi;
} UnsignedWide;
#endif
typedef UnsignedWide		AbsoluteTime;
#endif

#if TARGET_OS_MAC
extern mach_port_name_t mk_timer_create(void);
extern kern_return_t mk_timer_destroy(mach_port_name_t name);
extern kern_return_t mk_timer_cancel(mach_port_name_t name, AbsoluteTime *result_time);
extern kern_return_t mk_timer_arm(mach_port_name_t name, uint64_t expire_time);

static uint32_t __CFSendTrivialMachMessage(mach_port_t port, uint32_t msg_id, CFOptionFlags options, uint32_t timeout) {
    mach_msg_header_t header;
    header.msgh_bits = MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND, 0);
    header.msgh_size = sizeof(mach_msg_header_t);
    header.msgh_remote_port = port;
    header.msgh_local_port = MACH_PORT_NULL;
    header.msgh_id = msg_id;
    kern_return_t const result = mach_msg(&header, MACH_SEND_MSG|options, header.msgh_size, 0, MACH_PORT_NULL, timeout, MACH_PORT_NULL);
    __CFMachMessageCheckForAndDestroyUnsentMessage(result, &header);
    return result;
}
#elif TARGET_OS_LINUX

static int mk_timer_create(void) {
    return timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK|TFD_CLOEXEC);
}

static kern_return_t mk_timer_destroy(int timer) {
    return close(timer);
}

static kern_return_t mk_timer_arm(int timer, int64_t expire_time) {
    struct itimerspec ts;
    ts.it_value.tv_sec = expire_time / 1000000000UL;
    ts.it_value.tv_nsec = expire_time % 1000000000UL;
    
    // Non-repeating timer
    ts.it_interval.tv_sec = 0;
    ts.it_interval.tv_nsec = 0;
    
    return timerfd_settime(timer, TFD_TIMER_ABSTIME, &ts, NULL);
}

static kern_return_t mk_timer_cancel(int timer, const void *unused) {
    return mk_timer_arm(timer, 0);
}

CF_INLINE int64_t __CFUInt64ToAbsoluteTime(int64_t x) {
    return x;
}

#elif TARGET_OS_WIN32

static HANDLE mk_timer_create(void) {
    return CreateWaitableTimer(NULL, FALSE, NULL);
}

static kern_return_t mk_timer_destroy(HANDLE name) {
    BOOL res = CloseHandle(name);
    if (!res) {
        DWORD err = GetLastError();
        CFLog(kCFLogLevelError, CFSTR("CFRunLoop: Unable to destroy timer: %d"), err);
    }
    return (int)res;
}

static kern_return_t mk_timer_arm(HANDLE name, uint64_t expire_time) {
    LARGE_INTEGER result;
    // There is a race we know about here, (timer fire time calculated -> thread suspended -> timer armed == late timer fire), but we don't have a way to avoid it at this time, since the only way to specify an absolute value to the timer is to calculate the relative time first. Fixing that would probably require not using the TSR for timers on Windows.
    uint64_t now = mach_absolute_time();
    if (now > expire_time) {
        result.QuadPart = 0;
    } else {
        uint64_t timeDiff = expire_time - now;
        CFTimeInterval amountOfTimeToWait = __CFTSRToTimeInterval(timeDiff);
        // Result is in 100 ns (10**-7 sec) units to be consistent with a FILETIME.
        // CFTimeInterval is in seconds.
        result.QuadPart = -(amountOfTimeToWait * 10000000);
    }

    BOOL res = SetWaitableTimer(name, &result, 0, NULL, NULL, FALSE);
    if (!res) {
        DWORD err = GetLastError();
        CFLog(kCFLogLevelError, CFSTR("CFRunLoop: Unable to set timer: %d"), err);
    }
    return (int)res;
}

static kern_return_t mk_timer_cancel(HANDLE name, AbsoluteTime *result_time) {
    BOOL res = CancelWaitableTimer(name);
    if (!res) {
        DWORD err = GetLastError();
        CFLog(kCFLogLevelError, CFSTR("CFRunLoop: Unable to cancel timer: %d"), err);
    }
    return (int)res;
}
#elif TARGET_OS_BSD
/*
 * This implementation of the mk_timer_* stubs is defined with the
 * implementation of the CFPort* stubs.
 */
#else
#error "mk_timer_* stubs for this platform must be implemented"
#endif


CF_BREAKPOINT_FUNCTION(void _CFRunLoopError_MainThreadHasExited(void));

#pragma mark -
#pragma mark Modes

/* unlock a run loop and modes before doing callouts/sleeping */
/* never try to take the run loop lock with a mode locked */
/* be very careful of common subexpression elimination and compacting code, particular across locks and unlocks! */
/* run loop mode structures should never be deallocated, even if they become empty */

typedef struct __CFRunLoopMode *CFRunLoopModeRef;

struct __CFRunLoopMode {
    CFRuntimeBase _base;
    _CFRecursiveMutex _lock;	/* must have the run loop locked before locking this */
    CFStringRef _name;
    Boolean _stopped;
    char _padding[3];
    CFMutableSetRef _sources0;
    CFMutableSetRef _sources1;
    CFMutableArrayRef _observers;
    CFMutableArrayRef _timers;
    CFMutableDictionaryRef _portToV1SourceMap;
    __CFPortSet _portSet;
    CFIndex _observerMask;
#if USE_DISPATCH_SOURCE_FOR_TIMERS
    dispatch_source_t _timerSource;
    dispatch_queue_t _queue;
    Boolean _timerFired; // set to true by the source when a timer has fired
    Boolean _dispatchTimerArmed;
#endif
    __CFPort _timerPort;
    Boolean _mkTimerArmed;
#if TARGET_OS_WIN32
    DWORD _msgQMask;
    void (*_msgPump)(void);
#endif
    uint64_t _timerSoftDeadline; /* TSR */
    uint64_t _timerHardDeadline; /* TSR */
};

CF_INLINE void __CFRunLoopModeLock(CFRunLoopModeRef rlm) {
    _CFRecursiveMutexLock(&(rlm->_lock));
    //CFLog(6, CFSTR("__CFRunLoopModeLock locked %p"), rlm);
}

CF_INLINE void __CFRunLoopModeUnlock(CFRunLoopModeRef rlm) {
    //CFLog(6, CFSTR("__CFRunLoopModeLock unlocking %p"), rlm);
    _CFRecursiveMutexUnlock(&(rlm->_lock));
}

static Boolean __CFRunLoopModeEqual(CFTypeRef cf1, CFTypeRef cf2) {
    CFRunLoopModeRef rlm1 = (CFRunLoopModeRef)cf1;
    CFRunLoopModeRef rlm2 = (CFRunLoopModeRef)cf2;
    return CFEqual(rlm1->_name, rlm2->_name);
}

static CFHashCode __CFRunLoopModeHash(CFTypeRef cf) {
    CFRunLoopModeRef rlm = (CFRunLoopModeRef)cf;
    return CFHash(rlm->_name);
}

static CFStringRef __CFRunLoopModeCopyDescription(CFTypeRef cf) {
    CFRunLoopModeRef rlm = (CFRunLoopModeRef)cf;
    CFMutableStringRef result;
    result = CFStringCreateMutable(kCFAllocatorSystemDefault, 0);
    CFStringAppendFormat(result, NULL, CFSTR("<CFRunLoopMode %p [%p]>{name = %@, "), rlm, CFGetAllocator(rlm), rlm->_name);
    CFStringAppendFormat(result, NULL, CFSTR("port set = 0x%x, "), rlm->_portSet);
#if USE_DISPATCH_SOURCE_FOR_TIMERS
    CFStringAppendFormat(result, NULL, CFSTR("queue = %p, "), rlm->_queue);
    CFStringAppendFormat(result, NULL, CFSTR("source = %p (%s), "), rlm->_timerSource, rlm->_timerFired ? "fired" : "not fired");
#endif
    CFStringAppendFormat(result, NULL, CFSTR("timer port = 0x%x, "), rlm->_timerPort);
#if TARGET_OS_WIN32
    CFStringAppendFormat(result, NULL, CFSTR("MSGQ mask = %p, "), rlm->_msgQMask);
#endif
    CFStringAppendFormat(result, NULL, CFSTR("\n\tsources0 = %@,\n\tsources1 = %@,\n\tobservers = %@,\n\ttimers = %@,\n\tcurrently %0.09g (%lld) / soft deadline in: %0.09g sec (@ %lld) / hard deadline in: %0.09g sec (@ %lld)\n},\n"), rlm->_sources0, rlm->_sources1, rlm->_observers, rlm->_timers, CFAbsoluteTimeGetCurrent(), mach_absolute_time(), __CFTSRToTimeInterval(rlm->_timerSoftDeadline - mach_absolute_time()), rlm->_timerSoftDeadline, __CFTSRToTimeInterval(rlm->_timerHardDeadline - mach_absolute_time()), rlm->_timerHardDeadline);
    return result;
}

static void __CFRunLoopModeDeallocate(CFTypeRef cf) {
    CFRunLoopModeRef rlm = (CFRunLoopModeRef)cf;
    if (NULL != rlm->_sources0) CFRelease(rlm->_sources0);
    if (NULL != rlm->_sources1) CFRelease(rlm->_sources1);
    if (NULL != rlm->_observers) CFRelease(rlm->_observers);
    if (NULL != rlm->_timers) CFRelease(rlm->_timers);
    if (NULL != rlm->_portToV1SourceMap) CFRelease(rlm->_portToV1SourceMap);
    CFRelease(rlm->_name);
    __CFPortSetFree(rlm->_portSet);
#if USE_DISPATCH_SOURCE_FOR_TIMERS
    if (rlm->_timerSource) {
        dispatch_source_cancel(rlm->_timerSource);
        dispatch_release(rlm->_timerSource);
    }
    if (rlm->_queue) {
        dispatch_release(rlm->_queue);
    }
#endif
    if (CFPORT_NULL != rlm->_timerPort) mk_timer_destroy(rlm->_timerPort);
    _CFRecursiveMutexDestroy(&rlm->_lock);
    memset((char *)cf + sizeof(CFRuntimeBase), 0x7C, sizeof(struct __CFRunLoopMode) - sizeof(CFRuntimeBase));
}

#pragma mark -
#pragma mark Run Loops

struct _block_item {
    struct _block_item *_next;
    CFTypeRef _mode;	// CFString or CFSet
    void (^_block)(void);
};

typedef struct _per_run_data {
    uint32_t a;
    uint32_t b;
    uint32_t stopped;
    uint32_t ignoreWakeUps;
} _per_run_data;

struct __CFRunLoop {
    CFRuntimeBase _base;
    _CFRecursiveMutex _lock;			/* locked for accessing mode list */
    __CFPort _wakeUpPort;			// used for CFRunLoopWakeUp 
    volatile _per_run_data *_perRunData;              // reset for runs of the run loop
    _CFThreadRef _pthread;
    uint32_t _winthread;
    CFMutableSetRef _commonModes;
    CFMutableSetRef _commonModeItems;
    CFRunLoopModeRef _currentMode;
    CFMutableSetRef _modes;
    struct _block_item *_blocks_head;
    struct _block_item *_blocks_tail;
    CFAbsoluteTime _runTime;
    CFAbsoluteTime _sleepTime;
    CFTypeRef _counterpart;
    _Atomic(uint8_t) _fromTSD;
    Boolean _perCalloutARP;
    CFLock_t _timerTSRLock;
};

/* Bit 0 of the base reserved bits is used for stopped state */
/* Bit 1 of the base reserved bits is used for sleeping state */
/* Bit 2 of the base reserved bits is used for deallocating state */

// When `rl` is 0, will push an ARP unconditionally. A hack to facilitate places where we had ARPs before.
static inline uintptr_t __CFRunLoopPerCalloutARPBegin(CFRunLoopRef rl) {
#if DEPLOYMENT_RUNTIME_OBJC
    return !rl || rl->_perCalloutARP ? _CFAutoreleasePoolPush() : 0;
#else
    return 0;
#endif
}

static inline void __CFRunLoopPerCalloutARPEnd(const uintptr_t pool) {
#if DEPLOYMENT_RUNTIME_OBJC
    if (pool) {
        @try {
            _CFAutoreleasePoolPop(pool);
        } @catch (NSException *e) {
            os_log_error(_CFOSLog(), "Caught exception during runloop's autorelease pool drain of client objects %{public}@: %{private}@ userInfo: %{private}@", e.name, e.reason, e.userInfo);
            objc_terminate();
        } @catch (...) {
            objc_terminate();
        }
    }
#endif
}

CF_INLINE volatile _per_run_data *__CFRunLoopPushPerRunData(CFRunLoopRef rl) {
    volatile _per_run_data *previous = rl->_perRunData;
    rl->_perRunData = (volatile _per_run_data *)CFAllocatorAllocate(kCFAllocatorSystemDefault, sizeof(_per_run_data), 0);
    rl->_perRunData->a = 0x4346524C;
    rl->_perRunData->b = 0x4346524C; // 'CFRL'
    rl->_perRunData->stopped = 0x00000000;
    rl->_perRunData->ignoreWakeUps = 0x00000000;
    return previous;
}

CF_INLINE void __CFRunLoopPopPerRunData(CFRunLoopRef rl, volatile _per_run_data *previous) {
    if (rl->_perRunData) CFAllocatorDeallocate(kCFAllocatorSystemDefault, (void *)rl->_perRunData);
    rl->_perRunData = previous;
}

CF_INLINE Boolean __CFRunLoopIsStopped(CFRunLoopRef rl) {
    return (rl->_perRunData->stopped) ? true : false;
}

CF_INLINE void __CFRunLoopSetStopped(CFRunLoopRef rl) {
    rl->_perRunData->stopped = 0x53544F50;	// 'STOP'
}

CF_INLINE void __CFRunLoopUnsetStopped(CFRunLoopRef rl) {
    rl->_perRunData->stopped = 0x0;
}

CF_INLINE Boolean __CFRunLoopIsIgnoringWakeUps(CFRunLoopRef rl) {
    return (rl->_perRunData->ignoreWakeUps) ? true : false;    
}

CF_INLINE void __CFRunLoopSetIgnoreWakeUps(CFRunLoopRef rl) {
    rl->_perRunData->ignoreWakeUps = 0x57414B45; // 'WAKE'
}

CF_INLINE void __CFRunLoopUnsetIgnoreWakeUps(CFRunLoopRef rl) {
    rl->_perRunData->ignoreWakeUps = 0x0;
}

CF_INLINE Boolean __CFRunLoopIsSleeping(CFRunLoopRef rl) {
    return __CFRuntimeGetFlag(rl, 1);
}

CF_INLINE void __CFRunLoopSetSleeping(CFRunLoopRef rl) {
    __CFRuntimeSetFlag(rl, 1, true);
}

CF_INLINE void __CFRunLoopUnsetSleeping(CFRunLoopRef rl) {
    __CFRuntimeSetFlag(rl, 1, false);
}

CF_INLINE Boolean __CFRunLoopIsDeallocating(CFRunLoopRef rl) {
    return __CFRuntimeGetFlag(rl, 2);
}

CF_INLINE void __CFRunLoopSetDeallocating(CFRunLoopRef rl) {
    __CFRuntimeSetFlag(rl, 2, true);
}

CF_INLINE void __CFRunLoopLock(CFRunLoopRef rl) {
    _CFRecursiveMutexLock(&(((CFRunLoopRef)rl)->_lock));
    //    CFLog(6, CFSTR("__CFRunLoopLock locked %p"), rl);
}

CF_INLINE void __CFRunLoopUnlock(CFRunLoopRef rl) {
    //    CFLog(6, CFSTR("__CFRunLoopLock unlocking %p"), rl);
    _CFRecursiveMutexUnlock(&(((CFRunLoopRef)rl)->_lock));
}

static CFStringRef __CFRunLoopCopyDescription(CFTypeRef cf) {
    CFRunLoopRef rl = (CFRunLoopRef)cf;
    CFMutableStringRef result;
    result = CFStringCreateMutable(kCFAllocatorSystemDefault, 0);
#if TARGET_OS_WIN32
    CFStringAppendFormat(result, NULL, CFSTR("<CFRunLoop %p [%p]>{wakeup port = 0x%x, stopped = %s, ignoreWakeUps = %s, \ncurrent mode = %@,\n"), cf, CFGetAllocator(cf), rl->_wakeUpPort, __CFRunLoopIsStopped(rl) ? "true" : "false", __CFRunLoopIsIgnoringWakeUps(rl) ? "true" : "false", rl->_currentMode ? rl->_currentMode->_name : CFSTR("(none)"));
#else
    CFStringAppendFormat(result, NULL, CFSTR("<CFRunLoop %p [%p]>{wakeup port = 0x%x, stopped = %s, ignoreWakeUps = %s, \ncurrent mode = %@,\n"), cf, CFGetAllocator(cf), rl->_wakeUpPort, __CFRunLoopIsStopped(rl) ? "true" : "false", __CFRunLoopIsIgnoringWakeUps(rl) ? "true" : "false", rl->_currentMode ? rl->_currentMode->_name : CFSTR("(none)"));
#endif
    CFStringAppendFormat(result, NULL, CFSTR("common modes = %@,\ncommon mode items = %@,\nmodes = %@}\n"), rl->_commonModes, rl->_commonModeItems, rl->_modes);
    return result;
}

CF_PRIVATE void __CFRunLoopDump() { // __private_extern__ to keep the compiler from discarding it
    CFStringRef desc = CFCopyDescription(CFRunLoopGetCurrent());
    CFShow(desc);
    CFRelease(desc);
}

/* call with rl locked, returns mode retained */
static CFRunLoopModeRef __CFRunLoopCopyMode(CFRunLoopRef rl, CFStringRef modeName, Boolean create) {
    CHECK_FOR_FORK();
    CFRunLoopModeRef rlm;
    struct __CFRunLoopMode srlm;
    memset(&srlm, 0, sizeof(srlm));
    _CFRuntimeSetInstanceTypeIDAndIsa(&srlm, _kCFRuntimeIDCFRunLoopMode);
    srlm._name = modeName;
    rlm = (CFRunLoopModeRef)CFSetGetValue(rl->_modes, &srlm);
    if (NULL != rlm) {
        CFRetain(rlm);
	return rlm;
    }
    if (!create) {
	return NULL;
    }
    rlm = (CFRunLoopModeRef)_CFRuntimeCreateInstance(kCFAllocatorSystemDefault, _kCFRuntimeIDCFRunLoopMode, sizeof(struct __CFRunLoopMode) - sizeof(CFRuntimeBase), NULL);
    if (NULL == rlm) {
	return NULL;
    }
    _CFRecursiveMutexCreate(&rlm->_lock);
    rlm->_name = CFStringCreateCopy(kCFAllocatorSystemDefault, modeName);
    rlm->_portSet = __CFPortSetAllocate();
    rlm->_timerSoftDeadline = UINT64_MAX;
    rlm->_timerHardDeadline = UINT64_MAX;
    
    kern_return_t ret = KERN_SUCCESS;
#if USE_DISPATCH_SOURCE_FOR_TIMERS
    rlm->_timerFired = false;
    rlm->_queue = _dispatch_runloop_root_queue_create_4CF("Run Loop Mode Queue", 0);
    __CFPort queuePort = _dispatch_runloop_root_queue_get_port_4CF(rlm->_queue);
    if (queuePort == CFPORT_NULL) CRASH("*** Unable to create run loop mode queue port. (%d) ***", -1);
    rlm->_timerSource = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, rlm->_queue);
    
    __block Boolean *timerFiredPointer = &(rlm->_timerFired);
    dispatch_source_set_event_handler(rlm->_timerSource, ^{
        *timerFiredPointer = true;
    });
    
    // Set timer to far out there. The unique leeway makes this timer easy to spot in debug output.
    dispatch_source_set_timer(rlm->_timerSource, DISPATCH_TIME_FOREVER, DISPATCH_TIME_FOREVER, 321);
    dispatch_resume(rlm->_timerSource);
    
    ret = __CFPortSetInsert(queuePort, rlm->_portSet);
    if (KERN_SUCCESS != ret) CRASH("*** Unable to insert timer port into port set. (%d) ***", ret);
#endif
    rlm->_timerPort = CFPORT_NULL;
#if TARGET_OS_BSD
    rlm->_timerPort = mk_timer_create(rlm->_portSet);
#else
    rlm->_timerPort = mk_timer_create();
#endif
    if (rlm->_timerPort == CFPORT_NULL) {
        CRASH("*** Unable to create timer Port (%d) ***", rlm->_timerPort);
    }
    ret = __CFPortSetInsert(rlm->_timerPort, rlm->_portSet);
    if (KERN_SUCCESS != ret) CRASH("*** Unable to insert timer port into port set. (%d) ***", ret);

    ret = __CFPortSetInsert(rl->_wakeUpPort, rlm->_portSet);
    if (KERN_SUCCESS != ret) CRASH("*** Unable to insert wake up port into port set. (%d) ***", ret);
    
#if TARGET_OS_WIN32
    rlm->_msgQMask = 0;
    rlm->_msgPump = NULL;
#endif
    CFSetAddValue(rl->_modes, rlm);
    return rlm;
}


// expects rl and rlm locked
static Boolean __CFRunLoopModeIsEmpty(CFRunLoopRef rl, CFRunLoopModeRef rlm, CFRunLoopModeRef previousMode) {
    CHECK_FOR_FORK();
    if (NULL == rlm) return true;
#if TARGET_OS_WIN32
    if (0 != rlm->_msgQMask) return false;
#endif
#if __HAS_DISPATCH__
    Boolean libdispatchQSafe = pthread_main_np() == 1 && ((HANDLE_DISPATCH_ON_BASE_INVOCATION_ONLY && NULL == previousMode) || (!HANDLE_DISPATCH_ON_BASE_INVOCATION_ONLY && 0 == _CFGetTSD(__CFTSDKeyIsInGCDMainQ)));
    if (libdispatchQSafe && (CFRunLoopGetMain() == rl) && CFSetContainsValue(rl->_commonModes, rlm->_name)) return false; // represents the libdispatch main queue
#endif
    if (NULL != rlm->_sources0 && 0 < CFSetGetCount(rlm->_sources0)) return false;
    if (NULL != rlm->_sources1 && 0 < CFSetGetCount(rlm->_sources1)) return false;
    if (NULL != rlm->_timers && 0 < CFArrayGetCount(rlm->_timers)) return false;
    struct _block_item *item = rl->_blocks_head;
    while (item) {
        struct _block_item *curr = item;
        item = item->_next;
        Boolean doit = false;
        if (_kCFRuntimeIDCFString == CFGetTypeID(curr->_mode)) {
            doit = CFEqual(curr->_mode, rlm->_name) || (CFEqual(curr->_mode, kCFRunLoopCommonModes) && CFSetContainsValue(rl->_commonModes, rlm->_name));
        } else {
            doit = CFSetContainsValue((CFSetRef)curr->_mode, rlm->_name) || (CFSetContainsValue((CFSetRef)curr->_mode, kCFRunLoopCommonModes) && CFSetContainsValue(rl->_commonModes, rlm->_name));
        }
        if (doit) return false;
    }
    return true;
}

#if TARGET_OS_WIN32

uint32_t _CFRunLoopGetWindowsMessageQueueMask(CFRunLoopRef rl, CFStringRef modeName) {
    if (modeName == kCFRunLoopCommonModes) {
	CFLog(kCFLogLevelError, CFSTR("_CFRunLoopGetWindowsMessageQueueMask: kCFRunLoopCommonModes unsupported"));
	HALT;
    }
    DWORD result = 0;
    __CFRunLoopLock(rl);
    CFRunLoopModeRef rlm = __CFRunLoopCopyMode(rl, modeName, false);
    if (rlm) {
        __CFRunLoopModeLock(rlm);
	result = rlm->_msgQMask;
	__CFRunLoopModeUnlock(rlm);
        CFRelease(rlm);
    }
    __CFRunLoopUnlock(rl);
    return (uint32_t)result;
}

void _CFRunLoopSetWindowsMessageQueueMask(CFRunLoopRef rl, uint32_t mask, CFStringRef modeName) {
    if (modeName == kCFRunLoopCommonModes) {
	CFLog(kCFLogLevelError, CFSTR("_CFRunLoopSetWindowsMessageQueueMask: kCFRunLoopCommonModes unsupported"));
	HALT;
    }
    __CFRunLoopLock(rl);
    CFRunLoopModeRef rlm = __CFRunLoopCopyMode(rl, modeName, true);
    __CFRunLoopModeLock(rlm);
    rlm->_msgQMask = (DWORD)mask;
    __CFRunLoopModeUnlock(rlm);
    CFRelease(rlm);
    __CFRunLoopUnlock(rl);
}

uint32_t _CFRunLoopGetWindowsThreadID(CFRunLoopRef rl) {
    return rl->_winthread;
}

CFWindowsMessageQueueHandler _CFRunLoopGetWindowsMessageQueueHandler(CFRunLoopRef rl, CFStringRef modeName) {
    if (modeName == kCFRunLoopCommonModes) {
	CFLog(kCFLogLevelError, CFSTR("_CFRunLoopGetWindowsMessageQueueMask: kCFRunLoopCommonModes unsupported"));
	HALT;
    }
    if (!_CFRunLoopIsCurrent(rl)) {
	CFLog(kCFLogLevelError, CFSTR("_CFRunLoopGetWindowsMessageQueueHandler: run loop parameter must be the current run loop"));
	HALT;
    }
    void (*result)(void) = NULL;
    __CFRunLoopLock(rl);
    CFRunLoopModeRef rlm = __CFRunLoopCopyMode(rl, modeName, false);
    if (rlm) {
        __CFRunLoopModeLock(rlm);
        result = rlm->_msgPump;
	__CFRunLoopModeUnlock(rlm);
        CFRelease(rlm);
    }
    __CFRunLoopUnlock(rl);
    return result;
}

void _CFRunLoopSetWindowsMessageQueueHandler(CFRunLoopRef rl, CFStringRef modeName, CFWindowsMessageQueueHandler func) {
    if (modeName == kCFRunLoopCommonModes) {
	CFLog(kCFLogLevelError, CFSTR("_CFRunLoopGetWindowsMessageQueueMask: kCFRunLoopCommonModes unsupported"));
	HALT;
    }
    if (!_CFRunLoopIsCurrent(rl)) {
	CFLog(kCFLogLevelError, CFSTR("_CFRunLoopGetWindowsMessageQueueHandler: run loop parameter must be the current run loop"));
	HALT;
    }
    __CFRunLoopLock(rl);
    CFRunLoopModeRef rlm = __CFRunLoopCopyMode(rl, modeName, true);
    __CFRunLoopModeLock(rlm);
    rlm->_msgPump = func;
    __CFRunLoopModeUnlock(rlm);
    CFRelease(rlm);
    __CFRunLoopUnlock(rl);
}

#endif

#pragma mark -
#pragma mark Sources

/* Bit 3 in the base reserved bits is used for invalid state in run loop objects */

CF_INLINE Boolean __CFIsValid(const void *cf) {
    return __CFRuntimeGetFlag(cf, 3);
}

CF_INLINE void __CFSetValid(void *cf) {
    __CFRuntimeSetFlag(cf, 3, true);
}

CF_INLINE void __CFUnsetValid(void *cf) {
    __CFRuntimeSetFlag(cf, 3, false);
}

struct __CFRunLoopSource {
    CFRuntimeBase _base;
    _CFRecursiveMutex _lock;
    CFIndex _order;			/* immutable */
    _Atomic uint64_t _signaledTime;
    CFMutableBagRef _runLoops;
    union {
	CFRunLoopSourceContext version0;	/* immutable, except invalidation */
        CFRunLoopSourceContext1 version1;	/* immutable, except invalidation */
    } _context;
};

static uint64_t __CFRunLoopSourceGetSignaledTime(CFRunLoopSourceRef rls) {
    return atomic_load_explicit(&rls->_signaledTime, memory_order_acquire);
}

static Boolean __CFRunLoopSourceIsSignaled(CFRunLoopSourceRef rls) {
    return __CFRunLoopSourceGetSignaledTime(rls) != 0;
}

CF_INLINE void __CFRunLoopSourceSetSignaled(CFRunLoopSourceRef rls) {
    atomic_compare_exchange_strong_explicit(&rls->_signaledTime, &(uint64_t){0}, mach_absolute_time(), memory_order_acq_rel, memory_order_acq_rel);
}

CF_INLINE void __CFRunLoopSourceUnsetSignaled(CFRunLoopSourceRef rls) {
    atomic_store_explicit(&rls->_signaledTime, 0, memory_order_release);
}

CF_INLINE void __CFRunLoopSourceLock(CFRunLoopSourceRef rls) {
    _CFRecursiveMutexLock(&(rls->_lock));
//    CFLog(6, CFSTR("__CFRunLoopSourceLock locked %p"), rls);
}

CF_INLINE void __CFRunLoopSourceUnlock(CFRunLoopSourceRef rls) {
//    CFLog(6, CFSTR("__CFRunLoopSourceLock unlocking %p"), rls);
    _CFRecursiveMutexUnlock(&(rls->_lock));
}

#pragma mark Observers

struct __CFRunLoopObserver {
    CFRuntimeBase _base;
    _CFRecursiveMutex _lock;
    CFRunLoopRef _runLoop;
    CFIndex _rlCount;
    CFOptionFlags _activities;		/* immutable */
    CFIndex _order;			/* immutable */
    CFRunLoopObserverCallBack _callout;	/* immutable */
    CFRunLoopObserverContext _context;	/* immutable, except invalidation */
};

/* Bit 0 of the base reserved bits is used for firing state */
/* Bit 1 of the base reserved bits is used for repeats state */

CF_INLINE Boolean __CFRunLoopObserverIsFiring(CFRunLoopObserverRef rlo) {
    return __CFRuntimeGetFlag(rlo, 0);
}

CF_INLINE void __CFRunLoopObserverSetFiring(CFRunLoopObserverRef rlo) {
    __CFRuntimeSetFlag(rlo, 0, true);
}

CF_INLINE void __CFRunLoopObserverUnsetFiring(CFRunLoopObserverRef rlo) {
    __CFRuntimeSetFlag(rlo, 0, false);
}

CF_INLINE Boolean __CFRunLoopObserverRepeats(CFRunLoopObserverRef rlo) {
    return __CFRuntimeGetFlag(rlo, 1);
}

CF_INLINE void __CFRunLoopObserverSetRepeats(CFRunLoopObserverRef rlo) {
    __CFRuntimeSetFlag(rlo, 1, true);
}

CF_INLINE void __CFRunLoopObserverUnsetRepeats(CFRunLoopObserverRef rlo) {
    __CFRuntimeSetFlag(rlo, 1, false);
}

CF_INLINE void __CFRunLoopObserverLock(CFRunLoopObserverRef rlo) {
    _CFRecursiveMutexLock(&(rlo->_lock));
//    CFLog(6, CFSTR("__CFRunLoopObserverLock locked %p"), rlo);
}

CF_INLINE void __CFRunLoopObserverUnlock(CFRunLoopObserverRef rlo) {
//    CFLog(6, CFSTR("__CFRunLoopObserverLock unlocking %p"), rlo);
    _CFRecursiveMutexUnlock(&(rlo->_lock));
}

static void __CFRunLoopObserverSchedule(CFRunLoopObserverRef rlo, CFRunLoopRef rl, CFRunLoopModeRef rlm) {
    __CFRunLoopObserverLock(rlo);
    if (0 == rlo->_rlCount) {
	rlo->_runLoop = rl;
    }
    rlo->_rlCount++;
    __CFRunLoopObserverUnlock(rlo);
}

static void __CFRunLoopObserverCancel(CFRunLoopObserverRef rlo, CFRunLoopRef rl, CFRunLoopModeRef rlm) {
    __CFRunLoopObserverLock(rlo);
    rlo->_rlCount--;
    if (0 == rlo->_rlCount) {
	rlo->_runLoop = NULL;
    }
    __CFRunLoopObserverUnlock(rlo);
}

#pragma mark Timers

struct __CFRunLoopTimer {
    CFRuntimeBase _base;
    uint16_t _bits;
    _CFRecursiveMutex _lock;
    CFRunLoopRef _runLoop;
    CFMutableSetRef _rlModes;
    CFAbsoluteTime _nextFireDate;
    CFTimeInterval _interval;		/* immutable */
    CFTimeInterval _tolerance;          /* mutable */
    uint64_t _fireTSR;			/* TSR units */
    CFIndex _order;			/* immutable */
    CFRunLoopTimerCallBack _callout;	/* immutable */
    CFRunLoopTimerContext _context;	/* immutable, except invalidation */
};

/* Bit 0 of the base reserved bits is used for firing state */
/* Bit 1 of the base reserved bits is used for fired-during-callout state */
/* Bit 2 of the base reserved bits is used for waking state */

CF_INLINE Boolean __CFRunLoopTimerIsFiring(CFRunLoopTimerRef rlt) {
    return (Boolean)__CFBitfieldGetValue(rlt->_bits, 0, 0);
}

CF_INLINE void __CFRunLoopTimerSetFiring(CFRunLoopTimerRef rlt) {
    __CFBitfieldSetValue(rlt->_bits, 0, 0, 1);
}

CF_INLINE void __CFRunLoopTimerUnsetFiring(CFRunLoopTimerRef rlt) {
    __CFBitfieldSetValue(rlt->_bits, 0, 0, 0);
}

CF_INLINE Boolean __CFRunLoopTimerIsDeallocating(CFRunLoopTimerRef rlt) {
    return (Boolean)__CFBitfieldGetValue(rlt->_bits, 2, 2);
}

CF_INLINE void __CFRunLoopTimerSetDeallocating(CFRunLoopTimerRef rlt) {
    __CFBitfieldSetValue(rlt->_bits, 2, 2, 1);
}

CF_INLINE void __CFRunLoopTimerLock(CFRunLoopTimerRef rlt) {
    _CFRecursiveMutexLock(&(rlt->_lock));
//    CFLog(6, CFSTR("__CFRunLoopTimerLock locked %p"), rlt);
}

CF_INLINE void __CFRunLoopTimerUnlock(CFRunLoopTimerRef rlt) {
//    CFLog(6, CFSTR("__CFRunLoopTimerLock unlocking %p"), rlt);
    _CFRecursiveMutexUnlock(&(rlt->_lock));
}


#pragma mark -

/* CFRunLoop */

CONST_STRING_DECL(kCFRunLoopDefaultMode, "kCFRunLoopDefaultMode")
CONST_STRING_DECL(kCFRunLoopCommonModes, "kCFRunLoopCommonModes")

// call with rl and rlm locked
static CFRunLoopSourceRef __CFRunLoopModeFindSourceForMachPort(CFRunLoopRef rl, CFRunLoopModeRef rlm, __CFPort port) {	/* DOES CALLOUT */
    CHECK_FOR_FORK();
    CFRunLoopSourceRef found = rlm->_portToV1SourceMap ? (CFRunLoopSourceRef)CFDictionaryGetValue(rlm->_portToV1SourceMap, (const void *)(uintptr_t)port) : NULL;
    return found;
}

// Remove backreferences the mode's sources have to the rl (context);
// the primary purpose of rls->_runLoops is so that Invalidation can remove
// the source from the run loops it is in, but during deallocation of a
// run loop, we already know that the sources are going to be punted
// from it, so invalidation of sources does not need to remove from a
// deallocating run loop.
static void __CFRunLoopCleanseSources(const void *value, void *context) {
    CFRunLoopModeRef rlm = (CFRunLoopModeRef)value;
    CFRunLoopRef rl = (CFRunLoopRef)context;
    CFIndex idx, cnt;
    const void **list, *buffer[256];
    if (NULL == rlm->_sources0 && NULL == rlm->_sources1) return;

    cnt = (rlm->_sources0 ? CFSetGetCount(rlm->_sources0) : 0) + (rlm->_sources1 ? CFSetGetCount(rlm->_sources1) : 0);
    list = (const void **)((cnt <= 256) ? buffer : CFAllocatorAllocate(kCFAllocatorSystemDefault, cnt * sizeof(void *), 0));
    if (rlm->_sources0) CFSetGetValues(rlm->_sources0, list);
    if (rlm->_sources1) CFSetGetValues(rlm->_sources1, list + (rlm->_sources0 ? CFSetGetCount(rlm->_sources0) : 0));
    for (idx = 0; idx < cnt; idx++) {
	CFRunLoopSourceRef rls = (CFRunLoopSourceRef)list[idx];
	__CFRunLoopSourceLock(rls);
	if (NULL != rls->_runLoops) {
	    CFBagRemoveValue(rls->_runLoops, rl);
	}
	__CFRunLoopSourceUnlock(rls);
    }
    if (list != buffer) CFAllocatorDeallocate(kCFAllocatorSystemDefault, list);
}

static void __CFRunLoopDeallocateSources(const void *value, void *context) {
    CFRunLoopModeRef rlm = (CFRunLoopModeRef)value;
    CFRunLoopRef rl = (CFRunLoopRef)context;
    CFIndex idx, cnt;
    const void **list, *buffer[256];
    if (NULL == rlm->_sources0 && NULL == rlm->_sources1) return;

    cnt = (rlm->_sources0 ? CFSetGetCount(rlm->_sources0) : 0) + (rlm->_sources1 ? CFSetGetCount(rlm->_sources1) : 0);
    list = (const void **)((cnt <= 256) ? buffer : CFAllocatorAllocate(kCFAllocatorSystemDefault, cnt * sizeof(void *), 0));
    if (rlm->_sources0) CFSetGetValues(rlm->_sources0, list);
    if (rlm->_sources1) CFSetGetValues(rlm->_sources1, list + (rlm->_sources0 ? CFSetGetCount(rlm->_sources0) : 0));
    for (idx = 0; idx < cnt; idx++) {
	CFRetain(list[idx]);
    }
    if (rlm->_sources0) CFSetRemoveAllValues(rlm->_sources0);
    if (rlm->_sources1) CFSetRemoveAllValues(rlm->_sources1);
    for (idx = 0; idx < cnt; idx++) {
        CFRunLoopSourceRef rls = (CFRunLoopSourceRef)list[idx];
        __CFRunLoopSourceLock(rls);
        if (NULL != rls->_runLoops) {
            CFBagRemoveValue(rls->_runLoops, rl);
        }
        __CFRunLoopSourceUnlock(rls);
        if (0 == rls->_context.version0.version) {
            if (NULL != rls->_context.version0.cancel) {
                rls->_context.version0.cancel(rls->_context.version0.info, rl, rlm->_name);	/* CALLOUT */
            }
        } else if (1 == rls->_context.version0.version) {
            __CFPort port = rls->_context.version1.getPort(rls->_context.version1.info);	/* CALLOUT */
            if (CFPORT_NULL != port) {
                __CFPortSetRemove(port, rlm->_portSet);
            }
        }
	CFRelease(rls);
    }
    if (list != buffer) CFAllocatorDeallocate(kCFAllocatorSystemDefault, list);
}

static void __CFRunLoopDeallocateObservers(const void *value, void *context) {
    CFRunLoopModeRef rlm = (CFRunLoopModeRef)value;
    CFRunLoopRef rl = (CFRunLoopRef)context;
    CFIndex idx, cnt;
    const void **list, *buffer[256];
    if (NULL == rlm->_observers) return;
    cnt = CFArrayGetCount(rlm->_observers);
    list = (const void **)((cnt <= 256) ? buffer : CFAllocatorAllocate(kCFAllocatorSystemDefault, cnt * sizeof(void *), 0));
    CFArrayGetValues(rlm->_observers, CFRangeMake(0, cnt), list);
    for (idx = 0; idx < cnt; idx++) {
	CFRetain(list[idx]);
    }
    CFArrayRemoveAllValues(rlm->_observers);
    for (idx = 0; idx < cnt; idx++) {
	__CFRunLoopObserverCancel((CFRunLoopObserverRef)list[idx], rl, rlm);
	CFRelease(list[idx]);
    }
    if (list != buffer) CFAllocatorDeallocate(kCFAllocatorSystemDefault, list);
}

static void __CFRunLoopKillOneTimer(const void *value, void *context) {
    CFRunLoopTimerRef rlt = (CFRunLoopTimerRef)value;
    __CFRunLoopTimerLock(rlt);
    // if the run loop is deallocating, and since a timer can only be in one
    // run loop, we're going to be removing the timer from all modes, so be
    // a little heavy-handed and direct
    CFSetRemoveAllValues(rlt->_rlModes);
    rlt->_runLoop = NULL;
    __CFRunLoopTimerUnlock(rlt);
}

static void __CFRunLoopDeallocateTimers(const void *value, void *context) {
    CFRunLoopModeRef rlm = (CFRunLoopModeRef)value;
    if (NULL == rlm->_timers) return;
    
    const CFRange range = CFRangeMake(0, CFArrayGetCount(rlm->_timers));
    if (range.length) {
        CFArrayApplyFunction(rlm->_timers, range, __CFRunLoopKillOneTimer, context);
        CFArrayRemoveAllValues(rlm->_timers);
    }
}

CF_EXPORT CFRunLoopRef _CFRunLoopGet0b(_CFThreadRef t);


static void __CFRunLoopDeallocate(CFTypeRef cf) {
    CFRunLoopRef rl = (CFRunLoopRef)cf;

    if (rl->_fromTSD == 0) {
        CRSetCrashLogMessage("Attempting to deallocate CFRunLoop outside of thread destructor -- this is likely an over-release of the run loop");
        HALT;
    }
    
    cf_trace(KDEBUG_EVENT_CFRL_LIFETIME|DBG_FUNC_END, rl, NULL, NULL, NULL);

    // unsetting this here to possibly try to catch multiple concurrent __CFRunLoopDeallocate calls (never seen wild)
    rl->_fromTSD = 0;

    if (_CFRunLoopGet0b(pthread_main_thread_np()) == cf) HALT;

    /* We try to keep the run loop in a valid state as long as possible,
       since sources may have non-retained references to the run loop.
       Another reason is that we don't want to lock the run loop for
       callback reasons, if we can get away without that.  We start by
       eliminating the sources, since they are the most likely to call
       back into the run loop during their "cancellation". Common mode
       items will be removed from the mode indirectly by the following
       three lines. */
    __CFRunLoopSetDeallocating(rl);
    if (NULL != rl->_modes) {
	CFSetApplyFunction(rl->_modes, (__CFRunLoopCleanseSources), rl); // remove references to rl
	CFSetApplyFunction(rl->_modes, (__CFRunLoopDeallocateSources), rl);
	CFSetApplyFunction(rl->_modes, (__CFRunLoopDeallocateObservers), rl);
	CFSetApplyFunction(rl->_modes, (__CFRunLoopDeallocateTimers), rl);
    }
    __CFRunLoopLock(rl);
    struct _block_item *item = rl->_blocks_head;
    while (item) {
	struct _block_item *curr = item;
	item = item->_next;
	CFRelease(curr->_mode);
	Block_release(curr->_block);
	free(curr);
    }
    if (NULL != rl->_commonModeItems) {
	CFRelease(rl->_commonModeItems);
    }
    if (NULL != rl->_commonModes) {
	CFRelease(rl->_commonModes);
    }
    if (NULL != rl->_modes) {
	CFRelease(rl->_modes);
    }
    __CFPortFree(rl->_wakeUpPort, (uintptr_t)rl);
    rl->_wakeUpPort = CFPORT_NULL;
    __CFRunLoopPopPerRunData(rl, NULL);
    __CFRunLoopUnlock(rl);
    _CFRecursiveMutexDestroy(&rl->_lock);
    memset((char *)cf + sizeof(CFRuntimeBase), 0x8C, sizeof(struct __CFRunLoop) - sizeof(CFRuntimeBase));
}

const CFRuntimeClass __CFRunLoopModeClass = {
    0,
    "CFRunLoopMode",
    NULL,      // init
    NULL,      // copy
    __CFRunLoopModeDeallocate,
    __CFRunLoopModeEqual,
    __CFRunLoopModeHash,
    NULL,      // 
    __CFRunLoopModeCopyDescription
};

const CFRuntimeClass __CFRunLoopClass = {
    0,
    "CFRunLoop",
    NULL,      // init
    NULL,      // copy
    __CFRunLoopDeallocate,
    NULL,
    NULL,
    NULL,      // 
    __CFRunLoopCopyDescription
};

CF_PRIVATE void __CFFinalizeRunLoop(uintptr_t data);

CFTypeID CFRunLoopGetTypeID(void) {
    return _kCFRuntimeIDCFRunLoop;
}

static CFRunLoopRef __CFRunLoopCreate(_CFThreadRef t) {
    CFRunLoopRef loop = NULL;
    CFRunLoopModeRef rlm;
    uint32_t size = sizeof(struct __CFRunLoop) - sizeof(CFRuntimeBase);
    loop = (CFRunLoopRef)_CFRuntimeCreateInstance(kCFAllocatorSystemDefault, CFRunLoopGetTypeID(), size, NULL);
    if (NULL == loop) {
	return NULL;
    }
    (void)__CFRunLoopPushPerRunData(loop);
    _CFRecursiveMutexCreate(&loop->_lock);
    loop->_wakeUpPort = __CFPortAllocate((uintptr_t)loop);
    if (CFPORT_NULL == loop->_wakeUpPort) HALT;
    __CFRunLoopSetIgnoreWakeUps(loop);
    loop->_commonModes = CFSetCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeSetCallBacks);
    CFSetAddValue(loop->_commonModes, kCFRunLoopDefaultMode);
    loop->_modes = CFSetCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeSetCallBacks);
    loop->_pthread = t;
    loop->_timerTSRLock = CFLockInit;
    loop->_perCalloutARP = true;
#if TARGET_OS_WIN32
    loop->_winthread = GetCurrentThreadId();
#endif
    rlm = __CFRunLoopCopyMode(loop, kCFRunLoopDefaultMode, true);
    if (NULL != rlm) {
        CFRelease(rlm);
    }
    return loop;
}

static CFMutableDictionaryRef __CFRunLoops = NULL;
static CFLock_t loopsLock = CFLockInit;

CF_PRIVATE CFRunLoopRef _CFRunLoopCacheLookup(_CFThreadRef t, const Boolean createCache) {
    CFRunLoopRef loop = NULL;
    if (pthread_equal(t, kNilPthreadT)) {
        t = pthread_main_thread_np();
    }
    __CFLock(&loopsLock);
    if (!__CFRunLoops && createCache) {
        __CFUnlock(&loopsLock);
        CFMutableDictionaryRef dict = CFDictionaryCreateMutable(kCFAllocatorSystemDefault, 0, NULL, &kCFTypeDictionaryValueCallBacks);
        CFRunLoopRef mainLoop = __CFRunLoopCreate(pthread_main_thread_np());
        CFDictionarySetValue(dict, pthreadPointer(pthread_main_thread_np()), mainLoop);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated"
        if (!OSAtomicCompareAndSwapPtrBarrier(NULL, dict, (void * volatile *)&__CFRunLoops)) {
#pragma GCC diagnostic pop
            CFRelease(dict);
        }
        CFRelease(mainLoop);
        __CFLock(&loopsLock);
    }
    if (__CFRunLoops) {
        loop = (CFRunLoopRef)CFDictionaryGetValue(__CFRunLoops, pthreadPointer(t));
    }
    __CFUnlock(&loopsLock);
    return loop;
}

CF_PRIVATE CFRunLoopRef _CFRunLoopGetButDontCreateCurrent(void) CF_RETURNS_NOT_RETAINED {
    CFRunLoopRef loop = (CFRunLoopRef)_CFGetTSDCreateIfNeeded(__CFTSDKeyRunLoop, false);
    if (loop == NULL) {
        loop = _CFRunLoopCacheLookup(pthread_self(), false);
    }
    return loop;
}

// should only be called by Foundation
CF_EXPORT Boolean _CFRunLoopIsCurrent(const CFRunLoopRef rl) {
    Boolean result = false;
    if (rl) {
        const CFRunLoopRef loop = _CFRunLoopGetButDontCreateCurrent();
        result = (rl == loop);
    }
    return result;
}

// should only be called by Foundation
// t==0 is a synonym for "main thread" that always works
CF_EXPORT CFRunLoopRef _CFRunLoopGet0(_CFThreadRef t) {
    if (pthread_equal(t, kNilPthreadT)) {
	t = pthread_main_thread_np();
    }
    __CFLock(&loopsLock);
    if (!__CFRunLoops) {
	CFMutableDictionaryRef dict = CFDictionaryCreateMutable(kCFAllocatorSystemDefault, 0, NULL, &kCFTypeDictionaryValueCallBacks);
	CFRunLoopRef mainLoop = __CFRunLoopCreate(pthread_main_thread_np());
	CFDictionarySetValue(dict, pthreadPointer(pthread_main_thread_np()), mainLoop);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated"
	if (!OSAtomicCompareAndSwapPtrBarrier(NULL, dict, (void * volatile *)&__CFRunLoops)) {
#pragma GCC diagnostic pop
	    CFRelease(dict);
	}
	CFRelease(mainLoop);
    }
    CFRunLoopRef newLoop = NULL;
    CFRunLoopRef loop = (CFRunLoopRef)CFDictionaryGetValue(__CFRunLoops, pthreadPointer(t));
    if (!loop) {
	newLoop = __CFRunLoopCreate(t);
        
        cf_trace(KDEBUG_EVENT_CFRL_LIFETIME|DBG_FUNC_START, newLoop, NULL, NULL, NULL);
        CFDictionarySetValue(__CFRunLoops, pthreadPointer(t), newLoop);
        loop = newLoop;
    }
    __CFUnlock(&loopsLock);
    // don't release run loops inside the loopsLock, because CFRunLoopDeallocate may end up taking it
    if (newLoop) { CFRelease(newLoop); }
    
    if (pthread_equal(t, pthread_self())) {
        _CFSetTSD(__CFTSDKeyRunLoop, (void *)loop, NULL);
        if (0 == _CFGetTSD(__CFTSDKeyRunLoopCntr)) {
#if _POSIX_THREADS
            _CFSetTSD(__CFTSDKeyRunLoopCntr, (void *)(PTHREAD_DESTRUCTOR_ITERATIONS-1), (void (*)(void *))__CFFinalizeRunLoop);
#else
            _CFSetTSD(__CFTSDKeyRunLoopCntr, 0, &__CFFinalizeRunLoop);
#endif
        }
    }
    return loop;
}

// should only be called by Foundation
CFRunLoopRef _CFRunLoopGet0b(_CFThreadRef t) {
    if (pthread_equal(t, kNilPthreadT)) {
	t = pthread_main_thread_np();
    }
    __CFLock(&loopsLock);
    CFRunLoopRef loop = NULL;
    if (__CFRunLoops) {
        loop = (CFRunLoopRef)CFDictionaryGetValue(__CFRunLoops, pthreadPointer(t));
    }
    __CFUnlock(&loopsLock);
    return loop;
}

static void __CFRunLoopRemoveAllSources(CFRunLoopRef rl, CFStringRef modeName);

// Called for each thread as it exits
CF_PRIVATE void __CFFinalizeRunLoop(uintptr_t data) {
    CFRunLoopRef rl = NULL;
    if (data <= 1) {
	__CFLock(&loopsLock);
	if (__CFRunLoops) {
	    rl = (CFRunLoopRef)CFDictionaryGetValue(__CFRunLoops, pthreadPointer(pthread_self()));
	    if (rl) CFRetain(rl);
	    CFDictionaryRemoveValue(__CFRunLoops, pthreadPointer(pthread_self()));
	}
	__CFUnlock(&loopsLock);
    } else {
        _CFSetTSD(__CFTSDKeyRunLoopCntr, (void *)(data - 1), (void (*)(void *))__CFFinalizeRunLoop);
    }
    if (rl && _CFRunLoopGet0b(pthread_main_thread_np()) != rl) { // protect against cooperative threads
        if (NULL != rl->_counterpart) {
#if DEPLOYMENT_RUNTIME_SWIFT
            extern void swift_release(void *);
            swift_release((void *)rl->_counterpart);
#else
            CFRelease(rl->_counterpart);
#endif
	    rl->_counterpart = NULL;
        }
	// purge all sources before deallocation
        CFArrayRef array = CFRunLoopCopyAllModes(rl);
        for (CFIndex idx = CFArrayGetCount(array); idx--;) {
            CFStringRef modeName = (CFStringRef)CFArrayGetValueAtIndex(array, idx);
            __CFRunLoopRemoveAllSources(rl, modeName);
        }
        __CFRunLoopRemoveAllSources(rl, kCFRunLoopCommonModes);
        CFRelease(array);
    }
    if (rl) {
        rl->_fromTSD = 1;
        CFRelease(rl);
    }
}

_CFThreadRef _CFRunLoopGet1(CFRunLoopRef rl) {
    return rl->_pthread;
}

// should only be called by Foundation
CF_EXPORT CFTypeRef _CFRunLoopGet2(CFRunLoopRef rl) {
    CFTypeRef ret = NULL;
    __CFLock(&loopsLock);
#if DEPLOYMENT_RUNTIME_SWIFT
    if (rl->_counterpart == NULL) {
        CFTypeRef ns = __CFSwiftBridge.NSRunLoop._new(rl); // returns retained so we will claim ownership of that return value by just assigning (the release is balanced in the destruction of the CFRunLoop
        rl->_counterpart = ns;
    }
#endif
    ret = rl->_counterpart;
    __CFUnlock(&loopsLock);
    return ret;
}

// should only be called by Foundation
CF_EXPORT CFTypeRef _CFRunLoopGet2b(CFRunLoopRef rl) {
    return rl->_counterpart;
}

#if TARGET_OS_OSX
void _CFRunLoopSetCurrent(CFRunLoopRef rl) {
    if (pthread_main_np() == 1) return;
    CFRunLoopRef currentLoop = _CFRunLoopGetButDontCreateCurrent();
    if (rl != currentLoop) {
        if (currentLoop) {
            CFRetain(currentLoop); // avoid a deallocation of the currentLoop inside the lock
        }
        __CFLock(&loopsLock);
	if (rl) {
	    CFDictionarySetValue(__CFRunLoops, pthreadPointer(pthread_self()), rl);
	} else {
	    CFDictionaryRemoveValue(__CFRunLoops, pthreadPointer(pthread_self()));
	}
        __CFUnlock(&loopsLock);
        if (currentLoop) {
            CFRelease(currentLoop);
        }
        _CFSetTSD(__CFTSDKeyRunLoop, NULL, NULL);
        _CFSetTSD(__CFTSDKeyRunLoopCntr, 0, (void (*)(void *))__CFFinalizeRunLoop);
    }
}
#endif

CFRunLoopRef CFRunLoopGetMain(void) {
    CHECK_FOR_FORK();
    static CFRunLoopRef __main = NULL; // no retain needed
    if (!__main) __main = _CFRunLoopGet0(pthread_main_thread_np()); // no CAS needed
    return __main;
}


CFRunLoopRef CFRunLoopGetCurrent(void) {
    CHECK_FOR_FORK();
    CFRunLoopRef rl = (CFRunLoopRef)_CFGetTSD(__CFTSDKeyRunLoop);
    if (rl) return rl;
    return _CFRunLoopGet0(pthread_self());
}

CFStringRef CFRunLoopCopyCurrentMode(CFRunLoopRef rl) {
    CF_ASSERT_TYPE(_kCFRuntimeIDCFRunLoop, rl);
    CHECK_FOR_FORK();
    CFStringRef result = NULL;
    __CFRunLoopLock(rl);
    if (NULL != rl->_currentMode) {
	result = (CFStringRef)CFRetain(rl->_currentMode->_name);
    }
    __CFRunLoopUnlock(rl);
    return result;
}

static void __CFRunLoopGetModeName(const void *value, void *context) {
    CFRunLoopModeRef rlm = (CFRunLoopModeRef)value;
    CFMutableArrayRef array = (CFMutableArrayRef)context;
    CFArrayAppendValue(array, rlm->_name);
}

CFArrayRef CFRunLoopCopyAllModes(CFRunLoopRef rl) {
    CF_ASSERT_TYPE(_kCFRuntimeIDCFRunLoop, rl);
    CHECK_FOR_FORK();
    CFMutableArrayRef array;
    __CFRunLoopLock(rl);
    array = CFArrayCreateMutable(kCFAllocatorSystemDefault, CFSetGetCount(rl->_modes), &kCFTypeArrayCallBacks);
    CFSetApplyFunction(rl->_modes, (__CFRunLoopGetModeName), array);
    __CFRunLoopUnlock(rl);
    return array;
}

static void __CFRunLoopAddItemsToCommonMode(const void *value, void *ctx) {
    CFTypeRef item = (CFTypeRef)value;
    CFRunLoopRef rl = (CFRunLoopRef)(((CFTypeRef *)ctx)[0]);
    CFStringRef modeName = (CFStringRef)(((CFTypeRef *)ctx)[1]);
    CFTypeID const itemID = CFGetTypeID(item);
    if (itemID == CFRunLoopSourceGetTypeID()) {
	CFRunLoopAddSource(rl, (CFRunLoopSourceRef)item, modeName);
    } else if (itemID == CFRunLoopObserverGetTypeID()) {
	CFRunLoopAddObserver(rl, (CFRunLoopObserverRef)item, modeName);
    } else if (itemID == CFRunLoopTimerGetTypeID()) {
	CFRunLoopAddTimer(rl, (CFRunLoopTimerRef)item, modeName);
    }
}

static void __CFRunLoopAddItemToCommonModes(const void *value, void *ctx) {
    CFStringRef modeName = (CFStringRef)value;
    CFRunLoopRef rl = (CFRunLoopRef)(((CFTypeRef *)ctx)[0]);
    CFTypeRef item = (CFTypeRef)(((CFTypeRef *)ctx)[1]);
    CFTypeID const itemID = CFGetTypeID(item);
    if (itemID == CFRunLoopSourceGetTypeID()) {
	CFRunLoopAddSource(rl, (CFRunLoopSourceRef)item, modeName);
    } else if (itemID == CFRunLoopObserverGetTypeID()) {
	CFRunLoopAddObserver(rl, (CFRunLoopObserverRef)item, modeName);
    } else if (itemID == CFRunLoopTimerGetTypeID()) {
	CFRunLoopAddTimer(rl, (CFRunLoopTimerRef)item, modeName);
    }
}

static void __CFRunLoopRemoveItemFromCommonModes(const void *value, void *ctx) {
    CFStringRef modeName = (CFStringRef)value;
    CFRunLoopRef rl = (CFRunLoopRef)(((CFTypeRef *)ctx)[0]);
    CFTypeRef item = (CFTypeRef)(((CFTypeRef *)ctx)[1]);
    CFTypeID const itemID = CFGetTypeID(item);
    if (itemID == CFRunLoopSourceGetTypeID()) {
	CFRunLoopRemoveSource(rl, (CFRunLoopSourceRef)item, modeName);
    } else if (itemID == CFRunLoopObserverGetTypeID()) {
	CFRunLoopRemoveObserver(rl, (CFRunLoopObserverRef)item, modeName);
    } else if (itemID == CFRunLoopTimerGetTypeID()) {
	CFRunLoopRemoveTimer(rl, (CFRunLoopTimerRef)item, modeName);
    }
}

CF_EXPORT Boolean _CFRunLoop01(CFRunLoopRef rl, CFStringRef modeName) {
    __CFRunLoopLock(rl);
    Boolean present = CFSetContainsValue(rl->_commonModes, modeName);
    __CFRunLoopUnlock(rl);
    return present; 
}

void CFRunLoopAddCommonMode(CFRunLoopRef rl, CFStringRef modeName) {
    CF_ASSERT_TYPE(_kCFRuntimeIDCFRunLoop, rl);
    CHECK_FOR_FORK();
    if (__CFRunLoopIsDeallocating(rl)) return;
    __CFRunLoopLock(rl);
    if (!CFSetContainsValue(rl->_commonModes, modeName)) {
	CFSetRef set = rl->_commonModeItems ? CFSetCreateCopy(kCFAllocatorSystemDefault, rl->_commonModeItems) : NULL;
	CFSetAddValue(rl->_commonModes, modeName);
	if (NULL != set) {
	    CFTypeRef context[2] = {rl, modeName};
	    /* add all common-modes items to new mode */
	    CFSetApplyFunction(set, (__CFRunLoopAddItemsToCommonMode), (void *)context);
	    CFRelease(set);
	}
    } else {
    }
    __CFRunLoopUnlock(rl);
}

#if __HAS_DISPATCH__

static void __CFRUNLOOP_IS_SERVICING_THE_MAIN_DISPATCH_QUEUE__(void *) __attribute__((noinline));
static void __CFRUNLOOP_IS_SERVICING_THE_MAIN_DISPATCH_QUEUE__(void *msg) {
    _dispatch_main_queue_callback_4CF(msg);
    __asm __volatile__(""); // thwart tail-call optimization
}

#endif

static void __CFRUNLOOP_IS_CALLING_OUT_TO_AN_OBSERVER_CALLBACK_FUNCTION__(CFRunLoopObserverCallBack, CFRunLoopObserverRef, CFRunLoopActivity, void *) __attribute__((noinline));
static void __CFRUNLOOP_IS_CALLING_OUT_TO_AN_OBSERVER_CALLBACK_FUNCTION__(CFRunLoopObserverCallBack func, CFRunLoopObserverRef observer, CFRunLoopActivity activity, void *info) {
    if (func) {
        func(observer, activity, info);
    }
    __asm __volatile__(""); // thwart tail-call optimization
}

static void __CFRUNLOOP_IS_CALLING_OUT_TO_A_TIMER_CALLBACK_FUNCTION__(CFRunLoopTimerCallBack, CFRunLoopTimerRef, void *) __attribute__((noinline));
static void __CFRUNLOOP_IS_CALLING_OUT_TO_A_TIMER_CALLBACK_FUNCTION__(CFRunLoopTimerCallBack func, CFRunLoopTimerRef timer, void *info) {
    if (func) {
        func(timer, info);
    }
    __asm __volatile__(""); // thwart tail-call optimization
}

static void __CFRUNLOOP_IS_CALLING_OUT_TO_A_BLOCK__(void (^)(void)) __attribute__((noinline));
static void __CFRUNLOOP_IS_CALLING_OUT_TO_A_BLOCK__(void (^block)(void)) {
    if (block) {
        block();
    }
    __asm __volatile__(""); // thwart tail-call optimization
}

static Boolean __CFRunLoopDoBlocks(CFRunLoopRef rl, CFRunLoopModeRef rlm) { // Call with rl and rlm locked
    
    cf_trace(KDEBUG_EVENT_CFRL_IS_DOING_BLOCKS | DBG_FUNC_START, rl, rlm, 0, 0);
    
    if (!rl->_blocks_head) return false;
    if (!rlm || !rlm->_name) return false;
    Boolean did = false;
    struct _block_item *head = rl->_blocks_head;
    struct _block_item *tail = rl->_blocks_tail;
    rl->_blocks_head = NULL;
    rl->_blocks_tail = NULL;
    CFSetRef commonModes = rl->_commonModes;
    CFStringRef curMode = rlm->_name;
    __CFRunLoopModeUnlock(rlm);
    __CFRunLoopUnlock(rl);
    struct _block_item *prev = NULL;
    struct _block_item *item = head;
    while (item) {
        struct _block_item *curr = item;
        item = item->_next;
	Boolean doit = false;
	if (_kCFRuntimeIDCFString == CFGetTypeID(curr->_mode)) {
	    doit = CFEqual(curr->_mode, curMode) || (CFEqual(curr->_mode, kCFRunLoopCommonModes) && CFSetContainsValue(commonModes, curMode));
        } else {
	    doit = CFSetContainsValue((CFSetRef)curr->_mode, curMode) || (CFSetContainsValue((CFSetRef)curr->_mode, kCFRunLoopCommonModes) && CFSetContainsValue(commonModes, curMode));
	}
	if (!doit) prev = curr;
	if (doit) {
	    if (prev) prev->_next = item;
	    if (curr == head) head = item;
	    if (curr == tail) tail = prev;
	    void (^block)(void) = curr->_block;
            CFRelease(curr->_mode);
            free(curr);
	    if (doit) {
                CFRUNLOOP_ARP_BEGIN(rl);
                cf_trace(KDEBUG_EVENT_CFRL_IS_CALLING_BLOCK | DBG_FUNC_START, rl, rlm, block, 0);
                __CFRUNLOOP_IS_CALLING_OUT_TO_A_BLOCK__(block);
                cf_trace(KDEBUG_EVENT_CFRL_IS_CALLING_BLOCK | DBG_FUNC_END, rl, rlm, block, 0);
                CFRUNLOOP_ARP_END();
	        did = true;
	    }
            Block_release(block); // do this before relocking to prevent deadlocks where some yahoo wants to run the run loop reentrantly from their dealloc
	}
    }
    __CFRunLoopLock(rl);
    __CFRunLoopModeLock(rlm);
    if (head && tail) {
	tail->_next = rl->_blocks_head;
	rl->_blocks_head = head;
        if (!rl->_blocks_tail) rl->_blocks_tail = tail;
    }
    
    cf_trace(KDEBUG_EVENT_CFRL_IS_DOING_BLOCKS | DBG_FUNC_END, rl, rlm, 0, 0);
    
    return did;
}

/* rl is locked, rlm is locked on entrance and exit */
static void __CFRunLoopDoObservers(CFRunLoopRef, CFRunLoopModeRef, CFRunLoopActivity) __attribute__((noinline));
static void __CFRunLoopDoObservers(CFRunLoopRef rl, CFRunLoopModeRef rlm, CFRunLoopActivity activity) {	/* DOES CALLOUT */
    
    cf_trace(KDEBUG_EVENT_CFRL_IS_DOING_OBSERVERS | DBG_FUNC_START, rl, rlm, activity, 0);
    
    CHECK_FOR_FORK();

    CFIndex cnt = rlm->_observers ? CFArrayGetCount(rlm->_observers) : 0;
    if (cnt < 1) return;

    /* Fire the observers */
    STACK_BUFFER_DECL(CFRunLoopObserverRef, buffer, (cnt <= 1024) ? cnt : 1);
    CFRunLoopObserverRef *collectedObservers = (cnt <= 1024) ? buffer : (CFRunLoopObserverRef *)malloc(cnt * sizeof(CFRunLoopObserverRef));
    CFIndex obs_cnt = 0;
    for (CFIndex idx = 0; idx < cnt; idx++) {
        CFRunLoopObserverRef rlo = (CFRunLoopObserverRef)CFArrayGetValueAtIndex(rlm->_observers, idx);
        if (0 != (rlo->_activities & activity) && __CFIsValid(rlo) && !__CFRunLoopObserverIsFiring(rlo)) {
            collectedObservers[obs_cnt++] = (CFRunLoopObserverRef)CFRetain(rlo);
        }
    }
    __CFRunLoopModeUnlock(rlm);
    __CFRunLoopUnlock(rl);
    for (CFIndex idx = 0; idx < obs_cnt; idx++) {
        CFRunLoopObserverRef rlo = collectedObservers[idx];
        __CFRunLoopObserverLock(rlo);
        if (__CFIsValid(rlo)) {
            Boolean doInvalidate = !__CFRunLoopObserverRepeats(rlo);
            __CFRunLoopObserverSetFiring(rlo);
            __CFRunLoopObserverUnlock(rlo);
            CFRunLoopObserverCallBack callout = rlo->_callout;
            void *info = rlo->_context.info;
            CFRUNLOOP_ARP_BEGIN(rl)
            cf_trace(KDEBUG_EVENT_CFRL_IS_CALLING_OBSERVER | DBG_FUNC_START, callout, rlo, activity, info);
            __CFRUNLOOP_IS_CALLING_OUT_TO_AN_OBSERVER_CALLBACK_FUNCTION__(callout, rlo, activity, info);
            cf_trace(KDEBUG_EVENT_CFRL_IS_CALLING_OBSERVER | DBG_FUNC_END, callout, rlo, activity, info);
            CFRUNLOOP_ARP_END()
            if (doInvalidate) {
                CFRunLoopObserverInvalidate(rlo);
            }
            __CFRunLoopObserverUnsetFiring(rlo);
        } else {
            __CFRunLoopObserverUnlock(rlo);
        }
        CFRelease(rlo);
    }
    __CFRunLoopLock(rl);
    __CFRunLoopModeLock(rlm);

    if (collectedObservers != buffer) free(collectedObservers);
    
    cf_trace(KDEBUG_EVENT_CFRL_IS_DOING_OBSERVERS | DBG_FUNC_END, rl, rlm, activity, 0);
}

static CFComparisonResult __CFRunLoopSourceComparator(const void *val1, const void *val2, void *context) {
    CFRunLoopSourceRef o1 = (CFRunLoopSourceRef)val1;
    CFRunLoopSourceRef o2 = (CFRunLoopSourceRef)val2;
    if (o1->_order < o2->_order) return kCFCompareLessThan;
    if (o2->_order < o1->_order) return kCFCompareGreaterThan;
    
    const CFAbsoluteTime time1 = __CFRunLoopSourceGetSignaledTime(o1);
    const CFAbsoluteTime time2 = __CFRunLoopSourceGetSignaledTime(o2);
    
    if (time1 < time2) return kCFCompareLessThan;
    if (time1 > time2) return kCFCompareGreaterThan;
    return kCFCompareEqualTo;
}

static void __CFRunLoopCollectSources0(const void *value, void *context) {
    CFRunLoopSourceRef rls = (CFRunLoopSourceRef)value;
    CFTypeRef *sources = (CFTypeRef *)context;
    if (0 == rls->_context.version0.version && __CFIsValid(rls) && __CFRunLoopSourceIsSignaled(rls)) {
	if (NULL == *sources) {
	    *sources = CFRetain(rls);
	} else if (CFGetTypeID(*sources) == CFRunLoopSourceGetTypeID()) {
	    CFTypeRef oldrls = *sources;
	    *sources = CFArrayCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeArrayCallBacks);
	    CFArrayAppendValue((CFMutableArrayRef)*sources, oldrls);
	    CFArrayAppendValue((CFMutableArrayRef)*sources, rls);
	    CFRelease(oldrls);
	} else {
	    CFArrayAppendValue((CFMutableArrayRef)*sources, rls);
	}
    }
}

static void __CFRUNLOOP_IS_CALLING_OUT_TO_A_SOURCE0_PERFORM_FUNCTION__(void (*)(void *), void *) __attribute__((noinline));
static void __CFRUNLOOP_IS_CALLING_OUT_TO_A_SOURCE0_PERFORM_FUNCTION__(void (*perform)(void *), void *info) {
    if (perform) {
        perform(info);
    }
    __asm __volatile__(""); // thwart tail-call optimization
}

static void __CFRUNLOOP_IS_CALLING_OUT_TO_A_SOURCE1_PERFORM_FUNCTION__(
#if TARGET_OS_MAC
        void *(*perform)(void *msg, CFIndex size, CFAllocatorRef allocator, void *info),
        mach_msg_header_t *msg, CFIndex size, mach_msg_header_t **reply,
#else
        void (*perform)(void *),
#endif
        void *info) __attribute__((noinline));

static void __CFRUNLOOP_IS_CALLING_OUT_TO_A_SOURCE1_PERFORM_FUNCTION__(
#if TARGET_OS_MAC
        void *(*perform)(void *msg, CFIndex size, CFAllocatorRef allocator, void *info),
        mach_msg_header_t *msg, CFIndex size, mach_msg_header_t **reply,
#else
        void (*perform)(void *),
#endif
        void *info) {
    if (perform) {
#if TARGET_OS_MAC
        *reply = perform(msg, size, kCFAllocatorSystemDefault, info);
#else
        perform(info);
#endif
    }
    __asm __volatile__(""); // thwart tail-call optimization
}

static Boolean __CFRunLoopDoSource0(CFRunLoopRef rl, CFRunLoopSourceRef rls) {
    
    Boolean sourceHandled = false;
    __CFRunLoopSourceLock(rls);
    if (__CFRunLoopSourceIsSignaled(rls)) {
        __CFRunLoopSourceUnsetSignaled(rls);
        if (__CFIsValid(rls)) {
            __CFRunLoopSourceUnlock(rls);
            void *perform = rls->_context.version0.perform;
            void *info = rls->_context.version0.info;
            CFRUNLOOP_ARP_BEGIN(rl)
            cf_trace(KDEBUG_EVENT_CFRL_IS_CALLING_SOURCE0 | DBG_FUNC_START, perform, info, 0, 0);
            __CFRUNLOOP_IS_CALLING_OUT_TO_A_SOURCE0_PERFORM_FUNCTION__(perform, info);
            cf_trace(KDEBUG_EVENT_CFRL_IS_CALLING_SOURCE0 | DBG_FUNC_END, perform, info, 0, 0);
            CFRUNLOOP_ARP_END()
            CHECK_FOR_FORK();
            sourceHandled = true;
        } else {
            __CFRunLoopSourceUnlock(rls);
        }
    } else {
        __CFRunLoopSourceUnlock(rls);
    }
    
    
    return sourceHandled;
}

/* rl is locked, rlm is locked on entrance and exit */
static Boolean __CFRunLoopDoSources0(CFRunLoopRef rl, CFRunLoopModeRef rlm, Boolean stopAfterHandle) __attribute__((noinline));
static Boolean __CFRunLoopDoSources0(CFRunLoopRef rl, CFRunLoopModeRef rlm, Boolean stopAfterHandle) {	/* DOES CALLOUT */
    
    cf_trace(KDEBUG_EVENT_CFRL_IS_DOING_SOURCES0 | DBG_FUNC_START, rl, rlm, stopAfterHandle, 0);
    
    CHECK_FOR_FORK();
    CFTypeRef sources = NULL;
    Boolean sourceHandled = false;
    
    /* Fire the version 0 sources */
    if (NULL != rlm->_sources0 && 0 < CFSetGetCount(rlm->_sources0)) {
	CFSetApplyFunction(rlm->_sources0, (__CFRunLoopCollectSources0), &sources);
    }
    if (NULL != sources) {
        __CFRunLoopModeUnlock(rlm);
        __CFRunLoopUnlock(rl);
        // sources is either a single (retained) CFRunLoopSourceRef or an array of (retained) CFRunLoopSourceRef
        if (CFGetTypeID(sources) == CFRunLoopSourceGetTypeID()) {
            CFRunLoopSourceRef rls = (CFRunLoopSourceRef)sources;
            
            sourceHandled = __CFRunLoopDoSource0(rl, rls);
            
        } else {
            CFIndex cnt = CFArrayGetCount((CFArrayRef)sources);
            CFArraySortValues((CFMutableArrayRef)sources, CFRangeMake(0, cnt), (__CFRunLoopSourceComparator), NULL);
            for (CFIndex idx = 0; idx < cnt; idx++) {
                CFRunLoopSourceRef rls = (CFRunLoopSourceRef)CFArrayGetValueAtIndex((CFArrayRef)sources, idx);
                
                sourceHandled = __CFRunLoopDoSource0(rl, rls);
                
                if (stopAfterHandle && sourceHandled) {
                    break;
                }
            }
        }
        CFRelease(sources);
        __CFRunLoopLock(rl);
        __CFRunLoopModeLock(rlm);
    }
    
    cf_trace(KDEBUG_EVENT_CFRL_IS_DOING_SOURCES0 | DBG_FUNC_END, rl, rlm, stopAfterHandle, 0);
    
    return sourceHandled;
}

CF_INLINE void __CFRunLoopDebugInfoForRunLoopSource(CFRunLoopSourceRef rls) {
}

// msg, size and reply are unused on Windows
#if TARGET_OS_MAC
static Boolean __CFRunLoopDoSource1(CFRunLoopRef rl, CFRunLoopModeRef rlm, CFRunLoopSourceRef rls, mach_msg_header_t *msg, CFIndex size, mach_msg_header_t **reply) __attribute__((noinline));

static Boolean __CFRunLoopDoSource1(CFRunLoopRef rl, CFRunLoopModeRef rlm, CFRunLoopSourceRef rls, mach_msg_header_t *msg, CFIndex size, mach_msg_header_t **reply)
#else
static Boolean __CFRunLoopDoSource1(CFRunLoopRef rl, CFRunLoopModeRef rlm, CFRunLoopSourceRef rls)
#endif

{
    
    CHECK_FOR_FORK();
    Boolean sourceHandled = false;

    /* Fire a version 1 source */
    CFRetain(rls);
    __CFRunLoopModeUnlock(rlm);
    __CFRunLoopUnlock(rl);
    __CFRunLoopSourceLock(rls);
    if (__CFIsValid(rls)) {
	__CFRunLoopSourceUnsetSignaled(rls);
	__CFRunLoopSourceUnlock(rls);
        __CFRunLoopDebugInfoForRunLoopSource(rls);
        void *perform = rls->_context.version1.perform;
        void *info = rls->_context.version1.info;
        CFRUNLOOP_ARP_BEGIN(rl)
        cf_trace(KDEBUG_EVENT_CFRL_IS_CALLING_SOURCE1 | DBG_FUNC_START, rl, rlm, perform, info);
        __CFRUNLOOP_IS_CALLING_OUT_TO_A_SOURCE1_PERFORM_FUNCTION__(perform,
#if TARGET_OS_MAC
            msg, size, reply,
#endif
            info);
        cf_trace(KDEBUG_EVENT_CFRL_IS_CALLING_SOURCE1 | DBG_FUNC_END, rl, rlm, perform, info);
        CFRUNLOOP_ARP_END()
        CHECK_FOR_FORK();
	sourceHandled = true;
    } else {
        if (_LogCFRunLoop) { CFLog(kCFLogLevelDebug, CFSTR("%p (%s) __CFRunLoopDoSource1 rls %p is invalid"), CFRunLoopGetCurrent(), *_CFGetProgname(), rls); }
	__CFRunLoopSourceUnlock(rls);
    }
    CFRelease(rls);
    __CFRunLoopLock(rl);
    __CFRunLoopModeLock(rlm);
    
    return sourceHandled;
}

static CFIndex __CFRunLoopInsertionIndexInTimerArray(CFArrayRef array, CFRunLoopTimerRef rlt) __attribute__((noinline));
static CFIndex __CFRunLoopInsertionIndexInTimerArray(CFArrayRef array, CFRunLoopTimerRef rlt) {
    CFIndex cnt = CFArrayGetCount(array);
    if (cnt <= 0) {
        return 0;
    }
    if (256 < cnt) {
        CFRunLoopTimerRef item = (CFRunLoopTimerRef)CFArrayGetValueAtIndex(array, cnt - 1);
        if (item->_fireTSR <= rlt->_fireTSR) {
            return cnt;
        }
        item = (CFRunLoopTimerRef)CFArrayGetValueAtIndex(array, 0);
        if (rlt->_fireTSR < item->_fireTSR) {
            return 0;
        }
    }

    CFIndex add = (1 << flsl(cnt)) * 2;
    CFIndex idx = 0;
    Boolean lastTestLEQ;
    do {
        add = add / 2;
	lastTestLEQ = false;
        CFIndex testIdx = idx + add;
        if (testIdx < cnt) {
            CFRunLoopTimerRef item = (CFRunLoopTimerRef)CFArrayGetValueAtIndex(array, testIdx);
            if (item->_fireTSR <= rlt->_fireTSR) {
                idx = testIdx;
		lastTestLEQ = true;
            }
        }
    } while (0 < add);

    return lastTestLEQ ? idx + 1 : idx;
}

static void __CFArmNextTimerInMode(CFRunLoopModeRef rlm, CFRunLoopRef rl) {    
    uint64_t nextHardDeadline = UINT64_MAX;
    uint64_t nextSoftDeadline = UINT64_MAX;

    if (rlm->_timers) {
        // Look at the list of timers. We will calculate two TSR values; the next soft and next hard deadline.
        // The next soft deadline is the first time we can fire any timer. This is the fire date of the first timer in our sorted list of timers.
        // The next hard deadline is the last time at which we can fire the timer before we've moved out of the allowable tolerance of the timers in our list.
        for (CFIndex idx = 0, cnt = CFArrayGetCount(rlm->_timers); idx < cnt; idx++) {
            CFRunLoopTimerRef t = (CFRunLoopTimerRef)CFArrayGetValueAtIndex(rlm->_timers , idx);
            // discount timers currently firing
            if (__CFRunLoopTimerIsFiring(t)) continue;
            
            uint64_t oneTimerHardDeadline;
            uint64_t oneTimerSoftDeadline = t->_fireTSR;
            if (os_add_overflow(t->_fireTSR, __CFTimeIntervalToTSR(t->_tolerance), &oneTimerHardDeadline)) {
                oneTimerHardDeadline = UINT64_MAX;
            }
            
            // We can stop searching if the soft deadline for this timer exceeds the current hard deadline. Otherwise, later timers with lower tolerance could still have earlier hard deadlines.
            if (oneTimerSoftDeadline > nextHardDeadline) {
                break;
            }
            
            if (oneTimerSoftDeadline < nextSoftDeadline) {
                nextSoftDeadline = oneTimerSoftDeadline;
            }
            
            if (oneTimerHardDeadline < nextHardDeadline) {
                nextHardDeadline = oneTimerHardDeadline;
            }
        }
        
        if (nextSoftDeadline < UINT64_MAX && (nextHardDeadline != rlm->_timerHardDeadline || nextSoftDeadline != rlm->_timerSoftDeadline)) {
            if (CFRUNLOOP_NEXT_TIMER_ARMED_ENABLED()) {
                CFRUNLOOP_NEXT_TIMER_ARMED((unsigned long)(nextSoftDeadline - __CFNanosecondsToTSR(mach_absolute_time())));
            }
            
            cf_trace(KDEBUG_EVENT_CFRL_NEXT_TIMER_ARMED, rl, rlm, (nextSoftDeadline - __CFNanosecondsToTSR(mach_absolute_time())), 0);
#if USE_DISPATCH_SOURCE_FOR_TIMERS
            // We're going to hand off the range of allowable timer fire date to dispatch and let it fire when appropriate for the system.
            uint64_t leeway = __CFTSRToNanoseconds(nextHardDeadline - nextSoftDeadline);
            dispatch_time_t deadline = __CFTSRToDispatchTime(nextSoftDeadline);
            if (leeway > 0) {
                // Only use the dispatch timer if we have any leeway
                // <rdar://problem/14447675>
                
                // Cancel the mk timer
                if (rlm->_mkTimerArmed && rlm->_timerPort) {
                    AbsoluteTime dummy;
                    mk_timer_cancel(rlm->_timerPort, &dummy);
                    rlm->_mkTimerArmed = false;
                }
                
                // Arm the dispatch timer
                dispatch_source_set_timer(rlm->_timerSource, deadline, DISPATCH_TIME_FOREVER, leeway);
                rlm->_dispatchTimerArmed = true;
            } else {
                // Cancel the dispatch timer
                if (rlm->_dispatchTimerArmed) {
                    // Cancel the dispatch timer
                    dispatch_source_set_timer(rlm->_timerSource, DISPATCH_TIME_FOREVER, DISPATCH_TIME_FOREVER, 888);
                    rlm->_dispatchTimerArmed = false;
                }
                
                // Arm the mk timer
                if (rlm->_timerPort) {
                    mk_timer_arm(rlm->_timerPort, nextSoftDeadline);
                    rlm->_mkTimerArmed = true;
                }
            }
#else
            if (rlm->_timerPort) {
                mk_timer_arm(rlm->_timerPort, nextSoftDeadline);
            }
#endif
        } else if (nextSoftDeadline == UINT64_MAX) {
            // Disarm the timers - there is no timer scheduled
            
            if (rlm->_mkTimerArmed && rlm->_timerPort) {
                AbsoluteTime dummy;
                mk_timer_cancel(rlm->_timerPort, &dummy);
                rlm->_mkTimerArmed = false;
            }
            
#if USE_DISPATCH_SOURCE_FOR_TIMERS
            if (rlm->_dispatchTimerArmed) {
                dispatch_source_set_timer(rlm->_timerSource, DISPATCH_TIME_FOREVER, DISPATCH_TIME_FOREVER, 333);
                rlm->_dispatchTimerArmed = false;
            }
#endif
        }
    }
    rlm->_timerHardDeadline = nextHardDeadline;
    rlm->_timerSoftDeadline = nextSoftDeadline;
}

// call with rlm and its run loop locked, and the TSRLock locked; rlt not locked; returns with same state
static void __CFRepositionTimerInMode(CFRunLoopModeRef rlm, CFRunLoopTimerRef rlt, Boolean isInArray) __attribute__((noinline));
static void __CFRepositionTimerInMode(CFRunLoopModeRef rlm, CFRunLoopTimerRef rlt, Boolean isInArray) {
    if (!rlt) return;
    
    CFMutableArrayRef timerArray = rlm->_timers;
    if (!timerArray) return;
    Boolean found = false;
    
    // If we know in advance that the timer is not in the array (just being added now) then we can skip this search
    if (isInArray) {
        CFIndex idx = CFArrayGetFirstIndexOfValue(timerArray, CFRangeMake(0, CFArrayGetCount(timerArray)), rlt);
        if (kCFNotFound != idx) {
            CFRetain(rlt);
            CFArrayRemoveValueAtIndex(timerArray, idx);
            found = true;
        }
    }
    if (!found && isInArray) return;
    CFIndex newIdx = __CFRunLoopInsertionIndexInTimerArray(timerArray, rlt);
    CFArrayInsertValueAtIndex(timerArray, newIdx, rlt);
    __CFArmNextTimerInMode(rlm, rlt->_runLoop);
    if (isInArray) CFRelease(rlt);
}


// mode and rl are locked on entry and exit
static Boolean __CFRunLoopDoTimer(CFRunLoopRef rl, CFRunLoopModeRef rlm, CFRunLoopTimerRef rlt) {	/* DOES CALLOUT */
    
    cf_trace(KDEBUG_EVENT_CFRL_TIMERS_FIRING | DBG_FUNC_START, rl, rlm, rlt, 0);
    
    Boolean timerHandled = false;
    uint64_t oldFireTSR = 0;

    /* Fire a timer */
    CFRetain(rlt);
    __CFRunLoopTimerLock(rlt);

    if (__CFIsValid(rlt) && rlt->_fireTSR <= __CFNanosecondsToTSR(mach_absolute_time()) && !__CFRunLoopTimerIsFiring(rlt) && rlt->_runLoop == rl) {
        void *context_info = NULL;
        void (*context_release)(const void *) = NULL;
        if (rlt->_context.retain) {
            context_info = (void *)rlt->_context.retain(rlt->_context.info);
            context_release = rlt->_context.release;
        } else {
            context_info = rlt->_context.info;
        }
        Boolean doInvalidate = (0.0 == rlt->_interval);
	__CFRunLoopTimerSetFiring(rlt);
        // Just in case the next timer has exactly the same deadlines as this one, we reset these values so that the arm next timer code can correctly find the next timer in the list and arm the underlying timer.
        rlm->_timerSoftDeadline = UINT64_MAX;
        rlm->_timerHardDeadline = UINT64_MAX;
        __CFRunLoopTimerUnlock(rlt);
        __CFLock(&rl->_timerTSRLock);
	oldFireTSR = rlt->_fireTSR;
        __CFUnlock(&rl->_timerTSRLock);

        __CFArmNextTimerInMode(rlm, rl);

	__CFRunLoopModeUnlock(rlm);
	__CFRunLoopUnlock(rl);
        
        CFRunLoopTimerCallBack callout = rlt->_callout;
        CFRUNLOOP_ARP_BEGIN(NULL)
        cf_trace(KDEBUG_EVENT_CFRL_IS_CALLING_TIMER | DBG_FUNC_START, callout, rlt, context_info, 0);
	__CFRUNLOOP_IS_CALLING_OUT_TO_A_TIMER_CALLBACK_FUNCTION__(callout, rlt, context_info);
        cf_trace(KDEBUG_EVENT_CFRL_IS_CALLING_TIMER | DBG_FUNC_END, callout, rlt, context_info, 0);
        CFRUNLOOP_ARP_END()

	CHECK_FOR_FORK();
        if (doInvalidate) {
            CFRunLoopTimerInvalidate(rlt);      /* DOES CALLOUT */
        }
        if (context_release) {
            context_release(context_info);
        }
        
	__CFRunLoopLock(rl);
	__CFRunLoopModeLock(rlm);
        __CFRunLoopTimerLock(rlt);
	timerHandled = true;
	__CFRunLoopTimerUnsetFiring(rlt);
    }
    if (__CFIsValid(rlt) && timerHandled) {
        /* This is just a little bit tricky: we want to support calling
         * CFRunLoopTimerSetNextFireDate() from within the callout and
         * honor that new time here if it is a later date, otherwise
         * it is completely ignored. */
        if (oldFireTSR < rlt->_fireTSR) {
            /* Next fire TSR was set, and set to a date after the previous
            * fire date, so we honor it. */
            __CFRunLoopTimerUnlock(rlt);
            // The timer was adjusted and repositioned, during the
            // callout, but if it was still the min timer, it was
            // skipped because it was firing.  Need to redo the
            // min timer calculation in case rlt should now be that
            // timer instead of whatever was chosen.
            __CFArmNextTimerInMode(rlm, rl);
        } else {
	    uint64_t nextFireTSR = 0LL;
            uint64_t intervalTSR = 0LL;
            if (rlt->_interval <= 0.0) {
            } else if (TIMER_INTERVAL_LIMIT < rlt->_interval) {
        	intervalTSR = __CFTimeIntervalToTSR(TIMER_INTERVAL_LIMIT);
            } else {
        	intervalTSR = __CFTimeIntervalToTSR(rlt->_interval);
            }
            if (LLONG_MAX - intervalTSR <= oldFireTSR) {
                nextFireTSR = LLONG_MAX;
            } else {
                if (intervalTSR == 0) {
                    // 15304159: Make sure we don't accidentally loop forever here
                    CRSetCrashLogMessage("A CFRunLoopTimer with an interval of 0 is set to repeat");
                    HALT;
                }
                uint64_t currentTSR = __CFNanosecondsToTSR(mach_absolute_time());
                nextFireTSR = oldFireTSR;
                while (nextFireTSR <= currentTSR) {
                    nextFireTSR += intervalTSR;
                }
            }
            CFRunLoopRef rlt_rl = rlt->_runLoop;
            if (rlt_rl) {
                CFRetain(rlt_rl);
		CFIndex cnt = CFSetGetCount(rlt->_rlModes);
		STACK_BUFFER_DECL(CFTypeRef, modes, cnt);
		CFSetGetValues(rlt->_rlModes, (const void **)modes);
		// To avoid A->B, B->A lock ordering issues when coming up
		// towards the run loop from a source, the timer has to be
		// unlocked, which means we have to protect from object
		// invalidation, although that's somewhat expensive.
		for (CFIndex idx = 0; idx < cnt; idx++) {
		    CFRetain(modes[idx]);
		}
		__CFRunLoopTimerUnlock(rlt);
		for (CFIndex idx = 0; idx < cnt; idx++) {
		    CFStringRef name = (CFStringRef)modes[idx];
		    modes[idx] = (CFTypeRef)__CFRunLoopCopyMode(rlt_rl, name, false);
                    if (modes[idx]) {
                        __CFRunLoopModeLock((CFRunLoopModeRef)modes[idx]);
                    }
		    CFRelease(name);
		}
                __CFLock(&rl->_timerTSRLock);
		rlt->_fireTSR = nextFireTSR;
                rlt->_nextFireDate = CFAbsoluteTimeGetCurrent() + __CFTimeIntervalUntilTSR(nextFireTSR);
		for (CFIndex idx = 0; idx < cnt; idx++) {
		    CFRunLoopModeRef rlm = (CFRunLoopModeRef)modes[idx];
		    if (rlm) {
                        __CFRepositionTimerInMode(rlm, rlt, true);
		    }
		}
                __CFUnlock(&rl->_timerTSRLock);
                for (CFIndex idx = cnt - 1; idx >= 0; idx--) { // reverse index here so we unlock in the right order
                    if (modes[idx] != NULL) {
                        __CFRunLoopModeUnlock((CFRunLoopModeRef)modes[idx]);
                        CFRelease((CFRunLoopModeRef)modes[idx]);
                    }
		}
		CFRelease(rlt_rl);
	    } else {
		__CFRunLoopTimerUnlock(rlt);
                __CFLock(&rl->_timerTSRLock);
		rlt->_fireTSR = nextFireTSR;
                rlt->_nextFireDate = CFAbsoluteTimeGetCurrent() + __CFTimeIntervalUntilTSR(nextFireTSR);
                __CFUnlock(&rl->_timerTSRLock);
            }
        }
    } else {
        __CFRunLoopTimerUnlock(rlt);
    }
    CFRelease(rlt);
    
    cf_trace(KDEBUG_EVENT_CFRL_TIMERS_FIRING | DBG_FUNC_END, rl, rlm, rlt, 0);
    
    return timerHandled;
}


// rl and rlm are locked on entry and exit
static Boolean __CFRunLoopDoTimers(CFRunLoopRef rl, CFRunLoopModeRef rlm, uint64_t limitTSR) {	/* DOES CALLOUT */
    
    cf_trace(KDEBUG_EVENT_CFRL_IS_DOING_TIMERS | DBG_FUNC_START, rl, rlm, limitTSR, 0);
    
    Boolean timerHandled = false;
    CFMutableArrayRef timers = NULL;
    for (CFIndex idx = 0, cnt = rlm->_timers ? CFArrayGetCount(rlm->_timers) : 0; idx < cnt; idx++) {
        CFRunLoopTimerRef rlt = (CFRunLoopTimerRef)CFArrayGetValueAtIndex(rlm->_timers, idx);
        
        if (__CFIsValid(rlt) && !__CFRunLoopTimerIsFiring(rlt)) {
            if (rlt->_fireTSR <= limitTSR) {
                if (!timers) timers = CFArrayCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeArrayCallBacks);
                CFArrayAppendValue(timers, rlt);
            }
        }
    }

    for (CFIndex idx = 0, cnt = timers ? CFArrayGetCount(timers) : 0; idx < cnt; idx++) {
        CFRunLoopTimerRef rlt = (CFRunLoopTimerRef)CFArrayGetValueAtIndex(timers, idx);
        Boolean did = __CFRunLoopDoTimer(rl, rlm, rlt);
        timerHandled = timerHandled || did;
    }
        if (timers) CFRelease(timers);

    cf_trace(KDEBUG_EVENT_CFRL_IS_DOING_TIMERS | DBG_FUNC_END, rl, rlm, limitTSR, 0);
    
    return timerHandled;
}


CF_EXPORT Boolean _CFRunLoopFinished(CFRunLoopRef rl, CFStringRef modeName) {
    CHECK_FOR_FORK();
    CFRunLoopModeRef rlm;
    Boolean result = false;
    __CFRunLoopLock(rl);
    rlm = __CFRunLoopCopyMode(rl, modeName, false);
    if (rlm) {
        __CFRunLoopModeLock(rlm);
    }
    if (NULL == rlm || __CFRunLoopModeIsEmpty(rl, rlm, NULL)) {
	result = true;
    }
    if (rlm) {
        __CFRunLoopModeUnlock(rlm);
        CFRelease(rlm);
    }
    __CFRunLoopUnlock(rl);
    return result;
}

static int32_t __CFRunLoopRun(CFRunLoopRef rl, CFRunLoopModeRef rlm, CFTimeInterval seconds, Boolean stopAfterHandle, CFRunLoopModeRef previousMode) __attribute__((noinline));

#if TARGET_OS_MAC

#define TIMEOUT_INFINITY (~(mach_msg_timeout_t)0)

static Boolean __CFRunLoopServiceMachPort(mach_port_name_t port, mach_msg_header_t **buffer, size_t buffer_size, mach_port_t *livePort, mach_msg_timeout_t timeout, voucher_mach_msg_state_t * _Nonnull voucherState, voucher_t *voucherCopy, CFRunLoopRef rl, CFRunLoopModeRef rlm) {
    Boolean originalBuffer = true;
    kern_return_t ret = KERN_SUCCESS;
    for (;;) {		/* In that sleep of death what nightmares may come ... */
        mach_msg_header_t *msg = (mach_msg_header_t *)*buffer;
        msg->msgh_bits = 0;
        msg->msgh_local_port = port;
        msg->msgh_remote_port = MACH_PORT_NULL;
        msg->msgh_size = buffer_size;
        msg->msgh_id = 0;
        if (TIMEOUT_INFINITY == timeout) {
            CFRUNLOOP_SLEEP();
            cf_trace(KDEBUG_EVENT_CFRL_SLEEP, port, 0, 0, 0);
        } else {
            CFRUNLOOP_POLL();
            cf_trace(KDEBUG_EVENT_CFRL_POLL, port, 0, 0, 0);
        }
        cf_trace(KDEBUG_EVENT_CFRL_RUN | DBG_FUNC_END, rl, rlm, port, timeout);
        cf_trace(KDEBUG_EVENT_CFRL_IS_WAITING | DBG_FUNC_START, rl, rlm, port, timeout);
        ret = mach_msg(msg, MACH_RCV_MSG|MACH_RCV_VOUCHER|MACH_RCV_LARGE|((TIMEOUT_INFINITY != timeout) ? MACH_RCV_TIMEOUT : 0)|MACH_RCV_TRAILER_TYPE(MACH_MSG_TRAILER_FORMAT_0)|MACH_RCV_TRAILER_ELEMENTS(MACH_RCV_TRAILER_AV), 0, msg->msgh_size, port, timeout, MACH_PORT_NULL);
        cf_trace(KDEBUG_EVENT_CFRL_IS_WAITING | DBG_FUNC_END, rl, rlm, port, timeout);
        cf_trace(KDEBUG_EVENT_CFRL_RUN | DBG_FUNC_START, rl, rlm, port, timeout);
        // Take care of all voucher-related work right after mach_msg.
        // If we don't release the previous voucher we're going to leak it.
        voucher_mach_msg_revert(*voucherState);
        
        // Someone will be responsible for calling voucher_mach_msg_revert. This call makes the received voucher the current one.
        *voucherState = voucher_mach_msg_adopt(msg);
        
        if (voucherCopy) {
            *voucherCopy = NULL;
        }

        CFRUNLOOP_WAKEUP(ret);
        cf_trace(KDEBUG_EVENT_CFRL_DID_WAKEUP, port, 0, 0, 0);
        if (MACH_MSG_SUCCESS == ret) {
            *livePort = msg ? msg->msgh_local_port : MACH_PORT_NULL;
            return true;
        }
        if (MACH_RCV_TIMED_OUT == ret) {
            if (!originalBuffer) free(msg);
            *buffer = NULL;
            *livePort = MACH_PORT_NULL;
            return false;
        }
        if (MACH_RCV_TOO_LARGE != ret) {
            if (((MACH_RCV_HEADER_ERROR & ret) == MACH_RCV_HEADER_ERROR) || (MACH_RCV_BODY_ERROR & ret) == MACH_RCV_BODY_ERROR) {
                kern_return_t specialBits = MACH_MSG_MASK & ret;
                if (MACH_MSG_IPC_SPACE == specialBits) {
                    CRSetCrashLogMessage("Out of IPC space");
                } else if (MACH_MSG_VM_SPACE == specialBits) {
                    CRSetCrashLogMessage("Out of VM address space");
                } else if (MACH_MSG_IPC_KERNEL == specialBits) {
                    CRSetCrashLogMessage("Kernel resource shortage handling IPC");
                } else if (MACH_MSG_VM_KERNEL == specialBits) {
                    CRSetCrashLogMessage("Kernel resource shortage handling out-of-line memory");
                }
            } else {
                CRSetCrashLogMessage(mach_error_string(ret));
            }
            break;
        }
        buffer_size = round_msg(msg->msgh_size + MAX_TRAILER_SIZE);
        if (originalBuffer) *buffer = NULL;
        originalBuffer = false;
        *buffer = __CFSafelyReallocate(*buffer, buffer_size, NULL);

        if (voucherCopy != NULL && *voucherCopy != NULL) {
            os_release(*voucherCopy);
        }
    }
    HALT;
    return false;
}

#elif TARGET_OS_LINUX

#define TIMEOUT_INFINITY UINT64_MAX

static int __CFPollFileDescriptors(struct pollfd *fds, nfds_t nfds, uint64_t timeout) {
    uint64_t elapsed = 0;
    uint64_t start = mach_absolute_time();
    int result = 0;
    while (1) {
        struct timespec ts = {0};
        struct timespec *tsPtr = &ts;
        if (timeout == TIMEOUT_INFINITY) {
            tsPtr = NULL;
            
        } else if (elapsed < timeout) {
            uint64_t delta = timeout - elapsed;
            ts.tv_sec = delta / 1000000000UL;
            ts.tv_nsec = delta % 1000000000UL;
        }
        
        result = ppoll(fds, 1, tsPtr, NULL);
        
        if (result == -1 && errno == EINTR) {
            uint64_t end = mach_absolute_time();
            elapsed += (end - start);
            start = end;
            
        } else {
            return result;
        }
    }
}

// pass in either a portSet or onePort. portSet is an epollfd, onePort is either a timerfd or an eventfd.
// TODO: Better error handling. What should happen if we get an error on a file descriptor?
static Boolean __CFRunLoopServiceFileDescriptors(__CFPortSet portSet, __CFPort onePort, uint64_t timeout, int *livePort) {
    struct pollfd fdInfo = {
        .fd = (onePort == CFPORT_NULL) ? portSet : onePort,
        .events = POLLIN
    };
    
    ssize_t result = __CFPollFileDescriptors(&fdInfo, 1, timeout);
    if (result == 0)
        return false;
    
    CFAssert2(result != -1, __kCFLogAssertion, "%s(): error %d from ppoll", __PRETTY_FUNCTION__, errno);
    
    int awokenFd;
    
    if (onePort != CFPORT_NULL) {
        CFAssert1(0 == (fdInfo.revents & (POLLERR|POLLHUP)), __kCFLogAssertion, "%s(): ppoll reported error for fd", __PRETTY_FUNCTION__);
        awokenFd = onePort;
        
    } else {
        struct epoll_event event;
        do {
            result = epoll_wait(portSet, &event, 1 /*numEvents*/, 0 /*timeout*/);
        } while (result == -1 && errno == EINTR);
        CFAssert2(result >= 0, __kCFLogAssertion, "%s(): error %d from epoll_wait", __PRETTY_FUNCTION__, errno);
        
        if (result == 0) {
            return false;
        }
        
        awokenFd = event.data.fd;
    }
    
    // Now we acknowledge the wakeup. awokenFd is an eventfd (or possibly a
    // timerfd ?). In either case, we read an 8-byte integer, as per eventfd(2)
    // and timerfd_create(2).
    uint64_t value;
    do {
        result = read(awokenFd, &value, sizeof(value));
    } while (result == -1 && errno == EINTR);
    
    if (result == -1 && errno == EAGAIN) {
        // Another thread stole the wakeup for this fd. (FIXME Can this actually
        // happen?)
        return false;
    }
    
    CFAssert2(result == sizeof(value), __kCFLogAssertion, "%s(): error %d from read(2) while acknowledging wakeup", __PRETTY_FUNCTION__, errno);
    
    if (livePort)
        *livePort = awokenFd;
    
    return true;
}

#elif TARGET_OS_WIN32 || TARGET_OS_CYGWIN

#define TIMEOUT_INFINITY INFINITE

// pass in either a portSet or onePort
static Boolean __CFRunLoopWaitForMultipleObjects(__CFPortSet portSet, HANDLE *onePort, DWORD timeout, DWORD mask, HANDLE *livePort, Boolean *msgReceived) {
    DWORD waitResult = WAIT_TIMEOUT;
    HANDLE handleBuf[MAXIMUM_WAIT_OBJECTS];
    HANDLE *handles = NULL;
    uint32_t handleCount = 0;
    Boolean freeHandles = false;
    Boolean result = false;
    
    if (portSet) {
	// copy out the handles to be safe from other threads at work
	handles = __CFPortSetGetPorts(portSet, handleBuf, MAXIMUM_WAIT_OBJECTS, &handleCount);
	freeHandles = (handles != handleBuf);
    } else {
	handles = onePort;
	handleCount = 1;
	freeHandles = FALSE;
    }
    
    // The run loop mode and loop are already in proper unlocked state from caller
    waitResult = MsgWaitForMultipleObjectsEx(__CFMin(handleCount, MAXIMUM_WAIT_OBJECTS), handles, timeout, mask, MWMO_INPUTAVAILABLE);
    
    CFAssert2(waitResult != WAIT_FAILED, __kCFLogAssertion, "%s(): error %d from MsgWaitForMultipleObjects", __PRETTY_FUNCTION__, GetLastError());
    
    if (waitResult == WAIT_TIMEOUT) {
	// do nothing, just return to caller
	result = false;
    } else if (waitResult >= WAIT_OBJECT_0 && waitResult < WAIT_OBJECT_0+handleCount) {
	// a handle was signaled
	if (livePort) *livePort = handles[waitResult-WAIT_OBJECT_0];
	result = true;
    } else if (waitResult == WAIT_OBJECT_0+handleCount) {
	// windows message received
        if (msgReceived) *msgReceived = true;
	result = true;
    } else if (waitResult >= WAIT_ABANDONED_0 && waitResult < WAIT_ABANDONED_0+handleCount) {
	// an "abandoned mutex object"
	if (livePort) *livePort = handles[waitResult-WAIT_ABANDONED_0];
	result = true;
    } else {
	CFAssert2(waitResult == WAIT_FAILED, __kCFLogAssertion, "%s(): unexpected result from MsgWaitForMultipleObjects: %d", __PRETTY_FUNCTION__, waitResult);
	result = false;
    }
    
    if (freeHandles) {
	CFAllocatorDeallocate(kCFAllocatorSystemDefault, handles);
    }
    
    return result;
}

#endif

/* rl, rlm are locked on entrance and exit */
static int32_t __CFRunLoopRun(CFRunLoopRef rl, CFRunLoopModeRef rlm, CFTimeInterval seconds, Boolean stopAfterHandle, CFRunLoopModeRef previousMode) {
    uint64_t startTSR = __CFNanosecondsToTSR(mach_absolute_time());

    if (__CFRunLoopIsStopped(rl)) {
        __CFRunLoopUnsetStopped(rl);
	return kCFRunLoopRunStopped;
    } else if (rlm->_stopped) {
	rlm->_stopped = false;
	return kCFRunLoopRunStopped;
    }
    
#if __HAS_DISPATCH__
    __CFPort dispatchPort = CFPORT_NULL;
    Boolean libdispatchQSafe = (pthread_main_np() == 1) && ((HANDLE_DISPATCH_ON_BASE_INVOCATION_ONLY && NULL == previousMode) || (!HANDLE_DISPATCH_ON_BASE_INVOCATION_ONLY && 0 == _CFGetTSD(__CFTSDKeyIsInGCDMainQ)));
    if (libdispatchQSafe && (CFRunLoopGetMain() == rl) && CFSetContainsValue(rl->_commonModes, rlm->_name)) dispatchPort = _dispatch_get_main_queue_port_4CF();
#endif
    
#if USE_DISPATCH_SOURCE_FOR_TIMERS
    mach_port_name_t modeQueuePort = MACH_PORT_NULL;
    if (rlm->_queue) {
        modeQueuePort = _dispatch_runloop_root_queue_get_port_4CF(rlm->_queue);
        if (!modeQueuePort) {
            CRASH("Unable to get port for run loop mode queue (%d)", -1);
        }
    }
#endif

    uint64_t termTSR = 0ULL;
#if __HAS_DISPATCH__
    dispatch_source_t timeout_timer = NULL;
#endif
    if (seconds <= 0.0) { // instant timeout
        seconds = 0.0;
        termTSR = 0ULL;
    } else if (seconds <= TIMER_INTERVAL_LIMIT) {
        termTSR = startTSR + __CFTimeIntervalToTSR(seconds);
#if __HAS_DISPATCH__
	dispatch_queue_t queue = (pthread_main_np() == 1) ? __CFDispatchQueueGetGenericMatchingMain() : __CFDispatchQueueGetGenericBackground();
	timeout_timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, queue);

        CFRetain(rl);
        dispatch_source_set_event_handler(timeout_timer, ^{
            CFRUNLOOP_WAKEUP_FOR_TIMEOUT();
            cf_trace(KDEBUG_EVENT_CFRL_DID_WAKEUP_FOR_TIMEOUT, rl, 0, 0, 0);
            CFRunLoopWakeUp(rl);
            // The interval is DISPATCH_TIME_FOREVER, so this won't fire again
        });
        dispatch_source_set_cancel_handler(timeout_timer, ^{
            CFRelease(rl);
        });

        uint64_t ns_at = (uint64_t)((__CFTSRToTimeInterval(startTSR) + seconds) * 1000000000ULL);
        dispatch_source_set_timer(timeout_timer, dispatch_time(1, ns_at), DISPATCH_TIME_FOREVER, 1000ULL);
        dispatch_resume(timeout_timer);
#endif
    } else { // infinite timeout
        seconds = 9999999999.0;
        termTSR = UINT64_MAX;
    }

    Boolean didDispatchPortLastTime = true;
    int32_t retVal = 0;
    do {
#if TARGET_OS_MAC
        voucher_mach_msg_state_t voucherState = VOUCHER_MACH_MSG_STATE_UNCHANGED;
        voucher_t voucherCopy = NULL;
#endif
        uint8_t msg_buffer[3 * 1024];
#if TARGET_OS_MAC
        mach_msg_header_t *msg = NULL;
        mach_port_t livePort = MACH_PORT_NULL;
#elif TARGET_OS_WIN32 || TARGET_OS_CYGWIN
        HANDLE livePort = NULL;
        Boolean windowsMessageReceived = false;
#elif TARGET_OS_LINUX
        int livePort = -1;
#else
        __CFPort livePort = CFPORT_NULL;
#endif
	__CFPortSet waitSet = rlm->_portSet;

        __CFRunLoopUnsetIgnoreWakeUps(rl);

        if (rlm->_observerMask & kCFRunLoopBeforeTimers) {
            __CFRunLoopDoObservers(rl, rlm, kCFRunLoopBeforeTimers);
        }
        
        if (rlm->_observerMask & kCFRunLoopBeforeSources) {
            __CFRunLoopDoObservers(rl, rlm, kCFRunLoopBeforeSources);
        }

	__CFRunLoopDoBlocks(rl, rlm);

        Boolean sourceHandledThisLoop = __CFRunLoopDoSources0(rl, rlm, stopAfterHandle);
        if (sourceHandledThisLoop) {
            __CFRunLoopDoBlocks(rl, rlm);
        }

        Boolean poll = sourceHandledThisLoop || (0ULL == termTSR);

#if __HAS_DISPATCH__
        if (CFPORT_NULL != dispatchPort && !didDispatchPortLastTime) {
#if TARGET_OS_MAC
            msg = (mach_msg_header_t *)msg_buffer;
            if (__CFRunLoopServiceMachPort(dispatchPort, &msg, sizeof(msg_buffer), &livePort, 0, &voucherState, NULL, rl, rlm)) {
                goto handle_msg;
            }
#elif TARGET_OS_LINUX && !TARGET_OS_CYGWIN
            if (__CFRunLoopServiceFileDescriptors(CFPORTSET_NULL, dispatchPort, 0, &livePort)) {
                goto handle_msg;
            }
#elif TARGET_OS_WIN32 || TARGET_OS_CYGWIN
            if (__CFRunLoopWaitForMultipleObjects(NULL, &dispatchPort, 0, 0, &livePort, NULL)) {
                goto handle_msg;
            }
#elif TARGET_OS_BSD
            if (__CFRunLoopServiceFileDescriptors(CFPORTSET_NULL, dispatchPort, 0, &livePort)) {
                goto handle_msg;
            }
#else
#error "invoking the port select implementation is required"
#endif
        }
#endif

        didDispatchPortLastTime = false;

	if (!poll && (rlm->_observerMask & kCFRunLoopBeforeWaiting)) __CFRunLoopDoObservers(rl, rlm, kCFRunLoopBeforeWaiting);
	__CFRunLoopSetSleeping(rl);
	// do not do any user callouts after this point (after notifying of sleeping)

        // Must push the local-to-this-activation ports in on every loop
        // iteration, as this mode could be run re-entrantly and we don't
        // want these ports to get serviced.
#if __HAS_DISPATCH__
        __CFPortSetInsert(dispatchPort, waitSet);
#endif
        
	__CFRunLoopModeUnlock(rlm);
	__CFRunLoopUnlock(rl);

        CFAbsoluteTime sleepStart = poll ? 0.0 : CFAbsoluteTimeGetCurrent();

#if TARGET_OS_MAC
#if USE_DISPATCH_SOURCE_FOR_TIMERS
        do {
            msg = (mach_msg_header_t *)msg_buffer;
            
            __CFRunLoopServiceMachPort(waitSet, &msg, sizeof(msg_buffer), &livePort, poll ? 0 : TIMEOUT_INFINITY, &voucherState, &voucherCopy, rl, rlm);
            
            if (modeQueuePort != MACH_PORT_NULL && livePort == modeQueuePort) {
                // Drain the internal queue. If one of the callout blocks sets the timerFired flag, break out and service the timer.
                while (_dispatch_runloop_root_queue_perform_4CF(rlm->_queue));
                if (rlm->_timerFired) {
                    // Leave livePort as the queue port, and service timers below
                    rlm->_timerFired = false;
                    break;
                } else {
                    if (msg && msg != (mach_msg_header_t *)msg_buffer) free(msg);
                }
            } else {
                // Go ahead and leave the inner loop.
                break;
            }
        } while (1);
#else
        msg = (mach_msg_header_t *)msg_buffer;
        __CFRunLoopServiceMachPort(waitSet, &msg, sizeof(msg_buffer), &livePort, poll ? 0 : TIMEOUT_INFINITY, &voucherState, &voucherCopy, rl, rlm);
#endif
        
        
#elif TARGET_OS_WIN32
        // Here, use the app-supplied message queue mask. They will set this if they are interested in having this run loop receive windows messages.
        __CFRunLoopWaitForMultipleObjects(waitSet, NULL, poll ? 0 : TIMEOUT_INFINITY, rlm->_msgQMask, &livePort, &windowsMessageReceived);
#elif TARGET_OS_LINUX
        __CFRunLoopServiceFileDescriptors(waitSet, CFPORT_NULL, poll ? 0 : TIMEOUT_INFINITY, &livePort);
#elif TARGET_OS_BSD
        __CFRunLoopServiceFileDescriptors(waitSet, CFPORT_NULL, poll ? 0 : TIMEOUT_INFINITY, &livePort);
#else
#error "invoking the port set select implementation is required"
#endif
        
        __CFRunLoopLock(rl);
        __CFRunLoopModeLock(rlm);

        rl->_sleepTime += (poll ? 0.0 : (CFAbsoluteTimeGetCurrent() - sleepStart));

        // Must remove the local-to-this-activation ports in on every loop
        // iteration, as this mode could be run re-entrantly and we don't
        // want these ports to get serviced. Also, we don't want them left
        // in there if this function returns.
#if __HAS_DISPATCH__
        __CFPortSetRemove(dispatchPort, waitSet);
#endif
        
        __CFRunLoopSetIgnoreWakeUps(rl);

        // user callouts now OK again
	__CFRunLoopUnsetSleeping(rl);
	if (!poll && (rlm->_observerMask & kCFRunLoopAfterWaiting)) __CFRunLoopDoObservers(rl, rlm, kCFRunLoopAfterWaiting);

        handle_msg:;
        __CFRunLoopSetIgnoreWakeUps(rl);

#if TARGET_OS_WIN32
        if (windowsMessageReceived) {
            // These Win32 APIs cause a callout, so make sure we're unlocked first and relocked after
            __CFRunLoopModeUnlock(rlm);
	    __CFRunLoopUnlock(rl);

            if (rlm->_msgPump) {
                rlm->_msgPump();
            } else {
                MSG msg;
                if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE | PM_NOYIELD)) {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
            
            __CFRunLoopLock(rl);
	    __CFRunLoopModeLock(rlm);
 	    sourceHandledThisLoop = true;
            
            // To prevent starvation of sources other than the message queue, we check again to see if any other sources need to be serviced
            // Use 0 for the mask so windows messages are ignored this time. Also use 0 for the timeout, because we're just checking to see if the things are signalled right now -- we will wait on them again later.
            // NOTE: Ignore the dispatch source (it's not in the wait set anymore) and also don't run the observers here since we are polling.
            __CFRunLoopSetSleeping(rl);
            __CFRunLoopModeUnlock(rlm);
            __CFRunLoopUnlock(rl);
            
            __CFRunLoopWaitForMultipleObjects(waitSet, NULL, 0, 0, &livePort, NULL);
            
            __CFRunLoopLock(rl);
            __CFRunLoopModeLock(rlm);            
            __CFRunLoopUnsetSleeping(rl);
            // If we have a new live port then it will be handled below as normal
        }
        
        
#endif
        if (CFPORT_NULL == livePort) {
            CFRUNLOOP_WAKEUP_FOR_NOTHING();
            cf_trace(KDEBUG_EVENT_CFRL_DID_WAKEUP_FOR_NOTHING, rl, rlm, livePort, 0);
            // handle nothing
        } else if (livePort == rl->_wakeUpPort) {
            CFRUNLOOP_WAKEUP_FOR_WAKEUP();
            cf_trace(KDEBUG_EVENT_CFRL_DID_WAKEUP_FOR_WAKEUP, rl, rlm, livePort, 0);
            // do nothing on Mac OS
#if TARGET_OS_WIN32
            // Always reset the wake up port, or risk spinning forever
            ResetEvent(rl->_wakeUpPort);
#endif
        }
#if USE_DISPATCH_SOURCE_FOR_TIMERS
        else if (modeQueuePort != MACH_PORT_NULL && livePort == modeQueuePort) {
            CFRUNLOOP_WAKEUP_FOR_TIMER();
            cf_trace(KDEBUG_EVENT_CFRL_DID_WAKEUP_FOR_TIMER, rl, rlm, livePort, 0);
            if (!__CFRunLoopDoTimers(rl, rlm, __CFNanosecondsToTSR(mach_absolute_time()))) {
                // Re-arm the next timer, because we apparently fired early
                __CFArmNextTimerInMode(rlm, rl);
            }
        }
#endif
        else if (rlm->_timerPort != CFPORT_NULL && livePort == rlm->_timerPort) {
            CFRUNLOOP_WAKEUP_FOR_TIMER();
            // On Windows, we have observed an issue where the timer port is set before the time which we requested it to be set. For example, we set the fire time to be TSR 167646765860, but it is actually observed firing at TSR 167646764145, which is 1715 ticks early. The result is that, when __CFRunLoopDoTimers checks to see if any of the run loop timers should be firing, it appears to be 'too early' for the next timer, and no timers are handled.
            // In this case, the timer port has been automatically reset (since it was returned from MsgWaitForMultipleObjectsEx), and if we do not re-arm it, then no timers will ever be serviced again unless something adjusts the timer list (e.g. adding or removing timers). The fix for the issue is to reset the timer here if CFRunLoopDoTimers did not handle a timer itself. 9308754
            if (!__CFRunLoopDoTimers(rl, rlm, __CFNanosecondsToTSR(mach_absolute_time()))) {
                // Re-arm the next timer
                // Since we'll be resetting the same timer as before
                // with the same deadlines, we need to reset these
                // values so that the arm next timer code can
                // correctly find the next timer in the list and arm
                // the underlying timer.
                rlm->_timerSoftDeadline = UINT64_MAX;
                rlm->_timerHardDeadline = UINT64_MAX;
                __CFArmNextTimerInMode(rlm, rl);
            }
        }
        
        /* --- DISPATCHES  --- */
        
#if __HAS_DISPATCH__
        else if (livePort == dispatchPort) {
            CFRUNLOOP_WAKEUP_FOR_DISPATCH();
            cf_trace(KDEBUG_EVENT_CFRL_DID_WAKEUP_FOR_DISPATCH, rl, rlm, livePort, 0);
            __CFRunLoopModeUnlock(rlm);
            __CFRunLoopUnlock(rl);
            _CFSetTSD(__CFTSDKeyIsInGCDMainQ, (void *)6, NULL);

#if TARGET_OS_WIN32 || TARGET_OS_LINUX || TARGET_OS_BSD
            void *msg = 0;
#endif
            CFRUNLOOP_ARP_BEGIN(NULL)
            cf_trace(KDEBUG_EVENT_CFRL_IS_CALLING_DISPATCH | DBG_FUNC_START, rl, rlm, msg, livePort);
            __CFRUNLOOP_IS_SERVICING_THE_MAIN_DISPATCH_QUEUE__(msg);
            cf_trace(KDEBUG_EVENT_CFRL_IS_CALLING_DISPATCH | DBG_FUNC_END, rl, rlm, msg, livePort);
            CFRUNLOOP_ARP_END()

            _CFSetTSD(__CFTSDKeyIsInGCDMainQ, (void *)0, NULL);
            __CFRunLoopLock(rl);
            __CFRunLoopModeLock(rlm);
            sourceHandledThisLoop = true;
            didDispatchPortLastTime = true;
        }
#endif
        
        /* --- SOURCE1S  --- */
        
        else {
            CFRUNLOOP_WAKEUP_FOR_SOURCE();
            cf_trace(KDEBUG_EVENT_CFRL_DID_WAKEUP_FOR_SOURCE, rl, rlm, 0, 0);
            // Despite the name, this works for windows handles as well
            CFRunLoopSourceRef rls = __CFRunLoopModeFindSourceForMachPort(rl, rlm, livePort);
            if (rls) {
#if TARGET_OS_MAC
		mach_msg_header_t *reply = NULL;
		sourceHandledThisLoop = __CFRunLoopDoSource1(rl, rlm, rls, msg, msg->msgh_size, &reply) || sourceHandledThisLoop;
		if (NULL != reply) {
		    (void)mach_msg(reply, MACH_SEND_MSG, reply->msgh_size, 0, MACH_PORT_NULL, 0, MACH_PORT_NULL);
		    CFAllocatorDeallocate(kCFAllocatorSystemDefault, reply);
		}
#elif TARGET_OS_WIN32 || (TARGET_OS_LINUX && !TARGET_OS_CYGWIN)
                sourceHandledThisLoop = __CFRunLoopDoSource1(rl, rlm, rls) || sourceHandledThisLoop;
#endif
            } else {
                os_log_error(_CFOSLog(), "__CFRunLoopModeFindSourceForMachPort returned NULL for mode '%@' livePort: %u", rlm->_name, livePort);
            }
            
        }
        
        /* --- BLOCKS --- */
        
#if TARGET_OS_MAC
        if (msg && msg != (mach_msg_header_t *)msg_buffer) free(msg);
#endif
        
	__CFRunLoopDoBlocks(rl, rlm);
        
	if (sourceHandledThisLoop && stopAfterHandle) {
	    retVal = kCFRunLoopRunHandledSource;
        } else if (termTSR < __CFNanosecondsToTSR(mach_absolute_time())) {
            retVal = kCFRunLoopRunTimedOut;
	} else if (__CFRunLoopIsStopped(rl)) {
            __CFRunLoopUnsetStopped(rl);
	    retVal = kCFRunLoopRunStopped;
	} else if (rlm->_stopped) {
	    rlm->_stopped = false;
	    retVal = kCFRunLoopRunStopped;
	} else if (__CFRunLoopModeIsEmpty(rl, rlm, previousMode)) {
	    retVal = kCFRunLoopRunFinished;
	}

    } while (0 == retVal);
#if __HAS_DISPATCH__
    if (timeout_timer) {
        dispatch_source_cancel(timeout_timer);
        dispatch_release(timeout_timer);
    }
#endif
    
    return retVal;
}

CF_BREAKPOINT_FUNCTION(void _CFRunLoopError_RunCalledWithInvalidMode(void));

SInt32 CFRunLoopRunSpecific(CFRunLoopRef rl, CFStringRef modeName, CFTimeInterval seconds, Boolean returnAfterSourceHandled) {     /* DOES CALLOUT */
    CF_ASSERT_TYPE(_kCFRuntimeIDCFRunLoop, rl);
    CHECK_FOR_FORK();
    if (modeName == NULL || modeName == kCFRunLoopCommonModes || CFEqual(modeName, kCFRunLoopCommonModes)) {
        static dispatch_once_t onceToken;
        dispatch_once(&onceToken, ^{
            CFLog(kCFLogLevelError, CFSTR("invalid mode '%@' provided to CFRunLoopRunSpecific - break on _CFRunLoopError_RunCalledWithInvalidMode to debug. This message will only appear once per execution."), modeName);
            _CFRunLoopError_RunCalledWithInvalidMode();
        });
        return kCFRunLoopRunFinished;
    }
    if (__CFRunLoopIsDeallocating(rl)) return kCFRunLoopRunFinished;
    __CFRunLoopLock(rl);
    CFRunLoopModeRef currentMode = __CFRunLoopCopyMode(rl, modeName, false);
    if (NULL == currentMode || __CFRunLoopModeIsEmpty(rl, currentMode, rl->_currentMode)) {
	if (currentMode) {
            __CFRunLoopModeUnlock(currentMode);
            CFRelease(currentMode);
        }
	__CFRunLoopUnlock(rl);
	return kCFRunLoopRunFinished;
    }
    __CFRunLoopModeLock(currentMode);
    volatile _per_run_data *previousPerRun = __CFRunLoopPushPerRunData(rl);
    CFRunLoopModeRef previousMode = rl->_currentMode;
    rl->_currentMode = currentMode;
    int32_t result = kCFRunLoopRunFinished;

	if (currentMode->_observerMask & kCFRunLoopEntry ) __CFRunLoopDoObservers(rl, currentMode, kCFRunLoopEntry);
        cf_trace(KDEBUG_EVENT_CFRL_RUN | DBG_FUNC_START, rl, currentMode, seconds, previousMode);
        result = __CFRunLoopRun(rl, currentMode, seconds, returnAfterSourceHandled, previousMode);
        cf_trace(KDEBUG_EVENT_CFRL_RUN | DBG_FUNC_END, rl, currentMode, seconds, previousMode);
        if (currentMode->_observerMask & kCFRunLoopExit ) __CFRunLoopDoObservers(rl, currentMode, kCFRunLoopExit);

        __CFRunLoopModeUnlock(currentMode);
        CFRelease(currentMode);
        __CFRunLoopPopPerRunData(rl, previousPerRun);
        rl->_currentMode = previousMode;
    __CFRunLoopUnlock(rl);
    return result;
}

void CFRunLoopRun(void) {	/* DOES CALLOUT */
    int32_t result;
    do {
        result = CFRunLoopRunSpecific(CFRunLoopGetCurrent(), kCFRunLoopDefaultMode, 1.0e10, false);
        CHECK_FOR_FORK();
    } while (kCFRunLoopRunStopped != result && kCFRunLoopRunFinished != result);
}

SInt32 CFRunLoopRunInMode(CFStringRef modeName, CFTimeInterval seconds, Boolean returnAfterSourceHandled) {     /* DOES CALLOUT */
    CHECK_FOR_FORK();
    return CFRunLoopRunSpecific(CFRunLoopGetCurrent(), modeName, seconds, returnAfterSourceHandled);
}

CFAbsoluteTime CFRunLoopGetNextTimerFireDate(CFRunLoopRef rl, CFStringRef modeName) {
    CF_ASSERT_TYPE(_kCFRuntimeIDCFRunLoop, rl);
    CHECK_FOR_FORK();
    __CFRunLoopLock(rl);
    CFRunLoopModeRef rlm = __CFRunLoopCopyMode(rl, modeName, false);
    if (rlm) {
        __CFRunLoopModeLock(rlm);
    }
    CFAbsoluteTime at = 0.0;
    CFRunLoopTimerRef nextTimer = (rlm && rlm->_timers && 0 < CFArrayGetCount(rlm->_timers)) ? (CFRunLoopTimerRef)CFArrayGetValueAtIndex(rlm->_timers, 0) : NULL;
    if (nextTimer) {
        at = CFRunLoopTimerGetNextFireDate(nextTimer);
    }
    if (rlm) {
        __CFRunLoopModeUnlock(rlm);
        CFRelease(rlm);
    }
    __CFRunLoopUnlock(rl);
    return at;
}

Boolean CFRunLoopIsWaiting(CFRunLoopRef rl) {
    CF_ASSERT_TYPE(_kCFRuntimeIDCFRunLoop, rl);
    CHECK_FOR_FORK();
    return __CFRunLoopIsSleeping(rl);
}

void CFRunLoopWakeUp(CFRunLoopRef rl) {
    CHECK_FOR_FORK();
    if (__CFMainThreadHasExited && rl == CFRunLoopGetMain()) {
        static dispatch_once_t onceToken;
        dispatch_once(&onceToken, ^{
            os_log_error(_CFOSLog(), "Attempting to wake up main runloop, but the main thread as exited. This message will only log once. Break on _CFRunLoopError_MainThreadHasExited to debug.");
        });
        _CFRunLoopError_MainThreadHasExited();
        return;
    }
    // Temporarily relocating type check AFTER the above pointer comparison to CFRunLoopGetMain() to avoid 60187188.
    CF_ASSERT_TYPE(_kCFRuntimeIDCFRunLoop, rl);

    // This lock is crucial to ignorable wakeups, do not remove it.
    __CFRunLoopLock(rl);
    if (__CFRunLoopIsIgnoringWakeUps(rl)) {
        __CFRunLoopUnlock(rl);
        return;
    }
    
    cf_trace(KDEBUG_EVENT_CFRL_WAKEUP | DBG_FUNC_START, rl, 0, 0, 0);
    
#if TARGET_OS_MAC
    kern_return_t ret;
    /* We unconditionally try to send the message, since we don't want
     * to lose a wakeup, but the send may fail if there is already a
     * wakeup pending, since the queue length is 1. */
    ret = __CFSendTrivialMachMessage(rl->_wakeUpPort, 0, MACH_SEND_TIMEOUT, 0);
    if (ret != MACH_MSG_SUCCESS && ret != MACH_SEND_TIMED_OUT) CRASH("*** Unable to send message to wake up port. (%x) ***", ret);
#elif TARGET_OS_LINUX && !TARGET_OS_CYGWIN
    int ret;
    do {
        ret = eventfd_write(rl->_wakeUpPort, 1);
    } while (ret == -1 && errno == EINTR);
    
    CFAssert1(0 == ret, __kCFLogAssertion, "%s(): Unable to send wake message to eventfd", __PRETTY_FUNCTION__);
#elif TARGET_OS_WIN32
    SetEvent(rl->_wakeUpPort);
#elif TARGET_OS_BSD
    __CFPortTrigger(rl->_wakeUpPort);
#else
#error "required"
#endif
    
    cf_trace(KDEBUG_EVENT_CFRL_WAKEUP | DBG_FUNC_END, rl, 0, 0, 0);
    
    __CFRunLoopUnlock(rl);
}

void CFRunLoopStop(CFRunLoopRef rl) {
    CF_ASSERT_TYPE(_kCFRuntimeIDCFRunLoop, rl);
    Boolean doWake = false;
    CHECK_FOR_FORK();
    __CFRunLoopLock(rl);
    if (rl->_currentMode) {
        __CFRunLoopSetStopped(rl);
        doWake = true;
    }
    __CFRunLoopUnlock(rl);
    if (doWake) {
        CFRunLoopWakeUp(rl);
    }
}

CF_EXPORT void _CFRunLoopStopMode(CFRunLoopRef rl, CFStringRef modeName) {
    CHECK_FOR_FORK();
    CFRunLoopModeRef rlm;
    __CFRunLoopLock(rl);
    rlm = __CFRunLoopCopyMode(rl, modeName, true);
    if (NULL != rlm) {
        __CFRunLoopModeLock(rlm);
	rlm->_stopped = true;
	__CFRunLoopModeUnlock(rlm);
        CFRelease(rlm);
    }
    __CFRunLoopUnlock(rl);
    CFRunLoopWakeUp(rl);
}

CF_EXPORT Boolean _CFRunLoopModeContainsMode(CFRunLoopRef rl, CFStringRef modeName, CFStringRef candidateContainedName) {
    CHECK_FOR_FORK();
    return false;
}

void CFRunLoopPerformBlock(CFRunLoopRef rl, CFTypeRef mode, void (^block)(void)) {
    CHECK_FOR_FORK();
    if (__CFMainThreadHasExited && rl == CFRunLoopGetMain()) {
        static dispatch_once_t onceToken;
        dispatch_once(&onceToken, ^{
            os_log_error(_CFOSLog(), "Attempting to perform block on main runloop, but the main thread has exited. This message will only log once. Break on _CFRunLoopError_MainThreadHasExited to debug.");
        });
        _CFRunLoopError_MainThreadHasExited();
        return;
    }
    // Temporarily relocating type check AFTER the above pointer comparison to CFRunLoopGetMain() to avoid 60187188.
    CF_ASSERT_TYPE(_kCFRuntimeIDCFRunLoop, rl);
    
    if (_kCFRuntimeIDCFString == CFGetTypeID(mode)) {
	mode = CFStringCreateCopy(kCFAllocatorSystemDefault, (CFStringRef)mode);
        __CFRunLoopLock(rl);
	// ensure mode exists
        CFRunLoopModeRef currentMode = __CFRunLoopCopyMode(rl, (CFStringRef)mode, true);
        if (currentMode) {
            CFRelease(currentMode);
        }
        __CFRunLoopUnlock(rl);
    } else if (CFArrayGetTypeID() == CFGetTypeID(mode)) {
        CFIndex cnt = CFArrayGetCount((CFArrayRef)mode);
	const void **values = (const void **)malloc(sizeof(const void *) * cnt);
        CFArrayGetValues((CFArrayRef)mode, CFRangeMake(0, cnt), values);
	mode = CFSetCreate(kCFAllocatorSystemDefault, values, cnt, &kCFTypeSetCallBacks);
        __CFRunLoopLock(rl);
	// ensure modes exist
	for (CFIndex idx = 0; idx < cnt; idx++) {
            CFRunLoopModeRef currentMode = __CFRunLoopCopyMode(rl, (CFStringRef)values[idx], true);
            if (currentMode) {
                CFRelease(currentMode);
            }
	}
        __CFRunLoopUnlock(rl);
	free(values);
    } else if (CFSetGetTypeID() == CFGetTypeID(mode)) {
        CFIndex cnt = CFSetGetCount((CFSetRef)mode);
	const void **values = (const void **)malloc(sizeof(const void *) * cnt);
        CFSetGetValues((CFSetRef)mode, values);
	mode = CFSetCreate(kCFAllocatorSystemDefault, values, cnt, &kCFTypeSetCallBacks);
        __CFRunLoopLock(rl);
	// ensure modes exist
	for (CFIndex idx = 0; idx < cnt; idx++) {
            CFRunLoopModeRef currentMode = __CFRunLoopCopyMode(rl, (CFStringRef)values[idx], true);
            if (currentMode) {
                CFRelease(currentMode);
            }
	}
        __CFRunLoopUnlock(rl);
	free(values);
    } else {
	mode = NULL;
    }
    block = Block_copy(block);
    if (!mode || !block) {
	if (mode) CFRelease(mode);
	if (block) Block_release(block);
	return;
    }
    __CFRunLoopLock(rl);
    struct _block_item *new_item = (struct _block_item *)malloc(sizeof(struct _block_item));
    new_item->_next = NULL;
    new_item->_mode = mode;
    new_item->_block = block;
    if (!rl->_blocks_tail) {
	rl->_blocks_head = new_item;
    } else {
	rl->_blocks_tail->_next = new_item;
    }
    rl->_blocks_tail = new_item;
    __CFRunLoopUnlock(rl);
}

Boolean _CFRunLoopPerCalloutAutoreleasepoolEnabled(void) {
    return CFRunLoopGetCurrent()->_perCalloutARP;
}

Boolean _CFRunLoopSetPerCalloutAutoreleasepoolEnabled(Boolean enabled) {
    CFRunLoopRef rl = CFRunLoopGetCurrent();
    return rl->_perCalloutARP = enabled;
}

Boolean CFRunLoopContainsSource(CFRunLoopRef rl, CFRunLoopSourceRef rls, CFStringRef modeName) {
    CF_ASSERT_TYPE(_kCFRuntimeIDCFRunLoop, rl);
    CHECK_FOR_FORK();
    CFRunLoopModeRef rlm;
    Boolean hasValue = false;
    __CFRunLoopLock(rl);
    if (modeName == kCFRunLoopCommonModes) {
	if (NULL != rl->_commonModeItems) {
	    hasValue = CFSetContainsValue(rl->_commonModeItems, rls);
	}
    } else {
	rlm = __CFRunLoopCopyMode(rl, modeName, false);
	if (NULL != rlm) {
            __CFRunLoopModeLock(rlm);
            hasValue = (rlm->_sources0 ? CFSetContainsValue(rlm->_sources0, rls) : false) || (rlm->_sources1 ? CFSetContainsValue(rlm->_sources1, rls) : false);
	    __CFRunLoopModeUnlock(rlm);
            CFRelease(rlm);
	}
    }
    __CFRunLoopUnlock(rl);
    return hasValue;
}

CF_BREAKPOINT_FUNCTION(void __CFRunLoopError_AddingSourceLackingReceiveRight(void));

void CFRunLoopAddSource(CFRunLoopRef rl, CFRunLoopSourceRef rls, CFStringRef modeName) {	/* DOES CALLOUT */
    CHECK_FOR_FORK();
    if (__CFRunLoopIsDeallocating(rl)) return;
    if (__CFMainThreadHasExited && rl == CFRunLoopGetMain()) {
        static dispatch_once_t onceToken;
        dispatch_once(&onceToken, ^{
            CFLog(kCFLogLevelError, CFSTR("Attempting to add source to main runloop, but the main thread has exited. This message will only log once. Break on _CFRunLoopError_MainThreadHasExited to debug."));
        });
        _CFRunLoopError_MainThreadHasExited();
        return;
    }
    // Temporarily relocating type check AFTER the above pointer comparison to CFRunLoopGetMain() to avoid 60187188.
    CF_ASSERT_TYPE(_kCFRuntimeIDCFRunLoop, rl);

    
#if TARGET_OS_MAC
    // Preflight Version-1 ports to make sure their mach port has a RECV right. 
    if (rls->_context.version0.version == 1) {
        mach_port_t const mp = rls->_context.version1.getPort(rls->_context.version1.info);
        mach_port_type_t type = MACH_PORT_TYPE_NONE;
        kern_return_t const ret = mach_port_type(mach_task_self(), mp, &type);
        if (ret == KERN_SUCCESS && (type & MACH_PORT_TYPE_RECEIVE) == 0) {
            static dispatch_once_t onceToken;
            dispatch_once(&onceToken, ^{
                CFLog(kCFLogLevelError, CFSTR("Attempting to add a runloop source %p to runloop %p in mode '%@', but the port associated with this source does not have a receive right. This means your source will never be signalled and is likely an error. Break on __CFRunLoopError_AddingSourceLackingReceiveRight to debug. This message will only appear once per execution."), rls, rl, modeName);
            });
            __CFRunLoopError_AddingSourceLackingReceiveRight();
        }
    }
#endif
    
    if (!__CFIsValid(rls)) return;
    Boolean doVer0Callout = false;
    __CFRunLoopLock(rl);
    if (modeName == kCFRunLoopCommonModes) {
	CFSetRef set = rl->_commonModes ? CFSetCreateCopy(kCFAllocatorSystemDefault, rl->_commonModes) : NULL;
	if (NULL == rl->_commonModeItems) {
	    rl->_commonModeItems = CFSetCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeSetCallBacks);
	}
	CFSetAddValue(rl->_commonModeItems, rls);
	if (NULL != set) {
	    CFTypeRef context[2] = {rl, rls};
	    /* add new item to all common-modes */
	    CFSetApplyFunction(set, (__CFRunLoopAddItemToCommonModes), (void *)context);
	    CFRelease(set);
	}
    } else {
	CFRunLoopModeRef rlm = __CFRunLoopCopyMode(rl, modeName, true);
        if (rlm) {
            __CFRunLoopModeLock(rlm);
        }
	if (NULL != rlm && NULL == rlm->_sources0) {
	    rlm->_sources0 = CFSetCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeSetCallBacks);
	    rlm->_sources1 = CFSetCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeSetCallBacks);
	    rlm->_portToV1SourceMap = CFDictionaryCreateMutable(kCFAllocatorSystemDefault, 0, NULL, NULL);
	}
	if (NULL != rlm && !CFSetContainsValue(rlm->_sources0, rls) && !CFSetContainsValue(rlm->_sources1, rls)) {
	    if (0 == rls->_context.version0.version) {
	        CFSetAddValue(rlm->_sources0, rls);
	    } else if (1 == rls->_context.version0.version) {
	        CFSetAddValue(rlm->_sources1, rls);
		__CFPort src_port = rls->_context.version1.getPort(rls->_context.version1.info);
		if (CFPORT_NULL != src_port) {
		    CFDictionarySetValue(rlm->_portToV1SourceMap, (const void *)(uintptr_t)src_port, rls);
		    __CFPortSetInsert(src_port, rlm->_portSet);
	        }
	    }
	    __CFRunLoopSourceLock(rls);
	    if (NULL == rls->_runLoops) {
	        rls->_runLoops = CFBagCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeBagCallBacks); // sources retain run loops!
	    }
	    CFBagAddValue(rls->_runLoops, rl);
	    __CFRunLoopSourceUnlock(rls);
	    if (0 == rls->_context.version0.version) {
	        if (NULL != rls->_context.version0.schedule) {
	            doVer0Callout = true;
	        }
	    }
	}
        if (NULL != rlm) {
	    __CFRunLoopModeUnlock(rlm);
            CFRelease(rlm);
	}
    }
    __CFRunLoopUnlock(rl);
    if (doVer0Callout) {
        // although it looses some protection for the source, we have no choice but
        // to do this after unlocking the run loop and mode locks, to avoid deadlocks
        // where the source wants to take a lock which is already held in another
        // thread which is itself waiting for a run loop/mode lock
	rls->_context.version0.schedule(rls->_context.version0.info, rl, modeName);	/* CALLOUT */
    }
}

void CFRunLoopRemoveSource(CFRunLoopRef rl, CFRunLoopSourceRef rls, CFStringRef modeName) {	/* DOES CALLOUT */
    CF_ASSERT_TYPE(_kCFRuntimeIDCFRunLoop, rl);
    CHECK_FOR_FORK();
    Boolean doVer0Callout = false, doRLSRelease = false;
    __CFRunLoopLock(rl);
    if (modeName == kCFRunLoopCommonModes) {
	if (NULL != rl->_commonModeItems && CFSetContainsValue(rl->_commonModeItems, rls)) {
	    CFSetRef set = rl->_commonModes ? CFSetCreateCopy(kCFAllocatorSystemDefault, rl->_commonModes) : NULL;
	    CFSetRemoveValue(rl->_commonModeItems, rls);
	    if (NULL != set) {
		CFTypeRef context[2] = {rl, rls};
		/* remove new item from all common-modes */
		CFSetApplyFunction(set, (__CFRunLoopRemoveItemFromCommonModes), (void *)context);
		CFRelease(set);
	    }
	} else {
	}
    } else {
	CFRunLoopModeRef rlm = __CFRunLoopCopyMode(rl, modeName, false);
        if (rlm) {
            __CFRunLoopModeLock(rlm);
        }
	if (NULL != rlm && ((NULL != rlm->_sources0 && CFSetContainsValue(rlm->_sources0, rls)) || (NULL != rlm->_sources1 && CFSetContainsValue(rlm->_sources1, rls)))) {
	    CFRetain(rls);
	    if (1 == rls->_context.version0.version) {
		__CFPort src_port = rls->_context.version1.getPort(rls->_context.version1.info);
                if (CFPORT_NULL != src_port) {
		    CFDictionaryRemoveValue(rlm->_portToV1SourceMap, (const void *)(uintptr_t)src_port);
                    __CFPortSetRemove(src_port, rlm->_portSet);
                }
	    }
	    CFSetRemoveValue(rlm->_sources0, rls);
	    CFSetRemoveValue(rlm->_sources1, rls);
            __CFRunLoopSourceLock(rls);
            if (NULL != rls->_runLoops) {
                CFBagRemoveValue(rls->_runLoops, rl);
            }
            __CFRunLoopSourceUnlock(rls);
	    if (0 == rls->_context.version0.version) {
	        if (NULL != rls->_context.version0.cancel) {
	            doVer0Callout = true;
	        }
	    }
	    doRLSRelease = true;
	}
        if (NULL != rlm) {
	    __CFRunLoopModeUnlock(rlm);
            CFRelease(rlm);
	}
    }
    __CFRunLoopUnlock(rl);
    if (doVer0Callout) {
        // although it looses some protection for the source, we have no choice but
        // to do this after unlocking the run loop and mode locks, to avoid deadlocks
        // where the source wants to take a lock which is already held in another
        // thread which is itself waiting for a run loop/mode lock
        rls->_context.version0.cancel(rls->_context.version0.info, rl, modeName);	/* CALLOUT */
    }
    if (doRLSRelease) CFRelease(rls);
}

static void __CFRunLoopRemoveSourcesFromCommonMode(const void *value, void *ctx) {
    CFStringRef modeName = (CFStringRef)value;
    CFRunLoopRef rl = (CFRunLoopRef)ctx;
    __CFRunLoopRemoveAllSources(rl, modeName);
}

static void __CFRunLoopRemoveSourceFromMode(const void *value, void *ctx) {
    CFRunLoopSourceRef rls = (CFRunLoopSourceRef)value;
    CFRunLoopRef rl = (CFRunLoopRef)(((CFTypeRef *)ctx)[0]);
    CFStringRef modeName = (CFStringRef)(((CFTypeRef *)ctx)[1]);
    CFRunLoopRemoveSource(rl, rls, modeName);
}

static void __CFRunLoopRemoveAllSources(CFRunLoopRef rl, CFStringRef modeName) {
    CHECK_FOR_FORK();
    CFRunLoopModeRef rlm;
    __CFRunLoopLock(rl);
    if (modeName == kCFRunLoopCommonModes) {
	if (NULL != rl->_commonModeItems) {
	    CFSetRef set = rl->_commonModes ? CFSetCreateCopy(kCFAllocatorSystemDefault, rl->_commonModes) : NULL;
	    if (NULL != set) {
                CFSetApplyFunction(set, (__CFRunLoopRemoveSourcesFromCommonMode), (void *)rl);
		CFRelease(set);
	    }
	} else {
	}
    } else {
	rlm = __CFRunLoopCopyMode(rl, modeName, false);
        if (rlm) {
            __CFRunLoopModeLock(rlm);
        }
	if (NULL != rlm && NULL != rlm->_sources0) {
	    CFSetRef set = CFSetCreateCopy(kCFAllocatorSystemDefault, rlm->_sources0);
            CFTypeRef context[2] = {rl, modeName};
            CFSetApplyFunction(set, (__CFRunLoopRemoveSourceFromMode), (void *)context);
	    CFRelease(set);
	}
	if (NULL != rlm && NULL != rlm->_sources1) {
	    CFSetRef set = CFSetCreateCopy(kCFAllocatorSystemDefault, rlm->_sources1);
            CFTypeRef context[2] = {rl, modeName};
            CFSetApplyFunction(set, (__CFRunLoopRemoveSourceFromMode), (void *)context);
	    CFRelease(set);
	}
        if (NULL != rlm) {
	    __CFRunLoopModeUnlock(rlm);
            CFRelease(rlm);
	}
    }
    __CFRunLoopUnlock(rl);
}

Boolean CFRunLoopContainsObserver(CFRunLoopRef rl, CFRunLoopObserverRef rlo, CFStringRef modeName) {
    CF_ASSERT_TYPE(_kCFRuntimeIDCFRunLoop, rl);
    CHECK_FOR_FORK();
    CFRunLoopModeRef rlm;
    Boolean hasValue = false;
    __CFRunLoopLock(rl);
    if (modeName == kCFRunLoopCommonModes) {
	if (NULL != rl->_commonModeItems) {
	    hasValue = CFSetContainsValue(rl->_commonModeItems, rlo);
	}
    } else {
	rlm = __CFRunLoopCopyMode(rl, modeName, false);
        if (rlm) {
            __CFRunLoopModeLock(rlm);
        }
	if (NULL != rlm && NULL != rlm->_observers) {
	    hasValue = CFArrayContainsValue(rlm->_observers, CFRangeMake(0, CFArrayGetCount(rlm->_observers)), rlo);
	}
        if (NULL != rlm) {
	    __CFRunLoopModeUnlock(rlm);
            CFRelease(rlm);
	}
    }
    __CFRunLoopUnlock(rl);
    return hasValue;
}

void CFRunLoopAddObserver(CFRunLoopRef rl, CFRunLoopObserverRef rlo, CFStringRef modeName) {
    CHECK_FOR_FORK();
    CFRunLoopModeRef rlm;
    if (__CFRunLoopIsDeallocating(rl)) return;
    if (__CFMainThreadHasExited && rl == CFRunLoopGetMain()) {
        static dispatch_once_t onceToken;
        dispatch_once(&onceToken, ^{
            CFLog(kCFLogLevelError, CFSTR("Attempting to add observer to main runloop, but the main thread has exited. This message will only log once. Break on _CFRunLoopError_MainThreadHasExited to debug."));
        });
        _CFRunLoopError_MainThreadHasExited();
        return;
    }
    // Temporarily relocating type check AFTER the above pointer comparison to CFRunLoopGetMain() to avoid 60187188.
    CF_ASSERT_TYPE(_kCFRuntimeIDCFRunLoop, rl);
    
    if (!__CFIsValid(rlo) || (NULL != rlo->_runLoop && rlo->_runLoop != rl)) return;
    __CFRunLoopLock(rl);
    if (modeName == kCFRunLoopCommonModes) {
	CFSetRef set = rl->_commonModes ? CFSetCreateCopy(kCFAllocatorSystemDefault, rl->_commonModes) : NULL;
	if (NULL == rl->_commonModeItems) {
	    rl->_commonModeItems = CFSetCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeSetCallBacks);
	}
	CFSetAddValue(rl->_commonModeItems, rlo);
	if (NULL != set) {
	    CFTypeRef context[2] = {rl, rlo};
	    /* add new item to all common-modes */
	    CFSetApplyFunction(set, (__CFRunLoopAddItemToCommonModes), (void *)context);
	    CFRelease(set);
	}
    } else {
	rlm = __CFRunLoopCopyMode(rl, modeName, true);
        if (rlm) {
            __CFRunLoopModeLock(rlm);
        }
	if (NULL != rlm && NULL == rlm->_observers) {
	    rlm->_observers = CFArrayCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeArrayCallBacks);
	}
	if (NULL != rlm && !CFArrayContainsValue(rlm->_observers, CFRangeMake(0, CFArrayGetCount(rlm->_observers)), rlo)) {
            Boolean inserted = false;
            for (CFIndex idx = CFArrayGetCount(rlm->_observers); idx--; ) {
                CFRunLoopObserverRef obs = (CFRunLoopObserverRef)CFArrayGetValueAtIndex(rlm->_observers, idx);
                if (obs->_order <= rlo->_order) {
                    CFArrayInsertValueAtIndex(rlm->_observers, idx + 1, rlo);
                    inserted = true;
                    break;
                }
            }
            if (!inserted) {
	        CFArrayInsertValueAtIndex(rlm->_observers, 0, rlo);
            }
	    rlm->_observerMask |= rlo->_activities;
	    __CFRunLoopObserverSchedule(rlo, rl, rlm);
	}
        if (NULL != rlm) {
	    __CFRunLoopModeUnlock(rlm);
            CFRelease(rlm);
	}
    }
    __CFRunLoopUnlock(rl);
}

void CFRunLoopRemoveObserver(CFRunLoopRef rl, CFRunLoopObserverRef rlo, CFStringRef modeName) {
    CF_ASSERT_TYPE(_kCFRuntimeIDCFRunLoop, rl);
    CHECK_FOR_FORK();
    CFRunLoopModeRef rlm;
    __CFRunLoopLock(rl);
    if (modeName == kCFRunLoopCommonModes) {
	if (NULL != rl->_commonModeItems && CFSetContainsValue(rl->_commonModeItems, rlo)) {
	    CFSetRef set = rl->_commonModes ? CFSetCreateCopy(kCFAllocatorSystemDefault, rl->_commonModes) : NULL;
	    CFSetRemoveValue(rl->_commonModeItems, rlo);
	    if (NULL != set) {
		CFTypeRef context[2] = {rl, rlo};
		/* remove new item from all common-modes */
		CFSetApplyFunction(set, (__CFRunLoopRemoveItemFromCommonModes), (void *)context);
		CFRelease(set);
	    }
	} else {
	}
    } else {
	rlm = __CFRunLoopCopyMode(rl, modeName, false);
        if (rlm) {
            __CFRunLoopModeLock(rlm);
        }
	if (NULL != rlm && NULL != rlm->_observers) {
	    CFRetain(rlo);
            CFIndex idx = CFArrayGetFirstIndexOfValue(rlm->_observers, CFRangeMake(0, CFArrayGetCount(rlm->_observers)), rlo);
            if (kCFNotFound != idx) {
                CFArrayRemoveValueAtIndex(rlm->_observers, idx);
	        __CFRunLoopObserverCancel(rlo, rl, rlm);
            }
	    CFRelease(rlo);
	}
        if (NULL != rlm) {
	    __CFRunLoopModeUnlock(rlm);
            CFRelease(rlm);
	}
    }
    __CFRunLoopUnlock(rl);
}

Boolean CFRunLoopContainsTimer(CFRunLoopRef rl, CFRunLoopTimerRef rlt, CFStringRef modeName) {
    CF_ASSERT_TYPE(_kCFRuntimeIDCFRunLoop, rl);
    CHECK_FOR_FORK();
    if (NULL == rlt->_runLoop || rl != rlt->_runLoop) return false;
    Boolean hasValue = false;
    __CFRunLoopLock(rl);
    if (modeName == kCFRunLoopCommonModes) {
	if (NULL != rl->_commonModeItems) {
	    hasValue = CFSetContainsValue(rl->_commonModeItems, rlt);
	}
    } else {
	CFRunLoopModeRef rlm = __CFRunLoopCopyMode(rl, modeName, false);
	if (NULL != rlm) {
            __CFRunLoopModeLock(rlm);
            if (NULL != rlm->_timers) {
                CFIndex idx = CFArrayGetFirstIndexOfValue(rlm->_timers, CFRangeMake(0, CFArrayGetCount(rlm->_timers)), rlt);
                hasValue = (kCFNotFound != idx);
            }
	    __CFRunLoopModeUnlock(rlm);
            CFRelease(rlm);
	}
    }
    __CFRunLoopUnlock(rl);
    return hasValue;
}

void CFRunLoopAddTimer(CFRunLoopRef rl, CFRunLoopTimerRef rlt, CFStringRef modeName) {
    CHECK_FOR_FORK();
    if (__CFRunLoopIsDeallocating(rl)) return;
    if (__CFMainThreadHasExited && rl == CFRunLoopGetMain()) {
        static dispatch_once_t onceToken;
        dispatch_once(&onceToken, ^{
            CFLog(kCFLogLevelError, CFSTR("Attempting to add timer to main runloop, but the main thread as exited. This message will only log once. Break on _CFRunLoopError_MainThreadHasExited to debug."));
        });
        _CFRunLoopError_MainThreadHasExited();
        return;
    }
    // Temporarily relocating type check AFTER the above pointer comparison to CFRunLoopGetMain() to avoid 60187188.
    CF_ASSERT_TYPE(_kCFRuntimeIDCFRunLoop, rl);
    
    if (!__CFIsValid(rlt) || (NULL != rlt->_runLoop && rlt->_runLoop != rl)) return;
    __CFRunLoopLock(rl);
    if (modeName == kCFRunLoopCommonModes) {
	CFSetRef set = rl->_commonModes ? CFSetCreateCopy(kCFAllocatorSystemDefault, rl->_commonModes) : NULL;
	if (NULL == rl->_commonModeItems) {
	    rl->_commonModeItems = CFSetCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeSetCallBacks);
	}
	CFSetAddValue(rl->_commonModeItems, rlt);
	if (NULL != set) {
	    CFTypeRef context[2] = {rl, rlt};
	    /* add new item to all common-modes */
	    CFSetApplyFunction(set, (__CFRunLoopAddItemToCommonModes), (void *)context);
	    CFRelease(set);
	}
    } else {
	CFRunLoopModeRef rlm = __CFRunLoopCopyMode(rl, modeName, true);
	if (NULL != rlm) {
            __CFRunLoopModeLock(rlm);
            if (NULL == rlm->_timers) {
                CFArrayCallBacks cb = kCFTypeArrayCallBacks;
                cb.equal = NULL;
                rlm->_timers = CFArrayCreateMutable(kCFAllocatorSystemDefault, 0, &cb);
            }
	}
	if (NULL != rlm && !CFSetContainsValue(rlt->_rlModes, rlm->_name)) {
            __CFRunLoopTimerLock(rlt);
            if (NULL == rlt->_runLoop) {
		rlt->_runLoop = rl;
  	    } else if (rl != rlt->_runLoop) {
                __CFRunLoopTimerUnlock(rlt);
	        __CFRunLoopModeUnlock(rlm);
                CFRelease(rlm);
                __CFRunLoopUnlock(rl);
		return;
	    }
  	    CFSetAddValue(rlt->_rlModes, rlm->_name);
            __CFRunLoopTimerUnlock(rlt);
            __CFLock(&rl->_timerTSRLock);
            __CFRepositionTimerInMode(rlm, rlt, false);
            __CFUnlock(&rl->_timerTSRLock);
            if (!_CFExecutableLinkedOnOrAfter(CFSystemVersionLion)) {
                // Normally we don't do this on behalf of clients, but for
                // backwards compatibility due to the change in timer handling...
                if (!_CFRunLoopIsCurrent(rl)) { CFRunLoopWakeUp(rl); }
            }
	}
        if (NULL != rlm) {
	    __CFRunLoopModeUnlock(rlm);
            CFRelease(rlm);
	}
    }
    __CFRunLoopUnlock(rl);
}

void CFRunLoopRemoveTimer(CFRunLoopRef rl, CFRunLoopTimerRef rlt, CFStringRef modeName) {
    CF_ASSERT_TYPE(_kCFRuntimeIDCFRunLoop, rl);
    CHECK_FOR_FORK();
    __CFRunLoopLock(rl);
    if (modeName == kCFRunLoopCommonModes) {
	if (NULL != rl->_commonModeItems && CFSetContainsValue(rl->_commonModeItems, rlt)) {
	    CFSetRef set = rl->_commonModes ? CFSetCreateCopy(kCFAllocatorSystemDefault, rl->_commonModes) : NULL;
	    CFSetRemoveValue(rl->_commonModeItems, rlt);
	    if (NULL != set) {
		CFTypeRef context[2] = {rl, rlt};
		/* remove new item from all common-modes */
		CFSetApplyFunction(set, (__CFRunLoopRemoveItemFromCommonModes), (void *)context);
		CFRelease(set);
	    }
	} else {
	}
    } else {
	CFRunLoopModeRef rlm = __CFRunLoopCopyMode(rl, modeName, false);
        CFIndex idx = kCFNotFound;
        CFMutableArrayRef timerList = NULL;
        if (NULL != rlm) {
            __CFRunLoopModeLock(rlm);
            timerList = rlm->_timers;
            if (NULL != timerList) {
                idx = CFArrayGetFirstIndexOfValue(timerList, CFRangeMake(0, CFArrayGetCount(timerList)), rlt);
            }
        }
        if (kCFNotFound != idx) {
            __CFRunLoopTimerLock(rlt);
            CFSetRemoveValue(rlt->_rlModes, rlm->_name);
            if (0 == CFSetGetCount(rlt->_rlModes)) {
                rlt->_runLoop = NULL;
            }
            __CFRunLoopTimerUnlock(rlt);
	    CFArrayRemoveValueAtIndex(timerList, idx);
            __CFArmNextTimerInMode(rlm, rl);
        }
        if (NULL != rlm) {
	    __CFRunLoopModeUnlock(rlm);
            CFRelease(rlm);
	}
    }
    __CFRunLoopUnlock(rl);
}

/* CFRunLoopSource */

static Boolean __CFRunLoopSourceEqual(CFTypeRef cf1, CFTypeRef cf2) {	/* DOES CALLOUT */
    CFRunLoopSourceRef rls1 = (CFRunLoopSourceRef)cf1;
    CFRunLoopSourceRef rls2 = (CFRunLoopSourceRef)cf2;
    if (rls1 == rls2) return true;
    if (__CFIsValid(rls1) != __CFIsValid(rls2)) return false;
    if (rls1->_order != rls2->_order) return false;
    if (rls1->_context.version0.version != rls2->_context.version0.version) return false;
    if (rls1->_context.version0.hash != rls2->_context.version0.hash) return false;
    if (rls1->_context.version0.equal != rls2->_context.version0.equal) return false;
    if (0 == rls1->_context.version0.version && rls1->_context.version0.perform != rls2->_context.version0.perform) return false;
    if (1 == rls1->_context.version0.version && rls1->_context.version1.perform != rls2->_context.version1.perform) return false;
    if (rls1->_context.version0.equal)
	return rls1->_context.version0.equal(rls1->_context.version0.info, rls2->_context.version0.info);
    return (rls1->_context.version0.info == rls2->_context.version0.info);
}

static CFHashCode __CFRunLoopSourceHash(CFTypeRef cf) {	/* DOES CALLOUT */
    CFRunLoopSourceRef rls = (CFRunLoopSourceRef)cf;
    if (rls->_context.version0.hash)
	return rls->_context.version0.hash(rls->_context.version0.info);
    return (CFHashCode)rls->_context.version0.info;
}

static CFStringRef __CFRunLoopSourceCopyDescription(CFTypeRef cf) {	/* DOES CALLOUT */
    CFRunLoopSourceRef rls = (CFRunLoopSourceRef)cf;
    CFStringRef result;
    CFStringRef contextDesc = NULL;
    if (NULL != rls->_context.version0.copyDescription) {
	contextDesc = rls->_context.version0.copyDescription(rls->_context.version0.info);
    }
    if (NULL == contextDesc) {
	void *addr = rls->_context.version0.version == 0 ? (void *)rls->_context.version0.perform : (rls->_context.version0.version == 1 ? (void *)rls->_context.version1.perform : NULL);
#if TARGET_OS_WIN32
	contextDesc = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("<CFRunLoopSource context>{version = %ld, info = %p, callout = %p}"), rls->_context.version0.version, rls->_context.version0.info, addr);
#elif TARGET_OS_MAC || (TARGET_OS_LINUX && !TARGET_OS_CYGWIN)
	Dl_info info;
	const char *name = (dladdr(addr, &info) && info.dli_saddr == addr && info.dli_sname) ? info.dli_sname : "???";
	contextDesc = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("<CFRunLoopSource context>{version = %ld, info = %p, callout = %s (%p)}"), rls->_context.version0.version, rls->_context.version0.info, name, addr);
#endif
    }
#if TARGET_OS_WIN32
    result = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("<CFRunLoopSource %p [%p]>{signalled = %s, valid = %s, order = %d, context = %@}"), cf, CFGetAllocator(rls), __CFRunLoopSourceIsSignaled(rls) ? "Yes" : "No", __CFIsValid(rls) ? "Yes" : "No", rls->_order, contextDesc);
#else
    result = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("<CFRunLoopSource %p [%p]>{signalled = %s, valid = %s, order = %ld, context = %@}"), cf, CFGetAllocator(rls), __CFRunLoopSourceIsSignaled(rls) ? "Yes" : "No", __CFIsValid(rls) ? "Yes" : "No", (unsigned long)rls->_order, contextDesc);
#endif
    CFRelease(contextDesc);
    return result;
}

static void __CFRunLoopSourceDeallocate(CFTypeRef cf) {	/* DOES CALLOUT */
    CFRunLoopSourceRef rls = (CFRunLoopSourceRef)cf;
    CFRunLoopSourceInvalidate(rls);
    if (rls->_context.version0.release) {
	rls->_context.version0.release(rls->_context.version0.info);
    }
    _CFRecursiveMutexDestroy(&rls->_lock);
    memset((char *)cf + sizeof(CFRuntimeBase), 0, sizeof(struct __CFRunLoopSource) - sizeof(CFRuntimeBase));
}

const CFRuntimeClass __CFRunLoopSourceClass = {
    _kCFRuntimeScannedObject,
    "CFRunLoopSource",
    NULL,      // init
    NULL,      // copy
    __CFRunLoopSourceDeallocate,
    __CFRunLoopSourceEqual,
    __CFRunLoopSourceHash,
    NULL,      // 
    __CFRunLoopSourceCopyDescription
};

CFTypeID CFRunLoopSourceGetTypeID(void) {
    return _kCFRuntimeIDCFRunLoopSource;
}

CFRunLoopSourceRef CFRunLoopSourceCreate(CFAllocatorRef allocator, CFIndex order, CFRunLoopSourceContext *context) {
    CHECK_FOR_FORK();
    CFRunLoopSourceRef memory;
    uint32_t size;
    if (NULL == context) CRASH("*** NULL context value passed to CFRunLoopSourceCreate(). (%d) ***", -1);
    
    size = sizeof(struct __CFRunLoopSource) - sizeof(CFRuntimeBase);
    memory = (CFRunLoopSourceRef)_CFRuntimeCreateInstance(allocator, CFRunLoopSourceGetTypeID(), size, NULL);
    if (NULL == memory) {
	return NULL;
    }
    __CFSetValid(memory);
    __CFRunLoopSourceUnsetSignaled(memory);
    _CFRecursiveMutexCreate(&memory->_lock);
    memory->_order = order;
    size = 0;
    switch (context->version) {
    case 0:
	size = sizeof(CFRunLoopSourceContext);
	break;
    case 1:
	size = sizeof(CFRunLoopSourceContext1);
	break;
    }
    memmove(&memory->_context, context, size);
    if (context->retain) {
	memory->_context.version0.info = (void *)context->retain(context->info);
    }
    return memory;
}

CFIndex CFRunLoopSourceGetOrder(CFRunLoopSourceRef rls) {
    CF_ASSERT_TYPE(_kCFRuntimeIDCFRunLoopSource, rls);
    CHECK_FOR_FORK();
    __CFGenericValidateType(rls, CFRunLoopSourceGetTypeID());
    return rls->_order;
}

static void __CFRunLoopSourceWakeUpLoop(const void *value, void *context) {
    CFRunLoopWakeUp((CFRunLoopRef)value);
}

static void __CFRunLoopSourceRemoveFromRunLoop(const void *value, void *context) {
    CFRunLoopRef rl = (CFRunLoopRef)value;
    CFTypeRef *params = (CFTypeRef *)context;
    CFRunLoopSourceRef rls = (CFRunLoopSourceRef)params[0];
    CFIndex idx;
    if (rl == params[1]) return;

    // CFRunLoopRemoveSource will lock the run loop while it
    // needs that, but we also lock it out here to keep
    // changes from occurring for this whole sequence.
    __CFRunLoopLock(rl);
    CFArrayRef array = CFRunLoopCopyAllModes(rl);
    for (idx = CFArrayGetCount(array); idx--;) {
	CFStringRef modeName = (CFStringRef)CFArrayGetValueAtIndex(array, idx);
	CFRunLoopRemoveSource(rl, rls, modeName);
    }
    CFRunLoopRemoveSource(rl, rls, kCFRunLoopCommonModes);
    __CFRunLoopUnlock(rl);
    CFRelease(array);
    params[1] = rl;
}

void CFRunLoopSourceInvalidate(CFRunLoopSourceRef rls) {
    CF_ASSERT_TYPE(_kCFRuntimeIDCFRunLoopSource, rls);
    CHECK_FOR_FORK();
    __CFGenericValidateType(rls, CFRunLoopSourceGetTypeID());
    __CFRunLoopSourceLock(rls);
    CFRetain(rls);
    if (__CFIsValid(rls)) {
        CFBagRef rloops = rls->_runLoops;
        __CFUnsetValid(rls);
        __CFRunLoopSourceUnsetSignaled(rls);
        if (NULL != rloops) {
            // To avoid A->B, B->A lock ordering issues when coming up
            // towards the run loop from a source, the source has to be
            // unlocked, which means we have to protect from object
            // invalidation.
            rls->_runLoops = NULL; // transfer ownership to local stack
            __CFRunLoopSourceUnlock(rls);
            CFTypeRef params[2] = {rls, NULL};
            CFBagApplyFunction(rloops, (__CFRunLoopSourceRemoveFromRunLoop), params);
            CFRelease(rloops);
            __CFRunLoopSourceLock(rls);
        }
        /* for hashing- and equality-use purposes, can't actually release the context here */
    }
    __CFRunLoopSourceUnlock(rls);
    CFRelease(rls);
}

Boolean CFRunLoopSourceIsValid(CFRunLoopSourceRef rls) {
    CF_ASSERT_TYPE(_kCFRuntimeIDCFRunLoopSource, rls);
    CHECK_FOR_FORK();
    __CFGenericValidateType(rls, CFRunLoopSourceGetTypeID());
    return __CFIsValid(rls);
}

void CFRunLoopSourceGetContext(CFRunLoopSourceRef rls, CFRunLoopSourceContext *context) {
    CF_ASSERT_TYPE(_kCFRuntimeIDCFRunLoopSource, rls);
    CHECK_FOR_FORK();
    __CFGenericValidateType(rls, CFRunLoopSourceGetTypeID());
    CFAssert1(0 == context->version || 1 == context->version, __kCFLogAssertion, "%s(): context version not initialized to 0 or 1", __PRETTY_FUNCTION__);
    CFIndex size = 0;
    switch (context->version) {
    case 0:
	size = sizeof(CFRunLoopSourceContext);
	break;
    case 1:
	size = sizeof(CFRunLoopSourceContext1);
	break;
    }
    memmove(context, &rls->_context, size);
}

void CFRunLoopSourceSignal(CFRunLoopSourceRef rls) {
    CF_ASSERT_TYPE(_kCFRuntimeIDCFRunLoopSource, rls);
    CHECK_FOR_FORK();
    __CFRunLoopSourceLock(rls);
    if (__CFIsValid(rls)) {
	cf_trace(KDEBUG_EVENT_CFRL_SOURCE0_SIGNAL | DBG_FUNC_START, rls, rls->_context.version0.perform, rls->_context.version0.info, 0);
	__CFRunLoopSourceSetSignaled(rls);
	cf_trace(KDEBUG_EVENT_CFRL_SOURCE0_SIGNAL | DBG_FUNC_END, rls, rls->_context.version0.perform, rls->_context.version0.info, 0);
    }
    __CFRunLoopSourceUnlock(rls);
}

Boolean CFRunLoopSourceIsSignalled(CFRunLoopSourceRef rls) {
    CF_ASSERT_TYPE(_kCFRuntimeIDCFRunLoopSource, rls);
    CHECK_FOR_FORK();
    __CFRunLoopSourceLock(rls);
    Boolean ret = __CFRunLoopSourceIsSignaled(rls) ? true : false;
    __CFRunLoopSourceUnlock(rls);
    return ret;
}

CF_PRIVATE void _CFRunLoopSourceWakeUpRunLoops(CFRunLoopSourceRef rls) {
    CF_ASSERT_TYPE(_kCFRuntimeIDCFRunLoopSource, rls);
    CFBagRef loops = NULL;
    __CFRunLoopSourceLock(rls);
    if (__CFIsValid(rls) && NULL != rls->_runLoops) {
        loops = CFBagCreateCopy(kCFAllocatorSystemDefault, rls->_runLoops);
    }
    __CFRunLoopSourceUnlock(rls);
    if (loops) {
	CFBagApplyFunction(loops, __CFRunLoopSourceWakeUpLoop, NULL);
        CFRelease(loops);
    }
}

/* CFRunLoopObserver */

static CFStringRef __CFRunLoopObserverCopyDescription(CFTypeRef cf) {	/* DOES CALLOUT */
    CFRunLoopObserverRef rlo = (CFRunLoopObserverRef)cf;
    CFStringRef result;
    CFStringRef contextDesc = NULL;
    if (NULL != rlo->_context.copyDescription) {
	contextDesc = rlo->_context.copyDescription(rlo->_context.info);
    }
    if (!contextDesc) {
	contextDesc = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("<CFRunLoopObserver context %p>"), rlo->_context.info);
    }
#if TARGET_OS_WIN32
    result = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("<CFRunLoopObserver %p [%p]>{valid = %s, activities = 0x%x, repeats = %s, order = %d, callout = %p, context = %@}"), cf, CFGetAllocator(rlo), __CFIsValid(rlo) ? "Yes" : "No", rlo->_activities, __CFRunLoopObserverRepeats(rlo) ? "Yes" : "No", rlo->_order, rlo->_callout, contextDesc);    
#elif TARGET_OS_MAC || (TARGET_OS_LINUX && !TARGET_OS_CYGWIN)
    void *addr = rlo->_callout;
    Dl_info info;
    const char *name = (dladdr(addr, &info) && info.dli_saddr == addr && info.dli_sname) ? info.dli_sname : "???";
    result = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("<CFRunLoopObserver %p [%p]>{valid = %s, activities = 0x%lx, repeats = %s, order = %ld, callout = %s (%p), context = %@}"), cf, CFGetAllocator(rlo), __CFIsValid(rlo) ? "Yes" : "No", (long)rlo->_activities, __CFRunLoopObserverRepeats(rlo) ? "Yes" : "No", (long)rlo->_order, name, addr, contextDesc);
#endif
    CFRelease(contextDesc);
    return result;
}

static void __CFRunLoopObserverDeallocate(CFTypeRef cf) {	/* DOES CALLOUT */
    CFRunLoopObserverRef rlo = (CFRunLoopObserverRef)cf;
    CFRunLoopObserverInvalidate(rlo);
    _CFRecursiveMutexDestroy(&rlo->_lock);
}

const CFRuntimeClass __CFRunLoopObserverClass = {
    0,
    "CFRunLoopObserver",
    NULL,      // init
    NULL,      // copy
    __CFRunLoopObserverDeallocate,
    NULL,
    NULL,
    NULL,      // 
    __CFRunLoopObserverCopyDescription
};

CFTypeID CFRunLoopObserverGetTypeID(void) {
    return _kCFRuntimeIDCFRunLoopObserver;
}

CFRunLoopObserverRef CFRunLoopObserverCreate(CFAllocatorRef allocator, CFOptionFlags activities, Boolean repeats, CFIndex order, CFRunLoopObserverCallBack callout, CFRunLoopObserverContext *context) {
    CHECK_FOR_FORK();
    CFRunLoopObserverRef memory;
    UInt32 size;
    size = sizeof(struct __CFRunLoopObserver) - sizeof(CFRuntimeBase);
    memory = (CFRunLoopObserverRef)_CFRuntimeCreateInstance(allocator, CFRunLoopObserverGetTypeID(), size, NULL);
    if (NULL == memory) {
	return NULL;
    }
    __CFSetValid(memory);
    __CFRunLoopObserverUnsetFiring(memory);
    if (repeats) {
	__CFRunLoopObserverSetRepeats(memory);
    } else {
	__CFRunLoopObserverUnsetRepeats(memory);
    }
    _CFRecursiveMutexCreate(&memory->_lock);
    memory->_runLoop = NULL;
    memory->_rlCount = 0;
    memory->_activities = activities;
    memory->_order = order;
    memory->_callout = callout;
    if (context) {
	if (context->retain) {
	    memory->_context.info = (void *)context->retain(context->info);
	} else {
	    memory->_context.info = context->info;
	}
	memory->_context.retain = context->retain;
	memory->_context.release = context->release;
	memory->_context.copyDescription = context->copyDescription;
    }
    return memory;
}

static void _runLoopObserverWithBlockContext(CFRunLoopObserverRef observer, CFRunLoopActivity activity, void *opaqueBlock) {
    typedef void (^observer_block_t) (CFRunLoopObserverRef observer, CFRunLoopActivity activity);
    observer_block_t block = (observer_block_t)opaqueBlock;
    block(observer, activity);
}

CFRunLoopObserverRef CFRunLoopObserverCreateWithHandler(CFAllocatorRef allocator, CFOptionFlags activities, Boolean repeats, CFIndex order,
                                                      void (^block) (CFRunLoopObserverRef observer, CFRunLoopActivity activity)) {
    CFRunLoopObserverContext blockContext;
    blockContext.version = 0;
    blockContext.info = (void *)block;
    blockContext.retain = (const void *(*)(const void *info))_Block_copy;
    blockContext.release = (void (*)(const void *info))_Block_release;
    blockContext.copyDescription = NULL;
    return CFRunLoopObserverCreate(allocator, activities, repeats, order, _runLoopObserverWithBlockContext, &blockContext);
}

CFOptionFlags CFRunLoopObserverGetActivities(CFRunLoopObserverRef rlo) {
    CF_ASSERT_TYPE(_kCFRuntimeIDCFRunLoopObserver, rlo);
    CHECK_FOR_FORK();
    return rlo->_activities;
}

CFIndex CFRunLoopObserverGetOrder(CFRunLoopObserverRef rlo) {
    CF_ASSERT_TYPE(_kCFRuntimeIDCFRunLoopObserver, rlo);
    CHECK_FOR_FORK();
    return rlo->_order;
}

Boolean CFRunLoopObserverDoesRepeat(CFRunLoopObserverRef rlo) {
    CF_ASSERT_TYPE(_kCFRuntimeIDCFRunLoopObserver, rlo);
    CHECK_FOR_FORK();
    return __CFRunLoopObserverRepeats(rlo);
}

void CFRunLoopObserverInvalidate(CFRunLoopObserverRef rlo) {    /* DOES CALLOUT */
    CF_ASSERT_TYPE(_kCFRuntimeIDCFRunLoopObserver, rlo);
    CHECK_FOR_FORK();
    __CFRunLoopObserverLock(rlo);
    CFRetain(rlo);
    if (__CFIsValid(rlo)) {
        CFRunLoopRef rl = rlo->_runLoop;
        void *info = rlo->_context.info;
        rlo->_context.info = NULL;
        __CFUnsetValid(rlo);
        if (NULL != rl) {
            // To avoid A->B, B->A lock ordering issues when coming up
            // towards the run loop from an observer, it has to be
            // unlocked, which means we have to protect from object
            // invalidation.
            CFRetain(rl);
            __CFRunLoopObserverUnlock(rlo);
            // CFRunLoopRemoveObserver will lock the run loop while it
            // needs that, but we also lock it out here to keep
            // changes from occurring for this whole sequence.
            __CFRunLoopLock(rl);
            CFArrayRef array = CFRunLoopCopyAllModes(rl);
            for (CFIndex idx = CFArrayGetCount(array); idx--;) {
                CFStringRef modeName = (CFStringRef)CFArrayGetValueAtIndex(array, idx);
                CFRunLoopRemoveObserver(rl, rlo, modeName);
            }
            CFRunLoopRemoveObserver(rl, rlo, kCFRunLoopCommonModes);
            __CFRunLoopUnlock(rl);
            CFRelease(array);
            CFRelease(rl);
            __CFRunLoopObserverLock(rlo);
        }
        if (NULL != rlo->_context.release) {
            rlo->_context.release(info);        /* CALLOUT */
        }
    }
    __CFRunLoopObserverUnlock(rlo);
    CFRelease(rlo);
}

Boolean CFRunLoopObserverIsValid(CFRunLoopObserverRef rlo) {
    CF_ASSERT_TYPE(_kCFRuntimeIDCFRunLoopObserver, rlo);
    CHECK_FOR_FORK();
    return __CFIsValid(rlo);
}

void CFRunLoopObserverGetContext(CFRunLoopObserverRef rlo, CFRunLoopObserverContext *context) {
    CF_ASSERT_TYPE(_kCFRuntimeIDCFRunLoopObserver, rlo);
    CHECK_FOR_FORK();
    CFAssert1(0 == context->version, __kCFLogAssertion, "%s(): context version not initialized to 0", __PRETTY_FUNCTION__);
    *context = rlo->_context;
}

#pragma mark -
#pragma mark CFRunLoopTimer

static CFStringRef __CFRunLoopTimerCopyDescription(CFTypeRef cf) {	/* DOES CALLOUT */
    CFRunLoopTimerRef rlt = (CFRunLoopTimerRef)cf;
    CFStringRef contextDesc = NULL;
    if (NULL != rlt->_context.copyDescription) {
	contextDesc = rlt->_context.copyDescription(rlt->_context.info);
    }
    if (NULL == contextDesc) {
	contextDesc = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("<CFRunLoopTimer context %p>"), rlt->_context.info);
    }
    void *addr = (void *)rlt->_callout;
    char libraryName[2048];
    char functionName[2048];
    void *functionPtr = NULL;
    libraryName[0] = '?'; libraryName[1] = '\0';
    functionName[0] = '?'; functionName[1] = '\0';
    CFStringRef result = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL,
        CFSTR("<CFRunLoopTimer %p [%p]>{valid = %s, firing = %s, interval = %0.09g, tolerance = %0.09g, next fire date = %0.09g (%0.09g @ %lld), callout = %s (%p / %p) (%s), context = %@}"),
              cf,
              CFGetAllocator(rlt),
              __CFIsValid(rlt) ? "Yes" : "No",
              __CFRunLoopTimerIsFiring(rlt) ? "Yes" : "No",
              rlt->_interval,
              rlt->_tolerance,
              rlt->_nextFireDate,
              rlt->_nextFireDate - CFAbsoluteTimeGetCurrent(),
              rlt->_fireTSR,
              functionName,
              addr,
              functionPtr,
              libraryName,
              contextDesc);
    CFRelease(contextDesc);
    return result;
}

static void __CFRunLoopTimerDeallocate(CFTypeRef cf) {	/* DOES CALLOUT */
//CFLog(6, CFSTR("__CFRunLoopTimerDeallocate(%p)"), cf);
    CFRunLoopTimerRef rlt = (CFRunLoopTimerRef)cf;
    __CFRunLoopTimerSetDeallocating(rlt);
    CFRunLoopTimerInvalidate(rlt);	/* DOES CALLOUT */
    CFRelease(rlt->_rlModes);
    rlt->_rlModes = NULL;
    _CFRecursiveMutexDestroy(&rlt->_lock);
}

const CFRuntimeClass __CFRunLoopTimerClass = {
    0,
    "CFRunLoopTimer",
    NULL,      // init
    NULL,      // copy
    __CFRunLoopTimerDeallocate,
    NULL,	// equal
    NULL,
    NULL,      // 
    __CFRunLoopTimerCopyDescription
};

CFTypeID CFRunLoopTimerGetTypeID(void) {
    return _kCFRuntimeIDCFRunLoopTimer;
}

CFRunLoopTimerRef CFRunLoopTimerCreate(CFAllocatorRef allocator, CFAbsoluteTime fireDate, CFTimeInterval interval, CFOptionFlags flags, CFIndex order, CFRunLoopTimerCallBack callout, CFRunLoopTimerContext *context) {
    CHECK_FOR_FORK();
    if (isnan(interval)) {
        CRSetCrashLogMessage("NaN was used as an interval for a CFRunLoopTimer");
        HALT;
    }
    CFRunLoopTimerRef memory;
    UInt32 size;
    size = sizeof(struct __CFRunLoopTimer) - sizeof(CFRuntimeBase);
    memory = (CFRunLoopTimerRef)_CFRuntimeCreateInstance(allocator, CFRunLoopTimerGetTypeID(), size, NULL);
    if (NULL == memory) {
	return NULL;
    }
    __CFSetValid(memory);
    __CFRunLoopTimerUnsetFiring(memory);
    _CFRecursiveMutexCreate(&memory->_lock);
    memory->_runLoop = NULL;
    memory->_rlModes = CFSetCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeSetCallBacks);
    memory->_order = order;
    if (interval < 0.0) interval = 0.0;
    memory->_interval = interval;
    memory->_tolerance = 0.0;
    if (TIMER_DATE_LIMIT < fireDate) fireDate = TIMER_DATE_LIMIT;
    memory->_nextFireDate = fireDate;
    uint64_t now2 = __CFNanosecondsToTSR(mach_absolute_time());
    CFAbsoluteTime now1 = CFAbsoluteTimeGetCurrent();
    if (fireDate < now1) {
	memory->_fireTSR = now2;
    } else if (TIMER_INTERVAL_LIMIT < fireDate - now1) {
	memory->_fireTSR = now2 + __CFTimeIntervalToTSR(TIMER_INTERVAL_LIMIT);
    } else {
	memory->_fireTSR = now2 + __CFTimeIntervalToTSR(fireDate - now1);
    }
    memory->_callout = callout;
    if (NULL != context) {
	if (context->retain) {
	    memory->_context.info = (void *)context->retain(context->info);
	} else {
	    memory->_context.info = context->info;
	}
	memory->_context.retain = context->retain;
	memory->_context.release = context->release;
	memory->_context.copyDescription = context->copyDescription;
    }
    return memory;
}

static void _runLoopTimerWithBlockContext(CFRunLoopTimerRef timer, void *opaqueBlock) {
    typedef void (^timer_block_t) (CFRunLoopTimerRef timer);
    timer_block_t block = (timer_block_t)opaqueBlock;
    block(timer);
}

CFRunLoopTimerRef CFRunLoopTimerCreateWithHandler(CFAllocatorRef allocator, CFAbsoluteTime fireDate, CFTimeInterval interval, CFOptionFlags flags, CFIndex order,
						void (^block) (CFRunLoopTimerRef timer)) {
    
    CFRunLoopTimerContext blockContext;
    blockContext.version = 0;
    blockContext.info = (void *)block;
    blockContext.retain = (const void *(*)(const void *info))_Block_copy;
    blockContext.release = (void (*)(const void *info))_Block_release;
    blockContext.copyDescription = NULL;
    return CFRunLoopTimerCreate(allocator, fireDate, interval, flags, order, _runLoopTimerWithBlockContext, &blockContext);
}

CFAbsoluteTime CFRunLoopTimerGetNextFireDate(CFRunLoopTimerRef rlt) {
    CHECK_FOR_FORK();
    CF_OBJC_FUNCDISPATCHV(CFRunLoopTimerGetTypeID(), CFAbsoluteTime, (NSTimer *)rlt, _cffireTime);
    __CFGenericValidateType(rlt, CFRunLoopTimerGetTypeID());
    CFAbsoluteTime at = 0.0;
    __CFRunLoopTimerLock(rlt);
    if (__CFIsValid(rlt)) {
        at = rlt->_nextFireDate;
    }
    __CFRunLoopTimerUnlock(rlt);
    return at;
}

void CFRunLoopTimerSetNextFireDate(CFRunLoopTimerRef rlt, CFAbsoluteTime fireDate) {
    CF_ASSERT_TYPE(_kCFRuntimeIDCFRunLoopTimer, rlt);
    CHECK_FOR_FORK();
    if (!__CFIsValid(rlt)) return;
    if (TIMER_DATE_LIMIT < fireDate) fireDate = TIMER_DATE_LIMIT;
    uint64_t nextFireTSR = 0ULL;
    uint64_t now2 = __CFNanosecondsToTSR(mach_absolute_time());
    CFAbsoluteTime now1 = CFAbsoluteTimeGetCurrent();
    if (fireDate < now1) {
	nextFireTSR = now2;
    } else if (TIMER_INTERVAL_LIMIT < fireDate - now1) {
	nextFireTSR = now2 + __CFTimeIntervalToTSR(TIMER_INTERVAL_LIMIT);
    } else {
	nextFireTSR = now2 + __CFTimeIntervalToTSR(fireDate - now1);
    }
    __CFRunLoopTimerLock(rlt);
    if (NULL != rlt->_runLoop) {
        CFIndex cnt = CFSetGetCount(rlt->_rlModes);
        STACK_BUFFER_DECL(CFTypeRef, modes, cnt);
        CFSetGetValues(rlt->_rlModes, (const void **)modes);
        // To avoid A->B, B->A lock ordering issues when coming up
        // towards the run loop from a source, the timer has to be
        // unlocked, which means we have to protect from object
        // invalidation, although that's somewhat expensive.
        for (CFIndex idx = 0; idx < cnt; idx++) {
            CFRetain(modes[idx]);
        }
        CFRunLoopRef rl = (CFRunLoopRef)CFRetain(rlt->_runLoop);
        __CFRunLoopTimerUnlock(rlt);
        __CFRunLoopLock(rl);
        for (CFIndex idx = 0; idx < cnt; idx++) {
	    CFStringRef name = (CFStringRef)modes[idx];
            modes[idx] = __CFRunLoopCopyMode(rl, name, false);
	    CFRelease(name);
        }
        __CFLock(&rl->_timerTSRLock);
	rlt->_fireTSR = nextFireTSR;
        rlt->_nextFireDate = fireDate;
        for (CFIndex idx = 0; idx < cnt; idx++) {
	    CFRunLoopModeRef rlm = (CFRunLoopModeRef)modes[idx];
            if (rlm) {
                __CFRepositionTimerInMode(rlm, rlt, true);
            }
        }
        __CFUnlock(&rl->_timerTSRLock);
        for (CFIndex idx = 0; idx < cnt; idx++) {
            CFRunLoopModeRef rlm = (CFRunLoopModeRef)modes[idx];
            if (rlm) {
                __CFRunLoopModeUnlock(rlm);
                CFRelease(rlm);
            }
        }
        __CFRunLoopUnlock(rl);
        // This is setting the date of a timer, not a direct
        // interaction with a run loop, so we'll do a wakeup
        // (which may be costly) for the caller, just in case.
        // (And useful for binary compatibility with older
        // code used to the older timer implementation.)
        if (!_CFRunLoopIsCurrent(rl)) { CFRunLoopWakeUp(rl); }
        CFRelease(rl);
     } else {
	rlt->_fireTSR = nextFireTSR;
        rlt->_nextFireDate = fireDate;
         __CFRunLoopTimerUnlock(rlt);
     }
}

CFTimeInterval CFRunLoopTimerGetInterval(CFRunLoopTimerRef rlt) {
    CHECK_FOR_FORK();
    CF_OBJC_FUNCDISPATCHV(CFRunLoopTimerGetTypeID(), CFTimeInterval, (NSTimer *)rlt, timeInterval);
    __CFGenericValidateType(rlt, CFRunLoopTimerGetTypeID());
    return rlt->_interval;
}

Boolean CFRunLoopTimerDoesRepeat(CFRunLoopTimerRef rlt) {
    CF_ASSERT_TYPE(_kCFRuntimeIDCFRunLoopTimer, rlt);
    CHECK_FOR_FORK();
    return (0.0 < rlt->_interval);
}

CFIndex CFRunLoopTimerGetOrder(CFRunLoopTimerRef rlt) {
    CF_ASSERT_TYPE(_kCFRuntimeIDCFRunLoopTimer, rlt);
    CHECK_FOR_FORK();
    return rlt->_order;
}

void CFRunLoopTimerInvalidate(CFRunLoopTimerRef rlt) {	/* DOES CALLOUT */
    CHECK_FOR_FORK();
    CF_OBJC_FUNCDISPATCHV(CFRunLoopTimerGetTypeID(), void, (NSTimer *)rlt, invalidate);
    __CFGenericValidateType(rlt, CFRunLoopTimerGetTypeID());
    __CFRunLoopTimerLock(rlt);
    if (!__CFRunLoopTimerIsDeallocating(rlt)) {
        CFRetain(rlt);
    }
    if (__CFIsValid(rlt)) {
	CFRunLoopRef rl = rlt->_runLoop;
	void *info = rlt->_context.info;
	rlt->_context.info = NULL;
	__CFUnsetValid(rlt);
	if (NULL != rl) {
	    CFIndex cnt = CFSetGetCount(rlt->_rlModes);
	    STACK_BUFFER_DECL(CFStringRef, modes, cnt);
	    CFSetGetValues(rlt->_rlModes, (const void **)modes);
            // To avoid A->B, B->A lock ordering issues when coming up
            // towards the run loop from a source, the timer has to be
            // unlocked, which means we have to protect from object
            // invalidation, although that's somewhat expensive.
            for (CFIndex idx = 0; idx < cnt; idx++) {
                CFRetain(modes[idx]);
            }
            CFRetain(rl);
            __CFRunLoopTimerUnlock(rlt);
            // CFRunLoopRemoveTimer will lock the run loop while it
            // needs that, but we also lock it out here to keep
            // changes from occurring for this whole sequence.
            __CFRunLoopLock(rl);
	    for (CFIndex idx = 0; idx < cnt; idx++) {
		CFRunLoopRemoveTimer(rl, rlt, modes[idx]);
	    }
	    CFRunLoopRemoveTimer(rl, rlt, kCFRunLoopCommonModes);
            __CFRunLoopUnlock(rl);
            for (CFIndex idx = 0; idx < cnt; idx++) {
                CFRelease(modes[idx]);
            }
            CFRelease(rl);
            __CFRunLoopTimerLock(rlt);
	}
	if (NULL != rlt->_context.release) {
	    rlt->_context.release(info);	/* CALLOUT */
	}
    }
    __CFRunLoopTimerUnlock(rlt);
    if (!__CFRunLoopTimerIsDeallocating(rlt)) {
        CFRelease(rlt);
    }
}

Boolean CFRunLoopTimerIsValid(CFRunLoopTimerRef rlt) {
    CHECK_FOR_FORK();
    CF_OBJC_FUNCDISPATCHV(CFRunLoopTimerGetTypeID(), Boolean, (NSTimer *)rlt, isValid);
    __CFGenericValidateType(rlt, CFRunLoopTimerGetTypeID());
    return __CFIsValid(rlt);
}

void CFRunLoopTimerGetContext(CFRunLoopTimerRef rlt, CFRunLoopTimerContext *context) {
    CF_ASSERT_TYPE(_kCFRuntimeIDCFRunLoopTimer, rlt);
    CHECK_FOR_FORK();
    CFAssert1(0 == context->version, __kCFLogAssertion, "%s(): context version not initialized to 0", __PRETTY_FUNCTION__);
    *context = rlt->_context;
}

CFTimeInterval CFRunLoopTimerGetTolerance(CFRunLoopTimerRef rlt) {
#if TARGET_OS_MAC
    CHECK_FOR_FORK();
    CF_OBJC_FUNCDISPATCHV(CFRunLoopTimerGetTypeID(), CFTimeInterval, (NSTimer *)rlt, tolerance);
    __CFGenericValidateType(rlt, CFRunLoopTimerGetTypeID());
    return rlt->_tolerance;
#else
    return 0.0;
#endif
}

void CFRunLoopTimerSetTolerance(CFRunLoopTimerRef rlt, CFTimeInterval tolerance) {
#if TARGET_OS_MAC
    CHECK_FOR_FORK();
    CF_OBJC_FUNCDISPATCHV(CFRunLoopTimerGetTypeID(), void, (NSTimer *)rlt, setTolerance:tolerance);
    __CFGenericValidateType(rlt, CFRunLoopTimerGetTypeID());
    /*
     * dispatch rules:
     *
     * For the initial timer fire at 'start', the upper limit to the allowable
     * delay is set to 'leeway' nanoseconds. For the subsequent timer fires at
     * 'start' + N * 'interval', the upper limit is MIN('leeway','interval'/2).
     */
    if (rlt->_interval > 0) {
        rlt->_tolerance = MIN(tolerance, rlt->_interval / 2);
    } else {
        // Tolerance must be a positive value or zero
        if (tolerance < 0) tolerance = 0.0;
        rlt->_tolerance = tolerance;
    }
#endif
}

