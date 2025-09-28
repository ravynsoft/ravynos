/* Definitions for Altera Nios II assembler.
   Copyright (C) 2012-2023 Free Software Foundation, Inc.
   Contributed by Nigel Gray (ngray@altera.com).
   Contributed by Mentor Graphics, Inc.

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

#ifndef TC_NIOS2
#define TC_NIOS2

/* If unspecified, default to little endian.  We can explicitly specify
 * a big-endian default by configuring with --target=nios2eb-elf.  We
 * can override the default with the -EB and -EL options.  */
#ifndef TARGET_BYTES_BIG_ENDIAN
#define TARGET_BYTES_BIG_ENDIAN 0
#endif

/* Words are big enough to hold addresses.  */
#define WORKING_DOT_WORD	1

#ifdef OBJ_ELF
extern const char *nios2_target_format (void);
#define TARGET_FORMAT  nios2_target_format ()
#define TARGET_ARCH    bfd_arch_nios2
#endif

/* A NIOS2 instruction consists of tokens and separator characters
   the tokens are things like the instruction name (add, or jmp etc),
   the register indices ($5, $7 etc), and constant expressions. The
   separator characters are commas, brackets and space.
   The instruction name is always separated from other tokens by a space
   The maximum number of tokens in an instruction is 5 (the instruction name,
   3 arguments, and a 4th string representing the expected instruct in opcode
   after assembly. The latter is only used when the assemble is running in
   self test mode, otherwise its presence will generate an error.  */
#define NIOS2_MAX_INSN_TOKENS	6

/* There are no machine-specific operands so we #define this to nothing.  */
#define md_operand(x)

/* Function prototypes exported to rest of GAS.  */
extern void md_assemble (char *op_str);
extern void md_end (void);
extern void md_begin (void);

#define TC_FORCE_RELOCATION(fixp) nios2_force_relocation (fixp)
extern int nios2_force_relocation (struct fix *);

#define tc_fix_adjustable(fixp) nios2_fix_adjustable (fixp)
extern int nios2_fix_adjustable (struct fix *);

#define tc_frob_label(lab)	     nios2_frob_label (lab)
extern void nios2_frob_label (symbolS *);

#define tc_frob_symbol(symp, punt)  punt = nios2_frob_symbol (symp) ? 1 : punt
extern int nios2_frob_symbol (symbolS * symp);

#define md_cons_align(nbytes) nios2_cons_align (nbytes)
extern void nios2_cons_align (int);

extern void md_convert_frag (bfd * headers, segT sec, fragS * fragP);

/* When relaxing, we need to generate relocations for alignment
   directives.  */
#define HANDLE_ALIGN(frag) nios2_handle_align (frag)
extern void nios2_handle_align (fragS *);

#define md_relax_frag nios2_relax_frag
extern long nios2_relax_frag (segT segment, fragS * fragP, long stretch);

#ifdef OBJ_ELF
#define ELF_TC_SPECIAL_SECTIONS \
  { ".sdata",   SHT_PROGBITS,   SHF_ALLOC + SHF_WRITE + SHF_NIOS2_GPREL  }, \
  { ".sbss",    SHT_NOBITS,     SHF_ALLOC + SHF_WRITE + SHF_NIOS2_GPREL  }, \
  { ".lit4",	SHT_PROGBITS,	SHF_ALLOC + SHF_WRITE + SHF_NIOS2_GPREL  }, \
  { ".lit8",	SHT_PROGBITS,	SHF_ALLOC + SHF_WRITE + SHF_NIOS2_GPREL  },

/* Processor-specific section directives.  */
#define md_elf_section_letter		nios2_elf_section_letter
extern int nios2_elf_section_letter (int, const char **);
#define md_elf_section_flags		nios2_elf_section_flags
extern flagword nios2_elf_section_flags (flagword, int, int);
#endif

#define GLOBAL_OFFSET_TABLE_NAME       "_GLOBAL_OFFSET_TABLE_"

#define DIFF_EXPR_OK

/* Don't allow the generic code to convert fixups involving the
   subtraction of a label in the current section to pc-relative if we
   don't have the necessary pc-relative relocation.  */
#define TC_FORCE_RELOCATION_SUB_LOCAL(FIX, SEG)		\
  (!((FIX)->fx_r_type == BFD_RELOC_16			\
     || (FIX)->fx_r_type == BFD_RELOC_NIOS2_LO16	\
     || (FIX)->fx_r_type == BFD_RELOC_NIOS2_HIADJ16))

/* Nios2 ABI doesn't have 32-bit PCREL relocation, and, as relocations for
   CFI information will be in section other than .text, we can't use PC-biased
   relocs.  */
#define CFI_DIFF_EXPR_OK 0

#define TC_PARSE_CONS_EXPRESSION(EXP, NBYTES) nios2_cons (EXP, NBYTES)
extern bfd_reloc_code_real_type nios2_cons (expressionS *exp, int size);

/* We want .cfi_* pseudo-ops for generating unwind info.  */
#define TARGET_USE_CFIPOP 1
#define DWARF2_DEFAULT_RETURN_COLUMN 31
#define DWARF2_CIE_DATA_ALIGNMENT (-4)
#define tc_regname_to_dw2regnum nios2_regname_to_dw2regnum
extern int nios2_regname_to_dw2regnum (char *regname);
#define tc_cfi_frame_initial_instructions  nios2_frame_initial_instructions
extern void nios2_frame_initial_instructions (void);

#define elf_tc_final_processing nios2_elf_final_processing
extern void nios2_elf_final_processing (void);

#endif /* TC_NIOS2 */
