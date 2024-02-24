#define _GNU_SOURCE

#include <atf-c.h>

#include <sys/types.h>

#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <errno.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

#include "atf-c-leakcheck.h"

#ifndef nitems
#define nitems(x) (sizeof((x)) / sizeof((x)[0]))
#endif

ATF_TC_WITHOUT_HEAD(eventfd__constants);
ATF_TC_BODY(eventfd__constants, tc)
{
	ATF_REQUIRE(EFD_CLOEXEC == O_CLOEXEC);
	ATF_REQUIRE(EFD_NONBLOCK == O_NONBLOCK);
}

ATF_TC_WITHOUT_HEAD(eventfd__pollin);
ATF_TC_BODY_FD_LEAKCHECK(eventfd__pollin, tc)
{
	int efd;

	ATF_REQUIRE((efd = eventfd(0,
			 EFD_CLOEXEC | EFD_NONBLOCK | EFD_SEMAPHORE)) >= 0);
	{
		struct pollfd pfd = { .fd = efd, .events = POLLIN };
		ATF_REQUIRE(poll(&pfd, 1, 0) == 0);
	}
	ATF_REQUIRE(close(efd) == 0);

	ATF_REQUIRE((efd = eventfd(1,
			 EFD_CLOEXEC | EFD_NONBLOCK | EFD_SEMAPHORE)) >= 0);
	{
		struct pollfd pfd = { .fd = efd, .events = POLLIN };
		ATF_REQUIRE(poll(&pfd, 1, 0) == 1);
		ATF_REQUIRE(pfd.revents == POLLIN);
	}
	ATF_REQUIRE(close(efd) == 0);
}

ATF_TC_WITHOUT_HEAD(eventfd__pollout);
ATF_TC_BODY_FD_LEAKCHECK(eventfd__pollout, tc)
{
	int efd;

	ATF_REQUIRE((efd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK)) >= 0);
	{
		struct pollfd pfd = { .fd = efd, .events = POLLOUT };

		if (poll(&pfd, 1, 0) == 0) {
			atf_tc_skip(
			    "shim layer does not support polling for write");
		}

		ATF_REQUIRE(poll(&pfd, 1, 0) == 1);
		if (pfd.revents == POLLNVAL) {
			atf_tc_skip(
			    "shim layer does not support polling for write");
		}
		ATF_REQUIRE_MSG(pfd.revents == POLLOUT, "%x", pfd.revents);
	}
	ATF_REQUIRE(close(efd) == 0);

	ATF_REQUIRE((efd = eventfd(1, EFD_CLOEXEC | EFD_NONBLOCK)) >= 0);
	{
		struct pollfd pfd = { .fd = efd, .events = POLLOUT };
		ATF_REQUIRE(poll(&pfd, 1, 0) == 1);
		ATF_REQUIRE(pfd.revents == POLLOUT);
	}
	ATF_REQUIRE(close(efd) == 0);

	ATF_REQUIRE((efd = eventfd(UINT_MAX, /**/
			 EFD_CLOEXEC | EFD_NONBLOCK)) >= 0);
	{
		{
			struct pollfd pfd = { .fd = efd, .events = POLLOUT };
			ATF_REQUIRE(poll(&pfd, 1, 0) == 1);
			ATF_REQUIRE(pfd.revents == POLLOUT);
		}

		uint64_t increment = UINT64_MAX - 2 - UINT_MAX;
		ATF_REQUIRE(write(efd, &increment, sizeof(increment)) ==
		    (ssize_t)sizeof(increment));

		{
			struct pollfd pfd = { .fd = efd, .events = POLLOUT };
			ATF_REQUIRE(poll(&pfd, 1, 0) == 1);
			ATF_REQUIRE(pfd.revents == POLLOUT);
		}

		increment = 1;
		ATF_REQUIRE(write(efd, &increment, sizeof(increment)) ==
		    (ssize_t)sizeof(increment));

		{
			struct pollfd pfd = { .fd = efd, .events = POLLOUT };
			ATF_REQUIRE(poll(&pfd, 1, 0) == 0);
		}

		increment = 1;
		ATF_REQUIRE_ERRNO(EAGAIN,
		    write(efd, &increment, sizeof(increment)) < 0);

		uint64_t value;
		ATF_REQUIRE(eventfd_read(efd, &value) == 0);
		ATF_REQUIRE(value == UINT64_MAX - 1);
	}
	ATF_REQUIRE(close(efd) == 0);
}

