/* GCC internal format strings.
   Copyright (C) 2003-2009, 2019-2020, 2023 Free Software Foundation, Inc.
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

/* GCC internal format strings consist of language frontend independent
   format directives, implemented in gcc-4.3.0/gcc/pretty-print.c (function
   pp_base_format), plus some frontend dependent extensions:
     - for the C/ObjC frontend
       in gcc-4.3.0/gcc/c-objc-common.c (function c_tree_printer)
     - for the C++ frontend
       in gcc-4.3.0/gcc/cp/error.c (function cp_printer)
   Taking these together, GCC internal format strings are specified as follows.

   A directive
   - starts with '%',
   - either is finished by one of these:
       - '%', '<', '>', "'", that need no argument,
       - 'm', that needs no argument but looks at an err_no variable,
   - or is continued like this:
       - optionally 'm$' where m is a positive integer,
       - optionally any number of flags:
         'q' (once only),
         'l' (up to twice) or 'w' (once only) (exclusive),
         '+' (once only),
         '#' (once only),
       - finished by a specifier

           - 'c', that needs a character argument,
           - 's', that needs a string argument,
           - '.NNNs', where NNN is a nonempty digit sequence, that needs a
             string argument,
           - '.*NNN$s' where NNN is a positive integer and NNN = m - 1, that
             needs a signed integer argument at position NNN and a string
             argument,
           - '.*s', that needs a signed integer argument and a string argument,
           - 'i', 'd', that need a signed integer argument of the specified
             size,
           - 'o', 'u', 'x', that need an unsigned integer argument of the
             specified size,
           - 'p', that needs a 'void *' argument,
           - 'H', that needs a 'location_t *' argument,
           - 'J', that needs a general declaration argument,
           - 'K', that needs a statement argument,
             [see gcc/pretty-print.c]

           - 'D', that needs a general declaration argument,
           - 'F', that needs a function declaration argument,
           - 'T', that needs a type argument,
           - 'E', that needs an expression argument,
             [see gcc/c-objc-common.c and gcc/cp/error.c]

           - 'A', that needs a function argument list argument,
           - 'C', that needs a tree code argument,
           - 'L', that needs a language argument,
           - 'O', that needs a binary operator argument,
           - 'P', that needs a function parameter argument,
           - 'Q', that needs an assignment operator argument,
           - 'V', that needs a const/volatile qualifier argument.
             [see gcc/cp/error.c]

   Numbered ('%m$' or '*m$') and unnumbered argument specifications cannot
   be used in the same string.  */

enum format_arg_type
{
  FAT_NONE              = 0,
  /* Basic types */
  FAT_INTEGER           = 1,
  FAT_CHAR              = 2,
  FAT_STRING            = 3,
  FAT_POINTER           = 4,
  FAT_LOCATION          = 5,
  FAT_TREE              = 6,
  FAT_TREE_CODE         = 7,
  FAT_LANGUAGES         = 8,
  /* Flags */
  FAT_UNSIGNED          = 1 << 4,
  FAT_SIZE_LONG         = 1 << 5,
  FAT_SIZE_LONGLONG     = 2 << 5,
  FAT_SIZE_WIDE         = 3 << 5,
  FAT_TREE_DECL         = 1 << 7,
  FAT_TREE_STATEMENT    = 2 << 7,
  FAT_TREE_FUNCDECL     = 3 << 7,
  FAT_TREE_TYPE         = 4 << 7,
  FAT_TREE_ARGUMENT     = 5 << 7,
  FAT_TREE_EXPRESSION   = 6 << 7,
  FAT_TREE_CV           = 7 << 7,
  FAT_TREE_CODE_BINOP   = 1 << 10,
  FAT_TREE_CODE_ASSOP   = 2 << 10,
  FAT_FUNCPARAM         = 1 << 12,
  /* Bitmasks */
  FAT_SIZE_MASK         = (FAT_SIZE_LONG | FAT_SIZE_LONGLONG | FAT_SIZE_WIDE)
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
  bool uses_err_no;
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
  spec.uses_err_no = false;
  numbered_allocated = 0;
  unnumbered_arg_count = 0;

