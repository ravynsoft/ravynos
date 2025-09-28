#define _GNU_SOURCE

#include <atf-c.h>

#if defined(__FreeBSD__)
#include <sys/capsicum.h>
#endif
#include <sys/epoll.h>
#include <sys/stat.h>

#ifndef __linux__
#include <sys/event.h>
#include <sys/param.h>
#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <fcntl.h>
#include <limits.h>
#include <poll.h>
#include <unistd.h>

#include "atf-c-leakcheck.h"

#ifndef nitems
#define nitems(x) (sizeof((x)) / sizeof((x)[0]))
#endif

/*
 * These tests show that on Linux, POLLOUT and POLLERR may happen at the same
 * time on a write end of a pipe after the read end was closed depending on the
 * contents of the pipe buffer.
 */

ATF_TC_WITHOUT_HEAD(pipe__simple_poll);
ATF_TC_BODY_FD_LEAKCHECK(pipe__simple_poll, tc)
{
	int p[2] = { -1, -1 };

	ATF_REQUIRE(pipe2(p, O_CLOEXEC | O_NONBLOCK) == 0);
	ATF_REQUIRE(p[0] >= 0);
	ATF_REQUIRE(p[1] >= 0);

	{
		struct pollfd pfd = { .fd = p[0], .events = POLLIN };
		ATF_REQUIRE(poll(&pfd, 1, 0) == 0);
		ATF_REQUIRE(pfd.revents == 0);

		int ep = epoll_create1(EPOLL_CLOEXEC);
		ATF_REQUIRE(ep >= 0);

		struct epoll_event eps[32];
		eps[0] = (struct epoll_event) { .events = EPOLLIN | EPOLLET };
		ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, p[0], &eps[0]) == 0);

		ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 0);
		ATF_REQUIRE(close(ep) == 0);
	}

	{
		struct pollfd pfd = { .fd = p[1], .events = POLLOUT };
		ATF_REQUIRE(poll(&pfd, 1, 0) == 1);
		ATF_REQUIRE(pfd.revents == POLLOUT);

		int ep = epoll_create1(EPOLL_CLOEXEC);
		ATF_REQUIRE(ep >= 0);

		struct epoll_event eps[32];
		eps[0] = (struct epoll_event) { .events = EPOLLOUT | EPOLLET };
		ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, p[1], &eps[0]) == 0);

		ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 1);
		ATF_REQUIRE(eps[0].events == EPOLLOUT);
		ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 0);
		ATF_REQUIRE(close(ep) == 0);
	}

	ATF_REQUIRE(close(p[0]) == 0);
	ATF_REQUIRE(close(p[1]) == 0);
}

ATF_TC_WITHOUT_HEAD(pipe__poll_write_end_after_read_end_close);
ATF_TC_BODY_FD_LEAKCHECK(pipe__poll_write_end_after_read_end_close, tc)
{
	int p[2] = { -1, -1 };

	ATF_REQUIRE(pipe2(p, O_CLOEXEC | O_NONBLOCK) == 0);
	ATF_REQUIRE(p[0] >= 0);
	ATF_REQUIRE(p[1] >= 0);

	int ep = epoll_create1(EPOLL_CLOEXEC);
	ATF_REQUIRE(ep >= 0);

	struct epoll_event eps[32];
	eps[0] = (struct epoll_event) { .events = EPOLLOUT | EPOLLET };
	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, p[1], &eps[0]) == 0);

	ATF_REQUIRE(close(p[0]) == 0);

	{
		struct pollfd pfd = { .fd = p[1], .events = POLLOUT };
		ATF_REQUIRE(poll(&pfd, 1, 0) == 1);
#if defined(__linux__)
		ATF_REQUIRE(pfd.revents == (POLLOUT | POLLERR));
#elif defined(__NetBSD__)
		ATF_REQUIRE(pfd.revents == (POLLOUT | POLLHUP));
#else
		ATF_REQUIRE(pfd.revents == POLLHUP);
#endif
	}

	ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 1);
	if (eps[0].events == POLLERR) {
		atf_tc_skip(
		    "kqueue based emulation may return just POLLERR here");
	}
	ATF_REQUIRE(eps[0].events == (EPOLLOUT | POLLERR));
	ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 0);
	ATF_REQUIRE(close(ep) == 0);

	ATF_REQUIRE(close(p[1]) == 0);
}

ATF_TC_WITHOUT_HEAD(pipe__poll_full_write_end_after_read_end_close);
ATF_TC_BODY_FD_LEAKCHECK(pipe__poll_full_write_end_after_read_end_close, tc)
{
	int p[2] = { -1, -1 };

	ATF_REQUIRE(pipe2(p, O_CLOEXEC | O_NONBLOCK) == 0);
	ATF_REQUIRE(p[0] >= 0);
	ATF_REQUIRE(p[1] >= 0);

	char c = 0;
	ssize_t r;
	while ((r = write(p[1], &c, 1)) == 1) {
	}
	ATF_REQUIRE(r < 0);
	ATF_REQUIRE(errno == EAGAIN || errno == EWOULDBLOCK);

	int ep = epoll_create1(EPOLL_CLOEXEC);
	ATF_REQUIRE(ep >= 0);

	struct epoll_event eps[32];
	eps[0] = (struct epoll_event) { .events = EPOLLOUT | EPOLLET };
	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, p[1], &eps[0]) == 0);

	ATF_REQUIRE(close(p[0]) == 0);

	{
		struct pollfd pfd = { .fd = p[1], .events = POLLOUT };
		ATF_REQUIRE(poll(&pfd, 1, 0) == 1);
#ifdef __linux__
		ATF_REQUIRE(pfd.revents == POLLERR);
#elif defined(__NetBSD__)
		ATF_REQUIRE(pfd.revents == (POLLOUT | POLLHUP));
#else
		ATF_REQUIRE(pfd.revents == POLLHUP);
#endif
	}

	int ret = epoll_wait(ep, eps, 32, 0);
	if (ret == 0) {
		atf_tc_skip("NetBSD hangs here");
	}
	ATF_REQUIRE(ret == 1);
	ATF_REQUIRE(eps[0].events == POLLERR);
	ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 0);
	ATF_REQUIRE(close(ep) == 0);

	ATF_REQUIRE(close(p[1]) == 0);
}

ATF_TC_WITHOUT_HEAD(pipe__poll_full_write_end_after_read_end_close_hup);
ATF_TC_BODY_FD_LEAKCHECK(pipe__poll_full_write_end_after_read_end_close_hup, tc)
{
	int p[2] = { -1, -1 };

	ATF_REQUIRE(pipe2(p, O_CLOEXEC | O_NONBLOCK) == 0);
	ATF_REQUIRE(p[0] >= 0);
	ATF_REQUIRE(p[1] >= 0);

#if defined(__FreeBSD__)
	{
		cap_rights_t rights;
		ATF_REQUIRE(cap_rights_get(p[1], &rights) == 0);
		cap_rights_clear(&rights, CAP_READ);
		ATF_REQUIRE(cap_rights_limit(p[1], &rights) == 0);
	}
#endif

	char c = 0;
	ssize_t r;
	while ((r = write(p[1], &c, 1)) == 1) {
	}
	ATF_REQUIRE(r < 0);
	ATF_REQUIRE(errno == EAGAIN || errno == EWOULDBLOCK);

	int ep = epoll_create1(EPOLL_CLOEXEC);
	ATF_REQUIRE(ep >= 0);

	struct epoll_event eps[32];
	eps[0] = (struct epoll_event) { .events = EPOLLET };
	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, p[1], &eps[0]) == 0);

	ATF_REQUIRE(close(p[0]) == 0);

	{
		struct pollfd pfd = { .fd = p[1] };
		int rv = poll(&pfd, 1, 1000);
#if defined(__DragonFly__) || defined(__APPLE__)
		if (rv == 0) {
			atf_tc_skip("polling of pipes broken");
		}
#endif
		ATF_REQUIRE(rv == 1);

#ifdef __linux__
		ATF_REQUIRE(pfd.revents == POLLERR);
#elif defined(__NetBSD__)
		ATF_REQUIRE(pfd.revents == POLLHUP);
#else
		ATF_REQUIRE(pfd.revents == POLLHUP);
#endif
	}

	int ret = epoll_wait(ep, eps, 32, 0);
	if (ret == 0) {
		atf_tc_skip("NetBSD hangs here");
	}
	ATF_REQUIRE(ret == 1);
#if defined(__OpenBSD__)
	if (eps[0].events == POLLHUP) {
		atf_tc_skip("OpenBSD has duplex pipes but no way to tell p[0] "
			    "and p[1] apart");
	}
#endif
	ATF_REQUIRE_MSG(eps[0].events == POLLERR, "%04x", eps[0].events);
	ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 0);
	ATF_REQUIRE(close(ep) == 0);

	ATF_REQUIRE(close(p[1]) == 0);
}

