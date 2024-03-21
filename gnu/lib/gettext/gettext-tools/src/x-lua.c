/* xgettext Lua backend.
   Copyright (C) 2012-2013, 2016, 2018-2023 Free Software Foundation, Inc.

   This file was written by Ľubomír Remák <lubomirr@lubomirr.eu>, 2012.

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
#include "config.h"
#endif

/* Specification.  */
#include "x-lua.h"

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
#include "po-charset.h"

#define _(s) gettext(s)

#define SIZEOF(a) (sizeof(a) / sizeof(a[0]))

/* The Lua syntax is defined in the Lua manual sections 3.1 and 9,
   which can be found at
   https://www.lua.org/manual/5.2/manual.html#3.1
   https://www.lua.org/manual/5.2/manual.html#9  */

/* If true extract all strings.  */
static bool extract_all = false;

/* A hash table for keywords.  */
static hash_table keywords;
static bool default_keywords = true;

/* Set extract_all flag (gettext will extract all strings).  */
void
x_lua_extract_all ()
{
  extract_all = true;
}

/* Adds a keyword.  Copied from other lexers.  */
void
x_lua_keyword (const char *name)
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
      x_lua_keyword ("_");
      x_lua_keyword ("gettext.gettext");
      x_lua_keyword ("gettext.dgettext:2");
      x_lua_keyword ("gettext.dcgettext:2");
      x_lua_keyword ("gettext.ngettext:1,2");
      x_lua_keyword ("gettext.dngettext:2,3");
      x_lua_keyword ("gettext.dcngettext:2,3");
      default_keywords = false;
    }
}

void
init_flag_table_lua ()
{
  xgettext_record_flag ("_:1:pass-lua-format");
  xgettext_record_flag ("gettext.gettext:1:pass-lua-format");
  xgettext_record_flag ("gettext.dgettext:2:pass-lua-format");
  xgettext_record_flag ("gettext.dcgettext:2:pass-lua-format");
  xgettext_record_flag ("gettext.ngettext:1:pass-lua-format");
  xgettext_record_flag ("gettext.ngettext:2:pass-lua-format");
  xgettext_record_flag ("gettext.dngettext:2:pass-lua-format");
  xgettext_record_flag ("gettext.dngettext:3:pass-lua-format");
  xgettext_record_flag ("gettext.dcngettext:2:pass-lua-format");
  xgettext_record_flag ("gettext.dcngettext:3:pass-lua-format");
  xgettext_record_flag ("string.format:1:lua-format");
}


/* ======================== Reading of characters.  ======================== */

/* The input file stream.  */
static FILE *fp;


/* 1. line_number handling.  */

static unsigned char phase1_pushback[2];
static int phase1_pushback_length;

static bool first_character;

