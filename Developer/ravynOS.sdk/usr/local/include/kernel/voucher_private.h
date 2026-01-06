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

#ifndef __OS_VOUCHER_PRIVATE__
#define __OS_VOUCHER_PRIVATE__

#if __APPLE__
#include <os/base.h>
#include <os/availability.h>
#endif
#if __has_include(<mach/mach.h>)
#include <os/object.h>
#include <mach/mach.h>
#endif
#if __has_include(<bank/bank_types.h>)
#include <bank/bank_types.h>
#endif
#if __has_include(<sys/persona.h>)
#include <sys/persona.h>
#endif

#ifndef __DISPATCH_BUILDING_DISPATCH__
#include <dispatch/dispatch.h>
#endif /* !__DISPATCH_BUILDING_DISPATCH__ */

#define OS_VOUCHER_SPI_VERSION 20150630

#if OS_VOUCHER_WEAK_IMPORT
#define OS_VOUCHER_EXPORT OS_EXPORT OS_WEAK_IMPORT
#else
#define OS_VOUCHER_EXPORT OS_EXPORT
#endif

DISPATCH_ASSUME_NONNULL_BEGIN

__BEGIN_DECLS

/*!
 * @group Voucher Transport SPI
 * SPI intended for clients that need to transport vouchers.
 */

/*!
 * @typedef voucher_t
 *
 * @abstract
 * Vouchers are immutable sets of key/value attributes that can be adopted on a
 * thread in the current process or sent to another process.
 *
 * @discussion
 * Voucher objects are os_objects (c.f. <os/object.h>). They are memory-managed
 * with the os_retain()/os_release() functions or -[retain]/-[release] methods.
 */
OS_OBJECT_DECL_CLASS(voucher);

/*!
 * @const VOUCHER_NULL
 * Represents the empty base voucher with no attributes.
 */
#define VOUCHER_NULL		((voucher_t)0)
/*!
 * @const VOUCHER_INVALID
 * Represents an invalid voucher
 */
#define VOUCHER_INVALID		((voucher_t)-1)

/*!
 * @function voucher_adopt
 *
 * @abstract
 * Adopt the specified voucher on the current thread and return the voucher
 * that had been adopted previously.
 *
 * @discussion
 * Adopted vouchers are automatically carried forward by the system to other
 * threads and processes (across IPC).
 *
 * Consumes a reference to the specified voucher.
 * Returns a reference to the previous voucher.
 *
 * @param voucher
 * The voucher object to adopt on the current thread.
 *
 * @result
 * The previously adopted voucher object.
 */
API_AVAILABLE(macos(10.10), ios(8.0))
OS_VOUCHER_EXPORT OS_OBJECT_RETURNS_RETAINED OS_WARN_RESULT_NEEDS_RELEASE
OS_NOTHROW
voucher_t _Nullable
voucher_adopt(voucher_t _Nullable voucher OS_OBJECT_CONSUMED);

/*!
 * @function voucher_copy
 *
 * @abstract
 * Returns a reference to the voucher that had been adopted previously on the
 * current thread (or carried forward by the system).
 *
 * @result
 * The currently adopted voucher object.
 */
API_AVAILABLE(macos(10.10), ios(8.0))
OS_VOUCHER_EXPORT OS_OBJECT_RETURNS_RETAINED OS_WARN_RESULT OS_NOTHROW
voucher_t _Nullable
voucher_copy(void);

/*!
 * @function voucher_copy_without_importance
 *
 * @abstract
 * Returns a reference to a voucher object with all the properties of the
 * voucher that had been adopted previously on the current thread, but
 * without the importance properties that are frequently attached to vouchers
 * carried with IPC requests. Importance properties may elevate the scheduling
 * of threads that adopt or retain the voucher while they service the request.
 * See xpc_transaction_begin(3) for further details on importance.
 *
 * @result
 * A copy of the currently adopted voucher object, with importance removed.
 */
API_AVAILABLE(macos(10.10), ios(8.0))
OS_VOUCHER_EXPORT OS_OBJECT_RETURNS_RETAINED OS_WARN_RESULT OS_NOTHROW
voucher_t _Nullable
voucher_copy_without_importance(void);

/*!
 * @function voucher_replace_default_voucher
 *
 * @abstract
 * Replace process attributes of default voucher (used for IPC by this process
 * when no voucher is adopted on the sending thread) with the process attributes
 * of the voucher adopted on the current thread.
 *
 * @discussion
 * This allows a daemon to indicate from the context of an incoming IPC request
 * that all future outgoing IPC from the process should be marked as acting
 * "on behalf of" the sending process of the current IPC request (as long as the
 * thread sending that outgoing IPC is not itself in the direct context of an
 * IPC request, i.e. no voucher is adopted).
 *
 * If no voucher is adopted on the current thread or the current voucher does
 * not contain any process attributes, the default voucher is reset to the
 * default process attributes for the current process.
 *
 * CAUTION: Do NOT use this SPI without contacting the Darwin Runtime team.
 */
