/*
 * Copyright (c) 2013 Apple Inc. All rights reserved.
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

#ifndef __DISPATCH_INTROSPECTION__
#define __DISPATCH_INTROSPECTION__

#include <dispatch/dispatch.h>

/*!
 * @header
 *
 * @abstract
 * Interposable introspection hooks for libdispatch.
 *
 * @discussion
 * These hooks are only available in the introspection version of the library,
 * loaded by running a process with the environment variable
 * DYLD_LIBRARY_PATH=/usr/lib/system/introspection
 */

__BEGIN_DECLS

/*!
 * @function dispatch_introspection_hook_queue_create
 *
 * @abstract
 * Interposable hook function called when a dispatch queue was created.
 *
 * @param queue
 * The newly created dispatch queue.
 */

__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0)
DISPATCH_EXPORT
void
dispatch_introspection_hook_queue_create(dispatch_queue_t queue);

/*!
 * @function dispatch_introspection_hook_queue_destroy
 *
 * @abstract
 * Interposable hook function called when a dispatch queue is about to be
 * destroyed.
 *
 * @param queue
 * The dispatch queue about to be destroyed.
 */

__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0)
DISPATCH_EXPORT
void
dispatch_introspection_hook_queue_destroy(dispatch_queue_t queue);

/*!
 * @function dispatch_introspection_hook_queue_item_enqueue
 *
 * @abstract
 * Interposable hook function called when an item is about to be enqueued onto
 * a dispatch queue.
 *
 * @param queue
 * The dispatch queue enqueued onto.
 *
 * @param item
 * The object about to be enqueued.
 */

__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0)
DISPATCH_EXPORT
void
dispatch_introspection_hook_queue_item_enqueue(dispatch_queue_t queue,
		dispatch_object_t item);

/*!
 * @function dispatch_introspection_hook_queue_item_dequeue
 *
 * @abstract
 * Interposable hook function called when an item was dequeued from a dispatch
 * queue.
 *
 * @param queue
 * The dispatch queue dequeued from.
 *
 * @param item
 * The dequeued object.
 */

__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0)
DISPATCH_EXPORT
void
dispatch_introspection_hook_queue_item_dequeue(dispatch_queue_t queue,
		dispatch_object_t item);

/*!
 * @function dispatch_introspection_hook_queue_item_complete
 *
 * @abstract
 * Interposable hook function called when an item previously dequeued from a
 * dispatch queue has completed processing.
 *
 * @discussion
 * The object pointer value passed to this function must be treated as a value
 * only. It is intended solely for matching up with an earlier call to a
 * dequeue hook function and must NOT be dereferenced.
 *
 * @param item
 * Opaque dentifier for completed item. Must NOT be dereferenced.
 */

__OSX_AVAILABLE_STARTING(__MAC_10_10,__IPHONE_7_1)
DISPATCH_EXPORT
void
dispatch_introspection_hook_queue_item_complete(dispatch_object_t item);

/*!
 * @function dispatch_introspection_hook_queue_callout_begin
 *
 * @abstract
 * Interposable hook function called when a client function is about to be
 * called out to on a dispatch queue.
 *
 * @param queue
 * The dispatch queue the callout is performed on.
 *
 * @param context
 * The context parameter passed to the function. For a callout to a block,
 * this is a pointer to the block object.
 *
 * @param function
 * The client function about to be called out to. For a callout to a block,
 * this is the block object's invoke function.
 */

__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0)
DISPATCH_EXPORT
void
dispatch_introspection_hook_queue_callout_begin(dispatch_queue_t queue,
		void *context, dispatch_function_t function);

/*!
 * @function dispatch_introspection_hook_queue_callout_end
 *
 * @abstract
 * Interposable hook function called after a client function has returned from
 * a callout on a dispatch queue.
 *
 * @param queue
 * The dispatch queue the callout was performed on.
 *
 * @param context
 * The context parameter passed to the function. For a callout to a block,
 * this is a pointer to the block object.
 *
 * @param function
 * The client function that was called out to. For a callout to a block,
 * this is the block object's invoke function.
 */

__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0)
DISPATCH_EXPORT
void
dispatch_introspection_hook_queue_callout_end(dispatch_queue_t queue,
		void *context, dispatch_function_t function);

__END_DECLS

#endif
