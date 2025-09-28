/* xmalloc.c -- malloc with out of memory checking

   Copyright (C) 1990-2000, 2002-2006, 2008-2023 Free Software Foundation, Inc.

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

#include <config.h>

#define XALLOC_INLINE _GL_EXTERN_INLINE

#include "xalloc.h"

#include "ialloc.h"
#include "minmax.h"

#include <stdckdint.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

static void * _GL_ATTRIBUTE_PURE
nonnull (void *p)
{
  if (!p)
    xalloc_die ();
  return p;
}

/* Allocate S bytes of memory dynamically, with error checking.  */

void *
xmalloc (size_t s)
{
  return nonnull (malloc (s));
}

void *
ximalloc (idx_t s)
{
  return nonnull (imalloc (s));
}

char *
xcharalloc (size_t n)
{
  return XNMALLOC (n, char);
}

/* Change the size of an allocated block of memory P to S bytes,
   with error checking.  */

void *
xrealloc (void *p, size_t s)
{
  void *r = realloc (p, s);
  if (!r && (!p || s))
    xalloc_die ();
  return r;
}

void *
xirealloc (void *p, idx_t s)
{
  return nonnull (irealloc (p, s));
}

/* Change the size of an allocated block of memory P to an array of N
   objects each of S bytes, with error checking.  */

void *
xreallocarray (void *p, size_t n, size_t s)
{
  void *r = reallocarray (p, n, s);
  if (!r && (!p || (n && s)))
    xalloc_die ();
  return r;
}

void *
xireallocarray (void *p, idx_t n, idx_t s)
{
  return nonnull (ireallocarray (p, n, s));
}

/* Allocate an array of N objects, each with S bytes of memory,
   dynamically, with error checking.  S must be nonzero.  */

void *
xnmalloc (size_t n, size_t s)
{
  return xreallocarray (NULL, n, s);
}

void *
xinmalloc (idx_t n, idx_t s)
{
  return xireallocarray (NULL, n, s);
}

/* If P is null, allocate a block of at least *PS bytes; otherwise,
   reallocate P so that it contains more than *PS bytes.  *PS must be
   nonzero unless P is null.  Set *PS to the new block's size, and
   return the pointer to the new block.  *PS is never set to zero, and
   the returned pointer is never null.  */

void *
x2realloc (void *p, size_t *ps)
{
  return x2nrealloc (p, ps, 1);
}

/* If P is null, allocate a block of at least *PN such objects;
   otherwise, reallocate P so that it contains more than *PN objects
   each of S bytes.  S must be nonzero.  Set *PN to the new number of
   objects, and return the pointer to the new block.  *PN is never set
   to zero, and the returned pointer is never null.

   Repeated reallocations are guaranteed to make progress, either by
   allocating an initial block with a nonzero size, or by allocating a
   larger block.

   In the following implementation, nonzero sizes are increased by a
   factor of approximately 1.5 so that repeated reallocations have
   O(N) overall cost rather than O(N**2) cost, but the
   specification for this function does not guarantee that rate.

   Here is an example of use:

     int *p = NULL;
     size_t used = 0;
     size_t allocated = 0;

     void
     append_int (int value)
       {
         if (used == allocated)
           p = x2nrealloc (p, &allocated, sizeof *p);
         p[used++] = value;
       }

   This causes x2nrealloc to allocate a block of some nonzero size the
   first time it is called.

   To have finer-grained control over the initial size, set *PN to a
   nonzero value before calling this function with P == NULL.  For
   example:

     int *p = NULL;
     size_t used = 0;
     size_t allocated = 0;
     size_t allocated1 = 1000;

     void
     append_int (int value)
       {
         if (used == allocated)
           {
             p = x2nrealloc (p, &allocated1, sizeof *p);
             allocated = allocated1;
           }
         p[used++] = value;
       }

   */

void *
x2nrealloc (void *p, size_t *pn, size_t s)
{
  size_t n = *pn;

  if (! p)
    {
      if (! n)
        {
          /* The approximate size to use for initial small allocation
             requests, when the invoking code specifies an old size of
             zero.  This is the largest "small" request for the GNU C
             library malloc.  */
          enum { DEFAULT_MXFAST = 64 * sizeof (size_t) / 4 };

          n = DEFAULT_MXFAST / s;
          n += !n;
        }
    }
  else
    {
      /* Set N = floor (1.5 * N) + 1 to make progress even if N == 0.  */
      if (ckd_add (&n, n, (n >> 1) + 1))
        xalloc_die ();
    }

  p = xreallocarray (p, n, s);
  *pn = n;
  return p;
}

