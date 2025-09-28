/* GNU gettext - internationalization aids
   Copyright (C) 1995-2009, 2011, 2019 Free Software Foundation, Inc.

   This file was written by Peter Miller <millerp@canb.auug.org.au>.
   Multibyte character handling by Bruno Haible <haible@clisp.cons.org>.

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
#include "po-lex.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#if HAVE_ICONV
# include <iconv.h>
#endif

#include "c-ctype.h"
#include "uniwidth.h"
#include "gettext.h"
#include "po-charset.h"
#include "xalloc.h"
#include "error.h"
#include "error-progname.h"
#include "xvasprintf.h"
#include "po-error.h"
#include "po-xerror.h"
#include "pos.h"
#include "message.h"
#include "str-list.h"
#include "po-gram-gen2.h"

#define _(str) gettext(str)

#if HAVE_ICONV
# include "unistr.h"
#endif

#if HAVE_DECL_GETC_UNLOCKED
# undef getc
# define getc getc_unlocked
#endif


/* Current position within the PO file.  */
lex_pos_ty gram_pos;
int gram_pos_column;


/* Error handling during the parsing of a PO file.
   These functions can access gram_pos and gram_pos_column.  */

/* VARARGS1 */
void
po_gram_error (const char *fmt, ...)
{
  va_list ap;
  char *buffer;

  va_start (ap, fmt);
  if (vasprintf (&buffer, fmt, ap) < 0)
    error (EXIT_FAILURE, 0, _("memory exhausted"));
  va_end (ap);
  po_xerror (PO_SEVERITY_ERROR, NULL, gram_pos.file_name, gram_pos.line_number,
             gram_pos_column + 1, false, buffer);
  free (buffer);

  if (error_message_count >= gram_max_allowed_errors)
    po_error (EXIT_FAILURE, 0, _("too many errors, aborting"));
}

/* VARARGS2 */
void
po_gram_error_at_line (const lex_pos_ty *pp, const char *fmt, ...)
{
  va_list ap;
  char *buffer;

  va_start (ap, fmt);
  if (vasprintf (&buffer, fmt, ap) < 0)
    error (EXIT_FAILURE, 0, _("memory exhausted"));
  va_end (ap);
  po_xerror (PO_SEVERITY_ERROR, NULL, pp->file_name, pp->line_number,
             (size_t)(-1), false, buffer);
  free (buffer);

  if (error_message_count >= gram_max_allowed_errors)
    po_error (EXIT_FAILURE, 0, _("too many errors, aborting"));
}


/* The lowest level of PO file parsing converts bytes to multibyte characters.
   This is needed
   1. for C compatibility: ISO C 99 section 5.1.1.2 says that the first
      translation phase maps bytes to characters.
   2. to keep track of the current column, for the sake of precise error
      location. Emacs compile.el interprets the column in error messages
      by default as a screen column number, not as character number.
   3. to avoid skipping backslash-newline in the midst of a multibyte
      character. If XY is a multibyte character,  X \ newline Y  is invalid.
 */

/* Multibyte character data type.  */
/* Note this depends on po_lex_charset and po_lex_iconv, which get set
   while the file is being parsed.  */

#define MBCHAR_BUF_SIZE 24

struct mbchar
{
  size_t bytes;         /* number of bytes of current character, > 0 */
#if HAVE_ICONV
  bool uc_valid;        /* true if uc is a valid Unicode character */
  ucs4_t uc;            /* if uc_valid: the current character */
#endif
  char buf[MBCHAR_BUF_SIZE]; /* room for the bytes */
};

/* We want to pass multibyte characters by reference automatically,
   therefore we use an array type.  */
typedef struct mbchar mbchar_t[1];

/* A version of memcpy optimized for the case n <= 1.  */
static inline void
memcpy_small (void *dst, const void *src, size_t n)
{
  if (n > 0)
    {
      char *q = (char *) dst;
      const char *p = (const char *) src;

      *q = *p;
      if (--n > 0)
        do *++q = *++p; while (--n > 0);
    }
}

