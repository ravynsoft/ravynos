/* Java MessageFormat format strings.
   Copyright (C) 2001-2004, 2006-2007, 2009, 2019, 2023 Free Software Foundation, Inc.
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
#include <alloca.h>

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "format.h"
#include "c-ctype.h"
#include "xalloc.h"
#include "xmalloca.h"
#include "xvasprintf.h"
#include "format-invalid.h"
#include "gettext.h"

#define _(str) gettext (str)

/* Java MessageFormat format strings are described in
   java/text/MessageFormat.html.
   See also the ICU documentation class_MessageFormat.html.

   messageFormatPattern := string ( "{" messageFormatElement "}" string )*

   messageFormatElement := argument { "," elementFormat }

   elementFormat := "time" { "," datetimeStyle }
                  | "date" { "," datetimeStyle }
                  | "number" { "," numberStyle }
                  | "choice" { "," choiceStyle }

   datetimeStyle := "short"
                    | "medium"
                    | "long"
                    | "full"
                    | dateFormatPattern

   numberStyle := "currency"
                 | "percent"
                 | "integer"
                 | numberFormatPattern

   choiceStyle := choiceFormatPattern

   dateFormatPattern see SimpleDateFormat.applyPattern

   numberFormatPattern see DecimalFormat.applyPattern

   choiceFormatPattern see ChoiceFormat constructor

   In strings, literal curly braces can be used if quoted between single
   quotes.  A real single quote is represented by ''.

   If a pattern is used, then unquoted braces in the pattern, if any, must
   match: that is, "ab {0} de" and "ab '}' de" are ok, but "ab {0'}' de" and
   "ab } de" are not.

   The argument is a number from 0 to 9, which corresponds to the arguments
   presented in an array to be formatted.

   It is ok to have unused arguments in the array.

   Adding a dateFormatPattern / numberFormatPattern / choiceFormatPattern
   to an elementFormat is equivalent to creating a SimpleDateFormat /
   DecimalFormat / ChoiceFormat and use of setFormat. For example,

     MessageFormat form =
       new MessageFormat("The disk \"{1}\" contains {0,choice,0#no files|1#one file|2#{0,number} files}.");

   is equivalent to

     MessageFormat form = new MessageFormat("The disk \"{1}\" contains {0}.");
     form.setFormat(1, // Number of {} occurrence in the string!
                    new ChoiceFormat(new double[] { 0, 1, 2 },
                                     new String[] { "no files", "one file",
                                                    "{0,number} files" }));

   Note: The behaviour of quotes inside a choiceFormatPattern is not clear.
   Example 1:
     "abc{1,choice,0#{1,number,00';'000}}def"
       JDK 1.1.x: exception
       JDK 1.3.x: behaves like "abc{1,choice,0#{1,number,00;000}}def"
   Example 2:
     "abc{1,choice,0#{1,number,00';'}}def"
       JDK 1.1.x: interprets the semicolon as number suffix
       JDK 1.3.x: behaves like "abc{1,choice,0#{1,number,00;}}def"
 */

enum format_arg_type
{
  FAT_NONE,
  FAT_OBJECT,   /* java.lang.Object */
  FAT_NUMBER,   /* java.lang.Number */
  FAT_DATE      /* java.util.Date */
};

struct numbered_arg
{
  unsigned int number;
  enum format_arg_type type;
};

struct spec
{
  unsigned int directives;
  unsigned int numbered_arg_count;
  unsigned int allocated;
  struct numbered_arg *numbered;
};


/* Forward declaration of local functions.  */
static bool date_format_parse (const char *format);
static bool number_format_parse (const char *format);
static bool choice_format_parse (const char *format, struct spec *spec,
                                 char **invalid_reason);


/* Quote handling:
   - When we see a single-quote, ignore it, but toggle the quoting flag.
   - When we see a double single-quote, ignore the first of the two.
   Assumes local variables format, quoting.  */
#define HANDLE_QUOTE \
  if (*format == '\'' && *++format != '\'') \
    quoting = !quoting;

/* Note that message_format_parse and choice_format_parse are mutually
   recursive.  This is because MessageFormat can use some ChoiceFormats,
   and a ChoiceFormat is made up from several MessageFormats.  */

/* Return true if a format is a valid messageFormatPattern.
   Extracts argument type information into spec.  */
