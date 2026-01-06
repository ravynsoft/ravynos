/* hppa-aux.c -- Assembler for the PA - PA-RISC specific support routines
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

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "obstack.h"
#include "as.h"
#include "frags.h"
#include "flonum.h"
#include "expr.h"
#include "hash.h"
#include "md.h"
#include "symbols.h"
#include "messages.h"
#include "hppa-aux.h"
#include "stuff/hppa.h"

extern char *expr_end; /* defined in hppa.c */

static const int print_errors = 1;

static int reg_name_search(
    char *name);

int pa_parse_number(s)
	char **s;
{
	int num;
	char *name;
	char c;
	symbolS *sym;
	int status;
	char * p = *s;

	while ( *p == ' ' || *p == '\t' )
		p = p + 1;
	num=-1; /* assume invalid number to begin with */
	if (isdigit(*p)) {
		num = 0; /* now we know it is a number */

		if ( *p == '0'
			&& ( *(p+1) == 'x' || *(p+1) == 'X' ) ) { /* hex input */
			p = p + 2;
			while ( isdigit(*p)	|| ( (*p >= 'a') && (*p <= 'f') )
				|| ( (*p >= 'A') && (*p <= 'F') ) ){
				if ( isdigit(*p) )
					num = num*16 + *p-'0';
				else if ( *p >= 'a' && *p <= 'f' )
					num = num*16 + *p-'a' + 10; 
				else
					num = num*16 + *p-'A' + 10; 
				++p;
			}
		}
		else {
			while (isdigit(*p)) {
				num= num*10 + *p-'0';
				++p;
			}
		}
	}
	else if ( *p == '%' ) {   /* could be a pre-defined register */
		num = 0;
		name = p;
		p++;
		c = *p;
		
		/*
		 * tege hack: Special case for general registers as the 
		 * general code makes a binary search with case translation,
		 * and is VERY slow.
		 */
		if (c == 'r') {
			p++;
			if (!isdigit(*p))
				as_bad("Undefined register: '%s'. ASSUMING 0",name);
			else {
				do
					num= num*10 + *p++ - '0';
				while (isdigit(*p));
			}
		}
		else {
	    	while ( is_part_of_name(c) ) {
		    	p = p + 1;
		    	c = *p;
	    	}
	    	/* Terminate string with \0.  Restore below.  */
	    	*p = 0;
	    	status = reg_name_search(name);
	    	if ( status >= 0 )
				num = status;
	    	else {
		    	if ( print_errors )
			    	as_bad("Undefined register: '%s'. ASSUMING 0",name);
		    	else
			    	num = -1;
	    	}
	    	/* Restore orignal value of string.  */
	    	*p = c;
		}
	}
	else {
		num = 0;
		name = p;
		c = *p;
		while ( is_part_of_name(c) ) {
			p = p + 1;
			c = *p;
		}
		*p = 0;
		if ( (sym = symbol_table_lookup(name)) != NULL ) {
			if ( sym->sy_type == N_ABS &&  sym->sy_other == NO_SECT ) {
				num = sym->sy_value;
			}
			else {
				if ( print_errors )
					as_bad("Non-absolute constant: '%s'. ASSUMING 0",name);
				else
					num = -1;
			}
		}
		else {
			if ( print_errors )
				as_bad("Undefined absolute constant: '%s'. ASSUMING 0",name);
			else
				num = -1;
		}
		*p = c;
	}

	*s = p;
	return num;
}

struct pd_reg {
	char	*name;
	int	value;
};

