/*
 * Copyright (c) 2017 Apple Inc. All rights reserved.
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

#ifndef __DISPATCH_CHANNEL_PRIVATE__
#define __DISPATCH_CHANNEL_PRIVATE__

#ifndef __DISPATCH_INDIRECT__
#error "Please #include <dispatch/private.h> instead of this file directly."
#include <dispatch/base.h> // for HeaderDoc
#endif

DISPATCH_ASSUME_NONNULL_BEGIN

__BEGIN_DECLS

#if DISPATCH_CHANNEL_SPI

/*!
 * @typedef dispatch_channel_t
 *
 * @abstract
 */
DISPATCH_DECL(dispatch_channel);

typedef struct dispatch_channel_invoke_ctxt_s *dispatch_channel_invoke_ctxt_t;

/*! @typedef dispatch_channel_callbacks_t
 *
 * @abstract
 * Vtable used by dispatch channels (see dispatch_channel_create).
 */
API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0), bridgeos(4.0))
typedef struct dispatch_channel_callbacks_s {
#define DISPATCH_CHANNEL_CALLBACKS_VERSION 1ul
	/*! @field dcc_version
	 *
	 * @abstract
	 * Version of the callbacks, used for binary compatibilty.
	 * This must be set to DISPATCH_CHANNEL_CALLBACKS_VERSION
	 */
	unsigned long dcc_version;

	/*! @field dcc_probe
	 *
	 * @abstract
	 * This callback is called when GCD is considering whether it should wakeup
	 * the channel.
	 *
	 * @discussion
	 * This function may be called from ANY context. It may be called
	 * concurrently from several threads, it may be called concurrently with
	 * a call to other channel callbacks.
	 *
	 * Reasons for this function to be called include:
	 * - the channel became non empty,
	 * - the channel is receiving a Quality of Service override to resolve
	 *   a priority inversion,
	 * - dispatch_activate() or dispatch_resume() was called,
	 * - dispatch_channel_wakeup() was called.
	 *
	 * The implementation of this callback should be idempotent, and as cheap
	 * as possible, avoiding taking locks if possible. A typical implementation
	 * will perform a single atomic state look to determine what answer to
	 * return. Possible races or false positives can be later be debounced in
	 * dcc_invoke which is synchronized.
	 *
	 * Calling dispatch_channel_wakeup() from the context of this call is
	 * incorrect and will result in undefined behavior. Instead, it should be
	 * called in response to external events, in order to cause the channel to
	 * re-evaluate the `dcc_probe` hook.
	 *
	 * param channel
	 * The channel that is being probed.
	 *
	 * param context
	 * The context associated with the channel.
	 *
	 * returns
	 * - true if the dispatch channel can be woken up according to the other
	 *   runtime rules
	 *
	 * - false if the dispatch channel would not be able to make progress if
	 *   woken up. A subsequent explicit call to dispatch_channel_wakeup() will
	 *   be required when this condition has changed.
	 */
	bool
	(*_Nonnull dcc_probe)(dispatch_channel_t channel, void *_Nullable context);

	/*! @field dcc_invoke
	 *
	 * @abstract
	 * This callback is called when a dispatch channel is being drained.
	 *
	 * @discussion
	 * This callback is where the state machine for the channel can
	 * be implemented using dispatch_channel_foreach_work_item_peek()
	 * and dispatch_channel_drain().
	 *
	 * Note that if this function returns true, it must have called
	 * dispatch_channel_drain() exactly once. It is valid not to call
	 * peek nor drain if false is returned.
	 *
	 * param channel
	 * The channel that has been invoked.
	 *
	 * param invoke_context
	 * An opaque data structure that must be passed back to
	 * dispatch_channel_foreach_work_item_peek() and dispatch_channel_drain().
	 *
	 * param context
	 * The context associated with the channel.
	 *
	 * returns
	 * - true if the channel can drain further
	 * - false if an explicit call to dispatch_channel_wakeup() is required
	 *   for the channel to be able to drain items again. A subsequent explicit
	 *   call to dispatch_channel_wakeup() will be required when this condition
	 *   has changed.
	 */
	bool
	(*_Nonnull dcc_invoke)(dispatch_channel_t channel,
			dispatch_channel_invoke_ctxt_t invoke_context,
			void *_Nullable context);

	/*! @field dcc_acknowledge_cancel
	 *
	 * @abstract
	 * This optional callback is called when the channel has been cancelled
	 * until that cancellation is acknowledged.
	 *
	 * @discussion
	 * If this callback isn't set, the channel cancelation is implicit and can
	 * be tested with dispatch_channel_testcancel().
	 *
	 * When this callback is set, it will be called as soon as cancelation has
	 * been noticed. When it is called, it is called from a context serialized
	 * with `dcc_invoke`, or from `dcc_invoke` itself.
	 *
	 * Returning `false` causes the dispatch channel to stop its invocation
	 * early. A subsequent explicit call to dispatch_channel_wakeup() will be
	 * required when the cancellation can be acknowledged.
	 *
	 * param channel
	 * The channel that has been invoked.
	 *
	 * param context
	 * The context associated with the channel.
	 *
	 * returns
	 * Whether the cancellation was acknowledged.
	 */
	bool
	(*_Nullable dcc_acknowledge_cancel)(dispatch_channel_t channel,
			void *_Nullable context);
} const *dispatch_channel_callbacks_t;