static bool
message_format_parse (const char *format, char *fdi, struct spec *spec,
                      char **invalid_reason)
{
  const char *const format_start = format;
  bool quoting = false;

  for (;;)
    {
      HANDLE_QUOTE;
      if (!quoting && *format == '{')
        {
          unsigned int depth;
          const char *element_start;
          const char *element_end;
          size_t n;
          char *element_alloced;
          char *element;
          unsigned int number;
          enum format_arg_type type;

          FDI_SET (format, FMTDIR_START);
          spec->directives++;

          element_start = ++format;
          depth = 0;
          for (; *format != '\0'; format++)
            {
              if (*format == '{')
                depth++;
              else if (*format == '}')
                {
                  if (depth == 0)
                    break;
                  else
                    depth--;
                }
            }
          if (*format == '\0')
            {
              *invalid_reason =
                xstrdup (_("The string ends in the middle of a directive: found '{' without matching '}'."));
              FDI_SET (format - 1, FMTDIR_ERROR);
              return false;
            }
          element_end = format++;

          n = element_end - element_start;
          element = element_alloced = (char *) xmalloca (n + 1);
          memcpy (element, element_start, n);
          element[n] = '\0';

          if (!c_isdigit (*element))
            {
              *invalid_reason =
                xasprintf (_("In the directive number %u, '{' is not followed by an argument number."), spec->directives);
              FDI_SET (format - 1, FMTDIR_ERROR);
              freea (element_alloced);
              return false;
            }
          number = 0;
          do
            {
              number = 10 * number + (*element - '0');
              element++;
            }
          while (c_isdigit (*element));

          type = FAT_OBJECT;
          if (*element == '\0')
            ;
          else if (strncmp (element, ",time", 5) == 0
                   || strncmp (element, ",date", 5) == 0)
            {
              type = FAT_DATE;
              element += 5;
              if (*element == '\0')
                ;
              else if (*element == ',')
                {
                  element++;
                  if (strcmp (element, "short") == 0
                      || strcmp (element, "medium") == 0
                      || strcmp (element, "long") == 0
                      || strcmp (element, "full") == 0
                      || date_format_parse (element))
                    ;
                  else
                    {
                      *invalid_reason =
                        xasprintf (_("In the directive number %u, the substring \"%s\" is not a valid date/time style."), spec->directives, element);
                      FDI_SET (format - 1, FMTDIR_ERROR);
                      freea (element_alloced);
                      return false;
                    }
                }
              else
                {
                  *element = '\0';
                  element -= 4;
                  *invalid_reason =
                    xasprintf (_("In the directive number %u, \"%s\" is not followed by a comma."), spec->directives, element);
                  FDI_SET (format - 1, FMTDIR_ERROR);
                  freea (element_alloced);
                  return false;
                }
            }
          else if (strncmp (element, ",number", 7) == 0)
            {
              type = FAT_NUMBER;
              element += 7;
              if (*element == '\0')
                ;
              else if (*element == ',')
                {
                  element++;
                  if (strcmp (element, "currency") == 0
                      || strcmp (element, "percent") == 0
                      || strcmp (element, "integer") == 0
                      || number_format_parse (element))
                    ;
                  else
                    {
                      *invalid_reason =
                        xasprintf (_("In the directive number %u, the substring \"%s\" is not a valid number style."), spec->directives, element);
                      FDI_SET (format - 1, FMTDIR_ERROR);
                      freea (element_alloced);
                      return false;
                    }
                }
              else
                {
                  *element = '\0';
                  element -= 6;
                  *invalid_reason =
                    xasprintf (_("In the directive number %u, \"%s\" is not followed by a comma."), spec->directives, element);
                  FDI_SET (format - 1, FMTDIR_ERROR);
                  freea (element_alloced);
                  return false;
                }
            }
          else if (strncmp (element, ",choice", 7) == 0)
            {
              type = FAT_NUMBER; /* because ChoiceFormat extends NumberFormat */
              element += 7;
              if (*element == '\0')
                ;
              else if (*element == ',')
                {
                  element++;
                  if (choice_format_parse (element, spec, invalid_reason))
                    ;
                  else
                    {
                      FDI_SET (format - 1, FMTDIR_ERROR);
                      freea (element_alloced);
                      return false;
                    }
                }
              else
                {
                  *element = '\0';
                  element -= 6;
                  *invalid_reason =
                    xasprintf (_("In the directive number %u, \"%s\" is not followed by a comma."), spec->directives, element);
                  FDI_SET (format - 1, FMTDIR_ERROR);
                  freea (element_alloced);
                  return false;
                }
            }
          else
            {
              *invalid_reason =
                xasprintf (_("In the directive number %u, the argument number is not followed by a comma and one of \"%s\", \"%s\", \"%s\", \"%s\"."), spec->directives, "time", "date", "number", "choice");
              FDI_SET (format - 1, FMTDIR_ERROR);
              freea (element_alloced);
              return false;
            }
          freea (element_alloced);

          if (spec->allocated == spec->numbered_arg_count)
            {
              spec->allocated = 2 * spec->allocated + 1;
              spec->numbered = (struct numbered_arg *) xrealloc (spec->numbered, spec->allocated * sizeof (struct numbered_arg));
            }
          spec->numbered[spec->numbered_arg_count].number = number;
          spec->numbered[spec->numbered_arg_count].type = type;
          spec->numbered_arg_count++;

          FDI_SET (format - 1, FMTDIR_END);
        }
      /* The doc says "ab}de" is invalid.  Even though JDK accepts it.  */
      else if (!quoting && *format == '}')
        {
          FDI_SET (format, FMTDIR_START);
          *invalid_reason =
            xstrdup (_("The string starts in the middle of a directive: found '}' without matching '{'."));
          FDI_SET (format, FMTDIR_ERROR);
          return false;
        }
      else if (*format != '\0')
        format++;
      else
        break;
    }

  return true;
}

