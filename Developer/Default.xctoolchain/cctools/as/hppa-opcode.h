/*    hppa-opcode.h    */
/* Table of opcodes for the hppa.
   Copyright (C) 1990 Free Software Foundation, Inc.

This file is part of GAS, the GNU Assembler, and GDB, the GNU disassembler.

GAS/GDB is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 1, or (at your option)
any later version.

GAS/GDB is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GAS or GDB; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

/*
   HP PA-RISC support was contributed by the Center for Software Science
   at the University of Utah.
 */

/* HP-PA support for Mach-O ... USV */

#ifndef HPPA_OPCODE_INCLUDED
#define HPPA_OPCODE_INCLUDED

#include <stdint.h>

#if !defined(__STDC__) && !defined(const)
#define const
#endif

/* bug #41317 .... umeshv@NeXT.com Mon May  2 17:53:29 PDT 1994 */
/* The masks for 2 bits at 20 */

#define  NO_CACHE_CONTROL_HINT  (uint32_t)0x0U
#define  BC_OR_CO_CACHE_CONTROL_HINT  (uint32_t)0x400U

/************************************************************************
 * The parsing characters used in the opcode table are listed in ASCII
 * order. This is updated list as of Wed Jul 27 14:15:09 PDT 1994
 * --- Umesh.
 ***************** Begin List *******************************************
 !		negated or non-negated add conditions (used only by 'addb' and 'addib' pseudo-instructions)
 &		logical instruction conditions
 +		non-negated add conditions
 -		compare/subtract conditions
 0		10 bit Special Function Unit operation split between 5 bits at 20 and 5 bits at 31
 1		15 bit Special Function Unit operation split between 10 bits at 20 and 5 bits at 31
 2		22 bit Special Function Unit operation split between 17 bits at 20 and 5 bits at 31
 3		Store Instruction Cache Control Hint  (Table 5-8) with Indexed load completers (Table 5-10)
 4		a variation of the 'b' operand type for 'fmpyadd' and 'fmpysub'
 5		5 bit immediate at 15.
 6		a variation of the 'x' operand type for 'fmpyadd' and 'fmpysub'
 7		a variation of the 't' operand type for 'fmpyadd' and 'fmpysub'
 8		5 bit register field at 20 (used in 'fmpyadd' and 'fmpysub')
 9		5 bit register field at 25 (used in 'fmpyadd' and 'fmpysub')
 <		non-negated compare/subtract conditions.
 >		shift/extract/deposit conditions.
 ?		negated or non-negated compare/subtract conditions (used only by 'comb' and 'comib' pseudo-instructions)
 @		17 bit branch displacement --- JBSR
 A		13 bit immediate at 18 (to support the BREAK instruction)
 B		either s,b or b where 's' 2 bit space specifier at 17 and 'b' register field at 10.
 C		short load and store completer.
 D		26 bit immediate at 31 (to support the DIAG instruction)
 E		a 'b' operand type extended to handle L/R register halves.
 F		Source Floating Point Operand Format Completer encoded 2 bits at 20
 G		Destination Floating Point Operand Format Completer encoded 2 bits at 18
 H		Floating Point Operand Format at 26 for 'fmpyadd' and 'fmpysub' (very similar to 'F')
 L		Load and Clear Word Cache Control Hint with Indexed load completers
 M		Floating-Point Compare Conditions (encoded as 5 bits at 31)
 O		20 bit Special Function Unit operation split between 15 bits at 20 and 5 bits at 31
 P		5 bit bit position at 26
 Q		5 bit immediate value at 10 (a bit position specified in the bb instruction. It's the same as r above, except the value is in a different location)
 R		5 bit immediate value at 15 (for the ssm, rsm instruction) (same as r above, except the value is in a different location)
 S		3 bit space specifier at 18.
 T		5 bit field length at 31 (encoded as 32-T)
 U		unit instruction conditions
 V		5 bit immediate value at 31
 W		17 bit branch displacement (pc-rel for BL and GATE)
 X		an 'x' operand type extended to handle L/R register halves.
 Y		Store Bytes Short completer with cache control hints
 Z		System Control Completer (to support LDA, LHA, etc.)
 a		Load and Clear Word Cache Control Hint with Short displacement load and store completers
 b		register field at 10.
 c		indexed load completer.
 f		3 bit Special Function Unit identifier at 25
 i		11 bit immediate value at 31
 j		14 bit immediate value at 31
 k		21 bit immediate value at 31
 l		Store Instruction Cache Control Hint with Short displacement load and store completers 
 n		nullification for branch instructions
 o		15 bit Special Function Unit operation at 20
 p		5 bit shift count at 26 (to support the SHD instruction) encoded as (31-p)
 r		5 bit immediate value at 31 (for the break instruction) (very similar to V above, except the value is unsigned instead of low_sign_ext)
 s		2 bit space specifier at 17.
 t		register field at 31.
 u		3 bit coprocessor unit identifier at 25
 v		a 't' operand type extended to handle L/R register halves.
 w		12 bit branch displacement
 x		register field at 15.
 y		nullification at 26
 z		17 bit branch displacement (not pc-rel)
 ~		bvb,bb conditions
 *****************  End List  *******************************************
 * The following characters are available for use later. If used in future
 * please delete from this list and add to the list above (in order).
 
 " # $ % ' * . / : ; = I J K N [ \ ] ^ _ ` d e g h m q { | }
 
 ************************************************************************/
 
