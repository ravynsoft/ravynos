/*
 * Copyright (C) 2004, 2007-2023 Free Software Foundation, Inc.
 * Written by Bruno Haible and Eric Blake
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
SIGNATURE_CHECK (memmem, void *, (void const *, size_t, void const *, size_t));

#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include "zerosize-ptr.h"
#include "macros.h"

int
main (int argc, char *argv[])
{
#if HAVE_DECL_ALARM
  /* Declare failure if test takes too long, by using default abort
     caused by SIGALRM.  All known platforms that lack alarm also lack
     memmem, and the replacement memmem is known to not take too
     long.  */
  int alarm_value = 100;
  signal (SIGALRM, SIG_DFL);
  alarm (alarm_value);
#endif

  {
    const char input[] = "foo";
    const char *result = memmem (input, strlen (input), "", 0);
    ASSERT (result == input);
  }

  {
    const char input[] = "foo";
    const char *result = memmem (input, strlen (input), "o", 1);
    ASSERT (result == input + 1);
  }

  {
    const char input[] = "ABC ABCDAB ABCDABCDABDE";
    const char *result = memmem (input, strlen (input), "ABCDABD", 7);
    ASSERT (result == input + 15);
  }

  {
    const char input[] = "ABC ABCDAB ABCDABCDABDE";
    const char *result = memmem (input, strlen (input), "ABCDABE", 7);
    ASSERT (result == NULL);
  }

  {
    const char input[] = "ABC ABCDAB ABCDABCDABDE";
    const char *result = memmem (input, strlen (input), "ABCDABCD", 8);
    ASSERT (result == input + 11);
  }

  /* Check that length 0 does not dereference the pointer.  */
  void *page_boundary = zerosize_ptr ();
  if (page_boundary)
    {
      {
        const char *result = memmem (page_boundary, 0, "foo", 3);
        ASSERT (result == NULL);
      }

      {
        const char input[] = "foo";
        const char *result = memmem (input, strlen (input), page_boundary, 0);
        ASSERT (result == input);
      }
    }

  /* Check that a long periodic needle does not cause false positives.  */
  {
    const char input[] = "F_BD_CE_BD_EF_BF_BD_EF_BF_BD_EF_BF_BD_EF_BF_BD"
                         "_C3_88_20_EF_BF_BD_EF_BF_BD_EF_BF_BD"
                         "_C3_A7_20_EF_BF_BD";
    const char need[] = "_EF_BF_BD_EF_BF_BD_EF_BF_BD_EF_BF_BD_EF_BF_BD";
    const char *result = memmem (input, strlen (input), need, strlen (need));
    ASSERT (result == NULL);
  }
  {
    const char input[] = "F_BD_CE_BD_EF_BF_BD_EF_BF_BD_EF_BF_BD_EF_BF_BD"
                         "_C3_88_20_EF_BF_BD_EF_BF_BD_EF_BF_BD"
                         "_C3_A7_20_EF_BF_BD_DA_B5_C2_A6_20"
                         "_EF_BF_BD_EF_BF_BD_EF_BF_BD_EF_BF_BD_EF_BF_BD";
    const char need[] = "_EF_BF_BD_EF_BF_BD_EF_BF_BD_EF_BF_BD_EF_BF_BD";
    const char *result = memmem (input, strlen (input), need, strlen (need));
    ASSERT (result == input + 115);
  }

  /* Check that a very long haystack is handled quickly if the needle is
     short and occurs near the beginning.  */
  {
    size_t repeat = 10000;
    size_t m = 1000000;
    const char *needle =
      "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
      "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
    size_t n = strlen (needle);
    char *haystack = (char *) malloc (m + 1);
    if (haystack != NULL)
      {
        memset (haystack, 'A', m);
        haystack[0] = 'B';

        for (; repeat > 0; repeat--)
          {
            ASSERT (memmem (haystack, m, needle, n) == haystack + 1);
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
    size_t n = strlen (haystack);
    char *needle = (char *) malloc (m + 1);
    if (needle != NULL)
      {
        memset (needle, 'A', m);

        for (; repeat > 0; repeat--)
          {
            ASSERT (memmem (haystack, n, needle, m) == NULL);
          }

        free (needle);
      }
  }

  /* Check that the asymptotic worst-case complexity is not quadratic.  */
  {
    size_t m = 1000000;
    char *haystack = (char *) malloc (2 * m + 1);
    char *needle = (char *) malloc (m + 1);
    if (haystack != NULL && needle != NULL)
      {
        const char *result;

        memset (haystack, 'A', 2 * m);
        haystack[2 * m] = 'B';

        memset (needle, 'A', m);
        needle[m] = 'B';

        result = memmem (haystack, 2 * m + 1, needle, m + 1);
        ASSERT (result == haystack + m);
      }
    free (needle);
    free (haystack);
  }

  /* Check that long needles not present in a haystack can be handled
     with sublinear speed.  */
  {
    size_t repeat = 10000;
    size_t m = 1000000;
    size_t n = 1000;
    char *haystack = (char *) malloc (m);
    char *needle = (char *) malloc (n);
    if (haystack != NULL && needle != NULL)
      {
        const char *result;

        memset (haystack, 'A', m);
        memset (needle, 'B', n);

        for (; repeat > 0; repeat--)
          {
            result = memmem (haystack, m, needle, n);
            ASSERT (result == NULL);
          }
      }
    free (haystack);
    free (needle);
  }

  {
    /* Ensure that with a barely periodic "short" needle, memmem's
       search does not mistakenly skip just past the match point.
       This use of memmem would mistakenly return NULL before
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
    const char* p = memmem (haystack, strlen (haystack),
                            needle, strlen (needle));
    ASSERT (p - haystack == 114);
  }

  {
    /* Same bug, shorter trigger.  */
    const char *haystack = "..wi.d.";
    const char *needle = ".d.";
    const char* p = memmem (haystack, strlen (haystack),
                            needle, strlen (needle));
    ASSERT (p - haystack == 4);
  }

  {
    /* Like the above, but trigger the flaw in two_way_long_needle
       by using a needle of length LONG_NEEDLE_THRESHOLD (32) or greater.
       Rather than trying to find the right alignment manually, I've
       arbitrarily chosen the following needle and template for the
       haystack, and ensure that for each placement of the needle in
       that haystack, memmem finds it.  */
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
        p = memmem (haystack, strlen (haystack), needle, strlen (needle));
        ASSERT (p);
        ASSERT (p - haystack == i);
      }
    free (haystack);
  }

  /* Test case from Yves Bastide.
     <https://www.openwall.com/lists/musl/2014/04/18/2>  */
  {
    const char input[] = "playing play play play always";
    const char *result = memmem (input, strlen (input), "play play play", 14);
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
        p = memmem (haystack, strlen (haystack), needle, strlen (needle));
        ASSERT (p);
        ASSERT (p - haystack == m);
      }
    free (needle);
    free (haystack);
  }

  return 0;
}
