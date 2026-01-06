/* i860.c -- Assemble for the i860
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


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <mach-o/i860/reloc.h>

#include "i860-opcode.h"
#include "as.h"
#include "flonum.h"
#include "expr.h"
#include "hash.h"
#include "frags.h"
#include "fixes.h"
#include "read.h"
#include "md.h"
#include "symbols.h"
#include "messages.h"
#include "sections.h"

/*
 * These are the default cputype and cpusubtype for the i860 architecture.
 */
const cpu_type_t md_cputype = CPU_TYPE_I860;
cpu_subtype_t md_cpusubtype = CPU_SUBTYPE_I860_ALL;

/*
 * This is the byte sex for the i860 architecture.  The chip is running in
 * big endian mode so the assembler puts out the entire file (instructions
 * included) in big endian.  When the program is loaded in memory to be run
 * the program doing the loading byte swaps the fix width instructions.  If
 * this is not to be done by the loading program then BYTE_SWAP can be defined
 * in here that will put out the instructiona in little endian in the object
 * file.
 */
const enum byte_sex md_target_byte_sex = BIG_ENDIAN_BYTE_SEX;

static int i860_ip(
    char *str);
static void md_insn_to_chars(
    unsigned char *buf,
    int32_t val,
    int n);

const relax_typeS md_relax_table[] = { {0} };

/* handle of the OPCODE hash table */
static struct hash_control *op_hash = NULL;

static void s_dual(
    uintptr_t mode);
static void s_i860_align(
    uintptr_t value);
static void s_i860_org(
    uintptr_t value);

const pseudo_typeS md_pseudo_table[] = {
    { "float",	float_cons,	'f'	},
    { "int",	cons,		4	},
    { "align",	s_i860_align,	0	},	/* Alignment is in bytes */
    { "blkb",	s_space,	0	},	/* Reserve space, in bytes */
    { "dual",	s_dual,		1	},	/* Dual insn mode crock */
    { "enddual",s_dual,		0	},
    { "extern",	s_globl,	0	},	/* as860 equiv of .globl */
    { "ln",	s_line,		0	},	/* as860 equiv of .line */
    { "org",	s_i860_org,	0	},
#ifndef NeXT_MOD
    { "quad",	big_cons,	16	},	/* A quad is 16 bytes on 860 */
#endif /* NeXT_MOD */
    { "string",	stringer,	1	},	/* as860 equiv of .asciz */
    { NULL,     0,		0	},
};

static int dual_insn_mode = 0;

/* This array holds the chars that always start a comment.  If the
    pre-processor is disabled, these aren't very useful */
const char md_comment_chars[] = "|!";

/* This array holds the chars that only start a comment at the beginning of
   a line.  If the line seems to have the form '# 123 filename'
   .line and .file directives will appear in the pre-processed output */
/* Note that input_file.c hand checks for '#' at the beginning of the
   first line of the input file.  This is because the compiler outputs
   #NO_APP at the beginning of its output. */
/* Also note that a '/' followed by a '*' will always start a comment */
const char md_line_comment_chars[] = "#";

/* Chars that can be used to separate mant from exp in floating point nums */
const char md_EXP_CHARS[] = "eE";

/* Chars that mean this number is a floating point constant */
/* As in 0f12.456 */
/* or    0d1.2345e12 */
const char md_FLT_CHARS[] = "rRsSfFdDxXpP";

/* Also be aware that MAXIMUM_NUMBER_OF_CHARS_FOR_FLOAT may have to be
   changed in read.c .  Ideally it shouldn't have to know about it at all,
   but nothing is ideal around here.
 */
int size_reloc_info = sizeof(struct relocation_info);

static unsigned char octal[256];
#define isoctal(c)  octal[c]
static unsigned char toHex[256];

/* Local fatal error flag.  Used to bomb assembler in md_end after scanning input */
static int I860_errors;

static int insn_count;	/* Track insns assembled, as a word count */

struct i860_it {
    char    *error;
    uint32_t opcode;
    nlist_t *nlistp;
    expressionS exp;
    int pcrel;
    int reloc;
};
static struct i860_it the_insn;

#ifdef I860_DEBUG
static void print_insn(
    struct i860_it *insn);
#endif /* I860_DEBUG */

static int getExpression(
    char *str);
static char *expr_end;

/* Flags returned by i860_ip() */
#define INSERT_NOP	0x00000001

