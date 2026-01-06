/* hppa.c -- Assemble for the HP-PA
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


/*
   HP PA-RISC support was contributed by the Center for Software Science
   at the University of Utah.
*/

/* HP-PA support for Mach-O ... USV */

#include <stdio.h>
#include <stddef.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <mach-o/hppa/reloc.h>
#define HPPA_RELOC_12BRANCH (127) /* only used internal in here */

#include "obstack.h"
#include "hppa-opcode.h"
#include "as.h"
#include "frags.h"
#include "flonum.h"
#include "hash.h"
#include "md.h"
#include "symbols.h"
#include "hppa-aux.h"
#include "messages.h"
#include "stuff/hppa.h"
#include "sections.h"
#include "symbols.h"

/*
 * These are the default cputype and cpusubtype for the hppa architecture.
 */
const cpu_type_t md_cputype = CPU_TYPE_HPPA;
cpu_subtype_t md_cpusubtype = CPU_SUBTYPE_HPPA_ALL;

/* This is the byte sex for the hppa architecture */
const enum byte_sex md_target_byte_sex = BIG_ENDIAN_BYTE_SEX;

/* These characters start a comment anywhere on the line */
const char md_comment_chars[] = ";";

/* These characters only start a comment at the beginning of a line */
const char md_line_comment_chars[] = "#";

/*
 * These characters can be used to separate mantissa decimal digits from 
 * exponent decimal digits in floating point numbers.
 */
const char md_EXP_CHARS[] = "eE";

/*
 * The characters after a leading 0 that means this number is a floating point
 * constant as in 0f123.456 or 0d1.234E-12 (see md_EXP_CHARS above).
 */
const char md_FLT_CHARS[] = "dDfF";

/*
 * This is the machine dependent pseudo opcode table for this target machine.
 */
const pseudo_typeS md_pseudo_table[] =
{
    {0} /* end of table marker */
};


static int found_jbsr = 0;
static char *toP;

const relax_typeS md_relax_table[] = { {0} };

/* handle of the OPCODE hash table */
static struct hash_control *op_hash = NULL;

struct pa_it the_insn;   /* this structure is defined in pa-aux.h */

char *expr_end;

static void pa_ip(
    char *str);
static int parse_L_or_R(
    char *str);
static uint32_t parse_completer_with_cache_control_hint(
	char	**s,       /* Note : the function changes '*s' */
	int     option,    /* option = 0 for store instruction */
                       /* option = 1 for load and clear instruction */
	char    completer);/* 'c' or 'C' */
static uint32_t parse_cache_control_hint(
	char	**s,       /* Note : the function changes '*s' */
	int     option);   /* option = 0 for store instruction */
                       /* option = 1 for load and clear instruction */

/* This function is called once, at assembler startup time.  It should
   set up all the tables, etc. that the MD part of the assembler will need.  */
void
md_begin(
void)
{
	const char *retval = NULL;
	int lose = 0;
	register unsigned int i = 0;

	op_hash = hash_new();
	if (op_hash == NULL)
	as_fatal("Virtual memory exhausted");

	while (i < NUMOPCODES) {
		const char *name = pa_opcodes[i].name;
		retval = hash_insert(op_hash, (char *)name,
				     (char *)&pa_opcodes[i]);
		if(retval != NULL && *retval != '\0')  {
			as_fatal("Internal error: can't hash `%s': %s\n",
			pa_opcodes[i].name, retval);
			lose = 1;
		}
		++i;
	}

	if (lose)
		as_fatal ("Broken assembler.  No assembly attempted.");
}

void
md_end(
void)
{
	return;
}

void
md_assemble(
char *str)
{

	assert(str);
	pa_ip(str);
	if (!found_jbsr)
		toP = frag_more(4);
	else
		found_jbsr = 0;

#ifdef NeXT_MOD	/* mark sections containing instructions */
    /*
     * We are putting a machine instruction in this section so mark it as
     * containg some machine instructions.
     */
    frchain_now->frch_section.flags |= S_ATTR_SOME_INSTRUCTIONS;
#endif /* NeXT_MOD */

    /* put out the opcode */
    md_number_to_chars(toP, the_insn.opcode, 4);

    /* put out the symbol-dependent stuff */
    if (the_insn.reloc != NO_RELOC) {
	    fix_new(frag_now,			  /* which frag */
		    (toP - frag_now->fr_literal), /* where */
		    4,				  /* size */
		    the_insn.exp.X_add_symbol,
		    the_insn.exp.X_subtract_symbol,
		    the_insn.exp.X_add_number,	  /* offset */
		    the_insn.pcrel,
		    the_insn.pcrel_reloc,
		    the_insn.reloc);
    }
}

