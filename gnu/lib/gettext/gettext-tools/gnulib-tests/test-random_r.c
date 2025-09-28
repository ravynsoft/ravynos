/* Test random_r.
   Copyright (C) 2008-2023 Free Software Foundation, Inc.

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

#include <config.h>

#include <stdlib.h>

#include "signature.h"
SIGNATURE_CHECK (srandom_r, int, (unsigned int, struct random_data *));
SIGNATURE_CHECK (initstate_r, int, (unsigned int, char *, size_t,
                                    struct random_data *));
SIGNATURE_CHECK (setstate_r, int, (char *, struct random_data *));
SIGNATURE_CHECK (random_r, int, (struct random_data *, int32_t *));

#include <time.h>

#include "macros.h"

/* Note: This test crashes on glibc/SPARC systems.
   Reported at <https://sourceware.org/bugzilla/show_bug.cgi?id=30584>.  */

static int
test_failed (int alignment)
{
  struct random_data rand_state;
  char buf[128 + sizeof (int32_t)];
  unsigned int i;
  unsigned int n_big = 0;

  rand_state.state = NULL;
  if (initstate_r (time (NULL), buf + alignment, sizeof buf - alignment,
                   &rand_state))
    return 1;
  for (i = 0; i < 1000; i++)
    {
      int32_t r;
      ASSERT (random_r (&rand_state, &r) == 0);
      ASSERT (0 <= r);
      if (RAND_MAX / 2 < r)
        ++n_big;
    }

  /* Fail if none of the numbers were larger than RAND_MAX / 2.  */
  return !n_big;
}

int
main ()
{
  int alignment;
  for (alignment = 0; alignment < sizeof (int32_t); alignment++)
    if (test_failed (alignment))
      return 1;
  return 0;
}
