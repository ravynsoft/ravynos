/* Test of fpurge() function.
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

/* Written by Bruno Haible <bruno@clisp.org>, 2007.  */

#include <config.h>

/* None of the files accessed by this test are large, so disable the
   fseek link warning if we are not using the gnulib fseek module.  */
#define _GL_NO_LARGE_FILES
#include <stdio.h>

#include <string.h>

#include "macros.h"

#define TESTFILE "t-fpurge.tmp"

int
main (void)
{
  int check_filepos;

  for (check_filepos = 0; check_filepos <= 1; check_filepos++)
    {
      FILE *fp;

      /* Create a file with some contents.  */
      fp = fopen (TESTFILE, "w");
      if (fp == NULL)
        goto skip;
      if (fwrite ("foobarsh", 1, 8, fp) < 8)
        goto skip;
      if (fclose (fp))
        goto skip;

      /* The file's contents is now "foobarsh".  */

      /* Open it in read-write mode.  */
      fp = fopen (TESTFILE, "r+");
      if (fp == NULL)
        goto skip;
      if (fseek (fp, 3, SEEK_CUR))
        goto skip;
      if (fwrite ("g", 1, 1, fp) < 1)
        goto skip;
      if (fflush (fp))
        goto skip;
      if (fwrite ("bz", 1, 2, fp) < 2)
        goto skip;
      /* Discard pending write.  */
      ASSERT (fpurge (fp) == 0);
      /* Verify that when discarding pending output, the file position is set
         back to where it was before the write calls.  */
      if (check_filepos)
        ASSERT (ftell (fp) == 4);
      ASSERT (fclose (fp) == 0);

      /* Open it in read-only mode.  */
      fp = fopen (TESTFILE, "r");
      if (fp == NULL)
        goto skip;
      /* Verify that the pending writes before the fpurge were really
         discarded.  */
      {
        char buf[8];
        if (fread (buf, 1, 7, fp) < 7)
          goto skip;
        ASSERT (memcmp (buf, "foogars", 7) == 0);
      }
      /* Discard the buffered 'h'.  */
      if (check_filepos)
        ASSERT (ftell (fp) == 7);
      ASSERT (fpurge (fp) == 0);
      /* Verify that when discarding pending input, the file position is
         advanced to match the end of the previously read input.  */
      if (check_filepos)
        ASSERT (ftell (fp) == 8);
      ASSERT (getc (fp) == EOF);
      ASSERT (fclose (fp) == 0);

      /* The file's contents is now "foogarsh".  */

      /* Ensure that purging a read does not corrupt subsequent writes.  */
      fp = fopen (TESTFILE, "r+");
      if (fp == NULL)
        goto skip;
      if (fseek (fp, -1, SEEK_END))
        goto skip;
      ASSERT (getc (fp) == 'h');
      ASSERT (getc (fp) == EOF);
      if (check_filepos)
        ASSERT (ftell (fp) == 8);
      ASSERT (fpurge (fp) == 0);
      if (check_filepos)
        ASSERT (ftell (fp) == 8);
      ASSERT (putc ('!', fp) == '!');
      if (check_filepos)
        ASSERT (ftell (fp) == 9);
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
    }

  remove (TESTFILE);
  return 0;

 skip:
  fprintf (stderr, "Skipping test: prerequisite file operations failed.\n");
  remove (TESTFILE);
  return 77;
}
