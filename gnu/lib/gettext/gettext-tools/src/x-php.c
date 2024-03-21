/* xgettext PHP backend.
   Copyright (C) 2001-2003, 2005-2010, 2014, 2018-2023 Free Software Foundation, Inc.

   This file was written by Bruno Haible <bruno@clisp.org>, 2002.

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
#include "x-php.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "attribute.h"
#include "message.h"
#include "rc-str-list.h"
#include "xgettext.h"
#include "xg-pos.h"
#include "xg-mixed-string.h"
#include "xg-arglist-context.h"
#include "xg-arglist-callshape.h"
#include "xg-arglist-parser.h"
#include "xg-message.h"
#include "error.h"
#include "error-progname.h"
#include "xalloc.h"
#include "gettext.h"

#define _(s) gettext(s)

#define SIZEOF(a) (sizeof(a) / sizeof(a[0]))


/* The PHP syntax is defined in phpdoc/manual/langref.html.
   See also php-4.1.0/Zend/zend_language_scanner.l
   and      php-4.1.0/Zend/zend_language_parser.y.
   Note that variable and function names can contain bytes in the range
   0x7f..0xff; see
     http://www.php.net/manual/en/language.variables.php
     http://www.php.net/manual/en/language.functions.php  */


/* ====================== Keyword set customization.  ====================== */

/* If true extract all strings.  */
static bool extract_all = false;

static hash_table keywords;
static bool default_keywords = true;


void
x_php_extract_all ()
{
  extract_all = true;
}


void
x_php_keyword (const char *name)
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
      x_php_keyword ("_");
      x_php_keyword ("gettext");
      x_php_keyword ("dgettext:2");
      x_php_keyword ("dcgettext:2");
      /* The following were added in PHP 4.2.0.  */
      x_php_keyword ("ngettext:1,2");
      x_php_keyword ("dngettext:2,3");
      x_php_keyword ("dcngettext:2,3");
      default_keywords = false;
    }
}

void
init_flag_table_php ()
{
  xgettext_record_flag ("_:1:pass-php-format");
  xgettext_record_flag ("gettext:1:pass-php-format");
  xgettext_record_flag ("dgettext:2:pass-php-format");
  xgettext_record_flag ("dcgettext:2:pass-php-format");
  xgettext_record_flag ("ngettext:1:pass-php-format");
  xgettext_record_flag ("ngettext:2:pass-php-format");
  xgettext_record_flag ("dngettext:2:pass-php-format");
  xgettext_record_flag ("dngettext:3:pass-php-format");
  xgettext_record_flag ("dcngettext:2:pass-php-format");
  xgettext_record_flag ("dcngettext:3:pass-php-format");
  xgettext_record_flag ("sprintf:1:php-format");
  xgettext_record_flag ("printf:1:php-format");
}


/* ======================== Reading of characters.  ======================== */

/* The input file stream.  */
static FILE *fp;


/* 1. line_number handling.  */

static unsigned char phase1_pushback[2];
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
    line_number++;

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


/* 2. Ignore HTML sections.  They are equivalent to PHP echo commands and
   therefore don't contain translatable strings.  */

