/* provide a replacement openat function
   Copyright (C) 2004-2006, 2008-2023 Free Software Foundation, Inc.

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

/* written by Jim Meyering */

#ifndef _GL_HEADER_OPENAT
#define _GL_HEADER_OPENAT

/* This file uses _GL_INLINE_HEADER_BEGIN, _GL_INLINE, _Noreturn,
   _GL_ATTRIBUTE_DEPRECATED, HAVE_OPENAT.  */
#if !_GL_CONFIG_H_INCLUDED
 #error "Please include config.h first."
#endif

#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

_GL_INLINE_HEADER_BEGIN

#if !HAVE_OPENAT

int openat_permissive (int fd, char const *file, int flags, mode_t mode,
                       int *cwd_errno);
bool openat_needs_fchdir (void);

#else

# define openat_permissive(Fd, File, Flags, Mode, Cwd_errno) \
    openat (Fd, File, Flags, Mode)
# define openat_needs_fchdir() false

#endif

_Noreturn void openat_restore_fail (int);
_Noreturn void openat_save_fail (int);

/* Using these function names makes application code
   slightly more readable than it would be with
   fchownat (..., 0) or fchownat (..., AT_SYMLINK_NOFOLLOW).  */

#if GNULIB_CHOWNAT

# ifndef CHOWNAT_INLINE
#  define CHOWNAT_INLINE _GL_INLINE
# endif

CHOWNAT_INLINE int
chownat (int fd, char const *file, uid_t owner, gid_t group)
{
  return fchownat (fd, file, owner, group, 0);
}

CHOWNAT_INLINE int
lchownat (int fd, char const *file, uid_t owner, gid_t group)
{
  return fchownat (fd, file, owner, group, AT_SYMLINK_NOFOLLOW);
}

#endif

#if GNULIB_CHMODAT

# ifndef CHMODAT_INLINE
#  define CHMODAT_INLINE _GL_INLINE
# endif

CHMODAT_INLINE int
chmodat (int fd, char const *file, mode_t mode)
{
  return fchmodat (fd, file, mode, 0);
}

CHMODAT_INLINE int
lchmodat (int fd, char const *file, mode_t mode)
{
  return fchmodat (fd, file, mode, AT_SYMLINK_NOFOLLOW);
}

#endif

#if GNULIB_STATAT

# ifndef STATAT_INLINE
#  define STATAT_INLINE _GL_INLINE
# endif

_GL_ATTRIBUTE_DEPRECATED
STATAT_INLINE int
statat (int fd, char const *name, struct stat *st)
{
  return fstatat (fd, name, st, 0);
}

_GL_ATTRIBUTE_DEPRECATED
STATAT_INLINE int
lstatat (int fd, char const *name, struct stat *st)
{
  return fstatat (fd, name, st, AT_SYMLINK_NOFOLLOW);
}

#endif

/* For now, there are no wrappers named laccessat or leuidaccessat,
   since gnulib doesn't support faccessat(,AT_SYMLINK_NOFOLLOW) and
   since access rights on symlinks are of limited utility.  Likewise,
   wrappers are not provided for accessat or euidaccessat, so as to
   avoid dragging in -lgen on some platforms.  */

_GL_INLINE_HEADER_END

#endif /* _GL_HEADER_OPENAT */
