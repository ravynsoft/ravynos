/* Safe automatic memory allocation.
   Copyright (C) 2003, 2006-2007, 2009-2023 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2003, 2018.

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

#define _GL_USE_STDLIB_ALLOC 1
#include <config.h>

/* Specification.  */
#include "malloca.h"

#include <stdckdint.h>
#if defined __CHERI_PURE_CAPABILITY__
# include <cheri.h>
#endif

#include "idx.h"

/* The speed critical point in this file is freea() applied to an alloca()
   result: it must be fast, to match the speed of alloca().  The speed of
   mmalloca() and freea() in the other case are not critical, because they
   are only invoked for big memory sizes.
   Here we use a bit in the address as an indicator, an idea by Ondřej Bílka.
   malloca() can return three types of pointers:
     - Pointers ≡ 0 mod 2*sa_alignment_max come from stack allocation.
     - Pointers ≡ sa_alignment_max mod 2*sa_alignment_max come from heap
       allocation.
     - NULL comes from a failed heap allocation.  */

#if defined __CHERI_PURE_CAPABILITY__
/* Type for holding the original malloc() result.  */
typedef uintptr_t small_t;
#else
/* Type for holding very small pointer differences.  */
typedef unsigned char small_t;
/* Verify that it is wide enough.  */
static_assert (2 * sa_alignment_max - 1 <= (small_t) -1);
#endif

void *
mmalloca (size_t n)
{
#if HAVE_ALLOCA
  /* Allocate one more word, used to determine the address to pass to freea(),
     and room for the alignment ≡ sa_alignment_max mod 2*sa_alignment_max.  */
  uintptr_t alignment2_mask = 2 * sa_alignment_max - 1;
  int plus = sizeof (small_t) + alignment2_mask;
  idx_t nplus;
  if (!ckd_add (&nplus, n, plus) && !xalloc_oversized (nplus, 1))
    {
      char *mem = (char *) malloc (nplus);

      if (mem != NULL)
        {
          uintptr_t umem = (uintptr_t) mem;
          /* The ckd_add avoids signed integer overflow on
             theoretical platforms where UINTPTR_MAX <= INT_MAX.  */
          uintptr_t umemplus;
          ckd_add (&umemplus, umem, sizeof (small_t) + sa_alignment_max - 1);
          idx_t offset = (umemplus - umemplus % (2 * sa_alignment_max)
                          + sa_alignment_max - umem);
          void *p = mem + offset;
          /* Here p >= mem + sizeof (small_t),
             and p <= mem + sizeof (small_t) + 2 * sa_alignment_max - 1
             hence p + n <= mem + nplus.
             So, the memory range [p, p+n) lies in the allocated memory range
             [mem, mem + nplus).  */
          small_t *sp = p;
# if defined __CHERI_PURE_CAPABILITY__
          sp[-1] = umem;
          p = (char *) cheri_bounds_set ((char *) p - sizeof (small_t),
                                         sizeof (small_t) + n)
              + sizeof (small_t);
# else
          sp[-1] = offset;
# endif
          /* p ≡ sa_alignment_max mod 2*sa_alignment_max.  */
          return p;
        }
    }
  /* Out of memory.  */
  return NULL;
#else
# if !MALLOC_0_IS_NONNULL
  if (n == 0)
    n = 1;
# endif
  return malloc (n);
#endif
}

#if HAVE_ALLOCA
void
freea (void *p)
{
  /* Check argument.  */
  uintptr_t u = (uintptr_t) p;
  if (u & (sa_alignment_max - 1))
    {
      /* p was not the result of a malloca() call.  Invalid argument.  */
      abort ();
    }
  /* Determine whether p was a non-NULL pointer returned by mmalloca().  */
  if (u & sa_alignment_max)
    {
      char *cp = p;
      small_t *sp = p;
# if defined __CHERI_PURE_CAPABILITY__
      void *mem = sp[-1];
# else
      void *mem = cp - sp[-1];
# endif
      free (mem);
    }
}
#endif

/*
 * Hey Emacs!
 * Local Variables:
 * coding: utf-8
 * End:
 */
