/*
 * Copyright (c) 2012-2013 Apple Inc. All rights reserved.
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

#ifndef __DISPATCH_MACH_PRIVATE__
#define __DISPATCH_MACH_PRIVATE__

#ifndef __DISPATCH_INDIRECT__
#error "Please #include <dispatch/dispatch.h> instead of this file directly."
#include <dispatch/base.h> // for HeaderDoc
#endif

__BEGIN_DECLS

#if DISPATCH_MACH_SPI

#include <mach/mach.h>

/*!
 * @functiongroup Dispatch Mach Channel SPI
 *
 * IMPORTANT: This is Libsystem-internal SPI not intended for general use and
 * is subject to change at any time without warning.
 */

/*!
 * @typedef dispatch_mach_t
 * A dispatch mach channel asynchronously recevives and sends mach messages.
 */
DISPATCH_DECL(dispatch_mach);

/*!
 * @typedef dispatch_mach_reason_t
 * Reasons for a mach channel handler to be invoked.
 *
 * @const DISPATCH_MACH_CONNECTED
 * The channel has been connected. The first handler invocation on a channel
 * after calling dispatch_mach_connect() will have this reason.
 *
 * @const DISPATCH_MACH_MESSAGE_RECEIVED
 * A message was received, it is passed in the message parameter.
 *
 * @const DISPATCH_MACH_MESSAGE_SENT
 * A message was sent, it is passed in the message parameter (so that associated
 * resources can be disposed of).
 *
 * @const DISPATCH_MACH_MESSAGE_SEND_FAILED
 * A message failed to be sent, it is passed in the message parameter (so that
 * associated resources can be disposed of), along with the error code from
 * mach_msg().
 *
 * @const DISPATCH_MACH_MESSAGE_NOT_SENT
 * A message was not sent due to the channel being canceled or reconnected, it
 * is passed in the message parameter (so that associated resources can be
 * disposed of).
 *
 * @const DISPATCH_MACH_BARRIER_COMPLETED
 * A barrier block has finished executing.
 *
 * @const DISPATCH_MACH_DISCONNECTED
 * The channel has been disconnected by a call to dispatch_mach_reconnect() or
 * dispatch_mach_cancel(), an empty message is passed in the message parameter
 * (so that associated port rights can be disposed of).
 * The message header will contain either a remote port with a previously
 * connected send right, or a local port with a previously connected receive
 * right (if the channel was canceled), or a local port with a receive right
 * that was being monitored for a direct reply to a message previously sent to
 * the channel (if no reply was received).
 *
 * @const DISPATCH_MACH_CANCELED
 * The channel has been canceled.
 */
DISPATCH_ENUM(dispatch_mach_reason, unsigned long,
	DISPATCH_MACH_CONNECTED = 1,
	DISPATCH_MACH_MESSAGE_RECEIVED,
	DISPATCH_MACH_MESSAGE_SENT,
	DISPATCH_MACH_MESSAGE_SEND_FAILED,
	DISPATCH_MACH_MESSAGE_NOT_SENT,
	DISPATCH_MACH_BARRIER_COMPLETED,
	DISPATCH_MACH_DISCONNECTED,
	DISPATCH_MACH_CANCELED,
	DISPATCH_MACH_REASON_LAST, /* unused */
);

/*!
 * @typedef dispatch_mach_trailer_t
 * Trailer type of mach message received by dispatch mach channels
 */

typedef mach_msg_context_trailer_t dispatch_mach_trailer_t;

/*!
 * @constant DISPATCH_MACH_RECEIVE_MAX_INLINE_MESSAGE_SIZE
 * Maximum size of a message that can be received inline by a dispatch mach
 * channel, reception of larger messages requires an extra roundtrip through
 * the kernel.
 */

#define DISPATCH_MACH_RECEIVE_MAX_INLINE_MESSAGE_SIZE \
		(0x4000 - sizeof(dispatch_mach_trailer_t))