ATF_TC_WITHOUT_HEAD(pipe__poll_full_minus_1b_write_end_after_read_end_close);
ATF_TC_BODY_FD_LEAKCHECK(
    pipe__poll_full_minus_1b_write_end_after_read_end_close, tc)
{
	int p[2] = { -1, -1 };

	ATF_REQUIRE(pipe2(p, O_CLOEXEC | O_NONBLOCK) == 0);
	ATF_REQUIRE(p[0] >= 0);
	ATF_REQUIRE(p[1] >= 0);

	char c = 0;
	ssize_t r;
	while ((r = write(p[1], &c, 1)) == 1) {
	}
	ATF_REQUIRE(r < 0);
	ATF_REQUIRE(errno == EAGAIN || errno == EWOULDBLOCK);

	ATF_REQUIRE(read(p[0], &c, 1) == 1);

	int ep = epoll_create1(EPOLL_CLOEXEC);
	ATF_REQUIRE(ep >= 0);

	struct epoll_event eps[32];
	eps[0] = (struct epoll_event) { .events = EPOLLOUT | EPOLLET };
	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, p[1], &eps[0]) == 0);

	ATF_REQUIRE(close(p[0]) == 0);

	{
		struct pollfd pfd = { .fd = p[1], .events = POLLOUT };
		ATF_REQUIRE(poll(&pfd, 1, 0) == 1);
#ifdef __linux__
		ATF_REQUIRE(pfd.revents == POLLERR);
#elif defined(__NetBSD__)
		ATF_REQUIRE(pfd.revents == (POLLOUT | POLLHUP));
#else
		ATF_REQUIRE(pfd.revents == POLLHUP);
#endif
	}

	int ret = epoll_wait(ep, eps, 32, 0);
	if (ret == 0) {
		atf_tc_skip("NetBSD hangs here");
	}
	ATF_REQUIRE(ret == 1);
	ATF_REQUIRE(eps[0].events == POLLERR);
	ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 0);
	ATF_REQUIRE(close(ep) == 0);

	ATF_REQUIRE(close(p[1]) == 0);
}

ATF_TC_WITHOUT_HEAD(pipe__poll_full_minus_511b_write_end_after_read_end_close);
ATF_TC_BODY_FD_LEAKCHECK(
    pipe__poll_full_minus_511b_write_end_after_read_end_close, tc)
{
	int p[2] = { -1, -1 };

	ATF_REQUIRE(pipe2(p, O_CLOEXEC | O_NONBLOCK) == 0);
	ATF_REQUIRE(p[0] >= 0);
	ATF_REQUIRE(p[1] >= 0);

	char c = 0;
	ssize_t r;
	while ((r = write(p[1], &c, 1)) == 1) {
	}
	ATF_REQUIRE(r < 0);
	ATF_REQUIRE(errno == EAGAIN || errno == EWOULDBLOCK);

	for (int i = 0; i < PIPE_BUF - 1; ++i) {
		ATF_REQUIRE(read(p[0], &c, 1) == 1);
	}

	int ep = epoll_create1(EPOLL_CLOEXEC);
	ATF_REQUIRE(ep >= 0);

	struct epoll_event eps[32];
	eps[0] = (struct epoll_event) { .events = EPOLLOUT | EPOLLET };
	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, p[1], &eps[0]) == 0);

	ATF_REQUIRE(close(p[0]) == 0);

	{
		struct pollfd pfd = { .fd = p[1], .events = POLLOUT };
		ATF_REQUIRE(poll(&pfd, 1, 0) == 1);
#ifdef __linux__
		ATF_REQUIRE(pfd.revents == POLLERR);
#elif defined(__NetBSD__)
		ATF_REQUIRE(pfd.revents == (POLLOUT | POLLHUP));
#else
		ATF_REQUIRE(pfd.revents == POLLHUP);
#endif
	}

	int ret = epoll_wait(ep, eps, 32, 0);
	if (ret == 0) {
		atf_tc_skip("NetBSD hangs here");
	}
	ATF_REQUIRE(ret == 1);
	ATF_REQUIRE(eps[0].events == POLLERR);
	ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 0);
	ATF_REQUIRE(close(ep) == 0);

	ATF_REQUIRE(close(p[1]) == 0);
}

ATF_TC_WITHOUT_HEAD(pipe__poll_full_minus_512b_write_end_after_read_end_close);
ATF_TC_BODY_FD_LEAKCHECK(
    pipe__poll_full_minus_512b_write_end_after_read_end_close, tc)
{
	int p[2] = { -1, -1 };

	ATF_REQUIRE(pipe2(p, O_CLOEXEC | O_NONBLOCK) == 0);
	ATF_REQUIRE(p[0] >= 0);
	ATF_REQUIRE(p[1] >= 0);

	char c = 0;
	ssize_t r;
	while ((r = write(p[1], &c, 1)) == 1) {
	}
	ATF_REQUIRE(r < 0);
	ATF_REQUIRE(errno == EAGAIN || errno == EWOULDBLOCK);

	for (int i = 0; i < PIPE_BUF; ++i) {
		ATF_REQUIRE(read(p[0], &c, 1) == 1);
	}

	int ep = epoll_create1(EPOLL_CLOEXEC);
	ATF_REQUIRE(ep >= 0);

	struct epoll_event eps[32];
	eps[0] = (struct epoll_event) { .events = EPOLLOUT | EPOLLET };
	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, p[1], &eps[0]) == 0);

	ATF_REQUIRE(close(p[0]) == 0);

	{
		struct pollfd pfd = { .fd = p[1], .events = POLLOUT };
		ATF_REQUIRE(poll(&pfd, 1, 0) == 1);

#ifdef __linux__
		ATF_REQUIRE(pfd.revents == (POLLOUT | POLLERR));
#elif defined(__NetBSD__)
		ATF_REQUIRE(pfd.revents == (POLLOUT | POLLHUP));
#else
		ATF_REQUIRE(pfd.revents == POLLHUP);
#endif
	}

	ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 1);
	if (eps[0].events == POLLERR) {
		atf_tc_skip(
		    "kqueue based emulation may return just POLLERR here");
	}
	ATF_REQUIRE(eps[0].events == (POLLOUT | POLLERR));
	ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 0);
	ATF_REQUIRE(close(ep) == 0);

	ATF_REQUIRE(close(p[1]) == 0);
}

// #define FORCE_EPOLL

static void
print_statbuf(struct stat *sb)
{
	printf("st_dev: %lu\n", (unsigned long)sb->st_dev);
	printf("st_ino: %llu\n", (unsigned long long)sb->st_ino);
	printf("st_nlink: %lu\n", (unsigned long)sb->st_nlink);
	printf("st_mode: %o\n", (int)sb->st_mode);
	printf("st_uid: %u\n", sb->st_uid);
	printf("st_gid: %u\n", sb->st_gid);
	printf("st_rdev: %lu\n", (unsigned long)sb->st_rdev);
	printf("st_size: %llu\n", (unsigned long long)sb->st_size);
	printf("st_blocks: %llu\n", (unsigned long long)sb->st_blocks);
	printf("st_blksize: %d\n", sb->st_blksize);
#if !defined(__linux__)
	printf("st_flags: %x\n", sb->st_flags);
	printf("st_gen: %lu\n", (unsigned long)sb->st_gen);
#endif
}

static int const SPURIOUS_EV_ADD = 0
#if defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__) || \
    defined(__APPLE__)
    | EV_ADD
#endif
    ;

static int const SPURIOUS_EV_ONESHOT = 0
#if defined(__FreeBSD__)
    | EV_ONESHOT
#endif
    ;

static int const SPURIOUS_EV_NODATA = 0
#if defined(__DragonFly__)
    | EV_NODATA
#endif
    ;

static int const OPTIONAL_EV_HUP = 0
#if defined(__DragonFly__) && defined(EV_HUP)
    | EV_HUP
#endif
    ;

ATF_TC_WITHOUT_HEAD(pipe__pipe_event_poll);
ATF_TC_BODY_FD_LEAKCHECK(pipe__pipe_event_poll, tc)
{
	int p[2] = { -1, -1 };

	ATF_REQUIRE(pipe2(p, O_CLOEXEC | O_NONBLOCK) == 0);
	ATF_REQUIRE(p[0] >= 0);
	ATF_REQUIRE(p[1] >= 0);
#if defined(__FreeBSD__)
	{
		cap_rights_t rights;
		ATF_REQUIRE(cap_rights_get(p[1], &rights) == 0);
		cap_rights_clear(&rights, CAP_READ);
		ATF_REQUIRE(cap_rights_limit(p[1], &rights) == 0);
	}
#endif

#if !defined(__linux__) && !defined(FORCE_EPOLL)
	int kq = kqueue();
	ATF_REQUIRE(kq >= 0);

	struct kevent kev[32];
	EV_SET(&kev[0], (unsigned int)p[1], EVFILT_WRITE, /**/
	    EV_ADD | EV_CLEAR, 0, 0, 0);

	ATF_REQUIRE(kevent(kq, kev, 1, NULL, 0, NULL) == 0);

	ATF_REQUIRE(kevent(kq, NULL, 0, kev, nitems(kev),
			&(struct timespec) { 0, 0 }) == 1);
	ATF_REQUIRE(kev[0].ident == (uintptr_t)p[1]);
	ATF_REQUIRE(kev[0].filter == EVFILT_WRITE);

	ATF_REQUIRE_MSG(kev[0].flags == (EV_CLEAR | SPURIOUS_EV_ADD), "%x",
	    kev[0].flags);
	ATF_REQUIRE(kev[0].fflags == 0);
	ATF_REQUIRE_MSG(kev[0].data == 16384 ||
		kev[0].data == 32768 /* on DragonFly */,
	    "%d", (int)kev[0].data);
	ATF_REQUIRE(kev[0].udata == 0);
#endif
	int ep = epoll_create1(EPOLL_CLOEXEC);
	ATF_REQUIRE(ep >= 0);

	struct epoll_event eps[32];
	eps[0] = (struct epoll_event) { .events = EPOLLIN | EPOLLOUT |
		    EPOLLET };
	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, p[1], &eps[0]) == 0);

	ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 1);
	ATF_REQUIRE(eps[0].events == EPOLLOUT);

	char c = 0;
	ssize_t r;
	while ((r = write(p[1], &c, 1)) == 1) {
	}
	ATF_REQUIRE(r < 0);
	ATF_REQUIRE(errno == EAGAIN || errno == EWOULDBLOCK);

#if !defined(__linux__) && !defined(FORCE_EPOLL)
	ATF_REQUIRE(kevent(kq, NULL, 0, kev, nitems(kev),
			&(struct timespec) { 0, 0 }) == 0);
