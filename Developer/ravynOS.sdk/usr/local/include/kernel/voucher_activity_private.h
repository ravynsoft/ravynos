/*
 * Copyright (c) 2013-2015 Apple Inc. All rights reserved.
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

#ifndef __OS_VOUCHER_ACTIVITY_PRIVATE__
#define __OS_VOUCHER_ACTIVITY_PRIVATE__

#if OS_VOUCHER_ACTIVITY_SPI
#if __has_include(<mach/mach_time.h>)
#include <mach/mach_time.h>
#include <firehose/tracepoint_private.h>
#endif
#if __APPLE__
#include <os/base.h>
#include <os/availability.h>
#endif
#include <sys/uio.h>
#include <os/object.h>
#include "voucher_private.h"

#define OS_VOUCHER_ACTIVITY_SPI_VERSION 20161003

#if OS_VOUCHER_WEAK_IMPORT
#define OS_VOUCHER_EXPORT OS_EXPORT OS_WEAK_IMPORT
#else
#define OS_VOUCHER_EXPORT OS_EXPORT
#endif

__BEGIN_DECLS

/*!
 * @const VOUCHER_CURRENT
 * Shorthand for the currently adopted voucher
 *
 * This value can only be used as an argument to functions, and is never
 * actually returned. It looks enough like a tagged pointer object that ARC
 * won't crash if this is assigned to a temporary variable.
 */
#define VOUCHER_CURRENT		((OS_OBJECT_BRIDGE voucher_t)(void *)~2ul)

/*!
 * @function voucher_get_activity_id
 *
 * @abstract
 * Returns the activity_id associated with the specified voucher at the time
 * of the call.
 *
 * @discussion
 * When the passed voucher is VOUCHER_CURRENT this returns the current
 * activity ID.
 *
 * @param voucher
 * The specified voucher.
 *
 * @param parent_id
 * An out parameter to return the parent ID of the returned activity ID.
 *
 * @result
 * The current activity identifier, if any. When 0 is returned, parent_id will
 * also always be 0.
 */
API_AVAILABLE(macos(10.12), ios(10.0), tvos(10.0), watchos(3.0))
OS_VOUCHER_EXPORT OS_NOTHROW
firehose_activity_id_t
voucher_get_activity_id(voucher_t voucher, firehose_activity_id_t *parent_id);

/*!
 * @function voucher_get_activity_id_and_creator
 *
 * @abstract
 * Returns the activity_id associated with the specified voucher at the time
 * of the call.
 *
 * @discussion
 * When the passed voucher is VOUCHER_CURRENT this returns the current
 * activity ID.
 *
 * @param voucher
 * The specified voucher.
 *
 * @param creator_pid
 * The unique pid of the process that created the returned activity ID if any.
 *
 * @param parent_id
 * An out parameter to return the parent ID of the returned activity ID.
 *
 * @result
 * The current activity identifier, if any. When 0 is returned, parent_id will
 * also always be 0.
 */
API_AVAILABLE(macos(10.12), ios(10.0), tvos(10.0), watchos(3.0))
OS_VOUCHER_EXPORT OS_NOTHROW
firehose_activity_id_t
voucher_get_activity_id_and_creator(voucher_t voucher, uint64_t *creator_pid,
		firehose_activity_id_t *parent_id);

/*!
 * @function voucher_activity_create_with_data
 *
 * @abstract
 * Creates a voucher object with a new activity identifier.
 *
 * @discussion
 * As part of voucher transport, activities are automatically propagated by the
 * system to other threads and processes (across IPC).
 *
 * When a voucher with an activity identifier is applied to a thread, work
 * on that thread is done on behalf of this activity.
 *
 * @param trace_id
 * Tracepoint identifier returned by voucher_activity_trace_id(), intended for
 * identification of the automatic tracepoint generated as part of creating the
 * new activity.
 *
 * @param base
 * The base voucher used to create the activity. If the base voucher has an
 * activity identifier, then the created activity will be parented to that one.
 * If the passed in base has no activity identifier, the activity identifier
 * will be a top-level one, on behalf of the process that created the base
 * voucher.
 *
 * If base is VOUCHER_NONE, the activity is a top-level one, on behalf of the
 * current process.
 *
 * If base is VOUCHER_CURRENT, then the activity is naturally based on the
 * one currently applied to the current thread (the one voucher_copy() would
 * return).
 *
 * @param flags
 * See voucher_activity_flag_t documentation for effect.
 *
 * @param pubdata
 * Pointer to packed buffer of tracepoint data.
 *
 * @param publen
 * Length of data at 'pubdata'.
 *
 * @result
 * A new voucher with an activity identifier.
 */
