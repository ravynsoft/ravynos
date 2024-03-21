/* rl78-parse.y  Renesas RL78 parser
   Copyright (C) 2011-2023 Free Software Foundation, Inc.

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
#include "rl78-defs.h"

static int rl78_lex (void);

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

#define B1(b1)             rl78_base1 (b1)
#define B2(b1, b2)         rl78_base2 (b1, b2)
#define B3(b1, b2, b3)     rl78_base3 (b1, b2, b3)
#define B4(b1, b2, b3, b4) rl78_base4 (b1, b2, b3, b4)

/* POS is bits from the MSB of the first byte to the LSB of the last byte.  */
#define F(val,pos,sz)      rl78_field (val, pos, sz)
#define FE(exp,pos,sz)	   rl78_field (exp_val (exp), pos, sz);

#define O1(v)              rl78_op (v, 1, RL78REL_DATA)
#define O2(v)              rl78_op (v, 2, RL78REL_DATA)
#define O3(v)              rl78_op (v, 3, RL78REL_DATA)
#define O4(v)              rl78_op (v, 4, RL78REL_DATA)

#define PC1(v)             rl78_op (v, 1, RL78REL_PCREL)
#define PC2(v)             rl78_op (v, 2, RL78REL_PCREL)
#define PC3(v)             rl78_op (v, 3, RL78REL_PCREL)

#define IMM(v,pos)	   F (immediate (v, RL78REL_SIGNED, pos), pos, 2); \
			   if (v.X_op != O_constant && v.X_op != O_big) rl78_linkrelax_imm (pos)
#define NIMM(v,pos)	   F (immediate (v, RL78REL_NEGATIVE, pos), pos, 2)
#define NBIMM(v,pos)	   F (immediate (v, RL78REL_NEGATIVE_BORROW, pos), pos, 2)
#define DSP(v,pos,msz)	   if (!v.X_md) rl78_relax (RL78_RELAX_DISP, pos); \
			   else rl78_linkrelax_dsp (pos); \
			   F (displacement (v, msz), pos, 2)

#define id24(a,b2,b3)	   B3 (0xfb+a, b2, b3)

static int         expr_is_sfr (expressionS);
static int         expr_is_saddr (expressionS);
static int         expr_is_word_aligned (expressionS);
static int         exp_val (expressionS exp);

static int    need_flag = 0;
static int    rl78_in_brackets = 0;
static int    rl78_last_token = 0;
static char * rl78_init_start;
static char * rl78_last_exp_start = 0;
static int    rl78_bit_insn = 0;

#define YYDEBUG 1
#define YYERROR_VERBOSE 1

#define NOT_SADDR  rl78_error ("Expression not 0xFFE20 to 0xFFF1F")
#define SA(e) if (!expr_is_saddr (e)) NOT_SADDR;

#define SET_SA(e) e.X_md = BFD_RELOC_RL78_SADDR

#define NOT_SFR  rl78_error ("Expression not 0xFFF00 to 0xFFFFF")
#define SFR(e) if (!expr_is_sfr (e)) NOT_SFR;

#define NOT_SFR_OR_SADDR  rl78_error ("Expression not 0xFFE20 to 0xFFFFF")

#define NOT_ES if (rl78_has_prefix()) rl78_error ("ES: prefix not allowed here");

#define WA(x) if (!expr_is_word_aligned (x)) rl78_error ("Expression not word-aligned");

#define ISA_G10(s) if (!rl78_isa_g10()) rl78_error (s " is only supported on the G10")
#define ISA_G13(s) if (!rl78_isa_g13()) rl78_error (s " is only supported on the G13")
#define ISA_G14(s) if (!rl78_isa_g14()) rl78_error (s " is only supported on the G14")

static void check_expr_is_bit_index (expressionS);
#define Bit(e) check_expr_is_bit_index (e);

/* Returns TRUE (non-zero) if the expression is a constant in the
   given range.  */
static int check_expr_is_const (expressionS, int vmin, int vmax);

/* Convert a "regb" value to a "reg_xbc" value.  Error if other
   registers are passed.  Needed to avoid reduce-reduce conflicts.  */
static int
reg_xbc (int reg)
{
  switch (reg)
    {
      case 0: /* X */
        return 0x10;
      case 3: /* B */
        return 0x20;
      case 2: /* C */
        return 0x30;
      default:
        rl78_error ("Only X, B, or C allowed here");
	return 0;
    }
}

%}

%name-prefix="rl78_"

%union {
  int regno;
  expressionS exp;
}

%type <regno> regb regb_na regw regw_na FLAG sfr
%type <regno> A X B C D E H L AX BC DE HL
%type <exp> EXPR

%type <regno> addsub addsubw andor1 bt_bf setclr1 oneclrb oneclrw
%type <regno> incdec incdecw

%token A X B C D E H L AX BC DE HL
%token SPL SPH PSW CS ES PMC MEM
%token FLAG SP CY
%token RB0 RB1 RB2 RB3

%token EXPR UNKNOWN_OPCODE IS_OPCODE

%token DOT_S DOT_B DOT_W DOT_L DOT_A DOT_UB DOT_UW

%token ADD ADDC ADDW AND_ AND1
/* BC is also a register pair */
%token BF BH BNC BNH BNZ BR BRK BRK1 BT BTCLR BZ
%token CALL CALLT CLR1 CLRB CLRW CMP CMP0 CMPS CMPW
%token DEC DECW DI DIVHU DIVWU
%token EI
%token HALT
%token INC INCW
%token MACH MACHU MOV MOV1 MOVS MOVW MULH MULHU MULU
%token NOP NOT1
%token ONEB ONEW OR OR1
%token POP PUSH
%token RET RETI RETB ROL ROLC ROLWC ROR RORC
%token SAR SARW SEL SET1 SHL SHLW SHR SHRW
%token   SKC SKH SKNC SKNH SKNZ SKZ STOP SUB SUBC SUBW
%token XCH XCHW XOR XOR1

%%
/* ====================================================================== */

statement :

	  UNKNOWN_OPCODE
	  { as_bad (_("Unknown opcode: %s"), rl78_init_start); }

/* The opcodes are listed in approximately alphabetical order.  */

/* For reference:

  sfr  = special function register - symbol, 0xFFF00 to 0xFFFFF
  sfrp = special function register - symbol, 0xFFF00 to 0xFFFFE, even only
  saddr  = 0xFFE20 to 0xFFF1F
  saddrp = 0xFFE20 to 0xFFF1E, even only

  addr20 = 0x00000 to 0xFFFFF
  addr16 = 0x00000 to 0x0FFFF, even only for 16-bit ops
  addr5  = 0x00000 to 0x000BE, even only
*/

/* ---------------------------------------------------------------------- */