#endif
	ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 0);

	int const pipe_buf =
#ifdef __DragonFly__
	    16384
#else
	    PIPE_BUF
#endif
	    ;

	for (int i = 0; i < pipe_buf - 1; ++i) {
		ATF_REQUIRE(read(p[0], &c, 1) == 1);
	}

#if !defined(__linux__) && !defined(FORCE_EPOLL)
	ATF_REQUIRE(kevent(kq, NULL, 0, kev, nitems(kev),
			&(struct timespec) { 0, 0 }) == 0);
#endif
	ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 0);

	ATF_REQUIRE(read(p[0], &c, 1) == 1);

#if !defined(__linux__) && !defined(FORCE_EPOLL)
	r = kevent(kq, NULL, 0, kev, nitems(kev), &(struct timespec) { 0, 0 });
	ATF_REQUIRE(r == 1);
	ATF_REQUIRE(kev[0].ident == (uintptr_t)p[1]);
	ATF_REQUIRE(kev[0].filter == EVFILT_WRITE);
	ATF_REQUIRE(kev[0].flags == (EV_CLEAR | SPURIOUS_EV_ADD));
	ATF_REQUIRE(kev[0].fflags == 0);
	ATF_REQUIRE_MSG(kev[0].data == pipe_buf, "%d", (int)kev[0].data);
	ATF_REQUIRE(kev[0].udata == 0);
#endif
	ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 1);
	ATF_REQUIRE(eps[0].events == EPOLLOUT);

	ATF_REQUIRE(read(p[0], &c, 1) == 1);

#if !defined(__linux__) && !defined(FORCE_EPOLL)
	ATF_REQUIRE(kevent(kq, NULL, 0, kev, nitems(kev),
			&(struct timespec) { 0, 0 }) == 1);
	ATF_REQUIRE(kev[0].ident == (uintptr_t)p[1]);
	ATF_REQUIRE(kev[0].filter == EVFILT_WRITE);
	ATF_REQUIRE(kev[0].flags == (EV_CLEAR | SPURIOUS_EV_ADD));
	ATF_REQUIRE(kev[0].fflags == 0);
	ATF_REQUIRE_MSG(kev[0].data == pipe_buf + 1, "%d", (int)kev[0].data);
	ATF_REQUIRE(kev[0].udata == 0);

	/* kqueue based emulation will trigger another edge here. */
	ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 1);
	ATF_REQUIRE(eps[0].events == EPOLLOUT);
#endif
	ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 0);

	ATF_REQUIRE(close(p[0]) == 0);

#if !defined(__linux__) && !defined(FORCE_EPOLL)
	ATF_REQUIRE(kevent(kq, NULL, 0, kev, nitems(kev),
			&(struct timespec) { 0, 0 }) == 1);
	ATF_REQUIRE(kev[0].ident == (uintptr_t)p[1]);
	ATF_REQUIRE(kev[0].filter == EVFILT_WRITE);
	ATF_REQUIRE_MSG(kev[0].flags ==
		(EV_CLEAR | SPURIOUS_EV_ADD | EV_EOF | SPURIOUS_EV_ONESHOT |
		    SPURIOUS_EV_NODATA | OPTIONAL_EV_HUP),
	    "%04x", kev[0].flags);
	ATF_REQUIRE(kev[0].fflags == 0);
	if (kev[0].data == 0) {
		atf_tc_skip("kev.data == 0 is a valid value on EV_EOF");
	}
	ATF_REQUIRE_MSG(kev[0].data == pipe_buf + 1, "%d", (int)kev[0].data);
	ATF_REQUIRE(kev[0].udata == 0);
	ATF_REQUIRE(close(kq) == 0);
#endif
	ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 1);
	ATF_REQUIRE(eps[0].events == (EPOLLOUT | EPOLLERR));
	ATF_REQUIRE(close(ep) == 0);

	ATF_REQUIRE(close(p[1]) == 0);
}

ATF_TC_WITHOUT_HEAD(pipe__fifo_writes);
ATF_TC_BODY_FD_LEAKCHECK(pipe__fifo_writes, tc)
{
	int p[2] = { -1, -1 };

	ATF_REQUIRE(mkfifo("the_fifo", 0666) == 0);

	ATF_REQUIRE(
	    (p[0] = open("the_fifo", O_RDONLY | O_CLOEXEC | O_NONBLOCK)) >= 0);
	ATF_REQUIRE(
	    (p[1] = open("the_fifo", O_WRONLY | O_CLOEXEC | O_NONBLOCK)) >= 0);

	{
		int fl = fcntl(p[0], F_GETFL, 0);
		int rq = O_RDONLY;
		ATF_REQUIRE((fl & O_ACCMODE) == rq);
	}
	{
		int fl = fcntl(p[1], F_GETFL, 0);
		int rq = O_WRONLY;
		ATF_REQUIRE((fl & O_ACCMODE) == rq);
	}

#if !defined(__linux__) && !defined(FORCE_EPOLL)
	int kq = kqueue();
	ATF_REQUIRE(kq >= 0);

	struct kevent kev[32];
	EV_SET(&kev[0], (unsigned int)p[1], EVFILT_WRITE, /**/
	    EV_ADD | EV_CLEAR | EV_RECEIPT, 0, 0, 0);
	EV_SET(&kev[1], (unsigned int)p[1], EVFILT_READ, /**/
	    EV_ADD | EV_CLEAR | EV_RECEIPT, 0, 0, 0);

	ATF_REQUIRE(kevent(kq, kev, 2, kev, 2, NULL) == 2);
	ATF_REQUIRE((kev[0].flags & EV_ERROR) != 0);
	ATF_REQUIRE((kev[1].flags & EV_ERROR) != 0);
	ATF_REQUIRE(kev[0].data == 0);
#if defined(__OpenBSD__)
	ATF_REQUIRE(kev[1].data == EINVAL);
#else
	ATF_REQUIRE(kev[1].data == 0);
#endif

	ATF_REQUIRE(kevent(kq, NULL, 0, kev, nitems(kev),
			&(struct timespec) { 0, 0 }) == 1);
	ATF_REQUIRE(kev[0].ident == (uintptr_t)p[1]);
	ATF_REQUIRE(kev[0].filter == EVFILT_WRITE);

	/*
	 * NetBSD still suffers from the bug fixed by
	 * https://github.com/freebsd/freebsd/commit/4681aa72a96557236594d33069de3b60506be886
	 */
	if (kev[0].flags & EV_EOF) {
		atf_tc_skip("NetBSD's EVFILT_WRITE broken on FIFOs");
	}

	ATF_REQUIRE_MSG(kev[0].flags ==
		(EV_CLEAR | EV_RECEIPT | SPURIOUS_EV_ADD),
	    "%x", kev[0].flags);
	ATF_REQUIRE(kev[0].fflags == 0);
	ATF_REQUIRE_MSG(kev[0].data == 16384 ||
		kev[0].data == 4096 /* On OpenBSD < 7.0 */ ||
		kev[0].data == 8192 /* On OpenBSD 7.0 */ ||
		kev[0].data == 8192 /* On macOS */ ||
		kev[0].data == 65536 /* On DragonFly */,
	    "%d", (int)kev[0].data);
	ATF_REQUIRE(kev[0].udata == 0);
#endif
	int ep = epoll_create1(EPOLL_CLOEXEC);
	ATF_REQUIRE(ep >= 0);

	struct epoll_event eps[32];
	eps[0] = (struct epoll_event) {
		.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLET,
	};
	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, p[1], &eps[0]) == 0);

	ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 1);
	ATF_REQUIRE(eps[0].events == EPOLLOUT);

	char c = 0;
	ssize_t r;
	while ((r = write(p[1], &c, 1)) == 1) {
	}
	ATF_REQUIRE(r < 0);
	ATF_REQUIRE(errno == EAGAIN || errno == EWOULDBLOCK);

#if !defined(__linux__) && !defined(FORCE_EPOLL)
	r = kevent(kq, NULL, 0, kev, nitems(kev), &(struct timespec) { 0, 0 });
#if defined(__DragonFly__) || defined(__NetBSD__) || defined(__APPLE__)
	ATF_REQUIRE(r == 1);
	ATF_REQUIRE_MSG(kev[0].filter == EVFILT_READ, "%d", kev[0].filter);
#else
	ATF_REQUIRE(r == 0);
#endif
#endif
	ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 0);

	for (int i = 0; i < PIPE_BUF - 1; ++i) {
		ATF_REQUIRE(read(p[0], &c, 1) == 1);
	}

#if defined(__DragonFly__) || defined(__APPLE__)
try_again:
#endif

#if !defined(__linux__) && !defined(FORCE_EPOLL)
	r = kevent(kq, NULL, 0, kev, nitems(kev), &(struct timespec) { 0, 0 });
#if defined(__APPLE__)
	if (r == 2) {
		atf_tc_skip("Something is broken here.");
	}
#endif
#if defined(__DragonFly__)
	while (r == 1) {
		if (kev[0].filter == EVFILT_READ) {
			goto try_again;
		}

		ATF_REQUIRE(write(p[1], &c, 1) == 1);
		goto try_again;
	}
#endif
	ATF_REQUIRE(r == 0);
#endif

#if defined(__DragonFly__)
	ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 1);
	ATF_REQUIRE(eps[0].events == EPOLLOUT);
#endif
	ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 0);

	ATF_REQUIRE(read(p[0], &c, 1) == 1);

	int const pipe_buf =
#ifdef __DragonFly__
	    65536
#else
	    PIPE_BUF
#endif
	    ;

