/* i860_opcode.h -- Table of opcodes for the i860.
   Copyright (C) 1989 Free Software Foundation, Inc.

This file is part of GAS, the GNU Assembler.

GAS is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 1, or (at your option)
any later version.

GAS is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GAS; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

#include <stdint.h>

/* Assorted opcode bits used to diddle the instruction during operand processing */
#define OP_PREFIX_MASK	0xFC000000
#define PREFIX_FPU	0x48000000
#define OP_FNOP		0xB0000000
#define OP_NOP		0xA0000000
#define DUAL_INSN_MODE_BIT	0x00000200

#define LOGOP_MASK	0xC0000000
#define IS_LOGOP(x)	(((x)&LOGOP_MASK) == LOGOP_MASK)

/* Macros to fill in register fields of insns */
#define SET_RS1(op, rval)	(op)=(((op) & ~(0x1F << 11)) | ((rval) & 0x1F) << 11)
#define SET_RS2(op, rval)	(op)=(((op) & ~(0x1F << 21)) | ((rval) & 0x1F) << 21)
#define SET_RD(op, rval)	(op)=(((op) & ~(0x1F << 16)) | ((rval) & 0x1F) << 16)

#define GET_RS1(op)	(((op) >> 11) & 0x1F)
#define GET_RS2(op)	(((op) >> 21) & 0x1F)
#define GET_RD(op)	(((op) >> 16) & 0x1F)


/*
 * Structure of an opcode table entry.
 */
struct i860_opcode
{
    const char *name;
    uint32_t mask;     /* used only for error checking */
    uint32_t match;
    const char *args;
    unsigned int last;      /* used to simplify hashing */
};

/*
   All i860 opcodes are 32 bits.

   The match component is a mask saying which bits must match a
   particular opcode in order for an instruction to be an instance
   of that opcode.

   The args component is a string containing one character
   for each operand of the instruction.

Kinds of operands:
   #    Number used by optimizer.  It is ignored.
   1    rs1 register, bits 11-15 of insn.
   2    rs2 register, bits 21-25 of insn.
   d    rd register, bits 16-20 of insn.
   e    frs1 floating point register, bits 11-15 of insn.
   f    frs2 floating point register, bits 21-25 of insn.
   g    frsd floating point register, bits 16-20 of insn.
   E	Same as e, f, g above, but requires an even reg number.
   F
   G
   H	frsd floating point register, quad aligned! (f0, f4, f8....)
   i	16 bit byte address low half, default of RELOC_LOW0
   I	16 bit High portion of address, RELOC_HIGH.
   j	16 bit short address, RELOC_LOW1
   k	16 bit word/int	address low half, RELOC_LOW2
   l	16 bit 8-byte address (double) low half, RELOC_LOW3
   m	16 bit 16-byte address (quad) low half, RELOC_LOW4

   n	16 bit byte aligned low half, split fields, RELOC_SPLIT0
   o	16 bit short aligned low half, split fields, RELOC_SPLIT1
   p	16 bit int/word aligned low half, split fields, RELOC_SPLIT2

   J	16 bit High portion of address requiring adjustment, RELOC_HIGHADJ
   K	26 bit branch displacement
   L	16 bit split branch displacement
   B	5 bit immediate, for bte and btne insn, RS1 field.
   D	Immediate field constant, no label or reloc data permitted.
   C    Control Register, one of fir, psr, epsr, dirbase, db, or fsr.
   S    Special ops
   
Literals used and matched in operand list:
   ()	Used around indirect register ld and st operations.
   ,	Arg separator.
 */

/*
 *   The assembler requires that all instances of the same mnemonic must be
 *   consecutive.  If they aren't, the assembler will bomb at runtime.
 */