/* EOF (not a real character) is represented with bytes = 0 and
   uc_valid = false.  */
static inline bool
mb_iseof (const mbchar_t mbc)
{
  return (mbc->bytes == 0);
}

/* Access the current character.  */
static inline const char *
mb_ptr (const mbchar_t mbc)
{
  return mbc->buf;
}
static inline size_t
mb_len (const mbchar_t mbc)
{
  return mbc->bytes;
}

/* Comparison of characters.  */

static inline bool
mb_iseq (const mbchar_t mbc, char sc)
{
  /* Note: It is wrong to compare only mbc->uc, because when the encoding is
     SHIFT_JIS, mbc->buf[0] == '\\' corresponds to mbc->uc == 0x00A5, but we
     want to treat it as an escape character, although it looks like a Yen
     sign.  */
#if HAVE_ICONV && 0
  if (mbc->uc_valid)
    return (mbc->uc == sc); /* wrong! */
  else
#endif
    return (mbc->bytes == 1 && mbc->buf[0] == sc);
}

static inline bool
mb_isnul (const mbchar_t mbc)
{
#if HAVE_ICONV
  if (mbc->uc_valid)
    return (mbc->uc == 0);
  else
#endif
    return (mbc->bytes == 1 && mbc->buf[0] == 0);
}

static inline int
mb_cmp (const mbchar_t mbc1, const mbchar_t mbc2)
{
#if HAVE_ICONV
  if (mbc1->uc_valid && mbc2->uc_valid)
    return (int) mbc1->uc - (int) mbc2->uc;
  else
#endif
    return (mbc1->bytes == mbc2->bytes
            ? memcmp (mbc1->buf, mbc2->buf, mbc1->bytes)
            : mbc1->bytes < mbc2->bytes
              ? (memcmp (mbc1->buf, mbc2->buf, mbc1->bytes) > 0 ? 1 : -1)
              : (memcmp (mbc1->buf, mbc2->buf, mbc2->bytes) >= 0 ? 1 : -1));
}

static inline bool
mb_equal (const mbchar_t mbc1, const mbchar_t mbc2)
{
#if HAVE_ICONV
  if (mbc1->uc_valid && mbc2->uc_valid)
    return mbc1->uc == mbc2->uc;
  else
#endif
    return (mbc1->bytes == mbc2->bytes
            && memcmp (mbc1->buf, mbc2->buf, mbc1->bytes) == 0);
}

/* <ctype.h>, <wctype.h> classification.  */

static inline bool
mb_isascii (const mbchar_t mbc)
{
#if HAVE_ICONV
  if (mbc->uc_valid)
    return (mbc->uc >= 0x0000 && mbc->uc <= 0x007F);
  else
#endif
    return (mbc->bytes == 1
#if CHAR_MIN < 0x00 /* to avoid gcc warning */
            && mbc->buf[0] >= 0x00
#endif
#if CHAR_MAX > 0x7F /* to avoid gcc warning */
            && mbc->buf[0] <= 0x7F
#endif
           );
}

/* Extra <wchar.h> function.  */

/* Unprintable characters appear as a small box of width 1.  */
#define MB_UNPRINTABLE_WIDTH 1

static int
mb_width (const mbchar_t mbc)
{
#if HAVE_ICONV
  if (mbc->uc_valid)
    {
      ucs4_t uc = mbc->uc;
      const char *encoding =
        (po_lex_iconv != (iconv_t)(-1) ? po_lex_charset : "");
      int w = uc_width (uc, encoding);
      /* For unprintable characters, arbitrarily return 0 for control
         characters (except tab) and MB_UNPRINTABLE_WIDTH otherwise.  */
      if (w >= 0)
        return w;
      if (uc >= 0x0000 && uc <= 0x001F)
        {
          if (uc == 0x0009)
            return 8 - (gram_pos_column & 7);
          return 0;
        }
      if ((uc >= 0x007F && uc <= 0x009F) || (uc >= 0x2028 && uc <= 0x2029))
        return 0;
      return MB_UNPRINTABLE_WIDTH;
    }
  else
#endif
    {
      if (mbc->bytes == 1)
        {
          if (
#if CHAR_MIN < 0x00 /* to avoid gcc warning */
              mbc->buf[0] >= 0x00 &&
#endif
              mbc->buf[0] <= 0x1F)
            {
              if (mbc->buf[0] == 0x09)
                return 8 - (gram_pos_column & 7);
              return 0;
            }
          if (mbc->buf[0] == 0x7F)
            return 0;
        }
      return MB_UNPRINTABLE_WIDTH;
    }
}

