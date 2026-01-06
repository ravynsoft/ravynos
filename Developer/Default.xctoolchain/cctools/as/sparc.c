/* tc-sparc.c -- Assemble for the SPARC
   Copyright (C) 1989, 1990, 1991, 1992 Free Software Foundation, Inc.

   This file is part of GAS, the GNU Assembler.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   GAS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GAS; see the file COPYING.  If not, write to
   the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. */

/* relocation type for internal assembler use only */
#define SPARC_RELOC_13 (127)
#define SPARC_RELOC_22 (126)
#define SPARC_RELOC_NONE (125)

#define cypress 1234

#undef DEBUGINSN

#include <stdio.h>
#include <ctype.h>
#include "as.h"
#include "libc.h"
#include "md.h"
#include "messages.h"
#include "symbols.h"
#include "sections.h"

/* careful, this file includes data *declarations* */
#include "sparc-opcode.h"
#include <mach-o/sparc/reloc.h>

/* From GNU ansidecl.h */
#define PARAMS(paramlist)		paramlist

/*
 * These are the default cputype and cpusubtype for the Sparc architecture.
 */
const cpu_type_t md_cputype = CPU_TYPE_SPARC;
cpu_subtype_t md_cpusubtype = CPU_SUBTYPE_SPARC_ALL;

/* This is the byte sex for the Sparc architecture */
const enum byte_sex md_target_byte_sex = BIG_ENDIAN_BYTE_SEX;

/* These characters start a comment anywhere on the line */
const char md_comment_chars[] = ";!";

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

static void sparc_ip PARAMS ((char *));

static enum sparc_architecture current_architecture = v6;
static int architecture_requested;
static int warn_on_bump;

const relax_typeS md_relax_table[1];

/* handle of the OPCODE hash table */
static struct hash_control *op_hash = NULL;

#ifdef	NeXT_MOD
static void s_proc PARAMS ((uintptr_t));
extern void s_seg PARAMS ((int));
#else	/* NeXT_MOD */
static void s_data1 PARAMS ((void));
static void s_seg PARAMS ((int));
static void s_proc PARAMS ((int));
static void s_reserve PARAMS ((int));
static void s_common PARAMS ((int));
#endif	/* NeXT_MOD */

const pseudo_typeS md_pseudo_table[] =
{
#ifdef	NeXT_MOD
  {"global", s_globl, 0},	/* Maybe we should fix compiler to use globl */
  {"proc", s_proc, 0},		/* nop??? */
  /* These are to handle SUN assembler files and point to existing handlers */
  {"empty", s_ignore, 0},
  {"ident", s_ignore, 0},
  {"optim", s_ignore, 0},
  {"skip", s_space, 0},
  {"type", s_ignore, 0},
  {"word", cons, 4},
  {"half", cons, 2},
  /* these are custom handlers for SUN SPARC assembler only */

#else	/* NeXT_MOD */
  {"seg", s_seg, 0},
  {"align", s_align_bytes, 0},	/* Defaulting is invalid (0) */
  {"common", s_common, 0},
  {"global", s_globl, 0},
  {"half", cons, 2},
  {"optim", s_ignore, 0},
  {"proc", s_proc, 0},
  {"reserve", s_reserve, 0},
  {"seg", s_seg, 0},
  {"skip", s_space, 0},
  {"word", cons, 4},
#endif	/* NeXT_MOD */
  {NULL, 0, 0},
};

const int md_short_jump_size = 4;
const int md_long_jump_size = 4;
const int md_reloc_size = 12;	/* Size of relocation record */

/* This array holds the chars that always start a comment.  If the
   pre-processor is disabled, these aren't very useful */
const char comment_chars[] = "!";	/* JF removed '|' from comment_chars */

/* This array holds the chars that only start a comment at the beginning of
   a line.  If the line seems to have the form '# 123 filename'
   .line and .file directives will appear in the pre-processed output */
/* Note that input_file.c hand checks for '#' at the beginning of the
   first line of the input file.  This is because the compiler outputs
   #NO_APP at the beginning of its output. */
/* Also note that comments started like this one will always
   work if '/' isn't otherwise defined. */
const char line_comment_chars[] = "#";

const char line_separator_chars[] = "";

/* Chars that can be used to separate mant from exp in floating point nums */
const char EXP_CHARS[] = "eE";

