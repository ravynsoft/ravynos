/* Test of time() function.
   Copyright (C) 2023 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Written by Bruno Haible.  */

#include <config.h>

#include <time.h>

#include "signature.h"
SIGNATURE_CHECK (time, time_t, (time_t *));

#include <sys/time.h>

#include "macros.h"

int
main (void)
{
  /* Check consistency of time() with gettimeofday().tv_sec.  */
  struct timeval tv1;
  struct timeval tv2;
  time_t tt3;

  /* Wait until gettimeofday() reports an increase in tv_sec.  */
  ASSERT (gettimeofday (&tv1, NULL) == 0);
  do
    ASSERT (gettimeofday (&tv2, NULL) == 0);
  while (tv2.tv_sec == tv1.tv_sec);
  /* We are now at the beginning of a second.  Test whether time() reports
     the new second or the previous one.  */
  tt3 = time (NULL);
  ASSERT (tt3 >= tv2.tv_sec);

  return 0;
}