/*!
 * @typedef dispatch_mach_msg_t
 * A dispatch mach message encapsulates messages received or sent with dispatch
 * mach channels.
 */
DISPATCH_DECL(dispatch_mach_msg);

/*!
 * @typedef dispatch_mach_msg_destructor_t
 * Dispatch mach message object destructors.
 *
 * @const DISPATCH_MACH_MSG_DESTRUCTOR_DEFAULT
 * Message buffer storage is internal to the object, if a buffer is supplied
 * during object creation, its contents are copied.
 *
 * @const DISPATCH_MACH_MSG_DESTRUCTOR_FREE
 * Message buffer will be deallocated with free(3).
 *
 * @const DISPATCH_MACH_MSG_DESTRUCTOR_FREE
 * Message buffer will be deallocated with vm_deallocate.
 */
DISPATCH_ENUM(dispatch_mach_msg_destructor, unsigned int,
	DISPATCH_MACH_MSG_DESTRUCTOR_DEFAULT = 0,
	DISPATCH_MACH_MSG_DESTRUCTOR_FREE,
	DISPATCH_MACH_MSG_DESTRUCTOR_VM_DEALLOCATE,
);

/*!
 * @function dispatch_mach_msg_create
 * Creates a dispatch mach message object, either with a newly allocated message
 * buffer of given size, or from an existing message buffer that will be
 * deallocated with the specified destructor when the object is released.
 *
 * If a non-NULL reference to a pointer is provided in 'msg_ptr', it is filled
 * with the location of the (possibly newly allocated) message buffer.
 *
 * It is the responsibility of the application to ensure that it does not modify
 * the underlying message buffer once the dispatch mach message object is passed
 * to other dispatch mach API.
 *
 * @param msg			The message buffer to create the message object from.
 *						If 'destructor' is DISPATCH_MACH_MSG_DESTRUCTOR_DEFAULT,
 *						this argument may be NULL to leave the newly allocated
 *						message buffer zero-initialized.
 * @param size			The size of the message buffer.
 *						Must be >= sizeof(mach_msg_header_t)
 * @param destructor	The destructor to use to deallocate the message buffer
 *						when the object is released.
 * @param msg_ptr		A pointer to a pointer variable to be filled with the
 *						location of the (possibly newly allocated) message
 *						buffer, or NULL.
 * @result				A newly created dispatch mach message object.
 */
__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0)
DISPATCH_EXPORT DISPATCH_MALLOC DISPATCH_RETURNS_RETAINED DISPATCH_WARN_RESULT
DISPATCH_NOTHROW
dispatch_mach_msg_t
dispatch_mach_msg_create(mach_msg_header_t *msg, size_t size,
		dispatch_mach_msg_destructor_t destructor, mach_msg_header_t **msg_ptr);

/*!
 * @function dispatch_mach_msg_get_msg
 * Returns the message buffer underlying a dispatch mach message object.
 *
 * @param message	The dispatch mach message object to query.
 * @param size_ptr	A pointer to a size_t variable to be filled with the
 *					size of the message buffer, or NULL.
 * @result			Pointer to message buffer underlying the object.
 */
__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0)
DISPATCH_EXPORT DISPATCH_NONNULL1 DISPATCH_NOTHROW
mach_msg_header_t*
dispatch_mach_msg_get_msg(dispatch_mach_msg_t message, size_t *size_ptr);

#ifdef __BLOCKS__
/*!
 * @typedef dispatch_mach_handler_t
 * Prototype of dispatch mach channel handler blocks.
 *
 * @param reason	Reason the handler was invoked.
 * @param message	Message object that was sent or received.
 * @param error		Mach error code for the send operation.
 */
typedef void (^dispatch_mach_handler_t)(dispatch_mach_reason_t reason,
		dispatch_mach_msg_t message, mach_error_t error);

