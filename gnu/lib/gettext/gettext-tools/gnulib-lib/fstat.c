/* fstat() replacement.
   Copyright (C) 2011-2023 Free Software Foundation, Inc.

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

/* If the user's config.h happens to include <sys/stat.h>, let it include only
   the system's <sys/stat.h> here, so that orig_fstat doesn't recurse to
   rpl_fstat.  */
#define __need_system_sys_stat_h
#include <config.h>

/* Get the original definition of fstat.  It might be defined as a macro.  */
#include <sys/types.h>
#include <sys/stat.h>
#undef __need_system_sys_stat_h

#if defined _WIN32 && ! defined __CYGWIN__
# define WINDOWS_NATIVE
#endif

#if !defined WINDOWS_NATIVE

static int
orig_fstat (int fd, struct stat *buf)
{
  return fstat (fd, buf);
}

#endif

/* Specification.  */
#ifdef __osf__
/* Write "sys/stat.h" here, not <sys/stat.h>, otherwise OSF/1 5.1 DTK cc
   eliminates this include because of the preliminary #include <sys/stat.h>
   above.  */
# include "sys/stat.h"
#else
# include <sys/stat.h>
#endif

#include "stat-time.h"

#include <errno.h>
#include <unistd.h>
#ifdef WINDOWS_NATIVE
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
# if GNULIB_MSVC_NOTHROW
#  include "msvc-nothrow.h"
# else
#  include <io.h>
# endif
# include "stat-w32.h"
#endif

int
rpl_fstat (int fd, struct stat *buf)
{
#if REPLACE_FCHDIR && REPLACE_OPEN_DIRECTORY
  /* Handle the case when rpl_open() used a dummy file descriptor to work
     around an open() that can't normally visit directories.  */
  const char *name = _gl_directory_name (fd);
  if (name != NULL)
    return stat (name, buf);
#endif

#ifdef WINDOWS_NATIVE
  /* Fill the fields ourselves, because the original fstat function returns
     values for st_atime, st_mtime, st_ctime that depend on the current time
     zone.  See
     <https://lists.gnu.org/r/bug-gnulib/2017-04/msg00134.html>  */
  HANDLE h = (HANDLE) _get_osfhandle (fd);

  if (h == INVALID_HANDLE_VALUE)
    {
      errno = EBADF;
      return -1;
    }
  return _gl_fstat_by_handle (h, NULL, buf);
#else
  return stat_time_normalize (orig_fstat (fd, buf), buf);
#endif
}
