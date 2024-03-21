/* Test of calloc function.
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

#include "macros.h"

/* Return N.
   Usual compilers are not able to infer something about the return value.  */
static size_t
identity (size_t n)
{
  unsigned int x = rand ();
  unsigned int y = x * x * x * x;
  x++; y |= x * x * x * x;
  x++; y |= x * x * x * x;
  x++; y |= x * x * x * x;
  y = y >> 1;
  y &= -y;
  y -= 8;
  /* At this point Y is zero but GCC doesn't infer this.  */
  return n + y;
}

int
main ()
{
  /* Check that calloc (0, 0) is not a NULL pointer.  */
  {
    void * volatile p = calloc (0, 0);
    ASSERT (p != NULL);
    free (p);
  }

  /* Check that calloc fails when requested to allocate a block of memory
     larger than PTRDIFF_MAX or SIZE_MAX bytes.
     Use 'identity' to avoid a compiler warning from GCC 7.
     'volatile' is needed to defeat an incorrect optimization by clang 10,
     see <https://bugs.llvm.org/show_bug.cgi?id=46055>.  */
  {
    for (size_t n = 2; n != 0; n <<= 1)
      {
        void *volatile p = calloc (PTRDIFF_MAX / n + 1, identity (n));
        ASSERT (p == NULL);
        ASSERT (errno == ENOMEM);

        p = calloc (SIZE_MAX / n + 1, identity (n));
        ASSERT (p == NULL);
        ASSERT (errno == ENOMEM);
      }
  }

  return 0;
}
