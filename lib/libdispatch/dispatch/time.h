/*
 * Copyright (c) 2008-2011 Apple Inc. All rights reserved.
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

#ifndef __DISPATCH_TIME__
#define __DISPATCH_TIME__

#ifndef __DISPATCH_INDIRECT__
#error "Please #include <dispatch/dispatch.h> instead of this file directly."
#include <dispatch/base.h> // for HeaderDoc
#endif

#include <stdint.h>

// <rdar://problem/6368156&7563559>
#if TARGET_OS_MAC
#include <mach/clock_types.h>
#endif

#ifdef NSEC_PER_SEC
#undef NSEC_PER_SEC
#endif
#ifdef USEC_PER_SEC
#undef USEC_PER_SEC
#endif
#ifdef NSEC_PER_USEC
#undef NSEC_PER_USEC
#endif
#ifdef NSEC_PER_MSEC
#undef NSEC_PER_MSEC
#endif
#define NSEC_PER_SEC 1000000000ull
#define NSEC_PER_MSEC 1000000ull
#define USEC_PER_SEC 1000000ull
#define NSEC_PER_USEC 1000ull

__BEGIN_DECLS

struct timespec;

/*!
 * @typedef dispatch_time_t
 *
 * @abstract
 * A somewhat abstract representation of time; where zero means "now" and
 * DISPATCH_TIME_FOREVER means "infinity" and every value in between is an
 * opaque encoding.
 */
typedef uint64_t dispatch_time_t;

#define DISPATCH_TIME_NOW (0ull)
#define DISPATCH_TIME_FOREVER (~0ull)

/*!
 * @function dispatch_time
 *
 * @abstract
 * Create dispatch_time_t relative to the default clock or modify an existing
 * dispatch_time_t.
 *
 * @discussion
 * On Mac OS X the default clock is based on mach_absolute_time().
 *
 * @param when
 * An optional dispatch_time_t to add nanoseconds to. If zero is passed, then
 * dispatch_time() will use the result of mach_absolute_time().
 *
 * @param delta
 * Nanoseconds to add.
 *
 * @result
 * A new dispatch_time_t.
 */
__OSX_AVAILABLE_STARTING(__MAC_10_6,__IPHONE_4_0)
DISPATCH_EXPORT DISPATCH_WARN_RESULT DISPATCH_NOTHROW
dispatch_time_t
dispatch_time(dispatch_time_t when, int64_t delta);

/*!
 * @function dispatch_walltime
 *
 * @abstract
 * Create a dispatch_time_t using the wall clock.
 *
 * @discussion
 * On Mac OS X the wall clock is based on gettimeofday(3).
 *
 * @param when
 * A struct timespect to add time to. If NULL is passed, then
 * dispatch_walltime() will use the result of gettimeofday(3).
 *
 * @param delta
 * Nanoseconds to add.
 *
 * @result
 * A new dispatch_time_t.
 */
__OSX_AVAILABLE_STARTING(__MAC_10_6,__IPHONE_4_0)
DISPATCH_EXPORT DISPATCH_WARN_RESULT DISPATCH_NOTHROW
dispatch_time_t
dispatch_walltime(const struct timespec *when, int64_t delta);

__END_DECLS

#endif
