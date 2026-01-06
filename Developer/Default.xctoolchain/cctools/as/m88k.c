/* m88k.c -- Assemble for the 88100
   Copyright (C) 1989 Free Software Foundation, Inc.

This file is not yet part of GAS, the GNU Assembler.

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

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <mach-o/m88k/reloc.h>
#include "m88k-opcode.h"
#include "as.h"
#include "flonum.h"
#include "expr.h"
#include "hash.h"
#include "frags.h"
#include "fixes.h"
#include "read.h"
#include "md.h"
#include "obstack.h"
#include "symbols.h"
#include "messages.h"
#include "input-scrub.h"
#include "sections.h"

/*
 * These are the default cputype and cpusubtype for the m88k architecture.
 */
const cpu_type_t md_cputype = CPU_TYPE_MC88000;
cpu_subtype_t md_cpusubtype = CPU_SUBTYPE_MC88000_ALL;

/* This is the byte sex for the m88k architecture */
const enum byte_sex md_target_byte_sex = BIG_ENDIAN_BYTE_SEX;

#ifdef NeXT_MOD
static int32_t in_delay_slot = 0;
#endif

static char *cmpslot[] = { "**", "**", "eq", "ne", "gt", "le", "lt", "ge",
			   "hi", "ls", "lo", "hs",
#ifdef m88110
			   "be", "nb", "he", "nh",
#endif /* m88110 */
			    NULL };

static struct {
	char *name;
	unsigned int num;

} cndmsk[] = {
		{ "eq0", 0x02},
		{ "ne0", 0x0d},
		{ "gt0", 0x01},
		{ "lt0", 0x0c},
		{ "ge0", 0x03},
		{ "le0", 0x0e},
		{ NULL,  0x00},
	      };

struct m88k_insn {
        uint32_t opcode;
        expressionS exp;
#ifdef NeXT_MOD
        int reloc;
#else
        enum reloc_type reloc;
#endif
};

static struct hash_control *op_hash = NULL;

/* These chars start a comment anywhere in a source file (except inside
   another comment */
const char md_comment_chars[] = ";";

/* These chars only start a comment at the beginning of a line. */
const char md_line_comment_chars[] = "#";

/* Chars that can be used to separate mant from exp in floating point nums */
const char md_EXP_CHARS[] = "eE";

/* Chars that mean this number is a floating point constant */
/* as in 0f123.456 */
/* or    0H1.234E-12 (see exp chars above) */
const char md_FLT_CHARS[] = "dDfF";

static int calcop(
    struct m88k_opcode *format,
    char *param,
    struct m88k_insn *insn);
static char * parse_reg(
    char *param,
    struct m88k_insn *insn,
    struct m88k_opcode *format,
    int parcnt);
#ifdef m88110
static char *parse_ereg(
    char *param,
    struct m88k_insn *insn,
    struct m88k_opcode *format,
    int parcnt);
static char *parse_e4rot(
    char *param,
    struct m88k_insn *insn,
    struct m88k_opcode *format,
    int parcnt);
static char *parse_xreg(
    char *param,
    struct m88k_insn *insn,
    struct m88k_opcode *format,
    int parcnt);
#endif /* m88110 */
static char *parse_pcr(
    char *param,
    struct m88k_insn *insn,
    struct m88k_opcode *format,
    int parcnt);
static char *parse_cmp(
    char *param,
    struct m88k_insn *insn,
    struct m88k_opcode *format,
    int parcnt);
static char *parse_cnd(
    char *param,
    struct m88k_insn *insn,
    struct m88k_opcode *format,
    int parcnt);
static char *parse_bf(
    char *param,
    struct m88k_insn *insn,
    struct m88k_opcode *format,
    int parcnt);
static char *parse_rot(
    char *param,
    struct m88k_insn *insn,
    struct m88k_opcode *format,
    int parcnt);
static char *parse_rsc(
    char *param,
    struct m88k_insn *insn,
    struct m88k_opcode *format,
    int parcnt);
static char *parse_cr(
    char *param,
    struct m88k_insn *insn,
    struct m88k_opcode *format,
    int parcnt);
static char *parse_fcr(
    char *param,
    struct m88k_insn *insn,
    struct m88k_opcode *format,
    int parcnt);
static char *parse_cst(
    char *param,
    struct m88k_insn *insn,
    struct m88k_opcode *format,
    int parcnt);
static char *getval(
    char *param,
    unsigned int *val);
#ifdef NeXT_MOD
static void s_reg(
    uintptr_t reg);
static void s_scaled(
    uintptr_t value);
static void s_m88k_abs(
    uintptr_t value);
static void s_no_delay(
    uintptr_t value);
static void s_dot(
    uintptr_t value);
#endif /* NeXT_MOD */

