/* Recode Serbian text from Cyrillic to Latin script.
   Copyright (C) 2006-2007, 2009 Free Software Foundation, Inc.
   Written by Danilo Šegan <danilo@gnome.org>, 2006,
   and Bruno Haible <bruno@clisp.org>, 2006.

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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/* Specification.  */
#include "filters.h"

#include <stdlib.h>

#include "xalloc.h"


/* Table for Serbian Cyrillic to Latin transcription.
   The table is indexed by the Unicode code point, in the range 0x0400..0x04ef.
   The longest table entry is three bytes long.  */
static const char table[240][3 + 1] =
{
  /* U+0400 */ "\xC3\x88", /* "È" */
  /* U+0401 */ "",
  /* U+0402 */ "\xC4\x90", /* "Đ" */
  /* U+0403 */ "",
  /* U+0404 */ "",
  /* U+0405 */ "",
  /* U+0406 */ "",
  /* U+0407 */ "",
  /* U+0408 */ "J",
  /* U+0409 */ "Lj",
  /* U+040A */ "Nj",
  /* U+040B */ "\xC4\x86", /* "Ć" */
  /* U+040C */ "",
  /* U+040D */ "\xC3\x8C", /* "Ì" */
  /* U+040E */ "",
  /* U+040F */ "D\xC5\xBE", /* "Dž" */
  /* U+0410 */ "A",
  /* U+0411 */ "B",
  /* U+0412 */ "V",
  /* U+0413 */ "G",
  /* U+0414 */ "D",
  /* U+0415 */ "E",
  /* U+0416 */ "\xC5\xBD", /* "Ž" */
  /* U+0417 */ "Z",
  /* U+0418 */ "I",
  /* U+0419 */ "",
  /* U+041A */ "K",
  /* U+041B */ "L",
  /* U+041C */ "M",
  /* U+041D */ "N",
  /* U+041E */ "O",
  /* U+041F */ "P",
  /* U+0420 */ "R",
  /* U+0421 */ "S",
  /* U+0422 */ "T",
  /* U+0423 */ "U",
  /* U+0424 */ "F",
  /* U+0425 */ "H",
  /* U+0426 */ "C",
  /* U+0427 */ "\xC4\x8C", /* "Č" */
  /* U+0428 */ "\xC5\xA0", /* "Š" */
  /* U+0429 */ "",
  /* U+042A */ "",
  /* U+042B */ "",
  /* U+042C */ "",
  /* U+042D */ "",
  /* U+042E */ "",
  /* U+042F */ "",
  /* U+0430 */ "a",
  /* U+0431 */ "b",
  /* U+0432 */ "v",
  /* U+0433 */ "g",
  /* U+0434 */ "d",
  /* U+0435 */ "e",
  /* U+0436 */ "\xC5\xBE", /* "ž" */
  /* U+0437 */ "z",
  /* U+0438 */ "i",
  /* U+0439 */ "",
  /* U+043A */ "k",
  /* U+043B */ "l",
  /* U+043C */ "m",
  /* U+043D */ "n",
  /* U+043E */ "o",
  /* U+043F */ "p",
  /* U+0440 */ "r",
  /* U+0441 */ "s",
  /* U+0442 */ "t",
  /* U+0443 */ "u",
  /* U+0444 */ "f",
  /* U+0445 */ "h",
  /* U+0446 */ "c",
  /* U+0447 */ "\xC4\x8D", /* "č" */
  /* U+0448 */ "\xC5\xA1", /* "š" */
  /* U+0449 */ "",
  /* U+044A */ "",
  /* U+044B */ "",
  /* U+044C */ "",
  /* U+044D */ "",
  /* U+044E */ "",
  /* U+044F */ "",
  /* U+0450 */ "\xC3\xA8", /* "è" */
  /* U+0451 */ "",
  /* U+0452 */ "\xC4\x91", /* "đ" */
  /* U+0453 */ "",
  /* U+0454 */ "",
  /* U+0455 */ "",
  /* U+0456 */ "",
  /* U+0457 */ "",
  /* U+0458 */ "j",
  /* U+0459 */ "lj",
  /* U+045A */ "nj",
  /* U+045B */ "\xC4\x87", /* "ć" */
  /* U+045C */ "",
  /* U+045D */ "\xC3\xAC", /* "ì" */
  /* U+045E */ "",
  /* U+045F */ "d\xC5\xBE", /* "dž" */
  /* U+0460 */ "",
  /* U+0461 */ "",
  /* U+0462 */ "",
  /* U+0463 */ "",
  /* U+0464 */ "",
  /* U+0465 */ "",
  /* U+0466 */ "",
  /* U+0467 */ "",
  /* U+0468 */ "",
  /* U+0469 */ "",
  /* U+046A */ "",
  /* U+046B */ "",
  /* U+046C */ "",
  /* U+046D */ "",
  /* U+046E */ "",
  /* U+046F */ "",
  /* U+0470 */ "",
  /* U+0471 */ "",
  /* U+0472 */ "",
  /* U+0473 */ "",
  /* U+0474 */ "",
  /* U+0475 */ "",
  /* U+0476 */ "",
  /* U+0477 */ "",
  /* U+0478 */ "",
  /* U+0479 */ "",
  /* U+047A */ "",
  /* U+047B */ "",
  /* U+047C */ "",
  /* U+047D */ "",
  /* U+047E */ "",
  /* U+047F */ "",
  /* U+0480 */ "",
  /* U+0481 */ "",
  /* U+0482 */ "",
  /* U+0483 */ "",
  /* U+0484 */ "",
  /* U+0485 */ "",
  /* U+0486 */ "",
  /* U+0487 */ "",
  /* U+0488 */ "",
  /* U+0489 */ "",
  /* U+048A */ "",
  /* U+048B */ "",
  /* U+048C */ "",
  /* U+048D */ "",
  /* U+048E */ "",
  /* U+048F */ "",
  /* U+0490 */ "",
  /* U+0491 */ "",
  /* U+0492 */ "",
  /* U+0493 */ "",
  /* U+0494 */ "",
  /* U+0495 */ "",
  /* U+0496 */ "",
  /* U+0497 */ "",
  /* U+0498 */ "",
  /* U+0499 */ "",
  /* U+049A */ "",
  /* U+049B */ "",
  /* U+049C */ "",
  /* U+049D */ "",
  /* U+049E */ "",
  /* U+049F */ "",
  /* U+04A0 */ "",
  /* U+04A1 */ "",
  /* U+04A2 */ "",
  /* U+04A3 */ "",
  /* U+04A4 */ "",
  /* U+04A5 */ "",
  /* U+04A6 */ "",
  /* U+04A7 */ "",
  /* U+04A8 */ "",
  /* U+04A9 */ "",
  /* U+04AA */ "",
  /* U+04AB */ "",
  /* U+04AC */ "",
  /* U+04AD */ "",
  /* U+04AE */ "",
  /* U+04AF */ "",
  /* U+04B0 */ "",
  /* U+04B1 */ "",
  /* U+04B2 */ "",
  /* U+04B3 */ "",
  /* U+04B4 */ "",
  /* U+04B5 */ "",
  /* U+04B6 */ "",
  /* U+04B7 */ "",
  /* U+04B8 */ "",
  /* U+04B9 */ "",
  /* U+04BA */ "",
  /* U+04BB */ "",
  /* U+04BC */ "",
  /* U+04BD */ "",
  /* U+04BE */ "",
  /* U+04BF */ "",
  /* U+04C0 */ "",
  /* U+04C1 */ "",
  /* U+04C2 */ "",
  /* U+04C3 */ "",
  /* U+04C4 */ "",
  /* U+04C5 */ "",
  /* U+04C6 */ "",
  /* U+04C7 */ "",
  /* U+04C8 */ "",
  /* U+04C9 */ "",
  /* U+04CA */ "",
  /* U+04CB */ "",
  /* U+04CC */ "",
  /* U+04CD */ "",
  /* U+04CE */ "",
  /* U+04CF */ "",
  /* U+04D0 */ "",
  /* U+04D1 */ "",
  /* U+04D2 */ "",
  /* U+04D3 */ "",
  /* U+04D4 */ "",
  /* U+04D5 */ "",
  /* U+04D6 */ "",
  /* U+04D7 */ "",
  /* U+04D8 */ "",
  /* U+04D9 */ "",
  /* U+04DA */ "",
  /* U+04DB */ "",
  /* U+04DC */ "",
  /* U+04DD */ "",
  /* U+04DE */ "",
  /* U+04DF */ "",
  /* U+04E0 */ "",
  /* U+04E1 */ "",
  /* U+04E2 */ "\xC4\xAA", /* "Ī" */
  /* U+04E3 */ "\xC4\xAB", /* "ī" */
  /* U+04E4 */ "",
  /* U+04E5 */ "",
  /* U+04E6 */ "",
  /* U+04E7 */ "",
  /* U+04E8 */ "",
  /* U+04E9 */ "",
  /* U+04EA */ "",
  /* U+04EB */ "",
  /* U+04EC */ "",
  /* U+04ED */ "",
  /* U+04EE */ "\xC5\xAA", /* "Ū" */
  /* U+04EF */ "\xC5\xAB" /* "ū" */
};

