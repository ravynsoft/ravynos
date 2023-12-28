/* DO NOT EDIT!  -*- buffer-read-only: t -*- vi:set ro:  */
/* Assembler interface for targets using CGEN. -*- C -*-
   CGEN: Cpu tools GENerator

   THIS FILE IS MACHINE GENERATED WITH CGEN.
   - the resultant file is machine generated, cgen-asm.in isn't

   Copyright (C) 1996-2023 Free Software Foundation, Inc.

   This file is part of libopcodes.

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   It is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.  */


/* ??? Eventually more and more of this stuff can go to cpu-independent files.
   Keep that in mind.  */

#include "sysdep.h"
#include <stdio.h>
#include "ansidecl.h"
#include "bfd.h"
#include "symcat.h"
#include "epiphany-desc.h"
#include "epiphany-opc.h"
#include "opintl.h"
#include "xregex.h"
#include "libiberty.h"
#include "safe-ctype.h"

#undef  min
#define min(a,b) ((a) < (b) ? (a) : (b))
#undef  max
#define max(a,b) ((a) > (b) ? (a) : (b))

static const char * parse_insn_normal
  (CGEN_CPU_DESC, const CGEN_INSN *, const char **, CGEN_FIELDS *);

/* -- assembler routines inserted here.  */

/* -- asm.c */
const char *
parse_shortregs (CGEN_CPU_DESC cd,
		 const char ** strp,
		 CGEN_KEYWORD * keywords,
		 long * regno)
{
  const char * errmsg;

  /* Parse register.  */
  errmsg = cgen_parse_keyword (cd, strp, keywords, regno);

  if (errmsg)
    return errmsg;

  if (*regno > 7)
    errmsg = _("register unavailable for short instructions");

  return errmsg;
}

static const char * parse_simm_not_reg (CGEN_CPU_DESC, const char **, int,
					long *);

static const char *
parse_uimm_not_reg (CGEN_CPU_DESC cd,
		    const char ** strp,
		    int opindex,
		    unsigned long * valuep)
{
  long * svalp = (void *) valuep;
  return parse_simm_not_reg (cd, strp, opindex, svalp);
}

/* Handle simm3/simm11/imm3/imm12.  */

static const char *
parse_simm_not_reg (CGEN_CPU_DESC cd,
		   const char ** strp,
		   int opindex,
		   long * valuep)
{
  const char * errmsg;

  int   sign = 0;
  int   bits = 0;

  switch (opindex)
    {
    case EPIPHANY_OPERAND_SIMM3:
      sign = 1; bits = 3; break;
    case EPIPHANY_OPERAND_SIMM11:
      sign = 1; bits = 11; break;
    case EPIPHANY_OPERAND_DISP3:
      sign = 0; bits = 3; break;
    case EPIPHANY_OPERAND_DISP11:
      /* Load/store displacement is a sign-magnitude 12 bit value.  */
      sign = 0; bits = 11; break;
    }

  /* First try to parse as a register name and reject the operand.  */
  errmsg = cgen_parse_keyword (cd, strp, & epiphany_cgen_opval_gr_names,valuep);
  if (!errmsg)
    return _("register name used as immediate value");

  errmsg = (sign ? cgen_parse_signed_integer (cd, strp, opindex, valuep)
	    : cgen_parse_unsigned_integer (cd, strp, opindex,
					  (unsigned long *) valuep));
  if (errmsg)
    return errmsg;

  if (sign)
    errmsg = cgen_validate_signed_integer (*valuep,
					  -((1L << bits) - 1), (1 << (bits - 1)) - 1);
  else
    errmsg = cgen_validate_unsigned_integer (*valuep, 0, (1L << bits) - 1);

  return errmsg;
}

