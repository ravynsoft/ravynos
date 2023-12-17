/* Safe automatic memory allocation with out of memory checking.
   Copyright (C) 2003, 2005, 2007, 2009-2023 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2003.

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

#ifndef _XMALLOCA_H
#define _XMALLOCA_H

/* This file uses _GL_ATTRIBUTE_ALLOC_SIZE, _GL_ATTRIBUTE_DEALLOC,
   _GL_ATTRIBUTE_MALLOC, _GL_ATTRIBUTE_RETURNS_NONNULL, HAVE_ALLOCA.  */
#if !_GL_CONFIG_H_INCLUDED
 #error "Please include config.h first."
#endif

#include "malloca.h"
#include "xalloc.h"
#include "xalloc-oversized.h"


#ifdef __cplusplus
extern "C" {
#endif


/* xmalloca(N) is a checking safe variant of alloca(N).  It allocates N bytes
   of memory allocated on the stack, that must be freed using freea() before
   the function returns.  N should not have side effects.
   Upon failure, it exits with an error message.  */
#if HAVE_ALLOCA
# define xmalloca(N) \
  ((N) < 4032 - (2 * sa_alignment_max - 1)                                   \
   ? (void *) (((uintptr_t) (char *) alloca ((N) + 2 * sa_alignment_max - 1) \
                + (2 * sa_alignment_max - 1))                                \
               & ~(uintptr_t)(2 * sa_alignment_max - 1))                     \
   : xmmalloca (N))
extern void * xmmalloca (size_t n)
  _GL_ATTRIBUTE_MALLOC _GL_ATTRIBUTE_DEALLOC (freea, 1)
  _GL_ATTRIBUTE_ALLOC_SIZE ((1)) _GL_ATTRIBUTE_RETURNS_NONNULL;
#else
# define xmalloca(N) \
  xmalloc (N)
#endif

/* xnmalloca(N,S) is an overflow-safe variant of xmalloca (N * S).
   It allocates an array of N objects, each with S bytes of memory,
   on the stack.  S must be positive and N must be nonnegative,
   and S and N should not have side effects.
   The array must be freed using freea() before the function returns.
   Upon failure, it exits with an error message.  */
#if HAVE_ALLOCA
/* Rely on xmalloca (SIZE_MAX) calling xalloc_die ().  */
# define xnmalloca(n, s) \
    xmalloca (xalloc_oversized (n, s) ? (size_t) (-1) : (n) * (size_t) (s))
#else
# define xnmalloca(n, s) \
    xnmalloc (n, s)
#endif


#ifdef __cplusplus
}
#endif


#endif /* _XMALLOCA_H */