/* addsub is ADD, ADDC, SUB, SUBC, AND, OR, XOR, and parts of CMP.  */

	| addsub A ',' '#' EXPR
	  { B1 (0x0c|$1); O1 ($5); }

	| addsub EXPR {SA($2)} ',' '#' EXPR
	  { B1 (0x0a|$1); SET_SA ($2); O1 ($2); O1 ($6); }

	| addsub A ',' A
	  { B2 (0x61, 0x01|$1); }

	| addsub A ',' regb_na
	  { B2 (0x61, 0x08|$1); F ($4, 13, 3); }

	| addsub regb_na ',' A
	  { B2 (0x61, 0x00|$1); F ($2, 13, 3); }

	| addsub A ',' EXPR {SA($4)}
	  { B1 (0x0b|$1); SET_SA ($4); O1 ($4); }

	| addsub A ',' opt_es '!' EXPR
	  { B1 (0x0f|$1); O2 ($6); rl78_linkrelax_addr16 (); }

	| addsub A ',' opt_es '[' HL ']'
	  { B1 (0x0d|$1); }

	| addsub A ',' opt_es '[' HL '+' EXPR ']'
	  { B1 (0x0e|$1); O1 ($8); }

	| addsub A ',' opt_es '[' HL '+' B ']'
	  { B2 (0x61, 0x80|$1); }

	| addsub A ',' opt_es '[' HL '+' C ']'
	  { B2 (0x61, 0x82|$1); }

	| addsub opt_es '!' EXPR ',' '#' EXPR
	  { if ($1 != 0x40)
	      { rl78_error ("Only CMP takes these operands"); }
	    else
	      { B1 (0x00|$1); O2 ($4); O1 ($7); rl78_linkrelax_addr16 (); }
	  }

/* ---------------------------------------------------------------------- */

	| addsubw AX ',' '#' EXPR
	  { B1 (0x04|$1); O2 ($5); }

	| addsubw AX ',' regw
	  { B1 (0x01|$1); F ($4, 5, 2); }

	| addsubw AX ',' EXPR {SA($4)}
	  { B1 (0x06|$1); SET_SA ($4); O1 ($4); }

	| addsubw AX ',' opt_es '!' EXPR
	  { B1 (0x02|$1); O2 ($6); rl78_linkrelax_addr16 (); }

	| addsubw AX ',' opt_es '[' HL '+' EXPR ']'
	  { B2 (0x61, 0x09|$1); O1 ($8); }

	| addsubw AX ',' opt_es '[' HL ']'
	  { B3 (0x61, 0x09|$1, 0); }

	| addsubw SP ',' '#' EXPR
	  { B1 ($1 ? 0x20 : 0x10); O1 ($5);
	    if ($1 == 0x40)
	      rl78_error ("CMPW SP,#imm not allowed");
	  }

/* ---------------------------------------------------------------------- */

	| andor1 CY ',' sfr '.' EXPR {Bit($6)}
	  { B3 (0x71, 0x08|$1, $4); FE ($6, 9, 3); }

	| andor1 CY ',' EXPR '.' EXPR {Bit($6)}
	  { if (expr_is_sfr ($4))
	      { B2 (0x71, 0x08|$1); FE ($6, 9, 3); O1 ($4); }
	    else if (expr_is_saddr ($4))
	      { B2 (0x71, 0x00|$1); FE ($6, 9, 3); SET_SA ($4); O1 ($4); }
	    else
	      NOT_SFR_OR_SADDR;
	  }

	| andor1 CY ',' A '.' EXPR {Bit($6)}
	  { B2 (0x71, 0x88|$1);  FE ($6, 9, 3); }

	| andor1 CY ',' opt_es '[' HL ']' '.' EXPR {Bit($9)}
	  { B2 (0x71, 0x80|$1);  FE ($9, 9, 3); }

/* ---------------------------------------------------------------------- */

	| BC '$' EXPR
	  { B1 (0xdc); PC1 ($3); rl78_linkrelax_branch (); }

	| BNC '$' EXPR
	  { B1 (0xde); PC1 ($3); rl78_linkrelax_branch (); }

	| BZ '$' EXPR
	  { B1 (0xdd); PC1 ($3); rl78_linkrelax_branch (); }

	| BNZ '$' EXPR
	  { B1 (0xdf); PC1 ($3); rl78_linkrelax_branch (); }

	| BH '$' EXPR
	  { B2 (0x61, 0xc3); PC1 ($3); rl78_linkrelax_branch (); }

	| BNH '$' EXPR
	  { B2 (0x61, 0xd3); PC1 ($3); rl78_linkrelax_branch (); }

/* ---------------------------------------------------------------------- */

	| bt_bf sfr '.' EXPR ',' '$' EXPR
	  { B3 (0x31, 0x80|$1, $2); FE ($4, 9, 3); PC1 ($7); }

	| bt_bf EXPR '.' EXPR ',' '$' EXPR
	  { if (expr_is_sfr ($2))
	      { B2 (0x31, 0x80|$1); FE ($4, 9, 3); O1 ($2); PC1 ($7); }
	    else if (expr_is_saddr ($2))
	      { B2 (0x31, 0x00|$1); FE ($4, 9, 3); SET_SA ($2); O1 ($2); PC1 ($7); }
	    else
	      NOT_SFR_OR_SADDR;
	  }

	| bt_bf A '.' EXPR ',' '$' EXPR
	  { B2 (0x31, 0x01|$1); FE ($4, 9, 3); PC1 ($7); }

	| bt_bf opt_es '[' HL ']' '.' EXPR ',' '$' EXPR
	  { B2 (0x31, 0x81|$1); FE ($7, 9, 3); PC1 ($10); }

/* ---------------------------------------------------------------------- */

	| BR AX
	  { B2 (0x61, 0xcb); }

	| BR '$' EXPR
	  { B1 (0xef); PC1 ($3); rl78_linkrelax_branch (); }

	| BR '$' '!' EXPR
	  { B1 (0xee); PC2 ($4); rl78_linkrelax_branch (); }

	| BR '!' EXPR
	  { B1 (0xed); O2 ($3); rl78_linkrelax_branch (); }

	| BR '!' '!' EXPR
	  { B1 (0xec); O3 ($4); rl78_linkrelax_branch (); }

/* ---------------------------------------------------------------------- */

	| BRK
	  { B2 (0x61, 0xcc); }

	| BRK1
	  { B1 (0xff); }

