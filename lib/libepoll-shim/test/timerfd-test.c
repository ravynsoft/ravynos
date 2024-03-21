#define _GNU_SOURCE

#include <atf-c.h>

#include <sys/types.h>

#ifndef __linux__
#include <sys/event.h>
#endif

#include <sys/param.h>
#include <sys/select.h>
#include <sys/time.h>

#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <err.h>
#include <poll.h>
#include <time.h>
#include <unistd.h>

#include <sys/timerfd.h>

#include "atf-c-leakcheck.h"

// TODO(jan): Remove this once the definition is exposed in <sys/time.h> in
// all supported FreeBSD versions.
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

#ifndef timespecadd
#define timespecadd(tsp, usp, vsp)                                \
	do {                                                      \
		(vsp)->tv_sec = (tsp)->tv_sec + (usp)->tv_sec;    \
		(vsp)->tv_nsec = (tsp)->tv_nsec + (usp)->tv_nsec; \
		if ((vsp)->tv_nsec >= 1000000000L) {              \
			(vsp)->tv_sec++;                          \
			(vsp)->tv_nsec -= 1000000000L;            \
		}                                                 \
	} while (0)
#endif

#ifndef nitems
#define nitems(x) (sizeof((x)) / sizeof((x)[0]))
#endif

/* Time in ns that sleeps are allowed to take longer for in unit tests. */
#define TIMER_SLACK (90000000)

ATF_TC_WITHOUT_HEAD(timerfd__many_timers);
ATF_TC_BODY(timerfd__many_timers, tc)
{
	int timer_fds[256];
	int i;

	for (i = 0; i < (int)nitems(timer_fds); ++i) {
		timer_fds[i] = timerfd_create(CLOCK_MONOTONIC, /**/
		    TFD_CLOEXEC | TFD_NONBLOCK);
		if (timer_fds[i] < 0 && errno == EMFILE) {
			atf_tc_skip("timerfd_create: EMFILE");
		}
		ATF_REQUIRE_MSG(timer_fds[i] >= 0, "errno: %d", errno);
	}
}

static uint64_t
wait_for_timerfd(int timerfd)
{
	struct pollfd pfd = { .fd = timerfd, .events = POLLIN };

	ATF_REQUIRE(poll(&pfd, 1, -1) == 1);

	uint64_t timeouts;
	ssize_t r = read(timerfd, &timeouts, sizeof(timeouts));

	ATF_REQUIRE_MSG(r == (ssize_t)sizeof(timeouts), "%d %d", (int)r, errno);
	ATF_REQUIRE(timeouts > 0);
	return timeouts;
}

ATF_TC_WITHOUT_HEAD(timerfd__simple_timer);
ATF_TC_BODY_FD_LEAKCHECK(timerfd__simple_timer, tc)
{
	int timerfd = timerfd_create(CLOCK_MONOTONIC, /**/
	    TFD_CLOEXEC | TFD_NONBLOCK);

	ATF_REQUIRE(timerfd >= 0);

	struct itimerspec time = {
		.it_value.tv_sec = 0,
		.it_value.tv_nsec = 100000000,
	};

	struct timespec b, e;
	ATF_REQUIRE(clock_gettime(CLOCK_MONOTONIC, &b) == 0);

	ATF_REQUIRE(timerfd_settime(timerfd, 0, &time, NULL) == 0);
	(void)wait_for_timerfd(timerfd);

	ATF_REQUIRE(clock_gettime(CLOCK_MONOTONIC, &e) == 0);
	timespecsub(&e, &b, &e);

	ATF_REQUIRE((e.tv_sec == 0 && e.tv_nsec >= 100000000) || e.tv_sec > 0);
#ifndef ALLOW_TIMER_SLACK
	ATF_REQUIRE(e.tv_sec == 0 && e.tv_nsec < 100000000 + TIMER_SLACK);
#endif

	ATF_REQUIRE(close(timerfd) == 0);
}

ATF_TC_WITHOUT_HEAD(timerfd__simple_periodic_timer);
ATF_TC_BODY_FD_LEAKCHECK(timerfd__simple_periodic_timer, tc)
{
	int timerfd = timerfd_create(CLOCK_MONOTONIC, /**/
	    TFD_CLOEXEC | TFD_NONBLOCK);

	ATF_REQUIRE(timerfd >= 0);

	struct itimerspec time = {
		.it_value.tv_sec = 0,
		.it_value.tv_nsec = 200000000,
		.it_interval.tv_sec = 0,
		.it_interval.tv_nsec = 200000000,
	};

	struct timespec b, e;
	ATF_REQUIRE(clock_gettime(CLOCK_MONOTONIC, &b) == 0);

	ATF_REQUIRE(timerfd_settime(timerfd, 0, &time, NULL) == 0);
	uint64_t timeouts = wait_for_timerfd(timerfd);

	ATF_REQUIRE(clock_gettime(CLOCK_MONOTONIC, &e) == 0);
	timespecsub(&e, &b, &e);

	ATF_REQUIRE((e.tv_sec == 0 && e.tv_nsec >= 200000000) || e.tv_sec > 0);
	ATF_REQUIRE(timeouts >= 1);
#ifndef ALLOW_TIMER_SLACK
	ATF_REQUIRE(e.tv_sec == 0 && e.tv_nsec < 200000000 + TIMER_SLACK);
	ATF_REQUIRE(timeouts == 1);
#endif

	usleep(400000);

	ATF_REQUIRE(read(timerfd, &timeouts, sizeof(timeouts)) ==
	    (ssize_t)sizeof(timeouts));
	ATF_REQUIRE(timeouts >= 2);
#ifndef ALLOW_TIMER_SLACK
	ATF_REQUIRE(timeouts == 2);
#endif

	ATF_REQUIRE(close(timerfd) == 0);
}

