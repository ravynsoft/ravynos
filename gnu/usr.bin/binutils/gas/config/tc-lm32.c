/* tc-lm32.c - Lattice Mico32 assembler.
   Copyright (C) 2008-2023 Free Software Foundation, Inc.
   Contributed by Jon Beniston <jon@beniston.com>

   This file is part of GAS, the GNU Assembler.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   GAS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with GAS; see the file COPYING.  If not, write to the Free Software
   Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "as.h"
#include <string.h>
#include <stdlib.h>
#include "safe-ctype.h"
#include "subsegs.h"
#include "bfd.h"
#include "safe-ctype.h"
#include "opcodes/lm32-desc.h"
#include "opcodes/lm32-opc.h"
#include "cgen.h"
#include "elf/lm32.h"

typedef struct
{
  const CGEN_INSN *insn;
  const CGEN_INSN *orig_insn;
  CGEN_FIELDS fields;
#if CGEN_INT_INSN_P
  CGEN_INSN_INT buffer [1];
#define INSN_VALUE(buf) (*(buf))
#else
  unsigned char buffer[CGEN_MAX_INSN_SIZE];
#define INSN_VALUE(buf) (buf)
#endif
  char *addr;
  fragS *frag;
  int num_fixups;
  fixS *fixups[GAS_CGEN_MAX_FIXUPS];
  int indices[MAX_OPERAND_INSTANCES];
} lm32_insn;

/* Configuration options */

#define LM_CFG_MULTIPLIY_ENABLED        0x0001
#define LM_CFG_DIVIDE_ENABLED           0x0002
#define LM_CFG_BARREL_SHIFT_ENABLED     0x0004
#define LM_CFG_SIGN_EXTEND_ENABLED      0x0008
#define LM_CFG_USER_ENABLED             0x0010
#define LM_CFG_ICACHE_ENABLED           0x0020
#define LM_CFG_DCACHE_ENABLED           0x0040
#define LM_CFG_BREAK_ENABLED            0x0080

static unsigned config = 0U;

/* Target specific assembler tokens / delimiters.  */

const char comment_chars[]        = "#";
const char line_comment_chars[]   = "#";
const char line_separator_chars[] = ";";
const char EXP_CHARS[]            = "eE";
const char FLT_CHARS[]            = "dD";

/* Target specific assembly directives.  */

const pseudo_typeS md_pseudo_table[] =
{
  { "align",   s_align_bytes,       0 },
  { "byte",    cons,                1 },
  { "hword",   cons,                2 },
  { "word",    cons,                4 },
  { "dword",   cons,                8 },
  {(char *)0 , (void(*)(int))0,     0}
};

/* Target specific command line options.  */

const char * md_shortopts = "";

struct option md_longopts[] =
{
#define OPTION_MULTIPLY_ENABLED         (OPTION_MD_BASE + 1)
  { "mmultiply-enabled",            no_argument, NULL, OPTION_MULTIPLY_ENABLED },
#define OPTION_DIVIDE_ENABLED           (OPTION_MD_BASE + 2)
  { "mdivide-enabled",              no_argument, NULL, OPTION_DIVIDE_ENABLED },
#define OPTION_BARREL_SHIFT_ENABLED     (OPTION_MD_BASE + 3)
  { "mbarrel-shift-enabled",        no_argument, NULL, OPTION_BARREL_SHIFT_ENABLED },
#define OPTION_SIGN_EXTEND_ENABLED      (OPTION_MD_BASE + 4)
  { "msign-extend-enabled",         no_argument, NULL, OPTION_SIGN_EXTEND_ENABLED },
#define OPTION_USER_ENABLED             (OPTION_MD_BASE + 5)
  { "muser-enabled",                no_argument, NULL, OPTION_USER_ENABLED },
#define OPTION_ICACHE_ENABLED           (OPTION_MD_BASE + 6)
  { "micache-enabled",              no_argument, NULL, OPTION_ICACHE_ENABLED },
#define OPTION_DCACHE_ENABLED           (OPTION_MD_BASE + 7)
  { "mdcache-enabled",              no_argument, NULL, OPTION_DCACHE_ENABLED },
#define OPTION_BREAK_ENABLED            (OPTION_MD_BASE + 8)
  { "mbreak-enabled",               no_argument, NULL, OPTION_BREAK_ENABLED },
#define OPTION_ALL_ENABLED              (OPTION_MD_BASE + 9)
  { "mall-enabled",                 no_argument, NULL, OPTION_ALL_ENABLED },
};

size_t md_longopts_size = sizeof (md_longopts);

/* Display architecture specific options.  */