static const struct i860_opcode i860_opcodes[] =
{

{ "adds",	0xFC000000, 0x90000000,	"1,2,d", 0 },
{ "adds",	0xFC000000, 0x94000000,	"i,2,d", 1 },

{ "addu",	0xFC000000, 0x80000000,	"1,2,d", 0 },
{ "addu",	0xFC000000, 0x84000000,	"i,2,d", 1 },

{ "and",	0xFC000000, 0xC0000000,	"1,2,d", 0 },
{ "and",	0xFC000000, 0xC4000000,	"i,2,d", 1 },
{ "andh",	0xFC000000, 0xCC000000,	"i,2,d", 1 },

{ "andnot",	0xFC000000, 0xD0000000,	"1,2,d", 0 },
{ "andnot",	0xFC000000, 0xD4000000,	"i,2,d", 1 },
{ "andnoth",	0xFC000000, 0xDC000000,	"i,2,d", 1 },

{ "bc",		0xFC000000, 0x70000000,	"K", 1 },
{ "bc.t",	0xFC000000, 0x74000000,	"K", 1 },

{ "bla",	0xFC000000, 0xB4000000,	"1,2,L", 1 },

{ "bnc",	0xFC000000, 0x78000000,	"K", 1 },
{ "bnc.t",	0xFC000000, 0x7C000000,	"K", 1 },

{ "br",		0xFC000000, 0x68000000,	"K", 1 },

{ "bri",	0xFC000000, 0x40000000,	"1", 1 },

{ "bte",	0xFC000000, 0x58000000,	"1,2,L", 0 },
{ "bte",	0xFC000000, 0x5C000000,	"B,2,L", 1 },

{ "btne",	0xFC000000, 0x50000000,	"1,2,L", 0 },
{ "btne",	0xFC000000, 0x54000000,	"B,2,L", 1 },

{ "call",	0xFC000000, 0x6C000000,	"K", 1 },

{ "calli",	0xFC00001F, 0x4C000002,	"1", 1 },

{ "fadd.dd",	0xFC0007FF, 0x480001B0,	"E,F,G", 1 },
{ "fadd.sd",	0xFC0007FF, 0x480000B0,	"e,f,G", 1 },
{ "fadd.ss",	0xFC0007FF, 0x48000030,	"e,f,g", 1 },

{ "faddp",	0xFC0007FF, 0x480001D0,	"E,F,G", 1 },

{ "faddz",	0xFC0007FF, 0x480001D1,	"E,F,G", 1 },

{ "fiadd.dd",	0xFC0007FF, 0x480001C9,	"E,F,G", 1 },
{ "fiadd.ss",	0xFC0007FF, 0x48000049,	"e,f,g", 1 },

{ "fisub.dd",	0xFC0007FF, 0x480001CD,	"E,F,G", 1 },
{ "fisub.ss",	0xFC0007FF, 0x4800004D,	"e,f,g", 1 },

{ "fix.dd",	0xFC0007FF, 0x480001B2,	"E,G", 1 },
{ "fix.sd",	0xFC0007FF, 0x480000B2,	"e,G", 1 },
/* { "fix.ss",	0xFC0007FF, 0x48000032,	"e,g", 1 }, */	/* Not supported by Intel */

{ "fld.d",	0xFC000007, 0x20000000, "1(2),G", 0 },
{ "fld.d",	0xFC000007, 0x20000001, "1(2)++,G", 0 },
{ "fld.d",	0xFC000007, 0x24000000, "l(2),G", 0 },
{ "fld.d",	0xFC000007, 0x24000001, "l(2)++,G", 1 },
{ "fld.l",	0xFC000003, 0x20000002, "1(2),g", 0 },
{ "fld.l",	0xFC000003, 0x20000003, "1(2)++,g", 0 },
{ "fld.l",	0xFC000003, 0x24000002, "k(2),g", 0 },
{ "fld.l",	0xFC000003, 0x24000003, "k(2)++,g", 1 },
{ "fld.q",	0xFC000007, 0x20000004, "1(2),H", 0 },
{ "fld.q",	0xFC000007, 0x20000005, "1(2)++,H", 0 },
{ "fld.q",	0xFC000007, 0x24000004, "m(2),H", 0 },
{ "fld.q",	0xFC000007, 0x24000005, "m(2)++,H", 1 },

{ "flush",	0xFC000007, 0x34000000, "m(2)", 0 },
{ "flush",	0xFC000007, 0x34000001, "m(2)++", 1 },

{ "fmlow.dd",	0xFC0007FF, 0x480001A1,	"E,F,G", 1 },
/* { "fmlow.sd",	0xFC0007FF, 0x480000A1,	"e,f,g", 1 }, */ /* Not supported... */
/* { "fmlow.ss",	0xFC0007FF, 0x48000021,	"e,f,g", 1 }, */

{ "fmov.dd",	0xFFE007FF, 0x480001C9,	"E,G", 1 },
{ "fmov.ds",	0xFFE007FF, 0x48000133,	"E,g", 1 },	/* Update B.0 4.0 errata */
{ "famov.ds",	0xFFE007FF, 0x48000133,	"E,g", 1 },	/* B.0 insn */
{ "fmov.sd",	0xFFE007FF, 0x480000B3,	"e,G", 1 },	/* Update B.0 4.0 errata */
{ "famov.sd",	0xFFE007FF, 0x480000B3,	"e,G", 1 },	/* B.0 insn */
{ "famov.ss",	0xFFE007FF, 0x48000033,	"e,g", 1 },	/* B.0 insn */
{ "famov.dd",	0xFFE007FF, 0x480001B3,	"E,G", 1 },	/* B.0 insn */
{ "fmov.ss",	0xFFE007FF, 0x48000049,	"e,g", 1 },

{ "fmul.dd",	0xFC0007FF, 0x480001A0,	"E,F,G", 1 },
{ "fmul.sd",	0xFC0007FF, 0x480000A0,	"e,f,G", 1 },
{ "fmul.ss",	0xFC0007FF, 0x48000020,	"e,f,g", 1 },

{ "fnop",	0xFFFFFFFF, 0xB0000000, "", 1 },

{ "form",	0xFC0007FF, 0x480001DA, "E,G", 1 },

{ "frcp.dd",	0xFC0007FF, 0x480001A2,	"F,G", 1 },
{ "frcp.sd",	0xFC0007FF, 0x480000A2,	"f,G", 1 },
{ "frcp.ss",	0xFC0007FF, 0x48000022,	"f,g", 1 },

{ "frsqr.dd",	0xFC0007FF, 0x480001A3,	"F,G", 1 },
{ "frsqr.sd",	0xFC0007FF, 0x480000A3,	"f,G", 1 },
{ "frsqr.ss",	0xFC0007FF, 0x48000023,	"f,g", 1 },

{ "fst.d",	0xFC000007, 0x28000000, "G,1(2)", 0 },
{ "fst.d",	0xFC000007, 0x28000001, "G,1(2)++", 0 },
{ "fst.d",	0xFC000007, 0x2C000000, "G,l(2)", 0 },
{ "fst.d",	0xFC000007, 0x2C000001, "G,l(2)++", 1 },
{ "fst.l",	0xFC000003, 0x28000002, "g,1(2)", 0 },
{ "fst.l",	0xFC000003, 0x28000003, "g,1(2)++", 0 },
{ "fst.l",	0xFC000003, 0x2C000002, "g,k(2)", 0 },
{ "fst.l",	0xFC000003, 0x2C000003, "g,k(2)++", 1 },
{ "fst.q",	0xFC000007, 0x28000004, "H,1(2)", 0 },
{ "fst.q",	0xFC000007, 0x28000005, "H,1(2)++", 0 },
{ "fst.q",	0xFC000007, 0x2C000004, "H,m(2)", 0 },
{ "fst.q",	0xFC000007, 0x2C000005, "H,m(2)++", 1 },

{ "fsub.dd",	0xFC0007FF, 0x480001B1,	"E,F,G", 1 },
{ "fsub.sd",	0xFC0007FF, 0x480000B1,	"e,f,G", 1 },
{ "fsub.ss",	0xFC0007FF, 0x48000031,	"e,f,g", 1 },

{ "ftrunc.dd",	0xFC0007FF, 0x480001BA,	"E,G", 1 },
{ "ftrunc.sd",	0xFC0007FF, 0x480000BA,	"e,G", 1 },
/* { "ftrunc.ss",	0xFC0007FF, 0x4800003A,	"e,g", 1 }, */  /* Not supported... */

{ "fxfr",	0xFC0007FF, 0x48000040,	"e,d", 1 },

{ "fzchkl",	0xFC0007FF, 0x480001D7, "E,F,G", 1 },

{ "fzchks",	0xFC0007FF, 0x480001DF,	"E,F,G", 1 },

{ "intovr",	0xFC00001F, 0x4C000004,	"", 1 },

{ "ixfr",	0xFC000000, 0x08000000, "1,g", 1 },

{ "ld.b",	0xFC000000, 0x00000000, "1(2),d", 0 },
{ "ld.b",	0xFC000000, 0x04000000, "i(2),d", 1 },
{ "ld.c",	0xFC000000, 0x30000000, "C,d", 1 },
{ "ld.l",	0xFC000001, 0x10000001, "1(2),d", 0 },
{ "ld.l",	0xFC000001, 0x14000001, "k(2),d", 1 },
{ "ld.s",	0xFC000001, 0x10000000, "1(2),d", 0 },
{ "ld.s",	0xFC000001, 0x14000000, "j(2),d", 1 },

{ "lock",	0xFC00001F, 0x4C000001,	"", 1 },

{ "nop",	0xFFFFFFFF, 0xA0000000, "", 1 },

{ "mov",	0xFC00F800, 0xA0000000, "2,d", 1 },

{ "or",		0xFC000000, 0xE0000000,	"1,2,d", 0 },
{ "or",		0xFC000000, 0xE4000000,	"i,2,d", 1 },
{ "orh",	0xFC000000, 0xEC000000,	"i,2,d", 1 },

{ "pfadd.dd",	0xFC0007FF, 0x480005B0,	"E,F,G", 1 },
{ "pfadd.sd",	0xFC0007FF, 0x480004B0,	"e,f,G", 1 },
{ "pfadd.ss",	0xFC0007FF, 0x48000430,	"e,f,g", 1 },

{ "pfaddp",	0xFC0007FF, 0x480005D0,	"E,F,G", 1 },

{ "pfaddz",	0xFC0007FF, 0x480005D1,	"E,F,G", 1 },

{ "pfeq.dd",	0xFC0007FF, 0x48000535, "E,F,G", 1 },
{ "pfeq.ss",	0xFC0007FF, 0x48000435, "e,f,g", 1 },
{ "pfeq.sd",	0xFC0007FF, 0x48000435, "e,f,G", 1 },

{ "pfgt.dd",	0xFC0007FF, 0x48000534, "E,F,G", 1 },
{ "pfgt.ss",	0xFC0007FF, 0x48000434, "e,f,g", 1 },
{ "pfgt.sd",	0xFC0007FF, 0x48000434, "e,f,G", 1 },

{ "pfiadd.dd",	0xFC0007FF, 0x480005C9,	"E,F,G", 1 },
{ "pfiadd.ss",	0xFC0007FF, 0x48000449,	"e,f,g", 1 },

{ "pfisub.dd",	0xFC0007FF, 0x480005CD,	"E,F,G", 1 },
{ "pfisub.ss",	0xFC0007FF, 0x4800044D,	"e,f,g", 1 },

{ "pfix.dd",	0xFC0007FF, 0x480005B2,	"E,G", 1 },
{ "pfix.sd",	0xFC0007FF, 0x480004B2,	"e,G", 1 },
/* { "pfix.ss",	0xFC0007FF, 0x48000432,	"e,g", 1 }, */  /* Not supported... */

{ "pfld.d",	0xFC000007, 0x60000000, "1(2),G", 0 },
{ "pfld.d",	0xFC000007, 0x60000001, "1(2)++,G", 0 },
{ "pfld.d",	0xFC000007, 0x64000000, "l(2),G", 0 },
{ "pfld.d",	0xFC000007, 0x64000001, "l(2)++,G", 1 },
{ "pfld.l",	0xFC000003, 0x60000002, "1(2),g", 0 },
{ "pfld.l",	0xFC000003, 0x60000003, "1(2)++,g", 0 },
{ "pfld.l",	0xFC000003, 0x64000002, "k(2),g", 0 },
{ "pfld.l",	0xFC000003, 0x64000003, "k(2)++,g", 1 },

{ "pfle.dd",	0xFC0007FF, 0x480005b4, "E,F,G", 1 },
{ "pfle.sd",	0xFC0007FF, 0x480004b4, "e,f,G", 1 },
{ "pfle.ss",	0xFC0007FF, 0x480004b4, "e,f,g", 1 },

{ "pfmov.dd",	0xFFE007FF, 0x480005C9,	"E,G", 1 },
{ "pfmov.ds",	0xFFE007FF, 0x48000533,	"E,g", 1 },	/* Update B.0 4.0 errata */
{ "pfamov.ds",	0xFFE007FF, 0x48000533,	"E,g", 1 },	/* B.0 insn */
{ "pfmov.sd",	0xFFE007FF, 0x480004B3,	"e,G", 1 },	/* Update B.0 4.0 errata */
{ "pfamov.sd",	0xFFE007FF, 0x480004B3,	"e,G", 1 },	/* B.0 insn */
{ "pfamov.ss",	0xFFE007FF, 0x48000433,	"e,g", 1 },	/* B.0 insn */
{ "pfamov.dd",	0xFFE007FF, 0x480005B3,	"E,G", 1 },	/* B.0 insn */
{ "pfmov.ss",	0xFFE007FF, 0x48000449,	"e,g", 1 },

{ "pfmul.dd",	0xFC0007FF, 0x480005A0,	"E,F,G", 1 },
{ "pfmul.sd",	0xFC0007FF, 0x480004A0,	"e,f,G", 1 },
{ "pfmul.ss",	0xFC0007FF, 0x48000420,	"e,f,g", 1 },

{ "pfmul3.dd",	0xFC0007FF, 0x480005A4,	"E,F,G", 1 },
/* { "pfmul3.sd",	0xFC0007FF, 0x480004A4,	"e,f,g", 1 }, */ /* Not supported... */
/* { "pfmul3.ss",	0xFC0007FF, 0x48000424,	"e,f,g", 1 }, */

{ "pform",	0xFC0007FF, 0x480005DA, "E,G", 1 },

{ "pfsub.dd",	0xFC0007FF, 0x480005B1,	"E,F,G", 1 },
{ "pfsub.sd",	0xFC0007FF, 0x480004B1,	"e,f,G", 1 },
{ "pfsub.ss",	0xFC0007FF, 0x48000431,	"e,f,g", 1 },

{ "pftrunc.dd",	0xFC0007FF, 0x480005BA,	"E,G", 1 },
{ "pftrunc.sd",	0xFC0007FF, 0x480004BA,	"e,G", 1 },
/* { "pftrunc.ss",	0xFC0007FF, 0x4800043A,	"e,g", 1 }, */  /* Not supported... */

{ "pfzchkl",	0xFC0007FF, 0x480005D7, "E,F,G", 1 },

{ "pfzchks",	0xFC0007FF, 0x480005DF,	"E,F,G", 1 },

{ "pst.d",	0xFC000007, 0x3C000000, "G,l(2)", 0 },
{ "pst.d",	0xFC000007, 0x3C000001, "G,l(2)++", 1 },

{ "shl",	0xFC000000, 0xA0000000, "1,2,d", 0 },
{ "shl",	0xFC000000, 0xA4000000, "D,2,d", 1 },

{ "shr",	0xFC000000, 0xA8000000, "1,2,d", 0 },
{ "shr",	0xFC000000, 0xAC000000, "D,2,d", 1 },
{ "shra",	0xFC000000, 0xB8000000, "1,2,d", 0 },
{ "shra",	0xFC000000, 0xBC000000, "D,2,d", 1 },
{ "shrd",	0xFC000000, 0xB0000000, "1,2,d", 1 },

{ "st.b",	0xFC000000, 0x0C000000, "1,n(2)", 1 },
{ "st.c",	0xFC000000, 0x38000000, "1,C", 1 },
{ "st.l",	0xFC000001, 0x1C000001, "1,p(2)", 1 },
{ "st.s",	0xFC000001, 0x1C000000, "1,o(2)", 1 },

{ "subs",	0xFC000000, 0x98000000,	"1,2,d", 0 },
{ "subs",	0xFC000000, 0x9C000000,	"i,2,d", 1 },
{ "subu",	0xFC000000, 0x88000000,	"1,2,d", 0 },
{ "subu",	0xFC000000, 0x8C000000,	"i,2,d", 1 },

{ "trap",	0xFC000000, 0x44000000, "1,2,d", 1 },

{ "unlock",	0xFC00001F, 0x4C000007,	"", 1 },

{ "xor",	0xFC000000, 0xF0000000,	"1,2,d", 0 },
{ "xor",	0xFC000000, 0xF4000000,	"i,2,d", 1 },
{ "xorh",	0xFC000000, 0xFC000000,	"i,2,d", 1 },


/*
 *	      Pipelined Accumulate/Multiply Operators
 *
 *		Abandon all hope, ye who enter here...
 *
 * PFAM operators.  There are 48 of these horrors.  Don't ask....
 */
{ "r2p1.dd",	0xFC0007FF, 0x48000580,	"E,F,G", 1 },
{ "r2pt.dd",	0xFC0007FF, 0x48000581,	"E,F,G", 1 },
{ "r2ap1.dd",	0xFC0007FF, 0x48000582,	"E,F,G", 1 },
{ "r2apt.dd",	0xFC0007FF, 0x48000583,	"E,F,G", 1 },
{ "i2p1.dd",	0xFC0007FF, 0x48000584,	"E,F,G", 1 },
{ "i2pt.dd",	0xFC0007FF, 0x48000585,	"E,F,G", 1 },
{ "i2ap1.dd",	0xFC0007FF, 0x48000586,	"E,F,G", 1 },
{ "i2apt.dd",	0xFC0007FF, 0x48000587,	"E,F,G", 1 },
{ "rat1p2.dd",	0xFC0007FF, 0x48000588,	"E,F,G", 1 },
{ "m12apm.dd",	0xFC0007FF, 0x48000589,	"E,F,G", 1 },
{ "ra1p2.dd",	0xFC0007FF, 0x4800058A,	"E,F,G", 1 },
{ "m12ttpa.dd",	0xFC0007FF, 0x4800058B,	"E,F,G", 1 },
{ "iat1p2.dd",	0xFC0007FF, 0x4800058C,	"E,F,G", 1 },
{ "m12tpm.dd",	0xFC0007FF, 0x4800058D,	"E,F,G", 1 },
{ "ia1p2.dd",	0xFC0007FF, 0x4800058E,	"E,F,G", 1 },
{ "m12tpa.dd",	0xFC0007FF, 0x4800058F,	"E,F,G", 1 },

{ "r2p1.sd",	0xFC0007FF, 0x48000480,	"e,f,G", 1 },
{ "r2pt.sd",	0xFC0007FF, 0x48000481,	"e,f,G", 1 },
{ "r2ap1.sd",	0xFC0007FF, 0x48000482,	"e,f,G", 1 },
{ "r2apt.sd",	0xFC0007FF, 0x48000483,	"e,f,G", 1 },
{ "i2p1.sd",	0xFC0007FF, 0x48000484,	"e,f,G", 1 },
{ "i2pt.sd",	0xFC0007FF, 0x48000485,	"e,f,G", 1 },
{ "i2ap1.sd",	0xFC0007FF, 0x48000486,	"e,f,G", 1 },
{ "i2apt.sd",	0xFC0007FF, 0x48000487,	"e,f,G", 1 },
{ "rat1p2.sd",	0xFC0007FF, 0x48000488,	"e,f,G", 1 },
{ "m12apm.sd",	0xFC0007FF, 0x48000489,	"e,f,G", 1 },
{ "ra1p2.sd",	0xFC0007FF, 0x4800048A,	"e,f,G", 1 },
{ "m12ttpa.sd",	0xFC0007FF, 0x4800048B,	"e,f,G", 1 },
{ "iat1p2.sd",	0xFC0007FF, 0x4800048C,	"e,f,G", 1 },
{ "m12tpm.sd",	0xFC0007FF, 0x4800048D,	"e,f,G", 1 },
{ "ia1p2.sd",	0xFC0007FF, 0x4800048E,	"e,f,G", 1 },
{ "m12tpa.sd",	0xFC0007FF, 0x4800048F,	"e,f,G", 1 },

{ "r2p1.ss",	0xFC0007FF, 0x48000400,	"e,f,g", 1 },
{ "r2pt.ss",	0xFC0007FF, 0x48000401,	"e,f,g", 1 },
{ "r2ap1.ss",	0xFC0007FF, 0x48000402,	"e,f,g", 1 },
{ "r2apt.ss",	0xFC0007FF, 0x48000403,	"e,f,g", 1 },
{ "i2p1.ss",	0xFC0007FF, 0x48000404,	"e,f,g", 1 },
{ "i2pt.ss",	0xFC0007FF, 0x48000405,	"e,f,g", 1 },
{ "i2ap1.ss",	0xFC0007FF, 0x48000406,	"e,f,g", 1 },
{ "i2apt.ss",	0xFC0007FF, 0x48000407,	"e,f,g", 1 },
{ "rat1p2.ss",	0xFC0007FF, 0x48000408,	"e,f,g", 1 },
{ "m12apm.ss",	0xFC0007FF, 0x48000409,	"e,f,g", 1 },
{ "ra1p2.ss",	0xFC0007FF, 0x4800040A,	"e,f,g", 1 },
{ "m12ttpa.ss",	0xFC0007FF, 0x4800040B,	"e,f,g", 1 },
{ "iat1p2.ss",	0xFC0007FF, 0x4800040C,	"e,f,g", 1 },
{ "m12tpm.ss",	0xFC0007FF, 0x4800040D,	"e,f,g", 1 },
{ "ia1p2.ss",	0xFC0007FF, 0x4800040E,	"e,f,g", 1 },
{ "m12tpa.ss",	0xFC0007FF, 0x4800040F,	"e,f,g", 1 },

/*
 * PFMAM operators.  There are 48 of these.
 */
{ "mr2p1.dd",	0xFC0007FF, 0x48000180,	"E,F,G", 1 },
{ "mr2pt.dd",	0xFC0007FF, 0x48000181,	"E,F,G", 1 },
{ "mr2mp1.dd",	0xFC0007FF, 0x48000182,	"E,F,G", 1 },
{ "mr2mpt.dd",	0xFC0007FF, 0x48000183,	"E,F,G", 1 },
{ "mi2p1.dd",	0xFC0007FF, 0x48000184,	"E,F,G", 1 },
{ "mi2pt.dd",	0xFC0007FF, 0x48000185,	"E,F,G", 1 },
{ "mi2mp1.dd",	0xFC0007FF, 0x48000186,	"E,F,G", 1 },
{ "mi2mpt.dd",	0xFC0007FF, 0x48000187,	"E,F,G", 1 },
{ "mrmt1p2.dd",	0xFC0007FF, 0x48000188,	"E,F,G", 1 },
{ "mm12mpm.dd",	0xFC0007FF, 0x48000189,	"E,F,G", 1 },
{ "mrm1p2.dd",	0xFC0007FF, 0x4800018A,	"E,F,G", 1 },
{ "mm12ttpm.dd",0xFC0007FF, 0x4800018B,	"E,F,G", 1 },
{ "mimt1p2.dd",	0xFC0007FF, 0x4800018C,	"E,F,G", 1 },
{ "mm12tpm.dd",	0xFC0007FF, 0x4800018D,	"E,F,G", 1 },
{ "mim1p2.dd",	0xFC0007FF, 0x4800018E,	"E,F,G", 1 },
/* { "mm12tpm.dd",	0xFC0007FF, 0x4800018F,	"E,F,G", 1 }, */  /* ??? */

{ "mr2p1.sd",	0xFC0007FF, 0x48000080,	"e,f,G", 1 },
{ "mr2pt.sd",	0xFC0007FF, 0x48000081,	"e,f,G", 1 },
{ "mr2mp1.sd",	0xFC0007FF, 0x48000082,	"e,f,G", 1 },
{ "mr2mpt.sd",	0xFC0007FF, 0x48000083,	"e,f,G", 1 },
{ "mi2p1.sd",	0xFC0007FF, 0x48000084,	"e,f,G", 1 },
{ "mi2pt.sd",	0xFC0007FF, 0x48000085,	"e,f,G", 1 },
{ "mi2mp1.sd",	0xFC0007FF, 0x48000086,	"e,f,G", 1 },
{ "mi2mpt.sd",	0xFC0007FF, 0x48000087,	"e,f,G", 1 },
{ "mrmt1p2.sd",	0xFC0007FF, 0x48000088,	"e,f,G", 1 },
{ "mm12mpm.sd",	0xFC0007FF, 0x48000089,	"e,f,G", 1 },
{ "mrm1p2.sd",	0xFC0007FF, 0x4800008A,	"e,f,G", 1 },
{ "mm12ttpm.sd",0xFC0007FF, 0x4800008B,	"e,f,G", 1 },
{ "mimt1p2.sd",	0xFC0007FF, 0x4800008C,	"e,f,G", 1 },
{ "mm12tpm.sd",	0xFC0007FF, 0x4800008D,	"e,f,G", 1 },
{ "mim1p2.sd",	0xFC0007FF, 0x4800008E,	"e,f,G", 1 },
/* { "mm12tpm.sd",	0xFC0007FF, 0x4800008F,	"e,f,G", 1 }, */   /* ??? */

{ "mr2p1.ss",	0xFC0007FF, 0x48000000,	"e,f,g", 1 },
{ "mr2pt.ss",	0xFC0007FF, 0x48000001,	"e,f,g", 1 },
{ "mr2mp1.ss",	0xFC0007FF, 0x48000002,	"e,f,g", 1 },
{ "mr2mpt.ss",	0xFC0007FF, 0x48000003,	"e,f,g", 1 },
{ "mi2p1.ss",	0xFC0007FF, 0x48000004,	"e,f,g", 1 },
{ "mi2pt.ss",	0xFC0007FF, 0x48000005,	"e,f,g", 1 },
{ "mi2mp1.ss",	0xFC0007FF, 0x48000006,	"e,f,g", 1 },
{ "mi2mpt.ss",	0xFC0007FF, 0x48000007,	"e,f,g", 1 },
{ "mrmt1p2.ss",	0xFC0007FF, 0x48000008,	"e,f,g", 1 },
{ "mm12mpm.ss",	0xFC0007FF, 0x48000009,	"e,f,g", 1 },
{ "mrm1p2.ss",	0xFC0007FF, 0x4800000A,	"e,f,g", 1 },
{ "mm12ttpm.ss",0xFC0007FF, 0x4800000B,	"e,f,g", 1 },
{ "mimt1p2.ss",	0xFC0007FF, 0x4800000C,	"e,f,g", 1 },
{ "mm12tpm.ss",	0xFC0007FF, 0x4800000D,	"e,f,g", 1 },
{ "mim1p2.ss",	0xFC0007FF, 0x4800000E,	"e,f,g", 1 },
/* { "mm12tpm.ss",	0xFC0007FF, 0x4800000F,	"e,f,g", 1 }, */  /* ??? */

/*
 * PFMSM operators.  There are 48 of these.
 */
{ "mr2s1.dd",	0xFC0007FF, 0x48000190,	"E,F,G", 1 },
{ "mr2st.dd",	0xFC0007FF, 0x48000191,	"E,F,G", 1 },
{ "mr2ms1.dd",	0xFC0007FF, 0x48000192,	"E,F,G", 1 },
{ "mr2mst.dd",	0xFC0007FF, 0x48000193,	"E,F,G", 1 },
{ "mi2s1.dd",	0xFC0007FF, 0x48000194,	"E,F,G", 1 },
{ "mi2st.dd",	0xFC0007FF, 0x48000195,	"E,F,G", 1 },
{ "mi2ms1.dd",	0xFC0007FF, 0x48000196,	"E,F,G", 1 },
{ "mi2mst.dd",	0xFC0007FF, 0x48000197,	"E,F,G", 1 },
{ "mrmt1s2.dd",	0xFC0007FF, 0x48000198,	"E,F,G", 1 },
{ "mm12msm.dd",	0xFC0007FF, 0x48000199,	"E,F,G", 1 },
{ "mrm1s2.dd",	0xFC0007FF, 0x4800019A,	"E,F,G", 1 },
{ "mm12ttsm.dd",0xFC0007FF, 0x4800019B,	"E,F,G", 1 },
{ "mimt1s2.dd",	0xFC0007FF, 0x4800019C,	"E,F,G", 1 },
{ "mm12tsm.dd",	0xFC0007FF, 0x4800019D,	"E,F,G", 1 },
{ "mim1s2.dd",	0xFC0007FF, 0x4800019E,	"E,F,G", 1 },
/* { "mm12tsm.dd",	0xFC0007FF, 0x4800019F,	"E,F,G", 1 }, */  /* ??? */

{ "mr2s1.sd",	0xFC0007FF, 0x48000090,	"e,f,G", 1 },
{ "mr2st.sd",	0xFC0007FF, 0x48000091,	"e,f,G", 1 },
{ "mr2ms1.sd",	0xFC0007FF, 0x48000092,	"e,f,G", 1 },
{ "mr2mst.sd",	0xFC0007FF, 0x48000093,	"e,f,G", 1 },
{ "mi2s1.sd",	0xFC0007FF, 0x48000094,	"e,f,G", 1 },
{ "mi2st.sd",	0xFC0007FF, 0x48000095,	"e,f,G", 1 },
{ "mi2ms1.sd",	0xFC0007FF, 0x48000096,	"e,f,G", 1 },
{ "mi2mst.sd",	0xFC0007FF, 0x48000097,	"e,f,G", 1 },
{ "mrmt1s2.sd",	0xFC0007FF, 0x48000098,	"e,f,G", 1 },
{ "mm12msm.sd",	0xFC0007FF, 0x48000099,	"e,f,G", 1 },
{ "mrm1s2.sd",	0xFC0007FF, 0x4800009A,	"e,f,G", 1 },
{ "mm12ttsm.sd",0xFC0007FF, 0x4800009B,	"e,f,G", 1 },
{ "mimt1s2.sd",	0xFC0007FF, 0x4800009C,	"e,f,G", 1 },
{ "mm12tsm.sd",	0xFC0007FF, 0x4800009D,	"e,f,G", 1 },
{ "mim1s2.sd",	0xFC0007FF, 0x4800009E,	"e,f,G", 1 },
/* { "mm12tsm.sd",	0xFC0007FF, 0x4800009F,	"e,f,G", 1 }, */  /* ??? */

{ "mr2s1.ss",	0xFC0007FF, 0x48000010,	"e,f,g", 1 },
{ "mr2st.ss",	0xFC0007FF, 0x48000011,	"e,f,g", 1 },
{ "mr2ms1.ss",	0xFC0007FF, 0x48000012,	"e,f,g", 1 },
{ "mr2mst.ss",	0xFC0007FF, 0x48000013,	"e,f,g", 1 },
{ "mi2s1.ss",	0xFC0007FF, 0x48000014,	"e,f,g", 1 },
{ "mi2st.ss",	0xFC0007FF, 0x48000015,	"e,f,g", 1 },
{ "mi2ms1.ss",	0xFC0007FF, 0x48000016,	"e,f,g", 1 },
{ "mi2mst.ss",	0xFC0007FF, 0x48000017,	"e,f,g", 1 },
{ "mrmt1s2.ss",	0xFC0007FF, 0x48000018,	"e,f,g", 1 },
{ "mm12msm.ss",	0xFC0007FF, 0x48000019,	"e,f,g", 1 },
{ "mrm1s2.ss",	0xFC0007FF, 0x4800001A,	"e,f,g", 1 },
{ "mm12ttsm.ss",0xFC0007FF, 0x4800001B,	"e,f,g", 1 },
{ "mimt1s2.ss",	0xFC0007FF, 0x4800001C,	"e,f,g", 1 },
{ "mm12tsm.ss",	0xFC0007FF, 0x4800001D,	"e,f,g", 1 },
{ "mim1s2.ss",	0xFC0007FF, 0x4800001E,	"e,f,g", 1 },
/* { "mm12tsm.ss",	0xFC0007FF, 0x4800001F,	"e,f,g", 1 }, */  /* ??? */

/*
 * PFSM operators.  There are 48 of these.
 */
{ "r2s1.dd",	0xFC0007FF, 0x48000590,	"E,F,G", 1 },
{ "r2st.dd",	0xFC0007FF, 0x48000591,	"E,F,G", 1 },
{ "r2as1.dd",	0xFC0007FF, 0x48000592,	"E,F,G", 1 },
{ "r2ast.dd",	0xFC0007FF, 0x48000593,	"E,F,G", 1 },
{ "i2s1.dd",	0xFC0007FF, 0x48000594,	"E,F,G", 1 },
{ "i2st.dd",	0xFC0007FF, 0x48000595,	"E,F,G", 1 },
{ "i2as1.dd",	0xFC0007FF, 0x48000596,	"E,F,G", 1 },
{ "i2ast.dd",	0xFC0007FF, 0x48000597,	"E,F,G", 1 },
{ "rat1s2.dd",	0xFC0007FF, 0x48000598,	"E,F,G", 1 },
{ "m12asm.dd",	0xFC0007FF, 0x48000599,	"E,F,G", 1 },
{ "ra1s2.dd",	0xFC0007FF, 0x4800059A,	"E,F,G", 1 },
{ "m12ttsa.dd",	0xFC0007FF, 0x4800059B,	"E,F,G", 1 },
{ "iat1s2.dd",	0xFC0007FF, 0x4800059C,	"E,F,G", 1 },
{ "m12tsm.dd",	0xFC0007FF, 0x4800059D,	"E,F,G", 1 },
{ "ia1s2.dd",	0xFC0007FF, 0x4800059E,	"E,F,G", 1 },
{ "m12tsa.dd",	0xFC0007FF, 0x4800059F,	"E,F,G", 1 },

{ "r2s1.sd",	0xFC0007FF, 0x48000490,	"e,f,G", 1 },
{ "r2st.sd",	0xFC0007FF, 0x48000491,	"e,f,G", 1 },
{ "r2as1.sd",	0xFC0007FF, 0x48000492,	"e,f,G", 1 },
{ "r2ast.sd",	0xFC0007FF, 0x48000493,	"e,f,G", 1 },
{ "i2s1.sd",	0xFC0007FF, 0x48000494,	"e,f,G", 1 },
{ "i2st.sd",	0xFC0007FF, 0x48000495,	"e,f,G", 1 },
{ "i2as1.sd",	0xFC0007FF, 0x48000496,	"e,f,G", 1 },
{ "i2ast.sd",	0xFC0007FF, 0x48000497,	"e,f,G", 1 },
{ "rat1s2.sd",	0xFC0007FF, 0x48000498,	"e,f,G", 1 },
{ "m12asm.sd",	0xFC0007FF, 0x48000499,	"e,f,G", 1 },
{ "ra1s2.sd",	0xFC0007FF, 0x4800049A,	"e,f,G", 1 },
{ "m12ttsa.sd",	0xFC0007FF, 0x4800049B,	"e,f,G", 1 },
{ "iat1s2.sd",	0xFC0007FF, 0x4800049C,	"e,f,G", 1 },
{ "m12tsm.sd",	0xFC0007FF, 0x4800049D,	"e,f,G", 1 },
{ "ia1s2.sd",	0xFC0007FF, 0x4800049E,	"e,f,G", 1 },
{ "m12tsa.sd",	0xFC0007FF, 0x4800049F,	"e,f,G", 1 },

{ "r2s1.ss",	0xFC0007FF, 0x48000410,	"e,f,g", 1 },
{ "r2st.ss",	0xFC0007FF, 0x48000411,	"e,f,g", 1 },
{ "r2as1.ss",	0xFC0007FF, 0x48000412,	"e,f,g", 1 },
{ "r2ast.ss",	0xFC0007FF, 0x48000413,	"e,f,g", 1 },
{ "i2s1.ss",	0xFC0007FF, 0x48000414,	"e,f,g", 1 },
{ "i2st.ss",	0xFC0007FF, 0x48000415,	"e,f,g", 1 },
{ "i2as1.ss",	0xFC0007FF, 0x48000416,	"e,f,g", 1 },
{ "i2ast.ss",	0xFC0007FF, 0x48000417,	"e,f,g", 1 },
{ "rat1s2.ss",	0xFC0007FF, 0x48000418,	"e,f,g", 1 },
{ "m12asm.ss",	0xFC0007FF, 0x48000419,	"e,f,g", 1 },
{ "ra1s2.ss",	0xFC0007FF, 0x4800041A,	"e,f,g", 1 },
{ "m12ttsa.ss",	0xFC0007FF, 0x4800041B,	"e,f,g", 1 },
{ "iat1s2.ss",	0xFC0007FF, 0x4800041C,	"e,f,g", 1 },
{ "m12tsm.ss",	0xFC0007FF, 0x4800041D,	"e,f,g", 1 },
{ "ia1s2.ss",	0xFC0007FF, 0x4800041E,	"e,f,g", 1 },
{ "m12tsa.ss",	0xFC0007FF, 0x4800041F,	"e,f,g", 1 },

};

#define NUMOPCODES ((sizeof i860_opcodes)/(sizeof *i860_opcodes))
