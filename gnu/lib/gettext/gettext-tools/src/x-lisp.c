/* xgettext Lisp backend.
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
#include "x-lisp.h"

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
#include "gettext.h"

#define _(s) gettext(s)


/* The Common Lisp syntax is described in the Common Lisp HyperSpec, chapter 2.
   Since we are interested only in strings and in forms similar to
        (gettext msgid ...)
   or   (ngettext msgid msgid_plural ...)
   we make the following simplifications:

   - Assume the keywords and strings are in an ASCII compatible encoding.
     This means we can read the input file one byte at a time, instead of
     one character at a time.  No need to worry about multibyte characters:
     If they occur as part of identifiers, they most probably act as
     constituent characters, and the byte based approach will do the same.

   - Assume the read table is the standard Common Lisp read table.
     Non-standard read tables are mostly used to read data, not programs.

   - Assume the read table case is :UPCASE, and *READ-BASE* is 10.

   - Don't interpret #n= and #n#, they usually don't appear in programs.

   - Don't interpret #+, #-, they are unlikely to appear in a gettext form.

   The remaining syntax rules are:

   - The syntax code assigned to each character, and how tokens are built
     up from characters (single escape, multiple escape etc.).

   - Comment syntax: ';' and '#| ... |#'.

   - String syntax: "..." with single escapes.

   - Read macros and dispatch macro character '#'.  Needed to be able to
     tell which is the n-th argument of a function call.

 */


/* ========================= Lexer customization.  ========================= */

/* 'readtable_case' is the case conversion that is applied to non-escaped
    parts of symbol tokens.  In Common Lisp: (readtable-case *readtable*).  */

enum rtcase
{
  case_upcase,
  case_downcase,
  case_preserve,
  case_invert
};

static enum rtcase readtable_case = case_upcase;

/* 'read_base' is the assumed radix of integers and rational numbers.
   In Common Lisp: *read-base*.  */
static int read_base = 10;

/* 'read_preserve_whitespace' specifies whether a whitespace character
   that terminates a token must be pushed back on the input stream.
   We set it to true, because the special newline side effect in read_object()
   requires that read_object() sees every newline not inside a token.  */
static bool read_preserve_whitespace = true;


/* ====================== Keyword set customization.  ====================== */

/* If true extract all strings.  */
static bool extract_all = false;

static hash_table keywords;
static bool default_keywords = true;


void
x_lisp_extract_all ()
{
  extract_all = true;
}


void
x_lisp_keyword (const char *name)
{
  if (name == NULL)
    default_keywords = false;
  else
    {
      const char *end;
      struct callshape shape;
      const char *colon;
      size_t len;
      char *symname;
      size_t i;

      if (keywords.table == NULL)
        hash_init (&keywords, 100);

      split_keywordspec (name, &end, &shape);

      /* The characters between name and end should form a valid Lisp symbol.
         Extract the symbol name part.  */
      colon = strchr (name, ':');
      if (colon != NULL && colon < end)
        {
          name = colon + 1;
          if (name < end && *name == ':')
            name++;
          colon = strchr (name, ':');
          if (colon != NULL && colon < end)
            return;
        }

      /* Uppercase it.  */
      len = end - name;
      symname = XNMALLOC (len, char);
      for (i = 0; i < len; i++)
        symname[i] =
          (name[i] >= 'a' && name[i] <= 'z' ? name[i] - 'a' + 'A' : name[i]);

      insert_keyword_callshape (&keywords, symname, len, &shape);
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
      x_lisp_keyword ("gettext");       /* I18N:GETTEXT */
      x_lisp_keyword ("ngettext:1,2");  /* I18N:NGETTEXT */
      x_lisp_keyword ("gettext-noop");
      default_keywords = false;
    }
}

