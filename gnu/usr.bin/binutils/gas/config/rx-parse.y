/* rx-parse.y  Renesas RX parser
   Copyright (C) 2008-2023 Free Software Foundation, Inc.

   This file is part of GAS, the GNU Assembler.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GAS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GAS; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */
%{

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

%}

%name-prefix="rx_"

%union {
  int regno;
  expressionS exp;
}

%type <regno> REG FLAG CREG BCND BMCND SCCND ACC DREG DREGH DREGL DCREG DCMP
%type <regno> flag bwl bw memex
%type <exp> EXPR disp

%token REG FLAG CREG ACC DREG DREGH DREGL DCREG

%token EXPR UNKNOWN_OPCODE IS_OPCODE

%token DOT_S DOT_B DOT_W DOT_L DOT_A DOT_UB DOT_UW DOT_D

%token ABS ADC ADD AND_
%token BCLR BCND BFMOV BFMOVZ BMCND BNOT BRA BRK BSET BSR BTST
%token CLRPSW CMP
%token DABS DADD DBT DCMP DDIV DIV DIVU DMOV DMUL DNEG
%token   DPOPM DPUSHM DROUND DSQRT DSUB DTOF DTOI DTOU
%token EDIV EDIVU EMACA EMSBA EMUL EMULA EMULU
%token FADD FCMP FDIV FMUL FREIT FSUB FSQRT FTOD FTOI FTOU
%token INT ITOD ITOF
%token JMP JSR
%token MACHI MACLH MACLO MAX MIN MOV MOVCO MOVLI MOVU MSBHI MSBLH MSBLO MUL
%token   MULHI MULLH MULLO MULU MVFACHI MVFACGU MVFACMI MVFACLO MVFC MVFDC
%token     MVFDR MVTACGU MVTACHI MVTACLO MVTC MVTDC MVTIPL
%token NEG NOP NOT
%token OR
%token POP POPC POPM PUSH PUSHA PUSHC PUSHM
%token RACL RACW RDACL RDACW REIT REVL REVW RMPA ROLC RORC ROTL ROTR ROUND
%token   RSTR RTE RTFI RTS RTSD
%token SAT SATR SAVE SBB SCCND SCMPU SETPSW SHAR SHLL SHLR SMOVB SMOVF
%token   SMOVU SSTR STNZ STOP STZ SUB SUNTIL SWHILE
%token TST
%token UTOD UTOF
%token WAIT
%token XCHG XOR

%%
/* ====================================================================== */

statement :

	  UNKNOWN_OPCODE
	  { as_bad (_("Unknown opcode: %s"), rx_init_start); }

/* ---------------------------------------------------------------------- */

	| BRK
	  { B1 (0x00); }

	| DBT
	  { B1 (0x01); }

	| RTS
	  { B1 (0x02); }

	| NOP
	  { B1 (0x03); }

/* ---------------------------------------------------------------------- */

	| BRA EXPR
	  { if (rx_disp3op ($2))
	      { B1 (0x08); rx_disp3 ($2, 5); }
	    else if (rx_intop ($2, 8, 8))
	      { B1 (0x2e); PC1 ($2); }
	    else if (rx_intop ($2, 16, 16))
	      { B1 (0x38); PC2 ($2); }
	    else if (rx_intop ($2, 24, 24))
	      { B1 (0x04); PC3 ($2); }
	    else
	      { rx_relax (RX_RELAX_BRANCH, 0);
		rx_linkrelax_branch ();
		/* We'll convert this to a longer one later if needed.  */
		B1 (0x08); rx_disp3 ($2, 5); } }

	| BRA DOT_A EXPR
	  { B1 (0x04); PC3 ($3); }

	| BRA DOT_S EXPR
	  { B1 (0x08); rx_disp3 ($3, 5); }

/* ---------------------------------------------------------------------- */

	| BSR EXPR
	  { if (rx_intop ($2, 16, 16))
	      { B1 (0x39); PC2 ($2); }
	    else if (rx_intop ($2, 24, 24))
	      { B1 (0x05); PC3 ($2); }
	    else
	      { rx_relax (RX_RELAX_BRANCH, 0);
		rx_linkrelax_branch ();
		B1 (0x39); PC2 ($2); } }
	| BSR DOT_A EXPR
	  { B1 (0x05), PC3 ($3); }

/* ---------------------------------------------------------------------- */

	| BCND DOT_S EXPR
	  { if ($1 == COND_EQ || $1 == COND_NE)
	      { B1 ($1 == COND_EQ ? 0x10 : 0x18); rx_disp3 ($3, 5); }
	    else
	      as_bad (_("Only BEQ and BNE may have .S")); }

/* ---------------------------------------------------------------------- */

	| BCND DOT_B EXPR
	  { B1 (0x20); F ($1, 4, 4); PC1 ($3); }

	| BRA DOT_B EXPR
	  { B1 (0x2e), PC1 ($3); }

/* ---------------------------------------------------------------------- */

	| BRA DOT_W EXPR
	  { B1 (0x38), PC2 ($3); }
	| BSR DOT_W EXPR
	  { B1 (0x39), PC2 ($3); }
	| BCND DOT_W EXPR
	  { if ($1 == COND_EQ || $1 == COND_NE)
	      { B1 ($1 == COND_EQ ? 0x3a : 0x3b); PC2 ($3); }
	    else
	      as_bad (_("Only BEQ and BNE may have .W")); }
	| BCND EXPR
	  { if ($1 == COND_EQ || $1 == COND_NE)
	      {
		rx_relax (RX_RELAX_BRANCH, 0);
		rx_linkrelax_branch ();
		B1 ($1 == COND_EQ ? 0x10 : 0x18); rx_disp3 ($2, 5);
	      }
	    else
	      {
		rx_relax (RX_RELAX_BRANCH, 0);
		/* This is because we might turn it into a
		   jump-over-jump long branch.  */
		rx_linkrelax_branch ();
	        B1 (0x20); F ($1, 4, 4); PC1 ($2);
	      } }

