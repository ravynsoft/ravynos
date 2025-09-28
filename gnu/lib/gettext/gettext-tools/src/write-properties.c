/* Writing Java .properties files.
   Copyright (C) 2003, 2005-2009, 2019, 2021 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2003.

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
#include "write-properties.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <textstyle.h>

#include "error.h"
#include "message.h"
#include "msgl-ascii.h"
#include "msgl-iconv.h"
#include "po-charset.h"
#include "unistr.h"
#include "write-po.h"
#include "xalloc.h"

/* The format of the Java .properties files is documented in the JDK
   documentation for class java.util.Properties.  In the case of .properties
   files for PropertyResourceBundle, for each message, the msgid becomes the
   key (left-hand side) and the msgstr becomes the value (right-hand side)
   of a "key=value" line.  Messages with plurals are not supported in this
   format.  */

/* Handling of comments: We copy all comments from the PO file to the
   .properties file. This is not really needed; it's a service for translators
   who don't like PO files and prefer to maintain the .properties file.  */

/* Converts a string to JAVA encoding (with \uxxxx sequences for non-ASCII
   characters).  */
static const char *
conv_to_java (const char *string)
{
  /* We cannot use iconv to "JAVA" because not all iconv() implementations
     know about the "JAVA" encoding.  */
  static const char hexdigit[] = "0123456789abcdef";
  size_t length;
  char *result;

  if (is_ascii_string (string))
    return string;

  length = 0;
  {
    const char *str = string;
    const char *str_limit = str + strlen (str);

    while (str < str_limit)
      {
        ucs4_t uc;
        str += u8_mbtouc (&uc, (const unsigned char *) str, str_limit - str);
        length += (uc <= 0x007f ? 1 : uc < 0x10000 ? 6 : 12);
      }
  }

  result = XNMALLOC (length + 1, char);

  {
    char *newstr = result;
    const char *str = string;
    const char *str_limit = str + strlen (str);

    while (str < str_limit)
      {
        ucs4_t uc;
        str += u8_mbtouc (&uc, (const unsigned char *) str, str_limit - str);
        if (uc <= 0x007f)
          /* ASCII characters can be output literally.
             We could treat non-ASCII ISO-8859-1 characters (0x0080..0x00FF)
             the same way, but there is no point in doing this; Sun's
             nativetoascii doesn't do it either.  */
          *newstr++ = uc;
        else if (uc < 0x10000)
          {
            /* Single UCS-2 'char'  */
            sprintf (newstr, "\\u%c%c%c%c",
                     hexdigit[(uc >> 12) & 0x0f], hexdigit[(uc >> 8) & 0x0f],
                     hexdigit[(uc >> 4) & 0x0f], hexdigit[uc & 0x0f]);
            newstr += 6;
          }
        else
          {
            /* UTF-16 surrogate: two 'char's.  */
            ucs4_t uc1 = 0xd800 + ((uc - 0x10000) >> 10);
            ucs4_t uc2 = 0xdc00 + ((uc - 0x10000) & 0x3ff);
            sprintf (newstr, "\\u%c%c%c%c",
                     hexdigit[(uc1 >> 12) & 0x0f], hexdigit[(uc1 >> 8) & 0x0f],
                     hexdigit[(uc1 >> 4) & 0x0f], hexdigit[uc1 & 0x0f]);
            newstr += 6;
            sprintf (newstr, "\\u%c%c%c%c",
                     hexdigit[(uc2 >> 12) & 0x0f], hexdigit[(uc2 >> 8) & 0x0f],
                     hexdigit[(uc2 >> 4) & 0x0f], hexdigit[uc2 & 0x0f]);
            newstr += 6;
          }
      }
    *newstr = '\0';
  }

  return result;
}