static const char *
parse_postindex (CGEN_CPU_DESC cd ATTRIBUTE_UNUSED,
		 const char ** strp,
		 int opindex ATTRIBUTE_UNUSED,
		 unsigned long *valuep)
{
  if (**strp == '#')
    ++*strp;			/* Skip leading hashes.  */

  if (**strp == '-')
    {
      *valuep = 1;
      ++*strp;
    }
  else if (**strp == '+')
    {
      *valuep = 0;
      ++*strp;
    }
  else
    *valuep = 0;

  return NULL;
}

static const char *
parse_imm8 (CGEN_CPU_DESC cd,
	    const char ** strp,
	    int opindex,
	    bfd_reloc_code_real_type code,
	    enum cgen_parse_operand_result * result_type,
	    bfd_vma * valuep)
{
  const char * errmsg;
  enum cgen_parse_operand_result rt;
  long dummyval;

  if (!result_type)
    result_type = &rt;

  code = BFD_RELOC_NONE;

  if (!cgen_parse_keyword (cd, strp, &epiphany_cgen_opval_gr_names, &dummyval)
      || !cgen_parse_keyword (cd, strp, &epiphany_cgen_opval_cr_names,
			      &dummyval))
    /* Don't treat "mov ip,ip" as a move-immediate.  */
    return _("register source in immediate move");

  errmsg = cgen_parse_address (cd, strp, opindex, code, result_type, valuep);
  if (errmsg)
    return errmsg;

  if (*result_type == CGEN_PARSE_OPERAND_RESULT_NUMBER)
    errmsg = cgen_validate_unsigned_integer (*valuep, 0, 0xff);
  else
    errmsg = _("byte relocation unsupported");

  *valuep &= 0xff;
  return errmsg;
}

static const char * MISSING_CLOSE_PARENTHESIS = N_("missing `)'");

static const char *
parse_imm16 (CGEN_CPU_DESC cd,
	     const char ** strp,
	     int opindex,
	     bfd_reloc_code_real_type code ATTRIBUTE_UNUSED,
	     enum cgen_parse_operand_result * result_type,
	     bfd_vma * valuep)
{
  const char * errmsg;
  enum cgen_parse_operand_result rt;
  long dummyval;

  if (!result_type)
    result_type = &rt;

  if (strncasecmp (*strp, "%high(", 6) == 0)
    {
      *strp += 6;
      errmsg = cgen_parse_address (cd, strp, opindex, BFD_RELOC_EPIPHANY_HIGH,
				   result_type, valuep);
      if (**strp != ')')
	return MISSING_CLOSE_PARENTHESIS;
      ++*strp;
      *valuep >>= 16;
    }
  else if (strncasecmp (*strp, "%low(", 5) == 0)
    {
      *strp += 5;
      errmsg = cgen_parse_address (cd, strp, opindex, BFD_RELOC_EPIPHANY_LOW,
				   result_type, valuep);
      if (**strp != ')')
	return MISSING_CLOSE_PARENTHESIS;
      ++*strp;
    }
  else if (!cgen_parse_keyword (cd, strp, &epiphany_cgen_opval_gr_names,
				&dummyval)
	   || !cgen_parse_keyword (cd, strp, &epiphany_cgen_opval_cr_names,
				   &dummyval))
    /* Don't treat "mov ip,ip" as a move-immediate.  */
    return _("register source in immediate move");
  else
    errmsg = cgen_parse_address (cd, strp, opindex, BFD_RELOC_16,
				 result_type, valuep);

  if (!errmsg && result_type == CGEN_PARSE_OPERAND_RESULT_NUMBER)
    errmsg = cgen_validate_unsigned_integer (*valuep, 0, 0xffff);

  *valuep &= 0xffff;
  return errmsg;
}

