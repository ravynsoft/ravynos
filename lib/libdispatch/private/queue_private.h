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

#ifndef __DISPATCH_QUEUE_PRIVATE__
#define __DISPATCH_QUEUE_PRIVATE__

#ifndef __DISPATCH_INDIRECT__
#error "Please #include <dispatch/private.h> instead of this file directly."
#include <dispatch/base.h> // for HeaderDoc
#endif

__BEGIN_DECLS

/*!
 * @enum dispatch_queue_flags_t
 *
 * @constant DISPATCH_QUEUE_OVERCOMMIT
 * The queue will create a new thread for invoking blocks, regardless of how
 * busy the computer is.
 */
enum {
	DISPATCH_QUEUE_OVERCOMMIT = 0x2ull,
};

#define DISPATCH_QUEUE_FLAGS_MASK (DISPATCH_QUEUE_OVERCOMMIT)

/*!
 * @function dispatch_queue_attr_make_with_overcommit
 *
 * @discussion
 * Returns a dispatch queue attribute value with the overcommit flag set to the
 * specified value.
 *
 * @param attr
 * A queue attribute value to be combined with the overcommit flag, or NULL.
 *
 * @param overcommit
 * Boolean overcommit flag.
 *
 * @return
 * Returns an attribute value which may be provided to dispatch_queue_create().
 * This new value combines the attributes specified by the 'attr' parameter and
 * the overcommit flag.
 */
__OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_8_0)
DISPATCH_EXPORT DISPATCH_WARN_RESULT DISPATCH_PURE DISPATCH_NOTHROW
dispatch_queue_attr_t
dispatch_queue_attr_make_with_overcommit(dispatch_queue_attr_t attr,
		bool overcommit);

/*!
 * @typedef dispatch_queue_priority_t
 *
 * @constant DISPATCH_QUEUE_PRIORITY_NON_INTERACTIVE
 * Items dispatched to the queue will run at non-interactive priority.
 * This priority level is intended for user-initiated application activity that
 * is long-running and CPU or IO intensive and that the user is actively waiting
 * on, but that should not interfere with interactive use of the application.
 */
#define DISPATCH_QUEUE_PRIORITY_NON_INTERACTIVE INT8_MIN

/*!
 * @function dispatch_queue_set_width
 *
 * @abstract
 * Set the width of concurrency for a given queue. The width of a serial queue
 * is one.
 *
 * @discussion
 * This SPI is DEPRECATED and will be removed in a future release.
 * Uses of this SPI to make a queue concurrent by setting its width to LONG_MAX
 * should be replaced by passing DISPATCH_QUEUE_CONCURRENT to
 * dispatch_queue_create().
 * Uses of this SPI to limit queue concurrency are not recommended and should
 * be replaced by alternative mechanisms such as a dispatch semaphore created
 * with the desired concurrency width.
 *
 * @param queue
 * The queue to adjust. Passing the main queue or a global concurrent queue
 * will be ignored.
 *
 * @param width
 * The new maximum width of concurrency depending on available resources.
 * If zero is passed, then the value is promoted to one.
 * Negative values are magic values that map to automatic width values.
 * Unknown negative values default to DISPATCH_QUEUE_WIDTH_MAX_LOGICAL_CPUS.
 */
#define DISPATCH_QUEUE_WIDTH_ACTIVE_CPUS		-1
#define DISPATCH_QUEUE_WIDTH_MAX_PHYSICAL_CPUS	-2
#define DISPATCH_QUEUE_WIDTH_MAX_LOGICAL_CPUS	-3

__OSX_AVAILABLE_BUT_DEPRECATED_MSG(__MAC_10_6,__MAC_10_10,__IPHONE_4_0,__IPHONE_8_0, \
		"Use dispatch_queue_create(name, DISPATCH_QUEUE_CONCURRENT) instead")
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_NOTHROW
void
dispatch_queue_set_width(dispatch_queue_t dq, long width);

