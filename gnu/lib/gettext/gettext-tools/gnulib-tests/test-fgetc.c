/* Test of fgetc() function.
   Copyright (C) 2011-2023 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, see <https://www.gnu.org/licenses/>.  */

#include <config.h>

#include <stdio.h>

#include "signature.h"
SIGNATURE_CHECK (fgetc, int, (FILE *));

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#if HAVE_MSVC_INVALID_PARAMETER_HANDLER
# include "msvc-inval.h"
#endif

#include "macros.h"

int
main ()
{
  const char *filename = "test-fgetc.txt";

  /* We don't have an fgetc() function that installs an invalid parameter
     handler so far.  So install that handler here, explicitly.  */
#if HAVE_MSVC_INVALID_PARAMETER_HANDLER \
    && MSVC_INVALID_PARAMETER_HANDLING == DEFAULT_HANDLING
  gl_msvc_inval_ensure_handler ();
#endif

  /* Prepare a file.  */
  {
    const char text[] = "hello world";
    int fd = open (filename, O_RDWR | O_CREAT | O_TRUNC, 0600);
    ASSERT (fd >= 0);
    ASSERT (write (fd, text, sizeof (text)) == sizeof (text));
    ASSERT (close (fd) == 0);
  }

  /* Test that fgetc() sets errno if someone else closes the stream
     fd behind the back of stdio.  */
  #if !defined __ANDROID__ /* fdsan */
  {
    FILE *fp = fopen (filename, "r");
    ASSERT (fp != NULL);
    ASSERT (close (fileno (fp)) == 0);
    errno = 0;
    ASSERT (fgetc (fp) == EOF);
    ASSERT (errno == EBADF);
    ASSERT (ferror (fp));
    fclose (fp);
  }
  #endif

  /* Test that fgetc() sets errno if the stream was constructed with
     an invalid file descriptor.  */
  {
    FILE *fp = fdopen (-1, "r");
    if (fp != NULL)
      {
        errno = 0;
        ASSERT (fgetc (fp) == EOF);
        ASSERT (errno == EBADF);
        ASSERT (ferror (fp));
        fclose (fp);
      }
  }
  {
    FILE *fp;
    close (99);
    fp = fdopen (99, "r");
    if (fp != NULL)
      {
        errno = 0;
        ASSERT (fgetc (fp) == EOF);
        ASSERT (errno == EBADF);
        ASSERT (ferror (fp));
        fclose (fp);
      }
  }

  /* Clean up.  */
  unlink (filename);

  return 0;
}
