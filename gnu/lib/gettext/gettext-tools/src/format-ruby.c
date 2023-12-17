/* Ruby format strings.
   Copyright (C) 2001-2004, 2006-2009, 2019-2020, 2023 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2020.

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

#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "format.h"
#include "c-ctype.h"
#include "xalloc.h"
#include "xvasprintf.h"
#include "format-invalid.h"
#include "gettext.h"

#define _(str) gettext (str)

/* Ruby format strings are described in
   https://ruby-doc.org/core-2.7.1/Kernel.html#method-i-sprintf
   and are implemented in ruby-2.7.1/sprintf.c .
   A format string consists of literal text and directives.
   A directive
   - starts with '%',
   - is optionally followed by a sequence of the following:
     - any of the characters ' ', '#', '+', '-', '0', each of which acts as a
       flag,
     - a digit sequence starting with a non-zero digit, followed by '$', at
       most once per directive, indicating a positional argument to consume,
     - '<' KEY '>', at most once per directive, indicating a hash table element
       to consume,
     - a digit sequence starting with a non-zero digit, specifying a width,
     - '*', indicating a width, taken from the argument list,
     - '*' and a digit sequence, followed by '$', indicating a width, taken
       from a positional argument,
     - '.' and a digit sequence, indicating a precision,
     - '.' '*', indicating a precision, taken from the argument list,
     - '.' '*' and a digit sequence, followed by '$', indicating a precision,
       taken from a positional argument.
     This sequence is in any order, except that
       - flags must occur before width and precision,
       - width must occur before precision.
   - is finished by a specifier
     - 's', that takes an object to print without double-quote delimiters,
     - 'p', that takes an object to print with double-quote delimiters (in case
       of a string),
     - '{' KEY '}', indicating a hash table element to consume and to print
       like with 's',
     - 'c', that takes a character,
     - 'd', 'i', 'u', 'o', 'x', 'X', 'b', 'B', that take an integer,
     - 'f', 'g', 'G', 'e', 'E', 'a', 'A', that take a floating-point number.
   Additionally there are the directives '%%' '%<newline>', which take no
   argument.
   Numbered, unnumbered, and named argument specifications cannot be used in
   the same string; either all arguments are numbered, or all arguments are
   unnumbered, or all arguments are named.
 */

enum format_arg_type
{
  FAT_NONE,
  FAT_ANY,
  FAT_ESCAPED_ANY,
  FAT_CHARACTER,
  FAT_INTEGER,
  FAT_FLOAT
};

struct named_arg
{
  char *name;
  enum format_arg_type type;
};

struct numbered_arg
{
  unsigned int number;
  enum format_arg_type type;
};

struct spec
{
  unsigned int directives;
  unsigned int named_arg_count;
  unsigned int numbered_arg_count;
  struct named_arg *named;
  struct numbered_arg *numbered;
};

/* Locale independent test for a decimal digit.
   Argument can be  'char' or 'unsigned char'.  (Whereas the argument of
   <ctype.h> isdigit must be an 'unsigned char'.)  */
#undef isdigit
#define isdigit(c) ((unsigned int) ((c) - '0') < 10)


static int
named_arg_compare (const void *p1, const void *p2)
{
  return strcmp (((const struct named_arg *) p1)->name,
                 ((const struct named_arg *) p2)->name);
}

static int
numbered_arg_compare (const void *p1, const void *p2)
{
  unsigned int n1 = ((const struct numbered_arg *) p1)->number;
  unsigned int n2 = ((const struct numbered_arg *) p2)->number;

  return (n1 > n2 ? 1 : n1 < n2 ? -1 : 0);
}

#define INVALID_MIXES_NAMED_UNNAMED() \
  xstrdup (_("The string refers to arguments both through argument names and through unnamed argument specifications."))

#define INVALID_TWO_ARG_NAMES(directive_number) \
  xasprintf (_("In the directive number %u, two names are given for the same argument."), directive_number)