/* Return true if a format is a valid dateFormatPattern.  */
static bool
date_format_parse (const char *format)
{
  /* Any string is valid.  Single-quote starts a quoted section, to be
     terminated at the next single-quote or string end.  Double single-quote
     gives a single single-quote.  Non-quoted ASCII letters are first grouped
     into blocks of equal letters.  Then each block (e.g. 'yyyy') is
     interpreted according to some rules.  */
  return true;
}

/* Return true if a format is a valid numberFormatPattern.  */
static bool
number_format_parse (const char *format)
{
  /* Pattern Syntax:
       pattern     := pos_pattern{';' neg_pattern}
       pos_pattern := {prefix}number{suffix}
       neg_pattern := {prefix}number{suffix}
       number      := integer{'.' fraction}{exponent}
       prefix      := '\u0000'..'\uFFFD' - special_characters
       suffix      := '\u0000'..'\uFFFD' - special_characters
       integer     := min_int | '#' | '#' integer | '#' ',' integer
       min_int     := '0' | '0' min_int | '0' ',' min_int
       fraction    := '0'* '#'*
       exponent    := 'E' '0' '0'*
     Notation:
       X*       0 or more instances of X
       { X }    0 or 1 instances of X
       X | Y    either X or Y
       X..Y     any character from X up to Y, inclusive
       S - T    characters in S, except those in T
     Single-quote starts a quoted section, to be terminated at the next
     single-quote or string end.  Double single-quote gives a single
     single-quote.
   */
  bool quoting = false;
  bool seen_semicolon = false;

  HANDLE_QUOTE;
  for (;;)
    {
      /* Parse prefix.  */
      while (*format != '\0'
             && !(!quoting && (*format == '0' || *format == '#')))
        {
          if (format[0] == '\\')
            {
              if (format[1] == 'u'
                  && c_isxdigit (format[2])
                  && c_isxdigit (format[3])
                  && c_isxdigit (format[4])
                  && c_isxdigit (format[5]))
                format += 6;
              else
                format += 2;
            }
          else
            format += 1;
          HANDLE_QUOTE;
        }

      /* Parse integer.  */
      if (!(!quoting && (*format == '0' || *format == '#')))
        return false;
      while (!quoting && *format == '#')
        {
          format++;
          HANDLE_QUOTE;
          if (!quoting && *format == ',')
            {
              format++;
              HANDLE_QUOTE;
            }
        }
      while (!quoting && *format == '0')
        {
          format++;
          HANDLE_QUOTE;
          if (!quoting && *format == ',')
            {
              format++;
              HANDLE_QUOTE;
            }
        }

      /* Parse fraction.  */
      if (!quoting && *format == '.')
        {
          format++;
          HANDLE_QUOTE;
          while (!quoting && *format == '0')
            {
              format++;
              HANDLE_QUOTE;
            }
          while (!quoting && *format == '#')
            {
              format++;
              HANDLE_QUOTE;
            }
        }

      /* Parse exponent.  */
      if (!quoting && *format == 'E')
        {
          const char *format_save = format;
          format++;
          HANDLE_QUOTE;
          if (!quoting && *format == '0')
            {
              do
                {
                  format++;
                  HANDLE_QUOTE;
                }
              while (!quoting && *format == '0');
            }
          else
            {
              /* Back up.  */
              format = format_save;
              quoting = false;
            }
        }

      /* Parse suffix.  */
      while (*format != '\0'
             && (seen_semicolon || !(!quoting && *format == ';')))
        {
          if (format[0] == '\\')
            {
              if (format[1] == 'u'
                  && c_isxdigit (format[2])
                  && c_isxdigit (format[3])
                  && c_isxdigit (format[4])
                  && c_isxdigit (format[5]))
                format += 6;
              else
                format += 2;
            }
          else
            format += 1;
          HANDLE_QUOTE;
        }

      if (seen_semicolon || !(!quoting && *format == ';'))
        break;
    }

  return (*format == '\0');
}

