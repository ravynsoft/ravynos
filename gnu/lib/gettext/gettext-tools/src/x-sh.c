/* xgettext sh backend.
   Copyright (C) 2003, 2005-2009, 2014, 2018-2023 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2003.

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
#include "x-sh.h"

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
#include "xg-mixed-string.h"
#include "xg-arglist-context.h"
#include "xg-arglist-callshape.h"
#include "xg-arglist-parser.h"
#include "xg-message.h"
#include "error.h"
#include "error-progname.h"
#include "xalloc.h"
#include "mem-hash-map.h"
#include "../../gettext-runtime/src/escapes.h"
#include "gettext.h"

#define _(s) gettext(s)

#define SIZEOF(a) (sizeof(a) / sizeof(a[0]))


/* The sh syntax is defined in POSIX:2001, see
     http://www.opengroup.org/onlinepubs/007904975/utilities/xcu_chap02.html
   Summary of sh syntax:
   - Input is broken into words, which are then subject to
     - tilde expansion ~...
     - command substitution `...`
     - variable substitution $var
     - arithmetic substitution $((...))
     - field splitting at whitespace (IFS)
     - wildcard pattern expansion *?
     - quote removal
   - Strings are enclosed in "..."; command substitution, variable
     substitution and arithmetic substitution are performed here as well.
   - '...' is a string without substitutions.
   - The list of resulting words is split into commands by semicolon and
     newline.
   - '#' at the beginning of a word introduces a comment until end of line.
   The parser is implemented in bash-2.05b/parse.y.  */


/* ====================== Keyword set customization.  ====================== */

/* If true extract all strings.  */
static bool extract_all = false;

static hash_table keywords;
static bool default_keywords = true;


void
x_sh_extract_all ()
{
  extract_all = true;
}


void
x_sh_keyword (const char *name)
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
      x_sh_keyword ("gettext");
      x_sh_keyword ("ngettext:1,2");
      /* Note: There is also special handling for 'gettext' and 'ngettext'
         in read_command, below.  */
      x_sh_keyword ("eval_gettext");
      x_sh_keyword ("eval_ngettext:1,2");
      x_sh_keyword ("eval_pgettext:1c,2");
      x_sh_keyword ("eval_npgettext:1c,2,3");
      default_keywords = false;
    }
}

