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

#include "signature.h"
SIGNATURE_CHECK (setlocale, char *, (int, const char *));

#include <stdlib.h>
#include <string.h>

#include "macros.h"

int
main (int argc, char *argv[])
{
  char *name1;
  char *name2;

  /* Try to set the locale by implicitly looking at the LC_ALL environment
     variable.
     configure should already have checked that the locale is supported.  */
  if (setlocale (LC_ALL, "") == NULL)
    return 1;

  name1 = strdup (setlocale (LC_ALL, NULL));

  /* Reset the locale.  */
  if (setlocale (LC_ALL, "C") == NULL)
    return 1;

  /* Try to set the locale by explicitly looking at the LC_ALL environment
     variable.
     configure should already have checked that the locale is supported.  */
  if (setlocale (LC_ALL, getenv ("LC_ALL")) == NULL)
    return 1;

  name2 = strdup (setlocale (LC_ALL, NULL));

  ASSERT (name1);
  ASSERT (name2);

  /* Test that the two results are the same.  */
  ASSERT (strcmp (name1, name2) == 0);
  free (name1);
  free (name2);

  return 0;
}