/* ---------------------------------------------------------------------- */

	| MOV DOT_B '#' EXPR ',' '[' REG ']'
	  { B2 (0xf8, 0x04); F ($7, 8, 4); IMMB ($4, 12);}

	| MOV DOT_W '#' EXPR ',' '[' REG ']'
          { B2 (0xf8, 0x01); F ($7, 8, 4); IMMW ($4, 12);}

	| MOV DOT_L '#' EXPR ',' '[' REG ']'
	  { B2 (0xf8, 0x02); F ($7, 8, 4); IMM ($4, 12);}

	| MOV DOT_B '#' EXPR ',' disp '[' REG ']'
	  /* rx_disp5op changes the value if it succeeds, so keep it last.  */
	  { if ($8 <= 7 && rx_uintop ($4, 8) && rx_disp5op0 (&$6, BSIZE))
	      { B2 (0x3c, 0); rx_field5s2 ($6); F ($8, 9, 3); O1 ($4); }
	    else
	      { B2 (0xf8, 0x04); F ($8, 8, 4); DSP ($6, 6, BSIZE); O1 ($4);
	      if ($4.X_op != O_constant && $4.X_op != O_big) rx_linkrelax_imm (12); } }

	| MOV DOT_W '#' EXPR ',' disp '[' REG ']'
	  { if ($8 <= 7 && rx_uintop ($4, 8) && rx_disp5op0 (&$6, WSIZE))
	      { B2 (0x3d, 0); rx_field5s2 ($6); F ($8, 9, 3); O1 ($4); }
	    else
	      { B2 (0xf8, 0x01); F ($8, 8, 4); DSP ($6, 6, WSIZE); IMMW ($4, 12); } }

	| MOV DOT_L '#' EXPR ',' disp '[' REG ']'
	  { if ($8 <= 7 && rx_uintop ($4, 8) && rx_disp5op0 (&$6, LSIZE))
	      { B2 (0x3e, 0); rx_field5s2 ($6); F ($8, 9, 3); O1 ($4); }
	    else
	      { B2 (0xf8, 0x02); F ($8, 8, 4); DSP ($6, 6, LSIZE); IMM ($4, 12); } }

/* ---------------------------------------------------------------------- */

	| RTSD '#' EXPR ',' REG '-' REG
	  { B2 (0x3f, 0); F ($5, 8, 4); F ($7, 12, 4); rtsd_immediate ($3);
	    if ($5 == 0)
	      rx_error (_("RTSD cannot pop R0"));
	    if ($5 > $7)
	      rx_error (_("RTSD first reg must be <= second reg")); }

/* ---------------------------------------------------------------------- */

	| CMP REG ',' REG
	  { B2 (0x47, 0); F ($2, 8, 4); F ($4, 12, 4); }

/* ---------------------------------------------------------------------- */

	| CMP disp '[' REG ']' DOT_UB ',' REG
	  { B2 (0x44, 0); F ($4, 8, 4); F ($8, 12, 4); DSP ($2, 6, BSIZE); }

	| CMP disp '[' REG ']' memex ',' REG
	  { B3 (MEMEX, 0x04, 0); F ($6, 8, 2);  F ($4, 16, 4); F ($8, 20, 4); DSP ($2, 14, sizemap[$6]); }

/* ---------------------------------------------------------------------- */

	| MOVU bw REG ',' REG
	  { B2 (0x5b, 0x00); F ($2, 5, 1); F ($3, 8, 4); F ($5, 12, 4); }

/* ---------------------------------------------------------------------- */

	| MOVU bw '[' REG ']' ',' REG
	  { B2 (0x58, 0x00); F ($2, 5, 1); F ($4, 8, 4); F ($7, 12, 4); }

	| MOVU bw EXPR '[' REG ']' ',' REG
	  { if ($5 <= 7 && $8 <= 7 && rx_disp5op (&$3, $2))
	      { B2 (0xb0, 0); F ($2, 4, 1); F ($5, 9, 3); F ($8, 13, 3); rx_field5s ($3); }
	    else
	      { B2 (0x58, 0x00); F ($2, 5, 1); F ($5, 8, 4); F ($8, 12, 4); DSP ($3, 6, $2); } }

/* ---------------------------------------------------------------------- */

	| SUB '#' EXPR ',' REG
	  { if (rx_uintop ($3, 4))
	      { B2 (0x60, 0); FE ($3, 8, 4); F ($5, 12, 4); }
	    else
	      /* This is really an add, but we negate the immediate.  */
	      { B2 (0x70, 0); F ($5, 8, 4); F ($5, 12, 4); NIMM ($3, 6); } }

	| CMP '#' EXPR ',' REG
	  { if (rx_uintop ($3, 4))
	      { B2 (0x61, 0); FE ($3, 8, 4); F ($5, 12, 4); }
	    else if (rx_uintop ($3, 8))
	      { B2 (0x75, 0x50); F ($5, 12, 4); UO1 ($3); }
	    else
	      { B2 (0x74, 0x00); F ($5, 12, 4); IMM ($3, 6); } }

	| ADD '#' EXPR ',' REG
	  { if (rx_uintop ($3, 4))
	      { B2 (0x62, 0); FE ($3, 8, 4); F ($5, 12, 4); }
	    else
	      { B2 (0x70, 0); F ($5, 8, 4); F ($5, 12, 4); IMM ($3, 6); } }

	| MUL '#' EXPR ',' REG
	  { if (rx_uintop ($3, 4))
	      { B2 (0x63, 0); FE ($3, 8, 4); F ($5, 12, 4); }
	    else
	      { B2 (0x74, 0x10); F ($5, 12, 4); IMM ($3, 6); } }

	| AND_ '#' EXPR ',' REG
	  { if (rx_uintop ($3, 4))
	      { B2 (0x64, 0); FE ($3, 8, 4); F ($5, 12, 4); }
	    else
	      { B2 (0x74, 0x20); F ($5, 12, 4); IMM ($3, 6); } }

	| OR '#' EXPR ',' REG
	  { if (rx_uintop ($3, 4))
	      { B2 (0x65, 0); FE ($3, 8, 4); F ($5, 12, 4); }
	    else
	      { B2 (0x74, 0x30); F ($5, 12, 4); IMM ($3, 6); } }

	| MOV DOT_L '#' EXPR ',' REG
	  { if (rx_uintop ($4, 4))
	      { B2 (0x66, 0); FE ($4, 8, 4); F ($6, 12, 4); }
	    else if (rx_uintop ($4, 8))
	      { B2 (0x75, 0x40); F ($6, 12, 4); UO1 ($4); }
	    else
	      { B2 (0xfb, 0x02); F ($6, 8, 4); IMM ($4, 12); } }

	| MOV '#' EXPR ',' REG
	  { if (rx_uintop ($3, 4))
	      { B2 (0x66, 0); FE ($3, 8, 4); F ($5, 12, 4); }
	    else if (rx_uintop ($3, 8))
	      { B2 (0x75, 0x40); F ($5, 12, 4); UO1 ($3); }
	    else
	      { B2 (0xfb, 0x02); F ($5, 8, 4); IMM ($3, 12); } }

/* ---------------------------------------------------------------------- */

	| RTSD '#' EXPR
	  { B1 (0x67); rtsd_immediate ($3); }

/* ---------------------------------------------------------------------- */

	| SHLR { sub_op = 0; } op_shift
	| SHAR { sub_op = 1; } op_shift
	| SHLL { sub_op = 2; } op_shift