void
init_flag_table_sh ()
{
  xgettext_record_flag ("gettext:1:pass-sh-format");
  xgettext_record_flag ("ngettext:1:pass-sh-format");
  xgettext_record_flag ("ngettext:2:pass-sh-format");
  xgettext_record_flag ("eval_gettext:1:sh-format");
  xgettext_record_flag ("eval_ngettext:1:sh-format");
  xgettext_record_flag ("eval_ngettext:2:sh-format");
  xgettext_record_flag ("eval_pgettext:2:sh-format");
  xgettext_record_flag ("eval_npgettext:2:sh-format");
  xgettext_record_flag ("eval_npgettext:3:sh-format");
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


/* Remove backslash followed by newline from the input stream.  */

static int phase1_pushback[2];
static int phase1_pushback_length;

static int
phase1_getc ()
{
  int c;

  if (phase1_pushback_length)
    {
      c = phase1_pushback[--phase1_pushback_length];
      if (c == '\n')
        ++line_number;
      return c;
    }
  for (;;)
    {
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
    }
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
      --line_number;
      FALLTHROUGH;

    default:
      if (phase1_pushback_length == SIZEOF (phase1_pushback))
        abort ();
      phase1_pushback[phase1_pushback_length++] = c;
      break;
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

/* Convert a struct token * to a char*.  */
static char *
string_of_token (const struct token *tp)
{
  char *str;
  int n;

  n = tp->charcount;
  str = XNMALLOC (n + 1, char);
  memcpy (str, tp->chars, n);
  str[n] = '\0';
  return str;
}


/* ========================= Accumulating messages ========================= */


static message_list_ty *mlp;


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


/* ========================= Debackslashification ========================== */

/* This state tracks the effect of backquotes, double-quotes and single-quotes
   on the parsing of backslashes.  We make a single pass through the input
   file, keeping the state up to date.  This is much faster than accumulating
   strings and processing them with explicit debackslashification, like the
   shell does it.  */

/* The number of nested `...` or "`...`" constructs.  Assumed to be <= 32.  */
static unsigned int nested_backquotes;

/* A bit mask indicating which of the currently open `...` or "`...`"
   constructs is with double-quotes: "`...`".
   A bit value of 1 stands for "`...`", a bit value of 0 stands for `...`.
   Bit position 0 designates the outermost backquotes nesting,
   bit position 1 the second-outermost backquotes nesting,
   ...
   bit position (nested_backquotes-1) the innermost backquotes nesting.  */
static unsigned int open_doublequotes_mask;

/* A bit indicating whether a double-quote is currently open inside the
   innermost backquotes nesting.  */
static bool open_doublequote;

/* A bit indicating whether a single-quote is currently open inside the
   innermost backquotes nesting.  */
static bool open_singlequote;

/* The expected terminator of the currently open single-quote.
   Usually '\'', but can be '"' for i18n-quotes.  */
static char open_singlequote_terminator;


/* Functions to update the state.  */

static inline void
saw_opening_backquote ()
{
  if (open_singlequote)
    abort ();
  if (open_doublequote)
    open_doublequotes_mask |= (unsigned int) 1 << nested_backquotes;
  nested_backquotes++;
  open_doublequote = false;
}

static inline void
saw_closing_backquote ()
{
  nested_backquotes--;
  open_doublequote = (open_doublequotes_mask >> nested_backquotes) & 1;
  open_doublequotes_mask &= ((unsigned int) 1 << nested_backquotes) - 1;
  open_singlequote = false; /* just for safety */
}

static inline void
saw_opening_doublequote ()
{
  if (open_singlequote || open_doublequote)
    abort ();
  open_doublequote = true;
}

static inline void
saw_closing_doublequote ()
{
  if (open_singlequote || !open_doublequote)
    abort ();
  open_doublequote = false;
}

static inline void
saw_opening_singlequote ()
{
  if (open_doublequote || open_singlequote)
    abort ();
  open_singlequote = true;
  open_singlequote_terminator = '\'';
}

static inline void
saw_closing_singlequote ()
{
  if (open_doublequote || !open_singlequote)
    abort ();
  open_singlequote = false;
}


/* ========================== Reading of commands ========================== */

/* We are only interested in constant strings.  Other words need not to be
   represented precisely.  */
enum word_type
{
  t_string,     /* constant string */
  t_assignment, /* variable assignment */
  t_other,      /* other string */
  t_separator,  /* command separator: semicolon or newline */
  t_redirect,   /* redirection: one of < > >| << <<- >> <> <& >& */
  t_backquote,  /* closing '`' pseudo word */
  t_paren,      /* closing ')' pseudo word */
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

/* Convert a t_string token to a char*, ignoring the first OFFSET bytes.  */
static char *
substring_of_word (const struct word *wp, size_t offset)
{
  char *str;
  int n;

  if (!(wp->type == t_string))
    abort ();
  n = wp->token->charcount;
  if (!(offset <= n))
    abort ();
  str = XNMALLOC (n - offset + 1, char);
  memcpy (str, wp->token->chars + offset, n - offset);
  str[n - offset] = '\0';
  return str;
}


/* Whitespace recognition.  */

static inline bool
is_whitespace (int c)
{
  return (c == ' ' || c == '\t' || c == '\n');
}

/* Operator character recognition.  */

static inline bool
is_operator_start (int c)
{
  return (c == '|' || c == '&' || c == ';' || c == '<' || c == '>'
          || c == '(' || c == ')');
}


/* Denotation of a quoted character.
   The distinction between quoted and unquoted character is important only for
   the special, whitespace and operator characters; it is irrelevant for
   alphanumeric characters, '\\' and many others.  */
#define QUOTED(c) (UCHAR_MAX + 1 + (c))
/* Values in the 'unsigned char' range are implicitly unquoted.  Among these,
   the following are important:
     '"'         opening or closing double quote
     '\''        opening or closing single quote
     '$'         the unknown result of a dollar expansion
     '`'         does not occur - replaced with OPENING_BACKQUOTE or
                 CLOSING_BACKQUOTE
 */
#define OPENING_BACKQUOTE (2 * (UCHAR_MAX + 1) + '`')
#define CLOSING_BACKQUOTE (3 * (UCHAR_MAX + 1) + '`')

/* 2 characters of pushback are supported.
   2 characters of pushback occur only when the first is an 'x'; in all
   other cases only one character of pushback is needed.  */
static int phase2_pushback[2];
static int phase2_pushback_length;

/* Return the next character, with backslashes removed.
   The result is QUOTED(c) for some unsigned char c, if the next character
   is escaped sufficiently often to make it a regular constituent character,
   or simply an 'unsigned char' if it has its special meaning (of special,
   whitespace or operator charcter), or OPENING_BACKQUOTE, CLOSING_BACKQUOTE,
   EOF.
   It's the caller's responsibility to update the state.  */
static int
phase2_getc ()
{
  int c;

  if (phase2_pushback_length)
    {
      c = phase2_pushback[--phase2_pushback_length];
      if (c == '\n')
        ++line_number;
      return c;
    }

  c = phase1_getc ();
  if (c == EOF)
    return c;
  if (c == '\'')
    return ((open_doublequote
             || (open_singlequote && open_singlequote_terminator != c))
            ? QUOTED (c)
            : c);
  if (open_singlequote)
    {
      if (c == open_singlequote_terminator)
        return c;
    }
  else
    {
      if (c == '"' || c == '$')
        return c;
      if (c == '`')
        return (nested_backquotes > 0 ? CLOSING_BACKQUOTE : OPENING_BACKQUOTE);
    }
  if (c == '\\')
    {
      /* Number of debackslashification passes that are active at the
         current point.  */
      unsigned int debackslashify =
        nested_backquotes + (open_singlequote ? 0 : 1);
      /* Normal number of backslashes that yield a single backslash in the
         final output.  */
      unsigned int expected_count =
        (unsigned int) 1 << debackslashify;
      /* Number of backslashes found.  */
      unsigned int count;

      for (count = 1; count < expected_count; count++)
        {
          c = phase1_getc ();
          if (c != '\\')
            break;
        }
      if (count == expected_count)
        return '\\';

      /* The count of backslashes is > 0 and < expected_count, therefore the
         result depends on c, the first character after the backslashes.
         Note: The formulas below don't necessarily have a logic; they were
         empirically determined such that 1. the xgettext-sh-1 test succeeds,
         2. the behaviour for count == 0 would correspond to the one without
         any baskslash.  */
      if (c == '\'')
        {
          if (!open_singlequote && count > (expected_count >> 1))
            {
              phase1_ungetc (c);
              return '\\';
            }
          else
            return ((open_doublequote
                     || (open_singlequote
                         ? open_singlequote_terminator != c
                         : count == (expected_count >> 1)))
                    ? QUOTED (c)
                    : c);
        }
      else if (c == '"')
        {
          /* Each debackslashification pass converts \\ to \ and \" to ";
             passes corresponding to `...` drop a lone " whereas passes
             corresponding to "`...`" leave it alone.  Therefore, the
             minimum number of backslashes needed to get one double-quote
             in the end is  open_doublequotes_mask + 1.  */
          if (open_singlequote)
            {
              if (count > open_doublequotes_mask)
                {
                  phase1_ungetc (c);
                  return '\\';
                }
              else
                return (open_singlequote_terminator != c ? QUOTED (c) : c);
            }
          else
            {
              if (count > open_doublequotes_mask)
                return QUOTED (c);
              else
                /* Some of the count values <= open_doublequotes_mask are
                   actually invalid here, but we assume a syntactically
                   correct input file anyway.  */
                return c;
            }
        }
      else if (c == '`')
        {
          /* FIXME: This code looks fishy.  */
          if (count == expected_count - 1)
            return c;
          else
            /* Some of the count values < expected_count - 1 are
               actually invalid here, but we assume a syntactically
               correct input file anyway.  */
            if (nested_backquotes > 0 && !open_singlequote
                && count >= (expected_count >> 2))
              return OPENING_BACKQUOTE;
            else
              return CLOSING_BACKQUOTE;
        }
      else if (c == '$')
        {
          if (open_singlequote)
            return QUOTED (c);
          if (count >= (expected_count >> 1))
            return QUOTED (c);
          else
            return c;
        }
      else
        {
          /* When not followed by a quoting character or backslash or dollar,
             a backslash survives a debackslashification pass unmodified.
             Therefore each debackslashification pass performs a
               count := (count + 1) >> 1
             operation.  Therefore the minimum number of backslashes needed
             to get one backslash in the end is  (expected_count >> 1) + 1.  */
          if (open_doublequote || open_singlequote)
            {
              if (count > 0)
                {
                  phase1_ungetc (c);
                  return '\\';
                }
              else
                return QUOTED (c);
            }
          else
            {
              if (count > (expected_count >> 1))
                {
                  phase1_ungetc (c);
                  return '\\';
                }
              else if (count > 0)
                return QUOTED (c);
              else
                return c;
            }
        }
    }

  return (open_singlequote || open_doublequote ? QUOTED (c) : c);
}

/* Supports 2 characters of pushback.  */
static void
phase2_ungetc (int c)
{
  switch (c)
    {
    case EOF:
      break;

    case '\n':
      --line_number;
      FALLTHROUGH;

    default:
      if (phase2_pushback_length == SIZEOF (phase2_pushback))
        abort ();
      phase2_pushback[phase2_pushback_length++] = c;
      break;
    }
}


/* Context lookup table.  */
static flag_context_list_table_ty *flag_context_list_table;


/* Maximum supported nesting depth.  */
#define MAX_NESTING_DEPTH 1000

/* Current nesting depth.  */
static int nesting_depth;


/* Forward declaration of local functions.  */
static enum word_type read_command_list (int looking_for,
                                         flag_context_ty outer_context);



/* Read the next word.
   'looking_for' denotes a parse terminator, either CLOSING_BACKQUOTE, ')'
   or '\0'.  */
static void
read_word (struct word *wp, int looking_for, flag_context_ty context)
{
  int c;
  bool all_unquoted_digits;
  bool all_unquoted_name_characters;

  do
    {
      c = phase2_getc ();
      if (c == '#')
        {
          /* Skip a comment up to end of line.  */
          last_comment_line = line_number;
          comment_start ();
          for (;;)
            {
              c = phase1_getc ();
              if (c == EOF || c == '\n')
                break;
              /* We skip all leading white space, but not EOLs.  */
              if (!(buflen == 0 && (c == ' ' || c == '\t')))
                comment_add (c);
            }
          comment_line_end ();
        }
      if (c == '\n')
        {
          /* Comments assumed to be grouped with a message must immediately
             precede it, with no non-whitespace token on a line between
             both.  */
          if (last_non_comment_line > last_comment_line)
            savable_comment_reset ();
          wp->type = t_separator;
          return;
        }
    }
  while (is_whitespace (c));

  if (c == EOF)
    {
      wp->type = t_eof;
      return;
    }

  if (c == '<' || c == '>')
    {
      /* Recognize the redirection operators < > >| << <<- >> <> <& >&
         But <( and >) are handled below, not here.  */
      int c2 = phase2_getc ();
      if (c2 != '(')
        {
          if ((c == '<' ? c2 == '<' : c2 == '|') || c2 == '>' || c2 == '&')
            {
              if (c == '<' && c2 == '<')
                {
                  int c3 = phase2_getc ();
                  if (c3 != '-')
                    phase2_ungetc (c3);
                }
            }
          else
            phase2_ungetc (c2);
          wp->type = t_redirect;
          return;
        }
      else
        phase2_ungetc (c2);
    }

  if (c == CLOSING_BACKQUOTE)
    {
      if (looking_for == CLOSING_BACKQUOTE)
        {
          saw_closing_backquote ();
          wp->type = t_backquote;
          last_non_comment_line = line_number;
          return;
        }
      else if (looking_for == ')')
        {
          /* The input is invalid syntax, such as `a<(`
             Push back the closing backquote and pretend that we have seen a
             closing parenthesis.  */
          phase2_ungetc (c);
          wp->type = t_paren;
          last_non_comment_line = line_number;
          return;
        }
      else
        /* We shouldn't be reading a CLOSING_BACKQUOTE when
           looking_for == '\0'.  */
        abort ();
    }

  if (looking_for == ')' && c == ')')
    {
      wp->type = t_paren;
      last_non_comment_line = line_number;
      return;
    }

  if (is_operator_start (c))
    {
      wp->type = (c == ';' ? t_separator : t_other);
      return;
    }

  wp->type = t_string;
  wp->token = XMALLOC (struct token);
  init_token (wp->token);
  wp->line_number_at_start = line_number;
  /* True while all characters in the token seen so far are digits.  */
  all_unquoted_digits = true;
  /* True while all characters in the token seen so far form a "name":
     all characters are unquoted underscores, digits, or alphabetics from the
     portable character set, and the first character is not a digit.  Cf.
     <https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap03.html#tag_03_235>
   */
  all_unquoted_name_characters = true;

  for (;; c = phase2_getc ())
    {
      if (c == EOF)
        break;

      if (all_unquoted_digits && (c == '<' || c == '>'))
        {
          /* Recognize the redirection operators < > >| << <<- >> <> <& >&
             prefixed with a nonempty sequence of unquoted digits.  */
          int c2 = phase2_getc ();
          if ((c == '<' ? c2 == '<' : c2 == '|') || c2 == '>' || c2 == '&')
            {
              if (c == '<' && c2 == '<')
                {
                  int c3 = phase2_getc ();
                  if (c3 != '-')
                    phase2_ungetc (c3);
                }
            }
          else
            phase2_ungetc (c2);

          wp->type = t_redirect;
          free_token (wp->token);
          free (wp->token);

          last_non_comment_line = line_number;

          return;
        }

      all_unquoted_digits = all_unquoted_digits && (c >= '0' && c <= '9');

      if (all_unquoted_name_characters && wp->token->charcount > 0 && c == '=')
        {
          wp->type = t_assignment;
          continue;
        }

      all_unquoted_name_characters =
         all_unquoted_name_characters
         && ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_'
             || (wp->token->charcount > 0 && c >= '0' && c <= '9'));

      if (c == '$')
        {
          int c2;

          /* An unquoted dollar indicates we are not inside '...'.  */
          if (open_singlequote)
            abort ();
          /* After reading a dollar, we know that there is no pushed back
             character from an earlier lookahead.  */
          if (phase2_pushback_length > 0)
            abort ();
          /* Therefore we can use phase1 without interfering with phase2.
             We need to recognize $( outside and inside double-quotes.
             It would be incorrect to do
                c2 = phase2_getc ();
                if (c2 == '(' || c2 == QUOTED ('('))
             because that would also trigger for $\(.  */
          c2 = phase1_getc ();
          if (c2 == '(')
            {
              bool saved_open_doublequote;
              int c3;

              phase1_ungetc (c2);

              /* The entire inner command or arithmetic expression is read
                 ignoring possible surrounding double-quotes.  */
              saved_open_doublequote = open_doublequote;
              open_doublequote = false;

              c2 = phase2_getc ();
              if (c2 != '(')
                abort ();

              c3 = phase2_getc ();
              if (c3 == '(')
                {
                  /* Arithmetic expression (Bash syntax).  Skip until the
                     matching closing parenthesis.  */
                  unsigned int depth = 2;

                  do
                    {
                      c = phase2_getc ();
                      if (c == '(')
                        depth++;
                      else if (c == ')')
                        if (--depth == 0)
                          break;
                    }
                  while (c != EOF);
                }
              else
                {
                  /* Command substitution (Bash syntax).  */
                  phase2_ungetc (c3);
                  ++nesting_depth;
                  read_command_list (')', context);
                  nesting_depth--;
                }

              open_doublequote = saved_open_doublequote;
            }
          else
            {
              phase1_ungetc (c2);
              c2 = phase2_getc ();

              if (c2 == '\'' && !open_singlequote)
                {
                  /* Bash builtin for string with ANSI-C escape sequences.  */
                  for (;;)
                    {
                      /* We have to use phase1 throughout this loop,
                         because phase2 does debackslashification,
                         which is undesirable when parsing ANSI-C
                         escape sequences.  */
                      c = phase1_getc ();
                      if (c == EOF)
                        break;
                      if (c == '\'')
                        break;
                      if (c == '\\')
                        {
                          c = phase1_getc ();
                          switch (c)
                            {
                            default:
                              phase1_ungetc (c);
                              c = '\\';
                              break;

                            case '\\':
                              break;
                            case '\'':
                              break;
                            case '"':
                              break;

                            case 'a':
                              c = '\a';
                              break;
                            case 'b':
                              c = '\b';
                              break;
                            case 'e':
                            case 'E':
                              c = 0x1b; /* ESC */
                              break;
                            case 'f':
                              c = '\f';
                              break;
                            case 'n':
                              c = '\n';
                              break;
                            case 'r':
                              c = '\r';
                              break;
                            case 't':
                              c = '\t';
                              break;
                            case 'v':
                              c = '\v';
                              break;

                            case 'x':
                              c = phase1_getc ();
                              if ((c >= '0' && c <= '9')
                                  || (c >= 'A' && c <= 'F')
                                  || (c >= 'a' && c <= 'f'))
                                {
                                  int n;

                                  if (c >= '0' && c <= '9')
                                    n = c - '0';
                                  else if (c >= 'A' && c <= 'F')
                                    n = 10 + c - 'A';
                                  else if (c >= 'a' && c <= 'f')
                                    n = 10 + c - 'a';
                                  else
                                    abort ();

                                  c = phase1_getc ();
                                  if ((c >= '0' && c <= '9')
                                      || (c >= 'A' && c <= 'F')
                                      || (c >= 'a' && c <= 'f'))
                                    {
                                      if (c >= '0' && c <= '9')
                                        n = n * 16 + c - '0';
                                      else if (c >= 'A' && c <= 'F')
                                        n = n * 16 + 10 + c - 'A';
                                      else if (c >= 'a' && c <= 'f')
                                        n = n * 16 + 10 + c - 'a';
                                      else
                                        abort ();
                                    }
                                  else
                                    phase1_ungetc (c);

                                  c = n;
                                }
                              else
                                {
                                  phase1_ungetc (c);
                                  phase1_ungetc ('x');
                                  c = '\\';
                                }
                              break;

                            case '0': case '1': case '2': case '3':
                            case '4': case '5': case '6': case '7':
                              {
                                int n = c - '0';

                                c = phase1_getc ();
                                if (c >= '0' && c <= '7')
                                  {
                                    n = n * 8 + c - '0';

                                    c = phase1_getc ();
                                    if (c >= '0' && c <= '7')
                                      n = n * 8 + c - '0';
                                    else
                                      phase1_ungetc (c);
                                  }
                                else
                                  phase1_ungetc (c);

                                c = n;
                              }
                              break;
                            }
                        }
                      if (wp->type == t_string)
                        {
                          grow_token (wp->token);
                          wp->token->chars[wp->token->charcount++] =
                            (unsigned char) c;
                        }
                    }
                  /* The result is a literal string.  Don't change wp->type.  */
                  continue;
                }
              else if (c2 == '"' && !open_doublequote)
                {
                  /* Bash builtin for internationalized string.  */
                  lex_pos_ty pos;
                  struct token string;

                  saw_opening_singlequote ();
                  open_singlequote_terminator = '"';
                  pos.file_name = logical_file_name;
                  pos.line_number = line_number;
                  init_token (&string);
                  for (;;)
                    {
                      c = phase2_getc ();
                      if (c == EOF)
                        break;
                      if (c == '"')
                        {
                          saw_closing_singlequote ();
                          break;
                        }
                      grow_token (&string);
                      string.chars[string.charcount++] = (unsigned char) c;
                    }
                  remember_a_message (mlp, NULL, string_of_token (&string),
                                      false, false, context, &pos,
                                      NULL, savable_comment, false);
                  free_token (&string);

                  error_with_progname = false;
                  error (0, 0, _("%s:%lu: warning: the syntax $\"...\" is deprecated due to security reasons; use eval_gettext instead"),
                         pos.file_name, (unsigned long) pos.line_number);
                  error_with_progname = true;

                  /* The result at runtime is not constant. Therefore we
                     change wp->type.  */
                }
              else
                phase2_ungetc (c2);
            }
          wp->type = t_other;
          continue;
        }

      if (c == '\'')
        {
          if (!open_singlequote)
            {
              /* Handle an opening single quote.  */
              saw_opening_singlequote ();
            }
          else
            {
              /* Handle a closing single quote.  */
              saw_closing_singlequote ();
            }
          continue;
        }

      if (c == '"')
        {
          if (open_singlequote && open_singlequote_terminator == '"')
            {
              /* Handle a closing i18n quote.  */
              saw_closing_singlequote ();
            }
          else if (!open_doublequote)
            {
              /* Handle an opening double quote.  */
              saw_opening_doublequote ();
            }
          else
            {
              /* Handle a closing double quote.  */
              saw_closing_doublequote ();
            }
          continue;
        }

      if (c == OPENING_BACKQUOTE)
        {
          /* Handle an opening backquote.  */
          saw_opening_backquote ();

          ++nesting_depth;
          read_command_list (CLOSING_BACKQUOTE, context);
          nesting_depth--;

          wp->type = t_other;
          continue;
        }
      if (c == CLOSING_BACKQUOTE)
        break;

      if (c == '<' || c == '>')
        {
          int c2;

          /* An unquoted c indicates we are not inside '...' nor "...".  */
          if (open_singlequote || open_doublequote)
            abort ();

          c2 = phase2_getc ();
          if (c2 == '(')
            {
              /* Process substitution (Bash syntax).  */
              ++nesting_depth;
              read_command_list (')', context);
              nesting_depth--;

              wp->type = t_other;
              continue;
            }
          else
            phase2_ungetc (c2);
        }

      if (!open_singlequote && !open_doublequote
          && (is_whitespace (c) || is_operator_start (c)))
        break;

      if (wp->type == t_string)
        {
          grow_token (wp->token);
          wp->token->chars[wp->token->charcount++] = (unsigned char) c;
        }
    }

  phase2_ungetc (c);

  if (wp->type != t_string)
    {
      free_token (wp->token);
      free (wp->token);
    }
  last_non_comment_line = line_number;
}


/* Read the next command.
   'looking_for' denotes a parse terminator, either CLOSING_BACKQUOTE, ')'
   or '\0'.
   Returns the type of the word that terminated the command.  */
static enum word_type
read_command (int looking_for, flag_context_ty outer_context)
{
  /* Read the words that make up the command.
     Here we completely ignore field splitting at whitespace and wildcard
     expansions; i.e. we assume that the source is written in such a way that
     every word in the program determines exactly one word in the resulting
     command.
     But we do not require that the 'gettext'/'ngettext' command is the
     first in the command; this is because 1. we want to allow for prefixes
     like "$verbose" that may expand to nothing, and 2. it's a big effort
     to know where a command starts in a $(for ...) or $(case ...) compound
     command.  */
  int arg = 0;                  /* Current argument number.  */
  bool arg_of_redirect = false; /* True right after a redirection operator.  */
  bool must_expand_arg_strings = false; /* True if need to expand escape
                                           sequences in arguments.  */
  flag_context_list_iterator_ty context_iter;
  const struct callshapes *shapes = NULL;
  struct arglist_parser *argparser = NULL;

  for (;;)
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
      if (inner.type == t_separator
          || inner.type == t_backquote || inner.type == t_paren
          || inner.type == t_eof)
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

      if (arg_of_redirect)
        {
          /* Ignore arguments of redirection operators.  */
          arg_of_redirect = false;
        }
      else if (inner.type == t_redirect)
        {
          /* Ignore this word and the following one.  */
          arg_of_redirect = true;
        }
      else
        {
          bool matters_for_argparser = true;

          if (argparser == NULL)
            {
              /* This is the function position.  */
              arg = 0;
              if (inner.type == t_assignment)
                {
                  /* An assignment just sets an environment variable.
                     Ignore it.  */
                  /* Don't increment arg in this round.  */
                  matters_for_argparser = false;
                }
              else if (inner.type == t_string)
                {
                  char *function_name = string_of_word (&inner);

                  if (strcmp (function_name, "env") == 0)
                    {
                      /* The 'env' command just introduces more assignments.
                         Ignore it.  */
                      /* Don't increment arg in this round.  */
                      matters_for_argparser = false;
                    }
                  else
                    {
                      void *keyword_value;

                      if (hash_find_entry (&keywords,
                                           function_name,
                                           strlen (function_name),
                                           &keyword_value)
                          == 0)
                        shapes = (const struct callshapes *) keyword_value;

                      argparser = arglist_parser_alloc (mlp, shapes);

                      context_iter =
                        flag_context_list_iterator (
                          flag_context_list_table_lookup (
                            flag_context_list_table,
                            function_name, strlen (function_name)));
                    }

                  free (function_name);
                }
              else
                context_iter = null_context_list_iterator;
            }
          else
            {
              /* These are the argument positions.  */
              if (inner.type == t_string)
                {
                  bool accepts_context =
                    ((argparser->keyword_len == 7
                      && memcmp (argparser->keyword, "gettext", 7) == 0)
                     || (argparser->keyword_len == 8
                         && memcmp (argparser->keyword, "ngettext", 8) == 0));
                  bool accepts_expand =
                    ((argparser->keyword_len == 7
                      && memcmp (argparser->keyword, "gettext", 7) == 0)
                     || (argparser->keyword_len == 8
                         && memcmp (argparser->keyword, "ngettext", 8) == 0));
                  if (accepts_context && argparser->next_is_msgctxt)
                    {
                      char *s = string_of_word (&inner);
                      mixed_string_ty *ms =
                        mixed_string_alloc_simple (s, lc_string,
                                                   logical_file_name,
                                                   inner.line_number_at_start);
                      free (s);
                      argparser->next_is_msgctxt = false;
                      arglist_parser_remember_msgctxt (argparser, ms,
                                                       inner_context,
                                                       logical_file_name,
                                                       inner.line_number_at_start);
                      matters_for_argparser = false;
                    }
                  else if (accepts_context
                           && ((inner.token->charcount == 2
                                && memcmp (inner.token->chars, "-c", 2) == 0)
                               || (inner.token->charcount == 9
                                   && memcmp (inner.token->chars, "--context", 9) == 0)))
                    {
                      argparser->next_is_msgctxt = true;
                      matters_for_argparser = false;
                    }
                  else if (accepts_context
                           && (inner.token->charcount >= 10
                               && memcmp (inner.token->chars, "--context=", 10) == 0))
                    {
                      char *s = substring_of_word (&inner, 10);
                      mixed_string_ty *ms =
                        mixed_string_alloc_simple (s, lc_string,
                                                   logical_file_name,
                                                   inner.line_number_at_start);
                      free (s);
                      argparser->next_is_msgctxt = false;
                      arglist_parser_remember_msgctxt (argparser, ms,
                                                       inner_context,
                                                       logical_file_name,
                                                       inner.line_number_at_start);
                      matters_for_argparser = false;
                    }
                  else if (accepts_expand
                           && inner.token->charcount == 2
                           && memcmp (inner.token->chars, "-e", 2) == 0)
                    {
                      must_expand_arg_strings = true;
                      matters_for_argparser = false;
                    }
                  else
                    {
                      char *s = string_of_word (&inner);
                      mixed_string_ty *ms;

                      /* When '-e' was specified, expand escape sequences in s.  */
                      if (accepts_expand && must_expand_arg_strings)
                        {
                          bool expands_backslash_c =
                            (argparser->keyword_len == 7
                             && memcmp (argparser->keyword, "gettext", 7) == 0);
                          bool backslash_c = false;
                          char *expanded =
                            (char *)
                            expand_escapes (s, expands_backslash_c ? &backslash_c : NULL);
                          /* We can ignore the value of expands_backslash_c, because
                             here we don't support the gettext '-s' option.  */
                          if (expanded != s)
                            free (s);
                          s = expanded;
                        }

                      ms = mixed_string_alloc_simple (s, lc_string,
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

              if (matters_for_argparser)
                if (arglist_parser_decidedp (argparser, arg))
                  {
                    /* Stop looking for arguments of the last function_name.  */
                    /* FIXME: What about context_iter?  */
                    arglist_parser_done (argparser, arg);
                    shapes = NULL;
                    argparser = NULL;
                  }
            }

          if (matters_for_argparser)
            arg++;
        }

      free_word (&inner);
    }
}


/* Read a list of commands.
   'looking_for' denotes a parse terminator, either CLOSING_BACKQUOTE, ')'
   or '\0'.
   Returns the type of the word that terminated the command list.  */
static enum word_type
read_command_list (int looking_for, flag_context_ty outer_context)
{
  if (nesting_depth > MAX_NESTING_DEPTH)
    {
      error_with_progname = false;
      error (EXIT_FAILURE, 0, _("%s:%d: error: too deeply nested command list"),
             logical_file_name, line_number);
    }
  for (;;)
    {
      enum word_type terminator;

      terminator = read_command (looking_for, outer_context);
      if (terminator != t_separator)
        return terminator;
    }
}


void
extract_sh (FILE *f,
            const char *real_filename, const char *logical_filename,
            flag_context_list_table_ty *flag_table,
            msgdomain_list_ty *mdlp)
{
  mlp = mdlp->item[0]->messages;

  fp = f;
  real_file_name = real_filename;
  logical_file_name = xstrdup (logical_filename);
  line_number = 1;

  phase1_pushback_length = 0;

  last_comment_line = -1;
  last_non_comment_line = -1;

  nested_backquotes = 0;
  open_doublequotes_mask = 0;
  open_doublequote = false;
  open_singlequote = false;

  phase2_pushback_length = 0;

  flag_context_list_table = flag_table;
  nesting_depth = 0;

  init_keywords ();

  /* Eat tokens until eof is seen.  */
  read_command_list ('\0', null_context);

  fp = NULL;
  real_file_name = NULL;
  logical_file_name = NULL;
  line_number = 0;
}
