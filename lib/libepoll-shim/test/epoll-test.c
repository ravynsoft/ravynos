#define _GNU_SOURCE

#include <atf-c.h>

#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/param.h>
#include <sys/signalfd.h>
#include <sys/timerfd.h>

#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include <err.h>
#include <fcntl.h>
#include <limits.h>
#include <poll.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#ifndef __linux__
#include <epoll-shim/detail/poll.h>
#endif

#include "atf-c-leakcheck.h"

#ifdef USE_EPOLLRDHUP_LINUX_DEFINITION
#undef EPOLLRDHUP
#define EPOLLRDHUP 0x2000
#endif

static void
fd_pipe(int fds[3])
{
	fds[2] = -1;
	ATF_REQUIRE(pipe2(fds, O_CLOEXEC) == 0);
}

static void
fd_domain_socket(int fds[3])
{
	fds[2] = -1;
	ATF_REQUIRE(socketpair(PF_LOCAL, SOCK_STREAM, 0, fds) == 0);
}
static int connector_epfd = -1;

static void *
connector_client(void *arg)
{
	(void)arg;

	int sock = socket(PF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
	ATF_REQUIRE(sock >= 0);

	if (connector_epfd >= 0) {
		int ep = connector_epfd;

		struct epoll_event event;
		event.events = EPOLLOUT | EPOLLIN;
		event.data.fd = sock;

		ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, sock, &event) == 0);

		int ret;

		for (int i = 0; i < 3; ++i) {
			ret = epoll_wait(ep, &event, 1, 300);
#ifndef EV_FORCEONESHOT
			if (ret == 0) {
				continue;
			}
#endif
			ATF_REQUIRE(ret == 1);
			ATF_REQUIRE(event.events == (EPOLLOUT | EPOLLHUP));
		}
	}

	struct sockaddr_in addr = { 0 };
	addr.sin_family = AF_INET;
	addr.sin_port = htons(1337);
	ATF_REQUIRE(inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) == 1);

	ATF_REQUIRE(
	    connect(sock, (struct sockaddr const *)&addr, sizeof(addr)) == 0);

	return (void *)(intptr_t)sock;
}

static int
create_bound_socket()
{
	int sock = socket(PF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
	ATF_REQUIRE(sock >= 0);

	int enable = 1;
	ATF_REQUIRE(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, /**/
			&enable, sizeof(int)) == 0);

	struct sockaddr_in addr = { 0 };
	addr.sin_family = AF_INET;
	addr.sin_port = htons(1337);
	ATF_REQUIRE(inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) == 1);

	ATF_REQUIRE(bind(sock, /**/
			(struct sockaddr const *)&addr, sizeof(addr)) == 0);

	return sock;
}

static void
fd_tcp_socket(int fds[3])
{
	int sock = create_bound_socket();

	ATF_REQUIRE(listen(sock, 5) == 0);

	pthread_t client_thread;
	ATF_REQUIRE(
	    pthread_create(&client_thread, NULL, connector_client, NULL) == 0);

#ifdef __APPLE__
	int conn = accept(sock, NULL, NULL);
#else
	int conn = accept4(sock, NULL, NULL, SOCK_CLOEXEC);
#endif
	ATF_REQUIRE(conn >= 0);

	void *client_socket = NULL;

	ATF_REQUIRE(pthread_join(client_thread, &client_socket) == 0);

	fds[0] = conn;
	fds[1] = (int)(intptr_t)client_socket;
	fds[2] = sock;
}

ATF_TC_WITHOUT_HEAD(epoll__simple);
ATF_TC_BODY_FD_LEAKCHECK(epoll__simple, tc)
{
	int fd;

	ATF_REQUIRE_MSG((fd = epoll_create1(EPOLL_CLOEXEC)) >= 0,
	    "errno: %d/%s", errno, strerror(errno));
	ATF_REQUIRE(close(fd) == 0);

	ATF_REQUIRE_ERRNO(EINVAL, epoll_create(0) < 0);

	ATF_REQUIRE(epoll_create(1) >= 0);
	ATF_REQUIRE(close(fd) == 0);

	ATF_REQUIRE_ERRNO(EINVAL, epoll_create1(42) < 0);
}

ATF_TC_WITHOUT_HEAD(epoll__poll_flags);
ATF_TC_BODY_FD_LEAKCHECK(epoll__poll_flags, tc)
{
	ATF_REQUIRE(POLLIN == EPOLLIN);
	ATF_REQUIRE(POLLPRI == EPOLLPRI);
	ATF_REQUIRE(POLLOUT == EPOLLOUT);
	ATF_REQUIRE(POLLERR == EPOLLERR);
	ATF_REQUIRE(POLLHUP == EPOLLHUP);
#ifdef EPOLLNVAL
	ATF_REQUIRE(POLLNVAL == EPOLLNVAL);
#endif
#ifdef POLLRDHUP
	ATF_REQUIRE(POLLRDHUP == EPOLLRDHUP);
#endif
}

ATF_TC_WITHOUT_HEAD(epoll__leakcheck);
ATF_TC_BODY_FD_LEAKCHECK(epoll__leakcheck, tc)
{
	int fd;

	ATF_REQUIRE((fd = epoll_create1(EPOLL_CLOEXEC)) >= 0);

	atf_tc_expect_fail("Test that the leak check works");
}

ATF_TC_WITHOUT_HEAD(epoll__fd_exhaustion);
ATF_TC_BODY_FD_LEAKCHECK(epoll__fd_exhaustion, tc)
{
	struct rlimit lim = { 512, 512 };
	ATF_REQUIRE(setrlimit(RLIMIT_NOFILE, &lim) == 0);

	size_t nr_fds = 1000;
	int *fds = malloc(nr_fds * sizeof(int));
	ATF_REQUIRE(fds != NULL);

	size_t i = 0;
	for (i = 0; i < nr_fds; ++i) {
		if ((fds[i] = epoll_create1(EPOLL_CLOEXEC)) >= 0) {
			continue;
		}

		ATF_REQUIRE(errno == EMFILE);
		break;
	}

	while (i > 0) {
		--i;
		ATF_REQUIRE(close(fds[i]) == 0);
	}
	free(fds);
}

ATF_TC_WITHOUT_HEAD(epoll__invalid_op);
ATF_TC_BODY_FD_LEAKCHECK(epoll__invalid_op, tc)
{
	int fd;
	int fd2;

	int const invalid_fd = 0xbeef;

	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = 0;

	ATF_REQUIRE((fd = epoll_create1(EPOLL_CLOEXEC)) >= 0);
	ATF_REQUIRE_ERRNO(EINVAL, epoll_ctl(fd, EPOLL_CTL_ADD, fd, &event) < 0);
	ATF_REQUIRE_ERRNO(EINVAL, epoll_ctl(fd, EPOLL_CTL_DEL, fd, &event) < 0);
	ATF_REQUIRE_ERRNO(EINVAL, epoll_ctl(fd, EPOLL_CTL_MOD, fd, &event) < 0);
	ATF_REQUIRE_ERRNO(EINVAL, /**/
	    epoll_ctl(fd, 42, fd, &event) < 0);
	ATF_REQUIRE_ERRNO(EFAULT, /**/
	    epoll_ctl(fd, EPOLL_CTL_ADD, fd, NULL) < 0);
	ATF_REQUIRE_ERRNO(EINVAL, /**/
	    epoll_ctl(fd, EPOLL_CTL_DEL, fd, NULL) < 0);
	ATF_REQUIRE_ERRNO(EFAULT, /**/
	    epoll_ctl(fd, EPOLL_CTL_MOD, fd, NULL) < 0);
	ATF_REQUIRE_ERRNO(EFAULT, /**/
	    epoll_ctl(fd, 42, fd, NULL) < 0);
	ATF_REQUIRE(close(fd) == 0);

	ATF_REQUIRE((fd = epoll_create1(EPOLL_CLOEXEC)) >= 0);
	ATF_REQUIRE_ERRNO(EBADF,
	    epoll_ctl(fd, EPOLL_CTL_ADD, invalid_fd, &event) < 0);
	ATF_REQUIRE_ERRNO(EBADF,
	    epoll_ctl(fd, EPOLL_CTL_DEL, invalid_fd, &event) < 0);
	ATF_REQUIRE_ERRNO(EBADF,
	    epoll_ctl(fd, EPOLL_CTL_MOD, invalid_fd, &event) < 0);
	ATF_REQUIRE_ERRNO(EBADF, /**/
	    epoll_ctl(fd, 42, invalid_fd, &event) < 0);
	ATF_REQUIRE_ERRNO(EFAULT,
	    epoll_ctl(fd, EPOLL_CTL_ADD, invalid_fd, NULL) < 0);
	ATF_REQUIRE_ERRNO(EBADF,
	    epoll_ctl(fd, EPOLL_CTL_DEL, invalid_fd, NULL) < 0);
	ATF_REQUIRE_ERRNO(EFAULT,
	    epoll_ctl(fd, EPOLL_CTL_MOD, invalid_fd, NULL) < 0);
	ATF_REQUIRE_ERRNO(EFAULT, /**/
	    epoll_ctl(fd, 42, invalid_fd, NULL) < 0);
	ATF_REQUIRE(close(fd) == 0);

	ATF_REQUIRE((fd = epoll_create1(EPOLL_CLOEXEC)) >= 0);
	ATF_REQUIRE((fd2 = epoll_create1(EPOLL_CLOEXEC)) >= 0);
	ATF_REQUIRE_ERRNO(ENOENT,
	    epoll_ctl(fd, EPOLL_CTL_DEL, fd2, &event) < 0);
	ATF_REQUIRE_ERRNO(ENOENT,
	    epoll_ctl(fd, EPOLL_CTL_MOD, fd2, &event) < 0);
	ATF_REQUIRE_ERRNO(EINVAL, /**/
	    epoll_ctl(fd, 42, fd2, &event) < 0);
	ATF_REQUIRE_ERRNO(EFAULT, /**/
	    epoll_ctl(fd, EPOLL_CTL_ADD, fd2, NULL) < 0);
	ATF_REQUIRE_ERRNO(ENOENT, /**/
	    epoll_ctl(fd, EPOLL_CTL_DEL, fd2, NULL) < 0);
	ATF_REQUIRE_ERRNO(EFAULT, /**/
	    epoll_ctl(fd, EPOLL_CTL_MOD, fd2, NULL) < 0);
	ATF_REQUIRE_ERRNO(EFAULT, /**/
	    epoll_ctl(fd, 42, fd2, NULL) < 0);

	{
		struct epoll_event ev = { .events = POLLIN };
		ATF_REQUIRE(epoll_ctl(fd, EPOLL_CTL_ADD, fd2, &ev) == 0);
	}
	{
		struct epoll_event ev = { .events = ~(uint32_t)0 };
		ATF_REQUIRE(epoll_ctl(fd, EPOLL_CTL_DEL, fd2, &ev) == 0);
	}
	ATF_REQUIRE(close(fd2) == 0);
	ATF_REQUIRE(close(fd) == 0);

	/**/

	ATF_REQUIRE_ERRNO(EFAULT, /**/
	    epoll_ctl(fd, EPOLL_CTL_ADD, fd2, NULL) < 0);
	ATF_REQUIRE_ERRNO(EBADF, epoll_ctl(fd, EPOLL_CTL_ADD, fd2, &event) < 0);

	struct epoll_event ev;
	ATF_REQUIRE_ERRNO(EINVAL, epoll_wait(fd, &ev, -1, 0) < 0);
	ATF_REQUIRE_ERRNO(EINVAL, epoll_wait(fd, &ev, 0, 0) < 0);
	ATF_REQUIRE_ERRNO(EBADF, epoll_wait(fd, &ev, 1, 0) < 0);
}

