#undef CHECK_WORD_IMMEDIATES /* bug #26863 */

/* m68k.c  All the m68020 specific stuff in one convenient, huge,
   slow to compile, easy to find file.
   Copyright (C) 1987 Free Software Foundation, Inc.

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "m68k-opcode.h"
#include "as.h"
#include "obstack.h"
#include "struc-symbol.h"
#include "flonum.h"
#include "expr.h"
#include "hash.h"
#include "frags.h"
#include "fixes.h"
#include "read.h"
#include "md.h"
#ifndef NeXT_MOD
#include "m68k.h"
#endif /* NeXT_MOD */
#include "xmalloc.h"
#include "sections.h"
#include "messages.h"
#include "atof-ieee.h"
#include "input-scrub.h"
#include "symbols.h"

/*
 * These are the default cputype and cpusubtype for the m68k architecture.
 */
const cpu_type_t md_cputype = CPU_TYPE_MC680x0;
cpu_subtype_t md_cpusubtype = CPU_SUBTYPE_MC680x0_ALL;

/* This is the byte sex for the m68k architecture */
const enum byte_sex md_target_byte_sex = BIG_ENDIAN_BYTE_SEX;

/*
 * This array holds the chars that always start a comment.  If the
 * pre-processor is disabled, these aren't very useful.
 */
const char md_comment_chars[] = "|";

/*
 * This array holds the chars that only start a comment at the beginning of
 * a line.  If the line seems to have the form '# 123 filename'
 * .line and .file directives will appear in the pre-processed output.
 *
 * Note that input_file.c hand checks for '#' at the beginning of the
 * first line of the input file.  This is because the compiler outputs
 * #NO_APP at the beginning of its output.
 *
 * Also note that a '/' followed by a '*' will always start a comment.
 */
const char md_line_comment_chars[] = "#";

/* Chars that can be used to separate mant from exp in floating point nums. */
const char md_EXP_CHARS[] = "eE";

/*
 * Chars that mean this number is a floating point constant.
 * As in 0f12.456
 * or    0d1.2345e12
 *
 * Also be aware that MAXIMUM_NUMBER_OF_CHARS_FOR_FLOAT may have to be
 * changed in read.c .  Ideally it shouldn't have to know about it at all,
 * but nothing is ideal around here.
 */
const char md_FLT_CHARS[] = "rRsSfFdDxXeEpP";

/* Its an arbitrary name:  This means I don't approve of it */
/* See flames below */
static struct obstack robyn;

/*
 * These macros are used to encode a mode for the rlx_more field of the struct
 * relax_typeS in the array md_relax_table below.  The array is indexed by a
 * TAB(x,y) value where x is one of BRANCH, FBRANCH, ... and y is BYTE, SHORT,
 * ...  Thus the array md_relax_table below is declared to match this use of
 * indexes.  The macro TABTYPE(xy) take the value and returns one of BRANCH,
 * FBRANCH, ...
 */
#define TAB(x,y)	(((x)<<2)+(y))
#define TABTYPE(xy)     ((xy) >> 2)

#define BRANCH		1
#define FBRANCH		2
#define PCREL		3
#define BCC68000        4
#define DBCC            5

#define BYTE		0
#define SHORT		1
#define LONG		2
#define SZ_UNDEF	3

/*
 * BCC68000 is for patching in an extra jmp instruction for long offsets
 * on the 68000.  The 68000 doesn't support long branches with branchs.
 */

/*
 * Note that calls to frag_var need to specify the maximum expansion needed.
 * This is currently 10 bytes for DBCC.
 */

/*
 * This table desribes how you change sizes for the various types of variable
 * size expressions.  This version only supports two kinds.
 * The fields are:
 *	How far Forward this mode will reach:
 *	How far Backward this mode will reach:
 *	How many bytes this mode will add to the size of the frag
 *	Which mode to go to if the offset won't fit in this one
 */
const
relax_typeS
md_relax_table[] = {
{ 1,		1,		0,	0 },	/* First entries aren't used */
{ 1,		1,		0,	0 },	/* For no good reason except */
{ 1,		1,		0,	0 },	/* that the VAX doesn't either */
{ 1,		1,		0,	0 },

{ (127),	(-128),		0,	TAB(BRANCH,SHORT)},
{ (32767),	(-32768),	2,	TAB(BRANCH,LONG) },
{ 0,		0,		4,	0 },
{ 1,		1,		0,	0 },

{ 1,		1,		0,	0 },	/* FBRANCH doesn't come BYTE */
{ (32767),	(-32768),	2,	TAB(FBRANCH,LONG)},
{ 0,		0,		4,	0 },
{ 1,		1,		0,	0 },

{ 1,		1,		0,	0 },	/* PCREL doesn't come BYTE */
{ (32767),	(-32768),	2,	TAB(PCREL,LONG)},
{ 0,		0,		4,	0 },
{ 1,		1,		0,	0 },

{ (127),	(-128),		0,	TAB(BCC68000,SHORT)},
{ (32767),	(-32768),	2,	TAB(BCC68000,LONG) },
{ 0,		0,		6,	0 },	/* jmp long space */
{ 1,		1,		0,	0 },

{ 1,		1,		0,	0 },	/* DBCC doesn't come BYTE */
{ (32767),	(-32768),	2,	TAB(DBCC,LONG) },
{ 0,		0,		10,	0 },	/* bra/jmp long space */
{ 1,		1,		0,	0 },

};

static void s_even(
    uintptr_t value);
static void s_proc(
    uintptr_t value);

/*
 * These are the machine dependent pseudo-ops.  These are included so
 * the assembler can work on the output from the SUN C compiler, which
 * generates these.
 */

/* This table describes all the machine specific pseudo-ops the assembler
 * has to support.  The fields are:
 * 	  pseudo-op name without dot
 *	  function to call to execute this pseudo-op
 *	  Integer arg to pass to the function
 */
const pseudo_typeS md_pseudo_table[] = {
	{ "float",	float_cons,	'f'	},
	{ "int",	cons,		4	},
	{ "word",	cons,		2	},
	{ "quad",	big_cons,	8	},
	{ "octa",	big_cons,	16	},
	{ "even",	s_even,		0	},
	{ "skip",	s_space,	0	},
	{ "proc",	s_proc,		0	},
	{ 0,		0,		0	}
};

#define issbyte(x)	((x)>=-128 && (x)<=127)
#define isubyte(x)	((x)>=0 && (x)<=255)
#define issword(x)	((x)>=-32768 && (x)<=32767)
#define isuword(x)	((x)>=0 && (x)<=65535)

#define isbyte(x)	((x)>=-128 && (x)<=255)
#define isword(x)	((x)>=-32768 && (x)<=65535)
#define islong(x)	(1)

/* Operands we can parse:  (And associated modes)

numb:	8 bit num
numw:	16 bit num
numl:	32 bit num
dreg:	data reg 0-7
reg:	address or data register
areg:	address register
apc:	address register, PC, ZPC or empty string
num:	16 or 32 bit num
num2:	like num
sz:	w or l		if omitted, l assumed
scale:	1 2 4 or 8	if omitted, 1 assumed

7.4 IMMED #num				--> NUM
0.? DREG  dreg				--> dreg
1.? AREG  areg				--> areg
2.? AINDR areg@				--> *(areg)
3.? AINC  areg@+			--> *(areg++)
4.? ADEC  areg@-			--> *(--areg)
5.? AOFF  apc@(numw)			--> *(apc+numw)	-- empty string and ZPC not allowed here
6.? AINDX apc@(num,reg:sz:scale)	--> *(apc+num+reg*scale)
6.? AINDX apc@(reg:sz:scale)		--> same, with num=0
6.? APODX apc@(num)@(num2,reg:sz:scale)	--> *(*(apc+num)+num2+reg*scale)
6.? APODX apc@(num)@(reg:sz:scale)	--> same, with num2=0
6.? AMIND apc@(num)@(num2)		--> *(*(apc+num)+num2) (previous mode without an index reg)
6.? APRDX apc@(num,reg:sz:scale)@(num2)	--> *(*(apc+num+reg*scale)+num2)
6.? APRDX apc@(reg:sz:scale)@(num2)	--> same, with num=0
7.0 ABSL  num:sz			--> *(num)
          num				--> *(num) (sz L assumed)
*** MSCR  otherreg			--> Magic
With -l option
5.? AOFF  apc@(num)			--> *(apc+num) -- empty string and ZPC not allowed here still

examples:
	#foo	#0x35	#12
	d2
	a4
	a3@
	a5@+
	a6@-
	a2@(12)	pc@(14)
	a1@(5,d2:w:1)	@(45,d6:l:4)
	pc@(a2)		@(d4)
	etc . . .


#name@(numw)	-->turn into PC rel mode
apc@(num8,reg:sz:scale)		--> *(apc+num8+reg*scale)

*/

#define IMMED	1
#define DREG	2
#define AREG	3
#define AINDR	4
#define ADEC	5
#define AINC	6
#define AOFF	7
#define AINDX	8
#define APODX	9
#define AMIND	10
#define APRDX	11
#define ABSL	12
#define MSCR	13
#define REGLST	14

#define FAIL	0
#define OK	1

/* DATA and ADDR have to be contiguous, so that reg-DATA gives 0-7==data reg,
   8-15==addr reg for operands that take both types */
#define DATA	1		/*   1- 8 == data registers 0-7 */
#define ADDR	(DATA+8)	/*   9-16 == address regs 0-7 */
#define FPREG	(ADDR+8)	/*  17-24 Eight FP registers */
#define COPNUM	(FPREG+8)	/*  25-32 Co-processor #1-#8 */
#undef PC
#define PC	(COPNUM+8)	/*  33 Program counter */
#define ZPC	(PC+1)		/*  34 Hack for Program space, but 0 addressing */
#define SR	(ZPC+1)		/*  35 Status Reg */
#define CCR	(SR+1)		/*  36 Condition code Reg */

#define FPI	(CCR+1)		/*  37 floating-point instruction register */
#define FPS	(FPI+1)		/*  38 floating-point status register */
#define FPC	(FPS+1)		/*  39 floating-point condition register */

/* These have to be in order for the movec instruction to work. */
/* The comment above should read: All the registers that can be in a movec
   instruction must be bounded by USP and MSP (or SRP if BUILTIN_MMUS is
   defined) for the 'J' kind of operand to be checked correctly in m68_ip() */
#define USP	(FPC+1)		/*  40 User Stack Pointer */
#define ISP	(USP+1)		/*  41 Interrupt stack pointer */
#define SFC	(ISP+1)		/*  42 Source function control code register */
#define DFC	(SFC+1)		/*  43 Destination function code register */
#define CACR	(DFC+1)		/*  44 Cashe control register */
#define VBR	(CACR+1)	/*  45 wector base register */
#define CAAR	(VBR+1)		/*  46 Cashe address register */
#define MSP	(CAAR+1)	/*  47 Master stack pointer */

#ifdef BUILTIN_MMUS
/* mc68040 mmu registers, can be used in a movec instruction  */
#define	ITT0	(MSP+1)	 /* 48 instruction transparent translation register 0 */
#define	ITT1	(ITT0+1) /* 49 instruction transparent translation register 1 */
#define	DTT0	(ITT1+1) /* 50 data transparent translation register 0 */
#define	DTT1	(DTT0+1) /* 51 data transparent translation register 1 */
#define	URP	(DTT1+1) /* 53 user root pointer */

/* mc68030 and mc68040 mmu registers, can be used in a movec instruction */
#define	MMUSR	(URP+1)  /* 52 MMU status register */
#define TC	(MMUSR+1)/* 54 MMU translation control register */
#define SRP	(TC+1)   /* 55 supervisor root pointer */

/* mc68030 mmu registers, can't be used in a movec instruction (but rather in a
   pmove instruction) */
#define CRP	(SRP+1)  /* 56 cpu root pointer */
#define	TT0	(CRP+1)  /* 57 transparent translation register 0 */
#define	TT1	(TT0+1)  /* 58 transparent translation register 0 */

/* mc68040 operands to cache instructions  */
#define	IC	(TT1+1)  /* 59 instruction cache */
#define	DC	(IC+1)   /* 60 data cache */
#define	BC	(DC+1)   /* 61 both instruction and data caches */
#endif /* BUILTIN_MMUS */

#ifdef m68851
/*
 * these defines should be in m68k.c but
 * i put them here to keep all the m68851 stuff
 * together -rab
 * JF--Make sure these #s don't clash with the ones in m68k.c
 * That would be BAD.
 */
#define TC	(MSP+1)		/* 48 */
#define DRP	(TC+1)		/* 49 */
#define SRP	(DRP+1)		/* 50 */
#define CRP	(SRP+1)		/* 51 */
#define CAL	(CRP+1)		/* 52 */
#define VAL	(CAL+1)		/* 53 */
#define SCC	(VAL+1)		/* 54 */
#define AC	(SCC+1)		/* 55 */
#define BAD	(AC+1)		/* 56,57,58,59, 60,61,62,63 */
#define BAC	(BAD+8)		/* 64,65,66,67, 68,69,70,71 */
#define PSR	(BAC+8)		/* 72 */
#define PCSR	(PSR+1)		/* 73 */
#endif /* m68851 */


/* Note that COPNUM==processor #1 -- COPNUM+7==#8, which stores as 000 */
/* I think. . .  */

#undef SP
#define	SP	ADDR+7

/* JF these tables here are for speed at the expense of size */
/* You can replace them with the #if 0 versions if you really
   need space and don't mind it running a bit slower */

static char mklower_table[256];
#define mklower(c) (mklower_table[(unsigned char)(c)])
static char notend_table[256];
static char alt_notend_table[256];
#define notend(s) ( !(notend_table[(unsigned char)(*s)] || (*s==':' &&\
 alt_notend_table[(unsigned char)(s[1])])))

#if 0
#define mklower(c)	(isupper(c) ? tolower(c) : c)
#endif


struct m68k_exp {
	char	*e_beg;
	char	*e_end;
	expressionS e_exp;
	short	e_siz;		/* 0== default 1==short/byte 2==word 3==long */
};

/* Internal form of an operand.  */
struct m68k_op {
	char	*error;		/* Couldn't parse it */
	int	mode;		/* What mode this instruction is in.  */
	uint32_t	reg;		/* Base register */
	struct m68k_exp *con1;
	int	ireg;		/* Index register */
	int	isiz;		/* 0==unspec  1==byte(?)  2==short  3==long  */
	int	imul;		/* Multipy ireg by this (1,2,4,or 8) */
	struct	m68k_exp *con2;
};