/*	List of registers that are pre-defined:

	General Registers:

	Name	Value		Name	Value
	%r0			0		%r16	16
	%r1			1		%r17	17
	%r2			2		%r18	18
	%r3			3		%r19	19
	%r4			4		%r20	20
	%r5			5		%r21	21
	%r6			6		%r22	22
	%r7			7		%r23	23
	%r8			8		%r24	24
	%r9			9		%r25	25
	%r10		10		%r26	26
	%r11		11		%r27	27
	%r12		12		%r28	28
	%r13		13		%r29	29
	%r14		14		%r30	30
	%r15		15		%r31	31

	Floating-point Registers:
	[NOTE:  Also includes L and R versions of these (e.g. %fr19L, %fr19R)]

	Name	Value		Name	Value
	%fr0	0		%fr16	16
	%fr1	1		%fr17	17
	%fr2	2		%fr18	18
	%fr3	3		%fr19	19
	%fr4	4		%fr20	20
	%fr5	5		%fr21	21
	%fr6	6		%fr22	22
	%fr7	7		%fr23	23
	%fr8	8		%fr24	24
	%fr9	9		%fr25	25
	%fr10	10		%fr26	26
	%fr11	11		%fr27	27
	%fr12	12		%fr28	28
	%fr13	13		%fr29	29
	%fr14	14		%fr30	30
	%fr15	15		%fr31	31

	Space Registers:

	Name	Value		Name	Value
	%sr0	0		%sr4	4
	%sr1	1		%sr5	5
	%sr2	2		%sr6	6
	%sr3	3		%sr7	7

	Control registers and their synonyms:

	Names			Value
	%cr0	%rctr		0
	%cr8	%pidr1		8
	%cr9	%pidr2		9
	%cr10	%ccr		10
	%cr11	%sar		11
	%cr12	%pidr3		12
	%cr13	%pidr4		13
	%cr14	%iva		14
	%cr15	%eiem		15
	%cr16	%itmr		16
	%cr17	%pcsq		17
	%cr18	%pcoq		18
	%cr19	%iir		19
	%cr20	%isr		20
	%cr21	%ior		21
	%cr22	%ipsw		22
	%cr23	%eirr		23
	%cr24	%tr0 %ppda	24
	%cr25	%tr1 %hta	25
	%cr26	%tr2		26
	%cr27	%tr3		27
	%cr28	%tr4		28
	%cr29	%tr5		29
	%cr30	%tr6		30
	%cr31	%tr7		31

*/

/* This table is sorted. Suitable for searching by a binary search. */

