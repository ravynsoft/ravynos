/* tc-csky.h -- Header file for tc-csky.c
   Copyright (C) 1989-2023 Free Software Foundation, Inc.
   Contributed by C-SKY Microsystems and Mentor Graphics.

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

#ifndef TC_CSKY
#define TC_CSKY 1

#define WORKING_DOT_WORD

#define TARGET_ARCH    bfd_arch_csky

#define LISTING_HEADER "CSKY GAS"

#ifdef OBJ_ELF
#define TARGET_FORMAT elf32_csky_target_format ()
#endif

#define TARGET_BYTES_BIG_ENDIAN 0

#define MD_PCREL_FROM_SECTION(F,S) md_pcrel_from_section (F, S)

#define TC_GENERIC_RELAX_TABLE     csky_relax_table

#define md_finish                        csky_md_finish
#define md_relax_frag                 csky_relax_frag
#define DOUBLESLASH_LINE_COMMENTS
#define LOCAL_LABELS_FB     1
#define PAD_LITERAL_LENGTH  6
#define PAD_FILL_CONTENT    0x1c00

/* Reloc API.  */
#define EXTERN_FORCE_RELOC  1
#define TC_CONS_FIX_NEW               csky_cons_fix_new
#define TC_FORCE_RELOCATION(fix)      csky_force_relocation (fix)
#define tc_fix_adjustable(FIX)        csky_fix_adjustable (FIX)
#define TC_SEGMENT_INFO_TYPE          csky_segment_info_type

/* Dwarf API.  */
#define DWARF2_LINE_MIN_INSN_LENGTH   2
#define DWARF2_ADDR_SIZE(bfd)   4
#define DWARF2_FDE_RELOC_SIZE   4
#define TARGET_USE_CFIPOP 1
#define tc_cfi_frame_initial_instructions csky_cfi_frame_initial_instructions
#define tc_regname_to_dw2regnum tc_csky_regname_to_dw2regnum
#define DWARF2_DEFAULT_RETURN_COLUMN 15
#define DWARF2_CIE_DATA_ALIGNMENT (-4)

/* .-foo gets turned into PC relative relocs.  */
#define DIFF_EXPR_OK 1

typedef enum
{
  MAP_UNDEFINED = 0,
  MAP_DATA,
  MAP_TEXT,
} map_state;

typedef struct
{
  map_state current_state;
} csky_segment_info_type;

struct tls_addend
{
  fragS *frag;
  offsetT offset;
};

#define TC_FIX_TYPE struct tls_addend
#define TC_INIT_FIX_DATA(FIX) \
  { (FIX)->tc_fix_data.frag = NULL; (FIX)->tc_fix_data.offset = 0; }

#include "write.h"
extern const relax_typeS csky_relax_table [];

extern void csky_md_finish (void);
extern void csky_cons_fix_new (fragS *,
			       unsigned int off,
			       unsigned int len,
			       expressionS *,
			       bfd_reloc_code_real_type);
extern int csky_force_relocation (fixS *);
extern bool csky_fix_adjustable (fixS *);
extern void csky_cfi_frame_initial_instructions (void);
extern int tc_csky_regname_to_dw2regnum (char *);
extern long csky_relax_frag (segT, fragS *, long);

#ifdef OBJ_ELF
const char * elf32_csky_target_format (void);
#endif

#endif
