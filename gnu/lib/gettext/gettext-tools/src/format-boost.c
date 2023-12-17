/* Boost format strings.
   Copyright (C) 2001-2004, 2006-2007, 2009, 2019-2020, 2023 Free Software Foundation, Inc.
   Written by Bruno Haible <haible@clisp.cons.org>, 2006.

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

#include "attribute.h"
#include "format.h"
#include "c-ctype.h"
#include "xalloc.h"
#include "xvasprintf.h"
#include "format-invalid.h"
#include "gettext.h"

#define _(str) gettext (str)

/* Boost format strings are described in
     boost_1_33_1/libs/format/doc/format.html
   and implemented in
     boost_1_33_1/boost/format/parsing.hpp.
   A directive (other than '%%')
   - starts with '%' or '%|'; in the latter case it must end in '|',
   - is continued either by
       - 'm%' where m is a positive integer, starting with a nonzero digit;
         in this case the directive must not have started with '%|'; or
       - the following:
           - optional: 'm$' where m is a positive integer, starting with a
             nonzero digit,
           - optional: any of the characters '#', '0', '-', ' ', '+', "'",
             '_', '=', 'h', 'l',
           - optional: a width specification: '*' (reads an argument) or '*m$'
             or a nonempty digit sequence,
           - optional: a '.' and a precision specification: '*' (reads an
             argument) or '*m$' or a nonempty digit sequence,
           - optional: any of the characters 'h', 'l', 'L',
           - if the directive started with '%|':
               an optional specifier and a final '|',
             otherwise
               a mandatory specifier.
             If no specifier is given, it needs an argument of any type.
             The possible specifiers are:
               - 'c', 'C', that need a character argument,
               - 's', 'S', that need an argument of any type,
               - 'i', 'd', 'o', 'u', 'x', 'X', that need an integer argument,
               - 'e', 'E', 'f', 'g', 'G', that need a floating-point argument,
               - 'p', that needs a 'void *' argument,
               - 't', that doesn't need an argument,
               - 'TX', where X is any character, that doesn't need an argument,
               - 'n', that needs a pointer to integer.
             The Boost format string interpreter doesn't actually care about
             the argument types, but we do, because it increases the likelihood
             of detecting translator mistakes.
   Numbered ('%m%' or '%m$' or '*m$') and unnumbered argument specifications
   cannot be used in the same string.
 */

enum format_arg_type
{
  FAT_NONE              = 0,
  /* Basic types */
  FAT_INTEGER           = 1,
  FAT_DOUBLE            = 2,
  FAT_CHAR              = 3,
  FAT_POINTER           = 4,
  FAT_ANY               = 5
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
  struct spec spec;
  unsigned int numbered_allocated;
  unsigned int unnumbered_arg_count;
  struct spec *result;

  spec.directives = 0;
  spec.numbered_arg_count = 0;
  spec.numbered = NULL;
  numbered_allocated = 0;
  unnumbered_arg_count = 0;

