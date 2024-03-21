/* tc-epiphany.c -- Assembler for the Adapteva EPIPHANY
   Copyright (C) 2009-2023 Free Software Foundation, Inc.
   Contributed by Embecosm on behalf of Adapteva, Inc.

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
   along with GAS; see the file COPYING.  If not, write to
   the Free Software Foundation, 51 Franklin Street - Fifth Floor,
   Boston, MA 02110-1301, USA.  */

#include "as.h"
#include "subsegs.h"
#include "symcat.h"
#include "opcodes/epiphany-desc.h"
#include "opcodes/epiphany-opc.h"
#include "cgen.h"
#include "elf/common.h"
#include "elf/epiphany.h"
#include "dwarf2dbg.h"

/* Structure to hold all of the different components describing
   an individual instruction.  */
typedef struct
{
  const CGEN_INSN *	insn;
  const CGEN_INSN *	orig_insn;
  CGEN_FIELDS		fields;
#if CGEN_INT_INSN_P
  CGEN_INSN_INT         buffer [1];
#define INSN_VALUE(buf) (*(buf))
#else
  unsigned char         buffer [CGEN_MAX_INSN_SIZE];
#define INSN_VALUE(buf) (buf)
#endif
  char *		addr;
  fragS *		frag;
  int                   num_fixups;
  fixS *                fixups [GAS_CGEN_MAX_FIXUPS];
  int                   indices [MAX_OPERAND_INSTANCES];
}
epiphany_insn;

const char comment_chars[]        = ";";
const char line_comment_chars[]   = "#";
const char line_separator_chars[] = "`";
const char EXP_CHARS[]            = "eE";
const char FLT_CHARS[]            = "fFdD";

/* Flag to detect when switching to code section where insn alignment is
   implied.  */
static bool force_code_align = false;

static void
epiphany_elf_section_rtn (int i)
{
  obj_elf_section (i);

  if (force_code_align)
    {
      do_align (1, NULL, 0, 0);
      force_code_align = false;
    }
}

static void
epiphany_elf_section_text (int i)
{
  obj_elf_text (i);

  do_align (1, NULL, 0, 0);
  force_code_align = false;
}

/* The target specific pseudo-ops which we support.  */
const pseudo_typeS md_pseudo_table[] =
{
    { "text",   epiphany_elf_section_text,  0 },
    { "sect",   epiphany_elf_section_rtn,   0 },
    /* .word should be 32 bits.  */
    { "word",       cons, 4 },
    { "cpu",        s_ignore,         0 },
    { "thumb_func", s_ignore,         0 },
    { "code",       s_ignore,         0 },
    { NULL,         NULL,             0 }
};



enum options
{
  OPTION_CPU_EPIPHANY = OPTION_MD_BASE,
  OPTION_CPU_EPIPHANY16
};

struct option md_longopts[] =
{
  { "mepiphany ",  no_argument, NULL, OPTION_CPU_EPIPHANY },
  { "mepiphany16", no_argument, NULL, OPTION_CPU_EPIPHANY16 },
  { NULL,          no_argument, NULL, 0 },
};

size_t md_longopts_size = sizeof (md_longopts);

const char * md_shortopts = "";

int
md_parse_option (int c ATTRIBUTE_UNUSED, const char * arg ATTRIBUTE_UNUSED)
{
  return 0;	/* No target-specific options.  */
}

void
md_show_usage (FILE * stream)
{
  fprintf (stream, _("EPIPHANY specific command line options:\n"));
}


void
md_begin (void)
{
  /* Initialize the `cgen' interface.  */

  /* Set the machine number and endian.  */
  gas_cgen_cpu_desc = epiphany_cgen_cpu_open (CGEN_CPU_OPEN_MACHS,
					   bfd_mach_epiphany32,
					   CGEN_CPU_OPEN_ENDIAN,
					   CGEN_ENDIAN_LITTLE,
					   CGEN_CPU_OPEN_END);
  epiphany_cgen_init_asm (gas_cgen_cpu_desc);

  /* This is a callback from cgen to gas to parse operands.  */
  cgen_set_parse_operand_fn (gas_cgen_cpu_desc, gas_cgen_parse_operand);

  /* Set the machine type.  */
  bfd_default_set_arch_mach (stdoutput, bfd_arch_epiphany, bfd_mach_epiphany32);

  literal_prefix_dollar_hex = true;
}

valueT
md_section_align (segT segment, valueT size)
{
  int align = bfd_section_alignment (segment);

  return ((size + (1 << align) - 1) & -(1 << align));
}


/* Functions concerning relocs.  */