static struct pd_reg pre_defined_registers[] = {
	{	"%ccr",		10	},
	{	"%cr0",		0	},
	{	"%cr10",	10	},
	{	"%cr11",	11	},
	{	"%cr12",	12	},
	{	"%cr13",	13	},
	{	"%cr14",	14	},
	{	"%cr15",	15	},
	{	"%cr16",	16	},
	{	"%cr17",	17	},
	{	"%cr18",	18	},
	{	"%cr19",	19	},
	{	"%cr20",	20	},
	{	"%cr21",	21	},
	{	"%cr22",	22	},
	{	"%cr23",	23	},
	{	"%cr24",	24	},
	{	"%cr25",	25	},
	{	"%cr26",	26	},
	{	"%cr27",	27	},
	{	"%cr28",	28	},
	{	"%cr29",	29	},
	{	"%cr30",	30	},
	{	"%cr31",	31	},
	{	"%cr8",		8	},
	{	"%cr9",		9	},
	{	"%eiem",	15	},
	{	"%eirr",	23	},
	{	"%fr0",		0	},
	{	"%fr0L",	0	},
	{	"%fr0R",	0	},
	{	"%fr1",		1	},
	{	"%fr10",	10	},
	{	"%fr10L",	10	},
	{	"%fr10R",	10	},
	{	"%fr11",	11	},
	{	"%fr11L",	11	},
	{	"%fr11R",	11	},
	{	"%fr12",	12	},
	{	"%fr12L",	12	},
	{	"%fr12R",	12	},
	{	"%fr13",	13	},
	{	"%fr13L",	13	},
	{	"%fr13R",	13	},
	{	"%fr14",	14	},
	{	"%fr14L",	14	},
	{	"%fr14R",	14	},
	{	"%fr15",	15	},
	{	"%fr15L",	15	},
	{	"%fr15R",	15	},
	{	"%fr16",	16	},
	{	"%fr16L",	16	},
	{	"%fr16R",	16	},
	{	"%fr17",	17	},
	{	"%fr17L",	17	},
	{	"%fr17R",	17	},
	{	"%fr18",	18	},
	{	"%fr18L",	18	},
	{	"%fr18R",	18	},
	{	"%fr19",	19	},
	{	"%fr19L",	19	},
	{	"%fr19R",	19	},
	{	"%fr1L",	1	},
	{	"%fr1R",	1	},
	{	"%fr2",		2	},
	{	"%fr20",	20	},
	{	"%fr20L",	20	},
	{	"%fr20R",	20	},
	{	"%fr21",	21	},
	{	"%fr21L",	21	},
	{	"%fr21R",	21	},
	{	"%fr22",	22	},
	{	"%fr22L",	22	},
	{	"%fr22R",	22	},
	{	"%fr23",	23	},
	{	"%fr23L",	23	},
	{	"%fr23R",	23	},
	{	"%fr24",	24	},
	{	"%fr24L",	24	},
	{	"%fr24R",	24	},
	{	"%fr25",	25	},
	{	"%fr25L",	25	},
	{	"%fr25R",	25	},
	{	"%fr26",	26	},
	{	"%fr26L",	26	},
	{	"%fr26R",	26	},
	{	"%fr27",	27	},
	{	"%fr27L",	27	},
	{	"%fr27R",	27	},
	{	"%fr28",	28	},
	{	"%fr28L",	28	},
	{	"%fr28R",	28	},
	{	"%fr29",	29	},
	{	"%fr29L",	29	},
	{	"%fr29R",	29	},
	{	"%fr2L",	2	},
	{	"%fr2R",	2	},
	{	"%fr3",		3	},
	{	"%fr30",	30	},
	{	"%fr30L",	30	},
	{	"%fr30R",	30	},
	{	"%fr31",	31	},
	{	"%fr31L",	31	},
	{	"%fr31R",	31	},
	{	"%fr3L",	3	},
	{	"%fr3R",	3	},
	{	"%fr4",		4	},
	{	"%fr4L",	4	},
	{	"%fr4R",	4	},
	{	"%fr5",		5	},
	{	"%fr5L",	5	},
	{	"%fr5R",	5	},
	{	"%fr6",		6	},
	{	"%fr6L",	6	},
	{	"%fr6R",	6	},
	{	"%fr7",		7	},
	{	"%fr7L",	7	},
	{	"%fr7R",	7	},
	{	"%fr8",		8	},
	{	"%fr8L",	8	},
	{	"%fr8R",	8	},
	{	"%fr9",		9	},
	{	"%fr9L",	9	},
	{	"%fr9R",	9	},
	{	"%hta",		25	},
	{	"%iir",		19	},
	{	"%ior",		21	},
	{	"%ipsw",	22	},
	{	"%isr",		20	},
	{	"%itmr",	16	},
	{	"%iva",		14	},
	{	"%pcoq",	18	},
	{	"%pcsq",	17	},
	{	"%pidr1",	8	},
	{	"%pidr2",	9	},
	{	"%pidr3",	12	},
	{	"%pidr4",	13	},
	{	"%ppda",	24	},
	{	"%r0",		0	},
	{	"%r1",		1	},
	{	"%r10",		10	},
	{	"%r11",		11	},
	{	"%r12",		12	},
	{	"%r13",		13	},
	{	"%r14",		14	},
	{	"%r15",		15	},
	{	"%r16",		16	},
	{	"%r17",		17	},
	{	"%r18",		18	},
	{	"%r19",		19	},
	{	"%r2",		2	},
	{	"%r20",		20	},
	{	"%r21",		21	},
	{	"%r22",		22	},
	{	"%r23",		23	},
	{	"%r24",		24	},
	{	"%r25",		25	},
	{	"%r26",		26	},
	{	"%r27",		27	},
	{	"%r28",		28	},
	{	"%r29",		29	},
	{	"%r3",		3	},
	{	"%r30",		30	},
	{	"%r31",		31	},
	{	"%r4",		4	},
	{	"%r4L",		4	},
	{	"%r4R",		4	},
	{	"%r5",		5	},
	{	"%r5L",		5	},
	{	"%r5R",		5	},
	{	"%r6",		6	},
	{	"%r6L",		6	},
	{	"%r6R",		6	},
	{	"%r7",		7	},
	{	"%r7L",		7	},
	{	"%r7R",		7	},
	{	"%r8",		8	},
	{	"%r8L",		8	},
	{	"%r8R",		8	},
	{	"%r9",		9	},
	{	"%r9L",		9	},
	{	"%r9R",		9	},
	{	"%rctr",	0	},
	{	"%sar",		11	},
	{	"%sr0",		0	},
	{	"%sr1",		1	},
	{	"%sr2",		2	},
	{	"%sr3",		3	},
	{	"%sr4",		4	},
	{	"%sr5",		5	},
	{	"%sr6",		6	},
	{	"%sr7",		7	},
	{	"%tr0",		24	},
	{	"%tr1",		25	},
	{	"%tr2",		26	},
	{	"%tr3",		27	},
	{	"%tr4",		28	},
	{	"%tr5",		9	},
	{	"%tr6",		30	},
	{	"%tr7",		31	}
};

