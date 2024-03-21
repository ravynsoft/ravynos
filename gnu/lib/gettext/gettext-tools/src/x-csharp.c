/* xgettext C# backend.
   Copyright (C) 2003-2009, 2011, 2014, 2018-2023 Free Software Foundation, Inc.
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
#include "x-csharp.h"

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
#include "c-ctype.h"
#include "error.h"
#include "error-progname.h"
#include "xalloc.h"
#include "xerror.h"
#include "xvasprintf.h"
#include "mem-hash-map.h"
#include "po-charset.h"
#include "unistr.h"
#include "gettext.h"

#define _(s) gettext(s)

#define SIZEOF(a) (sizeof(a) / sizeof(a[0]))


/* The C# syntax is defined in ECMA-334, second edition.  */


/* ====================== Keyword set customization.  ====================== */

/* If true extract all strings.  */
static bool extract_all = false;

static hash_table keywords;
static bool default_keywords = true;


void
x_csharp_extract_all ()
{
  extract_all = true;
}


/* Processes a --keyword option.
   Non-ASCII function names can be used if given in UTF-8 encoding.  */
void
x_csharp_keyword (const char *name)
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

      /* The characters between name and end should form a valid C#
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
      x_csharp_keyword ("GetString");   /* Resource{Manager,Set}.GetString */
      x_csharp_keyword ("GetPluralString:1,2"); /* GettextResource{Manager,Set}.GetPluralString */
      x_csharp_keyword ("GetParticularString:1c,2"); /* Resource{Manager,Set}.GetParticularString */
      x_csharp_keyword ("GetParticularPluralString:1c,2,3"); /* Resource{Manager,Set}.GetParticularPluralString */
      default_keywords = false;
    }
}

void
init_flag_table_csharp ()
{
  xgettext_record_flag ("GetString:1:pass-csharp-format");
  xgettext_record_flag ("GetPluralString:1:pass-csharp-format");
  xgettext_record_flag ("GetPluralString:2:pass-csharp-format");
  xgettext_record_flag ("GetParticularString:2:pass-csharp-format");
  xgettext_record_flag ("GetParticularPluralString:2:pass-csharp-format");
  xgettext_record_flag ("GetParticularPluralString:3:pass-csharp-format");
  xgettext_record_flag ("String.Format:1:csharp-format");
}


/* ======================== Reading of characters.  ======================== */

/* The input file stream.  */
static FILE *fp;


/* Phase 1: line_number handling.  */

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
    {
      c = phase1_pushback[--phase1_pushback_length];
      if (c == '\n')
        ++line_number;
      return c;
    }

  c = getc (fp);
  if (c == EOF)
    {
      if (ferror (fp))
        error (EXIT_FAILURE, errno, _("error while reading \"%s\""),
               real_file_name);
      return EOF;
    }

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
   This is done early because ECMA-334 section 9.1. says that the source is
   "an ordered sequence of Unicode characters", and because the recognition
   of the line terminators (ECMA-334 section 9.3.1) is hardly possible without
   prior conversion to Unicode.  */

/* End-of-file indicator for functions returning an UCS-4 character.  */
#define UEOF -1

/* Newline Unicode character.  */
#define UNL 0x000a

static lexical_context_ty lexical_context;

static int phase2_pushback[1];
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
                                      _("Please specify the source encoding through --from-code.")));
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
                  multiline_error (xstrdup (""),
                                   xasprintf (_("\
%s:%d: Invalid multibyte sequence.\n\
Please specify the correct source encoding through --from-code.\n"),
                                   real_file_name, line_number));
                  exit (EXIT_FAILURE);
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
Please specify the correct source encoding through --from-code.\n"),
                                       real_file_name, line_number));
                      exit (EXIT_FAILURE);
                    }

                  /* Read one more byte and retry iconv.  */
                  c = phase1_getc ();
                  if (c == EOF)
                    {
                      multiline_error (xstrdup (""),
                                       xasprintf (_("\
%s:%d: Incomplete multibyte sequence at end of file.\n\
Please specify the correct source encoding through --from-code.\n"),
                                       real_file_name, line_number));
                      exit (EXIT_FAILURE);
                    }
                  if (c == '\n')
                    {
                      multiline_error (xstrdup (""),
                                       xasprintf (_("\
%s:%d: Incomplete multibyte sequence at end of line.\n\
Please specify the correct source encoding through --from-code.\n"),
                                       real_file_name, line_number - 1));
                      exit (EXIT_FAILURE);
                    }
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
                  multiline_error (xstrdup (""),
                                   xasprintf (_("\
%s:%d: Invalid multibyte sequence.\n\
Please specify the source encoding through --from-code.\n"),
                                   real_file_name, line_number));
                  exit (EXIT_FAILURE);
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
      /* Read an UTF-8 encoded character.  */
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
        }

      if (buf[0] >= 0xe0
          && ((buf[1] ^ 0x80) < 0x40))
        {
          c = phase1_getc ();
          if (c == EOF)
            return UEOF;
          buf[2] = c;
          count = 3;
        }

      if (buf[0] >= 0xf0
          && ((buf[1] ^ 0x80) < 0x40)
          && ((buf[2] ^ 0x80) < 0x40))
        {
          c = phase1_getc ();
          if (c == EOF)
            return UEOF;
          buf[3] = c;
          count = 4;
        }

      if (buf[0] >= 0xf8
          && ((buf[1] ^ 0x80) < 0x40)
          && ((buf[2] ^ 0x80) < 0x40)
          && ((buf[3] ^ 0x80) < 0x40))
        {
          c = phase1_getc ();
          if (c == EOF)
            return UEOF;
          buf[4] = c;
          count = 5;
        }

      if (buf[0] >= 0xfc
          && ((buf[1] ^ 0x80) < 0x40)
          && ((buf[2] ^ 0x80) < 0x40)
          && ((buf[3] ^ 0x80) < 0x40)
          && ((buf[4] ^ 0x80) < 0x40))
        {
          c = phase1_getc ();
          if (c == EOF)
            return UEOF;
          buf[5] = c;
          count = 6;
        }

      u8_mbtouc (&uc, buf, count);
      return uc;
    }
}

