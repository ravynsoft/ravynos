/* Test program, used by the intl-3 test.
   Copyright (C) 2000, 2005, 2007, 2013, 2018 Free Software Foundation, Inc.

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

#include <locale.h>
#include <stdlib.h>
#include <stdio.h>

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
  const char *dir = argv[1];
  const char *locale = argv[2];
  const char *codeset = argv[3];

  /* Clean up environment.  */
  unsetenv ("LANGUAGE");
  unsetenv ("OUTPUT_CHARSET");

  textdomain ("tstprog");

  xsetenv ("LC_ALL", locale, 1);
  if (setlocale (LC_ALL, "") == NULL)
    setlocale (LC_ALL, "C");

  bindtextdomain ("tstprog", dir);

  bind_textdomain_codeset ("tstprog", codeset);

  printf ("%s\n", gettext ("cheese"));

  return 0;
}