/*! @function dispatch_channel_create
 *
 * @abstract
 * Create a dispatch channel.
 *
 * @discussion
 * A dispatch channel is similar to a dispatch serial queue, however it will
 * accept arbitrary items into the queue, as well as regular dispatch blocks
 * to execute.
 *
 * Unlike serial queues, this object cannot be targeted by other dispatch
 * objects.
 *
 * Dispatch channels are created in an inactive state. After creating the
 * channel and setting any desired property, a call must be made to
 * dispatch_activate() in order to use the object.
 *
 * Calling dispatch_set_target_queue() on a channel after it has been activated
 * is not allowed (see dispatch_activate() and dispatch_set_target_queue()).
 *
 * @param label
 * A string label to attach to the channel.
 * This parameter is optional and may be NULL.
 *
 * @param context
 * A context to associated with the channel. It can be retrieved with
 * dispatch_get_context() at any time, but should not mutated.
 *
 * @param target
 * The target queue for the newly created channel. The target queue is retained.
 * If this parameter is DISPATCH_TARGET_QUEUE_DEFAULT, sets the channel's target
 * queue to the default target queue for the given channel type.
 *
 * @param callbacks
 * Hooks for the created channel.
 *
 * @returns
 * The newly created channel.
 */
API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0), bridgeos(4.0))
DISPATCH_EXPORT DISPATCH_MALLOC DISPATCH_RETURNS_RETAINED DISPATCH_WARN_RESULT
DISPATCH_NOTHROW DISPATCH_NONNULL4
dispatch_channel_t
dispatch_channel_create(const char *_Nullable label,
		dispatch_queue_t _Nullable target,
		void *_Nullable context, dispatch_channel_callbacks_t callbacks);

/*! @function dispatch_channel_wakeup
 *
 * @abstract
 * Re-evaluate whether a dispatch channel needs to be woken up.
 *
 * @discussion
 * Calling this function causes the GCD runtime to reevaluate whether
 * the specified dispatch channel needs to be woken up. If a previous call to
 * `dcc_probe`, `dcc_acknowledge_cancel` or `dcc_invoke` returned false then
 * a channel may remain asleep until wakeup is called.
 *
 * It is valid to call this function from the context of any of the the `invoke`
 * callbacks, but not from the `dcc_probe` callback.
 *
 * This function will have no effect if:
 * - the dispatch channel is suspeneded,
 * - the `dcc_probe` callback subsequently returns false,
 * - the dispatch channel has no work items queued, nor a pending cancellation
 *   to acknowledge.
 *
 * @param channel
 * The channel for which wakeup should be evaluated.
 *
 * @param qos_class
 * The QoS override that should be applied to this channel because of this
 * event. The override will persist until the channel has been drained of
 * pending items.
 *
 * It is expected that most wakeups will not require an additional QoS
 * override. In this case, passing QOS_CLASS_UNSPECIFIED indicates that no
 * additional override should be applied as a result of this wakeup.
 */
API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0), bridgeos(4.0))
DISPATCH_EXPORT DISPATCH_NOTHROW DISPATCH_NONNULL_ALL
void
dispatch_channel_wakeup(dispatch_channel_t channel, qos_class_t qos_class);

/*! @typedef dispatch_channel_enumerator_handler_t
 *
 * Type of the callbacks used by dispatch_channel_foreach_work_item_peek_f().
 */
typedef bool (*dispatch_channel_enumerator_handler_t)(void *_Nullable context, void *item);

/*! @function dispatch_channel_foreach_work_item_peek_f
 *
 * @abstract
 * Peek at opaque work items currently enqueued on the channel.
 *
 * @discussion
 * This function will enumerate items enqueued on the channel, in order, until
 * the first non-opaque work item is found. No work should be performed on
 * behalf of the items enumerated.
 *
 * This function allows the caller to preflight items that will be processed
 * when draining the channel (fex. counting items in order to pre-allocate
 * storage, or batch items into groups).
 *
 * This function can only be called from the context of the `dcc_invoke`
 * callback associated with this channel, and before any call to
 * dispatch_channel_drain().
 *
 * @param invoke_context
 * The opaque invoke context passed to the channel `dcc_invoke` callback.
 *
 * @param context
 * An application-defined context that will be passed to the handler.
 *
 * @param handler
 * The handler that will be passed `context` and the opaque work item
 * currently enumerated.
 */
