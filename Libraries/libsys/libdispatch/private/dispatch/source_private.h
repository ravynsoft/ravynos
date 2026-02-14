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

DISPATCH_ASSUME_NONNULL_BEGIN

__BEGIN_DECLS

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
 *
 * Starting with macOS 10.14, iOS 12, dispatch_source_set_timer()
 * can be used on such sources, and its arguments are used as follow:
 * - start:
 *   must be DISPATCH_TIME_NOW or DISPATCH_TIME_FOREVER.
 *   DISPATCH_TIME_NOW will enable the timer, and align its phase, and
 *   DISPATCH_TIME_FOREVER will disable the timer as usual.*
 * - interval:
 *   its unit is in milliseconds by default, or frames if the source
 *   was created with the DISPATCH_INTERVAL_UI_ANIMATION flag.
 * - leeway:
 *   per-thousands of the interval (valid values range from 0 to 1000).
 *   If ~0ull is passed, the default leeway for the interval is used instead.
 */
#define DISPATCH_SOURCE_TYPE_INTERVAL (&_dispatch_source_type_interval)
API_AVAILABLE(macos(10.9), ios(7.0))
DISPATCH_SOURCE_TYPE_DECL(interval);

/*!
 * @const DISPATCH_SOURCE_TYPE_VFS
 * @discussion Apple-internal dispatch source that monitors for vfs events
 * defined by dispatch_vfs_flags_t.
 * The handle is a process identifier (pid_t).
 */
#define DISPATCH_SOURCE_TYPE_VFS (&_dispatch_source_type_vfs)
API_AVAILABLE(macos(10.6), ios(4.0)) DISPATCH_LINUX_UNAVAILABLE()
DISPATCH_SOURCE_TYPE_DECL(vfs);

/*!
 * @const DISPATCH_SOURCE_TYPE_VM
 * @discussion A dispatch source that monitors virtual memory
 * The mask is a mask of desired events from dispatch_source_vm_flags_t.
 * This type is deprecated, use DISPATCH_SOURCE_TYPE_MEMORYPRESSURE instead.
 */
#define DISPATCH_SOURCE_TYPE_VM (&_dispatch_source_type_vm)
API_DEPRECATED_WITH_REPLACEMENT("DISPATCH_SOURCE_TYPE_MEMORYPRESSURE",
		macos(10.7,10.10), ios(4.3,8.0)) DISPATCH_LINUX_UNAVAILABLE()
DISPATCH_SOURCE_TYPE_DECL(vm);

/*!
 * @const DISPATCH_SOURCE_TYPE_MEMORYSTATUS
 * @discussion A dispatch source that monitors memory status
 * The mask is a mask of desired events from
 * dispatch_source_memorystatus_flags_t.
 */
#define DISPATCH_SOURCE_TYPE_MEMORYSTATUS (&_dispatch_source_type_memorystatus)
API_DEPRECATED_WITH_REPLACEMENT("DISPATCH_SOURCE_TYPE_MEMORYPRESSURE",
		macos(10.9, 10.12), ios(6.0, 10.0), tvos(6.0, 10.0), watchos(1.0, 3.0))
DISPATCH_LINUX_UNAVAILABLE()
DISPATCH_SOURCE_TYPE_DECL(memorystatus);

/*!
 * @const DISPATCH_SOURCE_TYPE_SOCK
 * @discussion A dispatch source that monitors events on socket state changes.
 */
#define DISPATCH_SOURCE_TYPE_SOCK (&_dispatch_source_type_sock)
API_AVAILABLE(macos(10.8), ios(6.0)) DISPATCH_LINUX_UNAVAILABLE()
DISPATCH_SOURCE_TYPE_DECL(sock);

/*!
 * @const DISPATCH_SOURCE_TYPE_NW_CHANNEL
 * @discussion A dispatch source that monitors events on a network channel.
 */
#define DISPATCH_SOURCE_TYPE_NW_CHANNEL (&_dispatch_source_type_nw_channel)
API_AVAILABLE(macos(10.13), ios(11.0), tvos(11.0), watchos(4.0)) DISPATCH_LINUX_UNAVAILABLE()
DISPATCH_SOURCE_TYPE_DECL(nw_channel);

__END_DECLS

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
 *
 * @constant DISPATCH_SOCK_NOTIFY_ACK
 * Notify acknowledgement
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
	DISPATCH_SOCK_NOTIFY_ACK = 0x00004000,
};

