/*
 * Copyright (c) 2013-2014 Apple Inc. All rights reserved.
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

#include <os/base.h>
#include <os/object.h>
#if !defined(__DISPATCH_BUILDING_DISPATCH__)
#include <os/voucher_private.h>
#endif

#define OS_VOUCHER_ACTIVITY_SPI_VERSION 20140708

#if OS_VOUCHER_WEAK_IMPORT
#define OS_VOUCHER_EXPORT OS_EXPORT OS_WEAK_IMPORT
#else
#define OS_VOUCHER_EXPORT OS_EXPORT
#endif

__BEGIN_DECLS

#if OS_VOUCHER_ACTIVITY_SPI

/*!
 * @group Voucher Activity SPI
 * SPI intended for libtrace only
 */

/*!
 * @typedef voucher_activity_id_t
 *
 * @abstract
 * Opaque activity identifier.
 *
 * @discussion
 * Scalar value type, not reference counted.
 */
typedef uint64_t voucher_activity_id_t;

/*!
 * @enum voucher_activity_tracepoint_type_t
 *
 * @abstract
 * Types of tracepoints.
 */
OS_ENUM(voucher_activity_tracepoint_type, uint8_t,
	voucher_activity_tracepoint_type_release = (1u << 0),
	voucher_activity_tracepoint_type_debug = (1u << 1),
	voucher_activity_tracepoint_type_error = (1u << 6) | (1u << 0),
	voucher_activity_tracepoint_type_fault = (1u << 7) | (1u << 6) | (1u << 0),
);

/*!
 * @enum voucher_activity_flag_t
 *
 * @abstract
 * Flags to pass to voucher_activity_start/voucher_activity_start_with_location
 */
OS_ENUM(voucher_activity_flag, unsigned long,
	voucher_activity_flag_default = 0,
	voucher_activity_flag_force = 0x1,
);

/*!
 * @typedef voucher_activity_trace_id_t
 *
 * @abstract
 * Opaque tracepoint identifier.
 */
typedef uint64_t voucher_activity_trace_id_t;
static const uint8_t _voucher_activity_trace_id_type_shift = 40;
static const uint8_t _voucher_activity_trace_id_code_namespace_shift = 32;

/*!
 * @function voucher_activity_trace_id
 *
 * @abstract
 * Return tracepoint identifier for specified arguments.
 *
 * @param type
 * Tracepoint type from voucher_activity_tracepoint_type_t.
 *
 * @param code_namespace
 * Namespace of 'code' argument.
 *
 * @param code
 * Tracepoint code.
 *
 * @result
 * Tracepoint identifier.
 */
__OSX_AVAILABLE_STARTING(__MAC_10_10,__IPHONE_8_0)
OS_INLINE OS_ALWAYS_INLINE
voucher_activity_trace_id_t
voucher_activity_trace_id(uint8_t type, uint8_t code_namespace, uint32_t code)
{
	return ((voucher_activity_trace_id_t)type <<
			_voucher_activity_trace_id_type_shift) |
			((voucher_activity_trace_id_t)code_namespace <<
			_voucher_activity_trace_id_code_namespace_shift) |
			(voucher_activity_trace_id_t)code;
}

/*!
 * @function voucher_activity_start
 *
 * @abstract
 * Creates a new activity identifier and marks the current thread as
 * participating in the activity.
 *
 * @discussion
 * As part of voucher transport, activities are automatically propagated by the
 * system to other threads and processes (across IPC).
 *
 * Activities persist as long as any threads in any process are marked as
 * participating. There may be many calls to voucher_activity_end()
 * corresponding to one call to voucher_activity_start().
 *
 * @param trace_id
 * Tracepoint identifier returned by voucher_activity_trace_id(), intended for
 * identification of the automatic tracepoint generated as part of creating the
 * new activity.
 *
 * @param flags
 * Pass voucher_activity_flag_force to indicate that existing activities
 * on the current thread should not be inherited and that a new toplevel
 * activity should be created.
 *
 * @result
 * A new activity identifier.
 */
__OSX_AVAILABLE_STARTING(__MAC_10_10,__IPHONE_8_0)
OS_VOUCHER_EXPORT OS_WARN_RESULT OS_NOTHROW
voucher_activity_id_t
voucher_activity_start(voucher_activity_trace_id_t trace_id,
		voucher_activity_flag_t flags);