API_AVAILABLE(macos(10.10), ios(8.0))
OS_VOUCHER_EXPORT OS_NOTHROW
void
voucher_replace_default_voucher(void);

/*!
 * @function voucher_decrement_importance_count4CF
 *
 * @abstract
 * Decrement external importance count of the mach voucher in the specified
 * voucher object.
 *
 * @discussion
 * This is only intended for use by CoreFoundation to explicitly manage the
 * App Nap state of an application following reception of a de-nap IPC message.
 *
 * CAUTION: Do NOT use this SPI without contacting the Darwin Runtime team.
 */
API_AVAILABLE(macos(10.10), ios(8.0))
OS_VOUCHER_EXPORT OS_NOTHROW
void
voucher_decrement_importance_count4CF(voucher_t _Nullable voucher);

/*!
 * @group Voucher dispatch block SPI
 */

/*!
 * @typedef dispatch_block_flags_t
 * SPI Flags to pass to the dispatch_block_create* functions.
 *
 * @const DISPATCH_BLOCK_NO_VOUCHER
 * Flag indicating that a dispatch block object should not be assigned a voucher
 * object. If invoked directly, the block object will be executed with the
 * voucher adopted on the calling thread. If the block object is submitted to a
 * queue, this replaces the default behavior of associating the submitted block
 * instance with the voucher adopted at the time of submission.
 * This flag is ignored if used with the dispatch_block_create_with_voucher*()
 * functions.
 *
 */
#define DISPATCH_BLOCK_NO_VOUCHER (0x40ul)

#define DISPATCH_BLOCK_IF_LAST_RESET_QUEUE_QOS_OVERRIDE (0x80ul)

/*!
 * @function dispatch_block_create_with_voucher
 *
 * @abstract
 * Create a new dispatch block object on the heap from an existing block and
 * the given flags, and assign it the specified voucher object.
 *
 * @discussion
 * The provided block is Block_copy'ed to the heap, it and the specified voucher
 * object are retained by the newly created dispatch block object.
 *
 * The returned dispatch block object is intended to be submitted to a dispatch
 * queue with dispatch_async() and related functions, but may also be invoked
 * directly. Both operations can be performed an arbitrary number of times but
 * only the first completed execution of a dispatch block object can be waited
 * on with dispatch_block_wait() or observed with dispatch_block_notify().
 *
 * The returned dispatch block will be executed with the specified voucher
 * adopted for the duration of the block body.
 *
 * If the returned dispatch block object is submitted to a dispatch queue, the
 * submitted block instance will be associated with the QOS class current at the
 * time of submission, unless one of the following flags assigned a specific QOS
 * class (or no QOS class) at the time of block creation:
 *  - DISPATCH_BLOCK_ASSIGN_CURRENT
 *  - DISPATCH_BLOCK_NO_QOS_CLASS
 *  - DISPATCH_BLOCK_DETACHED
 * The QOS class the block object will be executed with also depends on the QOS
 * class assigned to the queue and which of the following flags was specified or
 * defaulted to:
 *  - DISPATCH_BLOCK_INHERIT_QOS_CLASS (default for asynchronous execution)
 *  - DISPATCH_BLOCK_ENFORCE_QOS_CLASS (default for synchronous execution)
 * See description of dispatch_block_flags_t for details.
 *
 * If the returned dispatch block object is submitted directly to a serial queue
 * and is configured to execute with a specific QOS class, the system will make
 * a best effort to apply the necessary QOS overrides to ensure that blocks
 * submitted earlier to the serial queue are executed at that same QOS class or
 * higher.
 *
 * @param flags
 * Configuration flags for the block object.
 * Passing a value that is not a bitwise OR of flags from dispatch_block_flags_t
 * results in NULL being returned. The DISPATCH_BLOCK_NO_VOUCHER flag is
 * ignored.
 *
 * @param voucher
 * A voucher object or NULL.
 *
 * @param block
 * The block to create the dispatch block object from.
 *
 * @result
 * The newly created dispatch block object, or NULL.
 * When not building with Objective-C ARC, must be released with a -[release]
 * message or the Block_release() function.
 */