static void
skip_html ()
{
  for (;;)
    {
      int c = phase1_getc ();

      if (c == EOF)
        return;

      if (c == '<')
        {
          int c2 = phase1_getc ();

          if (c2 == EOF)
            break;

          if (c2 == '?')
            {
              /* <?php is the normal way to enter PHP mode. <? and <?= are
                 recognized by PHP depending on a configuration setting.  */
              int c3 = phase1_getc ();

              if (c3 != '=')
                phase1_ungetc (c3);

              return;
            }

          if (c2 == '%')
            {
              /* <% and <%= are recognized by PHP depending on a configuration
                 setting.  */
              int c3 = phase1_getc ();

              if (c3 != '=')
                phase1_ungetc (c3);

              return;
            }

          if (c2 == '<')
            {
              phase1_ungetc (c2);
              continue;
            }

          /* < script language = php >
             < script language = "php" >
             < script language = 'php' >
             are always recognized.  */
          while (c2 == ' ' || c2 == '\t' || c2 == '\n' || c2 == '\r')
            c2 = phase1_getc ();
          if (c2 != 's' && c2 != 'S')
            {
              phase1_ungetc (c2);
              continue;
            }
          c2 = phase1_getc ();
          if (c2 != 'c' && c2 != 'C')
            {
              phase1_ungetc (c2);
              continue;
            }
          c2 = phase1_getc ();
          if (c2 != 'r' && c2 != 'R')
            {
              phase1_ungetc (c2);
              continue;
            }
          c2 = phase1_getc ();
          if (c2 != 'i' && c2 != 'I')
            {
              phase1_ungetc (c2);
              continue;
            }
          c2 = phase1_getc ();
          if (c2 != 'p' && c2 != 'P')
            {
              phase1_ungetc (c2);
              continue;
            }
          c2 = phase1_getc ();
          if (c2 != 't' && c2 != 'T')
            {
              phase1_ungetc (c2);
              continue;
            }
          c2 = phase1_getc ();
          if (!(c2 == ' ' || c2 == '\t' || c2 == '\n' || c2 == '\r'))
            {
              phase1_ungetc (c2);
              continue;
            }
          do
            c2 = phase1_getc ();
          while (c2 == ' ' || c2 == '\t' || c2 == '\n' || c2 == '\r');
          if (c2 != 'l' && c2 != 'L')
            {
              phase1_ungetc (c2);
              continue;
            }
          c2 = phase1_getc ();
          if (c2 != 'a' && c2 != 'A')
            {
              phase1_ungetc (c2);
              continue;
            }
          c2 = phase1_getc ();
          if (c2 != 'n' && c2 != 'N')
            {
              phase1_ungetc (c2);
              continue;
            }
          c2 = phase1_getc ();
          if (c2 != 'g' && c2 != 'G')
            {
              phase1_ungetc (c2);
              continue;
            }
          c2 = phase1_getc ();
          if (c2 != 'u' && c2 != 'U')
            {
              phase1_ungetc (c2);
              continue;
            }
          c2 = phase1_getc ();
          if (c2 != 'a' && c2 != 'A')
            {
              phase1_ungetc (c2);
              continue;
            }
          c2 = phase1_getc ();
          if (c2 != 'g' && c2 != 'G')
            {
              phase1_ungetc (c2);
              continue;
            }
          c2 = phase1_getc ();
          if (c2 != 'e' && c2 != 'E')
            {
              phase1_ungetc (c2);
              continue;
            }
          c2 = phase1_getc ();
          while (c2 == ' ' || c2 == '\t' || c2 == '\n' || c2 == '\r')
            c2 = phase1_getc ();
          if (c2 != '=')
            {
              phase1_ungetc (c2);
              continue;
            }
          c2 = phase1_getc ();
          while (c2 == ' ' || c2 == '\t' || c2 == '\n' || c2 == '\r')
            c2 = phase1_getc ();
          if (c2 == '"')
            {
              c2 = phase1_getc ();
              if (c2 != 'p')
                {
                  phase1_ungetc (c2);
                  continue;
                }
              c2 = phase1_getc ();
              if (c2 != 'h')
                {
                  phase1_ungetc (c2);
                  continue;
                }
              c2 = phase1_getc ();
              if (c2 != 'p')
                {
                  phase1_ungetc (c2);
                  continue;
                }
              c2 = phase1_getc ();
              if (c2 != '"')
                {
                  phase1_ungetc (c2);
                  continue;
                }
            }
          else if (c2 == '\'')
            {
              c2 = phase1_getc ();
              if (c2 != 'p')
                {
                  phase1_ungetc (c2);
                  continue;
                }
              c2 = phase1_getc ();
              if (c2 != 'h')
                {
                  phase1_ungetc (c2);
                  continue;
                }
              c2 = phase1_getc ();
              if (c2 != 'p')
                {
                  phase1_ungetc (c2);
                  continue;
                }
              c2 = phase1_getc ();
              if (c2 != '\'')
                {
                  phase1_ungetc (c2);
                  continue;
                }
            }
          else
            {
              if (c2 != 'p')
                {
                  phase1_ungetc (c2);
                  continue;
                }
              c2 = phase1_getc ();
              if (c2 != 'h')
                {
                  phase1_ungetc (c2);
                  continue;
                }
              c2 = phase1_getc ();
              if (c2 != 'p')
                {
                  phase1_ungetc (c2);
                  continue;
                }
            }
          c2 = phase1_getc ();
          while (c2 == ' ' || c2 == '\t' || c2 == '\n' || c2 == '\r')
            c2 = phase1_getc ();
          if (c2 != '>')
            {
              phase1_ungetc (c2);
              continue;
            }
          return;
        }
    }
}