  for (; *format != '\0';)
    /* Invariant: spec.numbered_arg_count == 0 || unnumbered_arg_count == 0.  */
    if (*format++ == '%')
      {
        /* A directive.  */
        FDI_SET (format - 1, FMTDIR_START);
        spec.directives++;

        if (*format == '%')
          format++;
        else
          {
            bool brackets = false;
            bool done = false;
            unsigned int number = 0;
            enum format_arg_type type = FAT_NONE;

            if (*format == '|')
              {
                format++;
                brackets = true;
              }

            if (isdigit (*format) && *format != '0')
              {
                const char *f = format;
                unsigned int m = 0;

                do
                  {
                    m = 10 * m + (*f - '0');
                    f++;
                  }
                while (isdigit (*f));

                if ((!brackets && *f == '%') || *f == '$')
                  {
                    if (m == 0) /* can happen if m overflows */
                      {
                        *invalid_reason = INVALID_ARGNO_0 (spec.directives);
                        FDI_SET (f, FMTDIR_ERROR);
                        goto bad_format;
                      }
                    number = m;
                    if (*f == '%')
                      {
                        type = FAT_ANY;
                        done = true;
                      }
                    format = ++f;
                  }
              }

            if (!done)
              {
                /* Parse flags.  */
                for (;;)
                  {
                    if (*format == ' ' || *format == '+' || *format == '-'
                        || *format == '#' || *format == '0' || *format == '\''
                        || *format == '_' || *format == '=' || *format == 'h'
                        || *format == 'l')
                      format++;
                    else
                      break;
                  }

                /* Parse width.  */
                if (*format == '*')
                  {
                    unsigned int width_number = 0;

                    format++;

                    if (isdigit (*format))
                      {
                        const char *f = format;
                        unsigned int m = 0;

                        do
                          {
                            m = 10 * m + (*f - '0');
                            f++;
                          }
                        while (isdigit (*f));

                        if (*f == '$')
                          {
                            if (m == 0)
                              {
                                *invalid_reason =
                                  INVALID_WIDTH_ARGNO_0 (spec.directives);
                                FDI_SET (f, FMTDIR_ERROR);
                                goto bad_format;
                              }
                            width_number = m;
                            format = ++f;
                          }
                      }

                    if (width_number)
                      {
                        /* Numbered argument.  */

                        /* Numbered and unnumbered specifications are
                           exclusive.  */
                        if (unnumbered_arg_count > 0)
                          {
                            *invalid_reason =
                              INVALID_MIXES_NUMBERED_UNNUMBERED ();
                            FDI_SET (format - 1, FMTDIR_ERROR);
                            goto bad_format;
                          }

                        if (numbered_allocated == spec.numbered_arg_count)
                          {
                            numbered_allocated = 2 * numbered_allocated + 1;
                            spec.numbered = (struct numbered_arg *) xrealloc (spec.numbered, numbered_allocated * sizeof (struct numbered_arg));
                          }
                        spec.numbered[spec.numbered_arg_count].number = width_number;
                        spec.numbered[spec.numbered_arg_count].type = FAT_INTEGER;
                        spec.numbered_arg_count++;
                      }
                    else
                      {
                        /* Unnumbered argument.  */

                        /* Numbered and unnumbered specifications are
                           exclusive.  */
                        if (spec.numbered_arg_count > 0)
                          {
                            *invalid_reason =
                              INVALID_MIXES_NUMBERED_UNNUMBERED ();
                            FDI_SET (format - 1, FMTDIR_ERROR);
                            goto bad_format;
                          }

                        if (numbered_allocated == unnumbered_arg_count)
                          {
                            numbered_allocated = 2 * numbered_allocated + 1;
                            spec.numbered = (struct numbered_arg *) xrealloc (spec.numbered, numbered_allocated * sizeof (struct numbered_arg));
                          }
                        spec.numbered[unnumbered_arg_count].number = unnumbered_arg_count + 1;
                        spec.numbered[unnumbered_arg_count].type = FAT_INTEGER;
                        unnumbered_arg_count++;
                      }
                  }
                else if (isdigit (*format))
                  {
                    do format++; while (isdigit (*format));
                  }

                /* Parse precision.  */
                if (*format == '.')
                  {
                    format++;

                    if (*format == '*')
                      {
                        unsigned int precision_number = 0;

                        format++;

                        if (isdigit (*format))
                          {
                            const char *f = format;
                            unsigned int m = 0;

                            do
                              {
                                m = 10 * m + (*f - '0');
                                f++;
                              }
                            while (isdigit (*f));

                            if (*f == '$')
                              {
                                if (m == 0)
                                  {
                                    *invalid_reason =
                                      INVALID_PRECISION_ARGNO_0 (spec.directives);
                                    FDI_SET (f, FMTDIR_ERROR);
                                    goto bad_format;
                                  }
                                precision_number = m;
                                format = ++f;
                              }
                          }

                        if (precision_number)
                          {
                            /* Numbered argument.  */

                            /* Numbered and unnumbered specifications are
                               exclusive.  */
                            if (unnumbered_arg_count > 0)
                              {
                                *invalid_reason =
                                  INVALID_MIXES_NUMBERED_UNNUMBERED ();
                                FDI_SET (format - 1, FMTDIR_ERROR);
                                goto bad_format;
                              }

                            if (numbered_allocated == spec.numbered_arg_count)
                              {
                                numbered_allocated = 2 * numbered_allocated + 1;
                                spec.numbered = (struct numbered_arg *) xrealloc (spec.numbered, numbered_allocated * sizeof (struct numbered_arg));
                              }
                            spec.numbered[spec.numbered_arg_count].number = precision_number;
                            spec.numbered[spec.numbered_arg_count].type = FAT_INTEGER;
                            spec.numbered_arg_count++;
                          }
                        else
                          {
                            /* Unnumbered argument.  */

                            /* Numbered and unnumbered specifications are
                               exclusive.  */
                            if (spec.numbered_arg_count > 0)
                              {
                                *invalid_reason =
                                  INVALID_MIXES_NUMBERED_UNNUMBERED ();
                                FDI_SET (format - 1, FMTDIR_ERROR);
                                goto bad_format;
                              }

                            if (numbered_allocated == unnumbered_arg_count)
                              {
                                numbered_allocated = 2 * numbered_allocated + 1;
                                spec.numbered = (struct numbered_arg *) xrealloc (spec.numbered, numbered_allocated  * sizeof (struct numbered_arg));
                              }
                            spec.numbered[unnumbered_arg_count].number = unnumbered_arg_count + 1;
                            spec.numbered[unnumbered_arg_count].type = FAT_INTEGER;
                            unnumbered_arg_count++;
                          }
                      }
                    else if (isdigit (*format))
                      {
                        do format++; while (isdigit (*format));
                      }
                  }

                /* Parse size.  */
                for (;;)
                  {
                    if (*format == 'h' || *format == 'l' || *format == 'L')
                      format++;
                    else
                      break;
                  }

                switch (*format++)
                  {
                  case 'c': case 'C':
                    type = FAT_CHAR;
                    break;
                  case 's': case 'S':
                    type = FAT_ANY;
                    break;
                  case 'i': case 'd': case 'o': case 'u': case 'x': case 'X':
                    type = FAT_INTEGER;
                    break;
                  case 'e': case 'E': case 'f': case 'g': case 'G':
                    type = FAT_DOUBLE;
                    break;
                  case 'p':
                    type = FAT_POINTER;
                    break;
                  case 't':
                    type = FAT_NONE;
                    break;
                  case 'T':
                    if (*format == '\0')
                      {
                        *invalid_reason = INVALID_UNTERMINATED_DIRECTIVE ();
                        FDI_SET (format - 1, FMTDIR_ERROR);
                        goto bad_format;
                      }
                    format++;
                    type = FAT_NONE;
                    break;
                  case 'n':
                    type = FAT_NONE;
                    break;
                  case '|':
                    if (brackets)
                      {
                        --format;
                        type = FAT_ANY;
                        break;
                      }
                    FALLTHROUGH;
                  default:
                    --format;
                    if (*format == '\0')
                      {
                        *invalid_reason = INVALID_UNTERMINATED_DIRECTIVE ();
                        FDI_SET (format - 1, FMTDIR_ERROR);
                      }
                    else
                      {
                        *invalid_reason =
                          INVALID_CONVERSION_SPECIFIER (spec.directives,
                                                        *format);
                        FDI_SET (format, FMTDIR_ERROR);
                      }
                    goto bad_format;
                  }
                if (brackets)
                  {
                    if (*format != '|')
                      {
                        if (*format == '\0')
                          {
                            *invalid_reason = INVALID_UNTERMINATED_DIRECTIVE ();
                            FDI_SET (format - 1, FMTDIR_ERROR);
                          }
                        else
                          {
                            *invalid_reason =
                              xasprintf (_("The directive number %u starts with | but does not end with |."),
                                         spec.directives);
                            FDI_SET (format, FMTDIR_ERROR);
                          }
                        goto bad_format;
                      }
                    format++;
                  }
              }

            if (type != FAT_NONE)
              {
                if (number)
                  {
                    /* Numbered argument.  */

                    /* Numbered and unnumbered specifications are exclusive.  */
                    if (unnumbered_arg_count > 0)
                      {
                        *invalid_reason = INVALID_MIXES_NUMBERED_UNNUMBERED ();
                        FDI_SET (format - 1, FMTDIR_ERROR);
                        goto bad_format;
                      }

                    if (numbered_allocated == spec.numbered_arg_count)
                      {
                        numbered_allocated = 2 * numbered_allocated + 1;
                        spec.numbered = (struct numbered_arg *) xrealloc (spec.numbered, numbered_allocated * sizeof (struct numbered_arg));
                      }
                    spec.numbered[spec.numbered_arg_count].number = number;
                    spec.numbered[spec.numbered_arg_count].type = type;
                    spec.numbered_arg_count++;
                  }
                else
                  {
                    /* Unnumbered argument.  */

                    /* Numbered and unnumbered specifications are exclusive.  */
                    if (spec.numbered_arg_count > 0)
                      {
                        *invalid_reason = INVALID_MIXES_NUMBERED_UNNUMBERED ();
                        FDI_SET (format - 1, FMTDIR_ERROR);
                        goto bad_format;
                      }

                    if (numbered_allocated == unnumbered_arg_count)
                      {
                        numbered_allocated = 2 * numbered_allocated + 1;
                        spec.numbered = (struct numbered_arg *) xrealloc (spec.numbered, numbered_allocated * sizeof (struct numbered_arg));
                      }
                    spec.numbered[unnumbered_arg_count].number = unnumbered_arg_count + 1;
                    spec.numbered[unnumbered_arg_count].type = type;
                    unnumbered_arg_count++;
                  }
              }
          }

        FDI_SET (format - 1, FMTDIR_END);
      }

