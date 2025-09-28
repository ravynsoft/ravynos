/* Test of concatenation of two arbitrary file names.

   Copyright (C) 1996-2007, 2009-2023 Free Software Foundation, Inc.

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

/* Written by Jim Meyering.  */

#include <config.h>

#include "filenamecat.h"

#include "idx.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int
main (_GL_UNUSED int argc, char *argv[])
{
  static char const *const tests[][3] =
    {
      {"a", "b",   "a/b"},
      {"a/", "b",  "a/b"},
      {"a/", "/b", "a/b"},
      {"a", "/b",  "a/b"},

      {"/", "b",  "/b"},
      {"/", "/b", "/./b"}, /* This result could be shorter.  */
      {"/", "/",  "/./"},  /* This result could be shorter.  */
      {"a", "/",  "a/"},   /* this might deserve a diagnostic */
      {"/a", "/", "/a/"},  /* this might deserve a diagnostic */
      {"a", "//b",  "a//b"},
      {"", "a", "a"},  /* this might deserve a diagnostic */
    };
  unsigned int i;
  bool fail = false;

  for (i = 0; i < sizeof tests / sizeof tests[0]; i++)
    {
      char *base_in_result;
      char const *const *t = tests[i];
      char *res = file_name_concat (t[0], t[1], &base_in_result);
      idx_t prefixlen = base_in_result - res;
      size_t t0len = strlen (t[0]);
      size_t reslen = strlen (res);
      if (strcmp (res, t[2]) != 0)
        {
          fprintf (stderr, "test #%u: got %s, expected %s\n", i, res, t[2]);
          fail = true;
        }
      if (strcmp (t[1], base_in_result) != 0)
        {
          fprintf (stderr, "test #%u: base %s != base_in_result %s\n",
                   i, t[1], base_in_result);
          fail = true;
        }
      if (! (0 <= prefixlen && prefixlen <= reslen))
        {
          fprintf (stderr, "test #%u: base_in_result is not in result\n", i);
          fail = true;
        }
      if (reslen < t0len || memcmp (res, t[0], t0len) != 0)
        {
          fprintf (stderr, "test #%u: %s is not a prefix of %s\n",
                   i, t[0], res);
          fail = true;
        }
      free (res);
    }
  exit (fail ? EXIT_FAILURE : EXIT_SUCCESS);
}