/*!
 * @enum dispatch_source_nw_channel_flags_t
 *
 * @constant DISPATCH_NW_CHANNEL_FLOW_ADV_UPDATE
 * Received network channel flow advisory.
 * @constant DISPATCH_NW_CHANNEL_CHANNEL_EVENT
 * Received network channel event.
 * @constant DISPATCH_NW_CHANNEL_INTF_ADV_UPDATE
 * Received network channel interface advisory.
 */
enum {
	DISPATCH_NW_CHANNEL_FLOW_ADV_UPDATE = 0x00000001,
	DISPATCH_NW_CHANNEL_CHANNEL_EVENT = 0x00000002,
	DISPATCH_NW_CHANNEL_INTF_ADV_UPDATE = 0x00000004,
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
 *
 * @constant DISPATCH_VFS_QUOTA
 * We hit a user quota (quotactl) for this filesystem.
 *
 * @constant DISPATCH_VFS_NEARLOWDISK
 * Filesystem is nearly full (below NEARLOWDISK level).
 *
 * @constant DISPATCH_VFS_DESIREDDISK
 * Filesystem has exceeded the DESIREDDISK level
 *
 * @constant DISPATCH_VFS_FREE_SPACE_CHANGE
 * Filesystem free space changed.
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
	DISPATCH_VFS_QUOTA = 0x1000,
	DISPATCH_VFS_NEARLOWDISK = 0x2000,
	DISPATCH_VFS_DESIREDDISK = 0x4000,
	DISPATCH_VFS_FREE_SPACE_CHANGE = 0x8000,
};

/*!
 * @enum dispatch_clockid_t
 *
 * @discussion
 * These values can be used with DISPATCH_SOURCE_TYPE_TIMER as a "handle"
 * to anchor the timer to a given clock which allows for various optimizations.
 *
 * Note that using an explicit clock will make the dispatch source "strict"
 * like dispatch_source_set_mandatory_cancel_handler() does.
 *
 * @constant DISPATCH_CLOCKID_UPTIME
 * A monotonic clock that doesn't tick while the machine is asleep.
 * Equivalent to the CLOCK_UPTIME clock ID on BSD systems.
 *
 * @constant DISPATCH_CLOCKID_MONOTONIC
 * A monotonic clock that ticks while the machine sleeps.
 * Equivalent to POSIX CLOCK_MONOTONIC.
 * (Note that on Linux, CLOCK_MONOTONIC isn't conformant and doesn't tick while
 * sleeping, hence on Linux this is the same clock as CLOCK_BOOTTIME).
 *
 * @constant DISPATCH_CLOCKID_WALLTIME
 * A clock equivalent to the wall clock time, as returned by gettimeofday().
 * Equivalent to POSIX CLOCK_REALTIME.
 *
 * @constant DISPATCH_CLOCKID_REALTIME
 * An alias for DISPATCH_CLOCKID_WALLTIME to match the POSIX clock of the
 * same name.
 */
DISPATCH_ENUM(dispatch_clockid, uintptr_t,
	DISPATCH_CLOCKID_UPTIME DISPATCH_ENUM_API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0)) = 1,
	DISPATCH_CLOCKID_MONOTONIC DISPATCH_ENUM_API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0)) = 2,
	DISPATCH_CLOCKID_WALLTIME DISPATCH_ENUM_API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0)) = 3,
	DISPATCH_CLOCKID_REALTIME DISPATCH_ENUM_API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0)) = 3,
);

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
 * @enum dispatch_source_mach_recv_flags_t
 *
 * @constant DISPATCH_MACH_RECV_SYNC_PEEK
 * The receive source will participate in synchronous IPC priority inversion
 * avoidance when possible.
 */
enum {
	DISPATCH_MACH_RECV_SYNC_PEEK DISPATCH_ENUM_API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0), bridgeos(4.0)) =
			0x00008000,
};

/*!
 * @enum dispatch_source_proc_flags_t
 *
 * @constant DISPATCH_PROC_REAP
 * The process has been reaped by the parent process via wait*().
 * This flag is deprecated and will be removed in a future release.
 *
 * @constant DISPATCH_PROC_EXIT_STATUS
 * The process has exited. Specifying this flag allows the process exit status
 * to be retrieved from the source's status value, as returned by the
 * dispatch_source_get_extended_data() function. The macros
 * DISPATCH_PROC_EXIT_STATUS_EXITED(), DISPATCH_PROC_EXIT_STATUS_CODE(),
 * DISPATCH_PROC_EXIT_STATUS_SIGNALED(), DISPATCH_PROC_EXIT_STATUS_TERMSIG() and
 * DISPATCH_PROC_EXIT_STATUS_CORE_DUMPED() can be used to examine the status
 * value.
 */