const pseudo_typeS md_pseudo_table[] =
{
#ifdef NeXT_MOD
	{"greg", s_reg, 'r' },
	{"xreg", s_reg, 'x' },
	{"scaled", s_scaled, 0},
	{"abs", s_m88k_abs, 0},
	{"no_delay", s_no_delay, 0},
	{"dot", s_dot, 0},
#endif
#ifndef NeXT_MOD
  	/* At NeXT we don't allow these */
	{"dfloat", float_cons, 'd'},
	{"ffloat", float_cons, 'f'},
	{"global", s_globl, 0},
	{"half", cons, 2 },
	{"ln", s_line, 0},
	{"zero", s_space, 0},
	{"word", cons, 4 },
#endif
	{0}
};

#ifdef NeXT_MOD
static
void
s_dot(
uintptr_t value)
{
	char *name, *end_name, delim;
	symbolS *symbolP;

	if( * input_line_pointer == '"')
	  name = input_line_pointer + 1;
	else
	  name = input_line_pointer;
	delim = get_symbol_end();
	end_name = input_line_pointer;
	*end_name = 0;

	symbolP = symbol_find_or_make (name);
	symbolP -> sy_type = N_ABS;
	symbolP -> sy_other = 0; /* NO_SECT */
	symbolP -> sy_value = obstack_next_free(&frags) - frag_now->fr_literal;
	symbolP -> sy_frag = &zero_address_frag;

	*end_name = delim;
	totally_ignore_line();
}
/*
 * s_reg() is used to implement ".greg symbol,exp" and ".xreg symbol,exp"
 * which set symbol to 1 or 0 depending on if the expression is a general
 * register or extended register respectfully.  These are intended for use in
 * macros.
 */
static
void
s_reg(
uintptr_t reg)
{
	char *name, *end_name, delim;
	symbolS *symbolP;
	uint32_t n_value, val;

	if( * input_line_pointer == '"')
	  name = input_line_pointer + 1;
	else
	  name = input_line_pointer;
	delim = get_symbol_end();
	end_name = input_line_pointer;
	*end_name = delim;
	SKIP_WHITESPACE();
	if ( * input_line_pointer != ',' ) {
		*end_name = 0;
		as_warn("Expected comma after name \"%s\"", name);
		*end_name = delim;
		ignore_rest_of_line();
		return;
	}
	input_line_pointer ++;
	*end_name = 0;

	SKIP_WHITESPACE();
	n_value = 0;
	if (*input_line_pointer == reg || *input_line_pointer == toupper(reg)){
	    input_line_pointer++;
	    if(isdigit(*input_line_pointer)){
		val = 0;
		while (isdigit(*input_line_pointer)){
		    if ((val = val * 10 + *input_line_pointer++ - '0') > 31)
			break;
		}
		SKIP_WHITESPACE();
		if(val <= 31 &&
		   (*input_line_pointer == '\n' || *input_line_pointer == '@'))
		    n_value = 1;
	    }
	}

	symbolP = symbol_find_or_make (name);
	symbolP -> sy_type = N_ABS;
	symbolP -> sy_other = 0; /* NO_SECT */
	symbolP -> sy_value = n_value;
	symbolP -> sy_frag = &zero_address_frag;

	*end_name = delim;
	totally_ignore_line();
}

/*
 * s_scaled() is used to implement ".scaled symbol,exp" which sets symbol to 1
 * or 0 depending on if the expression is a scaled general register expression
 * "r1[r2]" or not respectfully.  This is intended for use in macros.
 */
static
void
s_scaled(
uintptr_t value)
{
	char *name, *end_name, delim;
	symbolS *symbolP;
	uint32_t n_value, val;

	if( * input_line_pointer == '"')
	  name = input_line_pointer + 1;
	else
	  name = input_line_pointer;
	delim = get_symbol_end();
	end_name = input_line_pointer;
	*end_name = delim;
	SKIP_WHITESPACE();
	if ( * input_line_pointer != ',' ) {
		*end_name = 0;
		as_warn("Expected comma after name \"%s\"", name);
		*end_name = delim;
		ignore_rest_of_line();
		return;
	}
	input_line_pointer ++;
	*end_name = 0;

	SKIP_WHITESPACE();
	n_value = 0;
	if (*input_line_pointer == 'r' || *input_line_pointer == 'R'){
	    input_line_pointer++;
	    if(isdigit(*input_line_pointer)){
		val = 0;
		while (isdigit(*input_line_pointer)){
		    if ((val = val * 10 + *input_line_pointer++ - '0') > 31)
			break;
		}
		SKIP_WHITESPACE();
		if(val <= 31 && *input_line_pointer == '['){
		    input_line_pointer++;
		    if (*input_line_pointer == 'r' ||
			*input_line_pointer == 'R'){
			input_line_pointer++;
			if(isdigit(*input_line_pointer)){
			    val = 0;
			    while (isdigit(*input_line_pointer)){
				if ((val = val * 10 +
					   *input_line_pointer++ - '0') > 31)
				    break;
			    }
			    if(val <= 31 && *input_line_pointer == ']'){
				input_line_pointer++;
				SKIP_WHITESPACE();
				if(*input_line_pointer == '\n' ||
				   *input_line_pointer == '@')
		    		    n_value = 1;
			    }
			}
		    }
		}
	    }
	}

	symbolP = symbol_find_or_make (name);
	symbolP -> sy_type = N_ABS;
	symbolP -> sy_other = 0; /* NO_SECT */
	symbolP -> sy_value = n_value;
	symbolP -> sy_frag = & zero_address_frag;

	*end_name = delim;
	totally_ignore_line();
}

