/* xgettext Vala backend.
   Copyright (C) 2013-2014, 2018-2023 Free Software Foundation, Inc.

   This file was written by Daiki Ueno <ueno@gnu.org>, 2013.

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
#include "x-vala.h"

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
#include "xalloc.h"
#include "xvasprintf.h"
#include "mem-hash-map.h"
#include "po-charset.h"
#include "gettext.h"

#define _(s) gettext(s)

#define SIZEOF(a) (sizeof(a) / sizeof(a[0]))

/* The Vala syntax is defined in the Vala Reference Manual
   https://www.vala-project.org/doc/vala/.
   See also vala/valascanner.vala.  */

/* ====================== Keyword set customization.  ====================== */

/* If true extract all strings.  */
static bool extract_all = false;

static hash_table keywords;
static bool default_keywords = true;


void
x_vala_extract_all ()
{
  extract_all = true;
}


static void
add_keyword (const char *name, hash_table *keywords)
{
  if (name == NULL)
    default_keywords = false;
  else
    {
      const char *end;
      struct callshape shape;
      const char *colon;

      if (keywords->table == NULL)
        hash_init (keywords, 100);

      split_keywordspec (name, &end, &shape);

      /* The characters between name and end should form a valid C identifier.
         A colon means an invalid parse in split_keywordspec().  */
      colon = strchr (name, ':');
      if (colon == NULL || colon >= end)
        insert_keyword_callshape (keywords, name, end - name, &shape);
    }
}

void
x_vala_keyword (const char *name)
{
  add_keyword (name, &keywords);
}

static void
init_keywords ()
{
  if (default_keywords)
    {
      /* When adding new keywords here, also update the documentation in
         xgettext.texi!  */
      x_vala_keyword ("dgettext:2");
      x_vala_keyword ("dcgettext:2");
      x_vala_keyword ("ngettext:1,2");
      x_vala_keyword ("dngettext:2,3");
      x_vala_keyword ("dpgettext:2g");
      x_vala_keyword ("dpgettext2:2c,3");
      x_vala_keyword ("_");
      x_vala_keyword ("Q_");
      x_vala_keyword ("N_");
      x_vala_keyword ("NC_:1c,2");

      default_keywords = false;
    }
}

void
init_flag_table_vala ()
{
  /* Vala leaves string formatting to Glib functions and thus the
     format string is exactly same as C.  See also
     vapi/glib-2.0.vapi.  */

  xgettext_record_flag ("dgettext:2:pass-c-format!Vala");
  xgettext_record_flag ("dcgettext:2:pass-c-format!Vala");
  xgettext_record_flag ("ngettext:1:pass-c-format!Vala");
  xgettext_record_flag ("ngettext:2:pass-c-format!Vala");
  xgettext_record_flag ("dngettext:2:pass-c-format!Vala");
  xgettext_record_flag ("dngettext:3:pass-c-format!Vala");
  xgettext_record_flag ("dpgettext:2:pass-c-format!Vala");
  xgettext_record_flag ("dpgettext2:3:pass-c-format!Vala");
  xgettext_record_flag ("_:1:pass-c-format!Vala");
  xgettext_record_flag ("Q_:1:pass-c-format!Vala");
  xgettext_record_flag ("N_:1:pass-c-format!Vala");
  xgettext_record_flag ("NC_:2:pass-c-format!Vala");

  xgettext_record_flag ("printf:1:c-format!Vala");
  xgettext_record_flag ("vprintf:1:c-format!Vala");
}


/* ======================== Reading of characters.  ======================== */

/* The input file stream.  */
static FILE *fp;


/* 1. line_number handling.  */

#define MAX_PHASE1_PUSHBACK 16
static unsigned char phase1_pushback[MAX_PHASE1_PUSHBACK];
static int phase1_pushback_length;


static int
phase1_getc ()
{
  int c;

  if (phase1_pushback_length)
    c = phase1_pushback[--phase1_pushback_length];
  else
    {
      c = getc (fp);
      if (c == EOF)
        {
          if (ferror (fp))
            error (EXIT_FAILURE, errno, _("error while reading \"%s\""),
                   real_file_name);
          return EOF;
        }
    }

  if (c == '\n')
    ++line_number;
  return c;
}