void
init_flag_table_lisp ()
{
  xgettext_record_flag ("gettext:1:pass-lisp-format");
  xgettext_record_flag ("ngettext:1:pass-lisp-format");
  xgettext_record_flag ("ngettext:2:pass-lisp-format");
  xgettext_record_flag ("gettext-noop:1:pass-lisp-format");
  xgettext_record_flag ("format:2:lisp-format");
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


/* ========= Reading of tokens.  See CLHS 2.2 "Reader Algorithm".  ========= */


/* Syntax code.  See CLHS 2.1.4 "Character Syntax Types".  */

enum syntax_code
{
  syntax_illegal,       /* non-printable, except whitespace     */
  syntax_single_esc,    /* '\' (single escape)                  */
  syntax_multi_esc,     /* '|' (multiple escape)                */
  syntax_constituent,   /* everything else (constituent)        */
  syntax_whitespace,    /* TAB,LF,FF,CR,' ' (whitespace)        */
  syntax_eof,           /* EOF                                  */
  syntax_t_macro,       /* '()'"' (terminating macro)           */
  syntax_nt_macro       /* '#' (non-terminating macro)          */
};

/* Returns the syntax code of a character.  */
static enum syntax_code
syntax_code_of (unsigned char c)
{
  switch (c)
    {
    case '\\':
      return syntax_single_esc;
    case '|':
      return syntax_multi_esc;
    case '\t': case '\n': case '\f': case '\r': case ' ':
      return syntax_whitespace;
    case '(': case ')': case '\'': case '"': case ',': case ';': case '`':
      return syntax_t_macro;
    case '#':
      return syntax_nt_macro;
    default:
      if (c < ' ' && c != '\b')
        return syntax_illegal;
      else
        return syntax_constituent;
    }
}

struct char_syntax
{
  int ch;                       /* character */
  enum syntax_code scode;       /* syntax code */
};

/* Returns the next character and its syntax code.  */
static void
read_char_syntax (struct char_syntax *p)
{
  int c = do_getc ();

  p->ch = c;
  p->scode = (c == EOF ? syntax_eof : syntax_code_of (c));
}

/* Every character in a token has an attribute assigned.  The attributes
   help during interpretation of the token.  See
   CLHS 2.3 "Interpretation of Tokens" for the possible interpretations,
   and CLHS 2.1.4.2 "Constituent Traits".  */

enum attribute
{
  a_illg,       /* invalid constituent */
  a_pack_m,     /* ':' package marker */
  a_alpha,      /* normal alphabetic */
  a_escaped,    /* alphabetic but not subject to case conversion */
  a_ratio,      /* '/' */
  a_dot,        /* '.' */
  a_sign,       /* '+-' */
  a_extens,     /* '_^' extension characters */
  a_digit,      /* '0123456789' */
  a_letterdigit,/* 'A'-'Z','a'-'z' below base, except 'esfdlESFDL' */
  a_expodigit,  /* 'esfdlESFDL' below base */
  a_letter,     /* 'A'-'Z','a'-'z', except 'esfdlESFDL' */
  a_expo        /* 'esfdlESFDL' */
};

#define is_letter_attribute(a) ((a) >= a_letter)
#define is_number_attribute(a) ((a) >= a_ratio)

/* Returns the attribute of a character, assuming base 10.  */
static enum attribute
attribute_of (unsigned char c)
{
  switch (c)
    {
    case ':':
      return a_pack_m;
    case '/':
      return a_ratio;
    case '.':
      return a_dot;
    case '+': case '-':
      return a_sign;
    case '_': case '^':
      return a_extens;
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
      return a_digit;
    case 'a': case 'b': case 'c': case 'g': case 'h': case 'i': case 'j':
    case 'k': case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
    case 't': case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
    case 'A': case 'B': case 'C': case 'G': case 'H': case 'I': case 'J':
    case 'K': case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
    case 'T': case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
      return a_letter;
    case 'e': case 's': case 'd': case 'f': case 'l':
    case 'E': case 'S': case 'D': case 'F': case 'L':
      return a_expo;
    default:
      /* Treat everything as valid.  Never return a_illg.  */
      return a_alpha;
    }
}

struct token_char
{
  unsigned char ch;             /* character */
  unsigned char attribute;      /* attribute */
};

/* A token consists of a sequence of characters with associated attribute.  */
struct token
{
  int allocated;                /* number of allocated 'token_char's */
  int charcount;                /* number of used 'token_char's */
  struct token_char *chars;     /* the token's constituents */
  bool with_escape;             /* whether single-escape or multiple escape occurs */
};

/* Initialize a 'struct token'.  */
static inline void
init_token (struct token *tp)
{
  tp->allocated = 10;
  tp->chars = XNMALLOC (tp->allocated, struct token_char);
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
      tp->chars = (struct token_char *) xrealloc (tp->chars, tp->allocated * sizeof (struct token_char));
    }
}

