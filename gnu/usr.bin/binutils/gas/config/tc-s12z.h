/* tc-s12z.h -- Header file for tc-s12z.c.
   Copyright (C) 1999-2023 Free Software Foundation, Inc.

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

struct fix;

#define TARGET_BYTES_BIG_ENDIAN 1

/* Motorola assembler specs does not require '.' before pseudo-ops.  */
#define NO_PSEUDO_DOT 1

/* The target BFD architecture.  */
#define TARGET_ARCH (s12z_arch ())
extern enum bfd_architecture s12z_arch (void);

#define TARGET_MACH (s12z_mach ())
extern int s12z_mach (void);

#define TARGET_FORMAT (s12z_arch_format ())
extern const char *s12z_arch_format (void);

#define LISTING_WORD_SIZE 1	/* A word is 1 bytes */
#define LISTING_LHS_WIDTH 4	/* One word on the first line */
#define LISTING_LHS_WIDTH_SECOND 4	/* One word on the second line */
#define LISTING_LHS_CONT_LINES 4	/* And 4 lines max */
#define LISTING_HEADER s12z_listing_header ()
extern const char *s12z_listing_header (void);

/* Permit temporary numeric labels.  */
#define LOCAL_LABELS_FB 1

#define tc_init_after_args s12z_init_after_args
extern void s12z_init_after_args (void);

#define md_parse_long_option s12z_parse_long_option
extern int s12z_parse_long_option (char *);

#define DWARF2_LINE_MIN_INSN_LENGTH 1

/* Use 32-bit address to represent a symbol address so that we can
   represent them with their page number.  */
#define DWARF2_ADDR_SIZE(bfd) 4

/* We don't need to handle .word strangely.  */
#define WORKING_DOT_WORD

#define md_number_to_chars           number_to_chars_bigendian

/* Relax table to translate short relative branches (-128..127) into
   absolute branches.  */
#define TC_GENERIC_RELAX_TABLE md_relax_table
extern struct relax_type md_relax_table[];

/* GAS only handles relaxations for pc-relative data targeting addresses
   in the same segment, so we have to handle the rest on our own.  */
#define md_relax_frag(SEG, FRAGP, STRETCH)		\
 ((FRAGP)->fr_symbol != NULL				\
  && S_GET_SEGMENT ((FRAGP)->fr_symbol) == (SEG)	\
  ? relax_frag (SEG, FRAGP, STRETCH)			\
  : s12z_relax_frag (SEG, FRAGP, STRETCH))
extern long s12z_relax_frag (segT, fragS*, long);

#define TC_HANDLES_FX_DONE

#define DIFF_EXPR_OK		/* .-foo gets turned into PC relative relocs */

/* Values passed to md_apply_fix don't include the symbol value.  */
#define MD_APPLY_SYM_VALUE(FIX) 0

/* No shared lib support, so we don't need to ensure externally
   visible symbols can be overridden.  */
#define EXTERN_FORCE_RELOC 0

#define TC_FORCE_RELOCATION(fix) tc_s12z_force_relocation (fix)
extern int tc_s12z_force_relocation (struct fix *);

#define tc_fix_adjustable(X) tc_s12z_fix_adjustable(X)
extern bool tc_s12z_fix_adjustable (struct fix *);

#define md_operand(x)

#define elf_tc_final_processing	s12z_elf_final_processing
extern void s12z_elf_final_processing (void);

#define tc_print_statistics(FILE) s12z_print_statistics (FILE)
extern void s12z_print_statistics (FILE *);
