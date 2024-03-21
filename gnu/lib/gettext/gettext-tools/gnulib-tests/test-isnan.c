/* Test of isnand() substitute.
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

/* Written by Ben Pfaff <blp@cs.stanford.edu>, from code by Bruno
   Haible <bruno@clisp.org>.  */

#include <config.h>

#include <math.h>

/* isnan must be a macro.  */
#ifndef isnan
# error missing declaration
#endif

#include <float.h>
#include <limits.h>

#include "minus-zero.h"
#include "infinity.h"
#include "nan.h"
#include "snan.h"
#include "macros.h"

static void
test_float (void)
{
  /* Finite values.  */
  ASSERT (!isnan (3.141f));
  ASSERT (!isnan (3.141e30f));
  ASSERT (!isnan (3.141e-30f));
  ASSERT (!isnan (-2.718f));
  ASSERT (!isnan (-2.718e30f));
  ASSERT (!isnan (-2.718e-30f));
  ASSERT (!isnan (0.0f));
  ASSERT (!isnan (minus_zerof));
  /* Infinite values.  */
  ASSERT (!isnan (Infinityf ()));
  ASSERT (!isnan (- Infinityf ()));
  /* Quiet NaN.  */
  ASSERT (isnan (NaNf ()));
#if HAVE_SNANF
  /* Signalling NaN.  */
  ASSERT (isnan (SNaNf ()));
#endif
}

static void
test_double (void)
{
  /* Finite values.  */
  ASSERT (!isnan (3.141));
  ASSERT (!isnan (3.141e30));
  ASSERT (!isnan (3.141e-30));
  ASSERT (!isnan (-2.718));
  ASSERT (!isnan (-2.718e30));
  ASSERT (!isnan (-2.718e-30));
  ASSERT (!isnan (0.0));
  ASSERT (!isnan (minus_zerod));
  /* Infinite values.  */
  ASSERT (!isnan (Infinityd ()));
  ASSERT (!isnan (- Infinityd ()));
  /* Quiet NaN.  */
  ASSERT (isnan (NaNd ()));
#if HAVE_SNAND
  /* Signalling NaN.  */
  ASSERT (isnan (SNaNd ()));
#endif
}

static void
test_long_double (void)
{
  /* Finite values.  */
  ASSERT (!isnan (3.141L));
  ASSERT (!isnan (3.141e30L));
  ASSERT (!isnan (3.141e-30L));
  ASSERT (!isnan (-2.718L));
  ASSERT (!isnan (-2.718e30L));
  ASSERT (!isnan (-2.718e-30L));
  ASSERT (!isnan (0.0L));
  ASSERT (!isnan (minus_zerol));
  /* Infinite values.  */
  ASSERT (!isnan (Infinityl ()));
  ASSERT (!isnan (- Infinityl ()));
  /* Quiet NaN.  */
  ASSERT (isnan (NaNl ()));
#if HAVE_SNANL
  /* Signalling NaN.  */
  ASSERT (isnan (SNaNl ()));
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
    ASSERT (isnan (x.value));
  }
  {
    /* Signalling NaN.  */
    static memory_long_double x =
      { .word = LDBL80_WORDS (0xFFFF, 0x83333333, 0x00000000) };
    ASSERT (isnan (x.value));
  }
  /* isnan should return something for noncanonical values.  */
  { /* Pseudo-NaN.  */
    static memory_long_double x =
      { .word = LDBL80_WORDS (0xFFFF, 0x40000001, 0x00000000) };
    ASSERT (isnan (x.value) || !isnan (x.value));
  }
  { /* Pseudo-Infinity.  */
    static memory_long_double x =
      { .word = LDBL80_WORDS (0xFFFF, 0x00000000, 0x00000000) };
    ASSERT (isnan (x.value) || !isnan (x.value));
  }
  { /* Pseudo-Zero.  */
    static memory_long_double x =
      { .word = LDBL80_WORDS (0x4004, 0x00000000, 0x00000000) };
    ASSERT (isnan (x.value) || !isnan (x.value));
  }
  { /* Unnormalized number.  */
    static memory_long_double x =
      { .word = LDBL80_WORDS (0x4000, 0x63333333, 0x00000000) };
    ASSERT (isnan (x.value) || !isnan (x.value));
  }
  { /* Pseudo-Denormal.  */
    static memory_long_double x =
      { .word = LDBL80_WORDS (0x0000, 0x83333333, 0x00000000) };
    ASSERT (isnan (x.value) || !isnan (x.value));
  }
  #undef NWORDS
#endif
}

int
main ()
{
  test_float ();
  test_double ();
  test_long_double ();
  return 0;
}
