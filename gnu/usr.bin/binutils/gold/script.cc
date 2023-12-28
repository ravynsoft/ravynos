// script.cc -- handle linker scripts for gold.

// Copyright (C) 2006-2023 Free Software Foundation, Inc.
// Written by Ian Lance Taylor <iant@google.com>.

// This file is part of gold.

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
// MA 02110-1301, USA.

#include "gold.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fnmatch.h>
#include <string>
#include <vector>
#include "filenames.h"

#include "elfcpp.h"
#include "demangle.h"
#include "dirsearch.h"
#include "options.h"
#include "fileread.h"
#include "workqueue.h"
#include "readsyms.h"
#include "parameters.h"
#include "layout.h"
#include "symtab.h"
#include "target-select.h"
#include "script.h"
#include "script-c.h"
#include "incremental.h"

namespace gold
{

// A token read from a script file.  We don't implement keywords here;
// all keywords are simply represented as a string.

class Token
{
 public:
  // Token classification.
  enum Classification
  {
    // Token is invalid.
    TOKEN_INVALID,
    // Token indicates end of input.
    TOKEN_EOF,
    // Token is a string of characters.
    TOKEN_STRING,
    // Token is a quoted string of characters.
    TOKEN_QUOTED_STRING,
    // Token is an operator.
    TOKEN_OPERATOR,
    // Token is a number (an integer).
    TOKEN_INTEGER
  };

  // We need an empty constructor so that we can put this STL objects.
  Token()
    : classification_(TOKEN_INVALID), value_(NULL), value_length_(0),
      opcode_(0), lineno_(0), charpos_(0)
  { }

  // A general token with no value.
  Token(Classification classification, int lineno, int charpos)
    : classification_(classification), value_(NULL), value_length_(0),
      opcode_(0), lineno_(lineno), charpos_(charpos)
  {
    gold_assert(classification == TOKEN_INVALID
		|| classification == TOKEN_EOF);
  }

  // A general token with a value.
  Token(Classification classification, const char* value, size_t length,
	int lineno, int charpos)
    : classification_(classification), value_(value), value_length_(length),
      opcode_(0), lineno_(lineno), charpos_(charpos)
  {
    gold_assert(classification != TOKEN_INVALID
		&& classification != TOKEN_EOF);
  }

  // A token representing an operator.
  Token(int opcode, int lineno, int charpos)
    : classification_(TOKEN_OPERATOR), value_(NULL), value_length_(0),
      opcode_(opcode), lineno_(lineno), charpos_(charpos)
  { }

  // Return whether the token is invalid.
  bool
  is_invalid() const
  { return this->classification_ == TOKEN_INVALID; }

  // Return whether this is an EOF token.
  bool
  is_eof() const
  { return this->classification_ == TOKEN_EOF; }

  // Return the token classification.
  Classification
  classification() const
  { return this->classification_; }

  // Return the line number at which the token starts.
  int
  lineno() const
  { return this->lineno_; }

  // Return the character position at this the token starts.
  int
  charpos() const
  { return this->charpos_; }

  // Get the value of a token.

  const char*
  string_value(size_t* length) const
  {
    gold_assert(this->classification_ == TOKEN_STRING
		|| this->classification_ == TOKEN_QUOTED_STRING);
    *length = this->value_length_;
    return this->value_;
  }

  int
  operator_value() const
  {
    gold_assert(this->classification_ == TOKEN_OPERATOR);
    return this->opcode_;
  }

  uint64_t
  integer_value() const;

 private:
  // The token classification.
  Classification classification_;
  // The token value, for TOKEN_STRING or TOKEN_QUOTED_STRING or
  // TOKEN_INTEGER.
  const char* value_;
  // The length of the token value.
  size_t value_length_;
  // The token value, for TOKEN_OPERATOR.
  int opcode_;
  // The line number where this token started (one based).
  int lineno_;
  // The character position within the line where this token started
  // (one based).
  int charpos_;
};

// Return the value of a TOKEN_INTEGER.

uint64_t
Token::integer_value() const
{
  gold_assert(this->classification_ == TOKEN_INTEGER);

  size_t len = this->value_length_;

  uint64_t multiplier = 1;
  char last = this->value_[len - 1];
  if (last == 'm' || last == 'M')
    {
      multiplier = 1024 * 1024;
      --len;
    }
  else if (last == 'k' || last == 'K')
    {
      multiplier = 1024;
      --len;
    }

  char *end;
  uint64_t ret = strtoull(this->value_, &end, 0);
  gold_assert(static_cast<size_t>(end - this->value_) == len);

  return ret * multiplier;
}

// This class handles lexing a file into a sequence of tokens.

class Lex
{
 public:
  // We unfortunately have to support different lexing modes, because
  // when reading different parts of a linker script we need to parse
  // things differently.
  enum Mode
  {
    // Reading an ordinary linker script.
    LINKER_SCRIPT,
    // Reading an expression in a linker script.
    EXPRESSION,
    // Reading a version script.
    VERSION_SCRIPT,
    // Reading a --dynamic-list file.
    DYNAMIC_LIST
  };

  Lex(const char* input_string, size_t input_length, int parsing_token)
    : input_string_(input_string), input_length_(input_length),
      current_(input_string), mode_(LINKER_SCRIPT),
      first_token_(parsing_token), token_(),
      lineno_(1), linestart_(input_string)
  { }

  // Read a file into a string.
  static void
  read_file(Input_file*, std::string*);

  // Return the next token.
  const Token*
  next_token();

  // Return the current lexing mode.
  Lex::Mode
  mode() const
  { return this->mode_; }

  // Set the lexing mode.
  void
  set_mode(Mode mode)
  { this->mode_ = mode; }

 private:
  Lex(const Lex&);
  Lex& operator=(const Lex&);

  // Make a general token with no value at the current location.
  Token
  make_token(Token::Classification c, const char* start) const
  { return Token(c, this->lineno_, start - this->linestart_ + 1); }

  // Make a general token with a value at the current location.
  Token
  make_token(Token::Classification c, const char* v, size_t len,
	     const char* start)
    const
  { return Token(c, v, len, this->lineno_, start - this->linestart_ + 1); }

  // Make an operator token at the current location.
  Token
  make_token(int opcode, const char* start) const
  { return Token(opcode, this->lineno_, start - this->linestart_ + 1); }

  // Make an invalid token at the current location.
  Token
  make_invalid_token(const char* start)
  { return this->make_token(Token::TOKEN_INVALID, start); }

  // Make an EOF token at the current location.
  Token
  make_eof_token(const char* start)
  { return this->make_token(Token::TOKEN_EOF, start); }

  // Return whether C can be the first character in a name.  C2 is the
  // next character, since we sometimes need that.
  inline bool
  can_start_name(char c, char c2);

  // If C can appear in a name which has already started, return a
  // pointer to a character later in the token or just past
  // it. Otherwise, return NULL.
  inline const char*
  can_continue_name(const char* c);

  // Return whether C, C2, C3 can start a hex number.
  inline bool
  can_start_hex(char c, char c2, char c3);

  // If C can appear in a hex number which has already started, return
  // a pointer to a character later in the token or just past
  // it. Otherwise, return NULL.
  inline const char*
  can_continue_hex(const char* c);

  // Return whether C can start a non-hex number.
  static inline bool
  can_start_number(char c);

  // If C can appear in a decimal number which has already started,
  // return a pointer to a character later in the token or just past
  // it. Otherwise, return NULL.
  inline const char*
  can_continue_number(const char* c)
  { return Lex::can_start_number(*c) ? c + 1 : NULL; }

  // If C1 C2 C3 form a valid three character operator, return the
  // opcode.  Otherwise return 0.
  static inline int
  three_char_operator(char c1, char c2, char c3);

  // If C1 C2 form a valid two character operator, return the opcode.
  // Otherwise return 0.
  static inline int
  two_char_operator(char c1, char c2);

  // If C1 is a valid one character operator, return the opcode.
  // Otherwise return 0.
  static inline int
  one_char_operator(char c1);

  // Read the next token.
  Token
  get_token(const char**);

  // Skip a C style /* */ comment.  Return false if the comment did
  // not end.
  bool
  skip_c_comment(const char**);

  // Skip a line # comment.  Return false if there was no newline.
  bool
  skip_line_comment(const char**);

  // Build a token CLASSIFICATION from all characters that match
  // CAN_CONTINUE_FN.  The token starts at START.  Start matching from
  // MATCH.  Set *PP to the character following the token.
  inline Token
  gather_token(Token::Classification,
	       const char* (Lex::*can_continue_fn)(const char*),
	       const char* start, const char* match, const char** pp);

  // Build a token from a quoted string.
  Token
  gather_quoted_string(const char** pp);

