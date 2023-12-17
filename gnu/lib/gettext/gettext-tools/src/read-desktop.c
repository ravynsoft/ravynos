/* Reading Desktop Entry files.
   Copyright (C) 1995-1998, 2000-2003, 2005-2006, 2008-2009, 2014-2019, 2023 Free Software Foundation, Inc.
   This file was written by Daiki Ueno <ueno@gnu.org>.

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

/* Specification.  */
#include "read-desktop.h"

#include "xalloc.h"

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "error-progname.h"
#include "xalloc.h"
#include "xvasprintf.h"
#include "c-ctype.h"
#include "po-lex.h"
#include "po-xerror.h"
#include "gettext.h"

#define _(str) gettext (str)

/* The syntax of a Desktop Entry file is defined at
   https://standards.freedesktop.org/desktop-entry-spec/latest/index.html.  */

desktop_reader_ty *
desktop_reader_alloc (desktop_reader_class_ty *method_table)
{
  desktop_reader_ty *reader;

  reader = (desktop_reader_ty *) xmalloc (method_table->size);
  reader->methods = method_table;
  if (method_table->constructor)
    method_table->constructor (reader);
  return reader;
}

void
desktop_reader_free (desktop_reader_ty *reader)
{
  if (reader->methods->destructor)
    reader->methods->destructor (reader);
  free (reader);
}

void
desktop_reader_handle_group (desktop_reader_ty *reader, const char *group)
{
  if (reader->methods->handle_group)
    reader->methods->handle_group (reader, group);
}

void
desktop_reader_handle_pair (desktop_reader_ty *reader,
                            lex_pos_ty *key_pos,
                            const char *key,
                            const char *locale,
                            const char *value)
{
  if (reader->methods->handle_pair)
    reader->methods->handle_pair (reader, key_pos, key, locale, value);
}

void
desktop_reader_handle_comment (desktop_reader_ty *reader, const char *s)
{
  if (reader->methods->handle_comment)
    reader->methods->handle_comment (reader, s);
}

void
desktop_reader_handle_blank (desktop_reader_ty *reader, const char *s)
{
  if (reader->methods->handle_blank)
    reader->methods->handle_blank (reader, s);
}

/* Real filename, used in error messages about the input file.  */
static const char *real_file_name;

/* File name and line number.  */
extern lex_pos_ty gram_pos;

/* The input file stream.  */
static FILE *fp;


static int
phase1_getc ()
{
  int c;

  c = getc (fp);

  if (c == EOF)
    {
      if (ferror (fp))
        {
          const char *errno_description = strerror (errno);
          po_xerror (PO_SEVERITY_FATAL_ERROR, NULL, NULL, 0, 0, false,
                     xasprintf ("%s: %s",
                                xasprintf (_("error while reading \"%s\""),
                                           real_file_name),
                                errno_description));
        }
      return EOF;
    }

  return c;
}

static inline void
phase1_ungetc (int c)
{
  if (c != EOF)
    ungetc (c, fp);
}


static unsigned char phase2_pushback[2];
static int phase2_pushback_length;

static int
phase2_getc ()
{
  int c;

  if (phase2_pushback_length)
    c = phase2_pushback[--phase2_pushback_length];
  else
    {
      c = phase1_getc ();

      if (c == '\r')
        {
          int c2 = phase1_getc ();
          if (c2 == '\n')
            c = c2;
          else
            phase1_ungetc (c2);
        }
    }

  if (c == '\n')
    gram_pos.line_number++;

  return c;
}

static void
phase2_ungetc (int c)
{
  if (c == '\n')
    --gram_pos.line_number;
  if (c != EOF)
    phase2_pushback[phase2_pushback_length++] = c;
}

enum token_type_ty
{
  token_type_eof,
  token_type_group,
  token_type_pair,
  /* Unlike other scanners, preserve comments and blank lines for
     merging translations back into a desktop file, with msgfmt.  */
  token_type_comment,
  token_type_blank,
  token_type_other
};
typedef enum token_type_ty token_type_ty;

typedef struct token_ty token_ty;
struct token_ty
{
  token_type_ty type;
  char *string;
  const char *value;
  const char *locale;
};

/* Free the memory pointed to by a 'struct token_ty'.  */
static inline void
free_token (token_ty *tp)
{
  if (tp->type == token_type_group || tp->type == token_type_pair
      || tp->type == token_type_comment || tp->type == token_type_blank)
    free (tp->string);
}