/*
 * s_m88k_abs() is used to implement ".abs symbol,exp" which sets symbol to 1
 * or 0 depending on if the expression is an absolute expression or not
 * respectfully.  This is intended for use in macros.
 */
static
void
s_m88k_abs(
uintptr_t value)
{
	char *name, *end_name, delim, *start;
	symbolS *symbolP;
	uint32_t n_value, val, is_reg_exp;

	start = input_line_pointer;
	if( * input_line_pointer == '"')
	  name = input_line_pointer + 1;
	else
	  name = input_line_pointer;
	delim = get_symbol_end();
	end_name = input_line_pointer;
	*end_name = delim;
	SKIP_WHITESPACE();
	if ( * input_line_pointer != ',' ) {
		*end_name = 0;
		as_warn("Expected comma after name \"%s\"", name);
		*end_name = delim;
		ignore_rest_of_line();
		return;
	}
	input_line_pointer ++;
	*end_name = 0;

	SKIP_WHITESPACE();
	is_reg_exp = 0;
	n_value = 0;
	if(*input_line_pointer == 'r' || *input_line_pointer == 'R'){
	    input_line_pointer++;
	    if(isdigit(*input_line_pointer)){
		val = 0;
		while (isdigit(*input_line_pointer)){
		    if ((val = val * 10 + *input_line_pointer++ - '0') > 31)
			break;
		}
		SKIP_WHITESPACE();
		if(val <= 31)
		    is_reg_exp = 1;
	    }
	}
	if(is_reg_exp == 0){
	    *end_name = delim;
	    input_line_pointer = start;
	    s_abs(value);
	    return;
	}

	symbolP = symbol_find_or_make (name);
	symbolP -> sy_type = N_ABS;
	symbolP -> sy_other = 0; /* NO_SECT */
	symbolP -> sy_value = n_value;
	symbolP -> sy_frag = & zero_address_frag;
	*end_name = delim;

	totally_ignore_line();
}

/*
 * s_no_delay() is used to implement ".no_delay string" which will abort and
 * print the string if the last instruction assembled has a delay slot.
 * This is intended for use in macros that expand to more than one instruction
 * that could be put in delay slots.  This is not really correct in it's
 * operation in that it is not per-section and does not take into account
 * anything other than assembled instructions.
 */
static
void
s_no_delay(
uintptr_t value)
{
	char *p, c;

	p = input_line_pointer;
	while(*p != '\n' && *p != '@' && *p != '\0')
	    p++;
	c = *p;
	*p = '\0';
	
	if(in_delay_slot)
	    as_fatal("delay slot abort %s detected.  Assembly stopping.",
		     input_line_pointer);
	input_line_pointer = p;
	*p = c;
}
#endif /* NeXT_MOD */

void
md_begin(
void)
{
	const char *retval = NULL;
	register unsigned int i = 0;

	/* initialize hash table */

	op_hash = hash_new();
	if (op_hash == NULL)
		as_fatal("Could not initialize hash table");

	/* loop until you see the end of the list */

	while (*m88k_opcodes[i].name) {
		char *name = m88k_opcodes[i].name;

		/* hash each mnemonic and record its position */

		retval = hash_insert(op_hash, name, (char *)&m88k_opcodes[i]);

		if (retval != NULL && *retval != '\0')
			as_fatal("Can't hash instruction '%s':%s",
					m88k_opcodes[i].name, retval);

		/* skip to next unique mnemonic or end of list */

		for (i++; !strcmp(m88k_opcodes[i].name, name); i++);
	}
}

int
md_parse_option(
char **argP,
int *cntP,
char ***vecP)
{
	return (1);
}