/* Output.  */
static inline void
mb_putc (const mbchar_t mbc, FILE *stream)
{
  fwrite (mbc->buf, 1, mbc->bytes, stream);
}

/* Assignment.  */
static inline void
mb_setascii (mbchar_t mbc, char sc)
{
  mbc->bytes = 1;
#if HAVE_ICONV
  mbc->uc_valid = 1;
  mbc->uc = sc;
#endif
  mbc->buf[0] = sc;
}

/* Copying a character.  */
static inline void
mb_copy (mbchar_t new_mbc, const mbchar_t old_mbc)
{
  memcpy_small (&new_mbc->buf[0], &old_mbc->buf[0], old_mbc->bytes);
  new_mbc->bytes = old_mbc->bytes;
#if HAVE_ICONV
  if ((new_mbc->uc_valid = old_mbc->uc_valid))
    new_mbc->uc = old_mbc->uc;
#endif
}


/* Multibyte character input.  */

/* Number of characters that can be pushed back.
   We need 1 for lex_getc, plus 1 for lex_ungetc.  */
#define NPUSHBACK 2

/* Data type of a multibyte character input stream.  */
struct mbfile
{
  FILE *fp;
  bool eof_seen;
  int have_pushback;
  unsigned int bufcount;
  char buf[MBCHAR_BUF_SIZE];
  struct mbchar pushback[NPUSHBACK];
};

/* We want to pass multibyte streams by reference automatically,
   therefore we use an array type.  */
typedef struct mbfile mbfile_t[1];

/* Whether invalid multibyte sequences in the input shall be signalled
   or silently tolerated.  */
static bool signal_eilseq;

static inline void
mbfile_init (mbfile_t mbf, FILE *stream)
{
  mbf->fp = stream;
  mbf->eof_seen = false;
  mbf->have_pushback = 0;
  mbf->bufcount = 0;
}

/* Read the next multibyte character from mbf and put it into mbc.
   If a read error occurs, errno is set and ferror (mbf->fp) becomes true.  */