enum {
	DISPATCH_PROC_REAP DISPATCH_ENUM_API_DEPRECATED("unsupported flag",
			macos(10.6,10.9), ios(4.0,7.0)) = 0x10000000,
	DISPATCH_PROC_EXIT_STATUS DISPATCH_ENUM_API_AVAILABLE(macos(10.13), ios(11.0), tvos(11.0), watchos(2.0)) = 0x04000000,
};

/*!
 * @enum dispatch_source_vm_flags_t
 *
 * @constant DISPATCH_VM_PRESSURE
 * The VM has experienced memory pressure.
 */

enum {
	DISPATCH_VM_PRESSURE DISPATCH_ENUM_API_DEPRECATED_WITH_REPLACEMENT("DISPATCH_MEMORYPRESSURE_WARN", macos(10.7, 10.10), ios(4.3, 8.0))
			= 0x80000000,
};

/*!
 * @typedef dispatch_source_memorypressure_flags_t
 * Type of dispatch_source_memorypressure flags
 *
 * @constant DISPATCH_MEMORYPRESSURE_LOW_SWAP
 * The system's memory pressure state has entered the "low swap" condition.
 * Restricted to the root user.
 */
enum {
	DISPATCH_MEMORYPRESSURE_LOW_SWAP DISPATCH_ENUM_API_AVAILABLE(macos(10.10), ios(8.0)) = 0x08,
};

/*!
 * @enum dispatch_source_memorystatus_flags_t
 * @warning Deprecated, see DISPATCH_MEMORYPRESSURE_*
 */
enum {
	DISPATCH_MEMORYSTATUS_PRESSURE_NORMAL
			DISPATCH_ENUM_API_DEPRECATED_WITH_REPLACEMENT("DISPATCH_MEMORYPRESSURE_NORMAL", macos(10.9, 10.12),
			ios(6.0, 10.0), tvos(6.0, 10.0), watchos(1.0, 3.0)) = 0x01,
	DISPATCH_MEMORYSTATUS_PRESSURE_WARN
			DISPATCH_ENUM_API_DEPRECATED_WITH_REPLACEMENT("DISPATCH_MEMORYPRESSURE_WARN", macos(10.9, 10.12),
			ios(6.0, 10.0), tvos(6.0, 10.0), watchos(1.0, 3.0)) = 0x02,
	DISPATCH_MEMORYSTATUS_PRESSURE_CRITICAL
			DISPATCH_ENUM_API_DEPRECATED_WITH_REPLACEMENT("DISPATCH_MEMORYPRESSURE_CRITICAL", macos(10.9, 10.12),
			ios(6.0, 10.0), tvos(6.0, 10.0), watchos(1.0, 3.0)) = 0x04,
	DISPATCH_MEMORYSTATUS_LOW_SWAP
			DISPATCH_ENUM_API_DEPRECATED_WITH_REPLACEMENT("DISPATCH_MEMORYPRESSURE_LOW_SWAP", macos(10.9, 10.12),
			ios(6.0, 10.0), tvos(6.0, 10.0), watchos(1.0, 3.0)) = 0x08,
};

/*!
 * @typedef dispatch_source_memorypressure_flags_t
 * Type of dispatch_source_memorypressure flags
 *
 * @constant DISPATCH_MEMORYPRESSURE_PROC_LIMIT_WARN
 * The memory of the process has crossed 80% of its high watermark limit.
 *
 * @constant DISPATCH_MEMORYPRESSURE_PROC_LIMIT_CRITICAL
 * The memory of the process has reached 100% of its high watermark limit.
 *
 * @constant DISPATCH_MEMORYPRESSURE_MSL_STATUS
 * Mask for enabling/disabling malloc stack logging.
 */
enum {
	DISPATCH_MEMORYPRESSURE_PROC_LIMIT_WARN DISPATCH_ENUM_API_AVAILABLE(macos(10.12), ios(10.0), tvos(10.0), watchos(3.0)) = 0x10,

	DISPATCH_MEMORYPRESSURE_PROC_LIMIT_CRITICAL DISPATCH_ENUM_API_AVAILABLE(macos(10.12), ios(10.0), tvos(10.0), watchos(3.0)) = 0x20,

	DISPATCH_MEMORYPRESSURE_MSL_STATUS DISPATCH_ENUM_API_AVAILABLE(macos(10.13), ios(11.0), tvos(11.0), watchos(4.0)) = 0xf0000000,
};