API_AVAILABLE(macos(10.12.4), ios(10.3), tvos(10.2), watchos(3.2))
OS_VOUCHER_EXPORT OS_OBJECT_RETURNS_RETAINED OS_WARN_RESULT OS_NOTHROW
voucher_t
voucher_activity_create_with_data(firehose_tracepoint_id_t *trace_id,
		voucher_t base, firehose_activity_flags_t flags,
		const void *pubdata, size_t publen);

API_DEPRECATED_WITH_REPLACEMENT("voucher_activity_create_with_data",
		macos(10.12,10.12.4), ios(10.0,10.3), tvos(10.0,10.2), watchos(3.0,3.2))
OS_VOUCHER_EXPORT OS_OBJECT_RETURNS_RETAINED OS_WARN_RESULT OS_NOTHROW
voucher_t
voucher_activity_create_with_location(firehose_tracepoint_id_t *trace_id,
		voucher_t base, firehose_activity_flags_t flags, uint64_t location);

/*!
 * @group Voucher Activity Trace SPI
 * SPI intended for libtrace only
 */

/*!
 * @function voucher_activity_id_allocate
 *
 * @abstract
 * Allocate a new system-wide unique activity ID.
 *
 * @param flags
 * The bottom-most 8 bits of the flags will be used to generate the ID.
 * See firehose_activity_flags_t.
 */
API_AVAILABLE(macos(10.13), ios(11.0), tvos(11.0), watchos(4.0))
OS_VOUCHER_EXPORT OS_NOTHROW
firehose_activity_id_t
voucher_activity_id_allocate(firehose_activity_flags_t flags);

/*!
 * @function voucher_activity_flush
 *
 * @abstract
 * Force flushing the specified stream.
 *
 * @discussion
 * This maks all the buffers currently being written to as full, so that
 * their current content is pushed in a timely fashion.
 *
 * When this call returns, the actual flush may or may not yet have happened.
 *
 * @param stream
 * The stream to flush.
 */
API_AVAILABLE(macos(10.12), ios(10.0), tvos(10.0), watchos(3.0))
OS_VOUCHER_EXPORT OS_NOTHROW
void
voucher_activity_flush(firehose_stream_t stream);

/*!
 * @function voucher_activity_trace
 *
 * @abstract
 * Add a tracepoint to the specified stream.
 *
 * @param stream
 * The stream to trace this entry into.
 *
 * @param trace_id
 * Tracepoint identifier returned by voucher_activity_trace_id()
 *
 * @param timestamp
 * The mach_approximate_time()/mach_absolute_time() value for this tracepoint.
 *
 * @param pubdata
 * Pointer to packed buffer of tracepoint data.
 *
 * @param publen
 * Length of data at 'pubdata'.
 */
API_AVAILABLE(macos(10.12), ios(10.0), tvos(10.0), watchos(3.0))
OS_VOUCHER_EXPORT OS_NOTHROW OS_NONNULL4
firehose_tracepoint_id_t
voucher_activity_trace(firehose_stream_t stream,
		firehose_tracepoint_id_t trace_id, uint64_t timestamp,
		const void *pubdata, size_t publen);

/*!
 * @function voucher_activity_trace_v
 *
 * @abstract
 * Add a tracepoint to the specified stream, with private data.
 *
 * @param stream
 * The stream to trace this entry into.
 *
 * @param trace_id
 * Tracepoint identifier returned by voucher_activity_trace_id()
 *
 * @param timestamp
 * The mach_approximate_time()/mach_absolute_time() value for this tracepoint.
 *
 * @param iov
 * Array of `struct iovec` pointing to the data to layout.
 * The total size of this iovec must span exactly `publen + privlen` bytes.
 * The `publen` boundary must coincide with the end of an iovec (each iovec
 * must either be pure public or pure private data).
 *
 * @param publen
 * Total length of data to read from the iovec for the public data.
 *
 * @param privlen
 * Length of data to read from the iovec after the public data for the private
 * data.
 */
