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

#ifndef YY_RL78_CONFIG_RL_PARSE_H_INCLUDED
# define YY_RL78_CONFIG_RL_PARSE_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int rl78_debug;
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
    A = 258,                       /* A  */
    X = 259,                       /* X  */
    B = 260,                       /* B  */
    C = 261,                       /* C  */
    D = 262,                       /* D  */
    E = 263,                       /* E  */
    H = 264,                       /* H  */
    L = 265,                       /* L  */
    AX = 266,                      /* AX  */
    BC = 267,                      /* BC  */
    DE = 268,                      /* DE  */
    HL = 269,                      /* HL  */
    SPL = 270,                     /* SPL  */
    SPH = 271,                     /* SPH  */
    PSW = 272,                     /* PSW  */
    CS = 273,                      /* CS  */
    ES = 274,                      /* ES  */
    PMC = 275,                     /* PMC  */
    MEM = 276,                     /* MEM  */
    FLAG = 277,                    /* FLAG  */
    SP = 278,                      /* SP  */
    CY = 279,                      /* CY  */
    RB0 = 280,                     /* RB0  */
    RB1 = 281,                     /* RB1  */
    RB2 = 282,                     /* RB2  */
    RB3 = 283,                     /* RB3  */
    EXPR = 284,                    /* EXPR  */
    UNKNOWN_OPCODE = 285,          /* UNKNOWN_OPCODE  */
    IS_OPCODE = 286,               /* IS_OPCODE  */
    DOT_S = 287,                   /* DOT_S  */
    DOT_B = 288,                   /* DOT_B  */
    DOT_W = 289,                   /* DOT_W  */
    DOT_L = 290,                   /* DOT_L  */
    DOT_A = 291,                   /* DOT_A  */
    DOT_UB = 292,                  /* DOT_UB  */
    DOT_UW = 293,                  /* DOT_UW  */
    ADD = 294,                     /* ADD  */
    ADDC = 295,                    /* ADDC  */
    ADDW = 296,                    /* ADDW  */
    AND_ = 297,                    /* AND_  */
    AND1 = 298,                    /* AND1  */
    BF = 299,                      /* BF  */
    BH = 300,                      /* BH  */
    BNC = 301,                     /* BNC  */
    BNH = 302,                     /* BNH  */
    BNZ = 303,                     /* BNZ  */
    BR = 304,                      /* BR  */
    BRK = 305,                     /* BRK  */
    BRK1 = 306,                    /* BRK1  */
    BT = 307,                      /* BT  */
    BTCLR = 308,                   /* BTCLR  */
    BZ = 309,                      /* BZ  */
    CALL = 310,                    /* CALL  */
    CALLT = 311,                   /* CALLT  */
    CLR1 = 312,                    /* CLR1  */
    CLRB = 313,                    /* CLRB  */
    CLRW = 314,                    /* CLRW  */
    CMP = 315,                     /* CMP  */
    CMP0 = 316,                    /* CMP0  */
    CMPS = 317,                    /* CMPS  */
    CMPW = 318,                    /* CMPW  */
    DEC = 319,                     /* DEC  */
    DECW = 320,                    /* DECW  */
    DI = 321,                      /* DI  */
    DIVHU = 322,                   /* DIVHU  */
    DIVWU = 323,                   /* DIVWU  */
    EI = 324,                      /* EI  */
    HALT = 325,                    /* HALT  */
    INC = 326,                     /* INC  */
    INCW = 327,                    /* INCW  */
    MACH = 328,                    /* MACH  */
    MACHU = 329,                   /* MACHU  */
    MOV = 330,                     /* MOV  */
    MOV1 = 331,                    /* MOV1  */
    MOVS = 332,                    /* MOVS  */
    MOVW = 333,                    /* MOVW  */
    MULH = 334,                    /* MULH  */
    MULHU = 335,                   /* MULHU  */
    MULU = 336,                    /* MULU  */
    NOP = 337,                     /* NOP  */
    NOT1 = 338,                    /* NOT1  */
    ONEB = 339,                    /* ONEB  */
    ONEW = 340,                    /* ONEW  */
    OR = 341,                      /* OR  */
    OR1 = 342,                     /* OR1  */
    POP = 343,                     /* POP  */
    PUSH = 344,                    /* PUSH  */
    RET = 345,                     /* RET  */
    RETI = 346,                    /* RETI  */
    RETB = 347,                    /* RETB  */
    ROL = 348,                     /* ROL  */
    ROLC = 349,                    /* ROLC  */
    ROLWC = 350,                   /* ROLWC  */
    ROR = 351,                     /* ROR  */
    RORC = 352,                    /* RORC  */
    SAR = 353,                     /* SAR  */
    SARW = 354,                    /* SARW  */
    SEL = 355,                     /* SEL  */
    SET1 = 356,                    /* SET1  */
    SHL = 357,                     /* SHL  */
    SHLW = 358,                    /* SHLW  */
    SHR = 359,                     /* SHR  */
    SHRW = 360,                    /* SHRW  */
    SKC = 361,                     /* SKC  */
    SKH = 362,                     /* SKH  */
    SKNC = 363,                    /* SKNC  */
    SKNH = 364,                    /* SKNH  */
    SKNZ = 365,                    /* SKNZ  */
    SKZ = 366,                     /* SKZ  */
    STOP = 367,                    /* STOP  */
    SUB = 368,                     /* SUB  */
    SUBC = 369,                    /* SUBC  */
    SUBW = 370,                    /* SUBW  */
    XCH = 371,                     /* XCH  */
    XCHW = 372,                    /* XCHW  */
    XOR = 373,                     /* XOR  */
    XOR1 = 374                     /* XOR1  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif
/* Token kinds.  */
#define YYEMPTY -2
#define YYEOF 0
#define YYerror 256
#define YYUNDEF 257
#define A 258
#define X 259
#define B 260
#define C 261
#define D 262
#define E 263
#define H 264
#define L 265
#define AX 266
#define BC 267
#define DE 268
#define HL 269
#define SPL 270
#define SPH 271
#define PSW 272
#define CS 273
#define ES 274
#define PMC 275
#define MEM 276
#define FLAG 277
#define SP 278
#define CY 279
#define RB0 280
#define RB1 281
#define RB2 282
#define RB3 283
#define EXPR 284
#define UNKNOWN_OPCODE 285
#define IS_OPCODE 286
#define DOT_S 287
#define DOT_B 288
#define DOT_W 289
#define DOT_L 290
#define DOT_A 291
#define DOT_UB 292
#define DOT_UW 293
#define ADD 294
#define ADDC 295
#define ADDW 296
#define AND_ 297
#define AND1 298
#define BF 299
#define BH 300
#define BNC 301
#define BNH 302
#define BNZ 303
#define BR 304
#define BRK 305
#define BRK1 306
#define BT 307
#define BTCLR 308
#define BZ 309
#define CALL 310
#define CALLT 311
#define CLR1 312
#define CLRB 313
#define CLRW 314
#define CMP 315
#define CMP0 316
#define CMPS 317
#define CMPW 318
#define DEC 319
#define DECW 320
#define DI 321
#define DIVHU 322
#define DIVWU 323
#define EI 324
#define HALT 325
#define INC 326
#define INCW 327
#define MACH 328
#define MACHU 329
#define MOV 330
#define MOV1 331
#define MOVS 332
#define MOVW 333
#define MULH 334
#define MULHU 335
#define MULU 336
#define NOP 337
#define NOT1 338
#define ONEB 339
#define ONEW 340
#define OR 341
#define OR1 342
#define POP 343
#define PUSH 344
#define RET 345
#define RETI 346
#define RETB 347
#define ROL 348
#define ROLC 349
#define ROLWC 350
#define ROR 351
#define RORC 352
#define SAR 353
#define SARW 354
#define SEL 355
#define SET1 356
#define SHL 357
#define SHLW 358
#define SHR 359
#define SHRW 360
#define SKC 361
#define SKH 362
#define SKNC 363
#define SKNH 364
#define SKNZ 365
#define SKZ 366
#define STOP 367
#define SUB 368
#define SUBC 369
#define SUBW 370
#define XCH 371
#define XCHW 372
#define XOR 373
#define XOR1 374

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 144 "./config/rl78-parse.y"

  int regno;
  expressionS exp;

#line 310 "config/rl78-parse.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE rl78_lval;


int rl78_parse (void);


#endif /* !YY_RL78_CONFIG_RL_PARSE_H_INCLUDED  */
