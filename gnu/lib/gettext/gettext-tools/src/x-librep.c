/* xgettext librep backend.
   Copyright (C) 2001-2003, 2005-2009, 2018-2023 Free Software Foundation, Inc.

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
#include "x-librep.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "attribute.h"
#include "c-ctype.h"
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
#include "mem-hash-map.h"
#include "gettext.h"

#define _(s) gettext(s)


/* Summary of librep syntax:
   - ';' starts a comment until end of line.
   - Block comments start with '#|' and end with '|#'.
   - Numbers are constituted of an optional prefix (#b, #B for binary,
     #o, #O for octal, #d, #D for decimal, #x, #X for hexadecimal,
     #e, #E for exact, #i, #I for inexact), an optional sign (+ or -), and
     the digits.
   - Characters are written as '?' followed by the character, possibly
     with an escape sequence, for examples '?a', '?\n', '?\177'.
   - Strings are delimited by double quotes. Backslash introduces an escape
     sequence. The following are understood: '\n', '\r', '\f', '\t', '\a',
     '\\', '\^C', '\012' (octal), '\x12' (hexadecimal).
   - Symbols: can contain meta-characters - whitespace or any from ()[]'";|\' -
     if preceded by backslash or enclosed in |...|.
   - Keywords: written as #:SYMBOL.
   - () delimit lists.
   - [] delimit vectors.
   The reader is implemented in librep-0.14/src/lisp.c.  */


/* ====================== Keyword set customization.  ====================== */

/* If true extract all strings.  */
static bool extract_all = false;

static hash_table keywords;
static bool default_keywords = true;


void
x_librep_extract_all ()
{
  extract_all = true;
}


void
x_librep_keyword (const char *name)
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

      /* The characters between name and end should form a valid Lisp
         symbol.  */
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
      x_librep_keyword ("_");
      default_keywords = false;
    }
}

void
init_flag_table_librep ()
{
  xgettext_record_flag ("_:1:pass-librep-format");
  xgettext_record_flag ("format:2:librep-format");
}


/* ======================== Reading of characters.  ======================== */

/* The input file stream.  */
static FILE *fp;


/* Fetch the next character from the input file.  */
static int
do_getc ()
{
  int c = getc (fp);

  if (c == EOF)
    {
      if (ferror (fp))
        error (EXIT_FAILURE, errno,
               _("error while reading \"%s\""), real_file_name);
    }
  else if (c == '\n')
   line_number++;

  return c;
}

/* Put back the last fetched character, not EOF.  */
static void
do_ungetc (int c)
{
  if (c == '\n')
    line_number--;
  ungetc (c, fp);
}


/* ========================== Reading of tokens.  ========================== */


/* A token consists of a sequence of characters.  */
struct token
{
  int allocated;                /* number of allocated 'token_char's */
  int charcount;                /* number of used 'token_char's */
  char *chars;                  /* the token's constituents */
};

/* Initialize a 'struct token'.  */
static inline void
init_token (struct token *tp)
{
  tp->allocated = 10;
  tp->chars = XNMALLOC (tp->allocated, char);
  tp->charcount = 0;
}

/* Free the memory pointed to by a 'struct token'.  */
static inline void
free_token (struct token *tp)
{
  free (tp->chars);
}

/* Ensure there is enough room in the token for one more character.  */
static inline void
grow_token (struct token *tp)
{
  if (tp->charcount == tp->allocated)
    {
      tp->allocated *= 2;
      tp->chars = (char *) xrealloc (tp->chars, tp->allocated * sizeof (char));
    }
}

/* Read the next token.  If 'first' is given, it points to the first
   character, which has already been read.  Returns true for a symbol,
   false for a number.  */
