/* Test of isnanl() substitute.
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

/* Written by Bruno Haible <bruno@clisp.org>, 2007.  */

#include <float.h>
#include <limits.h>

#include "minus-zero.h"
#include "infinity.h"
#include "nan.h"
#include "snan.h"
#include "macros.h"

int
main ()
{
  /* Finite values.  */
  ASSERT (!isnanl (3.141L));
  ASSERT (!isnanl (3.141e30L));
  ASSERT (!isnanl (3.141e-30L));
  ASSERT (!isnanl (-2.718L));
  ASSERT (!isnanl (-2.718e30L));
  ASSERT (!isnanl (-2.718e-30L));
  ASSERT (!isnanl (0.0L));
  ASSERT (!isnanl (minus_zerol));
  /* Infinite values.  */
  ASSERT (!isnanl (Infinityl ()));
  ASSERT (!isnanl (- Infinityl ()));
  /* Quiet NaN.  */
  ASSERT (isnanl (NaNl ()));
#if HAVE_SNANL
  /* Signalling NaN.  */
  ASSERT (isnanl (SNaNl ()));
#endif

#if ((defined __ia64 && LDBL_MANT_DIG == 64) || (defined __x86_64__ || defined __amd64__) || (defined __i386 || defined __i386__ || defined _I386 || defined _M_IX86 || defined _X86_)) && !HAVE_SAME_LONG_DOUBLE_AS_DOUBLE
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
    ASSERT (isnanl (x.value));
  }
  {
    /* Signalling NaN.  */
    static memory_long_double x =
      { .word = LDBL80_WORDS (0xFFFF, 0x83333333, 0x00000000) };
    ASSERT (isnanl (x.value));
  }
  /* isnanl should return something for noncanonical values.  */
  { /* Pseudo-NaN.  */
    static memory_long_double x =
      { .word = LDBL80_WORDS (0xFFFF, 0x40000001, 0x00000000) };
    ASSERT (isnanl (x.value) || !isnanl (x.value));
  }
  { /* Pseudo-Infinity.  */
    static memory_long_double x =
      { .word = LDBL80_WORDS (0xFFFF, 0x00000000, 0x00000000) };
    ASSERT (isnanl (x.value) || !isnanl (x.value));
  }
  { /* Pseudo-Zero.  */
    static memory_long_double x =
      { .word = LDBL80_WORDS (0x4004, 0x00000000, 0x00000000) };
    ASSERT (isnanl (x.value) || !isnanl (x.value));
  }
  { /* Unnormalized number.  */
    static memory_long_double x =
      { .word = LDBL80_WORDS (0x4000, 0x63333333, 0x00000000) };
    ASSERT (isnanl (x.value) || !isnanl (x.value));
  }
  { /* Pseudo-Denormal.  */
    static memory_long_double x =
      { .word = LDBL80_WORDS (0x0000, 0x83333333, 0x00000000) };
    ASSERT (isnanl (x.value) || !isnanl (x.value));
  }
  #undef NWORDS
#endif

  return 0;
}