ATF_TC_WITHOUT_HEAD(eventfd__argument_checks);
ATF_TC_BODY_FD_LEAKCHECK(eventfd__argument_checks, tc)
{
	int efd;

	ATF_REQUIRE_ERRNO(EINVAL,
	    eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK | EFD_SEMAPHORE | 42) < 0);

	ATF_REQUIRE((efd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK)) >= 0);

	uint16_t dummy = 0;
	ATF_REQUIRE_ERRNO(EINVAL, write(efd, &dummy, sizeof(dummy)) < 0);
	ATF_REQUIRE_ERRNO(EINVAL, read(efd, &dummy, sizeof(dummy)) < 0);

	uint64_t value;
	ATF_REQUIRE_ERRNO(EAGAIN, eventfd_read(efd, &value) < 0);
	value = 3;
	ATF_REQUIRE(eventfd_write(efd, value) == 0);
	ATF_REQUIRE(eventfd_read(efd, &value) == 0);
	ATF_REQUIRE(value == 3);

	ATF_REQUIRE(close(efd) == 0);

	ATF_REQUIRE_ERRNO(EBADF, eventfd_write(efd, value) < 0);
	ATF_REQUIRE_ERRNO(EBADF, eventfd_read(efd, &value) < 0);

	ATF_REQUIRE((efd = creat("tmpfile", 0777)) >= 0);

	/* You are actually able to use eventfd_write/eventfd_read as normal
	 * write/read, but this is a bad idea! */
	ATF_REQUIRE(eventfd_write(efd, value) == 0);
	ATF_REQUIRE_ERRNO(EBADF, eventfd_read(efd, &value) < 0);

	ATF_REQUIRE(close(efd) == 0);
	ATF_REQUIRE(unlink("tmpfile") == 0);
}

ATF_TC_WITHOUT_HEAD(eventfd__write);
ATF_TC_BODY_FD_LEAKCHECK(eventfd__write, tc)
{
	int efd;

	ATF_REQUIRE((efd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK)) >= 0);
	{
		ATF_REQUIRE_ERRNO(EINVAL, eventfd_write(efd, UINT64_MAX) < 0);
		ATF_REQUIRE(eventfd_write(efd, UINT64_MAX - 1) == 0);
		ATF_REQUIRE_ERRNO(EAGAIN, eventfd_write(efd, 1) < 0);
		ATF_REQUIRE_ERRNO(EAGAIN, eventfd_write(efd, 1) < 0);

		struct pollfd pfd = { .fd = efd, .events = POLLIN };
		ATF_REQUIRE(poll(&pfd, 1, 0) == 1);
		ATF_REQUIRE(pfd.revents == POLLIN);

		uint64_t value;
		ATF_REQUIRE(eventfd_read(efd, &value) == 0);
		ATF_REQUIRE(value == UINT64_MAX - 1);

		ATF_REQUIRE(poll(&pfd, 1, 0) == 0);
	}
	ATF_REQUIRE(close(efd) == 0);
}