/* Read the next token.  If 'first' is given, it points to the first
   character, which has already been read.
   The algorithm follows CLHS 2.2 "Reader Algorithm".  */
static void
read_token (struct token *tp, const struct char_syntax *first)
{
  bool multiple_escape_flag;
  struct char_syntax curr;

  init_token (tp);
  tp->with_escape = false;

  multiple_escape_flag = false;
  if (first)
    curr = *first;
  else
    read_char_syntax (&curr);

  for (;; read_char_syntax (&curr))
    {
      switch (curr.scode)
        {
        case syntax_illegal:
          /* Invalid input.  Be tolerant, no error message.  */
          do_ungetc (curr.ch);
          return;

        case syntax_single_esc:
          tp->with_escape = true;
          read_char_syntax (&curr);
          if (curr.scode == syntax_eof)
            /* Invalid input.  Be tolerant, no error message.  */
            return;
          grow_token (tp);
          tp->chars[tp->charcount].ch = curr.ch;
          tp->chars[tp->charcount].attribute = a_escaped;
          tp->charcount++;
          break;

        case syntax_multi_esc:
          multiple_escape_flag = !multiple_escape_flag;
          tp->with_escape = true;
          break;

        case syntax_constituent:
        case syntax_nt_macro:
          grow_token (tp);
          if (multiple_escape_flag)
            {
              tp->chars[tp->charcount].ch = curr.ch;
              tp->chars[tp->charcount].attribute = a_escaped;
              tp->charcount++;
            }
          else
            {
              tp->chars[tp->charcount].ch = curr.ch;
              tp->chars[tp->charcount].attribute = attribute_of (curr.ch);
              tp->charcount++;
            }
          break;

        case syntax_whitespace:
        case syntax_t_macro:
          if (multiple_escape_flag)
            {
              grow_token (tp);
              tp->chars[tp->charcount].ch = curr.ch;
              tp->chars[tp->charcount].attribute = a_escaped;
              tp->charcount++;
            }
          else
            {
              if (curr.scode != syntax_whitespace || read_preserve_whitespace)
                do_ungetc (curr.ch);
              return;
            }
          break;

        case syntax_eof:
          if (multiple_escape_flag)
            /* Invalid input.  Be tolerant, no error message.  */
            ;
          return;
        }
    }
}

/* A potential number is a token which
   1. consists only of digits, '+','-','/','^','_','.' and number markers.
      The base for digits is context dependent, but always 10 if a dot '.'
      occurs. A number marker is a non-digit letter which is not adjacent
      to a non-digit letter.
   2. has at least one digit.
   3. starts with a digit, '+','-','.','^' or '_'.
   4. does not end with '+' or '-'.
   See CLHS 2.3.1.1 "Potential Numbers as Tokens".
 */

static inline bool
has_a_dot (const struct token *tp)
{
  int n = tp->charcount;
  int i;

  for (i = 0; i < n; i++)
    if (tp->chars[i].attribute == a_dot)
      return true;
  return false;
}

static inline bool
all_a_number (const struct token *tp)
{
  int n = tp->charcount;
  int i;

  for (i = 0; i < n; i++)
    if (!is_number_attribute (tp->chars[i].attribute))
      return false;
  return true;
}

static inline void
a_letter_to_digit (const struct token *tp, int base)
{
  int n = tp->charcount;
  int i;

  for (i = 0; i < n; i++)
    if (is_letter_attribute (tp->chars[i].attribute))
      {
        int c = tp->chars[i].ch;

        if (c >= 'a')
          c -= 'a' - 'A';
        if (c - 'A' + 10 < base)
          tp->chars[i].attribute -= 2; /* a_letter -> a_letterdigit,
                                          a_expo -> a_expodigit */
      }
}

static inline bool
has_a_digit (const struct token *tp)
{
  int n = tp->charcount;
  int i;

  for (i = 0; i < n; i++)
    if (tp->chars[i].attribute == a_digit
        || tp->chars[i].attribute == a_letterdigit
        || tp->chars[i].attribute == a_expodigit)
      return true;
  return false;
}