/*!
 * @function voucher_activity_start_with_location
 *
 * @abstract
 * Creates a new activity identifier and marks the current thread as
 * participating in the activity.
 *
 * @discussion
 * As part of voucher transport, activities are automatically propagated by the
 * system to other threads and processes (across IPC).
 *
 * Activities persist as long as any threads in any process are marked as
 * participating. There may be many calls to voucher_activity_end()
 * corresponding to one call to voucher_activity_start_with_location().
 *
 * @param trace_id
 * Tracepoint identifier returned by voucher_activity_trace_id(), intended for
 * identification of the automatic tracepoint generated as part of creating the
 * new activity.
 *
 * @param location
 * Location identifier for the automatic tracepoint generated as part of
 * creating the new activity.
 *
 * @param flags
 * Pass voucher_activity_flag_force to indicate that existing activities
 * on the current thread should not be inherited and that a new toplevel
 * activity should be created.
 *
 * @result
 * A new activity identifier.
 */
__OSX_AVAILABLE_STARTING(__MAC_10_10,__IPHONE_8_0)
OS_VOUCHER_EXPORT OS_WARN_RESULT OS_NOTHROW
voucher_activity_id_t
voucher_activity_start_with_location(voucher_activity_trace_id_t trace_id,
		uint64_t location, voucher_activity_flag_t flags);

/*!
 * @function voucher_activity_end
 *
 * @abstract
 * Unmarks the current thread if it is marked as particpating in the activity
 * with the specified identifier.
 *
 * @discussion
 * Activities persist as long as any threads in any process are marked as
 * participating. There may be many calls to voucher_activity_end()
 * corresponding to one call to voucher_activity_start() or
 * voucher_activity_start_with_location().
 */
__OSX_AVAILABLE_STARTING(__MAC_10_10,__IPHONE_8_0)
OS_VOUCHER_EXPORT OS_NOTHROW
void
voucher_activity_end(voucher_activity_id_t activity_id);

/*!
 * @function voucher_get_activities
 *
 * @abstract
 * Returns the list of activity identifiers that the current thread is marked
 * with.
 *
 * @param entries
 * Pointer to an array of activity identifiers to be filled in.
 *
 * @param count
 * Pointer to the requested number of activity identifiers.
 * On output will be filled with the number of activities that are available.
 *
 * @result
 * Number of activity identifiers written to 'entries'
 */
__OSX_AVAILABLE_STARTING(__MAC_10_10,__IPHONE_8_0)
OS_VOUCHER_EXPORT OS_NOTHROW
unsigned int
voucher_get_activities(voucher_activity_id_t *entries, unsigned int *count);

/*!
 * @group Voucher Activity Trace SPI
 * SPI intended for libtrace only
 */

/*!
 * @function voucher_activity_get_namespace
 *
 * @abstract
 * Returns the namespace of the current activity.
 *
 * @result
 * The namespace of the current activity (if any).
 */
__OSX_AVAILABLE_STARTING(__MAC_10_10,__IPHONE_8_0)
OS_VOUCHER_EXPORT OS_NOTHROW
uint8_t
voucher_activity_get_namespace(void);

/*!
 * @function voucher_activity_trace
 *
 * @abstract
 * Add a tracepoint to trace buffer of the current activity.
 *
 * @param trace_id
 * Tracepoint identifier returned by voucher_activity_trace_id()
 *
 * @param location
 * Tracepoint location.
 *
 * @param buffer
 * Pointer to packed buffer of tracepoint data.
 *
 * @param length
 * Length of data at 'buffer'.
 *
 * @result
 * Timestamp recorded in tracepoint or 0 if no tracepoint was recorded.
 */
__OSX_AVAILABLE_STARTING(__MAC_10_10,__IPHONE_8_0)
OS_VOUCHER_EXPORT OS_NOTHROW
uint64_t
voucher_activity_trace(voucher_activity_trace_id_t trace_id, uint64_t location,
		void *buffer, size_t length);

/*!
 * @function voucher_activity_trace_args
 *
 * @abstract
 * Add a tracepoint to trace buffer of the current activity, recording
 * specified arguments passed in registers.
 *
 * @param trace_id
 * Tracepoint identifier returned by voucher_activity_trace_id()
 *
 * @param location
 * Tracepoint location.
 *
 * @param arg1
 * Argument to be recorded in tracepoint data.
 *
 * @param arg2
 * Argument to be recorded in tracepoint data.
 *
 * @param arg3
 * Argument to be recorded in tracepoint data.
 *
 * @param arg4
 * Argument to be recorded in tracepoint data.
 *
 * @result
 * Timestamp recorded in tracepoint or 0 if no tracepoint was recorded.
 */
