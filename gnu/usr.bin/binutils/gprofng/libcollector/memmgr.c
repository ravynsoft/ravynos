/* Copyright (C) 2021-2023 Free Software Foundation, Inc.
   Contributed by Oracle.

   This file is part of GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "config.h"
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "collector.h"
#include "libcol_util.h"
#include "gp-experiment.h"
#include "memmgr.h"

/* TprintfT(<level>,...) definitions.  Adjust per module as needed */
#define DBG_LT0 0 // for high-level configuration, unexpected errors/warnings
#define DBG_LT1 1 // for configuration details, warnings
#define DBG_LT2 2
#define DBG_LT3 3
#define DBG_LT4 4

/*
 * Memory allocation.
 *
 * Heap:
 *    chain[0] - linked list of chunks;
 *    chain[1] - linked list of free 16-byte objects;
 *    chain[2] - linked list of free 32-byte objects;
 *    ...
 *
 * Chunk:
 *
 * base               lo        hi
 * V                  V         V
 * +------------------+---------+-------------------+--+--+-----+
 * | Var size object  | ->    <-| Const size objects|  |  |Chunk|
 * +------------------+---------+-------------------+--+--+-----+
 *
 * Limitations:
 *   - one var size object per chunk
 *   - can't allocate const size objects larger than 2^MAXCHAIN
 */

#define MAXCHAIN    32
#define ALIGNMENT    4   /* 2^ALIGNMENT == minimal size and alignment */
#define ALIGN(x)    ((((x) - 1)/(1 << ALIGNMENT) + 1) * (1 << ALIGNMENT))

struct Heap
{
  collector_mutex_t lock;   /* master lock */
  void *chain[MAXCHAIN];    /* chain[0] - chunks */
			    /* chain[i] - structs of size 2^i */
};

typedef struct Chunk
{
  size_t size;
  char *base;
  char *lo;
  char *hi;
  struct Chunk *next;
} Chunk;

static void
not_implemented ()
{
  __collector_log_write ("<event kind=\"%s\" id=\"%d\">error memmgr not_implemented()</event>\n",
			 SP_JCMD_CERROR, COL_ERROR_NOZMEM);
  return;
}

/*
 * void __collector_mmgr_init_mutex_locks( Heap *heap )
 *      Iinitialize mmgr mutex locks.
 */
void
__collector_mmgr_init_mutex_locks (Heap *heap)
{
  if (heap == NULL)
    return;
  if (__collector_mutex_trylock (&heap->lock))
    {
      /*
       * We are in a child process immediately after the fork().
       * Parent process was in the middle of critical section when the fork() happened.
       * This is a placeholder for the cleanup.
       * See CR 6997020 for details.
       */
      __collector_mutex_init (&heap->lock);
    }
  __collector_mutex_init (&heap->lock);
}

/*
 * alloc_chunk( unsigned sz ) allocates a chunk of at least sz bytes.
 * If sz == 0, allocates a chunk of the default size.
 */
static Chunk *
alloc_chunk (unsigned sz, int log)
{
  static long pgsz = 0;
  char *ptr;
  Chunk *chnk;
  size_t chunksz;
  if (pgsz == 0)
    {
      pgsz = CALL_UTIL (sysconf)(_SC_PAGESIZE);
      Tprintf (DBG_LT2, "memmgr: pgsz = %ld (0x%lx)\n", pgsz, pgsz);
    }
  /* Allocate 2^n >= sz bytes */
  unsigned nsz = ALIGN (sizeof (Chunk)) + sz;
  for (chunksz = pgsz; chunksz < nsz; chunksz *= 2);
  if (log == 1)
    Tprintf (DBG_LT2, "alloc_chunk mapping %u, rounded up from %u\n", (unsigned int) chunksz, sz);
  /* mmap64 is only in 32-bits; this call goes to mmap in 64-bits */
  ptr = (char*) CALL_UTIL (mmap64_)(0, chunksz, PROT_READ | PROT_WRITE,
				   MAP_PRIVATE | MAP_ANON, (int) -1, (off64_t) 0);
  if (ptr == MAP_FAILED)
    {
      Tprintf (0, "alloc_chunk mapping failed COL_ERROR_NOZMEMMAP: %s\n", CALL_UTIL (strerror)(errno));
      __collector_log_write ("<event kind=\"%s\" id=\"%d\" ec=\"%d\">%s</event>\n",
			     SP_JCMD_CERROR, COL_ERROR_NOZMEMMAP, errno, "0");
      return NULL;
    }
  /* Put the chunk descriptor at the end of the chunk */
  chnk = (Chunk*) (ptr + chunksz - ALIGN (sizeof (Chunk)));
  chnk->size = chunksz;
  chnk->base = ptr;
  chnk->lo = chnk->base;
  chnk->hi = (char*) chnk;
  chnk->next = (Chunk*) NULL;
  if (log == 1)
    Tprintf (DBG_LT2, "memmgr: returning new chunk @%p, chunksx=%ld sz=%ld\n",
	     ptr, (long) chunksz, (long) sz);
  return chnk;
}

