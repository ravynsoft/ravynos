/*
 * Copyright (c) 2009-2013 Apple Inc. All rights reserved.
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

#ifndef __DISPATCH_IO_PRIVATE__
#define __DISPATCH_IO_PRIVATE__

#ifndef __DISPATCH_INDIRECT__
#error "Please #include <dispatch/dispatch.h> instead of this file directly."
#include <dispatch/base.h> // for HeaderDoc
#endif

DISPATCH_ASSUME_NONNULL_BEGIN

__BEGIN_DECLS

/*!
 * @function dispatch_read_f
 * Schedule a read operation for asynchronous execution on the specified file
 * descriptor. The specified handler is enqueued with the data read from the
 * file descriptor when the operation has completed or an error occurs.
 *
 * The data object passed to the handler will be automatically released by the
 * system when the handler returns. It is the responsibility of the application
 * to retain, concatenate or copy the data object if it is needed after the
 * handler returns.
 *
 * The data object passed to the handler will only contain as much data as is
 * currently available from the file descriptor (up to the specified length).
 *
 * If an unrecoverable error occurs on the file descriptor, the handler will be
 * enqueued with the appropriate error code along with a data object of any data
 * that could be read successfully.
 *
 * An invocation of the handler with an error code of zero and an empty data
 * object indicates that EOF was reached.
 *
 * The system takes control of the file descriptor until the handler is
 * enqueued, and during this time file descriptor flags such as O_NONBLOCK will
 * be modified by the system on behalf of the application. It is an error for
 * the application to modify a file descriptor directly while it is under the
 * control of the system, but it may create additional dispatch I/O convenience
 * operations or dispatch I/O channels associated with that file descriptor.
 *
 * @param fd		The file descriptor from which to read the data.
 * @param length	The length of data to read from the file descriptor,
 *			or SIZE_MAX to indicate that all of the data currently
 *			available from the file descriptor should be read.
 * @param queue		The dispatch queue to which the handler should be
 *			submitted.
 * @param context	The application-defined context parameter to pass to
 *			the handler function.
 * @param handler	The handler to enqueue when data is ready to be
 *			delivered.
 *		param context	Application-defined context parameter.
 *		param data	The data read from the file descriptor.
 *		param error	An errno condition for the read operation or
 *				zero if the read was successful.
 */
API_AVAILABLE(macos(10.9), ios(7.0))
DISPATCH_EXPORT DISPATCH_NONNULL3 DISPATCH_NONNULL5 DISPATCH_NOTHROW
void
dispatch_read_f(dispatch_fd_t fd,
	size_t length,
	dispatch_queue_t queue,
	void *_Nullable context,
	void (*handler)(void *_Nullable context, dispatch_data_t data, int error));

/*!
 * @function dispatch_write_f
 * Schedule a write operation for asynchronous execution on the specified file
 * descriptor. The specified handler is enqueued when the operation has
 * completed or an error occurs.
 *
 * If an unrecoverable error occurs on the file descriptor, the handler will be
 * enqueued with the appropriate error code along with the data that could not
 * be successfully written.
 *
 * An invocation of the handler with an error code of zero indicates that the
 * data was fully written to the channel.
 *
 * The system takes control of the file descriptor until the handler is
 * enqueued, and during this time file descriptor flags such as O_NONBLOCK will
 * be modified by the system on behalf of the application. It is an error for
 * the application to modify a file descriptor directly while it is under the
 * control of the system, but it may create additional dispatch I/O convenience
 * operations or dispatch I/O channels associated with that file descriptor.
 *
 * @param fd		The file descriptor to which to write the data.
 * @param data		The data object to write to the file descriptor.
 * @param queue		The dispatch queue to which the handler should be
 *			submitted.
 * @param context	The application-defined context parameter to pass to
 *			the handler function.
 * @param handler	The handler to enqueue when the data has been written.
 *		param context	Application-defined context parameter.
 *		param data	The data that could not be written to the I/O
 *				channel, or NULL.
 *		param error	An errno condition for the write operation or
 *				zero if the write was successful.
 */
API_AVAILABLE(macos(10.9), ios(7.0))
DISPATCH_EXPORT DISPATCH_NONNULL2 DISPATCH_NONNULL3 DISPATCH_NONNULL5
DISPATCH_NOTHROW
void
dispatch_write_f(dispatch_fd_t fd,
	dispatch_data_t data,
	dispatch_queue_t queue,
	void *_Nullable context,
	void (*handler)(void *_Nullable context, dispatch_data_t _Nullable data,
			int error));

