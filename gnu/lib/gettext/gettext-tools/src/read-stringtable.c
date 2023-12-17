/* Reading NeXTstep/GNUstep .strings files.
   Copyright (C) 2003, 2005-2007, 2009, 2019-2020, 2023 Free Software Foundation, Inc.
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
#include "read-stringtable.h"

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "attribute.h"
#include "error.h"
#include "error-progname.h"
#include "read-catalog-abstract.h"
#include "xalloc.h"
#include "xvasprintf.h"
#include "po-xerror.h"
#include "unistr.h"
#include "gettext.h"

#define _(str) gettext (str)

/* The format of NeXTstep/GNUstep .strings files is documented in
     gnustep-base-1.8.0/Tools/make_strings/Using.txt
   and in the comments of method propertyListFromStringsFileFormat in
     gnustep-base-1.8.0/Source/NSString.m
   In summary, it's a Objective-C like file with pseudo-assignments of the form
          "key" = "value";
   where the key is the msgid and the value is the msgstr.

   The implementation of the parser of .strings files is in
     gnustep-base-1.8.0/Source/NSString.m
     function GSPropertyListFromStringsFormat
     (indirectly called from NSBundle's method localizedStringForKey).

   A test case is in
     gnustep-base-1.8.0/Testing/English.lproj/NXStringTable.example
 */

/* Handling of comments: We copy all comments from the .strings file to
   the PO file. This is not really needed; it's a service for translators
   who don't like PO files and prefer to maintain the .strings file.  */


/* Real filename, used in error messages about the input file.  */
static const char *real_file_name;

/* File name and line number.  */
extern lex_pos_ty gram_pos;

/* The input file stream.  */
static FILE *fp;


/* Phase 1: Read a byte.
   Max. 4 pushback characters.  */

static unsigned char phase1_pushback[4];
static int phase1_pushback_length;