__OSX_AVAILABLE_STARTING(__MAC_10_10,__IPHONE_8_0)
OS_VOUCHER_EXPORT OS_NOTHROW
uint64_t
voucher_activity_trace_args(voucher_activity_trace_id_t trace_id,
		uint64_t location, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3,
		uintptr_t arg4);

/*!
 * @group Voucher Activity Mode SPI
 * SPI intended for libtrace only
 */

/*!
 * @enum voucher_activity_mode_t
 *
 * @abstract
 * Voucher activity mode.
 *
 * @discussion
 * Configure at process start by setting the OS_ACTIVITY_MODE environment
 * variable.
 */
OS_ENUM(voucher_activity_mode, unsigned long,
	voucher_activity_mode_disable = 0,
	voucher_activity_mode_release = (1u << 0),
	voucher_activity_mode_debug = (1u << 1),
	voucher_activity_mode_stream = (1u << 2),
);

/*!
 * @function voucher_activity_get_mode
 *
 * @abstract
 * Return current mode of voucher activity subsystem.
 *
 * @result
 * Value from voucher_activity_mode_t enum.
 */
__OSX_AVAILABLE_STARTING(__MAC_10_10,__IPHONE_8_0)
OS_VOUCHER_EXPORT OS_WARN_RESULT OS_NOTHROW
voucher_activity_mode_t
voucher_activity_get_mode(void);

/*!
 * @function voucher_activity_set_mode_4libtrace(void)
 *
 * @abstract
 * Set the current mode of voucher activity subsystem.
 *
 * @param mode
 * The new mode.
 *
 * Note that the new mode will take effect soon, but not immediately.
 */
__OSX_AVAILABLE_STARTING(__MAC_10_10,__IPHONE_8_0)
OS_VOUCHER_EXPORT OS_NOTHROW
void
voucher_activity_set_mode_4libtrace(voucher_activity_mode_t mode);

/*!
 * @group Voucher Activity Metadata SPI
 * SPI intended for libtrace only
 */

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
__OSX_AVAILABLE_STARTING(__MAC_10_10,__IPHONE_8_0)
OS_VOUCHER_EXPORT OS_WARN_RESULT OS_NOTHROW OS_NONNULL_ALL
void*
voucher_activity_get_metadata_buffer(size_t *length);

#endif // OS_VOUCHER_ACTIVITY_SPI

#if OS_VOUCHER_ACTIVITY_BUFFER_SPI

/*!
 * @group Voucher Activity Tracepoint SPI
 * SPI intended for diagnosticd only
 */

OS_ENUM(_voucher_activity_tracepoint_flag, uint16_t,
	_voucher_activity_trace_flag_buffer_empty = 0,
	_voucher_activity_trace_flag_tracepoint = (1u << 0),
	_voucher_activity_trace_flag_tracepoint_args = (1u << 1),
	_voucher_activity_trace_flag_wide_first = (1u << 6),
	_voucher_activity_trace_flag_wide_second = (1u << 6) | (1u << 7),
	_voucher_activity_trace_flag_start = (1u << 8),
	_voucher_activity_trace_flag_end = (1u << 8) | (1u << 9),
	_voucher_activity_trace_flag_libdispatch = (1u << 13),
	_voucher_activity_trace_flag_activity = (1u << 14),
	_voucher_activity_trace_flag_buffer_header = (1u << 15),
);

// for tracepoints with _voucher_activity_trace_flag_libdispatch
OS_ENUM(_voucher_activity_tracepoint_namespace, uint8_t,
	_voucher_activity_tracepoint_namespace_ipc = 0x1
);
OS_ENUM(_voucher_activity_tracepoint_code, uint32_t,
	_voucher_activity_tracepoint_namespace_ipc_send = 0x1,
	_voucher_activity_tracepoint_namespace_ipc_receive = 0x2,
);

typedef struct _voucher_activity_tracepoint_s {
	uint16_t vat_flags;		// voucher_activity_tracepoint_flag_t
	uint8_t  vat_type;		// voucher_activity_tracepoint_type_t
	uint8_t  vat_namespace;	// namespace for tracepoint code
	uint32_t vat_code;		// tracepoint code
	uint64_t vat_thread;	// pthread_t
	uint64_t vat_timestamp;	// absolute time
	uint64_t vat_location;	// tracepoint PC
	uint64_t vat_data[4];	// trace data
} *_voucher_activity_tracepoint_t;

/*!
 * @group Voucher Activity Buffer Internals
 * SPI intended for diagnosticd only
 * Layout of structs is subject to change without notice
 */