#define REG_NAME_CNT	(sizeof(pre_defined_registers) / sizeof(struct pd_reg))

static
int
reg_name_search(
char *name)
{
	int x,l,r;

	l = 0;
	r = REG_NAME_CNT - 1;

	do {
		x = (l + r) / 2;
		if (strcasecmp(name,pre_defined_registers[x].name) < 0)
			r = x - 1;
		else
			l = x + 1;
	} while ( !( (strcasecmp(name,pre_defined_registers[x].name) == 0) ||
		     (l > r) ) );

	if ( strcasecmp(name,pre_defined_registers[x].name) == 0 )
		return(pre_defined_registers[x].value);
	else
		return(-1);

}
    
static
int
is_R_select(
char *s)
{

  if ( *s == 'R' || *s == 'r' )
    return(TRUE);
  else
    return(FALSE);	
}

static
int
is_L_select(
char *s)
{

  if ( *s == 'L' || *s == 'l' )
    return(TRUE);
  else
    return(FALSE);	
}

int need_89_opcode(insn,result)
     struct pa_it *insn;
     struct pa_89_fp_reg_struct *result;
{
  if ( result->L_R_select == 1 && !(insn->fpof1 == DBL && insn->fpof2 == DBL) )
    return TRUE;
  else
    return FALSE;
}

int
pa_89_parse_number(s,result)
     char **s;
     struct pa_89_fp_reg_struct *result;
{
  int num;
  char *name;
  char c;
  symbolS *sym;
  int status;
  char * p = *s;

  while ( *p == ' ' || *p == '\t' )
    p = p + 1;
  num=-1; /* assume invalid number to begin with */
  result->number_part = -1;
  result->L_R_select  = -1;