/* internal form of a 68020 instruction */
struct m68_it {
	char	*error;
	char	*args;		/* list of opcode info */
	int	numargs;

#ifdef NeXT_MOD
	char	*cpus;
#endif /* NeXT_MOD */
	int	numo;		/* Number of shorts in opcode */
	short	opcode[11];

	struct m68k_op operands[6];

	int	nexp;		/* number of exprs in use */
	struct m68k_exp exprs[4];

	int	nfrag;		/* Number of frags we have to produce */
	struct {
		int fragoff;	/* Where in the current opcode[] the frag ends */
		symbolS *fadd;
		int32_t foff;
		int fragty;
	} fragb[4];

	int	nrel;		/* Num of reloc strucs in use */
	struct	{
		int	n;
		symbolS	*add,
			*sub;
		int32_t off;
		char	wid;
		char	pcrel;
	} reloc[5];		/* Five is enough??? */
};

static struct m68_it the_ins;		/* the instruction being assembled */


/* Macros for adding things to the m68_it struct */

#define addword(w)	the_ins.opcode[the_ins.numo++]=(w)

/* Like addword, but goes BEFORE general operands */
#define insop(w)	{int z;\
 for(z=the_ins.numo;z>opcode->m_codenum;--z)\
   the_ins.opcode[z]=the_ins.opcode[z-1];\
 for(z=0;z<the_ins.nrel;z++)\
   the_ins.reloc[z].n+=2;\
 the_ins.opcode[opcode->m_codenum]=(w);\
 the_ins.numo++;\
}


#define add_exp(beg,end) (\
	the_ins.exprs[the_ins.nexp].e_beg=(beg),\
	the_ins.exprs[the_ins.nexp].e_end=(end),\
	&the_ins.exprs[the_ins.nexp++]\
)


/* The numo+1 kludge is so we can hit the low order byte of the prev word. Blecch*/
#define add_fix(width,exp,pc_rel) {\
	the_ins.reloc[the_ins.nrel].n= ((width)=='B') ? (the_ins.numo*2-1) : \
		(((width)=='b') ? ((the_ins.numo-1)*2) : (the_ins.numo*2));\
	the_ins.reloc[the_ins.nrel].add=adds((exp));\
	the_ins.reloc[the_ins.nrel].sub=subs((exp));\
	the_ins.reloc[the_ins.nrel].off=offs((exp));\
	the_ins.reloc[the_ins.nrel].wid=(width);\
	the_ins.reloc[the_ins.nrel++].pcrel=(pc_rel);\
}

#define add_frag(add,off,type)  {\
	the_ins.fragb[the_ins.nfrag].fragoff=the_ins.numo;\
	the_ins.fragb[the_ins.nfrag].fadd=(add);\
	the_ins.fragb[the_ins.nfrag].foff=(off);\
	the_ins.fragb[the_ins.nfrag++].fragty=(type);\
}

#define isvar(exp)	((exp) && (adds(exp) || subs(exp)))

#define seg(exp)	((exp)->e_exp.X_seg)
#define adds(exp)	((exp)->e_exp.X_add_symbol)
#define subs(exp)	((exp)->e_exp.X_subtract_symbol)
#define offs(exp)	((exp)->e_exp.X_add_number)


struct m68_incant {
	char *m_operands;
	uint32_t m_opcode;
	short m_opnum;
	short m_codenum;
#ifdef NeXT_MOD
	char *m_cpus;
#endif /* NeXT_MOD */
	struct m68_incant *m_next;
};

#define getone(x)	((((x)->m_opcode)>>16)&0xffff)
#define gettwo(x)	(((x)->m_opcode)&0xffff)

/*
 * Declarations static functions in this file.
 */
static int m68k_reg_parse(
    char **ccp);
static int m68k_ip_op(
    char *str,
    struct m68k_op *opP);
static int try_index(
    char **s,
    struct m68k_op *opP);
static void m68_ip(
    char *instring);
static int get_regs(
    int i,
    struct m68k_op *opP,
    char *str);
static int reverse_16_bits(
    int in);
static int reverse_8_bits(
    int in);
static void install_operand(
    int mode,
    int val);
static void install_gen_operand(
    int mode,
    int val);
static char *crack_operand(
    char *str,
    struct m68k_op *opP);
static int get_num(
    struct m68k_exp *exp,
    int ok);

/* JF modified this to handle cases where the first part of a symbol name
   looks like a register */

static
int
m68k_reg_parse(
char **ccp)
{
	register char c1,
		c2,
		c3,
		c4;
#ifdef BUILTIN_MMUS
	char    c5;
#endif
	register int n = 0,
		ret = 0;

	c1=mklower(ccp[0][0]);
	c2=mklower(ccp[0][1]);
	c3=mklower(ccp[0][2]);
	c4=mklower(ccp[0][3]);
#ifdef BUILTIN_MMUS
	c5=mklower(ccp[0][4]);
#endif

	switch(c1) {
	case 'a':
		if(c2>='0' && c2<='7') {
			n=2;
			ret=ADDR+c2-'0';
		}
#ifdef m68851
		else if (c2 == 'c') {
			n = 2;
			ret = AC;
		}
#endif
		break;
#ifdef m68851
	case 'b':
		if (c2 == 'a') {
			if (c3 == 'd') {
				if (c4 >= '0' && c4 <= '7') {
					n = 4;
					ret = BAD + c4 - '0';
				}
			}
			if (c3 == 'c') {
				if (c4 >= '0' && c4 <= '7') {
					n = 4;
					ret = BAC + c4 - '0';
				}
			}
		}
		break;
#endif
#ifdef BUILTIN_MMUS
	case 'b':
		if (c2 == 'c') {
			n = 2;
			ret = (BC);
		}
		break;
#endif
	case 'c':
#ifdef m68851
		if (c2 == 'a' && c3 == 'l') {
			n = 3;
			ret = CAL;
		} else
#endif
			/* This supports both CCR and CC as the ccr reg. */
		if(c2=='c' && c3=='r') {
			n=3;
			ret = CCR;
		} else if(c2=='c') {
			n=2;
			ret = CCR;
		} else if(c2=='a' && (c3=='a' || c3=='c') && c4=='r') {
			n=4;
			ret = c3=='a' ? CAAR : CACR;
		}
#if defined(m68851) || defined (BUILTIN_MMUS)
		else if (c2 == 'r' && c3 == 'p') {
			n = 3;
			ret = (CRP);
		}
#endif
		break;
	case 'd':
		if(c2>='0' && c2<='7') {
			n=2;
			ret = DATA+c2-'0';
		} else if(c2=='f' && c3=='c') {
			n=3;
			ret = DFC;
		}
#ifdef m68851
		else if (c2 == 'r' && c3 == 'p') {
			n = 3;
			ret = (DRP);
		}
#endif
#ifdef BUILTIN_MMUS
		else if (c2 == 't' && c3 == 't' && (c4 == '0' || c4 == '1')) {
			n = 4;
			if(c4 == '0')
			    ret = (DTT0);
			else
			    ret = (DTT1);
		}
		else if (c2 == 'c') {
			n = 2;
			ret = (DC);
		}
#endif
		break;
	case 'f':
		if(c2=='p') {
 			if(c3>='0' && c3<='7') {
				n=3;
				ret = FPREG+c3-'0';
				if(c4==':')
					ccp[0][3]=',';
			} else if(c3=='i') {
				n=3;
				ret = FPI;
			} else if(c3=='s') {
				n= (c4 == 'r' ? 4 : 3);
				ret = FPS;
			} else if(c3=='c') {
				n= (c4 == 'r' ? 4 : 3);
				ret = FPC;
			}
		}
		break;
	case 'i':
		if(c2=='s' && c3=='p') {
			n=3;
			ret = ISP;
		}
#ifdef BUILTIN_MMUS
		else if (c2 == 't' && c3 == 't' && (c4 == '0' || c4 == '1')) {
			n = 4;
			if(c4 == '0')
			    ret = (ITT0);
			else
			    ret = (ITT1);
		}
		else if (c2 == 'c') {
			n = 2;
			ret = (IC);
		}
#endif
		break;
	case 'm':
		if(c2=='s' && c3=='p') {
			n=3;
			ret = MSP;
		}
#ifdef BUILTIN_MMUS
		if(c2=='m' && c3=='u' && c4=='s' && c5=='r') {
			n=5;
			ret = MMUSR;
		}
#endif
		break;
	case 'p':
		if(c2=='c') {
#ifdef m68851
			if(c3 == 's' && c4=='r') {
				n=4;
				ret = (PCSR);
			} else
#endif
			{
				n=2;
				ret = PC;
			}
		}
#ifdef m68851
		else if (c2 == 's' && c3 == 'r') {
			n = 3;
			ret = (PSR);
		}
#endif
#ifdef BUILTIN_MMUS
		else if (c2 == 's' && c3 == 'r') {
			n = 3;
			ret = (MMUSR);
		}
#endif
		break;
	case 's':
#if defined(m68851) || defined(BUILTIN_MMUS)
		if (c2 == 'r' && c3 == 'p') {
			n = 3;
			ret = (SRP);
		}
#endif
#ifdef m68851
		else if (c2 == 'c' && c3 == 'c') {
			n = 3;
			ret = (SCC);
		}
#endif
#if defined(m68851) || defined(BUILTIN_MMUS)
		else
#endif
		if(c2=='r') {
			n=2;
			ret = SR;
		} else if(c2=='p') {
			n=2;
			ret = ADDR+7;
		} else if(c2=='f' && c3=='c') {
			n=3;
			ret = SFC;
		}
		break;
#if defined(m68851) || defined(BUILTIN_MMUS)
	case 't':
		if(c2 == 'c') {
			n=2;
			ret=TC;
		}
#ifdef BUILTIN_MMUS
		else if (c2 == 't' && (c3 == '0' || c3 == '1')) {
			n = 3;
			if(c3 == '0')
			    ret = (TT0);
			else
			    ret = (TT1);
		}
#endif
		break;
#endif
	case 'u':
		if(c2=='s' && c3=='p') {
			n=3;
			ret = USP;
		}
#ifdef BUILTIN_MMUS
		else if(c2=='r' && c3=='p') {
			n=3;
			ret = URP;
		}
#endif
		break;
	case 'v':
#ifdef m68851
		if (c2 == 'a' && c3 == 'l') {
			n = 3;
			ret = (VAL);
		} else
#endif
		if(c2=='b' && c3=='r') {
			n=3;
			ret = VBR;
		}
		break;
	case 'z':
		if(c2=='p' && c3=='c') {
			n=3;
			ret = ZPC;
		}
		break;
	default:
		break;
	}
	if(n) {
		if(isalnum(ccp[0][n]) || ccp[0][n]=='_')
			ret=FAIL;
		else
			ccp[0]+=n;
	} else
		ret = FAIL;
	return ret;
}

#define SKIP_WHITE()	{ str++; if(*str==' ') str++;}

static
int
m68k_ip_op(
char *str,
struct m68k_op *opP)
{
	char	*strend;
	int32_t	i;

	if(*str==' ')
		str++;
		/* Find the end of the string */
	if(!*str) {
		/* Out of gas */
		opP->error="Missing operand";
		return FAIL;
	}
	for(strend=str;*strend;strend++)
		;
	--strend;

		/* Guess what:  A constant.  Shar and enjoy */
	if(*str=='#') {
		str++;
		opP->con1=add_exp(str,strend);
		opP->mode=IMMED;
		return OK;
	}
	i=m68k_reg_parse(&str);
	if((i==FAIL || *str!='\0') && *str!='@') {
		char *stmp;

		if(i!=FAIL && (*str=='/' || *str=='-')) {
			opP->mode=REGLST;
			return get_regs(i,opP,str);
		}
		if((stmp=index(str,'@'))) {
			opP->con1=add_exp(str,stmp-1);
			if(stmp==strend) {
				opP->mode=AINDX;
				return OK;
			}
			stmp++;
			if(*stmp++!='(' || *strend--!=')') {
				opP->error="Malformed operand";
				return FAIL;
			}
			i=try_index(&stmp,opP);
			opP->con2=add_exp(stmp,strend);
			if(i==FAIL) opP->mode=AMIND;
			else opP->mode=APODX;
			return OK;
		}
		opP->mode=ABSL;
		opP->con1=add_exp(str,strend);
		return OK;
	}
	opP->reg=i;
	if(*str=='\0') {
		if(i>=DATA+0 && i<=DATA+7)
			opP->mode=DREG;
		else if(i>=ADDR+0 && i<=ADDR+7)
			opP->mode=AREG;
		else
			opP->mode=MSCR;
		return OK;
	}
	if((i<ADDR+0 || i>ADDR+7) && i!=PC && i!=ZPC && i!=FAIL) {	/* Can't indirect off non address regs */
		opP->error="Invalid indirect register";
		return FAIL;
	}
	if(*str!='@')
		abort();
	str++;
	switch(*str) {
	case '\0':
		opP->mode=AINDR;
		return OK;
	case '-':
		opP->mode=ADEC;
		return OK;
	case '+':
		opP->mode=AINC;
		return OK;
	case '(':
		str++;
		break;
	default:
		opP->error="Junk after indirect";
		return FAIL;
	}
		/* Some kind of indexing involved.  Lets find out how bad it is */
	i=try_index(&str,opP);
		/* Didn't start with an index reg, maybe its offset or offset,reg */
	if(i==FAIL) {
		char *beg_str;

		beg_str=str;
		for(i=1;i;) {
			switch(*str++) {
			case '\0':
				opP->error="Missing )";
				return FAIL;
			case ',': i=0; break;
			case '(': i++; break;
			case ')': --i; break;
			}
		}
		opP->con1=add_exp(beg_str,str-2);
			/* Should be offset,reg */
		if(str[-1]==',') {
			i=try_index(&str,opP);
			if(i==FAIL) {
				opP->error="Malformed index reg";
				return FAIL;
			}
		}
	}
		/* We've now got offset)   offset,reg)   or    reg) */

