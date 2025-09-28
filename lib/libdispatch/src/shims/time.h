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

#ifndef __DISPATCH_SHIMS_TIME__
#define __DISPATCH_SHIMS_TIME__

#ifndef __DISPATCH_INDIRECT__
#error "Please #include <dispatch/dispatch.h> instead of this file directly."
#endif

#if TARGET_OS_WIN32
static inline unsigned int
sleep(unsigned int seconds)
{
	Sleep(seconds * 1000); // milliseconds
	return 0;
}
#endif

uint64_t _dispatch_get_nanoseconds(void);

#if defined(__i386__) || defined(__x86_64__) || !HAVE_MACH_ABSOLUTE_TIME
// x86 currently implements mach time in nanoseconds
// this is NOT likely to change
DISPATCH_ALWAYS_INLINE
static inline uint64_t
_dispatch_time_mach2nano(uint64_t machtime)
{
	return machtime;
}

DISPATCH_ALWAYS_INLINE
static inline uint64_t
_dispatch_time_nano2mach(uint64_t nsec)
{
	return nsec;
}
#else
typedef struct _dispatch_host_time_data_s {
	dispatch_once_t pred;
	long double frac;
	bool ratio_1_to_1;
} _dispatch_host_time_data_s;
extern _dispatch_host_time_data_s _dispatch_host_time_data;
void _dispatch_get_host_time_init(void *context);

static inline uint64_t
_dispatch_time_mach2nano(uint64_t machtime)
{
	_dispatch_host_time_data_s *const data = &_dispatch_host_time_data;
	dispatch_once_f(&data->pred, NULL, _dispatch_get_host_time_init);

	if (!machtime || slowpath(data->ratio_1_to_1)) {
		return machtime;
	}
	if (machtime >= INT64_MAX) {
		return INT64_MAX;
	}
	long double big_tmp = ((long double)machtime * data->frac) + .5;
	if (slowpath(big_tmp >= INT64_MAX)) {
		return INT64_MAX;
	}
	return (uint64_t)big_tmp;
}

static inline uint64_t
_dispatch_time_nano2mach(uint64_t nsec)
{
	_dispatch_host_time_data_s *const data = &_dispatch_host_time_data;
	dispatch_once_f(&data->pred, NULL, _dispatch_get_host_time_init);

	if (!nsec || slowpath(data->ratio_1_to_1)) {
		return nsec;
	}
	if (nsec >= INT64_MAX) {
		return INT64_MAX;
	}
	long double big_tmp = ((long double)nsec / data->frac) + .5;
	if (slowpath(big_tmp >= INT64_MAX)) {
		return INT64_MAX;
	}
	return (uint64_t)big_tmp;
}
#endif

static inline uint64_t
_dispatch_absolute_time(void)
{
#if HAVE_MACH_ABSOLUTE_TIME
	return mach_absolute_time();
#elif TARGET_OS_WIN32
	LARGE_INTEGER now;
	return QueryPerformanceCounter(&now) ? now.QuadPart : 0;
#else
	struct timespec ts;
	int ret;

#if HAVE_DECL_CLOCK_UPTIME
	ret = clock_gettime(CLOCK_UPTIME, &ts);
#elif HAVE_DECL_CLOCK_MONOTONIC
	ret = clock_gettime(CLOCK_MONOTONIC, &ts);
#else
#error "clock_gettime: no supported absolute time clock"
#endif
	(void)dispatch_assume_zero(ret);

	/* XXXRW: Some kind of overflow detection needed? */
	return (ts.tv_sec * NSEC_PER_SEC + ts.tv_nsec);
#endif // HAVE_MACH_ABSOLUTE_TIME
}


#endif // __DISPATCH_SHIMS_TIME__
