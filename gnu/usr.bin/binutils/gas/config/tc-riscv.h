/* tc-riscv.h -- header file for tc-riscv.c.
   Copyright (C) 2011-2023 Free Software Foundation, Inc.

   Contributed by Andrew Waterman (andrew@sifive.com).
   Based on MIPS target.

   This file is part of GAS.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GAS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING3. If not,
   see <http://www.gnu.org/licenses/>.  */

#ifndef TC_RISCV
#define TC_RISCV

#include "opcode/riscv.h"

struct frag;
struct expressionS;

#ifndef TARGET_BYTES_BIG_ENDIAN
#define TARGET_BYTES_BIG_ENDIAN 0
#endif

#define TARGET_ARCH bfd_arch_riscv

#define WORKING_DOT_WORD	1
#define LOCAL_LABELS_FB 	1

/* Symbols named FAKE_LABEL_NAME are emitted when generating DWARF, so make
   sure FAKE_LABEL_NAME is printable.  It still must be distinct from any
   real label name.  So, append a space, which other labels can't contain.  */
#define FAKE_LABEL_NAME RISCV_FAKE_LABEL_NAME
/* Changing the special character in FAKE_LABEL_NAME requires changing
   FAKE_LABEL_CHAR too.  */
#define FAKE_LABEL_CHAR RISCV_FAKE_LABEL_CHAR

#define md_relax_frag(segment, fragp, stretch) \
  riscv_relax_frag (segment, fragp, stretch)
extern int riscv_relax_frag (asection *, struct frag *, long);

#define md_section_align(seg,size)	(size)
#define md_undefined_symbol(name)	(0)
#define md_operand(x)

extern bool riscv_frag_align_code (int);
#define md_do_align(N, FILL, LEN, MAX, LABEL)				\
  if ((N) != 0 && !(FILL) && !need_pass_2 && subseg_text_p (now_seg))	\
    {									\
      if (riscv_frag_align_code (N))					\
	goto LABEL;							\
    }

extern void riscv_handle_align (fragS *);
#define HANDLE_ALIGN riscv_handle_align

#define MAX_MEM_FOR_RS_ALIGN_CODE (3 + 4)

/* The ISA of the target may change based on command-line arguments.  */
#define TARGET_FORMAT riscv_target_format ()
extern const char * riscv_target_format (void);

#define md_after_parse_args() riscv_after_parse_args ()
extern void riscv_after_parse_args (void);

#define md_parse_long_option(arg) riscv_parse_long_option (arg)
extern int riscv_parse_long_option (const char *);

#define md_pre_output_hook riscv_pre_output_hook ()
extern void riscv_pre_output_hook (void);
#define GAS_SORT_RELOCS 1

/* Let the linker resolve all the relocs due to relaxation.  */
#define tc_fix_adjustable(fixp) 0
#define md_allow_local_subtract(l,r,s) 0

/* Values passed to md_apply_fix don't include symbol values.  */
#define MD_APPLY_SYM_VALUE(FIX) 0

/* Global syms must not be resolved, to support ELF shared libraries.  */
#define EXTERN_FORCE_RELOC			\
  (OUTPUT_FLAVOR == bfd_target_elf_flavour)

/* Postpone text-section label subtraction calculation until linking, since
   linker relaxations might change the deltas.  */
#define TC_FORCE_RELOCATION_SUB_SAME(FIX, SEG)	\
  (GENERIC_FORCE_RELOCATION_SUB_SAME (FIX, SEG)	\
   || ((SEG)->flags & SEC_CODE) != 0)
#define TC_FORCE_RELOCATION_SUB_LOCAL(FIX, SEG) 1
#define TC_VALIDATE_FIX_SUB(FIX, SEG) 1
#define TC_FORCE_RELOCATION_LOCAL(FIX) 1
#define DIFF_EXPR_OK 1

extern void riscv_pop_insert (void);
#define md_pop_insert()		riscv_pop_insert ()

#define TARGET_USE_CFIPOP 1

#define tc_cfi_frame_initial_instructions riscv_cfi_frame_initial_instructions
extern void riscv_cfi_frame_initial_instructions (void);

#define tc_regname_to_dw2regnum tc_riscv_regname_to_dw2regnum
extern int tc_riscv_regname_to_dw2regnum (char *);

#define DWARF2_DEFAULT_RETURN_COLUMN X_RA

/* Even on RV64, use 4-byte alignment, as F registers may be only 32 bits.  */
#define DWARF2_CIE_DATA_ALIGNMENT -4

#define elf_tc_final_processing riscv_elf_final_processing
extern void riscv_elf_final_processing (void);

/* Adjust debug_line after relaxation.  */
#define DWARF2_USE_FIXED_ADVANCE_PC 1

#define md_parse_name(name, exp, mode, c) \
  riscv_parse_name (name, exp, mode)
bool riscv_parse_name (const char *, struct expressionS *, enum expr_mode);

#define md_finish riscv_md_finish
#define CONVERT_SYMBOLIC_ATTRIBUTE riscv_convert_symbolic_attribute

extern void riscv_md_finish (void);
extern int riscv_convert_symbolic_attribute (const char *);

/* Set mapping symbol states.  */
#define md_cons_align(nbytes) riscv_mapping_state (MAP_DATA, 0, 0)
void riscv_mapping_state (enum riscv_seg_mstate, int, bool);

/* Define target segment type.  */
#define TC_SEGMENT_INFO_TYPE struct riscv_segment_info_type
struct riscv_segment_info_type
{
  enum riscv_seg_mstate map_state;
  /* The current mapping symbol with architecture string.  */
  symbolS *arch_map_symbol;
};

/* Define target fragment type.  */
#define TC_FRAG_TYPE struct riscv_frag_type
struct riscv_frag_type
{
  symbolS *first_map_symbol, *last_map_symbol;
};

#define TC_FRAG_INIT(fragp, max_bytes) riscv_init_frag (fragp, max_bytes)
extern void riscv_init_frag (struct frag *, int);

#define obj_adjust_symtab() riscv_adjust_symtab ()
extern void riscv_adjust_symtab (void);

void riscv_elf_copy_symbol_attributes (symbolS *, symbolS *);
#define OBJ_COPY_SYMBOL_ATTRIBUTES(DEST, SRC)  \
  riscv_elf_copy_symbol_attributes (DEST, SRC)

#endif /* TC_RISCV */
