/* Smalltalk and YCP format strings.
   Copyright (C) 2001-2004, 2006-2007, 2009, 2019-2020 Free Software Foundation, Inc.
   Written by Bruno Haible <haible@clisp.cons.org>, 2001.

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
#include "format-invalid.h"
#include "gettext.h"

#define _(str) gettext (str)

/* YCP sformat strings are described in libycp documentation YCP-builtins.html.
   A directive starts with '%' and is followed by '%' or a nonzero digit ('1'
   to '9').
   GNU Smalltalk format strings are described in the CharArray documentation,
   methods 'bindWith:' and 'bindWithArguments:'. They have the same syntax.
 */

struct spec
{
  unsigned int directives;
  unsigned int arg_count;
  bool args_used[9];
};


static void *
format_parse (const char *format, bool translated, char *fdi,
              char **invalid_reason)
{
  const char *const format_start = format;
  struct spec spec;
  struct spec *result;

  spec.directives = 0;
  spec.arg_count = 0;

  for (; *format != '\0';)
    if (*format++ == '%')
      {
        /* A directive.  */
        FDI_SET (format - 1, FMTDIR_START);
        spec.directives++;

        if (*format == '%')
          format++;
        else if (*format >= '1' && *format <= '9')
          {
            unsigned int number = *format - '1';

            while (spec.arg_count <= number)
              spec.args_used[spec.arg_count++] = false;
            spec.args_used[number] = true;

            format++;
          }
        else
          {
            if (*format == '\0')
              {
                *invalid_reason = INVALID_UNTERMINATED_DIRECTIVE ();
                FDI_SET (format - 1, FMTDIR_ERROR);
              }
            else
              {
                *invalid_reason =
                  (c_isprint (*format)
                   ? xasprintf (_("In the directive number %u, the character '%c' is not a digit between 1 and 9."), spec.directives, *format)
                   : xasprintf (_("The character that terminates the directive number %u is not a digit between 1 and 9."), spec.directives));
                FDI_SET (format, FMTDIR_ERROR);
              }
            goto bad_format;
          }

        FDI_SET (format - 1, FMTDIR_END);
      }

  result = XMALLOC (struct spec);
  *result = spec;
  return result;

 bad_format:
  return NULL;
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
  unsigned int i;

  for (i = 0; i < spec1->arg_count || i < spec2->arg_count; i++)
    {
      bool arg_used1 = (i < spec1->arg_count && spec1->args_used[i]);
      bool arg_used2 = (i < spec2->arg_count && spec2->args_used[i]);

      if (equality ? (arg_used1 != arg_used2) : (!arg_used1 && arg_used2))
        {
          if (error_logger)
            {
              if (arg_used1)
                error_logger (_("a format specification for argument %u doesn't exist in '%s'"),
                              i + 1, pretty_msgstr);
              else
                error_logger (_("a format specification for argument %u, as in '%s', doesn't exist in '%s'"),
                              i + 1, pretty_msgstr, pretty_msgid);
            }
          err = true;
          break;
        }
    }

  return err;
}


struct formatstring_parser formatstring_smalltalk =
{
  format_parse,
  format_free,
  format_get_number_of_directives,
  NULL,
  format_check
};


struct formatstring_parser formatstring_ycp =
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
  for (i = 0; i < spec->arg_count; i++)
    {
      if (i > 0)
        printf (" ");
      if (spec->args_used[i])
        printf ("*");
      else
        printf ("_");
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
 * compile-command: "/bin/sh ../libtool --tag=CC --mode=link gcc -o a.out -static -O -g -Wall -I.. -I../gnulib-lib -I../../gettext-runtime/intl -DHAVE_CONFIG_H -DTEST format-smalltalk.c ../gnulib-lib/libgettextlib.la"
 * End:
 */

#endif /* TEST */