  // The string we are tokenizing.
  const char* input_string_;
  // The length of the string.
  size_t input_length_;
  // The current offset into the string.
  const char* current_;
  // The current lexing mode.
  Mode mode_;
  // The code to use for the first token.  This is set to 0 after it
  // is used.
  int first_token_;
  // The current token.
  Token token_;
  // The current line number.
  int lineno_;
  // The start of the current line in the string.
  const char* linestart_;
};

// Read the whole file into memory.  We don't expect linker scripts to
// be large, so we just use a std::string as a buffer.  We ignore the
// data we've already read, so that we read aligned buffers.

void
Lex::read_file(Input_file* input_file, std::string* contents)
{
  off_t filesize = input_file->file().filesize();
  contents->clear();
  contents->reserve(filesize);

  off_t off = 0;
  unsigned char buf[BUFSIZ];
  while (off < filesize)
    {
      off_t get = BUFSIZ;
      if (get > filesize - off)
	get = filesize - off;
      input_file->file().read(off, get, buf);
      contents->append(reinterpret_cast<char*>(&buf[0]), get);
      off += get;
    }
}

// Return whether C can be the start of a name, if the next character
// is C2.  A name can being with a letter, underscore, period, or
// dollar sign.  Because a name can be a file name, we also permit
// forward slash, backslash, and tilde.  Tilde is the tricky case
// here; GNU ld also uses it as a bitwise not operator.  It is only
// recognized as the operator if it is not immediately followed by
// some character which can appear in a symbol.  That is, when we
// don't know that we are looking at an expression, "~0" is a file
// name, and "~ 0" is an expression using bitwise not.  We are
// compatible.

inline bool
Lex::can_start_name(char c, char c2)
{
  switch (c)
    {
    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
    case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
    case 'M': case 'N': case 'O': case 'Q': case 'P': case 'R':
    case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
    case 'Y': case 'Z':
    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
    case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
    case 'm': case 'n': case 'o': case 'q': case 'p': case 'r':
    case 's': case 't': case 'u': case 'v': case 'w': case 'x':
    case 'y': case 'z':
    case '_': case '.': case '$':
      return true;

    case '/': case '\\':
      return this->mode_ == LINKER_SCRIPT;

    case '~':
      return this->mode_ == LINKER_SCRIPT && can_continue_name(&c2);

    case '*': case '[':
      return (this->mode_ == VERSION_SCRIPT
              || this->mode_ == DYNAMIC_LIST
	      || (this->mode_ == LINKER_SCRIPT
		  && can_continue_name(&c2)));

    default:
      return false;
    }
}

// Return whether C can continue a name which has already started.
// Subsequent characters in a name are the same as the leading
// characters, plus digits and "=+-:[],?*".  So in general the linker
// script language requires spaces around operators, unless we know
// that we are parsing an expression.

inline const char*
Lex::can_continue_name(const char* c)
{
  switch (*c)
    {
    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
    case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
    case 'M': case 'N': case 'O': case 'Q': case 'P': case 'R':
    case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
    case 'Y': case 'Z':
    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
    case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
    case 'm': case 'n': case 'o': case 'q': case 'p': case 'r':
    case 's': case 't': case 'u': case 'v': case 'w': case 'x':
    case 'y': case 'z':
    case '_': case '.': case '$':
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
      return c + 1;

    // TODO(csilvers): why not allow ~ in names for version-scripts?
    case '/': case '\\': case '~':
    case '=': case '+':
    case ',':
      if (this->mode_ == LINKER_SCRIPT)
        return c + 1;
      return NULL;

    case '[': case ']': case '*': case '?': case '-':
      if (this->mode_ == LINKER_SCRIPT || this->mode_ == VERSION_SCRIPT
          || this->mode_ == DYNAMIC_LIST)
        return c + 1;
      return NULL;

    // TODO(csilvers): why allow this?  ^ is meaningless in version scripts.
    case '^':
      if (this->mode_ == VERSION_SCRIPT || this->mode_ == DYNAMIC_LIST)
        return c + 1;
      return NULL;

    case ':':
      if (this->mode_ == LINKER_SCRIPT)
        return c + 1;
      else if ((this->mode_ == VERSION_SCRIPT || this->mode_ == DYNAMIC_LIST)
               && (c[1] == ':'))
        {
          // A name can have '::' in it, as that's a c++ namespace
          // separator. But a single colon is not part of a name.
          return c + 2;
        }
      return NULL;

    default:
      return NULL;
    }
}

// For a number we accept 0x followed by hex digits, or any sequence
// of digits.  The old linker accepts leading '$' for hex, and
// trailing HXBOD.  Those are for MRI compatibility and we don't
// accept them.

// Return whether C1 C2 C3 can start a hex number.

inline bool
Lex::can_start_hex(char c1, char c2, char c3)
{
  if (c1 == '0' && (c2 == 'x' || c2 == 'X'))
    return this->can_continue_hex(&c3);
  return false;
}

// Return whether C can appear in a hex number.

inline const char*
Lex::can_continue_hex(const char* c)
{
  switch (*c)
    {
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
      return c + 1;

    default:
      return NULL;
    }
}

// Return whether C can start a non-hex number.

inline bool
Lex::can_start_number(char c)
{
  switch (c)
    {
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
      return true;

    default:
      return false;
    }
}

// If C1 C2 C3 form a valid three character operator, return the
// opcode (defined in the yyscript.h file generated from yyscript.y).
// Otherwise return 0.

inline int
Lex::three_char_operator(char c1, char c2, char c3)
{
  switch (c1)
    {
    case '<':
      if (c2 == '<' && c3 == '=')
	return LSHIFTEQ;
      break;
    case '>':
      if (c2 == '>' && c3 == '=')
	return RSHIFTEQ;
      break;
    default:
      break;
    }
  return 0;
}

// If C1 C2 form a valid two character operator, return the opcode
// (defined in the yyscript.h file generated from yyscript.y).
// Otherwise return 0.

inline int
Lex::two_char_operator(char c1, char c2)
{
  switch (c1)
    {
    case '=':
      if (c2 == '=')
	return EQ;
      break;
    case '!':
      if (c2 == '=')
	return NE;
      break;
    case '+':
      if (c2 == '=')
	return PLUSEQ;
      break;
    case '-':
      if (c2 == '=')
	return MINUSEQ;
      break;
    case '*':
      if (c2 == '=')
	return MULTEQ;
      break;
    case '/':
      if (c2 == '=')
	return DIVEQ;
      break;
    case '|':
      if (c2 == '=')
	return OREQ;
      if (c2 == '|')
	return OROR;
      break;
    case '&':
      if (c2 == '=')
	return ANDEQ;
      if (c2 == '&')
	return ANDAND;
      break;
    case '>':
      if (c2 == '=')
	return GE;
      if (c2 == '>')
	return RSHIFT;
      break;
    case '<':
      if (c2 == '=')
	return LE;
      if (c2 == '<')
	return LSHIFT;
      break;
    default:
      break;
    }
  return 0;
}

// If C1 is a valid operator, return the opcode.  Otherwise return 0.

inline int
Lex::one_char_operator(char c1)
{
  switch (c1)
    {
    case '+':
    case '-':
    case '*':
    case '/':
    case '%':
    case '!':
    case '&':
    case '|':
    case '^':
    case '~':
    case '<':
    case '>':
    case '=':
    case '?':
    case ',':
    case '(':
    case ')':
    case '{':
    case '}':
    case '[':
    case ']':
    case ':':
    case ';':
      return c1;
    default:
      return 0;
    }
}

// Skip a C style comment.  *PP points to just after the "/*".  Return
// false if the comment did not end.

bool
Lex::skip_c_comment(const char** pp)
{
  const char* p = *pp;
  while (p[0] != '*' || p[1] != '/')
    {
      if (*p == '\0')
	{
	  *pp = p;
	  return false;
	}

      if (*p == '\n')
	{
	  ++this->lineno_;
	  this->linestart_ = p + 1;
	}
      ++p;
    }

  *pp = p + 2;
  return true;
}

// Skip a line # comment.  Return false if there was no newline.

bool
Lex::skip_line_comment(const char** pp)
{
  const char* p = *pp;
  size_t skip = strcspn(p, "\n");
  if (p[skip] == '\0')
    {
      *pp = p + skip;
      return false;
    }

  p += skip + 1;
  ++this->lineno_;
  this->linestart_ = p;
  *pp = p;

  return true;
}

// Build a token CLASSIFICATION from all characters that match
// CAN_CONTINUE_FN.  Update *PP.

inline Token
Lex::gather_token(Token::Classification classification,
		  const char* (Lex::*can_continue_fn)(const char*),
		  const char* start,
		  const char* match,
		  const char** pp)
{
  const char* new_match = NULL;
  while ((new_match = (this->*can_continue_fn)(match)) != NULL)
    match = new_match;

  // A special case: integers may be followed by a single M or K,
  // case-insensitive.
  if (classification == Token::TOKEN_INTEGER
      && (*match == 'm' || *match == 'M' || *match == 'k' || *match == 'K'))
    ++match;

  *pp = match;
  return this->make_token(classification, start, match - start, start);
}

// Build a token from a quoted string.

Token
Lex::gather_quoted_string(const char** pp)
{
  const char* start = *pp;
  const char* p = start;
  ++p;
  size_t skip = strcspn(p, "\"\n");
  if (p[skip] != '"')
    return this->make_invalid_token(start);
  *pp = p + skip + 1;
  return this->make_token(Token::TOKEN_QUOTED_STRING, p, skip, start);
}

// Return the next token at *PP.  Update *PP.  General guideline: we
// require linker scripts to be simple ASCII.  No unicode linker
// scripts.  In particular we can assume that any '\0' is the end of
// the input.

Token
Lex::get_token(const char** pp)
{
  const char* p = *pp;

  while (true)
    {
      // Skip whitespace quickly.
      while (*p == ' ' || *p == '\t' || *p == '\r')
	++p;

      if (*p == '\n')
	{
	  ++p;
	  ++this->lineno_;
	  this->linestart_ = p;
	  continue;
	}

      char c0 = *p;

      if (c0 == '\0')
	{
	  *pp = p;
	  return this->make_eof_token(p);
	}

      char c1 = p[1];

      // Skip C style comments.
      if (c0 == '/' && c1 == '*')
	{
	  int lineno = this->lineno_;
	  int charpos = p - this->linestart_ + 1;

	  *pp = p + 2;
	  if (!this->skip_c_comment(pp))
	    return Token(Token::TOKEN_INVALID, lineno, charpos);
	  p = *pp;

	  continue;
	}

      // Skip line comments.
      if (c0 == '#')
	{
	  *pp = p + 1;
	  if (!this->skip_line_comment(pp))
	    return this->make_eof_token(p);
	  p = *pp;
	  continue;
	}

      // Check for a name.
      if (this->can_start_name(c0, c1))
	return this->gather_token(Token::TOKEN_STRING,
				  &Lex::can_continue_name,
				  p, p + 1, pp);

      // We accept any arbitrary name in double quotes, as long as it
      // does not cross a line boundary.
      if (*p == '"')
	{
	  *pp = p;
	  return this->gather_quoted_string(pp);
	}

      // Be careful not to lookahead past the end of the buffer.
      char c2 = (c1 == '\0' ? '\0' : p[2]);

      // Check for a number.

      if (this->can_start_hex(c0, c1, c2))
	return this->gather_token(Token::TOKEN_INTEGER,
				  &Lex::can_continue_hex,
				  p, p + 3, pp);

      if (Lex::can_start_number(c0))
	return this->gather_token(Token::TOKEN_INTEGER,
				  &Lex::can_continue_number,
				  p, p + 1, pp);

      // Check for operators.

      int opcode = Lex::three_char_operator(c0, c1, c2);
      if (opcode != 0)
	{
	  *pp = p + 3;
	  return this->make_token(opcode, p);
	}

      opcode = Lex::two_char_operator(c0, c1);
      if (opcode != 0)
	{
	  *pp = p + 2;
	  return this->make_token(opcode, p);
	}

      opcode = Lex::one_char_operator(c0);
      if (opcode != 0)
	{
	  *pp = p + 1;
	  return this->make_token(opcode, p);
	}

      return this->make_token(Token::TOKEN_INVALID, p);
    }
}

// Return the next token.

const Token*
Lex::next_token()
{
  // The first token is special.
  if (this->first_token_ != 0)
    {
      this->token_ = Token(this->first_token_, 0, 0);
      this->first_token_ = 0;
      return &this->token_;
    }

  this->token_ = this->get_token(&this->current_);

  // Don't let an early null byte fool us into thinking that we've
  // reached the end of the file.
  if (this->token_.is_eof()
      && (static_cast<size_t>(this->current_ - this->input_string_)
	  < this->input_length_))
    this->token_ = this->make_invalid_token(this->current_);

  return &this->token_;
}

// class Symbol_assignment.

// Add the symbol to the symbol table.  This makes sure the symbol is
// there and defined.  The actual value is stored later.  We can't
// determine the actual value at this point, because we can't
// necessarily evaluate the expression until all ordinary symbols have
// been finalized.

// The GNU linker lets symbol assignments in the linker script
// silently override defined symbols in object files.  We are
// compatible.  FIXME: Should we issue a warning?

void
Symbol_assignment::add_to_table(Symbol_table* symtab)
{
  elfcpp::STV vis = this->hidden_ ? elfcpp::STV_HIDDEN : elfcpp::STV_DEFAULT;
  this->sym_ = symtab->define_as_constant(this->name_.c_str(),
					  NULL, // version
					  (this->is_defsym_
					   ? Symbol_table::DEFSYM
					   : Symbol_table::SCRIPT),
					  0, // value
					  0, // size
					  elfcpp::STT_NOTYPE,
					  elfcpp::STB_GLOBAL,
					  vis,
					  0, // nonvis
					  this->provide_,
                                          true); // force_override
}

// Finalize a symbol value.

void
Symbol_assignment::finalize(Symbol_table* symtab, const Layout* layout)
{
  this->finalize_maybe_dot(symtab, layout, false, 0, NULL);
}

// Finalize a symbol value which can refer to the dot symbol.

void
Symbol_assignment::finalize_with_dot(Symbol_table* symtab,
				     const Layout* layout,
				     uint64_t dot_value,
				     Output_section* dot_section)
{
  this->finalize_maybe_dot(symtab, layout, true, dot_value, dot_section);
}

// Finalize a symbol value, internal version.

void
Symbol_assignment::finalize_maybe_dot(Symbol_table* symtab,
				      const Layout* layout,
				      bool is_dot_available,
				      uint64_t dot_value,
				      Output_section* dot_section)
{
  // If we were only supposed to provide this symbol, the sym_ field
  // will be NULL if the symbol was not referenced.
  if (this->sym_ == NULL)
    {
      gold_assert(this->provide_);
      return;
    }

  if (parameters->target().get_size() == 32)
    {
#if defined(HAVE_TARGET_32_LITTLE) || defined(HAVE_TARGET_32_BIG)
      this->sized_finalize<32>(symtab, layout, is_dot_available, dot_value,
			       dot_section);
#else
      gold_unreachable();
#endif
    }
  else if (parameters->target().get_size() == 64)
    {
#if defined(HAVE_TARGET_64_LITTLE) || defined(HAVE_TARGET_64_BIG)
      this->sized_finalize<64>(symtab, layout, is_dot_available, dot_value,
			       dot_section);
#else
      gold_unreachable();
#endif
    }
  else
    gold_unreachable();
}

template<int size>
void
Symbol_assignment::sized_finalize(Symbol_table* symtab, const Layout* layout,
				  bool is_dot_available, uint64_t dot_value,
				  Output_section* dot_section)
{
  Output_section* section;
  elfcpp::STT type = elfcpp::STT_NOTYPE;
  elfcpp::STV vis = elfcpp::STV_DEFAULT;
  unsigned char nonvis = 0;
  uint64_t final_val = this->val_->eval_maybe_dot(symtab, layout, true,
						  is_dot_available,
						  dot_value, dot_section,
						  &section, NULL, &type,
						  &vis, &nonvis, false, NULL);
  Sized_symbol<size>* ssym = symtab->get_sized_symbol<size>(this->sym_);
  ssym->set_value(final_val);
  ssym->set_type(type);
  ssym->set_visibility(vis);
  ssym->set_nonvis(nonvis);
  if (section != NULL)
    ssym->set_output_section(section);
}

// Set the symbol value if the expression yields an absolute value or
// a value relative to DOT_SECTION.

void
Symbol_assignment::set_if_absolute(Symbol_table* symtab, const Layout* layout,
				   bool is_dot_available, uint64_t dot_value,
				   Output_section* dot_section)
{
  if (this->sym_ == NULL)
    return;

  Output_section* val_section;
  bool is_valid;
  uint64_t val = this->val_->eval_maybe_dot(symtab, layout, false,
					    is_dot_available, dot_value,
					    dot_section, &val_section, NULL,
					    NULL, NULL, NULL, false, &is_valid);
  if (!is_valid || (val_section != NULL && val_section != dot_section))
    return;

  if (parameters->target().get_size() == 32)
    {
#if defined(HAVE_TARGET_32_LITTLE) || defined(HAVE_TARGET_32_BIG)
      Sized_symbol<32>* ssym = symtab->get_sized_symbol<32>(this->sym_);
      ssym->set_value(val);
#else
      gold_unreachable();
#endif
    }
  else if (parameters->target().get_size() == 64)
    {
#if defined(HAVE_TARGET_64_LITTLE) || defined(HAVE_TARGET_64_BIG)
      Sized_symbol<64>* ssym = symtab->get_sized_symbol<64>(this->sym_);
      ssym->set_value(val);
#else
      gold_unreachable();
#endif
    }
  else
    gold_unreachable();
  if (val_section != NULL)
    this->sym_->set_output_section(val_section);
}

// Print for debugging.

void
Symbol_assignment::print(FILE* f) const
{
  if (this->provide_ && this->hidden_)
    fprintf(f, "PROVIDE_HIDDEN(");
  else if (this->provide_)
    fprintf(f, "PROVIDE(");
  else if (this->hidden_)
    gold_unreachable();

  fprintf(f, "%s = ", this->name_.c_str());
  this->val_->print(f);

  if (this->provide_ || this->hidden_)
    fprintf(f, ")");

  fprintf(f, "\n");
}

// Class Script_assertion.

// Check the assertion.

void
Script_assertion::check(const Symbol_table* symtab, const Layout* layout)
{
  if (!this->check_->eval(symtab, layout, true))
    gold_error("%s", this->message_.c_str());
}

// Print for debugging.

void
Script_assertion::print(FILE* f) const
{
  fprintf(f, "ASSERT(");
  this->check_->print(f);
  fprintf(f, ", \"%s\")\n", this->message_.c_str());
}

// Class Script_options.

Script_options::Script_options()
  : entry_(), symbol_assignments_(), symbol_definitions_(),
    symbol_references_(), version_script_info_(), script_sections_()
{
}

// Returns true if NAME is on the list of symbol assignments waiting
// to be processed.

bool
Script_options::is_pending_assignment(const char* name)
{
  for (Symbol_assignments::iterator p = this->symbol_assignments_.begin();
       p != this->symbol_assignments_.end();
       ++p)
    if ((*p)->name() == name)
      return true;
  return false;
}

// Populates the set with symbols defined in defsym LHS.

void Script_options::find_defsym_defs(Unordered_set<std::string>& defsym_set)
{
  for (Symbol_assignments::const_iterator p = this->symbol_assignments_.begin();
       p != this->symbol_assignments_.end();
       ++p)
    {
      defsym_set.insert((*p)->name());
    }
}

void
Script_options::set_defsym_uses_in_real_elf(Symbol_table* symtab) const
{
  for (Symbol_assignments::const_iterator p = this->symbol_assignments_.begin();
       p != this->symbol_assignments_.end();
       ++p)
    {
      (*p)->value()->set_expr_sym_in_real_elf(symtab);
    }
}

// Add a symbol to be defined.

void
Script_options::add_symbol_assignment(const char* name, size_t length,
				      bool is_defsym, Expression* value,
				      bool provide, bool hidden)
{
  if (length != 1 || name[0] != '.')
    {
      if (this->script_sections_.in_sections_clause())
	{
	  gold_assert(!is_defsym);
	  this->script_sections_.add_symbol_assignment(name, length, value,
						       provide, hidden);
	}
      else
	{
	  Symbol_assignment* p = new Symbol_assignment(name, length, is_defsym,
						       value, provide, hidden);
	  this->symbol_assignments_.push_back(p);
	}

      if (!provide)
	{
	  std::string n(name, length);
	  this->symbol_definitions_.insert(n);
	  this->symbol_references_.erase(n);
	}
    }
  else
    {
      if (provide || hidden)
	gold_error(_("invalid use of PROVIDE for dot symbol"));

      // The GNU linker permits assignments to dot outside of SECTIONS
      // clauses and treats them as occurring inside, so we don't
      // check in_sections_clause here.
      this->script_sections_.add_dot_assignment(value);
    }
}

// Add a reference to a symbol.

void
Script_options::add_symbol_reference(const char* name, size_t length)
{
  if (length != 1 || name[0] != '.')
    {
      std::string n(name, length);
      if (this->symbol_definitions_.find(n) == this->symbol_definitions_.end())
	this->symbol_references_.insert(n);
    }
}

// Add an assertion.

void
Script_options::add_assertion(Expression* check, const char* message,
			      size_t messagelen)
{
  if (this->script_sections_.in_sections_clause())
    this->script_sections_.add_assertion(check, message, messagelen);
  else
    {
      Script_assertion* p = new Script_assertion(check, message, messagelen);
      this->assertions_.push_back(p);
    }
}

// Create sections required by any linker scripts.

void
Script_options::create_script_sections(Layout* layout)
{
  if (this->saw_sections_clause())
    this->script_sections_.create_sections(layout);
}

// Add any symbols we are defining to the symbol table.

void
Script_options::add_symbols_to_table(Symbol_table* symtab)
{
  for (Symbol_assignments::iterator p = this->symbol_assignments_.begin();
       p != this->symbol_assignments_.end();
       ++p)
    (*p)->add_to_table(symtab);
  this->script_sections_.add_symbols_to_table(symtab);
}

// Finalize symbol values.  Also check assertions.

void
Script_options::finalize_symbols(Symbol_table* symtab, const Layout* layout)
{
  // We finalize the symbols defined in SECTIONS first, because they
  // are the ones which may have changed.  This way if symbol outside
  // SECTIONS are defined in terms of symbols inside SECTIONS, they
  // will get the right value.
  this->script_sections_.finalize_symbols(symtab, layout);

  for (Symbol_assignments::iterator p = this->symbol_assignments_.begin();
       p != this->symbol_assignments_.end();
       ++p)
    (*p)->finalize(symtab, layout);

  for (Assertions::iterator p = this->assertions_.begin();
       p != this->assertions_.end();
       ++p)
    (*p)->check(symtab, layout);
}

// Set section addresses.  We set all the symbols which have absolute
// values.  Then we let the SECTIONS clause do its thing.  This
// returns the segment which holds the file header and segment
// headers, if any.

Output_segment*
Script_options::set_section_addresses(Symbol_table* symtab, Layout* layout)
{
  for (Symbol_assignments::iterator p = this->symbol_assignments_.begin();
       p != this->symbol_assignments_.end();
       ++p)
    (*p)->set_if_absolute(symtab, layout, false, 0, NULL);

  return this->script_sections_.set_section_addresses(symtab, layout);
}

// This class holds data passed through the parser to the lexer and to
// the parser support functions.  This avoids global variables.  We
// can't use global variables because we need not be called by a
// singleton thread.

class Parser_closure
{
 public:
  Parser_closure(const char* filename,
		 const Position_dependent_options& posdep_options,
		 bool parsing_defsym, bool in_group, bool is_in_sysroot,
                 Command_line* command_line,
		 Script_options* script_options,
		 Lex* lex,
		 bool skip_on_incompatible_target,
		 Script_info* script_info)
    : filename_(filename), posdep_options_(posdep_options),
      parsing_defsym_(parsing_defsym), in_group_(in_group),
      is_in_sysroot_(is_in_sysroot),
      skip_on_incompatible_target_(skip_on_incompatible_target),
      found_incompatible_target_(false),
      command_line_(command_line), script_options_(script_options),
      version_script_info_(script_options->version_script_info()),
      lex_(lex), lineno_(0), charpos_(0), lex_mode_stack_(), inputs_(NULL),
      script_info_(script_info)
  {
    // We start out processing C symbols in the default lex mode.
    this->language_stack_.push_back(Version_script_info::LANGUAGE_C);
    this->lex_mode_stack_.push_back(lex->mode());
  }

