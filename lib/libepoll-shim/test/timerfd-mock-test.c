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
#include <unistd.h>

#include <sys/timerfd.h>

static unsigned int evfilt_timer_fflags;
static int64_t evfilt_timer_data;
static int kevent_called;

#ifndef __linux__

#ifdef __NetBSD__
#define kevent_n_type size_t
#else
#define kevent_n_type int
#endif

int
kevent(int kq, const struct kevent *changelist, kevent_n_type nchanges,
    struct kevent *eventlist, kevent_n_type nevents,
    const struct timespec *timeout)
{
	(void)kq;
	(void)eventlist;
	(void)nevents;
	(void)timeout;

	++kevent_called;

	if (nchanges == 1) {
		ATF_REQUIRE(changelist[0].filter == EVFILT_TIMER);
		evfilt_timer_fflags = changelist[0].fflags;
		evfilt_timer_data = changelist[0].data;
	} else if (nchanges == 2) {
		ATF_REQUIRE(changelist[0].filter == EVFILT_TIMER);
		ATF_REQUIRE(changelist[1].filter == EVFILT_TIMER);
		evfilt_timer_fflags = changelist[1].fflags;
		evfilt_timer_data = changelist[1].data;
	} else {
		ATF_REQUIRE(false);
	}

	errno = ENOSYS;
	return -1;
}

#if defined(__NetBSD__) && __NetBSD_Version__ < 999009100
static unsigned int
netbsd_mstohz(unsigned int ms)
{
	return ms >= 0x20000u ? (ms / 1000u) * CLK_TCK : (ms * CLK_TCK) / 1000u;
}
#endif

#endif

ATF_TC_WITHOUT_HEAD(timerfd_mock__mocked_kevent);
ATF_TC_BODY(timerfd_mock__mocked_kevent, tc)
{
	int tfd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
	ATF_REQUIRE(tfd >= 0);

	for (int64_t ms = 1; ms < 3600000; ++ms) {
		struct itimerspec time = {
			.it_value.tv_sec = ms / 1000,
			.it_value.tv_nsec = (ms % 1000) * 1000000L,
		};

		kevent_called = 0;

		int r;
		if ((r = timerfd_settime(tfd, 0, &time, NULL)) == 0) {
			atf_tc_skip("kevent could not be mocked");
		}
		ATF_REQUIRE_ERRNO(ENOSYS, r < 0);

		ATF_REQUIRE(kevent_called == 1 || kevent_called == 2);

#ifndef __linux__
#if defined(__FreeBSD__) || \
    (defined(__NetBSD__) && __NetBSD_Version__ >= 999009100)
		if (evfilt_timer_fflags == 0) {
			ATF_REQUIRE(evfilt_timer_data == ms);
		} else {
			ATF_REQUIRE(evfilt_timer_fflags == NOTE_USECONDS);
			ATF_REQUIRE(evfilt_timer_data == ms * 1000);
		}
#else
		ATF_REQUIRE(evfilt_timer_fflags == 0);
		ATF_REQUIRE(evfilt_timer_data >= ms);
		ATF_REQUIRE(evfilt_timer_data <= UINT_MAX);

#if defined(__NetBSD__) && __NetBSD_Version__ < 999009100
		unsigned long kernel_ms = netbsd_mstohz(
					      (unsigned int)evfilt_timer_data) *
		    1000UL / CLK_TCK;
#else
		unsigned long kernel_ms = evfilt_timer_data;
#endif

		ATF_REQUIRE(kernel_ms >= ms);
#endif
#endif
	}

	ATF_REQUIRE(close(tfd) == 0);
}

ATF_TP_ADD_TCS(tp)
{
	ATF_TP_ADD_TC(tp, timerfd_mock__mocked_kevent);

	return atf_no_error();
}
