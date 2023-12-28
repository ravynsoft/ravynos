/* tc-ft32.h -- Header file for tc-ft32.c.

   Copyright (C) 2013-2023 Free Software Foundation, Inc.
   Contributed by FTDI (support@ftdichip.com)

   This file is part of GAS, the GNU Assembler.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GAS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with GAS; see the file COPYING.  If not, write to the Free Software
   Foundation, 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.  */

#define TC_FT32 1
#define TARGET_BYTES_BIG_ENDIAN 0
#define WORKING_DOT_WORD

/* This macro is the BFD architecture to pass to `bfd_set_arch_mach'.  */
#define TARGET_FORMAT  "elf32-ft32"

#define TARGET_ARCH bfd_arch_ft32

#define md_undefined_symbol(NAME)           0

/* These macros must be defined, but is will be a fatal assembler
   error if we ever hit them.  */
#define md_estimate_size_before_relax(A, B) (as_fatal (_("estimate size\n")), 0)
#define md_convert_frag(B, S, F)            (as_fatal (_("convert_frag\n")))

/* PC relative operands are relative to the start of the opcode, and
   the operand is always one byte into the opcode.  */
#define md_pcrel_from(FIX)						\
	((FIX)->fx_where + (FIX)->fx_frag->fr_address - 1)

#define md_section_align(SEGMENT, SIZE)     (SIZE)

/* If this macro returns non-zero, it guarantees that a relocation will be emitted
   even when the value can be resolved locally. Do that if linkrelax is turned on */
#define TC_FORCE_RELOCATION(fix)	ft32_force_relocation (fix)
#define TC_FORCE_RELOCATION_SUB_SAME(fix, seg) \
  (! SEG_NORMAL (seg) || ft32_force_relocation (fix))
extern int ft32_force_relocation (struct fix *);

#define TC_LINKRELAX_FIXUP(seg) \
  ((seg->flags & SEC_CODE) || (seg->flags & SEC_DEBUGGING))

/* This macro is evaluated for any fixup with a fx_subsy that
   fixup_segment cannot reduce to a number.  If the macro returns
   false an error will be reported. */
#define TC_VALIDATE_FIX_SUB(fix, seg)   ft32_validate_fix_sub (fix)
extern int ft32_validate_fix_sub (struct fix *);

/* The difference between same-section symbols may be affected by linker
   relaxation, so do not resolve such expressions in the assembler.  */
#define md_allow_local_subtract(l,r,s) ft32_allow_local_subtract (l, r, s)
extern bool ft32_allow_local_subtract (expressionS *, expressionS *, segT);

#define md_operand(x)