/*!
 * @function dispatch_io_create_f
 * Create a dispatch I/O channel associated with a file descriptor. The system
 * takes control of the file descriptor until the channel is closed, an error
 * occurs on the file descriptor or all references to the channel are released.
 * At that time the specified cleanup handler will be enqueued and control over
 * the file descriptor relinquished.
 *
 * While a file descriptor is under the control of a dispatch I/O channel, file
 * descriptor flags such as O_NONBLOCK will be modified by the system on behalf
 * of the application. It is an error for the application to modify a file
 * descriptor directly while it is under the control of a dispatch I/O channel,
 * but it may create additional channels associated with that file descriptor.
 *
 * @param type	The desired type of I/O channel (DISPATCH_IO_STREAM
 *		or DISPATCH_IO_RANDOM).
 * @param fd	The file descriptor to associate with the I/O channel.
 * @param queue	The dispatch queue to which the handler should be submitted.
 * @param context	The application-defined context parameter to pass to
 *			the cleanup handler function.
 * @param cleanup_handler	The handler to enqueue when the system
 *				relinquishes control over the file descriptor.
 *	param context		Application-defined context parameter.
 *	param error		An errno condition if control is relinquished
 *				because channel creation failed, zero otherwise.
 * @result	The newly created dispatch I/O channel or NULL if an error
 *		occurred (invalid type specified).
 */
API_AVAILABLE(macos(10.9), ios(7.0))
DISPATCH_EXPORT DISPATCH_MALLOC DISPATCH_RETURNS_RETAINED DISPATCH_WARN_RESULT
DISPATCH_NOTHROW
dispatch_io_t
dispatch_io_create_f(dispatch_io_type_t type,
	dispatch_fd_t fd,
	dispatch_queue_t queue,
	void *_Nullable context,
	void (*cleanup_handler)(void *_Nullable context, int error));

/*!
 * @function dispatch_io_create_with_path_f
 * Create a dispatch I/O channel associated with a path name. The specified
 * path, oflag and mode parameters will be passed to open(2) when the first I/O
 * operation on the channel is ready to execute and the resulting file
 * descriptor will remain open and under the control of the system until the
 * channel is closed, an error occurs on the file descriptor or all references
 * to the channel are released. At that time the file descriptor will be closed
 * and the specified cleanup handler will be enqueued.
 *
 * @param type	The desired type of I/O channel (DISPATCH_IO_STREAM
 *		or DISPATCH_IO_RANDOM).
 * @param path	The absolute path to associate with the I/O channel.
 * @param oflag	The flags to pass to open(2) when opening the file at
 *		path.
 * @param mode	The mode to pass to open(2) when creating the file at
 *		path (i.e. with flag O_CREAT), zero otherwise.
 * @param queue	The dispatch queue to which the handler should be
 *		submitted.
 * @param context	The application-defined context parameter to pass to
 *			the cleanup handler function.
 * @param cleanup_handler	The handler to enqueue when the system
 *				has closed the file at path.
 *	param context		Application-defined context parameter.
 *	param error		An errno condition if control is relinquished
 *				because channel creation or opening of the
 *				specified file failed, zero otherwise.
 * @result	The newly created dispatch I/O channel or NULL if an error
 *		occurred (invalid type or non-absolute path specified).
 */
API_AVAILABLE(macos(10.9), ios(7.0))
DISPATCH_EXPORT DISPATCH_NONNULL2 DISPATCH_MALLOC DISPATCH_RETURNS_RETAINED
DISPATCH_WARN_RESULT DISPATCH_NOTHROW
dispatch_io_t
dispatch_io_create_with_path_f(dispatch_io_type_t type,
	const char *path, int oflag, mode_t mode,
	dispatch_queue_t queue,
	void *_Nullable context,
	void (*cleanup_handler)(void *_Nullable context, int error));