#if !defined(__linux__) && !defined(FORCE_EPOLL)
	ATF_REQUIRE(kevent(kq, NULL, 0, kev, nitems(kev),
			&(struct timespec) { 0, 0 }) == 1);
	ATF_REQUIRE(kev[0].ident == (uintptr_t)p[1]);
	ATF_REQUIRE(kev[0].filter == EVFILT_WRITE);
	ATF_REQUIRE_MSG(kev[0].flags ==
		(EV_CLEAR | EV_RECEIPT | SPURIOUS_EV_ADD),
	    "%04x", kev[0].flags);
	ATF_REQUIRE(kev[0].fflags == 0);
	ATF_REQUIRE_MSG(kev[0].data == pipe_buf, "%d", (int)kev[0].data);
	ATF_REQUIRE(kev[0].udata == 0);
#endif
	ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 1);
	ATF_REQUIRE(eps[0].events == EPOLLOUT);

	ATF_REQUIRE(read(p[0], &c, 1) == 1);

#if !defined(__linux__) && !defined(FORCE_EPOLL)
	ATF_REQUIRE(kevent(kq, NULL, 0, kev, nitems(kev),
			&(struct timespec) { 0, 0 }) == 1);
	ATF_REQUIRE(kev[0].ident == (uintptr_t)p[1]);
	ATF_REQUIRE(kev[0].filter == EVFILT_WRITE);
	ATF_REQUIRE(kev[0].flags == (EV_CLEAR | EV_RECEIPT | SPURIOUS_EV_ADD));
	ATF_REQUIRE(kev[0].fflags == 0);
#ifdef __DragonFly__
	ATF_REQUIRE_MSG(kev[0].data == pipe_buf + 0, "%d", (int)kev[0].data);
#else
	ATF_REQUIRE_MSG(kev[0].data == pipe_buf + 1, "%d", (int)kev[0].data);
#endif
	ATF_REQUIRE(kev[0].udata == 0);

	/* kqueue based emulation will trigger another edge here. */
	ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 1);
	ATF_REQUIRE(eps[0].events == EPOLLOUT);
#endif
	ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 0);

	ATF_REQUIRE(close(p[0]) == 0);

	bool data_empty = false;

#if !defined(__linux__) && !defined(FORCE_EPOLL)
	ATF_REQUIRE(kevent(kq, NULL, 0, kev, nitems(kev), NULL) == 1);
	ATF_REQUIRE(kev[0].ident == (uintptr_t)p[1]);
	ATF_REQUIRE(kev[0].filter == EVFILT_WRITE);
	ATF_REQUIRE_MSG(kev[0].flags ==
		(EV_CLEAR | EV_RECEIPT | EV_EOF | SPURIOUS_EV_ADD |
		    SPURIOUS_EV_NODATA | OPTIONAL_EV_HUP),
	    "%04x", kev[0].flags);
	ATF_REQUIRE(kev[0].fflags == 0);
	if (kev[0].data == 0) {
		/*
		 * This is the behavior on vanilla FreeBSD.
		 * Setting EV_EOF will always set data to 0.
		 * In FreeBSD 4, FIFOs were implemented with sockets. There,
		 * the EV_EOF flag and content of the data field were
		 * independent.
		 */
		data_empty = true;
	} else {
#ifdef __DragonFly__
		ATF_REQUIRE_MSG(kev[0].data == pipe_buf + 0, "%d",
		    (int)kev[0].data);
#else
		ATF_REQUIRE_MSG(kev[0].data == pipe_buf + 1, "%d",
		    (int)kev[0].data);
#endif
	}
	ATF_REQUIRE(kev[0].udata == 0);
	ATF_REQUIRE(close(kq) == 0);
#endif
	ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 1);
	if (data_empty) {
		ATF_REQUIRE(eps[0].events == EPOLLERR);
		atf_tc_skip("FreeBSD sets data to 0 on EV_EOF");
	} else {
		ATF_REQUIRE(eps[0].events == (EPOLLOUT | EPOLLERR));
	}

	ATF_REQUIRE(close(ep) == 0);

	ATF_REQUIRE(close(p[1]) == 0);
}

ATF_TC_WITHOUT_HEAD(pipe__fifo_connecting_reader);
ATF_TC_BODY_FD_LEAKCHECK(pipe__fifo_connecting_reader, tc)
{
	int p[2] = { -1, -1 };

	ATF_REQUIRE(mkfifo("the_fifo", 0666) == 0);

	ATF_REQUIRE(
	    (p[0] = open("the_fifo", O_RDONLY | O_CLOEXEC | O_NONBLOCK)) >= 0);
	ATF_REQUIRE(
	    (p[1] = open("the_fifo", O_WRONLY | O_CLOEXEC | O_NONBLOCK)) >= 0);

	{
		int fl = fcntl(p[0], F_GETFL, 0);
		int rq = O_RDONLY;
		ATF_REQUIRE((fl & O_ACCMODE) == rq);
	}
	{
		int fl = fcntl(p[1], F_GETFL, 0);
		int rq = O_WRONLY;
		ATF_REQUIRE((fl & O_ACCMODE) == rq);
	}

#if !defined(__linux__) && !defined(FORCE_EPOLL)
	int kq = kqueue();
	ATF_REQUIRE(kq >= 0);

	struct kevent kev[32];
	EV_SET(&kev[0], (unsigned int)p[1], EVFILT_WRITE, /**/
	    EV_ADD | EV_CLEAR | EV_RECEIPT, 0, 0, 0);
	EV_SET(&kev[1], (unsigned int)p[1], EVFILT_READ, /**/
	    EV_ADD | EV_CLEAR | EV_RECEIPT, 0, 0, 0);

	ATF_REQUIRE(kevent(kq, kev, 2, kev, 2, NULL) == 2);
	ATF_REQUIRE((kev[0].flags & EV_ERROR) != 0);
	ATF_REQUIRE((kev[1].flags & EV_ERROR) != 0);
	ATF_REQUIRE(kev[0].data == 0);
#if defined(__OpenBSD__)
	ATF_REQUIRE(kev[1].data == EINVAL);
#else
	ATF_REQUIRE(kev[1].data == 0);
#endif

	ATF_REQUIRE(kevent(kq, NULL, 0, kev, nitems(kev),
			&(struct timespec) { 0, 0 }) == 1);
	ATF_REQUIRE(kev[0].ident == (uintptr_t)p[1]);
	ATF_REQUIRE(kev[0].filter == EVFILT_WRITE);
	ATF_REQUIRE(kevent(kq, NULL, 0, kev, nitems(kev),
			&(struct timespec) { 0, 0 }) == 0);
#endif
	int ep = epoll_create1(EPOLL_CLOEXEC);
	ATF_REQUIRE(ep >= 0);

	struct epoll_event eps[32];
	eps[0] = (struct epoll_event) {
		.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLET,
	};
	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, p[1], &eps[0]) == 0);

	ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 1);
	/*
	 * NetBSD still suffers from the bug fixed by
	 * https://github.com/freebsd/freebsd/commit/4681aa72a96557236594d33069de3b60506be886
	 */
	if (eps[0].events & EPOLLERR) {
		atf_tc_skip("NetBSD's EVFILT_WRITE broken on FIFOs");
	}
	ATF_REQUIRE(eps[0].events == EPOLLOUT);
	ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 0);

	char c = 0;
	ssize_t r;
	while ((r = write(p[1], &c, 1)) == 1) {
	}
	ATF_REQUIRE(r < 0);
	ATF_REQUIRE(errno == EAGAIN || errno == EWOULDBLOCK);

	for (int i = 0; i < PIPE_BUF + 1; ++i) {
		ATF_REQUIRE(read(p[0], &c, 1) == 1);
	}

	ATF_REQUIRE(close(p[0]) == 0);

#if !defined(__linux__) && !defined(FORCE_EPOLL)
#if defined(__DragonFly__) || defined(__NetBSD__) || defined(__APPLE__)
	ATF_REQUIRE(kevent(kq, NULL, 0, kev, nitems(kev), NULL) == 2);
	int index = 1;
#else
	ATF_REQUIRE(kevent(kq, NULL, 0, kev, nitems(kev), NULL) == 1);
	int index = 0;
#endif
	ATF_REQUIRE(kev[index].filter == EVFILT_WRITE);
#if !defined(__APPLE__)
	ATF_REQUIRE((kev[index].flags & EV_EOF) != 0);
#endif
	ATF_REQUIRE(kevent(kq, NULL, 0, kev, nitems(kev),
			&(struct timespec) { 0, 0 }) == 0);
#endif
	ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 1);
	ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 0);

	ATF_REQUIRE(
	    (p[0] = open("the_fifo", O_RDONLY | O_CLOEXEC | O_NONBLOCK)) >= 0);

	bool will_notice_new_readers = true;

#if !defined(__linux__) && !defined(FORCE_EPOLL)
	r = kevent(kq, NULL, 0, kev, nitems(kev), &(struct timespec) { 1, 0 });
	if (r == 0) {
		will_notice_new_readers = false;
		/* This is the behavior on vanilla FreeBSD. Opening the read
		 * end of a FIFO will not send notifications to writers. */
	} else {
		ATF_REQUIRE(r == 1);
		ATF_REQUIRE(kev[0].ident == (uintptr_t)p[1]);
		ATF_REQUIRE(kev[0].filter == EVFILT_WRITE);
		ATF_REQUIRE(
		    kev[0].flags == (EV_CLEAR | EV_RECEIPT | SPURIOUS_EV_ADD));
		ATF_REQUIRE(kev[0].fflags == 0);
		ATF_REQUIRE_MSG(kev[0].data == PIPE_BUF + 1 ||
			kev[0].data == 65536 /* On DragonFly. */,
		    "%d", (int)kev[0].data);
		ATF_REQUIRE(kev[0].udata == 0);
		ATF_REQUIRE(kevent(kq, NULL, 0, kev, nitems(kev),
				&(struct timespec) { 0, 0 }) == 0);
	}
	ATF_REQUIRE(close(kq) == 0);