long
md_pcrel_from (fixS *fixP ATTRIBUTE_UNUSED)
{
  abort ();
}

/* Write a value out to the object file, using the appropriate endianness.  */

void
md_number_to_chars (char * buf, valueT val, int n)
{
  number_to_chars_littleendian (buf, val, n);
}

int
epiphany_elf_section_flags (int flags,
			    int attr ATTRIBUTE_UNUSED,
			    int type ATTRIBUTE_UNUSED)
{
  /* This is used to detect when the section changes to an executable section.
     This function is called by the elf section processing.  When we note an
     executable section specifier we set an internal flag to denote when
     word alignment should be forced.  */
  if (flags & SEC_CODE)
    force_code_align = true;

  return flags;
}

/* Non-zero if we are generating PIC code.  */
int pic_code;

/* Epiphany er_flags.  */
static int epiphany_flags = 0;

/* Relocations against symbols are done in two
   parts, with a HI relocation and a LO relocation.  Each relocation
   has only 16 bits of space to store an addend.  This means that in
   order for the linker to handle carries correctly, it must be able
   to locate both the HI and the LO relocation.  This means that the
   relocations must appear in order in the relocation table.

   In order to implement this, we keep track of each unmatched HI
   relocation.  We then sort them so that they immediately precede the
   corresponding LO relocation.  */

struct epiphany_hi_fixup
{
  /* Next HI fixup.  */
  struct epiphany_hi_fixup *next;

  /* This fixup.  */
  fixS *fixp;

  /* The section this fixup is in.  */
  segT seg;
};


#define GOT_NAME "_GLOBAL_OFFSET_TABLE_"
static symbolS * GOT_symbol;

static inline bool
epiphany_PIC_related_p (symbolS *sym)
{
  expressionS *exp;

  if (! sym)
    return false;

  if (sym == GOT_symbol)
    return true;

  exp = symbol_get_value_expression (sym);

  return (exp->X_op == O_PIC_reloc
	  || exp->X_md == BFD_RELOC_EPIPHANY_SIMM24
	  || exp->X_md == BFD_RELOC_EPIPHANY_SIMM8
	  || epiphany_PIC_related_p (exp->X_add_symbol)
	  || epiphany_PIC_related_p (exp->X_op_symbol));
}

/* Perform target dependent relocations that are done at compile time.
   There aren't very many of these.  */

void
epiphany_apply_fix (fixS *fixP, valueT *valP, segT seg)
{
  if (fixP->fx_addsy == (symbolS *) NULL)
    fixP->fx_done = 1;

  if (((int) fixP->fx_r_type < (int) BFD_RELOC_UNUSED)
      && fixP->fx_done)
    {
      /* Install EPIPHANY-dependent relocations HERE because nobody else
	 will.  */
      char *where = fixP->fx_frag->fr_literal + fixP->fx_where;
      unsigned char *insn = (unsigned char *)where;
      valueT value = * valP;

      switch (fixP->fx_r_type)
	{
	default:
	  break;

	case BFD_RELOC_NONE:
	  return;

	case BFD_RELOC_EPIPHANY_SIMM11:
	  where[0] = where[0] | ((value & 1) << 7);
	  where[1] = where[1] | ((value & 6) >> 1);
	  where[2] = (value >> 3) & 0xff;
	  return;

	case BFD_RELOC_EPIPHANY_IMM11:
	  where[0] = where[0] | ((value & 1) << 7);
	  where[1] = where[1] | ((value & 6) >> 1);
	  where[2] = (value >> 3) & 0xff;
	  return;

	case BFD_RELOC_EPIPHANY_SIMM8:
	  md_number_to_chars (where+1, value>>1, 1);
	  return;

	case BFD_RELOC_EPIPHANY_SIMM24:
	  md_number_to_chars (where+1, value>>1, 3);
	  return;

	case BFD_RELOC_EPIPHANY_HIGH:
	  value >>= 16;
	  /* fallthru */
	case BFD_RELOC_EPIPHANY_LOW:
	  value = (((value & 0xff) << 5) | insn[0])
	    | (insn[1] << 8)
	    | ((value & 0xff00) << 12)
	    | (insn[2] << 16);
	  md_number_to_chars (where, value, 3);
	  return;
	}
    }

  /* Just do the default if we can't special case.  */
  return gas_cgen_md_apply_fix (fixP, valP, seg);
}


/* This is called from HANDLE_ALIGN in write.c.  Fill in the contents
   of an rs_align_code fragment.  0x01a2 is 16-bit pattern for a "nop".  */

static const unsigned char nop_pattern[] = { 0xa2, 0x01 };

