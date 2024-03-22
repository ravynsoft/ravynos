/*
 * Copyright (C) 2010-2011 Marcin Kościelnicki <koriakin@0x04.net>
 * Copyright (C) 2010 Francisco Jerez <currojerez@riseup.net>
 * All Rights Reserved.
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
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

#define ADDARRAY(a, e) \
	do { \
	if ((a ## num) >= (a ## max)) { \
		if (!(a ## max)) \
			(a ## max) = 16; \
		else \
			(a ## max) *= 2; \
		(a) = realloc((a), (a ## max)*sizeof(*(a))); \
	} \
	(a)[(a ## num)++] = (e); \
	} while(0)

#define FINDARRAY(a, tmp, pred)				\
	({							\
		int __i;					\
								\
		for (__i = 0; __i < (a ## num); __i++) {	\
			tmp = (a)[__i];				\
			if (pred)				\
				break;				\
		}						\
								\
		tmp = ((pred) ? tmp : NULL);			\
	})

/* ceil(log2(x)) */
static inline int clog2(uint64_t x) {
	if (!x)
		return x;
	int r = 0;
	while (x - 1 > (1ull << r) - 1)
		r++;
	return r;
}

#define ARRAY_SIZE(a) (sizeof (a) / sizeof *(a))

#define min(a,b)				\
	({					\
		typeof (a) _a = (a);		\
		typeof (b) _b = (b);		\
		_a < _b ? _a : _b;		\
	})

#define max(a,b)				\
	({					\
		typeof (a) _a = (a);		\
		typeof (b) _b = (b);		\
		_a > _b ? _a : _b;		\
	})

#define CEILDIV(a, b) (((a) + (b) - 1)/(b))

#define extr(a, b, c) ((uint64_t)(a) << (64 - (b) - (c)) >> (64 - (c)))
#define extrs(a, b, c) ((int64_t)(a) << (64 - (b) - (c)) >> (64 - (c))) 
#define sext(a, b) extrs(a, 0, b+1)
#define bflmask(a) ((2ull << ((a)-1)) - 1)
#define insrt(a, b, c, d) ((a) = ((a) & ~(bflmask(c) << (b))) | ((d) & bflmask(c)) << (b))

struct envy_loc {
	int lstart;
	int cstart;
	int lend;
	int cend;
	const char *file;
};

#define LOC_FORMAT(loc, str) "%s:%d.%d-%d.%d: " str, (loc).file, (loc).lstart, (loc).cstart, (loc).lend, (loc).cend

uint32_t elf_hash(const char *str);

FILE *find_in_path(const char *name, const char *path, char **pfullname);

struct astr {
	char *str;
	size_t len;
};

void print_escaped_astr(FILE *out, struct astr *astr);

char *aprintf(const char *format, ...);

#endif
