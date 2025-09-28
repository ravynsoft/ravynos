/* tc-epiphany.h -- Header file for tc-epiphany.c.
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
   along with GAS; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

#define TC_EPIPHANY

#define LISTING_HEADER "EPIPHANY GAS "

/* The target BFD architecture.  */
#define TARGET_ARCH bfd_arch_epiphany

#define TARGET_FORMAT "elf32-epiphany"

/* Permit temporary numeric labels.  */
#define LOCAL_LABELS_FB 1

/* .-foo gets turned into PC relative relocs.  */
#define DIFF_EXPR_OK

/* We don't need to handle .word strangely.  */
#define WORKING_DOT_WORD

#define LITERAL_PREFIXPERCENT_BIN
#define DOUBLESLASH_LINE_COMMENTS

#define GAS_CGEN_PCREL_R_TYPE(R_TYPE) gas_cgen_pcrel_r_type (R_TYPE)

/* Values passed to md_apply_fix don't include the symbol value.  */
#define MD_APPLY_SYM_VALUE(FIX) 0

#define tc_fix_adjustable(FIX) epiphany_fix_adjustable (FIX)
extern bool epiphany_fix_adjustable (struct fix *);

#define MD_PCREL_FROM_SECTION(FIXP, SEC) md_pcrel_from_section (FIXP,SEC)

#define TC_HANDLES_FX_DONE

#define elf_tc_final_processing		epiphany_elf_final_processing
extern void epiphany_elf_final_processing (void);

#define md_elf_section_flags epiphany_elf_section_flags
extern int epiphany_elf_section_flags (int, int, int);

#define md_operand(x) epiphany_cgen_md_operand (x)
extern void epiphany_cgen_md_operand (expressionS *);

/* Values passed to md_apply_fix don't include the symbol value.  */
#define MD_APPLY_SYM_VALUE(FIX) 0

#define TC_CGEN_MAX_RELAX(insn, len)	4

#define O_PIC_reloc O_md1

#define TC_CGEN_PARSE_FIX_EXP(opinfo, exp) \
  epiphany_cgen_parse_fix_exp (opinfo, exp)
extern int epiphany_cgen_parse_fix_exp (int, expressionS *);

#define HANDLE_ALIGN(f)  epiphany_handle_align (f)
extern void epiphany_handle_align (fragS *);

#define TARGET_FORMAT "elf32-epiphany"

#define md_relax_frag epiphany_relax_frag

extern long epiphany_relax_frag (segT, fragS *, long);

/* If you don't define md_relax_frag, md_cgen_record_fixup_exp
   but do have TC_GENERIC_RELAX_TABLE gas will do the relaxation for you.

   If we have to add support for %LO and %HI relocations, we probably need
   to define the fixup_exp function to generate fancier relocations.  */

/* For 8 vs 24 bit branch selection.  */
extern const struct relax_type md_relax_table[];
#define TC_GENERIC_RELAX_TABLE md_relax_table

#define tc_gen_reloc gas_cgen_tc_gen_reloc


#define md_apply_fix epiphany_apply_fix
#include "write.h"

extern void epiphany_apply_fix (fixS *fixP, valueT *valP, segT seg);
