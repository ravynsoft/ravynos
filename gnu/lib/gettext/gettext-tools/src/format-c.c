/* C format strings.
   Copyright (C) 2001-2004, 2006-2007, 2009-2010, 2019, 2023 Free Software Foundation, Inc.
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
#include "gettext.h"

#define _(str) gettext (str)

#include "format-invalid.h"

#define INVALID_C99_MACRO(directive_number) \
  xasprintf (_("In the directive number %u, the token after '<' is not the name of a format specifier macro. The valid macro names are listed in ISO C 99 section 7.8.1."), directive_number)

#define INVALID_ANGLE_BRACKET(directive_number) \
  xasprintf (_("In the directive number %u, the token after '<' is not followed by '>'."), directive_number)

#define INVALID_SIZE_SPECIFIER(directive_number) \
  xasprintf (_("In the directive number %u, the argument size specifier is invalid."), directive_number)

#define INVALID_IGNORED_ARGUMENT(referenced_arg, ignored_arg) \
  xasprintf (_("The string refers to argument number %u but ignores argument number %u."), referenced_arg, ignored_arg)

/* Execute statement if memory allocation function returned NULL.  */
#define IF_OOM(allocated_ptr, statement)  /* nothing, since we use xalloc.h */

/* Specifies whether the system dependent segments in msgid and msgstr have
   been processed.  This means:
     - If false, ISO C 99 <inttypes.h> directives are denoted with angle
       brackets.  If true, they have already been expanded, leading in
       particular to %I64d directives on native Windows platforms.
     - If false, the 'I' flag may be present in msgstr (also on platforms
       other than glibc).  If true, the 'I' directive may be present in msgstr
       only on glibc >= 2.2 platforms.  */
#define SYSDEP_SEGMENTS_PROCESSED false

/* Include the bulk of the C format string parsing code.  */
#include "format-c-parse.h"

static void *
format_parse (const char *format, bool translated, bool objc_extensions,
              char *fdi, char **invalid_reason)
{
  struct spec result_buf;
  struct spec *result;

  result = format_parse_entrails (format, translated, objc_extensions, fdi, invalid_reason, &result_buf);

  if (result != NULL)
    {
      /* Copy the result to a heap-allocated object.  */
      struct spec *safe_result = XMALLOC (struct spec);
      *safe_result = *result;
      result = safe_result;
    }
  return result;
}

static void *
format_c_parse (const char *format, bool translated, char *fdi,
                char **invalid_reason)
{
  return format_parse (format, translated, false, fdi, invalid_reason);
}

static void *
format_objc_parse (const char *format, bool translated, char *fdi,
                   char **invalid_reason)
{
  return format_parse (format, translated, true, fdi, invalid_reason);
}

static void
format_free (void *descr)
{
  struct spec *spec = (struct spec *) descr;

  if (spec->unnumbered != NULL)
    free (spec->unnumbered);
  if (spec->sysdep_directives != NULL)
    free (spec->sysdep_directives);
  free (spec);
}

static bool
format_is_unlikely_intentional (void *descr)
{
  struct spec *spec = (struct spec *) descr;

  return spec->unlikely_intentional;
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
  unsigned int i;

  /* Check the argument types are the same.  */
  if (equality
      ? spec1->unnumbered_arg_count != spec2->unnumbered_arg_count
      : spec1->unnumbered_arg_count < spec2->unnumbered_arg_count)
    {
      if (error_logger)
        error_logger (_("number of format specifications in '%s' and '%s' does not match"),
                      pretty_msgid, pretty_msgstr);
      err = true;
    }
  else
    for (i = 0; i < spec2->unnumbered_arg_count; i++)
      if (spec1->unnumbered[i].type != spec2->unnumbered[i].type)
        {
          if (error_logger)
            error_logger (_("format specifications in '%s' and '%s' for argument %u are not the same"),
                          pretty_msgid, pretty_msgstr, i + 1);
          err = true;
        }

  return err;
}


struct formatstring_parser formatstring_c =
{
  format_c_parse,
  format_free,
  format_get_number_of_directives,
  format_is_unlikely_intentional,
  format_check
};


