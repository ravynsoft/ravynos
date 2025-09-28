/*
 * Copyright (C) 2010-2023 Free Software Foundation, Inc.
 * Written by Eric Blake
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#include <config.h>

#include <string.h>

#include "signature.h"
SIGNATURE_CHECK (strnlen, size_t, (char const *, size_t));

#include <stdlib.h>

#include "zerosize-ptr.h"
#include "macros.h"

int
main (void)
{
  size_t i;
  char *page_boundary = (char *) zerosize_ptr ();
  if (!page_boundary)
    {
      page_boundary = malloc (0x1000);
      ASSERT (page_boundary);
      page_boundary += 0x1000;
    }

  /* Basic behavior tests.  */
  ASSERT (strnlen ("a", 0) == 0);
  ASSERT (strnlen ("a", 1) == 1);
  ASSERT (strnlen ("a", 2) == 1);
  ASSERT (strnlen ("", 0x100000) == 0);

  /* Memory fence and alignment testing.  */
  for (i = 0; i < 512; i++)
    {
      char *start = page_boundary - i;
      size_t j = i;
      memset (start, 'x', i);
      do
        {
          if (i != j)
            {
              start[j] = 0;
              ASSERT (strnlen (start, i + j) == j);
            }
          ASSERT (strnlen (start, i) == j);
          ASSERT (strnlen (start, j) == j);
        }
      while (j--);
    }

  return 0;
}