ATF_TC_WITHOUT_HEAD(epoll__invalid_op2);
ATF_TC_BODY_FD_LEAKCHECK(epoll__invalid_op2, tc)
{
	int fd;
	ATF_REQUIRE((fd = epoll_create1(EPOLL_CLOEXEC)) >= 0);
	ATF_REQUIRE(close(fd) == 0);

	struct epoll_event *evs = malloc(
	    INT_MAX / sizeof(struct epoll_event) * sizeof(struct epoll_event));
	if (evs == NULL) {
		atf_tc_skip("could not alloc enough memory for test");
	}

	ATF_REQUIRE_ERRNO(EBADF,
	    epoll_wait(fd, evs, /**/
		INT_MAX / sizeof(struct epoll_event), 0) < 0);

	free(evs);

	ATF_REQUIRE_ERRNO(EINVAL,
	    epoll_wait(fd, evs, /**/
		INT_MAX / sizeof(struct epoll_event) + 1, 0) < 0);
}

ATF_TC_WITHOUT_HEAD(epoll__rdhup_linux);
ATF_TC_BODY_FD_LEAKCHECK(epoll__rdhup_linux, tcptr)
{
	if (EPOLLRDHUP == 0x2000) {
		return;
	}

	int fds[3];
	fd_tcp_socket(fds);

	int ep = epoll_create1(EPOLL_CLOEXEC);
	ATF_REQUIRE(ep >= 0);

	// It is not allowed to specify both 0x2000 and EPOLLRDHUP on
	// systems where 0x2000 and EPOLLRDHUP differ by value (for
	// example on FreeBSD). On those systems it is allowed to use 0x2000
	// instead of EPOLLRDHUP for Linux compatibility, but you have
	// to choose for each fd which value to use.
	struct epoll_event event = {
		.events = EPOLLIN | EPOLLRDHUP | 0x2000 | EPOLLPRI | EPOLLET,
	};
	ATF_REQUIRE_ERRNO(EINVAL, epoll_ctl(ep, EPOLL_CTL_ADD, fds[0], &event) < 0);

	ATF_REQUIRE(close(ep) == 0);
	ATF_REQUIRE(close(fds[0]) == 0);
	ATF_REQUIRE(close(fds[1]) == 0);
	ATF_REQUIRE(close(fds[2]) == 0);
}

ATF_TC_WITHOUT_HEAD(epoll__simple_wait);
ATF_TC_BODY_FD_LEAKCHECK(epoll__simple_wait, tc)
{
	int ep = epoll_create1(EPOLL_CLOEXEC);
	ATF_REQUIRE(ep >= 0);

	struct epoll_event event;

	ATF_REQUIRE(epoll_wait(ep, &event, 1, 1) == 0);
	ATF_REQUIRE(epoll_wait(ep, &event, 1, 0) == 0);

	ATF_REQUIRE(close(ep) == 0);
}

ATF_TC_WITHOUT_HEAD(epoll__event_size);
ATF_TC_BODY_FD_LEAKCHECK(epoll__event_size, tc)
{
	struct epoll_event event;
	// this check works on 32bit _and_ 64bit, since
	// sizeof(epoll_event) == sizeof(uint32_t) + sizeof(uint64_t)
	ATF_REQUIRE(sizeof(event) == 12);
}

ATF_TC_WITHOUT_HEAD(epoll__recursive_register);
ATF_TC_BODY_FD_LEAKCHECK(epoll__recursive_register, tcptr)
{
	int ep = epoll_create1(EPOLL_CLOEXEC);
	ATF_REQUIRE(ep >= 0);

	int ep_inner = epoll_create1(EPOLL_CLOEXEC);
	ATF_REQUIRE(ep_inner >= 0);

	{
		struct epoll_event event = { .events = EPOLLOUT };
		ATF_REQUIRE(epoll_ctl(ep, /**/
				EPOLL_CTL_ADD, ep_inner, &event) == 0);
	}
	{
		struct epoll_event event = { .events = EPOLLIN };
		ATF_REQUIRE(epoll_ctl(ep, /**/
				EPOLL_CTL_MOD, ep_inner, &event) == 0);
	}

	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_DEL, ep_inner, NULL) == 0);
	ATF_REQUIRE_ERRNO(ENOENT,
	    epoll_ctl(ep, EPOLL_CTL_DEL, ep_inner, NULL) < 0);

	ATF_REQUIRE(close(ep_inner) == 0);
	ATF_REQUIRE(close(ep) == 0);
}

static void
simple_epollin_impl(void (*fd_fun)(int fds[3]))
{
	int ep = epoll_create1(EPOLL_CLOEXEC);
	ATF_REQUIRE(ep >= 0);

	int fds[3];
	fd_fun(fds);

	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = fds[0];

	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, fds[0], &event) >= 0);

	uint8_t data = '\0';
	write(fds[1], &data, 1);

	struct epoll_event event_result;
	ATF_REQUIRE(epoll_wait(ep, &event_result, 1, -1) == 1);

	ATF_REQUIRE(event_result.data.fd == fds[0]);

	ATF_REQUIRE(close(fds[0]) == 0);
	ATF_REQUIRE(close(fds[1]) == 0);
	ATF_REQUIRE(fds[2] == -1 || close(fds[2]) == 0);
	ATF_REQUIRE(close(ep) == 0);
}

ATF_TC(epoll__simple_epollin);
ATF_TC_HEAD(epoll__simple_epollin, tc)
{
	atf_tc_set_md_var(tc, "X-ctest.properties", "RUN_SERIAL TRUE");
}
ATF_TC_BODY_FD_LEAKCHECK(epoll__simple_epollin, tc)
{
	simple_epollin_impl(fd_pipe);
	simple_epollin_impl(fd_domain_socket);
	simple_epollin_impl(fd_tcp_socket);
}

static void *
sleep_then_write(void *arg)
{
	usleep(100000);
	uint8_t data = '\0';
	write((int)(intptr_t)arg, &data, 1);
	return NULL;
}

static void
sleep_argument_impl(int sleep)
{
	int ep = epoll_create1(EPOLL_CLOEXEC);
	ATF_REQUIRE(ep >= 0);

	int fds[3];
	fd_pipe(fds);

	struct epoll_event event;
	event.events = EPOLLIN;

	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, fds[0], &event) >= 0);

	pthread_t writer_thread;
	ATF_REQUIRE(pthread_create(&writer_thread, NULL, sleep_then_write,
			(void *)(intptr_t)(fds[1])) == 0);

	ATF_REQUIRE(epoll_wait(ep, &event, 1, sleep) == 1);

	ATF_REQUIRE(pthread_join(writer_thread, NULL) == 0);

	ATF_REQUIRE(close(fds[0]) == 0);
	ATF_REQUIRE(close(fds[1]) == 0);
	ATF_REQUIRE(fds[2] == -1 || close(fds[2]) == 0);
	ATF_REQUIRE(close(ep) == 0);
}

ATF_TC_WITHOUT_HEAD(epoll__sleep_argument);
ATF_TC_BODY_FD_LEAKCHECK(epoll__sleep_argument, tc)
{
	sleep_argument_impl(-1);
	sleep_argument_impl(-2);
}

ATF_TC_WITHOUT_HEAD(epoll__remove_nonexistent);
ATF_TC_BODY_FD_LEAKCHECK(epoll__remove_nonexistent, tc)
{
	int ep = epoll_create1(EPOLL_CLOEXEC);
	ATF_REQUIRE(ep >= 0);

	int fds[3];
	fd_pipe(fds);

	ATF_REQUIRE_ERRNO(ENOENT,
	    epoll_ctl(ep, EPOLL_CTL_DEL, fds[0], NULL) < 0);

	ATF_REQUIRE(close(fds[0]) == 0);
	ATF_REQUIRE(close(fds[1]) == 0);
	ATF_REQUIRE(fds[2] == -1 || close(fds[2]) == 0);
	ATF_REQUIRE(close(ep) == 0);
}

ATF_TC_WITHOUT_HEAD(epoll__add_remove);
ATF_TC_BODY_FD_LEAKCHECK(epoll__add_remove, tc)
{
	int ep = epoll_create1(EPOLL_CLOEXEC);
	ATF_REQUIRE(ep >= 0);

	int fds[3];
	fd_pipe(fds);

	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = fds[0];

	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, fds[0], &event) == 0);
	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_DEL, fds[0], NULL) == 0);

	ATF_REQUIRE(close(fds[0]) == 0);
	ATF_REQUIRE(close(fds[1]) == 0);
	ATF_REQUIRE(fds[2] == -1 || close(fds[2]) == 0);
	ATF_REQUIRE(close(ep) == 0);
}