static
void
s_dual(
uintptr_t mode)
{
	dual_insn_mode = mode;
}

static
void
s_i860_align(
uintptr_t value)
{
    register unsigned int temp;
    register int32_t temp_fill;
    unsigned int i = 0;
    unsigned int bytes;
    char *toP, fill;

    bytes = temp = get_absolute_expression ();
#define MAX_ALIGNMENT (1 << 15)
    if ( temp > MAX_ALIGNMENT ) {
	as_warn("Alignment too large: %d. assumed.", temp = MAX_ALIGNMENT);
    }

    /*
     * For the i860, `.align (1<<n)' actually means `.align n'
     * so we have to convert it.
     */
    if (temp != 0) {
	for (i = 0; (temp & 1) == 0; temp >>= 1, ++i)
	    ;
    }
    if (temp != 1) {
	as_warn("Alignment not a power of 2");
    }
    temp = i;
    if (*input_line_pointer == ',') {
	input_line_pointer ++;
	temp_fill = get_absolute_expression ();
    } else {
	if ( frchain_now->frch_nsect == text_nsect )
    		temp_fill = OP_NOP;
	else
		temp_fill = 0;
    }
    if ( frchain_now->frch_nsect == text_nsect )	/* emit NOPs! */
    {	/* Grow the code frag as needed and dump nops into it. */
	if ( bytes & 3 )
		as_warn( "Instruction alignment must be a multiple of 4." );
	bytes &= ~3;
	/* This is really tacky, but works for a fixed width insn machine */
    	while ( bytes && ((insn_count * 4) % bytes) != 0 ) 
	{
		toP = frag_more(4);	/* Add an instruction */
		/* put out the opcode */
		md_insn_to_chars((unsigned char *)toP, temp_fill, 4);	/* Fill instruction */
		insn_count++;
	}
    	/* Clean up */
	demand_empty_rest_of_line();
	return;
    }
    /* Only make a frag if we HAVE to. . . */
    if (temp) {
	md_number_to_chars(&fill, temp_fill, 1);
	frag_align(temp, &fill, 1, 0);
    }
    /*
     * If this alignment is larger than any previous alignment then this
     * becomes the section's alignment.
     */
    if(frchain_now->frch_section.align < temp)
	frchain_now->frch_section.align = temp;
    demand_empty_rest_of_line();
    return;
}

static
void
s_i860_org(
uintptr_t value)
{
	register segT segment;
	expressionS exp;
	register int32_t temp_fill;
	register char *p;
	extern segT get_known_segmented_expression();

/*
 * Don't believe the documentation of BSD 4.2 AS.
 * There is no such thing as a sub-segment-relative origin.
 * Any absolute origin is given a warning, then assumed to be segment-relative.
 * Any segmented origin expression ("foo+42") had better be in the right
 * segment or the .org is ignored.
 *
 * BSD 4.2 AS warns if you try to .org backwards. We cannot because we
 * never know sub-segment sizes when we are reading code.
 * BSD will crash trying to emit -ve numbers of filler bytes in certain
 * .orgs. We don't crash, but see as-write for that code.
 */
	segment = get_known_segmented_expression(& exp);
	if ( *input_line_pointer == ',' ) {
		input_line_pointer ++;
		temp_fill = get_absolute_expression ();
	} else
		temp_fill = 0;

	if((segment != SEG_SECT ||
	    exp.X_add_symbol->sy_other != frchain_now->frch_nsect) &&
	    segment != SEG_ABSOLUTE)
	    as_warn("Illegal expression. current section assumed.");

	if ( exp.X_add_symbol != NULL )
		as_warn("Symbol relative .org may corrupt alignment.");
	else if ( exp.X_add_number & 3 )
	{
	    exp.X_add_number &= ~3;
	    as_warn(".org not on instruction boundry. Adjusted to \".org "
		    "%lld\"", exp.X_add_number);
	}
	if ( exp.X_add_symbol == NULL )
		insn_count = exp.X_add_number >> 2;
	p = frag_var (rs_org, 1, 1, (relax_substateT)0, exp . X_add_symbol,
exp . X_add_number, (char *)0);
	* p = temp_fill;

	demand_empty_rest_of_line();
}


/*
 * This function is called once, at assembler startup time.  This should
 * set up all the tables, etc that the MD part of the assembler needs
 */