static void
mbfile_getc (mbchar_t mbc, mbfile_t mbf)
{
  size_t bytes;

  /* If EOF has already been seen, don't use getc.  This matters if
     mbf->fp is connected to an interactive tty.  */
  if (mbf->eof_seen)
    goto eof;

  /* Return character pushed back, if there is one.  */
  if (mbf->have_pushback > 0)
    {
      mbf->have_pushback--;
      mb_copy (mbc, &mbf->pushback[mbf->have_pushback]);
      return;
    }

  /* Before using iconv, we need at least one byte.  */
  if (mbf->bufcount == 0)
    {
      int c = getc (mbf->fp);
      if (c == EOF)
        {
          mbf->eof_seen = true;
          goto eof;
        }
      mbf->buf[0] = (unsigned char) c;
      mbf->bufcount++;
    }

#if HAVE_ICONV
  if (po_lex_iconv != (iconv_t)(-1))
    {
      /* Use iconv on an increasing number of bytes.  Read only as many
         bytes from mbf->fp as needed.  This is needed to give reasonable
         interactive behaviour when mbf->fp is connected to an interactive
         tty.  */
      for (;;)
        {
          unsigned char scratchbuf[64];
          const char *inptr = &mbf->buf[0];
          size_t insize = mbf->bufcount;
          char *outptr = (char *) &scratchbuf[0];
          size_t outsize = sizeof (scratchbuf);

          size_t res = iconv (po_lex_iconv,
                              (ICONV_CONST char **) &inptr, &insize,
                              &outptr, &outsize);
          /* We expect that a character has been produced if and only if
             some input bytes have been consumed.  */
          if ((insize < mbf->bufcount) != (outsize < sizeof (scratchbuf)))
            abort ();
          if (outsize == sizeof (scratchbuf))
            {
              /* No character has been produced.  Must be an error.  */
              if (res != (size_t)(-1))
                abort ();

              if (errno == EILSEQ)
                {
                  /* An invalid multibyte sequence was encountered.  */
                  /* Return a single byte.  */
                  if (signal_eilseq)
                    po_gram_error (_("invalid multibyte sequence"));
                  bytes = 1;
                  mbc->uc_valid = false;
                  break;
                }
              else if (errno == EINVAL)
                {
                  /* An incomplete multibyte character.  */
                  int c;

                  if (mbf->bufcount == MBCHAR_BUF_SIZE)
                    {
                      /* An overlong incomplete multibyte sequence was
                         encountered.  */
                      /* Return a single byte.  */
                      bytes = 1;
                      mbc->uc_valid = false;
                      break;
                    }

                  /* Read one more byte and retry iconv.  */
                  c = getc (mbf->fp);
                  if (c == EOF)
                    {
                      mbf->eof_seen = true;
                      if (ferror (mbf->fp))
                        goto eof;
                      if (signal_eilseq)
                        po_gram_error (_("incomplete multibyte sequence at end of file"));
                      bytes = mbf->bufcount;
                      mbc->uc_valid = false;
                      break;
                    }
                  mbf->buf[mbf->bufcount++] = (unsigned char) c;
                  if (c == '\n')
                    {
                      if (signal_eilseq)
                        po_gram_error (_("incomplete multibyte sequence at end of line"));
                      bytes = mbf->bufcount - 1;
                      mbc->uc_valid = false;
                      break;
                    }
                }
              else
                {
                  const char *errno_description = strerror (errno);
                  po_xerror (PO_SEVERITY_FATAL_ERROR, NULL, NULL, 0, 0, false,
                             xasprintf ("%s: %s",
                                        _("iconv failure"),
                                        errno_description));
                }
            }
          else
            {
              size_t outbytes = sizeof (scratchbuf) - outsize;
              bytes = mbf->bufcount - insize;

              /* We expect that one character has been produced.  */
              if (bytes == 0)
                abort ();
              if (outbytes == 0)
                abort ();
              /* Convert it from UTF-8 to UCS-4.  */
              if (u8_mbtoucr (&mbc->uc, scratchbuf, outbytes) < (int) outbytes)
                {
                  /* scratchbuf contains an out-of-range Unicode character
                     (> 0x10ffff).  */
                  if (signal_eilseq)
                    po_gram_error (_("invalid multibyte sequence"));
                  mbc->uc_valid = false;
                  break;
                }
              mbc->uc_valid = true;
              break;
            }
        }
    }
  else
#endif
    {
      if (po_lex_weird_cjk
          /* Special handling of encodings with CJK structure.  */
          && (unsigned char) mbf->buf[0] >= 0x80)
        {
          if (mbf->bufcount == 1)
            {
              /* Read one more byte.  */
              int c = getc (mbf->fp);
              if (c == EOF)
                {
                  if (ferror (mbf->fp))
                    {
                      mbf->eof_seen = true;
                      goto eof;
                    }
                }
              else
                {
                  mbf->buf[1] = (unsigned char) c;
                  mbf->bufcount++;
                }
            }
          if (mbf->bufcount >= 2 && (unsigned char) mbf->buf[1] >= 0x30)
            /* Return a double byte.  */
            bytes = 2;
          else
            /* Return a single byte.  */
            bytes = 1;
        }
      else
        {
          /* Return a single byte.  */
          bytes = 1;
        }
#if HAVE_ICONV
      mbc->uc_valid = false;
#endif
    }

  /* Return the multibyte sequence mbf->buf[0..bytes-1].  */
  memcpy_small (&mbc->buf[0], &mbf->buf[0], bytes);
  mbc->bytes = bytes;

  mbf->bufcount -= bytes;
  if (mbf->bufcount > 0)
    {
      /* It's not worth calling memmove() for so few bytes.  */
      unsigned int count = mbf->bufcount;
      char *p = &mbf->buf[0];

      do
        {
          *p = *(p + bytes);
          p++;
        }
      while (--count > 0);
    }
  return;

eof:
  /* An mbchar_t with bytes == 0 is used to indicate EOF.  */
  mbc->bytes = 0;
#if HAVE_ICONV
  mbc->uc_valid = false;
#endif
  return;
}

