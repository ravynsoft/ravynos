/* Macros for floating-point negative zero.
   Copyright (C) 2010-2023 Free Software Foundation, Inc.

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

/* Keep in sync with m4/minus-zero.m4!  */

#include <float.h>


/* minus_zerof represents the value -0.0f.  */

/* HP cc on HP-UX 10.20 has a bug with the constant expression -0.0f.
   ICC 10.0 has a bug when optimizing the expression -zero.
   The expression -FLT_MIN * FLT_MIN does not work when cross-compiling
   to PowerPC on Mac OS X 10.5.  */
#if defined __hpux || defined __sgi || defined __ICC
static float
compute_minus_zerof (void)
{
  return -FLT_MIN * FLT_MIN;
}
# define minus_zerof compute_minus_zerof ()
#else
float minus_zerof = -0.0f;
#endif


/* minus_zerod represents the value -0.0.  */

/* HP cc on HP-UX 10.20 has a bug with the constant expression -0.0.
   ICC 10.0 has a bug when optimizing the expression -zero.
   The expression -DBL_MIN * DBL_MIN does not work when cross-compiling
   to PowerPC on Mac OS X 10.5.  */
#if defined __hpux || defined __sgi || defined __ICC
static double
compute_minus_zerod (void)
{
  return -DBL_MIN * DBL_MIN;
}
# define minus_zerod compute_minus_zerod ()
#else
double minus_zerod = -0.0;
#endif


/* minus_zerol represents the value -0.0L.  */

/* HP cc on HP-UX 10.20 has a bug with the constant expression -0.0L.
   IRIX cc can't put -0.0L into .data, but can compute at runtime.
   ICC 10.0 has a bug when optimizing the expression -zero.
   The expression -LDBL_MIN * LDBL_MIN does not work when cross-compiling
   to PowerPC on Mac OS X 10.5.  */
#if defined __hpux || defined __sgi || defined __ICC
static long double
compute_minus_zerol (void)
{
  return -LDBL_MIN * LDBL_MIN;
}
# define minus_zerol compute_minus_zerol ()
#else
long double minus_zerol = -0.0L;
#endif
