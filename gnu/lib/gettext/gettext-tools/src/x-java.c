/* xgettext Java backend.
   Copyright (C) 2003, 2005-2009, 2018-2023 Free Software Foundation, Inc.
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
# include "config.h"
#endif

/* Specification.  */
#include "x-java.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "attribute.h"
#include "message.h"
#include "rc-str-list.h"
#include "xgettext.h"
#include "xg-pos.h"
#include "xg-encoding.h"
#include "xg-mixed-string.h"
#include "xg-arglist-context.h"
#include "xg-arglist-callshape.h"
#include "xg-arglist-parser.h"
#include "xg-message.h"
#include "error.h"
#include "error-progname.h"
#include "xalloc.h"
#include "mem-hash-map.h"
#include "po-charset.h"
#include "unistr.h"
#include "unictype.h"
#include "gettext.h"

#define _(s) gettext(s)

#define SIZEOF(a) (sizeof(a) / sizeof(a[0]))


/* The Java syntax is defined in the
     Java Language Specification
     (available from https://docs.oracle.com/javase/specs/),
     chapter 3 "Lexical Structure".  */


/* ====================== Keyword set customization.  ====================== */

/* If true extract all strings.  */
static bool extract_all = false;

static hash_table keywords;
static bool default_keywords = true;


void
x_java_extract_all ()
{
  extract_all = true;
}


void
x_java_keyword (const char *name)
{
  if (name == NULL)
    default_keywords = false;
  else
    {
      const char *end;
      struct callshape shape;
      const char *colon;

      if (keywords.table == NULL)
        hash_init (&keywords, 100);

      split_keywordspec (name, &end, &shape);

      /* The characters between name and end should form a valid Java
         identifier sequence with dots.
         A colon means an invalid parse in split_keywordspec().  */
      colon = strchr (name, ':');
      if (colon == NULL || colon >= end)
        insert_keyword_callshape (&keywords, name, end - name, &shape);
    }
}

/* Finish initializing the keywords hash table.
   Called after argument processing, before each file is processed.  */
static void
init_keywords ()
{
  if (default_keywords)
    {
      /* When adding new keywords here, also update the documentation in
         xgettext.texi!  */
      x_java_keyword ("GettextResource.gettext:2");        /* static method */
      x_java_keyword ("GettextResource.ngettext:2,3");     /* static method */
      x_java_keyword ("GettextResource.pgettext:2c,3");    /* static method */
      x_java_keyword ("GettextResource.npgettext:2c,3,4"); /* static method */
      x_java_keyword ("gettext");
      x_java_keyword ("ngettext:1,2");
      x_java_keyword ("pgettext:1c,2");
      x_java_keyword ("npgettext:1c,2,3");
      x_java_keyword ("getString");     /* ResourceBundle.getString */
      default_keywords = false;
    }
}

void
init_flag_table_java ()
{
  xgettext_record_flag ("GettextResource.gettext:2:pass-java-format");
  xgettext_record_flag ("GettextResource.gettext:2:pass-java-printf-format");
  xgettext_record_flag ("GettextResource.ngettext:2:pass-java-format");
  xgettext_record_flag ("GettextResource.ngettext:2:pass-java-printf-format");
  xgettext_record_flag ("GettextResource.ngettext:3:pass-java-format");
  xgettext_record_flag ("GettextResource.ngettext:3:pass-java-printf-format");
  xgettext_record_flag ("GettextResource.pgettext:3:pass-java-format");
  xgettext_record_flag ("GettextResource.pgettext:3:pass-java-printf-format");
  xgettext_record_flag ("GettextResource.npgettext:3:pass-java-format");
  xgettext_record_flag ("GettextResource.npgettext:3:pass-java-printf-format");
  xgettext_record_flag ("GettextResource.npgettext:4:pass-java-format");
  xgettext_record_flag ("GettextResource.npgettext:4:pass-java-printf-format");
  xgettext_record_flag ("gettext:1:pass-java-format");
  xgettext_record_flag ("gettext:1:pass-java-printf-format");
  xgettext_record_flag ("ngettext:1:pass-java-format");
  xgettext_record_flag ("ngettext:1:pass-java-printf-format");
  xgettext_record_flag ("ngettext:2:pass-java-format");
  xgettext_record_flag ("ngettext:2:pass-java-printf-format");
  xgettext_record_flag ("pgettext:2:pass-java-format");
  xgettext_record_flag ("pgettext:2:pass-java-printf-format");
  xgettext_record_flag ("npgettext:2:pass-java-format");
  xgettext_record_flag ("npgettext:2:pass-java-printf-format");
  xgettext_record_flag ("npgettext:3:pass-java-format");
  xgettext_record_flag ("npgettext:3:pass-java-printf-format");
  xgettext_record_flag ("getString:1:pass-java-format");
  xgettext_record_flag ("getString:1:pass-java-printf-format");
  xgettext_record_flag ("MessageFormat:1:java-format");
  xgettext_record_flag ("MessageFormat.format:1:java-format");
  xgettext_record_flag ("String.format:1:java-printf-format");
  xgettext_record_flag ("printf:1:java-printf-format"); /* PrintStream.printf */
}


/* ======================== Reading of characters.  ======================== */

/* The input file stream.  */
static FILE *fp;


/* Fetch the next single-byte character from the input file.
   Pushback can consist of an unlimited number of 'u' followed by up to 4
   other characters.  */

/* Special coding of multiple 'u's in the pushback buffer.  */
#define MULTIPLE_U(count) (0x1000 + (count))

static int phase1_pushback[5];
static unsigned int phase1_pushback_length;

static int
phase1_getc ()
{
  int c;

  if (phase1_pushback_length)
    {
      c = phase1_pushback[--phase1_pushback_length];
      if (c >= MULTIPLE_U (0))
        {
          if (c > MULTIPLE_U (1))
            phase1_pushback[phase1_pushback_length++] = c - 1;
          return 'u';
        }
      else
        return c;
    }

  c = getc (fp);

  if (c == EOF)
    {
      if (ferror (fp))
        error (EXIT_FAILURE, errno,
               _("error while reading \"%s\""), real_file_name);
    }

  return c;
}

