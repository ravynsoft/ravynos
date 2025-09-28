/*
 * Copyright © 2019 Red Hat, Inc.
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
 */

#pragma once

#include "config.h"

#include "util-time.h"
#include <linux/input.h>

static inline struct input_event
input_event_init(uint64_t time,
		 unsigned int type,
		 unsigned int code,
		 int value)
{
	struct input_event ev;
	struct timeval tval = us2tv(time);

	ev.input_event_sec = tval.tv_sec;
	ev.input_event_usec = tval.tv_usec;
	ev.type = type;
	ev.code = code;
	ev.value = value;

	return ev;
}

static inline uint64_t
input_event_time(const struct input_event *e)
{
	struct timeval tval;

	tval.tv_sec = e->input_event_sec;
	tval.tv_usec = e->input_event_usec;

	return tv2us(&tval);
}

static inline void
input_event_set_time(struct input_event *e,
		     uint64_t time)
{
	struct timeval tval = us2tv(time);

	e->input_event_sec = tval.tv_sec;
	e->input_event_usec = tval.tv_usec;
}

static inline double
absinfo_range(const struct input_absinfo *abs)
{
	return (double)(abs->maximum - abs->minimum + 1);
}
