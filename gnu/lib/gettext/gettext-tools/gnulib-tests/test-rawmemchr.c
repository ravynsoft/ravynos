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
SIGNATURE_CHECK (rawmemchr, void *, (void const *, int));

#include <stdlib.h>

#include "zerosize-ptr.h"
#include "macros.h"

/* Calculating void * + int is not portable, so this wrapper converts
   to char * to make the tests easier to write.  */
#define RAWMEMCHR (char *) rawmemchr

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
  ASSERT (RAWMEMCHR (input, 'a') == input);
  ASSERT (RAWMEMCHR (input, 'b') == input + 1);
  ASSERT (RAWMEMCHR (input, 'c') == input + 2);
  ASSERT (RAWMEMCHR (input, 'd') == input + 1026);

  ASSERT (RAWMEMCHR (input + 1, 'a') == input + n - 1);
  ASSERT (RAWMEMCHR (input + 1, 'e') == input + n - 2);
  ASSERT (RAWMEMCHR (input + 1, 0x789abc00 | 'e') == input + n - 2);

  ASSERT (RAWMEMCHR (input, '\0') == input + n);

  /* Alignment tests.  */
  {
    int i, j;
    for (i = 0; i < 32; i++)
      {
        for (j = 0; j < 256; j++)
          input[i + j] = j;
        for (j = 0; j < 256; j++)
          {
            ASSERT (RAWMEMCHR (input + i, j) == input + i + j);
          }
      }
  }

  /* Ensure that no unaligned oversized reads occur.  */
  {
    char *page_boundary = (char *) zerosize_ptr ();
    size_t i;

    if (!page_boundary)
      page_boundary = input + 4096;
    memset (page_boundary - 512, '1', 511);
    page_boundary[-1] = '2';
    for (i = 1; i <= 512; i++)
      ASSERT (RAWMEMCHR (page_boundary - i, (i * 0x01010100) | '2')
              == page_boundary - 1);
  }

  free (input);

  /* Test aligned oversized reads, which are allowed on most architectures
     but not on CHERI.  */
  {
    input = malloc (5);
    memcpy (input, "abcde", 5);
    ASSERT (RAWMEMCHR (input, 'e') == input + 4);
    ASSERT (RAWMEMCHR (input + 1, 'e') == input + 4);
    ASSERT (RAWMEMCHR (input + 2, 'e') == input + 4);
    ASSERT (RAWMEMCHR (input + 3, 'e') == input + 4);
    ASSERT (RAWMEMCHR (input + 4, 'e') == input + 4);
    free (input);
  }

  return 0;
}
