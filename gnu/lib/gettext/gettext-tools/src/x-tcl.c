/* xgettext Tcl backend.
   Copyright (C) 2002-2003, 2005-2009, 2013, 2018-2023 Free Software Foundation, Inc.

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
#include "x-tcl.h"

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "attribute.h"
#include "message.h"
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
#include "mem-hash-map.h"
#include "c-ctype.h"
#include "po-charset.h"
#include "unistr.h"
#include "gettext.h"

#define _(s) gettext(s)

#define SIZEOF(a) (sizeof(a) / sizeof(a[0]))


/* The Tcl syntax is defined in the Tcl.n manual page, see
   https://www.tcl-lang.org/man/tcl8.6/TclCmd/Tcl.htm .
   Summary of Tcl syntax:
   Like sh syntax, except that `...` is replaced with [...]. In detail:
   - In a preprocessing pass, backslash-newline-anywhitespace is replaced
     with single space.
   - Input is broken into words, which are then subject to command
     substitution [...] , variable substitution $var, backslash substitution
     \escape.
   - Strings are enclosed in "..."; command substitution, variable
     substitution and backslash substitutions are performed here as well.
   - {...} is a string without substitutions.
   - The list of resulting words is split into commands by semicolon and
     newline.
   - '#' at the beginning of a command introduces a comment until end of line.
   The parser is implemented in tcl8.6/generic/tclParse.c.  */


/* ====================== Keyword set customization.  ====================== */

/* If true extract all strings.  */
static bool extract_all = false;

static hash_table keywords;
static bool default_keywords = true;


void
x_tcl_extract_all ()
{
  extract_all = true;
}


void
x_tcl_keyword (const char *name)
{
  if (name == NULL)
    default_keywords = false;
  else
    {
      const char *end;
      struct callshape shape;

      if (keywords.table == NULL)
        hash_init (&keywords, 100);

      split_keywordspec (name, &end, &shape);

      /* The characters between name and end should form a valid Tcl
         function name.  A leading "::" is redundant.  */
      if (end - name >= 2 && name[0] == ':' && name[1] == ':')
        name += 2;

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
      x_tcl_keyword ("::msgcat::mc");
      default_keywords = false;
    }
}