void
md_assemble(
char *op)
{
	char *param, *thisfrag;
	struct m88k_opcode *format;
	struct m88k_insn insn;
#ifdef NeXT_MOD
	int32_t pcrel_reloc;
#endif

	assert(op);

	/* skip over instruction to find parameters */

	/* *param != '\0' is need for instructions that have no parameters
	   like rte */
	for (param = op; !isspace(*param) && *param != '\0' ; param++);
	*param++ = '\0';

	/* try to find the instruction in the hash table */

	if ((format = (struct m88k_opcode *) hash_find(op_hash, op)) == NULL) {
		as_warn("Invalid mnemonic '%s'", op);
		return;
	}

	/* try parsing this instruction into insn */

	while (!calcop(format,param,&insn))

		/* if it doesn't parse try the next instruction */

		if (!strcmp(format->name, format[1].name))
			format++;
		else {
			as_warn("Parameter syntax error");
			return;
		}

	/* grow the current frag and plop in the opcode */

	thisfrag = frag_more(4);
	md_number_to_chars(thisfrag, insn.opcode, 4);
#ifdef NeXT_MOD
	in_delay_slot = format->delay_slot;
#endif
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

#ifdef NeXT_MOD
	pcrel_reloc = 0;
	if (insn.reloc == M88K_RELOC_PC16 || insn.reloc == M88K_RELOC_PC26){
	    /*
	     * The NeXT linker has the ability to scatter blocks of
	     * sections between labels.  This requires that brances to
	     * labels that survive to the link phase must be able to
	     * be relocated.
	     */
	    if(insn.exp.X_add_symbol != NULL &&
	       (insn.exp.X_add_symbol->sy_name[0] != 'L' || flagseen ['L']))
		pcrel_reloc = 1;
	    else
		pcrel_reloc = 0;
	}
#endif /* NeXT_MOD */

	/* if this instruction requires labels mark it for later */
	switch (insn.reloc) {

		case NO_RELOC:
				break;

		case M88K_RELOC_LO16:
		case M88K_RELOC_HI16:
				fix_new(
					frag_now,
#ifdef NeXT_MOD
					thisfrag - frag_now->fr_literal,
					4,
#else
					thisfrag - frag_now->fr_literal + 2,
					2,
#endif
					insn.exp.X_add_symbol,
					insn.exp.X_subtract_symbol,
					insn.exp.X_add_number,
					0, 0,
					insn.reloc
				);
				break;

#ifndef NeXT_MOD
		case M88K_RELOC_IW16:
				fix_new(
					frag_now,
					thisfrag - frag_now->fr_literal,
					4,
					insn.exp.X_add_symbol,
					insn.exp.X_subtract_symbol,
					insn.exp.X_add_number,
					0, 0,
					insn.reloc
				);
				break;
#endif /* !defined(NeXT_MOD) */

		case M88K_RELOC_PC16:
				fix_new(
					frag_now,
#ifdef NeXT_MOD
					thisfrag - frag_now->fr_literal,
					4,
#else
					thisfrag - frag_now->fr_literal + 2,
					2,
#endif
					insn.exp.X_add_symbol,
					insn.exp.X_subtract_symbol,
					insn.exp.X_add_number,
					1, pcrel_reloc,
					insn.reloc
				);
				break;

		case M88K_RELOC_PC26:
				fix_new(
					frag_now,
					thisfrag - frag_now->fr_literal,
					4,
					insn.exp.X_add_symbol,
					insn.exp.X_subtract_symbol,
					insn.exp.X_add_number,
					1, pcrel_reloc,
					insn.reloc
				);
				break;

		default:
				as_warn("Unknown relocation type");
				break;
	}
}

static
int
calcop(
struct m88k_opcode *format,
char *param,
struct m88k_insn *insn)
{
	int parcnt;

	/* initial the passed structure */

	memset(insn, '\0', sizeof(*insn));
	insn->reloc = NO_RELOC;
	insn->opcode = format->opcode;

	/* parse all parameters */

	for (parcnt=0; parcnt<3 && format->op[parcnt].type != NIL; parcnt++) {

		switch (format->op[parcnt].type) {

			case CNST:
				param = parse_cst(param, insn, format, parcnt);
				break;

			case REG:
				param = parse_reg(param, insn, format, parcnt);
				break;
#ifdef m88110
			case EREG:
				param = parse_ereg(param, insn, format, parcnt);
				break;

			case E4ROT:
				param = parse_e4rot(param, insn, format,parcnt);
				break;

			case XREG:
				param = parse_xreg(param, insn, format, parcnt);
				break;
#endif /* m88110 */
			case BF:
				param = parse_bf(param, insn, format, parcnt);
				break;

			case ROT:
				param = parse_rot(param, insn, format, parcnt);
				break;

			case REGSC:
				param = parse_rsc(param, insn, format, parcnt);
				break;

			case CRREG:
				param = parse_cr(param, insn, format, parcnt);
				break;

			case FCRREG:
				param = parse_fcr(param, insn, format, parcnt);
				break;

			case PCREL:
				param = parse_pcr(param, insn, format, parcnt);
				break;

			case CONDMASK:
				param = parse_cnd(param, insn, format, parcnt);
				break;

			case CMPRSLT:
				param = parse_cmp(param, insn, format, parcnt);
				break;

			default:
				as_fatal("Unknown parameter type");
		}

		/* see if parser failed or not */

		if (param == NULL)
			return 0;
	}

