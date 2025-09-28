// SPDX-License-Identifier: MIT
/*
 * Copyright © 2013 Red Hat, Inc.
 */

#ifndef _UTIL_H_
#define _UTIL_H_

#include "config.h"
#include <stdbool.h>
#include <string.h>

#define LONG_BITS (sizeof(long) * 8)
#define NLONGS(x) (((x) + LONG_BITS - 1) / LONG_BITS)
#define ARRAY_LENGTH(a) (sizeof(a) / (sizeof((a)[0])))
#define unlikely(x) (__builtin_expect(!!(x),0))

#undef min
#undef max
#ifdef __GNUC__
#define min(a,b) \
		({ __typeof__ (a) _a = (a); \
	          __typeof__ (b) _b = (b); \
		_a > _b ? _b : _a; \
		})
#define max(a,b) \
		({ __typeof__ (a) _a = (a); \
	          __typeof__ (b) _b = (b); \
		_a > _b ? _a : _b; \
		})
#else
#define min(a,b) ((a) > (b) ? (b) : (a))
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif

static inline bool
startswith(const char *str, size_t len, const char *prefix, size_t plen)
{
	return len >= plen && !strncmp(str, prefix, plen);
}

static inline int
bit_is_set(const unsigned long *array, int bit)
{
    return !!(array[bit / LONG_BITS] & (1LL << (bit % LONG_BITS)));
}

static inline void
set_bit(unsigned long *array, int bit)
{
    array[bit / LONG_BITS] |= (1LL << (bit % LONG_BITS));
}

static inline void
clear_bit(unsigned long *array, int bit)
{
    array[bit / LONG_BITS] &= ~(1LL << (bit % LONG_BITS));
}

static inline void
set_bit_state(unsigned long *array, int bit, int state)
{
	if (state)
		set_bit(array, bit);
	else
		clear_bit(array, bit);
}

#endif