  // Return the file name.
  const char*
  filename() const
  { return this->filename_; }

  // Return the position dependent options.  The caller may modify
  // this.
  Position_dependent_options&
  position_dependent_options()
  { return this->posdep_options_; }

  // Whether we are parsing a --defsym.
  bool
  parsing_defsym() const
  { return this->parsing_defsym_; }

  // Return whether this script is being run in a group.
  bool
  in_group() const
  { return this->in_group_; }

  // Return whether this script was found using a directory in the
  // sysroot.
  bool
  is_in_sysroot() const
  { return this->is_in_sysroot_; }

  // Whether to skip to the next file with the same name if we find an
  // incompatible target in an OUTPUT_FORMAT statement.
  bool
  skip_on_incompatible_target() const
  { return this->skip_on_incompatible_target_; }

  // Stop skipping to the next file on an incompatible target.  This
  // is called when we make some unrevocable change to the data
  // structures.
  void
  clear_skip_on_incompatible_target()
  { this->skip_on_incompatible_target_ = false; }

  // Whether we found an incompatible target in an OUTPUT_FORMAT
  // statement.
  bool
  found_incompatible_target() const
  { return this->found_incompatible_target_; }

  // Note that we found an incompatible target.
  void
  set_found_incompatible_target()
  { this->found_incompatible_target_ = true; }

  // Returns the Command_line structure passed in at constructor time.
  // This value may be NULL.  The caller may modify this, which modifies
  // the passed-in Command_line object (not a copy).
  Command_line*
  command_line()
  { return this->command_line_; }

  // Return the options which may be set by a script.
  Script_options*
  script_options()
  { return this->script_options_; }

  // Return the object in which version script information should be stored.
  Version_script_info*
  version_script()
  { return this->version_script_info_; }

  // Return the next token, and advance.
  const Token*
  next_token()
  {
    const Token* token = this->lex_->next_token();
    this->lineno_ = token->lineno();
    this->charpos_ = token->charpos();
    return token;
  }

  // Set a new lexer mode, pushing the current one.
  void
  push_lex_mode(Lex::Mode mode)
  {
    this->lex_mode_stack_.push_back(this->lex_->mode());
    this->lex_->set_mode(mode);
  }

  // Pop the lexer mode.
  void
  pop_lex_mode()
  {
    gold_assert(!this->lex_mode_stack_.empty());
    this->lex_->set_mode(this->lex_mode_stack_.back());
    this->lex_mode_stack_.pop_back();
  }

  // Return the current lexer mode.
  Lex::Mode
  lex_mode() const
  { return this->lex_mode_stack_.back(); }

  // Return the line number of the last token.
  int
  lineno() const
  { return this->lineno_; }

  // Return the character position in the line of the last token.
  int
  charpos() const
  { return this->charpos_; }

  // Return the list of input files, creating it if necessary.  This
  // is a space leak--we never free the INPUTS_ pointer.
  Input_arguments*
  inputs()
  {
    if (this->inputs_ == NULL)
      this->inputs_ = new Input_arguments();
    return this->inputs_;
  }

  // Return whether we saw any input files.
  bool
  saw_inputs() const
  { return this->inputs_ != NULL && !this->inputs_->empty(); }

  // Return the current language being processed in a version script
  // (eg, "C++").  The empty string represents unmangled C names.
  Version_script_info::Language
  get_current_language() const
  { return this->language_stack_.back(); }

  // Push a language onto the stack when entering an extern block.
  void
  push_language(Version_script_info::Language lang)
  { this->language_stack_.push_back(lang); }

  // Pop a language off of the stack when exiting an extern block.
  void
  pop_language()
  {
    gold_assert(!this->language_stack_.empty());
    this->language_stack_.pop_back();
  }

  // Return a pointer to the incremental info.
  Script_info*
  script_info()
  { return this->script_info_; }

