/* xgettext awk backend.
   Copyright (C) 2002-2003, 2005-2009, 2018-2023 Free Software Foundation, Inc.

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
#include "x-awk.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "attribute.h"
#include "message.h"
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


/* The awk syntax is defined in the gawk manual page and documentation.
   See also gawk/awkgram.y.  */


/* ====================== Keyword set customization.  ====================== */

/* If true extract all strings.  */
static bool extract_all = false;

static hash_table keywords;
static bool default_keywords = true;


void
x_awk_extract_all ()
{
  extract_all = true;
}


void
x_awk_keyword (const char *name)
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
      x_awk_keyword ("dcgettext");
      x_awk_keyword ("dcngettext:1,2");
      default_keywords = false;
    }
}

void
init_flag_table_awk ()
{
  xgettext_record_flag ("dcgettext:1:pass-awk-format");
  xgettext_record_flag ("dcngettext:1:pass-awk-format");
  xgettext_record_flag ("dcngettext:2:pass-awk-format");
  xgettext_record_flag ("printf:1:awk-format");
}


/* ======================== Reading of characters.  ======================== */

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


/* 2. Replace each comment that is not inside a string literal or regular
   expression with a newline character.  We need to remember the comment
   for later, because it may be attached to a keyword string.  */

static int
phase2_getc ()
{
  static char *buffer;
  static size_t bufmax;
  size_t buflen;
  int lineno;
  int c;

  c = phase1_getc ();
  if (c == '#')
    {
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
    }
  return c;
}

/* Supports only one pushback character.  */
static void
phase2_ungetc (int c)
{
  if (c != EOF)
    phase1_ungetc (c);
}


/* ========================== Reading of tokens.  ========================== */


enum token_type_ty
{
  token_type_eof,
  token_type_lparen,            /* ( */
  token_type_rparen,            /* ) */
  token_type_comma,             /* , */
  token_type_string,            /* "abc" */
  token_type_i18nstring,        /* _"abc" */
  token_type_symbol,            /* symbol, number */
  token_type_semicolon,         /* ; */
  token_type_other              /* regexp, misc. operator */
};
typedef enum token_type_ty token_type_ty;

typedef struct token_ty token_ty;
struct token_ty
{
  token_type_ty type;
  char *string;         /* for token_type_{symbol,string,i18nstring} */
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

      if (c == EOF || c == '\n')
        break;
      if (c == '"')
        return P7_QUOTES;
      if (c != '\\')
        return c;
      c = phase1_getc ();
      if (c == EOF)
        break;
      if (c != '\n')
        switch (c)
          {
          case 'a':
            return '\a';
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
          case '0': case '1': case '2': case '3': case '4':
          case '5': case '6': case '7':
            {
              int n = c - '0';

              c = phase1_getc ();
              if (c != EOF)
                {
                  if (c >= '0' && c <= '7')
                    {
                      n = (n << 3) + (c - '0');
                      c = phase1_getc ();
                      if (c != EOF)
                        {
                          if (c >= '0' && c <= '7')
                            n = (n << 3) + (c - '0');
                          else
                            phase1_ungetc (c);
                        }
                    }
                  else
                    phase1_ungetc (c);
                }
              return (unsigned char) n;
            }
          case 'x':
            {
              int n = 0;

              for (;;)
                {
                  c = phase1_getc ();
                  if (c == EOF)
                    break;
                  else if (c >= '0' && c <= '9')
                    n = (n << 4) + (c - '0');
                  else if (c >= 'A' && c <= 'F')
                    n = (n << 4) + (c - 'A' + 10);
                  else if (c >= 'a' && c <= 'f')
                    n = (n << 4) + (c - 'a' + 10);
                  else
                    {
                      phase1_ungetc (c);
                      break;
                    }
                }
              return (unsigned char) n;
            }
          default:
            return c;
          }
    }

  phase1_ungetc (c);
  error_with_progname = false;
  error (0, 0, _("%s:%d: warning: unterminated string"), logical_file_name,
         line_number);
  error_with_progname = true;
  return P7_QUOTES;
}


/* Free the memory pointed to by a 'struct token_ty'.  */
static inline void
free_token (token_ty *tp)
{
  switch (tp->type)
    {
    case token_type_string:
    case token_type_i18nstring:
    case token_type_symbol:
      free (tp->string);
      break;
    default:
      break;
    }
}


/* Combine characters into tokens.  Discard whitespace.  */

/* There is an ambiguity about '/': It can start a division operator ('/' or
   '/=') or it can start a regular expression.  The distinction is important
   because inside regular expressions, '#' and '"' lose its special meanings.
   If you look at the awk grammar, you see that the operator is only allowed
   right after a 'variable' or 'simp_exp' nonterminal, and these nonterminals
   can only end in the NAME, LENGTH, YSTRING, YNUMBER, ')', ']' terminals.
   So we prefer the division operator interpretation only right after
   symbol, string, number, ')', ']', with whitespace but no newline allowed
   in between.  */