static void
add_existing_impl(bool change_udata)
{
	int ep = epoll_create1(EPOLL_CLOEXEC);
	ATF_REQUIRE(ep >= 0);

	int fds[3];
	fd_pipe(fds);

	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.u32 = 42;

	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, fds[0], &event) == 0);

	if (change_udata) {
		event.data.u32 = 43;
	}

	ATF_REQUIRE_ERRNO(EEXIST,
	    epoll_ctl(ep, EPOLL_CTL_ADD, fds[0], &event) < 0);
	ATF_REQUIRE_ERRNO(EEXIST,
	    epoll_ctl(ep, EPOLL_CTL_ADD, fds[0], &event) < 0);

	uint8_t data = '\0';
	write(fds[1], &data, 1);

	struct epoll_event event_result;
	ATF_REQUIRE(epoll_wait(ep, &event_result, 1, -1) == 1);

	ATF_REQUIRE(event_result.data.u32 == 42);

	ATF_REQUIRE(close(fds[0]) == 0);
	ATF_REQUIRE(close(fds[1]) == 0);
	ATF_REQUIRE(fds[2] == -1 || close(fds[2]) == 0);
	ATF_REQUIRE(close(ep) == 0);
}

ATF_TC_WITHOUT_HEAD(epoll__add_existing);
ATF_TC_BODY_FD_LEAKCHECK(epoll__add_existing, tc)
{
	add_existing_impl(true);
	add_existing_impl(false);
}

ATF_TC_WITHOUT_HEAD(epoll__modify_existing);
ATF_TC_BODY_FD_LEAKCHECK(epoll__modify_existing, tc)
{
	int ep = epoll_create1(EPOLL_CLOEXEC);
	ATF_REQUIRE(ep >= 0);

	int fds[3];
	fd_pipe(fds);

	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = fds[0];

	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, fds[0], &event) == 0);

	event.events = 0;
	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_MOD, fds[0], &event) == 0);

	uint8_t data = '\0';
	write(fds[1], &data, 1);

	struct epoll_event event_result;
	ATF_REQUIRE(epoll_wait(ep, &event_result, 1, 50) == 0);

	event.events = EPOLLIN;
	event.data.fd = 42;
	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_MOD, fds[0], &event) == 0);

	ATF_REQUIRE(epoll_wait(ep, &event_result, 1, -1) == 1);
	ATF_REQUIRE(event_result.data.fd == 42);

	ATF_REQUIRE(close(fds[0]) == 0);
	ATF_REQUIRE(close(fds[1]) == 0);
	ATF_REQUIRE(fds[2] == -1 || close(fds[2]) == 0);
	ATF_REQUIRE(close(ep) == 0);
}

ATF_TC_WITHOUT_HEAD(epoll__modify_nonexisting);
ATF_TC_BODY_FD_LEAKCHECK(epoll__modify_nonexisting, tc)
{
	int ep = epoll_create1(EPOLL_CLOEXEC);
	ATF_REQUIRE(ep >= 0);

	int fds[3];
	fd_pipe(fds);

	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = fds[0];

	ATF_REQUIRE_ERRNO(ENOENT,
	    epoll_ctl(ep, EPOLL_CTL_MOD, fds[0], &event) < 0);

	ATF_REQUIRE(close(fds[0]) == 0);
	ATF_REQUIRE(close(fds[1]) == 0);
	ATF_REQUIRE(fds[2] == -1 || close(fds[2]) == 0);
	ATF_REQUIRE(close(ep) == 0);
}

static void *
poll_only_fd_thread_fun(void *arg)
{
	int ep = *(int *)arg;

	struct epoll_event event_result;
	ATF_REQUIRE(epoll_wait(ep, &event_result, 1, -1) == 1);

	return NULL;
}

ATF_TC_WITHOUT_HEAD(epoll__poll_only_fd);
ATF_TC_BODY_FD_LEAKCHECK(epoll__poll_only_fd, tc)
{
#ifdef __linux__
	atf_tc_skip("Test hangs on Linux");
#elif defined(__APPLE__)
	atf_tc_skip("/dev/random not pollable under macOS");
#endif

	int ep = epoll_create1(EPOLL_CLOEXEC);
	ATF_REQUIRE(ep >= 0);

	int fd1 = open("/dev/random", O_RDONLY | O_CLOEXEC);
	int fd2 = open("/dev/random", O_RDONLY | O_CLOEXEC);
	if (fd1 < 0 || fd2 < 0) {
		atf_tc_skip("This test needs /dev/random");
	}

	struct epoll_event event = { .events = 0 };
	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, fd1, &event) == 0);
	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, fd2, &event) == 0);

	pthread_t threads[16];
	for (int i = 0; i < 16; ++i) {
		ATF_REQUIRE(pthread_create(&threads[i], NULL,
				&poll_only_fd_thread_fun, &ep) == 0);
	}

	/*
	 * Racy way of making sure that all threads are waiting in epoll_wait.
	 */
	usleep(200000);

	event.events = EPOLLIN | EPOLLRDHUP | EPOLLOUT;
	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_MOD, fd1, &event) == 0);

	for (int i = 0; i < 16; ++i) {
		ATF_REQUIRE(pthread_join(threads[i], NULL) == 0);
	}

	ATF_REQUIRE(close(fd1) == 0);

	struct epoll_event event_result;
	ATF_REQUIRE(epoll_wait(ep, &event_result, 1, 0) == 0);

	ATF_REQUIRE_ERRNO(EBADF, /**/
	    epoll_ctl(ep, EPOLL_CTL_DEL, fd1, NULL) < 0);

	ATF_REQUIRE(close(fd2) == 0);
	ATF_REQUIRE(close(ep) == 0);
}

static void
no_epollin_on_closed_empty_pipe_impl(bool do_write_data)
{
	int ep = epoll_create1(EPOLL_CLOEXEC);
	ATF_REQUIRE(ep >= 0);

	int fds[3];
	fd_pipe(fds);

	struct epoll_event event;
	event.events = EPOLLIN | EPOLLRDHUP;
	event.data.fd = fds[0];

	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, fds[0], &event) == 0);

	uint8_t data = '\0';
	if (do_write_data) {
		write(fds[1], &data, 1);
	}
	close(fds[1]);

	struct epoll_event event_result;
	ATF_REQUIRE(epoll_wait(ep, &event_result, 1, -1) == 1);

	ATF_REQUIRE_MSG(event_result.events ==
		(EPOLLHUP | (do_write_data ? EPOLLIN : 0)),
	    "%x", event_result.events);

	ATF_REQUIRE(read(fds[0], &data, 1) >= 0);

	ATF_REQUIRE(event_result.data.fd == fds[0]);

	ATF_REQUIRE(close(fds[0]) == 0);
	ATF_REQUIRE(close(ep) == 0);
}

ATF_TC_WITHOUT_HEAD(epoll__no_epollin_on_closed_empty_pipe);
ATF_TC_BODY_FD_LEAKCHECK(epoll__no_epollin_on_closed_empty_pipe, tcptr)
{
	no_epollin_on_closed_empty_pipe_impl(false);
	no_epollin_on_closed_empty_pipe_impl(true);
}

ATF_TC_WITHOUT_HEAD(epoll__write_to_pipe_until_full);
ATF_TC_BODY_FD_LEAKCHECK(epoll__write_to_pipe_until_full, tcptr)
{
	int ep = epoll_create1(EPOLL_CLOEXEC);
	ATF_REQUIRE(ep >= 0);

	int fds[3];
	fd_pipe(fds);

	ATF_REQUIRE(fcntl(fds[1], F_SETFL, O_NONBLOCK) == 0);

	struct epoll_event event;
	event.events = EPOLLOUT;
	event.data.fd = fds[1];

	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, fds[1], &event) == 0);

	struct epoll_event event_result;
	ATF_REQUIRE(epoll_wait(ep, &event_result, 1, -1) == 1);

	ATF_REQUIRE(event_result.data.fd == fds[1]);
	ATF_REQUIRE(event_result.events == EPOLLOUT);

	uint8_t data[512] = { 0 };
	while (write(fds[1], &data, sizeof(data)) >= 0) {
	}
	ATF_REQUIRE(errno == EAGAIN || errno == EWOULDBLOCK);

	ATF_REQUIRE(epoll_wait(ep, &event_result, 1, 300) == 0);

	event.events = EPOLLIN;
	event.data.fd = fds[0];
	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, fds[0], &event) == 0);

	ATF_REQUIRE(epoll_wait(ep, &event_result, 1, -1) == 1);

	ATF_REQUIRE(event_result.data.fd == fds[0]);
	ATF_REQUIRE(event_result.events == EPOLLIN);

	ATF_REQUIRE(close(fds[0]) == 0);
	ATF_REQUIRE(close(fds[1]) == 0);
	ATF_REQUIRE(fds[2] == -1 || close(fds[2]) == 0);
	ATF_REQUIRE(close(ep) == 0);
}

ATF_TC_WITHOUT_HEAD(epoll__realtime_timer);
ATF_TC_BODY_FD_LEAKCHECK(epoll__realtime_timer, tcptr)
{
	struct timespec now;
	ATF_REQUIRE(clock_gettime(CLOCK_REALTIME, &now) == 0);

	int fd = timerfd_create(CLOCK_REALTIME, 0);
	ATF_REQUIRE(fd >= 0);

	ATF_REQUIRE(timerfd_settime(fd, TFD_TIMER_ABSTIME,
			&(struct itimerspec) {
			    .it_value.tv_sec = now.tv_sec + 1,
			    .it_value.tv_nsec = now.tv_nsec,
			    .it_interval.tv_sec = 0,
			    .it_interval.tv_nsec = 100000000,
			},
			NULL) == 0);

	int ep = epoll_create1(EPOLL_CLOEXEC);
	ATF_REQUIRE(ep >= 0);

	struct epoll_event event;
	event.events = EPOLLIN | EPOLLOUT;
	event.data.fd = fd;

	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, fd, &event) == 0);

	struct epoll_event event_result;

	for (uint64_t tot_exp = 0; tot_exp < 3;) {
		ATF_REQUIRE(epoll_wait(ep, &event_result, 1, -1) == 1);

		ATF_REQUIRE(event_result.events == EPOLLIN);
		ATF_REQUIRE(event_result.data.fd == fd);

		uint64_t exp;
		ssize_t s = read(fd, &exp, sizeof(uint64_t));
		ATF_REQUIRE(s == sizeof(uint64_t));

		tot_exp += exp;
		printf("read: %llu; total=%llu\n", (unsigned long long)exp,
		    (unsigned long long)tot_exp);
	}

	ATF_REQUIRE(close(ep) == 0);
	ATF_REQUIRE(close(fd) == 0);
}