static int
phase1_getc ()
{
  int c;

  if (phase1_pushback_length)
    c = phase1_pushback[--phase1_pushback_length];
  else
    {
      c = getc (fp);

      if (first_character)
        {
          first_character = false;

          /* Ignore shebang line.  No pushback required in this case.  */
          if (c == '#')
            {
              while (c != '\n' && c != EOF)
                c = getc (fp);
              if (c == '\n')
                {
                  line_number++;
                  c = getc (fp);
                }
            }
        }

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

/* Eats characters until '\n' and adds them to the comment.  */
static void
eat_comment_line ()
{
  for (;;)
    {
      int c = phase1_getc ();
      if (c == '\n' || c == EOF)
        {
          comment_line_end (0);
          break;
        }

      if (!(buflen == 0 && (c == ' ' || c == '\t')))
        comment_add (c);
    }
}

static int
phase2_getc ()
{
  int c;
  int lineno;

  c = phase1_getc ();

  if (c == '-')
    {
      c = phase1_getc ();

      if (c == '-')
        {
          /* It starts with '--', so it must be either a short or a long
             comment.  */
          c = phase1_getc ();

          if (c == '[')
            {
              c = phase1_getc ();

              int esigns = 0;
              while (c == '=')
                {
                  esigns++;
                  c = phase1_getc ();
                }

              if (c == '[')
                {
                  /* Long comment.  */
                  bool right_bracket = false;
                  bool end = false;
                  int esigns2 = 0;

                  lineno = line_number;
                  comment_start ();
                  while (!end)
                    {
                      c = phase1_getc ();

                      if (c == EOF)
                        break;

                      /* Ignore leading spaces and tabs.  */
                      if (!(buflen == 0 && (c == ' ' || c == '\t')))
                        {
                          comment_add (c);

                          switch (c)
                            {
                            case ']':
                              if (!right_bracket)
                                {
                                  right_bracket = true;
                                  esigns2 = 0;
                                }
                              else
                                {
                                  if (esigns2 == esigns)
                                    {
                                      comment_line_end (2 + esigns);
                                      end = true;
                                    }
                                }
                              break;

                            case '=':
                              if (right_bracket)
                                esigns2++;
                              break;

                            case '\n':
                              comment_line_end (1);
                              comment_start ();
                              lineno = line_number;
                              FALLTHROUGH;
                            default:
                              right_bracket = false;
                            }
                        }
                    }
                  last_comment_line = lineno;
                  return ' ';
                }
              else
                {
                  /* One line (short) comment, starting with '--[=...='.  */
                  lineno = last_comment_line;
                  comment_start ();
                  comment_add ('[');
                  while (esigns--)
                    comment_add ('=');
                  phase1_ungetc (c);
                  eat_comment_line ();
                  last_comment_line = lineno;
                  return '\n';
                }
            }
          else
            {
              /* One line (short) comment.  */
              lineno = line_number;
              comment_start ();
              phase1_ungetc (c);
              eat_comment_line ();
              last_comment_line = lineno;
              return '\n';
            }
        }
      else
        {
          /* Minus sign.  */
          phase1_ungetc (c);
          return '-';
        }
    }
  else
    return c;
}


/* ========================== Reading of tokens.  ========================== */

enum token_type_ty
{
  token_type_eof,
  token_type_lparen,            /* ( */
  token_type_rparen,            /* ) */
  token_type_lbracket,          /* [ */
  token_type_rbracket,          /* ] */
  token_type_comma,             /* , */
  token_type_dot,               /* . */
  token_type_doubledot,         /* .. */
  token_type_operator1,         /* + - * / % not # - ^ */
  token_type_operator2,         /* < > <= >= ~= == and or */
  token_type_string,
  token_type_number,
  token_type_symbol,
  token_type_other
};

typedef enum token_type_ty token_type_ty;

typedef struct token_ty token_ty;
struct token_ty
{
  token_type_ty type;
  char *string; /* for token_type_string_literal, token_type_symbol */
  refcounted_string_list_ty *comment;  /* for token_type_string_literal */
  int line_number;
};

/* Free the memory pointed to by a 'struct token_ty'.  */
static inline void
free_token (token_ty *tp)
{
  if (tp->type == token_type_string || tp->type == token_type_symbol)
    free (tp->string);
  if (tp->type == token_type_string)
    drop_reference (tp->comment);
}

/* Our current string.  */
static int string_buf_length;
static int string_buf_alloc;
static char *string_buf;

static void
string_start ()
{
  string_buf_length = 0;
}

static void
string_add (int c)
{
  if (string_buf_length >= string_buf_alloc)
    {
      string_buf_alloc = 2 * string_buf_alloc + 10;
      string_buf = xrealloc (string_buf, string_buf_alloc);
    }

  string_buf[string_buf_length++] = c;
}

static void
string_end ()
{
  if (string_buf_length >= string_buf_alloc)
    {
      string_buf_alloc = string_buf_alloc + 1;
      string_buf = xrealloc (string_buf, string_buf_alloc);
    }

  string_buf[string_buf_length] = '\0';
}


/* We need 3 pushback tokens for string optimization.  */
static int phase3_pushback_length;
static token_ty phase3_pushback[3];


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

static void
phase3_get (token_ty *tp)
{
  int c;
  int c2;
  int c_start;

  if (phase3_pushback_length)
    {
      *tp = phase3_pushback[--phase3_pushback_length];
      return;
    }

  tp->string = NULL;

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
        case ' ':
        case '\t':
        case '\f':
          continue;

        case '+':
        case '-':
        case '*':
        case '/':
        case '^':
        case '%':
        case '#':
          tp->type = token_type_operator1;
          return;
        case '<':
        case '>':
        case '=':
          c2 = phase1_getc ();
          if (c2 != '=')
            phase1_ungetc (c2);
          tp->type = token_type_operator2;
          return;
        case '~':
          c2 = phase1_getc ();
          if (c2 == '=')
            {
              tp->type = token_type_operator2;
              return;
            }
          else
            phase1_ungetc (c2);
          continue;
        case '(':
          tp->type = token_type_lparen;
          return;
        case ')':
          tp->type = token_type_rparen;
          return;
        case ',':
          tp->type = token_type_comma;
          return;

        case ';':
          tp->type = token_type_other;
          return;

          /* There are three operators beginning with a dot.  '.',
             '..' and '...'.  The most useful for us is the string
             concatenation operator ('..').  */
        case '.':
          c = phase1_getc ();
          if (c == '.')
            {
              c = phase1_getc ();
              if (c == '.')
                {
                  tp->type = token_type_other;
                  return;
                }
              else
                {
                  phase1_ungetc (c);
                  tp->type = token_type_doubledot;
                  return;
                }
            }
          else if (c >= '0' && c <= '9')
            {
              /* It's a number.  We aren't interested in the actual
                 numeric value, so ignore the dot and let next
                 iteration eat the number.  */
              phase1_ungetc (c);
              continue;
            }
          else
            {
              phase1_ungetc (c);
              tp->type = token_type_dot;
              return;
            }

        case '"':
        case '\'':
          c_start = c;
          string_start ();

          for (;;)
            {
              /* We need unprocessed characters from phase 1.  */
              c = phase1_getc ();

              if (c == EOF || c == c_start || c == '\n')
                {
                  /* End of string.  */
                  string_end ();
                  tp->string = xstrdup (string_buf);
                  tp->comment = add_reference (savable_comment);
                  tp->type = token_type_string;
                  return;
                }

              /* We got '\', this is probably an escape sequence.  */
              if (c == '\\')
                {
                  c = phase1_getc ();
                  switch (c)
                    {
                    case 'a':
                      string_add ('\a');
                      break;
                    case 'b':
                      string_add ('\b');
                      break;
                    case 'f':
                      string_add ('\f');
                      break;
                    case 'n':
                      string_add ('\n');
                      break;
                    case 'r':
                      string_add ('\r');
                      break;
                    case 't':
                      string_add ('\t');
                      break;
                    case 'v':
                      string_add ('\v');
                      break;
                    case 'x':
                      {
                        int num = 0;
                        int i = 0;

                        for (i = 0; i < 2; i++)
                          {
                            c = phase1_getc ();
                            if (c >= '0' && c <= '9')
                              num += c - '0';
                            else if (c >= 'a' && c <= 'f')
                              num += c - 'a' + 10;
                            else if (c >= 'A' && c <= 'F')
                              num += c - 'A' + 10;
                            else
                              {
                                phase1_ungetc (c);
                                break;
                              }

                            if (i == 0)
                              num *= 16;
                          }

                        if (i == 2)
                          string_add (num);
                      }

                      break;
                    case 'z':
                      /* Ignore the following whitespace.  */
                      do
                        {
                          c = phase1_getc ();
                        }
                      while (c == ' ' || c == '\n' || c == '\t' || c == '\r'
                             || c == '\f' || c == '\v');

                      phase1_ungetc (c);

                      break;
                    default:
                      /* Check if it's a '\ddd' sequence.  */
                      if (c >= '0' && c <= '9')
                        {
                          int num = 0;
                          int i = 0;

                          while (c >= '0' && c <= '9' && i < 3)
                            {
                              num *= 10;
                              num += (c - '0');
                              c = phase1_getc ();
                              i++;
                            }

                          /* The last read character is either a
                             non-number or another number after our
                             '\ddd' sequence.  We need to ungetc it.  */
                          phase1_ungetc (c);

                          /* The sequence number is too big, this
                             causes a lexical error.  Ignore it.  */
                          if (num < 256)
                            string_add (num);
                        }
                      else
                        string_add (c);
                    }
                }
              else
                string_add (c);
            }
          break;

        case '[':
          c = phase1_getc ();

          /* Count the number of equal signs.  */
          int esigns = 0;
          while (c == '=')
            {
              esigns++;
              c = phase1_getc ();
            }

          if (c != '[')
            {
              /* We did not find what we were looking for, ungetc it.  */
              phase1_ungetc (c);
              if (esigns == 0)
                {
                  /* Our current character isn't '[' and we got 0 equal
                     signs, so the first '[' must have been a left
                     bracket.  */
                  tp->type = token_type_lbracket;
                  return;
                }
              else
                /* Lexical error, ignore it.  */
                continue;
            }

          /* Found an opening long bracket.  */
          string_start ();

          /* See if it is immediately followed by a newline.  */
          c = phase1_getc ();
          if (c != '\n')
            phase1_ungetc (c);

          for (;;)
            {
              c = phase1_getc ();

              if (c == EOF)
                {
                  string_end ();
                  tp->string = xstrdup (string_buf);
                  tp->comment = add_reference (savable_comment);
                  tp->type = token_type_string;
                  return;
                }
              if (c == ']')
                {
                  c = phase1_getc ();

                  /* Count the number of equal signs.  */
                  int esigns2 = 0;
                  while (c == '=')
                    {
                      esigns2++;
                      c = phase1_getc ();
                    }

                  if (c == ']' && esigns == esigns2)
                    {
                      /* We got ']==...==]', where the number of equal
                         signs matches the number of equal signs in
                         the opening bracket.  */
                      string_end ();
                      tp->string = xstrdup (string_buf);
                      tp->comment = add_reference (savable_comment);
                      tp->type = token_type_string;
                      return;
                    }
                  else
                    {
                      /* Otherwise we got either ']==' garbage or
                         ']==...==]' with a different number of equal
                         signs.

                         Add ']' and equal signs to the string, and
                         ungetc the current character, because the
                         second ']' might be a part of another closing
                         long bracket, e.g. '==]===]'.  */
                      phase1_ungetc (c);

                      string_add (']');
                      while (esigns2--)
                        string_add ('=');
                    }
                }
              else
                string_add (c);
            }
          break;

        case ']':
          tp->type = token_type_rbracket;
          return;

        default:
          if (c >= '0' && c <= '9')
            {
              while (c >= '0' && c <= '9')
                c = phase1_getc ();

              if (c == '.')
                {
                  c = phase1_getc ();
                  while (c >= '0' && c <= '9')
                    c = phase1_getc ();
                }

              if (c == 'e' || c == 'E')
                {
                  if (c == '+' || c == '-')
                    c = phase1_getc ();
                  while (c >= '0' && c <= '9')
                    c = phase1_getc ();
                }

              phase1_ungetc (c);

              tp->type = token_type_number;
              return;
            }
          else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
                   || c == '_')
            {
              string_start ();
              while ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
                     || c == '_' || (c >= '0' && c <= '9'))
                {
                  string_add (c);
                  c = phase1_getc ();
                }
              string_end ();
              phase1_ungetc (c);

              if (strcmp (string_buf, "not") == 0)
                tp->type = token_type_operator1;
              else if (strcmp (string_buf, "and") == 0)
                tp->type = token_type_operator2;
              else if (strcmp (string_buf, "or") == 0)
                tp->type = token_type_operator2;
              else
                {
                  tp->string = xstrdup (string_buf);
                  tp->type = token_type_symbol;
                }
              return;
            }
          else
            tp->type = token_type_other;
        }
    }
}