API_AVAILABLE(macos(10.10), ios(8.0))
DISPATCH_EXPORT DISPATCH_NONNULL3 DISPATCH_RETURNS_RETAINED_BLOCK
DISPATCH_WARN_RESULT DISPATCH_NOTHROW
dispatch_block_t
dispatch_block_create_with_voucher(dispatch_block_flags_t flags,
		voucher_t _Nullable voucher, dispatch_block_t block);

/*!
 * @function dispatch_block_create_with_voucher_and_qos_class
 *
 * @abstract
 * Create a new dispatch block object on the heap from an existing block and
 * the given flags, and assign it the specified voucher object, QOS class and
 * relative priority.
 *
 * @discussion
 * The provided block is Block_copy'ed to the heap, it and the specified voucher
 * object are retained by the newly created dispatch block object.
 *
 * The returned dispatch block object is intended to be submitted to a dispatch
 * queue with dispatch_async() and related functions, but may also be invoked
 * directly. Both operations can be performed an arbitrary number of times but
 * only the first completed execution of a dispatch block object can be waited
 * on with dispatch_block_wait() or observed with dispatch_block_notify().
 *
 * The returned dispatch block will be executed with the specified voucher
 * adopted for the duration of the block body.
 *
 * If invoked directly, the returned dispatch block object will be executed with
 * the assigned QOS class as long as that does not result in a lower QOS class
 * than what is current on the calling thread.
 *
 * If the returned dispatch block object is submitted to a dispatch queue, the
 * QOS class it will be executed with depends on the QOS class assigned to the
 * block, the QOS class assigned to the queue and which of the following flags
 * was specified or defaulted to:
 *  - DISPATCH_BLOCK_INHERIT_QOS_CLASS: default for asynchronous execution
 *  - DISPATCH_BLOCK_ENFORCE_QOS_CLASS: default for synchronous execution
 * See description of dispatch_block_flags_t for details.
 *
 * If the returned dispatch block object is submitted directly to a serial queue
 * and is configured to execute with a specific QOS class, the system will make
 * a best effort to apply the necessary QOS overrides to ensure that blocks
 * submitted earlier to the serial queue are executed at that same QOS class or
 * higher.
 *
 * @param flags
 * Configuration flags for the block object.
 * Passing a value that is not a bitwise OR of flags from dispatch_block_flags_t
 * results in NULL being returned. The DISPATCH_BLOCK_NO_VOUCHER and
 * DISPATCH_BLOCK_NO_QOS flags are ignored.
 *
 * @param voucher
 * A voucher object or NULL.
 *
 * @param qos_class
 * A QOS class value:
 *  - QOS_CLASS_USER_INTERACTIVE
 *  - QOS_CLASS_USER_INITIATED
 *  - QOS_CLASS_DEFAULT
 *  - QOS_CLASS_UTILITY
 *  - QOS_CLASS_BACKGROUND
 *  - QOS_CLASS_UNSPECIFIED
 * Passing QOS_CLASS_UNSPECIFIED is equivalent to specifying the
 * DISPATCH_BLOCK_NO_QOS_CLASS flag. Passing any other value results in NULL
 * being returned.
 *
 * @param relative_priority
 * A relative priority within the QOS class. This value is a negative
 * offset from the maximum supported scheduler priority for the given class.
 * Passing a value greater than zero or less than QOS_MIN_RELATIVE_PRIORITY
 * results in NULL being returned.
 *
 * @param block
 * The block to create the dispatch block object from.
 *
 * @result
 * The newly created dispatch block object, or NULL.
 * When not building with Objective-C ARC, must be released with a -[release]
 * message or the Block_release() function.
 */
API_AVAILABLE(macos(10.10), ios(8.0))
DISPATCH_EXPORT DISPATCH_NONNULL5 DISPATCH_RETURNS_RETAINED_BLOCK
DISPATCH_WARN_RESULT DISPATCH_NOTHROW
dispatch_block_t
dispatch_block_create_with_voucher_and_qos_class(dispatch_block_flags_t flags,
		voucher_t _Nullable voucher, dispatch_qos_class_t qos_class,
		int relative_priority, dispatch_block_t block);

/*!
 * @group Voucher dispatch queue SPI
 */

/*!
 * @function dispatch_queue_create_with_accounting_override_voucher
 *
 * @abstract
 * Deprecated, do not use, will abort process if called.
 */
API_DEPRECATED("removed SPI", \
		macos(10.11,10.13), ios(9.0,11.0), watchos(2.0,4.0), tvos(9.0,11.0))
DISPATCH_EXPORT DISPATCH_MALLOC DISPATCH_RETURNS_RETAINED DISPATCH_WARN_RESULT
DISPATCH_NOTHROW
dispatch_queue_t
dispatch_queue_create_with_accounting_override_voucher(
		const char *_Nullable label,
		dispatch_queue_attr_t _Nullable attr,
		voucher_t _Nullable voucher);

