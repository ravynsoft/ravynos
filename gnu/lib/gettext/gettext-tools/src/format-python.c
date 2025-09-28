/* Python format strings.
   Copyright (C) 2001-2004, 2006-2009, 2019-2020, 2023 Free Software Foundation, Inc.
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
#include <string.h>

#include "format.h"
#include "c-ctype.h"
#include "xalloc.h"
#include "xvasprintf.h"
#include "format-invalid.h"
#include "gettext.h"

#define _(str) gettext (str)

/* Python format strings are described in
     Python Library reference
     2. Built-in Types, Exceptions and Functions
     2.1. Built-in Types
     2.1.5. Sequence Types
     2.1.5.2. String Formatting Operations
   Any string or Unicode string can act as format string via the '%' operator,
   implemented in stringobject.c and unicodeobject.c.
   A directive
   - starts with '%'
   - is optionally followed by '(ident)' where ident is any sequence of
     characters with balanced left and right parentheses,
   - is optionally followed by any of the characters '-' (left justification),
     '+' (sign), ' ' (blank), '#' (alt), '0' (zero), each of which acts as a
     flag,
   - is optionally followed by a width specification: '*' (reads an argument)
     or a nonempty digit sequence,
   - is optionally followed by '.' and a precision specification: '*' (reads
     an argument) or a nonempty digit sequence,
   - is optionally followed by a size specifier, one of 'h' 'l' 'L'.
   - is finished by a specifier
       - '%', that needs no argument,
       - 'c', that needs a character argument,
       - 's', 'r', that need a string argument (or, when a precision of 0 is
         given, an argument of any type),
       - 'i', 'd', 'u', 'o', 'x', 'X', that need an integer argument,
       - 'e', 'E', 'f', 'g', 'G', that need a floating-point argument.
   Use of '(ident)' and use of unnamed argument specifications are exclusive,
   because the first requires a mapping as argument, while the second requires
   a tuple as argument. When unnamed arguments are used, the number of
   arguments in the format string and the number of elements in the argument
   tuple (to the right of the '%' operator) must be the same.
 */

enum format_arg_type
{
  FAT_NONE,
  FAT_ANY,
  FAT_CHARACTER,
  FAT_STRING,
  FAT_INTEGER,
  FAT_FLOAT
};

struct named_arg
{
  char *name;
  enum format_arg_type type;
};

struct unnamed_arg
{
  enum format_arg_type type;
};

