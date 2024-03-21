/*
 * Copyright © 2014 Red Hat, Inc.
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

#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

#include "libinput-util.h"

struct libinput;

struct libinput_timer {
	struct libinput *libinput;
	char *timer_name;
	struct list link;
	uint64_t expire; /* in absolute us CLOCK_MONOTONIC */
	void (*timer_func)(uint64_t now, void *timer_func_data);
	void *timer_func_data;
};

void
libinput_timer_init(struct libinput_timer *timer, struct libinput *libinput,
		    const char *timer_name,
		    void (*timer_func)(uint64_t now, void *timer_func_data),
		    void *timer_func_data);

void
libinput_timer_destroy(struct libinput_timer *timer);

/* Set timer expire time, in absolute us CLOCK_MONOTONIC */
void
libinput_timer_set(struct libinput_timer *timer, uint64_t expire);

enum timer_flags {
	TIMER_FLAG_NONE = 0,
	TIMER_FLAG_ALLOW_NEGATIVE = bit(0),
};

void
libinput_timer_set_flags(struct libinput_timer *timer,
			 uint64_t expire,
			 uint32_t flags);

void
libinput_timer_cancel(struct libinput_timer *timer);

int
libinput_timer_subsys_init(struct libinput *libinput);

void
libinput_timer_subsys_destroy(struct libinput *libinput);

void
libinput_timer_flush(struct libinput *libinput, uint64_t now);

#endif
