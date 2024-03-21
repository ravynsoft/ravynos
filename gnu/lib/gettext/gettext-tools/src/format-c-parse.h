/* Parsing C format strings.
   Copyright (C) 2001-2004, 2006-2007, 2009-2010, 2018, 2020, 2022-2023 Free Software Foundation, Inc.
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


/* C format strings are described in
     * POSIX (IEEE P1003.1 2001), section XSH 3 fprintf().
     * ISO C 23, section 7.23.6.1 fprintf and section 7.31.2.1 fwprintf.
     * See also Linux fprintf(3) manual page.
   A directive
   - starts with '%' or '%m$' where m is a positive integer,
   - is optionally followed by any of the characters '#', '0', '-', ' ', '+',
     "'", or - only in msgstr strings - the string "I", each of which acts as
     a flag,
   - is optionally followed by a width specification: '*' (reads an argument)
     or '*m$' or a nonempty digit sequence,
   - is optionally followed by '.' and a precision specification: '*' (reads
     an argument) or '*m$' or a nonempty digit sequence,
   - is either continued like this:
       - is optionally followed by a size specifier, one of 'hh' 'h' 'l' 'll'
         'L' 'q' 'j' 'z' 't',
       - is finished by a specifier
           - '%', that needs no argument,
           - 'c', 'C', that need a character argument,
           - 's', 'S', that need a string argument,
           - 'i', 'd', that need a signed integer argument,
           - 'u', 'o', 'x', 'X', 'b', that need an unsigned integer argument,
           - 'e', 'E', 'f', 'F', 'g', 'G', 'a', 'A', that need a floating-point
             argument,
           - 'p', that needs a 'void *' argument,
           - 'n', that needs a pointer to integer.
     or is finished by a specifier '<' inttypes-macro '>' where inttypes-macro
     is an ISO C 99 section 7.8.1 format directive.
   Numbered ('%m$' or '*m$') and unnumbered argument specifications cannot
   be used in the same string.  When numbered argument specifications are
   used, specifying the Nth argument requires that all the leading arguments,
   from the first to the (N-1)th, are specified in the format string.
 */

