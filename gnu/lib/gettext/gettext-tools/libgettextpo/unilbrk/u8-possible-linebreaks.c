/* Line breaking of UTF-8 strings.
   Copyright (C) 2001-2003, 2006-2023 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2001.

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
#include "unilbrk.h"
#include "unilbrk/internal.h"

#include <stdlib.h>
#include <string.h>

#include "unilbrk/lbrktables.h"
#include "uniwidth/cjk.h"
#include "unistr.h"

/* This file implements
   Unicode Standard Annex #14 <https://www.unicode.org/reports/tr14/>.  */

void
u8_possible_linebreaks_loop (const uint8_t *s, size_t n, const char *encoding,
                             int cr, char *p)
{
  if (n > 0)
    {
      int LBP_AI_REPLACEMENT = (is_cjk_encoding (encoding) ? LBP_ID1 : LBP_AL);
      const uint8_t *s_end = s + n;
      int prev_prop = LBP_BK; /* line break property of last character */
      int last_prop = LBP_BK; /* line break property of last non-space character */
      char *seen_space = NULL; /* Was a space seen after the last non-space character? */

      /* Don't break inside multibyte characters.  */
      memset (p, UC_BREAK_PROHIBITED, n);

      /* Number of consecutive regional indicator (RI) characters seen
         immediately before the current point.  */
      size_t ri_count = 0;

      do
        {
          ucs4_t uc;
          int count = u8_mbtouc_unsafe (&uc, s, s_end - s);
          int prop = unilbrkprop_lookup (uc);

          if (prop == LBP_BK || prop == LBP_LF || prop == LBP_CR)
            {
              /* (LB4,LB5,LB6) Mandatory break.  */
              *p = UC_BREAK_MANDATORY;
              /* cr is either LBP_CR or -1.  In the first case, recognize
                 a CR-LF sequence.  */
              if (prev_prop == cr && prop == LBP_LF)
                p[-1] = UC_BREAK_CR_BEFORE_LF;
              prev_prop = prop;
              last_prop = LBP_BK;
              seen_space = NULL;
            }
          else
            {
              /* Resolve property values whose behaviour is not fixed.  */
              switch (prop)
                {
                case LBP_AI:
                  /* Resolve ambiguous.  */
                  prop = LBP_AI_REPLACEMENT;
                  break;
                case LBP_CB:
                  /* This is arbitrary.  */
                  prop = LBP_ID1;
                  break;
                case LBP_SA:
                  /* We don't handle complex scripts yet.
                     Treat LBP_SA like LBP_XX.  */
                case LBP_XX:
                  /* This is arbitrary.  */
                  prop = LBP_AL;
                  break;
                }

              /* Deal with spaces and combining characters.  */
              if (prop == LBP_SP)
                {
                  /* (LB7) Don't break just before a space.  */
                  *p = UC_BREAK_PROHIBITED;
                  seen_space = p;
                }
              else if (prop == LBP_ZW)
                {
                  /* (LB7) Don't break just before a zero-width space.  */
                  *p = UC_BREAK_PROHIBITED;
                  last_prop = LBP_ZW;
                  seen_space = NULL;
                }
              else if (prop == LBP_CM || prop == LBP_ZWJ)
                {
                  /* (LB9) Don't break just before a combining character or
                     zero-width joiner, except immediately after a mandatory
                     break character, space, or zero-width space.  */
                  if (last_prop == LBP_BK)
                    {
                      /* (LB4,LB5,LB6) Don't break at the beginning of a line.  */
                      *p = UC_BREAK_PROHIBITED;
                      /* (LB10) Treat CM or ZWJ as AL.  */
                      last_prop = LBP_AL;
                      seen_space = NULL;
                    }
                  else if (last_prop == LBP_ZW || seen_space != NULL)
                    {
                      /* (LB8) Break after zero-width space.  */
                      /* (LB18) Break after spaces.
                         We do *not* implement the "legacy support for space
                         character as base for combining marks" because now the
                         NBSP CM sequence is recommended instead of SP CM.  */
                      *p = UC_BREAK_POSSIBLE;
                      /* (LB10) Treat CM or ZWJ as AL.  */
                      last_prop = LBP_AL;
                      seen_space = NULL;
                    }
                  else
                    {
                      /* Treat X CM as if it were X.  */
                      *p = UC_BREAK_PROHIBITED;
                    }
                }
              else
                {
                  /* prop must be usable as an index for table 7.3 of UTR #14.  */
                  if (!(prop >= 0 && prop < sizeof (unilbrk_table) / sizeof (unilbrk_table[0])))
                    abort ();

                  if (last_prop == LBP_BK)
                    {
                      /* (LB4,LB5,LB6) Don't break at the beginning of a line.  */
                      *p = UC_BREAK_PROHIBITED;
                    }
                  else if (last_prop == LBP_ZW)
                    {
                      /* (LB8) Break after zero-width space.  */
                      *p = UC_BREAK_POSSIBLE;
                    }
                  else if (prev_prop == LBP_ZWJ)
                    {
                      /* (LB8a) Don't break right after a zero-width joiner.  */
                      *p = UC_BREAK_PROHIBITED;
                    }
                  else if (last_prop == LBP_RI && prop == LBP_RI)
                    {
                      /* (LB30a) Break between two regional indicator symbols
                         if and only if there are an even number of regional
                         indicators preceding the position of the break.  */
                      *p = (seen_space != NULL || (ri_count % 2) == 0
                            ? UC_BREAK_POSSIBLE
                            : UC_BREAK_PROHIBITED);
                    }
                  else if (prev_prop == LBP_HL_BA)
                    {
                      /* (LB21a) Don't break after Hebrew + Hyphen/Break-After.  */
                      *p = UC_BREAK_PROHIBITED;
                    }
                  else
                    {
                      switch (unilbrk_table [last_prop] [prop])
                        {
                        case D:
                          *p = UC_BREAK_POSSIBLE;
                          break;
                        case I:
                          *p = (seen_space != NULL ? UC_BREAK_POSSIBLE : UC_BREAK_PROHIBITED);
                          break;
                        case P:
                          *p = UC_BREAK_PROHIBITED;
                          break;
                        default:
                          abort ();
                        }
                    }
                  last_prop = prop;
                  seen_space = NULL;
                }

              prev_prop = (prev_prop == LBP_HL && (prop == LBP_HY || prop == LBP_BA)
                           ? LBP_HL_BA
                           : prop);
            }

          if (prop == LBP_RI)
            ri_count++;
          else
            ri_count = 0;

          s += count;
          p += count;
        }
      while (s < s_end);
    }
}

