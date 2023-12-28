/* Copyright (C) 2021-2023 Free Software Foundation, Inc.
   Contributed by Oracle.

   This file is part of GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

// To rebuild QLParser.tab.cc and QLParser.tab.hh, use bison 3.6 or newer:
// cd gprofng/src && bison QLParser.yy

// For "api.parser.class"
%require "3.0"
%language "C++"

%code top {
#include <stdio.h>
#include <string.h>
#include <string>
}
%code requires {
#include "QLParser.h"
#include "DbeSession.h"
#include "Expression.h"
#include "Table.h"
#include "i18n.h"
}

%code
{
namespace QL
{
  static QL::Parser::symbol_type yylex (QL::Result &result);

  static Expression *
  processName (std::string str)
  {
    const char *name = str.c_str();
    int propID = dbeSession->getPropIdByName (name);
    if (propID != PROP_NONE)
      return new Expression (Expression::OP_NAME,
		      new Expression (Expression::OP_NUM, (uint64_t) propID));

    // If a name is not statically known try user defined objects
    Expression *expr = dbeSession->findObjDefByName (name);
    if (expr != NULL)
      return expr->copy();

    throw Parser::syntax_error ("Name not found");
  }
}
}

%defines
%define api.namespace {QL}
// in Bison 3.3, use %define api.parser.class {Parser} instead parser_class_name
%define parser_class_name {Parser}
%define api.token.constructor
%define api.value.type variant
// Later: api.value.automove
%define api.token.prefix {L_}
%define parse.assert
%param {QL::Result &result}

%start S

%token LPAR "("
  RPAR ")"
  HASPROP
  FILEIOVFD

%token YYEOF 0
%token <uint64_t> NUM FNAME JGROUP JPARENT QSTR
%token <std::string> NAME

%nonassoc IN SOME ORDR
%left  COMMA ","
%right QWE "?"
       COLON ":"
%left  AND "&&"
       OR "|"
       EQV NEQV
       BITAND BITOR
       BITXOR "^"
%nonassoc EQ "="
	  NE "!="
	  LT "<"
	  GT ">"
	  LE "<="
	  GE ">="
%left LS "<<"
      RS ">>"
      ADD "+"
      MINUS "-"
      MUL "*"
      DIV "/"
      REM "%"
%right DEG
       NOT "!"
       BITNOT "~"

%type <Expression *>  exp term

// %destructor { delete $$; } <Expression *>;

%%

S:	/* empty */		{ result.out = new Expression (Expression::OP_NUM, (uint64_t) 1); }
|	exp			{ result.out = $1; }

exp:	  exp DEG exp		{ $$ = new Expression (Expression::OP_DEG, $1, $3); } /* dead? */
	| exp MUL exp		{ $$ = new Expression (Expression::OP_MUL, $1, $3); }
	| exp DIV exp		{ $$ = new Expression (Expression::OP_DIV, $1, $3); }
	| exp REM exp		{ $$ = new Expression (Expression::OP_REM, $1, $3); }
	| exp ADD exp		{ $$ = new Expression (Expression::OP_ADD, $1, $3); }
	| exp MINUS exp		{ $$ = new Expression (Expression::OP_MINUS, $1, $3); }
	| exp LS  exp		{ $$ = new Expression (Expression::OP_LS, $1, $3); }
	| exp RS  exp		{ $$ = new Expression (Expression::OP_RS, $1, $3); }
	| exp LT  exp		{ $$ = new Expression (Expression::OP_LT, $1, $3); }
	| exp LE  exp		{ $$ = new Expression (Expression::OP_LE, $1, $3); }
	| exp GT  exp		{ $$ = new Expression (Expression::OP_GT, $1, $3); }
	| exp GE  exp		{ $$ = new Expression (Expression::OP_GE, $1, $3); }
	| exp EQ  exp		{ $$ = new Expression (Expression::OP_EQ, $1, $3); }
	| exp NE  exp		{ $$ = new Expression (Expression::OP_NE, $1, $3); }
	| exp BITAND exp	{ $$ = new Expression (Expression::OP_BITAND, $1, $3); }
	| exp BITXOR exp	{ $$ = new Expression (Expression::OP_BITXOR, $1, $3); }
	| exp BITOR  exp	{ $$ = new Expression (Expression::OP_BITOR, $1, $3); }
	| exp AND exp		{ $$ = new Expression (Expression::OP_AND, $1, $3); }
	| exp OR  exp		{ $$ = new Expression (Expression::OP_OR, $1, $3); }
	| exp NEQV exp		{ $$ = new Expression (Expression::OP_NEQV, $1, $3); } /* dead? */
	| exp EQV exp		{ $$ = new Expression (Expression::OP_EQV, $1, $3); } /* dead? */
	| exp QWE exp COLON exp
	  {
	     $$ = new Expression (Expression::OP_QWE, $1,
				  new Expression (Expression::OP_COLON, $3, $5));
	  }
	| exp COMMA  exp	{ $$ = new Expression (Expression::OP_COMMA, $1, $3); }
	| exp IN exp		{ $$ = new Expression (Expression::OP_IN, $1, $3); }
	| exp SOME IN exp	{ $$ = new Expression (Expression::OP_SOMEIN, $1, $4); }
	| exp ORDR IN exp	{ $$ = new Expression (Expression::OP_ORDRIN, $1, $4); }
	| term                  { $$ = $1; }