/*
 * Structure of an opcode table entry.
 */

/* There are two kinds of delay slot nullification: normal which is
 * controled by the nullification bit, and conditional, which depends 
 * on the direction of the branch and its success or failure.
 */
enum delay_type {NONE, NORMAL, CONDITIONAL};
struct pa_opcode
{
    const char *name;
    uint32_t match;	/* Bits that must be set...  */
    uint32_t mask;	/* ... in these bits. */
    char *args;
    /* Nonzero if this is a delayed branch instruction.  */
    char delayed;
};

/*
   All hppa opcodes are 32 bits.

   The match component is a mask saying which bits must match a
   particular opcode in order for an instruction to be an instance
   of that opcode.

   The args component is a string containing one character
   for each operand of the instruction.

   Bit positions in this description follow HP usage of lsb = 31,
   "at" is lsb of field.
   
Kinds of operands:
   x    register field at 15.
   b    register field at 10.
   t    register field at 31.
   5    5 bit immediate at 15.
   s    2 bit space specifier at 17.
   S    3 bit space specifier at 18.
   c    indexed load completer.
   C    short load and store completer.
   Y	Store Bytes Short completer
   <    non-negated compare/subtract conditions.
   -	compare/subtract conditions
   +    non-negated add conditions
   &    logical instruction conditions
   U    unit instruction conditions
   >    shift/extract/deposit conditions.
   ~    bvb,bb conditions
   V    5 bit immediate value at 31
   i    11 bit immediate value at 31
   j    14 bit immediate value at 31
   k    21 bit immediate value at 31
   n	nullification for branch instructions
   w    12 bit branch displacement
   W    17 bit branch displacement (pc-rel for BL and GATE)
   z    17 bit branch displacement (not pc-rel)

Also these (PJH):

   B    either s,b or b where

           s    2 bit space specifier at 17.
           b    register field at 10.

   p    5 bit shift count at 26 (to support the SHD instruction) encoded as
        31-p
   P    5 bit bit position at 26
   T    5 bit field length at 31 (encoded as 32-T)
   A    13 bit immediate at 18 (to support the BREAK instruction)
   Z    System Control Completer (to support LDA, LHA, etc.)
   D    26 bit immediate at 31 (to support the DIAG instruction)

   f    3 bit Special Function Unit identifier at 25
   O    20 bit Special Function Unit operation split between 15 bits at 20
        and 5 bits at 31
   o    15 bit Special Function Unit operation at 20
   2    22 bit Special Function Unit operation split between 17 bits at 20
        and 5 bits at 31
   1    15 bit Special Function Unit operation split between 10 bits at 20
        and 5 bits at 31
   0    10 bit Special Function Unit operation split between 5 bits at 20
        and 5 bits at 31
   u    3 bit coprocessor unit identifier at 25
   F    Source Floating Point Operand Format Completer encoded 2 bits at 20
   G    Destination Floating Point Operand Format Completer encoded 2 bits at 18
   M    Floating-Point Compare Conditions (encoded as 5 bits at 31)
   ?    negated or non-negated compare/subtract conditions
        (used only by 'comb' and 'comib' pseudo-instructions)
   !    negated or non-negated add conditions
        (used only by 'addb' and 'addib' pseudo-instructions)
   r	5 bit immediate value at 31 (for the break instruction)
	(very similar to V above, except the value is unsigned instead of
	low_sign_ext)
   R	5 bit immediate value at 15 (for the ssm, rsm instruction)
	(same as r above, except the value is in a different location)
   Q	5 bit immediate value at 10 (a bit position specified in
	the bb instruction. It's the same as r above, except the
        value is in a different location)

And these (PJH) for PA-89 F.P. registers and instructions:

   v    a 't' operand type extended to handle L/R register halves.
   E    a 'b' operand type extended to handle L/R register halves.
   X    an 'x' operand type extended to handle L/R register halves.
   4    a variation of the 'b' operand type for 'fmpyadd' and 'fmpysub'
   6    a variation of the 'x' operand type for 'fmpyadd' and 'fmpysub'
   7    a variation of the 't' operand type for 'fmpyadd' and 'fmpysub'
   8    5 bit register field at 20 (used in 'fmpyadd' and 'fmpysub')
   9    5 bit register field at 25 (used in 'fmpyadd' and 'fmpysub')
   H    Floating Point Operand Format at 26 for 'fmpyadd' and 'fmpysub'
        (very similar to 'F')

*/

