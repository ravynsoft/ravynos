/* This file is tc-alpha.h
   Copyright (C) 1994-2023 Free Software Foundation, Inc.
   Written by Ken Raeburn <raeburn@cygnus.com>.

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

#define TC_ALPHA

#define TARGET_BYTES_BIG_ENDIAN 0

#define WORKING_DOT_WORD

#define TARGET_ARCH			bfd_arch_alpha

#ifdef TE_FreeBSD
#define ELF_TARGET_FORMAT	"elf64-alpha-freebsd"
#endif
#ifndef ELF_TARGET_FORMAT
#define ELF_TARGET_FORMAT	"elf64-alpha"
#endif

#define TARGET_FORMAT (OUTPUT_FLAVOR == bfd_target_ecoff_flavour	\
		       ? "ecoff-littlealpha"				\
		       : OUTPUT_FLAVOR == bfd_target_elf_flavour	\
		       ? ELF_TARGET_FORMAT				\
		       : OUTPUT_FLAVOR == bfd_target_evax_flavour	\
		       ? "vms-alpha"					\
		       : "unknown-format")

#define NEED_LITERAL_POOL
#define REPEAT_CONS_EXPRESSIONS

struct fix;
struct alpha_reloc_tag;

extern int alpha_force_relocation (struct fix *);
extern int alpha_fix_adjustable   (struct fix *);

extern unsigned long alpha_gprmask, alpha_fprmask;
extern valueT alpha_gp_value;

#define TC_FORCE_RELOCATION(FIX)	alpha_force_relocation (FIX)
#define tc_fix_adjustable(FIX)		alpha_fix_adjustable (FIX)
#define RELOC_REQUIRES_SYMBOL

/* Values passed to md_apply_fix don't include the symbol value.  */
#define MD_APPLY_SYM_VALUE(FIX) 0

#define md_convert_frag(b,s,f)		as_fatal ("alpha convert_frag\n")
#define md_estimate_size_before_relax(f,s) \
			(as_fatal ("estimate_size_before_relax called"),1)
#define md_operand(x)

#ifdef OBJ_EVAX
#define TC_VALIDATE_FIX_SUB(FIX, SEG) 1

#define tc_canonicalize_symbol_name evax_shorten_name

#define TC_CONS_FIX_NEW(FRAG,OFF,LEN,EXP,RELOC)	\
      (void) RELOC,				\
      fix_new_exp (FRAG, OFF, (int)LEN, EXP, 0, \
	LEN == 2 ? BFD_RELOC_16 \
	: LEN == 4 ? BFD_RELOC_32 \
	: LEN == 8 ? BFD_RELOC_64 \
	: BFD_RELOC_ALPHA_LINKAGE);
#endif

#ifdef OBJ_EVAX
#define TC_IMPLICIT_LCOMM_ALIGNMENT(SIZE, P2VAR) (P2VAR) = 3
#else
#define TC_IMPLICIT_LCOMM_ALIGNMENT(size, align) \
  do							\
    {							\
      align = 0;					\
      if (size > 1)					\
	{						\
	  addressT temp = 1;				\
	  while ((size & temp) == 0)			\
	    ++align, temp <<= 1;			\
	}						\
    }							\
  while (0)
#endif

#define md_number_to_chars		number_to_chars_littleendian

extern int tc_get_register (int);
extern void alpha_frob_ecoff_data (void);

#define tc_frob_label(sym) alpha_define_label (sym)
extern void alpha_define_label (symbolS *);

#define md_cons_align(nbytes) alpha_cons_align (nbytes)
extern void alpha_cons_align (int);

#define HANDLE_ALIGN(fragp) alpha_handle_align (fragp)
extern void alpha_handle_align (struct frag *);

#define MAX_MEM_FOR_RS_ALIGN_CODE  (3 + 4 + 8)

#ifdef OBJ_ECOFF
#define tc_frob_file_before_adjust() alpha_frob_file_before_adjust ()
extern void alpha_frob_file_before_adjust (void);

#define TC_VALIDATE_FIX_SUB(FIX, SEG) \
  ((md_register_arithmetic || (SEG) != reg_section)	\
   && ((FIX)->fx_r_type == BFD_RELOC_GPREL32		\
       || (FIX)->fx_r_type == BFD_RELOC_GPREL16))
#endif

#define DIFF_EXPR_OK   /* foo-. gets turned into PC relative relocs.  */

#ifdef OBJ_ELF
#define md_elf_section_letter		alpha_elf_section_letter
extern bfd_vma alpha_elf_section_letter (int, const char **);
#define md_elf_section_flags		alpha_elf_section_flags
extern flagword alpha_elf_section_flags (flagword, bfd_vma, int);
#endif

/* Whether to add support for explicit !relocation_op!sequence_number.  At the
   moment, only do this for ELF, though ECOFF could use it as well.  */

#ifdef OBJ_ELF
#define RELOC_OP_P
#endif

#ifndef OBJ_EVAX
/* Before the relocations are written, reorder them, so that user
   supplied !lituse relocations follow the appropriate !literal
   relocations.  Also convert the gas-internal relocations to the
   appropriate linker relocations.  */
#define tc_frob_file_before_fix() alpha_before_fix ()
extern void alpha_before_fix (void);
#endif

#ifdef OBJ_ELF
#define md_finish  alpha_elf_md_finish
extern void alpha_elf_md_finish (void);
#endif

/* New fields for supporting explicit relocations (such as !literal to mark
   where a pointer is loaded from the global table, and !lituse_base to track
   all of the normal uses of that pointer).  */

#define TC_FIX_TYPE struct alpha_fix_tag

struct alpha_fix_tag
{
  struct fix *next_reloc;		/* Next !lituse or !gpdisp.  */
  struct alpha_reloc_tag *info;		/* Other members with same sequence.  */
};

/* Initialize the TC_FIX_TYPE field.  */
#define TC_INIT_FIX_DATA(FIX)						\
do {									\
  FIX->tc_fix_data.next_reloc = NULL;					\
  FIX->tc_fix_data.info = NULL;						\
} while (0)

/* Work with DEBUG5 to print fields in tc_fix_type.  */
#define TC_FIX_DATA_PRINT(STREAM, FIX)					\
do {									\
  if (FIX->tc_fix_data.info)						\
    fprintf (STREAM, "\tinfo = 0x%lx, next_reloc = 0x%lx\n", \
	     (long) FIX->tc_fix_data.info,				\
	     (long) FIX->tc_fix_data.next_reloc);			\
} while (0)

#define TARGET_USE_CFIPOP 1

#define tc_cfi_frame_initial_instructions alpha_cfi_frame_initial_instructions
extern void alpha_cfi_frame_initial_instructions (void);

#define DWARF2_LINE_MIN_INSN_LENGTH	4
#define DWARF2_DEFAULT_RETURN_COLUMN	26
#define DWARF2_CIE_DATA_ALIGNMENT	(-8)