/* ---------------------------------------------------------------------- */

	| CALL regw
	  { B2 (0x61, 0xca); F ($2, 10, 2); }

	| CALL '$' '!' EXPR
	  { B1 (0xfe); PC2 ($4); }

	| CALL '!' EXPR
	  { B1 (0xfd); O2 ($3); }

	| CALL '!' '!' EXPR
	  { B1 (0xfc); O3 ($4); rl78_linkrelax_branch (); }

	| CALLT '[' EXPR ']'
	  { if ($3.X_op != O_constant)
	      rl78_error ("CALLT requires a numeric address");
	    else
	      {
	        int i = $3.X_add_number;
		if (i < 0x80 || i > 0xbe)
		  rl78_error ("CALLT address not 0x80..0xbe");
		else if (i & 1)
		  rl78_error ("CALLT address not even");
		else
		  {
		    B2 (0x61, 0x84);
	    	    F ((i >> 1) & 7, 9, 3);
	    	    F ((i >> 4) & 7, 14, 2);
		  }
	      }
	  }

/* ---------------------------------------------------------------------- */

	| setclr1 CY
	  { B2 (0x71, $1 ? 0x88 : 0x80); }

	| setclr1 sfr '.' EXPR
	  { B3 (0x71, 0x0a|$1, $2); FE ($4, 9, 3); }

	| setclr1 EXPR '.' EXPR
	  { if (expr_is_sfr ($2))
	      { B2 (0x71, 0x0a|$1); FE ($4, 9, 3); O1 ($2); }
	    else if (expr_is_saddr ($2))
	      { B2 (0x71, 0x02|$1); FE ($4, 9, 3); SET_SA ($2); O1 ($2); }
	    else
	      NOT_SFR_OR_SADDR;
	  }

	| setclr1 A '.' EXPR
	  { B2 (0x71, 0x8a|$1);  FE ($4, 9, 3); }

	| setclr1 opt_es '!' EXPR '.' EXPR
	  { B2 (0x71, 0x00+$1*0x08); FE ($6, 9, 3); O2 ($4); rl78_linkrelax_addr16 (); }

	| setclr1 opt_es '[' HL ']' '.' EXPR
	  { B2 (0x71, 0x82|$1); FE ($7, 9, 3); }

/* ---------------------------------------------------------------------- */

	| oneclrb A
	  { B1 (0xe1|$1); }
	| oneclrb X
	  { B1 (0xe0|$1); }
	| oneclrb B
	  { B1 (0xe3|$1); }
	| oneclrb C
	  { B1 (0xe2|$1); }

	| oneclrb EXPR {SA($2)}
	  { B1 (0xe4|$1); SET_SA ($2); O1 ($2); }

	| oneclrb opt_es '!' EXPR
	  { B1 (0xe5|$1); O2 ($4); rl78_linkrelax_addr16 (); }

/* ---------------------------------------------------------------------- */

	| oneclrw AX
	  { B1 (0xe6|$1); }
	| oneclrw BC
	  { B1 (0xe7|$1); }

/* ---------------------------------------------------------------------- */

	| CMP0 A
	  { B1 (0xd1); }

	| CMP0 X
	  { B1 (0xd0); }

	| CMP0 B
	  { B1 (0xd3); }

	| CMP0 C
	  { B1 (0xd2); }

	| CMP0 EXPR {SA($2)}
	  { B1 (0xd4); SET_SA ($2); O1 ($2); }

	| CMP0 opt_es '!' EXPR
	  { B1 (0xd5); O2 ($4); rl78_linkrelax_addr16 (); }

/* ---------------------------------------------------------------------- */

	| CMPS X ',' opt_es '[' HL '+' EXPR ']'
	  { B2 (0x61, 0xde); O1 ($8); }

/* ---------------------------------------------------------------------- */

	| incdec regb
	  { B1 (0x80|$1); F ($2, 5, 3); }

	| incdec EXPR {SA($2)}
	  { B1 (0xa4|$1); SET_SA ($2); O1 ($2); }
	| incdec '!' EXPR
	  { B1 (0xa0|$1); O2 ($3); rl78_linkrelax_addr16 (); }
	| incdec ES ':' '!' EXPR
	  { B2 (0x11, 0xa0|$1); O2 ($5); }
	| incdec '[' HL '+' EXPR ']'
	  { B2 (0x61, 0x59+$1); O1 ($5); }
	| incdec ES ':' '[' HL '+' EXPR ']'
	  { B3 (0x11, 0x61, 0x59+$1); O1 ($7); }

/* ---------------------------------------------------------------------- */

	| incdecw regw
	  { B1 (0xa1|$1); F ($2, 5, 2); }

	| incdecw EXPR {SA($2)}
	  { B1 (0xa6|$1); SET_SA ($2); O1 ($2); }

	| incdecw opt_es '!' EXPR
	  { B1 (0xa2|$1); O2 ($4); rl78_linkrelax_addr16 (); }

	| incdecw opt_es '[' HL '+' EXPR ']'
	  { B2 (0x61, 0x79+$1); O1 ($6); }

/* ---------------------------------------------------------------------- */

	| DI
	  { B3 (0x71, 0x7b, 0xfa); }

	| EI
	  { B3 (0x71, 0x7a, 0xfa); }

/* ---------------------------------------------------------------------- */

	| MULHU { ISA_G14 ("MULHU"); }
	  { B3 (0xce, 0xfb, 0x01); }

	| MULH { ISA_G14 ("MULH"); }
	  { B3 (0xce, 0xfb, 0x02); }

	| MULU X
	  { B1 (0xd6); }

	| DIVHU { ISA_G14 ("DIVHU"); }
	  { B3 (0xce, 0xfb, 0x03); }

/* Note that the DIVWU encoding was changed from [0xce,0xfb,0x04] to
   [0xce,0xfb,0x0b].  Different versions of the Software Manual exist
   with the same version number, but varying encodings.  The version
   here matches the hardware.  */

	| DIVWU { ISA_G14 ("DIVWU"); }
	  { B3 (0xce, 0xfb, 0x0b); }

	| MACHU { ISA_G14 ("MACHU"); }
	  { B3 (0xce, 0xfb, 0x05); }

	| MACH { ISA_G14 ("MACH"); }
	  { B3 (0xce, 0xfb, 0x06); }

/* ---------------------------------------------------------------------- */

	| HALT
	  { B2 (0x61, 0xed); }

