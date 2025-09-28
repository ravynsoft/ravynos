/* Test of POSIX compatible fflush() function.
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

#include <config.h>

#include <stdio.h>

#include "binary-io.h"
#include "macros.h"

int
main (int argc, char **argv)
{
  int c;

  /* Avoid the well-known bugs of fflush() on streams in O_TEXT mode
     on native Windows platforms.  */
  set_binary_mode (0, O_BINARY);

  if (argc > 1)
    switch (argv[1][0])
      {
      case '1':
        /* Check fflush after a backup ungetc() call.  This is case 1a in
           terms of
           <https://lists.gnu.org/r/bug-gnulib/2008-03/msg00131.html>,
           according to the Austin Group's resolution on 2009-01-08.  */

        c = fgetc (stdin);
        ASSERT (c == '#');

        c = fgetc (stdin);
        ASSERT (c == '!');

        /* Here the file-position indicator must be 2.  */

        c = ungetc ('!', stdin);
        ASSERT (c == '!');

        fflush (stdin);

        /* Here the file-position indicator must be 1.  */

        c = fgetc (stdin);
        ASSERT (c == '!');

        c = fgetc (stdin);
        ASSERT (c == '/');

        return 0;

      case '2':
        /* Check fflush after a non-backup ungetc() call.  This is case 2a in
           terms of
           <https://lists.gnu.org/r/bug-gnulib/2008-03/msg00131.html>,
           according to the Austin Group's resolution on 2009-01-08.  */
        /* Check that fflush after a non-backup ungetc() call discards the
           ungetc buffer.  This is mandated by POSIX
           <https://pubs.opengroup.org/onlinepubs/9699919799/functions/fflush.html>:
             "...any characters pushed back onto the stream by ungetc()
              or ungetwc() that have not subsequently been read from the
              stream shall be discarded..."  */

        c = fgetc (stdin);
        ASSERT (c == '#');

        c = fgetc (stdin);
        ASSERT (c == '!');

        /* Here the file-position indicator must be 2.  */

        c = ungetc ('@', stdin);
        ASSERT (c == '@');

        fflush (stdin);

        /* Here the file-position indicator must be 1.  */

        c = fgetc (stdin);
        ASSERT (c == '!');

        c = fgetc (stdin);
        ASSERT (c == '/');

        return 0;
      }

  return 1;
}