API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0), bridgeos(4.0))
DISPATCH_EXPORT DISPATCH_NOTHROW DISPATCH_NONNULL_ALL
void
dispatch_channel_foreach_work_item_peek_f(
		dispatch_channel_invoke_ctxt_t invoke_context,
		void *_Nullable context, dispatch_channel_enumerator_handler_t handler);

#ifdef __BLOCKS__
/*! @typedef dispatch_channel_enumerator_block_t
 *
 * Type of the callbacks used by dispatch_channel_foreach_work_item_peek().
 */
typedef bool (^dispatch_channel_enumerator_block_t)(void *item);

/*! @function dispatch_channel_foreach_work_item_peek_f
 *
 * @abstract
 * Peek at the opaque work items currently enqueued on the channel.
 *
 * @discussion
 * See dispatch_channel_foreach_work_item_peek_f()
 *
 * @param invoke_context
 * The opaque invoke context passed to the channel `dcc_invoke` callback.
 *
 * @param block
 * The block that will be passed the opaque work item currently enumerated.
 */
API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0), bridgeos(4.0))
DISPATCH_EXPORT DISPATCH_NOTHROW DISPATCH_NONNULL_ALL
void
dispatch_channel_foreach_work_item_peek(
		dispatch_channel_invoke_ctxt_t invoke_context,
		dispatch_channel_enumerator_block_t block DISPATCH_NOESCAPE);
#endif

/*! @typedef dispatch_channel_drain_handler_t
 *
 * @abstract
 * Type of the callbacks used by dispatch_channel_drain_f().
 *
 * @param context
 * The application defined context passed to dispatch_channel_drain_f().
 *
 * @param item
 * The opaque work item to consume.
 *
 * @param rejected_item
 * An out-parameter for an opaque work item to put back at the head of the
 * queue. On return from this handler, if rejected_item is set then the handler
 * must also return false (and, thus, interrupts the drain operation).
 *
 * @returns
 * - true if the drain may enumerate the next item
 * - false to cause dispatch_channel_drain_f() to return.
 *   in which case a rejected item can optionally be returned.
 */
typedef bool (*dispatch_channel_drain_handler_t)(void *_Nullable context,
		void *_Nonnull item, void *_Nonnull *_Nullable rejected_item);

/*! @function dispatch_channel_drain_f
 *
 * @abstract
 * Drain the opaque work items enqueued on the channel.
 *
 * @discussion
 * This function needs to be called by any `dcc_invoke` that returns true.
 *
 * Calling drain will cause every opaque work item that can be consumed to be
 * passed to the handler. While the handler is called, the runtime environment
 * matches the QOS and context captured at dispatch_channel_enqueue() time for
 * this opaque work item.
 *
 * Note, this function can (through factors internal to the GCD runtime) can
 * decide not to consume all items that are currently enqueued on the channel.
 * Therefore it is possible for dispatch_channel_drain_f() to enumerate fewer
 * items than dispatch_channel_foreach_work_item_peek_f() did when called
 * immediately beforehand.
 *
 * It is also possible for dispatch_channel_drain_f() to observe *more* items
 * than previously seen with peek, if enqueues are happening concurrently.
 *
 * Note that work items enqueued with dispatch_channel_async() act as
 * "separators". If the opaque work item O1 is enqueued before a regular
 * asynchronous work item A, and a new opaque work item O2 is then enqueued,
 * then neither dispatch_channel_foreach_work_item_peek_f() nor
 * dispatch_channel_drain_f() will ever return O1 and O2 as part of the same
 * drain streak.
 *
 * @param invoke_context
 * The opaque invoke context passed to the channel `dcc_invoke` callback.
 *
 * @param handler
 * The handler that will be passed the context and opaque work item to invoke.
 */
API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0), bridgeos(4.0))
DISPATCH_EXPORT DISPATCH_NOTHROW DISPATCH_NONNULL_ALL
void
dispatch_channel_drain_f(dispatch_channel_invoke_ctxt_t invoke_context,
		void *_Nullable context, dispatch_channel_drain_handler_t handler);

#ifdef __BLOCKS__
/*! @typedef dispatch_channel_drain_block_t
 *
 * @abstract
 * Type of the callbacks used by dispatch_channel_drain().
 *
 * @description
 * See dispatch_channel_drain_handler_t.
 */
typedef bool (^dispatch_channel_drain_block_t)(void *_Nonnull item,
		void *_Nonnull *_Nullable rejected_item);