	return 1;
}

static
char *
parse_pcr(
char *param,
struct m88k_insn *insn,
struct m88k_opcode *format,
int parcnt)
{
	char *saveptr, *saveparam;
	segT seg;

	saveptr = input_line_pointer;
	input_line_pointer = param;

	seg = expression(&insn->exp);

	saveparam = input_line_pointer;
	input_line_pointer = saveptr;

	switch (format->op[parcnt].width) {

		case 16: insn->reloc = M88K_RELOC_PC16;
			 break;

		case 26: insn->reloc = M88K_RELOC_PC26;
			 break;

		default: as_warn("Strange PC relative width %d",
						format->op[parcnt].width);
			 break;
	}

	return saveparam;
}

static
char *
parse_reg(
char *param,
struct m88k_insn *insn,
struct m88k_opcode *format,
int parcnt)
{
	unsigned int val = 0;

	if (*param != 'r' && *param != 'R')
		return NULL;

	param++;

	if (!isdigit(*param))
		return NULL;

	while (isdigit(*param))
		if ((val = val * 10 + *param++ - '0') > 31)
			return NULL;

	insn->opcode |= val << format->op[parcnt].offset;

	switch (*param) {

		case '\0' :
			if (parcnt == 2 || format->op[parcnt+1].type == NIL)
				return param;
			else
				return NULL;

		case '['  :
			if (parcnt != 2 && format->op[parcnt+1].type == REGSC)
				return param+1;
			else
				return NULL;

		case ','  :
			if (parcnt != 2 && format->op[parcnt+1].type != NIL)
				return param+1;
			else
				return NULL;
	}

	return NULL;
}

#ifdef m88110
static
char *
parse_ereg(
char *param,
struct m88k_insn *insn,
struct m88k_opcode *format,
int parcnt)
{
	unsigned int val = 0;

	if (*param != 'r' && *param != 'R')
		return NULL;

	param++;

	if (!isdigit(*param))
		return NULL;

	while (isdigit(*param))
		if ((val = val * 10 + *param++ - '0') > 31)
			return NULL;

	if((val & 0x1) != 0)
		return NULL;

	insn->opcode |= val << format->op[parcnt].offset;

	switch (*param) {

		case '\0' :
			if (parcnt == 2 || format->op[parcnt+1].type == NIL)
				return param;
			else
				return NULL;

		case '['  :
			if (parcnt != 2 && format->op[parcnt+1].type == REGSC)
				return param+1;
			else
				return NULL;

		case ','  :
			if (parcnt != 2 && format->op[parcnt+1].type != NIL)
				return param+1;
			else
				return NULL;
	}

	return NULL;
}

static
char *
parse_e4rot(
char *param,
struct m88k_insn *insn,
struct m88k_opcode *format,
int parcnt)
{
	int val;
	char *saveptr, save_c, *offset_ptr;
        expressionS exp;
	segT seg;

	/* Now step over the '<' and look for the offset expression before a
	   '>' and the end of line (which is a '\0' when we get here).  We
	   know there is a '\0' where the end of line was because that is
	   what parse_a_buffer() in read.c does before calling md_assemble */
	if (*param++ != '<')
		return NULL;
	offset_ptr = param;
	while(*param != '\0')
		param++;
	if(param == offset_ptr || param[-1] != '>')
		return NULL;
	param--;
	save_c = *param;
	*param = '\0';
	saveptr = input_line_pointer;
	input_line_pointer = offset_ptr;
	seg = expression(&exp);
	*param = save_c;
	input_line_pointer = saveptr;
	val = exp.X_add_number;
	if(seg != SEG_ABSOLUTE || val > 60 || (val & 0x3) != 0)
		return NULL;

	val >>= 2;
	insn->opcode |= val << format->op[parcnt].offset;

	return param+1;
}

static
char *
parse_xreg(
char *param,
struct m88k_insn *insn,
struct m88k_opcode *format,
int parcnt)
{
	unsigned int val = 0;

	if (*param != 'x' && *param != 'X')
		return NULL;

	param++;

	if (!isdigit(*param))
		return NULL;

	while (isdigit(*param))
		if ((val = val * 10 + *param++ - '0') > 31)
			return NULL;

	insn->opcode |= val << format->op[parcnt].offset;

	switch (*param) {

		case '\0' :
			if (parcnt == 2 || format->op[parcnt+1].type == NIL)
				return param;
			else
				return NULL;

		case '['  :
			if (parcnt != 2 && format->op[parcnt+1].type == REGSC)
				return param+1;
			else
				return NULL;

		case ','  :
			if (parcnt != 2 && format->op[parcnt+1].type != NIL)
				return param+1;
			else
				return NULL;
	}

	return NULL;
}
#endif /* m88110 */

