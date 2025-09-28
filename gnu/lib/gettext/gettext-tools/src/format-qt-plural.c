/* Qt plural format strings.
   Copyright (C) 2003-2004, 2006-2007, 2009, 2019 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2009.

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
#include "xalloc.h"
#include "gettext.h"

#define _(str) gettext (str)

/* Qt plural format strings are processed by QObject::tr and are documented in
   qt-x11-opensource-src-4.3.1/doc/html/qobject.html#tr.
   A directive
     - starts with '%',
     - is optionally followed by 'L' (a no-op),
     - is followed by 'n'.
   Every directive is replaced by the numeric argument N passed to QObject::tr.
 */

struct spec
{
  /* Number of format directives.  */
  unsigned int directives;
};


static void *
format_parse (const char *format, bool translated, char *fdi,
              char **invalid_reason)
{
  const char *const format_start = format;
  struct spec spec;
  struct spec *result;

  spec.directives = 0;

  for (; *format != '\0';)
    if (*format++ == '%')
      {
        const char *dir_start = format - 1;

        if (*format == 'L')
          format++;
        if (*format == 'n')
          {
            /* A directive.  */
            FDI_SET (dir_start, FMTDIR_START);
            spec.directives++;
            FDI_SET (format, FMTDIR_END);

            format++;
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

  /* Check the argument is used.  */
  if ((spec1->directives == 0 && spec2->directives > 0)
      || (equality && spec1->directives > 0 && spec2->directives == 0))
    {
      if (error_logger)
        error_logger (_("number of format specifications in '%s' and '%s' does not match"),
                      pretty_msgid, pretty_msgstr);
      err = true;
    }

  return err;
}


struct formatstring_parser formatstring_qt_plural =
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

  if (spec == NULL)
    {
      printf ("INVALID");
      return;
    }

  printf ("(");
  if (spec->directives > 0)
    printf ("*");
  else
    printf ("_");
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
 * compile-command: "/bin/sh ../libtool --tag=CC --mode=link gcc -o a.out -static -O -g -Wall -I.. -I../gnulib-lib -I../../gettext-runtime/intl -DHAVE_CONFIG_H -DTEST format-qt-plural.c ../gnulib-lib/libgettextlib.la"
 * End:
 */

#endif /* TEST */