static
void
pa_ip(
char *str)
{
	char *s;
	const char *args;
	char c;
	uint32_t i;
	struct pa_opcode *insn;
	char *argsStart;
	uint32_t   opcode;
	int match = FALSE;
	int comma = 0;

	int reg,reg1,reg2,s2,s3;
	unsigned int im21,im14,im11,im5;
	int m,a,u,f;
	int cmpltr,nullif, flag;
	int sfu, cond;
	char *name;
	char *save_s, *p;
	short reference;

	reference = 0;

#ifdef PA_DEBUG
	fprintf(stderr,"STATEMENT: \"%s\"\n",str);
#endif
	for (s = str; isupper(*s) || islower(*s) || (*s >= '0' && *s <= '3'); ++s)
		;
	switch (*s) {

	case '\0':
		break;

	case ',':
		comma = 1;

	/*FALLTHROUGH*/

	case ' ':
		*s++ = '\0';
		break;

	default:
		as_bad("Unknown opcode: `%s'", str);
		exit(1);
	}

	save_s = str;

	while ( *save_s ) {
		if ( isupper(*save_s) )
			*save_s = tolower(*save_s);
		save_s++;
	}

	if ((insn = (struct pa_opcode *) hash_find(op_hash, str)) == NULL) {
		as_bad("Unknown opcode: `%s'", str);
		return;
	}
	if (comma) {
		*--s = ',';
	}
	argsStart = s;
	for (;;) {
		opcode = insn->match;
		memset(&the_insn, '\0', sizeof(the_insn));
		the_insn.reloc = NO_RELOC;    /* USV */

/*
* Build the opcode, checking as we go to make
* sure that the operands match
*/
		for (args = insn->args; ; ++args) {

			switch (*args) {

			case '\0':  /* end of args */
  				if (*s == '\0') {
					match = TRUE;
  				}
  				break;

			case '(':   /* these must match exactly */
			case ')':
			case ',':
			case ' ':
  				if (*s++ == *args)
					continue;
  				break;

			case 'b':   /* 5 bit register field at 10 */
  				reg = pa_parse_number(&s);
  				if ( reg < 32 && reg >= 0 ) {
					opcode |= reg << 21;
					continue;
  				}
  				break;
			case 'x':   /* 5 bit register field at 15 */
  				reg = pa_parse_number(&s);
  				if ( reg < 32 && reg >= 0 ) {
					opcode |= reg << 16;
					continue;
  				}
  				break;
			case 't':   /* 5 bit register field at 31 */
  				reg = pa_parse_number(&s);
  				if ( reg < 32 && reg >= 0 ) {
					opcode |= reg;
					continue;
  				}
  				break;
			case 'T':   /* 5 bit field length at 31 (encoded as 32-T) */
  /*
reg = pa_parse_number(&s);
   */
				getAbsoluteExpression(s);
  				if ( the_insn.exp.X_seg == SEG_ABSOLUTE ) {
					reg = the_insn.exp.X_add_number;
					if ( reg <= 32 && reg > 0 ) {
  						opcode |= 32 - reg;
  						s = expr_end;
  						continue;
					}
  				}
  				break;
			case '5':   /* 5 bit immediate at 15 */
				getAbsoluteExpression(s);
/** PJH: The following 2 calls to as_bad() might eventually **/
/**      want to end up as as_warn().  **/
				if (   the_insn.exp.X_add_number > 15 ) {
					as_bad("5 bit immediate: %lld"
					       " > 15. Set to 15",
						the_insn.exp.X_add_number);
					the_insn.exp.X_add_number = 15;
				}
				else if ( the_insn.exp.X_add_number < -16 ) {
						as_bad("5 bit immediate: "
						   "%lld < -16. Set to -16",
						    the_insn.exp.X_add_number);
						the_insn.exp.X_add_number = -16;
					}

				im5 = low_sign_unext(evaluateAbsolute(
						     the_insn.exp,0),5);
				opcode |= ( im5 << 16 );
				s = expr_end;
				continue;

			case 's':   /* 2 bit space identifier at 17 */
				s2 = pa_parse_number(&s);
				if ( s2 < 4 && s2 >= 0 ) {
					opcode |= s2 << 14;
					continue;
				}
				break;
			case 'S':   /* 3 bit space identifier at 18 */
				s3 = pa_parse_number(&s);
				if ( s3 < 8 && s3 >= 0 ) {
					s3 = dis_assemble_3(s3);
					opcode |= s3 << 13;
					continue;
				}
				break;
			case 'c':   /* indexed load completer. */
				i = m = u = 0;
				while ( *s == ',' && i < 2 ) {
					s++;
					if ( strncasecmp(s,"sm",2) == 0 ) {
						m = u = 1;
						s++;
						i++;
					}
					else if ( strncasecmp(s,"m",1) == 0 )
	  						m = 1;
						else if ( strncasecmp(s,"s",1) == 0 )
  								u = 1;
							else
  								as_bad("Unrecognized Indexed Load"
									"Completer...assuming 0");
					s++;
					i++;
  				}
  				if ( i > 2 )
					as_bad("Illegal Indexed Load Completer Syntax..."
						"extras ignored");
  				while ( *s == ' ' || *s == '\t' )
					s++;
  
  				opcode |= m << 5;
  				opcode |= u << 13;
  				continue;
			case 'C':   /* short load and store completer */
				m = a = 0;
				if ( *s == ',' ) {
					s++;
					if ( strncasecmp(s,"ma",2) == 0 ) {
						a = 0;
						m = 1;
					}
					else if ( strncasecmp(s,"mb",2) == 0 ) {
							m = a = 1;
						}
						else
							as_bad("Unrecognized Indexed Load Completer"
								"...assuming 0");
					s += 2;
				}
				while ( *s == ' ' || *s == '\t' )
					s++;
				opcode |= m << 5;
				opcode |= a << 13;
				continue;

			/* bug #41317 .... umeshv@NeXT.com
			 * Fri Jul 22 09:43:46 PDT 1994
			 *
			 * Modified to parse 'cache control hints'
			 *
			 * These parse ",cc" and encode "cc" in 2 bits at 20,
			 * where "cc" encoding is as given in Tables 5-8, 5-9.
			 * Refer to 'PA-RISC 1.1 Architecture and Instruction Set
			 * Reference Manual, Second Edition' for the tables.
			 */
			case 'Y':   /* Store Bytes Short completer */
						/* with cache control hints    */
			{
				uint32_t result = (unsigned long)0U;
				
				i = m = a = 0;
				while ( *s == ',' && i < 3 ) {
					s++;
					if ( strncasecmp(s,"m",1) == 0 )
						m = 1;
					else if ( strncasecmp(s,"b",1) == 0 &&
							  (strncasecmp((s+1),"c",1) != 0) )
							a = 0;
						else if ( strncasecmp(s,"e",1) == 0 )
								a = 1;
							else if ( strncmp(s,",",1) == 0 ) /* no completer */
								result |= parse_cache_control_hint(&s, 0);
							else if ( (strncasecmp(s,"c",1) == 0) ||
			          				(strncasecmp(s,"b",1) == 0) ) {/* just 1 completer */
								s--;
								result |= parse_cache_control_hint(&s, 0);
							}
							else
								as_bad("Unrecognized Store Bytes Short"
									"Completer with cache control hints"
									" ...assuming 0");
					if (result == (uint32_t)0U)
						s++;
					i++;
				}
/**		if ( i >= 2 ) **/
				if ( i > 3 )
					as_bad("Illegal Store Bytes Short Completer "
						"with cache control hints ...  extras ignored");
				while ( *s == ' ' || *s == '\t' ) /* skip to next operand */
					s++;
				opcode |= result;
				opcode |= m << 5;
				opcode |= a << 13;
				continue;
			}
			case '<':   /* non-negated compare/subtract conditions. */
				cmpltr = pa_parse_nonneg_cmpsub_cmpltr(&s);
				if ( cmpltr < 0 ) {
					as_bad("Unrecognized Compare/Subtract Condition: %c",*s);
					cmpltr = 0;
				}
				opcode |= cmpltr << 13;
				continue;
			case '?':   /* negated or non-negated cmp/sub conditions. */
					/* used only by ``comb'' and ``comib'' pseudo-ops */
				save_s = s;
				cmpltr = pa_parse_nonneg_cmpsub_cmpltr(&s);
				if ( cmpltr < 0 ) {
					s = save_s;
					cmpltr = pa_parse_neg_cmpsub_cmpltr(&s);
					if ( cmpltr < 0 ) {
						as_bad("Unrecognized Compare/Subtract Condition: %c"
							,*s);
						cmpltr = 0;
					}
					else {
						opcode |= 1 << 27; /* required opcode change to make
											COMIBT into a COMIBF or a
											COMBT into a COMBF or a
											ADDBT into a ADDBF or a
											ADDIBT into a ADDIBF */
					}
				}
				opcode |= cmpltr << 13;
				continue;
			case '!':   /* negated or non-negated add conditions. */
				/* used only by ``addb'' and ``addib'' pseudo-ops */
				save_s = s;
				cmpltr = pa_parse_nonneg_add_cmpltr(&s);
				if ( cmpltr < 0 ) {
					s = save_s;
					cmpltr = pa_parse_neg_add_cmpltr(&s);
					if ( cmpltr < 0 ) {
						as_bad("Unrecognized Compare/Subtract Condition: %c",
							*s);
						cmpltr = 0;
					}
					else {
						opcode |= 1 << 27; /* required opcode change to make
											COMIBT into a COMIBF or a
											COMBT into a COMBF or a
											ADDBT into a ADDBF or a
											ADDIBT into a ADDIBF */
					}
				}
				opcode |= cmpltr << 13;
				continue;
			case '-':   /* compare/subtract conditions */
				f = cmpltr = 0;
				save_s = s;
				if ( *s == ',' ) {
					cmpltr = pa_parse_nonneg_cmpsub_cmpltr(&s);
					if ( cmpltr < 0 ) {
						f = 1;
						s = save_s;
						cmpltr = pa_parse_neg_cmpsub_cmpltr(&s);
						if ( cmpltr < 0 ) {
							as_bad("Unrecognized Compare/Subtract Condition");
						}
					}
				}
				opcode |= cmpltr << 13;
				opcode |= f << 12;
				continue;
			case '+':   /* non-negated add conditions */
				flag = nullif = cmpltr = 0;
				if ( *s == ',' ) {
					s++;
					name = s;
					while ( *s != ',' && *s != ' ' && *s != '\t' )
						s += 1;
					c = *s;
					*s = 0x00;
					if ( strcmp(name,"=") == 0 ) {
						cmpltr = 1;
						}
						else if ( strcmp(name,"<") == 0 ) {
						cmpltr = 2;
						}
						else if ( strcmp(name,"<=") == 0 ) {
						cmpltr = 3;
						}
						else if ( strcasecmp(name,"nuv") == 0 ) {
						cmpltr = 4;
						}
						else if ( strcasecmp(name,"znv") == 0 ) {
						cmpltr = 5;
						}
						else if ( strcasecmp(name,"sv") == 0 ) {
						cmpltr = 6;
						}
						else if ( strcasecmp(name,"od") == 0 ) {
						cmpltr = 7;
						}
						else if ( strcasecmp(name,"n") == 0 ) {
						nullif = 1;
						}
						else if ( strcasecmp(name,"tr") == 0 ) {
						cmpltr = 0;
						flag   = 1;
						}
						else if ( strcasecmp(name,"<>") == 0 ) {
						flag = cmpltr = 1;
						}
						else if ( strcasecmp(name,">=") == 0 ) {
						cmpltr = 2;
						flag   = 1;
						}
						else if ( strcasecmp(name,">") == 0 ) {
						cmpltr = 3;
						flag   = 1;
						}
						else if ( strcasecmp(name,"uv") == 0 ) {
						cmpltr = 4;
						flag   = 1;
						}
						else if ( strcasecmp(name,"vnz") == 0 ) {
						cmpltr = 5;
						flag   = 1;
						}
						else if ( strcasecmp(name,"nsv") == 0 ) {
						cmpltr = 6;
						flag   = 1;
						}
						else if ( strcasecmp(name,"ev") == 0 ) {
						cmpltr = 7;
						flag   = 1;
						}
						else
						as_bad("Unrecognized Add Condition: %s",name);
					*s = c;
				}
				nullif = pa_parse_nullif(&s);
				opcode |= nullif << 1;
				opcode |= cmpltr << 13;
				opcode |= flag << 12;
				continue;		
			case '&':   /* logical instruction conditions */
				f = cmpltr = 0;
				if ( *s == ',' ) {
					s++;
					name = s;
					while ( *s != ',' && *s != ' ' && *s != '\t' )
						s += 1;
					c = *s;
					*s = 0x00;
					if ( strcmp(name,"=") == 0 ) {
						cmpltr = 1;
						}
						else if ( strcmp(name,"<") == 0 ) {
						cmpltr = 2;
						}
						else if ( strcmp(name,"<=") == 0 ) {
						cmpltr = 3;
						}
						else if ( strcasecmp(name,"od") == 0 ) {
						cmpltr = 7;
						}
						else if ( strcasecmp(name,"tr") == 0 ) {
						cmpltr = 0;
						f = 1;
						}
						else if ( strcmp(name,"<>") == 0 ) {
						f = cmpltr = 1;
						}
						else if ( strcmp(name,">=") == 0 ) {
						cmpltr = 2;
						f = 1;
						}
						else if ( strcmp(name,">") == 0 ) {
						cmpltr = 3;
						f = 1;
						}
						else if ( strcasecmp(name,"ev") == 0 ) {
						cmpltr = 7;
						f = 1;
						}
						else
						as_bad("Unrecognized Logical Instruction Condition:"
							" %s",name);
					*s = c;
				}
				opcode |= cmpltr << 13;
				opcode |= f << 12;		
				continue;
			case 'U':   /* unit instruction conditions */
				cmpltr = 0;
				f = 0;
				if ( *s == ',' ) {
					s++;
					if ( strncasecmp(s,"sbz",3) == 0 ) {
						cmpltr = 2;
						s += 3;
						}
						else if ( strncasecmp(s,"shz",3) == 0 ) {
						cmpltr = 3;
						s += 3;
						}
						else if ( strncasecmp(s,"sdc",3) == 0 ) {
						cmpltr = 4;
						s += 3;
						}
						else if ( strncasecmp(s,"sbc",3) == 0 ) {
						cmpltr = 6;
						s += 3;
						}
						else if ( strncasecmp(s,"shc",3) == 0 ) {
						cmpltr = 7;
						s += 3;
						}
						else if ( strncasecmp(s,"tr",2) == 0 ) {
						cmpltr = 0;
						f = 1;
						s += 2;
						}
						else if ( strncasecmp(s,"nbz",3) == 0 ) {
						cmpltr = 2;
						f = 1;
						s += 3;
						}
						else if ( strncasecmp(s,"nhz",3) == 0 ) {
						cmpltr = 3;
						f = 1;
						s += 3;
						}
						else if ( strncasecmp(s,"ndc",3) == 0 ) {
						cmpltr = 4;
						f = 1;
						s += 3;
						}
						else if ( strncasecmp(s,"nbc",3) == 0 ) {
						cmpltr = 6;
						f = 1;
						s += 3;
						}
						else if ( strncasecmp(s,"nhc",3) == 0 ) {
						cmpltr = 7;
						f = 1;
						s += 3;
						}
						else
						as_bad("Unrecognized Logical Instruction Condition:"
							" %c",*s);
				}
				opcode |= cmpltr << 13;
				opcode |= f << 12;		
				continue;
			case '>':   /* shift/extract/deposit conditions. */
				cmpltr = 0;
				if ( *s == ',' ) {
					s++;
					name = s;
					while ( *s != ',' && *s != ' ' && *s != '\t' )
						s += 1;
					c = *s;
					*s = 0x00;
					if ( strcmp(name,"=") == 0 ) {
						cmpltr = 1;
						}
						else if ( strcmp(name,"<") == 0 ) {
						cmpltr = 2;
						}
						else if ( strcasecmp(name,"od") == 0 ) {
						cmpltr = 3;
						}
						else if ( strcasecmp(name,"tr") == 0 ) {
						cmpltr = 4;
						}
						else if ( strcmp(name,"<>") == 0 ) {
						cmpltr = 5;
						}
						else if ( strcmp(name,">=") == 0 ) {
						cmpltr = 6;
						}
						else if ( strcasecmp(name,"ev") == 0 ) {
						cmpltr = 7;
						}
						else
						as_bad("Unrecognized Shift/Extract/Deposit"
							"Condition: %s",name);
					*s = c;
				}
				opcode |= cmpltr << 13;
				continue;
			case '~':   /* bvb,bb conditions */
				cmpltr = 0;
				if ( *s == ',' ) {
					s++;
					if ( strncmp(s,"<",1) == 0 ) {
						cmpltr = 2;
						s++;
						}
						else if ( strncmp(s,">=",2) == 0 ) {
						cmpltr = 6;
						s += 2;
						}
						else
						as_bad("Unrecognized Bit Branch Condition: %c",*s);
				}
				opcode |= cmpltr << 13;
				continue;
			case 'V':   /* 5  bit immediate at 31 */
				getExpression(s);
				im5 = low_sign_unext(evaluateAbsolute(
						     the_insn.exp,0),5);
				opcode |= im5;
				s = expr_end;
				continue;
			case 'r':   /* 5  bit immediate at 31 */
						/* (unsigned value for the break instruction) */
				getExpression(s);
				im5 = evaluateAbsolute(the_insn.exp,0);
				if ( im5 > 31 ) {
					as_bad("Operand out of range. Was: %d. Should be"
						"[0..31]. Assuming %d.\n",im5,im5&0x1f);
					im5 = im5 & 0x1f;
				}
				opcode |= im5;
				s = expr_end;
				continue;
			case 'R':   /* 5  bit immediate at 15 */
/* (unsigned value for the ssm and rsm instruction) */
				getExpression(s);
				im5 = evaluateAbsolute(the_insn.exp,0);
				if ( im5 > 31 ) {
					as_bad("Operand out of range. Was: %d. Should be"
						"[0..31]. Assuming %d.\n",im5,im5&0x1f);
					im5 = im5 & 0x1f;
				}
				opcode |= im5 << 16;
				s = expr_end;
				continue;
			case 'i':   /* 11 bit immediate at 31 */
				getExpression(s);
				if ( the_insn.exp.X_seg == SEG_ABSOLUTE ) {
					im11 = low_sign_unext(evaluateAbsolute(
							the_insn.exp,0),11);
					opcode |= im11;
				}
				else {
					the_insn.code = 'i';
				}
				s = expr_end;
				continue;
			case 'j':   /* 14 bit immediate at 31 --- LO14 */
			{
				int field_selector = parse_L_or_R(s);
				switch (field_selector) {
				case 2:	/* found the field selector R`*/
				case 1:	/* found the field selector L`*/
					s += 2;  /* eat up L` or R` */
				case 0: /* not found */
					getExpression(s);
					break;
				default:
					as_bad("Bad field selector. Was: %.2s. Should be either L` or R`\n",s);
					break;
				}
				if ( the_insn.exp.X_seg == SEG_ABSOLUTE ) {
					im14 = low_sign_unext(
evaluateAbsolute(the_insn.exp,field_selector), 14);

/* I donot think the mask is necessary here  low_sign_unext() takes */
/* care of putting only 14 bits in im14 ! ...       090993 ... USV  */
/*					if (field_selector)
						opcode |= (im14 & 0x7ff);
					else
*/
					opcode |= im14;
				}
				else {
					the_insn.reloc = HPPA_RELOC_LO14;
					the_insn.code = 'j';
				}
				s = expr_end;
				continue;
			}  
			case 'z':	/* 17 bit branch displacement (non-pc-relative) */
				/* for be, ble --- BR17*/
				/* bl, ble  in absence of L` or R` can have */
				/* a 17 bit immmidiate number */
			{
				uint32_t w, w1, w2;
				int field_selector = parse_L_or_R(s);
				switch (field_selector) {
				case 2:	/* found the field selector R`*/
				case 1:	/* found the field selector L`*/
					s += 2;  /* eat up L` or R` */
				case 0: /* not found */
					getExpression(s);
					break;
				default:
					as_bad("Bad field selector. Was: %.2s." "Should be either L` or R`\n",s);
					break;
				}
				if ( the_insn.exp.X_seg == SEG_ABSOLUTE ) {
					im14 = sign_unext(
						evaluateAbsolute(the_insn.exp,field_selector),
						17);
					dis_assemble_17(im14>>2,&w1,&w2,&w);
					opcode |= ( ( w2 << 2 ) | ( w1 << 16 ) | w );
				}
				else {
					the_insn.reloc = HPPA_RELOC_BR17;
					the_insn.code = 'z';
				}
				s = expr_end;
				continue;
			}
			case 'k':   /* 21 bit immediate at 31 --- HI21 */
			{
				int field_selector = parse_L_or_R(s);
				switch (field_selector) {
				case 2:	/* found the field selector R`*/
				case 1:	/* found the field selector L`*/
					s += 2;  /* eat up L` or R` */
				case 0: /* not found */
					getExpression(s);
					break;
				default:
					as_bad("Bad field selector. Was: %.2s." "Should be either L` or R`\n",s);
					break;
				}
				if ( the_insn.exp.X_seg == SEG_ABSOLUTE ) {
					im21 = dis_assemble_21(
(evaluateAbsolute(the_insn.exp,field_selector) >> 11));
					opcode |=  im21 ;
				}
				else {
					the_insn.reloc = HPPA_RELOC_HI21;
					the_insn.code = 'k';
				}
				s = expr_end;
				continue;
  			}
			case 'n':   /* nullification for branch instructions */
				nullif = pa_parse_nullif(&s);
				opcode |= nullif << 1;
				continue;		
			case 'w':   /* 12 bit branch displacement */
				getExpression(s);
				the_insn.pcrel_reloc = 0;
				the_insn.pcrel = 1;
				if ( the_insn.exp.X_add_symbol ) {
				    if ( strcmp(
the_insn.exp.X_add_symbol->sy_name,"L0\001") == 0 ) {
				        uint32_t w1,w,result;
					result = sign_unext( (the_insn.exp.X_add_number
							- 8) >> 2,
							12);
					dis_assemble_12(result,&w1,&w);
					opcode |= ( ( w1 << 2 ) | w );
				    }
				    else {
/* this has to be wrong -- dont know what is right! */
/*  the_insn.reloc = R_PCREL_CALL; */
				        the_insn.reloc = HPPA_RELOC_12BRANCH;
						the_insn.code = 'w';
				    }
				}
				else {
				    uint32_t w1,w,result;
				    result = sign_unext( the_insn.exp.X_add_number >> 
					2,12);
				    dis_assemble_12(result,&w1,&w);
				    opcode |= ( ( w1 << 2 ) | w );
				}
				s = expr_end;
				continue;
			case 'W':   /* 17 bit branch displacement --- BL17 */
				getExpression(s);
				/*
				 * The NeXT linker has the ability to scatter
				 * blocks of sections between labels.  This
				 * requires that brances to labels that survive
				 * to the link phase must be able to be
				 * relocated.
				 */
				if(the_insn.exp.X_add_symbol != NULL &&
				   (the_insn.exp.X_add_symbol->sy_name[0] != 'L'
				    || flagseen['L']))
				    the_insn.pcrel_reloc = 1;
				else
				    the_insn.pcrel_reloc = 0;
				the_insn.pcrel = 1;
	      			if ( the_insn.exp.X_add_symbol ) {
					if ( strcmp(the_insn.exp.X_add_symbol->sy_name,"L0\001") == 0 ) {
						uint32_t w2,w1,w,result;

						result = sign_unext( 
							(the_insn.exp.X_add_number - 8) >> 2,17);
						dis_assemble_17(result,&w1,&w2,&w);
						opcode |= ( ( w2 << 2 ) | ( w1 << 16 ) | w );
					}
					else {
				 		if ( (the_insn.reloc == HPPA_RELOC_JBSR) &&
	       					 (the_insn.exp.X_add_symbol->sy_name[0] != 'L') )
							as_fatal("Stub label used in a JBSR must be "
								"non-relocatable");
						the_insn.reloc = HPPA_RELOC_BL17;
						the_insn.code = 'W';
					}
				}
				else {
					uint32_t w2,w1,w,result;

					result = sign_unext( the_insn.exp.X_add_number >> 2,17);
					dis_assemble_17(result,&w1,&w2,&w);
					opcode |= ( ( w2 << 2 ) | ( w1 << 16 ) | w );
				}
				s = expr_end;
				continue;
			case '@':   /* 17 bit branch displacement --- JBSR */
				/*
				 * If we are assembling -dynamic then if the
				 * symbol name before the ',' has not yet been
				 * seen it will be marked as a non-lazy
				 * reference.
				 */
				if(flagseen[(int)'k'] == TRUE){
				    p = strchr(s, ',');
				    if(p != NULL)
					*p = '\0';
				    if(symbol_find(s) == NULL)
					reference =
					    REFERENCE_FLAG_UNDEFINED_LAZY;
				    else
					reference =
					    REFERENCE_FLAG_UNDEFINED_NON_LAZY;
				    if(p != NULL)
					*p = ',';
				}
				getExpression(s);

/*
 * assumption here is this will only be used in case of jbsr
 * in which case the format is 
 *		jbsr,n symbol,register,label
 * and a relocation entry for symbol needs to be created
 */
 
				the_insn.pcrel = 0;
				the_insn.pcrel_reloc = 1;
				the_insn.reloc = HPPA_RELOC_JBSR;
				the_insn.code = '@';
				s = expr_end;
 /*
  * The code to hook a frag in the chain should be here.
  * Then set a flag saying that the next 'W' should not create a relocation
  * entry. The way 'jbsr' is expected to work is the label will always be
  * local!
  * This flag should be reset in 'W'.
  */
				found_jbsr = 1;
				toP = frag_more(4);
				fix_new(frag_now,
					(toP - frag_now->fr_literal),
					4,
					the_insn.exp.X_add_symbol,
					the_insn.exp.X_subtract_symbol,
					the_insn.exp.X_add_number,
					the_insn.pcrel,
					the_insn.pcrel_reloc,
					the_insn.reloc);
				if(flagseen[(int)'k'] == TRUE)
				    the_insn.exp.X_add_symbol->sy_desc |=
					reference;

				continue;
			case 'B':   /* either "s,b" or "b" where b & s are defined above */
				reg1 = pa_parse_number(&s);
				if ( *s == ',' ) {
					s++;
					reg2 = pa_parse_number(&s);
				}
				else {
					reg2 = reg1;
					reg1 = 0;
				}
				if ( reg1 < 4 && reg1 >= 0 ) {
					opcode |= reg1 << 14;
					opcode |= reg2 << 21;
					continue;
				}
				break;
			case 'p':   /* 5 bit shift count at 26 (to support SHD instr.) */
						/* value is encoded in instr. as 31-p where p is   */
						/* the value scanned here */
				getExpression(s);
				if ( the_insn.exp.X_seg == SEG_ABSOLUTE ) {
					opcode |= ( ( (31 - the_insn.exp.X_add_number) & 0x1f ) << 5 );
				}
				s = expr_end;
				continue;
			case 'P':   /* 5-bit bit position at 26 */
				getExpression(s);
				if ( the_insn.exp.X_seg == SEG_ABSOLUTE ) {
					opcode |= ( the_insn.exp.X_add_number & 0x1f ) << 5;
				}
				s = expr_end;
				continue;
			case 'Q':   /* 5  bit immediate at 10 */
						/* (unsigned bit position value for the bb instruction) */
				getExpression(s);
				im5 = evaluateAbsolute(the_insn.exp,0);
				if ( im5 > 31 ) {
					as_bad("Operand out of range. Was: %d. Should be"
						"[0..31]. Assuming %d.\n",im5,im5&0x1f);
					im5 = im5 & 0x1f;
				}
				opcode |= im5 << 21;
				s = expr_end;
				continue;
			case 'A':   /* 13 bit immediate at 18 (to support BREAK instr.) */
				getAbsoluteExpression(s);
				if ( the_insn.exp.X_seg == SEG_ABSOLUTE )
				opcode |= (the_insn.exp.X_add_number & 0x1fff) << 13;
  				s = expr_end;
  				continue;
			case 'Z':   /* System Control Completer(for LDA, LHA, etc.) */
  				if ( *s == ',' && ( *(s+1) == 'm' || *(s+1) == 'M' ) ) {
					m = 1;
					s += 2;
  				}
  				else
					m = 0;
  
  				opcode |= m << 5;
  				while ( *s == ' ' || *s == '\t' ) /* skip to next operand */
					s++;
  
  				continue;
			case 'D':   /* 26 bit immediate at 31 (to support DIAG instr.) */
  						/* the action (and interpretation of this operand is
		 					implementation dependent) */
  				getExpression(s);
  				if ( the_insn.exp.X_seg == SEG_ABSOLUTE ) {
					opcode |= ( (evaluateAbsolute(the_insn.exp,0) & 0x1ffffff) << 1 );
  				}
  				else
					as_bad("Illegal DIAG operand");
  					s = expr_end;
  				continue;
			case 'f':   /* 3 bit Special Function Unit (SFU) identifier at 25 */
  				sfu = pa_parse_number(&s);
  				if ( (sfu > 7) || (sfu < 0) )
					as_bad("Illegal SFU identifier: %02x", sfu);
  				opcode |= (sfu & 7) << 6;
  				continue;
			case 'O':   /* 20 bit SFU op. split between 15 bits at 20 and 5 bits at 31 */
  				getExpression(s);
  				s = expr_end;
  				continue;
			case 'o':   /* 15 bit Special Function Unit operation at 20 */
  				getExpression(s);
  				s = expr_end;
  				continue;
			case '2':   /* 22 bit SFU op. split between 17 bits at 20
			   				and 5 bits at 31 */
  				getExpression(s);
  				s = expr_end;
  				continue;
			case '1':   /* 15 bit SFU op. split between 10 bits at 20
			   				and 5 bits at 31 */
  				getExpression(s);
  				s = expr_end;
  				continue;
			case '0':   /* 10 bit SFU op. split between 5 bits at 20
			   				and 5 bits at 31 */
  				getExpression(s);
  				s = expr_end;
  				continue;
			case 'u':   /* 3 bit coprocessor unit identifier at 25 */
  				getExpression(s);
  				s = expr_end;
  				continue;
			case 'F':   /* Source FP Operand Format Completer (2 bits at 20) */
  				f = pa_parse_fp_format(&s);
  				opcode |= (int)f << 11;
  				the_insn.fpof1 = f;
  				continue;
			case 'G':   /* Destination FP Operand Format Completer (2 bits at 18) */
  				s--;    /* need to pass the previous comma to pa_parse_fp_format */
  				f = pa_parse_fp_format(&s);
  				opcode |= (int)f << 13;
  				the_insn.fpof2 = f;
  				continue;
			case 'M':   /* FP Compare Conditions (encoded as 5 bits at 31) */
  				cond = pa_parse_fp_cmp_cond(&s);
  				opcode |= cond;
  				continue;

			case 'v':   /* a 't' type extended to handle L/R register halves. */
  				{
					struct pa_89_fp_reg_struct result;

					pa_89_parse_number(&s,&result);
					if ( result.number_part < 32 && result.number_part >= 0 ) {
  						opcode |= (result.number_part & 0x1f);

						/* 0x30 opcodes are FP arithmetic operation opcodes */
						/* load/store FP opcodes do not get converted to 0x38 */
						/* opcodes like the 0x30 opcodes do */
						if ( need_89_opcode(&the_insn,&result) ) {
							if ( (opcode & 0xfc000000) == 0x30000000 ) {
								opcode |= (result.L_R_select & 1) << 6;
								opcode |= 1 << 27;
							}
							else {
								opcode |= (result.L_R_select & 1) << 6;
							}
						}
						continue;
					}
				}
				break;
			case 'E':   /* a 'b' type extended to handle L/R register halves. */
				{
					struct pa_89_fp_reg_struct result;

					pa_89_parse_number(&s,&result);
					if ( result.number_part < 32 && result.number_part >= 0 ) {
						opcode |= (result.number_part & 0x1f) << 21;
						if ( need_89_opcode(&the_insn,&result) ) {
							opcode |= (result.L_R_select & 1) << 7;
							opcode |= 1 << 27;
						}
						continue;
					}
				}
				break;

			case 'X':   /* an 'x' type extended to handle L/R register halves. */
				{
					struct pa_89_fp_reg_struct result;


					pa_89_parse_number(&s,&result);
					if ( result.number_part < 32 && result.number_part >= 0 ) {
						opcode |= (result.number_part & 0x1f) << 16;
						if ( need_89_opcode(&the_insn,&result) ) {
							opcode |= (result.L_R_select & 1) << 12;
							opcode |= 1 << 27;
						}
						continue;
					}
				}
				break;

			case '4':   /* 5 bit register field at 10
						(used in 'fmpyadd' and 'fmpysub') */
				{
					struct pa_89_fp_reg_struct result;
					int status;

					status = pa_89_parse_number(&s,&result);
					if ( result.number_part < 32 && result.number_part >= 0 ) {
						if ( the_insn.fpof1 == SGL ) {
							result.number_part &= 0xF;
							result.number_part |= (result.L_R_select & 1) << 4;
						}
						opcode |= result.number_part << 21;
						continue;
					}
				}
				break;

			case '6':   /* 5 bit register field at 15
						(used in 'fmpyadd' and 'fmpysub') */
				{
					struct pa_89_fp_reg_struct result;
					int status;

					status = pa_89_parse_number(&s,&result);
					if ( result.number_part < 32 && result.number_part >= 0 ) {
						if ( the_insn.fpof1 == SGL ) {
							result.number_part &= 0xF;
							result.number_part |= (result.L_R_select & 1) << 4;
						}
						opcode |= result.number_part << 16;
						continue;
					}
				}
				break;

			case '7':   /* 5 bit register field at 31
						(used in 'fmpyadd' and 'fmpysub') */
				{
					struct pa_89_fp_reg_struct result;
					int status;

					status = pa_89_parse_number(&s,&result);
					if ( result.number_part < 32 && result.number_part >= 0 ) {
						if ( the_insn.fpof1 == SGL ) {
							result.number_part &= 0xF;
							result.number_part |= (result.L_R_select & 1) << 4;
						}
						opcode |= result.number_part;
						continue;
					}
				}
				break;

			case '8':   /* 5 bit register field at 20
						(used in 'fmpyadd' and 'fmpysub') */
				{
					struct pa_89_fp_reg_struct result;
					int status;

					status = pa_89_parse_number(&s,&result);
					if ( result.number_part < 32 && result.number_part >= 0 ) {
						if ( the_insn.fpof1 == SGL ) {
							result.number_part &= 0xF;
							result.number_part |= (result.L_R_select & 1) << 4;
						}
						opcode |= result.number_part << 11;
						continue;
					}
				}
				break;

			case '9':   /* 5 bit register field at 25
						(used in 'fmpyadd' and 'fmpysub') */
				{
					struct pa_89_fp_reg_struct result;
					int status;

					status = pa_89_parse_number(&s,&result);
					if ( result.number_part < 32 && result.number_part >= 0 ) {
						if ( the_insn.fpof1 == SGL ) {
							result.number_part &= 0xF;
							result.number_part |= (result.L_R_select & 1) << 4;
						}
						opcode |= result.number_part << 6;
						continue;
					}
				}
				break;

			case 'H':  /* Floating Point Operand Format at 26 for       */
						/* 'fmpyadd' and 'fmpysub' (very similar to 'F') */
						/* bits are switched from other FP Operand       */
						/* formats. 1=SGL, 1=<none>, 0=DBL               */
				f = pa_parse_fp_format(&s);
				switch (f) {
				case SGL:
					opcode |= 0x20;
				case DBL:
					the_insn.fpof1 = f;
					continue;

				case QUAD:
				case ILLEGAL_FMT:
				default:
					as_bad("Illegal Floating Point Operand Format for"
						"this instruction: '%s'",s);
				}
				break;

			case 'y' :  /* nullify at 26 */
				nullif = pa_parse_nullif(&s);
				opcode |= nullif << 5;
				continue;
						
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

			case 'l' :  /* Store Instruction Cache Control Hint */
						/* Short displacement load and store completers */
				opcode |= parse_completer_with_cache_control_hint(&s, 0, 'C');
				continue;

			case 'L' :  /* Load and Clear Word Cache Control Hint */
						/* Indexed load completers  */
				opcode |= parse_completer_with_cache_control_hint(&s, 1, 'c');
				continue;

			case '3' :  /* Store Instruction Cache Control Hint */
						/* Indexed load completers */
				opcode |= parse_completer_with_cache_control_hint(&s, 0, 'c');
				continue;

			case 'a' :  /* Load and Clear Word Cache Control Hint */
						/* Short displacement load and store completers */
				opcode |= parse_completer_with_cache_control_hint(&s, 1, 'C');
				continue;

			default:
				abort();
			}
			break;
		}

		if (match == FALSE)
		{
		/* Args don't match.  */
			if (&insn[1] - pa_opcodes < (ptrdiff_t)NUMOPCODES
				&& !strcmp(insn->name, insn[1].name))
			{
				++insn;
				s = argsStart;
				continue;
			}
			else
			{
				as_bad("Illegal operands");
				return;
			}
		}
		break;
	}

	the_insn.opcode = opcode;
	return;
}    /* end pa_ip() */

