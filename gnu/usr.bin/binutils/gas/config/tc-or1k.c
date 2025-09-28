/* tc-or1k.c -- Assembler for the OpenRISC family.
   Copyright (C) 2001-2023 Free Software Foundation, Inc.
   Contributed for OR32 by Johan Rydberg, jrydberg@opencores.org

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
   along with this program; if not, see <http://www.gnu.org/licenses/> */
#include "as.h"
#include "safe-ctype.h"
#include "subsegs.h"
#include "symcat.h"
#include "opcodes/or1k-desc.h"
#include "opcodes/or1k-opc.h"
#include "cgen.h"
#include "elf/or1k.h"
#include "dw2gencfi.h"

/* Structure to hold all of the different components describing
   an individual instruction.  */

typedef struct
{
  const CGEN_INSN *     insn;
  const CGEN_INSN *     orig_insn;
  CGEN_FIELDS           fields;
#if CGEN_INT_INSN_P
  CGEN_INSN_INT         buffer [1];
#define INSN_VALUE(buf) (*(buf))
#else
  unsigned char         buffer [CGEN_MAX_INSN_SIZE];
#define INSN_VALUE(buf) (buf)
#endif
  char *                addr;
  fragS *               frag;
  int                   num_fixups;
  fixS *                fixups [GAS_CGEN_MAX_FIXUPS];
  int                   indices [MAX_OPERAND_INSTANCES];
}
or1k_insn;

const char comment_chars[]        = "#";
const char line_comment_chars[]   = "#";
const char line_separator_chars[] = ";";
const char EXP_CHARS[]            = "eE";
const char FLT_CHARS[]            = "dD";

#define OR1K_SHORTOPTS "m:"
const char * md_shortopts = OR1K_SHORTOPTS;

struct option md_longopts[] =
{
  {NULL, no_argument, NULL, 0}
};
size_t md_longopts_size = sizeof (md_longopts);

unsigned long or1k_machine = 0; /* default */

int
md_parse_option (int c ATTRIBUTE_UNUSED, const char * arg ATTRIBUTE_UNUSED)
{
  return 0;
}

void
md_show_usage (FILE * stream ATTRIBUTE_UNUSED)
{
}

static void
ignore_pseudo (int val ATTRIBUTE_UNUSED)
{
  discard_rest_of_line ();
}

static bool nodelay = false;
static void
s_nodelay (int val ATTRIBUTE_UNUSED)
{
  nodelay = true;
}

const char or1k_comment_chars [] = ";#";

/* The target specific pseudo-ops which we support.  */
const pseudo_typeS md_pseudo_table[] =
{
  { "align",    s_align_bytes,  0 },
  { "word",     cons,           4 },
  { "proc",     ignore_pseudo,  0 },
  { "endproc",  ignore_pseudo,  0 },
  { "nodelay",  s_nodelay,      0 },
  { NULL,       NULL,           0 }
};


void
md_begin (void)
{
  /* Initialize the `cgen' interface.  */

  /* Set the machine number and endian.  */
  gas_cgen_cpu_desc = or1k_cgen_cpu_open (CGEN_CPU_OPEN_MACHS, 0,
                                              CGEN_CPU_OPEN_ENDIAN,
                                              CGEN_ENDIAN_BIG,
                                              CGEN_CPU_OPEN_END);
  or1k_cgen_init_asm (gas_cgen_cpu_desc);

  /* This is a callback from cgen to gas to parse operands.  */
  cgen_set_parse_operand_fn (gas_cgen_cpu_desc, gas_cgen_parse_operand);
}

void
md_assemble (char * str)
{
  static int last_insn_had_delay_slot = 0;
  or1k_insn insn;
  char *    errmsg;

  /* Initialize GAS's cgen interface for a new instruction.  */
  gas_cgen_init_parse ();

  insn.insn = or1k_cgen_assemble_insn
    (gas_cgen_cpu_desc, str, & insn.fields, insn.buffer, & errmsg);

  if (!insn.insn)
    {
      as_bad ("%s", errmsg);
      return;
    }

  /* Doesn't really matter what we pass for RELAX_P here.  */
  gas_cgen_finish_insn (insn.insn, insn.buffer,
                        CGEN_FIELDS_BITSIZE (& insn.fields), 1, NULL);

  last_insn_had_delay_slot
    = CGEN_INSN_ATTR_VALUE (insn.insn, CGEN_INSN_DELAY_SLOT);
  (void) last_insn_had_delay_slot;
}