static inline bool
has_adjacent_letters (const struct token *tp)
{
  int n = tp->charcount;
  int i;

  for (i = 1; i < n; i++)
    if (is_letter_attribute (tp->chars[i-1].attribute)
        && is_letter_attribute (tp->chars[i].attribute))
      return true;
  return false;
}

static bool
is_potential_number (const struct token *tp, int *basep)
{
  /* CLHS 2.3.1.1.1:
     "A potential number cannot contain any escape characters."  */
  if (tp->with_escape)
    return false;

  if (has_a_dot (tp))
    *basep = 10;

  if (!all_a_number (tp))
    return false;

  a_letter_to_digit (tp, *basep);

  if (!has_a_digit (tp))
    return false;

  if (has_adjacent_letters (tp))
    return false;

  if (!(tp->chars[0].attribute >= a_dot
        && tp->chars[0].attribute <= a_expodigit))
    return false;

  if (tp->chars[tp->charcount - 1].attribute == a_sign)
    return false;

  return true;
}

/* A number is one of integer, ratio, float.  Each has a particular syntax.
   See CLHS 2.3.1 "Numbers as Tokens".
   But note a mistake: The exponent rule should read:
       exponent ::= exponent-marker [sign] {decimal-digit}+
   (see 22.1.3.1.3 "Printing Floats").  */

enum number_type
{
  n_none,
  n_integer,
  n_ratio,
  n_float
};

static enum number_type
is_number (const struct token *tp, int *basep)
{
  struct token_char *ptr_limit;
  struct token_char *ptr1;

  if (!is_potential_number (tp, basep))
    return n_none;

  /* is_potential_number guarantees
     - all attributes are >= a_ratio,
     - there is at least one a_digit or a_letterdigit or a_expodigit, and
     - if there is an a_dot, then *basep = 10.  */

  ptr1 = &tp->chars[0];
  ptr_limit = &tp->chars[tp->charcount];

  if (ptr1->attribute == a_sign)
    ptr1++;

  /* Test for syntax
   * { a_sign | }
   * { a_digit < base }+ { a_ratio { a_digit < base }+ | }
   */
  {
    bool seen_a_ratio = false;
    bool seen_a_digit = false;  /* seen a digit in last digit block? */
    struct token_char *ptr;

    for (ptr = ptr1;; ptr++)
      {
        if (ptr >= ptr_limit)
          {
            if (!seen_a_digit)
              break;
            if (seen_a_ratio)
              return n_ratio;
            else
              return n_integer;
          }
        if (ptr->attribute == a_digit
            || ptr->attribute == a_letterdigit
            || ptr->attribute == a_expodigit)
          {
            int c = ptr->ch;

            c = (c < 'A' ? c - '0' : c < 'a' ? c - 'A' + 10 : c - 'a' + 10);
            if (c >= *basep)
              break;
            seen_a_digit = true;
          }
        else if (ptr->attribute == a_ratio)
          {
            if (seen_a_ratio || !seen_a_digit)
              break;
            seen_a_ratio = true;
            seen_a_digit = false;
          }
        else
          break;
      }
  }

  /* Test for syntax
   * { a_sign | }
   * { a_digit }* { a_dot { a_digit }* | }
   * { a_expo { a_sign | } { a_digit }+ | }
   *
   * If there is an exponent part, there must be digits before the dot or
   * after the dot. The result is a float.
   * If there is no exponen:
   *   If there is no dot, it would an integer in base 10, but is has already
   *   been verified to not be an integer in the current base.
   *   If there is a dot:
   *     If there are digits after the dot, it's a float.
   *     Otherwise, if there are digits before the dot, it's an integer.
   */
  *basep = 10;
  {
    bool seen_a_dot = false;
    bool seen_a_dot_with_leading_digits = false;
    bool seen_a_digit = false;  /* seen a digit in last digit block? */
    struct token_char *ptr;

    for (ptr = ptr1;; ptr++)
      {
        if (ptr >= ptr_limit)
          {
            /* no exponent */
            if (!seen_a_dot)
              return n_none;
            if (seen_a_digit)
              return n_float;
            if (seen_a_dot_with_leading_digits)
              return n_integer;
            else
              return n_none;
          }
        if (ptr->attribute == a_digit)
          {
            seen_a_digit = true;
          }
        else if (ptr->attribute == a_dot)
          {
            if (seen_a_dot)
              return n_none;
            seen_a_dot = true;
            if (seen_a_digit)
              seen_a_dot_with_leading_digits = true;
            seen_a_digit = false;
          }
        else if (ptr->attribute == a_expo || ptr->attribute == a_expodigit)
          break;
        else
          return n_none;
      }
    ptr++;
    if (!seen_a_dot_with_leading_digits || !seen_a_digit)
      return n_none;
    if (ptr >= ptr_limit)
      return n_none;
    if (ptr->attribute == a_sign)
      ptr++;
    seen_a_digit = false;
    for (;; ptr++)
      {
        if (ptr >= ptr_limit)
          break;
        if (ptr->attribute != a_digit)
          return n_none;
        seen_a_digit = true;
      }
    if (!seen_a_digit)
      return n_none;
    return n_float;
  }
}