ATF_TC_WITHOUT_HEAD(eventfd__read);
ATF_TC_BODY_FD_LEAKCHECK(eventfd__read, tc)
{
	int efd;
	uint64_t value;

	ATF_REQUIRE((efd = eventfd(3,
			 EFD_CLOEXEC | EFD_NONBLOCK | EFD_SEMAPHORE)) >= 0);
	{
		struct pollfd pfd = { .fd = efd, .events = POLLIN };
		ATF_REQUIRE(poll(&pfd, 1, 0) == 1);
		ATF_REQUIRE(pfd.revents == POLLIN);

		ATF_REQUIRE(eventfd_read(efd, &value) == 0);
		ATF_REQUIRE(value == 1);
		ATF_REQUIRE(eventfd_read(efd, &value) == 0);
		ATF_REQUIRE(value == 1);
		ATF_REQUIRE(eventfd_read(efd, &value) == 0);
		ATF_REQUIRE(value == 1);
		ATF_REQUIRE_ERRNO(EAGAIN, eventfd_read(efd, &value) < 0);

		ATF_REQUIRE(poll(&pfd, 1, 0) == 0);
	}
	ATF_REQUIRE(close(efd) == 0);
}

ATF_TC_WITHOUT_HEAD(eventfd__write_read);
ATF_TC_BODY_FD_LEAKCHECK(eventfd__write_read, tc)
{
	int efd;
	uint64_t value;

	ATF_REQUIRE((efd = eventfd(6, EFD_CLOEXEC | EFD_NONBLOCK)) >= 0);
	{
		{
			struct pollfd pfd = { .fd = efd, .events = POLLIN };
			ATF_REQUIRE(poll(&pfd, 1, 0) == 1);
			ATF_REQUIRE(pfd.revents == POLLIN);
		}
		ATF_REQUIRE(eventfd_read(efd, &value) == 0);
		ATF_REQUIRE(value == 6);

		struct pollfd pfd = { .fd = efd, .events = POLLIN };
		ATF_REQUIRE(poll(&pfd, 1, 0) == 0);

		ATF_REQUIRE(eventfd_write(efd, 2) == 0);

		ATF_REQUIRE(poll(&pfd, 1, 0) == 1);
		ATF_REQUIRE(pfd.revents == POLLIN);

		ATF_REQUIRE(eventfd_read(efd, &value) == 0);
		ATF_REQUIRE(value == 2);
		ATF_REQUIRE_ERRNO(EAGAIN, eventfd_read(efd, &value) < 0);

		ATF_REQUIRE(poll(&pfd, 1, 0) == 0);
	}
	ATF_REQUIRE(close(efd) == 0);
}

ATF_TC_WITHOUT_HEAD(eventfd__write_read_semaphore);
ATF_TC_BODY_FD_LEAKCHECK(eventfd__write_read_semaphore, tc)
{
	int efd;
	uint64_t value;

	ATF_REQUIRE((efd = eventfd(0,
			 EFD_CLOEXEC | EFD_NONBLOCK | EFD_SEMAPHORE)) >= 0);
	{
		struct pollfd pfd = { .fd = efd, .events = POLLIN };
		ATF_REQUIRE(poll(&pfd, 1, 0) == 0);

		ATF_REQUIRE(eventfd_write(efd, 2) == 0);

		ATF_REQUIRE(poll(&pfd, 1, 0) == 1);
		ATF_REQUIRE(pfd.revents == POLLIN);

		ATF_REQUIRE(eventfd_read(efd, &value) == 0);
		ATF_REQUIRE(value == 1);
		ATF_REQUIRE(eventfd_read(efd, &value) == 0);
		ATF_REQUIRE(value == 1);
		ATF_REQUIRE_ERRNO(EAGAIN, eventfd_read(efd, &value) < 0);

		ATF_REQUIRE(poll(&pfd, 1, 0) == 0);
	}
	ATF_REQUIRE(close(efd) == 0);
}

typedef struct {
	int efd;
	int signal_pipe[2];
} ReadThreadArgs;

static atomic_int read_counter;

