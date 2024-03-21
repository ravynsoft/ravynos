/* Test program, used by the format-c-4 test.
   Copyright (C) 2002, 2009, 2013, 2018, 2020, 2023 Free Software Foundation, Inc.

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
#ifdef HAVE_INTTYPES_H
# include <inttypes.h>
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

/* Disable the override of setlocale that libgnuintl.h activates on MacOS X
   and Windows.  This test relies on the fake setlocale function in
   setlocale.c.  */
#undef setlocale
#if defined _WIN32 && !defined __CYGWIN__
# define setlocale fake_setlocale
extern char *setlocale (int category, const char *locale);
#endif

/* Fallback definition.  */
#if !defined PRId8
# define PRId8 "d"
#endif

int
main (int argc, char *argv[])
{
  unsigned char n = 5;
  const char *s;
  const char *c1;
  const char *c2;
  char buf[100];

  xsetenv ("LC_ALL", argv[1], 1);
  if (setlocale (LC_ALL, "") == NULL)
    {
      fprintf (stderr, "Couldn't set locale.\n");
      exit (1);
    }

  textdomain ("fc4");
  bindtextdomain ("fc4", "fc4-dir");

  if (strcmp (gettext ("the president"), "der Vorsitzende") != 0)
    {
      fprintf (stderr, "Simple messages not translated.\n");
      exit (1);
    }

  s = ngettext ("father of %"PRId8" child", "father of %"PRId8" children", n);
  c1 = "Vater von %"; c2 = " Kindern";

  if (!(strlen (s) > strlen (c1) + strlen (c2)
        && memcmp (s, c1, strlen (c1)) == 0
        && memcmp (s + strlen (s) - strlen (c2), c2, strlen (c2)) == 0))
    {
      fprintf (stderr, "String not translated.\n");
      exit (1);
    }
  if (strchr (s, '<') != NULL || strchr (s, '>') != NULL)
    {
      fprintf (stderr, "Translation contains <...> markers.\n");
      exit (1);
    }
  sprintf (buf, s, n);
  if (strcmp (buf, "Vater von 5 Kindern") != 0)
    {
      fprintf (stderr, "printf of translation wrong.\n");
      exit (1);
    }
  return 0;
}