/* Chars that mean this number is a floating point constant */
/* As in 0f12.456 */
/* or    0d1.2345e12 */
const char FLT_CHARS[] = "rRsSfFdDxXpP";

/* Also be aware that MAXIMUM_NUMBER_OF_CHARS_FOR_FLOAT may have to be
   changed in read.c.  Ideally it shouldn't have to know about it at all,
   but nothing is ideal around here.  */

static unsigned char octal[256];
#define isoctal(c)  octal[(unsigned char) (c)]
static unsigned char toHex[256];

struct sparc_it
  {
    char *error;
    uint32_t opcode;
    nlist_t *nlistp;
    expressionS exp;
    int pcrel;
    char pcrel_reloc;	/* do relocation? */
    int reloc;
  };

struct sparc_it the_insn;

#ifdef DEBUGINSN
static void print_insn PARAMS ((struct sparc_it *insn));
#endif
static int getExpression PARAMS ((char *str));

static char *expr_end;


/*
 * Indicates a 'set' instruction which may require a either
 * of the following instructions depending on the size of the 
 * value argument:
 *
 * sethi %hi(value),reg
 *
 * or    %g0,value,reg
 *
 * sethi %hi(value),reg
 * or    reg,%lo(value),reg
 *
 */
static int special_case_set = 0;

/* s_proc and s_ignore are included for rudimentary 
   compatibility with the Sun assembler only */

static void
s_proc (
uintptr_t ignore)
{
  totally_ignore_line();
}

/* This function is called once, at assembler startup time.  It should
   set up all the tables, etc. that the MD part of the assembler will need. */
void
md_begin ()
{
  register const char *retval = NULL;
  int lose = 0;
  register int i = 0;

  op_hash = hash_new ();

  while (i < NUMOPCODES)
    {
      const char *name = sparc_opcodes[i].name;
      retval = hash_insert (op_hash, (char *)name, (char *)&sparc_opcodes[i]);

      if(retval != NULL && *retval != '\0') {
	fprintf (stderr, "internal error: can't hash `%s': %s\n",
		 sparc_opcodes[i].name, retval);
	lose = 1;
      } do {
	if (sparc_opcodes[i].match & sparc_opcodes[i].lose)
	  {
	    fprintf (stderr, "internal error: losing opcode: `%s' \"%s\"\n",
		     sparc_opcodes[i].name, sparc_opcodes[i].args);
	    lose = 1;
	  }
	++i;
      }
      while (i < NUMOPCODES
	     && !strcmp (sparc_opcodes[i].name, name));
    }

  if (lose)
    as_fatal ("Broken assembler.  No assembly attempted.");

  for (i = '0'; i < '8'; ++i)
    octal[i] = 1;
  for (i = '0'; i <= '9'; ++i)
    toHex[i] = i - '0';
  for (i = 'a'; i <= 'f'; ++i)
    toHex[i] = i + 10 - 'a';
  for (i = 'A'; i <= 'F'; ++i)
    toHex[i] = i + 10 - 'A';
}

void
md_end(
void)
{
	return;
}