static void *
read_fun(void *arg)
{
	ReadThreadArgs *args = arg;
	int efd = args->efd;

	for (;;) {
		uint64_t value;

		if (eventfd_read(efd, &value) == 0) {
			int current_counter = atomic_fetch_add(&read_counter,
			    1);

			if (current_counter % 10 == 0 &&
			    current_counter < 100) {
				ATF_REQUIRE(eventfd_write(efd, 10) == 0);
			}

			continue;
		}

		ATF_REQUIRE(errno == EAGAIN);

		struct pollfd pfds[2] = { /**/
			{ .fd = efd, .events = POLLIN },
			{ .fd = args->signal_pipe[0], .events = POLLIN }
		};
		ATF_REQUIRE(poll(pfds, nitems(pfds), -1) > 0);

		if (pfds[1].revents) {
			break;
		}
	}

	return (NULL);
}

ATF_TC_WITHOUT_HEAD(eventfd__threads_read);
ATF_TC_BODY_FD_LEAKCHECK(eventfd__threads_read, tc)
{
	int efd;
	pthread_t threads[4];
	ReadThreadArgs thread_args[4];

	for (int i = 0; i < 1000; ++i) {
		read_counter = 0;
		ATF_REQUIRE(
		    (efd = eventfd(0,
			 EFD_CLOEXEC | EFD_NONBLOCK | EFD_SEMAPHORE)) >= 0);

		uint64_t counter_val = 100;

		for (int i = 0; i < (int)nitems(threads); ++i) {
			thread_args[i].efd = efd;
			ATF_REQUIRE(pipe2(thread_args[i].signal_pipe,
					O_CLOEXEC | O_NONBLOCK) == 0);
			ATF_REQUIRE(pthread_create(&threads[i], NULL, /**/
					read_fun, &thread_args[i]) == 0);
		}

		ATF_REQUIRE(eventfd_write(efd, counter_val) == 0);

		while (atomic_load(&read_counter) != 2 * (int)counter_val) {
		}

		for (int i = 0; i < (int)nitems(threads); ++i) {
			ATF_REQUIRE(close(thread_args[i].signal_pipe[1]) == 0);
			ATF_REQUIRE(pthread_join(threads[i], NULL) == 0);
			ATF_REQUIRE(close(thread_args[i].signal_pipe[0]) == 0);
		}

		ATF_REQUIRE(close(efd) == 0);
		ATF_REQUIRE(read_counter == 2 * (int)counter_val);
	}
}

ATF_TC_WITHOUT_HEAD(eventfd__fork);
ATF_TC_BODY_FD_LEAKCHECK(eventfd__fork, tc)
{
	int efd;

	ATF_REQUIRE((efd = eventfd(0,
			 EFD_CLOEXEC | EFD_NONBLOCK | EFD_SEMAPHORE)) >= 0);

	if (close(efd + 2) == 0) {
		atf_tc_skip("shimmed eventfd's implemented by self-pipe "
			    "cannot be shared between processes");
	}

	int pid;
	ATF_REQUIRE((pid = fork()) >= 0);
	if (pid == 0) {
		int r = eventfd_write(efd, 1);
		if (r < 0) {
			_Exit(errno);
		}
		if (r > 0) {
			(void)raise(SIGTERM);
			_Exit(1);
		}
		_Exit(0);
	}

	int status;
	ATF_REQUIRE(waitpid(pid, &status, 0) == pid);
	ATF_REQUIRE(WIFEXITED(status));
	if (WEXITSTATUS(status) == EBADF) {
		atf_tc_skip("only native eventfds can be shared "
			    "between processes");
	}
	ATF_REQUIRE(WEXITSTATUS(status) == 0);

	uint64_t value;
	ATF_REQUIRE(eventfd_read(efd, &value) == 0);
	ATF_REQUIRE(value == 1);

	ATF_REQUIRE(close(efd) == 0);
}

ATF_TC_WITHOUT_HEAD(eventfd__stat);
ATF_TC_BODY_FD_LEAKCHECK(eventfd__stat, tc)
{
	int efd;

	ATF_REQUIRE((efd = eventfd(5,
			 EFD_CLOEXEC | EFD_NONBLOCK | EFD_SEMAPHORE)) >= 0);

	struct stat sb;
	ATF_REQUIRE(fstat(efd, &sb) == 0);

	/*
	 * We actually don't care that much about the stat contents, only that
	 * it doesn't throw an error.
	 */

	ATF_REQUIRE(close(efd) == 0);
}

