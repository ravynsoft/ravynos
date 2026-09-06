/* Read symbolic links into a buffer without size limitation, relative to fd.

   Copyright (C) 2001, 2003-2004, 2007, 2009-2026 Free Software Foundation,
   Inc.

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

/* Written by Paul Eggert, Bruno Haible, and Jim Meyering.  */

#include <config.h>

#include "careadlinkat.h"

#include "idx.h"
#include "minmax.h"

#include <errno.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>

/* Define this independently so that stdint.h is not a prerequisite.  */
#ifndef SIZE_MAX
# define SIZE_MAX ((size_t) -1)
#endif

#include "allocator.h"

enum { STACK_BUF_SIZE = 1024 };

/* Act like careadlinkat (see below), with an additional argument
   STACK_BUF that can be used as temporary storage.

   If GCC_LINT is defined, do not inline this function with GCC 10.1
   and later, to avoid creating a pointer to the stack that GCC
   -Wreturn-local-addr incorrectly complains about.  See:
   https://gcc.gnu.org/PR93644
   Although the noinline attribute can hurt performance a bit, no better way
   to pacify GCC is known; even an explicit #pragma does not pacify GCC.
   When the GCC bug is fixed this workaround should be limited to the
   broken GCC versions.  */
#if _GL_GNUC_PREREQ (10, 1)
# if _GL_GNUC_PREREQ (12, 1)
#  pragma GCC diagnostic ignored "-Wreturn-local-addr"
# elif defined GCC_LINT || defined lint
__attribute__ ((__noinline__))
# elif __OPTIMIZE__ && !__NO_INLINE__
#  define GCC_BOGUS_WRETURN_LOCAL_ADDR
# endif
#endif
static char *
readlink_stk (int fd, char const *filename,
              char *buffer, size_t buffer_size,
              struct allocator const *alloc,
              ssize_t (*preadlinkat) (int, char const *, char *, size_t),
              char stack_buf[STACK_BUF_SIZE])
{
  if (! alloc)
    alloc = &stdlib_allocator;

  if (!buffer)
    {
      buffer = stack_buf;
      buffer_size = STACK_BUF_SIZE;
    }

  char *buf = buffer;
  idx_t buf_size_max = MIN (IDX_MAX, MIN (SSIZE_MAX, SIZE_MAX));
  idx_t buf_size = MIN (buffer_size, buf_size_max);

  while (buf)
    {
      /* Attempt to read the link into the current buffer.  */
      idx_t link_length = preadlinkat (fd, filename, buf, buf_size);
      if (link_length < 0)
        {
          if (buf != buffer)
            {
              int readlinkat_errno = errno;
              alloc->free (buf);
              errno = readlinkat_errno;
            }
          return NULL;
        }

      idx_t link_size = link_length;

      if (link_size < buf_size)
        {
          buf[link_size++] = '\0';

          if (buf == stack_buf)
            {
              char *b = alloc->allocate (link_size);
              buf_size = link_size;
              if (! b)
                break;
              return memcpy (b, buf, link_size);
            }

          if (link_size < buf_size && buf != buffer && alloc->reallocate)
            {
              /* Shrink BUF before returning it.  */
              char *b = alloc->reallocate (buf, link_size);
              if (b)
                return b;
            }

          return buf;
        }

      if (buf != buffer)
        alloc->free (buf);

      if (buf_size_max / 2 <= buf_size)
        {
          errno = ENAMETOOLONG;
          return NULL;
        }

      buf_size = 2 * buf_size + 1;
      buf = alloc->allocate (buf_size);
    }

  if (alloc->die)
    alloc->die (buf_size);
  errno = ENOMEM;
  return NULL;
}


/* Assuming the current directory is FD, get the symbolic link value
   of FILENAME as a null-terminated string and put it into a buffer.
   If FD is AT_FDCWD, FILENAME is interpreted relative to the current
   working directory, as in openat.

   If the link is small enough to fit into BUFFER put it there.
   BUFFER's size is BUFFER_SIZE, and BUFFER can be null
   if BUFFER_SIZE is zero.

   If the link is not small, put it into a dynamically allocated
   buffer managed by ALLOC.  It is the caller's responsibility to free
   the returned value if it is nonnull and is not BUFFER.  A null
   ALLOC stands for the standard allocator.

   The PREADLINKAT function specifies how to read links.  It operates
   like POSIX readlinkat()
   <https://pubs.opengroup.org/onlinepubs/9699919799/functions/readlink.html>
   but can assume that its first argument is the same as FD.

   If successful, return the buffer address; otherwise return NULL and
   set errno.  */

char *
careadlinkat (int fd, char const *filename,
              char *buffer, size_t buffer_size,
              struct allocator const *alloc,
              ssize_t (*preadlinkat) (int, char const *, char *, size_t))
{
  /* Allocate the initial buffer on the stack.  This way, in the
     common case of a symlink of small size, we get away with a
     single small malloc instead of a big malloc followed by a
     shrinking realloc.  */
  #ifdef GCC_BOGUS_WRETURN_LOCAL_ADDR
   #warning "GCC might issue a bogus -Wreturn-local-addr warning here."
   #warning "See <https://gcc.gnu.org/PR93644>."
  #endif
  char stack_buf[STACK_BUF_SIZE];
  return readlink_stk (fd, filename, buffer, buffer_size, alloc,
                       preadlinkat, stack_buf);
}