static void
desktop_lex (token_ty *tp)
{
  static char *buffer;
  static size_t bufmax;
  size_t bufpos;

#undef APPEND
#define APPEND(c)                               \
  do                                            \
    {                                           \
      if (bufpos >= bufmax)                     \
        {                                       \
          bufmax += 100;                        \
          buffer = xrealloc (buffer, bufmax);   \
        }                                       \
      buffer[bufpos++] = c;                     \
    }                                           \
  while (0)

  bufpos = 0;
  for (;;)
    {
      int c;

      c = phase2_getc ();

      switch (c)
        {
        case EOF:
          tp->type = token_type_eof;
          return;

        case '[':
          {
            bool non_blank = false;

            for (;;)
              {
                c = phase2_getc ();
                if (c == EOF || c == ']')
                  break;
                if (c == '\n')
                  {
                    po_xerror (PO_SEVERITY_WARNING, NULL,
                               real_file_name, gram_pos.line_number, 0, false,
                               _("unterminated group name"));
                    break;
                  }
                /* Group names may contain all ASCII characters
                   except for '[' and ']' and control characters.  */
                if (!(c_isascii (c) && c != '[' && !c_iscntrl (c)))
                  break;
                APPEND (c);
              }
            /* Skip until newline.  */
            while (c != '\n' && c != EOF)
              {
                c = phase2_getc ();
                if (c == EOF)
                  break;
                if (!c_isspace (c))
                  non_blank = true;
              }
            if (non_blank)
              po_xerror (PO_SEVERITY_WARNING, NULL,
                         real_file_name, gram_pos.line_number, 0, false,
                         _("invalid non-blank character"));
            APPEND (0);
            tp->type = token_type_group;
            tp->string = xstrdup (buffer);
            return;
          }

        case '#':
          {
            /* Read until newline.  */
            for (;;)
              {
                c = phase2_getc ();
                if (c == EOF || c == '\n')
                  break;
                APPEND (c);
              }
            APPEND (0);
            tp->type = token_type_comment;
            tp->string = xstrdup (buffer);
            return;
          }

        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
        case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
        case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
        case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
        case 'Y': case 'Z':
        case '-':
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
        case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
        case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
        case 's': case 't': case 'u': case 'v': case 'w': case 'x':
        case 'y': case 'z':
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
          {
            size_t locale_start;
            bool found_locale = false;
            size_t value_start;
            for (;;)
              {
                APPEND (c);

                c = phase2_getc ();
                switch (c)
                  {
                  case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
                  case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
                  case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
                  case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
                  case 'Y': case 'Z':
                  case '-':
                  case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
                  case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
                  case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
                  case 's': case 't': case 'u': case 'v': case 'w': case 'x':
                  case 'y': case 'z':
                  case '0': case '1': case '2': case '3': case '4':
                  case '5': case '6': case '7': case '8': case '9':
                    continue;

                  case '[':
                    /* Finish the key part and start the locale part.  */
                    APPEND (0);
                    found_locale = true;
                    locale_start = bufpos;

                    for (;;)
                      {
                        int c2 = phase2_getc ();
                        if (c2 == EOF || c2 == ']')
                          break;
                        APPEND (c2);
                      }
                    break;

                  default:
                    phase2_ungetc (c);
                    break;
                  }
                break;
              }
            APPEND (0);

            /* Skip any space before '='.  */
            for (;;)
              {
                c = phase2_getc ();
                switch (c)
                  {
                  case ' ':
                    continue;
                  default:
                    phase2_ungetc (c);
                    break;
                  case EOF: case '\n':
                    break;
                  }
                break;
              }

            c = phase2_getc ();
            if (c != '=')
              {
                po_xerror (PO_SEVERITY_WARNING, NULL,
                           real_file_name, gram_pos.line_number, 0, false,
                           xasprintf (_("missing '=' after \"%s\""), buffer));
                for (;;)
                  {
                    c = phase2_getc ();
                    if (c == EOF || c == '\n')
                      break;
                  }
                tp->type = token_type_other;
                return;
              }

            /* Skip any space after '='.  */
            for (;;)
              {
                c = phase2_getc ();
                switch (c)
                  {
                  case ' ':
                    continue;
                  default:
                    phase2_ungetc (c);
                    break;
                  case EOF:
                    break;
                  }
                break;
              }

            value_start = bufpos;
            for (;;)
              {
                c = phase2_getc ();
                if (c == EOF || c == '\n')
                  break;
                APPEND (c);
              }
            APPEND (0);
            tp->type = token_type_pair;
            tp->string = xmemdup (buffer, bufpos);
            tp->locale = found_locale ? &buffer[locale_start] : NULL;
            tp->value = &buffer[value_start];
            return;
          }
        default:
          {
            bool non_blank = false;

            for (;;)
              {
                if (c == '\n' || c == EOF)
                  break;

                if (!c_isspace (c))
                  non_blank = true;
                else
                  APPEND (c);

                c = phase2_getc ();
              }
            if (non_blank)
              {
                po_xerror (PO_SEVERITY_WARNING, NULL,
                           real_file_name, gram_pos.line_number, 0, false,
                           _("invalid non-blank line"));
                tp->type = token_type_other;
                return;
              }
            APPEND (0);
            tp->type = token_type_blank;
            tp->string = xstrdup (buffer);
            return;
          }
        }
    }
#undef APPEND
}