typedef struct {
	uint64_t ev_count;
	int loop;
	int efd;
} WriteThreadArgs;

static void *
write_fun(void *arg)
{
	WriteThreadArgs *td = arg;
	ssize_t s;
	int i;

	for (i = 0; i < td->loop; i++) {
		ATF_REQUIRE(
		    (s = write(td->efd, &td->ev_count, sizeof(td->ev_count))) ==
		    sizeof(td->ev_count));
		usleep(100);
	}
	return (NULL);
}

ATF_TC_WITHOUT_HEAD(eventfd__threads_blocking);
ATF_TC_BODY_FD_LEAKCHECK(eventfd__threads_blocking, tc)
{
	/* clang-format off */
	uint64_t count[] = {
	    2,   3,   5,   7,   11,  13,  17,  19,  23,  29,
	    31,  37,  41,  43,  47,  53,  59,  61,  67,  71,
	    73,  79,  83,  89,  97,  101, 103, 107, 109, 113,
	    127, 131, 137, 139, 149, 151, 157, 163, 167, 173,
	    179, 181, 191, 193, 197, 199, 211, 223, 227, 229,
	    233, 239, 241, 251, 257, 263, 269, 271, 277, 281,
	    283, 293, 307, 311, 313, 317, 331, 337, 347, 349,
	    353, 359, 367, 373, 379, 383, 389, 397, 401, 409,
	    419, 421, 431, 433, 439, 443, 449, 457, 461, 463,
	    467, 479, 487, 491, 499, 503, 509, 521, 523, 541,
	    547, 557, 563, 569, 571, 577, 587, 593, 599, 601,
	    607, 613, 617, 619, 631, 641, 643, 647, 653, 659,
	    661, 673, 677, 683, 691, 701, 709, 719, 727, 733,
	    739, 743, 751, 757, 761, 769, 773, 787, 797, 809,
	    811, 821, 823, 827, 829, 839, 853, 857, 859, 863,
	    877, 881, 883, 887, 907, 911, 919, 929, 937, 941,
	    947, 953, 967, 971, 977, 983, 991, 997, 1009,
	};
	/* clang-format on */

	int const LOOP = 1000;
#define THREADS ((int)(sizeof(count) / sizeof(count[0])))
	int efd;
	pthread_t thread[THREADS];
	WriteThreadArgs td[THREADS];
	uint64_t total;
	int i;
	ssize_t s;
	uint64_t u;
	uint64_t v;

	ATF_REQUIRE((efd = eventfd(0, EFD_CLOEXEC)) >= 0);

	total = 0;
	for (i = 0; i < THREADS; i++) {
		td[i].efd = efd;
		td[i].ev_count = count[i];
		td[i].loop = LOOP;
		total += (count[i] * (uint64_t)LOOP);
	}

	for (i = 0; i < THREADS; i++) {
		ATF_REQUIRE(
		    pthread_create(&thread[i], NULL, write_fun, &td[i]) == 0);
	}

	v = 0;
	while (total != v) {
		ATF_REQUIRE((s = read(efd, &u, sizeof(u))) == sizeof(u));
		v += u;
	}

	ATF_REQUIRE(v == total);

	/* verify all threads have finished */
	for (i = 0; i < THREADS; i++) {
		ATF_REQUIRE(pthread_join(thread[i], NULL) == 0);
	}
	ATF_REQUIRE(close(efd) == 0);
#undef THREADS
}