/* The syntax in the manual says constants begin with '#'.
   We just ignore it.  */

void
md_operand (expressionS * expressionP)
{
  if (* input_line_pointer == '#')
    {
      input_line_pointer ++;
      expression (expressionP);
    }
}

valueT
md_section_align (segT segment, valueT size)
{
  int align = bfd_section_alignment (segment);
  return ((size + (1 << align) - 1) & -(1 << align));
}

symbolS *
md_undefined_symbol (char * name ATTRIBUTE_UNUSED)
{
  return 0;
}


/* Interface to relax_segment.  */

const relax_typeS md_relax_table[] =
{
/* The fields are:
   1) most positive reach of this state,
   2) most negative reach of this state,
   3) how many bytes this mode will add to the size of the current frag
   4) which index into the table to try if we can't fit into this one.  */

  /* The first entry must be unused because an `rlx_more' value of zero ends
     each list.  */
  {1, 1, 0, 0},

  /* The displacement used by GAS is from the end of the 4 byte insn,
     so we subtract 4 from the following.  */
  {(((1 << 25) - 1) << 2) - 4, -(1 << 25) - 4, 0, 0},
};

int
md_estimate_size_before_relax (fragS * fragP, segT segment ATTRIBUTE_UNUSED)
{
  return md_relax_table[fragP->fr_subtype].rlx_length;
}

/* *fragP has been relaxed to its final size, and now needs to have
   the bytes inside it modified to conform to the new size.

   Called after relaxation is finished.
   fragP->fr_type == rs_machine_dependent.
   fragP->fr_subtype is the subtype of what the address relaxed to.  */

void
md_convert_frag (bfd *   abfd ATTRIBUTE_UNUSED,
                 segT    sec  ATTRIBUTE_UNUSED,
                 fragS * fragP ATTRIBUTE_UNUSED)
{
  /* FIXME */
}


/* Functions concerning relocs.  */

/* The location from which a PC relative jump should be calculated,
   given a PC relative reloc.  */

long
md_pcrel_from_section (fixS * fixP, segT sec)
{
  if (fixP->fx_addsy != (symbolS *) NULL
      && (! S_IS_DEFINED (fixP->fx_addsy)
          || (S_GET_SEGMENT (fixP->fx_addsy) != sec)
          || S_IS_EXTERNAL (fixP->fx_addsy)
          || S_IS_WEAK (fixP->fx_addsy)))
    {
        /* The symbol is undefined (or is defined but not in this section).
         Let the linker figure it out.  */
      return 0;
    }

  return fixP->fx_frag->fr_address + fixP->fx_where;
}


/* Return the bfd reloc type for OPERAND of INSN at fixup FIXP.
   Returns BFD_RELOC_NONE if no reloc type can be found.
   *FIXP may be modified if desired.  */

bfd_reloc_code_real_type
md_cgen_lookup_reloc (const CGEN_INSN *    insn ATTRIBUTE_UNUSED,
                      const CGEN_OPERAND * operand,
                      fixS *               fixP)
{
  if (fixP->fx_cgen.opinfo)
    return fixP->fx_cgen.opinfo;

  switch (operand->type)
    {
    case OR1K_OPERAND_DISP26:
      fixP->fx_pcrel = 1;
      return BFD_RELOC_OR1K_REL_26;

    default: /* avoid -Wall warning */
      return BFD_RELOC_NONE;
    }
}

/* Write a value out to the object file, using the appropriate endianness.  */

void
md_number_to_chars (char * buf, valueT val, int n)
{
  number_to_chars_bigendian (buf, val, n);
}