ATF_TC_WITHOUT_HEAD(epoll__simple_signalfd);
ATF_TC_BODY_FD_LEAKCHECK(epoll__simple_signalfd, tcptr)
{
	sigset_t mask;
	int sfd;
	struct signalfd_siginfo fdsi;
	ssize_t s;

	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);

	ATF_REQUIRE(sigprocmask(SIG_BLOCK, &mask, NULL) == 0);

	sfd = signalfd(-1, &mask, 0);
	ATF_REQUIRE(sfd >= 0);

	kill(getpid(), SIGINT);

	int ep = epoll_create1(EPOLL_CLOEXEC);
	ATF_REQUIRE(ep >= 0);

	struct epoll_event event;
	event.events = EPOLLIN | EPOLLOUT;
	event.data.fd = sfd;

	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, sfd, &event) == 0);

	struct epoll_event event_result;
	ATF_REQUIRE(epoll_wait(ep, &event_result, 1, -1) == 1);

	ATF_REQUIRE(event_result.events == EPOLLIN);
	ATF_REQUIRE(event_result.data.fd == sfd);

	s = read(sfd, &fdsi, sizeof(struct signalfd_siginfo));
	ATF_REQUIRE(s == sizeof(struct signalfd_siginfo));

	ATF_REQUIRE(fdsi.ssi_signo == SIGINT);

	ATF_REQUIRE(close(ep) == 0);
	ATF_REQUIRE(close(sfd) == 0);
}

ATF_TC_WITHOUT_HEAD(epoll__signalfd_poll_sigusr1);
ATF_TC_BODY_FD_LEAKCHECK(epoll__signalfd_poll_sigusr1, tcptr)
{
	/* This test is a simplified version of wayland's event_loop_signal() */
	sigset_t mask;
	int sfd;
	struct signalfd_siginfo fdsi;
	ssize_t s;

	int ep = epoll_create1(EPOLL_CLOEXEC);
	ATF_REQUIRE(ep >= 0);

	sigemptyset(&mask);
	sigaddset(&mask, SIGUSR1);
	sfd = signalfd(-1, &mask, SFD_CLOEXEC | SFD_NONBLOCK);
	ATF_REQUIRE(sfd >= 0);
	ATF_REQUIRE(sigprocmask(SIG_BLOCK, &mask, NULL) == 0);

	struct signal_event_source {
		int fd;
		int signum;
		void *data;
	} event_data = {
		.fd = sfd,
		.signum = SIGUSR1,
		.data = (void *)(intptr_t)0x12345678,
	};

	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.ptr = &event_data;
	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, sfd, &event) == 0);

	struct epoll_event event_results[32];

	for (int i = 0; i < 100; ++i) {
		ATF_REQUIRE(epoll_wait(ep, event_results, 32, 0) == 0);

		ATF_REQUIRE(kill(getpid(), SIGUSR1) == 0);

		int r = epoll_wait(ep, event_results, 32, i == 0 ? -1 : 0);
		if (i != 0 && r == 0) {
			atf_tc_skip("EVFILT_SIGNAL might not immediately "
				    "trigger after the kill(2)");
		}
		ATF_REQUIRE(r == 1);
		ATF_REQUIRE(event_results[0].events == EPOLLIN);
		ATF_REQUIRE(event_results[0].data.ptr == &event_data);
		struct signal_event_source *event_data_ptr =
		    event_results[0].data.ptr;
		ATF_REQUIRE(event_data_ptr->fd == sfd);

		s = read(sfd, &fdsi, sizeof(struct signalfd_siginfo));
		ATF_REQUIRE(s == sizeof(struct signalfd_siginfo));

		ATF_REQUIRE(fdsi.ssi_signo == SIGUSR1);
	}

	ATF_REQUIRE(close(ep) == 0);
	ATF_REQUIRE(close(sfd) == 0);
}

static void *
signalfd_thread(void *arg)
{
	int ep = *(int *)arg;

	struct epoll_event event_result;
	ATF_REQUIRE(epoll_wait(ep, &event_result, 1, 0) == 0);

	return NULL;
}

static void
signalfd_in_thread_test(int which)
{
	sigset_t mask;
	struct signalfd_siginfo fdsi;
	ssize_t s;

	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);

	ATF_REQUIRE(sigprocmask(SIG_BLOCK, &mask, NULL) == 0);

	int sfd = signalfd(-1, &mask, 0);
	ATF_REQUIRE(sfd >= 0);

	int ep = epoll_create1(EPOLL_CLOEXEC);
	ATF_REQUIRE(ep >= 0);

	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = sfd;

	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, sfd, &event) == 0);

	struct epoll_event event_result;
	ATF_REQUIRE(epoll_wait(ep, &event_result, 1, 0) == 0);

	ATF_REQUIRE(pthread_kill(pthread_self(), SIGINT) == 0);

	{
		struct pollfd pfd = { .fd = ep, .events = POLLIN };
#if defined(__DragonFly__) || defined(__APPLE__)
		ATF_REQUIRE(poll(&pfd, 1, 0) == 0);
		atf_tc_skip("signals sent to threads won't trigger "
			    "EVFILT_SIGNAL on DragonFly/macOS");
#endif
		ATF_REQUIRE(poll(&pfd, 1, -1) == 1);
		ATF_REQUIRE(pfd.revents == POLLIN);
	}

	pthread_t thread;
	ATF_REQUIRE(pthread_create(&thread, NULL, signalfd_thread, &ep) == 0);
	ATF_REQUIRE(pthread_join(thread, NULL) == 0);

	if (which) {
		struct pollfd pfd = { .fd = sfd, .events = POLLIN };
		ATF_REQUIRE(poll(&pfd, 1, -1) == 1);
		ATF_REQUIRE(pfd.revents == POLLIN);
	} else {
		struct pollfd pfd = { .fd = ep, .events = POLLIN };
		ATF_REQUIRE(poll(&pfd, 1, 0) == 0);

		ATF_REQUIRE(epoll_wait(ep, &event_result, 1, 0) == 0);

		ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_DEL, sfd, NULL) == 0);
		ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, sfd, &event) == 0);

		ATF_REQUIRE(epoll_wait(ep, &event_result, 1, -1) == 1);
		ATF_REQUIRE(event_result.events == EPOLLIN);
		ATF_REQUIRE(event_result.data.fd == sfd);
	}

	s = read(sfd, &fdsi, sizeof(struct signalfd_siginfo));
	ATF_REQUIRE(s == sizeof(struct signalfd_siginfo));

	ATF_REQUIRE(sigprocmask(SIG_UNBLOCK, &mask, NULL) == 0);

	ATF_REQUIRE(close(ep) == 0);
	ATF_REQUIRE(close(sfd) == 0);
}

ATF_TC_WITHOUT_HEAD(epoll__signalfd_in_thread);
ATF_TC_BODY_FD_LEAKCHECK(epoll__signalfd_in_thread, tcptr)
{
	signalfd_in_thread_test(0);
	signalfd_in_thread_test(1);
}

static void
socket_shutdown_impl(bool specify_rdhup)
{
	int ep = epoll_create1(EPOLL_CLOEXEC);
	ATF_REQUIRE(ep >= 0);

	int fds[3];
	fd_tcp_socket(fds);

	uint32_t rdhup_flag = specify_rdhup ? EPOLLRDHUP : 0;

	struct epoll_event event;
	event.events = EPOLLOUT | EPOLLIN | (specify_rdhup ? 0 : EPOLLRDHUP);
	event.data.fd = fds[0];

	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, fds[0], &event) == 0);

	event.events = EPOLLOUT | EPOLLIN | rdhup_flag;
	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_MOD, fds[0], &event) == 0);

	ATF_REQUIRE(shutdown(fds[1], SHUT_WR) == 0);

	for (;;) {
		ATF_REQUIRE(epoll_wait(ep, &event, 1, -1) == 1);

		fprintf(stderr, "got event: %x\n", (int)event.events);

		if (event.events == EPOLLOUT) {
			/* skip spurious event generated by EVFILT_WRITE */
			/* TODO(jan): find a better solution */
			continue;
		}

		if (event.events == (EPOLLOUT | EPOLLIN | rdhup_flag)) {
			uint8_t buf;
			ssize_t ret = read(fds[0], &buf, 1);
			ATF_REQUIRE(ret == 0);

			ATF_REQUIRE(shutdown(fds[0], SHUT_WR) == 0);
			shutdown(fds[0], SHUT_RDWR);
		} else if (event.events ==
		    (EPOLLOUT | EPOLLIN | rdhup_flag | EPOLLHUP)) {
			/* close() may fail here! Don't check return code. */
			close(fds[0]);
			break;
#ifdef __NetBSD__
		} else if (event.events == (EPOLLIN | rdhup_flag)) {
			continue;
#endif
		} else {
			ATF_REQUIRE(false);
		}
	}

	ATF_REQUIRE(epoll_wait(ep, &event, 1, 300) == 0);

	ATF_REQUIRE(close(fds[1]) == 0);
	ATF_REQUIRE(fds[2] == -1 || close(fds[2]) == 0);
	ATF_REQUIRE(close(ep) == 0);
}

ATF_TC(epoll__socket_shutdown);
ATF_TC_HEAD(epoll__socket_shutdown, tc)
{
	atf_tc_set_md_var(tc, "X-ctest.properties", "RUN_SERIAL TRUE");
}
ATF_TC_BODY_FD_LEAKCHECK(epoll__socket_shutdown, tcptr)
{
	socket_shutdown_impl(true);
	socket_shutdown_impl(false);
}

