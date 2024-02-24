#ifndef COMPAT_ITIMERSPEC_H
#define COMPAT_ITIMERSPEC_H

#ifdef COMPAT_ENABLE_ITIMERSPEC

#include <time.h>

struct itimerspec {
	struct timespec it_interval;
	struct timespec it_value;
};
#endif

#endif