static
char *
parse_cmp(
char *param,
struct m88k_insn *insn,
struct m88k_opcode *format,
int parcnt)
{
	int val;
	char *saveptr, save_c, *offset_ptr, c;
        expressionS exp;
	segT seg;

	/* look for the offset expression before a ',' */
	c = *param;
	if (isdigit(c) || c == '(' || c == '-' || c == '+' || c == '!' ||
	    c == '~'){
		offset_ptr = param;
		while(*param != ',')
			param++;
		if(param == offset_ptr || *param != ',')
			return NULL;
		save_c = *param;
		*param = '\0';
		saveptr = input_line_pointer;
		input_line_pointer = offset_ptr;
		seg = expression(&exp);
		*param = save_c;
		input_line_pointer = saveptr;
		val = exp.X_add_number;
		if(seg != SEG_ABSOLUTE ||
		   val > (1 << format->op[parcnt].width) || val < 0)
			return NULL;
	} else {
		if (isupper(*param))
			*param = tolower(*param);

		if (isupper(*(param+1)))
			*(param+1) = tolower(*(param+1));

		for (val=0; cmpslot[val] != NULL; val++)
			if (!strncmp(param,cmpslot[val],2))
				break;

		if (cmpslot[val] == NULL)
			return NULL;

		param += 2;
	}

	if (*param++ != ',')
		return NULL;

	insn->opcode |= val << format->op[parcnt].offset;

	return param;
}

static
char *
parse_cnd(
char *param,
struct m88k_insn *insn,
struct m88k_opcode *format,
int parcnt)
{
	int val;
	char *saveptr, save_c, *offset_ptr, c;
        expressionS exp;
	segT seg;

	/* look for the offset expression before a ',' */
	c = *param;
	if (isdigit(c) || c == '(' || c == '-' || c == '+' || c == '!' ||
	    c == '~'){
		offset_ptr = param;
		while(*param != ',')
			param++;
		if(param == offset_ptr || *param != ',')
			return NULL;
		save_c = *param;
		*param = '\0';
		saveptr = input_line_pointer;
		input_line_pointer = offset_ptr;
		seg = expression(&exp);
		*param = save_c;
		input_line_pointer = saveptr;
		val = exp.X_add_number;
		if(seg != SEG_ABSOLUTE ||
		   val > (1 << format->op[parcnt].width) || val < 0)
			return NULL;
	} else {
		if (isupper(*param))
			*param = tolower(*param);

		if (isupper(*(param+1)))
			*(param+1) = tolower(*(param+1));

		for (val=0; cndmsk[val].name != NULL; val++)
			if (!strncmp(param,cndmsk[val].name,3))
				break;

		if (cndmsk[val].name == NULL)
			return NULL;

		val = cndmsk[val].num;

		param += 3;
	}

	if (*param++ != ',')
		return NULL;

	insn->opcode |= val << format->op[parcnt].offset;

	return param;
}

static
char *
parse_bf(
char *param,
struct m88k_insn *insn,
struct m88k_opcode *format,
int parcnt)
{
	int val, width;
	char *saveptr, save_c, *offset_ptr, c;
        expressionS exp;
	segT seg;

	/* We know there is a '\0' where the end of line was because that is
	   what parse_a_buffer() in read.c does before calling md_assemble */

	/* First look for the width expression before a '<' */
	saveptr = input_line_pointer;
	input_line_pointer = param;
	while(*param != '<' && *param != '\0')
		param++;
	if(*param == '\0'){
		input_line_pointer = saveptr;
		return NULL;
	}
	save_c = *param;
	*param = '\0';
	seg = expression(&exp);
	*param = save_c;
	input_line_pointer = saveptr;
	width = exp.X_add_number;
	if(seg != SEG_ABSOLUTE || width > 32 || width < 0)
		return NULL;

	/* Now step over the '<' and look for the offset expression before a
	   '>' and the end of line (which is a '\0' when we get here) */
	param++;
	c = *param;
	if (isdigit(c) || c == '(' || c == '-' || c == '+' || c == '!' ||
	    c == '~'){
		offset_ptr = param;
		while(*param != '\0')
			param++;
		if(param != offset_ptr && param[-1] != '>')
			return NULL;
		param--;
		save_c = *param;
		*param = '\0';
		saveptr = input_line_pointer;
		input_line_pointer = offset_ptr;
		seg = expression(&exp);
		*param = save_c;
		input_line_pointer = saveptr;
		val = exp.X_add_number;
		if(seg != SEG_ABSOLUTE || val > 32 || val < 0)
			return NULL;
	}
	else {
		if (isupper(*param))
			*param = tolower(*param);

		if (isupper(*(param+1)))
			*(param+1) = tolower(*(param+1));

		for (val=0; cmpslot[val] != NULL; val++)
			if (!strncmp(param,cmpslot[val],2))
				break;

		if (cmpslot[val] == NULL)
			return NULL;

		param += 2;
	}
	if (*param != '>')
		return NULL;
	insn->opcode |= width << 5;
	insn->opcode |= val;

	return param+1;
}

