/* Test of binary mode I/O.
   Copyright (C) 2005, 2007-2023 Free Software Foundation, Inc.

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

/* Written by Bruno Haible <bruno@clisp.org>, 2005.  */

#include <config.h>

#include "binary-io.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "macros.h"

int
main (_GL_UNUSED int argc, char *argv[])
{
  /* Test the O_BINARY macro.  */
  {
    int fd =
      open ("t-bin-out0.tmp", O_CREAT | O_TRUNC | O_RDWR | O_BINARY, 0600);
    if (write (fd, "Hello\n", 6) < 0)
      exit (1);
    close (fd);
  }
  {
    struct stat statbuf;
    if (stat ("t-bin-out0.tmp", &statbuf) < 0)
      exit (1);
    ASSERT (statbuf.st_size == 6);
  }

  switch (argv[1][0])
    {
    case '1':
      /* Test the set_binary_mode() function.  */
      set_binary_mode (1, O_BINARY);
      fputs ("Hello\n", stdout);
      break;

    default:
      break;
    }

  return 0;
}
