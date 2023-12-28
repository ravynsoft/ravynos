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
#include <time.h>
#include "gp-time.h"

/*  CLOCK_MONOTONIC
    Clock that cannot be set and represents monotonic time since some
             unspecified starting point.  */

static hrtime_t
linux_gethrtime (void)
{
  struct timespec tp;
  hrtime_t rc = 0;

#ifdef CLOCK_MONOTONIC_RAW
  int r = clock_gettime (CLOCK_MONOTONIC_RAW, &tp);
#else
  int r = clock_gettime (CLOCK_MONOTONIC, &tp);
#endif

  if (r == 0)
    rc = ((hrtime_t) tp.tv_sec)*1000000000 + (hrtime_t) tp.tv_nsec;
  return rc;
}

hrtime_t (*__collector_gethrtime)() = linux_gethrtime;