ATF_TC_WITHOUT_HEAD(epoll__epollhup_on_fresh_socket);
ATF_TC_BODY_FD_LEAKCHECK(epoll__epollhup_on_fresh_socket, tcptr)
{
	int sock = socket(PF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
	ATF_REQUIRE(sock >= 0);

	int ep = epoll_create1(EPOLL_CLOEXEC);
	ATF_REQUIRE(ep >= 0);

	struct epoll_event event;
	event.events = EPOLLIN | EPOLLRDHUP | EPOLLOUT;
	event.data.fd = sock;

	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, sock, &event) == 0);

	for (int i = 0; i < 3; ++i) {
		int ret = epoll_wait(ep, &event, 1, 1000);
		if (ret == 0) {
			atf_tc_skip("BSD's don't return POLLHUP on "
				    "not yet connected sockets");
		}
		ATF_REQUIRE(ret == 1);

		ATF_REQUIRE(event.events == (EPOLLOUT | EPOLLHUP));

		usleep(100000);
	}

	ATF_REQUIRE(close(ep) == 0);
	ATF_REQUIRE(close(sock) == 0);
}

ATF_TC(epoll__epollout_on_connecting_socket);
ATF_TC_HEAD(epoll__epollout_on_connecting_socket, tc)
{
	atf_tc_set_md_var(tc, "X-ctest.properties", "RUN_SERIAL TRUE");
}
ATF_TC_BODY_FD_LEAKCHECK(epoll__epollout_on_connecting_socket, tcptr)
{
	int ep = epoll_create1(EPOLL_CLOEXEC);
	ATF_REQUIRE(ep >= 0);

	for (;;) {
		bool success = false;

		int sock = socket(PF_INET, /**/
		    SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK, 0);
		ATF_REQUIRE(sock >= 0);

		struct epoll_event event = { .events = EPOLLIN | EPOLLRDHUP |
			    EPOLLOUT };
		ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, sock, &event) == 0);

		int server_sock = create_bound_socket();
		ATF_REQUIRE(server_sock >= 0);

		{
			struct sockaddr_in addr = { 0 };
			addr.sin_family = AF_INET;
			addr.sin_port = htons(1337);
			ATF_REQUIRE(inet_pton(AF_INET, "127.0.0.1", /**/
					&addr.sin_addr) == 1);

			ATF_REQUIRE(
			    connect(sock, (struct sockaddr const *)&addr,
				sizeof(addr)) < 0);
			if (errno == ECONNREFUSED) {
				goto next;
			}
			ATF_REQUIRE(errno == EINPROGRESS);
		}

		usleep(100000);
		ATF_REQUIRE(close(server_sock) == 0);

		for (int i = 0; i < 3; ++i) {
			ATF_REQUIRE(epoll_wait(ep, &event, 1, -1) == 1);

			ATF_REQUIRE_MSG(event.events ==
				(EPOLLIN | EPOLLRDHUP | EPOLLOUT | /**/
				    EPOLLERR | EPOLLHUP),
			    "%04x", event.events);

			usleep(100000);
		}

		success = true;

	next:
		if (!success) {
			ATF_REQUIRE(close(server_sock) == 0);
		}
		ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_DEL, sock, NULL) == 0);
		ATF_REQUIRE(close(sock) == 0);

		if (success) {
			break;
		}
	}

	ATF_REQUIRE(close(ep) == 0);
}

static void *
epollpri_thread_fun(void *arg)
{
	int ep = *(int *)arg;

	struct epoll_event event_result;
	int r = epoll_wait(ep, &event_result, 1, 1000);
	if (r == 0) {
		atf_tc_skip("OOB data not efficiently supported without using "
			    "SO_OOBINLINE or EVFILT_EXCEPT");
	}
	ATF_REQUIRE(r == 1);

	return NULL;
}

ATF_TC(epoll__epollpri);
ATF_TC_HEAD(epoll__epollpri, tc)
{
	atf_tc_set_md_var(tc, "X-ctest.properties", "RUN_SERIAL TRUE");
}
ATF_TC_BODY_FD_LEAKCHECK(epoll__epollpri, tcptr)
{
	int fds[3];
	fd_tcp_socket(fds);

	ATF_REQUIRE(fcntl(fds[0], F_SETFL, O_NONBLOCK) == 0);
	ATF_REQUIRE(fcntl(fds[1], F_SETFL, O_NONBLOCK) == 0);

	int ep = epoll_create1(EPOLL_CLOEXEC);
	ATF_REQUIRE(ep >= 0);

	struct epoll_event event = {
		.events = EPOLLIN | EPOLLRDHUP | EPOLLPRI | EPOLLET,
	};
	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, fds[0], &event) == 0);

	char c = 'o';
	ATF_REQUIRE(send(fds[1], &c, 1, MSG_OOB) == 1);
	c = 'n';
	ATF_REQUIRE(send(fds[1], &c, 1, 0) == 1);

	for (;;) {
		ATF_REQUIRE(epoll_wait(ep, &event, 1, -1) == 1);
		ATF_REQUIRE(event.events == (EPOLLIN | EPOLLPRI) ||
		    event.events == EPOLLPRI);
		if (event.events == EPOLLPRI) {
			continue;
		}
		break;
	}
	{
		int r = epoll_wait(ep, &event, 1, 0);
		if (r == 1) {
			/* This can happen if the first event was triggered
			 * only by a EVFILT_EXCEPT and the EVFILT_READ has not
			 * arrived yet. */
			ATF_REQUIRE(event.events == (EPOLLIN | EPOLLPRI));
		} else {
			ATF_REQUIRE(r == 0);
		}
	}

	ATF_REQUIRE(recv(fds[0], &c, 1, MSG_OOB) == 1);
	ATF_REQUIRE(recv(fds[0], &c, 1, MSG_OOB) < 0);
	ATF_REQUIRE(c == 'o');
	ATF_REQUIRE(recv(fds[0], &c, 1, 0) == 1);
	ATF_REQUIRE(recv(fds[0], &c, 1, 0) < 0);
	ATF_REQUIRE(c == 'n');

	ATF_REQUIRE(epoll_wait(ep, &event, 1, 0) == 0);

	event.events = EPOLLIN | EPOLLRDHUP | EPOLLET;
	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_MOD, fds[0], &event) == 0);

	ATF_REQUIRE(epoll_wait(ep, &event, 1, 0) == 0);

	pthread_t thread;
	ATF_REQUIRE(pthread_create(&thread, /**/
			NULL, &epollpri_thread_fun, &ep) == 0);

	usleep(200000);

	event.events = EPOLLIN | EPOLLRDHUP | EPOLLPRI | EPOLLET;
	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_MOD, fds[0], &event) == 0);

	c = 'o';
	ATF_REQUIRE(send(fds[1], &c, 1, MSG_OOB) == 1);

	ATF_REQUIRE(pthread_join(thread, NULL) == 0);

	c = 'n';
	ATF_REQUIRE(send(fds[1], &c, 1, 0) == 1);

	ATF_REQUIRE(recv(fds[0], &c, 1, MSG_OOB) == 1);
	ATF_REQUIRE(recv(fds[0], &c, 1, MSG_OOB) < 0);
	while (recv(fds[0], &c, 1, 0) != 1) {
	}
	ATF_REQUIRE(recv(fds[0], &c, 1, 0) < 0);
	ATF_REQUIRE(errno == EAGAIN || errno == EWOULDBLOCK);

	c = 'n';
	ATF_REQUIRE(send(fds[1], &c, 1, 0) == 1);

	ATF_REQUIRE(epoll_wait(ep, &event, 1, -1) == 1);
	ATF_REQUIRE_MSG(event.events == EPOLLIN, "%04x", event.events);

	ATF_REQUIRE(close(ep) == 0);
	ATF_REQUIRE(close(fds[0]) == 0);
	ATF_REQUIRE(close(fds[1]) == 0);
	ATF_REQUIRE(close(fds[2]) == 0);
}

ATF_TC(epoll__epollpri_oobinline);
ATF_TC_HEAD(epoll__epollpri_oobinline, tc)
{
	atf_tc_set_md_var(tc, "X-ctest.properties", "RUN_SERIAL TRUE");
}
ATF_TC_BODY_FD_LEAKCHECK(epoll__epollpri_oobinline, tcptr)
{
	int fds[3];
	fd_tcp_socket(fds);

	ATF_REQUIRE(fcntl(fds[0], F_SETFL, O_NONBLOCK) == 0);
	ATF_REQUIRE(fcntl(fds[1], F_SETFL, O_NONBLOCK) == 0);

	{
		int enable = 1;
		setsockopt(fds[0], SOL_SOCKET, SO_OOBINLINE, /**/
		    &enable, sizeof(enable));
		setsockopt(fds[1], SOL_SOCKET, SO_OOBINLINE, /**/
		    &enable, sizeof(enable));
	}

	int ep = epoll_create1(EPOLL_CLOEXEC);
	ATF_REQUIRE(ep >= 0);

	struct epoll_event event = {
		.events = EPOLLIN | EPOLLRDHUP | EPOLLPRI | EPOLLET,
	};
	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, fds[0], &event) == 0);

	char c = 'o';
	ATF_REQUIRE(send(fds[1], &c, 1, MSG_OOB) == 1);

	usleep(200000);
	ATF_REQUIRE(epoll_wait(ep, &event, 1, -1) == 1);
	ATF_REQUIRE(event.events == (EPOLLIN | EPOLLPRI));
	ATF_REQUIRE(epoll_wait(ep, &event, 1, 0) == 0);

	c = 'n';
	ATF_REQUIRE(send(fds[1], &c, 1, 0) == 1);

	ATF_REQUIRE(recv(fds[0], &c, 1, 0) == 1);
	ATF_REQUIRE(c == 'o');

	ATF_REQUIRE(epoll_wait(ep, &event, 1, -1) == 1);
	ATF_REQUIRE(event.events == EPOLLIN);
	ATF_REQUIRE(epoll_wait(ep, &event, 1, 0) == 0);

	ATF_REQUIRE(recv(fds[0], &c, 1, 0) == 1);
	ATF_REQUIRE(c == 'n');

	ATF_REQUIRE(recv(fds[0], &c, 1, 0) < 0);
	ATF_REQUIRE(errno == EAGAIN || errno == EWOULDBLOCK);

	ATF_REQUIRE(epoll_wait(ep, &event, 1, 0) == 0);

	ATF_REQUIRE(close(ep) == 0);
	ATF_REQUIRE(close(fds[0]) == 0);
	ATF_REQUIRE(close(fds[1]) == 0);
	ATF_REQUIRE(close(fds[2]) == 0);
}