/* Supports only one pushback character.  */
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


/* Phase 3: Convert all line terminators to LF.
   See ECMA-334 section 9.3.1.  */

/* Line number defined in terms of phase3.  */
static int logical_line_number;

static int phase3_pushback[10];
static int phase3_pushback_length;

/* Read the next Unicode UCS-4 character from the input file, mapping
   all line terminators to U+000A, and dropping U+001A at the end of file.  */
static int
phase3_getc ()
{
  int c;

  if (phase3_pushback_length)
    {
      c = phase3_pushback[--phase3_pushback_length];
      if (c == UNL)
        ++logical_line_number;
      return c;
    }

  c = phase2_getc ();

  if (c == 0x000d)
    {
      int c1 = phase2_getc ();

      if (c1 != UEOF && c1 != 0x000a)
        phase2_ungetc (c1);

      /* Seen line terminator CR or CR/LF.  */
      ++logical_line_number;
      return UNL;
    }

  if (c == 0x0085 || c == 0x2028 || c == 0x2029)
    {
      /* Seen Unicode word processor newline.  */
      ++logical_line_number;
      return UNL;
    }

  if (c == 0x001a)
    {
      int c1 = phase2_getc ();

      if (c1 == UEOF)
        /* Seen U+001A right before the end of file.  */
        return UEOF;

      phase2_ungetc (c1);
    }

  if (c == UNL)
    ++logical_line_number;
  return c;
}

/* Supports 9 characters of pushback.  */
static void
phase3_ungetc (int c)
{
  if (c != UEOF)
    {
      if (c == UNL)
        --logical_line_number;
      if (phase3_pushback_length == SIZEOF (phase3_pushback))
        abort ();
      phase3_pushback[phase3_pushback_length++] = c;
    }
}


/* ========================= Accumulating strings.  ======================== */

/* See xg-mixed-string.h for the API.
   In this extractor, we add only Unicode characters.  */


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
  lexical_context = lc_outside;
}


/* These are for tracking whether comments count as immediately before
   keyword.  */
static int last_comment_line;
static int last_non_comment_line;


/* Phase 4: Replace each comment that is not inside a character constant or
   string literal with a space or newline character.
   See ECMA-334 section 9.3.2.  */

static int
phase4_getc ()
{
  int c0;
  int c;
  bool last_was_star;

  c0 = phase3_getc ();
  if (c0 != '/')
    return c0;
  c = phase3_getc ();
  switch (c)
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
          if (c == UEOF)
            break;
          /* We skip all leading white space, but not EOLs.  */
          if (!(comment_at_start () && (c == ' ' || c == '\t')))
            comment_add (c);
          switch (c)
            {
            case UNL:
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
      last_comment_line = logical_line_number;
      return ' ';

    case '/':
      /* C++ style comment.  */
      last_comment_line = logical_line_number;
      comment_start ();
      for (;;)
        {
          c = phase3_getc ();
          if (c == UNL || c == UEOF)
            break;
          /* We skip all leading white space, but not EOLs.  */
          if (!(comment_at_start () && (c == ' ' || c == '\t')))
            comment_add (c);
        }
      phase3_ungetc (c); /* push back the newline, to decrement logical_line_number */
      comment_line_end (0);
      phase3_getc (); /* read the newline again */
      return UNL;
    }
}

/* Supports only one pushback character.  */
static void
phase4_ungetc (int c)
{
  phase3_ungetc (c);
}


/* ======================= Character classification.  ====================== */


/* Return true if a given character is white space.
   See ECMA-334 section 9.3.3.  */
static bool
is_whitespace (int c)
{
  /* Unicode character class Zs, as of Unicode 4.0.  */
  /* grep '^[^;]*;[^;]*;Zs;' UnicodeData-4.0.0.txt */
  switch (c >> 8)
    {
    case 0x00:
      return (c == 0x0020 || c == 0x00a0);
    case 0x16:
      return (c == 0x1680);
    case 0x18:
      return (c == 0x180e);
    case 0x20:
      return ((c >= 0x2000 && c <= 0x200b) || c == 0x202f || c == 0x205f);
    case 0x30:
      return (c == 0x3000);
    default:
      return false;
    }
}


/* C# allows identifiers containing many Unicode characters.  We recognize
   them; to use an identifier with Unicode characters in a --keyword option,
   it must be specified in UTF-8.  */

