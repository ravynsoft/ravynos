/* Writing Desktop Entry files.
   Copyright (C) 1995-1998, 2000-2003, 2005-2006, 2008-2009, 2014-2016, 2019-2020 Free Software Foundation, Inc.
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
#include "write-desktop.h"

#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "error.h"
#include "msgl-iconv.h"
#include "msgl-header.h"
#include "po-charset.h"
#include "read-catalog.h"
#include "read-po.h"
#include "read-desktop.h"
#include "fwriteerror.h"
#include "xalloc.h"
#include "gettext.h"

#define _(str) gettext (str)

typedef struct msgfmt_desktop_reader_ty msgfmt_desktop_reader_ty;
struct msgfmt_desktop_reader_ty
{
  DESKTOP_READER_TY
  msgfmt_operand_list_ty *operands;
  hash_table *keywords;
  FILE *output_file;
};

static void
msgfmt_desktop_handle_group (struct desktop_reader_ty *reader,
                             const char *group)
{
  msgfmt_desktop_reader_ty *msgfmt_reader = (msgfmt_desktop_reader_ty *) reader;

  fprintf (msgfmt_reader->output_file, "[%s]\n", group);
}

static void
msgfmt_desktop_handle_pair (desktop_reader_ty *reader,
                            lex_pos_ty *key_pos,
                            const char *key,
                            const char *locale,
                            const char *value)
{
  msgfmt_desktop_reader_ty *msgfmt_reader = (msgfmt_desktop_reader_ty *) reader;
  void *keyword_value;

  if (!locale)
    {
      /* Write translated pair, if any.  */
      if (hash_find_entry (msgfmt_reader->keywords, key, strlen (key),
                           &keyword_value) == 0)
        {
          bool is_list = (bool) (uintptr_t) keyword_value;
          char *unescaped = desktop_unescape_string (value, is_list);
          size_t i;

          for (i = 0; i < msgfmt_reader->operands->nitems; i++)
            {
              msgfmt_operand_ty *operand = &msgfmt_reader->operands->items[i];
              message_ty *mp;

              mp = message_list_search (operand->mlp, NULL, unescaped);
              if (mp && *mp->msgstr != '\0')
                {
                  char *escaped;

                  escaped = desktop_escape_string (mp->msgstr, is_list);
                  fprintf (msgfmt_reader->output_file,
                           "%s[%s]=%s\n",
                           key, operand->language, escaped);
                  free (escaped);
                }
            }
          free (unescaped);
        }

      /* Write untranslated pair.  */
      fprintf (msgfmt_reader->output_file, "%s=%s\n", key, value);
    }
  else
    /* Preserve already translated pair.  */
    fprintf (msgfmt_reader->output_file, "%s[%s]=%s\n", key, locale, value);
}

static void
msgfmt_desktop_handle_comment (struct desktop_reader_ty *reader, const char *s)
{
  msgfmt_desktop_reader_ty *msgfmt_reader = (msgfmt_desktop_reader_ty *) reader;

  fputc ('#', msgfmt_reader->output_file);
  fputs (s, msgfmt_reader->output_file);
  fputc ('\n', msgfmt_reader->output_file);
}

static void
msgfmt_desktop_handle_blank (struct desktop_reader_ty *reader, const char *s)
{
  msgfmt_desktop_reader_ty *msgfmt_reader = (msgfmt_desktop_reader_ty *) reader;

  fputs (s, msgfmt_reader->output_file);
  fputc ('\n', msgfmt_reader->output_file);
}

static desktop_reader_class_ty msgfmt_methods =
  {
    sizeof (msgfmt_desktop_reader_ty),
    NULL,
    NULL,
    msgfmt_desktop_handle_group,
    msgfmt_desktop_handle_pair,
    msgfmt_desktop_handle_comment,
    msgfmt_desktop_handle_blank
  };

int
msgdomain_write_desktop_bulk (msgfmt_operand_list_ty *operands,
                              const char *template_file_name,
                              hash_table *keywords,
                              const char *file_name)
{
  desktop_reader_ty *reader;
  msgfmt_desktop_reader_ty *msgfmt_reader;
  FILE *template_file;

  reader = desktop_reader_alloc (&msgfmt_methods);
  msgfmt_reader = (msgfmt_desktop_reader_ty *) reader;

  msgfmt_reader->operands = operands;
  msgfmt_reader->keywords = keywords;

  if (strcmp (file_name, "-") == 0)
    msgfmt_reader->output_file = stdout;
  else
    {
      msgfmt_reader->output_file = fopen (file_name, "w");
      if (msgfmt_reader->output_file == NULL)
        {
          desktop_reader_free (reader);
          error (0, errno, _("error while opening \"%s\" for writing"),
                 file_name);
          return 1;
        }
    }

  template_file = fopen (template_file_name, "r");
  if (template_file == NULL)
    {
      desktop_reader_free (reader);
      error (0, errno, _("error while opening \"%s\" for reading"),
             template_file_name);
      return 1;
    }

  desktop_parse (reader, template_file, template_file_name, template_file_name);

  /* Make sure nothing went wrong.  */
  if (fwriteerror (msgfmt_reader->output_file))
    {
      error (0, errno, _("error while writing \"%s\" file"),
             file_name);
      return 1;
    }

  desktop_reader_free (reader);

  return 0;
}

int
msgdomain_write_desktop (message_list_ty *mlp,
                         const char *canon_encoding,
                         const char *locale_name,
                         const char *template_file_name,
                         hash_table *keywords,
                         const char *file_name)
{
  msgfmt_operand_ty operand;
  msgfmt_operand_list_ty operands;

  /* Convert the messages to Unicode.  */
  iconv_message_list (mlp, canon_encoding, po_charset_utf8, NULL);

  /* Support for "reproducible builds": Delete information that may vary
     between builds in the same conditions.  */
  message_list_delete_header_field (mlp, "POT-Creation-Date:");

  /* Create a single-element operands and run the bulk operation on it.  */
  operand.language = (char *) locale_name;
  operand.mlp = mlp;
  operands.nitems = 1;
  operands.items = &operand;

  return msgdomain_write_desktop_bulk (&operands,
                                       template_file_name,
                                       keywords,
                                       file_name);
}