	if(*str=='\0') {
		/* Th-the-thats all folks */
#ifdef NeXT_MOD
	/* all forms using zpc must use pc@(bd,Xn) and not pc@(d16) because
	   you can only suppress the base register in the first form */
		if(opP->reg==FAIL || opP->reg==ZPC) opP->mode=AINDX;	/* Other form of indirect */
#else /* !defined(NeXT_MOD) */
		if(opP->reg==FAIL) opP->mode=AINDX;	/* Other form of indirect */
#endif /* NeXT_MOD */
		else if(opP->ireg==FAIL) opP->mode=AOFF;
		else opP->mode=AINDX;
		return OK;
	}
		/* Next thing had better be another @ */
	if(*str!='@' || str[1]!='(') {
		opP->error="junk after indirect";
		return FAIL;
	}
	str+=2;
	if(opP->ireg!=FAIL) {
		opP->mode=APRDX;
		i=try_index(&str,opP);
		if(i!=FAIL) {
			opP->error="Two index registers!  not allowed!";
			return FAIL;
		}
	} else
		i=try_index(&str,opP);
	if(i==FAIL) {
		char *beg_str;

		beg_str=str;
		for(i=1;i;) {
			switch(*str++) {
			case '\0':
				opP->error="Missing )";
				return FAIL;
			case ',': i=0; break;
			case '(': i++; break;
			case ')': --i; break;
			}
		}
		opP->con2=add_exp(beg_str,str-2);
		if(str[-1]==',') {
			if(opP->ireg!=FAIL) {
				opP->error="Can't have two index regs";
				return FAIL;
			}
			i=try_index(&str,opP);
			if(i==FAIL) {
				opP->error="malformed index reg";
				return FAIL;
			}
			opP->mode=APODX;
		} else if(opP->ireg!=FAIL)
			opP->mode=APRDX;
		else
			opP->mode=AMIND;
	} else
		opP->mode=APODX;
	if(*str!='\0') {
		opP->error="Junk after indirect";
		return FAIL;
	}
	return OK;
}

static
int
try_index(
char **s,
struct m68k_op *opP)
{
	register int	i;
	char	*ss;
#define SKIP_W()	{ ss++; if(*ss==' ') ss++;}

	ss= *s;
	/* SKIP_W(); */
	i=m68k_reg_parse(&ss);
	if(!(i>=DATA+0 && i<=ADDR+7)) {	/* if i is not DATA or ADDR reg */
		*s=ss;
		return FAIL;
	}
	opP->ireg=i;
	/* SKIP_W(); */
	if(*ss==')') {
		opP->isiz=0;
		opP->imul=1;
		SKIP_W();
		*s=ss;
		return OK;
	}
	if(*ss!=':') {
		opP->error="Missing : in index register";
		*s=ss;
		return FAIL;
	}
	SKIP_W();
	if(mklower(*ss)=='w') opP->isiz=2;
	else if(mklower(*ss)=='l') opP->isiz=3;
	else {
		opP->error="Size spec not :w or :l";
		*s=ss;
		return FAIL;
	}
	SKIP_W();
	if(*ss==':') {
		SKIP_W();
		switch(*ss) {
		case '1':
		case '2':
		case '4':
		case '8':
			opP->imul= *ss-'0';
			break;
		default:
			opP->error="index multiplier not 1, 2, 4 or 8";
			*s=ss;
			return FAIL;
		}
		SKIP_W();
	} else opP->imul=1;
	if(*ss!=')') {
		opP->error="Missing )";
		*s=ss;
		return FAIL;
	}
	SKIP_W();
	*s=ss;
	return OK;
}

#ifdef TEST1	/* TEST1 tests m68k_ip_op(), which parses operands */
void
main(
int argc,
char *argv[],
char *envp[])
{
    char buf[128];
    struct m68k_op thark;

	for(;;){
		if(!gets(buf))
		    break;
		memset(&thark, '\0', sizeof(thark));
		if(!m68k_ip_op(buf, &thark))
		    printf("FAIL:");
		if(thark.error)
		    printf("op1 error %s in %s\n",thark.error,buf);
		printf("mode %d, reg %d, ",thark.mode,thark.reg);
		if(thark.con1)
		    printf("con1: '%.*s',",
			   1 + thark.con1->e_end - thark.con1->e_beg,
			   thark.con1->e_beg);
		printf("ireg %d, isiz %d, imul %d ",
		       thark.ireg, thark.isiz, thark.imul);
		if(thark.con2)
		    printf("con2: '%.*s'",
			   1 + thark.con2->e_end - thark.con2->e_beg,
			   thark.con2->e_beg);
		printf("\n");
	}
	exit(0);
}
#endif /* TEST1 */


/*
 * Handle of the OPCODE hash table NULL means any use before m68_ip_begin()
 * will crash.
 */
static struct hash_control *op_hash = NULL;


/*
 *		m 6 8 _ i p ( )
 *
 * This converts a string into a 68k instruction.
 * The string must be a bare single instruction in sun format
 * with RMS-style 68020 indirects
 *  (example:  )
 *
 * It provides some error messages: at most one fatal error message (which
 * stops the scan) and at most one warning message for each operand.
 * The 68k instruction is returned in exploded form, since we have no
 * knowledge of how you parse (or evaluate) your expressions.
 * We do however strip off and decode addressing modes and operation
 * mnemonic.
 *
 * This function's value is a string. If it is not "" then an internal
 * logic error was found: read this code to assign meaning to the string.
 * No argument string should generate such an error string:
 * it means a bug in our code, not in the user's text.
 *
 * You MUST have called m86_ip_begin() once and m86_ip_end() never before using
 * this function.
 */