static void
mbfile_ungetc (const mbchar_t mbc, mbfile_t mbf)
{
  if (mbf->have_pushback >= NPUSHBACK)
    abort ();
  mb_copy (&mbf->pushback[mbf->have_pushback], mbc);
  mbf->have_pushback++;
}


/* Lexer variables.  */

static mbfile_t mbf;
unsigned int gram_max_allowed_errors = 20;
static bool po_lex_obsolete;
static bool po_lex_previous;
static bool pass_comments = false;
bool pass_obsolete_entries = false;


/* Prepare lexical analysis.  */
void
lex_start (FILE *fp, const char *real_filename, const char *logical_filename)
{
  /* Ignore the logical_filename, because PO file entries already have
     their file names attached.  But use real_filename for error messages.  */
  gram_pos.file_name = xstrdup (real_filename);

  mbfile_init (mbf, fp);

  gram_pos.line_number = 1;
  gram_pos_column = 0;
  signal_eilseq = true;
  po_lex_obsolete = false;
  po_lex_previous = false;
  po_lex_charset_init ();
}

/* Terminate lexical analysis.  */
void
lex_end ()
{
  mbf->fp = NULL;
  gram_pos.file_name = NULL;
  gram_pos.line_number = 0;
  gram_pos_column = 0;
  signal_eilseq = false;
  po_lex_obsolete = false;
  po_lex_previous = false;
  po_lex_charset_close ();
}


/* Read a single character, dealing with backslash-newline.
   Also keep track of the current line number and column number.  */
static void
lex_getc (mbchar_t mbc)
{
  for (;;)
    {
      mbfile_getc (mbc, mbf);

      if (mb_iseof (mbc))
        {
          if (ferror (mbf->fp))
           bomb:
            {
              const char *errno_description = strerror (errno);
              po_xerror (PO_SEVERITY_FATAL_ERROR, NULL, NULL, 0, 0, false,
                         xasprintf ("%s: %s",
                                    xasprintf (_("error while reading \"%s\""),
                                               gram_pos.file_name),
                                    errno_description));
            }
          break;
        }

      if (mb_iseq (mbc, '\n'))
        {
          gram_pos.line_number++;
          gram_pos_column = 0;
          break;
        }

      gram_pos_column += mb_width (mbc);

      if (mb_iseq (mbc, '\\'))
        {
          mbchar_t mbc2;

          mbfile_getc (mbc2, mbf);

          if (mb_iseof (mbc2))
            {
              if (ferror (mbf->fp))
                goto bomb;
              break;
            }

          if (!mb_iseq (mbc2, '\n'))
            {
              mbfile_ungetc (mbc2, mbf);
              break;
            }

          gram_pos.line_number++;
          gram_pos_column = 0;
        }
      else
        break;
    }
}


static void
lex_ungetc (const mbchar_t mbc)
{
  if (!mb_iseof (mbc))
    {
      if (mb_iseq (mbc, '\n'))
        /* Decrement the line number, but don't care about the column.  */
        gram_pos.line_number--;
      else
        /* Decrement the column number.  Also works well enough for tabs.  */
        gram_pos_column -= mb_width (mbc);

      mbfile_ungetc (mbc, mbf);
    }
}


