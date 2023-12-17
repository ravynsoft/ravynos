/* xgettext Python backend.
   Copyright (C) 2002-2003, 2005-2011, 2013-2014, 2018-2023 Free Software Foundation, Inc.

   This file was written by Bruno Haible <haible@clisp.cons.org>, 2002.

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
#include "x-python.h"

#include <assert.h>
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
#include "progname.h"
#include "basename-lgpl.h"
#include "xerror.h"
#include "xvasprintf.h"
#include "xalloc.h"
#include "c-strstr.h"
#include "c-ctype.h"
#include "po-charset.h"
#include "uniname.h"
#include "unistr.h"
#include "gettext.h"

#define _(s) gettext(s)

#undef max /* clean up after MSVC's <stdlib.h> */
#define max(a,b) ((a) > (b) ? (a) : (b))

#define SIZEOF(a) (sizeof(a) / sizeof(a[0]))


/* The Python syntax is defined in the Python Reference Manual
   /usr/share/doc/packages/python/html/ref/index.html.
   See also Python-2.0/Parser/tokenizer.c, Python-2.0/Python/compile.c,
   Python-2.0/Objects/unicodeobject.c.  */


/* ====================== Keyword set customization.  ====================== */

/* If true extract all strings.  */
static bool extract_all = false;

static hash_table keywords;
static bool default_keywords = true;


void
x_python_extract_all ()
{
  extract_all = true;
}


void
x_python_keyword (const char *name)
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

      /* The characters between name and end should form a valid C identifier.
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
      x_python_keyword ("gettext");
      x_python_keyword ("ugettext");
      x_python_keyword ("dgettext:2");
      x_python_keyword ("ngettext:1,2");
      x_python_keyword ("ungettext:1,2");
      x_python_keyword ("dngettext:2,3");
      x_python_keyword ("_");
      default_keywords = false;
    }
}

void
init_flag_table_python ()
{
  xgettext_record_flag ("gettext:1:pass-python-format");
  xgettext_record_flag ("ugettext:1:pass-python-format");
  xgettext_record_flag ("dgettext:2:pass-python-format");
  xgettext_record_flag ("ngettext:1:pass-python-format");
  xgettext_record_flag ("ngettext:2:pass-python-format");
  xgettext_record_flag ("ungettext:1:pass-python-format");
  xgettext_record_flag ("ungettext:2:pass-python-format");
  xgettext_record_flag ("dngettext:2:pass-python-format");
  xgettext_record_flag ("dngettext:3:pass-python-format");
  xgettext_record_flag ("_:1:pass-python-format");
  /* xgettext_record_flag ("%:1:python-format"); // % is an infix operator! */

  xgettext_record_flag ("gettext:1:pass-python-brace-format");
  xgettext_record_flag ("ugettext:1:pass-python-brace-format");
  xgettext_record_flag ("dgettext:2:pass-python-brace-format");
  xgettext_record_flag ("ngettext:1:pass-python-brace-format");
  xgettext_record_flag ("ngettext:2:pass-python-brace-format");
  xgettext_record_flag ("ungettext:1:pass-python-brace-format");
  xgettext_record_flag ("ungettext:2:pass-python-brace-format");
  xgettext_record_flag ("dngettext:2:pass-python-brace-format");
  xgettext_record_flag ("dngettext:3:pass-python-brace-format");
  xgettext_record_flag ("_:1:pass-python-brace-format");
  /* xgettext_record_flag ("format:1:python-brace-format"); */
}


/* ======================== Reading of characters.  ======================== */

/* The input file stream.  */
static FILE *fp;


/* 0. Terminate line by \n, regardless whether the external
   representation of a line terminator is CR (Mac), and CR/LF
   (DOS/Windows), as Python treats them equally.  */
static int
phase0_getc ()
{
  int c;

  c = getc (fp);
  if (c == EOF)
    {
      if (ferror (fp))
        error (EXIT_FAILURE, errno, _("error while reading \"%s\""),
               real_file_name);
      return EOF;
    }

  if (c == '\r')
    {
      int c1 = getc (fp);

      if (c1 != EOF && c1 != '\n')
        ungetc (c1, fp);

      /* Seen line terminator CR or CR/LF.  */
      return '\n';
    }

  return c;
}

/* Supports only one pushback character, and not '\n'.  */
static inline void
phase0_ungetc (int c)
{
  if (c != EOF)
    ungetc (c, fp);
}


/* 1. line_number handling.  */

/* Maximum used, roughly a safer MB_LEN_MAX.  */
#define MAX_PHASE1_PUSHBACK 16
static unsigned char phase1_pushback[MAX_PHASE1_PUSHBACK];
static int phase1_pushback_length;

/* Read the next single byte from the input file.  */
static int
phase1_getc ()
{
  int c;

  if (phase1_pushback_length)
    c = phase1_pushback[--phase1_pushback_length];
  else
    c = phase0_getc ();

  if (c == '\n')
    ++line_number;

  return c;
}

/* Supports MAX_PHASE1_PUSHBACK characters of pushback.  */
static void
phase1_ungetc (int c)
{
  if (c != EOF)
    {
      if (c == '\n')
        --line_number;

      if (phase1_pushback_length == SIZEOF (phase1_pushback))
        abort ();
      phase1_pushback[phase1_pushback_length++] = c;
    }
}