#if 0

static unsigned char phase2_pushback[1];
static int phase2_pushback_length;

static int
phase2_getc ()
{
  int c;

  if (phase2_pushback_length)
    return phase2_pushback[--phase2_pushback_length];

  c = phase1_getc ();
  switch (c)
    {
    case '?':
    case '%':
      {
        int c2 = phase1_getc ();
        if (c2 == '>')
          {
            /* ?> and %> terminate PHP mode and switch back to HTML mode.  */
            skip_html ();
            return ' ';
          }
        phase1_ungetc (c2);
      }
      break;

    case '<':
      {
        int c2 = phase1_getc ();

        /* < / script > terminates PHP mode and switches back to HTML mode.  */
        while (c2 == ' ' || c2 == '\t' || c2 == '\n' || c2 == '\r')
          c2 = phase1_getc ();
        if (c2 == '/')
          {
            do
              c2 = phase1_getc ();
            while (c2 == ' ' || c2 == '\t' || c2 == '\n' || c2 == '\r');
            if (c2 == 's' || c2 == 'S')
              {
                c2 = phase1_getc ();
                if (c2 == 'c' || c2 == 'C')
                  {
                    c2 = phase1_getc ();
                    if (c2 == 'r' || c2 == 'R')
                      {
                        c2 = phase1_getc ();
                        if (c2 == 'i' || c2 == 'I')
                          {
                            c2 = phase1_getc ();
                            if (c2 == 'p' || c2 == 'P')
                              {
                                c2 = phase1_getc ();
                                if (c2 == 't' || c2 == 'T')
                                  {
                                    do
                                      c2 = phase1_getc ();
                                    while (c2 == ' ' || c2 == '\t'
                                           || c2 == '\n' || c2 == '\r');
                                    if (c2 == '>')
                                      {
                                        skip_html ();
                                        return ' ';
                                      }
                                  }
                              }
                          }
                      }
                  }
              }
          }
        phase1_ungetc (c2);
      }
      break;
    }

  return c;
}

static void
phase2_ungetc (int c)
{
  if (c != EOF)
    {
      if (phase2_pushback_length == SIZEOF (phase2_pushback))
        abort ();
      phase2_pushback[phase2_pushback_length++] = c;
    }
}

#endif


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


/* 3. Replace each comment that is not inside a string literal with a
   space character.  We need to remember the comment for later, because
   it may be attached to a keyword string.  */

/* These are for tracking whether comments count as immediately before
   keyword.  */
static int last_comment_line;
static int last_non_comment_line;

static unsigned char phase3_pushback[1];
static int phase3_pushback_length;