/* ---------------------------------------------------------------------- */

	| PUSHM REG '-' REG
	  {
	    if ($2 == $4)
	      { B2 (0x7e, 0x80); F (LSIZE, 10, 2); F ($2, 12, 4); }
	    else
	     { B2 (0x6e, 0); F ($2, 8, 4); F ($4, 12, 4); }
	    if ($2 == 0)
	      rx_error (_("PUSHM cannot push R0"));
	    if ($2 > $4)
	      rx_error (_("PUSHM first reg must be <= second reg")); }

/* ---------------------------------------------------------------------- */

	| POPM REG '-' REG
	  {
	    if ($2 == $4)
	      { B2 (0x7e, 0xb0); F ($2, 12, 4); }
	    else
	      { B2 (0x6f, 0); F ($2, 8, 4); F ($4, 12, 4); }
	    if ($2 == 0)
	      rx_error (_("POPM cannot pop R0"));
	    if ($2 > $4)
	      rx_error (_("POPM first reg must be <= second reg")); }

/* ---------------------------------------------------------------------- */

	| ADD '#' EXPR ',' REG ',' REG
	  { B2 (0x70, 0x00); F ($5, 8, 4); F ($7, 12, 4); IMM ($3, 6); }

/* ---------------------------------------------------------------------- */

	| INT '#' EXPR
	  { B2(0x75, 0x60), UO1 ($3); }

/* ---------------------------------------------------------------------- */

	| BSET '#' EXPR ',' REG
	  { B2 (0x78, 0); FE ($3, 7, 5); F ($5, 12, 4); }
	| BCLR '#' EXPR ',' REG
	  { B2 (0x7a, 0); FE ($3, 7, 5); F ($5, 12, 4); }

/* ---------------------------------------------------------------------- */

	| BTST '#' EXPR ',' REG
	  { B2 (0x7c, 0x00); FE ($3, 7, 5); F ($5, 12, 4); }

/* ---------------------------------------------------------------------- */

	| SAT REG
	  { B2 (0x7e, 0x30); F ($2, 12, 4); }
	| RORC REG
	  { B2 (0x7e, 0x40); F ($2, 12, 4); }
	| ROLC REG
	  { B2 (0x7e, 0x50); F ($2, 12, 4); }

/* ---------------------------------------------------------------------- */

	| PUSH bwl REG
	  { B2 (0x7e, 0x80); F ($2, 10, 2); F ($3, 12, 4); }

/* ---------------------------------------------------------------------- */

	| POP REG
	  { B2 (0x7e, 0xb0); F ($2, 12, 4); }

/* ---------------------------------------------------------------------- */

	| PUSHC CREG
	  { if ($2 == 13)
	      { rx_check_v2 (); }
	    if ($2 < 16)
	      { B2 (0x7e, 0xc0); F ($2, 12, 4); }
	    else
	      as_bad (_("PUSHC can only push the first 16 control registers")); }

/* ---------------------------------------------------------------------- */

	| POPC CREG
	  { if ($2 == 13)
	    { rx_check_v2 (); }
	    if ($2 < 16)
	      { B2 (0x7e, 0xe0); F ($2, 12, 4); }
	    else
	      as_bad (_("POPC can only pop the first 16 control registers")); }

/* ---------------------------------------------------------------------- */

	| SETPSW flag
	  { B2 (0x7f, 0xa0); F ($2, 12, 4); }
	| CLRPSW flag
	  { B2 (0x7f, 0xb0); F ($2, 12, 4); }

/* ---------------------------------------------------------------------- */

	| JMP REG
	  { B2 (0x7f, 0x00); F ($2, 12, 4); }
	| JSR REG
	  { B2 (0x7f, 0x10); F ($2, 12, 4); }
	| BRA opt_l REG
	  { B2 (0x7f, 0x40); F ($3, 12, 4); }
	| BSR opt_l REG
	  { B2 (0x7f, 0x50); F ($3, 12, 4); }

/* ---------------------------------------------------------------------- */

	| SCMPU
	  { B2 (0x7f, 0x83); rx_note_string_insn_use (); }
	| SMOVU
	  { B2 (0x7f, 0x87); rx_note_string_insn_use (); }
	| SMOVB
	  { B2 (0x7f, 0x8b); rx_note_string_insn_use (); }
	| SMOVF
	  { B2 (0x7f, 0x8f); rx_note_string_insn_use (); }

/* ---------------------------------------------------------------------- */

	| SUNTIL bwl
	  { B2 (0x7f, 0x80); F ($2, 14, 2); rx_note_string_insn_use (); }
	| SWHILE bwl
	  { B2 (0x7f, 0x84); F ($2, 14, 2); rx_note_string_insn_use (); }
	| SSTR bwl
	  { B2 (0x7f, 0x88); F ($2, 14, 2); }

/* ---------------------------------------------------------------------- */

	| RMPA bwl
	  { B2 (0x7f, 0x8c); F ($2, 14, 2); rx_note_string_insn_use (); }

/* ---------------------------------------------------------------------- */

	| RTFI
	  { B2 (0x7f, 0x94); }
	| RTE
	  { B2 (0x7f, 0x95); }
	| WAIT
	  { B2 (0x7f, 0x96); }
	| SATR
	  { B2 (0x7f, 0x93); }

/* ---------------------------------------------------------------------- */

	| MVTIPL '#' EXPR
	  { B3 (0x75, 0x70, 0x00); FE ($3, 20, 4); }

/* ---------------------------------------------------------------------- */

	/* rx_disp5op changes the value if it succeeds, so keep it last.  */
	| MOV bwl REG ',' EXPR '[' REG ']'
	  { if ($3 <= 7 && $7 <= 7 && rx_disp5op (&$5, $2))
	      { B2 (0x80, 0); F ($2, 2, 2); F ($7, 9, 3); F ($3, 13, 3); rx_field5s ($5); }
	    else
	      { B2 (0xc3, 0x00); F ($2, 2, 2); F ($7, 8, 4); F ($3, 12, 4); DSP ($5, 4, $2); }}

/* ---------------------------------------------------------------------- */

	| MOV bwl EXPR '[' REG ']' ',' REG
	  { if ($5 <= 7 && $8 <= 7 && rx_disp5op (&$3, $2))
	      { B2 (0x88, 0); F ($2, 2, 2); F ($5, 9, 3); F ($8, 13, 3); rx_field5s ($3); }
	    else
	      { B2 (0xcc, 0x00); F ($2, 2, 2); F ($5, 8, 4); F ($8, 12, 4); DSP ($3, 6, $2); } }