  if (isdigit(*p)) {
    num = 0; /* now we know it is a number */

    if ( *p == '0' && ( *(p+1) == 'x' || *(p+1) == 'X' ) ) { /* hex input */
      p = p + 2;
      while ( isdigit(*p)
	     || ( (*p >= 'a') && (*p <= 'f') )
	     || ( (*p >= 'A') && (*p <= 'F') ) ){
	if ( isdigit(*p) )
	  num = num*16 + *p-'0';
	else if ( *p >= 'a' && *p <= 'f' )
	  num = num*16 + *p-'a' + 10; 
	else
	  num = num*16 + *p-'A' + 10; 
	++p;
      }
    }
    else {
      while (isdigit(*p)) {
	num= num*10 + *p-'0';
	++p;
      }
    }

    result->number_part = num;

    if ( is_R_select(p) ) {
      result->L_R_select = 1;
      ++p;
    }
    else if ( is_L_select(p) ) {
      result->L_R_select = 0;
      ++p;
    }
    else
      result->L_R_select = 0;

  }
  else if ( *p == '%' ) {          /* could be a pre-defined register */
    num = 0;
    name = p;
    p = p + 1;
    c = *p;
    /* tege hack: Special case for general registers
       as the general code makes a binary search with case translation,
       and is VERY slow.  */
    if (c == 'r') {
      p++;
      if (!isdigit(*p))
        as_bad("Undefined register: '%s'. ASSUMING 0",name);
      else {
          do
            num= num*10 + *p++ - '0';
          while (isdigit(*p));
        }
      }
    else {
      while ( is_part_of_name(c) ) {
	p = p + 1;
	c = *p;
      }
      /* Terminate string with \0.  Restore below.  */
      *p = 0;
      status = reg_name_search(name);
      if ( status >= 0 )
	num = status;
      else {
        if ( print_errors )
	  as_bad("Undefined register: '%s'. ASSUMING 0",name);
	else
	  num = -1;
      }
      *p = c;
    }

    result->number_part = num;

    if ( is_R_select(p-1) )
      result->L_R_select = 1;
    else if ( is_L_select(p-1) )
      result->L_R_select = 0;
    else
      result->L_R_select = 0;

  }
  else {
    num = 0;
    name = p;
    c = *p;
    while ( is_part_of_name(c) ) {
	    p = p + 1;
	    c = *p;
    }
    *p = 0;
    if ( (sym = symbol_table_lookup(name)) != NULL ) {
      if ( sym->sy_type == N_ABS &&  sym->sy_other == NO_SECT ) {
	num = sym->sy_value;
      }
      else {
	if ( print_errors )
	  as_bad("Non-absolute constant: '%s'. ASSUMING 0",name);
	else
	  num = -1;
      }
    }
    else {
      if ( print_errors )
	as_bad("Undefined absolute constant: '%s'. ASSUMING 0",name);
      else
	num = -1;
    }
    *p = c;

    result->number_part = num;

    if ( is_R_select(p-1) ) {
      result->L_R_select = 1;
    }
    else if ( is_L_select(p-1) ) {
      result->L_R_select = 0;
    }
    else
      result->L_R_select = 0;
  }
  
  *s = p;
  return num;

}

int pa_parse_fp_cmp_cond(s)
  char **s;
{
  int cond,i;
  struct possibleS {
    char *string;
    int cond;
  };

  /* 
     This table is sorted by order of the length of the string. This is so we
     check for <> before we check for <. If we had a <> and checked for < first,
     we would get a false match.
   */
  static struct possibleS poss[] =
    {
      { "false?", 0 },
      { "false",  1 },
      { "true?",  30 },
      { "true",   31 },
      { "!<=>",   3 },
      { "!?>=",   8 },
      { "!?<=",   16 },
      { "!<>",    7 },
      { "!>=",    11 },
      { "!?>",    12 },
      { "?<=",    14 },
      { "!<=",    19 },
      { "!?<",    20 },
      { "?>=",    22 },
      { "!?=",    24 },
      { "!=t",    27 },
      { "<=>",    29 },
      { "=t",     5 },
      { "?=",     6 },
      { "?<",     10 },
      { "<=",     13 },
      { "!>",     15 },
      { "?>",     18 },
      { ">=",     21 },
      { "!<",     23 },
      { "<>",     25 },
      { "!=",     26 },
      { "!?",     28 },
      { "?",      2 },
      { "=",      4 },
      { "<",      9 },
      { ">",      17 }
    };

  cond=0;

  for ( i = 0; i < 32; i++ ) {
    if ( strncasecmp(*s,poss[i].string,strlen(poss[i].string)) == 0 ) {
      cond = poss[i].cond;
      *s += strlen(poss[i].string);
      while ( **s == ' ' || **s == '\t' )
	*s = *s + 1;
      return cond;
    }
  }

  as_bad("Illegal FP Compare Condition: %c",**s);
  return 0;
}