ATF_TC_WITHOUT_HEAD(timerfd__complex_periodic_timer);
ATF_TC_BODY_FD_LEAKCHECK(timerfd__complex_periodic_timer, tc)
{
	int timerfd = timerfd_create(CLOCK_MONOTONIC, /**/
	    TFD_CLOEXEC | TFD_NONBLOCK);

	ATF_REQUIRE(timerfd >= 0);

	struct itimerspec time = {
		.it_value.tv_sec = 0,
		.it_value.tv_nsec = 100000000,
		.it_interval.tv_sec = 0,
		.it_interval.tv_nsec = 200000001,
	};

	struct timespec b, e;
	ATF_REQUIRE(clock_gettime(CLOCK_MONOTONIC, &b) == 0);

	ATF_REQUIRE(timerfd_settime(timerfd, 0, &time, NULL) == 0);
	uint64_t timeouts = wait_for_timerfd(timerfd);

	ATF_REQUIRE(clock_gettime(CLOCK_MONOTONIC, &e) == 0);
	timespecsub(&e, &b, &e);

	ATF_REQUIRE((e.tv_sec == 0 && e.tv_nsec >= 100000000) || e.tv_sec > 0);
	ATF_REQUIRE(timeouts >= 1);
#ifndef ALLOW_TIMER_SLACK
	ATF_REQUIRE_MSG(e.tv_sec == 0 && e.tv_nsec >= 100000000 &&
		e.tv_nsec < 100000000 + TIMER_SLACK,
	    "%ld", (long)e.tv_nsec);
	ATF_REQUIRE(timeouts == 1);
#endif

	usleep(401000);

	ATF_REQUIRE(read(timerfd, &timeouts, sizeof(timeouts)) ==
	    (ssize_t)sizeof(timeouts));
	ATF_REQUIRE_MSG(timeouts >= 2, "%d", (int)timeouts);
#ifndef ALLOW_TIMER_SLACK
	ATF_REQUIRE_MSG(timeouts == 2, "%d", (int)timeouts);
#endif

	ATF_REQUIRE(close(timerfd) == 0);
}

ATF_TC_WITHOUT_HEAD(timerfd__reset_periodic_timer);
ATF_TC_BODY_FD_LEAKCHECK(timerfd__reset_periodic_timer, tc)
{
	int timerfd = timerfd_create(CLOCK_MONOTONIC, /**/
	    TFD_CLOEXEC | TFD_NONBLOCK);

	ATF_REQUIRE(timerfd >= 0);

	struct itimerspec time = {
		.it_value.tv_sec = 0,
		.it_value.tv_nsec = 100000000,
		.it_interval.tv_sec = 0,
		.it_interval.tv_nsec = 100000000,
	};

	struct timespec b, e;
	ATF_REQUIRE(clock_gettime(CLOCK_MONOTONIC, &b) == 0);

	ATF_REQUIRE(timerfd_settime(timerfd, 0, &time, NULL) == 0);
	(void)wait_for_timerfd(timerfd);

	time = (struct itimerspec) {
		.it_value.tv_sec = 0,
		.it_value.tv_nsec = 50000000,
		.it_interval.tv_sec = 0,
		.it_interval.tv_nsec = 100000000,
	};

	ATF_REQUIRE(timerfd_settime(timerfd, 0, &time, NULL) == 0);

	uint64_t timeouts = wait_for_timerfd(timerfd);
	ATF_REQUIRE(timeouts >= 1);
#ifndef ALLOW_TIMER_SLACK
	ATF_REQUIRE(timeouts == 1);
#endif

	ATF_REQUIRE(clock_gettime(CLOCK_MONOTONIC, &e) == 0);
	timespecsub(&e, &b, &e);
	ATF_REQUIRE((e.tv_sec == 0 && e.tv_nsec >= 150000000) || e.tv_sec > 0);
#ifndef ALLOW_TIMER_SLACK
	ATF_REQUIRE(e.tv_sec == 0 && e.tv_nsec >= 150000000 &&
	    e.tv_nsec < 150000000 + TIMER_SLACK * 2);
#endif

	ATF_REQUIRE(close(timerfd) == 0);
}

