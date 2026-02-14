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

#ifndef __DISPATCH_INTROSPECTION_PRIVATE__
#define __DISPATCH_INTROSPECTION_PRIVATE__

/*!
 * @header
 *
 * @abstract
 * Introspection SPI for libdispatch.
 *
 * @discussion
 * This SPI is only available in the introspection version of the library,
 * loaded by running a process with the environment variable
 * DYLD_LIBRARY_PATH=/usr/lib/system/introspection
 *
 * NOTE: most of these functions are _not_ exported from the shared library,
 * the unexported functions are intended to only be called from a debugger
 * context while the rest of the process is suspended.
 */

#ifndef __BEGIN_DECLS
#if defined(__cplusplus)
#define	__BEGIN_DECLS extern "C" {
#define	__END_DECLS }
#else
#define	__BEGIN_DECLS
#define	__END_DECLS
#endif
#endif

__BEGIN_DECLS

#ifndef __DISPATCH_INDIRECT__
/*
 * Typedefs of opaque types, for direct inclusion of header in lldb expressions
 */
typedef __typeof__(sizeof(int)) size_t;
typedef struct _opaque_pthread_t *pthread_t;
typedef void (*dispatch_function_t)(void *);
typedef struct Block_layout *dispatch_block_t;
typedef struct dispatch_continuation_s *dispatch_continuation_t;
typedef struct dispatch_queue_s *dispatch_queue_t;
typedef struct dispatch_source_s *dispatch_source_t;
typedef struct dispatch_group_s *dispatch_group_t;
typedef struct dispatch_object_s *dispatch_object_t;
#ifndef API_AVAILABLE
#define API_AVAILABLE(...)
#endif
#ifndef DISPATCH_EXPORT
#define DISPATCH_EXPORT extern
#endif
#endif // __DISPATCH_INDIRECT__

/*!
 * @typedef dispatch_introspection_versions_s
 *
 * @abstract
 * A structure of version and size information of introspection structures.
 *
 * @field introspection_version
 * Version of overall dispatch_introspection SPI.
 *
 * @field hooks_version
 * Version of dispatch_introspection_hooks_s structure.
 * Version 2 adds the queue_item_complete member.
 *
 * @field hooks_size
 * Size of dispatch_introspection_hooks_s structure.
 *
 * @field queue_item_version
 * Version of dispatch_introspection_queue_item_s structure.
 *
 * @field queue_item_size
 * Size of dispatch_introspection_queue_item_s structure.
 *
 * @field queue_block_version
 * Version of dispatch_introspection_queue_block_s structure.
 *
 * @field queue_block_size
 * Size of dispatch_introspection_queue_block_s structure.
 *
 * @field queue_function_version
 * Version of dispatch_introspection_queue_function_s structure.
 *
 * @field queue_function_size
 * Size of dispatch_introspection_queue_function_s structure.
 *
 * @field queue_thread_version
 * Version of dispatch_introspection_queue_thread_s structure.
 *
 * @field queue_thread_size
 * Size of dispatch_introspection_queue_thread_s structure.
 *
 * @field object_version
 * Version of dispatch_introspection_object_s structure.
 *
 * @field object_size
 * Size of dispatch_introspection_object_s structure.
 *
 * @field queue_version
 * Version of dispatch_introspection_queue_s structure.
 *
 * @field queue_size
 * Size of dispatch_introspection_queue_s structure.
 *
 * @field source_version
 * Version of dispatch_introspection_source_s structure.
 *
 * @field source_size
 * Size of dispatch_introspection_source_s structure.
 */
API_AVAILABLE(macos(10.9), ios(7.0))
DISPATCH_EXPORT const struct dispatch_introspection_versions_s {
	unsigned long introspection_version;
	unsigned long hooks_version;
	size_t hooks_size;
	unsigned long queue_item_version;
	size_t queue_item_size;
	unsigned long queue_block_version;
	size_t queue_block_size;
	unsigned long queue_function_version;
	size_t queue_function_size;
	unsigned long queue_thread_version;
	size_t queue_thread_size;
	unsigned long object_version;
	size_t object_size;
	unsigned long queue_version;
	size_t queue_size;
	unsigned long source_version;
	size_t source_size;
} dispatch_introspection_versions;

