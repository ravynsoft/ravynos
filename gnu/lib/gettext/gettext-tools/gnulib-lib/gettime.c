/* gettime -- get the system clock

   Copyright (C) 2002, 2004-2007, 2009-2023 Free Software Foundation, Inc.

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation, either version 3 of the
   License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Written by Paul Eggert.  */

#include <config.h>

#include "timespec.h"

#include <sys/time.h>

/* Get the system time into *TS.  */

void
gettime (struct timespec *ts)
{
#if defined CLOCK_REALTIME && HAVE_CLOCK_GETTIME
  clock_gettime (CLOCK_REALTIME, ts);
#elif defined HAVE_TIMESPEC_GET
  timespec_get (ts, TIME_UTC);
#else
  struct timeval tv;
  gettimeofday (&tv, NULL);
  *ts = (struct timespec) { .tv_sec  = tv.tv_sec,
                            .tv_nsec = tv.tv_usec * 1000 };
#endif
}

/* Return the current system time as a struct timespec.  */

struct timespec
current_timespec (void)
{
  struct timespec ts;
  gettime (&ts);
  return ts;
}
