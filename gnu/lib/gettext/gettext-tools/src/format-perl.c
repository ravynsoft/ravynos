/* Perl format strings.
   Copyright (C) 2004, 2006-2007, 2009, 2019-2020, 2023 Free Software Foundation, Inc.
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
#include "format-invalid.h"
#include "gettext.h"

#define _(str) gettext (str)

/* Perl format strings are implemented in function Perl_sv_vcatpvfn in
   perl-5.8.0/sv.c.
   A directive
   - starts with '%' or '%m$' where m is a positive integer starting with a
     nonzero digit,
   - is optionally followed by any of the characters '#', '0', '-', ' ', '+',
     each of which acts as a flag,
   - is optionally followed by a vector specification: 'v' or '*v' (reads an
     argument) or '*m$v' where m is a positive integer starting with a nonzero
     digit,
   - is optionally followed by a width specification: '*' (reads an argument)
     or '*m$' where m is a positive integer starting with a nonzero digit or
     a nonempty digit sequence starting with a nonzero digit,
   - is optionally followed by '.' and a precision specification: '*' (reads
     an argument) or '*m$' where m is a positive integer starting with a
     nonzero digit or a digit sequence,
   - is optionally followed by a size specifier, one of 'h' 'l' 'll' 'L' 'q'
     'V' 'I32' 'I64' 'I',
   - is finished by a specifier
       - '%', that needs no argument,
       - 'c', that needs a small integer argument,
       - 's', that needs a string argument,
       - '_', that needs a scalar vector argument,
       - 'p', that needs a pointer argument,
       - 'i', 'd', 'D', that need an integer argument,
       - 'u', 'U', 'b', 'o', 'O', 'x', 'X', that need an unsigned integer
         argument,
       - 'e', 'E', 'f', 'F', 'g', 'G', that need a floating-point argument,
       - 'n', that needs a pointer to integer.
   So there can be numbered argument specifications:
   - '%m$' for the format string,
   - '*m$v' for the vector,
   - '*m$' for the width,
   - '.*m$' for the precision.
   Numbered and unnumbered argument specifications can be used in the same
   string. The effect of '%m$' is to take argument number m, without affecting
   the current argument number. The current argument number is incremented
   after processing a directive with an unnumbered argument specification.
 */

enum format_arg_type
{
  FAT_NONE              = 0,
  /* Basic types */
  FAT_INTEGER           = 1,
  FAT_DOUBLE            = 2,
  FAT_CHAR              = 3,
  FAT_STRING            = 4,
  FAT_SCALAR_VECTOR     = 5,
  FAT_POINTER           = 6,
  FAT_COUNT_POINTER     = 7,
  /* Flags */
  FAT_UNSIGNED          = 1 << 3,
  FAT_SIZE_SHORT        = 1 << 4,
  FAT_SIZE_V            = 2 << 4,
  FAT_SIZE_PTR          = 3 << 4,
  FAT_SIZE_LONG         = 4 << 4,
  FAT_SIZE_LONGLONG     = 5 << 4,
  /* Bitmasks */
  FAT_SIZE_MASK         = (FAT_SIZE_SHORT | FAT_SIZE_V | FAT_SIZE_PTR
                           | FAT_SIZE_LONG | FAT_SIZE_LONGLONG)
};
#ifdef __cplusplus
typedef int format_arg_type_t;
#else
typedef enum format_arg_type format_arg_type_t;
#endif

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

