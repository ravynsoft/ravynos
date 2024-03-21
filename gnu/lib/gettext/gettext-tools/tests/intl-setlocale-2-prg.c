/* Test program, used by the intl-setlocale-2 test.
   Copyright (C) 2005, 2007, 2013, 2018 Free Software Foundation, Inc.

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

/* Written by Bruno Haible <haible@clisp.cons.org>, 2005.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <locale.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if USE_SYSTEM_LIBINTL
# include <libintl.h>
#else
/* Make sure we use the included libintl, not the system's one. */
# undef _LIBINTL_H
# include "libgnuintl.h"
#endif

int
main (void)
{
  char *s;
  int result = 0;

  unsetenv ("LANGUAGE");
  unsetenv ("OUTPUT_CHARSET");
  textdomain ("tstprog");
  bindtextdomain ("tstprog", "in-sl-2");

  setlocale (LC_ALL, "de_DE.ISO-8859-1");

  /* Here we expect output in ISO-8859-1.  */
  s = gettext ("cheese");
  if (strcmp (s, "K\344se"))
    {
      fprintf (stderr, "call 1 returned: %s\n", s);
      result = 1;
    }

  setlocale (LC_ALL, "de_DE.UTF-8");

  /* Here we expect output in UTF-8.  */
  s = gettext ("cheese");
  if (strcmp (s, "K\303\244se"))
    {
      fprintf (stderr, "call 2 returned: %s\n", s);
      result = 1;
    }

  return result;
}