ATF_TC(epoll__epollpri_oobinline_lt);
ATF_TC_HEAD(epoll__epollpri_oobinline_lt, tc)
{
	atf_tc_set_md_var(tc, "X-ctest.properties", "RUN_SERIAL TRUE");
}
ATF_TC_BODY_FD_LEAKCHECK(epoll__epollpri_oobinline_lt, tcptr)
{
	int fds[3];
	fd_tcp_socket(fds);

	ATF_REQUIRE(fcntl(fds[0], F_SETFL, O_NONBLOCK) == 0);
	ATF_REQUIRE(fcntl(fds[1], F_SETFL, O_NONBLOCK) == 0);

	{
		int enable = 1;
		setsockopt(fds[0], SOL_SOCKET, SO_OOBINLINE, /**/
		    &enable, sizeof(enable));
		setsockopt(fds[1], SOL_SOCKET, SO_OOBINLINE, /**/
		    &enable, sizeof(enable));
	}

	int ep = epoll_create1(EPOLL_CLOEXEC);
	ATF_REQUIRE(ep >= 0);

	struct epoll_event event = {
		.events = EPOLLPRI,
	};
	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, fds[0], &event) == 0);

	char c = 'o';
	ATF_REQUIRE(send(fds[1], &c, 1, MSG_OOB) == 1);

	ATF_REQUIRE(epoll_wait(ep, &event, 1, -1) == 1);
	ATF_REQUIRE(event.events == EPOLLPRI);
	ATF_REQUIRE(epoll_wait(ep, &event, 1, -1) == 1);
	ATF_REQUIRE(event.events == EPOLLPRI);
	ATF_REQUIRE(epoll_wait(ep, &event, 1, -1) == 1);
	ATF_REQUIRE(event.events == EPOLLPRI);

	ATF_REQUIRE(recv(fds[0], &c, 1, 0) == 1);
	ATF_REQUIRE(c == 'o');
	ATF_REQUIRE(recv(fds[0], &c, 1, 0) < 0);
	ATF_REQUIRE(errno == EAGAIN || errno == EWOULDBLOCK);

	ATF_REQUIRE(epoll_wait(ep, &event, 1, 0) == 0);

	c = 'n';
	ATF_REQUIRE(send(fds[1], &c, 1, 0) == 1);
	usleep(200000);
	int n;
	ATF_REQUIRE_MSG((n = epoll_wait(ep, &event, 1, 0)) == 0,
	    "%d 0x%x errno: %d", n, event.events, errno);

	ATF_REQUIRE(close(ep) == 0);
	ATF_REQUIRE(close(fds[0]) == 0);
	ATF_REQUIRE(close(fds[1]) == 0);
	ATF_REQUIRE(close(fds[2]) == 0);
}

ATF_TC(epoll__timeout_on_listening_socket);
ATF_TC_HEAD(epoll__timeout_on_listening_socket, tc)
{
	atf_tc_set_md_var(tc, "X-ctest.properties", "RUN_SERIAL TRUE");
}
ATF_TC_BODY_FD_LEAKCHECK(epoll__timeout_on_listening_socket, tcptr)
{
	int sock = socket(PF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
	ATF_REQUIRE(sock >= 0);

	int enable = 1;
	ATF_REQUIRE(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, /**/
			&enable, sizeof(int)) == 0);

	struct sockaddr_in addr = { 0 };
	addr.sin_family = AF_INET;
	addr.sin_port = htons(1337);
	ATF_REQUIRE(inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) == 1);

	ATF_REQUIRE(bind(sock, /**/
			(struct sockaddr const *)&addr, sizeof(addr)) == 0);

	ATF_REQUIRE(listen(sock, 5) == 0);

	int ep = epoll_create1(EPOLL_CLOEXEC);
	ATF_REQUIRE(ep >= 0);

	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = sock;

	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, sock, &event) == 0);

	int ret;

	for (int i = 0; i < 3; ++i) {
		ret = epoll_wait(ep, &event, 1, 100);
		ATF_REQUIRE(ret == 0);

		usleep(100000);
	}

	ATF_REQUIRE(close(ep) == 0);
	ATF_REQUIRE(close(sock) == 0);
}

ATF_TC_WITHOUT_HEAD(epoll__epollerr_on_closed_pipe);
ATF_TC_BODY_FD_LEAKCHECK(epoll__epollerr_on_closed_pipe, tcptr)
{
	int ep = epoll_create1(EPOLL_CLOEXEC);
	ATF_REQUIRE(ep >= 0);

	int fds[3];
	fd_pipe(fds);

	struct epoll_event event;
	event.events = EPOLLOUT;
	event.data.fd = fds[1];

	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, fds[1], &event) == 0);

	for (;;) {
		struct epoll_event event_result;
		ATF_REQUIRE(epoll_wait(ep, &event_result, 1, -1) == 1);

		ATF_REQUIRE(event_result.data.fd == fds[1]);

		if (event_result.events == EPOLLOUT) {
			// continue
		} else if (event_result.events == (EPOLLOUT | EPOLLERR)) {
			break;
#ifndef __linux__
		} else if (event_result.events == EPOLLERR) {
			/* kqueue based emulation may
			 * return just POLLERR here */
			break;
#endif
		} else {
			ATF_REQUIRE(false);
		}

		uint8_t data[512] = { 0 };
		write(fds[1], &data, sizeof(data));

		ATF_REQUIRE(close(fds[0]) == 0);
	}

	ATF_REQUIRE(close(fds[1]) == 0);
	ATF_REQUIRE(fds[2] == -1 || close(fds[2]) == 0);
	ATF_REQUIRE(close(ep) == 0);
}

static void
shutdown_behavior_impl(void (*fd_fun)(int fds[3]))
{
	int ep = epoll_create1(EPOLL_CLOEXEC);
	ATF_REQUIRE(ep >= 0);

	connector_epfd = ep;

	int fds[3];
	fd_fun(fds);

	connector_epfd = -1;

	int counter = 0;
	char c = 42;
	write(fds[0], &c, 1);

	struct epoll_event event;
	event.events = EPOLLOUT | EPOLLIN;
	event.data.fd = fds[1];
	epoll_ctl(ep, EPOLL_CTL_ADD, fds[1], &event);

	errno = 0;

	for (;;) {
		struct epoll_event event_result;
		int n;
		ATF_REQUIRE((n = epoll_wait(ep, &event_result, 1, -1)) == 1);

		ATF_REQUIRE(event_result.data.fd == fds[1]);

		// fprintf(stderr, "got event: %x %d\n",
		// (int)event_result.events,
		//     (int)event_result.events);

		if (event_result.events & EPOLLIN) {
			ATF_REQUIRE((n = (int)read(fds[1], &c, 1)) == 1);

			++counter;

			if (counter <= 5) {
				send(fds[0], &c, 1, MSG_NOSIGNAL);
			} else if (counter == 6) {
				send(fds[0], &c, 1, MSG_NOSIGNAL);
				shutdown(fds[0], SHUT_WR);

				usleep(100000);
			} else {
				uint8_t data[512] = { 0 };
				send(fds[1], &data, sizeof(data), MSG_NOSIGNAL);

				close(fds[0]);

				event.events = EPOLLOUT;
				event.data.fd = fds[1];
				ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_MOD, /**/
						fds[1], &event) == 0);

				usleep(100000);
			}

		} else if (event_result.events == EPOLLOUT) {
			send(event.data.fd, &c, 1, MSG_NOSIGNAL);
			// continue
		} else if (fd_fun == fd_domain_socket &&
		    (event_result.events & (EPOLLOUT | EPOLLHUP)) ==
			(EPOLLOUT | EPOLLHUP)) {
			// TODO(jan): Linux sets EPOLLERR in addition
			{
				int error = 0;
				socklen_t errlen = sizeof(error);
				getsockopt(fds[1], SOL_SOCKET, SO_ERROR,
				    (void *)&error, &errlen);
				fprintf(stderr, "socket error: %d (%s)\n",
				    error, strerror(error));
			}
			break;
		} else if (fd_fun == fd_tcp_socket &&
		    event_result.events == (EPOLLOUT | EPOLLERR | EPOLLHUP)) {
			{
				int error = 0;
				socklen_t errlen = sizeof(error);
				getsockopt(fds[1], SOL_SOCKET, SO_ERROR,
				    (void *)&error, &errlen);
				fprintf(stderr, "socket error: %d (%s)\n",
				    error, strerror(error));
			}
			break;
		} else if (fd_fun == fd_tcp_socket &&
		    event_result.events == (EPOLLOUT | EPOLLHUP)) {
			/*
			 * Rarely, we get here (no EPOLLERR). But don't fail
			 * the test. There is some non-determinism involved...
			 */
			fprintf(stderr, "no socket error\n");
			break;
		} else {
			ATF_REQUIRE_MSG(false, "%p(%p/%p): events %x",
			    (void *)fd_fun, (void *)fd_domain_socket,
			    (void *)fd_tcp_socket, event_result.events);
		}
	}

	ATF_REQUIRE(close(fds[1]) == 0);
	ATF_REQUIRE(fds[2] == -1 || close(fds[2]) == 0);
	ATF_REQUIRE(close(ep) == 0);
}

ATF_TC(epoll__shutdown_behavior);
ATF_TC_HEAD(epoll__shutdown_behavior, tc)
{
	atf_tc_set_md_var(tc, "X-ctest.properties", "RUN_SERIAL TRUE");
}
ATF_TC_BODY_FD_LEAKCHECK(epoll__shutdown_behavior, tcptr)
{
	shutdown_behavior_impl(fd_tcp_socket);
	shutdown_behavior_impl(fd_domain_socket);
}

static void *
datagram_connector(void *arg)
{
	(void)arg;

	int sock = socket(PF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	ATF_REQUIRE(sock >= 0);

	struct sockaddr_in addr = { 0 };
	addr.sin_family = AF_INET;
	addr.sin_port = htons(1337);
	ATF_REQUIRE(inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) == 1);

	ATF_REQUIRE(
	    connect(sock, (struct sockaddr const *)&addr, sizeof(addr)) == 0);

	fprintf(stderr, "got client\n");

	uint8_t data = '\0';
	write(sock, &data, 0);
	usleep(500000);
	close(sock);

	return NULL;
}

