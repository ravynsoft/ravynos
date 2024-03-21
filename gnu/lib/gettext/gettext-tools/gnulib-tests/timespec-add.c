/* Add two struct timespec values.

   Copyright (C) 2011-2023 Free Software Foundation, Inc.

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

/* Written by Paul Eggert.  */

/* Return the sum of two timespec values A and B.  On overflow, return
   an extremal value.  This assumes 0 <= tv_nsec < TIMESPEC_HZ.  */

#include <config.h>
#include "timespec.h"

#include <stdckdint.h>
#include "intprops.h"

struct timespec
timespec_add (struct timespec a, struct timespec b)
{
  time_t rs = a.tv_sec;
  time_t bs = b.tv_sec;
  int ns = a.tv_nsec + b.tv_nsec;
  int nsd = ns - TIMESPEC_HZ;
  int rns = ns;

  if (0 <= nsd)
    {
      rns = nsd;
      time_t bs1;
      if (!ckd_add (&bs1, bs, 1))
        bs = bs1;
      else if (rs < 0)
        rs++;
      else
        goto high_overflow;
    }

  if (ckd_add (&rs, rs, bs))
    {
      if (bs < 0)
        {
          rs = TYPE_MINIMUM (time_t);
          rns = 0;
        }
      else
        {
        high_overflow:
          rs = TYPE_MAXIMUM (time_t);
          rns = TIMESPEC_HZ - 1;
        }
    }

  return make_timespec (rs, rns);
}
