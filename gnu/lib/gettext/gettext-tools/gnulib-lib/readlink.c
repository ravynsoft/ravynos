/* Read the contents of a symbolic link.
   Copyright (C) 2003-2007, 2009-2023 Free Software Foundation, Inc.

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
#include <unistd.h>

#include <errno.h>
#include <string.h>
#include <sys/stat.h>

#if !HAVE_READLINK

/* readlink() substitute for systems that don't have a readlink() function,
   such as DJGPP 2.03 and mingw32.  */

ssize_t
readlink (char const *file, _GL_UNUSED char *buf,
          _GL_UNUSED size_t bufsize)
{
  struct stat statbuf;

  /* In general we should use lstat() here, not stat().  But on platforms
     without symbolic links, lstat() - if it exists - would be equivalent to
     stat(), therefore we can use stat().  This saves us a configure check.  */
  if (stat (file, &statbuf) >= 0)
    errno = EINVAL;
  return -1;
}

#else /* HAVE_READLINK */

# undef readlink

/* readlink() wrapper that uses correct types, for systems like cygwin
   1.5.x where readlink returns int, and which rejects trailing slash,
   for Solaris 9.  */

ssize_t
rpl_readlink (char const *file, char *buf, size_t bufsize)
{
# if READLINK_TRAILING_SLASH_BUG
  size_t file_len = strlen (file);
  if (file_len && file[file_len - 1] == '/')
    {
      /* Even if FILE without the slash is a symlink to a directory,
         both lstat() and stat() must resolve the trailing slash to
         the directory rather than the symlink.  We can therefore
         safely use stat() to distinguish between EINVAL and
         ENOTDIR/ENOENT, avoiding extra overhead of rpl_lstat().  */
      struct stat st;
      if (stat (file, &st) == 0 || errno == EOVERFLOW)
        errno = EINVAL;
      return -1;
    }
# endif /* READLINK_TRAILING_SLASH_BUG */

  ssize_t r = readlink (file, buf, bufsize);

# if READLINK_TRUNCATE_BUG
  if (r < 0 && errno == ERANGE)
    {
      /* Try again with a bigger buffer.  This is just for test cases;
         real code invariably discards short reads.  */
      char stackbuf[4032];
      r = readlink (file, stackbuf, sizeof stackbuf);
      if (r < 0)
        {
          if (errno == ERANGE)
            {
              /* Clear the buffer, which is good enough for real code.
                 Thankfully, no test cases try short reads of enormous
                 symlinks and what would be the point anyway?  */
              r = bufsize;
              memset (buf, 0, r);
            }
        }
      else
        {
          if (bufsize < r)
            r = bufsize;
          memcpy (buf, stackbuf, r);
        }
    }
# endif

  return r;
}

#endif /* HAVE_READLINK */
