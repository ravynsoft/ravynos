/* Test program, used by the intl-setlocale-1 test.
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

/* Contributed to the GNU C Library by
   Thorsten Kukuk <kukuk@suse.de> and Andreas Jaeger <aj@suse.de>, 2000.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <locale.h>
#include <stdlib.h>
#include <stdio.h>

#if USE_SYSTEM_LIBINTL
# include <libintl.h>
#else
/* Make sure we use the included libintl, not the system's one. */
# undef _LIBINTL_H
# include "libgnuintl.h"
#endif

#define N_(string) string

struct data_t
{
  const char *selection;
  const char *description;
};

struct data_t strings[] =
{
  { "String1", N_("First string for testing.") },
  { "String2", N_("Another string for testing.") }
};
const int data_cnt = sizeof (strings) / sizeof (strings[0]);

const char *lang[] = { "de_DE.UTF-8", "fr_FR.UTF-8", "ll_CC" };
const int lang_cnt = sizeof (lang) / sizeof (lang[0]);

int
main (void)
{
  int i;

  /* Clean up environment.  */
  unsetenv ("LANGUAGE");
  unsetenv ("LC_ALL");
  unsetenv ("LC_MESSAGES");
  unsetenv ("LC_CTYPE");
  unsetenv ("LANG");
  unsetenv ("OUTPUT_CHARSET");

  textdomain ("tstprog");

  for (i = 0; i < lang_cnt; ++i)
    {
      int j;

      if (setlocale (LC_ALL, lang[i]) == NULL)
        setlocale (LC_ALL, "C");

      bindtextdomain ("tstprog", "in-sl-1");

      for (j = 0; j < data_cnt; ++j)
        printf ("%s - %s\n", strings[j].selection,
                gettext (strings[j].description));
    }

  return 0;
}
