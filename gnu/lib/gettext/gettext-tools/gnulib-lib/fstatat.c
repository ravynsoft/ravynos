/* Work around an fstatat bug on Solaris 9.

   Copyright (C) 2006, 2009-2023 Free Software Foundation, Inc.

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

/* Written by Paul Eggert and Jim Meyering.  */

/* If the user's config.h happens to include <sys/stat.h>, let it include only
   the system's <sys/stat.h> here, so that orig_fstatat doesn't recurse to
   rpl_fstatat.  */
#define __need_system_sys_stat_h
#include <config.h>

/* Get the original definition of fstatat.  It might be defined as a macro.  */
#include <sys/types.h>
#include <sys/stat.h>
#undef __need_system_sys_stat_h

#if HAVE_FSTATAT && HAVE_WORKING_FSTATAT_ZERO_FLAG
static int
orig_fstatat (int fd, char const *filename, struct stat *buf, int flags)
{
  return fstatat (fd, filename, buf, flags);
}
#endif

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
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#if HAVE_FSTATAT && HAVE_WORKING_FSTATAT_ZERO_FLAG

# ifndef LSTAT_FOLLOWS_SLASHED_SYMLINK
#  define LSTAT_FOLLOWS_SLASHED_SYMLINK 0
# endif

static int
normal_fstatat (int fd, char const *file, struct stat *st, int flag)
{
  return stat_time_normalize (orig_fstatat (fd, file, st, flag), st);
}

/* fstatat should always follow symbolic links that end in /, but on
   Solaris 9 it doesn't if AT_SYMLINK_NOFOLLOW is specified.
   Likewise, trailing slash on a non-directory should be an error.
   These are the same problems that lstat.c and stat.c address, so
   solve it in a similar way.

   AIX 7.1 fstatat (AT_FDCWD, ..., 0) always fails, which is a bug.
   Work around this bug if FSTATAT_AT_FDCWD_0_BROKEN is nonzero.  */

int
rpl_fstatat (int fd, char const *file, struct stat *st, int flag)
{
  int result = normal_fstatat (fd, file, st, flag);
  size_t len;

  if (LSTAT_FOLLOWS_SLASHED_SYMLINK || result != 0)
    return result;
  len = strlen (file);
  if (flag & AT_SYMLINK_NOFOLLOW)
    {
      /* Fix lstat behavior.  */
      if (file[len - 1] != '/' || S_ISDIR (st->st_mode))
        return 0;
      if (!S_ISLNK (st->st_mode))
        {
          errno = ENOTDIR;
          return -1;
        }
      result = normal_fstatat (fd, file, st, flag & ~AT_SYMLINK_NOFOLLOW);
    }
  /* Fix stat behavior.  */
  if (result == 0 && !S_ISDIR (st->st_mode) && file[len - 1] == '/')
    {
      errno = ENOTDIR;
      return -1;
    }
  return result;
}

#else /* ! (HAVE_FSTATAT && HAVE_WORKING_FSTATAT_ZERO_FLAG) */

/* On mingw, the gnulib <sys/stat.h> defines 'stat' as a function-like
   macro; but using it in AT_FUNC_F2 causes compilation failure
   because the preprocessor sees a use of a macro that requires two
   arguments but is only given one.  Hence, we need an inline
   forwarder to get past the preprocessor.  */
static int
stat_func (char const *name, struct stat *st)
{
  return stat (name, st);
}

/* Likewise, if there is no native 'lstat', then the gnulib
   <sys/stat.h> defined it as stat, which also needs adjustment.  */
# if !HAVE_LSTAT
#  undef lstat
#  define lstat stat_func
# endif

/* Replacement for Solaris' function by the same name.
   <https://www.google.com/search?q=fstatat+site:docs.oracle.com>
   First, try to simulate it via l?stat ("/proc/self/fd/FD/FILE").
   Failing that, simulate it via save_cwd/fchdir/(stat|lstat)/restore_cwd.
   If either the save_cwd or the restore_cwd fails (relatively unlikely),
   then give a diagnostic and exit nonzero.
   Otherwise, this function works just like Solaris' fstatat.  */

# define AT_FUNC_NAME fstatat
# define AT_FUNC_F1 lstat
# define AT_FUNC_F2 stat_func
# define AT_FUNC_USE_F1_COND AT_SYMLINK_NOFOLLOW
# define AT_FUNC_POST_FILE_PARAM_DECLS , struct stat *st, int flag
# define AT_FUNC_POST_FILE_ARGS        , st
# include "at-func.c"
# undef AT_FUNC_NAME
# undef AT_FUNC_F1
# undef AT_FUNC_F2
# undef AT_FUNC_USE_F1_COND
# undef AT_FUNC_POST_FILE_PARAM_DECLS
# undef AT_FUNC_POST_FILE_ARGS

#endif /* !HAVE_FSTATAT */