#endif
#if defined(__linux__)
	/* Linux 5.10 doesn't notify on new readers. */
	will_notice_new_readers = false;
#endif
	if (!will_notice_new_readers) {
		atf_tc_skip(
		    "FreeBSD/Linux/NetBSD FIFOs don't notify on new readers");
	} else {
		ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 1);
		ATF_REQUIRE(eps[0].events == EPOLLOUT);
		ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 0);
	}

	ATF_REQUIRE(close(ep) == 0);

	ATF_REQUIRE(close(p[0]) == 0);
	ATF_REQUIRE(close(p[1]) == 0);
}

ATF_TC_WITHOUT_HEAD(pipe__fifo_reads);
ATF_TC_BODY_FD_LEAKCHECK(pipe__fifo_reads, tc)
{
	int p[2] = { -1, -1 };

	ATF_REQUIRE(mkfifo("the_fifo", 0666) == 0);

	ATF_REQUIRE(
	    (p[0] = open("the_fifo", O_RDONLY | O_CLOEXEC | O_NONBLOCK)) >= 0);
	ATF_REQUIRE(
	    (p[1] = open("the_fifo", O_WRONLY | O_CLOEXEC | O_NONBLOCK)) >= 0);

	char c = 0;
	ssize_t r;
	while ((r = write(p[1], &c, 1)) == 1) {
	}
	ATF_REQUIRE(r < 0);
	ATF_REQUIRE(errno == EAGAIN || errno == EWOULDBLOCK);

	for (int i = 0; i < PIPE_BUF + 1; ++i) {
		ATF_REQUIRE(read(p[0], &c, 1) == 1);
	}

#if !defined(__linux__) && !defined(FORCE_EPOLL)
	int kq = kqueue();
	ATF_REQUIRE(kq >= 0);

	struct kevent kev[32];

	EV_SET(&kev[0], (unsigned int)p[0], EVFILT_READ, /**/
	    EV_ADD | EV_CLEAR, 0, 0, 0);

	ATF_REQUIRE(kevent(kq, kev, 1, NULL, 0, NULL) == 0);

	ATF_REQUIRE(kevent(kq, NULL, 0, kev, nitems(kev),
			&(struct timespec) { 0, 0 }) == 1);
	ATF_REQUIRE(kev[0].ident == (uintptr_t)p[0]);
	ATF_REQUIRE(kev[0].filter == EVFILT_READ);
	ATF_REQUIRE_MSG(kev[0].flags == (EV_CLEAR | SPURIOUS_EV_ADD), "%x",
	    kev[0].flags);
	ATF_REQUIRE(kev[0].fflags == 0);
	ATF_REQUIRE_MSG(kev[0].data == 65023 ||
		kev[0].data == 7679 /* on NetBSD */ ||
		kev[0].data == 3583 /* on OpenBSD */,
	    "%d", (int)kev[0].data);
	ATF_REQUIRE(kev[0].udata == 0);
#endif
	int ep = epoll_create1(EPOLL_CLOEXEC);
	ATF_REQUIRE(ep >= 0);

	struct epoll_event eps[32];

	eps[0] = (struct epoll_event) {
		.events = EPOLLIN | EPOLLRDHUP | EPOLLET,
	};
	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, p[0], &eps[0]) == 0);

	ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 1);
	ATF_REQUIRE(eps[0].events == EPOLLIN);

	while ((r = read(p[0], &c, 1)) == 1) {
	}
	ATF_REQUIRE(r < 0);
	ATF_REQUIRE(errno == EAGAIN || errno == EWOULDBLOCK);

#if !defined(__linux__) && !defined(FORCE_EPOLL)
	ATF_REQUIRE(kevent(kq, NULL, 0, kev, nitems(kev),
			&(struct timespec) { 0, 0 }) == 0);
	ATF_REQUIRE(close(kq) == 0);
#endif
	ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 0);
	ATF_REQUIRE(close(ep) == 0);

	ATF_REQUIRE(close(p[0]) == 0);
	ATF_REQUIRE(close(p[1]) == 0);
}

ATF_TC_WITHOUT_HEAD(pipe__fifo_read_eof_wakeups);
ATF_TC_BODY_FD_LEAKCHECK(pipe__fifo_read_eof_wakeups, tc)
{
	int p[2] = { -1, -1 };

	ATF_REQUIRE(mkfifo("the_fifo", 0666) == 0);

	ATF_REQUIRE(
	    (p[0] = open("the_fifo", O_RDONLY | O_CLOEXEC | O_NONBLOCK)) >= 0);
	ATF_REQUIRE(
	    (p[1] = open("the_fifo", O_WRONLY | O_CLOEXEC | O_NONBLOCK)) >= 0);

	char c = 0;
	ssize_t r;

#if !defined(__linux__) && !defined(FORCE_EPOLL)
	int kq = kqueue();
	ATF_REQUIRE(kq >= 0);

	struct kevent kev[32];

	EV_SET(&kev[0], (unsigned int)p[0], EVFILT_READ, /**/
	    EV_ADD | EV_CLEAR, 0, 0, 0);
	ATF_REQUIRE(kevent(kq, kev, 1, NULL, 0, NULL) == 0);

	ATF_REQUIRE(kevent(kq, NULL, 0, kev, nitems(kev),
			&(struct timespec) { 0, 0 }) == 0);
#endif
	int ep = epoll_create1(EPOLL_CLOEXEC);
	ATF_REQUIRE(ep >= 0);

	struct epoll_event eps[32];

	eps[0] = (struct epoll_event) {
		.events = EPOLLIN | EPOLLRDHUP | EPOLLET,
	};
	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, p[0], &eps[0]) == 0);
	ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 0);

	ATF_REQUIRE(close(p[1]) == 0);

#if !defined(__linux__) && !defined(FORCE_EPOLL)
#if defined(__APPLE__)
	ATF_REQUIRE(kevent(kq, NULL, 0, kev, nitems(kev),
			&(struct timespec) { 0, 0 }) == 0);
	atf_tc_skip("This doesn't work on macOS");
#endif
	ATF_REQUIRE(kevent(kq, NULL, 0, kev, nitems(kev),
			&(struct timespec) { 0, 0 }) == 1);
	ATF_REQUIRE(kev[0].ident == (uintptr_t)p[0]);
	ATF_REQUIRE(kev[0].filter == EVFILT_READ);
	ATF_REQUIRE(kev[0].flags ==
	    (EV_EOF | EV_CLEAR | SPURIOUS_EV_ADD | SPURIOUS_EV_NODATA |
		OPTIONAL_EV_HUP));
	ATF_REQUIRE(kev[0].fflags == 0);
	ATF_REQUIRE_MSG(kev[0].data == 0, "%d", (int)kev[0].data);
	ATF_REQUIRE(kev[0].udata == 0);
#endif
	ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 1);
	ATF_REQUIRE(eps[0].events == EPOLLHUP);

	/* Reading from a closed pipe should not trigger EVFILT_READ edges, but
	 * on vanilla FreeBSD it happens. */
	ATF_REQUIRE(read(p[0], &c, 1) == 0);

	bool has_spurious_pipe_eof_wakeups_on_read = false;

#if !defined(__linux__) && !defined(FORCE_EPOLL)
	r = kevent(kq, NULL, 0, kev, nitems(kev), &(struct timespec) { 0, 0 });
	if (r == 1) {
		has_spurious_pipe_eof_wakeups_on_read = true;
	} else {
		ATF_REQUIRE(r == 0);
	}
	ATF_REQUIRE(close(kq) == 0);
#endif
	if (has_spurious_pipe_eof_wakeups_on_read) {
		atf_tc_skip("FreeBSD generates spurious wakeups when reading "
			    "from a closed pipe");
	} else {
		ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 0);
	}

	ATF_REQUIRE(close(ep) == 0);

	ATF_REQUIRE(close(p[0]) == 0);
}

ATF_TC_WITHOUT_HEAD(pipe__fifo_read_eof_state_when_reconnecting);
ATF_TC_BODY_FD_LEAKCHECK(pipe__fifo_read_eof_state_when_reconnecting, tc)
{
	int p[2] = { -1, -1 };

	ATF_REQUIRE(mkfifo("the_fifo", 0666) == 0);

	ATF_REQUIRE(
	    (p[0] = open("the_fifo", O_RDONLY | O_CLOEXEC | O_NONBLOCK)) >= 0);
	ATF_REQUIRE(
	    (p[1] = open("the_fifo", O_WRONLY | O_CLOEXEC | O_NONBLOCK)) >= 0);

	char c = 0;

#if !defined(__linux__) && !defined(FORCE_EPOLL)
	int kq = kqueue();
	ATF_REQUIRE(kq >= 0);

	struct kevent kev[32];

	EV_SET(&kev[0], (unsigned int)p[0], EVFILT_READ, /**/
	    EV_ADD | EV_CLEAR, 0, 0, 0);
	ATF_REQUIRE(kevent(kq, kev, 1, NULL, 0, NULL) == 0);

	ATF_REQUIRE(kevent(kq, NULL, 0, kev, nitems(kev),
			&(struct timespec) { 0, 0 }) == 0);
#endif
	int ep = epoll_create1(EPOLL_CLOEXEC);
	ATF_REQUIRE(ep >= 0);

	struct epoll_event eps[32];

	eps[0] = (struct epoll_event) {
		.events = EPOLLIN | EPOLLRDHUP | EPOLLET,
	};
	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, p[0], &eps[0]) == 0);
	ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 0);

	ATF_REQUIRE(close(p[1]) == 0);