Heap *
__collector_newHeap ()
{
  Heap *heap;
  Chunk *chnk;
  Tprintf (DBG_LT2, "__collector_newHeap calling alloc_chunk(0)\n");
  chnk = alloc_chunk (0, 1);
  if (chnk == NULL)
    return NULL;

  /* A bit of hackery: allocate heap from its own chunk */
  chnk->hi -= ALIGN (sizeof (Heap));
  heap = (Heap*) chnk->hi;
  heap->chain[0] = (void*) chnk;
  __collector_mutex_init (&heap->lock);
  return heap;
}

void
__collector_deleteHeap (Heap *heap)
{
  if (heap == NULL)
    return;
  /* Note: heap itself is in the last chunk */
  for (Chunk *chnk = heap->chain[0]; chnk;)
    {
      Chunk *next = chnk->next;
      CALL_UTIL (munmap)((void*) chnk->base, chnk->size);
      chnk = next;
    }
}

void *
__collector_allocCSize (Heap *heap, unsigned sz, int log)
{
  void *res;
  Chunk *chnk;
  if (heap == NULL)
    return NULL;

  /* block all signals and acquire lock */
  sigset_t old_mask, new_mask;
  CALL_UTIL (sigfillset)(&new_mask);
  CALL_UTIL (sigprocmask)(SIG_SETMASK, &new_mask, &old_mask);
  __collector_mutex_lock (&heap->lock);

  /* Allocate nsz = 2^idx >= sz bytes */
  unsigned idx = ALIGNMENT;
  unsigned nsz = 1 << idx;
  while (nsz < sz)
    nsz = 1 << ++idx;

  /* Look in the corresponding chain first */
  if (idx < MAXCHAIN)
    {
      if (heap->chain[idx] != NULL)
	{
	  res = heap->chain[idx];
	  heap->chain[idx] = *(void**) res;
	  __collector_mutex_unlock (&heap->lock);
	  CALL_UTIL (sigprocmask)(SIG_SETMASK, &old_mask, NULL);
	  if (log == 1)
	    Tprintf (DBG_LT2, "memmgr: allocCSize %p sz %d (0x%x) req = 0x%x, from chain idx = %d\n", res, nsz, nsz, sz, idx);
	  return res;
	}
    }
  else
    {
      not_implemented ();
      __collector_mutex_unlock (&heap->lock);
      CALL_UTIL (sigprocmask)(SIG_SETMASK, &old_mask, NULL);
      return NULL;
    }

  /* Chain is empty, allocate from chunks */
  for (chnk = (Chunk*) heap->chain[0]; chnk; chnk = chnk->next)
    if (chnk->lo + nsz < chnk->hi)
      break;
  if (chnk == NULL)
    {
      /* Get a new chunk */
      if (log == 1)
	Tprintf (DBG_LT2, "__collector_allocCSize (%u) calling alloc_chunk(%u)\n", sz, nsz);
      chnk = alloc_chunk (nsz, 1);
      if (chnk == NULL)
	{
	  __collector_mutex_unlock (&heap->lock);
	  CALL_UTIL (sigprocmask)(SIG_SETMASK, &old_mask, NULL);
	  return NULL;
	}
      chnk->next = (Chunk*) heap->chain[0];
      heap->chain[0] = chnk;
    }

  /* Allocate from the chunk */
  chnk->hi -= nsz;
  res = (void*) chnk->hi;
  __collector_mutex_unlock (&heap->lock);
  CALL_UTIL (sigprocmask)(SIG_SETMASK, &old_mask, NULL);
  if (log == 1)
    Tprintf (DBG_LT2, "memmgr: allocCSize %p sz %d (0x%x) req = 0x%x, new chunk\n", res, nsz, nsz, sz);
  return res;
}

