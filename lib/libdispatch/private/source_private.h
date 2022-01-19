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

#ifndef __DISPATCH_SOURCE_PRIVATE__
#define __DISPATCH_SOURCE_PRIVATE__

#ifndef __DISPATCH_INDIRECT__
#error "Please #include <dispatch/private.h> instead of this file directly."
#include <dispatch/base.h> // for HeaderDoc
#endif

/*!
 * @const DISPATCH_SOURCE_TYPE_TIMER_WITH_AGGREGATE
 * @discussion A dispatch timer source that is part of a timer aggregate.
 * The handle is the dispatch timer aggregate object.
 * The mask specifies which flags from dispatch_source_timer_flags_t to apply.
 */
#define DISPATCH_SOURCE_TYPE_TIMER_WITH_AGGREGATE \
		(&_dispatch_source_type_timer_with_aggregate)
__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0)
DISPATCH_SOURCE_TYPE_DECL(timer_with_aggregate);

/*!
 * @const DISPATCH_SOURCE_TYPE_INTERVAL
 * @discussion A dispatch source that submits the event handler block at a
 * specified time interval, phase-aligned with all other interval sources on
 * the system that have the same interval value.
 *
 * The initial submission of the event handler will occur at some point during
 * the first time interval after the source is created (assuming the source is
 * resumed at that time).
 *
 * By default, the unit for the interval value is milliseconds and the leeway
 * (maximum amount of time any individual handler submission may be deferred to
 * align with other system activity) for the source is fixed at interval/2.
 *
 * If the DISPATCH_INTERVAL_UI_ANIMATION flag is specified, the unit for the
 * interval value is animation frames (1/60th of a second) and the leeway is
 * fixed at one frame.
 *
 * The handle is the interval value in milliseconds or frames.
 * The mask specifies which flags from dispatch_source_timer_flags_t to apply.
 */
#define DISPATCH_SOURCE_TYPE_INTERVAL (&_dispatch_source_type_interval)
__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0)
DISPATCH_SOURCE_TYPE_DECL(interval);

/*!
 * @const DISPATCH_SOURCE_TYPE_VFS
 * @discussion Apple-internal dispatch source that monitors for vfs events
 * defined by dispatch_vfs_flags_t.
 * The handle is a process identifier (pid_t).
 */
#define DISPATCH_SOURCE_TYPE_VFS (&_dispatch_source_type_vfs)
__OSX_AVAILABLE_STARTING(__MAC_10_6,__IPHONE_4_0)
DISPATCH_EXPORT const struct dispatch_source_type_s _dispatch_source_type_vfs;

/*!
 * @const DISPATCH_SOURCE_TYPE_VM
 * @discussion A dispatch source that monitors virtual memory
 * The mask is a mask of desired events from dispatch_source_vm_flags_t.
 * This type is deprecated, use DISPATCH_SOURCE_TYPE_MEMORYSTATUS instead.
 */
#define DISPATCH_SOURCE_TYPE_VM (&_dispatch_source_type_vm)
__OSX_AVAILABLE_BUT_DEPRECATED_MSG(__MAC_10_7, __MAC_10_10, __IPHONE_4_3,
		__IPHONE_8_0, "Use DISPATCH_SOURCE_TYPE_MEMORYSTATUS instead")
DISPATCH_EXPORT const struct dispatch_source_type_s _dispatch_source_type_vm;

/*!
 * @const DISPATCH_SOURCE_TYPE_MEMORYSTATUS
 * @discussion A dispatch source that monitors memory status
 * The mask is a mask of desired events from
 * dispatch_source_memorystatus_flags_t.
 */
#define DISPATCH_SOURCE_TYPE_MEMORYSTATUS (&_dispatch_source_type_memorystatus)
__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_6_0)
DISPATCH_EXPORT const struct dispatch_source_type_s
		_dispatch_source_type_memorystatus;

/*!
 * @const DISPATCH_SOURCE_TYPE_SOCK
 * @discussion A dispatch source that monitors events on socket state changes.
 */
#define DISPATCH_SOURCE_TYPE_SOCK (&_dispatch_source_type_sock)
__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0)
DISPATCH_EXPORT const struct dispatch_source_type_s _dispatch_source_type_sock;

/*!
 * @enum dispatch_source_sock_flags_t
 *
 * @constant DISPATCH_SOCK_CONNRESET
 * Received RST
 *
 * @constant DISPATCH_SOCK_READCLOSED
 * Read side is shutdown
 *
 * @constant DISPATCH_SOCK_WRITECLOSED
 * Write side is shutdown
 *
 * @constant DISPATCH_SOCK_TIMEOUT
 * Timeout: rexmt, keep-alive or persist
 *
 * @constant DISPATCH_SOCK_NOSRCADDR
 * Source address not available
 *
 * @constant DISPATCH_SOCK_IFDENIED
 * Interface denied connection
 *
 * @constant DISPATCH_SOCK_SUSPEND
 * Output queue suspended
 *
 * @constant DISPATCH_SOCK_RESUME
 * Output queue resumed
 *
 * @constant DISPATCH_SOCK_KEEPALIVE
 * TCP Keepalive received
 *
 * @constant DISPATCH_SOCK_CONNECTED
 * Socket is connected
 *
 * @constant DISPATCH_SOCK_DISCONNECTED
 * Socket is disconnected
 *
 * @constant DISPATCH_SOCK_CONNINFO_UPDATED
 * Connection info was updated
 */