FP_Operand_Format pa_parse_fp_format(s)
     char **s;
{
  int f;

  f = SGL;
  if ( **s == ',' ) {
    *s += 1;
    if ( strncasecmp(*s,"sgl",3) == 0 ) {
      f = SGL;
      *s += 4;
    }
    else if ( strncasecmp(*s,"dbl",3) == 0 ) {
      f = DBL;
      *s += 4;
    }
    else if ( strncasecmp(*s,"quad",4) == 0 ) {
      f = QUAD;
      *s += 5;
    }
    else {
      f = ILLEGAL_FMT;
      as_bad("Unrecognized FP Operand Format: %3s",*s);
    }
  }
  while ( **s == ' ' || **s == '\t' || **s == 0 )
    *s = *s + 1;

  return f;
}

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
    case SEG_NONE:
    case SEG_BIG:
	break;

    default:
	the_insn.error = "illegal segment";
	expr_end = input_line_pointer;
	input_line_pointer=save_in;
	return 1;
    }
    expr_end = input_line_pointer;
    input_line_pointer = save_in;
    return 0;
}

int
getAbsoluteExpression(
char *str)
{
    char *save_in;
    segT seg;

	for ( ; *str == ' ' || *str == '\t' ; str++)
		;	/* do nothing */
    save_in = input_line_pointer;
    input_line_pointer = str;
    switch (seg = expression(&the_insn.exp)) {
    case SEG_ABSOLUTE:
		break;
    default:
		the_insn.error = "segment should be ABSOLUTE";
		expr_end = input_line_pointer;
		input_line_pointer=save_in;
		return 1;
    }
    expr_end = input_line_pointer;
    input_line_pointer = save_in;
    return 0;
}

int
evaluateAbsolute(
expressionS exp,
int field_selector)
{
	int value;
	uint32_t left21, right14;

	value = exp.X_add_number;
	calc_hppa_HILO(0, value, &left21, &right14);

	if ( exp.X_add_symbol ) {
		value += exp.X_add_symbol->sy_value;
	}
	if ( exp.X_subtract_symbol ) {
		value -= exp.X_subtract_symbol->sy_value;
	}

	switch (field_selector) {
	case /* no selector */ 0:
		break;
	case /* e_lsel */  1:	/* L` 	*/
		value = left21;
		break;

	case /* e_rsel */  2:	/* R`	*/
		value = right14;
		break;
	default:
		BAD_CASE(field_selector);
		break;
  }
  return value;
}

int pa_parse_nullif(s)
     char **s;
{
  int nullif;

  nullif = 0;
  if ( **s == ',' ) {
    *s = *s + 1;
    if ( strncasecmp(*s,"n",1) == 0 )
      nullif = 1;
    else {
      as_bad("Unrecognized Nullification: (%c)",**s);
      nullif = 0;
    }
    *s = *s + 1;
  }
  while ( **s == ' ' || **s == '\t' )
    *s = *s + 1;

  return nullif;
}

int pa_parse_nonneg_cmpsub_cmpltr(s)
     char **s;
{
  int cmpltr;
  char *name;
  char c;

  cmpltr = 0;
  if ( **s == ',' ) {
    *s+=1;
    name = *s;
    while ( **s != ',' && **s != ' ' && **s != '\t' )
      *s += 1;
    c = **s;
    **s = 0x00;
    if ( strcmp(name,"=") == 0 ) {
      cmpltr = 1;
    }
    else if ( strcmp(name,"<") == 0 ) {
      cmpltr = 2;
    }
    else if ( strcmp(name,"<=") == 0 ) {
      cmpltr = 3;
    }
    else if ( strcmp(name,"<<") == 0 ) {
      cmpltr = 4;
    }
    else if ( strcmp(name,"<<=") == 0 ) {
      cmpltr = 5;
    }
    else if ( strcasecmp(name,"sv") == 0 ) {
      cmpltr = 6;
    }
    else if ( strcasecmp(name,"od") == 0 ) {
      cmpltr = 7;
    }
    else
      cmpltr = -1;
    **s = c;
  }
  if ( cmpltr >= 0 ) {
    while ( **s == ' ' || **s == '\t' )
      *s = *s + 1;
  }

  return cmpltr;
}

