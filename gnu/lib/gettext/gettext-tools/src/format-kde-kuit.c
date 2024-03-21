/* KUIT (KDE User Interface Text) format strings.
   Copyright (C) 2015, 2018-2019 Free Software Foundation, Inc.
   Written by Daiki Ueno <ueno@gnu.org>, 2015.

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

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#include "format.h"
#include "unistr.h"
#include "xalloc.h"
#include "xvasprintf.h"
#include "gettext.h"

#if IN_LIBGETTEXTPO
/* Use included markup parser to avoid extra dependency from
   libgettextpo to libxml2.  */
# ifndef FORMAT_KDE_KUIT_FALLBACK_MARKUP
#  define FORMAT_KDE_KUIT_USE_FALLBACK_MARKUP 1
# endif
#else
#  define FORMAT_KDE_KUIT_USE_LIBXML2 1
#endif

#if FORMAT_KDE_KUIT_USE_LIBXML2
# include <libxml/parser.h>
#elif FORMAT_KDE_KUIT_USE_FALLBACK_MARKUP
# include "markup.h"
#endif


#define _(str) gettext (str)

#define SIZEOF(a) (sizeof(a) / sizeof(a[0]))


/* KUIT (KDE User Interface Text) is an XML-like markup which augments
   translatable strings with semantic information:
   https://api.kde.org/frameworks/ki18n/html/prg_guide.html#kuit_markup
   KUIT can be seen as a fragment of a well-formed XML document,
   except that it allows '&' as a Qt accelerator marker and '%' as a
   format directive.  */

struct spec
{
  /* A format string descriptor returned from formatstring_kde.parse.  */
  void *base;
};

#define XML_NS "https://www.gnu.org/s/gettext/kde"

struct char_range
{
  ucs4_t start;
  ucs4_t end;
};

/* Character ranges for NameStartChar defined in:
   https://www.w3.org/TR/REC-xml/#NT-NameStartChar  */
static const struct char_range name_chars1[] =
  {
    { ':', ':' },
    { 'A', 'Z' },
    { '_', '_' },
    { 'a', 'z' },
    { 0xC0, 0xD6 },
    { 0xD8, 0xF6 },
    { 0xF8, 0x2FF },
    { 0x370, 0x37D },
    { 0x37F, 0x1FFF },
    { 0x200C, 0x200D },
    { 0x2070, 0x218F },
    { 0x2C00, 0x2FEF },
    { 0x3001, 0xD7FF },
    { 0xF900, 0xFDCF },
    { 0xFDF0, 0xFFFD },
    { 0x10000, 0xEFFFF }
  };

/* Character ranges for NameChar, excluding NameStartChar:
   https://www.w3.org/TR/REC-xml/#NT-NameChar  */
static const struct char_range name_chars2[] =
  {
    { '-', '-' },
    { '.', '.' },
    { '0', '9' },
    { 0xB7, 0xB7 },
    { 0x0300, 0x036F },
    { 0x203F, 0x2040 }
  };

/* Return true if INPUT is an XML reference.  */
static bool
is_reference (const char *input)
{
  const char *str = input;
  const char *str_limit = str + strlen (input);
  ucs4_t uc;
  int i;

  str += u8_mbtouc (&uc, (const unsigned char *) str, str_limit - str);
  assert (uc == '&');

  str += u8_mbtouc (&uc, (const unsigned char *) str, str_limit - str);

  /* CharRef */
  if (uc == '#')
    {
      str += u8_mbtouc (&uc, (const unsigned char *) str, str_limit - str);
      if (uc == 'x')
        {
          while (str < str_limit)
            {
              str += u8_mbtouc (&uc, (const unsigned char *) str,
                                str_limit - str);
              if (!(('0' <= uc && uc <= '9')
                    || ('A' <= uc && uc <= 'F')
                    || ('a' <= uc && uc <= 'f')))
                break;
            }
          return uc == ';';
        }
      else if ('0' <= uc && uc <= '9')
        {
          while (str < str_limit)
            {
              str += u8_mbtouc (&uc, (const unsigned char *) str,
                                str_limit - str);
              if (!('0' <= uc && uc <= '9'))
                break;
            }
          return uc == ';';
        }
    }
  else
    {
      /* EntityRef */
      for (i = 0; i < SIZEOF (name_chars1); i++)
        if (name_chars1[i].start <= uc && uc <= name_chars1[i].end)
          break;

      if (i == SIZEOF (name_chars1))
        return false;

      while (str < str_limit)
        {
          str += u8_mbtouc (&uc, (const unsigned char *) str, str_limit - str);
          for (i = 0; i < SIZEOF (name_chars1); i++)
            if (name_chars1[i].start <= uc && uc <= name_chars1[i].end)
              break;
          if (i == SIZEOF (name_chars1))
            {
              for (i = 0; i < SIZEOF (name_chars2); i++)
                if (name_chars2[i].start <= uc && uc <= name_chars2[i].end)
                  break;
              if (i == SIZEOF (name_chars2))
                return false;
            }
        }
      return uc == ';';
    }

  return false;
}


