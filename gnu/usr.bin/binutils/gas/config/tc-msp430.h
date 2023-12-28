/* This file is tc-msp430.h
   Copyright (C) 2002-2023 Free Software Foundation, Inc.

   Contributed by Dmitry Diky <diwil@mail.ru>

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

#define TC_MSP430
/*   By convention, you should define this macro in the `.h' file.  For
     example, `tc-m68k.h' defines `TC_M68K'.  You might have to use this
     if it is necessary to add CPU specific code to the object format
     file.  */

#define TARGET_FORMAT "elf32-msp430"
/*   This macro is the BFD target name to use when creating the output
     file.  This will normally depend upon the `OBJ_FMT' macro.  */

#define TARGET_ARCH bfd_arch_msp430
/*   This macro is the BFD architecture to pass to `bfd_set_arch_mach'.  */

#define TARGET_MACH 0
/*   This macro is the BFD machine number to pass to
     `bfd_set_arch_mach'.  If it is not defined, GAS will use 0.  */

#define TARGET_BYTES_BIG_ENDIAN 0
/*   You should define this macro to be non-zero if the target is big
     endian, and zero if the target is little endian.  */

#define ONLY_STANDARD_ESCAPES
/*   If you define this macro, GAS will warn about the use of
     nonstandard escape sequences in a string.  */

#define md_operand(x)
/*   GAS will call this function for any expression that can not be
     recognized.  When the function is called, `input_line_pointer'
     will point to the start of the expression.  */

#define md_number_to_chars number_to_chars_littleendian
/*   This should just call either `number_to_chars_bigendian' or
     `number_to_chars_littleendian', whichever is appropriate.  On
     targets like the MIPS which support options to change the
     endianness, which function to call is a runtime decision.  On
     other targets, `md_number_to_chars' can be a simple macro.  */

#define WORKING_DOT_WORD
/*
`md_short_jump_size'
`md_long_jump_size'
`md_create_short_jump'
`md_create_long_jump'
     If `WORKING_DOT_WORD' is defined, GAS will not do broken word
     processing (*note Broken words::.).  Otherwise, you should set
     `md_short_jump_size' to the size of a short jump (a jump that is
     just long enough to jump around a long jmp) and
     `md_long_jump_size' to the size of a long jump (a jump that can go
     anywhere in the function), You should define
     `md_create_short_jump' to create a short jump around a long jump,
     and define `md_create_long_jump' to create a long jump.  */

#define MD_APPLY_FIX3
/* Values passed to md_apply_fix don't include symbol values.  */
#define MD_APPLY_SYM_VALUE(FIX) 0

#define TC_HANDLES_FX_DONE

#undef RELOC_EXPANSION_POSSIBLE
/*   If you define this macro, it means that `tc_gen_reloc' may return
     multiple relocation entries for a single fixup.  In this case, the
     return value of `tc_gen_reloc' is a pointer to a null terminated
     array.  */

#define MD_PCREL_FROM_SECTION(FIXP, SEC) md_pcrel_from_section(FIXP, SEC)
/*   If you define this macro, it should return the offset between the
     address of a PC relative fixup and the position from which the PC
     relative adjustment should be made.  On many processors, the base
     of a PC relative instruction is the next instruction, so this
     macro would return the length of an instruction.  */

#define LISTING_WORD_SIZE 2
/*   The number of bytes to put into a word in a listing.  This affects
     the way the bytes are clumped together in the listing.  For
     example, a value of 2 might print `1234 5678' where a value of 1
     would print `12 34 56 78'.  The default value is 4.  */

/* Support symbols like: C$$IO$$.  */
#undef  LEX_DOLLAR
#define LEX_DOLLAR 1

#define TC_IMPLICIT_LCOMM_ALIGNMENT(SIZE, P2VAR) (P2VAR) = 0
/*   An `.lcomm' directive with no explicit alignment parameter will
     use this macro to set P2VAR to the alignment that a request for
     SIZE bytes will have.  The alignment is expressed as a power of
     two.  If no alignment should take place, the macro definition
     should do nothing.  Some targets define a `.bss' directive that is
     also affected by this macro.  The default definition will set
     P2VAR to the truncated power of two of sizes up to eight bytes.  */

#define md_relax_frag(SEG, FRAGP, STRETCH)             \
   msp430_relax_frag (SEG, FRAGP, STRETCH)
extern long msp430_relax_frag (segT, fragS *, long);

#define TC_FORCE_RELOCATION_LOCAL(FIX)		\
  (GENERIC_FORCE_RELOCATION_LOCAL (FIX)		\
   || msp430_force_relocation_local (FIX))
extern int msp430_force_relocation_local (struct fix *);

/* We need to add reference symbols for .data/.bss.  */
#define tc_frob_section(sec) msp430_frob_section (sec)
extern void msp430_frob_section (asection *);

extern int msp430_enable_relax;
extern int msp430_enable_polys;

#define tc_fix_adjustable(FIX) msp430_fix_adjustable (FIX)
extern bool msp430_fix_adjustable (struct fix *);

/* Allow hexadecimal numbers with 'h' suffix.  Note that if the number
   starts with a letter it will be interpreted as a symbol name not a
   constant.  Thus "beach" is a symbol not the hex value 0xbeac.  So
   is A5A5h...  */
#define NUMBERS_WITH_SUFFIX 1

#define md_finish msp430_md_finish
extern void    msp430_md_finish (void);

/* Do not allow call frame debug info optimization as otherwise we could
   generate the DWARF directives without the relocs necessary to patch
   them up.  */
#define md_allow_eh_opt 0

/* The difference between same-section symbols may be affected by linker
   relaxation, so do not resolve such expressions in the assembler.  */
#define md_allow_local_subtract(l,r,s) msp430_allow_local_subtract (l, r, s)
extern bool msp430_allow_local_subtract (expressionS *, expressionS *, segT);

#define RELOC_EXPANSION_POSSIBLE
#define MAX_RELOC_EXPANSION 2

#define DIFF_EXPR_OK

/* Do not adjust relocations involving symbols in code sections,
   because it breaks linker relaxations.  This could be fixed in the
   linker, but this fix is simpler, and it pretty much only affects
   object size a little bit.  */
#define TC_FORCE_RELOCATION_SUB_SAME(FIX, SEC)	\
  (GENERIC_FORCE_RELOCATION_SUB_SAME (FIX, SEC)	\
   || ((SEC)->flags & SEC_CODE) != 0		\
   || ((SEC)->flags & SEC_DEBUGGING) != 0	\
   || TC_FORCE_RELOCATION (FIX))

/* We validate subtract arguments within tc_gen_reloc(),
   so don't report errors at this point.  */
#define TC_VALIDATE_FIX_SUB(FIX, SEG) 1

#define DWARF2_USE_FIXED_ADVANCE_PC 1

#define TC_LINKRELAX_FIXUP(seg) ((seg->flags & SEC_CODE) || (seg->flags & SEC_DEBUGGING))

#define DWARF2_ADDR_SIZE(bfd) 4