const char *
parse_branch_addr (CGEN_CPU_DESC cd,
		   const char ** strp,
		   int opindex,
		   int opinfo ATTRIBUTE_UNUSED,
		   enum cgen_parse_operand_result * resultp ATTRIBUTE_UNUSED,
		   bfd_vma *valuep ATTRIBUTE_UNUSED)
{
  const char * errmsg;
  enum cgen_parse_operand_result result_type;
  bfd_reloc_code_real_type code = BFD_RELOC_NONE;
  bfd_vma value;

  switch (opindex)
    {
    case EPIPHANY_OPERAND_SIMM24:
      code = BFD_RELOC_EPIPHANY_SIMM24;
      break;

    case EPIPHANY_OPERAND_SIMM8:
      code = BFD_RELOC_EPIPHANY_SIMM8;
      break;

    default:
      errmsg = _("ABORT: unknown operand");
      return errmsg;
    }

  errmsg = cgen_parse_address (cd, strp, opindex, code,
			       &result_type, &value);
  if (errmsg == NULL)
    {
      if (result_type == CGEN_PARSE_OPERAND_RESULT_NUMBER)
	{
	  /* Act as if we had done a PC-relative branch, ala .+num.  */
	  char buf[20];
	  const char * bufp = (const char *) buf;

	  sprintf (buf, ".+%ld", (long) value);
	  errmsg = cgen_parse_address (cd, &bufp, opindex, code, &result_type,
				       &value);
	}

      if (result_type == CGEN_PARSE_OPERAND_RESULT_QUEUED)
	{
	  /* This will happen for things like (s2-s1) where s2 and s1
	     are labels.  */
	  /* Nothing further to be done.  */
	}
      else
	errmsg = _("Not a pc-relative address.");
    }
  return errmsg;
}

/* -- dis.c */

const char * epiphany_cgen_parse_operand
  (CGEN_CPU_DESC, int, const char **, CGEN_FIELDS *);

/* Main entry point for operand parsing.

   This function is basically just a big switch statement.  Earlier versions
   used tables to look up the function to use, but
   - if the table contains both assembler and disassembler functions then
     the disassembler contains much of the assembler and vice-versa,
   - there's a lot of inlining possibilities as things grow,
   - using a switch statement avoids the function call overhead.

   This function could be moved into `parse_insn_normal', but keeping it
   separate makes clear the interface between `parse_insn_normal' and each of
   the handlers.  */

