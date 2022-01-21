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

#ifndef __DISPATCH_PRIVATE__
#define __DISPATCH_PRIVATE__

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

#if TARGET_OS_MAC
#include <mach/boolean.h>
#include <mach/mach.h>
#include <mach/message.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_SYS_CDEFS_H
#include <sys/cdefs.h>
#endif
#include <pthread.h>

#ifndef __DISPATCH_BUILDING_DISPATCH__
#include <dispatch/dispatch.h>

#ifndef __DISPATCH_INDIRECT__
#define __DISPATCH_INDIRECT__
#endif

#include <dispatch/benchmark.h>
#include <dispatch/queue_private.h>
#include <dispatch/source_private.h>
#include <dispatch/mach_private.h>
#include <dispatch/data_private.h>
#include <dispatch/io_private.h>
#include <dispatch/layout_private.h>

#undef __DISPATCH_INDIRECT__

#endif /* !__DISPATCH_BUILDING_DISPATCH__ */

// <rdar://problem/9627726> Check that public and private dispatch headers match
#if DISPATCH_API_VERSION != 20140804 // Keep in sync with <dispatch/dispatch.h>
#error "Dispatch header mismatch between /usr/include and /usr/local/include"
#endif

__BEGIN_DECLS

/*!
 * @function _dispatch_is_multithreaded
 *
 * @abstract
 * Returns true if the current process has become multithreaded by the use
 * of libdispatch functionality.
 *
 * @discussion
 * This SPI is intended for use by low-level system components that need to
 * ensure that they do not make a single-threaded process multithreaded, to
 * avoid negatively affecting child processes of a fork (without exec).
 *
 * Such components must not use any libdispatch functionality if this function
 * returns false.
 *
 * @result
 * Boolean indicating whether the process has used libdispatch and become
 * multithreaded.
 */
__OSX_AVAILABLE_STARTING(__MAC_10_8,__IPHONE_6_0)
DISPATCH_EXPORT DISPATCH_NOTHROW
bool _dispatch_is_multithreaded(void);

/*!
 * @function _dispatch_is_fork_of_multithreaded_parent
 *
 * @abstract
 * Returns true if the current process is a child of a parent process that had
 * become multithreaded by the use of libdispatch functionality at the time of
 * fork (without exec).
 *
 * @discussion
 * This SPI is intended for use by (rare) low-level system components that need
 * to continue working on the child side of a fork (without exec) of a
 * multithreaded process.
 *
 * Such components must not use any libdispatch functionality if this function
 * returns true.
 *
 * @result
 * Boolean indicating whether the parent process had used libdispatch and
 * become multithreaded at the time of fork.
 */
__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0)
DISPATCH_EXPORT DISPATCH_NOTHROW
bool _dispatch_is_fork_of_multithreaded_parent(void);

/*
 * dispatch_time convenience macros
 */

#define _dispatch_time_after_nsec(t) \
		dispatch_time(DISPATCH_TIME_NOW, (t))
#define _dispatch_time_after_usec(t) \
		dispatch_time(DISPATCH_TIME_NOW, (t) * NSEC_PER_USEC)
#define _dispatch_time_after_msec(t) \
		dispatch_time(DISPATCH_TIME_NOW, (t) * NSEC_PER_MSEC)
#define _dispatch_time_after_sec(t) \
		dispatch_time(DISPATCH_TIME_NOW, (t) * NSEC_PER_SEC)

/*
 * SPI for CoreFoundation/Foundation/libauto ONLY
 */

#define DISPATCH_COCOA_COMPAT (TARGET_OS_MAC || TARGET_OS_WIN32)

#if DISPATCH_COCOA_COMPAT

#if TARGET_OS_MAC
__OSX_AVAILABLE_STARTING(__MAC_10_6,__IPHONE_4_0)
DISPATCH_EXPORT DISPATCH_CONST DISPATCH_WARN_RESULT DISPATCH_NOTHROW
mach_port_t
_dispatch_get_main_queue_port_4CF(void);

__OSX_AVAILABLE_STARTING(__MAC_10_6,__IPHONE_4_0)
DISPATCH_EXPORT DISPATCH_NOTHROW
void
_dispatch_main_queue_callback_4CF(mach_msg_header_t *msg);
#elif TARGET_OS_WIN32
__OSX_AVAILABLE_STARTING(__MAC_10_6,__IPHONE_4_0)
DISPATCH_EXPORT DISPATCH_NOTHROW
HANDLE
_dispatch_get_main_queue_handle_4CF(void);

__OSX_AVAILABLE_STARTING(__MAC_10_6,__IPHONE_4_0)
DISPATCH_EXPORT DISPATCH_NOTHROW
void
_dispatch_main_queue_callback_4CF(void);
#endif // TARGET_OS_WIN32

__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0)
DISPATCH_EXPORT DISPATCH_MALLOC DISPATCH_RETURNS_RETAINED DISPATCH_WARN_RESULT
DISPATCH_NOTHROW
dispatch_queue_t
_dispatch_runloop_root_queue_create_4CF(const char *label, unsigned long flags);

#if TARGET_OS_MAC
__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0)
DISPATCH_EXPORT DISPATCH_WARN_RESULT DISPATCH_NOTHROW
mach_port_t
_dispatch_runloop_root_queue_get_port_4CF(dispatch_queue_t queue);
#endif

__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0)
DISPATCH_EXPORT DISPATCH_NOTHROW
void
_dispatch_runloop_root_queue_wakeup_4CF(dispatch_queue_t queue);

__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0)
DISPATCH_EXPORT DISPATCH_WARN_RESULT DISPATCH_NOTHROW
bool
_dispatch_runloop_root_queue_perform_4CF(dispatch_queue_t queue);

__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0)
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_NOTHROW
void
_dispatch_source_set_runloop_timer_4CF(dispatch_source_t source,
		dispatch_time_t start, uint64_t interval, uint64_t leeway);

__OSX_AVAILABLE_STARTING(__MAC_10_6,__IPHONE_4_0)
DISPATCH_EXPORT
void (*dispatch_begin_thread_4GC)(void);

__OSX_AVAILABLE_STARTING(__MAC_10_6,__IPHONE_4_0)
DISPATCH_EXPORT
void (*dispatch_end_thread_4GC)(void);
__OSX_AVAILABLE_STARTING(__MAC_10_6,__IPHONE_4_0)
DISPATCH_EXPORT
void *(*_dispatch_begin_NSAutoReleasePool)(void);

__OSX_AVAILABLE_STARTING(__MAC_10_6,__IPHONE_4_0)
DISPATCH_EXPORT
void (*_dispatch_end_NSAutoReleasePool)(void *);

#endif /* DISPATCH_COCOA_COMPAT */

__END_DECLS

#endif // __DISPATCH_PRIVATE__
