/* Binary mode I/O.
   Copyright (C) 2001-2023 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#ifndef _BINARY_H
#define _BINARY_H

/* Include this header after <fcntl.h> and <stdio.h>, because
   systems that distinguish between text and binary I/O usually
   define O_BINARY in <fcntl.h>, and the MSVC7 <stdio.h> doesn't
   like to be included after '#define fileno ...'

   We don't include <fcntl.h> here because not all systems have
   that header.  */

#if !defined O_BINARY && defined _O_BINARY
  /* For MSC-compatible compilers.  */
# define O_BINARY _O_BINARY
# define O_TEXT _O_TEXT
#endif
#ifdef __BEOS__
  /* BeOS 5 has O_BINARY and O_TEXT, but they have no effect.  */
# undef O_BINARY
# undef O_TEXT
#endif
#if O_BINARY
# if defined __EMX__ || defined __DJGPP__ || defined __CYGWIN__
#  include <io.h> /* declares setmode() */
# else
#  define setmode _setmode
#  undef fileno
#  define fileno _fileno
# endif
# ifdef __DJGPP__
#  include <unistd.h> /* declares isatty() */
#  /* Avoid putting stdin/stdout in binary mode if it is connected to the
#     console, because that would make it impossible for the user to
#     interrupt the program through Ctrl-C or Ctrl-Break.  */
#  define SET_BINARY(fd) (!isatty (fd) ? (setmode (fd, O_BINARY), 0) : 0)
# else
#  define SET_BINARY(fd) setmode (fd, O_BINARY)
# endif
#else
  /* On reasonable systems, binary I/O is the default.  */
# undef O_BINARY
# define O_BINARY 0
# define SET_BINARY(fd) /* nothing */
#endif

#endif /* _BINARY_H */
