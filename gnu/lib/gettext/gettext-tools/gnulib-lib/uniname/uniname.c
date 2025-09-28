/* Association between Unicode characters and their names.
   Copyright (C) 2000-2002, 2005-2007, 2009-2023 Free Software Foundation, Inc.

   This file is free software.
   It is dual-licensed under "the GNU LGPLv3+ or the GNU GPLv2+".
   You can redistribute it and/or modify it under either
     - the terms of the GNU Lesser General Public License as published
       by the Free Software Foundation, either version 3, or (at your
       option) any later version, or
     - the terms of the GNU General Public License as published by the
       Free Software Foundation; either version 2, or (at your option)
       any later version, or
     - the same dual license "the GNU LGPLv3+ or the GNU GPLv2+".

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License and the GNU General Public License
   for more details.

   You should have received a copy of the GNU Lesser General Public
   License and of the GNU General Public License along with this
   program.  If not, see <https://www.gnu.org/licenses/>.  */

#include <config.h>

/* Specification.  */
#include "uniname.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "attribute.h"

#define SIZEOF(a) (sizeof(a) / sizeof(a[0]))


/* Table of Unicode character names, derived from UnicodeData.txt.
   This table is generated in a way to minimize the memory footprint:
     1. its compiled size is small (less than 350 KB),
     2. it resides entirely in the text or read-only data segment of the
        executable or shared library: the table contains only immediate
        integers, no pointers, and the functions don't do heap allocation.
 */
#include "uninames.h"
/* It contains:
  static const char unicode_name_words[36303] = ...;
  #define UNICODE_CHARNAME_NUM_WORDS 6260
  static const struct { uint16_t extra_offset; uint16_t ind_offset; } unicode_name_by_length[26] = ...;
  #define UNICODE_CHARNAME_WORD_HANGUL 3902
  #define UNICODE_CHARNAME_WORD_SYLLABLE 4978
  #define UNICODE_CHARNAME_WORD_CJK 417
  #define UNICODE_CHARNAME_WORD_COMPATIBILITY 6107
  static const uint16_t unicode_names[68940] = ...;
  static const struct { uint16_t index; uint32_t name:24; } unicode_name_to_index[16626] = ...;
  static const struct { uint16_t index; uint32_t name:24; } unicode_index_to_name[16626] = ...;
  #define UNICODE_CHARNAME_MAX_LENGTH 83
  #define UNICODE_CHARNAME_MAX_WORDS 13
  static const struct { uint32_t index; uint32_t gap; uint16_t length; } unicode_ranges[401] = ...;
*/

/* Returns the word with a given index.  */
static const char *
unicode_name_word (unsigned int index, unsigned int *lengthp)
{
  unsigned int i1;
  unsigned int i2;

  assert (index < UNICODE_CHARNAME_NUM_WORDS);

  /* Binary search for i with
       unicode_name_by_length[i].ind_offset <= index
     and
       index < unicode_name_by_length[i+1].ind_offset
   */

  i1 = 0;
  i2 = SIZEOF (unicode_name_by_length) - 1;
  while (i2 - i1 > 1)
    {
      unsigned int i = (i1 + i2) >> 1;
      if (unicode_name_by_length[i].ind_offset <= index)
        i1 = i;
      else
        i2 = i;
    }
  unsigned int i = i1;
  assert (unicode_name_by_length[i].ind_offset <= index
          && index < unicode_name_by_length[i+1].ind_offset);
  *lengthp = i;
  return &unicode_name_words[unicode_name_by_length[i].extra_offset
                             + (index-unicode_name_by_length[i].ind_offset)*i];
}

