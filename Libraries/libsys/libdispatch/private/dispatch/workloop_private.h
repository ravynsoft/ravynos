/*
 * Copyright (c) 2017-2018 Apple Inc. All rights reserved.
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

#ifndef __DISPATCH_WORKLOOP_PRIVATE__
#define __DISPATCH_WORKLOOP_PRIVATE__

#ifndef __DISPATCH_INDIRECT__
#error "Please #include <dispatch/private.h> instead of this file directly."
#include <dispatch/base.h> // for HeaderDoc
#endif

/******************************************************************************\
 *
 * THIS FILE IS AN IN-PROGRESS INTERFACE THAT IS SUBJECT TO CHANGE
 *
\******************************************************************************/

DISPATCH_ASSUME_NONNULL_BEGIN

__BEGIN_DECLS

DISPATCH_OPTIONS(dispatch_workloop_param_flags, uint64_t,
	DISPATCH_WORKLOOP_NONE DISPATCH_ENUM_API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0)) = 0x0,
	DISPATCH_WORKLOOP_FIXED_PRIORITY DISPATCH_ENUM_API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0)) = 0x1,
);

/*!
 * @function dispatch_workloop_set_qos_class_floor
 *
 * @abstract
 * Sets the QOS class floor of a workloop.
 *
 * @discussion
 * See dispatch_set_qos_class_floor().
 *
 * This function is strictly equivalent to dispatch_set_qos_class_floor() but
 * allows to pass extra flags.
 *
 * Using both dispatch_workloop_set_scheduler_priority() and
 * dispatch_set_qos_class_floor() or dispatch_workloop_set_qos_class_floor()
 * is undefined and will cause the process to be terminated.
 *
 * @param workloop
 * The dispatch workloop to modify.
 *
 * This workloop must be inactive, passing an activated object is undefined
 * and will cause the process to be terminated.
 */
API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0))
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_NOTHROW
void
dispatch_workloop_set_qos_class_floor(dispatch_workloop_t workloop,
		qos_class_t qos, int relpri, dispatch_workloop_param_flags_t flags);

/*!
 * @function dispatch_workloop_set_scheduler_priority
 *
 * @abstract
 * Sets the scheduler priority for a dispatch workloop.
 *
 * @discussion
 * This sets the scheduler priority of the threads that the runtime will bring
 * up to service this workloop.
 *
 * QOS propagation still functions on these workloops, but its effect on the
 * priority of the thread brought up to service this workloop is ignored.
 *
 * Using both dispatch_workloop_set_scheduler_priority() and
 * dispatch_set_qos_class_floor() or dispatch_workloop_set_qos_class_floor()
 * is undefined and will cause the process to be terminated.
 *
 * @param workloop
 * The dispatch workloop to modify.
 *
 * This workloop must be inactive, passing an activated object is undefined
 * and will cause the process to be terminated.
 */
API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0))
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_NOTHROW
void
dispatch_workloop_set_scheduler_priority(dispatch_workloop_t workloop,
		int priority, dispatch_workloop_param_flags_t flags);

/*!
 * @function dispatch_workloop_set_cpupercent
 *
 * @abstract
 * Sets the cpu percent and refill attributes for a dispatch workloop.
 *
 * @discussion
 * This should only used if the workloop was also setup with the
 * DISPATCH_WORKLOOP_FIXED_PRIORITY flag as a safe guard against
 * busy loops that could starve the rest of the system forever.
 *
 * If DISPATCH_WORKLOOP_FIXED_PRIORITY wasn't passed, using this function is
 * undefined and will cause the process to be terminated.
 *
 * @param workloop
 * The dispatch workloop to modify.
 *
 * This workloop must be inactive, passing an activated object is undefined
 * and will cause the process to be terminated.
 */
API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0))
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_NOTHROW
void
dispatch_workloop_set_cpupercent(dispatch_workloop_t workloop, uint8_t percent,
		uint32_t refillms);

/*!
 * @function dispatch_workloop_is_current()
 *
 * @abstract
 * Returns whether the current thread has been made by the runtime to service
 * this workloop.
 *
 * @discussion
 * Note that when using <code>dispatch_async_and_wait(workloop, ^{ ... })</code>
 * then <code>workloop</code> will be seen as the "current" one by the submitted
 * workitem, but that is not the case when using dispatch_sync() on a queue
 * targeting the workloop.
 */
API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0))
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_NOTHROW
bool
dispatch_workloop_is_current(dispatch_workloop_t workloop);

/*!
 * @function dispatch_workloop_copy_current()
 *
 * @abstract
 * Returns a copy of the workoop that is being serviced on the calling thread
 * if any.
 *
 * @discussion
 * If the thread is not a workqueue thread, or is not servicing a dispatch
 * workloop, then NULL is returned.
 *
 * This returns a retained object that must be released with dispatch_release().
 */
API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0))
DISPATCH_EXPORT DISPATCH_RETURNS_RETAINED DISPATCH_NOTHROW
dispatch_workloop_t _Nullable
dispatch_workloop_copy_current(void);

// Equivalent to dispatch_workloop_set_qos_class_floor(workoop, qos, 0, flags)
API_DEPRECATED_WITH_REPLACEMENT("dispatch_workloop_set_qos_class_floor",
		macos(10.14,10.14), ios(12.0,12.0), tvos(12.0,12.0), watchos(5.0,5.0))
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_NOTHROW
void
dispatch_workloop_set_qos_class(dispatch_workloop_t workloop,
		qos_class_t qos, dispatch_workloop_param_flags_t flags);

API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0))
DISPATCH_EXPORT DISPATCH_NOTHROW
bool
_dispatch_workloop_should_yield_4NW(void);


__END_DECLS

DISPATCH_ASSUME_NONNULL_END

#endif
