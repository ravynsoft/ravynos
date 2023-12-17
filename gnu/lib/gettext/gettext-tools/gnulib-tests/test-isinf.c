/* Test of isinf() substitute.
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

/* Written by Ben Pfaff, 2008, using Bruno Haible's code as a
   template. */

#include <config.h>

#include <math.h>

/* isinf must be a macro.  */
#ifndef isinf
# error missing declaration
#endif

#include <float.h>
#include <limits.h>

#include "infinity.h"
#include "snan.h"
#include "macros.h"

float zerof = 0.0f;
double zerod = 0.0;
long double zerol = 0.0L;

static void
test_isinff ()
{
  /* Zero. */
  ASSERT (!isinf (0.0f));
  /* Subnormal values. */
  ASSERT (!isinf (FLT_MIN / 2));
  ASSERT (!isinf (-FLT_MIN / 2));
  /* Finite values.  */
  ASSERT (!isinf (3.141f));
  ASSERT (!isinf (3.141e30f));
  ASSERT (!isinf (3.141e-30f));
  ASSERT (!isinf (-2.718f));
  ASSERT (!isinf (-2.718e30f));
  ASSERT (!isinf (-2.718e-30f));
  ASSERT (!isinf (FLT_MAX));
  ASSERT (!isinf (-FLT_MAX));
  /* Infinite values.  */
  ASSERT (isinf (Infinityf ()));
  ASSERT (isinf (- Infinityf ()));
  /* Quiet NaN.  */
  ASSERT (!isinf (zerof / zerof));
#if HAVE_SNANF
  /* Signalling NaN.  */
  ASSERT (!isinf (SNaNf ()));
#endif
}

static void
test_isinfd ()
{
  /* Zero. */
  ASSERT (!isinf (0.0));
  /* Subnormal values. */
  ASSERT (!isinf (DBL_MIN / 2));
  ASSERT (!isinf (-DBL_MIN / 2));
  /* Finite values. */
  ASSERT (!isinf (3.141));
  ASSERT (!isinf (3.141e30));
  ASSERT (!isinf (3.141e-30));
  ASSERT (!isinf (-2.718));
  ASSERT (!isinf (-2.718e30));
  ASSERT (!isinf (-2.718e-30));
  ASSERT (!isinf (DBL_MAX));
  ASSERT (!isinf (-DBL_MAX));
  /* Infinite values.  */
  ASSERT (isinf (Infinityd ()));
  ASSERT (isinf (- Infinityd ()));
  /* Quiet NaN.  */
  ASSERT (!isinf (zerod / zerod));
#if HAVE_SNAND
  /* Signalling NaN.  */
  ASSERT (!isinf (SNaNd ()));
#endif
}

static void
test_isinfl ()
{
  /* Zero. */
  ASSERT (!isinf (0.0L));
  /* Subnormal values. */
  ASSERT (!isinf (LDBL_MIN / 2));
  ASSERT (!isinf (-LDBL_MIN / 2));
  /* Finite values. */
  ASSERT (!isinf (3.141L));
  ASSERT (!isinf (3.141e30L));
  ASSERT (!isinf (3.141e-30L));
  ASSERT (!isinf (-2.718L));
  ASSERT (!isinf (-2.718e30L));
  ASSERT (!isinf (-2.718e-30L));
  ASSERT (!isinf (LDBL_MAX));
  ASSERT (!isinf (-LDBL_MAX));
  /* Infinite values.  */
  ASSERT (isinf (Infinityl ()));
  ASSERT (isinf (- Infinityl ()));
  /* Quiet NaN.  */
  ASSERT (!isinf (zerol / zerol));
#if HAVE_SNANL
  /* Signalling NaN.  */
  ASSERT (!isinf (SNaNl ()));
#endif

#if ((defined __ia64 && LDBL_MANT_DIG == 64) || (defined __x86_64__ || defined __amd64__) || (defined __i386 || defined __i386__ || defined _I386 || defined _M_IX86 || defined _X86_)) && !HAVE_SAME_LONG_DOUBLE_AS_DOUBLE
  #define NWORDS \
    ((sizeof (long double) + sizeof (unsigned int) - 1) / sizeof (unsigned int))
  typedef union { unsigned int word[NWORDS]; long double value; }
          memory_long_double;
/* Representation of an 80-bit 'long double' as an initializer for a sequence
   of 'unsigned int' words.  */
# ifdef WORDS_BIGENDIAN
#  define LDBL80_WORDS(exponent,manthi,mantlo) \
     { ((unsigned int) (exponent) << 16) | ((unsigned int) (manthi) >> 16), \
       ((unsigned int) (manthi) << 16) | ((unsigned int) (mantlo) >> 16),   \
       (unsigned int) (mantlo) << 16                                        \
     }
# else
#  define LDBL80_WORDS(exponent,manthi,mantlo) \
     { mantlo, manthi, exponent }
# endif
  { /* Quiet NaN.  */
    static memory_long_double x =
      { .word = LDBL80_WORDS (0xFFFF, 0xC3333333, 0x00000000) };
    ASSERT (!isinf (x.value));
  }
  {
    /* Signalling NaN.  */
    static memory_long_double x =
      { .word = LDBL80_WORDS (0xFFFF, 0x83333333, 0x00000000) };
    ASSERT (!isinf (x.value));
  }
  /* isinf should return something for noncanonical values.  */
  { /* Pseudo-NaN.  */
    static memory_long_double x =
      { .word = LDBL80_WORDS (0xFFFF, 0x40000001, 0x00000000) };
    ASSERT (isinf (x.value) || !isinf (x.value));
  }
  { /* Pseudo-Infinity.  */
    static memory_long_double x =
      { .word = LDBL80_WORDS (0xFFFF, 0x00000000, 0x00000000) };
    ASSERT (isinf (x.value) || !isinf (x.value));
  }
  { /* Pseudo-Zero.  */
    static memory_long_double x =
      { .word = LDBL80_WORDS (0x4004, 0x00000000, 0x00000000) };
    ASSERT (isinf (x.value) || !isinf (x.value));
  }
  { /* Unnormalized number.  */
    static memory_long_double x =
      { .word = LDBL80_WORDS (0x4000, 0x63333333, 0x00000000) };
    ASSERT (isinf (x.value) || !isinf (x.value));
  }
  { /* Pseudo-Denormal.  */
    static memory_long_double x =
      { .word = LDBL80_WORDS (0x0000, 0x83333333, 0x00000000) };
    ASSERT (isinf (x.value) || !isinf (x.value));
  }
  #undef NWORDS
#endif
}

int
main ()
{
  test_isinff ();
  test_isinfd ();
  test_isinfl ();
  return 0;
}