/* Looks up the index of a word.  */
static int
unicode_name_word_lookup (const char *word, size_t length)
{
  if (length > 0 && length < SIZEOF (unicode_name_by_length) - 1)
    {
      /* Binary search among the words of given length.  */
      unsigned int extra_offset = unicode_name_by_length[length].extra_offset;
      unsigned int i0 = unicode_name_by_length[length].ind_offset;
      unsigned int i1 = i0;
      unsigned int i2 = unicode_name_by_length[length+1].ind_offset;
      while (i2 - i1 > 0)
        {
          unsigned int i = (i1 + i2) >> 1;
          const char *p = &unicode_name_words[extra_offset + (i-i0)*length];
          const char *w = word;
          unsigned int n = length;
          for (;;)
            {
              if (*p < *w)
                {
                  if (i1 == i)
                    return -1;
                  /* Note here: i1 < i < i2.  */
                  i1 = i;
                  break;
                }
              if (*p > *w)
                {
                  /* Note here: i1 <= i < i2.  */
                  i2 = i;
                  break;
                }
              p++; w++; n--;
              if (n == 0)
                return i;
            }
        }
    }
  return -1;
}

#define UNINAME_INVALID_INDEX UINT16_MAX

/* Looks up the internal index of a Unicode character.  */
static uint16_t
unicode_code_to_index (ucs4_t c)
{
  /* Binary search in unicode_ranges.  */
  unsigned int i1 = 0;
  unsigned int i2 = SIZEOF (unicode_ranges);

  for (;;)
    {
      unsigned int i = (i1 + i2) >> 1;
      ucs4_t start_code =
        unicode_ranges[i].index + unicode_ranges[i].gap;
      ucs4_t end_code =
        start_code + unicode_ranges[i].length - 1;

      if (start_code <= c && c <= end_code)
        return c - unicode_ranges[i].gap;

      if (end_code < c)
        {
          if (i1 == i)
            break;
          /* Note here: i1 < i < i2.  */
          i1 = i;
        }
      else if (c < start_code)
        {
          if (i2 == i)
            break;
          /* Note here: i1 <= i < i2.  */
          i2 = i;
        }
    }
  return UNINAME_INVALID_INDEX;
}

/* Looks up the codepoint of a Unicode character, from the given
   internal index.  */
static ucs4_t
unicode_index_to_code (uint16_t index)
{
  /* Binary search in unicode_ranges.  */
  unsigned int i1 = 0;
  unsigned int i2 = SIZEOF (unicode_ranges);

  for (;;)
    {
      unsigned int i = (i1 + i2) >> 1;
      uint16_t start_index = unicode_ranges[i].index;
      uint16_t end_index = start_index + unicode_ranges[i].length - 1;

      if (start_index <= index && index <= end_index)
        return index + unicode_ranges[i].gap;

      if (end_index < index)
        {
          if (i1 == i)
            break;
          /* Note here: i1 < i < i2.  */
          i1 = i;
        }
      else if (index < start_index)
        {
          if (i2 == i)
            break;
          /* Note here: i1 <= i < i2.  */
          i2 = i;
        }
    }
  return UNINAME_INVALID;
}


/* Auxiliary tables for Hangul syllable names, see the Unicode 3.0 book,
   sections 3.11 and 4.4.  */
static const char jamo_initial_short_name[19][3] =
{
  "G", "GG", "N", "D", "DD", "R", "M", "B", "BB", "S", "SS", "", "J", "JJ",
  "C", "K", "T", "P", "H"
};
static const char jamo_medial_short_name[21][4] =
{
  "A", "AE", "YA", "YAE", "EO", "E", "YEO", "YE", "O", "WA", "WAE", "OE", "YO",
  "U", "WEO", "WE", "WI", "YU", "EU", "YI", "I"
};
static const char jamo_final_short_name[28][3] =
{
  "", "G", "GG", "GS", "N", "NI", "NH", "D", "L", "LG", "LM", "LB", "LS", "LT",
  "LP", "LH", "M", "B", "BS", "S", "SS", "NG", "J", "C", "K", "T", "P", "H"
};

/* Looks up the name of a Unicode character, in uppercase ASCII.
   Returns the filled buf, or NULL if the character does not have a name.  */