/* Supports any number of 'u' and up to 4 arbitrary characters of pushback.  */
static void
phase1_ungetc (int c)
{
  if (c != EOF)
    {
      if (c == 'u')
        {
          if (phase1_pushback_length > 0
              && phase1_pushback[phase1_pushback_length - 1] >= MULTIPLE_U (0))
            phase1_pushback[phase1_pushback_length - 1]++;
          else
            {
              if (phase1_pushback_length == SIZEOF (phase1_pushback))
                abort ();
              phase1_pushback[phase1_pushback_length++] = MULTIPLE_U (1);
            }
        }
      else
        {
          if (phase1_pushback_length == SIZEOF (phase1_pushback))
            abort ();
          phase1_pushback[phase1_pushback_length++] = c;
        }
    }
}


/* Fetch the next single-byte character or Unicode character from the file.
   (Here, as in the Java Language Specification, when we say "Unicode
   character", we actually mean "UTF-16 encoding unit".)  */

/* Return value of phase 2, 3, 4 when EOF is reached.  */
#define P2_EOF 0xffff

/* Convert an UTF-16 code point to a return value that can be distinguished
   from a single-byte return value.  */
#define UNICODE(code) (0x10000 + (code))

/* Test a return value of phase 2, 3, 4 whether it designates an UTF-16 code
   point.  */
#define IS_UNICODE(p2_result) ((p2_result) >= 0x10000)

/* Extract the UTF-16 code of a return value that satisfies IS_UNICODE.  */
#define UTF16_VALUE(p2_result) ((p2_result) - 0x10000)

/* Reduces a return value of phase 2, 3, 4 by unmasking the UNICODE bit,
   so that it can be more easily compared against an ASCII character.
   (RED (c) == 'x')  is equivalent to  (c == 'x' || c == UNICODE ('x')).  */
#define RED(p2_result) ((p2_result) & 0xffff)

static int phase2_pushback[1];
static int phase2_pushback_length;

static int
phase2_getc ()
{
  int c;

  if (phase2_pushback_length)
    return phase2_pushback[--phase2_pushback_length];

  c = phase1_getc ();
  if (c == EOF)
    return P2_EOF;
  if (c == '\\')
    {
      c = phase1_getc ();
      if (c == 'u')
        {
          unsigned int u_count = 1;
          unsigned char buf[4];
          unsigned int n;
          int i;

          for (;;)
            {
              c = phase1_getc ();
              if (c != 'u')
                break;
              u_count++;
            }
          phase1_ungetc (c);

          n = 0;
          for (i = 0; i < 4; i++)
            {
              c = phase1_getc ();

              if (c >= '0' && c <= '9')
                n = (n << 4) + (c - '0');
              else if (c >= 'A' && c <= 'F')
                n = (n << 4) + (c - 'A' + 10);
              else if (c >= 'a' && c <= 'f')
                n = (n << 4) + (c - 'a' + 10);
              else
                {
                  phase1_ungetc (c);
                  while (--i >= 0)
                    phase1_ungetc (buf[i]);
                  for (; u_count > 0; u_count--)
                    phase1_ungetc ('u');
                  return '\\';
                }

              buf[i] = c;
            }
          return UNICODE (n);
        }
      phase1_ungetc (c);
      return '\\';
    }
  return c;
}

/* Supports only one pushback character.  */
static void
phase2_ungetc (int c)
{
  if (c != P2_EOF)
    {
      if (phase2_pushback_length == SIZEOF (phase2_pushback))
        abort ();
      phase2_pushback[phase2_pushback_length++] = c;
    }
}


/* Fetch the next single-byte character or Unicode character from the file.
   With line number handling.
   Convert line terminators to '\n' or UNICODE ('\n').  */

static int phase3_pushback[2];
static int phase3_pushback_length;

static int
phase3_getc ()
{
  int c;

  if (phase3_pushback_length)
    {
      c = phase3_pushback[--phase3_pushback_length];
      if (c == '\n')
        ++line_number;
      return c;
    }

  c = phase2_getc ();

  /* Handle line terminators.  */
  if (RED (c) == '\r')
    {
      int c1 = phase2_getc ();

      if (RED (c1) != '\n')
        phase2_ungetc (c1);

      /* Seen line terminator CR or CR/LF.  */
      if (c == '\r' || c1 == '\n')
        {
          ++line_number;
          return '\n';
        }
      else
        return UNICODE ('\n');
    }
  else if (RED (c) == '\n')
    {
      /* Seen line terminator LF.  */
      if (c == '\n')
        {
          ++line_number;
          return '\n';
        }
      else
        return UNICODE ('\n');
    }

  return c;
}

/* Supports 2 characters of pushback.  */
static void
phase3_ungetc (int c)
{
  if (c != P2_EOF)
    {
      if (c == '\n')
        --line_number;
      if (phase3_pushback_length == SIZEOF (phase3_pushback))
        abort ();
      phase3_pushback[phase3_pushback_length++] = c;
    }
}


/* ========================= Accumulating strings.  ======================== */

/* See xg-mixed-string.h for the main API.  */

/* Append a character or Unicode character to a 'struct mixed_string_buffer'.  */
static void
mixed_string_buffer_append (struct mixed_string_buffer *bp, int c)
{
  if (IS_UNICODE (c))
    {
      /* Append a Unicode character.  */
      mixed_string_buffer_append_unicode (bp, UTF16_VALUE (c));
    }
  else
    {
      /* Append a single byte.  */
      mixed_string_buffer_append_char (bp, (unsigned char) c);
    }
}


/* ======================== Accumulating comments.  ======================== */


/* Accumulating a single comment line.  */

static struct mixed_string_buffer comment_buffer;

