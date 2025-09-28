/* Macros for quiet not-a-number.
   Copyright (C) 2023 Free Software Foundation, Inc.

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

#ifndef _SIGNED_NAN_H
#define _SIGNED_NAN_H

#include <math.h>

#include "nan.h"


/* Returns a quiet 'float' NaN with sign bit == 0.  */
_GL_UNUSED static float
positive_NaNf ()
{
  /* 'volatile' works around a GCC bug:
     <https://gcc.gnu.org/bugzilla/show_bug.cgi?id=111655>  */
  float volatile nan = NaNf ();
  return (signbit (nan) ? - nan : nan);
}

/* Returns a quiet 'float' NaN with sign bit == 1.  */
_GL_UNUSED static float
negative_NaNf ()
{
  /* 'volatile' works around a GCC bug:
     <https://gcc.gnu.org/bugzilla/show_bug.cgi?id=111655>  */
  float volatile nan = NaNf ();
  return (signbit (nan) ? nan : - nan);
}


/* Returns a quiet 'double' NaN with sign bit == 0.  */
_GL_UNUSED static double
positive_NaNd ()
{
  /* 'volatile' works around a GCC bug:
     <https://gcc.gnu.org/bugzilla/show_bug.cgi?id=111655>  */
  double volatile nan = NaNd ();
  return (signbit (nan) ? - nan : nan);
}

/* Returns a quiet 'double' NaN with sign bit == 1.  */
_GL_UNUSED static double
negative_NaNd ()
{
  /* 'volatile' works around a GCC bug:
     <https://gcc.gnu.org/bugzilla/show_bug.cgi?id=111655>  */
  double volatile nan = NaNd ();
  return (signbit (nan) ? nan : - nan);
}


/* Returns a quiet 'long double' NaN with sign bit == 0.  */
_GL_UNUSED static long double
positive_NaNl ()
{
  /* 'volatile' works around a GCC bug:
     <https://gcc.gnu.org/bugzilla/show_bug.cgi?id=111655>  */
  long double volatile nan = NaNl ();
  return (signbit (nan) ? - nan : nan);
}

/* Returns a quiet 'long double' NaN with sign bit == 1.  */
_GL_UNUSED static long double
negative_NaNl ()
{
  /* 'volatile' works around a GCC bug:
     <https://gcc.gnu.org/bugzilla/show_bug.cgi?id=111655>  */
  long double volatile nan = NaNl ();
  return (signbit (nan) ? nan : - nan);
}


#endif /* _SIGNED_NAN_H */