void
md_begin(
void)
{
    const char *retval = NULL;
    uint32_t i;
    int j = 0;

    insn_count = 0;
    if ((op_hash = hash_new()) == NULL)
	as_fatal("Virtual memory exhausted");

    for (i = 0; i < NUMOPCODES; ++i) {
	if (~i860_opcodes[i].mask & i860_opcodes[i].match) {
	    printf("bad opcode - `%s %s'\n",
		i860_opcodes[i].name, i860_opcodes[i].args);
	    ++j;
	}
    }

    if (j)
	exit(1);

    for (i = 0; i < NUMOPCODES; ++i) {
      retval = hash_insert(op_hash, (char *)i860_opcodes[i].name,
			   (char *)&i860_opcodes[i]);
      if(retval && *retval) {
	  as_fatal("Internal Error:  Can't hash %s: %s",
	    i860_opcodes[i].name, retval);
      }
      while (!i860_opcodes[i].last)
	  ++i;
    }
    for (i = '0'; i < '8'; ++i)
	octal[i] = 1;
    for (i = '0'; i <= '9'; ++i)
	toHex[i] = i - '0';
    for (i = 'a'; i <= 'f'; ++i)
	toHex[i] = i + 10 - 'a';
    for (i = 'A'; i <= 'F'; ++i)
	toHex[i] = i + 10 - 'A';

    I860_errors = 0;
    return;
}

void
md_end(
void)
{
    if ( I860_errors )
    {
    	fprintf( stderr, "%d fatal %s encountered during assembly.\n", I860_errors,
		(I860_errors == 1 ? "error" : "errors") );
    	exit( 42 );	/* Fatal errors seen during assembly */
    }

    return;
}

void
md_assemble(
char *str)
{
    char *toP;
    int flags;

    assert(str);
    flags = i860_ip(str);
    if ( flags & INSERT_NOP )
    {
        toP = frag_more(4);
        /* put out the opcode */
        md_insn_to_chars((unsigned char *)toP, OP_NOP, 4);
	++insn_count;
    }
    toP = frag_more(4);
    /* put out the opcode */
    md_insn_to_chars((unsigned char *)toP, the_insn.opcode, 4);
    ++insn_count;

    /* put out the symbol-dependent stuff */
    if (the_insn.reloc != NO_RELOC) {
	fix_new(
	    frag_now,                           /* which frag */
	    (toP - frag_now->fr_literal), /* where */
	    4,                                  /* size */
	    the_insn.exp.X_add_symbol,
	    the_insn.exp.X_subtract_symbol,
	    the_insn.exp.X_add_number,
	    the_insn.pcrel, 0,
	    the_insn.reloc
	);
    }
}

