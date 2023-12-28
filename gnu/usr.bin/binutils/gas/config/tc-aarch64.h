/* tc-aarch64.h -- Header file for tc-aarch64.c.
   Copyright (C) 2009-2023 Free Software Foundation, Inc.
   Contributed by ARM Ltd.

   This file is part of GAS.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the license, or
   (at your option) any later version.

   GAS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING3. If not,
   see <http://www.gnu.org/licenses/>.  */

#ifndef TC_AARCH64
#define TC_AARCH64 1

#include "opcode/aarch64.h"

#ifndef TARGET_BYTES_BIG_ENDIAN
#define TARGET_BYTES_BIG_ENDIAN 0
#endif

#define WORKING_DOT_WORD

#define TARGET_ARCH 	bfd_arch_aarch64

#define DIFF_EXPR_OK

/* Permit // comments.  */
#define DOUBLESLASH_LINE_COMMENTS

#ifdef  LITTLE_ENDIAN
#undef  LITTLE_ENDIAN
#endif

#ifdef  BIG_ENDIAN
#undef  BIG_ENDIAN
#endif

#define LITTLE_ENDIAN 	1234
#define BIG_ENDIAN 	4321

#define SWAP_32(n) \
  ((((n) & 0xff) << 24) | (((n) & 0xff00) << 8) | (((n) >> 8) & 0xff00) \
   | (((n) >> 24) & 0xff))

struct fix;

struct aarch64_fix
{
  struct aarch64_inst *inst;
  enum aarch64_opnd opnd;
};

#ifdef OBJ_ELF
# define AARCH64_BI_ENDIAN
# define TARGET_FORMAT	elf64_aarch64_target_format ()
#elif defined (OBJ_COFF)
# define TARGET_FORMAT coff_aarch64_target_format ()
#endif

#define TC_FORCE_RELOCATION(FIX) aarch64_force_relocation (FIX)

/* Currently there is no machine specific frags generated.  */
#define md_convert_frag(b,s,f) as_fatal ("aarch64 convert_frag called\n")

#define md_cleanup() aarch64_cleanup ()

#define md_start_line_hook() aarch64_start_line_hook ()

#define tc_frob_label(S) aarch64_frob_label (S)

/* We also need to mark assembler created symbols:  */
#define tc_frob_fake_label(S) aarch64_frob_label (S)

#define tc_frob_section(S) aarch64_frob_section (S)

/* The key used to sign a function's return address.  */
enum pointer_auth_key {
  AARCH64_PAUTH_KEY_A,
  AARCH64_PAUTH_KEY_B
};

/* The extra fields required by AArch64 in fde_entry and cie_entry.  Currently
   only used to store the key used to sign the frame's return address.  */
#define tc_fde_entry_extras enum pointer_auth_key pauth_key;
#define tc_cie_entry_extras enum pointer_auth_key pauth_key;

/* The extra initialisation steps needed by AArch64 in alloc_fde_entry.
   Currently only used to initialise the key used to sign the return
   address.  */
#define tc_fde_entry_init_extra(fde) fde->pauth_key = AARCH64_PAUTH_KEY_A;

/* Extra checks required by AArch64 when outputting the current cie_entry.
   Currently only used to output a 'B' if the return address is signed with the
   B key.  */
#define tc_output_cie_extra(cie) \
    do \
      { \
	if (cie->pauth_key == AARCH64_PAUTH_KEY_B) \
	  out_one ('B'); \
      } \
    while (0)

/* Extra equivalence checks required by AArch64 when selecting the correct cie
   for some fde.  Currently only used to check for quivalence between keys used
   to sign ther return address.  */
#define tc_cie_fde_equivalent_extra(cie, fde) (cie->pauth_key == fde->pauth_key)

/* The extra initialisation steps needed by AArch64 in select_cie_for_fde.
   Currently only used to initialise the key used to sign the return
   address.  */
#define tc_cie_entry_init_extra(cie, fde) cie->pauth_key = fde->pauth_key;

#define TC_FIX_TYPE struct aarch64_fix
#define TC_INIT_FIX_DATA(FIX) { (FIX)->tc_fix_data.inst = NULL;	\
    (FIX)->tc_fix_data.opnd = AARCH64_OPND_NIL; }

#define TC_SYMFIELD_TYPE 	unsigned int
#define AARCH64_GET_FLAG(s)   	(*symbol_get_tc (s))

