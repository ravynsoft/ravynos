/* xgettext Emacs Lisp backend.
   Copyright (C) 2001-2003, 2005-2009, 2018-2023 Free Software Foundation, Inc.

   This file was written by Bruno Haible <haible@clisp.cons.org>, 2001-2002.

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
#include "x-elisp.h"

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
#include "mem-hash-map.h"
#include "c-ctype.h"
#include "gettext.h"

#define _(s) gettext(s)


/* Summary of Emacs Lisp syntax:
   - ';' starts a comment until end of line.
   - '#@nn' starts a comment of nn bytes.
   - Integers are constituted of an optional prefix (#b, #B for binary,
     #o, #O for octal, #x, #X for hexadecimal, #nnr, #nnR for any radix),
     an optional sign (+ or -), the digits, and an optional trailing dot.
   - Characters are written as '?' followed by the character, possibly
     with an escape sequence, for examples '?a', '?\n', '?\177'.
   - Strings are delimited by double quotes. Backslash introduces an escape
     sequence. The following are understood: '\n', '\r', '\f', '\t', '\a',
     '\\', '\^C', '\012' (octal), '\x12' (hexadecimal).
   - Symbols: can contain meta-characters if preceded by backslash.
   - Uninterned symbols: written as #:SYMBOL.
   - () delimit lists.
   - [] delimit vectors.
   The reader is implemented in emacs-21.1/src/lread.c.  */


/* ====================== Keyword set customization.  ====================== */

/* If true extract all strings.  */
static bool extract_all = false;

static hash_table keywords;
static bool default_keywords = true;


void
x_elisp_extract_all ()
{
  extract_all = true;
}


void
x_elisp_keyword (const char *name)
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
      x_elisp_keyword ("_");
      default_keywords = false;
    }
}

