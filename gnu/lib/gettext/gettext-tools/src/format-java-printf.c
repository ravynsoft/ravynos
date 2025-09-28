/* Java printf format strings.
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

/* Java printf format strings are described in java/util/Formatter.html.
   A directive
   - starts with '%' or '%<' or '%m$' where m is a positive integer,
   - is optionally followed by any of the characters '#', '0', '-', ' ', '+',
     ',', '(',
   - is optionally followed by a width specification: a nonempty digit sequence,
   - is optionally followed by '.' and a precision specification: a nonempty
     digit sequence,
   - is finished by a specifier
       - '%', 'n', that need no argument,
         Restrictions:
         - For '%': flags other than '-' are invalid, and a precision is
                    invalid.
         - For 'n': flags, width, and precision are invalid.
       - 'b', 'B', 'h', 'H', 's', 'S', that need a general argument.
         Restrictions:
         Flags other than '#' and '-' are invalid.
       - 'c', 'C', that need a character argument,
         Restrictions:
         Flags other than '-' are invalid.
         A precision is invalid.
       - 'd', 'o', 'x', 'X', that need an integer argument,
         Restrictions:
         - For 'd': The flag '#' is invalid.
         - For 'o', 'x', 'X': The flag ',' is invalid.
         A precision is invalid.
       - 'e', 'E', 'f', 'g', 'G', 'a', 'A', that need a floating-point argument,
         Restrictions:
         - For 'a', 'A': The flags ',', '(' are invalid.
       - 't', 'T', followed by one of
           'H', 'I', 'k', 'l', 'M', 'S', 'L', 'N', 'p', 'z', 'Z', 's', 'Q',
           'B', 'b', 'h', 'A', 'a', 'C', 'Y', 'y', 'j', 'm', 'd', 'e',
           'R', 'T', 'r', 'D', 'F', 'c'
         that need a date/time argument.
         Restrictions:
         Flags other than '-' are invalid.
         A precision is invalid.
   Numbered ('%m$') and unnumbered argument specifications can be mixed in the
   same string.  Numbered argument specifications have no influence on the
   unnumbered argument counter.
 */

enum format_arg_type
{
  FAT_NONE              = 0,
  /* Basic types */
  FAT_GENERAL           = 1,
  FAT_CHARACTER         = 2,
  FAT_INTEGER           = 3,
  FAT_FLOATINGPOINT     = 4,
  FAT_DATETIME          = 5
};
#ifdef __cplusplus
typedef int format_arg_type_t;
#else
typedef enum format_arg_type format_arg_type_t;
#endif

enum
{
  /* Flags */
  FAT_ALTERNATE         = 1 << 0, /* '#' */
  FAT_ZERO_PADDED       = 1 << 1, /* '0' */
  FAT_LEFT_JUSTIFIED    = 1 << 2, /* '-' */
  FAT_SPACE_SIGN        = 1 << 3, /* ' ' */
  FAT_SIGN              = 1 << 4, /* '+' */
  FAT_OBEY_LOCALE       = 1 << 5, /* ',' */
  FAT_MONETARY          = 1 << 6, /* '(' */
  /* Width */
  FAT_WIDTH             = 1 << 7,
  /* Precision */
  FAT_PRECISION         = 1 << 8,
};