enum {
	DISPATCH_SOCK_CONNRESET = 0x00000001,
	DISPATCH_SOCK_READCLOSED = 0x00000002,
	DISPATCH_SOCK_WRITECLOSED = 0x00000004,
	DISPATCH_SOCK_TIMEOUT = 0x00000008,
	DISPATCH_SOCK_NOSRCADDR = 0x00000010,
	DISPATCH_SOCK_IFDENIED = 0x00000020,
	DISPATCH_SOCK_SUSPEND = 0x00000040,
	DISPATCH_SOCK_RESUME = 0x00000080,
	DISPATCH_SOCK_KEEPALIVE = 0x00000100,
	DISPATCH_SOCK_ADAPTIVE_WTIMO = 0x00000200,
	DISPATCH_SOCK_ADAPTIVE_RTIMO = 0x00000400,
	DISPATCH_SOCK_CONNECTED = 0x00000800,
	DISPATCH_SOCK_DISCONNECTED = 0x00001000,
	DISPATCH_SOCK_CONNINFO_UPDATED = 0x00002000,
};

/*!
 * @enum dispatch_source_vfs_flags_t
 *
 * @constant DISPATCH_VFS_NOTRESP
 * Server down.
 *
 * @constant DISPATCH_VFS_NEEDAUTH
 * Server bad auth.
 *
 * @constant DISPATCH_VFS_LOWDISK
 * We're low on space.
 *
 * @constant DISPATCH_VFS_MOUNT
 * New filesystem arrived.
 *
 * @constant DISPATCH_VFS_UNMOUNT
 * Filesystem has left.
 *
 * @constant DISPATCH_VFS_DEAD
 * Filesystem is dead, needs force unmount.
 *
 * @constant DISPATCH_VFS_ASSIST
 * Filesystem needs assistance from external program.
 *
 * @constant DISPATCH_VFS_NOTRESPLOCK
 * Server lockd down.
 *
 * @constant DISPATCH_VFS_UPDATE
 * Filesystem information has changed.
 *
 * @constant DISPATCH_VFS_VERYLOWDISK
 * File system has *very* little disk space left.
 */
enum {
	DISPATCH_VFS_NOTRESP = 0x0001,
	DISPATCH_VFS_NEEDAUTH = 0x0002,
	DISPATCH_VFS_LOWDISK = 0x0004,
	DISPATCH_VFS_MOUNT = 0x0008,
	DISPATCH_VFS_UNMOUNT = 0x0010,
	DISPATCH_VFS_DEAD = 0x0020,
	DISPATCH_VFS_ASSIST = 0x0040,
	DISPATCH_VFS_NOTRESPLOCK = 0x0080,
	DISPATCH_VFS_UPDATE = 0x0100,
	DISPATCH_VFS_VERYLOWDISK = 0x0200,
};

/*!
 * @enum dispatch_source_timer_flags_t
 *
 * @constant DISPATCH_TIMER_BACKGROUND
 * Specifies that the timer is used to trigger low priority maintenance-level
 * activity and that the system may apply larger minimum leeway values to the
 * timer in order to align it with other system activity.
 *
 * @constant DISPATCH_INTERVAL_UI_ANIMATION
 * Specifies that the interval source is used for UI animation. The unit for
 * the interval value of such sources is frames (1/60th of a second) and the
 * leeway is fixed at one frame.
 */
enum {
	DISPATCH_TIMER_BACKGROUND = 0x2,
	DISPATCH_INTERVAL_UI_ANIMATION = 0x20,
};

/*!
 * @enum dispatch_source_mach_send_flags_t
 *
 * @constant DISPATCH_MACH_SEND_POSSIBLE
 * The mach port corresponding to the given send right has space available
 * for messages. Delivered only once a mach_msg() to that send right with
 * options MACH_SEND_MSG|MACH_SEND_TIMEOUT|MACH_SEND_NOTIFY has returned
 * MACH_SEND_TIMED_OUT (and not again until the next such mach_msg() timeout).
 * NOTE: The source must have registered the send right for monitoring with the
 *       system for such a mach_msg() to arm the send-possible notifcation, so
 *       the initial send attempt must occur from a source registration handler.
 */
enum {
	DISPATCH_MACH_SEND_POSSIBLE = 0x8,
};

/*!
 * @enum dispatch_source_proc_flags_t
 *
 * @constant DISPATCH_PROC_REAP
 * The process has been reaped by the parent process via wait*().
 * This flag is deprecated and will be removed in a future release.
 */
