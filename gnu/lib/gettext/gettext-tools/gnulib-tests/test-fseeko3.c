/* Test of fseeko() function.
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

/* Written by Eric Blake <eblake@redhat.com>, 2011.  */

#include <config.h>

#include <stdio.h>

#include <stdlib.h>

#include "macros.h"

int
main (int argc, char **argv)
{
  int do_initial_ftell = atoi (argv[1]);
  const char *filename = argv[2];
  FILE *fp = fopen (filename, "r");
  ASSERT (fp != NULL);

  if (do_initial_ftell)
    {
      off_t pos = ftell (fp);
      ASSERT (pos == 0);
    }

  ASSERT (fseeko (fp, 0, SEEK_END) == 0);

  {
    off_t pos = ftell (fp);
    ASSERT (pos > 0);
  }

  ASSERT (fclose (fp) == 0);

  return 0;
}