void
md_assemble (str)
     char *str;
{
  char *toP;
  int rsd;

  know (str);
  sparc_ip (str);

#ifdef DEBUGINSN
  print_insn(&the_insn);
#endif

  /* See if "set" operand is absolute and small; skip sethi if so. */
  if (special_case_set && the_insn.exp.X_seg == SEG_ABSOLUTE)
    {
      if (the_insn.exp.X_add_number >= -(1 << 12)
	  && the_insn.exp.X_add_number < (1 << 12))
	{
	  the_insn.opcode = 0x80102000	/* or %g0,imm,... */
	    | (the_insn.opcode & 0x3E000000)	/* dest reg */
	    | (the_insn.exp.X_add_number & 0x1FFF);	/* imm */
	  special_case_set = 0;	/* No longer special */
	  the_insn.reloc = SPARC_RELOC_NONE;	/* No longer relocated */
	}
    }

#ifdef NeXT_MOD	/* mark sections containing instructions */
  /*
   * We are putting a machine instruction in this section so mark it as
   * containg some machine instructions.
   */
  frchain_now->frch_section.flags |= S_ATTR_SOME_INSTRUCTIONS;
#endif /* NeXT_MOD */

  toP = frag_more (4);
  /* put out the opcode */
  md_number_to_chars (toP, (valueT) the_insn.opcode, 4);

  /* put out the symbol-dependent stuff */
  if (the_insn.reloc != SPARC_RELOC_NONE)
    {
	fix_new(frag_now,
		(toP - frag_now->fr_literal),
		4,
		the_insn.exp.X_add_symbol,
		the_insn.exp.X_subtract_symbol,
		the_insn.exp.X_add_number,
		the_insn.pcrel,
		the_insn.pcrel_reloc,	/* 1 for local labels due to scatter loading */
		the_insn.reloc);
    }

  if (special_case_set) {
    special_case_set = 0;
    assert (the_insn.reloc == SPARC_RELOC_HI22);
    /* See if "set" operand has no low-order bits; skip OR if so. */
    if ((the_insn.exp.X_seg == SEG_ABSOLUTE) && 
	((the_insn.exp.X_add_number & 0x3FF) == 0))
      return;

    toP = frag_more (4);
    rsd = (the_insn.opcode >> 25) & 0x1f;
    the_insn.opcode = 0x80102000 | (rsd << 25) | (rsd << 14);
    md_number_to_chars (toP, (valueT) the_insn.opcode, 4);
    the_insn.pcrel_reloc = 0;

    fix_new(frag_now,
	(toP - frag_now->fr_literal),
	4,
	the_insn.exp.X_add_symbol,
	the_insn.exp.X_subtract_symbol,
	the_insn.exp.X_add_number,
	the_insn.pcrel,
	the_insn.pcrel_reloc,
	SPARC_RELOC_LO10);
    return;
  }
}

