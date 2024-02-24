#define _GNU_SOURCE

#include <atf-c.h>

#include <sys/types.h>

#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/stat.h>

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

ATF_TC_WITHOUT_HEAD(socketpair__simple_socketpair);
ATF_TC_BODY_FD_LEAKCHECK(socketpair__simple_socketpair, tc)
{
	int p[2] = { -1, -1 };
	ATF_REQUIRE(socketpair(PF_LOCAL, SOCK_STREAM, 0, p) == 0);

	{
		struct pollfd pfd = { .fd = p[0], .events = POLLIN };
		ATF_REQUIRE(poll(&pfd, 1, 0) == 0);
		ATF_REQUIRE(pfd.revents == 0);

		int ep = epoll_create1(EPOLL_CLOEXEC);
		ATF_REQUIRE(ep >= 0);

		struct epoll_event eps[32];
		eps[0] = (struct epoll_event) { .events = EPOLLIN };
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
		eps[0] = (struct epoll_event) { .events = EPOLLOUT };
		ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, p[1], &eps[0]) == 0);

		ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 1);
		ATF_REQUIRE(eps[0].events == EPOLLOUT);
		ATF_REQUIRE(close(ep) == 0);
	}

	ATF_REQUIRE(close(p[0]) == 0);
	ATF_REQUIRE(close(p[1]) == 0);
}

ATF_TC_WITHOUT_HEAD(socketpair__simple_edge_triggering);
ATF_TC_BODY_FD_LEAKCHECK(socketpair__simple_edge_triggering, tc)
{
	int p[2] = { -1, -1 };
	ATF_REQUIRE(socketpair(PF_LOCAL,
			SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK, 0, p) == 0);

	int ep = epoll_create1(EPOLL_CLOEXEC);
	ATF_REQUIRE(ep >= 0);

	struct epoll_event eps[32];
	eps[0] = (struct epoll_event) { .events = EPOLLOUT | EPOLLET };
	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, p[0], &eps[0]) == 0);

	ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 1);
	ATF_REQUIRE(eps[0].events == EPOLLOUT);
	ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 0);

	eps[0] = (struct epoll_event) {
		.events = EPOLLIN | EPOLLRDHUP | EPOLLOUT | EPOLLET,
	};
	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_MOD, p[0], &eps[0]) == 0);

	ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 1);
	ATF_REQUIRE(eps[0].events == EPOLLOUT);
	ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 0);

	char c = 0;
	ATF_REQUIRE(write(p[1], &c, 1) == 1);

	ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 1);
	ATF_REQUIRE(eps[0].events == (EPOLLIN | EPOLLOUT));
	ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 0);

	ATF_REQUIRE(read(p[0], &c, 1) == 1);
	ATF_REQUIRE(read(p[0], &c, 1) < 0);

	ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 0);

	ATF_REQUIRE(shutdown(p[1], SHUT_WR) == 0);
	ATF_REQUIRE(epoll_wait(ep, eps, 32, -1) == 1);
	ATF_REQUIRE(eps[0].events == (EPOLLIN | EPOLLRDHUP | EPOLLOUT));
	ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 0);

	eps[0] = (struct epoll_event) {
		.events = EPOLLRDHUP | EPOLLOUT | EPOLLET,
	};
	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_MOD, p[0], &eps[0]) == 0);

	ATF_REQUIRE(epoll_wait(ep, eps, 32, -1) == 1);
	ATF_REQUIRE(eps[0].events == (EPOLLRDHUP | EPOLLOUT));
	ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 0);

#if defined(__APPLE__)
	ATF_REQUIRE_ERRNO(ENOTCONN, shutdown(p[0], SHUT_RD) < 0);
#else
	ATF_REQUIRE(shutdown(p[0], SHUT_RD) == 0);

	ATF_REQUIRE(epoll_wait(ep, eps, 32, -1) == 1);
	ATF_REQUIRE(eps[0].events == (EPOLLRDHUP | EPOLLOUT));
	ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 0);
