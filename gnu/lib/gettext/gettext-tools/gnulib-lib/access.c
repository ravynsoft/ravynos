/* Test the access rights of a file.
   Copyright (C) 2019-2023 Free Software Foundation, Inc.

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
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#if defined _WIN32 && !defined __CYGWIN__
# include <io.h>
#endif

int
access (const char *file, int mode)
#undef access
{
  int ret;

#if defined _WIN32 && !defined __CYGWIN__
  if ((mode & X_OK) != 0)
    mode = (mode & ~X_OK) | R_OK;
  ret = _access (file, mode);
#else
  ret = access (file, mode);
#endif

#if (defined _WIN32 && !defined __CYGWIN__) || ACCESS_TRAILING_SLASH_BUG
# if defined _WIN32 && !defined __CYGWIN__
  if (ret == 0 || errno == EINVAL)
# else
  if (ret == 0)
# endif
    {
      size_t file_len = strlen (file);
      if (file_len > 0 && file[file_len - 1] == '/')
        {
          struct stat st;
          if (stat (file, &st) == 0)
            {
              if (! S_ISDIR (st.st_mode))
                {
                  errno = ENOTDIR;
                  return -1;
                }
            }
          else
            return (mode == F_OK && errno == EOVERFLOW ? 0 : -1);
        }
    }
#endif
  return ret;
}
