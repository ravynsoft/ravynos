/* mstats.h - definitions for malloc statistics */

/*  Copyright (C) 2001-2021 Free Software Foundation, Inc.

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

#ifndef _MSTATS_H
#define _MSTATS_H

#include "imalloc.h"

#ifdef MALLOC_STATS

/* This needs to change if the definition in malloc.c changes */
#ifndef NBUCKETS
#  define NBUCKETS 28
#endif

/*
 * NMALLOC[i] is the difference between the number of mallocs and frees
 * for a given block size.  TMALLOC[i] is the total number of mallocs for
 * a given block size.  NMORECORE[i] is the total number of calls to
 * morecore(i).  NLESSCORE[i] is the total number of calls to lesscore(i).
 *
 * NMAL and NFRE are counts of the number of calls to malloc() and free(),
 * respectively.  NREALLOC is the total number of calls to realloc();
 * NRCOPY is the number of times realloc() had to allocate new memory and
 * copy to it.  NRECURSE is a count of the number of recursive calls to
 * malloc() for the same bucket size, which can be caused by calls to
 * malloc() from a signal handler.
 *
 * NSBRK is the number of calls to sbrk() (whether by morecore() or for
 * alignment); TSBRK is the total number of bytes requested from the kernel
 * with sbrk().
 *
 * BYTESUSED is the total number of bytes consumed by blocks currently in
 * use; BYTESFREE is the total number of bytes currently on all of the free
 * lists.  BYTESREQ is the total number of bytes requested by the caller
 * via calls to malloc() and realloc().
 *
 * TBSPLIT is the number of times a larger block was split to satisfy a
 * smaller request. NSPLIT[i] is the number of times a block of size I was
 * split.
 *
 * TBCOALESCE is the number of times two adjacent smaller blocks off the free
 * list were combined to satisfy a larger request.
 */
struct _malstats {
  int nmalloc[NBUCKETS];
  int tmalloc[NBUCKETS];
  int nmorecore[NBUCKETS];
  int nlesscore[NBUCKETS];
  int nmal;
  int nfre;
  int nrealloc;
  int nrcopy;
  int nrecurse;
  int nsbrk;
  bits32_t tsbrk;
  bits32_t bytesused;
  bits32_t bytesfree;
  u_bits32_t bytesreq;
  int tbsplit;
  int nsplit[NBUCKETS];
  int tbcoalesce;
  int ncoalesce[NBUCKETS];
  int nmmap;
  bits32_t tmmap;
};

/* Return statistics describing allocation of blocks of size BLOCKSIZE.
   NFREE is the number of free blocks for this allocation size.  NUSED
   is the number of blocks in use.  NMAL is the number of requests for
   blocks of size BLOCKSIZE.  NMORECORE is the number of times we had
   to call MORECORE to repopulate the free list for this bucket.
   NLESSCORE is the number of times we gave memory back to the system
   from this bucket.  NSPLIT is the number of times a block of this size
   was split to satisfy a smaller request.  NCOALESCE is the number of
   times two blocks of this size were combined to satisfy a larger
   request. */
struct bucket_stats {
  u_bits32_t blocksize;
  int nfree;
  int nused;
  int nmal;
  int nmorecore;
  int nlesscore;
  int nsplit;
  int ncoalesce;
  int nmmap;		/* currently unused */
};

extern struct bucket_stats malloc_bucket_stats PARAMS((int));
extern struct _malstats malloc_stats PARAMS((void));
extern void print_malloc_stats PARAMS((char *));
extern void trace_malloc_stats PARAMS((char *, char *));

#endif /* MALLOC_STATS */

#endif /* _MSTATS_H */