static inline void
comment_start ()
{
  mixed_string_buffer_init (&comment_buffer, lc_comment,
                            logical_file_name, line_number);
}

static inline bool
comment_at_start ()
{
  return mixed_string_buffer_is_empty (&comment_buffer);
}

static inline void
comment_add (int c)
{
  mixed_string_buffer_append (&comment_buffer, c);
}

static inline void
comment_line_end (size_t chars_to_remove)
{
  char *buffer =
    mixed_string_contents_free1 (mixed_string_buffer_result (&comment_buffer));
  size_t buflen = strlen (buffer);

  buflen -= chars_to_remove;
  while (buflen >= 1
         && (buffer[buflen - 1] == ' ' || buffer[buflen - 1] == '\t'))
    --buflen;
  buffer[buflen] = '\0';
  savable_comment_add (buffer);
}


/* These are for tracking whether comments count as immediately before
   keyword.  */
static int last_comment_line;
static int last_non_comment_line;


/* Replace each comment that is not inside a character constant or string
   literal with a space or newline character.  */

static int
phase4_getc ()
{
  int c0;
  int c;
  bool last_was_star;

  c0 = phase3_getc ();
  if (RED (c0) != '/')
    return c0;
  c = phase3_getc ();
  switch (RED (c))
    {
    default:
      phase3_ungetc (c);
      return c0;

    case '*':
      /* C style comment.  */
      comment_start ();
      last_was_star = false;
      for (;;)
        {
          c = phase3_getc ();
          if (c == P2_EOF)
            break;
          /* We skip all leading white space, but not EOLs.  */
          if (!(comment_at_start () && (RED (c) == ' ' || RED (c) == '\t')))
            comment_add (c);
          switch (RED (c))
            {
            case '\n':
              comment_line_end (1);
              comment_start ();
              last_was_star = false;
              continue;

            case '*':
              last_was_star = true;
              continue;

            case '/':
              if (last_was_star)
                {
                  comment_line_end (2);
                  break;
                }
              FALLTHROUGH;

            default:
              last_was_star = false;
              continue;
            }
          break;
        }
      last_comment_line = line_number;
      return ' ';

    case '/':
      /* C++ style comment.  */
      last_comment_line = line_number;
      comment_start ();
      for (;;)
        {
          c = phase3_getc ();
          if (RED (c) == '\n' || c == P2_EOF)
            break;
          /* We skip all leading white space, but not EOLs.  */
          if (!(comment_at_start () && (RED (c) == ' ' || RED (c) == '\t')))
            comment_add (c);
        }
      phase3_ungetc (c); /* push back the newline, to decrement line_number */
      comment_line_end (0);
      phase3_getc (); /* read the newline again */
      return '\n';
    }
}

/* Supports only one pushback character.  */
static void
phase4_ungetc (int c)
{
  phase3_ungetc (c);
}


/* ========================== Reading of tokens.  ========================== */

enum token_type_ty
{
  token_type_eof,
  token_type_lparen,            /* ( */
  token_type_rparen,            /* ) */
  token_type_lbrace,            /* { */
  token_type_rbrace,            /* } */
  token_type_comma,             /* , */
  token_type_dot,               /* . */
  token_type_string_literal,    /* "abc", """text block""" */
  token_type_number,            /* 1.23 */
  token_type_symbol,            /* identifier, keyword, null */
  token_type_plus,              /* + */
  token_type_other              /* character literal, misc. operator */
};
typedef enum token_type_ty token_type_ty;

typedef struct token_ty token_ty;
struct token_ty
{
  token_type_ty type;
  char *string;                         /* for token_type_symbol */
  mixed_string_ty *mixed_string;        /* for token_type_string_literal */
  refcounted_string_list_ty *comment;   /* for token_type_string_literal */
  int line_number;
};


/* Free the memory pointed to by a 'struct token_ty'.  */
static inline void
free_token (token_ty *tp)
{
  if (tp->type == token_type_symbol)
    free (tp->string);
  if (tp->type == token_type_string_literal)
    {
      free (tp->mixed_string);
      drop_reference (tp->comment);
    }
}


/* Read an escape sequence inside a string literal or character literal.  */
static inline int
do_getc_escaped ()
{
  int c;

  /* Use phase 3, because phase 4 elides comments.  */
  c = phase3_getc ();
  if (c == P2_EOF)
    return UNICODE ('\\');
  switch (RED (c))
    {
    case 'b':
      return UNICODE (0x08);
    case 't':
      return UNICODE (0x09);
    case 'n':
      return UNICODE (0x0a);
    case 'f':
      return UNICODE (0x0c);
    case 'r':
      return UNICODE (0x0d);
    case '"':
      return UNICODE ('"');
    case '\'':
      return UNICODE ('\'');
    case '\\':
      return UNICODE ('\\');
    case '0': case '1': case '2': case '3':
    case '4': case '5': case '6': case '7':
      {
        int n = RED (c) - '0';
        bool maybe3digits = (n < 4);

        c = phase3_getc ();
        if (RED (c) >= '0' && RED (c) <= '7')
          {
            n = (n << 3) + (RED (c) - '0');
            if (maybe3digits)
              {
                c = phase3_getc ();
                if (RED (c) >= '0' && RED (c) <= '7')
                  n = (n << 3) + (RED (c) - '0');
                else
                  phase3_ungetc (c);
              }
          }
        else
          phase3_ungetc (c);

        return UNICODE (n);
      }
    default:
      /* Invalid escape sequence.  */
      phase3_ungetc (c);
      return UNICODE ('\\');
    }
}

/* Read a string literal or character literal.  */
static void
accumulate_escaped (struct mixed_string_buffer *literal, int delimiter)
{
  int c;

  for (;;)
    {
      /* Use phase 3, because phase 4 elides comments.  */
      c = phase3_getc ();
      if (c == P2_EOF || RED (c) == delimiter)
        break;
      if (RED (c) == '\n')
        {
          phase3_ungetc (c);
          error_with_progname = false;
          if (delimiter == '\'')
            error (0, 0, _("%s:%d: warning: unterminated character constant"),
                   logical_file_name, line_number);
          else
            error (0, 0, _("%s:%d: warning: unterminated string constant"),
                   logical_file_name, line_number);
          error_with_progname = true;
          break;
        }
      if (RED (c) == '\\')
        c = do_getc_escaped ();
      mixed_string_buffer_append (literal, c);
    }
}