#include <sys/queue.h>
#include <atm/atm_types.h>
#include <os/lock_private.h>

static const atm_subaid32_t _voucher_default_activity_subid =
		ATM_SUBAID32_MAX-1;

static const size_t _voucher_activity_buffer_size = 4096;
static const size_t _voucher_activity_tracepoints_per_buffer =
		_voucher_activity_buffer_size /
		sizeof(struct _voucher_activity_tracepoint_s);
typedef uint8_t _voucher_activity_buffer_t[_voucher_activity_buffer_size];

struct _voucher_activity_self_metadata_s {
	struct _voucher_activity_metadata_opaque_s *vasm_baseaddr;
};
typedef struct _voucher_activity_metadata_opaque_s {
	_voucher_activity_buffer_t vam_kernel_metadata;
	_voucher_activity_buffer_t vam_client_metadata;
	union {
		struct _voucher_activity_self_metadata_s vam_self_metadata;
		_voucher_activity_buffer_t vam_self_metadata_opaque;
	};
} *_voucher_activity_metadata_opaque_t;

typedef os_lock_handoff_s _voucher_activity_lock_s;

typedef struct _voucher_atm_s {
	int32_t volatile vatm_refcnt;
	mach_voucher_t vatm_kvoucher;
	atm_aid_t vatm_id;
	atm_mailbox_offset_t vatm_mailbox_offset;
	TAILQ_ENTRY(_voucher_atm_s) vatm_list;
#if __LP64__
	uintptr_t vatm_pad[3];
	// cacheline
#endif
	_voucher_activity_lock_s vatm_activities_lock;
	TAILQ_HEAD(_voucher_atm_activities_s, _voucher_activity_s) vatm_activities;
	TAILQ_HEAD(, _voucher_activity_s) vatm_used_activities;
} *_voucher_atm_t;

// must match layout of _voucher_activity_tracepoint_s
typedef struct _voucher_activity_buffer_header_s {
	uint16_t vabh_flags;	// _voucher_activity_trace_flag_buffer_header
	uint8_t  vabh_unused[6];
	uint64_t vabh_thread;
	uint64_t vabh_timestamp;
	uint32_t volatile vabh_next_tracepoint_idx;
	uint32_t vabh_sequence_no;
	voucher_activity_id_t vabh_activity_id;
	uint64_t vabh_reserved;
	TAILQ_ENTRY(_voucher_activity_buffer_header_s) vabh_list;
} *_voucher_activity_buffer_header_t;

// must match layout of _voucher_activity_buffer_header_s
typedef struct _voucher_activity_s {
	// first tracepoint entry
	// must match layout of _voucher_activity_tracepoint_s
	uint16_t va_flags;		// _voucher_activity_trace_flag_buffer_header |
							// _voucher_activity_trace_flag_activity |
							// _voucher_activity_trace_flag_start |
							// _voucher_activity_trace_flag_wide_first
	uint8_t  va_type;
	uint8_t  va_namespace;
	uint32_t va_code;
	uint64_t va_thread;
	uint64_t va_timestamp;
	uint32_t volatile vabh_next_tracepoint_idx;
	uint32_t volatile va_max_sequence_no;
	voucher_activity_id_t va_id;
	int32_t volatile va_use_count;
	uint32_t va_buffer_limit;
	TAILQ_HEAD(_voucher_activity_buffer_list_s,
			_voucher_activity_buffer_header_s) va_buffers;
#if !__LP64__
	uint64_t va_pad;
#endif

	// second tracepoint entry
	// must match layout of _voucher_activity_tracepoint_s
	uint16_t va_flags2;
	uint8_t va_unused2[2];
	int32_t volatile va_refcnt;
	uint64_t va_location;
	_voucher_activity_buffer_header_t volatile va_current_buffer;
	_voucher_atm_t va_atm;
	_voucher_activity_lock_s va_buffers_lock;
	uintptr_t va_pad2[2];

#if __LP64__
	// third tracepoint entry
	// must match layout of _voucher_activity_tracepoint_s
	uint16_t va_flags3;
	uint8_t va_unused3[6];
	uintptr_t va_pad3;
#endif
	TAILQ_ENTRY(_voucher_activity_s) va_list;
	TAILQ_ENTRY(_voucher_activity_s) va_atm_list;
	TAILQ_ENTRY(_voucher_activity_s) va_atm_used_list;
} *_voucher_activity_t;

#endif // OS_VOUCHER_ACTIVITY_BUFFER_SPI

__END_DECLS

#endif // __OS_VOUCHER_ACTIVITY_PRIVATE__