static void
sparc_ip (str)
     char *str;
{
  char *error_message = "";
  char *s;
  const char *args;
  char c;
  struct sparc_opcode *insn;
  char *argsStart;
  uint32_t opcode;
  unsigned int mask = 0;
  int match = 0;
  int comma = 0;
  int32_t immediate_max = 0;

  for (s = str; islower (*s) || (*s >= '0' && *s <= '3'); ++s)
    ;
  switch (*s)
    {

    case '\0':
      break;

    case ',':
      comma = 1;

      /*FALLTHROUGH */

    case ' ':
      *s++ = '\0';
      break;

    default:
      as_bad ("Unknown opcode: `%s'", str);
      exit (1);
    }
  if ((insn = (struct sparc_opcode *) hash_find (op_hash, str)) == NULL)
    {
      as_bad ("Unknown opcode: `%s'", str);
      return;
    }
  if (comma)
    {
      *--s = ',';
    }
  argsStart = s;
  for (;;)
    {
      opcode = insn->match;
      memset (&the_insn, '\0', sizeof (the_insn));
      the_insn.reloc = SPARC_RELOC_NONE;
      the_insn.pcrel_reloc = 1; /* default, reloc, for scatter loading */

      /*
       * Build the opcode, checking as we go to make
       * sure that the operands match
       */
      for (args = insn->args;; ++args)
	{
	  switch (*args)
	    {
	    case 'M':
	    case 'm':
	      if (strncmp (s, "%asr", 4) == 0)
		{
		  s += 4;

		  if (isdigit (*s))
		    {
		      int32_t num = 0;

		      while (isdigit (*s))
			{
			  num = num * 10 + *s - '0';
			  ++s;
			}

		      if (num < 16 || 31 < num)
			{
			  error_message = ": asr number must be between 15 and 31";
			  goto error;
			}	/* out of range */

		      opcode |= (*args == 'M' ? RS1 (num) : RD (num));
		      continue;
		    }
		  else
		    {
		      error_message = ": expecting %asrN";
		      goto error;
		    }		/* if %asr followed by a number. */

		}		/* if %asr */
	      break;


	    case '\0':		/* end of args */
	      if (*s == '\0')
		{
		  match = 1;
		}
	      break;

	    case '+':
	      if (*s == '+')
		{
		  ++s;
		  continue;
		}
	      if (*s == '-')
		{
		  continue;
		}
	      break;

	    case '[':		/* these must match exactly */
	    case ']':
	    case ',':
	    case ' ':
	      if (*s++ == *args)
		continue;
	      break;

	    case '#':		/* must be at least one digit */
	      if (isdigit (*s++))
		{
		  while (isdigit (*s))
		    {
		      ++s;
		    }
		  continue;
		}
	      break;

	    case 'C':		/* coprocessor state register */
	      if (strncmp (s, "%csr", 4) == 0)
		{
		  s += 4;
		  continue;
		}
	      break;

	    case 'b':		/* next operand is a coprocessor register */
	    case 'c':
	    case 'D':
	      if (*s++ == '%' && *s++ == 'c' && isdigit (*s))
		{
		  mask = *s++;
		  if (isdigit (*s))
		    {
		      mask = 10 * (mask - '0') + (*s++ - '0');
		      if (mask >= 32)
			{
			  break;
			}
		    }
		  else
		    {
		      mask -= '0';
		    }
		  switch (*args)
		    {

		    case 'b':
		      opcode |= mask << 14;
		      continue;

		    case 'c':
		      opcode |= mask;
		      continue;

		    case 'D':
		      opcode |= mask << 25;
		      continue;
		    }
		}
	      break;

	    case 'r':		/* next operand must be a register */
	    case 'u':
	    case '1':
	    case '2':
	    case 'd':
	      if (*s++ == '%')
		{
		  switch (c = *s++)
		    {

		    case 'f':	/* frame pointer */
		      if (*s++ == 'p')
			{
			  mask = 0x1e;
			  break;
			}
		      error_message = ": register not fp";
		      goto error;

		    case 'g':	/* global register */
		      if (isoctal (c = *s++))
			{
			  mask = c - '0';
			  break;
			}
		      error_message = ": invalid global register";
		      goto error;

		    case 'i':	/* in register */
		      if (isoctal (c = *s++))
			{
			  mask = c - '0' + 24;
			  break;
			}
		      error_message = ": invalid in register";
		      goto error;

		    case 'l':	/* local register */
		      if (isoctal (c = *s++))
			{
			  mask = (c - '0' + 16);
			  break;
			}
		      error_message = ": invalid local register";
		      goto error;

		    case 'o':	/* out register */
		      if (isoctal (c = *s++))
			{
			  mask = (c - '0' + 8);
			  break;
			}
		      error_message = ": invalid out register";
		      goto error;

		    case 's':	/* stack pointer */
		      if (*s++ == 'p')
			{
			  mask = 0xe;
			  break;
			}
		      error_message = ": register is not sp";
		      goto error;

		    case 'r':	/* any register */
		      if (!isdigit (c = *s++))
			{
			  error_message = ": invalid register";
			  goto error;
			}
		      /* FALLTHROUGH */
		    case '0':
		    case '1':
		    case '2':
		    case '3':
		    case '4':
		    case '5':
		    case '6':
		    case '7':
		    case '8':
		    case '9':
		      if (isdigit (*s))
			{
			  if ((c = 10 * (c - '0') + (*s++ - '0')) >= 32)
			    {
			      error_message = ": register # out of range";
			      goto error;
			    }
			}
		      else
			{
			  c -= '0';
			}
		      mask = c;
		      break;

		    default:
		      error_message = ": invalid resgiter #";
		      goto error;
		    }
		 /*
		 * Got the register, now figure out where
		 * it goes in the opcode.
		 */
		  switch (*args)
		    {

		    case '1':
		      opcode |= mask << 14;
		      continue;

		    case '2':
		      opcode |= mask;
		      continue;

		    case 'd':
		      opcode |= mask << 25;
		      continue;

		    case 'r':
		      opcode |= (mask << 25) | (mask << 14);
		      continue;

		    case 'u':
		      opcode |= (mask << 25) | mask;
		      continue;
		    }
		}
	      break;

	    case 'e':		/* next operand is a floating point register */
	    case 'v':
	    case 'V':

	    case 'f':
	    case 'B':
	    case 'R':

	    case 'g':
	    case 'H':
	    case 'J':
	      {
		char format;

		if (*s++ == '%'
		    && ((format = *s) == 'f')
		    && isdigit (*++s))
		  {
		    for (mask = 0; isdigit (*s); ++s)
		      {
			mask = 10 * mask + (*s - '0');
		      }		/* read the number */

		    if ((*args == 'v'
			 || *args == 'B'
			 || *args == 'H')
			&& (mask & 1))
		      {
			break;
		      }		/* register must be even numbered */

		    if ((*args == 'V'
			 || *args == 'R'
			 || *args == 'J')
			&& (mask & 3))
		      {
			break;
		      }		/* register must be multiple of 4 */

		    if (mask >= 32)
		      {
			error_message = ": There are only 32 f registers; [0-31]";
			goto error;
		      }	/* on error */
		  }
		else
		  {
		    break;
		  }	/* if not an 'f' register. */

		switch (*args)
		  {

		  case 'v':
		  case 'V':
		  case 'e':
		    opcode |= RS1 (mask);
		    continue;


		  case 'f':
		  case 'B':
		  case 'R':
		    opcode |= RS2 (mask);
		    continue;

		  case 'g':
		  case 'H':
		  case 'J':
		    opcode |= RD (mask);
		    continue;
		  }		/* pack it in. */

		know (0);
		break;
	      }			/* float arg */

	    case 'F':
	      if (strncmp (s, "%fsr", 4) == 0)
		{
		  s += 4;
		  continue;
		}
	      break;

	    case 'h':		/* high 22 bits */
	      the_insn.reloc = SPARC_RELOC_HI22;
	      goto immediate;

	    case 'l':		/* 22 bit PC relative immediate */
	      the_insn.reloc = SPARC_RELOC_WDISP22;
	      the_insn.pcrel = 1;
	      goto immediate;

	    case 'L':		/* 30 bit immediate for call insn */
	      the_insn.reloc = SPARC_RELOC_WDISP30;
	      the_insn.pcrel = 1;
	      goto immediate;

	    case 'n':		/* 22 bit immediate */
	      the_insn.reloc = SPARC_RELOC_22;
	      goto immediate;

	    case 'i':		/* 13 bit immediate */
	      /* What's the difference between base13 and 13?  
	         13-bit immediate and 13-bit immediate+register */
	      the_insn.reloc = SPARC_RELOC_13;
	      immediate_max = 0x1FFF;

	      /*FALLTHROUGH */

	    immediate:
	      if (*s == ' ')
		s++;
	      if (*s == '%')
		{
		  if ((c = s[1]) == 'h' && s[2] == 'i')
		    {
		      the_insn.reloc = SPARC_RELOC_HI22;
		      s += 3;
		    }
		  else if (c == 'l' && s[2] == 'o')
		    {
		      the_insn.reloc = SPARC_RELOC_LO10;
		      s += 3;
		    }
		  else
		    break;
		}
	      /* Note that if the getExpression() fails, we will still
		 have created U entries in the symbol table for the
		 'symbols' in the input string.  Try not to create U
		 symbols for registers, etc.  */
	      {
		/* This stuff checks to see if the expression ends in
		   +%reg.  If it does, it removes the register from
		   the expression, and re-sets 's' to point to the
		   right place.  */

		char *s1;

		for (s1 = s; *s1 && *s1 != ',' && *s1 != ']'; s1++);

		if (s1 != s && isdigit (s1[-1]))
		  {
		    if (s1[-2] == '%' && s1[-3] == '+')
		      {
			s1 -= 3;
			*s1 = '\0';
			(void) getExpression (s);
			*s1 = '+';
			s = s1;
			continue;
		      }
		    else if (strchr ("goli0123456789", 
				     s1[-2]) && s1[-3] == '%' && s1[-4] == '+')
		      {
			s1 -= 4;
			*s1 = '\0';
			(void) getExpression (s);
			*s1 = '+';
			s = s1;
			continue;
		      }
		  }
	      }
	      (void) getExpression (s);
	      s = expr_end;
	/* The Next linker has the ability to scatter blocks of sections between
	 * labels.  This requires that branches to labels that survive to the
	 * link phase be relocatable.  These labels are those that are not L*
	 */
	     if (the_insn.exp.X_add_symbol != NULL && !flagseen['L']
		&& the_insn.exp.X_add_symbol->sy_name[0] == 'L') {
		/* local symbol which will be thrown away.  Don't bother
		 * to reloc it.
		 */
		the_insn.pcrel_reloc = 0;
	     }

	      if ((the_insn.exp.X_seg == SEG_ABSOLUTE || 
		   the_insn.exp.X_seg == SEG_BIG)
		  && the_insn.exp.X_add_symbol == 0)
		{

		  /* Check for invalid constant values.  Don't warn if
		     constant was inside %hi or %lo, since these
		     truncate the constant to fit.  */
		  if (immediate_max != 0
		      && the_insn.reloc != SPARC_RELOC_LO10
		      && the_insn.reloc != SPARC_RELOC_HI22
		      && (the_insn.exp.X_add_number > immediate_max
			  || the_insn.exp.X_add_number < ~immediate_max))
		    as_bad ("constant value must be between %d and %d",
			    ~immediate_max, immediate_max);

		  if ((the_insn.reloc == SPARC_RELOC_WDISP22 ||
		      the_insn.reloc == SPARC_RELOC_WDISP30) &&
		      the_insn.exp.X_add_number & 3) 
		    as_bad ("displacement is not long aligned");

 		  /* plug absolutes directly into opcode */

		  switch(the_insn.reloc) {
		  case SPARC_RELOC_13:
		    if (the_insn.exp.X_seg == SEG_BIG)
		      opcode |= (*(int *) generic_bignum) & 0x1fff;
		    else
		      opcode |= the_insn.exp.X_add_number & 0x1fff;
		    the_insn.reloc = SPARC_RELOC_NONE;
		    break;
		  case SPARC_RELOC_22:
		    if (the_insn.exp.X_seg == SEG_BIG)
		      opcode |= (*(int *) generic_bignum) & 0x3fffff;
		    else
		      opcode |= the_insn.exp.X_add_number & 0x3fffff;
		    the_insn.reloc = SPARC_RELOC_NONE;
		    break;
		  case SPARC_RELOC_HI22:
		    /* extract upper 22 bits from constant */
		    opcode |= (the_insn.exp.X_add_number >> 10) & 0x3fffff;
		    the_insn.reloc = SPARC_RELOC_NONE;
		    break;
		  case SPARC_RELOC_LO10:
		    opcode |= the_insn.exp.X_add_number & 0x3ff;
		    break;
	      
		    /* the PC relative displacements are plugged in
		       if the argument is absolute, but retain
		       relocatability */
		  case SPARC_RELOC_WDISP22:
		    opcode |= (the_insn.exp.X_add_number >> 2) & 0x3fffff;
		    break;
		  case SPARC_RELOC_WDISP30:
		    opcode |= (the_insn.exp.X_add_number >> 2) & 0x3fffffff;
		    break;
		  default:
		    printf("Unknown reloc entry\n");
		  }
		}

	      /* Reset to prevent extraneous range check.  */
	      immediate_max = 0;

	      continue;

	    case 'a':
	      if (*s++ == 'a')
		{
		  opcode |= ANNUL;
		  continue;
		}
	      break;

	    case 'A':
	      {
		char *push = input_line_pointer;
		expressionS e;

		input_line_pointer = s;

		expression (&e);

		if (e.X_seg == SEG_ABSOLUTE)
		  {
		    opcode |= e.X_add_number << 5;
		    s = input_line_pointer;
		    input_line_pointer = push;
		    continue;
		  }		/* if absolute */

		break;
	      }			/* alternate space */

	    case 'p':
	      if (strncmp (s, "%psr", 4) == 0)
		{
		  s += 4;
		  continue;
		}
	      break;

	    case 'q':		/* floating point queue */
	      if (strncmp (s, "%fq", 3) == 0)
		{
		  s += 3;
		  continue;
		}
	      break;

	    case 'Q':		/* coprocessor queue */
	      if (strncmp (s, "%cq", 3) == 0)
		{
		  s += 3;
		  continue;
		}
	      break;

	    case 'S':
	      if (strcmp (str, "set") == 0)
		{
		  special_case_set = 1;
		  continue;
		}
	      break;


	    case 't':
	      if (strncmp (s, "%tbr", 4) != 0)
		break;
	      s += 4;
	      continue;

	    case 'w':
	      if (strncmp (s, "%wim", 4) != 0)
		break;
	      s += 4;
	      continue;

	    case 'y':
	      if (strncmp (s, "%y", 2) != 0)
		break;
	      s += 2;
	      continue;

	    default:
	      as_fatal ("failed sanity check.");
	    }			/* switch on arg code */
	  break;
	}			/* for each arg that we expect */
    error:
      if (match == 0)
	{
	  /* Args don't match. */
	  if (((int) (&insn[1] - sparc_opcodes)) < NUMOPCODES
	      && !strcmp (insn->name, insn[1].name))
	    {
	      ++insn;
	      s = argsStart;
	      continue;
	    }
	  else
	    {
	      as_bad ("Illegal operands%s", error_message);
	      return;
	    }
	}
      else
	{
	  if (insn->architecture > current_architecture)
	    {
	      if ((!architecture_requested || warn_on_bump)
		  &&
		  1
		)
		{
		  if (warn_on_bump)
		    {
		      as_warn ("architecture bumped from \"%s\" to \"%s\" on \"%s\"",
			       architecture_pname[current_architecture],
			       architecture_pname[insn->architecture],
			       str);
		    }		/* if warning */

		  current_architecture = insn->architecture;
		}
	      else
		{
		  as_bad ("architecture mismatch on \"%s\" (\"%s\").  current architecture is \"%s\"",
			  str,
			  architecture_pname[insn->architecture],
			  architecture_pname[current_architecture]);
		  return;
		}		/* if bump ok else error */
	    }			/* if architecture higher */
	}			/* if no match */

      break;
    }				/* forever looking for a match */

  the_insn.opcode = opcode;
}

