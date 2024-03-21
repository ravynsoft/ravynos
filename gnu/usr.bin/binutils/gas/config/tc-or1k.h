/* tc-or1k.h -- Header file for tc-or1k.c.
   Copyright (C) 2001-2023 Free Software Foundation, Inc.

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

#define TC_OR1K

#define LISTING_HEADER "Or1k GAS "

/* The target BFD architecture.  */
#define TARGET_ARCH bfd_arch_or1k

extern unsigned long or1k_machine;
#define TARGET_MACH (or1k_machine)

#define TARGET_FORMAT           "elf32-or1k"
#define TARGET_BYTES_BIG_ENDIAN 1

extern const char or1k_comment_chars [];
#define tc_comment_chars or1k_comment_chars

/* Permit temporary numeric labels.  */
#define LOCAL_LABELS_FB 1

#define DIFF_EXPR_OK    1       /* .-foo gets turned into PC relative relocs.  */

/* We don't need to handle .word strangely.  */
#define WORKING_DOT_WORD

/* Values passed to md_apply_fix don't include the symbol value.  */
#define MD_APPLY_SYM_VALUE(FIX) 0

#define md_apply_fix or1k_apply_fix
extern void or1k_apply_fix (struct fix *, valueT *, segT);

extern bool or1k_fix_adjustable (struct fix *);
#define tc_fix_adjustable(FIX) or1k_fix_adjustable (FIX)

/* Call md_pcrel_from_section(), not md_pcrel_from().  */
#define MD_PCREL_FROM_SECTION(FIX, SEC) md_pcrel_from_section (FIX, SEC)

/* For 8 vs 16 vs 32 bit branch selection.  */
extern const struct relax_type md_relax_table[];
#define TC_GENERIC_RELAX_TABLE md_relax_table

#define elf_tc_final_processing or1k_elf_final_processing
void or1k_elf_final_processing (void);

/* Enable cfi directives.  */
#define TARGET_USE_CFIPOP 1

/* Stack grows to lower addresses and wants 4 byte boundary.  */
#define DWARF2_CIE_DATA_ALIGNMENT -4

/* Define the column that represents the PC.  */
#define DWARF2_DEFAULT_RETURN_COLUMN 9

/* or1k instructions are 4 bytes long.  */
#define DWARF2_LINE_MIN_INSN_LENGTH     4

#define tc_cfi_frame_initial_instructions \
    or1k_cfi_frame_initial_instructions
extern void or1k_cfi_frame_initial_instructions (void);

#define md_single_noop_insn "l.nop"
