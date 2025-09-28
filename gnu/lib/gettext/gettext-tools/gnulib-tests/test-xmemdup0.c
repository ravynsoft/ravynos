/* Test of xmemdup0() function.
   Copyright (C) 2008-2023 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, see <https://www.gnu.org/licenses/>.  */

/* Written by Eric Blake <ebb9@byu.net>, 2008.  */

#include <config.h>

#include "xmemdup0.h"

#include <stdlib.h>
#include <string.h>

#include "macros.h"

int
main (int argc, char **argv)
{
  char buffer[10] = { 'a', 'b', 'c', 'd', '\0',
                      'f', 'g', 'h', 'i', 'j'   };

  /* Empty string.  */
  {
    char *result = xmemdup0 (NULL, 0);
    ASSERT (result);
    ASSERT (!*result);
    free (result);
  }
  {
    char *result = xmemdup0 ("", 0);
    ASSERT (result);
    ASSERT (!*result);
    free (result);
  }

  /* Various buffer lengths.  */
  {
    char *result = xmemdup0 (buffer, 4);
    ASSERT (result);
    ASSERT (strcmp (result, buffer) == 0);
    free (result);
  }
  {
    char *result = xmemdup0 (buffer, 5);
    ASSERT (result);
    ASSERT (strcmp (result, buffer) == 0);
    ASSERT (result[5] == '\0');
    free (result);
  }
  {
    char *result = xmemdup0 (buffer, 9);
    ASSERT (result);
    ASSERT (memcmp (result, buffer, 9) == 0);
    ASSERT (result[9] == '\0');
    free (result);
  }
  {
    char *result = xmemdup0 (buffer, 10);
    ASSERT (result);
    ASSERT (memcmp (result, buffer, 10) == 0);
    ASSERT (result[10] == '\0');
    free (result);
  }

  return 0;
}
