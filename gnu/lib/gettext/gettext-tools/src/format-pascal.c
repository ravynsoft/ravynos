/* Object Pascal format strings.
   Copyright (C) 2001-2004, 2006-2007, 2009-2010, 2018-2020, 2023 Free Software Foundation, Inc.
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

/* Object Pascal format strings are usable with the "format" function in the
   "sysutils" unit.  They are described in
   <https://www.freepascal.org/docs-html/rtl/sysutils/format.html>
   and are implemented in fpc-2.4.0/rtl/objpas/sysutils/sysformt.inc.
   Another implementation exists in Borland Delphi.  The GNU Pascal's
   "sysutils" doesn't (yet?) have the "format" function.

   A directive
   - starts with '%',
   - either
     - is finished with '%', or
     - - is optionally followed by an index specification: '*' (reads an
         argument, must be of type integer) or a nonempty digit sequence
         or nothing (equivalent to 0), followed by ':',
       - is optionally followed by '-', which acts as a flag,
       - is optionally followed by a width specification: '*' (reads an
         argument, must be of type integer) or a nonempty digit sequence,
       - is optionally followed by '.' and a precision specification: '*'
         (reads an argument, must be of type integer) or a nonempty digit
         sequence,
       - is finished by a case-insensitive specifier. If no index was
         specified, it reads an argument; otherwise is uses the index-th
         argument, 0-based.
         - 'd', 'u', 'x', needs an 'integer' or 'int64' or 'qword' argument,
         - 'e', 'f', 'g', 'n', 'm', need an 'extended' or 'currency' floating-
           point argument,
         - 's', needs a 'string', 'char', 'pchar', 'widestring', 'widechar',
           'pwidechar' or 'ansistring' argument,
         - 'p', needs a 'pointer' argument.
   Numbered and unnumbered argument specifications can be used in the same
   string.  Numbered argument specifications have no influence on the
   "current argument index", that is incremented each time an argument is read.
 */

enum format_arg_type
{
  FAT_INTEGER,         /* integer, int64, qword */
  FAT_FLOAT,           /* extended, currency */
  FAT_STRING,          /* string, char, pchar, widestring, widechar, pwidechar,
                          ansistring */
  FAT_POINTER
};

struct numbered_arg
{
  unsigned int number;
  enum format_arg_type type;
};

struct spec
{
  unsigned int directives;
  unsigned int numbered_arg_count;
  struct numbered_arg *numbered;
};

/* Locale independent test for a decimal digit.
   Argument can be  'char' or 'unsigned char'.  (Whereas the argument of
   <ctype.h> isdigit must be an 'unsigned char'.)  */
#undef isdigit
#define isdigit(c) ((unsigned int) ((c) - '0') < 10)


static int
numbered_arg_compare (const void *p1, const void *p2)
{
  unsigned int n1 = ((const struct numbered_arg *) p1)->number;
  unsigned int n2 = ((const struct numbered_arg *) p2)->number;

  return (n1 > n2 ? 1 : n1 < n2 ? -1 : 0);
}