static
int
i860_ip(
char *str)
{
    char *s;
    char *op;
    const char *args;
    char c;
    struct i860_opcode *insn;
    char *argsStart;
    char *s1;
    uint32_t   opcode;
    unsigned int mask;
    int this_insn_is_dual = 0;
    int adjustment;
    int	align_mask;
    int match = FALSE;
    int comma = 0;
    int flags = 0;
    static int expect_int_insn;	/* Tracking for fp/int insns in dual mode. */

    /* Advance s to end of opcode */
    for (s = str; islower(*s) || *s == '.' || isdigit(*s); ++s)
	;
    switch (*s) {

    case '\0':
	break;

    case ',':
	comma = 1;

	/*FALLTHROUGH*/

    case ' ':
    case '\t':
	*s++ = '\0';
	break;

    default:
	    as_warn("Unknown opcode: `%s'", str);
	    exit(1);
    }
    /* Code to sniff for 'd.' prefix here and flag for dual insn mode */
    op = str;
    if ( *op == 'd' && *(op + 1) == '.' )
    {
    	op += 2;
	this_insn_is_dual = 1;
    }
        
    if ((insn = (struct i860_opcode *) hash_find(op_hash, op)) == NULL) {
	as_warn("Unknown instruction or format: `%s'.", str);
	memset(&the_insn, '\0', sizeof(the_insn));	/* Patch in no-op to hold alignment */
	the_insn.reloc = NO_RELOC;
	the_insn.opcode = OP_NOP;
	++I860_errors;				/* Flag as fatal error */
	return flags;
    }
    if (comma) {
	*--s = ',';
    }
    argsStart = s;
    for (;;) {
	opcode = insn->match;
	memset(&the_insn, '\0', sizeof(the_insn));
	the_insn.reloc = NO_RELOC;

	/*
	 * Build the opcode, checking as we go to make
	 * sure that the operands match
	 */
	for (args = insn->args; ; ++args) {
	    align_mask = 0;
	    switch (*args) {

	    case '\0':  /* end of args */
		if (*s == '\0') {
		    match = TRUE;
		}
		break;

	    case '+':
	    case '(':   /* these must match exactly */
	    case ')':
	    case ',':
	    case ' ':
		if (*s++ == *args)
		    continue;
		break;
		
	    case 'C':   /* Control register */
		if (strncmp(s, "fir", 3) == 0) {
		    s += 3;
		    SET_RS2(opcode, 0);
		    continue;
		}
		if (strncmp(s, "psr", 3) == 0) {
		    s += 3;
		    SET_RS2(opcode, 1);
		    continue;
		}
		if (strncmp(s, "dirbase", 7) == 0) {
		    s += 7;
		    SET_RS2(opcode, 2);
		    continue;
		}
		if (strncmp(s, "db", 2) == 0) {
		    s += 2;
		    SET_RS2(opcode, 3);
		    continue;
		}
		if (strncmp(s, "fsr", 3) == 0) {
		    s += 3;
		    SET_RS2(opcode, 4);
		    continue;
		}
		if (strncmp(s, "epsr", 4) == 0) {
		    s += 4;
		    SET_RS2(opcode, 5);
		    continue;
		}
		break;

	    case '1':	/* next operand must be a register */
	    case '2':
	    case 'd':
		{
		    switch (c = *s++) {

		    case 'f':   /* frame pointer */
		        if (*s++ == 'p') {
			    mask = 3;	/* register fp is alias for r3 */
			    break;
			}
			goto error;

		    case 's':   /* global register */
			if (*s++ == 'p') {
			    mask = 2;	/* register sp is alias for r2 */
			    break;
			}
			goto error;

		    case 'r': /* any register */
		        if (!isdigit(c = *s++)) {
			    goto error;
			}
			if (isdigit(*s)) {
			    if ((c = 10 * (c - '0') + (*s++ - '0')) >= 32) {
				goto error;
			    }
			} else {
			    c -= '0';
			}
			mask= c;
			break;

		    default:
			goto error;
		    }
		    /*
		     * Got the register, now figure out where
		     * it goes in the opcode.
		     */
		    switch (*args) {

		    case '1':
		    	SET_RS1(opcode, mask);
			continue;

		    case '2':
		    	SET_RS2(opcode, mask);
			continue;

		    case 'd':
		    	SET_RD(opcode, mask);
			continue;
		    }
		}
		break;

	    case 'e':    /* next operand is a floating point register */
	    case 'f':
	    case 'g':
	    case 'E':
	    case 'F':
	    case 'G':
	    case 'H':
	        if (*s++ == 'f' && isdigit(*s)) {
		    mask = *s++;
		    if (isdigit(*s)) {
			mask = 10 * (mask - '0') + (*s++ - '0');
			if (mask >= 32) {
			    break;
			}
		    } else {
			mask -= '0';
		    }
		    if ( (*args == 'E' || *args == 'F' || *args == 'G') && (mask & 1) )
		    {
		    	as_warn( "f%d: Even register required.  Adjusted to f%d.",
				mask, mask & 0x1E );
			mask &= 0x1E;
		    }
		    else if ( *args == 'H' && (mask & 3) )
		    {
		    	as_warn( "f%d: Quad register required.  Adjusted to f%d.",
				mask, mask & 0x1C );
			mask &= 0x1C;
		    }
		    switch (*args) {

		    case 'e':
		    case 'E':
		    	SET_RS1(opcode, mask);
			continue;

		    case 'f':
		    case 'F':
		    	SET_RS2(opcode, mask);
			continue;

		    case 'g':
		    case 'G':
		    case 'H':
		    	SET_RD(opcode, mask);
			continue;
		    }
		}
		break;
	
	    case 'B':	/* 5 bit immediate unsigned constant */	
	  	(void)getExpression(s);
		s = expr_end;
		if ( the_insn.exp.X_seg != SEG_ABSOLUTE )
		{
			as_warn( "Constant expression expected" );
			++I860_errors;
		}
		if ( the_insn.exp.X_add_number < 0 || the_insn.exp.X_add_number > 31 )
		    as_warn( "Constant must be between 0 and 31. Modulo 32 applied." );
		SET_RS1(opcode, the_insn.exp.X_add_number); /* Takes const modulo 32 */
		continue;
		
	    case 'D':	/* immediate unsigned constant used in shift opcodes */	
	  	(void)getExpression(s);
		s = expr_end;
		if ( the_insn.exp.X_seg != SEG_ABSOLUTE )
		{
			as_warn( "Constant expression expected" );
			++I860_errors;
		}
		if ( the_insn.exp.X_add_number < 0 || the_insn.exp.X_add_number > 31 )
		    as_warn( "Constant must be between 0 and 31. Modulo 32 applied." );
		opcode |= (the_insn.exp.X_add_number & 0x1F);
		continue;

	    case 'i':       /* low 16 bits, byte aligned */
	        the_insn.reloc = I860_RELOC_LOW0;
		goto immediate;
		
	    case 'I':       /* high 16 bits */
	        the_insn.reloc = I860_RELOC_HIGH;
		goto immediate;

	    case 'j':       /* low 16 bits, short aligned */
	        the_insn.reloc = I860_RELOC_LOW1;
		align_mask = 1;
		goto immediate;

	    case 'k':       /* low 16 bits, int aligned */
	        the_insn.reloc = I860_RELOC_LOW2;
		align_mask = 3;
		goto immediate;

	    case 'l':       /* low 16 bits, double aligned */
	        the_insn.reloc = I860_RELOC_LOW3;
		align_mask = 7;
		goto immediate;

	    case 'm':       /* low 16 bits, quad aligned */
	        the_insn.reloc = I860_RELOC_LOW4;
		align_mask = 15;
		goto immediate;

	    case 'n':       /* low 16 bits, byte aligned, split field */
	        the_insn.reloc = I860_RELOC_SPLIT0;
		goto immediate;

	    case 'o':       /* low 16 bits, short aligned, split field */
	        the_insn.reloc = I860_RELOC_SPLIT1;
		align_mask = 1;
		goto immediate;

	    case 'p':       /* low 16 bits, int aligned, split field */
	        the_insn.reloc = I860_RELOC_SPLIT2;
		align_mask = 3;
		goto immediate;

	    case 'J':       /* High 16 bits, requiring adjustment */
	        the_insn.reloc = I860_RELOC_HIGHADJ;
		goto immediate;


	    case 'K':   /* 26 bit PC relative immediate */
	        the_insn.reloc = I860_RELOC_BRADDR;
		the_insn.pcrel = 1;
		goto immediate;

	    case 'L':   /* 16 bit PC relative split format immediate */
	        the_insn.reloc = I860_RELOC_SPLIT0;
		the_insn.pcrel = 1;
		goto immediate;
		/*FALLTHROUGH*/

	    immediate:
		if(*s==' ')
		  s++;
		adjustment = 0;
		if ( *s == 'h' && *(s + 1) == '%' )
		{
			adjustment = I860_RELOC_HIGH;
			if ( the_insn.reloc == I860_RELOC_LOW0 && the_insn.pcrel == 0 )
			{
				the_insn.reloc = I860_RELOC_HIGH;
			}
			else
				as_warn("Improper use of h%%.");
			s += 2;
		}
		else if ( *s == 'h' && *(s + 1) == 'a' && *(s + 2) == '%' )
		{
			adjustment = I860_RELOC_HIGHADJ;
			if ( the_insn.reloc == I860_RELOC_LOW0 && the_insn.pcrel == 0 )
			{
				the_insn.reloc = I860_RELOC_HIGHADJ;
			}
			else
				as_warn("Improper use of ha%%.");

			s += 3;
		}
		else if ( *s == 'l' &&  *(s + 1) == '%' )
		{	/* the_insn.reloc is correct as is. */
			adjustment = I860_RELOC_LOW0;
			s += 2;
		}
		/* Note that if the getExpression() fails, we will still have
		   created U entries in the symbol table for the 'symbols'
		   in the input string.  Try not to create U symbols for
		   registers, etc. */
		   
		/* This stuff checks to see if the expression ends
		   in '(', as in ld.l foo(r20). If it does, it
		   removes the '(' from the expression, and 
		   re-sets 's' to point to the right place */

		for(s1=s;*s1 && *s1!=','&& *s1!=')';s1++)
			;

		if( s1 != s && *s1 == '(' && s1[1] == 'r' && isdigit(s1[2])
		    && (s1[3]==')' || (isdigit(s1[3]) && s1[4] == ')')) ) {
				*s1='\0';
				(void)getExpression(s);
				*s1='(';
				s=s1;
		}
		else
		{
			(void)getExpression(s);
			s = expr_end;
		}
		/*
		 * If there is an adjustment, we assume the user knows what
		 * they are doing.  If no adjustment, we very carefully range 
		 * check for both signed and unsigned operations, to avoid 
		 * unpleasant suprises.  The checks are skipped for branch and
		 * call instructions, matching the behavior of the Intel assembler.
		 */
		if ( ! adjustment && *op != 'b' && *op != 'c' )
		{
		    if ( the_insn.exp.X_seg != SEG_ABSOLUTE )
		    {
			as_warn(
			  "Non-absolute expression requires h%%, l%%, or ha%% prefix."
			);
		    }
		    else
		    {
		    	if ( IS_LOGOP(opcode) )
			{
			    if ( ((unsigned)the_insn.exp.X_add_number) > 0xFFFF )
			    	as_warn("%lld is too big for 16 bit unsigned value!",
					the_insn.exp.X_add_number);
			}
			else
			{
			    if ( ((int)the_insn.exp.X_add_number) > 32767 ||
			    	 ((int)the_insn.exp.X_add_number) < -32768 )
				as_warn("%lld is out of range for 16 bit signed value!",
					the_insn.exp.X_add_number);
					
			    if ((align_mask & the_insn.exp.X_add_number) != 0)
			    	as_warn("Const offset 0x%x incorrectly aligned.",
					(unsigned int)the_insn.exp.X_add_number);
			}
		    }
		}
		continue;
		
	    default:
		abort();
	    }
	    break;
	}
	error:
	if (match == FALSE) {
	    /* args don't match */
	    if (!insn->last) {
		++insn;
		s = argsStart;
		continue;
	    } else {
		as_warn("Illegal operands (%s %s).", str, argsStart);
		++I860_errors;
		return flags;
	    }
	}
	break;
    }
    /* If the last insn was dual, check and make sure that this insn is integer insn */
    if ( expect_int_insn && (dual_insn_mode || this_insn_is_dual) )
    {
    	    if ( (opcode & OP_PREFIX_MASK) == PREFIX_FPU || opcode == OP_FNOP )
	    {
		as_warn( "Core half of prev dual insn pair missing." );
	    }
	    expect_int_insn = 0;
    }
    /* Check the insn format and fold in the dual mode bit if appropriate. */
    if ( dual_insn_mode || this_insn_is_dual )
    {
	    if ( (opcode & OP_PREFIX_MASK) == PREFIX_FPU || opcode == OP_FNOP )
	    {
	    	    if ( insn_count & 1 )	/* Odd insn, not on 64 bit bound! */
		    {
			as_warn( "Dual FP insn on odd addr." );
		    }
		    opcode |= DUAL_INSN_MODE_BIT;
		    expect_int_insn = 1;
	    }
	    else if ( this_insn_is_dual ) /* d. prefix on a non-FPU insn error */
	    {
		    as_warn("d. prefix not allowed for `%s'. (Ignored!)", op);
	    } 
    }
    else
    	    expect_int_insn = 0;	/* Single insn mode. */
    /* Check for correct alignment of const in branch to label + offset */
    if (    (the_insn.reloc == I860_RELOC_BRADDR
    	     || (the_insn.pcrel && the_insn.reloc == I860_RELOC_SPLIT0)
	    ) && (the_insn.exp.X_add_number & 3) )
    		as_warn( "Branch offset is not aligned to instruction boundry!" );
    the_insn.opcode = opcode;
    return flags;
}