static
char *
parse_rot(
char *param,
struct m88k_insn *insn,
struct m88k_opcode *format,
int parcnt)
{
	int val;
	char *saveptr, save_c, *offset_ptr;
        expressionS exp;
	segT seg;

	/* Now step over the '<' and look for the offset expression before a
	   '>' and the end of line (which is a '\0' when we get here).  We
	   know there is a '\0' where the end of line was because that is
	   what parse_a_buffer() in read.c does before calling md_assemble */
	if (*param++ != '<')
		return NULL;
	offset_ptr = param;
	while(*param != '\0')
		param++;
	if(param != offset_ptr && param[-1] != '>')
		return NULL;
	param--;
	save_c = *param;
	*param = '\0';
	saveptr = input_line_pointer;
	input_line_pointer = offset_ptr;
	seg = expression(&exp);
	*param = save_c;
	input_line_pointer = saveptr;
	val = exp.X_add_number;
	if(seg != SEG_ABSOLUTE && (val > 32 || val < 0))
		return NULL;

	insn->opcode |= val;

	return param+1;
}

static
char *
parse_rsc(
char *param,
struct m88k_insn *insn,
struct m88k_opcode *format,
int parcnt)
{
	unsigned int val = 0;

	if (*param != 'r' && *param != 'R')
		return NULL;

	param++;

	if (!isdigit(*param))
		return NULL;

	while (isdigit(*param))
		if ((val = val * 10 + *param++ - '0') > 31)
			return NULL;

	insn->opcode |= val << format->op[parcnt].offset;

	if (*param != ']' || *(param+1) != '\0')
		return NULL;

	return param+1;
}

static
char *
parse_cr(
char *param,
struct m88k_insn *insn,
struct m88k_opcode *format,
int parcnt)
{
	unsigned int val = 0;

	if (strncmp(param, "cr", 2))
		return NULL;

	param += 2;

	if (!isdigit(*param))
		return NULL;

	while (isdigit(*param))
		if ((val = val * 10 + *param++ - '0') > 63)
			return NULL;

	/*
	 * the following fix is not as generic as I'd like, but the
	 * hardware is real picky about this.	- bowen@cs.buffalo.edu
	 * This fix is to make sure the S1 and S2 fields are the same.
	 */
	insn->opcode |= (insn->opcode & 0x001f0000) >> 16;

	insn->opcode |= val << format->op[parcnt].offset;

	if (*param != '\0')
		return NULL;

	return param;
}

static
char *
parse_fcr(
char *param,
struct m88k_insn *insn,
struct m88k_opcode *format,
int parcnt)
{
	unsigned int val = 0;

	if (strncmp(param, "fcr", 3))
		return NULL;

	param += 3;

	if (!isdigit(*param))
		return NULL;

	while (isdigit(*param))
		if ((val = val * 10 + *param++ - '0') > 63)
			return NULL;

	/*
	 * This is to make sure the S1 and S2 fields are the same.
	 */
	insn->opcode |= (insn->opcode & 0x001f0000) >> 16;

	insn->opcode |= val << format->op[parcnt].offset;

	if (*param != '\0')
		return NULL;

	return param;
}