/* bug #41317 .... umeshv@NeXT.com Mon May  2 17:53:29 PDT 1994

   These are for 'cache control hints'
	
	l	Store Instruction Cache Control Hint  (Table 5-8) with
		Short displacement load and store completers (Table 5-11) 
		 
	L	Load and Clear Word Cache Control Hint (Table 5-9) with
		Indexed load completers (Table 5-10)

	3	Store Instruction Cache Control Hint  (Table 5-8) with
		Indexed load completers (Table 5-10)

	a	Load and Clear Word Cache Control Hint (Table 5-9) with
		Short displacement load and store completers (Table 5-11)

	These parse ",cc" and encode "cc" in 2 bits at 20,
	where "cc" encoding is as given in Tables 5-8, 5-9.
    Refer to 'PA-RISC 1.1 Architecture and Instruction Set Reference
	Manual, Second Edition' for the tables.
*/

/* The order of the opcodes in this table is significant:
   
   * The assembler requires that all instances of the same mnemonic must be
   consecutive.  If they aren't, the assembler will bomb at runtime.

   * The disassembler should not care about the order of the opcodes.  */

static struct pa_opcode pa_opcodes[] =
{

/* pseudo-instructions (first so otool matches them) */

{ "b",		0xe8000000, 0xffe0e000, "nW", NORMAL},
{ "ldi",	0x34000000, 0xffe0c000, "j,x"}, /* j by a */
{ "comib", 	0x84000000, 0xfc000000, "?n5,b,w", CONDITIONAL},
{ "comb",	0x80000000, 0xfc000000, "?nx,b,w", CONDITIONAL},
{ "addb",	0xa0000000, 0xfc000000, "!nx,b,w", CONDITIONAL},
{ "addib",	0xa4000000, 0xfc000000, "!n5,b,w", CONDITIONAL},
{ "nop",    0x08000240, 0xffffffff, ""},      /* NOP  <=> OR 0,0,0 */
{ "copy",   0x08000240, 0xffe0ffe0, "x,t"},   /* COPY <=> OR r,0,t */

/* real instructions */
{ "ldw",        0x48000000, 0xfc000000, "j(B),x"},
{ "ldh",        0x44000000, 0xfc000000, "j(B),x"},
{ "ldb",        0x40000000, 0xfc000000, "j(B),x"}, 
{ "stw",        0x68000000, 0xfc000000, "x,j(B)"},
{ "sth",        0x64000000, 0xfc000000, "x,j(B)"},
{ "stb",        0x60000000, 0xfc000000, "x,j(B)"},
{ "ldwm",       0x4c000000, 0xfc000000, "j(B),x"},
{ "stwm",       0x6c000000, 0xfc000000, "x,j(B)"},
{ "ldwx",       0x0c000080, 0xfc001fc0, "cx(B),t"},
{ "ldhx",       0x0c000040, 0xfc001fc0, "cx(B),t"},
{ "ldbx",       0x0c000000, 0xfc001fc0, "cx(B),t"},
{ "ldwax",      0x0c000180, 0xfc00dfc0, "cx(b),t"},
{ "ldcwx",      0x0c0001c0, 0xfc001bc0, "Lx(B),t"}, /* #41317 */
{ "ldws",	0x0c001080, 0xfc001fc0, "C5(B),t"},
{ "ldhs",	0x0c001040, 0xfc001fc0, "C5(B),t"},
{ "ldbs",	0x0c001000, 0xfc001fc0, "C5(B),t"},
{ "ldwas",	0x0c001180, 0xfc00dfc0, "C5(b),t"},
{ "ldcws",	0x0c0011c0, 0xfc001bc0, "a5(B),t"}, /* #41317 was "CL5(B),t" */
{ "stws",	0x0c001280, 0xfc001bc0, "lx,V(B)"}, /* #41317 */
{ "sths",	0x0c001240, 0xfc001bc0, "lx,V(B)"}, /* #41317 */
{ "stbs",	0x0c001200, 0xfc001bc0, "lx,V(B)"}, /* #41317 */
{ "stwas",	0x0c001380, 0xfc00dbc0, "lx,V(b)"}, /* #41317 */
{ "stbys",	0x0c001300, 0xfc001bc0, "Yx,V(B)"}, /* #41317 */
{ "ldo",	0x34000000, 0xfc00c000, "j(b),x"},
{ "ldil",	0x20000000, 0xfc000000, "k,b"},
{ "addil",	0x28000000, 0xfc000000, "k,b"},
{ "bl",		0xe8000000, 0xfc00e000, "nW,b", NORMAL},
{ "gate",	0xe8002000, 0xfc00e000, "nW,b", NORMAL},
{ "blr",	0xe8004000, 0xfc00e001, "nx,b", NORMAL},
{ "bv",		0xe800c000, 0xfc00e001, "nx(b)", NORMAL},
{ "be",		0xe0000000, 0xfc000000, "nz(S,b)", NORMAL},
{ "ble",	0xe4000000, 0xfc000000, "nz(S,b)", NORMAL},
{ "movb",	0xc8000000, 0xfc000000, ">nx,b,w", CONDITIONAL},
{ "movib",	0xcc000000, 0xfc000000, ">n5,b,w", CONDITIONAL},
{ "combt",	0x80000000, 0xfc000000, "<nx,b,w", CONDITIONAL},
{ "combf",	0x88000000, 0xfc000000, "<nx,b,w", CONDITIONAL},
{ "comibt",	0x84000000, 0xfc000000, "<n5,b,w", CONDITIONAL},
{ "comibf",	0x8c000000, 0xfc000000, "<n5,b,w", CONDITIONAL},
{ "addbt",	0xa0000000, 0xfc000000, "+nx,b,w", CONDITIONAL},
{ "addbf",	0xa8000000, 0xfc000000, "+nx,b,w", CONDITIONAL},
{ "addibt",	0xa4000000, 0xfc000000, "+n5,b,w", CONDITIONAL},
{ "addibf",	0xac000000, 0xfc000000, "+n5,b,w", CONDITIONAL},
{ "bvb",	0xc0000000, 0xffe00000, "~nx,w", CONDITIONAL},
{ "bb",		0xc4000000, 0xfc000000, "~nx,Q,w", CONDITIONAL}, /* maybe */

/* Computation Instructions */
  
{ "add",        0x08000600, 0xfc000fe0, "+x,b,t", CONDITIONAL},
{ "addl",       0x08000a00, 0xfc000fe0, "+x,b,t", CONDITIONAL},
{ "addo",       0x08000e00, 0xfc000fe0, "+x,b,t", CONDITIONAL},
{ "addc",       0x08000700, 0xfc000fe0, "+x,b,t", CONDITIONAL},
{ "addco",      0x08000f00, 0xfc000fe0, "+x,b,t", CONDITIONAL},
{ "sh1add",     0x08000640, 0xfc000fe0, "+x,b,t", CONDITIONAL},
{ "sh1addl",    0x08000a40, 0xfc000fe0, "+x,b,t", CONDITIONAL},
{ "sh1addo",    0x08000e40, 0xfc000fe0, "+x,b,t", CONDITIONAL},
{ "sh2add",     0x08000680, 0xfc000fe0, "+x,b,t", CONDITIONAL},
{ "sh2addl",    0x08000a80, 0xfc000fe0, "+x,b,t", CONDITIONAL},
{ "sh2addo",    0x08000e80, 0xfc000fe0, "+x,b,t", CONDITIONAL},
{ "sh3add",     0x080006c0, 0xfc000fe0, "+x,b,t", CONDITIONAL},
{ "sh3addl",    0x08000ac0, 0xfc000fe0, "+x,b,t", CONDITIONAL},
{ "sh3addo",    0x08000ec0, 0xfc000fe0, "+x,b,t", CONDITIONAL},
{ "sub",        0x08000400, 0xfc000fe0, "-x,b,t", CONDITIONAL},
{ "subo",       0x08000c00, 0xfc000fe0, "-x,b,t", CONDITIONAL},
{ "subb",       0x08000500, 0xfc000fe0, "-x,b,t", CONDITIONAL},
{ "subbo",      0x08000d00, 0xfc000fe0, "-x,b,t", CONDITIONAL},
{ "subt",       0x080004c0, 0xfc000fe0, "-x,b,t", CONDITIONAL},
{ "subto",      0x08000cc0, 0xfc000fe0, "-x,b,t", CONDITIONAL},
{ "ds",         0x08000440, 0xfc000fe0, "-x,b,t", CONDITIONAL},
{ "comclr",     0x08000880, 0xfc000fe0, "-x,b,t", CONDITIONAL},
{ "or",         0x08000240, 0xfc000fe0, "&x,b,t", CONDITIONAL},
{ "xor",        0x08000280, 0xfc000fe0, "&x,b,t", CONDITIONAL},
{ "and",        0x08000200, 0xfc000fe0, "&x,b,t", CONDITIONAL},
{ "andcm",      0x08000000, 0xfc000fe0, "&x,b,t", CONDITIONAL},
{ "uxor",       0x08000380, 0xfc000fe0, "Ux,b,t", CONDITIONAL},
{ "uaddcm",     0x08000980, 0xfc000fe0, "Ux,b,t", CONDITIONAL},
{ "uaddcmt",    0x080009c0, 0xfc000fe0, "Ux,b,t", CONDITIONAL},
{ "dcor",       0x08000b80, 0xfc1f0fe0, "Ub,t",   CONDITIONAL},
{ "idcor",      0x08000bc0, 0xfc1f0fe0, "Ub,t",   CONDITIONAL},
{ "addi",       0xb4000000, 0xfc000800, "+i,b,x", CONDITIONAL},
{ "addio",      0xb4000800, 0xfc000800, "+i,b,x", CONDITIONAL},
{ "addit",      0xb0000000, 0xfc000800, "+i,b,x", CONDITIONAL},
{ "addito",     0xb0000800, 0xfc000800, "+i,b,x", CONDITIONAL},
{ "subi",       0x94000000, 0xfc000800, "-i,b,x", CONDITIONAL},
{ "subio",      0x94000800, 0xfc000800, "-i,b,x", CONDITIONAL},
{ "comiclr",    0x90000000, 0xfc000800, "-i,b,x", CONDITIONAL},
{ "vshd",       0xd0000000, 0xfc001fe0, ">x,b,t", CONDITIONAL},
{ "shd",        0xd0000800, 0xfc001c00, ">x,b,p,t", CONDITIONAL},
{ "vextru",     0xd0001000, 0xfc001fe0, ">b,T,x", CONDITIONAL},
{ "vextrs",     0xd0001400, 0xfc001fe0, ">b,T,x", CONDITIONAL},
{ "extru",      0xd0001800, 0xfc001c00, ">b,P,T,x", CONDITIONAL},
{ "extrs",      0xd0001c00, 0xfc001c00, ">b,P,T,x", CONDITIONAL},
{ "vdep",       0xd4000400, 0xfc001fe0, ">x,T,b", CONDITIONAL},
{ "dep",        0xd4000c00, 0xfc001c00, ">x,p,T,b", CONDITIONAL},
{ "vdepi",      0xd4001400, 0xfc001fe0, ">5,T,b", CONDITIONAL},
{ "depi",       0xd4001c00, 0xfc001c00, ">5,p,T,b", CONDITIONAL},
{ "zvdep",      0xd4000000, 0xfc001fe0, ">x,T,b", CONDITIONAL},
{ "zdep",       0xd4000800, 0xfc001c00, ">x,p,T,b", CONDITIONAL},
{ "zvdepi",     0xd4001000, 0xfc001fe0, ">5,T,b", CONDITIONAL},
{ "zdepi",      0xd4001800, 0xfc001c00, ">5,p,T,b", CONDITIONAL},

/* System Control Instructions */

{ "break",      0x00000000, 0xfc001fe0, "r,A"},
{ "rfi",        0x00000c00, 0xffffffff, ""},
{ "rfir",       0x00000ca0, 0xffffffff, ""},
{ "ssm",        0x00000d60, 0xffe0ffe0, "R,t"},
{ "rsm",        0x00000e60, 0xffe0ffe0, "R,t"},
{ "mtsm",       0x00001860, 0xffe0ffff, "x"},
{ "ldsid",      0x000010a0, 0xfc1f3fe0, "(B),t"},
{ "mtsp",       0x00001820, 0xffe01fff, "x,S"},
{ "mtctl",      0x00001840, 0xfc00ffff, "x,b"},
{ "mfsp",       0x000004a0, 0xffff1fe0, "S,t"},
{ "mfctl",      0x000008a0, 0xfc1fffe0, "b,t"},
{ "sync",       0x00000400, 0xffffffff, ""},
{ "prober",     0x04001180, 0xfc003fe0, "(B),x,t"},
{ "proberi",    0x04003180, 0xfc003fe0, "(B),5,t"},
{ "probew",     0x040011c0, 0xfc003fe0, "(B),x,t"},
{ "probewi",    0x040031c0, 0xfc003fe0, "(B),5,t"},
{ "lpa",        0x04001340, 0xfc003fc0, "Zx(B),t"},
{ "lha",        0x04001300, 0xfc003fc0, "Zx(B),t"},
{ "pdtlb",      0x04001200, 0xfc003fdf, "Zx(B)"},
{ "pitlb",      0x04000200, 0xfc001fdf, "Zx(S,b)"},
{ "pdtlbe",     0x04001240, 0xfc003fdf, "Zx(B)"},
{ "pitlbe",     0x04000240, 0xfc003fdf, "Zx(B)"},
{ "idtlba",     0x04001040, 0xfc003fff, "x,(B)"},
{ "iitlba",     0x04000040, 0xfc001fff, "x,(S,b)"},
{ "idtlbp",     0x04001000, 0xfc003fff, "x,(B)"},
{ "iitlbp",     0x04000000, 0xfc001fff, "x,(S,b)"},
{ "pdc",        0x04001380, 0xfc003fdf, "Zx(B)"},
{ "fdc",        0x04001280, 0xfc003fdf, "Zx(B)"},
{ "fic",        0x04000280, 0xfc001fdf, "Zx(S,b)"},
{ "fdce",       0x040012c0, 0xfc003fdf, "Zx(B)"},
{ "fice",       0x040002c0, 0xfc001fdf, "Zx(S,b)"},
{ "diag",       0x14000000, 0xfc000000, "D"},
{ "gfw",        0x04001680, 0xfc003fdf, "Zx(B)"},       /* variant of fdc */
{ "gfr",        0x04001a80, 0xfc003fdf, "Zx(B)"},       /* variant of fdc */

/* Floating Point Coprocessor Instructions */

{ "fldwx",      0x24000000, 0xfc001f80, "cx(B),v"}, /* PJH:  v used to be t */
                                    /* and 0xfc001f80 used to be 0xfc001fc0 */
{ "flddx",      0x2c000000, 0xfc001fc0, "cx(B),t"},
{ "fstwx",      0x24000200, 0xfc001fc0, "cv,x(B)"}, /* PJH:  v used to be t */
{ "fstdx",      0x2c000200, 0xfc001bc0, "3t,x(B)"}, /* #41317 was "clt,x(B)" */
{ "fldws",      0x24001000, 0xfc001fc0, "C5(B),v"}, /* PJH:  v used to be t */
{ "fldds",      0x2c001000, 0xfc001fc0, "C5(B),t"},
{ "fstws",      0x24001200, 0xfc001bc0, "lv,5(B)"}, /* #41317 */
{ "fstds",      0x2c001200, 0xfc001bc0, "lt,5(B)"}, /* #41317 */
{ "fadd",       0x30000600, 0xf400e720, "FE,X,v"}, /* PJH: operands were "Fb,x,t" */
{ "fsub",       0x30002600, 0xf400e720, "FE,X,v"}, /* PJH: operands were "Fb,x,t" */
{ "fmpy",       0x30004600, 0xf400e720, "FE,X,v"}, /* PJH: operands were "Fb,x,t" */
{ "fdiv",       0x30006600, 0xf400e720, "FE,X,v"}, /* PJH: operands were "Fb,x,t" */
{ "fsqrt",      0x30008000, 0xf41fe720, "FE,v"},   /* PJH: operands were "Fb,t" */
{ "fabs",       0x30006000, 0xf41fe720, "FE,v"},   /* PJH: operands were "Fb,t" */
{ "frem",       0x30008600, 0xf400e720, "FE,X,v"}, /* PJH: operands were "Fb,x,t" */
{ "frnd",       0x3000a000, 0xf41fe720, "FE,v"},   /* PJH: operands were "Fb,t" */
{ "fcpy",       0x30004000, 0xf41fe720, "FE,v"},   /* PJH: operands were "Fb,t" */
{ "fcnvff",     0x30000200, 0xf41f8720, "FGE,v"},  /* PJH: operands were "FGb,t" */
{ "fcnvxf",     0x30008200, 0xf41f8720, "FGE,v"},  /* PJH: operands were "FGb,t" */
{ "fcnvfx",     0x30010200, 0xf41f8720, "FGE,v"},  /* PJH: operands were "FGb,t" */
{ "fcnvfxt",    0x30018200, 0xf41f8720, "FGE,v"},  /* PJH: operands were "FGb,t" */
{ "fcmp",       0x30000400, 0xf400e760, "FME,X"},  /* PJH: operands were "FMb,x" */
{ "ftest",      0x30002420, 0xffffffff, ""},

/* PA-89 only instructions */

{ "xmpyu",	0x38004700, 0xfc00e720, "FE,X,v"},
{ "fmpyadd",	0x18000000, 0xfc000000, "H4,6,7,9,8"},
{ "fmpysub",	0x98000000, 0xfc000000, "H4,6,7,9,8"},

/* Assist Instructions */

/* { "spop0",      0x10000000, 0xfc000600, ",f,On", NORMAL}, */
/* { "spop1",      0x10000200, 0xfc000600, ",f,ont", NORMAL}, */
/* { "spop2",      0x10000400, 0xfc000600, ",f,1nb", NORMAL}, */
/* { "spop3",      0x10000600, 0xfc000600, ",f,0nx,b", NORMAL}, */
/* { "copr",       0x30000000, 0xfc000000, ",u,2n", NORMAL}, */
{ "spop0",      0x10000000, 0xfc000600, ",f,Oy", NORMAL},
{ "spop1",      0x10000200, 0xfc000600, ",f,oyt", NORMAL},
{ "spop2",      0x10000400, 0xfc000600, ",f,1yb", NORMAL},
{ "spop3",      0x10000600, 0xfc000600, ",f,0yx,b", NORMAL},
{ "copr",       0x30000000, 0xfc000000, ",u,2y", NORMAL},
{ "cldwx",      0x24000000, 0xfc001e00, ",u,Zx(B),t"},
{ "clddx",      0x2c000000, 0xfc001e00, ",u,Zx(B),t"},
{ "cstwx",      0x24000200, 0xfc001a00, ",u,3t,x(B)"}, /* #41317 */
{ "cstdx",      0x2c000200, 0xfc001200, ",u,3t,x(B)"}, /* #41317 */
{ "cldws",      0x24001000, 0xfc001e00, ",u,Z5(B),t"},
{ "cldds",      0x2c001000, 0xfc001e00, ",u,Z5(B),t"},
{ "cstws",      0x24001200, 0xfc001e00, ",u,Zt,5(B)"},
{ "cstds",      0x2c001200, 0xfc001a00, ",u,3t,5(B)"}, /* #41317 */
{ "mtsar",      0x01601840, 0xffe0ffff, "x"},

/*
 * This pseudo-instruction must be at the end so otool will dissassemble using
 * it over a bl.
 */

/* Gets translated to BL,n  W,b (NeXT procedure call)  and */
/* creates a relocation entry for @ .. USV */

{ "jbsr",	0xe8000000, 0xfc00e000, "n@,b,W", NORMAL},
};

#define NUMOPCODES ((sizeof pa_opcodes)/(sizeof pa_opcodes[0]))

#endif    /* HPPA_OPCODE_INCLUDED */

/* end hppa-opcode.h */