static int
getExpression (str)
     char *str;
{
  char *save_in;
  segT seg;

  save_in = input_line_pointer;
  input_line_pointer = str;
  seg = expression (&the_insn.exp);

  if (seg != SEG_ABSOLUTE
      && seg != SEG_SECT
      && seg != SEG_DIFFSECT
      && seg != SEG_UNKNOWN
      && seg != SEG_NONE
      && seg != SEG_BIG) {
    the_insn.error = "bad segment";
    expr_end = input_line_pointer;
    input_line_pointer = save_in;
    return 1;
  }
  expr_end = input_line_pointer;
  input_line_pointer = save_in;
  return 0;
}				/* getExpression() */


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
md_atof (type, litP, sizeP)
     char type;
     char *litP;
     int *sizeP;
{
  int prec;
  LITTLENUM_TYPE words[MAX_LITTLENUMS];
  LITTLENUM_TYPE *wordP;
  char *t;
  char *atof_ieee ();

  switch (type)
    {
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
      *sizeP = 0;
      return "Bad call to MD_ATOF()";
    }
  t = atof_ieee (input_line_pointer, type, words);
  if (t)
    input_line_pointer = t;
  *sizeP = prec * sizeof (LITTLENUM_TYPE);
  for (wordP = words; prec--;)
    {
      md_number_to_chars (litP, (valueT) (*wordP++), sizeof (LITTLENUM_TYPE));
      litP += sizeof (LITTLENUM_TYPE);
    }
  return "";
}