/*
    This is identical to the md_atof in m68k.c.  I think this is right,
    but I'm not sure.

   Turn a string in input_line_pointer into a floating point constant of type
   type, and store the appropriate bytes in *litP.  The number of LITTLENUMS
   emitted is stored in *sizeP .  An error message is returned, or NULL on OK.
 */

/* Equal to MAX_PRECISION in atof-ieee.c */
#define MAX_LITTLENUMS 6

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

/*
 * Write out big-endian.
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

void
md_number_to_imm(
unsigned char *buf,
signed_expr_t val,
int n,
fixS *fixP,
int nsect)
{
	uint32_t w1,w2,w;
	unsigned new_val = 0;
	uint32_t left21, right14;
	
	if(fixP->fx_r_type == NO_RELOC ||
	   fixP->fx_r_type == HPPA_RELOC_VANILLA){
	    switch(n){
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


	calc_hppa_HILO(val - fixP->fx_offset, fixP->fx_offset,
		       &left21, &right14);

	switch (fixP->fx_r_type) {
    default:
		break;
/*     case 'j': */
    case HPPA_RELOC_LO14 :
 		w = low_sign_unext(right14, 14); 
		goto fixit;

/*     case 'k': */
    case HPPA_RELOC_HI21 :
		w = dis_assemble_21((left21>>11));
