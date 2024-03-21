/* xgettext YCP backend.
   Copyright (C) 2001-2003, 2005-2009, 2011, 2018-2023 Free Software Foundation, Inc.

   This file was written by Bruno Haible <haible@clisp.cons.org>, 2001.

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
#include "x-ycp.h"

#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "attribute.h"
#include "message.h"
#include "rc-str-list.h"
#include "xgettext.h"
#include "xg-pos.h"
#include "xg-arglist-context.h"
#include "xg-message.h"
#include "error.h"
#include "error-progname.h"
#include "xalloc.h"
#include "gettext.h"

#define _(s) gettext(s)

#define SIZEOF(a) (sizeof(a) / sizeof(a[0]))


/* The YCP syntax is defined in libycp/doc/syntax.html.
   See also libycp/src/scanner.ll.
   Both are part of the yast2-core package in SuSE Linux distributions.  */


void
init_flag_table_ycp ()
{
  xgettext_record_flag ("sformat:1:ycp-format");
  xgettext_record_flag ("y2debug:1:ycp-format");
  xgettext_record_flag ("y2milestone:1:ycp-format");
  xgettext_record_flag ("y2warning:1:ycp-format");
  xgettext_record_flag ("y2error:1:ycp-format");
  xgettext_record_flag ("y2security:1:ycp-format");
  xgettext_record_flag ("y2internal:1:ycp-format");
}


/* ======================== Reading of characters.  ======================== */

/* Position in the current line.  */
static int char_in_line;

/* The input file stream.  */
static FILE *fp;

/* These are for tracking whether comments count as immediately before
   keyword.  */
static int last_comment_line;
static int last_non_comment_line;


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
    {
      line_number++;
      char_in_line = 0;
    }
  else
    char_in_line++;

  return c;
}

/* Supports only one pushback character.  */
static void
phase1_ungetc (int c)
{
  if (c != EOF)
    {
      if (c == '\n')
        {
          --line_number;
          char_in_line = INT_MAX;
        }
      else
        --char_in_line;

      ungetc (c, fp);
    }
}


/* 2. Replace each comment that is not inside a character constant or
   string literal with a space character.  We need to remember the
   comment for later, because it may be attached to a keyword string.
   YCP comments can be in C comment syntax, C++ comment syntax or sh
   comment syntax.  */

static unsigned char phase2_pushback[1];
static int phase2_pushback_length;

