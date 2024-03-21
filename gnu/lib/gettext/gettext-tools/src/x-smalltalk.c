/* xgettext Smalltalk backend.
   Copyright (C) 2002-2003, 2005-2009, 2011, 2018-2020 Free Software Foundation, Inc.

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
#include "x-smalltalk.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "attribute.h"
#include "message.h"
#include "xgettext.h"
#include "xg-pos.h"
#include "xg-message.h"
#include "error.h"
#include "xalloc.h"
#include "gettext.h"

#define _(s) gettext(s)

#define SIZEOF(a) (sizeof(a) / sizeof(a[0]))


/* The relevant parts of the Smalltalk syntax are:

     stringliteral ::= string | stringconst | symconst
     stringconst ::= "#"string
     string      ::= "'"[char]*"'"
     symconst    ::= "#"symbol
     symbol      ::= id | binsel | keysel[keysel]*
     keysel      ::= id":"
     id          ::= letter[letter|digit]*
     letter      ::= "A".."Z" | "a".."z"
     digit       ::= "0".."9"
     binsel      ::= selchar[selchar]
     selchar     ::= "+" | "-" | "*" | "/" | "~" | "|" | "," | "<" | ">"
                     | "=" | "&" | "@" | "?" | "%" | "\"

   Strings can contain any characters; to include the string delimiter itself,
   it must be duplicated.

   Character constants are written  "$"char

   Comments are enclosed within double quotes.

   In well-formed expressions, {} and [] and () are balanced.
 */


/* ======================== Reading of characters.  ======================== */

/* The input file stream.  */
static FILE *fp;


/* 1. line_number handling.  */

static int
phase1_getc ()
{
  int c = getc (fp);

  if (c == EOF)
    {
      if (ferror (fp))
        error (EXIT_FAILURE, errno, _("error while reading \"%s\""),
               real_file_name);
      return EOF;
    }

  if (c == '\n')
    line_number++;

  return c;
}

/* Supports only one pushback character.  */
static void
phase1_ungetc (int c)
{
  if (c != EOF)
    {
      if (c == '\n')
        --line_number;

      ungetc (c, fp);
    }
}


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
comment_line_end ()
{
  while (buflen >= 1
         && (buffer[buflen - 1] == ' ' || buffer[buflen - 1] == '\t'))
    --buflen;
  if (buflen >= bufmax)
    {
      bufmax = 2 * bufmax + 10;
      buffer = xrealloc (buffer, bufmax);
    }
  buffer[buflen] = '\0';
  savable_comment_add (buffer);
}


/* These are for tracking whether comments count as immediately before
   keyword.  */
static int last_comment_line;
static int last_non_comment_line;


/* ========================== Reading of tokens.  ========================== */


enum token_type_ty
{
  token_type_eof,
  token_type_uniq,              /* # */
  token_type_symbol,            /* symbol */
  token_type_string_literal,    /* string, stringconst, symbolconst */
  token_type_other              /* misc. operator */
};
typedef enum token_type_ty token_type_ty;

typedef struct token_ty token_ty;
struct token_ty
{
  token_type_ty type;
  char *string;         /* for token_type_string_literal, token_type_symbol */
  int line_number;
};


/* 2. Combine characters into tokens.  Discard comments and whitespace.  */

static token_ty phase2_pushback[1];
static int phase2_pushback_length;