term:	  MINUS term
	  {
	     $$ = new Expression (Expression::OP_MINUS,
				  new Expression (Expression::OP_NUM, (uint64_t) 0), $2);
	  }
	| NOT    term		{ $$ = new Expression (Expression::OP_NOT, $2); }
	| BITNOT term		{ $$ = new Expression (Expression::OP_BITNOT, $2); }
	| LPAR exp RPAR		{ $$ = $2; }
	| FNAME LPAR QSTR RPAR
	  {
	    $$ = new Expression (Expression::OP_FUNC,
				 new Expression (Expression::OP_NUM, $1),
				 new Expression (Expression::OP_NUM, $3));
	  }
	| HASPROP LPAR NAME RPAR
	  {
	    $$ = new Expression (Expression::OP_HASPROP,
				 new Expression (Expression::OP_NUM, processName($3)));
	  }
	| JGROUP LPAR QSTR RPAR
	  {
	    $$ = new Expression (Expression::OP_JAVA,
				 new Expression (Expression::OP_NUM, $1),
				 new Expression (Expression::OP_NUM, $3));
	  }
	| JPARENT LPAR QSTR RPAR
	  {
	     $$ = new Expression (Expression::OP_JAVA,
				  new Expression (Expression::OP_NUM, $1),
				  new Expression (Expression::OP_NUM, $3));
	  }
	| FILEIOVFD LPAR QSTR RPAR
	  {
	    $$ = new Expression (Expression::OP_FILE,
				 new Expression (Expression::OP_NUM, (uint64_t) 0),
				 new Expression (Expression::OP_NUM, $3));
	  }
	| NUM			{ $$ = new Expression (Expression::OP_NUM, $1); }
	| NAME			{ $$ = processName($1); }

%%

namespace QL
{
  static Parser::symbol_type
  unget_ret (std::istream &in, char c, Parser::symbol_type tok)
  {
    in.putback (c);
    return tok;
  }