/* Writes a key or value to the stream, without newline.  */
static void
write_escaped_string (ostream_t stream, const char *str, bool in_key)
{
  static const char hexdigit[] = "0123456789abcdef";
  const char *str_limit = str + strlen (str);
  bool first = true;

  while (str < str_limit)
    {
      ucs4_t uc;
      str += u8_mbtouc (&uc, (const unsigned char *) str, str_limit - str);
      /* Whitespace must be escaped.  */
      if (uc == 0x0020 && (first || in_key))
        ostream_write_str (stream, "\\ ");
      else if (uc == 0x0009)
        ostream_write_str (stream, "\\t");
      else if (uc == 0x000a)
        ostream_write_str (stream, "\\n");
      else if (uc == 0x000d)
        ostream_write_str (stream, "\\r");
      else if (uc == 0x000c)
        ostream_write_str (stream, "\\f");
      else if (/* Backslash must be escaped.  */
               uc == '\\'
               /* Possible comment introducers must be escaped.  */
               || uc == '#' || uc == '!'
               /* Key terminators must be escaped.  */
               || uc == '=' || uc == ':')
        {
          char seq[2];
          seq[0] = '\\';
          seq[1] = uc;
          ostream_write_mem (stream, seq, 2);
        }
      else if (uc >= 0x0020 && uc <= 0x007e)
        {
          /* ASCII characters can be output literally.
             We could treat non-ASCII ISO-8859-1 characters (0x0080..0x00FF)
             the same way, but there is no point in doing this; Sun's
             nativetoascii doesn't do it either.  */
          char seq[1];
          seq[0] = uc;
          ostream_write_mem (stream, seq, 1);
        }
      else if (uc < 0x10000)
        {
          /* Single UCS-2 'char'  */
          char seq[6];
          seq[0] = '\\';
          seq[1] = 'u';
          seq[2] = hexdigit[(uc >> 12) & 0x0f];
          seq[3] = hexdigit[(uc >> 8) & 0x0f];
          seq[4] = hexdigit[(uc >> 4) & 0x0f];
          seq[5] = hexdigit[uc & 0x0f];
          ostream_write_mem (stream, seq, 6);
        }
      else
        {
          /* UTF-16 surrogate: two 'char's.  */
          ucs4_t uc1 = 0xd800 + ((uc - 0x10000) >> 10);
          ucs4_t uc2 = 0xdc00 + ((uc - 0x10000) & 0x3ff);
          char seq[6];
          seq[0] = '\\';
          seq[1] = 'u';
          seq[2] = hexdigit[(uc1 >> 12) & 0x0f];
          seq[3] = hexdigit[(uc1 >> 8) & 0x0f];
          seq[4] = hexdigit[(uc1 >> 4) & 0x0f];
          seq[5] = hexdigit[uc1 & 0x0f];
          ostream_write_mem (stream, seq, 6);
          seq[0] = '\\';
          seq[1] = 'u';
          seq[2] = hexdigit[(uc2 >> 12) & 0x0f];
          seq[3] = hexdigit[(uc2 >> 8) & 0x0f];
          seq[4] = hexdigit[(uc2 >> 4) & 0x0f];
          seq[5] = hexdigit[uc2 & 0x0f];
          ostream_write_mem (stream, seq, 6);
        }
      first = false;
    }
}

/* Writes a message to the stream.  */
static void
write_message (ostream_t stream, const message_ty *mp,
               size_t page_width, bool debug)
{
  /* Print translator comment if available.  */
  message_print_comment (mp, stream);

  /* Print xgettext extracted comments.  */
  message_print_comment_dot (mp, stream);

  /* Print the file position comments.  */
  message_print_comment_filepos (mp, stream, po_charset_utf8, false,
                                 page_width);

  /* Print flag information in special comment.  */
  message_print_comment_flags (mp, stream, debug);

  /* Put a comment mark if the message is the header or untranslated or
     fuzzy.  */
  if (is_header (mp)
      || mp->msgstr[0] == '\0'
      || (mp->is_fuzzy && !is_header (mp)))
    ostream_write_str (stream, "!");

  /* Now write the untranslated string and the translated string.  */
  write_escaped_string (stream, mp->msgid, true);
  ostream_write_str (stream, "=");
  write_escaped_string (stream, mp->msgstr, false);

  ostream_write_str (stream, "\n");
}

/* Writes an entire message list to the stream.  */
static void
write_properties (ostream_t stream, message_list_ty *mlp,
                  const char *canon_encoding, size_t page_width, bool debug)
{
  bool blank_line;
  size_t j, i;

  /* Convert the messages to Unicode.  */
  iconv_message_list (mlp, canon_encoding, po_charset_utf8, NULL);
  for (j = 0; j < mlp->nitems; ++j)
    {
      message_ty *mp = mlp->item[j];

      if (mp->comment != NULL)
        for (i = 0; i < mp->comment->nitems; ++i)
          mp->comment->item[i] = conv_to_java (mp->comment->item[i]);
      if (mp->comment_dot != NULL)
        for (i = 0; i < mp->comment_dot->nitems; ++i)
          mp->comment_dot->item[i] = conv_to_java (mp->comment_dot->item[i]);
    }

  /* Loop through the messages.  */
  blank_line = false;
  for (j = 0; j < mlp->nitems; ++j)
    {
      const message_ty *mp = mlp->item[j];

      if (mp->msgid_plural == NULL && !mp->obsolete)
        {
          if (blank_line)
            ostream_write_str (stream, "\n");

          write_message (stream, mp, page_width, debug);

          blank_line = true;
        }
    }
}

/* Output the contents of a PO file in Java .properties syntax.  */
static void
msgdomain_list_print_properties (msgdomain_list_ty *mdlp, ostream_t stream,
                                 size_t page_width, bool debug)
{
  message_list_ty *mlp;

  if (mdlp->nitems == 1)
    mlp = mdlp->item[0]->messages;
  else
    mlp = message_list_alloc (false);
  write_properties (stream, mlp, mdlp->encoding, page_width, debug);
}

/* Describes a PO file in Java .properties syntax.  */
const struct catalog_output_format output_format_properties =
{
  msgdomain_list_print_properties,      /* print */
  true,                                 /* requires_utf8 */
  true,                                 /* requires_utf8_for_filenames_with_spaces */
  false,                                /* supports_color */
  false,                                /* supports_multiple_domains */
  false,                                /* supports_contexts */
  false,                                /* supports_plurals */
  false,                                /* sorts_obsoletes_to_end */
  true,                                 /* alternative_is_po */
  true                                  /* alternative_is_java_class */
};
