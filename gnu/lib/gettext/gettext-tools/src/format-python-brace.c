/* Python brace format strings.
   Copyright (C) 2004, 2006-2007, 2013-2014, 2016, 2019, 2023 Free Software Foundation, Inc.
   Written by Daiki Ueno <ueno@gnu.org>, 2013.

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

/* Python brace format strings are defined by PEP3101 together with the
   'format' method of the string class.
   Documentation:
     https://peps.python.org/pep-3101/
     https://docs.python.org/3/library/string.html#formatstrings
   A format string directive here consists of
     - an opening brace '{',
     - an identifier [_A-Za-z][_0-9A-Za-z]*|[0-9]+,
     - an optional sequence of
         - getattr ('.' identifier) or
         - getitem ('[' identifier ']')
       operators,
     - optionally, a ':' and a format specifier, where a format specifier is
       - either a format directive of the form '{' ... '}' without a format
         specifier, or
       - of the form [[fill]align][sign][#][0][minimumwidth][.precision][type]
         where
           - the fill character is any character,
           - the align flag is one of '<', '>', '=', '^',
           - the sign is one of '+', '-', ' ',
           - the # flag is '#',
           - the 0 flag is '0',
           - minimumwidth is a non-empty sequence of digits,
           - precision is a non-empty sequence of digits,
           - type is one of
             - 'b', 'c', 'd', 'o', 'x', 'X', 'n' for integers,
             - 'e', 'E', 'f', 'F', 'g', 'G', 'n', '%' for floating-point values,
     - a closing brace '}'.
   Brace characters '{' and '}' can be escaped by doubling them: '{{' and '}}'.
*/

struct named_arg
{
  char *name;
};

struct spec
{
  unsigned int directives;
  unsigned int named_arg_count;
  unsigned int allocated;
  struct named_arg *named;
};


/* Forward declaration of local functions.  */
static void free_named_args (struct spec *spec);


/* All the parse_* functions (except parse_upto) follow the same
   calling convention.  FORMATP shall point to the beginning of a token.
   If parsing succeeds, FORMATP will point to the next character after
   the token, and true is returned.  Otherwise, FORMATP will be
   unchanged and false is returned.  */

static bool
parse_named_field (struct spec *spec,
                   const char **formatp, bool translated, char *fdi,
                   char **invalid_reason)
{
  const char *format = *formatp;
  char c;

  c = *format;
  if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_')
    {
      do
        c = *++format;
      while ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_'
             || (c >= '0' && c <= '9'));
      *formatp = format;
      return true;
    }
  return false;
}

static bool
parse_numeric_field (struct spec *spec,
                     const char **formatp, bool translated, char *fdi,
                     char **invalid_reason)
{
  const char *format = *formatp;
  char c;

  c = *format;
  if (c >= '0' && c <= '9')
    {
      do
        c = *++format;
      while (c >= '0' && c <= '9');
      *formatp = format;
      return true;
    }
  return false;
}

/* Parses a directive.
   When this function is invoked, *formatp points to the start of the directive,
   i.e. to the '{' character.
   When this function returns true, *formatp points to the first character after
   the directive, i.e. in most cases to the character after the '}' character.
 */
