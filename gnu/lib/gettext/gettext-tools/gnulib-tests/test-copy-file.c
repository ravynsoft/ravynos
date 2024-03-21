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

#include "copy-file.h"

#include <stdlib.h>

#include "macros.h"

int
main (int argc, char *argv[])
{
  const char *file1;
  const char *file2;
  int null_stderr;

  ASSERT (argc == 3);

  file1 = argv[1];
  file2 = argv[2];
  null_stderr = (getenv ("NO_STDERR_OUTPUT") != NULL);

  if (null_stderr)
    ASSERT (qcopy_file_preserving (file1, file2) == 0);
  else
    copy_file_preserving (file1, file2);

  return 0;
}