static inline int
bitmap_lookup (const void *table, unsigned int uc)
{
  unsigned int index1 = uc >> 16;
  if (index1 < ((const int *) table)[0])
    {
      int lookup1 = ((const int *) table)[1 + index1];
      if (lookup1 >= 0)
        {
          unsigned int index2 = (uc >> 9) & 0x7f;
          int lookup2 = ((const int *) table)[lookup1 + index2];
          if (lookup2 >= 0)
            {
              unsigned int index3 = (uc >> 5) & 0xf;
              unsigned int lookup3 = ((const int *) table)[lookup2 + index3];

              return (lookup3 >> (uc & 0x1f)) & 1;
            }
        }
    }
  return 0;
}

/* Unicode character classes Lu, Ll, Lt, Lm, Lo, Nl, as of Unicode 4.0,
   plus the underscore.  */
static const
struct
  {
    int header[1];
    int level1[3];
    int level2[3 << 7];
    /*unsigned*/ int level3[34 << 4];
  }
table_identifier_start =
{
  { 3 },
  {     4,   132,   260 },
  {
      388,   404,   420,   436,   452,   468,   484,   500,
      516,   532,   548,   564,   580,    -1,   596,   612,
      628,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      644,    -1,   660,   660,   660,   660,   660,   660,
      660,   660,   660,   660,   660,   660,   676,   660,
      660,   660,   660,   660,   660,   660,   660,   660,
      660,   660,   660,   660,   660,   660,   660,   660,
      660,   660,   660,   660,   660,   660,   660,   660,
      660,   660,   660,   660,   660,   660,   660,   660,
      660,   660,   660,   660,   660,   660,   660,   692,
      660,   660,   708,    -1,    -1,    -1,   660,   660,
      660,   660,   660,   660,   660,   660,   660,   660,
      660,   660,   660,   660,   660,   660,   660,   660,
      660,   660,   660,   724,    -1,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,   740,   756,   772,   788,
      804,   820,   836,    -1,   852,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       -1,    -1,   868,   884,    -1,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      660,   660,   660,   660,   660,   660,   660,   660,
      660,   660,   660,   660,   660,   660,   660,   660,
      660,   660,   660,   660,   660,   660,   660,   660,
      660,   660,   660,   660,   660,   660,   660,   660,
      660,   660,   660,   660,   660,   660,   660,   660,
      660,   660,   660,   660,   660,   660,   660,   660,
      660,   660,   660,   660,   660,   660,   660,   660,
      660,   660,   660,   660,   660,   660,   660,   660,
      660,   660,   660,   660,   660,   660,   660,   660,
      660,   660,   660,   660,   660,   660,   660,   660,
      660,   660,   660,   900,    -1,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,   660,   916,    -1,    -1
  },
  {
    0x00000000, 0x00000000, 0x87FFFFFE, 0x07FFFFFE,
    0x00000000, 0x04200400, 0xFF7FFFFF, 0xFF7FFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0x007FFFFF, 0xFFFF0000, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0x0003FFC3, 0x0000401F,
    0x00000000, 0x00000000, 0x00000000, 0x04000000,
    0xFFFFD740, 0xFFFFFFFB, 0xFFFF7FFF, 0x0FBFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFC03, 0xFFFFFFFF, 0xFFFF7FFF, 0x033FFFFF,
    0x0000FFFF, 0xFFFE0000, 0x027FFFFF, 0xFFFFFFFE,
    0x000000FF, 0x00000000, 0xFFFF0000, 0x000707FF,
    0x00000000, 0x07FFFFFE, 0x000007FF, 0xFFFEC000,
    0xFFFFFFFF, 0xFFFFFFFF, 0x002FFFFF, 0x9C00C060,
    0xFFFD0000, 0x0000FFFF, 0x0000E000, 0x00000000,
    0xFFFFFFFF, 0x0002003F, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xFFFFFFF0, 0x23FFFFFF, 0xFF010000, 0x00000003,
    0xFFF99FE0, 0x23C5FDFF, 0xB0000000, 0x00030003,
    0xFFF987E0, 0x036DFDFF, 0x5E000000, 0x001C0000,
    0xFFFBBFE0, 0x23EDFDFF, 0x00010000, 0x00000003,
    0xFFF99FE0, 0x23EDFDFF, 0xB0000000, 0x00020003,
    0xD63DC7E8, 0x03BFC718, 0x00000000, 0x00000000,
    0xFFFDDFE0, 0x03EFFDFF, 0x00000000, 0x00000003,
    0xFFFDDFE0, 0x23EFFDFF, 0x40000000, 0x00000003,
    0xFFFDDFE0, 0x03FFFDFF, 0x00000000, 0x00000003,
    0xFC7FFFE0, 0x2FFBFFFF, 0x0000007F, 0x00000000,
    0xFFFFFFFE, 0x000DFFFF, 0x0000007F, 0x00000000,
    0xFEF02596, 0x200DECAE, 0x3000005F, 0x00000000,
    0x00000001, 0x00000000, 0xFFFFFEFF, 0x000007FF,
    0x00000F00, 0x00000000, 0x00000000, 0x00000000,
    0xFFFFFFFF, 0x000006FB, 0x003F0000, 0x00000000,
    0x00000000, 0xFFFFFFFF, 0xFFFF003F, 0x01FFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0x83FFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFF07, 0xFFFFFFFF, 0x03FFFFFF,
    0xFFFFFF7F, 0xFFFFFFFF, 0x3D7F3D7F, 0xFFFFFFFF,
    0xFFFF3D7F, 0x7F3D7FFF, 0xFF7F7F3D, 0xFFFF7FFF,
    0x7F3D7FFF, 0xFFFFFFFF, 0x07FFFF7F, 0x00000000,
    0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0x001FFFFF,
    0xFFFFFFFE, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x007F9FFF,
    0x07FFFFFE, 0xFFFFFFFF, 0xFFFFFFFF, 0x0001C7FF,
    0x0003DFFF, 0x0003FFFF, 0x0003FFFF, 0x0001DFFF,
    0xFFFFFFFF, 0x000FFFFF, 0x10800000, 0x00000000,
    0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0x00FFFFFF,
    0xFFFFFFFF, 0x000001FF, 0x00000000, 0x00000000,
    0x1FFFFFFF, 0x00000000, 0xFFFF0000, 0x001F3FFF,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000FFF,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0x0FFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x03FFFFFF,
    0x3F3FFFFF, 0xFFFFFFFF, 0xAAFF3F3F, 0x3FFFFFFF,
    0xFFFFFFFF, 0x5FDFFFFF, 0x0FCF1FDC, 0x1FDC1FFF,
    0x00000000, 0x00000000, 0x00000000, 0x80020000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x3E2FFC84, 0xE3FBBD50, 0x000003E0, 0xFFFFFFFF,
    0x0000000F, 0x00000000, 0x00000000, 0x00000000,
    0x000000E0, 0x1F3E03FE, 0xFFFFFFFE, 0xFFFFFFFF,
    0xE07FFFFF, 0xFFFFFFFE, 0xFFFFFFFF, 0xF7FFFFFF,
    0xFFFFFFE0, 0xFFFE1FFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0x00007FFF, 0x00FFFFFF, 0x00000000, 0xFFFF0000,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0x003FFFFF, 0x00000000, 0x00000000,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0x0000003F, 0x00000000, 0x00000000,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0x00001FFF, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0x0000000F, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFF3FFF, 0xFFFFFFFF, 0x000007FF,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xA0F8007F, 0x5F7FFDFF, 0xFFFFFFDB, 0xFFFFFFFF,
    0xFFFFFFFF, 0x0003FFFF, 0xFFF80000, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0x3FFFFFFF, 0xFFFF0000, 0xFFFFFFFF,
    0xFFFCFFFF, 0xFFFFFFFF, 0x000000FF, 0x0FFF0000,
    0x00000000, 0x00000000, 0x00000000, 0xFFDF0000,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x1FFFFFFF,
    0x00000000, 0x07FFFFFE, 0x07FFFFFE, 0xFFFFFFC0,
    0xFFFFFFFF, 0x7FFFFFFF, 0x1CFCFCFC, 0x00000000,
    0xFFFFEFFF, 0xB7FFFF7F, 0x3FFF3FFF, 0x00000000,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x07FFFFFF,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x7FFFFFFF, 0xFFFF0000, 0x000007FF, 0x00000000,
    0x3FFFFFFF, 0x00000000, 0x00000000, 0x00000000,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0x3FFFFFFF, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xFFFFFD3F, 0x91BFFFFF, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFDFFFFF, 0xFFFFFFFF,
    0xDFFFFFFF, 0xEBFFDE64, 0xFFFFFFEF, 0xFFFFFFFF,
    0xDFDFE7BF, 0x7BFFFFFF, 0xFFFDFC5F, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFF0F, 0xF7FFFFFD, 0xF7FFFFFF,
    0xFFDFFFFF, 0xFFDFFFFF, 0xFFFF7FFF, 0xFFFF7FFF,
    0xFFFFFDFF, 0xFFFFFDFF, 0x000003F7, 0x00000000,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0x007FFFFF, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x3FFFFFFF, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000
  }
};