void
init_flag_table_elisp ()
{
  xgettext_record_flag ("_:1:pass-elisp-format");
  xgettext_record_flag ("format:1:elisp-format");
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

/* Test whether a token has integer syntax.  */
static inline bool
is_integer (const char *p)
{
  /* NB: Yes, '+.' and '-.' both designate the integer 0.  */
  const char *p_start = p;

  if (*p == '+' || *p == '-')
    p++;
  if (*p == '\0')
    return false;
  while (*p >= '0' && *p <= '9')
    p++;
  if (p > p_start && *p == '.')
    p++;
  return (*p == '\0');
}

/* Test whether a token has float syntax.  */
static inline bool
is_float (const char *p)
{
  enum { LEAD_INT = 1, DOT_CHAR = 2, TRAIL_INT = 4, E_CHAR = 8, EXP_INT = 16 };
  int state;

  state = 0;
  if (*p == '+' || *p == '-')
    p++;
  if (*p >= '0' && *p <= '9')
    {
      state |= LEAD_INT;
      do
        p++;
      while (*p >= '0' && *p <= '9');
    }
  if (*p == '.')
    {
      state |= DOT_CHAR;
      p++;
    }
  if (*p >= '0' && *p <= '9')
    {
      state |= TRAIL_INT;
      do
        p++;
      while (*p >= '0' && *p <= '9');
    }
  if (*p == 'e' || *p == 'E')
    {
      state |= E_CHAR;
      p++;
      if (*p == '+' || *p == '-')
        p++;
      if (*p >= '0' && *p <= '9')
        {
          state |= EXP_INT;
          do
            p++;
          while (*p >= '0' && *p <= '9');
        }
      else if (p[-1] == '+'
               && ((p[0] == 'I' && p[1] == 'N' && p[2] == 'F')
                   || (p[0] == 'N' && p[1] == 'a' && p[2] == 'N')))
        {
          state |= EXP_INT;
          p += 3;
        }
    }
  return (*p == '\0')
         && (state == (LEAD_INT | DOT_CHAR | TRAIL_INT)
             || state == (DOT_CHAR | TRAIL_INT)
             || state == (LEAD_INT | E_CHAR | EXP_INT)
             || state == (LEAD_INT | DOT_CHAR | TRAIL_INT | E_CHAR | EXP_INT)
             || state == (DOT_CHAR | TRAIL_INT | E_CHAR | EXP_INT));
}

/* Read the next token.  'first' is the first character, which has already
   been read.  Returns true for a symbol, false for a number.  */
static bool
read_token (struct token *tp, int first)
{
  int c;
  bool quoted = false;

  init_token (tp);

  c = first;

  for (;; c = do_getc ())
    {
      if (c == EOF)
        break;
      if (c <= ' ') /* FIXME: Assumes ASCII compatible encoding */
        break;
      if (c == '\"' || c == '\'' || c == ';' || c == '(' || c == ')'
          || c == '[' || c == ']' || c == '#')
        break;
      if (c == '\\')
        {
          quoted = true;
          c = do_getc ();
          if (c == EOF)
            /* Invalid, but be tolerant.  */
            break;
        }
      grow_token (tp);
      tp->chars[tp->charcount++] = c;
    }
  if (c != EOF)
    do_ungetc (c);

  if (quoted)
    return true; /* symbol */

  /* Add a NUL byte at the end, for is_integer and is_float.  */
  grow_token (tp);
  tp->chars[tp->charcount] = '\0';

  if (is_integer (tp->chars) || is_float (tp->chars))
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
  t_listclose,  /* ')' pseudo object */
  t_vectorclose,/* ']' pseudo object */
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

/* Current nesting depths.  */
static int escape_nesting_depth;
static int nesting_depth;


/* Returns the character represented by an escape sequence.  */
#define IGNORABLE_ESCAPE (EOF - 1)
static int
do_getc_escaped (int c, bool in_string)
{
  if (escape_nesting_depth > MAX_NESTING_DEPTH)
    {
      error_with_progname = false;
      error (EXIT_FAILURE, 0, _("%s:%d: error: too deeply nested escape sequence"),
             logical_file_name, line_number);
    }
  switch (c)
    {
    case 'a':
      return '\a';
    case 'b':
      return '\b';
    case 'd':
      return 0x7F;
    case 'e':
      return 0x1B;
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

    case '\n':
      return IGNORABLE_ESCAPE;

    case ' ':
      return (in_string ? IGNORABLE_ESCAPE : ' ');

    case 'M': /* meta */
      c = do_getc ();
      if (c == EOF)
        return EOF;
      if (c != '-')
        /* Invalid input.  But be tolerant.  */
        return c;
      c = do_getc ();
      if (c == EOF)
        return EOF;
      if (c == '\\')
        {
          c = do_getc ();
          if (c == EOF)
            return EOF;
          ++escape_nesting_depth;
          c = do_getc_escaped (c, false);
          escape_nesting_depth--;
        }
      return c | 0x80;

    case 'S': /* shift */
      c = do_getc ();
      if (c == EOF)
        return EOF;
      if (c != '-')
        /* Invalid input.  But be tolerant.  */
        return c;
      c = do_getc ();
      if (c == EOF)
        return EOF;
      if (c == '\\')
        {
          c = do_getc ();
          if (c == EOF)
            return EOF;
          ++escape_nesting_depth;
          c = do_getc_escaped (c, false);
          escape_nesting_depth--;
        }
      return (c >= 'a' && c <= 'z' ? c - 'a' + 'A' : c);

    case 'H': /* hyper */
    case 'A': /* alt */
    case 's': /* super */
      c = do_getc ();
      if (c == EOF)
        return EOF;
      if (c != '-')
        /* Invalid input.  But be tolerant.  */
        return c;
      c = do_getc ();
      if (c == EOF)
        return EOF;
      if (c == '\\')
        {
          c = do_getc ();
          if (c == EOF)
            return EOF;
          ++escape_nesting_depth;
          c = do_getc_escaped (c, false);
          escape_nesting_depth--;
        }
      return c;

    case 'C': /* ctrl */
      c = do_getc ();
      if (c == EOF)
        return EOF;
      if (c != '-')
        /* Invalid input.  But be tolerant.  */
        return c;
      FALLTHROUGH;
    case '^':
      c = do_getc ();
      if (c == EOF)
        return EOF;
      if (c == '\\')
        {
          c = do_getc ();
          if (c == EOF)
            return EOF;
          ++escape_nesting_depth;
          c = do_getc_escaped (c, false);
          escape_nesting_depth--;
        }
      if (c == '?')
        return 0x7F;
      if ((c & 0x5F) >= 0x41 && (c & 0x5F) <= 0x5A)
        return c & 0x9F;
      if ((c & 0x7F) >= 0x40 && (c & 0x7F) <= 0x5F)
        return c & 0x9F;
#if 0 /* We cannot handle NUL bytes in strings.  */
      if (c == ' ')
        return 0x00;
#endif
      return c;

    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7':
      /* An octal escape, as in ANSI C.  */
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
      /* A hexadecimal escape, as in ANSI C.  */
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
      /* Ignore Emacs multibyte character stuff.  All the strings we are
         interested in are ASCII strings.  */
      return c;
    }
}

/* Read the next object.
   'first_in_list' and 'new_backquote_flag' are used for reading old
   backquote syntax and new backquote syntax.  */
static void
read_object (struct object *op, bool first_in_list, bool new_backquote_flag,
             flag_context_ty outer_context)
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
                read_object (&inner, arg == 0, new_backquote_flag,
                             inner_context);
                nesting_depth--;

                /* Recognize end of list.  */
                if (inner.type == t_listclose)
                  {
                    op->type = t_other;
                    /* Don't bother converting "()" to "NIL".  */
                    last_non_comment_line = line_number;
                    if (argparser != NULL)
                      arglist_parser_done (argparser, arg);
                    return;
                  }

                /* Dots are not allowed in every position. ']' is not allowed.
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

        case ')':
          /* Tell the caller about the end of list.
             Unmatched closing parenthesis is illegal.  But be tolerant.  */
          op->type = t_listclose;
          last_non_comment_line = line_number;
          return;

        case '[':
          {
            for (;;)
              {
                struct object inner;

                ++nesting_depth;
                read_object (&inner, false, new_backquote_flag, null_context);
                nesting_depth--;

                /* Recognize end of vector.  */
                if (inner.type == t_vectorclose)
                  {
                    op->type = t_other;
                    last_non_comment_line = line_number;
                    return;
                  }

                /* Dots and ')' are not allowed.  But be tolerant.  */

                /* EOF inside vector is illegal.  But be tolerant.  */
                if (inner.type == t_eof)
                  break;

                free_object (&inner);
              }
          }
          op->type = t_other;
          last_non_comment_line = line_number;
          return;

        case ']':
          /* Tell the caller about the end of vector.
             Unmatched closing bracket is illegal.  But be tolerant.  */
          op->type = t_vectorclose;
          last_non_comment_line = line_number;
          return;

        case '\'':
          {
            struct object inner;

            ++nesting_depth;
            read_object (&inner, false, new_backquote_flag, null_context);
            nesting_depth--;

            /* Dots and EOF are not allowed here.  But be tolerant.  */

            free_object (&inner);

            op->type = t_other;
            last_non_comment_line = line_number;
            return;
          }

        case '`':
          if (first_in_list)
            goto default_label;
          {
            struct object inner;

            ++nesting_depth;
            read_object (&inner, false, true, null_context);
            nesting_depth--;

            /* Dots and EOF are not allowed here.  But be tolerant.  */

            free_object (&inner);

            op->type = t_other;
            last_non_comment_line = line_number;
            return;
          }

        case ',':
          if (!new_backquote_flag)
            goto default_label;
          {
            int c = do_getc ();
            /* The ,@ handling inside lists is wrong anyway, because
               ,@form expands to an unknown number of elements.  */
            if (c != EOF && c != '@' && c != '.')
              do_ungetc (c);
          }
          {
            struct object inner;

            ++nesting_depth;
            read_object (&inner, false, false, null_context);
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
                if (c == EOF || c == '\n')
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
                    c = do_getc_escaped (c, true);
                    if (c == EOF)
                      /* Invalid input.  Be tolerant, no error message.  */
                      break;
                    if (c == IGNORABLE_ESCAPE)
                      /* Ignore escaped newline and escaped space.  */
                      ;
                    else
                      {
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
                    c = do_getc_escaped (c, false);
                    if (c == EOF)
                      /* Invalid input.  Be tolerant, no error message.  */
                      ;
                  }
              }
            /* Impossible to deal with Emacs multibyte character stuff here.  */
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
              case '^':
                {
                  int c = do_getc ();
                  if (c == '^')
                    c = do_getc ();
                  if (c == '[')
                    {
                      /* Read a char table, same syntax as a vector.  */
                      for (;;)
                        {
                          struct object inner;

                          ++nesting_depth;
                          read_object (&inner, false, new_backquote_flag,
                                       null_context);
                          nesting_depth--;

                          /* Recognize end of vector.  */
                          if (inner.type == t_vectorclose)
                            {
                              op->type = t_other;
                              last_non_comment_line = line_number;
                              return;
                            }

                          /* Dots and ')' are not allowed.  But be tolerant.  */

                          /* EOF inside vector is illegal.  But be tolerant.  */
                          if (inner.type == t_eof)
                            break;

                          free_object (&inner);
                        }
                      op->type = t_other;
                      last_non_comment_line = line_number;
                      return;
                    }
                  else
                    /* Invalid input.  Be tolerant, no error message.  */
                    {
                      op->type = t_other;
                      if (c != EOF)
                        last_non_comment_line = line_number;
                      return;
                    }
                }

              case '&':
                /* Read a bit vector.  */
                {
                  struct object length;
                  ++nesting_depth;
                  read_object (&length, first_in_list, new_backquote_flag,
                               null_context);
                  nesting_depth--;
                  /* Dots and EOF are not allowed here.
                     But be tolerant.  */
                  free_object (&length);
                }
                {
                  int c = do_getc ();
                  if (c == '"')
                    {
                      struct object string;
                      ++nesting_depth;
                      read_object (&string, first_in_list, new_backquote_flag,
                                   null_context);
                      nesting_depth--;
                      free_object (&string);
                    }
                  else
                    /* Invalid input.  Be tolerant, no error message.  */
                    do_ungetc (c);
                }
                op->type = t_other;
                last_non_comment_line = line_number;
                return;

              case '[':
                /* Read a compiled function, same syntax as a vector.  */
              case '(':
                /* Read a string with properties, same syntax as a list.  */
                {
                  struct object inner;
                  do_ungetc (dmc);
                  ++nesting_depth;
                  read_object (&inner, false, new_backquote_flag, null_context);
                  nesting_depth--;
                  /* Dots and EOF are not allowed here.
                     But be tolerant.  */
                  free_object (&inner);
                  op->type = t_other;
                  last_non_comment_line = line_number;
                  return;
                }

              case '@':
                /* Read a comment consisting of a given number of bytes.  */
                {
                  unsigned int nskip = 0;
                  int c;

                  for (;;)
                    {
                      c = do_getc ();
                      if (!(c >= '0' && c <= '9'))
                        break;
                      nskip = 10 * nskip + (c - '0');
                    }
                  if (c != EOF)
                    {
                      do_ungetc (c);
                      for (; nskip > 0; nskip--)
                        if (do_getc () == EOF)
                          break;
                    }
                  continue;
                }

              case '$':
                op->type = t_other;
                last_non_comment_line = line_number;
                return;

              case '\'':
              case ':':
              case 'S': case 's': /* XEmacs only */
                {
                  struct object inner;
                  ++nesting_depth;
                  read_object (&inner, false, new_backquote_flag, null_context);
                  nesting_depth--;
                  /* Dots and EOF are not allowed here.
                     But be tolerant.  */
                  free_object (&inner);
                  op->type = t_other;
                  last_non_comment_line = line_number;
                  return;
                }

              case '0': case '1': case '2': case '3': case '4':
              case '5': case '6': case '7': case '8': case '9':
                /* Read Common Lisp style #n# or #n=.  */
                {
                  int c;

                  for (;;)
                    {
                      c = do_getc ();
                      if (!(c >= '0' && c <= '9'))
                        break;
                    }
                  if (c == EOF)
                    /* Invalid input.  Be tolerant, no error message.  */
                    {
                      op->type = t_other;
                      return;
                    }
                  if (c == '=')
                    {
                      ++nesting_depth;
                      read_object (op, false, new_backquote_flag, outer_context);
                      nesting_depth--;
                      last_non_comment_line = line_number;
                      return;
                    }
                  if (c == '#')
                    {
                      op->type = t_other;
                      last_non_comment_line = line_number;
                      return;
                    }
                  if (c == 'R' || c == 'r')
                    {
                      /* Read an integer.  */
                      c = do_getc ();
                      if (c == '+' || c == '-')
                        c = do_getc ();
                      for (; c != EOF; c = do_getc ())
                        if (!c_isalnum (c))
                          {
                            do_ungetc (c);
                            break;
                          }
                      op->type = t_other;
                      last_non_comment_line = line_number;
                      return;
                    }
                  /* Invalid input.  Be tolerant, no error message.  */
                  op->type = t_other;
                  last_non_comment_line = line_number;
                  return;
                }

              case 'X': case 'x':
              case 'O': case 'o':
              case 'B': case 'b':
                {
                  /* Read an integer.  */
                  int c;

                  c = do_getc ();
                  if (c == '+' || c == '-')
                    c = do_getc ();
                  for (; c != EOF; c = do_getc ())
                    if (!c_isalnum (c))
                      {
                        do_ungetc (c);
                        break;
                      }
                  op->type = t_other;
                  last_non_comment_line = line_number;
                  return;
                }

              case '*': /* XEmacs only */
                {
                  /* Read a bit-vector.  */
                  int c;

                  do
                    c = do_getc ();
                  while (c == '0' || c == '1');
                  if (c != EOF)
                    do_ungetc (c);
                  op->type = t_other;
                  last_non_comment_line = line_number;
                  return;
                }

              case '+': /* XEmacs only */
              case '-': /* XEmacs only */
                /* Simply assume every feature expression is true.  */
                {
                  struct object inner;
                  ++nesting_depth;
                  read_object (&inner, false, new_backquote_flag, null_context);
                  nesting_depth--;
                  /* Dots and EOF are not allowed here.
                     But be tolerant.  */
                  free_object (&inner);
                  continue;
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

        case '.':
          ch = do_getc ();
          if (ch != EOF)
            {
              do_ungetc (ch);
              if (ch <= ' ' /* FIXME: Assumes ASCII compatible encoding */
                  || strchr ("\"'`,(", ch) != NULL)
                {
                  op->type = t_dot;
                  last_non_comment_line = line_number;
                  return;
                }
            }
          ch = '.';
          FALLTHROUGH;
        default:
        default_label:
          if (ch <= ' ') /* FIXME: Assumes ASCII compatible encoding */
            continue;
          /* Read a token.  */
          {
            bool symbol;

            op->token = XMALLOC (struct token);
            symbol = read_token (op->token, ch);
            if (symbol)
              {
                op->type = t_symbol;
                last_non_comment_line = line_number;
                return;
              }
            else
              {
                free_token (op->token);
                free (op->token);
                op->type = t_other;
                last_non_comment_line = line_number;
                return;
              }
          }
        }
    }
}


void
extract_elisp (FILE *f,
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
  escape_nesting_depth = 0;
  nesting_depth = 0;

  init_keywords ();

  /* Eat tokens until eof is seen.  When read_object returns
     due to an unbalanced closing parenthesis, just restart it.  */
  do
    {
      struct object toplevel_object;

      read_object (&toplevel_object, false, false, null_context);

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