static bool
read_token (struct token *tp, const int *first)
{
  int c;
  /* Variables for speculative number parsing:  */
  int radix = -1;
  int nfirst = 0;
  bool exact = true;
  bool rational = false;
  bool exponent = false;
  bool had_sign = false;
  bool expecting_prefix = false;

  init_token (tp);

  if (first)
    c = *first;
  else
    c = do_getc ();

  for (;; c = do_getc ())
    {
      switch (c)
        {
        case EOF:
          goto done;

        case ' ': case '\t': case '\n': case '\f': case '\r':
        case '(': case ')': case '[': case ']':
        case '\'': case '"': case ';': case ',': case '`':
          goto done;

        case '\\':
          radix = 0;
          c = do_getc ();
          if (c == EOF)
            /* Invalid, but be tolerant.  */
            break;
          grow_token (tp);
          tp->chars[tp->charcount++] = c;
          break;

        case '|':
          radix = 0;
          for (;;)
            {
              c = do_getc ();
              if (c == EOF || c == '|')
                break;
              grow_token (tp);
              tp->chars[tp->charcount++] = c;
            }
          break;

        default:
          if (radix != 0)
            {
              if (expecting_prefix)
                {
                  switch (c)
                    {
                    case 'B': case 'b':
                      radix = 2;
                      break;
                    case 'O': case 'o':
                      radix = 8;
                      break;
                    case 'D': case 'd':
                      radix = 10;
                      break;
                    case 'X': case 'x':
                      radix = 16;
                      break;
                    case 'E': case 'e':
                    case 'I': case 'i':
                      break;
                    default:
                      radix = 0;
                      break;
                    }
                  expecting_prefix = false;
                  nfirst = tp->charcount + 1;
                }
              else if (tp->charcount == nfirst
                       && (c == '+' || c == '-' || c == '#'))
                {
                  if (c == '#')
                    {
                      if (had_sign)
                        radix = 0;
                      else
                        expecting_prefix = true;
                    }
                  else
                    had_sign = true;
                  nfirst = tp->charcount + 1;
                }
              else
                {
                  switch (radix)
                    {
                    case -1:
                      if (c == '.')
                        {
                          radix = 10;
                          exact = false;
                        }
                      else if (!(c >= '0' && c <= '9'))
                        radix = 0;
                      else if (c == '0')
                        radix = 1;
                      else
                        radix = 10;
                      break;

                    case 1:
                      switch (c)
                        {
                        case 'X': case 'x':
                          radix = 16;
                          nfirst = tp->charcount + 1;
                          break;
                        case '0': case '1': case '2': case '3': case '4':
                        case '5': case '6': case '7':
                          radix = 8;
                          nfirst = tp->charcount;
                          break;
                        case '.': case 'E': case 'e':
                          radix = 10;
                          exact = false;
                          break;
                        case '/':
                          radix = 10;
                          rational = true;
                          break;
                        default:
                          radix = 0;
                          break;
                        }
                      break;

                    default:
                      switch (c)
                        {
                        case '.':
                          if (exact && radix == 10 && !rational)
                            exact = false;
                          else
                            radix = 0;
                          break;
                        case '/':
                          if (exact && !rational)
                            rational = true;
                          else
                            radix = 0;
                          break;
                        case 'E': case 'e':
                          if (radix == 10)
                            {
                              if (!rational && !exponent)
                                {
                                  exponent = true;
                                  exact = false;
                                }
                              else
                                radix = 0;
                              break;
                            }
                          FALLTHROUGH;
                        default:
                          if (exponent && (c == '+' || c == '-'))
                            break;
                          if ((radix <= 10
                               && !(c >= '0' && c <= '0' + radix - 1))
                              || (radix == 16 && !c_isxdigit (c)))
                            radix = 0;
                          break;
                        }
                      break;
                    }
                }
            }
          else
            {
              if (c == '#')
                goto done;
            }
          grow_token (tp);
          tp->chars[tp->charcount++] = c;
        }
    }
 done:
  if (c != EOF)
    do_ungetc (c);
  if (radix > 0 && nfirst < tp->charcount)
    return false; /* number */
  else
    return true; /* symbol */
}


/* ========================= Accumulating comments ========================= */


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


/* These are for tracking whether comments count as immediately before
   keyword.  */
static int last_comment_line;
static int last_non_comment_line;


