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

#ifndef YY_RX_CONFIG_RX_PARSE_H_INCLUDED
# define YY_RX_CONFIG_RX_PARSE_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int rx_debug;
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
    REG = 258,                     /* REG  */
    FLAG = 259,                    /* FLAG  */
    CREG = 260,                    /* CREG  */
    ACC = 261,                     /* ACC  */
    DREG = 262,                    /* DREG  */
    DREGH = 263,                   /* DREGH  */
    DREGL = 264,                   /* DREGL  */
    DCREG = 265,                   /* DCREG  */
    EXPR = 266,                    /* EXPR  */
    UNKNOWN_OPCODE = 267,          /* UNKNOWN_OPCODE  */
    IS_OPCODE = 268,               /* IS_OPCODE  */
    DOT_S = 269,                   /* DOT_S  */
    DOT_B = 270,                   /* DOT_B  */
    DOT_W = 271,                   /* DOT_W  */
    DOT_L = 272,                   /* DOT_L  */
    DOT_A = 273,                   /* DOT_A  */
    DOT_UB = 274,                  /* DOT_UB  */
    DOT_UW = 275,                  /* DOT_UW  */
    DOT_D = 276,                   /* DOT_D  */
    ABS = 277,                     /* ABS  */
    ADC = 278,                     /* ADC  */
    ADD = 279,                     /* ADD  */
    AND_ = 280,                    /* AND_  */
    BCLR = 281,                    /* BCLR  */
    BCND = 282,                    /* BCND  */
    BFMOV = 283,                   /* BFMOV  */
    BFMOVZ = 284,                  /* BFMOVZ  */
    BMCND = 285,                   /* BMCND  */
    BNOT = 286,                    /* BNOT  */
    BRA = 287,                     /* BRA  */
    BRK = 288,                     /* BRK  */
    BSET = 289,                    /* BSET  */
    BSR = 290,                     /* BSR  */
    BTST = 291,                    /* BTST  */
    CLRPSW = 292,                  /* CLRPSW  */
    CMP = 293,                     /* CMP  */
    DABS = 294,                    /* DABS  */
    DADD = 295,                    /* DADD  */
    DBT = 296,                     /* DBT  */
    DCMP = 297,                    /* DCMP  */
    DDIV = 298,                    /* DDIV  */
    DIV = 299,                     /* DIV  */
    DIVU = 300,                    /* DIVU  */
    DMOV = 301,                    /* DMOV  */
    DMUL = 302,                    /* DMUL  */
    DNEG = 303,                    /* DNEG  */
    DPOPM = 304,                   /* DPOPM  */
    DPUSHM = 305,                  /* DPUSHM  */
    DROUND = 306,                  /* DROUND  */
    DSQRT = 307,                   /* DSQRT  */
    DSUB = 308,                    /* DSUB  */
    DTOF = 309,                    /* DTOF  */
    DTOI = 310,                    /* DTOI  */
    DTOU = 311,                    /* DTOU  */
    EDIV = 312,                    /* EDIV  */
    EDIVU = 313,                   /* EDIVU  */
    EMACA = 314,                   /* EMACA  */
    EMSBA = 315,                   /* EMSBA  */
    EMUL = 316,                    /* EMUL  */
    EMULA = 317,                   /* EMULA  */
    EMULU = 318,                   /* EMULU  */
    FADD = 319,                    /* FADD  */
    FCMP = 320,                    /* FCMP  */
    FDIV = 321,                    /* FDIV  */
    FMUL = 322,                    /* FMUL  */
    FREIT = 323,                   /* FREIT  */
    FSUB = 324,                    /* FSUB  */
    FSQRT = 325,                   /* FSQRT  */
    FTOD = 326,                    /* FTOD  */
    FTOI = 327,                    /* FTOI  */
    FTOU = 328,                    /* FTOU  */
    INT = 329,                     /* INT  */
    ITOD = 330,                    /* ITOD  */
    ITOF = 331,                    /* ITOF  */
    JMP = 332,                     /* JMP  */
    JSR = 333,                     /* JSR  */
    MACHI = 334,                   /* MACHI  */
    MACLH = 335,                   /* MACLH  */
    MACLO = 336,                   /* MACLO  */
    MAX = 337,                     /* MAX  */
    MIN = 338,                     /* MIN  */
    MOV = 339,                     /* MOV  */
    MOVCO = 340,                   /* MOVCO  */
    MOVLI = 341,                   /* MOVLI  */
    MOVU = 342,                    /* MOVU  */
    MSBHI = 343,                   /* MSBHI  */
    MSBLH = 344,                   /* MSBLH  */
    MSBLO = 345,                   /* MSBLO  */
    MUL = 346,                     /* MUL  */
    MULHI = 347,                   /* MULHI  */
    MULLH = 348,                   /* MULLH  */
    MULLO = 349,                   /* MULLO  */
    MULU = 350,                    /* MULU  */
    MVFACHI = 351,                 /* MVFACHI  */
    MVFACGU = 352,                 /* MVFACGU  */
    MVFACMI = 353,                 /* MVFACMI  */
    MVFACLO = 354,                 /* MVFACLO  */
    MVFC = 355,                    /* MVFC  */
    MVFDC = 356,                   /* MVFDC  */
    MVFDR = 357,                   /* MVFDR  */
    MVTACGU = 358,                 /* MVTACGU  */
    MVTACHI = 359,                 /* MVTACHI  */
    MVTACLO = 360,                 /* MVTACLO  */
    MVTC = 361,                    /* MVTC  */
    MVTDC = 362,                   /* MVTDC  */
    MVTIPL = 363,                  /* MVTIPL  */
    NEG = 364,                     /* NEG  */
    NOP = 365,                     /* NOP  */
    NOT = 366,                     /* NOT  */
    OR = 367,                      /* OR  */
    POP = 368,                     /* POP  */
    POPC = 369,                    /* POPC  */
    POPM = 370,                    /* POPM  */
    PUSH = 371,                    /* PUSH  */
    PUSHA = 372,                   /* PUSHA  */
    PUSHC = 373,                   /* PUSHC  */
    PUSHM = 374,                   /* PUSHM  */
    RACL = 375,                    /* RACL  */
    RACW = 376,                    /* RACW  */
    RDACL = 377,                   /* RDACL  */
    RDACW = 378,                   /* RDACW  */
    REIT = 379,                    /* REIT  */
    REVL = 380,                    /* REVL  */
    REVW = 381,                    /* REVW  */
    RMPA = 382,                    /* RMPA  */
    ROLC = 383,                    /* ROLC  */
    RORC = 384,                    /* RORC  */
    ROTL = 385,                    /* ROTL  */
    ROTR = 386,                    /* ROTR  */
    ROUND = 387,                   /* ROUND  */
    RSTR = 388,                    /* RSTR  */
    RTE = 389,                     /* RTE  */
    RTFI = 390,                    /* RTFI  */
    RTS = 391,                     /* RTS  */
    RTSD = 392,                    /* RTSD  */
    SAT = 393,                     /* SAT  */
    SATR = 394,                    /* SATR  */
    SAVE = 395,                    /* SAVE  */
    SBB = 396,                     /* SBB  */
    SCCND = 397,                   /* SCCND  */
    SCMPU = 398,                   /* SCMPU  */
    SETPSW = 399,                  /* SETPSW  */
    SHAR = 400,                    /* SHAR  */
    SHLL = 401,                    /* SHLL  */
    SHLR = 402,                    /* SHLR  */
    SMOVB = 403,                   /* SMOVB  */
    SMOVF = 404,                   /* SMOVF  */
    SMOVU = 405,                   /* SMOVU  */
    SSTR = 406,                    /* SSTR  */
    STNZ = 407,                    /* STNZ  */
    STOP = 408,                    /* STOP  */
    STZ = 409,                     /* STZ  */
    SUB = 410,                     /* SUB  */
    SUNTIL = 411,                  /* SUNTIL  */
    SWHILE = 412,                  /* SWHILE  */
    TST = 413,                     /* TST  */
    UTOD = 414,                    /* UTOD  */
    UTOF = 415,                    /* UTOF  */
    WAIT = 416,                    /* WAIT  */
    XCHG = 417,                    /* XCHG  */
    XOR = 418                      /* XOR  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif
/* Token kinds.  */
#define YYEMPTY -2
#define YYEOF 0
#define YYerror 256
#define YYUNDEF 257
#define REG 258
#define FLAG 259
#define CREG 260
#define ACC 261
#define DREG 262
#define DREGH 263
#define DREGL 264
#define DCREG 265
#define EXPR 266
#define UNKNOWN_OPCODE 267
#define IS_OPCODE 268
#define DOT_S 269
#define DOT_B 270
#define DOT_W 271
#define DOT_L 272
#define DOT_A 273
#define DOT_UB 274
#define DOT_UW 275
#define DOT_D 276
#define ABS 277
#define ADC 278
#define ADD 279
#define AND_ 280
#define BCLR 281
#define BCND 282
#define BFMOV 283
#define BFMOVZ 284
#define BMCND 285
#define BNOT 286
#define BRA 287
#define BRK 288
#define BSET 289
#define BSR 290
#define BTST 291
#define CLRPSW 292
#define CMP 293
#define DABS 294
#define DADD 295
#define DBT 296
#define DCMP 297
#define DDIV 298
#define DIV 299
#define DIVU 300
#define DMOV 301
#define DMUL 302
#define DNEG 303
#define DPOPM 304
#define DPUSHM 305
#define DROUND 306
#define DSQRT 307
#define DSUB 308
#define DTOF 309
#define DTOI 310
#define DTOU 311
#define EDIV 312
#define EDIVU 313
#define EMACA 314
#define EMSBA 315
#define EMUL 316
#define EMULA 317
#define EMULU 318
#define FADD 319
#define FCMP 320
#define FDIV 321
#define FMUL 322
#define FREIT 323
#define FSUB 324
#define FSQRT 325
#define FTOD 326
#define FTOI 327
#define FTOU 328
#define INT 329
#define ITOD 330
#define ITOF 331
#define JMP 332
#define JSR 333
#define MACHI 334
#define MACLH 335
#define MACLO 336
#define MAX 337
#define MIN 338
#define MOV 339
#define MOVCO 340
#define MOVLI 341
#define MOVU 342
#define MSBHI 343
#define MSBLH 344
#define MSBLO 345
#define MUL 346
#define MULHI 347
#define MULLH 348
#define MULLO 349
#define MULU 350
#define MVFACHI 351
#define MVFACGU 352
#define MVFACMI 353
#define MVFACLO 354
#define MVFC 355
#define MVFDC 356
#define MVFDR 357
#define MVTACGU 358
#define MVTACHI 359
#define MVTACLO 360
#define MVTC 361
#define MVTDC 362
#define MVTIPL 363
#define NEG 364
#define NOP 365
#define NOT 366
#define OR 367
#define POP 368
#define POPC 369
#define POPM 370
#define PUSH 371
#define PUSHA 372
#define PUSHC 373
#define PUSHM 374
#define RACL 375
#define RACW 376
#define RDACL 377
#define RDACW 378
#define REIT 379
#define REVL 380
#define REVW 381
#define RMPA 382
#define ROLC 383
#define RORC 384
#define ROTL 385
#define ROTR 386
#define ROUND 387
#define RSTR 388
#define RTE 389
#define RTFI 390
#define RTS 391
#define RTSD 392
#define SAT 393
#define SATR 394
#define SAVE 395
#define SBB 396
#define SCCND 397
#define SCMPU 398
#define SETPSW 399
#define SHAR 400
#define SHLL 401
#define SHLR 402
#define SMOVB 403
#define SMOVF 404
#define SMOVU 405
#define SSTR 406
#define STNZ 407
#define STOP 408
#define STZ 409
#define SUB 410
#define SUNTIL 411
#define SWHILE 412
#define TST 413
#define UTOD 414
#define UTOF 415
#define WAIT 416
#define XCHG 417
#define XOR 418

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 140 "./config/rx-parse.y"

  int regno;
  expressionS exp;

#line 398 "config/rx-parse.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE rx_lval;


int rx_parse (void);


#endif /* !YY_RX_CONFIG_RX_PARSE_H_INCLUDED  */