  for (; *format != '\0';)
    /* Invariant: spec.numbered_arg_count == 0 || unnumbered_arg_count == 0.  */
    if (*format++ == '%')
      {
        /* A directive.  */
        FDI_SET (format - 1, FMTDIR_START);
        spec.directives++;

        if (*format == '%' || *format == '<' || *format == '>'
            || *format == '\'')
          ;
        else if (*format == 'm')
          spec.uses_err_no = true;
        else
          {
            unsigned int number = 0;
            unsigned int flag_q = 0;
            unsigned int flag_l = 0;
            unsigned int flag_w = 0;
            unsigned int flag_plus = 0;
            unsigned int flag_sharp = 0;
            format_arg_type_t size;
            format_arg_type_t type;

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

            /* Parse flags and size.  */
            for (;; format++)
              {
                switch (*format)
                  {
                  case 'q':
                    if (flag_q > 0)
                      goto invalid_flags;
                    flag_q = 1;
                    continue;
                  case 'l':
                    if (flag_l > 1 || flag_w)
                      goto invalid_flags;
                    flag_l++;
                    continue;
                  case 'w':
                    if (flag_w > 0 || flag_l)
                      goto invalid_flags;
                    flag_w = 1;
                    continue;
                  case '+':
                    if (flag_plus > 0)
                      goto invalid_flags;
                    flag_plus = 1;
                    continue;
                  case '#':
                    if (flag_sharp > 0)
                      goto invalid_flags;
                    flag_sharp = 1;
                    continue;
                  invalid_flags:
                    *invalid_reason = xasprintf (_("In the directive number %u, the flags combination is invalid."), spec.directives);
                    FDI_SET (format, FMTDIR_ERROR);
                    goto bad_format;
                  default:
                    break;
                  }
                break;
              }
            size = (flag_l == 2 ? FAT_SIZE_LONGLONG :
                    flag_l == 1 ? FAT_SIZE_LONG :
                    flag_w ? FAT_SIZE_WIDE :
                    0);

            if (*format == 'c')
              type = FAT_CHAR;
            else if (*format == 's')
              type = FAT_STRING;
            else if (*format == '.')
              {
                format++;

                if (isdigit (*format))
                  {
                    do
                      format++;
                    while (isdigit (*format));

                    if (*format != 's')
                      {
                        if (*format == '\0')
                          {
                            *invalid_reason = INVALID_UNTERMINATED_DIRECTIVE ();
                            FDI_SET (format - 1, FMTDIR_ERROR);
                          }
                        else
                          {
                            *invalid_reason =
                              xasprintf (_("In the directive number %u, a precision is not allowed before '%c'."), spec.directives, *format);
                            FDI_SET (format, FMTDIR_ERROR);
                          }
                        goto bad_format;
                      }

                    type = FAT_STRING;
                  }
                else if (*format == '*')
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
                                *invalid_reason = INVALID_WIDTH_ARGNO_0 (spec.directives);
                                FDI_SET (f, FMTDIR_ERROR);
                                goto bad_format;
                              }
                            if (unnumbered_arg_count > 0 || number == 0)
                              {
                                *invalid_reason = INVALID_MIXES_NUMBERED_UNNUMBERED ();
                                FDI_SET (f, FMTDIR_ERROR);
                                goto bad_format;
                              }
                            if (m != number - 1)
                              {
                                *invalid_reason = xasprintf (_("In the directive number %u, the argument number for the precision must be equal to %u."), spec.directives, number - 1);
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
                        spec.numbered[spec.numbered_arg_count].number = precision_number;
                        spec.numbered[spec.numbered_arg_count].type = FAT_INTEGER;
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
                        unnumbered_arg_count++;
                      }

                    if (*format == 's')
                      type = FAT_STRING;
                    else
                      {
                        if (*format == '\0')
                          {
                            *invalid_reason = INVALID_UNTERMINATED_DIRECTIVE ();
                            FDI_SET (format - 1, FMTDIR_ERROR);
                          }
                        else
                          {
                            *invalid_reason =
                              xasprintf (_("In the directive number %u, a precision specification is not allowed before '%c'."), spec.directives, *format);
                            FDI_SET (format, FMTDIR_ERROR);
                          }
                        goto bad_format;
                      }
                  }
                else
                  {
                    *invalid_reason = xasprintf (_("In the directive number %u, the precision specification is invalid."), spec.directives);
                    FDI_SET (*format == '\0' ? format - 1 : format,
                             FMTDIR_ERROR);
                    goto bad_format;
                  }
              }
            else if (*format == 'i' || *format == 'd')
              type = FAT_INTEGER | size;
            else if (*format == 'o' || *format == 'u' || *format == 'x')
              type = FAT_INTEGER | FAT_UNSIGNED | size;
            else if (*format == 'p')
              type = FAT_POINTER;
            else if (*format == 'H')
              type = FAT_LOCATION;
            else if (*format == 'J')
              type = FAT_TREE | FAT_TREE_DECL;
            else if (*format == 'K')
              type = FAT_TREE | FAT_TREE_STATEMENT;
            else
              {
                if (*format == 'D')
                  type = FAT_TREE | FAT_TREE_DECL;
                else if (*format == 'F')
                  type = FAT_TREE | FAT_TREE_FUNCDECL;
                else if (*format == 'T')
                  type = FAT_TREE | FAT_TREE_TYPE;
                else if (*format == 'E')
                  type = FAT_TREE | FAT_TREE_EXPRESSION;
                else if (*format == 'A')
                  type = FAT_TREE | FAT_TREE_ARGUMENT;
                else if (*format == 'C')
                  type = FAT_TREE_CODE;
                else if (*format == 'L')
                  type = FAT_LANGUAGES;
                else if (*format == 'O')
                  type = FAT_TREE_CODE | FAT_TREE_CODE_BINOP;
                else if (*format == 'P')
                  type = FAT_INTEGER | FAT_FUNCPARAM;
                else if (*format == 'Q')
                  type = FAT_TREE_CODE | FAT_TREE_CODE_ASSOP;
                else if (*format == 'V')
                  type = FAT_TREE | FAT_TREE_CV;
                else
                  {
                    if (*format == '\0')
                      {
                        *invalid_reason = INVALID_UNTERMINATED_DIRECTIVE ();
                        FDI_SET (format - 1, FMTDIR_ERROR);
                      }
                    else
                      {
                        *invalid_reason =
                          (*format == 'c'
                           || *format == 's'
                           || *format == 'i' || *format == 'd'
                           || *format == 'o' || *format == 'u' || *format == 'x'
                           || *format == 'H'
                           ? xasprintf (_("In the directive number %u, flags are not allowed before '%c'."), spec.directives, *format)
                           : INVALID_CONVERSION_SPECIFIER (spec.directives,
                                                           *format));
                        FDI_SET (format, FMTDIR_ERROR);
                      }
                    goto bad_format;
                  }
              }

