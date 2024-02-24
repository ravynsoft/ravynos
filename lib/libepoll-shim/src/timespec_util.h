#ifndef TIMESPEC_UTIL_H
#define TIMESPEC_UTIL_H

#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>

#include <time.h>

// TODO(jan): Remove this once the definition is exposed in <sys/time.h> in
// all supported FreeBSD versions.
#ifndef timespeccmp
#define timespeccmp(tvp, uvp, cmp)                   \
	(((tvp)->tv_sec == (uvp)->tv_sec) ?          \
		((tvp)->tv_nsec cmp(uvp)->tv_nsec) : \
		((tvp)->tv_sec cmp(uvp)->tv_sec))
#endif
#ifndef timespecsub
#define timespecsub(tsp, usp, vsp)                                \
	do {                                                      \
		(vsp)->tv_sec = (tsp)->tv_sec - (usp)->tv_sec;    \
		(vsp)->tv_nsec = (tsp)->tv_nsec - (usp)->tv_nsec; \
		if ((vsp)->tv_nsec < 0) {                         \
			(vsp)->tv_sec--;                          \
			(vsp)->tv_nsec += 1000000000L;            \
		}                                                 \
	} while (0)
#endif

bool timespec_is_valid(struct timespec const *ts);
bool itimerspec_is_valid(struct itimerspec const *its);
bool timespecadd_safe(struct timespec const *tsp, struct timespec const *usp,
    struct timespec *vsp);
bool timespecsub_safe(struct timespec const *tsp, struct timespec const *usp,
    struct timespec *vsp);

#endif