enum {
	DISPATCH_PROC_REAP __OSX_AVAILABLE_BUT_DEPRECATED(
			__MAC_10_6, __MAC_10_9, __IPHONE_4_0, __IPHONE_7_0) = 0x10000000,
};

/*!
 * @enum dispatch_source_vm_flags_t
 *
 * @constant DISPATCH_VM_PRESSURE
 * The VM has experienced memory pressure.
 */

enum {
	DISPATCH_VM_PRESSURE __OSX_AVAILABLE_BUT_DEPRECATED_MSG(
			__MAC_10_7, __MAC_10_10, __IPHONE_4_3, __IPHONE_8_0,
			"Use DISPATCH_MEMORYSTATUS_PRESSURE_WARN instead") = 0x80000000,
};

/*!
 * @enum dispatch_source_memorystatus_flags_t
 *
 * @constant DISPATCH_MEMORYSTATUS_PRESSURE_NORMAL
 * The system's memory pressure state has returned to normal.
 * @constant DISPATCH_MEMORYSTATUS_PRESSURE_WARN
 * The system's memory pressure state has changed to warning.
 * @constant DISPATCH_MEMORYSTATUS_PRESSURE_CRITICAL
 * The system's memory pressure state has changed to critical.
 * @constant DISPATCH_MEMORYSTATUS_LOW_SWAP
 * The system's memory pressure state has entered the "low swap" condition.
 * Restricted to the root user.
 */

enum {
	DISPATCH_MEMORYSTATUS_PRESSURE_NORMAL
			__OSX_AVAILABLE_STARTING(__MAC_10_9, __IPHONE_6_0) = 0x01,
	DISPATCH_MEMORYSTATUS_PRESSURE_WARN
			__OSX_AVAILABLE_STARTING(__MAC_10_9, __IPHONE_6_0) = 0x02,
	DISPATCH_MEMORYSTATUS_PRESSURE_CRITICAL
			__OSX_AVAILABLE_STARTING(__MAC_10_9, __IPHONE_8_0) = 0x04,
	DISPATCH_MEMORYSTATUS_LOW_SWAP
			__OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_8_0) = 0x08,
};

__BEGIN_DECLS

/*!
 * @typedef dispatch_timer_aggregate_t
 *
 * @abstract
 * Dispatch timer aggregates are sets of related timers.
 */
DISPATCH_DECL(dispatch_timer_aggregate);

/*!
 * @function dispatch_timer_aggregate_create
 *
 * @abstract
 * Creates a new dispatch timer aggregate.
 *
 * @discussion
 * A dispatch timer aggregate is a set of related timers whose overall timing
 * parameters can be queried.
 *
 * Timers are added to an aggregate when a timer source is created with type
 * DISPATCH_SOURCE_TYPE_TIMER_WITH_AGGREGATE.
 *
 * @result
 * The newly created dispatch timer aggregate.
 */
__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0)
DISPATCH_EXPORT DISPATCH_MALLOC DISPATCH_RETURNS_RETAINED DISPATCH_WARN_RESULT
DISPATCH_NOTHROW
dispatch_timer_aggregate_t
dispatch_timer_aggregate_create(void);

/*!
 * @function dispatch_timer_aggregate_get_delay
 *
 * @abstract
 * Retrieves the delay until a timer in the given aggregate will next fire.
 *
 * @param aggregate
 * The dispatch timer aggregate to query.
 *
 * @param leeway_ptr
 * Optional pointer to a variable filled with the leeway (in ns) that will be
 * applied to the return value. May be NULL.
 *
 * @result
 * Delay in ns from now.
 */
__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0)
DISPATCH_EXPORT DISPATCH_NOTHROW
uint64_t
dispatch_timer_aggregate_get_delay(dispatch_timer_aggregate_t aggregate,
		uint64_t *leeway_ptr);

#if TARGET_OS_MAC
/*!
 * @typedef dispatch_mig_callback_t
 *
 * @abstract
 * The signature of a function that handles Mach message delivery and response.
 */
typedef boolean_t (*dispatch_mig_callback_t)(mach_msg_header_t *message,
		mach_msg_header_t *reply);

__OSX_AVAILABLE_STARTING(__MAC_10_6,__IPHONE_4_0)
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_NOTHROW
mach_msg_return_t
dispatch_mig_server(dispatch_source_t ds, size_t maxmsgsz,
		dispatch_mig_callback_t callback);

/*!
 * @function dispatch_mach_msg_get_context
 *
 * @abstract
 * Extract the context pointer from a mach message trailer.
 */
__OSX_AVAILABLE_STARTING(__MAC_10_6,__IPHONE_4_0)
DISPATCH_EXPORT DISPATCH_PURE DISPATCH_WARN_RESULT DISPATCH_NONNULL_ALL
DISPATCH_NOTHROW
void *
dispatch_mach_msg_get_context(mach_msg_header_t *msg);
#endif

__END_DECLS

#endif