            if (number)
              {
                /* Numbered argument.  */

                /* Numbered and unnumbered specifications are exclusive.  */
                if (unnumbered_arg_count > 0)
                  {
                    *invalid_reason = INVALID_MIXES_NUMBERED_UNNUMBERED ();
                    FDI_SET (format, FMTDIR_ERROR);
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
                    FDI_SET (format, FMTDIR_ERROR);
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

        FDI_SET (format, FMTDIR_END);

        format++;
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
            format_arg_type_t type1 = spec.numbered[i].type;
            format_arg_type_t type2 = spec.numbered[j-1].type;
            format_arg_type_t type_both;

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

  /* Check that the use of err_no is the same.  */
  if (spec1->uses_err_no != spec2->uses_err_no)
    {
      if (error_logger)
        {
          if (spec1->uses_err_no)
            error_logger (_("'%s' uses %%m but '%s' doesn't"),
                          pretty_msgid, pretty_msgstr);
          else
            error_logger (_("'%s' does not use %%m but '%s' uses %%m"),
                          pretty_msgid, pretty_msgstr);
        }
      err = true;
    }

  return err;
}


struct formatstring_parser formatstring_gcc_internal =
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
        case FAT_SIZE_LONG:
          printf ("[long]");
          break;
        case FAT_SIZE_LONGLONG:
          printf ("[long long]");
          break;
        case FAT_SIZE_WIDE:
          printf ("[host-wide]");
          break;
        default:
          abort ();
        }
      switch (spec->numbered[i].type & ~(FAT_UNSIGNED | FAT_SIZE_MASK))
        {
        case FAT_INTEGER:
          printf ("i");
          break;
        case FAT_INTEGER | FAT_FUNCPARAM:
          printf ("P");
          break;
        case FAT_CHAR:
          printf ("c");
          break;
        case FAT_STRING:
          printf ("s");
          break;
        case FAT_POINTER:
          printf ("p");
          break;
        case FAT_LOCATION:
          printf ("H");
          break;
        case FAT_TREE | FAT_TREE_DECL:
          printf ("D");
          break;
        case FAT_TREE | FAT_TREE_STATEMENT:
          printf ("K");
          break;
        case FAT_TREE | FAT_TREE_FUNCDECL:
          printf ("F");
          break;
        case FAT_TREE | FAT_TREE_TYPE:
          printf ("T");
          break;
        case FAT_TREE | FAT_TREE_ARGUMENT:
          printf ("A");
          break;
        case FAT_TREE | FAT_TREE_EXPRESSION:
          printf ("E");
          break;
        case FAT_TREE | FAT_TREE_CV:
          printf ("V");
          break;
        case FAT_TREE_CODE:
          printf ("C");
          break;
        case FAT_TREE_CODE | FAT_TREE_CODE_BINOP:
          printf ("O");
          break;
        case FAT_TREE_CODE | FAT_TREE_CODE_ASSOP:
          printf ("Q");
          break;
        case FAT_LANGUAGES:
          printf ("L");
          break;
        default:
          abort ();
        }
      last = number + 1;
    }
  printf (")");
  if (spec->uses_err_no)
    printf (" ERR_NO");
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
 * compile-command: "/bin/sh ../libtool --tag=CC --mode=link gcc -o a.out -static -O -g -Wall -I.. -I../gnulib-lib -I../../gettext-runtime/intl -DHAVE_CONFIG_H -DTEST format-gcc-internal.c ../gnulib-lib/libgettextlib.la"
 * End:
 */

#endif /* TEST */
