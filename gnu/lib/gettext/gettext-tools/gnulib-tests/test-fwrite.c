/* Test of fwrite() function.
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
SIGNATURE_CHECK (fwrite, size_t, (const void *, size_t, size_t, FILE *));

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
  const char *filename = "test-fwrite.txt";

  /* We don't have an fwrite() function that installs an invalid parameter
     handler so far.  So install that handler here, explicitly.  */
#if HAVE_MSVC_INVALID_PARAMETER_HANDLER \
    && MSVC_INVALID_PARAMETER_HANDLING == DEFAULT_HANDLING
  gl_msvc_inval_ensure_handler ();
#endif

  /* Test that fwrite() on an unbuffered stream sets errno if someone else
     closes the stream fd behind the back of stdio.  */
  #if !defined __ANDROID__ /* fdsan */
  {
    FILE *fp = fopen (filename, "w");
    char buf[5] = "world";
    ASSERT (fp != NULL);
    setvbuf (fp, NULL, _IONBF, 0);
    ASSERT (close (fileno (fp)) == 0);
    errno = 0;
    ASSERT (fwrite (buf, 1, sizeof (buf), fp) == 0);
    ASSERT (errno == EBADF);
    ASSERT (ferror (fp));
    fclose (fp);
  }
  #endif

  /* Test that fwrite() on an unbuffered stream sets errno if the stream
     was constructed with an invalid file descriptor.  */
  {
    FILE *fp = fdopen (-1, "w");
    if (fp != NULL)
      {
        char buf[5] = "world";
        setvbuf (fp, NULL, _IONBF, 0);
        errno = 0;
        ASSERT (fwrite (buf, 1, sizeof (buf), fp) == 0);
        ASSERT (errno == EBADF);
        ASSERT (ferror (fp));
        fclose (fp);
      }
  }
  {
    FILE *fp;
    close (99);
    fp = fdopen (99, "w");
    if (fp != NULL)
      {
        char buf[5] = "world";
        setvbuf (fp, NULL, _IONBF, 0);
        errno = 0;
        ASSERT (fwrite (buf, 1, sizeof (buf), fp) == 0);
        ASSERT (errno == EBADF);
        ASSERT (ferror (fp));
        fclose (fp);
      }
  }

  /* Clean up.  */
  unlink (filename);

  return 0;
}