 private:
  // The name of the file we are reading.
  const char* filename_;
  // The position dependent options.
  Position_dependent_options posdep_options_;
  // True if we are parsing a --defsym.
  bool parsing_defsym_;
  // Whether we are currently in a --start-group/--end-group.
  bool in_group_;
  // Whether the script was found in a sysrooted directory.
  bool is_in_sysroot_;
  // If this is true, then if we find an OUTPUT_FORMAT with an
  // incompatible target, then we tell the parser to abort so that we
  // can search for the next file with the same name.
  bool skip_on_incompatible_target_;
  // True if we found an OUTPUT_FORMAT with an incompatible target.
  bool found_incompatible_target_;
  // May be NULL if the user chooses not to pass one in.
  Command_line* command_line_;
  // Options which may be set from any linker script.
  Script_options* script_options_;
  // Information parsed from a version script.
  Version_script_info* version_script_info_;
  // The lexer.
  Lex* lex_;
  // The line number of the last token returned by next_token.
  int lineno_;
  // The column number of the last token returned by next_token.
  int charpos_;
  // A stack of lexer modes.
  std::vector<Lex::Mode> lex_mode_stack_;
  // A stack of which extern/language block we're inside. Can be C++,
  // java, or empty for C.
  std::vector<Version_script_info::Language> language_stack_;
  // New input files found to add to the link.
  Input_arguments* inputs_;
  // Pointer to incremental linking info.
  Script_info* script_info_;
};

// FILE was found as an argument on the command line.  Try to read it
// as a script.  Return true if the file was handled.

bool
read_input_script(Workqueue* workqueue, Symbol_table* symtab, Layout* layout,
		  Dirsearch* dirsearch, int dirindex,
		  Input_objects* input_objects, Mapfile* mapfile,
		  Input_group* input_group,
		  const Input_argument* input_argument,
		  Input_file* input_file, Task_token* next_blocker,
		  bool* used_next_blocker)
{
  *used_next_blocker = false;

  std::string input_string;
  Lex::read_file(input_file, &input_string);

  Lex lex(input_string.c_str(), input_string.length(), PARSING_LINKER_SCRIPT);

  Script_info* script_info = NULL;
  if (layout->incremental_inputs() != NULL)
    {
      const std::string& filename = input_file->filename();
      Timespec mtime = input_file->file().get_mtime();
      unsigned int arg_serial = input_argument->file().arg_serial();
      script_info = new Script_info(filename);
      layout->incremental_inputs()->report_script(script_info, arg_serial,
						  mtime);
    }

  Parser_closure closure(input_file->filename().c_str(),
			 input_argument->file().options(),
			 false,
			 input_group != NULL,
			 input_file->is_in_sysroot(),
                         NULL,
			 layout->script_options(),
			 &lex,
			 input_file->will_search_for(),
			 script_info);

  bool old_saw_sections_clause =
    layout->script_options()->saw_sections_clause();

  if (yyparse(&closure) != 0)
    {
      if (closure.found_incompatible_target())
	{
	  Read_symbols::incompatible_warning(input_argument, input_file);
	  Read_symbols::requeue(workqueue, input_objects, symtab, layout,
				dirsearch, dirindex, mapfile, input_argument,
				input_group, next_blocker);
	  return true;
	}
      return false;
    }

  if (!old_saw_sections_clause
      && layout->script_options()->saw_sections_clause()
      && layout->have_added_input_section())
    gold_error(_("%s: SECTIONS seen after other input files; try -T/--script"),
	       input_file->filename().c_str());

  if (!closure.saw_inputs())
    return true;

  Task_token* this_blocker = NULL;
  for (Input_arguments::const_iterator p = closure.inputs()->begin();
       p != closure.inputs()->end();
       ++p)
    {
      Task_token* nb;
      if (p + 1 == closure.inputs()->end())
	nb = next_blocker;
      else
	{
	  nb = new Task_token(true);
	  nb->add_blocker();
	}
      workqueue->queue_soon(new Read_symbols(input_objects, symtab,
					     layout, dirsearch, 0, mapfile, &*p,
					     input_group, NULL, this_blocker, nb));
      this_blocker = nb;
    }

  *used_next_blocker = true;

  return true;
}

// Helper function for read_version_script(), read_commandline_script() and
// script_include_directive().  Processes the given file in the mode indicated
// by first_token and lex_mode.

static bool
read_script_file(const char* filename, Command_line* cmdline,
                 Script_options* script_options,
                 int first_token, Lex::Mode lex_mode)
{
  Dirsearch dirsearch;
  std::string name = filename;

  // If filename is a relative filename, search for it manually using "." +
  // cmdline->options()->library_path() -- not dirsearch.
  if (!IS_ABSOLUTE_PATH(filename))
    {
      const General_options::Dir_list& search_path =
          cmdline->options().library_path();
      name = Dirsearch::find_file_in_dir_list(name, search_path, ".");
    }

  // The file locking code wants to record a Task, but we haven't
  // started the workqueue yet.  This is only for debugging purposes,
  // so we invent a fake value.
  const Task* task = reinterpret_cast<const Task*>(-1);

  // We don't want this file to be opened in binary mode.
  Position_dependent_options posdep = cmdline->position_dependent_options();
  if (posdep.format_enum() == General_options::OBJECT_FORMAT_BINARY)
    posdep.set_format_enum(General_options::OBJECT_FORMAT_ELF);
  Input_file_argument input_argument(name.c_str(),
				     Input_file_argument::INPUT_FILE_TYPE_FILE,
				     "", false, posdep);
  Input_file input_file(&input_argument);
  int dummy = 0;
  if (!input_file.open(dirsearch, task, &dummy))
    return false;

  std::string input_string;
  Lex::read_file(&input_file, &input_string);

  Lex lex(input_string.c_str(), input_string.length(), first_token);
  lex.set_mode(lex_mode);

  Parser_closure closure(filename,
			 cmdline->position_dependent_options(),
			 first_token == Lex::DYNAMIC_LIST,
			 false,
			 input_file.is_in_sysroot(),
                         cmdline,
			 script_options,
			 &lex,
			 false,
			 NULL);
  if (yyparse(&closure) != 0)
    {
      input_file.file().unlock(task);
      return false;
    }

  input_file.file().unlock(task);

  gold_assert(!closure.saw_inputs());

  return true;
}

// FILENAME was found as an argument to --script (-T).
// Read it as a script, and execute its contents immediately.

bool
read_commandline_script(const char* filename, Command_line* cmdline)
{
  return read_script_file(filename, cmdline, &cmdline->script_options(),
                          PARSING_LINKER_SCRIPT, Lex::LINKER_SCRIPT);
}

// FILENAME was found as an argument to --version-script.  Read it as
// a version script, and store its contents in
// cmdline->script_options()->version_script_info().

bool
read_version_script(const char* filename, Command_line* cmdline)
{
  return read_script_file(filename, cmdline, &cmdline->script_options(),
                          PARSING_VERSION_SCRIPT, Lex::VERSION_SCRIPT);
}

// FILENAME was found as an argument to --dynamic-list.  Read it as a
// list of symbols, and store its contents in DYNAMIC_LIST.

bool
read_dynamic_list(const char* filename, Command_line* cmdline,
                  Script_options* dynamic_list)
{
  return read_script_file(filename, cmdline, dynamic_list,
                          PARSING_DYNAMIC_LIST, Lex::DYNAMIC_LIST);
}

// Implement the --defsym option on the command line.  Return true if
// all is well.

bool
Script_options::define_symbol(const char* definition)
{
  Lex lex(definition, strlen(definition), PARSING_DEFSYM);
  lex.set_mode(Lex::EXPRESSION);

  // Dummy value.
  Position_dependent_options posdep_options;

  Parser_closure closure("command line", posdep_options, true,
			 false, false, NULL, this, &lex, false, NULL);

  if (yyparse(&closure) != 0)
    return false;

  gold_assert(!closure.saw_inputs());

  return true;
}

// Print the script to F for debugging.

void
Script_options::print(FILE* f) const
{
  fprintf(f, "%s: Dumping linker script\n", program_name);

  if (!this->entry_.empty())
    fprintf(f, "ENTRY(%s)\n", this->entry_.c_str());

  for (Symbol_assignments::const_iterator p =
	 this->symbol_assignments_.begin();
       p != this->symbol_assignments_.end();
       ++p)
    (*p)->print(f);

  for (Assertions::const_iterator p = this->assertions_.begin();
       p != this->assertions_.end();
       ++p)
    (*p)->print(f);

  this->script_sections_.print(f);

  this->version_script_info_.print(f);
}

// Manage mapping from keywords to the codes expected by the bison
// parser.  We construct one global object for each lex mode with
// keywords.

class Keyword_to_parsecode
{
 public:
  // The structure which maps keywords to parsecodes.
  struct Keyword_parsecode
  {
    // Keyword.
    const char* keyword;
    // Corresponding parsecode.
    int parsecode;
  };

  Keyword_to_parsecode(const Keyword_parsecode* keywords,
                       int keyword_count)
      : keyword_parsecodes_(keywords), keyword_count_(keyword_count)
  { }

  // Return the parsecode corresponding KEYWORD, or 0 if it is not a
  // keyword.
  int
  keyword_to_parsecode(const char* keyword, size_t len) const;