struct formatstring_parser formatstring_objc =
{
  format_objc_parse,
  format_free,
  format_get_number_of_directives,
  format_is_unlikely_intentional,
  format_check
};


void
get_sysdep_c_format_directives (const char *string, bool translated,
                                struct interval **intervalsp, size_t *lengthp)
{
  /* Parse the format string with all possible extensions turned on.  (The
     caller has already verified that the format string is valid for the
     particular language.)  */
  char *invalid_reason = NULL;
  struct spec *descr =
    (struct spec *)
    format_parse (string, translated, true, NULL, &invalid_reason);

  if (descr != NULL && descr->sysdep_directives_count > 0)
    {
      unsigned int n = descr->sysdep_directives_count;
      struct interval *intervals = XNMALLOC (n, struct interval);
      unsigned int i;

      for (i = 0; i < n; i++)
        {
          intervals[i].startpos = descr->sysdep_directives[2 * i] - string;
          intervals[i].endpos = descr->sysdep_directives[2 * i + 1] - string;
        }
      *intervalsp = intervals;
      *lengthp = n;
    }
  else
    {
      *intervalsp = NULL;
      *lengthp = 0;
    }

  if (descr != NULL)
    format_free (descr);
  else
    free (invalid_reason);
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

  printf ("(");
  for (i = 0; i < spec->unnumbered_arg_count; i++)
    {
      if (i > 0)
        printf (" ");
      if (spec->unnumbered[i].type & FAT_UNSIGNED)
        printf ("[unsigned]");
      switch (spec->unnumbered[i].type & FAT_SIZE_MASK)
        {
        case 0:
          break;
        case FAT_SIZE_SHORT:
          printf ("[short]");
          break;
        case FAT_SIZE_CHAR:
          printf ("[char]");
          break;
        case FAT_SIZE_LONG:
          printf ("[long]");
          break;
        case FAT_SIZE_LONGLONG:
          printf ("[long long]");
          break;
        case FAT_SIZE_8_T:
          printf ("[int8_t]");
          break;
        case FAT_SIZE_16_T:
          printf ("[int16_t]");
          break;
        case FAT_SIZE_32_T:
          printf ("[int32_t]");
          break;
        case FAT_SIZE_64_T:
          printf ("[int64_t]");
          break;
        case FAT_SIZE_LEAST8_T:
          printf ("[int_least8_t]");
          break;
        case FAT_SIZE_LEAST16_T:
          printf ("[int_least16_t]");
          break;
        case FAT_SIZE_LEAST32_T:
          printf ("[int_least32_t]");
          break;
        case FAT_SIZE_LEAST64_T:
          printf ("[int_least64_t]");
          break;
        case FAT_SIZE_FAST8_T:
          printf ("[int_fast8_t]");
          break;
        case FAT_SIZE_FAST16_T:
          printf ("[int_fast16_t]");
          break;
        case FAT_SIZE_FAST32_T:
          printf ("[int_fast32_t]");
          break;
        case FAT_SIZE_FAST64_T:
          printf ("[int_fast64_t]");
          break;
        case FAT_SIZE_INTMAX_T:
          printf ("[intmax_t]");
          break;
        case FAT_SIZE_INTPTR_T:
          printf ("[intptr_t]");
          break;
        case FAT_SIZE_SIZE_T:
          printf ("[size_t]");
          break;
        case FAT_SIZE_PTRDIFF_T:
          printf ("[ptrdiff_t]");
          break;
        default:
          abort ();
        }
      switch (spec->unnumbered[i].type & ~(FAT_UNSIGNED | FAT_SIZE_MASK))
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
        case FAT_OBJC_OBJECT:
          printf ("@");
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
      descr = format_c_parse (line, false, NULL, &invalid_reason);

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
 * compile-command: "/bin/sh ../libtool --tag=CC --mode=link gcc -o a.out -static -O -g -Wall -I.. -I../gnulib-lib -I../../gettext-runtime/intl -DHAVE_CONFIG_H -DTEST format-c.c ../gnulib-lib/libgettextlib.la"
 * End:
 */

#endif /* TEST */