ATF_TC_WITHOUT_HEAD(eventfd__epoll);
ATF_TC_BODY_FD_LEAKCHECK(eventfd__epoll, tc)
{
	int ep = epoll_create1(EPOLL_CLOEXEC);
	ATF_REQUIRE(ep >= 0);

	int efd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
	ATF_REQUIRE(efd >= 0);

	struct epoll_event event;

	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, efd,
			&(struct epoll_event) {
			    .events = EPOLLIN | EPOLLET,
			    .data.fd = efd,
			}) == 0);
	ATF_REQUIRE(epoll_wait(ep, &event, 1, 0) == 0);

	ATF_REQUIRE(eventfd_write(efd, 3) == 0);
	ATF_REQUIRE(epoll_wait(ep, &event, 1, -1) == 1);
	ATF_REQUIRE(event.events == EPOLLIN);
	ATF_REQUIRE(event.data.fd == efd);
	ATF_REQUIRE(epoll_wait(ep, &event, 1, 0) == 0);

	uint64_t value;
	ATF_REQUIRE(eventfd_read(efd, &value) == 0);
	ATF_REQUIRE(epoll_wait(ep, &event, 1, 0) == 0);

	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_MOD, efd,
			&(struct epoll_event) {
			    .events = EPOLLOUT,
			    .data.fd = efd,
			}) == 0);
	int r = epoll_wait(ep, &event, 1, 0);
	if (r == 0) {
		atf_tc_skip("shim layer does not support polling for write");
	}
	ATF_REQUIRE(r == 1);
	ATF_REQUIRE(event.events == EPOLLOUT);
	ATF_REQUIRE(event.data.fd == efd);

	ATF_REQUIRE(eventfd_write(efd, UINT64_MAX - 2) == 0);
	ATF_REQUIRE(epoll_wait(ep, &event, 1, 0) == 1);
	ATF_REQUIRE(event.events == EPOLLOUT);
	ATF_REQUIRE(event.data.fd == efd);

	ATF_REQUIRE(eventfd_write(efd, 1) == 0);
	ATF_REQUIRE(epoll_wait(ep, &event, 1, 0) == 0);

	ATF_REQUIRE(close(efd) == 0);
	ATF_REQUIRE(close(ep) == 0);
}

ATF_TC_WITHOUT_HEAD(eventfd__toggle_nonblock);
ATF_TC_BODY_FD_LEAKCHECK(eventfd__toggle_nonblock, tc)
{
	int efd = eventfd(0, EFD_CLOEXEC);
	ATF_REQUIRE(efd >= 0);

	int r = fcntl(efd, F_GETFL);
	ATF_REQUIRE(r >= 0);
	r = fcntl(efd, F_SETFL, r | O_NONBLOCK);
#ifdef __NetBSD__
	if (r < 0 && errno == EOPNOTSUPP) {
		atf_tc_skip(
		    "NetBSD's native eventfd does not support F_SETFL.");
	}
#endif
	ATF_REQUIRE(r >= 0);

	uint64_t value;
	ATF_REQUIRE_ERRNO(EAGAIN, read(efd, &value, sizeof(value)) < 0);

	ATF_REQUIRE(close(efd) == 0);
}

ATF_TP_ADD_TCS(tp)
{
	ATF_TP_ADD_TC(tp, eventfd__constants);
	ATF_TP_ADD_TC(tp, eventfd__pollin);
	ATF_TP_ADD_TC(tp, eventfd__pollout);
	ATF_TP_ADD_TC(tp, eventfd__argument_checks);
	ATF_TP_ADD_TC(tp, eventfd__write);
	ATF_TP_ADD_TC(tp, eventfd__read);
	ATF_TP_ADD_TC(tp, eventfd__write_read);
	ATF_TP_ADD_TC(tp, eventfd__write_read_semaphore);
	ATF_TP_ADD_TC(tp, eventfd__threads_read);
	ATF_TP_ADD_TC(tp, eventfd__fork);
	ATF_TP_ADD_TC(tp, eventfd__stat);
	/*
	 * Following test based on:
	 * https://raw.githubusercontent.com/cloudius-systems/osv/master/tests/tst-eventfd.cc
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
	ATF_TP_ADD_TC(tp, eventfd__threads_blocking);
	ATF_TP_ADD_TC(tp, eventfd__epoll);
	ATF_TP_ADD_TC(tp, eventfd__toggle_nonblock);

	return atf_no_error();
}
