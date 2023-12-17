/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or po_gram__.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_PO_GRAM_PO_GRAM_GEN_TAB_H_INCLUDED
# define YY_PO_GRAM_PO_GRAM_GEN_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef PO_GRAM_DEBUG
# if defined YYDEBUG
#if YYDEBUG
#   define PO_GRAM_DEBUG 1
#  else
#   define PO_GRAM_DEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define PO_GRAM_DEBUG 0
# endif /* ! defined YYDEBUG */
#endif  /* ! defined PO_GRAM_DEBUG */
#if PO_GRAM_DEBUG
extern DLL_VARIABLE int po_gram_debug;
#endif

/* Token kinds.  */
#ifndef PO_GRAM_TOKENTYPE
# define PO_GRAM_TOKENTYPE
  enum po_gram_tokentype
  {
    PO_GRAM_EMPTY = -2,
    PO_GRAM_EOF = 0,               /* "end of file"  */
    PO_GRAM_error = 256,           /* error  */
    PO_GRAM_UNDEF = 257,           /* "invalid token"  */
    COMMENT = 258,                 /* COMMENT  */
    DOMAIN = 259,                  /* DOMAIN  */
    JUNK = 260,                    /* JUNK  */
    PREV_MSGCTXT = 261,            /* PREV_MSGCTXT  */
    PREV_MSGID = 262,              /* PREV_MSGID  */
    PREV_MSGID_PLURAL = 263,       /* PREV_MSGID_PLURAL  */
    PREV_STRING = 264,             /* PREV_STRING  */
    MSGCTXT = 265,                 /* MSGCTXT  */
    MSGID = 266,                   /* MSGID  */
    MSGID_PLURAL = 267,            /* MSGID_PLURAL  */
    MSGSTR = 268,                  /* MSGSTR  */
    NAME = 269,                    /* NAME  */
    NUMBER = 270,                  /* NUMBER  */
    STRING = 271                   /* STRING  */
  };
  typedef enum po_gram_tokentype po_gram_token_kind_t;
#endif

/* Value type.  */
#if ! defined PO_GRAM_STYPE && ! defined PO_GRAM_STYPE_IS_DECLARED
union PO_GRAM_STYPE
{
#line 103 "po-gram-gen.y"

  struct { char *string; lex_pos_ty pos; bool obsolete; } string;
  struct { string_list_ty stringlist; lex_pos_ty pos; bool obsolete; } stringlist;
  struct { long number; lex_pos_ty pos; bool obsolete; } number;
  struct { lex_pos_ty pos; bool obsolete; } pos;
  struct { char *ctxt; char *id; char *id_plural; lex_pos_ty pos; bool obsolete; } prev;
  struct { char *prev_ctxt; char *prev_id; char *prev_id_plural; char *ctxt; lex_pos_ty pos; bool obsolete; } message_intro;
  struct { struct msgstr_def rhs; lex_pos_ty pos; bool obsolete; } rhs;

#line 98 "po-gram-gen2.h"

};
typedef union PO_GRAM_STYPE PO_GRAM_STYPE;
# define PO_GRAM_STYPE_IS_TRIVIAL 1
# define PO_GRAM_STYPE_IS_DECLARED 1
#endif


extern DLL_VARIABLE PO_GRAM_STYPE po_gram_lval;


int po_gram_parse (void);


#endif /* !YY_PO_GRAM_PO_GRAM_GEN_TAB_H_INCLUDED  */
