/* xalloc-oversized.h -- memory allocation size checking

   Copyright (C) 1990-2000, 2003-2004, 2006-2026 Free Software Foundation, Inc.

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

#ifndef XALLOC_OVERSIZED_H_
#define XALLOC_OVERSIZED_H_

#include <stddef.h>
#include <stdint.h>

/* True if N * S does not fit into both ptrdiff_t and size_t.
   N and S should be nonnegative and free of side effects.
   This expands to a constant expression if N and S are both constants.
   By gnulib convention, SIZE_MAX represents overflow in size_t
   calculations, so the conservative size_t-based dividend to use here
   is SIZE_MAX - 1.  */
#define __xalloc_oversized(n, s) \
  ((s) != 0 \
   && (PTRDIFF_MAX < SIZE_MAX ? PTRDIFF_MAX : SIZE_MAX - 1) / (s) < (n))

/* Return 1 if and only if an array of N objects, each of size S,
   cannot exist reliably because its total size in bytes would exceed
   MIN (PTRDIFF_MAX, SIZE_MAX - 1).

   N and S should be nonnegative and free of side effects.

   Warning: (xalloc_oversized (N, S) ? NULL : malloc (N * S)) can
   misbehave if N and S are both narrower than ptrdiff_t and size_t,
   and can be rewritten as (xalloc_oversized (N, S) ?  NULL
   : malloc (N * (size_t) S)).

   This is a macro, not a function, so that it works even if an
   argument exceeds MAX (PTRDIFF_MAX, SIZE_MAX).  */
#if 7 <= __GNUC__ && !defined __clang__ && PTRDIFF_MAX < SIZE_MAX
# define xalloc_oversized(n, s) \
   __builtin_mul_overflow_p (n, s, (ptrdiff_t) 1)
#elif 5 <= __GNUC__ && !defined __clang__ && !defined __ICC \
      && PTRDIFF_MAX < SIZE_MAX
# define xalloc_oversized(n, s) \
   (__builtin_constant_p (n) && __builtin_constant_p (s) \
    ? __xalloc_oversized (n, s) \
    : __extension__ \
        ({ ptrdiff_t __xalloc_count; \
           __builtin_mul_overflow (n, s, &__xalloc_count); }))

/* Other compilers use integer division; this may be slower but is
   more portable.  */
#else
# define xalloc_oversized(n, s) __xalloc_oversized (n, s)
#endif

#endif /* !XALLOC_OVERSIZED_H_ */