#if defined IN_LIBUNISTRING
/* For backward compatibility with older versions of libunistring.  */

# undef u8_possible_linebreaks

void
u8_possible_linebreaks (const uint8_t *s, size_t n, const char *encoding,
                        char *p)
{
  u8_possible_linebreaks_loop (s, n, encoding, -1, p);
}

#endif

void
u8_possible_linebreaks_v2 (const uint8_t *s, size_t n, const char *encoding,
                           char *p)
{
  u8_possible_linebreaks_loop (s, n, encoding, LBP_CR, p);
}


#ifdef TEST

#include <stdio.h>
#include <string.h>

/* Read the contents of an input stream, and return it, terminated with a NUL
   byte. */
char *
read_file (FILE *stream)
{
#define BUFSIZE 4096
  char *buf = NULL;
  int alloc = 0;
  int size = 0;
  int count;

  while (! feof (stream))
    {
      if (size + BUFSIZE > alloc)
        {
          alloc = alloc + alloc / 2;
          if (alloc < size + BUFSIZE)
            alloc = size + BUFSIZE;
          buf = realloc (buf, alloc);
          if (buf == NULL)
            {
              fprintf (stderr, "out of memory\n");
              exit (1);
            }
        }
      count = fread (buf + size, 1, BUFSIZE, stream);
      if (count == 0)
        {
          if (ferror (stream))
            {
              perror ("fread");
              exit (1);
            }
        }
      else
        size += count;
    }
  buf = realloc (buf, size + 1);
  if (buf == NULL)
    {
      fprintf (stderr, "out of memory\n");
      exit (1);
    }
  buf[size] = '\0';
  return buf;
#undef BUFSIZE
}

int
main (int argc, char * argv[])
{
  if (argc == 1)
    {
      /* Display all the break opportunities in the input string.  */
      char *input = read_file (stdin);
      int length = strlen (input);
      char *breaks = malloc (length);
      int i;

      u8_possible_linebreaks_v2 ((uint8_t *) input, length, "UTF-8", breaks);

      for (i = 0; i < length; i++)
        {
          switch (breaks[i])
            {
            case UC_BREAK_POSSIBLE:
              /* U+2027 in UTF-8 encoding */
              putc (0xe2, stdout); putc (0x80, stdout); putc (0xa7, stdout);
              break;
            case UC_BREAK_MANDATORY:
              /* U+21B2 (or U+21B5) in UTF-8 encoding */
              putc (0xe2, stdout); putc (0x86, stdout); putc (0xb2, stdout);
              break;
            case UC_BREAK_CR_BEFORE_LF:
              /* U+21E4 in UTF-8 encoding */
              putc (0xe2, stdout); putc (0x87, stdout); putc (0xa4, stdout);
              break;
            case UC_BREAK_PROHIBITED:
              break;
            default:
              abort ();
            }
          putc (input[i], stdout);
        }

      free (breaks);

      return 0;
    }
  else
    return 1;
}

#endif /* TEST */