void aarch64_copy_symbol_attributes (symbolS *, symbolS *);
#ifndef TC_COPY_SYMBOL_ATTRIBUTES
#define TC_COPY_SYMBOL_ATTRIBUTES(DEST, SRC) \
  (aarch64_copy_symbol_attributes (DEST, SRC))
#endif

#ifdef OBJ_ELF
void aarch64_elf_copy_symbol_attributes (symbolS *, symbolS *);
#define OBJ_COPY_SYMBOL_ATTRIBUTES(DEST, SRC) \
  aarch64_elf_copy_symbol_attributes (DEST, SRC)
#endif

#define TC_START_LABEL(STR, NUL_CHAR, NEXT_CHAR)			\
  (NEXT_CHAR == ':' || (NEXT_CHAR == '/' && aarch64_data_in_code ()))
#define tc_canonicalize_symbol_name(str) aarch64_canonicalize_symbol_name (str);
#define obj_adjust_symtab() 		 aarch64_adjust_symtab ()

#define LISTING_HEADER "AARCH64 GAS "

#define LOCAL_LABEL(name)  (name[0] == '.' && name[1] == 'L')
#define LOCAL_LABELS_FB    1

/* This expression evaluates to true if the relocation is for a local
   object for which we still want to do the relocation at runtime.
   False if we are willing to perform this relocation while building
   the .o file.  GOTOFF does not need to be checked here because it is
   not pcrel.  I am not sure if some of the others are ever used with
   pcrel, but it is easier to be safe than sorry.  */

#define TC_FORCE_RELOCATION_LOCAL(FIX)			\
  (GENERIC_FORCE_RELOCATION_LOCAL (FIX)			\
   || (FIX)->fx_r_type == BFD_RELOC_64			\
   || (FIX)->fx_r_type == BFD_RELOC_32)

#define TC_CONS_FIX_NEW(f,w,s,e,r) cons_fix_new_aarch64 ((f), (w), (s), (e))

/* Max space for a rs_align_code fragment is 3 unaligned bytes
   (fr_fix) plus 4 bytes to contain the repeating NOP (fr_var).  */
#define MAX_MEM_FOR_RS_ALIGN_CODE 7

/* For frags in code sections we need to record whether they contain
   code or data.  */
struct aarch64_frag_type
{
  int recorded;
#if defined OBJ_ELF || defined OBJ_COFF
  /* If there is a mapping symbol at offset 0 in this frag,
     it will be saved in FIRST_MAP.  If there are any mapping
     symbols in this frag, the last one will be saved in
     LAST_MAP.  */
  symbolS *first_map, *last_map;
#endif
};

#define TC_FRAG_TYPE		struct aarch64_frag_type
#define TC_FRAG_INIT(fragp, max_bytes) aarch64_init_frag (fragp, max_bytes)
#define HANDLE_ALIGN(fragp)	aarch64_handle_align (fragp)

#define md_do_align(N, FILL, LEN, MAX, LABEL)					\
  if (FILL == NULL && (N) != 0 && ! need_pass_2 && subseg_text_p (now_seg))	\
    {										\
      frag_align_code (N, MAX);							\
      goto LABEL;								\
    }

/* COFF sub section alignment calculated using the write.c implementation.  */
#ifndef OBJ_COFF
#define SUB_SEGMENT_ALIGN(SEG, FRCHAIN) 0
#endif

#define DWARF2_LINE_MIN_INSN_LENGTH 	4

/* The lr register is r30.  */
#define DWARF2_DEFAULT_RETURN_COLUMN  30

/* Registers are generally saved at negative offsets to the CFA.  */
#define DWARF2_CIE_DATA_ALIGNMENT     (-8)

extern int aarch64_dwarf2_addr_size (void);
#define DWARF2_ADDR_SIZE(bfd) aarch64_dwarf2_addr_size ()

#if defined OBJ_ELF || defined OBJ_COFF
#ifdef OBJ_ELF
# define obj_frob_symbol(sym, punt)	aarch64elf_frob_symbol ((sym), & (punt))
#endif

# define GLOBAL_OFFSET_TABLE_NAME	"_GLOBAL_OFFSET_TABLE_"
# define TC_SEGMENT_INFO_TYPE 		struct aarch64_segment_info_type

/* This is not really an alignment operation, but it's something we
   need to do at the same time: whenever we are figuring out the
   alignment for data, we should check whether a $d symbol is
   necessary.  */