/* Phase 2: Conversion to Unicode.
   This is done early because PEP 0263 specifies that conversion to Unicode
   conceptually occurs before tokenization.  A test case where it matters
   is with encodings like BIG5: when a double-byte character ending in 0x5C
   is followed by '\' or 'u0021', the tokenizer must not treat the second
   half of the double-byte character as a backslash.  */

/* End-of-file indicator for functions returning an UCS-4 character.  */
#define UEOF -1

static lexical_context_ty lexical_context;

static int phase2_pushback[max (9, UNINAME_MAX + 3)];
static int phase2_pushback_length;

/* Read the next Unicode UCS-4 character from the input file.  */
static int
phase2_getc ()
{
  if (phase2_pushback_length)
    return phase2_pushback[--phase2_pushback_length];

  if (xgettext_current_source_encoding == po_charset_ascii)
    {
      int c = phase1_getc ();
      if (c == EOF)
        return UEOF;
      if (!c_isascii (c))
        {
          multiline_error (xstrdup (""),
                           xasprintf ("%s\n%s\n",
                                      non_ascii_error_message (lexical_context,
                                                               real_file_name,
                                                               line_number),
                                      _("\
Please specify the source encoding through --from-code or through a comment\n\
as specified in https://www.python.org/peps/pep-0263.html.\n")));
          exit (EXIT_FAILURE);
        }
      return c;
    }
  else if (xgettext_current_source_encoding != po_charset_utf8)
    {
#if HAVE_ICONV
      /* Use iconv on an increasing number of bytes.  Read only as many bytes
         through phase1_getc as needed.  This is needed to give reasonable
         interactive behaviour when fp is connected to an interactive tty.  */
      unsigned char buf[MAX_PHASE1_PUSHBACK];
      size_t bufcount;

      {
        int c = phase1_getc ();
        if (c == EOF)
          return UEOF;
        buf[0] = (unsigned char) c;
        bufcount = 1;
      }

      for (;;)
        {
          unsigned char scratchbuf[6];
          const char *inptr = (const char *) &buf[0];
          size_t insize = bufcount;
          char *outptr = (char *) &scratchbuf[0];
          size_t outsize = sizeof (scratchbuf);

          size_t res = iconv (xgettext_current_source_iconv,
                              (ICONV_CONST char **) &inptr, &insize,
                              &outptr, &outsize);
          /* We expect that a character has been produced if and only if
             some input bytes have been consumed.  */
          if ((insize < bufcount) != (outsize < sizeof (scratchbuf)))
            abort ();
          if (outsize == sizeof (scratchbuf))
            {
              /* No character has been produced.  Must be an error.  */
              if (res != (size_t)(-1))
                abort ();

              if (errno == EILSEQ)
                {
                  /* An invalid multibyte sequence was encountered.  */
                  goto invalid;
                }
              else if (errno == EINVAL)
                {
                  /* An incomplete multibyte character.  */
                  int c;

                  if (bufcount == MAX_PHASE1_PUSHBACK)
                    {
                      /* An overlong incomplete multibyte sequence was
                         encountered.  */
                      multiline_error (xstrdup (""),
                                       xasprintf (_("\
%s:%d: Long incomplete multibyte sequence.\n\
Please specify the correct source encoding through --from-code or through a\n\
comment as specified in https://www.python.org/peps/pep-0263.html.\n"),
                                       real_file_name, line_number));
                      exit (EXIT_FAILURE);
                    }

                  /* Read one more byte and retry iconv.  */
                  c = phase1_getc ();
                  if (c == EOF)
                    goto incomplete_at_eof;
                  if (c == '\n')
                    goto incomplete_at_eol;
                  buf[bufcount++] = (unsigned char) c;
                }
              else
                error (EXIT_FAILURE, errno, _("%s:%d: iconv failure"),
                       real_file_name, line_number);
            }
          else
            {
              size_t outbytes = sizeof (scratchbuf) - outsize;
              size_t bytes = bufcount - insize;
              ucs4_t uc;

              /* We expect that one character has been produced.  */
              if (bytes == 0)
                abort ();
              if (outbytes == 0)
                abort ();
              /* Push back the unused bytes.  */
              while (insize > 0)
                phase1_ungetc (buf[--insize]);
              /* Convert the character from UTF-8 to UCS-4.  */
              if (u8_mbtoucr (&uc, scratchbuf, outbytes) < (int) outbytes)
                {
                  /* scratchbuf contains an out-of-range Unicode character
                     (> 0x10ffff).  */
                  goto invalid;
                }
              return uc;
            }
        }
#else
      /* If we don't have iconv(), the only supported values for
         xgettext_global_source_encoding and thus also for
         xgettext_current_source_encoding are ASCII and UTF-8.  */
      abort ();
#endif
    }
  else
    {
      /* Read an UTF-8 encoded character.
         Reject invalid input, like u8_mbtouc does.  */
      int c;
      ucs4_t uc;

      c = phase1_getc ();
      if (c == EOF)
        return UEOF;
      if (c < 0x80)
        {
          uc = c;
        }
      else if (c < 0xc2)
        goto invalid;
      else if (c < 0xe0)
        {
          int c1 = phase1_getc ();
          if (c1 == EOF)
            goto incomplete_at_eof;
          if (c1 == '\n')
            goto incomplete_at_eol;
          if ((c1 ^ 0x80) < 0x40)
            uc = ((unsigned int) (c & 0x1f) << 6)
                 | (unsigned int) (c1 ^ 0x80);
          else
            goto invalid;
        }
      else if (c < 0xf0)
        {
          int c1 = phase1_getc ();
          if (c1 == EOF)
            goto incomplete_at_eof;
          if (c1 == '\n')
            goto incomplete_at_eol;
          if ((c1 ^ 0x80) < 0x40
              && (c >= 0xe1 || c1 >= 0xa0)
              && (c != 0xed || c1 < 0xa0))
            {
              int c2 = phase1_getc ();
              if (c2 == EOF)
                goto incomplete_at_eof;
              if (c2 == '\n')
                goto incomplete_at_eol;
              if ((c2 ^ 0x80) < 0x40)
                uc = ((unsigned int) (c & 0x0f) << 12)
                     | ((unsigned int) (c1 ^ 0x80) << 6)
                     | (unsigned int) (c2 ^ 0x80);
              else
                goto invalid;
            }
          else
            goto invalid;
        }
      else if (c < 0xf8)
        {
          int c1 = phase1_getc ();
          if (c1 == EOF)
            goto incomplete_at_eof;
          if (c1 == '\n')
            goto incomplete_at_eol;
          if ((c1 ^ 0x80) < 0x40
              && (c >= 0xf1 || c1 >= 0x90)
              && (c < 0xf4 || (c == 0xf4 && c1 < 0x90)))
            {
              int c2 = phase1_getc ();
              if (c2 == EOF)
                goto incomplete_at_eof;
              if (c2 == '\n')
                goto incomplete_at_eol;
              if ((c2 ^ 0x80) < 0x40)
                {
                  int c3 = phase1_getc ();
                  if (c3 == EOF)
                    goto incomplete_at_eof;
                  if (c3 == '\n')
                    goto incomplete_at_eol;
                  if ((c3 ^ 0x80) < 0x40)
                    uc = ((unsigned int) (c & 0x07) << 18)
                         | ((unsigned int) (c1 ^ 0x80) << 12)
                         | ((unsigned int) (c2 ^ 0x80) << 6)
                         | (unsigned int) (c3 ^ 0x80);
                  else
                    goto invalid;
                }
              else
                goto invalid;
            }
          else
            goto invalid;
        }
      else
        goto invalid;

      return uc;
    }

 invalid:
  /* An invalid multibyte sequence was encountered.  */
  multiline_error (xstrdup (""),
                   xasprintf (_("\
%s:%d: Invalid multibyte sequence.\n\
Please specify the correct source encoding through --from-code or through a\n\
comment as specified in https://www.python.org/peps/pep-0263.html.\n"),
                   real_file_name, line_number));
  exit (EXIT_FAILURE);

 incomplete_at_eof:
  multiline_error (xstrdup (""),
                   xasprintf (_("\
%s:%d: Incomplete multibyte sequence at end of file.\n\
Please specify the correct source encoding through --from-code or through a\n\
comment as specified in https://www.python.org/peps/pep-0263.html.\n"),
                   real_file_name, line_number));
  exit (EXIT_FAILURE);

 incomplete_at_eol:
  multiline_error (xstrdup (""),
                   xasprintf (_("\
%s:%d: Incomplete multibyte sequence at end of line.\n\
Please specify the correct source encoding through --from-code or through a\n\
comment as specified in https://www.python.org/peps/pep-0263.html.\n"),
                   real_file_name, line_number - 1));
  exit (EXIT_FAILURE);
}

