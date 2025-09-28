/* C++ format strings.
   Copyright (C) 2003-2023 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2023.

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

/* C++ format strings are specified in ISO C++ 20.
   Reference:
     - https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/n4861.pdf
       § 20.20 Formatting [format]
       corrected in
       https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/n4950.pdf
       § 22.14 Formatting [format]
     - https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/n4861.pdf
       § 27.12 Formatting [time.format]
     - https://en.cppreference.com/w/cpp/utility/format/format
   Implemented in GCC 13.1, usable with option '-std=c++20' or '-std=gnu++20'.

   In GNU gettext, we don't support custom format directives.  See the
   documentation, section "Preparing Translatable Strings".
   Also, we don't support time formatting, because its translation should be
   looked up according to the locale's LC_TIME category, where gettext() and
   dgettext() use the LC_MESSAGES category.  Support for time format strings
   may be added later, separately, with a 'c++-time-format' tag.

   There are two kinds of directives: replacement fields and escape sequences.
   A replacement field
     - starts with '{',
     - is optionally followed by an "argument index" specification: a nonempty
       digit sequence without redundant leading zeroes,
     - is optionally followed by ':' and
       - optionally a "fill-and-align" specification:
         an optional character other than '{' and '}', followed by one of
         '<', '>', '^',
       - optionally a "sign" specification: one of '+', '-', ' ',
         (but only for an [integer], [float] argument or when the type is one
         of 'b', 'B', 'd', 'o', 'x', 'X')
       - optionally: '#',
         (but only for an [integer], [float] argument or when the type is one
         of 'b', 'B', 'd', 'o', 'x', 'X')
       - optionally: '0',
         (but only for an [integer], [float] argument or when the type is one
         of 'b', 'B', 'd', 'o', 'x', 'X')
       - optionally a "width" specification:
         - a nonempty digit sequence with the first digit being non-zero, or
         - '{',
           then an "argument index" specification: a digit sequence without
           redundant leading zeroes,
           then '}',
       - optionally a "precision" specification:
         - '.', then
           - a nonempty digit sequence, or
           - '{',
             then an "argument index" specification: a digit sequence without
             redundant leading zeroes,
             then '}',
           (but only for a [float], [string] argument)
       - optionally: 'L',
         (but only for an [integer], [float], [character], [bool] argument)
       - optionally a "type" specification: one of
         [integer, character, bool]  'b', 'B', 'd', 'o', 'x', 'X',
         [float]                     'a', 'A', 'e', 'E', 'f', 'F', 'g', 'G',
         [integer, character]        'c',
         [string, bool]              's',
         [pointer]                   'p',
     - is finished by '}'.
   An escape sequence is either '{{' or '}}'.
   Numbered ({n} or {n:spec}) and unnumbered ({} or {:spec}) argument
   specifications cannot be used in the same string.

   The translator may add *argument index* specifications everywhere, i.e.
   convert all unnumbered argument specifications to numbered argument
   specifications.  Or vice versa.

   The translator may add or remove *fill-and-align* specifications, because
   they are valid for all types.

   The translator may add a *sign* specification when the type is one of
   'b', 'B', 'd', 'o', 'x', 'X', 'a', 'A', 'e', 'E', 'f', 'F', 'g', 'G',
   but not when it is 'c' (because for [character] arguments, "{:c}" is valid
   but "{:-c}" is not).
   The translator may remove *sign* specifications.

   The translator may add a '#' flag when the type is one of
   'b', 'B', 'd', 'o', 'x', 'X', 'a', 'A', 'e', 'E', 'f', 'F', 'g', 'G',
   but not when it is 'c' (because for [character] arguments, "{:c}" is valid
   but "{:#c}" is not).
   The translator may remove a '#' flag.

   The translator may add a '0' flag when the type is one of
   'b', 'B', 'd', 'o', 'x', 'X', 'a', 'A', 'e', 'E', 'f', 'F', 'g', 'G',
   but not when it is 'c' (because for [character] arguments, "{:c}" is valid
   but "{:0c}" is not).
   The translator may remove a '0' flag.

   The translator may add or remove *width* specifications, because they are
   valid for all types.

   The translator may add a *precision* specification when the type is one of
   'a', 'A', 'e', 'E', 'f', 'F', 'g', 'G',
   but not when it is 's' (because for [bool] arguments, "{:s}" is valid but
   "{:.9s}" is not).
   The translator may remove *precision* specifications.

   The translator may add an 'L' flag when the type is one of
   'b', 'B', 'd', 'o', 'x', 'X', 'a', 'A', 'e', 'E', 'f', 'F', 'g', 'G', 'c',
   but not when it is 's' (because for [string] arguments, "{:s}" is valid but
   "{:Ls}" is not).
   The translator may remove an 'L' flag.
 */