/* JF this function no longer returns a useful value.  Sorry */
static
void
m68_ip(
char *instring)
{
	register char *p;
	register struct m68k_op *opP;
	register struct m68_incant *opcode;
	register char *s;
	register int tmpreg = 0,
		baseo = 0,
		outro = 0,
		nextword;
	int	siz1,
		siz2;
	char	c;
	int	losing;
	int	opsfound;
	LITTLENUM_TYPE words[6];
	LITTLENUM_TYPE *wordp;

	if (*instring == ' ')
		instring++;			/* skip leading whitespace */

  /* Scan up to end of operation-code, which MUST end in end-of-string
     or exactly 1 space. */
	for (p = instring; *p != '\0'; p++)
		if (*p == ' ')
			break;


	if (p == instring) {
		the_ins.error = "No operator";
		the_ins.opcode[0] = 0;
		/* the_ins.numo=1; */
		return;
	}

  /* p now points to the end of the opcode name, probably whitespace.
     make sure the name is null terminated by clobbering the whitespace,
     look it up in the hash table, then fix it back. */   
	c = *p;
	*p = '\0';
	opcode = (struct m68_incant *)hash_find (op_hash, instring);
	*p = c;

	if (opcode == NULL) {
		the_ins.error = "Unknown operator";
		the_ins.opcode[0] = 0;
		/* the_ins.numo=1; */
		return;
	}

  /* found a legitimate opcode, start matching operands */
	for(opP= &the_ins.operands[0];*p;opP++) {
		p = crack_operand (p, opP);
		if(opP->error) {
			the_ins.error=opP->error;
			return;
		}
	}

	opsfound=opP- &the_ins.operands[0];
	/* This ugly hack is to support the floating pt opcodes in their standard form */
	/* Essentially, we fake a first enty of type COP#1 */
	if(opcode->m_operands[0]=='I') {
		int	n;

		for(n=opsfound;n>0;--n)
			the_ins.operands[n]=the_ins.operands[n-1];

		/* bcopy((char *)(&the_ins.operands[0]),(char *)(&the_ins.operands[1]),opsfound*sizeof(the_ins.operands[0])); */
		memset((char *)(&the_ins.operands[0]), '\0',
		       sizeof(the_ins.operands[0]));
		the_ins.operands[0].mode=MSCR;
		the_ins.operands[0].reg=COPNUM;		/* COP #1 */
		opsfound++;
	}
		/* We've got the operands.  Find an opcode that'll
		   accept them */
	for(losing=0;;) {
		if(opsfound!=opcode->m_opnum)
			losing++;
		else for(s=opcode->m_operands,opP= &the_ins.operands[0];*s && !losing;s+=2,opP++) {
				/* Warning: this switch is huge! */
				/* I've tried to organize the cases into  this order:
				   non-alpha first, then alpha by letter.  lower-case goes directly
				   before uppercase counterpart. */
				/* Code with multiple case ...: gets sorted by the lowest case ...
				   it belongs to.  I hope this makes sense. */
			switch(*s) {
			case '!':
				if(opP->mode==MSCR || opP->mode==IMMED ||
 opP->mode==DREG || opP->mode==AREG || opP->mode==AINC || opP->mode==ADEC || opP->mode==REGLST)
					losing++;
				break;

			case '#':
				if(opP->mode!=IMMED)
 					losing++;
				else {
					int32_t t;

					t=get_num(opP->con1,80);
					if(s[1]=='b' && !isbyte(t))
						losing++;
#ifdef CHECK_WORD_IMMEDIATES
					else if((s[1]=='w' || s[1]=='z') &&
						!isword(t))
						losing++;
#else
					else if(s[1]=='z' && !isword(t))
						losing++;
#endif
				}
				break;

			case '^':
			case 'T':
				if(opP->mode!=IMMED)
					losing++;
				break;

			case '$':
				if(opP->mode==MSCR || opP->mode==AREG ||
 opP->mode==IMMED || opP->reg==PC || opP->reg==ZPC || opP->mode==REGLST)
					losing++;
				break;

			case '%':
				if(opP->mode==MSCR || opP->reg==PC ||
 opP->reg==ZPC || opP->mode==REGLST)
					losing++;
				break;


			case '&':
				if(opP->mode==MSCR || opP->mode==DREG ||
 opP->mode==AREG || opP->mode==IMMED || opP->reg==PC || opP->reg==ZPC ||
 opP->mode==AINC || opP->mode==ADEC || opP->mode==REGLST)
					losing++;
				break;

			case '*':
				if(opP->mode==MSCR || opP->mode==REGLST)
					losing++;
				break;

			case '+':
				if(opP->mode!=AINC)
					losing++;
				break;

			case '-':
				if(opP->mode!=ADEC)
					losing++;
				break;

#ifdef NeXT_MOD
			case '0':
				if(opP->mode!=AINDR)
					losing++;
				break;
#endif /* NeXT_MOD */

			case '/':
				if(opP->mode==MSCR || opP->mode==AREG ||
 opP->mode==AINC || opP->mode==ADEC || opP->mode==IMMED || opP->mode==REGLST)
					losing++;
				break;

			case ';':
				if(opP->mode==MSCR || opP->mode==AREG || opP->mode==REGLST)
					losing++;
				break;

			case '?':
				if(opP->mode==MSCR || opP->mode==AREG ||
 opP->mode==AINC || opP->mode==ADEC || opP->mode==IMMED || opP->reg==PC ||
 opP->reg==ZPC || opP->mode==REGLST)
					losing++;
				break;

			case '@':
				if(opP->mode==MSCR || opP->mode==AREG ||
 opP->mode==IMMED || opP->mode==REGLST)
					losing++;
				break;

			case '~':		/* For now! (JF FOO is this right?) */
				if(opP->mode==MSCR || opP->mode==DREG ||
 opP->mode==AREG || opP->mode==IMMED || opP->reg==PC || opP->reg==ZPC || opP->mode==REGLST)
					losing++;
				break;

			case 'A':
				if(opP->mode!=AREG)
					losing++;
				break;

			case 'B':	/* FOO */
				if(opP->mode!=ABSL)
					losing++;
				break;

			case 'C':
				if(opP->mode!=MSCR || opP->reg!=CCR)
					losing++;
				break;

			case 'd':	/* FOO This mode is a KLUDGE!! */
				if(opP->mode!=AOFF && (opP->mode!=ABSL ||
 opP->con1->e_beg[0]!='(' || opP->con1->e_end[0]!=')'))
					losing++;
				break;

			case 'D':
				if(opP->mode!=DREG)
					losing++;
				break;

			case 'F':
				if(opP->mode!=MSCR || opP->reg<(FPREG+0) || opP->reg>(FPREG+7))
					losing++;
				break;

			case 'I':
				if(opP->mode!=MSCR || opP->reg<COPNUM ||
 opP->reg>=COPNUM+7)
					losing++;
				break;

			case 'J':
#ifdef BUILTIN_MMUS
				if(opP->mode!=MSCR || opP->reg<USP || opP->reg>SRP)
#else
				if(opP->mode!=MSCR || opP->reg<USP || opP->reg>MSP)
#endif
					losing++;
				break;

			case 'k':
				if(opP->mode!=IMMED)
					losing++;
				break;

			case 'l':
			case 'L':
				if(opP->mode==DREG || opP->mode==AREG || opP->mode==FPREG) {
					if(s[1]=='8')
						losing++;
					else {
						opP->mode=REGLST;
						opP->reg=1<<(opP->reg-DATA);
					}
				} else if(opP->mode!=REGLST) {
					losing++;
				} else if(s[1]=='8' && opP->reg&0x0FFffFF)
					losing++;
				else if(s[1]=='3' && opP->reg&0x7000000)
					losing++;
				break;

			case 'M':
				if(opP->mode!=IMMED)
					losing++;
				else {
					int32_t t;

					t=get_num(opP->con1,80);
#ifdef NeXT_MOD	/* feature to try to make expressions absolute */
					/* DJA -- Bug fix. allow absolute expressions */
					if(! (issbyte(t) && seg(opP->con1)==SEG_ABSOLUTE) )
#else /* !defined(NeXT_MOD) */
					if(!issbyte(t) || isvar(opP->con1))
#endif /* NeXT_MOD */
						losing++;
				}
				break;

			case 'O':
				if(opP->mode!=DREG && opP->mode!=IMMED)
					losing++;
				break;

			case 'Q':
				if(opP->mode!=IMMED)
					losing++;
				else {
					int32_t t;

					t=get_num(opP->con1,80);
					if(t<1 || t>8 || isvar(opP->con1))
						losing++;
				}
				break;

			case 'R':
				if(opP->mode!=DREG && opP->mode!=AREG)
					losing++;
				break;

			case 's':
				if(opP->mode!=MSCR || !(opP->reg==FPI || opP->reg==FPS || opP->reg==FPC))
					losing++;
				break;

			case 'S':
				if(opP->mode!=MSCR || opP->reg!=SR)
					losing++;
				break;

			case 'U':
				if(opP->mode!=MSCR || opP->reg!=USP)
					losing++;
				break;

			/* JF these are out of order.  We could put them
			   in order if we were willing to put up with
			   bunches of #ifdef m68851s in the code */
#ifdef m68851
			/* Memory addressing mode used by pflushr */
			case '|':
				if(opP->mode==MSCR || opP->mode==DREG ||
 opP->mode==AREG || opP->mode==REGLST)
					losing++;
				break;
#endif

#if defined(m68851) || defined(BUILTIN_MMUS)
			case 'f':
				if (opP->mode != MSCR || (opP->reg != SFC && opP->reg != DFC))
					losing++;
				break;
#endif

#ifdef m68851
			case 'P':
				if (opP->mode != MSCR || (opP->reg != TC && opP->reg != CAL &&
				    opP->reg != VAL && opP->reg != SCC && opP->reg != AC))
					losing++;
				break;

			case 'V':
				if (opP->reg != VAL)
					losing++;
				break;

			case 'W':
				if (opP->mode != MSCR || (opP->reg != DRP && opP->reg != SRP &&
				    opP->reg != CRP))
					losing++;
				break;

			case 'X':
				if (opP->mode != MSCR ||
				    (!(opP->reg >= BAD && opP->reg <= BAD+7) &&
				     !(opP->reg >= BAC && opP->reg <= BAC+7)))
					losing++;
				break;

			case 'Y':
				if (opP->reg != PSR)
					losing++;
				break;

			case 'Z':
				if (opP->reg != PCSR)
					losing++;
				break;
#endif
#ifdef BUILTIN_MMUS
			case 'a':
				if ((opP->mode != MSCR) || (opP->reg != SRP &&
				     opP->reg != CRP && opP->reg != TC))
					losing++;
				break;
			case 'b':
				if (opP->mode != MSCR || opP->reg != MMUSR)
					losing++;
				break;
			case 'c':
				if ((opP->mode != MSCR) || (opP->reg != IC &&
				     opP->reg != DC && opP->reg != BC))
					losing++;
				break;
			case 'e':
				if ((opP->mode != MSCR) || (opP->reg != TT0 &&
				     opP->reg != TT1))
					losing++;
				break;
#endif
			default:
				as_fatal("Internal error:  Operand mode %c unknown",*s);
			}
		}
		if(!losing)
			break;
		opcode=opcode->m_next;
		if(!opcode) {		/* Fell off the end */
			the_ins.error="instruction/operands mismatch";
			return;
		}
		losing=0;
	}
	the_ins.args=opcode->m_operands;
	the_ins.numargs=opcode->m_opnum;
	the_ins.numo=opcode->m_codenum;
	the_ins.opcode[0]=getone(opcode);
	the_ins.opcode[1]=gettwo(opcode);
#ifdef NeXT_MOD
	the_ins.cpus=opcode->m_cpus;
#endif /* NeXT_MOD */

	for(s=the_ins.args,opP= &the_ins.operands[0];*s;s+=2,opP++) {
			/* This switch is a doozy.
			   What the first step; its a big one! */
		switch(s[0]) {

		case '*':
		case '~':
		case '%':
		case ';':
		case '@':
		case '!':
		case '&':
		case '$':
		case '?':
		case '/':
#ifdef m68851
		case '|':
#endif
			switch(opP->mode) {
			case IMMED:
				tmpreg=0x3c;	/* 7.4 */
				if(index("bwzl",s[1]))
					nextword=get_num(opP->con1,80);
				else
					nextword=get_num(opP->con1,0);
				if(isvar(opP->con1))
					add_fix(s[1],opP->con1,0);
				switch(s[1]) {
				case 'b':
					if(!isbyte(nextword))
						opP->error="operand out of range";
					addword(nextword);
					baseo=0;
					break;
				case 'w':
				case 'z':
#ifdef CHECK_WORD_IMMEDIATES
					if(!isword(nextword))
						opP->error="operand out of range";
#endif
					addword(nextword);
					baseo=0;
					break;
#ifdef NeXT_MOD	/* Used in the fmoveml (control) registers */
				case 's':
#endif /* NeXT_MOD */
				case 'l':
					addword(nextword>>16);
					addword(nextword);
					baseo=0;
					break;

				case 'f':
					baseo=2;
					outro=8;
					break;
				case 'F':
					baseo=4;
					outro=11;
					break;
				case 'x':
					baseo=6;
					outro=15;
					break;
#ifdef PACKED_IMMEDIATE
/* This does not work.  The call to gen_to_words() below does not put out
   68k packed decimal format. */
				case 'p':
					baseo=6;
					outro= -1;
					break;
#endif
				default:
					as_fatal("Internal error:  Can't decode %c%c",*s,s[1]);
				}
				if(!baseo)
					break;

				/* We gotta put out some float */
				if(seg(opP->con1)!=SEG_BIG) {
					int_to_gen(nextword);
					gen_to_words(words,baseo,(int32_t)outro);
					for(wordp=words;baseo--;wordp++)
						addword(*wordp);
					break;
				}		/* Its BIG */
				if(offs(opP->con1)>0) {
#ifndef NeXT_MOD 	/* fix for bug #8331 */
					as_warn("Bignum assumed to be binary bit-pattern");
#endif /* NeXT_MOD */
					if(offs(opP->con1)>baseo) {
						as_warn("Bignum too big for %c format; truncated",s[1]);
						offs(opP->con1)=baseo;
					}
					baseo-=offs(opP->con1);
					for(wordp=generic_bignum+offs(opP->con1)-1;offs(opP->con1)--;--wordp)
						addword(*wordp);
					while(baseo--)
						addword(0);
					break;
				}
				gen_to_words(words,baseo,(int32_t)outro);
				for(wordp=words;baseo--;wordp++)
					addword(*wordp);
				break;
			case DREG:
				tmpreg=opP->reg-DATA; /* 0.dreg */
				break;
			case AREG:
				tmpreg=0x08+opP->reg-ADDR; /* 1.areg */
				break;
			case AINDR:
#ifdef NeXT_MOD
				/* fixes "pc@" operand */
				if(opP->reg==PC){
					tmpreg=0x3A; /* 7.2 */
					addword(0x0000);
				}
				else
#endif /* NeXT_MOD */
				tmpreg=0x10+opP->reg-ADDR; /* 2.areg */
				break;
			case ADEC:
				tmpreg=0x20+opP->reg-ADDR; /* 4.areg */
				break;
			case AINC:
				tmpreg=0x18+opP->reg-ADDR; /* 3.areg */
				break;
			case AOFF:
				if(opP->reg==PC)
					tmpreg=0x3A; /* 7.2 */
				else
					tmpreg=0x28+opP->reg-ADDR; /* 5.areg */
				nextword=get_num(opP->con1,80);
				/* Force into index mode.  Hope this works */
				if(!issword(nextword)) {
					if(opP->reg==PC)
						tmpreg=0x3B;	/* 7.3 */
					else
						tmpreg=0x30+opP->reg-ADDR;	/* 6.areg */
					/* addword(0x0171); */
/* 171 seems to be wrong, and I can't find the 68020 manual, so we'll try 170
   (which is what the Sun asm seems to generate */
					addword(0x0170);
					if(isvar(opP->con1))
						add_fix('l',opP->con1,0);
					addword(nextword>>16);
					/* addword(nextword); */
				} else if(isvar(opP->con1)) {
					if(opP->reg==PC)
					    tmpreg=0x3B; /* 7.3 */
					else
					    tmpreg=0x30+opP->reg-ADDR;
					addword(0x0170);
					/*
					 * If this is a pc register with
					 * a SEGDIFF where the -symbol
					 * is "." adjust the value of
					 * of "." to include the two
					 * bytes of opcode.
					 */
					if(opP->reg==PC &&
					   seg(opP->con1) == SEG_DIFFSECT &&
					   opP->con1-> e_exp.X_subtract_symbol->
					     sy_frag == frag_now &&
					   strcmp(opP->con1->
					      e_exp.X_subtract_symbol->sy_name,
						  "L0\001") == 0)
					   opP->con1-> e_exp.X_subtract_symbol->
					     sy_nlist.n_value += 2;
					add_fix('l',opP->con1,0);
					addword(nextword>>16);
				}
				addword(nextword);
				break;
			case AINDX:
			case APODX:
			case AMIND:
			case APRDX:
				nextword=0;
				baseo=get_num(opP->con1,80);
				outro=get_num(opP->con2,80);
					/* Figure out the 'addressing mode' */
					/* Also turn on the BASE_DISABLE bit, if needed */
				if(opP->reg==PC || opP->reg==ZPC) {
					tmpreg=0x3b; /* 7.3 */
					if(opP->reg==ZPC)
						nextword|=0x80;
				} else if(opP->reg==FAIL) {
					nextword|=0x80;
					tmpreg=0x30;	/* 6.garbage */
				} else tmpreg=0x30+opP->reg-ADDR; /* 6.areg */

				siz1= (opP->con1) ? opP->con1->e_siz : 0;
				siz2= (opP->con2) ? opP->con2->e_siz : 0;

					/* Index register stuff */
				if(opP->ireg>=DATA+0 && opP->ireg<=ADDR+7) {
					nextword|=(opP->ireg-DATA)<<12;

					if(opP->isiz==0 || opP->isiz==3)
						nextword|=0x800;
					switch(opP->imul) {
					case 1: break;
					case 2: nextword|=0x200; break;
					case 4: nextword|=0x400; break;
					case 8: nextword|=0x600; break;
					default: abort();
					}
						/* IF its simple, GET US OUT OF HERE! */
						/* Must be INDEX, with an index register.  Address register
						   cannot be ZERO-PC, and either :b was forced, or we know it'll fit */
					if(opP->mode==AINDX &&
 opP->reg!=FAIL && opP->reg!=ZPC && (siz1==1 || (issbyte(baseo) &&
 !isvar(opP->con1)))) {
						nextword +=baseo&0xff;
						addword(nextword);
						if(isvar(opP->con1))
							add_fix('B',opP->con1,0);
						break;
					}
				} else
					nextword|=0x40;	/* No index reg */

					/* It aint simple */
				nextword|=0x100;
					/* If the guy specified a width, we assume that
					   it is wide enough.  Maybe it isn't.  Ifso, we lose
					 */
				switch(siz1) {
				case 0:
					if(isvar(opP->con1) || !issword(baseo)) {
						siz1=3;
						nextword|=0x30;
					} else if(baseo==0)
						nextword|=0x10;
					else {	
						nextword|=0x20;
						siz1=2;
					}
					break;
				case 1:
					as_warn("Byte dispacement won't work.  Defaulting to :w");
				case 2:
					nextword|=0x20;
					break;
				case 3:
					nextword|=0x30;
					break;
				}

					/* Figure out innner displacement stuff */
				if(opP->mode!=AINDX) {
					switch(siz2) {
					case 0:
						if(isvar(opP->con2) || !issword(outro)) {
							siz2=3;
							nextword|=0x3;
						} else if(outro==0)
							nextword|=0x1;
						else {	
							nextword|=0x2;
							siz2=2;
						}
						break;
					case 1:
						as_warn("Byte dispacement won't work.  Defaulting to :w");
					case 2:
						nextword|=0x2;
						break;
					case 3:
						nextword|=0x3;
						break;
					}
					if(opP->mode==APODX) nextword|=0x04;
					else if(opP->mode==AMIND) nextword|=0x40;
				}
				addword(nextword);

				if(isvar(opP->con1))
					add_fix(siz1==3 ? 'l' : 'w',opP->con1,0);
				if(siz1==3)
					addword(baseo>>16);
				if(siz1)
					addword(baseo);

				if(isvar(opP->con2))
					add_fix(siz2==3 ? 'l' : 'w',opP->con2,0);
				if(siz2==3)
					addword(outro>>16);
				if(siz2)
					addword(outro);

				break;

			case ABSL:
				nextword=get_num(opP->con1,80);
				switch(opP->con1->e_siz) {
				case 1: /* treat like not there, fall through */
				    as_warn("ignoring :b suffix on %*s",
				       (int)(opP->con1->e_end-opP->con1->e_beg),
				       opP->con1->e_beg);
				case 0:
					if(!isvar(opP->con1) &&
					   issword(offs(opP->con1))) {
						tmpreg=0x38; /* 7.0 */
						addword(nextword);
						break;
					}
					/* Don't generate pc relative code
					   on 68010 and 68000 */
					if(isvar(opP->con1) &&
					   !subs(opP->con1) &&
					   seg(opP->con1) == SEG_SECT &&
					   frchain_now->frch_nsect ==
					       opP->con1->e_exp.X_add_symbol->
						    sy_nlist.n_sect &&
					   flagseen['m'] == 0 &&
					    !index("~%&$?", s[0])) {
						tmpreg=0x3A; /* 7.2 */
						add_frag(adds(opP->con1),
							 offs(opP->con1),
							 TAB(PCREL,SZ_UNDEF));
						break;
					}
				case 3:		/* Fall through into long */
					if(isvar(opP->con1))
						add_fix('l',opP->con1,0);

					tmpreg=0x39;	/* 7.1 mode */
					addword(nextword>>16);
					addword(nextword);
					break;

				case 2:		/* Word */
					if(isvar(opP->con1))
						add_fix('w',opP->con1,0);

					tmpreg=0x38;	/* 7.0 mode */
					addword(nextword);
					break;
				}
				break;
			case MSCR:
			default:
				as_bad("unknown/incorrect operand");
				/* abort(); */
			}
			install_gen_operand(s[1],tmpreg);
			break;

		case '#':
		case '^':
			switch(s[1]) {	/* JF: I hate floating point! */
			case 'j':
				tmpreg=70;
				break;
			case '8':
				tmpreg=20;
				break;
			case 'C':
				tmpreg=50;
				break;
			case '3':
			default:
				tmpreg=80;
				break;
			}
			tmpreg=get_num(opP->con1,tmpreg);
			if(isvar(opP->con1))
				add_fix(s[1],opP->con1,0);
			switch(s[1]) {
			case 'b':	/* Danger:  These do no check for
					   certain types of overflow.
					   user beware! */
				if(!isbyte(tmpreg))
					opP->error="out of range";
				insop(tmpreg);
				if(isvar(opP->con1))
					the_ins.reloc[the_ins.nrel-1].n=(opcode->m_codenum)*2;
				break;
#ifdef NeXT_MOD
			case 'j':
				if(tmpreg < 0 || tmpreg > 0xfff)
					opP->error="out of range";
				tmpreg&=0xFFF;
				install_operand(s[1],tmpreg);
				break;
#endif /* NeXT_MOD */
			case 'w':
			case 'z':
#ifdef CHECK_WORD_IMMEDIATES
				if(!isword(tmpreg))
					opP->error="out of range";
#endif
				insop(tmpreg);
				if(isvar(opP->con1))
					the_ins.reloc[the_ins.nrel-1].n=(opcode->m_codenum)*2;
				break;
			case 'l':
				insop(tmpreg);		/* Because of the way insop works, we put these two out backwards */
				insop(tmpreg>>16);
				if(isvar(opP->con1))
					the_ins.reloc[the_ins.nrel-1].n=(opcode->m_codenum)*2;
				break;
			case '3':
				tmpreg&=0xFF;
#ifdef NeXT_MOD
				if (isvar(opP->con1))
				  the_ins.reloc[the_ins.nrel-1].n = 
				    (opcode->m_codenum) + 1;
#endif /* NeXT_MOD */
			case '8':
			case 'C':
				install_operand(s[1],tmpreg);
				break;
			default:
				as_fatal("Internal error:  Unknown mode #%c",s[1]);
			}
			break;

		case '+':
		case '-':
		case 'A':
			install_operand(s[1],opP->reg-ADDR);
			break;

		case 'B':
			tmpreg = get_num(opP->con1, 80);
			switch(s[1]){
			case 'g':
			    /* Deal with fixed size stuff by hand */
			    if(opP->con1->e_siz){
				switch(opP->con1->e_siz){
				case 1:
				    add_fix('B', opP->con1, 1);
				    break;
				case 2:
				    if(strncmp(instring, "jbsr", 4) == 0){
					if(isvar(opP->con1))
					    add_fix('w', opP->con1, 0);
					/* force a jsr 7.0 mode (xxx):W */
					the_ins.opcode[the_ins.numo-1] = 0x4eb8;
					addword(tmpreg);
					break;
				    }
				    if(strncmp(instring, "jra", 3) == 0){
					if(isvar(opP->con1))
					    add_fix('w', opP->con1, 0);
					/* force a jmp 7.0 mode (xxx):W */
					the_ins.opcode[the_ins.numo-1] = 0x4ef8;
					addword(tmpreg);
					break;
				    }
				    opP->con1->e_exp.X_add_number += 2;
				    add_fix('w', opP->con1, 1);
				    addword(0);
				    break;
				case 3:
				    the_ins.opcode[the_ins.numo-1] |= 0xff;
				    opP->con1->e_exp.X_add_number += 4;
				    add_fix('l', opP->con1, 1);
				    addword(0);
				    addword(0);
				    break;
				default:
				    as_fatal("Bad size for expression %d",
					     opP->con1->e_siz);
				}
			    }
			    else if(subs(opP->con1)){
				/* We can't relax it */
				the_ins.opcode[the_ins.numo-1] |= 0xff;
				add_fix('l', opP->con1, 1);
				addword(0);
				addword(0);
			    }
			    else if(adds(opP->con1)){
				if(flagseen['m'] && 
				   (the_ins.opcode[0] >= 0x6200) &&
				   (the_ins.opcode[0] <= 0x6f00)){
				    add_frag(adds(opP->con1),
					     offs(opP->con1),
					     TAB(BCC68000, SZ_UNDEF));
				}
				else{
				    add_frag(adds(opP->con1),
					     offs(opP->con1),
					     TAB(BRANCH, SZ_UNDEF));
				}
			    }
			    else{
				the_ins.opcode[the_ins.numo-1] |= 0xff;
				opP->con1->e_exp.X_add_number += 4;
				add_fix('l', opP->con1, 1);
				addword(0);
				addword(0);
			    }
			    break;
			case 'w':
			    if(isvar(opP->con1)){
				/* check for DBcc instruction */
				if((the_ins.opcode[0] & 0xf0f8) ==0x50c8){
				    /* size varies if patch */
				    /* needed for long form */
				    add_frag(adds(opP->con1),
					     offs(opP->con1),
					     TAB(DBCC, SZ_UNDEF));
				    break;
				}
			    }
			    opP->con1->e_exp.X_add_number += 2;
			    add_fix('w', opP->con1, 1);
			    addword(0);
			    break;
			case 'c':
			    if(opP->con1->e_siz){
				switch(opP->con1->e_siz){
				case 2:
				    opP->con1->e_exp.X_add_number += 2;
				    add_fix('w', opP->con1, 1);
				    addword(0);
				    break;
				case 3:
				    the_ins.opcode[the_ins.numo-1] |= 0x40;
				    opP->con1->e_exp.X_add_number += 4;
				    add_fix('l', opP->con1, 1);
				    addword(0);
				    addword(0);
				    break;
				default:
				    as_bad("Bad size for offset, must be word "
					   "or long");
				    break;
				}
			    }
			    else if(subs(opP->con1)){
				/* We can't relax it */
				the_ins.opcode[the_ins.numo-1] |= 0x40;
				add_fix('l', opP->con1, 1);
				addword(0);
				addword(0);
			    }
			    else if(adds(opP->con1)){
				add_frag(adds(opP->con1),
					 offs(opP->con1),
					 TAB(FBRANCH, SZ_UNDEF));
			    }
			    else{
				the_ins.opcode[the_ins.numo-1] |= 0x40;
				opP->con1->e_exp.X_add_number += 4;
				add_fix('l', opP->con1, 1);
				addword(0);
				addword(0);
			    }
			    break;
			default:
			    as_fatal("Internal error: operand type B%c unknown",
				     s[1]);
			}
			break;

		case 'C':		/* Ignore it */
			break;

		case 'd':		/* JF this is a kludge */
			if(opP->mode==AOFF) {
				install_operand('s',opP->reg-ADDR);
			} else {
				char *tmpP;

				tmpP=opP->con1->e_end-2;
				opP->con1->e_beg++;
				opP->con1->e_end-=4;	/* point to the , */
				baseo=m68k_reg_parse(&tmpP);
				if(baseo<ADDR+0 || baseo>ADDR+7) {
					as_bad("Unknown address reg, using A0");
					baseo=0;
				} else baseo-=ADDR;
				install_operand('s',baseo);
			}
			tmpreg=get_num(opP->con1,80);
			if(!issword(tmpreg)) {
				as_warn("Expression out of range, using 0");
				tmpreg=0;
			}
			addword(tmpreg);
			break;

		case 'D':
			install_operand(s[1],opP->reg-DATA);
			break;

		case 'F':
			install_operand(s[1],opP->reg-FPREG);
			break;

		case 'I':
			tmpreg=1+opP->reg-COPNUM;
			if(tmpreg==8)
				tmpreg=0;
			install_operand(s[1],tmpreg);
			break;

		case 'J':		/* JF foo */
			switch(opP->reg) {
			case SFC:
				tmpreg=0;
				break;
			case DFC:
				tmpreg=0x001;
				break;
			case CACR:
				tmpreg=0x002;
				break;
			case USP:
				tmpreg=0x800;
				break;
			case VBR:
				tmpreg=0x801;
				break;
			case CAAR:
				tmpreg=0x802;
				break;
			case MSP:
				tmpreg=0x803;
				break;
			case ISP:
				tmpreg=0x804;
				break;
#ifdef BUILTIN_MMUS
			case TC:
				tmpreg=0x003;
				break;
			case ITT0:
				tmpreg=0x004;
				break;
			case ITT1:
				tmpreg=0x005;
				break;
			case DTT0:
				tmpreg=0x006;
				break;
			case DTT1:
				tmpreg=0x007;
				break;
			case MMUSR:
				tmpreg=0x805;
				break;
			case URP:
				tmpreg=0x806;
				break;
			case SRP:
				tmpreg=0x807;
				break;
#endif /* BUILTIN_MMUS */
			default:
				abort();
			}
			install_operand(s[1],tmpreg);
			break;
#ifdef NeXT_MOD
		case '0':
			tmpreg=opP->reg-ADDR;
			install_operand(s[1],tmpreg);
			break;
#endif /* NeXT_MOD */

		case 'k':
			tmpreg=get_num(opP->con1,55);
			install_operand(s[1],tmpreg&0x7f);
			break;

		case 'l':
			tmpreg=opP->reg;
			if(s[1]=='w') {
				if(tmpreg&0x7FF0000)
					as_bad("Floating point register in register list");
				insop(reverse_16_bits(tmpreg));
			} else {
				if(tmpreg&0x700FFFF)
					as_bad("Wrong register in floating-point reglist");
				install_operand(s[1],reverse_8_bits(tmpreg>>16));
			}
			break;

		case 'L':
			tmpreg=opP->reg;
			if(s[1]=='w') {
				if(tmpreg&0x7FF0000)
					as_bad("Floating point register in register list");
				insop(tmpreg);
			} else if(s[1]=='8') {
				if(tmpreg&0x0FFFFFF)
					as_bad("incorrect register in reglist");
				install_operand(s[1],tmpreg>>24);
			} else {
				if(tmpreg&0x700FFFF)
					as_bad("wrong register in floating-point reglist");
				else
					install_operand(s[1],tmpreg>>16);
			}
			break;

		case 'M':
			install_operand(s[1],get_num(opP->con1,60));
			break;

		case 'O':
			tmpreg= (opP->mode==DREG)
				? (int)(0x20+opP->reg-DATA)
				: (get_num(opP->con1,40)&0x1F);
			install_operand(s[1],tmpreg);
			break;

		case 'Q':
			tmpreg=get_num(opP->con1,10);
			if(tmpreg==8)
				tmpreg=0;
			install_operand(s[1],tmpreg);
			break;

		case 'R':
			/* This depends on the fact that ADDR registers are
			   eight more than their corresponding DATA regs, so
			   the result will have the ADDR_REG bit set */
			install_operand(s[1],opP->reg-DATA);
			break;

		case 's':
			if(opP->reg==FPI) tmpreg=0x1;
			else if(opP->reg==FPS) tmpreg=0x2;
			else if(opP->reg==FPC) tmpreg=0x4;
			else abort();
			install_operand(s[1],tmpreg);
			break;

		case 'S':	/* Ignore it */
			break;

		case 'T':
			install_operand(s[1],get_num(opP->con1,30));
			break;

		case 'U':	/* Ignore it */
			break;

#if defined(m68851) || defined(BUILTIN_MMUS)
			/* JF: These are out of order, I fear. */
		case 'f':
			switch (opP->reg) {
			case SFC:
				tmpreg=0;
				break;
			case DFC:
				tmpreg=1;
				break;
			default:
				abort();
			}
			install_operand(s[1],tmpreg);
			break;
#endif

#ifdef BUILTIN_MMUS
		case 'a':
			switch (opP->reg) {
			case SRP:
				tmpreg=2;
				break;
			case CRP:
				tmpreg=3;
				break;
			case TC:
				tmpreg=0;
				break;
			default:
				abort();
			}
			install_operand(s[1],tmpreg);
			break;
		case 'b':
			switch (opP->reg) {
			case MMUSR:
				tmpreg=0;
				break;
			default:
				abort();
			}
			install_operand(s[1],tmpreg);
			break;
		case 'c':
			switch (opP->reg) {
			case IC:
				tmpreg=2;
				break;
			case DC:
				tmpreg=1;
				break;
			case BC:
				tmpreg=3;
				break;
			default:
				abort();
			}
			install_operand(s[1],tmpreg);
			break;
		case 'e':
			switch (opP->reg) {
			case TT0:
				tmpreg=2;
				break;
			case TT1:
				tmpreg=3;
				break;
			default:
				abort();
			}
			install_operand(s[1],tmpreg);
			break;
#endif

#ifdef m68851
		case 'P':
			switch(opP->reg) {
			case TC:
				tmpreg=0;
				break;
			case CAL:
				tmpreg=4;
				break;
			case VAL:
				tmpreg=5;
				break;
			case SCC:
				tmpreg=6;
				break;
			case AC:
				tmpreg=7;
				break;
			default:
				abort();
			}
			install_operand(s[1],tmpreg);
			break;

		case 'V':
			if (opP->reg == VAL)
				break;
			abort();

		case 'W':
			switch(opP->reg) {

			case DRP:
				tmpreg=1;
				break;
			case SRP:
				tmpreg=2;
				break;
			case CRP:
				tmpreg=3;
				break;
			default:
				abort();
			}
			install_operand(s[1],tmpreg);
			break;

		case 'X':
			switch (opP->reg) {
			case BAD: case BAD+1: case BAD+2: case BAD+3:
			case BAD+4: case BAD+5: case BAD+6: case BAD+7:
				tmpreg = (4 << 10) | ((opP->reg - BAD) << 2);
				break;

			case BAC: case BAC+1: case BAC+2: case BAC+3:
			case BAC+4: case BAC+5: case BAC+6: case BAC+7:
				tmpreg = (5 << 10) | ((opP->reg - BAC) << 2);
				break;

			default:
				abort();
			}
			install_operand(s[1], tmpreg);
			break;
		case 'Y':
			if (opP->reg == PSR)
				break;
			abort();

		case 'Z':
			if (opP->reg == PCSR)
				break;
			abort();
#endif /* m68851 */
		default:
			as_fatal("Internal error:  Operand type %c unknown",s[0]);
		}
	}
	/* By the time whe get here (FINALLY) the_ins contains the complete
	   instruction, ready to be emitted. . . */
}