static bool
parse_directive (struct spec *spec,
                 const char **formatp, bool is_toplevel,
                 bool translated, char *fdi, char **invalid_reason)
{
  const char *format = *formatp;
  const char *const format_start = format;
  const char *name_start;
  char c;

  c = *++format;
  if (c == '{')
    {
      /* An escaped '{'.  */
      *formatp = ++format;
      return true;
    }

  name_start = format;
  if (!parse_named_field (spec, &format, translated, fdi, invalid_reason)
      && !parse_numeric_field (spec, &format, translated, fdi, invalid_reason))
    {
      *invalid_reason =
        xasprintf (_("In the directive number %u, '%c' cannot start a field name."),
                   spec->directives, *format);
      FDI_SET (format, FMTDIR_ERROR);
      return false;
    }

  /* Parse '.' (getattr) or '[..]' (getitem) operators followed by a
     name.  If must not recurse, but can be specifed in a chain, such
     as "foo.bar.baz[0]".  */
  for (;;)
    {
      c = *format;

      if (c == '.')
        {
          format++;
          if (!parse_named_field (spec, &format, translated, fdi,
                                  invalid_reason))
            {
              *invalid_reason =
                xasprintf (_("In the directive number %u, '%c' cannot start a getattr argument."),
                           spec->directives, *format);
              FDI_SET (format, FMTDIR_ERROR);
              return false;
            }
        }
      else if (c == '[')
        {
          format++;
          if (!parse_named_field (spec, &format, translated, fdi,
                                  invalid_reason)
              && !parse_numeric_field (spec, &format, translated, fdi,
                                       invalid_reason))
            {
              *invalid_reason =
                xasprintf (_("In the directive number %u, '%c' cannot start a getitem argument."),
                           spec->directives, *format);
              FDI_SET (format, FMTDIR_ERROR);
              return false;
            }

          if (*format != ']')
            {
              *invalid_reason =
                xasprintf (_("In the directive number %u, there is an unterminated getitem argument."),
                           spec->directives);
              FDI_SET (format, FMTDIR_ERROR);
              return false;
            }
          format++;
        }
      else
        break;
    }

  /* Here c == *format.  */
  if (c == ':')
    {
      if (!is_toplevel)
        {
          *invalid_reason =
            xasprintf (_("In the directive number %u, no more nesting is allowed in a format specifier."),
                       spec->directives);
          FDI_SET (format, FMTDIR_ERROR);
          return false;
        }

      format++;

      /* Format specifiers.  Although a format specifier can be any
         string in theory, we can only recognize two types of format
         specifiers below, because otherwise we would need to evaluate
         Python expressions by ourselves:

           - A nested format directive expanding to an argument
           - The Standard Format Specifiers, as described in PEP3101,
             not including a nested format directive  */
      if (*format == '{')
        {
          /* Nested format directive.  */
          if (!parse_directive (spec, &format, false, translated, fdi,
                                invalid_reason))
            {
              /* FDI and INVALID_REASON will be set by a recursive call of
                 parse_directive.  */
              return false;
            }
        }
      else
        {
          /* Standard format specifiers is in the form:
             [[fill]align][sign][#][0][minimumwidth][.precision][type]  */

          /* Look ahead two characters to skip [[fill]align].  */
          int c1, c2;

          c1 = format[0];
          if (c1 == '\0')
            {
              *invalid_reason =
                xasprintf (_("In the directive number %u, there is an unterminated format directive."),
                           spec->directives);
              FDI_SET (format, FMTDIR_ERROR);
              return false;
            }

          c2 = format[1];

          if (c2 == '<' || c2 == '>' || c2 == '=' || c2 == '^')
            format += 2;
          else if (c1 == '<' || c1 == '>' || c1 == '=' || c1 == '^')
            format++;

          if (*format == '+' || *format == '-' || *format == ' ')
            format++;
          if (*format == '#')
            format++;
          if (*format == '0')
            format++;

          /* Parse the optional minimumwidth.  */
          while (c_isdigit (*format))
            format++;

          /* Parse the optional .precision.  */
          if (*format == '.')
            {
              format++;
              if (c_isdigit (*format))
                do
                  format++;
                while (c_isdigit (*format));
              else
                format--;
            }

          switch (*format)
            {
            case 'b': case 'c': case 'd': case 'o': case 'x': case 'X':
            case 'n':
            case 'e': case 'E': case 'f': case 'F': case 'g': case 'G':
            case '%':
              format++;
              break;
            default:
              break;
            }
        }
    }

  if (*format != '}')
    {
      *invalid_reason =
        xasprintf (_("In the directive number %u, there is an unterminated format directive."),
                   spec->directives);
      FDI_SET (format, FMTDIR_ERROR);
      return false;
    }

  if (is_toplevel)
    {
      char *name;
      size_t n = format - name_start;

      FDI_SET (name_start - 1, FMTDIR_START);

      name = XNMALLOC (n + 1, char);
      memcpy (name, name_start, n);
      name[n] = '\0';

      spec->directives++;

      if (spec->allocated == spec->named_arg_count)
        {
          spec->allocated = 2 * spec->allocated + 1;
          spec->named = (struct named_arg *) xrealloc (spec->named, spec->allocated * sizeof (struct named_arg));
        }
      spec->named[spec->named_arg_count].name = name;
      spec->named_arg_count++;

      FDI_SET (format, FMTDIR_END);
    }

  *formatp = ++format;
  return true;
}

