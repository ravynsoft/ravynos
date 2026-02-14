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
#include <Availability.h>
#include <os/availability.h>
#include <TargetConditionals.h>
#include <os/base.h>
#elif defined(_WIN32)
#include <os/generic_win_base.h>
#elif defined(__unix__)
#include <os/generic_unix_base.h>
#endif

#if TARGET_OS_MAC
#include <mach/boolean.h>
#include <mach/mach.h>
#include <mach/message.h>
#endif
#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
#include <unistd.h>
#endif
#if !defined(_WIN32)
#include <pthread.h>
#endif
#if TARGET_OS_MAC
#include <pthread/qos.h>
#endif

#ifndef __DISPATCH_BUILDING_DISPATCH__
#include <dispatch/dispatch.h>

#ifndef __DISPATCH_INDIRECT__
#define __DISPATCH_INDIRECT__
#endif

#include <dispatch/benchmark.h>
#include <dispatch/queue_private.h>
#if DISPATCH_CHANNEL_SPI
#include <dispatch/channel_private.h>
#endif
#include <dispatch/workloop_private.h>
#include <dispatch/source_private.h>
#include <dispatch/mach_private.h>
#include <dispatch/data_private.h>
#include <dispatch/io_private.h>
#include <dispatch/layout_private.h>
#include <dispatch/time_private.h>

#undef __DISPATCH_INDIRECT__
#endif /* !__DISPATCH_BUILDING_DISPATCH__ */

// <rdar://problem/9627726> Check that public and private dispatch headers match
#if DISPATCH_API_VERSION != 20181008 // Keep in sync with <dispatch/dispatch.h>
#error "Dispatch header mismatch between /usr/include and /usr/local/include"
#endif

DISPATCH_ASSUME_NONNULL_BEGIN

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
API_AVAILABLE(macos(10.8), ios(6.0))
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
API_AVAILABLE(macos(10.9), ios(7.0))
DISPATCH_EXPORT DISPATCH_NOTHROW
bool _dispatch_is_fork_of_multithreaded_parent(void);

/*!
 * @function _dispatch_prohibit_transition_to_multithreaded
 *
 * @abstract
 * Sets a mode that aborts if a program tries to use dispatch.
 *
 * @discussion
 * This SPI is intended for use by programs that know they will use fork() and
 * want their children to be able to use dispatch before exec(). Such programs
 * should call _dispatch_prohibit_transition_to_multithreaded(true) as early as
 * possible, which will cause any use of dispatch API that would make the
 * process multithreaded to abort immediately.
 *
 * Once the program no longer intends to call fork() it can call
 * _dispatch_prohibit_transition_to_multithreaded(false).
 *
 * This status is not inherited by the child process, so if the behavior
 * is required after fork, _dispatch_prohibit_transition_to_multithreaded(true)
 * should be called manually in the child after fork.
 *
 * If the program already used dispatch before the guard is enabled, then
 * this function will abort immediately.
 */
API_AVAILABLE(macos(10.12), ios(10.0), tvos(10.0), watchos(3.0))
DISPATCH_EXPORT DISPATCH_NOTHROW
void _dispatch_prohibit_transition_to_multithreaded(bool prohibit);

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
 * SPI for CoreFoundation/Foundation ONLY
 */

#if TARGET_OS_MAC
#define DISPATCH_COCOA_COMPAT 1
#elif defined(__linux__) || defined(__FreeBSD__)
#define DISPATCH_COCOA_COMPAT 1
#else
#define DISPATCH_COCOA_COMPAT 0
#endif

#if DISPATCH_COCOA_COMPAT

#define DISPATCH_CF_SPI_VERSION 20160712

#if TARGET_OS_MAC
typedef mach_port_t dispatch_runloop_handle_t;
#elif defined(__linux__) || defined(__FreeBSD__)
typedef int dispatch_runloop_handle_t;
#else
#error "runloop support not implemented on this platform"
#endif

#if TARGET_OS_MAC
API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_EXPORT DISPATCH_CONST DISPATCH_WARN_RESULT DISPATCH_NOTHROW
dispatch_runloop_handle_t
_dispatch_get_main_queue_port_4CF(void);
#endif

API_AVAILABLE(macos(10.12), ios(10.0), tvos(10.0), watchos(3.0))
DISPATCH_EXPORT DISPATCH_NOTHROW
dispatch_runloop_handle_t
_dispatch_get_main_queue_handle_4CF(void);

API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_EXPORT DISPATCH_NOTHROW
void
_dispatch_main_queue_callback_4CF(void *_Null_unspecified msg);

API_AVAILABLE(macos(10.9), ios(7.0))
DISPATCH_EXPORT DISPATCH_MALLOC DISPATCH_RETURNS_RETAINED DISPATCH_WARN_RESULT
DISPATCH_NOTHROW
dispatch_queue_serial_t
_dispatch_runloop_root_queue_create_4CF(const char *_Nullable label,
		unsigned long flags);

#if TARGET_OS_MAC
API_AVAILABLE(macos(10.9), ios(7.0))
DISPATCH_EXPORT DISPATCH_WARN_RESULT DISPATCH_NOTHROW
mach_port_t
_dispatch_runloop_root_queue_get_port_4CF(dispatch_queue_t queue);

API_AVAILABLE(macos(10.13.2), ios(11.2), tvos(11.2), watchos(4.2))
DISPATCH_EXPORT DISPATCH_WARN_RESULT DISPATCH_NOTHROW
bool
_dispatch_source_will_reenable_kevent_4NW(dispatch_source_t source);
#endif

API_AVAILABLE(macos(10.9), ios(7.0))
DISPATCH_EXPORT DISPATCH_NOTHROW
void
_dispatch_runloop_root_queue_wakeup_4CF(dispatch_queue_t queue);

API_AVAILABLE(macos(10.9), ios(7.0))
DISPATCH_EXPORT DISPATCH_WARN_RESULT DISPATCH_NOTHROW
bool
_dispatch_runloop_root_queue_perform_4CF(dispatch_queue_t queue);

API_AVAILABLE(macos(10.9), ios(7.0))
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_NOTHROW
void
_dispatch_source_set_runloop_timer_4CF(dispatch_source_t source,
		dispatch_time_t start, uint64_t interval, uint64_t leeway);

API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_EXPORT
void *_Nonnull (*_Nullable _dispatch_begin_NSAutoReleasePool)(void);

API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_EXPORT
void (*_Nullable _dispatch_end_NSAutoReleasePool)(void *);

#endif /* DISPATCH_COCOA_COMPAT */

API_AVAILABLE(macos(10.13), ios(11.0), tvos(11.0), watchos(4.0))
DISPATCH_EXPORT DISPATCH_NOTHROW
void
_dispatch_poll_for_events_4launchd(void);

__END_DECLS

DISPATCH_ASSUME_NONNULL_END

#endif // __DISPATCH_PRIVATE__
