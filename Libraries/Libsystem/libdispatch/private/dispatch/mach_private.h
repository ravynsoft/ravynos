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

#define DISPATCH_MACH_SPI_VERSION 20161026

#include <mach/mach.h>

DISPATCH_ASSUME_NONNULL_BEGIN

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
 * Reasons for a mach channel handler to be invoked, or the result of an
 * immediate send attempt.
 *
 * @const DISPATCH_MACH_CONNECTED
 * The channel has been connected. The first handler invocation on a channel
 * after calling dispatch_mach_connect() will have this reason.
 *
 * @const DISPATCH_MACH_MESSAGE_RECEIVED
 * A message was received, it is passed in the message parameter.
 * It is the responsibility of the client of this API to handle this and consume
 * or dispose of the rights in the message (for example by calling
 * mach_msg_destroy()).
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
 *
 * @const DISPATCH_MACH_REPLY_RECEIVED
 * A synchronous reply to a call to dispatch_mach_send_and_wait_for_reply() has
 * been received on another thread, an empty message is passed in the message
 * parameter (so that associated port rights can be disposed of).
 * The message header will contain a local port with a receive right associated
 * with the reply to the message that was synchronously sent to the channel.
 *
 * @const DISPATCH_MACH_NEEDS_DEFERRED_SEND
 * The message could not be sent synchronously. Only returned from a send with
 * result operation and never passed to a channel handler. Indicates that the
 * message passed to the send operation must not be disposed of until it is
 * returned via the channel handler.
 *
 * @const DISPATCH_MACH_SIGTERM_RECEIVED
 * A SIGTERM signal has been received. This notification is delivered at most
 * once during the lifetime of the channel. This event is sent only for XPC
 * channels (i.e. channels that were created by calling
 * dispatch_mach_create_4libxpc()) and only if the
 * dmxh_enable_sigterm_notification function in the XPC hooks structure returned
 * true when it was called at channel activation time.
 *
 * @const DISPATCH_MACH_ASYNC_WAITER_DISCONNECTED
 * The channel has been disconnected by a call to dispatch_mach_reconnect() or
 * dispatch_mach_cancel(), an empty message is passed in the message parameter
 * (so that associated port rights can be disposed of). The message header will
 * contain a local port with the receive right previously allocated to receive
 * an asynchronous reply to a message previously sent to the channel. Used
 * only if the channel is disconnected while waiting for a reply to a message
 * sent with dispatch_mach_send_with_result_and_async_reply_4libxpc().
 *
 * @const DISPATCH_MACH_NO_SENDERS
 * Sent when a no senders requested with dispatch_mach_request_no_senders() has
 * been received. See dispatch_mach_request_no_senders().
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
	DISPATCH_MACH_REPLY_RECEIVED,
	DISPATCH_MACH_NEEDS_DEFERRED_SEND,
	DISPATCH_MACH_SIGTERM_RECEIVED,
	DISPATCH_MACH_ASYNC_WAITER_DISCONNECTED,
	DISPATCH_MACH_NO_SENDERS,
	DISPATCH_MACH_REASON_LAST, /* unused */
);

/*!
 * @typedef dispatch_mach_send_flags_t
 * Flags that can be passed to the *with_flags send functions.
 */