/* Supports 2 characters of pushback.  */
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


/* These are for tracking whether comments count as immediately before
   keyword.  */
static int last_comment_line;
static int last_non_comment_line;

/* Accumulating comments.  */

static char *buffer;
static size_t bufmax;
static size_t buflen;

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
      buffer = xrealloc (buffer, bufmax);
    }
  buffer[buflen++] = c;
}

static inline void
comment_line_end (size_t chars_to_remove)
{
  buflen -= chars_to_remove;
  while (buflen >= 1
         && (buffer[buflen - 1] == ' ' || buffer[buflen - 1] == '\t'))
    --buflen;
  if (chars_to_remove == 0 && buflen >= bufmax)
    {
      bufmax = 2 * bufmax + 10;
      buffer = xrealloc (buffer, bufmax);
    }
  buffer[buflen] = '\0';
  savable_comment_add (buffer);
}


/* 2. Replace each comment that is not inside a character constant or
   string literal with a space character.  */

static int
phase2_getc ()
{
  int c;
  bool last_was_star;

  c = phase1_getc ();
  if (c != '/')
    return c;
  c = phase1_getc ();
  switch (c)
    {
    default:
      phase1_ungetc (c);
      return '/';

    case '*':
      /* C comment.  */
      comment_start ();
      last_was_star = false;
      for (;;)
        {
          c = phase1_getc ();
          if (c == EOF)
            break;
          /* We skip all leading white space, but not EOLs.  */
          if (!(buflen == 0 && (c == ' ' || c == '\t')))
            comment_add (c);
          switch (c)
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
      /* C++ or ISO C 99 comment.  */
      comment_start ();
      for (;;)
        {
          c = phase1_getc ();
          if (c == '\n' || c == EOF)
            break;
          /* We skip all leading white space, but not EOLs.  */
          if (!(buflen == 0 && (c == ' ' || c == '\t')))
            comment_add (c);
        }
      comment_line_end (0);
      last_comment_line = line_number;
      return '\n';
    }
}


static void
phase2_ungetc (int c)
{
  phase1_ungetc (c);
}


/* ========================== Reading of tokens.  ========================== */

enum token_type_ty
{
  token_type_character_constant,        /* 'x' */
  token_type_eof,
  token_type_lparen,                    /* ( */
  token_type_rparen,                    /* ) */
  token_type_lbrace,                    /* { */
  token_type_rbrace,                    /* } */
  token_type_assign,                    /* = += -= *= /= %= <<= >>= &= |= ^= */
  token_type_return,                    /* return */
  token_type_plus,                      /* + */
  token_type_arithmetic_operator,       /* - * / % << >> & | ^ */
  token_type_equality_test_operator,    /* == < > >= <= != */
  token_type_logic_operator,            /* ! && || */
  token_type_comma,                     /* , */
  token_type_question,                  /* ? */
  token_type_colon,                     /* : */
  token_type_number,                    /* 2.7 */
  token_type_string_literal,            /* "abc" */
  token_type_string_template,           /* @"abc" */
  token_type_regex_literal,             /* /.../ */
  token_type_symbol,                    /* if else etc. */
  token_type_other
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
      mixed_string_free (tp->mixed_string);
      drop_reference (tp->comment);
    }
}


/* Return value of phase7_getc when EOF is reached.  */
#define P7_EOF (-1)

/* Replace escape sequences within character strings with their single
   character equivalents.  */
#define P7_QUOTES (-3)
#define P7_QUOTE (-4)
#define P7_NEWLINE (-5)

/* Convert an UTF-16 or UTF-32 code point to a return value that can be
   distinguished from a single-byte return value.  */
#define UNICODE(code) (0x100 + (code))

/* Test a return value of phase7_getuc whether it designates an UTF-16 or
   UTF-32 code point.  */
#define IS_UNICODE(p7_result) ((p7_result) >= 0x100)