 private:
  const Keyword_parsecode* keyword_parsecodes_;
  const int keyword_count_;
};

// Mapping from keyword string to keyword parsecode.  This array must
// be kept in sorted order.  Parsecodes are looked up using bsearch.
// This array must correspond to the list of parsecodes in yyscript.y.

static const Keyword_to_parsecode::Keyword_parsecode
script_keyword_parsecodes[] =
{
  { "ABSOLUTE", ABSOLUTE },
  { "ADDR", ADDR },
  { "ALIGN", ALIGN_K },
  { "ALIGNOF", ALIGNOF },
  { "ASSERT", ASSERT_K },
  { "AS_NEEDED", AS_NEEDED },
  { "AT", AT },
  { "BIND", BIND },
  { "BLOCK", BLOCK },
  { "BYTE", BYTE },
  { "CONSTANT", CONSTANT },
  { "CONSTRUCTORS", CONSTRUCTORS },
  { "COPY", COPY },
  { "CREATE_OBJECT_SYMBOLS", CREATE_OBJECT_SYMBOLS },
  { "DATA_SEGMENT_ALIGN", DATA_SEGMENT_ALIGN },
  { "DATA_SEGMENT_END", DATA_SEGMENT_END },
  { "DATA_SEGMENT_RELRO_END", DATA_SEGMENT_RELRO_END },
  { "DEFINED", DEFINED },
  { "DSECT", DSECT },
  { "ENTRY", ENTRY },
  { "EXCLUDE_FILE", EXCLUDE_FILE },
  { "EXTERN", EXTERN },
  { "FILL", FILL },
  { "FLOAT", FLOAT },
  { "FORCE_COMMON_ALLOCATION", FORCE_COMMON_ALLOCATION },
  { "GROUP", GROUP },
  { "HIDDEN", HIDDEN },
  { "HLL", HLL },
  { "INCLUDE", INCLUDE },
  { "INFO", INFO },
  { "INHIBIT_COMMON_ALLOCATION", INHIBIT_COMMON_ALLOCATION },
  { "INPUT", INPUT },
  { "KEEP", KEEP },
  { "LENGTH", LENGTH },
  { "LOADADDR", LOADADDR },
  { "LONG", LONG },
  { "MAP", MAP },
  { "MAX", MAX_K },
  { "MEMORY", MEMORY },
  { "MIN", MIN_K },
  { "NEXT", NEXT },
  { "NOCROSSREFS", NOCROSSREFS },
  { "NOFLOAT", NOFLOAT },
  { "NOLOAD", NOLOAD },
  { "ONLY_IF_RO", ONLY_IF_RO },
  { "ONLY_IF_RW", ONLY_IF_RW },
  { "OPTION", OPTION },
  { "ORIGIN", ORIGIN },
  { "OUTPUT", OUTPUT },
  { "OUTPUT_ARCH", OUTPUT_ARCH },
  { "OUTPUT_FORMAT", OUTPUT_FORMAT },
  { "OVERLAY", OVERLAY },
  { "PHDRS", PHDRS },
  { "PROVIDE", PROVIDE },
  { "PROVIDE_HIDDEN", PROVIDE_HIDDEN },
  { "QUAD", QUAD },
  { "SEARCH_DIR", SEARCH_DIR },
  { "SECTIONS", SECTIONS },
  { "SEGMENT_START", SEGMENT_START },
  { "SHORT", SHORT },
  { "SIZEOF", SIZEOF },
  { "SIZEOF_HEADERS", SIZEOF_HEADERS },
  { "SORT", SORT_BY_NAME },
  { "SORT_BY_ALIGNMENT", SORT_BY_ALIGNMENT },
  { "SORT_BY_INIT_PRIORITY", SORT_BY_INIT_PRIORITY },
  { "SORT_BY_NAME", SORT_BY_NAME },
  { "SPECIAL", SPECIAL },
  { "SQUAD", SQUAD },
  { "STARTUP", STARTUP },
  { "SUBALIGN", SUBALIGN },
  { "SYSLIB", SYSLIB },
  { "TARGET", TARGET_K },
  { "TRUNCATE", TRUNCATE },
  { "VERSION", VERSIONK },
  { "global", GLOBAL },
  { "l", LENGTH },
  { "len", LENGTH },
  { "local", LOCAL },
  { "o", ORIGIN },
  { "org", ORIGIN },
  { "sizeof_headers", SIZEOF_HEADERS },
};

static const Keyword_to_parsecode
script_keywords(&script_keyword_parsecodes[0],
                (sizeof(script_keyword_parsecodes)
                 / sizeof(script_keyword_parsecodes[0])));

static const Keyword_to_parsecode::Keyword_parsecode
version_script_keyword_parsecodes[] =
{
  { "extern", EXTERN },
  { "global", GLOBAL },
  { "local", LOCAL },
};

static const Keyword_to_parsecode
version_script_keywords(&version_script_keyword_parsecodes[0],
                        (sizeof(version_script_keyword_parsecodes)
                         / sizeof(version_script_keyword_parsecodes[0])));

static const Keyword_to_parsecode::Keyword_parsecode
dynamic_list_keyword_parsecodes[] =
{
  { "extern", EXTERN },
};

static const Keyword_to_parsecode
dynamic_list_keywords(&dynamic_list_keyword_parsecodes[0],
                      (sizeof(dynamic_list_keyword_parsecodes)
                       / sizeof(dynamic_list_keyword_parsecodes[0])));



// Comparison function passed to bsearch.

extern "C"
{

struct Ktt_key
{
  const char* str;
  size_t len;
};

static int
ktt_compare(const void* keyv, const void* kttv)
{
  const Ktt_key* key = static_cast<const Ktt_key*>(keyv);
  const Keyword_to_parsecode::Keyword_parsecode* ktt =
    static_cast<const Keyword_to_parsecode::Keyword_parsecode*>(kttv);
  int i = strncmp(key->str, ktt->keyword, key->len);
  if (i != 0)
    return i;
  if (ktt->keyword[key->len] != '\0')
    return -1;
  return 0;
}

} // End extern "C".

int
Keyword_to_parsecode::keyword_to_parsecode(const char* keyword,
                                           size_t len) const
{
  Ktt_key key;
  key.str = keyword;
  key.len = len;
  void* kttv = bsearch(&key,
                       this->keyword_parsecodes_,
                       this->keyword_count_,
                       sizeof(this->keyword_parsecodes_[0]),
                       ktt_compare);
  if (kttv == NULL)
    return 0;
  Keyword_parsecode* ktt = static_cast<Keyword_parsecode*>(kttv);
  return ktt->parsecode;
}

// The following structs are used within the VersionInfo class as well
// as in the bison helper functions.  They store the information
// parsed from the version script.

// A single version expression.
// For example, pattern="std::map*" and language="C++".
struct Version_expression
{
  Version_expression(const std::string& a_pattern,
		     Version_script_info::Language a_language,
                     bool a_exact_match)
    : pattern(a_pattern), language(a_language), exact_match(a_exact_match),
      was_matched_by_symbol(false)
  { }

  std::string pattern;
  Version_script_info::Language language;
  // If false, we use glob() to match pattern.  If true, we use strcmp().
  bool exact_match;
  // True if --no-undefined-version is in effect and we found this
  // version in get_symbol_version.  We use mutable because this
  // struct is generally not modifiable after it has been created.
  mutable bool was_matched_by_symbol;
};

// A list of expressions.
struct Version_expression_list
{
  std::vector<struct Version_expression> expressions;
};

// A list of which versions upon which another version depends.
// Strings should be from the Stringpool.
struct Version_dependency_list
{
  std::vector<std::string> dependencies;
};

// The total definition of a version.  It includes the tag for the
// version, its global and local expressions, and any dependencies.
struct Version_tree
{
  Version_tree()
      : tag(), global(NULL), local(NULL), dependencies(NULL)
  { }

  std::string tag;
  const struct Version_expression_list* global;
  const struct Version_expression_list* local;
  const struct Version_dependency_list* dependencies;
};

// Helper class that calls cplus_demangle when needed and takes care of freeing
// the result.

class Lazy_demangler
{
 public:
  Lazy_demangler(const char* symbol, int options)
    : symbol_(symbol), options_(options), demangled_(NULL), did_demangle_(false)
  { }

  ~Lazy_demangler()
  { free(this->demangled_); }

  // Return the demangled name. The actual demangling happens on the first call,
  // and the result is later cached.
  inline char*
  get();

