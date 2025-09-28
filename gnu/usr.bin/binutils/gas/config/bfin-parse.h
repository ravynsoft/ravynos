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

#ifndef YY_YY_CONFIG_BFIN_PARSE_H_INCLUDED
# define YY_YY_CONFIG_BFIN_PARSE_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
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
    BYTEOP16P = 258,               /* BYTEOP16P  */
    BYTEOP16M = 259,               /* BYTEOP16M  */
    BYTEOP1P = 260,                /* BYTEOP1P  */
    BYTEOP2P = 261,                /* BYTEOP2P  */
    BYTEOP3P = 262,                /* BYTEOP3P  */
    BYTEUNPACK = 263,              /* BYTEUNPACK  */
    BYTEPACK = 264,                /* BYTEPACK  */
    PACK = 265,                    /* PACK  */
    SAA = 266,                     /* SAA  */
    ALIGN8 = 267,                  /* ALIGN8  */
    ALIGN16 = 268,                 /* ALIGN16  */
    ALIGN24 = 269,                 /* ALIGN24  */
    VIT_MAX = 270,                 /* VIT_MAX  */
    EXTRACT = 271,                 /* EXTRACT  */
    DEPOSIT = 272,                 /* DEPOSIT  */
    EXPADJ = 273,                  /* EXPADJ  */
    SEARCH = 274,                  /* SEARCH  */
    ONES = 275,                    /* ONES  */
    SIGN = 276,                    /* SIGN  */
    SIGNBITS = 277,                /* SIGNBITS  */
    LINK = 278,                    /* LINK  */
    UNLINK = 279,                  /* UNLINK  */
    REG = 280,                     /* REG  */
    PC = 281,                      /* PC  */
    CCREG = 282,                   /* CCREG  */
    BYTE_DREG = 283,               /* BYTE_DREG  */
    REG_A_DOUBLE_ZERO = 284,       /* REG_A_DOUBLE_ZERO  */
    REG_A_DOUBLE_ONE = 285,        /* REG_A_DOUBLE_ONE  */
    A_ZERO_DOT_L = 286,            /* A_ZERO_DOT_L  */
    A_ZERO_DOT_H = 287,            /* A_ZERO_DOT_H  */
    A_ONE_DOT_L = 288,             /* A_ONE_DOT_L  */
    A_ONE_DOT_H = 289,             /* A_ONE_DOT_H  */
    HALF_REG = 290,                /* HALF_REG  */
    NOP = 291,                     /* NOP  */
    RTI = 292,                     /* RTI  */
    RTS = 293,                     /* RTS  */
    RTX = 294,                     /* RTX  */
    RTN = 295,                     /* RTN  */
    RTE = 296,                     /* RTE  */
    HLT = 297,                     /* HLT  */
    IDLE = 298,                    /* IDLE  */
    STI = 299,                     /* STI  */
    CLI = 300,                     /* CLI  */
    CSYNC = 301,                   /* CSYNC  */
    SSYNC = 302,                   /* SSYNC  */
    EMUEXCPT = 303,                /* EMUEXCPT  */
    RAISE = 304,                   /* RAISE  */
    EXCPT = 305,                   /* EXCPT  */
    LSETUP = 306,                  /* LSETUP  */
    LOOP = 307,                    /* LOOP  */
    LOOP_BEGIN = 308,              /* LOOP_BEGIN  */
    LOOP_END = 309,                /* LOOP_END  */
    DISALGNEXCPT = 310,            /* DISALGNEXCPT  */
    JUMP = 311,                    /* JUMP  */
    JUMP_DOT_S = 312,              /* JUMP_DOT_S  */
    JUMP_DOT_L = 313,              /* JUMP_DOT_L  */
    CALL = 314,                    /* CALL  */
    ABORT = 315,                   /* ABORT  */
    NOT = 316,                     /* NOT  */
    TILDA = 317,                   /* TILDA  */
    BANG = 318,                    /* BANG  */
    AMPERSAND = 319,               /* AMPERSAND  */
    BAR = 320,                     /* BAR  */
    PERCENT = 321,                 /* PERCENT  */
    CARET = 322,                   /* CARET  */
    BXOR = 323,                    /* BXOR  */
    MINUS = 324,                   /* MINUS  */
    PLUS = 325,                    /* PLUS  */
    STAR = 326,                    /* STAR  */
    SLASH = 327,                   /* SLASH  */
    NEG = 328,                     /* NEG  */
    MIN = 329,                     /* MIN  */
    MAX = 330,                     /* MAX  */
    ABS = 331,                     /* ABS  */
    DOUBLE_BAR = 332,              /* DOUBLE_BAR  */
    _PLUS_BAR_PLUS = 333,          /* _PLUS_BAR_PLUS  */
    _PLUS_BAR_MINUS = 334,         /* _PLUS_BAR_MINUS  */
    _MINUS_BAR_PLUS = 335,         /* _MINUS_BAR_PLUS  */
    _MINUS_BAR_MINUS = 336,        /* _MINUS_BAR_MINUS  */
    _MINUS_MINUS = 337,            /* _MINUS_MINUS  */
    _PLUS_PLUS = 338,              /* _PLUS_PLUS  */
    SHIFT = 339,                   /* SHIFT  */
    LSHIFT = 340,                  /* LSHIFT  */
    ASHIFT = 341,                  /* ASHIFT  */
    BXORSHIFT = 342,               /* BXORSHIFT  */
    _GREATER_GREATER_GREATER_THAN_ASSIGN = 343, /* _GREATER_GREATER_GREATER_THAN_ASSIGN  */
    ROT = 344,                     /* ROT  */
    LESS_LESS = 345,               /* LESS_LESS  */
    GREATER_GREATER = 346,         /* GREATER_GREATER  */
    _GREATER_GREATER_GREATER = 347, /* _GREATER_GREATER_GREATER  */
    _LESS_LESS_ASSIGN = 348,       /* _LESS_LESS_ASSIGN  */
    _GREATER_GREATER_ASSIGN = 349, /* _GREATER_GREATER_ASSIGN  */
    DIVS = 350,                    /* DIVS  */
    DIVQ = 351,                    /* DIVQ  */
    ASSIGN = 352,                  /* ASSIGN  */
    _STAR_ASSIGN = 353,            /* _STAR_ASSIGN  */
    _BAR_ASSIGN = 354,             /* _BAR_ASSIGN  */
    _CARET_ASSIGN = 355,           /* _CARET_ASSIGN  */
    _AMPERSAND_ASSIGN = 356,       /* _AMPERSAND_ASSIGN  */
    _MINUS_ASSIGN = 357,           /* _MINUS_ASSIGN  */
    _PLUS_ASSIGN = 358,            /* _PLUS_ASSIGN  */
    _ASSIGN_BANG = 359,            /* _ASSIGN_BANG  */
    _LESS_THAN_ASSIGN = 360,       /* _LESS_THAN_ASSIGN  */
    _ASSIGN_ASSIGN = 361,          /* _ASSIGN_ASSIGN  */
    GE = 362,                      /* GE  */
    LT = 363,                      /* LT  */
    LE = 364,                      /* LE  */
    GT = 365,                      /* GT  */
    LESS_THAN = 366,               /* LESS_THAN  */
    FLUSHINV = 367,                /* FLUSHINV  */
    FLUSH = 368,                   /* FLUSH  */
    IFLUSH = 369,                  /* IFLUSH  */
    PREFETCH = 370,                /* PREFETCH  */
    PRNT = 371,                    /* PRNT  */
    OUTC = 372,                    /* OUTC  */
    WHATREG = 373,                 /* WHATREG  */
    TESTSET = 374,                 /* TESTSET  */
    ASL = 375,                     /* ASL  */
    ASR = 376,                     /* ASR  */
    B = 377,                       /* B  */
    W = 378,                       /* W  */
    NS = 379,                      /* NS  */
    S = 380,                       /* S  */
    CO = 381,                      /* CO  */
    SCO = 382,                     /* SCO  */
    TH = 383,                      /* TH  */
    TL = 384,                      /* TL  */
    BP = 385,                      /* BP  */
    BREV = 386,                    /* BREV  */
    X = 387,                       /* X  */
    Z = 388,                       /* Z  */
    M = 389,                       /* M  */
    MMOD = 390,                    /* MMOD  */
    R = 391,                       /* R  */
    RND = 392,                     /* RND  */
    RNDL = 393,                    /* RNDL  */
    RNDH = 394,                    /* RNDH  */
    RND12 = 395,                   /* RND12  */
    RND20 = 396,                   /* RND20  */
    V = 397,                       /* V  */
    LO = 398,                      /* LO  */
    HI = 399,                      /* HI  */
    BITTGL = 400,                  /* BITTGL  */
    BITCLR = 401,                  /* BITCLR  */
    BITSET = 402,                  /* BITSET  */
    BITTST = 403,                  /* BITTST  */
    BITMUX = 404,                  /* BITMUX  */
    DBGAL = 405,                   /* DBGAL  */
    DBGAH = 406,                   /* DBGAH  */
    DBGHALT = 407,                 /* DBGHALT  */
    DBG = 408,                     /* DBG  */
    DBGA = 409,                    /* DBGA  */
    DBGCMPLX = 410,                /* DBGCMPLX  */
    IF = 411,                      /* IF  */
    COMMA = 412,                   /* COMMA  */
    BY = 413,                      /* BY  */
    COLON = 414,                   /* COLON  */
    SEMICOLON = 415,               /* SEMICOLON  */
    RPAREN = 416,                  /* RPAREN  */
    LPAREN = 417,                  /* LPAREN  */
    LBRACK = 418,                  /* LBRACK  */
    RBRACK = 419,                  /* RBRACK  */
    STATUS_REG = 420,              /* STATUS_REG  */
    MNOP = 421,                    /* MNOP  */
    SYMBOL = 422,                  /* SYMBOL  */
    NUMBER = 423,                  /* NUMBER  */
    GOT = 424,                     /* GOT  */
    GOT17M4 = 425,                 /* GOT17M4  */
    FUNCDESC_GOT17M4 = 426,        /* FUNCDESC_GOT17M4  */
    AT = 427,                      /* AT  */
    PLTPC = 428                    /* PLTPC  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif
/* Token kinds.  */
#define YYEMPTY -2
#define YYEOF 0
#define YYerror 256
#define YYUNDEF 257
#define BYTEOP16P 258
#define BYTEOP16M 259
#define BYTEOP1P 260
#define BYTEOP2P 261
#define BYTEOP3P 262
#define BYTEUNPACK 263
#define BYTEPACK 264
#define PACK 265
#define SAA 266
#define ALIGN8 267
#define ALIGN16 268
#define ALIGN24 269
#define VIT_MAX 270
#define EXTRACT 271
#define DEPOSIT 272
#define EXPADJ 273
#define SEARCH 274
#define ONES 275
#define SIGN 276
#define SIGNBITS 277
#define LINK 278
#define UNLINK 279
#define REG 280
#define PC 281
#define CCREG 282
#define BYTE_DREG 283
#define REG_A_DOUBLE_ZERO 284
#define REG_A_DOUBLE_ONE 285
#define A_ZERO_DOT_L 286
#define A_ZERO_DOT_H 287
#define A_ONE_DOT_L 288
#define A_ONE_DOT_H 289
#define HALF_REG 290
#define NOP 291
#define RTI 292
#define RTS 293
#define RTX 294
#define RTN 295
#define RTE 296
#define HLT 297
#define IDLE 298
#define STI 299
#define CLI 300
#define CSYNC 301
#define SSYNC 302
#define EMUEXCPT 303
#define RAISE 304
#define EXCPT 305
#define LSETUP 306
#define LOOP 307
#define LOOP_BEGIN 308
#define LOOP_END 309
#define DISALGNEXCPT 310
#define JUMP 311
#define JUMP_DOT_S 312
#define JUMP_DOT_L 313
#define CALL 314
#define ABORT 315
#define NOT 316
#define TILDA 317
#define BANG 318
#define AMPERSAND 319
#define BAR 320
#define PERCENT 321
#define CARET 322
#define BXOR 323
#define MINUS 324
#define PLUS 325
#define STAR 326
#define SLASH 327
#define NEG 328
#define MIN 329
#define MAX 330
#define ABS 331
#define DOUBLE_BAR 332
#define _PLUS_BAR_PLUS 333
#define _PLUS_BAR_MINUS 334
#define _MINUS_BAR_PLUS 335
#define _MINUS_BAR_MINUS 336
#define _MINUS_MINUS 337
#define _PLUS_PLUS 338
#define SHIFT 339
#define LSHIFT 340
#define ASHIFT 341
#define BXORSHIFT 342
#define _GREATER_GREATER_GREATER_THAN_ASSIGN 343
#define ROT 344
#define LESS_LESS 345
#define GREATER_GREATER 346
#define _GREATER_GREATER_GREATER 347
#define _LESS_LESS_ASSIGN 348
#define _GREATER_GREATER_ASSIGN 349
#define DIVS 350
#define DIVQ 351
#define ASSIGN 352
#define _STAR_ASSIGN 353
#define _BAR_ASSIGN 354
#define _CARET_ASSIGN 355
#define _AMPERSAND_ASSIGN 356
#define _MINUS_ASSIGN 357
#define _PLUS_ASSIGN 358
#define _ASSIGN_BANG 359
#define _LESS_THAN_ASSIGN 360
#define _ASSIGN_ASSIGN 361
#define GE 362
#define LT 363
#define LE 364
#define GT 365
#define LESS_THAN 366
#define FLUSHINV 367
#define FLUSH 368
#define IFLUSH 369
#define PREFETCH 370
#define PRNT 371
#define OUTC 372
#define WHATREG 373
#define TESTSET 374
#define ASL 375
#define ASR 376
#define B 377
#define W 378
#define NS 379
#define S 380
#define CO 381
#define SCO 382
#define TH 383
#define TL 384
#define BP 385
#define BREV 386
#define X 387
#define Z 388
#define M 389
#define MMOD 390
#define R 391
#define RND 392
#define RNDL 393
#define RNDH 394
#define RND12 395
#define RND20 396
#define V 397
#define LO 398
#define HI 399
#define BITTGL 400
#define BITCLR 401
#define BITSET 402
#define BITTST 403
#define BITMUX 404
#define DBGAL 405
#define DBGAH 406
#define DBGHALT 407
#define DBG 408
#define DBGA 409
#define DBGCMPLX 410
#define IF 411
#define COMMA 412
#define BY 413
#define COLON 414
#define SEMICOLON 415
#define RPAREN 416
#define LPAREN 417
#define LBRACK 418
#define RBRACK 419
#define STATUS_REG 420
#define MNOP 421
#define SYMBOL 422
#define NUMBER 423
#define GOT 424
#define GOT17M4 425
#define FUNCDESC_GOT17M4 426
#define AT 427
#define PLTPC 428

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 447 "./config/bfin-parse.y"

  INSTR_T instr;
  Expr_Node *expr;
  SYMBOL_T symbol;
  long value;
  Register reg;
  Macfunc macfunc;
  struct { int r0; int s0; int x0; int aop; } modcodes;
  struct { int r0; } r0;
  Opt_mode mod;

#line 425 "config/bfin-parse.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_CONFIG_BFIN_PARSE_H_INCLUDED  */
