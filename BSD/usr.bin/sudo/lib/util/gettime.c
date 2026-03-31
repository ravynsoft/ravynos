/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2014-2018 Todd C. Miller <Todd.Miller@sudo.ws>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * This is an open source non-commercial project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 */

#include <config.h>

#include <sys/time.h>
#include <time.h>

#if defined(__MACH__) && !defined(HAVE_CLOCK_GETTIME)
# include <mach/mach.h>
# include <mach/mach_time.h>
# include <mach/clock.h>
#endif

#include <sudo_compat.h>
#include <sudo_debug.h>
#include <sudo_util.h>

/*
 * On Linux and FreeBSD, CLOCK_MONOTONIC does not run while sleeping.
 * Linux provides CLOCK_BOOTTIME which runs while sleeping (FreeBSD does not).
 * Some systems provide CLOCK_UPTIME which only runs while awake.
 */
#if defined(CLOCK_BOOTTIME)
# define SUDO_CLOCK_BOOTTIME	CLOCK_BOOTTIME
#elif defined(CLOCK_MONOTONIC_RAW)
# define SUDO_CLOCK_BOOTTIME	CLOCK_MONOTONIC_RAW
#elif defined(CLOCK_MONOTONIC)
# define SUDO_CLOCK_BOOTTIME	CLOCK_MONOTONIC
#endif
#if defined(CLOCK_UPTIME_RAW)
# define SUDO_CLOCK_UPTIME	CLOCK_UPTIME_RAW
#elif defined(CLOCK_UPTIME)
# define SUDO_CLOCK_UPTIME	CLOCK_UPTIME
#elif defined(CLOCK_MONOTONIC)
# define SUDO_CLOCK_UPTIME	CLOCK_MONOTONIC
#endif

/*
 * Wall clock time, may run backward.
 */
#if defined(HAVE_CLOCK_GETTIME)
int
sudo_gettime_real_v1(struct timespec *ts)
{
    debug_decl(sudo_gettime_real, SUDO_DEBUG_UTIL);

    if (clock_gettime(CLOCK_REALTIME, ts) == -1) {
	struct timeval tv;

	sudo_debug_printf(SUDO_DEBUG_WARN|SUDO_DEBUG_ERRNO|SUDO_DEBUG_LINENO,
	    "clock_gettime(CLOCK_REALTIME) failed, trying gettimeofday()");
	if (gettimeofday(&tv, NULL) == -1)
	    debug_return_int(-1);
	TIMEVAL_TO_TIMESPEC(&tv, ts);
    }
    debug_return_int(0);
}
#else
int
sudo_gettime_real_v1(struct timespec *ts)
{
    struct timeval tv;
    debug_decl(sudo_gettime_real, SUDO_DEBUG_UTIL);

    if (gettimeofday(&tv, NULL) == -1)
	debug_return_int(-1);
    TIMEVAL_TO_TIMESPEC(&tv, ts);
    debug_return_int(0);
}
#endif

/*
 * Monotonic time, only runs forward.
 * We use a timer that only increments while sleeping, if possible.
 */