char *
unicode_character_name (ucs4_t c, char *buf)
{
  if (c >= 0xAC00 && c <= 0xD7A3)
    {
      /* Special case for Hangul syllables. Keeps the tables small.  */
      char *ptr;
      unsigned int tmp;
      unsigned int index1;
      unsigned int index2;
      unsigned int index3;
      const char *q;

      /* buf needs to have at least 16 + 7 + 1 bytes here.  */
      memcpy (buf, "HANGUL SYLLABLE ", 16);
      ptr = buf + 16;

      tmp = c - 0xAC00;
      index3 = tmp % 28; tmp = tmp / 28;
      index2 = tmp % 21; tmp = tmp / 21;
      index1 = tmp;

      q = jamo_initial_short_name[index1];
      while (*q != '\0')
        *ptr++ = *q++;
      q = jamo_medial_short_name[index2];
      while (*q != '\0')
        *ptr++ = *q++;
      q = jamo_final_short_name[index3];
      while (*q != '\0')
        *ptr++ = *q++;
      *ptr = '\0';
      return buf;
    }
  else if ((c >= 0xF900 && c <= 0xFA2D) || (c >= 0xFA30 && c <= 0xFA6A)
           || (c >= 0xFA70 && c <= 0xFAD9) || (c >= 0x2F800 && c <= 0x2FA1D))
    {
      /* Special case for CJK compatibility ideographs. Keeps the tables
         small.  */
      char *ptr;
      int i;

      /* buf needs to have at least 28 + 5 + 1 bytes here.  */
      memcpy (buf, "CJK COMPATIBILITY IDEOGRAPH-", 28);
      ptr = buf + 28;

      for (i = (c < 0x10000 ? 12 : 16); i >= 0; i -= 4)
        {
          unsigned int x = (c >> i) & 0xf;
          *ptr++ = (x < 10 ? '0' : 'A' - 10) + x;
        }
      *ptr = '\0';
      return buf;
    }
  else if ((c >= 0xFE00 && c <= 0xFE0F) || (c >= 0xE0100 && c <= 0xE01EF))
    {
      /* Special case for variation selectors. Keeps the tables
         small.  */

      /* buf needs to have at least 19 + 3 + 1 bytes here.  */
      sprintf (buf, "VARIATION SELECTOR-%u",
               c <= 0xFE0F ? c - 0xFE00 + 1 : c - 0xE0100 + 17);
      return buf;
    }
  else
    {
      uint16_t index = unicode_code_to_index (c);
      const uint16_t *words = NULL;

      if (index != UNINAME_INVALID_INDEX)
        {
          /* Binary search in unicode_code_to_name.  */
          unsigned int i1 = 0;
          unsigned int i2 = SIZEOF (unicode_index_to_name);
          for (;;)
            {
              unsigned int i = (i1 + i2) >> 1;
              if (unicode_index_to_name[i].index == index)
                {
                  words = &unicode_names[unicode_index_to_name[i].name];
                  break;
                }
              else if (unicode_index_to_name[i].index < index)
                {
                  if (i1 == i)
                    {
                      words = NULL;
                      break;
                    }
                  /* Note here: i1 < i < i2.  */
                  i1 = i;
                }
              else if (unicode_index_to_name[i].index > index)
                {
                  if (i2 == i)
                    {
                      words = NULL;
                      break;
                    }
                  /* Note here: i1 <= i < i2.  */
                  i2 = i;
                }
            }
        }
      if (words != NULL)
        {
          /* Found it in unicode_index_to_name. Now concatenate the words.  */
          /* buf needs to have at least UNICODE_CHARNAME_MAX_LENGTH + 1
             bytes.  */
          char *ptr = buf;
          for (;;)
            {
              unsigned int wordlen;
              const char *word = unicode_name_word (*words>>1, &wordlen);
              do
                *ptr++ = *word++;
              while (--wordlen > 0);
              if ((*words & 1) == 0)
                break;
              *ptr++ = ' ';
              words++;
            }
          *ptr = '\0';
          return buf;
        }
      return NULL;
    }
}

/* Looks up the Unicode character with a given name, in upper- or lowercase
   ASCII.  Returns the character if found, or UNINAME_INVALID if not found.  */
