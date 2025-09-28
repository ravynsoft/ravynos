/* Line breaking of strings.
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

#include <stdlib.h>
#include <string.h>

#include "c-ctype.h"
#include "uniconv.h"
#include "unilbrk/internal.h"
#include "unilbrk/lbrktables.h"
#include "unilbrk/ulc-common.h"

/* Line breaking of a string in an arbitrary encoding.

   We convert the input string to Unicode.

   The standardized Unicode encodings are UTF-8, UCS-2, UCS-4, UTF-16,
   UTF-16BE, UTF-16LE, UTF-7.  UCS-2 supports only characters up to
   \U0000FFFF.  UTF-16 and variants support only characters up to
   \U0010FFFF.  UTF-7 is way too complex and not supported by glibc-2.1.
   UCS-4 specification leaves doubts about endianness and byte order mark.
   glibc currently interprets it as big endian without byte order mark,
   but this is not backed by an RFC.  So we use UTF-8. It supports
   characters up to \U7FFFFFFF and is unambiguously defined.  */

static int
ulc_width_linebreaks_internal (const char *s, size_t n,
                               int width, int start_column, int at_end_columns,
                               const char *o, const char *encoding, int cr,
                               char *p)
{
  if (n > 0)
    {
      if (is_utf8_encoding (encoding))
        return u8_width_linebreaks_internal ((const uint8_t *) s, n, width, start_column, at_end_columns, o, encoding, cr, p);
      else
        {
          /* Convert the string to UTF-8 and build a translation table
             from offsets into s to offsets into the translated string.  */
          size_t *offsets = (size_t *) malloc (n * sizeof (size_t));

          if (offsets != NULL)
            {
              uint8_t *t;
              size_t m;

              t = u8_conv_from_encoding (encoding, iconveh_question_mark,
                                         s, n, offsets, NULL, &m);
              if (t != NULL)
                {
                  char *memory =
                    (char *) (m > 0 ? malloc (m + (o != NULL ? m : 0)) : NULL);

                  if (m == 0 || memory != NULL)
                    {
                      char *q = (char *) memory;
                      char *o8 = (o != NULL ? (char *) (q + m) : NULL);
                      int res_column;
                      size_t i;

                      /* Translate the overrides to the UTF-8 string.  */
                      if (o != NULL)
                        {
                          memset (o8, UC_BREAK_UNDEFINED, m);
                          for (i = 0; i < n; i++)
                            if (offsets[i] != (size_t)(-1))
                              o8[offsets[i]] = o[i];
                        }

                      /* Determine the line breaks of the UTF-8 string.  */
                      res_column =
                        u8_width_linebreaks_internal (t, m, width, start_column, at_end_columns, o8, encoding, cr, q);

                      /* Translate the result back to the original string.  */
                      memset (p, UC_BREAK_PROHIBITED, n);
                      for (i = 0; i < n; i++)
                        if (offsets[i] != (size_t)(-1))
                          p[i] = q[offsets[i]];

                      free (memory);
                      free (t);
                      free (offsets);
                      return res_column;
                    }
                  free (t);
                }
              free (offsets);
            }
          /* Impossible to convert.  */
#if C_CTYPE_ASCII
          if (is_all_ascii (s, n))
            {
              /* ASCII is a subset of UTF-8.  */
              return u8_width_linebreaks_internal ((const uint8_t *) s, n, width, start_column, at_end_columns, o, encoding, cr, p);
            }
#endif
          /* We have a non-ASCII string and cannot convert it.
             Don't produce line breaks except those already present in the
             input string.  All we assume here is that the encoding is
             minimally ASCII compatible.  */
          {
            const char *s_end = s + n;
            while (s < s_end)
              {
                *p = ((o != NULL && *o == UC_BREAK_MANDATORY)
                      || *s == '\n'
                      ? UC_BREAK_MANDATORY
                      : ((o != NULL && *o == UC_BREAK_CR_BEFORE_LF)
                         || (cr >= 0
                             && *s == '\r'
                             && s + 1 < s_end
                             && *(s + 1) == '\n')
                         ? UC_BREAK_CR_BEFORE_LF
                         : UC_BREAK_PROHIBITED));
                s++;
                p++;
                if (o != NULL)
                  o++;
              }
            /* We cannot compute widths in this case.  */
          }
        }
    }
  return start_column;
}

#if defined IN_LIBUNISTRING
/* For backward compatibility with older versions of libunistring.  */

# undef ulc_width_linebreaks

int
ulc_width_linebreaks (const char *s, size_t n,
                      int width, int start_column, int at_end_columns,
                      const char *o, const char *encoding,
                      char *p)
{
  return ulc_width_linebreaks_internal (s, n,
                                        width, start_column, at_end_columns,
                                        o, encoding, -1, p);
}

#endif

int
ulc_width_linebreaks_v2 (const char *s, size_t n,
                         int width, int start_column, int at_end_columns,
                         const char *o, const char *encoding,
                         char *p)
{
  return ulc_width_linebreaks_internal (s, n,
                                        width, start_column, at_end_columns,
                                        o, encoding, LBP_CR, p);
}


#ifdef TEST

#include <stdio.h>
#include <locale.h>

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
  setlocale (LC_CTYPE, "");
  if (argc == 2)
    {
      /* Insert line breaks for a given width.  */
      int width = atoi (argv[1]);
      char *input = read_file (stdin);
      int length = strlen (input);
      char *breaks = malloc (length);
      int i;

      ulc_width_linebreaks_v2 (input, length, width, 0, 0, NULL, locale_charset (), breaks);

      for (i = 0; i < length; i++)
        {
          switch (breaks[i])
            {
            case UC_BREAK_POSSIBLE:
              putc ('\n', stdout);
              break;
            case UC_BREAK_MANDATORY:
              break;
            case UC_BREAK_CR_BEFORE_LF:
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
