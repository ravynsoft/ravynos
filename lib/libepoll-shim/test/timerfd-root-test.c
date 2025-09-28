#define _GNU_SOURCE

#include <atf-c.h>

#include <sys/types.h>

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
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#include <sys/timerfd.h>

#include "atf-c-leakcheck.h"

static struct timespec current_time;
static void
reset_time(void)
{
	(void)clock_settime(CLOCK_REALTIME, &current_time);
}

static void
clock_settime_or_skip_test(clockid_t clockid, struct timespec const *ts)
{
	int r = clock_settime(clockid, ts);
	if (r < 0 && errno == EPERM) {
		atf_tc_skip("root required");
	}
	ATF_REQUIRE(r == 0);
}

ATF_TC(timerfd_root__zero_read_on_abs_realtime);
ATF_TC_HEAD(timerfd_root__zero_read_on_abs_realtime, tc)
{
	atf_tc_set_md_var(tc, "X-ctest.properties",
	    "RUN_SERIAL TRUE"
#ifdef __APPLE__
	    " ENVIRONMENT ASL_DISABLE=1"
#endif
	);
}
ATF_TC_BODY_FD_LEAKCHECK(timerfd_root__zero_read_on_abs_realtime, tc)
{
	bool netbsd_quirks = false;

	int tfd = timerfd_create(CLOCK_REALTIME, TFD_CLOEXEC);
	ATF_REQUIRE(tfd >= 0);

	ATF_REQUIRE(clock_gettime(CLOCK_REALTIME, &current_time) == 0);
	ATF_REQUIRE(atexit(reset_time) == 0);

	ATF_REQUIRE(timerfd_settime(tfd, TFD_TIMER_ABSTIME,
			&(struct itimerspec) {
			    .it_value = current_time,
			    .it_interval.tv_sec = 1,
			    .it_interval.tv_nsec = 0,
			},
			NULL) == 0);

	ATF_REQUIRE(
	    poll(&(struct pollfd) { .fd = tfd, .events = POLLIN }, 1, -1) == 1);

	clock_settime_or_skip_test(CLOCK_REALTIME,
	    &(struct timespec) {
		.tv_sec = current_time.tv_sec - 1,
		.tv_nsec = current_time.tv_nsec,
	    });

	uint64_t exp;
	ssize_t r = read(tfd, &exp, sizeof(exp));
	if (
#ifdef __NetBSD__
	    r == sizeof(exp) && exp == 1
#else
	    false
#endif
	) {
		netbsd_quirks = true;
	} else {
		ATF_REQUIRE_MSG(r == 0, "r: %d, errno: %d", (int)r, errno);
	}

	{
		int r = fcntl(tfd, F_GETFL);
		ATF_REQUIRE(r >= 0);
		r = fcntl(tfd, F_SETFL, r | O_NONBLOCK);
#ifdef __NetBSD__
		/* EPASSTHROUGH in userspace, should not happen. */
		if (r < 0 && errno == -4) {
			atf_tc_skip(
			    "NetBSD's native timerfd does not support F_SETFL.");
		}
#endif
		ATF_REQUIRE(r >= 0);
	}

	r = read(tfd, &exp, sizeof(exp));
	ATF_REQUIRE_ERRNO(EAGAIN, r < 0);

	current_time.tv_sec += 1;
	ATF_REQUIRE(poll(&(struct pollfd) { .fd = tfd, .events = POLLIN }, 1,
			1800) == 1);
	r = read(tfd, &exp, sizeof(exp));
	ATF_REQUIRE(r == (ssize_t)sizeof(exp));
	ATF_REQUIRE(exp == 1);

	ATF_REQUIRE(close(tfd) == 0);

	if (netbsd_quirks) {
		atf_tc_skip("NetBSD has some timerfd quirks");
	}
}