static int
keyword_p (const char *s)
{
  if (!po_lex_previous)
    {
      if (!strcmp (s, "domain"))
        return DOMAIN;
      if (!strcmp (s, "msgid"))
        return MSGID;
      if (!strcmp (s, "msgid_plural"))
        return MSGID_PLURAL;
      if (!strcmp (s, "msgstr"))
        return MSGSTR;
      if (!strcmp (s, "msgctxt"))
        return MSGCTXT;
    }
  else
    {
      /* Inside a "#|" context, the keywords have a different meaning.  */
      if (!strcmp (s, "msgid"))
        return PREV_MSGID;
      if (!strcmp (s, "msgid_plural"))
        return PREV_MSGID_PLURAL;
      if (!strcmp (s, "msgctxt"))
        return PREV_MSGCTXT;
    }
  po_gram_error_at_line (&gram_pos, _("keyword \"%s\" unknown"), s);
  return NAME;
}


static int
control_sequence ()
{
  mbchar_t mbc;
  int val;
  int max;

  lex_getc (mbc);
  if (mb_len (mbc) == 1)
    switch (mb_ptr (mbc) [0])
      {
      case 'n':
        return '\n';

      case 't':
        return '\t';

      case 'b':
        return '\b';

      case 'r':
        return '\r';

      case 'f':
        return '\f';

      case 'v':
        return '\v';

      case 'a':
        return '\a';

      case '\\':
      case '"':
        return mb_ptr (mbc) [0];

      case '0': case '1': case '2': case '3':
      case '4': case '5': case '6': case '7':
        val = 0;
        max = 0;
        for (;;)
          {
            char c = mb_ptr (mbc) [0];
            /* Warning: not portable, can't depend on '0'..'7' ordering.  */
            val = val * 8 + (c - '0');
            if (++max == 3)
              break;
            lex_getc (mbc);
            if (mb_len (mbc) == 1)
              switch (mb_ptr (mbc) [0])
                {
                case '0': case '1': case '2': case '3':
                case '4': case '5': case '6': case '7':
                  continue;

                default:
                  break;
                }
            lex_ungetc (mbc);
            break;
          }
        return val;

      case 'x':
        lex_getc (mbc);
        if (mb_iseof (mbc) || mb_len (mbc) != 1
            || !c_isxdigit (mb_ptr (mbc) [0]))
          break;

        val = 0;
        for (;;)
          {
            char c = mb_ptr (mbc) [0];
            val *= 16;
            if (c_isdigit (c))
              /* Warning: not portable, can't depend on '0'..'9' ordering */
              val += c - '0';
            else if (c_isupper (c))
              /* Warning: not portable, can't depend on 'A'..'F' ordering */
              val += c - 'A' + 10;
            else
              /* Warning: not portable, can't depend on 'a'..'f' ordering */
              val += c - 'a' + 10;

            lex_getc (mbc);
            if (mb_len (mbc) == 1)
              switch (mb_ptr (mbc) [0])
                {
                case '0': case '1': case '2': case '3': case '4':
                case '5': case '6': case '7': case '8': case '9':
                case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
                case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
                  continue;

                default:
                  break;
                }
            lex_ungetc (mbc);
            break;
          }
        return val;

      /* FIXME: \u and \U are not handled.  */
      }
  lex_ungetc (mbc);
  po_gram_error (_("invalid control sequence"));
  return ' ';
}


/* Return the next token in the PO file.  The return codes are defined
   in "po-gram-gen2.h".  Associated data is put in 'po_gram_lval'.  */