void
epiphany_handle_align (fragS *fragp)
{
  int bytes, fix;
  char *p;

  if (fragp->fr_type != rs_align_code)
    return;

  bytes = fragp->fr_next->fr_address - fragp->fr_address - fragp->fr_fix;
  p = fragp->fr_literal + fragp->fr_fix;
  fix = 0;

  if (bytes & 1)
    {
      fix = 1;
      *p++ = 0;
      bytes--;
    }

  if (bytes & 2)
    {
      memcpy (p, nop_pattern, 2);
      p += 2;
      bytes -= 2;
      fix += 2;
    }
  fragp->fr_fix += fix;
}

/* Read a comma separated incrementing list of register names
   and form a bit mask of up to 15 registers 0..14.  */

static const char *
parse_reglist (const char * s, int * mask)
{
  int regmask = 0;

  while (*s)
    {
      long value;

      while (*s == ' ')
	++s;

      /* Parse a list with "," or "}" as limiters.  */
      const char *errmsg
	= cgen_parse_keyword (gas_cgen_cpu_desc, &s,
			      &epiphany_cgen_opval_gr_names, &value);
      if (errmsg)
	return errmsg;

      if (value > 15)
	return _("register number too large for push/pop");

      regmask |= 1 << value;
      if (regmask < *mask)
	return _("register is out of order");
      *mask |= regmask;

      while (*s==' ')
	++s;

      if (*s == '}')
	return NULL;
      else if (*s++ == ',')
	continue;
      else
	return _("bad register list");
    }

  return _("malformed reglist in push/pop");
}


/* Assemble an instruction,  push and pop pseudo instructions should have
   already been expanded.  */

static void
epiphany_assemble (const char *str)
    {
  epiphany_insn insn;
  char *errmsg = 0;

  memset (&insn, 0, sizeof (insn));

  /* Initialize GAS's cgen interface for a new instruction.  */
  gas_cgen_init_parse ();

  insn.insn = epiphany_cgen_assemble_insn
    (gas_cgen_cpu_desc, str, &insn.fields, insn.buffer, & errmsg);

  if (!insn.insn)
    {
      as_bad ("%s", errmsg);
      return;
    }

  if (CGEN_INSN_BITSIZE (insn.insn) == 32)
    {
      /* Doesn't really matter what we pass for RELAX_P here.  */
      gas_cgen_finish_insn (insn.insn, insn.buffer,
			    CGEN_FIELDS_BITSIZE (&insn.fields), 1, NULL);
    }
  else
    {
      if (CGEN_INSN_BITSIZE (insn.insn) != 16)
	abort ();

      insn.orig_insn = insn.insn;

      gas_cgen_finish_insn (insn.orig_insn, insn.buffer,
			    CGEN_FIELDS_BITSIZE (&insn.fields),
			    1 /* relax_p  */, NULL);
    }

  /* Checks for behavioral restrictions on LD/ST instructions.  */
#define DISPMOD _("destination register modified by displacement-post-modified address")
#define LDSTODD _("ldrd/strd requires even:odd register pair")

  /* Helper macros for splitting apart instruction fields.  */
#define ADDR_POST_MODIFIED(i) (((i) >> 25) & 0x1)
#define ADDR_SIZE(i)          (((i) >>  5) &   3)
#define ADDR_LOADSTORE(i)     (((i) >>  4) & 0x1)

  switch (insn.buffer[0] & 0xf)
    {
      /* Post-modify registers cannot be destinations.  */
    case OP4_LDSTR16P:
      {
	if (ADDR_LOADSTORE (insn.buffer[0]) ==  OP_LOAD)
	  if (insn.fields.f_rd == insn.fields.f_rn /* Postmodify dest.  */
	      || (insn.fields.f_rd+1 == insn.fields.f_rn
		  && ADDR_SIZE (insn.buffer[0]) == OPW_DOUBLE))
	    {
	      as_bad ("%s", DISPMOD);
	      return;
	    }
	if ((insn.fields.f_rd & 1) /* Odd-numbered register...  */
	    && insn.fields.f_wordsize == OPW_DOUBLE) /* ...and 64 bit transfer.  */
	  {
	    as_bad ("%s", LDSTODD);
	    return;
	  }
	break;
      }

    case OP4_LDSTRP:
      {
	if (ADDR_LOADSTORE (insn.buffer[0]) == OP_LOAD) /* A load.  */
	  if (insn.fields.f_rd6 == insn.fields.f_rn6 /* Postmodify dest.  */
	      /* Check for regpair postindexed.  */
	      || (insn.fields.f_rd6 + 1 == insn.fields.f_rn6
		  && ADDR_SIZE (insn.buffer[0]) == OPW_DOUBLE))
	    {
	      as_bad ("%s", DISPMOD);
	      return;
	    }
	if ((insn.fields.f_rd6 & 1) && ADDR_SIZE (insn.buffer[0]) == OPW_DOUBLE)
	  /* Lsb of RD odd and 64 bit transfer.  */
	  {
	    as_bad ("%s", LDSTODD);
	    return;
	  }
	break;
      }

    case OP4_LDSTR16X:
    case OP4_LDSTR16D:
      {
	/* Check for unaligned load/store double.  */
	if ((insn.fields.f_rd & 1) && ADDR_SIZE (insn.buffer[0]) == OPW_DOUBLE)
	  /* Lsb of RD odd and 64 bit transfer.  */
	  {
	    as_bad ("%s", LDSTODD);
	    return;
	  }
	break;
      }

    case OP4_LDSTRD:
      {
	/* Check for load to post-modified register.  */
	if (ADDR_LOADSTORE (insn.buffer[0]) == OP_LOAD /* A load.  */
	    && ADDR_POST_MODIFIED (insn.buffer[0]) == PMOD_POST /* Post-mod.  */
	    && (insn.fields.f_rd6 == insn.fields.f_rn6
		|| (insn.fields.f_rd6+1 == insn.fields.f_rn6
		    && ADDR_SIZE (insn.buffer[0]) == OPW_DOUBLE)))
	  {
	    as_bad ("%s", DISPMOD);
	    return;
	  }
      }
      /* fallthru */

    case OP4_LDSTRX:
      {
	/* Check for unaligned load/store double.  */
	if ((insn.fields.f_rd6 & 1) && ADDR_SIZE (insn.buffer[0]) == OPW_DOUBLE)
	  {
	    as_bad ("%s", LDSTODD);
	    return;
	  }
	break;
      }

    default:
      break;
    }
}