/* Strip the common indentation of the non-blank lines of the given string and
   remove all trailing whitespace of all lines.
   Like the Java method String.stripIndent does.
   <https://docs.oracle.com/en/java/javase/13/docs/api/java.base/java/lang/String.html#stripIndent()>  */
static void
strip_indent (mixed_string_ty *ms)
{
  size_t nsegments = ms->nsegments;
  size_t minimum_indentation = SIZE_MAX;
  {
    size_t curr_line_indentation = 0;
    bool curr_line_blank = true;
    size_t i;

    for (i = 0; i < nsegments; i++)
      {
        struct mixed_string_segment *segment = ms->segments[i];

        if (segment->type == utf8_encoded
            || (segment->type == source_encoded
                && xgettext_current_source_encoding == po_charset_utf8))
          {
            /* Consider Unicode whitespace characters.  */
            size_t seglength = segment->length;
            size_t j;

            for (j = 0; j < seglength; )
              {
                ucs4_t uc;
                int bytes =
                  u8_mbtouc (&uc, (const uint8_t *) &segment->contents[j],
                             seglength - j);
                j += bytes;
                if (uc == 0x000a)
                  {
                    /* Newline.  */
                    if (!curr_line_blank)
                      if (minimum_indentation > curr_line_indentation)
                        minimum_indentation = curr_line_indentation;
                    curr_line_indentation = 0;
                    curr_line_blank = true;
                  }
                else if (uc_is_java_whitespace (uc))
                  {
                    /* Whitespace character.  */
                    if (curr_line_blank)
                      /* Every whitespace character counts as 1, even the TAB
                         character.  */
                      curr_line_indentation++;
                  }
                else
                  {
                    /* Other character.  */
                    curr_line_blank = false;
                  }
              }
          }
        else
          {
            /* When the encoding is not UTF-8, consider only ASCII whitespace
               characters.  */
            size_t seglength = segment->length;
            size_t j;

            for (j = 0; j < seglength; j++)
              {
                char c = segment->contents[j];
                if (c == '\n')
                  {
                    /* Newline.  */
                    if (!curr_line_blank)
                      if (minimum_indentation > curr_line_indentation)
                        minimum_indentation = curr_line_indentation;
                    curr_line_indentation = 0;
                    curr_line_blank = true;
                  }
                else if (c == ' '
                         || (c >= 0x09 && c <= 0x0d)
                         || (c >= 0x1c && c <= 0x1f))
                  {
                    /* Whitespace character.  */
                    if (curr_line_blank)
                      /* Every whitespace character counts as 1, even the TAB
                         character.  */
                      curr_line_indentation++;
                  }
                else
                  {
                    /* Other character.  */
                    curr_line_blank = false;
                  }
              }
          }
      }
    /* The indentation of the last line matters even if is blank.  */
    if (minimum_indentation > curr_line_indentation)
      minimum_indentation = curr_line_indentation;
  }

  /* The same loop as above, but this time remove the leading
     minimum_indentation whitespace characters and all trailing whitespace
     characters from every line.  */
  {
    size_t start_of_curr_line_i = 0;
    size_t start_of_curr_line_j = 0;
    size_t start_of_trailing_whitespace_i = 0;
    size_t start_of_trailing_whitespace_j = 0;
    size_t whitespace_to_remove = minimum_indentation;
    size_t i;

    for (i = 0; i < nsegments; i++)
      {
        struct mixed_string_segment *segment = ms->segments[i];
        /* Perform a sliding copy from segment->contents[from_j] to
           segment->contents[to_j].  0 <= to_j <= from_j.  */
        size_t to_j;

        if (segment->type == utf8_encoded
            || (segment->type == source_encoded
                && xgettext_current_source_encoding == po_charset_utf8))
          {
            /* Consider Unicode whitespace characters.  */
            size_t seglength = segment->length;
            size_t from_j;

            for (to_j = from_j = 0; from_j < seglength; )
              {
                ucs4_t uc;
                int bytes =
                  u8_mbtouc (&uc, (const uint8_t *) &segment->contents[from_j],
                             seglength - from_j);
                if (uc == 0x000a)
                  {
                    /* Newline.  */
                    if (whitespace_to_remove > 0)
                      {
                        /* It was a blank line with fewer than minimum_indentation
                           whitespace characters.  Remove all this whitespace.  */
                        if (start_of_curr_line_i < i)
                          {
                            size_t k;
                            ms->segments[start_of_curr_line_i]->length = start_of_curr_line_j;
                            for (k = start_of_curr_line_i + 1; k < i; k++)
                              ms->segments[k]->length = 0;
                            to_j = 0;
                          }
                        else
                          to_j = start_of_curr_line_j;
                      }
                    else
                      {
                        /* Remove the trailing whitespace characters from the
                           current line.  */
                        if (start_of_trailing_whitespace_i < i)
                          {
                            size_t k;
                            ms->segments[start_of_trailing_whitespace_i]->length = start_of_trailing_whitespace_j;
                            for (k = start_of_trailing_whitespace_i + 1; k < i; k++)
                              ms->segments[k]->length = 0;
                            to_j = 0;
                          }
                        else
                          to_j = start_of_trailing_whitespace_j;
                      }
                  }
                if (to_j < from_j)
                  memmove (&segment->contents[to_j], &segment->contents[from_j], bytes);
                from_j += bytes;
                to_j += bytes;
                if (uc == 0x000a)
                  {
                    /* Newline.  */
                    start_of_curr_line_i = i;
                    start_of_curr_line_j = to_j;
                    start_of_trailing_whitespace_i = i;
                    start_of_trailing_whitespace_j = to_j;
                    whitespace_to_remove = minimum_indentation;
                  }
                else if (uc_is_java_whitespace (uc))
                  {
                    /* Whitespace character.  */
                    if (whitespace_to_remove > 0
                        && --whitespace_to_remove == 0)
                      {
                        /* Remove the leading minimum_indentation whitespace
                           characters from the current line.  */
                        if (start_of_curr_line_i < i)
                          {
                            size_t k;
                            ms->segments[start_of_curr_line_i]->length = start_of_curr_line_j;
                            for (k = start_of_curr_line_i + 1; k < i; k++)
                              ms->segments[k]->length = 0;
                            to_j = 0;
                          }
                        else
                          to_j = start_of_curr_line_j;
                      }
                  }
                else
                  {
                    /* Other character.  */
                    if (whitespace_to_remove > 0)
                      abort ();
                    start_of_trailing_whitespace_i = i;
                    start_of_trailing_whitespace_j = to_j;
                  }
              }
          }
        else
          {
            /* When the encoding is not UTF-8, consider only ASCII whitespace
               characters.  */
            size_t seglength = segment->length;
            size_t from_j;

            for (to_j = from_j = 0; from_j < seglength; )
              {
                char c = segment->contents[from_j++];
                if (c == '\n')
                  {
                    /* Newline.  */
                    if (whitespace_to_remove > 0)
                      {
                        /* It was a blank line with fewer than minimum_indentation
                           whitespace characters.  Remove all this whitespace.  */
                        if (start_of_curr_line_i < i)
                          {
                            size_t k;
                            ms->segments[start_of_curr_line_i]->length = start_of_curr_line_j;
                            for (k = start_of_curr_line_i + 1; k < i; k++)
                              ms->segments[k]->length = 0;
                            to_j = 0;
                          }
                        else
                          to_j = start_of_curr_line_j;
                      }
                    else
                      {
                        /* Remove the trailing whitespace characters from the
                           current line.  */
                        if (start_of_trailing_whitespace_i < i)
                          {
                            size_t k;
                            ms->segments[start_of_trailing_whitespace_i]->length = start_of_trailing_whitespace_j;
                            for (k = start_of_trailing_whitespace_i + 1; k < i; k++)
                              ms->segments[k]->length = 0;
                            to_j = 0;
                          }
                        else
                          to_j = start_of_trailing_whitespace_j;
                      }
                  }
                segment->contents[to_j++] = c;
                if (c == '\n')
                  {
                    /* Newline.  */
                    start_of_curr_line_i = i;
                    start_of_curr_line_j = to_j;
                    start_of_trailing_whitespace_i = i;
                    start_of_trailing_whitespace_j = to_j;
                    whitespace_to_remove = minimum_indentation;
                  }
                else if (c == ' '
                         || (c >= 0x09 && c <= 0x0d)
                         || (c >= 0x1c && c <= 0x1f))
                  {
                    /* Whitespace character.  */
                    if (whitespace_to_remove > 0
                        && --whitespace_to_remove == 0)
                      {
                        /* Remove the leading minimum_indentation whitespace
                           characters from the current line.  */
                        if (start_of_curr_line_i < i)
                          {
                            size_t k;
                            ms->segments[start_of_curr_line_i]->length = start_of_curr_line_j;
                            for (k = start_of_curr_line_i + 1; k < i; k++)
                              ms->segments[k]->length = 0;
                            to_j = 0;
                          }
                        else
                          to_j = start_of_curr_line_j;
                      }
                  }
                else
                  {
                    /* Other character.  */
                    if (whitespace_to_remove > 0)
                      abort ();
                    start_of_trailing_whitespace_i = i;
                    start_of_trailing_whitespace_j = to_j;
                  }
              }
          }
        if (i + 1 == nsegments)
          {
            /* Handle the last line.  */
            if (whitespace_to_remove > 0)
              {
                /* It was a blank line with fewer than minimum_indentation
                   whitespace characters.  Remove all this whitespace.  */
                if (start_of_curr_line_i < i)
                  {
                    size_t k;
                    ms->segments[start_of_curr_line_i]->length = start_of_curr_line_j;
                    for (k = start_of_curr_line_i + 1; k < i; k++)
                      ms->segments[k]->length = 0;
                    to_j = 0;
                  }
                else
                  to_j = start_of_curr_line_j;
              }
            else
              {
                /* Remove the trailing whitespace characters from the
                   current line.  */
                if (start_of_trailing_whitespace_i < i)
                  {
                    size_t k;
                    ms->segments[start_of_trailing_whitespace_i]->length = start_of_trailing_whitespace_j;
                    for (k = start_of_trailing_whitespace_i + 1; k < i; k++)
                      ms->segments[k]->length = 0;
                    to_j = 0;
                  }
                else
                  to_j = start_of_trailing_whitespace_j;
              }
          }
        segment->length = to_j;
      }
  }
}