/* ---------------------------------------------------------------------- */

	/* MOV a,b - if a is a reg and b is mem, src and dest are
	   swapped.  */

	/* We don't use "disp" here because it causes a shift/reduce
	   conflict with the other displacement-less patterns.  */

	| MOV bwl REG ',' '[' REG ']'
	  { B2 (0xc3, 0x00); F ($2, 2, 2); F ($6, 8, 4); F ($3, 12, 4); }

/* ---------------------------------------------------------------------- */

	| MOV bwl '[' REG ']' ',' disp '[' REG ']'
	  { B2 (0xc0, 0); F ($2, 2, 2); F ($4, 8, 4); F ($9, 12, 4); DSP ($7, 4, $2); }

/* ---------------------------------------------------------------------- */

	| MOV bwl EXPR '[' REG ']' ',' disp '[' REG ']'
	  { B2 (0xc0, 0x00); F ($2, 2, 2); F ($5, 8, 4); F ($10, 12, 4); DSP ($3, 6, $2); DSP ($8, 4, $2); }

/* ---------------------------------------------------------------------- */

	| MOV bwl REG ',' REG
	  { B2 (0xcf, 0x00); F ($2, 2, 2); F ($3, 8, 4); F ($5, 12, 4); }

/* ---------------------------------------------------------------------- */

	| MOV bwl '[' REG ']' ',' REG
	  { B2 (0xcc, 0x00); F ($2, 2, 2); F ($4, 8, 4); F ($7, 12, 4); }

/* ---------------------------------------------------------------------- */

	| BSET '#' EXPR ',' disp '[' REG ']' DOT_B
	  { B2 (0xf0, 0x00); F ($7, 8, 4); FE ($3, 13, 3); DSP ($5, 6, BSIZE); }
	| BCLR '#' EXPR ',' disp '[' REG ']' DOT_B
	  { B2 (0xf0, 0x08); F ($7, 8, 4); FE ($3, 13, 3); DSP ($5, 6, BSIZE); }
	| BTST '#' EXPR ',' disp '[' REG ']' DOT_B
	  { B2 (0xf4, 0x00); F ($7, 8, 4); FE ($3, 13, 3); DSP ($5, 6, BSIZE); }

/* ---------------------------------------------------------------------- */

	| PUSH bwl disp '[' REG ']'
	  { B2 (0xf4, 0x08); F ($2, 14, 2); F ($5, 8, 4); DSP ($3, 6, $2); }

/* ---------------------------------------------------------------------- */

	| SBB   { sub_op = 0; } op_dp20_rm_l
	| NEG   { sub_op = 1; sub_op2 = 1; } op_dp20_rr
	| ADC   { sub_op = 2; } op_dp20_rim_l
	| ABS   { sub_op = 3; sub_op2 = 2; } op_dp20_rr
	| MAX   { sub_op = 4; } op_dp20_rim
	| MIN   { sub_op = 5; } op_dp20_rim
	| EMUL  { sub_op = 6; } op_dp20_i
	| EMULU { sub_op = 7; } op_dp20_i
	| DIV   { sub_op = 8; } op_dp20_rim
	| DIVU  { sub_op = 9; } op_dp20_rim
	| TST   { sub_op = 12; } op_dp20_rim
	| XOR   { sub_op = 13; } op_xor
	| NOT   { sub_op = 14; sub_op2 = 0; } op_dp20_rr
	| STZ   { sub_op = 14; sub_op2 = 0; } op_dp20_ri
	| STNZ  { sub_op = 15; sub_op2 = 1; } op_dp20_ri

/* ---------------------------------------------------------------------- */

	| EMUL  { sub_op = 6; } op_xchg
	| EMULU { sub_op = 7; } op_xchg
	| XCHG  { sub_op = 16; } op_xchg
	| ITOF  { sub_op = 17; } op_xchg
	| UTOF  { sub_op = 21; } op_xchg

/* ---------------------------------------------------------------------- */

	| BSET REG ',' REG
	  { id24 (1, 0x63, 0x00); F ($4, 16, 4); F ($2, 20, 4); }
	| BCLR REG ',' REG
	  { id24 (1, 0x67, 0x00); F ($4, 16, 4); F ($2, 20, 4); }
	| BTST REG ',' REG
	  { id24 (1, 0x6b, 0x00); F ($4, 16, 4); F ($2, 20, 4); }
	| BNOT REG ',' REG
	  { id24 (1, 0x6f, 0x00); F ($4, 16, 4); F ($2, 20, 4); }

	| BSET REG ',' disp '[' REG ']' opt_b
	  { id24 (1, 0x60, 0x00); F ($6, 16, 4); F ($2, 20, 4); DSP ($4, 14, BSIZE); }
	| BCLR REG ',' disp '[' REG ']' opt_b
	  { id24 (1, 0x64, 0x00); F ($6, 16, 4); F ($2, 20, 4); DSP ($4, 14, BSIZE); }
	| BTST REG ',' disp '[' REG ']' opt_b
	  { id24 (1, 0x68, 0x00); F ($6, 16, 4); F ($2, 20, 4); DSP ($4, 14, BSIZE); }
	| BNOT REG ',' disp '[' REG ']' opt_b
	  { id24 (1, 0x6c, 0x00); F ($6, 16, 4); F ($2, 20, 4); DSP ($4, 14, BSIZE); }

/* ---------------------------------------------------------------------- */

	| FSUB  { sub_op = 0; } float3_op
	| FCMP  { sub_op = 1; } float2_op
	| FADD  { sub_op = 2; } float3_op
	| FMUL  { sub_op = 3; } float3_op
	| FDIV  { sub_op = 4; } float2_op
	| FSQRT { sub_op = 8; } float2_op_ni
	| FTOI  { sub_op = 5; } float2_op_ni
	| FTOU  { sub_op = 9; } float2_op_ni
	| ROUND { sub_op = 6; } float2_op_ni

/* ---------------------------------------------------------------------- */


/* ---------------------------------------------------------------------- */

	| SCCND DOT_L REG
	  { id24 (1, 0xdb, 0x00); F ($1, 20, 4); F ($3, 16, 4); }
	| SCCND bwl disp '[' REG ']'
	  { id24 (1, 0xd0, 0x00); F ($1, 20, 4); F ($2, 12, 2); F ($5, 16, 4); DSP ($3, 14, $2); }

/* ---------------------------------------------------------------------- */

	| BMCND '#' EXPR ',' disp '[' REG ']' opt_b
	  { id24 (1, 0xe0, 0x00); F ($1, 20, 4); FE ($3, 11, 3);
	      F ($7, 16, 4); DSP ($5, 14, BSIZE); }

/* ---------------------------------------------------------------------- */

	| BNOT '#' EXPR ',' disp '[' REG ']' opt_b
	  { id24 (1, 0xe0, 0x0f); FE ($3, 11, 3); F ($7, 16, 4);
	      DSP ($5, 14, BSIZE); }