void
md_assemble (char *str)
{
  const char * pperr = 0;
  int regmask=0, push=0, pop=0;

  /* Special-case push/pop instruction macros.  */
  if (startswith (str, "push {"))
    {
      char * s = str + 6;
      push = 1;
      pperr = parse_reglist (s, &regmask);
    }
  else if (startswith (str, "pop {"))
    {
      char * s = str + 5;
      pop = 1;
      pperr = parse_reglist (s, &regmask);
    }

  if (pperr)
    {
      as_bad ("%s", pperr);
      return;
    }

  if (push && regmask)
    {
      char buff[32];
      int i,p ATTRIBUTE_UNUSED;

      epiphany_assemble ("mov r15,4");
      epiphany_assemble ("sub sp,sp,r15");

      for (i = 0, p = 1; i <= 15; ++i, regmask >>= 1)
	{
	  if (regmask == 1)
	    sprintf (buff, "str r%d,[sp]", i); /* Last one.  */
	  else if (regmask & 1)
	    sprintf (buff, "str r%d,[sp],-r15", i);
	  else
	    continue;
	  epiphany_assemble (buff);
	}
      return;
    }
  else if (pop && regmask)
    {
      char buff[32];
      int i,p;

      epiphany_assemble ("mov r15,4");

      for (i = 15, p = 1 << 15; i >= 0; --i, p >>= 1)
	if (regmask & p)
	  {
	    sprintf (buff, "ldr r%d,[sp],+r15", i);
	    epiphany_assemble (buff);
	  }
      return;
    }

  epiphany_assemble (str);
}

/* The syntax in the manual says constants begin with '#'.
   We just ignore it.  */

void
md_operand (expressionS *expressionP)
{
  if (*input_line_pointer == '#')
    {
      input_line_pointer++;
      expression (expressionP);
    }
}

symbolS *
md_undefined_symbol (char *name ATTRIBUTE_UNUSED)
{
  return NULL;
}

/* Interface to relax_segment.  */

/* FIXME: Build table by hand, get it working, then machine generate.  */

