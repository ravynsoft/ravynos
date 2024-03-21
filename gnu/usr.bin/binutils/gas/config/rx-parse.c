/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1


/* Substitute the variable and function names.  */
#define yyparse         rx_parse
#define yylex           rx_lex
#define yyerror         rx_error
#define yydebug         rx_debug
#define yynerrs         rx_nerrs
#define yylval          rx_lval
#define yychar          rx_char

/* First part of user prologue.  */
#line 20 "./config/rx-parse.y"


#include "as.h"
#include "safe-ctype.h"
#include "rx-defs.h"

static int rx_lex (void);

#define COND_EQ	0
#define COND_NE	1

#define MEMEX 0x06

#define BSIZE 0
#define WSIZE 1
#define LSIZE 2
#define DSIZE 3

/*                       .sb    .sw    .l     .uw   */
static int sizemap[] = { BSIZE, WSIZE, LSIZE, WSIZE };

/* Ok, here are the rules for using these macros...

   B*() is used to specify the base opcode bytes.  Fields to be filled
        in later, leave zero.  Call this first.

   F() and FE() are used to fill in fields within the base opcode bytes.  You MUST
        call B*() before any F() or FE().

   [UN]*O*(), PC*() appends operands to the end of the opcode.  You
        must call P() and B*() before any of these, so that the fixups
        have the right byte location.
        O = signed, UO = unsigned, NO = negated, PC = pcrel

   IMM() adds an immediate and fills in the field for it.
   NIMM() same, but negates the immediate.
   NBIMM() same, but negates the immediate, for sbb.
   DSP() adds a displacement, and fills in the field for it.

   Note that order is significant for the O, IMM, and DSP macros, as
   they append their data to the operand buffer in the order that you
   call them.

   Use "disp" for displacements whenever possible; this handles the
   "0" case properly.  */

#define B1(b1)             rx_base1 (b1)
#define B2(b1, b2)         rx_base2 (b1, b2)
#define B3(b1, b2, b3)     rx_base3 (b1, b2, b3)
#define B4(b1, b2, b3, b4) rx_base4 (b1, b2, b3, b4)

/* POS is bits from the MSB of the first byte to the LSB of the last byte.  */
#define F(val,pos,sz)      rx_field (val, pos, sz)
#define FE(exp,pos,sz)	   rx_field (exp_val (exp), pos, sz);

#define O1(v)              rx_op (v, 1, RXREL_SIGNED); rx_range (v, -128, 255)
#define O2(v)              rx_op (v, 2, RXREL_SIGNED); rx_range (v, -32768, 65536)
#define O3(v)              rx_op (v, 3, RXREL_SIGNED); rx_range (v, -8388608, 16777216)
#define O4(v)              rx_op (v, 4, RXREL_SIGNED)

#define UO1(v)             rx_op (v, 1, RXREL_UNSIGNED); rx_range (v, 0, 255)
#define UO2(v)             rx_op (v, 2, RXREL_UNSIGNED); rx_range (v, 0, 65536)
#define UO3(v)             rx_op (v, 3, RXREL_UNSIGNED); rx_range (v, 0, 16777216)
#define UO4(v)             rx_op (v, 4, RXREL_UNSIGNED)

#define NO1(v)             rx_op (v, 1, RXREL_NEGATIVE)
#define NO2(v)             rx_op (v, 2, RXREL_NEGATIVE)
#define NO3(v)             rx_op (v, 3, RXREL_NEGATIVE)
#define NO4(v)             rx_op (v, 4, RXREL_NEGATIVE)

#define PC1(v)             rx_op (v, 1, RXREL_PCREL)
#define PC2(v)             rx_op (v, 2, RXREL_PCREL)
#define PC3(v)             rx_op (v, 3, RXREL_PCREL)

#define POST(v)            rx_post (v)

#define IMM_(v,pos,size)   F (immediate (v, RXREL_SIGNED, pos, size), pos, 2); \
			   if (v.X_op != O_constant && v.X_op != O_big) rx_linkrelax_imm (pos)
#define IMM(v,pos)	   IMM_ (v, pos, 32)
#define IMMW(v,pos)	   IMM_ (v, pos, 16); rx_range (v, -32768, 65536)
#define IMMB(v,pos)	   IMM_ (v, pos, 8); rx_range (v, -128, 255)
#define NIMM(v,pos)	   F (immediate (v, RXREL_NEGATIVE, pos, 32), pos, 2)
#define NBIMM(v,pos)	   F (immediate (v, RXREL_NEGATIVE_BORROW, pos, 32), pos, 2)
#define DSP(v,pos,msz)	   if (!v.X_md) rx_relax (RX_RELAX_DISP, pos); \
			   else rx_linkrelax_dsp (pos); \
			   F (displacement (v, msz), pos, 2)

#define id24(a,b2,b3)	   B3 (0xfb + a, b2, b3)

static void	   rx_check_float_support (void);
static int         rx_intop (expressionS, int, int);
static int         rx_uintop (expressionS, int);
static int         rx_disp3op (expressionS);
static int         rx_disp5op (expressionS *, int);
static int         rx_disp5op0 (expressionS *, int);
static int         exp_val (expressionS exp);
static expressionS zero_expr (void);
static int         immediate (expressionS, int, int, int);
static int         displacement (expressionS, int);
static void        rtsd_immediate (expressionS);
static void	   rx_range (expressionS, int, int);
static void        rx_check_v2 (void);
static void        rx_check_v3 (void);
static void        rx_check_dfpu (void);

static int    need_flag = 0;
static int    rx_in_brackets = 0;
static int    rx_last_token = 0;
static char * rx_init_start;
static char * rx_last_exp_start = 0;
static int    sub_op;
static int    sub_op2;

#define YYDEBUG 1
#define YYERROR_VERBOSE 1


#line 196 "config/rx-parse.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

/* Use api.header.include to #include this header
   instead of duplicating it here.  */
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

#line 580 "config/rx-parse.c"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE rx_lval;


int rx_parse (void);