/* Describes the valid argument types for a given directive.  */
enum format_arg_type
{
  FAT_INTEGER    = 1 << 0,
  FAT_FLOAT      = 1 << 1,
  FAT_CHARACTER  = 1 << 2,
  FAT_STRING     = 1 << 3,
  FAT_BOOL       = 1 << 4,
  FAT_POINTER    = 1 << 5,
  FAT_NONE       = 0,
  FAT_ANY        = (FAT_INTEGER | FAT_FLOAT | FAT_CHARACTER | FAT_STRING
                    | FAT_BOOL | FAT_POINTER)
};

struct numbered_arg
{
  /* The number of the argument, 0-based.  */
  unsigned int number;

  /* The type is a bit mask that is the logical OR of the 'enum format_arg_type'
     values that represent each possible argument types for the argument.
     We use this field in order to report an error in format_check for cases like
       #, c++-format
       msgid  "{:c}"
       msgstr "{:-c}"
     because "{:c}" is valid for argument type [integer, character], whereas
     "{:-c}" is valid only for [integer].  */
  unsigned int type;

  /* The presentation is a bit mask that is the logical OR of the
     'enum format_arg_type' values of the directives that reference
     the argument.  Here the "type" specifications are mapped like this:
       'b', 'B', 'd', 'o', 'x', 'X'            -> FAT_INTEGER
       'a', 'A', 'e', 'E', 'f', 'F', 'g', 'G'  -> FAT_FLOAT
       'c'                                     -> FAT_CHARACTER
       's'                                     -> FAT_STRING
       'p'                                     -> FAT_POINTER
     The use of this presentation field is that
       * We want to allow the translator to change e.g. an 'o' type to an 'x'
         type, or an 'e' type to a 'g' type, and so on.  (This is possible
         for c-format strings too.)
       * But we do not want to allow the translator to change e.g. a 'c' type
         to a 'd' type:
           #, c++-format
           msgid  "{:c}"
           msgstr "{:d}"
     We use this field in order to report an error in format_check.  */
  unsigned int presentation;
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
    if (*format == '{')
      {
        FDI_SET (format, FMTDIR_START);
        format++;
        spec.directives++;
        if (*format == '{')
          /* An escape sequence '{{'.  */
          ;
        else
          {
            /* A replacement field.  */
            unsigned int arg_array_index;
            bool have_sign = false;
            bool have_hash_flag = false;
            bool have_zero_flag = false;
            bool have_precision = false;
            bool have_L_flag = false;
            char type_spec;
            unsigned int type;
            unsigned int presentation;

            /* Parse arg-id.  */
            if (isdigit (*format))
              {
                /* Numbered argument.  */
                unsigned int arg_id;

                arg_id = (*format - '0');
                if (*format == '0')
                  format++;
                else
                  {
                    format++;
                    while (isdigit (*format))
                      {
                        if (arg_id >= UINT_MAX / 10)
                          {
                            *invalid_reason =
                              xasprintf (_("In the directive number %u, the arg-id is too large."), spec.directives);
                            FDI_SET (format, FMTDIR_ERROR);
                            goto bad_format;
                          }
                        /* Here arg_id <= floor(UINT_MAX/10) - 1.  */
                        arg_id = arg_id * 10 + (*format - '0');
                        /* Here arg_id < floor(UINT_MAX/10)*10 <= UINT_MAX.  */
                        format++;
                      }
                  }

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
                arg_array_index = spec.numbered_arg_count;
                spec.numbered[spec.numbered_arg_count].number = arg_id + 1;
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
                arg_array_index = unnumbered_arg_count;
                spec.numbered[unnumbered_arg_count].number = unnumbered_arg_count + 1;
                unnumbered_arg_count++;
              }

            type = FAT_ANY;
            presentation = 0;

            if (*format == ':')
              {
                format++;
                /* Parse format-spec.  */

                /* Parse fill-and-align.  */
                if ((*format != '0' && *format != '{' && *format != '}')
                    && (format[1] == '<' || format[1] == '>'
                        || format[1] == '^'))
                  format += 2;
                else if (*format == '<' || *format == '>' || *format == '^')
                  format++;

                /* Parse sign.  */
                if (*format == '+' || *format == '-' || *format == ' ')
                  {
                    format++;
                    have_sign = true;
                  }

                /* Parse '#' flag.  */
                if (*format == '#')
                  {
                    format++;
                    have_hash_flag = true;
                  }

                /* Parse '0' flag.  */
                if (*format == '0')
                  {
                    format++;
                    have_zero_flag = true;
                  }

                /* Parse width.  */
                if (isdigit (*format) && *format != '0')
                  {
                    do
                      format++;
                    while (isdigit (*format));
                  }
                else if (*format == '{')
                  {
                    format++;
                    if (isdigit (*format))
                      {
                        /* Numbered argument.  */
                        unsigned int width_arg_id;

                        width_arg_id = (*format - '0');
                        if (*format == '0')
                          format++;
                        else
                          {
                            format++;
                            while (isdigit (*format))
                              {
                                if (width_arg_id >= UINT_MAX / 10)
                                  {
                                    *invalid_reason =
                                      xasprintf (_("In the directive number %u, the width's arg-id is too large."), spec.directives);
                                    FDI_SET (format, FMTDIR_ERROR);
                                    goto bad_format;
                                  }
                                /* Here width_arg_id <= floor(UINT_MAX/10) - 1.  */
                                width_arg_id = width_arg_id * 10 + (*format - '0');
                                /* Here width_arg_id < floor(UINT_MAX/10)*10 <= UINT_MAX.  */
                                format++;
                              }
                          }

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
                        spec.numbered[spec.numbered_arg_count].number = width_arg_id + 1;
                        spec.numbered[spec.numbered_arg_count].type = FAT_INTEGER;
                        spec.numbered[spec.numbered_arg_count].presentation = 0;
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
                        spec.numbered[unnumbered_arg_count].type = FAT_INTEGER;
                        spec.numbered[unnumbered_arg_count].presentation = 0;
                        unnumbered_arg_count++;
                      }

                    if (*format != '}')
                      {
                        *invalid_reason =
                          xasprintf (_("In the directive number %u, the width's arg-id is not terminated through '}'."), spec.directives);
                        FDI_SET (format - 1, FMTDIR_ERROR);
                        goto bad_format;
                      }
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

                        have_precision = true;
                      }
                    else if (*format == '{')
                      {
                        format++;
                        if (isdigit (*format))
                          {
                            /* Numbered argument.  */
                            unsigned int precision_arg_id;

                            precision_arg_id = (*format - '0');
                            if (*format == '0')
                              format++;
                            else
                              {
                                format++;
                                while (isdigit (*format))
                                  {
                                    if (precision_arg_id >= UINT_MAX / 10)
                                      {
                                        *invalid_reason =
                                          xasprintf (_("In the directive number %u, the width's arg-id is too large."), spec.directives);
                                        FDI_SET (format, FMTDIR_ERROR);
                                        goto bad_format;
                                      }
                                    /* Here precision_arg_id <= floor(UINT_MAX/10) - 1.  */
                                    precision_arg_id = precision_arg_id * 10 + (*format - '0');
                                    /* Here precision_arg_id < floor(UINT_MAX/10)*10 <= UINT_MAX.  */
                                    format++;
                                  }
                              }

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
                            spec.numbered[spec.numbered_arg_count].number = precision_arg_id + 1;
                            spec.numbered[spec.numbered_arg_count].type = FAT_INTEGER;
                            spec.numbered[spec.numbered_arg_count].presentation = 0;
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
                            spec.numbered[unnumbered_arg_count].type = FAT_INTEGER;
                            spec.numbered[unnumbered_arg_count].presentation = 0;
                            unnumbered_arg_count++;
                          }

                        if (*format != '}')
                          {
                            *invalid_reason =
                              xasprintf (_("In the directive number %u, the precision's arg-id is not terminated through '}'."), spec.directives);
                            FDI_SET (format - 1, FMTDIR_ERROR);
                            goto bad_format;
                          }
                        format++;

                        have_precision = true;
                      }
                    else
                      format--;
                  }

                /* Parse 'L' flag.  */
                if (*format == 'L')
                  {
                    format++;
                    have_L_flag = true;
                  }

                /* Parse type.  */
                type_spec = '\0';
                if (*format != '\0' && *format != '}')
                  {
                    type_spec = *format;

                    switch (type_spec)
                      {
                      case 'b': case 'B':
                      case 'd':
                      case 'o':
                      case 'x': case 'X':
                        type = FAT_INTEGER | FAT_CHARACTER | FAT_BOOL;
                        presentation = FAT_INTEGER;
                        break;

                      case 'a': case 'A':
                      case 'e': case 'E':
                      case 'f': case 'F':
                      case 'g': case 'G':
                        type = FAT_FLOAT;
                        presentation = FAT_FLOAT;
                        break;

                      case 'c':
                        type = FAT_INTEGER | FAT_CHARACTER;
                        presentation = FAT_CHARACTER;
                        break;

                      case 's':
                        type = FAT_STRING | FAT_BOOL;
                        presentation = FAT_STRING;
                        break;

                      case 'p':
                        type = FAT_POINTER;
                        presentation = FAT_POINTER;
                        break;

                      default:
                        *invalid_reason =
                          (c_isprint (type_spec)
                           ? xasprintf (_("In the directive number %u, the character '%c' is not a standard type specifier."), spec.directives, type_spec)
                           : xasprintf (_("The character that terminates the directive number %u is not a standard type specifier."), spec.directives));
                        FDI_SET (format, FMTDIR_ERROR);
                        goto bad_format;
                      }

                    if (have_sign && (type & (FAT_INTEGER | FAT_FLOAT)) == 0)
                      {
                        *invalid_reason =
                          xasprintf (_("In the directive number %u, the sign specification is incompatible with the type specifier '%c'."), spec.directives, type_spec);
                        FDI_SET (format, FMTDIR_ERROR);
                        goto bad_format;
                      }

                    if (have_hash_flag && (type & (FAT_INTEGER | FAT_FLOAT)) == 0)
                      {
                        *invalid_reason =
                          xasprintf (_("In the directive number %u, the '#' option is incompatible with the type specifier '%c'."), spec.directives, type_spec);
                        FDI_SET (format, FMTDIR_ERROR);
                        goto bad_format;
                      }

                    if (have_zero_flag && (type & (FAT_INTEGER | FAT_FLOAT)) == 0)
                      {
                        *invalid_reason =
                          xasprintf (_("In the directive number %u, the '0' option is incompatible with the type specifier '%c'."), spec.directives, type_spec);
                        FDI_SET (format, FMTDIR_ERROR);
                        goto bad_format;
                      }

                    if (have_precision && (type & (FAT_FLOAT | FAT_STRING)) == 0)
                      {
                        *invalid_reason =
                          xasprintf (_("In the directive number %u, the precision specification is incompatible with the type specifier '%c'."), spec.directives, type_spec);
                        FDI_SET (format, FMTDIR_ERROR);
                        goto bad_format;
                      }

                    if (have_L_flag && (type & (FAT_INTEGER | FAT_FLOAT | FAT_CHARACTER | FAT_BOOL)) == 0)
                      {
                        *invalid_reason =
                          xasprintf (_("In the directive number %u, the 'L' option is incompatible with the type specifier '%c'."), spec.directives, type_spec);
                        FDI_SET (format, FMTDIR_ERROR);
                        goto bad_format;
                      }

                    format++;
                  }

                if (have_sign)
                  {
                    /* Citing ISO C++ 20:
                       "The sign option is only valid for arithmetic types other
                        than charT and bool or when an integer presentation type
                        is specified."  */
                    switch (type_spec)
                      {
                      case 'b': case 'B':
                      case 'd':
                      case 'o':
                      case 'x': case 'X':
                        break;
                      default:
                        /* No type_spec:     FAT_ANY                     -> FAT_INTEGER | FAT_FLOAT
                           type_spec = 'c':  FAT_INTEGER | FAT_CHARACTER -> FAT_INTEGER
                          */
                        type = type & (FAT_INTEGER | FAT_FLOAT);
                        break;
                      }
                  }

                if (have_hash_flag)
                  {
                    /* Citing ISO C++ 20:
                       "The # option ... This option is valid for arithmetic
                        types other than charT and bool or when an integer
                        presentation type is specified, and not otherwise."  */
                    switch (type_spec)
                      {
                      case 'b': case 'B':
                      case 'd':
                      case 'o':
                      case 'x': case 'X':
                        break;
                      default:
                        /* No type_spec:     FAT_ANY                     -> FAT_INTEGER | FAT_FLOAT
                           type_spec = 'c':  FAT_INTEGER | FAT_CHARACTER -> FAT_INTEGER
                          */
                        type = type & (FAT_INTEGER | FAT_FLOAT);
                        break;
                      }
                  }

                if (have_zero_flag)
                  {
                    /* Citing ISO C++ 20:
                       "A zero (0) character preceding the width field ... This
                        option is only valid for arithmetic types other than
                        charT and bool or when an integer presentation type is
                        specified."  */
                    switch (type_spec)
                      {
                      case 'b': case 'B':
                      case 'd':
                      case 'o':
                      case 'x': case 'X':
                        break;
                      default:
                        /* No type_spec:     FAT_ANY                     -> FAT_INTEGER | FAT_FLOAT
                           type_spec = 'c':  FAT_INTEGER | FAT_CHARACTER -> FAT_INTEGER
                          */
                        type = type & (FAT_INTEGER | FAT_FLOAT);
                        break;
                      }
                  }

                if (have_precision)
                  {
                    /* Citing ISO C++ 20:
                       "The nonnegative-integer in precision ... It can only be
                        used with floating-point and string types."  */
                    /* No type_spec:     FAT_ANY               -> FAT_FLOAT | FAT_STRING
                       type_spec = 's':  FAT_STRING | FAT_BOOL -> FAT_STRING
                      */
                    type = type & (FAT_FLOAT | FAT_STRING);
                  }

                if (have_L_flag)
                  {
                    /* Citing ISO C++ 20:
                       "The L option is only valid for arithmetic types"  */
                    /* No type_spec:     FAT_ANY               -> FAT_INTEGER | FAT_FLOAT | FAT_CHARACTER | FAT_BOOL
                       type_spec = 's':  FAT_STRING | FAT_BOOL -> FAT_BOOL
                     */
                    type = type & (FAT_INTEGER | FAT_FLOAT | FAT_CHARACTER | FAT_BOOL);
                  }

                /* If no possible type is left for the directive, e.g. in the
                   format string "{:.9Ls}", the format string is invalid.  */
                if (type == FAT_NONE)
                  {
                    *invalid_reason =
                      xasprintf (_("The directive number %u, with all of its options, is not applicable to any type."), spec.directives);
                    FDI_SET (format - 1, FMTDIR_ERROR);
                    goto bad_format;
                  }
              }

            spec.numbered[arg_array_index].type = type;
            spec.numbered[arg_array_index].presentation = presentation;

            if (*format == '\0')
              {
                *invalid_reason =
                  xasprintf (_("The string ends in the middle of the directive number %u."), spec.directives);
                FDI_SET (format - 1, FMTDIR_ERROR);
                goto bad_format;
              }

            if (*format != '}')
              {
                *invalid_reason =
                  xasprintf (_("The directive number %u is not terminated through '}'."), spec.directives);
                FDI_SET (format - 1, FMTDIR_ERROR);
                goto bad_format;
              }
          }