 private:
  // The symbol to demangle.
  const char* symbol_;
  // Option flags to pass to cplus_demagle.
  const int options_;
  // The cached demangled value, or NULL if demangling didn't happen yet or
  // failed.
  char* demangled_;
  // Whether we already called cplus_demangle
  bool did_demangle_;
};

// Return the demangled name. The actual demangling happens on the first call,
// and the result is later cached. Returns NULL if the symbol cannot be
// demangled.

inline char*
Lazy_demangler::get()
{
  if (!this->did_demangle_)
    {
      this->demangled_ = cplus_demangle(this->symbol_, this->options_);
      this->did_demangle_ = true;
    }
  return this->demangled_;
}

// Class Version_script_info.

Version_script_info::Version_script_info()
  : dependency_lists_(), expression_lists_(), version_trees_(), globs_(),
    default_version_(NULL), default_is_global_(false), is_finalized_(false)
{
  for (int i = 0; i < LANGUAGE_COUNT; ++i)
    this->exact_[i] = NULL;
}

Version_script_info::~Version_script_info()
{
}

// Forget all the known version script information.

void
Version_script_info::clear()
{
  for (size_t k = 0; k < this->dependency_lists_.size(); ++k)
    delete this->dependency_lists_[k];
  this->dependency_lists_.clear();
  for (size_t k = 0; k < this->version_trees_.size(); ++k)
    delete this->version_trees_[k];
  this->version_trees_.clear();
  for (size_t k = 0; k < this->expression_lists_.size(); ++k)
    delete this->expression_lists_[k];
  this->expression_lists_.clear();
}

// Finalize the version script information.

void
Version_script_info::finalize()
{
  if (!this->is_finalized_)
    {
      this->build_lookup_tables();
      this->is_finalized_ = true;
    }
}

// Return all the versions.

std::vector<std::string>
Version_script_info::get_versions() const
{
  std::vector<std::string> ret;
  for (size_t j = 0; j < this->version_trees_.size(); ++j)
    if (!this->version_trees_[j]->tag.empty())
      ret.push_back(this->version_trees_[j]->tag);
  return ret;
}

// Return the dependencies of VERSION.

std::vector<std::string>
Version_script_info::get_dependencies(const char* version) const
{
  std::vector<std::string> ret;
  for (size_t j = 0; j < this->version_trees_.size(); ++j)
    if (this->version_trees_[j]->tag == version)
      {
        const struct Version_dependency_list* deps =
          this->version_trees_[j]->dependencies;
        if (deps != NULL)
          for (size_t k = 0; k < deps->dependencies.size(); ++k)
            ret.push_back(deps->dependencies[k]);
        return ret;
      }
  return ret;
}

// A version script essentially maps a symbol name to a version tag
// and an indication of whether symbol is global or local within that
// version tag.  Each symbol maps to at most one version tag.
// Unfortunately, in practice, version scripts are ambiguous, and list
// symbols multiple times.  Thus, we have to document the matching
// process.

// This is a description of what the GNU linker does as of 2010-01-11.
// It walks through the version tags in the order in which they appear
// in the version script.  For each tag, it first walks through the
// global patterns for that tag, then the local patterns.  When
// looking at a single pattern, it first applies any language specific
// demangling as specified for the pattern, and then matches the
// resulting symbol name to the pattern.  If it finds an exact match
// for a literal pattern (a pattern enclosed in quotes or with no
// wildcard characters), then that is the match that it uses.  If
// finds a match with a wildcard pattern, then it saves it and
// continues searching.  Wildcard patterns that are exactly "*" are
// saved separately.

// If no exact match with a literal pattern is ever found, then if a
// wildcard match with a global pattern was found it is used,
// otherwise if a wildcard match with a local pattern was found it is
// used.

// This is the result:
//   * If there is an exact match, then we use the first tag in the
//     version script where it matches.
//     + If the exact match in that tag is global, it is used.
//     + Otherwise the exact match in that tag is local, and is used.
//   * Otherwise, if there is any match with a global wildcard pattern:
//     + If there is any match with a wildcard pattern which is not
//       "*", then we use the tag in which the *last* such pattern
//       appears.
//     + Otherwise, we matched "*".  If there is no match with a local
//       wildcard pattern which is not "*", then we use the *last*
//       match with a global "*".  Otherwise, continue.
//   * Otherwise, if there is any match with a local wildcard pattern:
//     + If there is any match with a wildcard pattern which is not
//       "*", then we use the tag in which the *last* such pattern
//       appears.
//     + Otherwise, we matched "*", and we use the tag in which the
//       *last* such match occurred.

// There is an additional wrinkle.  When the GNU linker finds a symbol
// with a version defined in an object file due to a .symver
// directive, it looks up that symbol name in that version tag.  If it
// finds it, it matches the symbol name against the patterns for that
// version.  If there is no match with a global pattern, but there is
// a match with a local pattern, then the GNU linker marks the symbol
// as local.

// We want gold to be generally compatible, but we also want gold to
// be fast.  These are the rules that gold implements:
//   * If there is an exact match for the mangled name, we use it.
//     + If there is more than one exact match, we give a warning, and
//       we use the first tag in the script which matches.
//     + If a symbol has an exact match as both global and local for
//       the same version tag, we give an error.
//   * Otherwise, we look for an extern C++ or an extern Java exact
//     match.  If we find an exact match, we use it.
//     + If there is more than one exact match, we give a warning, and
//       we use the first tag in the script which matches.
//     + If a symbol has an exact match as both global and local for
//       the same version tag, we give an error.
//   * Otherwise, we look through the wildcard patterns, ignoring "*"
//     patterns.  We look through the version tags in reverse order.
//     For each version tag, we look through the global patterns and
//     then the local patterns.  We use the first match we find (i.e.,
//     the last matching version tag in the file).
//   * Otherwise, we use the "*" pattern if there is one.  We give an
//     error if there are multiple "*" patterns.

// At least for now, gold does not look up the version tag for a
// symbol version found in an object file to see if it should be
// forced local.  There are other ways to force a symbol to be local,
// and I don't understand why this one is useful.

// Build a set of fast lookup tables for a version script.

void
Version_script_info::build_lookup_tables()
{
  size_t size = this->version_trees_.size();
  for (size_t j = 0; j < size; ++j)
    {
      const Version_tree* v = this->version_trees_[j];
      this->build_expression_list_lookup(v->local, v, false);
      this->build_expression_list_lookup(v->global, v, true);
    }
}

// If a pattern has backlashes but no unquoted wildcard characters,
// then we apply backslash unquoting and look for an exact match.
// Otherwise we treat it as a wildcard pattern.  This function returns
// true for a wildcard pattern.  Otherwise, it does backslash
// unquoting on *PATTERN and returns false.  If this returns true,
// *PATTERN may have been partially unquoted.

bool
Version_script_info::unquote(std::string* pattern) const
{
  bool saw_backslash = false;
  size_t len = pattern->length();
  size_t j = 0;
  for (size_t i = 0; i < len; ++i)
    {
      if (saw_backslash)
	saw_backslash = false;
      else
	{
	  switch ((*pattern)[i])
	    {
	    case '?': case '[': case '*':
	      return true;
	    case '\\':
	      saw_backslash = true;
	      continue;
	    default:
	      break;
	    }
	}

      if (i != j)
	(*pattern)[j] = (*pattern)[i];
      ++j;
    }
  return false;
}

// Add an exact match for MATCH to *PE.  The result of the match is
// V/IS_GLOBAL.

void
Version_script_info::add_exact_match(const std::string& match,
				     const Version_tree* v, bool is_global,
				     const Version_expression* ve,
				     Exact* pe)
{
  std::pair<Exact::iterator, bool> ins =
    pe->insert(std::make_pair(match, Version_tree_match(v, is_global, ve)));
  if (ins.second)
    {
      // This is the first time we have seen this match.
      return;
    }

  Version_tree_match& vtm(ins.first->second);
  if (vtm.real->tag != v->tag)
    {
      // This is an ambiguous match.  We still return the
      // first version that we found in the script, but we
      // record the new version to issue a warning if we
      // wind up looking up this symbol.
      if (vtm.ambiguous == NULL)
	vtm.ambiguous = v;
    }
  else if (is_global != vtm.is_global)
    {
      // We have a match for both the global and local entries for a
      // version tag.  That's got to be wrong.
      gold_error(_("'%s' appears as both a global and a local symbol "
		   "for version '%s' in script"),
		 match.c_str(), v->tag.c_str());
    }
}

// Build fast lookup information for EXPLIST and store it in LOOKUP.
// All matches go to V, and IS_GLOBAL is true if they are global
// matches.

void
Version_script_info::build_expression_list_lookup(
    const Version_expression_list* explist,
    const Version_tree* v,
    bool is_global)
{
  if (explist == NULL)
    return;
  size_t size = explist->expressions.size();
  for (size_t i = 0; i < size; ++i)
    {
      const Version_expression& exp(explist->expressions[i]);

      if (exp.pattern.length() == 1 && exp.pattern[0] == '*')
	{
	  if (this->default_version_ != NULL
	      && this->default_version_->tag != v->tag)
	    gold_warning(_("wildcard match appears in both version '%s' "
			   "and '%s' in script"),
			 this->default_version_->tag.c_str(), v->tag.c_str());
	  else if (this->default_version_ != NULL
		   && this->default_is_global_ != is_global)
	    gold_error(_("wildcard match appears as both global and local "
			 "in version '%s' in script"),
		       v->tag.c_str());
	  this->default_version_ = v;
	  this->default_is_global_ = is_global;
	  continue;
	}

      std::string pattern = exp.pattern;
      if (!exp.exact_match)
	{
	  if (this->unquote(&pattern))
	    {
	      this->globs_.push_back(Glob(&exp, v, is_global));
	      continue;
	    }
	}

      if (this->exact_[exp.language] == NULL)
	this->exact_[exp.language] = new Exact();
      this->add_exact_match(pattern, v, is_global, &exp,
			    this->exact_[exp.language]);
    }
}

// Return the name to match given a name, a language code, and two
// lazy demanglers.

const char*
Version_script_info::get_name_to_match(const char* name,
				       int language,
				       Lazy_demangler* cpp_demangler,
				       Lazy_demangler* java_demangler) const
{
  switch (language)
    {
    case LANGUAGE_C:
      return name;
    case LANGUAGE_CXX:
      return cpp_demangler->get();
    case LANGUAGE_JAVA:
      return java_demangler->get();
    default:
      gold_unreachable();
    }
}

// Look up SYMBOL_NAME in the list of versions.  Return true if the
// symbol is found, false if not.  If the symbol is found, then if
// PVERSION is not NULL, set *PVERSION to the version tag, and if
// P_IS_GLOBAL is not NULL, set *P_IS_GLOBAL according to whether the
// symbol is global or not.

bool
Version_script_info::get_symbol_version(const char* symbol_name,
					std::string* pversion,
					bool* p_is_global) const
{
  Lazy_demangler cpp_demangled_name(symbol_name, DMGL_ANSI | DMGL_PARAMS);
  Lazy_demangler java_demangled_name(symbol_name,
				     DMGL_ANSI | DMGL_PARAMS | DMGL_JAVA);

  gold_assert(this->is_finalized_);
  for (int i = 0; i < LANGUAGE_COUNT; ++i)
    {
      Exact* exact = this->exact_[i];
      if (exact == NULL)
	continue;

      const char* name_to_match = this->get_name_to_match(symbol_name, i,
							  &cpp_demangled_name,
							  &java_demangled_name);
      if (name_to_match == NULL)
	{
	  // If the name can not be demangled, the GNU linker goes
	  // ahead and tries to match it anyhow.  That does not
	  // make sense to me and I have not implemented it.
	  continue;
	}

      Exact::const_iterator pe = exact->find(name_to_match);
      if (pe != exact->end())
	{
	  const Version_tree_match& vtm(pe->second);
	  if (vtm.ambiguous != NULL)
	    gold_warning(_("using '%s' as version for '%s' which is also "
			   "named in version '%s' in script"),
			 vtm.real->tag.c_str(), name_to_match,
			 vtm.ambiguous->tag.c_str());

	  if (pversion != NULL)
	    *pversion = vtm.real->tag;
	  if (p_is_global != NULL)
	    *p_is_global = vtm.is_global;

	  // If we are using --no-undefined-version, and this is a
	  // global symbol, we have to record that we have found this
	  // symbol, so that we don't warn about it.  We have to do
	  // this now, because otherwise we have no way to get from a
	  // non-C language back to the demangled name that we
	  // matched.
	  if (p_is_global != NULL && vtm.is_global)
	    vtm.expression->was_matched_by_symbol = true;

	  return true;
	}
    }

  // Look through the glob patterns in reverse order.

  for (Globs::const_reverse_iterator p = this->globs_.rbegin();
       p != this->globs_.rend();
       ++p)
    {
      int language = p->expression->language;
      const char* name_to_match = this->get_name_to_match(symbol_name,
							  language,
							  &cpp_demangled_name,
							  &java_demangled_name);
      if (name_to_match == NULL)
	continue;

      if (fnmatch(p->expression->pattern.c_str(), name_to_match,
		  FNM_NOESCAPE) == 0)
	{
	  if (pversion != NULL)
	    *pversion = p->version->tag;
	  if (p_is_global != NULL)
	    *p_is_global = p->is_global;
	  return true;
	}
    }

  // Finally, there may be a wildcard.
  if (this->default_version_ != NULL)
    {
      if (pversion != NULL)
	*pversion = this->default_version_->tag;
      if (p_is_global != NULL)
	*p_is_global = this->default_is_global_;
      return true;
    }

  return false;
}

// Give an error if any exact symbol names (not wildcards) appear in a
// version script, but there is no such symbol.

void
Version_script_info::check_unmatched_names(const Symbol_table* symtab) const
{
  for (size_t i = 0; i < this->version_trees_.size(); ++i)
    {
      const Version_tree* vt = this->version_trees_[i];
      if (vt->global == NULL)
	continue;
      for (size_t j = 0; j < vt->global->expressions.size(); ++j)
	{
	  const Version_expression& expression(vt->global->expressions[j]);

	  // Ignore cases where we used the version because we saw a
	  // symbol that we looked up.  Note that
	  // WAS_MATCHED_BY_SYMBOL will be true even if the symbol was
	  // not a definition.  That's OK as in that case we most
	  // likely gave an undefined symbol error anyhow.
	  if (expression.was_matched_by_symbol)
	    continue;

	  // Just ignore names which are in languages other than C.
	  // We have no way to look them up in the symbol table.
	  if (expression.language != LANGUAGE_C)
	    continue;

	  // Remove backslash quoting, and ignore wildcard patterns.
	  std::string pattern = expression.pattern;
	  if (!expression.exact_match)
	    {
	      if (this->unquote(&pattern))
		continue;
	    }

	  if (symtab->lookup(pattern.c_str(), vt->tag.c_str()) == NULL)
	    gold_error(_("version script assignment of %s to symbol %s "
			 "failed: symbol not defined"),
		       vt->tag.c_str(), pattern.c_str());
	}
    }
}

struct Version_dependency_list*
Version_script_info::allocate_dependency_list()
{
  dependency_lists_.push_back(new Version_dependency_list);
  return dependency_lists_.back();
}

struct Version_expression_list*
Version_script_info::allocate_expression_list()
{
  expression_lists_.push_back(new Version_expression_list);
  return expression_lists_.back();
}

struct Version_tree*
Version_script_info::allocate_version_tree()
{
  version_trees_.push_back(new Version_tree);
  return version_trees_.back();
}

// Print for debugging.

void
Version_script_info::print(FILE* f) const
{
  if (this->empty())
    return;

  fprintf(f, "VERSION {");

  for (size_t i = 0; i < this->version_trees_.size(); ++i)
    {
      const Version_tree* vt = this->version_trees_[i];

      if (vt->tag.empty())
	fprintf(f, "  {\n");
      else
	fprintf(f, "  %s {\n", vt->tag.c_str());

      if (vt->global != NULL)
	{
	  fprintf(f, "    global :\n");
	  this->print_expression_list(f, vt->global);
	}

      if (vt->local != NULL)
	{
	  fprintf(f, "    local :\n");
	  this->print_expression_list(f, vt->local);
	}

      fprintf(f, "  }");
      if (vt->dependencies != NULL)
	{
	  const Version_dependency_list* deps = vt->dependencies;
	  for (size_t j = 0; j < deps->dependencies.size(); ++j)
	    {
	      if (j < deps->dependencies.size() - 1)
		fprintf(f, "\n");
	      fprintf(f, "    %s", deps->dependencies[j].c_str());
	    }
	}
      fprintf(f, ";\n");
    }

  fprintf(f, "}\n");
}

void
Version_script_info::print_expression_list(
    FILE* f,
    const Version_expression_list* vel) const
{
  Version_script_info::Language current_language = LANGUAGE_C;
  for (size_t i = 0; i < vel->expressions.size(); ++i)
    {
      const Version_expression& ve(vel->expressions[i]);

      if (ve.language != current_language)
	{
	  if (current_language != LANGUAGE_C)
	    fprintf(f, "      }\n");
	  switch (ve.language)
	    {
	    case LANGUAGE_C:
	      break;
	    case LANGUAGE_CXX:
	      fprintf(f, "      extern \"C++\" {\n");
	      break;
	    case LANGUAGE_JAVA:
	      fprintf(f, "      extern \"Java\" {\n");
	      break;
	    default:
	      gold_unreachable();
	    }
	  current_language = ve.language;
	}

      fprintf(f, "      ");
      if (current_language != LANGUAGE_C)
	fprintf(f, "  ");

      if (ve.exact_match)
	fprintf(f, "\"");
      fprintf(f, "%s", ve.pattern.c_str());
      if (ve.exact_match)
	fprintf(f, "\"");

      fprintf(f, "\n");
    }

  if (current_language != LANGUAGE_C)
    fprintf(f, "      }\n");
}

} // End namespace gold.

// The remaining functions are extern "C", so it's clearer to not put
// them in namespace gold.

using namespace gold;

// This function is called by the bison parser to return the next
// token.

extern "C" int
yylex(YYSTYPE* lvalp, void* closurev)
{
  Parser_closure* closure = static_cast<Parser_closure*>(closurev);
  const Token* token = closure->next_token();
  switch (token->classification())
    {
    default:
      gold_unreachable();

    case Token::TOKEN_INVALID:
      yyerror(closurev, "invalid character");
      return 0;

    case Token::TOKEN_EOF:
      return 0;

    case Token::TOKEN_STRING:
      {
	// This is either a keyword or a STRING.
	size_t len;
	const char* str = token->string_value(&len);
	int parsecode = 0;
        switch (closure->lex_mode())
          {
          case Lex::LINKER_SCRIPT:
            parsecode = script_keywords.keyword_to_parsecode(str, len);
            break;
          case Lex::VERSION_SCRIPT:
            parsecode = version_script_keywords.keyword_to_parsecode(str, len);
            break;
          case Lex::DYNAMIC_LIST:
            parsecode = dynamic_list_keywords.keyword_to_parsecode(str, len);
            break;
          default:
            break;
          }
	if (parsecode != 0)
	  return parsecode;
	lvalp->string.value = str;
	lvalp->string.length = len;
	return STRING;
      }

    case Token::TOKEN_QUOTED_STRING:
      lvalp->string.value = token->string_value(&lvalp->string.length);
      return QUOTED_STRING;

    case Token::TOKEN_OPERATOR:
      return token->operator_value();

    case Token::TOKEN_INTEGER:
      lvalp->integer = token->integer_value();
      return INTEGER;
    }
}

