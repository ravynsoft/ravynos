/* This file is tc-wasm32.h.
   Copyright (C) 2017-2023 Free Software Foundation, Inc.

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

#define TC_WASM32
#define TARGET_FORMAT                       "elf32-wasm32"
#define TARGET_ARCH                        bfd_arch_wasm32
#define TARGET_MACH                                      1

/* WebAssembly is strictly little-endian.  */
#define TARGET_BYTES_BIG_ENDIAN                          0
#define md_number_to_chars    number_to_chars_littleendian

#define DIFF_EXPR_OK

/* No machine-dependent operand expressions.  */
#define md_operand(x)

/* No broken word processing.  */
#define WORKING_DOT_WORD

/* Force some relocations.  */
#define EXTERN_FORCE_RELOC                     1
extern int wasm32_force_relocation (struct fix *);
#define TC_FORCE_RELOCATION(fix)               wasm32_force_relocation (fix)
#define TC_FORCE_RELOCATION_LOCAL(fix)         1
#define TC_FORCE_RELOCATION_SUB_SAME(fix,seg)  wasm32_force_relocation (fix)
#define TC_FORCE_RELOCATION_SUB_ABS(fix,seg)   wasm32_force_relocation (fix)
#define TC_FORCE_RELOCATION_SUB_LOCAL(fix,seg) wasm32_force_relocation (fix)
#define TC_VALIDATE_FIX_SUB(fix,seg)           wasm32_force_relocation (fix)

/* This is ELF, values passed to md_apply_fix don't include the symbol
   value.  */
#define MD_APPLY_SYM_VALUE(FIX)         0

/* PC-relative relocations are relative to the relocation offset.  */
#define MD_PCREL_FROM_SECTION(FIX, SEC) 0

#define DWARF2_LINE_MIN_INSN_LENGTH 	1

/* WebAssembly uses 32-bit addresses.  */
#define TC_ADDRESS_BYTES()              4
#define DWARF2_ADDR_SIZE(bfd)           4

/* Enable cfi directives.  */
#define TARGET_USE_CFIPOP               1

/* The stack grows down, and there is no harm in claiming it is only
   byte aligned.  */
#define DWARF2_CIE_DATA_ALIGNMENT      -1

/* Define the column that represents the PC.  FIXME: this depends on
   the ABI. */
#define DWARF2_DEFAULT_RETURN_COLUMN   36

/* Define a hook to setup initial CFI state.  */
#define tc_cfi_frame_initial_instructions() do { } while (0)

#define elf_tc_final_processing()
#define md_post_relax_hook
#define md_start_line_hook()
#define HANDLE_ALIGN(fragP)


extern bool wasm32_fix_adjustable (struct fix *);
#define tc_fix_adjustable(FIX) wasm32_fix_adjustable (FIX)

/* Type names for blocks and signatures.  */
#define BLOCK_TYPE_NONE              0x40
#define BLOCK_TYPE_I32               0x7f
#define BLOCK_TYPE_I64               0x7e
#define BLOCK_TYPE_F32               0x7d
#define BLOCK_TYPE_F64               0x7c