ATF_TC(epoll__datagram_connection);
ATF_TC_HEAD(epoll__datagram_connection, tc)
{
	atf_tc_set_md_var(tc, "X-ctest.properties", "RUN_SERIAL TRUE");
}
ATF_TC_BODY_FD_LEAKCHECK(epoll__datagram_connection, tcptr)
{
	int ep = epoll_create1(EPOLL_CLOEXEC);
	ATF_REQUIRE(ep >= 0);

	int sock = socket(PF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	ATF_REQUIRE(sock >= 0);

	int enable = 1;
	ATF_REQUIRE(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, /**/
			&enable, sizeof(int)) == 0);

	struct sockaddr_in addr = { 0 };
	addr.sin_family = AF_INET;
	addr.sin_port = htons(1337);
	ATF_REQUIRE(inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) == 1);

	ATF_REQUIRE(bind(sock, /**/
			(struct sockaddr const *)&addr, sizeof(addr)) == 0);

	pthread_t client_thread;
	pthread_create(&client_thread, NULL, datagram_connector, NULL);

	int fds[2];
	fds[0] = sock;

	struct epoll_event event;
	event.events = EPOLLIN | EPOLLRDHUP;
	event.data.fd = fds[0];

	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, fds[0], &event) == 0);

	struct epoll_event event_result;
	ATF_REQUIRE(epoll_wait(ep, &event_result, 1, -1) == 1);

	fprintf(stderr, "got event: %x %d\n", (int)event_result.events,
	    (int)event_result.events);

	ATF_REQUIRE(event_result.events == EPOLLIN);

	uint8_t data = '\0';
	ATF_REQUIRE(read(fds[0], &data, 1) >= 0);

	ATF_REQUIRE(event_result.data.fd == fds[0]);

	pthread_join(client_thread, NULL);

	ATF_REQUIRE(close(fds[0]) == 0);
	ATF_REQUIRE(close(ep) == 0);
}

ATF_TC(epoll__epollout_on_own_shutdown);
ATF_TC_HEAD(epoll__epollout_on_own_shutdown, tc)
{
	atf_tc_set_md_var(tc, "X-ctest.properties", "RUN_SERIAL TRUE");
}
ATF_TC_BODY_FD_LEAKCHECK(epoll__epollout_on_own_shutdown, tcptr)
{
	int ep = epoll_create1(EPOLL_CLOEXEC);
	ATF_REQUIRE(ep >= 0);

	int fds[3];
	fd_tcp_socket(fds);

	struct epoll_event event;
	event.events = EPOLLOUT;
	event.data.fd = fds[0];

	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, fds[0], &event) == 0);

	shutdown(fds[0], SHUT_WR);
	usleep(100000);

	struct epoll_event event_result;
	ATF_REQUIRE(epoll_wait(ep, &event_result, 1, -1) == 1);

	ATF_REQUIRE(event_result.data.fd == fds[0]);

	fprintf(stderr, "got events: %x\n", (unsigned)event_result.events);

	ATF_REQUIRE(event_result.events == EPOLLOUT);

	ATF_REQUIRE(close(fds[0]) == 0);
	ATF_REQUIRE(close(fds[1]) == 0);
	ATF_REQUIRE(fds[2] == -1 || close(fds[2]) == 0);
	ATF_REQUIRE(close(ep) == 0);
}

ATF_TC_WITHOUT_HEAD(epoll__remove_closed);
ATF_TC_BODY_FD_LEAKCHECK(epoll__remove_closed, tcptr)
{
	int ep = epoll_create1(EPOLL_CLOEXEC);
	ATF_REQUIRE(ep >= 0);

	int fds[3];
	fd_pipe(fds);

	struct epoll_event event = { 0 };
	event.events = EPOLLIN;

	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, fds[0], &event) == 0);

	ATF_REQUIRE(close(fds[0]) == 0);
	ATF_REQUIRE(close(fds[1]) == 0);

	// Trying to delete an event that was already deleted by closing the
	// associated fd should fail.
	ATF_REQUIRE_ERRNO(EBADF,
	    epoll_ctl(ep, EPOLL_CTL_DEL, fds[0], &event) < 0);

	ATF_REQUIRE(close(ep) == 0);
}

ATF_TC_WITHOUT_HEAD(epoll__remove_closed_when_same_fd_open);
ATF_TC_BODY_FD_LEAKCHECK(epoll__remove_closed_when_same_fd_open, tcptr)
{
	int ep = epoll_create1(EPOLL_CLOEXEC);
	ATF_REQUIRE(ep >= 0);

	int fds[3];
	fd_pipe(fds);

	struct epoll_event event = { 0 };
	event.events = EPOLLIN;

	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, fds[0], &event) == 0);

	ATF_REQUIRE(close(fds[0]) == 0);
	ATF_REQUIRE(close(fds[1]) == 0);

	int p[2];
	ATF_REQUIRE(pipe2(p, O_CLOEXEC) == 0);
	ATF_REQUIRE(fds[0] == p[0]);
	ATF_REQUIRE(fds[1] == p[1]);

	// Trying to delete an event that was already deleted by closing the
	// associated fd should fail.
	ATF_REQUIRE_ERRNO(ENOENT,
	    epoll_ctl(ep, EPOLL_CTL_DEL, fds[0], &event) < 0);

	ATF_REQUIRE(close(p[0]) == 0);
	ATF_REQUIRE(close(p[1]) == 0);
	ATF_REQUIRE(close(ep) == 0);
}

ATF_TC_WITHOUT_HEAD(epoll__add_different_file_with_same_fd_value);
ATF_TC_BODY_FD_LEAKCHECK(epoll__add_different_file_with_same_fd_value, tcptr)
{
	int ep = epoll_create1(EPOLL_CLOEXEC);
	ATF_REQUIRE(ep >= 0);

	int fds[3];
	fd_pipe(fds);

	struct epoll_event event = { 0 };
	event.events = EPOLLIN;

	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, fds[0], &event) == 0);

	ATF_REQUIRE(close(fds[0]) == 0);
	ATF_REQUIRE(close(fds[1]) == 0);

	// Note: This wouldn't be needed under Linux as the close() calls above
	// properly removes the descriptor from the epoll instance. However, in
	// our epoll emulation we cannot (yet?) reliably detect if a descriptor
	// has been closed before it is deleted from the epoll instance.
	// See also: https://github.com/jiixyj/epoll-shim/pull/7
	ATF_REQUIRE_ERRNO(EBADF,
	    epoll_ctl(ep, EPOLL_CTL_DEL, fds[0], &event) < 0);

	// Creating new pipe. The file descriptors will have the same numerical
	// values as the previous ones.
	fd_pipe(fds);

	// If status of closed fds would not be cleared, adding an event with
	// the fd that has the same numerical value as the closed one would
	// fail.
	int ret;
	struct epoll_event event2 = { 0 };
	event2.events = EPOLLIN;
	ATF_REQUIRE((ret = epoll_ctl(ep, /**/
			 EPOLL_CTL_ADD, fds[0], &event2)) == 0);

	pthread_t writer_thread;
	pthread_create(&writer_thread, NULL, sleep_then_write,
	    (void *)(intptr_t)(fds[1]));

	ATF_REQUIRE((ret = epoll_wait(ep, &event, 1, 300)) == 1);

	pthread_join(writer_thread, NULL);

	ATF_REQUIRE(close(fds[0]) == 0);
	ATF_REQUIRE(close(fds[1]) == 0);
	ATF_REQUIRE(close(ep) == 0);
}

ATF_TC_WITHOUT_HEAD(epoll__invalid_writes);
ATF_TC_BODY_FD_LEAKCHECK(epoll__invalid_writes, tcptr)
{
	sigset_t mask;
	int fd;

	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);

	ATF_REQUIRE(sigprocmask(SIG_BLOCK, &mask, NULL) == 0);

	char dummy = 0;

	{
		fd = signalfd(-1, &mask, 0);
		ATF_REQUIRE(fd >= 0);
		ATF_REQUIRE(write(fd, &dummy, 1) < 0);
		/*
		 * FreeBSD's native write returns EOPNOTSUPP. write is not
		 * shimmed when using native eventfds.
		 *
		 * NetBSD's native write return ENXIO.
		 */
		ATF_REQUIRE_MSG(errno == EINVAL || errno == EOPNOTSUPP ||
			errno == ENXIO,
		    "%d", errno);
		ATF_REQUIRE(close(fd) == 0);
	}

	{
		fd = timerfd_create(CLOCK_MONOTONIC, 0);
		ATF_REQUIRE(fd >= 0);
		ATF_REQUIRE(write(fd, &dummy, 1) < 0);
		/*
		 * NetBSD's native write return EBADF.
		 */
		ATF_REQUIRE_MSG(errno == EINVAL || errno == EOPNOTSUPP ||
			errno == EBADF,
		    "%d", errno);
#ifndef __linux__
		ATF_REQUIRE(write(fd, &dummy, (size_t)SSIZE_MAX + 1) < 0);
		ATF_REQUIRE_MSG(errno == EINVAL || errno == EBADF, "%d", errno);
#endif
		ATF_REQUIRE(close(fd) == 0);
	}

	{
		fd = epoll_create1(EPOLL_CLOEXEC);
		ATF_REQUIRE(fd >= 0);
		ATF_REQUIRE(write(fd, &dummy, 1) < 0);
		ATF_REQUIRE_MSG(errno == EINVAL || errno == EOPNOTSUPP ||
			errno == ENXIO /* NetBSD */,
		    "%d", errno);
		ATF_REQUIRE_ERRNO(EINVAL, read(fd, &dummy, 1) < 0);
#ifndef __linux__
		ATF_REQUIRE_ERRNO(EINVAL,
		    read(fd, &dummy, (size_t)SSIZE_MAX + 1) < 0);
#endif
		ATF_REQUIRE(close(fd) == 0);
	}
}

