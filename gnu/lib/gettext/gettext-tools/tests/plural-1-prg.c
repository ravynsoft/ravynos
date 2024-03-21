/* Test program, used by the plural-1 test.
   Copyright (C) 2001-2002, 2009, 2013, 2018, 2020 Free Software Foundation, Inc.

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
# include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <locale.h>

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

int
main (int argc, char *argv[])
{
  int n = atoi (argv[2]);

  xsetenv ("LC_ALL", argv[1], 1);
  if (setlocale (LC_ALL, "") == NULL)
    return 1;

  textdomain ("cake");
  bindtextdomain ("cake", "plural-1-dir");
  printf (ngettext ("a piece of cake", "%d pieces of cake", n), n);
  printf ("\n");
  return 0;
}