static void *
format_parse (const char *format, bool translated, char *fdi,
              char **invalid_reason)
{
  const char *const format_start = format;
  unsigned int directives;
  unsigned int numbered_arg_count;
  struct numbered_arg *numbered;
  unsigned int numbered_allocated;
  unsigned int unnumbered_arg_count;
  struct spec *result;

  enum arg_index
  {
    index_numbered,     /* index given by a fixed integer */
    index_unnumbered,   /* index given by unnumbered_arg_count++ */
    index_unknown       /* index is only known at run time */
  };

  directives = 0;
  numbered_arg_count = 0;
  numbered = NULL;
  numbered_allocated = 0;
  unnumbered_arg_count = 0;

  for (; *format != '\0';)
    if (*format++ == '%')
      {
        /* A directive.  */
        FDI_SET (format - 1, FMTDIR_START);
        directives++;

        if (*format != '%')
          {
            /* A complex directive.  */
            enum arg_index main_arg = index_unnumbered;
            unsigned int main_number = 0;
            enum format_arg_type type;

            if (isdigit (*format) || *format == ':')
              {
                const char *f = format;
                unsigned int m = 0;

                while (isdigit (*f))
                  {
                    m = 10 * m + (*f - '0');
                    f++;
                  }

                if (*f == ':')
                  {
                    main_number = m;
                    main_arg = index_numbered;
                    format = ++f;
                  }
              }
            else if (*format == '*')
              {
                if (format[1] == ':')
                  {
                    main_arg = index_unknown;
                    format += 2;
                  }
              }

            /* Parse flags.  */
            if (*format == '-')
              format++;

            /* Parse width.  */
            if (isdigit (*format))
              {
                do
                  format++;
                while (isdigit (*format));
              }
            else if (*format == '*')
              {
                /* Unnumbered argument of type FAT_INTEGER.   */
                if (numbered_allocated == numbered_arg_count)
                  {
                    numbered_allocated = 2 * numbered_allocated + 1;
                    numbered = (struct numbered_arg *) xrealloc (numbered, numbered_allocated * sizeof (struct numbered_arg));
                  }
                numbered[numbered_arg_count].number = unnumbered_arg_count;
                numbered[numbered_arg_count].type = FAT_INTEGER;
                numbered_arg_count++;
                unnumbered_arg_count++;

                format++;
              }

            /* Parse precision.  */
            if (*format == '.')
              {
                format++;

                if (isdigit (*format))
                  {
                    do
                      format++;
                    while (isdigit (*format));
                  }
                else if (*format == '*')
                  {
                    /* Unnumbered argument of type FAT_INTEGER.   */
                    if (numbered_allocated == unnumbered_arg_count)
                      {
                        numbered_allocated = 2 * numbered_allocated + 1;
                        numbered = (struct numbered_arg *) xrealloc (numbered, numbered_allocated * sizeof (struct numbered_arg));
                      }
                    numbered[numbered_arg_count].number = unnumbered_arg_count;
                    numbered[numbered_arg_count].type = FAT_INTEGER;
                    numbered_arg_count++;
                    unnumbered_arg_count++;

                    format++;
                  }
                else
                  --format;     /* will jump to bad_format */
              }

            switch (c_tolower (*format))
              {
              case 'd': case 'u': case 'x':
                type = FAT_INTEGER;
                break;
              case 'e': case 'f': case 'g': case 'n': case 'm':
                type = FAT_FLOAT;
                break;
              case 's':
                type = FAT_STRING;
                break;
              case 'p':
                type = FAT_POINTER;
                break;
              default:
                if (*format == '\0')
                  {
                    *invalid_reason = INVALID_UNTERMINATED_DIRECTIVE ();
                    FDI_SET (format - 1, FMTDIR_ERROR);
                  }
                else
                  {
                    *invalid_reason =
                      INVALID_CONVERSION_SPECIFIER (directives, *format);
                    FDI_SET (format, FMTDIR_ERROR);
                  }
                goto bad_format;
              }

            if (numbered_allocated == numbered_arg_count)
              {
                numbered_allocated = 2 * numbered_allocated + 1;
                numbered = (struct numbered_arg *) xrealloc (numbered, numbered_allocated * sizeof (struct numbered_arg));
              }
            switch (main_arg)
              {
              case index_unnumbered:
                numbered[numbered_arg_count].number = unnumbered_arg_count;
                numbered[numbered_arg_count].type = type;
                unnumbered_arg_count++;
                break;
              case index_numbered:
                numbered[numbered_arg_count].number = main_number;
                numbered[numbered_arg_count].type = type;
                break;
              case index_unknown:
                numbered[numbered_arg_count].number = unnumbered_arg_count;
                numbered[numbered_arg_count].type = FAT_INTEGER;
                unnumbered_arg_count++;
                break;
              default:
                abort ();
              }
            numbered_arg_count++;
          }

        FDI_SET (format, FMTDIR_END);

        format++;
      }

  /* Sort the numbered argument array, and eliminate duplicates.  */
  if (numbered_arg_count > 1)
    {
      unsigned int i, j;
      bool err;

      qsort (numbered, numbered_arg_count,
             sizeof (struct numbered_arg), numbered_arg_compare);

      /* Remove duplicates: Copy from i to j, keeping 0 <= j <= i.  */
      err = false;
      for (i = j = 0; i < numbered_arg_count; i++)
        if (j > 0 && numbered[i].number == numbered[j-1].number)
          {
            enum format_arg_type type1 = numbered[i].type;
            enum format_arg_type type2 = numbered[j-1].type;
            enum format_arg_type type_both;

            if (type1 == type2)
              type_both = type1;
            else
              {
                /* Incompatible types.  */
                type_both = type1;
                if (!err)
                  *invalid_reason =
                    INVALID_INCOMPATIBLE_ARG_TYPES (numbered[i].number);
                err = true;
              }

            numbered[j-1].type = type_both;
          }
        else
          {
            if (j < i)
              {
                numbered[j].number = numbered[i].number;
                numbered[j].type = numbered[i].type;
              }
            j++;
          }
      numbered_arg_count = j;
      if (err)
        /* *invalid_reason has already been set above.  */
        goto bad_format;
    }

  result = XMALLOC (struct spec);
  result->directives = directives;
  result->numbered_arg_count = numbered_arg_count;
  result->numbered = numbered;
  return result;

 bad_format:
  if (numbered != NULL)
    free (numbered);
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
              else
                i++;
            }
          else
            j++, i++;
        }
      /* Check the argument types are the same.  */
      if (!err)
        for (i = 0, j = 0; j < n2; )
          {
            if (spec1->numbered[i].number == spec2->numbered[j].number)
              {
                if (spec1->numbered[i].type != spec2->numbered[j].type)
                  {
                    if (error_logger)
                      error_logger (_("format specifications in '%s' and '%s' for argument %u are not the same"),
                                    pretty_msgid, pretty_msgstr,
                                    spec2->numbered[j].number);
                    err = true;
                    break;
                  }
                j++, i++;
              }
            else
              i++;
          }
    }

  return err;
}


struct formatstring_parser formatstring_pascal =
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
  last = 0;
  for (i = 0; i < spec->numbered_arg_count; i++)
    {
      unsigned int number = spec->numbered[i].number;

      if (i > 0)
        printf (" ");
      if (number < last)
        abort ();
      for (; last < number; last++)
        printf ("_ ");
      switch (spec->numbered[i].type)
        {
        case FAT_INTEGER:
          printf ("i");
          break;
        case FAT_FLOAT:
          printf ("f");
          break;
        case FAT_STRING:
          printf ("s");
          break;
        case FAT_POINTER:
          printf ("p");
          break;
        default:
          abort ();
        }
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
 * compile-command: "/bin/sh ../libtool --tag=CC --mode=link gcc -o a.out -static -O -g -Wall -I.. -I../gnulib-lib -I../../gettext-runtime/intl -DHAVE_CONFIG_H -DTEST format-pascal.c ../gnulib-lib/libgettextlib.la"
 * End:
 */

#endif /* TEST */
