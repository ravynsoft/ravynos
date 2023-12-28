/* tc-visium.h -- Header file for tc-visium.c.

   Copyright (C) 2005-2023 Free Software Foundation, Inc.

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
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#define TC_VISIUM

#define LISTING_HEADER "VISIUM GAS "

/* The target BFD architecture.  */
#define TARGET_ARCH bfd_arch_visium

#define TARGET_BYTES_BIG_ENDIAN 1

#define TARGET_FORMAT "elf32-visium"

/* Permit temporary numeric labels.  */
#define LOCAL_LABELS_FB 1

/* .-foo gets turned into PC relative relocs.  */
#define DIFF_EXPR_OK

/* We don't support external symbols overriding.  */
#define EXTERN_FORCE_RELOC 0

/* We don't need to handle .word strangely.  */
#define WORKING_DOT_WORD

#define tc_fix_adjustable(FIXP) visium_fix_adjustable (FIXP)
extern bool visium_fix_adjustable (struct fix *);

#define HANDLE_ALIGN(FRAGP)		 \
  if ((FRAGP)->fr_type == rs_align_code) \
    visium_handle_align (FRAGP);
extern void visium_handle_align (struct frag *);

#define md_relax_frag(segment, fragp, stretch) \
  visium_relax_frag (segment, fragp, stretch)
extern int visium_relax_frag (asection *, struct frag *, long);

/* Call md_pcrel_from_section, not md_pcrel_from.  */
#define MD_PCREL_FROM_SECTION(FIXP, SEC) visium_pcrel_from_section (FIXP, SEC)
extern long visium_pcrel_from_section (struct fix *, segT);

/* Values passed to md_apply_fix3 don't include the symbol value.  */
#define MD_APPLY_SYM_VALUE(FIX) 0

#define md_operand(x)

#define tc_comment_chars visium_comment_chars
extern const char *visium_comment_chars;

#define TARGET_USE_CFIPOP 1

#define tc_cfi_frame_initial_instructions visium_cfi_frame_initial_instructions
extern void visium_cfi_frame_initial_instructions (void);

#define tc_regname_to_dw2regnum visium_regname_to_dw2regnum
extern int visium_regname_to_dw2regnum (char *regname);

#define DWARF2_LINE_MIN_INSN_LENGTH     4
#define DWARF2_DEFAULT_RETURN_COLUMN    21
#define DWARF2_CIE_DATA_ALIGNMENT       (-4)
