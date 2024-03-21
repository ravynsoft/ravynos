/* Test timespec functions.
   Copyright 2015-2023 Free Software Foundation, Inc.

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

#include <config.h>

#include "timespec.h"

#include "intprops.h"
#include "macros.h"

#include <limits.h>

static struct { int s; int ns; } const prototype[] =
  {
    { INT_MIN, 0 },
    { INT_MIN, 1 },
    { INT_MIN, TIMESPEC_HZ - 1 },
    { INT_MIN + 1, 0 },
    { INT_MIN + 1, 1 },
    { INT_MIN + 1, TIMESPEC_HZ - 1 },
    { -1, 0 },
    { -1, 1 },
    { -1, TIMESPEC_HZ - 1 },
    { 0, 0 },
    { 0, 1 },
    { 0, TIMESPEC_HZ - 1 },
    { 1, 0 },
    { 1, 1 },
    { 1, TIMESPEC_HZ - 1 },
    { 1234567890, 0 },
    { 1234567890, 1 },
    { 1234567890, TIMESPEC_HZ - 1 },
    { INT_MAX - 1, 0 },
    { INT_MAX - 1, 1 },
    { INT_MAX - 1, TIMESPEC_HZ - 1 },
    { INT_MAX, 0 },
    { INT_MAX, 1 },
    { INT_MAX, TIMESPEC_HZ - 1 },
    { INT_MAX, 2 * TIMESPEC_HZ }
  };
enum { nprototypes = sizeof prototype / sizeof *prototype };

static bool
valid (struct timespec a)
{
  return 0 <= a.tv_nsec && a.tv_nsec < TIMESPEC_HZ;
}

static int
sign (int i)
{
  return i < 0 ? -1 : 0 < i;
}

static int
cmp (struct timespec a, struct timespec b)
{
  return sign (timespec_cmp (a, b));
}

static bool
eq (struct timespec a, struct timespec b)
{
  return timespec_cmp (a, b) == 0;
}

static bool
extremal (struct timespec a)
{
  return ((a.tv_sec == TYPE_MINIMUM (time_t) && a.tv_nsec == 0)
          || (a.tv_sec == TYPE_MAXIMUM (time_t)
              && a.tv_nsec == TIMESPEC_HZ - 1));
}

int
main (void)
{
  int i, j, k;
  struct timespec test[nprototypes + 1];
  int ntests;
  int computed_hz = 1;
  struct timespec prevroundtrip;

  test[0] = make_timespec (TYPE_MINIMUM (time_t), -1);
  ntests = 1;
  for (i = 0; i < nprototypes; i++)
    {
      int s = prototype[i].s;
      if (TYPE_SIGNED (time_t) || 0 <= s)
        {
          time_t t = (s <= INT_MIN + 1 ? s - INT_MIN + TYPE_MINIMUM (time_t)
                      : INT_MAX - 1 <= s ? s - INT_MAX + TYPE_MAXIMUM (time_t)
                      : s);
          test[ntests++] = make_timespec (t, prototype[i].ns);
        }
    }

  for (i = 0; i < LOG10_TIMESPEC_HZ; i++)
    computed_hz *= 10;
  ASSERT (computed_hz == TIMESPEC_HZ);

  for (i = 0; i < ntests; i++)
    {
      struct timespec a = test[i];

      struct timespec roundtrip = dtotimespec (timespectod (a));
      if (i != 0)
        ASSERT (cmp (prevroundtrip, roundtrip) <= 0);
      prevroundtrip = roundtrip;

      ASSERT (sign (timespec_sign (a)) == cmp (a, make_timespec (0, 0)));

      if (valid (a))
        for (j = 0; j < ntests; j++)
          {
            struct timespec b = test[j];
            if (valid (b))
              {
                struct timespec sum = timespec_add (a, b);
                struct timespec diff = timespec_sub (a, b);
                struct timespec rdiff = timespec_sub (b, a);
                ASSERT (cmp (a, b) == sign (i - j));
                ASSERT (eq (sum, timespec_add (b, a)));
                if (! extremal (sum))
                  {
                    ASSERT (eq (a, timespec_sub (sum, b)));
                    ASSERT (eq (b, timespec_sub (sum, a)));

                    for (k = 0; k < ntests; k++)
                      {
                        struct timespec c = test[k];
                        if (valid (c))
                          {
                            struct timespec sumbc = timespec_add (b, c);
                            if (! extremal (sumbc))
                              ASSERT (eq (timespec_add (a, sumbc),
                                          timespec_add (sum, c)));
                          }
                      }
                  }
                if (! extremal (diff))
                  ASSERT (eq (a, timespec_add (diff, b)));
                if (! extremal (rdiff))
                  ASSERT (eq (b, timespec_add (rdiff, a)));

              }
          }
    }

  return 0;
}