  static Parser::symbol_type
  yylex (QL::Result &result)
  {
    int base = 0;
    int c;

    do
      c = result.in.get ();
    while (result.in && (c == ' ' || c == '\t'));
    if (!result.in)
      return Parser::make_YYEOF ();

    switch (c)
      {
      case '\0':
      case '\n': return Parser::make_YYEOF ();
      case '(': return Parser::make_LPAR () ;
      case ')': return Parser::make_RPAR ();
      case ',': return Parser::make_COMMA ();
      case '%': return Parser::make_REM ();
      case '/': return Parser::make_DIV ();
      case '*': return Parser::make_MUL ();
      case '-': return Parser::make_MINUS ();
      case '+': return Parser::make_ADD ();
      case '~': return Parser::make_BITNOT ();
      case '^': return Parser::make_BITXOR ();
      case '?': return Parser::make_QWE ();
      case ':': return Parser::make_COLON ();
      case '|':
	c = result.in.get ();
	if (c == '|')
	  return Parser::make_OR ();
	else
	  return unget_ret (result.in, c, Parser::make_BITOR ());
      case '&':
	c = result.in.get ();
	if (c == '&')
	  return Parser::make_AND ();
	else
	  return unget_ret (result.in, c, Parser::make_BITAND ());
      case '!':
	c = result.in.get ();
	if (c == '=')
	  return Parser::make_NE ();
	else
	  return unget_ret (result.in, c, Parser::make_NOT ());
      case '=':
	c = result.in.get ();
	if (c == '=')
	  return Parser::make_EQ ();
	else
	  throw Parser::syntax_error ("Syntax error after =");
      case '<':
	c = result.in.get ();
	if (c == '=')
	  return Parser::make_LE ();
	else if (c == '<')
	  return Parser::make_LS ();
	else
	  return unget_ret (result.in, c, Parser::make_LT ());
      case '>':
	c = result.in.get ();
	if (c == '=')
	  return Parser::make_GE ();
	else if (c == '>')
	  return Parser::make_RS ();
	else
	  return unget_ret (result.in, c, Parser::make_GT ());
      case '"':
	{
	  int  maxsz = 16;
	  char *str = (char *) malloc (maxsz);
	  char *ptr = str;

	  for (;;)
	    {
	      c = result.in.get ();
	      if (!result.in)
		{
		  free (str);
		  throw Parser::syntax_error ("Unclosed \"");
		}

	      switch (c)
		{
		case '"':
		  *ptr = (char)0;
		  // XXX omazur: need new string type
		  return Parser::make_QSTR ((uint64_t) str);
		case 0:
		case '\n':
		  free (str);
		  throw Parser::syntax_error ("Multiline strings are not supported");
		default:
		  if (ptr - str >= maxsz)
		    {
		      size_t len = ptr - str;
		      maxsz = maxsz > 8192 ? maxsz + 8192 : maxsz * 2;
		      char *new_s = (char *) realloc (str, maxsz);
		      str = new_s;
		      ptr = str + len;
		    }
		  *ptr++ = c;
		}
	    }
	}
      default:
	if (c == '0')
	  {
	    base = 8;
	    c = result.in.get ();
	    if ( c == 'x' )
	      {
		base = 16;
		c = result.in.get ();
	      }
	  }
	else if (c >= '1' && c <='9')
	  base = 10;

	if (base)
	  {
	    uint64_t lval = 0;
	    for (;;)
	      {
		int digit = -1;
		switch (c)
		  {
		  case '0': case '1': case '2': case '3':
		  case '4': case '5': case '6': case '7':
		    digit = c - '0';
		    break;
		  case '8': case '9':
		    if (base > 8)
		      digit = c - '0';
		    break;
		  case 'a': case 'b': case 'c':
		  case 'd': case 'e': case 'f':
		    if (base == 16)
		      digit = c - 'a' + 10;
		    break;
		  case 'A': case 'B': case 'C':
		  case 'D': case 'E': case 'F':
		    if (base == 16)
		      digit = c - 'A' + 10;
		    break;
		  }
		if  (digit == -1)
		  {
		    result.in.putback (c);
		    break;
		  }
		lval = lval * base + digit;
		c = result.in.get ();
	      }
	    return Parser::make_NUM (lval);
	  }

	if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))
	  {
	    char name[32];	// omazur XXX: accept any length
	    name[0] = (char)c;
	    for (size_t i = 1; i < sizeof (name); i++)
	      {
		c = result.in.get ();
		if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
		    (c >= '0' && c <= '9') || (c == '_'))
		  name[i] = c;
		else
		  {
		    name[i] = (char)0;
		    result.in.putback (c);
		    break;
		  }
	      }

	    if (strcasecmp (name, NTXT ("IN")) == 0)
	      return Parser::make_IN ();
	    else if (strcasecmp (name, NTXT ("SOME")) == 0)
	      return Parser::make_SOME ();
	    else if (strcasecmp (name, NTXT ("ORDERED")) == 0)
	      return Parser::make_ORDR ();
	    else if (strcasecmp (name, NTXT ("TRUE")) == 0)
	      return Parser::make_NUM ((uint64_t) 1);
	    else if (strcasecmp (name, NTXT ("FALSE")) == 0)
	      return Parser::make_NUM ((uint64_t) 0);
	    else if (strcasecmp (name, NTXT ("FNAME")) == 0)
	      return Parser::make_FNAME (Expression::FUNC_FNAME);
	    else if (strcasecmp (name, NTXT ("HAS_PROP")) == 0)
	      return Parser::make_HASPROP ();
	    else if (strcasecmp (name, NTXT ("JGROUP")) == 0)
	      return Parser::make_JGROUP (Expression::JAVA_JGROUP);
	    else if (strcasecmp (name, NTXT ("JPARENT")) == 0 )
	      return Parser::make_JPARENT (Expression::JAVA_JPARENT);
	    else if (strcasecmp (name, NTXT ("DNAME")) == 0)
	      return Parser::make_FNAME (Expression::FUNC_DNAME);
	    else if (strcasecmp (name, NTXT ("FILEIOVFD")) == 0 )
	      return Parser::make_FILEIOVFD ();

	    std::string nm = std::string (name);
	    return Parser::make_NAME (nm);
	  }

	throw Parser::syntax_error ("Syntax error");
      }
  }
  void
  Parser::error (const std::string &)
  {
    // do nothing for now
  }
}