/* Quick test for an uppercase character in the range U+0041..U+005A.
   The argument must be a byte in the range 0..UCHAR_MAX.  */
#define IS_UPPERCASE_LATIN(byte) \
  ((unsigned char) ((byte) - 'A') <= 'Z' - 'A')

/* Quick test for an uppercase character in the range U+0400..U+042F,
   or exactly U+04E2 or U+04EE.
   The arguments must be bytes in the range 0..UCHAR_MAX.  */
#define IS_UPPERCASE_CYRILLIC(byte1,byte2) \
  (((byte1) == 0xd0 && (unsigned char) ((byte2) - 0x80) < 0x30) \
   || ((byte1) == 0xd3 && ((byte2) == 0xa2 || (byte2) == 0xae)))

void
serbian_to_latin (const char *input, size_t input_len,
                  char **output_p, size_t *output_len_p)
{
  /* Loop through the input string, producing a replacement for each character.
     Only characters in the range U+0400..U+04EF (\xD0\x80..\xD3\xAF) need to
     be handled, and more precisely only those for which a replacement exists
     in the table.  Other characters are copied without modification.
     The characters U+0409, U+040A, U+040F are transliterated to uppercase or
     mixed-case replacements ("LJ" / "Lj", "NJ" / "Nj", "DŽ" / "Dž"), depending
     on the case of the surrounding characters.
     Since we assume UTF-8 encoding, the bytes \xD0..\xD3 can only occur at the
     beginning of a character; the second and further bytes of a character are
     all in the range \x80..\xBF.  */

  /* Since sequences of 2 bytes are mapped to sequences of at most 3 bytes,
     the size of the output will be at most 1.5 * input_len.  */
  size_t allocated = input_len + (input_len >> 1);
  char *output = XNMALLOC (allocated, char);

  const char *input_end = input + input_len;
  const char *ip;
  char *op;

  for (ip = input, op = output; ip < input_end; )
    {
      unsigned char byte = (unsigned char) *ip;

      /* Test for the first byte of a Cyrillic character.  */
      if ((byte >= 0xd0 && byte <= 0xd3) && (ip + 1 < input_end))
        {
          unsigned char second_byte = (unsigned char) ip[1];

          /* Verify the second byte is valid.  */
          if (second_byte >= 0x80 && second_byte < 0xc0)
            {
              unsigned int uc = ((byte & 0x1f) << 6) | (second_byte & 0x3f);

              if (uc >= 0x0400 && uc <= 0x04ef)
                {
                  /* Look up replacement from the table.  */
                  const char *repl = table[uc - 0x0400];

                  if (repl[0] != '\0')
                    {
                      /* Found a replacement.
                         Now handle the special cases.  */
                      if (uc == 0x0409 || uc == 0x040a || uc == 0x040f)
                        if ((ip + 2 < input_end
                             && IS_UPPERCASE_LATIN ((unsigned char) ip[2]))
                            || (ip + 3 < input_end
                                && IS_UPPERCASE_CYRILLIC ((unsigned char) ip[2],
                                                          (unsigned char) ip[3]))
                            || (ip >= input + 1
                                && IS_UPPERCASE_LATIN ((unsigned char) ip[-1]))
                            || (ip >= input + 2
                                && IS_UPPERCASE_CYRILLIC ((unsigned char) ip[-2],
                                                          (unsigned char) ip[-1])))
                          {
                            /* Use the upper-case replacement instead of
                               the mixed-case replacement.  */
                            switch (uc)
                              {
                              case 0x0409:
                                repl = "LJ"; break;
                              case 0x040a:
                                repl = "NJ"; break;
                              case 0x040f:
                                repl = "D\xC5\xBD"/* "DŽ" */; break;
                              default:
                                abort ();
                              }
                          }

                      /* Use the replacement.  */
                      *op++ = *repl++;
                      if (*repl != '\0')
                        {
                          *op++ = *repl++;
                          if (*repl != '\0')
                            {
                              *op++ = *repl++;
                              /* All replacements have at most 3 bytes.  */
                              if (*repl != '\0')
                                abort ();
                            }
                        }
                      ip += 2;
                      continue;
                    }
                }
            }
        }
      *op++ = *ip++;
    }

  {
    size_t output_len = op - output;

    /* Verify that the allocated size was not exceeded.  */
    if (output_len > allocated)
      abort ();
    /* Shrink the result.  */
    if (output_len < allocated)
      output = (char *) xrealloc (output, output_len);

    /* Done.  */
    *output_p = output;
    *output_len_p = output_len;
  }
}