/*
 * Write out big-endian.
 */
void
md_number_to_chars (buf, val, n)
     char *buf;
     signed_expr_t val;
     int n;
{
  // sigh, all architectures do this..,
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
}

/* Apply a fixS to the frags, now that we know the value it ought to
   hold. */


void
md_number_to_imm(unsigned char *buf, signed_expr_t val, int size, fixS *fixP, int nsect)
{

  /* handle the most common case quickly */
  if ((fixP->fx_r_type == NO_RELOC) ||
      (fixP->fx_r_type == SPARC_RELOC_NONE) ||
      (fixP->fx_r_type == SPARC_RELOC_VANILLA)) {
    switch(size){
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
  case SPARC_RELOC_WDISP30:
    val = (val >> 2) + 1;	/* adjust for word displacement */
    buf[0] |= (val >> 24) & 0x3f;
    buf[1] = (val >> 16);
    buf[2] = val >> 8;
    buf[3] = val;
    break;
  case SPARC_RELOC_WDISP22:
    val = (val >> 2) + 1;
    buf[1] |= (val >> 16) & 0x3f;
    buf[2] = val >> 8;
    buf[3] = val;
    break;
  case SPARC_RELOC_HI22:
    buf[1] |= (val >> 26) & 0x3f;
    buf[2] = val >> 18;
    buf[3] = val >> 10;
    break;
  case SPARC_RELOC_LO10:
    buf[2] |= (val >> 8) & 0x03;
    buf[3] = val;
    break;

  /* special cases that need to be handled internally by the as */
  case SPARC_RELOC_22:
    if (!fixP->fx_addsy) {
      if (val & ~0x003fffff) {
	as_bad ("relocation overflow");
      }			/* on overflow */
      buf[1] |= (val >> 16) & 0x3f;
      buf[2] = val >> 8;
      buf[3] = val & 0xff;
    } else
      as_bad ("Undefined symbolic 22-bit immediate reference: %s", 
	      fixP->fx_addsy->sy_name);
    break;
  case SPARC_RELOC_13:
    if (!fixP->fx_addsy) {
      if (((val > 0) && (val & ~(offsetT)0x00001fff))
	  || ((val < 0) && (~(val - 1) & ~(offsetT)0x00001fff))) {
	as_bad ("relocation overflow");
      }
      buf[2] |= (val >> 8) & 0x1f;
      buf[3] = val;
    } else
      as_bad ("Undefined symbolic 13-bit immediate reference: %s", 
	      fixP->fx_addsy->sy_name);
    break;
  case SPARC_RELOC_NONE:
  default:
    as_bad ("bad or unhandled relocation type: 0x%02x", fixP->fx_r_type);
    break;
  }
}


/*
 * md_parse_option
 *	Invocation line includes a switch not recognized by the base assembler.
 *	See if it's a processor-specific option.  These are:
 *
 *	-bump
 *		Warn on architecture bumps.  See also -A.
 *
 *	-Av6, -Av7, -Av8, -Asparclite
 *		Select the architecture.  Instructions or features not
 *		supported by the selected architecture cause fatal errors.
 *
 *		The default is to start at v6, and bump the architecture up
 *		whenever an instruction is seen at a higher level.
 *
 *		If -bump is specified, a warning is printing when bumping to
 *		higher levels.
 *
 *		If an architecture is specified, all instructions must match
 *		that architecture.  Any higher level instructions are flagged
 *		as errors.
 *
 *		if both an architecture and -bump are specified, the
 *		architecture starts at the specified level, but bumps are
 *		warnings.
 *
 */

int 
md_parse_option (argP, cntP, vecP)
     char **argP;
     int *cntP;
     char ***vecP;
{
  char *p;
  const char **arch;

  if (!strcmp (*argP, "bump"))
    {
      warn_on_bump = 1;
    }
  else if (**argP == 'A')
    {
      p = (*argP) + 1;

      for (arch = architecture_pname; *arch != NULL; ++arch)
	{
	  if (strcmp (p, *arch) == 0)
	    {
	      break;
	    }			/* found a match */
	}			/* walk the pname table */

      if (*arch == NULL)
	{
	  as_bad ("unknown architecture: %s", p);
	}
      else
	{
	  current_architecture = (enum sparc_architecture) (arch - architecture_pname);
	  architecture_requested = 1;
	}
    }
#ifndef NeXT_MOD
#ifdef OBJ_ELF
  else if (**argP == 'V')
    {
      print_version_id ();
    }
  else if (**argP == 'Q')
    {
      /* Qy - do emit .comment
	 Qn - do not emit .comment */
    }
  else if (**argP == 's')
    {
      /* use .stab instead of .stab.excl */
    }
#endif
  else if (strcmp (*argP, "sparc") == 0)
    {
      /* Ignore -sparc, used by SunOS make default .s.o rule.  */
    }
#endif /* NeXT_MOD */
  else
    {
      /* Unknown option */
      (*argP)++;
      return 0;
    }
  **argP = '\0';		/* Done parsing this switch */
  return 1;
}				/* md_parse_option() */


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


#ifdef DEBUGINSN

char *
S_GET_NAME(sym)
     symbolS *sym;
{
  return (sym->sy_nlist.n_un.n_name);
}

/* for debugging only */
static void
print_insn (insn)
     struct sparc_it *insn;
{
  const char *const Reloc[] = {
    "VANILLA",
    "PAIR",
    "HI22",
    "LO10",
    "DISP22",
    "PCREL",
    "22",
    "13",
    "SECTDIFF",
    "HI22_SECTDIFF",
    "LO10_SECTDIFF",
    "NONE",
    "UNUSED"
  };

  const char *const InternalReloc[] = {
    "13",
    "22"
  };

  if (insn->error)
    fprintf (stderr, "ERROR: %s\n", insn->error);
  fprintf (stderr, "opcode=0x%08x\n", (unsigned int)insn->opcode);
  if (insn->reloc >= 127)
    fprintf (stderr, "internal reloc = %s\n", InternalReloc[insn->reloc-127]);
  else
    fprintf (stderr, "reloc = %s\n", Reloc[insn->reloc]);
  fprintf (stderr, "X_add_number = 0x%x\n",
	   insn->exp.X_add_number);

  if (insn->exp.X_add_symbol != NULL)
    fprintf(stderr, "Add symbol: %s\n", S_GET_NAME(insn->exp.X_add_symbol));
  if (insn->exp.X_subtract_symbol != NULL)
    fprintf(stderr, "Subtract symbol: %s\n", S_GET_NAME(insn->exp.X_subtract_symbol));
}
#endif