static int
phase1_getc ()
{
  int c;

  if (phase1_pushback_length)
    return phase1_pushback[--phase1_pushback_length];

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

static void
phase1_ungetc (int c)
{
  if (c != EOF)
    phase1_pushback[phase1_pushback_length++] = c;
}


/* Phase 2: Read an UCS-4 character.
   Max. 2 pushback characters.  */

/* End-of-file indicator for functions returning an UCS-4 character.  */
#define UEOF -1

static int phase2_pushback[4];
static int phase2_pushback_length;

/* The input file can be in Unicode encoding (UCS-2BE, UCS-2LE, UTF-8, each
   with a BOM!), or otherwise the locale-dependent default encoding is used.
   Since we don't want to depend on the locale here, we use ISO-8859-1
   instead.  */
enum enc
{
  enc_undetermined,
  enc_ucs2be,
  enc_ucs2le,
  enc_utf8,
  enc_iso8859_1
};
static enum enc encoding;

static int
phase2_getc ()
{
  if (phase2_pushback_length)
    return phase2_pushback[--phase2_pushback_length];

  if (encoding == enc_undetermined)
    {
      /* Determine the input file's encoding.  */
      int c0, c1;

      c0 = phase1_getc ();
      if (c0 == EOF)
        return UEOF;
      c1 = phase1_getc ();
      if (c1 == EOF)
        {
          phase1_ungetc (c0);
          encoding = enc_iso8859_1;
        }
      else if (c0 == 0xfe && c1 == 0xff)
        encoding = enc_ucs2be;
      else if (c0 == 0xff && c1 == 0xfe)
        encoding = enc_ucs2le;
      else
        {
          int c2;

          c2 = phase1_getc ();
          if (c2 == EOF)
            {
              phase1_ungetc (c1);
              phase1_ungetc (c0);
              encoding = enc_iso8859_1;
            }
          else if (c0 == 0xef && c1 == 0xbb && c2 == 0xbf)
            encoding = enc_utf8;
          else
            {
              phase1_ungetc (c2);
              phase1_ungetc (c1);
              phase1_ungetc (c0);
              encoding = enc_iso8859_1;
            }
        }
    }

  switch (encoding)
    {
    case enc_ucs2be:
      /* Read an UCS-2BE encoded character.  */
      {
        int c0, c1;

        c0 = phase1_getc ();
        if (c0 == EOF)
          return UEOF;
        c1 = phase1_getc ();
        if (c1 == EOF)
          return UEOF;
        return (c0 << 8) + c1;
      }

    case enc_ucs2le:
      /* Read an UCS-2LE encoded character.  */
      {
        int c0, c1;

        c0 = phase1_getc ();
        if (c0 == EOF)
          return UEOF;
        c1 = phase1_getc ();
        if (c1 == EOF)
          return UEOF;
        return c0 + (c1 << 8);
      }

    case enc_utf8:
      /* Read an UTF-8 encoded character.  */
      {
        unsigned char buf[6];
        unsigned int count;
        int c;
        ucs4_t uc;

        c = phase1_getc ();
        if (c == EOF)
          return UEOF;
        buf[0] = c;
        count = 1;

        if (buf[0] >= 0xc0)
          {
            c = phase1_getc ();
            if (c == EOF)
              return UEOF;
            buf[1] = c;
            count = 2;

            if (buf[0] >= 0xe0
                && ((buf[1] ^ 0x80) < 0x40))
              {
                c = phase1_getc ();
                if (c == EOF)
                  return UEOF;
                buf[2] = c;
                count = 3;

                if (buf[0] >= 0xf0
                    && ((buf[2] ^ 0x80) < 0x40))
                  {
                    c = phase1_getc ();
                    if (c == EOF)
                      return UEOF;
                    buf[3] = c;
                    count = 4;

                    if (buf[0] >= 0xf8
                        && ((buf[3] ^ 0x80) < 0x40))
                      {
                        c = phase1_getc ();
                        if (c == EOF)
                          return UEOF;
                        buf[4] = c;
                        count = 5;

                        if (buf[0] >= 0xfc
                            && ((buf[4] ^ 0x80) < 0x40))
                          {
                            c = phase1_getc ();
                            if (c == EOF)
                              return UEOF;
                            buf[5] = c;
                            count = 6;
                          }
                      }
                  }
              }
          }

        u8_mbtouc (&uc, buf, count);
        return uc;
      }

    case enc_iso8859_1:
      /* Read an ISO-8859-1 encoded character.  */
      {
        int c = phase1_getc ();

        if (c == EOF)
          return UEOF;
        return c;
      }

    default:
      abort ();
    }
}

static void
phase2_ungetc (int c)
{
  if (c != UEOF)
    phase2_pushback[phase2_pushback_length++] = c;
}


/* Phase 3: Read an UCS-4 character, with line number handling.  */

static int
phase3_getc ()
{
  int c = phase2_getc ();

  if (c == '\n')
    gram_pos.line_number++;

  return c;
}

static void
phase3_ungetc (int c)
{
  if (c == '\n')
    --gram_pos.line_number;
  phase2_ungetc (c);
}


/* Convert from UCS-4 to UTF-8.  */
static char *
conv_from_ucs4 (const int *buffer, size_t buflen)
{
  unsigned char *utf8_string;
  size_t pos;
  unsigned char *q;

  /* Each UCS-4 word needs 6 bytes at worst.  */
  utf8_string = XNMALLOC (6 * buflen + 1, unsigned char);

  for (pos = 0, q = utf8_string; pos < buflen; )
    {
      unsigned int uc;
      int n;

      uc = buffer[pos++];
      n = u8_uctomb (q, uc, 6);
      assert (n > 0);
      q += n;
    }
  *q = '\0';
  assert (q - utf8_string <= 6 * buflen);

  return (char *) utf8_string;
}


/* Parse a string enclosed in double-quotes.  Input is UCS-4 encoded.
   Return the string in UTF-8 encoding, or NULL if the input doesn't represent
   a valid string enclosed in double-quotes.  */
static char *
parse_escaped_string (const int *string, size_t length)
{
  static int *buffer;
  static size_t bufmax;
  static size_t buflen;
  const int *string_limit = string + length;
  int c;

  if (string == string_limit)
    return NULL;
  c = *string++;
  if (c != '"')
    return NULL;
  buflen = 0;
  for (;;)
    {
      if (string == string_limit)
        return NULL;
      c = *string++;
      if (c == '"')
        break;
      if (c == '\\')
        {
          if (string == string_limit)
            return NULL;
          c = *string++;
          if (c >= '0' && c <= '7')
            {
              unsigned int n = 0;
              int j = 0;
              for (;;)
                {
                  n = n * 8 + (c - '0');
                  if (++j == 3)
                    break;
                  if (string == string_limit)
                    break;
                  c = *string;
                  if (!(c >= '0' && c <= '7'))
                    break;
                  string++;
                }
              c = n;
            }
          else if (c == 'u' || c == 'U')
            {
              unsigned int n = 0;
              int j;
              for (j = 0; j < 4; j++)
                {
                  if (string == string_limit)
                    break;
                  c = *string;
                  if (c >= '0' && c <= '9')
                    n = n * 16 + (c - '0');
                  else if (c >= 'A' && c <= 'F')
                    n = n * 16 + (c - 'A' + 10);
                  else if (c >= 'a' && c <= 'f')
                    n = n * 16 + (c - 'a' + 10);
                  else
                    break;
                  string++;
                }
              c = n;
            }
          else
            switch (c)
              {
              case 'a': c = '\a'; break;
              case 'b': c = '\b'; break;
              case 't': c = '\t'; break;
              case 'r': c = '\r'; break;
              case 'n': c = '\n'; break;
              case 'v': c = '\v'; break;
              case 'f': c = '\f'; break;
              }
        }
      if (buflen >= bufmax)
        {
          bufmax = 2 * bufmax + 10;
          buffer = xrealloc (buffer, bufmax * sizeof (int));
        }
      buffer[buflen++] = c;
    }

  return conv_from_ucs4 (buffer, buflen);
}


/* Accumulating flag comments.  */

static char *special_comment;

static inline void
special_comment_reset ()
{
  if (special_comment != NULL)
    free (special_comment);
  special_comment = NULL;
}

static void
special_comment_add (const char *flag)
{
  if (special_comment == NULL)
    special_comment = xstrdup (flag);
  else
    {
      size_t total_len = strlen (special_comment) + 2 + strlen (flag) + 1;
      special_comment = xrealloc (special_comment, total_len);
      strcat (special_comment, ", ");
      strcat (special_comment, flag);
    }
}

static inline void
special_comment_finish ()
{
  if (special_comment != NULL)
    {
      po_callback_comment_special (special_comment);
      free (special_comment);
      special_comment = NULL;
    }
}


/* Accumulating comments.  */

static int *buffer;
static size_t bufmax;
static size_t buflen;
static bool next_is_obsolete;
static bool next_is_fuzzy;
static char *fuzzy_msgstr;
static bool expect_fuzzy_msgstr_as_c_comment;
static bool expect_fuzzy_msgstr_as_cxx_comment;

static inline void
comment_start ()
{
  buflen = 0;
}

static inline void
comment_add (int c)
{
  if (buflen >= bufmax)
    {
      bufmax = 2 * bufmax + 10;
      buffer = xrealloc (buffer, bufmax * sizeof (int));
    }
  buffer[buflen++] = c;
}

static void
comment_line_end (size_t chars_to_remove, bool test_for_fuzzy_msgstr)
{
  char *line;

  buflen -= chars_to_remove;
  /* Drop trailing white space, but not EOLs.  */
  while (buflen >= 1
         && (buffer[buflen - 1] == ' ' || buffer[buflen - 1] == '\t'))
    --buflen;

  /* At special positions we interpret a comment of the form
       = "escaped string"
     with an optional trailing semicolon as being the fuzzy msgstr, not a
     regular comment.  */
  if (test_for_fuzzy_msgstr
      && buflen > 2 && buffer[0] == '=' && buffer[1] == ' '
      && (fuzzy_msgstr =
          parse_escaped_string (buffer + 2,
                                buflen - (buffer[buflen - 1] == ';') - 2)))
    return;

  line = conv_from_ucs4 (buffer, buflen);

  if (strcmp (line, "Flag: untranslated") == 0)
    {
      special_comment_add ("fuzzy");
      next_is_fuzzy = true;
    }
  else if (strcmp (line, "Flag: unmatched") == 0)
    next_is_obsolete = true;
  else if (strlen (line) >= 6 && memcmp (line, "Flag: ", 6) == 0)
    special_comment_add (line + 6);
  else if (strlen (line) >= 9 && memcmp (line, "Comment: ", 9) == 0)
    /* A comment extracted from the source.  */
    po_callback_comment_dot (line + 9);
  else
    {
      char *last_colon;
      unsigned long number;
      char *endp;

      if (strlen (line) >= 6 && memcmp (line, "File: ", 6) == 0
          && (last_colon = strrchr (line + 6, ':')) != NULL
          && *(last_colon + 1) != '\0'
          && (number = strtoul (last_colon + 1, &endp, 10), *endp == '\0'))
        {
          /* A "File: <filename>:<number>" type comment.  */
          *last_colon = '\0';
          po_callback_comment_filepos (line + 6, number);
        }
      else
        po_callback_comment (line);
    }
}


/* Phase 4: Replace each comment that is not inside a string with a space
   character.  */

static int
phase4_getc ()
{
  int c;

  c = phase3_getc ();
  if (c != '/')
    return c;
  c = phase3_getc ();
  switch (c)
    {
    default:
      phase3_ungetc (c);
      return '/';

    case '*':
      /* C style comment.  */
      {
        bool last_was_star;
        size_t trailing_stars;
        bool seen_newline;

        comment_start ();
        last_was_star = false;
        trailing_stars = 0;
        seen_newline = false;
        /* Drop additional stars at the beginning of the comment.  */
        for (;;)
          {
            c = phase3_getc ();
            if (c != '*')
              break;
            last_was_star = true;
          }
        phase3_ungetc (c);
        for (;;)
          {
            c = phase3_getc ();
            if (c == UEOF)
              break;
            /* We skip all leading white space, but not EOLs.  */
            if (!(buflen == 0 && (c == ' ' || c == '\t')))
              comment_add (c);
            switch (c)
              {
              case '\n':
                seen_newline = true;
                comment_line_end (1, false);
                comment_start ();
                last_was_star = false;
                trailing_stars = 0;
                continue;

              case '*':
                last_was_star = true;
                trailing_stars++;
                continue;

              case '/':
                if (last_was_star)
                  {
                    /* Drop additional stars at the end of the comment.  */
                    comment_line_end (trailing_stars + 1,
                                      expect_fuzzy_msgstr_as_c_comment
                                      && !seen_newline);
                    break;
                  }
                FALLTHROUGH;

              default:
                last_was_star = false;
                trailing_stars = 0;
                continue;
              }
            break;
          }
        return ' ';
      }

    case '/':
      /* C++ style comment.  */
      comment_start ();
      for (;;)
        {
          c = phase3_getc ();
          if (c == '\n' || c == UEOF)
            break;
          /* We skip all leading white space, but not EOLs.  */
          if (!(buflen == 0 && (c == ' ' || c == '\t')))
            comment_add (c);
        }
      comment_line_end (0, expect_fuzzy_msgstr_as_cxx_comment);
      return '\n';
    }
}

static inline void
phase4_ungetc (int c)
{
  phase3_ungetc (c);
}


/* Return true if a character is considered as whitespace.  */
static bool
is_whitespace (int c)
{
  return (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\f'
          || c == '\b');
}

/* Return true if a character needs quoting, i.e. cannot be used in unquoted
   tokens.  */
static bool
is_quotable (int c)
{
  if ((c >= '0' && c <= '9')
      || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))
    return false;
  switch (c)
    {
    case '!': case '#': case '$': case '%': case '&': case '*':
    case '+': case '-': case '.': case '/': case ':': case '?':
    case '@': case '|': case '~': case '_': case '^':
      return false;
    default:
      return true;
    }
}