static void
phase2_get (token_ty *tp)
{
  static char *buffer;
  static int bufmax;
  int bufpos;
  int c;

  if (phase2_pushback_length)
    {
      *tp = phase2_pushback[--phase2_pushback_length];
      return;
    }

  tp->string = NULL;

  for (;;)
    {
      tp->line_number = line_number;
      c = phase1_getc ();
      switch (c)
        {
        case EOF:
          tp->type = token_type_eof;
          return;

        case '"':
          {
            /* Comment.  */
            int lineno;

            comment_start ();
            lineno = line_number;
            for (;;)
              {
                c = phase1_getc ();
                if (c == '"' || c == EOF)
                  break;
                if (c == '\n')
                  {
                    comment_line_end ();
                    comment_start ();
                  }
                else
                  {
                    /* We skip all leading white space, but not EOLs.  */
                    if (!(buflen == 0 && (c == ' ' || c == '\t')))
                      comment_add (c);
                  }
              }
            comment_line_end ();
            last_comment_line = lineno;
            continue;
          }

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
        case '\'':
          /* String literal.  */
          bufpos = 0;
          for (;;)
            {
              c = phase1_getc ();
              if (c == EOF)
                break;
              if (c == '\'')
                {
                  c = phase1_getc ();
                  if (c != '\'')
                    {
                      phase1_ungetc (c);
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
          tp->type = token_type_string_literal;
          tp->string = xstrdup (buffer);
          return;

        case '+':
        case '-':
        case '*':
        case '/':
        case '~':
        case '|':
        case ',':
        case '<':
        case '>':
        case '=':
        case '&':
        case '@':
        case '?':
        case '%':
        case '\\':
          {
            char *name;
            int c2 = phase1_getc ();
            switch (c2)
              {
              case '+':
              case '-':
              case '*':
              case '/':
              case '~':
              case '|':
              case ',':
              case '<':
              case '>':
              case '=':
              case '&':
              case '@':
              case '?':
              case '%':
                name = XNMALLOC (3, char);
                name[0] = c;
                name[1] = c2;
                name[2] = '\0';
                tp->type = token_type_symbol;
                tp->string = name;
                return;
              default:
                phase1_ungetc (c2);
                break;
              }
            name = XNMALLOC (2, char);
            name[0] = c;
            name[1] = '\0';
            tp->type = token_type_symbol;
            tp->string = name;
            return;
          }

        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
        case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
        case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
        case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
        case 'Y': case 'Z':
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
        case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
        case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
        case 's': case 't': case 'u': case 'v': case 'w': case 'x':
        case 'y': case 'z':
          /* Recognize id or id":"[id":"]* or id":"[id":"]*id.  */
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
                case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
                case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
                case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
                case 's': case 't': case 'u': case 'v': case 'w': case 'x':
                case 'y': case 'z':
                case '0': case '1': case '2': case '3': case '4':
                case '5': case '6': case '7': case '8': case '9':
                  continue;
                case ':':
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
                    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
                    case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
                    case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
                    case 's': case 't': case 'u': case 'v': case 'w': case 'x':
                    case 'y': case 'z':
                      continue;
                    default:
                      phase1_ungetc (c);
                      break;
                    }
                  break;
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
          buffer[bufpos] = '\0';
          tp->string = xstrdup (buffer);
          tp->type = token_type_symbol;
          return;

        case '#':
          /* Uniquification operator.  */
          tp->type = token_type_uniq;
          return;

        case '$':
          c = phase1_getc ();
          tp->type = token_type_other;
          return;

        default:
          tp->type = token_type_other;
          return;
        }
    }
}

/* Supports only one pushback token.  */
static void
phase2_unget (token_ty *tp)
{
  if (tp->type != token_type_eof)
    {
      if (phase2_pushback_length == SIZEOF (phase2_pushback))
        abort ();
      phase2_pushback[phase2_pushback_length++] = *tp;
    }
}


/* 3. Combine "# string_literal" and "# symbol" to a single token.  */

static token_ty phase3_pushback[1];
static int phase3_pushback_length;

static void
phase3_get (token_ty *tp)
{
  if (phase3_pushback_length)
    {
      *tp = phase3_pushback[--phase3_pushback_length];
      return;
    }

  phase2_get (tp);
  if (tp->type == token_type_uniq)
    {
      token_ty token2;

      phase2_get (&token2);
      if (token2.type == token_type_symbol
          || token2.type == token_type_string_literal)
        {
          tp->type = token_type_string_literal;
          tp->string = token2.string;
        }
      else
        phase2_unget (&token2);
    }
}

/* Supports only one pushback token.  */
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


/* ========================= Extracting strings.  ========================== */

/* The file is broken into tokens.  Scan the token stream, looking for the
   following patterns
      NLS ? <string>
      NLS at: <string>
      NLS at: <string> plural: <string>
   where <string> is one of
      string_literal
      # string_literal
      # symbol
 */

void
extract_smalltalk (FILE *f,
                   const char *real_filename, const char *logical_filename,
                   flag_context_list_table_ty *flag_table,
                   msgdomain_list_ty *mdlp)
{
  message_list_ty *mlp = mdlp->item[0]->messages;

  fp = f;
  real_file_name = real_filename;
  logical_file_name = xstrdup (logical_filename);
  line_number = 1;

  last_comment_line = -1;
  last_non_comment_line = -1;

  phase2_pushback_length = 0;
  phase3_pushback_length = 0;

  /* Eat tokens until eof is seen.  */
  {
    /* 0 when no "NLS" has been seen.
       1 after "NLS".
       2 after "NLS ?".
       3 after "NLS at:".
       4 after "NLS at: <string>".
       5 after "NLS at: <string> plural:".  */
    int state;
    /* Remember the message containing the msgid, for msgid_plural.
       Non-NULL in states 4, 5.  */
    message_ty *plural_mp = NULL;

    /* Start state is 0.  */
    state = 0;

    for (;;)
      {
        token_ty token;

        phase3_get (&token);

        switch (token.type)
          {
          case token_type_symbol:
            state = (strcmp (token.string, "NLS") == 0 ? 1 :
                     strcmp (token.string, "?") == 0 && state == 1 ? 2 :
                     strcmp (token.string, "at:") == 0 && state == 1 ? 3 :
                     strcmp (token.string, "plural:") == 0 && state == 4 ? 5 :
                     0);
            free (token.string);
            break;

          case token_type_string_literal:
            if (state == 2)
              {
                lex_pos_ty pos;
                pos.file_name = logical_file_name;
                pos.line_number = token.line_number;
                remember_a_message (mlp, NULL, token.string, false, false,
                                    null_context, &pos, NULL, savable_comment,
                                    false);
                state = 0;
                break;
              }
            if (state == 3)
              {
                lex_pos_ty pos;
                token_ty token2;

                pos.file_name = logical_file_name;
                pos.line_number = token.line_number;

                phase3_get (&token2);

                plural_mp =
                  remember_a_message (mlp, NULL, token.string, false,
                                      token2.type == token_type_symbol
                                      && strcmp (token.string, "plural:") == 0,
                                      null_context, &pos,
                                      NULL, savable_comment, false);

                phase3_unget (&token2);

                state = 4;
                break;
              }
            if (state == 5)
              {
                lex_pos_ty pos;
                pos.file_name = logical_file_name;
                pos.line_number = token.line_number;
                if (plural_mp != NULL)
                  remember_a_message_plural (plural_mp, token.string, false,
                                             null_context, &pos,
                                             savable_comment, false);
                state = 0;
                break;
              }
            state = 0;
            free (token.string);
            break;

          case token_type_uniq:
          case token_type_other:
            state = 0;
            break;

          case token_type_eof:
            break;

          default:
            abort ();
          }

        if (token.type == token_type_eof)
          break;
      }
  }

  /* Close scanner.  */
  fp = NULL;
  real_file_name = NULL;
  logical_file_name = NULL;
  line_number = 0;
}
