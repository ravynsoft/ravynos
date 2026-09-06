/* realloc() function that is glibc compatible.

   Copyright (C) 1997, 2003-2004, 2006-2007, 2009-2026 Free Software
   Foundation, Inc.

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

/* written by Jim Meyering and Bruno Haible */

/* Ensure that we call the system's realloc() below.  */
#define _GL_USE_STDLIB_ALLOC 1

#define _GL_REALLOC_INLINE _GL_EXTERN_INLINE
#include <config.h>
#include <stdlib.h>

#include <errno.h>
#include <stdckdint.h>

#ifdef __CHERI_PURE_CAPABILITY__
# include <cheri.h>
#endif

#ifndef _GL_INLINE_RPL_REALLOC

/* Change the size of an allocated block of memory P to N bytes,
   with error checking.  If P is NULL, use malloc.  Otherwise if N is zero,
   free P and return NULL.  */

void *
rpl_realloc (void *p, size_t n)
{
  size_t n1 = n;

  if (n == 0)
    {
# if NEED_SANITIZED_REALLOC
      /* When P is non-null, ISO C23 §7.24.3.7.(3) says realloc (P, 0) has
         undefined behavior even though C17 and earlier partially defined
         the behavior.  Let the programmer know.
         When the undefined-behaviour sanitizers report this case, i.e. when
         <https://gcc.gnu.org/PR117233> and
         <https://github.com/llvm/llvm-project/issues/113065>
         have been closed and new releases of GCC and clang have been made,
         we can revisit this code.  */
      if (p != NULL)
        abort ();
# endif

      /* realloc (NULL, 0) acts like glibc malloc (0), i.e., like malloc (1)
         except the caller cannot dereference any non-null return.

         realloc (P, 0) with non-null P is a messier situation.
         As mentioned above, C23 says behavior is undefined.
         POSIX.1-2024 extends C17 to say realloc (P, 0)
         either fails by setting errno and returning a null pointer,
         or succeeds by freeing P and then either:
           (a) setting errno=EINVAL and returning a null pointer; or
           (b) acting like a successful malloc (0).
         glibc 1 through 2.1 realloc acted like (b),
         which conforms to C17, to C23 and to POSIX.1-2024.
         glibc 2.1.1+ realloc acts like (a) except it does not set errno;
         this conforms to C17 and to C23 but not to POSIX.1-2024.
         Quite possibly future versions of POSIX will change,
         due either to C23 or to (a)'s semantics being messy.
         Act like (b), as that's easy, matches GNU, BSD and V7 malloc,
         matches BSD and V7 realloc, and requires no extra code at
         caller sites.  */

# if !HAVE_REALLOC_0_NONNULL
      n1 = 1;
# endif
    }

# if !HAVE_MALLOC_PTRDIFF
  ptrdiff_t signed_n;
  if (ckd_add (&signed_n, n, 0))
    {
      errno = ENOMEM;
      return NULL;
    }
# endif

  void *result = realloc (p, n1);

# if !HAVE_REALLOC_POSIX
  if (result == NULL)
    errno = ENOMEM;
# endif

# ifdef __CHERI_PURE_CAPABILITY__
  if (result != NULL)
    result = cheri_bounds_set (result, n);
# endif

  return result;
}

#endif
