/* Test for NaN that does not need libm.
   Copyright (C) 2007-2023 Free Software Foundation, Inc.

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

/* This file uses HAVE_ISNANF_IN_LIBC.  */
#if !_GL_CONFIG_H_INCLUDED
 #error "Please include config.h first."
#endif

#if HAVE_ISNANF_IN_LIBC
/* Get declaration of isnan macro or (older) isnanf function.  */
# include <math.h>
# if (__GNUC__ >= 4) || (__clang_major__ >= 4)
   /* GCC >= 4.0 and clang provide a type-generic built-in for isnan.
      GCC >= 4.0 also provides __builtin_isnanf, but clang doesn't.  */
#  undef isnanf
#  define isnanf(x) __builtin_isnan ((float)(x))
# elif defined isnan
#  undef isnanf
#  define isnanf(x) isnan ((float)(x))
# else
   /* Get declaration of isnanf(), if not declared in <math.h>.  */
#  if defined __sgi
   /* We can't include <ieeefp.h>, because it conflicts with our definition of
      isnand.  Therefore declare isnanf separately.  */
extern int isnanf (float x);
#  endif
# endif
#else
/* Test whether X is a NaN.  */
# undef isnanf
# define isnanf rpl_isnanf
extern int isnanf (float x);
#endif