/* Supports max (9, UNINAME_MAX + 3) pushback characters.  */
static void
phase2_ungetc (int c)
{
  if (c != UEOF)
    {
      if (phase2_pushback_length == SIZEOF (phase2_pushback))
        abort ();
      phase2_pushback[phase2_pushback_length++] = c;
    }
}


/* ========================= Accumulating strings.  ======================== */

/* See xg-mixed-string.h for the API.  */


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
  mixed_string_buffer_append_unicode (&comment_buffer, c);
}

static inline const char *
comment_line_end ()
{
  char *buffer =
    mixed_string_contents_free1 (mixed_string_buffer_result (&comment_buffer));
  size_t buflen = strlen (buffer);

  while (buflen >= 1
         && (buffer[buflen - 1] == ' ' || buffer[buflen - 1] == '\t'))
    --buflen;
  buffer[buflen] = '\0';
  savable_comment_add (buffer);
  lexical_context = lc_outside;
  return buffer;
}


/* These are for tracking whether comments count as immediately before
   keyword.  */
static int last_comment_line;
static int last_non_comment_line;


/* ======================== Recognizing comments.  ======================== */


/* Recognizing the "coding" comment.
   As specified in PEP 0263, it takes the form
     "coding" [":"|"="] {alphanumeric or "-" or "_" or "*"}*
   or
     "set" "fileencoding" "=" {alphanumeric or "-" or "_" or "*"}*
   and is located in a comment in a line that
     - is either the first or second line,
     - is not a continuation line,
     - in the first form, contains no other tokens except this comment.  */

/* Canonicalized encoding name for the current input file.  */
static const char *xgettext_current_file_source_encoding;

#if HAVE_ICONV
/* Converter from xgettext_current_file_source_encoding to UTF-8 (except from
   ASCII or UTF-8, when this conversion is a no-op).  */
static iconv_t xgettext_current_file_source_iconv;
#endif

