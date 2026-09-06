/* Searching in a string.
   Copyright (C) 2008-2026 Free Software Foundation, Inc.

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

#include <config.h>

/* Specification.  */
#include <string.h>

/* A function definition is needed only if HAVE_RAWMEMCHR is not defined.  */
#if !HAVE_RAWMEMCHR

# include <limits.h>
# include <stdint.h>


/* Find the first occurrence of C in S.  */
void *
rawmemchr (const void *s, int c_in)
{
# ifdef __CHERI_PURE_CAPABILITY__
  /* Most architectures let you read an aligned word,
     even if the unsigned char array at S ends in the middle of the word.
     However CHERI does not, so call memchr
     with the underlying object's remaining length.
     This cannot return NULL if S points to a C_IN-terminated array.
     Use builtins rather than including <cheri.h> which is less stable.  */
  return memchr (s, c_in, (__builtin_cheri_length_get (s)
                           - __builtin_cheri_offset_get (s)));
# else

  /* You can change this typedef to experiment with performance.  */
  typedef uintptr_t longword;
  /* Verify that the longword type lacks padding bits.  */
  static_assert (UINTPTR_WIDTH == UCHAR_WIDTH * sizeof (uintptr_t));

  unsigned char c = c_in;

  {
    const unsigned char *char_ptr;

    /* Handle the first few bytes by reading one byte at a time.
       Do this until CHAR_PTR is aligned on a natural longword boundary,
       as using alignof (longword) might be slower.  */
    for (char_ptr = (const unsigned char *) s;
         (uintptr_t) char_ptr % sizeof (longword) != 0;
         ++char_ptr)
      if (*char_ptr == c)
        return (void *) char_ptr;

    s = char_ptr;
  }

  {
    longword const *longword_ptr = s;

    /* Compute auxiliary longword values:
       repeated_one is a value which has a 1 in every byte.
       repeated_c has c in every byte.  */
    longword repeated_one = (longword) -1 / UCHAR_MAX;
    longword repeated_c = repeated_one * c;
    longword repeated_hibit = repeated_one * (UCHAR_MAX / 2 + 1);

    /* Instead of the traditional loop which tests each byte, we will
       test a longword at a time.  The tricky part is testing if any of
       the bytes in the longword in question are equal to
       c.  We first use an xor with repeated_c.  This reduces the task
       to testing whether any of the bytes in longword1 is zero.

       (The following comments assume 8-bit bytes, as POSIX requires;
       the code's use of UCHAR_MAX should work even if bytes have more
       than 8 bits.)

       We compute tmp =
         ((longword1 - repeated_one) & ~longword1) & (repeated_one * 0x80).
       That is, we perform the following operations:
         1. Subtract repeated_one.
         2. & ~longword1.
         3. & a mask consisting of 0x80 in every byte.
       Consider what happens in each byte:
         - If a byte of longword1 is zero, step 1 and 2 transform it into 0xff,
           and step 3 transforms it into 0x80.  A carry can also be propagated
           to more significant bytes.
         - If a byte of longword1 is nonzero, let its lowest 1 bit be at
           position k (0 <= k <= 7); so the lowest k bits are 0.  After step 1,
           the byte ends in a single bit of value 0 and k bits of value 1.
           After step 2, the result is just k bits of value 1: 2^k - 1.  After
           step 3, the result is 0.  And no carry is produced.
       So, if longword1 has only non-zero bytes, tmp is zero.
       Whereas if longword1 has a zero byte, call j the position of the least
       significant zero byte.  Then the result has a zero at positions 0, ...,
       j-1 and a 0x80 at position j.  We cannot predict the result at the more
       significant bytes (positions j+1..3), but it does not matter since we
       already have a non-zero bit at position 8*j+7.

       The test whether any byte in longword1 is zero is equivalent
       to testing whether tmp is nonzero.

       This test can read beyond the end of a string, depending on where
       C_IN is encountered.  However, this is considered safe since the
       initialization phase ensured that the read will be aligned,
       therefore, the read will not cross page boundaries and will not
       cause a fault.  */

    while (1)
      {
        longword longword1 = *longword_ptr ^ repeated_c;

        if ((((longword1 - repeated_one) & ~longword1) & repeated_hibit) != 0)
          break;
        longword_ptr++;
      }

    s = longword_ptr;
  }

  {
    const unsigned char *char_ptr = s;

    /* At this point, we know that one of the sizeof (longword) bytes
       starting at char_ptr is == c.  If we knew endianness, we
       could determine the first such byte without any further memory
       accesses, just by looking at the tmp result from the last loop
       iteration.  However, the following simple and portable code does
       not attempt this potential optimization.  */

    while (*char_ptr != c)
      char_ptr++;
    return (void *) char_ptr;
  }
# endif
}

#endif