/* Extract the UTF-16 or UTF-32 code of a return value that satisfies
   IS_UNICODE.  */
#define UNICODE_VALUE(p7_result) ((p7_result) - 0x100)


static int
phase7_getc ()
{
  int c, j;

  /* Use phase 1, because phase 2 elides comments.  */
  c = phase1_getc ();

  if (c == EOF)
    return P7_EOF;

  /* Return a magic newline indicator, so that we can distinguish
     between the user requesting a newline in the string (e.g. using
     "\n" or "\012") from the user failing to terminate the string or
     character constant.  The ANSI C standard says: 3.1.3.4 Character
     Constants contain "any character except single quote, backslash or
     newline; or an escape sequence" and 3.1.4 String Literals contain
     "any character except double quote, backslash or newline; or an
     escape sequence".

     Most compilers give a fatal error in this case, however gcc is
     stupidly silent, even though this is a very common typo.  OK, so
     "gcc --pedantic" will tell me, but that gripes about too much other
     stuff.  Could I have a "gcc -Wnewline-in-string" option, or
     better yet a "gcc -fno-newline-in-string" option, please?  Gcc is
     also inconsistent between string literals and character constants:
     you may not embed newlines in character constants; try it, you get
     a useful diagnostic.  --PMiller  */
  if (c == '\n')
    return P7_NEWLINE;

  if (c == '"')
    return P7_QUOTES;
  if (c == '\'')
    return P7_QUOTE;
  if (c != '\\')
    return c;
  c = phase1_getc ();
  switch (c)
    {
    default:
      /* Unknown escape sequences really should be an error, but just
         ignore them, and let the real compiler complain.  */
      phase1_ungetc (c);
      return '\\';

    case '"':
    case '\'':
    case '\\':
    case '$':
      return c;

    case 'b':
      return '\b';

    case 'f':
      return '\f';
    case 'n':
      return '\n';
    case 'r':
      return '\r';
    case 't':
      return '\t';
    case 'v':
      return '\v';

    case 'x':
      c = phase1_getc ();
      switch (c)
        {
        default:
          phase1_ungetc (c);
          phase1_ungetc ('x');
          return '\\';

        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
          break;
        }
      {
        int n;
        bool overflow;

        n = 0;
        overflow = false;

        for (;;)
          {
            switch (c)
              {
              default:
                phase1_ungetc (c);
                if (overflow)
                  {
                    error_with_progname = false;
                    error (0, 0, _("%s:%d: warning: hexadecimal escape sequence out of range"),
                           logical_file_name, line_number);
                    error_with_progname = true;
                  }
                return n;

              case '0': case '1': case '2': case '3': case '4':
              case '5': case '6': case '7': case '8': case '9':
                if (n < 0x100 / 16)
                  n = n * 16 + c - '0';
                else
                  overflow = true;
              break;

              case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
                if (n < 0x100 / 16)
                  n = n * 16 + 10 + c - 'A';
                else
                  overflow = true;
                break;

              case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
                if (n < 0x100 / 16)
                  n = n * 16 + 10 + c - 'a';
                else
                  overflow = true;
                break;
              }
            c = phase1_getc ();
          }
      }

    case '0':
      {
        int n;

        n = 0;
        for (j = 0; j < 3; ++j)
          {
            n = n * 8 + c - '0';
            c = phase1_getc ();
            switch (c)
              {
              default:
                break;

              case '0': case '1': case '2': case '3':
              case '4': case '5': case '6': case '7':
                continue;
              }
            break;
          }
        phase1_ungetc (c);
        return n;
      }

    case 'u':
      {
        unsigned char buf[8];
        int n;

        n = 0;
        for (j = 0; j < 4; j++)
          {
            int c1 = phase1_getc ();

            if (c1 >= '0' && c1 <= '9')
              n = (n << 4) + (c1 - '0');
            else if (c1 >= 'A' && c1 <= 'F')
              n = (n << 4) + (c1 - 'A' + 10);
            else if (c1 >= 'a' && c1 <= 'f')
              n = (n << 4) + (c1 - 'a' + 10);
            else
              {
                phase1_ungetc (c1);
                while (--j >= 0)
                  phase1_ungetc (buf[j]);
                phase1_ungetc (c);
                return '\\';
              }

            buf[j] = c1;
          }

        if (n < 0x110000)
          return UNICODE (n);

        error_with_progname = false;
        error (0, 0, _("%s:%d: warning: invalid Unicode character"),
               logical_file_name, line_number);
        error_with_progname = true;

        while (--j >= 0)
          phase1_ungetc (buf[j]);
        phase1_ungetc (c);
        return '\\';
      }
    }
}