/* A token representing a symbol must be case converted.
   For portability, we convert only ASCII characters here.  */

static void
upcase_token (struct token *tp)
{
  int n = tp->charcount;
  int i;

  for (i = 0; i < n; i++)
    if (tp->chars[i].attribute != a_escaped)
      {
        unsigned char c = tp->chars[i].ch;
        if (c >= 'a' && c <= 'z')
          tp->chars[i].ch = c - 'a' + 'A';
      }
}

static void
downcase_token (struct token *tp)
{
  int n = tp->charcount;
  int i;

  for (i = 0; i < n; i++)
    if (tp->chars[i].attribute != a_escaped)
      {
        unsigned char c = tp->chars[i].ch;
        if (c >= 'A' && c <= 'Z')
          tp->chars[i].ch = c - 'A' + 'a';
      }
}

static void
case_convert_token (struct token *tp)
{
  int n = tp->charcount;
  int i;

  switch (readtable_case)
    {
    case case_upcase:
      upcase_token (tp);
      break;

    case case_downcase:
      downcase_token (tp);
      break;

    case case_preserve:
      break;

    case case_invert:
      {
        bool seen_uppercase = false;
        bool seen_lowercase = false;
        for (i = 0; i < n; i++)
          if (tp->chars[i].attribute != a_escaped)
            {
              unsigned char c = tp->chars[i].ch;
              if (c >= 'a' && c <= 'z')
                seen_lowercase = true;
              if (c >= 'A' && c <= 'Z')
                seen_uppercase = true;
            }
        if (seen_uppercase)
          {
            if (!seen_lowercase)
              downcase_token (tp);
          }
        else
          {
            if (seen_lowercase)
              upcase_token (tp);
          }
      }
      break;
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
  t_close,      /* ')' pseudo object */
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
  const struct token_char *p;
  char *q;
  int n;

  if (!(op->type == t_symbol || op->type == t_string))
    abort ();
  n = op->token->charcount;
  str = XNMALLOC (n + 1, char);
  q = str;
  for (p = op->token->chars; n > 0; p++, n--)
    *q++ = p->ch;
  *q = '\0';
  return str;
}


/* Context lookup table.  */
static flag_context_list_table_ty *flag_context_list_table;


/* Maximum supported nesting depth.  */
#define MAX_NESTING_DEPTH 1000

/* Current nesting depth.  */
static int nesting_depth;


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
      struct char_syntax curr;

      read_char_syntax (&curr);

      switch (curr.scode)
        {
        case syntax_eof:
          op->type = t_eof;
          return;

        case syntax_whitespace:
          if (curr.ch == '\n')
            /* Comments assumed to be grouped with a message must immediately
               precede it, with no non-whitespace token on a line between
               both.  */
            if (last_non_comment_line > last_comment_line)
              savable_comment_reset ();
          continue;

        case syntax_illegal:
          op->type = t_other;
          return;

        case syntax_single_esc:
        case syntax_multi_esc:
        case syntax_constituent:
          /* Start reading a token.  */
          op->token = XMALLOC (struct token);
          read_token (op->token, &curr);
          last_non_comment_line = line_number;

          /* Interpret the token.  */

          /* Dots.  */
          if (!op->token->with_escape
              && op->token->charcount == 1
              && op->token->chars[0].attribute == a_dot)
            {
              free_token (op->token);
              free (op->token);
              op->type = t_dot;
              return;
            }
          /* Tokens consisting entirely of dots are illegal, but be tolerant
             here.  */

          /* Number.  */
          {
            int base = read_base;

            if (is_number (op->token, &base) != n_none)
              {
                free_token (op->token);
                free (op->token);
                op->type = t_other;
                return;
              }
          }

          /* We interpret all other tokens as symbols (including 'reserved
             tokens', i.e. potential numbers which are not numbers).  */
          case_convert_token (op->token);
          op->type = t_symbol;
          return;

        case syntax_t_macro:
        case syntax_nt_macro:
          /* Read a macro.  */
          switch (curr.ch)
            {
            case '(':
              {
                int arg = 0;            /* Current argument number.  */
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

                    /* EOF inside list is illegal.
                       But be tolerant.  */
                    if (inner.type == t_eof)
                      break;

                    if (arg == 0)
                      {
                        /* This is the function position.  */
                        if (inner.type == t_symbol)
                          {
                            char *symbol_name = string_of_object (&inner);
                            int i;
                            int prefix_len;
                            void *keyword_value;

                            /* Omit any package name.  */
                            i = inner.token->charcount;
                            while (i > 0
                                   && inner.token->chars[i-1].attribute != a_pack_m)
                              i--;
                            prefix_len = i;

                            if (hash_find_entry (&keywords,
                                                 symbol_name + prefix_len,
                                                 strlen (symbol_name + prefix_len),
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
                 Unmatched closing parenthesis is illegal.
                 But be tolerant.  */
              op->type = t_close;
              last_non_comment_line = line_number;
              return;

            case ',':
              {
                int c = do_getc ();
                /* The ,@ handling inside lists is wrong anyway, because
                   ,@form expands to an unknown number of elements.  */
                if (c != EOF && c != '@' && c != '.')
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
                    if (c == '\\') /* syntax_single_esc */
                      {
                        c = do_getc ();
                        if (c == EOF)
                          /* Invalid input.  Be tolerant, no error message.  */
                          break;
                      }
                    grow_token (op->token);
                    op->token->chars[op->token->charcount++].ch = c;
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

            case '#':
              /* Dispatch macro handling.  */
              {
                int dmc;

                for (;;)
                  {
                    dmc = do_getc ();
                    if (dmc == EOF)
                      /* Invalid input.  Be tolerant, no error message.  */
                      {
                        op->type = t_other;
                        return;
                      }
                    if (!(dmc >= '0' && dmc <= '9'))
                      break;
                  }

                switch (dmc)
                  {
                  case '(':
                  case '"':
                    do_ungetc (dmc);
                    FALLTHROUGH;
                  case '\'':
                  case ':':
                  case '.':
                  case ',':
                  case 'A': case 'a':
                  case 'C': case 'c':
                  case 'P': case 'p':
                  case 'S': case 's':
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
                      struct char_syntax first;
                      first.ch = '\\';
                      first.scode = syntax_single_esc;
                      read_token (&token, &first);
                      free_token (&token);
                      op->type = t_other;
                      last_non_comment_line = line_number;
                      return;
                    }

                  case 'B': case 'b':
                  case 'O': case 'o':
                  case 'X': case 'x':
                  case 'R': case 'r':
                  case '*':
                    {
                      struct token token;
                      read_token (&token, NULL);
                      free_token (&token);
                      op->type = t_other;
                      last_non_comment_line = line_number;
                      return;
                    }

                  case '=':
                    /* Ignore read labels.  */
                    continue;

                  case '#':
                    /* Don't bother looking up the corresponding object.  */
                    op->type = t_other;
                    last_non_comment_line = line_number;
                    return;

                  case '+':
                  case '-':
                    /* Simply assume every feature expression is true.  */
                    {
                      struct object inner;
                      ++nesting_depth;
                      read_object (&inner, null_context);
                      nesting_depth--;
                      /* Dots and EOF are not allowed here.
                         But be tolerant.  */
                      free_object (&inner);
                      continue;
                    }

                  default:
                    op->type = t_other;
                    last_non_comment_line = line_number;
                    return;
                  }
                /*NOTREACHED*/
                abort ();
              }

            default:
              /*NOTREACHED*/
              abort ();
            }

        default:
          /*NOTREACHED*/
          abort ();
        }
    }
}


void
extract_lisp (FILE *f,
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