/* ---------------------------------------------------------------------- */

	| MULHI REG ',' REG
	  { id24 (2, 0x00, 0x00); F ($2, 16, 4); F ($4, 20, 4); }
	| MULHI REG ',' REG ',' ACC
	  { rx_check_v2 (); id24 (2, 0x00, 0x00); F ($2, 16, 4); F ($4, 20, 4); F ($6, 12, 1); }
	| MULLO REG ',' REG
	  { id24 (2, 0x01, 0x00); F ($2, 16, 4); F ($4, 20, 4); }
	| MULLO REG ',' REG ',' ACC
	  { rx_check_v2 (); id24 (2, 0x01, 0x00); F ($2, 16, 4); F ($4, 20, 4); F ($6, 12, 1); }
	| MACHI REG ',' REG
	  { id24 (2, 0x04, 0x00); F ($2, 16, 4); F ($4, 20, 4); }
	| MACHI REG ',' REG ',' ACC
	  { rx_check_v2 (); id24 (2, 0x04, 0x00); F ($2, 16, 4); F ($4, 20, 4); F ($6, 12, 1); }
	| MACLO REG ',' REG
	  { id24 (2, 0x05, 0x00); F ($2, 16, 4); F ($4, 20, 4); }
	|  MACLO REG ',' REG ',' ACC
	  { rx_check_v2 (); id24 (2, 0x05, 0x00); F ($2, 16, 4); F ($4, 20, 4); F ($6, 12, 1); }

/* ---------------------------------------------------------------------- */

	/* We don't have syntax for these yet.  */
	| MVTACHI REG
	  { id24 (2, 0x17, 0x00); F ($2, 20, 4); }
	|  MVTACHI REG ',' ACC
	  { rx_check_v2 (); id24 (2, 0x17, 0x00); F ($2, 20, 4); F ($4, 16, 1); }
	| MVTACLO REG
	  { id24 (2, 0x17, 0x10); F ($2, 20, 4); }
	| MVTACLO REG ',' ACC
	  { rx_check_v2 (); id24 (2, 0x17, 0x10); F ($2, 20, 4); F ($4, 16, 1); }
	| MVFACHI REG
	  { id24 (2, 0x1f, 0x00); F ($2, 20, 4); }
	| MVFACHI { sub_op = 0; } mvfa_op
	| MVFACMI REG
	  { id24 (2, 0x1f, 0x20); F ($2, 20, 4); }
	| MVFACMI { sub_op = 2; } mvfa_op
	| MVFACLO REG
	  { id24 (2, 0x1f, 0x10); F ($2, 20, 4); }
	| MVFACLO { sub_op = 1; } mvfa_op
	| RACW '#' EXPR
	  { id24 (2, 0x18, 0x00);
	    if (rx_uintop ($3, 4) && exp_val($3) == 1)
	      ;
	    else if (rx_uintop ($3, 4) && exp_val($3) == 2)
	      F (1, 19, 1);
	    else
	      as_bad (_("RACW expects #1 or #2"));}
	| RACW '#' EXPR ',' ACC
	    { rx_check_v2 (); id24 (2, 0x18, 0x00); F ($5, 16, 1);
	    if (rx_uintop ($3, 4) && exp_val($3) == 1)
	      ;
	    else if (rx_uintop ($3, 4) && exp_val($3) == 2)
	      F (1, 19, 1);
	    else
	      as_bad (_("RACW expects #1 or #2"));}

/* ---------------------------------------------------------------------- */

	| MOV bwl REG ',' '[' REG '+' ']'
	  { id24 (2, 0x20, 0); F ($2, 14, 2); F ($6, 16, 4); F ($3, 20, 4); }
	| MOV bwl REG ',' '[' '-' REG ']'
	  { id24 (2, 0x24, 0); F ($2, 14, 2); F ($7, 16, 4); F ($3, 20, 4); }

/* ---------------------------------------------------------------------- */

	| MOV bwl '[' REG '+' ']' ',' REG
	  { id24 (2, 0x28, 0); F ($2, 14, 2); F ($4, 16, 4); F ($8, 20, 4); }
	| MOV bwl '[' '-' REG ']' ',' REG
	  { id24 (2, 0x2c, 0); F ($2, 14, 2); F ($5, 16, 4); F ($8, 20, 4); }

/* ---------------------------------------------------------------------- */

	| MOVU bw '[' REG '+' ']' ','  REG
	  { id24 (2, 0x38, 0); F ($2, 15, 1); F ($4, 16, 4); F ($8, 20, 4); }
	| MOVU bw '[' '-' REG ']' ',' REG
	  { id24 (2, 0x3c, 0); F ($2, 15, 1); F ($5, 16, 4); F ($8, 20, 4); }

/* ---------------------------------------------------------------------- */

	| ROTL { sub_op = 6; } op_shift_rot
	| ROTR { sub_op = 4; } op_shift_rot
	| REVW { sub_op = 5; } op_shift_rot
	| REVL { sub_op = 7; } op_shift_rot

/* ---------------------------------------------------------------------- */

	| MVTC REG ',' CREG
	  { if ($4 == 13)
	      rx_check_v2 ();
	  id24 (2, 0x68, 0x00); F ($4 % 16, 20, 4); F ($4 / 16, 15, 1);
	    F ($2, 16, 4); }

/* ---------------------------------------------------------------------- */

	| MVFC CREG ',' REG
	  { if ($2 == 13)
	    rx_check_v2 ();
	  id24 (2, 0x6a, 0); F ($2, 15, 5); F ($4, 20, 4); }

/* ---------------------------------------------------------------------- */

	| ROTL '#' EXPR ',' REG
	  { id24 (2, 0x6e, 0); FE ($3, 15, 5); F ($5, 20, 4); }
	| ROTR '#' EXPR ',' REG
	  { id24 (2, 0x6c, 0); FE ($3, 15, 5); F ($5, 20, 4); }

/* ---------------------------------------------------------------------- */

	| MVTC '#' EXPR ',' CREG
	  { if ($5 == 13)
	      rx_check_v2 ();
	    id24 (2, 0x73, 0x00); F ($5, 19, 5); IMM ($3, 12); }

/* ---------------------------------------------------------------------- */

	| BMCND '#' EXPR ',' REG
	  { id24 (2, 0xe0, 0x00); F ($1, 16, 4); FE ($3, 11, 5);
	      F ($5, 20, 4); }

/* ---------------------------------------------------------------------- */

	| BNOT '#' EXPR ',' REG
	  { id24 (2, 0xe0, 0xf0); FE ($3, 11, 5); F ($5, 20, 4); }

