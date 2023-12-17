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

/* Written by Bruno Haible <bruno@clisp.org>, 2007.  */

#include <config.h>

/* None of the files accessed by this test are large, so disable the
   fseek link warning if we are not using the gnulib fseek module.  */
#define _GL_NO_LARGE_FILES
#include <stdio.h>

#include "signature.h"
SIGNATURE_CHECK (ftello, off_t, (FILE *));

#include "binary-io.h"
#include "macros.h"

#ifndef FUNC_UNGETC_BROKEN
# define FUNC_UNGETC_BROKEN 0
#endif

int
main (int argc, _GL_UNUSED char **argv)
{
  int ch;
  /* Assume stdin is seekable iff argc > 1.  */
  if (argc == 1)
    {
      ASSERT (ftell (stdin) == -1);
      ASSERT (ftello (stdin) == -1);
      return 0;
    }

  /* mingw ftell is unreliable on text mode input.  */
  set_binary_mode (0, O_BINARY);

  /* Simple tests.  For each test, make sure ftell and ftello agree.  */
  ASSERT (ftell (stdin) == 0);
  ASSERT (ftello (stdin) == 0);

  ch = fgetc (stdin);
  ASSERT (ch == '#');
  ASSERT (ftell (stdin) == 1);
  ASSERT (ftello (stdin) == 1);

  /* Test ftell after ungetc of read input.  */
  ch = ungetc ('#', stdin);
  ASSERT (ch == '#');
  ASSERT (ftell (stdin) == 0);
  ASSERT (ftello (stdin) == 0);

  ch = fgetc (stdin);
  ASSERT (ch == '#');
  ASSERT (ftell (stdin) == 1);
  ASSERT (ftello (stdin) == 1);

  /* Test ftell after fseek.  */
  ASSERT (fseek (stdin, 2, SEEK_SET) == 0);
  ASSERT (ftell (stdin) == 2);
  ASSERT (ftello (stdin) == 2);

  /* Test ftell after random ungetc.  */
  ch = fgetc (stdin);
  ASSERT (ch == '/');
  ch = ungetc ('@', stdin);
  ASSERT (ch == '@');
  ASSERT (ftell (stdin) == 2);
  ASSERT (ftello (stdin) == 2);

  ch = fgetc (stdin);
  ASSERT (ch == '@');
  ASSERT (ftell (stdin) == 3);
  ASSERT (ftello (stdin) == 3);

  if (2 < argc)
    {
      if (FUNC_UNGETC_BROKEN)
        {
          fputs ("Skipping test: ungetc cannot handle arbitrary bytes\n",
                 stderr);
          return 77;
        }
      /* Test ftell after ungetc without read.  */
      ASSERT (fseek (stdin, 0, SEEK_CUR) == 0);
      ASSERT (ftell (stdin) == 3);
      ASSERT (ftello (stdin) == 3);

      ch = ungetc ('~', stdin);
      ASSERT (ch == '~');
      ASSERT (ftell (stdin) == 2);
      ASSERT (ftello (stdin) == 2);
    }

#if !defined __MINT__ /* FreeMiNT has problems seeking past end of file */
  /* Test ftell beyond end of file.  */
  ASSERT (fseek (stdin, 0, SEEK_END) == 0);
  ch = ftello (stdin);
  ASSERT (fseek (stdin, 10, SEEK_END) == 0);
  ASSERT (ftell (stdin) == ch + 10);
  ASSERT (ftello (stdin) == ch + 10);
#endif

  return 0;
}