#if __has_include(<mach/mach.h>)
/*!
 * @group Voucher Mach SPI
 * SPI intended for clients that need to interact with mach messages or mach
 * voucher ports directly.
 */

/*!
 * @function voucher_create_with_mach_msg
 *
 * @abstract
 * Creates a new voucher object from a mach message carrying a mach voucher port
 *
 * @discussion
 * Ownership of the mach voucher port in the message is transfered to the new
 * voucher object and the message header mach voucher field is cleared.
 *
 * @param msg
 * The mach message to query.
 *
 * @result
 * The newly created voucher object or NULL if the message was not carrying a
 * mach voucher.
 */
API_AVAILABLE(macos(10.10), ios(8.0))
OS_VOUCHER_EXPORT OS_OBJECT_RETURNS_RETAINED OS_WARN_RESULT OS_NOTHROW
voucher_t _Nullable
voucher_create_with_mach_msg(mach_msg_header_t *msg);

/*!
 * @function voucher_kvoucher_debug
 *
 * @abstract
 * Writes a human-readable representation of a voucher to a memory buffer.
 *
 * @discussion
 * The formatted representation of the voucher is written starting at a given
 * offset in the buffer. If the remaining space in the buffer is too small, the
 * output is truncated. Nothing is written before buf[offset] or at or beyond
 * buf[bufsize].
 *
 * @param task
 * The task port for the task that owns the voucher port.
 *
 * @param voucher
 * The voucher port name.
 *
 * @param buf
 * The buffer to which the formatted representation of the voucher should be
 * written.
 *
 * @param bufsiz
 * The size of the buffer.
 *
 * @param offset
 * The offset of the first byte in the buffer to be used for output.
 *
 * @param prefix
 * A string to be written at the start of each line of formatted output.
 * Typically used to generate leading whitespace for indentation. Use NULL if
 * no prefix is required.
 *
 * @param max_hex_data
 * The maximum number of bytes of hex data to be formatted for voucher content
 * that is not of type MACH_VOUCHER_ATTR_KEY_ATM, MACH_VOUCHER_ATTR_KEY_BANK
 * or MACH_VOUCHER_ATTR_KEY_IMPORTANCE.
 *
 * @result
 * The offset of the first byte in the buffer following the formatted voucher
 * representation.
 */
API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0))
OS_VOUCHER_EXPORT OS_WARN_RESULT OS_NOTHROW DISPATCH_COLD
size_t
voucher_kvoucher_debug(mach_port_t task, mach_port_name_t voucher, char *buf,
		   size_t bufsiz, size_t offset, char * _Nullable prefix,
		   size_t max_hex_data) ;

/*!
 * @group Voucher Persona SPI
 * SPI intended for clients that need to interact with personas.
 */

struct proc_persona_info;

/*!
 * @function voucher_get_current_persona
 *
 * @abstract
 * Returns the persona identifier for the current thread.
 *
 * @discussion
 * Retrieve the persona identifier from the currently adopted voucher.
 *
 * If the thread has not adopted a voucher, or the current voucher does not
 * contain persona information, this function returns the persona identifier
 * of the current process.
 *
 * If the process is not running under a persona, then this returns
 * PERSONA_ID_NONE.
 *
 * @result
 * The persona identifier for the current voucher,
 * or the persona identifier of the current process
 * or PERSONA_ID_NONE
 */
API_AVAILABLE(macos(10.14), ios(9.2))
OS_VOUCHER_EXPORT OS_WARN_RESULT OS_NOTHROW
uid_t
voucher_get_current_persona(void);

/*!
 * @function voucher_get_current_persona_originator_info
 *
 * @abstract
 * Retrieve the ’originator’ process persona info for the currently adopted
 * voucher.
 *
 * @discussion
 * If there is no currently adopted voucher, or no PERSONA_TOKEN attribute
 * in that voucher, this function fails.
 *
 * @param persona_info
 * The proc_persona_info structure to fill in case of success
 *
 * @result
 * 0 on success: currently adopted voucher has a PERSONA_TOKEN
 * -1 on failure: persona_info is untouched/uninitialized
 */
API_AVAILABLE(macos(10.14), ios(9.2))
OS_VOUCHER_EXPORT OS_WARN_RESULT OS_NOTHROW OS_NONNULL1
int
voucher_get_current_persona_originator_info(
	struct proc_persona_info *persona_info);

