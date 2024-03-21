/* Test of searching in a string.
   Copyright (C) 2007-2023 Free Software Foundation, Inc.

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

/* Written by Bruno Haible <bruno@clisp.org>, 2007.  */

#include <config.h>

#include <string.h>

#include <locale.h>
#include <stdlib.h>

#include "macros.h"

int
main ()
{
  /* configure should already have checked that the locale is supported.  */
  if (setlocale (LC_ALL, "") == NULL)
    return 1;

  {
    const char input[] = "f\303\266\303\266";
    const char *result = mbsstr (input, "");
    ASSERT (result == input);
  }

  {
    const char input[] = "f\303\266\303\266";
    const char *result = mbsstr (input, "\303\266");
    ASSERT (result == input + 1);
  }

  {
    const char input[] = "f\303\266\303\266";
    const char *result = mbsstr (input, "\266\303");
    ASSERT (result == NULL);
  }

  {
    const char input[] = "\303\204BC \303\204BCD\303\204B \303\204BCD\303\204BCD\303\204BDE"; /* "ÄBC ÄBCDÄB ÄBCDÄBCDÄBDE" */
    const char *result = mbsstr (input, "\303\204BCD\303\204BD"); /* "ÄBCDÄBD" */
    ASSERT (result == input + 19);
  }

  {
    const char input[] = "\303\204BC \303\204BCD\303\204B \303\204BCD\303\204BCD\303\204BDE"; /* "ÄBC ÄBCDÄB ÄBCDÄBCDÄBDE" */
    const char *result = mbsstr (input, "\303\204BCD\303\204BE"); /* "ÄBCDÄBE" */
    ASSERT (result == NULL);
  }

  /* Check that a very long haystack is handled quickly if the needle is
     short and occurs near the beginning.  */
  {
    size_t repeat = 10000;
    size_t m = 1000000;
    const char *needle =
      "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
      "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
    char *haystack = (char *) malloc (m + 1);
    if (haystack != NULL)
      {
        memset (haystack, 'A', m);
        haystack[0] = '\303'; haystack[1] = '\204';
        haystack[m] = '\0';

        for (; repeat > 0; repeat--)
          {
            ASSERT (mbsstr (haystack, needle) == haystack + 2);
          }

        free (haystack);
      }
  }

  /* Check that a very long needle is discarded quickly if the haystack is
     short.  */
  {
    size_t repeat = 10000;
    size_t m = 1000000;
    const char *haystack =
      "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
      "A\303\207A\303\207A\303\207A\303\207A\303\207A\303\207A\303\207"
      "A\303\207A\303\207A\303\207A\303\207A\303\207A\303\207A\303\207"
      "A\303\207A\303\207A\303\207A\303\207A\303\207A\303\207A\303\207"
      "A\303\207A\303\207A\303\207A\303\207A\303\207A\303\207A\303\207"
      "A\303\207A\303\207A\303\207A\303\207A\303\207A\303\207";
    char *needle = (char *) malloc (m + 1);
    if (needle != NULL)
      {
        memset (needle, 'A', m);
        needle[m] = '\0';

        for (; repeat > 0; repeat--)
          {
            ASSERT (mbsstr (haystack, needle) == NULL);
          }

        free (needle);
      }
  }

  /* Check that the asymptotic worst-case complexity is not quadratic.  */
  {
    size_t m = 1000000;
    char *haystack = (char *) malloc (2 * m + 3);
    char *needle = (char *) malloc (m + 3);
    if (haystack != NULL && needle != NULL)
      {
        const char *result;

        memset (haystack, 'A', 2 * m);
        haystack[2 * m] = '\303'; haystack[2 * m + 1] = '\207';
        haystack[2 * m + 2] = '\0';

        memset (needle, 'A', m);
        needle[m] = '\303'; needle[m + 1] = '\207';
        needle[m + 2] = '\0';

        result = mbsstr (haystack, needle);
        ASSERT (result == haystack + m);
      }
    free (needle);
    free (haystack);
  }

  return 0;
}