/* Turn a string in input_line_pointer into a floating point constant of type
   type, and store the appropriate bytes in *litP.  The number of LITTLENUMS
   emitted is stored in *sizeP .  An error message is returned, or NULL on OK.  */

const char *
md_atof (int type, char * litP, int *  sizeP)
{
  return ieee_md_atof (type, litP, sizeP, true);
}

bool
or1k_fix_adjustable (fixS * fixP)
{
  /* We need the symbol name for the VTABLE entries.  */
  if (fixP->fx_r_type == BFD_RELOC_VTABLE_INHERIT
      || fixP->fx_r_type == BFD_RELOC_VTABLE_ENTRY)
    return false;

  return true;
}

#define GOT_NAME "_GLOBAL_OFFSET_TABLE_"

arelent *
tc_gen_reloc (asection * section, fixS * fixp)
{
  arelent *reloc;
  bfd_reloc_code_real_type code;

  reloc = XNEW (arelent);

  reloc->sym_ptr_ptr = XNEW (asymbol *);
  *reloc->sym_ptr_ptr = symbol_get_bfdsym (fixp->fx_addsy);
  reloc->address = fixp->fx_frag->fr_address + fixp->fx_where;

  if (fixp->fx_pcrel)
    {
      if (section->use_rela_p)
        fixp->fx_offset -= md_pcrel_from_section (fixp, section);
      else
        fixp->fx_offset = reloc->address;
    }
  reloc->addend = fixp->fx_offset;

  code = fixp->fx_r_type;
  switch (code)
    {
    case BFD_RELOC_16:
      if (fixp->fx_pcrel)
        code = BFD_RELOC_16_PCREL;
      break;

    case BFD_RELOC_32:
      if (fixp->fx_pcrel)
        code = BFD_RELOC_32_PCREL;
      break;

    case BFD_RELOC_64:
      if (fixp->fx_pcrel)
        code = BFD_RELOC_64_PCREL;
      break;

    default:
      break;
    }

  reloc->howto = bfd_reloc_type_lookup (stdoutput, code);
  if (reloc->howto == NULL)
    {
      as_bad_where (fixp->fx_file, fixp->fx_line,
                    _
                    ("cannot represent %s relocation in this object file format"),
                    bfd_get_reloc_code_name (code));
      return NULL;
    }

  return reloc;
}

void
or1k_apply_fix (struct fix *f, valueT *t, segT s)
{
  gas_cgen_md_apply_fix (f, t, s);

  switch (f->fx_r_type)
    {
    case BFD_RELOC_OR1K_TLS_GD_HI16:
    case BFD_RELOC_OR1K_TLS_GD_LO16:
    case BFD_RELOC_OR1K_TLS_GD_PG21:
    case BFD_RELOC_OR1K_TLS_GD_LO13:
    case BFD_RELOC_OR1K_TLS_LDM_HI16:
    case BFD_RELOC_OR1K_TLS_LDM_LO16:
    case BFD_RELOC_OR1K_TLS_LDM_PG21:
    case BFD_RELOC_OR1K_TLS_LDM_LO13:
    case BFD_RELOC_OR1K_TLS_LDO_HI16:
    case BFD_RELOC_OR1K_TLS_LDO_LO16:
    case BFD_RELOC_OR1K_TLS_IE_HI16:
    case BFD_RELOC_OR1K_TLS_IE_LO16:
    case BFD_RELOC_OR1K_TLS_IE_PG21:
    case BFD_RELOC_OR1K_TLS_IE_LO13:
    case BFD_RELOC_OR1K_TLS_LE_HI16:
    case BFD_RELOC_OR1K_TLS_LE_LO16:
      S_SET_THREAD_LOCAL (f->fx_addsy);
      break;
    default:
      break;
    }
}

void
or1k_elf_final_processing (void)
{
  if (nodelay)
    elf_elfheader (stdoutput)->e_flags |= EF_OR1K_NODELAY;
}

/* Standard calling conventions leave the CFA at SP on entry.  */

void
or1k_cfi_frame_initial_instructions (void)
{
    cfi_add_CFA_def_cfa_register (1);
}

