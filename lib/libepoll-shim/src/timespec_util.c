#include "timespec_util.h"

#include <assert.h>

bool
timespec_is_valid(struct timespec const *ts)
{
	assert(ts != NULL);

	return ts->tv_sec >= 0 && ts->tv_nsec >= 0 && ts->tv_nsec < 1000000000;
}

bool
itimerspec_is_valid(struct itimerspec const *its)
{
	return timespec_is_valid(&its->it_value) &&
	    timespec_is_valid(&its->it_interval);
}

bool
timespecadd_safe(struct timespec const *tsp, struct timespec const *usp,
    struct timespec *vsp)
{
	assert(timespec_is_valid(tsp));
	assert(timespec_is_valid(usp));

	if (__builtin_add_overflow(tsp->tv_sec, usp->tv_sec, &vsp->tv_sec)) {
		return false;
	}
	vsp->tv_nsec = tsp->tv_nsec + usp->tv_nsec;

	if (vsp->tv_nsec >= 1000000000L) {
		if (__builtin_add_overflow(vsp->tv_sec, 1, &vsp->tv_sec)) {
			return false;
		}
		vsp->tv_nsec -= 1000000000L;
	}

	return true;
}

bool
timespecsub_safe(struct timespec const *tsp, struct timespec const *usp,
    struct timespec *vsp)
{
	assert(timespec_is_valid(tsp));
	assert(timespec_is_valid(usp));

	if (__builtin_sub_overflow(tsp->tv_sec, usp->tv_sec, &vsp->tv_sec)) {
		return false;
	}
	vsp->tv_nsec = tsp->tv_nsec - usp->tv_nsec;

	if (vsp->tv_nsec < 0) {
		if (__builtin_sub_overflow(vsp->tv_sec, 1, &vsp->tv_sec)) {
			return false;
		}
		vsp->tv_nsec += 1000000000L;
	}

	return true;
}