/*!
 * Macros to check the exit status obtained from the status field of the
 * structure returned by the dispatch_source_get_extended_data() function for a
 * source of type DISPATCH_SOURCE_TYPE_PROC when DISPATCH_PROC_EXIT_STATUS has
 * been requested.
 *
 * DISPATCH_PROC_EXIT_STATUS_EXITED returns whether the process exited. If this
 * is true, the exit status can be obtained from DISPATCH_PROC_EXIT_STATUS_CODE.
 *
 * DISPATCH_PROC_EXIT_STATUS_SIGNALED returns whether the process was terminated
 * by a signal.
 *
 * DISPATCH_PROC_EXIT_STATUS_TERMSIG returns the signal that caused the process
 * to terminate, or 0 if the process was not terminated by a signal.
 *
 * DISPATCH_PROC_EXIT_STATUS_CORE_DUMPED returns whether a core dump of the
 * process was created.
 */
#define DISPATCH_PROC_EXIT_STATUS_EXITED(status) ((bool)WIFEXITED(status))
#define DISPATCH_PROC_EXIT_STATUS_CODE(status) ((int)WEXITSTATUS(status))
#define DISPATCH_PROC_EXIT_STATUS_SIGNALED(status) ((bool)WIFSIGNALED(status))
#define DISPATCH_PROC_EXIT_STATUS_TERMSIG(status) ((int)WTERMSIG(status))
#define DISPATCH_PROC_EXIT_STATUS_CORE_DUMPED(status) ((bool)WCOREDUMP(status))

__BEGIN_DECLS

/*!
 * @function dispatch_source_set_mandatory_cancel_handler
 *
 * @abstract
 * Sets the event handler block for the given dispatch source, and indicates
 * that calling dispatch_source_cancel() is mandatory for this source object.
 *
 * @discussion
 * The cancellation handler (if specified) will be submitted to the source's
 * target queue in response to a call to dispatch_source_cancel() once the
 * system has released all references to the source's underlying handle and
 * the source's event handler block has returned.
 *
 * When this function has been used used to set a cancellation handler, then
 * the following result in an assertion and the process being terminated:
 * - releasing the last reference on the dispatch source without having
 *   cancelled it by calling dispatch_source_cancel();
 * - changing any handler after the source has been activated;
 * - changing the target queue of the source after it has been activated.
 *
 * IMPORTANT:
 * Source cancellation and a cancellation handler are required for file
 * descriptor and mach port based sources in order to safely close the
 * descriptor or destroy the port. Making the cancellation handler of such
 * sources mandatory is strongly recommended.
 * Closing the descriptor or port before the cancellation handler is invoked may
 * result in a race condition. If a new descriptor is allocated with the same
 * value as the recently closed descriptor while the source's event handler is
 * still running, the event handler may read/write data to the wrong descriptor.
 *
 * @param source
 * The dispatch source to modify.
 * The result of passing NULL in this parameter is undefined.
 *
 * @param handler
 * The cancellation handler block to submit to the source's target queue.
 * The result of passing NULL in this parameter is undefined.
 */
#ifdef __BLOCKS__
API_AVAILABLE(macos(10.13), ios(11.0), tvos(11.0), watchos(4.0))
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_NOTHROW
void
dispatch_source_set_mandatory_cancel_handler(dispatch_source_t source,
		dispatch_block_t handler);
#endif /* __BLOCKS__ */

/*!
 * @function dispatch_source_set_mandatory_cancel_handler_f
 *
 * @abstract
 * Sets the event handler function for the given dispatch source, and causes an
 * assertion if this source is released before having been explicitly canceled.
 *
 * @discussion
 * See dispatch_source_set_mandatory_cancel_handler() for more details.
 *
 * @param source
 * The dispatch source to modify.
 * The result of passing NULL in this parameter is undefined.
 *
 * @param handler
 * The cancellation handler function to submit to the source's target queue.
 * The context parameter passed to the event handler function is the current
 * context of the dispatch source at the time the handler call is made.
 * The result of passing NULL in this parameter is undefined.
 */
API_AVAILABLE(macos(10.13), ios(11.0), tvos(11.0), watchos(4.0))
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_NOTHROW
void
dispatch_source_set_mandatory_cancel_handler_f(dispatch_source_t source,
		dispatch_function_t handler);