/* ---------------------------------------------------------------------- */
/* Note that opt_es is included even when it's not an option, to avoid
   shift/reduce conflicts.  The NOT_ES macro produces an error if ES:
   is given by the user.  */

	| MOV A ',' '#' EXPR
	  { B1 (0x51); O1 ($5); }
	| MOV regb_na ',' '#' EXPR
	  { B1 (0x50); F($2, 5, 3); O1 ($5); }

	| MOV sfr ',' '#' EXPR
	  { if ($2 != 0xfd)
	      { B2 (0xce, $2); O1 ($5); }
	    else
	      { B1 (0x41); O1 ($5); }
	  }

	| MOV opt_es EXPR ',' '#' EXPR  {NOT_ES}
	  { if (expr_is_sfr ($3))
	      { B1 (0xce); O1 ($3); O1 ($6); }
	    else if (expr_is_saddr ($3))
	      { B1 (0xcd); SET_SA ($3); O1 ($3); O1 ($6); }
	    else
	      NOT_SFR_OR_SADDR;
	  }

	| MOV '!' EXPR ',' '#' EXPR
	  { B1 (0xcf); O2 ($3); O1 ($6); rl78_linkrelax_addr16 (); }

	| MOV ES ':' '!' EXPR ',' '#' EXPR
	  { B2 (0x11, 0xcf); O2 ($5); O1 ($8); }

	| MOV regb_na ',' A
	  { B1 (0x70); F ($2, 5, 3); }

	| MOV A ',' regb_na
	  { B1 (0x60); F ($4, 5, 3); }

	| MOV opt_es EXPR ',' A  {NOT_ES}
	  { if (expr_is_sfr ($3))
	      { B1 (0x9e); O1 ($3); }
	    else if (expr_is_saddr ($3))
	      { B1 (0x9d); SET_SA ($3); O1 ($3); }
	    else
	      NOT_SFR_OR_SADDR;
	  }

	| MOV A ',' opt_es '!' EXPR
	  { B1 (0x8f); O2 ($6); rl78_linkrelax_addr16 (); }

	| MOV '!' EXPR ',' A
	  { B1 (0x9f); O2 ($3); rl78_linkrelax_addr16 (); }

	| MOV ES ':' '!' EXPR ',' A
	  { B2 (0x11, 0x9f); O2 ($5); }

	| MOV regb_na ',' opt_es '!' EXPR
	  { B1 (0xc9|reg_xbc($2)); O2 ($6); rl78_linkrelax_addr16 (); }

	| MOV A ',' opt_es EXPR  {NOT_ES}
	  { if (expr_is_saddr ($5))
	      { B1 (0x8d); SET_SA ($5); O1 ($5); }
	    else if (expr_is_sfr ($5))
	      { B1 (0x8e); O1 ($5); }
	    else
	      NOT_SFR_OR_SADDR;
	  }

	| MOV regb_na ',' opt_es EXPR {SA($5)} {NOT_ES}
	  { B1 (0xc8|reg_xbc($2)); SET_SA ($5); O1 ($5); }

	| MOV A ',' sfr
	  { B2 (0x8e, $4); }

	| MOV sfr ',' regb
	  { if ($4 != 1)
	      rl78_error ("Only A allowed here");
	    else
	      { B2 (0x9e, $2); }
	  }

	| MOV sfr ',' opt_es EXPR {SA($5)} {NOT_ES}
	  { if ($2 != 0xfd)
	      rl78_error ("Only ES allowed here");
	    else
	      { B2 (0x61, 0xb8); SET_SA ($5); O1 ($5); }
	  }

	| MOV A ',' opt_es '[' DE ']'
	  { B1 (0x89); }

	| MOV opt_es '[' DE ']' ',' A
	  { B1 (0x99); }

	| MOV opt_es '[' DE '+' EXPR ']' ',' '#' EXPR
	  { B1 (0xca); O1 ($6); O1 ($10); }

	| MOV A ',' opt_es '[' DE '+' EXPR ']'
	  { B1 (0x8a); O1 ($8); }

	| MOV opt_es '[' DE '+' EXPR ']' ',' A
	  { B1 (0x9a); O1 ($6); }

	| MOV A ',' opt_es '[' HL ']'
	  { B1 (0x8b); }

	| MOV opt_es '[' HL ']' ',' A
	  { B1 (0x9b); }

	| MOV opt_es '[' HL '+' EXPR ']' ',' '#' EXPR
	  { B1 (0xcc); O1 ($6); O1 ($10); }

	| MOV A ',' opt_es '[' HL '+' EXPR ']'
	  { B1 (0x8c); O1 ($8); }

	| MOV opt_es '[' HL '+' EXPR ']' ',' A
	  { B1 (0x9c); O1 ($6); }

	| MOV A ',' opt_es '[' HL '+' B ']'
	  { B2 (0x61, 0xc9); }

	| MOV opt_es '[' HL '+' B ']' ',' A
	  { B2 (0x61, 0xd9); }

	| MOV A ',' opt_es '[' HL '+' C ']'
	  { B2 (0x61, 0xe9); }

	| MOV opt_es '[' HL '+' C ']' ',' A
	  { B2 (0x61, 0xf9); }

	| MOV opt_es EXPR '[' B ']' ',' '#' EXPR
	  { B1 (0x19); O2 ($3); O1 ($9); }

	| MOV A ',' opt_es EXPR '[' B ']'
	  { B1 (0x09); O2 ($5); }

	| MOV opt_es EXPR '[' B ']' ',' A
	  { B1 (0x18); O2 ($3); }

	| MOV opt_es EXPR '[' C ']' ',' '#' EXPR
	  { B1 (0x38); O2 ($3); O1 ($9); }

	| MOV A ',' opt_es EXPR '[' C ']'
	  { B1 (0x29); O2 ($5); }

	| MOV opt_es EXPR '[' C ']' ',' A
	  { B1 (0x28); O2 ($3); }

	| MOV opt_es EXPR '[' BC ']' ',' '#' EXPR
	  { B1 (0x39); O2 ($3); O1 ($9); }

	| MOV opt_es '[' BC ']' ',' '#' EXPR
	  { B3 (0x39, 0, 0); O1 ($8); }

	| MOV A ',' opt_es EXPR '[' BC ']'
	  { B1 (0x49); O2 ($5); }

	| MOV A ',' opt_es '[' BC ']'
	  { B3 (0x49, 0, 0); }

	| MOV opt_es EXPR '[' BC ']' ',' A
	  { B1 (0x48); O2 ($3); }

	| MOV opt_es '[' BC ']' ',' A
	  { B3 (0x48, 0, 0); }

	| MOV opt_es '[' SP '+' EXPR ']' ',' '#' EXPR  {NOT_ES}
	  { B1 (0xc8); O1 ($6); O1 ($10); }

	| MOV opt_es '[' SP ']' ',' '#' EXPR  {NOT_ES}
	  { B2 (0xc8, 0); O1 ($8); }

	| MOV A ',' opt_es '[' SP '+' EXPR ']'  {NOT_ES}
	  { B1 (0x88); O1 ($8); }

	| MOV A ',' opt_es '[' SP ']'  {NOT_ES}
	  { B2 (0x88, 0); }

	| MOV opt_es '[' SP '+' EXPR ']' ',' A  {NOT_ES}
	  { B1 (0x98); O1 ($6); }

	| MOV opt_es '[' SP ']' ',' A  {NOT_ES}
	  { B2 (0x98, 0); }