static inline void
set_current_file_source_encoding (const char *canon_encoding)
{
  xgettext_current_file_source_encoding = canon_encoding;

  if (xgettext_current_file_source_encoding != po_charset_ascii
      && xgettext_current_file_source_encoding != po_charset_utf8)
    {
#if HAVE_ICONV
      iconv_t cd;

      /* Avoid glibc-2.1 bug with EUC-KR.  */
# if ((__GLIBC__ == 2 && __GLIBC_MINOR__ <= 1) && !defined __UCLIBC__) \
     && !defined _LIBICONV_VERSION
      if (strcmp (xgettext_current_file_source_encoding, "EUC-KR") == 0)
        cd = (iconv_t)(-1);
      else
# endif
      cd = iconv_open (po_charset_utf8, xgettext_current_file_source_encoding);
      if (cd == (iconv_t)(-1))
        error_at_line (EXIT_FAILURE, 0, logical_file_name, line_number - 1,
                       _("Cannot convert from \"%s\" to \"%s\". %s relies on iconv(), and iconv() does not support this conversion."),
                       xgettext_current_file_source_encoding, po_charset_utf8,
                       last_component (program_name));
      xgettext_current_file_source_iconv = cd;
#else
      error_at_line (EXIT_FAILURE, 0, logical_file_name, line_number - 1,
                     _("Cannot convert from \"%s\" to \"%s\". %s relies on iconv(). This version was built without iconv()."),
                     xgettext_current_file_source_encoding, po_charset_utf8,
                     last_component (program_name));
#endif
    }

  xgettext_current_source_encoding = xgettext_current_file_source_encoding;
#if HAVE_ICONV
  xgettext_current_source_iconv = xgettext_current_file_source_iconv;
#endif
}

static inline void
try_to_extract_coding (const char *comment)
{
  const char *p = c_strstr (comment, "coding");

  if (p != NULL)
    {
      p += 6;
      if (*p == ':' || *p == '=')
        {
          p++;
          while (*p == ' ' || *p == '\t')
            p++;
          {
            const char *encoding_start = p;

            while (c_isalnum (*p) || *p == '-' || *p == '_' || *p == '.')
              p++;
            {
              const char *encoding_end = p;

              if (encoding_end > encoding_start)
                {
                  /* Extract the encoding string.  */
                  size_t encoding_len = encoding_end - encoding_start;
                  char *encoding = XNMALLOC (encoding_len + 1, char);

                  memcpy (encoding, encoding_start, encoding_len);
                  encoding[encoding_len] = '\0';

                  {
                    /* Canonicalize it.  */
                    const char *canon_encoding = po_charset_canonicalize (encoding);
                    if (canon_encoding == NULL)
                      {
                        error_at_line (0, 0,
                                       logical_file_name, line_number - 1,
                                       _("Unknown encoding \"%s\". Proceeding with ASCII instead."),
                                       encoding);
                        canon_encoding = po_charset_ascii;
                      }

                    /* Activate it.  */
                    set_current_file_source_encoding (canon_encoding);
                  }

                  free (encoding);
                }
            }
          }
        }
    }
}

/* Tracking whether the current line is a continuation line or contains a
   non-blank character.  */
static bool continuation_or_nonblank_line;


/* Phase 3: Outside strings, replace backslash-newline with nothing and a
   comment with nothing.  */

static int
phase3_getc ()
{
  int c;

  for (;;)
    {
      c = phase2_getc ();
      if (c == '\\')
        {
          c = phase2_getc ();
          if (c != '\n')
            {
              phase2_ungetc (c);
              /* This shouldn't happen usually, because "A backslash is
                 illegal elsewhere on a line outside a string literal."  */
              return '\\';
            }
          /* Eat backslash-newline.  */
          continuation_or_nonblank_line = true;
        }
      else if (c == '#')
        {
          /* Eat a comment.  */
          const char *comment;

          last_comment_line = line_number;
          comment_start ();
          for (;;)
            {
              c = phase2_getc ();
              if (c == UEOF || c == '\n')
                break;
              /* We skip all leading white space, but not EOLs.  */
              if (!(comment_at_start () && (c == ' ' || c == '\t')))
                comment_add (c);
            }
          comment = comment_line_end ();
          if (line_number - 1 <= 2 && !continuation_or_nonblank_line)
            try_to_extract_coding (comment);
          continuation_or_nonblank_line = false;
          return c;
        }
      else
        {
          if (c == '\n')
            continuation_or_nonblank_line = false;
          else if (!(c == ' ' || c == '\t' || c == '\f'))
            continuation_or_nonblank_line = true;
          return c;
        }
    }
}

/* Supports only one pushback character.  */
static void
phase3_ungetc (int c)
{
  phase2_ungetc (c);
}


/* ========================= Accumulating strings.  ======================== */

/* Return value of phase7_getuc when EOF is reached.  */
#define P7_EOF (-1)
#define P7_STRING_END (-2)

/* Convert an UTF-16 or UTF-32 code point to a return value that can be
   distinguished from a single-byte return value.  */
#define UNICODE(code) (0x100 + (code))

/* Test a return value of phase7_getuc whether it designates an UTF-16 or
   UTF-32 code point.  */
#define IS_UNICODE(p7_result) ((p7_result) >= 0x100)

/* Extract the UTF-16 or UTF-32 code of a return value that satisfies
   IS_UNICODE.  */
#define UNICODE_VALUE(p7_result) ((p7_result) - 0x100)