fixit:
      /* There is no guarantee that buf is word-aligned,	*/
      /* so the adjustment must be done the hard way.		*/

		new_val  = (*buf & 0xff) << 24;
		new_val |= (*(buf+1) & 0xff) << 16;
		new_val |= (*(buf+2) & 0xff) << 8;
		new_val |= (*(buf+3) & 0xff);
		new_val |= w;	/* Now, make the adjustment */
		md_number_to_chars((char *)buf,new_val,4);
		break;

/*     case 'W': */
	case HPPA_RELOC_BL17 :
		if ( !fixP->fx_addsy ) {
			val -= 4;	/* PA adjustment: a 0 disp is actually 4 bytes */
						/* further because of the delay slot */
			val >>= 2;
			dis_assemble_17(val,&w1,&w2,&w);
		/* There is no guarantee that buf is word-aligned,	*/
		/* so the adjustment must be done the hard way.		*/

		new_val  = (*buf & 0xff) << 24;
		new_val |= (*(buf+1) & 0xff) << 16;
		new_val |= (*(buf+2) & 0xff) << 8;
		new_val |= (*(buf+3) & 0xff);
		new_val |= ( ( w2 << 2 ) | ( w1 << 16 ) | w );
				      /* Now, do the adjustment */
		md_number_to_chars((char *)buf,new_val,4);
      }
      else {
		uint32_t result;
	  	val -= 4;	/* PA adjustment: a 0 disp is actually 4 bytes */
						/* further because of the delay slot */
		val >>= 2;

		result = sign_unext( val,17);
		dis_assemble_17(result,&w1,&w2,&w);
		/* There is no guarantee that buf is word-aligned,	*/
		/* so the adjustment must be done the hard way.		*/

		new_val  = (*buf & 0xff) << 24;
		new_val |= (*(buf+1) & 0xff) << 16;
		new_val |= (*(buf+2) & 0xff) << 8;
		new_val |= (*(buf+3) & 0xff);
		new_val |= ( ( w2 << 2 ) | ( w1 << 16 ) | w );
				      /* Now, do the adjustment */
		md_number_to_chars((char *)buf,new_val,4);

		}
		break;