#define INVALID_TWO_ARG_NUMBERS(directive_number) \
  xasprintf (_("In the directive number %u, two numbers are given for the same argument."), directive_number)

#define INVALID_FLAG_AFTER_WIDTH(directive_number) \
  xasprintf (_("In the directive number %u, a flag is given after the width."), directive_number)

#define INVALID_FLAG_AFTER_PRECISION(directive_number) \
  xasprintf (_("In the directive number %u, a flag is given after the precision."), directive_number)

#define INVALID_WIDTH_AFTER_PRECISION(directive_number) \
  xasprintf (_("In the directive number %u, the width is given after the precision."), directive_number)

#define INVALID_WIDTH_TWICE(directive_number) \
  xasprintf (_("In the directive number %u, a width is given twice."), directive_number)

#define INVALID_PRECISION_TWICE(directive_number) \
  xasprintf (_("In the directive number %u, a precision is given twice."), directive_number)

static void *
format_parse (const char *format, bool translated, char *fdi,
              char **invalid_reason)
{
  const char *const format_start = format;
  struct spec spec;
  unsigned int unnumbered_arg_count;
  unsigned int allocated;
  struct spec *result;

  spec.directives = 0;
  spec.named_arg_count = 0;
  spec.numbered_arg_count = 0;
  spec.named = NULL;
  spec.numbered = NULL;
  unnumbered_arg_count = 0;
  allocated = 0;

  for (; *format != '\0';)
    /* Invariant: spec.numbered_arg_count == 0 || unnumbered_arg_count == 0.  */
    if (*format++ == '%')
      {
        /* A directive.  */
        char *name = NULL;
        unsigned int number = 0;

        bool seen_width = false;
        unsigned int width_number = 0;
        bool width_takenext = false;

        bool seen_precision = false;
        unsigned int precision_number = 0;
        bool precision_takenext = false;

        enum format_arg_type type;

        FDI_SET (format - 1, FMTDIR_START);
        spec.directives++;

        for (;;)
          {
            if (*format == ' '
                || *format == '#'
                || *format == '+'
                || *format == '-'
                || *format == '0')
              {
                /* A flag.  */
                if (seen_width)
                  {
                    *invalid_reason = INVALID_FLAG_AFTER_WIDTH (spec.directives);
                    FDI_SET (format, FMTDIR_ERROR);
                    goto bad_format;
                  }
                if (seen_precision)
                  {
                    *invalid_reason = INVALID_FLAG_AFTER_PRECISION (spec.directives);
                    FDI_SET (format, FMTDIR_ERROR);
                    goto bad_format;
                  }
                format++;
                continue;
              }

            if (*format == '<')
              {
                const char *name_start;
                const char *name_end;
                size_t n;

                if ((spec.numbered_arg_count > 0
                     || number > 0 || width_number > 0 || precision_number > 0)
                    || (unnumbered_arg_count > 0
                        || width_takenext || precision_takenext))
                  {
                    *invalid_reason = INVALID_MIXES_NAMED_UNNAMED ();
                    FDI_SET (format, FMTDIR_ERROR);
                    goto bad_format;
                  }
                if (name != NULL)
                  {
                    *invalid_reason = INVALID_TWO_ARG_NAMES (spec.directives);
                    FDI_SET (format, FMTDIR_ERROR);
                    goto bad_format;
                  }

                name_start = ++format;
                for (; *format != '\0'; format++)
                  if (*format == '>')
                    break;
                if (*format == '\0')
                  {
                    *invalid_reason = INVALID_UNTERMINATED_DIRECTIVE ();
                    FDI_SET (format - 1, FMTDIR_ERROR);
                    goto bad_format;
                  }
                name_end = format++;

                n = name_end - name_start;
                name = XNMALLOC (n + 1, char);
                memcpy (name, name_start, n);
                name[n] = '\0';

                continue;
              }

            if (isdigit (*format))
              {
                unsigned int m = 0;

                do
                  {
                    if (m < UINT_MAX / 10)
                      m = 10 * m + (*format - '0');
                    else
                      m = UINT_MAX - 1;
                    format++;
                  }
                while (isdigit (*format));

                if (*format == '$')
                  {
                    if (spec.named_arg_count > 0 || name != NULL)
                      {
                        *invalid_reason = INVALID_MIXES_NAMED_UNNAMED ();
                        FDI_SET (format, FMTDIR_ERROR);
                        goto bad_format;
                      }
                    if (unnumbered_arg_count > 0
                        || width_takenext || precision_takenext)
                      {
                        *invalid_reason = INVALID_MIXES_NUMBERED_UNNUMBERED ();
                        FDI_SET (format, FMTDIR_ERROR);
                        goto bad_format;
                      }
                    if (number > 0)
                      {
                        *invalid_reason = INVALID_TWO_ARG_NUMBERS (spec.directives);
                        FDI_SET (format, FMTDIR_ERROR);
                        goto bad_format;
                      }
                    number = m;
                    format++;
                  }
                else
                  {
                    /* Seen a constant width.  */
                    if (seen_precision)
                      {
                        *invalid_reason = INVALID_WIDTH_AFTER_PRECISION (spec.directives);
                        FDI_SET (format, FMTDIR_ERROR);
                        goto bad_format;
                      }
                    if (seen_width)
                      {
                        *invalid_reason = INVALID_WIDTH_TWICE (spec.directives);
                        FDI_SET (format, FMTDIR_ERROR);
                        goto bad_format;
                      }
                    seen_width = true;
                  }
                continue;
              }

            if (*format == '*')
              {
                /* Parse width.  */
                format++;

                if (isdigit (*format))
                  {
                    const char *f = format;
                    unsigned int m = 0;

                    do
                      {
                        if (m < UINT_MAX / 10)
                          m = 10 * m + (*f - '0');
                        else
                          m = UINT_MAX - 1;
                        f++;
                      }
                    while (isdigit (*f));

                    if (*f == '$')
                      {
                        format = f;
                        if (spec.named_arg_count > 0 || name != NULL)
                          {
                            *invalid_reason = INVALID_MIXES_NAMED_UNNAMED ();
                            FDI_SET (format, FMTDIR_ERROR);
                            goto bad_format;
                          }
                        if (unnumbered_arg_count > 0
                            || width_takenext || precision_takenext)
                          {
                            *invalid_reason = INVALID_MIXES_NUMBERED_UNNUMBERED ();
                            FDI_SET (format, FMTDIR_ERROR);
                            goto bad_format;
                          }
                        if (seen_precision)
                          {
                            *invalid_reason = INVALID_WIDTH_AFTER_PRECISION (spec.directives);
                            FDI_SET (format, FMTDIR_ERROR);
                            goto bad_format;
                          }
                        if (seen_width)
                          {
                            *invalid_reason = INVALID_WIDTH_TWICE (spec.directives);
                            FDI_SET (format, FMTDIR_ERROR);
                            goto bad_format;
                          }
                        if (m == 0)
                          {
                            *invalid_reason = INVALID_ARGNO_0 (spec.directives);
                            FDI_SET (format, FMTDIR_ERROR);
                            goto bad_format;
                          }
                        seen_width = true;
                        width_number = m;
                        format++;
                        continue;
                      }
                  }

                if (spec.named_arg_count > 0 || name != NULL)
                  {
                    *invalid_reason = INVALID_MIXES_NAMED_UNNAMED ();
                    FDI_SET (format - 1, FMTDIR_ERROR);
                    goto bad_format;
                  }
                if (spec.numbered_arg_count > 0
                    || number > 0 || width_number > 0 || precision_number > 0)
                  {
                    *invalid_reason = INVALID_MIXES_NUMBERED_UNNUMBERED ();
                    FDI_SET (format - 1, FMTDIR_ERROR);
                    goto bad_format;
                  }
                if (seen_precision)
                  {
                    *invalid_reason = INVALID_WIDTH_AFTER_PRECISION (spec.directives);
                    FDI_SET (format - 1, FMTDIR_ERROR);
                    goto bad_format;
                  }
                if (seen_width)
                  {
                    *invalid_reason = INVALID_WIDTH_TWICE (spec.directives);
                    FDI_SET (format - 1, FMTDIR_ERROR);
                    goto bad_format;
                  }
                seen_width = true;
                width_takenext = true;
                continue;
              }

            if (*format == '.')
              {
                /* Parse precision.  */
                format++;

                if (*format == '*')
                  {
                    format++;

                    if (isdigit (*format))
                      {
                        const char *f = format;
                        unsigned int m = 0;

                        do
                          {
                            if (m < UINT_MAX / 10)
                              m = 10 * m + (*f - '0');
                            else
                              m = UINT_MAX - 1;
                            f++;
                          }
                        while (isdigit (*f));

                        if (*f == '$')
                          {
                            format = f;
                            if (spec.named_arg_count > 0 || name != NULL)
                              {
                                *invalid_reason = INVALID_MIXES_NAMED_UNNAMED ();
                                FDI_SET (format, FMTDIR_ERROR);
                                goto bad_format;
                              }
                            if (unnumbered_arg_count > 0
                                || width_takenext || precision_takenext)
                              {
                                *invalid_reason = INVALID_MIXES_NUMBERED_UNNUMBERED ();
                                FDI_SET (format, FMTDIR_ERROR);
                                goto bad_format;
                              }
                            if (seen_precision)
                              {
                                *invalid_reason = INVALID_PRECISION_TWICE (spec.directives);
                                FDI_SET (format, FMTDIR_ERROR);
                                goto bad_format;
                              }
                            if (m == 0)
                              {
                                *invalid_reason = INVALID_ARGNO_0 (spec.directives);
                                FDI_SET (format, FMTDIR_ERROR);
                                goto bad_format;
                              }
                            seen_precision = true;
                            precision_number = m;
                            format++;
                            continue;
                          }
                      }

                    if (spec.named_arg_count > 0 || name != NULL)
                      {
                        *invalid_reason = INVALID_MIXES_NAMED_UNNAMED ();
                        FDI_SET (format - 1, FMTDIR_ERROR);
                        goto bad_format;
                      }
                    if (spec.numbered_arg_count > 0
                        || number > 0 || width_number > 0 || precision_number > 0)
                      {
                        *invalid_reason = INVALID_MIXES_NUMBERED_UNNUMBERED ();
                        FDI_SET (format - 1, FMTDIR_ERROR);
                        goto bad_format;
                      }
                    if (seen_precision)
                      {
                        *invalid_reason = INVALID_PRECISION_TWICE (spec.directives);
                        FDI_SET (format - 1, FMTDIR_ERROR);
                        goto bad_format;
                      }
                    seen_precision = true;
                    precision_takenext = true;
                    continue;
                  }

                while (isdigit (*format))
                  format++;

                /* Seen a constant precision.  */
                if (seen_precision)
                  {
                    *invalid_reason = INVALID_PRECISION_TWICE (spec.directives);
                    FDI_SET (format, FMTDIR_ERROR);
                    goto bad_format;
                  }
                seen_precision = true;
                continue;
              }

            break;
          }

        switch (*format)
          {
          case '%':
          case '\n':
            type = FAT_NONE;
            break;
          case 's':
            type = FAT_ANY;
            break;
          case 'p':
            type = FAT_ESCAPED_ANY;
            break;
          case 'c':
            type = FAT_CHARACTER;
            break;
          case 'd':
          case 'i':
          case 'u':
          case 'o':
          case 'x':
          case 'X':
          case 'b':
          case 'B':
            type = FAT_INTEGER;
            break;
          case 'f':
          case 'g':
          case 'G':
          case 'e':
          case 'E':
          case 'a':
          case 'A':
            type = FAT_FLOAT;
            break;
          case '{':
            {
              const char *name_start;
              const char *name_end;
              size_t n;

              if ((spec.numbered_arg_count > 0
                   || number > 0 || width_number > 0 || precision_number > 0)
                  || (unnumbered_arg_count > 0
                      || width_takenext || precision_takenext))
                {
                  *invalid_reason = INVALID_MIXES_NAMED_UNNAMED ();
                  FDI_SET (format, FMTDIR_ERROR);
                  goto bad_format;
                }
              if (name != NULL)
                {
                  *invalid_reason = INVALID_TWO_ARG_NAMES (spec.directives);
                  FDI_SET (format, FMTDIR_ERROR);
                  goto bad_format;
                }

              name_start = ++format;
              for (; *format != '\0'; format++)
                if (*format == '}')
                  break;
              if (*format == '\0')
                {
                  *invalid_reason = INVALID_UNTERMINATED_DIRECTIVE ();
                  FDI_SET (format - 1, FMTDIR_ERROR);
                  goto bad_format;
                }
              name_end = format;

              n = name_end - name_start;
              name = XNMALLOC (n + 1, char);
              memcpy (name, name_start, n);
              name[n] = '\0';
            }
            type = FAT_ANY;
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

        if (seen_width)
          {
            /* Register the argument specification for the width.  */
            if (width_number > 0)
              {
                if (allocated == spec.numbered_arg_count)
                  {
                    allocated = 2 * allocated + 1;
                    spec.numbered = (struct numbered_arg *) xrealloc (spec.numbered, allocated * sizeof (struct numbered_arg));
                  }
                spec.numbered[spec.numbered_arg_count].number = width_number;
                spec.numbered[spec.numbered_arg_count].type = FAT_INTEGER;
                spec.numbered_arg_count++;
              }
            else if (width_takenext)
              {
                if (allocated == unnumbered_arg_count)
                  {
                    allocated = 2 * allocated + 1;
                    spec.numbered = (struct numbered_arg *) xrealloc (spec.numbered, allocated * sizeof (struct numbered_arg));
                  }
                spec.numbered[unnumbered_arg_count].number = unnumbered_arg_count + 1;
                spec.numbered[unnumbered_arg_count].type = FAT_INTEGER;
                unnumbered_arg_count++;
              }
          }

        if (seen_precision)
          {
            /* Register the argument specification for the precision.  */
            if (precision_number > 0)
              {
                if (allocated == spec.numbered_arg_count)
                  {
                    allocated = 2 * allocated + 1;
                    spec.numbered = (struct numbered_arg *) xrealloc (spec.numbered, allocated * sizeof (struct numbered_arg));
                  }
                spec.numbered[spec.numbered_arg_count].number = precision_number;
                spec.numbered[spec.numbered_arg_count].type = FAT_INTEGER;
                spec.numbered_arg_count++;
              }
            else if (precision_takenext)
              {
                if (allocated == unnumbered_arg_count)
                  {
                    allocated = 2 * allocated + 1;
                    spec.numbered = (struct numbered_arg *) xrealloc (spec.numbered, allocated * sizeof (struct numbered_arg));
                  }
                spec.numbered[unnumbered_arg_count].number = unnumbered_arg_count + 1;
                spec.numbered[unnumbered_arg_count].type = FAT_INTEGER;
                unnumbered_arg_count++;
              }
          }

        if (type != FAT_NONE)
          {
            /* Register the argument specification for the value.  */
            if (name != NULL)
              {
                if (allocated == spec.named_arg_count)
                  {
                    allocated = 2 * allocated + 1;
                    spec.named = (struct named_arg *) xrealloc (spec.named, allocated * sizeof (struct named_arg));
                  }
                spec.named[spec.named_arg_count].name = name;
                spec.named[spec.named_arg_count].type = type;
                spec.named_arg_count++;
              }
            else if (number > 0)
              {
                if (allocated == spec.numbered_arg_count)
                  {
                    allocated = 2 * allocated + 1;
                    spec.numbered = (struct numbered_arg *) xrealloc (spec.numbered, allocated * sizeof (struct numbered_arg));
                  }
                spec.numbered[spec.numbered_arg_count].number = number;
                spec.numbered[spec.numbered_arg_count].type = type;
                spec.numbered_arg_count++;
              }
            else
              {
                if (spec.named_arg_count > 0)
                  {
                    *invalid_reason = INVALID_MIXES_NAMED_UNNAMED ();
                    FDI_SET (format, FMTDIR_ERROR);
                    goto bad_format;
                  }
                if (spec.numbered_arg_count > 0)
                  {
                    *invalid_reason = INVALID_MIXES_NUMBERED_UNNUMBERED ();
                    FDI_SET (format, FMTDIR_ERROR);
                    goto bad_format;
                  }
                if (allocated == unnumbered_arg_count)
                  {
                    allocated = 2 * allocated + 1;
                    spec.numbered = (struct numbered_arg *) xrealloc (spec.numbered, allocated * sizeof (struct numbered_arg));
                  }
                spec.numbered[unnumbered_arg_count].number = unnumbered_arg_count + 1;
                spec.numbered[unnumbered_arg_count].type = type;
                unnumbered_arg_count++;
              }
          }

        FDI_SET (format, FMTDIR_END);

        format++;
      }

  /* Verify that either all arguments are numbered, or all arguments are
     unnumbered, or all arguments are named.  */
  if ((spec.numbered_arg_count > 0)
      + (unnumbered_arg_count > 0)
      + (spec.named_arg_count > 0)
      > 1)
    abort ();

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

  /* Sort the named argument array, and eliminate duplicates.  */
  if (spec.named_arg_count > 1)
    {
      unsigned int i, j;
      bool err;

      qsort (spec.named, spec.named_arg_count, sizeof (struct named_arg),
             named_arg_compare);

      /* Remove duplicates: Copy from i to j, keeping 0 <= j <= i.  */
      err = false;
      for (i = j = 0; i < spec.named_arg_count; i++)
        if (j > 0 && strcmp (spec.named[i].name, spec.named[j-1].name) == 0)
          {
            enum format_arg_type type1 = spec.named[i].type;
            enum format_arg_type type2 = spec.named[j-1].type;
            enum format_arg_type type_both;

            if (type1 == type2)
              type_both = type1;
            else
              {
                /* Incompatible types.  */
                type_both = FAT_NONE;
                if (!err)
                  *invalid_reason =
                    xasprintf (_("The string refers to the argument named '%s' in incompatible ways."), spec.named[i].name);
                err = true;
              }

            spec.named[j-1].type = type_both;
            free (spec.named[i].name);
          }
        else
          {
            if (j < i)
              {
                spec.named[j].name = spec.named[i].name;
                spec.named[j].type = spec.named[i].type;
              }
            j++;
          }
      spec.named_arg_count = j;
      if (err)
        /* *invalid_reason has already been set above.  */
        goto bad_format;
    }

  result = XMALLOC (struct spec);
  *result = spec;
  return result;

 bad_format:
  if (spec.named != NULL)
    {
      unsigned int i;
      for (i = 0; i < spec.named_arg_count; i++)
        free (spec.named[i].name);
      free (spec.named);
    }
  if (spec.numbered != NULL)
    free (spec.numbered);
  return NULL;
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

  if (spec1->named_arg_count > 0 && spec2->numbered_arg_count > 0)
    {
      if (error_logger)
        error_logger (_("format specifications in '%s' expect a hash table, those in '%s' expect individual arguments"),
                      pretty_msgid, pretty_msgstr);
      err = true;
    }
  else if (spec1->numbered_arg_count > 0 && spec2->named_arg_count > 0)
    {
      if (error_logger)
        error_logger (_("format specifications in '%s' expect individual arguments, those in '%s' expect a hash table"),
                      pretty_msgid, pretty_msgstr);
      err = true;
    }
  else
    {
      if (spec1->named_arg_count + spec2->named_arg_count > 0)
        {
          unsigned int i, j;
          unsigned int n1 = spec1->named_arg_count;
          unsigned int n2 = spec2->named_arg_count;

          /* Check that the argument names are the same.
             Both arrays are sorted.  We search for the first difference.  */
          for (i = 0, j = 0; i < n1 || j < n2; )
            {
              int cmp = (i >= n1 ? 1 :
                         j >= n2 ? -1 :
                         strcmp (spec1->named[i].name, spec2->named[j].name));

              if (cmp > 0)
                {
                  if (error_logger)
                    error_logger (_("a format specification for argument '%s', as in '%s', doesn't exist in '%s'"),
                                  spec2->named[j].name, pretty_msgstr,
                                  pretty_msgid);
                  err = true;
                  break;
                }
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
          /* Check the argument types are the same.  */
          if (!err)
            for (i = 0, j = 0; j < n2; )
              {
                if (strcmp (spec1->named[i].name, spec2->named[j].name) == 0)
                  {
                    if (!(spec1->named[i].type == spec2->named[j].type))
                      {
                        if (error_logger)
                          error_logger (_("format specifications in '%s' and '%s' for argument '%s' are not the same"),
                                        pretty_msgid, pretty_msgstr,
                                        spec2->named[j].name);
                        err = true;
                        break;
                      }
                    j++, i++;
                  }
                else
                  i++;
              }
        }

      if (spec1->numbered_arg_count + spec2->numbered_arg_count > 0)
        {
          unsigned int i;

          /* Check the argument types are the same.  */
          if (spec1->numbered_arg_count != spec2->numbered_arg_count)
            {
              if (error_logger)
                error_logger (_("number of format specifications in '%s' and '%s' does not match"),
                              pretty_msgid, pretty_msgstr);
              err = true;
            }
          else
            for (i = 0; i < spec2->numbered_arg_count; i++)
              if (!(spec1->numbered[i].type == spec2->numbered[i].type))
                {
                  if (error_logger)
                    error_logger (_("format specifications in '%s' and '%s' for argument %u are not the same"),
                                  pretty_msgid, pretty_msgstr, i + 1);
                  err = true;
                }
        }
    }

  return err;
}


struct formatstring_parser formatstring_ruby =
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

  if (spec->named_arg_count > 0)
    {
      if (spec->numbered_arg_count > 0)
        abort ();

      printf ("({");
      for (i = 0; i < spec->named_arg_count; i++)
        {
          if (i > 0)
            printf (", ");
          printf (":%s => ", spec->named[i].name);
          switch (spec->named[i].type)
            {
            case FAT_ANY:
              printf ("s");
              break;
            case FAT_ESCAPED_ANY:
              printf ("p");
              break;
            case FAT_CHARACTER:
              printf ("c");
              break;
            case FAT_INTEGER:
              printf ("i");
              break;
            case FAT_FLOAT:
              printf ("f");
              break;
            default:
              abort ();
            }
        }
      printf ("})");
    }
  else
    {
      unsigned int last;

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
            case FAT_ANY:
              printf ("s");
              break;
            case FAT_ESCAPED_ANY:
              printf ("p");
              break;
            case FAT_CHARACTER:
              printf ("c");
              break;
            case FAT_INTEGER:
              printf ("i");
              break;
            case FAT_FLOAT:
              printf ("f");
              break;
            default:
              abort ();
            }
          last = number + 1;
        }
      printf (")");
    }
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
 * compile-command: "/bin/sh ../libtool --tag=CC --mode=link gcc -o a.out -static -O -g -Wall -I.. -I../gnulib-lib -I../../gettext-runtime/intl -DHAVE_CONFIG_H -DTEST format-ruby.c ../gnulib-lib/libgettextlib.la"
 * End:
 */

#endif /* TEST */
