/*
 * Copyright (c) 2008-2013 Apple Inc. All rights reserved.
 *
 * @APPLE_APACHE_LICENSE_HEADER_START@
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @APPLE_APACHE_LICENSE_HEADER_END@
 */

/*
 * IMPORTANT: This header file describes INTERNAL interfaces to libdispatch
 * which are subject to change in future releases of Mac OS X. Any applications
 * relying on these interfaces WILL break.
 */

#ifndef __DISPATCH_OS_SHIMS__
#define __DISPATCH_OS_SHIMS__

#if !defined(_WIN32)
#include <pthread.h>
#else // defined(_WIN32)
#include "shims/generic_win_stubs.h"
#include "shims/generic_sys_queue.h"
#endif // defined(_WIN32)

#ifdef __ANDROID__
#include "shims/android_stubs.h"
#endif // __ANDROID__

#if !HAVE_MACH
#include "shims/mach.h"
#endif
#include "shims/target.h"

#if DISPATCH_USE_INTERNAL_WORKQUEUE
#include "event/workqueue_internal.h"
#elif HAVE_PTHREAD_WORKQUEUES
#include <pthread/workqueue_private.h>
#else
#error Unsupported configuration
#endif

#ifndef DISPATCH_WORKQ_MAX_PTHREAD_COUNT
#define DISPATCH_WORKQ_MAX_PTHREAD_COUNT 255
#endif

#include "shims/hw_config.h"
#include "shims/priority.h"

#if HAVE_PTHREAD_NP_H
#include <pthread_np.h>
#endif

#if __has_include(<pthread/private.h>)
#include <pthread/private.h>
#endif

#if !HAVE_DECL_FD_COPY
#define FD_COPY(f, t) (void)(*(t) = *(f))
#endif

#if HAVE_STRLCPY
#include <string.h>
#else // that is, if !HAVE_STRLCPY

size_t strlcpy(char *dst, const char *src, size_t size);

#endif // HAVE_STRLCPY

#ifndef TAILQ_FOREACH_SAFE
#define TAILQ_FOREACH_SAFE(var, head, field, temp)                         \
	for ((var) = TAILQ_FIRST((head));                                      \
		(var) && ((temp) = TAILQ_NEXT((var), field), 1); (var) = (temp))
#endif

#if PTHREAD_WORKQUEUE_SPI_VERSION < 20140716
static inline int
_pthread_workqueue_override_start_direct(mach_port_t thread,
		pthread_priority_t priority)
{
	(void)thread; (void)priority;
	return 0;
}
#endif // PTHREAD_WORKQUEUE_SPI_VERSION < 20140716

#if PTHREAD_WORKQUEUE_SPI_VERSION < 20150319
static inline int
_pthread_workqueue_override_start_direct_check_owner(mach_port_t thread,
		pthread_priority_t priority, mach_port_t *ulock_addr)
{
	(void)ulock_addr;
	return _pthread_workqueue_override_start_direct(thread, priority);
}
#endif // PTHREAD_WORKQUEUE_SPI_VERSION < 20150319

#if PTHREAD_WORKQUEUE_SPI_VERSION < 20140707
static inline int
_pthread_override_qos_class_start_direct(mach_port_t thread,
		pthread_priority_t priority)
{
	(void)thread; (void)priority;
	return 0;
}

static inline int
_pthread_override_qos_class_end_direct(mach_port_t thread)
{
	(void)thread;
	return 0;
}
#endif // PTHREAD_WORKQUEUE_SPI_VERSION < 20140707

#if PTHREAD_WORKQUEUE_SPI_VERSION < 20150325
static inline int
_pthread_qos_override_start_direct(mach_port_t thread,
		pthread_priority_t priority, void *resource)
{
	(void)resource;
	return _pthread_override_qos_class_start_direct(thread, priority);
}

static inline int
_pthread_qos_override_end_direct(mach_port_t thread, void *resource)
{
	(void)resource;
	return _pthread_override_qos_class_end_direct(thread);
}
#endif // PTHREAD_WORKQUEUE_SPI_VERSION < 20150325

#if PTHREAD_WORKQUEUE_SPI_VERSION < 20160427
#define _PTHREAD_SET_SELF_WQ_KEVENT_UNBIND 0
#endif

#if PTHREAD_WORKQUEUE_SPI_VERSION < 20160427
static inline bool
_pthread_workqueue_should_narrow(pthread_priority_t priority)
{
	(void)priority;
	return false;
}
#endif

#if HAVE_PTHREAD_QOS_H && __has_include(<pthread/qos_private.h>) && \
		defined(PTHREAD_MAX_PARALLELISM_PHYSICAL) && \
		DISPATCH_HAVE_HW_CONFIG_COMMPAGE && \
		DISPATCH_MIN_REQUIRED_OSX_AT_LEAST(101300)
