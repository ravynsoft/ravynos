#include "wrap.h"

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <dlfcn.h>
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>
#include <unistd.h>

#include "compat_ppoll.h"

static struct {
	pthread_once_t wrap_init;

	typeof(read) *real_read;
	typeof(write) *real_write;
	typeof(close) *real_close;
	typeof(poll) *real_poll;
#if !defined(__APPLE__)
#ifdef __NetBSD__
	typeof(pollts) *real___pollts50;
#else
	typeof(ppoll) *real_ppoll;
#endif
#endif
	typeof(fcntl) *real_fcntl;
} wrap = { .wrap_init = PTHREAD_ONCE_INIT };

static void
wrap_initialize_impl(void)
{
	/*
	 * Try to get the "real" function with `RTLD_NEXT` first. If that
	 * fails, it means that `libc` is before `libepoll-shim` in library
	 * search order. This shouldn't really happen, but try with
	 * `RTLD_DEFAULT` as a fallback anyway.
	 */
#define WRAP(fun)                                                    \
	do {                                                         \
		wrap.real_##fun = dlsym(RTLD_NEXT, #fun);            \
		if (wrap.real_##fun == NULL) {                       \
			wrap.real_##fun = dlsym(RTLD_DEFAULT, #fun); \
		}                                                    \
		if (wrap.real_##fun == NULL) {                       \
			fprintf(stderr,                              \
			    "epoll-shim: error resolving \"%s\" "    \
			    "with dlsym RTLD_NEXT/RTLD_DEFAULT!\n",  \
			    #fun);                                   \
			abort();                                     \
		}                                                    \
	} while (0)

	WRAP(read);
	WRAP(write);
	WRAP(close);
	WRAP(poll);
#if !defined(__APPLE__)
#ifdef __NetBSD__
	WRAP(__pollts50);
#else
	WRAP(ppoll);
#endif
#endif
	WRAP(fcntl);

#undef WRAP
}

static void
wrap_initialize(void)
{
	int const oe = errno;
	(void)pthread_once(&wrap.wrap_init, wrap_initialize_impl);
	errno = oe;
}

ssize_t
real_read(int fd, void *buf, size_t nbytes)
{
	wrap_initialize();
	return wrap.real_read(fd, buf, nbytes);
}

ssize_t
real_write(int fd, void const *buf, size_t nbytes)
{
	wrap_initialize();
	return wrap.real_write(fd, buf, nbytes);
}

int
real_close(int fd)
{
	wrap_initialize();
	return wrap.real_close(fd);
}

int
real_poll(struct pollfd fds[], nfds_t nfds, int timeout)
{
	wrap_initialize();
	return wrap.real_poll(fds, nfds, timeout);
}

int
real_ppoll(struct pollfd fds[], nfds_t nfds,
    struct timespec const *restrict timeout,
    sigset_t const *restrict newsigmask)
{
#ifdef __APPLE__
	return compat_ppoll(fds, nfds, timeout, newsigmask);
#else
	wrap_initialize();
#ifdef __NetBSD__
	return wrap.real___pollts50(fds, nfds, timeout, newsigmask);
#else
	return wrap.real_ppoll(fds, nfds, timeout, newsigmask);
#endif
#endif
}

int
real_fcntl(int fd, int cmd, ...)
{
	wrap_initialize();
	va_list ap;

	va_start(ap, cmd);
	void *arg = va_arg(ap, void *);
	int rv = wrap.real_fcntl(fd, cmd, arg);
	va_end(ap);

	return rv;
}