static
int
get_regs(
int i,
struct m68k_op *opP,
char *str)
{
	/*			     26, 25, 24, 23-16,  15-8, 0-7 */
	/* Low order 24 bits encoded fpc,fps,fpi,fp7-fp0,a7-a0,d7-d0 */
	uint32_t cur_regs = 0;
	int	reg1,
		reg2;

#define ADD_REG(x)	{     if(x==FPI) cur_regs|=(1<<24);\
			 else if(x==FPS) cur_regs|=(1<<25);\
			 else if(x==FPC) cur_regs|=(1<<26);\
			 else cur_regs|=(1<<(x-1));  }

	reg1=i;
	for(;;) {
		if(*str=='/') {
			ADD_REG(reg1);
			str++;
		} else if(*str=='-') {
			str++;
			reg2=m68k_reg_parse(&str);
			if(reg2<DATA || reg2>=FPREG+8 || reg1==FPI || reg1==FPS || reg1==FPC) {
				opP->error="unknown register in register list";
				return FAIL;
			}
			while(reg1<=reg2) {
				ADD_REG(reg1);
				reg1++;
			}
			if(*str=='\0')
				break;
		} else if(*str=='\0') {
			ADD_REG(reg1);
			break;
		} else {
			opP->error="unknow character in register list";
			return FAIL;
		}
/* DJA -- Bug Fix.  Did't handle d1-d2/a1 until the following instruction was added */
		if (*str=='/')
		  str ++;
		reg1=m68k_reg_parse(&str);
		if((reg1<DATA || reg1>=FPREG+8) && !(reg1==FPI || reg1==FPS || reg1==FPC)) {
			opP->error="unknown register in register list";
			return FAIL;
		}
	}
	opP->reg=cur_regs;
	return OK;
}

static
int
reverse_16_bits(
int in)
{
	int out=0;
	int n;

	static int mask[16] = {
0x0001,0x0002,0x0004,0x0008,0x0010,0x0020,0x0040,0x0080,
0x0100,0x0200,0x0400,0x0800,0x1000,0x2000,0x4000,0x8000
	};
	for(n=0;n<16;n++) {
		if(in&mask[n])
			out|=mask[15-n];
	}
	return out;
}

static
int
reverse_8_bits(
int in)
{
	int out=0;
	int n;

	static int mask[8] = {
0x0001,0x0002,0x0004,0x0008,0x0010,0x0020,0x0040,0x0080,
	};

	for(n=0;n<8;n++) {
		if(in&mask[n])
			out|=mask[7-n];
	}
	return out;
}

