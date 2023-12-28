/* PowerPC64-specific support for 64-bit ELF.
   Copyright (C) 1999-2023 Free Software Foundation, Inc.
   Written by Linus Nordberg, Swox AB <info@swox.com>,
   based on elf32-ppc.c by Ian Lance Taylor.
   Largely rewritten by Alan Modra.

   This file is part of BFD, the Binary File Descriptor library.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.  */


/* The 64-bit PowerPC ELF ABI may be found at
   http://www.linuxbase.org/spec/ELF/ppc64/PPC-elf64abi.txt, and
   http://www.linuxbase.org/spec/ELF/ppc64/spec/book1.html  */

/* The assembler should generate a full set of section symbols even
   when they appear unused.  The linux kernel build tool recordmcount
   needs them.  */
#define TARGET_KEEP_UNUSED_SECTION_SYMBOLS true

#include "sysdep.h"
#include <stdarg.h>
#include "bfd.h"
#include "bfdlink.h"
#include "libbfd.h"
#include "elf-bfd.h"
#include "elf/ppc64.h"
#include "elf64-ppc.h"
#include "dwarf2.h"

/* All users of this file have bfd_octets_per_byte (abfd, sec) == 1.  */
#define OCTETS_PER_BYTE(ABFD, SEC) 1

static bfd_reloc_status_type ppc64_elf_ha_reloc
  (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);
static bfd_reloc_status_type ppc64_elf_branch_reloc
  (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);
static bfd_reloc_status_type ppc64_elf_brtaken_reloc
  (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);
static bfd_reloc_status_type ppc64_elf_sectoff_reloc
  (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);
static bfd_reloc_status_type ppc64_elf_sectoff_ha_reloc
  (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);
static bfd_reloc_status_type ppc64_elf_toc_reloc
  (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);
static bfd_reloc_status_type ppc64_elf_toc_ha_reloc
  (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);
static bfd_reloc_status_type ppc64_elf_toc64_reloc
  (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);
static bfd_reloc_status_type ppc64_elf_prefix_reloc
  (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);
static bfd_reloc_status_type ppc64_elf_unhandled_reloc
  (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);
static bfd_vma opd_entry_value
  (asection *, bfd_vma, asection **, bfd_vma *, bool);

#define TARGET_LITTLE_SYM	powerpc_elf64_le_vec
#define TARGET_LITTLE_NAME	"elf64-powerpcle"
#define TARGET_BIG_SYM		powerpc_elf64_vec
#define TARGET_BIG_NAME		"elf64-powerpc"
#define ELF_ARCH		bfd_arch_powerpc
#define ELF_TARGET_ID		PPC64_ELF_DATA
#define ELF_MACHINE_CODE	EM_PPC64
#define ELF_MAXPAGESIZE		0x10000
#define ELF_COMMONPAGESIZE	0x1000
#define elf_info_to_howto	ppc64_elf_info_to_howto

#define elf_backend_want_got_sym 0
#define elf_backend_want_plt_sym 0
#define elf_backend_plt_alignment 3
#define elf_backend_plt_not_loaded 1
#define elf_backend_got_header_size 8
#define elf_backend_want_dynrelro 1
#define elf_backend_can_gc_sections 1
#define elf_backend_can_refcount 1
#define elf_backend_rela_normal 1
#define elf_backend_dtrel_excludes_plt 1
#define elf_backend_default_execstack 0

#define bfd_elf64_mkobject		      ppc64_elf_mkobject
#define bfd_elf64_bfd_free_cached_info	      ppc64_elf_free_cached_info
#define bfd_elf64_bfd_reloc_type_lookup	      ppc64_elf_reloc_type_lookup
#define bfd_elf64_bfd_reloc_name_lookup	      ppc64_elf_reloc_name_lookup
#define bfd_elf64_bfd_merge_private_bfd_data  ppc64_elf_merge_private_bfd_data
#define bfd_elf64_bfd_print_private_bfd_data  ppc64_elf_print_private_bfd_data
#define bfd_elf64_new_section_hook	      ppc64_elf_new_section_hook
#define bfd_elf64_bfd_link_hash_table_create  ppc64_elf_link_hash_table_create
#define bfd_elf64_get_synthetic_symtab	      ppc64_elf_get_synthetic_symtab
#define bfd_elf64_bfd_link_just_syms	      ppc64_elf_link_just_syms
#define bfd_elf64_bfd_gc_sections	      ppc64_elf_gc_sections

#define elf_backend_object_p		      ppc64_elf_object_p
#define elf_backend_grok_prstatus	      ppc64_elf_grok_prstatus
#define elf_backend_grok_psinfo		      ppc64_elf_grok_psinfo
#define elf_backend_write_core_note	      ppc64_elf_write_core_note
#define elf_backend_create_dynamic_sections   _bfd_elf_create_dynamic_sections
#define elf_backend_copy_indirect_symbol      ppc64_elf_copy_indirect_symbol
#define elf_backend_add_symbol_hook	      ppc64_elf_add_symbol_hook
#define elf_backend_check_directives	      ppc64_elf_before_check_relocs
#define elf_backend_notice_as_needed	      ppc64_elf_notice_as_needed
#define elf_backend_archive_symbol_lookup     ppc64_elf_archive_symbol_lookup
#define elf_backend_check_relocs	      ppc64_elf_check_relocs
#define elf_backend_relocs_compatible	      _bfd_elf_relocs_compatible
#define elf_backend_gc_keep		      ppc64_elf_gc_keep
#define elf_backend_gc_mark_dynamic_ref       ppc64_elf_gc_mark_dynamic_ref
#define elf_backend_gc_mark_hook	      ppc64_elf_gc_mark_hook
#define elf_backend_adjust_dynamic_symbol     ppc64_elf_adjust_dynamic_symbol
#define elf_backend_hide_symbol		      ppc64_elf_hide_symbol
#define elf_backend_maybe_function_sym	      ppc64_elf_maybe_function_sym
#define elf_backend_always_size_sections      ppc64_elf_edit
#define elf_backend_size_dynamic_sections     ppc64_elf_size_dynamic_sections
#define elf_backend_hash_symbol		      ppc64_elf_hash_symbol
#define elf_backend_init_index_section	      _bfd_elf_init_2_index_sections
#define elf_backend_action_discarded	      ppc64_elf_action_discarded
#define elf_backend_relocate_section	      ppc64_elf_relocate_section
#define elf_backend_finish_dynamic_symbol     ppc64_elf_finish_dynamic_symbol
#define elf_backend_reloc_type_class	      ppc64_elf_reloc_type_class
#define elf_backend_finish_dynamic_sections   ppc64_elf_finish_dynamic_sections
#define elf_backend_link_output_symbol_hook   ppc64_elf_output_symbol_hook
#define elf_backend_special_sections	      ppc64_elf_special_sections
#define elf_backend_section_flags	      ppc64_elf_section_flags
#define elf_backend_merge_symbol_attribute    ppc64_elf_merge_symbol_attribute
#define elf_backend_merge_symbol	      ppc64_elf_merge_symbol
#define elf_backend_get_reloc_section	      bfd_get_section_by_name

/* The name of the dynamic interpreter.  This is put in the .interp
   section.  */
#define ELF_DYNAMIC_INTERPRETER "/usr/lib/ld.so.1"

/* The size in bytes of an entry in the procedure linkage table.  */
#define PLT_ENTRY_SIZE(htab) (htab->opd_abi ? 24 : 8)
#define LOCAL_PLT_ENTRY_SIZE(htab) (htab->opd_abi ? 16 : 8)

/* The initial size of the plt reserved for the dynamic linker.  */
#define PLT_INITIAL_ENTRY_SIZE(htab) (htab->opd_abi ? 24 : 16)

/* Offsets to some stack save slots.  */
#define STK_LR 16
#define STK_TOC(htab) (htab->opd_abi ? 40 : 24)
/* This one is dodgy.  ELFv2 does not have a linker word, so use the
   CR save slot.  Used only by optimised __tls_get_addr call stub,
   relying on __tls_get_addr_opt not saving CR..  */
#define STK_LINKER(htab) (htab->opd_abi ? 32 : 8)

/* TOC base pointers offset from start of TOC.  */
#define TOC_BASE_OFF	0x8000
/* TOC base alignment.  */
#define TOC_BASE_ALIGN	256

/* Offset of tp and dtp pointers from start of TLS block.  */
#define TP_OFFSET	0x7000
#define DTP_OFFSET	0x8000

/* .plt call stub instructions.  The normal stub is like this, but
   sometimes the .plt entry crosses a 64k boundary and we need to
   insert an addi to adjust r11.  */
#define STD_R2_0R1	0xf8410000	/* std	 %r2,0+40(%r1)	     */
#define ADDIS_R11_R2	0x3d620000	/* addis %r11,%r2,xxx@ha     */
#define LD_R12_0R11	0xe98b0000	/* ld	 %r12,xxx+0@l(%r11)  */
#define MTCTR_R12	0x7d8903a6	/* mtctr %r12		     */
#define LD_R2_0R11	0xe84b0000	/* ld	 %r2,xxx+8@l(%r11)   */
#define LD_R11_0R11	0xe96b0000	/* ld	 %r11,xxx+16@l(%r11) */
#define BCTR		0x4e800420	/* bctr			     */

#define ADDI_R11_R11	0x396b0000	/* addi %r11,%r11,off@l	 */
#define ADDI_R12_R11	0x398b0000	/* addi %r12,%r11,off@l	 */
#define ADDI_R12_R12	0x398c0000	/* addi %r12,%r12,off@l	 */
#define ADDIS_R2_R2	0x3c420000	/* addis %r2,%r2,off@ha	 */
#define ADDI_R2_R2	0x38420000	/* addi	 %r2,%r2,off@l	 */

#define XOR_R2_R12_R12	0x7d826278	/* xor	 %r2,%r12,%r12	 */
#define ADD_R11_R11_R2	0x7d6b1214	/* add	 %r11,%r11,%r2	 */
#define XOR_R11_R12_R12	0x7d8b6278	/* xor	 %r11,%r12,%r12	 */
#define ADD_R2_R2_R11	0x7c425a14	/* add	 %r2,%r2,%r11	 */
#define CMPLDI_R2_0	0x28220000	/* cmpldi %r2,0		 */
#define BNECTR		0x4ca20420	/* bnectr+		 */
#define BNECTR_P4	0x4ce20420	/* bnectr+		 */

#define LD_R12_0R2	0xe9820000	/* ld	 %r12,xxx+0(%r2) */
#define LD_R11_0R2	0xe9620000	/* ld	 %r11,xxx+0(%r2) */
#define LD_R2_0R2	0xe8420000	/* ld	 %r2,xxx+0(%r2)	 */

#define LD_R2_0R1	0xe8410000	/* ld	 %r2,0(%r1)	 */
#define LD_R2_0R12	0xe84c0000	/* ld	 %r2,0(%r12)	 */
#define ADD_R2_R2_R12	0x7c426214	/* add	 %r2,%r2,%r12	 */

#define LI_R11_0	0x39600000	/* li    %r11,0		*/
#define LIS_R2		0x3c400000	/* lis %r2,xxx@ha	  */
#define LIS_R11		0x3d600000	/* lis %r11,xxx@ha	  */
#define LIS_R12		0x3d800000	/* lis %r12,xxx@ha	  */
#define ADDIS_R2_R12	0x3c4c0000	/* addis %r2,%r12,xxx@ha  */
#define ADDIS_R12_R2	0x3d820000	/* addis %r12,%r2,xxx@ha  */
#define ADDIS_R12_R11	0x3d8b0000	/* addis %r12,%r11,xxx@ha */
#define ADDIS_R12_R12	0x3d8c0000	/* addis %r12,%r12,xxx@ha */
#define ORIS_R12_R12_0	0x658c0000	/* oris  %r12,%r12,xxx@hi */
#define ORI_R11_R11_0	0x616b0000	/* ori   %r11,%r11,xxx@l  */
#define ORI_R12_R12_0	0x618c0000	/* ori   %r12,%r12,xxx@l  */
#define LD_R12_0R12	0xe98c0000	/* ld	 %r12,xxx@l(%r12) */
#define SLDI_R11_R11_34	0x796b1746	/* sldi  %r11,%r11,34     */
#define SLDI_R12_R12_32	0x799c07c6	/* sldi  %r12,%r12,32     */
#define LDX_R12_R11_R12 0x7d8b602a	/* ldx   %r12,%r11,%r12   */
#define ADD_R12_R11_R12 0x7d8b6214	/* add   %r12,%r11,%r12   */
#define PADDI_R12_PC	0x0610000039800000ULL
#define PLD_R12_PC	0x04100000e5800000ULL
#define PNOP		0x0700000000000000ULL

/* __glink_PLTresolve stub instructions.  We enter with the index in
   R0 for ELFv1, and the address of a glink branch in R12 for ELFv2.  */
#define GLINK_PLTRESOLVE_SIZE(htab)			\
  (8u + (htab->opd_abi ? 11 * 4 : htab->has_plt_localentry0 ? 14 * 4 : 13 * 4))
					/* 0:				*/
					/*  .quad plt0-1f		*/
					/* __glink:			*/
#define MFLR_R12	0x7d8802a6	/*  mflr %12			*/
#define BCL_20_31	0x429f0005	/*  bcl 20,31,1f		*/
					/* 1:				*/
#define MFLR_R11	0x7d6802a6	/*  mflr %11			*/
					/*  ld %2,(0b-1b)(%11)		*/
#define MTLR_R12	0x7d8803a6	/*  mtlr %12			*/
#define ADD_R11_R2_R11	0x7d625a14	/*  add %11,%2,%11		*/
					/*  ld %12,0(%11)		*/
					/*  ld %2,8(%11)		*/
					/*  mtctr %12			*/
					/*  ld %11,16(%11)		*/
					/*  bctr			*/

#define MFLR_R0		0x7c0802a6	/* mflr %r0			*/
#define MTLR_R0		0x7c0803a6	/* mtlr %r0			*/
#define SUB_R12_R12_R11	0x7d8b6050	/* subf %r12,%r11,%r12		*/
#define ADDI_R0_R12	0x380c0000	/* addi %r0,%r12,0		*/
#define SRDI_R0_R0_2	0x7800f082	/* rldicl %r0,%r0,62,2		*/
#define LD_R0_0R11	0xe80b0000	/* ld %r0,0(%r11)		*/
#define ADD_R11_R0_R11	0x7d605a14	/* add %r11,%r0,%r11		*/

/* Pad with this.  */
#define NOP		0x60000000

/* Some other nops.  */
#define CROR_151515	0x4def7b82
#define CROR_313131	0x4ffffb82

/* .glink entries for the first 32k functions are two instructions.  */
#define LI_R0_0		0x38000000	/* li    %r0,0		*/
#define B_DOT		0x48000000	/* b     .		*/

/* After that, we need two instructions to load the index, followed by
   a branch.  */
#define LIS_R0_0	0x3c000000	/* lis   %r0,0		*/
#define ORI_R0_R0_0	0x60000000	/* ori	 %r0,%r0,0	*/

/* Instructions used by the save and restore reg functions.  */
#define STD_R0_0R1	0xf8010000	/* std   %r0,0(%r1)	*/
#define STD_R0_0R12	0xf80c0000	/* std   %r0,0(%r12)	*/
#define LD_R0_0R1	0xe8010000	/* ld    %r0,0(%r1)	*/
#define LD_R0_0R12	0xe80c0000	/* ld    %r0,0(%r12)	*/
#define STFD_FR0_0R1	0xd8010000	/* stfd  %fr0,0(%r1)	*/
#define LFD_FR0_0R1	0xc8010000	/* lfd   %fr0,0(%r1)	*/
#define LI_R12_0	0x39800000	/* li    %r12,0		*/
#define STVX_VR0_R12_R0	0x7c0c01ce	/* stvx  %v0,%r12,%r0	*/
#define LVX_VR0_R12_R0	0x7c0c00ce	/* lvx   %v0,%r12,%r0	*/
#define MTLR_R0		0x7c0803a6	/* mtlr  %r0		*/
#define BLR		0x4e800020	/* blr			*/

/* Since .opd is an array of descriptors and each entry will end up
   with identical R_PPC64_RELATIVE relocs, there is really no need to
   propagate .opd relocs;  The dynamic linker should be taught to
   relocate .opd without reloc entries.  */
#ifndef NO_OPD_RELOCS
#define NO_OPD_RELOCS 0
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof (a) / sizeof ((a)[0]))
#endif

static inline int
abiversion (bfd *abfd)
{
  return elf_elfheader (abfd)->e_flags & EF_PPC64_ABI;
}

static inline void
set_abiversion (bfd *abfd, int ver)
{
  elf_elfheader (abfd)->e_flags &= ~EF_PPC64_ABI;
  elf_elfheader (abfd)->e_flags |= ver & EF_PPC64_ABI;
}

#define is_ppc64_elf(bfd) \
  (bfd_get_flavour (bfd) == bfd_target_elf_flavour \
   && elf_object_id (bfd) == PPC64_ELF_DATA)

/* Relocation HOWTO's.  */
/* Like other ELF RELA targets that don't apply multiple
   field-altering relocations to the same localation, src_mask is
   always zero and pcrel_offset is the same as pc_relative.
   PowerPC can always use a zero bitpos, even when the field is not at
   the LSB.  For example, a REL24 could use rightshift=2, bisize=24
   and bitpos=2 which matches the ABI description, or as we do here,
   rightshift=0, bitsize=26 and bitpos=0.  */
#define HOW(type, size, bitsize, mask, rightshift, pc_relative, \
	    complain, special_func)				\
  HOWTO (type, rightshift, size, bitsize, pc_relative, 0,	\
	 complain_overflow_ ## complain, special_func,		\
	 #type, false, 0, mask, pc_relative)

static reloc_howto_type *ppc64_elf_howto_table[(int) R_PPC64_max];

static reloc_howto_type ppc64_elf_howto_raw[] =
{
  /* This reloc does nothing.  */
  HOW (R_PPC64_NONE, 0, 0, 0, 0, false, dont,
       bfd_elf_generic_reloc),

  /* A standard 32 bit relocation.  */
  HOW (R_PPC64_ADDR32, 4, 32, 0xffffffff, 0, false, bitfield,
       bfd_elf_generic_reloc),

  /* An absolute 26 bit branch; the lower two bits must be zero.
     FIXME: we don't check that, we just clear them.  */
  HOW (R_PPC64_ADDR24, 4, 26, 0x03fffffc, 0, false, bitfield,
       bfd_elf_generic_reloc),

  /* A standard 16 bit relocation.  */
  HOW (R_PPC64_ADDR16, 2, 16, 0xffff, 0, false, bitfield,
       bfd_elf_generic_reloc),

  /* A 16 bit relocation without overflow.  */
  HOW (R_PPC64_ADDR16_LO, 2, 16, 0xffff, 0, false, dont,
       bfd_elf_generic_reloc),

  /* Bits 16-31 of an address.  */
  HOW (R_PPC64_ADDR16_HI, 2, 16, 0xffff, 16, false, signed,
       bfd_elf_generic_reloc),

  /* Bits 16-31 of an address, plus 1 if the contents of the low 16
     bits, treated as a signed number, is negative.  */
  HOW (R_PPC64_ADDR16_HA, 2, 16, 0xffff, 16, false, signed,
       ppc64_elf_ha_reloc),

  /* An absolute 16 bit branch; the lower two bits must be zero.
     FIXME: we don't check that, we just clear them.  */
  HOW (R_PPC64_ADDR14, 4, 16, 0x0000fffc, 0, false, signed,
       ppc64_elf_branch_reloc),

  /* An absolute 16 bit branch, for which bit 10 should be set to
     indicate that the branch is expected to be taken.  The lower two
     bits must be zero.  */
  HOW (R_PPC64_ADDR14_BRTAKEN, 4, 16, 0x0000fffc, 0, false, signed,
       ppc64_elf_brtaken_reloc),

  /* An absolute 16 bit branch, for which bit 10 should be set to
     indicate that the branch is not expected to be taken.  The lower
     two bits must be zero.  */
  HOW (R_PPC64_ADDR14_BRNTAKEN, 4, 16, 0x0000fffc, 0, false, signed,
       ppc64_elf_brtaken_reloc),

  /* A relative 26 bit branch; the lower two bits must be zero.  */
  HOW (R_PPC64_REL24, 4, 26, 0x03fffffc, 0, true, signed,
       ppc64_elf_branch_reloc),

  /* A variant of R_PPC64_REL24, used when r2 is not the toc pointer.  */
  HOW (R_PPC64_REL24_NOTOC, 4, 26, 0x03fffffc, 0, true, signed,
       ppc64_elf_branch_reloc),

  /* Another variant, when p10 insns can't be used on stubs.  */
  HOW (R_PPC64_REL24_P9NOTOC, 4, 26, 0x03fffffc, 0, true, signed,
       ppc64_elf_branch_reloc),

  /* A relative 16 bit branch; the lower two bits must be zero.  */
  HOW (R_PPC64_REL14, 4, 16, 0x0000fffc, 0, true, signed,
       ppc64_elf_branch_reloc),

  /* A relative 16 bit branch.  Bit 10 should be set to indicate that
     the branch is expected to be taken.  The lower two bits must be
     zero.  */
  HOW (R_PPC64_REL14_BRTAKEN, 4, 16, 0x0000fffc, 0, true, signed,
       ppc64_elf_brtaken_reloc),

  /* A relative 16 bit branch.  Bit 10 should be set to indicate that
     the branch is not expected to be taken.  The lower two bits must
     be zero.  */
  HOW (R_PPC64_REL14_BRNTAKEN, 4, 16, 0x0000fffc, 0, true, signed,
       ppc64_elf_brtaken_reloc),

  /* Like R_PPC64_ADDR16, but referring to the GOT table entry for the
     symbol.  */
  HOW (R_PPC64_GOT16, 2, 16, 0xffff, 0, false, signed,
       ppc64_elf_unhandled_reloc),

  /* Like R_PPC64_ADDR16_LO, but referring to the GOT table entry for
     the symbol.  */
  HOW (R_PPC64_GOT16_LO, 2, 16, 0xffff, 0, false, dont,
       ppc64_elf_unhandled_reloc),

  /* Like R_PPC64_ADDR16_HI, but referring to the GOT table entry for
     the symbol.  */
  HOW (R_PPC64_GOT16_HI, 2, 16, 0xffff, 16, false, signed,
       ppc64_elf_unhandled_reloc),

  /* Like R_PPC64_ADDR16_HA, but referring to the GOT table entry for
     the symbol.  */
  HOW (R_PPC64_GOT16_HA, 2, 16, 0xffff, 16, false, signed,
       ppc64_elf_unhandled_reloc),

  /* This is used only by the dynamic linker.  The symbol should exist
     both in the object being run and in some shared library.  The
     dynamic linker copies the data addressed by the symbol from the
     shared library into the object, because the object being
     run has to have the data at some particular address.  */
  HOW (R_PPC64_COPY, 0, 0, 0, 0, false, dont,
       ppc64_elf_unhandled_reloc),

  /* Like R_PPC64_ADDR64, but used when setting global offset table
     entries.  */
  HOW (R_PPC64_GLOB_DAT, 8, 64, 0xffffffffffffffffULL, 0, false, dont,
       ppc64_elf_unhandled_reloc),

  /* Created by the link editor.  Marks a procedure linkage table
     entry for a symbol.  */
  HOW (R_PPC64_JMP_SLOT, 0, 0, 0, 0, false, dont,
       ppc64_elf_unhandled_reloc),

  /* Used only by the dynamic linker.  When the object is run, this
     doubleword64 is set to the load address of the object, plus the
     addend.  */
  HOW (R_PPC64_RELATIVE, 8, 64, 0xffffffffffffffffULL, 0, false, dont,
       bfd_elf_generic_reloc),

  /* Like R_PPC64_ADDR32, but may be unaligned.  */
  HOW (R_PPC64_UADDR32, 4, 32, 0xffffffff, 0, false, bitfield,
       bfd_elf_generic_reloc),

  /* Like R_PPC64_ADDR16, but may be unaligned.  */
  HOW (R_PPC64_UADDR16, 2, 16, 0xffff, 0, false, bitfield,
       bfd_elf_generic_reloc),

  /* 32-bit PC relative.  */
  HOW (R_PPC64_REL32, 4, 32, 0xffffffff, 0, true, signed,
       bfd_elf_generic_reloc),

  /* 32-bit relocation to the symbol's procedure linkage table.  */
  HOW (R_PPC64_PLT32, 4, 32, 0xffffffff, 0, false, bitfield,
       ppc64_elf_unhandled_reloc),

  /* 32-bit PC relative relocation to the symbol's procedure linkage table.
     FIXME: R_PPC64_PLTREL32 not supported.  */
  HOW (R_PPC64_PLTREL32, 4, 32, 0xffffffff, 0, true, signed,
       ppc64_elf_unhandled_reloc),

  /* Like R_PPC64_ADDR16_LO, but referring to the PLT table entry for
     the symbol.  */
  HOW (R_PPC64_PLT16_LO, 2, 16, 0xffff, 0, false, dont,
       ppc64_elf_unhandled_reloc),

  /* Like R_PPC64_ADDR16_HI, but referring to the PLT table entry for
     the symbol.  */
  HOW (R_PPC64_PLT16_HI, 2, 16, 0xffff, 16, false, signed,
       ppc64_elf_unhandled_reloc),

  /* Like R_PPC64_ADDR16_HA, but referring to the PLT table entry for
     the symbol.  */
  HOW (R_PPC64_PLT16_HA, 2, 16, 0xffff, 16, false, signed,
       ppc64_elf_unhandled_reloc),

  /* 16-bit section relative relocation.  */
  HOW (R_PPC64_SECTOFF, 2, 16, 0xffff, 0, false, signed,
       ppc64_elf_sectoff_reloc),

  /* Like R_PPC64_SECTOFF, but no overflow warning.  */
  HOW (R_PPC64_SECTOFF_LO, 2, 16, 0xffff, 0, false, dont,
       ppc64_elf_sectoff_reloc),

  /* 16-bit upper half section relative relocation.  */
  HOW (R_PPC64_SECTOFF_HI, 2, 16, 0xffff, 16, false, signed,
       ppc64_elf_sectoff_reloc),

  /* 16-bit upper half adjusted section relative relocation.  */
  HOW (R_PPC64_SECTOFF_HA, 2, 16, 0xffff, 16, false, signed,
       ppc64_elf_sectoff_ha_reloc),

  /* Like R_PPC64_REL24 without touching the two least significant bits.  */
  HOW (R_PPC64_REL30, 4, 30, 0xfffffffc, 2, true, dont,
       bfd_elf_generic_reloc),

  /* Relocs in the 64-bit PowerPC ELF ABI, not in the 32-bit ABI.  */

  /* A standard 64-bit relocation.  */
  HOW (R_PPC64_ADDR64, 8, 64, 0xffffffffffffffffULL, 0, false, dont,
       bfd_elf_generic_reloc),

  /* The bits 32-47 of an address.  */
  HOW (R_PPC64_ADDR16_HIGHER, 2, 16, 0xffff, 32, false, dont,
       bfd_elf_generic_reloc),

  /* The bits 32-47 of an address, plus 1 if the contents of the low
     16 bits, treated as a signed number, is negative.  */
  HOW (R_PPC64_ADDR16_HIGHERA, 2, 16, 0xffff, 32, false, dont,
       ppc64_elf_ha_reloc),

  /* The bits 48-63 of an address.  */
  HOW (R_PPC64_ADDR16_HIGHEST, 2, 16, 0xffff, 48, false, dont,
       bfd_elf_generic_reloc),

  /* The bits 48-63 of an address, plus 1 if the contents of the low
     16 bits, treated as a signed number, is negative.  */
  HOW (R_PPC64_ADDR16_HIGHESTA, 2, 16, 0xffff, 48, false, dont,
       ppc64_elf_ha_reloc),

  /* Like ADDR64, but may be unaligned.  */
  HOW (R_PPC64_UADDR64, 8, 64, 0xffffffffffffffffULL, 0, false, dont,
       bfd_elf_generic_reloc),

  /* 64-bit relative relocation.  */
  HOW (R_PPC64_REL64, 8, 64, 0xffffffffffffffffULL, 0, true, dont,
       bfd_elf_generic_reloc),

  /* 64-bit relocation to the symbol's procedure linkage table.  */
  HOW (R_PPC64_PLT64, 8, 64, 0xffffffffffffffffULL, 0, false, dont,
       ppc64_elf_unhandled_reloc),

  /* 64-bit PC relative relocation to the symbol's procedure linkage
     table.  */
  /* FIXME: R_PPC64_PLTREL64 not supported.  */
  HOW (R_PPC64_PLTREL64, 8, 64, 0xffffffffffffffffULL, 0, true, dont,
       ppc64_elf_unhandled_reloc),

  /* 16 bit TOC-relative relocation.  */
  /* R_PPC64_TOC16	  47	   half16*	S + A - .TOC.  */
  HOW (R_PPC64_TOC16, 2, 16, 0xffff, 0, false, signed,
       ppc64_elf_toc_reloc),

  /* 16 bit TOC-relative relocation without overflow.  */
  /* R_PPC64_TOC16_LO	  48	   half16	 #lo (S + A - .TOC.)  */
  HOW (R_PPC64_TOC16_LO, 2, 16, 0xffff, 0, false, dont,
       ppc64_elf_toc_reloc),

  /* 16 bit TOC-relative relocation, high 16 bits.  */
  /* R_PPC64_TOC16_HI	  49	   half16	 #hi (S + A - .TOC.)  */
  HOW (R_PPC64_TOC16_HI, 2, 16, 0xffff, 16, false, signed,
       ppc64_elf_toc_reloc),

  /* 16 bit TOC-relative relocation, high 16 bits, plus 1 if the
     contents of the low 16 bits, treated as a signed number, is
     negative.  */
  /* R_PPC64_TOC16_HA	  50	   half16	 #ha (S + A - .TOC.)  */
  HOW (R_PPC64_TOC16_HA, 2, 16, 0xffff, 16, false, signed,
       ppc64_elf_toc_ha_reloc),

  /* 64-bit relocation; insert value of TOC base (.TOC.).  */
  /* R_PPC64_TOC		  51	   doubleword64	 .TOC.  */
  HOW (R_PPC64_TOC, 8, 64, 0xffffffffffffffffULL, 0, false, dont,
       ppc64_elf_toc64_reloc),

  /* Like R_PPC64_GOT16, but also informs the link editor that the
     value to relocate may (!) refer to a PLT entry which the link
     editor (a) may replace with the symbol value.  If the link editor
     is unable to fully resolve the symbol, it may (b) create a PLT
     entry and store the address to the new PLT entry in the GOT.
     This permits lazy resolution of function symbols at run time.
     The link editor may also skip all of this and just (c) emit a
     R_PPC64_GLOB_DAT to tie the symbol to the GOT entry.  */
  /* FIXME: R_PPC64_PLTGOT16 not implemented.  */
    HOW (R_PPC64_PLTGOT16, 2, 16, 0xffff, 0, false,signed,
	  ppc64_elf_unhandled_reloc),

  /* Like R_PPC64_PLTGOT16, but without overflow.  */
  /* FIXME: R_PPC64_PLTGOT16_LO not implemented.  */
  HOW (R_PPC64_PLTGOT16_LO, 2, 16, 0xffff, 0, false, dont,
       ppc64_elf_unhandled_reloc),

  /* Like R_PPC64_PLT_GOT16, but using bits 16-31 of the address.  */
  /* FIXME: R_PPC64_PLTGOT16_HI not implemented.  */
  HOW (R_PPC64_PLTGOT16_HI, 2, 16, 0xffff, 16, false, signed,
       ppc64_elf_unhandled_reloc),

  /* Like R_PPC64_PLT_GOT16, but using bits 16-31 of the address, plus
     1 if the contents of the low 16 bits, treated as a signed number,
     is negative.  */
  /* FIXME: R_PPC64_PLTGOT16_HA not implemented.  */
  HOW (R_PPC64_PLTGOT16_HA, 2, 16, 0xffff, 16, false, signed,
       ppc64_elf_unhandled_reloc),

  /* Like R_PPC64_ADDR16, but for instructions with a DS field.  */
  HOW (R_PPC64_ADDR16_DS, 2, 16, 0xfffc, 0, false, signed,
       bfd_elf_generic_reloc),

  /* Like R_PPC64_ADDR16_LO, but for instructions with a DS field.  */
  HOW (R_PPC64_ADDR16_LO_DS, 2, 16, 0xfffc, 0, false, dont,
       bfd_elf_generic_reloc),

  /* Like R_PPC64_GOT16, but for instructions with a DS field.  */
  HOW (R_PPC64_GOT16_DS, 2, 16, 0xfffc, 0, false, signed,
       ppc64_elf_unhandled_reloc),

  /* Like R_PPC64_GOT16_LO, but for instructions with a DS field.  */
  HOW (R_PPC64_GOT16_LO_DS, 2, 16, 0xfffc, 0, false, dont,
       ppc64_elf_unhandled_reloc),

  /* Like R_PPC64_PLT16_LO, but for instructions with a DS field.  */
  HOW (R_PPC64_PLT16_LO_DS, 2, 16, 0xfffc, 0, false, dont,
       ppc64_elf_unhandled_reloc),

  /* Like R_PPC64_SECTOFF, but for instructions with a DS field.  */
  HOW (R_PPC64_SECTOFF_DS, 2, 16, 0xfffc, 0, false, signed,
       ppc64_elf_sectoff_reloc),

  /* Like R_PPC64_SECTOFF_LO, but for instructions with a DS field.  */
  HOW (R_PPC64_SECTOFF_LO_DS, 2, 16, 0xfffc, 0, false, dont,
       ppc64_elf_sectoff_reloc),

  /* Like R_PPC64_TOC16, but for instructions with a DS field.  */
  HOW (R_PPC64_TOC16_DS, 2, 16, 0xfffc, 0, false, signed,
       ppc64_elf_toc_reloc),

  /* Like R_PPC64_TOC16_LO, but for instructions with a DS field.  */
  HOW (R_PPC64_TOC16_LO_DS, 2, 16, 0xfffc, 0, false, dont,
       ppc64_elf_toc_reloc),

  /* Like R_PPC64_PLTGOT16, but for instructions with a DS field.  */
  /* FIXME: R_PPC64_PLTGOT16_DS not implemented.  */
  HOW (R_PPC64_PLTGOT16_DS, 2, 16, 0xfffc, 0, false, signed,
       ppc64_elf_unhandled_reloc),

  /* Like R_PPC64_PLTGOT16_LO, but for instructions with a DS field.  */
  /* FIXME: R_PPC64_PLTGOT16_LO not implemented.  */
  HOW (R_PPC64_PLTGOT16_LO_DS, 2, 16, 0xfffc, 0, false, dont,
       ppc64_elf_unhandled_reloc),

  /* Marker relocs for TLS.  */
  HOW (R_PPC64_TLS, 4, 32, 0, 0, false, dont,
       bfd_elf_generic_reloc),

  HOW (R_PPC64_TLSGD, 4, 32, 0, 0, false, dont,
       bfd_elf_generic_reloc),

  HOW (R_PPC64_TLSLD, 4, 32, 0, 0, false, dont,
       bfd_elf_generic_reloc),

  /* Marker reloc for optimizing r2 save in prologue rather than on
     each plt call stub.  */
  HOW (R_PPC64_TOCSAVE, 4, 32, 0, 0, false, dont,
       bfd_elf_generic_reloc),

  /* Marker relocs on inline plt call instructions.  */
  HOW (R_PPC64_PLTSEQ, 4, 32, 0, 0, false, dont,
       bfd_elf_generic_reloc),

  HOW (R_PPC64_PLTCALL, 4, 32, 0, 0, false, dont,
       bfd_elf_generic_reloc),

  /* Computes the load module index of the load module that contains the
     definition of its TLS sym.  */
  HOW (R_PPC64_DTPMOD64, 8, 64, 0xffffffffffffffffULL, 0, false, dont,
       ppc64_elf_unhandled_reloc),

  /* Computes a dtv-relative displacement, the difference between the value
     of sym+add and the base address of the thread-local storage block that
     contains the definition of sym, minus 0x8000.  */
  HOW (R_PPC64_DTPREL64, 8, 64, 0xffffffffffffffffULL, 0, false, dont,
       ppc64_elf_unhandled_reloc),

  /* A 16 bit dtprel reloc.  */
  HOW (R_PPC64_DTPREL16, 2, 16, 0xffff, 0, false, signed,
       ppc64_elf_unhandled_reloc),

  /* Like DTPREL16, but no overflow.  */
  HOW (R_PPC64_DTPREL16_LO, 2, 16, 0xffff, 0, false, dont,
       ppc64_elf_unhandled_reloc),

  /* Like DTPREL16_LO, but next higher group of 16 bits.  */
  HOW (R_PPC64_DTPREL16_HI, 2, 16, 0xffff, 16, false, signed,
       ppc64_elf_unhandled_reloc),

  /* Like DTPREL16_HI, but adjust for low 16 bits.  */
  HOW (R_PPC64_DTPREL16_HA, 2, 16, 0xffff, 16, false, signed,
       ppc64_elf_unhandled_reloc),

  /* Like DTPREL16_HI, but next higher group of 16 bits.  */
  HOW (R_PPC64_DTPREL16_HIGHER, 2, 16, 0xffff, 32, false, dont,
       ppc64_elf_unhandled_reloc),

  /* Like DTPREL16_HIGHER, but adjust for low 16 bits.  */
  HOW (R_PPC64_DTPREL16_HIGHERA, 2, 16, 0xffff, 32, false, dont,
       ppc64_elf_unhandled_reloc),

  /* Like DTPREL16_HIGHER, but next higher group of 16 bits.  */
  HOW (R_PPC64_DTPREL16_HIGHEST, 2, 16, 0xffff, 48, false, dont,
       ppc64_elf_unhandled_reloc),

  /* Like DTPREL16_HIGHEST, but adjust for low 16 bits.  */
  HOW (R_PPC64_DTPREL16_HIGHESTA, 2, 16, 0xffff, 48, false, dont,
       ppc64_elf_unhandled_reloc),

  /* Like DTPREL16, but for insns with a DS field.  */
  HOW (R_PPC64_DTPREL16_DS, 2, 16, 0xfffc, 0, false, signed,
       ppc64_elf_unhandled_reloc),

  /* Like DTPREL16_DS, but no overflow.  */
  HOW (R_PPC64_DTPREL16_LO_DS, 2, 16, 0xfffc, 0, false, dont,
       ppc64_elf_unhandled_reloc),

  /* Computes a tp-relative displacement, the difference between the value of
     sym+add and the value of the thread pointer (r13).  */
  HOW (R_PPC64_TPREL64, 8, 64, 0xffffffffffffffffULL, 0, false, dont,
       ppc64_elf_unhandled_reloc),

  /* A 16 bit tprel reloc.  */
  HOW (R_PPC64_TPREL16, 2, 16, 0xffff, 0, false, signed,
       ppc64_elf_unhandled_reloc),

  /* Like TPREL16, but no overflow.  */
  HOW (R_PPC64_TPREL16_LO, 2, 16, 0xffff, 0, false, dont,
       ppc64_elf_unhandled_reloc),

  /* Like TPREL16_LO, but next higher group of 16 bits.  */
  HOW (R_PPC64_TPREL16_HI, 2, 16, 0xffff, 16, false, signed,
       ppc64_elf_unhandled_reloc),

  /* Like TPREL16_HI, but adjust for low 16 bits.  */
  HOW (R_PPC64_TPREL16_HA, 2, 16, 0xffff, 16, false, signed,
       ppc64_elf_unhandled_reloc),

  /* Like TPREL16_HI, but next higher group of 16 bits.  */
  HOW (R_PPC64_TPREL16_HIGHER, 2, 16, 0xffff, 32, false, dont,
       ppc64_elf_unhandled_reloc),

  /* Like TPREL16_HIGHER, but adjust for low 16 bits.  */
  HOW (R_PPC64_TPREL16_HIGHERA, 2, 16, 0xffff, 32, false, dont,
       ppc64_elf_unhandled_reloc),

  /* Like TPREL16_HIGHER, but next higher group of 16 bits.  */
  HOW (R_PPC64_TPREL16_HIGHEST, 2, 16, 0xffff, 48, false, dont,
       ppc64_elf_unhandled_reloc),

  /* Like TPREL16_HIGHEST, but adjust for low 16 bits.  */
  HOW (R_PPC64_TPREL16_HIGHESTA, 2, 16, 0xffff, 48, false, dont,
       ppc64_elf_unhandled_reloc),

  /* Like TPREL16, but for insns with a DS field.  */
  HOW (R_PPC64_TPREL16_DS, 2, 16, 0xfffc, 0, false, signed,
       ppc64_elf_unhandled_reloc),

  /* Like TPREL16_DS, but no overflow.  */
  HOW (R_PPC64_TPREL16_LO_DS, 2, 16, 0xfffc, 0, false, dont,
       ppc64_elf_unhandled_reloc),

  /* Allocates two contiguous entries in the GOT to hold a tls_index structure,
     with values (sym+add)@dtpmod and (sym+add)@dtprel, and computes the offset
     to the first entry relative to the TOC base (r2).  */
  HOW (R_PPC64_GOT_TLSGD16, 2, 16, 0xffff, 0, false, signed,
       ppc64_elf_unhandled_reloc),

  /* Like GOT_TLSGD16, but no overflow.  */
  HOW (R_PPC64_GOT_TLSGD16_LO, 2, 16, 0xffff, 0, false, dont,
       ppc64_elf_unhandled_reloc),

  /* Like GOT_TLSGD16_LO, but next higher group of 16 bits.  */
  HOW (R_PPC64_GOT_TLSGD16_HI, 2, 16, 0xffff, 16, false, signed,
       ppc64_elf_unhandled_reloc),

  /* Like GOT_TLSGD16_HI, but adjust for low 16 bits.  */
  HOW (R_PPC64_GOT_TLSGD16_HA, 2, 16, 0xffff, 16, false, signed,
       ppc64_elf_unhandled_reloc),

  /* Allocates two contiguous entries in the GOT to hold a tls_index structure,
     with values (sym+add)@dtpmod and zero, and computes the offset to the
     first entry relative to the TOC base (r2).  */
  HOW (R_PPC64_GOT_TLSLD16, 2, 16, 0xffff, 0, false, signed,
       ppc64_elf_unhandled_reloc),

  /* Like GOT_TLSLD16, but no overflow.  */
  HOW (R_PPC64_GOT_TLSLD16_LO, 2, 16, 0xffff, 0, false, dont,
       ppc64_elf_unhandled_reloc),

  /* Like GOT_TLSLD16_LO, but next higher group of 16 bits.  */
  HOW (R_PPC64_GOT_TLSLD16_HI, 2, 16, 0xffff, 16, false, signed,
       ppc64_elf_unhandled_reloc),

  /* Like GOT_TLSLD16_HI, but adjust for low 16 bits.  */
  HOW (R_PPC64_GOT_TLSLD16_HA, 2, 16, 0xffff, 16, false, signed,
       ppc64_elf_unhandled_reloc),

  /* Allocates an entry in the GOT with value (sym+add)@dtprel, and computes
     the offset to the entry relative to the TOC base (r2).  */
  HOW (R_PPC64_GOT_DTPREL16_DS, 2, 16, 0xfffc, 0, false, signed,
       ppc64_elf_unhandled_reloc),

  /* Like GOT_DTPREL16_DS, but no overflow.  */
  HOW (R_PPC64_GOT_DTPREL16_LO_DS, 2, 16, 0xfffc, 0, false, dont,
       ppc64_elf_unhandled_reloc),

  /* Like GOT_DTPREL16_LO_DS, but next higher group of 16 bits.  */
  HOW (R_PPC64_GOT_DTPREL16_HI, 2, 16, 0xffff, 16, false, signed,
       ppc64_elf_unhandled_reloc),

  /* Like GOT_DTPREL16_HI, but adjust for low 16 bits.  */
  HOW (R_PPC64_GOT_DTPREL16_HA, 2, 16, 0xffff, 16, false, signed,
       ppc64_elf_unhandled_reloc),

  /* Allocates an entry in the GOT with value (sym+add)@tprel, and computes the
     offset to the entry relative to the TOC base (r2).  */
  HOW (R_PPC64_GOT_TPREL16_DS, 2, 16, 0xfffc, 0, false, signed,
       ppc64_elf_unhandled_reloc),

  /* Like GOT_TPREL16_DS, but no overflow.  */
  HOW (R_PPC64_GOT_TPREL16_LO_DS, 2, 16, 0xfffc, 0, false, dont,
       ppc64_elf_unhandled_reloc),

  /* Like GOT_TPREL16_LO_DS, but next higher group of 16 bits.  */
  HOW (R_PPC64_GOT_TPREL16_HI, 2, 16, 0xffff, 16, false, signed,
       ppc64_elf_unhandled_reloc),

  /* Like GOT_TPREL16_HI, but adjust for low 16 bits.  */
  HOW (R_PPC64_GOT_TPREL16_HA, 2, 16, 0xffff, 16, false, signed,
       ppc64_elf_unhandled_reloc),

  HOW (R_PPC64_JMP_IREL, 0, 0, 0, 0, false, dont,
       ppc64_elf_unhandled_reloc),

  HOW (R_PPC64_IRELATIVE, 8, 64, 0xffffffffffffffffULL, 0, false, dont,
       bfd_elf_generic_reloc),

  /* A 16 bit relative relocation.  */
  HOW (R_PPC64_REL16, 2, 16, 0xffff, 0, true, signed,
       bfd_elf_generic_reloc),

  /* A 16 bit relative relocation without overflow.  */
  HOW (R_PPC64_REL16_LO, 2, 16, 0xffff, 0, true, dont,
       bfd_elf_generic_reloc),

  /* The high order 16 bits of a relative address.  */
  HOW (R_PPC64_REL16_HI, 2, 16, 0xffff, 16, true, signed,
       bfd_elf_generic_reloc),

  /* The high order 16 bits of a relative address, plus 1 if the contents of
     the low 16 bits, treated as a signed number, is negative.  */
  HOW (R_PPC64_REL16_HA, 2, 16, 0xffff, 16, true, signed,
       ppc64_elf_ha_reloc),

  HOW (R_PPC64_REL16_HIGH, 2, 16, 0xffff, 16, true, dont,
       bfd_elf_generic_reloc),

  HOW (R_PPC64_REL16_HIGHA, 2, 16, 0xffff, 16, true, dont,
       ppc64_elf_ha_reloc),

  HOW (R_PPC64_REL16_HIGHER, 2, 16, 0xffff, 32, true, dont,
       bfd_elf_generic_reloc),

  HOW (R_PPC64_REL16_HIGHERA, 2, 16, 0xffff, 32, true, dont,
       ppc64_elf_ha_reloc),

  HOW (R_PPC64_REL16_HIGHEST, 2, 16, 0xffff, 48, true, dont,
       bfd_elf_generic_reloc),

  HOW (R_PPC64_REL16_HIGHESTA, 2, 16, 0xffff, 48, true, dont,
       ppc64_elf_ha_reloc),

  /* Like R_PPC64_REL16_HA but for split field in addpcis.  */
  HOW (R_PPC64_REL16DX_HA, 4, 16, 0x1fffc1, 16, true, signed,
       ppc64_elf_ha_reloc),

  /* A split-field reloc for addpcis, non-relative (gas internal use only).  */
  HOW (R_PPC64_16DX_HA, 4, 16, 0x1fffc1, 16, false, signed,
       ppc64_elf_ha_reloc),

  /* Like R_PPC64_ADDR16_HI, but no overflow.  */
  HOW (R_PPC64_ADDR16_HIGH, 2, 16, 0xffff, 16, false, dont,
       bfd_elf_generic_reloc),

  /* Like R_PPC64_ADDR16_HA, but no overflow.  */
  HOW (R_PPC64_ADDR16_HIGHA, 2, 16, 0xffff, 16, false, dont,
       ppc64_elf_ha_reloc),

  /* Like R_PPC64_DTPREL16_HI, but no overflow.  */
  HOW (R_PPC64_DTPREL16_HIGH, 2, 16, 0xffff, 16, false, dont,
       ppc64_elf_unhandled_reloc),

  /* Like R_PPC64_DTPREL16_HA, but no overflow.  */
  HOW (R_PPC64_DTPREL16_HIGHA, 2, 16, 0xffff, 16, false, dont,
       ppc64_elf_unhandled_reloc),

  /* Like R_PPC64_TPREL16_HI, but no overflow.  */
  HOW (R_PPC64_TPREL16_HIGH, 2, 16, 0xffff, 16, false, dont,
       ppc64_elf_unhandled_reloc),

  /* Like R_PPC64_TPREL16_HA, but no overflow.  */
  HOW (R_PPC64_TPREL16_HIGHA, 2, 16, 0xffff, 16, false, dont,
       ppc64_elf_unhandled_reloc),

  /* Marker reloc on ELFv2 large-model function entry.  */
  HOW (R_PPC64_ENTRY, 4, 32, 0, 0, false, dont,
       bfd_elf_generic_reloc),

  /* Like ADDR64, but use local entry point of function.  */
  HOW (R_PPC64_ADDR64_LOCAL, 8, 64, 0xffffffffffffffffULL, 0, false, dont,
       bfd_elf_generic_reloc),

  HOW (R_PPC64_PLTSEQ_NOTOC, 4, 32, 0, 0, false, dont,
       bfd_elf_generic_reloc),

  HOW (R_PPC64_PLTCALL_NOTOC, 4, 32, 0, 0, false, dont,
       bfd_elf_generic_reloc),

  HOW (R_PPC64_PCREL_OPT, 4, 32, 0, 0, false, dont,
       bfd_elf_generic_reloc),

  HOW (R_PPC64_D34, 8, 34, 0x3ffff0000ffffULL, 0, false, signed,
       ppc64_elf_prefix_reloc),

  HOW (R_PPC64_D34_LO, 8, 34, 0x3ffff0000ffffULL, 0, false, dont,
       ppc64_elf_prefix_reloc),

  HOW (R_PPC64_D34_HI30, 8, 34, 0x3ffff0000ffffULL, 34, false, dont,
       ppc64_elf_prefix_reloc),

  HOW (R_PPC64_D34_HA30, 8, 34, 0x3ffff0000ffffULL, 34, false, dont,
       ppc64_elf_prefix_reloc),

  HOW (R_PPC64_PCREL34, 8, 34, 0x3ffff0000ffffULL, 0, true, signed,
       ppc64_elf_prefix_reloc),

  HOW (R_PPC64_GOT_PCREL34, 8, 34, 0x3ffff0000ffffULL, 0, true, signed,
       ppc64_elf_unhandled_reloc),

  HOW (R_PPC64_PLT_PCREL34, 8, 34, 0x3ffff0000ffffULL, 0, true, signed,
       ppc64_elf_unhandled_reloc),

  HOW (R_PPC64_PLT_PCREL34_NOTOC, 8, 34, 0x3ffff0000ffffULL, 0, true, signed,
       ppc64_elf_unhandled_reloc),

  HOW (R_PPC64_TPREL34, 8, 34, 0x3ffff0000ffffULL, 0, false, signed,
       ppc64_elf_unhandled_reloc),

  HOW (R_PPC64_DTPREL34, 8, 34, 0x3ffff0000ffffULL, 0, false, signed,
       ppc64_elf_unhandled_reloc),

  HOW (R_PPC64_GOT_TLSGD_PCREL34, 8, 34, 0x3ffff0000ffffULL, 0, true, signed,
       ppc64_elf_unhandled_reloc),

  HOW (R_PPC64_GOT_TLSLD_PCREL34, 8, 34, 0x3ffff0000ffffULL, 0, true, signed,
       ppc64_elf_unhandled_reloc),

  HOW (R_PPC64_GOT_TPREL_PCREL34, 8, 34, 0x3ffff0000ffffULL, 0, true, signed,
       ppc64_elf_unhandled_reloc),

  HOW (R_PPC64_GOT_DTPREL_PCREL34, 8, 34, 0x3ffff0000ffffULL, 0, true, signed,
       ppc64_elf_unhandled_reloc),

  HOW (R_PPC64_ADDR16_HIGHER34, 2, 16, 0xffff, 34, false, dont,
       bfd_elf_generic_reloc),

  HOW (R_PPC64_ADDR16_HIGHERA34, 2, 16, 0xffff, 34, false, dont,
       ppc64_elf_ha_reloc),

  HOW (R_PPC64_ADDR16_HIGHEST34, 2, 16, 0xffff, 50, false, dont,
       bfd_elf_generic_reloc),

  HOW (R_PPC64_ADDR16_HIGHESTA34, 2, 16, 0xffff, 50, false, dont,
       ppc64_elf_ha_reloc),

  HOW (R_PPC64_REL16_HIGHER34, 2, 16, 0xffff, 34, true, dont,
       bfd_elf_generic_reloc),

  HOW (R_PPC64_REL16_HIGHERA34, 2, 16, 0xffff, 34, true, dont,
       ppc64_elf_ha_reloc),

  HOW (R_PPC64_REL16_HIGHEST34, 2, 16, 0xffff, 50, true, dont,
       bfd_elf_generic_reloc),

  HOW (R_PPC64_REL16_HIGHESTA34, 2, 16, 0xffff, 50, true, dont,
       ppc64_elf_ha_reloc),

  HOW (R_PPC64_D28, 8, 28, 0xfff0000ffffULL, 0, false, signed,
       ppc64_elf_prefix_reloc),

  HOW (R_PPC64_PCREL28, 8, 28, 0xfff0000ffffULL, 0, true, signed,
       ppc64_elf_prefix_reloc),

  /* GNU extension to record C++ vtable hierarchy.  */
  HOW (R_PPC64_GNU_VTINHERIT, 0, 0, 0, 0, false, dont,
       NULL),

  /* GNU extension to record C++ vtable member usage.  */
  HOW (R_PPC64_GNU_VTENTRY, 0, 0, 0, 0, false, dont,
       NULL),
};


/* Initialize the ppc64_elf_howto_table, so that linear accesses can
   be done.  */

static void
ppc_howto_init (void)
{
  unsigned int i, type;

  for (i = 0; i < ARRAY_SIZE (ppc64_elf_howto_raw); i++)
    {
      type = ppc64_elf_howto_raw[i].type;
      BFD_ASSERT (type < ARRAY_SIZE (ppc64_elf_howto_table));
      ppc64_elf_howto_table[type] = &ppc64_elf_howto_raw[i];
    }
}

static reloc_howto_type *
ppc64_elf_reloc_type_lookup (bfd *abfd, bfd_reloc_code_real_type code)
{
  enum elf_ppc64_reloc_type r = R_PPC64_NONE;

  if (!ppc64_elf_howto_table[R_PPC64_ADDR32])
    /* Initialize howto table if needed.  */
    ppc_howto_init ();

  switch (code)
    {
    default:
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: unsupported relocation type %#x"), abfd,
			  (int) code);
      bfd_set_error (bfd_error_bad_value);
      return NULL;

    case BFD_RELOC_NONE:			r = R_PPC64_NONE;
      break;
    case BFD_RELOC_32:				r = R_PPC64_ADDR32;
      break;
    case BFD_RELOC_PPC_BA26:			r = R_PPC64_ADDR24;
      break;
    case BFD_RELOC_16:				r = R_PPC64_ADDR16;
      break;
    case BFD_RELOC_LO16:			r = R_PPC64_ADDR16_LO;
      break;
    case BFD_RELOC_HI16:			r = R_PPC64_ADDR16_HI;
      break;
    case BFD_RELOC_PPC64_ADDR16_HIGH:		r = R_PPC64_ADDR16_HIGH;
      break;
    case BFD_RELOC_HI16_S:			r = R_PPC64_ADDR16_HA;
      break;
    case BFD_RELOC_PPC64_ADDR16_HIGHA:		r = R_PPC64_ADDR16_HIGHA;
      break;
    case BFD_RELOC_PPC_BA16:			r = R_PPC64_ADDR14;
      break;
    case BFD_RELOC_PPC_BA16_BRTAKEN:		r = R_PPC64_ADDR14_BRTAKEN;
      break;
    case BFD_RELOC_PPC_BA16_BRNTAKEN:		r = R_PPC64_ADDR14_BRNTAKEN;
      break;
    case BFD_RELOC_PPC_B26:			r = R_PPC64_REL24;
      break;
    case BFD_RELOC_PPC64_REL24_NOTOC:		r = R_PPC64_REL24_NOTOC;
      break;
    case BFD_RELOC_PPC64_REL24_P9NOTOC:		r = R_PPC64_REL24_P9NOTOC;
      break;
    case BFD_RELOC_PPC_B16:			r = R_PPC64_REL14;
      break;
    case BFD_RELOC_PPC_B16_BRTAKEN:		r = R_PPC64_REL14_BRTAKEN;
      break;
    case BFD_RELOC_PPC_B16_BRNTAKEN:		r = R_PPC64_REL14_BRNTAKEN;
      break;
    case BFD_RELOC_16_GOTOFF:			r = R_PPC64_GOT16;
      break;
    case BFD_RELOC_LO16_GOTOFF:			r = R_PPC64_GOT16_LO;
      break;
    case BFD_RELOC_HI16_GOTOFF:			r = R_PPC64_GOT16_HI;
      break;
    case BFD_RELOC_HI16_S_GOTOFF:		r = R_PPC64_GOT16_HA;
      break;
    case BFD_RELOC_PPC_COPY:			r = R_PPC64_COPY;
      break;
    case BFD_RELOC_PPC_GLOB_DAT:		r = R_PPC64_GLOB_DAT;
      break;
    case BFD_RELOC_32_PCREL:			r = R_PPC64_REL32;
      break;
    case BFD_RELOC_32_PLTOFF:			r = R_PPC64_PLT32;
      break;
    case BFD_RELOC_32_PLT_PCREL:		r = R_PPC64_PLTREL32;
      break;
    case BFD_RELOC_LO16_PLTOFF:			r = R_PPC64_PLT16_LO;
      break;
    case BFD_RELOC_HI16_PLTOFF:			r = R_PPC64_PLT16_HI;
      break;
    case BFD_RELOC_HI16_S_PLTOFF:		r = R_PPC64_PLT16_HA;
      break;
    case BFD_RELOC_16_BASEREL:			r = R_PPC64_SECTOFF;
      break;
    case BFD_RELOC_LO16_BASEREL:		r = R_PPC64_SECTOFF_LO;
      break;
    case BFD_RELOC_HI16_BASEREL:		r = R_PPC64_SECTOFF_HI;
      break;
    case BFD_RELOC_HI16_S_BASEREL:		r = R_PPC64_SECTOFF_HA;
      break;
    case BFD_RELOC_CTOR:			r = R_PPC64_ADDR64;
      break;
    case BFD_RELOC_64:				r = R_PPC64_ADDR64;
      break;
    case BFD_RELOC_PPC64_HIGHER:		r = R_PPC64_ADDR16_HIGHER;
      break;
    case BFD_RELOC_PPC64_HIGHER_S:		r = R_PPC64_ADDR16_HIGHERA;
      break;
    case BFD_RELOC_PPC64_HIGHEST:		r = R_PPC64_ADDR16_HIGHEST;
      break;
    case BFD_RELOC_PPC64_HIGHEST_S:		r = R_PPC64_ADDR16_HIGHESTA;
      break;
    case BFD_RELOC_64_PCREL:			r = R_PPC64_REL64;
      break;
    case BFD_RELOC_64_PLTOFF:			r = R_PPC64_PLT64;
      break;
    case BFD_RELOC_64_PLT_PCREL:		r = R_PPC64_PLTREL64;
      break;
    case BFD_RELOC_PPC_TOC16:			r = R_PPC64_TOC16;
      break;
    case BFD_RELOC_PPC64_TOC16_LO:		r = R_PPC64_TOC16_LO;
      break;
    case BFD_RELOC_PPC64_TOC16_HI:		r = R_PPC64_TOC16_HI;
      break;
    case BFD_RELOC_PPC64_TOC16_HA:		r = R_PPC64_TOC16_HA;
      break;
    case BFD_RELOC_PPC64_TOC:			r = R_PPC64_TOC;
      break;
    case BFD_RELOC_PPC64_PLTGOT16:		r = R_PPC64_PLTGOT16;
      break;
    case BFD_RELOC_PPC64_PLTGOT16_LO:		r = R_PPC64_PLTGOT16_LO;
      break;
    case BFD_RELOC_PPC64_PLTGOT16_HI:		r = R_PPC64_PLTGOT16_HI;
      break;
    case BFD_RELOC_PPC64_PLTGOT16_HA:		r = R_PPC64_PLTGOT16_HA;
      break;
    case BFD_RELOC_PPC64_ADDR16_DS:		r = R_PPC64_ADDR16_DS;
      break;
    case BFD_RELOC_PPC64_ADDR16_LO_DS:		r = R_PPC64_ADDR16_LO_DS;
      break;
    case BFD_RELOC_PPC64_GOT16_DS:		r = R_PPC64_GOT16_DS;
      break;
    case BFD_RELOC_PPC64_GOT16_LO_DS:		r = R_PPC64_GOT16_LO_DS;
      break;
    case BFD_RELOC_PPC64_PLT16_LO_DS:		r = R_PPC64_PLT16_LO_DS;
      break;
    case BFD_RELOC_PPC64_SECTOFF_DS:		r = R_PPC64_SECTOFF_DS;
      break;
    case BFD_RELOC_PPC64_SECTOFF_LO_DS:		r = R_PPC64_SECTOFF_LO_DS;
      break;
    case BFD_RELOC_PPC64_TOC16_DS:		r = R_PPC64_TOC16_DS;
      break;
    case BFD_RELOC_PPC64_TOC16_LO_DS:		r = R_PPC64_TOC16_LO_DS;
      break;
    case BFD_RELOC_PPC64_PLTGOT16_DS:		r = R_PPC64_PLTGOT16_DS;
      break;
    case BFD_RELOC_PPC64_PLTGOT16_LO_DS:	r = R_PPC64_PLTGOT16_LO_DS;
      break;
    case BFD_RELOC_PPC64_TLS_PCREL:
    case BFD_RELOC_PPC_TLS:			r = R_PPC64_TLS;
      break;
    case BFD_RELOC_PPC_TLSGD:			r = R_PPC64_TLSGD;
      break;
    case BFD_RELOC_PPC_TLSLD:			r = R_PPC64_TLSLD;
      break;
    case BFD_RELOC_PPC_DTPMOD:			r = R_PPC64_DTPMOD64;
      break;
    case BFD_RELOC_PPC_TPREL16:			r = R_PPC64_TPREL16;
      break;
    case BFD_RELOC_PPC_TPREL16_LO:		r = R_PPC64_TPREL16_LO;
      break;
    case BFD_RELOC_PPC_TPREL16_HI:		r = R_PPC64_TPREL16_HI;
      break;
    case BFD_RELOC_PPC64_TPREL16_HIGH:		r = R_PPC64_TPREL16_HIGH;
      break;
    case BFD_RELOC_PPC_TPREL16_HA:		r = R_PPC64_TPREL16_HA;
      break;
    case BFD_RELOC_PPC64_TPREL16_HIGHA:		r = R_PPC64_TPREL16_HIGHA;
      break;
    case BFD_RELOC_PPC_TPREL:			r = R_PPC64_TPREL64;
      break;
    case BFD_RELOC_PPC_DTPREL16:		r = R_PPC64_DTPREL16;
      break;
    case BFD_RELOC_PPC_DTPREL16_LO:		r = R_PPC64_DTPREL16_LO;
      break;
    case BFD_RELOC_PPC_DTPREL16_HI:		r = R_PPC64_DTPREL16_HI;
      break;
    case BFD_RELOC_PPC64_DTPREL16_HIGH:		r = R_PPC64_DTPREL16_HIGH;
      break;
    case BFD_RELOC_PPC_DTPREL16_HA:		r = R_PPC64_DTPREL16_HA;
      break;
    case BFD_RELOC_PPC64_DTPREL16_HIGHA:	r = R_PPC64_DTPREL16_HIGHA;
      break;
    case BFD_RELOC_PPC_DTPREL:			r = R_PPC64_DTPREL64;
      break;
    case BFD_RELOC_PPC_GOT_TLSGD16:		r = R_PPC64_GOT_TLSGD16;
      break;
    case BFD_RELOC_PPC_GOT_TLSGD16_LO:		r = R_PPC64_GOT_TLSGD16_LO;
      break;
    case BFD_RELOC_PPC_GOT_TLSGD16_HI:		r = R_PPC64_GOT_TLSGD16_HI;
      break;
    case BFD_RELOC_PPC_GOT_TLSGD16_HA:		r = R_PPC64_GOT_TLSGD16_HA;
      break;
    case BFD_RELOC_PPC_GOT_TLSLD16:		r = R_PPC64_GOT_TLSLD16;
      break;
    case BFD_RELOC_PPC_GOT_TLSLD16_LO:		r = R_PPC64_GOT_TLSLD16_LO;
      break;
    case BFD_RELOC_PPC_GOT_TLSLD16_HI:		r = R_PPC64_GOT_TLSLD16_HI;
      break;
    case BFD_RELOC_PPC_GOT_TLSLD16_HA:		r = R_PPC64_GOT_TLSLD16_HA;
      break;
    case BFD_RELOC_PPC_GOT_TPREL16:		r = R_PPC64_GOT_TPREL16_DS;
      break;
    case BFD_RELOC_PPC_GOT_TPREL16_LO:		r = R_PPC64_GOT_TPREL16_LO_DS;
      break;
    case BFD_RELOC_PPC_GOT_TPREL16_HI:		r = R_PPC64_GOT_TPREL16_HI;
      break;
    case BFD_RELOC_PPC_GOT_TPREL16_HA:		r = R_PPC64_GOT_TPREL16_HA;
      break;
    case BFD_RELOC_PPC_GOT_DTPREL16:		r = R_PPC64_GOT_DTPREL16_DS;
      break;
    case BFD_RELOC_PPC_GOT_DTPREL16_LO:		r = R_PPC64_GOT_DTPREL16_LO_DS;
      break;
    case BFD_RELOC_PPC_GOT_DTPREL16_HI:		r = R_PPC64_GOT_DTPREL16_HI;
      break;
    case BFD_RELOC_PPC_GOT_DTPREL16_HA:		r = R_PPC64_GOT_DTPREL16_HA;
      break;
    case BFD_RELOC_PPC64_TPREL16_DS:		r = R_PPC64_TPREL16_DS;
      break;
    case BFD_RELOC_PPC64_TPREL16_LO_DS:		r = R_PPC64_TPREL16_LO_DS;
      break;
    case BFD_RELOC_PPC64_TPREL16_HIGHER:	r = R_PPC64_TPREL16_HIGHER;
      break;
    case BFD_RELOC_PPC64_TPREL16_HIGHERA:	r = R_PPC64_TPREL16_HIGHERA;
      break;
    case BFD_RELOC_PPC64_TPREL16_HIGHEST:	r = R_PPC64_TPREL16_HIGHEST;
      break;
    case BFD_RELOC_PPC64_TPREL16_HIGHESTA:	r = R_PPC64_TPREL16_HIGHESTA;
      break;
    case BFD_RELOC_PPC64_DTPREL16_DS:		r = R_PPC64_DTPREL16_DS;
      break;
    case BFD_RELOC_PPC64_DTPREL16_LO_DS:	r = R_PPC64_DTPREL16_LO_DS;
      break;
    case BFD_RELOC_PPC64_DTPREL16_HIGHER:	r = R_PPC64_DTPREL16_HIGHER;
      break;
    case BFD_RELOC_PPC64_DTPREL16_HIGHERA:	r = R_PPC64_DTPREL16_HIGHERA;
      break;
    case BFD_RELOC_PPC64_DTPREL16_HIGHEST:	r = R_PPC64_DTPREL16_HIGHEST;
      break;
    case BFD_RELOC_PPC64_DTPREL16_HIGHESTA:	r = R_PPC64_DTPREL16_HIGHESTA;
      break;
    case BFD_RELOC_16_PCREL:			r = R_PPC64_REL16;
      break;
    case BFD_RELOC_LO16_PCREL:			r = R_PPC64_REL16_LO;
      break;
    case BFD_RELOC_HI16_PCREL:			r = R_PPC64_REL16_HI;
      break;
    case BFD_RELOC_HI16_S_PCREL:		r = R_PPC64_REL16_HA;
      break;
    case BFD_RELOC_PPC64_REL16_HIGH:		r = R_PPC64_REL16_HIGH;
      break;
    case BFD_RELOC_PPC64_REL16_HIGHA:		r = R_PPC64_REL16_HIGHA;
      break;
    case BFD_RELOC_PPC64_REL16_HIGHER:		r = R_PPC64_REL16_HIGHER;
      break;
    case BFD_RELOC_PPC64_REL16_HIGHERA:		r = R_PPC64_REL16_HIGHERA;
      break;
    case BFD_RELOC_PPC64_REL16_HIGHEST:		r = R_PPC64_REL16_HIGHEST;
      break;
    case BFD_RELOC_PPC64_REL16_HIGHESTA:	r = R_PPC64_REL16_HIGHESTA;
      break;
    case BFD_RELOC_PPC_16DX_HA:			r = R_PPC64_16DX_HA;
      break;
    case BFD_RELOC_PPC_REL16DX_HA:		r = R_PPC64_REL16DX_HA;
      break;
    case BFD_RELOC_PPC64_ENTRY:			r = R_PPC64_ENTRY;
      break;
    case BFD_RELOC_PPC64_ADDR64_LOCAL:		r = R_PPC64_ADDR64_LOCAL;
      break;
    case BFD_RELOC_PPC64_D34:			r = R_PPC64_D34;
      break;
    case BFD_RELOC_PPC64_D34_LO:		r = R_PPC64_D34_LO;
      break;
    case BFD_RELOC_PPC64_D34_HI30:		r = R_PPC64_D34_HI30;
      break;
    case BFD_RELOC_PPC64_D34_HA30:		r = R_PPC64_D34_HA30;
      break;
    case BFD_RELOC_PPC64_PCREL34:		r = R_PPC64_PCREL34;
      break;
    case BFD_RELOC_PPC64_GOT_PCREL34:		r = R_PPC64_GOT_PCREL34;
      break;
    case BFD_RELOC_PPC64_PLT_PCREL34:		r = R_PPC64_PLT_PCREL34;
      break;
    case BFD_RELOC_PPC64_TPREL34:		r = R_PPC64_TPREL34;
      break;
    case BFD_RELOC_PPC64_DTPREL34:		r = R_PPC64_DTPREL34;
      break;
    case BFD_RELOC_PPC64_GOT_TLSGD_PCREL34:	r = R_PPC64_GOT_TLSGD_PCREL34;
      break;
    case BFD_RELOC_PPC64_GOT_TLSLD_PCREL34:	r = R_PPC64_GOT_TLSLD_PCREL34;
      break;
    case BFD_RELOC_PPC64_GOT_TPREL_PCREL34:	r = R_PPC64_GOT_TPREL_PCREL34;
      break;
    case BFD_RELOC_PPC64_GOT_DTPREL_PCREL34:	r = R_PPC64_GOT_DTPREL_PCREL34;
      break;
    case BFD_RELOC_PPC64_ADDR16_HIGHER34:	r = R_PPC64_ADDR16_HIGHER34;
      break;
    case BFD_RELOC_PPC64_ADDR16_HIGHERA34:	r = R_PPC64_ADDR16_HIGHERA34;
      break;
    case BFD_RELOC_PPC64_ADDR16_HIGHEST34:	r = R_PPC64_ADDR16_HIGHEST34;
      break;
    case BFD_RELOC_PPC64_ADDR16_HIGHESTA34:	r = R_PPC64_ADDR16_HIGHESTA34;
      break;
    case BFD_RELOC_PPC64_REL16_HIGHER34:	r = R_PPC64_REL16_HIGHER34;
      break;
    case BFD_RELOC_PPC64_REL16_HIGHERA34:	r = R_PPC64_REL16_HIGHERA34;
      break;
    case BFD_RELOC_PPC64_REL16_HIGHEST34:	r = R_PPC64_REL16_HIGHEST34;
      break;
    case BFD_RELOC_PPC64_REL16_HIGHESTA34:	r = R_PPC64_REL16_HIGHESTA34;
      break;
    case BFD_RELOC_PPC64_D28:			r = R_PPC64_D28;
      break;
    case BFD_RELOC_PPC64_PCREL28:		r = R_PPC64_PCREL28;
      break;
    case BFD_RELOC_VTABLE_INHERIT:		r = R_PPC64_GNU_VTINHERIT;
      break;
    case BFD_RELOC_VTABLE_ENTRY:		r = R_PPC64_GNU_VTENTRY;
      break;
    }

  return ppc64_elf_howto_table[r];
};

static reloc_howto_type *
ppc64_elf_reloc_name_lookup (bfd *abfd, const char *r_name)
{
  unsigned int i;
  static char *compat_map[][2] = {
    { "R_PPC64_GOT_TLSGD34", "R_PPC64_GOT_TLSGD_PCREL34" },
    { "R_PPC64_GOT_TLSLD34", "R_PPC64_GOT_TLSLD_PCREL34" },
    { "R_PPC64_GOT_TPREL34", "R_PPC64_GOT_TPREL_PCREL34" },
    { "R_PPC64_GOT_DTPREL34", "R_PPC64_GOT_DTPREL_PCREL34" }
  };

  for (i = 0; i < ARRAY_SIZE (ppc64_elf_howto_raw); i++)
    if (ppc64_elf_howto_raw[i].name != NULL
	&& strcasecmp (ppc64_elf_howto_raw[i].name, r_name) == 0)
      return &ppc64_elf_howto_raw[i];

  /* Handle old names of relocations in case they were used by
     .reloc directives.
     FIXME: Remove this soon.  Mapping the reloc names is very likely
     completely unnecessary.  */
  for (i = 0; i < ARRAY_SIZE (compat_map); i++)
    if (strcasecmp (compat_map[i][0], r_name) == 0)
      {
	_bfd_error_handler (_("warning: %s should be used rather than %s"),
			    compat_map[i][1], compat_map[i][0]);
	return ppc64_elf_reloc_name_lookup (abfd, compat_map[i][1]);
      }

  return NULL;
}

/* Set the howto pointer for a PowerPC ELF reloc.  */

static bool
ppc64_elf_info_to_howto (bfd *abfd, arelent *cache_ptr,
			 Elf_Internal_Rela *dst)
{
  unsigned int type;

  /* Initialize howto table if needed.  */
  if (!ppc64_elf_howto_table[R_PPC64_ADDR32])
    ppc_howto_init ();

  type = ELF64_R_TYPE (dst->r_info);
  if (type >= ARRAY_SIZE (ppc64_elf_howto_table))
    {
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
			  abfd, type);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }
  cache_ptr->howto = ppc64_elf_howto_table[type];
  if (cache_ptr->howto == NULL || cache_ptr->howto->name == NULL)
    {
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
			  abfd, type);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  return true;
}

/* Handle the R_PPC64_ADDR16_HA and similar relocs.  */

static bfd_reloc_status_type
ppc64_elf_ha_reloc (bfd *abfd, arelent *reloc_entry, asymbol *symbol,
		    void *data, asection *input_section,
		    bfd *output_bfd, char **error_message)
{
  enum elf_ppc64_reloc_type r_type;
  long insn;
  bfd_size_type octets;
  bfd_vma value;

  /* If this is a relocatable link (output_bfd test tells us), just
     call the generic function.  Any adjustment will be done at final
     link time.  */
  if (output_bfd != NULL)
    return bfd_elf_generic_reloc (abfd, reloc_entry, symbol, data,
				  input_section, output_bfd, error_message);

  /* Adjust the addend for sign extension of the low 16 (or 34) bits.
     We won't actually be using the low bits, so trashing them
     doesn't matter.  */
  r_type = reloc_entry->howto->type;
  if (r_type == R_PPC64_ADDR16_HIGHERA34
      || r_type == R_PPC64_ADDR16_HIGHESTA34
      || r_type == R_PPC64_REL16_HIGHERA34
      || r_type == R_PPC64_REL16_HIGHESTA34)
    reloc_entry->addend += 1ULL << 33;
  else
    reloc_entry->addend += 1U << 15;
  if (r_type != R_PPC64_REL16DX_HA)
    return bfd_reloc_continue;

  value = 0;
  if (!bfd_is_com_section (symbol->section))
    value = symbol->value;
  value += (reloc_entry->addend
	    + symbol->section->output_offset
	    + symbol->section->output_section->vma);
  value -= (reloc_entry->address
	    + input_section->output_offset
	    + input_section->output_section->vma);
  value = (bfd_signed_vma) value >> 16;

  octets = reloc_entry->address * OCTETS_PER_BYTE (abfd, input_section);
  if (!bfd_reloc_offset_in_range (reloc_entry->howto, abfd,
				  input_section, octets))
    return bfd_reloc_outofrange;

  insn = bfd_get_32 (abfd, (bfd_byte *) data + octets);
  insn &= ~0x1fffc1;
  insn |= (value & 0xffc1) | ((value & 0x3e) << 15);
  bfd_put_32 (abfd, insn, (bfd_byte *) data + octets);
  if (value + 0x8000 > 0xffff)
    return bfd_reloc_overflow;
  return bfd_reloc_ok;
}

static bfd_reloc_status_type
ppc64_elf_branch_reloc (bfd *abfd, arelent *reloc_entry, asymbol *symbol,
			void *data, asection *input_section,
			bfd *output_bfd, char **error_message)
{
  if (output_bfd != NULL)
    return bfd_elf_generic_reloc (abfd, reloc_entry, symbol, data,
				  input_section, output_bfd, error_message);

  if (symbol->section->owner == NULL
      || !is_ppc64_elf (symbol->section->owner))
    return bfd_reloc_continue;

  if (strcmp (symbol->section->name, ".opd") == 0
      && (symbol->section->owner->flags & DYNAMIC) == 0)
    {
      bfd_vma dest = opd_entry_value (symbol->section,
				      symbol->value + reloc_entry->addend,
				      NULL, NULL, false);
      if (dest != (bfd_vma) -1)
	reloc_entry->addend = dest - (symbol->value
				      + symbol->section->output_section->vma
				      + symbol->section->output_offset);
    }
  else
    {
      elf_symbol_type *elfsym = (elf_symbol_type *) symbol;

      if (symbol->section->owner != abfd
	  && abiversion (symbol->section->owner) >= 2)
	{
	  unsigned int i;

	  for (i = 0; i < symbol->section->owner->symcount; ++i)
	    {
	      asymbol *symdef = symbol->section->owner->outsymbols[i];

	      if (strcmp (symdef->name, symbol->name) == 0)
		{
		  elfsym = (elf_symbol_type *) symdef;
		  break;
		}
	    }
	}
      reloc_entry->addend
	+= PPC64_LOCAL_ENTRY_OFFSET (elfsym->internal_elf_sym.st_other);
    }
  return bfd_reloc_continue;
}

static bfd_reloc_status_type
ppc64_elf_brtaken_reloc (bfd *abfd, arelent *reloc_entry, asymbol *symbol,
			 void *data, asection *input_section,
			 bfd *output_bfd, char **error_message)
{
  long insn;
  enum elf_ppc64_reloc_type r_type;
  bfd_size_type octets;
  /* Assume 'at' branch hints.  */
  bool is_isa_v2 = true;

  /* If this is a relocatable link (output_bfd test tells us), just
     call the generic function.  Any adjustment will be done at final
     link time.  */
  if (output_bfd != NULL)
    return bfd_elf_generic_reloc (abfd, reloc_entry, symbol, data,
				  input_section, output_bfd, error_message);

  octets = reloc_entry->address * OCTETS_PER_BYTE (abfd, input_section);
  if (!bfd_reloc_offset_in_range (reloc_entry->howto, abfd,
				  input_section, octets))
    return bfd_reloc_outofrange;

  insn = bfd_get_32 (abfd, (bfd_byte *) data + octets);
  insn &= ~(0x01 << 21);
  r_type = reloc_entry->howto->type;
  if (r_type == R_PPC64_ADDR14_BRTAKEN
      || r_type == R_PPC64_REL14_BRTAKEN)
    insn |= 0x01 << 21; /* 'y' or 't' bit, lowest bit of BO field.  */

  if (is_isa_v2)
    {
      /* Set 'a' bit.  This is 0b00010 in BO field for branch
	 on CR(BI) insns (BO == 001at or 011at), and 0b01000
	 for branch on CTR insns (BO == 1a00t or 1a01t).  */
      if ((insn & (0x14 << 21)) == (0x04 << 21))
	insn |= 0x02 << 21;
      else if ((insn & (0x14 << 21)) == (0x10 << 21))
	insn |= 0x08 << 21;
      else
	goto out;
    }
  else
    {
      bfd_vma target = 0;
      bfd_vma from;

      if (!bfd_is_com_section (symbol->section))
	target = symbol->value;
      target += symbol->section->output_section->vma;
      target += symbol->section->output_offset;
      target += reloc_entry->addend;

      from = (reloc_entry->address
	      + input_section->output_offset
	      + input_section->output_section->vma);

      /* Invert 'y' bit if not the default.  */
      if ((bfd_signed_vma) (target - from) < 0)
	insn ^= 0x01 << 21;
    }
  bfd_put_32 (abfd, insn, (bfd_byte *) data + octets);
 out:
  return ppc64_elf_branch_reloc (abfd, reloc_entry, symbol, data,
				 input_section, output_bfd, error_message);
}

static bfd_reloc_status_type
ppc64_elf_sectoff_reloc (bfd *abfd, arelent *reloc_entry, asymbol *symbol,
			 void *data, asection *input_section,
			 bfd *output_bfd, char **error_message)
{
  /* If this is a relocatable link (output_bfd test tells us), just
     call the generic function.  Any adjustment will be done at final
     link time.  */
  if (output_bfd != NULL)
    return bfd_elf_generic_reloc (abfd, reloc_entry, symbol, data,
				  input_section, output_bfd, error_message);

  /* Subtract the symbol section base address.  */
  reloc_entry->addend -= symbol->section->output_section->vma;
  return bfd_reloc_continue;
}

static bfd_reloc_status_type
ppc64_elf_sectoff_ha_reloc (bfd *abfd, arelent *reloc_entry, asymbol *symbol,
			    void *data, asection *input_section,
			    bfd *output_bfd, char **error_message)
{
  /* If this is a relocatable link (output_bfd test tells us), just
     call the generic function.  Any adjustment will be done at final
     link time.  */
  if (output_bfd != NULL)
    return bfd_elf_generic_reloc (abfd, reloc_entry, symbol, data,
				  input_section, output_bfd, error_message);

  /* Subtract the symbol section base address.  */
  reloc_entry->addend -= symbol->section->output_section->vma;

  /* Adjust the addend for sign extension of the low 16 bits.  */
  reloc_entry->addend += 0x8000;
  return bfd_reloc_continue;
}

static bfd_reloc_status_type
ppc64_elf_toc_reloc (bfd *abfd, arelent *reloc_entry, asymbol *symbol,
		     void *data, asection *input_section,
		     bfd *output_bfd, char **error_message)
{
  bfd_vma TOCstart;

  /* If this is a relocatable link (output_bfd test tells us), just
     call the generic function.  Any adjustment will be done at final
     link time.  */
  if (output_bfd != NULL)
    return bfd_elf_generic_reloc (abfd, reloc_entry, symbol, data,
				  input_section, output_bfd, error_message);

  TOCstart = _bfd_get_gp_value (input_section->output_section->owner);
  if (TOCstart == 0)
    TOCstart = ppc64_elf_set_toc (NULL, input_section->output_section->owner);

  /* Subtract the TOC base address.  */
  reloc_entry->addend -= TOCstart + TOC_BASE_OFF;
  return bfd_reloc_continue;
}

static bfd_reloc_status_type
ppc64_elf_toc_ha_reloc (bfd *abfd, arelent *reloc_entry, asymbol *symbol,
			void *data, asection *input_section,
			bfd *output_bfd, char **error_message)
{
  bfd_vma TOCstart;

  /* If this is a relocatable link (output_bfd test tells us), just
     call the generic function.  Any adjustment will be done at final
     link time.  */
  if (output_bfd != NULL)
    return bfd_elf_generic_reloc (abfd, reloc_entry, symbol, data,
				  input_section, output_bfd, error_message);

  TOCstart = _bfd_get_gp_value (input_section->output_section->owner);
  if (TOCstart == 0)
    TOCstart = ppc64_elf_set_toc (NULL, input_section->output_section->owner);

  /* Subtract the TOC base address.  */
  reloc_entry->addend -= TOCstart + TOC_BASE_OFF;

  /* Adjust the addend for sign extension of the low 16 bits.  */
  reloc_entry->addend += 0x8000;
  return bfd_reloc_continue;
}

static bfd_reloc_status_type
ppc64_elf_toc64_reloc (bfd *abfd, arelent *reloc_entry, asymbol *symbol,
		       void *data, asection *input_section,
		       bfd *output_bfd, char **error_message)
{
  bfd_vma TOCstart;
  bfd_size_type octets;

  /* If this is a relocatable link (output_bfd test tells us), just
     call the generic function.  Any adjustment will be done at final
     link time.  */
  if (output_bfd != NULL)
    return bfd_elf_generic_reloc (abfd, reloc_entry, symbol, data,
				  input_section, output_bfd, error_message);

  octets = reloc_entry->address * OCTETS_PER_BYTE (abfd, input_section);
  if (!bfd_reloc_offset_in_range (reloc_entry->howto, abfd,
				  input_section, octets))
    return bfd_reloc_outofrange;

  TOCstart = _bfd_get_gp_value (input_section->output_section->owner);
  if (TOCstart == 0)
    TOCstart = ppc64_elf_set_toc (NULL, input_section->output_section->owner);

  bfd_put_64 (abfd, TOCstart + TOC_BASE_OFF, (bfd_byte *) data + octets);
  return bfd_reloc_ok;
}

static bfd_reloc_status_type
ppc64_elf_prefix_reloc (bfd *abfd, arelent *reloc_entry, asymbol *symbol,
			void *data, asection *input_section,
			bfd *output_bfd, char **error_message)
{
  uint64_t insn;
  bfd_vma targ;
  bfd_size_type octets;

  if (output_bfd != NULL)
    return bfd_elf_generic_reloc (abfd, reloc_entry, symbol, data,
				  input_section, output_bfd, error_message);

  octets = reloc_entry->address * OCTETS_PER_BYTE (abfd, input_section);
  if (!bfd_reloc_offset_in_range (reloc_entry->howto, abfd,
				  input_section, octets))
    return bfd_reloc_outofrange;

  insn = bfd_get_32 (abfd, (bfd_byte *) data + octets);
  insn <<= 32;
  insn |= bfd_get_32 (abfd, (bfd_byte *) data + octets + 4);

  targ = (symbol->section->output_section->vma
	  + symbol->section->output_offset
	  + reloc_entry->addend);
  if (!bfd_is_com_section (symbol->section))
    targ += symbol->value;
  if (reloc_entry->howto->type == R_PPC64_D34_HA30)
    targ += 1ULL << 33;
  if (reloc_entry->howto->pc_relative)
    {
      bfd_vma from = (reloc_entry->address
		      + input_section->output_offset
		      + input_section->output_section->vma);
      targ -=from;
    }
  targ >>= reloc_entry->howto->rightshift;
  insn &= ~reloc_entry->howto->dst_mask;
  insn |= ((targ << 16) | (targ & 0xffff)) & reloc_entry->howto->dst_mask;
  bfd_put_32 (abfd, insn >> 32, (bfd_byte *) data + octets);
  bfd_put_32 (abfd, insn, (bfd_byte *) data + octets + 4);
  if (reloc_entry->howto->complain_on_overflow == complain_overflow_signed
      && (targ + (1ULL << (reloc_entry->howto->bitsize - 1))
	  >= 1ULL << reloc_entry->howto->bitsize))
    return bfd_reloc_overflow;
  return bfd_reloc_ok;
}

static bfd_reloc_status_type
ppc64_elf_unhandled_reloc (bfd *abfd, arelent *reloc_entry, asymbol *symbol,
			   void *data, asection *input_section,
			   bfd *output_bfd, char **error_message)
{
  /* If this is a relocatable link (output_bfd test tells us), just
     call the generic function.  Any adjustment will be done at final
     link time.  */
  if (output_bfd != NULL)
    return bfd_elf_generic_reloc (abfd, reloc_entry, symbol, data,
				  input_section, output_bfd, error_message);

  if (error_message != NULL)
    *error_message = bfd_asprintf (_("generic linker can't handle %s"),
				   reloc_entry->howto->name);
  return bfd_reloc_dangerous;
}

/* Track GOT entries needed for a given symbol.  We might need more
   than one got entry per symbol.  */
struct got_entry
{
  struct got_entry *next;

  /* The symbol addend that we'll be placing in the GOT.  */
  bfd_vma addend;

  /* Unlike other ELF targets, we use separate GOT entries for the same
     symbol referenced from different input files.  This is to support
     automatic multiple TOC/GOT sections, where the TOC base can vary
     from one input file to another.  After partitioning into TOC groups
     we merge entries within the group.

     Point to the BFD owning this GOT entry.  */
  bfd *owner;

  /* Zero for non-tls entries, or TLS_TLS and one of TLS_GD, TLS_LD,
     TLS_TPREL or TLS_DTPREL for tls entries.  */
  unsigned char tls_type;

  /* Non-zero if got.ent points to real entry.  */
  unsigned char is_indirect;

  /* Reference count until size_dynamic_sections, GOT offset thereafter.  */
  union
  {
    bfd_signed_vma refcount;
    bfd_vma offset;
    struct got_entry *ent;
  } got;
};

/* The same for PLT.  */
struct plt_entry
{
  struct plt_entry *next;

  bfd_vma addend;

  union
  {
    bfd_signed_vma refcount;
    bfd_vma offset;
  } plt;
};

struct ppc64_elf_obj_tdata
{
  struct elf_obj_tdata elf;

  /* Shortcuts to dynamic linker sections.  */
  asection *got;
  asection *relgot;

  /* Used during garbage collection.  We attach global symbols defined
     on removed .opd entries to this section so that the sym is removed.  */
  asection *deleted_section;

  /* TLS local dynamic got entry handling.  Support for multiple GOT
     sections means we potentially need one of these for each input bfd.  */
  struct got_entry tlsld_got;

  /* Nonzero if this bfd has small toc/got relocs, ie. that expect
     the reloc to be in the range -32768 to 32767.  */
  unsigned int has_small_toc_reloc : 1;

  /* Set if toc/got ha relocs detected not using r2, or lo reloc
     instruction not one we handle.  */
  unsigned int unexpected_toc_insn : 1;

  /* Set if PLT/GOT/TOC relocs that can be optimised are present in
     this file.  */
  unsigned int has_optrel : 1;
};

#define ppc64_elf_tdata(bfd) \
  ((struct ppc64_elf_obj_tdata *) (bfd)->tdata.any)

#define ppc64_tlsld_got(bfd) \
  (&ppc64_elf_tdata (bfd)->tlsld_got)

/* Override the generic function because we store some extras.  */

static bool
ppc64_elf_mkobject (bfd *abfd)
{
  return bfd_elf_allocate_object (abfd, sizeof (struct ppc64_elf_obj_tdata),
				  PPC64_ELF_DATA);
}

/* Fix bad default arch selected for a 64 bit input bfd when the
   default is 32 bit.  Also select arch based on apuinfo.  */

static bool
ppc64_elf_object_p (bfd *abfd)
{
  if (!abfd->arch_info->the_default)
    return true;

  if (abfd->arch_info->bits_per_word == 32)
    {
      Elf_Internal_Ehdr *i_ehdr = elf_elfheader (abfd);

      if (i_ehdr->e_ident[EI_CLASS] == ELFCLASS64)
	{
	  /* Relies on arch after 32 bit default being 64 bit default.  */
	  abfd->arch_info = abfd->arch_info->next;
	  BFD_ASSERT (abfd->arch_info->bits_per_word == 64);
	}
    }
  return _bfd_elf_ppc_set_arch (abfd);
}

/* Support for core dump NOTE sections.  */

static bool
ppc64_elf_grok_prstatus (bfd *abfd, Elf_Internal_Note *note)
{
  size_t offset, size;

  if (note->descsz != 504)
    return false;

  /* pr_cursig */
  elf_tdata (abfd)->core->signal = bfd_get_16 (abfd, note->descdata + 12);

  /* pr_pid */
  elf_tdata (abfd)->core->lwpid = bfd_get_32 (abfd, note->descdata + 32);

  /* pr_reg */
  offset = 112;
  size = 384;

  /* Make a ".reg/999" section.  */
  return _bfd_elfcore_make_pseudosection (abfd, ".reg",
					  size, note->descpos + offset);
}

static bool
ppc64_elf_grok_psinfo (bfd *abfd, Elf_Internal_Note *note)
{
  if (note->descsz != 136)
    return false;

  elf_tdata (abfd)->core->pid
    = bfd_get_32 (abfd, note->descdata + 24);
  elf_tdata (abfd)->core->program
    = _bfd_elfcore_strndup (abfd, note->descdata + 40, 16);
  elf_tdata (abfd)->core->command
    = _bfd_elfcore_strndup (abfd, note->descdata + 56, 80);

  return true;
}

static char *
ppc64_elf_write_core_note (bfd *abfd, char *buf, int *bufsiz, int note_type,
			   ...)
{
  switch (note_type)
    {
    default:
      return NULL;

    case NT_PRPSINFO:
      {
	char data[136] ATTRIBUTE_NONSTRING;
	va_list ap;

	va_start (ap, note_type);
	memset (data, 0, sizeof (data));
	strncpy (data + 40, va_arg (ap, const char *), 16);
#if GCC_VERSION == 8000 || GCC_VERSION == 8001
	DIAGNOSTIC_PUSH;
	/* GCC 8.0 and 8.1 warn about 80 equals destination size with
	   -Wstringop-truncation:
	   https://gcc.gnu.org/bugzilla/show_bug.cgi?id=85643
	 */
	DIAGNOSTIC_IGNORE_STRINGOP_TRUNCATION;
#endif
	strncpy (data + 56, va_arg (ap, const char *), 80);
#if GCC_VERSION == 8000 || GCC_VERSION == 8001
	DIAGNOSTIC_POP;
#endif
	va_end (ap);
	return elfcore_write_note (abfd, buf, bufsiz,
				   "CORE", note_type, data, sizeof (data));
      }

    case NT_PRSTATUS:
      {
	char data[504];
	va_list ap;
	long pid;
	int cursig;
	const void *greg;

	va_start (ap, note_type);
	memset (data, 0, 112);
	pid = va_arg (ap, long);
	bfd_put_32 (abfd, pid, data + 32);
	cursig = va_arg (ap, int);
	bfd_put_16 (abfd, cursig, data + 12);
	greg = va_arg (ap, const void *);
	memcpy (data + 112, greg, 384);
	memset (data + 496, 0, 8);
	va_end (ap);
	return elfcore_write_note (abfd, buf, bufsiz,
				   "CORE", note_type, data, sizeof (data));
      }
    }
}

/* Add extra PPC sections.  */

static const struct bfd_elf_special_section ppc64_elf_special_sections[] =
{
  { STRING_COMMA_LEN (".plt"),	  0, SHT_NOBITS,   0 },
  { STRING_COMMA_LEN (".sbss"),	 -2, SHT_NOBITS,   SHF_ALLOC + SHF_WRITE },
  { STRING_COMMA_LEN (".sdata"), -2, SHT_PROGBITS, SHF_ALLOC + SHF_WRITE },
  { STRING_COMMA_LEN (".toc"),	  0, SHT_PROGBITS, SHF_ALLOC + SHF_WRITE },
  { STRING_COMMA_LEN (".toc1"),	  0, SHT_PROGBITS, SHF_ALLOC + SHF_WRITE },
  { STRING_COMMA_LEN (".tocbss"), 0, SHT_NOBITS,   SHF_ALLOC + SHF_WRITE },
  { NULL,		      0,  0, 0,		   0 }
};

enum _ppc64_sec_type {
  sec_normal = 0,
  sec_opd = 1,
  sec_toc = 2,
  sec_stub = 3
};

struct _ppc64_elf_section_data
{
  struct bfd_elf_section_data elf;

  union
  {
    /* An array with one entry for each opd function descriptor,
       and some spares since opd entries may be either 16 or 24 bytes.  */
#define OPD_NDX(OFF) ((OFF) >> 4)
    struct _opd_sec_data
    {
      /* Points to the function code section for local opd entries.  */
      asection **func_sec;

      /* After editing .opd, adjust references to opd local syms.  */
      long *adjust;

      union
      {
	/* A copy of relocs before they are modified for --emit-relocs.  */
	Elf_Internal_Rela *relocs;

	/* Section contents.  */
	bfd_byte *contents;
      } u;
    } opd;

    /* An array for toc sections, indexed by offset/8.  */
    struct _toc_sec_data
    {
      /* Specifies the relocation symbol index used at a given toc offset.  */
      unsigned *symndx;

      /* And the relocation addend.  */
      bfd_vma *add;
    } toc;

    /* Stub debugging.  */
    struct ppc_stub_hash_entry *last_ent;
  } u;

  enum _ppc64_sec_type sec_type:2;

  /* Flag set when small branches are detected.  Used to
     select suitable defaults for the stub group size.  */
  unsigned int has_14bit_branch:1;

  /* Flag set when PLTCALL relocs are detected.  */
  unsigned int has_pltcall:1;

  /* Flag set when section has PLT/GOT/TOC relocations that can be
     optimised.  */
  unsigned int has_optrel:1;
};

#define ppc64_elf_section_data(sec) \
  ((struct _ppc64_elf_section_data *) elf_section_data (sec))

static bool
ppc64_elf_new_section_hook (bfd *abfd, asection *sec)
{
  if (!sec->used_by_bfd)
    {
      struct _ppc64_elf_section_data *sdata;
      size_t amt = sizeof (*sdata);

      sdata = bfd_zalloc (abfd, amt);
      if (sdata == NULL)
	return false;
      sec->used_by_bfd = sdata;
    }

  return _bfd_elf_new_section_hook (abfd, sec);
}

static bool
ppc64_elf_section_flags (const Elf_Internal_Shdr *hdr)
{
  const char *name = hdr->bfd_section->name;

  if (startswith (name, ".sbss")
      || startswith (name, ".sdata"))
    hdr->bfd_section->flags |= SEC_SMALL_DATA;

  return true;
}

static struct _opd_sec_data *
get_opd_info (asection * sec)
{
  if (sec != NULL
      && ppc64_elf_section_data (sec) != NULL
      && ppc64_elf_section_data (sec)->sec_type == sec_opd)
    return &ppc64_elf_section_data (sec)->u.opd;
  return NULL;
}

/* Parameters for the qsort hook.  */
static bool synthetic_relocatable;
static const asection *synthetic_opd;

/* qsort comparison function for ppc64_elf_get_synthetic_symtab.  */

static int
compare_symbols (const void *ap, const void *bp)
{
  const asymbol *a = *(const asymbol **) ap;
  const asymbol *b = *(const asymbol **) bp;

  /* Section symbols first.  */
  if ((a->flags & BSF_SECTION_SYM) && !(b->flags & BSF_SECTION_SYM))
    return -1;
  if (!(a->flags & BSF_SECTION_SYM) && (b->flags & BSF_SECTION_SYM))
    return 1;

  /* then .opd symbols.  */
  if (synthetic_opd != NULL)
    {
      if (strcmp (a->section->name, ".opd") == 0
	  && strcmp (b->section->name, ".opd") != 0)
	return -1;
      if (strcmp (a->section->name, ".opd") != 0
	  && strcmp (b->section->name, ".opd") == 0)
	return 1;
    }

  /* then other code symbols.  */
  if (((a->section->flags & (SEC_CODE | SEC_ALLOC | SEC_THREAD_LOCAL))
       == (SEC_CODE | SEC_ALLOC))
      && ((b->section->flags & (SEC_CODE | SEC_ALLOC | SEC_THREAD_LOCAL))
	  != (SEC_CODE | SEC_ALLOC)))
    return -1;

  if (((a->section->flags & (SEC_CODE | SEC_ALLOC | SEC_THREAD_LOCAL))
       != (SEC_CODE | SEC_ALLOC))
      && ((b->section->flags & (SEC_CODE | SEC_ALLOC | SEC_THREAD_LOCAL))
	  == (SEC_CODE | SEC_ALLOC)))
    return 1;

  if (synthetic_relocatable)
    {
      if (a->section->id < b->section->id)
	return -1;

      if (a->section->id > b->section->id)
	return 1;
    }

  if (a->value + a->section->vma < b->value + b->section->vma)
    return -1;

  if (a->value + a->section->vma > b->value + b->section->vma)
    return 1;

  /* For syms with the same value, prefer strong dynamic global function
     syms over other syms.  */
  if ((a->flags & BSF_GLOBAL) != 0 && (b->flags & BSF_GLOBAL) == 0)
    return -1;

  if ((a->flags & BSF_GLOBAL) == 0 && (b->flags & BSF_GLOBAL) != 0)
    return 1;

  if ((a->flags & BSF_FUNCTION) != 0 && (b->flags & BSF_FUNCTION) == 0)
    return -1;

  if ((a->flags & BSF_FUNCTION) == 0 && (b->flags & BSF_FUNCTION) != 0)
    return 1;

  if ((a->flags & BSF_WEAK) == 0 && (b->flags & BSF_WEAK) != 0)
    return -1;

  if ((a->flags & BSF_WEAK) != 0 && (b->flags & BSF_WEAK) == 0)
    return 1;

  if ((a->flags & BSF_DYNAMIC) != 0 && (b->flags & BSF_DYNAMIC) == 0)
    return -1;

  if ((a->flags & BSF_DYNAMIC) == 0 && (b->flags & BSF_DYNAMIC) != 0)
    return 1;

  /* Finally, sort on where the symbol is in memory.  The symbols will
     be in at most two malloc'd blocks, one for static syms, one for
     dynamic syms, and we distinguish the two blocks above by testing
     BSF_DYNAMIC.  Since we are sorting the symbol pointers which were
     originally in the same order as the symbols (and we're not
     sorting the symbols themselves), this ensures a stable sort.  */
  if (a < b)
    return -1;
  if (a > b)
    return 1;
  return 0;
}

/* Search SYMS for a symbol of the given VALUE.  */

static asymbol *
sym_exists_at (asymbol **syms, size_t lo, size_t hi, unsigned int id,
	       bfd_vma value)
{
  size_t mid;

  if (id == (unsigned) -1)
    {
      while (lo < hi)
	{
	  mid = (lo + hi) >> 1;
	  if (syms[mid]->value + syms[mid]->section->vma < value)
	    lo = mid + 1;
	  else if (syms[mid]->value + syms[mid]->section->vma > value)
	    hi = mid;
	  else
	    return syms[mid];
	}
    }
  else
    {
      while (lo < hi)
	{
	  mid = (lo + hi) >> 1;
	  if (syms[mid]->section->id < id)
	    lo = mid + 1;
	  else if (syms[mid]->section->id > id)
	    hi = mid;
	  else if (syms[mid]->value < value)
	    lo = mid + 1;
	  else if (syms[mid]->value > value)
	    hi = mid;
	  else
	    return syms[mid];
	}
    }
  return NULL;
}

static bool
section_covers_vma (bfd *abfd ATTRIBUTE_UNUSED, asection *section, void *ptr)
{
  bfd_vma vma = *(bfd_vma *) ptr;
  return ((section->flags & SEC_ALLOC) != 0
	  && section->vma <= vma
	  && vma < section->vma + section->size);
}

/* Create synthetic symbols, effectively restoring "dot-symbol" function
   entry syms.  Also generate @plt symbols for the glink branch table.
   Returns count of synthetic symbols in RET or -1 on error.  */

static long
ppc64_elf_get_synthetic_symtab (bfd *abfd,
				long static_count, asymbol **static_syms,
				long dyn_count, asymbol **dyn_syms,
				asymbol **ret)
{
  asymbol *s;
  size_t i, j, count;
  char *names;
  size_t symcount, codesecsym, codesecsymend, secsymend, opdsymend;
  asection *opd = NULL;
  bool relocatable = (abfd->flags & (EXEC_P | DYNAMIC)) == 0;
  asymbol **syms;
  int abi = abiversion (abfd);

  *ret = NULL;

  if (abi < 2)
    {
      opd = bfd_get_section_by_name (abfd, ".opd");
      if (opd == NULL && abi == 1)
	return 0;
    }

  syms = NULL;
  codesecsym = 0;
  codesecsymend = 0;
  secsymend = 0;
  opdsymend = 0;
  symcount = 0;
  if (opd != NULL)
    {
      symcount = static_count;
      if (!relocatable)
	symcount += dyn_count;
      if (symcount == 0)
	return 0;

      syms = bfd_malloc ((symcount + 1) * sizeof (*syms));
      if (syms == NULL)
	return -1;

      if (!relocatable && static_count != 0 && dyn_count != 0)
	{
	  /* Use both symbol tables.  */
	  memcpy (syms, static_syms, static_count * sizeof (*syms));
	  memcpy (syms + static_count, dyn_syms,
		  (dyn_count + 1) * sizeof (*syms));
	}
      else if (!relocatable && static_count == 0)
	memcpy (syms, dyn_syms, (symcount + 1) * sizeof (*syms));
      else
	memcpy (syms, static_syms, (symcount + 1) * sizeof (*syms));

      /* Trim uninteresting symbols.  Interesting symbols are section,
	 function, and notype symbols.  */
      for (i = 0, j = 0; i < symcount; ++i)
	if ((syms[i]->flags & (BSF_FILE | BSF_OBJECT | BSF_THREAD_LOCAL
			       | BSF_RELC | BSF_SRELC)) == 0)
	  syms[j++] = syms[i];
      symcount = j;

      synthetic_relocatable = relocatable;
      synthetic_opd = opd;
      qsort (syms, symcount, sizeof (*syms), compare_symbols);

      if (!relocatable && symcount > 1)
	{
	  /* Trim duplicate syms, since we may have merged the normal
	     and dynamic symbols.  Actually, we only care about syms
	     that have different values, so trim any with the same
	     value.  Don't consider ifunc and ifunc resolver symbols
	     duplicates however, because GDB wants to know whether a
	     text symbol is an ifunc resolver.  */
	  for (i = 1, j = 1; i < symcount; ++i)
	    {
	      const asymbol *s0 = syms[i - 1];
	      const asymbol *s1 = syms[i];

	      if ((s0->value + s0->section->vma
		   != s1->value + s1->section->vma)
		  || ((s0->flags & BSF_GNU_INDIRECT_FUNCTION)
		      != (s1->flags & BSF_GNU_INDIRECT_FUNCTION)))
		syms[j++] = syms[i];
	    }
	  symcount = j;
	}

      i = 0;
      /* Note that here and in compare_symbols we can't compare opd and
	 sym->section directly.  With separate debug info files, the
	 symbols will be extracted from the debug file while abfd passed
	 to this function is the real binary.  */
      if ((syms[i]->flags & BSF_SECTION_SYM) != 0
	  && strcmp (syms[i]->section->name, ".opd") == 0)
	++i;
      codesecsym = i;

      for (; i < symcount; ++i)
	if (((syms[i]->section->flags & (SEC_CODE | SEC_ALLOC
					 | SEC_THREAD_LOCAL))
	     != (SEC_CODE | SEC_ALLOC))
	    || (syms[i]->flags & BSF_SECTION_SYM) == 0)
	  break;
      codesecsymend = i;

      for (; i < symcount; ++i)
	if ((syms[i]->flags & BSF_SECTION_SYM) == 0)
	  break;
      secsymend = i;

      for (; i < symcount; ++i)
	if (strcmp (syms[i]->section->name, ".opd") != 0)
	  break;
      opdsymend = i;

      for (; i < symcount; ++i)
	if (((syms[i]->section->flags
	      & (SEC_CODE | SEC_ALLOC | SEC_THREAD_LOCAL)))
	    != (SEC_CODE | SEC_ALLOC))
	  break;
      symcount = i;
    }
  count = 0;

  if (relocatable)
    {
      bool (*slurp_relocs) (bfd *, asection *, asymbol **, bool);
      arelent *r;
      size_t size;
      size_t relcount;

      if (opdsymend == secsymend)
	goto done;

      slurp_relocs = get_elf_backend_data (abfd)->s->slurp_reloc_table;
      relcount = (opd->flags & SEC_RELOC) ? opd->reloc_count : 0;
      if (relcount == 0)
	goto done;

      if (!(*slurp_relocs) (abfd, opd, static_syms, false))
	{
	  count = -1;
	  goto done;
	}

      size = 0;
      for (i = secsymend, r = opd->relocation; i < opdsymend; ++i)
	{
	  asymbol *sym;

	  while (r < opd->relocation + relcount
		 && r->address < syms[i]->value + opd->vma)
	    ++r;

	  if (r == opd->relocation + relcount)
	    break;

	  if (r->address != syms[i]->value + opd->vma)
	    continue;

	  if (r->howto->type != R_PPC64_ADDR64)
	    continue;

	  sym = *r->sym_ptr_ptr;
	  if (!sym_exists_at (syms, opdsymend, symcount,
			      sym->section->id, sym->value + r->addend))
	    {
	      ++count;
	      size += sizeof (asymbol);
	      size += strlen (syms[i]->name) + 2;
	    }
	}

      if (size == 0)
	goto done;
      s = *ret = bfd_malloc (size);
      if (s == NULL)
	{
	  count = -1;
	  goto done;
	}

      names = (char *) (s + count);

      for (i = secsymend, r = opd->relocation; i < opdsymend; ++i)
	{
	  asymbol *sym;

	  while (r < opd->relocation + relcount
		 && r->address < syms[i]->value + opd->vma)
	    ++r;

	  if (r == opd->relocation + relcount)
	    break;

	  if (r->address != syms[i]->value + opd->vma)
	    continue;

	  if (r->howto->type != R_PPC64_ADDR64)
	    continue;

	  sym = *r->sym_ptr_ptr;
	  if (!sym_exists_at (syms, opdsymend, symcount,
			      sym->section->id, sym->value + r->addend))
	    {
	      size_t len;

	      *s = *syms[i];
	      s->flags |= BSF_SYNTHETIC;
	      s->section = sym->section;
	      s->value = sym->value + r->addend;
	      s->name = names;
	      *names++ = '.';
	      len = strlen (syms[i]->name);
	      memcpy (names, syms[i]->name, len + 1);
	      names += len + 1;
	      /* Have udata.p point back to the original symbol this
		 synthetic symbol was derived from.  */
	      s->udata.p = syms[i];
	      s++;
	    }
	}
    }
  else
    {
      bool (*slurp_relocs) (bfd *, asection *, asymbol **, bool);
      bfd_byte *contents = NULL;
      size_t size;
      size_t plt_count = 0;
      bfd_vma glink_vma = 0, resolv_vma = 0;
      asection *dynamic, *glink = NULL, *relplt = NULL;
      arelent *p;

      if (opd != NULL
	  && ((opd->flags & SEC_HAS_CONTENTS) == 0
	      || !bfd_malloc_and_get_section (abfd, opd, &contents)))
	{
	free_contents_and_exit_err:
	  count = -1;
	free_contents_and_exit:
	  free (contents);
	  goto done;
	}

      size = 0;
      for (i = secsymend; i < opdsymend; ++i)
	{
	  bfd_vma ent;

	  /* Ignore bogus symbols.  */
	  if (syms[i]->value > opd->size - 8)
	    continue;

	  ent = bfd_get_64 (abfd, contents + syms[i]->value);
	  if (!sym_exists_at (syms, opdsymend, symcount, -1, ent))
	    {
	      ++count;
	      size += sizeof (asymbol);
	      size += strlen (syms[i]->name) + 2;
	    }
	}

      /* Get start of .glink stubs from DT_PPC64_GLINK.  */
      if (dyn_count != 0
	  && (dynamic = bfd_get_section_by_name (abfd, ".dynamic")) != NULL)
	{
	  bfd_byte *dynbuf, *extdyn, *extdynend;
	  size_t extdynsize;
	  void (*swap_dyn_in) (bfd *, const void *, Elf_Internal_Dyn *);

	  if ((dynamic->flags & SEC_HAS_CONTENTS) == 0
	      || !bfd_malloc_and_get_section (abfd, dynamic, &dynbuf))
	    goto free_contents_and_exit_err;

	  extdynsize = get_elf_backend_data (abfd)->s->sizeof_dyn;
	  swap_dyn_in = get_elf_backend_data (abfd)->s->swap_dyn_in;

	  for (extdyn = dynbuf, extdynend = dynbuf + dynamic->size;
	       (size_t) (extdynend - extdyn) >= extdynsize;
	       extdyn += extdynsize)
	    {
	      Elf_Internal_Dyn dyn;
	      (*swap_dyn_in) (abfd, extdyn, &dyn);

	      if (dyn.d_tag == DT_NULL)
		break;

	      if (dyn.d_tag == DT_PPC64_GLINK)
		{
		  /* The first glink stub starts at DT_PPC64_GLINK plus 32.
		     See comment in ppc64_elf_finish_dynamic_sections. */
		  glink_vma = dyn.d_un.d_val + 8 * 4;
		  /* The .glink section usually does not survive the final
		     link; search for the section (usually .text) where the
		     glink stubs now reside.  */
		  glink = bfd_sections_find_if (abfd, section_covers_vma,
						&glink_vma);
		  break;
		}
	    }

	  free (dynbuf);
	}

      if (glink != NULL)
	{
	  /* Determine __glink trampoline by reading the relative branch
	     from the first glink stub.  */
	  bfd_byte buf[4];
	  unsigned int off = 0;

	  while (bfd_get_section_contents (abfd, glink, buf,
					   glink_vma + off - glink->vma, 4))
	    {
	      unsigned int insn = bfd_get_32 (abfd, buf);
	      insn ^= B_DOT;
	      if ((insn & ~0x3fffffc) == 0)
		{
		  resolv_vma
		    = glink_vma + off + (insn ^ 0x2000000) - 0x2000000;
		  break;
		}
	      off += 4;
	      if (off > 4)
		break;
	    }

	  if (resolv_vma)
	    size += sizeof (asymbol) + sizeof ("__glink_PLTresolve");

	  relplt = bfd_get_section_by_name (abfd, ".rela.plt");
	  if (relplt != NULL)
	    {
	      slurp_relocs = get_elf_backend_data (abfd)->s->slurp_reloc_table;
	      if (!(*slurp_relocs) (abfd, relplt, dyn_syms, true))
		goto free_contents_and_exit_err;

	      plt_count = NUM_SHDR_ENTRIES (&elf_section_data (relplt)->this_hdr);
	      size += plt_count * sizeof (asymbol);

	      p = relplt->relocation;
	      for (i = 0; i < plt_count; i++, p++)
		{
		  size += strlen ((*p->sym_ptr_ptr)->name) + sizeof ("@plt");
		  if (p->addend != 0)
		    size += sizeof ("+0x") - 1 + 16;
		}
	    }
	}

      if (size == 0)
	goto free_contents_and_exit;
      s = *ret = bfd_malloc (size);
      if (s == NULL)
	goto free_contents_and_exit_err;

      names = (char *) (s + count + plt_count + (resolv_vma != 0));

      for (i = secsymend; i < opdsymend; ++i)
	{
	  bfd_vma ent;

	  if (syms[i]->value > opd->size - 8)
	    continue;

	  ent = bfd_get_64 (abfd, contents + syms[i]->value);
	  if (!sym_exists_at (syms, opdsymend, symcount, -1, ent))
	    {
	      size_t lo, hi;
	      size_t len;
	      asection *sec = abfd->sections;

	      *s = *syms[i];
	      lo = codesecsym;
	      hi = codesecsymend;
	      while (lo < hi)
		{
		  size_t mid = (lo + hi) >> 1;
		  if (syms[mid]->section->vma < ent)
		    lo = mid + 1;
		  else if (syms[mid]->section->vma > ent)
		    hi = mid;
		  else
		    {
		      sec = syms[mid]->section;
		      break;
		    }
		}

	      if (lo >= hi && lo > codesecsym)
		sec = syms[lo - 1]->section;

	      for (; sec != NULL; sec = sec->next)
		{
		  if (sec->vma > ent)
		    break;
		  /* SEC_LOAD may not be set if SEC is from a separate debug
		     info file.  */
		  if ((sec->flags & SEC_ALLOC) == 0)
		    break;
		  if ((sec->flags & SEC_CODE) != 0)
		    s->section = sec;
		}
	      s->flags |= BSF_SYNTHETIC;
	      s->value = ent - s->section->vma;
	      s->name = names;
	      *names++ = '.';
	      len = strlen (syms[i]->name);
	      memcpy (names, syms[i]->name, len + 1);
	      names += len + 1;
	      /* Have udata.p point back to the original symbol this
		 synthetic symbol was derived from.  */
	      s->udata.p = syms[i];
	      s++;
	    }
	}
      free (contents);

      if (glink != NULL && relplt != NULL)
	{
	  if (resolv_vma)
	    {
	      /* Add a symbol for the main glink trampoline.  */
	      memset (s, 0, sizeof *s);
	      s->the_bfd = abfd;
	      s->flags = BSF_GLOBAL | BSF_SYNTHETIC;
	      s->section = glink;
	      s->value = resolv_vma - glink->vma;
	      s->name = names;
	      memcpy (names, "__glink_PLTresolve",
		      sizeof ("__glink_PLTresolve"));
	      names += sizeof ("__glink_PLTresolve");
	      s++;
	      count++;
	    }

	  /* FIXME: It would be very much nicer to put sym@plt on the
	     stub rather than on the glink branch table entry.  The
	     objdump disassembler would then use a sensible symbol
	     name on plt calls.  The difficulty in doing so is
	     a) finding the stubs, and,
	     b) matching stubs against plt entries, and,
	     c) there can be multiple stubs for a given plt entry.

	     Solving (a) could be done by code scanning, but older
	     ppc64 binaries used different stubs to current code.
	     (b) is the tricky one since you need to known the toc
	     pointer for at least one function that uses a pic stub to
	     be able to calculate the plt address referenced.
	     (c) means gdb would need to set multiple breakpoints (or
	     find the glink branch itself) when setting breakpoints
	     for pending shared library loads.  */
	  p = relplt->relocation;
	  for (i = 0; i < plt_count; i++, p++)
	    {
	      size_t len;

	      *s = **p->sym_ptr_ptr;
	      /* Undefined syms won't have BSF_LOCAL or BSF_GLOBAL set.  Since
		 we are defining a symbol, ensure one of them is set.  */
	      if ((s->flags & BSF_LOCAL) == 0)
		s->flags |= BSF_GLOBAL;
	      s->flags |= BSF_SYNTHETIC;
	      s->section = glink;
	      s->value = glink_vma - glink->vma;
	      s->name = names;
	      s->udata.p = NULL;
	      len = strlen ((*p->sym_ptr_ptr)->name);
	      memcpy (names, (*p->sym_ptr_ptr)->name, len);
	      names += len;
	      if (p->addend != 0)
		{
		  memcpy (names, "+0x", sizeof ("+0x") - 1);
		  names += sizeof ("+0x") - 1;
		  bfd_sprintf_vma (abfd, names, p->addend);
		  names += strlen (names);
		}
	      memcpy (names, "@plt", sizeof ("@plt"));
	      names += sizeof ("@plt");
	      s++;
	      if (abi < 2)
		{
		  glink_vma += 8;
		  if (i >= 0x8000)
		    glink_vma += 4;
		}
	      else
		glink_vma += 4;
	    }
	  count += plt_count;
	}
    }

 done:
  free (syms);
  return count;
}

/* The following functions are specific to the ELF linker, while
   functions above are used generally.  Those named ppc64_elf_* are
   called by the main ELF linker code.  They appear in this file more
   or less in the order in which they are called.  eg.
   ppc64_elf_check_relocs is called early in the link process,
   ppc64_elf_finish_dynamic_sections is one of the last functions
   called.

   PowerPC64-ELF uses a similar scheme to PowerPC64-XCOFF in that
   functions have both a function code symbol and a function descriptor
   symbol.  A call to foo in a relocatable object file looks like:

   .		.text
   .	x:
   .		bl	.foo
   .		nop

   The function definition in another object file might be:

   .		.section .opd
   .	foo:	.quad	.foo
   .		.quad	.TOC.@tocbase
   .		.quad	0
   .
   .		.text
   .	.foo:	blr

   When the linker resolves the call during a static link, the branch
   unsurprisingly just goes to .foo and the .opd information is unused.
   If the function definition is in a shared library, things are a little
   different:  The call goes via a plt call stub, the opd information gets
   copied to the plt, and the linker patches the nop.

   .	x:
   .		bl	.foo_stub
   .		ld	2,40(1)
   .
   .
   .	.foo_stub:
   .		std	2,40(1)			# in practice, the call stub
   .		addis	11,2,Lfoo@toc@ha	# is slightly optimized, but
   .		addi	11,11,Lfoo@toc@l	# this is the general idea
   .		ld	12,0(11)
   .		ld	2,8(11)
   .		mtctr	12
   .		ld	11,16(11)
   .		bctr
   .
   .		.section .plt
   .	Lfoo:	reloc (R_PPC64_JMP_SLOT, foo)

   The "reloc ()" notation is supposed to indicate that the linker emits
   an R_PPC64_JMP_SLOT reloc against foo.  The dynamic linker does the opd
   copying.

   What are the difficulties here?  Well, firstly, the relocations
   examined by the linker in check_relocs are against the function code
   sym .foo, while the dynamic relocation in the plt is emitted against
   the function descriptor symbol, foo.  Somewhere along the line, we need
   to carefully copy dynamic link information from one symbol to the other.
   Secondly, the generic part of the elf linker will make .foo a dynamic
   symbol as is normal for most other backends.  We need foo dynamic
   instead, at least for an application final link.  However, when
   creating a shared library containing foo, we need to have both symbols
   dynamic so that references to .foo are satisfied during the early
   stages of linking.  Otherwise the linker might decide to pull in a
   definition from some other object, eg. a static library.

   Update: As of August 2004, we support a new convention.  Function
   calls may use the function descriptor symbol, ie. "bl foo".  This
   behaves exactly as "bl .foo".  */

/* Of those relocs that might be copied as dynamic relocs, this
   function selects those that must be copied when linking a shared
   library or PIE, even when the symbol is local.  */

static int
must_be_dyn_reloc (struct bfd_link_info *info,
		   enum elf_ppc64_reloc_type r_type)
{
  switch (r_type)
    {
    default:
      /* Only relative relocs can be resolved when the object load
	 address isn't fixed.  DTPREL64 is excluded because the
	 dynamic linker needs to differentiate global dynamic from
	 local dynamic __tls_index pairs when PPC64_OPT_TLS is set.  */
      return 1;

    case R_PPC64_REL32:
    case R_PPC64_REL64:
    case R_PPC64_REL30:
    case R_PPC64_TOC16:
    case R_PPC64_TOC16_DS:
    case R_PPC64_TOC16_LO:
    case R_PPC64_TOC16_HI:
    case R_PPC64_TOC16_HA:
    case R_PPC64_TOC16_LO_DS:
      return 0;

    case R_PPC64_TPREL16:
    case R_PPC64_TPREL16_LO:
    case R_PPC64_TPREL16_HI:
    case R_PPC64_TPREL16_HA:
    case R_PPC64_TPREL16_DS:
    case R_PPC64_TPREL16_LO_DS:
    case R_PPC64_TPREL16_HIGH:
    case R_PPC64_TPREL16_HIGHA:
    case R_PPC64_TPREL16_HIGHER:
    case R_PPC64_TPREL16_HIGHERA:
    case R_PPC64_TPREL16_HIGHEST:
    case R_PPC64_TPREL16_HIGHESTA:
    case R_PPC64_TPREL64:
    case R_PPC64_TPREL34:
      /* These relocations are relative but in a shared library the
	 linker doesn't know the thread pointer base.  */
      return bfd_link_dll (info);
    }
}

/* If ELIMINATE_COPY_RELOCS is non-zero, the linker will try to avoid
   copying dynamic variables from a shared lib into an app's .dynbss
   section, and instead use a dynamic relocation to point into the
   shared lib.  With code that gcc generates it is vital that this be
   enabled;  In the PowerPC64 ELFv1 ABI the address of a function is
   actually the address of a function descriptor which resides in the
   .opd section.  gcc uses the descriptor directly rather than going
   via the GOT as some other ABIs do, which means that initialized
   function pointers reference the descriptor.  Thus, a function
   pointer initialized to the address of a function in a shared
   library will either require a .dynbss copy and a copy reloc, or a
   dynamic reloc.  Using a .dynbss copy redefines the function
   descriptor symbol to point to the copy.  This presents a problem as
   a PLT entry for that function is also initialized from the function
   descriptor symbol and the copy may not be initialized first.  */
#define ELIMINATE_COPY_RELOCS 1

/* Section name for stubs is the associated section name plus this
   string.  */
#define STUB_SUFFIX ".stub"

/* Linker stubs.
   ppc_stub_long_branch:
   Used when a 14 bit branch (or even a 24 bit branch) can't reach its
   destination, but a 24 bit branch in a stub section will reach.
   .	b	dest

   ppc_stub_plt_branch:
   Similar to the above, but a 24 bit branch in the stub section won't
   reach its destination.
   .	addis	%r12,%r2,xxx@toc@ha
   .	ld	%r12,xxx@toc@l(%r12)
   .	mtctr	%r12
   .	bctr

   ppc_stub_plt_call:
   Used to call a function in a shared library.  If it so happens that
   the plt entry referenced crosses a 64k boundary, then an extra
   "addi %r11,%r11,xxx@toc@l" will be inserted before the "mtctr".
   An r2save variant starts with "std %r2,40(%r1)".
   .	addis	%r11,%r2,xxx@toc@ha
   .	ld	%r12,xxx+0@toc@l(%r11)
   .	mtctr	%r12
   .	ld	%r2,xxx+8@toc@l(%r11)
   .	ld	%r11,xxx+16@toc@l(%r11)
   .	bctr

   ppc_stub_long_branch and ppc_stub_plt_branch may also have additional
   code to adjust the value and save r2 to support multiple toc sections.
   A ppc_stub_long_branch with an r2 offset looks like:
   .	std	%r2,40(%r1)
   .	addis	%r2,%r2,off@ha
   .	addi	%r2,%r2,off@l
   .	b	dest

   A ppc_stub_plt_branch with an r2 offset looks like:
   .	std	%r2,40(%r1)
   .	addis	%r12,%r2,xxx@toc@ha
   .	ld	%r12,xxx@toc@l(%r12)
   .	addis	%r2,%r2,off@ha
   .	addi	%r2,%r2,off@l
   .	mtctr	%r12
   .	bctr

   All of the above stubs are shown as their ELFv1 variants.  ELFv2
   variants exist too, simpler for plt calls since a new toc pointer
   and static chain are not loaded by the stub.  In addition, ELFv2
   has some more complex stubs to handle calls marked with NOTOC
   relocs from functions where r2 is not a valid toc pointer.
   ppc_stub_long_branch_p9notoc:
   .	mflr	%r12
   .	bcl	20,31,1f
   .  1:
   .	mflr	%r11
   .	mtlr	%r12
   .	addis	%r12,%r11,dest-1b@ha
   .	addi	%r12,%r12,dest-1b@l
   .	b	dest

   ppc_stub_plt_branch_p9notoc:
   .	mflr	%r12
   .	bcl	20,31,1f
   .  1:
   .	mflr	%r11
   .	mtlr	%r12
   .	lis	%r12,xxx-1b@highest
   .	ori	%r12,%r12,xxx-1b@higher
   .	sldi	%r12,%r12,32
   .	oris	%r12,%r12,xxx-1b@high
   .	ori	%r12,%r12,xxx-1b@l
   .	add	%r12,%r11,%r12
   .	mtctr	%r12
   .	bctr

   ppc_stub_plt_call_p9notoc:
   .	mflr	%r12
   .	bcl	20,31,1f
   .  1:
   .	mflr	%r11
   .	mtlr	%r12
   .	lis	%r12,xxx-1b@highest
   .	ori	%r12,%r12,xxx-1b@higher
   .	sldi	%r12,%r12,32
   .	oris	%r12,%r12,xxx-1b@high
   .	ori	%r12,%r12,xxx-1b@l
   .	ldx	%r12,%r11,%r12
   .	mtctr	%r12
   .	bctr

   There are also ELFv1 power10 variants of these stubs.
   ppc_stub_long_branch_notoc:
   .	pla	%r12,dest@pcrel
   .	b	dest
   ppc_stub_plt_branch_notoc:
   .	lis	%r11,(dest-1f)@highesta34
   .	ori	%r11,%r11,(dest-1f)@highera34
   .	sldi	%r11,%r11,34
   . 1: pla	%r12,dest@pcrel
   .	add	%r12,%r11,%r12
   .	mtctr	%r12
   .	bctr
   ppc_stub_plt_call_notoc:
   .	lis	%r11,(xxx-1f)@highesta34
   .	ori	%r11,%r11,(xxx-1f)@highera34
   .	sldi	%r11,%r11,34
   . 1: pla	%r12,xxx@pcrel
   .	ldx	%r12,%r11,%r12
   .	mtctr	%r12
   .	bctr

   In cases where the high instructions would add zero, they are
   omitted and following instructions modified in some cases.
   For example, a power10 ppc_stub_plt_call_notoc might simplify down
   to
   .	pld	%r12,xxx@pcrel
   .	mtctr	%r12
   .	bctr

   Stub variants may be merged.  For example, if printf is called from
   code with the tocsave optimization (ie. r2 saved in function
   prologue) and therefore calls use a ppc_stub_plt_call linkage stub,
   and from other code without the tocsave optimization requiring a
   ppc_stub_plt_call_r2save linkage stub, a single stub of the latter
   type will be created.  Calls with the tocsave optimization will
   enter this stub after the instruction saving r2.  A similar
   situation exists when calls are marked with R_PPC64_REL24_NOTOC
   relocations.  These require a ppc_stub_plt_call_notoc linkage stub
   to call an external function like printf.  If other calls to printf
   require a ppc_stub_plt_call linkage stub then a single
   ppc_stub_plt_call_notoc linkage stub may be used for both types of
   call.  */

enum ppc_stub_main_type
{
  ppc_stub_none,
  ppc_stub_long_branch,
  ppc_stub_plt_branch,
  ppc_stub_plt_call,
  ppc_stub_global_entry,
  ppc_stub_save_res
};

/* ppc_stub_long_branch, ppc_stub_plt_branch and ppc_stub_plt_call have
   these variations.  */

enum ppc_stub_sub_type
{
  ppc_stub_toc,
  ppc_stub_notoc,
  ppc_stub_p9notoc
};

struct ppc_stub_type
{
  ENUM_BITFIELD (ppc_stub_main_type) main : 3;
  ENUM_BITFIELD (ppc_stub_sub_type) sub : 2;
  unsigned int r2save : 1;
};

/* Information on stub grouping.  */
struct map_stub
{
  /* The stub section.  */
  asection *stub_sec;
  /* This is the section to which stubs in the group will be attached.  */
  asection *link_sec;
  /* Next group.  */
  struct map_stub *next;
  /* Whether to emit a copy of register save/restore functions in this
     group.  */
  int needs_save_res;
  /* Current offset within stubs after the insn restoring lr in a
     _notoc or _both stub using bcl for pc-relative addressing, or
     after the insn restoring lr in a __tls_get_addr_opt plt stub.  */
  unsigned int lr_restore;
  /* Accumulated size of EH info emitted to describe return address
     if stubs modify lr.  Does not include 17 byte FDE header.  */
  unsigned int eh_size;
  /* Offset in glink_eh_frame to the start of EH info for this group.  */
  unsigned int eh_base;
};

struct ppc_stub_hash_entry
{
  /* Base hash table entry structure.  */
  struct bfd_hash_entry root;

  struct ppc_stub_type type;

  /* Group information.  */
  struct map_stub *group;

  /* Offset within stub_sec of the beginning of this stub.  */
  bfd_vma stub_offset;

  /* Given the symbol's value and its section we can determine its final
     value when building the stubs (so the stub knows where to jump.  */
  bfd_vma target_value;
  asection *target_section;

  /* The symbol table entry, if any, that this was derived from.  */
  struct ppc_link_hash_entry *h;
  struct plt_entry *plt_ent;

  /* Symbol type.  */
  unsigned char symtype;

  /* Symbol st_other.  */
  unsigned char other;

  /* Debug: Track hash table traversal.  */
  unsigned int id;
};

struct ppc_branch_hash_entry
{
  /* Base hash table entry structure.  */
  struct bfd_hash_entry root;

  /* Offset within branch lookup table.  */
  unsigned int offset;

  /* Generation marker.  */
  unsigned int iter;
};

/* Used to track dynamic relocations.  */
struct ppc_dyn_relocs
{
  struct ppc_dyn_relocs *next;

  /* The input section of the reloc.  */
  asection *sec;

  /* Total number of relocs copied for the input section.  */
  unsigned int count;

  /* Number of pc-relative relocs copied for the input section.  */
  unsigned int pc_count;

  /* Number of relocs that might become R_PPC64_RELATIVE.  */
  unsigned int rel_count;
};

struct ppc_local_dyn_relocs
{
  struct ppc_local_dyn_relocs *next;

  /* The input section of the reloc.  */
  asection *sec;

  /* Total number of relocs copied for the input section.  */
  unsigned int count;

  /* Number of relocs that might become R_PPC64_RELATIVE.  */
  unsigned int rel_count : 31;

  /* Whether this entry is for STT_GNU_IFUNC symbols.  */
  unsigned int ifunc : 1;
};

struct ppc_link_hash_entry
{
  struct elf_link_hash_entry elf;

  union
  {
    /* A pointer to the most recently used stub hash entry against this
       symbol.  */
    struct ppc_stub_hash_entry *stub_cache;

    /* A pointer to the next symbol starting with a '.'  */
    struct ppc_link_hash_entry *next_dot_sym;
  } u;

  /* Link between function code and descriptor symbols.  */
  struct ppc_link_hash_entry *oh;

  /* Flag function code and descriptor symbols.  */
  unsigned int is_func:1;
  unsigned int is_func_descriptor:1;
  unsigned int fake:1;

  /* Whether global opd/toc sym has been adjusted or not.
     After ppc64_elf_edit_opd/ppc64_elf_edit_toc has run, this flag
     should be set for all globals defined in any opd/toc section.  */
  unsigned int adjust_done:1;

  /* Set if this is an out-of-line register save/restore function,
     with non-standard calling convention.  */
  unsigned int save_res:1;

  /* Set if a duplicate symbol with non-zero localentry is detected,
     even when the duplicate symbol does not provide a definition.  */
  unsigned int non_zero_localentry:1;

  /* Contexts in which symbol is used in the GOT (or TOC).
     Bits are or'd into the mask as the corresponding relocs are
     encountered during check_relocs, with TLS_TLS being set when any
     of the other TLS bits are set.  tls_optimize clears bits when
     optimizing to indicate the corresponding GOT entry type is not
     needed.  If set, TLS_TLS is never cleared.  tls_optimize may also
     set TLS_GDIE when a GD reloc turns into an IE one.
     These flags are also kept for local symbols.  */
#define TLS_TLS		 1	/* Any TLS reloc.  */
#define TLS_GD		 2	/* GD reloc. */
#define TLS_LD		 4	/* LD reloc. */
#define TLS_TPREL	 8	/* TPREL reloc, => IE. */
#define TLS_DTPREL	16	/* DTPREL reloc, => LD. */
#define TLS_MARK	32	/* __tls_get_addr call marked. */
#define TLS_GDIE	64	/* GOT TPREL reloc resulting from GD->IE. */
#define TLS_EXPLICIT   256	/* TOC section TLS reloc, not stored. */
  unsigned char tls_mask;

  /* The above field is also used to mark function symbols.  In which
     case TLS_TLS will be 0.  */
#define PLT_IFUNC	 2	/* STT_GNU_IFUNC.  */
#define PLT_KEEP	 4	/* inline plt call requires plt entry.  */
#define NON_GOT        256	/* local symbol plt, not stored.  */
};

static inline struct ppc_link_hash_entry *
ppc_elf_hash_entry (struct elf_link_hash_entry *ent)
{
  return (struct ppc_link_hash_entry *) ent;
}

static inline struct elf_link_hash_entry *
elf_hash_entry (struct ppc_link_hash_entry *ent)
{
  return (struct elf_link_hash_entry *) ent;
}

/* ppc64 ELF linker hash table.  */

struct ppc_link_hash_table
{
  struct elf_link_hash_table elf;

  /* The stub hash table.  */
  struct bfd_hash_table stub_hash_table;

  /* Another hash table for plt_branch stubs.  */
  struct bfd_hash_table branch_hash_table;

  /* Hash table for function prologue tocsave.  */
  htab_t tocsave_htab;

  /* Various options and other info passed from the linker.  */
  struct ppc64_elf_params *params;

  /* The size of sec_info below.  */
  unsigned int sec_info_arr_size;

  /* Per-section array of extra section info.  Done this way rather
     than as part of ppc64_elf_section_data so we have the info for
     non-ppc64 sections.  */
  struct
  {
    /* Along with elf_gp, specifies the TOC pointer used by this section.  */
    bfd_vma toc_off;

    union
    {
      /* The section group that this section belongs to.  */
      struct map_stub *group;
      /* A temp section list pointer.  */
      asection *list;
    } u;
  } *sec_info;

  /* Linked list of groups.  */
  struct map_stub *group;

  /* Temp used when calculating TOC pointers.  */
  bfd_vma toc_curr;
  bfd *toc_bfd;
  asection *toc_first_sec;

  /* Used when adding symbols.  */
  struct ppc_link_hash_entry *dot_syms;

  /* Shortcuts to get to dynamic linker sections.  */
  asection *glink;
  asection *global_entry;
  asection *sfpr;
  asection *pltlocal;
  asection *relpltlocal;
  asection *brlt;
  asection *relbrlt;
  asection *glink_eh_frame;

  /* Shortcut to .__tls_get_addr and __tls_get_addr.  */
  struct ppc_link_hash_entry *tls_get_addr;
  struct ppc_link_hash_entry *tls_get_addr_fd;
  struct ppc_link_hash_entry *tga_desc;
  struct ppc_link_hash_entry *tga_desc_fd;
  struct map_stub *tga_group;

  /* The size of reliplt used by got entry relocs.  */
  bfd_size_type got_reli_size;

  /* DT_RELR array of section/r_offset.  */
  size_t relr_alloc;
  size_t relr_count;
  struct
  {
    asection *sec;
    bfd_vma off;
  } *relr;

  /* Statistics.  */
  unsigned long stub_count[ppc_stub_save_res];

  /* Number of stubs against global syms.  */
  unsigned long stub_globals;

  /* Set if we're linking code with function descriptors.  */
  unsigned int opd_abi:1;

  /* Support for multiple toc sections.  */
  unsigned int do_multi_toc:1;
  unsigned int multi_toc_needed:1;
  unsigned int second_toc_pass:1;
  unsigned int do_toc_opt:1;

  /* Set if tls optimization is enabled.  */
  unsigned int do_tls_opt:1;

  /* Set if inline plt calls should be converted to direct calls.  */
  unsigned int can_convert_all_inline_plt:1;

  /* Set if a stub_offset changed.  */
  unsigned int stub_changed:1;

  /* Set on error.  */
  unsigned int stub_error:1;

  /* Whether func_desc_adjust needs to be run over symbols.  */
  unsigned int need_func_desc_adj:1;

  /* Whether plt calls for ELFv2 localentry:0 funcs have been optimized.  */
  unsigned int has_plt_localentry0:1;

  /* Whether calls are made via the PLT from NOTOC functions.  */
  unsigned int notoc_plt:1;

  /* Whether any code linked seems to be Power10.  */
  unsigned int has_power10_relocs:1;

  /* Incremented once for each stub sized.  */
  unsigned int stub_id;

  /* Incremented every time we size stubs.  */
  unsigned int stub_iteration;

/* After 20 iterations of stub sizing we no longer allow stubs to
   shrink.  This is to break out of a pathological case where adding
   stubs or increasing their size on one iteration decreases section
   gaps (perhaps due to alignment), which then results in smaller
   stubs on the next iteration.  */
#define STUB_SHRINK_ITER 20
};

/* Rename some of the generic section flags to better document how they
   are used here.  */

/* Nonzero if this section has TLS related relocations.  */
#define has_tls_reloc sec_flg0

/* Nonzero if this section has a call to __tls_get_addr lacking marker
   relocations.  */
#define nomark_tls_get_addr sec_flg1

/* Nonzero if this section has any toc or got relocs.  */
#define has_toc_reloc sec_flg2

/* Nonzero if this section has a call to another section that uses
   the toc or got.  */
#define makes_toc_func_call sec_flg3

/* Recursion protection when determining above flag.  */
#define call_check_in_progress sec_flg4
#define call_check_done sec_flg5

/* Get the ppc64 ELF linker hash table from a link_info structure.  */

#define ppc_hash_table(p) \
  ((is_elf_hash_table ((p)->hash)					\
    && elf_hash_table_id (elf_hash_table (p)) == PPC64_ELF_DATA)	\
   ? (struct ppc_link_hash_table *) (p)->hash : NULL)

#define ppc_stub_hash_lookup(table, string, create, copy) \
  ((struct ppc_stub_hash_entry *) \
   bfd_hash_lookup ((table), (string), (create), (copy)))

#define ppc_branch_hash_lookup(table, string, create, copy) \
  ((struct ppc_branch_hash_entry *) \
   bfd_hash_lookup ((table), (string), (create), (copy)))

/* Create an entry in the stub hash table.  */

static struct bfd_hash_entry *
stub_hash_newfunc (struct bfd_hash_entry *entry,
		   struct bfd_hash_table *table,
		   const char *string)
{
  /* Allocate the structure if it has not already been allocated by a
     subclass.  */
  if (entry == NULL)
    {
      entry = bfd_hash_allocate (table, sizeof (struct ppc_stub_hash_entry));
      if (entry == NULL)
	return entry;
    }

  /* Call the allocation method of the superclass.  */
  entry = bfd_hash_newfunc (entry, table, string);
  if (entry != NULL)
    {
      struct ppc_stub_hash_entry *eh;

      /* Initialize the local fields.  */
      eh = (struct ppc_stub_hash_entry *) entry;
      eh->type.main = ppc_stub_none;
      eh->type.sub = ppc_stub_toc;
      eh->type.r2save = 0;
      eh->group = NULL;
      eh->stub_offset = 0;
      eh->target_value = 0;
      eh->target_section = NULL;
      eh->h = NULL;
      eh->plt_ent = NULL;
      eh->symtype = 0;
      eh->other = 0;
      eh->id = 0;
    }

  return entry;
}

/* Create an entry in the branch hash table.  */

static struct bfd_hash_entry *
branch_hash_newfunc (struct bfd_hash_entry *entry,
		     struct bfd_hash_table *table,
		     const char *string)
{
  /* Allocate the structure if it has not already been allocated by a
     subclass.  */
  if (entry == NULL)
    {
      entry = bfd_hash_allocate (table, sizeof (struct ppc_branch_hash_entry));
      if (entry == NULL)
	return entry;
    }

  /* Call the allocation method of the superclass.  */
  entry = bfd_hash_newfunc (entry, table, string);
  if (entry != NULL)
    {
      struct ppc_branch_hash_entry *eh;

      /* Initialize the local fields.  */
      eh = (struct ppc_branch_hash_entry *) entry;
      eh->offset = 0;
      eh->iter = 0;
    }

  return entry;
}

/* Create an entry in a ppc64 ELF linker hash table.  */

static struct bfd_hash_entry *
link_hash_newfunc (struct bfd_hash_entry *entry,
		   struct bfd_hash_table *table,
		   const char *string)
{
  /* Allocate the structure if it has not already been allocated by a
     subclass.  */
  if (entry == NULL)
    {
      entry = bfd_hash_allocate (table, sizeof (struct ppc_link_hash_entry));
      if (entry == NULL)
	return entry;
    }

  /* Call the allocation method of the superclass.  */
  entry = _bfd_elf_link_hash_newfunc (entry, table, string);
  if (entry != NULL)
    {
      struct ppc_link_hash_entry *eh = (struct ppc_link_hash_entry *) entry;

      memset (&eh->u.stub_cache, 0,
	      (sizeof (struct ppc_link_hash_entry)
	       - offsetof (struct ppc_link_hash_entry, u.stub_cache)));

      /* When making function calls, old ABI code references function entry
	 points (dot symbols), while new ABI code references the function
	 descriptor symbol.  We need to make any combination of reference and
	 definition work together, without breaking archive linking.

	 For a defined function "foo" and an undefined call to "bar":
	 An old object defines "foo" and ".foo", references ".bar" (possibly
	 "bar" too).
	 A new object defines "foo" and references "bar".

	 A new object thus has no problem with its undefined symbols being
	 satisfied by definitions in an old object.  On the other hand, the
	 old object won't have ".bar" satisfied by a new object.

	 Keep a list of newly added dot-symbols.  */

      if (string[0] == '.')
	{
	  struct ppc_link_hash_table *htab;

	  htab = (struct ppc_link_hash_table *) table;
	  eh->u.next_dot_sym = htab->dot_syms;
	  htab->dot_syms = eh;
	}
    }

  return entry;
}

struct tocsave_entry
{
  asection *sec;
  bfd_vma offset;
};

static hashval_t
tocsave_htab_hash (const void *p)
{
  const struct tocsave_entry *e = (const struct tocsave_entry *) p;
  return ((bfd_vma) (intptr_t) e->sec ^ e->offset) >> 3;
}

static int
tocsave_htab_eq (const void *p1, const void *p2)
{
  const struct tocsave_entry *e1 = (const struct tocsave_entry *) p1;
  const struct tocsave_entry *e2 = (const struct tocsave_entry *) p2;
  return e1->sec == e2->sec && e1->offset == e2->offset;
}

/* Destroy a ppc64 ELF linker hash table.  */

static void
ppc64_elf_link_hash_table_free (bfd *obfd)
{
  struct ppc_link_hash_table *htab;

  htab = (struct ppc_link_hash_table *) obfd->link.hash;
  if (htab->tocsave_htab)
    htab_delete (htab->tocsave_htab);
  bfd_hash_table_free (&htab->branch_hash_table);
  bfd_hash_table_free (&htab->stub_hash_table);
  _bfd_elf_link_hash_table_free (obfd);
}

/* Create a ppc64 ELF linker hash table.  */

static struct bfd_link_hash_table *
ppc64_elf_link_hash_table_create (bfd *abfd)
{
  struct ppc_link_hash_table *htab;
  size_t amt = sizeof (struct ppc_link_hash_table);

  htab = bfd_zmalloc (amt);
  if (htab == NULL)
    return NULL;

  if (!_bfd_elf_link_hash_table_init (&htab->elf, abfd, link_hash_newfunc,
				      sizeof (struct ppc_link_hash_entry),
				      PPC64_ELF_DATA))
    {
      free (htab);
      return NULL;
    }

  /* Init the stub hash table too.  */
  if (!bfd_hash_table_init (&htab->stub_hash_table, stub_hash_newfunc,
			    sizeof (struct ppc_stub_hash_entry)))
    {
      _bfd_elf_link_hash_table_free (abfd);
      return NULL;
    }

  /* And the branch hash table.  */
  if (!bfd_hash_table_init (&htab->branch_hash_table, branch_hash_newfunc,
			    sizeof (struct ppc_branch_hash_entry)))
    {
      bfd_hash_table_free (&htab->stub_hash_table);
      _bfd_elf_link_hash_table_free (abfd);
      return NULL;
    }

  htab->tocsave_htab = htab_try_create (1024,
					tocsave_htab_hash,
					tocsave_htab_eq,
					NULL);
  if (htab->tocsave_htab == NULL)
    {
      ppc64_elf_link_hash_table_free (abfd);
      return NULL;
    }
  htab->elf.root.hash_table_free = ppc64_elf_link_hash_table_free;

  /* Initializing two fields of the union is just cosmetic.  We really
     only care about glist, but when compiled on a 32-bit host the
     bfd_vma fields are larger.  Setting the bfd_vma to zero makes
     debugger inspection of these fields look nicer.  */
  htab->elf.init_got_refcount.refcount = 0;
  htab->elf.init_got_refcount.glist = NULL;
  htab->elf.init_plt_refcount.refcount = 0;
  htab->elf.init_plt_refcount.glist = NULL;
  htab->elf.init_got_offset.offset = 0;
  htab->elf.init_got_offset.glist = NULL;
  htab->elf.init_plt_offset.offset = 0;
  htab->elf.init_plt_offset.glist = NULL;

  return &htab->elf.root;
}

/* Create sections for linker generated code.  */

static bool
create_linkage_sections (bfd *dynobj, struct bfd_link_info *info)
{
  struct ppc_link_hash_table *htab;
  flagword flags;

  htab = ppc_hash_table (info);

  flags = (SEC_ALLOC | SEC_LOAD | SEC_CODE | SEC_READONLY
	   | SEC_HAS_CONTENTS | SEC_IN_MEMORY | SEC_LINKER_CREATED);
  if (htab->params->save_restore_funcs)
    {
      /* Create .sfpr for code to save and restore fp regs.  */
      htab->sfpr = bfd_make_section_anyway_with_flags (dynobj, ".sfpr",
						       flags);
      if (htab->sfpr == NULL
	  || !bfd_set_section_alignment (htab->sfpr, 2))
	return false;
    }

  if (bfd_link_relocatable (info))
    return true;

  /* Create .glink for lazy dynamic linking support.  */
  htab->glink = bfd_make_section_anyway_with_flags (dynobj, ".glink",
						    flags);
  if (htab->glink == NULL
      || !bfd_set_section_alignment (htab->glink, 3))
    return false;

  /* The part of .glink used by global entry stubs, separate so that
     it can be aligned appropriately without affecting htab->glink.  */
  htab->global_entry = bfd_make_section_anyway_with_flags (dynobj, ".glink",
							   flags);
  if (htab->global_entry == NULL
      || !bfd_set_section_alignment (htab->global_entry, 2))
    return false;

  if (!info->no_ld_generated_unwind_info)
    {
      flags = (SEC_ALLOC | SEC_LOAD | SEC_READONLY | SEC_HAS_CONTENTS
	       | SEC_IN_MEMORY | SEC_LINKER_CREATED);
      htab->glink_eh_frame = bfd_make_section_anyway_with_flags (dynobj,
								 ".eh_frame",
								 flags);
      if (htab->glink_eh_frame == NULL
	  || !bfd_set_section_alignment (htab->glink_eh_frame, 2))
	return false;
    }

  flags = SEC_ALLOC | SEC_LINKER_CREATED;
  htab->elf.iplt = bfd_make_section_anyway_with_flags (dynobj, ".iplt", flags);
  if (htab->elf.iplt == NULL
      || !bfd_set_section_alignment (htab->elf.iplt, 3))
    return false;

  flags = (SEC_ALLOC | SEC_LOAD | SEC_READONLY
	   | SEC_HAS_CONTENTS | SEC_IN_MEMORY | SEC_LINKER_CREATED);
  htab->elf.irelplt
    = bfd_make_section_anyway_with_flags (dynobj, ".rela.iplt", flags);
  if (htab->elf.irelplt == NULL
      || !bfd_set_section_alignment (htab->elf.irelplt, 3))
    return false;

  /* Create branch lookup table for plt_branch stubs.  */
  flags = (SEC_ALLOC | SEC_LOAD
	   | SEC_HAS_CONTENTS | SEC_IN_MEMORY | SEC_LINKER_CREATED);
  htab->brlt = bfd_make_section_anyway_with_flags (dynobj, ".branch_lt",
						   flags);
  if (htab->brlt == NULL
      || !bfd_set_section_alignment (htab->brlt, 3))
    return false;

  /* Local plt entries, put in .branch_lt but a separate section for
     convenience.  */
  htab->pltlocal = bfd_make_section_anyway_with_flags (dynobj, ".branch_lt",
						       flags);
  if (htab->pltlocal == NULL
      || !bfd_set_section_alignment (htab->pltlocal, 3))
    return false;

  if (!bfd_link_pic (info))
    return true;

  flags = (SEC_ALLOC | SEC_LOAD | SEC_READONLY
	   | SEC_HAS_CONTENTS | SEC_IN_MEMORY | SEC_LINKER_CREATED);
  htab->relbrlt
    = bfd_make_section_anyway_with_flags (dynobj, ".rela.branch_lt", flags);
  if (htab->relbrlt == NULL
      || !bfd_set_section_alignment (htab->relbrlt, 3))
    return false;

  htab->relpltlocal
    = bfd_make_section_anyway_with_flags (dynobj, ".rela.branch_lt", flags);
  if (htab->relpltlocal == NULL
      || !bfd_set_section_alignment (htab->relpltlocal, 3))
    return false;

  return true;
}

/* Satisfy the ELF linker by filling in some fields in our fake bfd.  */

bool
ppc64_elf_init_stub_bfd (struct bfd_link_info *info,
			 struct ppc64_elf_params *params)
{
  struct ppc_link_hash_table *htab;

  elf_elfheader (params->stub_bfd)->e_ident[EI_CLASS] = ELFCLASS64;

/* Always hook our dynamic sections into the first bfd, which is the
   linker created stub bfd.  This ensures that the GOT header is at
   the start of the output TOC section.  */
  htab = ppc_hash_table (info);
  htab->elf.dynobj = params->stub_bfd;
  htab->params = params;

  return create_linkage_sections (htab->elf.dynobj, info);
}

/* Build a name for an entry in the stub hash table.  */

static char *
ppc_stub_name (const asection *input_section,
	       const asection *sym_sec,
	       const struct ppc_link_hash_entry *h,
	       const Elf_Internal_Rela *rel)
{
  char *stub_name;
  ssize_t len;

  /* rel->r_addend is actually 64 bit, but who uses more than +/- 2^31
     offsets from a sym as a branch target?  In fact, we could
     probably assume the addend is always zero.  */
  BFD_ASSERT (((int) rel->r_addend & 0xffffffff) == rel->r_addend);

  if (h)
    {
      len = 8 + 1 + strlen (h->elf.root.root.string) + 1 + 8 + 1;
      stub_name = bfd_malloc (len);
      if (stub_name == NULL)
	return stub_name;

      len = sprintf (stub_name, "%08x.%s+%x",
		     input_section->id & 0xffffffff,
		     h->elf.root.root.string,
		     (int) rel->r_addend & 0xffffffff);
    }
  else
    {
      len = 8 + 1 + 8 + 1 + 8 + 1 + 8 + 1;
      stub_name = bfd_malloc (len);
      if (stub_name == NULL)
	return stub_name;

      len = sprintf (stub_name, "%08x.%x:%x+%x",
		     input_section->id & 0xffffffff,
		     sym_sec->id & 0xffffffff,
		     (int) ELF64_R_SYM (rel->r_info) & 0xffffffff,
		     (int) rel->r_addend & 0xffffffff);
    }
  if (len > 2 && stub_name[len - 2] == '+' && stub_name[len - 1] == '0')
    stub_name[len - 2] = 0;
  return stub_name;
}

/* If mixing power10 with non-power10 code and --power10-stubs is not
   specified (or is auto) then there may be multiple stub types for any
   given symbol.  Up to three classes of stubs are stored in separate
   stub_hash_table entries having the same key string.  The entries
   will always be adjacent on entry->root.next chain, even if hash
   table resizing occurs.  This function selects the correct entry to
   use.  */

static struct ppc_stub_hash_entry *
select_alt_stub (struct ppc_stub_hash_entry *entry,
		 enum elf_ppc64_reloc_type r_type)
{
  enum ppc_stub_sub_type subt;

  switch (r_type)
    {
    case R_PPC64_REL24_NOTOC:
      subt = ppc_stub_notoc;
      break;
    case R_PPC64_REL24_P9NOTOC:
      subt = ppc_stub_p9notoc;
      break;
    default:
      subt = ppc_stub_toc;
      break;
    }

  while (entry != NULL && entry->type.sub != subt)
    {
      const char *stub_name = entry->root.string;

      entry = (struct ppc_stub_hash_entry *) entry->root.next;
      if (entry != NULL
	  && entry->root.string != stub_name)
	entry = NULL;
    }

  return entry;
}

/* Look up an entry in the stub hash.  Stub entries are cached because
   creating the stub name takes a bit of time.  */

static struct ppc_stub_hash_entry *
ppc_get_stub_entry (const asection *input_section,
		    const asection *sym_sec,
		    struct ppc_link_hash_entry *h,
		    const Elf_Internal_Rela *rel,
		    struct ppc_link_hash_table *htab)
{
  struct ppc_stub_hash_entry *stub_entry;
  struct map_stub *group;

  /* If this input section is part of a group of sections sharing one
     stub section, then use the id of the first section in the group.
     Stub names need to include a section id, as there may well be
     more than one stub used to reach say, printf, and we need to
     distinguish between them.  */
  group = htab->sec_info[input_section->id].u.group;
  if (group == NULL)
    return NULL;

  if (h != NULL && h->u.stub_cache != NULL
      && h->u.stub_cache->h == h
      && h->u.stub_cache->group == group)
    {
      stub_entry = h->u.stub_cache;
    }
  else
    {
      char *stub_name;

      stub_name = ppc_stub_name (group->link_sec, sym_sec, h, rel);
      if (stub_name == NULL)
	return NULL;

      stub_entry = ppc_stub_hash_lookup (&htab->stub_hash_table,
					 stub_name, false, false);
      if (h != NULL)
	h->u.stub_cache = stub_entry;

      free (stub_name);
    }

  if (stub_entry != NULL && htab->params->power10_stubs == -1)
    stub_entry = select_alt_stub (stub_entry, ELF64_R_TYPE (rel->r_info));

  return stub_entry;
}

/* Add a new stub entry to the stub hash.  Not all fields of the new
   stub entry are initialised.  */

static struct ppc_stub_hash_entry *
ppc_add_stub (const char *stub_name,
	      asection *section,
	      struct bfd_link_info *info)
{
  struct ppc_link_hash_table *htab = ppc_hash_table (info);
  struct map_stub *group;
  asection *link_sec;
  asection *stub_sec;
  struct ppc_stub_hash_entry *stub_entry;

  group = htab->sec_info[section->id].u.group;
  link_sec = group->link_sec;
  stub_sec = group->stub_sec;
  if (stub_sec == NULL)
    {
      size_t namelen;
      bfd_size_type len;
      char *s_name;

      namelen = strlen (link_sec->name);
      len = namelen + sizeof (STUB_SUFFIX);
      s_name = bfd_alloc (htab->params->stub_bfd, len);
      if (s_name == NULL)
	return NULL;

      memcpy (s_name, link_sec->name, namelen);
      memcpy (s_name + namelen, STUB_SUFFIX, sizeof (STUB_SUFFIX));
      stub_sec = (*htab->params->add_stub_section) (s_name, link_sec);
      if (stub_sec == NULL)
	return NULL;
      group->stub_sec = stub_sec;
    }

  /* Enter this entry into the linker stub hash table.  */
  stub_entry = ppc_stub_hash_lookup (&htab->stub_hash_table, stub_name,
				     true, false);
  if (stub_entry == NULL)
    {
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: cannot create stub entry %s"),
			  section->owner, stub_name);
      return NULL;
    }

  stub_entry->group = group;
  stub_entry->stub_offset = 0;
  return stub_entry;
}

/* A stub has already been created, but it may not be the required
   type.  We shouldn't be transitioning from plt_call to long_branch
   stubs or vice versa, but we might be upgrading from plt_call to
   plt_call with r2save for example.  */

static bool
ppc_merge_stub (struct ppc_link_hash_table *htab,
		struct ppc_stub_hash_entry *stub_entry,
		struct ppc_stub_type stub_type,
		enum elf_ppc64_reloc_type r_type)
{
  struct ppc_stub_type old_type = stub_entry->type;

  if (old_type.main == ppc_stub_save_res)
    return true;

  if (htab->params->power10_stubs == -1)
    {
      /* For --power10-stubs=auto, don't merge _notoc and other
	 varieties of stubs.  */
      struct ppc_stub_hash_entry *alt_stub;

      alt_stub = select_alt_stub (stub_entry, r_type);
      if (alt_stub == NULL)
	{
	  alt_stub = ((struct ppc_stub_hash_entry *)
		      stub_hash_newfunc (NULL,
					 &htab->stub_hash_table,
					 stub_entry->root.string));
	  if (alt_stub == NULL)
	    return false;

	  *alt_stub = *stub_entry;
	  stub_entry->root.next = &alt_stub->root;

	  /* Sort notoc stubs first, then toc stubs, then p9notoc.
	     Not that it matters, this just puts smaller stubs first.  */
	  if (stub_type.sub == ppc_stub_notoc)
	    alt_stub = stub_entry;
	  else if (stub_type.sub == ppc_stub_p9notoc
		   && alt_stub->root.next
		   && alt_stub->root.next->string == alt_stub->root.string)
	    {
	      struct ppc_stub_hash_entry *next
		= (struct ppc_stub_hash_entry *) alt_stub->root.next;
	      alt_stub->type = next->type;
	      alt_stub = next;
	    }
	  alt_stub->type = stub_type;
	  return true;
	}
      stub_entry = alt_stub;
    }

  old_type = stub_entry->type;
  if (old_type.main == ppc_stub_plt_branch)
    old_type.main = ppc_stub_long_branch;

  if (old_type.main != stub_type.main
      || (old_type.sub != stub_type.sub
	  && old_type.sub != ppc_stub_toc
	  && stub_type.sub != ppc_stub_toc))
    abort ();

  stub_entry->type.sub |= stub_type.sub;
  stub_entry->type.r2save |= stub_type.r2save;
  return true;
}

/* Create .got and .rela.got sections in ABFD, and .got in dynobj if
   not already done.  */

static bool
create_got_section (bfd *abfd, struct bfd_link_info *info)
{
  asection *got, *relgot;
  flagword flags;
  struct ppc_link_hash_table *htab = ppc_hash_table (info);

  if (!is_ppc64_elf (abfd))
    return false;
  if (htab == NULL)
    return false;

  if (!htab->elf.sgot
      && !_bfd_elf_create_got_section (htab->elf.dynobj, info))
    return false;

  flags = (SEC_ALLOC | SEC_LOAD | SEC_HAS_CONTENTS | SEC_IN_MEMORY
	   | SEC_LINKER_CREATED);

  got = bfd_make_section_anyway_with_flags (abfd, ".got", flags);
  if (!got
      || !bfd_set_section_alignment (got, 3))
    return false;

  relgot = bfd_make_section_anyway_with_flags (abfd, ".rela.got",
					       flags | SEC_READONLY);
  if (!relgot
      || !bfd_set_section_alignment (relgot, 3))
    return false;

  ppc64_elf_tdata (abfd)->got = got;
  ppc64_elf_tdata (abfd)->relgot = relgot;
  return true;
}

/* Follow indirect and warning symbol links.  */

static inline struct bfd_link_hash_entry *
follow_link (struct bfd_link_hash_entry *h)
{
  while (h->type == bfd_link_hash_indirect
	 || h->type == bfd_link_hash_warning)
    h = h->u.i.link;
  return h;
}

static inline struct elf_link_hash_entry *
elf_follow_link (struct elf_link_hash_entry *h)
{
  return (struct elf_link_hash_entry *) follow_link (&h->root);
}

static inline struct ppc_link_hash_entry *
ppc_follow_link (struct ppc_link_hash_entry *h)
{
  return ppc_elf_hash_entry (elf_follow_link (&h->elf));
}

/* Merge PLT info on FROM with that on TO.  */

static void
move_plt_plist (struct ppc_link_hash_entry *from,
		struct ppc_link_hash_entry *to)
{
  if (from->elf.plt.plist != NULL)
    {
      if (to->elf.plt.plist != NULL)
	{
	  struct plt_entry **entp;
	  struct plt_entry *ent;

	  for (entp = &from->elf.plt.plist; (ent = *entp) != NULL; )
	    {
	      struct plt_entry *dent;

	      for (dent = to->elf.plt.plist; dent != NULL; dent = dent->next)
		if (dent->addend == ent->addend)
		  {
		    dent->plt.refcount += ent->plt.refcount;
		    *entp = ent->next;
		    break;
		  }
	      if (dent == NULL)
		entp = &ent->next;
	    }
	  *entp = to->elf.plt.plist;
	}

      to->elf.plt.plist = from->elf.plt.plist;
      from->elf.plt.plist = NULL;
    }
}

/* Copy the extra info we tack onto an elf_link_hash_entry.  */

static void
ppc64_elf_copy_indirect_symbol (struct bfd_link_info *info,
				struct elf_link_hash_entry *dir,
				struct elf_link_hash_entry *ind)
{
  struct ppc_link_hash_entry *edir, *eind;

  edir = ppc_elf_hash_entry (dir);
  eind = ppc_elf_hash_entry (ind);

  edir->is_func |= eind->is_func;
  edir->is_func_descriptor |= eind->is_func_descriptor;
  edir->tls_mask |= eind->tls_mask;
  if (eind->oh != NULL)
    edir->oh = ppc_follow_link (eind->oh);

  if (edir->elf.versioned != versioned_hidden)
    edir->elf.ref_dynamic |= eind->elf.ref_dynamic;
  edir->elf.ref_regular |= eind->elf.ref_regular;
  edir->elf.ref_regular_nonweak |= eind->elf.ref_regular_nonweak;
  edir->elf.non_got_ref |= eind->elf.non_got_ref;
  edir->elf.needs_plt |= eind->elf.needs_plt;
  edir->elf.pointer_equality_needed |= eind->elf.pointer_equality_needed;

  /* If we were called to copy over info for a weak sym, don't copy
     dyn_relocs, plt/got info, or dynindx.  We used to copy dyn_relocs
     in order to simplify readonly_dynrelocs and save a field in the
     symbol hash entry, but that means dyn_relocs can't be used in any
     tests about a specific symbol, or affect other symbol flags which
     are then tested.  */
  if (eind->elf.root.type != bfd_link_hash_indirect)
    return;

  /* Copy over any dynamic relocs we may have on the indirect sym.  */
  if (ind->dyn_relocs != NULL)
    {
      if (dir->dyn_relocs != NULL)
	{
	  struct ppc_dyn_relocs **pp;
	  struct ppc_dyn_relocs *p;

	  /* Add reloc counts against the indirect sym to the direct sym
	     list.  Merge any entries against the same section.  */
	  for (pp = (struct ppc_dyn_relocs **) &ind->dyn_relocs;
	       (p = *pp) != NULL;
	       )
	    {
	      struct ppc_dyn_relocs *q;

	      for (q = (struct ppc_dyn_relocs *) dir->dyn_relocs;
		   q != NULL;
		   q = q->next)
		if (q->sec == p->sec)
		  {
		    q->count += p->count;
		    q->pc_count += p->pc_count;
		    q->rel_count += p->rel_count;
		    *pp = p->next;
		    break;
		  }
	      if (q == NULL)
		pp = &p->next;
	    }
	  *pp = (struct ppc_dyn_relocs *) dir->dyn_relocs;
	}

      dir->dyn_relocs = ind->dyn_relocs;
      ind->dyn_relocs = NULL;
    }

  /* Copy over got entries that we may have already seen to the
     symbol which just became indirect.  */
  if (eind->elf.got.glist != NULL)
    {
      if (edir->elf.got.glist != NULL)
	{
	  struct got_entry **entp;
	  struct got_entry *ent;

	  for (entp = &eind->elf.got.glist; (ent = *entp) != NULL; )
	    {
	      struct got_entry *dent;

	      for (dent = edir->elf.got.glist; dent != NULL; dent = dent->next)
		if (dent->addend == ent->addend
		    && dent->owner == ent->owner
		    && dent->tls_type == ent->tls_type)
		  {
		    dent->got.refcount += ent->got.refcount;
		    *entp = ent->next;
		    break;
		  }
	      if (dent == NULL)
		entp = &ent->next;
	    }
	  *entp = edir->elf.got.glist;
	}

      edir->elf.got.glist = eind->elf.got.glist;
      eind->elf.got.glist = NULL;
    }

  /* And plt entries.  */
  move_plt_plist (eind, edir);

  if (eind->elf.dynindx != -1)
    {
      if (edir->elf.dynindx != -1)
	_bfd_elf_strtab_delref (elf_hash_table (info)->dynstr,
				edir->elf.dynstr_index);
      edir->elf.dynindx = eind->elf.dynindx;
      edir->elf.dynstr_index = eind->elf.dynstr_index;
      eind->elf.dynindx = -1;
      eind->elf.dynstr_index = 0;
    }
}

/* Find the function descriptor hash entry from the given function code
   hash entry FH.  Link the entries via their OH fields.  */

static struct ppc_link_hash_entry *
lookup_fdh (struct ppc_link_hash_entry *fh, struct ppc_link_hash_table *htab)
{
  struct ppc_link_hash_entry *fdh = fh->oh;

  if (fdh == NULL)
    {
      const char *fd_name = fh->elf.root.root.string + 1;

      fdh = ppc_elf_hash_entry (elf_link_hash_lookup (&htab->elf, fd_name,
						      false, false, false));
      if (fdh == NULL)
	return fdh;

      fdh->is_func_descriptor = 1;
      fdh->oh = fh;
      fh->is_func = 1;
      fh->oh = fdh;
    }

  fdh = ppc_follow_link (fdh);
  fdh->is_func_descriptor = 1;
  fdh->oh = fh;
  return fdh;
}

/* Make a fake function descriptor sym for the undefined code sym FH.  */

static struct ppc_link_hash_entry *
make_fdh (struct bfd_link_info *info,
	  struct ppc_link_hash_entry *fh)
{
  bfd *abfd = fh->elf.root.u.undef.abfd;
  struct bfd_link_hash_entry *bh = NULL;
  struct ppc_link_hash_entry *fdh;
  flagword flags = (fh->elf.root.type == bfd_link_hash_undefweak
		    ? BSF_WEAK
		    : BSF_GLOBAL);

  if (!_bfd_generic_link_add_one_symbol (info, abfd,
					 fh->elf.root.root.string + 1,
					 flags, bfd_und_section_ptr, 0,
					 NULL, false, false, &bh))
    return NULL;

  fdh = (struct ppc_link_hash_entry *) bh;
  fdh->elf.non_elf = 0;
  fdh->fake = 1;
  fdh->is_func_descriptor = 1;
  fdh->oh = fh;
  fh->is_func = 1;
  fh->oh = fdh;
  return fdh;
}

/* Fix function descriptor symbols defined in .opd sections to be
   function type.  */

static bool
ppc64_elf_add_symbol_hook (bfd *ibfd,
			   struct bfd_link_info *info,
			   Elf_Internal_Sym *isym,
			   const char **name,
			   flagword *flags ATTRIBUTE_UNUSED,
			   asection **sec,
			   bfd_vma *value)
{
  if (*sec != NULL
      && strcmp ((*sec)->name, ".opd") == 0)
    {
      asection *code_sec;

      if (!(ELF_ST_TYPE (isym->st_info) == STT_GNU_IFUNC
	    || ELF_ST_TYPE (isym->st_info) == STT_FUNC))
	isym->st_info = ELF_ST_INFO (ELF_ST_BIND (isym->st_info), STT_FUNC);

      /* If the symbol is a function defined in .opd, and the function
	 code is in a discarded group, let it appear to be undefined.  */
      if (!bfd_link_relocatable (info)
	  && (*sec)->reloc_count != 0
	  && opd_entry_value (*sec, *value, &code_sec, NULL,
			      false) != (bfd_vma) -1
	  && discarded_section (code_sec))
	{
	  *sec = bfd_und_section_ptr;
	  isym->st_shndx = SHN_UNDEF;
	}
    }
  else if (*sec != NULL
	   && strcmp ((*sec)->name, ".toc") == 0
	   && ELF_ST_TYPE (isym->st_info) == STT_OBJECT)
    {
      struct ppc_link_hash_table *htab = ppc_hash_table (info);
      if (htab != NULL)
	htab->params->object_in_toc = 1;
    }

  if ((STO_PPC64_LOCAL_MASK & isym->st_other) != 0)
    {
      if (abiversion (ibfd) == 0)
	set_abiversion (ibfd, 2);
      else if (abiversion (ibfd) == 1)
	{
	  _bfd_error_handler (_("symbol '%s' has invalid st_other"
				" for ABI version 1"), *name);
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}
    }

  return true;
}

/* Merge non-visibility st_other attributes: local entry point.  */

static void
ppc64_elf_merge_symbol_attribute (struct elf_link_hash_entry *h,
				  unsigned int st_other,
				  bool definition,
				  bool dynamic)
{
  if (definition && (!dynamic || !h->def_regular))
    h->other = ((st_other & ~ELF_ST_VISIBILITY (-1))
		| ELF_ST_VISIBILITY (h->other));
}

/* Hook called on merging a symbol.  We use this to clear "fake" since
   we now have a real symbol.  */

static bool
ppc64_elf_merge_symbol (struct elf_link_hash_entry *h,
			const Elf_Internal_Sym *isym,
			asection **psec ATTRIBUTE_UNUSED,
			bool newdef ATTRIBUTE_UNUSED,
			bool olddef ATTRIBUTE_UNUSED,
			bfd *oldbfd ATTRIBUTE_UNUSED,
			const asection *oldsec ATTRIBUTE_UNUSED)
{
  ppc_elf_hash_entry (h)->fake = 0;
  if ((STO_PPC64_LOCAL_MASK & isym->st_other) != 0)
    ppc_elf_hash_entry (h)->non_zero_localentry = 1;
  return true;
}

/* This function makes an old ABI object reference to ".bar" cause the
   inclusion of a new ABI object archive that defines "bar".
   NAME is a symbol defined in an archive.  Return a symbol in the hash
   table that might be satisfied by the archive symbols.  */

static struct bfd_link_hash_entry *
ppc64_elf_archive_symbol_lookup (bfd *abfd,
				 struct bfd_link_info *info,
				 const char *name)
{
  struct bfd_link_hash_entry *h;
  char *dot_name;
  size_t len;

  h = _bfd_elf_archive_symbol_lookup (abfd, info, name);
  if (h != NULL
      && ppc_hash_table (info) != NULL
      /* Don't return this sym if it is a fake function descriptor
	 created by add_symbol_adjust.  */
      && !((struct ppc_link_hash_entry *) h)->fake)
    return h;

  if (name[0] == '.')
    return h;

  len = strlen (name);
  dot_name = bfd_alloc (abfd, len + 2);
  if (dot_name == NULL)
    return (struct bfd_link_hash_entry *) -1;
  dot_name[0] = '.';
  memcpy (dot_name + 1, name, len + 1);
  h = _bfd_elf_archive_symbol_lookup (abfd, info, dot_name);
  bfd_release (abfd, dot_name);
  if (h != NULL)
    return h;

  if (strcmp (name, "__tls_get_addr_opt") == 0)
    h = _bfd_elf_archive_symbol_lookup (abfd, info, "__tls_get_addr_desc");
  return h;
}

/* This function satisfies all old ABI object references to ".bar" if a
   new ABI object defines "bar".  Well, at least, undefined dot symbols
   are made weak.  This stops later archive searches from including an
   object if we already have a function descriptor definition.  It also
   prevents the linker complaining about undefined symbols.
   We also check and correct mismatched symbol visibility here.  The
   most restrictive visibility of the function descriptor and the
   function entry symbol is used.  */

static bool
add_symbol_adjust (struct ppc_link_hash_entry *eh, struct bfd_link_info *info)
{
  struct ppc_link_hash_table *htab;
  struct ppc_link_hash_entry *fdh;

  if (eh->elf.root.type == bfd_link_hash_warning)
    eh = (struct ppc_link_hash_entry *) eh->elf.root.u.i.link;

  if (eh->elf.root.type == bfd_link_hash_indirect)
    return true;

  if (eh->elf.root.root.string[0] != '.')
    abort ();

  htab = ppc_hash_table (info);
  if (htab == NULL)
    return false;

  fdh = lookup_fdh (eh, htab);
  if (fdh == NULL
      && !bfd_link_relocatable (info)
      && (eh->elf.root.type == bfd_link_hash_undefined
	  || eh->elf.root.type == bfd_link_hash_undefweak)
      && eh->elf.ref_regular)
    {
      /* Make an undefined function descriptor sym, in order to
	 pull in an --as-needed shared lib.  Archives are handled
	 elsewhere.  */
      fdh = make_fdh (info, eh);
      if (fdh == NULL)
	return false;
    }

  if (fdh != NULL)
    {
      unsigned entry_vis = ELF_ST_VISIBILITY (eh->elf.other) - 1;
      unsigned descr_vis = ELF_ST_VISIBILITY (fdh->elf.other) - 1;

      /* Make both descriptor and entry symbol have the most
	 constraining visibility of either symbol.  */
      if (entry_vis < descr_vis)
	fdh->elf.other += entry_vis - descr_vis;
      else if (entry_vis > descr_vis)
	eh->elf.other += descr_vis - entry_vis;

      /* Propagate reference flags from entry symbol to function
	 descriptor symbol.  */
      fdh->elf.root.non_ir_ref_regular |= eh->elf.root.non_ir_ref_regular;
      fdh->elf.root.non_ir_ref_dynamic |= eh->elf.root.non_ir_ref_dynamic;
      fdh->elf.ref_regular |= eh->elf.ref_regular;
      fdh->elf.ref_regular_nonweak |= eh->elf.ref_regular_nonweak;

      if (!fdh->elf.forced_local
	  && fdh->elf.dynindx == -1
	  && fdh->elf.versioned != versioned_hidden
	  && (bfd_link_dll (info)
	      || fdh->elf.def_dynamic
	      || fdh->elf.ref_dynamic)
	  && (eh->elf.ref_regular
	      || eh->elf.def_regular))
	{
	  if (!bfd_elf_link_record_dynamic_symbol (info, &fdh->elf))
	    return false;
	}
    }

  return true;
}

/* Set up opd section info and abiversion for IBFD, and process list
   of dot-symbols we made in link_hash_newfunc.  */

static bool
ppc64_elf_before_check_relocs (bfd *ibfd, struct bfd_link_info *info)
{
  struct ppc_link_hash_table *htab;
  struct ppc_link_hash_entry **p, *eh;
  asection *opd = bfd_get_section_by_name (ibfd, ".opd");

  if (opd != NULL && opd->size != 0)
    {
      if (ppc64_elf_section_data (opd)->sec_type == sec_normal)
	ppc64_elf_section_data (opd)->sec_type = sec_opd;
      else if (ppc64_elf_section_data (opd)->sec_type != sec_opd)
	BFD_FAIL ();

      if (abiversion (ibfd) == 0)
	set_abiversion (ibfd, 1);
      else if (abiversion (ibfd) >= 2)
	{
	  /* xgettext:c-format */
	  _bfd_error_handler (_("%pB .opd not allowed in ABI version %d"),
			      ibfd, abiversion (ibfd));
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}
    }

  if (is_ppc64_elf (info->output_bfd))
    {
      /* For input files without an explicit abiversion in e_flags
	 we should have flagged any with symbol st_other bits set
	 as ELFv1 and above flagged those with .opd as ELFv2.
	 Set the output abiversion if not yet set, and for any input
	 still ambiguous, take its abiversion from the output.
	 Differences in ABI are reported later.  */
      if (abiversion (info->output_bfd) == 0)
	set_abiversion (info->output_bfd, abiversion (ibfd));
      else if (abiversion (ibfd) == 0)
	set_abiversion (ibfd, abiversion (info->output_bfd));
    }

  htab = ppc_hash_table (info);
  if (htab == NULL)
    return true;

  if (opd != NULL && opd->size != 0
      && (ibfd->flags & DYNAMIC) == 0
      && (opd->flags & SEC_RELOC) != 0
      && opd->reloc_count != 0
      && !bfd_is_abs_section (opd->output_section)
      && info->gc_sections)
    {
      /* Garbage collection needs some extra help with .opd sections.
	 We don't want to necessarily keep everything referenced by
	 relocs in .opd, as that would keep all functions.  Instead,
	 if we reference an .opd symbol (a function descriptor), we
	 want to keep the function code symbol's section.  This is
	 easy for global symbols, but for local syms we need to keep
	 information about the associated function section.  */
      bfd_size_type amt;
      asection **opd_sym_map;
      Elf_Internal_Shdr *symtab_hdr;
      Elf_Internal_Rela *relocs, *rel_end, *rel;

      amt = OPD_NDX (opd->size) * sizeof (*opd_sym_map);
      opd_sym_map = bfd_zalloc (ibfd, amt);
      if (opd_sym_map == NULL)
	return false;
      ppc64_elf_section_data (opd)->u.opd.func_sec = opd_sym_map;
      relocs = _bfd_elf_link_read_relocs (ibfd, opd, NULL, NULL,
					  info->keep_memory);
      if (relocs == NULL)
	return false;
      symtab_hdr = &elf_symtab_hdr (ibfd);
      rel_end = relocs + opd->reloc_count - 1;
      for (rel = relocs; rel < rel_end; rel++)
	{
	  enum elf_ppc64_reloc_type r_type = ELF64_R_TYPE (rel->r_info);
	  unsigned long r_symndx = ELF64_R_SYM (rel->r_info);

	  if (r_type == R_PPC64_ADDR64
	      && ELF64_R_TYPE ((rel + 1)->r_info) == R_PPC64_TOC
	      && r_symndx < symtab_hdr->sh_info)
	    {
	      Elf_Internal_Sym *isym;
	      asection *s;

	      isym = bfd_sym_from_r_symndx (&htab->elf.sym_cache, ibfd,
					    r_symndx);
	      if (isym == NULL)
		{
		  if (elf_section_data (opd)->relocs != relocs)
		    free (relocs);
		  return false;
		}

	      s = bfd_section_from_elf_index (ibfd, isym->st_shndx);
	      if (s != NULL && s != opd)
		opd_sym_map[OPD_NDX (rel->r_offset)] = s;
	    }
	}
      if (elf_section_data (opd)->relocs != relocs)
	free (relocs);
    }

  p = &htab->dot_syms;
  while ((eh = *p) != NULL)
    {
      *p = NULL;
      if (&eh->elf == htab->elf.hgot)
	;
      else if (htab->elf.hgot == NULL
	       && strcmp (eh->elf.root.root.string, ".TOC.") == 0)
	htab->elf.hgot = &eh->elf;
      else if (abiversion (ibfd) <= 1)
	{
	  htab->need_func_desc_adj = 1;
	  if (!add_symbol_adjust (eh, info))
	    return false;
	}
      p = &eh->u.next_dot_sym;
    }
  return true;
}

/* Undo hash table changes when an --as-needed input file is determined
   not to be needed.  */

static bool
ppc64_elf_notice_as_needed (bfd *ibfd,
			    struct bfd_link_info *info,
			    enum notice_asneeded_action act)
{
  if (act == notice_not_needed)
    {
      struct ppc_link_hash_table *htab = ppc_hash_table (info);

      if (htab == NULL)
	return false;

      htab->dot_syms = NULL;
    }
  return _bfd_elf_notice_as_needed (ibfd, info, act);
}

/* If --just-symbols against a final linked binary, then assume we need
   toc adjusting stubs when calling functions defined there.  */

static void
ppc64_elf_link_just_syms (asection *sec, struct bfd_link_info *info)
{
  if ((sec->flags & SEC_CODE) != 0
      && (sec->owner->flags & (EXEC_P | DYNAMIC)) != 0
      && is_ppc64_elf (sec->owner))
    {
      if (abiversion (sec->owner) >= 2
	  || bfd_get_section_by_name (sec->owner, ".opd") != NULL)
	sec->has_toc_reloc = 1;
    }
  _bfd_elf_link_just_syms (sec, info);
}

static struct plt_entry **
update_local_sym_info (bfd *abfd, Elf_Internal_Shdr *symtab_hdr,
		       unsigned long r_symndx, bfd_vma r_addend, int tls_type)
{
  struct got_entry **local_got_ents = elf_local_got_ents (abfd);
  struct plt_entry **local_plt;
  unsigned char *local_got_tls_masks;

  if (local_got_ents == NULL)
    {
      bfd_size_type size = symtab_hdr->sh_info;

      size *= (sizeof (*local_got_ents)
	       + sizeof (*local_plt)
	       + sizeof (*local_got_tls_masks));
      local_got_ents = bfd_zalloc (abfd, size);
      if (local_got_ents == NULL)
	return NULL;
      elf_local_got_ents (abfd) = local_got_ents;
    }

  if ((tls_type & (NON_GOT | TLS_EXPLICIT)) == 0)
    {
      struct got_entry *ent;

      for (ent = local_got_ents[r_symndx]; ent != NULL; ent = ent->next)
	if (ent->addend == r_addend
	    && ent->owner == abfd
	    && ent->tls_type == tls_type)
	  break;
      if (ent == NULL)
	{
	  size_t amt = sizeof (*ent);
	  ent = bfd_alloc (abfd, amt);
	  if (ent == NULL)
	    return false;
	  ent->next = local_got_ents[r_symndx];
	  ent->addend = r_addend;
	  ent->owner = abfd;
	  ent->tls_type = tls_type;
	  ent->is_indirect = false;
	  ent->got.refcount = 0;
	  local_got_ents[r_symndx] = ent;
	}
      ent->got.refcount += 1;
    }

  local_plt = (struct plt_entry **) (local_got_ents + symtab_hdr->sh_info);
  local_got_tls_masks = (unsigned char *) (local_plt + symtab_hdr->sh_info);
  local_got_tls_masks[r_symndx] |= tls_type & 0xff;

  return local_plt + r_symndx;
}

static bool
update_plt_info (bfd *abfd, struct plt_entry **plist, bfd_vma addend)
{
  struct plt_entry *ent;

  for (ent = *plist; ent != NULL; ent = ent->next)
    if (ent->addend == addend)
      break;
  if (ent == NULL)
    {
      size_t amt = sizeof (*ent);
      ent = bfd_alloc (abfd, amt);
      if (ent == NULL)
	return false;
      ent->next = *plist;
      ent->addend = addend;
      ent->plt.refcount = 0;
      *plist = ent;
    }
  ent->plt.refcount += 1;
  return true;
}

static bool
is_branch_reloc (enum elf_ppc64_reloc_type r_type)
{
  return (r_type == R_PPC64_REL24
	  || r_type == R_PPC64_REL24_NOTOC
	  || r_type == R_PPC64_REL24_P9NOTOC
	  || r_type == R_PPC64_REL14
	  || r_type == R_PPC64_REL14_BRTAKEN
	  || r_type == R_PPC64_REL14_BRNTAKEN
	  || r_type == R_PPC64_ADDR24
	  || r_type == R_PPC64_ADDR14
	  || r_type == R_PPC64_ADDR14_BRTAKEN
	  || r_type == R_PPC64_ADDR14_BRNTAKEN
	  || r_type == R_PPC64_PLTCALL
	  || r_type == R_PPC64_PLTCALL_NOTOC);
}

/* Relocs on inline plt call sequence insns prior to the call.  */

static bool
is_plt_seq_reloc (enum elf_ppc64_reloc_type r_type)
{
  return (r_type == R_PPC64_PLT16_HA
	  || r_type == R_PPC64_PLT16_HI
	  || r_type == R_PPC64_PLT16_LO
	  || r_type == R_PPC64_PLT16_LO_DS
	  || r_type == R_PPC64_PLT_PCREL34
	  || r_type == R_PPC64_PLT_PCREL34_NOTOC
	  || r_type == R_PPC64_PLTSEQ
	  || r_type == R_PPC64_PLTSEQ_NOTOC);
}

/* Of relocs which might appear paired with TLSGD and TLSLD marker
   relocs, return true for those that operate on a dword.  */

static bool
is_8byte_reloc (enum elf_ppc64_reloc_type r_type)
{
  return (r_type == R_PPC64_PLT_PCREL34
	  || r_type == R_PPC64_PLT_PCREL34_NOTOC
	  || r_type == R_PPC64_PLTCALL);
}

/* Like bfd_reloc_offset_in_range but without a howto.  Return true
   iff a field of SIZE bytes at OFFSET is within SEC limits.  */

static bool
offset_in_range (asection *sec, bfd_vma offset, size_t size)
{
  return offset <= sec->size && size <= sec->size - offset;
}

/* Look through the relocs for a section during the first phase, and
   calculate needed space in the global offset table, procedure
   linkage table, and dynamic reloc sections.  */

static bool
ppc64_elf_check_relocs (bfd *abfd, struct bfd_link_info *info,
			asection *sec, const Elf_Internal_Rela *relocs)
{
  struct ppc_link_hash_table *htab;
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  const Elf_Internal_Rela *rel;
  const Elf_Internal_Rela *rel_end;
  asection *sreloc;
  struct elf_link_hash_entry *tga, *dottga;
  bool is_opd;

  if (bfd_link_relocatable (info))
    return true;

  BFD_ASSERT (is_ppc64_elf (abfd));

  htab = ppc_hash_table (info);
  if (htab == NULL)
    return false;

  tga = elf_link_hash_lookup (&htab->elf, "__tls_get_addr",
			      false, false, true);
  dottga = elf_link_hash_lookup (&htab->elf, ".__tls_get_addr",
				 false, false, true);
  symtab_hdr = &elf_symtab_hdr (abfd);
  sym_hashes = elf_sym_hashes (abfd);
  sreloc = NULL;
  is_opd = ppc64_elf_section_data (sec)->sec_type == sec_opd;
  rel_end = relocs + sec->reloc_count;
  for (rel = relocs; rel < rel_end; rel++)
    {
      unsigned long r_symndx;
      struct elf_link_hash_entry *h;
      Elf_Internal_Sym *isym;
      enum elf_ppc64_reloc_type r_type;
      int tls_type;
      struct _ppc64_elf_section_data *ppc64_sec;
      struct plt_entry **ifunc, **plt_list;

      r_symndx = ELF64_R_SYM (rel->r_info);
      if (r_symndx < symtab_hdr->sh_info)
	{
	  h = NULL;
	  isym = bfd_sym_from_r_symndx (&htab->elf.sym_cache, abfd, r_symndx);
	  if (isym == NULL)
	    return false;
	}
      else
	{
	  isym = NULL;
	  h = sym_hashes[r_symndx - symtab_hdr->sh_info];
	  h = elf_follow_link (h);

	  if (h == htab->elf.hgot)
	    sec->has_toc_reloc = 1;
	}

      r_type = ELF64_R_TYPE (rel->r_info);
      switch (r_type)
	{
	case R_PPC64_D34:
	case R_PPC64_D34_LO:
	case R_PPC64_D34_HI30:
	case R_PPC64_D34_HA30:
	case R_PPC64_D28:
	case R_PPC64_TPREL34:
	case R_PPC64_DTPREL34:
	case R_PPC64_PCREL34:
	case R_PPC64_GOT_PCREL34:
	case R_PPC64_GOT_TLSGD_PCREL34:
	case R_PPC64_GOT_TLSLD_PCREL34:
	case R_PPC64_GOT_TPREL_PCREL34:
	case R_PPC64_GOT_DTPREL_PCREL34:
	case R_PPC64_PLT_PCREL34:
	case R_PPC64_PLT_PCREL34_NOTOC:
	case R_PPC64_PCREL28:
	  htab->has_power10_relocs = 1;
	  break;
	default:
	  break;
	}

      switch (r_type)
	{
	case R_PPC64_PLT16_HA:
	case R_PPC64_GOT_TLSLD16_HA:
	case R_PPC64_GOT_TLSGD16_HA:
	case R_PPC64_GOT_TPREL16_HA:
	case R_PPC64_GOT_DTPREL16_HA:
	case R_PPC64_GOT16_HA:
	case R_PPC64_TOC16_HA:
	case R_PPC64_PLT16_LO:
	case R_PPC64_PLT16_LO_DS:
	case R_PPC64_GOT_TLSLD16_LO:
	case R_PPC64_GOT_TLSGD16_LO:
	case R_PPC64_GOT_TPREL16_LO_DS:
	case R_PPC64_GOT_DTPREL16_LO_DS:
	case R_PPC64_GOT16_LO:
	case R_PPC64_GOT16_LO_DS:
	case R_PPC64_TOC16_LO:
	case R_PPC64_TOC16_LO_DS:
	case R_PPC64_GOT_PCREL34:
	  ppc64_elf_tdata (abfd)->has_optrel = 1;
	  ppc64_elf_section_data (sec)->has_optrel = 1;
	  break;
	default:
	  break;
	}

      ifunc = NULL;
      if (h != NULL)
	{
	  if (h->type == STT_GNU_IFUNC)
	    {
	      h->needs_plt = 1;
	      ifunc = &h->plt.plist;
	    }
	}
      else
	{
	  if (ELF_ST_TYPE (isym->st_info) == STT_GNU_IFUNC)
	    {
	      ifunc = update_local_sym_info (abfd, symtab_hdr, r_symndx,
					     rel->r_addend,
					     NON_GOT | PLT_IFUNC);
	      if (ifunc == NULL)
		return false;
	    }
	}

      tls_type = 0;
      switch (r_type)
	{
	case R_PPC64_TLSGD:
	case R_PPC64_TLSLD:
	  /* These special tls relocs tie a call to __tls_get_addr with
	     its parameter symbol.  */
	  if (h != NULL)
	    ppc_elf_hash_entry (h)->tls_mask |= TLS_TLS | TLS_MARK;
	  else
	    if (!update_local_sym_info (abfd, symtab_hdr, r_symndx,
					rel->r_addend,
					NON_GOT | TLS_TLS | TLS_MARK))
	      return false;
	  sec->has_tls_reloc = 1;
	  break;

	case R_PPC64_GOT_TLSLD16:
	case R_PPC64_GOT_TLSLD16_LO:
	case R_PPC64_GOT_TLSLD16_HI:
	case R_PPC64_GOT_TLSLD16_HA:
	case R_PPC64_GOT_TLSLD_PCREL34:
	  tls_type = TLS_TLS | TLS_LD;
	  goto dogottls;

	case R_PPC64_GOT_TLSGD16:
	case R_PPC64_GOT_TLSGD16_LO:
	case R_PPC64_GOT_TLSGD16_HI:
	case R_PPC64_GOT_TLSGD16_HA:
	case R_PPC64_GOT_TLSGD_PCREL34:
	  tls_type = TLS_TLS | TLS_GD;
	  goto dogottls;

	case R_PPC64_GOT_TPREL16_DS:
	case R_PPC64_GOT_TPREL16_LO_DS:
	case R_PPC64_GOT_TPREL16_HI:
	case R_PPC64_GOT_TPREL16_HA:
	case R_PPC64_GOT_TPREL_PCREL34:
	  if (bfd_link_dll (info))
	    info->flags |= DF_STATIC_TLS;
	  tls_type = TLS_TLS | TLS_TPREL;
	  goto dogottls;

	case R_PPC64_GOT_DTPREL16_DS:
	case R_PPC64_GOT_DTPREL16_LO_DS:
	case R_PPC64_GOT_DTPREL16_HI:
	case R_PPC64_GOT_DTPREL16_HA:
	case R_PPC64_GOT_DTPREL_PCREL34:
	  tls_type = TLS_TLS | TLS_DTPREL;
	dogottls:
	  sec->has_tls_reloc = 1;
	  goto dogot;

	case R_PPC64_GOT16:
	case R_PPC64_GOT16_LO:
	case R_PPC64_GOT16_HI:
	case R_PPC64_GOT16_HA:
	case R_PPC64_GOT16_DS:
	case R_PPC64_GOT16_LO_DS:
	case R_PPC64_GOT_PCREL34:
	dogot:
	  /* This symbol requires a global offset table entry.  */
	  sec->has_toc_reloc = 1;
	  if (r_type == R_PPC64_GOT_TLSLD16
	      || r_type == R_PPC64_GOT_TLSGD16
	      || r_type == R_PPC64_GOT_TPREL16_DS
	      || r_type == R_PPC64_GOT_DTPREL16_DS
	      || r_type == R_PPC64_GOT16
	      || r_type == R_PPC64_GOT16_DS)
	    {
	      htab->do_multi_toc = 1;
	      ppc64_elf_tdata (abfd)->has_small_toc_reloc = 1;
	    }

	  if (ppc64_elf_tdata (abfd)->got == NULL
	      && !create_got_section (abfd, info))
	    return false;

	  if (h != NULL)
	    {
	      struct ppc_link_hash_entry *eh;
	      struct got_entry *ent;

	      eh = ppc_elf_hash_entry (h);
	      for (ent = eh->elf.got.glist; ent != NULL; ent = ent->next)
		if (ent->addend == rel->r_addend
		    && ent->owner == abfd
		    && ent->tls_type == tls_type)
		  break;
	      if (ent == NULL)
		{
		  size_t amt = sizeof (*ent);
		  ent = bfd_alloc (abfd, amt);
		  if (ent == NULL)
		    return false;
		  ent->next = eh->elf.got.glist;
		  ent->addend = rel->r_addend;
		  ent->owner = abfd;
		  ent->tls_type = tls_type;
		  ent->is_indirect = false;
		  ent->got.refcount = 0;
		  eh->elf.got.glist = ent;
		}
	      ent->got.refcount += 1;
	      eh->tls_mask |= tls_type;
	    }
	  else
	    /* This is a global offset table entry for a local symbol.  */
	    if (!update_local_sym_info (abfd, symtab_hdr, r_symndx,
					rel->r_addend, tls_type))
	      return false;
	  break;

	case R_PPC64_PLT16_HA:
	case R_PPC64_PLT16_HI:
	case R_PPC64_PLT16_LO:
	case R_PPC64_PLT16_LO_DS:
	case R_PPC64_PLT_PCREL34:
	case R_PPC64_PLT_PCREL34_NOTOC:
	case R_PPC64_PLT32:
	case R_PPC64_PLT64:
	  /* This symbol requires a procedure linkage table entry.  */
	  plt_list = ifunc;
	  if (h != NULL)
	    {
	      h->needs_plt = 1;
	      if (h->root.root.string[0] == '.'
		  && h->root.root.string[1] != '\0')
		ppc_elf_hash_entry (h)->is_func = 1;
	      ppc_elf_hash_entry (h)->tls_mask |= PLT_KEEP;
	      plt_list = &h->plt.plist;
	    }
	  if (plt_list == NULL)
	    plt_list = update_local_sym_info (abfd, symtab_hdr, r_symndx,
					      rel->r_addend,
					      NON_GOT | PLT_KEEP);
	  if (!update_plt_info (abfd, plt_list, rel->r_addend))
	    return false;
	  break;

	  /* The following relocations don't need to propagate the
	     relocation if linking a shared object since they are
	     section relative.  */
	case R_PPC64_SECTOFF:
	case R_PPC64_SECTOFF_LO:
	case R_PPC64_SECTOFF_HI:
	case R_PPC64_SECTOFF_HA:
	case R_PPC64_SECTOFF_DS:
	case R_PPC64_SECTOFF_LO_DS:
	case R_PPC64_DTPREL16:
	case R_PPC64_DTPREL16_LO:
	case R_PPC64_DTPREL16_HI:
	case R_PPC64_DTPREL16_HA:
	case R_PPC64_DTPREL16_DS:
	case R_PPC64_DTPREL16_LO_DS:
	case R_PPC64_DTPREL16_HIGH:
	case R_PPC64_DTPREL16_HIGHA:
	case R_PPC64_DTPREL16_HIGHER:
	case R_PPC64_DTPREL16_HIGHERA:
	case R_PPC64_DTPREL16_HIGHEST:
	case R_PPC64_DTPREL16_HIGHESTA:
	  break;

	  /* Nor do these.  */
	case R_PPC64_REL16:
	case R_PPC64_REL16_LO:
	case R_PPC64_REL16_HI:
	case R_PPC64_REL16_HA:
	case R_PPC64_REL16_HIGH:
	case R_PPC64_REL16_HIGHA:
	case R_PPC64_REL16_HIGHER:
	case R_PPC64_REL16_HIGHERA:
	case R_PPC64_REL16_HIGHEST:
	case R_PPC64_REL16_HIGHESTA:
	case R_PPC64_REL16_HIGHER34:
	case R_PPC64_REL16_HIGHERA34:
	case R_PPC64_REL16_HIGHEST34:
	case R_PPC64_REL16_HIGHESTA34:
	case R_PPC64_REL16DX_HA:
	  break;

	  /* Not supported as a dynamic relocation.  */
	case R_PPC64_ADDR64_LOCAL:
	  if (bfd_link_pic (info))
	    {
	      if (!ppc64_elf_howto_table[R_PPC64_ADDR32])
		ppc_howto_init ();
	      /* xgettext:c-format */
	      info->callbacks->einfo (_("%H: %s reloc unsupported "
					"in shared libraries and PIEs\n"),
				      abfd, sec, rel->r_offset,
				      ppc64_elf_howto_table[r_type]->name);
	      bfd_set_error (bfd_error_bad_value);
	      return false;
	    }
	  break;

	case R_PPC64_TOC16:
	case R_PPC64_TOC16_DS:
	  htab->do_multi_toc = 1;
	  ppc64_elf_tdata (abfd)->has_small_toc_reloc = 1;
	  /* Fall through.  */
	case R_PPC64_TOC16_LO:
	case R_PPC64_TOC16_HI:
	case R_PPC64_TOC16_HA:
	case R_PPC64_TOC16_LO_DS:
	  sec->has_toc_reloc = 1;
	  if (h != NULL && bfd_link_executable (info))
	    {
	      /* We may need a copy reloc.  */
	      h->non_got_ref = 1;
	      /* Strongly prefer a copy reloc over a dynamic reloc.
		 glibc ld.so as of 2019-08 will error out if one of
		 these relocations is emitted.  */
	      h->needs_copy = 1;
	      goto dodyn;
	    }
	  break;

	  /* Marker reloc.  */
	case R_PPC64_ENTRY:
	  break;

	  /* This relocation describes the C++ object vtable hierarchy.
	     Reconstruct it for later use during GC.  */
	case R_PPC64_GNU_VTINHERIT:
	  if (!bfd_elf_gc_record_vtinherit (abfd, sec, h, rel->r_offset))
	    return false;
	  break;

	  /* This relocation describes which C++ vtable entries are actually
	     used.  Record for later use during GC.  */
	case R_PPC64_GNU_VTENTRY:
	  if (!bfd_elf_gc_record_vtentry (abfd, sec, h, rel->r_addend))
	    return false;
	  break;

	case R_PPC64_REL14:
	case R_PPC64_REL14_BRTAKEN:
	case R_PPC64_REL14_BRNTAKEN:
	  {
	    asection *dest = NULL;

	    /* Heuristic: If jumping outside our section, chances are
	       we are going to need a stub.  */
	    if (h != NULL)
	      {
		/* If the sym is weak it may be overridden later, so
		   don't assume we know where a weak sym lives.  */
		if (h->root.type == bfd_link_hash_defined)
		  dest = h->root.u.def.section;
	      }
	    else
	      dest = bfd_section_from_elf_index (abfd, isym->st_shndx);

	    if (dest != sec)
	      ppc64_elf_section_data (sec)->has_14bit_branch = 1;
	  }
	  goto rel24;

	case R_PPC64_PLTCALL:
	case R_PPC64_PLTCALL_NOTOC:
	  ppc64_elf_section_data (sec)->has_pltcall = 1;
	  /* Fall through.  */

	case R_PPC64_REL24:
	case R_PPC64_REL24_NOTOC:
	case R_PPC64_REL24_P9NOTOC:
	rel24:
	  plt_list = ifunc;
	  if (h != NULL)
	    {
	      h->needs_plt = 1;
	      if (h->root.root.string[0] == '.'
		  && h->root.root.string[1] != '\0')
		ppc_elf_hash_entry (h)->is_func = 1;

	      if (h == tga || h == dottga)
		{
		  sec->has_tls_reloc = 1;
		  if (rel != relocs
		      && (ELF64_R_TYPE (rel[-1].r_info) == R_PPC64_TLSGD
			  || ELF64_R_TYPE (rel[-1].r_info) == R_PPC64_TLSLD))
		    /* We have a new-style __tls_get_addr call with
		       a marker reloc.  */
		    ;
		  else
		    /* Mark this section as having an old-style call.  */
		    sec->nomark_tls_get_addr = 1;
		}
	      plt_list = &h->plt.plist;
	    }

	  /* We may need a .plt entry if the function this reloc
	     refers to is in a shared lib.  */
	  if (plt_list
	      && !update_plt_info (abfd, plt_list, rel->r_addend))
	    return false;
	  break;

	case R_PPC64_ADDR14:
	case R_PPC64_ADDR14_BRNTAKEN:
	case R_PPC64_ADDR14_BRTAKEN:
	case R_PPC64_ADDR24:
	  goto dodyn;

	case R_PPC64_TPREL64:
	  tls_type = TLS_EXPLICIT | TLS_TLS | TLS_TPREL;
	  if (bfd_link_dll (info))
	    info->flags |= DF_STATIC_TLS;
	  goto dotlstoc;

	case R_PPC64_DTPMOD64:
	  if (rel + 1 < rel_end
	      && rel[1].r_info == ELF64_R_INFO (r_symndx, R_PPC64_DTPREL64)
	      && rel[1].r_offset == rel->r_offset + 8)
	    tls_type = TLS_EXPLICIT | TLS_TLS | TLS_GD;
	  else
	    tls_type = TLS_EXPLICIT | TLS_TLS | TLS_LD;
	  goto dotlstoc;

	case R_PPC64_DTPREL64:
	  tls_type = TLS_EXPLICIT | TLS_TLS | TLS_DTPREL;
	  if (rel != relocs
	      && rel[-1].r_info == ELF64_R_INFO (r_symndx, R_PPC64_DTPMOD64)
	      && rel[-1].r_offset == rel->r_offset - 8)
	    /* This is the second reloc of a dtpmod, dtprel pair.
	       Don't mark with TLS_DTPREL.  */
	    goto dodyn;

	dotlstoc:
	  sec->has_tls_reloc = 1;
	  if (h != NULL)
	    ppc_elf_hash_entry (h)->tls_mask |= tls_type & 0xff;
	  else
	    if (!update_local_sym_info (abfd, symtab_hdr, r_symndx,
					rel->r_addend, tls_type))
	      return false;

	  ppc64_sec = ppc64_elf_section_data (sec);
	  if (ppc64_sec->sec_type == sec_normal)
	    {
	      bfd_size_type amt;

	      /* One extra to simplify get_tls_mask.  */
	      amt = sec->size * sizeof (unsigned) / 8 + sizeof (unsigned);
	      ppc64_sec->u.toc.symndx = bfd_zalloc (abfd, amt);
	      if (ppc64_sec->u.toc.symndx == NULL)
		return false;
	      amt = sec->size * sizeof (bfd_vma) / 8;
	      ppc64_sec->u.toc.add = bfd_zalloc (abfd, amt);
	      if (ppc64_sec->u.toc.add == NULL)
		return false;
	      ppc64_sec->sec_type = sec_toc;
	    }
	  if (ppc64_sec->sec_type != sec_toc
	      || rel->r_offset % 8 != 0)
	    {
	      info->callbacks->einfo (_("%H: %s reloc unsupported here\n"),
				      abfd, sec, rel->r_offset,
				      ppc64_elf_howto_table[r_type]->name);
	      bfd_set_error (bfd_error_bad_value);
	      return false;
	    }
	  ppc64_sec->u.toc.symndx[rel->r_offset / 8] = r_symndx;
	  ppc64_sec->u.toc.add[rel->r_offset / 8] = rel->r_addend;

	  /* Mark the second slot of a GD or LD entry.
	     -1 to indicate GD and -2 to indicate LD.  */
	  if (tls_type == (TLS_EXPLICIT | TLS_TLS | TLS_GD))
	    ppc64_sec->u.toc.symndx[rel->r_offset / 8 + 1] = -1;
	  else if (tls_type == (TLS_EXPLICIT | TLS_TLS | TLS_LD))
	    ppc64_sec->u.toc.symndx[rel->r_offset / 8 + 1] = -2;
	  goto dodyn;

	case R_PPC64_TPREL16_HI:
	case R_PPC64_TPREL16_HA:
	case R_PPC64_TPREL16_HIGH:
	case R_PPC64_TPREL16_HIGHA:
	case R_PPC64_TPREL16_HIGHER:
	case R_PPC64_TPREL16_HIGHERA:
	case R_PPC64_TPREL16_HIGHEST:
	case R_PPC64_TPREL16_HIGHESTA:
	  sec->has_tls_reloc = 1;
	  /* Fall through.  */
	case R_PPC64_TPREL34:
	case R_PPC64_TPREL16:
	case R_PPC64_TPREL16_DS:
	case R_PPC64_TPREL16_LO:
	case R_PPC64_TPREL16_LO_DS:
	  if (bfd_link_dll (info))
	    info->flags |= DF_STATIC_TLS;
	  goto dodyn;

	case R_PPC64_ADDR64:
	  if (is_opd
	      && rel + 1 < rel_end
	      && ELF64_R_TYPE ((rel + 1)->r_info) == R_PPC64_TOC)
	    {
	      if (h != NULL)
		ppc_elf_hash_entry (h)->is_func = 1;
	    }
	  /* Fall through.  */

	case R_PPC64_ADDR16:
	case R_PPC64_ADDR16_DS:
	case R_PPC64_ADDR16_HA:
	case R_PPC64_ADDR16_HI:
	case R_PPC64_ADDR16_HIGH:
	case R_PPC64_ADDR16_HIGHA:
	case R_PPC64_ADDR16_HIGHER:
	case R_PPC64_ADDR16_HIGHERA:
	case R_PPC64_ADDR16_HIGHEST:
	case R_PPC64_ADDR16_HIGHESTA:
	case R_PPC64_ADDR16_LO:
	case R_PPC64_ADDR16_LO_DS:
	case R_PPC64_D34:
	case R_PPC64_D34_LO:
	case R_PPC64_D34_HI30:
	case R_PPC64_D34_HA30:
	case R_PPC64_ADDR16_HIGHER34:
	case R_PPC64_ADDR16_HIGHERA34:
	case R_PPC64_ADDR16_HIGHEST34:
	case R_PPC64_ADDR16_HIGHESTA34:
	case R_PPC64_D28:
	  if (h != NULL && !bfd_link_pic (info) && abiversion (abfd) != 1
	      && rel->r_addend == 0)
	    {
	      /* We may need a .plt entry if this reloc refers to a
		 function in a shared lib.  */
	      if (!update_plt_info (abfd, &h->plt.plist, 0))
		return false;
	      h->pointer_equality_needed = 1;
	    }
	  /* Fall through.  */

	case R_PPC64_REL30:
	case R_PPC64_REL32:
	case R_PPC64_REL64:
	case R_PPC64_ADDR32:
	case R_PPC64_UADDR16:
	case R_PPC64_UADDR32:
	case R_PPC64_UADDR64:
	case R_PPC64_TOC:
	  if (h != NULL && bfd_link_executable (info))
	    /* We may need a copy reloc.  */
	    h->non_got_ref = 1;

	  /* Don't propagate .opd relocs.  */
	  if (NO_OPD_RELOCS && is_opd)
	    break;

	  /* Set up information for symbols that might need dynamic
	     relocations.  At this point in linking we have read all
	     the input files and resolved most symbols, but have not
	     yet decided whether symbols are dynamic or finalized
	     symbol flags.  In some cases we might be setting dynamic
	     reloc info for symbols that do not end up needing such.
	     That's OK, adjust_dynamic_symbol and allocate_dynrelocs
	     work together with this code.  */
	dodyn:
	  if ((h != NULL
	       && !SYMBOL_REFERENCES_LOCAL (info, h))
	      || (bfd_link_pic (info)
		  && (h != NULL
		      ? !bfd_is_abs_symbol (&h->root)
		      : isym->st_shndx != SHN_ABS)
		  && must_be_dyn_reloc (info, r_type))
	      || (!bfd_link_pic (info)
		  && ifunc != NULL))
	    {
	      /* We must copy these reloc types into the output file.
		 Create a reloc section in dynobj and make room for
		 this reloc.  */
	      if (sreloc == NULL)
		{
		  sreloc = _bfd_elf_make_dynamic_reloc_section
		    (sec, htab->elf.dynobj, 3, abfd, /*rela?*/ true);

		  if (sreloc == NULL)
		    return false;
		}

	      /* If this is a global symbol, we count the number of
		 relocations we need for this symbol.  */
	      if (h != NULL)
		{
		  struct ppc_dyn_relocs *p;
		  struct ppc_dyn_relocs **head;

		  head = (struct ppc_dyn_relocs **) &h->dyn_relocs;
		  p = *head;
		  if (p == NULL || p->sec != sec)
		    {
		      p = bfd_alloc (htab->elf.dynobj, sizeof *p);
		      if (p == NULL)
			return false;
		      p->next = *head;
		      *head = p;
		      p->sec = sec;
		      p->count = 0;
		      p->pc_count = 0;
		      p->rel_count = 0;
		    }
		  p->count += 1;
		  if (!must_be_dyn_reloc (info, r_type))
		    p->pc_count += 1;
		  if ((r_type == R_PPC64_ADDR64 || r_type == R_PPC64_TOC)
		      && rel->r_offset % 2 == 0
		      && sec->alignment_power != 0)
		    p->rel_count += 1;
		}
	      else
		{
		  /* Track dynamic relocs needed for local syms too.  */
		  struct ppc_local_dyn_relocs *p;
		  struct ppc_local_dyn_relocs **head;
		  bool is_ifunc;
		  asection *s;
		  void *vpp;

		  s = bfd_section_from_elf_index (abfd, isym->st_shndx);
		  if (s == NULL)
		    s = sec;

		  vpp = &elf_section_data (s)->local_dynrel;
		  head = (struct ppc_local_dyn_relocs **) vpp;
		  is_ifunc = ELF_ST_TYPE (isym->st_info) == STT_GNU_IFUNC;
		  p = *head;
		  if (p != NULL && p->sec == sec && p->ifunc != is_ifunc)
		    p = p->next;
		  if (p == NULL || p->sec != sec || p->ifunc != is_ifunc)
		    {
		      p = bfd_alloc (htab->elf.dynobj, sizeof *p);
		      if (p == NULL)
			return false;
		      p->next = *head;
		      *head = p;
		      p->sec = sec;
		      p->count = 0;
		      p->rel_count = 0;
		      p->ifunc = is_ifunc;
		    }
		  p->count += 1;
		  if ((r_type == R_PPC64_ADDR64 || r_type == R_PPC64_TOC)
		      && rel->r_offset % 2 == 0
		      && sec->alignment_power != 0)
		    p->rel_count += 1;
		}
	    }
	  break;

	default:
	  break;
	}
    }

  return true;
}

/* Merge backend specific data from an object file to the output
   object file when linking.  */

static bool
ppc64_elf_merge_private_bfd_data (bfd *ibfd, struct bfd_link_info *info)
{
  bfd *obfd = info->output_bfd;
  unsigned long iflags, oflags;

  if ((ibfd->flags & BFD_LINKER_CREATED) != 0)
    return true;

  if (!is_ppc64_elf (ibfd) || !is_ppc64_elf (obfd))
    return true;

  if (!_bfd_generic_verify_endian_match (ibfd, info))
    return false;

  iflags = elf_elfheader (ibfd)->e_flags;
  oflags = elf_elfheader (obfd)->e_flags;

  if (iflags & ~EF_PPC64_ABI)
    {
      _bfd_error_handler
	/* xgettext:c-format */
	(_("%pB uses unknown e_flags 0x%lx"), ibfd, iflags);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }
  else if (iflags != oflags && iflags != 0)
    {
      _bfd_error_handler
	/* xgettext:c-format */
	(_("%pB: ABI version %ld is not compatible with ABI version %ld output"),
	 ibfd, iflags, oflags);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  if (!_bfd_elf_ppc_merge_fp_attributes (ibfd, info))
    return false;

  /* Merge Tag_compatibility attributes and any common GNU ones.  */
  return _bfd_elf_merge_object_attributes (ibfd, info);
}

static bool
ppc64_elf_print_private_bfd_data (bfd *abfd, void *ptr)
{
  /* Print normal ELF private data.  */
  _bfd_elf_print_private_bfd_data (abfd, ptr);

  if (elf_elfheader (abfd)->e_flags != 0)
    {
      FILE *file = ptr;

      fprintf (file, _("private flags = 0x%lx:"),
	       elf_elfheader (abfd)->e_flags);

      if ((elf_elfheader (abfd)->e_flags & EF_PPC64_ABI) != 0)
	fprintf (file, _(" [abiv%ld]"),
		 elf_elfheader (abfd)->e_flags & EF_PPC64_ABI);
      fputc ('\n', file);
    }

  return true;
}

/* OFFSET in OPD_SEC specifies a function descriptor.  Return the address
   of the code entry point, and its section, which must be in the same
   object as OPD_SEC.  Returns (bfd_vma) -1 on error.  */

static bfd_vma
opd_entry_value (asection *opd_sec,
		 bfd_vma offset,
		 asection **code_sec,
		 bfd_vma *code_off,
		 bool in_code_sec)
{
  bfd *opd_bfd = opd_sec->owner;
  Elf_Internal_Rela *relocs;
  Elf_Internal_Rela *lo, *hi, *look;
  bfd_vma val;

  if (!is_ppc64_elf (opd_bfd))
    return (bfd_vma) -1;

  if (ppc64_elf_section_data (opd_sec)->sec_type == sec_normal)
    ppc64_elf_section_data (opd_sec)->sec_type = sec_opd;
  else if (ppc64_elf_section_data (opd_sec)->sec_type != sec_opd)
    return (bfd_vma) -1;

  /* No relocs implies we are linking a --just-symbols object, or looking
     at a final linked executable with addr2line or somesuch.  */
  if (opd_sec->reloc_count == 0)
    {
      bfd_byte *contents = ppc64_elf_section_data (opd_sec)->u.opd.u.contents;

      if (contents == NULL)
	{
	  if ((opd_sec->flags & SEC_HAS_CONTENTS) == 0
	      || !bfd_malloc_and_get_section (opd_bfd, opd_sec, &contents))
	    return (bfd_vma) -1;
	  ppc64_elf_section_data (opd_sec)->u.opd.u.contents = contents;
	}

      /* PR 17512: file: 64b9dfbb.  */
      if (offset + 7 >= opd_sec->size || offset + 7 < offset)
	return (bfd_vma) -1;

      val = bfd_get_64 (opd_bfd, contents + offset);
      if (code_sec != NULL)
	{
	  asection *sec, *likely = NULL;

	  if (in_code_sec)
	    {
	      sec = *code_sec;
	      if (sec->vma <= val
		  && val < sec->vma + sec->size)
		likely = sec;
	      else
		val = -1;
	    }
	  else
	    for (sec = opd_bfd->sections; sec != NULL; sec = sec->next)
	      if (sec->vma <= val
		  && (sec->flags & SEC_LOAD) != 0
		  && (sec->flags & SEC_ALLOC) != 0)
		likely = sec;
	  if (likely != NULL)
	    {
	      *code_sec = likely;
	      if (code_off != NULL)
		*code_off = val - likely->vma;
	    }
	}
      return val;
    }

  relocs = ppc64_elf_section_data (opd_sec)->u.opd.u.relocs;
  if (relocs == NULL)
    relocs = _bfd_elf_link_read_relocs (opd_bfd, opd_sec, NULL, NULL, true);
  /* PR 17512: file: df8e1fd6.  */
  if (relocs == NULL)
    return (bfd_vma) -1;

  /* Go find the opd reloc at the sym address.  */
  lo = relocs;
  hi = lo + opd_sec->reloc_count - 1; /* ignore last reloc */
  val = (bfd_vma) -1;
  while (lo < hi)
    {
      look = lo + (hi - lo) / 2;
      if (look->r_offset < offset)
	lo = look + 1;
      else if (look->r_offset > offset)
	hi = look;
      else
	{
	  Elf_Internal_Shdr *symtab_hdr = &elf_symtab_hdr (opd_bfd);

	  if (ELF64_R_TYPE (look->r_info) == R_PPC64_ADDR64
	      && ELF64_R_TYPE ((look + 1)->r_info) == R_PPC64_TOC)
	    {
	      unsigned long symndx = ELF64_R_SYM (look->r_info);
	      asection *sec = NULL;

	      if (symndx >= symtab_hdr->sh_info
		  && elf_sym_hashes (opd_bfd) != NULL)
		{
		  struct elf_link_hash_entry **sym_hashes;
		  struct elf_link_hash_entry *rh;

		  sym_hashes = elf_sym_hashes (opd_bfd);
		  rh = sym_hashes[symndx - symtab_hdr->sh_info];
		  if (rh != NULL)
		    {
		      rh = elf_follow_link (rh);
		      if (rh->root.type != bfd_link_hash_defined
			  && rh->root.type != bfd_link_hash_defweak)
			break;
		      if (rh->root.u.def.section->owner == opd_bfd)
			{
			  val = rh->root.u.def.value;
			  sec = rh->root.u.def.section;
			}
		    }
		}

	      if (sec == NULL)
		{
		  Elf_Internal_Sym *sym;

		  if (symndx < symtab_hdr->sh_info)
		    {
		      sym = (Elf_Internal_Sym *) symtab_hdr->contents;
		      if (sym == NULL)
			{
			  size_t symcnt = symtab_hdr->sh_info;
			  sym = bfd_elf_get_elf_syms (opd_bfd, symtab_hdr,
						      symcnt, 0,
						      NULL, NULL, NULL);
			  if (sym == NULL)
			    break;
			  symtab_hdr->contents = (bfd_byte *) sym;
			}
		      sym += symndx;
		    }
		  else
		    {
		      sym = bfd_elf_get_elf_syms (opd_bfd, symtab_hdr,
						  1, symndx,
						  NULL, NULL, NULL);
		      if (sym == NULL)
			break;
		    }
		  sec = bfd_section_from_elf_index (opd_bfd, sym->st_shndx);
		  if (sec == NULL)
		    break;
		  BFD_ASSERT ((sec->flags & SEC_MERGE) == 0);
		  val = sym->st_value;
		}

	      val += look->r_addend;
	      if (code_off != NULL)
		*code_off = val;
	      if (code_sec != NULL)
		{
		  if (in_code_sec && *code_sec != sec)
		    return -1;
		  else
		    *code_sec = sec;
		}
	      if (sec->output_section != NULL)
		val += sec->output_section->vma + sec->output_offset;
	    }
	  break;
	}
    }

  return val;
}

/* If the ELF symbol SYM might be a function in SEC, return the
   function size and set *CODE_OFF to the function's entry point,
   otherwise return zero.  */

static bfd_size_type
ppc64_elf_maybe_function_sym (const asymbol *sym, asection *sec,
			      bfd_vma *code_off)
{
  bfd_size_type size;
  elf_symbol_type * elf_sym = (elf_symbol_type *) sym;

  if ((sym->flags & (BSF_SECTION_SYM | BSF_FILE | BSF_OBJECT
		     | BSF_THREAD_LOCAL | BSF_RELC | BSF_SRELC)) != 0)
    return 0;

  size = (sym->flags & BSF_SYNTHETIC) ? 0 : elf_sym->internal_elf_sym.st_size;

  /* In theory we should check that the symbol's type satisfies
     _bfd_elf_is_function_type(), but there are some function-like
     symbols which would fail this test.  (eg _start).  Instead
     we check for hidden, local, notype symbols with zero size.
     This type of symbol is generated by the annobin plugin for gcc
     and clang, and should not be considered to be a function symbol.  */
  if (size == 0
      && ((sym->flags & (BSF_SYNTHETIC | BSF_LOCAL)) == BSF_LOCAL)
      && ELF_ST_TYPE (elf_sym->internal_elf_sym.st_info) == STT_NOTYPE
      && ELF_ST_VISIBILITY (elf_sym->internal_elf_sym.st_other) == STV_HIDDEN)
    return 0;

  if (strcmp (sym->section->name, ".opd") == 0)
    {
      struct _opd_sec_data *opd = get_opd_info (sym->section);
      bfd_vma symval = sym->value;

      if (opd != NULL
	  && opd->adjust != NULL
	  && elf_section_data (sym->section)->relocs != NULL)
	{
	  /* opd_entry_value will use cached relocs that have been
	     adjusted, but with raw symbols.  That means both local
	     and global symbols need adjusting.  */
	  long adjust = opd->adjust[OPD_NDX (symval)];
	  if (adjust == -1)
	    return 0;
	  symval += adjust;
	}

      if (opd_entry_value (sym->section, symval,
			   &sec, code_off, true) == (bfd_vma) -1)
	return 0;
      /* An old ABI binary with dot-syms has a size of 24 on the .opd
	 symbol.  This size has nothing to do with the code size of the
	 function, which is what we're supposed to return, but the
	 code size isn't available without looking up the dot-sym.
	 However, doing that would be a waste of time particularly
	 since elf_find_function will look at the dot-sym anyway.
	 Now, elf_find_function will keep the largest size of any
	 function sym found at the code address of interest, so return
	 1 here to avoid it incorrectly caching a larger function size
	 for a small function.  This does mean we return the wrong
	 size for a new-ABI function of size 24, but all that does is
	 disable caching for such functions.  */
      if (size == 24)
	size = 1;
    }
  else
    {
      if (sym->section != sec)
	return 0;
      *code_off = sym->value;
    }

  /* Do not return 0 for the function's size.  */
  return size ? size : 1;
}

/* Return true if symbol is a strong function defined in an ELFv2
   object with st_other localentry bits of zero, ie. its local entry
   point coincides with its global entry point.  */

static bool
is_elfv2_localentry0 (struct elf_link_hash_entry *h)
{
  return (h != NULL
	  && h->type == STT_FUNC
	  && h->root.type == bfd_link_hash_defined
	  && (STO_PPC64_LOCAL_MASK & h->other) == 0
	  && !ppc_elf_hash_entry (h)->non_zero_localentry
	  && is_ppc64_elf (h->root.u.def.section->owner)
	  && abiversion (h->root.u.def.section->owner) >= 2);
}

/* Return true if symbol is defined in a regular object file.  */

static bool
is_static_defined (struct elf_link_hash_entry *h)
{
  return ((h->root.type == bfd_link_hash_defined
	   || h->root.type == bfd_link_hash_defweak)
	  && h->root.u.def.section != NULL
	  && h->root.u.def.section->output_section != NULL);
}

/* If FDH is a function descriptor symbol, return the associated code
   entry symbol if it is defined.  Return NULL otherwise.  */

static struct ppc_link_hash_entry *
defined_code_entry (struct ppc_link_hash_entry *fdh)
{
  if (fdh->is_func_descriptor)
    {
      struct ppc_link_hash_entry *fh = ppc_follow_link (fdh->oh);
      if (fh->elf.root.type == bfd_link_hash_defined
	  || fh->elf.root.type == bfd_link_hash_defweak)
	return fh;
    }
  return NULL;
}

/* If FH is a function code entry symbol, return the associated
   function descriptor symbol if it is defined.  Return NULL otherwise.  */

static struct ppc_link_hash_entry *
defined_func_desc (struct ppc_link_hash_entry *fh)
{
  if (fh->oh != NULL
      && fh->oh->is_func_descriptor)
    {
      struct ppc_link_hash_entry *fdh = ppc_follow_link (fh->oh);
      if (fdh->elf.root.type == bfd_link_hash_defined
	  || fdh->elf.root.type == bfd_link_hash_defweak)
	return fdh;
    }
  return NULL;
}

/* Given H is a symbol that satisfies is_static_defined, return the
   value in the output file.  */

static bfd_vma
defined_sym_val (struct elf_link_hash_entry *h)
{
  return (h->root.u.def.section->output_section->vma
	  + h->root.u.def.section->output_offset
	  + h->root.u.def.value);
}

/* Return true if H matches __tls_get_addr or one of its variants.  */

static bool
is_tls_get_addr (struct elf_link_hash_entry *h,
		 struct ppc_link_hash_table *htab)
{
  return (h == elf_hash_entry (htab->tls_get_addr_fd)
	  || h == elf_hash_entry (htab->tga_desc_fd)
	  || h == elf_hash_entry (htab->tls_get_addr)
	  || h == elf_hash_entry (htab->tga_desc));
}

static bool func_desc_adjust (struct elf_link_hash_entry *, void *);

/* Garbage collect sections, after first dealing with dot-symbols.  */

static bool
ppc64_elf_gc_sections (bfd *abfd, struct bfd_link_info *info)
{
  struct ppc_link_hash_table *htab = ppc_hash_table (info);

  if (htab != NULL && htab->need_func_desc_adj)
    {
      elf_link_hash_traverse (&htab->elf, func_desc_adjust, info);
      htab->need_func_desc_adj = 0;
    }
  return bfd_elf_gc_sections (abfd, info);
}

/* Mark all our entry sym sections, both opd and code section.  */

static void
ppc64_elf_gc_keep (struct bfd_link_info *info)
{
  struct ppc_link_hash_table *htab = ppc_hash_table (info);
  struct bfd_sym_chain *sym;

  if (htab == NULL)
    return;

  for (sym = info->gc_sym_list; sym != NULL; sym = sym->next)
    {
      struct ppc_link_hash_entry *eh, *fh;
      asection *sec;

      eh = ppc_elf_hash_entry (elf_link_hash_lookup (&htab->elf, sym->name,
						     false, false, true));
      if (eh == NULL)
	continue;
      if (eh->elf.root.type != bfd_link_hash_defined
	  && eh->elf.root.type != bfd_link_hash_defweak)
	continue;

      fh = defined_code_entry (eh);
      if (fh != NULL)
	{
	  sec = fh->elf.root.u.def.section;
	  sec->flags |= SEC_KEEP;
	}
      else if (get_opd_info (eh->elf.root.u.def.section) != NULL
	       && opd_entry_value (eh->elf.root.u.def.section,
				   eh->elf.root.u.def.value,
				   &sec, NULL, false) != (bfd_vma) -1)
	sec->flags |= SEC_KEEP;

      sec = eh->elf.root.u.def.section;
      sec->flags |= SEC_KEEP;
    }
}

/* Mark sections containing dynamically referenced symbols.  When
   building shared libraries, we must assume that any visible symbol is
   referenced.  */

static bool
ppc64_elf_gc_mark_dynamic_ref (struct elf_link_hash_entry *h, void *inf)
{
  struct bfd_link_info *info = (struct bfd_link_info *) inf;
  struct ppc_link_hash_entry *eh = ppc_elf_hash_entry (h);
  struct ppc_link_hash_entry *fdh;
  struct bfd_elf_dynamic_list *d = info->dynamic_list;

  /* Dynamic linking info is on the func descriptor sym.  */
  fdh = defined_func_desc (eh);
  if (fdh != NULL)
    eh = fdh;

  if ((eh->elf.root.type == bfd_link_hash_defined
       || eh->elf.root.type == bfd_link_hash_defweak)
      && (!eh->elf.start_stop
	  || eh->elf.root.ldscript_def
	  || !info->start_stop_gc)
      && ((eh->elf.ref_dynamic && !eh->elf.forced_local)
	  || ((eh->elf.def_regular || ELF_COMMON_DEF_P (&eh->elf))
	      && ELF_ST_VISIBILITY (eh->elf.other) != STV_INTERNAL
	      && ELF_ST_VISIBILITY (eh->elf.other) != STV_HIDDEN
	      && (!bfd_link_executable (info)
		  || info->gc_keep_exported
		  || info->export_dynamic
		  || (eh->elf.dynamic
		      && d != NULL
		      && (*d->match) (&d->head, NULL,
				      eh->elf.root.root.string)))
	      && (eh->elf.versioned >= versioned
		  || !bfd_hide_sym_by_version (info->version_info,
					       eh->elf.root.root.string)))))
    {
      asection *code_sec;
      struct ppc_link_hash_entry *fh;

      eh->elf.root.u.def.section->flags |= SEC_KEEP;

      /* Function descriptor syms cause the associated
	 function code sym section to be marked.  */
      fh = defined_code_entry (eh);
      if (fh != NULL)
	{
	  code_sec = fh->elf.root.u.def.section;
	  code_sec->flags |= SEC_KEEP;
	}
      else if (get_opd_info (eh->elf.root.u.def.section) != NULL
	       && opd_entry_value (eh->elf.root.u.def.section,
				   eh->elf.root.u.def.value,
				   &code_sec, NULL, false) != (bfd_vma) -1)
	code_sec->flags |= SEC_KEEP;
    }

  return true;
}

/* Return the section that should be marked against GC for a given
   relocation.  */

static asection *
ppc64_elf_gc_mark_hook (asection *sec,
			struct bfd_link_info *info,
			Elf_Internal_Rela *rel,
			struct elf_link_hash_entry *h,
			Elf_Internal_Sym *sym)
{
  asection *rsec;

  /* Syms return NULL if we're marking .opd, so we avoid marking all
     function sections, as all functions are referenced in .opd.  */
  rsec = NULL;
  if (get_opd_info (sec) != NULL)
    return rsec;

  if (h != NULL)
    {
      enum elf_ppc64_reloc_type r_type;
      struct ppc_link_hash_entry *eh, *fh, *fdh;

      r_type = ELF64_R_TYPE (rel->r_info);
      switch (r_type)
	{
	case R_PPC64_GNU_VTINHERIT:
	case R_PPC64_GNU_VTENTRY:
	  break;

	default:
	  switch (h->root.type)
	    {
	    case bfd_link_hash_defined:
	    case bfd_link_hash_defweak:
	      eh = ppc_elf_hash_entry (h);
	      fdh = defined_func_desc (eh);
	      if (fdh != NULL)
		{
		  /* -mcall-aixdesc code references the dot-symbol on
		     a call reloc.  Mark the function descriptor too
		     against garbage collection.  */
		  fdh->elf.mark = 1;
		  if (fdh->elf.is_weakalias)
		    weakdef (&fdh->elf)->mark = 1;
		  eh = fdh;
		}

	      /* Function descriptor syms cause the associated
		 function code sym section to be marked.  */
	      fh = defined_code_entry (eh);
	      if (fh != NULL)
		{
		  /* They also mark their opd section.  */
		  eh->elf.root.u.def.section->gc_mark = 1;

		  rsec = fh->elf.root.u.def.section;
		}
	      else if (get_opd_info (eh->elf.root.u.def.section) != NULL
		       && opd_entry_value (eh->elf.root.u.def.section,
					   eh->elf.root.u.def.value,
					   &rsec, NULL, false) != (bfd_vma) -1)
		eh->elf.root.u.def.section->gc_mark = 1;
	      else
		rsec = h->root.u.def.section;
	      break;

	    case bfd_link_hash_common:
	      rsec = h->root.u.c.p->section;
	      break;

	    default:
	      return _bfd_elf_gc_mark_hook (sec, info, rel, h, sym);
	    }
	}
    }
  else
    {
      struct _opd_sec_data *opd;

      rsec = bfd_section_from_elf_index (sec->owner, sym->st_shndx);
      opd = get_opd_info (rsec);
      if (opd != NULL && opd->func_sec != NULL)
	{
	  rsec->gc_mark = 1;

	  rsec = opd->func_sec[OPD_NDX (sym->st_value + rel->r_addend)];
	}
    }

  return rsec;
}

/* The maximum size of .sfpr.  */
#define SFPR_MAX (218*4)

struct sfpr_def_parms
{
  const char name[12];
  unsigned char lo, hi;
  bfd_byte *(*write_ent) (bfd *, bfd_byte *, int);
  bfd_byte *(*write_tail) (bfd *, bfd_byte *, int);
};

/* Auto-generate _save*, _rest* functions in .sfpr.
   If STUB_SEC is non-null, define alias symbols in STUB_SEC
   instead.  */

static bool
sfpr_define (struct bfd_link_info *info,
	     const struct sfpr_def_parms *parm,
	     asection *stub_sec)
{
  struct ppc_link_hash_table *htab = ppc_hash_table (info);
  unsigned int i;
  size_t len = strlen (parm->name);
  bool writing = false;
  char sym[16];

  if (htab == NULL)
    return false;

  memcpy (sym, parm->name, len);
  sym[len + 2] = 0;

  for (i = parm->lo; i <= parm->hi; i++)
    {
      struct ppc_link_hash_entry *h;

      sym[len + 0] = i / 10 + '0';
      sym[len + 1] = i % 10 + '0';
      h = ppc_elf_hash_entry (elf_link_hash_lookup (&htab->elf, sym,
						    writing, true, true));
      if (stub_sec != NULL)
	{
	  if (h != NULL
	      && h->elf.root.type == bfd_link_hash_defined
	      && h->elf.root.u.def.section == htab->sfpr)
	    {
	      struct elf_link_hash_entry *s;
	      char buf[32];
	      sprintf (buf, "%08x.%s", stub_sec->id & 0xffffffff, sym);
	      s = elf_link_hash_lookup (&htab->elf, buf, true, true, false);
	      if (s == NULL)
		return false;
	      if (s->root.type == bfd_link_hash_new)
		{
		  s->root.type = bfd_link_hash_defined;
		  s->root.u.def.section = stub_sec;
		  s->root.u.def.value = (stub_sec->size - htab->sfpr->size
					 + h->elf.root.u.def.value);
		  s->ref_regular = 1;
		  s->def_regular = 1;
		  s->ref_regular_nonweak = 1;
		  s->forced_local = 1;
		  s->non_elf = 0;
		  s->root.linker_def = 1;
		}
	    }
	  continue;
	}
      if (h != NULL)
	{
	  h->save_res = 1;
	  if (!h->elf.def_regular)
	    {
	      h->elf.root.type = bfd_link_hash_defined;
	      h->elf.root.u.def.section = htab->sfpr;
	      h->elf.root.u.def.value = htab->sfpr->size;
	      h->elf.type = STT_FUNC;
	      h->elf.def_regular = 1;
	      h->elf.non_elf = 0;
	      _bfd_elf_link_hash_hide_symbol (info, &h->elf, true);
	      writing = true;
	      if (htab->sfpr->contents == NULL)
		{
		  htab->sfpr->contents
		    = bfd_alloc (htab->elf.dynobj, SFPR_MAX);
		  if (htab->sfpr->contents == NULL)
		    return false;
		}
	    }
	}
      if (writing)
	{
	  bfd_byte *p = htab->sfpr->contents + htab->sfpr->size;
	  if (i != parm->hi)
	    p = (*parm->write_ent) (htab->elf.dynobj, p, i);
	  else
	    p = (*parm->write_tail) (htab->elf.dynobj, p, i);
	  htab->sfpr->size = p - htab->sfpr->contents;
	}
    }

  return true;
}

static bfd_byte *
savegpr0 (bfd *abfd, bfd_byte *p, int r)
{
  bfd_put_32 (abfd, STD_R0_0R1 + (r << 21) + (1 << 16) - (32 - r) * 8, p);
  return p + 4;
}

static bfd_byte *
savegpr0_tail (bfd *abfd, bfd_byte *p, int r)
{
  p = savegpr0 (abfd, p, r);
  bfd_put_32 (abfd, STD_R0_0R1 + STK_LR, p);
  p = p + 4;
  bfd_put_32 (abfd, BLR, p);
  return p + 4;
}

static bfd_byte *
restgpr0 (bfd *abfd, bfd_byte *p, int r)
{
  bfd_put_32 (abfd, LD_R0_0R1 + (r << 21) + (1 << 16) - (32 - r) * 8, p);
  return p + 4;
}

static bfd_byte *
restgpr0_tail (bfd *abfd, bfd_byte *p, int r)
{
  bfd_put_32 (abfd, LD_R0_0R1 + STK_LR, p);
  p = p + 4;
  p = restgpr0 (abfd, p, r);
  bfd_put_32 (abfd, MTLR_R0, p);
  p = p + 4;
  if (r == 29)
    {
      p = restgpr0 (abfd, p, 30);
      p = restgpr0 (abfd, p, 31);
    }
  bfd_put_32 (abfd, BLR, p);
  return p + 4;
}

static bfd_byte *
savegpr1 (bfd *abfd, bfd_byte *p, int r)
{
  bfd_put_32 (abfd, STD_R0_0R12 + (r << 21) + (1 << 16) - (32 - r) * 8, p);
  return p + 4;
}

static bfd_byte *
savegpr1_tail (bfd *abfd, bfd_byte *p, int r)
{
  p = savegpr1 (abfd, p, r);
  bfd_put_32 (abfd, BLR, p);
  return p + 4;
}

static bfd_byte *
restgpr1 (bfd *abfd, bfd_byte *p, int r)
{
  bfd_put_32 (abfd, LD_R0_0R12 + (r << 21) + (1 << 16) - (32 - r) * 8, p);
  return p + 4;
}

static bfd_byte *
restgpr1_tail (bfd *abfd, bfd_byte *p, int r)
{
  p = restgpr1 (abfd, p, r);
  bfd_put_32 (abfd, BLR, p);
  return p + 4;
}

static bfd_byte *
savefpr (bfd *abfd, bfd_byte *p, int r)
{
  bfd_put_32 (abfd, STFD_FR0_0R1 + (r << 21) + (1 << 16) - (32 - r) * 8, p);
  return p + 4;
}

static bfd_byte *
savefpr0_tail (bfd *abfd, bfd_byte *p, int r)
{
  p = savefpr (abfd, p, r);
  bfd_put_32 (abfd, STD_R0_0R1 + STK_LR, p);
  p = p + 4;
  bfd_put_32 (abfd, BLR, p);
  return p + 4;
}

static bfd_byte *
restfpr (bfd *abfd, bfd_byte *p, int r)
{
  bfd_put_32 (abfd, LFD_FR0_0R1 + (r << 21) + (1 << 16) - (32 - r) * 8, p);
  return p + 4;
}

static bfd_byte *
restfpr0_tail (bfd *abfd, bfd_byte *p, int r)
{
  bfd_put_32 (abfd, LD_R0_0R1 + STK_LR, p);
  p = p + 4;
  p = restfpr (abfd, p, r);
  bfd_put_32 (abfd, MTLR_R0, p);
  p = p + 4;
  if (r == 29)
    {
      p = restfpr (abfd, p, 30);
      p = restfpr (abfd, p, 31);
    }
  bfd_put_32 (abfd, BLR, p);
  return p + 4;
}

static bfd_byte *
savefpr1_tail (bfd *abfd, bfd_byte *p, int r)
{
  p = savefpr (abfd, p, r);
  bfd_put_32 (abfd, BLR, p);
  return p + 4;
}

static bfd_byte *
restfpr1_tail (bfd *abfd, bfd_byte *p, int r)
{
  p = restfpr (abfd, p, r);
  bfd_put_32 (abfd, BLR, p);
  return p + 4;
}

static bfd_byte *
savevr (bfd *abfd, bfd_byte *p, int r)
{
  bfd_put_32 (abfd, LI_R12_0 + (1 << 16) - (32 - r) * 16, p);
  p = p + 4;
  bfd_put_32 (abfd, STVX_VR0_R12_R0 + (r << 21), p);
  return p + 4;
}

static bfd_byte *
savevr_tail (bfd *abfd, bfd_byte *p, int r)
{
  p = savevr (abfd, p, r);
  bfd_put_32 (abfd, BLR, p);
  return p + 4;
}

static bfd_byte *
restvr (bfd *abfd, bfd_byte *p, int r)
{
  bfd_put_32 (abfd, LI_R12_0 + (1 << 16) - (32 - r) * 16, p);
  p = p + 4;
  bfd_put_32 (abfd, LVX_VR0_R12_R0 + (r << 21), p);
  return p + 4;
}

static bfd_byte *
restvr_tail (bfd *abfd, bfd_byte *p, int r)
{
  p = restvr (abfd, p, r);
  bfd_put_32 (abfd, BLR, p);
  return p + 4;
}

#define STDU_R1_0R1	0xf8210001
#define ADDI_R1_R1	0x38210000

/* Emit prologue of wrapper preserving regs around a call to
   __tls_get_addr_opt.  */

static bfd_byte *
tls_get_addr_prologue (bfd *obfd, bfd_byte *p, struct ppc_link_hash_table *htab)
{
  unsigned int i;

  bfd_put_32 (obfd, MFLR_R0, p);
  p += 4;
  bfd_put_32 (obfd, STD_R0_0R1 + 16, p);
  p += 4;

  if (htab->opd_abi)
    {
      for (i = 4; i < 12; i++)
	{
	  bfd_put_32 (obfd,
		      STD_R0_0R1 | i << 21 | (-(13 - i) * 8 & 0xffff), p);
	  p += 4;
	}
      bfd_put_32 (obfd, STDU_R1_0R1 | (-128 & 0xffff), p);
      p += 4;
    }
  else
    {
      for (i = 4; i < 12; i++)
	{
	  bfd_put_32 (obfd,
		      STD_R0_0R1 | i << 21 | (-(12 - i) * 8 & 0xffff), p);
	  p += 4;
	}
      bfd_put_32 (obfd, STDU_R1_0R1 | (-96 & 0xffff), p);
      p += 4;
    }
  return p;
}

/* Emit epilogue of wrapper preserving regs around a call to
   __tls_get_addr_opt.  */

static bfd_byte *
tls_get_addr_epilogue (bfd *obfd, bfd_byte *p, struct ppc_link_hash_table *htab)
{
  unsigned int i;

  if (htab->opd_abi)
    {
      for (i = 4; i < 12; i++)
	{
	  bfd_put_32 (obfd, LD_R0_0R1 | i << 21 | (128 - (13 - i) * 8), p);
	  p += 4;
	}
      bfd_put_32 (obfd, ADDI_R1_R1 | 128, p);
      p += 4;
    }
  else
    {
      for (i = 4; i < 12; i++)
	{
	  bfd_put_32 (obfd, LD_R0_0R1 | i << 21 | (96 - (12 - i) * 8), p);
	  p += 4;
	}
      bfd_put_32 (obfd, ADDI_R1_R1 | 96, p);
      p += 4;
    }
  bfd_put_32 (obfd, LD_R0_0R1 | 16, p);
  p += 4;
  bfd_put_32 (obfd, MTLR_R0, p);
  p += 4;
  bfd_put_32 (obfd, BLR, p);
  p += 4;
  return p;
}

/* Called via elf_link_hash_traverse to transfer dynamic linking
   information on function code symbol entries to their corresponding
   function descriptor symbol entries.  Must not be called twice for
   any given code symbol.  */

static bool
func_desc_adjust (struct elf_link_hash_entry *h, void *inf)
{
  struct bfd_link_info *info;
  struct ppc_link_hash_table *htab;
  struct ppc_link_hash_entry *fh;
  struct ppc_link_hash_entry *fdh;
  bool force_local;

  fh = ppc_elf_hash_entry (h);
  if (fh->elf.root.type == bfd_link_hash_indirect)
    return true;

  if (!fh->is_func)
    return true;

  if (fh->elf.root.root.string[0] != '.'
      || fh->elf.root.root.string[1] == '\0')
    return true;

  info = inf;
  htab = ppc_hash_table (info);
  if (htab == NULL)
    return false;

  /* Find the corresponding function descriptor symbol.  */
  fdh = lookup_fdh (fh, htab);

  /* Resolve undefined references to dot-symbols as the value
     in the function descriptor, if we have one in a regular object.
     This is to satisfy cases like ".quad .foo".  Calls to functions
     in dynamic objects are handled elsewhere.  */
  if ((fh->elf.root.type == bfd_link_hash_undefined
       || fh->elf.root.type == bfd_link_hash_undefweak)
      && (fdh->elf.root.type == bfd_link_hash_defined
	  || fdh->elf.root.type == bfd_link_hash_defweak)
      && get_opd_info (fdh->elf.root.u.def.section) != NULL
      && opd_entry_value (fdh->elf.root.u.def.section,
			  fdh->elf.root.u.def.value,
			  &fh->elf.root.u.def.section,
			  &fh->elf.root.u.def.value, false) != (bfd_vma) -1)
    {
      fh->elf.root.type = fdh->elf.root.type;
      fh->elf.forced_local = 1;
      fh->elf.def_regular = fdh->elf.def_regular;
      fh->elf.def_dynamic = fdh->elf.def_dynamic;
    }

  if (!fh->elf.dynamic)
    {
      struct plt_entry *ent;

      for (ent = fh->elf.plt.plist; ent != NULL; ent = ent->next)
	if (ent->plt.refcount > 0)
	  break;
      if (ent == NULL)
	{
	  if (fdh != NULL && fdh->fake)
	    _bfd_elf_link_hash_hide_symbol (info, &fdh->elf, true);
	  return true;
	}
    }

  /* Create a descriptor as undefined if necessary.  */
  if (fdh == NULL
      && !bfd_link_executable (info)
      && (fh->elf.root.type == bfd_link_hash_undefined
	  || fh->elf.root.type == bfd_link_hash_undefweak))
    {
      fdh = make_fdh (info, fh);
      if (fdh == NULL)
	return false;
    }

  /* We can't support overriding of symbols on a fake descriptor.  */
  if (fdh != NULL
      && fdh->fake
      && (fh->elf.root.type == bfd_link_hash_defined
	  || fh->elf.root.type == bfd_link_hash_defweak))
    _bfd_elf_link_hash_hide_symbol (info, &fdh->elf, true);

  /* Transfer dynamic linking information to the function descriptor.  */
  if (fdh != NULL)
    {
      fdh->elf.ref_regular |= fh->elf.ref_regular;
      fdh->elf.ref_dynamic |= fh->elf.ref_dynamic;
      fdh->elf.ref_regular_nonweak |= fh->elf.ref_regular_nonweak;
      fdh->elf.non_got_ref |= fh->elf.non_got_ref;
      fdh->elf.dynamic |= fh->elf.dynamic;
      fdh->elf.needs_plt |= (fh->elf.needs_plt
			     || fh->elf.type == STT_FUNC
			     || fh->elf.type == STT_GNU_IFUNC);
      move_plt_plist (fh, fdh);

      if (!fdh->elf.forced_local
	  && fh->elf.dynindx != -1)
	if (!bfd_elf_link_record_dynamic_symbol (info, &fdh->elf))
	  return false;
    }

  /* Now that the info is on the function descriptor, clear the
     function code sym info.  Any function code syms for which we
     don't have a definition in a regular file, we force local.
     This prevents a shared library from exporting syms that have
     been imported from another library.  Function code syms that
     are really in the library we must leave global to prevent the
     linker dragging in a definition from a static library.  */
  force_local = (!fh->elf.def_regular
		 || fdh == NULL
		 || !fdh->elf.def_regular
		 || fdh->elf.forced_local);
  _bfd_elf_link_hash_hide_symbol (info, &fh->elf, force_local);

  return true;
}

static const struct sfpr_def_parms save_res_funcs[] =
  {
    { "_savegpr0_", 14, 31, savegpr0, savegpr0_tail },
    { "_restgpr0_", 14, 29, restgpr0, restgpr0_tail },
    { "_restgpr0_", 30, 31, restgpr0, restgpr0_tail },
    { "_savegpr1_", 14, 31, savegpr1, savegpr1_tail },
    { "_restgpr1_", 14, 31, restgpr1, restgpr1_tail },
    { "_savefpr_", 14, 31, savefpr, savefpr0_tail },
    { "_restfpr_", 14, 29, restfpr, restfpr0_tail },
    { "_restfpr_", 30, 31, restfpr, restfpr0_tail },
    { "._savef", 14, 31, savefpr, savefpr1_tail },
    { "._restf", 14, 31, restfpr, restfpr1_tail },
    { "_savevr_", 20, 31, savevr, savevr_tail },
    { "_restvr_", 20, 31, restvr, restvr_tail }
  };

/* Called near the start of bfd_elf_size_dynamic_sections.  We use
   this hook to a) run the edit functions in this file, b) provide
   some gcc support functions, and c) transfer dynamic linking
   information gathered so far on function code symbol entries, to
   their corresponding function descriptor symbol entries.  */

static bool
ppc64_elf_edit (bfd *obfd ATTRIBUTE_UNUSED, struct bfd_link_info *info)
{
  struct ppc_link_hash_table *htab;

  htab = ppc_hash_table (info);
  if (htab == NULL)
    return false;

  /* Call back into the linker, which then runs the edit functions.  */
  htab->params->edit ();

  /* Provide any missing _save* and _rest* functions.  */
  if (htab->sfpr != NULL)
    {
      unsigned int i;

      htab->sfpr->size = 0;
      for (i = 0; i < ARRAY_SIZE (save_res_funcs); i++)
	if (!sfpr_define (info, &save_res_funcs[i], NULL))
	  return false;
      if (htab->sfpr->size == 0)
	htab->sfpr->flags |= SEC_EXCLUDE;
    }

  if (bfd_link_relocatable (info))
    return true;

  if (htab->elf.hgot != NULL)
    {
      _bfd_elf_link_hash_hide_symbol (info, htab->elf.hgot, true);
      /* Make .TOC. defined so as to prevent it being made dynamic.
	 The wrong value here is fixed later in ppc64_elf_set_toc.  */
      if (!htab->elf.hgot->def_regular
	  || htab->elf.hgot->root.type != bfd_link_hash_defined)
	{
	  htab->elf.hgot->root.type = bfd_link_hash_defined;
	  htab->elf.hgot->root.u.def.value = 0;
	  htab->elf.hgot->root.u.def.section = bfd_abs_section_ptr;
	  htab->elf.hgot->def_regular = 1;
	  htab->elf.hgot->root.linker_def = 1;
	}
      htab->elf.hgot->type = STT_OBJECT;
      htab->elf.hgot->other
	= (htab->elf.hgot->other & ~ELF_ST_VISIBILITY (-1)) | STV_HIDDEN;
    }

  return true;
}

/* Return true if we have dynamic relocs against H or any of its weak
   aliases, that apply to read-only sections.  Cannot be used after
   size_dynamic_sections.  */

static bool
alias_readonly_dynrelocs (struct elf_link_hash_entry *h)
{
  struct ppc_link_hash_entry *eh = ppc_elf_hash_entry (h);
  do
    {
      if (_bfd_elf_readonly_dynrelocs (&eh->elf))
	return true;
      eh = ppc_elf_hash_entry (eh->elf.u.alias);
    }
  while (eh != NULL && &eh->elf != h);

  return false;
}

/* Return whether EH has pc-relative dynamic relocs.  */

static bool
pc_dynrelocs (struct ppc_link_hash_entry *eh)
{
  struct ppc_dyn_relocs *p;

  for (p = (struct ppc_dyn_relocs *) eh->elf.dyn_relocs; p != NULL; p = p->next)
    if (p->pc_count != 0)
      return true;
  return false;
}

/* Return true if a global entry stub will be created for H.  Valid
   for ELFv2 before plt entries have been allocated.  */

static bool
global_entry_stub (struct elf_link_hash_entry *h)
{
  struct plt_entry *pent;

  if (!h->pointer_equality_needed
      || h->def_regular)
    return false;

  for (pent = h->plt.plist; pent != NULL; pent = pent->next)
    if (pent->plt.refcount > 0
	&& pent->addend == 0)
      return true;

  return false;
}

/* Adjust a symbol defined by a dynamic object and referenced by a
   regular object.  The current definition is in some section of the
   dynamic object, but we're not including those sections.  We have to
   change the definition to something the rest of the link can
   understand.  */

static bool
ppc64_elf_adjust_dynamic_symbol (struct bfd_link_info *info,
				 struct elf_link_hash_entry *h)
{
  struct ppc_link_hash_table *htab;
  asection *s, *srel;

  htab = ppc_hash_table (info);
  if (htab == NULL)
    return false;

  /* Deal with function syms.  */
  if (h->type == STT_FUNC
      || h->type == STT_GNU_IFUNC
      || h->needs_plt)
    {
      bool local = (ppc_elf_hash_entry (h)->save_res
		    || SYMBOL_CALLS_LOCAL (info, h)
		    || UNDEFWEAK_NO_DYNAMIC_RELOC (info, h));
      /* Discard dyn_relocs when non-pic if we've decided that a
	 function symbol is local and not an ifunc.  We keep dynamic
	 relocs for ifuncs when local rather than always emitting a
	 plt call stub for them and defining the symbol on the call
	 stub.  We can't do that for ELFv1 anyway (a function symbol
	 is defined on a descriptor, not code) and it can be faster at
	 run-time due to not needing to bounce through a stub.  The
	 dyn_relocs for ifuncs will be applied even in a static
	 executable.  */
      if (!bfd_link_pic (info)
	  && h->type != STT_GNU_IFUNC
	  && local)
	h->dyn_relocs = NULL;

      /* Clear procedure linkage table information for any symbol that
	 won't need a .plt entry.  */
      struct plt_entry *ent;
      for (ent = h->plt.plist; ent != NULL; ent = ent->next)
	if (ent->plt.refcount > 0)
	  break;
      if (ent == NULL
	  || (h->type != STT_GNU_IFUNC
	      && local
	      && (htab->can_convert_all_inline_plt
		  || (ppc_elf_hash_entry (h)->tls_mask
		      & (TLS_TLS | PLT_KEEP)) != PLT_KEEP)))
	{
	  h->plt.plist = NULL;
	  h->needs_plt = 0;
	  h->pointer_equality_needed = 0;
	}
      else if (abiversion (info->output_bfd) >= 2)
	{
	  /* Taking a function's address in a read/write section
	     doesn't require us to define the function symbol in the
	     executable on a global entry stub.  A dynamic reloc can
	     be used instead.  The reason we prefer a few more dynamic
	     relocs is that calling via a global entry stub costs a
	     few more instructions, and pointer_equality_needed causes
	     extra work in ld.so when resolving these symbols.  */
	  if (global_entry_stub (h))
	    {
	      if (!_bfd_elf_readonly_dynrelocs (h))
		{
		  h->pointer_equality_needed = 0;
		  /* If we haven't seen a branch reloc and the symbol
		     isn't an ifunc then we don't need a plt entry.  */
		  if (!h->needs_plt)
		    h->plt.plist = NULL;
		}
	      else if (!bfd_link_pic (info))
		/* We are going to be defining the function symbol on the
		   plt stub, so no dyn_relocs needed when non-pic.  */
		h->dyn_relocs = NULL;
	    }

	  /* ELFv2 function symbols can't have copy relocs.  */
	  return true;
	}
      else if (!h->needs_plt
	       && !_bfd_elf_readonly_dynrelocs (h))
	{
	  /* If we haven't seen a branch reloc and the symbol isn't an
	     ifunc then we don't need a plt entry.  */
	  h->plt.plist = NULL;
	  h->pointer_equality_needed = 0;
	  return true;
	}
    }
  else
    h->plt.plist = NULL;

  /* If this is a weak symbol, and there is a real definition, the
     processor independent code will have arranged for us to see the
     real definition first, and we can just use the same value.  */
  if (h->is_weakalias)
    {
      struct elf_link_hash_entry *def = weakdef (h);
      BFD_ASSERT (def->root.type == bfd_link_hash_defined);
      h->root.u.def.section = def->root.u.def.section;
      h->root.u.def.value = def->root.u.def.value;
      if (def->root.u.def.section == htab->elf.sdynbss
	  || def->root.u.def.section == htab->elf.sdynrelro)
	h->dyn_relocs = NULL;
      return true;
    }

  /* If we are creating a shared library, we must presume that the
     only references to the symbol are via the global offset table.
     For such cases we need not do anything here; the relocations will
     be handled correctly by relocate_section.  */
  if (!bfd_link_executable (info))
    return true;

  /* If there are no references to this symbol that do not use the
     GOT, we don't need to generate a copy reloc.  */
  if (!h->non_got_ref)
    return true;

  /* Don't generate a copy reloc for symbols defined in the executable.  */
  if (!h->def_dynamic || !h->ref_regular || h->def_regular

      /* If -z nocopyreloc was given, don't generate them either.  */
      || info->nocopyreloc

      /* If we don't find any dynamic relocs in read-only sections, then
	 we'll be keeping the dynamic relocs and avoiding the copy reloc.  */
      || (ELIMINATE_COPY_RELOCS
	  && !h->needs_copy
	  && !alias_readonly_dynrelocs (h))

      /* Protected variables do not work with .dynbss.  The copy in
	 .dynbss won't be used by the shared library with the protected
	 definition for the variable.  Text relocations are preferable
	 to an incorrect program.  */
      || h->protected_def)
    return true;

  if (h->type == STT_FUNC
      || h->type == STT_GNU_IFUNC)
    {
      /* .dynbss copies of function symbols only work if we have
	 ELFv1 dot-symbols.  ELFv1 compilers since 2004 default to not
	 use dot-symbols and set the function symbol size to the text
	 size of the function rather than the size of the descriptor.
	 That's wrong for copying a descriptor.  */
      if (ppc_elf_hash_entry (h)->oh == NULL
	  || !(h->size == 24 || h->size == 16))
	return true;

      /* We should never get here, but unfortunately there are old
	 versions of gcc (circa gcc-3.2) that improperly for the
	 ELFv1 ABI put initialized function pointers, vtable refs and
	 suchlike in read-only sections.  Allow them to proceed, but
	 warn that this might break at runtime.  */
      info->callbacks->einfo
	(_("%P: copy reloc against `%pT' requires lazy plt linking; "
	   "avoid setting LD_BIND_NOW=1 or upgrade gcc\n"),
	 h->root.root.string);
    }

  /* This is a reference to a symbol defined by a dynamic object which
     is not a function.  */

  /* We must allocate the symbol in our .dynbss section, which will
     become part of the .bss section of the executable.  There will be
     an entry for this symbol in the .dynsym section.  The dynamic
     object will contain position independent code, so all references
     from the dynamic object to this symbol will go through the global
     offset table.  The dynamic linker will use the .dynsym entry to
     determine the address it must put in the global offset table, so
     both the dynamic object and the regular object will refer to the
     same memory location for the variable.  */
  if ((h->root.u.def.section->flags & SEC_READONLY) != 0)
    {
      s = htab->elf.sdynrelro;
      srel = htab->elf.sreldynrelro;
    }
  else
    {
      s = htab->elf.sdynbss;
      srel = htab->elf.srelbss;
    }
  if ((h->root.u.def.section->flags & SEC_ALLOC) != 0 && h->size != 0)
    {
      /* We must generate a R_PPC64_COPY reloc to tell the dynamic
	 linker to copy the initial value out of the dynamic object
	 and into the runtime process image.  */
      srel->size += sizeof (Elf64_External_Rela);
      h->needs_copy = 1;
    }

  /* We no longer want dyn_relocs.  */
  h->dyn_relocs = NULL;
  return _bfd_elf_adjust_dynamic_copy (info, h, s);
}

/* If given a function descriptor symbol, hide both the function code
   sym and the descriptor.  */
static void
ppc64_elf_hide_symbol (struct bfd_link_info *info,
		       struct elf_link_hash_entry *h,
		       bool force_local)
{
  struct ppc_link_hash_entry *eh;
  _bfd_elf_link_hash_hide_symbol (info, h, force_local);

  if (ppc_hash_table (info) == NULL)
    return;

  eh = ppc_elf_hash_entry (h);
  if (eh->is_func_descriptor)
    {
      struct ppc_link_hash_entry *fh = eh->oh;

      if (fh == NULL)
	{
	  const char *p, *q;
	  struct elf_link_hash_table *htab = elf_hash_table (info);
	  char save;

	  /* We aren't supposed to use alloca in BFD because on
	     systems which do not have alloca the version in libiberty
	     calls xmalloc, which might cause the program to crash
	     when it runs out of memory.  This function doesn't have a
	     return status, so there's no way to gracefully return an
	     error.  So cheat.  We know that string[-1] can be safely
	     accessed;  It's either a string in an ELF string table,
	     or allocated in an objalloc structure.  */

	  p = eh->elf.root.root.string - 1;
	  save = *p;
	  *(char *) p = '.';
	  fh = ppc_elf_hash_entry (elf_link_hash_lookup (htab, p, false,
							 false, false));
	  *(char *) p = save;

	  /* Unfortunately, if it so happens that the string we were
	     looking for was allocated immediately before this string,
	     then we overwrote the string terminator.  That's the only
	     reason the lookup should fail.  */
	  if (fh == NULL)
	    {
	      q = eh->elf.root.root.string + strlen (eh->elf.root.root.string);
	      while (q >= eh->elf.root.root.string && *q == *p)
		--q, --p;
	      if (q < eh->elf.root.root.string && *p == '.')
		fh = ppc_elf_hash_entry (elf_link_hash_lookup (htab, p, false,
							       false, false));
	    }
	  if (fh != NULL)
	    {
	      eh->oh = fh;
	      fh->oh = eh;
	    }
	}
      if (fh != NULL)
	_bfd_elf_link_hash_hide_symbol (info, &fh->elf, force_local);
    }
}

static bool
get_sym_h (struct elf_link_hash_entry **hp,
	   Elf_Internal_Sym **symp,
	   asection **symsecp,
	   unsigned char **tls_maskp,
	   Elf_Internal_Sym **locsymsp,
	   unsigned long r_symndx,
	   bfd *ibfd)
{
  Elf_Internal_Shdr *symtab_hdr = &elf_symtab_hdr (ibfd);

  if (r_symndx >= symtab_hdr->sh_info)
    {
      struct elf_link_hash_entry **sym_hashes = elf_sym_hashes (ibfd);
      struct elf_link_hash_entry *h;

      h = sym_hashes[r_symndx - symtab_hdr->sh_info];
      h = elf_follow_link (h);

      if (hp != NULL)
	*hp = h;

      if (symp != NULL)
	*symp = NULL;

      if (symsecp != NULL)
	{
	  asection *symsec = NULL;
	  if (h->root.type == bfd_link_hash_defined
	      || h->root.type == bfd_link_hash_defweak)
	    symsec = h->root.u.def.section;
	  *symsecp = symsec;
	}

      if (tls_maskp != NULL)
	*tls_maskp = &ppc_elf_hash_entry (h)->tls_mask;
    }
  else
    {
      Elf_Internal_Sym *sym;
      Elf_Internal_Sym *locsyms = *locsymsp;

      if (locsyms == NULL)
	{
	  locsyms = (Elf_Internal_Sym *) symtab_hdr->contents;
	  if (locsyms == NULL)
	    locsyms = bfd_elf_get_elf_syms (ibfd, symtab_hdr,
					    symtab_hdr->sh_info,
					    0, NULL, NULL, NULL);
	  if (locsyms == NULL)
	    return false;
	  *locsymsp = locsyms;
	}
      sym = locsyms + r_symndx;

      if (hp != NULL)
	*hp = NULL;

      if (symp != NULL)
	*symp = sym;

      if (symsecp != NULL)
	*symsecp = bfd_section_from_elf_index (ibfd, sym->st_shndx);

      if (tls_maskp != NULL)
	{
	  struct got_entry **lgot_ents;
	  unsigned char *tls_mask;

	  tls_mask = NULL;
	  lgot_ents = elf_local_got_ents (ibfd);
	  if (lgot_ents != NULL)
	    {
	      struct plt_entry **local_plt = (struct plt_entry **)
		(lgot_ents + symtab_hdr->sh_info);
	      unsigned char *lgot_masks = (unsigned char *)
		(local_plt + symtab_hdr->sh_info);
	      tls_mask = &lgot_masks[r_symndx];
	    }
	  *tls_maskp = tls_mask;
	}
    }
  return true;
}

/* Returns TLS_MASKP for the given REL symbol.  Function return is 0 on
   error, 2 on a toc GD type suitable for optimization, 3 on a toc LD
   type suitable for optimization, and 1 otherwise.  */

static int
get_tls_mask (unsigned char **tls_maskp,
	      unsigned long *toc_symndx,
	      bfd_vma *toc_addend,
	      Elf_Internal_Sym **locsymsp,
	      const Elf_Internal_Rela *rel,
	      bfd *ibfd)
{
  unsigned long r_symndx;
  int next_r;
  struct elf_link_hash_entry *h;
  Elf_Internal_Sym *sym;
  asection *sec;
  bfd_vma off;

  r_symndx = ELF64_R_SYM (rel->r_info);
  if (!get_sym_h (&h, &sym, &sec, tls_maskp, locsymsp, r_symndx, ibfd))
    return 0;

  if ((*tls_maskp != NULL
       && (**tls_maskp & TLS_TLS) != 0
       && **tls_maskp != (TLS_TLS | TLS_MARK))
      || sec == NULL
      || ppc64_elf_section_data (sec) == NULL
      || ppc64_elf_section_data (sec)->sec_type != sec_toc)
    return 1;

  /* Look inside a TOC section too.  */
  if (h != NULL)
    {
      BFD_ASSERT (h->root.type == bfd_link_hash_defined);
      off = h->root.u.def.value;
    }
  else
    off = sym->st_value;
  off += rel->r_addend;
  BFD_ASSERT (off % 8 == 0);
  r_symndx = ppc64_elf_section_data (sec)->u.toc.symndx[off / 8];
  next_r = ppc64_elf_section_data (sec)->u.toc.symndx[off / 8 + 1];
  if (toc_symndx != NULL)
    *toc_symndx = r_symndx;
  if (toc_addend != NULL)
    *toc_addend = ppc64_elf_section_data (sec)->u.toc.add[off / 8];
  if (!get_sym_h (&h, &sym, &sec, tls_maskp, locsymsp, r_symndx, ibfd))
    return 0;
  if ((h == NULL || is_static_defined (h))
      && (next_r == -1 || next_r == -2))
    return 1 - next_r;
  return 1;
}

/* Find (or create) an entry in the tocsave hash table.  */

static struct tocsave_entry *
tocsave_find (struct ppc_link_hash_table *htab,
	      enum insert_option insert,
	      Elf_Internal_Sym **local_syms,
	      const Elf_Internal_Rela *irela,
	      bfd *ibfd)
{
  unsigned long r_indx;
  struct elf_link_hash_entry *h;
  Elf_Internal_Sym *sym;
  struct tocsave_entry ent, *p;
  hashval_t hash;
  struct tocsave_entry **slot;

  r_indx = ELF64_R_SYM (irela->r_info);
  if (!get_sym_h (&h, &sym, &ent.sec, NULL, local_syms, r_indx, ibfd))
    return NULL;
  if (ent.sec == NULL || ent.sec->output_section == NULL)
    {
      _bfd_error_handler
	(_("%pB: undefined symbol on R_PPC64_TOCSAVE relocation"), ibfd);
      return NULL;
    }

  if (h != NULL)
    ent.offset = h->root.u.def.value;
  else
    ent.offset = sym->st_value;
  ent.offset += irela->r_addend;

  hash = tocsave_htab_hash (&ent);
  slot = ((struct tocsave_entry **)
	  htab_find_slot_with_hash (htab->tocsave_htab, &ent, hash, insert));
  if (slot == NULL)
    return NULL;

  if (*slot == NULL)
    {
      p = (struct tocsave_entry *) bfd_alloc (ibfd, sizeof (*p));
      if (p == NULL)
	return NULL;
      *p = ent;
      *slot = p;
    }
  return *slot;
}

/* Adjust all global syms defined in opd sections.  In gcc generated
   code for the old ABI, these will already have been done.  */

static bool
adjust_opd_syms (struct elf_link_hash_entry *h, void *inf ATTRIBUTE_UNUSED)
{
  struct ppc_link_hash_entry *eh;
  asection *sym_sec;
  struct _opd_sec_data *opd;

  if (h->root.type == bfd_link_hash_indirect)
    return true;

  if (h->root.type != bfd_link_hash_defined
      && h->root.type != bfd_link_hash_defweak)
    return true;

  eh = ppc_elf_hash_entry (h);
  if (eh->adjust_done)
    return true;

  sym_sec = eh->elf.root.u.def.section;
  opd = get_opd_info (sym_sec);
  if (opd != NULL && opd->adjust != NULL)
    {
      long adjust = opd->adjust[OPD_NDX (eh->elf.root.u.def.value)];
      if (adjust == -1)
	{
	  /* This entry has been deleted.  */
	  asection *dsec = ppc64_elf_tdata (sym_sec->owner)->deleted_section;
	  if (dsec == NULL)
	    {
	      for (dsec = sym_sec->owner->sections; dsec; dsec = dsec->next)
		if (discarded_section (dsec))
		  {
		    ppc64_elf_tdata (sym_sec->owner)->deleted_section = dsec;
		    break;
		  }
	    }
	  eh->elf.root.u.def.value = 0;
	  eh->elf.root.u.def.section = dsec;
	}
      else
	eh->elf.root.u.def.value += adjust;
      eh->adjust_done = 1;
    }
  return true;
}

/* Handles decrementing dynamic reloc counts for the reloc specified by
   R_INFO in section SEC.  If LOCAL_SYMS is NULL, then H and SYM
   have already been determined.  */

static bool
dec_dynrel_count (const Elf_Internal_Rela *rel,
		  asection *sec,
		  struct bfd_link_info *info,
		  Elf_Internal_Sym **local_syms,
		  struct elf_link_hash_entry *h,
		  Elf_Internal_Sym *sym)
{
  enum elf_ppc64_reloc_type r_type;
  asection *sym_sec = NULL;

  /* Can this reloc be dynamic?  This switch, and later tests here
     should be kept in sync with the code in check_relocs.  */
  r_type = ELF64_R_TYPE (rel->r_info);
  switch (r_type)
    {
    default:
      return true;

    case R_PPC64_TOC16:
    case R_PPC64_TOC16_DS:
    case R_PPC64_TOC16_LO:
    case R_PPC64_TOC16_HI:
    case R_PPC64_TOC16_HA:
    case R_PPC64_TOC16_LO_DS:
      if (h == NULL)
	return true;
      break;

    case R_PPC64_TPREL16:
    case R_PPC64_TPREL16_LO:
    case R_PPC64_TPREL16_HI:
    case R_PPC64_TPREL16_HA:
    case R_PPC64_TPREL16_DS:
    case R_PPC64_TPREL16_LO_DS:
    case R_PPC64_TPREL16_HIGH:
    case R_PPC64_TPREL16_HIGHA:
    case R_PPC64_TPREL16_HIGHER:
    case R_PPC64_TPREL16_HIGHERA:
    case R_PPC64_TPREL16_HIGHEST:
    case R_PPC64_TPREL16_HIGHESTA:
    case R_PPC64_TPREL64:
    case R_PPC64_TPREL34:
    case R_PPC64_DTPMOD64:
    case R_PPC64_DTPREL64:
    case R_PPC64_ADDR64:
    case R_PPC64_REL30:
    case R_PPC64_REL32:
    case R_PPC64_REL64:
    case R_PPC64_ADDR14:
    case R_PPC64_ADDR14_BRNTAKEN:
    case R_PPC64_ADDR14_BRTAKEN:
    case R_PPC64_ADDR16:
    case R_PPC64_ADDR16_DS:
    case R_PPC64_ADDR16_HA:
    case R_PPC64_ADDR16_HI:
    case R_PPC64_ADDR16_HIGH:
    case R_PPC64_ADDR16_HIGHA:
    case R_PPC64_ADDR16_HIGHER:
    case R_PPC64_ADDR16_HIGHERA:
    case R_PPC64_ADDR16_HIGHEST:
    case R_PPC64_ADDR16_HIGHESTA:
    case R_PPC64_ADDR16_LO:
    case R_PPC64_ADDR16_LO_DS:
    case R_PPC64_ADDR24:
    case R_PPC64_ADDR32:
    case R_PPC64_UADDR16:
    case R_PPC64_UADDR32:
    case R_PPC64_UADDR64:
    case R_PPC64_TOC:
    case R_PPC64_D34:
    case R_PPC64_D34_LO:
    case R_PPC64_D34_HI30:
    case R_PPC64_D34_HA30:
    case R_PPC64_ADDR16_HIGHER34:
    case R_PPC64_ADDR16_HIGHERA34:
    case R_PPC64_ADDR16_HIGHEST34:
    case R_PPC64_ADDR16_HIGHESTA34:
    case R_PPC64_D28:
      break;
    }

  if (local_syms != NULL)
    {
      unsigned long r_symndx;
      bfd *ibfd = sec->owner;

      r_symndx = ELF64_R_SYM (rel->r_info);
      if (!get_sym_h (&h, &sym, &sym_sec, NULL, local_syms, r_symndx, ibfd))
	return false;
    }

  if ((h != NULL
       && !SYMBOL_REFERENCES_LOCAL (info, h))
      || (bfd_link_pic (info)
	  && (h != NULL
	      ? !bfd_is_abs_symbol (&h->root)
	      : sym_sec != bfd_abs_section_ptr)
	  && must_be_dyn_reloc (info, r_type))
      || (!bfd_link_pic (info)
	  && (h != NULL
	      ? h->type == STT_GNU_IFUNC
	      : ELF_ST_TYPE (sym->st_info) == STT_GNU_IFUNC)))
    ;
  else
    return true;

  if (h != NULL)
    {
      struct ppc_dyn_relocs *p;
      struct ppc_dyn_relocs **pp;
      pp = (struct ppc_dyn_relocs **) &h->dyn_relocs;

      /* elf_gc_sweep may have already removed all dyn relocs associated
	 with local syms for a given section.  Also, symbol flags are
	 changed by elf_gc_sweep_symbol, confusing the test above.  Don't
	 report a dynreloc miscount.  */
      if (*pp == NULL && info->gc_sections)
	return true;

      while ((p = *pp) != NULL)
	{
	  if (p->sec == sec)
	    {
	      if (!must_be_dyn_reloc (info, r_type))
		p->pc_count -= 1;
	      if ((r_type == R_PPC64_ADDR64 || r_type == R_PPC64_TOC)
		  && rel->r_offset % 2 == 0
		  && sec->alignment_power != 0)
		p->rel_count -= 1;
	      p->count -= 1;
	      if (p->count == 0)
		*pp = p->next;
	      return true;
	    }
	  pp = &p->next;
	}
    }
  else
    {
      struct ppc_local_dyn_relocs *p;
      struct ppc_local_dyn_relocs **pp;
      void *vpp;
      bool is_ifunc;

      if (local_syms == NULL)
	sym_sec = bfd_section_from_elf_index (sec->owner, sym->st_shndx);
      if (sym_sec == NULL)
	sym_sec = sec;

      vpp = &elf_section_data (sym_sec)->local_dynrel;
      pp = (struct ppc_local_dyn_relocs **) vpp;

      if (*pp == NULL && info->gc_sections)
	return true;

      is_ifunc = ELF_ST_TYPE (sym->st_info) == STT_GNU_IFUNC;
      while ((p = *pp) != NULL)
	{
	  if (p->sec == sec && p->ifunc == is_ifunc)
	    {
	      if ((r_type == R_PPC64_ADDR64 || r_type == R_PPC64_TOC)
		  && rel->r_offset % 2 == 0
		  && sec->alignment_power != 0)
		p->rel_count -= 1;
	      p->count -= 1;
	      if (p->count == 0)
		*pp = p->next;
	      return true;
	    }
	  pp = &p->next;
	}
    }

  /* xgettext:c-format */
  _bfd_error_handler (_("dynreloc miscount for %pB, section %pA"),
		      sec->owner, sec);
  bfd_set_error (bfd_error_bad_value);
  return false;
}

/* Remove unused Official Procedure Descriptor entries.  Currently we
   only remove those associated with functions in discarded link-once
   sections, or weakly defined functions that have been overridden.  It
   would be possible to remove many more entries for statically linked
   applications.  */

bool
ppc64_elf_edit_opd (struct bfd_link_info *info)
{
  bfd *ibfd;
  bool some_edited = false;
  asection *need_pad = NULL;
  struct ppc_link_hash_table *htab;

  htab = ppc_hash_table (info);
  if (htab == NULL)
    return false;

  for (ibfd = info->input_bfds; ibfd != NULL; ibfd = ibfd->link.next)
    {
      asection *sec;
      Elf_Internal_Rela *relstart, *rel, *relend;
      Elf_Internal_Shdr *symtab_hdr;
      Elf_Internal_Sym *local_syms;
      struct _opd_sec_data *opd;
      bool need_edit, add_aux_fields, broken;
      bfd_size_type cnt_16b = 0;

      if (!is_ppc64_elf (ibfd))
	continue;

      sec = bfd_get_section_by_name (ibfd, ".opd");
      if (sec == NULL
	  || sec->size == 0
	  || (sec->flags & SEC_HAS_CONTENTS) == 0)
	continue;

      if (sec->sec_info_type == SEC_INFO_TYPE_JUST_SYMS)
	continue;

      if (sec->output_section == bfd_abs_section_ptr)
	continue;

      /* Look through the section relocs.  */
      if ((sec->flags & SEC_RELOC) == 0 || sec->reloc_count == 0)
	continue;

      local_syms = NULL;
      symtab_hdr = &elf_symtab_hdr (ibfd);

      /* Read the relocations.  */
      relstart = _bfd_elf_link_read_relocs (ibfd, sec, NULL, NULL,
					    info->keep_memory);
      if (relstart == NULL)
	return false;

      /* First run through the relocs to check they are sane, and to
	 determine whether we need to edit this opd section.  */
      need_edit = false;
      broken = false;
      need_pad = sec;
      relend = relstart + sec->reloc_count;
      for (rel = relstart; rel < relend; )
	{
	  enum elf_ppc64_reloc_type r_type;
	  unsigned long r_symndx;
	  asection *sym_sec;
	  struct elf_link_hash_entry *h;
	  Elf_Internal_Sym *sym;
	  bfd_vma offset;

	  /* .opd contains an array of 16 or 24 byte entries.  We're
	     only interested in the reloc pointing to a function entry
	     point.  */
	  offset = rel->r_offset;
	  if (rel + 1 == relend
	      || rel[1].r_offset != offset + 8)
	    {
	      /* If someone messes with .opd alignment then after a
		 "ld -r" we might have padding in the middle of .opd.
		 Also, there's nothing to prevent someone putting
		 something silly in .opd with the assembler.  No .opd
		 optimization for them!  */
	    broken_opd:
	      _bfd_error_handler
		(_("%pB: .opd is not a regular array of opd entries"), ibfd);
	      broken = true;
	      break;
	    }

	  if ((r_type = ELF64_R_TYPE (rel->r_info)) != R_PPC64_ADDR64
	      || (r_type = ELF64_R_TYPE ((rel + 1)->r_info)) != R_PPC64_TOC)
	    {
	      _bfd_error_handler
		/* xgettext:c-format */
		(_("%pB: unexpected reloc type %u in .opd section"),
		 ibfd, r_type);
	      broken = true;
	      break;
	    }

	  r_symndx = ELF64_R_SYM (rel->r_info);
	  if (!get_sym_h (&h, &sym, &sym_sec, NULL, &local_syms,
			  r_symndx, ibfd))
	    goto error_ret;

	  if (sym_sec == NULL || sym_sec->owner == NULL)
	    {
	      const char *sym_name;
	      if (h != NULL)
		sym_name = h->root.root.string;
	      else
		sym_name = bfd_elf_sym_name (ibfd, symtab_hdr, sym,
					     sym_sec);

	      _bfd_error_handler
		/* xgettext:c-format */
		(_("%pB: undefined sym `%s' in .opd section"),
		 ibfd, sym_name);
	      broken = true;
	      break;
	    }

	  /* opd entries are always for functions defined in the
	     current input bfd.  If the symbol isn't defined in the
	     input bfd, then we won't be using the function in this
	     bfd;  It must be defined in a linkonce section in another
	     bfd, or is weak.  It's also possible that we are
	     discarding the function due to a linker script /DISCARD/,
	     which we test for via the output_section.  */
	  if (sym_sec->owner != ibfd
	      || sym_sec->output_section == bfd_abs_section_ptr)
	    need_edit = true;

	  rel += 2;
	  if (rel + 1 == relend
	      || (rel + 2 < relend
		  && ELF64_R_TYPE (rel[2].r_info) == R_PPC64_TOC))
	    ++rel;

	  if (rel == relend)
	    {
	      if (sec->size == offset + 24)
		{
		  need_pad = NULL;
		  break;
		}
	      if (sec->size == offset + 16)
		{
		  cnt_16b++;
		  break;
		}
	      goto broken_opd;
	    }
	  else if (rel + 1 < relend
		   && ELF64_R_TYPE (rel[0].r_info) == R_PPC64_ADDR64
		   && ELF64_R_TYPE (rel[1].r_info) == R_PPC64_TOC)
	    {
	      if (rel[0].r_offset == offset + 16)
		cnt_16b++;
	      else if (rel[0].r_offset != offset + 24)
		goto broken_opd;
	    }
	  else
	    goto broken_opd;
	}

      add_aux_fields = htab->params->non_overlapping_opd && cnt_16b > 0;

      if (!broken && (need_edit || add_aux_fields))
	{
	  Elf_Internal_Rela *write_rel;
	  Elf_Internal_Shdr *rel_hdr;
	  bfd_byte *rptr, *wptr;
	  bfd_byte *new_contents;
	  bfd_size_type amt;

	  new_contents = NULL;
	  amt = OPD_NDX (sec->size) * sizeof (long);
	  opd = &ppc64_elf_section_data (sec)->u.opd;
	  opd->adjust = bfd_zalloc (sec->owner, amt);
	  if (opd->adjust == NULL)
	    return false;

	  /* This seems a waste of time as input .opd sections are all
	     zeros as generated by gcc, but I suppose there's no reason
	     this will always be so.  We might start putting something in
	     the third word of .opd entries.  */
	  if ((sec->flags & SEC_IN_MEMORY) == 0)
	    {
	      bfd_byte *loc;
	      if (!bfd_malloc_and_get_section (ibfd, sec, &loc))
		{
		  free (loc);
		error_ret:
		  if (symtab_hdr->contents != (unsigned char *) local_syms)
		    free (local_syms);
		  if (elf_section_data (sec)->relocs != relstart)
		    free (relstart);
		  return false;
		}
	      sec->contents = loc;
	      sec->flags |= (SEC_IN_MEMORY | SEC_HAS_CONTENTS);
	    }

	  elf_section_data (sec)->relocs = relstart;

	  new_contents = sec->contents;
	  if (add_aux_fields)
	    {
	      new_contents = bfd_malloc (sec->size + cnt_16b * 8);
	      if (new_contents == NULL)
		return false;
	      need_pad = NULL;
	    }
	  wptr = new_contents;
	  rptr = sec->contents;
	  write_rel = relstart;
	  for (rel = relstart; rel < relend; )
	    {
	      unsigned long r_symndx;
	      asection *sym_sec;
	      struct elf_link_hash_entry *h;
	      struct ppc_link_hash_entry *fdh = NULL;
	      Elf_Internal_Sym *sym;
	      long opd_ent_size;
	      Elf_Internal_Rela *next_rel;
	      bool skip;

	      r_symndx = ELF64_R_SYM (rel->r_info);
	      if (!get_sym_h (&h, &sym, &sym_sec, NULL, &local_syms,
			      r_symndx, ibfd))
		goto error_ret;

	      next_rel = rel + 2;
	      if (next_rel + 1 == relend
		  || (next_rel + 2 < relend
		      && ELF64_R_TYPE (next_rel[2].r_info) == R_PPC64_TOC))
		++next_rel;

	      /* See if the .opd entry is full 24 byte or
		 16 byte (with fd_aux entry overlapped with next
		 fd_func).  */
	      opd_ent_size = 24;
	      if (next_rel == relend)
		{
		  if (sec->size == rel->r_offset + 16)
		    opd_ent_size = 16;
		}
	      else if (next_rel->r_offset == rel->r_offset + 16)
		opd_ent_size = 16;

	      if (h != NULL
		  && h->root.root.string[0] == '.')
		{
		  fdh = ppc_elf_hash_entry (h)->oh;
		  if (fdh != NULL)
		    {
		      fdh = ppc_follow_link (fdh);
		      if (fdh->elf.root.type != bfd_link_hash_defined
			  && fdh->elf.root.type != bfd_link_hash_defweak)
			fdh = NULL;
		    }
		}

	      skip = (sym_sec->owner != ibfd
		      || sym_sec->output_section == bfd_abs_section_ptr);
	      if (skip)
		{
		  if (fdh != NULL && sym_sec->owner == ibfd)
		    {
		      /* Arrange for the function descriptor sym
			 to be dropped.  */
		      fdh->elf.root.u.def.value = 0;
		      fdh->elf.root.u.def.section = sym_sec;
		    }
		  opd->adjust[OPD_NDX (rel->r_offset)] = -1;

		  if (NO_OPD_RELOCS || bfd_link_relocatable (info))
		    rel = next_rel;
		  else
		    while (1)
		      {
			if (!dec_dynrel_count (rel, sec, info,
					       NULL, h, sym))
			  goto error_ret;

			if (++rel == next_rel)
			  break;

			r_symndx = ELF64_R_SYM (rel->r_info);
			if (!get_sym_h (&h, &sym, &sym_sec, NULL, &local_syms,
					r_symndx, ibfd))
			  goto error_ret;
		      }
		}
	      else
		{
		  /* We'll be keeping this opd entry.  */
		  long adjust;

		  if (fdh != NULL)
		    {
		      /* Redefine the function descriptor symbol to
			 this location in the opd section.  It is
			 necessary to update the value here rather
			 than using an array of adjustments as we do
			 for local symbols, because various places
			 in the generic ELF code use the value
			 stored in u.def.value.  */
		      fdh->elf.root.u.def.value = wptr - new_contents;
		      fdh->adjust_done = 1;
		    }

		  /* Local syms are a bit tricky.  We could
		     tweak them as they can be cached, but
		     we'd need to look through the local syms
		     for the function descriptor sym which we
		     don't have at the moment.  So keep an
		     array of adjustments.  */
		  adjust = (wptr - new_contents) - (rptr - sec->contents);
		  opd->adjust[OPD_NDX (rel->r_offset)] = adjust;

		  if (wptr != rptr)
		    memcpy (wptr, rptr, opd_ent_size);
		  wptr += opd_ent_size;
		  if (add_aux_fields && opd_ent_size == 16)
		    {
		      memset (wptr, '\0', 8);
		      wptr += 8;
		    }

		  /* We need to adjust any reloc offsets to point to the
		     new opd entries.  */
		  for ( ; rel != next_rel; ++rel)
		    {
		      rel->r_offset += adjust;
		      if (write_rel != rel)
			memcpy (write_rel, rel, sizeof (*rel));
		      ++write_rel;
		    }
		}

	      rptr += opd_ent_size;
	    }

	  sec->size = wptr - new_contents;
	  sec->reloc_count = write_rel - relstart;
	  if (add_aux_fields)
	    {
	      free (sec->contents);
	      sec->contents = new_contents;
	    }

	  /* Fudge the header size too, as this is used later in
	     elf_bfd_final_link if we are emitting relocs.  */
	  rel_hdr = _bfd_elf_single_rel_hdr (sec);
	  rel_hdr->sh_size = sec->reloc_count * rel_hdr->sh_entsize;
	  some_edited = true;
	}
      else if (elf_section_data (sec)->relocs != relstart)
	free (relstart);

      if (local_syms != NULL
	  && symtab_hdr->contents != (unsigned char *) local_syms)
	{
	  if (!info->keep_memory)
	    free (local_syms);
	  else
	    symtab_hdr->contents = (unsigned char *) local_syms;
	}
    }

  if (some_edited)
    elf_link_hash_traverse (elf_hash_table (info), adjust_opd_syms, NULL);

  /* If we are doing a final link and the last .opd entry is just 16 byte
     long, add a 8 byte padding after it.  */
  if (need_pad != NULL && !bfd_link_relocatable (info))
    {
      bfd_byte *p;

      if ((need_pad->flags & SEC_IN_MEMORY) == 0)
	{
	  BFD_ASSERT (need_pad->size > 0);

	  p = bfd_malloc (need_pad->size + 8);
	  if (p == NULL)
	    return false;

	  if (!bfd_get_section_contents (need_pad->owner, need_pad,
					 p, 0, need_pad->size))
	    return false;

	  need_pad->contents = p;
	  need_pad->flags |= (SEC_IN_MEMORY | SEC_HAS_CONTENTS);
	}
      else
	{
	  p = bfd_realloc (need_pad->contents, need_pad->size + 8);
	  if (p == NULL)
	    return false;

	  need_pad->contents = p;
	}

      memset (need_pad->contents + need_pad->size, 0, 8);
      need_pad->size += 8;
    }

  return true;
}

/* Analyze inline PLT call relocations to see whether calls to locally
   defined functions can be converted to direct calls.  */

bool
ppc64_elf_inline_plt (struct bfd_link_info *info)
{
  struct ppc_link_hash_table *htab;
  bfd *ibfd;
  asection *sec;
  bfd_vma low_vma, high_vma, limit;

  htab = ppc_hash_table (info);
  if (htab == NULL)
    return false;

  /* A bl insn can reach -0x2000000 to 0x1fffffc.  The limit is
     reduced somewhat to cater for possible stubs that might be added
     between the call and its destination.  */
  if (htab->params->group_size < 0)
    {
      limit = -htab->params->group_size;
      if (limit == 1)
	limit = 0x1e00000;
    }
  else
    {
      limit = htab->params->group_size;
      if (limit == 1)
	limit = 0x1c00000;
    }

  low_vma = -1;
  high_vma = 0;
  for (sec = info->output_bfd->sections; sec != NULL; sec = sec->next)
    if ((sec->flags & (SEC_ALLOC | SEC_CODE)) == (SEC_ALLOC | SEC_CODE))
      {
	if (low_vma > sec->vma)
	  low_vma = sec->vma;
	if (high_vma < sec->vma + sec->size)
	  high_vma = sec->vma + sec->size;
      }

  /* If a "bl" can reach anywhere in local code sections, then we can
     convert all inline PLT sequences to direct calls when the symbol
     is local.  */
  if (high_vma - low_vma < limit)
    {
      htab->can_convert_all_inline_plt = 1;
      return true;
    }

  /* Otherwise, go looking through relocs for cases where a direct
     call won't reach.  Mark the symbol on any such reloc to disable
     the optimization and keep the PLT entry as it seems likely that
     this will be better than creating trampolines.  Note that this
     will disable the optimization for all inline PLT calls to a
     particular symbol, not just those that won't reach.  The
     difficulty in doing a more precise optimization is that the
     linker needs to make a decision depending on whether a
     particular R_PPC64_PLTCALL insn can be turned into a direct
     call, for each of the R_PPC64_PLTSEQ and R_PPC64_PLT16* insns in
     the sequence, and there is nothing that ties those relocs
     together except their symbol.  */

  for (ibfd = info->input_bfds; ibfd != NULL; ibfd = ibfd->link.next)
    {
      Elf_Internal_Shdr *symtab_hdr;
      Elf_Internal_Sym *local_syms;

      if (!is_ppc64_elf (ibfd))
	continue;

      local_syms = NULL;
      symtab_hdr = &elf_symtab_hdr (ibfd);

      for (sec = ibfd->sections; sec != NULL; sec = sec->next)
	if (ppc64_elf_section_data (sec)->has_pltcall
	    && !bfd_is_abs_section (sec->output_section))
	  {
	    Elf_Internal_Rela *relstart, *rel, *relend;

	    /* Read the relocations.  */
	    relstart = _bfd_elf_link_read_relocs (ibfd, sec, NULL, NULL,
						  info->keep_memory);
	    if (relstart == NULL)
	      return false;

	    relend = relstart + sec->reloc_count;
	    for (rel = relstart; rel < relend; rel++)
	      {
		enum elf_ppc64_reloc_type r_type;
		unsigned long r_symndx;
		asection *sym_sec;
		struct elf_link_hash_entry *h;
		Elf_Internal_Sym *sym;
		unsigned char *tls_maskp;

		r_type = ELF64_R_TYPE (rel->r_info);
		if (r_type != R_PPC64_PLTCALL
		    && r_type != R_PPC64_PLTCALL_NOTOC)
		  continue;

		r_symndx = ELF64_R_SYM (rel->r_info);
		if (!get_sym_h (&h, &sym, &sym_sec, &tls_maskp, &local_syms,
				r_symndx, ibfd))
		  {
		    if (elf_section_data (sec)->relocs != relstart)
		      free (relstart);
		    if (symtab_hdr->contents != (bfd_byte *) local_syms)
		      free (local_syms);
		    return false;
		  }

		if (sym_sec != NULL && sym_sec->output_section != NULL)
		  {
		    bfd_vma from, to;
		    if (h != NULL)
		      to = h->root.u.def.value;
		    else
		      to = sym->st_value;
		    to += (rel->r_addend
			   + sym_sec->output_offset
			   + sym_sec->output_section->vma);
		    from = (rel->r_offset
			    + sec->output_offset
			    + sec->output_section->vma);
		    if (to - from + limit < 2 * limit
			&& !(r_type == R_PPC64_PLTCALL_NOTOC
			     && (((h ? h->other : sym->st_other)
				  & STO_PPC64_LOCAL_MASK)
				 > 1 << STO_PPC64_LOCAL_BIT)))
		      *tls_maskp &= ~PLT_KEEP;
		  }
	      }
	    if (elf_section_data (sec)->relocs != relstart)
	      free (relstart);
	  }

      if (local_syms != NULL
	  && symtab_hdr->contents != (unsigned char *) local_syms)
	{
	  if (!info->keep_memory)
	    free (local_syms);
	  else
	    symtab_hdr->contents = (unsigned char *) local_syms;
	}
    }

  return true;
}

/* Set htab->tls_get_addr and various other info specific to TLS.
   This needs to run before dynamic symbols are processed in
   bfd_elf_size_dynamic_sections.  */

bool
ppc64_elf_tls_setup (struct bfd_link_info *info)
{
  struct ppc_link_hash_table *htab;
  struct elf_link_hash_entry *tga, *tga_fd, *desc, *desc_fd;

  htab = ppc_hash_table (info);
  if (htab == NULL)
    return false;

  /* Move dynamic linking info to the function descriptor sym.  */
  if (htab->need_func_desc_adj)
    {
      elf_link_hash_traverse (&htab->elf, func_desc_adjust, info);
      htab->need_func_desc_adj = 0;
    }

  if (abiversion (info->output_bfd) == 1)
    htab->opd_abi = 1;

  if (htab->params->no_multi_toc)
    htab->do_multi_toc = 0;
  else if (!htab->do_multi_toc)
    htab->params->no_multi_toc = 1;

  /* Default to --no-plt-localentry, as this option can cause problems
     with symbol interposition.  For example, glibc libpthread.so and
     libc.so duplicate many pthread symbols, with a fallback
     implementation in libc.so.  In some cases the fallback does more
     work than the pthread implementation.  __pthread_condattr_destroy
     is one such symbol: the libpthread.so implementation is
     localentry:0 while the libc.so implementation is localentry:8.
     An app that "cleverly" uses dlopen to only load necessary
     libraries at runtime may omit loading libpthread.so when not
     running multi-threaded, which then results in the libc.so
     fallback symbols being used and ld.so complaining.  Now there
     are workarounds in ld (see non_zero_localentry) to detect the
     pthread situation, but that may not be the only case where
     --plt-localentry can cause trouble.  */
  if (htab->params->plt_localentry0 < 0)
    htab->params->plt_localentry0 = 0;
  if (htab->params->plt_localentry0 && htab->has_power10_relocs)
    {
      /* The issue is that __glink_PLTresolve saves r2, which is done
	 because glibc ld.so _dl_runtime_resolve restores r2 to support
	 a glibc plt call optimisation where global entry code is
	 skipped on calls that resolve to the same binary.  The
	 __glink_PLTresolve save of r2 is incompatible with code
	 making tail calls, because the tail call might go via the
	 resolver and thus overwrite the proper saved r2.  */
      _bfd_error_handler (_("warning: --plt-localentry is incompatible with "
			    "power10 pc-relative code"));
      htab->params->plt_localentry0 = 0;
    }
  if (htab->params->plt_localentry0
      && elf_link_hash_lookup (&htab->elf, "GLIBC_2.26",
			       false, false, false) == NULL)
    _bfd_error_handler
      (_("warning: --plt-localentry is especially dangerous without "
	 "ld.so support to detect ABI violations"));

  tga = elf_link_hash_lookup (&htab->elf, ".__tls_get_addr",
			      false, false, true);
  htab->tls_get_addr = ppc_elf_hash_entry (tga);
  tga_fd = elf_link_hash_lookup (&htab->elf, "__tls_get_addr",
				 false, false, true);
  htab->tls_get_addr_fd = ppc_elf_hash_entry (tga_fd);

  desc = elf_link_hash_lookup (&htab->elf, ".__tls_get_addr_desc",
			       false, false, true);
  htab->tga_desc = ppc_elf_hash_entry (desc);
  desc_fd = elf_link_hash_lookup (&htab->elf, "__tls_get_addr_desc",
				  false, false, true);
  htab->tga_desc_fd = ppc_elf_hash_entry (desc_fd);

  if (htab->params->tls_get_addr_opt)
    {
      struct elf_link_hash_entry *opt, *opt_fd;

      opt = elf_link_hash_lookup (&htab->elf, ".__tls_get_addr_opt",
				  false, false, true);
      opt_fd = elf_link_hash_lookup (&htab->elf, "__tls_get_addr_opt",
				     false, false, true);
      if (opt_fd != NULL
	  && (opt_fd->root.type == bfd_link_hash_defined
	      || opt_fd->root.type == bfd_link_hash_defweak))
	{
	  /* If glibc supports an optimized __tls_get_addr call stub,
	     signalled by the presence of __tls_get_addr_opt, and we'll
	     be calling __tls_get_addr via a plt call stub, then
	     make __tls_get_addr point to __tls_get_addr_opt.  */
	  if (!(htab->elf.dynamic_sections_created
		&& tga_fd != NULL
		&& (tga_fd->type == STT_FUNC
		    || tga_fd->needs_plt)
		&& !(SYMBOL_CALLS_LOCAL (info, tga_fd)
		     || UNDEFWEAK_NO_DYNAMIC_RELOC (info, tga_fd))))
	    tga_fd = NULL;
	  if (!(htab->elf.dynamic_sections_created
		&& desc_fd != NULL
		&& (desc_fd->type == STT_FUNC
		    || desc_fd->needs_plt)
		&& !(SYMBOL_CALLS_LOCAL (info, desc_fd)
		     || UNDEFWEAK_NO_DYNAMIC_RELOC (info, desc_fd))))
	    desc_fd = NULL;

	  if (tga_fd != NULL || desc_fd != NULL)
	    {
	      struct plt_entry *ent = NULL;

	      if (tga_fd != NULL)
		for (ent = tga_fd->plt.plist; ent != NULL; ent = ent->next)
		  if (ent->plt.refcount > 0)
		    break;
	      if (ent == NULL && desc_fd != NULL)
		for (ent = desc_fd->plt.plist; ent != NULL; ent = ent->next)
		  if (ent->plt.refcount > 0)
		    break;
	      if (ent != NULL)
		{
		  if (tga_fd != NULL)
		    {
		      tga_fd->root.type = bfd_link_hash_indirect;
		      tga_fd->root.u.i.link = &opt_fd->root;
		      tga_fd->root.u.i.warning = NULL;
		      ppc64_elf_copy_indirect_symbol (info, opt_fd, tga_fd);
		    }
		  if (desc_fd != NULL)
		    {
		      desc_fd->root.type = bfd_link_hash_indirect;
		      desc_fd->root.u.i.link = &opt_fd->root;
		      desc_fd->root.u.i.warning = NULL;
		      ppc64_elf_copy_indirect_symbol (info, opt_fd, desc_fd);
		    }
		  opt_fd->mark = 1;
		  if (opt_fd->dynindx != -1)
		    {
		      /* Use __tls_get_addr_opt in dynamic relocations.  */
		      opt_fd->dynindx = -1;
		      _bfd_elf_strtab_delref (elf_hash_table (info)->dynstr,
					      opt_fd->dynstr_index);
		      if (!bfd_elf_link_record_dynamic_symbol (info, opt_fd))
			return false;
		    }
		  if (tga_fd != NULL)
		    {
		      htab->tls_get_addr_fd = ppc_elf_hash_entry (opt_fd);
		      tga = elf_hash_entry (htab->tls_get_addr);
		      if (opt != NULL && tga != NULL)
			{
			  tga->root.type = bfd_link_hash_indirect;
			  tga->root.u.i.link = &opt->root;
			  tga->root.u.i.warning = NULL;
			  ppc64_elf_copy_indirect_symbol (info, opt, tga);
			  opt->mark = 1;
			  _bfd_elf_link_hash_hide_symbol (info, opt,
							  tga->forced_local);
			  htab->tls_get_addr = ppc_elf_hash_entry (opt);
			}
		      htab->tls_get_addr_fd->oh = htab->tls_get_addr;
		      htab->tls_get_addr_fd->is_func_descriptor = 1;
		      if (htab->tls_get_addr != NULL)
			{
			  htab->tls_get_addr->oh = htab->tls_get_addr_fd;
			  htab->tls_get_addr->is_func = 1;
			}
		    }
		  if (desc_fd != NULL)
		    {
		      htab->tga_desc_fd = ppc_elf_hash_entry (opt_fd);
		      if (opt != NULL && desc != NULL)
			{
			  desc->root.type = bfd_link_hash_indirect;
			  desc->root.u.i.link = &opt->root;
			  desc->root.u.i.warning = NULL;
			  ppc64_elf_copy_indirect_symbol (info, opt, desc);
			  opt->mark = 1;
			  _bfd_elf_link_hash_hide_symbol (info, opt,
							  desc->forced_local);
			  htab->tga_desc = ppc_elf_hash_entry (opt);
			}
		      htab->tga_desc_fd->oh = htab->tga_desc;
		      htab->tga_desc_fd->is_func_descriptor = 1;
		      if (htab->tga_desc != NULL)
			{
			  htab->tga_desc->oh = htab->tga_desc_fd;
			  htab->tga_desc->is_func = 1;
			}
		    }
		}
	    }
	}
      else if (htab->params->tls_get_addr_opt < 0)
	htab->params->tls_get_addr_opt = 0;
    }

  if (htab->tga_desc_fd != NULL
      && htab->params->tls_get_addr_opt
      && htab->params->no_tls_get_addr_regsave == -1)
    htab->params->no_tls_get_addr_regsave = 0;

  return true;
}

/* Return TRUE iff REL is a branch reloc with a global symbol matching
   any of HASH1, HASH2, HASH3, or HASH4.  */

static bool
branch_reloc_hash_match (bfd *ibfd,
			 Elf_Internal_Rela *rel,
			 struct ppc_link_hash_entry *hash1,
			 struct ppc_link_hash_entry *hash2,
			 struct ppc_link_hash_entry *hash3,
			 struct ppc_link_hash_entry *hash4)
{
  Elf_Internal_Shdr *symtab_hdr = &elf_symtab_hdr (ibfd);
  enum elf_ppc64_reloc_type r_type = ELF64_R_TYPE (rel->r_info);
  unsigned int r_symndx = ELF64_R_SYM (rel->r_info);

  if (r_symndx >= symtab_hdr->sh_info && is_branch_reloc (r_type))
    {
      struct elf_link_hash_entry **sym_hashes = elf_sym_hashes (ibfd);
      struct elf_link_hash_entry *h;

      h = sym_hashes[r_symndx - symtab_hdr->sh_info];
      h = elf_follow_link (h);
      if (h == elf_hash_entry (hash1)
	  || h == elf_hash_entry (hash2)
	  || h == elf_hash_entry (hash3)
	  || h == elf_hash_entry (hash4))
	return true;
    }
  return false;
}

/* Run through all the TLS relocs looking for optimization
   opportunities.  The linker has been hacked (see ppc64elf.em) to do
   a preliminary section layout so that we know the TLS segment
   offsets.  We can't optimize earlier because some optimizations need
   to know the tp offset, and we need to optimize before allocating
   dynamic relocations.  */

bool
ppc64_elf_tls_optimize (struct bfd_link_info *info)
{
  bfd *ibfd;
  asection *sec;
  struct ppc_link_hash_table *htab;
  unsigned char *toc_ref;
  int pass;

  if (!bfd_link_executable (info))
    return true;

  htab = ppc_hash_table (info);
  if (htab == NULL)
    return false;

  htab->do_tls_opt = 1;

  /* Make two passes over the relocs.  On the first pass, mark toc
     entries involved with tls relocs, and check that tls relocs
     involved in setting up a tls_get_addr call are indeed followed by
     such a call.  If they are not, we can't do any tls optimization.
     On the second pass twiddle tls_mask flags to notify
     relocate_section that optimization can be done, and adjust got
     and plt refcounts.  */
  toc_ref = NULL;
  for (pass = 0; pass < 2; ++pass)
    for (ibfd = info->input_bfds; ibfd != NULL; ibfd = ibfd->link.next)
      {
	Elf_Internal_Sym *locsyms = NULL;
	asection *toc = bfd_get_section_by_name (ibfd, ".toc");

	for (sec = ibfd->sections; sec != NULL; sec = sec->next)
	  if (sec->has_tls_reloc && !bfd_is_abs_section (sec->output_section))
	    {
	      Elf_Internal_Rela *relstart, *rel, *relend;
	      bool found_tls_get_addr_arg = 0;

	      /* Read the relocations.  */
	      relstart = _bfd_elf_link_read_relocs (ibfd, sec, NULL, NULL,
						    info->keep_memory);
	      if (relstart == NULL)
		{
		  free (toc_ref);
		  return false;
		}

	      relend = relstart + sec->reloc_count;
	      for (rel = relstart; rel < relend; rel++)
		{
		  enum elf_ppc64_reloc_type r_type;
		  unsigned long r_symndx;
		  struct elf_link_hash_entry *h;
		  Elf_Internal_Sym *sym;
		  asection *sym_sec;
		  unsigned char *tls_mask;
		  unsigned int tls_set, tls_clear, tls_type = 0;
		  bfd_vma value;
		  bool ok_tprel, is_local;
		  long toc_ref_index = 0;
		  int expecting_tls_get_addr = 0;
		  bool ret = false;

		  r_symndx = ELF64_R_SYM (rel->r_info);
		  if (!get_sym_h (&h, &sym, &sym_sec, &tls_mask, &locsyms,
				  r_symndx, ibfd))
		    {
		    err_free_rel:
		      if (elf_section_data (sec)->relocs != relstart)
			free (relstart);
		      free (toc_ref);
		      if (elf_symtab_hdr (ibfd).contents
			  != (unsigned char *) locsyms)
			free (locsyms);
		      return ret;
		    }

		  if (h != NULL)
		    {
		      if (h->root.type == bfd_link_hash_defined
			  || h->root.type == bfd_link_hash_defweak)
			value = h->root.u.def.value;
		      else if (h->root.type == bfd_link_hash_undefweak)
			value = 0;
		      else
			{
			  found_tls_get_addr_arg = 0;
			  continue;
			}
		    }
		  else
		    /* Symbols referenced by TLS relocs must be of type
		       STT_TLS.  So no need for .opd local sym adjust.  */
		    value = sym->st_value;

		  ok_tprel = false;
		  is_local = SYMBOL_REFERENCES_LOCAL (info, h);
		  if (is_local)
		    {
		      if (h != NULL
			  && h->root.type == bfd_link_hash_undefweak)
			ok_tprel = true;
		      else if (sym_sec != NULL
			       && sym_sec->output_section != NULL)
			{
			  value += sym_sec->output_offset;
			  value += sym_sec->output_section->vma;
			  value -= htab->elf.tls_sec->vma + TP_OFFSET;
			  /* Note that even though the prefix insns
			     allow a 1<<33 offset we use the same test
			     as for addis;addi.  There may be a mix of
			     pcrel and non-pcrel code and the decision
			     to optimise is per symbol, not per TLS
			     sequence.  */
			  ok_tprel = value + 0x80008000ULL < 1ULL << 32;
			}
		    }

		  r_type = ELF64_R_TYPE (rel->r_info);
		  /* If this section has old-style __tls_get_addr calls
		     without marker relocs, then check that each
		     __tls_get_addr call reloc is preceded by a reloc
		     that conceivably belongs to the __tls_get_addr arg
		     setup insn.  If we don't find matching arg setup
		     relocs, don't do any tls optimization.  */
		  if (pass == 0
		      && sec->nomark_tls_get_addr
		      && h != NULL
		      && is_tls_get_addr (h, htab)
		      && !found_tls_get_addr_arg
		      && is_branch_reloc (r_type))
		    {
		      info->callbacks->minfo (_("%H __tls_get_addr lost arg, "
						"TLS optimization disabled\n"),
					      ibfd, sec, rel->r_offset);
		      ret = true;
		      goto err_free_rel;
		    }

		  found_tls_get_addr_arg = 0;
		  switch (r_type)
		    {
		    case R_PPC64_GOT_TLSLD16:
		    case R_PPC64_GOT_TLSLD16_LO:
		    case R_PPC64_GOT_TLSLD_PCREL34:
		      expecting_tls_get_addr = 1;
		      found_tls_get_addr_arg = 1;
		      /* Fall through.  */

		    case R_PPC64_GOT_TLSLD16_HI:
		    case R_PPC64_GOT_TLSLD16_HA:
		      /* These relocs should never be against a symbol
			 defined in a shared lib.  Leave them alone if
			 that turns out to be the case.  */
		      if (!is_local)
			continue;

		      /* LD -> LE */
		      tls_set = 0;
		      tls_clear = TLS_LD;
		      tls_type = TLS_TLS | TLS_LD;
		      break;

		    case R_PPC64_GOT_TLSGD16:
		    case R_PPC64_GOT_TLSGD16_LO:
		    case R_PPC64_GOT_TLSGD_PCREL34:
		      expecting_tls_get_addr = 1;
		      found_tls_get_addr_arg = 1;
		      /* Fall through. */

		    case R_PPC64_GOT_TLSGD16_HI:
		    case R_PPC64_GOT_TLSGD16_HA:
		      if (ok_tprel)
			/* GD -> LE */
			tls_set = 0;
		      else
			/* GD -> IE */
			tls_set = TLS_TLS | TLS_GDIE;
		      tls_clear = TLS_GD;
		      tls_type = TLS_TLS | TLS_GD;
		      break;

		    case R_PPC64_GOT_TPREL_PCREL34:
		    case R_PPC64_GOT_TPREL16_DS:
		    case R_PPC64_GOT_TPREL16_LO_DS:
		    case R_PPC64_GOT_TPREL16_HI:
		    case R_PPC64_GOT_TPREL16_HA:
		      if (ok_tprel)
			{
			  /* IE -> LE */
			  tls_set = 0;
			  tls_clear = TLS_TPREL;
			  tls_type = TLS_TLS | TLS_TPREL;
			  break;
			}
		      continue;

		    case R_PPC64_TLSLD:
		      if (!is_local)
			continue;
		      /* Fall through.  */
		    case R_PPC64_TLSGD:
		      if (rel + 1 < relend
			  && is_plt_seq_reloc (ELF64_R_TYPE (rel[1].r_info)))
			{
			  if (pass != 0
			      && (ELF64_R_TYPE (rel[1].r_info)
				  != R_PPC64_PLTSEQ)
			      && (ELF64_R_TYPE (rel[1].r_info)
				  != R_PPC64_PLTSEQ_NOTOC))
			    {
			      r_symndx = ELF64_R_SYM (rel[1].r_info);
			      if (!get_sym_h (&h, NULL, NULL, NULL, &locsyms,
					      r_symndx, ibfd))
				goto err_free_rel;
			      if (h != NULL)
				{
				  struct plt_entry *ent = NULL;

				  for (ent = h->plt.plist;
				       ent != NULL;
				       ent = ent->next)
				    if (ent->addend == rel[1].r_addend)
				      break;

				  if (ent != NULL
				      && ent->plt.refcount > 0)
				    ent->plt.refcount -= 1;
				}
			    }
			  continue;
			}
		      found_tls_get_addr_arg = 1;
		      /* Fall through.  */

		    case R_PPC64_TLS:
		    case R_PPC64_TOC16:
		    case R_PPC64_TOC16_LO:
		      if (sym_sec == NULL || sym_sec != toc)
			continue;

		      /* Mark this toc entry as referenced by a TLS
			 code sequence.  We can do that now in the
			 case of R_PPC64_TLS, and after checking for
			 tls_get_addr for the TOC16 relocs.  */
		      if (toc_ref == NULL)
			toc_ref
			  = bfd_zmalloc (toc->output_section->rawsize / 8);
		      if (toc_ref == NULL)
			goto err_free_rel;

		      if (h != NULL)
			value = h->root.u.def.value;
		      else
			value = sym->st_value;
		      value += rel->r_addend;
		      if (value % 8 != 0)
			continue;
		      BFD_ASSERT (value < toc->size
				  && toc->output_offset % 8 == 0);
		      toc_ref_index = (value + toc->output_offset) / 8;
		      if (r_type == R_PPC64_TLS
			  || r_type == R_PPC64_TLSGD
			  || r_type == R_PPC64_TLSLD)
			{
			  toc_ref[toc_ref_index] = 1;
			  continue;
			}

		      if (pass != 0 && toc_ref[toc_ref_index] == 0)
			continue;

		      tls_set = 0;
		      tls_clear = 0;
		      expecting_tls_get_addr = 2;
		      break;

		    case R_PPC64_TPREL64:
		      if (pass == 0
			  || sec != toc
			  || toc_ref == NULL
			  || !toc_ref[(rel->r_offset + toc->output_offset) / 8])
			continue;
		      if (ok_tprel)
			{
			  /* IE -> LE */
			  tls_set = TLS_EXPLICIT;
			  tls_clear = TLS_TPREL;
			  break;
			}
		      continue;

		    case R_PPC64_DTPMOD64:
		      if (pass == 0
			  || sec != toc
			  || toc_ref == NULL
			  || !toc_ref[(rel->r_offset + toc->output_offset) / 8])
			continue;
		      if (rel + 1 < relend
			  && (rel[1].r_info
			      == ELF64_R_INFO (r_symndx, R_PPC64_DTPREL64))
			  && rel[1].r_offset == rel->r_offset + 8)
			{
			  if (ok_tprel)
			    /* GD -> LE */
			    tls_set = TLS_EXPLICIT | TLS_GD;
			  else
			    /* GD -> IE */
			    tls_set = TLS_EXPLICIT | TLS_GD | TLS_GDIE;
			  tls_clear = TLS_GD;
			}
		      else
			{
			  if (!is_local)
			    continue;

			  /* LD -> LE */
			  tls_set = TLS_EXPLICIT;
			  tls_clear = TLS_LD;
			}
		      break;

		    case R_PPC64_TPREL16_HA:
		      if (pass == 0)
			{
			  unsigned char buf[4];
			  unsigned int insn;
			  bfd_vma off = rel->r_offset & ~3;
			  if (!bfd_get_section_contents (ibfd, sec, buf,
							 off, 4))
			    goto err_free_rel;
			  insn = bfd_get_32 (ibfd, buf);
			  /* addis rt,13,imm */
			  if ((insn & ((0x3fu << 26) | 0x1f << 16))
			      != ((15u << 26) | (13 << 16)))
			    {
			      /* xgettext:c-format */
			      info->callbacks->minfo
				(_("%H: warning: %s unexpected insn %#x.\n"),
				 ibfd, sec, off, "R_PPC64_TPREL16_HA", insn);
			      htab->do_tls_opt = 0;
			    }
			}
		      continue;

		    case R_PPC64_TPREL16_HI:
		    case R_PPC64_TPREL16_HIGH:
		    case R_PPC64_TPREL16_HIGHA:
		    case R_PPC64_TPREL16_HIGHER:
		    case R_PPC64_TPREL16_HIGHERA:
		    case R_PPC64_TPREL16_HIGHEST:
		    case R_PPC64_TPREL16_HIGHESTA:
		      /* These can all be used in sequences along with
			 TPREL16_LO or TPREL16_LO_DS in ways we aren't
			 able to verify easily.  */
		      htab->do_tls_opt = 0;
		      continue;

		    default:
		      continue;
		    }

		  if (pass == 0)
		    {
		      if (!expecting_tls_get_addr
			  || !sec->nomark_tls_get_addr)
			continue;

		      if (rel + 1 < relend
			  && branch_reloc_hash_match (ibfd, rel + 1,
						      htab->tls_get_addr_fd,
						      htab->tga_desc_fd,
						      htab->tls_get_addr,
						      htab->tga_desc))
			{
			  if (expecting_tls_get_addr == 2)
			    {
			      /* Check for toc tls entries.  */
			      unsigned char *toc_tls;
			      int retval;

			      retval = get_tls_mask (&toc_tls, NULL, NULL,
						     &locsyms,
						     rel, ibfd);
			      if (retval == 0)
				goto err_free_rel;
			      if (toc_tls != NULL)
				{
				  if ((*toc_tls & TLS_TLS) != 0
				      && ((*toc_tls & (TLS_GD | TLS_LD)) != 0))
				    found_tls_get_addr_arg = 1;
				  if (retval > 1)
				    toc_ref[toc_ref_index] = 1;
				}
			    }
			  continue;
			}

		      /* Uh oh, we didn't find the expected call.  We
			 could just mark this symbol to exclude it
			 from tls optimization but it's safer to skip
			 the entire optimization.  */
		      /* xgettext:c-format */
		      info->callbacks->minfo (_("%H arg lost __tls_get_addr, "
						"TLS optimization disabled\n"),
					      ibfd, sec, rel->r_offset);
		      ret = true;
		      goto err_free_rel;
		    }

		  /* If we don't have old-style __tls_get_addr calls
		     without TLSGD/TLSLD marker relocs, and we haven't
		     found a new-style __tls_get_addr call with a
		     marker for this symbol, then we either have a
		     broken object file or an -mlongcall style
		     indirect call to __tls_get_addr without a marker.
		     Disable optimization in this case.  */
		  if ((tls_clear & (TLS_GD | TLS_LD)) != 0
		      && (tls_set & TLS_EXPLICIT) == 0
		      && !sec->nomark_tls_get_addr
		      && ((*tls_mask & (TLS_TLS | TLS_MARK))
			  != (TLS_TLS | TLS_MARK)))
		    continue;

		  if (expecting_tls_get_addr == 1 + !sec->nomark_tls_get_addr)
		    {
		      struct plt_entry *ent = NULL;

		      if (htab->tls_get_addr_fd != NULL)
			for (ent = htab->tls_get_addr_fd->elf.plt.plist;
			     ent != NULL;
			     ent = ent->next)
			  if (ent->addend == 0)
			    break;

		      if (ent == NULL && htab->tga_desc_fd != NULL)
			for (ent = htab->tga_desc_fd->elf.plt.plist;
			     ent != NULL;
			     ent = ent->next)
			  if (ent->addend == 0)
			    break;

		      if (ent == NULL && htab->tls_get_addr != NULL)
			for (ent = htab->tls_get_addr->elf.plt.plist;
			     ent != NULL;
			     ent = ent->next)
			  if (ent->addend == 0)
			    break;

		      if (ent == NULL && htab->tga_desc != NULL)
			for (ent = htab->tga_desc->elf.plt.plist;
			     ent != NULL;
			     ent = ent->next)
			  if (ent->addend == 0)
			    break;

		      if (ent != NULL
			  && ent->plt.refcount > 0)
			ent->plt.refcount -= 1;
		    }

		  if (tls_clear == 0)
		    continue;

		  if ((tls_set & TLS_EXPLICIT) == 0)
		    {
		      struct got_entry *ent;

		      /* Adjust got entry for this reloc.  */
		      if (h != NULL)
			ent = h->got.glist;
		      else
			ent = elf_local_got_ents (ibfd)[r_symndx];

		      for (; ent != NULL; ent = ent->next)
			if (ent->addend == rel->r_addend
			    && ent->owner == ibfd
			    && ent->tls_type == tls_type)
			  break;
		      if (ent == NULL)
			abort ();

		      if (tls_set == 0)
			{
			  /* We managed to get rid of a got entry.  */
			  if (ent->got.refcount > 0)
			    ent->got.refcount -= 1;
			}
		    }
		  else
		    {
		      /* If we got rid of a DTPMOD/DTPREL reloc pair then
			 we'll lose one or two dyn relocs.  */
		      if (!dec_dynrel_count (rel, sec, info,
					     NULL, h, sym))
			return false;

		      if (tls_set == (TLS_EXPLICIT | TLS_GD))
			{
			  if (!dec_dynrel_count (rel + 1, sec, info,
						 NULL, h, sym))
			    return false;
			}
		    }

		  *tls_mask |= tls_set & 0xff;
		  *tls_mask &= ~tls_clear;
		}

	      if (elf_section_data (sec)->relocs != relstart)
		free (relstart);
	    }

	if (locsyms != NULL
	    && (elf_symtab_hdr (ibfd).contents != (unsigned char *) locsyms))
	  {
	    if (!info->keep_memory)
	      free (locsyms);
	    else
	      elf_symtab_hdr (ibfd).contents = (unsigned char *) locsyms;
	  }
      }

  free (toc_ref);
  return true;
}

/* Called via elf_link_hash_traverse from ppc64_elf_edit_toc to adjust
   the values of any global symbols in a toc section that has been
   edited.  Globals in toc sections should be a rarity, so this function
   sets a flag if any are found in toc sections other than the one just
   edited, so that further hash table traversals can be avoided.  */

struct adjust_toc_info
{
  asection *toc;
  unsigned long *skip;
  bool global_toc_syms;
};

enum toc_skip_enum { ref_from_discarded = 1, can_optimize = 2 };

static bool
adjust_toc_syms (struct elf_link_hash_entry *h, void *inf)
{
  struct ppc_link_hash_entry *eh;
  struct adjust_toc_info *toc_inf = (struct adjust_toc_info *) inf;
  unsigned long i;

  if (h->root.type != bfd_link_hash_defined
      && h->root.type != bfd_link_hash_defweak)
    return true;

  eh = ppc_elf_hash_entry (h);
  if (eh->adjust_done)
    return true;

  if (eh->elf.root.u.def.section == toc_inf->toc)
    {
      if (eh->elf.root.u.def.value > toc_inf->toc->rawsize)
	i = toc_inf->toc->rawsize >> 3;
      else
	i = eh->elf.root.u.def.value >> 3;

      if ((toc_inf->skip[i] & (ref_from_discarded | can_optimize)) != 0)
	{
	  _bfd_error_handler
	    (_("%s defined on removed toc entry"), eh->elf.root.root.string);
	  do
	    ++i;
	  while ((toc_inf->skip[i] & (ref_from_discarded | can_optimize)) != 0);
	  eh->elf.root.u.def.value = (bfd_vma) i << 3;
	}

      eh->elf.root.u.def.value -= toc_inf->skip[i];
      eh->adjust_done = 1;
    }
  else if (strcmp (eh->elf.root.u.def.section->name, ".toc") == 0)
    toc_inf->global_toc_syms = true;

  return true;
}

/* Return TRUE iff INSN with a relocation of R_TYPE is one we expect
   on a _LO variety toc/got reloc.  */

static bool
ok_lo_toc_insn (unsigned int insn, enum elf_ppc64_reloc_type r_type)
{
  return ((insn & (0x3fu << 26)) == 12u << 26 /* addic */
	  || (insn & (0x3fu << 26)) == 14u << 26 /* addi */
	  || (insn & (0x3fu << 26)) == 32u << 26 /* lwz */
	  || (insn & (0x3fu << 26)) == 34u << 26 /* lbz */
	  || (insn & (0x3fu << 26)) == 36u << 26 /* stw */
	  || (insn & (0x3fu << 26)) == 38u << 26 /* stb */
	  || (insn & (0x3fu << 26)) == 40u << 26 /* lhz */
	  || (insn & (0x3fu << 26)) == 42u << 26 /* lha */
	  || (insn & (0x3fu << 26)) == 44u << 26 /* sth */
	  || (insn & (0x3fu << 26)) == 46u << 26 /* lmw */
	  || (insn & (0x3fu << 26)) == 47u << 26 /* stmw */
	  || (insn & (0x3fu << 26)) == 48u << 26 /* lfs */
	  || (insn & (0x3fu << 26)) == 50u << 26 /* lfd */
	  || (insn & (0x3fu << 26)) == 52u << 26 /* stfs */
	  || (insn & (0x3fu << 26)) == 54u << 26 /* stfd */
	  || (insn & (0x3fu << 26)) == 56u << 26 /* lq,lfq */
	  || ((insn & (0x3fu << 26)) == 57u << 26 /* lxsd,lxssp,lfdp */
	      /* Exclude lfqu by testing reloc.  If relocs are ever
		 defined for the reduced D field in psq_lu then those
		 will need testing too.  */
	      && r_type != R_PPC64_TOC16_LO && r_type != R_PPC64_GOT16_LO)
	  || ((insn & (0x3fu << 26)) == 58u << 26 /* ld,lwa */
	      && (insn & 1) == 0)
	  || (insn & (0x3fu << 26)) == 60u << 26 /* stfq */
	  || ((insn & (0x3fu << 26)) == 61u << 26 /* lxv,stx{v,sd,ssp},stfdp */
	      /* Exclude stfqu.  psq_stu as above for psq_lu.  */
	      && r_type != R_PPC64_TOC16_LO && r_type != R_PPC64_GOT16_LO)
	  || ((insn & (0x3fu << 26)) == 62u << 26 /* std,stq */
	      && (insn & 1) == 0));
}

/* PCREL_OPT in one instance flags to the linker that a pair of insns:
     pld ra,symbol@got@pcrel
     load/store rt,off(ra)
   or
     pla ra,symbol@pcrel
     load/store rt,off(ra)
   may be translated to
     pload/pstore rt,symbol+off@pcrel
     nop.
   This function returns true if the optimization is possible, placing
   the prefix insn in *PINSN1, a NOP in *PINSN2 and the offset in *POFF.

   On entry to this function, the linker has already determined that
   the pld can be replaced with pla: *PINSN1 is that pla insn,
   while *PINSN2 is the second instruction.  */

static bool
xlate_pcrel_opt (uint64_t *pinsn1, uint64_t *pinsn2, bfd_signed_vma *poff)
{
  uint64_t insn1 = *pinsn1;
  uint64_t insn2 = *pinsn2;
  bfd_signed_vma off;

  if ((insn2 & (63ULL << 58)) == 1ULL << 58)
    {
      /* Check that regs match.  */
      if (((insn2 >> 16) & 31) != ((insn1 >> 21) & 31))
	return false;

      /* P8LS or PMLS form, non-pcrel.  */
      if ((insn2 & (-1ULL << 50) & ~(1ULL << 56)) != (1ULL << 58))
	return false;

      *pinsn1 = (insn2 & ~(31 << 16) & ~0x3ffff0000ffffULL) | (1ULL << 52);
      *pinsn2 = PNOP;
      off = ((insn2 >> 16) & 0x3ffff0000ULL) | (insn2 & 0xffff);
      *poff = (off ^ 0x200000000ULL) - 0x200000000ULL;
      return true;
    }

  insn2 >>= 32;

  /* Check that regs match.  */
  if (((insn2 >> 16) & 31) != ((insn1 >> 21) & 31))
    return false;

  switch ((insn2 >> 26) & 63)
    {
    default:
      return false;

    case 32: /* lwz */
    case 34: /* lbz */
    case 36: /* stw */
    case 38: /* stb */
    case 40: /* lhz */
    case 42: /* lha */
    case 44: /* sth */
    case 48: /* lfs */
    case 50: /* lfd */
    case 52: /* stfs */
    case 54: /* stfd */
      /* These are the PMLS cases, where we just need to tack a prefix
	 on the insn.  */
      insn1 = ((1ULL << 58) | (2ULL << 56) | (1ULL << 52)
	       | (insn2 & ((63ULL << 26) | (31ULL << 21))));
      off = insn2 & 0xffff;
      break;

    case 58: /* lwa, ld */
      if ((insn2 & 1) != 0)
	return false;
      insn1 = ((1ULL << 58) | (1ULL << 52)
	       | (insn2 & 2 ? 41ULL << 26 : 57ULL << 26)
	       | (insn2 & (31ULL << 21)));
      off = insn2 & 0xfffc;
      break;

    case 57: /* lxsd, lxssp */
      if ((insn2 & 3) < 2)
	return false;
      insn1 = ((1ULL << 58) | (1ULL << 52)
	       | ((40ULL | (insn2 & 3)) << 26)
	       | (insn2 & (31ULL << 21)));
      off = insn2 & 0xfffc;
      break;

    case 61: /* stxsd, stxssp, lxv, stxv  */
      if ((insn2 & 3) == 0)
	return false;
      else if ((insn2 & 3) >= 2)
	{
	  insn1 = ((1ULL << 58) | (1ULL << 52)
		   | ((44ULL | (insn2 & 3)) << 26)
		   | (insn2 & (31ULL << 21)));
	  off = insn2 & 0xfffc;
	}
      else
	{
	  insn1 = ((1ULL << 58) | (1ULL << 52)
		   | ((50ULL | (insn2 & 4) | ((insn2 & 8) >> 3)) << 26)
		   | (insn2 & (31ULL << 21)));
	  off = insn2 & 0xfff0;
	}
      break;

    case 56: /* lq */
      insn1 = ((1ULL << 58) | (1ULL << 52)
	       | (insn2 & ((63ULL << 26) | (31ULL << 21))));
      off = insn2 & 0xffff;
      break;

    case 6: /* lxvp, stxvp */
      if ((insn2 & 0xe) != 0)
	return false;
      insn1 = ((1ULL << 58) | (1ULL << 52)
	       | ((insn2 & 1) == 0 ? 58ULL << 26 : 62ULL << 26)
	       | (insn2 & (31ULL << 21)));
      off = insn2 & 0xfff0;
      break;

    case 62: /* std, stq */
      if ((insn2 & 1) != 0)
	return false;
      insn1 = ((1ULL << 58) | (1ULL << 52)
	       | ((insn2 & 2) == 0 ? 61ULL << 26 : 60ULL << 26)
	       | (insn2 & (31ULL << 21)));
      off = insn2 & 0xfffc;
      break;
    }

  *pinsn1 = insn1;
  *pinsn2 = (uint64_t) NOP << 32;
  *poff = (off ^ 0x8000) - 0x8000;
  return true;
}

/* Examine all relocs referencing .toc sections in order to remove
   unused .toc entries.  */

bool
ppc64_elf_edit_toc (struct bfd_link_info *info)
{
  bfd *ibfd;
  struct adjust_toc_info toc_inf;
  struct ppc_link_hash_table *htab = ppc_hash_table (info);

  htab->do_toc_opt = 1;
  toc_inf.global_toc_syms = true;
  for (ibfd = info->input_bfds; ibfd != NULL; ibfd = ibfd->link.next)
    {
      asection *toc, *sec;
      Elf_Internal_Shdr *symtab_hdr;
      Elf_Internal_Sym *local_syms;
      Elf_Internal_Rela *relstart, *rel, *toc_relocs;
      unsigned long *skip, *drop;
      unsigned char *used;
      unsigned char *keep, last, some_unused;

      if (!is_ppc64_elf (ibfd))
	continue;

      toc = bfd_get_section_by_name (ibfd, ".toc");
      if (toc == NULL
	  || toc->size == 0
	  || (toc->flags & SEC_HAS_CONTENTS) == 0
	  || toc->sec_info_type == SEC_INFO_TYPE_JUST_SYMS
	  || discarded_section (toc))
	continue;

      toc_relocs = NULL;
      local_syms = NULL;
      symtab_hdr = &elf_symtab_hdr (ibfd);

      /* Look at sections dropped from the final link.  */
      skip = NULL;
      relstart = NULL;
      for (sec = ibfd->sections; sec != NULL; sec = sec->next)
	{
	  if (sec->reloc_count == 0
	      || !discarded_section (sec)
	      || get_opd_info (sec)
	      || (sec->flags & SEC_ALLOC) == 0
	      || (sec->flags & SEC_DEBUGGING) != 0)
	    continue;

	  relstart = _bfd_elf_link_read_relocs (ibfd, sec, NULL, NULL, false);
	  if (relstart == NULL)
	    goto error_ret;

	  /* Run through the relocs to see which toc entries might be
	     unused.  */
	  for (rel = relstart; rel < relstart + sec->reloc_count; ++rel)
	    {
	      enum elf_ppc64_reloc_type r_type;
	      unsigned long r_symndx;
	      asection *sym_sec;
	      struct elf_link_hash_entry *h;
	      Elf_Internal_Sym *sym;
	      bfd_vma val;

	      r_type = ELF64_R_TYPE (rel->r_info);
	      switch (r_type)
		{
		default:
		  continue;

		case R_PPC64_TOC16:
		case R_PPC64_TOC16_LO:
		case R_PPC64_TOC16_HI:
		case R_PPC64_TOC16_HA:
		case R_PPC64_TOC16_DS:
		case R_PPC64_TOC16_LO_DS:
		  break;
		}

	      r_symndx = ELF64_R_SYM (rel->r_info);
	      if (!get_sym_h (&h, &sym, &sym_sec, NULL, &local_syms,
			      r_symndx, ibfd))
		goto error_ret;

	      if (sym_sec != toc)
		continue;

	      if (h != NULL)
		val = h->root.u.def.value;
	      else
		val = sym->st_value;
	      val += rel->r_addend;

	      if (val >= toc->size)
		continue;

	      /* Anything in the toc ought to be aligned to 8 bytes.
		 If not, don't mark as unused.  */
	      if (val & 7)
		continue;

	      if (skip == NULL)
		{
		  skip = bfd_zmalloc (sizeof (*skip) * (toc->size + 15) / 8);
		  if (skip == NULL)
		    goto error_ret;
		}

	      skip[val >> 3] = ref_from_discarded;
	    }

	  if (elf_section_data (sec)->relocs != relstart)
	    free (relstart);
	}

      /* For largetoc loads of address constants, we can convert
	 .  addis rx,2,addr@got@ha
	 .  ld ry,addr@got@l(rx)
	 to
	 .  addis rx,2,addr@toc@ha
	 .  addi ry,rx,addr@toc@l
	 when addr is within 2G of the toc pointer.  This then means
	 that the word storing "addr" in the toc is no longer needed.  */

      if (!ppc64_elf_tdata (ibfd)->has_small_toc_reloc
	  && toc->output_section->rawsize < (bfd_vma) 1 << 31
	  && toc->reloc_count != 0)
	{
	  /* Read toc relocs.  */
	  toc_relocs = _bfd_elf_link_read_relocs (ibfd, toc, NULL, NULL,
						  info->keep_memory);
	  if (toc_relocs == NULL)
	    goto error_ret;

	  for (rel = toc_relocs; rel < toc_relocs + toc->reloc_count; ++rel)
	    {
	      enum elf_ppc64_reloc_type r_type;
	      unsigned long r_symndx;
	      asection *sym_sec;
	      struct elf_link_hash_entry *h;
	      Elf_Internal_Sym *sym;
	      bfd_vma val, addr;

	      r_type = ELF64_R_TYPE (rel->r_info);
	      if (r_type != R_PPC64_ADDR64)
		continue;

	      r_symndx = ELF64_R_SYM (rel->r_info);
	      if (!get_sym_h (&h, &sym, &sym_sec, NULL, &local_syms,
			      r_symndx, ibfd))
		goto error_ret;

	      if (sym_sec == NULL
		  || sym_sec->output_section == NULL
		  || discarded_section (sym_sec))
		continue;

	      if (!SYMBOL_REFERENCES_LOCAL (info, h)
		  || (bfd_link_pic (info)
		      && sym_sec == bfd_abs_section_ptr))
		continue;

	      if (h != NULL)
		{
		  if (h->type == STT_GNU_IFUNC)
		    continue;
		  val = h->root.u.def.value;
		}
	      else
		{
		  if (ELF_ST_TYPE (sym->st_info) == STT_GNU_IFUNC)
		    continue;
		  val = sym->st_value;
		}
	      val += rel->r_addend;
	      val += sym_sec->output_section->vma + sym_sec->output_offset;

	      /* We don't yet know the exact toc pointer value, but we
		 know it will be somewhere in the toc section.  Don't
		 optimize if the difference from any possible toc
		 pointer is outside [ff..f80008000, 7fff7fff].  */
	      addr = toc->output_section->vma + TOC_BASE_OFF;
	      if (val - addr + (bfd_vma) 0x80008000 >= (bfd_vma) 1 << 32)
		continue;

	      addr = toc->output_section->vma + toc->output_section->rawsize;
	      if (val - addr + (bfd_vma) 0x80008000 >= (bfd_vma) 1 << 32)
		continue;

	      if (skip == NULL)
		{
		  skip = bfd_zmalloc (sizeof (*skip) * (toc->size + 15) / 8);
		  if (skip == NULL)
		    goto error_ret;
		}

	      skip[rel->r_offset >> 3]
		|= can_optimize | ((rel - toc_relocs) << 2);
	    }
	}

      if (skip == NULL)
	continue;

      used = bfd_zmalloc (sizeof (*used) * (toc->size + 7) / 8);
      if (used == NULL)
	{
	error_ret:
	  if (symtab_hdr->contents != (unsigned char *) local_syms)
	    free (local_syms);
	  if (sec != NULL
	      && elf_section_data (sec)->relocs != relstart)
	    free (relstart);
	  if (elf_section_data (toc)->relocs != toc_relocs)
	    free (toc_relocs);
	  free (skip);
	  return false;
	}

      /* Now check all kept sections that might reference the toc.
	 Check the toc itself last.  */
      for (sec = (ibfd->sections == toc && toc->next ? toc->next
		  : ibfd->sections);
	   sec != NULL;
	   sec = (sec == toc ? NULL
		  : sec->next == NULL ? toc
		  : sec->next == toc && toc->next ? toc->next
		  : sec->next))
	{
	  int repeat;

	  if (sec->reloc_count == 0
	      || discarded_section (sec)
	      || get_opd_info (sec)
	      || (sec->flags & SEC_ALLOC) == 0
	      || (sec->flags & SEC_DEBUGGING) != 0)
	    continue;

	  relstart = _bfd_elf_link_read_relocs (ibfd, sec, NULL, NULL,
						info->keep_memory);
	  if (relstart == NULL)
	    {
	      free (used);
	      goto error_ret;
	    }

	  /* Mark toc entries referenced as used.  */
	  do
	    {
	      repeat = 0;
	      for (rel = relstart; rel < relstart + sec->reloc_count; ++rel)
		{
		  enum elf_ppc64_reloc_type r_type;
		  unsigned long r_symndx;
		  asection *sym_sec;
		  struct elf_link_hash_entry *h;
		  Elf_Internal_Sym *sym;
		  bfd_vma val;

		  r_type = ELF64_R_TYPE (rel->r_info);
		  switch (r_type)
		    {
		    case R_PPC64_TOC16:
		    case R_PPC64_TOC16_LO:
		    case R_PPC64_TOC16_HI:
		    case R_PPC64_TOC16_HA:
		    case R_PPC64_TOC16_DS:
		    case R_PPC64_TOC16_LO_DS:
		      /* In case we're taking addresses of toc entries.  */
		    case R_PPC64_ADDR64:
		      break;

		    default:
		      continue;
		    }

		  r_symndx = ELF64_R_SYM (rel->r_info);
		  if (!get_sym_h (&h, &sym, &sym_sec, NULL, &local_syms,
				  r_symndx, ibfd))
		    {
		      free (used);
		      goto error_ret;
		    }

		  if (sym_sec != toc)
		    continue;

		  if (h != NULL)
		    val = h->root.u.def.value;
		  else
		    val = sym->st_value;
		  val += rel->r_addend;

		  if (val >= toc->size)
		    continue;

		  if ((skip[val >> 3] & can_optimize) != 0)
		    {
		      bfd_vma off;
		      unsigned char opc;

		      switch (r_type)
			{
			case R_PPC64_TOC16_HA:
			  break;

			case R_PPC64_TOC16_LO_DS:
			  off = rel->r_offset;
			  off += (bfd_big_endian (ibfd) ? -2 : 3);
			  if (!bfd_get_section_contents (ibfd, sec, &opc,
							 off, 1))
			    {
			      free (used);
			      goto error_ret;
			    }
			  if ((opc & (0x3f << 2)) == (58u << 2))
			    break;
			  /* Fall through.  */

			default:
			  /* Wrong sort of reloc, or not a ld.  We may
			     as well clear ref_from_discarded too.  */
			  skip[val >> 3] = 0;
			}
		    }

		  if (sec != toc)
		    used[val >> 3] = 1;
		  /* For the toc section, we only mark as used if this
		     entry itself isn't unused.  */
		  else if ((used[rel->r_offset >> 3]
			    || !(skip[rel->r_offset >> 3] & ref_from_discarded))
			   && !used[val >> 3])
		    {
		      /* Do all the relocs again, to catch reference
			 chains.  */
		      repeat = 1;
		      used[val >> 3] = 1;
		    }
		}
	    }
	  while (repeat);

	  if (elf_section_data (sec)->relocs != relstart)
	    free (relstart);
	}

      /* Merge the used and skip arrays.  Assume that TOC
	 doublewords not appearing as either used or unused belong
	 to an entry more than one doubleword in size.  */
      for (drop = skip, keep = used, last = 0, some_unused = 0;
	   drop < skip + (toc->size + 7) / 8;
	   ++drop, ++keep)
	{
	  if (*keep)
	    {
	      *drop &= ~ref_from_discarded;
	      if ((*drop & can_optimize) != 0)
		some_unused = 1;
	      last = 0;
	    }
	  else if ((*drop & ref_from_discarded) != 0)
	    {
	      some_unused = 1;
	      last = ref_from_discarded;
	    }
	  else
	    *drop = last;
	}

      free (used);

      if (some_unused)
	{
	  bfd_byte *contents, *src;
	  unsigned long off;
	  Elf_Internal_Sym *sym;
	  bool local_toc_syms = false;

	  /* Shuffle the toc contents, and at the same time convert the
	     skip array from booleans into offsets.  */
	  if (!bfd_malloc_and_get_section (ibfd, toc, &contents))
	    goto error_ret;

	  elf_section_data (toc)->this_hdr.contents = contents;

	  for (src = contents, off = 0, drop = skip;
	       src < contents + toc->size;
	       src += 8, ++drop)
	    {
	      if ((*drop & (can_optimize | ref_from_discarded)) != 0)
		off += 8;
	      else if (off != 0)
		{
		  *drop = off;
		  memcpy (src - off, src, 8);
		}
	    }
	  *drop = off;
	  toc->rawsize = toc->size;
	  toc->size = src - contents - off;

	  /* Adjust addends for relocs against the toc section sym,
	     and optimize any accesses we can.  */
	  for (sec = ibfd->sections; sec != NULL; sec = sec->next)
	    {
	      if (sec->reloc_count == 0
		  || discarded_section (sec))
		continue;

	      relstart = _bfd_elf_link_read_relocs (ibfd, sec, NULL, NULL,
						    info->keep_memory);
	      if (relstart == NULL)
		goto error_ret;

	      for (rel = relstart; rel < relstart + sec->reloc_count; ++rel)
		{
		  enum elf_ppc64_reloc_type r_type;
		  unsigned long r_symndx;
		  asection *sym_sec;
		  struct elf_link_hash_entry *h;
		  bfd_vma val;

		  r_type = ELF64_R_TYPE (rel->r_info);
		  switch (r_type)
		    {
		    default:
		      continue;

		    case R_PPC64_TOC16:
		    case R_PPC64_TOC16_LO:
		    case R_PPC64_TOC16_HI:
		    case R_PPC64_TOC16_HA:
		    case R_PPC64_TOC16_DS:
		    case R_PPC64_TOC16_LO_DS:
		    case R_PPC64_ADDR64:
		      break;
		    }

		  r_symndx = ELF64_R_SYM (rel->r_info);
		  if (!get_sym_h (&h, &sym, &sym_sec, NULL, &local_syms,
				  r_symndx, ibfd))
		    goto error_ret;

		  if (sym_sec != toc)
		    continue;

		  if (h != NULL)
		    val = h->root.u.def.value;
		  else
		    {
		      val = sym->st_value;
		      if (val != 0)
			local_toc_syms = true;
		    }

		  val += rel->r_addend;

		  if (val > toc->rawsize)
		    val = toc->rawsize;
		  else if ((skip[val >> 3] & ref_from_discarded) != 0)
		    continue;
		  else if ((skip[val >> 3] & can_optimize) != 0)
		    {
		      Elf_Internal_Rela *tocrel
			= toc_relocs + (skip[val >> 3] >> 2);
		      unsigned long tsym = ELF64_R_SYM (tocrel->r_info);

		      switch (r_type)
			{
			case R_PPC64_TOC16_HA:
			  rel->r_info = ELF64_R_INFO (tsym, R_PPC64_TOC16_HA);
			  break;

			case R_PPC64_TOC16_LO_DS:
			  rel->r_info = ELF64_R_INFO (tsym, R_PPC64_LO_DS_OPT);
			  break;

			default:
			  if (!ppc64_elf_howto_table[R_PPC64_ADDR32])
			    ppc_howto_init ();
			  info->callbacks->einfo
			    /* xgettext:c-format */
			    (_("%H: %s references "
			       "optimized away TOC entry\n"),
			     ibfd, sec, rel->r_offset,
			     ppc64_elf_howto_table[r_type]->name);
			  bfd_set_error (bfd_error_bad_value);
			  goto error_ret;
			}
		      rel->r_addend = tocrel->r_addend;
		      elf_section_data (sec)->relocs = relstart;
		      continue;
		    }

		  if (h != NULL || sym->st_value != 0)
		    continue;

		  rel->r_addend -= skip[val >> 3];
		  elf_section_data (sec)->relocs = relstart;
		}

	      if (elf_section_data (sec)->relocs != relstart)
		free (relstart);
	    }

	  /* We shouldn't have local or global symbols defined in the TOC,
	     but handle them anyway.  */
	  if (local_syms != NULL)
	    for (sym = local_syms;
		 sym < local_syms + symtab_hdr->sh_info;
		 ++sym)
	      if (sym->st_value != 0
		  && bfd_section_from_elf_index (ibfd, sym->st_shndx) == toc)
		{
		  unsigned long i;

		  if (sym->st_value > toc->rawsize)
		    i = toc->rawsize >> 3;
		  else
		    i = sym->st_value >> 3;

		  if ((skip[i] & (ref_from_discarded | can_optimize)) != 0)
		    {
		      if (local_toc_syms)
			_bfd_error_handler
			  (_("%s defined on removed toc entry"),
			   bfd_elf_sym_name (ibfd, symtab_hdr, sym, NULL));
		      do
			++i;
		      while ((skip[i] & (ref_from_discarded | can_optimize)));
		      sym->st_value = (bfd_vma) i << 3;
		    }

		  sym->st_value -= skip[i];
		  symtab_hdr->contents = (unsigned char *) local_syms;
		}

	  /* Adjust any global syms defined in this toc input section.  */
	  if (toc_inf.global_toc_syms)
	    {
	      toc_inf.toc = toc;
	      toc_inf.skip = skip;
	      toc_inf.global_toc_syms = false;
	      elf_link_hash_traverse (elf_hash_table (info), adjust_toc_syms,
				      &toc_inf);
	    }

	  if (toc->reloc_count != 0)
	    {
	      Elf_Internal_Shdr *rel_hdr;
	      Elf_Internal_Rela *wrel;
	      bfd_size_type sz;

	      /* Remove unused toc relocs, and adjust those we keep.  */
	      if (toc_relocs == NULL)
		toc_relocs = _bfd_elf_link_read_relocs (ibfd, toc, NULL, NULL,
							info->keep_memory);
	      if (toc_relocs == NULL)
		goto error_ret;

	      wrel = toc_relocs;
	      for (rel = toc_relocs; rel < toc_relocs + toc->reloc_count; ++rel)
		if ((skip[rel->r_offset >> 3]
		     & (ref_from_discarded | can_optimize)) == 0)
		  {
		    wrel->r_offset = rel->r_offset - skip[rel->r_offset >> 3];
		    wrel->r_info = rel->r_info;
		    wrel->r_addend = rel->r_addend;
		    ++wrel;
		  }
		else if (!dec_dynrel_count (rel, toc, info,
					    &local_syms, NULL, NULL))
		  goto error_ret;

	      elf_section_data (toc)->relocs = toc_relocs;
	      toc->reloc_count = wrel - toc_relocs;
	      rel_hdr = _bfd_elf_single_rel_hdr (toc);
	      sz = rel_hdr->sh_entsize;
	      rel_hdr->sh_size = toc->reloc_count * sz;
	    }
	}
      else if (elf_section_data (toc)->relocs != toc_relocs)
	free (toc_relocs);

      if (local_syms != NULL
	  && symtab_hdr->contents != (unsigned char *) local_syms)
	{
	  if (!info->keep_memory)
	    free (local_syms);
	  else
	    symtab_hdr->contents = (unsigned char *) local_syms;
	}
      free (skip);
    }

  /* Look for cases where we can change an indirect GOT access to
     a GOT relative or PC relative access, possibly reducing the
     number of GOT entries.  */
  for (ibfd = info->input_bfds; ibfd != NULL; ibfd = ibfd->link.next)
    {
      asection *sec;
      Elf_Internal_Shdr *symtab_hdr;
      Elf_Internal_Sym *local_syms;
      Elf_Internal_Rela *relstart, *rel;
      bfd_vma got;

      if (!is_ppc64_elf (ibfd))
	continue;

      if (!ppc64_elf_tdata (ibfd)->has_optrel)
	continue;

      sec = ppc64_elf_tdata (ibfd)->got;
      got = 0;
      if (sec != NULL)
	got = sec->output_section->vma + sec->output_offset + 0x8000;

      local_syms = NULL;
      symtab_hdr = &elf_symtab_hdr (ibfd);

      for (sec = ibfd->sections; sec != NULL; sec = sec->next)
	{
	  if (sec->reloc_count == 0
	      || !ppc64_elf_section_data (sec)->has_optrel
	      || discarded_section (sec))
	    continue;

	  relstart = _bfd_elf_link_read_relocs (ibfd, sec, NULL, NULL,
						info->keep_memory);
	  if (relstart == NULL)
	    {
	    got_error_ret:
	      if (symtab_hdr->contents != (unsigned char *) local_syms)
		free (local_syms);
	      if (sec != NULL
		  && elf_section_data (sec)->relocs != relstart)
		free (relstart);
	      return false;
	    }

	  for (rel = relstart; rel < relstart + sec->reloc_count; ++rel)
	    {
	      enum elf_ppc64_reloc_type r_type;
	      unsigned long r_symndx;
	      Elf_Internal_Sym *sym;
	      asection *sym_sec;
	      struct elf_link_hash_entry *h;
	      struct got_entry *ent;
	      bfd_vma val, pc;
	      unsigned char buf[8];
	      unsigned int insn;
	      enum {no_check, check_lo, check_ha} insn_check;

	      r_type = ELF64_R_TYPE (rel->r_info);
	      switch (r_type)
		{
		default:
		  insn_check = no_check;
		  break;

		case R_PPC64_PLT16_HA:
		case R_PPC64_GOT_TLSLD16_HA:
		case R_PPC64_GOT_TLSGD16_HA:
		case R_PPC64_GOT_TPREL16_HA:
		case R_PPC64_GOT_DTPREL16_HA:
		case R_PPC64_GOT16_HA:
		case R_PPC64_TOC16_HA:
		  insn_check = check_ha;
		  break;

		case R_PPC64_PLT16_LO:
		case R_PPC64_PLT16_LO_DS:
		case R_PPC64_GOT_TLSLD16_LO:
		case R_PPC64_GOT_TLSGD16_LO:
		case R_PPC64_GOT_TPREL16_LO_DS:
		case R_PPC64_GOT_DTPREL16_LO_DS:
		case R_PPC64_GOT16_LO:
		case R_PPC64_GOT16_LO_DS:
		case R_PPC64_TOC16_LO:
		case R_PPC64_TOC16_LO_DS:
		  insn_check = check_lo;
		  break;
		}

	      if (insn_check != no_check)
		{
		  bfd_vma off = rel->r_offset & ~3;

		  if (!bfd_get_section_contents (ibfd, sec, buf, off, 4))
		    goto got_error_ret;

		  insn = bfd_get_32 (ibfd, buf);
		  if (insn_check == check_lo
		      ? !ok_lo_toc_insn (insn, r_type)
		      : ((insn & ((0x3fu << 26) | 0x1f << 16))
			 != ((15u << 26) | (2 << 16)) /* addis rt,2,imm */))
		    {
		      char str[12];

		      ppc64_elf_tdata (ibfd)->unexpected_toc_insn = 1;
		      sprintf (str, "%#08x", insn);
		      info->callbacks->einfo
			/* xgettext:c-format */
			(_("%H: got/toc optimization is not supported for"
			   " %s instruction\n"),
			 ibfd, sec, rel->r_offset & ~3, str);
		      continue;
		    }
		}

	      switch (r_type)
		{
		/* Note that we don't delete GOT entries for
		   R_PPC64_GOT16_DS since we'd need a lot more
		   analysis.  For starters, the preliminary layout is
		   before the GOT, PLT, dynamic sections and stubs are
		   laid out.  Then we'd need to allow for changes in
		   distance between sections caused by alignment.  */
		default:
		  continue;

		case R_PPC64_GOT16_HA:
		case R_PPC64_GOT16_LO_DS:
		case R_PPC64_GOT_PCREL34:
		  break;
		}

	      r_symndx = ELF64_R_SYM (rel->r_info);
	      if (!get_sym_h (&h, &sym, &sym_sec, NULL, &local_syms,
			      r_symndx, ibfd))
		goto got_error_ret;

	      if (sym_sec == NULL
		  || sym_sec->output_section == NULL
		  || discarded_section (sym_sec))
		continue;

	      if ((h ? h->type : ELF_ST_TYPE (sym->st_info)) == STT_GNU_IFUNC)
		continue;

	      if (!SYMBOL_REFERENCES_LOCAL (info, h)
		  || (bfd_link_pic (info)
		      && sym_sec == bfd_abs_section_ptr))
		continue;

	      if (h != NULL)
		val = h->root.u.def.value;
	      else
		val = sym->st_value;
	      val += rel->r_addend;
	      val += sym_sec->output_section->vma + sym_sec->output_offset;

/* Fudge factor to allow for the fact that the preliminary layout
   isn't exact.  Reduce limits by this factor.  */
#define LIMIT_ADJUST(LIMIT) ((LIMIT) - (LIMIT) / 16)

	      switch (r_type)
		{
		default:
		  continue;

		case R_PPC64_GOT16_HA:
		  if (val - got + LIMIT_ADJUST (0x80008000ULL)
		      >= LIMIT_ADJUST (0x100000000ULL))
		    continue;

		  if (!bfd_get_section_contents (ibfd, sec, buf,
						 rel->r_offset & ~3, 4))
		    goto got_error_ret;
		  insn = bfd_get_32 (ibfd, buf);
		  if (((insn & ((0x3fu << 26) | 0x1f << 16))
		       != ((15u << 26) | (2 << 16)) /* addis rt,2,imm */))
		    continue;
		  break;

		case R_PPC64_GOT16_LO_DS:
		  if (val - got + LIMIT_ADJUST (0x80008000ULL)
		      >= LIMIT_ADJUST (0x100000000ULL))
		    continue;
		  if (!bfd_get_section_contents (ibfd, sec, buf,
						 rel->r_offset & ~3, 4))
		    goto got_error_ret;
		  insn = bfd_get_32 (ibfd, buf);
		  if ((insn & (0x3fu << 26 | 0x3)) != 58u << 26 /* ld */)
		    continue;
		  break;

		case R_PPC64_GOT_PCREL34:
		  pc = rel->r_offset;
		  pc += sec->output_section->vma + sec->output_offset;
		  if (val - pc + LIMIT_ADJUST (1ULL << 33)
		      >= LIMIT_ADJUST (1ULL << 34))
		    continue;
		  if (!bfd_get_section_contents (ibfd, sec, buf,
						 rel->r_offset & ~3, 8))
		    goto got_error_ret;
		  insn = bfd_get_32 (ibfd, buf);
		  if ((insn & (-1u << 18)) != ((1u << 26) | (1u << 20)))
		    continue;
		  insn = bfd_get_32 (ibfd, buf + 4);
		  if ((insn & (0x3fu << 26)) != 57u << 26)
		    continue;
		  break;
		}
#undef LIMIT_ADJUST

	      if (h != NULL)
		ent = h->got.glist;
	      else
		{
		  struct got_entry **local_got_ents = elf_local_got_ents (ibfd);
		  ent = local_got_ents[r_symndx];
		}
	      for (; ent != NULL; ent = ent->next)
		if (ent->addend == rel->r_addend
		    && ent->owner == ibfd
		    && ent->tls_type == 0)
		  break;
	      BFD_ASSERT (ent && ent->got.refcount > 0);
	      ent->got.refcount -= 1;
	    }

	  if (elf_section_data (sec)->relocs != relstart)
	    free (relstart);
	}

      if (local_syms != NULL
	  && symtab_hdr->contents != (unsigned char *) local_syms)
	{
	  if (!info->keep_memory)
	    free (local_syms);
	  else
	    symtab_hdr->contents = (unsigned char *) local_syms;
	}
    }

  return true;
}

/* Return true iff input section I references the TOC using
   instructions limited to +/-32k offsets.  */

bool
ppc64_elf_has_small_toc_reloc (asection *i)
{
  return (is_ppc64_elf (i->owner)
	  && ppc64_elf_tdata (i->owner)->has_small_toc_reloc);
}

/* Allocate space for one GOT entry.  */

static void
allocate_got (struct elf_link_hash_entry *h,
	      struct bfd_link_info *info,
	      struct got_entry *gent)
{
  struct ppc_link_hash_table *htab = ppc_hash_table (info);
  struct ppc_link_hash_entry *eh = ppc_elf_hash_entry (h);
  int entsize = (gent->tls_type & eh->tls_mask & (TLS_GD | TLS_LD)
		 ? 16 : 8);
  int rentsize = (gent->tls_type & eh->tls_mask & TLS_GD
		  ? 2 : 1) * sizeof (Elf64_External_Rela);
  asection *got = ppc64_elf_tdata (gent->owner)->got;

  gent->got.offset = got->size;
  got->size += entsize;

  if (h->type == STT_GNU_IFUNC)
    {
      htab->elf.irelplt->size += rentsize;
      htab->got_reli_size += rentsize;
    }
  else if (((bfd_link_pic (info)
	     && (gent->tls_type == 0
		 ? !info->enable_dt_relr
		 : !(bfd_link_executable (info)
		     && SYMBOL_REFERENCES_LOCAL (info, h)))
	     && !bfd_is_abs_symbol (&h->root))
	    || (htab->elf.dynamic_sections_created
		&& h->dynindx != -1
		&& !SYMBOL_REFERENCES_LOCAL (info, h)))
	   && !UNDEFWEAK_NO_DYNAMIC_RELOC (info, h))
    {
      asection *relgot = ppc64_elf_tdata (gent->owner)->relgot;
      relgot->size += rentsize;
    }
}

/* This function merges got entries in the same toc group.  */

static void
merge_got_entries (struct got_entry **pent)
{
  struct got_entry *ent, *ent2;

  for (ent = *pent; ent != NULL; ent = ent->next)
    if (!ent->is_indirect)
      for (ent2 = ent->next; ent2 != NULL; ent2 = ent2->next)
	if (!ent2->is_indirect
	    && ent2->addend == ent->addend
	    && ent2->tls_type == ent->tls_type
	    && elf_gp (ent2->owner) == elf_gp (ent->owner))
	  {
	    ent2->is_indirect = true;
	    ent2->got.ent = ent;
	  }
}

/* If H is undefined, make it dynamic if that makes sense.  */

static bool
ensure_undef_dynamic (struct bfd_link_info *info,
		      struct elf_link_hash_entry *h)
{
  struct elf_link_hash_table *htab = elf_hash_table (info);

  if (htab->dynamic_sections_created
      && ((info->dynamic_undefined_weak != 0
	   && h->root.type == bfd_link_hash_undefweak)
	  || h->root.type == bfd_link_hash_undefined)
      && h->dynindx == -1
      && !h->forced_local
      && ELF_ST_VISIBILITY (h->other) == STV_DEFAULT)
    return bfd_elf_link_record_dynamic_symbol (info, h);
  return true;
}

/* Choose whether to use htab->iplt or htab->pltlocal rather than the
   usual htab->elf.splt section for a PLT entry.  */

static inline
bool use_local_plt (struct bfd_link_info *info,
			   struct elf_link_hash_entry *h)
{
  return (h == NULL
	  || h->dynindx == -1
	  || !elf_hash_table (info)->dynamic_sections_created);
}

/* Allocate space in .plt, .got and associated reloc sections for
   dynamic relocs.  */

static bool
allocate_dynrelocs (struct elf_link_hash_entry *h, void *inf)
{
  struct bfd_link_info *info;
  struct ppc_link_hash_table *htab;
  asection *s;
  struct ppc_link_hash_entry *eh;
  struct got_entry **pgent, *gent;

  if (h->root.type == bfd_link_hash_indirect)
    return true;

  info = (struct bfd_link_info *) inf;
  htab = ppc_hash_table (info);
  if (htab == NULL)
    return false;

  eh = ppc_elf_hash_entry (h);
  /* Run through the TLS GD got entries first if we're changing them
     to TPREL.  */
  if ((eh->tls_mask & (TLS_TLS | TLS_GDIE)) == (TLS_TLS | TLS_GDIE))
    for (gent = h->got.glist; gent != NULL; gent = gent->next)
      if (gent->got.refcount > 0
	  && (gent->tls_type & TLS_GD) != 0)
	{
	  /* This was a GD entry that has been converted to TPREL.  If
	     there happens to be a TPREL entry we can use that one.  */
	  struct got_entry *ent;
	  for (ent = h->got.glist; ent != NULL; ent = ent->next)
	    if (ent->got.refcount > 0
		&& (ent->tls_type & TLS_TPREL) != 0
		&& ent->addend == gent->addend
		&& ent->owner == gent->owner)
	      {
		gent->got.refcount = 0;
		break;
	      }

	  /* If not, then we'll be using our own TPREL entry.  */
	  if (gent->got.refcount != 0)
	    gent->tls_type = TLS_TLS | TLS_TPREL;
	}

  /* Remove any list entry that won't generate a word in the GOT before
     we call merge_got_entries.  Otherwise we risk merging to empty
     entries.  */
  pgent = &h->got.glist;
  while ((gent = *pgent) != NULL)
    if (gent->got.refcount > 0)
      {
	if ((gent->tls_type & TLS_LD) != 0
	    && SYMBOL_REFERENCES_LOCAL (info, h))
	  {
	    ppc64_tlsld_got (gent->owner)->got.refcount += 1;
	    *pgent = gent->next;
	  }
	else
	  pgent = &gent->next;
      }
    else
      *pgent = gent->next;

  if (!htab->do_multi_toc)
    merge_got_entries (&h->got.glist);

  for (gent = h->got.glist; gent != NULL; gent = gent->next)
    if (!gent->is_indirect)
      {
	/* Ensure we catch all the cases where this symbol should
	   be made dynamic.  */
	if (!ensure_undef_dynamic (info, h))
	  return false;

	if (!is_ppc64_elf (gent->owner))
	  abort ();

	allocate_got (h, info, gent);
      }

  /* If no dynamic sections we can't have dynamic relocs, except for
     IFUNCs which are handled even in static executables.  */
  if (!htab->elf.dynamic_sections_created
      && h->type != STT_GNU_IFUNC)
    h->dyn_relocs = NULL;

  /* Discard relocs on undefined symbols that must be local.  */
  else if (h->root.type == bfd_link_hash_undefined
	   && ELF_ST_VISIBILITY (h->other) != STV_DEFAULT)
    h->dyn_relocs = NULL;

  /* Also discard relocs on undefined weak syms with non-default
     visibility, or when dynamic_undefined_weak says so.  */
  else if (UNDEFWEAK_NO_DYNAMIC_RELOC (info, h))
    h->dyn_relocs = NULL;

  if (h->dyn_relocs != NULL)
    {
      struct ppc_dyn_relocs *p, **pp;

      /* In the shared -Bsymbolic case, discard space allocated for
	 dynamic pc-relative relocs against symbols which turn out to
	 be defined in regular objects.  For the normal shared case,
	 discard space for relocs that have become local due to symbol
	 visibility changes.  */
      if (bfd_link_pic (info))
	{
	  /* Relocs that use pc_count are those that appear on a call
	     insn, or certain REL relocs (see must_be_dyn_reloc) that
	     can be generated via assembly.  We want calls to
	     protected symbols to resolve directly to the function
	     rather than going via the plt.  If people want function
	     pointer comparisons to work as expected then they should
	     avoid writing weird assembly.  */
	  if (SYMBOL_CALLS_LOCAL (info, h))
	    {
	      for (pp = (struct ppc_dyn_relocs **) &h->dyn_relocs;
		   (p = *pp) != NULL;
		   )
		{
		  p->count -= p->pc_count;
		  p->pc_count = 0;
		  if (p->count == 0)
		    *pp = p->next;
		  else
		    pp = &p->next;
		}
	    }

	  if (h->dyn_relocs != NULL)
	    {
	      /* Ensure we catch all the cases where this symbol
		 should be made dynamic.  */
	      if (!ensure_undef_dynamic (info, h))
		return false;
	    }
	}

      /* For a fixed position executable, discard space for
	 relocs against symbols which are not dynamic.  */
      else if (h->type != STT_GNU_IFUNC)
	{
	  if ((h->dynamic_adjusted
	       || (h->ref_regular
		   && h->root.type == bfd_link_hash_undefweak
		   && (info->dynamic_undefined_weak > 0
		       || !_bfd_elf_readonly_dynrelocs (h))))
	      && !h->def_regular
	      && !ELF_COMMON_DEF_P (h))
	    {
	      /* Ensure we catch all the cases where this symbol
		 should be made dynamic.  */
	      if (!ensure_undef_dynamic (info, h))
		return false;

	      /* But if that didn't work out, discard dynamic relocs.  */
	      if (h->dynindx == -1)
		h->dyn_relocs = NULL;
	    }
	  else
	    h->dyn_relocs = NULL;
	}

      /* Finally, allocate space.  */
      for (p = (struct ppc_dyn_relocs *) h->dyn_relocs; p != NULL; p = p->next)
	if (!discarded_section (p->sec))
	  {
	    unsigned int count;
	    asection *sreloc = elf_section_data (p->sec)->sreloc;
	    if (eh->elf.type == STT_GNU_IFUNC)
	      sreloc = htab->elf.irelplt;
	    count = p->count;
	    if (info->enable_dt_relr
		&& ((!NO_OPD_RELOCS
		     && ppc64_elf_section_data (p->sec)->sec_type == sec_opd)
		    || (eh->elf.type != STT_GNU_IFUNC
			&& SYMBOL_REFERENCES_LOCAL (info, h))))
	      count -= p->rel_count;
	    sreloc->size += count * sizeof (Elf64_External_Rela);
	  }
    }

  /* We might need a PLT entry when the symbol
     a) is dynamic, or
     b) is an ifunc, or
     c) has plt16 relocs and has been processed by adjust_dynamic_symbol, or
     d) has plt16 relocs and we are linking statically.  */
  if ((htab->elf.dynamic_sections_created && h->dynindx != -1)
      || h->type == STT_GNU_IFUNC
      || (h->needs_plt && h->dynamic_adjusted)
      || (h->needs_plt
	  && h->def_regular
	  && !htab->elf.dynamic_sections_created
	  && !htab->can_convert_all_inline_plt
	  && (ppc_elf_hash_entry (h)->tls_mask
	      & (TLS_TLS | PLT_KEEP)) == PLT_KEEP))
    {
      struct plt_entry *pent;
      bool doneone = false;
      for (pent = h->plt.plist; pent != NULL; pent = pent->next)
	if (pent->plt.refcount > 0)
	  {
	    if (!ensure_undef_dynamic (info, h))
	      return false;

	    if (use_local_plt (info, h))
	      {
		if (h->type == STT_GNU_IFUNC)
		  {
		    s = htab->elf.iplt;
		    pent->plt.offset = s->size;
		    s->size += PLT_ENTRY_SIZE (htab);
		    s = htab->elf.irelplt;
		  }
		else
		  {
		    s = htab->pltlocal;
		    pent->plt.offset = s->size;
		    s->size += LOCAL_PLT_ENTRY_SIZE (htab);
		    s = NULL;
		    if (bfd_link_pic (info)
			&& !(info->enable_dt_relr && !htab->opd_abi))
		      s = htab->relpltlocal;
		  }
	      }
	    else
	      {
		/* If this is the first .plt entry, make room for the special
		   first entry.  */
		s = htab->elf.splt;
		if (s->size == 0)
		  s->size += PLT_INITIAL_ENTRY_SIZE (htab);

		pent->plt.offset = s->size;

		/* Make room for this entry.  */
		s->size += PLT_ENTRY_SIZE (htab);

		/* Make room for the .glink code.  */
		s = htab->glink;
		if (s->size == 0)
		  s->size += GLINK_PLTRESOLVE_SIZE (htab);
		if (htab->opd_abi)
		  {
		    /* We need bigger stubs past index 32767.  */
		    if (s->size >= GLINK_PLTRESOLVE_SIZE (htab) + 32768*2*4)
		      s->size += 4;
		    s->size += 2*4;
		  }
		else
		  s->size += 4;

		/* We also need to make an entry in the .rela.plt section.  */
		s = htab->elf.srelplt;
	      }
	    if (s != NULL)
	      s->size += sizeof (Elf64_External_Rela);
	    doneone = true;
	  }
	else
	  pent->plt.offset = (bfd_vma) -1;
      if (!doneone)
	{
	  h->plt.plist = NULL;
	  h->needs_plt = 0;
	}
    }
  else
    {
      h->plt.plist = NULL;
      h->needs_plt = 0;
    }

  return true;
}

#define PPC_LO(v) ((v) & 0xffff)
#define PPC_HI(v) (((v) >> 16) & 0xffff)
#define PPC_HA(v) PPC_HI ((v) + 0x8000)
#define D34(v) \
  ((((v) & 0x3ffff0000ULL) << 16) | (v & 0xffff))
#define HA34(v) ((v + (1ULL << 33)) >> 34)

/* Called via elf_link_hash_traverse from ppc64_elf_size_dynamic_sections
   to set up space for global entry stubs.  These are put in glink,
   after the branch table.  */

static bool
size_global_entry_stubs (struct elf_link_hash_entry *h, void *inf)
{
  struct bfd_link_info *info;
  struct ppc_link_hash_table *htab;
  struct plt_entry *pent;
  asection *s, *plt;

  if (h->root.type == bfd_link_hash_indirect)
    return true;

  if (!h->pointer_equality_needed)
    return true;

  if (h->def_regular)
    return true;

  info = inf;
  htab = ppc_hash_table (info);
  if (htab == NULL)
    return false;

  s = htab->global_entry;
  plt = htab->elf.splt;
  for (pent = h->plt.plist; pent != NULL; pent = pent->next)
    if (pent->plt.offset != (bfd_vma) -1
	&& pent->addend == 0)
      {
	/* For ELFv2, if this symbol is not defined in a regular file
	   and we are not generating a shared library or pie, then we
	   need to define the symbol in the executable on a call stub.
	   This is to avoid text relocations.  */
	bfd_vma off, stub_align, stub_off, stub_size;
	unsigned int align_power;

	stub_size = 16;
	stub_off = s->size;
	if (htab->params->plt_stub_align >= 0)
	  align_power = htab->params->plt_stub_align;
	else
	  align_power = -htab->params->plt_stub_align;
	/* Setting section alignment is delayed until we know it is
	   non-empty.  Otherwise the .text output section will be
	   aligned at least to plt_stub_align even when no global
	   entry stubs are needed.  */
	if (s->alignment_power < align_power)
	  s->alignment_power = align_power;
	stub_align = (bfd_vma) 1 << align_power;
	if (htab->params->plt_stub_align >= 0
	    || ((((stub_off + stub_size - 1) & -stub_align)
		 - (stub_off & -stub_align))
		> ((stub_size - 1) & -stub_align)))
	  stub_off = (stub_off + stub_align - 1) & -stub_align;
	off = pent->plt.offset + plt->output_offset + plt->output_section->vma;
	off -= stub_off + s->output_offset + s->output_section->vma;
	/* Note that for --plt-stub-align negative we have a possible
	   dependency between stub offset and size.  Break that
	   dependency by assuming the max stub size when calculating
	   the stub offset.  */
	if (PPC_HA (off) == 0)
	  stub_size -= 4;
	h->root.type = bfd_link_hash_defined;
	h->root.u.def.section = s;
	h->root.u.def.value = stub_off;
	s->size = stub_off + stub_size;
	break;
      }
  return true;
}

/* Set the sizes of the dynamic sections.  */

static bool
ppc64_elf_size_dynamic_sections (bfd *output_bfd,
				 struct bfd_link_info *info)
{
  struct ppc_link_hash_table *htab;
  bfd *dynobj;
  asection *s;
  bool relocs;
  bfd *ibfd;
  struct got_entry *first_tlsld;

  htab = ppc_hash_table (info);
  if (htab == NULL)
    return false;

  dynobj = htab->elf.dynobj;
  if (dynobj == NULL)
    abort ();

  if (htab->elf.dynamic_sections_created)
    {
      /* Set the contents of the .interp section to the interpreter.  */
      if (bfd_link_executable (info) && !info->nointerp)
	{
	  s = bfd_get_linker_section (dynobj, ".interp");
	  if (s == NULL)
	    abort ();
	  s->size = sizeof ELF_DYNAMIC_INTERPRETER;
	  s->contents = (unsigned char *) ELF_DYNAMIC_INTERPRETER;
	}
    }

  /* Set up .got offsets for local syms, and space for local dynamic
     relocs.  */
  for (ibfd = info->input_bfds; ibfd != NULL; ibfd = ibfd->link.next)
    {
      struct got_entry **lgot_ents;
      struct got_entry **end_lgot_ents;
      struct plt_entry **local_plt;
      struct plt_entry **end_local_plt;
      unsigned char *lgot_masks;
      bfd_size_type locsymcount;
      Elf_Internal_Shdr *symtab_hdr;
      Elf_Internal_Sym *local_syms;
      Elf_Internal_Sym *isym;

      if (!is_ppc64_elf (ibfd))
	continue;

      for (s = ibfd->sections; s != NULL; s = s->next)
	{
	  struct ppc_local_dyn_relocs *p;

	  for (p = elf_section_data (s)->local_dynrel; p != NULL; p = p->next)
	    {
	      if (discarded_section (p->sec))
		{
		  /* Input section has been discarded, either because
		     it is a copy of a linkonce section or due to
		     linker script /DISCARD/, so we'll be discarding
		     the relocs too.  */
		}
	      else if (p->count != 0)
		{
		  unsigned int count;
		  asection *srel;

		  count = p->count;
		  if (info->enable_dt_relr
		      && ((!NO_OPD_RELOCS
			   && (ppc64_elf_section_data (p->sec)->sec_type
			       == sec_opd))
			  || !p->ifunc))
		    count -= p->rel_count;
		  srel = elf_section_data (p->sec)->sreloc;
		  if (p->ifunc)
		    srel = htab->elf.irelplt;
		  srel->size += count * sizeof (Elf64_External_Rela);
		  if ((p->sec->output_section->flags & SEC_READONLY) != 0)
		    info->flags |= DF_TEXTREL;
		}
	    }
	}

      lgot_ents = elf_local_got_ents (ibfd);
      if (!lgot_ents)
	continue;

      symtab_hdr = &elf_symtab_hdr (ibfd);
      locsymcount = symtab_hdr->sh_info;
      end_lgot_ents = lgot_ents + locsymcount;
      local_plt = (struct plt_entry **) end_lgot_ents;
      end_local_plt = local_plt + locsymcount;
      lgot_masks = (unsigned char *) end_local_plt;
      local_syms = (Elf_Internal_Sym *) symtab_hdr->contents;
      if (local_syms == NULL && locsymcount != 0)
	{
	  local_syms = bfd_elf_get_elf_syms (ibfd, symtab_hdr, locsymcount,
					     0, NULL, NULL, NULL);
	  if (local_syms == NULL)
	    return false;
	}
      s = ppc64_elf_tdata (ibfd)->got;
      for (isym = local_syms;
	   lgot_ents < end_lgot_ents;
	   ++lgot_ents, ++lgot_masks, isym++)
	{
	  struct got_entry **pent, *ent;

	  pent = lgot_ents;
	  while ((ent = *pent) != NULL)
	    if (ent->got.refcount > 0)
	      {
		if ((ent->tls_type & *lgot_masks & TLS_LD) != 0)
		  {
		    ppc64_tlsld_got (ibfd)->got.refcount += 1;
		    *pent = ent->next;
		  }
		else
		  {
		    unsigned int ent_size = 8;
		    unsigned int rel_size = sizeof (Elf64_External_Rela);

		    ent->got.offset = s->size;
		    if ((ent->tls_type & *lgot_masks & TLS_GD) != 0)
		      {
			ent_size *= 2;
			rel_size *= 2;
		      }
		    s->size += ent_size;
		    if ((*lgot_masks & (TLS_TLS | PLT_IFUNC)) == PLT_IFUNC)
		      {
			htab->elf.irelplt->size += rel_size;
			htab->got_reli_size += rel_size;
		      }
		    else if (bfd_link_pic (info)
			     && (ent->tls_type == 0
				 ? !info->enable_dt_relr
				 : !bfd_link_executable (info))
			     && isym->st_shndx != SHN_ABS)
		      {
			asection *srel = ppc64_elf_tdata (ibfd)->relgot;
			srel->size += rel_size;
		      }
		    pent = &ent->next;
		  }
	      }
	    else
	      *pent = ent->next;
	}
      if (local_syms != NULL
	  && symtab_hdr->contents != (unsigned char *) local_syms)
	{
	  if (!info->keep_memory)
	    free (local_syms);
	  else
	    symtab_hdr->contents = (unsigned char *) local_syms;
	}

      /* Allocate space for plt calls to local syms.  */
      lgot_masks = (unsigned char *) end_local_plt;
      for (; local_plt < end_local_plt; ++local_plt, ++lgot_masks)
	{
	  struct plt_entry *ent;

	  for (ent = *local_plt; ent != NULL; ent = ent->next)
	    if (ent->plt.refcount > 0)
	      {
		if ((*lgot_masks & (TLS_TLS | PLT_IFUNC)) == PLT_IFUNC)
		  {
		    s = htab->elf.iplt;
		    ent->plt.offset = s->size;
		    s->size += PLT_ENTRY_SIZE (htab);
		    htab->elf.irelplt->size += sizeof (Elf64_External_Rela);
		  }
		else if (htab->can_convert_all_inline_plt
			 || (*lgot_masks & (TLS_TLS | PLT_KEEP)) != PLT_KEEP)
		  ent->plt.offset = (bfd_vma) -1;
		else
		  {
		    s = htab->pltlocal;
		    ent->plt.offset = s->size;
		    s->size += LOCAL_PLT_ENTRY_SIZE (htab);
		    if (bfd_link_pic (info)
			&& !(info->enable_dt_relr && !htab->opd_abi))
		      htab->relpltlocal->size += sizeof (Elf64_External_Rela);
		  }
	      }
	    else
	      ent->plt.offset = (bfd_vma) -1;
	}
    }

  /* Allocate global sym .plt and .got entries, and space for global
     sym dynamic relocs.  */
  elf_link_hash_traverse (&htab->elf, allocate_dynrelocs, info);

  if (!htab->opd_abi && !bfd_link_pic (info))
    elf_link_hash_traverse (&htab->elf, size_global_entry_stubs, info);

  first_tlsld = NULL;
  for (ibfd = info->input_bfds; ibfd != NULL; ibfd = ibfd->link.next)
    {
      struct got_entry *ent;

      if (!is_ppc64_elf (ibfd))
	continue;

      ent = ppc64_tlsld_got (ibfd);
      if (ent->got.refcount > 0)
	{
	  if (!htab->do_multi_toc && first_tlsld != NULL)
	    {
	      ent->is_indirect = true;
	      ent->got.ent = first_tlsld;
	    }
	  else
	    {
	      if (first_tlsld == NULL)
		first_tlsld = ent;
	      s = ppc64_elf_tdata (ibfd)->got;
	      ent->got.offset = s->size;
	      ent->owner = ibfd;
	      s->size += 16;
	      if (bfd_link_dll (info))
		{
		  asection *srel = ppc64_elf_tdata (ibfd)->relgot;
		  srel->size += sizeof (Elf64_External_Rela);
		}
	    }
	}
      else
	ent->got.offset = (bfd_vma) -1;
    }

  /* We now have determined the sizes of the various dynamic sections.
     Allocate memory for them.  */
  relocs = false;
  for (s = dynobj->sections; s != NULL; s = s->next)
    {
      if ((s->flags & SEC_LINKER_CREATED) == 0)
	continue;

      if (s == htab->brlt || s == htab->relbrlt || s == htab->elf.srelrdyn)
	/* These haven't been allocated yet;  don't strip.  */
	continue;
      else if (s == htab->elf.sgot
	       || s == htab->elf.splt
	       || s == htab->elf.iplt
	       || s == htab->pltlocal
	       || s == htab->glink
	       || s == htab->global_entry
	       || s == htab->elf.sdynbss
	       || s == htab->elf.sdynrelro)
	{
	  /* Strip this section if we don't need it; see the
	     comment below.  */
	}
      else if (s == htab->glink_eh_frame)
	{
	  if (!bfd_is_abs_section (s->output_section))
	    /* Not sized yet.  */
	    continue;
	}
      else if (startswith (s->name, ".rela"))
	{
	  if (s->size != 0)
	    {
	      if (s != htab->elf.srelplt)
		relocs = true;

	      /* We use the reloc_count field as a counter if we need
		 to copy relocs into the output file.  */
	      s->reloc_count = 0;
	    }
	}
      else
	{
	  /* It's not one of our sections, so don't allocate space.  */
	  continue;
	}

      if (s->size == 0)
	{
	  /* If we don't need this section, strip it from the
	     output file.  This is mostly to handle .rela.bss and
	     .rela.plt.  We must create both sections in
	     create_dynamic_sections, because they must be created
	     before the linker maps input sections to output
	     sections.  The linker does that before
	     adjust_dynamic_symbol is called, and it is that
	     function which decides whether anything needs to go
	     into these sections.  */
	  s->flags |= SEC_EXCLUDE;
	  continue;
	}

      if (bfd_is_abs_section (s->output_section))
	_bfd_error_handler (_("warning: discarding dynamic section %s"),
			    s->name);

      if ((s->flags & SEC_HAS_CONTENTS) == 0)
	continue;

      /* Allocate memory for the section contents.  We use bfd_zalloc
	 here in case unused entries are not reclaimed before the
	 section's contents are written out.  This should not happen,
	 but this way if it does we get a R_PPC64_NONE reloc in .rela
	 sections instead of garbage.
	 We also rely on the section contents being zero when writing
	 the GOT and .dynrelro.  */
      s->contents = bfd_zalloc (dynobj, s->size);
      if (s->contents == NULL)
	return false;
    }

  for (ibfd = info->input_bfds; ibfd != NULL; ibfd = ibfd->link.next)
    {
      if (!is_ppc64_elf (ibfd))
	continue;

      s = ppc64_elf_tdata (ibfd)->got;
      if (s != NULL && s != htab->elf.sgot)
	{
	  if (s->size == 0)
	    s->flags |= SEC_EXCLUDE;
	  else
	    {
	      s->contents = bfd_zalloc (ibfd, s->size);
	      if (s->contents == NULL)
		return false;
	    }
	}
      s = ppc64_elf_tdata (ibfd)->relgot;
      if (s != NULL)
	{
	  if (s->size == 0)
	    s->flags |= SEC_EXCLUDE;
	  else
	    {
	      s->contents = bfd_zalloc (ibfd, s->size);
	      if (s->contents == NULL)
		return false;
	      relocs = true;
	      s->reloc_count = 0;
	    }
	}
    }

  if (htab->elf.dynamic_sections_created)
    {
      bool tls_opt;

      /* Add some entries to the .dynamic section.  We fill in the
	 values later, in ppc64_elf_finish_dynamic_sections, but we
	 must add the entries now so that we get the correct size for
	 the .dynamic section.  The DT_DEBUG entry is filled in by the
	 dynamic linker and used by the debugger.  */
#define add_dynamic_entry(TAG, VAL) \
  _bfd_elf_add_dynamic_entry (info, TAG, VAL)

      if (bfd_link_executable (info))
	{
	  if (!add_dynamic_entry (DT_DEBUG, 0))
	    return false;
	}

      if (htab->elf.splt != NULL && htab->elf.splt->size != 0)
	{
	  if (!add_dynamic_entry (DT_PLTGOT, 0)
	      || !add_dynamic_entry (DT_PLTRELSZ, 0)
	      || !add_dynamic_entry (DT_PLTREL, DT_RELA)
	      || !add_dynamic_entry (DT_JMPREL, 0)
	      || !add_dynamic_entry (DT_PPC64_GLINK, 0))
	    return false;
	}

      if (NO_OPD_RELOCS && abiversion (output_bfd) <= 1)
	{
	  if (!add_dynamic_entry (DT_PPC64_OPD, 0)
	      || !add_dynamic_entry (DT_PPC64_OPDSZ, 0))
	    return false;
	}

      tls_opt = (htab->params->tls_get_addr_opt
		 && ((htab->tls_get_addr_fd != NULL
		      && htab->tls_get_addr_fd->elf.plt.plist != NULL)
		     || (htab->tga_desc_fd != NULL
			 && htab->tga_desc_fd->elf.plt.plist != NULL)));
      if (tls_opt || !htab->opd_abi)
	{
	  if (!add_dynamic_entry (DT_PPC64_OPT, tls_opt ? PPC64_OPT_TLS : 0))
	    return false;
	}

      if (relocs)
	{
	  if (!add_dynamic_entry (DT_RELA, 0)
	      || !add_dynamic_entry (DT_RELASZ, 0)
	      || !add_dynamic_entry (DT_RELAENT, sizeof (Elf64_External_Rela)))
	    return false;

	  /* If any dynamic relocs apply to a read-only section,
	     then we need a DT_TEXTREL entry.  */
	  if ((info->flags & DF_TEXTREL) == 0)
	    elf_link_hash_traverse (&htab->elf,
				    _bfd_elf_maybe_set_textrel, info);

	  if ((info->flags & DF_TEXTREL) != 0)
	    {
	      if (!add_dynamic_entry (DT_TEXTREL, 0))
		return false;
	    }
	}
    }
#undef add_dynamic_entry

  return true;
}

/* Return TRUE if symbol should be hashed in the `.gnu.hash' section.  */

static bool
ppc64_elf_hash_symbol (struct elf_link_hash_entry *h)
{
  if (h->plt.plist != NULL
      && !h->def_regular
      && !h->pointer_equality_needed)
    return false;

  return _bfd_elf_hash_symbol (h);
}

/* Determine the type of stub needed, if any, for a call.  */

static inline enum ppc_stub_main_type
ppc_type_of_stub (asection *input_sec,
		  const Elf_Internal_Rela *rel,
		  struct ppc_link_hash_entry **hash,
		  struct plt_entry **plt_ent,
		  bfd_vma destination,
		  unsigned long local_off)
{
  struct ppc_link_hash_entry *h = *hash;
  bfd_vma location;
  bfd_vma branch_offset;
  bfd_vma max_branch_offset;
  enum elf_ppc64_reloc_type r_type;

  if (h != NULL)
    {
      struct plt_entry *ent;
      struct ppc_link_hash_entry *fdh = h;
      if (h->oh != NULL
	  && h->oh->is_func_descriptor)
	{
	  fdh = ppc_follow_link (h->oh);
	  *hash = fdh;
	}

      for (ent = fdh->elf.plt.plist; ent != NULL; ent = ent->next)
	if (ent->addend == rel->r_addend
	    && ent->plt.offset != (bfd_vma) -1)
	  {
	    *plt_ent = ent;
	    return ppc_stub_plt_call;
	  }

      /* Here, we know we don't have a plt entry.  If we don't have a
	 either a defined function descriptor or a defined entry symbol
	 in a regular object file, then it is pointless trying to make
	 any other type of stub.  */
      if (!is_static_defined (&fdh->elf)
	  && !is_static_defined (&h->elf))
	return ppc_stub_none;
    }
  else if (elf_local_got_ents (input_sec->owner) != NULL)
    {
      Elf_Internal_Shdr *symtab_hdr = &elf_symtab_hdr (input_sec->owner);
      struct plt_entry **local_plt = (struct plt_entry **)
	elf_local_got_ents (input_sec->owner) + symtab_hdr->sh_info;
      unsigned long r_symndx = ELF64_R_SYM (rel->r_info);

      if (local_plt[r_symndx] != NULL)
	{
	  struct plt_entry *ent;

	  for (ent = local_plt[r_symndx]; ent != NULL; ent = ent->next)
	    if (ent->addend == rel->r_addend
		&& ent->plt.offset != (bfd_vma) -1)
	      {
		*plt_ent = ent;
		return ppc_stub_plt_call;
	      }
	}
    }

  /* Determine where the call point is.  */
  location = (input_sec->output_offset
	      + input_sec->output_section->vma
	      + rel->r_offset);

  branch_offset = destination - location;
  r_type = ELF64_R_TYPE (rel->r_info);

  /* Determine if a long branch stub is needed.  */
  max_branch_offset = 1 << 25;
  if (r_type == R_PPC64_REL14
      || r_type == R_PPC64_REL14_BRTAKEN
      || r_type == R_PPC64_REL14_BRNTAKEN)
    max_branch_offset = 1 << 15;

  if (branch_offset + max_branch_offset >= 2 * max_branch_offset - local_off)
    /* We need a stub.  Figure out whether a long_branch or plt_branch
       is needed later.  */
    return ppc_stub_long_branch;

  return ppc_stub_none;
}

/* Gets the address of a label (1:) in r11 and builds an offset in r12,
   then adds it to r11 (LOAD false) or loads r12 from r11+r12 (LOAD true).
   .	mflr	%r12
   .	bcl	20,31,1f
   .1:	mflr	%r11
   .	mtlr	%r12
   .	lis	%r12,xxx-1b@highest
   .	ori	%r12,%r12,xxx-1b@higher
   .	sldi	%r12,%r12,32
   .	oris	%r12,%r12,xxx-1b@high
   .	ori	%r12,%r12,xxx-1b@l
   .	add/ldx	%r12,%r11,%r12  */

static bfd_byte *
build_offset (bfd *abfd, bfd_byte *p, bfd_vma off, bool load)
{
  bfd_put_32 (abfd, MFLR_R12, p);
  p += 4;
  bfd_put_32 (abfd, BCL_20_31, p);
  p += 4;
  bfd_put_32 (abfd, MFLR_R11, p);
  p += 4;
  bfd_put_32 (abfd, MTLR_R12, p);
  p += 4;
  if (off + 0x8000 < 0x10000)
    {
      if (load)
	bfd_put_32 (abfd, LD_R12_0R11 + PPC_LO (off), p);
      else
	bfd_put_32 (abfd, ADDI_R12_R11 + PPC_LO (off), p);
      p += 4;
    }
  else if (off + 0x80008000ULL < 0x100000000ULL)
    {
      bfd_put_32 (abfd, ADDIS_R12_R11 + PPC_HA (off), p);
      p += 4;
      if (load)
	bfd_put_32 (abfd, LD_R12_0R12 + PPC_LO (off), p);
      else
	bfd_put_32 (abfd, ADDI_R12_R12 + PPC_LO (off), p);
      p += 4;
    }
  else
    {
      if (off + 0x800000000000ULL < 0x1000000000000ULL)
	{
	  bfd_put_32 (abfd, LI_R12_0 + ((off >> 32) & 0xffff), p);
	  p += 4;
	}
      else
	{
	  bfd_put_32 (abfd, LIS_R12 + ((off >> 48) & 0xffff), p);
	  p += 4;
	  if (((off >> 32) & 0xffff) != 0)
	    {
	      bfd_put_32 (abfd, ORI_R12_R12_0 + ((off >> 32) & 0xffff), p);
	      p += 4;
	    }
	}
      if (((off >> 32) & 0xffffffffULL) != 0)
	{
	  bfd_put_32 (abfd, SLDI_R12_R12_32, p);
	  p += 4;
	}
      if (PPC_HI (off) != 0)
	{
	  bfd_put_32 (abfd, ORIS_R12_R12_0 + PPC_HI (off), p);
	  p += 4;
	}
      if (PPC_LO (off) != 0)
	{
	  bfd_put_32 (abfd, ORI_R12_R12_0 + PPC_LO (off), p);
	  p += 4;
	}
      if (load)
	bfd_put_32 (abfd, LDX_R12_R11_R12, p);
      else
	bfd_put_32 (abfd, ADD_R12_R11_R12, p);
      p += 4;
    }
  return p;
}

static unsigned int
size_offset (bfd_vma off)
{
  unsigned int size;
  if (off + 0x8000 < 0x10000)
    size = 4;
  else if (off + 0x80008000ULL < 0x100000000ULL)
    size = 8;
  else
    {
      if (off + 0x800000000000ULL < 0x1000000000000ULL)
	size = 4;
      else
	{
	  size = 4;
	  if (((off >> 32) & 0xffff) != 0)
	    size += 4;
	}
      if (((off >> 32) & 0xffffffffULL) != 0)
	size += 4;
      if (PPC_HI (off) != 0)
	size += 4;
      if (PPC_LO (off) != 0)
	size += 4;
      size += 4;
    }
  return size + 16;
}

static unsigned int
num_relocs_for_offset (bfd_vma off)
{
  unsigned int num_rel;
  if (off + 0x8000 < 0x10000)
    num_rel = 1;
  else if (off + 0x80008000ULL < 0x100000000ULL)
    num_rel = 2;
  else
    {
      num_rel = 1;
      if (off + 0x800000000000ULL >= 0x1000000000000ULL
	  && ((off >> 32) & 0xffff) != 0)
	num_rel += 1;
      if (PPC_HI (off) != 0)
	num_rel += 1;
      if (PPC_LO (off) != 0)
	num_rel += 1;
    }
  return num_rel;
}

static Elf_Internal_Rela *
emit_relocs_for_offset (struct bfd_link_info *info, Elf_Internal_Rela *r,
			bfd_vma roff, bfd_vma targ, bfd_vma off)
{
  bfd_vma relative_targ = targ - (roff - 8);
  if (bfd_big_endian (info->output_bfd))
    roff += 2;
  r->r_offset = roff;
  r->r_addend = relative_targ + roff;
  if (off + 0x8000 < 0x10000)
    r->r_info = ELF64_R_INFO (0, R_PPC64_REL16);
  else if (off + 0x80008000ULL < 0x100000000ULL)
    {
      r->r_info = ELF64_R_INFO (0, R_PPC64_REL16_HA);
      ++r;
      roff += 4;
      r->r_offset = roff;
      r->r_info = ELF64_R_INFO (0, R_PPC64_REL16_LO);
      r->r_addend = relative_targ + roff;
    }
  else
    {
      if (off + 0x800000000000ULL < 0x1000000000000ULL)
	r->r_info = ELF64_R_INFO (0, R_PPC64_REL16_HIGHER);
      else
	{
	  r->r_info = ELF64_R_INFO (0, R_PPC64_REL16_HIGHEST);
	  if (((off >> 32) & 0xffff) != 0)
	    {
	      ++r;
	      roff += 4;
	      r->r_offset = roff;
	      r->r_info = ELF64_R_INFO (0, R_PPC64_REL16_HIGHER);
	      r->r_addend = relative_targ + roff;
	    }
	}
      if (((off >> 32) & 0xffffffffULL) != 0)
	roff += 4;
      if (PPC_HI (off) != 0)
	{
	  ++r;
	  roff += 4;
	  r->r_offset = roff;
	  r->r_info = ELF64_R_INFO (0, R_PPC64_REL16_HIGH);
	  r->r_addend = relative_targ + roff;
	}
      if (PPC_LO (off) != 0)
	{
	  ++r;
	  roff += 4;
	  r->r_offset = roff;
	  r->r_info = ELF64_R_INFO (0, R_PPC64_REL16_LO);
	  r->r_addend = relative_targ + roff;
	}
    }
  return r;
}

static bfd_byte *
build_power10_offset (bfd *abfd, bfd_byte *p, bfd_vma off, int odd,
		      bool load)
{
  uint64_t insn;
  if (off - odd + (1ULL << 33) < 1ULL << 34)
    {
      off -= odd;
      if (odd)
	{
	  bfd_put_32 (abfd, NOP, p);
	  p += 4;
	}
      if (load)
	insn = PLD_R12_PC;
      else
	insn = PADDI_R12_PC;
      insn |= D34 (off);
      bfd_put_32 (abfd, insn >> 32, p);
      p += 4;
      bfd_put_32 (abfd, insn, p);
    }
  /* The minimum value for paddi is -0x200000000.  The minimum value
     for li is -0x8000, which when shifted by 34 and added gives a
     minimum value of -0x2000200000000.  The maximum value is
     0x1ffffffff+0x7fff<<34 which is 0x2000200000000-1.  */
  else if (off - (8 - odd) + (0x20002ULL << 32) < 0x40004ULL << 32)
    {
      off -= 8 - odd;
      bfd_put_32 (abfd, LI_R11_0 | (HA34 (off) & 0xffff), p);
      p += 4;
      if (!odd)
	{
	  bfd_put_32 (abfd, SLDI_R11_R11_34, p);
	  p += 4;
	}
      insn = PADDI_R12_PC | D34 (off);
      bfd_put_32 (abfd, insn >> 32, p);
      p += 4;
      bfd_put_32 (abfd, insn, p);
      p += 4;
      if (odd)
	{
	  bfd_put_32 (abfd, SLDI_R11_R11_34, p);
	  p += 4;
	}
      if (load)
	bfd_put_32 (abfd, LDX_R12_R11_R12, p);
      else
	bfd_put_32 (abfd, ADD_R12_R11_R12, p);
    }
  else
    {
      off -= odd + 8;
      bfd_put_32 (abfd, LIS_R11 | ((HA34 (off) >> 16) & 0x3fff), p);
      p += 4;
      bfd_put_32 (abfd, ORI_R11_R11_0 | (HA34 (off) & 0xffff), p);
      p += 4;
      if (odd)
	{
	  bfd_put_32 (abfd, SLDI_R11_R11_34, p);
	  p += 4;
	}
      insn = PADDI_R12_PC | D34 (off);
      bfd_put_32 (abfd, insn >> 32, p);
      p += 4;
      bfd_put_32 (abfd, insn, p);
      p += 4;
      if (!odd)
	{
	  bfd_put_32 (abfd, SLDI_R11_R11_34, p);
	  p += 4;
	}
      if (load)
	bfd_put_32 (abfd, LDX_R12_R11_R12, p);
      else
	bfd_put_32 (abfd, ADD_R12_R11_R12, p);
    }
  p += 4;
  return p;
}

static unsigned int
size_power10_offset (bfd_vma off, int odd)
{
  if (off - odd + (1ULL << 33) < 1ULL << 34)
    return odd + 8;
  else if (off - (8 - odd) + (0x20002ULL << 32) < 0x40004ULL << 32)
    return 20;
  else
    return 24;
}

static unsigned int
num_relocs_for_power10_offset (bfd_vma off, int odd)
{
  if (off - odd + (1ULL << 33) < 1ULL << 34)
    return 1;
  else if (off - (8 - odd) + (0x20002ULL << 32) < 0x40004ULL << 32)
    return 2;
  else
    return 3;
}

static Elf_Internal_Rela *
emit_relocs_for_power10_offset (struct bfd_link_info *info,
				Elf_Internal_Rela *r, bfd_vma roff,
				bfd_vma targ, bfd_vma off, int odd)
{
  if (off - odd + (1ULL << 33) < 1ULL << 34)
    roff += odd;
  else if (off - (8 - odd) + (0x20002ULL << 32) < 0x40004ULL << 32)
    {
      int d_offset = bfd_big_endian (info->output_bfd) ? 2 : 0;
      r->r_offset = roff + d_offset;
      r->r_addend = targ + 8 - odd - d_offset;
      r->r_info = ELF64_R_INFO (0, R_PPC64_REL16_HIGHERA34);
      ++r;
      roff += 8 - odd;
    }
  else
    {
      int d_offset = bfd_big_endian (info->output_bfd) ? 2 : 0;
      r->r_offset = roff + d_offset;
      r->r_addend = targ + 8 + odd - d_offset;
      r->r_info = ELF64_R_INFO (0, R_PPC64_REL16_HIGHESTA34);
      ++r;
      roff += 4;
      r->r_offset = roff + d_offset;
      r->r_addend = targ + 4 + odd - d_offset;
      r->r_info = ELF64_R_INFO (0, R_PPC64_REL16_HIGHERA34);
      ++r;
      roff += 4 + odd;
    }
  r->r_offset = roff;
  r->r_addend = targ;
  r->r_info = ELF64_R_INFO (0, R_PPC64_PCREL34);
  return r;
}

/* Emit .eh_frame opcode to advance pc by DELTA.  */

static bfd_byte *
eh_advance (bfd *abfd, bfd_byte *eh, unsigned int delta)
{
  delta /= 4;
  if (delta < 64)
    *eh++ = DW_CFA_advance_loc + delta;
  else if (delta < 256)
    {
      *eh++ = DW_CFA_advance_loc1;
      *eh++ = delta;
    }
  else if (delta < 65536)
    {
      *eh++ = DW_CFA_advance_loc2;
      bfd_put_16 (abfd, delta, eh);
      eh += 2;
    }
  else
    {
      *eh++ = DW_CFA_advance_loc4;
      bfd_put_32 (abfd, delta, eh);
      eh += 4;
    }
  return eh;
}

/* Size of required .eh_frame opcode to advance pc by DELTA.  */

static unsigned int
eh_advance_size (unsigned int delta)
{
  if (delta < 64 * 4)
    /* DW_CFA_advance_loc+[1..63].  */
    return 1;
  if (delta < 256 * 4)
    /* DW_CFA_advance_loc1, byte.  */
    return 2;
  if (delta < 65536 * 4)
    /* DW_CFA_advance_loc2, 2 bytes.  */
    return 3;
  /* DW_CFA_advance_loc4, 4 bytes.  */
  return 5;
}

/* With power7 weakly ordered memory model, it is possible for ld.so
   to update a plt entry in one thread and have another thread see a
   stale zero toc entry.  To avoid this we need some sort of acquire
   barrier in the call stub.  One solution is to make the load of the
   toc word seem to appear to depend on the load of the function entry
   word.  Another solution is to test for r2 being zero, and branch to
   the appropriate glink entry if so.

   .	fake dep barrier	compare
   .	ld 12,xxx(2)		ld 12,xxx(2)
   .	mtctr 12		mtctr 12
   .	xor 11,12,12		ld 2,xxx+8(2)
   .	add 2,2,11		cmpldi 2,0
   .	ld 2,xxx+8(2)		bnectr+
   .	bctr			b <glink_entry>

   The solution involving the compare turns out to be faster, so
   that's what we use unless the branch won't reach.  */

#define ALWAYS_USE_FAKE_DEP 0
#define ALWAYS_EMIT_R2SAVE 0

static inline unsigned int
plt_stub_size (struct ppc_link_hash_table *htab,
	       struct ppc_stub_hash_entry *stub_entry,
	       bfd_vma off,
	       unsigned int odd)
{
  unsigned size;

  if (stub_entry->type.sub == ppc_stub_notoc)
    {
      size = 8 + size_power10_offset (off, odd);
      if (stub_entry->type.r2save)
	size += 4;
    }
  else if (stub_entry->type.sub == ppc_stub_p9notoc)
    {
      size = 8 + size_offset (off - 8);
      if (stub_entry->type.r2save)
	size += 4;
    }
  else
    {
      size = 12;
      if (ALWAYS_EMIT_R2SAVE || stub_entry->type.r2save)
	size += 4;
      if (PPC_HA (off) != 0)
	size += 4;
      if (htab->opd_abi)
	{
	  size += 4;
	  if (htab->params->plt_static_chain)
	    size += 4;
	  if (htab->params->plt_thread_safe
	      && htab->elf.dynamic_sections_created
	      && stub_entry->h != NULL
	      && stub_entry->h->elf.dynindx != -1)
	    size += 8;
	  if (PPC_HA (off + 8 + 8 * htab->params->plt_static_chain)
	      != PPC_HA (off))
	    size += 4;
	}
    }
  if (stub_entry->h != NULL
      && is_tls_get_addr (&stub_entry->h->elf, htab)
      && htab->params->tls_get_addr_opt)
    {
      if (!htab->params->no_tls_get_addr_regsave)
	{
	  size += 30 * 4;
	  if (stub_entry->type.r2save)
	    size += 4;
	}
      else
	{
	  size += 7 * 4;
	  if (stub_entry->type.r2save)
	    size += 6 * 4;
	}
    }
  return size;
}

/* Depending on the sign of plt_stub_align:
   If positive, return the padding to align to a 2**plt_stub_align
   boundary.
   If negative, if this stub would cross fewer 2**plt_stub_align
   boundaries if we align, then return the padding needed to do so.  */

static inline unsigned int
plt_stub_pad (int plt_stub_align,
	      bfd_vma stub_off,
	      unsigned int stub_size)
{
  unsigned int stub_align;

  if (plt_stub_align >= 0)
    stub_align = 1u << plt_stub_align;
  else
    {
      stub_align = 1u << -plt_stub_align;
      if (((stub_off + stub_size - 1) & -stub_align) - (stub_off & -stub_align)
	  <= ((stub_size - 1) & -stub_align))
	return 0;
    }
  return stub_align - 1 - ((stub_off - 1) & (stub_align - 1));
}

/* Build a toc using .plt call stub.  */

static inline bfd_byte *
build_plt_stub (struct ppc_link_hash_table *htab,
		struct ppc_stub_hash_entry *stub_entry,
		bfd_byte *p, bfd_vma offset, Elf_Internal_Rela *r)
{
  bfd *obfd = htab->params->stub_bfd;
  bool plt_load_toc = htab->opd_abi;
  bool plt_static_chain = htab->params->plt_static_chain;
  bool plt_thread_safe = (htab->params->plt_thread_safe
			  && htab->elf.dynamic_sections_created
			  && stub_entry->h != NULL
			  && stub_entry->h->elf.dynindx != -1);
  bool use_fake_dep = plt_thread_safe;
  bfd_vma cmp_branch_off = 0;

  if (!ALWAYS_USE_FAKE_DEP
      && plt_load_toc
      && plt_thread_safe
      && !(stub_entry->h != NULL
	   && is_tls_get_addr (&stub_entry->h->elf, htab)
	   && htab->params->tls_get_addr_opt))
    {
      bfd_vma pltoff = stub_entry->plt_ent->plt.offset & ~1;
      bfd_vma pltindex = ((pltoff - PLT_INITIAL_ENTRY_SIZE (htab))
			  / PLT_ENTRY_SIZE (htab));
      bfd_vma glinkoff = GLINK_PLTRESOLVE_SIZE (htab) + pltindex * 8;
      bfd_vma to, from;

      if (pltindex > 32768)
	glinkoff += (pltindex - 32768) * 4;
      to = (glinkoff
	    + htab->glink->output_offset
	    + htab->glink->output_section->vma);
      from = (p - stub_entry->group->stub_sec->contents
	      + 4 * (ALWAYS_EMIT_R2SAVE || stub_entry->type.r2save)
	      + 4 * (PPC_HA (offset) != 0)
	      + 4 * (PPC_HA (offset + 8 + 8 * plt_static_chain)
		     != PPC_HA (offset))
	      + 4 * (plt_static_chain != 0)
	      + 20
	      + stub_entry->group->stub_sec->output_offset
	      + stub_entry->group->stub_sec->output_section->vma);
      cmp_branch_off = to - from;
      use_fake_dep = cmp_branch_off + (1 << 25) >= (1 << 26);
    }

  if (PPC_HA (offset) != 0)
    {
      if (r != NULL)
	{
	  if (ALWAYS_EMIT_R2SAVE || stub_entry->type.r2save)
	    r[0].r_offset += 4;
	  r[0].r_info = ELF64_R_INFO (0, R_PPC64_TOC16_HA);
	  r[1].r_offset = r[0].r_offset + 4;
	  r[1].r_info = ELF64_R_INFO (0, R_PPC64_TOC16_LO_DS);
	  r[1].r_addend = r[0].r_addend;
	  if (plt_load_toc)
	    {
	      if (PPC_HA (offset + 8 + 8 * plt_static_chain) != PPC_HA (offset))
		{
		  r[2].r_offset = r[1].r_offset + 4;
		  r[2].r_info = ELF64_R_INFO (0, R_PPC64_TOC16_LO);
		  r[2].r_addend = r[0].r_addend;
		}
	      else
		{
		  r[2].r_offset = r[1].r_offset + 8 + 8 * use_fake_dep;
		  r[2].r_info = ELF64_R_INFO (0, R_PPC64_TOC16_LO_DS);
		  r[2].r_addend = r[0].r_addend + 8;
		  if (plt_static_chain)
		    {
		      r[3].r_offset = r[2].r_offset + 4;
		      r[3].r_info = ELF64_R_INFO (0, R_PPC64_TOC16_LO_DS);
		      r[3].r_addend = r[0].r_addend + 16;
		    }
		}
	    }
	}
      if (ALWAYS_EMIT_R2SAVE || stub_entry->type.r2save)
	bfd_put_32 (obfd, STD_R2_0R1 + STK_TOC (htab), p),	p += 4;
      if (plt_load_toc)
	{
	  bfd_put_32 (obfd, ADDIS_R11_R2 | PPC_HA (offset), p),	p += 4;
	  bfd_put_32 (obfd, LD_R12_0R11 | PPC_LO (offset), p),	p += 4;
	}
      else
	{
	  bfd_put_32 (obfd, ADDIS_R12_R2 | PPC_HA (offset), p),	p += 4;
	  bfd_put_32 (obfd, LD_R12_0R12 | PPC_LO (offset), p),	p += 4;
	}
      if (plt_load_toc
	  && PPC_HA (offset + 8 + 8 * plt_static_chain) != PPC_HA (offset))
	{
	  bfd_put_32 (obfd, ADDI_R11_R11 | PPC_LO (offset), p),	p += 4;
	  offset = 0;
	}
      bfd_put_32 (obfd, MTCTR_R12, p),				p += 4;
      if (plt_load_toc)
	{
	  if (use_fake_dep)
	    {
	      bfd_put_32 (obfd, XOR_R2_R12_R12, p),		p += 4;
	      bfd_put_32 (obfd, ADD_R11_R11_R2, p),		p += 4;
	    }
	  bfd_put_32 (obfd, LD_R2_0R11 | PPC_LO (offset + 8), p), p += 4;
	  if (plt_static_chain)
	    bfd_put_32 (obfd, LD_R11_0R11 | PPC_LO (offset + 16), p), p += 4;
	}
    }
  else
    {
      if (r != NULL)
	{
	  if (ALWAYS_EMIT_R2SAVE || stub_entry->type.r2save)
	    r[0].r_offset += 4;
	  r[0].r_info = ELF64_R_INFO (0, R_PPC64_TOC16_DS);
	  if (plt_load_toc)
	    {
	      if (PPC_HA (offset + 8 + 8 * plt_static_chain) != PPC_HA (offset))
		{
		  r[1].r_offset = r[0].r_offset + 4;
		  r[1].r_info = ELF64_R_INFO (0, R_PPC64_TOC16);
		  r[1].r_addend = r[0].r_addend;
		}
	      else
		{
		  r[1].r_offset = r[0].r_offset + 8 + 8 * use_fake_dep;
		  r[1].r_info = ELF64_R_INFO (0, R_PPC64_TOC16_DS);
		  r[1].r_addend = r[0].r_addend + 8 + 8 * plt_static_chain;
		  if (plt_static_chain)
		    {
		      r[2].r_offset = r[1].r_offset + 4;
		      r[2].r_info = ELF64_R_INFO (0, R_PPC64_TOC16_DS);
		      r[2].r_addend = r[0].r_addend + 8;
		    }
		}
	    }
	}
      if (ALWAYS_EMIT_R2SAVE || stub_entry->type.r2save)
	bfd_put_32 (obfd, STD_R2_0R1 + STK_TOC (htab), p),	p += 4;
      bfd_put_32 (obfd, LD_R12_0R2 | PPC_LO (offset), p),	p += 4;
      if (plt_load_toc
	  && PPC_HA (offset + 8 + 8 * plt_static_chain) != PPC_HA (offset))
	{
	  bfd_put_32 (obfd, ADDI_R2_R2 | PPC_LO (offset), p),	p += 4;
	  offset = 0;
	}
      bfd_put_32 (obfd, MTCTR_R12, p),				p += 4;
      if (plt_load_toc)
	{
	  if (use_fake_dep)
	    {
	      bfd_put_32 (obfd, XOR_R11_R12_R12, p),		p += 4;
	      bfd_put_32 (obfd, ADD_R2_R2_R11, p),		p += 4;
	    }
	  if (plt_static_chain)
	    bfd_put_32 (obfd, LD_R11_0R2 | PPC_LO (offset + 16), p), p += 4;
	  bfd_put_32 (obfd, LD_R2_0R2 | PPC_LO (offset + 8), p), p += 4;
	}
    }
  if (plt_load_toc && plt_thread_safe && !use_fake_dep)
    {
      bfd_put_32 (obfd, CMPLDI_R2_0, p),			p += 4;
      bfd_put_32 (obfd, BNECTR_P4, p),				p += 4;
      bfd_put_32 (obfd, B_DOT | (cmp_branch_off & 0x3fffffc), p), p += 4;
    }
  else
    bfd_put_32 (obfd, BCTR, p),					p += 4;
  return p;
}

/* Build a special .plt call stub for __tls_get_addr.  */

#define LD_R0_0R3	0xe8030000
#define LD_R12_0R3	0xe9830000
#define MR_R0_R3	0x7c601b78
#define CMPDI_R0_0	0x2c200000
#define ADD_R3_R12_R13	0x7c6c6a14
#define BEQLR		0x4d820020
#define MR_R3_R0	0x7c030378
#define BCTRL		0x4e800421

static bfd_byte *
build_tls_get_addr_head (struct ppc_link_hash_table *htab,
			 struct ppc_stub_hash_entry *stub_entry,
			 bfd_byte *p)
{
  bfd *obfd = htab->params->stub_bfd;

  bfd_put_32 (obfd, LD_R0_0R3 + 0, p),		p += 4;
  bfd_put_32 (obfd, LD_R12_0R3 + 8, p),		p += 4;
  bfd_put_32 (obfd, CMPDI_R0_0, p),		p += 4;
  bfd_put_32 (obfd, MR_R0_R3, p),		p += 4;
  bfd_put_32 (obfd, ADD_R3_R12_R13, p),		p += 4;
  bfd_put_32 (obfd, BEQLR, p),			p += 4;
  bfd_put_32 (obfd, MR_R3_R0, p),		p += 4;

  if (!htab->params->no_tls_get_addr_regsave)
    p = tls_get_addr_prologue (obfd, p, htab);
  else if (stub_entry->type.r2save)
    {
      bfd_put_32 (obfd, MFLR_R0, p);
      p += 4;
      bfd_put_32 (obfd, STD_R0_0R1 + STK_LINKER (htab), p);
      p += 4;
    }
  return p;
}

static bfd_byte *
build_tls_get_addr_tail (struct ppc_link_hash_table *htab,
			 struct ppc_stub_hash_entry *stub_entry,
			 bfd_byte *p,
			 bfd_byte *loc)
{
  bfd *obfd = htab->params->stub_bfd;

  if (!htab->params->no_tls_get_addr_regsave)
    {
      bfd_put_32 (obfd, BCTRL, p - 4);

      if (stub_entry->type.r2save)
	{
	  bfd_put_32 (obfd, LD_R2_0R1 + STK_TOC (htab), p);
	  p += 4;
	}
      p = tls_get_addr_epilogue (obfd, p, htab);
    }
  else if (stub_entry->type.r2save)
    {
      bfd_put_32 (obfd, BCTRL, p - 4);

      bfd_put_32 (obfd, LD_R2_0R1 + STK_TOC (htab), p);
      p += 4;
      bfd_put_32 (obfd, LD_R0_0R1 + STK_LINKER (htab), p);
      p += 4;
      bfd_put_32 (obfd, MTLR_R0, p);
      p += 4;
      bfd_put_32 (obfd, BLR, p);
      p += 4;
    }

  if (htab->glink_eh_frame != NULL
      && htab->glink_eh_frame->size != 0)
    {
      bfd_byte *base, *eh;

      base = htab->glink_eh_frame->contents + stub_entry->group->eh_base + 17;
      eh = base + stub_entry->group->eh_size;

      if (!htab->params->no_tls_get_addr_regsave)
	{
	  unsigned int cfa_updt, delta, i;

	  /* After the bctrl, lr has been modified so we need to emit
	     .eh_frame info saying the return address is on the stack.  In
	     fact we must put the EH info at or before the call rather
	     than after it, because the EH info for a call needs to be
	     specified by that point.
	     See libgcc/unwind-dw2.c execute_cfa_program.
	     Any stack pointer update must be described immediately after
	     the instruction making the change, and since the stdu occurs
	     after saving regs we put all the reg saves and the cfa
	     change there.  */
	  cfa_updt = stub_entry->stub_offset + 18 * 4;
	  delta = cfa_updt - stub_entry->group->lr_restore;
	  stub_entry->group->lr_restore
	    = stub_entry->stub_offset + (p - loc) - 4;
	  eh = eh_advance (htab->elf.dynobj, eh, delta);
	  *eh++ = DW_CFA_def_cfa_offset;
	  if (htab->opd_abi)
	    {
	      *eh++ = 128;
	      *eh++ = 1;
	    }
	  else
	    *eh++ = 96;
	  *eh++ = DW_CFA_offset_extended_sf;
	  *eh++ = 65;
	  *eh++ = (-16 / 8) & 0x7f;
	  for (i = 4; i < 12; i++)
	    {
	      *eh++ = DW_CFA_offset + i;
	      *eh++ = (htab->opd_abi ? 13 : 12) - i;
	    }
	  *eh++ = (DW_CFA_advance_loc
		   + (stub_entry->group->lr_restore - 8 - cfa_updt) / 4);
	  *eh++ = DW_CFA_def_cfa_offset;
	  *eh++ = 0;
	  for (i = 4; i < 12; i++)
	    *eh++ = DW_CFA_restore + i;
	  *eh++ = DW_CFA_advance_loc + 2;
	  *eh++ = DW_CFA_restore_extended;
	  *eh++ = 65;
	  stub_entry->group->eh_size = eh - base;
	}
      else if (stub_entry->type.r2save)
	{
	  unsigned int lr_used, delta;

	  lr_used = stub_entry->stub_offset + (p - 20 - loc);
	  delta = lr_used - stub_entry->group->lr_restore;
	  stub_entry->group->lr_restore = lr_used + 16;
	  eh = eh_advance (htab->elf.dynobj, eh, delta);
	  *eh++ = DW_CFA_offset_extended_sf;
	  *eh++ = 65;
	  *eh++ = -(STK_LINKER (htab) / 8) & 0x7f;
	  *eh++ = DW_CFA_advance_loc + 4;
	  *eh++ = DW_CFA_restore_extended;
	  *eh++ = 65;
	  stub_entry->group->eh_size = eh - base;
	}
    }
  return p;
}

static Elf_Internal_Rela *
get_relocs (asection *sec, int count)
{
  Elf_Internal_Rela *relocs;
  struct bfd_elf_section_data *elfsec_data;

  elfsec_data = elf_section_data (sec);
  relocs = elfsec_data->relocs;
  if (relocs == NULL)
    {
      bfd_size_type relsize;
      relsize = sec->reloc_count * sizeof (*relocs);
      relocs = bfd_alloc (sec->owner, relsize);
      if (relocs == NULL)
	return NULL;
      elfsec_data->relocs = relocs;
      elfsec_data->rela.hdr = bfd_zalloc (sec->owner,
					  sizeof (Elf_Internal_Shdr));
      if (elfsec_data->rela.hdr == NULL)
	return NULL;
      elfsec_data->rela.hdr->sh_size = (sec->reloc_count
					* sizeof (Elf64_External_Rela));
      elfsec_data->rela.hdr->sh_entsize = sizeof (Elf64_External_Rela);
      sec->reloc_count = 0;
    }
  relocs += sec->reloc_count;
  sec->reloc_count += count;
  return relocs;
}

/* Convert the relocs R[0] thru R[-NUM_REL+1], which are all no-symbol
   forms, to the equivalent relocs against the global symbol given by
   STUB_ENTRY->H.  */

static bool
use_global_in_relocs (struct ppc_link_hash_table *htab,
		      struct ppc_stub_hash_entry *stub_entry,
		      Elf_Internal_Rela *r, unsigned int num_rel)
{
  struct elf_link_hash_entry **hashes;
  unsigned long symndx;
  struct ppc_link_hash_entry *h;
  bfd_vma symval;

  /* Relocs are always against symbols in their own object file.  Fake
     up global sym hashes for the stub bfd (which has no symbols).  */
  hashes = elf_sym_hashes (htab->params->stub_bfd);
  if (hashes == NULL)
    {
      bfd_size_type hsize;

      /* When called the first time, stub_globals will contain the
	 total number of symbols seen during stub sizing.  After
	 allocating, stub_globals is used as an index to fill the
	 hashes array.  */
      hsize = (htab->stub_globals + 1) * sizeof (*hashes);
      hashes = bfd_zalloc (htab->params->stub_bfd, hsize);
      if (hashes == NULL)
	return false;
      elf_sym_hashes (htab->params->stub_bfd) = hashes;
      htab->stub_globals = 1;
    }
  symndx = htab->stub_globals++;
  h = stub_entry->h;
  hashes[symndx] = &h->elf;
  if (h->oh != NULL && h->oh->is_func)
    h = ppc_follow_link (h->oh);
  BFD_ASSERT (h->elf.root.type == bfd_link_hash_defined
	      || h->elf.root.type == bfd_link_hash_defweak);
  symval = defined_sym_val (&h->elf);
  while (num_rel-- != 0)
    {
      r->r_info = ELF64_R_INFO (symndx, ELF64_R_TYPE (r->r_info));
      if (h->elf.root.u.def.section != stub_entry->target_section)
	{
	  /* H is an opd symbol.  The addend must be zero, and the
	     branch reloc is the only one we can convert.  */
	  r->r_addend = 0;
	  break;
	}
      else
	r->r_addend -= symval;
      --r;
    }
  return true;
}

static bfd_vma
get_r2off (struct bfd_link_info *info,
	   struct ppc_stub_hash_entry *stub_entry)
{
  struct ppc_link_hash_table *htab = ppc_hash_table (info);
  bfd_vma r2off = htab->sec_info[stub_entry->target_section->id].toc_off;

  if (r2off == 0)
    {
      /* Support linking -R objects.  Get the toc pointer from the
	 opd entry.  */
      char buf[8];
      if (!htab->opd_abi)
	return r2off;
      asection *opd = stub_entry->h->elf.root.u.def.section;
      bfd_vma opd_off = stub_entry->h->elf.root.u.def.value;

      if (strcmp (opd->name, ".opd") != 0
	  || opd->reloc_count != 0)
	{
	  info->callbacks->einfo
	    (_("%P: cannot find opd entry toc for `%pT'\n"),
	     stub_entry->h->elf.root.root.string);
	  bfd_set_error (bfd_error_bad_value);
	  return (bfd_vma) -1;
	}
      if (!bfd_get_section_contents (opd->owner, opd, buf, opd_off + 8, 8))
	return (bfd_vma) -1;
      r2off = bfd_get_64 (opd->owner, buf);
      r2off -= elf_gp (info->output_bfd);
    }
  r2off -= htab->sec_info[stub_entry->group->link_sec->id].toc_off;
  return r2off;
}

/* Debug dump.  */

static void
dump_stub (const char *header,
	   struct ppc_stub_hash_entry *stub_entry,
	   size_t end_offset)
{
  const char *t1, *t2, *t3;
  switch (stub_entry->type.main)
    {
    case ppc_stub_none:		t1 = "none";		break;
    case ppc_stub_long_branch:	t1 = "long_branch";	break;
    case ppc_stub_plt_branch:	t1 = "plt_branch";	break;
    case ppc_stub_plt_call:	t1 = "plt_call";	break;
    case ppc_stub_global_entry:	t1 = "global_entry";	break;
    case ppc_stub_save_res:	t1 = "save_res";	break;
    default:			t1 = "???";		break;
    }
  switch (stub_entry->type.sub)
    {
    case ppc_stub_toc:		t2 = "toc";		break;
    case ppc_stub_notoc:	t2 = "notoc";		break;
    case ppc_stub_p9notoc:	t2 = "p9notoc";		break;
    default:			t2 = "???";		break;
    }
  t3 = stub_entry->type.r2save ? "r2save" : "";
  fprintf (stderr, "%s id = %u type = %s:%s:%s\n",
	   header, stub_entry->id, t1, t2, t3);
  fprintf (stderr, "name = %s\n", stub_entry->root.string);
  fprintf (stderr, "offset = 0x%" PRIx64 ":", stub_entry->stub_offset);
  for (size_t i = stub_entry->stub_offset; i < end_offset; i += 4)
    {
      asection *stub_sec = stub_entry->group->stub_sec;
      uint32_t *p = (uint32_t *) (stub_sec->contents + i);
      fprintf (stderr, " %08x", (uint32_t) bfd_get_32 (stub_sec->owner, p));
    }
  fprintf (stderr, "\n");
}

static bool
ppc_build_one_stub (struct bfd_hash_entry *gen_entry, void *in_arg)
{
  struct ppc_stub_hash_entry *stub_entry;
  struct ppc_branch_hash_entry *br_entry;
  struct bfd_link_info *info;
  struct ppc_link_hash_table *htab;
  bfd *obfd;
  bfd_byte *loc;
  bfd_byte *p, *relp;
  bfd_vma targ, off;
  Elf_Internal_Rela *r;
  asection *plt;
  int num_rel;
  int odd;
  bool is_tga;

  /* Massage our args to the form they really have.  */
  stub_entry = (struct ppc_stub_hash_entry *) gen_entry;
  info = in_arg;

  htab = ppc_hash_table (info);
  if (htab == NULL)
    return false;

  struct _ppc64_elf_section_data *esd
    = ppc64_elf_section_data (stub_entry->group->stub_sec);
  ++htab->stub_id;
  if (stub_entry->id != htab->stub_id
      || (stub_entry->type.main != ppc_stub_save_res
	  && stub_entry->stub_offset < stub_entry->group->stub_sec->size))
    {
      BFD_ASSERT (0);
      if (stub_entry->id != htab->stub_id)
	fprintf (stderr, "Expected id %u, got %u\n",
		 htab->stub_id, stub_entry->id);
      if (stub_entry->stub_offset < stub_entry->group->stub_sec->size)
	fprintf (stderr, "Expected offset >= %" PRIx64 ", got %"
		 PRIx64 "\n", stub_entry->group->stub_sec->size,
		 stub_entry->stub_offset);
      if (esd->sec_type == sec_stub)
	dump_stub ("Previous:", esd->u.last_ent, stub_entry->stub_offset);
      dump_stub ("Current:", stub_entry, 0);
    }
  if (esd->sec_type == sec_normal)
    esd->sec_type = sec_stub;
  if (esd->sec_type == sec_stub)
    esd->u.last_ent = stub_entry;
  loc = stub_entry->group->stub_sec->contents + stub_entry->stub_offset;

  htab->stub_count[stub_entry->type.main - 1] += 1;
  if (stub_entry->type.main == ppc_stub_long_branch
      && stub_entry->type.sub == ppc_stub_toc)
    {
      /* Branches are relative.  This is where we are going to.  */
      targ = (stub_entry->target_value
	      + stub_entry->target_section->output_offset
	      + stub_entry->target_section->output_section->vma);
      targ += PPC64_LOCAL_ENTRY_OFFSET (stub_entry->other);

      /* And this is where we are coming from.  */
      off = (stub_entry->stub_offset
	     + stub_entry->group->stub_sec->output_offset
	     + stub_entry->group->stub_sec->output_section->vma);
      off = targ - off;

      p = loc;
      obfd = htab->params->stub_bfd;
      if (stub_entry->type.r2save)
	{
	  bfd_vma r2off = get_r2off (info, stub_entry);

	  if (r2off == (bfd_vma) -1)
	    {
	      htab->stub_error = true;
	      return false;
	    }
	  bfd_put_32 (obfd, STD_R2_0R1 + STK_TOC (htab), p);
	  p += 4;
	  if (PPC_HA (r2off) != 0)
	    {
	      bfd_put_32 (obfd, ADDIS_R2_R2 | PPC_HA (r2off), p);
	      p += 4;
	    }
	  if (PPC_LO (r2off) != 0)
	    {
	      bfd_put_32 (obfd, ADDI_R2_R2 | PPC_LO (r2off), p);
	      p += 4;
	    }
	  off -= p - loc;
	}
      bfd_put_32 (obfd, B_DOT | (off & 0x3fffffc), p);
      p += 4;

      if (off + (1 << 25) >= (bfd_vma) (1 << 26))
	{
	  _bfd_error_handler
	    (_("long branch stub `%s' offset overflow"),
	     stub_entry->root.string);
	  htab->stub_error = true;
	  return false;
	}

      if (info->emitrelocations)
	{
	  r = get_relocs (stub_entry->group->stub_sec, 1);
	  if (r == NULL)
	    return false;
	  r->r_offset = p - 4 - stub_entry->group->stub_sec->contents;
	  r->r_info = ELF64_R_INFO (0, R_PPC64_REL24);
	  r->r_addend = targ;
	  if (stub_entry->h != NULL
	      && !use_global_in_relocs (htab, stub_entry, r, 1))
	    return false;
	}
    }
  else if (stub_entry->type.main == ppc_stub_plt_branch
	   && stub_entry->type.sub == ppc_stub_toc)
    {
      br_entry = ppc_branch_hash_lookup (&htab->branch_hash_table,
					 stub_entry->root.string + 9,
					 false, false);
      if (br_entry == NULL)
	{
	  _bfd_error_handler (_("can't find branch stub `%s'"),
			      stub_entry->root.string);
	  htab->stub_error = true;
	  return false;
	}

      targ = (stub_entry->target_value
	      + stub_entry->target_section->output_offset
	      + stub_entry->target_section->output_section->vma);
      if (!stub_entry->type.r2save)
	targ += PPC64_LOCAL_ENTRY_OFFSET (stub_entry->other);

      bfd_put_64 (htab->brlt->owner, targ,
		  htab->brlt->contents + br_entry->offset);

      if (br_entry->iter == htab->stub_iteration)
	{
	  br_entry->iter = 0;

	  if (htab->relbrlt != NULL && !info->enable_dt_relr)
	    {
	      /* Create a reloc for the branch lookup table entry.  */
	      Elf_Internal_Rela rela;
	      bfd_byte *rl;

	      rela.r_offset = (br_entry->offset
			       + htab->brlt->output_offset
			       + htab->brlt->output_section->vma);
	      rela.r_info = ELF64_R_INFO (0, R_PPC64_RELATIVE);
	      rela.r_addend = targ;

	      rl = htab->relbrlt->contents;
	      rl += (htab->relbrlt->reloc_count++
		     * sizeof (Elf64_External_Rela));
	      bfd_elf64_swap_reloca_out (htab->relbrlt->owner, &rela, rl);
	    }
	  else if (info->emitrelocations)
	    {
	      r = get_relocs (htab->brlt, 1);
	      if (r == NULL)
		return false;
	      /* brlt, being SEC_LINKER_CREATED does not go through the
		 normal reloc processing.  Symbols and offsets are not
		 translated from input file to output file form, so
		 set up the offset per the output file.  */
	      r->r_offset = (br_entry->offset
			     + htab->brlt->output_offset
			     + htab->brlt->output_section->vma);
	      r->r_info = ELF64_R_INFO (0, R_PPC64_RELATIVE);
	      r->r_addend = targ;
	    }
	}

      targ = (br_entry->offset
	      + htab->brlt->output_offset
	      + htab->brlt->output_section->vma);

      off = (elf_gp (info->output_bfd)
	     + htab->sec_info[stub_entry->group->link_sec->id].toc_off);
      off = targ - off;

      if (off + 0x80008000 > 0xffffffff || (off & 7) != 0)
	{
	  info->callbacks->einfo
	    (_("%P: linkage table error against `%pT'\n"),
	     stub_entry->root.string);
	  bfd_set_error (bfd_error_bad_value);
	  htab->stub_error = true;
	  return false;
	}

      if (info->emitrelocations)
	{
	  r = get_relocs (stub_entry->group->stub_sec, 1 + (PPC_HA (off) != 0));
	  if (r == NULL)
	    return false;
	  r[0].r_offset = loc - stub_entry->group->stub_sec->contents;
	  if (bfd_big_endian (info->output_bfd))
	    r[0].r_offset += 2;
	  if (stub_entry->type.r2save)
	    r[0].r_offset += 4;
	  r[0].r_info = ELF64_R_INFO (0, R_PPC64_TOC16_DS);
	  r[0].r_addend = targ;
	  if (PPC_HA (off) != 0)
	    {
	      r[0].r_info = ELF64_R_INFO (0, R_PPC64_TOC16_HA);
	      r[1].r_offset = r[0].r_offset + 4;
	      r[1].r_info = ELF64_R_INFO (0, R_PPC64_TOC16_LO_DS);
	      r[1].r_addend = r[0].r_addend;
	    }
	}

      p = loc;
      obfd = htab->params->stub_bfd;
      if (!stub_entry->type.r2save)
	{
	  if (PPC_HA (off) != 0)
	    {
	      bfd_put_32 (obfd, ADDIS_R12_R2 | PPC_HA (off), p);
	      p += 4;
	      bfd_put_32 (obfd, LD_R12_0R12 | PPC_LO (off), p);
	    }
	  else
	    bfd_put_32 (obfd, LD_R12_0R2 | PPC_LO (off), p);
	}
      else
	{
	  bfd_vma r2off = get_r2off (info, stub_entry);

	  if (r2off == (bfd_vma) -1)
	    {
	      htab->stub_error = true;
	      return false;
	    }

	  bfd_put_32 (obfd, STD_R2_0R1 + STK_TOC (htab), p);
	  p += 4;
	  if (PPC_HA (off) != 0)
	    {
	      bfd_put_32 (obfd, ADDIS_R12_R2 | PPC_HA (off), p);
	      p += 4;
	      bfd_put_32 (obfd, LD_R12_0R12 | PPC_LO (off), p);
	    }
	  else
	    bfd_put_32 (obfd, LD_R12_0R2 | PPC_LO (off), p);

	  if (PPC_HA (r2off) != 0)
	    {
	      p += 4;
	      bfd_put_32 (obfd, ADDIS_R2_R2 | PPC_HA (r2off), p);
	    }
	  if (PPC_LO (r2off) != 0)
	    {
	      p += 4;
	      bfd_put_32 (obfd, ADDI_R2_R2 | PPC_LO (r2off), p);
	    }
	}
      p += 4;
      bfd_put_32 (obfd, MTCTR_R12, p);
      p += 4;
      bfd_put_32 (obfd, BCTR, p);
      p += 4;
    }
  else if (stub_entry->type.sub >= ppc_stub_notoc)
    {
      bool is_plt = stub_entry->type.main == ppc_stub_plt_call;
      p = loc;
      off = (stub_entry->stub_offset
	     + stub_entry->group->stub_sec->output_offset
	     + stub_entry->group->stub_sec->output_section->vma);
      obfd = htab->params->stub_bfd;
      is_tga = (is_plt
		&& stub_entry->h != NULL
		&& is_tls_get_addr (&stub_entry->h->elf, htab)
		&& htab->params->tls_get_addr_opt);
      if (is_tga)
	{
	  p = build_tls_get_addr_head (htab, stub_entry, p);
	  off += p - loc;
	}
      if (stub_entry->type.r2save)
	{
	  off += 4;
	  bfd_put_32 (obfd, STD_R2_0R1 + STK_TOC (htab), p);
	  p += 4;
	}
      if (is_plt)
	{
	  targ = stub_entry->plt_ent->plt.offset & ~1;
	  if (targ >= (bfd_vma) -2)
	    abort ();

	  plt = htab->elf.splt;
	  if (use_local_plt (info, elf_hash_entry (stub_entry->h)))
	    {
	      if (stub_entry->symtype == STT_GNU_IFUNC)
		plt = htab->elf.iplt;
	      else
		plt = htab->pltlocal;
	    }
	  targ += plt->output_offset + plt->output_section->vma;
	}
      else
	targ = (stub_entry->target_value
		+ stub_entry->target_section->output_offset
		+ stub_entry->target_section->output_section->vma);
      odd = off & 4;
      off = targ - off;

      relp = p;
      num_rel = 0;
      if (stub_entry->type.sub == ppc_stub_notoc)
	p = build_power10_offset (obfd, p, off, odd, is_plt);
      else
	{
	  if (htab->glink_eh_frame != NULL
	      && htab->glink_eh_frame->size != 0)
	    {
	      bfd_byte *base, *eh;
	      unsigned int lr_used, delta;

	      base = (htab->glink_eh_frame->contents
		      + stub_entry->group->eh_base + 17);
	      eh = base + stub_entry->group->eh_size;
	      lr_used = stub_entry->stub_offset + (p - loc) + 8;
	      delta = lr_used - stub_entry->group->lr_restore;
	      stub_entry->group->lr_restore = lr_used + 8;
	      eh = eh_advance (htab->elf.dynobj, eh, delta);
	      *eh++ = DW_CFA_register;
	      *eh++ = 65;
	      *eh++ = 12;
	      *eh++ = DW_CFA_advance_loc + 2;
	      *eh++ = DW_CFA_restore_extended;
	      *eh++ = 65;
	      stub_entry->group->eh_size = eh - base;
	    }

	  /* The notoc stubs calculate their target (either a PLT entry or
	     the global entry point of a function) relative to the PC
	     returned by the "bcl" two instructions past the start of the
	     sequence emitted by build_offset.  The offset is therefore 8
	     less than calculated from the start of the sequence.  */
	  off -= 8;
	  p = build_offset (obfd, p, off, is_plt);
	}

      if (stub_entry->type.main == ppc_stub_long_branch)
	{
	  bfd_vma from;
	  num_rel = 1;
	  from = (stub_entry->stub_offset
		  + stub_entry->group->stub_sec->output_offset
		  + stub_entry->group->stub_sec->output_section->vma
		  + (p - loc));
	  bfd_put_32 (obfd, B_DOT | ((targ - from) & 0x3fffffc), p);
	}
      else
	{
	  bfd_put_32 (obfd, MTCTR_R12, p);
	  p += 4;
	  bfd_put_32 (obfd, BCTR, p);
	}
      p += 4;

      if (is_tga)
	p = build_tls_get_addr_tail (htab, stub_entry, p, loc);

      if (info->emitrelocations)
	{
	  bfd_vma roff = relp - stub_entry->group->stub_sec->contents;
	  if (stub_entry->type.sub == ppc_stub_notoc)
	    num_rel += num_relocs_for_power10_offset (off, odd);
	  else
	    {
	      num_rel += num_relocs_for_offset (off);
	      roff += 16;
	    }
	  r = get_relocs (stub_entry->group->stub_sec, num_rel);
	  if (r == NULL)
	    return false;
	  if (stub_entry->type.sub == ppc_stub_notoc)
	    r = emit_relocs_for_power10_offset (info, r, roff, targ, off, odd);
	  else
	    r = emit_relocs_for_offset (info, r, roff, targ, off);
	  if (stub_entry->type.main == ppc_stub_long_branch)
	    {
	      ++r;
	      roff = p - 4 - stub_entry->group->stub_sec->contents;
	      r->r_offset = roff;
	      r->r_info = ELF64_R_INFO (0, R_PPC64_REL24);
	      r->r_addend = targ;
	      if (stub_entry->h != NULL
		  && !use_global_in_relocs (htab, stub_entry, r, num_rel))
		return false;
	    }
	}
    }
  else if (stub_entry->type.main == ppc_stub_plt_call)
    {
      if (stub_entry->h != NULL
	  && stub_entry->h->is_func_descriptor
	  && stub_entry->h->oh != NULL)
	{
	  struct ppc_link_hash_entry *fh = ppc_follow_link (stub_entry->h->oh);

	  /* If the old-ABI "dot-symbol" is undefined make it weak so
	     we don't get a link error from RELOC_FOR_GLOBAL_SYMBOL.  */
	  if (fh->elf.root.type == bfd_link_hash_undefined
	      && (stub_entry->h->elf.root.type == bfd_link_hash_defined
		  || stub_entry->h->elf.root.type == bfd_link_hash_defweak))
	    fh->elf.root.type = bfd_link_hash_undefweak;
	}

      /* Now build the stub.  */
      targ = stub_entry->plt_ent->plt.offset & ~1;
      if (targ >= (bfd_vma) -2)
	abort ();

      plt = htab->elf.splt;
      if (use_local_plt (info, elf_hash_entry (stub_entry->h)))
	{
	  if (stub_entry->symtype == STT_GNU_IFUNC)
	    plt = htab->elf.iplt;
	  else
	    plt = htab->pltlocal;
	}
      targ += plt->output_offset + plt->output_section->vma;

      off = (elf_gp (info->output_bfd)
	     + htab->sec_info[stub_entry->group->link_sec->id].toc_off);
      off = targ - off;

      if (off + 0x80008000 > 0xffffffff || (off & 7) != 0)
	{
	  info->callbacks->einfo
	    /* xgettext:c-format */
	    (_("%P: linkage table error against `%pT'\n"),
	     stub_entry->h != NULL
	     ? stub_entry->h->elf.root.root.string
	     : "<local sym>");
	  bfd_set_error (bfd_error_bad_value);
	  htab->stub_error = true;
	  return false;
	}

      r = NULL;
      if (info->emitrelocations)
	{
	  r = get_relocs (stub_entry->group->stub_sec,
			  ((PPC_HA (off) != 0)
			   + (htab->opd_abi
			      ? 2 + (htab->params->plt_static_chain
				     && PPC_HA (off + 16) == PPC_HA (off))
			      : 1)));
	  if (r == NULL)
	    return false;
	  r[0].r_offset = loc - stub_entry->group->stub_sec->contents;
	  if (bfd_big_endian (info->output_bfd))
	    r[0].r_offset += 2;
	  r[0].r_addend = targ;
	}
      p = loc;
      obfd = htab->params->stub_bfd;
      is_tga = (stub_entry->h != NULL
		&& is_tls_get_addr (&stub_entry->h->elf, htab)
		&& htab->params->tls_get_addr_opt);
      if (is_tga)
	{
	  p = build_tls_get_addr_head (htab, stub_entry, p);
	  if (r != NULL)
	    r[0].r_offset += p - loc;
	}
      p = build_plt_stub (htab, stub_entry, p, off, r);
      if (is_tga)
	p = build_tls_get_addr_tail (htab, stub_entry, p, loc);
    }
  else if (stub_entry->type.main == ppc_stub_save_res)
    return true;
  else
    {
      BFD_FAIL ();
      return false;
    }

  stub_entry->group->stub_sec->size = stub_entry->stub_offset + (p - loc);

  if (htab->params->emit_stub_syms)
    {
      struct elf_link_hash_entry *h;
      size_t len1, len2;
      char *name;
      const char *const stub_str[] = { "long_branch",
				       "plt_branch",
				       "plt_call" };

      len1 = strlen (stub_str[stub_entry->type.main - 1]);
      len2 = strlen (stub_entry->root.string);
      name = bfd_malloc (len1 + len2 + 2);
      if (name == NULL)
	return false;
      memcpy (name, stub_entry->root.string, 9);
      memcpy (name + 9, stub_str[stub_entry->type.main - 1], len1);
      memcpy (name + len1 + 9, stub_entry->root.string + 8, len2 - 8 + 1);
      h = elf_link_hash_lookup (&htab->elf, name, true, false, false);
      if (h == NULL)
	return false;
      if (h->root.type == bfd_link_hash_new)
	{
	  h->root.type = bfd_link_hash_defined;
	  h->root.u.def.section = stub_entry->group->stub_sec;
	  h->root.u.def.value = stub_entry->stub_offset;
	  h->ref_regular = 1;
	  h->def_regular = 1;
	  h->ref_regular_nonweak = 1;
	  h->forced_local = 1;
	  h->non_elf = 0;
	  h->root.linker_def = 1;
	}
    }

  return true;
}

/* As above, but don't actually build the stub.  Just bump offset so
   we know stub section sizes, and select plt_branch stubs where
   long_branch stubs won't do.  */

static bool
ppc_size_one_stub (struct bfd_hash_entry *gen_entry, void *in_arg)
{
  struct ppc_stub_hash_entry *stub_entry;
  struct bfd_link_info *info;
  struct ppc_link_hash_table *htab;
  asection *plt;
  bfd_vma targ, off, r2off;
  unsigned int size, pad, extra, lr_used, delta, odd;
  bfd_vma stub_offset;

  /* Massage our args to the form they really have.  */
  stub_entry = (struct ppc_stub_hash_entry *) gen_entry;
  info = in_arg;

  htab = ppc_hash_table (info);
  if (htab == NULL)
    return false;

  /* Fail if the target section could not be assigned to an output
     section.  The user should fix his linker script.  */
  if (stub_entry->target_section != NULL
      && stub_entry->target_section->output_section == NULL
      && info->non_contiguous_regions)
    info->callbacks->einfo (_("%F%P: Could not assign `%pA' to an output section. "
			      "Retry without --enable-non-contiguous-regions.\n"),
			    stub_entry->target_section);

  /* Same for the group.  */
  if (stub_entry->group->stub_sec != NULL
      && stub_entry->group->stub_sec->output_section == NULL
      && info->non_contiguous_regions)
    info->callbacks->einfo (_("%F%P: Could not assign `%pA' to an output section. "
			      "Retry without --enable-non-contiguous-regions.\n"),
			    stub_entry->group->stub_sec);

  /* Make a note of the offset within the stubs for this entry.  */
  stub_offset = stub_entry->group->stub_sec->size;
  if (htab->stub_iteration > STUB_SHRINK_ITER
      && stub_entry->stub_offset > stub_offset)
    stub_offset = stub_entry->stub_offset;
  stub_entry->id = ++htab->stub_id;

  if (stub_entry->h != NULL
      && stub_entry->h->save_res
      && stub_entry->h->elf.root.type == bfd_link_hash_defined
      && stub_entry->h->elf.root.u.def.section == htab->sfpr)
    {
      /* Don't make stubs to out-of-line register save/restore
	 functions.  Instead, emit copies of the functions.  */
      stub_entry->group->needs_save_res = 1;
      stub_entry->type.main = ppc_stub_save_res;
      stub_entry->type.sub = ppc_stub_toc;
      stub_entry->type.r2save = 0;
      return true;
    }

  if (stub_entry->type.main == ppc_stub_plt_branch)
    {
      /* Reset the stub type from the plt branch variant in case we now
	 can reach with a shorter stub.  */
      stub_entry->type.main = ppc_stub_long_branch;
    }

  if (stub_entry->type.main == ppc_stub_long_branch
      && stub_entry->type.sub == ppc_stub_toc)
    {
      targ = (stub_entry->target_value
	      + stub_entry->target_section->output_offset
	      + stub_entry->target_section->output_section->vma);
      targ += PPC64_LOCAL_ENTRY_OFFSET (stub_entry->other);
      off = (stub_offset
	     + stub_entry->group->stub_sec->output_offset
	     + stub_entry->group->stub_sec->output_section->vma);

      size = 4;
      r2off = 0;
      if (stub_entry->type.r2save)
	{
	  r2off = get_r2off (info, stub_entry);
	  if (r2off == (bfd_vma) -1)
	    {
	      htab->stub_error = true;
	      return false;
	    }
	  size = 8;
	  if (PPC_HA (r2off) != 0)
	    size += 4;
	  if (PPC_LO (r2off) != 0)
	    size += 4;
	  off += size - 4;
	}
      off = targ - off;

      /* If the branch offset is too big, use a ppc_stub_plt_branch.
	 Do the same for -R objects without function descriptors.  */
      if ((stub_entry->type.r2save
	   && r2off == 0
	   && htab->sec_info[stub_entry->target_section->id].toc_off == 0)
	  || off + (1 << 25) >= (bfd_vma) (1 << 26))
	{
	  struct ppc_branch_hash_entry *br_entry;

	  br_entry = ppc_branch_hash_lookup (&htab->branch_hash_table,
					     stub_entry->root.string + 9,
					     true, false);
	  if (br_entry == NULL)
	    {
	      _bfd_error_handler (_("can't build branch stub `%s'"),
				  stub_entry->root.string);
	      htab->stub_error = true;
	      return false;
	    }

	  if (br_entry->iter != htab->stub_iteration)
	    {
	      br_entry->iter = htab->stub_iteration;
	      br_entry->offset = htab->brlt->size;
	      htab->brlt->size += 8;

	      if (htab->relbrlt != NULL && !info->enable_dt_relr)
		htab->relbrlt->size += sizeof (Elf64_External_Rela);
	      else if (info->emitrelocations)
		{
		  htab->brlt->reloc_count += 1;
		  htab->brlt->flags |= SEC_RELOC;
		}
	    }

	  targ = (br_entry->offset
		  + htab->brlt->output_offset
		  + htab->brlt->output_section->vma);
	  off = (elf_gp (info->output_bfd)
		 + htab->sec_info[stub_entry->group->link_sec->id].toc_off);
	  off = targ - off;

	  if (info->emitrelocations)
	    {
	      stub_entry->group->stub_sec->reloc_count
		+= 1 + (PPC_HA (off) != 0);
	      stub_entry->group->stub_sec->flags |= SEC_RELOC;
	    }

	  stub_entry->type.main = ppc_stub_plt_branch;
	  if (!stub_entry->type.r2save)
	    {
	      size = 12;
	      if (PPC_HA (off) != 0)
		size = 16;
	    }
	  else
	    {
	      size = 16;
	      if (PPC_HA (off) != 0)
		size += 4;

	      if (PPC_HA (r2off) != 0)
		size += 4;
	      if (PPC_LO (r2off) != 0)
		size += 4;
	    }
	  pad = plt_stub_pad (htab->params->plt_stub_align, stub_offset, size);
	  stub_offset += pad;
	}
      else if (info->emitrelocations)
	{
	  stub_entry->group->stub_sec->reloc_count += 1;
	  stub_entry->group->stub_sec->flags |= SEC_RELOC;
	}
    }
  else if (stub_entry->type.main == ppc_stub_long_branch)
    {
      off = (stub_offset
	     + stub_entry->group->stub_sec->output_offset
	     + stub_entry->group->stub_sec->output_section->vma);
      size = 0;
      if (stub_entry->type.r2save)
	size = 4;
      off += size;
      targ = (stub_entry->target_value
	      + stub_entry->target_section->output_offset
	      + stub_entry->target_section->output_section->vma);
      odd = off & 4;
      off = targ - off;

      if (stub_entry->type.sub == ppc_stub_notoc)
	extra = size_power10_offset (off, odd);
      else
	extra = size_offset (off - 8);
      /* Include branch insn plus those in the offset sequence.  */
      size += 4 + extra;

      /* If the branch can't reach, use a plt_branch.
	 The branch insn is at the end, or "extra" bytes along.  So
	 its offset will be "extra" bytes less that that already
	 calculated.  */
      if (off - extra + (1 << 25) >= (bfd_vma) (1 << 26))
	{
	  stub_entry->type.main = ppc_stub_plt_branch;
	  size += 4;
	  pad = plt_stub_pad (htab->params->plt_stub_align, stub_offset, size);
	  if (pad != 0)
	    {
	      stub_offset += pad;
	      off -= pad;
	      odd ^= pad & 4;
	      size -= extra;
	      if (stub_entry->type.sub == ppc_stub_notoc)
		extra = size_power10_offset (off, odd);
	      else
		extra = size_offset (off - 8);
	      size += extra;
	    }
	}
      else if (info->emitrelocations)
	stub_entry->group->stub_sec->reloc_count +=1;

      if (info->emitrelocations)
	{
	  unsigned int num_rel;
	  if (stub_entry->type.sub == ppc_stub_notoc)
	    num_rel = num_relocs_for_power10_offset (off, odd);
	  else
	    num_rel = num_relocs_for_offset (off - 8);
	  stub_entry->group->stub_sec->reloc_count += num_rel;
	  stub_entry->group->stub_sec->flags |= SEC_RELOC;
	}

      if (stub_entry->type.sub != ppc_stub_notoc)
	{
	  /* After the bcl, lr has been modified so we need to emit
	     .eh_frame info saying the return address is in r12.  */
	  lr_used = stub_offset + 8;
	  if (stub_entry->type.r2save)
	    lr_used += 4;
	  /* The eh_frame info will consist of a DW_CFA_advance_loc or
	     variant, DW_CFA_register, 65, 12, DW_CFA_advance_loc+2,
	     DW_CFA_restore_extended 65.  */
	  delta = lr_used - stub_entry->group->lr_restore;
	  stub_entry->group->eh_size += eh_advance_size (delta) + 6;
	  stub_entry->group->lr_restore = lr_used + 8;
	}
    }
  else if (stub_entry->type.sub >= ppc_stub_notoc)
    {
      BFD_ASSERT (stub_entry->type.main == ppc_stub_plt_call);
      lr_used = 0;
      if (stub_entry->h != NULL
	  && is_tls_get_addr (&stub_entry->h->elf, htab)
	  && htab->params->tls_get_addr_opt)
	{
	  lr_used += 7 * 4;
	  if (!htab->params->no_tls_get_addr_regsave)
	    lr_used += 11 * 4;
	  else if (stub_entry->type.r2save)
	    lr_used += 2 * 4;
	}
      if (stub_entry->type.r2save)
	lr_used += 4;
      targ = stub_entry->plt_ent->plt.offset & ~1;
      if (targ >= (bfd_vma) -2)
	abort ();

      plt = htab->elf.splt;
      if (use_local_plt (info, elf_hash_entry (stub_entry->h)))
	{
	  if (stub_entry->symtype == STT_GNU_IFUNC)
	    plt = htab->elf.iplt;
	  else
	    plt = htab->pltlocal;
	}
      targ += plt->output_offset + plt->output_section->vma;
      off = (stub_offset
	     + stub_entry->group->stub_sec->output_offset
	     + stub_entry->group->stub_sec->output_section->vma
	     + lr_used);
      odd = off & 4;
      off = targ - off;

      size = plt_stub_size (htab, stub_entry, off, odd);
      pad = plt_stub_pad (htab->params->plt_stub_align, stub_offset, size);
      if (pad != 0)
	{
	  stub_offset += pad;
	  off -= pad;
	  odd ^= pad & 4;
	  size = plt_stub_size (htab, stub_entry, off, odd);
	}

      if (info->emitrelocations)
	{
	  unsigned int num_rel;
	  if (stub_entry->type.sub == ppc_stub_notoc)
	    num_rel = num_relocs_for_power10_offset (off, odd);
	  else
	    num_rel = num_relocs_for_offset (off - 8);
	  stub_entry->group->stub_sec->reloc_count += num_rel;
	  stub_entry->group->stub_sec->flags |= SEC_RELOC;
	}

      if (stub_entry->type.sub != ppc_stub_notoc)
	{
	  /* After the bcl, lr has been modified so we need to emit
	     .eh_frame info saying the return address is in r12.  */
	  lr_used += stub_offset + 8;
	  /* The eh_frame info will consist of a DW_CFA_advance_loc or
	     variant, DW_CFA_register, 65, 12, DW_CFA_advance_loc+2,
	     DW_CFA_restore_extended 65.  */
	  delta = lr_used - stub_entry->group->lr_restore;
	  stub_entry->group->eh_size += eh_advance_size (delta) + 6;
	  stub_entry->group->lr_restore = lr_used + 8;
	}
      if (stub_entry->h != NULL
	  && is_tls_get_addr (&stub_entry->h->elf, htab)
	  && htab->params->tls_get_addr_opt)
	{
	  if (!htab->params->no_tls_get_addr_regsave)
	    {
	      unsigned int cfa_updt = stub_offset + 18 * 4;
	      delta = cfa_updt - stub_entry->group->lr_restore;
	      stub_entry->group->eh_size += eh_advance_size (delta);
	      stub_entry->group->eh_size += htab->opd_abi ? 36 : 35;
	      stub_entry->group->lr_restore = stub_offset + size - 4;
	    }
	  else if (stub_entry->type.r2save)
	    {
	      lr_used = stub_offset + size - 20;
	      delta = lr_used - stub_entry->group->lr_restore;
	      stub_entry->group->eh_size += eh_advance_size (delta) + 6;
	      stub_entry->group->lr_restore = stub_offset + size - 4;
	    }
	}
    }
  else if (stub_entry->type.main == ppc_stub_plt_call)
    {
      targ = stub_entry->plt_ent->plt.offset & ~(bfd_vma) 1;
      if (targ >= (bfd_vma) -2)
	abort ();
      plt = htab->elf.splt;
      if (use_local_plt (info, elf_hash_entry (stub_entry->h)))
	{
	  if (stub_entry->symtype == STT_GNU_IFUNC)
	    plt = htab->elf.iplt;
	  else
	    plt = htab->pltlocal;
	}
      targ += plt->output_offset + plt->output_section->vma;

      off = (elf_gp (info->output_bfd)
	     + htab->sec_info[stub_entry->group->link_sec->id].toc_off);
      off = targ - off;

      size = plt_stub_size (htab, stub_entry, off, 0);
      pad = plt_stub_pad (htab->params->plt_stub_align, stub_offset, size);
      stub_offset += pad;

      if (info->emitrelocations)
	{
	  stub_entry->group->stub_sec->reloc_count
	    += ((PPC_HA (off) != 0)
		+ (htab->opd_abi
		   ? 2 + (htab->params->plt_static_chain
			  && PPC_HA (off + 16) == PPC_HA (off))
		   : 1));
	  stub_entry->group->stub_sec->flags |= SEC_RELOC;
	}

      if (stub_entry->h != NULL
	  && is_tls_get_addr (&stub_entry->h->elf, htab)
	  && htab->params->tls_get_addr_opt
	  && stub_entry->type.r2save)
	{
	  if (!htab->params->no_tls_get_addr_regsave)
	    {
	      /* Adjustments to r1 need to be described.  */
	      unsigned int cfa_updt = stub_offset + 18 * 4;
	      delta = cfa_updt - stub_entry->group->lr_restore;
	      stub_entry->group->eh_size += eh_advance_size (delta);
	      stub_entry->group->eh_size += htab->opd_abi ? 36 : 35;
	    }
	  else
	    {
	      lr_used = stub_offset + size - 20;
	      /* The eh_frame info will consist of a DW_CFA_advance_loc
		 or variant, DW_CFA_offset_externed_sf, 65, -stackoff,
		 DW_CFA_advance_loc+4, DW_CFA_restore_extended, 65.  */
	      delta = lr_used - stub_entry->group->lr_restore;
	      stub_entry->group->eh_size += eh_advance_size (delta) + 6;
	    }
	  stub_entry->group->lr_restore = stub_offset + size - 4;
	}
    }
  else
    {
      BFD_FAIL ();
      return false;
    }

  if (stub_entry->stub_offset != stub_offset)
    htab->stub_changed = true;
  stub_entry->stub_offset = stub_offset;
  stub_entry->group->stub_sec->size = stub_offset + size;
  return true;
}

/* Set up various things so that we can make a list of input sections
   for each output section included in the link.  Returns -1 on error,
   0 when no stubs will be needed, and 1 on success.  */

int
ppc64_elf_setup_section_lists (struct bfd_link_info *info)
{
  unsigned int id;
  size_t amt;
  struct ppc_link_hash_table *htab = ppc_hash_table (info);

  if (htab == NULL)
    return -1;

  htab->sec_info_arr_size = _bfd_section_id;
  amt = sizeof (*htab->sec_info) * (htab->sec_info_arr_size);
  htab->sec_info = bfd_zmalloc (amt);
  if (htab->sec_info == NULL)
    return -1;

  /* Set toc_off for com, und, abs and ind sections.  */
  for (id = 0; id < 3; id++)
    htab->sec_info[id].toc_off = TOC_BASE_OFF;

  return 1;
}

/* Set up for first pass at multitoc partitioning.  */

void
ppc64_elf_start_multitoc_partition (struct bfd_link_info *info)
{
  struct ppc_link_hash_table *htab = ppc_hash_table (info);

  htab->toc_curr = ppc64_elf_set_toc (info, info->output_bfd);
  htab->toc_bfd = NULL;
  htab->toc_first_sec = NULL;
}

/* The linker repeatedly calls this function for each TOC input section
   and linker generated GOT section.  Group input bfds such that the toc
   within a group is less than 64k in size.  */

bool
ppc64_elf_next_toc_section (struct bfd_link_info *info, asection *isec)
{
  struct ppc_link_hash_table *htab = ppc_hash_table (info);
  bfd_vma addr, off, limit;

  if (htab == NULL)
    return false;

  if (!htab->second_toc_pass)
    {
      /* Keep track of the first .toc or .got section for this input bfd.  */
      bool new_bfd = htab->toc_bfd != isec->owner;

      if (new_bfd)
	{
	  htab->toc_bfd = isec->owner;
	  htab->toc_first_sec = isec;
	}

      addr = isec->output_offset + isec->output_section->vma;
      off = addr - htab->toc_curr;
      limit = 0x80008000;
      if (ppc64_elf_tdata (isec->owner)->has_small_toc_reloc)
	limit = 0x10000;
      if (off + isec->size > limit)
	{
	  addr = (htab->toc_first_sec->output_offset
		  + htab->toc_first_sec->output_section->vma);
	  htab->toc_curr = addr;
	  htab->toc_curr &= -TOC_BASE_ALIGN;
	}

      /* toc_curr is the base address of this toc group.  Set elf_gp
	 for the input section to be the offset relative to the
	 output toc base plus 0x8000.  Making the input elf_gp an
	 offset allows us to move the toc as a whole without
	 recalculating input elf_gp.  */
      off = htab->toc_curr - elf_gp (info->output_bfd);
      off += TOC_BASE_OFF;

      /* Die if someone uses a linker script that doesn't keep input
	 file .toc and .got together.  */
      if (new_bfd
	  && elf_gp (isec->owner) != 0
	  && elf_gp (isec->owner) != off)
	return false;

      elf_gp (isec->owner) = off;
      return true;
    }

  /* During the second pass toc_first_sec points to the start of
     a toc group, and toc_curr is used to track the old elf_gp.
     We use toc_bfd to ensure we only look at each bfd once.  */
  if (htab->toc_bfd == isec->owner)
    return true;
  htab->toc_bfd = isec->owner;

  if (htab->toc_first_sec == NULL
      || htab->toc_curr != elf_gp (isec->owner))
    {
      htab->toc_curr = elf_gp (isec->owner);
      htab->toc_first_sec = isec;
    }
  addr = (htab->toc_first_sec->output_offset
	  + htab->toc_first_sec->output_section->vma);
  off = addr - elf_gp (info->output_bfd) + TOC_BASE_OFF;
  elf_gp (isec->owner) = off;

  return true;
}

/* Called via elf_link_hash_traverse to merge GOT entries for global
   symbol H.  */

static bool
merge_global_got (struct elf_link_hash_entry *h, void *inf ATTRIBUTE_UNUSED)
{
  if (h->root.type == bfd_link_hash_indirect)
    return true;

  merge_got_entries (&h->got.glist);

  return true;
}

/* Called via elf_link_hash_traverse to allocate GOT entries for global
   symbol H.  */

static bool
reallocate_got (struct elf_link_hash_entry *h, void *inf)
{
  struct got_entry *gent;

  if (h->root.type == bfd_link_hash_indirect)
    return true;

  for (gent = h->got.glist; gent != NULL; gent = gent->next)
    if (!gent->is_indirect)
      allocate_got (h, (struct bfd_link_info *) inf, gent);
  return true;
}

/* Called on the first multitoc pass after the last call to
   ppc64_elf_next_toc_section.  This function removes duplicate GOT
   entries.  */

bool
ppc64_elf_layout_multitoc (struct bfd_link_info *info)
{
  struct ppc_link_hash_table *htab = ppc_hash_table (info);
  struct bfd *ibfd, *ibfd2;
  bool done_something;

  htab->multi_toc_needed = htab->toc_curr != elf_gp (info->output_bfd);

  if (!htab->do_multi_toc)
    return false;

  /* Merge global sym got entries within a toc group.  */
  elf_link_hash_traverse (&htab->elf, merge_global_got, info);

  /* And tlsld_got.  */
  for (ibfd = info->input_bfds; ibfd != NULL; ibfd = ibfd->link.next)
    {
      struct got_entry *ent, *ent2;

      if (!is_ppc64_elf (ibfd))
	continue;

      ent = ppc64_tlsld_got (ibfd);
      if (!ent->is_indirect
	  && ent->got.offset != (bfd_vma) -1)
	{
	  for (ibfd2 = ibfd->link.next; ibfd2 != NULL; ibfd2 = ibfd2->link.next)
	    {
	      if (!is_ppc64_elf (ibfd2))
		continue;

	      ent2 = ppc64_tlsld_got (ibfd2);
	      if (!ent2->is_indirect
		  && ent2->got.offset != (bfd_vma) -1
		  && elf_gp (ibfd2) == elf_gp (ibfd))
		{
		  ent2->is_indirect = true;
		  ent2->got.ent = ent;
		}
	    }
	}
    }

  /* Zap sizes of got sections.  */
  htab->elf.irelplt->rawsize = htab->elf.irelplt->size;
  htab->elf.irelplt->size -= htab->got_reli_size;
  htab->got_reli_size = 0;

  for (ibfd = info->input_bfds; ibfd != NULL; ibfd = ibfd->link.next)
    {
      asection *got, *relgot;

      if (!is_ppc64_elf (ibfd))
	continue;

      got = ppc64_elf_tdata (ibfd)->got;
      if (got != NULL)
	{
	  got->rawsize = got->size;
	  got->size = 0;
	  relgot = ppc64_elf_tdata (ibfd)->relgot;
	  relgot->rawsize = relgot->size;
	  relgot->size = 0;
	}
    }

  /* Now reallocate the got, local syms first.  We don't need to
     allocate section contents again since we never increase size.  */
  for (ibfd = info->input_bfds; ibfd != NULL; ibfd = ibfd->link.next)
    {
      struct got_entry **lgot_ents;
      struct got_entry **end_lgot_ents;
      struct plt_entry **local_plt;
      struct plt_entry **end_local_plt;
      unsigned char *lgot_masks;
      bfd_size_type locsymcount;
      Elf_Internal_Shdr *symtab_hdr;
      asection *s;
      Elf_Internal_Sym *local_syms;
      Elf_Internal_Sym *isym;

      if (!is_ppc64_elf (ibfd))
	continue;

      lgot_ents = elf_local_got_ents (ibfd);
      if (!lgot_ents)
	continue;

      symtab_hdr = &elf_symtab_hdr (ibfd);
      locsymcount = symtab_hdr->sh_info;
      end_lgot_ents = lgot_ents + locsymcount;
      local_plt = (struct plt_entry **) end_lgot_ents;
      end_local_plt = local_plt + locsymcount;
      lgot_masks = (unsigned char *) end_local_plt;
      local_syms = (Elf_Internal_Sym *) symtab_hdr->contents;
      if (local_syms == NULL && locsymcount != 0)
	{
	  local_syms = bfd_elf_get_elf_syms (ibfd, symtab_hdr, locsymcount,
					     0, NULL, NULL, NULL);
	  if (local_syms == NULL)
	    return false;
	}
      s = ppc64_elf_tdata (ibfd)->got;
      for (isym = local_syms;
	   lgot_ents < end_lgot_ents;
	   ++lgot_ents, ++lgot_masks, isym++)
	{
	  struct got_entry *ent;

	  for (ent = *lgot_ents; ent != NULL; ent = ent->next)
	    {
	      unsigned int ent_size = 8;
	      unsigned int rel_size = sizeof (Elf64_External_Rela);

	      ent->got.offset = s->size;
	      if ((ent->tls_type & *lgot_masks & TLS_GD) != 0)
		{
		  ent_size *= 2;
		  rel_size *= 2;
		}
	      s->size += ent_size;
	      if ((*lgot_masks & (TLS_TLS | PLT_IFUNC)) == PLT_IFUNC)
		{
		  htab->elf.irelplt->size += rel_size;
		  htab->got_reli_size += rel_size;
		}
	      else if (bfd_link_pic (info)
		       && (ent->tls_type == 0
			   ? !info->enable_dt_relr
			   : !bfd_link_executable (info))
		       && isym->st_shndx != SHN_ABS)
		{
		  asection *srel = ppc64_elf_tdata (ibfd)->relgot;
		  srel->size += rel_size;
		}
	    }
	}
    }

  elf_link_hash_traverse (&htab->elf, reallocate_got, info);

  for (ibfd = info->input_bfds; ibfd != NULL; ibfd = ibfd->link.next)
    {
      struct got_entry *ent;

      if (!is_ppc64_elf (ibfd))
	continue;

      ent = ppc64_tlsld_got (ibfd);
      if (!ent->is_indirect
	  && ent->got.offset != (bfd_vma) -1)
	{
	  asection *s = ppc64_elf_tdata (ibfd)->got;
	  ent->got.offset = s->size;
	  s->size += 16;
	  if (bfd_link_dll (info))
	    {
	      asection *srel = ppc64_elf_tdata (ibfd)->relgot;
	      srel->size += sizeof (Elf64_External_Rela);
	    }
	}
    }

  done_something = htab->elf.irelplt->rawsize != htab->elf.irelplt->size;
  if (!done_something)
    for (ibfd = info->input_bfds; ibfd != NULL; ibfd = ibfd->link.next)
      {
	asection *got;

	if (!is_ppc64_elf (ibfd))
	  continue;

	got = ppc64_elf_tdata (ibfd)->got;
	if (got != NULL)
	  {
	    done_something = got->rawsize != got->size;
	    if (done_something)
	      break;
	  }
      }

  if (done_something)
    (*htab->params->layout_sections_again) ();

  /* Set up for second pass over toc sections to recalculate elf_gp
     on input sections.  */
  htab->toc_bfd = NULL;
  htab->toc_first_sec = NULL;
  htab->second_toc_pass = true;
  return done_something;
}

/* Called after second pass of multitoc partitioning.  */

void
ppc64_elf_finish_multitoc_partition (struct bfd_link_info *info)
{
  struct ppc_link_hash_table *htab = ppc_hash_table (info);

  /* After the second pass, toc_curr tracks the TOC offset used
     for code sections below in ppc64_elf_next_input_section.  */
  htab->toc_curr = TOC_BASE_OFF;
}

/* No toc references were found in ISEC.  If the code in ISEC makes no
   calls, then there's no need to use toc adjusting stubs when branching
   into ISEC.  Actually, indirect calls from ISEC are OK as they will
   load r2.  Returns -1 on error, 0 for no stub needed, 1 for stub
   needed, and 2 if a cyclical call-graph was found but no other reason
   for a stub was detected.  If called from the top level, a return of
   2 means the same as a return of 0.  */

static int
toc_adjusting_stub_needed (struct bfd_link_info *info, asection *isec)
{
  int ret;

  /* Mark this section as checked.  */
  isec->call_check_done = 1;

  /* We know none of our code bearing sections will need toc stubs.  */
  if ((isec->flags & SEC_LINKER_CREATED) != 0)
    return 0;

  if (isec->size == 0)
    return 0;

  if (isec->output_section == NULL)
    return 0;

  ret = 0;
  if (isec->reloc_count != 0)
    {
      Elf_Internal_Rela *relstart, *rel;
      Elf_Internal_Sym *local_syms;
      struct ppc_link_hash_table *htab;

      relstart = _bfd_elf_link_read_relocs (isec->owner, isec, NULL, NULL,
					    info->keep_memory);
      if (relstart == NULL)
	return -1;

      /* Look for branches to outside of this section.  */
      local_syms = NULL;
      htab = ppc_hash_table (info);
      if (htab == NULL)
	return -1;

      for (rel = relstart; rel < relstart + isec->reloc_count; ++rel)
	{
	  enum elf_ppc64_reloc_type r_type;
	  unsigned long r_symndx;
	  struct elf_link_hash_entry *h;
	  struct ppc_link_hash_entry *eh;
	  Elf_Internal_Sym *sym;
	  asection *sym_sec;
	  struct _opd_sec_data *opd;
	  bfd_vma sym_value;
	  bfd_vma dest;

	  r_type = ELF64_R_TYPE (rel->r_info);
	  if (r_type != R_PPC64_REL24
	      && r_type != R_PPC64_REL24_NOTOC
	      && r_type != R_PPC64_REL24_P9NOTOC
	      && r_type != R_PPC64_REL14
	      && r_type != R_PPC64_REL14_BRTAKEN
	      && r_type != R_PPC64_REL14_BRNTAKEN
	      && r_type != R_PPC64_PLTCALL
	      && r_type != R_PPC64_PLTCALL_NOTOC)
	    continue;

	  r_symndx = ELF64_R_SYM (rel->r_info);
	  if (!get_sym_h (&h, &sym, &sym_sec, NULL, &local_syms, r_symndx,
			  isec->owner))
	    {
	      ret = -1;
	      break;
	    }

	  /* Calls to dynamic lib functions go through a plt call stub
	     that uses r2.  */
	  eh = ppc_elf_hash_entry (h);
	  if (eh != NULL
	      && (eh->elf.plt.plist != NULL
		  || (eh->oh != NULL
		      && ppc_follow_link (eh->oh)->elf.plt.plist != NULL)))
	    {
	      ret = 1;
	      break;
	    }

	  if (sym_sec == NULL)
	    /* Ignore other undefined symbols.  */
	    continue;

	  /* Assume branches to other sections not included in the
	     link need stubs too, to cover -R and absolute syms.  */
	  if (sym_sec->output_section == NULL)
	    {
	      ret = 1;
	      break;
	    }

	  if (h == NULL)
	    sym_value = sym->st_value;
	  else
	    {
	      if (h->root.type != bfd_link_hash_defined
		  && h->root.type != bfd_link_hash_defweak)
		abort ();
	      sym_value = h->root.u.def.value;
	    }
	  sym_value += rel->r_addend;

	  /* If this branch reloc uses an opd sym, find the code section.  */
	  opd = get_opd_info (sym_sec);
	  if (opd != NULL)
	    {
	      if (h == NULL && opd->adjust != NULL)
		{
		  long adjust;

		  adjust = opd->adjust[OPD_NDX (sym_value)];
		  if (adjust == -1)
		    /* Assume deleted functions won't ever be called.  */
		    continue;
		  sym_value += adjust;
		}

	      dest = opd_entry_value (sym_sec, sym_value,
				      &sym_sec, NULL, false);
	      if (dest == (bfd_vma) -1)
		continue;
	    }
	  else
	    dest = (sym_value
		    + sym_sec->output_offset
		    + sym_sec->output_section->vma);

	  /* Ignore branch to self.  */
	  if (sym_sec == isec)
	    continue;

	  /* If the called function uses the toc, we need a stub.  */
	  if (sym_sec->has_toc_reloc
	      || sym_sec->makes_toc_func_call)
	    {
	      ret = 1;
	      break;
	    }

	  /* Assume any branch that needs a long branch stub might in fact
	     need a plt_branch stub.  A plt_branch stub uses r2.  */
	  else if (dest - (isec->output_offset
			   + isec->output_section->vma
			   + rel->r_offset) + (1 << 25)
		   >= (2u << 25) - PPC64_LOCAL_ENTRY_OFFSET (h
							     ? h->other
							     : sym->st_other))
	    {
	      ret = 1;
	      break;
	    }

	  /* If calling back to a section in the process of being
	     tested, we can't say for sure that no toc adjusting stubs
	     are needed, so don't return zero.  */
	  else if (sym_sec->call_check_in_progress)
	    ret = 2;

	  /* Branches to another section that itself doesn't have any TOC
	     references are OK.  Recursively call ourselves to check.  */
	  else if (!sym_sec->call_check_done)
	    {
	      int recur;

	      /* Mark current section as indeterminate, so that other
		 sections that call back to current won't be marked as
		 known.  */
	      isec->call_check_in_progress = 1;
	      recur = toc_adjusting_stub_needed (info, sym_sec);
	      isec->call_check_in_progress = 0;

	      if (recur != 0)
		{
		  ret = recur;
		  if (recur != 2)
		    break;
		}
	    }
	}

      if (elf_symtab_hdr (isec->owner).contents
	  != (unsigned char *) local_syms)
	free (local_syms);
      if (elf_section_data (isec)->relocs != relstart)
	free (relstart);
    }

  if ((ret & 1) == 0
      && isec->map_head.s != NULL
      && (strcmp (isec->output_section->name, ".init") == 0
	  || strcmp (isec->output_section->name, ".fini") == 0))
    {
      if (isec->map_head.s->has_toc_reloc
	  || isec->map_head.s->makes_toc_func_call)
	ret = 1;
      else if (!isec->map_head.s->call_check_done)
	{
	  int recur;
	  isec->call_check_in_progress = 1;
	  recur = toc_adjusting_stub_needed (info, isec->map_head.s);
	  isec->call_check_in_progress = 0;
	  if (recur != 0)
	    ret = recur;
	}
    }

  if (ret == 1)
    isec->makes_toc_func_call = 1;

  return ret;
}

/* The linker repeatedly calls this function for each input section,
   in the order that input sections are linked into output sections.
   Build lists of input sections to determine groupings between which
   we may insert linker stubs.  */

bool
ppc64_elf_next_input_section (struct bfd_link_info *info, asection *isec)
{
  struct ppc_link_hash_table *htab = ppc_hash_table (info);

  if (htab == NULL)
    return false;

  if ((isec->output_section->flags & SEC_CODE) != 0
      && isec->output_section->id < htab->sec_info_arr_size)
    {
      /* This happens to make the list in reverse order,
	 which is what we want.  */
      htab->sec_info[isec->id].u.list
	= htab->sec_info[isec->output_section->id].u.list;
      htab->sec_info[isec->output_section->id].u.list = isec;
    }

  if (htab->multi_toc_needed)
    {
      /* Analyse sections that aren't already flagged as needing a
	 valid toc pointer.  Exclude .fixup for the linux kernel.
	 .fixup contains branches, but only back to the function that
	 hit an exception.  */
      if (!(isec->has_toc_reloc
	    || (isec->flags & SEC_CODE) == 0
	    || strcmp (isec->name, ".fixup") == 0
	    || isec->call_check_done))
	{
	  if (toc_adjusting_stub_needed (info, isec) < 0)
	    return false;
	}
      /* Make all sections use the TOC assigned for this object file.
	 This will be wrong for pasted sections;  We fix that in
	 check_pasted_section().  */
      if (elf_gp (isec->owner) != 0)
	htab->toc_curr = elf_gp (isec->owner);
    }

  htab->sec_info[isec->id].toc_off = htab->toc_curr;
  return true;
}

/* Check that all .init and .fini sections use the same toc, if they
   have toc relocs.  */

static bool
check_pasted_section (struct bfd_link_info *info, const char *name)
{
  asection *o = bfd_get_section_by_name (info->output_bfd, name);

  if (o != NULL)
    {
      struct ppc_link_hash_table *htab = ppc_hash_table (info);
      bfd_vma toc_off = 0;
      asection *i;

      for (i = o->map_head.s; i != NULL; i = i->map_head.s)
	if (i->has_toc_reloc)
	  {
	    if (toc_off == 0)
	      toc_off = htab->sec_info[i->id].toc_off;
	    else if (toc_off != htab->sec_info[i->id].toc_off)
	      return false;
	  }

      if (toc_off == 0)
	for (i = o->map_head.s; i != NULL; i = i->map_head.s)
	  if (i->makes_toc_func_call)
	    {
	      toc_off = htab->sec_info[i->id].toc_off;
	      break;
	    }

      /* Make sure the whole pasted function uses the same toc offset.  */
      if (toc_off != 0)
	for (i = o->map_head.s; i != NULL; i = i->map_head.s)
	  htab->sec_info[i->id].toc_off = toc_off;
    }
  return true;
}

bool
ppc64_elf_check_init_fini (struct bfd_link_info *info)
{
  bool ret1 = check_pasted_section (info, ".init");
  bool ret2 = check_pasted_section (info, ".fini");

  return ret1 && ret2;
}

/* See whether we can group stub sections together.  Grouping stub
   sections may result in fewer stubs.  More importantly, we need to
   put all .init* and .fini* stubs at the beginning of the .init or
   .fini output sections respectively, because glibc splits the
   _init and _fini functions into multiple parts.  Putting a stub in
   the middle of a function is not a good idea.  */

static bool
group_sections (struct bfd_link_info *info,
		bfd_size_type stub_group_size,
		bool stubs_always_before_branch)
{
  struct ppc_link_hash_table *htab;
  asection *osec;
  bool suppress_size_errors;

  htab = ppc_hash_table (info);
  if (htab == NULL)
    return false;

  suppress_size_errors = false;
  if (stub_group_size == 1)
    {
      /* Default values.  */
      if (stubs_always_before_branch)
	stub_group_size = 0x1e00000;
      else
	stub_group_size = 0x1c00000;
      suppress_size_errors = true;
    }

  for (osec = info->output_bfd->sections; osec != NULL; osec = osec->next)
    {
      asection *tail;

      if (osec->id >= htab->sec_info_arr_size)
	continue;

      tail = htab->sec_info[osec->id].u.list;
      while (tail != NULL)
	{
	  asection *curr;
	  asection *prev;
	  bfd_size_type total;
	  bool big_sec;
	  bfd_vma curr_toc;
	  struct map_stub *group;
	  bfd_size_type group_size;

	  curr = tail;
	  total = tail->size;
	  group_size = (ppc64_elf_section_data (tail) != NULL
			&& ppc64_elf_section_data (tail)->has_14bit_branch
			? stub_group_size >> 10 : stub_group_size);

	  big_sec = total > group_size;
	  if (big_sec && !suppress_size_errors)
	    /* xgettext:c-format */
	    _bfd_error_handler (_("%pB section %pA exceeds stub group size"),
				tail->owner, tail);
	  curr_toc = htab->sec_info[tail->id].toc_off;

	  while ((prev = htab->sec_info[curr->id].u.list) != NULL
		 && ((total += curr->output_offset - prev->output_offset)
		     < (ppc64_elf_section_data (prev) != NULL
			&& ppc64_elf_section_data (prev)->has_14bit_branch
			? (group_size = stub_group_size >> 10) : group_size))
		 && htab->sec_info[prev->id].toc_off == curr_toc)
	    curr = prev;

	  /* OK, the size from the start of CURR to the end is less
	     than group_size and thus can be handled by one stub
	     section.  (or the tail section is itself larger than
	     group_size, in which case we may be toast.)  We should
	     really be keeping track of the total size of stubs added
	     here, as stubs contribute to the final output section
	     size.  That's a little tricky, and this way will only
	     break if stubs added make the total size more than 2^25,
	     ie. for the default stub_group_size, if stubs total more
	     than 2097152 bytes, or nearly 75000 plt call stubs.  */
	  group = bfd_alloc (curr->owner, sizeof (*group));
	  if (group == NULL)
	    return false;
	  group->link_sec = curr;
	  group->stub_sec = NULL;
	  group->needs_save_res = 0;
	  group->lr_restore = 0;
	  group->eh_size = 0;
	  group->eh_base = 0;
	  group->next = htab->group;
	  htab->group = group;
	  do
	    {
	      prev = htab->sec_info[tail->id].u.list;
	      /* Set up this stub group.  */
	      htab->sec_info[tail->id].u.group = group;
	    }
	  while (tail != curr && (tail = prev) != NULL);

	  /* But wait, there's more!  Input sections up to group_size
	     bytes before the stub section can be handled by it too.
	     Don't do this if we have a really large section after the
	     stubs, as adding more stubs increases the chance that
	     branches may not reach into the stub section.  */
	  if (!stubs_always_before_branch && !big_sec)
	    {
	      total = 0;
	      while (prev != NULL
		     && ((total += tail->output_offset - prev->output_offset)
			 < (ppc64_elf_section_data (prev) != NULL
			    && ppc64_elf_section_data (prev)->has_14bit_branch
			    ? (group_size = stub_group_size >> 10)
			    : group_size))
		     && htab->sec_info[prev->id].toc_off == curr_toc)
		{
		  tail = prev;
		  prev = htab->sec_info[tail->id].u.list;
		  htab->sec_info[tail->id].u.group = group;
		}
	    }
	  tail = prev;
	}
    }
  return true;
}

static const unsigned char glink_eh_frame_cie[] =
{
  0, 0, 0, 16,				/* length.  */
  0, 0, 0, 0,				/* id.  */
  1,					/* CIE version.  */
  'z', 'R', 0,				/* Augmentation string.  */
  4,					/* Code alignment.  */
  0x78,					/* Data alignment.  */
  65,					/* RA reg.  */
  1,					/* Augmentation size.  */
  DW_EH_PE_pcrel | DW_EH_PE_sdata4,	/* FDE encoding.  */
  DW_CFA_def_cfa, 1, 0			/* def_cfa: r1 offset 0.  */
};

/* Stripping output sections is normally done before dynamic section
   symbols have been allocated.  This function is called later, and
   handles cases like htab->brlt which is mapped to its own output
   section.  */

static void
maybe_strip_output (struct bfd_link_info *info, asection *isec)
{
  if (isec->size == 0
      && isec->output_section->size == 0
      && !(isec->output_section->flags & SEC_KEEP)
      && !bfd_section_removed_from_list (info->output_bfd,
					 isec->output_section)
      && elf_section_data (isec->output_section)->dynindx == 0)
    {
      isec->output_section->flags |= SEC_EXCLUDE;
      bfd_section_list_remove (info->output_bfd, isec->output_section);
      info->output_bfd->section_count--;
    }
}

/* Stash R_PPC64_RELATIVE reloc at input section SEC, r_offset OFF to
   the array of such relocs.  */

static bool
append_relr_off (struct ppc_link_hash_table *htab, asection *sec, bfd_vma off)
{
  if (htab->relr_count >= htab->relr_alloc)
    {
      if (htab->relr_alloc == 0)
	htab->relr_alloc = 4096;
      else
	htab->relr_alloc *= 2;
      htab->relr = bfd_realloc (htab->relr,
				htab->relr_alloc * sizeof (*htab->relr));
      if (htab->relr == NULL)
	return false;
    }
  htab->relr[htab->relr_count].sec = sec;
  htab->relr[htab->relr_count].off = off;
  htab->relr_count++;
  return true;
}

/* qsort comparator for bfd_vma args.  */

static int
compare_relr_address (const void *arg1, const void *arg2)
{
  bfd_vma a = *(bfd_vma *) arg1;
  bfd_vma b = *(bfd_vma *) arg2;
  return a < b ? -1 : a > b ? 1 : 0;
}

/* Produce a malloc'd sorted array of reloc addresses from the info
   stored by append_relr_off.  */

static bfd_vma *
sort_relr (struct ppc_link_hash_table *htab)
{
  bfd_vma *addr = bfd_malloc (htab->relr_count * sizeof (*addr));
  if (addr == NULL)
    return NULL;

  for (size_t i = 0; i < htab->relr_count; i++)
    addr[i] = (htab->relr[i].sec->output_section->vma
	       + htab->relr[i].sec->output_offset
	       + htab->relr[i].off);

  if (htab->relr_count > 1)
    qsort (addr, htab->relr_count, sizeof (*addr), compare_relr_address);

  return addr;
}

/* Look over GOT and PLT entries saved on elf_local_got_ents for all
   input files, stashing info about needed relative relocs.  */

static bool
got_and_plt_relr_for_local_syms (struct bfd_link_info *info)
{
  struct ppc_link_hash_table *htab = ppc_hash_table (info);
  bfd *ibfd;

  for (ibfd = info->input_bfds; ibfd != NULL; ibfd = ibfd->link.next)
    {
      struct got_entry **lgot_ents, **lgot, **end_lgot_ents;
      struct plt_entry **local_plt, **lplt, **end_local_plt;
      Elf_Internal_Shdr *symtab_hdr;
      bfd_size_type locsymcount;
      Elf_Internal_Sym *local_syms;
      Elf_Internal_Sym *isym;
      struct plt_entry *pent;
      struct got_entry *gent;

      if (!is_ppc64_elf (ibfd))
	continue;

      lgot_ents = elf_local_got_ents (ibfd);
      if (!lgot_ents)
	continue;

      symtab_hdr = &elf_symtab_hdr (ibfd);
      locsymcount = symtab_hdr->sh_info;
      local_syms = (Elf_Internal_Sym *) symtab_hdr->contents;
      if (local_syms == NULL && locsymcount != 0)
	{
	  local_syms = bfd_elf_get_elf_syms (ibfd, symtab_hdr, locsymcount,
					     0, NULL, NULL, NULL);
	  if (local_syms == NULL)
	    return false;
	}
      end_lgot_ents = lgot_ents + locsymcount;
      local_plt = (struct plt_entry **) end_lgot_ents;
      end_local_plt = local_plt + locsymcount;
      for (lgot = lgot_ents, isym = local_syms;
	   lgot < end_lgot_ents;
	   ++lgot, ++isym)
	for (gent = *lgot; gent != NULL; gent = gent->next)
	  if (!gent->is_indirect
	      && gent->tls_type == 0
	      && gent->got.offset != (bfd_vma) -1
	      && isym->st_shndx != SHN_ABS)
	    {
	      asection *got = ppc64_elf_tdata (gent->owner)->got;
	      if (!append_relr_off (htab, got, gent->got.offset))
		{
		  htab->stub_error = true;
		  return false;
		}
	    }

      if (!htab->opd_abi)
	for (lplt = local_plt, isym = local_syms;
	     lplt < end_local_plt;
	     ++lplt, ++isym)
	  for (pent = *lplt; pent != NULL; pent = pent->next)
	    if (pent->plt.offset != (bfd_vma) -1
		&& ELF_ST_TYPE (isym->st_info) != STT_GNU_IFUNC)
	      {
		if (!append_relr_off (htab, htab->pltlocal, pent->plt.offset))
		  {
		    if (symtab_hdr->contents != (unsigned char *) local_syms)
		      free (local_syms);
		    return false;
		  }
	      }

      if (local_syms != NULL
	  && symtab_hdr->contents != (unsigned char *) local_syms)
	{
	  if (!info->keep_memory)
	    free (local_syms);
	  else
	    symtab_hdr->contents = (unsigned char *) local_syms;
	}
    }
  return true;
}

/* Stash info about needed GOT and PLT entry relative relocs for
   global symbol H.  */

static bool
got_and_plt_relr (struct elf_link_hash_entry *h, void *inf)
{
  struct bfd_link_info *info;
  struct ppc_link_hash_table *htab;
  struct plt_entry *pent;
  struct got_entry *gent;

  if (h->root.type == bfd_link_hash_indirect)
    return true;

  info = (struct bfd_link_info *) inf;
  htab = ppc_hash_table (info);
  if (htab == NULL)
    return false;

  if (h->type != STT_GNU_IFUNC
      && h->def_regular
      && (h->root.type == bfd_link_hash_defined
	  || h->root.type == bfd_link_hash_defweak))
    {
      if ((!htab->elf.dynamic_sections_created
	   || h->dynindx == -1
	   || SYMBOL_REFERENCES_LOCAL (info, h))
	  && !bfd_is_abs_symbol (&h->root))
	for (gent = h->got.glist; gent != NULL; gent = gent->next)
	  if (!gent->is_indirect
	      && gent->tls_type == 0
	      && gent->got.offset != (bfd_vma) -1)
	    {
	      asection *got = ppc64_elf_tdata (gent->owner)->got;
	      if (!append_relr_off (htab, got, gent->got.offset))
		{
		  htab->stub_error = true;
		  return false;
		}
	    }

      if (!htab->opd_abi
	  && use_local_plt (info, h))
	for (pent = h->plt.plist; pent != NULL; pent = pent->next)
	  if (pent->plt.offset != (bfd_vma) -1)
	    {
	      if (!append_relr_off (htab, htab->pltlocal, pent->plt.offset))
		{
		  htab->stub_error = true;
		  return false;
		}
	    }
    }
  return true;
}

/* Determine and set the size of the stub section for a final link.

   The basic idea here is to examine all the relocations looking for
   PC-relative calls to a target that is unreachable with a "bl"
   instruction.  */

bool
ppc64_elf_size_stubs (struct bfd_link_info *info)
{
  bfd_size_type stub_group_size;
  bool stubs_always_before_branch;
  struct ppc_link_hash_table *htab = ppc_hash_table (info);

  if (htab == NULL)
    return false;

  if (htab->params->power10_stubs == -1 && !htab->has_power10_relocs)
    htab->params->power10_stubs = 0;

  if (htab->params->plt_thread_safe == -1 && !bfd_link_executable (info))
    htab->params->plt_thread_safe = 1;
  if (!htab->opd_abi)
    htab->params->plt_thread_safe = 0;
  else if (htab->params->plt_thread_safe == -1)
    {
      static const char *const thread_starter[] =
	{
	  "pthread_create",
	  /* libstdc++ */
	  "_ZNSt6thread15_M_start_threadESt10shared_ptrINS_10_Impl_baseEE",
	  /* librt */
	  "aio_init", "aio_read", "aio_write", "aio_fsync", "lio_listio",
	  "mq_notify", "create_timer",
	  /* libanl */
	  "getaddrinfo_a",
	  /* libgomp */
	  "GOMP_parallel",
	  "GOMP_parallel_start",
	  "GOMP_parallel_loop_static",
	  "GOMP_parallel_loop_static_start",
	  "GOMP_parallel_loop_dynamic",
	  "GOMP_parallel_loop_dynamic_start",
	  "GOMP_parallel_loop_guided",
	  "GOMP_parallel_loop_guided_start",
	  "GOMP_parallel_loop_runtime",
	  "GOMP_parallel_loop_runtime_start",
	  "GOMP_parallel_sections",
	  "GOMP_parallel_sections_start",
	  /* libgo */
	  "__go_go",
	};
      unsigned i;

      for (i = 0; i < ARRAY_SIZE (thread_starter); i++)
	{
	  struct elf_link_hash_entry *h;
	  h = elf_link_hash_lookup (&htab->elf, thread_starter[i],
				    false, false, true);
	  htab->params->plt_thread_safe = h != NULL && h->ref_regular;
	  if (htab->params->plt_thread_safe)
	    break;
	}
    }
  stubs_always_before_branch = htab->params->group_size < 0;
  if (htab->params->group_size < 0)
    stub_group_size = -htab->params->group_size;
  else
    stub_group_size = htab->params->group_size;

  if (!group_sections (info, stub_group_size, stubs_always_before_branch))
    return false;

  htab->tga_group = NULL;
  if (!htab->params->no_tls_get_addr_regsave
      && htab->tga_desc_fd != NULL
      && (htab->tga_desc_fd->elf.root.type == bfd_link_hash_undefined
	  || htab->tga_desc_fd->elf.root.type == bfd_link_hash_undefweak)
      && htab->tls_get_addr_fd != NULL
      && is_static_defined (&htab->tls_get_addr_fd->elf))
    {
      asection *sym_sec, *code_sec, *stub_sec;
      bfd_vma sym_value;
      struct _opd_sec_data *opd;

      sym_sec = htab->tls_get_addr_fd->elf.root.u.def.section;
      sym_value = defined_sym_val (&htab->tls_get_addr_fd->elf);
      code_sec = sym_sec;
      opd = get_opd_info (sym_sec);
      if (opd != NULL)
	opd_entry_value (sym_sec, sym_value, &code_sec, NULL, false);
      htab->tga_group = htab->sec_info[code_sec->id].u.group;
      stub_sec = (*htab->params->add_stub_section) (".tga_desc.stub",
						    htab->tga_group->link_sec);
      if (stub_sec == NULL)
	return false;
      htab->tga_group->stub_sec = stub_sec;

      htab->tga_desc_fd->elf.root.type = bfd_link_hash_defined;
      htab->tga_desc_fd->elf.root.u.def.section = stub_sec;
      htab->tga_desc_fd->elf.root.u.def.value = 0;
      htab->tga_desc_fd->elf.type = STT_FUNC;
      htab->tga_desc_fd->elf.def_regular = 1;
      htab->tga_desc_fd->elf.non_elf = 0;
      _bfd_elf_link_hash_hide_symbol (info, &htab->tga_desc_fd->elf, true);
    }

  /* Loop until no stubs added.  After iteration 20 of this loop we may
     exit on a stub section shrinking.  */

  while (1)
    {
      bfd *input_bfd;
      unsigned int bfd_indx;
      struct map_stub *group;

      htab->stub_iteration += 1;
      htab->relr_count = 0;

      for (input_bfd = info->input_bfds, bfd_indx = 0;
	   input_bfd != NULL;
	   input_bfd = input_bfd->link.next, bfd_indx++)
	{
	  Elf_Internal_Shdr *symtab_hdr;
	  asection *section;
	  Elf_Internal_Sym *local_syms = NULL;

	  if (!is_ppc64_elf (input_bfd))
	    continue;

	  /* We'll need the symbol table in a second.  */
	  symtab_hdr = &elf_symtab_hdr (input_bfd);
	  if (symtab_hdr->sh_info == 0)
	    continue;

	  /* Walk over each section attached to the input bfd.  */
	  for (section = input_bfd->sections;
	       section != NULL;
	       section = section->next)
	    {
	      Elf_Internal_Rela *internal_relocs, *irelaend, *irela;
	      bool is_opd;

	      /* If there aren't any relocs, then there's nothing more
		 to do.  */
	      if ((section->flags & SEC_RELOC) == 0
		  || (section->flags & SEC_ALLOC) == 0
		  || (section->flags & SEC_LOAD) == 0
		  || section->reloc_count == 0)
		continue;

	      if (!info->enable_dt_relr
		  && (section->flags & SEC_CODE) == 0)
		continue;

	      /* If this section is a link-once section that will be
		 discarded, then don't create any stubs.  */
	      if (section->output_section == NULL
		  || section->output_section->owner != info->output_bfd)
		continue;

	      /* Get the relocs.  */
	      internal_relocs
		= _bfd_elf_link_read_relocs (input_bfd, section, NULL, NULL,
					     info->keep_memory);
	      if (internal_relocs == NULL)
		goto error_ret_free_local;

	      is_opd = ppc64_elf_section_data (section)->sec_type == sec_opd;

	      /* Now examine each relocation.  */
	      irela = internal_relocs;
	      irelaend = irela + section->reloc_count;
	      for (; irela < irelaend; irela++)
		{
		  enum elf_ppc64_reloc_type r_type;
		  unsigned int r_indx;
		  struct ppc_stub_type stub_type;
		  struct ppc_stub_hash_entry *stub_entry;
		  asection *sym_sec, *code_sec;
		  bfd_vma sym_value, code_value;
		  bfd_vma destination;
		  unsigned long local_off;
		  bool ok_dest;
		  struct ppc_link_hash_entry *hash;
		  struct ppc_link_hash_entry *fdh;
		  struct elf_link_hash_entry *h;
		  Elf_Internal_Sym *sym;
		  char *stub_name;
		  const asection *id_sec;
		  struct _opd_sec_data *opd;
		  struct plt_entry *plt_ent;

		  r_type = ELF64_R_TYPE (irela->r_info);
		  r_indx = ELF64_R_SYM (irela->r_info);

		  if (r_type >= R_PPC64_max)
		    {
		      bfd_set_error (bfd_error_bad_value);
		      goto error_ret_free_internal;
		    }

		  /* Only look for stubs on branch instructions.  */
		  switch (r_type)
		    {
		    default:
		      continue;

		    case R_PPC64_REL24:
		    case R_PPC64_REL24_NOTOC:
		    case R_PPC64_REL24_P9NOTOC:
		    case R_PPC64_REL14:
		    case R_PPC64_REL14_BRTAKEN:
		    case R_PPC64_REL14_BRNTAKEN:
		      if ((section->flags & SEC_CODE) != 0)
			break;
		      continue;

		    case R_PPC64_ADDR64:
		    case R_PPC64_TOC:
		      if (info->enable_dt_relr
			  && irela->r_offset % 2 == 0
			  && section->alignment_power != 0)
			break;
		      continue;
		    }

		  /* Now determine the call target, its name, value,
		     section.  */
		  if (!get_sym_h (&h, &sym, &sym_sec, NULL, &local_syms,
				  r_indx, input_bfd))
		    goto error_ret_free_internal;

		  if (r_type == R_PPC64_ADDR64 || r_type == R_PPC64_TOC)
		    {
		      /* Only locally defined symbols can possibly use
			 relative relocations.  */
		      bfd_vma r_offset;
		      if ((sym_sec == NULL
			   || sym_sec->output_section == NULL)
			  /* No symbol is OK too.  */
			  && !(sym != NULL && sym->st_shndx == 0)
			  /* Hack for __ehdr_start, which is undefined
			     at this point.  */
			  && !(h != NULL && h->root.linker_def))
			continue;
		      if (NO_OPD_RELOCS && is_opd)
			continue;
		      if (!is_opd
			  && r_type == R_PPC64_ADDR64)
			{
			  if (h != NULL
			      ? h->type == STT_GNU_IFUNC
			      : ELF_ST_TYPE (sym->st_info) == STT_GNU_IFUNC)
			    continue;
			  if (h != NULL
			      ? bfd_is_abs_symbol (&h->root)
			      : sym->st_shndx == SHN_ABS)
			    continue;
			  if (h != NULL
			      && !SYMBOL_REFERENCES_LOCAL (info, h))
			    continue;
			}
		      r_offset = _bfd_elf_section_offset (info->output_bfd,
							  info,
							  section,
							  irela->r_offset);
		      if (r_offset >= (bfd_vma) -2)
			continue;
		      if (!append_relr_off (htab, section, r_offset))
			goto error_ret_free_internal;
		      continue;
		    }

		  hash = ppc_elf_hash_entry (h);
		  ok_dest = false;
		  fdh = NULL;
		  sym_value = 0;
		  if (hash == NULL)
		    {
		      sym_value = sym->st_value;
		      if (sym_sec != NULL
			  && sym_sec->output_section != NULL)
			ok_dest = true;
		    }
		  else if (hash->elf.root.type == bfd_link_hash_defined
			   || hash->elf.root.type == bfd_link_hash_defweak)
		    {
		      sym_value = hash->elf.root.u.def.value;
		      if (sym_sec->output_section != NULL)
			ok_dest = true;
		    }
		  else if (hash->elf.root.type == bfd_link_hash_undefweak
			   || hash->elf.root.type == bfd_link_hash_undefined)
		    {
		      /* Recognise an old ABI func code entry sym, and
			 use the func descriptor sym instead if it is
			 defined.  */
		      if (hash->elf.root.root.string[0] == '.'
			  && hash->oh != NULL)
			{
			  fdh = ppc_follow_link (hash->oh);
			  if (fdh->elf.root.type == bfd_link_hash_defined
			      || fdh->elf.root.type == bfd_link_hash_defweak)
			    {
			      sym_sec = fdh->elf.root.u.def.section;
			      sym_value = fdh->elf.root.u.def.value;
			      if (sym_sec->output_section != NULL)
				ok_dest = true;
			    }
			  else
			    fdh = NULL;
			}
		    }
		  else
		    {
		      bfd_set_error (bfd_error_bad_value);
		      goto error_ret_free_internal;
		    }

		  destination = 0;
		  local_off = 0;
		  if (ok_dest)
		    {
		      sym_value += irela->r_addend;
		      destination = (sym_value
				     + sym_sec->output_offset
				     + sym_sec->output_section->vma);
		      local_off = PPC64_LOCAL_ENTRY_OFFSET (hash
							    ? hash->elf.other
							    : sym->st_other);
		    }

		  code_sec = sym_sec;
		  code_value = sym_value;
		  opd = get_opd_info (sym_sec);
		  if (opd != NULL)
		    {
		      bfd_vma dest;

		      if (hash == NULL && opd->adjust != NULL)
			{
			  long adjust = opd->adjust[OPD_NDX (sym_value)];
			  if (adjust == -1)
			    continue;
			  code_value += adjust;
			  sym_value += adjust;
			}
		      dest = opd_entry_value (sym_sec, sym_value,
					      &code_sec, &code_value, false);
		      if (dest != (bfd_vma) -1)
			{
			  destination = dest;
			  if (fdh != NULL)
			    {
			      /* Fixup old ABI sym to point at code
				 entry.  */
			      hash->elf.root.type = bfd_link_hash_defweak;
			      hash->elf.root.u.def.section = code_sec;
			      hash->elf.root.u.def.value = code_value;
			    }
			}
		    }

		  /* Determine what (if any) linker stub is needed.  */
		  plt_ent = NULL;
		  stub_type.main = ppc_type_of_stub (section, irela, &hash,
						     &plt_ent, destination,
						     local_off);
		  stub_type.sub = ppc_stub_toc;
		  stub_type.r2save = 0;

		  if (r_type == R_PPC64_REL24_NOTOC
		      || r_type == R_PPC64_REL24_P9NOTOC)
		    {
		      enum ppc_stub_sub_type notoc = ppc_stub_notoc;
		      if (htab->params->power10_stubs == 0
			  || (r_type == R_PPC64_REL24_P9NOTOC
			      && htab->params->power10_stubs != 1))
			notoc = ppc_stub_p9notoc;
		      if (stub_type.main == ppc_stub_plt_call)
			stub_type.sub = notoc;
		      else if (stub_type.main == ppc_stub_long_branch
			       || (code_sec != NULL
				   && code_sec->output_section != NULL
				   && (((hash ? hash->elf.other : sym->st_other)
					& STO_PPC64_LOCAL_MASK)
				       > 1 << STO_PPC64_LOCAL_BIT)))
			{
			  stub_type.main = ppc_stub_long_branch;
			  stub_type.sub = notoc;
			  stub_type.r2save = 0;
			}
		    }
		  else if (stub_type.main != ppc_stub_plt_call)
		    {
		      /* Check whether we need a TOC adjusting stub.
			 Since the linker pastes together pieces from
			 different object files when creating the
			 _init and _fini functions, it may be that a
			 call to what looks like a local sym is in
			 fact a call needing a TOC adjustment.  */
		      if ((code_sec != NULL
			   && code_sec->output_section != NULL
			   && (code_sec->has_toc_reloc
			       || code_sec->makes_toc_func_call)
			   && (htab->sec_info[code_sec->id].toc_off
			       != htab->sec_info[section->id].toc_off))
			  || (((hash ? hash->elf.other : sym->st_other)
			       & STO_PPC64_LOCAL_MASK)
			      == 1 << STO_PPC64_LOCAL_BIT))
			{
			  stub_type.main = ppc_stub_long_branch;
			  stub_type.sub = ppc_stub_toc;
			  stub_type.r2save = 1;
			}
		    }

		  if (stub_type.main == ppc_stub_none)
		    continue;

		  /* __tls_get_addr calls might be eliminated.  */
		  if (stub_type.main != ppc_stub_plt_call
		      && hash != NULL
		      && is_tls_get_addr (&hash->elf, htab)
		      && section->has_tls_reloc
		      && irela != internal_relocs)
		    {
		      /* Get tls info.  */
		      unsigned char *tls_mask;

		      if (!get_tls_mask (&tls_mask, NULL, NULL, &local_syms,
					 irela - 1, input_bfd))
			goto error_ret_free_internal;
		      if ((*tls_mask & TLS_TLS) != 0
			  && (*tls_mask & (TLS_GD | TLS_LD)) == 0)
			continue;
		    }

		  if (stub_type.main == ppc_stub_plt_call
		      && stub_type.sub == ppc_stub_toc)
		    {
		      if (!htab->opd_abi
			  && htab->params->plt_localentry0 != 0
			  && is_elfv2_localentry0 (&hash->elf))
			htab->has_plt_localentry0 = 1;
		      else if (irela + 1 < irelaend
			       && irela[1].r_offset == irela->r_offset + 4
			       && (ELF64_R_TYPE (irela[1].r_info)
				   == R_PPC64_TOCSAVE))
			{
			  if (!tocsave_find (htab, INSERT,
					     &local_syms, irela + 1, input_bfd))
			    goto error_ret_free_internal;
			}
		      else
			stub_type.r2save = 1;
		    }

		  /* Support for grouping stub sections.  */
		  id_sec = htab->sec_info[section->id].u.group->link_sec;

		  /* Get the name of this stub.  */
		  stub_name = ppc_stub_name (id_sec, sym_sec, hash, irela);
		  if (!stub_name)
		    goto error_ret_free_internal;

		  stub_entry = ppc_stub_hash_lookup (&htab->stub_hash_table,
						     stub_name, false, false);
		  if (stub_entry != NULL)
		    {
		      free (stub_name);
		      if (!ppc_merge_stub (htab, stub_entry, stub_type, r_type))
			{
			  /* xgettext:c-format */
			  _bfd_error_handler
			    (_("%pB: cannot create stub entry %s"),
			     section->owner, stub_entry->root.string);
			  goto error_ret_free_internal;
			}
		      continue;
		    }

		  stub_entry = ppc_add_stub (stub_name, section, info);
		  if (stub_entry == NULL)
		    {
		      free (stub_name);
		    error_ret_free_internal:
		      if (elf_section_data (section)->relocs == NULL)
			free (internal_relocs);
		    error_ret_free_local:
		      if (symtab_hdr->contents
			  != (unsigned char *) local_syms)
			free (local_syms);
		      return false;
		    }

		  stub_entry->type = stub_type;
		  if (stub_type.main == ppc_stub_plt_call)
		    {
		      stub_entry->target_value = sym_value;
		      stub_entry->target_section = sym_sec;
		    }
		  else
		    {
		      stub_entry->target_value = code_value;
		      stub_entry->target_section = code_sec;
		    }
		  stub_entry->h = hash;
		  stub_entry->plt_ent = plt_ent;
		  stub_entry->symtype
		    = hash ? hash->elf.type : ELF_ST_TYPE (sym->st_info);
		  stub_entry->other = hash ? hash->elf.other : sym->st_other;

		  if (hash != NULL
		      && (hash->elf.root.type == bfd_link_hash_defined
			  || hash->elf.root.type == bfd_link_hash_defweak))
		    htab->stub_globals += 1;
		}

	      /* We're done with the internal relocs, free them.  */
	      if (elf_section_data (section)->relocs != internal_relocs)
		free (internal_relocs);
	    }

	  if (local_syms != NULL
	      && symtab_hdr->contents != (unsigned char *) local_syms)
	    {
	      if (!info->keep_memory)
		free (local_syms);
	      else
		symtab_hdr->contents = (unsigned char *) local_syms;
	    }
	}

      /* We may have added some stubs.  Find out the new size of the
	 stub sections.  */
      for (group = htab->group; group != NULL; group = group->next)
	{
	  group->lr_restore = 0;
	  group->eh_size = 0;
	  if (group->stub_sec != NULL)
	    {
	      asection *stub_sec = group->stub_sec;

	      stub_sec->rawsize = stub_sec->size;
	      stub_sec->size = 0;
	      stub_sec->reloc_count = 0;
	      stub_sec->flags &= ~SEC_RELOC;
	    }
	}
      if (htab->tga_group != NULL)
	{
	  /* See emit_tga_desc and emit_tga_desc_eh_frame.  */
	  htab->tga_group->eh_size
	    = 1 + 2 + (htab->opd_abi != 0) + 3 + 8 * 2 + 3 + 8 + 3;
	  htab->tga_group->lr_restore = 23 * 4;
	  htab->tga_group->stub_sec->size = 24 * 4;
	}

      htab->brlt->rawsize = htab->brlt->size;
      htab->brlt->size = 0;
      htab->brlt->reloc_count = 0;
      htab->brlt->flags &= ~SEC_RELOC;
      if (htab->relbrlt != NULL)
	htab->relbrlt->size = 0;

      if (htab->elf.srelrdyn != NULL)
	{
	  htab->elf.srelrdyn->rawsize = htab->elf.srelrdyn->size;
	  htab->elf.srelrdyn->size = 0;
	}

      htab->stub_changed = false;
      htab->stub_id = 0;
      bfd_hash_traverse (&htab->stub_hash_table, ppc_size_one_stub, info);

      for (group = htab->group; group != NULL; group = group->next)
	if (group->needs_save_res)
	  group->stub_sec->size += htab->sfpr->size;

      if (info->emitrelocations
	  && htab->glink != NULL && htab->glink->size != 0)
	{
	  htab->glink->reloc_count = 1;
	  htab->glink->flags |= SEC_RELOC;
	}

      if (htab->glink_eh_frame != NULL
	  && !bfd_is_abs_section (htab->glink_eh_frame->output_section)
	  && htab->glink_eh_frame->output_section->size > 8)
	{
	  size_t size = 0, align = 4;

	  for (group = htab->group; group != NULL; group = group->next)
	    if (group->eh_size != 0)
	      size += (group->eh_size + 17 + align - 1) & -align;
	  if (htab->glink != NULL && htab->glink->size != 0)
	    size += (24 + align - 1) & -align;
	  if (size != 0)
	    size += (sizeof (glink_eh_frame_cie) + align - 1) & -align;
	  align = 1ul << htab->glink_eh_frame->output_section->alignment_power;
	  size = (size + align - 1) & -align;
	  htab->glink_eh_frame->rawsize = htab->glink_eh_frame->size;
	  htab->glink_eh_frame->size = size;
	}

      if (htab->params->plt_stub_align != 0)
	for (group = htab->group; group != NULL; group = group->next)
	  if (group->stub_sec != NULL)
	    {
	      int align = abs (htab->params->plt_stub_align);
	      group->stub_sec->size
		= (group->stub_sec->size + (1 << align) - 1) & -(1 << align);
	    }

      if (htab->elf.srelrdyn != NULL)
	{
	  bfd_vma r_offset;

	  for (r_offset = 0; r_offset < htab->brlt->size; r_offset += 8)
	    if (!append_relr_off (htab, htab->brlt, r_offset))
	      return false;

	  if (!got_and_plt_relr_for_local_syms (info))
	    return false;
	  elf_link_hash_traverse (&htab->elf, got_and_plt_relr, info);
	  if (htab->stub_error)
	    return false;

	  bfd_vma *relr_addr = sort_relr (htab);
	  if (htab->relr_count != 0 && relr_addr == NULL)
	    return false;

	  size_t i = 0;
	  while (i < htab->relr_count)
	    {
	      bfd_vma base = relr_addr[i];
	      htab->elf.srelrdyn->size += 8;
	      i++;
	      /* Handle possible duplicate address.  This can happen
		 as sections increase in size when adding stubs.  */
	      while (i < htab->relr_count
		     && relr_addr[i] == base)
		i++;
	      base += 8;
	      while (1)
		{
		  size_t start_i = i;
		  while (i < htab->relr_count
			 && relr_addr[i] - base < 63 * 8
			 && (relr_addr[i] - base) % 8 == 0)
		    i++;
		  if (i == start_i)
		    break;
		  htab->elf.srelrdyn->size += 8;
		  base += 63 * 8;
		}
	    }
	  free (relr_addr);
	}

      for (group = htab->group; group != NULL; group = group->next)
	if (group->stub_sec != NULL
	    && group->stub_sec->rawsize != group->stub_sec->size
	    && (htab->stub_iteration <= STUB_SHRINK_ITER
		|| group->stub_sec->rawsize < group->stub_sec->size))
	  break;

      if (group == NULL
	  && (!htab->stub_changed
	      || htab->stub_iteration > STUB_SHRINK_ITER)
	  && (htab->brlt->rawsize == htab->brlt->size
	      || (htab->stub_iteration > STUB_SHRINK_ITER
		  && htab->brlt->rawsize > htab->brlt->size))
	  && (htab->elf.srelrdyn == NULL
	      || htab->elf.srelrdyn->rawsize == htab->elf.srelrdyn->size
	      || (htab->stub_iteration > STUB_SHRINK_ITER
		  && htab->elf.srelrdyn->rawsize > htab->elf.srelrdyn->size))
	  && (htab->glink_eh_frame == NULL
	      || htab->glink_eh_frame->rawsize == htab->glink_eh_frame->size)
	  && (htab->tga_group == NULL
	      || htab->stub_iteration > 1))
	break;

      if (htab->stub_iteration > STUB_SHRINK_ITER)
	{
	  for (group = htab->group; group != NULL; group = group->next)
	    if (group->stub_sec != NULL
		&& group->stub_sec->size < group->stub_sec->rawsize)
	      group->stub_sec->size = group->stub_sec->rawsize;

	  if (htab->brlt->size < htab->brlt->rawsize)
	    htab->brlt->size = htab->brlt->rawsize;

	  if (htab->elf.srelrdyn != NULL
	      && htab->elf.srelrdyn->size < htab->elf.srelrdyn->rawsize)
	    htab->elf.srelrdyn->size = htab->elf.srelrdyn->rawsize;
	}

      /* Ask the linker to do its stuff.  */
      (*htab->params->layout_sections_again) ();
    }

  if (htab->glink_eh_frame != NULL
      && htab->glink_eh_frame->size != 0)
    {
      bfd_vma val;
      bfd_byte *p, *last_fde;
      size_t last_fde_len, size, align, pad;
      struct map_stub *group;

      /* It is necessary to at least have a rough outline of the
	 linker generated CIEs and FDEs written before
	 bfd_elf_discard_info is run, in order for these FDEs to be
	 indexed in .eh_frame_hdr.  */
      p = bfd_zalloc (htab->glink_eh_frame->owner, htab->glink_eh_frame->size);
      if (p == NULL)
	return false;
      htab->glink_eh_frame->contents = p;
      last_fde = p;
      align = 4;

      memcpy (p, glink_eh_frame_cie, sizeof (glink_eh_frame_cie));
      /* CIE length (rewrite in case little-endian).  */
      last_fde_len = ((sizeof (glink_eh_frame_cie) + align - 1) & -align) - 4;
      bfd_put_32 (htab->elf.dynobj, last_fde_len, p);
      p += last_fde_len + 4;

      for (group = htab->group; group != NULL; group = group->next)
	if (group->eh_size != 0)
	  {
	    group->eh_base = p - htab->glink_eh_frame->contents;
	    last_fde = p;
	    last_fde_len = ((group->eh_size + 17 + align - 1) & -align) - 4;
	    /* FDE length.  */
	    bfd_put_32 (htab->elf.dynobj, last_fde_len, p);
	    p += 4;
	    /* CIE pointer.  */
	    val = p - htab->glink_eh_frame->contents;
	    bfd_put_32 (htab->elf.dynobj, val, p);
	    p += 4;
	    /* Offset to stub section, written later.  */
	    p += 4;
	    /* stub section size.  */
	    bfd_put_32 (htab->elf.dynobj, group->stub_sec->size, p);
	    p += 4;
	    /* Augmentation.  */
	    p += 1;
	    /* Make sure we don't have all nops.  This is enough for
	       elf-eh-frame.c to detect the last non-nop opcode.  */
	    p[group->eh_size - 1] = DW_CFA_advance_loc + 1;
	    p = last_fde + last_fde_len + 4;
	  }
      if (htab->glink != NULL && htab->glink->size != 0)
	{
	  last_fde = p;
	  last_fde_len = ((24 + align - 1) & -align) - 4;
	  /* FDE length.  */
	  bfd_put_32 (htab->elf.dynobj, last_fde_len, p);
	  p += 4;
	  /* CIE pointer.  */
	  val = p - htab->glink_eh_frame->contents;
	  bfd_put_32 (htab->elf.dynobj, val, p);
	  p += 4;
	  /* Offset to .glink, written later.  */
	  p += 4;
	  /* .glink size.  */
	  bfd_put_32 (htab->elf.dynobj, htab->glink->size - 8, p);
	  p += 4;
	  /* Augmentation.  */
	  p += 1;

	  *p++ = DW_CFA_advance_loc + (htab->has_plt_localentry0 ? 3 : 2);
	  *p++ = DW_CFA_register;
	  *p++ = 65;
	  *p++ = htab->opd_abi ? 12 : 0;
	  *p++ = DW_CFA_advance_loc + (htab->opd_abi ? 4 : 2);
	  *p++ = DW_CFA_restore_extended;
	  *p++ = 65;
	  p += ((24 + align - 1) & -align) - 24;
	}
      /* Subsume any padding into the last FDE if user .eh_frame
	 sections are aligned more than glink_eh_frame.  Otherwise any
	 zero padding will be seen as a terminator.  */
      align = 1ul << htab->glink_eh_frame->output_section->alignment_power;
      size = p - htab->glink_eh_frame->contents;
      pad = ((size + align - 1) & -align) - size;
      htab->glink_eh_frame->size = size + pad;
      bfd_put_32 (htab->elf.dynobj, last_fde_len + pad, last_fde);
    }

  maybe_strip_output (info, htab->brlt);
  if (htab->relbrlt != NULL)
    maybe_strip_output (info, htab->relbrlt);
  if (htab->glink_eh_frame != NULL)
    maybe_strip_output (info, htab->glink_eh_frame);
  if (htab->elf.srelrdyn != NULL)
    maybe_strip_output (info, htab->elf.srelrdyn);

  return true;
}

/* Called after we have determined section placement.  If sections
   move, we'll be called again.  Provide a value for TOCstart.  */

bfd_vma
ppc64_elf_set_toc (struct bfd_link_info *info, bfd *obfd)
{
  asection *s;
  bfd_vma TOCstart, adjust;

  if (info != NULL)
    {
      struct elf_link_hash_entry *h;
      struct elf_link_hash_table *htab = elf_hash_table (info);

      if (is_elf_hash_table (&htab->root)
	  && htab->hgot != NULL)
	h = htab->hgot;
      else
	{
	  h = (struct elf_link_hash_entry *)
	    bfd_link_hash_lookup (&htab->root, ".TOC.", false, false, true);
	  if (is_elf_hash_table (&htab->root))
	    htab->hgot = h;
	}
      if (h != NULL
	  && h->root.type == bfd_link_hash_defined
	  && !h->root.linker_def
	  && (!is_elf_hash_table (&htab->root)
	      || h->def_regular))
	{
	  TOCstart = defined_sym_val (h) - TOC_BASE_OFF;
	  _bfd_set_gp_value (obfd, TOCstart);
	  return TOCstart;
	}
    }

  /* The TOC consists of sections .got, .toc, .tocbss, .plt in that
     order.  The TOC starts where the first of these sections starts.  */
  s = bfd_get_section_by_name (obfd, ".got");
  if (s == NULL || (s->flags & SEC_EXCLUDE) != 0)
    s = bfd_get_section_by_name (obfd, ".toc");
  if (s == NULL || (s->flags & SEC_EXCLUDE) != 0)
    s = bfd_get_section_by_name (obfd, ".tocbss");
  if (s == NULL || (s->flags & SEC_EXCLUDE) != 0)
    s = bfd_get_section_by_name (obfd, ".plt");
  if (s == NULL || (s->flags & SEC_EXCLUDE) != 0)
    {
      /* This may happen for
	 o  references to TOC base (SYM@toc / TOC[tc0]) without a
	 .toc directive
	 o  bad linker script
	 o --gc-sections and empty TOC sections

	 FIXME: Warn user?  */

      /* Look for a likely section.  We probably won't even be
	 using TOCstart.  */
      for (s = obfd->sections; s != NULL; s = s->next)
	if ((s->flags & (SEC_ALLOC | SEC_SMALL_DATA | SEC_READONLY
			 | SEC_EXCLUDE))
	    == (SEC_ALLOC | SEC_SMALL_DATA))
	  break;
      if (s == NULL)
	for (s = obfd->sections; s != NULL; s = s->next)
	  if ((s->flags & (SEC_ALLOC | SEC_SMALL_DATA | SEC_EXCLUDE))
	      == (SEC_ALLOC | SEC_SMALL_DATA))
	    break;
      if (s == NULL)
	for (s = obfd->sections; s != NULL; s = s->next)
	  if ((s->flags & (SEC_ALLOC | SEC_READONLY | SEC_EXCLUDE))
	      == SEC_ALLOC)
	    break;
      if (s == NULL)
	for (s = obfd->sections; s != NULL; s = s->next)
	  if ((s->flags & (SEC_ALLOC | SEC_EXCLUDE)) == SEC_ALLOC)
	    break;
    }

  TOCstart = 0;
  if (s != NULL)
    TOCstart = s->output_section->vma + s->output_offset;

  /* Force alignment.  */
  adjust = TOCstart & (TOC_BASE_ALIGN - 1);
  TOCstart -= adjust;
  _bfd_set_gp_value (obfd, TOCstart);

  if (info != NULL && s != NULL)
    {
      struct ppc_link_hash_table *htab = ppc_hash_table (info);

      if (htab != NULL)
	{
	  if (htab->elf.hgot != NULL)
	    {
	      htab->elf.hgot->root.u.def.value = TOC_BASE_OFF - adjust;
	      htab->elf.hgot->root.u.def.section = s;
	    }
	}
      else
	{
	  struct bfd_link_hash_entry *bh = NULL;
	  _bfd_generic_link_add_one_symbol (info, obfd, ".TOC.", BSF_GLOBAL,
					    s, TOC_BASE_OFF - adjust,
					    NULL, false, false, &bh);
	}
    }
  return TOCstart;
}

/* Called via elf_link_hash_traverse from ppc64_elf_build_stubs to
   write out any global entry stubs, and PLT relocations.  */

static bool
build_global_entry_stubs_and_plt (struct elf_link_hash_entry *h, void *inf)
{
  struct bfd_link_info *info;
  struct ppc_link_hash_table *htab;
  struct plt_entry *ent;
  asection *s;

  if (h->root.type == bfd_link_hash_indirect)
    return true;

  info = inf;
  htab = ppc_hash_table (info);
  if (htab == NULL)
    return false;

  for (ent = h->plt.plist; ent != NULL; ent = ent->next)
    if (ent->plt.offset != (bfd_vma) -1)
      {
	/* This symbol has an entry in the procedure linkage
	   table.  Set it up.  */
	Elf_Internal_Rela rela;
	asection *plt, *relplt;
	bfd_byte *loc;

	if (use_local_plt (info, h))
	  {
	    if (!(h->def_regular
		  && (h->root.type == bfd_link_hash_defined
		      || h->root.type == bfd_link_hash_defweak)))
	      continue;
	    if (h->type == STT_GNU_IFUNC)
	      {
		plt = htab->elf.iplt;
		relplt = htab->elf.irelplt;
		htab->elf.ifunc_resolvers = true;
		if (htab->opd_abi)
		  rela.r_info = ELF64_R_INFO (0, R_PPC64_JMP_IREL);
		else
		  rela.r_info = ELF64_R_INFO (0, R_PPC64_IRELATIVE);
	      }
	    else
	      {
		plt = htab->pltlocal;
		relplt = NULL;
		if (bfd_link_pic (info)
		    && !(info->enable_dt_relr && !htab->opd_abi))
		  {
		    relplt = htab->relpltlocal;
		    if (htab->opd_abi)
		      rela.r_info = ELF64_R_INFO (0, R_PPC64_JMP_SLOT);
		    else
		      rela.r_info = ELF64_R_INFO (0, R_PPC64_RELATIVE);
		  }
	      }
	    rela.r_addend = defined_sym_val (h) + ent->addend;

	    if (relplt == NULL)
	      {
		loc = plt->contents + ent->plt.offset;
		bfd_put_64 (info->output_bfd, rela.r_addend, loc);
		if (htab->opd_abi)
		  {
		    bfd_vma toc = elf_gp (info->output_bfd);
		    toc += htab->sec_info[h->root.u.def.section->id].toc_off;
		    bfd_put_64 (info->output_bfd, toc, loc + 8);
		  }
	      }
	    else
	      {
		rela.r_offset = (plt->output_section->vma
				 + plt->output_offset
				 + ent->plt.offset);
		loc = relplt->contents + (relplt->reloc_count++
					  * sizeof (Elf64_External_Rela));
		bfd_elf64_swap_reloca_out (info->output_bfd, &rela, loc);
	      }
	  }
	else
	  {
	    rela.r_offset = (htab->elf.splt->output_section->vma
			     + htab->elf.splt->output_offset
			     + ent->plt.offset);
	    rela.r_info = ELF64_R_INFO (h->dynindx, R_PPC64_JMP_SLOT);
	    rela.r_addend = ent->addend;
	    loc = (htab->elf.srelplt->contents
		   + ((ent->plt.offset - PLT_INITIAL_ENTRY_SIZE (htab))
		      / PLT_ENTRY_SIZE (htab) * sizeof (Elf64_External_Rela)));
	    if (h->type == STT_GNU_IFUNC && is_static_defined (h))
	      htab->elf.ifunc_resolvers = true;
	    bfd_elf64_swap_reloca_out (info->output_bfd, &rela, loc);
	  }
      }

  if (!h->pointer_equality_needed)
    return true;

  if (h->def_regular)
    return true;

  s = htab->global_entry;
  if (s == NULL || s->size == 0)
    return true;

  for (ent = h->plt.plist; ent != NULL; ent = ent->next)
    if (ent->plt.offset != (bfd_vma) -1
	&& ent->addend == 0)
      {
	bfd_byte *p;
	asection *plt;
	bfd_vma off;

	p = s->contents + h->root.u.def.value;
	plt = htab->elf.splt;
	if (use_local_plt (info, h))
	  {
	    if (h->type == STT_GNU_IFUNC)
	      plt = htab->elf.iplt;
	    else
	      plt = htab->pltlocal;
	  }
	off = ent->plt.offset + plt->output_offset + plt->output_section->vma;
	off -= h->root.u.def.value + s->output_offset + s->output_section->vma;

	if (off + 0x80008000 > 0xffffffff || (off & 3) != 0)
	  {
	    info->callbacks->einfo
	      (_("%P: linkage table error against `%pT'\n"),
	       h->root.root.string);
	    bfd_set_error (bfd_error_bad_value);
	    htab->stub_error = true;
	  }

	htab->stub_count[ppc_stub_global_entry - 1] += 1;
	if (htab->params->emit_stub_syms)
	  {
	    size_t len = strlen (h->root.root.string);
	    char *name = bfd_malloc (sizeof "12345678.global_entry." + len);

	    if (name == NULL)
	      return false;

	    sprintf (name, "%08x.global_entry.%s", s->id, h->root.root.string);
	    h = elf_link_hash_lookup (&htab->elf, name, true, false, false);
	    if (h == NULL)
	      return false;
	    if (h->root.type == bfd_link_hash_new)
	      {
		h->root.type = bfd_link_hash_defined;
		h->root.u.def.section = s;
		h->root.u.def.value = p - s->contents;
		h->ref_regular = 1;
		h->def_regular = 1;
		h->ref_regular_nonweak = 1;
		h->forced_local = 1;
		h->non_elf = 0;
		h->root.linker_def = 1;
	      }
	  }

	if (PPC_HA (off) != 0)
	  {
	    bfd_put_32 (s->owner, ADDIS_R12_R12 | PPC_HA (off), p);
	    p += 4;
	  }
	bfd_put_32 (s->owner, LD_R12_0R12 | PPC_LO (off), p);
	p += 4;
	bfd_put_32 (s->owner, MTCTR_R12, p);
	p += 4;
	bfd_put_32 (s->owner, BCTR, p);
	break;
      }
  return true;
}

/* Write PLT relocs for locals.  */

static bool
write_plt_relocs_for_local_syms (struct bfd_link_info *info)
{
  struct ppc_link_hash_table *htab = ppc_hash_table (info);
  bfd *ibfd;

  for (ibfd = info->input_bfds; ibfd != NULL; ibfd = ibfd->link.next)
    {
      struct got_entry **lgot_ents, **end_lgot_ents;
      struct plt_entry **local_plt, **lplt, **end_local_plt;
      Elf_Internal_Shdr *symtab_hdr;
      bfd_size_type locsymcount;
      Elf_Internal_Sym *local_syms = NULL;
      struct plt_entry *ent;

      if (!is_ppc64_elf (ibfd))
	continue;

      lgot_ents = elf_local_got_ents (ibfd);
      if (!lgot_ents)
	continue;

      symtab_hdr = &elf_symtab_hdr (ibfd);
      locsymcount = symtab_hdr->sh_info;
      end_lgot_ents = lgot_ents + locsymcount;
      local_plt = (struct plt_entry **) end_lgot_ents;
      end_local_plt = local_plt + locsymcount;
      for (lplt = local_plt; lplt < end_local_plt; ++lplt)
	for (ent = *lplt; ent != NULL; ent = ent->next)
	  if (ent->plt.offset != (bfd_vma) -1)
	    {
	      Elf_Internal_Sym *sym;
	      asection *sym_sec;
	      asection *plt, *relplt;
	      bfd_byte *loc;
	      bfd_vma val;

	      if (!get_sym_h (NULL, &sym, &sym_sec, NULL, &local_syms,
			      lplt - local_plt, ibfd))
		{
		  if (symtab_hdr->contents != (unsigned char *) local_syms)
		    free (local_syms);
		  return false;
		}

	      val = sym->st_value + ent->addend;
	      if (sym_sec != NULL && sym_sec->output_section != NULL)
		val += sym_sec->output_offset + sym_sec->output_section->vma;

	      if (ELF_ST_TYPE (sym->st_info) == STT_GNU_IFUNC)
		{
		  htab->elf.ifunc_resolvers = true;
		  plt = htab->elf.iplt;
		  relplt = htab->elf.irelplt;
		}
	      else
		{
		  plt = htab->pltlocal;
		  relplt = NULL;
		  if (bfd_link_pic (info)
		      && !(info->enable_dt_relr && !htab->opd_abi))
		    relplt = htab->relpltlocal;
		}

	      if (relplt == NULL)
		{
		  loc = plt->contents + ent->plt.offset;
		  bfd_put_64 (info->output_bfd, val, loc);
		  if (htab->opd_abi)
		    {
		      bfd_vma toc = elf_gp (ibfd);
		      bfd_put_64 (info->output_bfd, toc, loc + 8);
		    }
		}
	      else
		{
		  Elf_Internal_Rela rela;
		  rela.r_offset = (ent->plt.offset
				   + plt->output_offset
				   + plt->output_section->vma);
		  if (ELF_ST_TYPE (sym->st_info) == STT_GNU_IFUNC)
		    {
		      if (htab->opd_abi)
			rela.r_info = ELF64_R_INFO (0, R_PPC64_JMP_IREL);
		      else
			rela.r_info = ELF64_R_INFO (0, R_PPC64_IRELATIVE);
		    }
		  else
		    {
		      if (htab->opd_abi)
			rela.r_info = ELF64_R_INFO (0, R_PPC64_JMP_SLOT);
		      else
			rela.r_info = ELF64_R_INFO (0, R_PPC64_RELATIVE);
		    }
		  rela.r_addend = val;
		  loc = relplt->contents + (relplt->reloc_count++
					    * sizeof (Elf64_External_Rela));
		  bfd_elf64_swap_reloca_out (info->output_bfd, &rela, loc);
		}
	    }

      if (local_syms != NULL
	  && symtab_hdr->contents != (unsigned char *) local_syms)
	{
	  if (!info->keep_memory)
	    free (local_syms);
	  else
	    symtab_hdr->contents = (unsigned char *) local_syms;
	}
    }
  return true;
}

/* Emit the static wrapper function preserving registers around a
   __tls_get_addr_opt call.  */

static bool
emit_tga_desc (struct ppc_link_hash_table *htab)
{
  asection *stub_sec = htab->tga_group->stub_sec;
  unsigned int cfa_updt = 11 * 4;
  bfd_byte *p;
  bfd_vma to, from, delta;

  BFD_ASSERT (htab->tga_desc_fd->elf.root.type == bfd_link_hash_defined
	      && htab->tga_desc_fd->elf.root.u.def.section == stub_sec
	      && htab->tga_desc_fd->elf.root.u.def.value == 0);
  to = defined_sym_val (&htab->tls_get_addr_fd->elf);
  from = defined_sym_val (&htab->tga_desc_fd->elf) + cfa_updt;
  delta = to - from;
  if (delta + (1 << 25) >= 1 << 26)
    {
      _bfd_error_handler (_("__tls_get_addr call offset overflow"));
      htab->stub_error = true;
      return false;
    }

  p = stub_sec->contents;
  p = tls_get_addr_prologue (htab->elf.dynobj, p, htab);
  bfd_put_32 (stub_sec->owner, B_DOT | 1 | (delta & 0x3fffffc), p);
  p += 4;
  p = tls_get_addr_epilogue (htab->elf.dynobj, p, htab);
  return stub_sec->size == (bfd_size_type) (p - stub_sec->contents);
}

/* Emit eh_frame describing the static wrapper function.  */

static bfd_byte *
emit_tga_desc_eh_frame (struct ppc_link_hash_table *htab, bfd_byte *p)
{
  unsigned int cfa_updt = 11 * 4;
  unsigned int i;

  *p++ = DW_CFA_advance_loc + cfa_updt / 4;
  *p++ = DW_CFA_def_cfa_offset;
  if (htab->opd_abi)
    {
      *p++ = 128;
      *p++ = 1;
    }
  else
    *p++ = 96;
  *p++ = DW_CFA_offset_extended_sf;
  *p++ = 65;
  *p++ = (-16 / 8) & 0x7f;
  for (i = 4; i < 12; i++)
    {
      *p++ = DW_CFA_offset + i;
      *p++ = (htab->opd_abi ? 13 : 12) - i;
    }
  *p++ = DW_CFA_advance_loc + 10;
  *p++ = DW_CFA_def_cfa_offset;
  *p++ = 0;
  for (i = 4; i < 12; i++)
    *p++ = DW_CFA_restore + i;
  *p++ = DW_CFA_advance_loc + 2;
  *p++ = DW_CFA_restore_extended;
  *p++ = 65;
  return p;
}

/* Build all the stubs associated with the current output file.
   The stubs are kept in a hash table attached to the main linker
   hash table.  This function is called via gldelf64ppc_finish.  */

bool
ppc64_elf_build_stubs (struct bfd_link_info *info,
		       char **stats)
{
  struct ppc_link_hash_table *htab = ppc_hash_table (info);
  struct map_stub *group;
  asection *stub_sec;
  bfd_byte *p;
  int stub_sec_count = 0;

  if (htab == NULL)
    return false;

  /* Allocate memory to hold the linker stubs.  */
  for (group = htab->group; group != NULL; group = group->next)
    {
      group->eh_size = 0;
      group->lr_restore = 0;
      if ((stub_sec = group->stub_sec) != NULL
	  && stub_sec->size != 0)
	{
	  stub_sec->contents = bfd_zalloc (htab->params->stub_bfd,
					   stub_sec->size);
	  if (stub_sec->contents == NULL)
	    return false;
	  stub_sec->size = 0;
	}
    }

  if (htab->glink != NULL && htab->glink->size != 0)
    {
      unsigned int indx;
      bfd_vma plt0;

      /* Build the .glink plt call stub.  */
      if (htab->params->emit_stub_syms)
	{
	  struct elf_link_hash_entry *h;
	  h = elf_link_hash_lookup (&htab->elf, "__glink_PLTresolve",
				    true, false, false);
	  if (h == NULL)
	    return false;
	  if (h->root.type == bfd_link_hash_new)
	    {
	      h->root.type = bfd_link_hash_defined;
	      h->root.u.def.section = htab->glink;
	      h->root.u.def.value = 8;
	      h->ref_regular = 1;
	      h->def_regular = 1;
	      h->ref_regular_nonweak = 1;
	      h->forced_local = 1;
	      h->non_elf = 0;
	      h->root.linker_def = 1;
	    }
	}
      plt0 = (htab->elf.splt->output_section->vma
	      + htab->elf.splt->output_offset
	      - 16);
      if (info->emitrelocations)
	{
	  Elf_Internal_Rela *r = get_relocs (htab->glink, 1);
	  if (r == NULL)
	    return false;
	  r->r_offset = (htab->glink->output_offset
			 + htab->glink->output_section->vma);
	  r->r_info = ELF64_R_INFO (0, R_PPC64_REL64);
	  r->r_addend = plt0;
	}
      p = htab->glink->contents;
      plt0 -= htab->glink->output_section->vma + htab->glink->output_offset;
      bfd_put_64 (htab->glink->owner, plt0, p);
      p += 8;
      if (htab->opd_abi)
	{
	  bfd_put_32 (htab->glink->owner, MFLR_R12, p);
	  p += 4;
	  bfd_put_32 (htab->glink->owner, BCL_20_31, p);
	  p += 4;
	  bfd_put_32 (htab->glink->owner, MFLR_R11, p);
	  p += 4;
	  bfd_put_32 (htab->glink->owner, LD_R2_0R11 | (-16 & 0xfffc), p);
	  p += 4;
	  bfd_put_32 (htab->glink->owner, MTLR_R12, p);
	  p += 4;
	  bfd_put_32 (htab->glink->owner, ADD_R11_R2_R11, p);
	  p += 4;
	  bfd_put_32 (htab->glink->owner, LD_R12_0R11, p);
	  p += 4;
	  bfd_put_32 (htab->glink->owner, LD_R2_0R11 | 8, p);
	  p += 4;
	  bfd_put_32 (htab->glink->owner, MTCTR_R12, p);
	  p += 4;
	  bfd_put_32 (htab->glink->owner, LD_R11_0R11 | 16, p);
	  p += 4;
	}
      else
	{
	  unsigned int insn;

	  /* 0:
	     .	.quad plt0-1f		# plt0 entry relative to 1:
	     #
	     # We get here with r12 initially @ a glink branch
	     # Load the address of _dl_runtime_resolve from plt0 and
	     # jump to it, with r0 set to the index of the PLT entry
	     # to be resolved and r11 the link map.
	     __glink_PLTresolve:
	     .	std %r2,24(%r1)		# optional
	     .	mflr %r0
	     .	bcl 20,31,1f
	     1:
	     .	mflr %r11
	     .	mtlr %r0
	     .	ld %r0,(0b-1b)(%r11)
	     .	sub %r12,%r12,%r11
	     .	add %r11,%r0,%r11
	     .	addi %r0,%r12,1b-2f
	     .	ld %r12,0(%r11)
	     .	srdi %r0,%r0,2
	     .	mtctr %r12
	     .	ld %r11,8(%r11)
	     .	bctr
	     2:
	     .	b __glink_PLTresolve
	     .	...
	     .	b __glink_PLTresolve  */

	  if (htab->has_plt_localentry0)
	    {
	      bfd_put_32 (htab->glink->owner, STD_R2_0R1 + 24, p);
	      p += 4;
	    }
	  bfd_put_32 (htab->glink->owner, MFLR_R0, p);
	  p += 4;
	  bfd_put_32 (htab->glink->owner, BCL_20_31, p);
	  p += 4;
	  bfd_put_32 (htab->glink->owner, MFLR_R11, p);
	  p += 4;
	  bfd_put_32 (htab->glink->owner, MTLR_R0, p);
	  p += 4;
	  if (htab->has_plt_localentry0)
	    insn = LD_R0_0R11 | (-20 & 0xfffc);
	  else
	    insn = LD_R0_0R11 | (-16 & 0xfffc);
	  bfd_put_32 (htab->glink->owner, insn, p);
	  p += 4;
	  bfd_put_32 (htab->glink->owner, SUB_R12_R12_R11, p);
	  p += 4;
	  bfd_put_32 (htab->glink->owner, ADD_R11_R0_R11, p);
	  p += 4;
	  bfd_put_32 (htab->glink->owner, ADDI_R0_R12 | (-44 & 0xffff), p);
	  p += 4;
	  bfd_put_32 (htab->glink->owner, LD_R12_0R11, p);
	  p += 4;
	  bfd_put_32 (htab->glink->owner, SRDI_R0_R0_2, p);
	  p += 4;
	  bfd_put_32 (htab->glink->owner, MTCTR_R12, p);
	  p += 4;
	  bfd_put_32 (htab->glink->owner, LD_R11_0R11 | 8, p);
	  p += 4;
	}
      bfd_put_32 (htab->glink->owner, BCTR, p);
      p += 4;
      BFD_ASSERT (p == htab->glink->contents + GLINK_PLTRESOLVE_SIZE (htab));

      /* Build the .glink lazy link call stubs.  */
      indx = 0;
      while (p < htab->glink->contents + htab->glink->size)
	{
	  if (htab->opd_abi)
	    {
	      if (indx < 0x8000)
		{
		  bfd_put_32 (htab->glink->owner, LI_R0_0 | indx, p);
		  p += 4;
		}
	      else
		{
		  bfd_put_32 (htab->glink->owner, LIS_R0_0 | PPC_HI (indx), p);
		  p += 4;
		  bfd_put_32 (htab->glink->owner, ORI_R0_R0_0 | PPC_LO (indx),
			      p);
		  p += 4;
		}
	    }
	  bfd_put_32 (htab->glink->owner,
		      B_DOT | ((htab->glink->contents - p + 8) & 0x3fffffc), p);
	  indx++;
	  p += 4;
	}
    }

  if (htab->tga_group != NULL)
    {
      htab->tga_group->lr_restore = 23 * 4;
      htab->tga_group->stub_sec->size = 24 * 4;
      if (!emit_tga_desc (htab))
	return false;
      if (htab->glink_eh_frame != NULL
	  && htab->glink_eh_frame->size != 0)
	{
	  size_t align = 4;

	  p = htab->glink_eh_frame->contents;
	  p += (sizeof (glink_eh_frame_cie) + align - 1) & -align;
	  p += 17;
	  htab->tga_group->eh_size = emit_tga_desc_eh_frame (htab, p) - p;
	}
    }

  /* Build .glink global entry stubs, and PLT relocs for globals.  */
  elf_link_hash_traverse (&htab->elf, build_global_entry_stubs_and_plt, info);

  if (!write_plt_relocs_for_local_syms (info))
    return false;

  if (htab->brlt != NULL && htab->brlt->size != 0)
    {
      htab->brlt->contents = bfd_zalloc (htab->brlt->owner,
					 htab->brlt->size);
      if (htab->brlt->contents == NULL)
	return false;
    }
  if (htab->relbrlt != NULL && htab->relbrlt->size != 0)
    {
      htab->relbrlt->contents = bfd_zalloc (htab->relbrlt->owner,
					    htab->relbrlt->size);
      if (htab->relbrlt->contents == NULL)
	return false;
    }

  /* Build the stubs as directed by the stub hash table.  */
  htab->stub_id = 0;
  bfd_hash_traverse (&htab->stub_hash_table, ppc_build_one_stub, info);

  for (group = htab->group; group != NULL; group = group->next)
    if (group->needs_save_res)
      group->stub_sec->size += htab->sfpr->size;

  if (htab->relbrlt != NULL)
    htab->relbrlt->reloc_count = 0;

  if (htab->params->plt_stub_align != 0)
    for (group = htab->group; group != NULL; group = group->next)
      if ((stub_sec = group->stub_sec) != NULL)
	{
	  int align = abs (htab->params->plt_stub_align);
	  stub_sec->size = (stub_sec->size + (1 << align) - 1) & -(1 << align);
	}

  for (group = htab->group; group != NULL; group = group->next)
    if (group->needs_save_res)
      {
	stub_sec = group->stub_sec;
	memcpy (stub_sec->contents + stub_sec->size - htab->sfpr->size,
		htab->sfpr->contents, htab->sfpr->size);
	if (htab->params->emit_stub_syms)
	  {
	    unsigned int i;

	    for (i = 0; i < ARRAY_SIZE (save_res_funcs); i++)
	      if (!sfpr_define (info, &save_res_funcs[i], stub_sec))
		return false;
	  }
      }

  if (htab->glink_eh_frame != NULL
      && htab->glink_eh_frame->size != 0)
    {
      bfd_vma val;
      size_t align = 4;

      p = htab->glink_eh_frame->contents;
      p += (sizeof (glink_eh_frame_cie) + align - 1) & -align;

      for (group = htab->group; group != NULL; group = group->next)
	if (group->eh_size != 0)
	  {
	    /* Offset to stub section.  */
	    val = (group->stub_sec->output_section->vma
		   + group->stub_sec->output_offset);
	    val -= (htab->glink_eh_frame->output_section->vma
		    + htab->glink_eh_frame->output_offset
		    + (p + 8 - htab->glink_eh_frame->contents));
	    if (val + 0x80000000 > 0xffffffff)
	      {
		_bfd_error_handler
		  (_("%s offset too large for .eh_frame sdata4 encoding"),
		   group->stub_sec->name);
		return false;
	      }
	    bfd_put_32 (htab->elf.dynobj, val, p + 8);
	    p += (group->eh_size + 17 + 3) & -4;
	  }
      if (htab->glink != NULL && htab->glink->size != 0)
	{
	  /* Offset to .glink.  */
	  val = (htab->glink->output_section->vma
		 + htab->glink->output_offset
		 + 8);
	  val -= (htab->glink_eh_frame->output_section->vma
		  + htab->glink_eh_frame->output_offset
		  + (p + 8 - htab->glink_eh_frame->contents));
	  if (val + 0x80000000 > 0xffffffff)
	    {
	      _bfd_error_handler
		(_("%s offset too large for .eh_frame sdata4 encoding"),
		 htab->glink->name);
	      return false;
	    }
	  bfd_put_32 (htab->elf.dynobj, val, p + 8);
	  p += (24 + align - 1) & -align;
	}
    }

  if (htab->elf.srelrdyn != NULL && htab->elf.srelrdyn->size != 0)
    {
      htab->elf.srelrdyn->contents
	= bfd_alloc (htab->elf.dynobj, htab->elf.srelrdyn->size);
      if (htab->elf.srelrdyn->contents == NULL)
	return false;

      bfd_vma *relr_addr = sort_relr (htab);
      if (htab->relr_count != 0 && relr_addr == NULL)
	return false;

      size_t i = 0;
      bfd_byte *loc = htab->elf.srelrdyn->contents;
      while (i < htab->relr_count)
	{
	  bfd_vma base = relr_addr[i];
	  BFD_ASSERT (base % 2 == 0);
	  bfd_put_64 (htab->elf.dynobj, base, loc);
	  loc += 8;
	  i++;
	  while (i < htab->relr_count
		 && relr_addr[i] == base)
	    {
	      htab->stub_error = true;
	      i++;
	    }
	  base += 8;
	  while (1)
	    {
	      bfd_vma bits = 0;
	      while (i < htab->relr_count
		     && relr_addr[i] - base < 63 * 8
		     && (relr_addr[i] - base) % 8 == 0)
		{
		  bits |= (bfd_vma) 1 << ((relr_addr[i] - base) / 8);
		  i++;
		}
	      if (bits == 0)
		break;
	      bfd_put_64 (htab->elf.dynobj, (bits << 1) | 1, loc);
	      loc += 8;
	      base += 63 * 8;
	    }
	}
      free (relr_addr);
      /* Pad any excess with 1's, a do-nothing encoding.  */
      while ((size_t) (loc - htab->elf.srelrdyn->contents)
	     < htab->elf.srelrdyn->size)
	{
	  bfd_put_64 (htab->elf.dynobj, 1, loc);
	  loc += 8;
	}
    }

  for (group = htab->group; group != NULL; group = group->next)
    if ((stub_sec = group->stub_sec) != NULL)
      {
	stub_sec_count += 1;
	if (stub_sec->rawsize != stub_sec->size
	    && (htab->stub_iteration <= STUB_SHRINK_ITER
		|| stub_sec->rawsize < stub_sec->size))
	  break;
      }

  if (group != NULL)
    htab->stub_error = true;

  if (htab->stub_error)
    {
      _bfd_error_handler (_("stubs don't match calculated size"));
      return false;
    }

  if (stats != NULL)
    {
      char *groupmsg;
      if (asprintf (&groupmsg,
		    ngettext ("linker stubs in %u group",
			      "linker stubs in %u groups",
			      stub_sec_count),
		    stub_sec_count) < 0)
	*stats = NULL;
      else
	{
	  if (asprintf (stats, _("%s, iter %u\n"
				 "  branch         %lu\n"
				 "  long branch    %lu\n"
				 "  plt call       %lu\n"
				 "  global entry   %lu"),
			groupmsg, htab->stub_iteration,
			htab->stub_count[ppc_stub_long_branch - 1],
			htab->stub_count[ppc_stub_plt_branch - 1],
			htab->stub_count[ppc_stub_plt_call - 1],
			htab->stub_count[ppc_stub_global_entry - 1]) < 0)
	    *stats = NULL;
	  free (groupmsg);
	}
    }
  return true;
}

/* What to do when ld finds relocations against symbols defined in
   discarded sections.  */

static unsigned int
ppc64_elf_action_discarded (asection *sec)
{
  if (strcmp (".opd", sec->name) == 0)
    return 0;

  if (strcmp (".toc", sec->name) == 0)
    return 0;

  if (strcmp (".toc1", sec->name) == 0)
    return 0;

  return _bfd_elf_default_action_discarded (sec);
}

/* These are the dynamic relocations supported by glibc.  */

static bool
ppc64_glibc_dynamic_reloc (enum elf_ppc64_reloc_type r_type)
{
  switch (r_type)
    {
    case R_PPC64_RELATIVE:
    case R_PPC64_NONE:
    case R_PPC64_ADDR64:
    case R_PPC64_GLOB_DAT:
    case R_PPC64_IRELATIVE:
    case R_PPC64_JMP_IREL:
    case R_PPC64_JMP_SLOT:
    case R_PPC64_DTPMOD64:
    case R_PPC64_DTPREL64:
    case R_PPC64_TPREL64:
    case R_PPC64_TPREL16_LO_DS:
    case R_PPC64_TPREL16_DS:
    case R_PPC64_TPREL16:
    case R_PPC64_TPREL16_LO:
    case R_PPC64_TPREL16_HI:
    case R_PPC64_TPREL16_HIGH:
    case R_PPC64_TPREL16_HA:
    case R_PPC64_TPREL16_HIGHA:
    case R_PPC64_TPREL16_HIGHER:
    case R_PPC64_TPREL16_HIGHEST:
    case R_PPC64_TPREL16_HIGHERA:
    case R_PPC64_TPREL16_HIGHESTA:
    case R_PPC64_ADDR16_LO_DS:
    case R_PPC64_ADDR16_LO:
    case R_PPC64_ADDR16_HI:
    case R_PPC64_ADDR16_HIGH:
    case R_PPC64_ADDR16_HA:
    case R_PPC64_ADDR16_HIGHA:
    case R_PPC64_REL30:
    case R_PPC64_COPY:
    case R_PPC64_UADDR64:
    case R_PPC64_UADDR32:
    case R_PPC64_ADDR32:
    case R_PPC64_ADDR24:
    case R_PPC64_ADDR16:
    case R_PPC64_UADDR16:
    case R_PPC64_ADDR16_DS:
    case R_PPC64_ADDR16_HIGHER:
    case R_PPC64_ADDR16_HIGHEST:
    case R_PPC64_ADDR16_HIGHERA:
    case R_PPC64_ADDR16_HIGHESTA:
    case R_PPC64_ADDR14:
    case R_PPC64_ADDR14_BRTAKEN:
    case R_PPC64_ADDR14_BRNTAKEN:
    case R_PPC64_REL32:
    case R_PPC64_REL64:
      return true;

    default:
      return false;
    }
}

/* The RELOCATE_SECTION function is called by the ELF backend linker
   to handle the relocations for a section.

   The relocs are always passed as Rela structures; if the section
   actually uses Rel structures, the r_addend field will always be
   zero.

   This function is responsible for adjust the section contents as
   necessary, and (if using Rela relocs and generating a
   relocatable output file) adjusting the reloc addend as
   necessary.

   This function does not have to worry about setting the reloc
   address or the reloc symbol index.

   LOCAL_SYMS is a pointer to the swapped in local symbols.

   LOCAL_SECTIONS is an array giving the section in the input file
   corresponding to the st_shndx field of each local symbol.

   The global hash table entry for the global symbols can be found
   via elf_sym_hashes (input_bfd).

   When generating relocatable output, this function must handle
   STB_LOCAL/STT_SECTION symbols specially.  The output symbol is
   going to be the section symbol corresponding to the output
   section, which means that the addend must be adjusted
   accordingly.  */

static int
ppc64_elf_relocate_section (bfd *output_bfd,
			    struct bfd_link_info *info,
			    bfd *input_bfd,
			    asection *input_section,
			    bfd_byte *contents,
			    Elf_Internal_Rela *relocs,
			    Elf_Internal_Sym *local_syms,
			    asection **local_sections)
{
  struct ppc_link_hash_table *htab;
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  Elf_Internal_Rela *rel;
  Elf_Internal_Rela *wrel;
  Elf_Internal_Rela *relend;
  Elf_Internal_Rela outrel;
  bfd_byte *loc;
  struct got_entry **local_got_ents;
  bfd_vma TOCstart;
  bool ret = true;
  bool is_opd;
  /* Assume 'at' branch hints.  */
  bool is_isa_v2 = true;
  bool warned_dynamic = false;
  bfd_vma d_offset = (bfd_big_endian (input_bfd) ? 2 : 0);

  /* Initialize howto table if needed.  */
  if (!ppc64_elf_howto_table[R_PPC64_ADDR32])
    ppc_howto_init ();

  htab = ppc_hash_table (info);
  if (htab == NULL)
    return false;

  /* Don't relocate stub sections.  */
  if (input_section->owner == htab->params->stub_bfd)
    return true;

  if (!is_ppc64_elf (input_bfd))
    {
      bfd_set_error (bfd_error_wrong_format);
      return false;
    }

  local_got_ents = elf_local_got_ents (input_bfd);
  TOCstart = elf_gp (output_bfd);
  symtab_hdr = &elf_symtab_hdr (input_bfd);
  sym_hashes = elf_sym_hashes (input_bfd);
  is_opd = ppc64_elf_section_data (input_section)->sec_type == sec_opd;

  rel = wrel = relocs;
  relend = relocs + input_section->reloc_count;
  for (; rel < relend; wrel++, rel++)
    {
      enum elf_ppc64_reloc_type r_type;
      bfd_vma addend;
      bfd_reloc_status_type r;
      Elf_Internal_Sym *sym;
      asection *sec;
      struct elf_link_hash_entry *h_elf;
      struct ppc_link_hash_entry *h;
      struct ppc_link_hash_entry *fdh;
      const char *sym_name;
      unsigned long r_symndx, toc_symndx;
      bfd_vma toc_addend;
      unsigned char tls_mask, tls_gd, tls_type;
      unsigned char sym_type;
      bfd_vma relocation;
      bool unresolved_reloc, save_unresolved_reloc;
      bool warned;
      enum { DEST_NORMAL, DEST_OPD, DEST_STUB } reloc_dest;
      unsigned int insn;
      unsigned int mask;
      struct ppc_stub_hash_entry *stub_entry;
      bfd_vma max_br_offset;
      bfd_vma from;
      Elf_Internal_Rela orig_rel;
      reloc_howto_type *howto;
      struct reloc_howto_struct alt_howto;
      uint64_t pinsn;
      bfd_vma offset;

    again:
      orig_rel = *rel;

      r_type = ELF64_R_TYPE (rel->r_info);
      r_symndx = ELF64_R_SYM (rel->r_info);

      /* For old style R_PPC64_TOC relocs with a zero symbol, use the
	 symbol of the previous ADDR64 reloc.  The symbol gives us the
	 proper TOC base to use.  */
      if (rel->r_info == ELF64_R_INFO (0, R_PPC64_TOC)
	  && wrel != relocs
	  && ELF64_R_TYPE (wrel[-1].r_info) == R_PPC64_ADDR64
	  && is_opd)
	r_symndx = ELF64_R_SYM (wrel[-1].r_info);

      sym = NULL;
      sec = NULL;
      h_elf = NULL;
      sym_name = NULL;
      unresolved_reloc = false;
      warned = false;

      if (r_symndx < symtab_hdr->sh_info)
	{
	  /* It's a local symbol.  */
	  struct _opd_sec_data *opd;

	  sym = local_syms + r_symndx;
	  sec = local_sections[r_symndx];
	  sym_name = bfd_elf_sym_name (input_bfd, symtab_hdr, sym, sec);
	  sym_type = ELF64_ST_TYPE (sym->st_info);
	  relocation = _bfd_elf_rela_local_sym (output_bfd, sym, &sec, rel);
	  opd = get_opd_info (sec);
	  if (opd != NULL && opd->adjust != NULL)
	    {
	      long adjust = opd->adjust[OPD_NDX (sym->st_value
						 + rel->r_addend)];
	      if (adjust == -1)
		relocation = 0;
	      else
		{
		  /* If this is a relocation against the opd section sym
		     and we have edited .opd, adjust the reloc addend so
		     that ld -r and ld --emit-relocs output is correct.
		     If it is a reloc against some other .opd symbol,
		     then the symbol value will be adjusted later.  */
		  if (ELF_ST_TYPE (sym->st_info) == STT_SECTION)
		    rel->r_addend += adjust;
		  else
		    relocation += adjust;
		}
	    }
	}
      else
	{
	  bool ignored;

	  RELOC_FOR_GLOBAL_SYMBOL (info, input_bfd, input_section, rel,
				   r_symndx, symtab_hdr, sym_hashes,
				   h_elf, sec, relocation,
				   unresolved_reloc, warned, ignored);
	  sym_name = h_elf->root.root.string;
	  sym_type = h_elf->type;
	  if (sec != NULL
	      && sec->owner == output_bfd
	      && strcmp (sec->name, ".opd") == 0)
	    {
	      /* This is a symbol defined in a linker script.  All
		 such are defined in output sections, even those
		 defined by simple assignment from a symbol defined in
		 an input section.  Transfer the symbol to an
		 appropriate input .opd section, so that a branch to
		 this symbol will be mapped to the location specified
		 by the opd entry.  */
	      struct bfd_link_order *lo;
	      for (lo = sec->map_head.link_order; lo != NULL; lo = lo->next)
		if (lo->type == bfd_indirect_link_order)
		  {
		    asection *isec = lo->u.indirect.section;
		    if (h_elf->root.u.def.value >= isec->output_offset
			&& h_elf->root.u.def.value < (isec->output_offset
						      + isec->size))
		      {
			h_elf->root.u.def.value -= isec->output_offset;
			h_elf->root.u.def.section = isec;
			sec = isec;
			break;
		      }
		  }
	    }
	}
      h = ppc_elf_hash_entry (h_elf);

      if (sec != NULL && discarded_section (sec))
	{
	  _bfd_clear_contents (ppc64_elf_howto_table[r_type],
			       input_bfd, input_section,
			       contents, rel->r_offset);
	  wrel->r_offset = rel->r_offset;
	  wrel->r_info = 0;
	  wrel->r_addend = 0;

	  /* For ld -r, remove relocations in debug sections against
	     symbols defined in discarded sections.  Not done for
	     non-debug to preserve relocs in .eh_frame which the
	     eh_frame editing code expects to be present.  */
	  if (bfd_link_relocatable (info)
	      && (input_section->flags & SEC_DEBUGGING))
	    wrel--;

	  continue;
	}

      if (bfd_link_relocatable (info))
	goto copy_reloc;

      if (h != NULL && &h->elf == htab->elf.hgot)
	{
	  relocation = TOCstart + htab->sec_info[input_section->id].toc_off;
	  sec = bfd_abs_section_ptr;
	  unresolved_reloc = false;
	}

      /* TLS optimizations.  Replace instruction sequences and relocs
	 based on information we collected in tls_optimize.  We edit
	 RELOCS so that --emit-relocs will output something sensible
	 for the final instruction stream.  */
      tls_mask = 0;
      tls_gd = 0;
      toc_symndx = 0;
      if (h != NULL)
	tls_mask = h->tls_mask;
      else if (local_got_ents != NULL)
	{
	  struct plt_entry **local_plt = (struct plt_entry **)
	    (local_got_ents + symtab_hdr->sh_info);
	  unsigned char *lgot_masks = (unsigned char *)
	    (local_plt + symtab_hdr->sh_info);
	  tls_mask = lgot_masks[r_symndx];
	}
      if (((tls_mask & TLS_TLS) == 0 || tls_mask == (TLS_TLS | TLS_MARK))
	  && (r_type == R_PPC64_TLS
	      || r_type == R_PPC64_TLSGD
	      || r_type == R_PPC64_TLSLD))
	{
	  /* Check for toc tls entries.  */
	  unsigned char *toc_tls;

	  if (!get_tls_mask (&toc_tls, &toc_symndx, &toc_addend,
			     &local_syms, rel, input_bfd))
	    return false;

	  if (toc_tls)
	    tls_mask = *toc_tls;
	}

      /* Check that tls relocs are used with tls syms, and non-tls
	 relocs are used with non-tls syms.  */
      if (r_symndx != STN_UNDEF
	  && r_type != R_PPC64_NONE
	  && (h == NULL
	      || h->elf.root.type == bfd_link_hash_defined
	      || h->elf.root.type == bfd_link_hash_defweak)
	  && IS_PPC64_TLS_RELOC (r_type) != (sym_type == STT_TLS))
	{
	  if ((tls_mask & TLS_TLS) != 0
	      && (r_type == R_PPC64_TLS
		  || r_type == R_PPC64_TLSGD
		  || r_type == R_PPC64_TLSLD))
	    /* R_PPC64_TLS is OK against a symbol in the TOC.  */
	    ;
	  else
	    info->callbacks->einfo
	      (!IS_PPC64_TLS_RELOC (r_type)
	       /* xgettext:c-format */
	       ? _("%H: %s used with TLS symbol `%pT'\n")
	       /* xgettext:c-format */
	       : _("%H: %s used with non-TLS symbol `%pT'\n"),
	       input_bfd, input_section, rel->r_offset,
	       ppc64_elf_howto_table[r_type]->name,
	       sym_name);
	}

      /* Ensure reloc mapping code below stays sane.  */
      if (R_PPC64_TOC16_LO_DS != R_PPC64_TOC16_DS + 1
	  || R_PPC64_TOC16_LO != R_PPC64_TOC16 + 1
	  || (R_PPC64_GOT_TLSLD16 & 3)    != (R_PPC64_GOT_TLSGD16 & 3)
	  || (R_PPC64_GOT_TLSLD16_LO & 3) != (R_PPC64_GOT_TLSGD16_LO & 3)
	  || (R_PPC64_GOT_TLSLD16_HI & 3) != (R_PPC64_GOT_TLSGD16_HI & 3)
	  || (R_PPC64_GOT_TLSLD16_HA & 3) != (R_PPC64_GOT_TLSGD16_HA & 3)
	  || (R_PPC64_GOT_TLSLD16 & 3)    != (R_PPC64_GOT_TPREL16_DS & 3)
	  || (R_PPC64_GOT_TLSLD16_LO & 3) != (R_PPC64_GOT_TPREL16_LO_DS & 3)
	  || (R_PPC64_GOT_TLSLD16_HI & 3) != (R_PPC64_GOT_TPREL16_HI & 3)
	  || (R_PPC64_GOT_TLSLD16_HA & 3) != (R_PPC64_GOT_TPREL16_HA & 3))
	abort ();

      switch (r_type)
	{
	default:
	  break;

	case R_PPC64_LO_DS_OPT:
	  if (offset_in_range (input_section, rel->r_offset - d_offset, 4))
	    {
	      insn = bfd_get_32 (input_bfd,
				 contents + rel->r_offset - d_offset);
	      if ((insn & (0x3fu << 26)) != 58u << 26)
		abort ();
	      insn += (14u << 26) - (58u << 26);
	      bfd_put_32 (input_bfd, insn,
			  contents + rel->r_offset - d_offset);
	      r_type = R_PPC64_TOC16_LO;
	      rel->r_info = ELF64_R_INFO (r_symndx, r_type);
	    }
	  break;

	case R_PPC64_TOC16:
	case R_PPC64_TOC16_LO:
	case R_PPC64_TOC16_DS:
	case R_PPC64_TOC16_LO_DS:
	  {
	    /* Check for toc tls entries.  */
	    unsigned char *toc_tls;
	    int retval;

	    retval = get_tls_mask (&toc_tls, &toc_symndx, &toc_addend,
				   &local_syms, rel, input_bfd);
	    if (retval == 0)
	      return false;

	    if (toc_tls)
	      {
		tls_mask = *toc_tls;
		if (r_type == R_PPC64_TOC16_DS
		    || r_type == R_PPC64_TOC16_LO_DS)
		  {
		    if ((tls_mask & TLS_TLS) != 0
			&& (tls_mask & (TLS_DTPREL | TLS_TPREL)) == 0)
		      goto toctprel;
		  }
		else
		  {
		    /* If we found a GD reloc pair, then we might be
		       doing a GD->IE transition.  */
		    if (retval == 2)
		      {
			tls_gd = TLS_GDIE;
			if ((tls_mask & TLS_TLS) != 0
			    && (tls_mask & TLS_GD) == 0)
			  goto tls_ldgd_opt;
		      }
		    else if (retval == 3)
		      {
			if ((tls_mask & TLS_TLS) != 0
			    && (tls_mask & TLS_LD) == 0)
			  goto tls_ldgd_opt;
		      }
		  }
	      }
	  }
	  break;

	case R_PPC64_GOT_TPREL16_HI:
	case R_PPC64_GOT_TPREL16_HA:
	  if ((tls_mask & TLS_TLS) != 0
	      && (tls_mask & TLS_TPREL) == 0
	      && offset_in_range (input_section, rel->r_offset - d_offset, 4))
	    {
	      rel->r_offset -= d_offset;
	      bfd_put_32 (input_bfd, NOP, contents + rel->r_offset);
	      r_type = R_PPC64_NONE;
	      rel->r_info = ELF64_R_INFO (r_symndx, r_type);
	    }
	  break;

	case R_PPC64_GOT_TPREL16_DS:
	case R_PPC64_GOT_TPREL16_LO_DS:
	  if ((tls_mask & TLS_TLS) != 0
	      && (tls_mask & TLS_TPREL) == 0
	      && offset_in_range (input_section, rel->r_offset - d_offset, 4))
	    {
	    toctprel:
	      insn = bfd_get_32 (input_bfd,
				 contents + rel->r_offset - d_offset);
	      insn &= 31 << 21;
	      insn |= 0x3c0d0000;	/* addis 0,13,0 */
	      bfd_put_32 (input_bfd, insn,
			  contents + rel->r_offset - d_offset);
	      r_type = R_PPC64_TPREL16_HA;
	      if (toc_symndx != 0)
		{
		  rel->r_info = ELF64_R_INFO (toc_symndx, r_type);
		  rel->r_addend = toc_addend;
		  /* We changed the symbol.  Start over in order to
		     get h, sym, sec etc. right.  */
		  goto again;
		}
	      else
		rel->r_info = ELF64_R_INFO (r_symndx, r_type);
	    }
	  break;

	case R_PPC64_GOT_TPREL_PCREL34:
	  if ((tls_mask & TLS_TLS) != 0
	      && (tls_mask & TLS_TPREL) == 0
	      && offset_in_range (input_section, rel->r_offset, 8))
	    {
	      /* pld ra,sym@got@tprel@pcrel -> paddi ra,r13,sym@tprel  */
	      pinsn = bfd_get_32 (input_bfd, contents + rel->r_offset);
	      pinsn <<= 32;
	      pinsn |= bfd_get_32 (input_bfd, contents + rel->r_offset + 4);
	      pinsn += ((2ULL << 56) + (-1ULL << 52)
			+ (14ULL << 26) - (57ULL << 26) + (13ULL << 16));
	      bfd_put_32 (input_bfd, pinsn >> 32,
			  contents + rel->r_offset);
	      bfd_put_32 (input_bfd, pinsn & 0xffffffff,
			  contents + rel->r_offset + 4);
	      r_type = R_PPC64_TPREL34;
	      rel->r_info = ELF64_R_INFO (r_symndx, r_type);
	    }
	  break;

	case R_PPC64_TLS:
	  if ((tls_mask & TLS_TLS) != 0
	      && (tls_mask & TLS_TPREL) == 0
	      && offset_in_range (input_section, rel->r_offset & ~3, 4))
	    {
	      insn = bfd_get_32 (input_bfd, contents + (rel->r_offset & ~3));
	      insn = _bfd_elf_ppc_at_tls_transform (insn, 13);
	      if (insn == 0)
		break;
	      if ((rel->r_offset & 3) == 0)
		{
		  bfd_put_32 (input_bfd, insn, contents + rel->r_offset);
		  /* Was PPC64_TLS which sits on insn boundary, now
		     PPC64_TPREL16_LO which is at low-order half-word.  */
		  rel->r_offset += d_offset;
		  r_type = R_PPC64_TPREL16_LO;
		  if (toc_symndx != 0)
		    {
		      rel->r_info = ELF64_R_INFO (toc_symndx, r_type);
		      rel->r_addend = toc_addend;
		      /* We changed the symbol.  Start over in order to
			 get h, sym, sec etc. right.  */
		      goto again;
		    }
		  else
		    rel->r_info = ELF64_R_INFO (r_symndx, r_type);
		}
	      else if ((rel->r_offset & 3) == 1)
		{
		  /* For pcrel IE to LE we already have the full
		     offset and thus don't need an addi here.  A nop
		     or mr will do.  */
		  if ((insn & (0x3fu << 26)) == 14 << 26)
		    {
		      /* Extract regs from addi rt,ra,si.  */
		      unsigned int rt = (insn >> 21) & 0x1f;
		      unsigned int ra = (insn >> 16) & 0x1f;
		      if (rt == ra)
			insn = NOP;
		      else
			{
			  /* Build or ra,rs,rb with rb==rs, ie. mr ra,rs.  */
			  insn = (rt << 16) | (ra << 21) | (ra << 11);
			  insn |= (31u << 26) | (444u << 1);
			}
		    }
		  bfd_put_32 (input_bfd, insn, contents + rel->r_offset - 1);
		}
	    }
	  break;

	case R_PPC64_GOT_TLSGD16_HI:
	case R_PPC64_GOT_TLSGD16_HA:
	  tls_gd = TLS_GDIE;
	  if ((tls_mask & TLS_TLS) != 0 && (tls_mask & TLS_GD) == 0
	      && offset_in_range (input_section, rel->r_offset & ~3, 4))
	    goto tls_gdld_hi;
	  break;

	case R_PPC64_GOT_TLSLD16_HI:
	case R_PPC64_GOT_TLSLD16_HA:
	  if ((tls_mask & TLS_TLS) != 0 && (tls_mask & TLS_LD) == 0
	      && offset_in_range (input_section, rel->r_offset & ~3, 4))
	    {
	    tls_gdld_hi:
	      if ((tls_mask & tls_gd) != 0)
		r_type = (((r_type - (R_PPC64_GOT_TLSGD16 & 3)) & 3)
			  + R_PPC64_GOT_TPREL16_DS);
	      else
		{
		  rel->r_offset -= d_offset;
		  bfd_put_32 (input_bfd, NOP, contents + rel->r_offset);
		  r_type = R_PPC64_NONE;
		}
	      rel->r_info = ELF64_R_INFO (r_symndx, r_type);
	    }
	  break;

	case R_PPC64_GOT_TLSGD16:
	case R_PPC64_GOT_TLSGD16_LO:
	  tls_gd = TLS_GDIE;
	  if ((tls_mask & TLS_TLS) != 0 && (tls_mask & TLS_GD) == 0
	      && offset_in_range (input_section, rel->r_offset & ~3, 4))
	    goto tls_ldgd_opt;
	  break;

	case R_PPC64_GOT_TLSLD16:
	case R_PPC64_GOT_TLSLD16_LO:
	  if ((tls_mask & TLS_TLS) != 0 && (tls_mask & TLS_LD) == 0
	      && offset_in_range (input_section, rel->r_offset & ~3, 4))
	    {
	      unsigned int insn1, insn2;

	    tls_ldgd_opt:
	      offset = (bfd_vma) -1;
	      /* If not using the newer R_PPC64_TLSGD/LD to mark
		 __tls_get_addr calls, we must trust that the call
		 stays with its arg setup insns, ie. that the next
		 reloc is the __tls_get_addr call associated with
		 the current reloc.  Edit both insns.  */
	      if (input_section->nomark_tls_get_addr
		  && rel + 1 < relend
		  && branch_reloc_hash_match (input_bfd, rel + 1,
					      htab->tls_get_addr_fd,
					      htab->tga_desc_fd,
					      htab->tls_get_addr,
					      htab->tga_desc))
		offset = rel[1].r_offset;
	      /* We read the low GOT_TLS (or TOC16) insn because we
		 need to keep the destination reg.  It may be
		 something other than the usual r3, and moved to r3
		 before the call by intervening code.  */
	      insn1 = bfd_get_32 (input_bfd,
				  contents + rel->r_offset - d_offset);
	      if ((tls_mask & tls_gd) != 0)
		{
		  /* IE */
		  insn1 &= (0x1f << 21) | (0x1f << 16);
		  insn1 |= 58u << 26;	/* ld */
		  insn2 = 0x7c636a14;	/* add 3,3,13 */
		  if (offset != (bfd_vma) -1)
		    rel[1].r_info = ELF64_R_INFO (STN_UNDEF, R_PPC64_NONE);
		  if (r_type == R_PPC64_TOC16
		      || r_type == R_PPC64_TOC16_LO)
		    r_type += R_PPC64_TOC16_DS - R_PPC64_TOC16;
		  else
		    r_type = (((r_type - (R_PPC64_GOT_TLSGD16 & 1)) & 1)
			      + R_PPC64_GOT_TPREL16_DS);
		  rel->r_info = ELF64_R_INFO (r_symndx, r_type);
		}
	      else
		{
		  /* LE */
		  insn1 &= 0x1f << 21;
		  insn1 |= 0x3c0d0000;	/* addis r,13,0 */
		  insn2 = 0x38630000;	/* addi 3,3,0 */
		  if (tls_gd == 0)
		    {
		      /* Was an LD reloc.  */
		      r_symndx = STN_UNDEF;
		      rel->r_addend = htab->elf.tls_sec->vma + DTP_OFFSET;
		    }
		  else if (toc_symndx != 0)
		    {
		      r_symndx = toc_symndx;
		      rel->r_addend = toc_addend;
		    }
		  r_type = R_PPC64_TPREL16_HA;
		  rel->r_info = ELF64_R_INFO (r_symndx, r_type);
		  if (offset != (bfd_vma) -1)
		    {
		      rel[1].r_info = ELF64_R_INFO (r_symndx,
						    R_PPC64_TPREL16_LO);
		      rel[1].r_offset = offset + d_offset;
		      rel[1].r_addend = rel->r_addend;
		    }
		}
	      bfd_put_32 (input_bfd, insn1,
			  contents + rel->r_offset - d_offset);
	      if (offset != (bfd_vma) -1
		  && offset_in_range (input_section, offset, 4))
		{
		  bfd_put_32 (input_bfd, insn2, contents + offset);
		  if (offset_in_range (input_section, offset + 4, 4))
		    {
		      insn2 = bfd_get_32 (input_bfd, contents + offset + 4);
		      if (insn2 == LD_R2_0R1 + STK_TOC (htab))
			bfd_put_32 (input_bfd, NOP, contents + offset + 4);
		    }
		}
	      if ((tls_mask & tls_gd) == 0
		  && (tls_gd == 0 || toc_symndx != 0))
		{
		  /* We changed the symbol.  Start over in order
		     to get h, sym, sec etc. right.  */
		  goto again;
		}
	    }
	  break;

	case R_PPC64_GOT_TLSGD_PCREL34:
	  if ((tls_mask & TLS_TLS) != 0 && (tls_mask & TLS_GD) == 0
	      && offset_in_range (input_section, rel->r_offset, 8))
	    {
	      pinsn = bfd_get_32 (input_bfd, contents + rel->r_offset);
	      pinsn <<= 32;
	      pinsn |= bfd_get_32 (input_bfd, contents + rel->r_offset + 4);
	      if ((tls_mask & TLS_GDIE) != 0)
		{
		  /* IE, pla -> pld  */
		  pinsn += (-2ULL << 56) + (57ULL << 26) - (14ULL << 26);
		  r_type = R_PPC64_GOT_TPREL_PCREL34;
		}
	      else
		{
		  /* LE, pla pcrel -> paddi r13  */
		  pinsn += (-1ULL << 52) + (13ULL << 16);
		  r_type = R_PPC64_TPREL34;
		}
	      rel->r_info = ELF64_R_INFO (r_symndx, r_type);
	      bfd_put_32 (input_bfd, pinsn >> 32,
			  contents + rel->r_offset);
	      bfd_put_32 (input_bfd, pinsn & 0xffffffff,
			  contents + rel->r_offset + 4);
	    }
	  break;

	case R_PPC64_GOT_TLSLD_PCREL34:
	  if ((tls_mask & TLS_TLS) != 0 && (tls_mask & TLS_LD) == 0
	      && offset_in_range (input_section, rel->r_offset, 8))
	    {
	      pinsn = bfd_get_32 (input_bfd, contents + rel->r_offset);
	      pinsn <<= 32;
	      pinsn |= bfd_get_32 (input_bfd, contents + rel->r_offset + 4);
	      pinsn += (-1ULL << 52) + (13ULL << 16);
	      bfd_put_32 (input_bfd, pinsn >> 32,
			  contents + rel->r_offset);
	      bfd_put_32 (input_bfd, pinsn & 0xffffffff,
			  contents + rel->r_offset + 4);
	      rel->r_addend = htab->elf.tls_sec->vma + DTP_OFFSET;
	      r_symndx = STN_UNDEF;
	      r_type = R_PPC64_TPREL34;
	      rel->r_info = ELF64_R_INFO (r_symndx, r_type);
	      goto again;
	    }
	  break;

	case R_PPC64_TLSGD:
	  if ((tls_mask & TLS_TLS) != 0 && (tls_mask & TLS_GD) == 0
	      && rel + 1 < relend
	      && offset_in_range (input_section, rel->r_offset,
				  is_8byte_reloc (ELF64_R_TYPE (rel[1].r_info))
				  ? 8 : 4))
	    {
	      unsigned int insn2;
	      enum elf_ppc64_reloc_type r_type1 = ELF64_R_TYPE (rel[1].r_info);

	      offset = rel->r_offset;
	      if (is_plt_seq_reloc (r_type1))
		{
		  bfd_put_32 (output_bfd, NOP, contents + offset);
		  if (r_type1 == R_PPC64_PLT_PCREL34
		      || r_type1 == R_PPC64_PLT_PCREL34_NOTOC)
		    bfd_put_32 (output_bfd, NOP, contents + offset + 4);
		  rel[1].r_info = ELF64_R_INFO (STN_UNDEF, R_PPC64_NONE);
		  break;
		}

	      if (r_type1 == R_PPC64_PLTCALL)
		bfd_put_32 (output_bfd, NOP, contents + offset + 4);

	      if ((tls_mask & TLS_GDIE) != 0)
		{
		  /* IE */
		  r_type = R_PPC64_NONE;
		  insn2 = 0x7c636a14;	/* add 3,3,13 */
		}
	      else
		{
		  /* LE */
		  if (toc_symndx != 0)
		    {
		      r_symndx = toc_symndx;
		      rel->r_addend = toc_addend;
		    }
		  if (r_type1 == R_PPC64_REL24_NOTOC
		      || r_type1 == R_PPC64_REL24_P9NOTOC
		      || r_type1 == R_PPC64_PLTCALL_NOTOC)
		    {
		      r_type = R_PPC64_NONE;
		      insn2 = NOP;
		    }
		  else
		    {
		      rel->r_offset = offset + d_offset;
		      r_type = R_PPC64_TPREL16_LO;
		      insn2 = 0x38630000;	/* addi 3,3,0 */
		    }
		}
	      rel->r_info = ELF64_R_INFO (r_symndx, r_type);
	      /* Zap the reloc on the _tls_get_addr call too.  */
	      BFD_ASSERT (offset == rel[1].r_offset);
	      rel[1].r_info = ELF64_R_INFO (STN_UNDEF, R_PPC64_NONE);
	      bfd_put_32 (input_bfd, insn2, contents + offset);
	      if ((tls_mask & TLS_GDIE) == 0
		  && toc_symndx != 0
		  && r_type != R_PPC64_NONE)
		goto again;
	    }
	  break;

	case R_PPC64_TLSLD:
	  if ((tls_mask & TLS_TLS) != 0 && (tls_mask & TLS_LD) == 0
	      && rel + 1 < relend
	      && offset_in_range (input_section, rel->r_offset,
				  is_8byte_reloc (ELF64_R_TYPE (rel[1].r_info))
				  ? 8 : 4))
	    {
	      unsigned int insn2;
	      enum elf_ppc64_reloc_type r_type1 = ELF64_R_TYPE (rel[1].r_info);

	      offset = rel->r_offset;
	      if (is_plt_seq_reloc (r_type1))
		{
		  bfd_put_32 (output_bfd, NOP, contents + offset);
		  if (r_type1 == R_PPC64_PLT_PCREL34
		      || r_type1 == R_PPC64_PLT_PCREL34_NOTOC)
		    bfd_put_32 (output_bfd, NOP, contents + offset + 4);
		  rel[1].r_info = ELF64_R_INFO (STN_UNDEF, R_PPC64_NONE);
		  break;
		}

	      if (r_type1 == R_PPC64_PLTCALL)
		bfd_put_32 (output_bfd, NOP, contents + offset + 4);

	      if (r_type1 == R_PPC64_REL24_NOTOC
		  || r_type1 == R_PPC64_REL24_P9NOTOC
		  || r_type1 == R_PPC64_PLTCALL_NOTOC)
		{
		  r_type = R_PPC64_NONE;
		  insn2 = NOP;
		}
	      else
		{
		  rel->r_offset = offset + d_offset;
		  r_symndx = STN_UNDEF;
		  r_type = R_PPC64_TPREL16_LO;
		  rel->r_addend = htab->elf.tls_sec->vma + DTP_OFFSET;
		  insn2 = 0x38630000;	/* addi 3,3,0 */
		}
	      rel->r_info = ELF64_R_INFO (r_symndx, r_type);
	      /* Zap the reloc on the _tls_get_addr call too.  */
	      BFD_ASSERT (offset == rel[1].r_offset);
	      rel[1].r_info = ELF64_R_INFO (STN_UNDEF, R_PPC64_NONE);
	      bfd_put_32 (input_bfd, insn2, contents + offset);
	      if (r_type != R_PPC64_NONE)
		goto again;
	    }
	  break;

	case R_PPC64_DTPMOD64:
	  if (rel + 1 < relend
	      && rel[1].r_info == ELF64_R_INFO (r_symndx, R_PPC64_DTPREL64)
	      && rel[1].r_offset == rel->r_offset + 8)
	    {
	      if ((tls_mask & TLS_GD) == 0
		  && offset_in_range (input_section, rel->r_offset, 8))
		{
		  rel[1].r_info = ELF64_R_INFO (r_symndx, R_PPC64_NONE);
		  if ((tls_mask & TLS_GDIE) != 0)
		    r_type = R_PPC64_TPREL64;
		  else
		    {
		      bfd_put_64 (output_bfd, 1, contents + rel->r_offset);
		      r_type = R_PPC64_NONE;
		    }
		  rel->r_info = ELF64_R_INFO (r_symndx, r_type);
		}
	    }
	  else
	    {
	      if ((tls_mask & TLS_LD) == 0
		  && offset_in_range (input_section, rel->r_offset, 8))
		{
		  bfd_put_64 (output_bfd, 1, contents + rel->r_offset);
		  r_type = R_PPC64_NONE;
		  rel->r_info = ELF64_R_INFO (r_symndx, r_type);
		}
	    }
	  break;

	case R_PPC64_TPREL64:
	  if ((tls_mask & TLS_TPREL) == 0)
	    {
	      r_type = R_PPC64_NONE;
	      rel->r_info = ELF64_R_INFO (r_symndx, r_type);
	    }
	  break;

	case R_PPC64_ENTRY:
	  relocation = TOCstart + htab->sec_info[input_section->id].toc_off;
	  if (!bfd_link_pic (info)
	      && !info->traditional_format
	      && relocation + 0x80008000 <= 0xffffffff
	      && offset_in_range (input_section, rel->r_offset, 8))
	    {
	      unsigned int insn1, insn2;

	      insn1 = bfd_get_32 (input_bfd, contents + rel->r_offset);
	      insn2 = bfd_get_32 (input_bfd, contents + rel->r_offset + 4);
	      if ((insn1 & ~0xfffc) == LD_R2_0R12
		  && insn2 == ADD_R2_R2_R12)
		{
		  bfd_put_32 (input_bfd,
			      LIS_R2 + PPC_HA (relocation),
			      contents + rel->r_offset);
		  bfd_put_32 (input_bfd,
			      ADDI_R2_R2 + PPC_LO (relocation),
			      contents + rel->r_offset + 4);
		}
	    }
	  else
	    {
	      relocation -= (rel->r_offset
			     + input_section->output_offset
			     + input_section->output_section->vma);
	      if (relocation + 0x80008000 <= 0xffffffff
		  && offset_in_range (input_section, rel->r_offset, 8))
		{
		  unsigned int insn1, insn2;

		  insn1 = bfd_get_32 (input_bfd, contents + rel->r_offset);
		  insn2 = bfd_get_32 (input_bfd, contents + rel->r_offset + 4);
		  if ((insn1 & ~0xfffc) == LD_R2_0R12
		      && insn2 == ADD_R2_R2_R12)
		    {
		      bfd_put_32 (input_bfd,
				  ADDIS_R2_R12 + PPC_HA (relocation),
				  contents + rel->r_offset);
		      bfd_put_32 (input_bfd,
				  ADDI_R2_R2 + PPC_LO (relocation),
				  contents + rel->r_offset + 4);
		    }
		}
	    }
	  break;

	case R_PPC64_REL16_HA:
	  /* If we are generating a non-PIC executable, edit
	     .	0:	addis 2,12,.TOC.-0b@ha
	     .		addi 2,2,.TOC.-0b@l
	     used by ELFv2 global entry points to set up r2, to
	     .		lis 2,.TOC.@ha
	     .		addi 2,2,.TOC.@l
	     if .TOC. is in range.  */
	  if (!bfd_link_pic (info)
	      && !info->traditional_format
	      && !htab->opd_abi
	      && rel->r_addend == d_offset
	      && h != NULL && &h->elf == htab->elf.hgot
	      && rel + 1 < relend
	      && rel[1].r_info == ELF64_R_INFO (r_symndx, R_PPC64_REL16_LO)
	      && rel[1].r_offset == rel->r_offset + 4
	      && rel[1].r_addend == rel->r_addend + 4
	      && relocation + 0x80008000 <= 0xffffffff
	      && offset_in_range (input_section, rel->r_offset - d_offset, 8))
	    {
	      unsigned int insn1, insn2;
	      offset = rel->r_offset - d_offset;
	      insn1 = bfd_get_32 (input_bfd, contents + offset);
	      insn2 = bfd_get_32 (input_bfd, contents + offset + 4);
	      if ((insn1 & 0xffff0000) == ADDIS_R2_R12
		  && (insn2 & 0xffff0000) == ADDI_R2_R2)
		{
		  r_type = R_PPC64_ADDR16_HA;
		  rel->r_info = ELF64_R_INFO (r_symndx, r_type);
		  rel->r_addend -= d_offset;
		  rel[1].r_info = ELF64_R_INFO (r_symndx, R_PPC64_ADDR16_LO);
		  rel[1].r_addend -= d_offset + 4;
		  bfd_put_32 (input_bfd, LIS_R2, contents + offset);
		}
	    }
	  break;
	}

      /* Handle other relocations that tweak non-addend part of insn.  */
      insn = 0;
      max_br_offset = 1 << 25;
      addend = rel->r_addend;
      reloc_dest = DEST_NORMAL;
      switch (r_type)
	{
	default:
	  break;

	case R_PPC64_TOCSAVE:
	  if (relocation + addend == (rel->r_offset
				      + input_section->output_offset
				      + input_section->output_section->vma)
	      && tocsave_find (htab, NO_INSERT,
			       &local_syms, rel, input_bfd)
	      && offset_in_range (input_section, rel->r_offset, 4))
	    {
	      insn = bfd_get_32 (input_bfd, contents + rel->r_offset);
	      if (insn == NOP
		  || insn == CROR_151515 || insn == CROR_313131)
		bfd_put_32 (input_bfd,
			    STD_R2_0R1 + STK_TOC (htab),
			    contents + rel->r_offset);
	    }
	  break;

	  /* Branch taken prediction relocations.  */
	case R_PPC64_ADDR14_BRTAKEN:
	case R_PPC64_REL14_BRTAKEN:
	  insn = 0x01 << 21; /* 'y' or 't' bit, lowest bit of BO field.  */
	  /* Fall through.  */

	  /* Branch not taken prediction relocations.  */
	case R_PPC64_ADDR14_BRNTAKEN:
	case R_PPC64_REL14_BRNTAKEN:
	  if (!offset_in_range (input_section, rel->r_offset, 4))
	    break;
	  insn |= bfd_get_32 (input_bfd,
			      contents + rel->r_offset) & ~(0x01 << 21);
	  /* Fall through.  */

	case R_PPC64_REL14:
	  max_br_offset = 1 << 15;
	  /* Fall through.  */

	case R_PPC64_REL24:
	case R_PPC64_REL24_NOTOC:
	case R_PPC64_REL24_P9NOTOC:
	case R_PPC64_PLTCALL:
	case R_PPC64_PLTCALL_NOTOC:
	  /* Calls to functions with a different TOC, such as calls to
	     shared objects, need to alter the TOC pointer.  This is
	     done using a linkage stub.  A REL24 branching to these
	     linkage stubs needs to be followed by a nop, as the nop
	     will be replaced with an instruction to restore the TOC
	     base pointer.  */
	  fdh = h;
	  if (h != NULL
	      && h->oh != NULL
	      && h->oh->is_func_descriptor)
	    fdh = ppc_follow_link (h->oh);
	  stub_entry = ppc_get_stub_entry (input_section, sec, fdh, &orig_rel,
					   htab);
	  if ((r_type == R_PPC64_PLTCALL
	       || r_type == R_PPC64_PLTCALL_NOTOC)
	      && stub_entry != NULL
	      && stub_entry->type.main == ppc_stub_plt_call)
	    stub_entry = NULL;

	  if (stub_entry != NULL
	      && (stub_entry->type.main == ppc_stub_plt_call
		  || stub_entry->type.r2save))
	    {
	      bool can_plt_call = false;

	      if (r_type == R_PPC64_REL24_NOTOC
		  || r_type == R_PPC64_REL24_P9NOTOC)
		{
		  /* NOTOC calls don't need to restore r2.  */
		  can_plt_call = true;
		}
	      else if (stub_entry->type.main == ppc_stub_plt_call
		       && !htab->opd_abi
		       && htab->params->plt_localentry0 != 0
		       && h != NULL
		       && is_elfv2_localentry0 (&h->elf))
		{
		  /* The function doesn't use or change r2.  */
		  can_plt_call = true;
		}

	      /* All of these stubs may modify r2, so there must be a
		 branch and link followed by a nop.  The nop is
		 replaced by an insn to restore r2.  */
	      else if (offset_in_range (input_section, rel->r_offset, 8))
		{
		  unsigned long br;

		  br = bfd_get_32 (input_bfd,
				   contents + rel->r_offset);
		  if ((br & 1) != 0)
		    {
		      unsigned long nop;

		      nop = bfd_get_32 (input_bfd,
					contents + rel->r_offset + 4);
		      if (nop == LD_R2_0R1 + STK_TOC (htab))
			can_plt_call = true;
		      else if (nop == NOP
			       || nop == CROR_151515
			       || nop == CROR_313131)
			{
			  if (h != NULL
			      && is_tls_get_addr (&h->elf, htab)
			      && htab->params->tls_get_addr_opt)
			    {
			      /* Special stub used, leave nop alone.  */
			    }
			  else
			    bfd_put_32 (input_bfd,
					LD_R2_0R1 + STK_TOC (htab),
					contents + rel->r_offset + 4);
			  can_plt_call = true;
			}
		    }
		}

	      if (!can_plt_call && h != NULL)
		{
		  const char *name = h->elf.root.root.string;

		  if (*name == '.')
		    ++name;

		  if (startswith (name, "__libc_start_main")
		      && (name[17] == 0 || name[17] == '@'))
		    {
		      /* Allow crt1 branch to go via a toc adjusting
			 stub.  Other calls that never return could do
			 the same, if we could detect such.  */
		      can_plt_call = true;
		    }
		}

	      if (!can_plt_call)
		{
		  /* g++ as of 20130507 emits self-calls without a
		     following nop.  This is arguably wrong since we
		     have conflicting information.  On the one hand a
		     global symbol and on the other a local call
		     sequence, but don't error for this special case.
		     It isn't possible to cheaply verify we have
		     exactly such a call.  Allow all calls to the same
		     section.  */
		  asection *code_sec = sec;

		  if (get_opd_info (sec) != NULL)
		    {
		      bfd_vma off = (relocation + addend
				     - sec->output_section->vma
				     - sec->output_offset);

		      opd_entry_value (sec, off, &code_sec, NULL, false);
		    }
		  if (code_sec == input_section)
		    can_plt_call = true;
		}

	      if (!can_plt_call)
		{
		  if (stub_entry->type.main == ppc_stub_plt_call)
		    info->callbacks->einfo
		      /* xgettext:c-format */
		      (_("%H: call to `%pT' lacks nop, can't restore toc; "
			 "(plt call stub)\n"),
		       input_bfd, input_section, rel->r_offset, sym_name);
		  else
		    info->callbacks->einfo
		      /* xgettext:c-format */
		      (_("%H: call to `%pT' lacks nop, can't restore toc; "
			 "(toc save/adjust stub)\n"),
		       input_bfd, input_section, rel->r_offset, sym_name);

		  bfd_set_error (bfd_error_bad_value);
		  ret = false;
		}

	      if (can_plt_call
		  && stub_entry->type.main == ppc_stub_plt_call)
		unresolved_reloc = false;
	    }

	  if ((stub_entry == NULL
	       || stub_entry->type.main == ppc_stub_long_branch
	       || stub_entry->type.main == ppc_stub_plt_branch)
	      && get_opd_info (sec) != NULL)
	    {
	      /* The branch destination is the value of the opd entry. */
	      bfd_vma off = (relocation + addend
			     - sec->output_section->vma
			     - sec->output_offset);
	      bfd_vma dest = opd_entry_value (sec, off, NULL, NULL, false);
	      if (dest != (bfd_vma) -1)
		{
		  relocation = dest;
		  addend = 0;
		  reloc_dest = DEST_OPD;
		}
	    }

	  /* If the branch is out of reach we ought to have a long
	     branch stub.  */
	  from = (rel->r_offset
		  + input_section->output_offset
		  + input_section->output_section->vma);

	  relocation += PPC64_LOCAL_ENTRY_OFFSET (fdh
						  ? fdh->elf.other
						  : sym->st_other);

	  if (stub_entry != NULL
	      && (stub_entry->type.main == ppc_stub_long_branch
		  || stub_entry->type.main == ppc_stub_plt_branch))
	    {
	      if (stub_entry->type.sub == ppc_stub_toc
		  && !stub_entry->type.r2save
		  && (r_type == R_PPC64_ADDR14_BRTAKEN
		      || r_type == R_PPC64_ADDR14_BRNTAKEN
		      || (relocation + addend - from + max_br_offset
			  < 2 * max_br_offset)))
		/* Don't use the stub if this branch is in range.  */
		stub_entry = NULL;

	      if (stub_entry != NULL
		  && stub_entry->type.sub >= ppc_stub_notoc
		  && ((r_type != R_PPC64_REL24_NOTOC
		       && r_type != R_PPC64_REL24_P9NOTOC)
		      || ((fdh ? fdh->elf.other : sym->st_other)
			  & STO_PPC64_LOCAL_MASK) <= 1 << STO_PPC64_LOCAL_BIT)
		  && (relocation + addend - from + max_br_offset
		      < 2 * max_br_offset))
		stub_entry = NULL;

	      if (stub_entry != NULL
		  && stub_entry->type.r2save
		  && (r_type == R_PPC64_REL24_NOTOC
		      || r_type == R_PPC64_REL24_P9NOTOC)
		  && (relocation + addend - from + max_br_offset
		      < 2 * max_br_offset))
		stub_entry = NULL;
	    }

	  if (stub_entry != NULL)
	    {
	      /* Munge up the value and addend so that we call the stub
		 rather than the procedure directly.  */
	      asection *stub_sec = stub_entry->group->stub_sec;

	      if (stub_entry->type.main == ppc_stub_save_res)
		relocation += (stub_sec->output_offset
			       + stub_sec->output_section->vma
			       + stub_sec->size - htab->sfpr->size
			       - htab->sfpr->output_offset
			       - htab->sfpr->output_section->vma);
	      else
		relocation = (stub_entry->stub_offset
			      + stub_sec->output_offset
			      + stub_sec->output_section->vma);
	      addend = 0;
	      reloc_dest = DEST_STUB;

	      if (((stub_entry->type.r2save
		    && (r_type == R_PPC64_REL24_NOTOC
			|| r_type == R_PPC64_REL24_P9NOTOC))
		   || ((stub_entry->type.main == ppc_stub_plt_call
			&& (ALWAYS_EMIT_R2SAVE || stub_entry->type.r2save))
		       && rel + 1 < relend
		       && rel[1].r_offset == rel->r_offset + 4
		       && ELF64_R_TYPE (rel[1].r_info) == R_PPC64_TOCSAVE))
		  && !(stub_entry->type.main == ppc_stub_plt_call
		       && htab->params->tls_get_addr_opt
		       && h != NULL
		       && is_tls_get_addr (&h->elf, htab)))
		{
		  /* Skip over the r2 store at the start of the stub.  */
		  relocation += 4;
		}

	      if ((r_type == R_PPC64_REL24_NOTOC
		   || r_type == R_PPC64_REL24_P9NOTOC)
		  && stub_entry->type.main == ppc_stub_plt_call
		  && stub_entry->type.sub >= ppc_stub_notoc)
		htab->notoc_plt = 1;
	    }

	  if (insn != 0)
	    {
	      if (is_isa_v2)
		{
		  /* Set 'a' bit.  This is 0b00010 in BO field for branch
		     on CR(BI) insns (BO == 001at or 011at), and 0b01000
		     for branch on CTR insns (BO == 1a00t or 1a01t).  */
		  if ((insn & (0x14 << 21)) == (0x04 << 21))
		    insn |= 0x02 << 21;
		  else if ((insn & (0x14 << 21)) == (0x10 << 21))
		    insn |= 0x08 << 21;
		  else
		    break;
		}
	      else
		{
		  /* Invert 'y' bit if not the default.  */
		  if ((bfd_signed_vma) (relocation + addend - from) < 0)
		    insn ^= 0x01 << 21;
		}

	      bfd_put_32 (input_bfd, insn, contents + rel->r_offset);
	    }

	  /* NOP out calls to undefined weak functions.
	     We can thus call a weak function without first
	     checking whether the function is defined.  */
	  else if (h != NULL
		   && h->elf.root.type == bfd_link_hash_undefweak
		   && h->elf.dynindx == -1
		   && (r_type == R_PPC64_REL24
		       || r_type == R_PPC64_REL24_NOTOC
		       || r_type == R_PPC64_REL24_P9NOTOC)
		   && relocation == 0
		   && addend == 0
		   && offset_in_range (input_section, rel->r_offset, 4))
	    {
	      bfd_put_32 (input_bfd, NOP, contents + rel->r_offset);
	      goto copy_reloc;
	    }
	  break;

	case R_PPC64_GOT16_DS:
	  if ((h ? h->elf.type : ELF_ST_TYPE (sym->st_info)) == STT_GNU_IFUNC
	      || (bfd_link_pic (info)
		  && sec == bfd_abs_section_ptr)
	      || !htab->do_toc_opt)
	    break;
	  from = TOCstart + htab->sec_info[input_section->id].toc_off;
	  if (relocation + addend - from + 0x8000 < 0x10000
	      && sec != NULL
	      && sec->output_section != NULL
	      && !discarded_section (sec)
	      && (h == NULL || SYMBOL_REFERENCES_LOCAL (info, &h->elf))
	      && offset_in_range (input_section, rel->r_offset & ~3, 4))
	    {
	      insn = bfd_get_32 (input_bfd, contents + (rel->r_offset & ~3));
	      if ((insn & (0x3fu << 26 | 0x3)) == 58u << 26 /* ld */)
		{
		  insn += (14u << 26) - (58u << 26);
		  bfd_put_32 (input_bfd, insn, contents + (rel->r_offset & ~3));
		  r_type = R_PPC64_TOC16;
		  rel->r_info = ELF64_R_INFO (r_symndx, r_type);
		}
	    }
	  break;

	case R_PPC64_GOT16_LO_DS:
	case R_PPC64_GOT16_HA:
	  if ((h ? h->elf.type : ELF_ST_TYPE (sym->st_info)) == STT_GNU_IFUNC
	      || (bfd_link_pic (info)
		  && sec == bfd_abs_section_ptr)
	      || !htab->do_toc_opt)
	    break;
	  from = TOCstart + htab->sec_info[input_section->id].toc_off;
	  if (relocation + addend - from + 0x80008000ULL < 0x100000000ULL
	      && sec != NULL
	      && sec->output_section != NULL
	      && !discarded_section (sec)
	      && (h == NULL || SYMBOL_REFERENCES_LOCAL (info, &h->elf))
	      && offset_in_range (input_section, rel->r_offset & ~3, 4))
	    {
	      insn = bfd_get_32 (input_bfd, contents + (rel->r_offset & ~3));
	      if (r_type == R_PPC64_GOT16_LO_DS
		  && (insn & (0x3fu << 26 | 0x3)) == 58u << 26 /* ld */)
		{
		  insn += (14u << 26) - (58u << 26);
		  bfd_put_32 (input_bfd, insn, contents + (rel->r_offset & ~3));
		  r_type = R_PPC64_TOC16_LO;
		  rel->r_info = ELF64_R_INFO (r_symndx, r_type);
		}
	      else if (r_type == R_PPC64_GOT16_HA
		       && (insn & (0x3fu << 26)) == 15u << 26 /* addis */)
		{
		  r_type = R_PPC64_TOC16_HA;
		  rel->r_info = ELF64_R_INFO (r_symndx, r_type);
		}
	    }
	  break;

	case R_PPC64_GOT_PCREL34:
	  if ((h ? h->elf.type : ELF_ST_TYPE (sym->st_info)) == STT_GNU_IFUNC
	      || (bfd_link_pic (info)
		  && sec == bfd_abs_section_ptr)
	      || !htab->do_toc_opt)
	    break;
	  from = (rel->r_offset
		  + input_section->output_section->vma
		  + input_section->output_offset);
	  if (!(relocation - from + (1ULL << 33) < 1ULL << 34
		&& sec != NULL
		&& sec->output_section != NULL
		&& !discarded_section (sec)
		&& (h == NULL || SYMBOL_REFERENCES_LOCAL (info, &h->elf))
		&& offset_in_range (input_section, rel->r_offset, 8)))
	    break;

	  offset = rel->r_offset;
	  pinsn = bfd_get_32 (input_bfd, contents + offset);
	  pinsn <<= 32;
	  pinsn |= bfd_get_32 (input_bfd, contents + offset + 4);
	  if ((pinsn & ((-1ULL << 50) | (63ULL << 26)))
	      != ((1ULL << 58) | (1ULL << 52) | (57ULL << 26) /* pld */))
	    break;

	  /* Replace with paddi.  */
	  pinsn += (2ULL << 56) + (14ULL << 26) - (57ULL << 26);
	  r_type = R_PPC64_PCREL34;
	  rel->r_info = ELF64_R_INFO (r_symndx, r_type);
	  bfd_put_32 (input_bfd, pinsn >> 32, contents + offset);
	  bfd_put_32 (input_bfd, pinsn, contents + offset + 4);
	  /* Fall through.  */

	case R_PPC64_PCREL34:
	  if (!htab->params->no_pcrel_opt
	      && rel + 1 < relend
	      && rel[1].r_offset == rel->r_offset
	      && rel[1].r_info == ELF64_R_INFO (0, R_PPC64_PCREL_OPT)
	      && (h == NULL || SYMBOL_REFERENCES_LOCAL (info, &h->elf))
	      && offset_in_range (input_section, rel->r_offset, 8))
	    {
	      offset = rel->r_offset;
	      pinsn = bfd_get_32 (input_bfd, contents + offset);
	      pinsn <<= 32;
	      pinsn |= bfd_get_32 (input_bfd, contents + offset + 4);
	      if ((pinsn & ((-1ULL << 50) | (63ULL << 26)))
		   == ((1ULL << 58) | (2ULL << 56) | (1ULL << 52)
		       | (14ULL << 26) /* paddi */))
		{
		  bfd_vma off2 = rel[1].r_addend;
		  if (off2 == 0)
		    /* zero means next insn.  */
		    off2 = 8;
		  off2 += offset;
		  if (offset_in_range (input_section, off2, 4))
		    {
		      uint64_t pinsn2;
		      bfd_signed_vma addend_off;
		      pinsn2 = bfd_get_32 (input_bfd, contents + off2);
		      pinsn2 <<= 32;
		      if ((pinsn2 & (63ULL << 58)) == 1ULL << 58)
			{
			  if (!offset_in_range (input_section, off2, 8))
			    break;
			  pinsn2 |= bfd_get_32 (input_bfd,
						contents + off2 + 4);
			}
		      if (xlate_pcrel_opt (&pinsn, &pinsn2, &addend_off))
			{
			  addend += addend_off;
			  rel->r_addend = addend;
			  bfd_put_32 (input_bfd, pinsn >> 32,
				      contents + offset);
			  bfd_put_32 (input_bfd, pinsn,
				      contents + offset + 4);
			  bfd_put_32 (input_bfd, pinsn2 >> 32,
				      contents + off2);
			  if ((pinsn2 & (63ULL << 58)) == 1ULL << 58)
			    bfd_put_32 (input_bfd, pinsn2,
					contents + off2 + 4);
			}
		    }
		}
	    }
	  break;
	}

      tls_type = 0;
      save_unresolved_reloc = unresolved_reloc;
      switch (r_type)
	{
	default:
	  /* xgettext:c-format */
	  _bfd_error_handler (_("%pB: %s unsupported"),
			      input_bfd, ppc64_elf_howto_table[r_type]->name);

	  bfd_set_error (bfd_error_bad_value);
	  ret = false;
	  goto copy_reloc;

	case R_PPC64_NONE:
	case R_PPC64_TLS:
	case R_PPC64_TLSGD:
	case R_PPC64_TLSLD:
	case R_PPC64_TOCSAVE:
	case R_PPC64_GNU_VTINHERIT:
	case R_PPC64_GNU_VTENTRY:
	case R_PPC64_ENTRY:
	case R_PPC64_PCREL_OPT:
	  goto copy_reloc;

	  /* GOT16 relocations.  Like an ADDR16 using the symbol's
	     address in the GOT as relocation value instead of the
	     symbol's value itself.  Also, create a GOT entry for the
	     symbol and put the symbol value there.  */
	case R_PPC64_GOT_TLSGD16:
	case R_PPC64_GOT_TLSGD16_LO:
	case R_PPC64_GOT_TLSGD16_HI:
	case R_PPC64_GOT_TLSGD16_HA:
	case R_PPC64_GOT_TLSGD_PCREL34:
	  tls_type = TLS_TLS | TLS_GD;
	  goto dogot;

	case R_PPC64_GOT_TLSLD16:
	case R_PPC64_GOT_TLSLD16_LO:
	case R_PPC64_GOT_TLSLD16_HI:
	case R_PPC64_GOT_TLSLD16_HA:
	case R_PPC64_GOT_TLSLD_PCREL34:
	  tls_type = TLS_TLS | TLS_LD;
	  goto dogot;

	case R_PPC64_GOT_TPREL16_DS:
	case R_PPC64_GOT_TPREL16_LO_DS:
	case R_PPC64_GOT_TPREL16_HI:
	case R_PPC64_GOT_TPREL16_HA:
	case R_PPC64_GOT_TPREL_PCREL34:
	  tls_type = TLS_TLS | TLS_TPREL;
	  goto dogot;

	case R_PPC64_GOT_DTPREL16_DS:
	case R_PPC64_GOT_DTPREL16_LO_DS:
	case R_PPC64_GOT_DTPREL16_HI:
	case R_PPC64_GOT_DTPREL16_HA:
	case R_PPC64_GOT_DTPREL_PCREL34:
	  tls_type = TLS_TLS | TLS_DTPREL;
	  goto dogot;

	case R_PPC64_GOT16:
	case R_PPC64_GOT16_LO:
	case R_PPC64_GOT16_HI:
	case R_PPC64_GOT16_HA:
	case R_PPC64_GOT16_DS:
	case R_PPC64_GOT16_LO_DS:
	case R_PPC64_GOT_PCREL34:
	dogot:
	  {
	    /* Relocation is to the entry for this symbol in the global
	       offset table.  */
	    asection *got;
	    bfd_vma *offp;
	    bfd_vma off;
	    unsigned long indx = 0;
	    struct got_entry *ent;

	    if (tls_type == (TLS_TLS | TLS_LD)
		&& (h == NULL || SYMBOL_REFERENCES_LOCAL (info, &h->elf)))
	      ent = ppc64_tlsld_got (input_bfd);
	    else
	      {
		if (h != NULL)
		  {
		    if (!htab->elf.dynamic_sections_created
			|| h->elf.dynindx == -1
			|| SYMBOL_REFERENCES_LOCAL (info, &h->elf)
			|| UNDEFWEAK_NO_DYNAMIC_RELOC (info, &h->elf))
		      /* This is actually a static link, or it is a
			 -Bsymbolic link and the symbol is defined
			 locally, or the symbol was forced to be local
			 because of a version file.  */
		      ;
		    else
		      {
			indx = h->elf.dynindx;
			unresolved_reloc = false;
		      }
		    ent = h->elf.got.glist;
		  }
		else
		  {
		    if (local_got_ents == NULL)
		      abort ();
		    ent = local_got_ents[r_symndx];
		  }

		for (; ent != NULL; ent = ent->next)
		  if (ent->addend == orig_rel.r_addend
		      && ent->owner == input_bfd
		      && ent->tls_type == tls_type)
		    break;
	      }

	    if (ent == NULL)
	      abort ();
	    if (ent->is_indirect)
	      ent = ent->got.ent;
	    offp = &ent->got.offset;
	    got = ppc64_elf_tdata (ent->owner)->got;
	    if (got == NULL)
	      abort ();

	    /* The offset must always be a multiple of 8.  We use the
	       least significant bit to record whether we have already
	       processed this entry.  */
	    off = *offp;
	    if ((off & 1) != 0)
	      off &= ~1;
	    else
	      {
		/* Generate relocs for the dynamic linker, except in
		   the case of TLSLD where we'll use one entry per
		   module.  */
		asection *relgot;
		bool ifunc;

		*offp = off | 1;
		relgot = NULL;
		ifunc = (h != NULL
			 ? h->elf.type == STT_GNU_IFUNC
			 : ELF_ST_TYPE (sym->st_info) == STT_GNU_IFUNC);
		if (ifunc)
		  {
		    relgot = htab->elf.irelplt;
		    if (indx == 0 || is_static_defined (&h->elf))
		      htab->elf.ifunc_resolvers = true;
		  }
		else if (indx != 0
			 || (bfd_link_pic (info)
			     && (h == NULL
				 || !UNDEFWEAK_NO_DYNAMIC_RELOC (info, &h->elf))
			     && !(tls_type != 0
				  && bfd_link_executable (info)
				  && (h == NULL
				      || SYMBOL_REFERENCES_LOCAL (info,
								  &h->elf)))
			     && (h != NULL
				 ? !bfd_is_abs_symbol (&h->elf.root)
				 : sym->st_shndx != SHN_ABS)))

		  relgot = ppc64_elf_tdata (ent->owner)->relgot;
		if (relgot != NULL)
		  {
		    outrel.r_offset = (got->output_section->vma
				       + got->output_offset
				       + off);
		    outrel.r_addend = orig_rel.r_addend;
		    if (tls_type & (TLS_LD | TLS_GD))
		      {
			outrel.r_addend = 0;
			outrel.r_info = ELF64_R_INFO (indx, R_PPC64_DTPMOD64);
			if (tls_type == (TLS_TLS | TLS_GD))
			  {
			    loc = relgot->contents;
			    loc += (relgot->reloc_count++
				    * sizeof (Elf64_External_Rela));
			    bfd_elf64_swap_reloca_out (output_bfd,
						       &outrel, loc);
			    outrel.r_offset += 8;
			    outrel.r_addend = orig_rel.r_addend;
			    outrel.r_info
			      = ELF64_R_INFO (indx, R_PPC64_DTPREL64);
			  }
		      }
		    else if (tls_type == (TLS_TLS | TLS_DTPREL))
		      outrel.r_info = ELF64_R_INFO (indx, R_PPC64_DTPREL64);
		    else if (tls_type == (TLS_TLS | TLS_TPREL))
		      outrel.r_info = ELF64_R_INFO (indx, R_PPC64_TPREL64);
		    else if (indx != 0)
		      outrel.r_info = ELF64_R_INFO (indx, R_PPC64_GLOB_DAT);
		    else
		      {
			if (ifunc)
			  outrel.r_info = ELF64_R_INFO (0, R_PPC64_IRELATIVE);
			else
			  outrel.r_info = ELF64_R_INFO (0, R_PPC64_RELATIVE);

			/* Write the .got section contents for the sake
			   of prelink.  */
			loc = got->contents + off;
			bfd_put_64 (output_bfd, outrel.r_addend + relocation,
				    loc);
		      }

		    if (indx == 0 && tls_type != (TLS_TLS | TLS_LD))
		      {
			outrel.r_addend += relocation;
			if (tls_type & (TLS_GD | TLS_DTPREL | TLS_TPREL))
			  {
			    if (htab->elf.tls_sec == NULL)
			      outrel.r_addend = 0;
			    else
			      outrel.r_addend -= htab->elf.tls_sec->vma;
			  }
		      }
		    if (!(info->enable_dt_relr
			  && ELF64_R_TYPE (outrel.r_info) == R_PPC64_RELATIVE))
		      {
			loc = relgot->contents;
			loc += (relgot->reloc_count++
				* sizeof (Elf64_External_Rela));
			bfd_elf64_swap_reloca_out (output_bfd, &outrel, loc);
		      }
		  }

		/* Init the .got section contents here if we're not
		   emitting a reloc.  */
		else
		  {
		    relocation += orig_rel.r_addend;
		    if (tls_type != 0)
		      {
			if (htab->elf.tls_sec == NULL)
			  relocation = 0;
			else
			  {
			    if (tls_type & TLS_LD)
			      relocation = 0;
			    else
			      relocation -= htab->elf.tls_sec->vma + DTP_OFFSET;
			    if (tls_type & TLS_TPREL)
			      relocation += DTP_OFFSET - TP_OFFSET;
			  }

			if (tls_type & (TLS_GD | TLS_LD))
			  {
			    bfd_put_64 (output_bfd, relocation,
					got->contents + off + 8);
			    relocation = 1;
			  }
		      }
		    bfd_put_64 (output_bfd, relocation,
				got->contents + off);
		  }
	      }

	    if (off >= (bfd_vma) -2)
	      abort ();

	    relocation = got->output_section->vma + got->output_offset + off;
	    addend = 0;
	    if (!(r_type == R_PPC64_GOT_PCREL34
		  || r_type == R_PPC64_GOT_TLSGD_PCREL34
		  || r_type == R_PPC64_GOT_TLSLD_PCREL34
		  || r_type == R_PPC64_GOT_TPREL_PCREL34
		  || r_type == R_PPC64_GOT_DTPREL_PCREL34))
	      addend = -(TOCstart + htab->sec_info[input_section->id].toc_off);
	  }
	  break;

	case R_PPC64_PLT16_HA:
	case R_PPC64_PLT16_HI:
	case R_PPC64_PLT16_LO:
	case R_PPC64_PLT16_LO_DS:
	case R_PPC64_PLT_PCREL34:
	case R_PPC64_PLT_PCREL34_NOTOC:
	case R_PPC64_PLT32:
	case R_PPC64_PLT64:
	case R_PPC64_PLTSEQ:
	case R_PPC64_PLTSEQ_NOTOC:
	case R_PPC64_PLTCALL:
	case R_PPC64_PLTCALL_NOTOC:
	  /* Relocation is to the entry for this symbol in the
	     procedure linkage table.  */
	  unresolved_reloc = true;
	  {
	    struct plt_entry **plt_list = NULL;
	    if (h != NULL)
	      plt_list = &h->elf.plt.plist;
	    else if (local_got_ents != NULL)
	      {
		struct plt_entry **local_plt = (struct plt_entry **)
		  (local_got_ents + symtab_hdr->sh_info);
		plt_list = local_plt + r_symndx;
	      }
	    if (plt_list)
	      {
		struct plt_entry *ent;

		for (ent = *plt_list; ent != NULL; ent = ent->next)
		  if (ent->plt.offset != (bfd_vma) -1
		      && ent->addend == orig_rel.r_addend)
		    {
		      asection *plt;
		      bfd_vma got;

		      plt = htab->elf.splt;
		      if (use_local_plt (info, elf_hash_entry (h)))
			{
			  if (h != NULL
			      ? h->elf.type == STT_GNU_IFUNC
			      : ELF_ST_TYPE (sym->st_info) == STT_GNU_IFUNC)
			    plt = htab->elf.iplt;
			  else
			    plt = htab->pltlocal;
			}
		      relocation = (plt->output_section->vma
				    + plt->output_offset
				    + ent->plt.offset);
		      if (r_type == R_PPC64_PLT16_HA
			  || r_type == R_PPC64_PLT16_HI
			  || r_type == R_PPC64_PLT16_LO
			  || r_type == R_PPC64_PLT16_LO_DS)
			{
			  got = (elf_gp (output_bfd)
				 + htab->sec_info[input_section->id].toc_off);
			  relocation -= got;
			}
		      addend = 0;
		      unresolved_reloc = false;
		      break;
		    }
	      }
	  }
	  break;

	case R_PPC64_TOC:
	  /* Relocation value is TOC base.  */
	  relocation = TOCstart;
	  if (r_symndx == STN_UNDEF)
	    relocation += htab->sec_info[input_section->id].toc_off;
	  else if (unresolved_reloc)
	    ;
	  else if (sec != NULL && sec->id < htab->sec_info_arr_size)
	    relocation += htab->sec_info[sec->id].toc_off;
	  else
	    unresolved_reloc = true;
	  if (unresolved_reloc
	      || (!is_opd
		  && h != NULL
		  && !SYMBOL_REFERENCES_LOCAL (info, &h->elf)))
	    info->callbacks->einfo
	      /* xgettext:c-format */
	      (_("%H: %s against %pT is not supported\n"),
	       input_bfd, input_section, rel->r_offset,
	       ppc64_elf_howto_table[r_type]->name, sym_name);
	  goto dodyn;

	  /* TOC16 relocs.  We want the offset relative to the TOC base,
	     which is the address of the start of the TOC plus 0x8000.
	     The TOC consists of sections .got, .toc, .tocbss, and .plt,
	     in this order.  */
	case R_PPC64_TOC16:
	case R_PPC64_TOC16_LO:
	case R_PPC64_TOC16_HI:
	case R_PPC64_TOC16_DS:
	case R_PPC64_TOC16_LO_DS:
	case R_PPC64_TOC16_HA:
	  addend -= TOCstart + htab->sec_info[input_section->id].toc_off;
	  if (h != NULL)
	    goto dodyn;
	  break;

	  /* Relocate against the beginning of the section.  */
	case R_PPC64_SECTOFF:
	case R_PPC64_SECTOFF_LO:
	case R_PPC64_SECTOFF_HI:
	case R_PPC64_SECTOFF_DS:
	case R_PPC64_SECTOFF_LO_DS:
	case R_PPC64_SECTOFF_HA:
	  if (sec != NULL)
	    addend -= sec->output_section->vma;
	  break;

	case R_PPC64_REL16:
	case R_PPC64_REL16_LO:
	case R_PPC64_REL16_HI:
	case R_PPC64_REL16_HA:
	case R_PPC64_REL16_HIGH:
	case R_PPC64_REL16_HIGHA:
	case R_PPC64_REL16_HIGHER:
	case R_PPC64_REL16_HIGHERA:
	case R_PPC64_REL16_HIGHEST:
	case R_PPC64_REL16_HIGHESTA:
	case R_PPC64_REL16_HIGHER34:
	case R_PPC64_REL16_HIGHERA34:
	case R_PPC64_REL16_HIGHEST34:
	case R_PPC64_REL16_HIGHESTA34:
	case R_PPC64_REL16DX_HA:
	case R_PPC64_REL14:
	case R_PPC64_REL14_BRNTAKEN:
	case R_PPC64_REL14_BRTAKEN:
	case R_PPC64_REL24:
	case R_PPC64_REL24_NOTOC:
	case R_PPC64_REL24_P9NOTOC:
	case R_PPC64_PCREL34:
	case R_PPC64_PCREL28:
	  break;

	case R_PPC64_TPREL16:
	case R_PPC64_TPREL16_LO:
	case R_PPC64_TPREL16_HI:
	case R_PPC64_TPREL16_HA:
	case R_PPC64_TPREL16_DS:
	case R_PPC64_TPREL16_LO_DS:
	case R_PPC64_TPREL16_HIGH:
	case R_PPC64_TPREL16_HIGHA:
	case R_PPC64_TPREL16_HIGHER:
	case R_PPC64_TPREL16_HIGHERA:
	case R_PPC64_TPREL16_HIGHEST:
	case R_PPC64_TPREL16_HIGHESTA:
	  if (h != NULL
	      && h->elf.root.type == bfd_link_hash_undefweak
	      && h->elf.dynindx == -1
	      && offset_in_range (input_section, rel->r_offset - d_offset, 4))
	    {
	      /* Make this relocation against an undefined weak symbol
		 resolve to zero.  This is really just a tweak, since
		 code using weak externs ought to check that they are
		 defined before using them.  */
	      bfd_byte *p = contents + rel->r_offset - d_offset;

	      insn = bfd_get_32 (input_bfd, p);
	      insn = _bfd_elf_ppc_at_tprel_transform (insn, 13);
	      if (insn != 0)
		bfd_put_32 (input_bfd, insn, p);
	      break;
	    }
	  /* Fall through.  */

	case R_PPC64_TPREL34:
	  if (htab->elf.tls_sec != NULL)
	    addend -= htab->elf.tls_sec->vma + TP_OFFSET;
	  /* The TPREL16 relocs shouldn't really be used in shared
	     libs or with non-local symbols as that will result in
	     DT_TEXTREL being set, but support them anyway.  */
	  goto dodyn;

	case R_PPC64_DTPREL16:
	case R_PPC64_DTPREL16_LO:
	case R_PPC64_DTPREL16_HI:
	case R_PPC64_DTPREL16_HA:
	case R_PPC64_DTPREL16_DS:
	case R_PPC64_DTPREL16_LO_DS:
	case R_PPC64_DTPREL16_HIGH:
	case R_PPC64_DTPREL16_HIGHA:
	case R_PPC64_DTPREL16_HIGHER:
	case R_PPC64_DTPREL16_HIGHERA:
	case R_PPC64_DTPREL16_HIGHEST:
	case R_PPC64_DTPREL16_HIGHESTA:
	case R_PPC64_DTPREL34:
	  if (htab->elf.tls_sec != NULL)
	    addend -= htab->elf.tls_sec->vma + DTP_OFFSET;
	  break;

	case R_PPC64_ADDR64_LOCAL:
	  addend += PPC64_LOCAL_ENTRY_OFFSET (h != NULL
					      ? h->elf.other
					      : sym->st_other);
	  break;

	case R_PPC64_DTPMOD64:
	  relocation = 1;
	  addend = 0;
	  goto dodyn;

	case R_PPC64_TPREL64:
	  if (htab->elf.tls_sec != NULL)
	    addend -= htab->elf.tls_sec->vma + TP_OFFSET;
	  goto dodyn;

	case R_PPC64_DTPREL64:
	  if (htab->elf.tls_sec != NULL)
	    addend -= htab->elf.tls_sec->vma + DTP_OFFSET;
	  /* Fall through.  */

	  /* Relocations that may need to be propagated if this is a
	     dynamic object.  */
	case R_PPC64_REL30:
	case R_PPC64_REL32:
	case R_PPC64_REL64:
	case R_PPC64_ADDR14:
	case R_PPC64_ADDR14_BRNTAKEN:
	case R_PPC64_ADDR14_BRTAKEN:
	case R_PPC64_ADDR16:
	case R_PPC64_ADDR16_DS:
	case R_PPC64_ADDR16_HA:
	case R_PPC64_ADDR16_HI:
	case R_PPC64_ADDR16_HIGH:
	case R_PPC64_ADDR16_HIGHA:
	case R_PPC64_ADDR16_HIGHER:
	case R_PPC64_ADDR16_HIGHERA:
	case R_PPC64_ADDR16_HIGHEST:
	case R_PPC64_ADDR16_HIGHESTA:
	case R_PPC64_ADDR16_LO:
	case R_PPC64_ADDR16_LO_DS:
	case R_PPC64_ADDR16_HIGHER34:
	case R_PPC64_ADDR16_HIGHERA34:
	case R_PPC64_ADDR16_HIGHEST34:
	case R_PPC64_ADDR16_HIGHESTA34:
	case R_PPC64_ADDR24:
	case R_PPC64_ADDR32:
	case R_PPC64_ADDR64:
	case R_PPC64_UADDR16:
	case R_PPC64_UADDR32:
	case R_PPC64_UADDR64:
	case R_PPC64_D34:
	case R_PPC64_D34_LO:
	case R_PPC64_D34_HI30:
	case R_PPC64_D34_HA30:
	case R_PPC64_D28:
	dodyn:
	  if ((input_section->flags & SEC_ALLOC) == 0)
	    break;

	  if (NO_OPD_RELOCS && is_opd)
	    break;

	  if (bfd_link_pic (info)
	      ? ((h == NULL
		  || h->elf.dyn_relocs != NULL)
		 && ((h != NULL && pc_dynrelocs (h))
		     || must_be_dyn_reloc (info, r_type)))
	      : (h != NULL
		 ? h->elf.dyn_relocs != NULL
		 : ELF_ST_TYPE (sym->st_info) == STT_GNU_IFUNC))
	    {
	      bool skip, relocate;
	      asection *sreloc;
	      bfd_vma out_off;
	      long indx = 0;

	      /* When generating a dynamic object, these relocations
		 are copied into the output file to be resolved at run
		 time.  */

	      skip = false;
	      relocate = false;

	      out_off = _bfd_elf_section_offset (output_bfd, info,
						 input_section, rel->r_offset);
	      if (out_off == (bfd_vma) -1)
		skip = true;
	      else if (out_off == (bfd_vma) -2)
		skip = true, relocate = true;
	      out_off += (input_section->output_section->vma
			  + input_section->output_offset);
	      outrel.r_offset = out_off;
	      outrel.r_addend = rel->r_addend;

	      /* Optimize unaligned reloc use.  */
	      if ((r_type == R_PPC64_ADDR64 && (out_off & 7) != 0)
		  || (r_type == R_PPC64_UADDR64 && (out_off & 7) == 0))
		r_type ^= R_PPC64_ADDR64 ^ R_PPC64_UADDR64;
	      else if ((r_type == R_PPC64_ADDR32 && (out_off & 3) != 0)
		       || (r_type == R_PPC64_UADDR32 && (out_off & 3) == 0))
		r_type ^= R_PPC64_ADDR32 ^ R_PPC64_UADDR32;
	      else if ((r_type == R_PPC64_ADDR16 && (out_off & 1) != 0)
		       || (r_type == R_PPC64_UADDR16 && (out_off & 1) == 0))
		r_type ^= R_PPC64_ADDR16 ^ R_PPC64_UADDR16;

	      if (skip)
		memset (&outrel, 0, sizeof outrel);
	      else if (h != NULL
		       && !SYMBOL_REFERENCES_LOCAL (info, &h->elf)
		       && !is_opd
		       && r_type != R_PPC64_TOC)
		{
		  indx = h->elf.dynindx;
		  BFD_ASSERT (indx != -1);
		  outrel.r_info = ELF64_R_INFO (indx, r_type);
		}
	      else
		{
		  /* This symbol is local, or marked to become local,
		     or this is an opd section reloc which must point
		     at a local function.  */
		  outrel.r_addend += relocation;
		  if (r_type == R_PPC64_ADDR64 || r_type == R_PPC64_TOC)
		    {
		      if (is_opd && h != NULL)
			{
			  /* Lie about opd entries.  This case occurs
			     when building shared libraries and we
			     reference a function in another shared
			     lib.  The same thing happens for a weak
			     definition in an application that's
			     overridden by a strong definition in a
			     shared lib.  (I believe this is a generic
			     bug in binutils handling of weak syms.)
			     In these cases we won't use the opd
			     entry in this lib.  */
			  unresolved_reloc = false;
			}
		      if (!is_opd
			  && r_type == R_PPC64_ADDR64
			  && (h != NULL
			      ? h->elf.type == STT_GNU_IFUNC
			      : ELF_ST_TYPE (sym->st_info) == STT_GNU_IFUNC))
			outrel.r_info = ELF64_R_INFO (0, R_PPC64_IRELATIVE);
		      else
			{
			  outrel.r_info = ELF64_R_INFO (0, R_PPC64_RELATIVE);

			  /* We need to relocate .opd contents for ld.so.
			     Prelink also wants simple and consistent rules
			     for relocs.  This make all RELATIVE relocs have
			     *r_offset equal to r_addend.  */
			  relocate = true;
			}
		    }
		  else
		    {
		      if (h != NULL
			  ? h->elf.type == STT_GNU_IFUNC
			  : ELF_ST_TYPE (sym->st_info) == STT_GNU_IFUNC)
			{
			  info->callbacks->einfo
			    /* xgettext:c-format */
			    (_("%H: %s for indirect "
			       "function `%pT' unsupported\n"),
			     input_bfd, input_section, rel->r_offset,
			     ppc64_elf_howto_table[r_type]->name,
			     sym_name);
			  ret = false;
			}
		      else if (r_symndx == STN_UNDEF || bfd_is_abs_section (sec))
			;
		      else if (sec == NULL || sec->owner == NULL)
			{
			  bfd_set_error (bfd_error_bad_value);
			  return false;
			}
		      else
			{
			  asection *osec = sec->output_section;

			  if ((osec->flags & SEC_THREAD_LOCAL) != 0)
			    {
			      /* TLS symbol values are relative to the
				 TLS segment.  Dynamic relocations for
				 local TLS symbols therefore can't be
				 reduced to a relocation against their
				 section symbol because it holds the
				 address of the section, not a value
				 relative to the TLS segment.  We could
				 change the .tdata dynamic section symbol
				 to be zero value but STN_UNDEF works
				 and is used elsewhere, eg. for TPREL64
				 GOT relocs against local TLS symbols.  */
			      osec = htab->elf.tls_sec;
			      indx = 0;
			    }
			  else
			    {
			      indx = elf_section_data (osec)->dynindx;
			      if (indx == 0)
				{
				  if ((osec->flags & SEC_READONLY) == 0
				      && htab->elf.data_index_section != NULL)
				    osec = htab->elf.data_index_section;
				  else
				    osec = htab->elf.text_index_section;
				  indx = elf_section_data (osec)->dynindx;
				}
			      BFD_ASSERT (indx != 0);
			    }

			  /* We are turning this relocation into one
			     against a section symbol, so subtract out
			     the output section's address but not the
			     offset of the input section in the output
			     section.  */
			  outrel.r_addend -= osec->vma;
			}

		      outrel.r_info = ELF64_R_INFO (indx, r_type);
		    }
		}

	      if (!(info->enable_dt_relr
		    && ELF64_R_TYPE (outrel.r_info) == R_PPC64_RELATIVE
		    && rel->r_offset % 2 == 0
		    && input_section->alignment_power != 0
		    && ELF64_R_TYPE (orig_rel.r_info) != R_PPC64_UADDR64))
		{
		  sreloc = elf_section_data (input_section)->sreloc;
		  if (h != NULL
		      ? h->elf.type == STT_GNU_IFUNC
		      : ELF_ST_TYPE (sym->st_info) == STT_GNU_IFUNC)
		    {
		      sreloc = htab->elf.irelplt;
		      if (indx == 0 || is_static_defined (&h->elf))
			htab->elf.ifunc_resolvers = true;
		    }
		  if (sreloc == NULL)
		    abort ();

		  if (sreloc->reloc_count * sizeof (Elf64_External_Rela)
		      >= sreloc->size)
		    abort ();
		  loc = sreloc->contents;
		  loc += sreloc->reloc_count++ * sizeof (Elf64_External_Rela);
		  bfd_elf64_swap_reloca_out (output_bfd, &outrel, loc);
		}

	      if (!warned_dynamic
		  && !ppc64_glibc_dynamic_reloc (ELF64_R_TYPE (outrel.r_info)))
		{
		  info->callbacks->einfo
		    /* xgettext:c-format */
		    (_("%X%P: %pB: %s against %pT "
		       "is not supported by glibc as a dynamic relocation\n"),
		     input_bfd,
		     ppc64_elf_howto_table[ELF64_R_TYPE (outrel.r_info)]->name,
		     sym_name);
		  warned_dynamic = true;
		}

	      /* If this reloc is against an external symbol, it will
		 be computed at runtime, so there's no need to do
		 anything now.  However, for the sake of prelink ensure
		 that the section contents are a known value.  */
	      if (!relocate)
		{
		  unresolved_reloc = false;
		  /* The value chosen here is quite arbitrary as ld.so
		     ignores section contents except for the special
		     case of .opd where the contents might be accessed
		     before relocation.  Choose zero, as that won't
		     cause reloc overflow.  */
		  relocation = 0;
		  addend = 0;
		  /* Use *r_offset == r_addend for R_PPC64_ADDR64 relocs
		     to improve backward compatibility with older
		     versions of ld.  */
		  if (r_type == R_PPC64_ADDR64)
		    addend = outrel.r_addend;
		  /* Adjust pc_relative relocs to have zero in *r_offset.  */
		  else if (ppc64_elf_howto_table[r_type]->pc_relative)
		    addend = outrel.r_offset;
		}
	    }
	  break;

	case R_PPC64_COPY:
	case R_PPC64_GLOB_DAT:
	case R_PPC64_JMP_SLOT:
	case R_PPC64_JMP_IREL:
	case R_PPC64_RELATIVE:
	  /* We shouldn't ever see these dynamic relocs in relocatable
	     files.  */
	  /* Fall through.  */

	case R_PPC64_PLTGOT16:
	case R_PPC64_PLTGOT16_DS:
	case R_PPC64_PLTGOT16_HA:
	case R_PPC64_PLTGOT16_HI:
	case R_PPC64_PLTGOT16_LO:
	case R_PPC64_PLTGOT16_LO_DS:
	case R_PPC64_PLTREL32:
	case R_PPC64_PLTREL64:
	  /* These ones haven't been implemented yet.  */

	  info->callbacks->einfo
	    /* xgettext:c-format */
	    (_("%P: %pB: %s is not supported for `%pT'\n"),
	     input_bfd,
	     ppc64_elf_howto_table[r_type]->name, sym_name);

	  bfd_set_error (bfd_error_invalid_operation);
	  ret = false;
	  goto copy_reloc;
	}

      /* Multi-instruction sequences that access the TOC can be
	 optimized, eg. addis ra,r2,0; addi rb,ra,x;
	 to		nop;	       addi rb,r2,x;  */
      switch (r_type)
	{
	default:
	  break;

	case R_PPC64_GOT_TLSLD16_HI:
	case R_PPC64_GOT_TLSGD16_HI:
	case R_PPC64_GOT_TPREL16_HI:
	case R_PPC64_GOT_DTPREL16_HI:
	case R_PPC64_GOT16_HI:
	case R_PPC64_TOC16_HI:
	  /* These relocs would only be useful if building up an
	     offset to later add to r2, perhaps in an indexed
	     addressing mode instruction.  Don't try to optimize.
	     Unfortunately, the possibility of someone building up an
	     offset like this or even with the HA relocs, means that
	     we need to check the high insn when optimizing the low
	     insn.  */
	  break;

	case R_PPC64_PLTCALL_NOTOC:
	  if (!unresolved_reloc)
	    htab->notoc_plt = 1;
	  /* Fall through.  */
	case R_PPC64_PLTCALL:
	  if (unresolved_reloc
	      && offset_in_range (input_section, rel->r_offset,
				  r_type == R_PPC64_PLTCALL ? 8 : 4))
	    {
	      /* No plt entry.  Make this into a direct call.  */
	      bfd_byte *p = contents + rel->r_offset;
	      insn = bfd_get_32 (input_bfd, p);
	      insn &= 1;
	      bfd_put_32 (input_bfd, B_DOT | insn, p);
	      if (r_type == R_PPC64_PLTCALL)
		bfd_put_32 (input_bfd, NOP, p + 4);
	      unresolved_reloc = save_unresolved_reloc;
	      r_type = R_PPC64_REL24;
	    }
	  break;

	case R_PPC64_PLTSEQ_NOTOC:
	case R_PPC64_PLTSEQ:
	  if (unresolved_reloc)
	    {
	      unresolved_reloc = false;
	      goto nop_it;
	    }
	  break;

	case R_PPC64_PLT_PCREL34_NOTOC:
	  if (!unresolved_reloc)
	    htab->notoc_plt = 1;
	  /* Fall through.  */
	case R_PPC64_PLT_PCREL34:
	  if (unresolved_reloc
	      && offset_in_range (input_section, rel->r_offset, 8))
	    {
	      bfd_byte *p = contents + rel->r_offset;
	      bfd_put_32 (input_bfd, PNOP >> 32, p);
	      bfd_put_32 (input_bfd, PNOP, p + 4);
	      unresolved_reloc = false;
	      goto copy_reloc;
	    }
	  break;

	case R_PPC64_PLT16_HA:
	  if (unresolved_reloc)
	    {
	      unresolved_reloc = false;
	      goto nop_it;
	    }
	  /* Fall through.  */
	case R_PPC64_GOT_TLSLD16_HA:
	case R_PPC64_GOT_TLSGD16_HA:
	case R_PPC64_GOT_TPREL16_HA:
	case R_PPC64_GOT_DTPREL16_HA:
	case R_PPC64_GOT16_HA:
	case R_PPC64_TOC16_HA:
	  if (htab->do_toc_opt && relocation + addend + 0x8000 < 0x10000
	      && !ppc64_elf_tdata (input_bfd)->unexpected_toc_insn
	      && !(bfd_link_pic (info)
		   && (h != NULL
		       ? bfd_is_abs_symbol (&h->elf.root)
		       : sec == bfd_abs_section_ptr)))
	    {
	      bfd_byte *p;
	    nop_it:
	      if (offset_in_range (input_section, rel->r_offset & ~3, 4))
		{
		  p = contents + (rel->r_offset & ~3);
		  bfd_put_32 (input_bfd, NOP, p);
		  goto copy_reloc;
		}
	    }
	  break;

	case R_PPC64_PLT16_LO:
	case R_PPC64_PLT16_LO_DS:
	  if (unresolved_reloc)
	    {
	      unresolved_reloc = false;
	      goto nop_it;
	    }
	  /* Fall through.  */
	case R_PPC64_GOT_TLSLD16_LO:
	case R_PPC64_GOT_TLSGD16_LO:
	case R_PPC64_GOT_TPREL16_LO_DS:
	case R_PPC64_GOT_DTPREL16_LO_DS:
	case R_PPC64_GOT16_LO:
	case R_PPC64_GOT16_LO_DS:
	case R_PPC64_TOC16_LO:
	case R_PPC64_TOC16_LO_DS:
	  if (htab->do_toc_opt && relocation + addend + 0x8000 < 0x10000
	      && !ppc64_elf_tdata (input_bfd)->unexpected_toc_insn
	      && !(bfd_link_pic (info)
		   && (h != NULL
		       ? bfd_is_abs_symbol (&h->elf.root)
		       : sec == bfd_abs_section_ptr))
	      && offset_in_range (input_section, rel->r_offset & ~3, 4))
	    {
	      bfd_byte *p = contents + (rel->r_offset & ~3);
	      insn = bfd_get_32 (input_bfd, p);
	      if ((insn & (0x3fu << 26)) == 12u << 26 /* addic */)
		{
		  /* Transform addic to addi when we change reg.  */
		  insn &= ~((0x3fu << 26) | (0x1f << 16));
		  insn |= (14u << 26) | (2 << 16);
		}
	      else
		{
		  insn &= ~(0x1f << 16);
		  insn |= 2 << 16;
		}
	      bfd_put_32 (input_bfd, insn, p);
	    }
	  break;

	case R_PPC64_TPREL16_HA:
	  if (htab->do_tls_opt
	      && relocation + addend + 0x8000 < 0x10000
	      && offset_in_range (input_section, rel->r_offset & ~3, 4))
	    {
	      bfd_byte *p = contents + (rel->r_offset & ~3);
	      bfd_put_32 (input_bfd, NOP, p);
	      goto copy_reloc;
	    }
	  break;

	case R_PPC64_TPREL16_LO:
	case R_PPC64_TPREL16_LO_DS:
	  if (htab->do_tls_opt
	      && relocation + addend + 0x8000 < 0x10000
	      && offset_in_range (input_section, rel->r_offset & ~3, 4))
	    {
	      bfd_byte *p = contents + (rel->r_offset & ~3);
	      insn = bfd_get_32 (input_bfd, p);
	      insn &= ~(0x1f << 16);
	      insn |= 13 << 16;
	      bfd_put_32 (input_bfd, insn, p);
	    }
	  break;
	}

      /* Do any further special processing.  */
      switch (r_type)
	{
	default:
	  break;

	case R_PPC64_REL16_HA:
	case R_PPC64_REL16_HIGHA:
	case R_PPC64_REL16_HIGHERA:
	case R_PPC64_REL16_HIGHESTA:
	case R_PPC64_REL16DX_HA:
	case R_PPC64_ADDR16_HA:
	case R_PPC64_ADDR16_HIGHA:
	case R_PPC64_ADDR16_HIGHERA:
	case R_PPC64_ADDR16_HIGHESTA:
	case R_PPC64_TOC16_HA:
	case R_PPC64_SECTOFF_HA:
	case R_PPC64_TPREL16_HA:
	case R_PPC64_TPREL16_HIGHA:
	case R_PPC64_TPREL16_HIGHERA:
	case R_PPC64_TPREL16_HIGHESTA:
	case R_PPC64_DTPREL16_HA:
	case R_PPC64_DTPREL16_HIGHA:
	case R_PPC64_DTPREL16_HIGHERA:
	case R_PPC64_DTPREL16_HIGHESTA:
	  /* It's just possible that this symbol is a weak symbol
	     that's not actually defined anywhere. In that case,
	     'sec' would be NULL, and we should leave the symbol
	     alone (it will be set to zero elsewhere in the link).  */
	  if (sec == NULL)
	    break;
	  /* Fall through.  */

	case R_PPC64_GOT16_HA:
	case R_PPC64_PLTGOT16_HA:
	case R_PPC64_PLT16_HA:
	case R_PPC64_GOT_TLSGD16_HA:
	case R_PPC64_GOT_TLSLD16_HA:
	case R_PPC64_GOT_TPREL16_HA:
	case R_PPC64_GOT_DTPREL16_HA:
	  /* Add 0x10000 if sign bit in 0:15 is set.
	     Bits 0:15 are not used.  */
	  addend += 0x8000;
	  break;

	case R_PPC64_D34_HA30:
	case R_PPC64_ADDR16_HIGHERA34:
	case R_PPC64_ADDR16_HIGHESTA34:
	case R_PPC64_REL16_HIGHERA34:
	case R_PPC64_REL16_HIGHESTA34:
	  if (sec != NULL)
	    addend += 1ULL << 33;
	  break;

	case R_PPC64_ADDR16_DS:
	case R_PPC64_ADDR16_LO_DS:
	case R_PPC64_GOT16_DS:
	case R_PPC64_GOT16_LO_DS:
	case R_PPC64_PLT16_LO_DS:
	case R_PPC64_SECTOFF_DS:
	case R_PPC64_SECTOFF_LO_DS:
	case R_PPC64_TOC16_DS:
	case R_PPC64_TOC16_LO_DS:
	case R_PPC64_PLTGOT16_DS:
	case R_PPC64_PLTGOT16_LO_DS:
	case R_PPC64_GOT_TPREL16_DS:
	case R_PPC64_GOT_TPREL16_LO_DS:
	case R_PPC64_GOT_DTPREL16_DS:
	case R_PPC64_GOT_DTPREL16_LO_DS:
	case R_PPC64_TPREL16_DS:
	case R_PPC64_TPREL16_LO_DS:
	case R_PPC64_DTPREL16_DS:
	case R_PPC64_DTPREL16_LO_DS:
	  if (!offset_in_range (input_section, rel->r_offset & ~3, 4))
	    break;
	  insn = bfd_get_32 (input_bfd, contents + (rel->r_offset & ~3));
	  mask = 3;
	  /* If this reloc is against an lq, lxv, or stxv insn, then
	     the value must be a multiple of 16.  This is somewhat of
	     a hack, but the "correct" way to do this by defining _DQ
	     forms of all the _DS relocs bloats all reloc switches in
	     this file.  It doesn't make much sense to use these
	     relocs in data, so testing the insn should be safe.  */
	  if ((insn & (0x3fu << 26)) == (56u << 26)
	      || ((insn & (0x3fu << 26)) == (61u << 26) && (insn & 3) == 1))
	    mask = 15;
	  relocation += addend;
	  addend = insn & (mask ^ 3);
	  if ((relocation & mask) != 0)
	    {
	      relocation ^= relocation & mask;
	      info->callbacks->einfo
		/* xgettext:c-format */
		(_("%H: error: %s not a multiple of %u\n"),
		 input_bfd, input_section, rel->r_offset,
		 ppc64_elf_howto_table[r_type]->name,
		 mask + 1);
	      bfd_set_error (bfd_error_bad_value);
	      ret = false;
	      goto copy_reloc;
	    }
	  break;
	}

      /* Dynamic relocs are not propagated for SEC_DEBUGGING sections
	 because such sections are not SEC_ALLOC and thus ld.so will
	 not process them.  */
      howto = ppc64_elf_howto_table[(int) r_type];
      if (unresolved_reloc
	  && !((input_section->flags & SEC_DEBUGGING) != 0
	       && h->elf.def_dynamic)
	  && _bfd_elf_section_offset (output_bfd, info, input_section,
				      rel->r_offset) != (bfd_vma) -1)
	{
	  info->callbacks->einfo
	    /* xgettext:c-format */
	    (_("%H: unresolvable %s against `%pT'\n"),
	     input_bfd, input_section, rel->r_offset,
	     howto->name,
	     h->elf.root.root.string);
	  ret = false;
	}

      /* 16-bit fields in insns mostly have signed values, but a
	 few insns have 16-bit unsigned values.  Really, we should
	 have different reloc types.  */
      if (howto->complain_on_overflow != complain_overflow_dont
	  && howto->dst_mask == 0xffff
	  && (input_section->flags & SEC_CODE) != 0
	  && offset_in_range (input_section, rel->r_offset & ~3, 4))
	{
	  enum complain_overflow complain = complain_overflow_signed;

	  insn = bfd_get_32 (input_bfd, contents + (rel->r_offset & ~3));
	  if ((insn & (0x3fu << 26)) == 10u << 26 /* cmpli */)
	    complain = complain_overflow_bitfield;
	  else if (howto->rightshift == 0
		   ? ((insn & (0x3fu << 26)) == 28u << 26 /* andi */
		      || (insn & (0x3fu << 26)) == 24u << 26 /* ori */
		      || (insn & (0x3fu << 26)) == 26u << 26 /* xori */)
		   : ((insn & (0x3fu << 26)) == 29u << 26 /* andis */
		      || (insn & (0x3fu << 26)) == 25u << 26 /* oris */
		      || (insn & (0x3fu << 26)) == 27u << 26 /* xoris */))
	    complain = complain_overflow_unsigned;
	  if (howto->complain_on_overflow != complain)
	    {
	      alt_howto = *howto;
	      alt_howto.complain_on_overflow = complain;
	      howto = &alt_howto;
	    }
	}

      switch (r_type)
	{
	  /* Split field relocs aren't handled by _bfd_final_link_relocate.  */
	case R_PPC64_D34:
	case R_PPC64_D34_LO:
	case R_PPC64_D34_HI30:
	case R_PPC64_D34_HA30:
	case R_PPC64_PCREL34:
	case R_PPC64_GOT_PCREL34:
	case R_PPC64_TPREL34:
	case R_PPC64_DTPREL34:
	case R_PPC64_GOT_TLSGD_PCREL34:
	case R_PPC64_GOT_TLSLD_PCREL34:
	case R_PPC64_GOT_TPREL_PCREL34:
	case R_PPC64_GOT_DTPREL_PCREL34:
	case R_PPC64_PLT_PCREL34:
	case R_PPC64_PLT_PCREL34_NOTOC:
	case R_PPC64_D28:
	case R_PPC64_PCREL28:
	  if (!offset_in_range (input_section, rel->r_offset, 8))
	    r = bfd_reloc_outofrange;
	  else
	    {
	      relocation += addend;
	      if (howto->pc_relative)
		relocation -= (rel->r_offset
			       + input_section->output_offset
			       + input_section->output_section->vma);
	      relocation >>= howto->rightshift;

	      pinsn = bfd_get_32 (input_bfd, contents + rel->r_offset);
	      pinsn <<= 32;
	      pinsn |= bfd_get_32 (input_bfd, contents + rel->r_offset + 4);

	      pinsn &= ~howto->dst_mask;
	      pinsn |= (((relocation << 16) | (relocation & 0xffff))
			& howto->dst_mask);
	      bfd_put_32 (input_bfd, pinsn >> 32, contents + rel->r_offset);
	      bfd_put_32 (input_bfd, pinsn, contents + rel->r_offset + 4);
	      r = bfd_reloc_ok;
	      if (howto->complain_on_overflow == complain_overflow_signed
		  && (relocation + (1ULL << (howto->bitsize - 1))
		      >= 1ULL << howto->bitsize))
		r = bfd_reloc_overflow;
	    }
	  break;

	case R_PPC64_REL16DX_HA:
	  if (!offset_in_range (input_section, rel->r_offset, 4))
	    r = bfd_reloc_outofrange;
	  else
	    {
	      relocation += addend;
	      relocation -= (rel->r_offset
			     + input_section->output_offset
			     + input_section->output_section->vma);
	      relocation = (bfd_signed_vma) relocation >> 16;
	      insn = bfd_get_32 (input_bfd, contents + rel->r_offset);
	      insn &= ~0x1fffc1;
	      insn |= (relocation & 0xffc1) | ((relocation & 0x3e) << 15);
	      bfd_put_32 (input_bfd, insn, contents + rel->r_offset);
	      r = bfd_reloc_ok;
	      if (relocation + 0x8000 > 0xffff)
		r = bfd_reloc_overflow;
	    }
	  break;

	default:
	  r = _bfd_final_link_relocate (howto, input_bfd, input_section,
					contents, rel->r_offset,
					relocation, addend);
	}

      if (r != bfd_reloc_ok)
	{
	  char *more_info = NULL;
	  const char *reloc_name = howto->name;

	  if (reloc_dest != DEST_NORMAL)
	    {
	      more_info = bfd_malloc (strlen (reloc_name) + 8);
	      if (more_info != NULL)
		{
		  strcpy (more_info, reloc_name);
		  strcat (more_info, (reloc_dest == DEST_OPD
				      ? " (OPD)" : " (stub)"));
		  reloc_name = more_info;
		}
	    }

	  if (r == bfd_reloc_overflow)
	    {
	      /* On code like "if (foo) foo();" don't report overflow
		 on a branch to zero when foo is undefined.  */
	      if (!warned
		  && (reloc_dest == DEST_STUB
		      || !(h != NULL
			   && (h->elf.root.type == bfd_link_hash_undefweak
			       || h->elf.root.type == bfd_link_hash_undefined)
			   && is_branch_reloc (r_type))))
		info->callbacks->reloc_overflow
		  (info, (struct bfd_link_hash_entry *) h, sym_name,
		   reloc_name, orig_rel.r_addend, input_bfd, input_section,
		   rel->r_offset);
	    }
	  else
	    {
	      info->callbacks->einfo
		/* xgettext:c-format */
		(_("%H: %s against `%pT': error %d\n"),
		 input_bfd, input_section, rel->r_offset,
		 reloc_name, sym_name, (int) r);
	      ret = false;
	    }
	  free (more_info);
	}
    copy_reloc:
      if (wrel != rel)
	*wrel = *rel;
    }

  if (wrel != rel)
    {
      Elf_Internal_Shdr *rel_hdr;
      size_t deleted = rel - wrel;

      rel_hdr = _bfd_elf_single_rel_hdr (input_section->output_section);
      rel_hdr->sh_size -= rel_hdr->sh_entsize * deleted;
      if (rel_hdr->sh_size == 0)
	{
	  /* It is too late to remove an empty reloc section.  Leave
	     one NONE reloc.
	     ??? What is wrong with an empty section???  */
	  rel_hdr->sh_size = rel_hdr->sh_entsize;
	  deleted -= 1;
	}
      rel_hdr = _bfd_elf_single_rel_hdr (input_section);
      rel_hdr->sh_size -= rel_hdr->sh_entsize * deleted;
      input_section->reloc_count -= deleted;
    }

  /* If we're emitting relocations, then shortly after this function
     returns, reloc offsets and addends for this section will be
     adjusted.  Worse, reloc symbol indices will be for the output
     file rather than the input.  Save a copy of the relocs for
     opd_entry_value.  */
  if (is_opd
      && (info->emitrelocations || bfd_link_relocatable (info))
      && input_section->reloc_count != 0)
    {
      bfd_size_type amt;
      amt = input_section->reloc_count * sizeof (Elf_Internal_Rela);
      rel = bfd_alloc (input_bfd, amt);
      ppc64_elf_section_data (input_section)->u.opd.u.relocs = rel;
      if (rel == NULL)
	return false;
      memcpy (rel, relocs, amt);
    }
  return ret;
}

/* Adjust the value of any local symbols in opd sections.  */

static int
ppc64_elf_output_symbol_hook (struct bfd_link_info *info,
			      const char *name ATTRIBUTE_UNUSED,
			      Elf_Internal_Sym *elfsym,
			      asection *input_sec,
			      struct elf_link_hash_entry *h)
{
  struct _opd_sec_data *opd;
  long adjust;
  bfd_vma value;

  if (h != NULL)
    return 1;

  opd = get_opd_info (input_sec);
  if (opd == NULL || opd->adjust == NULL)
    return 1;

  value = elfsym->st_value - input_sec->output_offset;
  if (!bfd_link_relocatable (info))
    value -= input_sec->output_section->vma;

  adjust = opd->adjust[OPD_NDX (value)];
  if (adjust == -1)
    return 2;

  elfsym->st_value += adjust;
  return 1;
}

/* Finish up dynamic symbol handling.  We set the contents of various
   dynamic sections here.  */

static bool
ppc64_elf_finish_dynamic_symbol (bfd *output_bfd,
				 struct bfd_link_info *info,
				 struct elf_link_hash_entry *h,
				 Elf_Internal_Sym *sym)
{
  struct ppc_link_hash_table *htab;
  struct plt_entry *ent;

  htab = ppc_hash_table (info);
  if (htab == NULL)
    return false;

  if (!htab->opd_abi && !h->def_regular)
    for (ent = h->plt.plist; ent != NULL; ent = ent->next)
      if (ent->plt.offset != (bfd_vma) -1)
	{
	  /* Mark the symbol as undefined, rather than as
	     defined in glink.  Leave the value if there were
	     any relocations where pointer equality matters
	     (this is a clue for the dynamic linker, to make
	     function pointer comparisons work between an
	     application and shared library), otherwise set it
	     to zero.  */
	  sym->st_shndx = SHN_UNDEF;
	  if (!h->pointer_equality_needed)
	    sym->st_value = 0;
	  else if (!h->ref_regular_nonweak)
	    {
	      /* This breaks function pointer comparisons, but
		 that is better than breaking tests for a NULL
		 function pointer.  */
	      sym->st_value = 0;
	    }
	  break;
	}

  if (h->needs_copy
      && (h->root.type == bfd_link_hash_defined
	  || h->root.type == bfd_link_hash_defweak)
      && (h->root.u.def.section == htab->elf.sdynbss
	  || h->root.u.def.section == htab->elf.sdynrelro))
    {
      /* This symbol needs a copy reloc.  Set it up.  */
      Elf_Internal_Rela rela;
      asection *srel;
      bfd_byte *loc;

      if (h->dynindx == -1)
	abort ();

      rela.r_offset = defined_sym_val (h);
      rela.r_info = ELF64_R_INFO (h->dynindx, R_PPC64_COPY);
      rela.r_addend = 0;
      if (h->root.u.def.section == htab->elf.sdynrelro)
	srel = htab->elf.sreldynrelro;
      else
	srel = htab->elf.srelbss;
      loc = srel->contents;
      loc += srel->reloc_count++ * sizeof (Elf64_External_Rela);
      bfd_elf64_swap_reloca_out (output_bfd, &rela, loc);
    }

  return true;
}

/* Used to decide how to sort relocs in an optimal manner for the
   dynamic linker, before writing them out.  */

static enum elf_reloc_type_class
ppc64_elf_reloc_type_class (const struct bfd_link_info *info,
			    const asection *rel_sec,
			    const Elf_Internal_Rela *rela)
{
  enum elf_ppc64_reloc_type r_type;
  struct ppc_link_hash_table *htab = ppc_hash_table (info);

  if (rel_sec == htab->elf.irelplt)
    return reloc_class_ifunc;

  r_type = ELF64_R_TYPE (rela->r_info);
  switch (r_type)
    {
    case R_PPC64_RELATIVE:
      return reloc_class_relative;
    case R_PPC64_JMP_SLOT:
      return reloc_class_plt;
    case R_PPC64_COPY:
      return reloc_class_copy;
    default:
      return reloc_class_normal;
    }
}

/* Finish up the dynamic sections.  */

static bool
ppc64_elf_finish_dynamic_sections (bfd *output_bfd,
				   struct bfd_link_info *info)
{
  struct ppc_link_hash_table *htab;
  bfd *dynobj;
  asection *sdyn;

  htab = ppc_hash_table (info);
  if (htab == NULL)
    return false;

  dynobj = htab->elf.dynobj;
  sdyn = bfd_get_linker_section (dynobj, ".dynamic");

  if (htab->elf.dynamic_sections_created)
    {
      Elf64_External_Dyn *dyncon, *dynconend;

      if (sdyn == NULL || htab->elf.sgot == NULL)
	abort ();

      dyncon = (Elf64_External_Dyn *) sdyn->contents;
      dynconend = (Elf64_External_Dyn *) (sdyn->contents + sdyn->size);
      for (; dyncon < dynconend; dyncon++)
	{
	  Elf_Internal_Dyn dyn;
	  asection *s;

	  bfd_elf64_swap_dyn_in (dynobj, dyncon, &dyn);

	  switch (dyn.d_tag)
	    {
	    default:
	      continue;

	    case DT_PPC64_GLINK:
	      s = htab->glink;
	      dyn.d_un.d_ptr = s->output_section->vma + s->output_offset;
	      /* We stupidly defined DT_PPC64_GLINK to be the start
		 of glink rather than the first entry point, which is
		 what ld.so needs, and now have a bigger stub to
		 support automatic multiple TOCs.  */
	      dyn.d_un.d_ptr += GLINK_PLTRESOLVE_SIZE (htab) - 8 * 4;
	      break;

	    case DT_PPC64_OPD:
	      s = bfd_get_section_by_name (output_bfd, ".opd");
	      if (s == NULL)
		continue;
	      dyn.d_un.d_ptr = s->vma;
	      break;

	    case DT_PPC64_OPT:
	      if ((htab->do_multi_toc && htab->multi_toc_needed)
		  || htab->notoc_plt)
		dyn.d_un.d_val |= PPC64_OPT_MULTI_TOC;
	      if (htab->has_plt_localentry0)
		dyn.d_un.d_val |= PPC64_OPT_LOCALENTRY;
	      break;

	    case DT_PPC64_OPDSZ:
	      s = bfd_get_section_by_name (output_bfd, ".opd");
	      if (s == NULL)
		continue;
	      dyn.d_un.d_val = s->size;
	      break;

	    case DT_PLTGOT:
	      s = htab->elf.splt;
	      dyn.d_un.d_ptr = s->output_section->vma + s->output_offset;
	      break;

	    case DT_JMPREL:
	      s = htab->elf.srelplt;
	      dyn.d_un.d_ptr = s->output_section->vma + s->output_offset;
	      break;

	    case DT_PLTRELSZ:
	      dyn.d_un.d_val = htab->elf.srelplt->size;
	      break;

	    case DT_TEXTREL:
	      if (htab->elf.ifunc_resolvers)
		info->callbacks->einfo
		  (_("%P: warning: text relocations and GNU indirect "
		     "functions may result in a segfault at runtime\n"));
	      continue;
	    }

	  bfd_elf64_swap_dyn_out (output_bfd, &dyn, dyncon);
	}
    }

  if (htab->elf.sgot != NULL && htab->elf.sgot->size != 0
      && htab->elf.sgot->output_section != bfd_abs_section_ptr)
    {
      /* Fill in the first entry in the global offset table.
	 We use it to hold the link-time TOCbase.  */
      bfd_put_64 (output_bfd,
		  elf_gp (output_bfd) + TOC_BASE_OFF,
		  htab->elf.sgot->contents);

      /* Set .got entry size.  */
      elf_section_data (htab->elf.sgot->output_section)->this_hdr.sh_entsize
	= 8;
    }

  if (htab->elf.splt != NULL && htab->elf.splt->size != 0
      && htab->elf.splt->output_section != bfd_abs_section_ptr)
    {
      /* Set .plt entry size.  */
      elf_section_data (htab->elf.splt->output_section)->this_hdr.sh_entsize
	= PLT_ENTRY_SIZE (htab);
    }

  /* brlt is SEC_LINKER_CREATED, so we need to write out relocs for
     brlt ourselves if emitrelocations.  */
  if (htab->brlt != NULL
      && htab->brlt->reloc_count != 0
      && !_bfd_elf_link_output_relocs (output_bfd,
				       htab->brlt,
				       elf_section_data (htab->brlt)->rela.hdr,
				       elf_section_data (htab->brlt)->relocs,
				       NULL))
    return false;

  if (htab->glink != NULL
      && htab->glink->reloc_count != 0
      && !_bfd_elf_link_output_relocs (output_bfd,
				       htab->glink,
				       elf_section_data (htab->glink)->rela.hdr,
				       elf_section_data (htab->glink)->relocs,
				       NULL))
    return false;


  if (htab->glink_eh_frame != NULL
      && htab->glink_eh_frame->size != 0
      && htab->glink_eh_frame->sec_info_type == SEC_INFO_TYPE_EH_FRAME
      && !_bfd_elf_write_section_eh_frame (output_bfd, info,
					   htab->glink_eh_frame,
					   htab->glink_eh_frame->contents))
    return false;

  /* We need to handle writing out multiple GOT sections ourselves,
     since we didn't add them to DYNOBJ.  We know dynobj is the first
     bfd.  */
  while ((dynobj = dynobj->link.next) != NULL)
    {
      asection *s;

      if (!is_ppc64_elf (dynobj))
	continue;

      s = ppc64_elf_tdata (dynobj)->got;
      if (s != NULL
	  && s->size != 0
	  && s->output_section != bfd_abs_section_ptr
	  && !bfd_set_section_contents (output_bfd, s->output_section,
					s->contents, s->output_offset,
					s->size))
	return false;
      s = ppc64_elf_tdata (dynobj)->relgot;
      if (s != NULL
	  && s->size != 0
	  && s->output_section != bfd_abs_section_ptr
	  && !bfd_set_section_contents (output_bfd, s->output_section,
					s->contents, s->output_offset,
					s->size))
	return false;
    }

  return true;
}

static bool
ppc64_elf_free_cached_info (bfd *abfd)
{
  if (abfd->sections)
    for (asection *opd = bfd_get_section_by_name (abfd, ".opd");
	 opd != NULL;
	 opd = bfd_get_next_section_by_name (NULL, opd))
      if (opd->reloc_count == 0)
	free (ppc64_elf_section_data (opd)->u.opd.u.contents);

  return _bfd_elf_free_cached_info (abfd);
}

#include "elf64-target.h"

/* FreeBSD support */

#undef  TARGET_LITTLE_SYM
#define TARGET_LITTLE_SYM powerpc_elf64_fbsd_le_vec
#undef  TARGET_LITTLE_NAME
#define TARGET_LITTLE_NAME "elf64-powerpcle-freebsd"

#undef  TARGET_BIG_SYM
#define TARGET_BIG_SYM	powerpc_elf64_fbsd_vec
#undef  TARGET_BIG_NAME
#define TARGET_BIG_NAME "elf64-powerpc-freebsd"

#undef  ELF_OSABI
#define	ELF_OSABI       ELFOSABI_FREEBSD

#undef  elf64_bed
#define elf64_bed	elf64_powerpc_fbsd_bed

#include "elf64-target.h"
