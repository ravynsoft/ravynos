/* testlocale - test whether the locale given by the environment is installed.
   Copyright (C) 2003, 2006, 2019 Free Software Foundation, Inc.

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
#include <stdio.h>
#include <stdlib.h>

/* This must match intl/localename.c.  */
#if defined _LIBC || (defined __GNU_LIBRARY__ && __GNU_LIBRARY__ >= 2)
# define HAVE_LOCALE_NULL
#endif

int
main (int argc, char *argv[])
{
/* This test must match the one in intl/localename.c.  */
#if defined HAVE_LC_MESSAGES && defined HAVE_LOCALE_NULL
  if (setlocale (LC_ALL, "") == NULL)
    /* Couldn't set locale.  */
    exit (77);
#endif
  exit (0);
}
