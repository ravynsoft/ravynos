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
#include <dlfcn.h>
#include "util.h"

#define CHECK_OUT_OF_MEM(ptr, size) if (ptr == NULL) err_out_of_memory(size)

/* Report Out of Memory error and exit */
static void
err_out_of_memory (unsigned nbytes)
{
  char *nm = get_prog_name (1);
  if (nm)
    fprintf (stderr, GTXT ("%s: Error: Memory capacity exceeded.\n"), nm);
  else
    fprintf (stderr, GTXT ("Error: Memory capacity exceeded.\n"));
  fprintf (stderr, GTXT ("  Requested %u bytes.\n"), nbytes);
  exit (16);
}

#define CALL_REAL(x) (__real_##x)
#define NULL_PTR(x) ( __real_##x == NULL )

static void *(*__real_malloc)(size_t) = NULL;
static void (*__real_free)(void *) = NULL;
static void *(*__real_realloc)(void *, size_t) = NULL;
static void *(*__real_calloc)(size_t, size_t) = NULL;
static char *(*__real_strdup)(const char*) = NULL;
static volatile int in_init = 0;

static int
init_heap_intf ()
{
  in_init = 1;
  __real_malloc = (void*(*)(size_t))dlsym (RTLD_NEXT, "malloc");
  __real_free = (void(*)(void *))dlsym (RTLD_NEXT, "free");
  __real_realloc = (void*(*)(void *, size_t))dlsym (RTLD_NEXT, "realloc");
  __real_calloc = (void*(*)(size_t, size_t))dlsym (RTLD_NEXT, "calloc");
  __real_strdup = (char*(*)(const char*))dlsym (RTLD_NEXT, "strdup");
  in_init = 0;
  return 0;
}

/* --------------------------------------------------------------------------- */
/* libc's memory management functions substitutions */

/* Allocate memory and make sure we got some */
void *
malloc (size_t size)
{
  if (NULL_PTR (malloc))
    init_heap_intf ();
  void *ptr = CALL_REAL (malloc)(size);
  CHECK_OUT_OF_MEM (ptr, size);
  return ptr;
}


/* Implement a workaround for a libdl recursion problem */
void *
calloc (size_t nelem, size_t size)
{
  if (NULL_PTR (calloc))
    {
      /* If a program is linked with libpthread then the following
       * calling sequence occurs:
       * init_heap_intf -> dlsym -> calloc -> malloc -> init_heap_intf
       * We break some performance improvement in libdl by returning
       * NULL but preserve functionality.
       */
      if (in_init)
	return NULL;
      init_heap_intf ();
    }
  return CALL_REAL (calloc)(nelem, size);
}

/* Free the storage associated with data */
void
free (void *ptr)
{
  if (ptr == NULL)
    return;
  if (NULL_PTR (free))
    init_heap_intf ();
  CALL_REAL (free)(ptr);
  return;
}

/* Reallocate buffer */
void *
realloc (void *ptr, size_t size)
{
  if (NULL_PTR (realloc))
    init_heap_intf ();
  ptr = CALL_REAL (realloc)(ptr, size);
  CHECK_OUT_OF_MEM (ptr, size);
  return ptr;
}
