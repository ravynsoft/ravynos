/* ialloc.h -- malloc with idx_t rather than size_t

   Copyright 2021-2023 Free Software Foundation, Inc.

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

#ifndef IALLOC_H_
#define IALLOC_H_

/* This file uses _GL_INLINE_HEADER_BEGIN, _GL_INLINE, _GL_ATTRIBUTE_COLD,
   _GL_ATTRIBUTE_MALLOC.  */
#if !_GL_CONFIG_H_INCLUDED
 #error "Please include config.h first."
#endif

#include "idx.h"

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#if defined __CHERI_PURE_CAPABILITY__
# include <cheri.h>
#endif

_GL_INLINE_HEADER_BEGIN
#ifndef IALLOC_INLINE
# define IALLOC_INLINE _GL_INLINE
#endif

#ifdef __cplusplus
extern "C" {
#endif

IALLOC_INLINE void * _GL_ATTRIBUTE_COLD
_gl_alloc_nomem (void)
{
  errno = ENOMEM;
  return NULL;
}

/* imalloc (size) is like malloc (size).
   It returns a non-NULL pointer to size bytes of memory.
   Upon failure, it returns NULL with errno set.  */
IALLOC_INLINE
_GL_ATTRIBUTE_MALLOC /*_GL_ATTRIBUTE_DEALLOC_FREE*/
void *
imalloc (idx_t s)
{
  return s <= SIZE_MAX ? malloc (s) : _gl_alloc_nomem ();
}

/* irealloc (ptr, size) is like realloc (ptr, size).
   It returns a non-NULL pointer to size bytes of memory.
   Upon failure, it returns NULL with errno set.  */
IALLOC_INLINE
/*_GL_ATTRIBUTE_DEALLOC_FREE*/
void *
irealloc (void *p, idx_t s)
{
  if (s <= SIZE_MAX)
    {
      /* Work around GNU realloc glitch by treating a zero size as if it
         were 1, so that returning NULL is equivalent to failing.  */
      p = realloc (p, s | !s);
#if defined __CHERI_PURE_CAPABILITY__
      if (p != NULL)
        p = cheri_bounds_set (p, s);
#endif
      return p;
    }
  else
    return _gl_alloc_nomem ();
}

/* icalloc (num, size) is like calloc (num, size).
   It returns a non-NULL pointer to num * size bytes of memory.
   Upon failure, it returns NULL with errno set.  */
IALLOC_INLINE
_GL_ATTRIBUTE_MALLOC /*_GL_ATTRIBUTE_DEALLOC_FREE*/
void *
icalloc (idx_t n, idx_t s)
{
  if (SIZE_MAX < n)
    {
      if (s != 0)
        return _gl_alloc_nomem ();
      n = 0;
    }
  if (SIZE_MAX < s)
    {
      if (n != 0)
        return _gl_alloc_nomem ();
      s = 0;
    }
  return calloc (n, s);
}

/* ireallocarray (ptr, num, size) is like reallocarray (ptr, num, size).
   It returns a non-NULL pointer to num * size bytes of memory.
   Upon failure, it returns NULL with errno set.  */
IALLOC_INLINE void *
ireallocarray (void *p, idx_t n, idx_t s)
{
  if (n <= SIZE_MAX && s <= SIZE_MAX)
    {
      /* Work around GNU reallocarray glitch by treating a zero size as if
         it were 1, so that returning NULL is equivalent to failing.  */
      size_t nx = n;
      size_t sx = s;
      if (n == 0 || s == 0)
        nx = sx = 1;
      p = reallocarray (p, nx, sx);
#if defined __CHERI_PURE_CAPABILITY__
      if (p != NULL && (n == 0 || s == 0))
        p = cheri_bounds_set (p, 0);
#endif
      return p;
    }
  else
    return _gl_alloc_nomem ();
}

#ifdef __cplusplus
}
#endif

_GL_INLINE_HEADER_END

#endif