#endif /* !YY_RX_CONFIG_RX_PARSE_H_INCLUDED  */
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_REG = 3,                        /* REG  */
  YYSYMBOL_FLAG = 4,                       /* FLAG  */
  YYSYMBOL_CREG = 5,                       /* CREG  */
  YYSYMBOL_ACC = 6,                        /* ACC  */
  YYSYMBOL_DREG = 7,                       /* DREG  */
  YYSYMBOL_DREGH = 8,                      /* DREGH  */
  YYSYMBOL_DREGL = 9,                      /* DREGL  */
  YYSYMBOL_DCREG = 10,                     /* DCREG  */
  YYSYMBOL_EXPR = 11,                      /* EXPR  */
  YYSYMBOL_UNKNOWN_OPCODE = 12,            /* UNKNOWN_OPCODE  */
  YYSYMBOL_IS_OPCODE = 13,                 /* IS_OPCODE  */
  YYSYMBOL_DOT_S = 14,                     /* DOT_S  */
  YYSYMBOL_DOT_B = 15,                     /* DOT_B  */
  YYSYMBOL_DOT_W = 16,                     /* DOT_W  */
  YYSYMBOL_DOT_L = 17,                     /* DOT_L  */
  YYSYMBOL_DOT_A = 18,                     /* DOT_A  */
  YYSYMBOL_DOT_UB = 19,                    /* DOT_UB  */
  YYSYMBOL_DOT_UW = 20,                    /* DOT_UW  */
  YYSYMBOL_DOT_D = 21,                     /* DOT_D  */
  YYSYMBOL_ABS = 22,                       /* ABS  */
  YYSYMBOL_ADC = 23,                       /* ADC  */
  YYSYMBOL_ADD = 24,                       /* ADD  */
  YYSYMBOL_AND_ = 25,                      /* AND_  */
  YYSYMBOL_BCLR = 26,                      /* BCLR  */
  YYSYMBOL_BCND = 27,                      /* BCND  */
  YYSYMBOL_BFMOV = 28,                     /* BFMOV  */
  YYSYMBOL_BFMOVZ = 29,                    /* BFMOVZ  */
  YYSYMBOL_BMCND = 30,                     /* BMCND  */
  YYSYMBOL_BNOT = 31,                      /* BNOT  */
  YYSYMBOL_BRA = 32,                       /* BRA  */
  YYSYMBOL_BRK = 33,                       /* BRK  */
  YYSYMBOL_BSET = 34,                      /* BSET  */
  YYSYMBOL_BSR = 35,                       /* BSR  */
  YYSYMBOL_BTST = 36,                      /* BTST  */
  YYSYMBOL_CLRPSW = 37,                    /* CLRPSW  */
  YYSYMBOL_CMP = 38,                       /* CMP  */
  YYSYMBOL_DABS = 39,                      /* DABS  */
  YYSYMBOL_DADD = 40,                      /* DADD  */
  YYSYMBOL_DBT = 41,                       /* DBT  */
  YYSYMBOL_DCMP = 42,                      /* DCMP  */
  YYSYMBOL_DDIV = 43,                      /* DDIV  */
  YYSYMBOL_DIV = 44,                       /* DIV  */
  YYSYMBOL_DIVU = 45,                      /* DIVU  */
  YYSYMBOL_DMOV = 46,                      /* DMOV  */
  YYSYMBOL_DMUL = 47,                      /* DMUL  */
  YYSYMBOL_DNEG = 48,                      /* DNEG  */
  YYSYMBOL_DPOPM = 49,                     /* DPOPM  */
  YYSYMBOL_DPUSHM = 50,                    /* DPUSHM  */
  YYSYMBOL_DROUND = 51,                    /* DROUND  */
  YYSYMBOL_DSQRT = 52,                     /* DSQRT  */
  YYSYMBOL_DSUB = 53,                      /* DSUB  */
  YYSYMBOL_DTOF = 54,                      /* DTOF  */
  YYSYMBOL_DTOI = 55,                      /* DTOI  */
  YYSYMBOL_DTOU = 56,                      /* DTOU  */
  YYSYMBOL_EDIV = 57,                      /* EDIV  */
  YYSYMBOL_EDIVU = 58,                     /* EDIVU  */
  YYSYMBOL_EMACA = 59,                     /* EMACA  */
  YYSYMBOL_EMSBA = 60,                     /* EMSBA  */
  YYSYMBOL_EMUL = 61,                      /* EMUL  */
  YYSYMBOL_EMULA = 62,                     /* EMULA  */
  YYSYMBOL_EMULU = 63,                     /* EMULU  */
  YYSYMBOL_FADD = 64,                      /* FADD  */
  YYSYMBOL_FCMP = 65,                      /* FCMP  */
  YYSYMBOL_FDIV = 66,                      /* FDIV  */
  YYSYMBOL_FMUL = 67,                      /* FMUL  */
  YYSYMBOL_FREIT = 68,                     /* FREIT  */
  YYSYMBOL_FSUB = 69,                      /* FSUB  */
  YYSYMBOL_FSQRT = 70,                     /* FSQRT  */
  YYSYMBOL_FTOD = 71,                      /* FTOD  */
  YYSYMBOL_FTOI = 72,                      /* FTOI  */
  YYSYMBOL_FTOU = 73,                      /* FTOU  */
  YYSYMBOL_INT = 74,                       /* INT  */
  YYSYMBOL_ITOD = 75,                      /* ITOD  */
  YYSYMBOL_ITOF = 76,                      /* ITOF  */
  YYSYMBOL_JMP = 77,                       /* JMP  */
  YYSYMBOL_JSR = 78,                       /* JSR  */
  YYSYMBOL_MACHI = 79,                     /* MACHI  */
  YYSYMBOL_MACLH = 80,                     /* MACLH  */
  YYSYMBOL_MACLO = 81,                     /* MACLO  */
  YYSYMBOL_MAX = 82,                       /* MAX  */
  YYSYMBOL_MIN = 83,                       /* MIN  */
  YYSYMBOL_MOV = 84,                       /* MOV  */
  YYSYMBOL_MOVCO = 85,                     /* MOVCO  */
  YYSYMBOL_MOVLI = 86,                     /* MOVLI  */
  YYSYMBOL_MOVU = 87,                      /* MOVU  */
  YYSYMBOL_MSBHI = 88,                     /* MSBHI  */
  YYSYMBOL_MSBLH = 89,                     /* MSBLH  */
  YYSYMBOL_MSBLO = 90,                     /* MSBLO  */
  YYSYMBOL_MUL = 91,                       /* MUL  */
  YYSYMBOL_MULHI = 92,                     /* MULHI  */
  YYSYMBOL_MULLH = 93,                     /* MULLH  */
  YYSYMBOL_MULLO = 94,                     /* MULLO  */
  YYSYMBOL_MULU = 95,                      /* MULU  */
  YYSYMBOL_MVFACHI = 96,                   /* MVFACHI  */
  YYSYMBOL_MVFACGU = 97,                   /* MVFACGU  */
  YYSYMBOL_MVFACMI = 98,                   /* MVFACMI  */
  YYSYMBOL_MVFACLO = 99,                   /* MVFACLO  */
  YYSYMBOL_MVFC = 100,                     /* MVFC  */
  YYSYMBOL_MVFDC = 101,                    /* MVFDC  */
  YYSYMBOL_MVFDR = 102,                    /* MVFDR  */
  YYSYMBOL_MVTACGU = 103,                  /* MVTACGU  */
  YYSYMBOL_MVTACHI = 104,                  /* MVTACHI  */
  YYSYMBOL_MVTACLO = 105,                  /* MVTACLO  */
  YYSYMBOL_MVTC = 106,                     /* MVTC  */
  YYSYMBOL_MVTDC = 107,                    /* MVTDC  */
  YYSYMBOL_MVTIPL = 108,                   /* MVTIPL  */
  YYSYMBOL_NEG = 109,                      /* NEG  */
  YYSYMBOL_NOP = 110,                      /* NOP  */
  YYSYMBOL_NOT = 111,                      /* NOT  */
  YYSYMBOL_OR = 112,                       /* OR  */
  YYSYMBOL_POP = 113,                      /* POP  */
  YYSYMBOL_POPC = 114,                     /* POPC  */
  YYSYMBOL_POPM = 115,                     /* POPM  */
  YYSYMBOL_PUSH = 116,                     /* PUSH  */
  YYSYMBOL_PUSHA = 117,                    /* PUSHA  */
  YYSYMBOL_PUSHC = 118,                    /* PUSHC  */
  YYSYMBOL_PUSHM = 119,                    /* PUSHM  */
  YYSYMBOL_RACL = 120,                     /* RACL  */
  YYSYMBOL_RACW = 121,                     /* RACW  */
  YYSYMBOL_RDACL = 122,                    /* RDACL  */
  YYSYMBOL_RDACW = 123,                    /* RDACW  */
  YYSYMBOL_REIT = 124,                     /* REIT  */
  YYSYMBOL_REVL = 125,                     /* REVL  */
  YYSYMBOL_REVW = 126,                     /* REVW  */
  YYSYMBOL_RMPA = 127,                     /* RMPA  */
  YYSYMBOL_ROLC = 128,                     /* ROLC  */
  YYSYMBOL_RORC = 129,                     /* RORC  */
  YYSYMBOL_ROTL = 130,                     /* ROTL  */
  YYSYMBOL_ROTR = 131,                     /* ROTR  */
  YYSYMBOL_ROUND = 132,                    /* ROUND  */
  YYSYMBOL_RSTR = 133,                     /* RSTR  */
  YYSYMBOL_RTE = 134,                      /* RTE  */
  YYSYMBOL_RTFI = 135,                     /* RTFI  */
  YYSYMBOL_RTS = 136,                      /* RTS  */
  YYSYMBOL_RTSD = 137,                     /* RTSD  */
  YYSYMBOL_SAT = 138,                      /* SAT  */
  YYSYMBOL_SATR = 139,                     /* SATR  */
  YYSYMBOL_SAVE = 140,                     /* SAVE  */
  YYSYMBOL_SBB = 141,                      /* SBB  */
  YYSYMBOL_SCCND = 142,                    /* SCCND  */
  YYSYMBOL_SCMPU = 143,                    /* SCMPU  */
  YYSYMBOL_SETPSW = 144,                   /* SETPSW  */
  YYSYMBOL_SHAR = 145,                     /* SHAR  */
  YYSYMBOL_SHLL = 146,                     /* SHLL  */
  YYSYMBOL_SHLR = 147,                     /* SHLR  */
  YYSYMBOL_SMOVB = 148,                    /* SMOVB  */
  YYSYMBOL_SMOVF = 149,                    /* SMOVF  */
  YYSYMBOL_SMOVU = 150,                    /* SMOVU  */
  YYSYMBOL_SSTR = 151,                     /* SSTR  */
  YYSYMBOL_STNZ = 152,                     /* STNZ  */
  YYSYMBOL_STOP = 153,                     /* STOP  */
  YYSYMBOL_STZ = 154,                      /* STZ  */
  YYSYMBOL_SUB = 155,                      /* SUB  */
  YYSYMBOL_SUNTIL = 156,                   /* SUNTIL  */
  YYSYMBOL_SWHILE = 157,                   /* SWHILE  */
  YYSYMBOL_TST = 158,                      /* TST  */
  YYSYMBOL_UTOD = 159,                     /* UTOD  */
  YYSYMBOL_UTOF = 160,                     /* UTOF  */
  YYSYMBOL_WAIT = 161,                     /* WAIT  */
  YYSYMBOL_XCHG = 162,                     /* XCHG  */
  YYSYMBOL_XOR = 163,                      /* XOR  */
  YYSYMBOL_164_ = 164,                     /* '#'  */
  YYSYMBOL_165_ = 165,                     /* ','  */
  YYSYMBOL_166_ = 166,                     /* '['  */
  YYSYMBOL_167_ = 167,                     /* ']'  */
  YYSYMBOL_168_ = 168,                     /* '-'  */
  YYSYMBOL_169_ = 169,                     /* '+'  */
  YYSYMBOL_YYACCEPT = 170,                 /* $accept  */
  YYSYMBOL_statement = 171,                /* statement  */
  YYSYMBOL_172_1 = 172,                    /* $@1  */
  YYSYMBOL_173_2 = 173,                    /* $@2  */
  YYSYMBOL_174_3 = 174,                    /* $@3  */
  YYSYMBOL_175_4 = 175,                    /* $@4  */
  YYSYMBOL_176_5 = 176,                    /* $@5  */
  YYSYMBOL_177_6 = 177,                    /* $@6  */
  YYSYMBOL_178_7 = 178,                    /* $@7  */
  YYSYMBOL_179_8 = 179,                    /* $@8  */
  YYSYMBOL_180_9 = 180,                    /* $@9  */
  YYSYMBOL_181_10 = 181,                   /* $@10  */
  YYSYMBOL_182_11 = 182,                   /* $@11  */
  YYSYMBOL_183_12 = 183,                   /* $@12  */
  YYSYMBOL_184_13 = 184,                   /* $@13  */
  YYSYMBOL_185_14 = 185,                   /* $@14  */
  YYSYMBOL_186_15 = 186,                   /* $@15  */
  YYSYMBOL_187_16 = 187,                   /* $@16  */
  YYSYMBOL_188_17 = 188,                   /* $@17  */
  YYSYMBOL_189_18 = 189,                   /* $@18  */
  YYSYMBOL_190_19 = 190,                   /* $@19  */
  YYSYMBOL_191_20 = 191,                   /* $@20  */
  YYSYMBOL_192_21 = 192,                   /* $@21  */
  YYSYMBOL_193_22 = 193,                   /* $@22  */
  YYSYMBOL_194_23 = 194,                   /* $@23  */
  YYSYMBOL_195_24 = 195,                   /* $@24  */
  YYSYMBOL_196_25 = 196,                   /* $@25  */
  YYSYMBOL_197_26 = 197,                   /* $@26  */
  YYSYMBOL_198_27 = 198,                   /* $@27  */
  YYSYMBOL_199_28 = 199,                   /* $@28  */
  YYSYMBOL_200_29 = 200,                   /* $@29  */
  YYSYMBOL_201_30 = 201,                   /* $@30  */
  YYSYMBOL_202_31 = 202,                   /* $@31  */
  YYSYMBOL_203_32 = 203,                   /* $@32  */
  YYSYMBOL_204_33 = 204,                   /* $@33  */
  YYSYMBOL_205_34 = 205,                   /* $@34  */
  YYSYMBOL_206_35 = 206,                   /* $@35  */
  YYSYMBOL_207_36 = 207,                   /* $@36  */
  YYSYMBOL_208_37 = 208,                   /* $@37  */
  YYSYMBOL_209_38 = 209,                   /* $@38  */
  YYSYMBOL_210_39 = 210,                   /* $@39  */
  YYSYMBOL_211_40 = 211,                   /* $@40  */
  YYSYMBOL_212_41 = 212,                   /* $@41  */
  YYSYMBOL_213_42 = 213,                   /* $@42  */
  YYSYMBOL_214_43 = 214,                   /* $@43  */
  YYSYMBOL_215_44 = 215,                   /* $@44  */
  YYSYMBOL_216_45 = 216,                   /* $@45  */
  YYSYMBOL_217_46 = 217,                   /* $@46  */
  YYSYMBOL_218_47 = 218,                   /* $@47  */
  YYSYMBOL_219_48 = 219,                   /* $@48  */
  YYSYMBOL_220_49 = 220,                   /* $@49  */
  YYSYMBOL_221_50 = 221,                   /* $@50  */
  YYSYMBOL_222_51 = 222,                   /* $@51  */
  YYSYMBOL_223_52 = 223,                   /* $@52  */
  YYSYMBOL_224_53 = 224,                   /* $@53  */
  YYSYMBOL_225_54 = 225,                   /* $@54  */
  YYSYMBOL_226_55 = 226,                   /* $@55  */
  YYSYMBOL_227_56 = 227,                   /* $@56  */
  YYSYMBOL_228_57 = 228,                   /* $@57  */
  YYSYMBOL_229_58 = 229,                   /* $@58  */
  YYSYMBOL_230_59 = 230,                   /* $@59  */
  YYSYMBOL_231_60 = 231,                   /* $@60  */
  YYSYMBOL_op_subadd = 232,                /* op_subadd  */
  YYSYMBOL_op_dp20_rm_l = 233,             /* op_dp20_rm_l  */
  YYSYMBOL_op_dp20_rm = 234,               /* op_dp20_rm  */
  YYSYMBOL_op_dp20_i = 235,                /* op_dp20_i  */
  YYSYMBOL_op_dp20_rim = 236,              /* op_dp20_rim  */
  YYSYMBOL_op_dp20_rim_l = 237,            /* op_dp20_rim_l  */
  YYSYMBOL_op_dp20_rr = 238,               /* op_dp20_rr  */
  YYSYMBOL_op_dp20_r = 239,                /* op_dp20_r  */
  YYSYMBOL_op_dp20_ri = 240,               /* op_dp20_ri  */
  YYSYMBOL_241_61 = 241,                   /* $@61  */
  YYSYMBOL_op_xchg = 242,                  /* op_xchg  */
  YYSYMBOL_op_shift_rot = 243,             /* op_shift_rot  */
  YYSYMBOL_op_shift = 244,                 /* op_shift  */
  YYSYMBOL_float3_op = 245,                /* float3_op  */
  YYSYMBOL_float2_op = 246,                /* float2_op  */
  YYSYMBOL_247_62 = 247,                   /* $@62  */
  YYSYMBOL_float2_op_ni = 248,             /* float2_op_ni  */
  YYSYMBOL_249_63 = 249,                   /* $@63  */
  YYSYMBOL_250_64 = 250,                   /* $@64  */
  YYSYMBOL_mvfa_op = 251,                  /* mvfa_op  */
  YYSYMBOL_252_65 = 252,                   /* $@65  */
  YYSYMBOL_op_xor = 253,                   /* op_xor  */
  YYSYMBOL_op_bfield = 254,                /* op_bfield  */
  YYSYMBOL_255_66 = 255,                   /* $@66  */
  YYSYMBOL_op_save_rstr = 256,             /* op_save_rstr  */
  YYSYMBOL_double2_op = 257,               /* double2_op  */
  YYSYMBOL_double3_op = 258,               /* double3_op  */
  YYSYMBOL_disp = 259,                     /* disp  */
  YYSYMBOL_flag = 260,                     /* flag  */
  YYSYMBOL_261_67 = 261,                   /* $@67  */
  YYSYMBOL_memex = 262,                    /* memex  */
  YYSYMBOL_bwl = 263,                      /* bwl  */
  YYSYMBOL_bw = 264,                       /* bw  */
  YYSYMBOL_opt_l = 265,                    /* opt_l  */
  YYSYMBOL_opt_b = 266                     /* opt_b  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int16 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  307
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   967

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  170
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  97
/* YYNRULES -- Number of rules.  */
#define YYNRULES  356
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  924

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   418


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,   164,     2,     2,     2,     2,
       2,     2,     2,   169,   165,   168,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   166,     2,   167,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,   158,   159,   160,   161,   162,   163
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   184,   184,   189,   192,   195,   198,   203,   218,   221,
     226,   235,   240,   248,   251,   256,   258,   260,   265,   283,
     286,   289,   292,   300,   306,   314,   323,   328,   331,   336,
     341,   344,   352,   359,   367,   373,   379,   385,   391,   399,
     409,   414,   414,   415,   415,   416,   416,   420,   433,   446,
     451,   456,   458,   463,   468,   470,   472,   477,   482,   487,
     497,   507,   509,   514,   516,   518,   520,   525,   527,   529,
     531,   536,   538,   540,   545,   550,   552,   554,   556,   561,
     567,   575,   589,   594,   599,   604,   609,   614,   616,   618,
     623,   628,   628,   629,   629,   630,   630,   631,   631,   632,
     632,   633,   633,   634,   634,   635,   635,   636,   636,   637,
     637,   638,   638,   639,   639,   640,   640,   641,   641,   642,
     642,   646,   646,   647,   647,   648,   648,   649,   649,   650,
     650,   654,   656,   658,   660,   663,   665,   667,   669,   674,
     674,   675,   675,   676,   676,   677,   677,   678,   678,   679,
     679,   680,   680,   681,   681,   682,   682,   689,   691,   696,
     702,   708,   710,   712,   714,   716,   718,   720,   722,   728,
     730,   732,   734,   736,   738,   738,   739,   741,   741,   742,
     744,   744,   745,   753,   764,   766,   771,   773,   778,   780,
     785,   785,   786,   786,   787,   787,   788,   788,   792,   800,
     807,   809,   814,   821,   827,   832,   835,   838,   843,   843,
     844,   844,   845,   845,   846,   846,   847,   847,   852,   857,
     862,   867,   869,   871,   873,   875,   877,   879,   881,   883,
     883,   884,   886,   894,   902,   912,   912,   913,   913,   916,
     916,   917,   917,   920,   920,   921,   921,   922,   922,   923,
     923,   924,   924,   925,   925,   926,   926,   927,   927,   928,
     928,   929,   929,   930,   930,   931,   933,   936,   939,   942,
     945,   948,   951,   954,   958,   961,   965,   968,   971,   974,
     977,   980,   983,   986,   989,   991,   994,   997,  1000,  1011,
    1013,  1015,  1017,  1024,  1026,  1034,  1036,  1038,  1044,  1049,
    1050,  1054,  1055,  1059,  1061,  1066,  1071,  1071,  1073,  1078,
    1080,  1082,  1089,  1093,  1095,  1097,  1101,  1103,  1105,  1107,
    1112,  1112,  1115,  1119,  1119,  1122,  1122,  1128,  1128,  1151,
    1152,  1157,  1157,  1165,  1167,  1172,  1176,  1181,  1182,  1185,
    1185,  1190,  1191,  1192,  1193,  1194,  1197,  1198,  1199,  1200,
    1203,  1204,  1205,  1208,  1209,  1212,  1213
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "REG", "FLAG", "CREG",
  "ACC", "DREG", "DREGH", "DREGL", "DCREG", "EXPR", "UNKNOWN_OPCODE",
  "IS_OPCODE", "DOT_S", "DOT_B", "DOT_W", "DOT_L", "DOT_A", "DOT_UB",
  "DOT_UW", "DOT_D", "ABS", "ADC", "ADD", "AND_", "BCLR", "BCND", "BFMOV",
  "BFMOVZ", "BMCND", "BNOT", "BRA", "BRK", "BSET", "BSR", "BTST", "CLRPSW",
  "CMP", "DABS", "DADD", "DBT", "DCMP", "DDIV", "DIV", "DIVU", "DMOV",
  "DMUL", "DNEG", "DPOPM", "DPUSHM", "DROUND", "DSQRT", "DSUB", "DTOF",
  "DTOI", "DTOU", "EDIV", "EDIVU", "EMACA", "EMSBA", "EMUL", "EMULA",
  "EMULU", "FADD", "FCMP", "FDIV", "FMUL", "FREIT", "FSUB", "FSQRT",
  "FTOD", "FTOI", "FTOU", "INT", "ITOD", "ITOF", "JMP", "JSR", "MACHI",
  "MACLH", "MACLO", "MAX", "MIN", "MOV", "MOVCO", "MOVLI", "MOVU", "MSBHI",
  "MSBLH", "MSBLO", "MUL", "MULHI", "MULLH", "MULLO", "MULU", "MVFACHI",
  "MVFACGU", "MVFACMI", "MVFACLO", "MVFC", "MVFDC", "MVFDR", "MVTACGU",
  "MVTACHI", "MVTACLO", "MVTC", "MVTDC", "MVTIPL", "NEG", "NOP", "NOT",
  "OR", "POP", "POPC", "POPM", "PUSH", "PUSHA", "PUSHC", "PUSHM", "RACL",
  "RACW", "RDACL", "RDACW", "REIT", "REVL", "REVW", "RMPA", "ROLC", "RORC",
  "ROTL", "ROTR", "ROUND", "RSTR", "RTE", "RTFI", "RTS", "RTSD", "SAT",
  "SATR", "SAVE", "SBB", "SCCND", "SCMPU", "SETPSW", "SHAR", "SHLL",
  "SHLR", "SMOVB", "SMOVF", "SMOVU", "SSTR", "STNZ", "STOP", "STZ", "SUB",
  "SUNTIL", "SWHILE", "TST", "UTOD", "UTOF", "WAIT", "XCHG", "XOR", "'#'",
  "','", "'['", "']'", "'-'", "'+'", "$accept", "statement", "$@1", "$@2",
  "$@3", "$@4", "$@5", "$@6", "$@7", "$@8", "$@9", "$@10", "$@11", "$@12",
  "$@13", "$@14", "$@15", "$@16", "$@17", "$@18", "$@19", "$@20", "$@21",
  "$@22", "$@23", "$@24", "$@25", "$@26", "$@27", "$@28", "$@29", "$@30",
  "$@31", "$@32", "$@33", "$@34", "$@35", "$@36", "$@37", "$@38", "$@39",
  "$@40", "$@41", "$@42", "$@43", "$@44", "$@45", "$@46", "$@47", "$@48",
  "$@49", "$@50", "$@51", "$@52", "$@53", "$@54", "$@55", "$@56", "$@57",
  "$@58", "$@59", "$@60", "op_subadd", "op_dp20_rm_l", "op_dp20_rm",
  "op_dp20_i", "op_dp20_rim", "op_dp20_rim_l", "op_dp20_rr", "op_dp20_r",
  "op_dp20_ri", "$@61", "op_xchg", "op_shift_rot", "op_shift", "float3_op",
  "float2_op", "$@62", "float2_op_ni", "$@63", "$@64", "mvfa_op", "$@65",
  "op_xor", "op_bfield", "$@66", "op_save_rstr", "double2_op",
  "double3_op", "disp", "flag", "$@67", "memex", "bwl", "bw", "opt_l",
  "opt_b", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-728)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-324)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     323,  -728,  -728,  -728,  -136,  -129,     2,   116,  -728,  -728,
    -120,    29,   127,  -728,    31,   136,    33,  -728,    12,  -728,
    -728,  -728,    45,  -728,  -728,  -728,   170,  -728,  -728,   183,
     193,  -728,  -728,  -728,  -728,  -728,  -728,    66,    77,  -118,
      79,   -98,  -728,  -728,  -728,  -728,  -728,  -728,   110,  -728,
    -728,   -30,   143,  -728,   155,   158,   191,   210,   221,  -728,
    -728,    41,   244,    85,    34,   249,   250,   251,    99,   252,
     253,   254,   255,  -728,   256,   257,   259,   258,  -728,   262,
     263,   264,    35,   266,   112,  -728,  -728,  -728,   113,   268,
     269,   270,   162,   273,   272,   115,   118,   119,   120,  -728,
    -728,   162,   277,   282,   124,   128,  -728,  -728,  -728,  -728,
    -728,   129,   286,  -728,  -728,   130,   227,  -728,  -728,  -728,
    -728,  -728,  -728,  -728,  -728,   162,  -728,  -728,   131,   162,
     162,  -728,   287,  -728,  -728,  -728,  -728,   261,   288,    19,
     285,    64,   289,    64,   132,   290,  -728,   291,   292,   294,
    -728,  -728,   295,   133,   296,  -728,   297,   298,   299,  -728,
     300,   309,   134,   303,  -728,   304,   305,   314,   153,   308,
    -728,   317,   157,  -728,   312,   160,   320,   321,   159,   321,
      22,    22,    18,     6,   321,   320,   319,   324,   322,   326,
     320,   320,   321,   320,   320,   320,   165,   169,   172,    91,
     173,   172,    91,    26,    37,    37,    26,    26,   334,   174,
     334,   334,   329,   176,    91,  -728,  -728,   177,   178,   179,
      22,    22,   216,   276,   283,   380,     5,   311,   415,  -728,
    -728,     7,   327,   330,   332,   476,    64,   333,   335,   336,
    -728,  -728,  -728,  -728,  -728,  -728,  -728,   337,   338,   339,
     340,   341,   342,   478,   343,   480,   288,   288,   483,    64,
    -728,  -728,   344,  -728,  -728,  -728,    92,  -728,   346,   498,
     499,   500,   504,   513,   513,  -728,  -728,  -728,   506,   513,
     507,   513,   334,    38,   508,  -728,    38,   509,    93,   518,
     511,  -728,    39,    39,    39,  -728,   172,   172,   512,    64,
    -728,  -728,    22,   359,    91,    91,    28,  -728,   360,  -728,
     361,   516,  -728,  -728,  -728,   362,   364,   365,  -728,   366,
     368,  -728,    94,   369,  -728,  -728,  -728,  -728,   367,  -728,
     370,    95,   371,  -728,  -728,  -728,  -728,  -728,    96,   372,
    -728,  -728,  -728,    97,   373,  -728,   536,   375,   538,   377,
    -728,   378,  -728,   537,  -728,   381,  -728,  -728,  -728,   379,
    -728,   382,   383,   384,   539,   386,   387,   542,   551,   389,
    -728,  -728,   388,   390,   391,   392,  -728,  -728,  -728,  -728,
    -728,  -728,   554,   558,  -728,   397,  -728,   398,   560,  -728,
    -728,   400,   555,  -728,   401,  -728,   404,  -728,   566,   511,
    -728,  -728,  -728,  -728,   563,  -728,  -728,  -728,   564,  -728,
     569,   570,   571,  -728,  -728,   565,   567,   568,   410,   412,
     414,    -1,   416,   417,   418,   419,     0,   578,   583,   584,
     423,  -728,   586,   587,   588,  -728,   428,  -728,  -728,  -728,
     590,   591,   589,   592,   593,   595,   431,   594,  -728,  -728,
    -728,   432,  -728,   598,  -728,   436,   600,   440,   441,   442,
     443,   444,  -728,  -728,   445,  -728,   446,  -728,  -728,  -728,
     601,  -728,   448,  -728,   449,  -728,  -728,   450,   604,  -728,
    -728,  -728,  -728,  -728,  -728,   614,  -728,   453,  -728,  -728,
     612,  -728,  -728,   455,  -728,  -728,   618,   619,   458,   621,
     622,   623,   624,   625,  -728,   463,    98,   620,   107,  -728,
     464,   108,  -728,   466,   109,  -728,   467,   111,  -728,   631,
     468,   629,   630,  -728,   635,   636,   231,   637,   638,   477,
     642,    40,   479,   484,   640,   643,   639,   644,   645,   490,
     491,   654,   655,   494,   657,   496,   659,   634,   501,   497,
    -728,  -728,   502,   503,   505,   510,   514,   515,   661,     8,
     662,    62,   666,   668,   517,   669,   670,    63,   671,   519,
     520,   521,   673,   522,   523,   524,   667,  -728,  -728,  -728,
    -728,  -728,  -728,   672,  -728,   678,  -728,   680,  -728,   684,
     685,   686,   687,   691,   692,   693,  -728,   695,   696,   697,
     540,   541,  -728,   698,  -728,   699,  -728,  -728,   700,   543,
     544,   546,   545,  -728,   701,  -728,   547,   549,  -728,   550,
     704,  -728,   552,   705,  -728,   553,   712,  -728,   556,  -728,
     140,  -728,   559,  -728,   561,  -728,  -728,  -728,  -728,   237,
    -728,  -728,   714,   557,   713,   562,   572,  -728,  -728,  -728,
    -728,   719,   720,  -728,   573,   723,   576,   717,   575,   579,
     727,   728,   729,   730,   731,    -4,    32,     9,  -728,  -728,
     577,     1,   581,   735,   580,   582,   585,   596,   743,  -728,
     597,   747,   602,   599,   603,   745,   748,   749,  -728,   750,
     751,   752,   606,  -728,  -728,   605,  -728,  -728,  -728,  -728,
    -728,  -728,  -728,   607,  -728,   609,   756,   757,  -728,   608,
    -728,   245,   758,   759,   192,   610,   762,   615,   765,   611,
     766,   613,   771,   616,   778,  -728,  -728,  -728,   617,  -728,
     626,   726,   200,  -728,  -728,   627,   781,  -728,   746,   628,
    -728,  -728,   206,  -728,   782,  -728,   245,   783,  -728,   632,
    -728,  -728,  -728,   784,   641,   785,   646,  -728,   786,   647,
     787,    68,   789,   633,   648,   125,   649,   651,  -728,  -728,
     652,   653,   792,   656,   658,  -728,  -728,  -728,  -728,  -728,
    -728,   790,  -728,   794,  -728,   660,  -728,   797,   663,  -728,
    -728,   664,   665,   791,   674,   793,   675,   791,   676,   791,
     677,   791,   679,   798,   799,  -728,   682,   683,  -728,   688,
    -728,   801,   689,   694,  -728,   702,  -728,   245,   690,   802,
     703,   806,   706,   807,   707,   815,  -728,   708,   709,   126,
     715,  -728,   711,   816,   819,   821,   716,  -728,   823,   824,
     718,  -728,   828,  -728,   829,   830,   831,  -728,  -728,   820,
     721,   791,  -728,   791,  -728,   822,  -728,   825,  -728,  -728,
     833,   835,  -728,  -728,   836,   842,   846,   722,  -728,   724,
    -728,   725,  -728,   732,  -728,   733,  -728,  -728,  -728,   736,
     847,   848,  -728,  -728,  -728,   849,  -728,  -728,   850,  -728,
    -728,  -728,  -728,  -728,   734,  -728,  -728,  -728,  -728,  -728,
    -728,  -728,  -728,  -728,   853,  -728,  -728,  -728,  -728,   855,
    -728,   737,  -728,  -728,   851,  -728,   738,  -728,   741,  -728,
     857,   742,   858,  -728
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int16 yydefact[] =
{
       0,     2,    97,    95,   210,   214,     0,     0,   235,   237,
       0,     0,   353,     3,     0,   353,     0,   339,   337,   243,
     257,     4,     0,   259,   107,   109,     0,   261,   245,     0,
       0,   247,   249,   263,   251,   253,   255,     0,     0,   121,
       0,   123,   143,   141,   147,   145,   139,   149,     0,   151,
     153,     0,     0,   127,     0,     0,     0,     0,     0,    99,
     101,   346,     0,     0,   350,     0,     0,     0,   212,     0,
       0,     0,   174,   229,   177,   180,     0,     0,   284,     0,
       0,     0,     0,     0,     0,    93,     6,   115,   216,     0,
       0,     0,   346,     0,     0,     0,     0,     0,     0,   196,
     194,   346,     0,     0,   190,   192,   155,   239,    76,    75,
       5,     0,     0,    78,   241,    91,   346,    67,   339,    43,
      45,    41,    69,    70,    68,   346,   119,   117,   208,   346,
     346,   111,     0,   129,    77,   125,   113,     0,     0,   337,
       0,   337,     0,   337,     0,     0,    18,     0,     0,     0,
     331,   331,     0,     0,     0,     7,     0,     0,     0,   354,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
      62,     0,     0,   338,     0,     0,     0,     0,     0,     0,
     337,   337,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   337,
       0,     0,   337,   337,   325,   325,   337,   337,   325,     0,
     325,   325,     0,     0,   337,    63,    64,     0,     0,     0,
     337,   337,   347,   348,   349,     0,     0,     0,     0,   351,
     352,     0,     0,     0,     0,     0,   337,     0,     0,     0,
     173,   327,   327,   176,   327,   179,   327,     0,     0,     0,
     169,   171,     0,     0,     0,     0,     0,     0,     0,   337,
      58,    60,     0,   347,   348,   349,   337,    59,     0,     0,
       0,     0,     0,     0,     0,    74,    56,    55,     0,     0,
       0,     0,   325,     0,     0,    54,     0,     0,   337,   349,
     337,    61,     0,     0,     0,    73,   306,   306,     0,   337,
      71,    72,   337,     0,   337,   337,   337,     1,   304,    98,
       0,     0,   301,   302,    96,     0,     0,     0,   211,     0,
       0,   215,   337,     0,    12,    13,    17,   236,     0,   238,
       0,   337,     0,     9,    14,    15,     8,    65,   337,     0,
      16,    11,    66,   337,     0,   340,     0,     0,     0,     0,
     244,     0,   258,     0,   260,     0,   299,   300,   108,     0,
     110,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     262,   246,     0,     0,     0,     0,   248,   250,   264,   252,
     254,   256,     0,     0,   104,     0,   122,     0,     0,   106,
     124,     0,     0,   144,     0,   142,     0,   322,     0,   337,
     148,   146,   140,   150,     0,   152,   154,    50,     0,   128,
       0,     0,     0,   100,   102,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   213,     0,     0,     0,   175,     0,   230,   178,   181,
       0,     0,     0,     0,     0,     0,     0,     0,    79,    94,
     116,     0,   217,     0,    57,     0,     0,     0,   182,     0,
       0,     0,   197,   195,     0,   191,     0,   193,   156,   334,
       0,   240,    40,   242,     0,    92,   157,     0,     0,   315,
      44,    46,    42,   308,   120,     0,   118,     0,   209,   112,
       0,   130,   126,     0,   329,   114,     0,     0,     0,     0,
       0,     0,     0,     0,   132,     0,   337,     0,   337,   134,
       0,   337,   131,     0,   337,   133,     0,   337,    26,     0,
       0,     0,     0,   265,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     286,   287,   165,     0,   167,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   161,     0,   163,     0,   199,   283,   231,
     170,   172,   198,     0,   285,     0,    48,     0,    47,     0,
       0,     0,     0,     0,     0,     0,   333,     0,     0,     0,
       0,     0,   307,     0,   288,     0,   303,   293,     0,     0,
      34,   289,     0,    36,     0,    52,     0,     0,   203,     0,
       0,   204,     0,     0,    51,     0,     0,    53,     0,    33,
     343,   335,     0,   295,     0,   267,   268,   269,   270,     0,
     266,   271,     0,     0,     0,     0,     0,   280,   279,   282,
     281,     0,     0,   309,     0,     0,   317,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    39,    85,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    29,
       0,     0,     0,     0,     0,     0,     0,     0,    35,     0,
       0,     0,     0,   202,    37,     0,   232,   183,   233,   234,
     312,   200,   201,     0,   218,     0,     0,     0,    32,   295,
     298,   353,     0,     0,   343,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   341,   342,   344,     0,   345,
       0,     0,   343,   277,   278,     0,     0,   276,     0,     0,
     221,   222,   343,   223,     0,   316,   353,     0,   324,     0,
     166,   224,   168,     0,     0,     0,     0,    38,     0,     0,
       0,     0,     0,     0,     0,   337,     0,     0,   219,   220,
       0,     0,     0,     0,     0,   225,   226,   227,   162,   228,
     164,     0,    90,     0,   158,   313,   305,     0,     0,    49,
     292,     0,     0,   355,     0,     0,     0,   355,     0,   355,
       0,   355,     0,     0,     0,   336,     0,     0,   272,     0,
     274,     0,     0,     0,   319,     0,   321,   353,     0,     0,
       0,     0,     0,     0,     0,     0,    82,     0,     0,   337,
       0,    86,     0,     0,     0,     0,     0,    30,     0,     0,
       0,    25,     0,   330,     0,     0,     0,   356,   136,     0,
       0,   355,   138,   355,   135,     0,   137,     0,    27,    28,
       0,     0,   273,   275,     0,     0,     0,     0,    19,     0,
      20,     0,    21,     0,    80,     0,   184,   185,    81,     0,
       0,     0,   186,   187,    31,     0,   188,   189,     0,   314,
     294,   290,   291,    88,     0,   159,   160,    87,    89,   296,
     297,   310,   311,   318,     0,    22,    23,    24,   205,     0,
     206,     0,   207,   328,     0,   326,     0,    83,     0,    84,
       0,     0,     0,   332
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -728,  -728,  -728,  -728,  -728,  -728,  -728,  -728,  -728,  -728,
    -728,  -728,  -728,  -728,  -728,  -728,  -728,  -728,  -728,  -728,
    -728,  -728,  -728,  -728,  -728,  -728,  -728,  -728,  -728,  -728,
    -728,  -728,  -728,  -728,  -728,  -728,  -728,  -728,  -728,  -728,
    -728,  -728,  -728,  -728,  -728,  -728,  -728,  -728,  -728,  -728,
    -728,  -728,  -728,  -728,  -728,  -728,  -728,  -728,  -728,  -728,
    -728,  -728,  -119,   650,  -728,  -133,  -167,  -728,  -141,  -728,
     437,  -728,  -154,  -196,  -145,    43,   710,  -728,  -149,  -728,
    -728,    -8,  -728,  -728,   739,  -728,   681,  -104,  -108,   -18,
     753,  -728,  -669,   -37,  -728,   -14,  -727
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,   137,   294,   292,   293,   288,   256,   139,   138,   220,
     221,   198,   201,   180,   181,   302,   306,   257,   297,   296,
     199,   202,   305,   214,   304,   207,   204,   203,   206,   205,
     208,   210,   211,   282,   241,   244,   246,   279,   281,   274,
     273,   299,   141,   236,   143,   259,   242,   150,   151,   283,
     286,   176,   185,   190,   191,   193,   194,   195,   177,   179,
     184,   192,   318,   312,   356,   357,   358,   314,   309,   602,
     484,   485,   386,   479,   480,   393,   395,   396,   397,   398,
     399,   435,   436,   495,   327,   328,   471,   350,   352,   359,
     170,   171,   730,   226,   231,   161,   848
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     175,   167,   561,   567,   761,   144,   313,   173,   419,   365,
     424,   669,   757,   366,   360,   172,   420,   173,   425,   670,
     173,   361,   310,   173,   321,   355,   362,   363,   140,   391,
     173,   493,   153,   173,   162,   142,   168,   173,   252,   173,
    -323,   469,   461,   173,   152,   792,  -103,   641,   390,   229,
     230,   173,   178,   413,   414,   266,   222,   223,   224,   403,
     409,   405,   406,   807,   275,   384,  -105,   317,   389,   196,
     852,   354,   854,   813,   856,   173,   370,   462,   463,   290,
     197,   371,   200,   465,   378,   467,   376,   377,   295,   379,
     380,   381,   300,   301,   385,   454,   310,   504,   509,   512,
     515,   615,   173,   173,   173,   173,   173,   173,   173,   173,
     618,   621,   624,   209,   627,   449,   450,   431,   173,   173,
     173,   315,   173,   319,   895,   319,   896,   146,   831,   878,
     147,   148,   149,   468,   212,   489,   173,   173,   155,   494,
     452,   156,   157,   158,   159,   160,   213,   164,   481,   482,
     491,   492,   165,   159,   166,   725,   726,   727,   215,   728,
     729,   216,   753,   483,   483,   369,   145,   562,   568,   762,
     367,   421,   368,   426,   671,   758,   174,   263,   264,   265,
     488,   387,   364,   311,   387,   394,   311,   182,   394,   394,
     392,   183,   311,   154,   217,   163,   387,   169,   755,   253,
     186,  -320,   470,   478,   187,   225,   642,   725,   726,   727,
     188,   791,   729,   218,   189,   725,   726,   727,   319,   806,
     729,   725,   726,   727,   219,   812,   729,   673,   681,   674,
     682,   675,   683,   825,   437,   826,   438,   827,   439,   635,
     636,   319,   263,   264,   289,   733,   734,   227,   455,   401,
     402,   228,   232,   233,   234,   237,   238,   239,   240,   243,
     245,   307,   159,   235,   247,   249,   250,   251,   248,   254,
     315,   260,   477,   262,   261,   268,   255,   258,   267,   269,
     276,   319,   270,   271,   272,   277,   387,   387,   278,   285,
     303,   308,   280,   284,   287,   298,   316,   322,   331,   338,
     320,   323,   324,   325,   505,   326,   330,   332,   333,   334,
     335,   336,   337,   510,   339,   340,   341,   342,   343,   344,
     513,   345,   346,   347,   353,   516,   348,   349,   351,   372,
     382,   373,   374,   375,   383,     1,   311,  -323,   388,   404,
     407,   408,   410,   411,   412,     2,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
     415,   549,    37,    38,    39,    40,    41,    42,    43,    44,
      45,   418,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,   423,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
     416,    93,    94,    95,    96,    97,    98,   417,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   422,   127,   128,   129,
     130,   131,   132,   133,   134,   135,   136,   430,   616,   446,
     619,   448,   427,   622,   451,   428,   625,   429,   432,   628,
     433,   434,   440,   441,   442,   443,   444,   445,   447,   457,
     458,   459,   453,   643,   456,   460,   461,   464,   466,   472,
     474,   476,   173,   487,   490,   496,   497,   498,   499,   500,
     501,   507,   502,   503,   506,   508,   511,   514,   517,   518,
     519,   520,   521,   522,   523,   525,   524,   526,   527,   528,
     529,   530,   531,   532,   533,   534,   535,   539,   536,   537,
     538,   540,   541,   543,   542,   544,   545,   546,   547,   548,
     550,   551,   552,   553,   554,   558,   555,   559,   556,   557,
     560,   569,   563,   565,   564,   566,   570,   571,   572,   573,
     574,   575,   576,   577,   578,   579,   583,   585,   580,   581,
     582,   586,   587,   588,   584,   589,   590,   591,   592,   593,
     594,   595,   596,   597,   598,   600,   599,   601,   603,   604,
     605,   606,   607,   608,   609,   610,   611,   612,   613,   614,
     620,   617,   623,   626,   629,   630,   631,   632,   633,   634,
     637,   638,   639,   646,   644,   659,   648,   754,   756,   759,
     640,   645,   650,   647,   649,   651,   652,   653,   654,   655,
     656,   657,   658,   661,   668,   672,   660,   662,   663,   676,
     664,   677,   679,   680,   684,   665,   688,   693,   692,   666,
     667,   694,   678,   695,   685,   686,   687,   689,   690,   691,
     696,   697,   698,   699,   700,   701,   702,   788,   703,   704,
     705,   708,   709,   710,   715,   706,   707,   719,   721,   712,
     711,   713,   714,   716,   717,   723,   718,   735,   720,   722,
     745,   737,   724,   736,   731,   740,   741,   738,   732,   743,
     748,   749,   815,   805,   486,   750,   751,   752,   764,   739,
     742,   744,   746,   760,   747,   765,   769,   832,   763,   766,
     771,   775,   767,   810,   776,   777,   778,   779,   780,   785,
     786,   789,   790,   768,   770,   794,   773,   772,   796,   798,
     774,   781,   782,   787,   800,   783,   784,   793,   797,   795,
     799,   802,   803,   801,   809,   814,   816,   818,   820,   822,
     824,   804,   828,   811,   808,   837,   840,   841,   829,   817,
     843,   858,   859,   867,   850,   869,   847,   819,   863,   871,
     873,   879,   821,   823,   833,   830,   834,   835,   875,   882,
     836,   838,   883,   839,   884,   842,   886,   887,   844,   845,
     846,   889,   890,   891,   892,   893,   899,   897,   900,   901,
     898,   849,   851,   853,   855,   902,   857,   860,   861,   903,
     910,   911,   912,   913,   864,   862,   915,   868,   916,   865,
     921,   923,   918,     0,     0,     0,     0,   866,     0,     0,
     870,   291,     0,   872,   874,   876,   877,   881,     0,     0,
     880,   885,     0,   888,     0,     0,   894,   904,     0,     0,
     329,   905,   906,     0,     0,     0,     0,     0,   914,   907,
     908,     0,   909,     0,   917,   919,   920,   922,     0,     0,
       0,     0,     0,     0,     0,   400,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   475,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   473
};