/*!
 * @function dispatch_io_create_with_io_f
 * Create a new dispatch I/O channel from an existing dispatch I/O channel.
 * The new channel inherits the file descriptor or path name associated with
 * the existing channel, but not its channel type or policies.
 *
 * If the existing channel is associated with a file descriptor, control by the
 * system over that file descriptor is extended until the new channel is also
 * closed, an error occurs on the file descriptor, or all references to both
 * channels are released. At that time the specified cleanup handler will be
 * enqueued and control over the file descriptor relinquished.
 *
 * While a file descriptor is under the control of a dispatch I/O channel, file
 * descriptor flags such as O_NONBLOCK will be modified by the system on behalf
 * of the application. It is an error for the application to modify a file
 * descriptor directly while it is under the control of a dispatch I/O channel,
 * but it may create additional channels associated with that file descriptor.
 *
 * @param type	The desired type of I/O channel (DISPATCH_IO_STREAM
 *		or DISPATCH_IO_RANDOM).
 * @param io	The existing channel to create the new I/O channel from.
 * @param queue	The dispatch queue to which the handler should be submitted.
 * @param context	The application-defined context parameter to pass to
 *			the cleanup handler function.
 * @param cleanup_handler	The handler to enqueue when the system
 *				relinquishes control over the file descriptor
 *				(resp. closes the file at path) associated with
 *				the existing channel.
 *	param context		Application-defined context parameter.
 *	param error		An errno condition if control is relinquished
 *				because channel creation failed, zero otherwise.
 * @result	The newly created dispatch I/O channel or NULL if an error
 *		occurred (invalid type specified).
 */
API_AVAILABLE(macos(10.9), ios(7.0))
DISPATCH_EXPORT DISPATCH_NONNULL2 DISPATCH_MALLOC DISPATCH_RETURNS_RETAINED
DISPATCH_WARN_RESULT DISPATCH_NOTHROW
dispatch_io_t
dispatch_io_create_with_io_f(dispatch_io_type_t type,
	dispatch_io_t io,
	dispatch_queue_t queue,
	void *_Nullable context,
	void (*cleanup_handler)(void *_Nullable context, int error));

/*!
 * @typedef dispatch_io_handler_function_t
 * The prototype of I/O handler functions for dispatch I/O operations.
 *
 * @param context	Application-defined context parameter.
 * @param done		A flag indicating whether the operation is complete.
 * @param data		The data object to be handled.
 * @param error		An errno condition for the operation.
 */
typedef void (*dispatch_io_handler_function_t)(void *_Nullable context,
	bool done, dispatch_data_t _Nullable data, int error);

/*!
 * @function dispatch_io_read_f
 * Schedule a read operation for asynchronous execution on the specified I/O
 * channel. The I/O handler is enqueued one or more times depending on the
 * general load of the system and the policy specified on the I/O channel.
 *
 * Any data read from the channel is described by the dispatch data object
 * passed to the I/O handler. This object will be automatically released by the
 * system when the I/O handler returns. It is the responsibility of the
 * application to retain, concatenate or copy the data object if it is needed
 * after the I/O handler returns.
 *
 * Dispatch I/O handlers are not reentrant. The system will ensure that no new
 * I/O handler instance is invoked until the previously enqueued handler
 * function has returned.
 *
 * An invocation of the I/O handler with the done flag set indicates that the
 * read operation is complete and that the handler will not be enqueued again.
 *
 * If an unrecoverable error occurs on the I/O channel's underlying file
 * descriptor, the I/O handler will be enqueued with the done flag set, the
 * appropriate error code and a NULL data object.
 *
 * An invocation of the I/O handler with the done flag set, an error code of
 * zero and an empty data object indicates that EOF was reached.
 *
 * @param channel	The dispatch I/O channel from which to read the data.
 * @param offset	The offset relative to the channel position from which
 *			to start reading (only for DISPATCH_IO_RANDOM).
 * @param length	The length of data to read from the I/O channel, or
 *			SIZE_MAX to indicate that data should be read until EOF
 *			is reached.
 * @param queue		The dispatch queue to which the I/O handler should be
 *			submitted.
 * @param context	The application-defined context parameter to pass to
 *			the handler function.
 * @param io_handler	The I/O handler to enqueue when data is ready to be
 *			delivered.
 *	param context	Application-defined context parameter.
 *	param done	A flag indicating whether the operation is complete.
 *	param data	An object with the data most recently read from the
 *			I/O channel as part of this read operation, or NULL.
 *	param error	An errno condition for the read operation or zero if
 *			the read was successful.
 */