/* ---------------------------------------------------------------------- */

	| MOV bwl REG ',' '[' REG ',' REG ']'
	  { id24 (3, 0x00, 0); F ($2, 10, 2); F ($6, 12, 4); F ($8, 16, 4); F ($3, 20, 4); }

	| MOV bwl '[' REG ',' REG ']' ',' REG
	  { id24 (3, 0x40, 0); F ($2, 10, 2); F ($4, 12, 4); F ($6, 16, 4); F ($9, 20, 4); }

	| MOVU bw '[' REG ',' REG ']' ',' REG
	  { id24 (3, 0xc0, 0); F ($2, 10, 2); F ($4, 12, 4); F ($6, 16, 4); F ($9, 20, 4); }

/* ---------------------------------------------------------------------- */

	| SUB { sub_op = 0; } op_subadd
	| ADD { sub_op = 2; } op_subadd
	| MUL { sub_op = 3; } op_subadd
	| AND_ { sub_op = 4; } op_subadd
	| OR  { sub_op = 5; } op_subadd

/* ---------------------------------------------------------------------- */
/* There is no SBB #imm so we fake it with ADC.  */

	| SBB '#' EXPR ',' REG
	  { id24 (2, 0x70, 0x20); F ($5, 20, 4); NBIMM ($3, 12); }

/* ---------------------------------------------------------------------- */

	| MOVCO REG ',' '[' REG ']'
	  { rx_check_v2 (); B3 (0xfd, 0x27, 0x00); F ($5, 16, 4); F ($2, 20, 4); }

/* ---------------------------------------------------------------------- */

	| MOVLI '[' REG ']' ',' REG
	  { rx_check_v2 (); B3 (0xfd, 0x2f, 0x00); F ($3, 16, 4); F ($6, 20, 4); }

/* ---------------------------------------------------------------------- */

	| EMACA REG ',' REG ',' ACC
	  { rx_check_v2 (); id24 (2, 0x07, 0x00); F ($2, 16, 4); F ($4, 20, 4); F ($6, 12, 1); }
	| EMSBA REG ',' REG ',' ACC
	  { rx_check_v2 (); id24 (2, 0x47, 0x00); F ($2, 16, 4); F ($4, 20, 4); F ($6, 12, 1); }
	| EMULA REG ',' REG ',' ACC
	  { rx_check_v2 (); id24 (2, 0x03, 0x00); F ($2, 16, 4); F ($4, 20, 4); F ($6, 12, 1); }
	| MACLH REG ',' REG ',' ACC
	  { rx_check_v2 (); id24 (2, 0x06, 0x00); F ($2, 16, 4); F ($4, 20, 4); F ($6, 12, 1); }
	| MSBHI REG ',' REG ',' ACC
	  { rx_check_v2 (); id24 (2, 0x44, 0x00); F ($2, 16, 4); F ($4, 20, 4); F ($6, 12, 1); }
	| MSBLH REG ',' REG ',' ACC
	  { rx_check_v2 (); id24 (2, 0x46, 0x00); F ($2, 16, 4); F ($4, 20, 4); F ($6, 12, 1); }
	| MSBLO REG ',' REG ',' ACC
	  { rx_check_v2 (); id24 (2, 0x45, 0x00); F ($2, 16, 4); F ($4, 20, 4); F ($6, 12, 1); }
	| MULLH REG ',' REG ',' ACC
	  { rx_check_v2 (); id24 (2, 0x02, 0x00); F ($2, 16, 4); F ($4, 20, 4); F ($6, 12, 1); }
	| MVFACGU { sub_op = 3; } mvfa_op
	| MVTACGU REG ',' ACC
	  { rx_check_v2 (); id24 (2, 0x17, 0x30); F ($4, 16, 1); F ($2, 20, 4); }
	| RACL '#' EXPR ',' ACC
	{ rx_check_v2 (); id24 (2, 0x19, 0x00); F ($5, 16, 1);
	    if (rx_uintop ($3, 4) && $3.X_add_number == 1)
	      ;
	    else if (rx_uintop ($3, 4) && $3.X_add_number == 2)
	      F (1, 19, 1);
	    else
	      as_bad (_("RACL expects #1 or #2"));}
	| RDACL '#' EXPR ',' ACC
	{ rx_check_v2 (); id24 (2, 0x19, 0x40); F ($5, 16, 1);
	    if (rx_uintop ($3, 4) && $3.X_add_number == 1)
	      ;
	    else if (rx_uintop ($3, 4) && $3.X_add_number == 2)
	      F (1, 19, 1);
	    else
	      as_bad (_("RDACL expects #1 or #2"));}
	| RDACW '#' EXPR ',' ACC
	{ rx_check_v2 (); id24 (2, 0x18, 0x40); F ($5, 16, 1);
	    if (rx_uintop ($3, 4) && $3.X_add_number == 1)
	      ;
	    else if (rx_uintop ($3, 4) && $3.X_add_number == 2)
	      F (1, 19, 1);
	    else
	      as_bad (_("RDACW expects #1 or #2"));}

/* ---------------------------------------------------------------------- */
	| BFMOV { rx_check_v3(); sub_op = 1; } op_bfield
	| BFMOVZ { rx_check_v3(); sub_op = 0; } op_bfield

/* ---------------------------------------------------------------------- */
	| RSTR { rx_check_v3(); sub_op = 1; } op_save_rstr
	| SAVE { rx_check_v3(); sub_op = 0; } op_save_rstr

