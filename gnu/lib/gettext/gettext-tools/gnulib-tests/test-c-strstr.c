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

#include "c-strstr.h"

#include <stdlib.h>
#include <string.h>

#include "macros.h"

int
main ()
{
  {
    const char input[] = "foo";
    const char *result = c_strstr (input, "");
    ASSERT (result == input);
  }

  {
    const char input[] = "foo";
    const char *result = c_strstr (input, "o");
    ASSERT (result == input + 1);
  }

  {
    const char input[] = "ABC ABCDAB ABCDABCDABDE";
    const char *result = c_strstr (input, "ABCDABD");
    ASSERT (result == input + 15);
  }

  {
    const char input[] = "ABC ABCDAB ABCDABCDABDE";
    const char *result = c_strstr (input, "ABCDABE");
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
        haystack[0] = 'B';
        haystack[m] = '\0';

        for (; repeat > 0; repeat--)
          {
            ASSERT (c_strstr (haystack, needle) == haystack + 1);
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
      "ABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABAB";
    char *needle = (char *) malloc (m + 1);
    if (needle != NULL)
      {
        memset (needle, 'A', m);
        needle[m] = '\0';

        for (; repeat > 0; repeat--)
          {
            ASSERT (c_strstr (haystack, needle) == NULL);
          }

        free (needle);
      }
  }

  /* Check that the asymptotic worst-case complexity is not quadratic.  */
  {
    size_t m = 1000000;
    char *haystack = (char *) malloc (2 * m + 2);
    char *needle = (char *) malloc (m + 2);
    if (haystack != NULL && needle != NULL)
      {
        const char *result;

        memset (haystack, 'A', 2 * m);
        haystack[2 * m] = 'B';
        haystack[2 * m + 1] = '\0';

        memset (needle, 'A', m);
        needle[m] = 'B';
        needle[m + 1] = '\0';

        result = c_strstr (haystack, needle);
        ASSERT (result == haystack + m);
      }
    free (needle);
    free (haystack);
  }

  {
    /* Ensure that with a barely periodic "short" needle, c_strstr's
       search does not mistakenly skip just past the match point.
       This use of c_strstr would mistakenly return NULL before
       gnulib v0.0-4927.  */
    const char *haystack =
      "\n"
      "with_build_libsubdir\n"
      "with_local_prefix\n"
      "with_gxx_include_dir\n"
      "with_cpp_install_dir\n"
      "enable_generated_files_in_srcdir\n"
      "with_gnu_ld\n"
      "with_ld\n"
      "with_demangler_in_ld\n"
      "with_gnu_as\n"
      "with_as\n"
      "enable_largefile\n"
      "enable_werror_always\n"
      "enable_checking\n"
      "enable_coverage\n"
      "enable_gather_detailed_mem_stats\n"
      "enable_build_with_cxx\n"
      "with_stabs\n"
      "enable_multilib\n"
      "enable___cxa_atexit\n"
      "enable_decimal_float\n"
      "enable_fixed_point\n"
      "enable_threads\n"
      "enable_tls\n"
      "enable_objc_gc\n"
      "with_dwarf2\n"
      "enable_shared\n"
      "with_build_sysroot\n"
      "with_sysroot\n"
      "with_specs\n"
      "with_pkgversion\n"
      "with_bugurl\n"
      "enable_languages\n"
      "with_multilib_list\n";
    const char *needle = "\n"
      "with_gnu_ld\n";
    const char* p = c_strstr (haystack, needle);
    ASSERT (p - haystack == 114);
  }

  {
    /* Same bug, shorter trigger.  */
    const char *haystack = "..wi.d.";
    const char *needle = ".d.";
    const char* p = c_strstr (haystack, needle);
    ASSERT (p - haystack == 4);
  }

  {
    /* Like the above, but trigger the flaw in two_way_long_needle
       by using a needle of length LONG_NEEDLE_THRESHOLD (32) or greater.
       Rather than trying to find the right alignment manually, I've
       arbitrarily chosen the following needle and template for the
       haystack, and ensure that for each placement of the needle in
       that haystack, c_strstr finds it.  */
    const char *needle = "\nwith_gnu_ld-extend-to-len-32-b\n";
    const char *h =
      "\n"
      "with_build_libsubdir\n"
      "with_local_prefix\n"
      "with_gxx_include_dir\n"
      "with_cpp_install_dir\n"
      "with_e_\n"
      "..............................\n"
      "with_FGHIJKLMNOPQRSTUVWXYZ\n"
      "with_567890123456789\n"
      "with_multilib_list\n";
    size_t h_len = strlen (h);
    char *haystack = malloc (h_len + 1);
    size_t i;
    ASSERT (haystack);
    for (i = 0; i < h_len - strlen (needle); i++)
      {
        const char *p;
        memcpy (haystack, h, h_len + 1);
        memcpy (haystack + i, needle, strlen (needle) + 1);
        p = c_strstr (haystack, needle);
        ASSERT (p);
        ASSERT (p - haystack == i);
      }
    free (haystack);
  }

  /* Test case from Yves Bastide.
     <https://www.openwall.com/lists/musl/2014/04/18/2>  */
  {
    const char input[] = "playing play play play always";
    const char *result = c_strstr (input, "play play play");
    ASSERT (result == input + 8);
  }

  /* Test long needles.  */
  {
    size_t m = 1024;
    char *haystack = (char *) malloc (2 * m + 1);
    char *needle = (char *) malloc (m + 1);
    if (haystack != NULL && needle != NULL)
      {
        const char *p;
        haystack[0] = 'x';
        memset (haystack + 1, ' ', m - 1);
        memset (haystack + m, 'x', m);
        haystack[2 * m] = '\0';
        memset (needle, 'x', m);
        needle[m] = '\0';
        p = c_strstr (haystack, needle);
        ASSERT (p);
        ASSERT (p - haystack == m);
      }
    free (needle);
    free (haystack);
  }

  return 0;
}