/* ========================== Reading of tokens.  ========================== */


enum token_type_ty
{
  token_type_eof,
  token_type_lparen,            /* ( */
  token_type_rparen,            /* ) */
  token_type_comma,             /* , */
  token_type_lbracket,          /* [ */
  token_type_rbracket,          /* ] */
  token_type_string,            /* "abc", 'abc', """abc""", '''abc''' */
  token_type_symbol,            /* symbol, number */
  token_type_plus,              /* + */
  token_type_other              /* misc. operator */
};
typedef enum token_type_ty token_type_ty;

typedef struct token_ty token_ty;
struct token_ty
{
  token_type_ty type;
  char *string;                         /* for token_type_symbol */
  mixed_string_ty *mixed_string;        /* for token_type_string */
  refcounted_string_list_ty *comment;   /* for token_type_string */
  int line_number;
};

/* Free the memory pointed to by a 'struct token_ty'.  */
static inline void
free_token (token_ty *tp)
{
  if (tp->type == token_type_symbol)
    free (tp->string);
  if (tp->type == token_type_string)
    {
      mixed_string_free (tp->mixed_string);
      drop_reference (tp->comment);
    }
}


/* There are two different input syntaxes for strings, "abc" and r"abc",
   and two different input syntaxes for Unicode strings, u"abc" and ur"abc".
   Which escape sequences are understood, i.e. what is interpreted specially
   after backslash?
    "abc"     \<nl> \\ \' \" \a\b\f\n\r\t\v \ooo \xnn
    r"abc"
    u"abc"    \<nl> \\ \' \" \a\b\f\n\r\t\v \ooo \xnn \unnnn \Unnnnnnnn \N{...}
    ur"abc"                                           \unnnn
   The \unnnn values are UTF-16 values; a single \Unnnnnnnn can expand to two
   \unnnn items.  The \ooo and \xnn values are in the current source encoding
   for byte strings, and Unicode code points for Unicode strings.
 */