static
int
getExpression(
char *str)
{
    char *save_in;
    segT seg;

    save_in = input_line_pointer;
    input_line_pointer = str;
    switch (seg = expression(&the_insn.exp)) {

    case SEG_ABSOLUTE:
    case SEG_SECT:
    case SEG_DIFFSECT:
    case SEG_UNKNOWN:
    case SEG_BIG:
    case SEG_NONE:
	break;

    default:
	the_insn.error = "bad segment";
	expr_end = input_line_pointer;
	input_line_pointer=save_in;
	return 1;
    }
    expr_end = input_line_pointer;
    input_line_pointer = save_in;
    return 0;
}


#define MAX_LITTLENUMS 6

/*
    This is identical to the md_atof in m68k.c.  I think this is right,
    but I'm not sure.

   Turn a string in input_line_pointer into a floating point constant of type
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
    char	*atof_ieee();

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
    /* The following two formats get reduced to doubles. */
    case 'x':
    case 'X':
    	type = 'd';
	prec = 4;
	break;

    case 'p':
    case 'P':
    	type = 'd';
	prec = 4;
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

/*
 * Write out big-endian.  Valid for data only in our I860 implementation.
 */
void
md_number_to_chars(
char *buf,
signed_expr_t val,
int n)
{

    switch(n) {

    case 4:
	*buf++ = val >> 24;
	*buf++ = val >> 16;
    case 2:
	*buf++ = val >> 8;
    case 1:
	*buf = val;
	break;

    default:
	abort();
    }
    return;
}