/* Combine characters into tokens.  Discard whitespace.  */

static token_ty phase5_pushback[3];
static int phase5_pushback_length;

static void
phase5_get (token_ty *tp)
{
  int c;

  if (phase5_pushback_length)
    {
      *tp = phase5_pushback[--phase5_pushback_length];
      return;
    }
  tp->string = NULL;

  for (;;)
    {
      tp->line_number = line_number;
      c = phase4_getc ();

      if (c == P2_EOF)
        {
          tp->type = token_type_eof;
          return;
        }

      switch (RED (c))
        {
        case '\n':
          if (last_non_comment_line > last_comment_line)
            savable_comment_reset ();
          FALLTHROUGH;
        case ' ':
        case '\t':
        case '\f':
          /* Ignore whitespace and comments.  */
          continue;
        }

      last_non_comment_line = tp->line_number;

      switch (RED (c))
        {
        case '(':
          tp->type = token_type_lparen;
          return;

        case ')':
          tp->type = token_type_rparen;
          return;

        case '{':
          tp->type = token_type_lbrace;
          return;

        case '}':
          tp->type = token_type_rbrace;
          return;

        case ',':
          tp->type = token_type_comma;
          return;

        case '.':
          c = phase4_getc ();
          if (!(RED (c) >= '0' && RED (c) <= '9'))
            {
              phase4_ungetc (c);
              tp->type = token_type_dot;
              return;
            }
          FALLTHROUGH;

        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
          {
            /* Don't need to verify the complicated syntax of integers and
               floating-point numbers.  We assume a valid Java input.
               The simplified syntax that we recognize as number is: any
               sequence of alphanumeric characters, additionally '+' and '-'
               immediately after 'e' or 'E' except in hexadecimal numbers.  */
            bool hexadecimal = false;

            for (;;)
              {
                c = phase4_getc ();
                if (RED (c) >= '0' && RED (c) <= '9')
                  continue;
                if ((RED (c) >= 'A' && RED (c) <= 'Z')
                    || (RED (c) >= 'a' && RED (c) <= 'z'))
                  {
                    if (RED (c) == 'X' || RED (c) == 'x')
                      hexadecimal = true;
                    if ((RED (c) == 'E' || RED (c) == 'e') && !hexadecimal)
                      {
                        c = phase4_getc ();
                        if (!(RED (c) == '+' || RED (c) == '-'))
                          phase4_ungetc (c);
                      }
                    continue;
                  }
                if (RED (c) == '.')
                  continue;
                break;
              }
            phase4_ungetc (c);
            tp->type = token_type_number;
            return;
          }

        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
        case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
        case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
        case 'V': case 'W': case 'X': case 'Y': case 'Z':
        case '_':
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
        case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
        case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
        case 'v': case 'w': case 'x': case 'y': case 'z':
          /* Although Java allows identifiers containing many Unicode
             characters, we recognize only identifiers consisting of ASCII
             characters.  This avoids conversion hassles w.r.t. the --keyword
             arguments, and shouldn't be a big problem in practice.  */
          {
            static char *buffer;
            static int bufmax;
            int bufpos = 0;
            for (;;)
              {
                if (bufpos >= bufmax)
                  {
                    bufmax = 2 * bufmax + 10;
                    buffer = xrealloc (buffer, bufmax);
                  }
                buffer[bufpos++] = RED (c);
                c = phase4_getc ();
                if (!((RED (c) >= 'A' && RED (c) <= 'Z')
                      || (RED (c) >= 'a' && RED (c) <= 'z')
                      || (RED (c) >= '0' && RED (c) <= '9')
                      || RED (c) == '_'))
                  break;
              }
            phase4_ungetc (c);
            if (bufpos >= bufmax)
              {
                bufmax = 2 * bufmax + 10;
                buffer = xrealloc (buffer, bufmax);
              }
            buffer[bufpos] = '\0';
            tp->string = xstrdup (buffer);
            tp->type = token_type_symbol;
            return;
          }

        case '"':
          {
            int c2 = phase3_getc ();
            if (c2 == '"')
              {
                int c3 = phase3_getc ();
                if (c3 == '"')
                  {
                    /* Text block.  Specification:
                       <https://docs.oracle.com/javase/specs/jls/se13/preview/text-blocks.html>  */
                    struct mixed_string_buffer block;
                    unsigned int consecutive_unescaped_doublequotes;
                    mixed_string_ty *block_content;

                    /* Parse the part up to and including the first newline.  */
                    for (;;)
                      {
                        int ic = phase3_getc ();
                        if (ic == P2_EOF)
                          {
                            error_with_progname = false;
                            error (0, 0, _("%s:%d: warning: unterminated text block"),
                                   logical_file_name, line_number);
                            error_with_progname = true;
                            tp->type = token_type_other;
                            return;
                          }
                        if (RED (ic) == ' ' || RED (ic) == '\t' || RED (ic) == '\f')
                          ;
                        else if (RED (ic) == '\n')
                          break;
                        else
                          {
                            error_with_progname = false;
                            error (0, 0, _("%s:%d: warning: invalid syntax in text block"),
                                   logical_file_name, line_number);
                            error_with_progname = true;
                            tp->type = token_type_other;
                            return;
                          }
                      }

                    /* Parse the part after the first newline.  */
                    mixed_string_buffer_init (&block, lc_string,
                                              logical_file_name, line_number);
                    consecutive_unescaped_doublequotes = 0;
                    for (;;)
                      {
                        int ic = phase3_getc ();
                        if (RED (ic) == '"')
                          {
                            consecutive_unescaped_doublequotes++;
                            if (consecutive_unescaped_doublequotes == 3)
                              break;
                          }
                        else
                          {
                            while (consecutive_unescaped_doublequotes > 0)
                              {
                                mixed_string_buffer_append (&block, '"');
                                consecutive_unescaped_doublequotes--;
                              }
                            if (ic == P2_EOF)
                              {
                                error_with_progname = false;
                                error (0, 0, _("%s:%d: warning: unterminated text block"),
                                       logical_file_name, block.line_number);
                                error_with_progname = true;
                                break;
                              }
                            if (RED (ic) == '\\')
                              ic = do_getc_escaped ();
                            mixed_string_buffer_append (&block, ic);
                          }
                      }
                    block_content = mixed_string_buffer_result (&block);

                    /* Remove the common indentation from the content.  */
                    strip_indent (block_content);

                    tp->mixed_string = block_content;
                    tp->comment = add_reference (savable_comment);
                    tp->type = token_type_string_literal;
                    return;
                  }
                phase3_ungetc (c3);
              }
            phase3_ungetc (c2);
          }
          /* String literal.  */
          {
            struct mixed_string_buffer literal;

            mixed_string_buffer_init (&literal, lc_string,
                                      logical_file_name, line_number);
            accumulate_escaped (&literal, '"');
            tp->mixed_string = mixed_string_buffer_result (&literal);
            tp->comment = add_reference (savable_comment);
            tp->type = token_type_string_literal;
            return;
          }

        case '\'':
          /* Character literal.  */
          {
            struct mixed_string_buffer literal;

            mixed_string_buffer_init (&literal, lc_outside,
                                      logical_file_name, line_number);
            accumulate_escaped (&literal, '\'');
            mixed_string_buffer_destroy (&literal);
            tp->type = token_type_other;
            return;
          }

        case '+':
          c = phase4_getc ();
          if (RED (c) == '+')
            /* Operator ++ */
            tp->type = token_type_other;
          else if (RED (c) == '=')
            /* Operator += */
            tp->type = token_type_other;
          else
            {
              /* Operator + */
              phase4_ungetc (c);
              tp->type = token_type_plus;
            }
          return;

        default:
          /* Misc. operator.  */
          tp->type = token_type_other;
          return;
        }
    }
}

