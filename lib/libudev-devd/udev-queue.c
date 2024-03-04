/*
 * Copyright (c) 2021 Vladimir Kondratyev <vladimir@kondratyev.su>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "utils.h"

#include <sys/socket.h>
#include <sys/un.h>

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include "libudev.h"

#include "udev-utils.h"

#define	DEVD_SOCK_PATH	"/var/run/devd.pipe"

struct udev_queue {
	int refcount;
	struct udev *udev;
	int fd;
};

LIBUDEV_EXPORT struct udev_queue *
udev_queue_new(struct udev *udev)
{
	struct udev_queue *uq;

	TRC();
	uq = calloc(1, sizeof(struct udev_queue));
	if (uq == NULL)
		return (NULL);

	uq->udev = udev;
	udev_ref(udev);
	uq->refcount = 1;
	uq->fd = -1;

	return (uq);
}

LIBUDEV_EXPORT struct udev_queue *
udev_queue_ref(struct udev_queue *uq)
{
	TRC("(%p) refcount=%d", uq, uq->refcount);
	++uq->refcount;
	return (uq);
}

LIBUDEV_EXPORT struct udev_queue *
udev_queue_unref(struct udev_queue *uq)
{
	TRC("(%p) refcount=%d", uq, uq->refcount);
	if (--uq->refcount == 0) {
		if (uq->fd >= 0)
			close(uq->fd);
		udev_unref(uq->udev);
                free(uq);
        }
        return (uq);
}

LIBUDEV_EXPORT struct udev *
udev_queue_get_udev(struct udev_queue *uq)
{
	TRC("(%p)", uq);
	return (uq->udev);
}

LIBUDEV_EXPORT unsigned long long int
udev_queue_get_kernel_seqnum(struct udev_queue *uq)
{
	TRC("(%p)", uq);
	return (0);
}

LIBUDEV_EXPORT unsigned long long int
udev_queue_get_udev_seqnum(struct udev_queue *uq)
{
	TRC("(%p)", uq);
	return (0);
}

LIBUDEV_EXPORT int
udev_queue_get_udev_is_active(struct udev_queue *uq)
{
	TRC("(%p)", uq);
	return (1);
}

static int
_udev_queue_get_queue_is_empty(struct udev_queue *uq)
{
	UNIMPL();
	return (1);
}

LIBUDEV_EXPORT int
udev_queue_get_queue_is_empty(struct udev_queue *uq)
{
	TRC("(%p)", uq);
	return (_udev_queue_get_queue_is_empty(uq));
}

LIBUDEV_EXPORT int
udev_queue_get_seqnum_sequence_is_finished(struct udev_queue *uq,
    unsigned long long int start, unsigned long long int end)
{
	TRC("(%p)", uq);
	return (_udev_queue_get_queue_is_empty(uq));
}

LIBUDEV_EXPORT int
udev_queue_get_seqnum_is_finished(struct udev_queue *uq,
    unsigned long long int seqnum)
{
	TRC("(%p)", uq);
	return (_udev_queue_get_queue_is_empty(uq));
}

LIBUDEV_EXPORT struct udev_list_entry *
udev_queue_get_queued_list_entry(struct udev_queue *uq)
{
	TRC("(%p)", uq);
	errno = ENOMSG;
	return (NULL);
}

LIBUDEV_EXPORT int
udev_queue_get_fd(struct udev_queue *uq)
{
	TRC("(%p)", uq);

	if (uq->fd >= 0)
		return (uq->fd);
#if 0
	const static struct sockaddr_un sa = {
		.sun_family = AF_UNIX,
		.sun_path = DEVD_SOCK_PATH,
	};

	uq->fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (uq->fd >= 0 &&
	    connect(uq->fd, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
		close(uq->fd);
		uq->fd = -1;
	}
#else
	uq->fd = open("/dev/null", O_RDONLY | O_CLOEXEC);
#endif

	return (uq->fd);
}

LIBUDEV_EXPORT int
udev_queue_flush(struct udev_queue *uq)
{
	TRC("(%p)", uq);
	UNIMPL();
	return 0;
}