/* ---------------------------------------------------------------------- */

	| mov1 CY ',' EXPR '.' EXPR
	  { if (expr_is_saddr ($4))
	      { B2 (0x71, 0x04); FE ($6, 9, 3); SET_SA ($4); O1 ($4); }
	    else if (expr_is_sfr ($4))
	      { B2 (0x71, 0x0c); FE ($6, 9, 3); O1 ($4); }
	    else
	      NOT_SFR_OR_SADDR;
	  }

	| mov1 CY ',' A '.' EXPR
	  { B2 (0x71, 0x8c); FE ($6, 9, 3); }

	| mov1 CY ',' sfr '.' EXPR
	  { B3 (0x71, 0x0c, $4); FE ($6, 9, 3); }

	| mov1 CY ',' opt_es '[' HL ']' '.' EXPR
	  { B2 (0x71, 0x84); FE ($9, 9, 3); }

	| mov1 EXPR '.' EXPR ',' CY
	  { if (expr_is_saddr ($2))
	      { B2 (0x71, 0x01); FE ($4, 9, 3); SET_SA ($2); O1 ($2); }
	    else if (expr_is_sfr ($2))
	      { B2 (0x71, 0x09); FE ($4, 9, 3); O1 ($2); }
	    else
	      NOT_SFR_OR_SADDR;
	  }

	| mov1 A '.' EXPR ',' CY
	  { B2 (0x71, 0x89); FE ($4, 9, 3); }

	| mov1 sfr '.' EXPR ',' CY
	  { B3 (0x71, 0x09, $2); FE ($4, 9, 3); }

	| mov1 opt_es '[' HL ']' '.' EXPR ',' CY
	  { B2 (0x71, 0x81); FE ($7, 9, 3); }

/* ---------------------------------------------------------------------- */

	| MOVS opt_es '[' HL '+' EXPR ']' ',' X
	  { B2 (0x61, 0xce); O1 ($6); }

/* ---------------------------------------------------------------------- */

	| MOVW AX ',' '#' EXPR
	  { B1 (0x30); O2 ($5); }

	| MOVW regw_na ',' '#' EXPR
	  { B1 (0x30); F ($2, 5, 2); O2 ($5); }

	| MOVW opt_es EXPR ',' '#' EXPR {NOT_ES}
	  { if (expr_is_saddr ($3))
	      { B1 (0xc9); SET_SA ($3); O1 ($3); O2 ($6); }
	    else if (expr_is_sfr ($3))
	      { B1 (0xcb); O1 ($3); O2 ($6); }
	    else
	      NOT_SFR_OR_SADDR;
	  }

	| MOVW AX ',' opt_es EXPR {NOT_ES}
	  { if (expr_is_saddr ($5))
	      { B1 (0xad); SET_SA ($5); O1 ($5); WA($5); }
	    else if (expr_is_sfr ($5))
	      { B1 (0xae); O1 ($5); WA($5); }
	    else
	      NOT_SFR_OR_SADDR;
	  }

	| MOVW opt_es EXPR ',' AX {NOT_ES}
	  { if (expr_is_saddr ($3))
	      { B1 (0xbd); SET_SA ($3); O1 ($3); WA($3); }
	    else if (expr_is_sfr ($3))
	      { B1 (0xbe); O1 ($3); WA($3); }
	    else
	      NOT_SFR_OR_SADDR;
	  }

	| MOVW AX ',' regw_na
	  { B1 (0x11); F ($4, 5, 2); }

	| MOVW regw_na ',' AX
	  { B1 (0x10); F ($2, 5, 2); }

	| MOVW AX ',' opt_es '!' EXPR
	  { B1 (0xaf); O2 ($6); WA($6); rl78_linkrelax_addr16 (); }

	| MOVW opt_es '!' EXPR ',' AX
	  { B1 (0xbf); O2 ($4); WA($4); rl78_linkrelax_addr16 (); }

	| MOVW AX ',' opt_es '[' DE ']'
	  { B1 (0xa9); }

	| MOVW opt_es '[' DE ']' ',' AX
	  { B1 (0xb9); }

	| MOVW AX ',' opt_es '[' DE '+' EXPR ']'
	  { B1 (0xaa); O1 ($8); }

	| MOVW opt_es '[' DE '+' EXPR ']' ',' AX
	  { B1 (0xba); O1 ($6); }

	| MOVW AX ',' opt_es '[' HL ']'
	  { B1 (0xab); }

	| MOVW opt_es '[' HL ']' ',' AX
	  { B1 (0xbb); }

	| MOVW AX ',' opt_es '[' HL '+' EXPR ']'
	  { B1 (0xac); O1 ($8); }

	| MOVW opt_es '[' HL '+' EXPR ']' ',' AX
	  { B1 (0xbc); O1 ($6); }

	| MOVW AX ',' opt_es EXPR '[' B ']'
	  { B1 (0x59); O2 ($5); }

	| MOVW opt_es EXPR '[' B ']' ',' AX
	  { B1 (0x58); O2 ($3); }

	| MOVW AX ',' opt_es EXPR '[' C ']'
	  { B1 (0x69); O2 ($5); }

	| MOVW opt_es EXPR '[' C ']' ',' AX
	  { B1 (0x68); O2 ($3); }

	| MOVW AX ',' opt_es EXPR '[' BC ']'
	  { B1 (0x79); O2 ($5); }

	| MOVW AX ',' opt_es '[' BC ']'
	  { B3 (0x79, 0, 0); }

	| MOVW opt_es EXPR '[' BC ']' ',' AX
	  { B1 (0x78); O2 ($3); }

	| MOVW opt_es '[' BC ']' ',' AX
	  { B3 (0x78, 0, 0); }

	| MOVW AX ',' opt_es '[' SP '+' EXPR ']' {NOT_ES}
	  { B1 (0xa8); O1 ($8);  WA($8);}

	| MOVW AX ',' opt_es '[' SP ']' {NOT_ES}
	  { B2 (0xa8, 0); }

	| MOVW opt_es '[' SP '+' EXPR ']' ',' AX {NOT_ES}
	  { B1 (0xb8); O1 ($6); WA($6); }

	| MOVW opt_es '[' SP ']' ',' AX {NOT_ES}
	  { B2 (0xb8, 0); }

	| MOVW regw_na ',' EXPR {SA($4)}
	  { B1 (0xca); F ($2, 2, 2); SET_SA ($4); O1 ($4); WA($4); }

	| MOVW regw_na ',' opt_es '!' EXPR
	  { B1 (0xcb); F ($2, 2, 2); O2 ($6); WA($6); rl78_linkrelax_addr16 (); }

	| MOVW SP ',' '#' EXPR
	  { B2 (0xcb, 0xf8); O2 ($5); }

	| MOVW SP ',' AX
	  { B2 (0xbe, 0xf8); }

	| MOVW AX ',' SP
	  { B2 (0xae, 0xf8); }

	| MOVW regw_na ',' SP
	  { B3 (0xcb, 0xf8, 0xff); F ($2, 2, 2); }