void
init_flag_table_tcl ()
{
  xgettext_record_flag ("::msgcat::mc:1:pass-tcl-format");
  xgettext_record_flag ("format:1:tcl-format");
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


/* Combine backslash followed by newline and additional whitespace to
   a single space.  */

/* An int that becomes a space when casted to 'unsigned char'.  */
#define BS_NL (UCHAR_MAX + 1 + ' ')

static int phase1_pushback[5];
static int phase1_pushback_length;

static int
phase1_getc ()
{
  int c;

  if (phase1_pushback_length)
    {
      c = phase1_pushback[--phase1_pushback_length];
      if (c == '\n' || c == BS_NL)
        ++line_number;
      return c;
    }
  c = do_getc ();
  if (c != '\\')
    return c;
  c = do_getc ();
  if (c != '\n')
    {
      if (c != EOF)
        do_ungetc (c);
      return '\\';
    }
  for (;;)
    {
      c = do_getc ();
      if (!(c == ' ' || c == '\t'))
        break;
    }
  if (c != EOF)
    do_ungetc (c);
  return BS_NL;
}

/* Supports only one pushback character.  */
static void
phase1_ungetc (int c)
{
  switch (c)
    {
    case EOF:
      break;

    case '\n':
    case BS_NL:
      --line_number;
      FALLTHROUGH;

    default:
      if (phase1_pushback_length == SIZEOF (phase1_pushback))
        abort ();
      phase1_pushback[phase1_pushback_length++] = c;
      break;
    }
}


/* Keep track of brace nesting depth.
   When a word starts with an opening brace, a character group begins that
   ends with the corresponding closing brace.  In theory these character
   groups are string literals, but they are used by so many Tcl primitives
   (proc, if, ...) as representing command lists, that we treat them as
   command lists.  */

/* An int that becomes a closing brace when casted to 'unsigned char'.  */
#define CL_BRACE (UCHAR_MAX + 1 + '}')

static int phase2_pushback[2];
static int phase2_pushback_length;

/* Brace nesting depth inside the current character group.  */
static int brace_depth;

static int
phase2_push ()
{
  int previous_depth = brace_depth;
  brace_depth = 1;
  return previous_depth;
}

static void
phase2_pop (int previous_depth)
{
  brace_depth = previous_depth;
}

static int
phase2_getc ()
{
  int c;

  if (phase2_pushback_length)
    {
      c = phase2_pushback[--phase2_pushback_length];
      if (c == '\n' || c == BS_NL)
        ++line_number;
      else if (c == '{')
        ++brace_depth;
      else if (c == '}')
        --brace_depth;
      return c;
    }
  c = phase1_getc ();
  if (c == '{')
    ++brace_depth;
  else if (c == '}')
    {
      if (--brace_depth == 0)
        c = CL_BRACE;
    }
  return c;
}

/* Supports 2 characters of pushback.  */
static void
phase2_ungetc (int c)
{
  if (c != EOF)
    {
      switch (c)
        {
        case '\n':
        case BS_NL:
          --line_number;
          break;

        case '{':
          --brace_depth;
          break;

        case '}':
          ++brace_depth;
          break;
        }
      if (phase2_pushback_length == SIZEOF (phase2_pushback))
        abort ();
      phase2_pushback[phase2_pushback_length++] = c;
    }
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


/* ========================= Accumulating messages ========================= */


static message_list_ty *mlp;


/* ========================== Reading of commands ========================== */


/* We are only interested in constant strings (e.g. "msgcat::mc" or other
   string literals).  Other words need not to be represented precisely.  */
enum word_type
{
  t_string,     /* constant string */
  t_other,      /* other string */
  t_separator,  /* command separator: semicolon or newline */
  t_bracket,    /* ']' pseudo word */
  t_brace,      /* '}' pseudo word */
  t_eof         /* EOF marker */
};

struct word
{
  enum word_type type;
  struct token *token;          /* for t_string */
  int line_number_at_start;     /* for t_string */
};

/* Free the memory pointed to by a 'struct word'.  */
static inline void
free_word (struct word *wp)
{
  if (wp->type == t_string)
    {
      free_token (wp->token);
      free (wp->token);
    }
}

/* Convert a t_string token to a char*.  */
static char *
string_of_word (const struct word *wp)
{
  char *str;
  int n;

  if (!(wp->type == t_string))
    abort ();
  n = wp->token->charcount;
  str = XNMALLOC (n + 1, char);
  memcpy (str, wp->token->chars, n);
  str[n] = '\0';
  return str;
}


/* Context lookup table.  */
static flag_context_list_table_ty *flag_context_list_table;


/* Maximum supported nesting depth.  */
#define MAX_NESTING_DEPTH 1000

/* Current nesting depths.  */
static int bracket_nesting_depth;
static int brace_nesting_depth;


/* Read an escape sequence.  The value is an ISO-8859-1 character (in the
   range 0x00..0xff) or a Unicode character (in the range 0x0000..0x10FFFF).  */
static int
do_getc_escaped ()
{
  int c;

  c = phase1_getc ();
  switch (c)
    {
    case EOF:
      return '\\';
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
    case 'x':
      {
        unsigned int n = 0;
        unsigned int i;

        for (i = 0; i < 2; i++)
          {
            c = phase1_getc ();
            if (c == EOF || !c_isxdigit ((unsigned char) c))
              {
                phase1_ungetc (c);
                break;
              }

            if (c >= '0' && c <= '9')
              n = (n << 4) + (c - '0');
            else if (c >= 'A' && c <= 'F')
              n = (n << 4) + (c - 'A' + 10);
            else if (c >= 'a' && c <= 'f')
              n = (n << 4) + (c - 'a' + 10);
          }
        return (i > 0 ? (unsigned char) n : 'x');
      }
    case 'u':
      {
        unsigned int n = 0;
        unsigned int i;

        for (i = 0; i < 4; i++)
          {
            c = phase1_getc ();
            if (c == EOF || !c_isxdigit ((unsigned char) c))
              {
                phase1_ungetc (c);
                break;
              }

            if (c >= '0' && c <= '9')
              n = (n << 4) + (c - '0');
            else if (c >= 'A' && c <= 'F')
              n = (n << 4) + (c - 'A' + 10);
            else if (c >= 'a' && c <= 'f')
              n = (n << 4) + (c - 'a' + 10);
          }
        return (i > 0 ? n : 'u');
      }
    case 'U':
      {
        unsigned int n = 0;
        unsigned int i;

        for (i = 0; i < 8; i++)
          {
            c = phase1_getc ();
            if (c == EOF || !c_isxdigit ((unsigned char) c) || n >= 0x11000)
              {
                phase1_ungetc (c);
                break;
              }

            if (c >= '0' && c <= '9')
              n = (n << 4) + (c - '0');
            else if (c >= 'A' && c <= 'F')
              n = (n << 4) + (c - 'A' + 10);
            else if (c >= 'a' && c <= 'f')
              n = (n << 4) + (c - 'a' + 10);
          }
        return (i > 0 ? n : 'u');
      }
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
    default:
      /* Note: If c is non-ASCII, Tcl's behaviour is undefined here.  */
      return (unsigned char) c;
    }
}

/* Read an escape sequence for a low surrogate Unicode character.
   The value is in the range 0xDC00..0xDFFF.
   Return -1 when none was seen.  */
static int
do_getc_escaped_low_surrogate ()
{
  int c;

  c = phase1_getc ();
  switch (c)
    {
    case 'u':
      {
        unsigned char buf[4];
        unsigned int n = 0;
        unsigned int i;

        for (i = 0; i < 4; i++)
          {
            c = phase1_getc ();
            if (c == EOF || !c_isxdigit ((unsigned char) c))
              {
                phase1_ungetc (c);
                while (i > 0)
                  phase1_ungetc (buf[--i]);
                phase1_ungetc ('u');
                return -1;
              }
            buf[i] = c;

            if (c >= '0' && c <= '9')
              n = (n << 4) + (c - '0');
            else if (c >= 'A' && c <= 'F')
              n = (n << 4) + (c - 'A' + 10);
            else if (c >= 'a' && c <= 'f')
              n = (n << 4) + (c - 'a' + 10);
          }
        if (n >= 0xdc00 && n <= 0xdfff)
          return n;
        else
          {
            while (i > 0)
              phase1_ungetc (buf[--i]);
            phase1_ungetc ('u');
            return -1;
          }
      }
    default:
      phase1_ungetc (c);
      return -1;
    }
}


enum terminator
{
  te_space_separator,           /* looking for space semicolon newline */
  te_space_separator_bracket,   /* looking for space semicolon newline ']' */
  te_paren,                     /* looking for ')' */
  te_quote                      /* looking for '"' */
};

/* Forward declaration of local functions.  */
static enum word_type read_command_list (int looking_for,
                                         flag_context_ty outer_context);

/* Accumulate tokens into the given word.
   'looking_for' denotes a parse terminator combination.
   Return the first character past the token.  */
static int
accumulate_word (struct word *wp, enum terminator looking_for,
                 flag_context_ty context)
{
  int c;

  for (;;)
    {
      c = phase2_getc ();

      if (c == EOF || c == CL_BRACE)
        return c;
      if ((looking_for == te_space_separator
           || looking_for == te_space_separator_bracket)
          && (c == ' ' || c == BS_NL
              || c == '\t' || c == '\v' || c == '\f' || c == '\r'
              || c == ';' || c == '\n'))
        return c;
      if (looking_for == te_space_separator_bracket && c == ']')
        return c;
      if (looking_for == te_paren && c == ')')
        return c;
      if (looking_for == te_quote && c == '"')
        return c;

      if (c == '$')
        {
          /* Distinguish $varname, ${varname} and lone $.  */
          c = phase2_getc ();
          if (c == '{')
            {
              /* ${varname} */
              do
                c = phase2_getc ();
              while (c != EOF && c != '}');
              wp->type = t_other;
            }
          else
            {
              bool nonempty = false;

              for (; c != EOF && c != CL_BRACE; c = phase2_getc ())
                {
                  if (c_isalnum ((unsigned char) c) || (c == '_'))
                    {
                      nonempty = true;
                      continue;
                    }
                  if (c == ':')
                    {
                      c = phase2_getc ();
                      if (c == ':')
                        {
                          do
                            c = phase2_getc ();
                          while (c == ':');

                          phase2_ungetc (c);
                          nonempty = true;
                          continue;
                        }
                      phase2_ungetc (c);
                      c = ':';
                    }
                  break;
                }
              if (c == '(')
                {
                  /* $varname(index) */
                  struct word index_word;

                  index_word.type = t_other;
                  c = accumulate_word (&index_word, te_paren, null_context);
                  if (c != EOF && c != ')')
                    phase2_ungetc (c);
                  wp->type = t_other;
                }
              else
                {
                  phase2_ungetc (c);
                  if (nonempty)
                    {
                      /* $varname */
                      wp->type = t_other;
                    }
                  else
                    {
                      /* lone $ */
                      if (wp->type == t_string)
                        {
                          grow_token (wp->token);
                          wp->token->chars[wp->token->charcount++] = '$';
                        }
                    }
                }
            }
        }
      else if (c == '[')
        {
          if (++bracket_nesting_depth > MAX_NESTING_DEPTH)
            {
              error_with_progname = false;
              error (EXIT_FAILURE, 0, _("%s:%d: error: too many open brackets"),
                     logical_file_name, line_number);
            }
          read_command_list (']', context);
          bracket_nesting_depth--;
          wp->type = t_other;
        }
      else if (c == '\\')
        {
          unsigned int uc = do_getc_escaped ();
          assert (uc < 0x110000);
          if (uc >= 0xd800 && uc <= 0xdfff)
            {
              if (uc < 0xdc00)
                {
                  /* Saw a high surrogate Unicode character.
                     Is it followed by a low surrogate Unicode character?  */
                  c = phase2_getc ();
                  if (c == '\\')
                    {
                      int uc2 = do_getc_escaped_low_surrogate ();
                      if (uc2 >= 0)
                        {
                          /* Saw a low surrogate Unicode character.  */
                          assert (uc2 >= 0xdc00 && uc2 <= 0xdfff);
                          uc = 0x10000 + ((uc - 0xd800) << 10) + (uc2 - 0xdc00);
                          goto saw_unicode_escape;
                        }
                    }
                  phase2_ungetc (c);
                }
              error_with_progname = false;
              error (0, 0, _("%s:%d: warning: invalid Unicode character"),
                     logical_file_name, line_number);
              error_with_progname = true;
              goto done_escape;
            }
         saw_unicode_escape:
          {
            unsigned char utf8buf[6];
            int count = u8_uctomb (utf8buf, uc, 6);
            int i;
            assert (count > 0);
            if (wp->type == t_string)
              for (i = 0; i < count; i++)
                {
                  grow_token (wp->token);
                  wp->token->chars[wp->token->charcount++] = utf8buf[i];
                }
          }
         done_escape: ;
        }
      else
        {
          if (wp->type == t_string)
            {
              grow_token (wp->token);
              wp->token->chars[wp->token->charcount++] = (unsigned char) c;
            }
        }
    }
}


/* Read the next word.
   'looking_for' denotes a parse terminator, either ']' or '\0'.  */
static void
read_word (struct word *wp, int looking_for, flag_context_ty context)
{
  int c;

  do
    c = phase2_getc ();
  while (c == ' ' || c == BS_NL
         || c == '\t' || c == '\v' || c == '\f' || c == '\r');

  if (c == EOF)
    {
      wp->type = t_eof;
      return;
    }

  if (c == CL_BRACE)
    {
      wp->type = t_brace;
      last_non_comment_line = line_number;
      return;
    }

  if (c == '\n')
    {
      /* Comments assumed to be grouped with a message must immediately
         precede it, with no non-whitespace token on a line between both.  */
      if (last_non_comment_line > last_comment_line)
        savable_comment_reset ();
      wp->type = t_separator;
      return;
    }

  if (c == ';')
    {
      wp->type = t_separator;
      last_non_comment_line = line_number;
      return;
    }

  if (looking_for == ']' && c == ']')
    {
      wp->type = t_bracket;
      last_non_comment_line = line_number;
      return;
    }

  if (c == '{')
    {
      int previous_depth;
      enum word_type terminator;

      /* Start a new nested character group, which lasts until the next
         balanced '}' (ignoring \} things).  */
      previous_depth = phase2_push () - 1;

      /* Interpret it as a command list.  */
      if (++brace_nesting_depth > MAX_NESTING_DEPTH)
        {
          error_with_progname = false;
          error (EXIT_FAILURE, 0, _("%s:%d: error: too many open braces"),
                 logical_file_name, line_number);
        }
      terminator = read_command_list ('\0', null_context);
      brace_nesting_depth--;

      if (terminator == t_brace)
        phase2_pop (previous_depth);

      wp->type = t_other;
      last_non_comment_line = line_number;
      return;
    }

  wp->type = t_string;
  wp->token = XMALLOC (struct token);
  init_token (wp->token);
  wp->line_number_at_start = line_number;

  if (c == '"')
    {
      c = accumulate_word (wp, te_quote, context);
      if (c != EOF && c != '"')
        phase2_ungetc (c);
    }
  else
    {
      phase2_ungetc (c);
      c = accumulate_word (wp,
                           looking_for == ']'
                           ? te_space_separator_bracket
                           : te_space_separator,
                           context);
      if (c != EOF)
        phase2_ungetc (c);
    }

  if (wp->type != t_string)
    {
      free_token (wp->token);
      free (wp->token);
    }
  last_non_comment_line = line_number;
}


/* Read the next command.
   'looking_for' denotes a parse terminator, either ']' or '\0'.
   Returns the type of the word that terminated the command: t_separator or
   t_bracket (only if looking_for is ']') or t_brace or t_eof.  */
static enum word_type
read_command (int looking_for, flag_context_ty outer_context)
{
  int c;

  /* Skip whitespace and comments.  */
  for (;;)
    {
      c = phase2_getc ();

      if (c == ' ' || c == BS_NL
          || c == '\t' || c == '\v' || c == '\f' || c == '\r')
        continue;
      if (c == '#')
        {
          /* Skip a comment up to end of line.  */
          last_comment_line = line_number;
          comment_start ();
          for (;;)
            {
              c = phase2_getc ();
              if (c == EOF || c == CL_BRACE || c == '\n')
                break;
              /* We skip all leading white space, but not EOLs.  */
              if (!(buflen == 0 && (c == ' ' || c == '\t')))
                comment_add (c);
            }
          comment_line_end ();
          continue;
        }
      break;
    }
  phase2_ungetc (c);

  /* Read the words that make up the command.  */
  {
    int arg = 0;                /* Current argument number.  */
    flag_context_list_iterator_ty context_iter;
    const struct callshapes *shapes = NULL;
    struct arglist_parser *argparser = NULL;

    for (;; arg++)
      {
        struct word inner;
        flag_context_ty inner_context;

        if (arg == 0)
          inner_context = null_context;
        else
          inner_context =
            inherited_context (outer_context,
                               flag_context_list_iterator_advance (
                                 &context_iter));

        read_word (&inner, looking_for, inner_context);

        /* Recognize end of command.  */
        if (inner.type == t_separator || inner.type == t_bracket
            || inner.type == t_brace || inner.type == t_eof)
          {
            if (argparser != NULL)
              arglist_parser_done (argparser, arg);
            return inner.type;
          }

        if (extract_all)
          {
            if (inner.type == t_string)
              {
                lex_pos_ty pos;

                pos.file_name = logical_file_name;
                pos.line_number = inner.line_number_at_start;
                remember_a_message (mlp, NULL, string_of_word (&inner), false,
                                    false, inner_context, &pos,
                                    NULL, savable_comment, false);
              }
          }

        if (arg == 0)
          {
            /* This is the function position.  */
            if (inner.type == t_string)
              {
                char *function_name = string_of_word (&inner);
                char *stripped_name;
                void *keyword_value;

                /* A leading "::" is redundant.  */
                stripped_name = function_name;
                if (function_name[0] == ':' && function_name[1] == ':')
                  stripped_name += 2;

                if (hash_find_entry (&keywords,
                                     stripped_name, strlen (stripped_name),
                                     &keyword_value)
                    == 0)
                  shapes = (const struct callshapes *) keyword_value;

                argparser = arglist_parser_alloc (mlp, shapes);

                context_iter =
                  flag_context_list_iterator (
                    flag_context_list_table_lookup (
                      flag_context_list_table,
                      stripped_name, strlen (stripped_name)));

                free (function_name);
              }
            else
              context_iter = null_context_list_iterator;
          }
        else
          {
            /* These are the argument positions.  */
            if (argparser != NULL && inner.type == t_string)
              {
                char *s = string_of_word (&inner);
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

        free_word (&inner);
      }
  }
}


/* Read a list of commands.
   'looking_for' denotes a parse terminator, either ']' or '\0'.
   Returns the type of the word that terminated the command list:
   t_bracket (only if looking_for is ']') or t_brace or t_eof.  */
static enum word_type
read_command_list (int looking_for, flag_context_ty outer_context)
{
  for (;;)
    {
      enum word_type terminator;

      terminator = read_command (looking_for, outer_context);
      if (terminator != t_separator)
        return terminator;
    }
}


void
extract_tcl (FILE *f,
             const char *real_filename, const char *logical_filename,
             flag_context_list_table_ty *flag_table,
             msgdomain_list_ty *mdlp)
{
  mlp = mdlp->item[0]->messages;

  /* We convert our strings to UTF-8 encoding.  */
  xgettext_current_source_encoding = po_charset_utf8;

  fp = f;
  real_file_name = real_filename;
  logical_file_name = xstrdup (logical_filename);
  line_number = 1;

  phase1_pushback_length = 0;
  phase2_pushback_length = 0;

  /* Initially, no brace is open.  */
  brace_depth = 1000000;

  last_comment_line = -1;
  last_non_comment_line = -1;

  flag_context_list_table = flag_table;
  bracket_nesting_depth = 0;
  brace_nesting_depth = 0;

  init_keywords ();

  /* Eat tokens until eof is seen.  */
  read_command_list ('\0', null_context);

  fp = NULL;
  real_file_name = NULL;
  logical_file_name = NULL;
  line_number = 0;
}
