/* table.h - definitions for tables for keeping track of allocated memory */

/*  Copyright (C) 2001-2020 Free Software Foundation, Inc.

    This file is part of GNU Bash, the Bourne-Again SHell.

   Bash is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Bash is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Bash.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _MTABLE_H
#define _MTABLE_H

#include "imalloc.h"

#ifdef MALLOC_REGISTER

/* values for flags byte. */
#define MT_ALLOC	0x01
#define MT_FREE		0x02

/*
 * Memory table entry.
 *
 * MEM is the address of the allocated pointer.
 * SIZE is the requested allocation size.
 * FLAGS includes either MT_ALLOC (MEM is allocated) or MT_FREE (MEM is
 * not allocated).  Other flags later.
 * FUNC is set to the name of the function doing the allocation (from the
 * `tag' argument to register_alloc().
 * FILE and LINE are the filename and line number of the last allocation
 * and free (depending on STATUS) of MEM.
 * NALLOC and NFREE are incremented on each allocation that returns MEM or
 * each free of MEM, respectively (way to keep track of memory reuse
 * and how well the free lists are working).
 *
 */
typedef struct mr_table {
	PTR_T mem;
	size_t size;
	char flags;
	const char *func;
	const char *file;
	int line;
	int nalloc, nfree;
} mr_table_t;

#define REG_TABLE_SIZE	8192

extern mr_table_t *mr_table_entry PARAMS((PTR_T));
extern void mregister_alloc PARAMS((const char *, PTR_T, size_t, const char *, int));
extern void mregister_free PARAMS((PTR_T, int, const char *, int));
extern void mregister_describe_mem ();
extern void mregister_dump_table PARAMS((void));
extern void mregister_table_init PARAMS((void));

typedef struct ma_table {
	const char *file;
	int line;
	int nalloc;
} ma_table_t;

extern void mlocation_register_alloc PARAMS((const char *, int));
extern void mlocation_table_init PARAMS((void));
extern void mlocation_dump_table PARAMS((void));
extern void mlocation_write_table PARAMS((void));

/* NOTE:  HASH_MIX taken from dmalloc (http://dmalloc.com) */

/*
 * void HASH_MIX
 *
 * DESCRIPTION:
 *
 * Mix 3 32-bit values reversibly.  For every delta with one or two
 * bits set, and the deltas of all three high bits or all three low
 * bits, whether the original value of a,b,c is almost all zero or is
 * uniformly distributed.
 *
 * If HASH_MIX() is run forward or backward, at least 32 bits in a,b,c
 * have at least 1/4 probability of changing.  If mix() is run
 * forward, every bit of c will change between 1/3 and 2/3 of the
 * time.  (Well, 22/100 and 78/100 for some 2-bit deltas.)
 *
 * HASH_MIX() takes 36 machine instructions, but only 18 cycles on a
 * superscalar machine (like a Pentium or a Sparc).  No faster mixer
 * seems to work, that's the result of my brute-force search.  There
 * were about 2^68 hashes to choose from.  I only tested about a
 * billion of those.
 */
#define HASH_MIX(a, b, c) \
 do { \
   a -= b; a -= c; a ^= (c >> 13); \
   b -= c; b -= a; b ^= (a << 8); \
   c -= a; c -= b; c ^= (b >> 13); \
   a -= b; a -= c; a ^= (c >> 12); \
   b -= c; b -= a; b ^= (a << 16); \
   c -= a; c -= b; c ^= (b >> 5); \
   a -= b; a -= c; a ^= (c >> 3); \
   b -= c; b -= a; b ^= (a << 10); \
   c -= a; c -= b; c ^= (b >> 15); \
 } while(0)

#endif /* MALLOC_REGISTER */

#endif /* _MTABLE_H */
