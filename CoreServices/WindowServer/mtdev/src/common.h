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

#ifndef COMMON_H
#define COMMON_H

#define MTDEV_NO_LEGACY_API

#include <mtdev-mapping.h>
#include <mtdev-plumbing.h>
#include <malloc.h>
#include <string.h>
#include <errno.h>

#define DIM_FINGER 32
#define DIM2_FINGER (DIM_FINGER * DIM_FINGER)

/* event buffer size (must be a power of two) */
#define DIM_EVENTS 512

/* all bit masks have this type */
typedef unsigned int bitmask_t;

#define BITMASK(x) (1U << (x))
#define BITONES(x) (BITMASK(x) - 1U)
#define GETBIT(m, x) (((m) >> (x)) & 1U)
#define SETBIT(m, x) (m |= BITMASK(x))
#define CLEARBIT(m, x) (m &= ~BITMASK(x))
#define MODBIT(m, x, b) ((b) ? SETBIT(m, x) : CLEARBIT(m, x))

static inline int maxval(int x, int y) { return x > y ? x : y; }
static inline int minval(int x, int y) { return x < y ? x : y; }

static inline int clamp15(int x)
{
	return x < -32767 ? -32767 : x > 32767 ? 32767 : x;
}

/* absolute scale is assumed to fit in 15 bits */
static inline int dist2(int dx, int dy)
{
	dx = clamp15(dx);
	dy = clamp15(dy);
	return dx * dx + dy * dy;
}

/* Count number of bits (Sean Eron Andersson's Bit Hacks) */
static inline int bitcount(unsigned v)
{
	v -= ((v>>1) & 0x55555555);
	v = (v&0x33333333) + ((v>>2) & 0x33333333);
	return (((v + (v>>4)) & 0xF0F0F0F) * 0x1010101) >> 24;
}

/* Return index of first bit [0-31], -1 on zero */
#define firstbit(v) (__builtin_ffs(v) - 1)

/* boost-style foreach bit */
#define foreach_bit(i, m)						\
	for (i = firstbit(m); i >= 0; i = firstbit((m) & (~0U << (i + 1))))

/* robust system ioctl calls */
#define SYSCALL(call) while (((call) == -1) && (errno == EINTR))

/* To be compatible to the original, non-opaque mtdev API, we can only use 11
 * axes in the basic struct. Everything else is hidden in the state, see the
 * use of dev->abs[idx] vs dev->state->ext_abs[idx]
 *
 * See MT_ABS_SIZE in include/mtdev.h
 */
#define LEGACY_API_NUM_MT_AXES 11

/**
 * struct mtdev - represents an input MT device
 * @has_mtdata: true if the device has MT capabilities
 * @has_slot: true if the device sends MT slots
 * @slot: slot event properties
 * @abs: ABS_MT event properties
 * @state: internal mtdev parsing state
 *
 * The mtdev structure represents a kernel MT device type B, emitting
 * MT slot events. The events put into mtdev may be from any MT
 * device, specifically type A without contact tracking, type A with
 * contact tracking, or type B with contact tracking. See the kernel
 * documentation for further details.
 *
 */
struct mtdev {
	int has_mtdata;
	int has_slot;
	int has_abs[LEGACY_API_NUM_MT_AXES];
	struct input_absinfo slot;
	struct input_absinfo abs[LEGACY_API_NUM_MT_AXES];
	struct mtdev_state *state;
};

#define MT_ABS_SIZE 12

static const unsigned int mtdev_map_abs2mt[ABS_CNT] = {
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008,
 0x0009, 0x000a, 0x000b, 0x000c, 0x0000, 0x0000, 0x0000, 0x0000,
};

static const unsigned int mtdev_map_mt2abs[MT_ABS_SIZE] = {
 0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
 0x0038, 0x0039, 0x003a, 0x003b,
};

static inline int mtdev_is_absmt(unsigned int code)
{
	return mtdev_map_abs2mt[code];
}

static inline unsigned int mtdev_abs2mt(unsigned int code)
{
	return mtdev_map_abs2mt[code] - 1;
}

static inline unsigned int mtdev_mt2abs(unsigned int mtcode)
{
	return mtdev_map_mt2abs[mtcode];
}

#endif