/* String and symbol concatenation.  */

static token_type_ty phase4_last;

/* We need 3 pushback tokens for string and symbol concatenation.  */
static int phase4_pushback_length;
static token_ty phase4_pushback[3];

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

static void
phase4_get (token_ty *tp)
{
  if (phase4_pushback_length)
    {
      *tp = phase4_pushback[--phase4_pushback_length];
      phase4_last = tp->type;
      return;
    }

  phase3_get (tp);
  if (tp->type == token_type_string
      && !(phase4_last == token_type_operator1
           || phase4_last == token_type_dot
           || phase4_last == token_type_symbol
           || phase4_last == token_type_doubledot
           || phase4_last == token_type_rparen))
    {
      char *sum = tp->string;
      size_t sum_len = strlen (sum);

      for (;;)
        {
          token_ty token2;

          phase3_get (&token2);
          if (token2.type == token_type_doubledot)
            {
              token_ty token3;

              phase3_get (&token3);
              if (token3.type == token_type_string)
                {
                  token_ty token_after;

                  phase3_get (&token_after);
                  if (token_after.type != token_type_operator1)
                    {
                      char *addend = token3.string;
                      size_t addend_len = strlen (addend);

                      sum = (char *) xrealloc (sum, sum_len + addend_len + 1);
                      memcpy (sum + sum_len, addend, addend_len + 1);
                      sum_len += addend_len;

                      phase3_unget (&token_after);
                      free_token (&token3);
                      free_token (&token2);
                      continue;
                    }
                  phase3_unget (&token_after);
                }
              phase3_unget (&token3);
            }
          phase3_unget (&token2);
          break;
        }
      tp->string = sum;
    }
  phase4_last = tp->type;
}