/*!
 * @typedef dispatch_introspection_queue_block_s
 *
 * @abstract
 * A structure of introspection information for a block item enqueued on a
 * dispatch queue.
 *
 * @field continuation
 * Pointer to enqueued item.
 *
 * @field target_queue
 * Target queue of item (may be different to the queue the item is currently
 * enqueued on).
 *
 * @field block
 * Block for enqueued item.
 *
 * @field block_invoke
 * Function pointer of block for enqueued item.
 *
 * @field group
 * Group containing enqueued item (may be NULL).
 *
 * @field waiter
 * Thread waiting for completion of enqueued item (NULL if sync == 0).
 *
 * @field barrier
 * Item is a barrier on the queue (all items on serial queues are barriers).
 *
 * @field sync
 * Item was enqueued by a dispatch_sync/dispatch_barrier_sync.
 *
 * @field apply
 * Item is part of a dispatch_apply.
 */
typedef struct dispatch_introspection_queue_block_s {
	dispatch_continuation_t continuation;
	dispatch_queue_t target_queue;
	dispatch_block_t block;
	dispatch_function_t block_invoke;
	dispatch_group_t group;
	pthread_t waiter;
	unsigned long barrier:1,
			sync:1,
			apply:1;
} dispatch_introspection_queue_block_s;
typedef dispatch_introspection_queue_block_s
		*dispatch_introspection_queue_block_t;

/*!
 * @typedef dispatch_introspection_queue_function_s
 *
 * @abstract
 * A structure of introspection information for a function & context pointer
 * item enqueued on a dispatch queue.
 *
 * @field continuation
 * Pointer to enqueued item.
 *
 * @field target_queue
 * Target queue of item (may be different to the queue the item is currently
 * enqueued on).
 *
 * @field context
 * Context in enqueued item.
 *
 * @field block_invoke
 * Function pointer in enqueued item.
 *
 * @field group
 * Group containing enqueued item (may be NULL).
 *
 * @field waiter
 * Thread waiting for completion of enqueued item (NULL if sync == 0).
 *
 * @field barrier
 * Item is a barrier on the queue (all items on serial queues are barriers).
 *
 * @field sync
 * Item was enqueued by a dispatch_sync_f/dispatch_barrier_sync_f.
 *
 * @field apply
 * Item is part of a dispatch_apply_f.
 */
typedef struct dispatch_introspection_queue_function_s {
	dispatch_continuation_t continuation;
	dispatch_queue_t target_queue;
	void *context;
	dispatch_function_t function;
	dispatch_group_t group;
	pthread_t waiter;
	unsigned long barrier:1,
			sync:1,
			apply:1;
} dispatch_introspection_queue_function_s;
typedef dispatch_introspection_queue_function_s
		*dispatch_introspection_queue_function_t;

/*!
 * @typedef dispatch_introspection_object_s
 *
 * @abstract
 * A structure of introspection information for a generic dispatch object.
 *
 * @field object
 * Pointer to object.
 *
 * @field target_queue
 * Target queue of object (may be different to the queue the object is
 * currently enqueued on).
 *
 * @field type
 * Object class pointer.
 *
 * @field kind
 * String describing the object type.
 */
typedef struct dispatch_introspection_object_s {
	dispatch_continuation_t object;
	dispatch_queue_t target_queue;
	void *type;
	const char *kind;
} dispatch_introspection_object_s;
typedef dispatch_introspection_object_s *dispatch_introspection_object_t;

/*!
 * @typedef dispatch_introspection_queue_s
 *
 * @abstract
 * A structure of introspection information for a dispatch queue.
 *
 * @field queue
 * Pointer to queue object.
 *
 * @field target_queue
 * Target queue of queue (may be different to the queue the queue is currently
 * enqueued on). NULL indicates queue is a root queue.
 *
 * @field label
 * Pointer to queue label.
 *
 * @field serialnum
 * Queue serial number (unique per process).
 *
 * @field width
 * Queue width (1: serial queue, UINT_MAX: concurrent queue).
 *
 * @field suspend_count
 * Number of times the queue has been suspended.
 *
 * @field enqueued
 * Queue is enqueued on another queue.
 *
 * @field barrier
 * Queue is executing a barrier item.
 *
 * @field draining
 * Queue is being drained (cannot get queue items).
 *
 * @field global
 * Queue is a global queue.
 *
 * @field main
 * Queue is the main queue.
 */