/* Unicode character classes Lu, Ll, Lt, Lm, Lo, Nl, Nd, Pc, Mn, Mc, Cf,
   as of Unicode 4.0.  */
static const
struct
  {
    int header[1];
    int level1[15];
    int level2[4 << 7];
    /*unsigned*/ int level3[36 << 4];
  }
table_identifier_part =
{
  { 15 },
  {
       16,   144,   272,    -1,    -1,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,    -1,    -1,   400
  },
  {
      528,   544,   560,   576,   592,   608,   624,   640,
      656,   672,   688,   704,   720,    -1,   736,   752,
      768,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      784,    -1,   800,   800,   800,   800,   800,   800,
      800,   800,   800,   800,   800,   800,   816,   800,
      800,   800,   800,   800,   800,   800,   800,   800,
      800,   800,   800,   800,   800,   800,   800,   800,
      800,   800,   800,   800,   800,   800,   800,   800,
      800,   800,   800,   800,   800,   800,   800,   800,
      800,   800,   800,   800,   800,   800,   800,   832,
      800,   800,   848,    -1,    -1,    -1,   800,   800,
      800,   800,   800,   800,   800,   800,   800,   800,
      800,   800,   800,   800,   800,   800,   800,   800,
      800,   800,   800,   864,    -1,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,   880,   896,   912,   928,
      944,   960,   976,    -1,   992,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     1008,    -1,  1024,  1040,    -1,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      800,   800,   800,   800,   800,   800,   800,   800,
      800,   800,   800,   800,   800,   800,   800,   800,
      800,   800,   800,   800,   800,   800,   800,   800,
      800,   800,   800,   800,   800,   800,   800,   800,
      800,   800,   800,   800,   800,   800,   800,   800,
      800,   800,   800,   800,   800,   800,   800,   800,
      800,   800,   800,   800,   800,   800,   800,   800,
      800,   800,   800,   800,   800,   800,   800,   800,
      800,   800,   800,   800,   800,   800,   800,   800,
      800,   800,   800,   800,   800,   800,   800,   800,
      800,   800,   800,  1056,    -1,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,   800,  1072,    -1,    -1,
     1088,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1
  },
  {
    0x00000000, 0x03FF0000, 0x87FFFFFE, 0x07FFFFFE,
    0x00000000, 0x04202400, 0xFF7FFFFF, 0xFF7FFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0x007FFFFF, 0xFFFF0000, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0x0003FFC3, 0x0000401F,
    0xFFFFFFFF, 0xFFFFFFFF, 0xE0FFFFFF, 0x0400FFFF,
    0xFFFFD740, 0xFFFFFFFB, 0xFFFF7FFF, 0x0FBFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFC7B, 0xFFFFFFFF, 0xFFFF7FFF, 0x033FFFFF,
    0x0000FFFF, 0xFFFE0000, 0x027FFFFF, 0xFFFFFFFE,
    0xFFFE00FF, 0xBBFFFFFB, 0xFFFF0016, 0x000707FF,
    0x003F000F, 0x07FFFFFE, 0x01FFFFFF, 0xFFFFC3FF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xBFEFFFFF, 0x9FFFFDFF,
    0xFFFF8000, 0xFFFFFFFF, 0x0000E7FF, 0x00000000,
    0xFFFFFFFF, 0x0003FFFF, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xFFFFFFFE, 0xF3FFFFFF, 0xFF1F3FFF, 0x0000FFCF,
    0xFFF99FEE, 0xF3C5FDFF, 0xB080399F, 0x0003FFCF,
    0xFFF987EE, 0xD36DFDFF, 0x5E003987, 0x001FFFC0,
    0xFFFBBFEE, 0xF3EDFDFF, 0x00013BBF, 0x0000FFCF,
    0xFFF99FEE, 0xF3EDFDFF, 0xB0C0398F, 0x0002FFC3,
    0xD63DC7EC, 0xC3BFC718, 0x00803DC7, 0x0000FF80,
    0xFFFDDFEE, 0xC3EFFDFF, 0x00603DDF, 0x0000FFC3,
    0xFFFDDFEC, 0xF3EFFDFF, 0x40603DDF, 0x0000FFC3,
    0xFFFDDFEC, 0xC3FFFDFF, 0x00803DCF, 0x0000FFC3,
    0xFC7FFFEC, 0x2FFBFFFF, 0xFF5F847F, 0x000C0000,
    0xFFFFFFFE, 0x07FFFFFF, 0x03FF7FFF, 0x00000000,
    0xFEF02596, 0x3BFFECAE, 0x33FF3F5F, 0x00000000,
    0x03000001, 0xC2A003FF, 0xFFFFFEFF, 0xFFFE07FF,
    0xFEFF0FDF, 0x1FFFFFFF, 0x00000040, 0x00000000,
    0xFFFFFFFF, 0x03C7F6FB, 0x03FF03FF, 0x00000000,
    0x00000000, 0xFFFFFFFF, 0xFFFF003F, 0x01FFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0x83FFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFF07, 0xFFFFFFFF, 0x03FFFFFF,
    0xFFFFFF7F, 0xFFFFFFFF, 0x3D7F3D7F, 0xFFFFFFFF,
    0xFFFF3D7F, 0x7F3D7FFF, 0xFF7F7F3D, 0xFFFF7FFF,
    0x7F3D7FFF, 0xFFFFFFFF, 0x07FFFF7F, 0x0003FE00,
    0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0x001FFFFF,
    0xFFFFFFFE, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x007F9FFF,
    0x07FFFFFE, 0xFFFFFFFF, 0xFFFFFFFF, 0x0001C7FF,
    0x001FDFFF, 0x001FFFFF, 0x000FFFFF, 0x000DDFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0x308FFFFF, 0x000003FF,
    0x03FF3800, 0xFFFFFFFF, 0xFFFFFFFF, 0x00FFFFFF,
    0xFFFFFFFF, 0x000003FF, 0x00000000, 0x00000000,
    0x1FFFFFFF, 0x0FFF0FFF, 0xFFFFFFC0, 0x001F3FFF,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000FFF,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0x0FFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x03FFFFFF,
    0x3F3FFFFF, 0xFFFFFFFF, 0xAAFF3F3F, 0x3FFFFFFF,
    0xFFFFFFFF, 0x5FDFFFFF, 0x0FCF1FDC, 0x1FDC1FFF,
    0x0000F000, 0x80007C00, 0x00100001, 0x8002FC0F,
    0x00000000, 0x00000000, 0x1FFF0000, 0x000007E2,
    0x3E2FFC84, 0xE3FBBD50, 0x000003E0, 0xFFFFFFFF,
    0x0000000F, 0x00000000, 0x00000000, 0x00000000,
    0x000000E0, 0x1F3EFFFE, 0xFFFFFFFE, 0xFFFFFFFF,
    0xE67FFFFF, 0xFFFFFFFE, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFE0, 0xFFFE1FFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0x00007FFF, 0x00FFFFFF, 0x00000000, 0xFFFF0000,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0x003FFFFF, 0x00000000, 0x00000000,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0x0000003F, 0x00000000, 0x00000000,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0x00001FFF, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0x0000000F, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFF3FFF, 0xFFFFFFFF, 0x000007FF,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xE0F8007F, 0x5F7FFDFF, 0xFFFFFFDB, 0xFFFFFFFF,
    0xFFFFFFFF, 0x0003FFFF, 0xFFF80000, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0x3FFFFFFF, 0xFFFF0000, 0xFFFFFFFF,
    0xFFFCFFFF, 0xFFFFFFFF, 0x000000FF, 0x0FFF0000,
    0x0000FFFF, 0x0018000F, 0x0000E000, 0xFFDF0000,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x9FFFFFFF,
    0x03FF0000, 0x87FFFFFE, 0x07FFFFFE, 0xFFFFFFE0,
    0xFFFFFFFF, 0x7FFFFFFF, 0x1CFCFCFC, 0x0E000000,
    0xFFFFEFFF, 0xB7FFFF7F, 0x3FFF3FFF, 0x00000000,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x07FFFFFF,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x7FFFFFFF, 0xFFFF0000, 0x000007FF, 0x00000000,
    0x3FFFFFFF, 0x00000000, 0x00000000, 0x00000000,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0x3FFFFFFF, 0x000003FF, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xFFFFFD3F, 0x91BFFFFF, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0xFFFFE3E0,
    0x00000FE7, 0x00003C00, 0x00000000, 0x00000000,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFDFFFFF, 0xFFFFFFFF,
    0xDFFFFFFF, 0xEBFFDE64, 0xFFFFFFEF, 0xFFFFFFFF,
    0xDFDFE7BF, 0x7BFFFFFF, 0xFFFDFC5F, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFF0F, 0xF7FFFFFD, 0xF7FFFFFF,
    0xFFDFFFFF, 0xFFDFFFFF, 0xFFFF7FFF, 0xFFFF7FFF,
    0xFFFFFDFF, 0xFFFFFDFF, 0xFFFFC3F7, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0x007FFFFF, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x3FFFFFFF, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000002, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x0000FFFF
  }
};