void
__collector_freeCSize (Heap *heap, void *ptr, unsigned sz)
{
  if (heap == NULL || ptr == NULL)
    return;

  /* block all signals and acquire lock */
  sigset_t old_mask, new_mask;
  CALL_UTIL (sigfillset)(&new_mask);
  CALL_UTIL (sigprocmask)(SIG_SETMASK, &new_mask, &old_mask);
  __collector_mutex_lock (&heap->lock);

  /* Free 2^idx >= sz bytes */
  unsigned idx = ALIGNMENT;
  unsigned nsz = 1 << idx;
  while (nsz < sz)
    nsz = 1 << ++idx;
  if (idx < MAXCHAIN)
    {
      *(void**) ptr = heap->chain[idx];
      heap->chain[idx] = ptr;
    }
  else
    not_implemented ();
  __collector_mutex_unlock (&heap->lock);
  CALL_UTIL (sigprocmask)(SIG_SETMASK, &old_mask, NULL);
  Tprintf (DBG_LT4, "memmgr: freeC %p sz %ld\n", ptr, (long) sz);
}

static void *
allocVSize_nolock (Heap *heap, unsigned sz)
{
  void *res;
  Chunk *chnk;
  if (sz == 0)
    return NULL;

  /* Find a good chunk */
  for (chnk = (Chunk*) heap->chain[0]; chnk; chnk = chnk->next)
    if (chnk->lo == chnk->base && chnk->lo + sz < chnk->hi)
      break;
  if (chnk == NULL)
    {
      /* Get a new chunk */
      Tprintf (DBG_LT2, "allocVsize_nolock calling alloc_chunk(%u)\n", sz);
      chnk = alloc_chunk (sz, 0);
      if (chnk == NULL)
	return NULL;
      chnk->next = (Chunk*) heap->chain[0];
      heap->chain[0] = chnk;
    }
  chnk->lo = chnk->base + sz;
  res = (void*) (chnk->base);
  Tprintf (DBG_LT4, "memmgr: allocV %p for %ld\n", res, (long) sz);
  return res;
}

void *
__collector_allocVSize (Heap *heap, unsigned sz)
{
  void *res;
  if (heap == NULL)
    return NULL;

  /* block all signals and acquire lock */
  sigset_t old_mask, new_mask;
  CALL_UTIL (sigfillset)(&new_mask);
  CALL_UTIL (sigprocmask)(SIG_SETMASK, &new_mask, &old_mask);
  __collector_mutex_lock (&heap->lock);
  res = allocVSize_nolock (heap, sz);
  __collector_mutex_unlock (&heap->lock);
  CALL_UTIL (sigprocmask)(SIG_SETMASK, &old_mask, NULL);
  return res;
}

/*
 *  reallocVSize( Heap *heap, void *ptr, unsigned newsz )
 *  Changes the size of memory pointed by ptr to newsz.
 *  If ptr == NULL, allocates new memory of size newsz.
 *  If newsz == 0, frees ptr and returns NULL.
 */
void *
__collector_reallocVSize (Heap *heap, void *ptr, unsigned newsz)
{
  Chunk *chnk;
  void *res;
  if (heap == NULL)
    return NULL;
  if (ptr == NULL)
    return __collector_allocVSize (heap, newsz);

  /* block all signals and acquire lock */
  sigset_t old_mask, new_mask;
  CALL_UTIL (sigfillset)(&new_mask);
  CALL_UTIL (sigprocmask)(SIG_SETMASK, &new_mask, &old_mask);
  __collector_mutex_lock (&heap->lock);

  /* Find its chunk */
  for (chnk = (Chunk*) heap->chain[0]; chnk; chnk = chnk->next)
    if (ptr == chnk->base)
      break;
  if (chnk == NULL)
    {
      /* memory corrpution */
      not_implemented ();
      __collector_mutex_unlock (&heap->lock);
      CALL_UTIL (sigprocmask)(SIG_SETMASK, &old_mask, NULL);
      return NULL;
    }
  if (chnk->base + newsz < chnk->hi)
    {
      /* easy case */
      chnk->lo = chnk->base + newsz;
      res = newsz ? chnk->base : NULL;
      __collector_mutex_unlock (&heap->lock);
      CALL_UTIL (sigprocmask)(SIG_SETMASK, &old_mask, NULL);
      Tprintf (DBG_LT4, "memmgr: reallocV %p for %ld\n", ptr, (long) newsz);
      return res;
    }
  res = allocVSize_nolock (heap, newsz);
  /* Copy to new location */
  if (res)
    {
      int size = chnk->lo - chnk->base;
      if (newsz < size)
	size = newsz;
      char *s1 = (char*) res;
      char *s2 = chnk->base;
      while (size--)
	*s1++ = *s2++;
    }
  /* Free old memory*/
  chnk->lo = chnk->base;
  __collector_mutex_unlock (&heap->lock);
  CALL_UTIL (sigprocmask)(SIG_SETMASK, &old_mask, NULL);
  return res;
}
