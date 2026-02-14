/*
 * Copyright (c) 2018 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

#ifndef __PTHREAD_DEPENDENCY_PRIVATE__
#define __PTHREAD_DEPENDENCY_PRIVATE__

#include <os/base.h>
#include <sys/cdefs.h>
#include <pthread/pthread.h>
#include <Availability.h>

__BEGIN_DECLS

OS_ASSUME_NONNULL_BEGIN

/*!
 * @typedef pthread_dependency_t
 *
 * @abstract
 * A pthread dependency is a one-time dependency between a thread producing
 * a value and a waiter thread, expressed to the system in a way
 * that priority inversion avoidance can be applied if necessary.
 *
 * @discussion
 * These tokens are one-time use, and meant to be on the stack of the waiter
 * thread.
 *
 * These tokens must be both fulfilled and waited on, exactly one of each.
 */
typedef struct pthread_dependency_s {
	uint32_t __pdep_owner;
	uint32_t __pdep_opaque1;
	uint64_t __pdep_opaque2;
} pthread_dependency_t;

/*!
 * @typedef pthread_dependency_attr_t
 *
 * @abstract
 * An opaque type to allow for future expansion of the pthread_dependency
 * interface.
 */
typedef struct pthread_dependency_attr_s pthread_dependency_attr_t;

#if (!defined(_POSIX_C_SOURCE) && !defined(_XOPEN_SOURCE)) || defined(_DARWIN_C_SOURCE) || defined(__cplusplus)
/*!
 * @macro PTHREAD_DEPENDENCY_INITIALIZER_NP
 *
 * @abstract
 * Initialize a one-time dependency token.
 *
 * @param __pthread
 * The thread that will be waited on for this dependency to be fulfilled.
 * It is expected that this thread will call pthread_dependency_fulfill_np().
 */
#define PTHREAD_DEPENDENCY_INITIALIZER_NP(__pthread) \
		{ pthread_mach_thread_np(__pthread), 0, 0 }
#endif

/*!
 * @function pthread_dependency_init_np
 *
 * @abstract
 * Initialize a dependency token.
 *
 * @param __dependency
 * A pointer to a dependency token to initialize.
 *
 * @param __pthread
 * The thread that will be waited on for this dependency to be fulfilled.
 * It is expected that this thread will call pthread_dependency_fulfill_np().
 *
 * @param __attrs
 * This argument is reserved for future expansion purposes, and NULL should be
 * passed.
 */
__API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0))
OS_NONNULL1 OS_NONNULL2 OS_NOTHROW
void pthread_dependency_init_np(pthread_dependency_t *__dependency,
		pthread_t __pthread, pthread_dependency_attr_t *_Nullable __attrs);

/*!
 * @function pthread_dependency_fulfill_np
 *
 * @abstract
 * Fulfill a dependency.
 *
 * @discussion
 * Calling pthread_dependency_fulfill_np() with a token that hasn't been
 * initialized yet, or calling pthread_dependency_fulfill_np() on the same
 * dependency token more than once is undefined and will cause the process
 * to be terminated.
 *
 * The thread that calls pthread_dependency_fulfill_np() must be the same
 * as the pthread_t that was specified when initializing the token. Not doing so
 * is undefined and will cause the process to be terminated.
 *
 * @param __dependency
 * A pointer to a dependency token that was previously initialized.
 *
 * @param __value
 * An optional value that can be returned through the dependency token
 * to the waiter.
 */
__API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0))
OS_NONNULL1 OS_NOTHROW
void pthread_dependency_fulfill_np(pthread_dependency_t *__dependency,
		void * _Nullable __value);

/*!
 * @function pthread_dependency_wait_np
 *
 * @abstract
 * Wait on a dependency.
 *
 * @discussion
 * Calling pthread_dependency_wait_np() with a token that hasn't been
 * initialized yet, or calling pthread_dependency_wait_np() on the same
 * dependency token more than once is undefined and will cause the process
 * to be terminated.
 *
 * If the dependency is not fulfilled yet when this function is called, priority
 * inversion avoidance will be applied to the thread that was specified when
 * initializing the token, to ensure that it can call
 * pthread_dependency_fulfill_np() without causing a priority inversion for the
 * thread calling pthread_dependency_wait_np().
 *
 * @param __dependency
 * A pointer to a dependency token that was previously initialized with
 * PTHREAD_DEPENDENCY_INITIALIZER_NP() or pthread_dependency_init_np().
 *
 * @returns
 * The value that was passed to pthread_dependency_fulfill_np() as the `__value`
 * argument.
 */
__API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0))
OS_NONNULL1 OS_NOTHROW
void *_Nullable pthread_dependency_wait_np(pthread_dependency_t *__dependency);

OS_ASSUME_NONNULL_END

__END_DECLS

#endif //__PTHREAD_DEPENDENCY_PRIVATE__
