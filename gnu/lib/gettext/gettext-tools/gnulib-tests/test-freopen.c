/* Test of opening a file stream.
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

#include <stdio.h>

#include "signature.h"
SIGNATURE_CHECK (freopen, FILE *, (char const *, char const *, FILE *));

#include <errno.h>
#include <unistd.h>

#include "macros.h"

int
main ()
{
  const char *filename = "test-freopen.txt";

  close (STDIN_FILENO);
  ASSERT (freopen ("/dev/null", "r", stdin) != NULL);
  ASSERT (getchar () == EOF);
  ASSERT (!ferror (stdin));
  ASSERT (feof (stdin));

#if 0 /* freopen (NULL, ...) is unsupported on most platforms.  */
  /* Test that freopen() sets errno if someone else closes the stream
     fd behind the back of stdio.  */
  {
    FILE *fp = fopen (filename, "w+");
    ASSERT (fp != NULL);
    ASSERT (close (fileno (fp)) == 0);
    errno = 0;
    ASSERT (freopen (NULL, "r", fp) == NULL);
    perror("freopen");
    ASSERT (errno == EBADF);
    fclose (fp);
  }

  /* Test that freopen() sets errno if the stream was constructed with
     an invalid file descriptor.  */
  {
    FILE *fp = fdopen (-1, "w+");
    if (fp != NULL)
      {
        errno = 0;
        ASSERT (freopen (NULL, "r", fp) == NULL);
        ASSERT (errno == EBADF);
        fclose (fp);
      }
  }
  {
    FILE *fp;
    close (99);
    fp = fdopen (99, "w+");
    if (fp != NULL)
      {
        errno = 0;
        ASSERT (freopen (NULL, "r", fp) == NULL);
        ASSERT (errno == EBADF);
        fclose (fp);
      }
  }
#endif

  /* Clean up.  */
  unlink (filename);

  return 0;
}