/* ---------------------------------------------------------------------- */

	| NOP
	  { B1 (0x00); }

/* ---------------------------------------------------------------------- */

	| NOT1 CY
	  { B2 (0x71, 0xc0); }

/* ---------------------------------------------------------------------- */

	| POP regw
	  { B1 (0xc0); F ($2, 5, 2); }

	| POP PSW
	  { B2 (0x61, 0xcd); };

	| PUSH regw
	  { B1 (0xc1); F ($2, 5, 2); }

	| PUSH PSW
	  { B2 (0x61, 0xdd); };

/* ---------------------------------------------------------------------- */

	| RET
	  { B1 (0xd7); }

	| RETI
	  { B2 (0x61, 0xfc); }

	| RETB
	  { B2 (0x61, 0xec); }

/* ---------------------------------------------------------------------- */

	| ROL A ',' EXPR
	  { if (check_expr_is_const ($4, 1, 1))
	      { B2 (0x61, 0xeb); }
	  }

	| ROLC A ',' EXPR
	  { if (check_expr_is_const ($4, 1, 1))
	      { B2 (0x61, 0xdc); }
	  }

	| ROLWC AX ',' EXPR
	  { if (check_expr_is_const ($4, 1, 1))
	      { B2 (0x61, 0xee); }
	  }

	| ROLWC BC ',' EXPR
	  { if (check_expr_is_const ($4, 1, 1))
	      { B2 (0x61, 0xfe); }
	  }

	| ROR A ',' EXPR
	  { if (check_expr_is_const ($4, 1, 1))
	      { B2 (0x61, 0xdb); }
	  }

	| RORC A ',' EXPR
	  { if (check_expr_is_const ($4, 1, 1))
	      { B2 (0x61, 0xfb);}
	  }

/* ---------------------------------------------------------------------- */

	| SAR A ',' EXPR
	  { if (check_expr_is_const ($4, 1, 7))
	      { B2 (0x31, 0x0b); FE ($4, 9, 3); }
	  }

	| SARW AX ',' EXPR
	  { if (check_expr_is_const ($4, 1, 15))
	      { B2 (0x31, 0x0f); FE ($4, 8, 4); }
	  }

/* ---------------------------------------------------------------------- */

	| SEL RB0
	  { B2 (0x61, 0xcf); }

	| SEL RB1
	  { B2 (0x61, 0xdf); }

	| SEL RB2
	  { B2 (0x61, 0xef); }

	| SEL RB3
	  { B2 (0x61, 0xff); }

/* ---------------------------------------------------------------------- */

	| SHL A ',' EXPR
	  { if (check_expr_is_const ($4, 1, 7))
	      { B2 (0x31, 0x09); FE ($4, 9, 3); }
	  }

	| SHL B ',' EXPR
	  { if (check_expr_is_const ($4, 1, 7))
	      { B2 (0x31, 0x08); FE ($4, 9, 3); }
	  }

	| SHL C ',' EXPR
	  { if (check_expr_is_const ($4, 1, 7))
	      { B2 (0x31, 0x07); FE ($4, 9, 3); }
	  }

	| SHLW AX ',' EXPR
	  { if (check_expr_is_const ($4, 1, 15))
	      { B2 (0x31, 0x0d); FE ($4, 8, 4); }
	  }

	| SHLW BC ',' EXPR
	  { if (check_expr_is_const ($4, 1, 15))
	      { B2 (0x31, 0x0c); FE ($4, 8, 4); }
	  }

/* ---------------------------------------------------------------------- */

	| SHR A ',' EXPR
	  { if (check_expr_is_const ($4, 1, 7))
	      { B2 (0x31, 0x0a); FE ($4, 9, 3); }
	  }

	| SHRW AX ',' EXPR
	  { if (check_expr_is_const ($4, 1, 15))
	      { B2 (0x31, 0x0e); FE ($4, 8, 4); }
	  }

/* ---------------------------------------------------------------------- */

	| SKC
	  { B2 (0x61, 0xc8); rl78_relax (RL78_RELAX_BRANCH, 0); }

	| SKH
	  { B2 (0x61, 0xe3); rl78_relax (RL78_RELAX_BRANCH, 0); }

	| SKNC
	  { B2 (0x61, 0xd8); rl78_relax (RL78_RELAX_BRANCH, 0); }

	| SKNH
	  { B2 (0x61, 0xf3); rl78_relax (RL78_RELAX_BRANCH, 0); }

	| SKNZ
	  { B2 (0x61, 0xf8); rl78_relax (RL78_RELAX_BRANCH, 0); }

	| SKZ
	  { B2 (0x61, 0xe8); rl78_relax (RL78_RELAX_BRANCH, 0); }

/* ---------------------------------------------------------------------- */

	| STOP
	  { B2 (0x61, 0xfd); }

/* ---------------------------------------------------------------------- */

	| XCH A ',' regb_na
	  { if ($4 == 0) /* X */
	      { B1 (0x08); }
	    else
	      { B2 (0x61, 0x88); F ($4, 13, 3); }
	  }

	| XCH A ',' opt_es '!' EXPR
	  { B2 (0x61, 0xaa); O2 ($6); rl78_linkrelax_addr16 (); }

	| XCH A ',' opt_es '[' DE ']'
	  { B2 (0x61, 0xae); }

	| XCH A ',' opt_es '[' DE '+' EXPR ']'
	  { B2 (0x61, 0xaf); O1 ($8); }

	| XCH A ',' opt_es '[' HL ']'
	  { B2 (0x61, 0xac); }

	| XCH A ',' opt_es '[' HL '+' EXPR ']'
	  { B2 (0x61, 0xad); O1 ($8); }

	| XCH A ',' opt_es '[' HL '+' B ']'
	  { B2 (0x61, 0xb9); }

	| XCH A ',' opt_es '[' HL '+' C ']'
	  { B2 (0x61, 0xa9); }

	| XCH A ',' EXPR
	  { if (expr_is_sfr ($4))
	      { B2 (0x61, 0xab); O1 ($4); }
	    else if (expr_is_saddr ($4))
	      { B2 (0x61, 0xa8); SET_SA ($4); O1 ($4); }
	    else
	      NOT_SFR_OR_SADDR;
	  }

/* ---------------------------------------------------------------------- */

	| XCHW AX ',' regw_na
	  { B1 (0x31); F ($4, 5, 2); }

