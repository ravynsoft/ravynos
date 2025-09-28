/* Writing NeXTstep/GNUstep .strings files.
   Copyright (C) 2003, 2006-2008, 2019, 2021 Free Software Foundation, Inc.
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
#include "write-stringtable.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <textstyle.h>

#include "message.h"
#include "msgl-ascii.h"
#include "msgl-iconv.h"
#include "po-charset.h"
#include "c-strstr.h"
#include "xvasprintf.h"
#include "write-po.h"

/* The format of NeXTstep/GNUstep .strings files is documented in
     gnustep-base-1.8.0/Tools/make_strings/Using.txt
   and in the comments of method propertyListFromStringsFileFormat in
     gnustep-base-1.8.0/Source/NSString.m
   In summary, it's a Objective-C like file with pseudo-assignments of the form
          "key" = "value";
   where the key is the msgid and the value is the msgstr.
 */

/* Handling of comments: We copy all comments from the PO file to the
   .strings file. This is not really needed; it's a service for translators
   who don't like PO files and prefer to maintain the .strings file.  */

/* Since the interpretation of text files in GNUstep depends on the locale's
   encoding if they don't have a BOM, we choose one of three encodings with
   a BOM: UCS-2BE, UCS-2LE, UTF-8.  Since the first two of these don't cope
   with all of Unicode and we don't know whether GNUstep will switch to
   UTF-16 instead of UCS-2, we use UTF-8 with BOM.  BOMs are bad because they
   get in the way when concatenating files, but here we have no choice.  */

/* Writes a key or value to the stream, without newline.  */
static void
write_escaped_string (ostream_t stream, const char *str)
{
  const char *str_limit = str + strlen (str);

  ostream_write_str (stream, "\"");
  while (str < str_limit)
    {
      unsigned char c = (unsigned char) *str++;

      if (c == '\t')
        ostream_write_str (stream, "\\t");
      else if (c == '\n')
        ostream_write_str (stream, "\\n");
      else if (c == '\r')
        ostream_write_str (stream, "\\r");
      else if (c == '\f')
        ostream_write_str (stream, "\\f");
      else if (c == '\\' || c == '"')
        {
          char seq[2];
          seq[0] = '\\';
          seq[1] = c;
          ostream_write_mem (stream, seq, 2);
        }
      else
        {
          char seq[1];
          seq[0] = c;
          ostream_write_mem (stream, seq, 1);
        }
    }
  ostream_write_str (stream, "\"");
}

