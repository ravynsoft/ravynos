/* Test of setting the current locale.
   Copyright (C) 2011-2023 Free Software Foundation, Inc.

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

#include <config.h>

#include <locale.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
main ()
{
  /* Try to set the locale by implicitly looking at the LC_ALL environment
     variable.  */
  if (setlocale (LC_ALL, "") != NULL)
    /* It was successful.  Check whether LC_CTYPE is non-trivial.  */
    if (strcmp (setlocale (LC_CTYPE, NULL), "C") == 0)
      {
        fprintf (stderr, "setlocale did not fail for implicit %s\n",
                 getenv ("LC_ALL"));
        return 1;
      }

  /* Reset the locale.  */
  if (setlocale (LC_ALL, "C") == NULL)
    return 1;

  /* Try to set the locale by explicitly looking at the LC_ALL environment
     variable.  */
  if (setlocale (LC_ALL, getenv ("LC_ALL")) != NULL)
    /* It was successful.  Check whether LC_CTYPE is non-trivial.  */
    if (strcmp (setlocale (LC_CTYPE, NULL), "C") == 0)
      {
        fprintf (stderr, "setlocale did not fail for explicit %s\n",
                 getenv ("LC_ALL"));
        return 1;
      }

  return 0;
}