/* ---------------------------------------------------------------------- */

	; /* end of statement */

/* ---------------------------------------------------------------------- */

opt_es	: /* nothing */
	| ES ':'
	  { rl78_prefix (0x11); }
	;

regb	: X { $$ = 0; }
	| A { $$ = 1; }
	| C { $$ = 2; }
	| B { $$ = 3; }
	| E { $$ = 4; }
	| D { $$ = 5; }
	| L { $$ = 6; }
	| H { $$ = 7; }
	;

regb_na	: X { $$ = 0; }
	| C { $$ = 2; }
	| B { $$ = 3; }
	| E { $$ = 4; }
	| D { $$ = 5; }
	| L { $$ = 6; }
	| H { $$ = 7; }
	;

regw	: AX { $$ = 0; }
	| BC { $$ = 1; }
	| DE { $$ = 2; }
	| HL { $$ = 3; }
	;

regw_na	: BC { $$ = 1; }
	| DE { $$ = 2; }
	| HL { $$ = 3; }
	;

sfr	: SPL { $$ = 0xf8; }
	| SPH { $$ = 0xf9; }
	| PSW { $$ = 0xfa; }
	| CS  { $$ = 0xfc; }
	| ES  { $$ = 0xfd; }
	| PMC { $$ = 0xfe; }
	| MEM { $$ = 0xff; }
	;

/* ---------------------------------------------------------------------- */
/* Shortcuts for groups of opcodes with common encodings.                 */

addsub	: ADD  { $$ = 0x00; }
	| ADDC { $$ = 0x10; }
	| SUB  { $$ = 0x20; }
	| SUBC { $$ = 0x30; }
	| CMP  { $$ = 0x40; }
	| AND_ { $$ = 0x50; }
	| OR   { $$ = 0x60; }
	| XOR  { $$ = 0x70; }
	;

addsubw	: ADDW  { $$ = 0x00; }
	| SUBW  { $$ = 0x20; }
	| CMPW  { $$ = 0x40; }
	;

andor1	: AND1 { $$ = 0x05; rl78_bit_insn = 1; }
	| OR1  { $$ = 0x06; rl78_bit_insn = 1; }
	| XOR1 { $$ = 0x07; rl78_bit_insn = 1; }
	;

bt_bf	: BT { $$ = 0x02;    rl78_bit_insn = 1; rl78_linkrelax_branch (); }
	| BF { $$ = 0x04;    rl78_bit_insn = 1; rl78_linkrelax_branch (); }
	| BTCLR { $$ = 0x00; rl78_bit_insn = 1; }
	;

setclr1	: SET1 { $$ = 0; rl78_bit_insn = 1; }
	| CLR1 { $$ = 1; rl78_bit_insn = 1; }
	;

oneclrb	: ONEB { $$ = 0x00; }
	| CLRB { $$ = 0x10; }
	;

oneclrw	: ONEW { $$ = 0x00; }
	| CLRW { $$ = 0x10; }
	;

incdec	: INC { $$ = 0x00; }
	| DEC { $$ = 0x10; }
	;

incdecw	: INCW { $$ = 0x00; }
	| DECW { $$ = 0x10; }
	;

mov1	: MOV1 { rl78_bit_insn = 1; }
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
  { "r0", X, 0 },
  { "r1", A, 1 },
  { "r2", C, 2 },
  { "r3", B, 3 },
  { "r4", E, 4 },
  { "r5", D, 5 },
  { "r6", L, 6 },
  { "r7", H, 7 },
  { "x", X, 0 },
  { "a", A, 1 },
  { "c", C, 2 },
  { "b", B, 3 },
  { "e", E, 4 },
  { "d", D, 5 },
  { "l", L, 6 },
  { "h", H, 7 },

  { "rp0", AX, 0 },
  { "rp1", BC, 1 },
  { "rp2", DE, 2 },
  { "rp3", HL, 3 },
  { "ax", AX, 0 },
  { "bc", BC, 1 },
  { "de", DE, 2 },
  { "hl", HL, 3 },

  { "RB0", RB0, 0 },
  { "RB1", RB1, 1 },
  { "RB2", RB2, 2 },
  { "RB3", RB3, 3 },

  { "sp", SP, 0 },
  { "cy", CY, 0 },

  { "spl", SPL, 0xf8 },
  { "sph", SPH, 0xf9 },
  { "psw", PSW, 0xfa },
  { "cs", CS, 0xfc },
  { "es", ES, 0xfd },
  { "pmc", PMC, 0xfe },
  { "mem", MEM, 0xff },

  { ".s", DOT_S, 0 },
  { ".b", DOT_B, 0 },
  { ".w", DOT_W, 0 },
  { ".l", DOT_L, 0 },
  { ".a", DOT_A , 0},
  { ".ub", DOT_UB, 0 },
  { ".uw", DOT_UW , 0},

  { "c", FLAG, 0 },
  { "z", FLAG, 1 },
  { "s", FLAG, 2 },
  { "o", FLAG, 3 },
  { "i", FLAG, 8 },
  { "u", FLAG, 9 },

#define OPC(x) { #x, x, IS_OPCODE }

  OPC(ADD),
  OPC(ADDC),
  OPC(ADDW),
  { "and", AND_, IS_OPCODE },
  OPC(AND1),
  OPC(BC),
  OPC(BF),
  OPC(BH),
  OPC(BNC),
  OPC(BNH),
  OPC(BNZ),
  OPC(BR),
  OPC(BRK),
  OPC(BRK1),
  OPC(BT),
  OPC(BTCLR),
  OPC(BZ),
  OPC(CALL),
  OPC(CALLT),
  OPC(CLR1),
  OPC(CLRB),
  OPC(CLRW),
  OPC(CMP),
  OPC(CMP0),
  OPC(CMPS),
  OPC(CMPW),
  OPC(DEC),
  OPC(DECW),
  OPC(DI),
  OPC(DIVHU),
  OPC(DIVWU),
  OPC(EI),
  OPC(HALT),
  OPC(INC),
  OPC(INCW),
  OPC(MACH),
  OPC(MACHU),
  OPC(MOV),
  OPC(MOV1),
  OPC(MOVS),
  OPC(MOVW),
  OPC(MULH),
  OPC(MULHU),
  OPC(MULU),
  OPC(NOP),
  OPC(NOT1),
  OPC(ONEB),
  OPC(ONEW),
  OPC(OR),
  OPC(OR1),
  OPC(POP),
  OPC(PUSH),
  OPC(RET),
  OPC(RETI),
  OPC(RETB),
  OPC(ROL),
  OPC(ROLC),
  OPC(ROLWC),
  OPC(ROR),
  OPC(RORC),
  OPC(SAR),
  OPC(SARW),
  OPC(SEL),
  OPC(SET1),
  OPC(SHL),
  OPC(SHLW),
  OPC(SHR),
  OPC(SHRW),
  OPC(SKC),
  OPC(SKH),
  OPC(SKNC),
  OPC(SKNH),
  OPC(SKNZ),
  OPC(SKZ),
  OPC(STOP),
  OPC(SUB),
  OPC(SUBC),
  OPC(SUBW),
  OPC(XCH),
  OPC(XCHW),
  OPC(XOR),
  OPC(XOR1),
};

