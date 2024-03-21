/*
 * Copyright (C) 2008-2023 Free Software Foundation, Inc.
 * Written by Eric Blake and Bruno Haible
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
SIGNATURE_CHECK (strchrnul, char *, (char const *, int));

#include <stdlib.h>

#include "macros.h"

int
main (void)
{
  size_t n = 0x100000;
  char *input = malloc (n + 1);
  ASSERT (input);

  input[0] = 'a';
  input[1] = 'b';
  memset (input + 2, 'c', 1024);
  memset (input + 1026, 'd', n - 1028);
  input[n - 2] = 'e';
  input[n - 1] = 'a';
  input[n] = '\0';

  /* Basic behavior tests.  */
  ASSERT (strchrnul (input, 'a') == input);
  ASSERT (strchrnul (input, 'b') == input + 1);
  ASSERT (strchrnul (input, 'c') == input + 2);
  ASSERT (strchrnul (input, 'd') == input + 1026);

  ASSERT (strchrnul (input + 1, 'a') == input + n - 1);
  ASSERT (strchrnul (input + 1, 'e') == input + n - 2);

  ASSERT (strchrnul (input, 'f') == input + n);
  ASSERT (strchrnul (input, '\0') == input + n);

  /* Check that a very long haystack is handled quickly if the byte is
     found near the beginning.  */
  {
    size_t repeat = 10000;
    for (; repeat > 0; repeat--)
      {
        ASSERT (strchrnul (input, 'c') == input + 2);
      }
  }

  /* Alignment tests.  */
  {
    int i, j;
    for (i = 0; i < 32; i++)
      {
        for (j = 0; j < 256; j++)
          input[i + j] = (j + 1) & 0xff;
        for (j = 1; j < 256; j++)
          {
            ASSERT (strchrnul (input + i, j) == input + i + j - 1);
            input[i + j - 1] = (j == 1 ? 2 : 1);
            ASSERT (strchrnul (input + i, j) == input + i + 255);
            input[i + j - 1] = j;
          }
      }
  }

  free (input);

  return 0;
}