const relax_typeS md_relax_table[] =
{
  /* The fields are:
     1) most positive reach of this state,
     2) most negative reach of this state,
     3) how many bytes this mode will add to the size of the current frag
     4) which index into the table to try if we can't fit into this one.  */

  /* The first entry must be unused because an `rlx_more' value of zero ends
     each list.  */
  {1, 1, 0, EPIPHANY_RELAX_NONE},
  {0, 0, 0, EPIPHANY_RELAX_NONE},    /* Also a dummy entry to indicate we need to expand codes.  */

  /* The displacement used by GAS is from the end of the 2 byte insn,
     so we subtract 2 from the following.  */
  /* 16 bit insn, 8 bit disp -> +127 words, -128 words.  */
  {0x00000100 - 1 - 2, -0x00000100 - 2, 0, EPIPHANY_RELAX_BRANCH_LONG },
  /* 32 bit insn, 24 bit disp -> 25 bit range.  */
  {0x01000000 - 1 - 2, -0x01000000 - 2, 2, EPIPHANY_RELAX_NONE },

  /* addi/subi 3 bits -4..+3.  */
  {    3,           -4,0, EPIPHANY_RELAX_ARITH_SIMM11 },
  /* addi/subi 11 bits.  */
  {  1023,       -1024,2, EPIPHANY_RELAX_NONE },

  /* mov r,imm8.  */
  {   255,           0,0, EPIPHANY_RELAX_MOV_IMM16 },
  /* mov r,imm16. */
  { 65535,           0,2, EPIPHANY_RELAX_NONE },

  /* ld/st rd,[rn,imm3].  */
  {     7,           0,0, EPIPHANY_RELAX_LDST_IMM11},
  /* ld/st rd,[rn,imm11].  */
  {  2047,           0,2, EPIPHANY_RELAX_NONE }

};

static const EPIPHANY_RELAX_TYPES relax_insn[] =
{
  EPIPHANY_RELAX_BRANCH_SHORT,	/* OP4_BRANCH16 */
  EPIPHANY_RELAX_NONE,		/* OP4_LDSTR16X */
  EPIPHANY_RELAX_NONE,		/* OP4_FLOW16 */
  EPIPHANY_RELAX_ARITH_SIMM3,	/* OP4_IMM16 - special */
  EPIPHANY_RELAX_LDST_IMM3,	/* OP4_LDSTR16D */
  EPIPHANY_RELAX_NONE,		/* OP4_LDSTR126P */
  EPIPHANY_RELAX_NONE,		/* OP4_LSHIFT16 */
  EPIPHANY_RELAX_NONE,		/* OP4_DSP16 */
  EPIPHANY_RELAX_BRANCH_LONG,	/* OP4_BRANCH */
  EPIPHANY_RELAX_NONE,		/* OP4_LDSTRX */
  EPIPHANY_RELAX_NONE,		/* OP4_ALU16 */
  EPIPHANY_RELAX_ARITH_SIMM11,	/* OP4_IMM32 - special */
  EPIPHANY_RELAX_LDST_IMM11,	/* OP4_LDSTRD */
  EPIPHANY_RELAX_NONE,		/* OP4_LDSTRP */
  EPIPHANY_RELAX_NONE,		/* OP4_ASHIFT16 */
  EPIPHANY_RELAX_NONE		/* OP4_MISC */
};

long
epiphany_relax_frag (segT segment, fragS *fragP, long stretch)
{
  /* Address of branch insn.  */
  long address ATTRIBUTE_UNUSED = fragP->fr_address + fragP->fr_fix - 2;
  long growth = 0;

  if (fragP->fr_subtype == EPIPHANY_RELAX_NEED_RELAXING)
    {
      EPIPHANY_RELAX_TYPES subtype = relax_insn [*fragP->fr_opcode & 0xf];

      /* Special cases add/sub vs mov immediates.  */
      if (subtype == EPIPHANY_RELAX_ARITH_SIMM3)
	{
	  if ((*fragP->fr_opcode & 0x10) == 0)
	    subtype = EPIPHANY_RELAX_MOV_IMM8;
	}
      else if (subtype == EPIPHANY_RELAX_ARITH_SIMM11)
	{
	  if ((*fragP->fr_opcode & 0x10) == 0)
	    subtype = EPIPHANY_RELAX_MOV_IMM16;
	}

      /* Remember refinements for the future.  */
      fragP->fr_subtype = subtype;
    }

  growth = relax_frag (segment, fragP, stretch);

  return growth;
}

/* Return an initial guess of the length by which a fragment must grow to
   hold a branch to reach its destination.
   Also updates fr_type/fr_subtype as necessary.

   Called just before doing relaxation.
   Any symbol that is now undefined will not become defined.
   The guess for fr_var is ACTUALLY the growth beyond fr_fix.
   Whatever we do to grow fr_fix or fr_var contributes to our returned value.
   Although it may not be explicit in the frag, pretend fr_var starts
   with a 0 value.  */