/* Return true if a format is a valid choiceFormatPattern.
   Extracts argument type information into spec.  */
static bool
choice_format_parse (const char *format, struct spec *spec,
                     char **invalid_reason)
{
  /* Pattern syntax:
       pattern   := | choice | choice '|' pattern
       choice    := number separator messageformat
       separator := '<' | '#' | '\u2264'
     Single-quote starts a quoted section, to be terminated at the next
     single-quote or string end.  Double single-quote gives a single
     single-quote.
   */
  bool quoting = false;

  HANDLE_QUOTE;
  if (*format == '\0')
    return true;
  for (;;)
    {
      /* Don't bother looking too precisely into the syntax of the number.
         It can contain various Unicode characters.  */
      bool number_nonempty;
      char *msgformat;
      char *mp;
      bool msgformat_valid;

      /* Parse number.  */
      number_nonempty = false;
      while (*format != '\0'
             && !(!quoting && (*format == '<' || *format == '#'
                               || strncmp (format, "\\u2264", 6) == 0
                               || *format == '|')))
        {
          if (format[0] == '\\')
            {
              if (format[1] == 'u'
                  && c_isxdigit (format[2])
                  && c_isxdigit (format[3])
                  && c_isxdigit (format[4])
                  && c_isxdigit (format[5]))
                format += 6;
              else
                format += 2;
            }
          else
            format += 1;
          number_nonempty = true;
          HANDLE_QUOTE;
        }

      /* Short clause at end of pattern is valid and is ignored!  */
      if (*format == '\0')
        break;

      if (!number_nonempty)
        {
          *invalid_reason =
            xasprintf (_("In the directive number %u, a choice contains no number."), spec->directives);
          return false;
        }

      if (*format == '<' || *format == '#')
        format += 1;
      else if (strncmp (format, "\\u2264", 6) == 0)
        format += 6;
      else
        {
          *invalid_reason =
            xasprintf (_("In the directive number %u, a choice contains a number that is not followed by '<', '#' or '%s'."), spec->directives, "\\u2264");
          return false;
        }
      HANDLE_QUOTE;

      msgformat = (char *) xmalloca (strlen (format) + 1);
      mp = msgformat;

      while (*format != '\0' && !(!quoting && *format == '|'))
        {
          *mp++ = *format++;
          HANDLE_QUOTE;
        }
      *mp = '\0';

      msgformat_valid =
        message_format_parse (msgformat, NULL, spec, invalid_reason);

      freea (msgformat);

      if (!msgformat_valid)
        return false;

      if (*format == '\0')
        break;

      format++;
      HANDLE_QUOTE;
    }

  return true;
}

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
  struct spec spec;
  struct spec *result;

  spec.directives = 0;
  spec.numbered_arg_count = 0;
  spec.allocated = 0;
  spec.numbered = NULL;

  if (!message_format_parse (format, fdi, &spec, invalid_reason))
    goto bad_format;

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

            if (type1 == type2 || type2 == FAT_OBJECT)
              type_both = type1;
            else if (type1 == FAT_OBJECT)
              type_both = type2;
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
                error_logger (_("a format specification for argument {%u}, as in '%s', doesn't exist in '%s'"),
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
                    error_logger (_("a format specification for argument {%u} doesn't exist in '%s'"),
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
                      error_logger (_("format specifications in '%s' and '%s' for argument {%u} are not the same"),
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


struct formatstring_parser formatstring_java =
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
  last = 0;
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
        case FAT_OBJECT:
          printf ("*");
          break;
        case FAT_NUMBER:
          printf ("Number");
          break;
        case FAT_DATE:
          printf ("Date");
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
 * compile-command: "/bin/sh ../libtool --tag=CC --mode=link gcc -o a.out -static -O -g -Wall -I.. -I../gnulib-lib -I../../gettext-runtime/intl -DHAVE_CONFIG_H -DTEST format-java.c ../gnulib-lib/libgettextlib.la"
 * End:
 */

#endif /* TEST */
