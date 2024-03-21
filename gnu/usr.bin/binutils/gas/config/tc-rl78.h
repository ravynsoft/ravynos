/* tc-rl78.h - header file for Renesas RL78
   Copyright (C) 2011-2023 Free Software Foundation, Inc.

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

#define TC_RL78

extern int target_little_endian;

#define LISTING_HEADER "RL78 GAS LE"
#define LISTING_LHS_WIDTH 8
#define LISTING_WORD_SIZE 1

#define TARGET_ARCH bfd_arch_rl78

#define TARGET_BYTES_BIG_ENDIAN 0

#define TARGET_FORMAT "elf32-rl78"

/* We don't need to handle .word strangely.  */
#define WORKING_DOT_WORD

/* Permit temporary numeric labels.  */
#define LOCAL_LABELS_FB 1
/* But make sure that the binutils treat them as locals.  */
#define LOCAL_LABEL_PREFIX '.'

/* Allow classic-style constants.  */
#define NUMBERS_WITH_SUFFIX 1

/* .-foo gets turned into PC relative relocs.  */
#define DIFF_EXPR_OK

#define md_end rl78_md_end
extern void rl78_md_end (void);

#define md_relax_frag rl78_relax_frag
extern int rl78_relax_frag (segT, fragS *, long);

#define TC_FRAG_TYPE struct rl78_bytesT *
#define TC_FRAG_INIT(fragp, max_bytes) rl78_frag_init (fragp)
extern void rl78_frag_init (fragS *);

/* Call md_pcrel_from_section(), not md_pcrel_from().  */
#define MD_PCREL_FROM_SECTION(FIXP, SEC) md_pcrel_from_section (FIXP, SEC)

/* RL78 doesn't have a 32 bit PCREL relocations.  */
#define TC_FORCE_RELOCATION_SUB_LOCAL(FIX, SEG) 1

#define TC_VALIDATE_FIX_SUB(FIX, SEG)		\
  rl78_validate_fix_sub (FIX)
extern int rl78_validate_fix_sub (struct fix *);

#define TC_CONS_FIX_NEW(FRAG, WHERE, NBYTES, EXP, RET)	\
  rl78_cons_fix_new (FRAG, WHERE, NBYTES, EXP)
extern void rl78_cons_fix_new (fragS *, int, int, expressionS *);

#define tc_fix_adjustable(x) 0

#define RELOC_EXPANSION_POSSIBLE 1
#define MAX_RELOC_EXPANSION      8

#define MAX_MEM_FOR_RS_ALIGN_CODE 8
#define HANDLE_ALIGN(FRAG) rl78_handle_align (FRAG)
extern void rl78_handle_align (fragS *);

#define elf_tc_final_processing	rl78_elf_final_processing
extern void rl78_elf_final_processing (void);

#define TC_PARSE_CONS_EXPRESSION(EXP, NBYTES)	\
  ((EXP)->X_md = 0, expression (EXP), TC_PARSE_CONS_RETURN_NONE)

#define TC_LINKRELAX_FIXUP(seg) ((seg->flags & SEC_CODE) || (seg->flags & SEC_DEBUGGING))

/* Do not adjust relocations involving symbols in code sections,
   because it breaks linker relaxations.  This could be fixed in the
   linker, but this fix is simpler, and it pretty much only affects
   object size a little bit.  */
#define TC_FORCE_RELOCATION_SUB_SAME(FIX, SEC)	\
  (GENERIC_FORCE_RELOCATION_SUB_SAME (FIX, SEC)	\
   || ((SEC)->flags & SEC_CODE) != 0		\
   || ((SEC)->flags & SEC_DEBUGGING) != 0	\
   || TC_FORCE_RELOCATION (FIX))

#define DWARF2_USE_FIXED_ADVANCE_PC 1

#define TC_FORCE_RELOCATION(FIX) (linkrelax)
