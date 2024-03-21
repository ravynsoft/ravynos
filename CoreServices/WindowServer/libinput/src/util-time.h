/*
 * Copyright © 2013-2019 Red Hat, Inc.
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

#include <assert.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>
#include <linux/input.h>

#include "util-macros.h"

static inline void
msleep(unsigned int ms)
{
	usleep(ms * 1000);
}

static inline uint64_t
us(uint64_t us)
{
	return us;
}

static inline uint64_t
ns2us(uint64_t ns)
{
	return us(ns / 1000);
}

static inline uint64_t
ms2us(uint64_t ms)
{
	return us(ms * 1000);
}

static inline uint64_t
s2us(uint64_t s)
{
	return ms2us(s * 1000);
}

static inline uint64_t
h2us(uint64_t h)
{
	return s2us(h * 3600);
}

static inline uint32_t
us2ms(uint64_t us)
{
	return (uint32_t)(us / 1000);
}

static inline double
us2ms_f(uint64_t us)
{
	return (double)us / 1000.0;
}

static inline uint64_t
tv2us(const struct timeval *tv)
{
	return s2us(tv->tv_sec) + tv->tv_usec;
}

static inline struct timeval
us2tv(uint64_t time)
{
	struct timeval tv;

	tv.tv_sec = time / ms2us(1000);
	tv.tv_usec = time % ms2us(1000);

	return tv;
}

struct human_time {
	unsigned int value;
	const char *unit;
};

/**
 * Converts a time delta in µs to a human-readable time like "2h" or "4d"
 */
static inline struct human_time
to_human_time(uint64_t us)
{
	struct human_time t;
	struct c {
		const char *unit;
		unsigned int change_from_previous;
		uint64_t limit;
	} conversion[] = {
		{"us", 1, 5000},
		{"ms", 1000, 5000},
		{"s", 1000, 120},
		{"min", 60, 120},
		{"h", 60, 48},
		{"d", 24, ~0},
	};
	uint64_t value = us;

	ARRAY_FOR_EACH(conversion, c) {
		value = value/c->change_from_previous;
		if (value < c->limit) {
			t.unit = c->unit;
			t.value = value;
			return t;
		}
	}

	assert(!"We should never get here");
}