ATF_TC_WITHOUT_HEAD(timerfd__reenable_periodic_timer);
ATF_TC_BODY_FD_LEAKCHECK(timerfd__reenable_periodic_timer, tc)
{
	int timerfd = timerfd_create(CLOCK_MONOTONIC, /**/
	    TFD_CLOEXEC | TFD_NONBLOCK);

	ATF_REQUIRE(timerfd >= 0);

	struct itimerspec time = {
		.it_value.tv_sec = 0,
		.it_value.tv_nsec = 100000000,
		.it_interval.tv_sec = 0,
		.it_interval.tv_nsec = 100000000,
	};

	struct timespec b, e;
	ATF_REQUIRE(clock_gettime(CLOCK_MONOTONIC, &b) == 0);

	ATF_REQUIRE(timerfd_settime(timerfd, 0, &time, NULL) == 0);
	uint64_t timeouts = wait_for_timerfd(timerfd);

	ATF_REQUIRE(timeouts >= 1);
#ifndef ALLOW_TIMER_SLACK
	ATF_REQUIRE(timeouts == 1);
#endif

	time = (struct itimerspec) {
		.it_value.tv_sec = 0,
		.it_value.tv_nsec = 0,
		.it_interval.tv_sec = 0,
		.it_interval.tv_nsec = 0,
	};

	ATF_REQUIRE(timerfd_settime(timerfd, 0, &time, NULL) == 0);

	struct pollfd pfd = { .fd = timerfd, .events = POLLIN };
	ATF_REQUIRE(poll(&pfd, 1, 250) == 0);

	ATF_REQUIRE(clock_gettime(CLOCK_MONOTONIC, &e) == 0);
	timespecsub(&e, &b, &e);

	ATF_REQUIRE((e.tv_sec == 0 && e.tv_nsec >= 350000000) || e.tv_sec > 0);
#ifndef ALLOW_TIMER_SLACK
	ATF_REQUIRE(e.tv_sec == 0 && e.tv_nsec >= 350000000 &&
	    e.tv_nsec < 350000000 + TIMER_SLACK * 2);
#endif

	time = (struct itimerspec) {
		.it_value.tv_sec = 1,
		.it_value.tv_nsec = 0,
		.it_interval.tv_sec = 1,
		.it_interval.tv_nsec = 0,
	};
	ATF_REQUIRE(timerfd_settime(timerfd, 0, &time, NULL) == 0);

	ATF_REQUIRE(close(timerfd) == 0);
}

/*
 * Adapted from sghctoma's example here:
 * https://github.com/jiixyj/epoll-shim/issues/2
 *
 * The SIGUSR1 signal should not kill the process.
 */
ATF_TC_WITHOUT_HEAD(timerfd__expire_five);
ATF_TC_BODY_FD_LEAKCHECK(timerfd__expire_five, tc)
{
	int fd;
	struct itimerspec value;
	uint64_t total_exp = 0;

	fd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
	ATF_REQUIRE(fd >= 0);

	value.it_value.tv_sec = 3;
	value.it_value.tv_nsec = 0;
	value.it_interval.tv_sec = 1;
	value.it_interval.tv_nsec = 0;

	ATF_REQUIRE(timerfd_settime(fd, 0, &value, NULL) == 0);

	sigset_t sigs;
	sigemptyset(&sigs);
	sigaddset(&sigs, SIGUSR1);
	sigprocmask(SIG_BLOCK, &sigs, NULL);

	kill(getpid(), SIGUSR1);

	for (;;) {
		uint64_t exp = wait_for_timerfd(fd);

		printf("timer expired %u times\n", (unsigned)exp);

		total_exp += exp;
		if (total_exp >= 5) {
			break;
		}
	}

	ATF_REQUIRE(close(fd) == 0);
}

ATF_TC_WITHOUT_HEAD(timerfd__simple_gettime);
ATF_TC_BODY_FD_LEAKCHECK(timerfd__simple_gettime, tc)
{
	struct itimerspec curr_value;

	int fd = timerfd_create(CLOCK_MONOTONIC, 0);
	ATF_REQUIRE(fd >= 0);

	ATF_REQUIRE(timerfd_gettime(fd, &curr_value) == 0);

	ATF_REQUIRE(curr_value.it_value.tv_sec == 0);
	ATF_REQUIRE(curr_value.it_value.tv_nsec == 0);
	ATF_REQUIRE(curr_value.it_interval.tv_sec == 0);
	ATF_REQUIRE(curr_value.it_interval.tv_nsec == 0);

	struct itimerspec time = {
		.it_value.tv_sec = 0,
		.it_value.tv_nsec = 100000000,
		.it_interval.tv_sec = 0,
		.it_interval.tv_nsec = 100000000,
	};

	curr_value = time;
	ATF_REQUIRE(timerfd_settime(fd, 0, &time, &curr_value) == 0);
	ATF_REQUIRE(curr_value.it_value.tv_sec == 0);
	ATF_REQUIRE(curr_value.it_value.tv_nsec == 0);
	ATF_REQUIRE(curr_value.it_interval.tv_sec == 0);
	ATF_REQUIRE(curr_value.it_interval.tv_nsec == 0);

	ATF_REQUIRE(close(fd) == 0);
}