/* ========================= Accumulating messages ========================= */


static message_list_ty *mlp;


/* ============== Reading of objects.  See CLHS 2 "Syntax".  ============== */


/* We are only interested in symbols (e.g. GETTEXT or NGETTEXT) and strings.
   Other objects need not to be represented precisely.  */
enum object_type
{
  t_symbol,     /* symbol */
  t_string,     /* string */
  t_other,      /* other kind of real object */
  t_dot,        /* '.' pseudo object */
  t_close,      /* ')' or ']' pseudo object */
  t_eof         /* EOF marker */
};

struct object
{
  enum object_type type;
  struct token *token;          /* for t_symbol and t_string */
  int line_number_at_start;     /* for t_string */
};

/* Free the memory pointed to by a 'struct object'.  */
static inline void
free_object (struct object *op)
{
  if (op->type == t_symbol || op->type == t_string)
    {
      free_token (op->token);
      free (op->token);
    }
}

/* Convert a t_symbol/t_string token to a char*.  */
static char *
string_of_object (const struct object *op)
{
  char *str;
  int n;

  if (!(op->type == t_symbol || op->type == t_string))
    abort ();
  n = op->token->charcount;
  str = XNMALLOC (n + 1, char);
  memcpy (str, op->token->chars, n);
  str[n] = '\0';
  return str;
}


/* Context lookup table.  */
static flag_context_list_table_ty *flag_context_list_table;


/* Maximum supported nesting depth.  */
#define MAX_NESTING_DEPTH 1000

/* Current nesting depth.  */
static int nesting_depth;


/* Returns the character represented by an escape sequence.  */
static int
do_getc_escaped (int c)
{
  switch (c)
    {
    case 'n':
      return '\n';
    case 'r':
      return '\r';
    case 'f':
      return '\f';
    case 't':
      return '\t';
    case 'v':
      return '\v';
    case 'a':
      return '\a';
    case '^':
      c = do_getc ();
      if (c == EOF)
        return EOF;
      return c & 0x1f;
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7':
      {
        int n = c - '0';

        c = do_getc ();
        if (c != EOF)
          {
            if (c >= '0' && c <= '7')
              {
                n = (n << 3) + (c - '0');
                c = do_getc ();
                if (c != EOF)
                  {
                    if (c >= '0' && c <= '7')
                      n = (n << 3) + (c - '0');
                    else
                      do_ungetc (c);
                  }
              }
            else
              do_ungetc (c);
          }
        return (unsigned char) n;
      }
    case 'x':
      {
        int n = 0;

        for (;;)
          {
            c = do_getc ();
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
                do_ungetc (c);
                break;
              }
          }
        return (unsigned char) n;
      }
    default:
      return c;
    }
}