static void *
format_parse (const char *format, bool translated, char *fdi,
              char **invalid_reason)
{
  struct spec spec;
  struct spec *result;
  const char *str;
  const char *str_limit;
  size_t amp_count;
  char *buffer, *bp;

  spec.base = NULL;

  /* Preprocess the input, putting the content in a <gt:kuit> element.  */
  str = format;
  str_limit = str + strlen (format);

  for (amp_count = 0; str < str_limit; amp_count++)
    {
      const char *amp = strchrnul (str, '&');
      if (*amp != '&')
        break;
      str = amp + 1;
    }

  buffer = xmalloc (amp_count * 4
                    + strlen (format)
                    + strlen ("<gt:kuit xmlns:gt=\"" XML_NS "\"></gt:kuit>")
                    + 1);
  *buffer = '\0';

  bp = buffer;
  bp = stpcpy (bp, "<gt:kuit xmlns:gt=\"" XML_NS "\">");
  str = format;
  while (str < str_limit)
    {
      const char *amp = strchrnul (str, '&');

      bp = stpncpy (bp, str, amp - str);
      if (*amp != '&')
        break;

      bp = stpcpy (bp, is_reference (amp) ? "&" : "&amp;");
      str = amp + 1;
    }
  stpcpy (bp, "</gt:kuit>");

#if FORMAT_KDE_KUIT_USE_LIBXML2
    {
      xmlDocPtr doc;

      doc = xmlReadMemory (buffer, strlen (buffer), "", NULL,
                           XML_PARSE_NONET
                           | XML_PARSE_NOWARNING
                           | XML_PARSE_NOERROR
                           | XML_PARSE_NOBLANKS);
      if (doc == NULL)
        {
          xmlError *err = xmlGetLastError ();
          *invalid_reason =
            xasprintf (_("error while parsing: %s"),
                       err->message);
          free (buffer);
          xmlFreeDoc (doc);
          return NULL;
        }

      free (buffer);
      xmlFreeDoc (doc);
    }
#elif FORMAT_KDE_KUIT_USE_FALLBACK_MARKUP
    {
      markup_parser_ty parser;
      markup_parse_context_ty *context;

      memset (&parser, 0, sizeof (markup_parser_ty));
      context = markup_parse_context_new (&parser, 0, NULL);
      if (!markup_parse_context_parse (context, buffer, strlen (buffer)))
        {
          *invalid_reason =
            xasprintf (_("error while parsing: %s"),
                       markup_parse_context_get_error (context));
          free (buffer);
          markup_parse_context_free (context);
          return NULL;
        }

      if (!markup_parse_context_end_parse (context))
        {
          *invalid_reason =
            xasprintf (_("error while parsing: %s"),
                       markup_parse_context_get_error (context));
          free (buffer);
          markup_parse_context_free (context);
          return NULL;
        }

      free (buffer);
      markup_parse_context_free (context);
    }
#else
    /* No support for XML.  */
    free (buffer);
#endif

  spec.base = formatstring_kde.parse (format, translated, fdi, invalid_reason);
  if (spec.base == NULL)
    return NULL;

  result = XMALLOC (struct spec);
  *result = spec;
  return result;
}

static void
format_free (void *descr)
{
  struct spec *spec = descr;
  formatstring_kde.free (spec->base);
  free (spec);
}

static int
format_get_number_of_directives (void *descr)
{
  struct spec *spec = descr;
  return formatstring_kde.get_number_of_directives (spec->base);
}

static bool
format_check (void *msgid_descr, void *msgstr_descr, bool equality,
              formatstring_error_logger_t error_logger,
              const char *pretty_msgid, const char *pretty_msgstr)
{
  struct spec *msgid_spec = msgid_descr;
  struct spec *msgstr_spec = msgstr_descr;

  return formatstring_kde.check (msgid_spec->base, msgstr_spec->base, equality,
                                 error_logger,
                                 pretty_msgid, pretty_msgstr);
}

struct formatstring_parser formatstring_kde_kuit =
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
 * compile-command: "/bin/sh ../libtool --tag=CC --mode=link gcc -o a.out -static -O -g -Wall -I.. -I../gnulib-lib -I../../gettext-runtime/intl -DHAVE_CONFIG_H -DTEST format-kde-kuit.c ../gnulib-lib/libgettextlib.la"
 * End:
 */

#endif /* TEST */
