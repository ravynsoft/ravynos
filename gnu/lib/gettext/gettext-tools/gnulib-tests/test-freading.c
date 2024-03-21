/* Test of freading() function.
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
#include "freading.h"

#include <stdio.h>

#include "macros.h"

#define TESTFILE "t-freading.tmp"

int
main (void)
{
  FILE *fp;

  /* Create a file with some contents.  Write-only file is never reading.  */
  fp = fopen (TESTFILE, "w");
  ASSERT (fp);
  ASSERT (!freading (fp));
  ASSERT (fwrite ("foobarsh", 1, 8, fp) == 8);
  ASSERT (!freading (fp));
  ASSERT (fclose (fp) == 0);

  /* Open it in read-only mode.  Read-only file is always reading.  */
  fp = fopen (TESTFILE, "r");
  ASSERT (fp);
  ASSERT (freading (fp));
  ASSERT (fgetc (fp) == 'f');
  ASSERT (freading (fp));
  ASSERT (fseek (fp, 2, SEEK_CUR) == 0);
  ASSERT (freading (fp));
  ASSERT (fgetc (fp) == 'b');
  ASSERT (freading (fp));
  fflush (fp);
  ASSERT (freading (fp));
  ASSERT (fgetc (fp) == 'a');
  ASSERT (freading (fp));
  ASSERT (fseek (fp, 0, SEEK_END) == 0);
  ASSERT (freading (fp));
  ASSERT (fclose (fp) == 0);

  /* Open it in read-write mode.  POSIX requires a reposition (fseek,
     fsetpos, rewind) or EOF when transitioning from read to write;
     freading is only deterministic after input or output, but this
     test case should be portable even on open, after reposition, and
     at EOF.  */
  /* First a scenario with only fgetc, fseek, fputc.  */
  fp = fopen (TESTFILE, "r+");
  ASSERT (fp);
  ASSERT (!freading (fp));
  ASSERT (fgetc (fp) == 'f');
  ASSERT (freading (fp));
  ASSERT (fseek (fp, 2, SEEK_CUR) ==  0);
  /* freading (fp) is undefined here, but fwriting (fp) is false.  */
  ASSERT (fgetc (fp) == 'b');
  ASSERT (freading (fp));
  /* This fseek call is necessary when switching from reading to writing.
     See the description of fopen(), ISO C 99 7.19.5.3.(6).  */
  ASSERT (fseek (fp, 0, SEEK_CUR) == 0);
  /* freading (fp) is undefined here, but fwriting (fp) is false.  */
  ASSERT (fputc ('x', fp) == 'x');
  ASSERT (!freading (fp));
  ASSERT (fseek (fp, 0, SEEK_END) == 0);
  /* freading (fp) is undefined here, because on some implementations (e.g.
     glibc) fseek causes a buffer to be read.
     fwriting (fp) is undefined as well.  */
  ASSERT (fclose (fp) == 0);

  /* Open it in read-write mode.  POSIX requires a reposition (fseek,
     fsetpos, rewind) or EOF when transitioning from read to write;
     freading is only deterministic after input or output, but this
     test case should be portable even on open, after reposition, and
     at EOF.  */
  /* Here a scenario that includes fflush.  */
  fp = fopen (TESTFILE, "r+");
  ASSERT (fp);
  ASSERT (!freading (fp));
  ASSERT (fgetc (fp) == 'f');
  ASSERT (freading (fp));
  ASSERT (fseek (fp, 2, SEEK_CUR) == 0);
  /* freading (fp) is undefined here, but fwriting (fp) is false.  */
  ASSERT (fgetc (fp) == 'b');
  ASSERT (freading (fp));
  fflush (fp);
  /* freading (fp) is undefined here, but fwriting (fp) is false.  */
  ASSERT (fgetc (fp) == 'x');
  ASSERT (freading (fp));
  /* This fseek call is necessary when switching from reading to writing.
     See the description of fopen(), ISO C 99 7.19.5.3.(6).  */
  ASSERT (fseek (fp, 0, SEEK_CUR) == 0);
  /* freading (fp) is undefined here, but fwriting (fp) is false.  */
  ASSERT (fputc ('z', fp) == 'z');
  ASSERT (!freading (fp));
  ASSERT (fseek (fp, 0, SEEK_END) == 0);
  /* freading (fp) is undefined here, because on some implementations (e.g.
     glibc) fseek causes a buffer to be read.
     fwriting (fp) is undefined as well.  */
  ASSERT (fclose (fp) == 0);

  /* Open it in append mode.  Write-only file is never reading.  */
  fp = fopen (TESTFILE, "a");
  ASSERT (fp);
  ASSERT (!freading (fp));
  ASSERT (fwrite ("bla", 1, 3, fp) == 3);
  ASSERT (!freading (fp));
  ASSERT (fclose (fp) == 0);
  ASSERT (remove (TESTFILE) == 0);
  return 0;
}