/*     case 'z': */
	case HPPA_RELOC_BR17 :
	{
		uint32_t result;
		right14 >>= 2;
		result = sign_unext(right14,17);
		dis_assemble_17(result,&w1,&w2,&w);
	/* There is no guarantee that buf is word-aligned,	*/
	/* so the adjustment must be done the hard way.		*/

		new_val  = (*buf & 0xff) << 24;
		new_val |= (*(buf+1) & 0xff) << 16;
		new_val |= (*(buf+2) & 0xff) << 8;
		new_val |= (*(buf+3) & 0xff);
		new_val |= ( ( w2 << 2 ) | ( w1 << 16 ) | w );
				      /* Now, do the adjustment */
		md_number_to_chars((char *)buf,new_val,4);
     }
			break;
/*     case '@': */
    case HPPA_RELOC_JBSR :
/*
 * In case of the jbsr relocation no bytes are to be written to the 
 * output.
 * 
 * SO DO NOTHING!
 */
      break;
      
/*	  case 'w': */
/* To take care of 12 bit label */
      case HPPA_RELOC_12BRANCH :
		if ( !fixP->fx_addsy ) {
			val -= 4;	/* PA adjustment: a 0 disp is actually 4 bytes */
						/* further because of the delay slot */
			val >>= 2;
			dis_assemble_12(val,&w1,&w);
			/* There is no guarantee that buf is word-aligned,	*/
			/* so the adjustment must be done the hard way.		*/

			new_val  = (*buf & 0xff) << 24;
			new_val |= (*(buf+1) & 0xff) << 16;
			new_val |= (*(buf+2) & 0xff) << 8;
			new_val |= (*(buf+3) & 0xff);
			new_val |= ( ( w1 << 2 ) | w );	/* Now, do the adjustment */
			md_number_to_chars((char *)buf,new_val,4);
		}
		else {
			as_bad("Undefined symbol %s", fixP->fx_addsy->sy_name);
		}
		
		 break;
    }
}

