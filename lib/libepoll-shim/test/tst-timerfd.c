
/*
 * Based on:
 * https://raw.githubusercontent.com/cloudius-systems/osv/master/tests/tst-timerfd.cc
 *
 * https://github.com/cloudius-systems/osv
 * https://github.com/cloudius-systems/osv/blob/master/LICENSE
 */

#if 0
Copyright (C) 2013 Cloudius Systems, Ltd.

Parts are copyright by other contributors. Please refer to copyright notices
in the individual source files, and to the git commit log, for a more accurate
list of copyright holders.

OSv is open-source software, distributed under the 3-clause BSD license:

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.

    * Neither the name of the Cloudius Systems, Ltd. nor the names of its
      contributors may be used to endorse or promote products derived from this
      software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
    OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

This project also includes source code adopted and adapted from four other
open-source projects - FreeBSD, OpenSolaris, Prex and Musl. These projects have
their own licenses. Please refer to the files documentation/LICENSE-*
for the licenses and copyright statements of these projects.
#endif

/*
 * Copyright (C) 2013 Cloudius Systems, Ltd.
 *
 * This work is open source software, licensed under the terms of the
 * BSD license as described in the LICENSE file in the top-level directory.
 */
// To compile on Linux, use: g++ -g -pthread -std=c++11 tests/tst-timerfd.cc

#include <atf-c.h>

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <poll.h>
#include <pthread.h>
#include <unistd.h>

#include <sys/timerfd.h>

#define do_expect(actual, expected, actuals, expecteds, file, line) \
	do {                                                        \
		if (actual != expected) {                           \
			fprintf(stderr,                             \
			    "expecteds: %s, actuals: %s, "          \
			    "file: %s, line: %d\n",                 \
			    expecteds, actuals, file, line);        \
			abort();                                    \
		}                                                   \
	} while (0)

#define do_expectge(actual, expected, actuals, expecteds, file, line) \
	do {                                                          \
		if (actual < expected) {                              \
			fprintf(stderr,                               \
			    "expecteds: %s, actuals: %s, "            \
			    "file: %s, line: %d\n",                   \
			    expecteds, actuals, file, line);          \
			abort();                                      \
		}                                                     \
	} while (0)

