/* Open a descriptor to a file.
   Copyright (C) 2007-2026 Free Software Foundation, Inc.

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

/* Written by Bruno Haible <bruno@clisp.org>, 2007.  */

/* If the user's config.h happens to include <fcntl.h>, let it include only
   the system's <fcntl.h> here, so that orig_open doesn't recurse to
   rpl_open.  */
#define __need_system_fcntl_h
#include <config.h>

/* Get the original definition of open.  It might be defined as a macro.  */
#include <fcntl.h>
#include <sys/types.h>
#undef __need_system_fcntl_h

static int
orig_open (const char *filename, int flags, mode_t mode)
{
#if defined _WIN32 && !defined __CYGWIN__
  return _open (filename, flags, mode);
#else
  return open (filename, flags, mode);
#endif
}

/* Specification.  */
#include <fcntl.h>

#include "cloexec.h"

#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#ifndef HAVE_WORKING_O_DIRECTORY
# define HAVE_WORKING_O_DIRECTORY false
#endif

#ifndef OPEN_TRAILING_SLASH_BUG
# define OPEN_TRAILING_SLASH_BUG false
#endif

#ifndef REPLACE_OPEN_DIRECTORY
# define REPLACE_OPEN_DIRECTORY false
#endif

static int
lstatif (char const *filename, struct stat *st, int flags)
{
  return flags & O_NOFOLLOW ? lstat (filename, st) : stat (filename, st);
}

int
open (const char *filename, int flags, ...)
{
  mode_t mode = 0;

  if (flags & O_CREAT)
    {
      va_list arg;
      va_start (arg, flags);

      /* We have to use PROMOTED_MODE_T instead of mode_t, otherwise GCC 4
         creates crashing code when 'mode_t' is smaller than 'int'.  */
      mode = va_arg (arg, PROMOTED_MODE_T);

      va_end (arg);
    }

#if GNULIB_defined_O_NONBLOCK
  /* The only known platform that lacks O_NONBLOCK is mingw, but it
     also lacks named pipes and Unix sockets, which are the only two
     file types that require non-blocking handling in open().
     Therefore, it is safe to ignore O_NONBLOCK here.  It is handy
     that mingw also lacks openat(), so that is also covered here.  */
  flags &= ~O_NONBLOCK;
#endif

#if defined _WIN32 && ! defined __CYGWIN__
  if (streq (filename, "/dev/null"))
    filename = "NUL";
#endif

  /* Fail if one of O_CREAT, O_WRONLY, O_RDWR is specified and the filename
     ends in a slash, as POSIX says such a filename must name a directory
     <https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap04.html#tag_04_13>:
       "A pathname that contains at least one non-<slash> character and that
        ends with one or more trailing <slash> characters shall not be resolved
        successfully unless the last pathname component before the trailing
        <slash> characters names an existing directory"
     If the named file already exists as a directory, then
       - if O_CREAT is specified, open() must fail because of the semantics
         of O_CREAT,
       - if O_WRONLY or O_RDWR is specified, open() must fail because POSIX
         <https://pubs.opengroup.org/onlinepubs/9699919799/functions/open.html>
         says that it fails with errno = EISDIR in this case.
     If the named file does not exist or does not name a directory, then
       - if O_CREAT is specified, open() must fail since open() cannot create
         directories,
       - if O_WRONLY or O_RDWR is specified, open() must fail because the
         file does not contain a '.' directory.  */
  bool check_for_slash_bug;
  if (OPEN_TRAILING_SLASH_BUG)
    {
      size_t len = strlen (filename);
      check_for_slash_bug = len && filename[len - 1] == '/';
    }
  else
    check_for_slash_bug = false;

  if (check_for_slash_bug
      && (flags & O_CREAT
          || (flags & O_ACCMODE) == O_RDWR
          || (flags & O_ACCMODE) == O_WRONLY))
    {
      errno = EISDIR;
      return -1;
    }

  /* With the trailing slash bug or without working O_DIRECTORY, check with
     stat first lest we hang trying to open a fifo.  Although there is
     a race between this and opening the file, we can do no better.
     After opening the file we will check again with fstat.  */
  bool check_directory =
    (check_for_slash_bug
     || (!HAVE_WORKING_O_DIRECTORY && flags & O_DIRECTORY));
  if (check_directory)
    {
      struct stat statbuf;
      if (lstatif (filename, &statbuf, flags) < 0)
        {
          if (! (flags & O_CREAT && errno == ENOENT))
            return -1;
        }
      else if (!S_ISDIR (statbuf.st_mode))
        {
          errno = ENOTDIR;
          return -1;
        }
    }

  /* 0 = unknown, 1 = yes, -1 = no.  */
#if GNULIB_defined_O_CLOEXEC
  int have_cloexec = -1;
#else
  static int have_cloexec;
#endif

  int fd = orig_open (filename,
                      flags & ~(have_cloexec < 0 ? O_CLOEXEC : 0), mode);

  if (flags & O_CLOEXEC)
    {
      if (! have_cloexec)
        {
          if (0 <= fd)
            have_cloexec = 1;
          else if (errno == EINVAL)
            {
              fd = orig_open (filename, flags & ~O_CLOEXEC, mode);
              have_cloexec = -1;
            }
        }
      if (have_cloexec < 0 && 0 <= fd)
        set_cloexec_flag (fd, true);
    }


#if REPLACE_FCHDIR
  /* Implementing fchdir and fdopendir requires the ability to open a
     directory file descriptor.  If open doesn't support that (as on
     mingw), use a dummy file that behaves the same as directories
     on Linux (ie. always reports EOF on attempts to read()), and
     override fstat in fchdir.c to hide the dummy.  */
  if (REPLACE_OPEN_DIRECTORY && fd < 0 && errno == EACCES
      && ((flags & (O_ACCMODE | O_CREAT)) == O_RDONLY
          || (O_SEARCH != O_RDONLY
              && (flags & (O_ACCMODE | O_CREAT)) == O_SEARCH)))
    {
      struct stat statbuf;
      if (check_directory
          || (lstatif (filename, &statbuf, flags) == 0
              && S_ISDIR (statbuf.st_mode)))
        {
          /* Maximum recursion depth of 1.  */
          fd = open ("/dev/null", flags & ~O_DIRECTORY, mode);
          if (0 <= fd)
            fd = _gl_register_fd (fd, filename);
        }
      else
        errno = EACCES;
    }
#endif

  /* If checking for directories, fail if fd does not refer to a directory.
     Rationale: A filename ending in slash cannot name a non-directory
     <https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap04.html#tag_04_13>:
       "A pathname that contains at least one non-<slash> character and that
        ends with one or more trailing <slash> characters shall not be resolved
        successfully unless the last pathname component before the trailing
        <slash> characters names an existing directory"
     If the named file without the slash is not a directory, open() must fail
     with ENOTDIR.  */
  if (check_directory && 0 <= fd)
    {
      struct stat statbuf;
      int r = fstat (fd, &statbuf);
      if (r < 0 || !S_ISDIR (statbuf.st_mode))
        {
          int err = r < 0 ? errno : ENOTDIR;
          close (fd);
          errno = err;
          return -1;
        }
    }

#if REPLACE_FCHDIR
  if (!REPLACE_OPEN_DIRECTORY && 0 <= fd)
    fd = _gl_register_fd (fd, filename);
#endif

  return fd;
}