static int
phase7_getuc (int quote_char,
              bool triple, bool interpret_ansic, bool interpret_unicode,
              unsigned int *backslash_counter)
{
  int c;

  for (;;)
    {
      /* Use phase 2, because phase 3 elides comments.  */
      c = phase2_getc ();

      if (c == UEOF)
        return P7_EOF;

      if (c == quote_char && (interpret_ansic || (*backslash_counter & 1) == 0))
        {
          if (triple)
            {
              int c1 = phase2_getc ();
              if (c1 == quote_char)
                {
                  int c2 = phase2_getc ();
                  if (c2 == quote_char)
                    return P7_STRING_END;
                  phase2_ungetc (c2);
                }
              phase2_ungetc (c1);
              return UNICODE (c);
            }
          else
            return P7_STRING_END;
        }

      if (c == '\n')
        {
          if (triple)
            {
              *backslash_counter = 0;
              return UNICODE ('\n');
            }
          /* In r"..." and ur"..." strings, newline is only allowed
             immediately after an odd number of backslashes (although the
             backslashes are not interpreted!).  */
          if (!(interpret_ansic || (*backslash_counter & 1) == 0))
            {
              *backslash_counter = 0;
              return UNICODE ('\n');
            }
          phase2_ungetc (c);
          error_with_progname = false;
          error (0, 0, _("%s:%d: warning: unterminated string"),
                 logical_file_name, line_number);
          error_with_progname = true;
          return P7_STRING_END;
        }

      if (c != '\\')
        {
          *backslash_counter = 0;
          return UNICODE (c);
        }

      /* Backslash handling.  */

      if (!interpret_ansic && !interpret_unicode)
        {
          ++*backslash_counter;
          return UNICODE ('\\');
        }

      /* Dispatch according to the character following the backslash.  */
      c = phase2_getc ();
      if (c == UEOF)
        {
          ++*backslash_counter;
          return UNICODE ('\\');
        }

      if (interpret_ansic)
        switch (c)
          {
          case '\n':
            continue;
          case '\\':
            ++*backslash_counter;
            return UNICODE (c);
          case '\'': case '"':
            *backslash_counter = 0;
            return UNICODE (c);
          case 'a':
            *backslash_counter = 0;
            return UNICODE ('\a');
          case 'b':
            *backslash_counter = 0;
            return UNICODE ('\b');
          case 'f':
            *backslash_counter = 0;
            return UNICODE ('\f');
          case 'n':
            *backslash_counter = 0;
            return UNICODE ('\n');
          case 'r':
            *backslash_counter = 0;
            return UNICODE ('\r');
          case 't':
            *backslash_counter = 0;
            return UNICODE ('\t');
          case 'v':
            *backslash_counter = 0;
            return UNICODE ('\v');
          case '0': case '1': case '2': case '3': case '4':
          case '5': case '6': case '7':
            {
              int n = c - '0';

              c = phase2_getc ();
              if (c != UEOF)
                {
                  if (c >= '0' && c <= '7')
                    {
                      n = (n << 3) + (c - '0');
                      c = phase2_getc ();
                      if (c != UEOF)
                        {
                          if (c >= '0' && c <= '7')
                            n = (n << 3) + (c - '0');
                          else
                            phase2_ungetc (c);
                        }
                    }
                  else
                    phase2_ungetc (c);
                }
              *backslash_counter = 0;
              if (interpret_unicode)
                return UNICODE (n);
              else
                return (unsigned char) n;
            }
          case 'x':
            {
              int c1 = phase2_getc ();
              int n1;

              if (c1 >= '0' && c1 <= '9')
                n1 = c1 - '0';
              else if (c1 >= 'A' && c1 <= 'F')
                n1 = c1 - 'A' + 10;
              else if (c1 >= 'a' && c1 <= 'f')
                n1 = c1 - 'a' + 10;
              else
                n1 = -1;

              if (n1 >= 0)
                {
                  int c2 = phase2_getc ();
                  int n2;

                  if (c2 >= '0' && c2 <= '9')
                    n2 = c2 - '0';
                  else if (c2 >= 'A' && c2 <= 'F')
                    n2 = c2 - 'A' + 10;
                  else if (c2 >= 'a' && c2 <= 'f')
                    n2 = c2 - 'a' + 10;
                  else
                    n2 = -1;

                  if (n2 >= 0)
                    {
                      int n = (n1 << 4) + n2;
                      *backslash_counter = 0;
                      if (interpret_unicode)
                        return UNICODE (n);
                      else
                        return (unsigned char) n;
                    }

                  phase2_ungetc (c2);
                }
              phase2_ungetc (c1);
              phase2_ungetc (c);
              ++*backslash_counter;
              return UNICODE ('\\');
            }
          }

      if (interpret_unicode)
        {
          if (c == 'u')
            {
              unsigned char buf[4];
              unsigned int n = 0;
              int i;

              for (i = 0; i < 4; i++)
                {
                  int c1 = phase2_getc ();

                  if (c1 >= '0' && c1 <= '9')
                    n = (n << 4) + (c1 - '0');
                  else if (c1 >= 'A' && c1 <= 'F')
                    n = (n << 4) + (c1 - 'A' + 10);
                  else if (c1 >= 'a' && c1 <= 'f')
                    n = (n << 4) + (c1 - 'a' + 10);
                  else
                    {
                      phase2_ungetc (c1);
                      while (--i >= 0)
                        phase2_ungetc (buf[i]);
                      phase2_ungetc (c);
                      ++*backslash_counter;
                      return UNICODE ('\\');
                    }

                  buf[i] = c1;
                }
              *backslash_counter = 0;
              return UNICODE (n);
            }

          if (interpret_ansic)
            {
              if (c == 'U')
                {
                  unsigned char buf[8];
                  unsigned int n = 0;
                  int i;

                  for (i = 0; i < 8; i++)
                    {
                      int c1 = phase2_getc ();

                      if (c1 >= '0' && c1 <= '9')
                        n = (n << 4) + (c1 - '0');
                      else if (c1 >= 'A' && c1 <= 'F')
                        n = (n << 4) + (c1 - 'A' + 10);
                      else if (c1 >= 'a' && c1 <= 'f')
                        n = (n << 4) + (c1 - 'a' + 10);
                      else
                        {
                          phase2_ungetc (c1);
                          while (--i >= 0)
                            phase2_ungetc (buf[i]);
                          phase2_ungetc (c);
                          ++*backslash_counter;
                          return UNICODE ('\\');
                        }

                      buf[i] = c1;
                    }
                  if (n < 0x110000)
                    {
                      *backslash_counter = 0;
                      return UNICODE (n);
                    }

                  error_with_progname = false;
                  error (0, 0, _("%s:%d: warning: invalid Unicode character"),
                         logical_file_name, line_number);
                  error_with_progname = true;

                  while (--i >= 0)
                    phase2_ungetc (buf[i]);
                  phase2_ungetc (c);
                  ++*backslash_counter;
                  return UNICODE ('\\');
                }

              if (c == 'N')
                {
                  int c1 = phase2_getc ();
                  if (c1 == '{')
                    {
                      unsigned char buf[UNINAME_MAX + 1];
                      int i;
                      unsigned int n;

                      for (i = 0; i < UNINAME_MAX; i++)
                        {
                          int c2 = phase2_getc ();
                          if (!(c2 >= ' ' && c2 <= '~'))
                            {
                              phase2_ungetc (c2);
                              while (--i >= 0)
                                phase2_ungetc (buf[i]);
                              phase2_ungetc (c1);
                              phase2_ungetc (c);
                              ++*backslash_counter;
                              return UNICODE ('\\');
                            }
                          if (c2 == '}')
                            break;
                          buf[i] = c2;
                        }
                      buf[i] = '\0';

                      n = unicode_name_character ((char *) buf);
                      if (n != UNINAME_INVALID)
                        {
                          *backslash_counter = 0;
                          return UNICODE (n);
                        }

                      phase2_ungetc ('}');
                      while (--i >= 0)
                        phase2_ungetc (buf[i]);
                    }
                  phase2_ungetc (c1);
                  phase2_ungetc (c);
                  ++*backslash_counter;
                  return UNICODE ('\\');
                }
            }
        }

      phase2_ungetc (c);
      ++*backslash_counter;
      return UNICODE ('\\');
    }
}


/* Combine characters into tokens.  Discard whitespace except newlines at
   the end of logical lines.  */

/* Number of pending open parentheses/braces/brackets.  */
static int open_pbb;

