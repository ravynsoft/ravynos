/* Macros for signalling not-a-number.
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

#ifndef _SIGNED_SNAN_H
#define _SIGNED_SNAN_H

#include "signed-nan.h"
#include "snan.h"


#if HAVE_SNANF

/* Returns a signalling 'float' NaN with sign bit == 0 in memory.  */
_GL_UNUSED static memory_float
memory_positive_SNaNf ()
{
  return construct_memory_SNaNf (positive_NaNf ());
}

/* Returns a signalling 'float' NaN with sign bit == 1 in memory.  */
_GL_UNUSED static memory_float
memory_negative_SNaNf ()
{
  return construct_memory_SNaNf (negative_NaNf ());
}

/* Note: On 32-bit x86 processors, as well as on x86_64 processors with
   CC="gcc -mfpmath=387", the following functions may return a quiet NaN
   instead.  Use the functions with 'memory_' prefix if you need to avoid this.
   See <https://lists.gnu.org/archive/html/bug-gnulib/2023-10/msg00060.html>
   for details.  */

/* Returns a signalling 'float' NaN with sign bit == 0.  */
_GL_UNUSED static float
positive_SNaNf ()
{
  return memory_positive_SNaNf ().value;
}

/* Returns a signalling 'float' NaN with sign bit == 1.  */
_GL_UNUSED static float
negative_SNaNf ()
{
  return memory_negative_SNaNf ().value;
}

#endif


#if HAVE_SNAND

/* Returns a signalling 'double' NaN with sign bit == 0 in memory.  */
_GL_UNUSED static memory_double
memory_positive_SNaNd ()
{
  return construct_memory_SNaNd (positive_NaNd ());
}

/* Returns a signalling 'double' NaN with sign bit == 1 in memory.  */
_GL_UNUSED static memory_double
memory_negative_SNaNd ()
{
  return construct_memory_SNaNd (negative_NaNd ());
}

/* Note: On 32-bit x86 processors, as well as on x86_64 processors with
   CC="gcc -mfpmath=387", the following functions may return a quiet NaN
   instead.  Use the functions with 'memory_' prefix if you need to avoid this.
   See <https://lists.gnu.org/archive/html/bug-gnulib/2023-10/msg00060.html>
   for details.  */

/* Returns a signalling 'double' NaN with sign bit == 0.  */
_GL_UNUSED static double
positive_SNaNd ()
{
  return memory_positive_SNaNd ().value;
}

/* Returns a signalling 'double' NaN with sign bit == 1.  */
_GL_UNUSED static double
negative_SNaNd ()
{
  return memory_negative_SNaNd ().value;
}

#endif


#if HAVE_SNANL

/* Returns a signalling 'long double' NaN with sign bit == 0 in memory.  */
_GL_UNUSED static memory_long_double
memory_positive_SNaNl ()
{
  return construct_memory_SNaNl (positive_NaNl ());
}

/* Returns a signalling 'long double' NaN with sign bit == 1 in memory.  */
_GL_UNUSED static memory_long_double
memory_negative_SNaNl ()
{
  return construct_memory_SNaNl (negative_NaNl ());
}

/* Note: On 32-bit x86 processors, as well as on x86_64 processors with
   CC="gcc -mfpmath=387", if HAVE_SAME_LONG_DOUBLE_AS_DOUBLE is 1, the
   following functions may return a quiet NaN instead.  Use the functions
   with 'memory_' prefix if you need to avoid this.  See
   <https://lists.gnu.org/archive/html/bug-gnulib/2023-10/msg00060.html>
   for details.  */

/* Returns a signalling 'long double' NaN with sign bit == 0.  */
_GL_UNUSED static long double
positive_SNaNl ()
{
  return memory_positive_SNaNl ().value;
}

/* Returns a signalling 'long double' NaN with sign bit == 1.  */
_GL_UNUSED static long double
negative_SNaNl ()
{
  return memory_negative_SNaNl ().value;
}

#endif


#endif /* _SIGNED_SNAN_H */
