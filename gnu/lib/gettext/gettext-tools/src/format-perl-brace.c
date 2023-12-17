/* Perl brace format strings.
   Copyright (C) 2004, 2006-2007, 2009, 2019-2020 Free Software Foundation, Inc.
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
#include <string.h>

#include "format.h"
#include "xalloc.h"
#include "gettext.h"

#define _(str) gettext (str)

/* Perl brace format strings are supported by Guido Flohr's libintl-perl
   package, more precisely by the __expand and __x functions therein.
   A format string directive here consists of
     - an opening brace '{',
     - an identifier [_A-Za-z][_0-9A-Za-z]*,
     - a closing brace '}'.
 */

struct named_arg
{
  char *name;
};

struct spec
{
  unsigned int directives;
  unsigned int named_arg_count;
  struct named_arg *named;
};


static int
named_arg_compare (const void *p1, const void *p2)
{
  return strcmp (((const struct named_arg *) p1)->name,
                 ((const struct named_arg *) p2)->name);
}

static void *
format_parse (const char *format, bool translated, char *fdi,
              char **invalid_reason)
{
  const char *const format_start = format;
  struct spec spec;
  unsigned int named_allocated;
  struct spec *result;

  spec.directives = 0;
  spec.named_arg_count = 0;
  spec.named = NULL;
  named_allocated = 0;

  for (; *format != '\0';)
    if (*format++ == '{')
      {
        const char *f = format;
        char c;

        c = *f;
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_')
          {
            do
              c = *++f;
            while ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_'
                   || (c >= '0' && c <= '9'));
            if (c == '}')
              {
                /* A directive.  */
                char *name;
                const char *name_start = format;
                const char *name_end = f;
                size_t n = name_end - name_start;

                FDI_SET (format - 1, FMTDIR_START);

                name = XNMALLOC (n + 1, char);
                memcpy (name, name_start, n);
                name[n] = '\0';

                spec.directives++;

                if (named_allocated == spec.named_arg_count)
                  {
                    named_allocated = 2 * named_allocated + 1;
                    spec.named = (struct named_arg *) xrealloc (spec.named, named_allocated * sizeof (struct named_arg));
                  }
                spec.named[spec.named_arg_count].name = name;
                spec.named_arg_count++;

                FDI_SET (f, FMTDIR_END);

                format = ++f;
              }
          }
      }

  /* Sort the named argument array, and eliminate duplicates.  */
  if (spec.named_arg_count > 1)
    {
      unsigned int i, j;

      qsort (spec.named, spec.named_arg_count, sizeof (struct named_arg),
             named_arg_compare);

      /* Remove duplicates: Copy from i to j, keeping 0 <= j <= i.  */
      for (i = j = 0; i < spec.named_arg_count; i++)
        if (j > 0 && strcmp (spec.named[i].name, spec.named[j-1].name) == 0)
          free (spec.named[i].name);
        else
          {
            if (j < i)
              spec.named[j].name = spec.named[i].name;
            j++;
          }
      spec.named_arg_count = j;
    }

  result = XMALLOC (struct spec);
  *result = spec;
  return result;
}

static void
format_free (void *descr)
{
  struct spec *spec = (struct spec *) descr;

  if (spec->named != NULL)
    {
      unsigned int i;
      for (i = 0; i < spec->named_arg_count; i++)
        free (spec->named[i].name);
      free (spec->named);
    }
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

  if (spec1->named_arg_count + spec2->named_arg_count > 0)
    {
      unsigned int i, j;
      unsigned int n1 = spec1->named_arg_count;
      unsigned int n2 = spec2->named_arg_count;

      /* Check the argument names in spec1 are contained in those of spec2.
         Additional arguments in spec2 are allowed; they expand to themselves
         (including the surrounding braces) at runtime.
         Both arrays are sorted.  We search for the differences.  */
      for (i = 0, j = 0; i < n1 || j < n2; )
        {
          int cmp = (i >= n1 ? 1 :
                     j >= n2 ? -1 :
                     strcmp (spec1->named[i].name, spec2->named[j].name));

          if (cmp > 0)
            j++;
          else if (cmp < 0)
            {
              if (equality)
                {
                  if (error_logger)
                    error_logger (_("a format specification for argument '%s' doesn't exist in '%s'"),
                                  spec1->named[i].name, pretty_msgstr);
                  err = true;
                  break;
                }
              else
                i++;
            }
          else
            j++, i++;
        }
    }

  return err;
}


struct formatstring_parser formatstring_perl_brace =
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

  printf ("{");
  for (i = 0; i < spec->named_arg_count; i++)
    {
      if (i > 0)
        printf (", ");
      printf ("'%s'", spec->named[i].name);
    }
  printf ("}");
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
 * compile-command: "/bin/sh ../libtool --tag=CC --mode=link gcc -o a.out -static -O -g -Wall -I.. -I../gnulib-lib -I../../gettext-runtime/intl -DHAVE_CONFIG_H -DTEST format-perl-brace.c ../gnulib-lib/libgettextlib.la"
 * End:
 */

#endif /* TEST */