ATF_TC_WITHOUT_HEAD(timerfd__simple_blocking_periodic_timer);
ATF_TC_BODY_FD_LEAKCHECK(timerfd__simple_blocking_periodic_timer, tc)
{
	int timerfd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC);

	ATF_REQUIRE(timerfd >= 0);

	struct itimerspec time = {
		.it_value.tv_sec = 0,
		.it_value.tv_nsec = 100000000,
		.it_interval.tv_sec = 0,
		.it_interval.tv_nsec = 100000000,
	};

	struct timespec b, e;
	ATF_REQUIRE(clock_gettime(CLOCK_MONOTONIC, &b) == 0);

	ATF_REQUIRE(timerfd_settime(timerfd, 0, &time, NULL) == 0);

	uint64_t timeouts = 0;
	int num_loop_iterations = 0;

	while (timeouts < 3) {
		uint64_t timeouts_local;
		ATF_REQUIRE(
		    read(timerfd, &timeouts_local, sizeof(timeouts_local)) ==
		    (ssize_t)sizeof(timeouts_local));
		ATF_REQUIRE(timeouts_local > 0);

		++num_loop_iterations;
		timeouts += timeouts_local;
	}

	ATF_REQUIRE(clock_gettime(CLOCK_MONOTONIC, &e) == 0);
	timespecsub(&e, &b, &e);

	ATF_REQUIRE((e.tv_sec == 0 && e.tv_nsec >= 300000000) || e.tv_sec > 0);
#ifndef ALLOW_TIMER_SLACK
	ATF_REQUIRE(e.tv_sec == 0 && e.tv_nsec >= 300000000 &&
	    e.tv_nsec < 300000000 + TIMER_SLACK);
#endif

	ATF_REQUIRE(num_loop_iterations <= 3);

	ATF_REQUIRE(close(timerfd) == 0);
}

ATF_TC_WITHOUT_HEAD(timerfd__argument_checks);
ATF_TC_BODY_FD_LEAKCHECK(timerfd__argument_checks, tc)
{
	int timerfd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC);
	ATF_REQUIRE(timerfd >= 0);

	struct itimerspec time = {
		.it_value.tv_sec = 0,
		.it_value.tv_nsec = 100000000,
		.it_interval.tv_sec = 0,
		.it_interval.tv_nsec = 100000000,
	};

	ATF_REQUIRE_ERRNO(EFAULT, timerfd_settime(timerfd, 0, NULL, NULL) < 0);
	ATF_REQUIRE_ERRNO(EFAULT, timerfd_settime(-2, 0, NULL, NULL) < 0);
	ATF_REQUIRE_ERRNO(EBADF, timerfd_settime(-2, 0, &time, NULL) < 0);
	ATF_REQUIRE_ERRNO(EFAULT, timerfd_settime(-2, 42, NULL, NULL) < 0);
	ATF_REQUIRE_ERRNO(EINVAL, timerfd_settime(-2, 42, &time, NULL) < 0);
	ATF_REQUIRE_ERRNO(EINVAL,
	    timerfd_settime(timerfd, 42, &time, NULL) < 0);

	{
		time = (struct itimerspec) {
			.it_value.tv_sec = -1,
			.it_value.tv_nsec = 100000000,
			.it_interval.tv_sec = 0,
			.it_interval.tv_nsec = 100000000,
		};
		int r = timerfd_settime(timerfd, 0, &time, NULL);
#if defined(__NetBSD__)
		if (r == 0) {
			atf_tc_skip(
			    "NetBSD timerfd_settime does not check time");
		}
#endif
		ATF_REQUIRE_ERRNO(EINVAL, r < 0);
	}
	{
		time = (struct itimerspec) {
			.it_value.tv_sec = 0,
			.it_value.tv_nsec = -1,
			.it_interval.tv_sec = 0,
			.it_interval.tv_nsec = 100000000,
		};
		ATF_REQUIRE_ERRNO(EINVAL,
		    timerfd_settime(timerfd, 0, &time, NULL) < 0);
	}
	{
		time = (struct itimerspec) {
			.it_value.tv_sec = 0,
			.it_value.tv_nsec = 100000000,
			.it_interval.tv_sec = -1,
			.it_interval.tv_nsec = 100000000,
		};
		ATF_REQUIRE_ERRNO(EINVAL,
		    timerfd_settime(timerfd, 0, &time, NULL) < 0);
	}
	{
		time = (struct itimerspec) {
			.it_value.tv_sec = 0,
			.it_value.tv_nsec = 100000000,
			.it_interval.tv_sec = 0,
			.it_interval.tv_nsec = -1,
		};
		ATF_REQUIRE_ERRNO(EINVAL,
		    timerfd_settime(timerfd, 0, &time, NULL) < 0);
	}
	{
		time = (struct itimerspec) {
			.it_value.tv_sec = 0,
			.it_value.tv_nsec = 1000000000,
			.it_interval.tv_sec = 0,
			.it_interval.tv_nsec = 100000000,
		};
		ATF_REQUIRE_ERRNO(EINVAL,
		    timerfd_settime(timerfd, 0, &time, NULL) < 0);
	}
	{
		time = (struct itimerspec) {
			.it_value.tv_sec = 0,
			.it_value.tv_nsec = 100000000,
			.it_interval.tv_sec = 0,
			.it_interval.tv_nsec = 1000000000,
		};
		ATF_REQUIRE_ERRNO(EINVAL,
		    timerfd_settime(timerfd, 0, &time, NULL) < 0);
	}

	ATF_REQUIRE_ERRNO(EINVAL,
	    timerfd_create(CLOCK_MONOTONIC | 42, TFD_CLOEXEC));
	ATF_REQUIRE_ERRNO(EINVAL,
	    timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | 42));

	ATF_REQUIRE(close(timerfd) == 0);

	struct itimerspec itimerspec;
	ATF_REQUIRE_ERRNO(EBADF, timerfd_gettime(timerfd, &itimerspec) < 0);

	ATF_REQUIRE(timerfd_settime(timerfd, 0, &itimerspec, NULL) < 0);
	/* Linux 5.10 returns EINVAL instead of EBADF. */
	ATF_REQUIRE(errno == EBADF || errno == EINVAL);
}

