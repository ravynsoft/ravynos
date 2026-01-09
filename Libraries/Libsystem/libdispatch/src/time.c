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

#include "internal.h"

#if DISPATCH_USE_HOST_TIME
typedef struct _dispatch_host_time_data_s {
	long double frac;
	bool ratio_1_to_1;
} _dispatch_host_time_data_s;

static _dispatch_host_time_data_s _dispatch_host_time_data;

uint64_t (*_dispatch_host_time_mach2nano)(uint64_t machtime);
uint64_t (*_dispatch_host_time_nano2mach)(uint64_t nsec);

static uint64_t
_dispatch_mach_host_time_mach2nano(uint64_t machtime)
{
	_dispatch_host_time_data_s *const data = &_dispatch_host_time_data;

	if (unlikely(!machtime || data->ratio_1_to_1)) {
		return machtime;
	}
	if (machtime >= INT64_MAX) {
		return INT64_MAX;
	}
	long double big_tmp = ((long double)machtime * data->frac) + .5L;
	if (unlikely(big_tmp >= INT64_MAX)) {
		return INT64_MAX;
	}
	return (uint64_t)big_tmp;
}

static uint64_t
_dispatch_mach_host_time_nano2mach(uint64_t nsec)
{
	_dispatch_host_time_data_s *const data = &_dispatch_host_time_data;

	if (unlikely(!nsec || data->ratio_1_to_1)) {
		return nsec;
	}
	if (nsec >= INT64_MAX) {
		return INT64_MAX;
	}
	long double big_tmp = ((long double)nsec / data->frac) + .5L;
	if (unlikely(big_tmp >= INT64_MAX)) {
		return INT64_MAX;
	}
	return (uint64_t)big_tmp;
}

static void
_dispatch_host_time_init(mach_timebase_info_data_t *tbi)
{
	_dispatch_host_time_data.frac = tbi->numer;
	_dispatch_host_time_data.frac /= tbi->denom;
	_dispatch_host_time_data.ratio_1_to_1 = (tbi->numer == tbi->denom);
	_dispatch_host_time_mach2nano = _dispatch_mach_host_time_mach2nano;
	_dispatch_host_time_nano2mach = _dispatch_mach_host_time_nano2mach;
}
#endif // DISPATCH_USE_HOST_TIME

void
_dispatch_time_init(void)
{
#if DISPATCH_USE_HOST_TIME
	mach_timebase_info_data_t tbi;
	(void)dispatch_assume_zero(mach_timebase_info(&tbi));
	_dispatch_host_time_init(&tbi);
#endif // DISPATCH_USE_HOST_TIME
}

dispatch_time_t
dispatch_time(dispatch_time_t inval, int64_t delta)
{
	uint64_t offset;
	if (inval == DISPATCH_TIME_FOREVER) {
		return DISPATCH_TIME_FOREVER;
	}

	dispatch_clock_t clock;
	uint64_t value;
	_dispatch_time_to_clock_and_value(inval, &clock, &value);
	if (value == DISPATCH_TIME_FOREVER) {
		// Out-of-range for this clock.
		return value;
	}
	if (clock == DISPATCH_CLOCK_WALL) {
		// wall clock
		offset = (uint64_t)delta;
		if (delta >= 0) {
			if ((int64_t)(value += offset) <= 0) {
				return DISPATCH_TIME_FOREVER; // overflow
			}
		} else {
			if ((int64_t)(value += offset) < 1) {
				// -1 is special == DISPATCH_TIME_FOREVER == forever, so
				// return -2 (after conversion to dispatch_time_t) instead.
				value = 2; // underflow.
			}
		}
		return _dispatch_clock_and_value_to_time(DISPATCH_CLOCK_WALL, value);
	}

	// up time or monotonic time. "value" has the clock type removed,
	// so the test against DISPATCH_TIME_NOW is correct for either clock.
	if (value == DISPATCH_TIME_NOW) {
		if (clock == DISPATCH_CLOCK_UPTIME) {
			value = _dispatch_uptime();
		} else {
			dispatch_assert(clock == DISPATCH_CLOCK_MONOTONIC);
			value = _dispatch_monotonic_time();
		}
	}
	if (delta >= 0) {
		offset = _dispatch_time_nano2mach((uint64_t)delta);
		if ((int64_t)(value += offset) <= 0) {
			return DISPATCH_TIME_FOREVER; // overflow
		}
		return _dispatch_clock_and_value_to_time(clock, value);
	} else {
		offset = _dispatch_time_nano2mach((uint64_t)-delta);
		if ((int64_t)(value -= offset) < 1) {
			return _dispatch_clock_and_value_to_time(clock, 1); // underflow
		}
		return _dispatch_clock_and_value_to_time(clock, value);
	}
}

dispatch_time_t
dispatch_walltime(const struct timespec *inval, int64_t delta)
{
	int64_t nsec;
	if (inval) {
		nsec = (int64_t)_dispatch_timespec_to_nano(*inval);
	} else {
		nsec = (int64_t)_dispatch_get_nanoseconds();
	}
	nsec += delta;
	if (nsec <= 1) {
		// -1 is special == DISPATCH_TIME_FOREVER == forever
		return delta >= 0 ? DISPATCH_TIME_FOREVER : (dispatch_time_t)-2ll;
	}
	return (dispatch_time_t)-nsec;
}

uint64_t
_dispatch_timeout(dispatch_time_t when)
{
	dispatch_time_t now;
	if (when == DISPATCH_TIME_FOREVER) {
		return DISPATCH_TIME_FOREVER;
	}
	if (when == DISPATCH_TIME_NOW) {
		return 0;
	}

	dispatch_clock_t clock;
	uint64_t value;
	_dispatch_time_to_clock_and_value(when, &clock, &value);
	if (clock == DISPATCH_CLOCK_WALL) {
		now = _dispatch_get_nanoseconds();
		return now >= value ? 0 : value - now;
	} else {
		if (clock == DISPATCH_CLOCK_UPTIME) {
			now = _dispatch_uptime();
		} else {
			dispatch_assert(clock == DISPATCH_CLOCK_MONOTONIC);
			now = _dispatch_monotonic_time();
		}
		return now >= value ? 0 : _dispatch_time_mach2nano(value - now);
	}
}

uint64_t
_dispatch_time_nanoseconds_since_epoch(dispatch_time_t when)
{
	if (when == DISPATCH_TIME_FOREVER) {
		return DISPATCH_TIME_FOREVER;
	}
	if ((int64_t)when < 0) {
		// time in nanoseconds since the POSIX epoch already
		return (uint64_t)-(int64_t)when;
	}

	// Up time or monotonic time.
	return _dispatch_get_nanoseconds() + _dispatch_timeout(when);
}