#define NUM_TOKENS (sizeof (token_table) / sizeof (token_table[0]))

void
rl78_lex_init (char * beginning, char * ending)
{
  rl78_init_start = beginning;
  rl78_lex_start = beginning;
  rl78_lex_end = ending;
  rl78_in_brackets = 0;
  rl78_last_token = 0;

  rl78_bit_insn = 0;

  setbuf (stdout, 0);
}

/* Return a pointer to the '.' in a bit index expression (like
   foo.5), or NULL if none is found.  */
static char *
find_bit_index (char *tok)
{
  char *last_dot = NULL;
  char *last_digit = NULL;
  while (*tok && *tok != ',')
    {
      if (*tok == '.')
	{
	  last_dot = tok;
	  last_digit = NULL;
	}
      else if (*tok >= '0' && *tok <= '7'
	       && last_dot != NULL
	       && last_digit == NULL)
	{
	  last_digit = tok;
	}
      else if (ISSPACE (*tok))
	{
	  /* skip */
	}
      else
	{
	  last_dot = NULL;
	  last_digit = NULL;
	}
      tok ++;
    }
  if (last_dot != NULL
      && last_digit != NULL)
    return last_dot;
  return NULL;
}

static int
rl78_lex (void)
{
  /*unsigned int ci;*/
  char * save_input_pointer;
  char * bit = NULL;

  while (ISSPACE (*rl78_lex_start)
	 && rl78_lex_start != rl78_lex_end)
    rl78_lex_start ++;

  rl78_last_exp_start = rl78_lex_start;

  if (rl78_lex_start == rl78_lex_end)
    return 0;

  if (ISALPHA (*rl78_lex_start)
      || (*rl78_lex_start == '.' && ISALPHA (rl78_lex_start[1])))
    {
      unsigned int i;
      char * e;
      char save;

      for (e = rl78_lex_start + 1;
	   e < rl78_lex_end && ISALNUM (*e);
	   e ++)
	;
      save = *e;
      *e = 0;

      for (i = 0; i < NUM_TOKENS; i++)
	if (strcasecmp (rl78_lex_start, token_table[i].string) == 0
	    && !(token_table[i].val == IS_OPCODE && rl78_last_token != 0)
	    && !(token_table[i].token == FLAG && !need_flag))
	  {
	    rl78_lval.regno = token_table[i].val;
	    *e = save;
	    rl78_lex_start = e;
	    rl78_last_token = token_table[i].token;
	    return token_table[i].token;
	  }
      *e = save;
    }

  if (rl78_last_token == 0)
    {
      rl78_last_token = UNKNOWN_OPCODE;
      return UNKNOWN_OPCODE;
    }

  if (rl78_last_token == UNKNOWN_OPCODE)
    return 0;

  if (*rl78_lex_start == '[')
    rl78_in_brackets = 1;
  if (*rl78_lex_start == ']')
    rl78_in_brackets = 0;

  /* '.' is funny - the syntax includes it for bitfields, but only for
      bitfields.  We check for it specially so we can allow labels
      with '.' in them.  */

  if (rl78_bit_insn
      && *rl78_lex_start == '.'
      && find_bit_index (rl78_lex_start) == rl78_lex_start)
    {
      rl78_last_token = *rl78_lex_start;
      return *rl78_lex_start ++;
    }

  if ((rl78_in_brackets && *rl78_lex_start == '+')
      || strchr ("[],#!$:", *rl78_lex_start))
    {
      rl78_last_token = *rl78_lex_start;
      return *rl78_lex_start ++;
    }

  /* Again, '.' is funny.  Look for '.<digit>' at the end of the line
     or before a comma, which is a bitfield, not an expression.  */

  if (rl78_bit_insn)
    {
      bit = find_bit_index (rl78_lex_start);
      if (bit)
	*bit = 0;
      else
	bit = NULL;
    }

  save_input_pointer = input_line_pointer;
  input_line_pointer = rl78_lex_start;
  rl78_lval.exp.X_md = 0;
  expression (&rl78_lval.exp);

  if (bit)
    *bit = '.';

  rl78_lex_start = input_line_pointer;
  input_line_pointer = save_input_pointer;
  rl78_last_token = EXPR;
  return EXPR;
}

int
rl78_error (const char * str)
{
  int len;

  len = rl78_last_exp_start - rl78_init_start;

  as_bad ("%s", rl78_init_start);
  as_bad ("%*s^ %s", len, "", str);
  return 0;
}

static int
expr_is_sfr (expressionS exp)
{
  unsigned long v;

  if (exp.X_op != O_constant)
    return 0;

  v = exp.X_add_number;
  if (0xFFF00 <= v && v <= 0xFFFFF)
    return 1;
  return 0;
}

static int
expr_is_saddr (expressionS exp)
{
  unsigned long v;

  if (exp.X_op != O_constant)
    return 1;

  v = exp.X_add_number;
  if (0xFFE20 <= v && v <= 0xFFF1F)
    return 1;
  return 0;
}

static int
expr_is_word_aligned (expressionS exp)
{
  unsigned long v;

  if (exp.X_op != O_constant)
    return 1;

  v = exp.X_add_number;
  if (v & 1)
    return 0;
  return 1;

}

static void
check_expr_is_bit_index (expressionS exp)
{
  int val;

  if (exp.X_op != O_constant)
    {
      rl78_error (_("bit index must be a constant"));
      return;
    }
  val = exp.X_add_number;

  if (val < 0 || val > 7)
    rl78_error (_("rtsd size must be 0..7"));
}

static int
exp_val (expressionS exp)
{
  if (exp.X_op != O_constant)
  {
    rl78_error (_("constant expected"));
    return 0;
  }
  return exp.X_add_number;
}

static int
check_expr_is_const (expressionS e, int vmin, int vmax)
{
  static char buf[100];
  if (e.X_op != O_constant
      || e.X_add_number < vmin
      || e.X_add_number > vmax)
    {
      if (vmin == vmax)
	sprintf (buf, "%d expected here", vmin);
      else
	sprintf (buf, "%d..%d expected here", vmin, vmax);
      rl78_error(buf);
      return 0;
    }
  return 1;
}