static const yytype_int16 yycheck[] =
{
      18,    15,     3,     3,     3,     3,   139,    11,     3,     3,
       3,     3,     3,     7,   181,     3,    11,    11,    11,    11,
      11,     3,     3,    11,   143,     3,     8,     9,   164,     3,
      11,     3,     3,    11,     3,   164,     3,    11,     3,    11,
       3,     3,     3,    11,   164,   714,   164,     7,   202,    15,
      16,    11,     7,   220,   221,    92,    15,    16,    17,   208,
     214,   210,   211,   732,   101,   198,   164,     3,   201,     3,
     797,   179,   799,   742,   801,    11,   184,   273,   274,   116,
       3,   185,     3,   279,   192,   281,   190,   191,   125,   193,
     194,   195,   129,   130,     3,     3,     3,     3,     3,     3,
       3,     3,    11,    11,    11,    11,    11,    11,    11,    11,
       3,     3,     3,     3,     3,   256,   257,   236,    11,    11,
      11,   139,    11,   141,   851,   143,   853,    11,     3,     3,
      14,    15,    16,   282,   164,   302,    11,    11,    11,   306,
     259,    14,    15,    16,    17,    18,     3,    11,   293,   294,
     304,   305,    16,    17,    18,    15,    16,    17,     3,    19,
      20,     3,   166,   296,   297,   183,   164,   168,   168,   168,
     164,   166,   166,   166,   166,   166,   164,    15,    16,    17,
     299,   199,   164,   164,   202,   203,   164,    17,   206,   207,
     164,    21,   164,   164,     3,   164,   214,   164,   166,   164,
      17,   164,   164,   164,    21,   164,   166,    15,    16,    17,
      17,    19,    20,     3,    21,    15,    16,    17,   236,    19,
      20,    15,    16,    17,     3,    19,    20,   165,   165,   167,
     167,   169,   169,   165,   242,   167,   244,   169,   246,     8,
       9,   259,    15,    16,    17,     8,     9,     3,   266,   206,
     207,   166,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     0,    17,   164,     5,     3,     3,     3,    10,     3,
     288,     3,   290,     3,     5,     3,   164,   164,     5,   164,
       3,   299,   164,   164,   164,     3,   304,   305,   164,     3,
       3,     3,   164,   164,   164,   164,    11,   165,   165,   165,
      11,    11,    11,    11,   322,    11,    11,    11,    11,    11,
      11,    11,     3,   331,    11,    11,    11,     3,   165,    11,
     338,     4,   165,    11,   165,   343,   166,     7,     7,    10,
     165,     7,    10,     7,   165,    12,   164,     3,   165,   165,
      11,   165,   165,   165,   165,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
     164,   399,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    11,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,     3,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     164,   118,   119,   120,   121,   122,   123,   164,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   165,   154,   155,   156,
     157,   158,   159,   160,   161,   162,   163,    11,   506,    11,
     508,    11,   165,   511,    11,   165,   514,   165,   165,   517,
     165,   165,   165,   165,   165,   165,   165,   165,   165,    11,
      11,    11,   168,   531,   168,    11,     3,    11,    11,    11,
      11,     3,    11,    11,   165,   165,   165,    11,   166,   165,
     165,   164,   166,   165,   165,   165,   165,   165,   165,     3,
     165,     3,   165,   165,     7,   166,   165,   165,   165,   165,
      11,   165,   165,    11,     3,   166,   168,     3,   168,   168,
     168,     3,   165,     3,   166,   165,    11,   166,   164,     3,
       7,     7,     3,     3,     3,   165,    11,   165,    11,    11,
     166,     3,   166,   165,   167,   166,     3,     3,   165,     3,
       3,     3,   164,     3,     3,     6,   165,   165,     6,     6,
       5,     3,   166,     3,    10,   165,   165,   165,   165,   165,
     165,   165,    11,   165,   165,    11,   166,     3,   165,     7,
     165,     3,     3,   165,     3,     3,     3,     3,     3,   166,
     166,    11,   166,   166,     3,   167,     7,     7,     3,     3,
       3,     3,   165,     3,   165,    11,     7,   665,   666,   667,
       8,   167,     7,    10,    10,   165,   165,     3,     3,   165,
       3,   165,     3,   166,     3,     3,   165,   165,   165,     3,
     165,     3,     3,     3,     3,   165,     3,     5,    11,   165,
     165,     3,   165,     3,   165,   165,   165,   165,   165,   165,
       6,     6,     6,     6,     3,     3,     3,   711,     3,     3,
       3,     3,     3,     3,     3,   165,   165,     3,     3,   165,
     167,   165,   167,   166,   165,     3,   166,     3,   166,   166,
       3,     8,   166,   166,   165,     6,     6,   165,   167,     6,
       3,     3,   746,     7,   297,     6,     6,     6,     3,   167,
     167,   165,   167,   166,   165,   165,     3,   765,   167,   167,
       3,     6,   167,     7,     6,     6,     6,     6,     6,     3,
       3,     3,     3,   167,   167,     3,   167,   165,     3,     3,
     167,   165,   167,   165,     3,   168,   167,   167,   167,   164,
     167,     3,   165,   167,     3,     3,     3,     3,     3,     3,
       3,   165,     3,   165,   167,     3,     6,     3,   165,   167,
       3,     3,     3,   817,    11,     3,    15,   166,     7,     3,
       3,   829,   166,   166,   165,   167,   165,   165,     3,     3,
     167,   165,     3,   165,     3,   165,     3,     3,   165,   165,
     165,     3,     3,     3,     3,    15,     3,    15,     3,     3,
      15,   167,   167,   167,   167,     3,   167,   165,   165,     3,
       3,     3,     3,     3,   165,   167,     3,   167,     3,   165,
       3,     3,    11,    -1,    -1,    -1,    -1,   165,    -1,    -1,
     167,   118,    -1,   167,   167,   167,   167,   166,    -1,    -1,
     165,   165,    -1,   165,    -1,    -1,   165,   165,    -1,    -1,
     151,   167,   167,    -1,    -1,    -1,    -1,    -1,   164,   167,
     167,    -1,   166,    -1,   167,   167,   165,   165,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   205,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   288,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   286
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int16 yystos[] =
{
       0,    12,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   118,   119,   120,   121,   122,   123,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   134,   135,
     136,   137,   138,   139,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   154,   155,   156,
     157,   158,   159,   160,   161,   162,   163,   171,   178,   177,
     164,   212,   164,   214,     3,   164,    11,    14,    15,    16,
     217,   218,   164,     3,   164,    11,    14,    15,    16,    17,
      18,   265,     3,   164,    11,    16,    18,   265,     3,   164,
     260,   261,     3,    11,   164,   259,   221,   228,     7,   229,
     183,   184,    17,    21,   230,   222,    17,    21,    17,    21,
     223,   224,   231,   225,   226,   227,     3,     3,   181,   190,
       3,   182,   191,   197,   196,   199,   198,   195,   200,     3,
     201,   202,   164,     3,   193,     3,     3,     3,     3,     3,
     179,   180,    15,    16,    17,   164,   263,     3,   166,    15,
      16,   264,     3,     3,     3,   164,   213,     3,     3,     3,
       3,   204,   216,     3,   205,     3,   206,     5,    10,     3,
       3,     3,     3,   164,     3,   164,   176,   187,   164,   215,
       3,     5,     3,    15,    16,    17,   263,     5,     3,   164,
     164,   164,   164,   210,   209,   263,     3,     3,   164,   207,
     164,   208,   203,   219,   164,     3,   220,   164,   175,    17,
     263,   260,   173,   174,   172,   263,   189,   188,   164,   211,
     263,   263,   185,     3,   194,   192,   186,     0,     3,   238,
       3,   164,   233,   235,   237,   259,    11,     3,   232,   259,
      11,   232,   165,    11,    11,    11,    11,   254,   255,   254,
      11,   165,    11,    11,    11,    11,    11,     3,   165,    11,
      11,    11,     3,   165,    11,     4,   165,    11,   166,     7,
     257,     7,   258,   165,   258,     3,   234,   235,   236,   259,
     236,     3,     8,     9,   164,     3,     7,   164,   166,   259,
     258,   257,    10,     7,    10,     7,   257,   257,   258,   257,
     257,   257,   165,   165,   235,     3,   242,   259,   165,   235,
     242,     3,   164,   245,   259,   246,   247,   248,   249,   250,
     246,   245,   245,   248,   165,   248,   248,    11,   165,   242,
     165,   165,   165,   236,   236,   164,   164,   164,    11,     3,
      11,   166,   165,     3,     3,    11,   166,   165,   165,   165,
      11,   232,   165,   165,   165,   251,   252,   251,   251,   251,
     165,   165,   165,   165,   165,   165,    11,   165,    11,   238,
     238,    11,   232,   168,     3,   259,   168,    11,    11,    11,
      11,     3,   243,   243,    11,   243,    11,   243,   248,     3,
     164,   256,    11,   256,    11,   233,     3,   259,   164,   243,
     244,   244,   244,   235,   240,   241,   240,    11,   232,   236,
     165,   242,   242,     3,   236,   253,   165,   165,    11,   166,
     165,   165,   166,   165,     3,   259,   165,   164,   165,     3,
     259,   165,     3,   259,   165,     3,   259,   165,     3,   165,
       3,   165,   165,     7,   165,   166,   165,   165,   165,    11,
     165,   165,    11,     3,   166,   168,   168,   168,   168,     3,
       3,   165,   166,     3,   165,    11,   166,   164,     3,   259,
       7,     7,     3,     3,     3,    11,    11,    11,   165,   165,
     166,     3,   168,   166,   167,   165,   166,     3,   168,     3,
       3,     3,   165,     3,     3,     3,   164,     3,     3,     6,
       6,     6,     5,   165,    10,   165,     3,   166,     3,   165,
     165,   165,   165,   165,   165,   165,    11,   165,   165,   166,
      11,     3,   239,   165,     7,   165,     3,     3,   165,     3,
       3,     3,     3,     3,   166,     3,   259,    11,     3,   259,
     166,     3,   259,   166,     3,   259,   166,     3,   259,     3,
     167,     7,     7,     3,     3,     8,     9,     3,     3,   165,
       8,     7,   166,   259,   165,   167,     3,    10,     7,    10,
       7,   165,   165,     3,     3,   165,     3,   165,     3,    11,
     165,   166,   165,   165,   165,   165,   165,   165,     3,     3,
      11,   166,     3,   165,   167,   169,     3,     3,   165,     3,
       3,   165,   167,   169,     3,   165,   165,   165,     3,   165,
     165,   165,    11,     5,     3,     3,     6,     6,     6,     6,
       3,     3,     3,     3,     3,     3,   165,   165,     3,     3,
       3,   167,   165,   165,   167,     3,   166,   165,   166,     3,
     166,     3,   166,     3,   166,    15,    16,    17,    19,    20,
     262,   165,   167,     8,     9,     3,   166,     8,   165,   167,
       6,     6,   167,     6,   165,     3,   167,   165,     3,     3,
       6,     6,     6,   166,   259,   166,   259,     3,   166,   259,
     166,     3,   168,   167,     3,   165,   167,   167,   167,     3,
     167,     3,   165,   167,   167,     6,     6,     6,     6,     6,
       6,   165,   167,   168,   167,     3,     3,   165,   265,     3,
       3,    19,   262,   167,     3,   164,     3,   167,     3,   167,
       3,   167,     3,   165,   165,     7,    19,   262,   167,     3,
       7,   165,    19,   262,     3,   265,     3,   167,     3,   166,
       3,   166,     3,   166,     3,   165,   167,   169,     3,   165,
     167,     3,   259,   165,   165,   165,   167,     3,   165,   165,
       6,     3,   165,     3,   165,   165,   165,    15,   266,   167,
      11,   167,   266,   167,   266,   167,   266,   167,     3,     3,
     165,   165,   167,     7,   165,   165,   165,   265,   167,     3,
     167,     3,   167,     3,   167,     3,   167,   167,     3,   259,
     165,   166,     3,     3,     3,   165,     3,     3,   165,     3,
       3,     3,     3,    15,   165,   266,   266,    15,    15,     3,
       3,     3,     3,     3,   165,   167,   167,   167,   167,   166,
       3,     3,     3,     3,   164,     3,     3,   167,    11,   167,
     165,     3,   165,     3
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int16 yyr1[] =
{
       0,   170,   171,   171,   171,   171,   171,   171,   171,   171,
     171,   171,   171,   171,   171,   171,   171,   171,   171,   171,
     171,   171,   171,   171,   171,   171,   171,   171,   171,   171,
     171,   171,   171,   171,   171,   171,   171,   171,   171,   171,
     171,   172,   171,   173,   171,   174,   171,   171,   171,   171,
     171,   171,   171,   171,   171,   171,   171,   171,   171,   171,
     171,   171,   171,   171,   171,   171,   171,   171,   171,   171,
     171,   171,   171,   171,   171,   171,   171,   171,   171,   171,
     171,   171,   171,   171,   171,   171,   171,   171,   171,   171,
     171,   175,   171,   176,   171,   177,   171,   178,   171,   179,
     171,   180,   171,   181,   171,   182,   171,   183,   171,   184,
     171,   185,   171,   186,   171,   187,   171,   188,   171,   189,
     171,   190,   171,   191,   171,   192,   171,   193,   171,   194,
     171,   171,   171,   171,   171,   171,   171,   171,   171,   195,
     171,   196,   171,   197,   171,   198,   171,   199,   171,   200,
     171,   201,   171,   202,   171,   203,   171,   171,   171,   171,
     171,   171,   171,   171,   171,   171,   171,   171,   171,   171,
     171,   171,   171,   171,   204,   171,   171,   205,   171,   171,
     206,   171,   171,   171,   171,   171,   171,   171,   171,   171,
     207,   171,   208,   171,   209,   171,   210,   171,   171,   171,
     171,   171,   171,   171,   171,   171,   171,   171,   211,   171,
     212,   171,   213,   171,   214,   171,   215,   171,   171,   171,
     171,   171,   171,   171,   171,   171,   171,   171,   171,   216,
     171,   171,   171,   171,   171,   217,   171,   218,   171,   219,
     171,   220,   171,   221,   171,   222,   171,   223,   171,   224,
     171,   225,   171,   226,   171,   227,   171,   228,   171,   229,
     171,   230,   171,   231,   171,   171,   171,   171,   171,   171,
     171,   171,   171,   171,   171,   171,   171,   171,   171,   171,
     171,   171,   171,   171,   171,   171,   171,   171,   171,   232,
     232,   232,   232,   233,   233,   234,   234,   234,   235,   236,
     236,   237,   237,   238,   238,   239,   241,   240,   240,   242,
     242,   242,   243,   244,   244,   244,   245,   245,   245,   245,
     247,   246,   246,   249,   248,   250,   248,   252,   251,   253,
     253,   255,   254,   256,   256,   257,   258,   259,   259,   261,
     260,   262,   262,   262,   262,   262,   263,   263,   263,   263,
     264,   264,   264,   265,   265,   266,   266
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     1,     1,     1,     1,     2,     3,     3,
       2,     3,     3,     3,     3,     3,     3,     3,     2,     8,
       8,     8,     9,     9,     9,     7,     4,     8,     8,     5,
       7,     8,     5,     5,     5,     5,     5,     5,     6,     5,
       3,     0,     3,     0,     3,     0,     3,     4,     4,     7,
       3,     5,     5,     5,     2,     2,     2,     3,     2,     2,
       2,     2,     2,     2,     2,     3,     3,     1,     1,     1,
       1,     2,     2,     2,     2,     1,     1,     1,     1,     3,
       8,     8,     7,    10,    11,     5,     7,     9,     9,     9,
       6,     0,     3,     0,     3,     0,     3,     0,     3,     0,
       3,     0,     3,     0,     3,     0,     3,     0,     3,     0,
       3,     0,     3,     0,     3,     0,     3,     0,     3,     0,
       3,     0,     3,     0,     3,     0,     3,     0,     3,     0,
       3,     4,     4,     4,     4,     8,     8,     8,     8,     0,
       3,     0,     3,     0,     3,     0,     3,     0,     3,     0,
       3,     0,     3,     0,     3,     0,     3,     3,     6,     9,
       9,     4,     6,     4,     6,     4,     6,     4,     6,     2,
       4,     2,     4,     2,     0,     3,     2,     0,     3,     2,
       0,     3,     3,     5,     8,     8,     8,     8,     8,     8,
       0,     3,     0,     3,     0,     3,     0,     3,     4,     4,
       5,     5,     5,     5,     5,     9,     9,     9,     0,     3,
       0,     3,     0,     3,     0,     3,     0,     3,     5,     6,
       6,     6,     6,     6,     6,     6,     6,     6,     6,     0,
       3,     4,     5,     5,     5,     0,     3,     0,     3,     0,
       3,     0,     3,     0,     3,     0,     3,     0,     3,     0,
       3,     0,     3,     0,     3,     0,     3,     0,     3,     0,
       3,     0,     3,     0,     3,     4,     5,     5,     5,     5,
       5,     5,     7,     8,     7,     8,     6,     6,     6,     5,
       5,     5,     5,     4,     1,     4,     4,     4,     4,     3,
       7,     7,     5,     3,     7,     3,     7,     7,     4,     1,
       1,     1,     1,     3,     1,     3,     0,     2,     1,     3,
       7,     7,     3,     4,     6,     1,     4,     3,     7,     5,
       0,     5,     1,     0,     4,     0,     8,     0,     7,     1,
       5,     0,    13,     2,     1,     3,     5,     0,     1,     0,
       2,     1,     1,     0,     1,     1,     0,     1,     1,     1,
       0,     1,     1,     0,     1,     0,     1
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* statement: UNKNOWN_OPCODE  */
#line 185 "./config/rx-parse.y"
          { as_bad (_("Unknown opcode: %s"), rx_init_start); }
#line 2450 "config/rx-parse.c"
    break;

  case 3: /* statement: BRK  */
#line 190 "./config/rx-parse.y"
          { B1 (0x00); }
#line 2456 "config/rx-parse.c"
    break;

  case 4: /* statement: DBT  */
#line 193 "./config/rx-parse.y"
          { B1 (0x01); }
#line 2462 "config/rx-parse.c"
    break;

  case 5: /* statement: RTS  */
#line 196 "./config/rx-parse.y"
          { B1 (0x02); }
#line 2468 "config/rx-parse.c"
    break;

  case 6: /* statement: NOP  */
#line 199 "./config/rx-parse.y"
          { B1 (0x03); }
#line 2474 "config/rx-parse.c"
    break;

  case 7: /* statement: BRA EXPR  */
#line 204 "./config/rx-parse.y"
          { if (rx_disp3op ((yyvsp[0].exp)))
	      { B1 (0x08); rx_disp3 ((yyvsp[0].exp), 5); }
	    else if (rx_intop ((yyvsp[0].exp), 8, 8))
	      { B1 (0x2e); PC1 ((yyvsp[0].exp)); }
	    else if (rx_intop ((yyvsp[0].exp), 16, 16))
	      { B1 (0x38); PC2 ((yyvsp[0].exp)); }
	    else if (rx_intop ((yyvsp[0].exp), 24, 24))
	      { B1 (0x04); PC3 ((yyvsp[0].exp)); }
	    else
	      { rx_relax (RX_RELAX_BRANCH, 0);
		rx_linkrelax_branch ();
		/* We'll convert this to a longer one later if needed.  */
		B1 (0x08); rx_disp3 ((yyvsp[0].exp), 5); } }
#line 2492 "config/rx-parse.c"
    break;

  case 8: /* statement: BRA DOT_A EXPR  */
#line 219 "./config/rx-parse.y"
          { B1 (0x04); PC3 ((yyvsp[0].exp)); }
#line 2498 "config/rx-parse.c"
    break;

  case 9: /* statement: BRA DOT_S EXPR  */
#line 222 "./config/rx-parse.y"
          { B1 (0x08); rx_disp3 ((yyvsp[0].exp), 5); }
#line 2504 "config/rx-parse.c"
    break;

  case 10: /* statement: BSR EXPR  */
#line 227 "./config/rx-parse.y"
          { if (rx_intop ((yyvsp[0].exp), 16, 16))
	      { B1 (0x39); PC2 ((yyvsp[0].exp)); }
	    else if (rx_intop ((yyvsp[0].exp), 24, 24))
	      { B1 (0x05); PC3 ((yyvsp[0].exp)); }
	    else
	      { rx_relax (RX_RELAX_BRANCH, 0);
		rx_linkrelax_branch ();
		B1 (0x39); PC2 ((yyvsp[0].exp)); } }
#line 2517 "config/rx-parse.c"
    break;

  case 11: /* statement: BSR DOT_A EXPR  */
#line 236 "./config/rx-parse.y"
          { B1 (0x05), PC3 ((yyvsp[0].exp)); }
#line 2523 "config/rx-parse.c"
    break;

  case 12: /* statement: BCND DOT_S EXPR  */
#line 241 "./config/rx-parse.y"
          { if ((yyvsp[-2].regno) == COND_EQ || (yyvsp[-2].regno) == COND_NE)
	      { B1 ((yyvsp[-2].regno) == COND_EQ ? 0x10 : 0x18); rx_disp3 ((yyvsp[0].exp), 5); }
	    else
	      as_bad (_("Only BEQ and BNE may have .S")); }
#line 2532 "config/rx-parse.c"
    break;

  case 13: /* statement: BCND DOT_B EXPR  */
#line 249 "./config/rx-parse.y"
          { B1 (0x20); F ((yyvsp[-2].regno), 4, 4); PC1 ((yyvsp[0].exp)); }
#line 2538 "config/rx-parse.c"
    break;

  case 14: /* statement: BRA DOT_B EXPR  */
#line 252 "./config/rx-parse.y"
          { B1 (0x2e), PC1 ((yyvsp[0].exp)); }
#line 2544 "config/rx-parse.c"
    break;

  case 15: /* statement: BRA DOT_W EXPR  */
#line 257 "./config/rx-parse.y"
          { B1 (0x38), PC2 ((yyvsp[0].exp)); }
#line 2550 "config/rx-parse.c"
    break;

  case 16: /* statement: BSR DOT_W EXPR  */
#line 259 "./config/rx-parse.y"
          { B1 (0x39), PC2 ((yyvsp[0].exp)); }
#line 2556 "config/rx-parse.c"
    break;

  case 17: /* statement: BCND DOT_W EXPR  */
#line 261 "./config/rx-parse.y"
          { if ((yyvsp[-2].regno) == COND_EQ || (yyvsp[-2].regno) == COND_NE)
	      { B1 ((yyvsp[-2].regno) == COND_EQ ? 0x3a : 0x3b); PC2 ((yyvsp[0].exp)); }
	    else
	      as_bad (_("Only BEQ and BNE may have .W")); }
#line 2565 "config/rx-parse.c"
    break;

  case 18: /* statement: BCND EXPR  */
#line 266 "./config/rx-parse.y"
          { if ((yyvsp[-1].regno) == COND_EQ || (yyvsp[-1].regno) == COND_NE)
	      {
		rx_relax (RX_RELAX_BRANCH, 0);
		rx_linkrelax_branch ();
		B1 ((yyvsp[-1].regno) == COND_EQ ? 0x10 : 0x18); rx_disp3 ((yyvsp[0].exp), 5);
	      }
	    else
	      {
		rx_relax (RX_RELAX_BRANCH, 0);
		/* This is because we might turn it into a
		   jump-over-jump long branch.  */
		rx_linkrelax_branch ();
	        B1 (0x20); F ((yyvsp[-1].regno), 4, 4); PC1 ((yyvsp[0].exp));
	      } }
#line 2584 "config/rx-parse.c"
    break;

  case 19: /* statement: MOV DOT_B '#' EXPR ',' '[' REG ']'  */
#line 284 "./config/rx-parse.y"
          { B2 (0xf8, 0x04); F ((yyvsp[-1].regno), 8, 4); IMMB ((yyvsp[-4].exp), 12);}
#line 2590 "config/rx-parse.c"
    break;

  case 20: /* statement: MOV DOT_W '#' EXPR ',' '[' REG ']'  */
#line 287 "./config/rx-parse.y"
          { B2 (0xf8, 0x01); F ((yyvsp[-1].regno), 8, 4); IMMW ((yyvsp[-4].exp), 12);}
#line 2596 "config/rx-parse.c"
    break;

  case 21: /* statement: MOV DOT_L '#' EXPR ',' '[' REG ']'  */
#line 290 "./config/rx-parse.y"
          { B2 (0xf8, 0x02); F ((yyvsp[-1].regno), 8, 4); IMM ((yyvsp[-4].exp), 12);}
#line 2602 "config/rx-parse.c"
    break;

  case 22: /* statement: MOV DOT_B '#' EXPR ',' disp '[' REG ']'  */
#line 294 "./config/rx-parse.y"
          { if ((yyvsp[-1].regno) <= 7 && rx_uintop ((yyvsp[-5].exp), 8) && rx_disp5op0 (&(yyvsp[-3].exp), BSIZE))
	      { B2 (0x3c, 0); rx_field5s2 ((yyvsp[-3].exp)); F ((yyvsp[-1].regno), 9, 3); O1 ((yyvsp[-5].exp)); }
	    else
	      { B2 (0xf8, 0x04); F ((yyvsp[-1].regno), 8, 4); DSP ((yyvsp[-3].exp), 6, BSIZE); O1 ((yyvsp[-5].exp));
	      if ((yyvsp[-5].exp).X_op != O_constant && (yyvsp[-5].exp).X_op != O_big) rx_linkrelax_imm (12); } }
#line 2612 "config/rx-parse.c"
    break;

  case 23: /* statement: MOV DOT_W '#' EXPR ',' disp '[' REG ']'  */
#line 301 "./config/rx-parse.y"
          { if ((yyvsp[-1].regno) <= 7 && rx_uintop ((yyvsp[-5].exp), 8) && rx_disp5op0 (&(yyvsp[-3].exp), WSIZE))
	      { B2 (0x3d, 0); rx_field5s2 ((yyvsp[-3].exp)); F ((yyvsp[-1].regno), 9, 3); O1 ((yyvsp[-5].exp)); }
	    else
	      { B2 (0xf8, 0x01); F ((yyvsp[-1].regno), 8, 4); DSP ((yyvsp[-3].exp), 6, WSIZE); IMMW ((yyvsp[-5].exp), 12); } }
#line 2621 "config/rx-parse.c"
    break;

  case 24: /* statement: MOV DOT_L '#' EXPR ',' disp '[' REG ']'  */
#line 307 "./config/rx-parse.y"
          { if ((yyvsp[-1].regno) <= 7 && rx_uintop ((yyvsp[-5].exp), 8) && rx_disp5op0 (&(yyvsp[-3].exp), LSIZE))
	      { B2 (0x3e, 0); rx_field5s2 ((yyvsp[-3].exp)); F ((yyvsp[-1].regno), 9, 3); O1 ((yyvsp[-5].exp)); }
	    else
	      { B2 (0xf8, 0x02); F ((yyvsp[-1].regno), 8, 4); DSP ((yyvsp[-3].exp), 6, LSIZE); IMM ((yyvsp[-5].exp), 12); } }
#line 2630 "config/rx-parse.c"
    break;

  case 25: /* statement: RTSD '#' EXPR ',' REG '-' REG  */
#line 315 "./config/rx-parse.y"
          { B2 (0x3f, 0); F ((yyvsp[-2].regno), 8, 4); F ((yyvsp[0].regno), 12, 4); rtsd_immediate ((yyvsp[-4].exp));
	    if ((yyvsp[-2].regno) == 0)
	      rx_error (_("RTSD cannot pop R0"));
	    if ((yyvsp[-2].regno) > (yyvsp[0].regno))
	      rx_error (_("RTSD first reg must be <= second reg")); }
#line 2640 "config/rx-parse.c"
    break;

  case 26: /* statement: CMP REG ',' REG  */
#line 324 "./config/rx-parse.y"
          { B2 (0x47, 0); F ((yyvsp[-2].regno), 8, 4); F ((yyvsp[0].regno), 12, 4); }
#line 2646 "config/rx-parse.c"
    break;

  case 27: /* statement: CMP disp '[' REG ']' DOT_UB ',' REG  */
#line 329 "./config/rx-parse.y"
          { B2 (0x44, 0); F ((yyvsp[-4].regno), 8, 4); F ((yyvsp[0].regno), 12, 4); DSP ((yyvsp[-6].exp), 6, BSIZE); }
#line 2652 "config/rx-parse.c"
    break;

  case 28: /* statement: CMP disp '[' REG ']' memex ',' REG  */
#line 332 "./config/rx-parse.y"
          { B3 (MEMEX, 0x04, 0); F ((yyvsp[-2].regno), 8, 2);  F ((yyvsp[-4].regno), 16, 4); F ((yyvsp[0].regno), 20, 4); DSP ((yyvsp[-6].exp), 14, sizemap[(yyvsp[-2].regno)]); }
#line 2658 "config/rx-parse.c"
    break;

  case 29: /* statement: MOVU bw REG ',' REG  */
#line 337 "./config/rx-parse.y"
          { B2 (0x5b, 0x00); F ((yyvsp[-3].regno), 5, 1); F ((yyvsp[-2].regno), 8, 4); F ((yyvsp[0].regno), 12, 4); }
#line 2664 "config/rx-parse.c"
    break;

  case 30: /* statement: MOVU bw '[' REG ']' ',' REG  */
#line 342 "./config/rx-parse.y"
          { B2 (0x58, 0x00); F ((yyvsp[-5].regno), 5, 1); F ((yyvsp[-3].regno), 8, 4); F ((yyvsp[0].regno), 12, 4); }
#line 2670 "config/rx-parse.c"
    break;

  case 31: /* statement: MOVU bw EXPR '[' REG ']' ',' REG  */
#line 345 "./config/rx-parse.y"
          { if ((yyvsp[-3].regno) <= 7 && (yyvsp[0].regno) <= 7 && rx_disp5op (&(yyvsp[-5].exp), (yyvsp[-6].regno)))
	      { B2 (0xb0, 0); F ((yyvsp[-6].regno), 4, 1); F ((yyvsp[-3].regno), 9, 3); F ((yyvsp[0].regno), 13, 3); rx_field5s ((yyvsp[-5].exp)); }
	    else
	      { B2 (0x58, 0x00); F ((yyvsp[-6].regno), 5, 1); F ((yyvsp[-3].regno), 8, 4); F ((yyvsp[0].regno), 12, 4); DSP ((yyvsp[-5].exp), 6, (yyvsp[-6].regno)); } }
#line 2679 "config/rx-parse.c"
    break;

  case 32: /* statement: SUB '#' EXPR ',' REG  */
#line 353 "./config/rx-parse.y"
          { if (rx_uintop ((yyvsp[-2].exp), 4))
	      { B2 (0x60, 0); FE ((yyvsp[-2].exp), 8, 4); F ((yyvsp[0].regno), 12, 4); }
	    else
	      /* This is really an add, but we negate the immediate.  */
	      { B2 (0x70, 0); F ((yyvsp[0].regno), 8, 4); F ((yyvsp[0].regno), 12, 4); NIMM ((yyvsp[-2].exp), 6); } }
#line 2689 "config/rx-parse.c"
    break;

  case 33: /* statement: CMP '#' EXPR ',' REG  */
#line 360 "./config/rx-parse.y"
          { if (rx_uintop ((yyvsp[-2].exp), 4))
	      { B2 (0x61, 0); FE ((yyvsp[-2].exp), 8, 4); F ((yyvsp[0].regno), 12, 4); }
	    else if (rx_uintop ((yyvsp[-2].exp), 8))
	      { B2 (0x75, 0x50); F ((yyvsp[0].regno), 12, 4); UO1 ((yyvsp[-2].exp)); }
	    else
	      { B2 (0x74, 0x00); F ((yyvsp[0].regno), 12, 4); IMM ((yyvsp[-2].exp), 6); } }
#line 2700 "config/rx-parse.c"
    break;

  case 34: /* statement: ADD '#' EXPR ',' REG  */
#line 368 "./config/rx-parse.y"
          { if (rx_uintop ((yyvsp[-2].exp), 4))
	      { B2 (0x62, 0); FE ((yyvsp[-2].exp), 8, 4); F ((yyvsp[0].regno), 12, 4); }
	    else
	      { B2 (0x70, 0); F ((yyvsp[0].regno), 8, 4); F ((yyvsp[0].regno), 12, 4); IMM ((yyvsp[-2].exp), 6); } }
#line 2709 "config/rx-parse.c"
    break;

  case 35: /* statement: MUL '#' EXPR ',' REG  */
#line 374 "./config/rx-parse.y"
          { if (rx_uintop ((yyvsp[-2].exp), 4))
	      { B2 (0x63, 0); FE ((yyvsp[-2].exp), 8, 4); F ((yyvsp[0].regno), 12, 4); }
	    else
	      { B2 (0x74, 0x10); F ((yyvsp[0].regno), 12, 4); IMM ((yyvsp[-2].exp), 6); } }
#line 2718 "config/rx-parse.c"
    break;

  case 36: /* statement: AND_ '#' EXPR ',' REG  */
#line 380 "./config/rx-parse.y"
          { if (rx_uintop ((yyvsp[-2].exp), 4))
	      { B2 (0x64, 0); FE ((yyvsp[-2].exp), 8, 4); F ((yyvsp[0].regno), 12, 4); }
	    else
	      { B2 (0x74, 0x20); F ((yyvsp[0].regno), 12, 4); IMM ((yyvsp[-2].exp), 6); } }
#line 2727 "config/rx-parse.c"
    break;

  case 37: /* statement: OR '#' EXPR ',' REG  */
#line 386 "./config/rx-parse.y"
          { if (rx_uintop ((yyvsp[-2].exp), 4))
	      { B2 (0x65, 0); FE ((yyvsp[-2].exp), 8, 4); F ((yyvsp[0].regno), 12, 4); }
	    else
	      { B2 (0x74, 0x30); F ((yyvsp[0].regno), 12, 4); IMM ((yyvsp[-2].exp), 6); } }
#line 2736 "config/rx-parse.c"
    break;

  case 38: /* statement: MOV DOT_L '#' EXPR ',' REG  */
#line 392 "./config/rx-parse.y"
          { if (rx_uintop ((yyvsp[-2].exp), 4))
	      { B2 (0x66, 0); FE ((yyvsp[-2].exp), 8, 4); F ((yyvsp[0].regno), 12, 4); }
	    else if (rx_uintop ((yyvsp[-2].exp), 8))
	      { B2 (0x75, 0x40); F ((yyvsp[0].regno), 12, 4); UO1 ((yyvsp[-2].exp)); }
	    else
	      { B2 (0xfb, 0x02); F ((yyvsp[0].regno), 8, 4); IMM ((yyvsp[-2].exp), 12); } }
#line 2747 "config/rx-parse.c"
    break;

  case 39: /* statement: MOV '#' EXPR ',' REG  */
#line 400 "./config/rx-parse.y"
          { if (rx_uintop ((yyvsp[-2].exp), 4))
	      { B2 (0x66, 0); FE ((yyvsp[-2].exp), 8, 4); F ((yyvsp[0].regno), 12, 4); }
	    else if (rx_uintop ((yyvsp[-2].exp), 8))
	      { B2 (0x75, 0x40); F ((yyvsp[0].regno), 12, 4); UO1 ((yyvsp[-2].exp)); }
	    else
	      { B2 (0xfb, 0x02); F ((yyvsp[0].regno), 8, 4); IMM ((yyvsp[-2].exp), 12); } }
#line 2758 "config/rx-parse.c"
    break;

  case 40: /* statement: RTSD '#' EXPR  */
#line 410 "./config/rx-parse.y"
          { B1 (0x67); rtsd_immediate ((yyvsp[0].exp)); }
#line 2764 "config/rx-parse.c"
    break;

  case 41: /* $@1: %empty  */
#line 414 "./config/rx-parse.y"
               { sub_op = 0; }
#line 2770 "config/rx-parse.c"
    break;

  case 43: /* $@2: %empty  */
#line 415 "./config/rx-parse.y"
               { sub_op = 1; }
#line 2776 "config/rx-parse.c"
    break;

  case 45: /* $@3: %empty  */
#line 416 "./config/rx-parse.y"
               { sub_op = 2; }
#line 2782 "config/rx-parse.c"
    break;

  case 47: /* statement: PUSHM REG '-' REG  */
#line 421 "./config/rx-parse.y"
          {
	    if ((yyvsp[-2].regno) == (yyvsp[0].regno))
	      { B2 (0x7e, 0x80); F (LSIZE, 10, 2); F ((yyvsp[-2].regno), 12, 4); }
	    else
	     { B2 (0x6e, 0); F ((yyvsp[-2].regno), 8, 4); F ((yyvsp[0].regno), 12, 4); }
	    if ((yyvsp[-2].regno) == 0)
	      rx_error (_("PUSHM cannot push R0"));
	    if ((yyvsp[-2].regno) > (yyvsp[0].regno))
	      rx_error (_("PUSHM first reg must be <= second reg")); }
#line 2796 "config/rx-parse.c"
    break;

  case 48: /* statement: POPM REG '-' REG  */
#line 434 "./config/rx-parse.y"
          {
	    if ((yyvsp[-2].regno) == (yyvsp[0].regno))
	      { B2 (0x7e, 0xb0); F ((yyvsp[-2].regno), 12, 4); }
	    else
	      { B2 (0x6f, 0); F ((yyvsp[-2].regno), 8, 4); F ((yyvsp[0].regno), 12, 4); }
	    if ((yyvsp[-2].regno) == 0)
	      rx_error (_("POPM cannot pop R0"));
	    if ((yyvsp[-2].regno) > (yyvsp[0].regno))
	      rx_error (_("POPM first reg must be <= second reg")); }
#line 2810 "config/rx-parse.c"
    break;

  case 49: /* statement: ADD '#' EXPR ',' REG ',' REG  */
#line 447 "./config/rx-parse.y"
          { B2 (0x70, 0x00); F ((yyvsp[-2].regno), 8, 4); F ((yyvsp[0].regno), 12, 4); IMM ((yyvsp[-4].exp), 6); }
#line 2816 "config/rx-parse.c"
    break;

  case 50: /* statement: INT '#' EXPR  */
#line 452 "./config/rx-parse.y"
          { B2(0x75, 0x60), UO1 ((yyvsp[0].exp)); }
#line 2822 "config/rx-parse.c"
    break;

  case 51: /* statement: BSET '#' EXPR ',' REG  */
#line 457 "./config/rx-parse.y"
          { B2 (0x78, 0); FE ((yyvsp[-2].exp), 7, 5); F ((yyvsp[0].regno), 12, 4); }
#line 2828 "config/rx-parse.c"
    break;

  case 52: /* statement: BCLR '#' EXPR ',' REG  */
#line 459 "./config/rx-parse.y"
          { B2 (0x7a, 0); FE ((yyvsp[-2].exp), 7, 5); F ((yyvsp[0].regno), 12, 4); }
#line 2834 "config/rx-parse.c"
    break;

  case 53: /* statement: BTST '#' EXPR ',' REG  */
#line 464 "./config/rx-parse.y"
          { B2 (0x7c, 0x00); FE ((yyvsp[-2].exp), 7, 5); F ((yyvsp[0].regno), 12, 4); }
#line 2840 "config/rx-parse.c"
    break;

  case 54: /* statement: SAT REG  */
#line 469 "./config/rx-parse.y"
          { B2 (0x7e, 0x30); F ((yyvsp[0].regno), 12, 4); }
#line 2846 "config/rx-parse.c"
    break;

  case 55: /* statement: RORC REG  */
#line 471 "./config/rx-parse.y"
          { B2 (0x7e, 0x40); F ((yyvsp[0].regno), 12, 4); }
#line 2852 "config/rx-parse.c"
    break;

  case 56: /* statement: ROLC REG  */
#line 473 "./config/rx-parse.y"
          { B2 (0x7e, 0x50); F ((yyvsp[0].regno), 12, 4); }
#line 2858 "config/rx-parse.c"
    break;

  case 57: /* statement: PUSH bwl REG  */
#line 478 "./config/rx-parse.y"
          { B2 (0x7e, 0x80); F ((yyvsp[-1].regno), 10, 2); F ((yyvsp[0].regno), 12, 4); }
#line 2864 "config/rx-parse.c"
    break;

  case 58: /* statement: POP REG  */
#line 483 "./config/rx-parse.y"
          { B2 (0x7e, 0xb0); F ((yyvsp[0].regno), 12, 4); }
#line 2870 "config/rx-parse.c"
    break;

  case 59: /* statement: PUSHC CREG  */
#line 488 "./config/rx-parse.y"
          { if ((yyvsp[0].regno) == 13)
	      { rx_check_v2 (); }
	    if ((yyvsp[0].regno) < 16)
	      { B2 (0x7e, 0xc0); F ((yyvsp[0].regno), 12, 4); }
	    else
	      as_bad (_("PUSHC can only push the first 16 control registers")); }
#line 2881 "config/rx-parse.c"
    break;

  case 60: /* statement: POPC CREG  */
#line 498 "./config/rx-parse.y"
          { if ((yyvsp[0].regno) == 13)
	    { rx_check_v2 (); }
	    if ((yyvsp[0].regno) < 16)
	      { B2 (0x7e, 0xe0); F ((yyvsp[0].regno), 12, 4); }
	    else
	      as_bad (_("POPC can only pop the first 16 control registers")); }
#line 2892 "config/rx-parse.c"
    break;

  case 61: /* statement: SETPSW flag  */
#line 508 "./config/rx-parse.y"
          { B2 (0x7f, 0xa0); F ((yyvsp[0].regno), 12, 4); }
#line 2898 "config/rx-parse.c"
    break;

  case 62: /* statement: CLRPSW flag  */
#line 510 "./config/rx-parse.y"
          { B2 (0x7f, 0xb0); F ((yyvsp[0].regno), 12, 4); }
#line 2904 "config/rx-parse.c"
    break;

  case 63: /* statement: JMP REG  */
#line 515 "./config/rx-parse.y"
          { B2 (0x7f, 0x00); F ((yyvsp[0].regno), 12, 4); }
#line 2910 "config/rx-parse.c"
    break;

  case 64: /* statement: JSR REG  */
#line 517 "./config/rx-parse.y"
          { B2 (0x7f, 0x10); F ((yyvsp[0].regno), 12, 4); }
#line 2916 "config/rx-parse.c"
    break;

  case 65: /* statement: BRA opt_l REG  */
#line 519 "./config/rx-parse.y"
          { B2 (0x7f, 0x40); F ((yyvsp[0].regno), 12, 4); }
#line 2922 "config/rx-parse.c"
    break;

  case 66: /* statement: BSR opt_l REG  */
#line 521 "./config/rx-parse.y"
          { B2 (0x7f, 0x50); F ((yyvsp[0].regno), 12, 4); }
#line 2928 "config/rx-parse.c"
    break;

  case 67: /* statement: SCMPU  */
#line 526 "./config/rx-parse.y"
          { B2 (0x7f, 0x83); rx_note_string_insn_use (); }
#line 2934 "config/rx-parse.c"
    break;

  case 68: /* statement: SMOVU  */
#line 528 "./config/rx-parse.y"
          { B2 (0x7f, 0x87); rx_note_string_insn_use (); }
#line 2940 "config/rx-parse.c"
    break;

  case 69: /* statement: SMOVB  */
#line 530 "./config/rx-parse.y"
          { B2 (0x7f, 0x8b); rx_note_string_insn_use (); }
#line 2946 "config/rx-parse.c"
    break;

  case 70: /* statement: SMOVF  */
#line 532 "./config/rx-parse.y"
          { B2 (0x7f, 0x8f); rx_note_string_insn_use (); }
#line 2952 "config/rx-parse.c"
    break;

  case 71: /* statement: SUNTIL bwl  */
#line 537 "./config/rx-parse.y"
          { B2 (0x7f, 0x80); F ((yyvsp[0].regno), 14, 2); rx_note_string_insn_use (); }
#line 2958 "config/rx-parse.c"
    break;

  case 72: /* statement: SWHILE bwl  */
#line 539 "./config/rx-parse.y"
          { B2 (0x7f, 0x84); F ((yyvsp[0].regno), 14, 2); rx_note_string_insn_use (); }
#line 2964 "config/rx-parse.c"
    break;

  case 73: /* statement: SSTR bwl  */
#line 541 "./config/rx-parse.y"
          { B2 (0x7f, 0x88); F ((yyvsp[0].regno), 14, 2); }
#line 2970 "config/rx-parse.c"
    break;

  case 74: /* statement: RMPA bwl  */
#line 546 "./config/rx-parse.y"
          { B2 (0x7f, 0x8c); F ((yyvsp[0].regno), 14, 2); rx_note_string_insn_use (); }
#line 2976 "config/rx-parse.c"
    break;

  case 75: /* statement: RTFI  */
#line 551 "./config/rx-parse.y"
          { B2 (0x7f, 0x94); }
#line 2982 "config/rx-parse.c"
    break;

  case 76: /* statement: RTE  */
#line 553 "./config/rx-parse.y"
          { B2 (0x7f, 0x95); }
#line 2988 "config/rx-parse.c"
    break;

  case 77: /* statement: WAIT  */
#line 555 "./config/rx-parse.y"
          { B2 (0x7f, 0x96); }
#line 2994 "config/rx-parse.c"
    break;

  case 78: /* statement: SATR  */
#line 557 "./config/rx-parse.y"
          { B2 (0x7f, 0x93); }
#line 3000 "config/rx-parse.c"
    break;

  case 79: /* statement: MVTIPL '#' EXPR  */
#line 562 "./config/rx-parse.y"
          { B3 (0x75, 0x70, 0x00); FE ((yyvsp[0].exp), 20, 4); }
#line 3006 "config/rx-parse.c"
    break;

  case 80: /* statement: MOV bwl REG ',' EXPR '[' REG ']'  */
#line 568 "./config/rx-parse.y"
          { if ((yyvsp[-5].regno) <= 7 && (yyvsp[-1].regno) <= 7 && rx_disp5op (&(yyvsp[-3].exp), (yyvsp[-6].regno)))
	      { B2 (0x80, 0); F ((yyvsp[-6].regno), 2, 2); F ((yyvsp[-1].regno), 9, 3); F ((yyvsp[-5].regno), 13, 3); rx_field5s ((yyvsp[-3].exp)); }
	    else
	      { B2 (0xc3, 0x00); F ((yyvsp[-6].regno), 2, 2); F ((yyvsp[-1].regno), 8, 4); F ((yyvsp[-5].regno), 12, 4); DSP ((yyvsp[-3].exp), 4, (yyvsp[-6].regno)); }}
#line 3015 "config/rx-parse.c"
    break;

  case 81: /* statement: MOV bwl EXPR '[' REG ']' ',' REG  */
#line 576 "./config/rx-parse.y"
          { if ((yyvsp[-3].regno) <= 7 && (yyvsp[0].regno) <= 7 && rx_disp5op (&(yyvsp[-5].exp), (yyvsp[-6].regno)))
	      { B2 (0x88, 0); F ((yyvsp[-6].regno), 2, 2); F ((yyvsp[-3].regno), 9, 3); F ((yyvsp[0].regno), 13, 3); rx_field5s ((yyvsp[-5].exp)); }
	    else
	      { B2 (0xcc, 0x00); F ((yyvsp[-6].regno), 2, 2); F ((yyvsp[-3].regno), 8, 4); F ((yyvsp[0].regno), 12, 4); DSP ((yyvsp[-5].exp), 6, (yyvsp[-6].regno)); } }
#line 3024 "config/rx-parse.c"
    break;

  case 82: /* statement: MOV bwl REG ',' '[' REG ']'  */
#line 590 "./config/rx-parse.y"
          { B2 (0xc3, 0x00); F ((yyvsp[-5].regno), 2, 2); F ((yyvsp[-1].regno), 8, 4); F ((yyvsp[-4].regno), 12, 4); }
#line 3030 "config/rx-parse.c"
    break;

  case 83: /* statement: MOV bwl '[' REG ']' ',' disp '[' REG ']'  */
#line 595 "./config/rx-parse.y"
          { B2 (0xc0, 0); F ((yyvsp[-8].regno), 2, 2); F ((yyvsp[-6].regno), 8, 4); F ((yyvsp[-1].regno), 12, 4); DSP ((yyvsp[-3].exp), 4, (yyvsp[-8].regno)); }
#line 3036 "config/rx-parse.c"
    break;

  case 84: /* statement: MOV bwl EXPR '[' REG ']' ',' disp '[' REG ']'  */
#line 600 "./config/rx-parse.y"
          { B2 (0xc0, 0x00); F ((yyvsp[-9].regno), 2, 2); F ((yyvsp[-6].regno), 8, 4); F ((yyvsp[-1].regno), 12, 4); DSP ((yyvsp[-8].exp), 6, (yyvsp[-9].regno)); DSP ((yyvsp[-3].exp), 4, (yyvsp[-9].regno)); }
#line 3042 "config/rx-parse.c"
    break;

  case 85: /* statement: MOV bwl REG ',' REG  */
#line 605 "./config/rx-parse.y"
          { B2 (0xcf, 0x00); F ((yyvsp[-3].regno), 2, 2); F ((yyvsp[-2].regno), 8, 4); F ((yyvsp[0].regno), 12, 4); }
#line 3048 "config/rx-parse.c"
    break;

  case 86: /* statement: MOV bwl '[' REG ']' ',' REG  */
#line 610 "./config/rx-parse.y"
          { B2 (0xcc, 0x00); F ((yyvsp[-5].regno), 2, 2); F ((yyvsp[-3].regno), 8, 4); F ((yyvsp[0].regno), 12, 4); }
#line 3054 "config/rx-parse.c"
    break;

  case 87: /* statement: BSET '#' EXPR ',' disp '[' REG ']' DOT_B  */
#line 615 "./config/rx-parse.y"
          { B2 (0xf0, 0x00); F ((yyvsp[-2].regno), 8, 4); FE ((yyvsp[-6].exp), 13, 3); DSP ((yyvsp[-4].exp), 6, BSIZE); }
#line 3060 "config/rx-parse.c"
    break;

  case 88: /* statement: BCLR '#' EXPR ',' disp '[' REG ']' DOT_B  */
#line 617 "./config/rx-parse.y"
          { B2 (0xf0, 0x08); F ((yyvsp[-2].regno), 8, 4); FE ((yyvsp[-6].exp), 13, 3); DSP ((yyvsp[-4].exp), 6, BSIZE); }
#line 3066 "config/rx-parse.c"
    break;

  case 89: /* statement: BTST '#' EXPR ',' disp '[' REG ']' DOT_B  */
#line 619 "./config/rx-parse.y"
          { B2 (0xf4, 0x00); F ((yyvsp[-2].regno), 8, 4); FE ((yyvsp[-6].exp), 13, 3); DSP ((yyvsp[-4].exp), 6, BSIZE); }
#line 3072 "config/rx-parse.c"
    break;

  case 90: /* statement: PUSH bwl disp '[' REG ']'  */
#line 624 "./config/rx-parse.y"
          { B2 (0xf4, 0x08); F ((yyvsp[-4].regno), 14, 2); F ((yyvsp[-1].regno), 8, 4); DSP ((yyvsp[-3].exp), 6, (yyvsp[-4].regno)); }
#line 3078 "config/rx-parse.c"
    break;

  case 91: /* $@4: %empty  */
#line 628 "./config/rx-parse.y"
                { sub_op = 0; }
#line 3084 "config/rx-parse.c"
    break;

  case 93: /* $@5: %empty  */
#line 629 "./config/rx-parse.y"
                { sub_op = 1; sub_op2 = 1; }
#line 3090 "config/rx-parse.c"
    break;

  case 95: /* $@6: %empty  */
#line 630 "./config/rx-parse.y"
                { sub_op = 2; }
#line 3096 "config/rx-parse.c"
    break;

  case 97: /* $@7: %empty  */
#line 631 "./config/rx-parse.y"
                { sub_op = 3; sub_op2 = 2; }
#line 3102 "config/rx-parse.c"
    break;

  case 99: /* $@8: %empty  */
#line 632 "./config/rx-parse.y"
                { sub_op = 4; }
#line 3108 "config/rx-parse.c"
    break;

  case 101: /* $@9: %empty  */
#line 633 "./config/rx-parse.y"
                { sub_op = 5; }
#line 3114 "config/rx-parse.c"
    break;

  case 103: /* $@10: %empty  */
#line 634 "./config/rx-parse.y"
                { sub_op = 6; }
#line 3120 "config/rx-parse.c"
    break;

  case 105: /* $@11: %empty  */
#line 635 "./config/rx-parse.y"
                { sub_op = 7; }
#line 3126 "config/rx-parse.c"
    break;

  case 107: /* $@12: %empty  */
#line 636 "./config/rx-parse.y"
                { sub_op = 8; }
#line 3132 "config/rx-parse.c"
    break;

  case 109: /* $@13: %empty  */
#line 637 "./config/rx-parse.y"
                { sub_op = 9; }
#line 3138 "config/rx-parse.c"
    break;

  case 111: /* $@14: %empty  */
#line 638 "./config/rx-parse.y"
                { sub_op = 12; }
#line 3144 "config/rx-parse.c"
    break;

  case 113: /* $@15: %empty  */
#line 639 "./config/rx-parse.y"
                { sub_op = 13; }
#line 3150 "config/rx-parse.c"
    break;

  case 115: /* $@16: %empty  */
#line 640 "./config/rx-parse.y"
                { sub_op = 14; sub_op2 = 0; }
#line 3156 "config/rx-parse.c"
    break;

  case 117: /* $@17: %empty  */
#line 641 "./config/rx-parse.y"
                { sub_op = 14; sub_op2 = 0; }
#line 3162 "config/rx-parse.c"
    break;

  case 119: /* $@18: %empty  */
#line 642 "./config/rx-parse.y"
                { sub_op = 15; sub_op2 = 1; }
#line 3168 "config/rx-parse.c"
    break;

  case 121: /* $@19: %empty  */
#line 646 "./config/rx-parse.y"
                { sub_op = 6; }
#line 3174 "config/rx-parse.c"
    break;

  case 123: /* $@20: %empty  */
#line 647 "./config/rx-parse.y"
                { sub_op = 7; }
#line 3180 "config/rx-parse.c"
    break;

  case 125: /* $@21: %empty  */
#line 648 "./config/rx-parse.y"
                { sub_op = 16; }
#line 3186 "config/rx-parse.c"
    break;

  case 127: /* $@22: %empty  */
#line 649 "./config/rx-parse.y"
                { sub_op = 17; }
#line 3192 "config/rx-parse.c"
    break;

  case 129: /* $@23: %empty  */
#line 650 "./config/rx-parse.y"
                { sub_op = 21; }
#line 3198 "config/rx-parse.c"
    break;

  case 131: /* statement: BSET REG ',' REG  */
#line 655 "./config/rx-parse.y"
          { id24 (1, 0x63, 0x00); F ((yyvsp[0].regno), 16, 4); F ((yyvsp[-2].regno), 20, 4); }
#line 3204 "config/rx-parse.c"
    break;

  case 132: /* statement: BCLR REG ',' REG  */
#line 657 "./config/rx-parse.y"
          { id24 (1, 0x67, 0x00); F ((yyvsp[0].regno), 16, 4); F ((yyvsp[-2].regno), 20, 4); }
#line 3210 "config/rx-parse.c"
    break;

  case 133: /* statement: BTST REG ',' REG  */
#line 659 "./config/rx-parse.y"
          { id24 (1, 0x6b, 0x00); F ((yyvsp[0].regno), 16, 4); F ((yyvsp[-2].regno), 20, 4); }
#line 3216 "config/rx-parse.c"
    break;

  case 134: /* statement: BNOT REG ',' REG  */
#line 661 "./config/rx-parse.y"
          { id24 (1, 0x6f, 0x00); F ((yyvsp[0].regno), 16, 4); F ((yyvsp[-2].regno), 20, 4); }
#line 3222 "config/rx-parse.c"
    break;

  case 135: /* statement: BSET REG ',' disp '[' REG ']' opt_b  */
#line 664 "./config/rx-parse.y"
          { id24 (1, 0x60, 0x00); F ((yyvsp[-2].regno), 16, 4); F ((yyvsp[-6].regno), 20, 4); DSP ((yyvsp[-4].exp), 14, BSIZE); }
#line 3228 "config/rx-parse.c"
    break;

  case 136: /* statement: BCLR REG ',' disp '[' REG ']' opt_b  */
#line 666 "./config/rx-parse.y"
          { id24 (1, 0x64, 0x00); F ((yyvsp[-2].regno), 16, 4); F ((yyvsp[-6].regno), 20, 4); DSP ((yyvsp[-4].exp), 14, BSIZE); }
#line 3234 "config/rx-parse.c"
    break;

  case 137: /* statement: BTST REG ',' disp '[' REG ']' opt_b  */
#line 668 "./config/rx-parse.y"
          { id24 (1, 0x68, 0x00); F ((yyvsp[-2].regno), 16, 4); F ((yyvsp[-6].regno), 20, 4); DSP ((yyvsp[-4].exp), 14, BSIZE); }
#line 3240 "config/rx-parse.c"
    break;

  case 138: /* statement: BNOT REG ',' disp '[' REG ']' opt_b  */
#line 670 "./config/rx-parse.y"
          { id24 (1, 0x6c, 0x00); F ((yyvsp[-2].regno), 16, 4); F ((yyvsp[-6].regno), 20, 4); DSP ((yyvsp[-4].exp), 14, BSIZE); }
#line 3246 "config/rx-parse.c"
    break;

  case 139: /* $@24: %empty  */
#line 674 "./config/rx-parse.y"
                { sub_op = 0; }
#line 3252 "config/rx-parse.c"
    break;

  case 141: /* $@25: %empty  */
#line 675 "./config/rx-parse.y"
                { sub_op = 1; }
#line 3258 "config/rx-parse.c"
    break;

  case 143: /* $@26: %empty  */
#line 676 "./config/rx-parse.y"
                { sub_op = 2; }
#line 3264 "config/rx-parse.c"
    break;

  case 145: /* $@27: %empty  */
#line 677 "./config/rx-parse.y"
                { sub_op = 3; }
#line 3270 "config/rx-parse.c"
    break;

  case 147: /* $@28: %empty  */
#line 678 "./config/rx-parse.y"
                { sub_op = 4; }
#line 3276 "config/rx-parse.c"
    break;

  case 149: /* $@29: %empty  */
#line 679 "./config/rx-parse.y"
                { sub_op = 8; }
#line 3282 "config/rx-parse.c"
    break;

  case 151: /* $@30: %empty  */
#line 680 "./config/rx-parse.y"
                { sub_op = 5; }
#line 3288 "config/rx-parse.c"
    break;

  case 153: /* $@31: %empty  */
#line 681 "./config/rx-parse.y"
                { sub_op = 9; }
#line 3294 "config/rx-parse.c"
    break;

  case 155: /* $@32: %empty  */
#line 682 "./config/rx-parse.y"
                { sub_op = 6; }
#line 3300 "config/rx-parse.c"
    break;

  case 157: /* statement: SCCND DOT_L REG  */
#line 690 "./config/rx-parse.y"
          { id24 (1, 0xdb, 0x00); F ((yyvsp[-2].regno), 20, 4); F ((yyvsp[0].regno), 16, 4); }
#line 3306 "config/rx-parse.c"
    break;

  case 158: /* statement: SCCND bwl disp '[' REG ']'  */
#line 692 "./config/rx-parse.y"
          { id24 (1, 0xd0, 0x00); F ((yyvsp[-5].regno), 20, 4); F ((yyvsp[-4].regno), 12, 2); F ((yyvsp[-1].regno), 16, 4); DSP ((yyvsp[-3].exp), 14, (yyvsp[-4].regno)); }
#line 3312 "config/rx-parse.c"
    break;

  case 159: /* statement: BMCND '#' EXPR ',' disp '[' REG ']' opt_b  */
#line 697 "./config/rx-parse.y"
          { id24 (1, 0xe0, 0x00); F ((yyvsp[-8].regno), 20, 4); FE ((yyvsp[-6].exp), 11, 3);
	      F ((yyvsp[-2].regno), 16, 4); DSP ((yyvsp[-4].exp), 14, BSIZE); }
#line 3319 "config/rx-parse.c"
    break;

  case 160: /* statement: BNOT '#' EXPR ',' disp '[' REG ']' opt_b  */
#line 703 "./config/rx-parse.y"
          { id24 (1, 0xe0, 0x0f); FE ((yyvsp[-6].exp), 11, 3); F ((yyvsp[-2].regno), 16, 4);
	      DSP ((yyvsp[-4].exp), 14, BSIZE); }
#line 3326 "config/rx-parse.c"
    break;

  case 161: /* statement: MULHI REG ',' REG  */
#line 709 "./config/rx-parse.y"
          { id24 (2, 0x00, 0x00); F ((yyvsp[-2].regno), 16, 4); F ((yyvsp[0].regno), 20, 4); }
#line 3332 "config/rx-parse.c"
    break;

  case 162: /* statement: MULHI REG ',' REG ',' ACC  */
#line 711 "./config/rx-parse.y"
          { rx_check_v2 (); id24 (2, 0x00, 0x00); F ((yyvsp[-4].regno), 16, 4); F ((yyvsp[-2].regno), 20, 4); F ((yyvsp[0].regno), 12, 1); }
#line 3338 "config/rx-parse.c"
    break;

  case 163: /* statement: MULLO REG ',' REG  */
#line 713 "./config/rx-parse.y"
          { id24 (2, 0x01, 0x00); F ((yyvsp[-2].regno), 16, 4); F ((yyvsp[0].regno), 20, 4); }
#line 3344 "config/rx-parse.c"
    break;

  case 164: /* statement: MULLO REG ',' REG ',' ACC  */
#line 715 "./config/rx-parse.y"
          { rx_check_v2 (); id24 (2, 0x01, 0x00); F ((yyvsp[-4].regno), 16, 4); F ((yyvsp[-2].regno), 20, 4); F ((yyvsp[0].regno), 12, 1); }
#line 3350 "config/rx-parse.c"
    break;

  case 165: /* statement: MACHI REG ',' REG  */
#line 717 "./config/rx-parse.y"
          { id24 (2, 0x04, 0x00); F ((yyvsp[-2].regno), 16, 4); F ((yyvsp[0].regno), 20, 4); }
#line 3356 "config/rx-parse.c"
    break;

  case 166: /* statement: MACHI REG ',' REG ',' ACC  */
#line 719 "./config/rx-parse.y"
          { rx_check_v2 (); id24 (2, 0x04, 0x00); F ((yyvsp[-4].regno), 16, 4); F ((yyvsp[-2].regno), 20, 4); F ((yyvsp[0].regno), 12, 1); }
#line 3362 "config/rx-parse.c"
    break;

  case 167: /* statement: MACLO REG ',' REG  */
#line 721 "./config/rx-parse.y"
          { id24 (2, 0x05, 0x00); F ((yyvsp[-2].regno), 16, 4); F ((yyvsp[0].regno), 20, 4); }
#line 3368 "config/rx-parse.c"
    break;

  case 168: /* statement: MACLO REG ',' REG ',' ACC  */
#line 723 "./config/rx-parse.y"
          { rx_check_v2 (); id24 (2, 0x05, 0x00); F ((yyvsp[-4].regno), 16, 4); F ((yyvsp[-2].regno), 20, 4); F ((yyvsp[0].regno), 12, 1); }
#line 3374 "config/rx-parse.c"
    break;

  case 169: /* statement: MVTACHI REG  */
#line 729 "./config/rx-parse.y"
          { id24 (2, 0x17, 0x00); F ((yyvsp[0].regno), 20, 4); }
#line 3380 "config/rx-parse.c"
    break;

  case 170: /* statement: MVTACHI REG ',' ACC  */
#line 731 "./config/rx-parse.y"
          { rx_check_v2 (); id24 (2, 0x17, 0x00); F ((yyvsp[-2].regno), 20, 4); F ((yyvsp[0].regno), 16, 1); }
#line 3386 "config/rx-parse.c"
    break;

  case 171: /* statement: MVTACLO REG  */
#line 733 "./config/rx-parse.y"
          { id24 (2, 0x17, 0x10); F ((yyvsp[0].regno), 20, 4); }
#line 3392 "config/rx-parse.c"
    break;

  case 172: /* statement: MVTACLO REG ',' ACC  */
#line 735 "./config/rx-parse.y"
          { rx_check_v2 (); id24 (2, 0x17, 0x10); F ((yyvsp[-2].regno), 20, 4); F ((yyvsp[0].regno), 16, 1); }
#line 3398 "config/rx-parse.c"
    break;

  case 173: /* statement: MVFACHI REG  */
#line 737 "./config/rx-parse.y"
          { id24 (2, 0x1f, 0x00); F ((yyvsp[0].regno), 20, 4); }
#line 3404 "config/rx-parse.c"
    break;

  case 174: /* $@33: %empty  */
#line 738 "./config/rx-parse.y"
                  { sub_op = 0; }
#line 3410 "config/rx-parse.c"
    break;

  case 176: /* statement: MVFACMI REG  */
#line 740 "./config/rx-parse.y"
          { id24 (2, 0x1f, 0x20); F ((yyvsp[0].regno), 20, 4); }
#line 3416 "config/rx-parse.c"
    break;

  case 177: /* $@34: %empty  */
#line 741 "./config/rx-parse.y"
                  { sub_op = 2; }
#line 3422 "config/rx-parse.c"
    break;

  case 179: /* statement: MVFACLO REG  */
#line 743 "./config/rx-parse.y"
          { id24 (2, 0x1f, 0x10); F ((yyvsp[0].regno), 20, 4); }
#line 3428 "config/rx-parse.c"
    break;

  case 180: /* $@35: %empty  */
#line 744 "./config/rx-parse.y"
                  { sub_op = 1; }
#line 3434 "config/rx-parse.c"
    break;

  case 182: /* statement: RACW '#' EXPR  */
#line 746 "./config/rx-parse.y"
          { id24 (2, 0x18, 0x00);
	    if (rx_uintop ((yyvsp[0].exp), 4) && exp_val((yyvsp[0].exp)) == 1)
	      ;
	    else if (rx_uintop ((yyvsp[0].exp), 4) && exp_val((yyvsp[0].exp)) == 2)
	      F (1, 19, 1);
	    else
	      as_bad (_("RACW expects #1 or #2"));}
#line 3446 "config/rx-parse.c"
    break;

  case 183: /* statement: RACW '#' EXPR ',' ACC  */
#line 754 "./config/rx-parse.y"
            { rx_check_v2 (); id24 (2, 0x18, 0x00); F ((yyvsp[0].regno), 16, 1);
	    if (rx_uintop ((yyvsp[-2].exp), 4) && exp_val((yyvsp[-2].exp)) == 1)
	      ;
	    else if (rx_uintop ((yyvsp[-2].exp), 4) && exp_val((yyvsp[-2].exp)) == 2)
	      F (1, 19, 1);
	    else
	      as_bad (_("RACW expects #1 or #2"));}
#line 3458 "config/rx-parse.c"
    break;

  case 184: /* statement: MOV bwl REG ',' '[' REG '+' ']'  */
#line 765 "./config/rx-parse.y"
          { id24 (2, 0x20, 0); F ((yyvsp[-6].regno), 14, 2); F ((yyvsp[-2].regno), 16, 4); F ((yyvsp[-5].regno), 20, 4); }
#line 3464 "config/rx-parse.c"
    break;

  case 185: /* statement: MOV bwl REG ',' '[' '-' REG ']'  */
#line 767 "./config/rx-parse.y"
          { id24 (2, 0x24, 0); F ((yyvsp[-6].regno), 14, 2); F ((yyvsp[-1].regno), 16, 4); F ((yyvsp[-5].regno), 20, 4); }
#line 3470 "config/rx-parse.c"
    break;

  case 186: /* statement: MOV bwl '[' REG '+' ']' ',' REG  */
#line 772 "./config/rx-parse.y"
          { id24 (2, 0x28, 0); F ((yyvsp[-6].regno), 14, 2); F ((yyvsp[-4].regno), 16, 4); F ((yyvsp[0].regno), 20, 4); }
#line 3476 "config/rx-parse.c"
    break;

  case 187: /* statement: MOV bwl '[' '-' REG ']' ',' REG  */
#line 774 "./config/rx-parse.y"
          { id24 (2, 0x2c, 0); F ((yyvsp[-6].regno), 14, 2); F ((yyvsp[-3].regno), 16, 4); F ((yyvsp[0].regno), 20, 4); }
#line 3482 "config/rx-parse.c"
    break;

  case 188: /* statement: MOVU bw '[' REG '+' ']' ',' REG  */
#line 779 "./config/rx-parse.y"
          { id24 (2, 0x38, 0); F ((yyvsp[-6].regno), 15, 1); F ((yyvsp[-4].regno), 16, 4); F ((yyvsp[0].regno), 20, 4); }
#line 3488 "config/rx-parse.c"
    break;

  case 189: /* statement: MOVU bw '[' '-' REG ']' ',' REG  */
#line 781 "./config/rx-parse.y"
          { id24 (2, 0x3c, 0); F ((yyvsp[-6].regno), 15, 1); F ((yyvsp[-3].regno), 16, 4); F ((yyvsp[0].regno), 20, 4); }
#line 3494 "config/rx-parse.c"
    break;

  case 190: /* $@36: %empty  */
#line 785 "./config/rx-parse.y"
               { sub_op = 6; }
#line 3500 "config/rx-parse.c"
    break;

  case 192: /* $@37: %empty  */
#line 786 "./config/rx-parse.y"
               { sub_op = 4; }
#line 3506 "config/rx-parse.c"
    break;

  case 194: /* $@38: %empty  */
#line 787 "./config/rx-parse.y"
               { sub_op = 5; }
#line 3512 "config/rx-parse.c"
    break;

  case 196: /* $@39: %empty  */
#line 788 "./config/rx-parse.y"
               { sub_op = 7; }
#line 3518 "config/rx-parse.c"
    break;

  case 198: /* statement: MVTC REG ',' CREG  */
#line 793 "./config/rx-parse.y"
          { if ((yyvsp[0].regno) == 13)
	      rx_check_v2 ();
	  id24 (2, 0x68, 0x00); F ((yyvsp[0].regno) % 16, 20, 4); F ((yyvsp[0].regno) / 16, 15, 1);
	    F ((yyvsp[-2].regno), 16, 4); }
#line 3527 "config/rx-parse.c"
    break;

  case 199: /* statement: MVFC CREG ',' REG  */
#line 801 "./config/rx-parse.y"
          { if ((yyvsp[-2].regno) == 13)
	    rx_check_v2 ();
	  id24 (2, 0x6a, 0); F ((yyvsp[-2].regno), 15, 5); F ((yyvsp[0].regno), 20, 4); }
#line 3535 "config/rx-parse.c"
    break;

  case 200: /* statement: ROTL '#' EXPR ',' REG  */
#line 808 "./config/rx-parse.y"
          { id24 (2, 0x6e, 0); FE ((yyvsp[-2].exp), 15, 5); F ((yyvsp[0].regno), 20, 4); }
#line 3541 "config/rx-parse.c"
    break;

  case 201: /* statement: ROTR '#' EXPR ',' REG  */
#line 810 "./config/rx-parse.y"
          { id24 (2, 0x6c, 0); FE ((yyvsp[-2].exp), 15, 5); F ((yyvsp[0].regno), 20, 4); }
#line 3547 "config/rx-parse.c"
    break;

  case 202: /* statement: MVTC '#' EXPR ',' CREG  */
#line 815 "./config/rx-parse.y"
          { if ((yyvsp[0].regno) == 13)
	      rx_check_v2 ();
	    id24 (2, 0x73, 0x00); F ((yyvsp[0].regno), 19, 5); IMM ((yyvsp[-2].exp), 12); }
#line 3555 "config/rx-parse.c"
    break;

  case 203: /* statement: BMCND '#' EXPR ',' REG  */
#line 822 "./config/rx-parse.y"
          { id24 (2, 0xe0, 0x00); F ((yyvsp[-4].regno), 16, 4); FE ((yyvsp[-2].exp), 11, 5);
	      F ((yyvsp[0].regno), 20, 4); }
#line 3562 "config/rx-parse.c"
    break;

  case 204: /* statement: BNOT '#' EXPR ',' REG  */
#line 828 "./config/rx-parse.y"
          { id24 (2, 0xe0, 0xf0); FE ((yyvsp[-2].exp), 11, 5); F ((yyvsp[0].regno), 20, 4); }
#line 3568 "config/rx-parse.c"
    break;

  case 205: /* statement: MOV bwl REG ',' '[' REG ',' REG ']'  */
#line 833 "./config/rx-parse.y"
          { id24 (3, 0x00, 0); F ((yyvsp[-7].regno), 10, 2); F ((yyvsp[-3].regno), 12, 4); F ((yyvsp[-1].regno), 16, 4); F ((yyvsp[-6].regno), 20, 4); }
#line 3574 "config/rx-parse.c"
    break;

  case 206: /* statement: MOV bwl '[' REG ',' REG ']' ',' REG  */
#line 836 "./config/rx-parse.y"
          { id24 (3, 0x40, 0); F ((yyvsp[-7].regno), 10, 2); F ((yyvsp[-5].regno), 12, 4); F ((yyvsp[-3].regno), 16, 4); F ((yyvsp[0].regno), 20, 4); }
#line 3580 "config/rx-parse.c"
    break;

  case 207: /* statement: MOVU bw '[' REG ',' REG ']' ',' REG  */
#line 839 "./config/rx-parse.y"
          { id24 (3, 0xc0, 0); F ((yyvsp[-7].regno), 10, 2); F ((yyvsp[-5].regno), 12, 4); F ((yyvsp[-3].regno), 16, 4); F ((yyvsp[0].regno), 20, 4); }
#line 3586 "config/rx-parse.c"
    break;

  case 208: /* $@40: %empty  */
#line 843 "./config/rx-parse.y"
              { sub_op = 0; }
#line 3592 "config/rx-parse.c"
    break;

  case 210: /* $@41: %empty  */
#line 844 "./config/rx-parse.y"
              { sub_op = 2; }
#line 3598 "config/rx-parse.c"
    break;

  case 212: /* $@42: %empty  */
#line 845 "./config/rx-parse.y"
              { sub_op = 3; }
#line 3604 "config/rx-parse.c"
    break;

  case 214: /* $@43: %empty  */
#line 846 "./config/rx-parse.y"
               { sub_op = 4; }
#line 3610 "config/rx-parse.c"
    break;

  case 216: /* $@44: %empty  */
#line 847 "./config/rx-parse.y"
              { sub_op = 5; }
#line 3616 "config/rx-parse.c"
    break;

  case 218: /* statement: SBB '#' EXPR ',' REG  */
#line 853 "./config/rx-parse.y"
          { id24 (2, 0x70, 0x20); F ((yyvsp[0].regno), 20, 4); NBIMM ((yyvsp[-2].exp), 12); }
#line 3622 "config/rx-parse.c"
    break;

  case 219: /* statement: MOVCO REG ',' '[' REG ']'  */
#line 858 "./config/rx-parse.y"
          { rx_check_v2 (); B3 (0xfd, 0x27, 0x00); F ((yyvsp[-1].regno), 16, 4); F ((yyvsp[-4].regno), 20, 4); }
#line 3628 "config/rx-parse.c"
    break;

  case 220: /* statement: MOVLI '[' REG ']' ',' REG  */
#line 863 "./config/rx-parse.y"
          { rx_check_v2 (); B3 (0xfd, 0x2f, 0x00); F ((yyvsp[-3].regno), 16, 4); F ((yyvsp[0].regno), 20, 4); }
#line 3634 "config/rx-parse.c"
    break;

  case 221: /* statement: EMACA REG ',' REG ',' ACC  */
#line 868 "./config/rx-parse.y"
          { rx_check_v2 (); id24 (2, 0x07, 0x00); F ((yyvsp[-4].regno), 16, 4); F ((yyvsp[-2].regno), 20, 4); F ((yyvsp[0].regno), 12, 1); }
#line 3640 "config/rx-parse.c"
    break;

  case 222: /* statement: EMSBA REG ',' REG ',' ACC  */
#line 870 "./config/rx-parse.y"
          { rx_check_v2 (); id24 (2, 0x47, 0x00); F ((yyvsp[-4].regno), 16, 4); F ((yyvsp[-2].regno), 20, 4); F ((yyvsp[0].regno), 12, 1); }
#line 3646 "config/rx-parse.c"
    break;

  case 223: /* statement: EMULA REG ',' REG ',' ACC  */
#line 872 "./config/rx-parse.y"
          { rx_check_v2 (); id24 (2, 0x03, 0x00); F ((yyvsp[-4].regno), 16, 4); F ((yyvsp[-2].regno), 20, 4); F ((yyvsp[0].regno), 12, 1); }
#line 3652 "config/rx-parse.c"
    break;

  case 224: /* statement: MACLH REG ',' REG ',' ACC  */
#line 874 "./config/rx-parse.y"
          { rx_check_v2 (); id24 (2, 0x06, 0x00); F ((yyvsp[-4].regno), 16, 4); F ((yyvsp[-2].regno), 20, 4); F ((yyvsp[0].regno), 12, 1); }
#line 3658 "config/rx-parse.c"
    break;

  case 225: /* statement: MSBHI REG ',' REG ',' ACC  */
#line 876 "./config/rx-parse.y"
          { rx_check_v2 (); id24 (2, 0x44, 0x00); F ((yyvsp[-4].regno), 16, 4); F ((yyvsp[-2].regno), 20, 4); F ((yyvsp[0].regno), 12, 1); }
#line 3664 "config/rx-parse.c"
    break;

  case 226: /* statement: MSBLH REG ',' REG ',' ACC  */
#line 878 "./config/rx-parse.y"
          { rx_check_v2 (); id24 (2, 0x46, 0x00); F ((yyvsp[-4].regno), 16, 4); F ((yyvsp[-2].regno), 20, 4); F ((yyvsp[0].regno), 12, 1); }
#line 3670 "config/rx-parse.c"
    break;

  case 227: /* statement: MSBLO REG ',' REG ',' ACC  */
#line 880 "./config/rx-parse.y"
          { rx_check_v2 (); id24 (2, 0x45, 0x00); F ((yyvsp[-4].regno), 16, 4); F ((yyvsp[-2].regno), 20, 4); F ((yyvsp[0].regno), 12, 1); }
#line 3676 "config/rx-parse.c"
    break;

  case 228: /* statement: MULLH REG ',' REG ',' ACC  */
#line 882 "./config/rx-parse.y"
          { rx_check_v2 (); id24 (2, 0x02, 0x00); F ((yyvsp[-4].regno), 16, 4); F ((yyvsp[-2].regno), 20, 4); F ((yyvsp[0].regno), 12, 1); }
#line 3682 "config/rx-parse.c"
    break;

  case 229: /* $@45: %empty  */
#line 883 "./config/rx-parse.y"
                  { sub_op = 3; }
#line 3688 "config/rx-parse.c"
    break;

  case 231: /* statement: MVTACGU REG ',' ACC  */
#line 885 "./config/rx-parse.y"
          { rx_check_v2 (); id24 (2, 0x17, 0x30); F ((yyvsp[0].regno), 16, 1); F ((yyvsp[-2].regno), 20, 4); }
#line 3694 "config/rx-parse.c"
    break;

  case 232: /* statement: RACL '#' EXPR ',' ACC  */
#line 887 "./config/rx-parse.y"
        { rx_check_v2 (); id24 (2, 0x19, 0x00); F ((yyvsp[0].regno), 16, 1);
	    if (rx_uintop ((yyvsp[-2].exp), 4) && (yyvsp[-2].exp).X_add_number == 1)
	      ;
	    else if (rx_uintop ((yyvsp[-2].exp), 4) && (yyvsp[-2].exp).X_add_number == 2)
	      F (1, 19, 1);
	    else
	      as_bad (_("RACL expects #1 or #2"));}
#line 3706 "config/rx-parse.c"
    break;

  case 233: /* statement: RDACL '#' EXPR ',' ACC  */
#line 895 "./config/rx-parse.y"
        { rx_check_v2 (); id24 (2, 0x19, 0x40); F ((yyvsp[0].regno), 16, 1);
	    if (rx_uintop ((yyvsp[-2].exp), 4) && (yyvsp[-2].exp).X_add_number == 1)
	      ;
	    else if (rx_uintop ((yyvsp[-2].exp), 4) && (yyvsp[-2].exp).X_add_number == 2)
	      F (1, 19, 1);
	    else
	      as_bad (_("RDACL expects #1 or #2"));}
#line 3718 "config/rx-parse.c"
    break;

  case 234: /* statement: RDACW '#' EXPR ',' ACC  */
#line 903 "./config/rx-parse.y"
        { rx_check_v2 (); id24 (2, 0x18, 0x40); F ((yyvsp[0].regno), 16, 1);
	    if (rx_uintop ((yyvsp[-2].exp), 4) && (yyvsp[-2].exp).X_add_number == 1)
	      ;
	    else if (rx_uintop ((yyvsp[-2].exp), 4) && (yyvsp[-2].exp).X_add_number == 2)
	      F (1, 19, 1);
	    else
	      as_bad (_("RDACW expects #1 or #2"));}
#line 3730 "config/rx-parse.c"
    break;

  case 235: /* $@46: %empty  */
#line 912 "./config/rx-parse.y"
                { rx_check_v3(); sub_op = 1; }
#line 3736 "config/rx-parse.c"
    break;

  case 237: /* $@47: %empty  */
#line 913 "./config/rx-parse.y"
                 { rx_check_v3(); sub_op = 0; }
#line 3742 "config/rx-parse.c"
    break;

  case 239: /* $@48: %empty  */
#line 916 "./config/rx-parse.y"
               { rx_check_v3(); sub_op = 1; }
#line 3748 "config/rx-parse.c"
    break;

  case 241: /* $@49: %empty  */
#line 917 "./config/rx-parse.y"
               { rx_check_v3(); sub_op = 0; }
#line 3754 "config/rx-parse.c"
    break;

  case 243: /* $@50: %empty  */
#line 920 "./config/rx-parse.y"
               { rx_check_dfpu(); sub_op = 0x0c; sub_op2 = 0x01; }
#line 3760 "config/rx-parse.c"
    break;

  case 245: /* $@51: %empty  */
#line 921 "./config/rx-parse.y"
               { rx_check_dfpu(); sub_op = 0x0c; sub_op2 = 0x02; }
#line 3766 "config/rx-parse.c"
    break;

  case 247: /* $@52: %empty  */
#line 922 "./config/rx-parse.y"
                 { rx_check_dfpu(); sub_op = 0x0d; sub_op2 = 0x0d; }
#line 3772 "config/rx-parse.c"
    break;

  case 249: /* $@53: %empty  */
#line 923 "./config/rx-parse.y"
                { rx_check_dfpu(); sub_op = 0x0d; sub_op2 = 0x00; }
#line 3778 "config/rx-parse.c"
    break;

  case 251: /* $@54: %empty  */
#line 924 "./config/rx-parse.y"
               { rx_check_dfpu(); sub_op = 0x0d; sub_op2 = 0x0c; }
#line 3784 "config/rx-parse.c"
    break;

  case 253: /* $@55: %empty  */
#line 925 "./config/rx-parse.y"
               { rx_check_dfpu(); sub_op = 0x0d; sub_op2 = 0x08;}
#line 3790 "config/rx-parse.c"
    break;

  case 255: /* $@56: %empty  */
#line 926 "./config/rx-parse.y"
               { rx_check_dfpu(); sub_op = 0x0d; sub_op2 = 0x09; }
#line 3796 "config/rx-parse.c"
    break;

  case 257: /* $@57: %empty  */
#line 927 "./config/rx-parse.y"
               { rx_check_dfpu(); sub_op = 0x00; }
#line 3802 "config/rx-parse.c"
    break;

  case 259: /* $@58: %empty  */
#line 928 "./config/rx-parse.y"
               { rx_check_dfpu(); sub_op = 0x05; }
#line 3808 "config/rx-parse.c"
    break;

  case 261: /* $@59: %empty  */
#line 929 "./config/rx-parse.y"
               { rx_check_dfpu(); sub_op = 0x02; }
#line 3814 "config/rx-parse.c"
    break;

  case 263: /* $@60: %empty  */
#line 930 "./config/rx-parse.y"
               { rx_check_dfpu(); sub_op = 0x01; }
#line 3820 "config/rx-parse.c"
    break;

  case 265: /* statement: DCMP DREG ',' DREG  */
#line 931 "./config/rx-parse.y"
                             { rx_check_dfpu();
	    B4(0x76, 0x90, 0x08, 0x00); F((yyvsp[-3].regno), 24, 4); F((yyvsp[-2].regno), 28, 4); F((yyvsp[0].regno), 16, 4); }
#line 3827 "config/rx-parse.c"
    break;

  case 266: /* statement: DMOV DOT_D REG ',' DREGH  */
#line 934 "./config/rx-parse.y"
        { rx_check_dfpu();
	  B4(0xfd, 0x77, 0x80, 0x03); F((yyvsp[-2].regno), 20, 4); F((yyvsp[0].regno), 24, 4); }
#line 3834 "config/rx-parse.c"
    break;

  case 267: /* statement: DMOV DOT_L REG ',' DREGH  */
#line 937 "./config/rx-parse.y"
        { rx_check_dfpu();
	  B4(0xfd, 0x77, 0x80, 0x02); F((yyvsp[-2].regno), 20, 4); F((yyvsp[0].regno), 24, 4); }
#line 3841 "config/rx-parse.c"
    break;

  case 268: /* statement: DMOV DOT_L REG ',' DREGL  */
#line 940 "./config/rx-parse.y"
        { rx_check_dfpu();
	  B4(0xfd, 0x77, 0x80, 0x00); F((yyvsp[-2].regno), 20, 4); F((yyvsp[0].regno), 24, 4); }
#line 3848 "config/rx-parse.c"
    break;

  case 269: /* statement: DMOV DOT_L DREGH ',' REG  */
#line 943 "./config/rx-parse.y"
        { rx_check_dfpu();
	  B4(0xfd, 0x75, 0x80, 0x02); F((yyvsp[-2].regno), 24, 4); F((yyvsp[0].regno), 20, 4); }
#line 3855 "config/rx-parse.c"
    break;

  case 270: /* statement: DMOV DOT_L DREGL ',' REG  */
#line 946 "./config/rx-parse.y"
        { rx_check_dfpu();
	  B4(0xfd, 0x75, 0x80, 0x00); F((yyvsp[-2].regno), 24, 4); F((yyvsp[0].regno), 20, 4); }
#line 3862 "config/rx-parse.c"
    break;

  case 271: /* statement: DMOV DOT_D DREG ',' DREG  */
#line 949 "./config/rx-parse.y"
        { rx_check_dfpu();
	  B4(0x76, 0x90, 0x0c, 0x00); F((yyvsp[-2].regno), 16, 4); F((yyvsp[0].regno), 24, 4); }
#line 3869 "config/rx-parse.c"
    break;

  case 272: /* statement: DMOV DOT_D DREG ',' '[' REG ']'  */
#line 952 "./config/rx-parse.y"
        { rx_check_dfpu();
	  B4(0xfc, 0x78, 0x08, 0x00); F((yyvsp[-1].regno), 16, 4); F((yyvsp[-4].regno), 24, 4); }
#line 3876 "config/rx-parse.c"
    break;

  case 273: /* statement: DMOV DOT_D DREG ',' disp '[' REG ']'  */
#line 955 "./config/rx-parse.y"
        { rx_check_dfpu();
	  B3(0xfc, 0x78, 0x08); F((yyvsp[-1].regno), 16, 4); DSP((yyvsp[-3].exp), 14, DSIZE);
	  POST((yyvsp[-5].regno) << 4); }
#line 3884 "config/rx-parse.c"
    break;

  case 274: /* statement: DMOV DOT_D '[' REG ']' ',' DREG  */
#line 959 "./config/rx-parse.y"
        { rx_check_dfpu();
	  B4(0xfc, 0xc8, 0x08, 0x00); F((yyvsp[-3].regno), 16, 4); F((yyvsp[0].regno), 24, 4); }
#line 3891 "config/rx-parse.c"
    break;

  case 275: /* statement: DMOV DOT_D disp '[' REG ']' ',' DREG  */
#line 962 "./config/rx-parse.y"
        { rx_check_dfpu();
	  B3(0xfc, 0xc8, 0x08); F((yyvsp[-3].regno), 16, 4); DSP((yyvsp[-5].exp), 14, DSIZE);
	  POST((yyvsp[0].regno) << 4); }
#line 3899 "config/rx-parse.c"
    break;

  case 276: /* statement: DMOV DOT_D '#' EXPR ',' DREGH  */
#line 966 "./config/rx-parse.y"
        { rx_check_dfpu();
	  B3(0xf9, 0x03, 0x03); F((yyvsp[0].regno), 16, 4); IMM((yyvsp[-2].exp), -1); }
#line 3906 "config/rx-parse.c"
    break;

  case 277: /* statement: DMOV DOT_L '#' EXPR ',' DREGH  */
#line 969 "./config/rx-parse.y"
        { rx_check_dfpu();
	  B3(0xf9, 0x03, 0x02); F((yyvsp[0].regno), 16, 4); IMM((yyvsp[-2].exp), -1); }
#line 3913 "config/rx-parse.c"
    break;

  case 278: /* statement: DMOV DOT_L '#' EXPR ',' DREGL  */
#line 972 "./config/rx-parse.y"
        { rx_check_dfpu();
	  B3(0xf9, 0x03, 0x00); F((yyvsp[0].regno), 16, 4); IMM((yyvsp[-2].exp), -1); }
#line 3920 "config/rx-parse.c"
    break;

  case 279: /* statement: DPOPM DOT_D DREG '-' DREG  */
#line 975 "./config/rx-parse.y"
        { rx_check_dfpu();
	  B3(0x75, 0xb8, 0x00); F((yyvsp[-2].regno), 16, 4); F((yyvsp[0].regno) - (yyvsp[-2].regno), 20, 4); }
#line 3927 "config/rx-parse.c"
    break;

  case 280: /* statement: DPOPM DOT_L DCREG '-' DCREG  */
#line 978 "./config/rx-parse.y"
        { rx_check_dfpu();
	  B3(0x75, 0xa8, 0x00); F((yyvsp[-2].regno), 16, 4); F((yyvsp[0].regno) - (yyvsp[-2].regno), 20, 4); }
#line 3934 "config/rx-parse.c"
    break;

  case 281: /* statement: DPUSHM DOT_D DREG '-' DREG  */
#line 981 "./config/rx-parse.y"
        { rx_check_dfpu();
	  B3(0x75, 0xb0, 0x00); F((yyvsp[-2].regno), 16, 4); F((yyvsp[0].regno) - (yyvsp[-2].regno), 20, 4); }
#line 3941 "config/rx-parse.c"
    break;

  case 282: /* statement: DPUSHM DOT_L DCREG '-' DCREG  */
#line 984 "./config/rx-parse.y"
        { rx_check_dfpu();
	  B3(0x75, 0xa0, 0x00); F((yyvsp[-2].regno), 16, 4); F((yyvsp[0].regno) - (yyvsp[-2].regno), 20, 4); }
#line 3948 "config/rx-parse.c"
    break;

  case 283: /* statement: MVFDC DCREG ',' REG  */
#line 987 "./config/rx-parse.y"
        { rx_check_dfpu();
	  B4(0xfd, 0x75, 0x80, 0x04); F((yyvsp[-2].regno), 24, 4); F((yyvsp[0].regno), 20, 4); }
#line 3955 "config/rx-parse.c"
    break;

  case 284: /* statement: MVFDR  */
#line 990 "./config/rx-parse.y"
        { rx_check_dfpu(); B3(0x75, 0x90, 0x1b); }
#line 3961 "config/rx-parse.c"
    break;

  case 285: /* statement: MVTDC REG ',' DCREG  */
#line 992 "./config/rx-parse.y"
        { rx_check_dfpu();
	  B4(0xfd, 0x77, 0x80, 0x04); F((yyvsp[-2].regno), 24, 4); F((yyvsp[0].regno), 20, 4); }
#line 3968 "config/rx-parse.c"
    break;

  case 286: /* statement: FTOD REG ',' DREG  */
#line 995 "./config/rx-parse.y"
        { rx_check_dfpu();
	  B4(0xfd, 0x77, 0x80, 0x0a); F((yyvsp[-2].regno), 24, 4); F((yyvsp[0].regno), 20, 4); }
#line 3975 "config/rx-parse.c"
    break;

  case 287: /* statement: ITOD REG ',' DREG  */
#line 998 "./config/rx-parse.y"
        { rx_check_dfpu();
	  B4(0xfd, 0x77, 0x80, 0x09); F((yyvsp[-2].regno), 24, 4); F((yyvsp[0].regno), 20, 4); }
#line 3982 "config/rx-parse.c"
    break;

  case 288: /* statement: UTOD REG ',' DREG  */
#line 1001 "./config/rx-parse.y"
        { rx_check_dfpu();
	  B4(0xfd, 0x77, 0x80, 0x0d); F((yyvsp[-2].regno), 24, 4); F((yyvsp[0].regno), 20, 4); }
#line 3989 "config/rx-parse.c"
    break;

  case 289: /* op_subadd: REG ',' REG  */
#line 1012 "./config/rx-parse.y"
          { B2 (0x43 + (sub_op<<2), 0); F ((yyvsp[-2].regno), 8, 4); F ((yyvsp[0].regno), 12, 4); }
#line 3995 "config/rx-parse.c"
    break;

  case 290: /* op_subadd: disp '[' REG ']' DOT_UB ',' REG  */
#line 1014 "./config/rx-parse.y"
          { B2 (0x40 + (sub_op<<2), 0); F ((yyvsp[-4].regno), 8, 4); F ((yyvsp[0].regno), 12, 4); DSP ((yyvsp[-6].exp), 6, BSIZE); }
#line 4001 "config/rx-parse.c"
    break;

  case 291: /* op_subadd: disp '[' REG ']' memex ',' REG  */
#line 1016 "./config/rx-parse.y"
          { B3 (MEMEX, sub_op<<2, 0); F ((yyvsp[-2].regno), 8, 2); F ((yyvsp[-4].regno), 16, 4); F ((yyvsp[0].regno), 20, 4); DSP ((yyvsp[-6].exp), 14, sizemap[(yyvsp[-2].regno)]); }
#line 4007 "config/rx-parse.c"
    break;

  case 292: /* op_subadd: REG ',' REG ',' REG  */
#line 1018 "./config/rx-parse.y"
          { id24 (4, sub_op<<4, 0), F ((yyvsp[0].regno), 12, 4), F ((yyvsp[-4].regno), 16, 4), F ((yyvsp[-2].regno), 20, 4); }
#line 4013 "config/rx-parse.c"
    break;

  case 293: /* op_dp20_rm_l: REG ',' REG  */
#line 1025 "./config/rx-parse.y"
          { id24 (1, 0x03 + (sub_op<<2), 0x00); F ((yyvsp[-2].regno), 16, 4); F ((yyvsp[0].regno), 20, 4); }
#line 4019 "config/rx-parse.c"
    break;

  case 294: /* op_dp20_rm_l: disp '[' REG ']' opt_l ',' REG  */
#line 1027 "./config/rx-parse.y"
          { B4 (MEMEX, 0xa0, 0x00 + sub_op, 0x00);
	  F ((yyvsp[-4].regno), 24, 4); F ((yyvsp[0].regno), 28, 4); DSP ((yyvsp[-6].exp), 14, LSIZE); }
#line 4026 "config/rx-parse.c"
    break;

  case 295: /* op_dp20_rm: REG ',' REG  */
#line 1035 "./config/rx-parse.y"
          { id24 (1, 0x03 + (sub_op<<2), 0x00); F ((yyvsp[-2].regno), 16, 4); F ((yyvsp[0].regno), 20, 4); }
#line 4032 "config/rx-parse.c"
    break;

  case 296: /* op_dp20_rm: disp '[' REG ']' DOT_UB ',' REG  */
#line 1037 "./config/rx-parse.y"
          { id24 (1, 0x00 + (sub_op<<2), 0x00); F ((yyvsp[-4].regno), 16, 4); F ((yyvsp[0].regno), 20, 4); DSP ((yyvsp[-6].exp), 14, BSIZE); }
#line 4038 "config/rx-parse.c"
    break;

  case 297: /* op_dp20_rm: disp '[' REG ']' memex ',' REG  */
#line 1039 "./config/rx-parse.y"
          { B4 (MEMEX, 0x20 + ((yyvsp[-2].regno) << 6), 0x00 + sub_op, 0x00);
	  F ((yyvsp[-4].regno), 24, 4); F ((yyvsp[0].regno), 28, 4); DSP ((yyvsp[-6].exp), 14, sizemap[(yyvsp[-2].regno)]); }
#line 4045 "config/rx-parse.c"
    break;

  case 298: /* op_dp20_i: '#' EXPR ',' REG  */
#line 1045 "./config/rx-parse.y"
          { id24 (2, 0x70, sub_op<<4); F ((yyvsp[0].regno), 20, 4); IMM ((yyvsp[-2].exp), 12); }
#line 4051 "config/rx-parse.c"
    break;

  case 303: /* op_dp20_rr: REG ',' REG  */
#line 1060 "./config/rx-parse.y"
          { id24 (1, 0x03 + (sub_op<<2), 0x00); F ((yyvsp[-2].regno), 16, 4); F ((yyvsp[0].regno), 20, 4); }
#line 4057 "config/rx-parse.c"
    break;

  case 304: /* op_dp20_rr: REG  */
#line 1062 "./config/rx-parse.y"
          { B2 (0x7e, sub_op2 << 4); F ((yyvsp[0].regno), 12, 4); }
#line 4063 "config/rx-parse.c"
    break;

  case 305: /* op_dp20_r: REG ',' REG  */
#line 1067 "./config/rx-parse.y"
          { id24 (1, 0x4b + (sub_op2<<2), 0x00); F ((yyvsp[-2].regno), 16, 4); F ((yyvsp[0].regno), 20, 4); }
#line 4069 "config/rx-parse.c"
    break;

  case 306: /* $@61: %empty  */
#line 1071 "./config/rx-parse.y"
          { rx_check_v2 (); }
#line 4075 "config/rx-parse.c"
    break;

  case 309: /* op_xchg: REG ',' REG  */
#line 1079 "./config/rx-parse.y"
          { id24 (1, 0x03 + (sub_op<<2), 0); F ((yyvsp[-2].regno), 16, 4); F ((yyvsp[0].regno), 20, 4); }
#line 4081 "config/rx-parse.c"
    break;

  case 310: /* op_xchg: disp '[' REG ']' DOT_UB ',' REG  */
#line 1081 "./config/rx-parse.y"
          { id24 (1, 0x00 + (sub_op<<2), 0); F ((yyvsp[-4].regno), 16, 4); F ((yyvsp[0].regno), 20, 4); DSP ((yyvsp[-6].exp), 14, BSIZE); }
#line 4087 "config/rx-parse.c"
    break;

  case 311: /* op_xchg: disp '[' REG ']' memex ',' REG  */
#line 1083 "./config/rx-parse.y"
          { B4 (MEMEX, 0x20, 0x00 + sub_op, 0); F ((yyvsp[-2].regno), 8, 2); F ((yyvsp[-4].regno), 24, 4); F ((yyvsp[0].regno), 28, 4);
	    DSP ((yyvsp[-6].exp), 14, sizemap[(yyvsp[-2].regno)]); }
#line 4094 "config/rx-parse.c"
    break;

  case 312: /* op_shift_rot: REG ',' REG  */
#line 1090 "./config/rx-parse.y"
          { id24 (2, 0x60 + sub_op, 0); F ((yyvsp[-2].regno), 16, 4); F ((yyvsp[0].regno), 20, 4); }
#line 4100 "config/rx-parse.c"
    break;

  case 313: /* op_shift: '#' EXPR ',' REG  */
#line 1094 "./config/rx-parse.y"
          { B2 (0x68 + (sub_op<<1), 0); FE ((yyvsp[-2].exp), 7, 5); F ((yyvsp[0].regno), 12, 4); }
#line 4106 "config/rx-parse.c"
    break;

  case 314: /* op_shift: '#' EXPR ',' REG ',' REG  */
#line 1096 "./config/rx-parse.y"
          { id24 (2, 0x80 + (sub_op << 5), 0); FE ((yyvsp[-4].exp), 11, 5); F ((yyvsp[-2].regno), 16, 4); F ((yyvsp[0].regno), 20, 4); }
#line 4112 "config/rx-parse.c"
    break;

  case 316: /* float3_op: '#' EXPR ',' REG  */
#line 1102 "./config/rx-parse.y"
          { rx_check_float_support (); id24 (2, 0x72, sub_op << 4); F ((yyvsp[0].regno), 20, 4); O4 ((yyvsp[-2].exp)); }
#line 4118 "config/rx-parse.c"
    break;

  case 317: /* float3_op: REG ',' REG  */
#line 1104 "./config/rx-parse.y"
          { rx_check_float_support (); id24 (1, 0x83 + (sub_op << 2), 0); F ((yyvsp[-2].regno), 16, 4); F ((yyvsp[0].regno), 20, 4); }
#line 4124 "config/rx-parse.c"
    break;

  case 318: /* float3_op: disp '[' REG ']' opt_l ',' REG  */
#line 1106 "./config/rx-parse.y"
          { rx_check_float_support (); id24 (1, 0x80 + (sub_op << 2), 0); F ((yyvsp[-4].regno), 16, 4); F ((yyvsp[0].regno), 20, 4); DSP ((yyvsp[-6].exp), 14, LSIZE); }
#line 4130 "config/rx-parse.c"
    break;

  case 319: /* float3_op: REG ',' REG ',' REG  */
#line 1108 "./config/rx-parse.y"
          { rx_check_v2 (); id24 (4, 0x80 + (sub_op << 4), 0 ); F ((yyvsp[-4].regno), 16, 4); F ((yyvsp[-2].regno), 20, 4); F ((yyvsp[0].regno), 12, 4); }
#line 4136 "config/rx-parse.c"
    break;

  case 320: /* $@62: %empty  */
#line 1112 "./config/rx-parse.y"
          { rx_check_float_support (); }
#line 4142 "config/rx-parse.c"
    break;

  case 321: /* float2_op: $@62 '#' EXPR ',' REG  */
#line 1114 "./config/rx-parse.y"
          { id24 (2, 0x72, sub_op << 4); F ((yyvsp[0].regno), 20, 4); O4 ((yyvsp[-2].exp)); }
#line 4148 "config/rx-parse.c"
    break;

  case 323: /* $@63: %empty  */
#line 1119 "./config/rx-parse.y"
          { rx_check_float_support (); }
#line 4154 "config/rx-parse.c"
    break;

  case 324: /* float2_op_ni: $@63 REG ',' REG  */
#line 1121 "./config/rx-parse.y"
          { id24 (1, 0x83 + (sub_op << 2), 0); F ((yyvsp[-2].regno), 16, 4); F ((yyvsp[0].regno), 20, 4); }
#line 4160 "config/rx-parse.c"
    break;

  case 325: /* $@64: %empty  */
#line 1122 "./config/rx-parse.y"
          { rx_check_float_support (); }
#line 4166 "config/rx-parse.c"
    break;

  case 326: /* float2_op_ni: $@64 disp '[' REG ']' opt_l ',' REG  */
#line 1124 "./config/rx-parse.y"
          { id24 (1, 0x80 + (sub_op << 2), 0); F ((yyvsp[-4].regno), 16, 4); F ((yyvsp[0].regno), 20, 4); DSP ((yyvsp[-6].exp), 14, LSIZE); }
#line 4172 "config/rx-parse.c"
    break;

  case 327: /* $@65: %empty  */
#line 1128 "./config/rx-parse.y"
          { rx_check_v2 (); }
#line 4178 "config/rx-parse.c"
    break;

  case 328: /* mvfa_op: $@65 '#' EXPR ',' ACC ',' REG  */
#line 1130 "./config/rx-parse.y"
          { id24 (2, 0x1e, sub_op << 4); F ((yyvsp[0].regno), 20, 4); F ((yyvsp[-2].regno), 16, 1);
	    if (rx_uintop ((yyvsp[-4].exp), 4))
	      {
		switch (exp_val ((yyvsp[-4].exp)))
		  {
		  case 0:
		    F (1, 15, 1);
		    break;
		  case 1:
		    F (1, 15, 1);
		    F (1, 17, 1);
		    break;
		  case 2:
		    break;
		  default:
		    as_bad (_("IMM expects #0 to #2"));}
	      } else
	        as_bad (_("IMM expects #0 to #2"));}
#line 4201 "config/rx-parse.c"
    break;

  case 330: /* op_xor: REG ',' REG ',' REG  */
#line 1153 "./config/rx-parse.y"
          { rx_check_v3(); B3(0xff,0x60,0x00), F ((yyvsp[0].regno), 12, 4), F ((yyvsp[-4].regno), 16, 4), F ((yyvsp[-2].regno), 20, 4); }
#line 4207 "config/rx-parse.c"
    break;

  case 331: /* $@66: %empty  */
#line 1157 "./config/rx-parse.y"
          { rx_check_v3(); }
#line 4213 "config/rx-parse.c"
    break;

  case 332: /* op_bfield: $@66 '#' EXPR ',' '#' EXPR ',' '#' EXPR ',' REG ',' REG  */
#line 1159 "./config/rx-parse.y"
          { rx_range((yyvsp[-10].exp), 0, 31); rx_range((yyvsp[-7].exp), 0, 31); rx_range((yyvsp[-4].exp), 1, 31);
	    B3(0xfc, 0x5a + (sub_op << 2), 0); F((yyvsp[-2].regno), 16, 4); F((yyvsp[0].regno), 20, 4);
	  rx_bfield((yyvsp[-10].exp), (yyvsp[-7].exp), (yyvsp[-4].exp));}
#line 4221 "config/rx-parse.c"
    break;

  case 333: /* op_save_rstr: '#' EXPR  */
#line 1166 "./config/rx-parse.y"
          { B3(0xfd,0x76,0xe0 + (sub_op << 4)); UO1((yyvsp[0].exp)); }
#line 4227 "config/rx-parse.c"
    break;

  case 334: /* op_save_rstr: REG  */
#line 1168 "./config/rx-parse.y"
          { B4(0xfd,0x76,0xc0 + (sub_op << 4), 0x00); F((yyvsp[0].regno), 20, 4); }
#line 4233 "config/rx-parse.c"
    break;

  case 335: /* double2_op: DREG ',' DREG  */
#line 1173 "./config/rx-parse.y"
        { B4(0x76, 0x90, sub_op, sub_op2); F((yyvsp[-2].regno), 16, 4); F((yyvsp[0].regno), 24, 4);}
#line 4239 "config/rx-parse.c"
    break;

  case 336: /* double3_op: DREG ',' DREG ',' DREG  */
#line 1177 "./config/rx-parse.y"
        { B4(0x76, 0x90, sub_op, 0x00); F((yyvsp[-4].regno), 28, 4); F((yyvsp[-2].regno), 16,4); F((yyvsp[0].regno), 24, 4);}
#line 4245 "config/rx-parse.c"
    break;

  case 337: /* disp: %empty  */
#line 1181 "./config/rx-parse.y"
               { (yyval.exp) = zero_expr (); }
#line 4251 "config/rx-parse.c"
    break;

  case 338: /* disp: EXPR  */
#line 1182 "./config/rx-parse.y"
               { (yyval.exp) = (yyvsp[0].exp); }
#line 4257 "config/rx-parse.c"
    break;

  case 339: /* $@67: %empty  */
#line 1185 "./config/rx-parse.y"
          { need_flag = 1; }
#line 4263 "config/rx-parse.c"
    break;

  case 340: /* flag: $@67 FLAG  */
#line 1185 "./config/rx-parse.y"
                                  { need_flag = 0; (yyval.regno) = (yyvsp[0].regno); }
#line 4269 "config/rx-parse.c"
    break;

  case 341: /* memex: DOT_B  */
#line 1190 "./config/rx-parse.y"
                 { (yyval.regno) = 0; }
#line 4275 "config/rx-parse.c"
    break;

  case 342: /* memex: DOT_W  */
#line 1191 "./config/rx-parse.y"
                 { (yyval.regno) = 1; }
#line 4281 "config/rx-parse.c"
    break;

  case 343: /* memex: %empty  */
#line 1192 "./config/rx-parse.y"
                 { (yyval.regno) = 2; }
#line 4287 "config/rx-parse.c"
    break;

  case 344: /* memex: DOT_L  */
#line 1193 "./config/rx-parse.y"
                 { (yyval.regno) = 2; }
#line 4293 "config/rx-parse.c"
    break;

  case 345: /* memex: DOT_UW  */
#line 1194 "./config/rx-parse.y"
                 { (yyval.regno) = 3; }
#line 4299 "config/rx-parse.c"
    break;

  case 346: /* bwl: %empty  */
#line 1197 "./config/rx-parse.y"
                { (yyval.regno) = LSIZE; }
#line 4305 "config/rx-parse.c"
    break;

  case 347: /* bwl: DOT_B  */
#line 1198 "./config/rx-parse.y"
                { (yyval.regno) = BSIZE; }
#line 4311 "config/rx-parse.c"
    break;

  case 348: /* bwl: DOT_W  */
#line 1199 "./config/rx-parse.y"
                { (yyval.regno) = WSIZE; }
#line 4317 "config/rx-parse.c"
    break;

  case 349: /* bwl: DOT_L  */
#line 1200 "./config/rx-parse.y"
                { (yyval.regno) = LSIZE; }
#line 4323 "config/rx-parse.c"
    break;

  case 350: /* bw: %empty  */
#line 1203 "./config/rx-parse.y"
                { (yyval.regno) = 1; }
#line 4329 "config/rx-parse.c"
    break;

  case 351: /* bw: DOT_B  */
#line 1204 "./config/rx-parse.y"
                { (yyval.regno) = 0; }
#line 4335 "config/rx-parse.c"
    break;

  case 352: /* bw: DOT_W  */
#line 1205 "./config/rx-parse.y"
                { (yyval.regno) = 1; }
#line 4341 "config/rx-parse.c"
    break;

  case 353: /* opt_l: %empty  */
#line 1208 "./config/rx-parse.y"
                {}
#line 4347 "config/rx-parse.c"
    break;

  case 354: /* opt_l: DOT_L  */
#line 1209 "./config/rx-parse.y"
                {}
#line 4353 "config/rx-parse.c"
    break;

  case 355: /* opt_b: %empty  */
#line 1212 "./config/rx-parse.y"
                {}
#line 4359 "config/rx-parse.c"
    break;

  case 356: /* opt_b: DOT_B  */
#line 1213 "./config/rx-parse.y"
                {}
#line 4365 "config/rx-parse.c"
    break;


#line 4369 "config/rx-parse.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 1216 "./config/rx-parse.y"

/* ====================================================================== */

static struct
{
  const char * string;
  int          token;
  int          val;
}
token_table[] =
{
  { "r0", REG, 0 },
  { "r1", REG, 1 },
  { "r2", REG, 2 },
  { "r3", REG, 3 },
  { "r4", REG, 4 },
  { "r5", REG, 5 },
  { "r6", REG, 6 },
  { "r7", REG, 7 },
  { "r8", REG, 8 },
  { "r9", REG, 9 },
  { "r10", REG, 10 },
  { "r11", REG, 11 },
  { "r12", REG, 12 },
  { "r13", REG, 13 },
  { "r14", REG, 14 },
  { "r15", REG, 15 },

  { "psw", CREG, 0 },
  { "pc", CREG, 1 },
  { "usp", CREG, 2 },
  { "fpsw", CREG, 3 },
  /* reserved */
  /* reserved */
  /* reserved */
  { "wr", CREG, 7 },

  { "bpsw", CREG, 8 },
  { "bpc", CREG, 9 },
  { "isp", CREG, 10 },
  { "fintv", CREG, 11 },
  { "intb", CREG, 12 },
  { "extb", CREG, 13 },

  { "pbp", CREG, 16 },
  { "pben", CREG, 17 },

  { "bbpsw", CREG, 24 },
  { "bbpc", CREG, 25 },

  { "dr0", DREG, 0 },
  { "dr1", DREG, 1 },
  { "dr2", DREG, 2 },
  { "dr3", DREG, 3 },
  { "dr4", DREG, 4 },
  { "dr5", DREG, 5 },
  { "dr6", DREG, 6 },
  { "dr7", DREG, 7 },
  { "dr8", DREG, 8 },
  { "dr9", DREG, 9 },
  { "dr10", DREG, 10 },
  { "dr11", DREG, 11 },
  { "dr12", DREG, 12 },
  { "dr13", DREG, 13 },
  { "dr14", DREG, 14 },
  { "dr15", DREG, 15 },
  
  { "drh0", DREGH, 0 },
  { "drh1", DREGH, 1 },
  { "drh2", DREGH, 2 },
  { "drh3", DREGH, 3 },
  { "drh4", DREGH, 4 },
  { "drh5", DREGH, 5 },
  { "drh6", DREGH, 6 },
  { "drh7", DREGH, 7 },
  { "drh8", DREGH, 8 },
  { "drh9", DREGH, 9 },
  { "drh10", DREGH, 10 },
  { "drh11", DREGH, 11 },
  { "drh12", DREGH, 12 },
  { "drh13", DREGH, 13 },
  { "drh14", DREGH, 14 },
  { "drh15", DREGH, 15 },

  { "drl0", DREGL, 0 },
  { "drl1", DREGL, 1 },
  { "drl2", DREGL, 2 },
  { "drl3", DREGL, 3 },
  { "drl4", DREGL, 4 },
  { "drl5", DREGL, 5 },
  { "drl6", DREGL, 6 },
  { "drl7", DREGL, 7 },
  { "drl8", DREGL, 8 },
  { "drl9", DREGL, 9 },
  { "drl10", DREGL, 10 },
  { "drl11", DREGL, 11 },
  { "drl12", DREGL, 12 },
  { "drl13", DREGL, 13 },
  { "drl14", DREGL, 14 },
  { "drl15", DREGL, 15 },

  { "DPSW", DCREG, 0 },
  { "DCMR", DCREG, 1 },
  { "DECNT", DCREG, 2 },
  { "DEPC", DCREG, 3 },
  { "DCR0", DCREG, 0 },
  { "DCR1", DCREG, 1 },
  { "DCR2", DCREG, 2 },
  { "DCR3", DCREG, 3 },
  
  { ".s", DOT_S, 0 },
  { ".b", DOT_B, 0 },
  { ".w", DOT_W, 0 },
  { ".l", DOT_L, 0 },
  { ".a", DOT_A , 0},
  { ".ub", DOT_UB, 0 },
  { ".uw", DOT_UW , 0},
  { ".d", DOT_D , 0},

  { "c", FLAG, 0 },
  { "z", FLAG, 1 },
  { "s", FLAG, 2 },
  { "o", FLAG, 3 },
  { "i", FLAG, 8 },
  { "u", FLAG, 9 },

  { "a0", ACC, 0 },
  { "a1", ACC, 1 },

#define OPC(x) { #x, x, IS_OPCODE }
  OPC(ABS),
  OPC(ADC),
  OPC(ADD),
  { "and", AND_, IS_OPCODE },
  OPC(BCLR),
  OPC(BCND),
  OPC(BFMOV),
  OPC(BFMOVZ),
  OPC(BMCND),
  OPC(BNOT),
  OPC(BRA),
  OPC(BRK),
  OPC(BSET),
  OPC(BSR),
  OPC(BTST),
  OPC(CLRPSW),
  OPC(CMP),
  OPC(DABS),
  OPC(DADD),
  OPC(DBT),
  OPC(DDIV),
  OPC(DIV),
  OPC(DIVU),
  OPC(DMOV),
  OPC(DMUL),
  OPC(DNEG),
  OPC(DPOPM),
  OPC(DPUSHM),
  OPC(DROUND),
  OPC(DSQRT),
  OPC(DSUB),
  OPC(DTOF),
  OPC(DTOI),
  OPC(DTOU),
  OPC(EDIV),
  OPC(EDIVU),
  OPC(EMACA),
  OPC(EMSBA),
  OPC(EMUL),
  OPC(EMULA),
  OPC(EMULU),
  OPC(FADD),
  OPC(FCMP),
  OPC(FDIV),
  OPC(FMUL),
  OPC(FREIT),
  OPC(FSQRT),
  OPC(FTOD),
  OPC(FTOU),
  OPC(FSUB),
  OPC(FTOI),
  OPC(INT),
  OPC(ITOD),
  OPC(ITOF),
  OPC(JMP),
  OPC(JSR),
  OPC(MVFACGU),
  OPC(MVFACHI),
  OPC(MVFACMI),
  OPC(MVFACLO),
  OPC(MVFC),
  OPC(MVFDC),
  OPC(MVFDR),
  OPC(MVTDC),
  OPC(MVTACGU),
  OPC(MVTACHI),
  OPC(MVTACLO),
  OPC(MVTC),
  OPC(MVTIPL),
  OPC(MACHI),
  OPC(MACLO),
  OPC(MACLH),
  OPC(MAX),
  OPC(MIN),
  OPC(MOV),
  OPC(MOVCO),
  OPC(MOVLI),
  OPC(MOVU),
  OPC(MSBHI),
  OPC(MSBLH),
  OPC(MSBLO),
  OPC(MUL),
  OPC(MULHI),
  OPC(MULLH),
  OPC(MULLO),
  OPC(MULU),
  OPC(NEG),
  OPC(NOP),
  OPC(NOT),
  OPC(OR),
  OPC(POP),
  OPC(POPC),
  OPC(POPM),
  OPC(PUSH),
  OPC(PUSHA),
  OPC(PUSHC),
  OPC(PUSHM),
  OPC(RACL),
  OPC(RACW),
  OPC(RDACL),
  OPC(RDACW),
  OPC(REIT),
  OPC(REVL),
  OPC(REVW),
  OPC(RMPA),
  OPC(ROLC),
  OPC(RORC),
  OPC(ROTL),
  OPC(ROTR),
  OPC(ROUND),
  OPC(RSTR),
  OPC(RTE),
  OPC(RTFI),
  OPC(RTS),
  OPC(RTSD),
  OPC(SAT),
  OPC(SATR),
  OPC(SAVE),
  OPC(SBB),
  OPC(SCCND),
  OPC(SCMPU),
  OPC(SETPSW),
  OPC(SHAR),
  OPC(SHLL),
  OPC(SHLR),
  OPC(SMOVB),
  OPC(SMOVF),
  OPC(SMOVU),
  OPC(SSTR),
  OPC(STNZ),
  OPC(STOP),
  OPC(STZ),
  OPC(SUB),
  OPC(SUNTIL),
  OPC(SWHILE),
  OPC(TST),
  OPC(UTOD),
  OPC(UTOF),
  OPC(WAIT),
  OPC(XCHG),
  OPC(XOR),
};

#define NUM_TOKENS (sizeof (token_table) / sizeof (token_table[0]))

static struct
{
  const char * string;
  int    token;
}
condition_opcode_table[] =
{
  { "b", BCND },
  { "bm", BMCND },
  { "sc", SCCND },
};

#define NUM_CONDITION_OPCODES (sizeof (condition_opcode_table) / sizeof (condition_opcode_table[0]))

struct condition_symbol
{
  const char * string;
  int    val;
};

static struct condition_symbol condition_table[] =
{
  { "z", 0 },
  { "eq", 0 },
  { "geu",  2 },
  { "c",  2 },
  { "gtu", 4 },
  { "pz", 6 },
  { "ge", 8 },
  { "gt", 10 },
  { "o",  12},
  /* always = 14 */
  { "nz", 1 },
  { "ne", 1 },
  { "ltu", 3 },
  { "nc", 3 },
  { "leu", 5 },
  { "n", 7 },
  { "lt", 9 },
  { "le", 11 },
  { "no", 13 },
  /* never = 15 */
};

static struct condition_symbol double_condition_table[] =
{
  { "un", 1 },
  { "eq", 2 },
  { "lt", 4 },
  { "le", 6 },
};

#define NUM_CONDITIONS (sizeof (condition_table) / sizeof (condition_table[0]))
#define NUM_DOUBLE_CONDITIONS (sizeof (double_condition_table) / sizeof (double_condition_table[0]))

void
rx_lex_init (char * beginning, char * ending)
{
  rx_init_start = beginning;
  rx_lex_start = beginning;
  rx_lex_end = ending;
  rx_in_brackets = 0;
  rx_last_token = 0;

  setbuf (stdout, 0);
}

static int
check_condition (const char * base, struct condition_symbol *t, unsigned int num)
{
  char * cp;
  unsigned int i;

  if ((unsigned) (rx_lex_end - rx_lex_start) < strlen (base) + 1)
    return 0;
  if (memcmp (rx_lex_start, base, strlen (base)))
    return 0;
  cp = rx_lex_start + strlen (base);
  for (i = 0; i < num; i ++)
    {
      if (strcasecmp (cp, t[i].string) == 0)
	{
	  rx_lval.regno = t[i].val;
	  return 1;
	}
    }
  return 0;
}

static int
rx_lex (void)
{
  unsigned int ci;
  char * save_input_pointer;

  while (ISSPACE (*rx_lex_start)
	 && rx_lex_start != rx_lex_end)
    rx_lex_start ++;

  rx_last_exp_start = rx_lex_start;

  if (rx_lex_start == rx_lex_end)
    return 0;

  if (ISALPHA (*rx_lex_start)
      || (rx_pid_register != -1 && memcmp (rx_lex_start, "%pidreg", 7) == 0)
      || (rx_gp_register != -1 && memcmp (rx_lex_start, "%gpreg", 6) == 0)
      || (*rx_lex_start == '.' && ISALPHA (rx_lex_start[1])))
    {
      unsigned int i;
      char * e;
      char save;

      for (e = rx_lex_start + 1;
	   e < rx_lex_end && ISALNUM (*e);
	   e ++)
	;
      save = *e;
      *e = 0;

      if (strcmp (rx_lex_start, "%pidreg") == 0)
	{
	  {
	    rx_lval.regno = rx_pid_register;
	    *e = save;
	    rx_lex_start = e;
	    rx_last_token = REG;
	    return REG;
	  }
	}

      if (strcmp (rx_lex_start, "%gpreg") == 0)
	{
	  {
	    rx_lval.regno = rx_gp_register;
	    *e = save;
	    rx_lex_start = e;
	    rx_last_token = REG;
	    return REG;
	  }
	}

      if (rx_last_token == 0)
	{
	  for (ci = 0; ci < NUM_CONDITION_OPCODES; ci ++)
	    if (check_condition (condition_opcode_table[ci].string,
				 condition_table, NUM_CONDITIONS))
	      {
		*e = save;
		rx_lex_start = e;
		rx_last_token = condition_opcode_table[ci].token;
		return condition_opcode_table[ci].token;
	      }
	  if  (check_condition ("dcmp", double_condition_table,
				NUM_DOUBLE_CONDITIONS))
	    {
	      *e = save;
	      rx_lex_start = e;
	      rx_last_token = DCMP;
	      return DCMP;
	    }
	}

      for (i = 0; i < NUM_TOKENS; i++)
	if (strcasecmp (rx_lex_start, token_table[i].string) == 0
	    && !(token_table[i].val == IS_OPCODE && rx_last_token != 0)
	    && !(token_table[i].token == FLAG && !need_flag))
	  {
	    rx_lval.regno = token_table[i].val;
	    *e = save;
	    rx_lex_start = e;
	    rx_last_token = token_table[i].token;
	    return token_table[i].token;
	  }
      *e = save;
    }

  if (rx_last_token == 0)
    {
      rx_last_token = UNKNOWN_OPCODE;
      return UNKNOWN_OPCODE;
    }

  if (rx_last_token == UNKNOWN_OPCODE)
    return 0;

  if (*rx_lex_start == '[')
    rx_in_brackets = 1;
  if (*rx_lex_start == ']')
    rx_in_brackets = 0;

  if (rx_in_brackets
      || rx_last_token == REG || rx_last_token == DREG || rx_last_token == DCREG
      || strchr ("[],#", *rx_lex_start))
    {
      rx_last_token = *rx_lex_start;
      return *rx_lex_start ++;
    }

  save_input_pointer = input_line_pointer;
  input_line_pointer = rx_lex_start;
  rx_lval.exp.X_md = 0;
  expression (&rx_lval.exp);

  /* We parse but ignore any :<size> modifier on expressions.  */
  if (*input_line_pointer == ':')
    {
      char *cp;

      for (cp  = input_line_pointer + 1; *cp && cp < rx_lex_end; cp++)
	if (!ISDIGIT (*cp))
	  break;
      if (cp > input_line_pointer+1)
	input_line_pointer = cp;
    }

  rx_lex_start = input_line_pointer;
  input_line_pointer = save_input_pointer;
  rx_last_token = EXPR;
  return EXPR;
}

int
rx_error (const char * str)
{
  int len;

  len = rx_last_exp_start - rx_init_start;

  as_bad ("%s", rx_init_start);
  as_bad ("%*s^ %s", len, "", str);
  return 0;
}

static int
rx_intop (expressionS exp, int nbits, int opbits)
{
  valueT v;
  valueT mask, msb;

  if (exp.X_op == O_big)
    {
      if (nbits == 32)
	return 1;
      if (exp.X_add_number == -1)
	return 0;
    }
  else if (exp.X_op != O_constant)
    return 0;
  v = exp.X_add_number;

  msb = (valueT) 1 << (opbits - 1);
  mask = (msb << 1) - 1;

  if ((v & msb) && ! (v & ~mask))
    v -= mask + 1;

  switch (nbits)
    {
    case 4:
      return v + 0x8 <= 0x7 + 0x8;
    case 5:
      return v + 0x10 <= 0xf + 0x10;
    case 8:
      return v + 0x80 <= 0x7f + 0x80;
    case 16:
      return v + 0x8000 <= 0x7fff + 0x8000;
    case 24:
      return v + 0x800000 <= 0x7fffff + 0x800000;
    case 32:
      return 1;
    default:
      printf ("rx_intop passed %d\n", nbits);
      abort ();
    }
  return 1;
}

static int
rx_uintop (expressionS exp, int nbits)
{
  valueT v;

  if (exp.X_op != O_constant)
    return 0;
  v = exp.X_add_number;

  switch (nbits)
    {
    case 4:
      return v <= 0xf;
    case 8:
      return v <= 0xff;
    case 16:
      return v <= 0xffff;
    case 24:
      return v <= 0xffffff;
    default:
      printf ("rx_uintop passed %d\n", nbits);
      abort ();
    }
  return 1;
}

static int
rx_disp3op (expressionS exp)
{
  valueT v;

  if (exp.X_op != O_constant)
    return 0;
  v = exp.X_add_number;
  if (v < 3 || v > 10)
    return 0;
  return 1;
}

static int
rx_disp5op (expressionS * exp, int msize)
{
  valueT v;

  if (exp->X_op != O_constant)
    return 0;
  v = exp->X_add_number;

  switch (msize)
    {
    case BSIZE:
      if (v <= 31)
	return 1;
      break;
    case WSIZE:
      if (v & 1)
	return 0;
      if (v <= 63)
	{
	  exp->X_add_number >>= 1;
	  return 1;
	}
      break;
    case LSIZE:
      if (v & 3)
	return 0;
      if (v <= 127)
	{
	  exp->X_add_number >>= 2;
	  return 1;
	}
      break;
    }
  return 0;
}

/* Just like the above, but allows a zero displacement.  */

static int
rx_disp5op0 (expressionS * exp, int msize)
{
  if (exp->X_op != O_constant)
    return 0;
  if (exp->X_add_number == 0)
    return 1;
  return rx_disp5op (exp, msize);
}

static int
exp_val (expressionS exp)
{
  if (exp.X_op != O_constant)
  {
    rx_error (_("constant expected"));
    return 0;
  }
  return exp.X_add_number;
}

static expressionS
zero_expr (void)
{
  /* Static, so program load sets it to all zeros, which is what we want.  */
  static expressionS zero;
  zero.X_op = O_constant;
  return zero;
}

static int
immediate (expressionS exp, int type, int pos, int bits)
{
  /* We will emit constants ourselves here, so negate them.  */
  if (type == RXREL_NEGATIVE && exp.X_op == O_constant)
    exp.X_add_number = - exp.X_add_number;
  if (type == RXREL_NEGATIVE_BORROW)
    {
      if (exp.X_op == O_constant)
	exp.X_add_number = - exp.X_add_number - 1;
      else
	rx_error (_("sbb cannot use symbolic immediates"));
    }

  if (pos >= 0 && rx_intop (exp, 8, bits))
    {
      rx_op (exp, 1, type);
      return 1;
    }
  else if (pos >= 0 && rx_intop (exp, 16, bits))
    {
      rx_op (exp, 2, type);
      return 2;
    }
  else if (pos >= 0 && rx_uintop (exp, 16) && bits == 16)
    {
      rx_op (exp, 2, type);
      return 2;
    }
  else if (pos >= 0 && rx_intop (exp, 24, bits))
    {
      rx_op (exp, 3, type);
      return 3;
    }
  else if (pos < 0 || rx_intop (exp, 32, bits))
    {
      rx_op (exp, 4, type);
      return 0;
    }
  else if (type == RXREL_SIGNED && pos >= 0)
    {
      /* This is a symbolic immediate, we will relax it later.  */
      rx_relax (RX_RELAX_IMM, pos);
      rx_op (exp, linkrelax ? 4 : 1, type);
      return 1;
    }
  else
    {
      /* Let the linker deal with it.  */
      rx_op (exp, 4, type);
      return 0;
    }
}

static int
displacement (expressionS exp, int msize)
{
  valueT val;
  int vshift = 0;

  if (exp.X_op == O_symbol
      && exp.X_md)
    {
      switch (exp.X_md)
	{
	case BFD_RELOC_GPREL16:
	  switch (msize)
	    {
	    case BSIZE:
	      exp.X_md = BFD_RELOC_RX_GPRELB;
	      break;
	    case WSIZE:
	      exp.X_md = BFD_RELOC_RX_GPRELW;
	      break;
	    case LSIZE:
	      exp.X_md = BFD_RELOC_RX_GPRELL;
	      break;
	    }
	  O2 (exp);
	  return 2;
	}
    }

  if (exp.X_op == O_subtract)
    {
      exp.X_md = BFD_RELOC_RX_DIFF;
      O2 (exp);
      return 2;
    }

  if (exp.X_op != O_constant)
    {
      rx_error (_("displacements must be constants"));
      return -1;
    }
  val = exp.X_add_number;

  if (val == 0)
    return 0;

  switch (msize)
    {
    case BSIZE:
      break;
    case WSIZE:
      if (val & 1)
	rx_error (_("word displacement not word-aligned"));
      vshift = 1;
      break;
    case LSIZE:
      if (val & 3)
	rx_error (_("long displacement not long-aligned"));
      vshift = 2;
      break;
    case DSIZE:
      if (val & 7)
	rx_error (_("double displacement not double-aligned"));
      vshift = 3;
      break;
    default:
      as_bad (_("displacement with unknown size (internal bug?)\n"));
      break;
    }

  val >>= vshift;
  exp.X_add_number = val;

  if (val <= 255 )
    {
      O1 (exp);
      return 1;
    }

  if (val <= 65535)
    {
      O2 (exp);
      return 2;
    }
  if ((offsetT) val < 0)
    rx_error (_("negative displacements not allowed"));
  else
    rx_error (_("displacement too large"));
  return -1;
}

static void
rtsd_immediate (expressionS exp)
{
  valueT val;

  if (exp.X_op != O_constant)
    {
      rx_error (_("rtsd size must be constant"));
      return;
    }
  val = exp.X_add_number;
  if (val & 3)
    rx_error (_("rtsd size must be multiple of 4"));

  if (val > 1020)
    rx_error (_("rtsd size must be 0..1020"));

  val >>= 2;
  exp.X_add_number = val;
  O1 (exp);
}

static void
rx_range (expressionS exp, int minv, int maxv)
{
  offsetT val;

  if (exp.X_op != O_constant)
    return;

  val = exp.X_add_number;
  if (val < minv || val > maxv)
    as_warn (_("Value %ld out of range %d..%d"), (long) val, minv, maxv);
}

static void
rx_check_float_support (void)
{
  if (rx_cpu == RX100 || rx_cpu == RX200)
    rx_error (_("target CPU type does not support floating point instructions"));
}

static void
rx_check_v2 (void)
{
  if (rx_cpu < RXV2)
    rx_error (_("target CPU type does not support v2 instructions"));
}

static void
rx_check_v3 (void)
{
  if (rx_cpu < RXV3)
    rx_error (_("target CPU type does not support v3 instructions"));
}

static void
rx_check_dfpu (void)
{
  if (rx_cpu != RXV3FPU)
    rx_error (_("target CPU type does not support double float instructions"));
}
