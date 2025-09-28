/* Macros for signalling not-a-number.
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

#ifndef _SNAN_H
#define _SNAN_H

#include <float.h>
#include <limits.h>
#include <math.h>

#include "nan.h"


/* The bit that distinguishes a quiet NaN from a signalling NaN is, according to
   <https://en.wikipedia.org/wiki/NaN#Encoding>, the most significant bit of the
   mantissa field.
   According to <https://en.wikipedia.org/wiki/IEEE_754#Formats>, this is the
   next bit, right below the bit 0 of the exponent.
   This bit is
     *  == 0 to indicate a quiet NaN or Infinity,
        == 1 to indicate a signalling NaN,
        on these CPUs: hppa, mips (*), sh4.
     *  == 1 to indicate a quiet NaN,
        == 0 to indicate a signalling NaN or Infinity,
        on all other CPUs.
        On these platforms, additionally a signalling NaN must have some other
        mantissa bit == 1, because when all exponent bits are == 1 and all
        mantissa bits are == 0, the number denotes ±Infinity.
        This NaN encoding is specified by IEEE 754-2008 § 6.2.1.

   (*) On mips CPUs, it depends on the CPU model.  The classical behaviour is
   as indicated above.  On some newer models, it's like on the other CPUs.
   On some (but not all!) models this meta-info can be determined from two
   special CPU registers: If the "Floating Point Implementation Register" (fir)
   bit 23, also known as Has2008 bit, is set, the "Floating Point Control and
   Status Register" (fcsr) bit 18, also known as the NAN2008 bit, has the value
     - 0 for the classical behaviour,
     - 1 for like on the other CPUs.
   Both of these bits are read-only.
   This module has determined the behaviour at configure time and defines the
   C macros MIPS_NAN2008_FLOAT, MIPS_NAN2008_DOUBLE, MIPS_NAN2008_LONG_DOUBLE
   accordingly.  */


/* 'float' = IEEE 754 single-precision
   <https://en.wikipedia.org/wiki/Single-precision_floating-point_format>  */

#define NWORDS \
  ((sizeof (float) + sizeof (unsigned int) - 1) / sizeof (unsigned int))
typedef union { float value; unsigned int word[NWORDS]; } memory_float;

#if defined FLT_EXPBIT0_WORD && defined FLT_EXPBIT0_BIT

# define HAVE_SNANF 1

_GL_UNUSED static memory_float
construct_memory_SNaNf (float quiet_value)
{
  memory_float m;
  m.value = quiet_value;
  /* Turn the quiet NaN into a signalling NaN.  */
  #if FLT_EXPBIT0_BIT > 0
    m.word[FLT_EXPBIT0_WORD] ^= (unsigned int) 1 << (FLT_EXPBIT0_BIT - 1);
  #else
    m.word[FLT_EXPBIT0_WORD + (FLT_EXPBIT0_WORD < NWORDS / 2 ? 1 : - 1)]
      ^= (unsigned int) 1 << (sizeof (unsigned int) * CHAR_BIT - 1);
  #endif
  /* Set some arbitrary mantissa bit.  */
  if (FLT_EXPBIT0_WORD < NWORDS / 2) /* NWORDS > 1 and big endian */
    m.word[FLT_EXPBIT0_WORD + 1] |= (unsigned int) 1 << FLT_EXPBIT0_BIT;
  else /* NWORDS == 1 or little endian */
    m.word[0] |= (unsigned int) 1;
  return m;
}

/* Returns a signalling 'float' NaN in memory.  */
_GL_UNUSED static memory_float
memory_SNaNf ()
{
  return construct_memory_SNaNf (NaNf ());
}

_GL_UNUSED static float
construct_SNaNf (float quiet_value)
{
  return construct_memory_SNaNf (quiet_value).value;
}

/* Returns a signalling 'float' NaN.
   Note: On 32-bit x86 processors, as well as on x86_64 processors with
   CC="gcc -mfpmath=387", this function may return a quiet NaN instead.
   Use memory_SNaNf() if you need to avoid this.  See
   <https://lists.gnu.org/archive/html/bug-gnulib/2023-10/msg00060.html>
   for details.  */
_GL_UNUSED static float
SNaNf ()
{
  return memory_SNaNf ().value;
}

#endif

#undef NWORDS


/* 'double' = IEEE 754 double-precision
   <https://en.wikipedia.org/wiki/Double-precision_floating-point_format>  */

#define NWORDS \
  ((sizeof (double) + sizeof (unsigned int) - 1) / sizeof (unsigned int))
typedef union { double value; unsigned int word[NWORDS]; } memory_double;

#if defined DBL_EXPBIT0_WORD && defined DBL_EXPBIT0_BIT

# define HAVE_SNAND 1

_GL_UNUSED static memory_double
construct_memory_SNaNd (double quiet_value)
{
  memory_double m;
  m.value = quiet_value;
  /* Turn the quiet NaN into a signalling NaN.  */
  #if DBL_EXPBIT0_BIT > 0
    m.word[DBL_EXPBIT0_WORD] ^= (unsigned int) 1 << (DBL_EXPBIT0_BIT - 1);
  #else
    m.word[DBL_EXPBIT0_WORD + (DBL_EXPBIT0_WORD < NWORDS / 2 ? 1 : - 1)]
      ^= (unsigned int) 1 << (sizeof (unsigned int) * CHAR_BIT - 1);
  #endif
  /* Set some arbitrary mantissa bit.  */
  m.word[DBL_EXPBIT0_WORD + (DBL_EXPBIT0_WORD < NWORDS / 2 ? 1 : - 1)]
    |= (unsigned int) 1 << DBL_EXPBIT0_BIT;
  return m;
}