static int
phase2_getc ()
{
  static char *buffer;
  static size_t bufmax;
  size_t buflen;
  int lineno;
  int c;
  bool last_was_star;

  if (phase2_pushback_length)
    return phase2_pushback[--phase2_pushback_length];

  if (char_in_line == 0)
    {
      /* Eat whitespace, to recognize ^[\t ]*# pattern.  */
      do
        c = phase1_getc ();
      while (c == '\t' || c == ' ');

      if (c == '#')
        {
          /* sh comment.  */
          buflen = 0;
          lineno = line_number;
          for (;;)
            {
              c = phase1_getc ();
              if (c == '\n' || c == EOF)
                break;
              /* We skip all leading white space, but not EOLs.  */
              if (!(buflen == 0 && (c == ' ' || c == '\t')))
                {
                  if (buflen >= bufmax)
                    {
                      bufmax = 2 * bufmax + 10;
                      buffer = xrealloc (buffer, bufmax);
                    }
                  buffer[buflen++] = c;
                }
            }
          if (buflen >= bufmax)
            {
              bufmax = 2 * bufmax + 10;
              buffer = xrealloc (buffer, bufmax);
            }
          buffer[buflen] = '\0';
          savable_comment_add (buffer);
          last_comment_line = lineno;
          return '\n';
        }
    }
  else
    c = phase1_getc ();

  if (c == '/')
    {
      c = phase1_getc ();

      switch (c)
        {
        default:
          phase1_ungetc (c);
          return '/';

        case '*':
          /* C comment.  */
          buflen = 0;
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
              if (buflen >= bufmax)
                {
                  bufmax = 2 * bufmax + 10;
                  buffer = xrealloc (buffer, bufmax);
                }
              buffer[buflen++] = c;
              switch (c)
                {
                case '\n':
                  --buflen;
                  while (buflen >= 1
                         && (buffer[buflen - 1] == ' '
                             || buffer[buflen - 1] == '\t'))
                    --buflen;
                  buffer[buflen] = '\0';
                  savable_comment_add (buffer);
                  buflen = 0;
                  lineno = line_number;
                  last_was_star = false;
                  continue;

                case '*':
                  last_was_star = true;
                  continue;

                case '/':
                  if (last_was_star)
                    {
                      buflen -= 2;
                      while (buflen >= 1
                             && (buffer[buflen - 1] == ' '
                                 || buffer[buflen - 1] == '\t'))
                        --buflen;
                      buffer[buflen] = '\0';
                      savable_comment_add (buffer);
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

        case '/':
          /* C++ comment.  */
          buflen = 0;
          lineno = line_number;
          for (;;)
            {
              c = phase1_getc ();
              if (c == '\n' || c == EOF)
                break;
              /* We skip all leading white space, but not EOLs.  */
              if (!(buflen == 0 && (c == ' ' || c == '\t')))
                {
                  if (buflen >= bufmax)
                    {
                      bufmax = 2 * bufmax + 10;
                      buffer = xrealloc (buffer, bufmax);
                    }
                  buffer[buflen++] = c;
                }
            }
          if (buflen >= bufmax)
            {
              bufmax = 2 * bufmax + 10;
              buffer = xrealloc (buffer, bufmax);
            }
          buffer[buflen] = '\0';
          savable_comment_add (buffer);
          last_comment_line = lineno;
          return '\n';
        }
    }
  else
    return c;
}

/* Supports only one pushback character.  */
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


/* ========================== Reading of tokens.  ========================== */


enum token_type_ty
{
  token_type_eof,
  token_type_lparen,            /* ( */
  token_type_rparen,            /* ) */
  token_type_comma,             /* , */
  token_type_i18n,              /* _( */
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


/* 7. Replace escape sequences within character strings with their
   single character equivalents.  */

#define P7_QUOTES (1000 + '"')

static int
phase7_getc ()
{
  int c;

  for (;;)
    {
      /* Use phase 1, because phase 2 elides comments.  */
      c = phase1_getc ();

      if (c == '"')
        return P7_QUOTES;
      if (c != '\\')
        return c;
      c = phase1_getc ();
      if (c != '\n')
        switch (c)
          {
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

          /* FIXME: What is the octal escape syntax?
             syntax.html says: [0] [0-7]+
             scanner.ll says:  [0-7] [0-7] [0-7]
           */
#if 0
          case '0': case '1': case '2': case '3':
          case '4': case '5': case '6': case '7':
            {
              int n, j;

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
#endif

          default:
            return c;
          }
    }
}


/* Free the memory pointed to by a 'struct token_ty'.  */
static inline void
free_token (token_ty *tp)
{
  if (tp->type == token_type_string_literal || tp->type == token_type_symbol)
    free (tp->string);
  if (tp->type == token_type_string_literal)
    drop_reference (tp->comment);
}


/* Combine characters into tokens.  Discard whitespace.  */

static token_ty phase5_pushback[1];
static int phase5_pushback_length;

static void
phase5_get (token_ty *tp)
{
  static char *buffer;
  static int bufmax;
  int bufpos;
  int c;

  if (phase5_pushback_length)
    {
      *tp = phase5_pushback[--phase5_pushback_length];
      return;
    }
  for (;;)
    {
      tp->line_number = line_number;
      c = phase2_getc ();

      switch (c)
        {
        case EOF:
          tp->type = token_type_eof;
          return;

        case '\n':
          if (last_non_comment_line > last_comment_line)
            savable_comment_reset ();
          FALLTHROUGH;
        case '\r':
        case '\t':
        case ' ':
          /* Ignore whitespace and comments.  */
          continue;
        }

      last_non_comment_line = tp->line_number;

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
          /* Symbol, or part of a number.  */
          bufpos = 0;
          for (;;)
            {
              if (bufpos >= bufmax)
                {
                  bufmax = 2 * bufmax + 10;
                  buffer = xrealloc (buffer, bufmax);
                }
              buffer[bufpos++] = c;
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
                  if (bufpos == 1 && buffer[0] == '_' && c == '(')
                    {
                      tp->type = token_type_i18n;
                      return;
                    }
                  phase2_ungetc (c);
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

        case '"':
          bufpos = 0;
          for (;;)
            {
              c = phase7_getc ();
              if (c == EOF || c == P7_QUOTES)
                break;
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
          buffer[bufpos] = '\0';
          tp->string = xstrdup (buffer);
          tp->type = token_type_string_literal;
          tp->comment = add_reference (savable_comment);
          return;

        case '(':
          tp->type = token_type_lparen;
          return;

        case ')':
          tp->type = token_type_rparen;
          return;

        case ',':
          tp->type = token_type_comma;
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


/* Concatenate adjacent string literals to form single string literals.
   (See libycp/src/parser.yy, rule 'string' vs. terminal 'STRING'.)  */

static token_ty phase8_pushback[1];
static int phase8_pushback_length;

static void
phase8_get (token_ty *tp)
{
  if (phase8_pushback_length)
    {
      *tp = phase8_pushback[--phase8_pushback_length];
      return;
    }
  phase5_get (tp);
  if (tp->type != token_type_string_literal)
    return;
  for (;;)
    {
      token_ty tmp;
      size_t len;

      phase5_get (&tmp);
      if (tmp.type != token_type_string_literal)
        {
          phase5_unget (&tmp);
          return;
        }
      len = strlen (tp->string);
      tp->string = xrealloc (tp->string, len + strlen (tmp.string) + 1);
      strcpy (tp->string + len, tmp.string);
      free_token (&tmp);
    }
}

/* Supports only one pushback token.  */
static void
phase8_unget (token_ty *tp)
{
  if (tp->type != token_type_eof)
    {
      if (phase8_pushback_length == SIZEOF (phase8_pushback))
        abort ();
      phase8_pushback[phase8_pushback_length++] = *tp;
    }
}


/* ========================= Extracting strings.  ========================== */


/* Context lookup table.  */
static flag_context_list_table_ty *flag_context_list_table;


/* Maximum supported nesting depth.  */
#define MAX_NESTING_DEPTH 1000

/* Current nesting depth.  */
static int nesting_depth;


/* The file is broken into tokens.

     Normal handling: Look for
       [A] _( [B] msgid ... )
     Plural handling: Look for
       [A] _( [B] msgid [C] , [D] msgid_plural ... )
     At point [A]: state == 0.
     At point [B]: state == 1, plural_mp == NULL.
     At point [C]: state == 2, plural_mp != NULL.
     At point [D]: state == 1, plural_mp != NULL.

   We use recursion because we have to set the context according to the given
   flags.  */


/* Extract messages until the next balanced closing parenthesis.
   Extracted messages are added to MLP.
   Return true upon eof, false upon closing parenthesis.  */
static bool
extract_parenthesized (message_list_ty *mlp,
                       flag_context_ty outer_context,
                       flag_context_list_iterator_ty context_iter,
                       bool in_i18n)
{
  int state; /* 1 or 2 inside _( ... ), otherwise 0 */
  int plural_state = 0; /* defined only when in states 1 and 2 */
  message_ty *plural_mp = NULL; /* defined only when in states 1 and 2 */
  /* Context iterator that will be used if the next token is a '('.  */
  flag_context_list_iterator_ty next_context_iter =
    passthrough_context_list_iterator;
  /* Current context.  */
  flag_context_ty inner_context =
    inherited_context (outer_context,
                       flag_context_list_iterator_advance (&context_iter));

  /* Start state is 0 or 1.  */
  state = (in_i18n ? 1 : 0);

  for (;;)
    {
      token_ty token;

      if (in_i18n)
        phase8_get (&token);
      else
        phase5_get (&token);

      switch (token.type)
        {
        case token_type_i18n:
          if (++nesting_depth > MAX_NESTING_DEPTH)
            {
              error_with_progname = false;
              error (EXIT_FAILURE, 0, _("%s:%d: error: too many open parentheses"),
                     logical_file_name, line_number);
            }
          if (extract_parenthesized (mlp, inner_context, next_context_iter,
                                     true))
            return true;
          nesting_depth--;
          next_context_iter = null_context_list_iterator;
          state = 0;
          continue;

        case token_type_string_literal:
          if (state == 1)
            {
              lex_pos_ty pos;
              pos.file_name = logical_file_name;
              pos.line_number = token.line_number;

              if (plural_state == 0)
                {
                  /* Seen an msgid.  */
                  token_ty token2;

                  if (in_i18n)
                    phase8_get (&token2);
                  else
                    phase5_get (&token2);

                  plural_mp =
                    remember_a_message (mlp, NULL, token.string, false,
                                        token2.type == token_type_comma,
                                        inner_context, &pos,
                                        NULL, token.comment, false);

                  if (in_i18n)
                    phase8_unget (&token2);
                  else
                    phase5_unget (&token2);

                  plural_state = 1;
                  state = 2;
                }
              else
                {
                  /* Seen an msgid_plural.  */
                  if (plural_mp != NULL)
                    remember_a_message_plural (plural_mp, token.string, false,
                                               inner_context, &pos,
                                               token.comment, false);
                  state = 0;
                }
              drop_reference (token.comment);
            }
          else
            {
              free_token (&token);
              state = 0;
            }
          next_context_iter = null_context_list_iterator;
          continue;

        case token_type_symbol:
          next_context_iter =
            flag_context_list_iterator (
              flag_context_list_table_lookup (
                flag_context_list_table,
                token.string, strlen (token.string)));
          free_token (&token);
          state = 0;
          continue;

        case token_type_lparen:
          if (++nesting_depth > MAX_NESTING_DEPTH)
            {
              error_with_progname = false;
              error (EXIT_FAILURE, 0, _("%s:%d: error: too many open parentheses"),
                     logical_file_name, line_number);
            }
          if (extract_parenthesized (mlp, inner_context, next_context_iter,
                                     false))
            return true;
          nesting_depth--;
          next_context_iter = null_context_list_iterator;
          state = 0;
          continue;

        case token_type_rparen:
          return false;

        case token_type_comma:
          if (state == 2)
            state = 1;
          else
            state = 0;
          inner_context =
            inherited_context (outer_context,
                               flag_context_list_iterator_advance (
                                 &context_iter));
          next_context_iter = passthrough_context_list_iterator;
          continue;

        case token_type_other:
          next_context_iter = null_context_list_iterator;
          state = 0;
          continue;

        case token_type_eof:
          return true;

        default:
          abort ();
        }
    }
}


void
extract_ycp (FILE *f,
             const char *real_filename, const char *logical_filename,
             flag_context_list_table_ty *flag_table,
             msgdomain_list_ty *mdlp)
{
  message_list_ty *mlp = mdlp->item[0]->messages;

  fp = f;
  real_file_name = real_filename;
  logical_file_name = xstrdup (logical_filename);
  line_number = 1;
  char_in_line = 0;

  last_comment_line = -1;
  last_non_comment_line = -1;

  phase2_pushback_length = 0;
  phase5_pushback_length = 0;
  phase8_pushback_length = 0;

  flag_context_list_table = flag_table;
  nesting_depth = 0;

  /* Eat tokens until eof is seen.  When extract_parenthesized returns
     due to an unbalanced closing parenthesis, just restart it.  */
  while (!extract_parenthesized (mlp, null_context, null_context_list_iterator,
                                 false))
    ;

  fp = NULL;
  real_file_name = NULL;
  logical_file_name = NULL;
  line_number = 0;
  char_in_line = 0;
}