#if !defined(__linux__) && !defined(FORCE_EPOLL)
#if defined(__APPLE__)
	ATF_REQUIRE(kevent(kq, NULL, 0, kev, nitems(kev),
			&(struct timespec) { 0, 0 }) == 0);
	atf_tc_skip("This doesn't work on macOS");
#endif
	ATF_REQUIRE(kevent(kq, NULL, 0, kev, nitems(kev),
			&(struct timespec) { 0, 0 }) == 1);
	ATF_REQUIRE(kev[0].ident == (uintptr_t)p[0]);
	ATF_REQUIRE(kev[0].filter == EVFILT_READ);
	ATF_REQUIRE(kev[0].flags ==
	    (EV_EOF | EV_CLEAR | SPURIOUS_EV_ADD | SPURIOUS_EV_NODATA |
		OPTIONAL_EV_HUP));
	ATF_REQUIRE(kev[0].fflags == 0);
	ATF_REQUIRE_MSG(kev[0].data == 0, "%d", (int)kev[0].data);
	ATF_REQUIRE(kev[0].udata == 0);
#endif
	ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 1);
	ATF_REQUIRE(eps[0].events == EPOLLHUP);

	ATF_REQUIRE(
	    (p[1] = open("the_fifo", O_WRONLY | O_CLOEXEC | O_NONBLOCK)) >= 0);

#if !defined(__linux__) && !defined(FORCE_EPOLL)
	ATF_REQUIRE(kevent(kq, NULL, 0, kev, nitems(kev),
			&(struct timespec) { 0, 0 }) == 0);
#endif
	ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 0);

	ATF_REQUIRE(write(p[1], &c, 1) == 1);

	bool eof_gets_cleared_on_new_writers = true;

#if !defined(__linux__) && !defined(FORCE_EPOLL)
	ATF_REQUIRE(kevent(kq, NULL, 0, kev, nitems(kev),
			&(struct timespec) { 0, 0 }) == 1);
	ATF_REQUIRE(kev[0].ident == (uintptr_t)p[0]);
	ATF_REQUIRE(kev[0].filter == EVFILT_READ);
	if (kev[0].flags == (EV_EOF | EV_CLEAR)) {
		eof_gets_cleared_on_new_writers = false;
	} else {
		ATF_REQUIRE(kev[0].flags == (EV_CLEAR | SPURIOUS_EV_ADD));
		ATF_REQUIRE(kev[0].fflags == 0);
		ATF_REQUIRE_MSG(kev[0].data == 1, "%d", (int)kev[0].data);
		ATF_REQUIRE(kev[0].udata == 0);
	}
	ATF_REQUIRE(close(kq) == 0);
#endif
	ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 1);
	if (eof_gets_cleared_on_new_writers) {
		ATF_REQUIRE(eps[0].events == EPOLLIN);
	} else {
		ATF_REQUIRE(eps[0].events == (EPOLLIN | EPOLLHUP));
		atf_tc_skip("FreeBSD's kevent does not clear EV_EOF when a "
			    "new writer connects to a FIFO");
	}
	ATF_REQUIRE(close(ep) == 0);

	ATF_REQUIRE(close(p[0]) == 0);
	ATF_REQUIRE(close(p[1]) == 0);
}

ATF_TC_WITHOUT_HEAD(pipe__closed_read_end);
ATF_TC_BODY_FD_LEAKCHECK(pipe__closed_read_end, tc)
{
	int p[2] = { -1, -1 };

	ATF_REQUIRE(pipe2(p, O_CLOEXEC | O_NONBLOCK) == 0);
	ATF_REQUIRE(p[0] >= 0);
	ATF_REQUIRE(p[1] >= 0);
#if defined(__FreeBSD__)
	{
		cap_rights_t rights;
		ATF_REQUIRE(cap_rights_get(p[1], &rights) == 0);
		cap_rights_clear(&rights, CAP_READ);
		ATF_REQUIRE(cap_rights_limit(p[1], &rights) == 0);
	}
#endif

	ATF_REQUIRE(close(p[0]) == 0);

#if !defined(__linux__) && !defined(FORCE_EPOLL)
	int kq = kqueue();
	ATF_REQUIRE(kq >= 0);

	struct kevent kev[32];
	EV_SET(&kev[0], (unsigned int)p[1], EVFILT_READ, /**/
	    EV_ADD | EV_CLEAR | EV_RECEIPT,		 /**/
	    0, 0, 0);
	EV_SET(&kev[1], (unsigned int)p[1], EVFILT_WRITE, /**/
	    EV_ADD | EV_CLEAR | EV_RECEIPT,		  /**/
	    0, 0, 0);

	ATF_REQUIRE(kevent(kq, kev, 2, kev, 2, NULL) == 2);
	ATF_REQUIRE((kev[0].flags & EV_ERROR) != 0);
	ATF_REQUIRE(kev[0].data == 0);
	ATF_REQUIRE((kev[1].flags & EV_ERROR) != 0);
#ifdef __NetBSD__
	ATF_REQUIRE(kev[1].data == EBADF);
#elif defined(__APPLE__)
	ATF_REQUIRE(kev[1].data == 0);
	atf_tc_skip("This doesn't work on macOS");
#elif defined(__DragonFly__)
	ATF_REQUIRE(kev[1].data == 0);
#else
	ATF_REQUIRE(kev[1].data == EPIPE);
#endif

	ATF_REQUIRE(kevent(kq, NULL, 0, kev, nitems(kev),
			&(struct timespec) { 0, 0 }) ==
#if defined(__DragonFly__)
	    2
#else
	    1
#endif
	);
	ATF_REQUIRE(kev[0].ident == (uintptr_t)p[1]);
	ATF_REQUIRE(kev[0].filter == EVFILT_READ);
	ATF_REQUIRE_MSG(kev[0].flags ==
		(EV_EOF | EV_CLEAR | SPURIOUS_EV_ADD | EV_RECEIPT |
		    SPURIOUS_EV_NODATA | OPTIONAL_EV_HUP),
	    "%04x", kev[0].flags);
	ATF_REQUIRE(kev[0].fflags == 0);
	ATF_REQUIRE_MSG(kev[0].data == 0, "%d", (int)kev[0].data);
	ATF_REQUIRE(kev[0].udata == 0);
#if defined(__DragonFly__)
	ATF_REQUIRE(kev[1].ident == (uintptr_t)p[1]);
	ATF_REQUIRE(kev[1].filter == EVFILT_WRITE);
	ATF_REQUIRE(kev[1].flags & EV_EOF);
	ATF_REQUIRE(kev[1].data == 0);
#endif
	ATF_REQUIRE(close(kq) == 0);
#endif
	{
		int ep = epoll_create1(EPOLL_CLOEXEC);
		ATF_REQUIRE(ep >= 0);

		struct epoll_event eps[32];
		eps[0] = (struct epoll_event) {
			.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLET,
		};
		int ret = epoll_ctl(ep, EPOLL_CTL_ADD, p[1], &eps[0]);
		ATF_REQUIRE(ret == 0);

		ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 1);
#if defined(__OpenBSD__) || defined(__DragonFly__)
		if (eps[0].events == EPOLLHUP) {
			atf_tc_skip("OpenBSD/DragonFly have duplex pipes but "
				    "no way to tell p[0] and p[1] apart");
		}
#endif
		ATF_REQUIRE_MSG(eps[0].events == (EPOLLOUT | EPOLLERR), "%04x",
		    eps[0].events);
		ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 0);

		ATF_REQUIRE(close(ep) == 0);
	}
	{
		int ep = epoll_create1(EPOLL_CLOEXEC);
		ATF_REQUIRE(ep >= 0);

		struct epoll_event eps[32];
		eps[0] = (struct epoll_event) {
			.events = EPOLLIN | EPOLLRDHUP | EPOLLET,
		};
		ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, p[1], &eps[0]) == 0);

		ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 1);
		ATF_REQUIRE(eps[0].events == EPOLLERR);
		ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 0);

		ATF_REQUIRE(close(ep) == 0);
	}
	{
		int ep = epoll_create1(EPOLL_CLOEXEC);
		ATF_REQUIRE(ep >= 0);

		struct epoll_event eps[32];
		eps[0] = (struct epoll_event) { .events = EPOLLET };
		ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, p[1], &eps[0]) == 0);

		ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 1);
		ATF_REQUIRE(eps[0].events == EPOLLERR);
		ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 0);

		ATF_REQUIRE(close(ep) == 0);
	}
	{
		int ep = epoll_create1(EPOLL_CLOEXEC);
		ATF_REQUIRE(ep >= 0);

		struct epoll_event eps[32];
		eps[0] = (struct epoll_event) { .events = 0 };
		ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, p[1], &eps[0]) == 0);

		ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 1);
		ATF_REQUIRE(eps[0].events == EPOLLERR);
		ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 1);
		ATF_REQUIRE(eps[0].events == EPOLLERR);
		ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 1);
		ATF_REQUIRE(eps[0].events == EPOLLERR);

		ATF_REQUIRE(close(ep) == 0);
	}

	ATF_REQUIRE(close(p[1]) == 0);
}

