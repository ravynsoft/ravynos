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
#include <pthread.h>

#include "collector.h"
#include "libcol_util.h"
#include "tsd.h"
#include "memmgr.h"

/* TprintfT(<level>,...) definitions.  Adjust per module as needed */
#define DBG_LT0 0 // for high-level configuration, unexpected errors/warnings
#define DBG_LT1 1 // for configuration details, warnings
#define DBG_LT2 2
#define DBG_LT3 3

/*
 * Build our thread-specific-data support on pthread interfaces.
 */
#define MAXNKEYS    64  /* hard-wired? really? well, it depends only on us and we have a sense for how many keys we will use */
static pthread_key_t tsd_pkeys[MAXNKEYS];
static size_t tsd_sizes[MAXNKEYS];
static unsigned tsd_nkeys = 0;

int
__collector_tsd_init ()
{
  return 0;
}

void
__collector_tsd_fini ()
{
  Tprintf (DBG_LT1, "tsd_fini()\n");
  while (tsd_nkeys)
    {
      tsd_nkeys--;
      pthread_key_delete (tsd_pkeys[tsd_nkeys]);
      tsd_sizes[tsd_nkeys] = 0; // should be unneeded
    }
}

int
__collector_tsd_allocate ()
{
  return 0;
}

void
__collector_tsd_release () { }

static void
tsd_destructor (void *p)
{
  if (p)
    __collector_freeCSize (__collector_heap, p, *((size_t *) p));
}

unsigned
__collector_tsd_create_key (size_t sz, void (*init)(void*), void (*fini)(void*))
{
  /*
   * We no longer support init and fini arguments (and weren't using them anyhow).
   * Our hard-wired MAXNKEYS presumably is considerably higher than the number of keys we use.
   */
  if (init || fini || (tsd_nkeys >= MAXNKEYS))
    return COLLECTOR_TSD_INVALID_KEY;

  /*
   * A pthread key has a value that is (void *).
   * We don't know where it is stored, and can access its value only through {get|set}specific.
   * But libcollector expects a pointer to memory that it can modify.
   * So we have to allocate that memory and store the pointer.
   *
   * For now, we just have to register a destructor that will free the memory
   * when the thread finishes.
   */
  if (pthread_key_create (&tsd_pkeys[tsd_nkeys], &tsd_destructor))
   return COLLECTOR_TSD_INVALID_KEY;
  tsd_sizes[tsd_nkeys] = sz;
  tsd_nkeys++;
  return (tsd_nkeys - 1);
}

void *
__collector_tsd_get_by_key (unsigned key_index)
{
  if (key_index == COLLECTOR_TSD_INVALID_KEY)
    return NULL;
  if (key_index < 0 || key_index >= tsd_nkeys)
    return NULL;
  pthread_key_t key = tsd_pkeys[key_index];
  size_t sz = tsd_sizes[key_index];

  /*
   * When we use __collector_freeCSize(), we need to know the
   * size that had been allocated.  So, stick a header to the
   * front of the allocation to hold the size.  The header could
   * just be sizeof(size_t), but pad it to preserve alignment for
   * the usable area.
   */
  size_t header = 8;
  void *value = pthread_getspecific (key);

  // check whether we have allocated the memory
  if (value == NULL)
    {
      // add room to record the size
      value = __collector_allocCSize (__collector_heap, sz + header, 0);
      if (value == NULL)
	{
	  // do we need to guard against trying to alloc each time?
	  return NULL;
	}
      // write the size of the allocation
      *((size_t *) value) = sz + header;
      CALL_UTIL (memset)(((char *) value) + header, 0, sz);

      // record the allocation for future retrieval
      if (pthread_setspecific (key, value))
	return NULL;
    }
  // return the pointer, skipping the header
  return ((char *) value) +header;
}

void
__collector_tsd_fork_child_cleanup ()
{
  __collector_tsd_fini ();
}
