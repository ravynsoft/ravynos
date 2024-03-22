/*
 * Copyright © 2012 Collabora, Ltd.
 * Copyright © 2012 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "../config.h"

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dlfcn.h>
#include <errno.h>
#include <stdarg.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/epoll.h>

#include "wayland-private.h"
#include "test-runner.h"
#include "wayland-os.h"

static int fall_back;

/* Play nice with sanitizers
 *
 * Sanitizers need to intercept syscalls in the compiler run-time library. As
 * this isn't a separate ELF object, the usual dlsym(RTLD_NEXT) approach won't
 * work: there can only be one function named "socket" etc. To support this, the
 * sanitizer library names its interceptors with the prefix __interceptor_ ("__"
 * being reserved for the implementation) and then weakly aliases it to the real
 * function. The functions we define below will override the weak alias, and we
 * can call them by the __interceptor_ name directly. This allows the sanitizer
 * to do its work before calling the next version of the function via dlsym.
 *
 * However! We also don't know which of these functions the sanitizer actually
 * wants to override, so we have to declare our own weak symbols for
 * __interceptor_ and check at run time if they linked to anything or not.
*/

#define DECL(ret_type, func, ...) \
	ret_type __interceptor_ ## func(__VA_ARGS__) __attribute__((weak)); \
	static ret_type (*real_ ## func)(__VA_ARGS__);			\
	static int wrapped_calls_ ## func;

#define REAL(func) (__interceptor_ ## func) ?				\
	__interceptor_ ## func :					\
	(__typeof__(&__interceptor_ ## func))dlsym(RTLD_NEXT, #func)

DECL(int, socket, int, int, int);
DECL(int, fcntl, int, int, ...);
DECL(ssize_t, recvmsg, int, struct msghdr *, int);
DECL(int, epoll_create1, int);

static void
init_fallbacks(int do_fallbacks)
{
	fall_back = do_fallbacks;
	real_socket = REAL(socket);
	real_fcntl = REAL(fcntl);
	real_recvmsg = REAL(recvmsg);
	real_epoll_create1 = REAL(epoll_create1);
}

__attribute__ ((visibility("default"))) int
socket(int domain, int type, int protocol)
{
	wrapped_calls_socket++;

	if (fall_back && (type & SOCK_CLOEXEC)) {
		errno = EINVAL;
		return -1;
	}

	return real_socket(domain, type, protocol);
}

__attribute__ ((visibility("default"))) int
(fcntl)(int fd, int cmd, ...)
{
	va_list ap;
	int arg;
	int has_arg;

	wrapped_calls_fcntl++;

	if (fall_back && (cmd == F_DUPFD_CLOEXEC)) {
		errno = EINVAL;
		return -1;
	}
	switch (cmd) {
	case F_DUPFD_CLOEXEC:
	case F_DUPFD:
	case F_SETFD:
		va_start(ap, cmd);
		arg = va_arg(ap, int);
		has_arg = 1;
		va_end(ap);
		break;
	case F_GETFD:
		has_arg = 0;
		break;
	default:
		fprintf(stderr, "Unexpected fctnl cmd %d\n", cmd);
		abort();
	}

	if (has_arg) {
		return real_fcntl(fd, cmd, arg);
	}
	return real_fcntl(fd, cmd);
}

__attribute__ ((visibility("default"))) ssize_t
recvmsg(int sockfd, struct msghdr *msg, int flags)
{
	wrapped_calls_recvmsg++;

	if (fall_back && (flags & MSG_CMSG_CLOEXEC)) {
		errno = EINVAL;
		return -1;
	}

	return real_recvmsg(sockfd, msg, flags);
}

__attribute__ ((visibility("default"))) int
epoll_create1(int flags)
{
	wrapped_calls_epoll_create1++;

	if (fall_back) {
		wrapped_calls_epoll_create1++; /* epoll_create() not wrapped */
		errno = EINVAL;
		return -1;
	}

	return real_epoll_create1(flags);
}

static void
do_os_wrappers_socket_cloexec(int n)
{
	int fd;
	int nr_fds;

	nr_fds = count_open_fds();

	/* simply create a socket that closes on exec */
	fd = wl_os_socket_cloexec(PF_LOCAL, SOCK_STREAM, 0);
	assert(fd >= 0);

	/*
	 * Must have 2 calls if falling back, but must also allow
	 * falling back without a forced fallback.
	 */
	assert(wrapped_calls_socket > n);

	exec_fd_leak_check(nr_fds);
}

TEST(os_wrappers_socket_cloexec)
{
	/* normal case */
	init_fallbacks(0);
	do_os_wrappers_socket_cloexec(0);
}

TEST(os_wrappers_socket_cloexec_fallback)
{
	/* forced fallback */
	init_fallbacks(1);
	do_os_wrappers_socket_cloexec(1);
}

static void
do_os_wrappers_dupfd_cloexec(int n)
{
	int base_fd;
	int fd;
	int nr_fds;

	nr_fds = count_open_fds();

	base_fd = socket(PF_LOCAL, SOCK_STREAM, 0);
	assert(base_fd >= 0);

	fd = wl_os_dupfd_cloexec(base_fd, 13);
	assert(fd >= 13);

	close(base_fd);

	/*
	 * Must have 4 calls if falling back, but must also allow
	 * falling back without a forced fallback.
	 */
	assert(wrapped_calls_fcntl > n);

	exec_fd_leak_check(nr_fds);
}

TEST(os_wrappers_dupfd_cloexec)
{
	init_fallbacks(0);
	do_os_wrappers_dupfd_cloexec(0);
}

TEST(os_wrappers_dupfd_cloexec_fallback)
{
	init_fallbacks(1);
	do_os_wrappers_dupfd_cloexec(3);
}

struct marshal_data {
	struct wl_connection *read_connection;
	struct wl_connection *write_connection;
	int s[2];
	uint32_t read_mask;
	uint32_t write_mask;
	union {
		int h[3];
	} value;
	int nr_fds_begin;
	int nr_fds_conn;
	int wrapped_calls;
};

static void
setup_marshal_data(struct marshal_data *data)
{
	assert(socketpair(AF_UNIX,
			  SOCK_STREAM | SOCK_CLOEXEC, 0, data->s) == 0);

	data->read_connection = wl_connection_create(data->s[0]);
	assert(data->read_connection);

	data->write_connection = wl_connection_create(data->s[1]);
	assert(data->write_connection);
}

static void
marshal_demarshal(struct marshal_data *data,
		  void (*func)(void), int size, const char *format, ...)
{
	struct wl_closure *closure;
	static const int opcode = 4444;
	static struct wl_object sender = { NULL, NULL, 1234 };
	struct wl_message message = { "test", format, NULL };
	struct wl_map objects;
	struct wl_object object = { NULL, &func, 1234 };
	va_list ap;
	uint32_t msg[1] = { 1234 };

	va_start(ap, format);
	closure = wl_closure_vmarshal(&sender, opcode, ap, &message);
	va_end(ap);

	assert(closure);
	assert(wl_closure_send(closure, data->write_connection) == 0);
	wl_closure_destroy(closure);
	assert(wl_connection_flush(data->write_connection) == size);

	assert(wl_connection_read(data->read_connection) == size);

	wl_map_init(&objects, WL_MAP_SERVER_SIDE);
	object.id = msg[0];
	closure = wl_connection_demarshal(data->read_connection,
					  size, &objects, &message);
	assert(closure);
	wl_closure_invoke(closure, WL_CLOSURE_INVOKE_SERVER, &object, 0, data);
	wl_closure_destroy(closure);
}

static void
validate_recvmsg_h(struct marshal_data *data,
		   struct wl_object *object, int fd1, int fd2, int fd3)
{
	struct stat buf1, buf2;

	assert(fd1 >= 0);
	assert(fd2 >= 0);
	assert(fd3 >= 0);

	assert(fd1 != data->value.h[0]);
	assert(fd2 != data->value.h[1]);
	assert(fd3 != data->value.h[2]);

	assert(fstat(fd3, &buf1) == 0);
	assert(fstat(data->value.h[2], &buf2) == 0);
	assert(buf1.st_dev == buf2.st_dev);
	assert(buf1.st_ino == buf2.st_ino);

	/* close the original file descriptors */
	close(data->value.h[0]);
	close(data->value.h[1]);
	close(data->value.h[2]);

	/* the dup'd (received) fds should still be open */
	assert(count_open_fds() == data->nr_fds_conn + 3);

	/*
	 * Must have 2 calls if falling back, but must also allow
	 * falling back without a forced fallback.
	 */
	assert(wrapped_calls_recvmsg > data->wrapped_calls);

	if (data->wrapped_calls == 0 && wrapped_calls_recvmsg > 1)
		printf("recvmsg fell back unforced.\n");

	/* all fds opened during the test in any way should be gone on exec */
	exec_fd_leak_check(data->nr_fds_begin);
}

static void
do_os_wrappers_recvmsg_cloexec(int n)
{
	struct marshal_data data;

	data.nr_fds_begin = count_open_fds();
#if HAVE_BROKEN_MSG_CMSG_CLOEXEC
	/* We call the fallback directly on FreeBSD versions with a broken
	 * MSG_CMSG_CLOEXEC, so we don't call the local recvmsg() wrapper. */
	data.wrapped_calls = 0;
#else
	data.wrapped_calls = n;
#endif

	setup_marshal_data(&data);
	data.nr_fds_conn = count_open_fds();

	assert(pipe(data.value.h) >= 0);

	data.value.h[2] = open("/dev/zero", O_RDONLY);
	assert(data.value.h[2] >= 0);

	marshal_demarshal(&data, (void *) validate_recvmsg_h,
			  8, "hhh", data.value.h[0], data.value.h[1],
			  data.value.h[2]);
}

TEST(os_wrappers_recvmsg_cloexec)
{
	init_fallbacks(0);
	do_os_wrappers_recvmsg_cloexec(0);
}

TEST(os_wrappers_recvmsg_cloexec_fallback)
{
	init_fallbacks(1);
	do_os_wrappers_recvmsg_cloexec(1);
}

static void
do_os_wrappers_epoll_create_cloexec(int n)
{
	int fd;
	int nr_fds;

	nr_fds = count_open_fds();

	fd = wl_os_epoll_create_cloexec();
	assert(fd >= 0);

#ifdef EPOLL_CLOEXEC
	assert(wrapped_calls_epoll_create1 == n);
#else
	printf("No epoll_create1.\n");
#endif

	exec_fd_leak_check(nr_fds);
}

TEST(os_wrappers_epoll_create_cloexec)
{
	init_fallbacks(0);
	do_os_wrappers_epoll_create_cloexec(1);
}

TEST(os_wrappers_epoll_create_cloexec_fallback)
{
	init_fallbacks(1);
	do_os_wrappers_epoll_create_cloexec(2);
}

/* FIXME: add tests for wl_os_accept_cloexec() */
