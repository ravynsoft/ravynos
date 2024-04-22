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

#ifndef MTDEV_EVBUF_H
#define MTDEV_EVBUF_H

#include "common.h"

struct mtdev_evbuf {
	int head;
	int tail;
	struct input_event buffer[DIM_EVENTS];
};

static inline int evbuf_empty(const struct mtdev_evbuf *evbuf)
{
	return evbuf->head == evbuf->tail;
}

static inline void evbuf_put(struct mtdev_evbuf *evbuf,
			     const struct input_event *ev)
{
	evbuf->buffer[evbuf->head++] = *ev;
	evbuf->head &= DIM_EVENTS - 1;
}

static inline void evbuf_get(struct mtdev_evbuf *evbuf,
			     struct input_event *ev)
{
	*ev = evbuf->buffer[evbuf->tail++];
	evbuf->tail &= DIM_EVENTS - 1;
}

#endif