static bool prefer_division_over_regexp;

static void
x_awk_lex (token_ty *tp)
{
  static char *buffer;
  static int bufmax;
  int bufpos;
  int c;

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
          /* Newline is not allowed inside expressions.  It usually
             introduces a fresh statement.
             FIXME: Newlines after any of ',' '{' '?' ':' '||' '&&' 'do' 'else'
             does *not* introduce a fresh statement.  */
          prefer_division_over_regexp = false;
          FALLTHROUGH;
        case '\t':
        case ' ':
          /* Ignore whitespace and comments.  */
          continue;

        case '\\':
          /* Backslash ought to be immediately followed by a newline.  */
          continue;
        }

      last_non_comment_line = tp->line_number;

      switch (c)
        {
        case '.':
          {
            int c2 = phase2_getc ();
            phase2_ungetc (c2);
            if (!(c2 >= '0' && c2 <= '9'))
              {

                tp->type = token_type_other;
                prefer_division_over_regexp = false;
                return;
              }
          }
          FALLTHROUGH;
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
                  if (bufpos == 1 && buffer[0] == '_' && c == '"')
                    {
                      tp->type = token_type_i18nstring;
                      goto case_string;
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
          /* Most identifiers can be variable names; after them we must
             interpret '/' as division operator.  But for awk's builtin
             keywords we have three cases:
             (a) Must interpret '/' as division operator. "length".
             (b) Must interpret '/' as start of a regular expression.
                 "do", "exit", "print", "printf", "return".
             (c) '/' after this keyword in invalid anyway. All others.
             I used the following script for the distinction.
                for k in $awk_keywords; do
                  echo; echo $k; awk "function foo () { $k / 10 }" < /dev/null
                done
           */
          if (strcmp (buffer, "do") == 0
              || strcmp (buffer, "exit") == 0
              || strcmp (buffer, "print") == 0
              || strcmp (buffer, "printf") == 0
              || strcmp (buffer, "return") == 0)
            prefer_division_over_regexp = false;
          else
            prefer_division_over_regexp = true;
          return;

        case '"':
          tp->type = token_type_string;
        case_string:
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
          prefer_division_over_regexp = true;
          return;

        case '(':
          tp->type = token_type_lparen;
          prefer_division_over_regexp = false;
          return;

        case ')':
          tp->type = token_type_rparen;
          prefer_division_over_regexp = true;
          return;

        case ',':
          tp->type = token_type_comma;
          prefer_division_over_regexp = false;
          return;

        case ';':
          tp->type = token_type_semicolon;
          prefer_division_over_regexp = false;
          return;

        case ']':
          tp->type = token_type_other;
          prefer_division_over_regexp = true;
          return;

        case '/':
          if (!prefer_division_over_regexp)
            {
              /* Regular expression.
                 Counting brackets is non-trivial. [[] is balanced, and so is
                 [\]]. Also, /[/]/ is balanced and ends at the third slash.
                 Do not count [ or ] if either one is preceded by a \.
                 A '[' should be counted if
                  a) it is the first one so far (brackets == 0), or
                  b) it is the '[' in '[:'.
                 A ']' should be counted if not preceded by a \.
                 According to POSIX, []] is how you put a ] into a set.
                 Try to handle that too.
               */
              int brackets = 0;
              bool pos0 = true;         /* true at start of regexp */
              bool pos1_open = false;   /* true after [ at start of regexp */
              bool pos2_open_not = false; /* true after [^ at start of regexp */

              for (;;)
                {
                  c = phase1_getc ();

                  if (c == EOF || c == '\n')
                    {
                      phase1_ungetc (c);
                      error_with_progname = false;
                      error (0, 0, _("%s:%d: warning: unterminated regular expression"),
                             logical_file_name, line_number);
                      error_with_progname = true;
                      break;
                    }
                  else if (c == '[')
                    {
                      if (brackets == 0)
                        brackets++;
                      else
                        {
                          c = phase1_getc ();
                          if (c == ':')
                            brackets++;
                          phase1_ungetc (c);
                        }
                      if (pos0)
                        {
                          pos0 = false;
                          pos1_open = true;
                          continue;
                        }
                    }
                  else if (c == ']')
                    {
                      if (!(pos1_open || pos2_open_not))
                        brackets--;
                    }
                  else if (c == '^')
                    {
                      if (pos1_open)
                        {
                          pos1_open = false;
                          pos2_open_not = true;
                          continue;
                        }
                    }
                  else if (c == '\\')
                    {
                      c = phase1_getc ();
                      /* Backslash-newline is valid and ignored.  */
                    }
                  else if (c == '/')
                    {
                      if (brackets <= 0)
                        break;
                    }

                  pos0 = false;
                  pos1_open = false;
                  pos2_open_not = false;
                }

              tp->type = token_type_other;
              prefer_division_over_regexp = false;
              return;
            }
          FALLTHROUGH;

        default:
          /* We could carefully recognize each of the 2 and 3 character
             operators, but it is not necessary, as we only need to recognize
             gettext invocations.  Don't bother.  */
          tp->type = token_type_other;
          prefer_division_over_regexp = false;
          return;
        }
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
   looking at a valid C or C++ program, and leave the complaints about
   the grammar to the compiler.

     Normal handling: Look for
       keyword ( ... msgid ... )
     Plural handling: Look for
       keyword ( ... msgid ... msgid_plural ... )

   We use recursion because the arguments before msgid or between msgid
   and msgid_plural can contain subexpressions of the same form.  */


/* Extract messages until the next balanced closing parenthesis.
   Extracted messages are added to MLP.
   Return true upon eof, false upon closing parenthesis.  */
static bool
extract_parenthesized (message_list_ty *mlp,
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
  /* Whether to implicitly assume the next tokens are arguments even without
     a '('.  */
  bool next_is_argument = false;
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

      x_awk_lex (&token);

      if (next_is_argument && token.type != token_type_lparen)
        {
          /* An argument list starts, even though there is no '('.  */
          context_iter = next_context_iter;
          outer_context = inner_context;
          inner_context =
            inherited_context (outer_context,
                               flag_context_list_iterator_advance (
                                 &context_iter));
        }

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
          next_is_argument =
            (strcmp (token.string, "print") == 0
             || strcmp (token.string, "printf") == 0);
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
          if (extract_parenthesized (mlp, inner_context, next_context_iter,
                                     arglist_parser_alloc (mlp,
                                                           state ? next_shapes : NULL)))
            {
              arglist_parser_done (argparser, arg);
              return true;
            }
          nesting_depth--;
          next_is_argument = false;
          next_context_iter = null_context_list_iterator;
          state = 0;
          continue;

        case token_type_rparen:
          arglist_parser_done (argparser, arg);
          return false;

        case token_type_comma:
          arg++;
          inner_context =
            inherited_context (outer_context,
                               flag_context_list_iterator_advance (
                                 &context_iter));
          next_is_argument = false;
          next_context_iter = passthrough_context_list_iterator;
          state = 0;
          continue;

        case token_type_string:
          {
            lex_pos_ty pos;
            pos.file_name = logical_file_name;
            pos.line_number = token.line_number;

            if (extract_all)
              remember_a_message (mlp, NULL, token.string, false, false,
                                  inner_context, &pos,
                                  NULL, savable_comment, false);
            else
              {
                mixed_string_ty *ms =
                  mixed_string_alloc_simple (token.string, lc_string,
                                             pos.file_name, pos.line_number);
                free (token.string);
                arglist_parser_remember (argparser, arg, ms,
                                         inner_context,
                                         pos.file_name, pos.line_number,
                                         savable_comment, false);
              }
          }
          next_is_argument = false;
          next_context_iter = null_context_list_iterator;
          state = 0;
          continue;

        case token_type_i18nstring:
          {
            lex_pos_ty pos;
            pos.file_name = logical_file_name;
            pos.line_number = token.line_number;

            remember_a_message (mlp, NULL, token.string, false, false,
                                inner_context, &pos,
                                NULL, savable_comment, false);
          }
          next_is_argument = false;
          next_context_iter = null_context_list_iterator;
          state = 0;
          continue;

        case token_type_semicolon:
          /* An argument list ends, and a new statement begins.  */
          /* FIXME: Should handle newline that acts as statement separator
             in the same way.  */
          /* FIXME: Instead of resetting outer_context here, it may be better
             to recurse in the next_is_argument handling above, waiting for
             the next semicolon or other statement terminator.  */
          outer_context = null_context;
          context_iter = null_context_list_iterator;
          next_is_argument = false;
          next_context_iter = passthrough_context_list_iterator;
          inner_context =
            inherited_context (outer_context,
                               flag_context_list_iterator_advance (
                                 &context_iter));
          state = 0;
          continue;

        case token_type_eof:
          arglist_parser_done (argparser, arg);
          return true;

        case token_type_other:
          next_is_argument = false;
          next_context_iter = null_context_list_iterator;
          state = 0;
          continue;

        default:
          abort ();
        }
    }
}


void
extract_awk (FILE *f,
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

  prefer_division_over_regexp = false;

  flag_context_list_table = flag_table;
  nesting_depth = 0;

  init_keywords ();

  /* Eat tokens until eof is seen.  When extract_parenthesized returns
     due to an unbalanced closing parenthesis, just restart it.  */
  while (!extract_parenthesized (mlp, null_context, null_context_list_iterator,
                                 arglist_parser_alloc (mlp, NULL)))
    ;

  fp = NULL;
  real_file_name = NULL;
  logical_file_name = NULL;
  line_number = 0;
}