/*!
 * @function dispatch_mach_create
 * Create a dispatch mach channel to asynchronously receive and send mach
 * messages.
 *
 * The specified handler will be called with the corresponding reason parameter
 * for each message received and for each message that was successfully sent,
 * that failed to be sent, or was not sent; as well as when a barrier block
 * has completed, or when channel connection, reconnection or cancellation has
 * taken effect.
 *
 * Dispatch mach channels are created in a disconnected state, they must be
 * connected via dispatch_mach_connect() to begin receiving and sending
 * messages.
 *
 * @param label
 * An optional string label to attach to the channel. The string is not copied,
 * if it is non-NULL it must point to storage that remains valid for the
 * lifetime of the channel object. May be NULL.
 *
 * @param queue
 * The target queue of the channel, where the handler and barrier blocks will
 * be submitted.
 *
 * @param handler
 * The handler block to submit when a message has been sent or received.
 *
 * @result
 * The newly created dispatch mach channel.
 */
__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_6_0)
DISPATCH_EXPORT DISPATCH_MALLOC DISPATCH_RETURNS_RETAINED DISPATCH_WARN_RESULT
DISPATCH_NONNULL3 DISPATCH_NOTHROW
dispatch_mach_t
dispatch_mach_create(const char *label, dispatch_queue_t queue,
		dispatch_mach_handler_t handler);
#endif

/*!
 * @typedef dispatch_mach_handler_function_t
 * Prototype of dispatch mach channel handler functions.
 *
 * @param context	Application-defined context parameter.
 * @param reason	Reason the handler was invoked.
 * @param message	Message object that was sent or received.
 * @param error		Mach error code for the send operation.
 */
typedef void (*dispatch_mach_handler_function_t)(void *context,
		dispatch_mach_reason_t reason, dispatch_mach_msg_t message,
		mach_error_t error);

/*!
 * @function dispatch_mach_create_f
 * Create a dispatch mach channel to asynchronously receive and send mach
 * messages.
 *
 * The specified handler will be called with the corresponding reason parameter
 * for each message received and for each message that was successfully sent,
 * that failed to be sent, or was not sent; as well as when a barrier block
 * has completed, or when channel connection, reconnection or cancellation has
 * taken effect.
 *
 * Dispatch mach channels are created in a disconnected state, they must be
 * connected via dispatch_mach_connect() to begin receiving and sending
 * messages.
 *
 * @param label
 * An optional string label to attach to the channel. The string is not copied,
 * if it is non-NULL it must point to storage that remains valid for the
 * lifetime of the channel object. May be NULL.
 *
 * @param queue
 * The target queue of the channel, where the handler and barrier blocks will
 * be submitted.
 *
 * @param context
 * The application-defined context to pass to the handler.
 *
 * @param handler
 * The handler function to submit when a message has been sent or received.
 *
 * @result
 * The newly created dispatch mach channel.
 */
__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_6_0)
DISPATCH_EXPORT DISPATCH_MALLOC DISPATCH_RETURNS_RETAINED DISPATCH_WARN_RESULT
DISPATCH_NONNULL4 DISPATCH_NOTHROW
dispatch_mach_t
dispatch_mach_create_f(const char *label, dispatch_queue_t queue, void *context,
		dispatch_mach_handler_function_t handler);

/*!
 * @function dispatch_mach_connect
 * Connect a mach channel to the specified receive and send rights.
 *
 * This function must only be called once during the lifetime of a channel, it
 * will initiate message reception and perform any already submitted message
 * sends or barrier operations.
 *
 * @param channel
 * The mach channel to connect.
 *
 * @param receive
 * The receive right to associate with the channel. May be MACH_PORT_NULL.
 *
 * @param send
 * The send right to associate with the channel. May be MACH_PORT_NULL.
 *
 * @param checkin
 * An optional message object encapsulating the initial check-in message to send
 * upon channel connection. The check-in message is sent immediately before the
 * first message submitted via dispatch_mach_send(). The message object will be
 * retained until the initial send operation is complete (or not peformed due
 * to channel cancellation or reconnection) and the channel handler has
 * returned. May be NULL.
 */