void
md_convert_frag(
fragS *fragP)
{
  unsigned int address;

  if ( fragP -> fr_type == rs_machine_dependent ) {
    switch ( (int) fragP -> fr_subtype ) {
    case 0:
      fragP -> fr_type = rs_fill;
      know( fragP -> fr_var == 1 );
      know( fragP -> fr_next );
      address = fragP -> fr_address + fragP -> fr_fix;
      if ( address % fragP -> fr_offset ) {
	fragP -> fr_offset =
	  fragP -> fr_next -> fr_address
	    -   fragP -> fr_address
	      - fragP -> fr_fix;
      }
      else
	fragP -> fr_offset = 0;
      break;
    }
  }
}

int
md_estimate_size_before_relax(
fragS *fragP,
int nsect)
{
  int size;

  size = 0;

  while ( (fragP->fr_fix + size) % fragP->fr_offset )
    size++;

  return size;
}

int
md_parse_option(
char **argP,
int *cntP,
char ***vecP)
{
    return 1;
}

/*
int is_end_of_statement()
{
  return (   (*input_line_pointer == '\n')
	  || (*input_line_pointer == ';')
	  || (*input_line_pointer == '!') );
}
*/

static
int
parse_L_or_R(
char *str)
{
/* not much work done as yet! */
	switch (*str) {
	case '%':
	case '(':
		return 0;	/* ie. not found */
		break;
	case 'L':
	case 'l':
		if (*(str+1) == '\'' || *(str+1) == '`') /* check next character */
			return 1; /* found */
		else
			return 0; /* not found */
		break;
	case 'R':
	case 'r':
		if (*(str+1) == '\'' || *(str+1) == '`') /* check next character */
			return 2; /* found */
		else
			return 0; /* not found */
		break;
	default: /* default is not found ... at least for the time being */
		return 0;
		break;
	}
}    /* end parse_L_or_R() */