        FDI_SET (format, FMTDIR_END);

        format++;
      }
    else if (*format == '}')
      {
        FDI_SET (format, FMTDIR_START);
        format++;
        spec.directives++;
        if (*format == '}')
          /* An escape sequence '}}'.  */
          ;
        else
          {
            *invalid_reason =
              (spec.directives == 0
               ? xstrdup (_("The string starts in the middle of a directive: found '}' without matching '{'."))
               : xasprintf (_("The string contains a lone '}' after directive number %u."), spec.directives));
            FDI_SET (*format == '\0' ? format - 1 : format, FMTDIR_ERROR);
            goto bad_format;
          }

        FDI_SET (format, FMTDIR_END);

        format++;
      }
    else
      format++;

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
            unsigned int type1 = spec.numbered[i].type;
            unsigned int type2 = spec.numbered[j-1].type;
            unsigned int type_both = type1 & type2;

            if (type_both == FAT_NONE)
              {
                /* Incompatible types.  */
                if (!err)
                  *invalid_reason =
                    INVALID_INCOMPATIBLE_ARG_TYPES (spec.numbered[i].number);
                err = true;
              }

            spec.numbered[j-1].type = type_both;
            spec.numbered[j-1].presentation =
              spec.numbered[i].presentation | spec.numbered[j-1].presentation;
          }
        else
          {
            if (j < i)
              {
                spec.numbered[j].number = spec.numbered[i].number;
                spec.numbered[j].type = spec.numbered[i].type;
                spec.numbered[j].presentation = spec.numbered[i].presentation;
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

/* Size of buffer needed for type_description result.  */
#define MAX_TYPE_DESCRIPTION_LEN (50 + 1)
/* Returns a textual description of TYPE in BUF.  */
static void
get_type_description (char buf[MAX_TYPE_DESCRIPTION_LEN], unsigned int type)
{
  char *p = buf;
  bool first = true;

  p = stpcpy (p, "[");
  if (type & FAT_INTEGER)
    {
      if (!first)
        p = stpcpy (p, ", ");
      p = stpcpy (p, "integer");
      first = false;
    }
  if (type & FAT_FLOAT)
    {
      if (!first)
        p = stpcpy (p, ", ");
      p = stpcpy (p, "float");
      first = false;
    }
  if (type & FAT_CHARACTER)
    {
      if (!first)
        p = stpcpy (p, ", ");
      p = stpcpy (p, "character");
      first = false;
    }
  if (type & FAT_STRING)
    {
      if (!first)
        p = stpcpy (p, ", ");
      p = stpcpy (p, "string");
      first = false;
    }
  if (type & FAT_BOOL)
    {
      if (!first)
        p = stpcpy (p, ", ");
      p = stpcpy (p, "bool");
      first = false;
    }
  if (type & FAT_POINTER)
    {
      if (!first)
        p = stpcpy (p, ", ");
      p = stpcpy (p, "pointer");
      first = false;
    }
  p = stpcpy (p, "]");
  *p++ = '\0';
  /* Verify that the buffer was large enough.  */
  if (p - buf > MAX_TYPE_DESCRIPTION_LEN)
    abort ();
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
      /* Check that the argument types are not being restricted in the msgstr,
         and that the presentation does not get changed in the msgstr.  */
      if (!err)
        for (i = 0, j = 0; j < n2; )
          {
            if (spec1->numbered[i].number == spec2->numbered[j].number)
              {
                unsigned int type_difference = spec1->numbered[i].type & ~spec2->numbered[j].type;
                if (type_difference != 0)
                  {
                    if (error_logger)
                      {
                        char buf[MAX_TYPE_DESCRIPTION_LEN];
                        get_type_description (buf, type_difference);
                        error_logger (_("The format specification for argument %u in '%s' is applicable to the types %s, but the format specification for argument %u in '%s' is not."),
                                      spec1->numbered[i].number, pretty_msgid, buf,
                                      spec2->numbered[j].number, pretty_msgstr);
                      }
                    err = true;
                    break;
                  }
                unsigned int presentation_difference =
                  spec2->numbered[j].presentation & ~spec1->numbered[i].presentation;
                if (presentation_difference != 0)
                  {
                    if (error_logger)
                      error_logger (_("The format specification for argument %u in '%s' uses a different presentation than the format specification for argument %u in '%s'."),
                                    spec2->numbered[j].number, pretty_msgstr,
                                    spec1->numbered[i].number, pretty_msgid);
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


struct formatstring_parser formatstring_cplusplus_brace =
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
      char buf[MAX_TYPE_DESCRIPTION_LEN];

      if (i > 0)
        printf (" ");
      if (number < last)
        abort ();
      for (; last < number; last++)
        printf ("_ ");
      get_type_description (buf, spec->numbered[i].type);
      printf ("%s", buf);
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
 * compile-command: "/bin/sh ../libtool --tag=CC --mode=link gcc -o a.out -static -O -g -Wall -I.. -I../gnulib-lib -I../../gettext-runtime/intl -DHAVE_CONFIG_H -DTEST format-c++-brace.c ../gnulib-lib/libgettextlib.la"
 * End:
 */

#endif /* TEST */