typedef struct dispatch_introspection_queue_s {
	dispatch_queue_t queue;
	dispatch_queue_t target_queue;
	const char *label;
	unsigned long serialnum;
	unsigned int width;
	unsigned int suspend_count;
	unsigned long enqueued:1,
			barrier:1,
			draining:1,
			global:1,
			main:1;
} dispatch_introspection_queue_s;
typedef dispatch_introspection_queue_s *dispatch_introspection_queue_t;

/*!
 * @typedef dispatch_introspection_source_s
 *
 * @abstract
 * A structure of introspection information for a dispatch source.
 *
 * @field source
 * Pointer to source object.
 *
 * @field target_queue
 * Target queue of source (may be different to the queue the source is currently
 * enqueued on).
 *
 * @field type
 * Source type (kevent filter)
 *
 * @field handle
 * Source handle (monitored entity).
 *
 * @field context
 * Context pointer passed to source handler. Pointer to handler block if
 * handler_is_block == 1.
 *
 * @field handler
 * Source handler function. Function pointer of handler block if
 * handler_is_block == 1.
 *
 * @field suspend_count
 * Number of times the source has been suspended.
 *
 * @field enqueued
 * Source is enqueued on a queue.
 *
 * @field handler_is_block
 * Source handler is a block.
 *
 * @field timer
 * Source is a timer.
 *
 * @field after
 * Source is a dispatch_after timer.
 */
typedef struct dispatch_introspection_source_s {
	dispatch_source_t source;
	dispatch_queue_t target_queue;
	unsigned long type;
	unsigned long handle;
	void *context;
	dispatch_function_t handler;
	unsigned int suspend_count;
	unsigned long enqueued:1,
			handler_is_block:1,
			timer:1,
			after:1,
			is_xpc:1;
} dispatch_introspection_source_s;
typedef dispatch_introspection_source_s *dispatch_introspection_source_t;

/*!
 * @typedef dispatch_introspection_queue_thread_s
 *
 * @abstract
 * A structure of introspection information about a thread executing items for
 * a dispatch queue.
 *
 * @field object
 * Pointer to thread object.
 *
 * @field thread
 * Thread executing items for a queue.
 *
 * @field queue
 * Queue introspection information. The queue.queue field is NULL if this thread
 * is not currently executing items for a queue.
 */
typedef struct dispatch_introspection_queue_thread_s {
	dispatch_continuation_t object;
	pthread_t thread;
	dispatch_introspection_queue_s queue;
} dispatch_introspection_queue_thread_s;
typedef dispatch_introspection_queue_thread_s
		*dispatch_introspection_queue_thread_t;

/*!
 * @enum dispatch_introspection_queue_item_type
 *
 * @abstract
 * Types of items enqueued on a dispatch queue.
 */
enum dispatch_introspection_queue_item_type {
	dispatch_introspection_queue_item_type_none = 0x0,
	dispatch_introspection_queue_item_type_block = 0x11,
	dispatch_introspection_queue_item_type_function = 0x12,
	dispatch_introspection_queue_item_type_object = 0x100,
	dispatch_introspection_queue_item_type_queue = 0x101,
	dispatch_introspection_queue_item_type_source = 0x42,
};

/*!
 * @typedef dispatch_introspection_queue_item_s
 *
 * @abstract
 * A structure of introspection information about an item enqueued on a
 * dispatch queue.
 *
 * @field type
 * Indicates which of the union members applies to this item.
 */
typedef struct dispatch_introspection_queue_item_s {
	unsigned long type; // dispatch_introspection_queue_item_type
	union {
		dispatch_introspection_queue_block_s block;
		dispatch_introspection_queue_function_s function;
		dispatch_introspection_object_s object;
		dispatch_introspection_queue_s queue;
		dispatch_introspection_source_s source;
	};
} dispatch_introspection_queue_item_s;
typedef dispatch_introspection_queue_item_s
		*dispatch_introspection_queue_item_t;

/*!
 * @typedef dispatch_introspection_hook_queue_create_t
 *
 * @abstract
 * A function pointer called when a dispatch queue is created.
 *
 * @param queue_info
 * Pointer to queue introspection structure.
 */
typedef void (*dispatch_introspection_hook_queue_create_t)(
		dispatch_introspection_queue_t queue_info);

/*!
 * @typedef dispatch_introspection_hook_queue_dispose_t
 *
 * @abstract
 * A function pointer called when a dispatch queue is destroyed.
 *
 * @param queue_info
 * Pointer to queue introspection structure.
 */
typedef void (*dispatch_introspection_hook_queue_dispose_t)(
		dispatch_introspection_queue_t queue_info);

