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
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_SUDOERS_Y_TAB_H_INCLUDED
# define YY_SUDOERS_Y_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int sudoersdebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    COMMAND = 258,                 /* COMMAND  */
    ALIAS = 259,                   /* ALIAS  */
    DEFVAR = 260,                  /* DEFVAR  */
    NTWKADDR = 261,                /* NTWKADDR  */
    NETGROUP = 262,                /* NETGROUP  */
    USERGROUP = 263,               /* USERGROUP  */
    WORD = 264,                    /* WORD  */
    DIGEST = 265,                  /* DIGEST  */
    INCLUDE = 266,                 /* INCLUDE  */
    INCLUDEDIR = 267,              /* INCLUDEDIR  */
    DEFAULTS = 268,                /* DEFAULTS  */
    DEFAULTS_HOST = 269,           /* DEFAULTS_HOST  */
    DEFAULTS_USER = 270,           /* DEFAULTS_USER  */
    DEFAULTS_RUNAS = 271,          /* DEFAULTS_RUNAS  */
    DEFAULTS_CMND = 272,           /* DEFAULTS_CMND  */
    NOPASSWD = 273,                /* NOPASSWD  */
    PASSWD = 274,                  /* PASSWD  */
    NOEXEC = 275,                  /* NOEXEC  */
    EXEC = 276,                    /* EXEC  */
    SETENV = 277,                  /* SETENV  */
    NOSETENV = 278,                /* NOSETENV  */
    LOG_INPUT = 279,               /* LOG_INPUT  */
    NOLOG_INPUT = 280,             /* NOLOG_INPUT  */
    LOG_OUTPUT = 281,              /* LOG_OUTPUT  */
    NOLOG_OUTPUT = 282,            /* NOLOG_OUTPUT  */
    MAIL = 283,                    /* MAIL  */
    NOMAIL = 284,                  /* NOMAIL  */
    FOLLOWLNK = 285,               /* FOLLOWLNK  */
    NOFOLLOWLNK = 286,             /* NOFOLLOWLNK  */
    INTERCEPT = 287,               /* INTERCEPT  */
    NOINTERCEPT = 288,             /* NOINTERCEPT  */
    ALL = 289,                     /* ALL  */
    HOSTALIAS = 290,               /* HOSTALIAS  */
    CMNDALIAS = 291,               /* CMNDALIAS  */
    USERALIAS = 292,               /* USERALIAS  */
    RUNASALIAS = 293,              /* RUNASALIAS  */
    ERROR = 294,                   /* ERROR  */
    NOMATCH = 295,                 /* NOMATCH  */
    CHROOT = 296,                  /* CHROOT  */
    CWD = 297,                     /* CWD  */
    TYPE = 298,                    /* TYPE  */
    ROLE = 299,                    /* ROLE  */
    APPARMOR_PROFILE = 300,        /* APPARMOR_PROFILE  */
    PRIVS = 301,                   /* PRIVS  */
    LIMITPRIVS = 302,              /* LIMITPRIVS  */
    CMND_TIMEOUT = 303,            /* CMND_TIMEOUT  */
    NOTBEFORE = 304,               /* NOTBEFORE  */
    NOTAFTER = 305,                /* NOTAFTER  */
    MYSELF = 306,                  /* MYSELF  */
    SHA224_TOK = 307,              /* SHA224_TOK  */
    SHA256_TOK = 308,              /* SHA256_TOK  */
    SHA384_TOK = 309,              /* SHA384_TOK  */
    SHA512_TOK = 310               /* SHA512_TOK  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif
/* Token kinds.  */
#define YYEMPTY -2
#define YYEOF 0
#define YYerror 256
#define YYUNDEF 257
#define COMMAND 258
#define ALIAS 259
#define DEFVAR 260
#define NTWKADDR 261
#define NETGROUP 262
#define USERGROUP 263
#define WORD 264
#define DIGEST 265
#define INCLUDE 266
#define INCLUDEDIR 267
#define DEFAULTS 268
#define DEFAULTS_HOST 269
#define DEFAULTS_USER 270
#define DEFAULTS_RUNAS 271
#define DEFAULTS_CMND 272
#define NOPASSWD 273
#define PASSWD 274
#define NOEXEC 275
#define EXEC 276
#define SETENV 277
#define NOSETENV 278
#define LOG_INPUT 279
#define NOLOG_INPUT 280
#define LOG_OUTPUT 281
#define NOLOG_OUTPUT 282
#define MAIL 283
#define NOMAIL 284
#define FOLLOWLNK 285
#define NOFOLLOWLNK 286
#define INTERCEPT 287
#define NOINTERCEPT 288
#define ALL 289
#define HOSTALIAS 290
#define CMNDALIAS 291
#define USERALIAS 292
#define RUNASALIAS 293
#define ERROR 294
#define NOMATCH 295
#define CHROOT 296
#define CWD 297
#define TYPE 298
#define ROLE 299
#define APPARMOR_PROFILE 300
#define PRIVS 301
#define LIMITPRIVS 302
#define CMND_TIMEOUT 303
#define NOTBEFORE 304
#define NOTAFTER 305
#define MYSELF 306
#define SHA224_TOK 307
#define SHA256_TOK 308
#define SHA384_TOK 309
#define SHA512_TOK 310

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 90 "gram.y"

    struct cmndspec *cmndspec;
    struct defaults *defaults;
    struct member *member;
    struct runascontainer *runas;
    struct privilege *privilege;
    struct command_digest *digest;
    struct sudo_command command;
    struct command_options options;
    struct cmndtag tag;
    char *string;
    const char *cstring;
    int tok;

#line 192 "gram.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE sudoerslval;


int sudoersparse (void);


#endif /* !YY_SUDOERS_Y_TAB_H_INCLUDED  */