/* Read the next object.  */
static void
read_object (struct object *op, flag_context_ty outer_context)
{
  if (nesting_depth > MAX_NESTING_DEPTH)
    {
      error_with_progname = false;
      error (EXIT_FAILURE, 0, _("%s:%d: error: too deeply nested objects"),
             logical_file_name, line_number);
    }
  for (;;)
    {
      int ch;

      ch = do_getc ();

      switch (ch)
        {
        case EOF:
          op->type = t_eof;
          return;

        case '\n':
          /* Comments assumed to be grouped with a message must immediately
             precede it, with no non-whitespace token on a line between
             both.  */
          if (last_non_comment_line > last_comment_line)
            savable_comment_reset ();
          continue;

        case ' ': case '\t': case '\f': case '\r':
          continue;

        case '(':
          {
            int arg = 0;                /* Current argument number.  */
            flag_context_list_iterator_ty context_iter;
            const struct callshapes *shapes = NULL;
            struct arglist_parser *argparser = NULL;

            for (;; arg++)
              {
                struct object inner;
                flag_context_ty inner_context;

                if (arg == 0)
                  inner_context = null_context;
                else
                  inner_context =
                    inherited_context (outer_context,
                                       flag_context_list_iterator_advance (
                                         &context_iter));

                ++nesting_depth;
                read_object (&inner, inner_context);
                nesting_depth--;

                /* Recognize end of list.  */
                if (inner.type == t_close)
                  {
                    op->type = t_other;
                    /* Don't bother converting "()" to "NIL".  */
                    last_non_comment_line = line_number;
                    if (argparser != NULL)
                      arglist_parser_done (argparser, arg);
                    return;
                  }

                /* Dots are not allowed in every position.
                   But be tolerant.  */

                /* EOF inside list is illegal.  But be tolerant.  */
                if (inner.type == t_eof)
                  break;

                if (arg == 0)
                  {
                    /* This is the function position.  */
                    if (inner.type == t_symbol)
                      {
                        char *symbol_name = string_of_object (&inner);
                        void *keyword_value;

                        if (hash_find_entry (&keywords,
                                             symbol_name, strlen (symbol_name),
                                             &keyword_value)
                            == 0)
                          shapes = (const struct callshapes *) keyword_value;

                        argparser = arglist_parser_alloc (mlp, shapes);

                        context_iter =
                          flag_context_list_iterator (
                            flag_context_list_table_lookup (
                              flag_context_list_table,
                              symbol_name, strlen (symbol_name)));

                        free (symbol_name);
                      }
                    else
                      context_iter = null_context_list_iterator;
                  }
                else
                  {
                    /* These are the argument positions.  */
                    if (argparser != NULL && inner.type == t_string)
                      {
                        char *s = string_of_object (&inner);
                        mixed_string_ty *ms =
                          mixed_string_alloc_simple (s, lc_string,
                                                     logical_file_name,
                                                     inner.line_number_at_start);
                        free (s);
                        arglist_parser_remember (argparser, arg, ms,
                                                 inner_context,
                                                 logical_file_name,
                                                 inner.line_number_at_start,
                                                 savable_comment, false);
                      }
                  }

                free_object (&inner);
              }

            if (argparser != NULL)
              arglist_parser_done (argparser, arg);
          }
          op->type = t_other;
          last_non_comment_line = line_number;
          return;

        case '[':
          {
            for (;;)
              {
                struct object inner;

                ++nesting_depth;
                read_object (&inner, null_context);
                nesting_depth--;

                /* Recognize end of vector.  */
                if (inner.type == t_close)
                  {
                    op->type = t_other;
                    last_non_comment_line = line_number;
                    return;
                  }

                /* Dots are not allowed.  But be tolerant.  */

                /* EOF inside vector is illegal.  But be tolerant.  */
                if (inner.type == t_eof)
                  break;

                free_object (&inner);
              }
          }
          op->type = t_other;
          last_non_comment_line = line_number;
          return;

        case ')': case ']':
          /* Tell the caller about the end of list or vector.
             Unmatched closing parenthesis is illegal.  But be tolerant.  */
          op->type = t_close;
          last_non_comment_line = line_number;
          return;

        case ',':
          {
            int c = do_getc ();
            /* The ,@ handling inside lists is wrong anyway, because
               ,@form expands to an unknown number of elements.  */
            if (c != EOF && c != '@')
              do_ungetc (c);
          }
          FALLTHROUGH;
        case '\'':
        case '`':
          {
            struct object inner;

            ++nesting_depth;
            read_object (&inner, null_context);
            nesting_depth--;

            /* Dots and EOF are not allowed here.  But be tolerant.  */

            free_object (&inner);

            op->type = t_other;
            last_non_comment_line = line_number;
            return;
          }

        case ';':
          {
            bool all_semicolons = true;

            last_comment_line = line_number;
            comment_start ();
            for (;;)
              {
                int c = do_getc ();
                if (c == EOF || c == '\n' || c == '\f' || c == '\r')
                  break;
                if (c != ';')
                  all_semicolons = false;
                if (!all_semicolons)
                  {
                    /* We skip all leading white space, but not EOLs.  */
                    if (!(buflen == 0 && (c == ' ' || c == '\t')))
                      comment_add (c);
                  }
              }
            comment_line_end (0);
            continue;
          }

        case '"':
          {
            op->token = XMALLOC (struct token);
            init_token (op->token);
            op->line_number_at_start = line_number;
            for (;;)
              {
                int c = do_getc ();
                if (c == EOF)
                  /* Invalid input.  Be tolerant, no error message.  */
                  break;
                if (c == '"')
                  break;
                if (c == '\\')
                  {
                    c = do_getc ();
                    if (c == EOF)
                      /* Invalid input.  Be tolerant, no error message.  */
                      break;
                    if (c == '\n')
                      /* Ignore escaped newline.  */
                      ;
                    else
                      {
                        c = do_getc_escaped (c);
                        if (c == EOF)
                          /* Invalid input.  Be tolerant, no error message.  */
                          break;
                        grow_token (op->token);
                        op->token->chars[op->token->charcount++] = c;
                      }
                  }
                else
                  {
                    grow_token (op->token);
                    op->token->chars[op->token->charcount++] = c;
                  }
              }
            op->type = t_string;

            if (extract_all)
              {
                lex_pos_ty pos;

                pos.file_name = logical_file_name;
                pos.line_number = op->line_number_at_start;
                remember_a_message (mlp, NULL, string_of_object (op), false,
                                    false, null_context, &pos,
                                    NULL, savable_comment, false);
              }
            last_non_comment_line = line_number;
            return;
          }

        case '?':
          {
            int c = do_getc ();
            if (c == EOF)
              /* Invalid input.  Be tolerant, no error message.  */
              ;
            else if (c == '\\')
              {
                c = do_getc ();
                if (c == EOF)
                  /* Invalid input.  Be tolerant, no error message.  */
                  ;
                else
                  {
                    c = do_getc_escaped (c);
                    if (c == EOF)
                      /* Invalid input.  Be tolerant, no error message.  */
                      ;
                  }
              }
            op->type = t_other;
            last_non_comment_line = line_number;
            return;
          }

        case '#':
          /* Dispatch macro handling.  */
          {
            int dmc = do_getc ();
            if (dmc == EOF)
              /* Invalid input.  Be tolerant, no error message.  */
              {
                op->type = t_other;
                return;
              }

            switch (dmc)
              {
              case '!':
                if (ftell (fp) == 2)
                  /* Skip comment until !# */
                  {
                    int c;

                    c = do_getc ();
                    for (;;)
                      {
                        if (c == EOF)
                          break;
                        if (c == '!')
                          {
                            c = do_getc ();
                            if (c == EOF || c == '#')
                              break;
                          }
                        else
                          c = do_getc ();
                      }
                    if (c == EOF)
                      {
                        /* EOF not allowed here.  But be tolerant.  */
                        op->type = t_eof;
                        return;
                      }
                    continue;
                  }
                FALLTHROUGH;
              case '\'':
              case ':':
                {
                  struct object inner;
                  ++nesting_depth;
                  read_object (&inner, null_context);
                  nesting_depth--;
                  /* Dots and EOF are not allowed here.
                     But be tolerant.  */
                  free_object (&inner);
                  op->type = t_other;
                  last_non_comment_line = line_number;
                  return;
                }

              case '[':
              case '(':
                {
                  struct object inner;
                  do_ungetc (dmc);
                  ++nesting_depth;
                  read_object (&inner, null_context);
                  nesting_depth--;
                  /* Dots and EOF are not allowed here.
                     But be tolerant.  */
                  free_object (&inner);
                  op->type = t_other;
                  last_non_comment_line = line_number;
                  return;
                }

              case '|':
                {
                  int depth = 0;
                  int c;

                  comment_start ();
                  c = do_getc ();
                  for (;;)
                    {
                      if (c == EOF)
                        break;
                      if (c == '|')
                        {
                          c = do_getc ();
                          if (c == EOF)
                            break;
                          if (c == '#')
                            {
                              if (depth == 0)
                                {
                                  comment_line_end (0);
                                  break;
                                }
                              depth--;
                              comment_add ('|');
                              comment_add ('#');
                              c = do_getc ();
                            }
                          else
                            comment_add ('|');
                        }
                      else if (c == '#')
                        {
                          c = do_getc ();
                          if (c == EOF)
                            break;
                          comment_add ('#');
                          if (c == '|')
                            {
                              depth++;
                              comment_add ('|');
                              c = do_getc ();
                            }
                        }
                      else
                        {
                          /* We skip all leading white space.  */
                          if (!(buflen == 0 && (c == ' ' || c == '\t')))
                            comment_add (c);
                          if (c == '\n')
                            {
                              comment_line_end (1);
                              comment_start ();
                            }
                          c = do_getc ();
                        }
                    }
                  if (c == EOF)
                    {
                      /* EOF not allowed here.  But be tolerant.  */
                      op->type = t_eof;
                      return;
                    }
                  last_comment_line = line_number;
                  continue;
                }

              case '\\':
                {
                  struct token token;
                  int first = '\\';
                  read_token (&token, &first);
                  free_token (&token);
                  op->type = t_other;
                  last_non_comment_line = line_number;
                  return;
                }

              case 'T': case 't':
              case 'F': case 'f':
                op->type = t_other;
                last_non_comment_line = line_number;
                return;

              case 'B': case 'b':
              case 'O': case 'o':
              case 'D': case 'd':
              case 'X': case 'x':
              case 'E': case 'e':
              case 'I': case 'i':
                {
                  struct token token;
                  do_ungetc (dmc);
                  {
                    int c;
                    c = '#';
                    read_token (&token, &c);
                    free_token (&token);
                  }
                  op->type = t_other;
                  last_non_comment_line = line_number;
                  return;
                }

              default:
                /* Invalid input.  Be tolerant, no error message.  */
                op->type = t_other;
                last_non_comment_line = line_number;
                return;
              }

            /*NOTREACHED*/
            abort ();
          }

        default:
          /* Read a token.  */
          {
            bool symbol;

            op->token = XMALLOC (struct token);
            symbol = read_token (op->token, &ch);
            if (op->token->charcount == 1 && op->token->chars[0] == '.')
              {
                free_token (op->token);
                free (op->token);
                op->type = t_dot;
                last_non_comment_line = line_number;
                return;
              }
            if (!symbol)
              {
                free_token (op->token);
                free (op->token);
                op->type = t_other;
                last_non_comment_line = line_number;
                return;
              }
            /* Distinguish between "foo" and "foo#bar".  */
            {
              int c = do_getc ();
              if (c == '#')
                {
                  struct token second_token;

                  free_token (op->token);
                  free (op->token);
                  read_token (&second_token, NULL);
                  free_token (&second_token);
                  op->type = t_other;
                  last_non_comment_line = line_number;
                  return;
                }
              else
                {
                  if (c != EOF)
                    do_ungetc (c);
                  op->type = t_symbol;
                  last_non_comment_line = line_number;
                  return;
                }
            }
          }
        }
    }
}


void
extract_librep (FILE *f,
                const char *real_filename, const char *logical_filename,
                flag_context_list_table_ty *flag_table,
                msgdomain_list_ty *mdlp)
{
  mlp = mdlp->item[0]->messages;

  fp = f;
  real_file_name = real_filename;
  logical_file_name = xstrdup (logical_filename);
  line_number = 1;

  last_comment_line = -1;
  last_non_comment_line = -1;

  flag_context_list_table = flag_table;
  nesting_depth = 0;

  init_keywords ();

  /* Eat tokens until eof is seen.  When read_object returns
     due to an unbalanced closing parenthesis, just restart it.  */
  do
    {
      struct object toplevel_object;

      read_object (&toplevel_object, null_context);

      if (toplevel_object.type == t_eof)
        break;

      free_object (&toplevel_object);
    }
  while (!feof (fp));

  /* Close scanner.  */
  fp = NULL;
  real_file_name = NULL;
  logical_file_name = NULL;
  line_number = 0;
}