void
md_show_usage (FILE * fp)
{
  fprintf (fp, "LM32 specific options:\n"
        "  -mmultiply-enabled          enable multiply instructions\n"
        "  -mdivide-enabled            enable divide and modulus instructions\n"
        "  -mbarrel-shift-enabled      enable multi-bit shift instructions\n"
        "  -msign-extend-enabled       enable sign-extension instructions\n"
        "  -muser-enabled              enable user-defined instructions\n"
        "  -micache-enabled            enable instruction cache instructions\n"
        "  -mdcache-enabled            enable data cache instructions\n"
        "  -mbreak-enabled             enable the break instruction\n"
        "  -mall-enabled               enable all optional instructions\n"
        );
}

/* Parse command line options.  */

int
md_parse_option (int c, const char * arg ATTRIBUTE_UNUSED)
{
  switch (c)
    {
      case OPTION_MULTIPLY_ENABLED:
        config |= LM_CFG_MULTIPLIY_ENABLED;
        break;
      case OPTION_DIVIDE_ENABLED:
        config |= LM_CFG_DIVIDE_ENABLED;
        break;
      case OPTION_BARREL_SHIFT_ENABLED:
        config |= LM_CFG_BARREL_SHIFT_ENABLED;
        break;
      case OPTION_SIGN_EXTEND_ENABLED:
        config |= LM_CFG_SIGN_EXTEND_ENABLED;
        break;
      case OPTION_USER_ENABLED:
        config |= LM_CFG_USER_ENABLED;
        break;
      case OPTION_ICACHE_ENABLED:
        config |= LM_CFG_ICACHE_ENABLED;
        break;
      case OPTION_DCACHE_ENABLED:
        config |= LM_CFG_DCACHE_ENABLED;
        break;
      case OPTION_BREAK_ENABLED:
        config |= LM_CFG_BREAK_ENABLED;
        break;
      case OPTION_ALL_ENABLED:
        config |= LM_CFG_MULTIPLIY_ENABLED;
        config |= LM_CFG_DIVIDE_ENABLED;
        config |= LM_CFG_BARREL_SHIFT_ENABLED;
        config |= LM_CFG_SIGN_EXTEND_ENABLED;
        config |= LM_CFG_USER_ENABLED;
        config |= LM_CFG_ICACHE_ENABLED;
        config |= LM_CFG_DCACHE_ENABLED;
        config |= LM_CFG_BREAK_ENABLED;
        break;
      default:
        return 0;
    }
  return 1;
}

/* Do any architecture specific initialisation.  */

void
md_begin (void)
{
  /* Initialize the `cgen' interface.  */

  /* Set the machine number and endian.  */
  gas_cgen_cpu_desc = lm32_cgen_cpu_open (CGEN_CPU_OPEN_MACHS, 0,
					   CGEN_CPU_OPEN_ENDIAN,
					   CGEN_ENDIAN_BIG,
					   CGEN_CPU_OPEN_END);
  lm32_cgen_init_asm (gas_cgen_cpu_desc);

  /* This is a callback from cgen to gas to parse operands.  */
  cgen_set_parse_operand_fn (gas_cgen_cpu_desc, gas_cgen_parse_operand);

  if (! bfd_set_arch_mach (stdoutput, bfd_arch_lm32, bfd_mach_lm32))
    as_warn (_("could not set architecture and machine"));
}

/* Turn an integer of n bytes (in val) into a stream of bytes appropriate
   for use in the a.out file, and stores them in the array pointed to by buf. */

void
md_number_to_chars (char * buf, valueT val, int n)
{
  if (target_big_endian)
    number_to_chars_bigendian (buf, val, n);
  else
    number_to_chars_littleendian (buf, val, n);
}

/* Turn a string in input_line_pointer into a floating point constant
   of type TYPE, and store the appropriate bytes in *LITP.  The number
   of LITTLENUMS emitted is stored in *SIZEP.  An error message is
   returned, or NULL on OK.  */

const char *
md_atof (int type, char *litP, int *sizeP)
{
  int i;
  int prec;
  LITTLENUM_TYPE words[4];

  char *t;

  switch (type)
    {
    case 'f':
      prec = 2;
      break;
    case 'd':
      prec = 4;
      break;
    default:
      *sizeP = 0;
      return _("bad call to md_atof");
    }

  t = atof_ieee (input_line_pointer, type, words);
  if (t)
    input_line_pointer = t;

  *sizeP = prec * sizeof (LITTLENUM_TYPE);

  if (target_big_endian)
    {
      for (i = 0; i < prec; i++)
	{
	  md_number_to_chars (litP, (valueT) words[i],
			      sizeof (LITTLENUM_TYPE));
	  litP += sizeof (LITTLENUM_TYPE);
	}
    }
  else
    {
      for (i = prec - 1; i >= 0; i--)
	{
	  md_number_to_chars (litP, (valueT) words[i],
			      sizeof (LITTLENUM_TYPE));
	  litP += sizeof (LITTLENUM_TYPE);
	}
    }

  return NULL;
}

