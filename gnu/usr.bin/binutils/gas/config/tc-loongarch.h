/* tc-loongarch.h -- Header file for tc-loongarch.c.
   Copyright (C) 2021-2023 Free Software Foundation, Inc.
   Contributed by Loongson Ltd.

   This file is part of GAS.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the license, or
   (at your option) any later version.

   GAS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING3.  If not,
   see <http://www.gnu.org/licenses/>.  */

#ifndef TC_LOONGARCH
#define TC_LOONGARCH

#include "opcode/loongarch.h"

#define TARGET_BYTES_BIG_ENDIAN 0
#define TARGET_ARCH bfd_arch_loongarch

#define TARGET_MACH (loongarch_mach ())
extern unsigned long loongarch_mach (void);

#define WORKING_DOT_WORD 1
#define REPEAT_CONS_EXPRESSIONS

/* Early than md_begin.  */
#define md_after_parse_args loongarch_after_parse_args
extern void loongarch_after_parse_args (void);

extern void loongarch_pop_insert (void);
#define md_pop_insert() loongarch_pop_insert ()

#define TARGET_FORMAT loongarch_target_format ()
extern const char *loongarch_target_format (void);

#define md_relax_frag(segment, fragp, stretch)	\
  loongarch_relax_frag (segment, fragp, stretch)
extern int loongarch_relax_frag (asection *, struct frag *, long);
#define md_section_align(seg, size) (size)
#define md_undefined_symbol(name) (0)
#define md_operand(x)

extern bool loongarch_frag_align_code (int);
#define md_do_align(N, FILL, LEN, MAX, LABEL)				\
  if ((N) != 0 && !(FILL) && !need_pass_2 && subseg_text_p (now_seg))	\
    {									\
      if (loongarch_frag_align_code (N))				\
	goto LABEL;							\
    }

/* The following two macros let the linker resolve all the relocs
   due to relaxation.

   This is called to see whether a reloc against a defined symbol
   should be converted into a reloc against a section.

   If relax and norelax have different value may cause ld ".eh_frame_hdr
   refers to overlapping FDEs" error when link relax .o and norelax .o.  */
#define tc_fix_adjustable(fixp) 0

/* Tne difference between same-section symbols may be affected by linker
   relaxation, so do not resolve such expressions in the assembler.  */
#define md_allow_local_subtract(l,r,s) 0

/* Values passed to md_apply_fix don't include symbol values.  */
#define TC_FORCE_RELOCATION_SUB_LOCAL(FIX, SEG) 1

#define TC_VALIDATE_FIX_SUB(FIX, SEG) 1
#define DIFF_EXPR_OK 1

/* Postpone text-section label subtraction calculation until linking, since
   linker relaxations might change the deltas.  */
#define TC_FORCE_RELOCATION_SUB_SAME(FIX, SEC)	\
  (LARCH_opts.relax ?  \
    (GENERIC_FORCE_RELOCATION_SUB_SAME (FIX, SEC)	\
      || ((SEC)->flags & SEC_CODE) != 0		\
      || ((SEC)->flags & SEC_DEBUGGING) != 0	\
      || TC_FORCE_RELOCATION (FIX)) \
    : (GENERIC_FORCE_RELOCATION_SUB_SAME (FIX, SEC))) \

#define TC_LINKRELAX_FIXUP(seg) ((seg->flags & SEC_CODE)  \
				    || (seg->flags & SEC_DEBUGGING))

#define TC_FORCE_RELOCATION_LOCAL(FIX) 1

/* Adjust debug_line after relaxation.  */
#define DWARF2_USE_FIXED_ADVANCE_PC 1

/* Values passed to md_apply_fix don't include symbol values.  */
#define MD_APPLY_SYM_VALUE(FIX) 0

#define TARGET_USE_CFIPOP 1
#define DWARF2_DEFAULT_RETURN_COLUMN 1 /* $ra.  */
#define DWARF2_CIE_DATA_ALIGNMENT -4

#define tc_cfi_frame_initial_instructions	\
  loongarch_cfi_frame_initial_instructions
extern void loongarch_cfi_frame_initial_instructions (void);

#define tc_symbol_new_hook(sym) \
  if (0 == strcmp (sym->bsym->name, FAKE_LABEL_NAME)) \
    S_SET_OTHER (sym, STV_HIDDEN);

#define tc_parse_to_dw2regnum tc_loongarch_parse_to_dw2regnum
extern void tc_loongarch_parse_to_dw2regnum (expressionS *);

extern void loongarch_pre_output_hook (void);
#define md_pre_output_hook loongarch_pre_output_hook ()

#define SUB_SEGMENT_ALIGN(SEG, FRCHAIN) 0

#define HANDLE_ALIGN(fragp) loongarch_handle_align (fragp)
extern void loongarch_handle_align (struct frag *);
#define MAX_MEM_FOR_RS_ALIGN_CODE (3 + 4)

#define elf_tc_final_processing loongarch_elf_final_processing
extern void loongarch_elf_final_processing (void);

#define MAX_RELOC_NUMBER_A_INSN 20

struct reloc_info
{
  bfd_reloc_code_real_type type;
  expressionS value;
};

#define md_finish loongarch_md_finish
extern void loongarch_md_finish (void);

#endif
