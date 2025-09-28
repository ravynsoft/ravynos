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


#define DYNSOROUTINE "sx_cputime"
#define DYNSONAME "./so_syx.so"

/* callsx -- dynamically link a shared object, and call a routine in it */

#ifndef NONSHARED

static void *sx_object = NULL;
static void closesx (void);

int
callsx (int k)
{
  int i;
  char buf[1024];
  char *p;
  char *q = DYNSONAME;
  int errnum;

  hrtime_t start = gethrtime ();
  hrtime_t vstart = gethrvtime ();

  /* Log the event */
  wlog ("start of callsx", NULL);

  /* see if already linked */
  if (sx_object != NULL)
    {
      fprintf (stderr, "Execution error -- callsx: sx_object already linked\n");
      return 0;
    }

  /* open the dynamic shared object */
  /* open the dynamic shared object */
  sx_object = NULL;
  while (sx_object == NULL)
    {
      sx_object = dlopen (DYNSONAME, RTLD_NOW);
      if (sx_object != NULL)
        break;
      p = dlerror ();
      if (q == NULL) q = "DYNSONAME";
      if (p == NULL) p = "dlerror() returns NULL";
      errnum = errno;
      if (errnum == EINTR)
        continue;  /* retry */
      else
        {
          fprintf (stderr, "Execution error -- callso: dlopen of %s failed--%s, errno=%d (%s)\n",
                   q, p, errnum, strerror (errnum));
          return (0);
        }
    }

  /* look up the routine name in it */
  int (*sx_routine)() = (int (*)())dlsym (sx_object, DYNSOROUTINE);
  if (sx_routine == NULL)
    {
      fprintf (stderr, "Execution error -- callsx: dlsym %s not found\n",
               DYNSOROUTINE);
      return (0);
    }

  /* invoke the routine */
  long long count = 0;
  do
    {
      i = (*sx_routine)();
      count++;
    }
  while (start + testtime * 1e9 > gethrtime ());

  closesx ();
  sprintf (buf, "end of callsx, %s returned %d", DYNSOROUTINE, i);
  wlog (buf, NULL);
  fprintf (stderr, "   Performed %lld while-loop iterations\n", count);
  whrvlog ((gethrtime () - start), (gethrvtime () - vstart), "callsx", NULL);
  return 0;
}

/* closesx -- close a DSO */
void
closesx (void)
{
  /* Log the event */
  wlog ("start of closesx", NULL);

  /* ensure already linked */
  if (sx_object == NULL)
    {
      fprintf (stderr, "Execution error -- closesx: sx_object not linked\n");
      return;
    }

#if 0
  /* close the dynamic shared object */
  rc = dlclose (sx_object);
  if (rc != 0)
    {
      fprintf (stderr, "Execution error -- closesx: dlclose() failed--%s\n",
               dlerror ());
      return;
    }
  /* clear the pointer */
  sx_object = NULL;
#endif
  wlog ("end of closesx", NULL);
  return;
}

#else /* NONSHARED */

int
callsx (int i)
{
  return 0;
}

void
closesx (void)
{
  return;
}
#endif /* NONSHARED */
