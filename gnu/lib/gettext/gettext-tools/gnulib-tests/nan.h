/* Macros for quiet not-a-number.
   Copyright (C) 2007-2023 Free Software Foundation, Inc.

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

#ifndef _GL_NAN_H
#define _GL_NAN_H


/* IBM z/OS supports both hexadecimal and IEEE floating-point formats. The
   former does not support NaN and its isnan() implementation returns zero
   for all values.  */
#if defined __MVS__ && defined __IBMC__ && !defined __BFP__
# error "NaN is not supported with IBM's hexadecimal floating-point format; please re-compile with -qfloat=ieee"
#endif

/* NaNf () returns a 'float' not-a-number.  */

/* The Compaq (ex-DEC) C 6.4 compiler and the Microsoft MSVC 9 compiler choke
   on the expression 0.0 / 0.0.  The IBM XL C compiler on z/OS complains.
   PGI 16.10 complains.  clang 13 on mips64 does incorrect constant-folding.  */
#if (defined __DECC || defined _MSC_VER \
     || (defined __MVS__ && defined __IBMC__) \
     || defined __PGI \
     || defined __mips__)
static float
NaNf ()
{
  static float volatile zero = 0.0f;
  return zero / zero;
}
#else
# define NaNf() (0.0f / 0.0f)
#endif


/* NaNd () returns a 'double' not-a-number.  */

/* The Compaq (ex-DEC) C 6.4 compiler and the Microsoft MSVC 9 compiler choke
   on the expression 0.0 / 0.0.  The IBM XL C compiler on z/OS complains.
   PGI 16.10 complains.  clang 13 on mips64 does incorrect constant-folding.  */
#if (defined __DECC || defined _MSC_VER \
     || (defined __MVS__ && defined __IBMC__) \
     || defined __PGI \
     || defined __mips__)
static double
NaNd ()
{
  static double volatile zero = 0.0;
  return zero / zero;
}
#else
# define NaNd() (0.0 / 0.0)
#endif


/* NaNl () returns a 'long double' not-a-number.  */

/* On Irix 6.5, gcc 3.4.3 can't compute compile-time NaN, and needs the
   runtime type conversion.
   The Microsoft MSVC 9 compiler chokes on the expression 0.0L / 0.0L.
   The IBM XL C compiler on z/OS complains.
   PGI 16.10 complains.
   Avoid possible incorrect constant-folding on mips.  */
#ifdef __sgi
static long double NaNl ()
{
  double zero = 0.0;
  return zero / zero;
}
#elif (defined _MSC_VER \
       || (defined __MVS__ && defined __IBMC__) \
       || defined __PGI \
       || defined __mips__)
static long double
NaNl ()
{
  static long double volatile zero = 0.0L;
  return zero / zero;
}
#else
# define NaNl() (0.0L / 0.0L)
#endif


#endif /* _GL_NAN_H */