/*!
 * @function dispatch_queue_create_with_target
 *
 * @abstract
 * Creates a new dispatch queue with a specified target queue.
 *
 * @discussion
 * Dispatch queues created with the DISPATCH_QUEUE_SERIAL or a NULL attribute
 * invoke blocks serially in FIFO order.
 *
 * Dispatch queues created with the DISPATCH_QUEUE_CONCURRENT attribute may
 * invoke blocks concurrently (similarly to the global concurrent queues, but
 * potentially with more overhead), and support barrier blocks submitted with
 * the dispatch barrier API, which e.g. enables the implementation of efficient
 * reader-writer schemes.
 *
 * When a dispatch queue is no longer needed, it should be released with
 * dispatch_release(). Note that any pending blocks submitted to a queue will
 * hold a reference to that queue. Therefore a queue will not be deallocated
 * until all pending blocks have finished.
 *
 * @param label
 * A string label to attach to the queue.
 * This parameter is optional and may be NULL.
 *
 * @param attr
 * DISPATCH_QUEUE_SERIAL, DISPATCH_QUEUE_CONCURRENT, or the result of a call to
 * the function dispatch_queue_attr_make_with_qos_class().
 *
 * @param target
 * The target queue for the newly created queue. The target queue is retained.
 * If this parameter is DISPATCH_TARGET_QUEUE_DEFAULT, sets the queue's target
 * queue to the default target queue for the given queue type.
 *
 * @result
 * The newly created dispatch queue.
 */
__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0)
DISPATCH_EXPORT DISPATCH_MALLOC DISPATCH_RETURNS_RETAINED DISPATCH_WARN_RESULT
DISPATCH_NOTHROW
dispatch_queue_t
dispatch_queue_create_with_target(const char *label,
	dispatch_queue_attr_t attr, dispatch_queue_t target);

#ifdef __BLOCKS__
/*!
 * @function dispatch_pthread_root_queue_create
 *
 * @abstract
 * Creates a new concurrent dispatch root queue with a pthread-based pool of
 * worker threads owned by the application.
 *
 * @discussion
 * Dispatch pthread root queues are similar to the global concurrent dispatch
 * queues in that they invoke blocks concurrently, however the blocks are not
 * executed on ordinary worker threads but use a dedicated pool of pthreads not
 * shared with the global queues or any other pthread root queues.
 *
 * NOTE: this is a special-purpose facility that should only be used in very
 * limited circumstances, in almost all cases the global concurrent queues
 * should be preferred. While this facility allows for more flexibility in
 * configuring worker threads for special needs it comes at the cost of
 * increased overall memory usage due to reduced thread sharing and higher
 * latency in worker thread bringup.
 *
 * Dispatch pthread root queues do not support suspension, application context
 * and change of width or of target queue. They can however be used as the
 * target queue for serial or concurrent queues obtained via
 * dispatch_queue_create() or dispatch_queue_create_with_target(), which
 * enables the blocks submitted to those queues to be processed on the root
 * queue's pthread pool.
 *
 * When a dispatch pthread root queue is no longer needed, it should be
 * released with dispatch_release(). Existing worker pthreads and pending blocks
 * submitted to the root queue will hold a reference to the queue so it will not
 * be deallocated until all blocks have finished and worker threads exited.
 *
 * @param label
 * A string label to attach to the queue.
 * This parameter is optional and may be NULL.
 *
 * @param flags
 * Pass flags value returned by dispatch_pthread_root_queue_flags_pool_size()
 * or 0 if unused.
 *
 * @param attr
 * Attributes passed to pthread_create(3) when creating worker pthreads. This
 * parameter is copied and can be destroyed after this call returns.
 * This parameter is optional and may be NULL.
 *
 * @param configure
 * Configuration block called on newly created worker pthreads before any blocks
 * for the root queue are executed. The block may configure the current thread
 * as needed.
 * This parameter is optional and may be NULL.
 *
 * @result
 * The newly created dispatch pthread root queue.
 */
__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_6_0)
DISPATCH_EXPORT DISPATCH_MALLOC DISPATCH_RETURNS_RETAINED DISPATCH_WARN_RESULT
DISPATCH_NOTHROW
dispatch_queue_t
dispatch_pthread_root_queue_create(const char *label, unsigned long flags,
	const pthread_attr_t *attr, dispatch_block_t configure);

