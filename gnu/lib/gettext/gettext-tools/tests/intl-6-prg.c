/* Test program, used by the intl-6 test.
   Copyright (C) 2000, 2005, 2007, 2013, 2018, 2020, 2023 Free Software Foundation, Inc.

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
#include <string.h>
#if defined _WIN32 && !defined __CYGWIN__
# include <wchar.h>
#endif

#include "xsetenv.h"
/* Make sure we use the included libintl, not the system's one. */
#undef _LIBINTL_H
#include "libgnuintl.h"

const char unicodedir[] = "русский…日本語…हिंदी…😷";
#if defined _WIN32 && !defined __CYGWIN__
const wchar_t wunicodedir[] = /* the same string in UTF-16 encoding */
  { 0x0440, 0x0443, 0x0441, 0x0441, 0x043A, 0x0438, 0x0439, 0x2026,
    0x65E5, 0x672C, 0x8A9E, 0x2026,
    0x0939, 0x093F, 0x0902, 0x0926, 0x0940, 0x2026,
    0xD83D, 0xDE37, 0
  };
#endif

int
main (int argc, char *argv[])
{
  const char *dir = argv[1];
  const char *locale = argv[2];
  wchar_t *wdir;
  int ret;

  /* Clean up environment.  */
  unsetenv ("LANGUAGE");
  unsetenv ("OUTPUT_CHARSET");

  xsetenv ("LC_ALL", locale, 1);
  if (setlocale (LC_ALL, "") == NULL)
    setlocale (LC_ALL, "C");

  /* Set up translation domain.  */

  wdir = (wchar_t *) malloc ((strlen (dir) + 1) * sizeof (wchar_t));
  mbstowcs (wdir, dir, strlen (dir) + 1);

  /* Rename the directory.  */
#if defined _WIN32 && !defined __CYGWIN__
  ret = _wrename (wdir, wunicodedir);
#else
  ret = rename (dir, unicodedir);
#endif
  if (ret != 0)
    {
      fprintf (stderr, "Initial rename failed.\n");
      exit (1);
    }

  textdomain ("tstprog");

#if defined _WIN32 && !defined __CYGWIN__
  wbindtextdomain ("tstprog", wunicodedir);
#else
  bindtextdomain ("tstprog", unicodedir);
#endif

  /* Look up the translation.  */
  printf ("%s\n", gettext ("cheese"));

  /* Rename the directory back.  */
#if defined _WIN32 && !defined __CYGWIN__
  ret = _wrename (wunicodedir, wdir);
#else
  ret = rename (unicodedir, dir);
#endif
  if (ret != 0)
    {
      fprintf (stderr, "Final rename failed.\n");
      exit (1);
    }

  return 0;
}