/*!
 * @typedef dispatch_introspection_hook_queue_item_enqueue_t
 *
 * @abstract
 * A function pointer called when an item is enqueued onto a dispatch queue.
 *
 * @param queue
 * Pointer to queue.
 *
 * @param item
 * Pointer to item introspection structure.
 */
typedef void (*dispatch_introspection_hook_queue_item_enqueue_t)(
		dispatch_queue_t queue, dispatch_introspection_queue_item_t item);

/*!
 * @typedef dispatch_introspection_hook_queue_item_dequeue_t
 *
 * @abstract
 * A function pointer called when an item is dequeued from a dispatch queue.
 *
 * @param queue
 * Pointer to queue.
 *
 * @param item
 * Pointer to item introspection structure.
 */
typedef void (*dispatch_introspection_hook_queue_item_dequeue_t)(
		dispatch_queue_t queue, dispatch_introspection_queue_item_t item);

/*!
 * @typedef dispatch_introspection_hook_queue_item_complete_t
 *
 * @abstract
 * A function pointer called when an item previously dequeued from a dispatch
 * queue has completed processing.
 *
 * @discussion
 * The object pointer value passed to this function pointer must be treated as a
 * value only. It is intended solely for matching up with an earlier call to a
 * dequeue hook function pointer by comparing to the first member of the
 * dispatch_introspection_queue_item_t structure. It must NOT be dereferenced
 * or e.g. passed to dispatch_introspection_queue_item_get_info(), the memory
 * that was backing it may have been reused at the time this hook is called.
 *
 * @param object
 * Opaque dentifier for completed item. Must NOT be dereferenced.
 */
typedef void (*dispatch_introspection_hook_queue_item_complete_t)(
		dispatch_continuation_t object);

/*!
 * @enum dispatch_introspection_runtime_event
 *
 * @abstract
 * Types for major events the dispatch runtime goes through as sent by
 * the runtime_event hook.
 *
 * @const dispatch_introspection_runtime_event_worker_event_delivery
 * A worker thread was unparked to deliver some kernel events.
 * There may be an unpark event if the thread will pick up a queue to drain.
 * There always is a worker_park event when the thread is returned to the pool.
 * `ptr` is the queue for which events are being delivered, or NULL (for generic
 * events).
 * `value` is the number of events delivered.
 *
 * @const dispatch_introspection_runtime_event_worker_unpark
 * A worker thread junst unparked (sent from the context of the thread).
 * `ptr` is the queue for which the thread unparked.
 * `value` is 0.
 *
 * @const dispatch_introspection_runtime_event_worker_request
 * `ptr` is set to the queue on behalf of which the thread request is made.
 * `value` is the number of threads requested.
 *
 * @const dispatch_introspection_runtime_event_worker_park
 * A worker thread is about to park (sent from the context of the thread).
 * `ptr` and `value` are 0.
 *
 * @const dispatch_introspection_runtime_event_sync_wait
 * A caller of dispatch_sync or dispatch_async_and_wait hit contention.
 * `ptr` is the queue that caused the initial contention.
 * `value` is 0.
 *
 * @const dispatch_introspection_runtime_event_async_sync_handoff
 * @const dispatch_introspection_runtime_event_sync_sync_handoff
 * @const dispatch_introspection_runtime_event_sync_async_handoff
 *
 * A queue is being handed off from a thread to another due to respectively:
 * - async/sync contention
 * - sync/sync contention
 * - sync/async contention
 *
 * `ptr` is set to dispatch_queue_t which is handed off to the next thread.
 * `value` is 0.
 */
#ifndef __DISPATCH_BUILDING_DISPATCH__
enum dispatch_introspection_runtime_event {
	dispatch_introspection_runtime_event_worker_event_delivery = 1,
	dispatch_introspection_runtime_event_worker_unpark = 2,
	dispatch_introspection_runtime_event_worker_request = 3,
	dispatch_introspection_runtime_event_worker_park = 4,

	dispatch_introspection_runtime_event_sync_wait = 10,
	dispatch_introspection_runtime_event_async_sync_handoff = 11,
	dispatch_introspection_runtime_event_sync_sync_handoff = 12,
	dispatch_introspection_runtime_event_sync_async_handoff = 13,
};
#endif

/*!
 * @typedef dispatch_introspection_hook_runtime_event_t
 *
 * @abstract
 * A function pointer called for various runtime events.
 *
 * @discussion
 * The actual payloads are discussed in the documentation of the
 * dispatch_introspection_runtime_event enum.
 */