int
md_estimate_size_before_relax (fragS *fragP, segT segment)
{
  /* The only thing we have to handle here are symbols outside of the
     current segment.  They may be undefined or in a different segment in
     which case linker scripts may place them anywhere.
     However, we can't finish the fragment here and emit the reloc as insn
     alignment requirements may move the insn about.  */
  if (S_GET_SEGMENT (fragP->fr_symbol) != segment
      || S_IS_EXTERNAL (fragP->fr_symbol)
      || S_IS_WEAK (fragP->fr_symbol))
    {
      /* The symbol is undefined in this segment.  Change the
	 relaxation subtype to the max allowable and leave all further
	 handling to md_convert_frag.  */

      EPIPHANY_RELAX_TYPES subtype;
      const CGEN_INSN *insn;
      int i;
      /* We haven't relaxed this at all, so the relaxation type may be
	 completely wrong.  Set the subtype correctly.  */
      epiphany_relax_frag (segment, fragP, 0);
      subtype = fragP->fr_subtype;

      switch (subtype)
	{
	case EPIPHANY_RELAX_LDST_IMM3:
	  subtype = EPIPHANY_RELAX_LDST_IMM11;
	  break;
	case EPIPHANY_RELAX_BRANCH_SHORT:
	  subtype = EPIPHANY_RELAX_BRANCH_LONG;
	  break;
	case EPIPHANY_RELAX_MOV_IMM8:
	  subtype = EPIPHANY_RELAX_MOV_IMM16;
	  break;
	case EPIPHANY_RELAX_ARITH_SIMM3:
	  subtype = EPIPHANY_RELAX_ARITH_SIMM11;
	  break;

	default:
	  break;
	}

      fragP->fr_subtype = subtype;

      /* Update the recorded insn.  */
      for (i = 0, insn = fragP->fr_cgen.insn; i < 4; i++, insn++)
	{
	  if (strcmp (CGEN_INSN_MNEMONIC (insn),
		      CGEN_INSN_MNEMONIC (fragP->fr_cgen.insn)) == 0
	      && CGEN_INSN_ATTR_VALUE (insn, CGEN_INSN_RELAXED))
	    break;
	}

      if (i == 4)
	abort ();

      /* When changing from a 2-byte to 4-byte insn, don't leave
	 opcode bytes uninitialised.  */
      if (CGEN_INSN_BITSIZE (fragP->fr_cgen.insn) < CGEN_INSN_BITSIZE (insn))
	{
	  gas_assert (CGEN_INSN_BITSIZE (fragP->fr_cgen.insn) == 16);
	  gas_assert (CGEN_INSN_BITSIZE (insn) == 32);
	  fragP->fr_opcode[2] = 0;
	  fragP->fr_opcode[3] = 0;
	}

      fragP->fr_cgen.insn = insn;
    }

  return md_relax_table[fragP->fr_subtype].rlx_length;
}

/* *FRAGP has been relaxed to its final size, and now needs to have
   the bytes inside it modified to conform to the new size.

   Called after relaxation is finished.
   fragP->fr_type == rs_machine_dependent.
   fragP->fr_subtype is the subtype of what the address relaxed to.  */