/* Return true if a given character can occur as first character of an
   identifier.  See ECMA-334 section 9.4.2.  */
static bool
is_identifier_start (int c)
{
  return bitmap_lookup (&table_identifier_start, c);
  /* In ASCII only this would be:
     return ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_');
   */
}

/* Return true if a given character can occur as character of an identifier.
   See ECMA-334 section 9.4.2.  */
static bool
is_identifier_part (int c)
{
  return bitmap_lookup (&table_identifier_part, c);
  /* In ASCII only this would be:
     return ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')
             || (c >= '0' && c <= '9') || c == '_');
   */
}

static bool
is_any_character (int c)
{
  return true;
}


/* ======================= Preprocessor directives.  ======================= */


/* Phase 5: Remove preprocessor lines.  See ECMA-334 section 9.5.
   As a side effect, this also removes initial whitespace on every line;
   this whitespace doesn't matter.  */

static int phase5_pushback[10];
static int phase5_pushback_length;

static int
phase5_getc ()
{
  int c;

  if (phase5_pushback_length)
    return phase5_pushback[--phase5_pushback_length];

  c = phase4_getc ();
  if (c != UNL)
    return c;

  do
    c = phase3_getc ();
  while (c != UEOF && is_whitespace (c));

  if (c == '#')
    {
      /* Ignore the entire line containing the preprocessor directive
         (including the // comment if it contains one).  */
      do
        c = phase3_getc ();
      while (c != UEOF && c != UNL);
      return c;
    }
  else
    {
      phase3_ungetc (c);
      return UNL;
    }
}