#ifdef BYTE_SWAP
/*
 * Write out little-endian.  Valid for instructions only in
 * our i860 implementation.
 */
static
void
md_insn_to_chars(
unsigned char *buf,
int32_t val,
int n)
{

    switch(n) {

    case 4:
	*buf++ = val;
	*buf++ = val >> 8;
	*buf++ = val >> 16;
	*buf++ = val >> 24;
	break;
    case 2:
	*buf++ = val;
	*buf++ = val >> 8;
	break;
    case 1:
	*buf = val;
	break;

    default:
	abort();
    }
    return;
}
#else /* !defined(BYTE_SWAP) */

static
void
md_insn_to_chars(
unsigned char *buf,
int32_t val,
int n)
{
	md_number_to_chars((char *)buf,val,n);
}
#endif /* BYTE_SWAP */

void
md_number_to_imm(
unsigned char *buf,
signed_expr_t val,
int n,
fixS *fixP,
int nsect)
{
    uint32_t opcode;

    if ( nsect == (int)text_nsect && (n % 4) != 0 )
    	as_warn("Immediate write of non-aligned data into text segment." );
	
    if (nsect != (int)text_nsect ||
	fixP->fx_r_type == NO_RELOC ||
	fixP->fx_r_type == I860_RELOC_VANILLA)
    {
	switch (n) {	/* Write out the data big-endian style. */
	case 1:
		*buf = val;
		break;
	case 2:
		*buf++ = (val>>8);
		*buf = val;
		break;
	case 4:
		*buf++ = (val>>24);
		*buf++ = (val>>16);
		*buf++ = (val>>8);
		*buf = val;
		break;
	default:
		abort();
	}
	return;
    }

    assert(n == 4);	/* Better be an instruction with relocation data.... */
    assert(fixP->fx_r_type < NO_RELOC && fixP->fx_r_type > I860_RELOC_VANILLA);
    /*
     * Here is where we do initial bit fiddling to load immediate 
     * values into the i860 bit fields.
     */
#ifdef BYTE_SWAP     
    /* Note that all of these insns are ultimately little-endian */
    /* Get the opcode from the buffer.  Less efficient, but more coherent... */
    opcode = (buf[3] << 24) | (buf[2] << 16) | (buf[1] << 8) | buf[0];
#else
    opcode = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
#endif  
    /* Apply the relocation value 'val' */
    switch (fixP->fx_r_type) { 
    	case I860_RELOC_PAIR:
		as_warn("questionable relocation type I860_RELOC_PAIR");
		break;
    	case I860_RELOC_HIGH:
		opcode &= ~0xFFFF;
		opcode |= ((val >> 16) & 0xFFFF);
		break;
    	case I860_RELOC_LOW0:
		opcode &= ~0xFFFF;
		opcode |= (val & 0xFFFF);
		break;
    	case I860_RELOC_LOW1:
		opcode &= 0xFFFF0001;
		opcode |= (val & 0xFFFE);	/* Bit 0 is an insn bit! */
		break;
    	case I860_RELOC_LOW2:
		opcode &= 0xFFFF0003;
		opcode |= (val & 0xFFFC);	/* Bits 0 and 1 are insn bits! */
		break;
    	case I860_RELOC_LOW3:
		opcode &= 0xFFFF0007;
		opcode |= (val & 0xFFF8);	/* Bits 0 thru 2 are insn bits! */
		break;
    	case I860_RELOC_LOW4:
		opcode &= 0xFFFF000F;
		opcode |= (val & 0xFFF0);	/* Bits 0 thru 3 are insn bits! */
		break;
    	case I860_RELOC_SPLIT0:
		opcode &= 0xFFE0F800;
		if ( fixP->fx_pcrel ) 		/* A 16 bit branch relative insn? */
			val >>= 2;		/* Convert to word address */
		opcode |= ((val & 0xF800) << 5) | (val & 0x7FF);
		break;
    	case I860_RELOC_SPLIT1:
		opcode &= 0xFFE0F801;		/* Again, bit 0 is an insn bit! */
		opcode |= ((val & 0xF800) << 5) | (val & 0x7FE);
		break;
    	case I860_RELOC_SPLIT2:
		opcode &= 0xFFE0F803;		/* Bits 0 and 1 are insn bits! */
		opcode |= ((val & 0xF800) << 5) | (val & 0x7FC);
		break;
    	case I860_RELOC_HIGHADJ:			/* Adjusted variant */
		opcode &= ~0xFFFF;
		/* If the low half would be negative, compensate by adding 1 to 
		 * high half.
		 */
		if ( (val & 0x8000) != 0 )
			val = (val >> 16) + 1;
		else
			val = (val >> 16);
		opcode |= (val & 0xFFFF);
		break;
    	case I860_RELOC_BRADDR:
		if ( fixP->fx_pcrel ) 		/* A 26 bit branch relative insn? */
			val >>= 2;		/* Convert to word address */
		opcode &= 0xFC000000;
		opcode |= (val & 0x03FFFFFF);
		break;
	
	default:
		as_warn("bad relocation type: 0x%02x", fixP->fx_r_type);
		break;
	}
#ifdef BYTE_SWAP
	buf[0] = opcode;
	buf[1] = opcode >> 8;
	buf[2] = opcode >> 16;
	buf[3] = opcode >> 24;
#else
	buf[3] = opcode;
	buf[2] = opcode >> 8;
	buf[1] = opcode >> 16;
	buf[0] = opcode >> 24;
#endif
	return;
}