DISPATCH_ENUM(dispatch_mach_send_flags, unsigned long,
	DISPATCH_MACH_SEND_DEFAULT = 0,
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
API_AVAILABLE(macos(10.9), ios(7.0))
DISPATCH_EXPORT DISPATCH_MALLOC DISPATCH_RETURNS_RETAINED DISPATCH_WARN_RESULT
DISPATCH_NOTHROW
dispatch_mach_msg_t
dispatch_mach_msg_create(mach_msg_header_t *_Nullable msg, size_t size,
		dispatch_mach_msg_destructor_t destructor,
		mach_msg_header_t *_Nonnull *_Nullable msg_ptr);

/*!
 * @function dispatch_mach_msg_get_msg
 * Returns the message buffer underlying a dispatch mach message object.
 *
 * @param message	The dispatch mach message object to query.
 * @param size_ptr	A pointer to a size_t variable to be filled with the
 *					size of the message buffer, or NULL.
 * @result			Pointer to message buffer underlying the object.
 */
API_AVAILABLE(macos(10.9), ios(7.0))
DISPATCH_EXPORT DISPATCH_NONNULL1 DISPATCH_NOTHROW
mach_msg_header_t*
dispatch_mach_msg_get_msg(dispatch_mach_msg_t message,
		size_t *_Nullable size_ptr);

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
		dispatch_mach_msg_t _Nullable message, mach_error_t error);

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
API_AVAILABLE(macos(10.9), ios(6.0))
DISPATCH_EXPORT DISPATCH_MALLOC DISPATCH_RETURNS_RETAINED DISPATCH_WARN_RESULT
DISPATCH_NONNULL3 DISPATCH_NOTHROW
dispatch_mach_t
dispatch_mach_create(const char *_Nullable label,
		dispatch_queue_t _Nullable queue, dispatch_mach_handler_t handler);
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
typedef void (*dispatch_mach_handler_function_t)(void *_Nullable context,
		dispatch_mach_reason_t reason, dispatch_mach_msg_t _Nullable message,
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
API_AVAILABLE(macos(10.9), ios(6.0))
DISPATCH_EXPORT DISPATCH_MALLOC DISPATCH_RETURNS_RETAINED DISPATCH_WARN_RESULT
DISPATCH_NONNULL4 DISPATCH_NOTHROW
dispatch_mach_t
dispatch_mach_create_f(const char *_Nullable label,
		dispatch_queue_t _Nullable queue, void *_Nullable context,
		dispatch_mach_handler_function_t handler);

/*!
 * @function dispatch_mach_request_no_senders
 *
 * Configure the mach channel to receive no more senders notifications.
 *
 * @discussion
 * This function must be called before dispatch_mach_connect() has been called.
 *
 * When a checkin message is passed to dispatch_mach_connect() or
 * dispatch_mach_reconnect(), the notification is armed after the checkin
 * message has been sent successfully.
 *
 * If no checkin message is passed, then the mach channel is assumed to be
 * a "server" peer connection and the no more senders request is armed
 * immediately.
 *
 * @param channel
 * The mach channel to request no senders notifications on.
 */
API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0), bridgeos(4.0))
DISPATCH_EXPORT DISPATCH_NONNULL1 DISPATCH_NOTHROW
void
dispatch_mach_request_no_senders(dispatch_mach_t channel);

/*!
 * @typedef dispatch_mach_flags_t
 *
 * Flags that can be passed to the dispatch_mach_set_flags function.
 *
 * @const DMF_USE_STRICT_REPLY
 * Instruct the dispatch mach channel to use strict reply port semantics. When
 * using strict reply port semantics, the kernel will enforce that the port
 * used as the reply port has precisely 1 extant send-once right, its receive
 * right exists in the same space as the sender, and any voucher context,
 * e.g., the persona in the bank attribute, used when sending the message is
 * also used when replying.
 *
 * @const DMF_REQUEST_NO_SENDERS
 * Configure the mach channel to receive no more senders notifications.
 * When a checkin message is passed to dispatch_mach_connect() or
 * dispatch_mach_reconnect(), the notification is armed after the checkin
 * message has been sent successfully.  If no checkin message is passed, then
 * the mach channel is assumed to be a "server" peer connection and the no
 * more senders request is armed immediately.
 */
DISPATCH_OPTIONS(dispatch_mach_flags, uint64_t,
	DMF_NONE               = 0x0,
	DMF_USE_STRICT_REPLY   = 0x1,
	DMF_REQUEST_NO_SENDERS = 0x2,
);

/*!
 * @function dispatch_mach_set_flags
 *
 * Configure optional properties on the mach channel.
 *
 * @discussion
 * This function must be called before dispatch_mach_connect() has been called.
 *
 * @param channel
 * The mach channel to configure.
 *
 * @param flags
 * Flags to configure the dispatch mach channel.
 *
 * @see dispatch_mach_flags_t
 */