__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_6_0)
DISPATCH_EXPORT DISPATCH_NONNULL1 DISPATCH_NOTHROW
void
dispatch_mach_connect(dispatch_mach_t channel, mach_port_t receive,
		mach_port_t send, dispatch_mach_msg_t checkin);

/*!
 * @function dispatch_mach_reconnect
 * Reconnect a mach channel to the specified send right.
 *
 * Disconnects the channel from the current send right, interrupts any pending
 * message sends (and returns the messages as unsent), and reconnects the
 * channel to a new send right.
 *
 * The application must wait for the channel handler to be invoked with
 * DISPATCH_MACH_DISCONNECTED before releasing the previous send right.
 *
 * @param channel
 * The mach channel to reconnect.
 *
 * @param send
 * The new send right to associate with the channel. May be MACH_PORT_NULL.
 *
 * @param checkin
 * An optional message object encapsulating the initial check-in message to send
 * upon channel reconnection. The check-in message is sent immediately before
 * the first message submitted via dispatch_mach_send() after this function
 * returns. The message object will be retained until the initial send operation
 * is complete (or not peformed due to channel cancellation or reconnection)
 * and the channel handler has returned. May be NULL.
 */
__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_6_0)
DISPATCH_EXPORT DISPATCH_NONNULL1 DISPATCH_NOTHROW
void
dispatch_mach_reconnect(dispatch_mach_t channel, mach_port_t send,
		dispatch_mach_msg_t checkin);

/*!
 * @function dispatch_mach_cancel
 * Cancel a mach channel, preventing any further messages from being sent or
 * received.
 *
 * The application must wait for the channel handler to be invoked with
 * DISPATCH_MACH_DISCONNECTED before releasing the underlying send and receive
 * rights.
 *
 * Note: explicit cancellation of mach channels is required, no implicit
 *       cancellation takes place on release of the last application reference
 *       to the channel object. Failure to cancel will cause the channel and
 *       its associated resources to be leaked.
 *
 * @param channel
 * The mach channel to cancel.
 */
__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_6_0)
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_NOTHROW
void
dispatch_mach_cancel(dispatch_mach_t channel);

/*!
 * @function dispatch_mach_send
 * Asynchronously send a message encapsulated in a dispatch mach message object
 * to the specified mach channel.
 *
 * Unless the message is being sent to a send-once right (as determined by the
 * presence of MACH_MSG_TYPE_MOVE_SEND_ONCE in the message header remote bits),
 * the message header remote port is set to the channel send right before the
 * send operation is performed.
 *
 * If the message expects a direct reply (as determined by the presence of
 * MACH_MSG_TYPE_MAKE_SEND_ONCE in the message header local bits) the receive
 * right specified in the message header local port will be monitored until a
 * reply message (or a send-once notification) is received, or the channel is
 * canceled. Hence the application must wait for the channel handler to be
 * invoked with a DISPATCH_MACH_DISCONNECTED message before releasing that
 * receive right.
 *
 * If the message send operation is attempted but the channel is canceled
 * before the send operation succesfully completes, the message returned to the
 * channel handler with DISPATCH_MACH_MESSAGE_NOT_SENT may be the result of a
 * pseudo-receive operation. If the message expected a direct reply, the
 * receive right originally specified in the message header local port will
 * returned in a DISPATCH_MACH_DISCONNECTED message.
 *
 * @param channel
 * The mach channel to which to send the message.
 *
 * @param message
 * The message object encapsulating the message to send. The object will be
 * retained until the send operation is complete and the channel handler has
 * returned. The storage underlying the message object may be modified by the
 * send operation.
 *
 * @param options
 * Additional send options to pass to mach_msg() when performing the send
 * operation.
 */
__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_6_0)
DISPATCH_EXPORT DISPATCH_NONNULL1 DISPATCH_NONNULL2 DISPATCH_NOTHROW
void
dispatch_mach_send(dispatch_mach_t channel, dispatch_mach_msg_t message,
		mach_msg_option_t options);