static int
phase3_getc ()
{
  int lineno;
  int c;

  if (phase3_pushback_length)
    return phase3_pushback[--phase3_pushback_length];

  c = phase1_getc ();

  if (c == '#')
    {
      /* sh comment.  */
      bool last_was_qmark = false;

      comment_start ();
      lineno = line_number;
      for (;;)
        {
          c = phase1_getc ();
          if (c == '\n' || c == EOF)
            {
              comment_line_end (0);
              break;
            }
          if (last_was_qmark && c == '>')
            {
              comment_line_end (1);
              skip_html ();
              break;
            }
          /* We skip all leading white space, but not EOLs.  */
          if (!(buflen == 0 && (c == ' ' || c == '\t')))
            comment_add (c);
          last_was_qmark = (c == '?' || c == '%');
        }
      last_comment_line = lineno;
      return '\n';
    }
  else if (c == '/')
    {
      c = phase1_getc ();

      switch (c)
        {
        default:
          phase1_ungetc (c);
          return '/';

        case '*':
          {
            /* C comment.  */
            bool last_was_star;

            comment_start ();
            lineno = line_number;
            last_was_star = false;
            for (;;)
              {
                c = phase1_getc ();
                if (c == EOF)
                  break;
                /* We skip all leading white space, but not EOLs.  */
                if (buflen == 0 && (c == ' ' || c == '\t'))
                  continue;
                comment_add (c);
                switch (c)
                  {
                  case '\n':
                    comment_line_end (1);
                    comment_start ();
                    lineno = line_number;
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
            last_comment_line = lineno;
            return ' ';
          }

        case '/':
          {
            /* C++ comment.  */
            bool last_was_qmark = false;

            comment_start ();
            lineno = line_number;
            for (;;)
              {
                c = phase1_getc ();
                if (c == '\n' || c == EOF)
                  {
                    comment_line_end (0);
                    break;
                  }
                if (last_was_qmark && c == '>')
                  {
                    comment_line_end (1);
                    skip_html ();
                    break;
                  }
                /* We skip all leading white space, but not EOLs.  */
                if (!(buflen == 0 && (c == ' ' || c == '\t')))
                  comment_add (c);
                last_was_qmark = (c == '?' || c == '%');
              }
            last_comment_line = lineno;
            return '\n';
          }
        }
    }
  else
    return c;
}

#ifdef unused
static void
phase3_ungetc (int c)
{
  if (c != EOF)
    {
      if (phase3_pushback_length == SIZEOF (phase3_pushback))
        abort ();
      phase3_pushback[phase3_pushback_length++] = c;
    }
}
#endif


/* ========================== Reading of tokens.  ========================== */


enum token_type_ty
{
  token_type_eof,
  token_type_lparen,            /* ( */
  token_type_rparen,            /* ) */
  token_type_comma,             /* , */
  token_type_lbracket,          /* [ */
  token_type_rbracket,          /* ] */
  token_type_dot,               /* . */
  token_type_operator1,         /* * / % ++ -- */
  token_type_operator2,         /* + - ! ~ @ */
  token_type_string_literal,    /* "abc" */
  token_type_symbol,            /* symbol, number */
  token_type_other              /* misc. operator */
};
typedef enum token_type_ty token_type_ty;

typedef struct token_ty token_ty;
struct token_ty
{
  token_type_ty type;
  char *string;         /* for token_type_string_literal, token_type_symbol */
  refcounted_string_list_ty *comment;   /* for token_type_string_literal */
  int line_number;
};


/* Free the memory pointed to by a 'struct token_ty'.  */
static inline void
free_token (token_ty *tp)
{
  if (tp->type == token_type_string_literal || tp->type == token_type_symbol)
    free (tp->string);
  if (tp->type == token_type_string_literal)
    drop_reference (tp->comment);
}


/* 4. Combine characters into tokens.  Discard whitespace.  */

static token_ty phase4_pushback[3];
static int phase4_pushback_length;

static void
phase4_get (token_ty *tp)
{
  static char *buffer;
  static int bufmax;
  int bufpos;
  int c;

  if (phase4_pushback_length)
    {
      *tp = phase4_pushback[--phase4_pushback_length];
      return;
    }
  tp->string = NULL;

  for (;;)
    {
      tp->line_number = line_number;
      c = phase3_getc ();
      switch (c)
        {
        case EOF:
          tp->type = token_type_eof;
          return;

        case '\n':
          if (last_non_comment_line > last_comment_line)
            savable_comment_reset ();
          FALLTHROUGH;
        case ' ':
        case '\t':
        case '\r':
          /* Ignore whitespace.  */
          continue;
        }

      last_non_comment_line = tp->line_number;

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
        case 127: case 128: case 129: case 130: case 131: case 132: case 133:
        case 134: case 135: case 136: case 137: case 138: case 139: case 140:
        case 141: case 142: case 143: case 144: case 145: case 146: case 147:
        case 148: case 149: case 150: case 151: case 152: case 153: case 154:
        case 155: case 156: case 157: case 158: case 159: case 160: case 161:
        case 162: case 163: case 164: case 165: case 166: case 167: case 168:
        case 169: case 170: case 171: case 172: case 173: case 174: case 175:
        case 176: case 177: case 178: case 179: case 180: case 181: case 182:
        case 183: case 184: case 185: case 186: case 187: case 188: case 189:
        case 190: case 191: case 192: case 193: case 194: case 195: case 196:
        case 197: case 198: case 199: case 200: case 201: case 202: case 203:
        case 204: case 205: case 206: case 207: case 208: case 209: case 210:
        case 211: case 212: case 213: case 214: case 215: case 216: case 217:
        case 218: case 219: case 220: case 221: case 222: case 223: case 224:
        case 225: case 226: case 227: case 228: case 229: case 230: case 231:
        case 232: case 233: case 234: case 235: case 236: case 237: case 238:
        case 239: case 240: case 241: case 242: case 243: case 244: case 245:
        case 246: case 247: case 248: case 249: case 250: case 251: case 252:
        case 253: case 254: case 255:
          bufpos = 0;
          for (;;)
            {
              if (bufpos >= bufmax)
                {
                  bufmax = 2 * bufmax + 10;
                  buffer = xrealloc (buffer, bufmax);
                }
              buffer[bufpos++] = c;
              c = phase1_getc ();
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
                case 127: case 128: case 129: case 130: case 131: case 132:
                case 133: case 134: case 135: case 136: case 137: case 138:
                case 139: case 140: case 141: case 142: case 143: case 144:
                case 145: case 146: case 147: case 148: case 149: case 150:
                case 151: case 152: case 153: case 154: case 155: case 156:
                case 157: case 158: case 159: case 160: case 161: case 162:
                case 163: case 164: case 165: case 166: case 167: case 168:
                case 169: case 170: case 171: case 172: case 173: case 174:
                case 175: case 176: case 177: case 178: case 179: case 180:
                case 181: case 182: case 183: case 184: case 185: case 186:
                case 187: case 188: case 189: case 190: case 191: case 192:
                case 193: case 194: case 195: case 196: case 197: case 198:
                case 199: case 200: case 201: case 202: case 203: case 204:
                case 205: case 206: case 207: case 208: case 209: case 210:
                case 211: case 212: case 213: case 214: case 215: case 216:
                case 217: case 218: case 219: case 220: case 221: case 222:
                case 223: case 224: case 225: case 226: case 227: case 228:
                case 229: case 230: case 231: case 232: case 233: case 234:
                case 235: case 236: case 237: case 238: case 239: case 240:
                case 241: case 242: case 243: case 244: case 245: case 246:
                case 247: case 248: case 249: case 250: case 251: case 252:
                case 253: case 254: case 255:
                  continue;

                default:
                  phase1_ungetc (c);
                  break;
                }
              break;
            }
          if (bufpos >= bufmax)
            {
              bufmax = 2 * bufmax + 10;
              buffer = xrealloc (buffer, bufmax);
            }
          buffer[bufpos] = 0;
          tp->string = xstrdup (buffer);
          tp->type = token_type_symbol;
          return;

        case '\'':
          /* Single-quoted string literal.  */
          bufpos = 0;
          for (;;)
            {
              c = phase1_getc ();
              if (c == EOF || c == '\'')
                break;
              if (c == '\\')
                {
                  c = phase1_getc ();
                  if (c != '\\' && c != '\'')
                    {
                      phase1_ungetc (c);
                      c = '\\';
                    }
                }
              if (bufpos >= bufmax)
                {
                  bufmax = 2 * bufmax + 10;
                  buffer = xrealloc (buffer, bufmax);
                }
              buffer[bufpos++] = c;
            }
          if (bufpos >= bufmax)
            {
              bufmax = 2 * bufmax + 10;
              buffer = xrealloc (buffer, bufmax);
            }
          buffer[bufpos] = 0;
          tp->type = token_type_string_literal;
          tp->string = xstrdup (buffer);
          tp->comment = add_reference (savable_comment);
          return;

        case '"':
          /* Double-quoted string literal.  */
          tp->type = token_type_string_literal;
          bufpos = 0;
          for (;;)
            {
              c = phase1_getc ();
              if (c == EOF || c == '"')
                break;
              if (c == '$')
                {
                  c = phase1_getc ();
                  if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')
                      || c == '_' || c == '{' || c >= 0x7f)
                    {
                      /* String with variables.  */
                      tp->type = token_type_other;
                      continue;
                    }
                  phase1_ungetc (c);
                  c = '$';
                }
              if (c == '{')
                {
                  c = phase1_getc ();
                  if (c == '$')
                    {
                      /* String with expressions.  */
                      tp->type = token_type_other;
                      continue;
                    }
                  phase1_ungetc (c);
                  c = '{';
                }
              if (c == '\\')
                {
                  int n, j;

                  c = phase1_getc ();
                  switch (c)
                    {
                    case '"':
                    case '\\':
                    case '$':
                      break;

                    case '0': case '1': case '2': case '3':
                    case '4': case '5': case '6': case '7':
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
                      c = n;
                      break;

                    case 'x':
                      n = 0;
                      for (j = 0; j < 2; ++j)
                        {
                          c = phase1_getc ();
                          switch (c)
                            {
                            case '0': case '1': case '2': case '3': case '4':
                            case '5': case '6': case '7': case '8': case '9':
                              n = n * 16 + c - '0';
                              break;
                            case 'A': case 'B': case 'C': case 'D': case 'E':
                            case 'F':
                              n = n * 16 + 10 + c - 'A';
                              break;
                            case 'a': case 'b': case 'c': case 'd': case 'e':
                            case 'f':
                              n = n * 16 + 10 + c - 'a';
                              break;
                            default:
                              phase1_ungetc (c);
                              c = 0;
                              break;
                            }
                          if (c == 0)
                            break;
                        }
                      if (j == 0)
                        {
                          phase1_ungetc ('x');
                          c = '\\';
                        }
                      else
                        c = n;
                      break;

                    case 'n':
                      c = '\n';
                      break;
                    case 't':
                      c = '\t';
                      break;
                    case 'r':
                      c = '\r';
                      break;

                    default:
                      phase1_ungetc (c);
                      c = '\\';
                      break;
                    }
                }
              if (bufpos >= bufmax)
                {
                  bufmax = 2 * bufmax + 10;
                  buffer = xrealloc (buffer, bufmax);
                }
              buffer[bufpos++] = c;
            }
          if (bufpos >= bufmax)
            {
              bufmax = 2 * bufmax + 10;
              buffer = xrealloc (buffer, bufmax);
            }
          buffer[bufpos] = 0;
          if (tp->type == token_type_string_literal)
            {
              tp->string = xstrdup (buffer);
              tp->comment = add_reference (savable_comment);
            }
          return;

        case '?':
        case '%':
          {
            int c2 = phase1_getc ();
            if (c2 == '>')
              {
                /* ?> and %> terminate PHP mode and switch back to HTML
                   mode.  */
                skip_html ();
                tp->type = token_type_other;
              }
            else
              {
                phase1_ungetc (c2);
                tp->type = (c == '%' ? token_type_operator1 : token_type_other);
              }
            return;
          }

        case '(':
          tp->type = token_type_lparen;
          return;

        case ')':
          tp->type = token_type_rparen;
          return;

        case ',':
          tp->type = token_type_comma;
          return;

        case '[':
          tp->type = token_type_lbracket;
          return;

        case ']':
          tp->type = token_type_rbracket;
          return;

        case '.':
          tp->type = token_type_dot;
          return;

        case '*':
        case '/':
          tp->type = token_type_operator1;
          return;

        case '+':
        case '-':
          {
            int c2 = phase1_getc ();
            if (c2 == c)
              /* ++ or -- */
              tp->type = token_type_operator1;
            else
              /* + or - */
              {
                phase1_ungetc (c2);
                tp->type = token_type_operator2;
              }
            return;
          }

        case '!':
        case '~':
        case '@':
          tp->type = token_type_operator2;
          return;

        case '<':
          {
            int c2 = phase1_getc ();
            if (c2 == '<')
              {
                int c3 = phase1_getc ();
                if (c3 == '<')
                  {
                    int label_start = 0;

                    /* Start of here and now document.
                       Parse whitespace, then label, then newline.  */
                    do
                      c = phase3_getc ();
                    while (c == ' ' || c == '\t' || c == '\n' || c == '\r');

                    bufpos = 0;
                    do
                      {
                        if (bufpos >= bufmax)
                          {
                            bufmax = 2 * bufmax + 10;
                            buffer = xrealloc (buffer, bufmax);
                          }
                        buffer[bufpos++] = c;
                        c = phase3_getc ();
                      }
                    while (c != EOF && c != '\n' && c != '\r');
                    /* buffer[0..bufpos-1] now contains the label
                       (including single or double quotes).  */

                    if (*buffer == '\'' || *buffer == '"')
                      {
                        label_start++;
                        bufpos--;
                      }

                    /* Now skip the here document.  */
                    for (;;)
                      {
                        c = phase1_getc ();
                        if (c == EOF)
                          break;
                        if (c == '\n' || c == '\r')
                          {
                            int bufidx = label_start;

                            while (bufidx < bufpos)
                              {
                                c = phase1_getc ();
                                if (c == EOF)
                                  break;
                                if (c != buffer[bufidx])
                                  {
                                    phase1_ungetc (c);
                                    break;
                                  }
                                bufidx++;
                              }
                            if (bufidx == bufpos)
                              {
                                c = phase1_getc ();
                                if (c != ';')
                                  phase1_ungetc (c);
                                c = phase1_getc ();
                                if (c == '\n' || c == '\r')
                                  break;
                              }
                          }
                      }

                    /* FIXME: Ideally we should turn the here document into a
                       string literal if it didn't contain $ substitution.  And
                       we should also respect backslash escape sequences like
                       in double-quoted strings.  */
                    tp->type = token_type_other;
                    return;
                  }
                phase1_ungetc (c3);
              }

            /* < / script > terminates PHP mode and switches back to HTML
               mode.  */
            while (c2 == ' ' || c2 == '\t' || c2 == '\n' || c2 == '\r')
              c2 = phase1_getc ();
            if (c2 == '/')
              {
                do
                  c2 = phase1_getc ();
                while (c2 == ' ' || c2 == '\t' || c2 == '\n' || c2 == '\r');
                if (c2 == 's' || c2 == 'S')
                  {
                    c2 = phase1_getc ();
                    if (c2 == 'c' || c2 == 'C')
                      {
                        c2 = phase1_getc ();
                        if (c2 == 'r' || c2 == 'R')
                          {
                            c2 = phase1_getc ();
                            if (c2 == 'i' || c2 == 'I')
                              {
                                c2 = phase1_getc ();
                                if (c2 == 'p' || c2 == 'P')
                                  {
                                    c2 = phase1_getc ();
                                    if (c2 == 't' || c2 == 'T')
                                      {
                                        do
                                          c2 = phase1_getc ();
                                        while (c2 == ' ' || c2 == '\t'
                                               || c2 == '\n' || c2 == '\r');
                                        if (c2 == '>')
                                          {
                                            skip_html ();
                                          }
                                        else
                                          phase1_ungetc (c2);
                                      }
                                    else
                                      phase1_ungetc (c2);
                                  }
                                else
                                  phase1_ungetc (c2);
                              }
                            else
                              phase1_ungetc (c2);
                          }
                        else
                          phase1_ungetc (c2);
                      }
                    else
                      phase1_ungetc (c2);
                  }
                else
                  phase1_ungetc (c2);
              }
            else
              phase1_ungetc (c2);

            tp->type = token_type_other;
            return;
          }

        case '`':
          /* Execution operator.  */
        default:
          /* We could carefully recognize each of the 2 and 3 character
             operators, but it is not necessary, as we only need to recognize
             gettext invocations.  Don't bother.  */
          tp->type = token_type_other;
          return;
        }
    }
}