  /* Convert the unnumbered argument array to numbered arguments.  */
  if (unnumbered_arg_count > 0)
    spec.numbered_arg_count = unnumbered_arg_count;
  /* Sort the numbered argument array, and eliminate duplicates.  */
  else if (spec.numbered_arg_count > 1)
    {
      unsigned int i, j;
      bool err;

      qsort (spec.numbered, spec.numbered_arg_count,
             sizeof (struct numbered_arg), numbered_arg_compare);

      /* Remove duplicates: Copy from i to j, keeping 0 <= j <= i.  */
      err = false;
      for (i = j = 0; i < spec.numbered_arg_count; i++)
        if (j > 0 && spec.numbered[i].number == spec.numbered[j-1].number)
          {
            enum format_arg_type type1 = spec.numbered[i].type;
            enum format_arg_type type2 = spec.numbered[j-1].type;
            enum format_arg_type type_both;

            if (type1 == type2 || type2 == FAT_ANY)
              type_both = type1;
            else if (type1 == FAT_ANY)
              type_both = type2;
            else
              {
                /* Incompatible types.  */
                type_both = FAT_NONE;
                if (!err)
                  *invalid_reason =
                    INVALID_INCOMPATIBLE_ARG_TYPES (spec.numbered[i].number);
                err = true;
              }

            spec.numbered[j-1].type = type_both;
          }
        else
          {
            if (j < i)
              {
                spec.numbered[j].number = spec.numbered[i].number;
                spec.numbered[j].type = spec.numbered[i].type;
              }
            j++;
          }
      spec.numbered_arg_count = j;
      if (err)
        /* *invalid_reason has already been set above.  */
        goto bad_format;
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


struct formatstring_parser formatstring_boost =
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
      switch (spec->numbered[i].type)
        {
        case FAT_INTEGER:
          printf ("i");
          break;
        case FAT_DOUBLE:
          printf ("f");
          break;
        case FAT_CHAR:
          printf ("c");
          break;
        case FAT_POINTER:
          printf ("p");
          break;
        case FAT_ANY:
          printf ("*");
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
 * compile-command: "/bin/sh ../libtool --tag=CC --mode=link gcc -o a.out -static -O -g -Wall -I.. -I../gnulib-lib -I../../gettext-runtime/intl -DHAVE_CONFIG_H -DTEST format-boost.c ../gnulib-lib/libgettextlib.la"
 * End:
 */

#endif /* TEST */