struct numbered_arg
{
  unsigned int number;
  format_arg_type_t type;
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

#define INVALID_LAST_ARG(directive_number) \
  xasprintf (_("In the directive number %u, the reference to the argument of the previous directive is invalid."), directive_number)

#define INVALID_PRECISION_MISSING(directive_number) \
  xasprintf (_("In the directive number %u, the precision is missing."), directive_number)

#define INVALID_FLAG_FOR(directive_number,flag_char,conv_char) \
  xasprintf (_("In the directive number %u, the flag '%c' is invalid for the conversion '%c'."), directive_number, flag_char, conv_char)

#define INVALID_WIDTH_FOR(directive_number,conv_char) \
  xasprintf (_("In the directive number %u, a width is invalid for the conversion '%c'."), directive_number, conv_char)

#define INVALID_PRECISION_FOR(directive_number,conv_char) \
  xasprintf (_("In the directive number %u, a precision is invalid for the conversion '%c'."), directive_number, conv_char)

#define INVALID_DATETIME_CONVERSION_SUFFIX(directive_number,conv_char,suffix_char) \
  (c_isprint (conv_char) \
   ? xasprintf (_("In the directive number %u, for the conversion '%c', the character '%c' is not a valid conversion suffix."), directive_number, conv_char, suffix_char) \
   : xasprintf (_("The character that terminates the directive number %u, for the conversion '%c', is not a valid conversion suffix."), directive_number, conv_char))

static void *
format_parse (const char *format, bool translated, char *fdi,
              char **invalid_reason)
{
  const char *const format_start = format;
  struct spec spec;
  unsigned int numbered_allocated;
  struct spec *result;
  unsigned int unnumbered_arg_count;
  unsigned int last_arg_number;

  spec.directives = 0;
  spec.numbered_arg_count = 0;
  spec.numbered = NULL;
  numbered_allocated = 0;
  unnumbered_arg_count = 0;
  last_arg_number = 0;

  for (; *format != '\0';)
    if (*format++ == '%')
      {
        /* A directive.  */
        unsigned int number = 0;
        unsigned int flags;
        format_arg_type_t type;
        unsigned int invalid_flags;

        FDI_SET (format - 1, FMTDIR_START);
        spec.directives++;

        if (*format == '<')
          {
            if (last_arg_number == 0)
              {
                *invalid_reason = INVALID_LAST_ARG (spec.directives);
                FDI_SET (format, FMTDIR_ERROR);
                goto bad_format;
              }
            number = last_arg_number;
            format++;
          }
        else if (isdigit (*format))
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
                    *invalid_reason = INVALID_ARGNO_0 (spec.directives);
                    FDI_SET (f, FMTDIR_ERROR);
                    goto bad_format;
                  }
                number = m;
                format = ++f;
              }
          }

        flags = 0;

        /* Parse flags.  */
        for (;;)
          {
            if (*format == '#')
              {
                flags |= FAT_ALTERNATE;
                format++;
              }
            else if (*format == '0')
              {
                flags |= FAT_ZERO_PADDED;
                format++;
              }
            else if (*format == '-')
              {
                flags |= FAT_LEFT_JUSTIFIED;
                format++;
              }
            else if (*format == ' ')
              {
                flags |= FAT_SPACE_SIGN;
                format++;
              }
            else if (*format == '+')
              {
                flags |= FAT_SIGN;
                format++;
              }
            else if (*format == ',')
              {
                flags |= FAT_OBEY_LOCALE;
                format++;
              }
            else if (*format == '(')
              {
                flags |= FAT_MONETARY;
                format++;
              }
            else
              break;
          }

        /* Parse width.  */
        if (isdigit (*format))
          {
            do format++; while (isdigit (*format));
            flags |= FAT_WIDTH;
          }

        /* Parse precision.  */
        if (*format == '.')
          {
            format++;

            if (!isdigit (*format))
              {
                if (*format == '\0')
                  {
                    *invalid_reason = INVALID_UNTERMINATED_DIRECTIVE ();
                    FDI_SET (format - 1, FMTDIR_ERROR);
                  }
                else
                  {
                    *invalid_reason = INVALID_PRECISION_MISSING (spec.directives);
                    FDI_SET (format, FMTDIR_ERROR);
                  }
                goto bad_format;
              }

            do format++; while (isdigit (*format));
            flags |= FAT_PRECISION;
          }

        /* Parse conversion.  */
        switch (*format)
          {
          case '%':
            type = FAT_NONE;
            invalid_flags = (FAT_ALTERNATE | FAT_ZERO_PADDED | FAT_SPACE_SIGN
                             | FAT_SIGN | FAT_OBEY_LOCALE | FAT_MONETARY)
                            | FAT_PRECISION;
            break;
          case 'n':
            type = FAT_NONE;
            invalid_flags = (FAT_ALTERNATE | FAT_ZERO_PADDED | FAT_LEFT_JUSTIFIED
                             | FAT_SPACE_SIGN | FAT_SIGN | FAT_OBEY_LOCALE
                             | FAT_MONETARY)
                            | FAT_WIDTH | FAT_PRECISION;
            break;
          case 'b': case 'B':
          case 'h': case 'H':
          case 's': case 'S':
            type = FAT_GENERAL;
            invalid_flags = (FAT_ZERO_PADDED | FAT_SPACE_SIGN | FAT_SIGN
                             | FAT_OBEY_LOCALE | FAT_MONETARY);
            break;
          case 'c': case 'C':
            type = FAT_CHARACTER;
            invalid_flags = (FAT_ALTERNATE | FAT_ZERO_PADDED | FAT_SPACE_SIGN
                             | FAT_SIGN | FAT_OBEY_LOCALE | FAT_MONETARY)
                            | FAT_PRECISION;
            break;
          case 'd':
            type = FAT_INTEGER;
            invalid_flags = FAT_ALTERNATE | FAT_PRECISION;
            break;
          case 'o': case 'x': case 'X':
            type = FAT_INTEGER;
            invalid_flags = FAT_OBEY_LOCALE | FAT_PRECISION;
            break;
          case 'e': case 'E':
          case 'f':
          case 'g': case 'G':
            type = FAT_FLOATINGPOINT;
            invalid_flags = 0;
            break;
          case 'a': case 'A':
            type = FAT_FLOATINGPOINT;
            invalid_flags = FAT_OBEY_LOCALE | FAT_MONETARY;
            break;
          case 't': case 'T':
            type = FAT_DATETIME;
            invalid_flags = (FAT_ALTERNATE | FAT_ZERO_PADDED | FAT_SPACE_SIGN
                             | FAT_SIGN | FAT_OBEY_LOCALE | FAT_MONETARY)
                            | FAT_PRECISION;
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
                  INVALID_CONVERSION_SPECIFIER (spec.directives, *format);
                FDI_SET (format, FMTDIR_ERROR);
              }
            goto bad_format;
          }

        /* Report invalid flags, width, precision.  */
        invalid_flags &= flags;
        if (invalid_flags & FAT_ALTERNATE)
          {
            *invalid_reason = INVALID_FLAG_FOR (spec.directives, '#', *format);
            FDI_SET (format, FMTDIR_ERROR);
            goto bad_format;
          }
        if (invalid_flags & FAT_ZERO_PADDED)
          {
            *invalid_reason = INVALID_FLAG_FOR (spec.directives, '0', *format);
            FDI_SET (format, FMTDIR_ERROR);
            goto bad_format;
          }
        if (invalid_flags & FAT_LEFT_JUSTIFIED)
          {
            *invalid_reason = INVALID_FLAG_FOR (spec.directives, '-', *format);
            FDI_SET (format, FMTDIR_ERROR);
            goto bad_format;
          }
        if (invalid_flags & FAT_SPACE_SIGN)
          {
            *invalid_reason = INVALID_FLAG_FOR (spec.directives, ' ', *format);
            FDI_SET (format, FMTDIR_ERROR);
            goto bad_format;
          }
        if (invalid_flags & FAT_SIGN)
          {
            *invalid_reason = INVALID_FLAG_FOR (spec.directives, '+', *format);
            FDI_SET (format, FMTDIR_ERROR);
            goto bad_format;
          }
        if (invalid_flags & FAT_OBEY_LOCALE)
          {
            *invalid_reason = INVALID_FLAG_FOR (spec.directives, ',', *format);
            FDI_SET (format, FMTDIR_ERROR);
            goto bad_format;
          }
        if (invalid_flags & FAT_MONETARY)
          {
            *invalid_reason = INVALID_FLAG_FOR (spec.directives, '(', *format);
            FDI_SET (format, FMTDIR_ERROR);
            goto bad_format;
          }
        if (invalid_flags & FAT_WIDTH)
          {
            *invalid_reason = INVALID_WIDTH_FOR (spec.directives, *format);
            FDI_SET (format, FMTDIR_ERROR);
            goto bad_format;
          }
        if (invalid_flags & FAT_PRECISION)
          {
            *invalid_reason = INVALID_PRECISION_FOR (spec.directives, *format);
            FDI_SET (format, FMTDIR_ERROR);
            goto bad_format;
          }

        if (type == FAT_DATETIME)
          {
            format++;

            /* Parse conversion suffix.  */
            switch (*format)
              {
              case 'H': case 'I': case 'k': case 'l': case 'M': case 'S':
              case 'L': case 'N': case 'p': case 'z': case 'Z': case 's':
              case 'Q':
              case 'B': case 'b': case 'h': case 'A': case 'a': case 'C':
              case 'Y': case 'y': case 'j': case 'm': case 'd': case 'e':
              case 'R': case 'T': case 'r': case 'D': case 'F': case 'c':
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
                      INVALID_DATETIME_CONVERSION_SUFFIX (spec.directives,
                                                          format[-1], *format);
                    FDI_SET (format, FMTDIR_ERROR);
                  }
                goto bad_format;
              }
          }

        if (type != FAT_NONE)
          {
            if (number == 0)
              number = ++unnumbered_arg_count;

            if (numbered_allocated == spec.numbered_arg_count)
              {
                numbered_allocated = 2 * numbered_allocated + 1;
                spec.numbered = (struct numbered_arg *) xrealloc (spec.numbered, numbered_allocated * sizeof (struct numbered_arg));
              }
            spec.numbered[spec.numbered_arg_count].number = number;
            spec.numbered[spec.numbered_arg_count].type = type;
            spec.numbered_arg_count++;

            last_arg_number = number;
          }

        FDI_SET (format, FMTDIR_END);

        format++;
      }

  /* Sort the numbered argument array, and eliminate duplicates.  */
  if (spec.numbered_arg_count > 1)
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

            if (type1 == type2)
              type_both = type1;
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


struct formatstring_parser formatstring_java_printf =
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
      switch (spec->numbered[i].type)
        {
        case FAT_GENERAL:
          printf ("s");
          break;
        case FAT_CHARACTER:
          printf ("c");
          break;
        case FAT_INTEGER:
          printf ("d");
          break;
        case FAT_FLOATINGPOINT:
          printf ("f");
          break;
        case FAT_DATETIME:
          printf ("t");
          break;
        default:
          abort ();
        }
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
 * compile-command: "/bin/sh ../libtool --tag=CC --mode=link gcc -o a.out -static -O -g -Wall -I.. -I../gnulib-lib -I../../gettext-runtime/intl -DHAVE_CONFIG_H -DTEST format-java-printf.c ../gnulib-lib/libgettextlib.la"
 * End:
 */

#endif /* TEST */