/* ---------------------------------------------------------------------- */
	| DABS { rx_check_dfpu(); sub_op = 0x0c; sub_op2 = 0x01; } double2_op
	| DNEG { rx_check_dfpu(); sub_op = 0x0c; sub_op2 = 0x02; } double2_op
	| DROUND { rx_check_dfpu(); sub_op = 0x0d; sub_op2 = 0x0d; } double2_op
	| DSQRT { rx_check_dfpu(); sub_op = 0x0d; sub_op2 = 0x00; } double2_op
	| DTOF { rx_check_dfpu(); sub_op = 0x0d; sub_op2 = 0x0c; } double2_op
	| DTOI { rx_check_dfpu(); sub_op = 0x0d; sub_op2 = 0x08;} double2_op
	| DTOU { rx_check_dfpu(); sub_op = 0x0d; sub_op2 = 0x09; } double2_op
	| DADD { rx_check_dfpu(); sub_op = 0x00; } double3_op
	| DDIV { rx_check_dfpu(); sub_op = 0x05; } double3_op
	| DMUL { rx_check_dfpu(); sub_op = 0x02; } double3_op
	| DSUB { rx_check_dfpu(); sub_op = 0x01; } double3_op
	| DCMP DREG ',' DREG { rx_check_dfpu();
	    B4(0x76, 0x90, 0x08, 0x00); F($1, 24, 4); F($2, 28, 4); F($4, 16, 4); }
	| DMOV DOT_D REG ',' DREGH
	{ rx_check_dfpu();
	  B4(0xfd, 0x77, 0x80, 0x03); F($3, 20, 4); F($5, 24, 4); }
	| DMOV DOT_L REG ',' DREGH
	{ rx_check_dfpu();
	  B4(0xfd, 0x77, 0x80, 0x02); F($3, 20, 4); F($5, 24, 4); }
	| DMOV DOT_L REG ',' DREGL
	{ rx_check_dfpu();
	  B4(0xfd, 0x77, 0x80, 0x00); F($3, 20, 4); F($5, 24, 4); }
	| DMOV DOT_L DREGH ',' REG
	{ rx_check_dfpu();
	  B4(0xfd, 0x75, 0x80, 0x02); F($3, 24, 4); F($5, 20, 4); }
	| DMOV DOT_L DREGL ',' REG
	{ rx_check_dfpu();
	  B4(0xfd, 0x75, 0x80, 0x00); F($3, 24, 4); F($5, 20, 4); }
	| DMOV DOT_D DREG ',' DREG
	{ rx_check_dfpu();
	  B4(0x76, 0x90, 0x0c, 0x00); F($3, 16, 4); F($5, 24, 4); }
	| DMOV DOT_D DREG ',' '[' REG ']'
	{ rx_check_dfpu();
	  B4(0xfc, 0x78, 0x08, 0x00); F($6, 16, 4); F($3, 24, 4); }
	| DMOV DOT_D DREG ',' disp '[' REG ']'
	{ rx_check_dfpu();
	  B3(0xfc, 0x78, 0x08); F($7, 16, 4); DSP($5, 14, DSIZE);
	  POST($3 << 4); }
	| DMOV DOT_D '[' REG ']' ',' DREG
	{ rx_check_dfpu();
	  B4(0xfc, 0xc8, 0x08, 0x00); F($4, 16, 4); F($7, 24, 4); }
	| DMOV DOT_D disp '[' REG ']' ',' DREG
	{ rx_check_dfpu();
	  B3(0xfc, 0xc8, 0x08); F($5, 16, 4); DSP($3, 14, DSIZE);
	  POST($8 << 4); }
	| DMOV DOT_D '#' EXPR ',' DREGH
	{ rx_check_dfpu();
	  B3(0xf9, 0x03, 0x03); F($6, 16, 4); IMM($4, -1); }
	| DMOV DOT_L '#' EXPR ',' DREGH
	{ rx_check_dfpu();
	  B3(0xf9, 0x03, 0x02); F($6, 16, 4); IMM($4, -1); }
	| DMOV DOT_L '#' EXPR ',' DREGL
	{ rx_check_dfpu();
	  B3(0xf9, 0x03, 0x00); F($6, 16, 4); IMM($4, -1); }
	| DPOPM DOT_D DREG '-' DREG
	{ rx_check_dfpu();
	  B3(0x75, 0xb8, 0x00); F($3, 16, 4); F($5 - $3, 20, 4); }
	| DPOPM DOT_L DCREG '-' DCREG
	{ rx_check_dfpu();
	  B3(0x75, 0xa8, 0x00); F($3, 16, 4); F($5 - $3, 20, 4); }
	| DPUSHM DOT_D DREG '-' DREG
	{ rx_check_dfpu();
	  B3(0x75, 0xb0, 0x00); F($3, 16, 4); F($5 - $3, 20, 4); }
	| DPUSHM DOT_L DCREG '-' DCREG
	{ rx_check_dfpu();
	  B3(0x75, 0xa0, 0x00); F($3, 16, 4); F($5 - $3, 20, 4); }
	| MVFDC DCREG ',' REG
	{ rx_check_dfpu();
	  B4(0xfd, 0x75, 0x80, 0x04); F($2, 24, 4); F($4, 20, 4); }
	| MVFDR
	{ rx_check_dfpu(); B3(0x75, 0x90, 0x1b); }
	| MVTDC REG ',' DCREG
	{ rx_check_dfpu();
	  B4(0xfd, 0x77, 0x80, 0x04); F($2, 24, 4); F($4, 20, 4); }
	| FTOD REG ',' DREG
	{ rx_check_dfpu();
	  B4(0xfd, 0x77, 0x80, 0x0a); F($2, 24, 4); F($4, 20, 4); }
	| ITOD REG ',' DREG
	{ rx_check_dfpu();
	  B4(0xfd, 0x77, 0x80, 0x09); F($2, 24, 4); F($4, 20, 4); }
	| UTOD REG ',' DREG
	{ rx_check_dfpu();
	  B4(0xfd, 0x77, 0x80, 0x0d); F($2, 24, 4); F($4, 20, 4); }

/* ---------------------------------------------------------------------- */

	;

/* ====================================================================== */

op_subadd
	: REG ',' REG
	  { B2 (0x43 + (sub_op<<2), 0); F ($1, 8, 4); F ($3, 12, 4); }
	| disp '[' REG ']' DOT_UB ',' REG
	  { B2 (0x40 + (sub_op<<2), 0); F ($3, 8, 4); F ($7, 12, 4); DSP ($1, 6, BSIZE); }
	| disp '[' REG ']' memex ',' REG
	  { B3 (MEMEX, sub_op<<2, 0); F ($5, 8, 2); F ($3, 16, 4); F ($7, 20, 4); DSP ($1, 14, sizemap[$5]); }
	| REG ',' REG ',' REG
	  { id24 (4, sub_op<<4, 0), F ($5, 12, 4), F ($1, 16, 4), F ($3, 20, 4); }
	;

/* sbb, neg, adc, abs, max, min, div, divu, tst, not, xor, stz, stnz, emul, emulu */

op_dp20_rm_l
	: REG ',' REG
	  { id24 (1, 0x03 + (sub_op<<2), 0x00); F ($1, 16, 4); F ($3, 20, 4); }
	| disp '[' REG ']' opt_l ',' REG
	  { B4 (MEMEX, 0xa0, 0x00 + sub_op, 0x00);
	  F ($3, 24, 4); F ($7, 28, 4); DSP ($1, 14, LSIZE); }
	;

/* neg, adc, abs, max, min, div, divu, tst, not, xor, stz, stnz, emul, emulu */

op_dp20_rm
	: REG ',' REG
	  { id24 (1, 0x03 + (sub_op<<2), 0x00); F ($1, 16, 4); F ($3, 20, 4); }
	| disp '[' REG ']' DOT_UB ',' REG
	  { id24 (1, 0x00 + (sub_op<<2), 0x00); F ($3, 16, 4); F ($7, 20, 4); DSP ($1, 14, BSIZE); }
	| disp '[' REG ']' memex ',' REG
	  { B4 (MEMEX, 0x20 + ($5 << 6), 0x00 + sub_op, 0x00);
	  F ($3, 24, 4); F ($7, 28, 4); DSP ($1, 14, sizemap[$5]); }
	;