#ifdef unused
static void
phase5_ungetc (int c)
{
  if (c != UEOF)
    {
      if (phase5_pushback_length == SIZEOF (phase5_pushback))
        abort ();
      phase5_pushback[phase5_pushback_length++] = c;
    }
}
#endif


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
  token_type_string_literal,    /* "abc", @"abc" */
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
  int logical_line_number;
};


/* Free the memory pointed to by a 'struct token_ty'.  */
static inline void
free_token (token_ty *tp)
{
  if (tp->type == token_type_symbol)
    free (tp->string);
  if (tp->type == token_type_string_literal)
    {
      mixed_string_free (tp->mixed_string);
      drop_reference (tp->comment);
    }
}


/* Read a Unicode escape sequence outside string/character literals.
   Reject Unicode escapes that don't fulfill the given predicate.
   See ECMA-334 section 9.4.2.  */
static int
do_getc_unicode_escaped (bool (*predicate) (int))
{
  int c;

  /* Use phase 3, because phase 4 elides comments.  */
  c = phase3_getc ();
  if (c == UEOF)
    return '\\';
  if (c == 'u' || c == 'U')
    {
      unsigned char buf[8];
      int expect;
      unsigned int n;
      int i;

      expect = (c == 'U' ? 8 : 4);
      n = 0;
      for (i = 0; i < expect; i++)
        {
          int c1 = phase3_getc ();

          if (c1 >= '0' && c1 <= '9')
            n = (n << 4) + (c1 - '0');
          else if (c1 >= 'A' && c1 <= 'F')
            n = (n << 4) + (c1 - 'A' + 10);
          else if (c1 >= 'a' && c1 <= 'f')
            n = (n << 4) + (c1 - 'a' + 10);
          else
            {
              phase3_ungetc (c1);
              while (--i >= 0)
                phase3_ungetc (buf[i]);
              phase3_ungetc (c);
              return '\\';
            }

          buf[i] = c1;
        }

      if (n >= 0x110000)
        {
          error_with_progname = false;
          error (0, 0, _("%s:%d: warning: invalid Unicode character"),
                 logical_file_name, line_number);
          error_with_progname = true;
        }
      else if (predicate (n))
        return n;

      while (--i >= 0)
        phase3_ungetc (buf[i]);
    }
  phase3_ungetc (c);
  return '\\';
}