static bool
parse_upto (struct spec *spec,
            const char **formatp, bool is_toplevel, char terminator,
            bool translated, char *fdi, char **invalid_reason)
{
  const char *format = *formatp;

  for (; *format != terminator && *format != '\0';)
    {
      if (*format == '{')
        {
          if (!parse_directive (spec, &format, is_toplevel, translated, fdi,
                                invalid_reason))
            return false;
        }
      else
        format++;
    }

  *formatp = format;
  return true;
}

static int
named_arg_compare (const void *p1, const void *p2)
{
  return strcmp (((const struct named_arg *) p1)->name,
                 ((const struct named_arg *) p2)->name);
}

static void *
format_parse (const char *format, bool translated, char *fdi,
              char **invalid_reason)
{
  struct spec spec;
  struct spec *result;

  spec.directives = 0;
  spec.named_arg_count = 0;
  spec.allocated = 0;
  spec.named = NULL;

  if (!parse_upto (&spec, &format, true, '\0', translated, fdi, invalid_reason))
    {
      free_named_args (&spec);
      return NULL;
    }

  /* Sort the named argument array, and eliminate duplicates.  */
  if (spec.named_arg_count > 1)
    {
      unsigned int i, j;

      qsort (spec.named, spec.named_arg_count, sizeof (struct named_arg),
             named_arg_compare);

      /* Remove duplicates: Copy from i to j, keeping 0 <= j <= i.  */
      for (i = j = 0; i < spec.named_arg_count; i++)
        if (j > 0 && strcmp (spec.named[i].name, spec.named[j-1].name) == 0)
          free (spec.named[i].name);
        else
          {
            if (j < i)
              spec.named[j].name = spec.named[i].name;
            j++;
          }
      spec.named_arg_count = j;
    }

  result = XMALLOC (struct spec);
  *result = spec;
  return result;
}

static void
free_named_args (struct spec *spec)
{
  if (spec->named != NULL)
    {
      unsigned int i;
      for (i = 0; i < spec->named_arg_count; i++)
        free (spec->named[i].name);
      free (spec->named);
    }
}

static void
format_free (void *descr)
{
  struct spec *spec = (struct spec *) descr;

  free_named_args (spec);
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

  if (spec1->named_arg_count + spec2->named_arg_count > 0)
    {
      unsigned int i, j;
      unsigned int n1 = spec1->named_arg_count;
      unsigned int n2 = spec2->named_arg_count;

      /* Check the argument names in spec1 are contained in those of spec2.
         Both arrays are sorted.  We search for the differences.  */
      for (i = 0, j = 0; i < n1 || j < n2; )
        {
          int cmp = (i >= n1 ? 1 :
                     j >= n2 ? -1 :
                     strcmp (spec1->named[i].name, spec2->named[j].name));

          if (cmp > 0)
            {
              if (equality)
                {
                  if (error_logger)
                    error_logger (_("a format specification for argument '%s' doesn't exist in '%s'"),
                                  spec2->named[i].name, pretty_msgid);
                  err = true;
                  break;
                }
              else
                j++;
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
    }

  return err;
}


struct formatstring_parser formatstring_python_brace =
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

  printf ("{");
  for (i = 0; i < spec->named_arg_count; i++)
    {
      if (i > 0)
        printf (", ");
      printf ("'%s'", spec->named[i].name);
    }
  printf ("}");
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
 * compile-command: "/bin/sh ../libtool --tag=CC --mode=link gcc -o a.out -static -O -g -Wall -I.. -I../gnulib-lib -I../../gettext-runtime/intl -DHAVE_CONFIG_H -DTEST format-python-brace.c ../gnulib-lib/libgettextlib.la"
 * End:
 */

#endif /* TEST */
