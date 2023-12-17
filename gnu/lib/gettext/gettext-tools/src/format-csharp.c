/* C# format strings.
   Copyright (C) 2003-2004, 2006-2007, 2009, 2018-2019 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2003.

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

#include <stdbool.h>
#include <stdlib.h>

#include "format.h"
#include "c-ctype.h"
#include "xalloc.h"
#include "xvasprintf.h"
#include "gettext.h"

#define _(str) gettext (str)

/* C# format strings are described in the description of the .NET System.String
   class and implemented in mcs-0.28/class/corlib/System/String.cs .
   A format string consists of literal text (that is output verbatim), doubled
   braces ('{{' and '}}', that lead to a single brace when output), and
   directives.
   A directive
   - starts with '{',
   - is followed by a nonnegative integer m,
   - is optionally followed by ',' and an integer denoting a width,
   - is optionally followed by ':' and a sequence of format specifiers.
     (But the interpretation of the format specifiers is up to the IFormattable
     implementation, depending on the argument's runtime value. New classes
     implementing IFormattable can be defined by the user.)
   - is finished with '}'.
 */

struct spec
{
  unsigned int directives;
  unsigned int numbered_arg_count;
};

static void *
format_parse (const char *format, bool translated, char *fdi,
              char **invalid_reason)
{
  const char *const format_start = format;
  struct spec spec;
  struct spec *result;

  spec.directives = 0;
  spec.numbered_arg_count = 0;

  for (; *format != '\0';)
    {
      char c = *format++;

      if (c == '{')
        {
          FDI_SET (format - 1, FMTDIR_START);
          if (*format == '{')
            format++;
          else
            {
              /* A directive.  */
              unsigned int number;

              spec.directives++;

              if (!c_isdigit (*format))
                {
                  *invalid_reason =
                    xasprintf (_("In the directive number %u, '{' is not followed by an argument number."), spec.directives);
                  FDI_SET (*format == '\0' ? format - 1 : format, FMTDIR_ERROR);
                  return NULL;
                }
              number = 0;
              do
                {
                  number = 10 * number + (*format - '0');
                  format++;
                }
              while (c_isdigit (*format));

              if (*format == ',')
                {
                  /* Parse width.  */
                  format++;
                  if (*format == '-')
                    format++;
                  if (!c_isdigit (*format))
                    {
                      *invalid_reason =
                        xasprintf (_("In the directive number %u, ',' is not followed by a number."), spec.directives);
                      FDI_SET (*format == '\0' ? format - 1 : format,
                               FMTDIR_ERROR);
                      return NULL;
                    }
                  do
                    format++;
                  while (c_isdigit (*format));
                }

              if (*format == ':')
                {
                  /* Parse format specifiers.  */
                  do
                    format++;
                  while (*format != '\0' && *format != '}');
                }

              if (*format == '\0')
                {
                  *invalid_reason =
                    xstrdup (_("The string ends in the middle of a directive: found '{' without matching '}'."));
                  FDI_SET (format - 1, FMTDIR_ERROR);
                  return NULL;
                }

              if (*format != '}')
                {
                  *invalid_reason =
                    (c_isprint (*format)
                     ? xasprintf (_("The directive number %u ends with an invalid character '%c' instead of '}'."), spec.directives, *format)
                     : xasprintf (_("The directive number %u ends with an invalid character instead of '}'."), spec.directives));
                  FDI_SET (format, FMTDIR_ERROR);
                  return NULL;
                }

              format++;

              if (spec.numbered_arg_count <= number)
                spec.numbered_arg_count = number + 1;
            }
          FDI_SET (format - 1, FMTDIR_END);
        }
      else if (c == '}')
        {
          FDI_SET (format - 1, FMTDIR_START);
          if (*format == '}')
            format++;
          else
            {
              *invalid_reason =
                (spec.directives == 0
                 ? xstrdup (_("The string starts in the middle of a directive: found '}' without matching '{'."))
                 : xasprintf (_("The string contains a lone '}' after directive number %u."), spec.directives));
              FDI_SET (*format == '\0' ? format - 1 : format, FMTDIR_ERROR);
              return NULL;
            }
          FDI_SET (format - 1, FMTDIR_END);
        }
    }

  result = XMALLOC (struct spec);
  *result = spec;
  return result;
}

static void
format_free (void *descr)
{
  struct spec *spec = (struct spec *) descr;

  free (spec);
}

static int
format_get_number_of_directives (void *descr)
{
  struct spec *spec = (struct spec *) descr;

  return spec->directives;
}

static bool
format_check (void *msgid_descr, void *msgstr_descr, bool equality,
              formatstring_error_logger_t error_logger,
              const char *pretty_msgid, const char *pretty_msgstr)
{
  struct spec *spec1 = (struct spec *) msgid_descr;
  struct spec *spec2 = (struct spec *) msgstr_descr;
  bool err = false;

  /* Check that the argument counts are the same.  */
  if (equality
      ? spec1->numbered_arg_count != spec2->numbered_arg_count
      : spec1->numbered_arg_count < spec2->numbered_arg_count)
    {
      if (error_logger)
        error_logger (_("number of format specifications in '%s' and '%s' does not match"),
                      pretty_msgid, pretty_msgstr);
      err = true;
    }

  return err;
}


struct formatstring_parser formatstring_csharp =
{
  format_parse,
  format_free,
  format_get_number_of_directives,
  NULL,
  format_check
};


#ifdef TEST

/* Test program: Print the argument list specification returned by
   format_parse for strings read from standard input.  */

#include <stdio.h>

static void
format_print (void *descr)
{
  struct spec *spec = (struct spec *) descr;
  unsigned int i;

  if (spec == NULL)
    {
      printf ("INVALID");
      return;
    }

  printf ("(");
  for (i = 0; i < spec->numbered_arg_count; i++)
    {
      if (i > 0)
        printf (" ");
      printf ("*");
    }
  printf (")");
}

int
main ()
{
  for (;;)
    {
      char *line = NULL;
      size_t line_size = 0;
      int line_len;
      char *invalid_reason;
      void *descr;

      line_len = getline (&line, &line_size, stdin);
      if (line_len < 0)
        break;
      if (line_len > 0 && line[line_len - 1] == '\n')
        line[--line_len] = '\0';

      invalid_reason = NULL;
      descr = format_parse (line, false, NULL, &invalid_reason);

      format_print (descr);
      printf ("\n");
      if (descr == NULL)
        printf ("%s\n", invalid_reason);

      free (invalid_reason);
      free (line);
    }

  return 0;
}

/*
 * For Emacs M-x compile
 * Local Variables:
 * compile-command: "/bin/sh ../libtool --tag=CC --mode=link gcc -o a.out -static -O -g -Wall -I.. -I../gnulib-lib -I../../gettext-runtime/intl -DHAVE_CONFIG_H -DTEST format-csharp.c ../gnulib-lib/libgettextlib.la"
 * End:
 */

#endif /* TEST */