static
uint32_t
parse_cache_control_hint(
char	**s,       /* Note : the function changes '*s' */
int     option)    /* option = 0 for store instruction */
                   /* option = 1 for load and clear instruction */
{
	uint32_t cc = NO_CACHE_CONTROL_HINT;
				
	if (**s == ',') {
		(*s)++;
		switch (option) {
		case 0 : /* Store Instruction Cache Control Hint */ 
			if ( strncasecmp(*s,"bc",2) == 0 ) {
				/* BLOCK_COPY */
				(*s) += 2;
				cc = BC_OR_CO_CACHE_CONTROL_HINT;
				/* eat up extra blanks and tabs */
				while ( **s == ' ' || **s == '\t' )
					(*s)++;
			} else
				as_fatal("Illegal Cache Control Hint: '%s'"
					" - expected 'bc'",*s);
			break;
			
		case 1 : /* Load and Clear Word Cache Control Hint */
			if ( strncasecmp(*s,"co",2) == 0 ) {
				/* COHERENT_OPERATION */
				(*s) +=2;
				cc = BC_OR_CO_CACHE_CONTROL_HINT;
				/* eat up extra blanks and tabs */
				while ( **s == ' ' || **s == '\t' )
					(*s)++;
			} else
				as_fatal("Illegal Cache Control Hint: '%s'"
					" - expected 'co'",*s);
			break;
			
		default :
			as_fatal("Invalid option (%d) for parsing cache control hints",
				option);
			break;
		}
	}
		/* else NO_HINT */
				
	/*
	* the completers have already eaten up extra blanks
	* and tabs. So there is no need to do that again here.
	*/
				 
	return cc;

}    /* end parse_cache_control_hint() */