API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0), bridgeos(4.0))
DISPATCH_EXPORT DISPATCH_NONNULL1 DISPATCH_NOTHROW
void
dispatch_mach_set_flags(dispatch_mach_t channel, dispatch_mach_flags_t flags);

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
API_AVAILABLE(macos(10.9), ios(6.0))
DISPATCH_EXPORT DISPATCH_NONNULL1 DISPATCH_NOTHROW
void
dispatch_mach_connect(dispatch_mach_t channel, mach_port_t receive,
		mach_port_t send, dispatch_mach_msg_t _Nullable checkin);

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
API_AVAILABLE(macos(10.9), ios(6.0))
DISPATCH_EXPORT DISPATCH_NONNULL1 DISPATCH_NOTHROW
void
dispatch_mach_reconnect(dispatch_mach_t channel, mach_port_t send,
		dispatch_mach_msg_t _Nullable checkin);

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
API_AVAILABLE(macos(10.9), ios(6.0))
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_NOTHROW
void
dispatch_mach_cancel(dispatch_mach_t channel);

/*!
 * @function dispatch_mach_mig_demux
 *
 * @abstract
 * Handles an incoming DISPATCH_MACH_MESSAGE_RECEIVED event through a series of
 * MIG subsystem demultiplexers.
 *
 * @discussion
 * This function can be used with a static array of MIG subsystems to try.
 * If it returns true, then the dispatch mach message has been consumed as per
 * usual MIG rules.
 *
 * If it returns false, then the mach message has not been touched, and
 * consuming or disposing of the rights in the message is mandatory.
 *
 * It is hence possible to write a manual demuxer this way:
 *
 * <code>
 * if (!dispatch_mach_mig_demux(context, subsystems, count, message)) {
 *     mach_msg_header_t hdr = dispatch_mach_msg_get_msg(message, NULL);
 *     switch (hdr->msgh_id) {
 *     case ...: // manual consumption of messages
 *         ...
 *         break;
 *     default:
 *         mach_msg_destroy(hdr); // no one claimed the message, destroy it
 *     }
 * }
 * </code>
 *
 * @param context
 * An optional context that the MIG routines can query with
 * dispatch_mach_mig_demux_get_context() as MIG doesn't support contexts.
 *
 * @param subsystems
 * An array of mig_subsystem structs for all the demuxers to try.
 * These are exposed by MIG in the Server header of the generated interface.
 *
 * @param count
 * The number of entries in the subsystems array.
 *
 * @param msg
 * The dispatch mach message to process.
 *
 * @returns
 * Whether or not the dispatch mach message has been consumed.
 * If false is returned, then it is the responsibility of the caller to consume
 * or dispose of the received message rights.
 */
API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0))
DISPATCH_EXPORT DISPATCH_NONNULL2 DISPATCH_NONNULL4 DISPATCH_NOTHROW
bool
dispatch_mach_mig_demux(void *_Nullable context,
		const struct mig_subsystem *_Nonnull const subsystems[_Nonnull],
		size_t count, dispatch_mach_msg_t msg);

/*!
 * @function dispatch_mach_mig_demux_get_context
 *
 * @abstract
 * Returns the context passed to dispatch_mach_mig_demux() from the context of
 * a MIG routine implementation.
 *
 * @discussion
 * Calling this function from another context than a MIG routine called from the
 * context of dispatch_mach_mig_demux_get_context() is invalid and will cause
 * your process to be terminated.
 *
 * @returns
 * The context passed to the outer call to dispatch_mach_mig_demux().
 */
API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0))
DISPATCH_EXPORT DISPATCH_NOTHROW
void *_Nullable
dispatch_mach_mig_demux_get_context(void);

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
API_AVAILABLE(macos(10.9), ios(6.0))
DISPATCH_EXPORT DISPATCH_NONNULL1 DISPATCH_NONNULL2 DISPATCH_NOTHROW
void
dispatch_mach_send(dispatch_mach_t channel, dispatch_mach_msg_t message,
		mach_msg_option_t options);