/* should never be called for i860 */
void
md_convert_frag(
fragS *fragP)
{
    fprintf(stderr, "i860_convert_frag\n");
    abort();
}

/* should never be called for i860 */
int
md_estimate_size_before_relax(
fragS *fragP,
int nsect)
{
    fprintf(stderr, "i860_estimate_size_before_relax\n");
    abort();
    return 0;
}

#ifdef I860_DEBUG
/* for debugging only */
static void
print_insn(
struct i860_it *insn)
{
    char *Reloc[] = {
    "RELOC_8",
    "RELOC_16",
    "RELOC_32",
    "RELOC_DISP8",
    "RELOC_DISP16",
    "RELOC_DISP32",
    "RELOC_WDISP30",
    "RELOC_WDISP22",
    "RELOC_HI22",
    "RELOC_22",
    "RELOC_13",
    "RELOC_LO10",
    "RELOC_SFA_BASE",
    "RELOC_SFA_OFF13",
    "RELOC_BASE10",
    "RELOC_BASE13",
    "RELOC_BASE22",
    "RELOC_PC10",
    "RELOC_PC22",
    "RELOC_JMP_TBL",
    "RELOC_SEGOFF16",
    "RELOC_GLOB_DAT",
    "RELOC_JMP_SLOT",
    "RELOC_RELATIVE",
    "NO_RELOC"
    };

    if (insn->error) {
	fprintf(stderr, "ERROR: %s\n");
    }
    fprintf(stderr, "opcode=0x%08x\n", insn->opcode);
    fprintf(stderr, "reloc = %s\n", Reloc[insn->reloc]);
    fprintf(stderr, "exp =  {\n");
    fprintf(stderr, "\t\tX_add_symbol = %s\n",
	insn->exp.X_add_symbol ?
	(insn->exp.X_add_symbol->sy_name ? 
	insn->exp.X_add_symbol->sy_name : "???") : "0");
    fprintf(stderr, "\t\tX_sub_symbol = %s\n",
	insn->exp.X_subtract_symbol ?
	    (insn->exp.X_subtract_symbol->sy_name ? 
	        insn->exp.X_subtract_symbol->sy_name : "???") : "0");
    fprintf(stderr, "\t\tX_add_number = %d\n",
	insn->exp.X_add_number);
    fprintf(stderr, "}\n");
    return;
}
#endif /* I860_DEBUG */

int
md_parse_option(
char **argP,
int *cntP,
char ***vecP)
{
    return 1;
}

