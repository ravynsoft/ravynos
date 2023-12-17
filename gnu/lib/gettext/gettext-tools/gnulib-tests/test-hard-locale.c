/* Test of determination whether a locale is different from the "C" locale.
   Copyright (C) 2019-2023 Free Software Foundation, Inc.

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

/* Written by Bruno Haible <bruno@clisp.org>, 2019.  */

#include <config.h>

#include "hard-locale.h"

#include <locale.h>
#include <stdio.h>
#include <string.h>

/* True if all locale names are accepted and all locales are trivial.
   This is the case e.g. on OpenBSD 3.8.  */
static bool all_trivial;

static int
test_one (const char *name, int failure_bitmask)
{
  if (setlocale (LC_ALL, name) != NULL)
    {
      bool expected;

      /* musl libc has special code for the C.UTF-8 locale; other than that,
         all locale names are accepted and all locales are trivial.
         OpenBSD returns the locale name that was set, but we don't know how it
         behaves under the hood.  Likewise for Haiku.
         On Android >= 5.0, the "C" locale may have UTF-8 encoding, and we don't
         know how it will behave in the future.  */
#if defined MUSL_LIBC || defined __OpenBSD__ || defined __HAIKU__ || defined __ANDROID__
      expected = true;
#else
      expected = !all_trivial;
#endif
      if (hard_locale (LC_CTYPE) != expected)
        {
          if (expected)
            fprintf (stderr, "Unexpected: The category LC_CTYPE of the locale '%s' is not equivalent to C or POSIX.\n",
                     name);
          else
            fprintf (stderr, "Unexpected: The category LC_CTYPE of the locale '%s' is equivalent to C or POSIX.\n",
                     name);
          return failure_bitmask;
        }

      /* On NetBSD 7.0, some locales such as de_DE.ISO8859-1 and de_DE.UTF-8
         have the LC_COLLATE category set to "C".
         Similarly, on musl libc, with the C.UTF-8 locale.
         On Android >= 5.0, the "C" locale may have UTF-8 encoding, and we don't
         know how it will behave in the future.  */
#if defined __NetBSD__
      expected = false;
#elif defined MUSL_LIBC
      expected = strcmp (name, "C.UTF-8") != 0;
#elif (defined __OpenBSD__ && HAVE_DUPLOCALE) || defined __HAIKU__ || defined __ANDROID__ /* OpenBSD >= 6.2, Haiku, Android */
      expected = true;
#else
      expected = !all_trivial;
#endif
      if (hard_locale (LC_COLLATE) != expected)
        {
          if (expected)
            fprintf (stderr, "Unexpected: The category LC_COLLATE of the locale '%s' is not equivalent to C or POSIX.\n",
                     name);
          else
            fprintf (stderr, "Unexpected: The category LC_COLLATE of the locale '%s' is equivalent to C or POSIX.\n",
                     name);
          return failure_bitmask;
        }
    }
  return 0;
}

int
main ()
{
  int fail = 0;

  /* The initial locale is the "C" or "POSIX" locale.
     On Android >= 5.0, it is equivalent to the "C.UTF-8" locale, cf.
     <https://lists.gnu.org/archive/html/bug-gnulib/2023-01/msg00141.html>.  */
#if ! defined __ANDROID__
  if (hard_locale (LC_CTYPE) || hard_locale (LC_COLLATE))
    {
      fprintf (stderr, "The initial locale should not be hard!\n");
      fail |= 1;
    }
#endif

  all_trivial = (setlocale (LC_ALL, "foobar") != NULL);

  fail |= test_one ("de", 2);
  fail |= test_one ("de_DE", 4);
  fail |= test_one ("de_DE.ISO8859-1", 8);
  fail |= test_one ("de_DE.iso88591", 8);
  fail |= test_one ("de_DE.UTF-8", 16);
  fail |= test_one ("de_DE.utf8", 16);
  fail |= test_one ("german", 32);
  fail |= test_one ("C.UTF-8", 64);

  return fail;
}
