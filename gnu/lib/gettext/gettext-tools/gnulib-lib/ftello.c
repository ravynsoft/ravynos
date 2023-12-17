/* An ftello() function that works around platform bugs.
   Copyright (C) 2007, 2009-2023 Free Software Foundation, Inc.

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#include <config.h>

/* Specification.  */
#include <stdio.h>

#include <errno.h>
#include "intprops.h"

/* Get lseek.  */
#include <unistd.h>

#include "stdio-impl.h"

off_t
ftello (FILE *fp)
#undef ftello
#if !HAVE_FTELLO
# undef ftell
# define ftello ftell
#endif
#if _GL_WINDOWS_64_BIT_OFF_T
# undef ftello
# if HAVE__FTELLI64 /* msvc, mingw64 */
#  define ftello _ftelli64
# else /* mingw */
#  define ftello ftello64
# endif
#endif
{
#if FTELLO_BROKEN_AFTER_UNGETC /* macOS >= 10.15 */
  /* The system's ftello() is completely broken, because it calls __sflush,
     which makes side effects on the stream.  */

  /* Handle non-seekable files first.  */
  if (fp->_file < 0 || fp->_seek == NULL)
    {
      errno = ESPIPE;
      return -1;
    }

  /* Determine the current offset, ignoring buffered and pushed-back bytes.  */
  off_t pos;

  if (fp->_flags & __SOFF)
    pos = fp->_offset;
  else
    {
      pos = fp->_seek (fp->_cookie, 0, SEEK_CUR);
      if (pos < 0)
        return -1;
      if (fp->_flags & __SOPT)
        {
          fp->_offset = pos;
          fp->_flags |= __SOFF;
        }
    }

  if (fp->_flags & __SRD)
    {
      /* Now consider buffered and pushed-back bytes from ungetc.  */
      if (fp->_ub._base != NULL)
        /* Considering the buffered bytes, we are at position
             pos - fp->_ur.
           Considering also the pushed-back bytes, we are at position
             pos - fp->_ur - fp->_r.  */
        pos = pos - fp->_ur - fp->_r;
      else
        /* Considering the buffered bytes, we are at position
             pos - fp->_r.  */
        pos = pos - fp->_r;
      if (pos < 0)
        {
          errno = EIO;
          return -1;
        }
    }
  else if ((fp->_flags & __SWR) && fp->_p != NULL)
    {
      /* Consider the buffered bytes.  */
      off_t buffered = fp->_p - fp->_bf._base;

      /* Compute pos + buffered, with overflow check.  */
      off_t sum;
      if (! INT_ADD_OK (pos, buffered, &sum))
        {
          errno = EOVERFLOW;
          return -1;
        }
      pos = sum;
    }

  return pos;

#else

# if LSEEK_PIPE_BROKEN
  /* mingw gives bogus answers rather than failure on non-seekable files.  */
  if (lseek (fileno (fp), 0, SEEK_CUR) == -1)
    return -1;
# endif

# if FTELLO_BROKEN_AFTER_SWITCHING_FROM_READ_TO_WRITE /* Solaris */
  /* The Solaris stdio leaves the _IOREAD flag set after reading from a file
     reaches EOF and the program then starts writing to the file.  ftello
     gets confused by this.  */
  if (fp_->_flag & _IOWRT)
    {
      off_t pos;

      /* Call ftello nevertheless, for the side effects that it does on fp.  */
      ftello (fp);

      /* Compute the file position ourselves.  */
      pos = lseek (fileno (fp), (off_t) 0, SEEK_CUR);
      if (pos >= 0)
        {
          if ((fp_->_flag & _IONBF) == 0 && fp_->_base != NULL)
            pos += fp_->_ptr - fp_->_base;
        }
      return pos;
    }
# endif

# if defined __SL64 && defined __SCLE /* Cygwin */
  if ((fp->_flags & __SL64) == 0)
    {
      /* Cygwin 1.5.0 through 1.5.24 failed to open stdin in 64-bit
         mode; but has an ftello that requires 64-bit mode.  */
      FILE *tmp = fopen ("/dev/null", "r");
      if (!tmp)
        return -1;
      fp->_flags |= __SL64;
      fp->_seek64 = tmp->_seek64;
      fclose (tmp);
    }
# endif

  return ftello (fp);

#endif
}
