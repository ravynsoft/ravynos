#include <sys/signalfd.h>

#include <sys/types.h>

#include <sys/event.h>
#include <sys/stat.h>

#include <assert.h>
#include <errno.h>
#include <poll.h>
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "wrap.h"

#include "epoll_shim_ctx.h"
#include "epoll_shim_export.h"
#include "errno_return.h"

static errno_t
signalfd_ctx_read_or_block(FileDescription *desc, int kq,
    SignalFDCtxSiginfo *siginfo, bool force_nonblock)
{
	errno_t ec;
	SignalFDCtx *signalfd_ctx = &desc->ctx.signalfd;

	for (;;) {
		(void)pthread_mutex_lock(&desc->mutex);
		ec = signalfd_ctx_read(signalfd_ctx, kq, siginfo);
		bool nonblock = force_nonblock ||
		    (desc->flags & O_NONBLOCK) != 0;
		(void)pthread_mutex_unlock(&desc->mutex);
		if (nonblock || (ec != EAGAIN && ec != EWOULDBLOCK)) {
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
signalfd_read(FileDescription *desc, int kq, void *buf, size_t nbytes,
    size_t *bytes_transferred)
{
	errno_t ec;

	if (nbytes < sizeof(struct signalfd_siginfo)) {
		return EINVAL;
	}

	bool force_nonblock = false;
	size_t bytes_transferred_local = 0;

	while (nbytes >= sizeof(struct signalfd_siginfo)) {
		_Static_assert(sizeof(struct signalfd_siginfo) ==
			sizeof(SignalFDCtxSiginfo),
		    "");

		SignalFDCtxSiginfo siginfo;
		memset(&siginfo, 0, sizeof(siginfo));

		if ((ec = signalfd_ctx_read_or_block(desc, kq, &siginfo,
			 force_nonblock)) != 0) {
			break;
		}

		memcpy(buf, &siginfo, sizeof(siginfo));
		bytes_transferred_local += sizeof(siginfo);

		force_nonblock = true;
		nbytes -= sizeof(siginfo);
		buf = ((unsigned char *)buf) + sizeof(siginfo);
	}

	if (bytes_transferred_local > 0) {
		ec = 0;
	}

	*bytes_transferred = bytes_transferred_local;
	return ec;
}

static errno_t
signalfd_close(FileDescription *desc)
{
	return signalfd_ctx_terminate(&desc->ctx.signalfd);
}

static void
signalfd_poll(FileDescription *desc, int kq, uint32_t *revents)
{
	(void)pthread_mutex_lock(&desc->mutex);
	signalfd_ctx_poll(&desc->ctx.signalfd, kq, revents);
	(void)pthread_mutex_unlock(&desc->mutex);
}

static struct file_description_vtable const signalfd_vtable = {
	.read_fun = signalfd_read,
	.write_fun = fd_context_default_write,
	.close_fun = signalfd_close,
	.poll_fun = signalfd_poll,
};

static errno_t
signalfd_impl(int *sfd_out, int fd, sigset_t const *sigs, int flags)
{
	errno_t ec;

	if (sigs == NULL || (flags & ~(SFD_NONBLOCK | SFD_CLOEXEC))) {
		return EINVAL;
	}

	if (fd != -1) {
		struct stat sb;
		return (fd < 0 || fstat(fd, &sb) < 0) ? EBADF : EINVAL;
	}

	_Static_assert(SFD_CLOEXEC == O_CLOEXEC, "");
	_Static_assert(SFD_NONBLOCK == O_NONBLOCK, "");

	EpollShimCtx *epoll_shim_ctx;
	if ((ec = epoll_shim_ctx_global(&epoll_shim_ctx)) != 0) {
		return ec;
	}

	int sfd;
	FileDescription *desc;
	ec = epoll_shim_ctx_create_desc(epoll_shim_ctx,
	    flags & (O_CLOEXEC | O_NONBLOCK), &sfd, &desc);
	if (ec != 0) {
		return ec;
	}

	desc->flags = flags & O_NONBLOCK;

	if ((ec = signalfd_ctx_init(&desc->ctx.signalfd, sfd, sigs)) != 0) {
		goto fail;
	}

	desc->vtable = &signalfd_vtable;
	epoll_shim_ctx_install_desc(epoll_shim_ctx, sfd, desc);

	*sfd_out = sfd;
	return 0;

fail:
	epoll_shim_ctx_drop_desc(epoll_shim_ctx, sfd, desc);
	return ec;
}

EPOLL_SHIM_EXPORT
int
signalfd(int fd, sigset_t const *sigs, int flags)
{
	ERRNO_SAVE;
	errno_t ec;

	int sfd_out;
	ec = signalfd_impl(&sfd_out, fd, sigs, flags);

	ERRNO_RETURN(ec, -1, sfd_out);
}
