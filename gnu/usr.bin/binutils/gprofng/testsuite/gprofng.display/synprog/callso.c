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

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <errno.h>
#include <string.h>
#include "stopwatch.h"


#define DYNSOROUTINE    "so_cputime"
#define DYNSONAME       "./so_syn.so"

/* callso -- dynamically link a shared object, and call a routine in it */

#ifndef NONSHARED

static void *so_object = NULL;
static void closeso (void);

int
callso (int k)
{
  int i;
  char buf[1024];
  char *p;
  char *q = DYNSONAME;
  int errnum;

  hrtime_t start = gethrtime ();
  hrtime_t vstart = gethrvtime ();

  /* Log the event */
  wlog ("start of callso", NULL);

  /* see if already linked */
  if (so_object != NULL)
    {
      fprintf (stderr, "Execution error -- callso: so_object already linked\n");
      return 0;
    }

  /* open the dynamic shared object */
  so_object = NULL;
  while (so_object == NULL)
    {
      so_object = dlopen (DYNSONAME, RTLD_NOW);
      if (so_object != NULL)
        break;
      p = dlerror ();
      if (q == NULL)
        q = "DYNSONAME";
      if (p == NULL)
        p = "dlerror() returns NULL";
      errnum = errno;
      if (errnum == EINTR)
        continue;   /* retry */
      else
        {
          fprintf (stderr, "Execution error -- callso: dlopen of %s failed--%s, errno=%d (%s)\n",
                   q, p, errnum, strerror (errnum));
          return (0);
        }
    }

  /* look up the routine name in it */
  int (*so_routine)() = (int (*)())dlsym (so_object, DYNSOROUTINE);
  if (so_routine == NULL)
    {
      fprintf (stderr, "Execution error -- callso: dlsym %s not found\n",
               DYNSOROUTINE);
      return (0);
    }

  /* invoke the routine */
  long long count = 0;
  do
    {
      i = (*so_routine)();
      count++;
    }
  while (start + testtime * 1e9 > gethrtime ());

  closeso ();
  sprintf (buf, "end of callso, %s returned %d", DYNSOROUTINE, i);
  wlog (buf, NULL);
  fprintf (stderr, "   Performed %lld while-loop iterations\n", count);
  whrvlog ((gethrtime () - start), (gethrvtime () - vstart), "callso", NULL);
  return 0;
}

/* closeso -- close a DSO */
void
closeso (void)
{
  /* Log the event */
  wlog ("start of closeso", NULL);

  /* ensure already linked */
  if (so_object == NULL)
    {
      fprintf (stderr, "Execution error -- closeso: so_object not linked\n");
      return;
    }

  /* close the dynamic shared object */
  int rc = dlclose (so_object);
  if (rc != 0)
    {
      fprintf (stderr, "Execution error -- closeso: dlclose() failed--%s\n",
               dlerror ());
      return;
    }

  /* clear the pointer */
  so_object = NULL;
  wlog ("end of closeso", NULL);
  return;
}

#else /* NONSHARED */

int
callso (int i)
{
  return 0;
}

void
closeso (void)
{
  return;
}
#endif /* NONSHARED */
