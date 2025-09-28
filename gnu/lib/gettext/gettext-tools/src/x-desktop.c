/* xgettext Desktop Entry backend.
   Copyright (C) 2014, 2018-2020, 2023 Free Software Foundation, Inc.

   This file was written by Daiki Ueno <ueno@gnu.org>, 2014.

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
# include "config.h"
#endif

/* Specification.  */
#include "x-desktop.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "message.h"
#include "xgettext.h"
#include "xg-message.h"
#include "error.h"
#include "error-progname.h"
#include "xalloc.h"
#include "xvasprintf.h"
#include "mem-hash-map.h"
#include "gettext.h"
#include "read-desktop.h"
#include "po-charset.h"
#include "c-ctype.h"

#define _(s) gettext(s)

/* ====================== Keyword set customization.  ====================== */

/* The syntax of a Desktop Entry file is defined at
   https://standards.freedesktop.org/desktop-entry-spec/latest/index.html

   Basically, values with 'localestring' type can be localized.
   However, the values of 'Icon', while being localizable, are not supported
   by xgettext.  See the documentation for more info.

   The type of a value is determined by looking at the key associated
   with it.  The list of available keys are listed on:
   https://standards.freedesktop.org/desktop-entry-spec/latest/ar01s04.html  */

static hash_table keywords;
static bool default_keywords = true;

static void
add_keyword (const char *name, hash_table *keywords, bool is_list)
{
  if (name == NULL)
    default_keywords = false;
  else
    {
      if (keywords->table == NULL)
        hash_init (keywords, 100);

      desktop_add_keyword (keywords, name, is_list);
    }
}

void
x_desktop_keyword (const char *name)
{
  add_keyword (name, &keywords, false);
}

static void
init_keywords (void)
{
  if (default_keywords)
    {
      if (keywords.table == NULL)
        hash_init (&keywords, 100);

      desktop_add_default_keywords (&keywords);
      default_keywords = false;
    }
}

typedef struct extract_desktop_reader_ty extract_desktop_reader_ty;
struct extract_desktop_reader_ty
{
  DESKTOP_READER_TY

  message_list_ty *mlp;
};

static void
extract_desktop_handle_group (struct desktop_reader_ty *reader,
                              const char *group)
{
  savable_comment_reset ();
}

static void
extract_desktop_handle_pair (struct desktop_reader_ty *reader,
                             lex_pos_ty *key_pos,
                             const char *key,
                             const char *locale,
                             const char *value)
{
  extract_desktop_reader_ty *extract_reader =
    (extract_desktop_reader_ty *) reader;
  void *keyword_value;

  if (!locale                   /* Skip already translated entry.  */
      && hash_find_entry (&keywords, key, strlen (key), &keyword_value) == 0)
    {
      bool is_list = (bool) keyword_value;

      remember_a_message (extract_reader->mlp, NULL,
                          desktop_unescape_string (value, is_list), false,
                          false, null_context, key_pos,
                          NULL, savable_comment, false);
    }
  savable_comment_reset ();
}

static void
extract_desktop_handle_comment (struct desktop_reader_ty *reader,
                                const char *buffer)
{
  size_t buflen = strlen (buffer);
  size_t bufpos = 0;

  while (bufpos < buflen
         && c_isspace (buffer[bufpos]))
    ++bufpos;
  while (buflen >= bufpos
         && c_isspace (buffer[buflen - 1]))
    --buflen;
  if (bufpos < buflen)
    {
      char *comment = xstrdup (buffer);
      comment[buflen] = 0;
      savable_comment_add (&comment[bufpos]);
      free (comment);
    }
}

static void
extract_desktop_handle_blank (struct desktop_reader_ty *reader,
                              const char *s)
{
  savable_comment_reset ();
}

static desktop_reader_class_ty extract_methods =
  {
    sizeof (extract_desktop_reader_ty),
    NULL,
    NULL,
    extract_desktop_handle_group,
    extract_desktop_handle_pair,
    extract_desktop_handle_comment,
    extract_desktop_handle_blank
  };

void
extract_desktop (FILE *f,
                 const char *real_filename, const char *logical_filename,
                 flag_context_list_table_ty *flag_table,
                 msgdomain_list_ty *mdlp)
{
  desktop_reader_ty *reader = desktop_reader_alloc (&extract_methods);
  extract_desktop_reader_ty *extract_reader =
    (extract_desktop_reader_ty *) reader;

  init_keywords ();
  xgettext_current_source_encoding = po_charset_utf8;

  extract_reader->mlp = mdlp->item[0]->messages;

  desktop_parse (reader, f, real_filename, logical_filename);
  desktop_reader_free (reader);

  reader = NULL;
}