#if defined(HAVE_CLOCK_GETTIME) && defined(SUDO_CLOCK_BOOTTIME)
int
sudo_gettime_mono_v1(struct timespec *ts)
{
    static int has_monoclock = -1;
    debug_decl(sudo_gettime_mono, SUDO_DEBUG_UTIL);

    /* Check whether the kernel/libc actually supports a monotonic clock. */
# ifdef _SC_MONOTONIC_CLOCK
    if (has_monoclock == -1)
	has_monoclock = sysconf(_SC_MONOTONIC_CLOCK) != -1;
# endif
    if (!has_monoclock)
	debug_return_int(sudo_gettime_real(ts));
    if (clock_gettime(SUDO_CLOCK_BOOTTIME, ts) == -1) {
	sudo_debug_printf(SUDO_DEBUG_WARN|SUDO_DEBUG_ERRNO|SUDO_DEBUG_LINENO,
	    "clock_gettime(%d) failed, using wall clock",
	    (int)SUDO_CLOCK_BOOTTIME);
	has_monoclock = 0;
	debug_return_int(sudo_gettime_real(ts));
    }
    debug_return_int(0);
}
#elif defined(HAVE_GETHRTIME)
int
sudo_gettime_mono_v1(struct timespec *ts)
{
    hrtime_t nsec;
    debug_decl(sudo_gettime_mono, SUDO_DEBUG_UTIL);

    nsec = gethrtime();
    ts->tv_sec = nsec / 1000000000;
    ts->tv_nsec = nsec % 1000000000;
    debug_return_int(0);
}
#elif defined(__MACH__)
int
sudo_gettime_mono_v1(struct timespec *ts)
{
    uint64_t abstime, nsec;
    static mach_timebase_info_data_t timebase_info;
    debug_decl(sudo_gettime_mono, SUDO_DEBUG_UTIL);

    if (timebase_info.denom == 0)
	(void) mach_timebase_info(&timebase_info);
#ifdef HAVE_MACH_CONTINUOUS_TIME
    abstime = mach_continuous_time();		/* runs while asleep */
#else
    abstime = mach_absolute_time();		/* doesn't run while asleep */
#endif
    nsec = abstime * timebase_info.numer / timebase_info.denom;
    ts->tv_sec = nsec / 1000000000;
    ts->tv_nsec = nsec % 1000000000;
    debug_return_int(0);
}
#else
int
sudo_gettime_mono_v1(struct timespec *ts)
{
    /* No monotonic clock available, use wall clock. */
    return sudo_gettime_real(ts);
}
#endif

/*
 * Monotonic time, only runs forward.
 * We use a timer that only increments while awake, if possible.
 */
#if defined(HAVE_CLOCK_GETTIME) && defined(SUDO_CLOCK_UPTIME)
int
sudo_gettime_awake_v1(struct timespec *ts)
{
    static int has_monoclock = -1;
    debug_decl(sudo_gettime_awake, SUDO_DEBUG_UTIL);

    /* Check whether the kernel/libc actually supports a monotonic clock. */
# ifdef _SC_MONOTONIC_CLOCK
    if (has_monoclock == -1)
	has_monoclock = sysconf(_SC_MONOTONIC_CLOCK) != -1;
# endif
    if (!has_monoclock)
	debug_return_int(sudo_gettime_real(ts));
    if (clock_gettime(SUDO_CLOCK_UPTIME, ts) == -1) {
	sudo_debug_printf(SUDO_DEBUG_WARN|SUDO_DEBUG_ERRNO|SUDO_DEBUG_LINENO,
	    "clock_gettime(%d) failed, using wall clock",
	    (int)SUDO_CLOCK_UPTIME);
	has_monoclock = 0;
	debug_return_int(sudo_gettime_real(ts));
    }
    debug_return_int(0);
}
#elif defined(HAVE_GETHRTIME)
int
sudo_gettime_awake_v1(struct timespec *ts)
{
    hrtime_t nsec;
    debug_decl(sudo_gettime_awake, SUDO_DEBUG_UTIL);

    /* Currently the same as sudo_gettime_mono() */
    nsec = gethrtime();
    ts->tv_sec = nsec / 1000000000;
    ts->tv_nsec = nsec % 1000000000;
    debug_return_int(0);
}
#elif defined(__MACH__)
int
sudo_gettime_awake_v1(struct timespec *ts)
{
    uint64_t abstime, nsec;
    static mach_timebase_info_data_t timebase_info;
    debug_decl(sudo_gettime_awake, SUDO_DEBUG_UTIL);

    if (timebase_info.denom == 0)
	(void) mach_timebase_info(&timebase_info);
    abstime = mach_absolute_time();
    nsec = abstime * timebase_info.numer / timebase_info.denom;
    ts->tv_sec = nsec / 1000000000;
    ts->tv_nsec = nsec % 1000000000;
    debug_return_int(0);
}
#else
int
sudo_gettime_awake_v1(struct timespec *ts)
{
    /* No monotonic uptime clock available, use wall clock. */
    return sudo_gettime_real(ts);
}
#endif
