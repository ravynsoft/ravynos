/* tc-mips.h -- header file for tc-mips.c.
   Copyright (C) 1993-2023 Free Software Foundation, Inc.
   Contributed by the OSF and Ralph Campbell.
   Written by Keith Knowles and Ralph Campbell, working independently.
   Modified for ECOFF support by Ian Lance Taylor of Cygnus Support.

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
   along with GAS; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

#ifndef TC_MIPS
#define TC_MIPS

struct frag;
struct expressionS;

/* Default to big endian.  */
#ifndef TARGET_BYTES_BIG_ENDIAN
#define TARGET_BYTES_BIG_ENDIAN		1
#endif

#define TARGET_ARCH bfd_arch_mips

#define WORKING_DOT_WORD	1
#define OLD_FLOAT_READS
#define REPEAT_CONS_EXPRESSIONS
#define RELOC_EXPANSION_POSSIBLE
#define MAX_RELOC_EXPANSION 3
#define LOCAL_LABELS_FB 1

#define TC_ADDRESS_BYTES mips_address_bytes
extern int mips_address_bytes (void);

/* Maximum symbol offset that can be encoded in a BFD_RELOC_GPREL16
   relocation.  */
#define MAX_GPREL_OFFSET (0x7FF0)

#define md_relax_frag(segment, fragp, stretch) \
  mips_relax_frag(segment, fragp, stretch)
extern int mips_relax_frag (asection *, struct frag *, long);

#define md_undefined_symbol(name)	(0)
#define md_operand(x)

extern char mips_nop_opcode (void);
#define NOP_OPCODE (mips_nop_opcode ())

extern void mips_handle_align (struct frag *);
#define HANDLE_ALIGN(fragp)  mips_handle_align (fragp)

#define MAX_MEM_FOR_RS_ALIGN_CODE  (3 + 4)

struct insn_label_list;
struct mips_segment_info {
  struct insn_label_list *labels;
  unsigned int mips16 : 1;
  unsigned int micromips : 1;
};
#define TC_SEGMENT_INFO_TYPE struct mips_segment_info

/* This field is nonzero if the symbol is the target of a MIPS16 jump.  */
#define TC_SYMFIELD_TYPE int

/* Tell assembler that we have an itbl_mips.h header file to include.  */
#define HAVE_ITBL_CPU

/* The endianness of the target format may change based on command
   line arguments.  */
#define TARGET_FORMAT mips_target_format()
extern const char *mips_target_format (void);

/* MIPS PIC level.  */

enum mips_pic_level
{
  /* Do not generate PIC code.  */
  NO_PIC,

  /* Generate PIC code as in the SVR4 MIPS ABI.  */
  SVR4_PIC,

  /* VxWorks's PIC model.  */
  VXWORKS_PIC
};

extern enum mips_pic_level mips_pic;

extern int tc_get_register (int frame);

#define md_after_parse_args() mips_after_parse_args()
extern void mips_after_parse_args (void);

#define tc_init_after_args() mips_init_after_args()
extern void mips_init_after_args (void);

#define md_parse_long_option(arg) mips_parse_long_option (arg)
extern int mips_parse_long_option (const char *);

#define tc_frob_label(sym) mips_define_label (sym)
extern void mips_define_label (symbolS *);

#define tc_new_dot_label(sym) mips_add_dot_label (sym)
extern void mips_add_dot_label (symbolS *);

#define tc_frob_file_before_adjust() mips_frob_file_before_adjust ()
extern void mips_frob_file_before_adjust (void);

#define tc_frob_file_before_fix() mips_frob_file ()
extern void mips_frob_file (void);

#if defined (OBJ_ELF) || defined (OBJ_MAYBE_ELF)
#define tc_frob_file_after_relocs mips_frob_file_after_relocs
extern void mips_frob_file_after_relocs (void);
#endif

#ifdef TE_IRIX
#define tc_frob_symbol(sym, punt) mips_frob_symbol (sym)
extern void mips_frob_symbol (symbolS *);
#endif

#define tc_fix_adjustable(fixp) mips_fix_adjustable (fixp)
extern int mips_fix_adjustable (struct fix *);

/* Values passed to md_apply_fix don't include symbol values.  */
#define MD_APPLY_SYM_VALUE(FIX) 0

/* Global syms must not be resolved, to support ELF shared libraries.  */
#define EXTERN_FORCE_RELOC			\
  (OUTPUT_FLAVOR == bfd_target_elf_flavour)

#define TC_FORCE_RELOCATION(FIX) mips_force_relocation (FIX)
extern int mips_force_relocation (struct fix *);

#define TC_FORCE_RELOCATION_SUB_SAME(FIX, SEG) \
  (GENERIC_FORCE_RELOCATION_SUB_SAME (FIX, SEG) \
   || mips_force_relocation (FIX))

#define TC_FORCE_RELOCATION_ABS(FIX) mips_force_relocation_abs (FIX)
extern bool mips_force_relocation_abs (struct fix *);

/* Register mask variables.  These are set by the MIPS assembly code
   and used by ECOFF and possibly other object file formats.  */
extern unsigned long mips_gprmask;
extern unsigned long mips_cprmask[4];

#if defined (OBJ_ELF) || defined (OBJ_MAYBE_ELF)

#define elf_tc_final_processing mips_elf_final_processing
extern void mips_elf_final_processing (void);

#endif

extern void mips_md_finish (void);
#define md_finish()	mips_md_finish()

extern void mips_pop_insert (void);
#define md_pop_insert()		mips_pop_insert()

extern void mips_emit_delays (void);
#define md_flush_pending_output mips_emit_delays

extern void mips_enable_auto_align (void);
#define md_elf_section_change_hook()	mips_enable_auto_align()

#ifdef TE_IRIX
enum dwarf2_format;
extern enum dwarf2_format mips_dwarf2_format (asection *);
# define DWARF2_FORMAT(SEC) mips_dwarf2_format (SEC)
#else
/* Use GAS' defaults.  */
#endif

extern int mips_dwarf2_addr_size (void);
#define DWARF2_ADDR_SIZE(bfd) mips_dwarf2_addr_size ()
#define DWARF2_FDE_RELOC_SIZE (compact_eh ? 4 : mips_dwarf2_addr_size ())
#define DWARF2_FDE_RELOC_ENCODING(enc) \
  (enc | (compact_eh ? DW_EH_PE_pcrel : 0))

#define TARGET_USE_CFIPOP 1

#define tc_cfi_frame_initial_instructions mips_cfi_frame_initial_instructions
extern void mips_cfi_frame_initial_instructions (void);

#define tc_regname_to_dw2regnum tc_mips_regname_to_dw2regnum
extern int tc_mips_regname_to_dw2regnum (char *regname);

#define DWARF2_DEFAULT_RETURN_COLUMN 31
#define DWARF2_CIE_DATA_ALIGNMENT (-4)

#if defined(OBJ_ELF)

#define tc_cfi_reloc_for_encoding mips_cfi_reloc_for_encoding
extern bfd_reloc_code_real_type mips_cfi_reloc_for_encoding (int encoding);

#define tc_compact_eh_opcode_stop 0x5c
#define tc_compact_eh_opcode_pad 0x5f

#endif
#define DIFF_EXPR_OK
/* We define DIFF_EXPR_OK because of R_MIPS_PC32, but we have no
   64-bit form for n64 CFIs.  */
#define CFI_DIFF_EXPR_OK 0

#define CONVERT_SYMBOLIC_ATTRIBUTE(name) mips_convert_symbolic_attribute (name)
extern int mips_convert_symbolic_attribute (const char *);

#endif /* TC_MIPS */