/* Read a key or value string.
   Return the string in UTF-8 encoding, or NULL if no string is seen.
   Return the start position of the string in *pos.  */
static char *
read_string (lex_pos_ty *pos)
{
  static int *buffer;
  static size_t bufmax;
  static size_t buflen;
  int c;

  /* Skip whitespace before the string.  */
  do
    c = phase4_getc ();
  while (is_whitespace (c));

  if (c == UEOF)
    /* No more string.  */
    return NULL;

  *pos = gram_pos;
  buflen = 0;
  if (c == '"')
    {
      /* Read a string enclosed in double-quotes.  */
      for (;;)
        {
          c = phase3_getc ();
          if (c == UEOF || c == '"')
            break;
          if (c == '\\')
            {
              c = phase3_getc ();
              if (c == UEOF)
                break;
              if (c >= '0' && c <= '7')
                {
                  unsigned int n = 0;
                  int j = 0;
                  for (;;)
                    {
                      n = n * 8 + (c - '0');
                      if (++j == 3)
                        break;
                      c = phase3_getc ();
                      if (!(c >= '0' && c <= '7'))
                        {
                          phase3_ungetc (c);
                          break;
                        }
                    }
                  c = n;
                }
              else if (c == 'u' || c == 'U')
                {
                  unsigned int n = 0;
                  int j;
                  for (j = 0; j < 4; j++)
                    {
                      c = phase3_getc ();
                      if (c >= '0' && c <= '9')
                        n = n * 16 + (c - '0');
                      else if (c >= 'A' && c <= 'F')
                        n = n * 16 + (c - 'A' + 10);
                      else if (c >= 'a' && c <= 'f')
                        n = n * 16 + (c - 'a' + 10);
                      else
                        {
                          phase3_ungetc (c);
                          break;
                        }
                    }
                  c = n;
                }
              else
                switch (c)
                  {
                  case 'a': c = '\a'; break;
                  case 'b': c = '\b'; break;
                  case 't': c = '\t'; break;
                  case 'r': c = '\r'; break;
                  case 'n': c = '\n'; break;
                  case 'v': c = '\v'; break;
                  case 'f': c = '\f'; break;
                  }
            }
          if (buflen >= bufmax)
            {
              bufmax = 2 * bufmax + 10;
              buffer = xrealloc (buffer, bufmax * sizeof (int));
            }
          buffer[buflen++] = c;
        }
      if (c == UEOF)
        po_xerror (PO_SEVERITY_ERROR, NULL,
                   real_file_name, gram_pos.line_number, (size_t)(-1), false,
                   _("warning: unterminated string"));
    }
  else
    {
      /* Read a token outside quotes.  */
      if (is_quotable (c))
        po_xerror (PO_SEVERITY_ERROR, NULL,
                   real_file_name, gram_pos.line_number, (size_t)(-1), false,
                   _("warning: syntax error"));
      for (; c != UEOF && !is_quotable (c); c = phase4_getc ())
        {
          if (buflen >= bufmax)
            {
              bufmax = 2 * bufmax + 10;
              buffer = xrealloc (buffer, bufmax * sizeof (int));
            }
          buffer[buflen++] = c;
        }
    }

  return conv_from_ucs4 (buffer, buflen);
}


