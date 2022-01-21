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

#include <pthread.h>
#if HAVE_PTHREAD_QOS_H && __has_include(<pthread/qos.h>)
#include <pthread/qos.h>
#if __has_include(<pthread/qos_private.h>)
#include <pthread/qos_private.h>
#define _DISPATCH_QOS_CLASS_USER_INTERACTIVE QOS_CLASS_USER_INTERACTIVE
#define _DISPATCH_QOS_CLASS_USER_INITIATED QOS_CLASS_USER_INITIATED
#ifndef QOS_CLASS_LEGACY
#define _DISPATCH_QOS_CLASS_DEFAULT QOS_CLASS_LEGACY
#else
#define _DISPATCH_QOS_CLASS_DEFAULT QOS_CLASS_DEFAULT
#endif
#define _DISPATCH_QOS_CLASS_UTILITY QOS_CLASS_UTILITY
#define _DISPATCH_QOS_CLASS_BACKGROUND QOS_CLASS_BACKGROUND
#define _DISPATCH_QOS_CLASS_UNSPECIFIED QOS_CLASS_UNSPECIFIED
#else // pthread/qos_private.h
typedef unsigned long pthread_priority_t;
#endif // pthread/qos_private.h
#if __has_include(<sys/qos_private.h>)
#include <sys/qos_private.h>
#define _DISPATCH_QOS_CLASS_MAINTENANCE QOS_CLASS_MAINTENANCE
#else // sys/qos_private.h
#define _DISPATCH_QOS_CLASS_MAINTENANCE	0x05
#endif // sys/qos_private.h
#ifndef _PTHREAD_PRIORITY_ROOTQUEUE_FLAG
#define _PTHREAD_PRIORITY_ROOTQUEUE_FLAG 0x20000000
#endif
#ifndef _PTHREAD_PRIORITY_ENFORCE_FLAG
#define _PTHREAD_PRIORITY_ENFORCE_FLAG 0x10000000
#endif
#ifndef _PTHREAD_PRIORITY_OVERRIDE_FLAG
#define _PTHREAD_PRIORITY_OVERRIDE_FLAG 0x08000000
#endif
#ifndef _PTHREAD_PRIORITY_DEFAULTQUEUE_FLAG
#define _PTHREAD_PRIORITY_DEFAULTQUEUE_FLAG 0x04000000
#endif
#else // HAVE_PTHREAD_QOS_H
typedef unsigned int qos_class_t;
typedef unsigned long pthread_priority_t;
#define QOS_MIN_RELATIVE_PRIORITY (-15)
#define _PTHREAD_PRIORITY_QOS_CLASS_MASK 0x00ffff00
#define _PTHREAD_PRIORITY_ROOTQUEUE_FLAG 0x20000000
#define _PTHREAD_PRIORITY_ENFORCE_FLAG 0x10000000
#define _PTHREAD_PRIORITY_OVERRIDE_FLAG 0x08000000
#define _PTHREAD_PRIORITY_DEFAULTQUEUE_FLAG 0x04000000
#endif // HAVE_PTHREAD_QOS_H
#ifndef _DISPATCH_QOS_CLASS_USER_INTERACTIVE
enum {
	_DISPATCH_QOS_CLASS_USER_INTERACTIVE = 0x21,
	_DISPATCH_QOS_CLASS_USER_INITIATED = 0x19,
	_DISPATCH_QOS_CLASS_DEFAULT = 0x15,
	_DISPATCH_QOS_CLASS_UTILITY = 0x11,
	_DISPATCH_QOS_CLASS_BACKGROUND = 0x09,
	_DISPATCH_QOS_CLASS_MAINTENANCE = 0x05,
	_DISPATCH_QOS_CLASS_UNSPECIFIED = 0x00,
};
#endif // _DISPATCH_QOS_CLASS_USER_INTERACTIVE
#if HAVE_PTHREAD_WORKQUEUES
#if __has_include(<pthread/workqueue_private.h>)
#include <pthread/workqueue_private.h>
#else
#include <pthread_workqueue.h>
#endif
#ifndef WORKQ_FEATURE_MAINTENANCE
#define WORKQ_FEATURE_MAINTENANCE 0x10
#endif
#endif // HAVE_PTHREAD_WORKQUEUES

#if HAVE_PTHREAD_NP_H
#include <pthread_np.h>
#endif

#if !HAVE_DECL_FD_COPY
#define FD_COPY(f, t) (void)(*(t) = *(f))
#endif

#if TARGET_OS_WIN32
#define bzero(ptr,len) memset((ptr), 0, (len))
#define snprintf _snprintf

inline size_t strlcpy(char *dst, const char *src, size_t size) {
	int res = strlen(dst) + strlen(src) + 1;
	if (size > 0) {
		size_t n = size - 1;
		strncpy(dst, src, n);
		dst[n] = 0;
	}
	return res;
}
#endif // TARGET_OS_WIN32

#if PTHREAD_WORKQUEUE_SPI_VERSION < 20140716
static inline int
_pthread_workqueue_override_start_direct(mach_port_t thread,
		pthread_priority_t priority)
{
	(void)thread; (void)priority;
	return 0;
}
#endif // PTHREAD_WORKQUEUE_SPI_VERSION < 20140716

#if PTHREAD_WORKQUEUE_SPI_VERSION < 20140707
static inline int
_pthread_override_qos_class_start_direct(pthread_t thread,
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
#define DISPATCH_ATOMIC_UP 1
#endif

#include "shims/atomic.h"
#include "shims/atomic_sfb.h"
#include "shims/tsd.h"
#include "shims/yield.h"

#include "shims/hw_config.h"
#include "shims/perfmon.h"

#include "shims/getprogname.h"
#include "shims/time.h"

#if __FreeBSD__
#define _dispatch_clear_stack(s)
#elif __APPLE__
// Clear the stack before calling long-running thread-handler functions that
// never return (and don't take arguments), to facilitate leak detection and
// provide cleaner backtraces. <rdar://problem/9050566>
#define _dispatch_clear_stack(s) do { \
		void *a[(s)/sizeof(void*) ? (s)/sizeof(void*) : 1]; \
		a[0] = pthread_get_stackaddr_np(pthread_self()); \
		bzero((void*)&a[1], (size_t)(a[0] - (void*)&a[1])); \
	} while (0)
#else
#define _dispatch_clear_stack(s)
#endif

#endif