ATF_TC(timerfd_root__read_on_abs_realtime_no_interval);
ATF_TC_HEAD(timerfd_root__read_on_abs_realtime_no_interval, tc)
{
	atf_tc_set_md_var(tc, "X-ctest.properties",
	    "RUN_SERIAL TRUE"
#ifdef __APPLE__
	    " ENVIRONMENT ASL_DISABLE=1"
#endif
	);
}
ATF_TC_BODY_FD_LEAKCHECK(timerfd_root__read_on_abs_realtime_no_interval, tc)
{
	int tfd = timerfd_create(CLOCK_REALTIME, TFD_CLOEXEC);
	ATF_REQUIRE(tfd >= 0);

	ATF_REQUIRE(clock_gettime(CLOCK_REALTIME, &current_time) == 0);
	ATF_REQUIRE(atexit(reset_time) == 0);

	ATF_REQUIRE(timerfd_settime(tfd, TFD_TIMER_ABSTIME,
			&(struct itimerspec) {
			    .it_value = current_time,
			    .it_interval.tv_sec = 0,
			    .it_interval.tv_nsec = 0,
			},
			NULL) == 0);

	ATF_REQUIRE(
	    poll(&(struct pollfd) { .fd = tfd, .events = POLLIN }, 1, -1) == 1);

	clock_settime_or_skip_test(CLOCK_REALTIME,
	    &(struct timespec) {
		.tv_sec = current_time.tv_sec - 1,
		.tv_nsec = current_time.tv_nsec,
	    });

	uint64_t exp;
	ssize_t r = read(tfd, &exp, sizeof(exp));
	ATF_REQUIRE(r == (ssize_t)sizeof(exp));
	ATF_REQUIRE(exp == 1);

	ATF_REQUIRE(close(tfd) == 0);
}

ATF_TC(timerfd_root__cancel_on_set);
ATF_TC_HEAD(timerfd_root__cancel_on_set, tc)
{
	atf_tc_set_md_var(tc, "X-ctest.properties",
	    "RUN_SERIAL TRUE"
#ifdef __APPLE__
	    " ENVIRONMENT ASL_DISABLE=1"
#endif
	);
}
ATF_TC_BODY_FD_LEAKCHECK(timerfd_root__cancel_on_set, tc)
{
	bool netbsd_quirks = false;

	int tfd = timerfd_create(CLOCK_REALTIME, TFD_CLOEXEC);
	ATF_REQUIRE(tfd >= 0);

	ATF_REQUIRE(clock_gettime(CLOCK_REALTIME, &current_time) == 0);
	ATF_REQUIRE(atexit(reset_time) == 0);

	ATF_REQUIRE(
	    timerfd_settime(tfd, TFD_TIMER_ABSTIME | TFD_TIMER_CANCEL_ON_SET,
		&(struct itimerspec) {
		    .it_value.tv_sec = current_time.tv_sec + 10,
		    .it_value.tv_nsec = current_time.tv_nsec,
		    .it_interval.tv_sec = 0,
		    .it_interval.tv_nsec = 0,
		},
		NULL) == 0);

	clock_settime_or_skip_test(CLOCK_REALTIME, &current_time);

	ATF_REQUIRE(
	    poll(&(struct pollfd) { .fd = tfd, .events = POLLIN }, 1, -1) == 1);

	{
		int r = timerfd_settime(tfd,
		    TFD_TIMER_ABSTIME | TFD_TIMER_CANCEL_ON_SET,
		    &(struct itimerspec) {
			.it_value.tv_sec = current_time.tv_sec,
			.it_value.tv_nsec = current_time.tv_nsec,
			.it_interval.tv_sec = 0,
			.it_interval.tv_nsec = 0,
		    },
		    NULL);
		if (
#ifdef __NetBSD__
		    r == 0
#else
		    false
#endif
		) {
			netbsd_quirks = true;
		} else {
			ATF_REQUIRE_ERRNO(ECANCELED, r < 0);
		}
	}

	ATF_REQUIRE(poll(&(struct pollfd) { .fd = tfd, .events = POLLIN }, 1,
			800) == 1);

	uint64_t exp;
	ssize_t r;

	r = read(tfd, &exp, sizeof(exp));
	ATF_REQUIRE(r == (ssize_t)sizeof(exp));
	ATF_REQUIRE(exp == 1);

	ATF_REQUIRE(
	    timerfd_settime(tfd, TFD_TIMER_ABSTIME | TFD_TIMER_CANCEL_ON_SET,
		&(struct itimerspec) {
		    .it_value.tv_sec = current_time.tv_sec + 1,
		    .it_value.tv_nsec = current_time.tv_nsec,
		    .it_interval.tv_sec = 1,
		    .it_interval.tv_nsec = 0,
		},
		NULL) == 0);

	clock_settime_or_skip_test(CLOCK_REALTIME, &current_time);

	ATF_REQUIRE(
	    poll(&(struct pollfd) { .fd = tfd, .events = POLLIN }, 1, -1) == 1);

	r = read(tfd, &exp, sizeof(exp));
	ATF_REQUIRE_ERRNO(ECANCELED, r < 0);

	r = read(tfd, &exp, sizeof(exp));
	if (
#ifdef __NetBSD__
	    r < 0 && errno == ECANCELED
#else
	    false
#endif
	) {
		netbsd_quirks = true;
	} else {
		current_time.tv_sec += 1;
		ATF_REQUIRE_MSG(r == (ssize_t)sizeof(exp), "%d %d", (int)r,
		    errno);
		ATF_REQUIRE(exp == 1);
	}

	ATF_REQUIRE(
	    timerfd_settime(tfd, TFD_TIMER_ABSTIME | TFD_TIMER_CANCEL_ON_SET,
		&(struct itimerspec) {
		    .it_value.tv_sec = current_time.tv_sec + 1,
		    .it_value.tv_nsec = current_time.tv_nsec,
		    .it_interval.tv_sec = 1,
		    .it_interval.tv_nsec = 0,
		},
		NULL) == 0);

	clock_settime_or_skip_test(CLOCK_REALTIME, &current_time);
	current_time.tv_sec += 2;
	ATF_REQUIRE(nanosleep(&(struct timespec) { .tv_sec = 2 }, NULL) == 0);

	r = read(tfd, &exp, sizeof(exp));
	ATF_REQUIRE_ERRNO(ECANCELED, r < 0);

	r = poll(&(struct pollfd) { .fd = tfd, .events = POLLIN }, 1, 3000);
	if (
#ifdef __NetBSD__
	    r == 1
#else
	    false
#endif
	) {
		netbsd_quirks = true;
	} else {
		ATF_REQUIRE(r == 0);
		current_time.tv_sec += 3;
	}

	ATF_REQUIRE(close(tfd) == 0);

	if (netbsd_quirks) {
		atf_tc_skip("NetBSD has some timerfd quirks");
	}
}