/* Supports 3 tokens of pushback.  */
static void
phase5_unget (token_ty *tp)
{
  if (tp->type != token_type_eof)
    {
      if (phase5_pushback_length == SIZEOF (phase5_pushback))
        abort ();
      phase5_pushback[phase5_pushback_length++] = *tp;
    }
}


/* Compile-time optimization of string literal concatenation.
   Combine "string1" + ... + "stringN" to the concatenated string if
     - the token before this expression is not ')' (because then the first
       string could be part of a cast expression),
     - the token after this expression is not '.' (because then the last
       string could be part of a method call expression).  */

static token_ty phase6_pushback[2];
static int phase6_pushback_length;

static token_type_ty phase6_last;

static void
phase6_get (token_ty *tp)
{
  if (phase6_pushback_length)
    {
      *tp = phase6_pushback[--phase6_pushback_length];
      return;
    }

  phase5_get (tp);
  if (tp->type == token_type_string_literal && phase6_last != token_type_rparen)
    {
      mixed_string_ty *sum = tp->mixed_string;

      for (;;)
        {
          token_ty token2;

          phase5_get (&token2);
          if (token2.type == token_type_plus)
            {
              token_ty token3;

              phase5_get (&token3);
              if (token3.type == token_type_string_literal)
                {
                  token_ty token_after;

                  phase5_get (&token_after);
                  if (token_after.type != token_type_dot)
                    {
                      sum = mixed_string_concat_free1 (sum, token3.mixed_string);

                      phase5_unget (&token_after);
                      free_token (&token3);
                      free_token (&token2);
                      continue;
                    }
                  phase5_unget (&token_after);
                }
              phase5_unget (&token3);
            }
          phase5_unget (&token2);
          break;
        }
      tp->mixed_string = sum;
    }
  phase6_last = tp->type;
}

