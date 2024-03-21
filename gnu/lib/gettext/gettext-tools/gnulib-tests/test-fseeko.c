/* Test of fseeko() function.
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
SIGNATURE_CHECK (fseeko, int, (FILE *, off_t, int));


#include "macros.h"

#ifndef FUNC_UNGETC_BROKEN
# define FUNC_UNGETC_BROKEN 0
#endif

int
main (int argc, _GL_UNUSED char **argv)
{
  /* Assume stdin is non-empty, seekable, and starts with '#!/bin/sh'
     iff argc > 1.  */
  int expected = argc > 1 ? 0 : -1;
  /* Exit with success only if fseek/fseeko agree.  */
  int r1 = fseeko (stdin, 0, SEEK_CUR);
  int r2 = fseek (stdin, 0, SEEK_CUR);
  ASSERT (r1 == r2 && r1 == expected);
  if (argc > 1)
    {
      /* Test that fseek discards previously read ungetc data.  */
      int ch = fgetc (stdin);
      ASSERT (ch == '#');
      ASSERT (ungetc (ch, stdin) == ch);
      ASSERT (fseeko (stdin, 2, SEEK_SET) == 0);
      ch = fgetc (stdin);
      ASSERT (ch == '/');
      if (2 < argc)
        {
          if (FUNC_UNGETC_BROKEN)
            {
              fputs ("Skipping test: ungetc cannot handle arbitrary bytes\n",
                     stderr);
              return 77;
            }
          /* Test that fseek discards random ungetc data.  */
          ASSERT (ungetc (ch ^ 0xff, stdin) == (ch ^ 0xff));
        }
      ASSERT (fseeko (stdin, 0, SEEK_END) == 0);
      ASSERT (fgetc (stdin) == EOF);
      /* Test that fseek resets end-of-file marker.  */
      ASSERT (feof (stdin));
      ASSERT (fseeko (stdin, 0, SEEK_END) == 0);
      ASSERT (!feof (stdin));
    }
  return 0;
}