typedef void (*dispatch_introspection_hook_runtime_event_t)(
		enum dispatch_introspection_runtime_event event,
		void *ptr, unsigned long long value);

/*!
 * @typedef dispatch_introspection_hooks_s
 *
 * @abstract
 * A structure of function pointer hooks into libdispatch.
 */
typedef struct dispatch_introspection_hooks_s {
	dispatch_introspection_hook_queue_create_t queue_create;
	dispatch_introspection_hook_queue_dispose_t queue_dispose;
	dispatch_introspection_hook_queue_item_enqueue_t queue_item_enqueue;
	dispatch_introspection_hook_queue_item_dequeue_t queue_item_dequeue;
	dispatch_introspection_hook_queue_item_complete_t queue_item_complete;
	dispatch_introspection_hook_runtime_event_t runtime_event;
	void *_reserved[4];
} dispatch_introspection_hooks_s;
typedef dispatch_introspection_hooks_s *dispatch_introspection_hooks_t;

/*!
 * @function dispatch_introspection_get_queues
 *
 * @abstract
 * Retrieve introspection information about all dispatch queues in the process,
 * in batches of specified size.
 *
 * @discussion
 * Retrieving queue information and iterating through the list of all queues
 * must take place from a debugger context (while the rest of the process is
 * suspended).
 *
 * @param start
 * Starting point for this batch of queue information, as returned by a previous
 * call to _dispatch_introspection_get_queues().
 * Pass NULL to retrieve the initial batch.
 *
 * @param count
 * Number of queues to introspect.
 *
 * @param queues
 * Array to fill with queue information. If less than 'count' queues are left
 * in this batch, the end of valid entries in the array will be indicated
 * by an entry with NULL queue member.
 *
 * @result
 * Queue to pass to another call to _dispatch_introspection_get_queues() to
 * retrieve information about the next batch of queues. May be NULL if there
 * are no more queues to iterate over.
 */
extern dispatch_queue_t
dispatch_introspection_get_queues(dispatch_queue_t start, size_t count,
		dispatch_introspection_queue_t queues);

/*!
 * @function dispatch_introspection_get_queue_threads
 *
 * @abstract
 * Retrieve introspection information about all threads in the process executing
 * items for dispatch queues, in batches of specified size.
 *
 * @discussion
 * Retrieving thread information and iterating through the list of all queue
 * threads must take place from a debugger context (while the rest of the
 * process is suspended).
 *
 * @param start
 * Starting point for this batch of thread information, as returned by a
 * previous call to _dispatch_introspection_get_queue_threads().
 * Pass NULL to retrieve the initial batch.
 *
 * @param count
 * Number of queue threads to introspect.
 *
 * @param threads
 * Array to fill with queue thread information. If less than 'count' threads are
 * left in this batch, the end of valid entries in the array will be indicated
 * by an entry with NULL object member.
 *
 * @result
 * Object to pass to another call to _dispatch_introspection_get_queues() to
 * retrieve information about the next batch of queues. May be NULL if there
 * are no more queues to iterate over.
 */
extern dispatch_continuation_t
dispatch_introspection_get_queue_threads(dispatch_continuation_t start,
		size_t count, dispatch_introspection_queue_thread_t threads);

/*!
 * @function dispatch_introspection_queue_get_items
 *
 * @abstract
 * Retrieve introspection information about all items enqueued on a queue, in
 * batches of specified size.
 *
 * @discussion
 * Retrieving queue item information and iterating through a queue must take
 * place from a debugger context (while the rest of the process is suspended).
 *
 * @param queue
 * Queue to introspect.
 *
 * @param start
 * Starting point for this batch of queue item information, as returned by a
 * previous call to _dispatch_introspection_queue_get_items().
 * Pass NULL to retrieve the initial batch.
 *
 * @param count
 * Number of items to introspect.
 *
 * @param items
 * Array to fill with queue item information. If less than 'count' queues are
 * left in this batch, the end of valid entries in the array will be indicated
 * by an entry with type dispatch_introspection_queue_item_type_none.
 *
 * @result
 * Item to pass to another call to _dispatch_introspection_queue_get_items() to
 * retrieve information about the next batch of queue items. May be NULL if
 * there are no more items to iterate over.
 */