/* Supports 2 tokens of pushback.  */
static void
phase6_unget (token_ty *tp)
{
  if (tp->type != token_type_eof)
    {
      if (phase6_pushback_length == SIZEOF (phase6_pushback))
        abort ();
      phase6_pushback[phase6_pushback_length++] = *tp;
    }
}


static void
x_java_lex (token_ty *tp)
{
  phase6_get (tp);
}

/* Supports 2 tokens of pushback.  */
static void
x_java_unlex (token_ty *tp)
{
  phase6_unget (tp);
}


/* ========================= Extracting strings.  ========================== */


/* Context lookup table.  */
static flag_context_list_table_ty *flag_context_list_table;


/* Maximum supported nesting depth.  */
#define MAX_NESTING_DEPTH 1000

/* Current nesting depths.  */
static int paren_nesting_depth;
static int brace_nesting_depth;


/* The file is broken into tokens.  Scan the token stream, looking for
   a keyword, followed by a left paren, followed by a string.  When we
   see this sequence, we have something to remember.  We assume we are
   looking at a valid C or C++ program, and leave the complaints about
   the grammar to the compiler.

     Normal handling: Look for
       keyword ( ... msgid ... )
     Plural handling: Look for
       keyword ( ... msgid ... msgid_plural ... )

   We use recursion because the arguments before msgid or between msgid
   and msgid_plural can contain subexpressions of the same form.  */


/* Extract messages until the next balanced closing parenthesis or brace,
   depending on TERMINATOR.
   Extracted messages are added to MLP.
   Return true upon eof, false upon closing parenthesis or brace.  */
static bool
extract_parenthesized (message_list_ty *mlp, token_type_ty terminator,
                       flag_context_ty outer_context,
                       flag_context_list_iterator_ty context_iter,
                       struct arglist_parser *argparser)
{
  /* Current argument number.  */
  int arg = 1;
  /* 0 when no keyword has been seen.  1 right after a keyword is seen.  */
  int state;
  /* Parameters of the keyword just seen.  Defined only in state 1.  */
  const struct callshapes *next_shapes = NULL;
  /* Context iterator that will be used if the next token is a '('.  */
  flag_context_list_iterator_ty next_context_iter =
    passthrough_context_list_iterator;
  /* Current context.  */
  flag_context_ty inner_context =
    inherited_context (outer_context,
                       flag_context_list_iterator_advance (&context_iter));

  /* Start state is 0.  */
  state = 0;