static
void
install_operand(
int mode,
int val)
{
	switch(mode) {
	case 's':
		the_ins.opcode[0]|=val & 0xFF;	/* JF FF is for M kludge */
		break;
	case 'd':
		the_ins.opcode[0]|=val<<9;
		break;
	case '1':
		the_ins.opcode[1]|=val<<12;
		break;
	case '2':
		the_ins.opcode[1]|=val<<6;
		break;
	case '3':
		the_ins.opcode[1]|=val;
		break;
	case '4':
		the_ins.opcode[2]|=val<<12;
		break;
	case '5':
		the_ins.opcode[2]|=val<<6;
		break;
	case '6':
			/* DANGER!  This is a hack to force cas2l and cas2w cmds
			   to be three words long! */
		the_ins.numo++;
		the_ins.opcode[2]|=val;
		break;
	case '7':
		the_ins.opcode[1]|=val<<7;
		break;
	case '8':
		the_ins.opcode[1]|=val<<10;
		break;
#if defined(m68851) || defined(BUILTIN_MMUS)
	case '9':
		the_ins.opcode[1]|=val<<5;
		break;
#endif
#ifdef BUILTIN_MMUS
	case 'S':
		the_ins.opcode[0]|=val<<6;
		break;
#endif
	case 't':
		the_ins.opcode[1]|=(val<<10)|(val<<7);
		break;
	case 'D':
		the_ins.opcode[1]|=(val<<12)|val;
		break;
	case 'g':
		the_ins.opcode[0]|=val=0xff;
		break;
	case 'i':
		the_ins.opcode[0]|=val<<9;
		break;
	case 'C':
		the_ins.opcode[1]|=val;
		break;
	case 'j':
		the_ins.opcode[1]|=val;
		the_ins.numo++;		/* What a hack */
		break;
	case 'k':
		the_ins.opcode[1]|=val<<4;
		break;
	case 'b':
	case 'w':
	case 'l':
		break;
	case 'c':
	default:
		abort();
	}
}

static
void
install_gen_operand(
int mode,
int val)
{
	switch(mode) {
	case 's':
		the_ins.opcode[0]|=val;
		break;
	case 'd':
			/* This is a kludge!!! */
		the_ins.opcode[0]|=(val&0x07)<<9|(val&0x38)<<3;
		break;
	case 'b':
	case 'w':
	case 'l':
	case 'f':
	case 'F':
	case 'x':
	case 'p':
		the_ins.opcode[0]|=val;
		break;
		/* more stuff goes here */
	default:
		abort();
	}
}

static
char *
crack_operand(
char *str,
struct m68k_op *opP)
{
	register int parens;
	register int c;
	register char *beg_str;

	if(!str) {
		return str;
	}
	beg_str=str;
	for(parens=0;*str && (parens>0 || notend(str));str++) {
		if(*str == '"') {
			str++;
			while(*str && *str != '"')
				str++;
			if(*str != '"'){	/* ERROR */
				opP->error="Missing \"";
				return str;
			}
		}
		else{	
			if(*str=='(')
				parens++;
			else if(*str==')') {
				if(!parens) {		/* ERROR */
					opP->error="Extra )";
					return str;
				}
				--parens;
			}
		}
	}
	if(!*str && parens) {		/* ERROR */
		opP->error="Missing )";
		return str;
	}
	c= *str;
	*str='\0';
	if(m68k_ip_op(beg_str,opP)==FAIL) {
		*str=c;
		return str;
	}
	*str=c;
	if(c=='}')
		c= *++str;		/* JF bitfield hack */
	if(c) {
 		c= *++str;
		if(!c)
			as_bad("Missing operand");
	}
	return str;
}

/* See the comment up above where the #define notend(... is */
#if 0
notend(s)
char *s;
{
	if(*s==',') return 0;
	if(*s=='{' || *s=='}')
		return 0;
	if(*s!=':') return 1;
		/* This kludge here is for the division cmd, which is a kludge */
	if(index("aAdD#",s[1])) return 0;
	return 1;
}
#endif /* 0 */

#ifdef NeXT_MOD
static char *file_030, *file_040;
static uint32_t line_030, line_040;
#endif /* NeXT_MOD */

/* This is the guts of the machine-dependent assembler.  STR points to a
   machine dependent instruction.  This funciton is supposed to emit
   the frags/bytes it assembles to.
 */
void
md_assemble(
char *str)
{
	char *er;
	short	*fromP;
	char	*toP = NULL;
	int	m,n;
	char	*to_beg_P;
	int	shorts_this_frag;

	n = 0;
	memset((char *)(&the_ins), '\0', sizeof(the_ins));
	m68_ip(str);
	er=the_ins.error;
	if(!er) {
		for(n=the_ins.numargs;n;--n)
			if(the_ins.operands[n].error) {
				er=the_ins.operands[n].error;
				break;
			}
	}
	if(er) {
		as_bad("\"%s\" -- Statement '%s' ignored",er,str);
		return;
	}

#ifdef NeXT_MOD
	if(the_ins.cpus != NULL && !force_cpusubtype_ALL){
	    if(md_cpusubtype == CPU_SUBTYPE_MC680x0_ALL){
		switch(*the_ins.cpus){
		case '2':
		    as_bad("implementation specific instruction for the MC68020"
			   " and -force_cpusubtype_ALL not specified");
		    break;
		case '3':
		    if(archflag_cpusubtype == CPU_SUBTYPE_MC68040)
			as_bad("030 instruction not allowed with -arch m68040");
		    else{
			file_030 = logical_input_file ?
				   logical_input_file : physical_input_file;
			line_030 = logical_input_line ?
				   logical_input_line : physical_input_line;
			md_cpusubtype = CPU_SUBTYPE_MC68030_ONLY;
		    }
		    break;
		case '4':
		    if(archflag_cpusubtype == CPU_SUBTYPE_MC68030_ONLY)
			as_bad("040 instruction not allowed with -arch m68030");
		    else{
			file_040 = logical_input_file ?
				   logical_input_file : physical_input_file;
			line_040 = logical_input_line ?
				   logical_input_line : physical_input_line;
			md_cpusubtype = CPU_SUBTYPE_MC68040;
		    }
		    break;
		}
	    }
	    else{
		switch(*the_ins.cpus){
		case '2':
		    as_bad("implementation specific instruction for the MC68020"
			   " and -force_cpusubtype_ALL not specified");
		    break;
		case '3':
		    if(archflag_cpusubtype == CPU_SUBTYPE_MC68040)
			as_bad("030 instruction not allowed with -arch m68040");
		    else{
			if(md_cpusubtype != CPU_SUBTYPE_MC680x0_ALL &&
			   md_cpusubtype != CPU_SUBTYPE_MC68030_ONLY)
			    as_bad("more than one implementation specific "
				   "instruction seen and -force_cpusubtype_ALL "
				   " not specified (first 040 instruction in: "
				   "%s at line %u)", file_040, line_040);
			md_cpusubtype = CPU_SUBTYPE_MC68030_ONLY;
		    }
		    break;
		case '4':
		    if(archflag_cpusubtype == CPU_SUBTYPE_MC68030_ONLY)
			as_bad("040 instruction not allowed with -arch m68030");
		    else{
			if(md_cpusubtype != CPU_SUBTYPE_MC680x0_ALL &&
			   md_cpusubtype != CPU_SUBTYPE_MC68040)
			    as_bad("more than one implementation specific "
				   "instruction seen and -force_cpusubtype_ALL "
				   "not specified (first 030 instruction in: "
				   "%s at line %u)", file_030, line_030);
			md_cpusubtype = CPU_SUBTYPE_MC68040;
		    }
		    break;
		}
	    }
	}
#endif /* NeXT_MOD */

#ifdef NeXT_MOD	/* generate stabs for debugging assembly code */
	/*
	 * If the -g flag is present generate a line number stab for the
	 * instruction.
	 * 
	 * See the detailed comments about stabs in read_a_source_file() for a
	 * description of what is going on here.
	 */
	if(flagseen['g'] && frchain_now->frch_nsect == text_nsect){
	    (void)symbol_new(
		  "",
		  68 /* N_SLINE */,
		  text_nsect,
		  logical_input_line /* n_desc, line number */,
		  obstack_next_free(&frags) - frag_now->fr_literal,
		  frag_now);
	}
#endif /* NeXT_MOD */

#ifdef NeXT_MOD	/* mark sections containing instructions */
	/*
	 * We are putting a machine instruction in this section so mark it as
	 * containg some machine instructions.
	 */
	frchain_now->frch_section.flags |= S_ATTR_SOME_INSTRUCTIONS;
#endif /* NeXT_MOD */

	if(the_ins.nfrag==0) {	/* No frag hacking involved; just put it out */
		toP=frag_more(2*the_ins.numo);
		fromP= &the_ins.opcode[0];
		for(m=the_ins.numo;m;--m) {
			md_number_to_chars(toP,(int32_t)(*fromP),2);
			toP+=2;
			fromP++;
		}
			/* put out symbol-dependent info */
		for(m=0;m<the_ins.nrel;m++) {
			switch(the_ins.reloc[m].wid) {
			case 'B':
				n=1;
				break;
			case 'b':
				n=1;
				break;
			case '3':
#ifdef NeXT_MOD
				/* This is a bug fix that is not in the 1.36
				 * version of GAS for this construct:
				 *	fmovemx	#foo,a0@-
				 * foo = 0xffff;
				 * Where the width of the relocation should be
				 * one byte (the low 8 bits of the second word)
				 * for the floating point register mask. Other-					 * wise the next byte after this instruction
				 * gets trashed by this relocation.
				 */
				n=1;
#else /* !defined(NeXT_MOD) */
				n=2;
#endif /* NeXT_MOD */
				break;
			case 'w':
				n=2;
				break;
			case 'l':
				n=4;
				break;
			default:
				as_fatal("Don't know how to figure width of %c in md_assemble()",the_ins.reloc[m].wid);
			}

			fix_new(frag_now,
			        (toP - frag_now->fr_literal) -
				    the_ins.numo * 2 + the_ins.reloc[m].n,
				n,
				the_ins.reloc[m].add,
				the_ins.reloc[m].sub,
				the_ins.reloc[m].off,
				the_ins.reloc[m].pcrel,
				0,0);
		}
		return;
	}

		/* There's some frag hacking */
	for(n=0,fromP= &the_ins.opcode[0];n<the_ins.nfrag;n++) {
		int wid;

		if(n==0) wid=2*the_ins.fragb[n].fragoff;
		else wid=2*(the_ins.numo-the_ins.fragb[n-1].fragoff);
		toP=frag_more(wid);
		to_beg_P=toP;
		shorts_this_frag=0;
		for(m=wid/2;m;--m) {
			md_number_to_chars(toP,(int32_t)(*fromP),2);
			toP+=2;
			fromP++;
			shorts_this_frag++;
		}
		for(m=0;m<the_ins.nrel;m++) {
			if((the_ins.reloc[m].n)>= 2*shorts_this_frag /* 2*the_ins.fragb[n].fragoff */) {
				the_ins.reloc[m].n-= 2*shorts_this_frag /* 2*the_ins.fragb[n].fragoff */;
				break;
			}
			wid=the_ins.reloc[m].wid;
			if(wid==0)
				continue;
			the_ins.reloc[m].wid=0;
			wid = (wid=='b') ? 1 : (wid=='w') ? 2 : (wid=='l') ? 4 : 4000;

			fix_new(frag_now,
			        (toP - frag_now->fr_literal) -
				    the_ins.numo * 2 + the_ins.reloc[m].n,
				wid,
				the_ins.reloc[m].add,
				the_ins.reloc[m].sub,
				the_ins.reloc[m].off,
				the_ins.reloc[m].pcrel,
				0,0);
		}
		know(the_ins.fragb[n].fadd);
		(void)frag_var(rs_machine_dependent,10,0,(relax_substateT)(the_ins.fragb[n].fragty),
 the_ins.fragb[n].fadd,the_ins.fragb[n].foff,to_beg_P);
	}
	n=(the_ins.numo-the_ins.fragb[n-1].fragoff);
	shorts_this_frag=0;
	if(n) {
		toP=frag_more(n*sizeof(short));
		while(n--) {
			md_number_to_chars(toP,(int32_t)(*fromP),2);
			toP+=2;
			fromP++;
			shorts_this_frag++;
		}
	}
	for(m=0;m<the_ins.nrel;m++) {
		int wid;

		wid=the_ins.reloc[m].wid;
		if(wid==0)
			continue;
		the_ins.reloc[m].wid=0;
		wid = (wid=='b') ? 1 : (wid=='w') ? 2 : (wid=='l') ? 4 : 4000;

		fix_new(frag_now,
		        (the_ins.reloc[m].n + toP-frag_now->fr_literal) -
			    /* the_ins.numo */ shorts_this_frag * 2,
			wid,
			the_ins.reloc[m].add,
			the_ins.reloc[m].sub,
			the_ins.reloc[m].off,
			the_ins.reloc[m].pcrel,
			0,0);
	}
}

/* This function is called once, at assembler startup time.  This should
   set up all the tables, etc that the MD part of the assembler needs
 */
void
md_begin(
void)
{
/*
 * md_begin -- set up hash tables with 68000 instructions.
 * similar to what the vax assembler does.  ---phr
 */
	/* RMS claims the thing to do is take the m68k-opcode.h table, and make
	   a copy of it at runtime, adding in the information we want but isn't
	   there.  I think it'd be better to have an awk script hack the table
	   at compile time.  Or even just xstr the table and use it as-is.  But
	   my lord ghod hath spoken, so we do it this way.  Excuse the ugly var
	   names.  */

	register struct m68k_opcode *ins;
	register struct m68_incant *hack,
		*slak;
	const char *retval = 0;		/* empty string, or error msg text */
	register int i;
	register char c;

	if ((op_hash = hash_new()) == NULL)
		as_fatal("Virtual memory exhausted");

	obstack_begin(&robyn,4000);
	for (ins = (struct m68k_opcode *)m68k_opcodes; ins < endop; ins++) {
		hack=slak=(struct m68_incant *)obstack_alloc(&robyn,sizeof(struct m68_incant));
		do {
			slak->m_operands=ins->args;
			slak->m_opnum=strlen(slak->m_operands)/2;
			slak->m_opcode=ins->opcode;
				/* This is kludgey */
			slak->m_codenum=((ins->match)&0xffffL) ? 2 : 1;
#ifdef NeXT_MOD
			slak->m_cpus = ins->cpus;
#endif /* NeXT_MOD */
			if((ins+1)!=endop && !strcmp(ins->name,(ins+1)->name)) {
				slak->m_next=(struct m68_incant *)
obstack_alloc(&robyn,sizeof(struct m68_incant));
				ins++;
			} else
				slak->m_next=0;
			slak=slak->m_next;
		} while(slak);

		retval = hash_insert (op_hash, ins->name,(char *)hack);
			/* Didn't his mommy tell him about null pointers? */
		if(retval && *retval)
			as_fatal("Internal Error:  Can't hash %s: %s",ins->name,retval);
	}

	for (i = 0; i < (int)sizeof(mklower_table) ; i++)
		mklower_table[i] = (isupper(c = (char) i)) ? tolower(c) : c;

	for (i = 0 ; i < (int)sizeof(notend_table) ; i++) {
		notend_table[i] = 0;
		alt_notend_table[i] = 0;
	}
	notend_table[','] = 1;
	notend_table['{'] = 1;
	notend_table['}'] = 1;
	alt_notend_table['a'] = 1;
	alt_notend_table['A'] = 1;
	alt_notend_table['d'] = 1;
	alt_notend_table['D'] = 1;
	alt_notend_table['#'] = 1;
	alt_notend_table['f'] = 1;
	alt_notend_table['F'] = 1;
}

