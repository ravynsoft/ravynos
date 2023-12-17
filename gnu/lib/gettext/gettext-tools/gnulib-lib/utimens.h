/* Set file access and modification times.

   Copyright 2012-2023 Free Software Foundation, Inc.

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation, either version 3 of the
   License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Written by Paul Eggert.  */

/* This file uses _GL_INLINE_HEADER_BEGIN, _GL_INLINE.  */
#if !_GL_CONFIG_H_INCLUDED
 #error "Please include config.h first."
#endif

#include <time.h>
int fdutimens (int, char const *, struct timespec const [2]);
int utimens (char const *, struct timespec const [2]);
int lutimens (char const *, struct timespec const [2]);

#if GNULIB_FDUTIMENSAT
# include <fcntl.h>
# include <sys/stat.h>

_GL_INLINE_HEADER_BEGIN
#ifndef _GL_UTIMENS_INLINE
# define _GL_UTIMENS_INLINE _GL_INLINE
#endif

int fdutimensat (int fd, int dir, char const *name, struct timespec const [2],
                 int atflag);

/* Using this function makes application code slightly more readable.  */
_GL_UTIMENS_INLINE int
lutimensat (int dir, char const *file, struct timespec const times[2])
{
  return utimensat (dir, file, times, AT_SYMLINK_NOFOLLOW);
}

_GL_INLINE_HEADER_END

#endif