/* Returns a signalling 'double' NaN in memory.  */
_GL_UNUSED static memory_double
memory_SNaNd ()
{
  return construct_memory_SNaNd (NaNd ());
}

_GL_UNUSED static double
construct_SNaNd (double quiet_value)
{
  return construct_memory_SNaNd (quiet_value).value;
}

/* Returns a signalling 'double' NaN.
   Note: On 32-bit x86 processors, as well as on x86_64 processors with
   CC="gcc -mfpmath=387", this function may return a quiet NaN instead.
   Use memory_SNaNf() if you need to avoid this.  See
   <https://lists.gnu.org/archive/html/bug-gnulib/2023-10/msg00060.html>
   for details.  */
_GL_UNUSED static double
SNaNd ()
{
  return memory_SNaNd ().value;
}

#endif

#undef NWORDS


/* 'long double' =
   * if HAVE_SAME_LONG_DOUBLE_AS_DOUBLE:
     IEEE 754 double-precision
     <https://en.wikipedia.org/wiki/Double-precision_floating-point_format>
   * Otherwise:
     - On i386, x86_64, ia64:
       80-bits extended-precision
       <https://en.wikipedia.org/wiki/Extended_precision#x86_extended_precision_format>
     - On alpha, arm64, loongarch64, mips64, riscv64, s390x, sparc64:
       IEEE 754 quadruple-precision
       <https://en.wikipedia.org/wiki/Quadruple-precision_floating-point_format#IEEE_754_quadruple-precision_binary_floating-point_format:_binary128>
     - On powerpc, powerpc64, powerpc64le:
       2x64-bits double-double
       <https://en.wikipedia.org/wiki/Quadruple-precision_floating-point_format#Double-double_arithmetic>
     - On m68k:
       80-bits extended-precision, padded to 96 bits, with non-IEEE exponent
 */

#define NWORDS \
  ((sizeof (long double) + sizeof (unsigned int) - 1) / sizeof (unsigned int))
typedef union { long double value; unsigned int word[NWORDS]; }
        memory_long_double;

#if defined LDBL_EXPBIT0_WORD && defined LDBL_EXPBIT0_BIT

# define HAVE_SNANL 1

_GL_UNUSED static memory_long_double
construct_memory_SNaNl (long double quiet_value)
{
  memory_long_double m;
  m.value = quiet_value;
  #if defined __powerpc__ && LDBL_MANT_DIG == 106
    /* This is PowerPC "double double", a pair of two doubles.  Inf and NaN are
       represented as the corresponding 64-bit IEEE values in the first double;
       the second is ignored.  Manipulate only the first double.  */
    #define HNWORDS \
      ((sizeof (double) + sizeof (unsigned int) - 1) / sizeof (unsigned int))
  #else
    #define HNWORDS NWORDS
  #endif
  /* Turn the quiet NaN into a signalling NaN.  */
  #if ((defined __ia64 && LDBL_MANT_DIG == 64) || (defined __x86_64__ || defined __amd64__) || (defined __i386 || defined __i386__ || defined _I386 || defined _M_IX86 || defined _X86_)) && !HAVE_SAME_LONG_DOUBLE_AS_DOUBLE
  /* In this representation, the leading 1 of the mantissa is explicitly
     stored.  */
   #if LDBL_EXPBIT0_BIT > 1
    m.word[LDBL_EXPBIT0_WORD] ^= (unsigned int) 1 << (LDBL_EXPBIT0_BIT - 2);
   #else
    m.word[LDBL_EXPBIT0_WORD + (LDBL_EXPBIT0_WORD < HNWORDS / 2 ? 1 : - 1)]
      ^= (unsigned int) 1 << (sizeof (unsigned int) * CHAR_BIT - 2);
   #endif
  #else
  /* In this representation, the leading 1 of the mantissa is implicit.  */
   #if LDBL_EXPBIT0_BIT > 0
    m.word[LDBL_EXPBIT0_WORD] ^= (unsigned int) 1 << (LDBL_EXPBIT0_BIT - 1);
   #else
    m.word[LDBL_EXPBIT0_WORD + (LDBL_EXPBIT0_WORD < HNWORDS / 2 ? 1 : - 1)]
      ^= (unsigned int) 1 << (sizeof (unsigned int) * CHAR_BIT - 1);
   #endif
  #endif
  /* Set some arbitrary mantissa bit.  */
  m.word[LDBL_EXPBIT0_WORD + (LDBL_EXPBIT0_WORD < HNWORDS / 2 ? 1 : - 1)]
    |= (unsigned int) 1 << LDBL_EXPBIT0_BIT;
  #undef HNWORDS
  return m;
}

/* Returns a signalling 'long double' NaN in memory.  */
_GL_UNUSED static memory_long_double
memory_SNaNl ()
{
  return construct_memory_SNaNl (NaNl ());
}

_GL_UNUSED static long double
construct_SNaNl (long double quiet_value)
{
  return construct_memory_SNaNl (quiet_value).value;
}

/* Returns a signalling 'long double' NaN.
   Note: On 32-bit x86 processors, as well as on x86_64 processors with
   CC="gcc -mfpmath=387", if HAVE_SAME_LONG_DOUBLE_AS_DOUBLE is 1, this
   function may return a quiet NaN instead.  Use memory_SNaNf() if you
   need to avoid this.  See
   <https://lists.gnu.org/archive/html/bug-gnulib/2023-10/msg00060.html>
   for details.  */
_GL_UNUSED static long double
SNaNl ()
{
  return memory_SNaNl ().value;
}

#endif

#undef NWORDS


#endif /* _SNAN_H */
