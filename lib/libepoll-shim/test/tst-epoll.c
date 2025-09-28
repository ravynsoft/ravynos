/*
 * Based on:
 * https://raw.githubusercontent.com/cloudius-systems/osv/master/tests/tst-epoll.cc
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

#define _GNU_SOURCE

#include <atf-c.h>

#include <sys/epoll.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <errno.h>

#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>

#include "atf-c-leakcheck.h"

static void
write_one(int fd)
{
	char c = 0;
	ssize_t w = write(fd, &c, 1);
	ATF_REQUIRE_MSG(w == 1, "write");
}

static struct timespec
steady_clock_now(void)
{
	struct timespec ts;
	ATF_REQUIRE(clock_gettime(CLOCK_MONOTONIC, &ts) == 0);
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

#define MAXEVENTS (1024)

ATF_TC_WITHOUT_HEAD(epollfd_osv__epolloneshot);
ATF_TC_BODY_FD_LEAKCHECK(epollfd_osv__epolloneshot, tc)
{
	struct epoll_event events[MAXEVENTS];

	int ep = epoll_create(1);
	ATF_REQUIRE_MSG(ep >= 0, "epoll_create");

	int s[2];
	int r = pipe(s);
	ATF_REQUIRE_MSG(r == 0, "create pipe");

	struct epoll_event event;
	event.events = EPOLLIN | EPOLLONESHOT;
	event.data.u32 = 123;

	r = epoll_ctl(ep, EPOLL_CTL_ADD, s[0], &event);
	ATF_REQUIRE_MSG(r == 0, "epoll_ctl ADD");

	write_one(s[1]);

	r = epoll_wait(ep, events, MAXEVENTS, 0);
	ATF_REQUIRE_MSG(r == 1, "epoll_wait");

	write_one(s[1]);

	// Test that new event is not delivered
	r = epoll_wait(ep, events, MAXEVENTS, 0);
	ATF_REQUIRE_MSG(r == 0, "epoll_wait");

	// Unblock the descriptor
	r = epoll_ctl(ep, EPOLL_CTL_MOD, s[0], &event);
	ATF_REQUIRE_MSG(r == 0, "epoll_ctl MOD");

	// The event should be ready now
	r = epoll_wait(ep, events, MAXEVENTS, 0);
	ATF_REQUIRE_MSG(r == 1, "epoll_wait");

	// Test that one-shotness does not remove the descriptor
	r = epoll_ctl(ep, EPOLL_CTL_ADD, s[0], &event);
	ATF_REQUIRE_MSG(r == -1, "epoll_ctl ADD");
	ATF_REQUIRE_MSG(errno == EEXIST, "errno == EEXIST");

	r = epoll_ctl(ep, EPOLL_CTL_DEL, s[0], &event);
	ATF_REQUIRE_MSG(r == 0, "epoll_ctl DEL");

	ATF_REQUIRE(close(ep) == 0);
	ATF_REQUIRE(close(s[0]) == 0);
	ATF_REQUIRE(close(s[1]) == 0);
}

ATF_TC_WITHOUT_HEAD(epollfd_osv__epoll_file);
ATF_TC_BODY_FD_LEAKCHECK(epollfd_osv__epoll_file, tc)
{
#if defined(__APPLE__)
	atf_tc_skip("/dev/random not pollable under macOS");
#endif

	struct epoll_event events[MAXEVENTS];

	int ep = epoll_create(1);
	ATF_REQUIRE_MSG(ep >= 0, "epoll_create");

	int fd = open("/dev/random", O_RDONLY);
	ATF_REQUIRE_MSG(fd >= 0, "open file");

	struct pollfd pfd = { .fd = fd, .events = POLLIN };
	ATF_REQUIRE_MSG(poll(&pfd, 1, -1) == 1 && pfd.revents == EPOLLIN,
	    "poll");

	struct epoll_event event;
	event.events = EPOLLIN | EPOLLONESHOT;
	event.data.u32 = 123;

	int r = epoll_ctl(ep, EPOLL_CTL_ADD, fd, &event);
	ATF_REQUIRE_MSG(r == 0, "epoll_ctl ADD");

	r = epoll_wait(ep, events, MAXEVENTS, -1);
	ATF_REQUIRE_MSG(r == 1, "epoll_wait");

	r = epoll_wait(ep, events, MAXEVENTS, 0);
	ATF_REQUIRE_MSG(r == 0, "epoll_wait");

	r = epoll_ctl(ep, EPOLL_CTL_DEL, fd, &event);
	ATF_REQUIRE_MSG(r == 0, "epoll_ctl DEL");

	ATF_REQUIRE_MSG(close(fd) == 0, "close");
	ATF_REQUIRE_MSG(close(ep) == 0, "close");
}

struct t1_args {
	int ep;
	int s_0;
};

static void *
t1_func(void *arg)
{
	struct t1_args *t1_args = arg;
	int ep = t1_args->ep;
	int s_0 = t1_args->s_0;
	struct epoll_event events[MAXEVENTS];
	int r;

	int r2 = epoll_wait(ep, events, MAXEVENTS, 5000);
	ATF_REQUIRE_MSG(r2 == 1 && (events[0].events & EPOLLIN) &&
		(events[0].data.u32 == 123),
	    "epoll_wait in thread");

	char c;
	r2 = (int)read(s_0, &c, 1);
	ATF_REQUIRE_MSG(r2 == 1, "read after poll");

	r = epoll_wait(ep, events, MAXEVENTS, 0);
	ATF_REQUIRE_MSG(r == 0, "epoll after read");

	return NULL;
}

ATF_TC_WITHOUT_HEAD(epollfd_osv__some_tests);
ATF_TC_BODY_FD_LEAKCHECK(epollfd_osv__some_tests, tc)
{
	int ep = epoll_create(1);
	ATF_REQUIRE_MSG(ep >= 0, "epoll_create");

	int s[2];
	int r = pipe(s);
	ATF_REQUIRE_MSG(r == 0, "create pipe");

	struct epoll_event events[MAXEVENTS];

	r = epoll_wait(ep, events, MAXEVENTS, 0);
	ATF_REQUIRE_MSG(r == 0, "epoll_wait for empty epoll");

	r = epoll_wait(s[0], events, MAXEVENTS, 0);
	ATF_REQUIRE_MSG(r == -1 && errno == EINVAL,
	    "epoll_wait on non-epoll fd");

	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.u32 = 123;
	r = epoll_ctl(ep, EPOLL_CTL_ADD, s[0], &event);
	ATF_REQUIRE_MSG(r == 0, "epoll_ctl_add");

	r = epoll_wait(ep, events, MAXEVENTS, 0);
	ATF_REQUIRE_MSG(r == 0, "epoll_wait returns nothing");

	char c = 'N';
	r = (int)write(s[1], &c, 1);
	ATF_REQUIRE_MSG(r == 1, "write single character");

	struct pollfd p;
	p.fd = s[0];
	p.events = POLLIN;
	r = poll(&p, 1, 0);
	ATF_REQUIRE_MSG(r == 1 && (p.revents & POLLIN), "poll finds fd");

	r = epoll_wait(ep, events, MAXEVENTS, 0);
	ATF_REQUIRE_MSG(r == 1 && (events[0].events & EPOLLIN) &&
		(events[0].data.u32 == 123),
	    "epoll_wait finds fd");

	r = epoll_wait(ep, events, MAXEVENTS, 0);
	ATF_REQUIRE_MSG(r == 1 && (events[0].events & EPOLLIN) &&
		(events[0].data.u32 == 123),
	    "epoll_wait finds again (because not EPOLLET)");

	r = (int)read(s[0], &c, 1);
	ATF_REQUIRE_MSG(r == 1, "read after poll");

	r = epoll_wait(ep, events, MAXEVENTS, 0);
	ATF_REQUIRE_MSG(r == 0, "epoll after read");

	pthread_t t1;
	struct t1_args t1_args = { .ep = ep, .s_0 = s[0] };
	ATF_REQUIRE(pthread_create(&t1, NULL, t1_func, &t1_args) == 0);

	ATF_REQUIRE(usleep(500000) == 0);
	r = (int)write(s[1], &c, 1);
	ATF_REQUIRE_MSG(r == 1, "write single character");
	ATF_REQUIRE(pthread_join(t1, NULL) == 0);

	struct timespec ts = steady_clock_now();
	r = epoll_wait(ep, events, MAXEVENTS, 300);
	struct timespec te = steady_clock_now();
	ATF_REQUIRE_MSG(r == 0 && (nano_diff(te, ts) > 200000000),
	    "epoll timeout");

#if 0
	////////////////////////////////////////////////////////////////////////////
	event.events = EPOLLIN | EPOLLET;
	r = epoll_ctl(ep, EPOLL_CTL_MOD, s[0], &event);
	ATF_REQUIRE_MSG(r == 0, "epoll_ctl_mod");

	latch first_epoll_wait_returned;
	std::thread t2([&] {
		int r = epoll_wait(ep, events, MAXEVENTS, 5000);
		ATF_REQUIRE_MSG(r == 1, "epoll_wait in thread");
		first_epoll_wait_returned.count_down();
		r = epoll_wait(ep, events, MAXEVENTS, 5000);
		ATF_REQUIRE_MSG(r == 1, "epoll_wait in thread");
		r = read(s[0], &c, 1);
		ATF_REQUIRE_MSG(r == 1, "read the last byte on the pipe");
	});
	r = write(s[1], &c, 1);
	ATF_REQUIRE_MSG(r == 1, "write single character");
	first_epoll_wait_returned.await();
	r = epoll_ctl(ep, EPOLL_CTL_MOD, s[0], &event);
	ATF_REQUIRE_MSG(r == 0, "epoll_ctl_mod");
	t2.join();
#endif

	////////////////////////////////////////////////////////////////////////////
	// Test EPOLLET (edge-triggered event notification)
	// Also test EPOLL_CTL_MOD at the same time.
	event.events = EPOLLIN | EPOLLET;
	event.data.u32 = 456;
	r = epoll_ctl(ep, EPOLL_CTL_MOD, s[0], &event);
	ATF_REQUIRE_MSG(r == 0, "epoll_ctl_mod");
	r = epoll_wait(ep, events, MAXEVENTS, 0);
	ATF_REQUIRE_MSG(r == 0, "epoll nothing happened yet");
	r = (int)write(s[1], &c, 1);
	ATF_REQUIRE_MSG(r == 1, "write single character");
	r = epoll_wait(ep, events, MAXEVENTS, 0);
	ATF_REQUIRE_MSG(r == 1 && (events[0].events & EPOLLIN) &&
		(events[0].data.u32 == 456),
	    "epoll_wait finds fd");
	r = epoll_wait(ep, events, MAXEVENTS, 0);
	ATF_REQUIRE_MSG(r == 0,
	    "epoll_wait doesn't find again (because of EPOLLET)");
	// Also verify that epoll_wait with a timeout doesn't return
	// immediately, (despite fp->poll() being true right after
	// poll_install()).
	ts = steady_clock_now();
	r = epoll_wait(ep, events, MAXEVENTS, 300);
	te = steady_clock_now();
	ATF_REQUIRE_MSG(r == 0 && (nano_diff(te, ts) > 200000000),
	    "epoll timeout doesn't return immediately (EPOLLET)");
	// The accurate edge-triggered behavior of EPOLLET means that until the
	// all the data is read from the pipe, epoll should not find the pipe
	// again even if new data comes in. However, both Linux and OSv gives a
	// "false positive" where if new data arrives, epoll will return even
	// if the data was not fully read previously.
	r = (int)write(s[1], &c, 1);
	ATF_REQUIRE_MSG(r == 1, "write single character");
	r = epoll_wait(ep, events, MAXEVENTS, 0);
	ATF_REQUIRE_MSG(r == 0 ||
		(r == 1 && (events[0].events & EPOLLIN) &&
		    (events[0].data.u32 == 456)),
	    "epoll_wait false positive (fine)");
	r = (int)read(s[0], &c, 1);
	ATF_REQUIRE_MSG(r == 1, "read one byte out of 2 on the pipe");
	// We only read one byte out of the 2 on the pipe, so there's still
	// data on the pipe (and poll() verifies this), but with EPOLLET, epoll
	// won't return it.
#if !defined(__linux__) && !defined(__DragonFly__)
	// kqueue based emulation will trigger an edge here.
	r = epoll_wait(ep, events, MAXEVENTS, 0);
	ATF_REQUIRE(r == 1);
#endif
	r = epoll_wait(ep, events, MAXEVENTS, 0);
	ATF_REQUIRE_MSG(r == 0,
	    "now epoll_wait doesn't find this fd (EPOLLET)");
	r = poll(&p, 1, 0);
	ATF_REQUIRE_MSG(r == 1 && (p.revents & POLLIN),
	    "but poll() does find this fd");
	r = (int)read(s[0], &c, 1);
	ATF_REQUIRE_MSG(r == 1, "read the last byte on the pipe");

	int s2 = dup(s[0]);
	r = epoll_ctl(ep, EPOLL_CTL_ADD, s2, &event);
	ATF_REQUIRE_MSG(r == 0, "add dup() fd");
	r = epoll_ctl(ep, EPOLL_CTL_DEL, s2, &event);
	ATF_REQUIRE_MSG(r == 0, "del fd");
	r = epoll_ctl(ep, EPOLL_CTL_ADD, s[0], &event);
	ATF_REQUIRE_MSG(r == -1 && errno == EEXIST, "EEXIST");

	ATF_REQUIRE(close(s2) == 0);
	ATF_REQUIRE(close(s[0]) == 0);
	ATF_REQUIRE(close(s[1]) == 0);
	ATF_REQUIRE(close(ep) == 0);
}

ATF_TP_ADD_TCS(tp)
{
	ATF_TP_ADD_TC(tp, epollfd_osv__epolloneshot);
	ATF_TP_ADD_TC(tp, epollfd_osv__epoll_file);
	ATF_TP_ADD_TC(tp, epollfd_osv__some_tests);

	return atf_no_error();
}