ATF_TC_WITHOUT_HEAD(pipe__closed_read_end_of_duplex);
ATF_TC_BODY_FD_LEAKCHECK(pipe__closed_read_end_of_duplex, tc)
{
	int p[2] = { -1, -1 };

	ATF_REQUIRE(pipe2(p, O_CLOEXEC | O_NONBLOCK) == 0);
	ATF_REQUIRE(p[0] >= 0);
	ATF_REQUIRE(p[1] >= 0);

	{
		int fl = fcntl(p[0], F_GETFL, 0);
		if ((fl & O_ACCMODE) != O_RDWR) {
			atf_tc_skip("need duplex pipe for this test");
		}
	}
	{
		int fl = fcntl(p[1], F_GETFL, 0);
		if ((fl & O_ACCMODE) != O_RDWR) {
			atf_tc_skip("need duplex pipe for this test");
		}
	}

	ATF_REQUIRE(close(p[0]) == 0);

	{
		int ep = epoll_create1(EPOLL_CLOEXEC);
		ATF_REQUIRE(ep >= 0);

		struct epoll_event eps[32];
		eps[0] = (struct epoll_event) {
			.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLET,
		};
		int ret = epoll_ctl(ep, EPOLL_CTL_ADD, p[1], &eps[0]);
		ATF_REQUIRE(ret == 0);

		ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 1);
		ATF_REQUIRE(eps[0].events == EPOLLHUP);
		ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 0);

		ATF_REQUIRE(close(ep) == 0);
	}
	{
		int ep = epoll_create1(EPOLL_CLOEXEC);
		ATF_REQUIRE(ep >= 0);

		struct epoll_event eps[32];
		eps[0] = (struct epoll_event) { .events = EPOLLET };
		ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, p[1], &eps[0]) == 0);

		ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 1);
		ATF_REQUIRE(eps[0].events == EPOLLHUP);
		ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 0);

		ATF_REQUIRE(close(ep) == 0);
	}
	{
		int ep = epoll_create1(EPOLL_CLOEXEC);
		ATF_REQUIRE(ep >= 0);

		struct epoll_event eps[32];
		eps[0] = (struct epoll_event) { .events = 0 };
		ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, p[1], &eps[0]) == 0);

		ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 1);
		ATF_REQUIRE(eps[0].events == EPOLLHUP);
		ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 1);
		ATF_REQUIRE(eps[0].events == EPOLLHUP);
		ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 1);
		ATF_REQUIRE(eps[0].events == EPOLLHUP);

		ATF_REQUIRE(close(ep) == 0);
	}

	ATF_REQUIRE(close(p[1]) == 0);
}

ATF_TC_WITHOUT_HEAD(pipe__closed_read_end_register_before_close);
ATF_TC_BODY_FD_LEAKCHECK(pipe__closed_read_end_register_before_close, tc)
{
	int p[2] = { -1, -1 };

	ATF_REQUIRE(pipe2(p, O_CLOEXEC | O_NONBLOCK) == 0);
	ATF_REQUIRE(p[0] >= 0);
	ATF_REQUIRE(p[1] >= 0);

#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__DragonFly__)
	{
		int fl = fcntl(p[0], F_GETFL, 0);
		ATF_REQUIRE((fl & O_ACCMODE) == O_RDWR);
	}
	{
		int fl = fcntl(p[1], F_GETFL, 0);
		ATF_REQUIRE((fl & O_ACCMODE) == O_RDWR);
	}
#if defined(__OpenBSD__) || defined(__DragonFly__)
	atf_tc_skip("OpenBSD/DragonFly have duplex pipes but "
		    "no way to tell p[0] and p[1] apart");
#else
	{
		cap_rights_t rights;
		ATF_REQUIRE(cap_rights_get(p[1], &rights) == 0);
		cap_rights_clear(&rights, CAP_READ);
		ATF_REQUIRE(cap_rights_limit(p[1], &rights) == 0);
	}
#endif
#else
	{
		int fl = fcntl(p[0], F_GETFL, 0);
		ATF_REQUIRE((fl & O_ACCMODE) == O_RDONLY);
	}
	{
		int fl = fcntl(p[1], F_GETFL, 0);
		ATF_REQUIRE((fl & O_ACCMODE) == O_WRONLY);
	}
#endif

#if !defined(__linux__) && !defined(FORCE_EPOLL)
	int kq = kqueue();
	ATF_REQUIRE(kq >= 0);

	struct kevent kev[32];
	EV_SET(&kev[0], (unsigned int)p[1], EVFILT_READ, /**/
	    EV_ADD | EV_CLEAR | EV_RECEIPT,		 /**/
	    0, 0, 0);
	EV_SET(&kev[1], (unsigned int)p[1], EVFILT_WRITE, /**/
	    EV_ADD | EV_CLEAR | EV_RECEIPT,		  /**/
	    0, 0, 0);

	ATF_REQUIRE(kevent(kq, kev, 2, kev, 2, NULL) == 2);
	ATF_REQUIRE((kev[0].flags & EV_ERROR) != 0);
	ATF_REQUIRE(kev[0].data == 0);
	ATF_REQUIRE((kev[1].flags & EV_ERROR) != 0);
	ATF_REQUIRE(kev[1].data == 0);
#endif
	int ep = epoll_create1(EPOLL_CLOEXEC);
	ATF_REQUIRE(ep >= 0);

	struct epoll_event eps[32];
	eps[0] = (struct epoll_event) {
		.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLET,
	};
	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, p[1], &eps[0]) == 0);

	ATF_REQUIRE(close(p[0]) == 0);

#if !defined(__linux__) && !defined(FORCE_EPOLL)
	ATF_REQUIRE(kevent(kq, NULL, 0, kev, nitems(kev),
			&(struct timespec) { 0, 0 }) == 2);
	{
		ATF_REQUIRE(kev[1].ident == (uintptr_t)p[1]);
		ATF_REQUIRE(kev[1].filter == EVFILT_READ);
		ATF_REQUIRE_MSG(kev[1].flags ==
			(EV_EOF | EV_CLEAR | SPURIOUS_EV_ADD | EV_RECEIPT),
		    "%04x", kev[1].flags);
		ATF_REQUIRE(kev[1].fflags == 0);
		ATF_REQUIRE_MSG(kev[1].data == 0, "%d", (int)kev[0].data);
		ATF_REQUIRE(kev[1].udata == 0);
	}
	{
		ATF_REQUIRE(kev[0].ident == (uintptr_t)p[1]);
		ATF_REQUIRE(kev[0].filter == EVFILT_WRITE);
		ATF_REQUIRE_MSG(kev[0].flags ==
			(EV_EOF | EV_CLEAR | SPURIOUS_EV_ADD |
			    SPURIOUS_EV_ONESHOT | EV_RECEIPT),
		    "%04x", kev[0].flags);
		ATF_REQUIRE(kev[0].fflags == 0);
		if (kev[0].data == 0) {
			atf_tc_skip("kev.data == 0 is a valid value on EV_EOF");
		}
		ATF_REQUIRE_MSG(kev[0].data == 16384, "%d", (int)kev[0].data);
		ATF_REQUIRE(kev[0].udata == 0);
	}
	ATF_REQUIRE(close(kq) == 0);
#endif

	ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 1);
	ATF_REQUIRE(eps[0].events == (EPOLLOUT | EPOLLERR));
	ATF_REQUIRE(close(ep) == 0);

	ATF_REQUIRE(close(p[1]) == 0);
}

ATF_TC_WITHOUT_HEAD(pipe__closed_write_end);
ATF_TC_BODY_FD_LEAKCHECK(pipe__closed_write_end, tc)
{
	int p[2] = { -1, -1 };

	ATF_REQUIRE(pipe2(p, O_CLOEXEC | O_NONBLOCK) == 0);
	ATF_REQUIRE(p[0] >= 0);
	ATF_REQUIRE(p[1] >= 0);

#if defined(__FreeBSD__)
	{
		cap_rights_t rights;
		ATF_REQUIRE(cap_rights_get(p[0], &rights) == 0);
		cap_rights_clear(&rights, CAP_WRITE);
		ATF_REQUIRE(cap_rights_limit(p[0], &rights) == 0);
	}
#endif

	char c = 0;
	ssize_t r;
	while ((r = write(p[1], &c, 1)) == 1) {
	}
	ATF_REQUIRE(r < 0);
	ATF_REQUIRE(errno == EAGAIN || errno == EWOULDBLOCK);

	ATF_REQUIRE(close(p[1]) == 0);

#if !defined(__linux__) && !defined(FORCE_EPOLL)
	int kq = kqueue();
	ATF_REQUIRE(kq >= 0);

	struct kevent kev[32];
	EV_SET(&kev[0], (unsigned int)p[0], EVFILT_READ, /**/
	    EV_ADD | EV_CLEAR | EV_RECEIPT,		 /**/
	    0, 0, 0);
	EV_SET(&kev[1], (unsigned int)p[0], EVFILT_WRITE, /**/
	    EV_ADD | EV_CLEAR | EV_RECEIPT,		  /**/
	    0, 0, 0);

	ATF_REQUIRE(kevent(kq, kev, 2, kev, 2, NULL) == 2);
	ATF_REQUIRE((kev[0].flags & EV_ERROR) != 0);
	ATF_REQUIRE(kev[0].data == 0);
	ATF_REQUIRE((kev[1].flags & EV_ERROR) != 0);
#ifdef __NetBSD__
	ATF_REQUIRE(kev[1].data == EBADF);
#elif defined(__APPLE__)
	ATF_REQUIRE(kev[1].data == 0);
	atf_tc_skip("This doesn't work on macOS");
#elif defined(__DragonFly__)
	ATF_REQUIRE(kev[1].data == 0);
#else
	ATF_REQUIRE(kev[1].data == EPIPE);
#endif

	ATF_REQUIRE(kevent(kq, NULL, 0, kev, nitems(kev),
			&(struct timespec) { 0, 0 }) ==
#if defined(__DragonFly__)
	    2
#else
	    1
#endif
	);
	ATF_REQUIRE(kev[0].ident == (uintptr_t)p[0]);
	ATF_REQUIRE(kev[0].filter == EVFILT_READ);
	ATF_REQUIRE_MSG(kev[0].flags ==
		(EV_EOF | (EV_CLEAR | SPURIOUS_EV_ADD) | EV_RECEIPT |
		    OPTIONAL_EV_HUP),
	    "%04x", kev[0].flags);
	ATF_REQUIRE(kev[0].fflags == 0);
	int const pipe_read_size =
#if defined(__NetBSD__) || defined(__OpenBSD__)
	    16384
#elif defined(__DragonFly__)
	    32768
#else
	    65536
#endif
	    ;
	ATF_REQUIRE_MSG(kev[0].data == pipe_read_size, "%d", (int)kev[0].data);
	ATF_REQUIRE(kev[0].udata == 0);
#if defined(__DragonFly__)
	ATF_REQUIRE(kev[1].ident == (uintptr_t)p[0]);
	ATF_REQUIRE(kev[1].filter == EVFILT_WRITE);
	ATF_REQUIRE(kev[1].flags & EV_EOF);
#endif
	ATF_REQUIRE(close(kq) == 0);
#endif
	{
		int ep = epoll_create1(EPOLL_CLOEXEC);
		ATF_REQUIRE(ep >= 0);

		struct epoll_event eps[32];
		eps[0] = (struct epoll_event) {
			.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLET,
		};
		int ret = epoll_ctl(ep, EPOLL_CTL_ADD, p[0], &eps[0]);
		ATF_REQUIRE(ret == 0);

		ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 1);
		ATF_REQUIRE(eps[0].events == (EPOLLIN | EPOLLHUP));
		ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 0);

		ATF_REQUIRE(close(ep) == 0);
	}
	{
		int ep = epoll_create1(EPOLL_CLOEXEC);
		ATF_REQUIRE(ep >= 0);

		struct epoll_event eps[32];
		eps[0] = (struct epoll_event) { .events = EPOLLOUT | EPOLLET };
		ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, p[0], &eps[0]) == 0);

		ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 1);