/* Called for each undefined symbol. */

symbolS *
md_undefined_symbol (char * name ATTRIBUTE_UNUSED)
{
  return 0;
}

/* Round up a section size to the appropriate boundary.  */

valueT
md_section_align (asection *seg, valueT addr)
{
  int align = bfd_section_alignment (seg);
  return ((addr + (1 << align) - 1) & -(1 << align));
}

/* This function assembles the instructions. It emits the frags/bytes to the
   sections and creates the relocation entries.  */

void
md_assemble (char * str)
{
  lm32_insn insn;
  char * errmsg;

  /* Initialize GAS's cgen interface for a new instruction.  */
  gas_cgen_init_parse ();

  insn.insn = lm32_cgen_assemble_insn
      (gas_cgen_cpu_desc, str, &insn.fields, insn.buffer, &errmsg);

  if (!insn.insn)
    {
      as_bad ("%s", errmsg);
      return;
    }

  gas_cgen_finish_insn (insn.insn, insn.buffer,
			CGEN_FIELDS_BITSIZE (&insn.fields), 1, NULL);
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
    case LM32_OPERAND_GOT16:
      return BFD_RELOC_LM32_16_GOT;
    case LM32_OPERAND_GOTOFFHI16:
      return BFD_RELOC_LM32_GOTOFF_HI16;
    case LM32_OPERAND_GOTOFFLO16:
      return BFD_RELOC_LM32_GOTOFF_LO16;
    case LM32_OPERAND_GP16:
      return BFD_RELOC_GPREL16;
    case LM32_OPERAND_LO16:
      return BFD_RELOC_LO16;
    case LM32_OPERAND_HI16:
      return BFD_RELOC_HI16;
    case LM32_OPERAND_BRANCH:
      return BFD_RELOC_LM32_BRANCH;
    case LM32_OPERAND_CALL:
      return BFD_RELOC_LM32_CALL;
    default:
      break;
    }
  return BFD_RELOC_NONE;
}

/* Return the position from which the PC relative adjustment for a PC relative
   fixup should be made.  */

long
md_pcrel_from (fixS *fixP)
{
  /* Shouldn't get called.  */
  abort ();
  /* Return address of current instruction.  */
  return fixP->fx_where + fixP->fx_frag->fr_address;
}

/* The location from which a PC relative jump should be calculated,
   given a PC relative reloc.  */

long
md_pcrel_from_section (fixS * fixP, segT sec)
{
  if ((fixP->fx_addsy != (symbolS *) NULL)
      && (! S_IS_DEFINED (fixP->fx_addsy)
	  || (S_GET_SEGMENT (fixP->fx_addsy) != sec)))
    {
      /* The symbol is undefined (or is defined but not in this section).
	 Let the linker figure it out.  */
      return 0;
    }

  /*fprintf(stderr, "%s extern %d local %d\n", S_GET_NAME (fixP->fx_addsy), S_IS_EXTERN (fixP->fx_addsy), S_IS_LOCAL (fixP->fx_addsy));*/
  /* FIXME: Weak problem? */
  if ((fixP->fx_addsy != (symbolS *) NULL)
      && S_IS_EXTERNAL (fixP->fx_addsy))
    {
      /* If the symbol is external, let the linker handle it.  */
      return 0;
    }

  return fixP->fx_where + fixP->fx_frag->fr_address;
}

/* Return true if we can partially resolve a relocation now.  */

bool
lm32_fix_adjustable (fixS * fixP)
{
  /* We need the symbol name for the VTABLE entries */
  if (fixP->fx_r_type == BFD_RELOC_VTABLE_INHERIT
      || fixP->fx_r_type == BFD_RELOC_VTABLE_ENTRY)
    return false;

  return true;
}

/* Relaxation isn't required/supported on this target.  */

int
md_estimate_size_before_relax (fragS *fragp ATTRIBUTE_UNUSED,
			       asection *seg ATTRIBUTE_UNUSED)
{
  abort ();
  return 0;
}

void
md_convert_frag (bfd *abfd ATTRIBUTE_UNUSED,
		 asection *sec ATTRIBUTE_UNUSED,
		 fragS *fragP ATTRIBUTE_UNUSED)
{
  abort ();
}

void
md_apply_fix (fixS * fixP, valueT * valP, segT seg)
{
  /* Fix for weak symbols. Why do we have fx_addsy for weak symbols?  */
  if (fixP->fx_addsy != NULL && S_IS_WEAK (fixP->fx_addsy))
    *valP = 0;

  gas_cgen_md_apply_fix (fixP, valP, seg);
  return;
}