ATF_TC_WITHOUT_HEAD(timerfd__upgrade_simple_to_complex);
ATF_TC_BODY_FD_LEAKCHECK(timerfd__upgrade_simple_to_complex, tc)
{
	int timerfd = timerfd_create(CLOCK_MONOTONIC, /**/
	    TFD_CLOEXEC | TFD_NONBLOCK);

	ATF_REQUIRE(timerfd >= 0);

	struct itimerspec time = {
		.it_value.tv_sec = 0,
		.it_value.tv_nsec = 100000000,
		.it_interval.tv_sec = 0,
		.it_interval.tv_nsec = 100000000,
	};

	ATF_REQUIRE(timerfd_settime(timerfd, 0, &time, NULL) == 0);
	(void)wait_for_timerfd(timerfd);

	time = (struct itimerspec) {
		.it_value.tv_sec = 0,
		.it_value.tv_nsec = 50000000,
		.it_interval.tv_sec = 0,
		.it_interval.tv_nsec = 95000000,
	};

	struct timespec b, e;
	ATF_REQUIRE(clock_gettime(CLOCK_MONOTONIC, &b) == 0);

	ATF_REQUIRE(timerfd_settime(timerfd, 0, &time, NULL) == 0);

	uint64_t timeouts = wait_for_timerfd(timerfd);
	ATF_REQUIRE(timeouts >= 1);
#ifndef ALLOW_TIMER_SLACK
	ATF_REQUIRE(timeouts == 1);
#endif

	ATF_REQUIRE(clock_gettime(CLOCK_MONOTONIC, &e) == 0);
	timespecsub(&e, &b, &e);
	ATF_REQUIRE((e.tv_sec == 0 && e.tv_nsec >= 50000000) || e.tv_sec > 0);
#ifndef ALLOW_TIMER_SLACK
	ATF_REQUIRE_MSG(e.tv_sec == 0 && e.tv_nsec < 50000000 + TIMER_SLACK,
	    "%ld", e.tv_nsec);
#endif

	timeouts = wait_for_timerfd(timerfd);
	ATF_REQUIRE(timeouts >= 1);
#ifndef ALLOW_TIMER_SLACK
	ATF_REQUIRE(timeouts == 1);
#endif

	ATF_REQUIRE(clock_gettime(CLOCK_MONOTONIC, &e) == 0);
	timespecsub(&e, &b, &e);
	ATF_REQUIRE((e.tv_sec == 0 && e.tv_nsec >= 145000000) || e.tv_sec > 0);
#ifndef ALLOW_TIMER_SLACK
	ATF_REQUIRE(e.tv_sec == 0 && e.tv_nsec >= 145000000 &&
	    e.tv_nsec < 145000000 + TIMER_SLACK);
#endif

	ATF_REQUIRE(close(timerfd) == 0);
}

ATF_TC_WITHOUT_HEAD(timerfd__absolute_timer);
ATF_TC_BODY_FD_LEAKCHECK(timerfd__absolute_timer, tc)
{
	int timerfd = timerfd_create(CLOCK_MONOTONIC, /**/
	    TFD_CLOEXEC | TFD_NONBLOCK);

	ATF_REQUIRE(timerfd >= 0);

	struct timespec b, e;
	ATF_REQUIRE(clock_gettime(CLOCK_MONOTONIC, &b) == 0);

	struct itimerspec time = {
		.it_value = b,
		.it_interval.tv_sec = 0,
		.it_interval.tv_nsec = 0,
	};

	struct timespec ts_600ms = {
		.tv_sec = 0,
		.tv_nsec = 600000000,
	};

	timespecadd(&time.it_value, &ts_600ms, &time.it_value);

	ATF_REQUIRE(timerfd_settime(timerfd, /**/
			TFD_TIMER_ABSTIME, &time, NULL) == 0);

	struct pollfd pfd = { .fd = timerfd, .events = POLLIN };
	ATF_REQUIRE(poll(&pfd, 1, -1) == 1);

	// Don't read(2) here!

	ATF_REQUIRE(clock_gettime(CLOCK_MONOTONIC, &e) == 0);
	timespecsub(&e, &b, &e);
#ifndef ALLOW_TIMER_SLACK
	ATF_REQUIRE(e.tv_sec == 0 &&
	    /* Don't check for this because of spurious wakeups. */
	    /* e.tv_nsec >= 600000000 && */
	    e.tv_nsec < 600000000 + TIMER_SLACK);
#endif

	struct itimerspec zeroed_its = {
		.it_value.tv_sec = 0,
		.it_value.tv_nsec = 0,
		.it_interval.tv_sec = 0,
		.it_interval.tv_nsec = 0,
	};
	ATF_REQUIRE(timerfd_settime(timerfd, 0, &zeroed_its, NULL) == 0);

	uint64_t timeouts;
	ATF_REQUIRE_ERRNO(EAGAIN,
	    read(timerfd, &timeouts, sizeof(timeouts)) < 0);

	ATF_REQUIRE(poll(&pfd, 1, 0) == 0);

	ATF_REQUIRE(close(timerfd) == 0);
}