/* Read an escape sequence inside a string literal or character literal.
   See ECMA-334 sections 9.4.4.4., 9.4.4.5.  */
static int
do_getc_escaped ()
{
  int c;
  int n;
  int i;

  /* Use phase 3, because phase 4 elides comments.  */
  c = phase3_getc ();
  if (c == UEOF)
    return '\\';
  switch (c)
    {
    case 'a':
      return 0x0007;
    case 'b':
      return 0x0008;
    case 't':
      return 0x0009;
    case 'n':
      return 0x000a;
    case 'v':
      return 0x000b;
    case 'f':
      return 0x000c;
    case 'r':
      return 0x000d;
    case '"':
      return '"';
    case '\'':
      return '\'';
    case '\\':
      return '\\';
    case '0':
      return 0x0000;
    case 'x':
      c = phase3_getc ();
      switch (c)
        {
        default:
          phase3_ungetc (c);
          phase3_ungetc ('x');
          return '\\';

        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
          break;
        }
      n = 0;
      for (i = 0;; i++)
        {
          switch (c)
            {
            default:
              phase3_ungetc (c);
              return n;
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
              n = n * 16 + c - '0';
              break;
            case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
              n = n * 16 + 10 + c - 'A';
              break;
            case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
              n = n * 16 + 10 + c - 'a';
              break;
            }
          if (i == 3)
            break;
          c = phase3_getc ();
        }
      return n;
    case 'u': case 'U':
      phase3_ungetc (c);
      return do_getc_unicode_escaped (is_any_character);
    default:
      /* Invalid escape sequence.  */
      phase3_ungetc (c);
      return '\\';
    }
}

/* Read a regular string literal or character literal.
   See ECMA-334 sections 9.4.4.4., 9.4.4.5.  */
static void
accumulate_escaped (struct mixed_string_buffer *literal, int delimiter)
{
  int c;

  for (;;)
    {
      /* Use phase 3, because phase 4 elides comments.  */
      c = phase3_getc ();
      if (c == UEOF || c == delimiter)
        break;
      if (c == UNL)
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
      if (c == '\\')
        c = do_getc_escaped ();
      if (literal)
        mixed_string_buffer_append_unicode (literal, c);
    }
}


/* Combine characters into tokens.  Discard whitespace.  */

/* Maximum used guaranteed to be < 4.  */
static token_ty phase6_pushback[4];
static int phase6_pushback_length;