struct spec
{
  unsigned int directives;
  unsigned int named_arg_count;
  unsigned int unnamed_arg_count;
  struct named_arg *named;
  struct unnamed_arg *unnamed;
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

#define INVALID_MIXES_NAMED_UNNAMED() \
  xstrdup (_("The string refers to arguments both through argument names and through unnamed argument specifications."))

static void *
format_parse (const char *format, bool translated, char *fdi,
              char **invalid_reason)
{
  const char *const format_start = format;
  struct spec spec;
  unsigned int allocated;
  struct spec *result;

  spec.directives = 0;
  spec.named_arg_count = 0;
  spec.unnamed_arg_count = 0;
  spec.named = NULL;
  spec.unnamed = NULL;
  allocated = 0;

  for (; *format != '\0';)
    if (*format++ == '%')
      {
        /* A directive.  */
        char *name = NULL;
        bool zero_precision = false;
        enum format_arg_type type;

        FDI_SET (format - 1, FMTDIR_START);
        spec.directives++;

        if (*format == '(')
          {
            unsigned int depth;
            const char *name_start;
            const char *name_end;
            size_t n;

            name_start = ++format;
            depth = 0;
            for (; *format != '\0'; format++)
              {
                if (*format == '(')
                  depth++;
                else if (*format == ')')
                  {
                    if (depth == 0)
                      break;
                    else
                      depth--;
                  }
              }
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
          }

        while (*format == '-' || *format == '+' || *format == ' '
               || *format == '#' || *format == '0')
          format++;

        if (*format == '*')
          {
            format++;

            /* Named and unnamed specifications are exclusive.  */
            if (spec.named_arg_count > 0)
              {
                *invalid_reason = INVALID_MIXES_NAMED_UNNAMED ();
                FDI_SET (format - 1, FMTDIR_ERROR);
                goto bad_format;
              }

            if (allocated == spec.unnamed_arg_count)
              {
                allocated = 2 * allocated + 1;
                spec.unnamed = (struct unnamed_arg *) xrealloc (spec.unnamed, allocated * sizeof (struct unnamed_arg));
              }
            spec.unnamed[spec.unnamed_arg_count].type = FAT_INTEGER;
            spec.unnamed_arg_count++;
          }
        else if (isdigit (*format))
          {
            do format++; while (isdigit (*format));
          }

        if (*format == '.')
          {
            format++;

            if (*format == '*')
              {
                format++;

                /* Named and unnamed specifications are exclusive.  */
                if (spec.named_arg_count > 0)
                  {
                    *invalid_reason = INVALID_MIXES_NAMED_UNNAMED ();
                    FDI_SET (format - 1, FMTDIR_ERROR);
                    goto bad_format;
                  }

                if (allocated == spec.unnamed_arg_count)
                  {
                    allocated = 2 * allocated + 1;
                    spec.unnamed = (struct unnamed_arg *) xrealloc (spec.unnamed, allocated * sizeof (struct unnamed_arg));
                  }
                spec.unnamed[spec.unnamed_arg_count].type = FAT_INTEGER;
                spec.unnamed_arg_count++;
              }
            else if (isdigit (*format))
              {
                zero_precision = true;
                do
                  {
                    if (*format != '0')
                      zero_precision = false;
                    format++;
                  }
                while (isdigit (*format));
              }
          }

        if (*format == 'h' || *format == 'l' || *format == 'L')
          format++;

        switch (*format)
          {
          case '%':
            type = FAT_NONE;
            break;
          case 'c':
            type = FAT_CHARACTER;
            break;
          case 's': case 'r':
            type = (zero_precision ? FAT_ANY : FAT_STRING);
            break;
          case 'i': case 'd': case 'u': case 'o': case 'x': case 'X':
            type = FAT_INTEGER;
            break;
          case 'e': case 'E': case 'f': case 'g': case 'G':
            type = FAT_FLOAT;
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

        if (name != NULL)
          {
            /* Named argument.  */

            /* Named and unnamed specifications are exclusive.  */
            if (spec.unnamed_arg_count > 0)
              {
                *invalid_reason = INVALID_MIXES_NAMED_UNNAMED ();
                FDI_SET (format, FMTDIR_ERROR);
                goto bad_format;
              }

            if (allocated == spec.named_arg_count)
              {
                allocated = 2 * allocated + 1;
                spec.named = (struct named_arg *) xrealloc (spec.named, allocated * sizeof (struct named_arg));
              }
            spec.named[spec.named_arg_count].name = name;
            spec.named[spec.named_arg_count].type = type;
            spec.named_arg_count++;
          }
        else if (*format != '%')
          {
            /* Unnamed argument.  */

            /* Named and unnamed specifications are exclusive.  */
            if (spec.named_arg_count > 0)
              {
                *invalid_reason = INVALID_MIXES_NAMED_UNNAMED ();
                FDI_SET (format, FMTDIR_ERROR);
                goto bad_format;
              }

            if (allocated == spec.unnamed_arg_count)
              {
                allocated = 2 * allocated + 1;
                spec.unnamed = (struct unnamed_arg *) xrealloc (spec.unnamed, allocated * sizeof (struct unnamed_arg));
              }
            spec.unnamed[spec.unnamed_arg_count].type = type;
            spec.unnamed_arg_count++;
          }

        FDI_SET (format, FMTDIR_END);

        format++;
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
  if (spec.unnamed != NULL)
    free (spec.unnamed);
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
  if (spec->unnamed != NULL)
    free (spec->unnamed);
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

  if (spec1->named_arg_count > 0 && spec2->unnamed_arg_count > 0)
    {
      if (error_logger)
        error_logger (_("format specifications in '%s' expect a mapping, those in '%s' expect a tuple"),
                      pretty_msgid, pretty_msgstr);
      err = true;
    }
  else if (spec1->unnamed_arg_count > 0 && spec2->named_arg_count > 0)
    {
      if (error_logger)
        error_logger (_("format specifications in '%s' expect a tuple, those in '%s' expect a mapping"),
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
                    if (!(spec1->named[i].type == spec2->named[j].type
                          || (!equality
                              && (spec1->named[i].type == FAT_ANY
                                  || spec2->named[j].type == FAT_ANY))))
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

      if (spec1->unnamed_arg_count + spec2->unnamed_arg_count > 0)
        {
          unsigned int i;

          /* Check the argument types are the same.  */
          if (spec1->unnamed_arg_count != spec2->unnamed_arg_count)
            {
              if (error_logger)
                error_logger (_("number of format specifications in '%s' and '%s' does not match"),
                              pretty_msgid, pretty_msgstr);
              err = true;
            }
          else
            for (i = 0; i < spec2->unnamed_arg_count; i++)
              if (!(spec1->unnamed[i].type == spec2->unnamed[i].type
                    || (!equality
                        && (spec1->unnamed[i].type == FAT_ANY
                            || spec2->unnamed[i].type == FAT_ANY))))
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


struct formatstring_parser formatstring_python =
{
  format_parse,
  format_free,
  format_get_number_of_directives,
  NULL,
  format_check
};


unsigned int
get_python_format_unnamed_arg_count (const char *string)
{
  /* Parse the format string.  */
  char *invalid_reason = NULL;
  struct spec *descr =
    (struct spec *) format_parse (string, false, NULL, &invalid_reason);

  if (descr != NULL)
    {
      unsigned int result = descr->unnamed_arg_count;

      format_free (descr);
      return result;
    }
  else
    {
      free (invalid_reason);
      return 0;
    }
}


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
      if (spec->unnamed_arg_count > 0)
        abort ();

      printf ("{");
      for (i = 0; i < spec->named_arg_count; i++)
        {
          if (i > 0)
            printf (", ");
          printf ("'%s':", spec->named[i].name);
          switch (spec->named[i].type)
            {
            case FAT_ANY:
              printf ("*");
              break;
            case FAT_CHARACTER:
              printf ("c");
              break;
            case FAT_STRING:
              printf ("s");
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
      printf ("}");
    }
  else
    {
      printf ("(");
      for (i = 0; i < spec->unnamed_arg_count; i++)
        {
          if (i > 0)
            printf (" ");
          switch (spec->unnamed[i].type)
            {
            case FAT_ANY:
              printf ("*");
              break;
            case FAT_CHARACTER:
              printf ("c");
              break;
            case FAT_STRING:
              printf ("s");
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
 * compile-command: "/bin/sh ../libtool --tag=CC --mode=link gcc -o a.out -static -O -g -Wall -I.. -I../gnulib-lib -I../../gettext-runtime/intl -DHAVE_CONFIG_H -DTEST format-python.c ../gnulib-lib/libgettextlib.la"
 * End:
 */

#endif /* TEST */
