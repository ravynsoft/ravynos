/* Determine whether two stat buffers are known to refer to the same file.

   Copyright (C) 2006, 2009-2023 Free Software Foundation, Inc.

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

#ifndef SAME_INODE_H
#define SAME_INODE_H 1

/* This file uses _GL_INLINE_HEADER_BEGIN, _GL_INLINE.  */
#if !_GL_CONFIG_H_INCLUDED
 #error "Please include config.h first."
#endif

#include <sys/stat.h>

_GL_INLINE_HEADER_BEGIN
#ifndef SAME_INODE_INLINE
# define SAME_INODE_INLINE _GL_INLINE
#endif

/* True if A and B point to structs with st_dev and st_ino members
   that are known to represent the same file.

   Use | and ^ to shorten generated code, and to lessen the
   probability of screwups if st_ino is an array.  */

#if defined __VMS && __CRTL_VER < 80200000
# define PSAME_INODE(a, b) (! (((a)->st_dev ^ (b)->st_dev) \
                               | ((a)->st_ino[0] ^ (b)->st_ino[0]) \
                               | ((a)->st_ino[1] ^ (b)->st_ino[1]) \
                               | ((a)->st_ino[2] ^ (b)->st_ino[2])))
#elif defined _WIN32 && ! defined __CYGWIN__
  /* Native Windows.  */
# if _GL_WINDOWS_STAT_INODES
  /* stat() and fstat() set st_dev and st_ino to 0 if information about
     the inode is not available.  */
#  if _GL_WINDOWS_STAT_INODES == 2
#   define PSAME_INODE(a, b) \
     (! (! ((a)->st_dev | (a)->st_ino._gl_ino[0] | (a)->st_ino._gl_ino[1]) \
         | ((a)->st_dev ^ (b)->st_dev) \
         | ((a)->st_ino._gl_ino[0] ^ (b)->st_ino._gl_ino[0]) \
         | ((a)->st_ino._gl_ino[1] ^ (b)->st_ino._gl_ino[1])))
#  else
#   define PSAME_INODE(a, b) (! (! ((a)->st_dev | (a)->st_ino) \
                                 | ((a)->st_dev ^ (b)->st_dev) \
                                 | ((a)->st_ino ^ (b)->st_ino)))
#  endif
# else
  /* stat() and fstat() set st_ino to 0 always.  */
#  define PSAME_INODE(a, b) 0
# endif
#else
  /* POSIX.  */
# define PSAME_INODE(a, b) (! (((a)->st_dev ^ (b)->st_dev) \
                               | ((a)->st_ino ^ (b)->st_ino)))
#endif

/* True if struct objects A and B are known to represent the same file.  */

#define SAME_INODE(a, b) PSAME_INODE (&(a), &(b))

/* True if *A and *B represent the same file.  Unlike PSAME_INODE,
   args are evaluated once and must point to struct stat.  */

SAME_INODE_INLINE bool
psame_inode (struct stat const *a, struct stat const *b)
{
  return PSAME_INODE (a, b);
}

_GL_INLINE_HEADER_END

#endif
