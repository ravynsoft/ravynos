/* On some systems, mkdir ("foo/", 0700) fails because of the trailing
   slash.  On those systems, this wrapper removes the trailing slash.

   Copyright (C) 2001, 2003, 2006, 2008-2023 Free Software Foundation, Inc.

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

/* written by Jim Meyering */

#include <config.h>

/* Specification.  */
#include <sys/stat.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dirname.h"

/* Disable the definition of mkdir to rpl_mkdir (from the <sys/stat.h>
   substitute) in this file.  Otherwise, we'd get an endless recursion.  */
#undef mkdir

/* mingw's _mkdir() function has 1 argument, but we pass 2 arguments.
   Additionally, it declares _mkdir (and depending on compile flags, an
   alias mkdir), only in the nonstandard includes <direct.h> and <io.h>,
   which are included in the <sys/stat.h> override.  */
#if defined _WIN32 && ! defined __CYGWIN__
# define mkdir(name,mode) _mkdir (name)
# define maybe_unused _GL_UNUSED
#else
# define maybe_unused /* empty */
#endif

/* This function is required at least for NetBSD 1.5.2.  */

int
rpl_mkdir (char const *dir, maybe_unused mode_t mode)
{
  int ret_val;
  char *tmp_dir;
  size_t len = strlen (dir);

  if (len && dir[len - 1] == '/')
    {
      tmp_dir = strdup (dir);
      if (!tmp_dir)
        {
          /* Rather than rely on strdup-posix, we set errno ourselves.  */
          errno = ENOMEM;
          return -1;
        }
      strip_trailing_slashes (tmp_dir);
    }
  else
    {
      tmp_dir = (char *) dir;
    }
#if FUNC_MKDIR_DOT_BUG
  /* Additionally, cygwin 1.5 mistakenly creates a directory "d/./".  */
  {
    char *last = last_component (tmp_dir);
    if (*last == '.' && (last[1] == '\0'
                         || (last[1] == '.' && last[2] == '\0')))
      {
        struct stat st;
        if (stat (tmp_dir, &st) == 0 || errno == EOVERFLOW)
          errno = EEXIST;
        return -1;
      }
  }
#endif /* FUNC_MKDIR_DOT_BUG */

  ret_val = mkdir (tmp_dir, mode);

  if (tmp_dir != dir)
    free (tmp_dir);

  return ret_val;
}