/*!
 * @function dispatch_source_cancel_and_wait
 *
 * @abstract
 * Synchronously cancel the dispatch source, preventing any further invocation
 * of its event handler block.
 *
 * @discussion
 * Cancellation prevents any further invocation of handler blocks for the
 * specified dispatch source, but does not interrupt a handler block that is
 * already in progress.
 *
 * When this function returns, any handler block that may have been in progress
 * has returned, the specified source has been unregistered and it is safe to
 * reclaim any system resource (such as file descriptors or mach ports) that
 * the specified source was monitoring.
 *
 * If the specified dispatch source is inactive, it will be activated as a side
 * effect of calling this function.
 *
 * It is possible to call this function from several threads concurrently,
 * and it is the responsibility of the callers to synchronize reclaiming the
 * associated system resources.
 *
 * This function is not subject to priority inversion when it is waiting on
 * a handler block still in progress, unlike patterns based on waiting on
 * a dispatch semaphore or a dispatch group signaled (or left) from the source
 * cancel handler.
 *
 * This function must not be called if the specified source has a cancel
 * handler set, or from the context of its handler blocks.
 *
 * This function must not be called from the context of the target queue of
 * the specified source or from any queue that synchronizes with it. Note that
 * calling dispatch_source_cancel() from such a context already guarantees
 * that no handler is in progress, and that no new event will be delivered.
 *
 * This function must not be called on sources suspended with an explicit
 * call to dispatch_suspend(), or being concurrently activated on another
 * thread.
 *
 * @param source
 * The dispatch source to be canceled.
 * The result of passing NULL in this parameter is undefined.
 */
API_AVAILABLE(macos(10.12), ios(10.0), tvos(10.0), watchos(3.0))
DISPATCH_EXPORT DISPATCH_NOTHROW
void
dispatch_source_cancel_and_wait(dispatch_source_t source);

#if __has_include(<mach/mach.h>)
/*!
 * @typedef dispatch_mig_callback_t
 *
 * @abstract
 * The signature of a function that handles Mach message delivery and response.
 */
typedef boolean_t (*dispatch_mig_callback_t)(mach_msg_header_t *message,
		mach_msg_header_t *reply);

API_AVAILABLE(macos(10.6), ios(4.0)) DISPATCH_LINUX_UNAVAILABLE()
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
API_AVAILABLE(macos(10.6), ios(4.0)) DISPATCH_LINUX_UNAVAILABLE()
DISPATCH_EXPORT DISPATCH_PURE DISPATCH_WARN_RESULT DISPATCH_NONNULL_ALL
DISPATCH_NOTHROW
void *_Nullable
dispatch_mach_msg_get_context(mach_msg_header_t *msg);
#endif

/*!
 * @typedef dispatch_source_extended_data_t
 *
 * @abstract
 * Type used by dispatch_source_get_extended_data() to return a consistent
 * snapshot of the data and status of a dispatch source.
 */
typedef struct dispatch_source_extended_data_s {
    unsigned long data;
    unsigned long status;
} *dispatch_source_extended_data_t;

/*!
 * @function dispatch_source_get_extended_data
 *
 * @abstract
 * Returns the current data and status values for a dispatch source.
 *
 * @discussion
 * This function is intended to be called from within the event handler block.
 * The result of calling this function outside of the event handler callback is
 * undefined.
 *
 * @param source
 * The result of passing NULL in this parameter is undefined.
 *
 * @param data
 * A pointer to a dispatch_source_extended_data_s in which the data and status
 * will be returned. The data field is populated with the value that would be
 * returned by dispatch_source_get_data(). The value of the status field should
 * be interpreted according to the type of the dispatch source:
 *
 *  DISPATCH_SOURCE_TYPE_PROC:            dispatch_source_proc_exit_flags_t
 *
 * If called from the event handler of a data source type not listed above, the
 * status value is undefined.
 *
 * @param size
 * The size of the specified structure. Should be set to
 * sizeof(dispatch_source_extended_data_s).
 *
 * @result
 * The size of the structure returned in *data, which will never be greater than
 * the value of the size argument. If this is less than the value of the size
 * argument, the remaining space in data will have been populated with zeroes.
 */
API_AVAILABLE(macos(10.13), ios(11.0), tvos(11.0), watchos(4.0))
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_WARN_RESULT DISPATCH_PURE
DISPATCH_NOTHROW
size_t
dispatch_source_get_extended_data(dispatch_source_t source,
		dispatch_source_extended_data_t data, size_t size);

__END_DECLS

DISPATCH_ASSUME_NONNULL_END

#endif