#ifdef __BLOCKS__
/*!
 * @function dispatch_mach_send_barrier
 * Submit a send barrier to the specified mach channel. Messages submitted to
 * the channel before the barrier will be sent before the barrier block is
 * executed, and messages submitted to the channel after the barrier will only
 * be sent once the barrier block has completed and the channel handler
 * invocation for the barrier has returned.
 *
 * @param channel
 * The mach channel to which to submit the barrier.
 *
 * @param barrier
 * The barrier block to submit to the channel target queue.
 */
__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_6_0)
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_NOTHROW
void
dispatch_mach_send_barrier(dispatch_mach_t channel, dispatch_block_t barrier);
#endif

/*!
 * @function dispatch_mach_send_barrier_f
 * Submit a send barrier to the specified mach channel. Messages submitted to
 * the channel before the barrier will be sent before the barrier block is
 * executed, and messages submitted to the channel after the barrier will only
 * be sent once the barrier block has completed and the channel handler
 * invocation for the barrier has returned.
 *
 * @param channel
 * The mach channel to which to submit the barrier.
 *
 * @param context
 * The application-defined context parameter to pass to the function.
 *
 * @param barrier
 * The barrier function to submit to the channel target queue.
 */
__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_6_0)
DISPATCH_EXPORT DISPATCH_NONNULL1 DISPATCH_NONNULL3 DISPATCH_NOTHROW
void
dispatch_mach_send_barrier_f(dispatch_mach_t channel, void *context,
		dispatch_function_t barrier);

#ifdef __BLOCKS__
/*!
 * @function dispatch_mach_receive_barrier
 * Submit a receive barrier to the specified mach channel. Channel handlers for
 * messages received by the channel after the receive barrier has been
 * submitted will only be invoked once the barrier block has completed and the
 * channel handler invocation for the barrier has returned.
 *
 * @param channel
 * The mach channel to which to submit the receive barrier.
 *
 * @param barrier
 * The barrier block to submit to the channel target queue.
 */
__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_6_0)
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_NOTHROW
void
dispatch_mach_receive_barrier(dispatch_mach_t channel,
		dispatch_block_t barrier);
#endif

/*!
 * @function dispatch_mach_receive_barrier_f
 * Submit a receive barrier to the specified mach channel. Channel handlers for
 * messages received by the channel after the receive barrier has been
 * submitted will only be invoked once the barrier block has completed and the
 * channel handler invocation for the barrier has returned.
 *
 * @param channel
 * The mach channel to which to submit the receive barrier.
 *
 * @param context
 * The application-defined context parameter to pass to the function.
 *
 * @param barrier
 * The barrier function to submit to the channel target queue.
 */
__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_6_0)
DISPATCH_EXPORT DISPATCH_NONNULL1 DISPATCH_NONNULL3 DISPATCH_NOTHROW
void
dispatch_mach_receive_barrier_f(dispatch_mach_t channel, void *context,
		dispatch_function_t barrier);

/*!
 * @function dispatch_mach_get_checkin_port
 * Returns the port specified in the message header remote port of the check-in
 * message passed to the most recent invocation of dispatch_mach_connect() or
 * dispatch_mach_reconnect() for the provided mach channel (irrespective of the
 * completion of the (re)connect or check-in operations in question).
 *
 * Returns MACH_PORT_NULL if dispatch_mach_connect() has not yet been called or
 * if the most recently specified check-in message was NULL, and MACH_PORT_DEAD
 * if the channel has been canceled.
 *
 * It is the responsibility of the application to ensure that the port
 * specified in a check-in message remains valid at the time this function is
 * called.
 *
 * @param channel
 * The mach channel to query.
 *
 * @result
 * The most recently specified check-in port for the channel.
 */
__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_6_0)
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_NOTHROW
mach_port_t
dispatch_mach_get_checkin_port(dispatch_mach_t channel);

#endif // DISPATCH_MACH_SPI

__END_DECLS

#endif