void
desktop_parse (desktop_reader_ty *reader, FILE *file,
               const char *real_filename, const char *logical_filename)
{
  fp = file;
  real_file_name = real_filename;
  gram_pos.file_name = xstrdup (logical_filename);
  gram_pos.line_number = 1;

  for (;;)
    {
      struct token_ty token;
      desktop_lex (&token);
      switch (token.type)
        {
        case token_type_eof:
          goto out;
        case token_type_group:
          desktop_reader_handle_group (reader, token.string);
          break;
        case token_type_comment:
          desktop_reader_handle_comment (reader, token.string);
          break;
        case token_type_pair:
          desktop_reader_handle_pair (reader, &gram_pos,
                                      token.string, token.locale, token.value);
          break;
        case token_type_blank:
          desktop_reader_handle_blank (reader, token.string);
          break;
        case token_type_other:
          break;
        }
      free_token (&token);
    }

 out:
  fp = NULL;
  real_file_name = NULL;
  gram_pos.line_number = 0;
}

char *
desktop_escape_string (const char *s, bool is_list)
{
  char *buffer, *p;

  p = buffer = XNMALLOC (strlen (s) * 2 + 1, char);

  /* The first character must not be a whitespace.  */
  if (*s == ' ')
    {
      p = stpcpy (p, "\\s");
      s++;
    }
  else if (*s == '\t')
    {
      p = stpcpy (p, "\\t");
      s++;
    }

  for (;; s++)
    {
      if (*s == '\0')
        {
          *p = '\0';
          break;
        }

      switch (*s)
        {
        case '\n':
          p = stpcpy (p, "\\n");
          break;
        case '\r':
          p = stpcpy (p, "\\r");
          break;
        case '\\':
          if (is_list && *(s + 1) == ';')
            {
              p = stpcpy (p, "\\;");
              s++;
            }
          else
            p = stpcpy (p, "\\\\");
          break;
        default:
          *p++ = *s;
          break;
        }
    }

  return buffer;
}

char *
desktop_unescape_string (const char *s, bool is_list)
{
  char *buffer, *p;

  p = buffer = XNMALLOC (strlen (s) + 1, char);
  for (;; s++)
    {
      if (*s == '\0')
        {
          *p = '\0';
          break;
        }

      if (*s == '\\')
        {
          s++;

          if (*s == '\0')
            {
              *p = '\0';
              break;
            }

          switch (*s)
            {
            case 's':
              *p++ = ' ';
              break;
            case 'n':
              *p++ = '\n';
              break;
            case 't':
              *p++ = '\t';
              break;
            case 'r':
              *p++ = '\r';
              break;
            case ';':
              p = stpcpy (p, "\\;");
              break;
            default:
              *p++ = *s;
              break;
            }
        }
      else
        *p++ = *s;
    }
  return buffer;
}

void
desktop_add_keyword (hash_table *keywords, const char *name, bool is_list)
{
  hash_insert_entry (keywords, name, strlen (name), (void *) is_list);
}

void
desktop_add_default_keywords (hash_table *keywords)
{
  /* When adding new keywords here, also update the documentation in
     xgettext.texi!  */
  desktop_add_keyword (keywords, "Name", false);
  desktop_add_keyword (keywords, "GenericName", false);
  desktop_add_keyword (keywords, "Comment", false);
#if 0 /* Icon values are localizable, but not supported by xgettext.  */
  desktop_add_keyword (keywords, "Icon", false);
#endif
  desktop_add_keyword (keywords, "Keywords", true);
}