static token_ty phase5_pushback[2];
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

  for (;;)
    {
      tp->line_number = line_number;
      c = phase3_getc ();

      switch (c)
        {
        case UEOF:
          tp->type = token_type_eof;
          return;

        case ' ':
        case '\t':
        case '\f':
          /* Ignore whitespace and comments.  */
          continue;

        case '\n':
          if (last_non_comment_line > last_comment_line)
            savable_comment_reset ();
          /* Ignore newline if and only if it is used for implicit line
             joining.  */
          if (open_pbb > 0)
            continue;
          tp->type = token_type_other;
          return;
        }

      last_non_comment_line = tp->line_number;

      switch (c)
        {
        case '.':
          {
            int c1 = phase3_getc ();
            phase3_ungetc (c1);
            if (!(c1 >= '0' && c1 <= '9'))
              {

                tp->type = token_type_other;
                return;
              }
          }
          FALLTHROUGH;
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
        case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
        case 'M': case 'N': case 'O': case 'P': case 'Q':
        case 'S': case 'T':           case 'V': case 'W': case 'X':
        case 'Y': case 'Z':
        case '_':
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
        case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
        case 'm': case 'n': case 'o': case 'p': case 'q':
        case 's': case 't':           case 'v': case 'w': case 'x':
        case 'y': case 'z':
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
        symbol:
          /* Symbol, or part of a number.  */
          {
            static char *buffer;
            static int bufmax;
            int bufpos;

            bufpos = 0;
            for (;;)
              {
                if (bufpos >= bufmax)
                  {
                    bufmax = 2 * bufmax + 10;
                    buffer = xrealloc (buffer, bufmax);
                  }
                buffer[bufpos++] = c;
                c = phase3_getc ();
                switch (c)
                  {
                  case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
                  case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
                  case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
                  case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
                  case 'Y': case 'Z':
                  case '_':
                  case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
                  case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
                  case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
                  case 's': case 't': case 'u': case 'v': case 'w': case 'x':
                  case 'y': case 'z':
                  case '0': case '1': case '2': case '3': case '4':
                  case '5': case '6': case '7': case '8': case '9':
                    continue;
                  default:
                    phase3_ungetc (c);
                    break;
                  }
                break;
              }
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

        /* Strings.  */
          {
            int quote_char;
            bool interpret_ansic;
            bool interpret_unicode;
            bool triple;
            unsigned int backslash_counter;

            case 'R': case 'r':
              {
                int c1 = phase2_getc ();
                if (c1 == '"' || c1 == '\'')
                  {
                    quote_char = c1;
                    interpret_ansic = false;
                    interpret_unicode = false;
                    goto string;
                  }
                phase2_ungetc (c1);
                goto symbol;
              }

            case 'U': case 'u':
              {
                int c1 = phase2_getc ();
                if (c1 == '"' || c1 == '\'')
                  {
                    quote_char = c1;
                    interpret_ansic = true;
                    interpret_unicode = true;
                    goto string;
                  }
                if (c1 == 'R' || c1 == 'r')
                  {
                    int c2 = phase2_getc ();
                    if (c2 == '"' || c2 == '\'')
                      {
                        quote_char = c2;
                        interpret_ansic = false;
                        interpret_unicode = true;
                        goto string;
                      }
                    phase2_ungetc (c2);
                  }
                phase2_ungetc (c1);
                goto symbol;
              }

            case '"': case '\'':
              quote_char = c;
              interpret_ansic = true;
              interpret_unicode = false;
            string:
              triple = false;
              lexical_context = lc_string;
              {
                int c1 = phase2_getc ();
                if (c1 == quote_char)
                  {
                    int c2 = phase2_getc ();
                    if (c2 == quote_char)
                      triple = true;
                    else
                      {
                        phase2_ungetc (c2);
                        phase2_ungetc (c1);
                      }
                  }
                else
                  phase2_ungetc (c1);
              }
              backslash_counter = 0;
              {
                struct mixed_string_buffer msb;

                /* Start accumulating the string.  */
                mixed_string_buffer_init (&msb, lexical_context,
                                          logical_file_name, line_number);
                for (;;)
                  {
                    int uc = phase7_getuc (quote_char, triple, interpret_ansic,
                                           interpret_unicode, &backslash_counter);

                    /* Keep line_number in sync.  */
                    msb.line_number = line_number;

                    if (uc == P7_EOF || uc == P7_STRING_END)
                      break;

                    if (IS_UNICODE (uc))
                      {
                        assert (UNICODE_VALUE (uc) >= 0
                                && UNICODE_VALUE (uc) < 0x110000);
                        mixed_string_buffer_append_unicode (&msb,
                                                            UNICODE_VALUE (uc));
                      }
                    else
                      mixed_string_buffer_append_char (&msb, uc);
                  }
                tp->mixed_string = mixed_string_buffer_result (&msb);
                tp->comment = add_reference (savable_comment);
                lexical_context = lc_outside;
                tp->type = token_type_string;
              }
              return;
          }

        case '(':
          open_pbb++;
          tp->type = token_type_lparen;
          return;

        case ')':
          if (open_pbb > 0)
            open_pbb--;
          tp->type = token_type_rparen;
          return;

        case ',':
          tp->type = token_type_comma;
          return;

        case '[': case '{':
          open_pbb++;
          tp->type = (c == '[' ? token_type_lbracket : token_type_other);
          return;

        case ']': case '}':
          if (open_pbb > 0)
            open_pbb--;
          tp->type = (c == ']' ? token_type_rbracket : token_type_other);
          return;

        case '+':
          tp->type = token_type_plus;
          return;

        default:
          /* We could carefully recognize each of the 2 and 3 character
             operators, but it is not necessary, as we only need to recognize
             gettext invocations.  Don't bother.  */
          tp->type = token_type_other;
          return;
        }
    }
}