// This function is called by the bison parser to report an error.

extern "C" void
yyerror(void* closurev, const char* message)
{
  Parser_closure* closure = static_cast<Parser_closure*>(closurev);
  gold_error(_("%s:%d:%d: %s"), closure->filename(), closure->lineno(),
	     closure->charpos(), message);
}

// Called by the bison parser to add an external symbol to the link.

extern "C" void
script_add_extern(void* closurev, const char* name, size_t length)
{
  Parser_closure* closure = static_cast<Parser_closure*>(closurev);
  closure->script_options()->add_symbol_reference(name, length);
}

// Called by the bison parser to add a file to the link.

extern "C" void
script_add_file(void* closurev, const char* name, size_t length)
{
  Parser_closure* closure = static_cast<Parser_closure*>(closurev);

  // If this is an absolute path, and we found the script in the
  // sysroot, then we want to prepend the sysroot to the file name.
  // For example, this is how we handle a cross link to the x86_64
  // libc.so, which refers to /lib/libc.so.6.
  std::string name_string(name, length);
  const char* extra_search_path = ".";
  std::string script_directory;
  if (IS_ABSOLUTE_PATH(name_string.c_str()))
    {
      if (closure->is_in_sysroot())
	{
	  const std::string& sysroot(parameters->options().sysroot());
	  gold_assert(!sysroot.empty());
	  name_string = sysroot + name_string;
	}
    }
  else
    {
      // In addition to checking the normal library search path, we
      // also want to check in the script-directory.
      const char* slash = strrchr(closure->filename(), '/');
      if (slash != NULL)
	{
	  script_directory.assign(closure->filename(),
				  slash - closure->filename() + 1);
	  extra_search_path = script_directory.c_str();
	}
    }

  Input_file_argument file(name_string.c_str(),
			   Input_file_argument::INPUT_FILE_TYPE_FILE,
			   extra_search_path, false,
			   closure->position_dependent_options());
  Input_argument& arg = closure->inputs()->add_file(file);
  arg.set_script_info(closure->script_info());
}

// Called by the bison parser to add a library to the link.

extern "C" void
script_add_library(void* closurev, const char* name, size_t length)
{
  Parser_closure* closure = static_cast<Parser_closure*>(closurev);
  std::string name_string(name, length);

  if (name_string[0] != 'l')
    gold_error(_("library name must be prefixed with -l"));

  Input_file_argument file(name_string.c_str() + 1,
			   Input_file_argument::INPUT_FILE_TYPE_LIBRARY,
			   "", false,
			   closure->position_dependent_options());
  Input_argument& arg = closure->inputs()->add_file(file);
  arg.set_script_info(closure->script_info());
}

// Called by the bison parser to start a group.  If we are already in
// a group, that means that this script was invoked within a
// --start-group --end-group sequence on the command line, or that
// this script was found in a GROUP of another script.  In that case,
// we simply continue the existing group, rather than starting a new
// one.  It is possible to construct a case in which this will do
// something other than what would happen if we did a recursive group,
// but it's hard to imagine why the different behaviour would be
// useful for a real program.  Avoiding recursive groups is simpler
// and more efficient.

extern "C" void
script_start_group(void* closurev)
{
  Parser_closure* closure = static_cast<Parser_closure*>(closurev);
  if (!closure->in_group())
    closure->inputs()->start_group();
}

// Called by the bison parser at the end of a group.

extern "C" void
script_end_group(void* closurev)
{
  Parser_closure* closure = static_cast<Parser_closure*>(closurev);
  if (!closure->in_group())
    closure->inputs()->end_group();
}

// Called by the bison parser to start an AS_NEEDED list.

extern "C" void
script_start_as_needed(void* closurev)
{
  Parser_closure* closure = static_cast<Parser_closure*>(closurev);
  closure->position_dependent_options().set_as_needed(true);
}

// Called by the bison parser at the end of an AS_NEEDED list.

extern "C" void
script_end_as_needed(void* closurev)
{
  Parser_closure* closure = static_cast<Parser_closure*>(closurev);
  closure->position_dependent_options().set_as_needed(false);
}

// Called by the bison parser to set the entry symbol.

extern "C" void
script_set_entry(void* closurev, const char* entry, size_t length)
{
  // We'll parse this exactly the same as --entry=ENTRY on the commandline
  // TODO(csilvers): FIXME -- call set_entry directly.
  std::string arg("--entry=");
  arg.append(entry, length);
  script_parse_option(closurev, arg.c_str(), arg.size());
}

// Called by the bison parser to set whether to define common symbols.

extern "C" void
script_set_common_allocation(void* closurev, int set)
{
  const char* arg = set != 0 ? "--define-common" : "--no-define-common";
  script_parse_option(closurev, arg, strlen(arg));
}

// Called by the bison parser to refer to a symbol.

extern "C" Expression*
script_symbol(void* closurev, const char* name, size_t length)
{
  Parser_closure* closure = static_cast<Parser_closure*>(closurev);
  if (length != 1 || name[0] != '.')
    closure->script_options()->add_symbol_reference(name, length);
  return script_exp_string(name, length);
}

// Called by the bison parser to define a symbol.

extern "C" void
script_set_symbol(void* closurev, const char* name, size_t length,
		  Expression* value, int providei, int hiddeni)
{
  Parser_closure* closure = static_cast<Parser_closure*>(closurev);
  const bool provide = providei != 0;
  const bool hidden = hiddeni != 0;
  closure->script_options()->add_symbol_assignment(name, length,
						   closure->parsing_defsym(),
						   value, provide, hidden);
  closure->clear_skip_on_incompatible_target();
}

// Called by the bison parser to add an assertion.

extern "C" void
script_add_assertion(void* closurev, Expression* check, const char* message,
		     size_t messagelen)
{
  Parser_closure* closure = static_cast<Parser_closure*>(closurev);
  closure->script_options()->add_assertion(check, message, messagelen);
  closure->clear_skip_on_incompatible_target();
}

// Called by the bison parser to parse an OPTION.

extern "C" void
script_parse_option(void* closurev, const char* option, size_t length)
{
  Parser_closure* closure = static_cast<Parser_closure*>(closurev);
  // We treat the option as a single command-line option, even if
  // it has internal whitespace.
  if (closure->command_line() == NULL)
    {
      // There are some options that we could handle here--e.g.,
      // -lLIBRARY.  Should we bother?
      gold_warning(_("%s:%d:%d: ignoring command OPTION; OPTION is only valid"
		     " for scripts specified via -T/--script"),
		   closure->filename(), closure->lineno(), closure->charpos());
    }
  else
    {
      bool past_a_double_dash_option = false;
      const char* mutable_option = strndup(option, length);
      gold_assert(mutable_option != NULL);
      closure->command_line()->process_one_option(1, &mutable_option, 0,
                                                  &past_a_double_dash_option);
      // The General_options class will quite possibly store a pointer
      // into mutable_option, so we can't free it.  In cases the class
      // does not store such a pointer, this is a memory leak.  Alas. :(
    }
  closure->clear_skip_on_incompatible_target();
}

// Called by the bison parser to handle OUTPUT_FORMAT.  OUTPUT_FORMAT
// takes either one or three arguments.  In the three argument case,
// the format depends on the endianness option, which we don't
// currently support (FIXME).  If we see an OUTPUT_FORMAT for the
// wrong format, then we want to search for a new file.  Returning 0
// here will cause the parser to immediately abort.

extern "C" int
script_check_output_format(void* closurev,
			   const char* default_name, size_t default_length,
			   const char*, size_t, const char*, size_t)
{
  Parser_closure* closure = static_cast<Parser_closure*>(closurev);
  std::string name(default_name, default_length);
  Target* target = select_target_by_bfd_name(name.c_str());
  if (target == NULL || !parameters->is_compatible_target(target))
    {
      if (closure->skip_on_incompatible_target())
	{
	  closure->set_found_incompatible_target();
	  return 0;
	}
      // FIXME: Should we warn about the unknown target?
    }
  return 1;
}

// Called by the bison parser to handle TARGET.

extern "C" void
script_set_target(void* closurev, const char* target, size_t len)
{
  Parser_closure* closure = static_cast<Parser_closure*>(closurev);
  std::string s(target, len);
  General_options::Object_format format_enum;
  format_enum = General_options::string_to_object_format(s.c_str());
  closure->position_dependent_options().set_format_enum(format_enum);
}

// Called by the bison parser to handle SEARCH_DIR.  This is handled
// exactly like a -L option.

extern "C" void
script_add_search_dir(void* closurev, const char* option, size_t length)
{
  Parser_closure* closure = static_cast<Parser_closure*>(closurev);
  if (closure->command_line() == NULL)
    gold_warning(_("%s:%d:%d: ignoring SEARCH_DIR; SEARCH_DIR is only valid"
		   " for scripts specified via -T/--script"),
		 closure->filename(), closure->lineno(), closure->charpos());
  else if (!closure->command_line()->options().nostdlib())
    {
      std::string s = "-L" + std::string(option, length);
      script_parse_option(closurev, s.c_str(), s.size());
    }
}

/* Called by the bison parser to push the lexer into expression
   mode.  */

extern "C" void
script_push_lex_into_expression_mode(void* closurev)
{
  Parser_closure* closure = static_cast<Parser_closure*>(closurev);
  closure->push_lex_mode(Lex::EXPRESSION);
}

/* Called by the bison parser to push the lexer into version
   mode.  */

extern "C" void
script_push_lex_into_version_mode(void* closurev)
{
  Parser_closure* closure = static_cast<Parser_closure*>(closurev);
  if (closure->version_script()->is_finalized())
    gold_error(_("%s:%d:%d: invalid use of VERSION in input file"),
	       closure->filename(), closure->lineno(), closure->charpos());
  closure->push_lex_mode(Lex::VERSION_SCRIPT);
}

/* Called by the bison parser to pop the lexer mode.  */

extern "C" void
script_pop_lex_mode(void* closurev)
{
  Parser_closure* closure = static_cast<Parser_closure*>(closurev);
  closure->pop_lex_mode();
}

// Register an entire version node. For example:
//
// GLIBC_2.1 {
//   global: foo;
// } GLIBC_2.0;
//
// - tag is "GLIBC_2.1"
// - tree contains the information "global: foo"
// - deps contains "GLIBC_2.0"

extern "C" void
script_register_vers_node(void*,
			  const char* tag,
			  int taglen,
			  struct Version_tree* tree,
			  struct Version_dependency_list* deps)
{
  gold_assert(tree != NULL);
  tree->dependencies = deps;
  if (tag != NULL)
    tree->tag = std::string(tag, taglen);
}

// Add a dependencies to the list of existing dependencies, if any,
// and return the expanded list.

extern "C" struct Version_dependency_list*
script_add_vers_depend(void* closurev,
		       struct Version_dependency_list* all_deps,
		       const char* depend_to_add, int deplen)
{
  Parser_closure* closure = static_cast<Parser_closure*>(closurev);
  if (all_deps == NULL)
    all_deps = closure->version_script()->allocate_dependency_list();
  all_deps->dependencies.push_back(std::string(depend_to_add, deplen));
  return all_deps;
}

// Add a pattern expression to an existing list of expressions, if any.

extern "C" struct Version_expression_list*
script_new_vers_pattern(void* closurev,
			struct Version_expression_list* expressions,
			const char* pattern, int patlen, int exact_match)
{
  Parser_closure* closure = static_cast<Parser_closure*>(closurev);
  if (expressions == NULL)
    expressions = closure->version_script()->allocate_expression_list();
  expressions->expressions.push_back(
      Version_expression(std::string(pattern, patlen),
                         closure->get_current_language(),
                         static_cast<bool>(exact_match)));
  return expressions;
}

// Attaches b to the end of a, and clears b.  So a = a + b and b = {}.

extern "C" struct Version_expression_list*
script_merge_expressions(struct Version_expression_list* a,
                         struct Version_expression_list* b)
{
  a->expressions.insert(a->expressions.end(),
                        b->expressions.begin(), b->expressions.end());
  // We could delete b and remove it from expressions_lists_, but
  // that's a lot of work.  This works just as well.
  b->expressions.clear();
  return a;
}

// Combine the global and local expressions into a a Version_tree.