void
md_convert_frag (bfd *abfd ATTRIBUTE_UNUSED,
		 segT sec,
		 fragS *fragP)
{
  char *opcode;
  char *displacement;
  int target_address;
  int opcode_address;
  int extension;
  int addend;
  int opindx = -1;

  opcode = fragP->fr_opcode;

  /* Address opcode resides at in file space.  */
  opcode_address = fragP->fr_address + fragP->fr_fix - 2;
  extension = 0;
  displacement = &opcode[1];

  /* Set up any addend necessary for branches.  */
  if (S_GET_SEGMENT (fragP->fr_symbol) != sec
      || S_IS_EXTERNAL (fragP->fr_symbol)
      || S_IS_WEAK (fragP->fr_symbol))
    {
      /* Symbol must be resolved by linker.  */
      if (fragP->fr_offset & 1)
	as_warn (_("Addend to unresolved symbol not on word boundary."));
      addend = 0;
    }
  else
    {
      /* Address we want to reach in file space.  */
      target_address = S_GET_VALUE (fragP->fr_symbol) + fragP->fr_offset;
      addend = (target_address - (opcode_address & -2));
    }

  /* Do all the housekeeping for frag conversions. */
  switch (fragP->fr_subtype)
    {
    case EPIPHANY_RELAX_ARITH_SIMM11:
      *opcode |= OP4_IMM32;
      displacement = &opcode[0];
      extension += 3;

      addend
	= (((addend & 0x7) << 7)
	   | opcode[0]
	   | ((addend & 0x7f8) << 13)
	   | (opcode[1] << 8)
	   | (opcode[2] << 16));

      opindx = EPIPHANY_OPERAND_SIMM11;
      break;

    case EPIPHANY_RELAX_BRANCH_LONG:
      /* Branches differ only in low nibble of instruction being 8 not 0.
	 24 bit displacement goes to bytes 1..3 .  */
      *opcode |= OP4_BRANCH;
      extension += 2;

      addend >>= 1;		/* Convert to word offset.  */
      opindx = EPIPHANY_OPERAND_SIMM24;
      break;

    case EPIPHANY_RELAX_MOV_IMM16:
      *opcode |=  OP4_IMM32;
      extension += 3;

      addend
	= (((addend & 0xff00) << 12)
	   | (opcode[2] << 16)
	   | ((addend & 0x00ff) << 5)
	   | (opcode[1] << 8)
	   | opcode[0]);
      displacement = &opcode[0];
      opindx = EPIPHANY_OPERAND_IMM16;
      break;

    case EPIPHANY_RELAX_LDST_IMM11:
      *opcode |= OP4_LDSTRD;
      displacement = &opcode[0];
      extension += 3;

      if (addend < 0)
	/* Convert twos-complement address value to sign-magnitude.  */
	addend = (-addend & 0x7ff) | 0x800;

      addend
	= (((addend & 0x7) << 5)
	   | opcode[0]
	   | ((addend & 0xff8) << 13)
	   | (opcode[1] << 8)
	   | (opcode[2] << 16));

      opindx = EPIPHANY_OPERAND_DISP11;
      break;

    case EPIPHANY_RELAX_ARITH_SIMM3:
      addend = ((addend & 7) << 5) | opcode[0];
      opindx = EPIPHANY_OPERAND_SIMM3;
      break;

    case EPIPHANY_RELAX_LDST_IMM3:
      addend = ((addend & 7) << 5) | opcode[0];
      opindx = EPIPHANY_OPERAND_DISP3;
      break;

    case EPIPHANY_RELAX_BRANCH_SHORT:
      addend >>= 1;		/* Convert to a word offset.  */
      displacement = & opcode[1];
      opindx = EPIPHANY_OPERAND_SIMM8;
      break;

    case EPIPHANY_RELAX_MOV_IMM8:
      addend
	= (((addend & 0xff) << 5)
	   | opcode[0]
	   | (opcode[1] << 8));
      opindx = EPIPHANY_OPERAND_IMM8;
      break;

    case EPIPHANY_RELAX_NONE:
    case EPIPHANY_RELAX_NEED_RELAXING:
    default:			/* Anything else?  */
      as_bad ("unrecognized fragment subtype");
      break;
    }

  /* Create a relocation for symbols that must be resolved by the linker.
     Otherwise output the completed insn.  */

  if (S_GET_SEGMENT (fragP->fr_symbol) != sec
      || S_IS_EXTERNAL (fragP->fr_symbol)
      || S_IS_WEAK (fragP->fr_symbol))
    {
      fixS *fixP;
      const CGEN_OPERAND *operand
	= cgen_operand_lookup_by_num (gas_cgen_cpu_desc, opindx);
      bfd_reloc_code_real_type reloc_type;

      gas_assert (fragP->fr_cgen.insn != 0);

      reloc_type = md_cgen_lookup_reloc (fragP->fr_cgen.insn, operand, NULL);

      fixP = gas_cgen_record_fixup (fragP,
				    /* Offset of insn in frag.  */
				    (opcode - fragP->fr_literal),
				    fragP->fr_cgen.insn,
				    CGEN_INSN_BITSIZE (fragP->fr_cgen.insn) / 8,
				    operand,
				    reloc_type,
				    fragP->fr_symbol, fragP->fr_offset);
      fixP->fx_r_type = fixP->fx_cgen.opinfo;
    }

  md_number_to_chars (displacement, (valueT) addend, extension + 1);

  fragP->fr_fix += (extension & -2); /* 0,2 or 4 bytes added.  */
}


/* Functions concerning relocs.  */

/* The location from which a PC relative jump should be calculated,
   given a PC relative reloc.  */

long
md_pcrel_from_section (fixS *fixP, segT sec)
{
  if (fixP->fx_addsy != (symbolS *) NULL
      && (!S_IS_DEFINED (fixP->fx_addsy)
	  || (S_GET_SEGMENT (fixP->fx_addsy) != sec)
	  || S_IS_EXTERNAL (fixP->fx_addsy)
	  || S_IS_WEAK (fixP->fx_addsy)))
    return 0;

  return fixP->fx_frag->fr_address + fixP->fx_where;
}