const char *
epiphany_cgen_parse_operand (CGEN_CPU_DESC cd,
			   int opindex,
			   const char ** strp,
			   CGEN_FIELDS * fields)
{
  const char * errmsg = NULL;
  /* Used by scalar operands that still need to be parsed.  */
  long junk ATTRIBUTE_UNUSED;

  switch (opindex)
    {
    case EPIPHANY_OPERAND_DIRECTION :
      errmsg = parse_postindex (cd, strp, EPIPHANY_OPERAND_DIRECTION, (unsigned long *) (& fields->f_addsubx));
      break;
    case EPIPHANY_OPERAND_DISP11 :
      errmsg = parse_uimm_not_reg (cd, strp, EPIPHANY_OPERAND_DISP11, (unsigned long *) (& fields->f_disp11));
      break;
    case EPIPHANY_OPERAND_DISP3 :
      errmsg = cgen_parse_unsigned_integer (cd, strp, EPIPHANY_OPERAND_DISP3, (unsigned long *) (& fields->f_disp3));
      break;
    case EPIPHANY_OPERAND_DPMI :
      errmsg = parse_postindex (cd, strp, EPIPHANY_OPERAND_DPMI, (unsigned long *) (& fields->f_subd));
      break;
    case EPIPHANY_OPERAND_FRD :
      errmsg = parse_shortregs (cd, strp, & epiphany_cgen_opval_gr_names, & fields->f_rd);
      break;
    case EPIPHANY_OPERAND_FRD6 :
      errmsg = cgen_parse_keyword (cd, strp, & epiphany_cgen_opval_gr_names, & fields->f_rd6);
      break;
    case EPIPHANY_OPERAND_FRM :
      errmsg = parse_shortregs (cd, strp, & epiphany_cgen_opval_gr_names, & fields->f_rm);
      break;
    case EPIPHANY_OPERAND_FRM6 :
      errmsg = cgen_parse_keyword (cd, strp, & epiphany_cgen_opval_gr_names, & fields->f_rm6);
      break;
    case EPIPHANY_OPERAND_FRN :
      errmsg = parse_shortregs (cd, strp, & epiphany_cgen_opval_gr_names, & fields->f_rn);
      break;
    case EPIPHANY_OPERAND_FRN6 :
      errmsg = cgen_parse_keyword (cd, strp, & epiphany_cgen_opval_gr_names, & fields->f_rn6);
      break;
    case EPIPHANY_OPERAND_IMM16 :
      {
        bfd_vma value = 0;
        errmsg = parse_imm16 (cd, strp, EPIPHANY_OPERAND_IMM16, 0, NULL,  & value);
        fields->f_imm16 = value;
      }
      break;
    case EPIPHANY_OPERAND_IMM8 :
      {
        bfd_vma value = 0;
        errmsg = parse_imm8 (cd, strp, EPIPHANY_OPERAND_IMM8, 0, NULL,  & value);
        fields->f_imm8 = value;
      }
      break;
    case EPIPHANY_OPERAND_RD :
      errmsg = parse_shortregs (cd, strp, & epiphany_cgen_opval_gr_names, & fields->f_rd);
      break;
    case EPIPHANY_OPERAND_RD6 :
      errmsg = cgen_parse_keyword (cd, strp, & epiphany_cgen_opval_gr_names, & fields->f_rd6);
      break;
    case EPIPHANY_OPERAND_RM :
      errmsg = parse_shortregs (cd, strp, & epiphany_cgen_opval_gr_names, & fields->f_rm);
      break;
    case EPIPHANY_OPERAND_RM6 :
      errmsg = cgen_parse_keyword (cd, strp, & epiphany_cgen_opval_gr_names, & fields->f_rm6);
      break;
    case EPIPHANY_OPERAND_RN :
      errmsg = parse_shortregs (cd, strp, & epiphany_cgen_opval_gr_names, & fields->f_rn);
      break;
    case EPIPHANY_OPERAND_RN6 :
      errmsg = cgen_parse_keyword (cd, strp, & epiphany_cgen_opval_gr_names, & fields->f_rn6);
      break;
    case EPIPHANY_OPERAND_SD :
      errmsg = parse_shortregs (cd, strp, & epiphany_cgen_opval_cr_names, & fields->f_sd);
      break;
    case EPIPHANY_OPERAND_SD6 :
      errmsg = cgen_parse_keyword (cd, strp, & epiphany_cgen_opval_cr_names, & fields->f_sd6);
      break;
    case EPIPHANY_OPERAND_SDDMA :
      errmsg = cgen_parse_keyword (cd, strp, & epiphany_cgen_opval_crdma_names, & fields->f_sd6);
      break;
    case EPIPHANY_OPERAND_SDMEM :
      errmsg = cgen_parse_keyword (cd, strp, & epiphany_cgen_opval_crmem_names, & fields->f_sd6);
      break;
    case EPIPHANY_OPERAND_SDMESH :
      errmsg = cgen_parse_keyword (cd, strp, & epiphany_cgen_opval_crmesh_names, & fields->f_sd6);
      break;
    case EPIPHANY_OPERAND_SHIFT :
      errmsg = cgen_parse_unsigned_integer (cd, strp, EPIPHANY_OPERAND_SHIFT, (unsigned long *) (& fields->f_shift));
      break;
    case EPIPHANY_OPERAND_SIMM11 :
      errmsg = parse_simm_not_reg (cd, strp, EPIPHANY_OPERAND_SIMM11, (long *) (& fields->f_sdisp11));
      break;
    case EPIPHANY_OPERAND_SIMM24 :
      {
        bfd_vma value = 0;
        errmsg = parse_branch_addr (cd, strp, EPIPHANY_OPERAND_SIMM24, 0, NULL,  & value);
        fields->f_simm24 = value;
      }
      break;
    case EPIPHANY_OPERAND_SIMM3 :
      errmsg = parse_simm_not_reg (cd, strp, EPIPHANY_OPERAND_SIMM3, (long *) (& fields->f_sdisp3));
      break;
    case EPIPHANY_OPERAND_SIMM8 :
      {
        bfd_vma value = 0;
        errmsg = parse_branch_addr (cd, strp, EPIPHANY_OPERAND_SIMM8, 0, NULL,  & value);
        fields->f_simm8 = value;
      }
      break;
    case EPIPHANY_OPERAND_SN :
      errmsg = parse_shortregs (cd, strp, & epiphany_cgen_opval_cr_names, & fields->f_sn);
      break;
    case EPIPHANY_OPERAND_SN6 :
      errmsg = cgen_parse_keyword (cd, strp, & epiphany_cgen_opval_cr_names, & fields->f_sn6);
      break;
    case EPIPHANY_OPERAND_SNDMA :
      errmsg = cgen_parse_keyword (cd, strp, & epiphany_cgen_opval_crdma_names, & fields->f_sn6);
      break;
    case EPIPHANY_OPERAND_SNMEM :
      errmsg = cgen_parse_keyword (cd, strp, & epiphany_cgen_opval_crmem_names, & fields->f_sn6);
      break;
    case EPIPHANY_OPERAND_SNMESH :
      errmsg = cgen_parse_keyword (cd, strp, & epiphany_cgen_opval_crmesh_names, & fields->f_sn6);
      break;
    case EPIPHANY_OPERAND_SWI_NUM :
      errmsg = parse_uimm_not_reg (cd, strp, EPIPHANY_OPERAND_SWI_NUM, (unsigned long *) (& fields->f_trap_num));
      break;
    case EPIPHANY_OPERAND_TRAPNUM6 :
      errmsg = cgen_parse_unsigned_integer (cd, strp, EPIPHANY_OPERAND_TRAPNUM6, (unsigned long *) (& fields->f_trap_num));
      break;

    default :
      /* xgettext:c-format */
      opcodes_error_handler
	(_("internal error: unrecognized field %d while parsing"),
	 opindex);
      abort ();
  }

  return errmsg;
}