static
uint32_t
parse_completer_with_cache_control_hint(
char	**s,       /* Note : the function changes '*s' */
int     option,    /* option = 0 for store instruction */
                   /* option = 1 for load and clear instruction */
char    completer) /* 'c' or 'C' */
{
	uint32_t i, result = (uint32_t) 0U;
	int m, a, u;
	
	switch (completer) {
	case 'c':   /* indexed load completer. */
		i = m = u = 0;
		while ( **s == ',' && i < 3 ) {
			(*s)++;
			if ( strncasecmp((*s),"sm",2) == 0 ) {
				m = u = 1;
				(*s)++;
				i++;
			}
			else if ( strncasecmp((*s),"m",1) == 0 )
				m = 1;
			else if ( strncasecmp((*s),"s",1) == 0 )
				u = 1;
			else if ( strncmp((*s),",",1) == 0 ) /* no completer */
				result |= parse_cache_control_hint(s, option);
			else if ( (strncasecmp((*s),"c",1) == 0) ||
			          (strncasecmp((*s),"b",1) == 0) ) {/* just 1 completer */
				(*s)--;
				result |= parse_cache_control_hint(s, option);
			}
			else
				as_bad("Unrecognized Indexed Load"
					"Completer with cache control hints...assuming 0");
			if (result == (uint32_t)0U)
				(*s)++;
			i++;
		}
		if ( i > 3 )
			as_bad("Illegal Indexed Load Completer with cache control hints"
			" Syntax... extras ignored");
		while ( **s == ' ' || **s == '\t' )
			(*s)++;
  
		result |= m << 5;
		result |= u << 13;
		break;
	case 'C':   /* short load and store completer */
		i = m = a = 0;
		while ( **s == ',' && i < 2 ) {
			(*s)++;
			if ( strncasecmp((*s),"ma",2) == 0 ) {
				a = 0;
				m = 1;
			}
			else if ( strncasecmp((*s),"mb",2) == 0 ) {
				m = a = 1;
			}
			else if ( strncmp((*s),",",1) == 0 ) /* no completer */
				result |= parse_cache_control_hint(s, option);
			else if ( (strncasecmp((*s),"c",1) == 0) ||
			          (strncasecmp((*s),"b",1) == 0) ) {/* just 1 completer */
				(*s)--;
				result |= parse_cache_control_hint(s, option);
			}
			else
				as_bad("Unrecognized Indexed Load Completer"
				"...assuming 0");
			i++;
			(*s) += 2;
		}
		while ( **s == ' ' || **s == '\t' )
			(*s)++;
		result |= m << 5;
		result |= a << 13;
		break;
		
		
	default :
			as_fatal("Invalid completer (%c) for parsing cache control hints",
				completer);
			break;
	}
	return result;
}    /* end parse_completer_with_cache_control_hint() */

/* end hppa.c */