enum format_arg_type
{
  FAT_NONE              = 0,
  /* Basic types */
  FAT_INTEGER           = 1,
  FAT_DOUBLE            = 2,
  FAT_CHAR              = 3,
  FAT_STRING            = 4,
  FAT_OBJC_OBJECT       = 5,
  FAT_POINTER           = 6,
  FAT_COUNT_POINTER     = 7,
  /* Flags */
  FAT_UNSIGNED          = 1 << 3,
  FAT_SIZE_SHORT        = 1 << 4,
  FAT_SIZE_CHAR         = 2 << 4,
  FAT_SIZE_LONG         = 1 << 6,
  FAT_SIZE_LONGLONG     = 2 << 6,
  FAT_SIZE_8_T          = 1 << 8,
  FAT_SIZE_16_T         = 1 << 9,
  FAT_SIZE_32_T         = 1 << 10,
  FAT_SIZE_64_T         = 1 << 11,
  FAT_SIZE_LEAST8_T     = 1 << 12,
  FAT_SIZE_LEAST16_T    = 1 << 13,
  FAT_SIZE_LEAST32_T    = 1 << 14,
  FAT_SIZE_LEAST64_T    = 1 << 15,
  FAT_SIZE_FAST8_T      = 1 << 16,
  FAT_SIZE_FAST16_T     = 1 << 17,
  FAT_SIZE_FAST32_T     = 1 << 18,
  FAT_SIZE_FAST64_T     = 1 << 19,
  FAT_SIZE_INTMAX_T     = 1 << 20,
  FAT_SIZE_INTPTR_T     = 1 << 21,
  FAT_SIZE_SIZE_T       = 1 << 22,
  FAT_SIZE_PTRDIFF_T    = 1 << 23,
  FAT_WIDE              = FAT_SIZE_LONG,
  /* Meaningful combinations of basic types and flags:
  'signed char'                 = FAT_INTEGER | FAT_SIZE_CHAR,
  'unsigned char'               = FAT_INTEGER | FAT_SIZE_CHAR | FAT_UNSIGNED,
  'short'                       = FAT_INTEGER | FAT_SIZE_SHORT,
  'unsigned short'              = FAT_INTEGER | FAT_SIZE_SHORT | FAT_UNSIGNED,
  'int'                         = FAT_INTEGER,
  'unsigned int'                = FAT_INTEGER | FAT_UNSIGNED,
  'long int'                    = FAT_INTEGER | FAT_SIZE_LONG,
  'unsigned long int'           = FAT_INTEGER | FAT_SIZE_LONG | FAT_UNSIGNED,
  'long long int'               = FAT_INTEGER | FAT_SIZE_LONGLONG,
  'unsigned long long int'      = FAT_INTEGER | FAT_SIZE_LONGLONG | FAT_UNSIGNED,
  'double'                      = FAT_DOUBLE,
  'long double'                 = FAT_DOUBLE | FAT_SIZE_LONGLONG,
  'char'/'int'                  = FAT_CHAR,
  'wchar_t'/'wint_t'            = FAT_CHAR | FAT_SIZE_LONG,
  'const char *'                = FAT_STRING,
  'const wchar_t *'             = FAT_STRING | FAT_SIZE_LONG,
  'void *'                      = FAT_POINTER,
  FAT_COUNT_SCHAR_POINTER       = FAT_COUNT_POINTER | FAT_SIZE_CHAR,
  FAT_COUNT_SHORT_POINTER       = FAT_COUNT_POINTER | FAT_SIZE_SHORT,
  FAT_COUNT_INT_POINTER         = FAT_COUNT_POINTER,
  FAT_COUNT_LONGINT_POINTER     = FAT_COUNT_POINTER | FAT_SIZE_LONG,
  FAT_COUNT_LONGLONGINT_POINTER = FAT_COUNT_POINTER | FAT_SIZE_LONGLONG,
  */
  /* Bitmasks */
  FAT_BASIC_MASK        = (FAT_INTEGER | FAT_DOUBLE | FAT_CHAR | FAT_STRING
                           | FAT_OBJC_OBJECT | FAT_POINTER | FAT_COUNT_POINTER),
  FAT_SIZE_MASK         = (FAT_SIZE_SHORT | FAT_SIZE_CHAR
                           | FAT_SIZE_LONG | FAT_SIZE_LONGLONG
                           | FAT_SIZE_8_T | FAT_SIZE_16_T
                           | FAT_SIZE_32_T | FAT_SIZE_64_T
                           | FAT_SIZE_LEAST8_T | FAT_SIZE_LEAST16_T
                           | FAT_SIZE_LEAST32_T | FAT_SIZE_LEAST64_T
                           | FAT_SIZE_FAST8_T | FAT_SIZE_FAST16_T
                           | FAT_SIZE_FAST32_T | FAT_SIZE_FAST64_T
                           | FAT_SIZE_INTMAX_T | FAT_SIZE_INTPTR_T
                           | FAT_SIZE_SIZE_T | FAT_SIZE_PTRDIFF_T)
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

struct unnumbered_arg
{
  format_arg_type_t type;
};

struct spec
{
  unsigned int directives;
  unsigned int unnumbered_arg_count;
  struct unnumbered_arg *unnumbered;
  bool unlikely_intentional;
  unsigned int sysdep_directives_count;
  const char **sysdep_directives;
};

/* Locale independent test for a decimal digit.
   Argument can be  'char' or 'unsigned char'.  (Whereas the argument of
   <ctype.h> isdigit must be an 'unsigned char'.)  */
#undef isdigit
#define isdigit(c) ((unsigned int) ((c) - '0') < 10)

/* Whether to recognize the 'I' flag.  */
#if SYSDEP_SEGMENTS_PROCESSED
/* The 'I' flag can only occur in glibc >= 2.2.  On other platforms, gettext()
   filters it away even if it is present in the msgstr in the .mo file.  */
# define HANDLE_I_FLAG \
   ((__GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 2)) \
    && !defined __UCLIBC__)