cgen_parse_fn * const epiphany_cgen_parse_handlers[] =
{
  parse_insn_normal,
};

void
epiphany_cgen_init_asm (CGEN_CPU_DESC cd)
{
  epiphany_cgen_init_opcode_table (cd);
  epiphany_cgen_init_ibld_table (cd);
  cd->parse_handlers = & epiphany_cgen_parse_handlers[0];
  cd->parse_operand = epiphany_cgen_parse_operand;
#ifdef CGEN_ASM_INIT_HOOK
CGEN_ASM_INIT_HOOK
#endif
}



/* Regex construction routine.

   This translates an opcode syntax string into a regex string,
   by replacing any non-character syntax element (such as an
   opcode) with the pattern '.*'

   It then compiles the regex and stores it in the opcode, for
   later use by epiphany_cgen_assemble_insn

   Returns NULL for success, an error message for failure.  */

char *
epiphany_cgen_build_insn_regex (CGEN_INSN *insn)
{
  CGEN_OPCODE *opc = (CGEN_OPCODE *) CGEN_INSN_OPCODE (insn);
  const char *mnem = CGEN_INSN_MNEMONIC (insn);
  char rxbuf[CGEN_MAX_RX_ELEMENTS];
  char *rx = rxbuf;
  const CGEN_SYNTAX_CHAR_TYPE *syn;
  int reg_err;

  syn = CGEN_SYNTAX_STRING (CGEN_OPCODE_SYNTAX (opc));

  /* Mnemonics come first in the syntax string.  */
  if (! CGEN_SYNTAX_MNEMONIC_P (* syn))
    return _("missing mnemonic in syntax string");
  ++syn;

  /* Generate a case sensitive regular expression that emulates case
     insensitive matching in the "C" locale.  We cannot generate a case
     insensitive regular expression because in Turkish locales, 'i' and 'I'
     are not equal modulo case conversion.  */

  /* Copy the literal mnemonic out of the insn.  */
  for (; *mnem; mnem++)
    {
      char c = *mnem;

      if (ISALPHA (c))
	{
	  *rx++ = '[';
	  *rx++ = TOLOWER (c);
	  *rx++ = TOUPPER (c);
	  *rx++ = ']';
	}
      else
	*rx++ = c;
    }

  /* Copy any remaining literals from the syntax string into the rx.  */
  for(; * syn != 0 && rx <= rxbuf + (CGEN_MAX_RX_ELEMENTS - 7 - 4); ++syn)
    {
      if (CGEN_SYNTAX_CHAR_P (* syn))
	{
	  char c = CGEN_SYNTAX_CHAR (* syn);

	  switch (c)
	    {
	      /* Escape any regex metacharacters in the syntax.  */
	    case '.': case '[': case '\\':
	    case '*': case '^': case '$':

#ifdef CGEN_ESCAPE_EXTENDED_REGEX
	    case '?': case '{': case '}':
	    case '(': case ')': case '*':
	    case '|': case '+': case ']':
#endif
	      *rx++ = '\\';
	      *rx++ = c;
	      break;

	    default:
	      if (ISALPHA (c))
		{
		  *rx++ = '[';
		  *rx++ = TOLOWER (c);
		  *rx++ = TOUPPER (c);
		  *rx++ = ']';
		}
	      else
		*rx++ = c;
	      break;
	    }
	}
      else
	{
	  /* Replace non-syntax fields with globs.  */
	  *rx++ = '.';
	  *rx++ = '*';
	}
    }

  /* Trailing whitespace ok.  */
  * rx++ = '[';
  * rx++ = ' ';
  * rx++ = '\t';
  * rx++ = ']';
  * rx++ = '*';

  /* But anchor it after that.  */
  * rx++ = '$';
  * rx = '\0';

  CGEN_INSN_RX (insn) = xmalloc (sizeof (regex_t));
  reg_err = regcomp ((regex_t *) CGEN_INSN_RX (insn), rxbuf, REG_NOSUB);

  if (reg_err == 0)
    return NULL;
  else
    {
      static char msg[80];

      regerror (reg_err, (regex_t *) CGEN_INSN_RX (insn), msg, 80);
      regfree ((regex_t *) CGEN_INSN_RX (insn));
      free (CGEN_INSN_RX (insn));
      (CGEN_INSN_RX (insn)) = NULL;
      return msg;
    }
}


