/* Child program invoked by test-pipe-filter-ii2-main.

   Copyright (C) 2009-2023 Free Software Foundation, Inc.
   Written by Paolo Bonzini <bonzini@gnu.org>, 2009.

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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "binary-io.h"

int
main ()
{
  set_binary_mode (STDOUT_FILENO, O_BINARY);

  /* Repeatedly: Read two integers i and j, then output all integers in the
     range i..j, one per line.  */
  for (;;)
    {
      int i, j;

      if (scanf (" %d", &i) != 1)
        break;
      if (scanf (" %d", &j) != 1)
        break;
      if (j == -1)
        exit (i);
      while (i <= j)
        printf ("abcdefghijklmnopqrstuvwxyz%d\n", i++);
    }
  exit (0);
}