extern dispatch_continuation_t
dispatch_introspection_queue_get_items(dispatch_queue_t queue,
		dispatch_continuation_t start, size_t count,
		dispatch_introspection_queue_item_t items);

/*!
 * @function dispatch_introspection_queue_get_info
 *
 * @abstract
 * Retrieve introspection information about a specified dispatch queue.
 *
 * @discussion
 * Retrieving queue information must take place from a debugger context (while
 * the rest of the process is suspended).
 *
 * @param queue
 * Queue to introspect.
 *
 * @result
 * Queue information struct.
 */
extern dispatch_introspection_queue_s
dispatch_introspection_queue_get_info(dispatch_queue_t queue);

/*!
 * @function dispatch_introspection_queue_item_get_info
 *
 * @abstract
 * Retrieve introspection information about a specified dispatch queue item.
 *
 * @discussion
 * Retrieving queue item information must take place from a debugger context
 * (while the rest of the process is suspended).
 *
 * @param queue
 * Queue to introspect.
 *
 * @param item
 * Item to introspect.
 *
 * @result
 * Queue item information struct.
 */
extern dispatch_introspection_queue_item_s
dispatch_introspection_queue_item_get_info(dispatch_queue_t queue,
		dispatch_continuation_t item);

/*!
 * @function dispatch_introspection_hooks_install
 *
 * @abstract
 * Install hook functions into libdispatch.
 *
 * @discussion
 * Installing hook functions must take place from a debugger context (while the
 * rest of the process is suspended) or early enough in the process lifecycle
 * that the process is still single-threaded.
 *
 * The caller is responsible for implementing chaining to the hooks that were
 * previously installed (if any).
 *
 * @param hooks
 * Pointer to structure of hook function pointers. Any of the structure members
 * may be NULL to indicate that the hook in question should not be installed.
 * The structure is copied on input and filled with the previously installed
 * hooks on output.
 */
API_AVAILABLE(macos(10.9), ios(7.0))
DISPATCH_EXPORT void
dispatch_introspection_hooks_install(dispatch_introspection_hooks_t hooks);

/*!
 * @function dispatch_introspection_hook_callouts_enable
 *
 * @abstract
 * Enable hook callout functions in libdispatch that a debugger can break on
 * and get introspection arguments even if there are no hook functions
 * installed via dispatch_introspection_hooks_install().
 *
 * @discussion
 * Enabling hook callout functions must take place from a debugger context
 * (while the rest of the process is suspended).
 *
 * @param enable
 * Pointer to dispatch_introspection_hooks_s structure. For every structure
 * member with (any) non-NULL value, the corresponding hook callout will be
 * enabled; for every NULL member the hook callout will be disabled (if there
 * is no hook function installed).
 * As a convenience, the 'enable' pointer may itself be NULL to indicate that
 * all hook callouts should be enabled.
 */
extern void
dispatch_introspection_hook_callouts_enable(
		dispatch_introspection_hooks_t enable);

/*!
 * @function dispatch_introspection_hook_callout_queue_create
 *
 * @abstract
 * Callout to queue creation hook that a debugger can break on.
 */
extern void
dispatch_introspection_hook_callout_queue_create(
		dispatch_introspection_queue_t queue_info);

/*!
 * @function dispatch_introspection_hook_callout_queue_dispose
 *
 * @abstract
 * Callout to queue destruction hook that a debugger can break on.
 */
extern void
dispatch_introspection_hook_callout_queue_dispose(
		dispatch_introspection_queue_t queue_info);

/*!
 * @function dispatch_introspection_hook_callout_queue_item_enqueue
 *
 * @abstract
 * Callout to queue enqueue hook that a debugger can break on.
 */
extern void
dispatch_introspection_hook_callout_queue_item_enqueue(
		dispatch_queue_t queue, dispatch_introspection_queue_item_t item);

/*!
 * @function dispatch_introspection_hook_callout_queue_item_dequeue
 *
 * @abstract
 * Callout to queue dequeue hook that a debugger can break on.
 */
extern void
dispatch_introspection_hook_callout_queue_item_dequeue(
		dispatch_queue_t queue, dispatch_introspection_queue_item_t item);

/*!
 * @function dispatch_introspection_hook_callout_queue_item_complete
 *
 * @abstract
 * Callout to queue item complete hook that a debugger can break on.
 */
extern void
dispatch_introspection_hook_callout_queue_item_complete(
		dispatch_continuation_t object);

__END_DECLS

#endif