ATF_TC_WITHOUT_HEAD(epoll__using_real_close);
ATF_TC_BODY_FD_LEAKCHECK(epoll__using_real_close, tcptr)
{
	int ep = epoll_create1(EPOLL_CLOEXEC);
	ATF_REQUIRE(ep >= 0);

	int fds[3];
	fd_pipe(fds);

	struct epoll_event event = { 0 };
	event.events = EPOLLIN;

	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, fds[0], &event) == 0);

	// This closes the underlying kqueue fd directly, without going through
	// our epoll_shim_close wrapper. It shouldn't blow up too badly.
	extern int real_close_for_test(int fd);
	ATF_REQUIRE(real_close_for_test(ep) == 0);

	ep = epoll_create1(EPOLL_CLOEXEC);
	ATF_REQUIRE(ep >= 0);

	ATF_REQUIRE(epoll_ctl(ep, EPOLL_CTL_ADD, fds[0], &event) == 0);

	ATF_REQUIRE(close(fds[0]) == 0);
	ATF_REQUIRE(close(fds[1]) == 0);
	ATF_REQUIRE(close(ep) == 0);
}

static sig_atomic_t volatile epoll_pwait_got_signal = 0;
static void
epoll_pwait_sighandler(int sig)
{
	epoll_pwait_got_signal = 1;
	(void)sig;
}

ATF_TC_WITHOUT_HEAD(epoll__epoll_pwait);
ATF_TC_BODY_FD_LEAKCHECK(epoll__epoll_pwait, tcptr)
{
	sigset_t emptyset;
	sigset_t blockset;

	sigemptyset(&blockset);
	sigaddset(&blockset, SIGINT);
	sigprocmask(SIG_BLOCK, &blockset, NULL);

	struct sigaction sa = { .sa_handler = epoll_pwait_sighandler };
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT, &sa, NULL);

	kill(getpid(), SIGINT);

	sigemptyset(&emptyset);

	int ep = epoll_create1(EPOLL_CLOEXEC);
	ATF_REQUIRE(ep >= 0);

	struct epoll_event ev;
	ATF_REQUIRE(epoll_pwait(ep, &ev, 1, 0, &emptyset) == 0);
	ATF_REQUIRE(epoll_pwait_got_signal == 0);
	ATF_REQUIRE_ERRNO(EINTR, epoll_pwait(ep, &ev, 1, 1000, &emptyset) < 0);
	ATF_REQUIRE(epoll_pwait_got_signal == 1);

	epoll_pwait_got_signal = 0;
	kill(getpid(), SIGINT);
	{
		struct pollfd pfd = { .fd = ep, .events = POLLIN };

		ATF_REQUIRE(poll(&pfd, 1, 0) == 0);
		ATF_REQUIRE(epoll_pwait_got_signal == 0);

#ifdef __NetBSD__
		int n = pollts(
#else
		int n = ppoll(
#endif
		    &pfd, 1,
		    &(struct timespec) {
			    0,
#if defined(__DragonFly__)
				1
#else
				0
#endif
		    },
		    &emptyset);
		ATF_REQUIRE(n == 0 || (n < 0 && errno == EINTR));
		ATF_REQUIRE(epoll_pwait_got_signal == 1);

#ifdef __APPLE__
		n = ppoll(&pfd, 1, &(struct timespec) { 0, 500000000 }, NULL);
		ATF_REQUIRE(n == 0);
#endif
	}

	ATF_REQUIRE(close(ep) == 0);
}

ATF_TC_WITHOUT_HEAD(epoll__cloexec);
ATF_TC_BODY_FD_LEAKCHECK(epoll__cloexec, tcptr)
{
	int fd;
	int r;

#define CLOEXEC_TEST(fun, cmp, ...)                               \
	do {                                                      \
		fd = fun(__VA_ARGS__);                            \
		ATF_REQUIRE(fd >= 0);                             \
		r = fcntl(fd, F_GETFD);                           \
		ATF_REQUIRE(r >= 0);                              \
		ATF_REQUIRE_MSG((r & FD_CLOEXEC) cmp 0, "%d", r); \
		ATF_REQUIRE(close(fd) == 0);                      \
	} while (0)

	CLOEXEC_TEST(epoll_create1, !=, EPOLL_CLOEXEC);
	CLOEXEC_TEST(epoll_create1, ==, 0);
	CLOEXEC_TEST(timerfd_create, !=, CLOCK_MONOTONIC, TFD_CLOEXEC);
	CLOEXEC_TEST(timerfd_create, ==, CLOCK_MONOTONIC, 0);
	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	CLOEXEC_TEST(signalfd, !=, -1, &mask, SFD_CLOEXEC);
	CLOEXEC_TEST(signalfd, ==, -1, &mask, 0);
	CLOEXEC_TEST(eventfd, !=, 0, EFD_CLOEXEC);
	CLOEXEC_TEST(eventfd, ==, 0, 0);
}

ATF_TC_WITHOUT_HEAD(epoll__fcntl_fl);
ATF_TC_BODY_FD_LEAKCHECK(epoll__fcntl_fl, tcptr)
{
	int fd;
	int r;

#define FCNTL_FL_TEST(fun, expected, ...)                    \
	do {                                                 \
		fd = fun(__VA_ARGS__);                       \
		ATF_REQUIRE(fd >= 0);                        \
		r = fcntl(fd, F_GETFL, 0);                   \
		ATF_REQUIRE_MSG(r == (expected), "%04x", r); \
		ATF_REQUIRE(close(fd) == 0);                 \
	} while (0)

	FCNTL_FL_TEST(epoll_create1, O_RDWR, 0);
	{
		fd = epoll_create1(0);
		ATF_REQUIRE(fd >= 0);

		r = fcntl(fd, F_GETFL, 0);
		ATF_REQUIRE(r >= 0);
		r = fcntl(fd, F_SETFL, r | O_NONBLOCK);
		ATF_REQUIRE_MSG(r >= 0, "%s", strerror(errno));

		r = fcntl(fd, F_GETFL, 0);
		ATF_REQUIRE(r == (O_RDWR | O_NONBLOCK));
		ATF_REQUIRE(close(fd) == 0);
	}
	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	FCNTL_FL_TEST(signalfd, O_RDWR, -1, &mask, 0);
	FCNTL_FL_TEST(signalfd, O_RDWR | O_NONBLOCK, -1, &mask, SFD_NONBLOCK);
	FCNTL_FL_TEST(eventfd, O_RDWR, 0, 0);
	FCNTL_FL_TEST(eventfd, O_RDWR | O_NONBLOCK, 0, EFD_NONBLOCK);
#if defined(__NetBSD__) && __NetBSD_Version__ >= 999009100
	FCNTL_FL_TEST(timerfd_create, 0, CLOCK_MONOTONIC, 0);
	FCNTL_FL_TEST(timerfd_create, 0 | O_NONBLOCK, CLOCK_MONOTONIC,
	    TFD_NONBLOCK);
#else
	FCNTL_FL_TEST(timerfd_create, O_RDWR, CLOCK_MONOTONIC, 0);
	FCNTL_FL_TEST(timerfd_create, O_RDWR | O_NONBLOCK, CLOCK_MONOTONIC,
	    TFD_NONBLOCK);
#endif
}

ATF_TP_ADD_TCS(tp)
{
	ATF_TP_ADD_TC(tp, epoll__simple);
#ifndef USE_EPOLLRDHUP_LINUX_DEFINITION
	ATF_TP_ADD_TC(tp, epoll__poll_flags);
#endif
	ATF_TP_ADD_TC(tp, epoll__leakcheck);
	ATF_TP_ADD_TC(tp, epoll__fd_exhaustion);
	ATF_TP_ADD_TC(tp, epoll__invalid_op);
	ATF_TP_ADD_TC(tp, epoll__invalid_op2);
	ATF_TP_ADD_TC(tp, epoll__rdhup_linux);
	ATF_TP_ADD_TC(tp, epoll__simple_wait);
	ATF_TP_ADD_TC(tp, epoll__event_size);
	ATF_TP_ADD_TC(tp, epoll__recursive_register);
	ATF_TP_ADD_TC(tp, epoll__simple_epollin);
	ATF_TP_ADD_TC(tp, epoll__sleep_argument);
	ATF_TP_ADD_TC(tp, epoll__remove_nonexistent);
	ATF_TP_ADD_TC(tp, epoll__add_remove);
	ATF_TP_ADD_TC(tp, epoll__add_existing);
	ATF_TP_ADD_TC(tp, epoll__modify_existing);
	ATF_TP_ADD_TC(tp, epoll__modify_nonexisting);
	ATF_TP_ADD_TC(tp, epoll__poll_only_fd);
	ATF_TP_ADD_TC(tp, epoll__no_epollin_on_closed_empty_pipe);
	ATF_TP_ADD_TC(tp, epoll__write_to_pipe_until_full);
	ATF_TP_ADD_TC(tp, epoll__realtime_timer);
	ATF_TP_ADD_TC(tp, epoll__simple_signalfd);
	ATF_TP_ADD_TC(tp, epoll__signalfd_poll_sigusr1);
	ATF_TP_ADD_TC(tp, epoll__signalfd_in_thread);
	ATF_TP_ADD_TC(tp, epoll__socket_shutdown);
	ATF_TP_ADD_TC(tp, epoll__epollhup_on_fresh_socket);
	ATF_TP_ADD_TC(tp, epoll__epollout_on_connecting_socket);
	ATF_TP_ADD_TC(tp, epoll__epollpri);
	ATF_TP_ADD_TC(tp, epoll__epollpri_oobinline);
	ATF_TP_ADD_TC(tp, epoll__epollpri_oobinline_lt);
	ATF_TP_ADD_TC(tp, epoll__timeout_on_listening_socket);
	ATF_TP_ADD_TC(tp, epoll__epollerr_on_closed_pipe);
	ATF_TP_ADD_TC(tp, epoll__shutdown_behavior);
	ATF_TP_ADD_TC(tp, epoll__datagram_connection);
	ATF_TP_ADD_TC(tp, epoll__epollout_on_own_shutdown);
	ATF_TP_ADD_TC(tp, epoll__remove_closed);
	ATF_TP_ADD_TC(tp, epoll__remove_closed_when_same_fd_open);
	ATF_TP_ADD_TC(tp, epoll__add_different_file_with_same_fd_value);
	ATF_TP_ADD_TC(tp, epoll__invalid_writes);
	ATF_TP_ADD_TC(tp, epoll__using_real_close);
	ATF_TP_ADD_TC(tp, epoll__epoll_pwait);
	ATF_TP_ADD_TC(tp, epoll__cloexec);
	ATF_TP_ADD_TC(tp, epoll__fcntl_fl);

	return atf_no_error();
}
