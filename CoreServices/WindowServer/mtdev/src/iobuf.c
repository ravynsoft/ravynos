/*****************************************************************************
 *
 * mtdev - Multitouch Protocol Translation Library (MIT license)
 *
 * Copyright (C) 2010 Henrik Rydberg <rydberg@euromail.se>
 * Copyright (C) 2010 Canonical Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 ****************************************************************************/

#include "iobuf.h"
#include "state.h"
#include <unistd.h>
#include <poll.h>

int mtdev_idle(struct mtdev *dev, int fd, int ms)
{
	struct mtdev_iobuf *buf = &dev->state->iobuf;
        struct pollfd fds = { fd, POLLIN, 0 };
	return	buf->head == buf->tail && poll(&fds, 1, ms) <= 0;
}

int mtdev_fetch_event(struct mtdev *dev, int fd, struct input_event *ev)
{
	struct mtdev_iobuf *buf = &dev->state->iobuf;
	int n = buf->head - buf->tail;
	if (n < EVENT_SIZE) {
		if (buf->tail && n > 0)
			memmove(buf->data, buf->data + buf->tail, n);
		buf->head = n;
		buf->tail = 0;
		SYSCALL(n = read(fd, buf->data + buf->head,
				 DIM_BUFFER - buf->head));
		if (n <= 0)
			return n;
		buf->head += n;
	}
	if (buf->head - buf->tail < EVENT_SIZE)
		return 0;
	memcpy(ev, buf->data + buf->tail, EVENT_SIZE);
	buf->tail += EVENT_SIZE;
	return 1;
}

int mtdev_empty(struct mtdev *dev)
{
	return evbuf_empty(&dev->state->outbuf);
}

void mtdev_get_event(struct mtdev *dev, struct input_event* ev)
{
	evbuf_get(&dev->state->outbuf, ev);
}

int mtdev_get(struct mtdev *dev, int fd, struct input_event* ev, int ev_max)
{
	struct input_event kev;
	int ret, count = 0;
	while (count < ev_max) {
		while (mtdev_empty(dev)) {
			ret = mtdev_fetch_event(dev, fd, &kev);
			if (ret <= 0)
				return count > 0 ? count : ret;
			mtdev_put_event(dev, &kev);
		}
		mtdev_get_event(dev, &ev[count++]);
	}
	return count;
}