#endif

	ATF_REQUIRE(shutdown(p[0], SHUT_WR) == 0);

	ATF_REQUIRE(epoll_wait(ep, eps, 32, -1) == 1);
	ATF_REQUIRE(eps[0].events == (EPOLLRDHUP | EPOLLOUT | EPOLLHUP));
	ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 0);

	eps[0] = (struct epoll_event) { .events = EPOLLET };
	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_MOD, p[0], &eps[0]) == 0);

	ATF_REQUIRE(epoll_wait(ep, eps, 32, -1) == 1);
	ATF_REQUIRE(eps[0].events == EPOLLHUP);
	ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 0);

	eps[0] = (struct epoll_event) { .events = 0 };
	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_MOD, p[0], &eps[0]) == 0);

	ATF_REQUIRE(epoll_wait(ep, eps, 32, -1) == 1);
	ATF_REQUIRE(eps[0].events == EPOLLHUP);
	ATF_REQUIRE(epoll_wait(ep, eps, 32, -1) == 1);
	ATF_REQUIRE(eps[0].events == EPOLLHUP);

	eps[0] = (struct epoll_event) { .events = EPOLLOUT };
	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_MOD, p[0], &eps[0]) == 0);

	ATF_REQUIRE(epoll_wait(ep, eps, 32, -1) == 1);
	ATF_REQUIRE(eps[0].events == (EPOLLOUT | EPOLLHUP));
	ATF_REQUIRE(epoll_wait(ep, eps, 32, -1) == 1);
	ATF_REQUIRE(eps[0].events == (EPOLLOUT | EPOLLHUP));

	eps[0] = (struct epoll_event) { .events = EPOLLOUT | EPOLLET };
	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_MOD, p[0], &eps[0]) == 0);

	ATF_REQUIRE(epoll_wait(ep, eps, 32, -1) == 1);
	ATF_REQUIRE(eps[0].events == (EPOLLOUT | EPOLLHUP));
	ATF_REQUIRE(epoll_wait(ep, eps, 32, 0) == 0);

	ATF_REQUIRE(close(ep) == 0);

	ATF_REQUIRE(close(p[0]) == 0);
	ATF_REQUIRE(close(p[1]) == 0);
}

ATF_TC_WITHOUT_HEAD(socketpair__epollhup);
ATF_TC_BODY_FD_LEAKCHECK(socketpair__epollhup, tc)
{
	int p[2] = { -1, -1 };
	ATF_REQUIRE(socketpair(PF_LOCAL,
			SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK, 0, p) == 0);

	int ep = epoll_create1(EPOLL_CLOEXEC);
	ATF_REQUIRE(ep >= 0);

	struct epoll_event eps[1];
	eps[0] = (struct epoll_event) { .events = EPOLLET };
	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, p[0], &eps[0]) == 0);

	ATF_REQUIRE(epoll_wait(ep, eps, 1, 0) == 0);

	ATF_REQUIRE(shutdown(p[1], SHUT_WR) == 0);
	ATF_REQUIRE(epoll_wait(ep, eps, 1, 0) == 0);

	ATF_REQUIRE(shutdown(p[0], SHUT_WR) == 0);
	ATF_REQUIRE(epoll_wait(ep, eps, 1, 0) == 1);
	ATF_REQUIRE(eps[0].events == EPOLLHUP);
	ATF_REQUIRE(epoll_wait(ep, eps, 1, 0) == 0);

	ATF_REQUIRE(close(ep) == 0);

	ATF_REQUIRE(close(p[0]) == 0);
	ATF_REQUIRE(close(p[1]) == 0);
}

ATF_TC_WITHOUT_HEAD(socketpair__epollrdhup);
ATF_TC_BODY_FD_LEAKCHECK(socketpair__epollrdhup, tc)
{
	int p[2] = { -1, -1 };
	ATF_REQUIRE(socketpair(PF_LOCAL,
			SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK, 0, p) == 0);

	int ep = epoll_create1(EPOLL_CLOEXEC);
	ATF_REQUIRE(ep >= 0);

	struct epoll_event eps[1];
	eps[0] = (struct epoll_event) { .events = EPOLLRDHUP };
	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, p[0], &eps[0]) == 0);

	ATF_REQUIRE(epoll_wait(ep, eps, 1, 0) == 0);

	ATF_REQUIRE(shutdown(p[1], SHUT_WR) == 0);
	ATF_REQUIRE(epoll_wait(ep, eps, 1, 0) == 1);
	ATF_REQUIRE(eps[0].events == EPOLLRDHUP);
	ATF_REQUIRE(epoll_wait(ep, eps, 1, 0) == 1);
	ATF_REQUIRE(eps[0].events == EPOLLRDHUP);

	ATF_REQUIRE(shutdown(p[0], SHUT_WR) == 0);
	ATF_REQUIRE(epoll_wait(ep, eps, 1, 0) == 1);
	ATF_REQUIRE(eps[0].events == (EPOLLRDHUP | EPOLLHUP));
	ATF_REQUIRE(epoll_wait(ep, eps, 1, 0) == 1);
	ATF_REQUIRE(eps[0].events == (EPOLLRDHUP | EPOLLHUP));

	ATF_REQUIRE(close(ep) == 0);

	ATF_REQUIRE(close(p[0]) == 0);
	ATF_REQUIRE(close(p[1]) == 0);
}

ATF_TP_ADD_TCS(tp)
{
	ATF_TP_ADD_TC(tp, socketpair__simple_socketpair);
	ATF_TP_ADD_TC(tp, socketpair__simple_edge_triggering);
	ATF_TP_ADD_TC(tp, socketpair__epollhup);
	ATF_TP_ADD_TC(tp, socketpair__epollrdhup);

	return atf_no_error();
}