/* Default insn parser.

   The syntax string is scanned and operands are parsed and stored in FIELDS.
   Relocs are queued as we go via other callbacks.

   ??? Note that this is currently an all-or-nothing parser.  If we fail to
   parse the instruction, we return 0 and the caller will start over from
   the beginning.  Backtracking will be necessary in parsing subexpressions,
   but that can be handled there.  Not handling backtracking here may get
   expensive in the case of the m68k.  Deal with later.

   Returns NULL for success, an error message for failure.  */

static const char *
parse_insn_normal (CGEN_CPU_DESC cd,
		   const CGEN_INSN *insn,
		   const char **strp,
		   CGEN_FIELDS *fields)
{
  /* ??? Runtime added insns not handled yet.  */
  const CGEN_SYNTAX *syntax = CGEN_INSN_SYNTAX (insn);
  const char *str = *strp;
  const char *errmsg;
  const char *p;
  const CGEN_SYNTAX_CHAR_TYPE * syn;
#ifdef CGEN_MNEMONIC_OPERANDS
  /* FIXME: wip */
  int past_opcode_p;
#endif

  /* For now we assume the mnemonic is first (there are no leading operands).
     We can parse it without needing to set up operand parsing.
     GAS's input scrubber will ensure mnemonics are lowercase, but we may
     not be called from GAS.  */
  p = CGEN_INSN_MNEMONIC (insn);
  while (*p && TOLOWER (*p) == TOLOWER (*str))
    ++p, ++str;

  if (* p)
    return _("unrecognized instruction");

#ifndef CGEN_MNEMONIC_OPERANDS
  if (* str && ! ISSPACE (* str))
    return _("unrecognized instruction");
#endif

  CGEN_INIT_PARSE (cd);
  cgen_init_parse_operand (cd);
#ifdef CGEN_MNEMONIC_OPERANDS
  past_opcode_p = 0;
#endif

  /* We don't check for (*str != '\0') here because we want to parse
     any trailing fake arguments in the syntax string.  */
  syn = CGEN_SYNTAX_STRING (syntax);

  /* Mnemonics come first for now, ensure valid string.  */
  if (! CGEN_SYNTAX_MNEMONIC_P (* syn))
    abort ();

  ++syn;

  while (* syn != 0)
    {
      /* Non operand chars must match exactly.  */
      if (CGEN_SYNTAX_CHAR_P (* syn))
	{
	  /* FIXME: While we allow for non-GAS callers above, we assume the
	     first char after the mnemonic part is a space.  */
	  /* FIXME: We also take inappropriate advantage of the fact that
	     GAS's input scrubber will remove extraneous blanks.  */
	  if (TOLOWER (*str) == TOLOWER (CGEN_SYNTAX_CHAR (* syn)))
	    {
#ifdef CGEN_MNEMONIC_OPERANDS
	      if (CGEN_SYNTAX_CHAR(* syn) == ' ')
		past_opcode_p = 1;
#endif
	      ++ syn;
	      ++ str;
	    }
	  else if (*str)
	    {
	      /* Syntax char didn't match.  Can't be this insn.  */
	      static char msg [80];

	      /* xgettext:c-format */
	      sprintf (msg, _("syntax error (expected char `%c', found `%c')"),
		       CGEN_SYNTAX_CHAR(*syn), *str);
	      return msg;
	    }
	  else
	    {
	      /* Ran out of input.  */
	      static char msg [80];

	      /* xgettext:c-format */
	      sprintf (msg, _("syntax error (expected char `%c', found end of instruction)"),
		       CGEN_SYNTAX_CHAR(*syn));
	      return msg;
	    }
	  continue;
	}

#ifdef CGEN_MNEMONIC_OPERANDS
      (void) past_opcode_p;
#endif
      /* We have an operand of some sort.  */
      errmsg = cd->parse_operand (cd, CGEN_SYNTAX_FIELD (*syn), &str, fields);
      if (errmsg)
	return errmsg;

      /* Done with this operand, continue with next one.  */
      ++ syn;
    }

  /* If we're at the end of the syntax string, we're done.  */
  if (* syn == 0)
    {
      /* FIXME: For the moment we assume a valid `str' can only contain
	 blanks now.  IE: We needn't try again with a longer version of
	 the insn and it is assumed that longer versions of insns appear
	 before shorter ones (eg: lsr r2,r3,1 vs lsr r2,r3).  */
      while (ISSPACE (* str))
	++ str;

      if (* str != '\0')
	return _("junk at end of line"); /* FIXME: would like to include `str' */

      return NULL;
    }

  /* We couldn't parse it.  */
  return _("unrecognized instruction");
}