static void
phase6_get (token_ty *tp)
{
  int c;

  if (phase6_pushback_length)
    {
      *tp = phase6_pushback[--phase6_pushback_length];
      return;
    }
  tp->string = NULL;

  for (;;)
    {
      tp->line_number = line_number;
      tp->logical_line_number = logical_line_number;
      c = phase5_getc ();

      if (c == UEOF)
        {
          tp->type = token_type_eof;
          return;
        }

      switch (c)
        {
        case UNL:
          if (last_non_comment_line > last_comment_line)
            savable_comment_reset ();
          FALLTHROUGH;
        case ' ':
        case '\t':
        case '\f':
          /* Ignore whitespace and comments.  */
          continue;
        }

      last_non_comment_line = tp->logical_line_number;

      switch (c)
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
          if (!(c >= '0' && c <= '9'))
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
               floating-point numbers.  We assume a valid C# input.
               The simplified syntax that we recognize as number is: any
               sequence of alphanumeric characters, additionally '+' and '-'
               immediately after 'e' or 'E' except in hexadecimal numbers.  */
            bool hexadecimal = false;

            for (;;)
              {
                c = phase4_getc ();
                if (c >= '0' && c <= '9')
                  continue;
                if ((c >= 'A' && c <= 'Z') || (c >= 'a' &&c <= 'z'))
                  {
                    if (c == 'X' || c == 'x')
                      hexadecimal = true;
                    if ((c == 'E' || c == 'e') && !hexadecimal)
                      {
                        c = phase4_getc ();
                        if (!(c == '+' || c == '-'))
                          phase4_ungetc (c);
                      }
                    continue;
                  }
                if (c == '.')
                  continue;
                break;
              }
            phase4_ungetc (c);
            tp->type = token_type_number;
            return;
          }

        case '"':
          /* Regular string literal.  */
          {
            struct mixed_string_buffer literal;

            lexical_context = lc_string;
            mixed_string_buffer_init (&literal,
                                      lexical_context,
                                      logical_file_name,
                                      logical_line_number);
            accumulate_escaped (&literal, '"');
            tp->mixed_string = mixed_string_buffer_result (&literal);
            tp->comment = add_reference (savable_comment);
            lexical_context = lc_outside;
            tp->type = token_type_string_literal;
            return;
          }

        case '\'':
          /* Character literal.  */
          {
            accumulate_escaped (NULL, '\'');
            tp->type = token_type_other;
            return;
          }

        case '+':
          c = phase4_getc ();
          if (c == '+')
            /* Operator ++ */
            tp->type = token_type_other;
          else if (c == '=')
            /* Operator += */
            tp->type = token_type_other;
          else
            {
              /* Operator + */
              phase4_ungetc (c);
              tp->type = token_type_plus;
            }
          return;

        case '@':
          c = phase4_getc ();
          if (c == '"')
            {
              /* Verbatim string literal.  */
              struct mixed_string_buffer literal;

              lexical_context = lc_string;
              mixed_string_buffer_init (&literal, lexical_context,
                                        logical_file_name, logical_line_number);
              for (;;)
                {
                  /* Use phase 2, because phase 4 elides comments and phase 3
                     mixes up the newline characters.  */
                  c = phase2_getc ();
                  if (c == UEOF)
                    break;
                  if (c == '"')
                    {
                      c = phase2_getc ();
                      if (c != '"')
                        {
                          phase2_ungetc (c);
                          break;
                        }
                    }
                  /* No special treatment of newline and backslash here.  */
                  mixed_string_buffer_append_unicode (&literal, c);
                }
              tp->mixed_string = mixed_string_buffer_result (&literal);
              tp->comment = add_reference (savable_comment);
              lexical_context = lc_outside;
              tp->type = token_type_string_literal;
              return;
            }
          FALLTHROUGH; /* so that @identifier is recognized.  */

        default:
          if (c == '\\')
            c = do_getc_unicode_escaped (is_identifier_start);
          if (is_identifier_start (c))
            {
              struct mixed_string_buffer buffer;
              mixed_string_ty *mixed_string;

              mixed_string_buffer_init (&buffer, lexical_context,
                                        logical_file_name, logical_line_number);
              for (;;)
                {
                  mixed_string_buffer_append_unicode (&buffer, c);
                  c = phase4_getc ();
                  if (c == '\\')
                    c = do_getc_unicode_escaped (is_identifier_part);
                  if (!is_identifier_part (c))
                    break;
                }
              phase4_ungetc (c);
              mixed_string = mixed_string_buffer_result (&buffer);
              tp->string = mixed_string_contents (mixed_string);
              mixed_string_free (mixed_string);
              tp->type = token_type_symbol;
              return;
            }
          else
            {
              /* Misc. operator.  */
              tp->type = token_type_other;
              return;
            }
        }
    }
}

/* Supports 3 tokens of pushback.  */
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


/* Compile-time optimization of string literal concatenation.
   Combine "string1" + ... + "stringN" to the concatenated string if
     - the token after this expression is not '.' (because then the last
       string could be part of a method call expression).  */

static token_ty phase7_pushback[2];
static int phase7_pushback_length;

static void
phase7_get (token_ty *tp)
{
  if (phase7_pushback_length)
    {
      *tp = phase7_pushback[--phase7_pushback_length];
      return;
    }

  phase6_get (tp);
  if (tp->type == token_type_string_literal)
    {
      mixed_string_ty *sum = tp->mixed_string;

      for (;;)
        {
          token_ty token2;

          phase6_get (&token2);
          if (token2.type == token_type_plus)
            {
              token_ty token3;

              phase6_get (&token3);
              if (token3.type == token_type_string_literal)
                {
                  token_ty token_after;

                  phase6_get (&token_after);
                  if (token_after.type != token_type_dot)
                    {
                      sum = mixed_string_concat_free1 (sum, token3.mixed_string);

                      phase6_unget (&token_after);
                      free_token (&token3);
                      free_token (&token2);
                      continue;
                    }
                  phase6_unget (&token_after);
                }
              phase6_unget (&token3);
            }
          phase6_unget (&token2);
          break;
        }
      tp->mixed_string = sum;
    }
}

/* Supports 2 tokens of pushback.  */
static void
phase7_unget (token_ty *tp)
{
  if (tp->type != token_type_eof)
    {
      if (phase7_pushback_length == SIZEOF (phase7_pushback))
        abort ();
      phase7_pushback[phase7_pushback_length++] = *tp;
    }
}


static void
x_csharp_lex (token_ty *tp)
{
  phase7_get (tp);
}

/* Supports 2 tokens of pushback.  */
static void
x_csharp_unlex (token_ty *tp)
{
  phase7_unget (tp);
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

      x_csharp_lex (&token);
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

                x_csharp_lex (&token2);
                if (token2.type == token_type_dot)
                  {
                    token_ty token3;

                    x_csharp_lex (&token3);
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
                    x_csharp_unlex (&token3);
                  }
                x_csharp_unlex (&token2);
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
extract_csharp (FILE *f,
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

  logical_line_number = 1;

  phase3_pushback_length = 0;

  last_comment_line = -1;
  last_non_comment_line = -1;

  phase5_pushback_length = 0;
  phase6_pushback_length = 0;
  phase7_pushback_length = 0;

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