/* Supports 3 tokens of pushback.  */
static void
phase4_unget (token_ty *tp)
{
  if (tp->type != token_type_eof)
    {
      if (phase4_pushback_length == SIZEOF (phase4_pushback))
        abort ();
      phase4_pushback[phase4_pushback_length++] = *tp;
    }
}


/* 5. Compile-time optimization of string literal concatenation.
   Combine "string1" . ... . "stringN" to the concatenated string if
     - the token before this expression is none of
       '+' '-' '.' '*' '/' '%' '!' '~' '++' '--' ')' '@'
       (because then the first string could be part of an expression with
       the same or higher precedence as '.', such as an additive,
       multiplicative, negation, preincrement, or cast expression),
     - the token after this expression is none of
       '*' '/' '%' '++' '--'
       (because then the last string could be part of an expression with
       higher precedence as '.', such as a multiplicative or postincrement
       expression).  */

static token_type_ty phase5_last;

static void
x_php_lex (token_ty *tp)
{
  phase4_get (tp);
  if (tp->type == token_type_string_literal
      && !(phase5_last == token_type_dot
           || phase5_last == token_type_operator1
           || phase5_last == token_type_operator2
           || phase5_last == token_type_rparen))
    {
      char *sum = tp->string;
      size_t sum_len = strlen (sum);

      for (;;)
        {
          token_ty token2;

          phase4_get (&token2);
          if (token2.type == token_type_dot)
            {
              token_ty token3;

              phase4_get (&token3);
              if (token3.type == token_type_string_literal)
                {
                  token_ty token_after;

                  phase4_get (&token_after);
                  if (token_after.type != token_type_operator1)
                    {
                      char *addend = token3.string;
                      size_t addend_len = strlen (addend);

                      sum = (char *) xrealloc (sum, sum_len + addend_len + 1);
                      memcpy (sum + sum_len, addend, addend_len + 1);
                      sum_len += addend_len;

                      phase4_unget (&token_after);
                      free_token (&token3);
                      free_token (&token2);
                      continue;
                    }
                  phase4_unget (&token_after);
                }
              phase4_unget (&token3);
            }
          phase4_unget (&token2);
          break;
        }
      tp->string = sum;
    }
  phase5_last = tp->type;
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

      x_php_lex (&token);
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

        case token_type_string_literal:
          {
            lex_pos_ty pos;
            pos.file_name = logical_file_name;
            pos.line_number = token.line_number;

            if (extract_all)
              remember_a_message (mlp, NULL, token.string, false, false,
                                  inner_context, &pos,
                                  NULL, token.comment, false);
            else
              {
                mixed_string_ty *ms =
                  mixed_string_alloc_simple (token.string, lc_string,
                                             pos.file_name, pos.line_number);
                free (token.string);
                arglist_parser_remember (argparser, arg, ms, inner_context,
                                         pos.file_name, pos.line_number,
                                         token.comment, false);
              }
            drop_reference (token.comment);
          }
          next_context_iter = null_context_list_iterator;
          state = 0;
          continue;

        case token_type_dot:
        case token_type_operator1:
        case token_type_operator2:
        case token_type_other:
          next_context_iter = null_context_list_iterator;
          state = 0;
          continue;

        case token_type_eof:
          arglist_parser_done (argparser, arg);
          return true;

        default:
          abort ();
        }
    }
}


void
extract_php (FILE *f,
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
#if 0
  phase2_pushback_length = 0;
#endif

  last_comment_line = -1;
  last_non_comment_line = -1;

  phase3_pushback_length = 0;
  phase4_pushback_length = 0;

  phase5_last = token_type_eof;

  flag_context_list_table = flag_table;
  paren_nesting_depth = 0;
  bracket_nesting_depth = 0;

  init_keywords ();

  /* Initial mode is HTML mode, not PHP mode.  */
  skip_html ();

  /* Eat tokens until eof is seen.  When extract_balanced returns
     due to an unbalanced closing parenthesis, just restart it.  */
  while (!extract_balanced (mlp, token_type_eof,
                            null_context, null_context_list_iterator,
                            arglist_parser_alloc (mlp, NULL)))
    ;

  /* Close scanner.  */
  fp = NULL;
  real_file_name = NULL;
  logical_file_name = NULL;
  line_number = 0;
}