#else
# define HANDLE_I_FLAG 1
#endif


static int
numbered_arg_compare (const void *p1, const void *p2)
{
  unsigned int n1 = ((const struct numbered_arg *) p1)->number;
  unsigned int n2 = ((const struct numbered_arg *) p2)->number;

  return (n1 > n2 ? 1 : n1 < n2 ? -1 : 0);
}

static struct spec *
format_parse_entrails (const char *format, bool translated,
                       bool objc_extensions, char *fdi, char **invalid_reason,
                       struct spec *result)
{
  const char *const format_start = format;
  struct spec spec;
  unsigned int numbered_arg_count;
  struct numbered_arg *numbered;
  unsigned int allocated;

  spec.directives = 0;
  spec.unnumbered_arg_count = 0;
  spec.unnumbered = NULL;
  spec.unlikely_intentional = false;
  spec.sysdep_directives_count = 0;
  spec.sysdep_directives = NULL;
  numbered_arg_count = 0;
  numbered = NULL;
  allocated = 0;

  for (; *format != '\0';)
    /* Invariant: spec.unnumbered_arg_count == 0 || numbered_arg_count == 0.  */
    if (*format++ == '%')
      {
        /* A directive.  */
        unsigned int number = 0;
        format_arg_type_t type;
        /* Relevant for the conversion characters d, i, b, o, u, x, X, n.  */
        format_arg_type_t integer_size;
        /* Relevant for the conversion characters a, A, e, E, f, F, g, G.  */
        format_arg_type_t floatingpoint_size;

        FDI_SET (format - 1, FMTDIR_START);
        spec.directives++;

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
                    *invalid_reason = INVALID_ARGNO_0 (spec.directives);
                    FDI_SET (f, FMTDIR_ERROR);
                    goto bad_format;
                  }
                number = m;
                format = ++f;
              }
          }

        /* Parse flags.  */
        for (;;)
          {
            if (*format == ' ' || *format == '+' || *format == '-'
                || *format == '#' || *format == '0' || *format == '\'')
              format++;
#if HANDLE_I_FLAG
            else if (translated && *format == 'I')
              {
                spec.sysdep_directives =
                  (const char **)
                  xrealloc (spec.sysdep_directives,
                            2 * (spec.sysdep_directives_count + 1)
                            * sizeof (const char *));
                IF_OOM (spec.sysdep_directives, goto bad_format;)
                spec.sysdep_directives[2 * spec.sysdep_directives_count] = format;
                spec.sysdep_directives[2 * spec.sysdep_directives_count + 1] = format + 1;
                spec.sysdep_directives_count++;
                format++;
              }
#endif
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

                /* Numbered and unnumbered specifications are exclusive.  */
                if (spec.unnumbered_arg_count > 0)
                  {
                    *invalid_reason = INVALID_MIXES_NUMBERED_UNNUMBERED ();
                    FDI_SET (format - 1, FMTDIR_ERROR);
                    goto bad_format;
                  }

                if (allocated == numbered_arg_count)
                  {
                    allocated = 2 * allocated + 1;
                    numbered = (struct numbered_arg *) xrealloc (numbered, allocated * sizeof (struct numbered_arg));
                    IF_OOM (numbered, goto bad_format;)
                  }
                numbered[numbered_arg_count].number = width_number;
                numbered[numbered_arg_count].type = FAT_INTEGER;
                numbered_arg_count++;
              }
            else
              {
                /* Unnumbered argument.  */

                /* Numbered and unnumbered specifications are exclusive.  */
                if (numbered_arg_count > 0)
                  {
                    *invalid_reason = INVALID_MIXES_NUMBERED_UNNUMBERED ();
                    FDI_SET (format - 1, FMTDIR_ERROR);
                    goto bad_format;
                  }

                if (allocated == spec.unnumbered_arg_count)
                  {
                    allocated = 2 * allocated + 1;
                    spec.unnumbered = (struct unnumbered_arg *) xrealloc (spec.unnumbered, allocated * sizeof (struct unnumbered_arg));
                    IF_OOM (spec.unnumbered, goto bad_format;)
                  }
                spec.unnumbered[spec.unnumbered_arg_count].type = FAT_INTEGER;
                spec.unnumbered_arg_count++;
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

                    /* Numbered and unnumbered specifications are exclusive.  */
                    if (spec.unnumbered_arg_count > 0)
                      {
                        *invalid_reason = INVALID_MIXES_NUMBERED_UNNUMBERED ();
                        FDI_SET (format - 1, FMTDIR_ERROR);
                        goto bad_format;
                      }

                    if (allocated == numbered_arg_count)
                      {
                        allocated = 2 * allocated + 1;
                        numbered = (struct numbered_arg *) xrealloc (numbered, allocated * sizeof (struct numbered_arg));
                        IF_OOM (numbered, goto bad_format;)
                      }
                    numbered[numbered_arg_count].number = precision_number;
                    numbered[numbered_arg_count].type = FAT_INTEGER;
                    numbered_arg_count++;
                  }
                else
                  {
                    /* Unnumbered argument.  */

                    /* Numbered and unnumbered specifications are exclusive.  */
                    if (numbered_arg_count > 0)
                      {
                        *invalid_reason = INVALID_MIXES_NUMBERED_UNNUMBERED ();
                        FDI_SET (format - 1, FMTDIR_ERROR);
                        goto bad_format;
                      }

                    if (allocated == spec.unnumbered_arg_count)
                      {
                        allocated = 2 * allocated + 1;
                        spec.unnumbered = (struct unnumbered_arg *) xrealloc (spec.unnumbered, allocated * sizeof (struct unnumbered_arg));
                        IF_OOM (spec.unnumbered, goto bad_format;)
                      }
                    spec.unnumbered[spec.unnumbered_arg_count].type = FAT_INTEGER;
                    spec.unnumbered_arg_count++;
                  }
              }
            else if (isdigit (*format))
              {
                do format++; while (isdigit (*format));
              }
          }

        if (!SYSDEP_SEGMENTS_PROCESSED && *format == '<')
          {
            spec.sysdep_directives =
              (const char **)
              xrealloc (spec.sysdep_directives,
                        2 * (spec.sysdep_directives_count + 1)
                        * sizeof (const char *));
            IF_OOM (spec.sysdep_directives, goto bad_format;)
            spec.sysdep_directives[2 * spec.sysdep_directives_count] = format;

            format++;
            /* Parse ISO C 99 section 7.8.1 format string directive.
               Syntax:
               P R I { d | i | o | u | x | X }
               { { | LEAST | FAST } { 8 | 16 | 32 | 64 } | MAX | PTR }  */
            if (*format != 'P')
              {
                *invalid_reason = INVALID_C99_MACRO (spec.directives);
                FDI_SET (*format == '\0' ? format - 1 : format, FMTDIR_ERROR);
                goto bad_format;
              }
            format++;
            if (*format != 'R')
              {
                *invalid_reason = INVALID_C99_MACRO (spec.directives);
                FDI_SET (*format == '\0' ? format - 1 : format, FMTDIR_ERROR);
                goto bad_format;
              }
            format++;
            if (*format != 'I')
              {
                *invalid_reason = INVALID_C99_MACRO (spec.directives);
                FDI_SET (*format == '\0' ? format - 1 : format, FMTDIR_ERROR);
                goto bad_format;
              }
            format++;

            switch (*format)
              {
              case 'i': case 'd':
                type = FAT_INTEGER;
                break;
              case 'u': case 'o': case 'x': case 'X':
                type = FAT_INTEGER | FAT_UNSIGNED;
                break;
              default:
                *invalid_reason = INVALID_C99_MACRO (spec.directives);
                FDI_SET (*format == '\0' ? format - 1 : format, FMTDIR_ERROR);
                goto bad_format;
              }
            format++;

            if (format[0] == 'M' && format[1] == 'A' && format[2] == 'X')
              {
                type |= FAT_SIZE_INTMAX_T;
                format += 3;
              }
            else if (format[0] == 'P' && format[1] == 'T' && format[2] == 'R')
              {
                type |= FAT_SIZE_INTPTR_T;
                format += 3;
              }
            else
              {
                if (format[0] == 'L' && format[1] == 'E' && format[2] == 'A'
                    && format[3] == 'S' && format[4] == 'T')
                  {
                    format += 5;
                    if (format[0] == '8')
                      {
                        type |= FAT_SIZE_LEAST8_T;
                        format++;
                      }
                    else if (format[0] == '1' && format[1] == '6')
                      {
                        type |= FAT_SIZE_LEAST16_T;
                        format += 2;
                      }
                    else if (format[0] == '3' && format[1] == '2')
                      {
                        type |= FAT_SIZE_LEAST32_T;
                        format += 2;
                      }
                    else if (format[0] == '6' && format[1] == '4')
                      {
                        type |= FAT_SIZE_LEAST64_T;
                        format += 2;
                      }
                    else
                      {
                        *invalid_reason = INVALID_C99_MACRO (spec.directives);
                        FDI_SET (*format == '\0' ? format - 1 : format,
                                 FMTDIR_ERROR);
                        goto bad_format;
                      }
                  }
                else if (format[0] == 'F' && format[1] == 'A'
                         && format[2] == 'S' && format[3] == 'T')
                  {
                    format += 4;
                    if (format[0] == '8')
                      {
                        type |= FAT_SIZE_FAST8_T;
                        format++;
                      }
                    else if (format[0] == '1' && format[1] == '6')
                      {
                        type |= FAT_SIZE_FAST16_T;
                        format += 2;
                      }
                    else if (format[0] == '3' && format[1] == '2')
                      {
                        type |= FAT_SIZE_FAST32_T;
                        format += 2;
                      }
                    else if (format[0] == '6' && format[1] == '4')
                      {
                        type |= FAT_SIZE_FAST64_T;
                        format += 2;
                      }
                    else
                      {
                        *invalid_reason = INVALID_C99_MACRO (spec.directives);
                        FDI_SET (*format == '\0' ? format - 1 : format,
                                 FMTDIR_ERROR);
                        goto bad_format;
                      }
                  }
                else
                  {
                    if (format[0] == '8')
                      {
                        type |= FAT_SIZE_8_T;
                        format++;
                      }
                    else if (format[0] == '1' && format[1] == '6')
                      {
                        type |= FAT_SIZE_16_T;
                        format += 2;
                      }
                    else if (format[0] == '3' && format[1] == '2')
                      {
                        type |= FAT_SIZE_32_T;
                        format += 2;
                      }
                    else if (format[0] == '6' && format[1] == '4')
                      {
                        type |= FAT_SIZE_64_T;
                        format += 2;
                      }
                    else
                      {
                        *invalid_reason = INVALID_C99_MACRO (spec.directives);
                        FDI_SET (*format == '\0' ? format - 1 : format,
                                 FMTDIR_ERROR);
                        goto bad_format;
                      }
                  }
              }

            if (*format != '>')
              {
                *invalid_reason = INVALID_ANGLE_BRACKET (spec.directives);
                FDI_SET (*format == '\0' ? format - 1 : format, FMTDIR_ERROR);
                goto bad_format;
              }

            spec.sysdep_directives[2 * spec.sysdep_directives_count + 1] = format + 1;
            spec.sysdep_directives_count++;
          }
        else
          {
            /* Parse size.  */
            integer_size = 0;
            floatingpoint_size = 0;

            if (*format == 'h')
              {
                if (format[1] == 'h')
                  {
                    integer_size = FAT_SIZE_CHAR;
                    format += 2;
                  }
                else
                  {
                    integer_size = FAT_SIZE_SHORT;
                    format++;
                  }
              }
            else if (*format == 'l')
              {
                if (format[1] == 'l')
                  {
                    integer_size = FAT_SIZE_LONGLONG;
                    /* For backward compatibility only.  */
                    floatingpoint_size = FAT_SIZE_LONGLONG;
                    format += 2;
                  }
                else
                  {
                    integer_size = FAT_SIZE_LONG;
                    format++;
                  }
              }
            else if (*format == 'L'
                     /* Old BSD 4.4 convention.  */
                     || *format == 'q')
              {
                integer_size = FAT_SIZE_LONGLONG;
                floatingpoint_size = FAT_SIZE_LONGLONG;
                format++;
              }
            else if (*format == 'j')
              {
                integer_size = FAT_SIZE_INTMAX_T;
                format++;
              }
            else if (*format == 'z' || *format == 'Z')
              {
                /* 'z' is standardized in ISO C 99, but glibc uses 'Z'
                   because the warning facility in gcc-2.95.2 understands
                   only 'Z' (see gcc-2.95.2/gcc/c-common.c:1784).  */
                integer_size = FAT_SIZE_SIZE_T;
                format++;
              }
            else if (*format == 't')
              {
                integer_size = FAT_SIZE_PTRDIFF_T;
                format++;
              }
            else if (*format == 'w')
              {
                /* wN and wfN are standardized in ISO C 23.  */
                if (format[1] == 'f')
                  {
                    if (format[2] == '8')
                      {
                        integer_size = FAT_SIZE_FAST8_T;
                        format += 3;
                      }
                    else if (format[2] == '1' && format[3] == '6')
                      {
                        integer_size = FAT_SIZE_FAST16_T;
                        format += 4;
                      }
                    else if (format[2] == '3' && format[3] == '2')
                      {
                        integer_size = FAT_SIZE_FAST32_T;
                        format += 4;
                      }
                    else if (format[2] == '6' && format[3] == '4')
                      {
                        integer_size = FAT_SIZE_FAST64_T;
                        format += 4;
                      }
                  }
                else
                  {
                    if (format[1] == '8')
                      {
                        integer_size = FAT_SIZE_LEAST8_T;
                        format += 2;
                      }
                    else if (format[1] == '1' && format[2] == '6')
                      {
                        integer_size = FAT_SIZE_LEAST16_T;
                        format += 3;
                      }
                    else if (format[1] == '3' && format[2] == '2')
                      {
                        integer_size = FAT_SIZE_LEAST32_T;
                        format += 3;
                      }
                    else if (format[1] == '6' && format[2] == '4')
                      {
                        integer_size = FAT_SIZE_LEAST64_T;
                        format += 3;
                      }
                  }
              }
#if defined _WIN32 && ! defined __CYGWIN__
            else if (SYSDEP_SEGMENTS_PROCESSED
                     && *format == 'I'
                     && format[1] == '6'
                     && format[2] == '4')
              {
                integer_size = FAT_SIZE_64_T;
                format += 3;
              }
#endif
            if (!((integer_size & ~FAT_SIZE_MASK) == 0))
              abort ();
            if (!((floatingpoint_size & ~FAT_SIZE_LONGLONG) == 0))
              abort ();

            switch (*format)
              {
              case '%':
                /* Programmers writing _("%2%") most often will not want to
                   use this string as a c-format string, but rather as a
                   literal or as a different kind of format string.  */
                if (format[-1] != '%')
                  spec.unlikely_intentional = true;
                type = FAT_NONE;
                break;
              case 'm': /* glibc extension */
                type = FAT_NONE;
                break;
              case 'c':
                type = FAT_CHAR;
                if (integer_size != 0)
                  {
                    if (integer_size != FAT_SIZE_LONG)
                      {
                        *invalid_reason = INVALID_SIZE_SPECIFIER (spec.directives);
                        FDI_SET (format, FMTDIR_ERROR);
                        goto bad_format;
                      }
                    type |= FAT_WIDE;
                  }
                break;
              case 'C': /* obsolete */
                type = FAT_CHAR | FAT_WIDE;
                break;
              case 's':
                type = FAT_STRING;
                if (integer_size != 0)
                  {
                    if (integer_size != FAT_SIZE_LONG)
                      {
                        *invalid_reason = INVALID_SIZE_SPECIFIER (spec.directives);
                        FDI_SET (format, FMTDIR_ERROR);
                        goto bad_format;
                      }
                    type |= FAT_WIDE;
                  }
                break;
              case 'S': /* obsolete */
                type = FAT_STRING | FAT_WIDE;
                break;
              case 'i': case 'd':
                type = FAT_INTEGER | integer_size;
                break;
              case 'u': case 'o': case 'x': case 'X': case 'b':
                type = FAT_INTEGER | FAT_UNSIGNED | integer_size;
                break;
              case 'e': case 'E': case 'f': case 'F': case 'g': case 'G':
              case 'a': case 'A':
                if (integer_size != 0 && floatingpoint_size == 0)
                  {
                    *invalid_reason = INVALID_SIZE_SPECIFIER (spec.directives);
                    FDI_SET (format, FMTDIR_ERROR);
                    goto bad_format;
                  }
                type = FAT_DOUBLE | floatingpoint_size;
                break;
              case '@':
                if (objc_extensions)
                  {
                    type = FAT_OBJC_OBJECT;
                    break;
                  }
                goto other;
              case 'p':
                type = FAT_POINTER;
                break;
              case 'n':
                type = FAT_COUNT_POINTER | integer_size;
                break;
              other:
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
          }

        if (type != FAT_NONE)
          {
            if (number)
              {
                /* Numbered argument.  */

                /* Numbered and unnumbered specifications are exclusive.  */
                if (spec.unnumbered_arg_count > 0)
                  {
                    *invalid_reason = INVALID_MIXES_NUMBERED_UNNUMBERED ();
                    FDI_SET (format, FMTDIR_ERROR);
                    goto bad_format;
                  }

                if (allocated == numbered_arg_count)
                  {
                    allocated = 2 * allocated + 1;
                    numbered = (struct numbered_arg *) xrealloc (numbered, allocated * sizeof (struct numbered_arg));
                    IF_OOM (numbered, goto bad_format;)
                  }
                numbered[numbered_arg_count].number = number;
                numbered[numbered_arg_count].type = type;
                numbered_arg_count++;
              }
            else
              {
                /* Unnumbered argument.  */

                /* Numbered and unnumbered specifications are exclusive.  */
                if (numbered_arg_count > 0)
                  {
                    *invalid_reason = INVALID_MIXES_NUMBERED_UNNUMBERED ();
                    FDI_SET (format, FMTDIR_ERROR);
                    goto bad_format;
                  }

                if (allocated == spec.unnumbered_arg_count)
                  {
                    allocated = 2 * allocated + 1;
                    spec.unnumbered = (struct unnumbered_arg *) xrealloc (spec.unnumbered, allocated * sizeof (struct unnumbered_arg));
                    IF_OOM (spec.unnumbered, goto bad_format;)
                  }
                spec.unnumbered[spec.unnumbered_arg_count].type = type;
                spec.unnumbered_arg_count++;
              }
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

  /* Verify that the format string uses all arguments up to the highest
     numbered one.  */
  if (numbered_arg_count > 0)
    {
      unsigned int i;

      for (i = 0; i < numbered_arg_count; i++)
        if (numbered[i].number != i + 1)
          {
            *invalid_reason = INVALID_IGNORED_ARGUMENT (numbered[i].number, i + 1);
            goto bad_format;
          }

      /* So now the numbered arguments array is equivalent to a sequence
         of unnumbered arguments.  */
      spec.unnumbered_arg_count = numbered_arg_count;
      allocated = spec.unnumbered_arg_count;
      spec.unnumbered = XNMALLOC (allocated, struct unnumbered_arg);
      IF_OOM (spec.unnumbered, goto bad_format;)
      for (i = 0; i < spec.unnumbered_arg_count; i++)
        spec.unnumbered[i].type = numbered[i].type;
      free (numbered);
      numbered_arg_count = 0;
    }

  *result = spec;
  return result;

 bad_format:
  if (numbered != NULL)
    free (numbered);
  if (spec.unnumbered != NULL)
    free (spec.unnumbered);
  if (spec.sysdep_directives != NULL)
    free (spec.sysdep_directives);
  return NULL;
}