ATF_TC_WITHOUT_HEAD(timerfd__absolute_timer_in_the_past);
ATF_TC_BODY_FD_LEAKCHECK(timerfd__absolute_timer_in_the_past, tc)
{
	int timerfd = timerfd_create(CLOCK_MONOTONIC, /**/
	    TFD_CLOEXEC | TFD_NONBLOCK);

	ATF_REQUIRE(timerfd >= 0);

	struct timespec b;
	ATF_REQUIRE(clock_gettime(CLOCK_MONOTONIC, &b) == 0);

	{
		struct itimerspec time = {
			.it_value = b,
			.it_interval.tv_sec = 10,
			.it_interval.tv_nsec = 0,
		};
		time.it_value.tv_sec -= 1;

		ATF_REQUIRE(timerfd_settime(timerfd, /**/
				TFD_TIMER_ABSTIME, &time, NULL) == 0);

		struct pollfd pfd = { .fd = timerfd, .events = POLLIN };
		ATF_REQUIRE(poll(&pfd, 1, 1000) == 1);
	}

	{
		struct itimerspec time = {
			.it_value = b,
			.it_interval.tv_sec = 0,
			.it_interval.tv_nsec = 10000000,
		};
		time.it_value.tv_sec -= 1;

		ATF_REQUIRE(timerfd_settime(timerfd, /**/
				TFD_TIMER_ABSTIME, &time, NULL) == 0);

		struct pollfd pfd = { .fd = timerfd, .events = POLLIN };
		ATF_REQUIRE(poll(&pfd, 1, -1) == 1);
	}

	uint64_t timeouts;
	ATF_REQUIRE(read(timerfd, &timeouts, sizeof(timeouts)) ==
	    (ssize_t)sizeof(timeouts));

	ATF_REQUIRE_MSG(timeouts >= 101, "%d", (int)timeouts);

	ATF_REQUIRE(close(timerfd) == 0);
}

ATF_TC_WITHOUT_HEAD(timerfd__reset_absolute);
ATF_TC_BODY_FD_LEAKCHECK(timerfd__reset_absolute, tc)
{
	int timerfd = timerfd_create(CLOCK_MONOTONIC, /**/
	    TFD_CLOEXEC | TFD_NONBLOCK);

	ATF_REQUIRE(timerfd >= 0);

	struct timespec b;
	ATF_REQUIRE(clock_gettime(CLOCK_MONOTONIC, &b) == 0);

	{
		struct itimerspec time = {
			.it_value = b,
		};
		time.it_value.tv_sec += 10;

		ATF_REQUIRE(timerfd_settime(timerfd, /**/
				TFD_TIMER_ABSTIME, &time, NULL) == 0);

		struct pollfd pfd = { .fd = timerfd, .events = POLLIN };
		ATF_REQUIRE(poll(&pfd, 1, 100) == 0);
	}

	{
		struct itimerspec time = {
			.it_value = b,
		};
		time.it_value.tv_nsec += 500000000;
		if (time.it_value.tv_nsec >= 1000000000) {
			time.it_value.tv_nsec -= 1000000000;
			time.it_value.tv_sec += 1;
		}

		ATF_REQUIRE(timerfd_settime(timerfd, /**/
				TFD_TIMER_ABSTIME, &time, NULL) == 0);

		struct pollfd pfd = { .fd = timerfd, .events = POLLIN };
		ATF_REQUIRE(poll(&pfd, 1, 1000) == 1);
	}

	uint64_t timeouts;
	ATF_REQUIRE(read(timerfd, &timeouts, sizeof(timeouts)) ==
	    (ssize_t)sizeof(timeouts));

	ATF_REQUIRE_MSG(timeouts == 1, "%d", (int)timeouts);

	ATF_REQUIRE(close(timerfd) == 0);
}

ATF_TC(timerfd__periodic_timer_performance);
ATF_TC_HEAD(timerfd__periodic_timer_performance, tc)
{
	atf_tc_set_md_var(tc, "timeout", "1");
}
ATF_TC_BODY_FD_LEAKCHECK(timerfd__periodic_timer_performance, tc)
{
	int timerfd = timerfd_create(CLOCK_MONOTONIC, /**/
	    TFD_CLOEXEC | TFD_NONBLOCK);

	ATF_REQUIRE(timerfd >= 0);

	struct itimerspec time = {
		.it_value.tv_sec = 0,
		.it_value.tv_nsec = 1,
		.it_interval.tv_sec = 0,
		.it_interval.tv_nsec = 1,
	};

	ATF_REQUIRE(timerfd_settime(timerfd, 0, &time, NULL) == 0);

	usleep(400000);

	struct pollfd pfd = { .fd = timerfd, .events = POLLIN };
	ATF_REQUIRE(poll(&pfd, 1, -1) == 1);

	uint64_t timeouts;
	ATF_REQUIRE(read(timerfd, &timeouts, sizeof(timeouts)) ==
	    (ssize_t)sizeof(timeouts));
	ATF_REQUIRE(timeouts >= 400000000);

	ATF_REQUIRE(close(timerfd) == 0);
}

