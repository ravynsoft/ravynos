/* Test program, used by the format-c-5 test.
   Copyright (C) 2004, 2006, 2010, 2018, 2023 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* For %Id to work, we need the real setlocale(), not the fake one. */
#if !USE_SYSTEM_LIBINTL && !(__GLIBC__ >= 2 && !defined __UCLIBC__)
# include "setlocale.c"
#endif

#if USE_SYSTEM_LIBINTL
# define xsetenv setenv
# include <libintl.h>
#else
# include "xsetenv.h"
/* Make sure we use the included libintl, not the system's one. */
# undef _LIBINTL_H
# include "libgnuintl.h"
#endif

int
main (int argc, char *argv[])
{
  int n = 5;
  const char *en;
  const char *s;
  const char *expected_translation;
  const char *expected_result;
  char buf[100];

  xsetenv ("LC_ALL", argv[1], 1);
  if (setlocale (LC_ALL, "") == NULL)
    /* Couldn't set locale.  */
    exit (77);

  textdomain ("fc5");
  bindtextdomain ("fc5", ".");

  if (strcmp (gettext ("the president"), "der Vorsitzende") != 0)
    {
      fprintf (stderr, "Simple messages not translated.\n");
      exit (1);
    }

  s = gettext ("father of %d children");
  en = "father of %d children";
#if (__GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 2)) && !defined __UCLIBC__
  expected_translation = "Vater von %Id Kindern";
  if (strncmp (argv[1], "fa", 2) == 0)
    expected_result = "Vater von \xdb\xb5 Kindern";
  else
    expected_result = "Vater von 5 Kindern";
#else
  expected_translation = "Vater von %d Kindern";
  expected_result = "Vater von 5 Kindern";
#endif

  if (strcmp (s, en) == 0)
    {
      fprintf (stderr, "String not translated.\n");
      exit (1);
    }
  if (strcmp (s, expected_translation) != 0)
    {
      fprintf (stderr, "String incorrectly translated.\n");
      exit (1);
    }
  sprintf (buf, s, n);
  if (strcmp (buf, expected_result) != 0)
    {
      fprintf (stderr, "printf of translation wrong.\n");
      exit (1);
    }
  return 0;
}
