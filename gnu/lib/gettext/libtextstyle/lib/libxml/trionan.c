/* NaNs and Infinity in floating-point numbers.
   Copyright (C) 2015-2023 Free Software Foundation, Inc.

   This file was written by Daiki Ueno <ueno@gnu.org>, 2015.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Replacement 'trionan.c', using Gnulib functions.  */

#include "config.h"
#include <float.h>
#include <math.h>

/* Copied from gnulib/tests/infinity.h.  */

/* Infinityd () returns a 'double' +Infinity.  */

/* The Microsoft MSVC 9 compiler chokes on the expression 1.0 / 0.0.  */
#if defined _MSC_VER
static double
Infinityd ()
{
  static double zero = 0.0;
  return 1.0 / zero;
}
#else
# define Infinityd() (1.0 / 0.0)
#endif

/* Copied from gnulib/tests/nan.h.  */

/* NaNd () returns a 'double' not-a-number.  */

/* The Compaq (ex-DEC) C 6.4 compiler and the Microsoft MSVC 9 compiler choke
   on the expression 0.0 / 0.0.  */
#if defined __DECC || defined _MSC_VER
static double
NaNd ()
{
  static double zero = 0.0;
  return zero / zero;
}
#else
# define NaNd() (0.0 / 0.0)
#endif

/* Copied from gnulib/tests/minus-zero.h.  */

/* minus_zerod represents the value -0.0.  */

/* HP cc on HP-UX 10.20 has a bug with the constant expression -0.0.
   ICC 10.0 has a bug when optimizing the expression -zero.
   The expression -DBL_MIN * DBL_MIN does not work when cross-compiling
   to PowerPC on Mac OS X 10.5.  */
#if defined __hpux || defined __sgi || defined __ICC
_GL_ATTRIBUTE_MAYBE_UNUSED
static double
compute_minus_zerod (void)
{
  return -DBL_MIN * DBL_MIN;
}
# define minus_zerod compute_minus_zerod ()
#else
_GL_ATTRIBUTE_MAYBE_UNUSED
static double minus_zerod = -0.0;
#endif

#undef INFINITY
#undef NAN

#define INFINITY Infinityd()
#define NAN NaNd()

#define trio_pinf() INFINITY
#define trio_ninf() -INFINITY
#define trio_nan() NAN
#define trio_nzero() minus_zerod

#define trio_isnan(x) isnan(x)
#define trio_isinf(x) isinf(x)
#define trio_signbit(x) signbit(x)