/* Return the bfd reloc type for OPERAND of INSN at fixup FIXP.
   Returns BFD_RELOC_NONE if no reloc type can be found.
   *FIXP may be modified if desired.  */

bfd_reloc_code_real_type
md_cgen_lookup_reloc (const CGEN_INSN *insn ATTRIBUTE_UNUSED,
		      const CGEN_OPERAND *operand,
		      fixS *fixP ATTRIBUTE_UNUSED)
{
  switch (operand->type)
    {
    case EPIPHANY_OPERAND_SIMM11:
      return BFD_RELOC_EPIPHANY_SIMM11;
    case EPIPHANY_OPERAND_DISP11:
      return BFD_RELOC_EPIPHANY_IMM11;

    case EPIPHANY_OPERAND_SIMM8:
      return BFD_RELOC_EPIPHANY_SIMM8;
    case EPIPHANY_OPERAND_SIMM24:
      return BFD_RELOC_EPIPHANY_SIMM24;

    case EPIPHANY_OPERAND_IMM8:
      return BFD_RELOC_EPIPHANY_IMM8;

    case EPIPHANY_OPERAND_IMM16:
      if (0 == strcmp ("movt", CGEN_INSN_MNEMONIC (insn)))
	return BFD_RELOC_EPIPHANY_HIGH;
      else if (0 == strcmp ("mov", CGEN_INSN_MNEMONIC (insn)))
	return BFD_RELOC_EPIPHANY_LOW;
      else
	as_bad ("unknown imm16 operand");
      /* fallthru */

    default:
      break;
    }
  return BFD_RELOC_NONE;
}


/* Turn a string in input_line_pointer into a floating point constant
   of type TYPE, and store the appropriate bytes in *LITP.  The number
   of LITTLENUMS emitted is stored in *SIZEP.  An error message is
   returned, or NULL on OK.  */

const char *
md_atof (int type, char *litP, int *sizeP)
{
  return ieee_md_atof (type, litP, sizeP, false);
}

/* Return true if can adjust the reloc to be relative to its section
   (such as .data) instead of relative to some symbol.  */

bool
epiphany_fix_adjustable (fixS *fixP)
{
 bfd_reloc_code_real_type reloc_type;

  if ((int) fixP->fx_r_type >= (int) BFD_RELOC_UNUSED)
    {
      const CGEN_INSN *insn = fixP->fx_cgen.insn;
      int opindex = (int) fixP->fx_r_type - (int) BFD_RELOC_UNUSED;
      const CGEN_OPERAND *operand =
	cgen_operand_lookup_by_num (gas_cgen_cpu_desc, opindex);

      reloc_type = md_cgen_lookup_reloc (insn, operand, fixP);
    }
  else
    reloc_type = fixP->fx_r_type;

  if (fixP->fx_addsy == NULL)
    return true;

  /* Prevent all adjustments to global symbols.  */
  if (S_IS_EXTERNAL (fixP->fx_addsy))
    return false;

  if (S_IS_WEAK (fixP->fx_addsy))
    return false;

  if (pic_code
      && (reloc_type == BFD_RELOC_EPIPHANY_SIMM24
	  || reloc_type == BFD_RELOC_EPIPHANY_SIMM8
	  || reloc_type == BFD_RELOC_EPIPHANY_HIGH
	  || reloc_type == BFD_RELOC_EPIPHANY_LOW))
    return false;

  /* Since we don't use partial_inplace, we must not reduce symbols in
     mergeable sections to their section symbol.  */
  if ((S_GET_SEGMENT (fixP->fx_addsy)->flags & SEC_MERGE) != 0)
    return false;

  return true;
}

void
epiphany_elf_final_processing (void)
{
  elf_elfheader (stdoutput)->e_flags |= epiphany_flags;
}

int
epiphany_cgen_parse_fix_exp (int opinfo, expressionS *exp ATTRIBUTE_UNUSED)
{
  LITTLENUM_TYPE words[2];

  switch (opinfo)
    {
    case BFD_RELOC_EPIPHANY_LOW:
    case BFD_RELOC_EPIPHANY_HIGH:
      break;
    default:
      return opinfo;
    }

  /* Doing a %LOW or %HIGH.  */
  switch (exp->X_op)
    {
    default:
      return opinfo;
    case O_big:				/* Bignum.  */
      if (exp->X_add_number > 0)	/* Integer value too large.  */
	return opinfo;
    }

  /* Convert to SP number.  */
  gen_to_words (words, 2, 8L);
  exp->X_add_number = words[1] | (words[0] << 16);
  exp->X_op = O_constant;
  return opinfo;
}