extern "C" struct Version_tree*
script_new_vers_node(void* closurev,
		     struct Version_expression_list* global,
		     struct Version_expression_list* local)
{
  Parser_closure* closure = static_cast<Parser_closure*>(closurev);
  Version_tree* tree = closure->version_script()->allocate_version_tree();
  tree->global = global;
  tree->local = local;
  return tree;
}

// Handle a transition in language, such as at the
// start or end of 'extern "C++"'

extern "C" void
version_script_push_lang(void* closurev, const char* lang, int langlen)
{
  Parser_closure* closure = static_cast<Parser_closure*>(closurev);
  std::string language(lang, langlen);
  Version_script_info::Language code;
  if (language.empty() || language == "C")
    code = Version_script_info::LANGUAGE_C;
  else if (language == "C++")
    code = Version_script_info::LANGUAGE_CXX;
  else if (language == "Java")
    code = Version_script_info::LANGUAGE_JAVA;
  else
    {
      char* buf = new char[langlen + 100];
      snprintf(buf, langlen + 100,
	       _("unrecognized version script language '%s'"),
	       language.c_str());
      yyerror(closurev, buf);
      delete[] buf;
      code = Version_script_info::LANGUAGE_C;
    }
  closure->push_language(code);
}

extern "C" void
version_script_pop_lang(void* closurev)
{
  Parser_closure* closure = static_cast<Parser_closure*>(closurev);
  closure->pop_language();
}

// Called by the bison parser to start a SECTIONS clause.

extern "C" void
script_start_sections(void* closurev)
{
  Parser_closure* closure = static_cast<Parser_closure*>(closurev);
  closure->script_options()->script_sections()->start_sections();
  closure->clear_skip_on_incompatible_target();
}

// Called by the bison parser to finish a SECTIONS clause.

extern "C" void
script_finish_sections(void* closurev)
{
  Parser_closure* closure = static_cast<Parser_closure*>(closurev);
  closure->script_options()->script_sections()->finish_sections();
}

// Start processing entries for an output section.

extern "C" void
script_start_output_section(void* closurev, const char* name, size_t namelen,
			    const struct Parser_output_section_header* header)
{
  Parser_closure* closure = static_cast<Parser_closure*>(closurev);
  closure->script_options()->script_sections()->start_output_section(name,
								     namelen,
								     header);
}

// Finish processing entries for an output section.

extern "C" void
script_finish_output_section(void* closurev,
			     const struct Parser_output_section_trailer* trail)
{
  Parser_closure* closure = static_cast<Parser_closure*>(closurev);
  closure->script_options()->script_sections()->finish_output_section(trail);
}

// Add a data item (e.g., "WORD (0)") to the current output section.

extern "C" void
script_add_data(void* closurev, int data_token, Expression* val)
{
  Parser_closure* closure = static_cast<Parser_closure*>(closurev);
  int size;
  bool is_signed = true;
  switch (data_token)
    {
    case QUAD:
      size = 8;
      is_signed = false;
      break;
    case SQUAD:
      size = 8;
      break;
    case LONG:
      size = 4;
      break;
    case SHORT:
      size = 2;
      break;
    case BYTE:
      size = 1;
      break;
    default:
      gold_unreachable();
    }
  closure->script_options()->script_sections()->add_data(size, is_signed, val);
}

// Add a clause setting the fill value to the current output section.

extern "C" void
script_add_fill(void* closurev, Expression* val)
{
  Parser_closure* closure = static_cast<Parser_closure*>(closurev);
  closure->script_options()->script_sections()->add_fill(val);
}

// Add a new input section specification to the current output
// section.

extern "C" void
script_add_input_section(void* closurev,
			 const struct Input_section_spec* spec,
			 int keepi)
{
  Parser_closure* closure = static_cast<Parser_closure*>(closurev);
  bool keep = keepi != 0;
  closure->script_options()->script_sections()->add_input_section(spec, keep);
}

// When we see DATA_SEGMENT_ALIGN we record that following output
// sections may be relro.

extern "C" void
script_data_segment_align(void* closurev)
{
  Parser_closure* closure = static_cast<Parser_closure*>(closurev);
  if (!closure->script_options()->saw_sections_clause())
    gold_error(_("%s:%d:%d: DATA_SEGMENT_ALIGN not in SECTIONS clause"),
	       closure->filename(), closure->lineno(), closure->charpos());
  else
    closure->script_options()->script_sections()->data_segment_align();
}

// When we see DATA_SEGMENT_RELRO_END we know that all output sections
// since DATA_SEGMENT_ALIGN should be relro.

extern "C" void
script_data_segment_relro_end(void* closurev)
{
  Parser_closure* closure = static_cast<Parser_closure*>(closurev);
  if (!closure->script_options()->saw_sections_clause())
    gold_error(_("%s:%d:%d: DATA_SEGMENT_ALIGN not in SECTIONS clause"),
	       closure->filename(), closure->lineno(), closure->charpos());
  else
    closure->script_options()->script_sections()->data_segment_relro_end();
}

// Create a new list of string/sort pairs.

extern "C" String_sort_list_ptr
script_new_string_sort_list(const struct Wildcard_section* string_sort)
{
  return new String_sort_list(1, *string_sort);
}

// Add an entry to a list of string/sort pairs.  The way the parser
// works permits us to simply modify the first parameter, rather than
// copy the vector.

extern "C" String_sort_list_ptr
script_string_sort_list_add(String_sort_list_ptr pv,
			    const struct Wildcard_section* string_sort)
{
  if (pv == NULL)
    return script_new_string_sort_list(string_sort);
  else
    {
      pv->push_back(*string_sort);
      return pv;
    }
}

// Create a new list of strings.

extern "C" String_list_ptr
script_new_string_list(const char* str, size_t len)
{
  return new String_list(1, std::string(str, len));
}

// Add an element to a list of strings.  The way the parser works
// permits us to simply modify the first parameter, rather than copy
// the vector.

extern "C" String_list_ptr
script_string_list_push_back(String_list_ptr pv, const char* str, size_t len)
{
  if (pv == NULL)
    return script_new_string_list(str, len);
  else
    {
      pv->push_back(std::string(str, len));
      return pv;
    }
}

// Concatenate two string lists.  Either or both may be NULL.  The way
// the parser works permits us to modify the parameters, rather than
// copy the vector.

extern "C" String_list_ptr
script_string_list_append(String_list_ptr pv1, String_list_ptr pv2)
{
  if (pv1 == NULL)
    return pv2;
  if (pv2 == NULL)
    return pv1;
  pv1->insert(pv1->end(), pv2->begin(), pv2->end());
  return pv1;
}

// Add a new program header.

extern "C" void
script_add_phdr(void* closurev, const char* name, size_t namelen,
		unsigned int type, const Phdr_info* info)
{
  Parser_closure* closure = static_cast<Parser_closure*>(closurev);
  bool includes_filehdr = info->includes_filehdr != 0;
  bool includes_phdrs = info->includes_phdrs != 0;
  bool is_flags_valid = info->is_flags_valid != 0;
  Script_sections* ss = closure->script_options()->script_sections();
  ss->add_phdr(name, namelen, type, includes_filehdr, includes_phdrs,
	       is_flags_valid, info->flags, info->load_address);
  closure->clear_skip_on_incompatible_target();
}

// Convert a program header string to a type.

#define PHDR_TYPE(NAME) { #NAME, sizeof(#NAME) - 1, elfcpp::NAME }

static struct
{
  const char* name;
  size_t namelen;
  unsigned int val;
} phdr_type_names[] =
{
  PHDR_TYPE(PT_NULL),
  PHDR_TYPE(PT_LOAD),
  PHDR_TYPE(PT_DYNAMIC),
  PHDR_TYPE(PT_INTERP),
  PHDR_TYPE(PT_NOTE),
  PHDR_TYPE(PT_SHLIB),
  PHDR_TYPE(PT_PHDR),
  PHDR_TYPE(PT_TLS),
  PHDR_TYPE(PT_GNU_EH_FRAME),
  PHDR_TYPE(PT_GNU_STACK),
  PHDR_TYPE(PT_GNU_RELRO)
};

extern "C" unsigned int
script_phdr_string_to_type(void* closurev, const char* name, size_t namelen)
{
  for (unsigned int i = 0;
       i < sizeof(phdr_type_names) / sizeof(phdr_type_names[0]);
       ++i)
    if (namelen == phdr_type_names[i].namelen
	&& strncmp(name, phdr_type_names[i].name, namelen) == 0)
      return phdr_type_names[i].val;
  yyerror(closurev, _("unknown PHDR type (try integer)"));
  return elfcpp::PT_NULL;
}

extern "C" void
script_saw_segment_start_expression(void* closurev)
{
  Parser_closure* closure = static_cast<Parser_closure*>(closurev);
  Script_sections* ss = closure->script_options()->script_sections();
  ss->set_saw_segment_start_expression(true);
}

extern "C" void
script_set_section_region(void* closurev, const char* name, size_t namelen,
			  int set_vma)
{
  Parser_closure* closure = static_cast<Parser_closure*>(closurev);
  if (!closure->script_options()->saw_sections_clause())
    {
      gold_error(_("%s:%d:%d: MEMORY region '%.*s' referred to outside of "
		   "SECTIONS clause"),
		 closure->filename(), closure->lineno(), closure->charpos(),
		 static_cast<int>(namelen), name);
      return;
    }

  Script_sections* ss = closure->script_options()->script_sections();
  Memory_region* mr = ss->find_memory_region(name, namelen);
  if (mr == NULL)
    {
      gold_error(_("%s:%d:%d: MEMORY region '%.*s' not declared"),
		 closure->filename(), closure->lineno(), closure->charpos(),
		 static_cast<int>(namelen), name);
      return;
    }

  ss->set_memory_region(mr, set_vma);
}

extern "C" void
script_add_memory(void* closurev, const char* name, size_t namelen,
		  unsigned int attrs, Expression* origin, Expression* length)
{
  Parser_closure* closure = static_cast<Parser_closure*>(closurev);
  Script_sections* ss = closure->script_options()->script_sections();
  ss->add_memory_region(name, namelen, attrs, origin, length);
}

extern "C" unsigned int
script_parse_memory_attr(void* closurev, const char* attrs, size_t attrlen,
			 int invert)
{
  int attributes = 0;

  while (attrlen--)
    switch (*attrs++)
      {
      case 'R':
      case 'r':
	attributes |= MEM_READABLE; break;
      case 'W':
      case 'w':
	attributes |= MEM_READABLE | MEM_WRITEABLE; break;
      case 'X':
      case 'x':
	attributes |= MEM_EXECUTABLE; break;
      case 'A':
      case 'a':
	attributes |= MEM_ALLOCATABLE; break;
      case 'I':
      case 'i':
      case 'L':
      case 'l':
	attributes |= MEM_INITIALIZED; break;
      default:
	yyerror(closurev, _("unknown MEMORY attribute"));
      }

  if (invert)
    attributes = (~ attributes) & MEM_ATTR_MASK;

  return attributes;
}

extern "C" void
script_include_directive(int first_token, void* closurev,
			 const char* filename, size_t length)
{
  Parser_closure* closure = static_cast<Parser_closure*>(closurev);
  std::string name(filename, length);
  Command_line* cmdline = closure->command_line();
  read_script_file(name.c_str(), cmdline, &cmdline->script_options(),
                   first_token, Lex::LINKER_SCRIPT);
}

// Functions for memory regions.

extern "C" Expression*
script_exp_function_origin(void* closurev, const char* name, size_t namelen)
{
  Parser_closure* closure = static_cast<Parser_closure*>(closurev);
  Script_sections* ss = closure->script_options()->script_sections();
  Expression* origin = ss->find_memory_region_origin(name, namelen);

  if (origin == NULL)
    {
      gold_error(_("undefined memory region '%s' referenced "
		   "in ORIGIN expression"),
		 name);
      // Create a dummy expression to prevent crashes later on.
      origin = script_exp_integer(0);
    }

  return origin;
}

extern "C" Expression*
script_exp_function_length(void* closurev, const char* name, size_t namelen)
{
  Parser_closure* closure = static_cast<Parser_closure*>(closurev);
  Script_sections* ss = closure->script_options()->script_sections();
  Expression* length = ss->find_memory_region_length(name, namelen);

  if (length == NULL)
    {
      gold_error(_("undefined memory region '%s' referenced "
		   "in LENGTH expression"),
		 name);
      // Create a dummy expression to prevent crashes later on.
      length = script_exp_integer(0);
    }

  return length;
}