static
char *
parse_cst(
char *param,
struct m88k_insn *insn,
struct m88k_opcode *format,
int parcnt)
{
	char c, *saveptr, *saveparam;
	int val, nohilo = 0;
	segT seg;
        expressionS exp;

	c = *param;
	if (isdigit(c) || c == '(' || c == '-' || c == '+' || c == '!' ||
	    c == '~'){
		saveptr = input_line_pointer;
		input_line_pointer = param;
		while(*param != '\0')
			param++;
		seg = expression(&exp);
		input_line_pointer = saveptr;
		val = exp.X_add_number;
		if(seg != SEG_ABSOLUTE || val > (1 << format->op[parcnt].width))
			return NULL;
	}
	else if (!strncmp(param,"hi16(",5))

		if (isdigit(*(param+5))) {
			param = getval(param+5, (unsigned int *)&val);
			val = (val & 0xffff0000) >> 16;
			if (*param++ != ')')
				return NULL;

		} else
			insn->reloc = M88K_RELOC_HI16;
	else if (!strncmp(param,"lo16(",5))

		if (isdigit(*(param+5))) {
			param = getval(param+5, (unsigned int *)&val);
			val &= 0x0000ffff;
			if (*param++ != ')')
				return NULL;

		} else
			insn->reloc = M88K_RELOC_LO16;

#ifndef NeXT_MOD
	else if (!strncmp(param,"iw16(",5))

		if (isdigit(*(param+5))) {
			param = getval(param+5,&val);
			val &= 0x0000ffff;
			if (*param++ != ')')
				return NULL;

		} else
			insn->reloc = M88K_RELOC_IW16;
#endif /* !defined(NeXT_MOD) */

	else if (*param == 'r' && isdigit(*(param+1)))

		return NULL;

	else {
		insn->reloc = M88K_RELOC_LO16;
		nohilo = 1;
	}

	if (insn->reloc != NO_RELOC) {

		saveptr = input_line_pointer;
		input_line_pointer = param + (nohilo ? 0 : 5);

		seg = expression(&insn->exp);

		saveparam = input_line_pointer;
		input_line_pointer = saveptr;

		if (nohilo) {

			if (*saveparam != '\0')
				return NULL;

			return saveparam;
		}

		if (*saveparam != ')')
			return NULL;

		return saveparam+1;
	}

	if ((1 << format->op[parcnt].width) <= val)
		return NULL;

	insn->opcode |= val << format->op[parcnt].offset;

	if (*param != '\0')
		return NULL;

	return param;
}

#define isoct(z) (z >= '0' && z <= '7')
#define ishex(z) ((z >= '0' && z <= '9') || (z >= 'a' && z <= 'f') || (z >= 'A' && z <= 'F'))
#define hexval(z) \
  (isdigit(z) ? (z) - '0' :		\
   islower(z) ? (z) - 'a' + 10 :	\
   (z) - 'A' + 10)

static
char *
getval(
char *param,
unsigned int *val)
{
	*val = 0;

	if (*param == '0' && (*(param+1) == 'x' || *(param+1) == 'X'))

		for (param += 2; ishex(*param); param++)

			if (*val > 0x0fffffff)
				return param;
			else
				*val = *val * 16 + hexval(*param);

	else if (*param == '0')

		for (param++; isoct(*param); param++)

			if (*val > 0x1fffffff)
				return param;
			else
				*val = *val * 8 + *param - '0';

	else

		for (; isdigit(*param); param++)

			*val = *val * 10 + *param - '0';

	return param;
}

void
md_number_to_chars(
char *buf,
signed_expr_t val,
int nbytes)
{
	switch(nbytes) {

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
}

void
md_number_to_imm(
unsigned char *buf,
signed_expr_t val,
int nbytes,
fixS *fixP,
int nsect)
{
	if(fixP->fx_r_type == NO_RELOC ||
	   fixP->fx_r_type == M88K_RELOC_VANILLA) {
		switch (nbytes) {
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

	switch (fixP->fx_r_type) {
#ifdef NeXT_MOD
			case M88K_RELOC_LO16:
				buf[2] = val >> 8;
				buf[3] = val;
				break;
			case M88K_RELOC_HI16:
				buf[2] = val >> 24;
				buf[3] = val >> 16;
				break;

			case M88K_RELOC_PC16:
				val += 4;
				buf[2] = val >> 10;
				buf[3] = val >> 2;
				break;

			case M88K_RELOC_PC26:
				val += 4;
				buf[0] |= (val >> 26) & 0x03;
				buf[1] = val >> 18;
				buf[2] = val >> 10;
				buf[3] = val >> 2;
				break;
#else /* !defined NeXT_MOD */
			case M88K_RELOC_LO16:
				buf[0] = val >> 8;
				buf[1] = val;
				break;

			case M88K_RELOC_IW16:
				buf[2] = val >> 8;
				buf[3] = val;
				break;

			case M88K_RELOC_HI16:
				buf[0] = val >> 24;
				buf[1] = val >> 16;
				break;

			case M88K_RELOC_PC16:
				val += 4;
				buf[0] = val >> 10;
				buf[1] = val >> 2;
				break;

			case M88K_RELOC_PC26:
				val += 4;
				buf[0] |= (val >> 26) & 0x03;
				buf[1] = val >> 18;
				buf[2] = val >> 10;
				buf[3] = val >> 2;
				break;

			case M88K_RELOC_32:
				buf[0] = val >> 24;
				buf[1] = val >> 16;
				buf[2] = val >> 8;
				buf[3] = val;
				break;
#endif /* !defined(NeXT_MOD) */

			default:
				as_warn("Bad relocation type");
				break;
	}
}

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

const relax_typeS md_relax_table[] = { {0} };

int
md_estimate_size_before_relax(
fragS *fragP,
int segment_type)
{
	as_fatal("internal error: Relaxation should never occur");
	return(0);
}

void
md_convert_frag(
fragS *fragP)
{
	as_fatal("internal error: Relaxation should never occur");
}

void
md_end(
void)
{
}
