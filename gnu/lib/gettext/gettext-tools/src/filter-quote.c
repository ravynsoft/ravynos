/* Convert ASCII quotations to Unicode quotations.
   Copyright (C) 2014-2016 Free Software Foundation, Inc.
   Written by Daiki Ueno <ueno@gnu.org>, 2014.

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

#include "quote.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "xalloc.h"

#define BOLD_START "\x1b[1m"
#define BOLD_END "\x1b[0m"

struct result
{
  char *output;
  char *offset;
  bool bold;
};

static void
convert_quote_callback (char quote, const char *quoted, size_t quoted_length,
                        void *data)
{
  struct result *result = data;

  switch (quote)
    {
    case '\0':
      memcpy (result->offset, quoted, quoted_length);
      result->offset += quoted_length;
      break;

    case '"':
      /* U+201C: LEFT DOUBLE QUOTATION MARK */
      memcpy (result->offset, "\xe2\x80\x9c", 3);
      result->offset += 3;
      if (result->bold)
        {
          memcpy (result->offset, BOLD_START, 4);
          result->offset += 4;
        }
      memcpy (result->offset, quoted, quoted_length);
      result->offset += quoted_length;
      if (result->bold)
        {
          memcpy (result->offset, BOLD_END, 4);
          result->offset += 4;
        }
      /* U+201D: RIGHT DOUBLE QUOTATION MARK */
      memcpy (result->offset, "\xe2\x80\x9d", 3);
      result->offset += 3;
      break;

    case '\'':
      /* U+2018: LEFT SINGLE QUOTATION MARK */
      memcpy (result->offset, "\xe2\x80\x98", 3);
      result->offset += 3;
      if (result->bold)
        {
          memcpy (result->offset, BOLD_START, 4);
          result->offset += 4;
        }
      memcpy (result->offset, quoted, quoted_length);
      result->offset += quoted_length;
      if (result->bold)
        {
          memcpy (result->offset, BOLD_END, 4);
          result->offset += 4;
        }
      /* U+2019: RIGHT SINGLE QUOTATION MARK */
      memcpy (result->offset, "\xe2\x80\x99", 3);
      result->offset += 3;
      break;
    }
}

/* This is a direct translation of po/quot.sed and po/boldquot.sed.  */
static void
convert_ascii_quote_to_unicode (const char *input, size_t input_len,
                                char **output_p, size_t *output_len_p,
                                bool bold)
{
  const char *p;
  size_t quote_count;
  struct result result;

  /* Count the number of quotation characters.  */
  quote_count = 0;
  for (p = input; p < input + input_len; p++)
    {
      size_t len;

      p = strpbrk (p, "`'\"");
      if (!p)
        break;

      len = strspn (p, "`'\"");
      quote_count += len;
      p += len;
    }

  /* Large enough.  */
  result.output = XNMALLOC (input_len - quote_count
                            + (bold ? 7 : 3) * quote_count + 1,
                            char);
  result.offset = result.output;
  result.bold = bold;

  scan_quoted (input, input_len, convert_quote_callback, &result);

  *output_p = result.output;
  *output_len_p = result.offset - result.output;
}

void
ascii_quote_to_unicode (const char *input, size_t input_len,
                        char **output_p, size_t *output_len_p)
{
  convert_ascii_quote_to_unicode (input, input_len,
                                  output_p, output_len_p,
                                  false);
}

void
ascii_quote_to_unicode_bold (const char *input, size_t input_len,
                             char **output_p, size_t *output_len_p)
{
  convert_ascii_quote_to_unicode (input, input_len,
                                  output_p, output_len_p,
                                  true);
}
