/* xgettext PO, JavaProperties, and NXStringTable backends.
   Copyright (C) 1995-1998, 2000-2003, 2005-2006, 2008-2009, 2014, 2018, 2020, 2023 Free Software Foundation, Inc.

   This file was written by Peter Miller <millerp@canb.auug.org.au>

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
#include "x-po.h"
#include "x-properties.h"
#include "x-stringtable.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "message.h"
#include "xgettext.h"
#include "xalloc.h"
#include "read-catalog.h"
#include "read-po.h"
#include "read-properties.h"
#include "read-stringtable.h"
#include "msgl-iconv.h"
#include "msgl-ascii.h"
#include "po-charset.h"
#include "po-lex.h"
#include "gettext.h"

/* A convenience macro.  I don't like writing gettext() every time.  */
#define _(str) gettext (str)


/* The charset found in the header entry.  */
static char *header_charset;

/* Define a subclass extract_catalog_reader_ty of default_catalog_reader_ty.  */

static void
extract_add_message (default_catalog_reader_ty *this,
                     char *msgctxt,
                     char *msgid,
                     lex_pos_ty *msgid_pos,
                     char *msgid_plural,
                     char *msgstr, size_t msgstr_len,
                     lex_pos_ty *msgstr_pos,
                     char *prev_msgctxt,
                     char *prev_msgid,
                     char *prev_msgid_plural,
                     bool force_fuzzy, bool obsolete)
{
  /* See whether we shall exclude this message.  */
  if (exclude != NULL && message_list_search (exclude, msgctxt, msgid) != NULL)
    goto discard;

  /* If the msgid is the empty string, it is the old header.  Throw it
     away, we have constructed a new one.  Only remember its charset.
     But if no new one was constructed, keep the old header.  This is useful
     because the old header may contain a charset= directive.  */
  if (msgctxt == NULL && *msgid == '\0' && !xgettext_omit_header)
    {
      {
        const char *charsetstr = strstr (msgstr, "charset=");

        if (charsetstr != NULL)
          {
            size_t len;
            char *charset;

            charsetstr += strlen ("charset=");
            len = strcspn (charsetstr, " \t\n");
            charset = XNMALLOC (len + 1, char);
            memcpy (charset, charsetstr, len);
            charset[len] = '\0';

            if (header_charset != NULL)
              free (header_charset);
            header_charset = charset;
          }
      }

     discard:
      if (msgctxt != NULL)
        free (msgctxt);
      free (msgid);
      if (msgid_plural != NULL)
        free (msgid_plural);
      free (msgstr);
      if (prev_msgctxt != NULL)
        free (prev_msgctxt);
      if (prev_msgid != NULL)
        free (prev_msgid);
      if (prev_msgid_plural != NULL)
        free (prev_msgid_plural);
      return;
    }

  /* Invoke superclass method.  */
  default_add_message (this, msgctxt, msgid, msgid_pos, msgid_plural,
                       msgstr, msgstr_len, msgstr_pos,
                       prev_msgctxt, prev_msgid, prev_msgid_plural,
                       force_fuzzy, obsolete);
}


/* So that the one parser can be used for multiple programs, and also
   use good data hiding and encapsulation practices, an object
   oriented approach has been taken.  An object instance is allocated,
   and all actions resulting from the parse will be through
   invocations of method functions of that object.  */

static default_catalog_reader_class_ty extract_methods =
{
  {
    sizeof (default_catalog_reader_ty),
    default_constructor,
    default_destructor,
    default_parse_brief,
    default_parse_debrief,
    default_directive_domain,
    default_directive_message,
    default_comment,
    default_comment_dot,
    default_comment_filepos,
    default_comment_special
  },
  default_set_domain, /* set_domain */
  extract_add_message, /* add_message */
  NULL /* frob_new_message */
};


static void
extract (FILE *fp,
         const char *real_filename, const char *logical_filename,
         catalog_input_format_ty input_syntax,
         msgdomain_list_ty *mdlp)
{
  default_catalog_reader_ty *pop;

  header_charset = NULL;

  pop = default_catalog_reader_alloc (&extract_methods);
  pop->handle_comments = true;
  pop->allow_domain_directives = false;
  pop->allow_duplicates = false;
  pop->allow_duplicates_if_same_msgstr = true;
  pop->file_name = real_filename;
  pop->mdlp = NULL;
  pop->mlp = mdlp->item[0]->messages;
  catalog_reader_parse ((abstract_catalog_reader_ty *) pop, fp, real_filename,
                        logical_filename, input_syntax);
  catalog_reader_free ((abstract_catalog_reader_ty *) pop);

  if (header_charset != NULL)
    {
      if (!xgettext_omit_header)
        {
          /* Put the old charset into the freshly constructed header entry.  */
          message_ty *mp =
            message_list_search (mdlp->item[0]->messages, NULL, "");

          if (mp != NULL && !mp->obsolete)
            {
              const char *header = mp->msgstr;

              if (header != NULL)
                {
                  const char *charsetstr = strstr (header, "charset=");

                  if (charsetstr != NULL)
                    {
                      size_t len, len1, len2, len3;
                      char *new_header;

                      charsetstr += strlen ("charset=");
                      len = strcspn (charsetstr, " \t\n");

                      len1 = charsetstr - header;
                      len2 = strlen (header_charset);
                      len3 = (header + strlen (header)) - (charsetstr + len);
                      new_header = XNMALLOC (len1 + len2 + len3 + 1, char);
                      memcpy (new_header, header, len1);
                      memcpy (new_header + len1, header_charset, len2);
                      memcpy (new_header + len1 + len2, charsetstr + len, len3 + 1);
                      mp->msgstr = new_header;
                      mp->msgstr_len = len1 + len2 + len3 + 1;
                    }
                }
            }

          if (!input_syntax->produces_utf8)
            {
              /* Convert the messages to UTF-8.
                 finalize_header() expects this.  */
              message_list_ty *mlp = mdlp->item[0]->messages;
              iconv_message_list (mlp, NULL, po_charset_utf8, logical_filename);
            }
        }

      free (header_charset);
    }
  else
    {
      if (!xgettext_omit_header && !input_syntax->produces_utf8)
        {
          /* finalize_header() expects the messages to be in UTF-8 encoding.
             We don't know the encoding here; therefore we have to reject the
             input if it is not entirely ASCII.  */
          if (!is_ascii_msgdomain_list (mdlp))
            error (EXIT_FAILURE, 0,
                   _("%s: input file doesn't contain a header entry with a charset specification"),
                   logical_filename);
        }
    }
}


void
extract_po (FILE *fp,
            const char *real_filename, const char *logical_filename,
            flag_context_list_table_ty *flag_table,
            msgdomain_list_ty *mdlp)
{
  extract (fp, real_filename,  logical_filename, &input_format_po, mdlp);
}


void
extract_properties (FILE *fp,
                    const char *real_filename, const char *logical_filename,
                    flag_context_list_table_ty *flag_table,
                    msgdomain_list_ty *mdlp)
{
  extract (fp, real_filename,  logical_filename, &input_format_properties,
           mdlp);
}


void
extract_stringtable (FILE *fp,
                     const char *real_filename, const char *logical_filename,
                     flag_context_list_table_ty *flag_table,
                     msgdomain_list_ty *mdlp)
{
  extract (fp, real_filename,  logical_filename, &input_format_stringtable,
           mdlp);
}