/* Locale independent test for a nonzero decimal digit.  */
#define isnonzerodigit(c) ((unsigned int) ((c) - '1') < 9)


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

  directives = 0;
  numbered_arg_count = 0;
  numbered = NULL;
  numbered_allocated = 0;
  unnumbered_arg_count = 0;

  for (; *format != '\0';)
    if (*format++ == '%')
      {
        /* A directive.  */
        unsigned int number = 0;
        bool vectorize = false;
        format_arg_type_t type;
        format_arg_type_t size;

        FDI_SET (format - 1, FMTDIR_START);
        directives++;

        if (isnonzerodigit (*format))
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
                number = m;
                format = ++f;
              }
          }

        /* Parse flags.  */
        while (*format == ' ' || *format == '+' || *format == '-'
               || *format == '#' || *format == '0')
          format++;

        /* Parse vector.  */
        if (*format == 'v')
          {
            format++;
            vectorize = true;
          }
        else if (*format == '*')
          {
            const char *f = format;

            f++;
            if (*f == 'v')
              {
                format = ++f;
                vectorize = true;

                /* Unnumbered argument.  */
                if (numbered_allocated == numbered_arg_count)
                  {
                    numbered_allocated = 2 * numbered_allocated + 1;
                    numbered = (struct numbered_arg *) xrealloc (numbered, numbered_allocated * sizeof (struct numbered_arg));
                  }
                numbered[numbered_arg_count].number = ++unnumbered_arg_count;
                numbered[numbered_arg_count].type = FAT_SCALAR_VECTOR; /* or FAT_STRING? */
                numbered_arg_count++;
              }
            else if (isnonzerodigit (*f))
              {
                unsigned int m = 0;

                do
                  {
                    m = 10 * m + (*f - '0');
                    f++;
                  }
                while (isdigit (*f));

                if (*f == '$')
                  {
                    f++;
                    if (*f == 'v')
                      {
                        unsigned int vector_number = m;

                        format = ++f;
                        vectorize = true;

                        /* Numbered argument.  */
                        /* Note: As of perl-5.8.0, this is not correctly
                           implemented in perl's sv.c.  */
                        if (numbered_allocated == numbered_arg_count)
                          {
                            numbered_allocated = 2 * numbered_allocated + 1;
                            numbered = (struct numbered_arg *) xrealloc (numbered, numbered_allocated * sizeof (struct numbered_arg));
                          }
                        numbered[numbered_arg_count].number = vector_number;
                        numbered[numbered_arg_count].type = FAT_SCALAR_VECTOR; /* or FAT_STRING? */
                        numbered_arg_count++;
                      }
                  }
              }
          }

        if (vectorize)
          {
            /* Numbered or unnumbered argument.  */
            if (numbered_allocated == numbered_arg_count)
              {
                numbered_allocated = 2 * numbered_allocated + 1;
                numbered = (struct numbered_arg *) xrealloc (numbered, numbered_allocated * sizeof (struct numbered_arg));
              }
            numbered[numbered_arg_count].number = (number ? number : ++unnumbered_arg_count);
            numbered[numbered_arg_count].type = FAT_SCALAR_VECTOR;
            numbered_arg_count++;
          }

        /* Parse width.  */
        if (*format == '*')
          {
            unsigned int width_number = 0;

            format++;

            if (isnonzerodigit (*format))
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
                    width_number = m;
                    format = ++f;
                  }
              }

            /* Numbered or unnumbered argument.  */
            /* Note: As of perl-5.8.0, this is not correctly
               implemented in perl's sv.c.  */
            if (numbered_allocated == numbered_arg_count)
              {
                numbered_allocated = 2 * numbered_allocated + 1;
                numbered = (struct numbered_arg *) xrealloc (numbered, numbered_allocated * sizeof (struct numbered_arg));
              }
            numbered[numbered_arg_count].number = (width_number ? width_number : ++unnumbered_arg_count);
            numbered[numbered_arg_count].type = FAT_INTEGER;
            numbered_arg_count++;
          }
        else if (isnonzerodigit (*format))
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

                if (isnonzerodigit (*format))
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
                        precision_number = m;
                        format = ++f;
                      }
                  }

                /* Numbered or unnumbered argument.  */
                if (numbered_allocated == numbered_arg_count)
                  {
                    numbered_allocated = 2 * numbered_allocated + 1;
                    numbered = (struct numbered_arg *) xrealloc (numbered, numbered_allocated * sizeof (struct numbered_arg));
                  }
                numbered[numbered_arg_count].number = (precision_number ? precision_number : ++unnumbered_arg_count);
                numbered[numbered_arg_count].type = FAT_INTEGER;
                numbered_arg_count++;
              }
            else
              {
                while (isdigit (*format)) format++;
              }
          }

        /* Parse size.  */
        size = 0;
        if (*format == 'h')
          {
            size = FAT_SIZE_SHORT;
            format++;
          }
        else if (*format == 'l')
          {
            if (format[1] == 'l')
              {
                size = FAT_SIZE_LONGLONG;
                format += 2;
              }
            else
              {
                size = FAT_SIZE_LONG;
                format++;
              }
          }
        else if (*format == 'L' || *format == 'q')
          {
            size = FAT_SIZE_LONGLONG;
            format++;
          }
        else if (*format == 'V')
          {
            size = FAT_SIZE_V;
            format++;
          }
        else if (*format == 'I')
          {
            if (format[1] == '6' && format[2] == '4')
              {
                size = FAT_SIZE_LONGLONG;
                format += 3;
              }
            else if (format[1] == '3' && format[2] == '2')
              {
                size = 0; /* FAT_SIZE_INT */
                format += 3;
              }
            else
              {
                size = FAT_SIZE_PTR;
                format++;
              }
          }

        switch (*format)
          {
          case '%':
            type = FAT_NONE;
            break;
          case 'c':
            type = FAT_CHAR;
            break;
          case 's':
            type = FAT_STRING;
            break;
          case '_':
            type = FAT_SCALAR_VECTOR;
            break;
          case 'D':
            type = FAT_INTEGER | FAT_SIZE_V;
            break;
          case 'i': case 'd':
            type = FAT_INTEGER | size;
            break;
          case 'U': case 'O':
            type = FAT_INTEGER | FAT_UNSIGNED | FAT_SIZE_V;
            break;
          case 'u': case 'b': case 'o': case 'x': case 'X':
            type = FAT_INTEGER | FAT_UNSIGNED | size;
            break;
          case 'e': case 'E': case 'f': case 'F': case 'g': case 'G':
            if (size == FAT_SIZE_SHORT || size == FAT_SIZE_LONG)
              {
                *invalid_reason =
                  xasprintf (_("In the directive number %u, the size specifier is incompatible with the conversion specifier '%c'."), directives, *format);
                FDI_SET (format, FMTDIR_ERROR);
                goto bad_format;
              }
            type = FAT_DOUBLE | size;
            break;
          case 'p':
            type = FAT_POINTER;
            break;
          case 'n':
            type = FAT_COUNT_POINTER | size;
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

        if (type != FAT_NONE && !vectorize)
          {
            /* Numbered or unnumbered argument.  */
            if (numbered_allocated == numbered_arg_count)
              {
                numbered_allocated = 2 * numbered_allocated + 1;
                numbered = (struct numbered_arg *) xrealloc (numbered, numbered_allocated * sizeof (struct numbered_arg));
              }
            numbered[numbered_arg_count].number = (number ? number : ++unnumbered_arg_count);
            numbered[numbered_arg_count].type = type;
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
            format_arg_type_t type1 = numbered[i].type;
            format_arg_type_t type2 = numbered[j-1].type;
            format_arg_type_t type_both;

            if (type1 == type2)
              type_both = type1;
            else
              {
                /* Incompatible types.  */
                type_both = FAT_NONE;
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


struct formatstring_parser formatstring_perl =
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
      if (spec->numbered[i].type & FAT_UNSIGNED)
        printf ("[unsigned]");
      switch (spec->numbered[i].type & FAT_SIZE_MASK)
        {
        case 0:
          break;
        case FAT_SIZE_SHORT:
          printf ("[short]");
          break;
        case FAT_SIZE_V:
          printf ("[IV]");
          break;
        case FAT_SIZE_PTR:
          printf ("[PTR]");
          break;
        case FAT_SIZE_LONG:
          printf ("[long]");
          break;
        case FAT_SIZE_LONGLONG:
          printf ("[long long]");
          break;
        default:
          abort ();
        }
      switch (spec->numbered[i].type & ~(FAT_UNSIGNED | FAT_SIZE_MASK))
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
        case FAT_STRING:
          printf ("s");
          break;
        case FAT_SCALAR_VECTOR:
          printf ("sv");
          break;
        case FAT_POINTER:
          printf ("p");
          break;
        case FAT_COUNT_POINTER:
          printf ("n");
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
 * compile-command: "/bin/sh ../libtool --tag=CC --mode=link gcc -o a.out -static -O -g -Wall -I.. -I../gnulib-lib -I../../gettext-runtime/intl -DHAVE_CONFIG_H -DTEST format-perl.c ../gnulib-lib/libgettextlib.la"
 * End:
 */

#endif /* TEST */