API_AVAILABLE(macos(10.9), ios(7.0))
DISPATCH_EXPORT DISPATCH_NONNULL1 DISPATCH_NONNULL4 DISPATCH_NONNULL6
DISPATCH_NOTHROW
void
dispatch_io_read_f(dispatch_io_t channel,
	off_t offset,
	size_t length,
	dispatch_queue_t queue,
	void *_Nullable context,
	dispatch_io_handler_function_t io_handler);

/*!
 * @function dispatch_io_write_f
 * Schedule a write operation for asynchronous execution on the specified I/O
 * channel. The I/O handler is enqueued one or more times depending on the
 * general load of the system and the policy specified on the I/O channel.
 *
 * Any data remaining to be written to the I/O channel is described by the
 * dispatch data object passed to the I/O handler. This object will be
 * automatically released by the system when the I/O handler returns. It is the
 * responsibility of the application to retain, concatenate or copy the data
 * object if it is needed after the I/O handler returns.
 *
 * Dispatch I/O handlers are not reentrant. The system will ensure that no new
 * I/O handler instance is invoked until the previously enqueued handler
 * function has returned.
 *
 * An invocation of the I/O handler with the done flag set indicates that the
 * write operation is complete and that the handler will not be enqueued again.
 *
 * If an unrecoverable error occurs on the I/O channel's underlying file
 * descriptor, the I/O handler will be enqueued with the done flag set, the
 * appropriate error code and an object containing the data that could not be
 * written.
 *
 * An invocation of the I/O handler with the done flag set and an error code of
 * zero indicates that the data was fully written to the channel.
 *
 * @param channel	The dispatch I/O channel on which to write the data.
 * @param offset	The offset relative to the channel position from which
 *			to start writing (only for DISPATCH_IO_RANDOM).
 * @param data		The data to write to the I/O channel. The data object
 *			will be retained by the system until the write operation
 *			is complete.
 * @param queue		The dispatch queue to which the I/O handler should be
 *			submitted.
 * @param context	The application-defined context parameter to pass to
 *			the handler function.
 * @param io_handler	The I/O handler to enqueue when data has been delivered.
 *	param context	Application-defined context parameter.
 *	param done	A flag indicating whether the operation is complete.
 *	param data	An object of the data remaining to be
 *			written to the I/O channel as part of this write
 *			operation, or NULL.
 *	param error	An errno condition for the write operation or zero
 *			if the write was successful.
 */
API_AVAILABLE(macos(10.9), ios(7.0))
DISPATCH_EXPORT DISPATCH_NONNULL1 DISPATCH_NONNULL3 DISPATCH_NONNULL4
DISPATCH_NONNULL6 DISPATCH_NOTHROW
void
dispatch_io_write_f(dispatch_io_t channel,
	off_t offset,
	dispatch_data_t data,
	dispatch_queue_t queue,
	void *_Nullable context,
	dispatch_io_handler_function_t io_handler);

/*!
 * @function dispatch_io_barrier_f
 * Schedule a barrier operation on the specified I/O channel; all previously
 * scheduled operations on the channel will complete before the provided
 * barrier function is enqueued onto the global queue determined by the
 * channel's target queue, and no subsequently scheduled operations will start
 * until the barrier function has returned.
 *
 * If multiple channels are associated with the same file descriptor, a barrier
 * operation scheduled on any of these channels will act as a barrier across all
 * channels in question, i.e. all previously scheduled operations on any of the
 * channels will complete before the barrier function is enqueued, and no
 * operations subsequently scheduled on any of the channels will start until the
 * barrier function has returned.
 *
 * While the barrier function is running, it may safely operate on the channel's
 * underlying file descriptor with fsync(2), lseek(2) etc. (but not close(2)).
 *
 * @param channel	The dispatch I/O channel to schedule the barrier on.
 * @param context	The application-defined context parameter to pass to
 *			the barrier function.
 * @param barrier	The barrier function.
 */
API_AVAILABLE(macos(10.9), ios(7.0))
DISPATCH_EXPORT DISPATCH_NONNULL1 DISPATCH_NONNULL3 DISPATCH_NOTHROW
void
dispatch_io_barrier_f(dispatch_io_t channel,
	void *_Nullable context,
	dispatch_function_t barrier);

__END_DECLS

DISPATCH_ASSUME_NONNULL_END

#endif /* __DISPATCH_IO_PRIVATE__ */
