/* Lua format strings.
   Copyright (C) 2012-2013, 2018-2020 Free Software Foundation, Inc.
   Written by Ľubomír Remák <lubomirr@lubomirr.eu>, 2012.

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
#include <config.h>
#endif

#include <stdbool.h>
#include <stdlib.h>

#include "format.h"
#include "gettext.h"
#include "xalloc.h"
#include "format-invalid.h"
#include "c-ctype.h"
#include "xvasprintf.h"

#define _(str) gettext (str)

/* The Lua format strings are described in the Lua manual,
   which can be found at:
   https://www.lua.org/manual/5.2/manual.html

   A directive
   - starts with '%'
   - is optionally followed by any of the characters '0', '-', ' ', or
     each of which acts as a flag,
   - is optionally followed by a width specification: a nonempty digit
     sequence,
   - is optionally followed by '.' and a precision specification: a nonempty
     digit sequence,
   - is finished by a specifier
       - 's', 'q', that needs a string argument,
       - 'd', 'i', 'o', 'u', 'X', 'x', that need an integer argument,
       - 'A', 'a', 'E', 'e', 'f', 'G', 'g', that need a floating-point argument,
       - 'c', that needs a character argument.
   Additionally there is the directive '%%', which takes no argument.

   Note: Lua does not distinguish between integer, floating-point
   and character arguments, since it has a number data type only.
   However, we should not allow users to use %d instead of %c.
   The same applies to %s and %q - we should not allow intermixing them.
 */

enum format_arg_type
{
  FAT_INTEGER,
  FAT_CHARACTER,
  FAT_FLOAT,
  FAT_STRING,
  FAT_ESCAPED_STRING
};

struct spec
{
  unsigned int directives;
  unsigned int format_args_count;
  enum format_arg_type *format_args;
};

/* Locale independent test for a decimal digit.
   Argument can be  'char' or 'unsigned char'.  (Whereas the argument of
   <ctype.h> isdigit must be an 'unsigned char'.)  */
#undef isdigit
#define isdigit(c) ((unsigned int) ((c) - '0') < 10)

static void format_free (void *descr);

static void *
format_parse (const char *format, bool translated, char *fdi,
              char **invalid_reason)
{

  const char *format_start = format;
  const char *fatstr = format;
  struct spec *result = NULL;
  unsigned int format_args_allocated;

  result = XMALLOC (struct spec);
  result->directives = 0;
  result->format_args_count = 0;
  result->format_args = NULL;
  format_args_allocated = 0;

  for (; *fatstr != '\0';)
    {
      if (*fatstr++ == '%')
        {
          FDI_SET (fatstr - 1, FMTDIR_START);
          result->directives++;

          if (*fatstr != '%')
            {
              enum format_arg_type type;

              /* Remove width. */
              while (isdigit (*fatstr))
                fatstr++;

              if (*fatstr == '.')
                {
                  fatstr++;

                  /* Remove precision. */
                  while (isdigit (*fatstr))
                    fatstr++;
                }

              switch (*fatstr)
                {
                case 'c':
                  type = FAT_CHARACTER;
                  break;
                case 'd':
                case 'i':
                case 'o':
                case 'u':
                case 'X':
                case 'x':
                  type = FAT_INTEGER;
                  break;
                case 'a':
                case 'A':
                case 'E':
                case 'e':
                case 'f':
                case 'g':
                case 'G':
                  type = FAT_FLOAT;
                  break;
                case 's':
                  type = FAT_STRING;
                  break;
                case 'q':
                  type = FAT_ESCAPED_STRING;
                  break;
                default:
                  if (*fatstr == '\0')
                    {
                      *invalid_reason = INVALID_UNTERMINATED_DIRECTIVE ();
                      FDI_SET (fatstr - 1, FMTDIR_ERROR);
                    }
                  else
                    {
                      *invalid_reason =
                        INVALID_CONVERSION_SPECIFIER (result->
                                                      format_args_count + 1,
                                                      *fatstr);
                      FDI_SET (fatstr, FMTDIR_ERROR);
                    }
                  goto fmt_error;
                }

              if (result->format_args_count == format_args_allocated)
                {
                  format_args_allocated = 2 * format_args_allocated + 10;
                  result->format_args =
                    xrealloc (result->format_args,
                              format_args_allocated *
                              sizeof (enum format_arg_type));
                }
              result->format_args[result->format_args_count++] = type;
            }
          FDI_SET (fatstr, FMTDIR_END);
          fatstr++;
        }
    }

  return result;

fmt_error:
  format_free (result);
  return NULL;
}

static void
format_free (void *descr)
{
  struct spec *spec = (struct spec *) descr;

  if (spec->format_args != NULL)
    free (spec->format_args);
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

  if (spec1->format_args_count + spec2->format_args_count > 0)
    {
      unsigned int i, n1, n2;

      n1 = spec1->format_args_count;
      n2 = spec2->format_args_count;

      for (i = 0; i < n1 || i < n2; i++)
        {
          if (i >= n1)
            {
              if (error_logger)
                error_logger (_("a format specification for argument %u, as in '%s', doesn't exist in '%s'"),
                              i + 1, pretty_msgstr, pretty_msgid);
              return true;
            }
          else if (i >= n2)
            {
              if (error_logger)
                error_logger (_("a format specification for argument %u doesn't exist in '%s'"),
                              i + 1, pretty_msgstr);
              return true;
            }
          else if (spec1->format_args[i] != spec2->format_args[i])
            {
              if (error_logger)
                error_logger (_("format specifications in '%s' and '%s' for argument %u are not the same"),
                              pretty_msgid, pretty_msgstr, i + 1);
              return true;
            }
        }
    }

  return false;
}

struct formatstring_parser formatstring_lua =
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
  for (i = 0; i < spec->format_args_count; i++)
    {
      if (i > 0)
        printf (" ");
      switch (spec->format_args[i])
        {
        case FAT_INTEGER:
          printf ("i");
          break;
        case FAT_FLOAT:
          printf ("f");
          break;
        case FAT_CHARACTER:
          printf ("c");
          break;
        case FAT_STRING:
          printf ("s");
          break;
        case FAT_ESCAPED_STRING:
          printf ("q");
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
 * compile-command: "/bin/sh ../libtool --tag=CC --mode=link gcc -o a.out -static -O -g -Wall -I.. -I../gnulib-lib -I../../gettext-runtime/intl -DHAVE_CONFIG_H -DTEST format-lua.c ../gnulib-lib/libgettextlib.la"
 * End:
 */

#endif /* TEST */