ATF_TC_WITHOUT_HEAD(timerfd__argument_overflow);
ATF_TC_BODY_FD_LEAKCHECK(timerfd__argument_overflow, tc)
{
	int timerfd = timerfd_create(CLOCK_MONOTONIC, /**/
	    TFD_CLOEXEC | TFD_NONBLOCK);
	ATF_REQUIRE(timerfd >= 0);
	{
		struct itimerspec time = {
			.it_value.tv_sec = 0,
			.it_value.tv_nsec = 1,
		};

		ATF_REQUIRE(timerfd_settime(timerfd, /**/
				TFD_TIMER_ABSTIME, &time, NULL) == 0);

		struct pollfd pfd = { .fd = timerfd, .events = POLLIN };
		ATF_REQUIRE(poll(&pfd, 1, -1) == 1);

		uint64_t timeouts;
		ATF_REQUIRE(read(timerfd, &timeouts, sizeof(timeouts)) ==
		    (ssize_t)sizeof(timeouts));
		ATF_REQUIRE(timeouts == 1);

		ATF_REQUIRE(read(timerfd, &timeouts, sizeof(timeouts)) < 0);
	}
	{
		struct itimerspec time = {
			.it_value.tv_sec = INT_MAX,
			.it_value.tv_nsec = 999999999,
		};

		ATF_REQUIRE(timerfd_settime(timerfd, /**/
				TFD_TIMER_ABSTIME, &time, NULL) == 0);

		struct pollfd pfd = { .fd = timerfd, .events = POLLIN };
		ATF_REQUIRE(poll(&pfd, 1, 500) == 0);

		uint64_t timeouts;
		ATF_REQUIRE(read(timerfd, &timeouts, sizeof(timeouts)) < 0);
	}

	ATF_REQUIRE(close(timerfd) == 0);
}