/* Main entry point.
   This routine is called for each instruction to be assembled.
   STR points to the insn to be assembled.
   We assume all necessary tables have been initialized.
   The assembled instruction, less any fixups, is stored in BUF.
   Remember that if CGEN_INT_INSN_P then BUF is an int and thus the value
   still needs to be converted to target byte order, otherwise BUF is an array
   of bytes in target byte order.
   The result is a pointer to the insn's entry in the opcode table,
   or NULL if an error occured (an error message will have already been
   printed).

   Note that when processing (non-alias) macro-insns,
   this function recurses.

   ??? It's possible to make this cpu-independent.
   One would have to deal with a few minor things.
   At this point in time doing so would be more of a curiosity than useful
   [for example this file isn't _that_ big], but keeping the possibility in
   mind helps keep the design clean.  */

const CGEN_INSN *
epiphany_cgen_assemble_insn (CGEN_CPU_DESC cd,
			   const char *str,
			   CGEN_FIELDS *fields,
			   CGEN_INSN_BYTES_PTR buf,
			   char **errmsg)
{
  const char *start;
  CGEN_INSN_LIST *ilist;
  const char *parse_errmsg = NULL;
  const char *insert_errmsg = NULL;
  int recognized_mnemonic = 0;

  /* Skip leading white space.  */
  while (ISSPACE (* str))
    ++ str;

  /* The instructions are stored in hashed lists.
     Get the first in the list.  */
  ilist = CGEN_ASM_LOOKUP_INSN (cd, str);

  /* Keep looking until we find a match.  */
  start = str;
  for ( ; ilist != NULL ; ilist = CGEN_ASM_NEXT_INSN (ilist))
    {
      const CGEN_INSN *insn = ilist->insn;
      recognized_mnemonic = 1;

#ifdef CGEN_VALIDATE_INSN_SUPPORTED
      /* Not usually needed as unsupported opcodes
	 shouldn't be in the hash lists.  */
      /* Is this insn supported by the selected cpu?  */
      if (! epiphany_cgen_insn_supported (cd, insn))
	continue;
#endif
      /* If the RELAXED attribute is set, this is an insn that shouldn't be
	 chosen immediately.  Instead, it is used during assembler/linker
	 relaxation if possible.  */
      if (CGEN_INSN_ATTR_VALUE (insn, CGEN_INSN_RELAXED) != 0)
	continue;

      str = start;

      /* Skip this insn if str doesn't look right lexically.  */
      if (CGEN_INSN_RX (insn) != NULL &&
	  regexec ((regex_t *) CGEN_INSN_RX (insn), str, 0, NULL, 0) == REG_NOMATCH)
	continue;

      /* Allow parse/insert handlers to obtain length of insn.  */
      CGEN_FIELDS_BITSIZE (fields) = CGEN_INSN_BITSIZE (insn);

      parse_errmsg = CGEN_PARSE_FN (cd, insn) (cd, insn, & str, fields);
      if (parse_errmsg != NULL)
	continue;

      /* ??? 0 is passed for `pc'.  */
      insert_errmsg = CGEN_INSERT_FN (cd, insn) (cd, insn, fields, buf,
						 (bfd_vma) 0);
      if (insert_errmsg != NULL)
        continue;

      /* It is up to the caller to actually output the insn and any
         queued relocs.  */
      return insn;
    }

  {
    static char errbuf[150];
    const char *tmp_errmsg;
#ifdef CGEN_VERBOSE_ASSEMBLER_ERRORS
#define be_verbose 1
#else
#define be_verbose 0
#endif

    if (be_verbose)
      {
	/* If requesting verbose error messages, use insert_errmsg.
	   Failing that, use parse_errmsg.  */
	tmp_errmsg = (insert_errmsg ? insert_errmsg :
		      parse_errmsg ? parse_errmsg :
		      recognized_mnemonic ?
		      _("unrecognized form of instruction") :
		      _("unrecognized instruction"));

	if (strlen (start) > 50)
	  /* xgettext:c-format */
	  sprintf (errbuf, "%s `%.50s...'", tmp_errmsg, start);
	else
	  /* xgettext:c-format */
	  sprintf (errbuf, "%s `%.50s'", tmp_errmsg, start);
      }
    else
      {
	if (strlen (start) > 50)
	  /* xgettext:c-format */
	  sprintf (errbuf, _("bad instruction `%.50s...'"), start);
	else
	  /* xgettext:c-format */
	  sprintf (errbuf, _("bad instruction `%.50s'"), start);
      }

    *errmsg = errbuf;
    return NULL;
  }
}
