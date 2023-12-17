/* xalloc.h -- malloc with out-of-memory checking

   Copyright (C) 1990-2000, 2003-2004, 2006-2023 Free Software Foundation, Inc.

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

#ifndef XALLOC_H_
#define XALLOC_H_

/* This file uses _GL_INLINE_HEADER_BEGIN, _GL_INLINE, _Noreturn,
   _GL_ATTRIBUTE_ALLOC_SIZE, _GL_ATTRIBUTE_MALLOC,
   _GL_ATTRIBUTE_RETURNS_NONNULL.  */
#if !_GL_CONFIG_H_INCLUDED
 #error "Please include config.h first."
#endif

#include <stddef.h>
#include <stdlib.h>

#if GNULIB_XALLOC
# include "idx.h"
#endif

_GL_INLINE_HEADER_BEGIN
#ifndef XALLOC_INLINE
# define XALLOC_INLINE _GL_INLINE
#endif


#ifdef __cplusplus
extern "C" {
#endif


#if GNULIB_XALLOC_DIE

/* This function is always triggered when memory is exhausted.
   It must be defined by the application, either explicitly
   or by using gnulib's xalloc-die module.  This is the
   function to call when one wants the program to die because of a
   memory allocation failure.  */
_Noreturn void xalloc_die (void);

#endif /* GNULIB_XALLOC_DIE */

#if GNULIB_XALLOC

void *xmalloc (size_t s)
  _GL_ATTRIBUTE_MALLOC _GL_ATTRIBUTE_DEALLOC_FREE
  _GL_ATTRIBUTE_ALLOC_SIZE ((1)) _GL_ATTRIBUTE_RETURNS_NONNULL;
void *ximalloc (idx_t s)
  _GL_ATTRIBUTE_MALLOC _GL_ATTRIBUTE_DEALLOC_FREE
  _GL_ATTRIBUTE_ALLOC_SIZE ((1)) _GL_ATTRIBUTE_RETURNS_NONNULL;
void *xinmalloc (idx_t n, idx_t s)
  _GL_ATTRIBUTE_MALLOC _GL_ATTRIBUTE_DEALLOC_FREE
  _GL_ATTRIBUTE_ALLOC_SIZE ((1, 2)) _GL_ATTRIBUTE_RETURNS_NONNULL;
void *xzalloc (size_t s)
  _GL_ATTRIBUTE_MALLOC _GL_ATTRIBUTE_DEALLOC_FREE
  _GL_ATTRIBUTE_ALLOC_SIZE ((1)) _GL_ATTRIBUTE_RETURNS_NONNULL;
void *xizalloc (idx_t s)
  _GL_ATTRIBUTE_MALLOC _GL_ATTRIBUTE_DEALLOC_FREE
  _GL_ATTRIBUTE_ALLOC_SIZE ((1)) _GL_ATTRIBUTE_RETURNS_NONNULL;
void *xcalloc (size_t n, size_t s)
  _GL_ATTRIBUTE_MALLOC _GL_ATTRIBUTE_DEALLOC_FREE
  _GL_ATTRIBUTE_ALLOC_SIZE ((1, 2)) _GL_ATTRIBUTE_RETURNS_NONNULL;
void *xicalloc (idx_t n, idx_t s)
  _GL_ATTRIBUTE_MALLOC _GL_ATTRIBUTE_DEALLOC_FREE
  _GL_ATTRIBUTE_ALLOC_SIZE ((1, 2)) _GL_ATTRIBUTE_RETURNS_NONNULL;
void *xrealloc (void *p, size_t s)
  _GL_ATTRIBUTE_ALLOC_SIZE ((2));
void *xirealloc (void *p, idx_t s)
  _GL_ATTRIBUTE_ALLOC_SIZE ((2)) _GL_ATTRIBUTE_RETURNS_NONNULL;
void *xreallocarray (void *p, size_t n, size_t s)
  _GL_ATTRIBUTE_ALLOC_SIZE ((2, 3));
void *xireallocarray (void *p, idx_t n, idx_t s)
  _GL_ATTRIBUTE_ALLOC_SIZE ((2, 3)) _GL_ATTRIBUTE_RETURNS_NONNULL;
void *x2realloc (void *p, size_t *ps) /* superseded by xpalloc */
  _GL_ATTRIBUTE_RETURNS_NONNULL;
void *x2nrealloc (void *p, size_t *pn, size_t s) /* superseded by xpalloc */
  _GL_ATTRIBUTE_RETURNS_NONNULL;
void *xpalloc (void *pa, idx_t *pn, idx_t n_incr_min, ptrdiff_t n_max, idx_t s)
  _GL_ATTRIBUTE_RETURNS_NONNULL;
void *xmemdup (void const *p, size_t s)
  _GL_ATTRIBUTE_MALLOC _GL_ATTRIBUTE_DEALLOC_FREE
  _GL_ATTRIBUTE_ALLOC_SIZE ((2)) _GL_ATTRIBUTE_RETURNS_NONNULL;
void *ximemdup (void const *p, idx_t s)
  _GL_ATTRIBUTE_MALLOC _GL_ATTRIBUTE_DEALLOC_FREE
  _GL_ATTRIBUTE_ALLOC_SIZE ((2)) _GL_ATTRIBUTE_RETURNS_NONNULL;
char *ximemdup0 (void const *p, idx_t s)
  _GL_ATTRIBUTE_MALLOC _GL_ATTRIBUTE_DEALLOC_FREE
  _GL_ATTRIBUTE_RETURNS_NONNULL;
char *xstrdup (char const *str)
  _GL_ATTRIBUTE_MALLOC _GL_ATTRIBUTE_DEALLOC_FREE
  _GL_ATTRIBUTE_RETURNS_NONNULL;

/* In the following macros, T must be an elementary or structure/union or
   typedef'ed type, or a pointer to such a type.  To apply one of the
   following macros to a function pointer or array type, you need to typedef
   it first and use the typedef name.  */

/* Allocate an object of type T dynamically, with error checking.  */
/* extern t *XMALLOC (typename t); */
# define XMALLOC(t) ((t *) xmalloc (sizeof (t)))

/* Allocate memory for N elements of type T, with error checking.  */
/* extern t *XNMALLOC (size_t n, typename t); */
# define XNMALLOC(n, t) \
    ((t *) (sizeof (t) == 1 ? xmalloc (n) : xnmalloc (n, sizeof (t))))

/* Allocate an object of type T dynamically, with error checking,
   and zero it.  */
/* extern t *XZALLOC (typename t); */
# define XZALLOC(t) ((t *) xzalloc (sizeof (t)))

/* Allocate memory for N elements of type T, with error checking,
   and zero it.  */
/* extern t *XCALLOC (size_t n, typename t); */
# define XCALLOC(n, t) \
    ((t *) (sizeof (t) == 1 ? xzalloc (n) : xcalloc (n, sizeof (t))))


/* Allocate an array of N objects, each with S bytes of memory,
   dynamically, with error checking.  S must be nonzero.  */

void *xnmalloc (size_t n, size_t s)
  _GL_ATTRIBUTE_MALLOC _GL_ATTRIBUTE_DEALLOC_FREE
  _GL_ATTRIBUTE_ALLOC_SIZE ((1, 2)) _GL_ATTRIBUTE_RETURNS_NONNULL;

/* FIXME: Deprecate this in favor of xreallocarray?  */
/* Change the size of an allocated block of memory P to an array of N
   objects each of S bytes, with error checking.  S must be nonzero.  */

XALLOC_INLINE void *xnrealloc (void *p, size_t n, size_t s)
  _GL_ATTRIBUTE_ALLOC_SIZE ((2, 3));
XALLOC_INLINE void *
xnrealloc (void *p, size_t n, size_t s)
{
  return xreallocarray (p, n, s);
}

/* Return a pointer to a new buffer of N bytes.  This is like xmalloc,
   except it returns char *.  */

char *xcharalloc (size_t n)
  _GL_ATTRIBUTE_MALLOC _GL_ATTRIBUTE_DEALLOC_FREE
  _GL_ATTRIBUTE_ALLOC_SIZE ((1)) _GL_ATTRIBUTE_RETURNS_NONNULL;

#endif /* GNULIB_XALLOC */


#ifdef __cplusplus
}
#endif


#if GNULIB_XALLOC && defined __cplusplus

/* C++ does not allow conversions from void * to other pointer types
   without a cast.  Use templates to work around the problem when
   possible.  */

template <typename T> inline T *
xrealloc (T *p, size_t s)
{
  return (T *) xrealloc ((void *) p, s);
}

template <typename T> inline T *
xreallocarray (T *p, size_t n, size_t s)
{
  return (T *) xreallocarray ((void *) p, n, s);
}

/* FIXME: Deprecate this in favor of xreallocarray?  */
template <typename T> inline T *
xnrealloc (T *p, size_t n, size_t s)
{
  return xreallocarray (p, n, s);
}

template <typename T> inline T *
x2realloc (T *p, size_t *pn)
{
  return (T *) x2realloc ((void *) p, pn);
}

template <typename T> inline T *
x2nrealloc (T *p, size_t *pn, size_t s)
{
  return (T *) x2nrealloc ((void *) p, pn, s);
}

template <typename T> inline T *
xmemdup (T const *p, size_t s)
{
  return (T *) xmemdup ((void const *) p, s);
}

#endif /* GNULIB_XALLOC && C++ */


_GL_INLINE_HEADER_END

#endif /* !XALLOC_H_ */
