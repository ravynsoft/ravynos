/* Test of dirfd() function.
   Copyright (C) 2023 Free Software Foundation, Inc.

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

/* Written by Bruno Haible <bruno@clisp.org>, 2023.  */

#include <config.h>

#include <dirent.h>

#include <stdio.h>

#include "macros.h"

int
main ()
{
#if defined _WIN32 && !defined __CYGWIN__
  fprintf (stderr, "Skipping test: The DIR type does not contain a file descriptor.\n");
  return 77;
#else
  /* On all other platforms, we expect to have either
       - a dirfd() function, or
       - a dirfd macro, or
       - a DIR struct with a d_fd member, or
       - a DIR struct with a dd_fd member.
     If we don't have this, dirfd.c produces a function that always returns -1.
     Check here that this does not happen.  */
  DIR *d = opendir (".");
  int fd = dirfd (d);
  ASSERT (fd >= 0);

  return 0;
#endif
}
