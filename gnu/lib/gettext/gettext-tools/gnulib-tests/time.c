/* Provide time() for systems for which it's broken.
   Copyright (C) 2023 Free Software Foundation, Inc.

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Written by Bruno Haible.  */

#include <config.h>

/* Specification.  */
#include <time.h>

#include <stdlib.h>
#include <sys/time.h>

time_t
time (time_t *tp)
{
  struct timeval tv;
  time_t tt;

  if (gettimeofday (&tv, NULL) < 0)
    abort ();
  tt = tv.tv_sec;

  if (tp)
    *tp = tt;

  return tt;
}