API_AVAILABLE(macos(10.12.4), ios(10.3), tvos(10.2), watchos(3.2))
OS_VOUCHER_EXPORT OS_NOTHROW OS_NONNULL4
firehose_tracepoint_id_t
voucher_activity_trace_v(firehose_stream_t stream,
		firehose_tracepoint_id_t trace_id, uint64_t timestamp,
		const struct iovec *iov, size_t publen, size_t privlen);

#define VOUCHER_ACTIVITY_TRACE_FLAG_UNRELIABLE 0x01

API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0))
OS_VOUCHER_EXPORT OS_NOTHROW OS_NONNULL4
firehose_tracepoint_id_t
voucher_activity_trace_v_2(firehose_stream_t stream,
		firehose_tracepoint_id_t trace_id, uint64_t timestamp,
		const struct iovec *iov, size_t publen, size_t privlen, uint32_t flags);

typedef const struct voucher_activity_hooks_s {
#define VOUCHER_ACTIVITY_HOOKS_VERSION     5
	long vah_version;
	mach_port_t (*vah_get_logd_port)(void);
	dispatch_mach_handler_function_t vah_debug_channel_handler;
	kern_return_t (*vah_get_reconnect_info)(mach_vm_address_t *, mach_vm_size_t *);
	void (*vah_metadata_init)(void *metadata_buffer, size_t size);
	void (*vah_quarantine_starts)(void);
} *voucher_activity_hooks_t;

/*!
 * @function voucher_activity_initialize_4libtrace
 *
 * @abstract
 * Configure upcall hooks for libtrace.
 *
 * @param hooks
 * A pointer to a voucher_activity_hooks_s structure.
 */
API_AVAILABLE(macos(10.12), ios(10.0), tvos(10.0), watchos(3.0))
OS_VOUCHER_EXPORT OS_NOTHROW OS_NONNULL_ALL
void
voucher_activity_initialize_4libtrace(voucher_activity_hooks_t hooks);

/*!
 * @function voucher_activity_get_metadata_buffer
 *
 * @abstract
 * Return address and length of buffer in the process trace memory area
 * reserved for libtrace metadata.
 *
 * @param length
 * Pointer to size_t variable, filled with length of metadata buffer.
 *
 * @result
 * Address of metadata buffer.
 */
API_AVAILABLE(macos(10.10), ios(8.0))
OS_VOUCHER_EXPORT OS_WARN_RESULT OS_NOTHROW OS_NONNULL_ALL
void *
voucher_activity_get_metadata_buffer(size_t *length);

/*!
 * @function voucher_activity_get_logging_preferences
 *
 * @abstract
 * Return address and length of vm_map()ed configuration data for the logging
 * subsystem.
 *
 * @discussion
 * The data must be deallocated with vm_deallocate().
 *
 * @param length
 * Pointer to size_t variable, filled with length of preferences buffer.
 *
 * @result
 * Address of preferences buffer, returns NULL on error.
 */
API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0), bridgeos(3.0))
OS_VOUCHER_EXPORT OS_WARN_RESULT OS_NOTHROW OS_NONNULL_ALL
void *
voucher_activity_get_logging_preferences(size_t *length);

/*!
 * @function voucher_activity_should_send_strings
 *
 * @abstract
 * Returns whether the client should send the strings or not.
 */
API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0), bridgeos(4.0))
OS_VOUCHER_EXPORT OS_WARN_RESULT OS_NOTHROW
bool
voucher_activity_should_send_strings(void);

/*!
 * @function voucher_get_activity_id_4dyld
 *
 * @abstract
 * Return the current voucher activity ID. Available for the dyld client stub
 * only.
 */
API_AVAILABLE(macos(10.12), ios(10.0), tvos(10.0), watchos(3.0))
OS_VOUCHER_EXPORT OS_WARN_RESULT OS_NOTHROW
firehose_activity_id_t
voucher_get_activity_id_4dyld(void);

__END_DECLS

#endif // OS_VOUCHER_ACTIVITY_SPI

#endif // __OS_VOUCHER_ACTIVITY_PRIVATE__