ATF_TC(timerfd_root__cancel_on_set_init);
ATF_TC_HEAD(timerfd_root__cancel_on_set_init, tc)
{
	atf_tc_set_md_var(tc, "X-ctest.properties",
	    "RUN_SERIAL TRUE"
#ifdef __APPLE__
	    " ENVIRONMENT ASL_DISABLE=1"
#endif
	);
}
ATF_TC_BODY_FD_LEAKCHECK(timerfd_root__cancel_on_set_init, tc)
{
	int tfd = timerfd_create(CLOCK_REALTIME, TFD_CLOEXEC);
	ATF_REQUIRE(tfd >= 0);

	ATF_REQUIRE(clock_gettime(CLOCK_REALTIME, &current_time) == 0);
	ATF_REQUIRE(atexit(reset_time) == 0);

	clock_settime_or_skip_test(CLOCK_REALTIME, &current_time);

	ATF_REQUIRE(
	    timerfd_settime(tfd, TFD_TIMER_ABSTIME | TFD_TIMER_CANCEL_ON_SET,
		&(struct itimerspec) {
		    .it_value.tv_sec = current_time.tv_sec + 10,
		    .it_value.tv_nsec = current_time.tv_nsec,
		    .it_interval.tv_sec = 0,
		    .it_interval.tv_nsec = 0,
		},
		NULL) == 0);

	clock_settime_or_skip_test(CLOCK_REALTIME, &current_time);

	int r = timerfd_settime(tfd,
	    TFD_TIMER_ABSTIME | TFD_TIMER_CANCEL_ON_SET,
	    &(struct itimerspec) {
		.it_value.tv_sec = current_time.tv_sec + 10,
		.it_value.tv_nsec = current_time.tv_nsec,
		.it_interval.tv_sec = 0,
		.it_interval.tv_nsec = 0,
	    },
	    NULL);
	if (
#ifdef __NetBSD__
	    r == 0
#else
	    false
#endif
	) {
		atf_tc_skip("NetBSD has some timerfd quirks");
	} else {
		ATF_REQUIRE_ERRNO(ECANCELED, r < 0);
	}
	ATF_REQUIRE(close(tfd) == 0);
}

static void *
clock_change_thread(void *arg)
{
	(void)arg;

	fprintf(stderr, "clock change\n");
	clock_settime_or_skip_test(CLOCK_REALTIME, &current_time);

	current_time.tv_sec += 2;
	ATF_REQUIRE(nanosleep(&(struct timespec) { .tv_sec = 2 }, NULL) == 0);

	fprintf(stderr, "clock change\n");
	clock_settime_or_skip_test(CLOCK_REALTIME, &current_time);

	return NULL;
}