#if 0
#define notend(s) ((*s == ',' || *s == '}' || *s == '{' \
                   || (*s == ':' && index("aAdD#", s[1]))) \
               ? 0 : 1)
#endif

/* This funciton is called once, before the assembler exits.  It is
   supposed to do any final cleanup for this part of the assembler.
 */
void
md_end(
void)
{
}

/* Equal to MAX_PRECISION in atof-ieee.c */
#define MAX_LITTLENUMS 6

/* Turn a string in input_line_pointer into a floating point constant of type
   type, and store the appropriate bytes in *litP.  The number of LITTLENUMS
   emitted is stored in *sizeP .  An error message is returned, or NULL on OK.
 */
char *
md_atof(
int type,
char *litP,
int *sizeP)
{
	int	prec;
	LITTLENUM_TYPE words[MAX_LITTLENUMS];
	LITTLENUM_TYPE *wordP;
	char	*t;

	switch(type) {
	case 'f':
	case 'F':
	case 's':
	case 'S':
		prec = 2;
		break;

	case 'd':
	case 'D':
	case 'r':
	case 'R':
		prec = 4;
		break;

	case 'x':
	case 'X':
		prec = 6;
		break;

	case 'p':
	case 'P':
		prec = 6;
		break;

	default:
		*sizeP=0;
		return "Bad call to MD_ATOF()";
	}
	t=atof_ieee(input_line_pointer,type,words);
	if(t)
		input_line_pointer=t;

	*sizeP=prec * sizeof(LITTLENUM_TYPE);
	for(wordP=words;prec--;) {
		md_number_to_chars(litP,(int32_t)(*wordP++),sizeof(LITTLENUM_TYPE));
		litP+=sizeof(LITTLENUM_TYPE);
	}
	return "";	/* Someone should teach Dean about null pointers */
}

/* Turn an integer of n bytes (in val) into a stream of bytes appropriate
   for use in the a.out file, and stores them in the array pointed to by buf.
   This knows about the endian-ness of the target machine and does
   THE RIGHT THING, whatever it is.  Possible values for n are 1 (byte)
   2 (short) and 4 (long)  Floating numbers are put out as a series of
   LITTLENUMS (shorts, here at least)
 */
void
md_number_to_chars(
char *buf,
signed_expr_t val,
int n)
{
	switch(n) {
	case 1:
		*buf++=val;
		break;
	case 2:
		*buf++=(val>>8);
		*buf++=val;
		break;
	case 4:
		*buf++=(val>>24);
		*buf++=(val>>16);
		*buf++=(val>>8);
		*buf++=val;
		break;
	default:
		abort();
	}
}

void
md_number_to_imm(
unsigned char *buf,
signed_expr_t val,
int n,
fixS *fixP,
int nsect)
{
	switch(n) {
	case 1:
		*buf++=val;
		break;
	case 2:
		*buf++=(val>>8);
		*buf++=val;
		break;
	case 4:
		*buf++=(val>>24);
		*buf++=(val>>16);
		*buf++=(val>>8);
		*buf++=val;
		break;
	default:
		abort();
	}
}

/*
 * Force truly undefined symbols to their maximum size, and generally set up
 * the frag list to be relaxed.  It is the caller's responsiblity to set the
 * current section, frchain_now, to the corresponding nsect specified so that
 * calls to fix_new() will make fixes for this section.
 */
int
md_estimate_size_before_relax(
fragS *fragP,
int nsect)
{
    int old_fix;

	old_fix = fragP->fr_fix;

	/*
	 * Handle SZ_UNDEF first, it can be changed to BYTE or SHORT.
	 */
	switch(fragP->fr_subtype){
	case TAB(DBCC, SZ_UNDEF):
	    if(fragP->fr_symbol->sy_nlist.n_sect == nsect){
		fragP->fr_subtype = TAB(DBCC, SHORT);
		fragP->fr_var += 2;
		break;
	    }
	    /*
	     * Only DBcc 68000 instructions can come here.
	     * Change dbcc into dbcc/jmp absl long.
	     */
	    fragP->fr_opcode[2] = 0x00;  /* branch offset = 4 */
	    fragP->fr_opcode[3] = 0x04;  
	    fragP->fr_opcode[4] = 0x60;  /* put in bra pc+6 */ 
	    fragP->fr_opcode[5] = 0x06;  
	    fragP->fr_opcode[6] = 0x4e;  /* put in jmp long (0x4ef9) */ 
	    fragP->fr_opcode[7] = 0xf9;  
	    fragP->fr_fix += 6;	  /* account for bra/jmp instructions */
	    fix_new(fragP,
		    fragP->fr_fix,
		    4,
		    fragP->fr_symbol,
		    0,
		    fragP->fr_offset,
		    0,
		    0,
		    0);
	    fragP->fr_fix += 4;	/* account for jmp instruction displacement */
	    frag_wane(fragP);
	    break;

	case TAB(BCC68000, SZ_UNDEF):
	    if(fragP->fr_symbol->sy_nlist.n_sect == nsect){
		fragP->fr_subtype = TAB(BCC68000, BYTE);
		break;
	    }
	    /*
 	     * Only Bcc 68000 instructions can come here.
	     * Change bcc into b!cc/jmp absl long.
	     */
	    fragP->fr_opcode[0] ^= 0x01; /* invert bcc */
	    fragP->fr_opcode[1] = 0x6;   /* branch offset = 6 */
	    fragP->fr_opcode[2] = 0x4e;  /* put in jmp long (0x4ef9) */ 
	    fragP->fr_opcode[3] = 0xf9;  
	    fragP->fr_fix += 2;	     /* account for jmp instruction */
	    fix_new(fragP,
		    fragP->fr_fix,
		    4,
		    fragP->fr_symbol,
		    0, 
		    fragP->fr_offset,
		    0,
		    0,
		    0);
	    fragP->fr_fix += 4;	/* account for jmp instruction displacement */
	    frag_wane(fragP);
	    break;

	case TAB(BRANCH, SZ_UNDEF):
	    if(fragP->fr_symbol->sy_nlist.n_sect == nsect){
		/*
		 * The NeXT linker has the ability to scatter blocks of
		 * sections between labels.  This requires that brances to
		 * labels that survive to the link phase must be able to
		 * be relocated.
		 */
		if(fragP->fr_symbol->sy_name[0] != 'L' || flagseen ['L']){
		    fix_new(fragP,
			    fragP->fr_fix,
			    4,
			    fragP->fr_symbol,
			    0,
			    fragP->fr_offset + 4,
			    1,
			    1,
			    0);
		    fragP->fr_fix += 4;
		    fragP->fr_opcode[1] = 0xff;
		    frag_wane(fragP);
		    break;
		}
		else
		    fragP->fr_subtype = TAB(BRANCH, BYTE);
		break;
	    }
	    else if(flagseen['m']){
		if(fragP->fr_opcode[0] == 0x61){
		    fragP->fr_opcode[0] = 0x4E;
		    fragP->fr_opcode[1] = 0xB9;	/* JBSR with ABSL LONG offset */
		    fix_new(fragP,
			    fragP->fr_fix,
			    4, 
			    fragP->fr_symbol,
			    0,
			    fragP->fr_offset,
			    0,
			    0,
			    0);
		    fragP->fr_fix += 4;
		    frag_wane(fragP);
		}
		else if(fragP->fr_opcode[0] == 0x60){
		    fragP->fr_opcode[0] = 0x4E;
		    fragP->fr_opcode[1] = 0xF9;  /* JMP with ABSL LONG offset */
		    fix_new(fragP,
			    fragP->fr_fix,
			    4, 
			    fragP->fr_symbol,
			    0,
			    fragP->fr_offset,
			    0,
			    0,
			    0);
		    fragP->fr_fix += 4;
		    frag_wane(fragP);
		}
		else{
		    as_warn("Long branch offset to extern symbol not "
			    "supported.");
		}
		break;
	    }
	    else{
		/* Symbol is still undefined.  Make it simple */
		fix_new(fragP,
			fragP->fr_fix,
			4,
			fragP->fr_symbol,
			0,
			fragP->fr_offset + 4,
			1,
			1,
			0);
		fragP->fr_fix += 4;
		fragP->fr_opcode[1] = 0xff;
		frag_wane(fragP);
		break;
	    }
	    break;

	case TAB(FBRANCH, SZ_UNDEF):
	    if(fragP->fr_symbol->sy_nlist.n_sect == nsect){
		/*
		 * The NeXT linker has the ability to scatter blocks of
		 * sections between labels.  This requires that brances to
		 * labels that survive to the link phase must be able to
		 * be relocated.
		 */
		if(fragP->fr_symbol->sy_name[0] != 'L' || flagseen ['L']) {
		    fix_new(fragP,
			    fragP->fr_fix,
			    4,
			    fragP->fr_symbol,
			    0,
			    fragP->fr_offset + 4,
			    1,
			    1,
			    0);
		    fragP->fr_fix += 4;
		    fragP->fr_opcode[1] |= 0x40;
		    frag_wane(fragP);
		    break;
		}
		else{
		    fragP->fr_subtype = TAB(FBRANCH, SHORT);
		    fragP->fr_var += 2;
		}
	    }
	    else {
		/* Symbol is still undefined.  Make it long */
		fix_new(fragP,
			fragP->fr_fix,
			4,
			fragP->fr_symbol,
			0,
			fragP->fr_offset + 4,
			1,
			1,
			0);
		fragP->fr_fix += 4;
		fragP->fr_opcode[1] |= 0x40;
		frag_wane(fragP);
		break;
	    }
	    break;

	case TAB(PCREL, SZ_UNDEF):
	    if(fragP->fr_symbol->sy_nlist.n_sect == nsect){
		/*
		 * The NeXT linker has the ability to scatter blocks of
		 * sections between labels.  This requires that brances to
		 * labels that survive to the link phase must be able to
		 * be relocated.
		 */
		if(fragP->fr_symbol->sy_name[0] != 'L' || flagseen ['L']) {
		    /*
		     * The thing to do here is force it to ABSOLUTE LONG, since
		     * PCREL is really trying to shorten an ABSOLUTE address
		     * anyway.
		     */
		    if((fragP->fr_opcode[1] & 0x3F) != 0x3A)
			as_bad("Internal error (long PC-relative operand) for "
			       "insn 0x%04x at 0x%llx", fragP->fr_opcode[0],
			       fragP->fr_address);
		    fragP->fr_opcode[1] &= ~0x3F;
		    fragP->fr_opcode[1] |= 0x39;	/* Mode 7.1 */
		    fix_new(fragP,
			    fragP->fr_fix,
			    4,
			    fragP->fr_symbol,
			    0,
			    fragP->fr_offset,
			    0,
			    0,
			    0);
		    fragP->fr_fix += 4;
		    frag_wane(fragP);
		}
		else{
		    fragP->fr_subtype = TAB(PCREL, SHORT);
		    fragP->fr_var += 2;
		}
	    }
	    else {
		/* Symbol is still undefined.  Make it long */
		if((fragP->fr_opcode[1] & 0x3F) != 0x3A)
		    as_bad("Internal error (long PC-relative operand) for "
			   "insn 0x%04x at 0x%llx", fragP->fr_opcode[0],
			   fragP->fr_address);
		fragP->fr_opcode[1] &= ~0x3F;
		fragP->fr_opcode[1] |= 0x39;	/* Mode 7.1 */
		fix_new(fragP,
			fragP->fr_fix,
			4,
			fragP->fr_symbol,
			0,
			fragP->fr_offset,
			1,
			1,
			0);
		fragP->fr_fix += 4;
		frag_wane(fragP);
		break;
	    }
	    break;

	default:
	    break;
	}

	/*
	 * Now that SZ_UNDEF are taken care of, check others
	 */
	switch(fragP->fr_subtype) {
	case TAB(BCC68000, BYTE):
	case TAB(BRANCH, BYTE):
	    /*
	     * We can't do a short jump to the next instruction,
	     * so we force word mode.
	     */
	    if(fragP->fr_symbol != NULL &&
	       fragP->fr_symbol->sy_value == 0 &&
	       fragP->fr_symbol->sy_frag == fragP->fr_next) {
		fragP->fr_subtype = TAB(TABTYPE(fragP->fr_subtype), SHORT);
		fragP->fr_var += 2;
	    }
	    break;
	default:
	    break;
	}
	return(fragP->fr_var + fragP->fr_fix - old_fix);
}

/*
 * *fragP has been relaxed to its final size, and now needs to have
 * the bytes inside it modified to conform to the new size.  There is UGLY
 * MAGIC here interms of changing the addressing mode of some instructions
 * and using other instructions in place of the original in the case of the
 * 68000 and 68010 where long pc-relative forms don't exist.
 */