/*!
 * @function dispatch_mach_send_with_result
 * Asynchronously send a message encapsulated in a dispatch mach message object
 * to the specified mach channel. If an immediate send can be performed, return
 * its result via out parameters.
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
 * If an immediate send could be performed, returns the resulting reason
 * (e.g. DISPATCH_MACH_MESSAGE_SENT) and possible error to the caller in the
 * send_result and send_error out parameters (instead of via the channel
 * handler), in which case the passed-in message and associated resources
 * can be disposed of synchronously.
 *
 * If a deferred send is required, returns DISPATCH_MACH_NEEDS_DEFERRED_SEND
 * in the send_result out parameter to indicate that the passed-in message has
 * been retained and associated resources must not be disposed of until the
 * message is returned asynchronusly via the channel handler.
 *
 * @param channel
 * The mach channel to which to send the message.
 *
 * @param message
 * The message object encapsulating the message to send. Unless an immediate
 * send could be performed, the object will be retained until the asynchronous
 * send operation is complete and the channel handler has returned. The storage
 * underlying the message object may be modified by the send operation.
 *
 * @param options
 * Additional send options to pass to mach_msg() when performing the send
 * operation.
 *
 * @param send_flags
 * Flags to configure the send operation. Must be 0 for now.
 *
 * @param send_result
 * Out parameter to return the result of the immediate send attempt.
 * If a deferred send is required, returns DISPATCH_MACH_NEEDS_DEFERRED_SEND.
 * Must not be NULL.
 *
 * @param send_error
 * Out parameter to return the error from the immediate send attempt.
 * If a deferred send is required, returns 0. Must not be NULL.
 */
API_AVAILABLE(macos(10.12), ios(10.0), tvos(10.0), watchos(3.0))
DISPATCH_EXPORT DISPATCH_NONNULL1 DISPATCH_NONNULL2 DISPATCH_NONNULL5
DISPATCH_NONNULL6 DISPATCH_NOTHROW
void
dispatch_mach_send_with_result(dispatch_mach_t channel,
		dispatch_mach_msg_t message, mach_msg_option_t options,
		dispatch_mach_send_flags_t send_flags,
		dispatch_mach_reason_t *send_result, mach_error_t *send_error);

/*!
 * @function dispatch_mach_send_and_wait_for_reply
 * Synchronously send a message encapsulated in a dispatch mach message object
 * to the specified mach channel and wait for a reply.
 *
 * Unless the message is being sent to a send-once right (as determined by the
 * presence of MACH_MSG_TYPE_MOVE_SEND_ONCE in the message header remote bits),
 * the message header remote port is set to the channel send right before the
 * send operation is performed.
 *
 * The message is required to expect a direct reply (as determined by the
 * presence of MACH_MSG_TYPE_MAKE_SEND_ONCE in the message header local bits)
 * and this function will not complete until the receive right specified in the
 * message header local port receives a reply message (or a send-once
 * notification) which will be returned, or until that receive right is
 * destroyed in response to the channel being canceled, in which case NULL will
 * be returned.
 * In all these cases the application must wait for the channel handler to
 * be invoked with a DISPATCH_MACH_REPLY_RECEIVED or DISPATCH_MACH_DISCONNECTED
 * message before releasing that receive right.
 *
 * Alternatively, the application may specify MACH_PORT_NULL in the header local
 * port to indicate that the channel should create and manage the reply receive
 * right internally, including destroying it upon channel cancellation.
 * This is a more efficient mode of operation as no asynchronous operations are
 * required to return the receive right (i.e. the channel handler will not be
 * called as described above).
 *
 * If the message send operation is attempted but the channel is canceled
 * before the send operation succesfully completes, the message returned to the
 * channel handler with DISPATCH_MACH_MESSAGE_NOT_SENT may be the result of a
 * pseudo-receive operation. The receive right originally specified in the
 * message header local port will returned in a DISPATCH_MACH_DISCONNECTED
 * message (unless it was MACH_PORT_NULL).
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
 *
 * @result
 * The received reply message object, or NULL if the channel was canceled.
 */
API_AVAILABLE(macos(10.11), ios(9.0))
DISPATCH_EXPORT DISPATCH_MALLOC DISPATCH_RETURNS_RETAINED DISPATCH_WARN_RESULT
DISPATCH_NONNULL1 DISPATCH_NONNULL2 DISPATCH_NOTHROW
dispatch_mach_msg_t _Nullable
dispatch_mach_send_and_wait_for_reply(dispatch_mach_t channel,
		dispatch_mach_msg_t message, mach_msg_option_t options);

