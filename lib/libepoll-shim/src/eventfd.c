#include <sys/eventfd.h>

#include <sys/types.h>

#include <sys/event.h>
#include <sys/param.h>

#include <errno.h>
#include <poll.h>
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "wrap.h"

#include "epoll_shim_ctx.h"
#include "epoll_shim_export.h"

static errno_t
eventfd_ctx_read_or_block(FileDescription *desc, int kq, uint64_t *value)
{
	errno_t ec;
	EventFDCtx *eventfd_ctx = &desc->ctx.eventfd;

	for (;;) {
		(void)pthread_mutex_lock(&desc->mutex);
		ec = eventfd_ctx_read(eventfd_ctx, kq, value);
		bool nonblock = (desc->flags & O_NONBLOCK) != 0;
		(void)pthread_mutex_unlock(&desc->mutex);

		if (nonblock || ec != EAGAIN) {
			return ec;
		}

		struct pollfd pfd = {
			.fd = kq,
			.events = POLLIN,
		};
		if (real_poll(&pfd, 1, -1) < 0) {
			return errno;
		}
	}
}

static errno_t
eventfd_helper_read(FileDescription *desc, int kq, /**/
    void *buf, size_t nbytes, size_t *bytes_transferred)
{
	errno_t ec;

	if (nbytes != sizeof(uint64_t)) {
		return EINVAL;
	}

	uint64_t value;
	if ((ec = eventfd_ctx_read_or_block(desc, kq, &value)) != 0) {
		return ec;
	}

	memcpy(buf, &value, sizeof(value));
	*bytes_transferred = sizeof(value);
	return 0;
}

static errno_t
eventfd_helper_write(FileDescription *desc, int kq, /**/
    void const *buf, size_t nbytes, size_t *bytes_transferred)
{
	errno_t ec;

	if (nbytes != sizeof(uint64_t)) {
		return EINVAL;
	}

	uint64_t value;
	memcpy(&value, buf, sizeof(uint64_t));

	(void)pthread_mutex_lock(&desc->mutex);
	ec = eventfd_ctx_write(&desc->ctx.eventfd, kq, value);
	(void)pthread_mutex_unlock(&desc->mutex);
	if (ec != 0) {
		return ec;
	}

	*bytes_transferred = sizeof(value);
	return 0;
}

static errno_t
eventfd_close(FileDescription *desc)
{
	return eventfd_ctx_terminate(&desc->ctx.eventfd);
}

static struct file_description_vtable const eventfd_vtable = {
	.read_fun = eventfd_helper_read,
	.write_fun = eventfd_helper_write,
	.close_fun = eventfd_close,
};

static errno_t
eventfd_impl(int *fd_out, unsigned int initval, int flags)
{
	errno_t ec;

	if (flags & ~(EFD_SEMAPHORE | EFD_CLOEXEC | EFD_NONBLOCK)) {
		return EINVAL;
	}

	_Static_assert(EFD_CLOEXEC == O_CLOEXEC, "");
	_Static_assert(EFD_NONBLOCK == O_NONBLOCK, "");

	EpollShimCtx *epoll_shim_ctx;
	if ((ec = epoll_shim_ctx_global(&epoll_shim_ctx)) != 0) {
		return ec;
	}

	int fd;
	FileDescription *desc;
	ec = epoll_shim_ctx_create_desc(epoll_shim_ctx,
	    flags & (O_CLOEXEC | O_NONBLOCK), &fd, &desc);
	if (ec != 0) {
		return ec;
	}

	desc->flags = flags & O_NONBLOCK;

	int ctx_flags = 0;
	if (flags & EFD_SEMAPHORE) {
		ctx_flags |= EVENTFD_CTX_FLAG_SEMAPHORE;
	}

	if ((ec = eventfd_ctx_init(&desc->ctx.eventfd, fd, initval,
		 ctx_flags)) != 0) {
		goto fail;
	}

	desc->vtable = &eventfd_vtable;
	epoll_shim_ctx_install_desc(epoll_shim_ctx, fd, desc);

	*fd_out = fd;
	return 0;

fail:
	epoll_shim_ctx_drop_desc(epoll_shim_ctx, fd, desc);
	return ec;
}

EPOLL_SHIM_EXPORT
int
eventfd(unsigned int initval, int flags)
{
	errno_t ec;

	int fd;
	ec = eventfd_impl(&fd, initval, flags);
	if (ec != 0) {
		errno = ec;
		return -1;
	}

	return fd;
}

EPOLL_SHIM_EXPORT
int
eventfd_read(int fd, eventfd_t *value)
{
	return (epoll_shim_read(fd, /**/
		    value, sizeof(*value)) == (ssize_t)sizeof(*value)) ?
	    0 :
	    -1;
}

EPOLL_SHIM_EXPORT
int
eventfd_write(int fd, eventfd_t value)
{
	return (epoll_shim_write(fd, /**/
		    &value, sizeof(value)) == (ssize_t)sizeof(value)) ?
	    0 :
	    -1;
}