  for (;;)
    {
      token_ty token;

      x_java_lex (&token);
      switch (token.type)
        {
        case token_type_symbol:
          {
            /* Combine symbol1 . ... . symbolN to a single strings, so that
               we can recognize static function calls like
               GettextResource.gettext.  The information present for
               symbolI.....symbolN has precedence over the information for
               symbolJ.....symbolN with J > I.  */
            char *sum = token.string;
            size_t sum_len = strlen (sum);
            const char *dottedname;
            flag_context_list_ty *context_list;

            for (;;)
              {
                token_ty token2;

                x_java_lex (&token2);
                if (token2.type == token_type_dot)
                  {
                    token_ty token3;

                    x_java_lex (&token3);
                    if (token3.type == token_type_symbol)
                      {
                        char *addend = token3.string;
                        size_t addend_len = strlen (addend);

                        sum =
                          (char *) xrealloc (sum, sum_len + 1 + addend_len + 1);
                        sum[sum_len] = '.';
                        memcpy (sum + sum_len + 1, addend, addend_len + 1);
                        sum_len += 1 + addend_len;

                        free_token (&token3);
                        free_token (&token2);
                        continue;
                      }
                    x_java_unlex (&token3);
                  }
                x_java_unlex (&token2);
                break;
              }

            for (dottedname = sum;;)
              {
                void *keyword_value;

                if (hash_find_entry (&keywords, dottedname, strlen (dottedname),
                                     &keyword_value)
                    == 0)
                  {
                    next_shapes = (const struct callshapes *) keyword_value;
                    state = 1;
                    break;
                  }

                dottedname = strchr (dottedname, '.');
                if (dottedname == NULL)
                  {
                    state = 0;
                    break;
                  }
                dottedname++;
              }

            for (dottedname = sum;;)
              {
                context_list =
                  flag_context_list_table_lookup (
                    flag_context_list_table,
                    dottedname, strlen (dottedname));
                if (context_list != NULL)
                  break;

                dottedname = strchr (dottedname, '.');
                if (dottedname == NULL)
                  break;
                dottedname++;
              }
            next_context_iter = flag_context_list_iterator (context_list);

            free (sum);
            continue;
          }

        case token_type_lparen:
          if (++paren_nesting_depth > MAX_NESTING_DEPTH)
            {
              error_with_progname = false;
              error (EXIT_FAILURE, 0, _("%s:%d: error: too many open parentheses"),
                     logical_file_name, line_number);
            }
          if (extract_parenthesized (mlp, token_type_rparen,
                                     inner_context, next_context_iter,
                                     arglist_parser_alloc (mlp,
                                                           state ? next_shapes : NULL)))
            {
              arglist_parser_done (argparser, arg);
              return true;
            }
          paren_nesting_depth--;
          next_context_iter = null_context_list_iterator;
          state = 0;
          continue;

        case token_type_rparen:
          if (terminator == token_type_rparen)
            {
              arglist_parser_done (argparser, arg);
              return false;
            }
          if (terminator == token_type_rbrace)
            {
              error_with_progname = false;
              error (0, 0,
                     _("%s:%d: warning: ')' found where '}' was expected"),
                     logical_file_name, token.line_number);
              error_with_progname = true;
            }
          next_context_iter = null_context_list_iterator;
          state = 0;
          continue;

        case token_type_lbrace:
          if (++brace_nesting_depth > MAX_NESTING_DEPTH)
            {
              error_with_progname = false;
              error (EXIT_FAILURE, 0, _("%s:%d: error: too many open braces"),
                     logical_file_name, line_number);
            }
          if (extract_parenthesized (mlp, token_type_rbrace,
                                     null_context, null_context_list_iterator,
                                     arglist_parser_alloc (mlp, NULL)))
            {
              arglist_parser_done (argparser, arg);
              return true;
            }
          brace_nesting_depth--;
          next_context_iter = null_context_list_iterator;
          state = 0;
          continue;

        case token_type_rbrace:
          if (terminator == token_type_rbrace)
            {
              arglist_parser_done (argparser, arg);
              return false;
            }
          if (terminator == token_type_rparen)
            {
              error_with_progname = false;
              error (0, 0,
                     _("%s:%d: warning: '}' found where ')' was expected"),
                     logical_file_name, token.line_number);
              error_with_progname = true;
            }
          next_context_iter = null_context_list_iterator;
          state = 0;
          continue;

        case token_type_comma:
          arg++;
          inner_context =
            inherited_context (outer_context,
                               flag_context_list_iterator_advance (
                                 &context_iter));
          next_context_iter = passthrough_context_list_iterator;
          state = 0;
          continue;

        case token_type_string_literal:
          {
            lex_pos_ty pos;

            pos.file_name = logical_file_name;
            pos.line_number = token.line_number;

            if (extract_all)
              {
                char *string = mixed_string_contents (token.mixed_string);
                mixed_string_free (token.mixed_string);
                remember_a_message (mlp, NULL, string, true, false,
                                    inner_context, &pos,
                                    NULL, token.comment, true);
              }
            else
              arglist_parser_remember (argparser, arg, token.mixed_string,
                                       inner_context,
                                       pos.file_name, pos.line_number,
                                       token.comment, true);
          }
          drop_reference (token.comment);
          next_context_iter = null_context_list_iterator;
          state = 0;
          continue;

        case token_type_eof:
          arglist_parser_done (argparser, arg);
          return true;

        case token_type_dot:
        case token_type_number:
        case token_type_plus:
        case token_type_other:
          next_context_iter = null_context_list_iterator;
          state = 0;
          continue;

        default:
          abort ();
        }
    }
}


void
extract_java (FILE *f,
              const char *real_filename, const char *logical_filename,
              flag_context_list_table_ty *flag_table,
              msgdomain_list_ty *mdlp)
{
  message_list_ty *mlp = mdlp->item[0]->messages;

  fp = f;
  real_file_name = real_filename;
  logical_file_name = xstrdup (logical_filename);
  line_number = 1;

  phase1_pushback_length = 0;
  phase2_pushback_length = 0;
  phase3_pushback_length = 0;

  last_comment_line = -1;
  last_non_comment_line = -1;

  phase5_pushback_length = 0;
  phase6_pushback_length = 0;
  phase6_last = token_type_eof;

  flag_context_list_table = flag_table;
  paren_nesting_depth = 0;
  brace_nesting_depth = 0;

  init_keywords ();

  /* Eat tokens until eof is seen.  When extract_parenthesized returns
     due to an unbalanced closing parenthesis, just restart it.  */
  while (!extract_parenthesized (mlp, token_type_eof,
                                 null_context, null_context_list_iterator,
                                 arglist_parser_alloc (mlp, NULL)))
    ;

  fp = NULL;
  real_file_name = NULL;
  logical_file_name = NULL;
  line_number = 0;
}