int pa_parse_neg_cmpsub_cmpltr(s)
     char **s;
{
  int cmpltr;
  char *name;
  char c;

  cmpltr = -1;
  if ( **s == ',' ) {
    *s+=1;
    name = *s;
    while ( **s != ',' && **s != ' ' && **s != '\t' )
      *s += 1;
    c = **s;
    **s = 0x00;
    if ( strcasecmp(name,"tr") == 0 ) {
      cmpltr = 0;
    }
    else if ( strcmp(name,"<>") == 0 ) {
      cmpltr = 1;
    }
    else if ( strcmp(name,">=") == 0 ) {
      cmpltr = 2;
    }
    else if ( strcmp(name,">") == 0 ) {
      cmpltr = 3;
    }
    else if ( strcmp(name,">>=") == 0 ) {
      cmpltr = 4;
    }
    else if ( strcmp(name,">>") == 0 ) {
      cmpltr = 5;
    }
    else if ( strcasecmp(name,"nsv") == 0 ) {
      cmpltr = 6;
    }
    else if ( strcasecmp(name,"ev") == 0 ) {
      cmpltr = 7;
    }
    **s = c;
  }
  if ( cmpltr >= 0 ) {
    while ( **s == ' ' || **s == '\t' )
      *s = *s + 1;
  }

  return cmpltr;
}

int pa_parse_nonneg_add_cmpltr(s)
     char **s;
{
  int cmpltr;
  char *name;
  char c;

  cmpltr = -1;
  if ( **s == ',' ) {
    *s+=1;
    name = *s;
    while ( **s != ',' && **s != ' ' && **s != '\t' )
      *s += 1;
    c = **s;
    **s = 0x00;
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
    **s = c;
  }
  if ( cmpltr >= 0 ) {
    while ( **s == ' ' || **s == '\t' )
      *s = *s + 1;
  }

  return cmpltr;
}

int pa_parse_neg_add_cmpltr(s)
     char **s;
{
  int cmpltr;
  char *name;
  char c;

  cmpltr = -1;
  if ( **s == ',' ) {
    *s+=1;
    name = *s;
    while ( **s != ',' && **s != ' ' && **s != '\t' )
      *s += 1;
    c = **s;
    **s = 0x00;
    if ( strcasecmp(name,"tr") == 0 ) {
      cmpltr = 0;
    }
    else if ( strcmp(name,"<>") == 0 ) {
      cmpltr = 1;
    }
    else if ( strcmp(name,">=") == 0 ) {
      cmpltr = 2;
    }
    else if ( strcmp(name,">") == 0 ) {
      cmpltr = 3;
    }
    else if ( strcmp(name,"uv") == 0 ) {
      cmpltr = 4;
    }
    else if ( strcmp(name,"vnz") == 0 ) {
      cmpltr = 5;
    }
    else if ( strcasecmp(name,"nsv") == 0 ) {
      cmpltr = 6;
    }
    else if ( strcasecmp(name,"ev") == 0 ) {
      cmpltr = 7;
    }
    **s = c;
  }
  if ( cmpltr >= 0 ) {
    while ( **s == ' ' || **s == '\t' )
      *s = *s + 1;
  }

  return cmpltr;
}

#if 0
static int
is_same_frag(frag1P,frag2P)
     fragS *frag1P;
     fragS *frag2P;
{

  if ( frag1P == NULL )
    return (FALSE);
  else if ( frag2P == NULL )
    return (FALSE);
  else if ( frag1P == frag2P )
    return (TRUE);
  else if ( frag2P->fr_type == rs_fill && frag2P->fr_fix == 0 )
    return is_same_frag(frag1P,frag2P->fr_next);
  else if ( frag2P->fr_type == rs_align )
    return is_same_frag(frag1P,frag2P->fr_next);
  else
    return (FALSE);
}
#endif

/* end hppa-aux.c */