#define expect(actual, expected) \
	do_expect(actual, expected, #actual, #expected, __FILE__, __LINE__)
#define expectge(actual, expected) \
	do_expectge(actual, expected, #actual, #expected, __FILE__, __LINE__)
#define expect_errno(call, experrno)                                        \
	do_expect((long)(call), (long)-1, #call, "-1", __FILE__, __LINE__); \
	do_expect(errno, experrno, #call " errno", #experrno, __FILE__,     \
	    __LINE__);
#define expect_success(var, call)                            \
	errno = 0;                                           \
	var = call;                                          \
	do_expectge(var, 0, #call, "0", __FILE__, __LINE__); \
	do_expect(errno, 0, #call " errno", "0", __FILE__, __LINE__);

#define MS_TO_NSEC(x) ((long long)(x * 1000000LL))

#define u64 uint64_t

static struct timespec
steady_clock_now(void)
{
	struct timespec ts;
	if (clock_gettime(CLOCK_MONOTONIC, &ts) < 0) {
		abort();
	}
	return ts;
}

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

static int64_t
nano_diff(struct timespec end, struct timespec begin)
{
	struct timespec diff;
	timespecsub(&end, &begin, &diff);
	return diff.tv_sec * 1000000000LL + diff.tv_nsec;
}

static void *
th2_func(void *arg)
{
	int fd = *(int *)arg;
	u64 counter2;
	struct timespec before = steady_clock_now();
	expect(read(fd, &counter2, sizeof(counter2)), (ssize_t)8);
	expect(counter2, (u64)1);
	struct timespec after = steady_clock_now();
	expectge(nano_diff(after, before), (int64_t)MS_TO_NSEC(500));
	return NULL;
}

static void
dotest(int clockid)
{
	int fd;
	expect_success(fd, timerfd_create(clockid, TFD_NONBLOCK));
	int junk;

	// Before we set a timer, timerfd has no events for polling or reading
	struct pollfd pfd = { .fd = fd, .events = POLLIN };
	expect(poll(&pfd, 1, 0), 0);
	char buf[1024];
	expect_errno(read(fd, buf, sizeof(buf)), EAGAIN);
	// Reading less than 8 bytes result in an error
	expect_errno(read(fd, buf, 7), EINVAL);

	// Set a timer to expire in 500ms from now (relative time)
	struct itimerspec t1 = { { 0, 0 }, { 0, MS_TO_NSEC(500) } };
	expect_success(junk, timerfd_settime(fd, 0, &t1, NULL));
	// If we check now, the timer should have not yet expired
	expect_errno(read(fd, buf, sizeof(buf)), EAGAIN);
	// But if we sleep for a whole second, the timer would have expired and
	// read returns a counter of 1
	sleep(1);
	u64 counter;
	expect(read(fd, &counter, sizeof(counter)), (ssize_t)8);
	expect(counter, (u64)1);
	expect_errno(read(fd, &counter, sizeof(counter)), EAGAIN);

	// Similarly, set a timer to expire in 300ms, and then again every
	// 200ms. After 400ms, poll sees the timer expired at least once, but
	// we do not read. After 400ms more, we read and see the timer expired
	// 3 times.
	struct itimerspec t2 = { { 0, MS_TO_NSEC(200) },
		{ 0, MS_TO_NSEC(300) } };
	expect_success(junk, timerfd_settime(fd, 0, &t2, NULL));
	expect_errno(read(fd, buf, sizeof(buf)), EAGAIN);
	expect(usleep(400000), 0);
#ifdef ALLOW_TIMER_SLACK
	expect(poll(&pfd, 1, -1), 1);
#else
	expect(poll(&pfd, 1, 0), 1);
#endif
	expect((int)pfd.revents, POLLIN);
	expect(usleep(400000), 0);
#ifdef ALLOW_TIMER_SLACK
	expect(poll(&pfd, 1, -1), 1);
#else
	expect(poll(&pfd, 1, 0), 1);
#endif
	expect((int)pfd.revents, POLLIN);
	expect(read(fd, &counter, sizeof(counter)), (ssize_t)8);
	expectge(counter, (u64)3);
#ifndef ALLOW_TIMER_SLACK
	expect(counter, (u64)3);
#endif
	expect_errno(read(fd, &counter, sizeof(counter)), EAGAIN);

	// If sleep (so counter becomes nonzero again) and then cancel the
	// timer, the counter remains zero
	usleep(400000);
	expect(poll(&pfd, 1, 0), 1); // counter is nonzero
	struct itimerspec t3 = { { 0, 0 }, { 0, 0 } };
	expect_success(junk, timerfd_settime(fd, 0, &t3, NULL));
	expect(poll(&pfd, 1, 0), 0); // counter is back to zero
	usleep(400000);
	expect(poll(&pfd, 1, 0), 0); // and timer was really canceled

	// Check absolute time setting
	// Set a timer to expire in 300ms from now (using absolute time)
	struct itimerspec t4 = { { 0, 0 }, { 0, 0 } };
	clock_gettime(clockid, &(t4.it_value));
	t4.it_value.tv_nsec += MS_TO_NSEC(300);
	if (t4.it_value.tv_nsec >= MS_TO_NSEC(1000)) {
		t4.it_value.tv_nsec -= MS_TO_NSEC(1000);
		t4.it_value.tv_sec++;
	}
	expect_success(junk, timerfd_settime(fd, TFD_TIMER_ABSTIME, &t4, NULL));
	expect_errno(read(fd, buf, sizeof(buf)), EAGAIN);
	usleep(500000);
	expect(read(fd, &counter, sizeof(counter)), (ssize_t)8);
	expect(counter, (u64)1);
	expect_errno(read(fd, &counter, sizeof(counter)), EAGAIN);

	// Check blocking poll - simple case, no interval (see more complex
	// cases in blocking read tests below).
	struct itimerspec t45 = { { 0, 0 }, { 0, MS_TO_NSEC(400) } };
	expect_success(junk, timerfd_settime(fd, 0, &t45, NULL));
	struct timespec before = steady_clock_now();
	expect(poll(&pfd, 1, 0), 0);
	expect(poll(&pfd, 1, 100 /*ms*/), 0);
	expect(poll(&pfd, 1, 1000 /*ms*/), 1);
	struct timespec after = steady_clock_now();
	expectge(nano_diff(after, before), (int64_t)MS_TO_NSEC(300));

	// Check close
	expect_success(junk, close(fd));

	// Check that it's not a disaster to close an fd with a yet-unexpired
	// timer
	expect_success(fd, timerfd_create(CLOCK_REALTIME, 0));
	pfd.fd = fd;
	struct itimerspec t46 = { { 0, 0 }, { 10, 0 } };
	expect_success(junk, timerfd_settime(fd, 0, &t46, NULL));
	expect(poll(&pfd, 1, 0), 0);
	expect_success(junk, close(fd));

	// Open again with blocking read enabled
	expect_success(fd, timerfd_create(CLOCK_REALTIME, 0));
	pfd.fd = fd;

	// Check blocking read, no interval, timer set before read blocks
	struct itimerspec t5 = { { 0, 0 }, { 0, MS_TO_NSEC(500) } };
	expect_success(junk, timerfd_settime(fd, 0, &t5, NULL));
	before = steady_clock_now();
	expect(read(fd, &counter, sizeof(counter)), (ssize_t)8);
	expect(counter, (u64)1);
	after = steady_clock_now();
	expectge(nano_diff(after, before), (int64_t)MS_TO_NSEC(400));

	// Check blocking read, no interval, timer set after read blocks
	pthread_t th2;
	expect(pthread_create(&th2, NULL, th2_func, &fd), 0);
	usleep(400000);
	struct itimerspec t6 = { { 0, 0 }, { 0, MS_TO_NSEC(200) } };
	expect_success(junk, timerfd_settime(fd, 0, &t6, NULL));
	expect(pthread_join(th2, NULL), 0);

	// Check blocking read, with interval
	struct itimerspec t7 = { { 0, MS_TO_NSEC(200) },
		{ 0, MS_TO_NSEC(500) } };
	expect_success(junk, timerfd_settime(fd, 0, &t7, NULL));
	before = steady_clock_now();
	expect(read(fd, &counter, sizeof(counter)), (ssize_t)8);
	expect(counter, (u64)1);
	expect(read(fd, &counter, sizeof(counter)), (ssize_t)8);
	expect(counter, (u64)1);
	after = steady_clock_now();
	expectge(nano_diff(after, before), (int64_t)MS_TO_NSEC(600));

	// Check timerfd_gettime():
	struct itimerspec t8 = { { 0, MS_TO_NSEC(400) },
		{ 0, MS_TO_NSEC(300) } };
	expect_success(junk, timerfd_settime(fd, 0, &t8, NULL));
	struct itimerspec tout;
	// Right in the beginning:
	expect_success(junk, timerfd_gettime(fd, &tout));
	expect(tout.it_interval.tv_sec, t8.it_interval.tv_sec);
	expect(tout.it_interval.tv_nsec, t8.it_interval.tv_nsec);
	expect(tout.it_value.tv_sec, t8.it_value.tv_sec);
	expectge(t8.it_value.tv_nsec, tout.it_value.tv_nsec);
	expectge(tout.it_value.tv_nsec,
	    t8.it_value.tv_nsec - (long)MS_TO_NSEC(100));
	// After a while but before expiration:
	usleep(100000);
	expect_success(junk, timerfd_gettime(fd, &tout));
	expect(tout.it_interval.tv_sec, t8.it_interval.tv_sec);
	expect(tout.it_interval.tv_nsec, t8.it_interval.tv_nsec);
	expect(tout.it_value.tv_sec, t8.it_value.tv_sec);
	expectge(t8.it_value.tv_nsec - (long)MS_TO_NSEC(100),
	    tout.it_value.tv_nsec);
#ifndef ALLOW_TIMER_SLACK
	expectge(tout.it_value.tv_nsec,
	    t8.it_value.tv_nsec - (long)MS_TO_NSEC(200));
#endif
	// After expiration, we have the interval
	usleep(300000);
	expect_success(junk, timerfd_gettime(fd, &tout));
	expect(tout.it_interval.tv_sec, t8.it_interval.tv_sec);
	expect(tout.it_interval.tv_nsec, t8.it_interval.tv_nsec);
	expect(tout.it_value.tv_sec, t8.it_value.tv_sec);
#ifndef ALLOW_TIMER_SLACK
	expectge(tout.it_value.tv_nsec, (long)MS_TO_NSEC(200));
	expectge((long)MS_TO_NSEC(400), tout.it_value.tv_nsec);
#endif

	// Check timerfd_gettime() after expiration of a single-time timer:
	struct itimerspec t9 = { { 0, 0 }, { 0, MS_TO_NSEC(100) } };
	expect_success(junk, timerfd_settime(fd, 0, &t9, NULL));
	usleep(200000);
	expect_success(junk, timerfd_gettime(fd, &tout));
	expect(tout.it_interval.tv_sec, (long)0);
	expect(tout.it_interval.tv_nsec, (long)0);
	expect(tout.it_value.tv_sec, (long)0);
	expect(tout.it_value.tv_nsec, (long)0);
	expect(tout.it_value.tv_nsec, (long)0);
}

ATF_TC_WITHOUT_HEAD(timerfd_osv__clock_realtime);
ATF_TC_BODY(timerfd_osv__clock_realtime, tc)
{
	dotest(CLOCK_REALTIME);
}

ATF_TC_WITHOUT_HEAD(timerfd_osv__clock_monotonic);
ATF_TC_BODY(timerfd_osv__clock_monotonic, tc)
{
	dotest(CLOCK_MONOTONIC);
}

ATF_TP_ADD_TCS(tp)
{
	ATF_TP_ADD_TC(tp, timerfd_osv__clock_realtime);
	ATF_TP_ADD_TC(tp, timerfd_osv__clock_monotonic);

	return atf_no_error();
}