/*!
 * @function dispatch_pthread_root_queue_flags_pool_size
 *
 * @abstract
 * Returns flags argument to pass to dispatch_pthread_root_queue_create() to
 * specify the maximum size of the pthread pool to use for a pthread root queue.
 *
 * @param pool_size
 * Maximum size of the pthread pool to use for the root queue. The number of
 * pthreads created for this root queue will never exceed this number but there
 * is no guarantee that the specified number will be reached.
 * Pass 0 to specify that a default pool size determined by the system should
 * be used.
 *
 * @result
 * The flags argument to pass to dispatch_pthread_root_queue_create().
 */
DISPATCH_INLINE DISPATCH_ALWAYS_INLINE
unsigned long
dispatch_pthread_root_queue_flags_pool_size(uint8_t pool_size)
{
	#define _DISPATCH_PTHREAD_ROOT_QUEUE_FLAG_POOL_SIZE (0x80000000ul)
	return (_DISPATCH_PTHREAD_ROOT_QUEUE_FLAG_POOL_SIZE |
			(unsigned long)pool_size);
}

#endif /* __BLOCKS__ */

/*!
 * @constant DISPATCH_APPLY_CURRENT_ROOT_QUEUE
 * @discussion Constant to pass to the dispatch_apply() and dispatch_apply_f()
 * functions to indicate that the root queue for the current thread should be
 * used (i.e. one of the global concurrent queues or a queue created with
 * dispatch_pthread_root_queue_create()). If there is no such queue, the
 * default priority global concurrent queue will be used.
 */
#define DISPATCH_APPLY_CURRENT_ROOT_QUEUE NULL

/*!
 * @function dispatch_assert_queue
 *
 * @abstract
 * Verifies that the current block is executing on a certain dispatch queue.
 *
 * @discussion
 * Some code expects to be run on a specific dispatch queue. This function
 * verifies that expectation for debugging.
 *
 * This function will only return if the currently executing block was submitted
 * to the specified queue or to any queue targeting it (see
 * dispatch_set_target_queue()). Otherwise, it logs an explanation to the system
 * log, then terminates the application.
 *
 * When dispatch_assert_queue() is called outside of the context of a
 * submitted block, its behavior is undefined.
 *
 * Passing the result of dispatch_get_main_queue() to this function verifies
 * that the current block was submitted to the main queue or to a queue
 * targeting it.
 * IMPORTANT: this is NOT the same as verifying that the current block is
 * executing on the main thread.
 *
 * The variant dispatch_assert_queue_debug() is compiled out when the
 * preprocessor macro NDEBUG is defined. (See also assert(3)).
 *
 * @param queue
 * The dispatch queue that the current block is expected to run on.
 * The result of passing NULL in this parameter is undefined.
 */
__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0)
DISPATCH_EXPORT DISPATCH_NONNULL1
void
dispatch_assert_queue(dispatch_queue_t queue);

/*!
 * @function dispatch_assert_queue_not
 *
 * @abstract
 * Verifies that the current block is not executing on a certain dispatch queue.
 *
 * @discussion
 * This function is the equivalent of dispatch_queue_assert() with the test for
 * equality inverted. See discussion there.
 *
 * The variant dispatch_assert_queue_not_debug() is compiled out when the
 * preprocessor macro NDEBUG is defined. (See also assert(3)).
 *
 * @param queue
 * The dispatch queue that the current block is expected not to run on.
 * The result of passing NULL in this parameter is undefined.
 */
__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0)
DISPATCH_EXPORT DISPATCH_NONNULL1
void
dispatch_assert_queue_not(dispatch_queue_t queue);

#ifdef NDEBUG
#define dispatch_assert_queue_debug(q) ((void)0)
#define dispatch_assert_queue_not_debug(q) ((void)0)
#else
#define dispatch_assert_queue_debug(q) dispatch_assert_queue(q)
#define dispatch_assert_queue_not_debug(q) dispatch_assert_queue_not(q)
#endif

__END_DECLS

#endif