int
po_gram_lex ()
{
  static char *buf;
  static size_t bufmax;
  mbchar_t mbc;
  size_t bufpos;

  for (;;)
    {
      lex_getc (mbc);

      if (mb_iseof (mbc))
        /* Yacc want this for end of file.  */
        return 0;

      if (mb_len (mbc) == 1)
        switch (mb_ptr (mbc) [0])
          {
          case '\n':
            po_lex_obsolete = false;
            po_lex_previous = false;
            /* Ignore whitespace, not relevant for the grammar.  */
            break;

          case ' ':
          case '\t':
          case '\r':
          case '\f':
          case '\v':
            /* Ignore whitespace, not relevant for the grammar.  */
            break;

          case '#':
            lex_getc (mbc);
            if (mb_iseq (mbc, '~'))
              /* A pseudo-comment beginning with #~ is found.  This is
                 not a comment.  It is the format for obsolete entries.
                 We simply discard the "#~" prefix.  The following
                 characters are expected to be well formed.  */
              {
                po_lex_obsolete = true;
                /* A pseudo-comment beginning with #~| denotes a previous
                   untranslated string in an obsolete entry.  This does not
                   make much sense semantically, and is implemented here
                   for completeness only.  */
                lex_getc (mbc);
                if (mb_iseq (mbc, '|'))
                  po_lex_previous = true;
                else
                  lex_ungetc (mbc);
                break;
              }
            if (mb_iseq (mbc, '|'))
              /* A pseudo-comment beginning with #| is found.  This is
                 the previous untranslated string.  We discard the "#|"
                 prefix, but change the keywords and string returns
                 accordingly.  */
              {
                po_lex_previous = true;
                break;
              }

            /* Accumulate comments into a buffer.  If we have been asked
               to pass comments, generate a COMMENT token, otherwise
               discard it.  */
            signal_eilseq = false;
            if (pass_comments)
              {
                bufpos = 0;
                for (;;)
                  {
                    while (bufpos + mb_len (mbc) >= bufmax)
                      {
                        bufmax += 100;
                        buf = xrealloc (buf, bufmax);
                      }
                    if (mb_iseof (mbc) || mb_iseq (mbc, '\n'))
                      break;

                    memcpy_small (&buf[bufpos], mb_ptr (mbc), mb_len (mbc));
                    bufpos += mb_len (mbc);

                    lex_getc (mbc);
                  }
                buf[bufpos] = '\0';

                po_gram_lval.string.string = buf;
                po_gram_lval.string.pos = gram_pos;
                po_gram_lval.string.obsolete = po_lex_obsolete;
                po_lex_obsolete = false;
                signal_eilseq = true;
                return COMMENT;
              }
            else
              {
                /* We do this in separate loop because collecting large
                   comments while they get not passed to the upper layers
                   is not very efficient.  */
                while (!mb_iseof (mbc) && !mb_iseq (mbc, '\n'))
                  lex_getc (mbc);
                po_lex_obsolete = false;
                signal_eilseq = true;
              }
            break;

          case '"':
            /* Accumulate a string.  */
            bufpos = 0;
            for (;;)
              {
                lex_getc (mbc);
                while (bufpos + mb_len (mbc) >= bufmax)
                  {
                    bufmax += 100;
                    buf = xrealloc (buf, bufmax);
                  }
                if (mb_iseof (mbc))
                  {
                    po_gram_error_at_line (&gram_pos,
                                           _("end-of-file within string"));
                    break;
                  }
                if (mb_iseq (mbc, '\n'))
                  {
                    po_gram_error_at_line (&gram_pos,
                                           _("end-of-line within string"));
                    break;
                  }
                if (mb_iseq (mbc, '"'))
                  break;
                if (mb_iseq (mbc, '\\'))
                  {
                    buf[bufpos++] = control_sequence ();
                    continue;
                  }

                /* Add mbc to the accumulator.  */
                memcpy_small (&buf[bufpos], mb_ptr (mbc), mb_len (mbc));
                bufpos += mb_len (mbc);
              }
            buf[bufpos] = '\0';

            /* Strings cannot contain the msgctxt separator, because it cannot
               be faithfully represented in the msgid of a .mo file.  */
            if (strchr (buf, MSGCTXT_SEPARATOR) != NULL)
              po_gram_error_at_line (&gram_pos,
                                     _("context separator <EOT> within string"));

            /* FIXME: Treatment of embedded \000 chars is incorrect.  */
            po_gram_lval.string.string = xstrdup (buf);
            po_gram_lval.string.pos = gram_pos;
            po_gram_lval.string.obsolete = po_lex_obsolete;
            return (po_lex_previous ? PREV_STRING : STRING);

          case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
          case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
          case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
          case 's': case 't': case 'u': case 'v': case 'w': case 'x':
          case 'y': case 'z':
          case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
          case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
          case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
          case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
          case 'Y': case 'Z':
          case '_': case '$':
            bufpos = 0;
            for (;;)
              {
                char c = mb_ptr (mbc) [0];
                if (bufpos + 1 >= bufmax)
                  {
                    bufmax += 100;
                    buf = xrealloc (buf, bufmax);
                  }
                buf[bufpos++] = c;
                lex_getc (mbc);
                if (mb_len (mbc) == 1)
                  switch (mb_ptr (mbc) [0])
                    {
                    default:
                      break;
                    case 'a': case 'b': case 'c': case 'd': case 'e':
                    case 'f': case 'g': case 'h': case 'i': case 'j':
                    case 'k': case 'l': case 'm': case 'n': case 'o':
                    case 'p': case 'q': case 'r': case 's': case 't':
                    case 'u': case 'v': case 'w': case 'x': case 'y':
                    case 'z':
                    case 'A': case 'B': case 'C': case 'D': case 'E':
                    case 'F': case 'G': case 'H': case 'I': case 'J':
                    case 'K': case 'L': case 'M': case 'N': case 'O':
                    case 'P': case 'Q': case 'R': case 'S': case 'T':
                    case 'U': case 'V': case 'W': case 'X': case 'Y':
                    case 'Z':
                    case '_': case '$':
                    case '0': case '1': case '2': case '3': case '4':
                    case '5': case '6': case '7': case '8': case '9':
                      continue;
                    }
                break;
              }
            lex_ungetc (mbc);

            buf[bufpos] = '\0';

            {
              int k = keyword_p (buf);
              if (k == NAME)
                {
                  po_gram_lval.string.string = xstrdup (buf);
                  po_gram_lval.string.pos = gram_pos;
                  po_gram_lval.string.obsolete = po_lex_obsolete;
                }
              else
                {
                  po_gram_lval.pos.pos = gram_pos;
                  po_gram_lval.pos.obsolete = po_lex_obsolete;
                }
              return k;
            }

          case '0': case '1': case '2': case '3': case '4':
          case '5': case '6': case '7': case '8': case '9':
            bufpos = 0;
            for (;;)
              {
                char c = mb_ptr (mbc) [0];
                if (bufpos + 1 >= bufmax)
                  {
                    bufmax += 100;
                    buf = xrealloc (buf, bufmax + 1);
                  }
                buf[bufpos++] = c;
                lex_getc (mbc);
                if (mb_len (mbc) == 1)
                  switch (mb_ptr (mbc) [0])
                    {
                    default:
                      break;

                    case '0': case '1': case '2': case '3': case '4':
                    case '5': case '6': case '7': case '8': case '9':
                      continue;
                    }
                break;
              }
            lex_ungetc (mbc);

            buf[bufpos] = '\0';

            po_gram_lval.number.number = atol (buf);
            po_gram_lval.number.pos = gram_pos;
            po_gram_lval.number.obsolete = po_lex_obsolete;
            return NUMBER;

          case '[':
            po_gram_lval.pos.pos = gram_pos;
            po_gram_lval.pos.obsolete = po_lex_obsolete;
            return '[';

          case ']':
            po_gram_lval.pos.pos = gram_pos;
            po_gram_lval.pos.obsolete = po_lex_obsolete;
            return ']';

          default:
            /* This will cause a syntax error.  */
            return JUNK;
          }
      else
        /* This will cause a syntax error.  */
        return JUNK;
    }
}


/* po_gram_lex() can return comments as COMMENT.  Switch this on or off.  */
void
po_lex_pass_comments (bool flag)
{
  pass_comments = flag;
}


/* po_gram_lex() can return obsolete entries as if they were normal entries.
   Switch this on or off.  */
void
po_lex_pass_obsolete_entries (bool flag)
{
  pass_obsolete_entries = flag;
}