#define DISPATCH_USE_PTHREAD_QOS_MAX_PARALLELISM 1
#define DISPATCH_MAX_PARALLELISM_PHYSICAL PTHREAD_MAX_PARALLELISM_PHYSICAL
#else
#define DISPATCH_MAX_PARALLELISM_PHYSICAL 0x1
#endif
#define DISPATCH_MAX_PARALLELISM_ACTIVE 0x2
_Static_assert(!(DISPATCH_MAX_PARALLELISM_PHYSICAL &
		DISPATCH_MAX_PARALLELISM_ACTIVE), "Overlapping parallelism flags");

DISPATCH_ALWAYS_INLINE
static inline uint32_t
_dispatch_qos_max_parallelism(dispatch_qos_t qos, unsigned long flags)
{
	uint32_t p;
	int r = 0;

	if (qos) {
#if DISPATCH_USE_PTHREAD_QOS_MAX_PARALLELISM
		r = pthread_qos_max_parallelism(_dispatch_qos_to_qos_class(qos),
				flags & PTHREAD_MAX_PARALLELISM_PHYSICAL);
#endif
	}
	if (likely(r > 0)) {
		p = (uint32_t)r;
	} else {
		p = (flags & DISPATCH_MAX_PARALLELISM_PHYSICAL) ?
				dispatch_hw_config(physical_cpus) :
				dispatch_hw_config(logical_cpus);
	}
	if (flags & DISPATCH_MAX_PARALLELISM_ACTIVE) {
		uint32_t active_cpus = dispatch_hw_config(active_cpus);
		if ((flags & DISPATCH_MAX_PARALLELISM_PHYSICAL) &&
				active_cpus < dispatch_hw_config(logical_cpus)) {
			active_cpus /= dispatch_hw_config(logical_cpus) /
					dispatch_hw_config(physical_cpus);
		}
		if (active_cpus < p) p = active_cpus;
	}
	return p;
}

#if !HAVE_NORETURN_BUILTIN_TRAP
/*
 * XXXRW: Work-around for possible clang bug in which __builtin_trap() is not
 * marked noreturn, leading to a build error as dispatch_main() *is* marked
 * noreturn. Mask by marking __builtin_trap() as noreturn locally.
 */
DISPATCH_NORETURN
void __builtin_trap(void);
#endif

#if DISPATCH_HW_CONFIG_UP
#define OS_ATOMIC_UP 1
#else
#define OS_ATOMIC_UP 0
#endif


#ifndef __OS_INTERNAL_ATOMIC__
#include "shims/atomic.h"
#endif
#define DISPATCH_ATOMIC64_ALIGN  __attribute__((aligned(8)))

#include "shims/atomic_sfb.h"
#include "shims/tsd.h"
#include "shims/yield.h"
#include "shims/lock.h"

#include "shims/perfmon.h"

#include "shims/getprogname.h"
#include "shims/time.h"

#if __has_include(<os/overflow.h>)
#include <os/overflow.h>
#elif __has_builtin(__builtin_add_overflow)
#define os_add_overflow(a, b, c) __builtin_add_overflow(a, b, c)
#define os_sub_overflow(a, b, c) __builtin_sub_overflow(a, b, c)
#define os_mul_overflow(a, b, c) __builtin_mul_overflow(a, b, c)
#else
#error unsupported compiler
#endif

#ifndef os_mul_and_add_overflow
#define os_mul_and_add_overflow(a, x, b, res) __extension__({ \
	__typeof(*(res)) _tmp; \
	bool _s, _t; \
	_s = os_mul_overflow((a), (x), &_tmp); \
	_t = os_add_overflow((b), _tmp, (res)); \
	_s | _t; \
})
#endif


#if __has_feature(c_static_assert)
#define __dispatch_is_array(x) \
	_Static_assert(!__builtin_types_compatible_p(__typeof__((x)[0]) *, __typeof__(x)), \
				#x " isn't an array")
#define countof(x) \
	({ __dispatch_is_array(x); sizeof(x) / sizeof((x)[0]); })
#else
#define countof(x) (sizeof(x) / sizeof(x[0]))
#endif

DISPATCH_ALWAYS_INLINE
static inline void *
_dispatch_mempcpy(void *ptr, const void *data, size_t len)
{
	memcpy(ptr, data, len);
	return (char *)ptr + len;
}
#define _dispatch_memappend(ptr, e) \
	_dispatch_mempcpy(ptr, e, sizeof(*(e)))

#ifdef __APPLE__
// Clear the stack before calling long-running thread-handler functions that
// never return (and don't take arguments), to facilitate leak detection and
// provide cleaner backtraces. <rdar://problem/9050566>
#define _dispatch_clear_stack(s) do { \
		void *a[(s)/sizeof(void*) ? (s)/sizeof(void*) : 1]; \
		a[0] = pthread_get_stackaddr_np(pthread_self()); \
		void* volatile const p = (void*)&a[1]; /* <rdar://32604885> */ \
		bzero((void*)p, (size_t)(a[0] - (void*)&a[1])); \
	} while (0)
#else
#define _dispatch_clear_stack(s)
#endif

#endif