/*! @function dispatch_channel_drain
 *
 * @abstract
 * Drain the opaque work items enqueued on the channel.
 *
 * @discussion
 * See dispatch_channel_drain_f()
 *
 * @param invoke_context
 * The opaque invoke context passed to the channel `dcc_invoke` callback.
 *
 * @param block
 * The block that will be passed the opaque work item to invoke.
 */
API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0), bridgeos(4.0))
DISPATCH_EXPORT DISPATCH_NOTHROW DISPATCH_NONNULL_ALL
void
dispatch_channel_drain(dispatch_channel_invoke_ctxt_t invoke_context,
		dispatch_channel_drain_block_t block DISPATCH_NOESCAPE);
#endif

/*!
 * @function dispatch_channel_cancel
 *
 * @abstract
 * Asynchronously cancel the dispatch channel.
 *
 * @discussion
 * Cancellation will cause the channel to repeatedly call the
 * `dcc_acknowledge_cancel` handler until it returns true. This allows the
 * associated state machine to handle cancellation asynchronously (and, if
 * needed, in multiple phases).
 *
 * The precise semantics of cancellation are up to the dispatch channel
 * associated state machine, and not all dispatch channels need to use
 * cancellation.
 *
 * However, if the `dcc_acknowledge_cancel` callback is implemented, then an
 * explicit call to dispatch_channel_cancel() is mandatory before the last
 * reference to the dispatch channel is released.
 *
 * @param channel
 * The dispatch channel to be canceled.
 * The result of passing NULL in this parameter is undefined.
 */
API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0), bridgeos(4.0))
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_NOTHROW
void
dispatch_channel_cancel(dispatch_channel_t channel);

/*!
 * @function dispatch_channel_testcancel
 *
 * @abstract
 * Tests whether the given dispatch channel has been canceled.
 *
 * @param channel
 * The dispatch channel to be tested.
 * The result of passing NULL in this parameter is undefined.
 *
 * @result
 * Non-zero if canceled and zero if not canceled.
 */
API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0), bridgeos(4.0))
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_WARN_RESULT DISPATCH_PURE
DISPATCH_NOTHROW
long
dispatch_channel_testcancel(dispatch_channel_t channel);

/*!
 * @function dispatch_channel_async
 *
 * @abstract
 * Submits a block for asynchronous execution on a dispatch channel.
 *
 * @discussion
 * See dispatch_async().
 *
 * @param channel
 * The target dispatch channel to which the block is submitted.
 * The system will hold a reference on the target channel until the block
 * has finished.
 * The result of passing NULL in this parameter is undefined.
 *
 * @param block
 * The block to submit to the target dispatch channel. This function performs
 * Block_copy() and Block_release() on behalf of callers.
 * The result of passing NULL in this parameter is undefined.
 */
#ifdef __BLOCKS__
API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0), bridgeos(4.0))
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_NOTHROW
void
dispatch_channel_async(dispatch_channel_t queue, dispatch_block_t block);
#endif

/*!
 * @function dispatch_channel_async_f
 *
 * @abstract
 * Submits a function for asynchronous execution on a dispatch channel.
 *
 * @discussion
 * See dispatch_async() for details.
 *
 * @param queue
 * The target dispatch channel to which the function is submitted.
 * The system will hold a reference on the target channel until the function
 * has returned.
 * The result of passing NULL in this parameter is undefined.
 *
 * @param context
 * The application-defined context parameter to pass to the function.
 *
 * @param work
 * The application-defined function to invoke on the target channel. The first
 * parameter passed to this function is the context provided to
 * dispatch_channel_async_f().
 * The result of passing NULL in this parameter is undefined.
 */
API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0), bridgeos(4.0))
DISPATCH_EXPORT DISPATCH_NONNULL1 DISPATCH_NONNULL3 DISPATCH_NOTHROW
void
dispatch_channel_async_f(dispatch_queue_t queue,
		void *_Nullable context, dispatch_function_t work);

/*!
 * @function dispatch_channel_enqueue
 *
 * @abstract
 * Enqueues an opaque work item for asynchronous dequeue on a dispatch channel.
 *
 * @discussion
 * See dispatch_channel_async() for details.
 *
 * @param channel
 * The target dispatch channel to which the work item is submitted.
 * The system will hold a reference on the target channel until the work item
 * is consumed.
 * The result of passing NULL in this parameter is undefined.
 *
 * @param item
 * The application-defined work item to enqueue on the target channel.
 * The result of passing NULL in this parameter is undefined.
 */
API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0), bridgeos(4.0))
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_NOTHROW
void
dispatch_channel_enqueue(dispatch_channel_t channel, void *item);

#endif // DISPATCH_CHANNEL_SPI

__END_DECLS

DISPATCH_ASSUME_NONNULL_END

#endif
