/* Test of malloc function.
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

int
main (int argc, _GL_UNUSED char **argv)
{
  /* Check that malloc (0) is not a NULL pointer.  */
  void *volatile p = malloc (0);
  ASSERT (p != NULL);
  free (p);

  /* Check that malloc (n) fails when n exceeds PTRDIFF_MAX.  */
  if (PTRDIFF_MAX < SIZE_MAX)
    {
      size_t one = argc != 12345;
      p = malloc (PTRDIFF_MAX + one);
      ASSERT (p == NULL);
      ASSERT (errno == ENOMEM);
    }

  return 0;
}