/*!
 * @function voucher_get_current_persona_proximate_info
 *
 * @abstract
 * Retrieve the ’proximate’ process persona info for the currently adopted
 * voucher.
 *
 * @discussion
 * If there is no currently adopted voucher, or no PERSONA_TOKEN attribute
 * in that voucher, this function fails.
 *
 * @param persona_info
 * The proc_persona_info structure to fill in case of success
 *
 * @result
 * 0 on success: currently adopted voucher has a PERSONA_TOKEN
 * -1 on failure: persona_info is untouched/uninitialized
 */
API_AVAILABLE(macos(10.14), ios(9.2))
OS_VOUCHER_EXPORT OS_WARN_RESULT OS_NOTHROW OS_NONNULL1
int
voucher_get_current_persona_proximate_info(
	struct proc_persona_info *persona_info);

/*!
 * @function voucher_copy_with_persona_mach_voucher
 *
 * @abstract
 * Creates a copy of the currently adopted voucher and replaces its
 * persona information with the one passed in the specified mach voucher
 *
 * @discussion
 * If the specified mach voucher is not one returned from
 * mach_voucher_persona_for_originator() (called on behalf
 * of the current process), this function will fail
 *
 * @param persona_mach_voucher
 * mach voucher containing the new persona information
 *
 * @result
 * On success, a copy of the current voucher with the new
 * persona information
 * On failure, VOUCHER_INVALID
 */
API_AVAILABLE(macos(10.14), ios(12))
OS_VOUCHER_EXPORT OS_OBJECT_RETURNS_RETAINED OS_WARN_RESULT OS_NOTHROW
voucher_t _Nullable
voucher_copy_with_persona_mach_voucher(
	mach_voucher_t persona_mach_voucher);

/*!
 * @function mach_voucher_persona_self
 *
 * @abstract
 * Creates a mach voucher containing the persona information of the
 * current process that can be sent as a mach port descriptor in a message
 *
 * @discussion
 * The returned mach voucher has been pre-processed so that it can be sent
 * in a message
 *
 * @param persona_mach_voucher
 * If successful, a reference to the newly created mach voucher
 *
 * @result
 * KERN_SUCCESS: a mach voucher ready to be sent in a message is
 * successfully created
 * KERN_RESOURCE_SHORTAGE: mach voucher creation failed due to
 * lack of free space
 */
API_AVAILABLE(macos(10.14), ios(12))
OS_VOUCHER_EXPORT OS_WARN_RESULT OS_NOTHROW OS_NONNULL1
kern_return_t
mach_voucher_persona_self(mach_voucher_t *persona_mach_voucher);

/*!
 * @function mach_voucher_persona_for_originator
 *
 * @abstract
 * Creates a mach voucher on behalf of the originator process by copying
 * the persona information from the specified mach voucher and then
 * updating the persona identifier to the specified value
 *
 * @discussion
 * Should be called by a privileged process on behalf of the originator process.
 * The newly created mach voucher should be returned to the originator in a
 * message. The originator's thread can adopt the new persona by passing
 * this mach voucher to voucher_copy_with_persona_mach_voucher().
 *
 * @param persona_id
 * The new persona identifier to be set in the mach voucher
 *
 * @param originator_persona_mach_voucher
 * A mach voucher received from the originator, where it was created using
 * mach_voucher_persona_self()
 *
 * @param originator_unique_pid
 * Unique pid of the originator process
 *
 * @param persona_mach_voucher
 * If successful, a reference to the newly created mach voucher
 *
 * @result
 * KERN_SUCCESS: a mach voucher ready to be returned to the
 * originator was successfully created
 * KERN_NO_ACCESS: process does not have privilege to carry
 * out this operation
 * KERN_INVALID_ARGUMENT: specified persona identifier is invalid
 * KERN_INVALID_CAPABILITY: originator_unique_pid does not
 * match the specified voucher originator's unique pid
 * KERN_RESOURCE_SHORTAGE: mach voucher creation failed due to
 * lack of free space
 */
API_AVAILABLE(macos(10.14), ios(12))
OS_VOUCHER_EXPORT OS_WARN_RESULT OS_NOTHROW OS_NONNULL4
kern_return_t
mach_voucher_persona_for_originator(uid_t persona_id,
	mach_voucher_t originator_persona_mach_voucher,
	uint64_t originator_unique_pid, mach_voucher_t *persona_mach_voucher);

#endif // __has_include(<mach/mach.h>)

__END_DECLS

DISPATCH_ASSUME_NONNULL_END

#endif // __OS_VOUCHER_PRIVATE__

#if OS_VOUCHER_ACTIVITY_SPI
#include "voucher_activity_private.h"
#endif
