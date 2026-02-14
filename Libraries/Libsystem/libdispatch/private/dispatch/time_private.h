/*
 * Copyright (c) 20017 Apple Inc. All rights reserved.
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
 * which are subject to change in future releases. Any applications relying on
 * these interfaces WILL break.
 */

#ifndef __DISPATCH_TIME_PRIVATE__
#define __DISPATCH_TIME_PRIVATE__

#ifndef __DISPATCH_INDIRECT__
#error "Please #include <dispatch/private.h> instead of this file directly."
#include <dispatch/base.h> // for HeaderDoc
#endif

/*
 * @constant DISPATCH_MONOTONICTIME_NOW
 * A dispatch_time_t value that corresponds to the current value of the
 * platform's monotonic clock. On Apple platforms, this clock is based on
 * mach_continuous_time(). Use this value with the dispatch_time() function to
 * derive a time value for a timer in monotonic time (i.e. a timer that
 * continues to tick while the system is asleep). For example:
 *
 * dispatch_time_t t = dispatch_time(DISPATCH_MONOTONICTIME_NOW,5*NSEC_PER_SEC);
 * dispatch_source_t ds = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER,
 *			0, 0, q);
 * dispatch_source_set_event_handler(ds, ^{ ...  });
 * dispatch_source_set_timer(ds, t, 10 * NSEC_PER_SEC, 0);
 * dispatch_activate(ds);
 */
enum {
	DISPATCH_MONOTONICTIME_NOW DISPATCH_ENUM_API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0)) = (1ull << 63)
};

#ifdef __APPLE__

// Helper macros for up time, montonic time and wall time.
#define _dispatch_uptime_after_nsec(t) \
		dispatch_time(DISPATCH_TIME_NOW, (t))
#define _dispatch_uptime_after_usec(t) \
		dispatch_time(DISPATCH_TIME_NOW, (t) * NSEC_PER_USEC)
#define _dispatch_uptime_after_msec(t) \
		dispatch_time(DISPATCH_TIME_NOW, (t) * NSEC_PER_MSEC)
#define _dispatch_uptime_after_sec(t) \
		dispatch_time(DISPATCH_TIME_NOW, (t) * NSEC_PER_SEC)

#define _dispatch_monotonictime_after_nsec(t) \
		dispatch_time(DISPATCH_MONOTONICTIME_NOW, (t))
#define _dispatch_monotonictime_after_usec(t) \
		dispatch_time(DISPATCH_MONOTONICTIME_NOW, (t) * NSEC_PER_USEC)
#define _dispatch_monotonictime_after_msec(t) \
		dispatch_time(DISPATCH_MONOTONICTIME_NOW, (t) * NSEC_PER_MSEC)
#define _dispatch_monotonictime_after_sec(t) \
		dispatch_time(DISPATCH_MONOTONICTIME_NOW, (t) * NSEC_PER_SEC)

#define _dispatch_walltime_after_nsec(t) \
		dispatch_time(DISPATCH_WALLTIME_NOW, (t))
#define _dispatch_walltime_after_usec(t) \
		dispatch_time(DISPATCH_WALLTIME_NOW, (t) * NSEC_PER_USEC)
#define _dispatch_walltime_after_msec(t) \
		dispatch_time(DISPATCH_WALLTIME_NOW, (t) * NSEC_PER_MSEC)
#define _dispatch_walltime_after_sec(t) \
		dispatch_time(DISPATCH_WALLTIME_NOW, (t) * NSEC_PER_SEC)

#endif // __APPLE__

#endif

