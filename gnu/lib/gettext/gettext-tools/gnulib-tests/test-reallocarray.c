/* Test of reallocarray function.
   Copyright (C) 2010-2023 Free Software Foundation, Inc.

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

/* Specification.  */
#include <stdlib.h>

#include <errno.h>
#include <stdint.h>

#include "signature.h"
SIGNATURE_CHECK (reallocarray, void *, (void *, size_t, size_t));

#include "macros.h"

int
main ()
{
  /* Check that reallocarray fails when requested to allocate a block
     of memory larger than PTRDIFF_MAX or SIZE_MAX bytes.  */
  for (size_t n = 2; n != 0; n <<= 1)
    {
      void *volatile p = NULL;

      if (PTRDIFF_MAX / n + 1 <= SIZE_MAX)
        {
          p = reallocarray (p, PTRDIFF_MAX / n + 1, n);
          ASSERT (p == NULL);
          ASSERT (errno == ENOMEM);
        }

      p = reallocarray (p, SIZE_MAX / n + 1, n);
      ASSERT (p == NULL);
      ASSERT (errno == ENOMEM
              || errno == EOVERFLOW /* NetBSD */);

      /* Reallocarray should not crash with zero sizes.  */
      p = reallocarray (p, 0, n);
      p = reallocarray (p, n, 0);
      free (p);
    }

  return 0;
}
