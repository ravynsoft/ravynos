/* Sentence handling.
   Copyright (C) 2015 Free Software Foundation, Inc.
   Written by Daiki Ueno <ueno@gnu.org>, 2015.

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
#include "sentence.h"

#include <stdlib.h>
#include <string.h>
#include "unistr.h"


/* The minimal number of white spaces which should follow after the
   end of sentence.  */
int sentence_end_required_spaces = 1;

/* This function works in a similar way to 'forward-sentence' in
   Emacs, which basically does a regular expression matching of:

     [.?!\u2026]
       []"'\u201d)}]*
         \($\|[ \u00a0]$\|\t\|[ \u00a0]\{REQUIRED_SPACES\}\)

   Since we are lacking a regular expression routine capable of
   Unicode (though gnulib-lib/lib/regex.c provides a locale-dependent
   version, we would rather avoid depending on it), apply a manually
   constructed DFA, which consists of 8 states where 4 of them are a
   terminal.  */
const char *
sentence_end (const char *string, ucs4_t *ending_charp)
{
  const char *str = string;
  const char *str_limit = string + strlen (str);
  /* States of the DFA, 0 to 7, where 3, 5, 6, and 7 are a terminal.  */
  int state = 0;
  /* Previous character before an end marker.  */
  ucs4_t ending_char = 0xfffd;
  /* Possible starting position of the match, and the next starting
     position if the current match fails.  */
  const char *match_start = NULL, *match_next = NULL;
  /* Number of spaces.  */
  int spaces = 0;

  while (str <= str_limit)
    {
      ucs4_t uc;
      size_t length;

      length = u8_mbtouc (&uc, (const unsigned char *) str, str_limit - str);

      if (state == 0)
        {
          switch (uc)
            {
            case '.': case '?': case '!': case 0x2026:
              state = 1;
              match_start = str;
              match_next = str + length;
              ending_char = uc;
              spaces = 0;
              break;

            default:
              break;
            }

          str += length;
          continue;
        }

      if (state == 1)
        {
          switch (uc)
            {
            case ']': case '"': case '\'': case ')': case '}': case 0x201d:
              state = 2;
              break;

            case '\0': case '\n':
              /* State 3.  */
              *ending_charp = ending_char;
              return match_start;

            case ' ': case 0x00a0:
              if (++spaces == sentence_end_required_spaces)
                {
                  /* State 7.  */
                  *ending_charp = ending_char;
                  return match_start;
                }
              state = 4;
              break;

            case '\t':
              /* State 5.  */
              *ending_charp = ending_char;
              return match_start;

            default:
              str = match_next;
              state = 0;
              continue;
            }

          str += length;
          continue;
        }

      if (state == 2)
        {
          switch (uc)
            {
            case ']': case '"': case '\'': case ')': case '}': case 0x201d:
              break;

            case '\0': case '\n':
              /* State 3.  */
              *ending_charp = ending_char;
              return match_start;

            case ' ': case 0x00a0:
              if (++spaces == sentence_end_required_spaces)
                {
                  /* State 7.  */
                  *ending_charp = ending_char;
                  return match_start;
                }
              state = 4;
              break;

            case '\t':
              /* State 5.  */
              *ending_charp = ending_char;
              return match_start;

            default:
              state = 0;
              str = match_next;
              continue;
            }

          str += length;
          continue;
        }

      if (state == 4)
        {
          switch (uc)
            {
            case '\0': case '\n':
              /* State 6.  */
              *ending_charp = ending_char;
              return match_start;

            case ' ': case 0x00a0:
              if (++spaces == sentence_end_required_spaces)
                {
                  /* State 7.  */
                  *ending_charp = ending_char;
                  return match_start;
                }
              break;

            default:
              state = 0;
              str = match_next;
              continue;
            }

          str += length;
          continue;
        }
    }

  *ending_charp = 0xfffd;
  return str_limit;
}