op_dp20_i
	: '#' EXPR ',' REG
	  { id24 (2, 0x70, sub_op<<4); F ($4, 20, 4); IMM ($2, 12); }
	;

op_dp20_rim
	: op_dp20_rm
	| op_dp20_i
	;

op_dp20_rim_l
	: op_dp20_rm_l
	| op_dp20_i
	;

op_dp20_rr
	: REG ',' REG
	  { id24 (1, 0x03 + (sub_op<<2), 0x00); F ($1, 16, 4); F ($3, 20, 4); }
	| REG
	  { B2 (0x7e, sub_op2 << 4); F ($1, 12, 4); }
	;

op_dp20_r
	: REG ',' REG
	  { id24 (1, 0x4b + (sub_op2<<2), 0x00); F ($1, 16, 4); F ($3, 20, 4); }
	;

op_dp20_ri
	: { rx_check_v2 (); }
	  op_dp20_r
	| op_dp20_i
	;

/* xchg, utof, itof, emul, emulu */
op_xchg
	: REG ',' REG
	  { id24 (1, 0x03 + (sub_op<<2), 0); F ($1, 16, 4); F ($3, 20, 4); }
	| disp '[' REG ']' DOT_UB ',' REG
	  { id24 (1, 0x00 + (sub_op<<2), 0); F ($3, 16, 4); F ($7, 20, 4); DSP ($1, 14, BSIZE); }
	| disp '[' REG ']' memex ',' REG
	  { B4 (MEMEX, 0x20, 0x00 + sub_op, 0); F ($5, 8, 2); F ($3, 24, 4); F ($7, 28, 4);
	    DSP ($1, 14, sizemap[$5]); }
	;

/* 000:SHLR, 001:SHAR, 010:SHLL, 011:-, 100:ROTR, 101:REVW, 110:ROTL, 111:REVL */
op_shift_rot
	: REG ',' REG
	  { id24 (2, 0x60 + sub_op, 0); F ($1, 16, 4); F ($3, 20, 4); }
	;
op_shift
	: '#' EXPR ',' REG
	  { B2 (0x68 + (sub_op<<1), 0); FE ($2, 7, 5); F ($4, 12, 4); }
	| '#' EXPR ',' REG ',' REG
	  { id24 (2, 0x80 + (sub_op << 5), 0); FE ($2, 11, 5); F ($4, 16, 4); F ($6, 20, 4); }
	| op_shift_rot
	;

float3_op
	: '#' EXPR ',' REG
	  { rx_check_float_support (); id24 (2, 0x72, sub_op << 4); F ($4, 20, 4); O4 ($2); }
	| REG ',' REG
	  { rx_check_float_support (); id24 (1, 0x83 + (sub_op << 2), 0); F ($1, 16, 4); F ($3, 20, 4); }
	| disp '[' REG ']' opt_l ',' REG
	  { rx_check_float_support (); id24 (1, 0x80 + (sub_op << 2), 0); F ($3, 16, 4); F ($7, 20, 4); DSP ($1, 14, LSIZE); }
	| REG ',' REG ',' REG
	  { rx_check_v2 (); id24 (4, 0x80 + (sub_op << 4), 0 ); F ($1, 16, 4); F ($3, 20, 4); F ($5, 12, 4); }
	;

float2_op
	: { rx_check_float_support (); }
	  '#' EXPR ',' REG
	  { id24 (2, 0x72, sub_op << 4); F ($5, 20, 4); O4 ($3); }
	| float2_op_ni
	;

float2_op_ni
	: { rx_check_float_support (); }
	  REG ',' REG
	  { id24 (1, 0x83 + (sub_op << 2), 0); F ($2, 16, 4); F ($4, 20, 4); }
	| { rx_check_float_support (); }
	  disp '[' REG ']' opt_l ',' REG
	  { id24 (1, 0x80 + (sub_op << 2), 0); F ($4, 16, 4); F ($8, 20, 4); DSP ($2, 14, LSIZE); }
	;

mvfa_op
	: { rx_check_v2 (); }
	  '#' EXPR ',' ACC ',' REG
	  { id24 (2, 0x1e, sub_op << 4); F ($7, 20, 4); F ($5, 16, 1);
	    if (rx_uintop ($3, 4))
	      {
		switch (exp_val ($3))
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
	;

op_xor
	: op_dp20_rim
	| REG ',' REG ',' REG
	  { rx_check_v3(); B3(0xff,0x60,0x00), F ($5, 12, 4), F ($1, 16, 4), F ($3, 20, 4); }
	;

op_bfield
	: { rx_check_v3(); }
	  '#' EXPR ',' '#' EXPR ',' '#' EXPR ',' REG ',' REG
	  { rx_range($3, 0, 31); rx_range($6, 0, 31); rx_range($9, 1, 31);
	    B3(0xfc, 0x5a + (sub_op << 2), 0); F($11, 16, 4); F($13, 20, 4);
	  rx_bfield($3, $6, $9);}
	;

op_save_rstr
	: '#' EXPR
	  { B3(0xfd,0x76,0xe0 + (sub_op << 4)); UO1($2); }
	| REG
	  { B4(0xfd,0x76,0xc0 + (sub_op << 4), 0x00); F($1, 20, 4); }
	;

double2_op
	: DREG ',' DREG
	{ B4(0x76, 0x90, sub_op, sub_op2); F($1, 16, 4); F($3, 24, 4);}

double3_op
	: DREG ',' DREG ',' DREG
	{ B4(0x76, 0x90, sub_op, 0x00); F($1, 28, 4); F($3, 16,4); F($5, 24, 4);}

/* ====================================================================== */

disp	:      { $$ = zero_expr (); }
	| EXPR { $$ = $1; }
	;

flag	: { need_flag = 1; } FLAG { need_flag = 0; $$ = $2; }
	;

/* DOT_UB is not listed here, it's handled with a separate pattern.  */
/* Use sizemap[$n] to get LSIZE etc.  */
memex	: DOT_B  { $$ = 0; }
	| DOT_W  { $$ = 1; }
	|        { $$ = 2; }
	| DOT_L  { $$ = 2; }
	| DOT_UW { $$ = 3; }
	;

bwl	:       { $$ = LSIZE; }
	| DOT_B { $$ = BSIZE; }
	| DOT_W { $$ = WSIZE; }
	| DOT_L { $$ = LSIZE; }
	;

bw	:       { $$ = 1; }
	| DOT_B { $$ = 0; }
	| DOT_W { $$ = 1; }
	;

opt_l	: 	{}
	| DOT_L {}
	;

opt_b	: 	{}
	| DOT_B {}
	;

%%
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