# define md_cons_align(nbytes)		mapping_state (MAP_DATA)

enum mstate
{
  MAP_UNDEFINED = 0, /* Must be zero, for seginfo in new sections.  */
  MAP_DATA,
  MAP_INSN,
};

void mapping_state (enum mstate);

struct aarch64_segment_info_type
{
  const char *last_file;
  unsigned last_line;
  enum mstate mapstate;
  unsigned int marked_pr_dependency;
  aarch64_instr_sequence insn_sequence;
};

/* We want .cfi_* pseudo-ops for generating unwind info.  */
#define TARGET_USE_CFIPOP              1

/* CFI hooks.  */
#define tc_regname_to_dw2regnum            tc_aarch64_regname_to_dw2regnum
#define tc_cfi_frame_initial_instructions  tc_aarch64_frame_initial_instructions

extern void aarch64_after_parse_args (void);
#define md_after_parse_args() aarch64_after_parse_args ()

# define EXTERN_FORCE_RELOC 			1
# define tc_fix_adjustable(FIX) 		1

/* Values passed to md_apply_fix don't include the symbol value.  */
# define MD_APPLY_SYM_VALUE(FIX) 		0

#else /* Neither OBJ_ELF nor OBJ_COFF.  */

#define GLOBAL_OFFSET_TABLE_NAME "__GLOBAL_OFFSET_TABLE_"

#endif /*  OBJ_ELF || OBJ_COFF.  */

#ifdef OBJ_ELF

/* Whether SFrame stack trace info is supported.  */
extern bool aarch64_support_sframe_p (void);
#define support_sframe_p aarch64_support_sframe_p

/* The stack-pointer register number for SFrame stack trace info.  */
extern unsigned int aarch64_sframe_cfa_sp_reg;
#define SFRAME_CFA_SP_REG aarch64_sframe_cfa_sp_reg

/* The base-pointer register number for CFA stack trace info.  */
extern unsigned int aarch64_sframe_cfa_fp_reg;
#define SFRAME_CFA_FP_REG aarch64_sframe_cfa_fp_reg

/* The return address register number for CFA stack trace info.  */
extern unsigned int aarch64_sframe_cfa_ra_reg;
#define SFRAME_CFA_RA_REG aarch64_sframe_cfa_ra_reg

/* Specify if RA tracking is needed.  */
extern bool aarch64_sframe_ra_tracking_p (void);
#define sframe_ra_tracking_p aarch64_sframe_ra_tracking_p

/* Specify the fixed offset to recover RA from CFA.
   (useful only when RA tracking is not needed).  */
extern offsetT aarch64_sframe_cfa_ra_offset (void);
#define sframe_cfa_ra_offset aarch64_sframe_cfa_ra_offset

/* The abi/arch indentifier for SFrame.  */
unsigned char aarch64_sframe_get_abi_arch (void);
#define sframe_get_abi_arch aarch64_sframe_get_abi_arch

#endif /* OBJ_ELF  */

#define MD_PCREL_FROM_SECTION(F,S) md_pcrel_from_section(F,S)

extern void aarch64_frag_align_code (int, int);
extern const char * elf64_aarch64_target_format (void);
extern const char * coff_aarch64_target_format (void);
extern int aarch64_force_relocation (struct fix *);
extern void aarch64_cleanup (void);
extern void aarch64_start_line_hook (void);
extern void aarch64_frob_label (symbolS *);
extern void aarch64_frob_section (asection *sec);
extern int aarch64_data_in_code (void);
extern char * aarch64_canonicalize_symbol_name (char *);
extern void aarch64_adjust_symtab (void);
extern void aarch64elf_frob_symbol (symbolS *, int *);
extern void cons_fix_new_aarch64 (fragS *, int, int, expressionS *);
extern void aarch64_init_frag (struct frag *, int);
extern void aarch64_handle_align (struct frag *);
extern int tc_aarch64_regname_to_dw2regnum (char *regname);
extern void tc_aarch64_frame_initial_instructions (void);

#ifdef TE_PE

#define O_secrel O_md1

#define TC_DWARF2_EMIT_OFFSET  tc_pe_dwarf2_emit_offset
void tc_pe_dwarf2_emit_offset (symbolS *, unsigned int);

#endif /* TE_PE */

#endif /* TC_AARCH64 */
