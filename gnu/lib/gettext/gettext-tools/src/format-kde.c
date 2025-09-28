/* KDE format strings.
   Copyright (C) 2003-2004, 2006-2007, 2009, 2019-2020, 2023 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2007.

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
#include "xvasprintf.h"
#include "gettext.h"

#define _(str) gettext (str)

/* KDE 4 format strings are processed by method
   KLocalizedStringPrivate::substituteSimple(string,'%',false) in
   kde4libs-3.93.0.orig/kdecore/localization/klocalizedstring.cpp .
   A directive
     - starts with '%',
     - is followed by a non-zero digit and optionally more digits. All
       the following digits are eaten up.
   An unterminated directive ('%' not followed by a digit or at the end) is
   not an error.
   %1 denotes the first argument, %2 the second argument, etc.
   The set of used argument numbers must be of the form {1,...,n} or
   {1,...,n} \ {m}: one of the supplied arguments may be ignored by the
   format string. This allows the processing of singular forms (msgstr[0]).
   Which argument may be skipped, depends on the argument types at runtime;
   since xgettext cannot extract this info, it is considered unknown here.  */

struct numbered_arg
{
  unsigned int number;
};

struct spec
{
  unsigned int directives;
  unsigned int numbered_arg_count;
  struct numbered_arg *numbered;
};

static int
numbered_arg_compare (const void *p1, const void *p2)
{
  /* Subtract 1, because argument number 0 can only occur through overflow.  */
  unsigned int n1 = ((const struct numbered_arg *) p1)->number - 1;
  unsigned int n2 = ((const struct numbered_arg *) p2)->number - 1;

  return (n1 > n2 ? 1 : n1 < n2 ? -1 : 0);
}

static void *
format_parse (const char *format, bool translated, char *fdi,
              char **invalid_reason)
{
  const char *const format_start = format;
  struct spec spec;
  unsigned int numbered_allocated;
  struct spec *result;

  spec.directives = 0;
  spec.numbered_arg_count = 0;
  spec.numbered = NULL;
  numbered_allocated = 0;

  for (; *format != '\0';)
    if (*format++ == '%')
      {
        const char *dir_start = format - 1;

        if (*format > '0' && *format <= '9')
          {
            /* A directive.  */
            unsigned int number;

            FDI_SET (dir_start, FMTDIR_START);
            spec.directives++;

            number = *format - '0';
            while (format[1] >= '0' && format[1] <= '9')
              {
                number = 10 * number + (format[1] - '0');
                format++;
              }

            if (numbered_allocated == spec.numbered_arg_count)
              {
                numbered_allocated = 2 * numbered_allocated + 1;
                spec.numbered = (struct numbered_arg *) xrealloc (spec.numbered, numbered_allocated * sizeof (struct numbered_arg));
              }
            spec.numbered[spec.numbered_arg_count].number = number;
            spec.numbered_arg_count++;

            FDI_SET (format, FMTDIR_END);

            format++;
          }
      }

  /* Sort the numbered argument array, and eliminate duplicates.  */
  if (spec.numbered_arg_count > 1)
    {
      unsigned int i, j;

      qsort (spec.numbered, spec.numbered_arg_count,
             sizeof (struct numbered_arg), numbered_arg_compare);

      /* Remove duplicates: Copy from i to j, keeping 0 <= j <= i.  */
      for (i = j = 0; i < spec.numbered_arg_count; i++)
        if (j > 0 && spec.numbered[i].number == spec.numbered[j-1].number)
          ;
        else
          {
            if (j < i)
              spec.numbered[j].number = spec.numbered[i].number;
            j++;
          }
      spec.numbered_arg_count = j;
    }
  /* Now spec.numbered[i] >= i + 1 for i = 0,..,spec.numbered_arg_count-1
     (since the numbered argument counts are strictly increasing, considering
     0 as overflow).  */

  /* Verify that the argument numbers are of the form {1,...,n} or
     {1,...,n} \ {m}.  */
  if (spec.numbered_arg_count > 0)
    {
      unsigned int i;

      i = 0;
      for (; i < spec.numbered_arg_count; i++)
        if (spec.numbered[i].number > i + 1)
          {
            unsigned int first_gap = i + 1;
            for (; i < spec.numbered_arg_count; i++)
              if (spec.numbered[i].number > i + 2)
                {
                  unsigned int second_gap = i + 2;
                  *invalid_reason =
                    xasprintf (_("The string refers to argument number %u but ignores the arguments %u and %u."),
                               spec.numbered[i].number, first_gap, second_gap);
                  goto bad_format;
                }
             break;
          }
    }

  result = XMALLOC (struct spec);
  *result = spec;
  return result;

 bad_format:
  if (spec.numbered != NULL)
    free (spec.numbered);
  return NULL;
}

static void
format_free (void *descr)
{
  struct spec *spec = (struct spec *) descr;

  if (spec->numbered != NULL)
    free (spec->numbered);
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

  if (spec1->numbered_arg_count + spec2->numbered_arg_count > 0)
    {
      unsigned int i, j;
      unsigned int n1 = spec1->numbered_arg_count;
      unsigned int n2 = spec2->numbered_arg_count;
      unsigned int missing = 0; /* only used if !equality */

      /* Check that the argument numbers are the same.
         Both arrays are sorted.  We search for the first difference.  */
      for (i = 0, j = 0; i < n1 || j < n2; )
        {
          int cmp = (i >= n1 ? 1 :
                     j >= n2 ? -1 :
                     spec1->numbered[i].number > spec2->numbered[j].number ? 1 :
                     spec1->numbered[i].number < spec2->numbered[j].number ? -1 :
                     0);

          if (cmp > 0)
            {
              if (error_logger)
                error_logger (_("a format specification for argument %u, as in '%s', doesn't exist in '%s'"),
                              spec2->numbered[j].number, pretty_msgstr,
                              pretty_msgid);
              err = true;
              break;
            }
          else if (cmp < 0)
            {
              if (equality)
                {
                  if (error_logger)
                    error_logger (_("a format specification for argument %u doesn't exist in '%s'"),
                                  spec1->numbered[i].number, pretty_msgstr);
                  err = true;
                  break;
                }
              else if (missing)
                {
                  if (error_logger)
                    error_logger (_("a format specification for arguments %u and %u doesn't exist in '%s', only one argument may be ignored"),
                                  missing, spec1->numbered[i].number,
                                  pretty_msgstr);
                  err = true;
                  break;
                }
              else
                {
                  missing = spec1->numbered[i].number;
                  i++;
                }
            }
          else
            j++, i++;
        }
    }

  return err;
}


struct formatstring_parser formatstring_kde =
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
  unsigned int last;
  unsigned int i;

  if (spec == NULL)
    {
      printf ("INVALID");
      return;
    }

  printf ("(");
  last = 1;
  for (i = 0; i < spec->numbered_arg_count; i++)
    {
      unsigned int number = spec->numbered[i].number;

      if (i > 0)
        printf (" ");
      if (number < last)
        abort ();
      for (; last < number; last++)
        printf ("_ ");
      last = number + 1;
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
 * compile-command: "/bin/sh ../libtool --tag=CC --mode=link gcc -o a.out -static -O -g -Wall -I.. -I../gnulib-lib -I../../gettext-runtime/intl -DHAVE_CONFIG_H -DTEST format-kde.c ../gnulib-lib/libgettextlib.la"
 * End:
 */

#endif /* TEST */
