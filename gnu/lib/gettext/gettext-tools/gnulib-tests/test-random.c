/* Test random.
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
SIGNATURE_CHECK (srandom, void, (unsigned int));
SIGNATURE_CHECK (initstate, char *, (unsigned int, char *, size_t));
SIGNATURE_CHECK (setstate, char *, (char *));
SIGNATURE_CHECK (random, long, (void));

#include <time.h>

#include "macros.h"

int
main ()
{
  char buf[128];
  unsigned int i;
  unsigned int n_big = 0;

  initstate (time (NULL), buf, sizeof buf);
  for (i = 0; i < 1000; i++)
    {
      long r = random ();
      ASSERT (0 <= r);
      if (RAND_MAX / 2 < r)
        ++n_big;
    }

  /* Fail if none of the numbers were larger than RAND_MAX / 2.  */
  return !n_big;
}
