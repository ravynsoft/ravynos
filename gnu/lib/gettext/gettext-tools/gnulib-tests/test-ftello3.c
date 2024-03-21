/* Test of ftello() function.
   Copyright (C) 2007-2023 Free Software Foundation, Inc.

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

/* None of the files accessed by this test are large, so disable the
   fseek link warning if we are not using the gnulib fseek module.  */
#define _GL_NO_LARGE_FILES
#include <stdio.h>

#include <string.h>

#include "macros.h"

#define TESTFILE "t-ftello3.tmp"

int
main (void)
{
  FILE *fp;

  /* Create a file with some contents.  */
  fp = fopen (TESTFILE, "w");
  if (fp == NULL)
    goto skip;
  if (fwrite ("foogarsh", 1, 8, fp) < 8)
    goto skip;
  if (fclose (fp))
    goto skip;

  /* The file's contents is now "foogarsh".  */

  /* Try writing after reading to EOF.  */
  fp = fopen (TESTFILE, "r+");
  if (fp == NULL)
    goto skip;
  if (fseek (fp, -1, SEEK_END))
    goto skip;
  ASSERT (getc (fp) == 'h');
  ASSERT (getc (fp) == EOF);
  ASSERT (ftello (fp) == 8);
  ASSERT (ftello (fp) == 8);
  ASSERT (putc ('!', fp) == '!');
  ASSERT (ftello (fp) == 9);
  ASSERT (fclose (fp) == 0);
  fp = fopen (TESTFILE, "r");
  if (fp == NULL)
    goto skip;
  {
    char buf[10];
    ASSERT (fread (buf, 1, 10, fp) == 9);
    ASSERT (memcmp (buf, "foogarsh!", 9) == 0);
  }
  ASSERT (fclose (fp) == 0);

  /* The file's contents is now "foogarsh!".  */

  remove (TESTFILE);
  return 0;

 skip:
  fprintf (stderr, "Skipping test: prerequisite file operations failed.\n");
  remove (TESTFILE);
  return 77;
}