/* Supports only one pushback token.  */
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


/* Combine adjacent strings to form a single string.  Note that the end
   of a logical line appears as a token of its own, therefore strings that
   belong to different logical lines will not be concatenated.  */

static void
x_python_lex (token_ty *tp)
{
  phase5_get (tp);
  if (tp->type == token_type_string)
    {
      mixed_string_ty *sum = tp->mixed_string;

      for (;;)
        {
          token_ty token2;
          token_ty token3;
          token_ty *tp2 = NULL;

          phase5_get (&token2);
          switch (token2.type)
            {
            case token_type_plus:
              {
                phase5_get (&token3);
                if (token3.type == token_type_string)
                  {
                    free_token (&token2);
                    tp2 = &token3;
                  }
                else
                  phase5_unget (&token3);
              }
              break;
            case token_type_string:
              tp2 = &token2;
              break;
            default:
              break;
            }

          if (tp2)
            {
              sum = mixed_string_concat_free1 (sum, tp2->mixed_string);

              free_token (tp2);
              continue;
            }
          phase5_unget (&token2);
          break;
        }
      tp->mixed_string = sum;
    }
}


/* ========================= Extracting strings.  ========================== */


/* Context lookup table.  */
static flag_context_list_table_ty *flag_context_list_table;


/* Maximum supported nesting depth.  */
#define MAX_NESTING_DEPTH 1000

/* Current nesting depths.  */
static int paren_nesting_depth;
static int bracket_nesting_depth;


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


/* Extract messages until the next balanced closing parenthesis or bracket.
   Extracted messages are added to MLP.
   DELIM can be either token_type_rparen or token_type_rbracket, or
   token_type_eof to accept both.
   Return true upon eof, false upon closing parenthesis or bracket.  */
static bool
extract_balanced (message_list_ty *mlp,
                  token_type_ty delim,
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

      x_python_lex (&token);
      switch (token.type)
        {
        case token_type_symbol:
          {
            void *keyword_value;

            if (hash_find_entry (&keywords, token.string, strlen (token.string),
                                 &keyword_value)
                == 0)
              {
                next_shapes = (const struct callshapes *) keyword_value;
                state = 1;
              }
            else
              state = 0;
          }
          next_context_iter =
            flag_context_list_iterator (
              flag_context_list_table_lookup (
                flag_context_list_table,
                token.string, strlen (token.string)));
          free (token.string);
          continue;

        case token_type_lparen:
          if (++paren_nesting_depth > MAX_NESTING_DEPTH)
            {
              error_with_progname = false;
              error (EXIT_FAILURE, 0, _("%s:%d: error: too many open parentheses"),
                     logical_file_name, line_number);
            }
          if (extract_balanced (mlp, token_type_rparen,
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
          if (delim == token_type_rparen || delim == token_type_eof)
            {
              arglist_parser_done (argparser, arg);
              return false;
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

        case token_type_lbracket:
          if (++bracket_nesting_depth > MAX_NESTING_DEPTH)
            {
              error_with_progname = false;
              error (EXIT_FAILURE, 0, _("%s:%d: error: too many open brackets"),
                     logical_file_name, line_number);
            }
          if (extract_balanced (mlp, token_type_rbracket,
                                null_context, null_context_list_iterator,
                                arglist_parser_alloc (mlp, NULL)))
            {
              arglist_parser_done (argparser, arg);
              return true;
            }
          bracket_nesting_depth--;
          next_context_iter = null_context_list_iterator;
          state = 0;
          continue;

        case token_type_rbracket:
          if (delim == token_type_rbracket || delim == token_type_eof)
            {
              arglist_parser_done (argparser, arg);
              return false;
            }
          next_context_iter = null_context_list_iterator;
          state = 0;
          continue;

        case token_type_string:
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
extract_python (FILE *f,
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

  lexical_context = lc_outside;

  phase2_pushback_length = 0;

  last_comment_line = -1;
  last_non_comment_line = -1;

  /* For Python, the default source file encoding is UTF-8.  This is specified
     in PEP 3120.  */
  xgettext_current_file_source_encoding =
   (xgettext_global_source_encoding != NULL ? xgettext_global_source_encoding :
    po_charset_utf8);
#if HAVE_ICONV
  xgettext_current_file_source_iconv = xgettext_global_source_iconv;
#endif

  xgettext_current_source_encoding = xgettext_current_file_source_encoding;
#if HAVE_ICONV
  xgettext_current_source_iconv = xgettext_current_file_source_iconv;
#endif

  continuation_or_nonblank_line = false;

  open_pbb = 0;

  phase5_pushback_length = 0;

  flag_context_list_table = flag_table;
  paren_nesting_depth = 0;
  bracket_nesting_depth = 0;

  init_keywords ();

  /* Eat tokens until eof is seen.  When extract_balanced returns
     due to an unbalanced closing parenthesis, just restart it.  */
  while (!extract_balanced (mlp, token_type_eof,
                            null_context, null_context_list_iterator,
                            arglist_parser_alloc (mlp, NULL)))
    ;

  fp = NULL;
  real_file_name = NULL;
  logical_file_name = NULL;
  line_number = 0;
}