/*!
 * @function dispatch_mach_send_with_result_and_wait_for_reply
 * Synchronously send a message encapsulated in a dispatch mach message object
 * to the specified mach channel and wait for a reply. If an immediate send can
 * be performed, return its result via out parameters.
 *
 * Unless the message is being sent to a send-once right (as determined by the
 * presence of MACH_MSG_TYPE_MOVE_SEND_ONCE in the message header remote bits),
 * the message header remote port is set to the channel send right before the
 * send operation is performed.
 *
 * The message is required to expect a direct reply (as determined by the
 * presence of MACH_MSG_TYPE_MAKE_SEND_ONCE in the message header local bits)
 * and this function will not complete until the receive right specified in the
 * message header local port receives a reply message (or a send-once
 * notification) which will be returned, or until that receive right is
 * destroyed in response to the channel being canceled, in which case NULL will
 * be returned.
 * In all these cases the application must wait for the channel handler to
 * be invoked with a DISPATCH_MACH_REPLY_RECEIVED or DISPATCH_MACH_DISCONNECTED
 * message before releasing that receive right.
 *
 * Alternatively, the application may specify MACH_PORT_NULL in the header local
 * port to indicate that the channel should create and manage the reply receive
 * right internally, including destroying it upon channel cancellation.
 * This is a more efficient mode of operation as no asynchronous operations are
 * required to return the receive right (i.e. the channel handler will not be
 * called as described above).
 *
 * If the message send operation is attempted but the channel is canceled
 * before the send operation succesfully completes, the message returned to the
 * channel handler with DISPATCH_MACH_MESSAGE_NOT_SENT may be the result of a
 * pseudo-receive operation. The receive right originally specified in the
 * message header local port will returned in a DISPATCH_MACH_DISCONNECTED
 * message (unless it was MACH_PORT_NULL).
 *
 * If an immediate send could be performed, returns the resulting reason
 * (e.g. DISPATCH_MACH_MESSAGE_SENT) and possible error to the caller in the
 * send_result and send_error out parameters (instead of via the channel
 * handler), in which case the passed-in message and associated resources
 * can be disposed of synchronously.
 *
 * If a deferred send is required, returns DISPATCH_MACH_NEEDS_DEFERRED_SEND
 * in the send_result out parameter to indicate that the passed-in message has
 * been retained and associated resources must not be disposed of until the
 * message is returned asynchronusly via the channel handler.
 *
 * @param channel
 * The mach channel to which to send the message.
 *
 * @param message
 * The message object encapsulating the message to send. Unless an immediate
 * send could be performed, the object will be retained until the asynchronous
 * send operation is complete and the channel handler has returned. The storage
 * underlying the message object may be modified by the send operation.
 *
 * @param options
 * Additional send options to pass to mach_msg() when performing the send
 * operation.
 *
 * @param send_flags
 * Flags to configure the send operation. Must be 0 for now.
 *
 * @param send_result
 * Out parameter to return the result of the immediate send attempt.
 * If a deferred send is required, returns DISPATCH_MACH_NEEDS_DEFERRED_SEND.
 * Must not be NULL.
 *
 * @param send_error
 * Out parameter to return the error from the immediate send attempt.
 * If a deferred send is required, returns 0. Must not be NULL.
 *
 * @result
 * The received reply message object, or NULL if the channel was canceled.
 */
