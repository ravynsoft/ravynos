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

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "stopwatch.h"

/*=======================================================*/

/* pagethrash - allocate some memory, and thrash around in it */
int
pagethrash (int thrashmb)
{
  char buf[1024];

  hrtime_t start = gethrtime ();
  hrtime_t vstart = gethrvtime ();

  /* Log the event */
  wlog ("start of pagethrash", NULL);

  /* Start a stopwatch */
  stopwatch_t *w = stpwtch_alloc ("pagethrash", 0);

  /* compute the size */
  unsigned long size = thrashmb * 1024 * 1024;
  int pagesize = getpagesize ();
  void *space = malloc (size + pagesize);
  if (space == NULL)
    {
      fprintf (stderr, "\tpagethrash failed; can't get %ld bytes.\n", size);
      exit (1);
    }

  /* round address to page boundary */
  unsigned long loc = (((unsigned long) space + pagesize - 1) & ~(pagesize - 1));
  long npages = size / pagesize;

  /* touch all the pages to force them in */
  for (long i = 0; i < npages; i++)
    {
      stpwtch_start (w);
      *(int *) (loc + i * pagesize) = i;
      stpwtch_stop (w);
    }

  /* now free up the space */
  free (space);

  /* print the timing results */
  stpwtch_print (w);
  free ((void *) w);

  sprintf (buf, "pagethrash: %ld pages", npages);
  whrvlog (gethrtime () - start, gethrvtime () - vstart, buf, NULL);
  return 0;
}
