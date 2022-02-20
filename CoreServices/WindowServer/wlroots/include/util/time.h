#ifndef UTIL_TIME_H
#define UTIL_TIME_H

#include <time.h>

/**
 * Get the current time, in milliseconds.
 */
uint32_t get_current_time_msec(void);

/**
 * Convert a timespec to milliseconds.
 */
int64_t timespec_to_msec(const struct timespec *a);

/**
 * Convert nanoseconds to a timespec.
 */
void timespec_from_nsec(struct timespec *r, int64_t nsec);

/**
 * Subtracts timespec `b` from timespec `a`, and stores the difference in `r`.
 */
void timespec_sub(struct timespec *r, const struct timespec *a,
		const struct timespec *b);

#endif