ATF_TC(timerfd__short_evfilt_timer_timeout);
ATF_TC_HEAD(timerfd__short_evfilt_timer_timeout, tc)
{
	atf_tc_set_md_var(tc, "timeout", "30");
}
ATF_TC_BODY_FD_LEAKCHECK(timerfd__short_evfilt_timer_timeout, tc)
{
#ifndef __linux__
	int kq = kqueue();
	ATF_REQUIRE(kq >= 0);

	bool returns_early = false;

#if defined(__NetBSD__) && __NetBSD_Version__ < 999009100
	/*
	 * Expect that NetBSD's EVFILT_TIMER returns early at some point. The
	 * test should exceed the timeout and thus fail if/when this bug is
	 * fixed in NetBSD.
	 */
	for (;;) {
#else
	for (int l = 0; l < 10; ++l) {
#endif
		for (int i = 1; i <= 17; ++i) {
			struct kevent kev;
			EV_SET(&kev, 0, EVFILT_TIMER, EV_ADD | EV_ONESHOT, 0, i,
			    0);

			struct timespec b;
			ATF_REQUIRE(clock_gettime(CLOCK_MONOTONIC, &b) == 0);

			ATF_REQUIRE(kevent(kq, &kev, 1, NULL, 0, NULL) == 0);

			ATF_REQUIRE(kevent(kq, NULL, 0, &kev, 1, NULL) == 1);

			struct timespec e;
			ATF_REQUIRE(clock_gettime(CLOCK_MONOTONIC, &e) == 0);

			struct timespec diff;
			timespecsub(&e, &b, &diff);

			if (diff.tv_sec != 0 || diff.tv_nsec < i * 1000000) {
				fprintf(stderr,
				    "expected: %lldns, got: %lldns\n",
				    (long long)(i * 1000000LL),
				    (long long)diff.tv_nsec);
				returns_early = true;
				goto check;
			}
		}
	}

check:
#if defined(__NetBSD__) && __NetBSD_Version__ < 999009100
	ATF_REQUIRE(returns_early);
#else
	ATF_REQUIRE(!returns_early);
#endif

	ATF_REQUIRE(close(kq) == 0);
#endif

	/*
	 * timerfd's should never return early, regardless of how
	 * EVFILT_TIMER behaves.
	 */

	int timerfd = timerfd_create(CLOCK_MONOTONIC, /**/
	    TFD_CLOEXEC | TFD_NONBLOCK);

	ATF_REQUIRE(timerfd >= 0);

	for (int l = 0; l < 10; ++l) {
		for (int i = 1; i <= 17; ++i) {
			struct itimerspec time = {
				.it_value.tv_sec = 0,
				.it_value.tv_nsec = i * 1000000,
			};

			struct timespec b;
			ATF_REQUIRE(clock_gettime(CLOCK_MONOTONIC, &b) == 0);

			ATF_REQUIRE(
			    timerfd_settime(timerfd, 0, &time, NULL) == 0);
			(void)wait_for_timerfd(timerfd);

			struct timespec e;
			ATF_REQUIRE(clock_gettime(CLOCK_MONOTONIC, &e) == 0);

			struct timespec diff;
			timespecsub(&e, &b, &diff);

			ATF_REQUIRE(
			    diff.tv_sec == 0 && diff.tv_nsec >= i * 1000000);
			fprintf(stderr, "%dms, waited %lldns\n", i,
			    (long long)diff.tv_nsec);
		}
	}

	ATF_REQUIRE(close(timerfd) == 0);
}

ATF_TC_WITHOUT_HEAD(timerfd__unmodified_errno);
ATF_TC_BODY_FD_LEAKCHECK(timerfd__unmodified_errno, tc)
{
	int timerfd = timerfd_create(CLOCK_MONOTONIC, /**/
	    TFD_CLOEXEC | TFD_NONBLOCK);
	ATF_REQUIRE(timerfd >= 0);
	ATF_REQUIRE(errno == 0);

	ATF_REQUIRE(timerfd_settime(timerfd, 0,
			&(struct itimerspec) {
			    .it_value.tv_sec = 0,
			    .it_value.tv_nsec = 100000000,
			},
			NULL) == 0);
	(void)wait_for_timerfd(timerfd);
	ATF_REQUIRE(errno == 0);

	ATF_REQUIRE(timerfd_settime(timerfd, 0,
			&(struct itimerspec) {
			    .it_value.tv_sec = 0,
			    .it_value.tv_nsec = 0,
			},
			NULL) == 0);
	ATF_REQUIRE(errno == 0);

	ATF_REQUIRE(timerfd_settime(timerfd, 0,
			&(struct itimerspec) {
			    .it_value.tv_sec = 0,
			    .it_value.tv_nsec = 0,
			},
			NULL) == 0);
	ATF_REQUIRE(errno == 0);

	ATF_REQUIRE(close(timerfd) == 0);
	ATF_REQUIRE(errno == 0);
}

ATF_TC_WITHOUT_HEAD(timerfd__reset_to_very_long);
ATF_TC_BODY_FD_LEAKCHECK(timerfd__reset_to_very_long, tc)
{
	int timerfd = timerfd_create(CLOCK_MONOTONIC, /**/
	    TFD_CLOEXEC | TFD_NONBLOCK);
	ATF_REQUIRE(timerfd >= 0);
	ATF_REQUIRE(errno == 0);

	ATF_REQUIRE(timerfd_settime(timerfd, 0,
			&(struct itimerspec) {
			    .it_value.tv_sec = 0,
			    .it_value.tv_nsec = 100000000,
			},
			NULL) == 0);
	ATF_REQUIRE(errno == 0);

	ATF_REQUIRE(timerfd_settime(timerfd, 0,
			&(struct itimerspec) {
			    .it_value.tv_sec = 630720000,
			    .it_value.tv_nsec = 0,
			},
			NULL) == 0);
	ATF_REQUIRE(errno == 0);

	struct pollfd pfd = { .fd = timerfd, .events = POLLIN };
	ATF_REQUIRE(poll(&pfd, 1, 500) == 0);
	uint64_t timeouts;
	ssize_t r = read(timerfd, &timeouts, sizeof(timeouts));
	ATF_REQUIRE_ERRNO(EAGAIN, r < 0);

	ATF_REQUIRE(close(timerfd) == 0);
	ATF_REQUIRE(errno == EAGAIN);
}

ATF_TP_ADD_TCS(tp)
{
	ATF_TP_ADD_TC(tp, timerfd__many_timers);
	ATF_TP_ADD_TC(tp, timerfd__simple_timer);
	ATF_TP_ADD_TC(tp, timerfd__simple_periodic_timer);
	ATF_TP_ADD_TC(tp, timerfd__complex_periodic_timer);
	ATF_TP_ADD_TC(tp, timerfd__reset_periodic_timer);
	ATF_TP_ADD_TC(tp, timerfd__reenable_periodic_timer);
	ATF_TP_ADD_TC(tp, timerfd__expire_five);
	ATF_TP_ADD_TC(tp, timerfd__simple_gettime);
	ATF_TP_ADD_TC(tp, timerfd__simple_blocking_periodic_timer);
	ATF_TP_ADD_TC(tp, timerfd__argument_checks);
	ATF_TP_ADD_TC(tp, timerfd__upgrade_simple_to_complex);
	ATF_TP_ADD_TC(tp, timerfd__absolute_timer);
	ATF_TP_ADD_TC(tp, timerfd__absolute_timer_in_the_past);
	ATF_TP_ADD_TC(tp, timerfd__reset_absolute);
	ATF_TP_ADD_TC(tp, timerfd__periodic_timer_performance);
	ATF_TP_ADD_TC(tp, timerfd__argument_overflow);
	ATF_TP_ADD_TC(tp, timerfd__short_evfilt_timer_timeout);
	ATF_TP_ADD_TC(tp, timerfd__unmodified_errno);
	ATF_TP_ADD_TC(tp, timerfd__reset_to_very_long);

	return atf_no_error();
}
