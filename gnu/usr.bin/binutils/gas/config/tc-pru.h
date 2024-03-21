/* Definitions for TI PRU assembler.
   Copyright (C) 2014-2023 Free Software Foundation, Inc.
   Contributed by Dimitar Dimitrov <dimitar@dinux.eu>

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

#ifndef __TC_PRU__
#define __TC_PRU__

#define TARGET_BYTES_BIG_ENDIAN 0

/* Words are big enough to hold addresses.  */
#define WORKING_DOT_WORD	1

extern const char *pru_target_format (void);
#define TARGET_FORMAT  pru_target_format ()
#define TARGET_ARCH    bfd_arch_pru

/* A PRU instruction consists of tokens and separator characters
   the tokens are things like the instruction name (add, or jmp etc),
   the register indices ($5, $7 etc), and constant expressions.  The
   separator characters are commas, brackets and space.
   The instruction name is always separated from other tokens by a space
   The maximum number of tokens in an instruction is 6 (the instruction name,
   4 arguments, and a 4th string representing the expected instruction opcode
   after assembly.  The latter is only used when the assemble is running in
   self test mode, otherwise its presence will generate an error.  */
#define PRU_MAX_INSN_TOKENS	7

/* There are no machine-specific operands so we #define this to nothing.  */
#define md_operand(x)

/* Function prototypes exported to rest of GAS.  */
extern void md_assemble (char *op_str);
extern void pru_md_end (void);
#define md_end pru_md_end
extern void md_begin (void);

#define tc_fix_adjustable(fixp) pru_fix_adjustable (fixp)
extern int pru_fix_adjustable (struct fix *);

#define tc_frob_label(lab) pru_frob_label (lab)
extern void pru_frob_label (symbolS *);

extern void md_convert_frag (bfd * headers, segT sec, fragS * fragP);

#define DIFF_EXPR_OK

/* FIXME This seems appropriate, given that we intentionally prevent
   PRU's .text from being used in a DIFF expression with symbols from
   other sections.  Revisit once GDB is ported.  */
#define CFI_DIFF_EXPR_OK 0

#define TC_PARSE_CONS_RETURN_TYPE int
#define TC_PARSE_CONS_RETURN_NONE 0

#define TC_PARSE_CONS_EXPRESSION(EXP, NBYTES) \
	pru_parse_cons_expression (EXP, NBYTES)
extern int pru_parse_cons_expression (expressionS *exp, int size);

#define TC_CONS_FIX_NEW pru_cons_fix_new
extern void pru_cons_fix_new (struct frag *frag, int where,
				unsigned int nbytes, struct expressionS *exp,
				const int is_pmem);

/* If you define this macro, it means that `tc_gen_reloc' may return
   multiple relocation entries for a single fixup.  In this case, the
   return value of `tc_gen_reloc' is a pointer to a null terminated
   array.  */
#undef RELOC_EXPANSION_POSSIBLE

/* No shared lib support, so we don't need to ensure externally
   visible symbols can be overridden.  */
#define EXTERN_FORCE_RELOC 0

/* If defined, this macro allows control over whether fixups for a
   given section will be processed when the linkrelax variable is
   set.  Define it to zero and handle things in md_apply_fix instead.  */
#define TC_LINKRELAX_FIXUP(SEG) 0

/* If this macro returns non-zero, it guarantees that a relocation will be
   emitted even when the value can be resolved locally.  Do that if
   linkrelax is turned on.  */
#define TC_FORCE_RELOCATION(fix)	pru_force_relocation (fix)
#define TC_FORCE_RELOCATION_SUB_SAME(fix, seg) \
  (GENERIC_FORCE_RELOCATION_SUB_SAME (fix, seg) || pru_force_relocation (fix))
extern int pru_force_relocation (struct fix *);

/* Do not use PC relative fixups and relocations for
   anything but real PCREL relocations.  */
#define TC_FORCE_RELOCATION_SUB_LOCAL(FIX, SEG) \
  (((FIX)->fx_r_type != BFD_RELOC_PRU_S10_PCREL) \
   && ((FIX)->fx_r_type != BFD_RELOC_PRU_U8_PCREL))

/* Values passed to md_apply_fix don't include the symbol value.  */
#define MD_APPLY_SYM_VALUE(FIX) 0

/* We don't want gas to fixup the following memory related relocations.
   We will need them in case that we want to do linker relaxation.
   We could in principle keep these fixups in gas when not relaxing.
   However, there is no serious performance penalty when making the linker
   make the fixup work.  Check also that fx_addsy is not NULL, in order to
   make sure that the fixup refers to some sort of label.  */
#define TC_VALIDATE_FIX(FIXP,SEG,SKIP)			      \
  if ((FIXP->fx_r_type == BFD_RELOC_PRU_LDI32		      \
       || FIXP->fx_r_type == BFD_RELOC_PRU_U16		      \
       || FIXP->fx_r_type == BFD_RELOC_PRU_U16_PMEMIMM	      \
       || FIXP->fx_r_type == BFD_RELOC_PRU_S10_PCREL	      \
       || FIXP->fx_r_type == BFD_RELOC_PRU_U8_PCREL	      \
       || FIXP->fx_r_type == BFD_RELOC_PRU_32_PMEM	      \
       || FIXP->fx_r_type == BFD_RELOC_PRU_16_PMEM)	      \
      && FIXP->fx_addsy != NULL				      \
      && FIXP->fx_subsy == NULL)			      \
    {							      \
      symbol_mark_used_in_reloc (FIXP->fx_addsy);	      \
      goto SKIP;					      \
    }

/* This macro is evaluated for any fixup with a fx_subsy that
   fixup_segment cannot reduce to a number.  If the macro returns
   false an error will be reported.  */
#define TC_VALIDATE_FIX_SUB(fix, seg)   pru_validate_fix_sub (fix)
extern int pru_validate_fix_sub (struct fix *);

/* We want .cfi_* pseudo-ops for generating unwind info.  */
#define TARGET_USE_CFIPOP 1

/* Program Counter register number is not defined by TI documents.
   Pick the virtual number used by GCC.  */
#define DWARF2_DEFAULT_RETURN_COLUMN 132

/* The stack grows down, and is only byte aligned.  */
#define DWARF2_CIE_DATA_ALIGNMENT -1

#define tc_regname_to_dw2regnum pru_regname_to_dw2regnum
extern int pru_regname_to_dw2regnum (char *regname);
#define tc_cfi_frame_initial_instructions  pru_frame_initial_instructions
extern void pru_frame_initial_instructions (void);

/* The difference between same-section symbols may be affected by linker
   relaxation, so do not resolve such expressions in the assembler.  */
#define md_allow_local_subtract(l,r,s) pru_allow_local_subtract (l, r, s)
extern bool pru_allow_local_subtract (expressionS *, expressionS *, segT);

#endif /* __TC_PRU__ */
