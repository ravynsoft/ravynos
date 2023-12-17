/* Internals for openat-like functions.

   Copyright (C) 2005-2006, 2009-2023 Free Software Foundation, Inc.

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

#ifndef _GL_HEADER_OPENAT_PRIV
#define _GL_HEADER_OPENAT_PRIV

#include <errno.h>
#include <limits.h>
#include <stdlib.h>

/* Maximum number of bytes that it is safe to allocate as a single
   array on the stack, and that is known as a compile-time constant.
   The assumption is that we'll touch the array very quickly, or a
   temporary very near the array, provoking an out-of-memory trap.  On
   some operating systems, there is only one guard page for the stack,
   and a page size can be as small as 4096 bytes.  Subtract 64 in the
   hope that this will let the compiler touch a nearby temporary and
   provoke a trap.  */
#define SAFER_ALLOCA_MAX (4096 - 64)

#define SAFER_ALLOCA(m) ((m) < SAFER_ALLOCA_MAX ? (m) : SAFER_ALLOCA_MAX)

#if defined PATH_MAX
# define OPENAT_BUFFER_SIZE SAFER_ALLOCA (PATH_MAX)
#elif defined _XOPEN_PATH_MAX
# define OPENAT_BUFFER_SIZE SAFER_ALLOCA (_XOPEN_PATH_MAX)
#else
# define OPENAT_BUFFER_SIZE SAFER_ALLOCA (1024)
#endif

char *openat_proc_name (char buf[OPENAT_BUFFER_SIZE], int fd, char const *file);

/* Trying to access a BUILD_PROC_NAME file will fail on systems without
   /proc support, and even on systems *with* ProcFS support.  Return
   nonzero if the failure may be legitimate, e.g., because /proc is not
   readable, or the particular .../fd/N directory is not present.  */
#define EXPECTED_ERRNO(Errno)                   \
  ((Errno) == ENOTDIR || (Errno) == ENOENT      \
   || (Errno) == EPERM || (Errno) == EACCES     \
   || (Errno) == ENOSYS /* Solaris 8 */         \
   || (Errno) == EOPNOTSUPP /* FreeBSD */)

/* Wrapper function shared among linkat and renameat.  */
int at_func2 (int fd1, char const *file1,
              int fd2, char const *file2,
              int (*func) (char const *file1, char const *file2));

#endif /* _GL_HEADER_OPENAT_PRIV */