/* Writes a message to the stream.  */
static void
write_message (ostream_t stream, const message_ty *mp,
               size_t page_width, bool debug)
{
  /* Print translator comment if available.  */
  if (mp->comment != NULL)
    {
      size_t j;

      for (j = 0; j < mp->comment->nitems; ++j)
        {
          const char *s = mp->comment->item[j];

          /* Test whether it is safe to output the comment in C style, or
             whether we need C++ style for it.  */
          if (c_strstr (s, "*/") == NULL)
            {
              ostream_write_str (stream, "/*");
              if (*s != '\0' && *s != '\n')
                ostream_write_str (stream, " ");
              ostream_write_str (stream, s);
              ostream_write_str (stream, " */\n");
            }
          else
            do
              {
                const char *e;
                ostream_write_str (stream, "//");
                if (*s != '\0' && *s != '\n')
                  ostream_write_str (stream, " ");
                e = strchr (s, '\n');
                if (e == NULL)
                  {
                    ostream_write_str (stream, s);
                    s = NULL;
                  }
                else
                  {
                    ostream_write_mem (stream, s, e - s);
                    s = e + 1;
                  }
                ostream_write_str (stream, "\n");
              }
            while (s != NULL);
        }
    }

  /* Print xgettext extracted comments.  */
  if (mp->comment_dot != NULL)
    {
      size_t j;

      for (j = 0; j < mp->comment_dot->nitems; ++j)
        {
          const char *s = mp->comment_dot->item[j];

          /* Test whether it is safe to output the comment in C style, or
             whether we need C++ style for it.  */
          if (c_strstr (s, "*/") == NULL)
            {
              ostream_write_str (stream, "/* Comment: ");
              ostream_write_str (stream, s);
              ostream_write_str (stream, " */\n");
            }
          else
            {
              bool first = true;
              do
                {
                  const char *e;
                  ostream_write_str (stream, "//");
                  if (first || (*s != '\0' && *s != '\n'))
                    ostream_write_str (stream, " ");
                  if (first)
                    ostream_write_str (stream, "Comment: ");
                  e = strchr (s, '\n');
                  if (e == NULL)
                    {
                      ostream_write_str (stream, s);
                      s = NULL;
                    }
                  else
                    {
                      ostream_write_mem (stream, s, e - s);
                      s = e + 1;
                    }
                  ostream_write_str (stream, "\n");
                  first = false;
                }
              while (s != NULL);
            }
        }
    }

  /* Print the file position comments.  */
  if (mp->filepos_count != 0)
    {
      size_t j;

      for (j = 0; j < mp->filepos_count; ++j)
        {
          lex_pos_ty *pp = &mp->filepos[j];
          const char *cp = pp->file_name;
          char *str;

          while (cp[0] == '.' && cp[1] == '/')
            cp += 2;
          str = xasprintf ("/* File: %s:%ld */\n", cp, (long) pp->line_number);
          ostream_write_str (stream, str);
          free (str);
        }
    }

  /* Print flag information in special comment.  */
  if (mp->is_fuzzy || mp->msgstr[0] == '\0')
    ostream_write_str (stream, "/* Flag: untranslated */\n");
  if (mp->obsolete)
    ostream_write_str (stream, "/* Flag: unmatched */\n");
  {
    size_t i;
    for (i = 0; i < NFORMATS; i++)
      if (significant_format_p (mp->is_format[i]))
        {
          ostream_write_str (stream, "/* Flag: ");
          ostream_write_str (stream,
                             make_format_description_string (mp->is_format[i],
                                                             format_language[i],
                                                             debug));
          ostream_write_str (stream, " */\n");
        }
  }
  if (has_range_p (mp->range))
    {
      char *string;

      ostream_write_str (stream, "/* Flag: ");
      string = make_range_description_string (mp->range);
      ostream_write_str (stream, string);
      free (string);
      ostream_write_str (stream, " */\n");
    }

  /* Now write the untranslated string and the translated string.  */
  write_escaped_string (stream, mp->msgid);
  ostream_write_str (stream, " = ");
  if (mp->msgstr[0] != '\0')
    {
      if (mp->is_fuzzy)
        {
          /* Output the msgid as value, so that at runtime the untranslated
             string is returned.  */
          write_escaped_string (stream, mp->msgid);

          /* Output the msgstr as a comment, so that at runtime
             propertyListFromStringsFileFormat ignores it.  */
          if (c_strstr (mp->msgstr, "*/") == NULL)
            {
              ostream_write_str (stream, " /* = ");
              write_escaped_string (stream, mp->msgstr);
              ostream_write_str (stream, " */");
            }
          else
            {
              ostream_write_str (stream, "; // = ");
              write_escaped_string (stream, mp->msgstr);
            }
        }
      else
        write_escaped_string (stream, mp->msgstr);
    }
  else
    {
      /* Output the msgid as value, so that at runtime the untranslated
         string is returned.  */
      write_escaped_string (stream, mp->msgid);
    }
  ostream_write_str (stream, ";");

  ostream_write_str (stream, "\n");
}

/* Writes an entire message list to the stream.  */
static void
write_stringtable (ostream_t stream, message_list_ty *mlp,
                   const char *canon_encoding, size_t page_width, bool debug)
{
  bool blank_line;
  size_t j;

  /* Convert the messages to Unicode.  */
  iconv_message_list (mlp, canon_encoding, po_charset_utf8, NULL);

  /* Output the BOM.  */
  if (!is_ascii_message_list (mlp))
    ostream_write_str (stream, "\xef\xbb\xbf");

  /* Loop through the messages.  */
  blank_line = false;
  for (j = 0; j < mlp->nitems; ++j)
    {
      const message_ty *mp = mlp->item[j];

      if (mp->msgid_plural == NULL)
        {
          if (blank_line)
            ostream_write_str (stream, "\n");

          write_message (stream, mp, page_width, debug);

          blank_line = true;
        }
    }
}

/* Output the contents of a PO file in .strings syntax.  */
static void
msgdomain_list_print_stringtable (msgdomain_list_ty *mdlp, ostream_t stream,
                                  size_t page_width, bool debug)
{
  message_list_ty *mlp;

  if (mdlp->nitems == 1)
    mlp = mdlp->item[0]->messages;
  else
    mlp = message_list_alloc (false);
  write_stringtable (stream, mlp, mdlp->encoding, page_width, debug);
}

/* Describes a PO file in .strings syntax.  */
const struct catalog_output_format output_format_stringtable =
{
  msgdomain_list_print_stringtable,     /* print */
  true,                                 /* requires_utf8 */
  false,                                /* requires_utf8_for_filenames_with_spaces */
  false,                                /* supports_color */
  false,                                /* supports_multiple_domains */
  false,                                /* supports_contexts */
  false,                                /* supports_plurals */
  false,                                /* sorts_obsoletes_to_end */
  false,                                /* alternative_is_po */
  false                                 /* alternative_is_java_class */
};