/* Read a .strings file from a stream, and dispatch to the various
   abstract_catalog_reader_class_ty methods.  */
static void
stringtable_parse (abstract_catalog_reader_ty *pop, FILE *file,
                   const char *real_filename, const char *logical_filename)
{
  fp = file;
  real_file_name = real_filename;
  gram_pos.file_name = xstrdup (real_file_name);
  gram_pos.line_number = 1;
  encoding = enc_undetermined;
  expect_fuzzy_msgstr_as_c_comment = false;
  expect_fuzzy_msgstr_as_cxx_comment = false;

  for (;;)
    {
      char *msgid;
      lex_pos_ty msgid_pos;
      char *msgstr;
      lex_pos_ty msgstr_pos;
      int c;

      /* Prepare for next msgid/msgstr pair.  */
      special_comment_reset ();
      next_is_obsolete = false;
      next_is_fuzzy = false;
      fuzzy_msgstr = NULL;

      /* Read the key and all the comments preceding it.  */
      msgid = read_string (&msgid_pos);
      if (msgid == NULL)
        break;

      special_comment_finish ();

      /* Skip whitespace.  */
      do
        c = phase4_getc ();
      while (is_whitespace (c));

      /* Expect a '=' or ';'.  */
      if (c == UEOF)
        {
          po_xerror (PO_SEVERITY_ERROR, NULL,
                     real_file_name, gram_pos.line_number, (size_t)(-1), false,
                     _("warning: unterminated key/value pair"));
          break;
        }
      if (c == ';')
        {
          /* "key"; is an abbreviation for "key"=""; and does not
             necessarily designate an untranslated entry.  */
          msgstr = xstrdup ("");
          msgstr_pos = msgid_pos;
          po_callback_message (NULL, msgid, &msgid_pos, NULL,
                               msgstr, strlen (msgstr) + 1, &msgstr_pos,
                               NULL, NULL, NULL,
                               false, next_is_obsolete);
        }
      else if (c == '=')
        {
          /* Read the value.  */
          msgstr = read_string (&msgstr_pos);
          if (msgstr == NULL)
            {
              po_xerror (PO_SEVERITY_ERROR, NULL,
                         real_file_name, gram_pos.line_number, (size_t)(-1),
                         false, _("warning: unterminated key/value pair"));
              break;
            }

          /* Skip whitespace.  But for fuzzy key/value pairs, look for the
             tentative msgstr in the form of a C style comment.  */
          expect_fuzzy_msgstr_as_c_comment = next_is_fuzzy;
          do
            {
              c = phase4_getc ();
              if (fuzzy_msgstr != NULL)
                expect_fuzzy_msgstr_as_c_comment = false;
            }
          while (is_whitespace (c));
          expect_fuzzy_msgstr_as_c_comment = false;

          /* Expect a ';'.  */
          if (c == ';')
            {
              /* But for fuzzy key/value pairs, look for the tentative msgstr
                 in the form of a C++ style comment. */
              if (fuzzy_msgstr == NULL && next_is_fuzzy)
                {
                  do
                    c = phase3_getc ();
                  while (c == ' ');
                  phase3_ungetc (c);

                  expect_fuzzy_msgstr_as_cxx_comment = true;
                  c = phase4_getc ();
                  phase4_ungetc (c);
                  expect_fuzzy_msgstr_as_cxx_comment = false;
                }
              if (fuzzy_msgstr != NULL && strcmp (msgstr, msgid) == 0)
                msgstr = fuzzy_msgstr;

              /* A key/value pair.  */
              po_callback_message (NULL, msgid, &msgid_pos, NULL,
                                   msgstr, strlen (msgstr) + 1, &msgstr_pos,
                                   NULL, NULL, NULL,
                                   false, next_is_obsolete);
            }
          else
            {
              po_xerror (PO_SEVERITY_ERROR, NULL,
                         real_file_name, gram_pos.line_number, (size_t)(-1),
                         false,
                         _("warning: syntax error, expected ';' after string"));
              break;
            }
        }
      else
        {
          po_xerror (PO_SEVERITY_ERROR, NULL,
                     real_file_name, gram_pos.line_number, (size_t)(-1), false,
                     _("warning: syntax error, expected '=' or ';' after string"));
          break;
        }
    }

  fp = NULL;
  real_file_name = NULL;
  gram_pos.line_number = 0;
}

const struct catalog_input_format input_format_stringtable =
{
  stringtable_parse,                    /* parse */
  true                                  /* produces_utf8 */
};
