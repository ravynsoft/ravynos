#include <sys/timerfd.h>

#include <sys/event.h>
#include <sys/select.h>
#include <sys/stat.h>

#include <poll.h>
#include <pthread.h>

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "wrap.h"

#include "epoll_shim_ctx.h"
#include "epoll_shim_export.h"
#include "errno_return.h"

static errno_t
timerfd_ctx_read_or_block(FileDescription *desc, int kq, uint64_t *value)
{
	errno_t ec;
	TimerFDCtx *timerfd = &desc->ctx.timerfd;

	for (;;) {
		(void)pthread_mutex_lock(&desc->mutex);
		ec = timerfd_ctx_read(timerfd, kq, value);
		bool nonblock = (desc->flags & O_NONBLOCK) != 0;
		(void)pthread_mutex_unlock(&desc->mutex);
		if (nonblock && ec == 0 && *value == 0) {
			ec = EAGAIN;
		}
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
timerfd_read(FileDescription *desc, int kq, void *buf, size_t nbytes,
    size_t *bytes_transferred)
{
	errno_t ec;

	if (nbytes < sizeof(uint64_t)) {
		return EINVAL;
	}

	uint64_t nr_expired;
	if ((ec = timerfd_ctx_read_or_block(desc, kq, &nr_expired)) != 0) {
		return ec;
	}

	if (nr_expired == 0) {
		*bytes_transferred = 0;
	} else {
		memcpy(buf, &nr_expired, sizeof(uint64_t));
		*bytes_transferred = sizeof(uint64_t);
	}

	return 0;
}

static errno_t
timerfd_close(FileDescription *desc)
{
	EpollShimCtx *epoll_shim_ctx;
	errno_t ec = epoll_shim_ctx_global(&epoll_shim_ctx);
	assert(ec == 0);
	(void)ec;

	int const old_can_jump = /**/
	    desc->ctx.timerfd.clockid == CLOCK_REALTIME &&
	    desc->ctx.timerfd.is_abstime;
	epoll_shim_ctx_update_realtime_change_monitoring(epoll_shim_ctx,
	    -old_can_jump);
	return timerfd_ctx_terminate(&desc->ctx.timerfd);
}

static void
timerfd_poll(FileDescription *desc, int kq, uint32_t *revents)
{
	(void)pthread_mutex_lock(&desc->mutex);
	timerfd_ctx_poll(&desc->ctx.timerfd, kq, revents);
	(void)pthread_mutex_unlock(&desc->mutex);
}

static void
timerfd_realtime_change(FileDescription *desc, int kq)
{
	(void)pthread_mutex_lock(&desc->mutex);
	timerfd_ctx_realtime_change(&desc->ctx.timerfd, kq);
	(void)pthread_mutex_unlock(&desc->mutex);
}

static struct file_description_vtable const timerfd_vtable = {
	.read_fun = timerfd_read,
	.write_fun = fd_context_default_write,
	.close_fun = timerfd_close,
	.poll_fun = timerfd_poll,
	.realtime_change_fun = timerfd_realtime_change,
};

static errno_t
timerfd_create_impl(int *fd_out, int clockid, int flags)
{
	errno_t ec;

	if (clockid != CLOCK_MONOTONIC && clockid != CLOCK_REALTIME) {
		return EINVAL;
	}

	if (flags & ~(TFD_CLOEXEC | TFD_NONBLOCK)) {
		return EINVAL;
	}

	_Static_assert(TFD_CLOEXEC == O_CLOEXEC, "");
	_Static_assert(TFD_NONBLOCK == O_NONBLOCK, "");

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

	if ((ec = timerfd_ctx_init(&desc->ctx.timerfd, clockid)) != 0) {
		goto fail;
	}

	desc->vtable = &timerfd_vtable;
	epoll_shim_ctx_install_desc(epoll_shim_ctx, fd, desc);

	*fd_out = fd;
	return 0;

fail:
	epoll_shim_ctx_drop_desc(epoll_shim_ctx, fd, desc);
	return ec;
}

EPOLL_SHIM_EXPORT
int
timerfd_create(int clockid, int flags)
{
	ERRNO_SAVE;
	errno_t ec;

	int fd;
	ec = timerfd_create_impl(&fd, clockid, flags);

	ERRNO_RETURN(ec, -1, fd);
}

static errno_t
timerfd_settime_impl(int fd, int flags, const struct itimerspec *new,
    struct itimerspec *old)
{
	errno_t ec;

	if (!new) {
		return EFAULT;
	}

	if (flags & ~(TFD_TIMER_ABSTIME | TFD_TIMER_CANCEL_ON_SET)) {
		return EINVAL;
	}

	EpollShimCtx *epoll_shim_ctx;
	if ((ec = epoll_shim_ctx_global(&epoll_shim_ctx)) != 0) {
		return ec;
	}

	FileDescription *desc = epoll_shim_ctx_find_desc(epoll_shim_ctx, fd);
	if (!desc || desc->vtable != &timerfd_vtable) {
		struct stat sb;
		ec = (fd < 0 || fstat(fd, &sb)) ? EBADF : EINVAL;
		goto out;
	}

	(void)pthread_mutex_lock(&desc->mutex);
	{
		int const old_can_jump = /**/
		    desc->ctx.timerfd.clockid == CLOCK_REALTIME &&
		    desc->ctx.timerfd.is_abstime;

		ec = timerfd_ctx_settime(&desc->ctx.timerfd, fd,
		    (flags & TFD_TIMER_ABSTIME) != 0,	    /**/
		    (flags & TFD_TIMER_CANCEL_ON_SET) != 0, /**/
		    new, old);

		if (ec == 0 || ec == ECANCELED) {
			int const new_can_jump = /**/
			    desc->ctx.timerfd.clockid == CLOCK_REALTIME &&
			    desc->ctx.timerfd.is_abstime;

			epoll_shim_ctx_update_realtime_change_monitoring(
			    epoll_shim_ctx, new_can_jump - old_can_jump);
		}
	}
	(void)pthread_mutex_unlock(&desc->mutex);

out:
	if (desc) {
		(void)file_description_unref(&desc);
	}
	return ec;
}

EPOLL_SHIM_EXPORT
int
timerfd_settime(int fd, int flags, const struct itimerspec *new,
    struct itimerspec *old)
{
	ERRNO_SAVE;
	errno_t ec;

	ec = timerfd_settime_impl(fd, flags, new, old);

	ERRNO_RETURN(ec, -1, 0);
}

static errno_t
timerfd_gettime_impl(int fd, struct itimerspec *cur)
{
	errno_t ec;

	EpollShimCtx *epoll_shim_ctx;
	if ((ec = epoll_shim_ctx_global(&epoll_shim_ctx)) != 0) {
		return ec;
	}

	FileDescription *desc = epoll_shim_ctx_find_desc(epoll_shim_ctx, fd);
	if (!desc || desc->vtable != &timerfd_vtable) {
		struct stat sb;
		ec = (fd < 0 || fstat(fd, &sb)) ? EBADF : EINVAL;
		goto out;
	}

	(void)pthread_mutex_lock(&desc->mutex);
	ec = timerfd_ctx_gettime(&desc->ctx.timerfd, cur);
	(void)pthread_mutex_unlock(&desc->mutex);

out:
	if (desc) {
		(void)file_description_unref(&desc);
	}
	return ec;
}

EPOLL_SHIM_EXPORT
int
timerfd_gettime(int fd, struct itimerspec *cur)
{
	ERRNO_SAVE;
	errno_t ec;

	ec = timerfd_gettime_impl(fd, cur);

	ERRNO_RETURN(ec, -1, 0);
}