static void
phase7_ungetc (int c)
{
  phase1_ungetc (c);
}


/* 3. Parse each resulting logical line as preprocessing tokens and
   white space.  Preprocessing tokens and Vala tokens don't always
   match.  */

static token_ty phase3_pushback[2];
static int phase3_pushback_length;


static token_type_ty last_token_type;

static void
phase3_scan_regex ()
{
    int c;

    for (;;)
      {
        c = phase1_getc ();
        if (c == '/')
          break;
        if (c == '\\')
          {
            c = phase1_getc ();
            if (c != EOF)
              continue;
          }
        if (c == EOF)
          {
            error_with_progname = false;
            error (0, 0,
                   _("%s:%d: warning: regular expression literal terminated too early"),
                   logical_file_name, line_number);
            error_with_progname = true;
            return;
          }
      }

    c = phase2_getc ();
    if (!(c == 'i' || c == 's' || c == 'm' || c == 'x'))
      phase2_ungetc (c);
}

static void
phase3_get (token_ty *tp)
{
  static char *buffer;
  static int bufmax;
  int bufpos;

#undef APPEND
#define APPEND(c)                               \
  do                                            \
    {                                           \
      if (bufpos >= bufmax)                     \
        {                                       \
          bufmax = 2 * bufmax + 10;             \
          buffer = xrealloc (buffer, bufmax);   \
        }                                       \
      buffer[bufpos++] = c;                     \
    }                                           \
  while (0)

  if (phase3_pushback_length)
    {
      *tp = phase3_pushback[--phase3_pushback_length];
      last_token_type = tp->type;
      return;
    }

  for (;;)
    {
      bool template;
      bool verbatim;
      int c;

      tp->line_number = line_number;
      c = phase2_getc ();

      switch (c)
        {
        case EOF:
          tp->type = last_token_type = token_type_eof;
          return;

        case '\n':
          if (last_non_comment_line > last_comment_line)
            savable_comment_reset ();
          FALLTHROUGH;
        case ' ':
        case '\f':
        case '\t':
          /* Ignore whitespace and comments.  */
          continue;
        default:
          break;
        }

      last_non_comment_line = tp->line_number;
      template = false;
      verbatim = false;

      switch (c)
        {
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
        case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
        case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
        case 'V': case 'W': case 'X': case 'Y': case 'Z':
        case '_':
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
        case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
        case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
        case 'v': case 'w': case 'x': case 'y': case 'z':
          bufpos = 0;
          for (;;)
            {
              APPEND (c);
              c = phase2_getc ();
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
                  phase2_ungetc (c);
                  break;
                }
              break;
            }
          APPEND (0);
          if (strcmp (buffer, "return") == 0)
            tp->type = last_token_type = token_type_return;
          else
            {
              tp->string = xstrdup (buffer);
              tp->type = last_token_type = token_type_symbol;
            }
          return;

        case '.':
          c = phase2_getc ();
          phase2_ungetc (c);
          switch (c)
            {
            default:
              tp->string = xstrdup (".");
              tp->type = last_token_type = token_type_symbol;
              return;

            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
              c = '.';
              break;
            }
          FALLTHROUGH;

        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
          /* The preprocessing number token is more "generous" than the C
             number tokens.  This is mostly due to token pasting (another
             thing we can ignore here).  */
          bufpos = 0;
          for (;;)
            {
              APPEND (c);
              c = phase2_getc ();
              switch (c)
                {
                case 'e':
                case 'E':
                  APPEND (c);
                  c = phase2_getc ();
                  if (c != '+' && c != '-')
                    {
                      phase2_ungetc (c);
                      break;
                    }
                  continue;

                case 'A': case 'B': case 'C': case 'D':           case 'F':
                case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
                case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
                case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
                case 'Y': case 'Z':
                case 'a': case 'b': case 'c': case 'd':           case 'f':
                case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
                case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
                case 's': case 't': case 'u': case 'v': case 'w': case 'x':
                case 'y': case 'z':
                case '0': case '1': case '2': case '3': case '4':
                case '5': case '6': case '7': case '8': case '9':
                case '.':
                  continue;

                default:
                  phase2_ungetc (c);
                  break;
                }
              break;
            }
          APPEND (0);
          tp->type = last_token_type = token_type_number;
          return;

        case '\'':
          for (;;)
            {
              c = phase7_getc ();
              if (c == P7_NEWLINE)
                {
                  error_with_progname = false;
                  error (0, 0, _("%s:%d: warning: unterminated character constant"),
                         logical_file_name, line_number - 1);
                  error_with_progname = true;
                  phase7_ungetc ('\n');
                  break;
                }
              if (c == P7_EOF || c == P7_QUOTE)
                break;
            }
          tp->type = last_token_type = token_type_character_constant;
          return;

          /* Vala provides strings in three different formats.

             Usual string literals:
               "..."
             Verbatim string literals:
               """...""" (where ... can include newlines and double quotes)
             String templates.
               @"...", @"""..."""

             Note that, with the current implementation string
             templates are not subject to translation, because they are
             inspected at compile time.  For example, the following code

               string bar = "bar";
               string foo = _(@"foo $bar");

             will be translated into the C code, like:

               _(g_strconcat ("foo ", "bar", NULL));  */
        case '@':
          c = phase2_getc ();
          if (c != '"')
            {
              phase2_ungetc (c);
              tp->type = last_token_type = token_type_other;
              return;
            }
          template = true;
          FALLTHROUGH;
        case '"':
          {
            struct mixed_string_buffer msb;
            {
              int c2 = phase1_getc ();

              if (c2 == '"')
                {
                  int c3 = phase1_getc ();
                  if (c3 == '"')
                    verbatim = true;
                  else
                    {
                      phase1_ungetc (c3);
                      phase1_ungetc (c2);
                    }
                }
              else
                phase2_ungetc (c2);
            }

            /* Start accumulating the string.  */
            mixed_string_buffer_init (&msb, lc_string,
                                      logical_file_name, line_number);
            if (verbatim)
              for (;;)
                {
                  c = phase1_getc ();

                  /* Keep line_number in sync.  */
                  msb.line_number = line_number;

                  if (c == '"')
                    {
                      int c2 = phase1_getc ();
                      if (c2 == '"')
                        {
                          int c3 = phase1_getc ();
                          if (c3 == '"')
                            break;
                          phase1_ungetc (c3);
                        }
                      phase1_ungetc (c2);
                    }
                  if (c == EOF)
                    break;
                  mixed_string_buffer_append_char (&msb, c);
                }
            else
              for (;;)
                {
                  c = phase7_getc ();

                  /* Keep line_number in sync.  */
                  msb.line_number = line_number;

                  if (c == P7_NEWLINE)
                    {
                      error_with_progname = false;
                      error (0, 0,
                             _("%s:%d: warning: unterminated string literal"),
                             logical_file_name, line_number - 1);
                      error_with_progname = true;
                      phase7_ungetc ('\n');
                      break;
                    }
                  if (c == P7_QUOTES)
                    break;
                  if (c == P7_EOF)
                    break;
                  if (c == P7_QUOTE)
                    c = '\'';
                  if (IS_UNICODE (c))
                    {
                      assert (UNICODE_VALUE (c) >= 0
                              && UNICODE_VALUE (c) < 0x110000);
                      mixed_string_buffer_append_unicode (&msb,
                                                          UNICODE_VALUE (c));
                    }
                  else
                    mixed_string_buffer_append_char (&msb, c);
                }
            /* Done accumulating the string.  */
            if (template)
              {
                tp->type = token_type_string_template;
                mixed_string_buffer_destroy (&msb);
              }
            else
              {
                tp->type = token_type_string_literal;
                tp->mixed_string = mixed_string_buffer_result (&msb);
                tp->comment = add_reference (savable_comment);
              }
            last_token_type = tp->type;
            return;
          }

        case '/':
          switch (last_token_type)
            {
            case token_type_lparen:
            case token_type_lbrace:
            case token_type_assign:
            case token_type_return:
            case token_type_plus:
            case token_type_arithmetic_operator:
            case token_type_equality_test_operator:
            case token_type_logic_operator:
            case token_type_comma:
            case token_type_question:
            case token_type_colon:
              phase3_scan_regex ();
              tp->type = last_token_type = token_type_regex_literal;
              break;
            default:
              {
                int c2 = phase2_getc ();
                if (c2 == '=')
                  tp->type = last_token_type = token_type_assign;
                else
                  {
                    phase2_ungetc (c2);
                    tp->type = last_token_type = token_type_arithmetic_operator;
                  }
                break;
              }
            }
          return;

        case '(':
          tp->type = last_token_type = token_type_lparen;
          return;

        case ')':
          tp->type = last_token_type = token_type_rparen;
          return;

        case '{':
          tp->type = last_token_type = token_type_lbrace;
          return;

        case '}':
          tp->type = last_token_type = token_type_rbrace;
          return;

        case '+':
          {
            int c2 = phase2_getc ();
            switch (c2)
              {
              case '+':
                tp->type = last_token_type = token_type_other;
                break;
              case '=':
                tp->type = last_token_type = token_type_assign;
                break;
              default:
                phase2_ungetc (c2);
                tp->type = last_token_type = token_type_plus;
                break;
              }
            return;
          }

        case '-':
          {
            int c2 = phase2_getc ();
            switch (c2)
              {
              case '-':
                tp->type = last_token_type = token_type_other;
                break;
              case '=':
                tp->type = last_token_type = token_type_assign;
                break;
              default:
                phase2_ungetc (c2);
                tp->type = last_token_type = token_type_arithmetic_operator;
                break;
              }
            return;
          }

        case '%':
        case '^':
          {
            int c2 = phase2_getc ();
            if (c2 == '=')
	      tp->type = last_token_type = token_type_assign;
            else
              {
                phase2_ungetc (c2);
                tp->type = last_token_type = token_type_logic_operator;
              }
            return;
          }

        case '=':
          {
            int c2 = phase2_getc ();
            switch (c2)
              {
              case '=':
                tp->type = last_token_type = token_type_equality_test_operator;
                break;
              case '>':
                tp->type = last_token_type = token_type_other;
                break;
              default:
                phase2_ungetc (c2);
                tp->type = last_token_type = token_type_assign;
                break;
              }
            return;
          }

        case '!':
          {
            int c2 = phase2_getc ();
            if (c2 == '=')
              tp->type = last_token_type = token_type_equality_test_operator;
            else
              {
                phase2_ungetc (c2);
                tp->type = last_token_type = token_type_logic_operator;
              }
            return;
          }

        case '>':
        case '<':
          {
            int c2 = phase2_getc ();
            if (c2 == '=')
	      tp->type = last_token_type = token_type_equality_test_operator;
            else if (c2 == c)
              {
                int c3 = phase2_getc ();
                if (c3 == '=')
                  tp->type = last_token_type = token_type_assign;
                else
                  {
                    phase2_ungetc (c2);
                    phase2_ungetc (c3);
                    tp->type = last_token_type = token_type_other;
                  }
              }
            else
              {
                phase2_ungetc (c2);
                tp->type = last_token_type = token_type_equality_test_operator;
              }
            return;
          }

        case ',':
          tp->type = last_token_type = token_type_comma;
          return;

        case ':':
          tp->type = last_token_type = token_type_colon;
          return;

        case '&':
        case '|':
          {
            int c2 = phase2_getc ();
            if (c2 == c)
	      tp->type = last_token_type = token_type_logic_operator;
            else if (c2 == '=')
	      tp->type = last_token_type = token_type_assign;
            else
              {
                phase2_ungetc (c2);
                tp->type = last_token_type = token_type_arithmetic_operator;
              }
            return;
          }

        case '?':
          {
            int c2 = phase2_getc ();
            if (c2 == '?')
              tp->type = last_token_type = token_type_logic_operator;
            else
              {
                phase2_ungetc (c2);
                tp->type = last_token_type = token_type_question;
              }
            return;
          }

        default:
          tp->type = last_token_type = token_type_other;
          return;
        }
    }