#if defined(__OpenBSD__) || defined(__DragonFly__)
		if (eps[0].events == (EPOLLOUT | EPOLLERR) ||
		    eps[0].events == EPOLLERR) {
			atf_tc_skip("OpenBSD/DragonFly have duplex pipes but "
				    "no way to tell p[0] and p[1] apart");
		}
#endif
		ATF_REQUIRE_MSG(eps[0].events == EPOLLHUP, "%04x",
		    eps[0].events);
		ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 0);

		ATF_REQUIRE(close(ep) == 0);
	}
	{
		int ep = epoll_create1(EPOLL_CLOEXEC);
		ATF_REQUIRE(ep >= 0);

		struct epoll_event eps[32];
		eps[0] = (struct epoll_event) { .events = EPOLLET };
		ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, p[0], &eps[0]) == 0);

		ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 1);
		ATF_REQUIRE(eps[0].events == EPOLLHUP);
		ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 0);

		ATF_REQUIRE(close(ep) == 0);
	}

	ATF_REQUIRE(close(p[0]) == 0);
}

ATF_TC_WITHOUT_HEAD(pipe__closed_write_end_register_before_close);
ATF_TC_BODY_FD_LEAKCHECK(pipe__closed_write_end_register_before_close, tc)
{
	int p[2] = { -1, -1 };

	ATF_REQUIRE(pipe2(p, O_CLOEXEC | O_NONBLOCK) == 0);
	ATF_REQUIRE(p[0] >= 0);
	ATF_REQUIRE(p[1] >= 0);

#if !defined(__linux__) && !defined(FORCE_EPOLL)
	int kq = kqueue();
	ATF_REQUIRE(kq >= 0);

	struct kevent kev[32];
	EV_SET(&kev[0], (unsigned int)p[0], EVFILT_READ, /**/
	    EV_ADD | EV_CLEAR | EV_RECEIPT,		 /**/
	    0, 0, 0);
	EV_SET(&kev[1], (unsigned int)p[0], EVFILT_WRITE, /**/
	    EV_ADD | EV_CLEAR | EV_RECEIPT,		  /**/
	    0, 0, 0);

	ATF_REQUIRE(kevent(kq, kev, 2, kev, 2, NULL) == 2);
	ATF_REQUIRE((kev[0].flags & EV_ERROR) != 0);
	ATF_REQUIRE(kev[0].data == 0);
	ATF_REQUIRE((kev[1].flags & EV_ERROR) != 0);
	ATF_REQUIRE(kev[1].data == 0);
#endif
	int ep = epoll_create1(EPOLL_CLOEXEC);
	ATF_REQUIRE(ep >= 0);

	struct epoll_event eps[32];
	eps[0] = (struct epoll_event) {
		.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLET,
	};
	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, p[0], &eps[0]) == 0);

	char c = 0;
	ssize_t r;
	while ((r = write(p[1], &c, 1)) == 1) {
	}
	ATF_REQUIRE(r < 0);
	ATF_REQUIRE(errno == EAGAIN || errno == EWOULDBLOCK);

	ATF_REQUIRE(close(p[1]) == 0);

#if !defined(__linux__) && !defined(FORCE_EPOLL)
#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__DragonFly__) || \
    defined(__APPLE__)
	ATF_REQUIRE(kevent(kq, NULL, 0, kev, nitems(kev),
			&(struct timespec) { 0, 0 }) == 2);
#if defined(__APPLE__)
	{
		struct kevent tmp;
		memcpy(&tmp, &kev[0], sizeof(tmp));
		memcpy(&kev[0], &kev[1], sizeof(tmp));
		memcpy(&kev[1], &tmp, sizeof(tmp));
	}
#endif
	{
		ATF_REQUIRE(kev[0].ident == (uintptr_t)p[0]);
		ATF_REQUIRE(kev[0].filter == EVFILT_WRITE);
		ATF_REQUIRE_MSG(kev[0].flags ==
			(EV_EOF | EV_CLEAR | SPURIOUS_EV_ONESHOT | EV_RECEIPT |
			    SPURIOUS_EV_ADD | SPURIOUS_EV_NODATA |
			    OPTIONAL_EV_HUP),
		    "%04x", kev[0].flags);
		ATF_REQUIRE(kev[0].fflags == 0);
		ATF_REQUIRE_MSG(kev[0].data == 4096 ||
			kev[0].data == 512 /* on FreeBSD 11.3 */ ||
			kev[0].data == 0 /* on OpenBSD 6.6 */,
		    "%d", (int)kev[0].data);
		ATF_REQUIRE(kev[0].udata == 0);
	}
	{
		ATF_REQUIRE(kev[1].ident == (uintptr_t)p[0]);
		ATF_REQUIRE(kev[1].filter == EVFILT_READ);
		ATF_REQUIRE_MSG(kev[1].flags ==
			(EV_EOF | EV_CLEAR | EV_RECEIPT | SPURIOUS_EV_ADD |
			    OPTIONAL_EV_HUP),
		    "%04x", kev[1].flags);
		ATF_REQUIRE(kev[1].fflags == 0);
		ATF_REQUIRE_MSG(kev[1].data == 65536 ||
			kev[1].data == 16384 /* on OpenBSD 6.6 */ ||
			kev[1].data == 32768 /* on DragonFly 5.8 */,
		    "%d", (int)kev[1].data);
		ATF_REQUIRE(kev[1].udata == 0);
	}
#else
	ATF_REQUIRE(kevent(kq, NULL, 0, kev, nitems(kev),
			&(struct timespec) { 0, 0 }) == 1);
	{
		ATF_REQUIRE(kev[0].ident == (uintptr_t)p[0]);
		ATF_REQUIRE(kev[0].filter == EVFILT_READ);
		ATF_REQUIRE_MSG(kev[0].flags ==
			(EV_EOF | EV_CLEAR | SPURIOUS_EV_ADD | EV_RECEIPT),
		    "%04x", kev[0].flags);
		ATF_REQUIRE(kev[0].fflags == 0);
		ATF_REQUIRE_MSG(kev[0].data == 16384, "%d", (int)kev[1].data);
		ATF_REQUIRE(kev[0].udata == 0);
	}
#endif
	ATF_REQUIRE(close(kq) == 0);
#endif
	ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 1);
	ATF_REQUIRE(eps[0].events == (EPOLLIN | EPOLLHUP));
	ATF_REQUIRE(close(ep) == 0);

	ATF_REQUIRE(close(p[0]) == 0);
}

ATF_TP_ADD_TCS(tp)
{
	(void)print_statbuf;

	ATF_TP_ADD_TC(tp, pipe__simple_poll);
	ATF_TP_ADD_TC(tp, pipe__poll_write_end_after_read_end_close);
	ATF_TP_ADD_TC(tp, pipe__poll_full_write_end_after_read_end_close);
	ATF_TP_ADD_TC(tp, pipe__poll_full_write_end_after_read_end_close_hup);
	ATF_TP_ADD_TC(tp,
	    pipe__poll_full_minus_1b_write_end_after_read_end_close);
	ATF_TP_ADD_TC(tp,
	    pipe__poll_full_minus_511b_write_end_after_read_end_close);
	ATF_TP_ADD_TC(tp,
	    pipe__poll_full_minus_512b_write_end_after_read_end_close);
	ATF_TP_ADD_TC(tp, pipe__pipe_event_poll);
	ATF_TP_ADD_TC(tp, pipe__fifo_writes);
	ATF_TP_ADD_TC(tp, pipe__fifo_connecting_reader);
	ATF_TP_ADD_TC(tp, pipe__fifo_reads);
	ATF_TP_ADD_TC(tp, pipe__fifo_read_eof_wakeups);
	ATF_TP_ADD_TC(tp, pipe__fifo_read_eof_state_when_reconnecting);
	ATF_TP_ADD_TC(tp, pipe__closed_read_end);
	ATF_TP_ADD_TC(tp, pipe__closed_read_end_of_duplex);
	ATF_TP_ADD_TC(tp, pipe__closed_read_end_register_before_close);
	ATF_TP_ADD_TC(tp, pipe__closed_write_end);
	ATF_TP_ADD_TC(tp, pipe__closed_write_end_register_before_close);

	return atf_no_error();
}