/* Grow PA, which points to an array of *PN items, and return the
   location of the reallocated array, updating *PN to reflect its
   new size.  The new array will contain at least N_INCR_MIN more
   items, but will not contain more than N_MAX items total.
   S is the size of each item, in bytes.

   S and N_INCR_MIN must be positive.  *PN must be
   nonnegative.  If N_MAX is -1, it is treated as if it were
   infinity.

   If PA is null, then allocate a new array instead of reallocating
   the old one.

   Thus, to grow an array A without saving its old contents, do
   { free (A); A = xpalloc (NULL, &AITEMS, ...); }.  */

void *
xpalloc (void *pa, idx_t *pn, idx_t n_incr_min, ptrdiff_t n_max, idx_t s)
{
  idx_t n0 = *pn;

  /* The approximate size to use for initial small allocation
     requests.  This is the largest "small" request for the GNU C
     library malloc.  */
  enum { DEFAULT_MXFAST = 64 * sizeof (size_t) / 4 };

  /* If the array is tiny, grow it to about (but no greater than)
     DEFAULT_MXFAST bytes.  Otherwise, grow it by about 50%.
     Adjust the growth according to three constraints: N_INCR_MIN,
     N_MAX, and what the C language can represent safely.  */

  idx_t n;
  if (ckd_add (&n, n0, n0 >> 1))
    n = IDX_MAX;
  if (0 <= n_max && n_max < n)
    n = n_max;

  /* NBYTES is of a type suitable for holding the count of bytes in an object.
     This is typically idx_t, but it should be size_t on (theoretical?)
     platforms where SIZE_MAX < IDX_MAX so xpalloc does not pass
     values greater than SIZE_MAX to xrealloc.  */
#if IDX_MAX <= SIZE_MAX
  idx_t nbytes;
#else
  size_t nbytes;
#endif
  idx_t adjusted_nbytes
    = (ckd_mul (&nbytes, n, s)
       ? MIN (IDX_MAX, SIZE_MAX)
       : nbytes < DEFAULT_MXFAST ? DEFAULT_MXFAST : 0);
  if (adjusted_nbytes)
    {
      n = adjusted_nbytes / s;
      nbytes = adjusted_nbytes - adjusted_nbytes % s;
    }

  if (! pa)
    *pn = 0;
  if (n - n0 < n_incr_min
      && (ckd_add (&n, n0, n_incr_min)
          || (0 <= n_max && n_max < n)
          || ckd_mul (&nbytes, n, s)))
    xalloc_die ();
  pa = xrealloc (pa, nbytes);
  *pn = n;
  return pa;
}

/* Allocate S bytes of zeroed memory dynamically, with error checking.
   There's no need for xnzalloc (N, S), since it would be equivalent
   to xcalloc (N, S).  */

void *
xzalloc (size_t s)
{
  return xcalloc (s, 1);
}

void *
xizalloc (idx_t s)
{
  return xicalloc (s, 1);
}

/* Allocate zeroed memory for N elements of S bytes, with error
   checking.  S must be nonzero.  */

void *
xcalloc (size_t n, size_t s)
{
  return nonnull (calloc (n, s));
}

void *
xicalloc (idx_t n, idx_t s)
{
  return nonnull (icalloc (n, s));
}

/* Clone an object P of size S, with error checking.  There's no need
   for xnmemdup (P, N, S), since xmemdup (P, N * S) works without any
   need for an arithmetic overflow check.  */

void *
xmemdup (void const *p, size_t s)
{
  return memcpy (xmalloc (s), p, s);
}

void *
ximemdup (void const *p, idx_t s)
{
  return memcpy (ximalloc (s), p, s);
}

/* Clone an object P of size S, with error checking.  Append
   a terminating NUL byte.  */

char *
ximemdup0 (void const *p, idx_t s)
{
  char *result = ximalloc (s + 1);
  result[s] = 0;
  return memcpy (result, p, s);
}

/* Clone STRING.  */

char *
xstrdup (char const *string)
{
  return xmemdup (string, strlen (string) + 1);
}