#undef APPEND
}

static void
phase3_unget (token_ty *tp)
{
  if (tp->type != token_type_eof)
    {
      if (phase3_pushback_length == SIZEOF (phase3_pushback))
        abort ();
      phase3_pushback[phase3_pushback_length++] = *tp;
    }
}


/* String concatenation with '+'.  */

static void
x_vala_lex (token_ty *tp)
{
  phase3_get (tp);
  if (tp->type == token_type_string_literal)
    {
      mixed_string_ty *sum = tp->mixed_string;

      for (;;)
        {
          token_ty token2;

          phase3_get (&token2);
          if (token2.type == token_type_plus)
            {
              token_ty token3;

              phase3_get (&token3);
              if (token3.type == token_type_string_literal)
                {
                  sum = mixed_string_concat_free1 (sum, token3.mixed_string);

                  free_token (&token3);
                  free_token (&token2);
                  continue;
                }
              phase3_unget (&token3);
            }
          phase3_unget (&token2);
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

/* Current nesting depth.  */
static int nesting_depth;


/* The file is broken into tokens.  Scan the token stream, looking for
   a keyword, followed by a left paren, followed by a string.  When we
   see this sequence, we have something to remember.  We assume we are
   looking at a valid Vala program, and leave the complaints about the
   grammar to the compiler.

     Normal handling: Look for
       keyword ( ... msgid ... )
       keyword msgid
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
extract_balanced (message_list_ty *mlp, token_type_ty delim,
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

      x_vala_lex (&token);

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
          if (++nesting_depth > MAX_NESTING_DEPTH)
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
          nesting_depth--;
          next_context_iter = null_context_list_iterator;
          state = 0;
          break;

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

        case token_type_eof:
          arglist_parser_done (argparser, arg);
          return true;

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
                                    NULL, token.comment, false);
              }
            else
              {
                /* A string immediately after a symbol means a function call.  */
                if (state)
                  {
                    struct arglist_parser *tmp_argparser;
                    tmp_argparser = arglist_parser_alloc (mlp, next_shapes);

                    arglist_parser_remember (tmp_argparser, 1,
                                             token.mixed_string, inner_context,
                                             pos.file_name, pos.line_number,
                                             token.comment, false);
                    arglist_parser_done (tmp_argparser, 1);
                  }
                else
                  arglist_parser_remember (argparser, arg,
                                           token.mixed_string, inner_context,
                                           pos.file_name, pos.line_number,
                                           token.comment, false);
              }
          }
          drop_reference (token.comment);
          next_context_iter = null_context_list_iterator;
          state = 0;
          continue;

        case token_type_character_constant:
        case token_type_lbrace:
        case token_type_rbrace:
        case token_type_assign:
        case token_type_return:
        case token_type_plus:
        case token_type_arithmetic_operator:
        case token_type_equality_test_operator:
        case token_type_logic_operator:
        case token_type_question:
        case token_type_colon:
        case token_type_number:
        case token_type_string_template:
        case token_type_regex_literal:
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
extract_vala (FILE *f,
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

  last_comment_line = -1;
  last_non_comment_line = -1;

  phase3_pushback_length = 0;
  last_token_type = token_type_other;

  flag_context_list_table = flag_table;
  nesting_depth = 0;

  init_keywords ();

  /* Eat tokens until eof is seen.  When extract_parenthesized returns
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