ucs4_t
unicode_name_character (const char *name)
{
  size_t len = strlen (name);
  if (len > 1 && len <= UNICODE_CHARNAME_MAX_LENGTH)
    {
      /* Test for "word1 word2 ..." syntax.  */
      char buf[UNICODE_CHARNAME_MAX_LENGTH];
      char *ptr = buf;
      for (;;)
        {
          char c = *name++;
          if (!(c >= ' ' && c <= '~'))
            break;
          *ptr++ = (c >= 'a' && c <= 'z' ? c - 'a' + 'A' : c);
          if (--len == 0)
            goto filled_buf;
        }
      if (false)
      filled_buf:
        {
          {
            /* Special case for variation selector aliases. Keeps the
               tables small.  */
            const char *p1 = buf;
            if (ptr >= buf + 3 && *p1++ == 'V')
              {
                if (*p1++ == 'S')
                  {
                    if (*p1 != '0')
                      {
                        unsigned int c = 0;
                        for (;;)
                          {
                            if (*p1 >= '0' && *p1 <= '9')
                              c += (*p1 - '0');
                            p1++;
                            if (p1 == ptr)
                              {
                                if (c >= 1 && c <= 16)
                                  return c - 1 + 0xFE00;
                                else if (c >= 17 && c <= 256)
                                  return c - 17 + 0xE0100;
                                else
                                  break;
                              }
                            c = c * 10;
                          }
                      }
                  }
              }
          }
          {
            /* Convert the constituents to uint16_t words.  */
            uint16_t words[UNICODE_CHARNAME_MAX_WORDS];
            uint16_t *wordptr = words;
            {
              const char *p1 = buf;
              for (;;)
                {
                  {
                    int word;
                    const char *p2 = p1;
                    while (p2 < ptr && *p2 != ' ')
                      p2++;
                    word = unicode_name_word_lookup (p1, p2 - p1);
                    if (word < 0)
                      break;
                    if (wordptr == &words[UNICODE_CHARNAME_MAX_WORDS])
                      break;
                    *wordptr++ = word;
                    if (p2 == ptr)
                      goto filled_words;
                    p1 = p2 + 1;
                  }
                  /* Special case for Hangul syllables. Keeps the tables small. */
                  if (wordptr == &words[2]
                      && words[0] == UNICODE_CHARNAME_WORD_HANGUL
                      && words[1] == UNICODE_CHARNAME_WORD_SYLLABLE)
                    {
                      /* Split the last word [p1..ptr) into three parts:
                           1) [BCDGHJKMNPRST]
                           2) [AEIOUWY]
                           3) [BCDGHIJKLMNPST]
                       */
                      const char *p2;
                      const char *p3;
                      const char *p4;

                      p2 = p1;
                      while (p2 < ptr
                             && (*p2 == 'B' || *p2 == 'C' || *p2 == 'D'
                                 || *p2 == 'G' || *p2 == 'H' || *p2 == 'J'
                                 || *p2 == 'K' || *p2 == 'M' || *p2 == 'N'
                                 || *p2 == 'P' || *p2 == 'R' || *p2 == 'S'
                                 || *p2 == 'T'))
                        p2++;
                      p3 = p2;
                      while (p3 < ptr
                             && (*p3 == 'A' || *p3 == 'E' || *p3 == 'I'
                                 || *p3 == 'O' || *p3 == 'U' || *p3 == 'W'
                                 || *p3 == 'Y'))
                        p3++;
                      p4 = p3;
                      while (p4 < ptr
                             && (*p4 == 'B' || *p4 == 'C' || *p4 == 'D'
                                 || *p4 == 'G' || *p4 == 'H' || *p4 == 'I'
                                 || *p4 == 'J' || *p4 == 'K' || *p4 == 'L'
                                 || *p4 == 'M' || *p4 == 'N' || *p4 == 'P'
                                 || *p4 == 'S' || *p4 == 'T'))
                        p4++;
                      if (p4 == ptr)
                        {
                          size_t n1 = p2 - p1;
                          size_t n2 = p3 - p2;
                          size_t n3 = p4 - p3;

                          if (n1 <= 2 && (n2 >= 1 && n2 <= 3) && n3 <= 2)
                            {
                              unsigned int index1;

                              for (index1 = 0; index1 < 19; index1++)
                                if (memcmp (jamo_initial_short_name[index1], p1, n1) == 0
                                    && jamo_initial_short_name[index1][n1] == '\0')
                                  {
                                    unsigned int index2;

                                    for (index2 = 0; index2 < 21; index2++)
                                      if (memcmp (jamo_medial_short_name[index2], p2, n2) == 0
                                          && jamo_medial_short_name[index2][n2] == '\0')
                                        {
                                          unsigned int index3;

                                          for (index3 = 0; index3 < 28; index3++)
                                            if (memcmp (jamo_final_short_name[index3], p3, n3) == 0
                                                && jamo_final_short_name[index3][n3] == '\0')
                                              {
                                                return 0xAC00 + (index1 * 21 + index2) * 28 + index3;
                                              }
                                          break;
                                        }
                                    break;
                                  }
                            }
                        }
                    }
                  /* Special case for CJK compatibility ideographs. Keeps the
                     tables small.  */
                  if (wordptr == &words[2]
                      && words[0] == UNICODE_CHARNAME_WORD_CJK
                      && words[1] == UNICODE_CHARNAME_WORD_COMPATIBILITY
                      && p1 + 14 <= ptr
                      && p1 + 15 >= ptr
                      && memcmp (p1, "IDEOGRAPH-", 10) == 0)
                    {
                      const char *p2 = p1 + 10;

                      if (*p2 != '0')
                        {
                          unsigned int c = 0;

                          for (;;)
                            {
                              if (*p2 >= '0' && *p2 <= '9')
                                c += (*p2 - '0');
                              else if (*p2 >= 'A' && *p2 <= 'F')
                                c += (*p2 - 'A' + 10);
                              else
                                break;
                              p2++;
                              if (p2 == ptr)
                                {
                                  if ((c >= 0xF900 && c <= 0xFA2D)
                                      || (c >= 0xFA30 && c <= 0xFA6A)
                                      || (c >= 0xFA70 && c <= 0xFAD9)
                                      || (c >= 0x2F800 && c <= 0x2FA1D))
                                    return c;
                                  else
                                    break;
                                }
                              c = c << 4;
                            }
                        }
                    }
                  /* Special case for variation selectors. Keeps the
                     tables small.  */
                  if (wordptr == &words[1]
                      && words[0] == UNICODE_CHARNAME_WORD_VARIATION
                      && p1 + 10 <= ptr
                      && p1 + 12 >= ptr
                      && memcmp (p1, "SELECTOR-", 9) == 0)
                    {
                      const char *p2 = p1 + 9;

                      if (*p2 != '0')
                        {
                          unsigned int c = 0;

                          for (;;)
                            {
                              if (*p2 >= '0' && *p2 <= '9')
                                c += (*p2 - '0');
                              p2++;
                              if (p2 == ptr)
                                {
                                  if (c >= 1 && c <= 16)
                                    return c - 1 + 0xFE00;
                                  else if (c >= 17 && c <= 256)
                                    return c - 17 + 0xE0100;
                                  else
                                    break;
                                }
                              c = c * 10;
                            }
                        }
                    }
                }
            }
            if (false)
            filled_words:
              {
                /* Multiply by 2, to simplify later comparisons.  */
                size_t words_length = wordptr - words;
                {
                  size_t i = words_length - 1;
                  words[i] = 2 * words[i];
                  for (; i > 0; )
                    {
                      --i;
                      words[i] = 2 * words[i] + 1;
                    }
                }
                /* Binary search in unicode_name_to_index.  */
                {
                  unsigned int i1 = 0;
                  unsigned int i2 = SIZEOF (unicode_name_to_index);
                  for (;;)
                    {
                      unsigned int i = (i1 + i2) >> 1;
                      const uint16_t *w = words;
                      const uint16_t *p = &unicode_names[unicode_name_to_index[i].name];
                      size_t n = words_length;
                      for (;;)
                        {
                          if (*p < *w)
                            {
                              if (i1 == i)
                                goto name_not_found;
                              /* Note here: i1 < i < i2.  */
                              i1 = i;
                              break;
                            }
                          else if (*p > *w)
                            {
                              if (i2 == i)
                                goto name_not_found;
                              /* Note here: i1 <= i < i2.  */
                              i2 = i;
                              break;
                            }
                          p++; w++; n--;
                          if (n == 0)
                            return unicode_index_to_code (unicode_name_to_index[i].index);
                        }
                    }
                }
              name_not_found: ;
              }
          }
        }
    }
  return UNINAME_INVALID;
}