void
md_convert_frag(
fragS *fragP)
{
    int32_t disp;
    int32_t ext;
    char *buffer_address;
    int object_address;

	ext = 0;

	/* Address in gas core of the place to store the displacement.  */
	buffer_address = fragP->fr_fix + fragP->fr_literal;

	/* Address in object code of the displacement.  */
	object_address = fragP->fr_fix + fragP->fr_address;

	know(fragP->fr_symbol);

	/* The displacement of the address, from current location.  */
	disp = (fragP->fr_symbol->sy_value + fragP->fr_offset) - object_address;

	switch(fragP->fr_subtype){
	case TAB(BCC68000, BYTE):
	case TAB(BRANCH, BYTE):
	    know(issbyte(disp));
	    if(disp == 0){
		/* Replace this with a nop. */
		fragP->fr_opcode[0] = 0x4e;
		fragP->fr_opcode[1] = 0x71;
	    }
	    else{
		fragP->fr_opcode[1] = disp;
	    }
	    ext = 0;
	    break;

	case TAB(DBCC, SHORT):
	    know(issword(disp));
	    ext=2;
	    break;

	case TAB(BCC68000, SHORT):
	case TAB(BRANCH, SHORT):
	    know(issword(disp));
	    fragP->fr_opcode[1] = 0x00;
	    ext = 2;
	    break;

	case TAB(BRANCH,LONG):
	    if(flagseen['m']){
		if(fragP->fr_opcode[0] == 0x61){
		    fragP->fr_opcode[0] = 0x4E;
		    fragP->fr_opcode[1] = 0xB9;	/* JBSR with ABSL LONG offset */
		    fix_new(fragP,
			    fragP->fr_fix,
			    4,
			    fragP->fr_symbol,
			    0,
			    fragP->fr_offset,
			    0,
			    0,
			    0);
		    fragP->fr_fix += 4;
		    ext = 0;
		}
		else if(fragP->fr_opcode[0] == 0x60){
		    fragP->fr_opcode[0]= 0x4E;
		    fragP->fr_opcode[1]= 0xF9; /* JMP  with ABSL LONG offset */
		  fix_new(fragP,
			  fragP->fr_fix,
			  4,
			  fragP->fr_symbol,
			  0,
			  fragP->fr_offset,
			  0,
			  0,
			  0);
		  fragP->fr_fix += 4;
		  ext = 0;
		}
		else{
		    as_bad("Long branch offset not supported.");
		}
	    }
	    else{
		fragP->fr_opcode[1] = 0xff;
		ext = 4;
	    }
	    break;

	case TAB(BCC68000, LONG):
	    /*
	     * Only Bcc 68000 instructions can come here.
	     * Change bcc into b!cc/jmp absl long.
	     */
	    fragP->fr_opcode[0] ^= 0x01; /* invert bcc */
	    fragP->fr_opcode[1] = 0x6;   /* branch offset = 6 */
	    fragP->fr_opcode[2] = 0x4e;  /* put in jmp long (0x4ef9) */ 
	    fragP->fr_opcode[3] = 0xf9;  
	    fragP->fr_fix += 2;		 /* account for jmp instruction */
	    fix_new(fragP,
		    fragP->fr_fix,
		    4,
		    fragP->fr_symbol,
		    0,
		    fragP->fr_offset,
		    0,
		    0,
		    0);
	    fragP->fr_fix += 4; /* account for jmp instruction's displacement */
	    ext = 0;
	    break;

	case TAB(DBCC, LONG):
	    /*
	     * Only DBcc 68000 instructions can come here.
	     * Change dbcc into dbcc/jmp absl long.
	     */
	    fragP->fr_opcode[2] = 0x00;  /* branch offset = 4 */
	    fragP->fr_opcode[3] = 0x04;  
	    fragP->fr_opcode[4] = 0x60;  /* put in bra pc+6 */ 
	    fragP->fr_opcode[5] = 0x06;  
	    fragP->fr_opcode[6] = 0x4e;  /* put in jmp long (0x4ef9) */ 
	    fragP->fr_opcode[7] = 0xf9;  
	    fragP->fr_fix += 6;	     /* account for bra/jmp instructions */
	    fix_new(fragP,
		    fragP->fr_fix,
		    4,
		    fragP->fr_symbol,
		    0, 
		    fragP->fr_offset,
		    0,
		    0,
		    0);
	    fragP->fr_fix += 4; /* account for jmp instruction's displacement */
	    ext = 0;
	    break;

	case TAB(FBRANCH, SHORT):
	    know((fragP->fr_opcode[1] & 0x40) == 0);
	    ext = 2;
	    break;

	case TAB(FBRANCH, LONG):
	    fragP->fr_opcode[1] |= 0x40;	/* Turn on LONG bit */
	    ext = 4;
	    break;

	case TAB(PCREL,SHORT):
	    ext = 2;
	    break;

	case TAB(PCREL,LONG):
	    /*
	     * The thing to do here is force it to ABSOLUTE LONG, since
	     * PCREL is really trying to shorten an ABSOLUTE address anyway.
	     */
	    if((fragP->fr_opcode[1] & 0x3F) != 0x3A)
		as_bad("Internal error (long PC-relative operand) for insn "
		       "0x%04x at 0x%llx", fragP->fr_opcode[0],
		       fragP->fr_address);
	    fragP->fr_opcode[1] &= ~0x3F;
	    fragP->fr_opcode[1] |= 0x39;	/* Mode 7.1 */
	    fix_new(fragP,
		    fragP->fr_fix,
		    4,
		    fragP->fr_symbol,
		    0,
		    fragP->fr_offset,
		    0,
		    0,
		    0);
	    fragP->fr_fix += 4; /* account for the instruction's displacement */
	    ext = 0;
	    break;

	default:
	    break;
	}

	if(ext != 0){
	    md_number_to_chars(buffer_address, (int32_t)disp, (int)ext);
	    fragP->fr_fix += ext;
	}
}

/* Different values of OK tell what its OK to return.  Things that aren't OK are an error (what a shock, no?)

	0:  Everything is OK
	10:  Absolute 1:8	only
	20:  Absolute 0:7	only
	30:  absolute 0:15	only
	40:  Absolute 0:31	only
	50:  absolute 0:127	only
	55:  absolute -64:63    only
	60:  absolute -128:127	only
	70:  absolute 0:4095	only
	80:  No bignums

*/
static
int
get_num(
struct m68k_exp *exp,
int ok)
{
#ifdef TEST2
	int32_t	l = 0;

	if(!exp->e_beg)
		return 0;
	if(*exp->e_beg=='0') {
		if(exp->e_beg[1]=='x')
			sscanf(exp->e_beg+2,"%x",&l);
		else
			sscanf(exp->e_beg+1,"%O",&l);
		return l;
	}
	return atol(exp->e_beg);
#else /* !defined(TEST2) */
	char	*save_in;
	char	c_save;

	if(!exp) {
		/* Can't do anything */
		return 0;
	}
	if(!exp->e_beg || !exp->e_end) {
		seg(exp)=SEG_ABSOLUTE;
		adds(exp)=0;
		subs(exp)=0;
		offs(exp)= (ok==10) ? 1 : 0;
		as_warn("Null expression defaults to %lld", offs(exp));
		return 0;
	}

	exp->e_siz=0;
	if(/* ok!=80 && */exp->e_end[-1]==':' && (exp->e_end-exp->e_beg)>=2) {
		switch(exp->e_end[0]) {
		case 's':
		case 'b':
			exp->e_siz=1;
			break;
		case 'w':
			exp->e_siz=2;
			break;
		case 'l':
			exp->e_siz=3;
			break;
		default:
			as_bad("Unknown size for expression \"%c\"",exp->e_end[0]);
		}
		exp->e_end-=2;
	}
	c_save=exp->e_end[1];
	exp->e_end[1]='\0';
	save_in=input_line_pointer;
	input_line_pointer=exp->e_beg;
#ifdef NeXT_MOD	/* feature to try to make expressions absolute */
	(void) expression (&(exp->e_exp));
	/* DJA -- we will try to make an absolute number here */
	switch(try_to_make_absolute(&(exp->e_exp))) {
#else /* !defined(NeXT_MOD) */
	switch(expression(&(exp->e_exp))) {
#endif /* NeXT_MOD */
	case SEG_NONE:
		/* Do the same thing the VAX asm does */
		seg(exp)=SEG_ABSOLUTE;
		adds(exp)=0;
		subs(exp)=0;
		offs(exp)=0;
		if(ok==10) {
			as_warn("expression out of range: defaulting to 1");
			offs(exp)=1;
		}
		break;
	case SEG_ABSOLUTE:
		switch(ok) {
		case 10:
			if(offs(exp)<1 || offs(exp)>8) {
				as_warn("expression out of range: defaulting to 1");
				offs(exp)=1;
			}
			break;
		case 20:
			if(offs(exp)<0 || offs(exp)>7)
				goto outrange;
			break;
		case 30:
			if(offs(exp)<0 || offs(exp)>15)
				goto outrange;
			break;
		case 40:
			if(offs(exp)<0 || offs(exp)>32)
				goto outrange;
			break;
		case 50:
			if(offs(exp)<0 || offs(exp)>127)
				goto outrange;
			break;
		case 55:
			if(offs(exp)<-64 || offs(exp)>63)
				goto outrange;
			break;
		case 60:
			if(offs(exp)<-128 || offs(exp)>127)
				goto outrange;
			break;
		case 70:
			if(offs(exp)<0 || offs(exp)>4095) {
			outrange:
				as_warn("expression out of range: defaulting to 0");
				offs(exp)=0;
			}
			break;
		default:
			break;
		}
		break;
	case SEG_SECT:
	case SEG_UNKNOWN:
	case SEG_DIFFSECT:
		if(ok>=10 && ok<=70) {
			seg(exp)=SEG_ABSOLUTE;
			adds(exp)=0;
			subs(exp)=0;
			offs(exp)= (ok==10) ? 1 : 0;
			as_warn("Can't deal with expression \"%s\": defaulting "
				"to %lld", exp->e_beg, offs(exp));
		}
		break;
	case SEG_BIG:
#ifndef NeXT_MOD	/* fix for bug #8331 */ /* This hack is already done by expr */
		if(ok==80 && offs(exp)<0) {	/* HACK! Turn it into a long */
			LITTLENUM_TYPE words[6];

			gen_to_words(words,2,8L);/* These numbers are magic! */
			seg(exp)=SEG_ABSOLUTE;
			adds(exp)=0;
			subs(exp)=0;
			offs(exp)=words[1]|(words[0]<<16);
		} else if(ok!=0) {
#else /* defined(NeXT_MOD) */
		if(ok!=0) {
#endif /* NeXT_MOD */
			seg(exp)=SEG_ABSOLUTE;
			adds(exp)=0;
			subs(exp)=0;
			offs(exp)= (ok==10) ? 1 : 0;
			as_warn("Can't deal with expression \"%s\": defaulting "
				"to %lld", exp->e_beg, offs(exp));
		}
		break;
	default:
		abort();
	}
	if(input_line_pointer!=exp->e_end+1)
		as_bad("Ignoring junk after expression");
	exp->e_end[1]=c_save;
	input_line_pointer=save_in;
	if(exp->e_siz) {
		switch(exp->e_siz) {
		case 1:
			if(!isbyte(offs(exp)))
				as_warn("expression doesn't fit in BYTE");
			break;
		case 2:
			if(!isword(offs(exp)))
				as_warn("expression doesn't fit in WORD");
			break;
		}
	}
	return offs(exp);

#endif /* !defined(TEST2) */
}

/* These are the back-ends for the various machine dependent pseudo-ops.  */
void demand_empty_rest_of_line();	/* Hate those extra verbose names */

static
void
s_even(
uintptr_t value)
{
	register int power_of_2_alignment;
	register int32_t temp_fill;
	char fill;

	/* power of 2 alignment, 2^1 or 2 byte (even) alignment */
	power_of_2_alignment = 1;
	temp_fill = get_absolute_expression ();
	md_number_to_chars(&fill, temp_fill, 1);
	frag_align(power_of_2_alignment, &fill, 1, 0);
	/*
	 * If this alignment is larger than any previous alignment then this
	 * becomes the section's alignment.
	 */
	if(frchain_now->frch_section.align <
	   (uint32_t)power_of_2_alignment)
	    frchain_now->frch_section.align = power_of_2_alignment;
	demand_empty_rest_of_line();
}

static
void
s_proc(
uintptr_t value)
{
	demand_empty_rest_of_line();
}

/* s_space is defined in read.c .skip is simply an alias to it. */

int
md_parse_option(
char **argP,
int *cntP,
char ***vecP)
{
	switch(**argP) {
	case 'm':
		/* Gas simply ignores this option! */
		(*argP)++;
		if(**argP=='c')
			(*argP)++;
		if(!strcmp(*argP,"68000"))
			flagseen['m']=2;
		else if(!strcmp(*argP,"68010")) {
			flagseen['m']=1;
		} else if(!strcmp(*argP,"68020"))
			flagseen['m']=0;
		else
			as_warn("Unknown -m option ignored");
		while(**argP)
			(*argP)++;
		break;

	default:
		return 0;
	}
	return 1;
}


#ifdef TEST2

/* TEST2:  Test md_assemble() */

static
int
is_label(
char *str)
{
	while(*str == ' ')
	    str++;
	while(*str && *str != ' ')
	    str++;
	if(str[-1] == ':' || str[1] == '=')
	    return(1);
	return(0);
}

void
main(
int argc,
char *argv[],
char *envp[])
{
    char buf[120];
    char *cp;
    int n;

	m68_ip_begin();
	for(;;){
	    if(!gets(buf) || !*buf)
		break;
	    if(buf[0] == '|' || buf[1] == '.')
		continue;
	    for(cp = buf; *cp; cp++)
		if(*cp == '\t')
		    *cp = ' ';
	    if(is_label(buf))
		continue;
	    memset(&the_ins, '\0', sizeof(the_ins));
	    m68_ip(buf);
	    if(the_ins.error){
		printf("Error %s in %s\n", the_ins.error, buf);
	    }
	    else{
		printf("Opcode(%d.%s): ", the_ins.numo, the_ins.args);
		for(n = 0; n < the_ins.numo; n++)
		    printf(" 0x%x", the_ins.opcode[n] & 0xffff);
		printf("    ");
		print_the_insn(&the_ins.opcode[0], stdout);
		(void)putchar('\n');
	    }
	    for(n = 0; n < strlen(the_ins.args) / 2; n++){
		if(the_ins.operands[n].error){
		    printf("op%d Error %s in %s\n",
			   n, the_ins.operands[n].error, buf);
		    continue;
		}
		printf("mode %d, reg %d, ",
		       the_ins.operands[n].mode, the_ins.operands[n].reg);
		if(the_ins.operands[n].con1)
		    printf("con1: '%.*s', ",
			   1 + the_ins.operands[n].con1->e_end -
			       the_ins.operands[n].con1->e_beg,
			   the_ins.operands[n].con1->e_beg);
		printf("ireg %d, isiz %d, imul %d, ",
		       the_ins.operands[n].ireg,
		       the_ins.operands[n].isiz,
		       the_ins.operands[n].imul);
		if(the_ins.operands[n].con2)
		    printf("con2: '%.*s',",
			   1 + the_ins.operands[n].con2->e_end -
			       the_ins.operands[n].con2->e_beg,
			   the_ins.operands[n].con2->e_beg);
		(void)putchar('\n');
	    }
	}
	m68_ip_end();
}
#endif /* TEST2 */