API_AVAILABLE(macos(10.12), ios(10.0), tvos(10.0), watchos(3.0))
DISPATCH_EXPORT DISPATCH_MALLOC DISPATCH_RETURNS_RETAINED DISPATCH_WARN_RESULT
DISPATCH_NONNULL1 DISPATCH_NONNULL2 DISPATCH_NONNULL5 DISPATCH_NONNULL6
DISPATCH_NOTHROW
dispatch_mach_msg_t _Nullable
dispatch_mach_send_with_result_and_wait_for_reply(dispatch_mach_t channel,
		dispatch_mach_msg_t message, mach_msg_option_t options,
		dispatch_mach_send_flags_t send_flags,
		dispatch_mach_reason_t *send_result, mach_error_t *send_error);

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
API_AVAILABLE(macos(10.9), ios(6.0))
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
API_AVAILABLE(macos(10.9), ios(6.0))
DISPATCH_EXPORT DISPATCH_NONNULL1 DISPATCH_NONNULL3 DISPATCH_NOTHROW
void
dispatch_mach_send_barrier_f(dispatch_mach_t channel, void *_Nullable context,
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
API_AVAILABLE(macos(10.9), ios(6.0))
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
API_AVAILABLE(macos(10.9), ios(6.0))
DISPATCH_EXPORT DISPATCH_NONNULL1 DISPATCH_NONNULL3 DISPATCH_NOTHROW
void
dispatch_mach_receive_barrier_f(dispatch_mach_t channel, void *_Nullable context,
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
API_AVAILABLE(macos(10.9), ios(6.0))
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_NOTHROW
mach_port_t
dispatch_mach_get_checkin_port(dispatch_mach_t channel);

#if DISPATCH_MACH_SPI

// SPI for libxpc
/*
 * Type for the callback for receipt of asynchronous replies to
 * dispatch_mach_send_with_result_and_async_reply_4libxpc().
 */
typedef void (*_Nonnull dispatch_mach_async_reply_callback_t)(void *context,
		dispatch_mach_reason_t reason, dispatch_mach_msg_t message);

API_AVAILABLE(macos(10.13), ios(11.0), tvos(11.0), watchos(4.0))
typedef const struct dispatch_mach_xpc_hooks_s {
#define DISPATCH_MACH_XPC_MIN_HOOKS_VERSION 3
#define DISPATCH_MACH_XPC_HOOKS_VERSION     3
	unsigned long version;

	/* Fields available in version 1. */

	/*
	 * Called to handle a Mach message event inline if possible. Returns true
	 * if the event was handled, false if the event should be delivered to the
	 * channel event handler. The implementation should not make any assumptions
	 * about the thread in which the function is called and cannot assume that
	 * invocations of this function are serialized relative to each other or
	 * relative to the channel's event handler function. In addition, the
	 * handler must not throw an exception or call out to any code that might
	 * throw an exception.
	 */
	bool (* _Nonnull dmxh_direct_message_handler)(void *_Nullable context,
			dispatch_mach_reason_t reason, dispatch_mach_msg_t message,
			mach_error_t error);

	/* Fields available in version 2. */

#define DMXH_MSG_CONTEXT_REPLY_QUEUE_SELF ((dispatch_queue_t)NULL)

	/*
	 * Gets the queue to which a reply to a message sent using
	 * dispatch_mach_send_with_result_and_async_reply_4libxpc() should be
	 * delivered. The msg_context argument is the value of the do_ctxt field
	 * of the outgoing message, as returned by dispatch_get_context().
	 *
	 * This function should return a consistent result until an event is
	 * received for this message. This function must return NULL if
	 * dispatch_mach_send_with_result_and_async_reply_4libxpc() wasn't used to
	 * send the message, and non NULL otherwise.
	 */
	dispatch_queue_t _Nullable (*_Nonnull dmxh_msg_context_reply_queue)(
			void *_Nonnull msg_context);

	/*
	 * Called when a reply to a message sent by
	 * dispatch_mach_send_with_result_and_async_reply_4libxpc() is received. The
	 * message argument points to the reply message and the context argument is
	 * the context value passed to dispatch_mach_create_4libxpc() when creating
	 * the Mach channel. The handler is called on the queue that is returned by
	 * dmxh_msg_context_reply_queue() when the reply is received or if the
	 * channel is disconnected. The reason argument is
	 * DISPATCH_MACH_MESSAGE_RECEIVED if a reply has been received or
	 * DISPATCH_MACH_ASYNC_WAITER_DISCONNECTED if the channel has been
	 * disconnected. Refer to the documentation for
	 * dispatch_mach_send_with_result_and_async_reply_4libxpc() for more
	 * details.
	 */
	dispatch_mach_async_reply_callback_t dmxh_async_reply_handler;

	/* Fields available in version 3. */
	/*
	 * Called once when the Mach channel has been activated. If this function
	 * returns true, a DISPATCH_MACH_SIGTERM_RECEIVED notification will be
	 * delivered to the channel's event handler when a SIGTERM is received.
	 */
	bool (*_Nonnull dmxh_enable_sigterm_notification)(
			void *_Nullable context);
} *dispatch_mach_xpc_hooks_t;

/*!
 * @function dispatch_mach_hooks_install_4libxpc
 *
 * @abstract
 * installs XPC callbacks for dispatch Mach channels.
 *
 * @discussion
 * In order to improve the performance of the XPC/dispatch interface, it is
 * sometimes useful for dispatch to be able to call directly into XPC. The
 * channel hooks structure should be initialized with pointers to XPC callback
 * functions, or NULL for callbacks that XPC does not support. The version
 * number in the structure must be set to reflect the fields that have been
 * initialized. This function may be called only once.
 *
 * @param hooks
 * A pointer to the channel hooks structure. This must remain valid once set.
 */
API_AVAILABLE(macos(10.13), ios(11.0), tvos(11.0), watchos(4.0))
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_NOTHROW
void
dispatch_mach_hooks_install_4libxpc(dispatch_mach_xpc_hooks_t hooks);

/*!
 * @function dispatch_mach_create_4libxpc
 * Create a dispatch mach channel to asynchronously receive and send mach
 * messages, specifically for libxpc.
 *
 * The specified handler will be called with the corresponding reason parameter
 * for each message received and for each message that was successfully sent,
 * that failed to be sent, or was not sent; as well as when a barrier block
 * has completed, or when channel connection, reconnection or cancellation has
 * taken effect. However, the handler will not be called for messages that
 * were passed to the XPC hooks dmxh_direct_message_handler function if that
 * function returned true.
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
API_AVAILABLE(macos(10.13), ios(11.0), tvos(11.0), watchos(4.0))
DISPATCH_EXPORT DISPATCH_MALLOC DISPATCH_RETURNS_RETAINED DISPATCH_WARN_RESULT
DISPATCH_NONNULL4 DISPATCH_NOTHROW
dispatch_mach_t
dispatch_mach_create_4libxpc(const char *_Nullable label,
		dispatch_queue_t _Nullable queue, void *_Nullable context,
		dispatch_mach_handler_function_t handler);

/*!
 * @function dispatch_mach_send_with_result_and_async_reply_4libxpc
 * SPI for XPC that asynchronously sends a message encapsulated in a dispatch
 * mach message object to the specified mach channel. If an immediate send can
 * be performed, returns its result via out parameters.
 *
 * The reply message is processed on the queue returned by the
 * dmxh_msg_context_reply_queue function in the dispatch_mach_xpc_hooks_s
 * structure, which is called with a single argument whose value is the
 * do_ctxt field of the message argument to this function. The reply message is
 * delivered to the dmxh_async_reply_handler hook function instead of being
 * passed to the channel event handler.
 *
 * If the dmxh_msg_context_reply_queue function is not implemented or returns
 * NULL, the reply message is delivered to the channel event handler on the
 * channel queue.
 *
 * Unless the message is being sent to a send-once right (as determined by the
 * presence of MACH_MSG_TYPE_MOVE_SEND_ONCE in the message header remote bits),
 * the message header remote port is set to the channel send right before the
 * send operation is performed.
 *
 * The message is required to expect a direct reply (as determined by the
 * presence of MACH_MSG_TYPE_MAKE_SEND_ONCE in the message header local bits).
 * The receive right specified in the message header local port will be
 * monitored until a reply message (or a send-once notification) is received, or
 * the channel is canceled. Hence the application must wait for the reply
 * to be received or for a DISPATCH_MACH_ASYNC_WAITER_DISCONNECTED message
 * before releasing that receive right.
 *
 * If the message send operation is attempted but the channel is canceled
 * before the send operation succesfully completes, the message returned to the
 * channel handler with DISPATCH_MACH_MESSAGE_NOT_SENT may be the result of a
 * pseudo-receive operation and the receive right originally specified in the
 * message header local port will be returned in a
 * DISPATCH_MACH_ASYNC_WAITER_DISCONNECTED message.
 *
 * If an immediate send could be performed, returns the resulting reason
 * (e.g. DISPATCH_MACH_MESSAGE_SENT) and possible error to the caller in the
 * send_result and send_error out parameters (instead of via the channel
 * handler), in which case the passed-in message and associated resources
 * can be disposed of synchronously.
 *
 * If a deferred send is required, returns DISPATCH_MACH_NEEDS_DEFERRED_SEND
 * in the send_result out parameter to indicate that the passed-in message has
 * been retained and associated resources must not be disposed of until the
 * message is returned asynchronusly via the channel handler.
 *
 * @param channel
 * The mach channel to which to send the message.
 *
 * @param message
 * The message object encapsulating the message to send. Unless an immediate
 * send could be performed, the object will be retained until the asynchronous
 * send operation is complete and the channel handler has returned. The storage
 * underlying the message object may be modified by the send operation.
 *
 * @param options
 * Additional send options to pass to mach_msg() when performing the send
 * operation.
 *
 * @param send_flags
 * Flags to configure the send operation. Must be 0 for now.
 *
 * @param send_result
 * Out parameter to return the result of the immediate send attempt.
 * If a deferred send is required, returns DISPATCH_MACH_NEEDS_DEFERRED_SEND.
 * Must not be NULL.
 *
 * @param send_error
 * Out parameter to return the error from the immediate send attempt.
 * If a deferred send is required, returns 0. Must not be NULL.
 */
API_AVAILABLE(macos(10.13), ios(11.0), tvos(11.0), watchos(4.0))
DISPATCH_EXPORT DISPATCH_NONNULL1 DISPATCH_NONNULL2 DISPATCH_NONNULL5
DISPATCH_NONNULL6 DISPATCH_NOTHROW
void
dispatch_mach_send_with_result_and_async_reply_4libxpc(dispatch_mach_t channel,
		dispatch_mach_msg_t message, mach_msg_option_t options,
		dispatch_mach_send_flags_t send_flags,
		dispatch_mach_reason_t *send_result, mach_error_t *send_error);

#endif // DISPATCH_MACH_SPI

/*!
 * @function dispatch_mach_handoff_reply_f
 *
 * @abstract
 * Inform the runtime that a given sync IPC is being handed off to a new queue
 * hierarchy.
 *
 * @discussion
 * This function can only be called from the context of an IPC handler, or from
 * a work item created by dispatch_mach_handoff_reply_f. Calling
 * dispatch_mach_handoff_reply_f from a different context is undefined and will
 * cause the process to be terminated.
 *
 * dispatch_mach_handoff_reply_f will only take effect when the work item that
 * issued it returns.
 *
 * @param queue
 * The queue the IPC reply will be handed off to. This queue must be an
 * immutable queue hierarchy (with all nodes created with
 * dispatch_queue_create_with_target() for example).
 *
 * @param port
 * The send once right that will be replied to.
 */
API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0), bridgeos(4.0))
DISPATCH_EXPORT DISPATCH_NONNULL1 DISPATCH_NONNULL4 DISPATCH_NOTHROW
void
dispatch_mach_handoff_reply_f(dispatch_queue_t queue, mach_port_t port,
		void *_Nullable ctxt, dispatch_function_t func);

/*!
 * @function dispatch_mach_handoff_reply
 *
 * @abstract
 * Inform the runtime that a given sync IPC is being handed off to a new queue
 * hierarchy.
 *
 * @see dispatch_mach_handoff_reply_f
 */
#ifdef __BLOCKS__
API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0), bridgeos(4.0))
DISPATCH_EXPORT DISPATCH_NONNULL1 DISPATCH_NONNULL3 DISPATCH_NOTHROW
void
dispatch_mach_handoff_reply(dispatch_queue_t queue, mach_port_t port,
		dispatch_block_t block);
#endif /* __BLOCKS__ */

DISPATCH_ASSUME_NONNULL_END

__END_DECLS

#endif
