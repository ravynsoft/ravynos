/*
 * Copyright (C) 2005, 2007, 2009-2023 Free Software Foundation, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Written by Jim Meyering and Bruno Haible.  */

#include <config.h>

#include <sys/time.h>

#include "signature.h"
SIGNATURE_CHECK (gettimeofday, int,
                 (struct timeval *, GETTIMEOFDAY_TIMEZONE *));

#include <time.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "macros.h"

static void
test_clobber ()
{
  time_t t = 0;
  struct tm *lt;
  struct tm saved_lt;
  struct timeval tv;
  lt = localtime (&t);
  saved_lt = *lt;
  gettimeofday (&tv, NULL);
  if (memcmp (lt, &saved_lt, sizeof (struct tm)) != 0)
    {
      fprintf (stderr, "gettimeofday still clobbers the localtime buffer!\n");
      exit (1);
    }
}

static void
test_consistency ()
{
  struct timeval tv1;
  time_t tt2;
  struct timeval tv3;
  time_t tt4;

  ASSERT (gettimeofday (&tv1, NULL) == 0);
  tt2 = time (NULL);
  ASSERT (gettimeofday (&tv3, NULL) == 0);
  tt4 = time (NULL);

  /* Verify monotonicity of gettimeofday().  */
  ASSERT (tv1.tv_sec < tv3.tv_sec
          || (tv1.tv_sec == tv3.tv_sec && tv1.tv_usec <= tv3.tv_usec));

  /* Verify monotonicity of time().  */
  ASSERT (tt2 <= tt4);

  /* Verify that the tv_sec field of the result is the same as time(NULL).  */
  /* Note: It's here that the dependency to the 'time' module is needed.
     Without it, this assertion would sometimes fail on glibc systems, see
     https://sourceware.org/bugzilla/show_bug.cgi?id=30200  */
  ASSERT (tv1.tv_sec <= tt2);
  ASSERT (tt2 <= tv3.tv_sec);
  ASSERT (tv3.tv_sec <= tt4);
}

int
main (void)
{
  test_clobber ();
  test_consistency ();

  return 0;
}
