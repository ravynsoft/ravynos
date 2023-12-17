/* Test of copying of files.
   Copyright (C) 2008-2023 Free Software Foundation, Inc.

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

/* Written by Bruno Haible <bruno@clisp.org>, 2008.  */

#include <config.h>

#include "acl.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "macros.h"

int
main (int argc, char *argv[])
{
  const char *file1;
  const char *file2;
  int fd1;
  struct stat statbuf;
  int mode;
  int fd2;

  ASSERT (argc == 3);

  file1 = argv[1];
  file2 = argv[2];

  fd1 = open (file1, O_RDONLY);
  if (fd1 < 0 || fstat (fd1, &statbuf) < 0)
    {
      fprintf (stderr, "could not open file \"%s\"\n", file1);
      exit (EXIT_FAILURE);
    }
  mode = statbuf.st_mode & 07777;

  fd2 = open (file2, O_WRONLY, 0600);
  if (fd2 < 0)
    {
      fprintf (stderr, "could not open file \"%s\"\n", file2);
      exit (EXIT_FAILURE);
    }

#if USE_ACL
  if (copy_acl (file1, fd1, file2, fd2, mode))
    exit (EXIT_FAILURE);
#else
  chmod (file2, mode);
#endif

  close (fd2);
  close (fd1);

  return 0;
}