ATF_TC(timerfd_root__clock_change_notification);
ATF_TC_HEAD(timerfd_root__clock_change_notification, tc)
{
	atf_tc_set_md_var(tc, "timeout", "10");
	atf_tc_set_md_var(tc, "X-ctest.properties",
	    "RUN_SERIAL TRUE"
#ifdef __APPLE__
	    " ENVIRONMENT ASL_DISABLE=1"
#endif
	);
}
ATF_TC_BODY_FD_LEAKCHECK(timerfd_root__clock_change_notification, tc)
{
	ATF_REQUIRE(clock_gettime(CLOCK_REALTIME, &current_time) == 0);
	ATF_REQUIRE(atexit(reset_time) == 0);

	clock_settime_or_skip_test(CLOCK_REALTIME, &current_time);

#define TIME_T_MAX (time_t)((UINTMAX_C(1) << ((sizeof(time_t) << 3) - 1)) - 1)
	struct itimerspec its = {
		.it_value.tv_sec = TIME_T_MAX,
	};
#undef TIME_T_MAX

	int tfd = timerfd_create(CLOCK_REALTIME, TFD_CLOEXEC);
	ATF_REQUIRE(tfd >= 0);

	ATF_REQUIRE(
	    timerfd_settime(tfd, TFD_TIMER_ABSTIME | TFD_TIMER_CANCEL_ON_SET,
		&its, NULL) == 0);

	pthread_t clock_changer;
	ATF_REQUIRE(pthread_create(&clock_changer, NULL, /**/
			clock_change_thread, NULL) == 0);

	uint64_t exp;
	ssize_t r;

	r = read(tfd, &exp, sizeof(exp));
	ATF_REQUIRE_ERRNO(ECANCELED, r < 0);
	fprintf(stderr, "clock change detected\n");

	r = read(tfd, &exp, sizeof(exp));
	ATF_REQUIRE_ERRNO(ECANCELED, r < 0);
	fprintf(stderr, "clock change detected\n");

	ATF_REQUIRE(pthread_join(clock_changer, NULL) == 0);

	ATF_REQUIRE(close(tfd) == 0);
}

ATF_TC(timerfd_root__advance_time_no_cancel);
ATF_TC_HEAD(timerfd_root__advance_time_no_cancel, tc)
{
	atf_tc_set_md_var(tc, "X-ctest.properties",
	    "RUN_SERIAL TRUE"
#ifdef __APPLE__
	    " ENVIRONMENT ASL_DISABLE=1"
#endif
	);
}
ATF_TC_BODY_FD_LEAKCHECK(timerfd_root__advance_time_no_cancel, tc)
{
	int tfd = timerfd_create(CLOCK_REALTIME, TFD_CLOEXEC);
	ATF_REQUIRE(tfd >= 0);

	ATF_REQUIRE(clock_gettime(CLOCK_REALTIME, &current_time) == 0);
	ATF_REQUIRE(atexit(reset_time) == 0);

	ATF_REQUIRE(timerfd_settime(tfd, TFD_TIMER_ABSTIME,
			&(struct itimerspec) {
			    .it_value.tv_sec = current_time.tv_sec + 10,
			    .it_value.tv_nsec = current_time.tv_nsec,
			    .it_interval.tv_sec = 0,
			    .it_interval.tv_nsec = 0,
			},
			NULL) == 0);

	current_time.tv_sec += 9;
	clock_settime_or_skip_test(CLOCK_REALTIME, &current_time);
	current_time.tv_sec -= 8;

	{
		int r = poll(&(struct pollfd) { .fd = tfd, .events = POLLIN },
		    1, 1800);
#ifdef __NetBSD__
		if (r == 0) {
			atf_tc_skip("NetBSD has some timerfd quirks");
		}
#endif
		ATF_REQUIRE(r == 1);
	}

	uint64_t exp;
	ssize_t r;

	r = read(tfd, &exp, sizeof(exp));
	ATF_REQUIRE(r == (ssize_t)sizeof(exp));
	ATF_REQUIRE(exp == 1);

	ATF_REQUIRE(close(tfd) == 0);
}


ATF_TP_ADD_TCS(tp)
{
	ATF_TP_ADD_TC(tp, timerfd_root__zero_read_on_abs_realtime);
	ATF_TP_ADD_TC(tp, timerfd_root__read_on_abs_realtime_no_interval);
	ATF_TP_ADD_TC(tp, timerfd_root__cancel_on_set);
	ATF_TP_ADD_TC(tp, timerfd_root__cancel_on_set_init);
	ATF_TP_ADD_TC(tp, timerfd_root__clock_change_notification);
	ATF_TP_ADD_TC(tp, timerfd_root__advance_time_no_cancel);

	return atf_no_error();
}
