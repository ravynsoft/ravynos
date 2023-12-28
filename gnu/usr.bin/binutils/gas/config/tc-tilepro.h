/* tc-tile.h - Macros and type defines for a TILEPro chip.
   Copyright (C) 2011-2023 Free Software Foundation, Inc.

   This file is part of GAS, the GNU Assembler.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#ifndef TC_TILEPRO

#include "opcode/tilepro.h"

#define TC_TILEPRO

#define TARGET_BYTES_BIG_ENDIAN 0

#define WORKING_DOT_WORD

#define TARGET_ARCH	bfd_arch_tilepro

#define TARGET_FORMAT	"elf32-tilepro"


#define DWARF2_LINE_MIN_INSN_LENGTH	8

#define md_number_to_chars		number_to_chars_littleendian

#define DIFF_EXPR_OK   /* foo-. gets turned into PC relative relocs */

#define HANDLE_ALIGN(fragp) tilepro_handle_align (fragp)
extern void tilepro_handle_align (struct frag *);

#define MAX_MEM_FOR_RS_ALIGN_CODE (7 + 8)

struct tilepro_operand;
#define TC_FIX_TYPE const struct tilepro_operand *

/* Initialize the TC_FIX_TYPE field.  */
#define TC_INIT_FIX_DATA(FIX) \
  FIX->tc_fix_data = 0

extern void tilepro_cons_fix_new (struct frag *, int,
				  int, struct expressionS *);

#define TC_CONS_FIX_NEW(FRAG, WHERE, NBYTES, EXP, RELOC)	\
  tilepro_cons_fix_new (FRAG, WHERE, NBYTES, EXP)

extern int tilepro_parse_name (char *, expressionS *, char *);
#define md_parse_name(name, e, m, nextP) tilepro_parse_name (name, e, nextP)

extern int tilepro_fix_adjustable (struct fix *);
#define tc_fix_adjustable(FIX)   tilepro_fix_adjustable (FIX)

extern int tilepro_unrecognized_line (int);
#define tc_unrecognized_line(ch) tilepro_unrecognized_line (ch)

/* Values passed to md_apply_fix3 don't include the symbol value.  */
#define MD_APPLY_SYM_VALUE(FIX) 0

#define md_convert_frag(b,s,f) \
  as_fatal ("tilepro convert_frag called")
#define md_estimate_size_before_relax(f,s) \
  (as_fatal ("tilepro estimate_size_before_relax called"),1)
#define md_operand(x)

#define md_section_align(seg,size)	(size)

/* We want .cfi_* pseudo-ops for generating unwind info.  */
#define TARGET_USE_CFIPOP 1

#define tc_cfi_frame_initial_instructions \
  tilepro_cfi_frame_initial_instructions
extern void tilepro_cfi_frame_initial_instructions (void);

#define tc_regname_to_dw2regnum tc_tilepro_regname_to_dw2regnum
extern int tc_tilepro_regname_to_dw2regnum (char *);

#define DWARF2_DEFAULT_RETURN_COLUMN  55
#define DWARF2_CIE_DATA_ALIGNMENT     (-4)

#endif /* TC_TILEPRO */