static void
phase5_get (token_ty *tp)
{
  phase4_get (tp);

  /* Combine symbol1 . ... . symbolN to a single strings, so that
     we can recognize function calls like
     gettext.gettext.  The information present for
     symbolI.....symbolN has precedence over the information for
     symbolJ.....symbolN with J > I.  */
  if (tp->type == token_type_symbol)
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
              if (token3.type == token_type_symbol)
                {
                  char *addend = token3.string;
                  size_t addend_len = strlen (addend);

                  sum = (char *) xrealloc (sum, sum_len + 1 + addend_len + 1);
                  sum[sum_len] = '.';
                  memcpy (sum + sum_len + 1, addend, addend_len + 1);
                  sum_len += 1 + addend_len;

                  free_token (&token2);
                  free_token (&token3);
                  continue;
                }
              phase4_unget (&token3);
            }
          phase4_unget (&token2);
          break;
        }
      tp->string = sum;
    }
}

static void
x_lua_lex (token_ty *tok)
{
  phase5_get (tok);
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
   looking at a valid Lua program, and leave the complaints about the
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

      x_lua_lex (&token);

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
          break;

        case token_type_rbracket:
          if (delim == token_type_rbracket || delim == token_type_eof)
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

        case token_type_string:
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
                /* A string immediately after a symbol means a function call.  */
                if (state)
                  {
                    struct arglist_parser *tmp_argparser;
                    tmp_argparser = arglist_parser_alloc (mlp, next_shapes);

                    arglist_parser_remember (tmp_argparser, 1, ms,
                                             inner_context,
                                             pos.file_name, pos.line_number,
                                             token.comment, false);
                    arglist_parser_done (tmp_argparser, 1);
                  }
                else
                  arglist_parser_remember (argparser, arg, ms,
                                           inner_context,
                                           pos.file_name, pos.line_number,
                                           token.comment, false);
              }
          }
          drop_reference (token.comment);
          next_context_iter = null_context_list_iterator;
          state = 0;
          continue;

        case token_type_dot:
        case token_type_doubledot:
        case token_type_operator1:
        case token_type_operator2:
        case token_type_number:
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
extract_lua (FILE *f,
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
  first_character = true;

  last_comment_line = -1;
  last_non_comment_line = -1;

  phase3_pushback_length = 0;

  phase4_last = token_type_eof;
  phase4_pushback_length = 0;

  flag_context_list_table = flag_table;
  paren_nesting_depth = 0;
  bracket_nesting_depth = 0;

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
