/* MIPS ELF specific backend routines.
   Copyright (C) 2002-2023 Free Software Foundation, Inc.

   This file is part of BFD, the Binary File Descriptor library.

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

#include "elf/common.h"
#include "elf/internal.h"
#include "elf/mips.h"

enum reloc_check
{
  check_std,
  check_inplace,
  check_shuffle
};

#ifndef MIPS_DEFAULT_R6
#define MIPS_DEFAULT_R6 0
#endif

struct ecoff_debug_info;

extern bool _bfd_mips_elf_mkobject
  (bfd *);
extern bool _bfd_mips_elf_free_cached_info
  (bfd *);
extern bool _bfd_mips_elf_new_section_hook
  (bfd *, asection *);
extern void _bfd_mips_elf_symbol_processing
  (bfd *, asymbol *);
extern unsigned int _bfd_mips_elf_eh_frame_address_size
  (bfd *, const asection *);
extern bool _bfd_mips_elf_name_local_section_symbols
  (bfd *);
extern bool _bfd_mips_elf_section_processing
  (bfd *, Elf_Internal_Shdr *);
extern bool _bfd_mips_elf_section_from_shdr
  (bfd *, Elf_Internal_Shdr *, const char *, int);
extern bool _bfd_mips_elf_fake_sections
  (bfd *, Elf_Internal_Shdr *, asection *);
extern bool _bfd_mips_elf_section_from_bfd_section
  (bfd *, asection *, int *);
extern bool _bfd_mips_elf_add_symbol_hook
  (bfd *, struct bfd_link_info *, Elf_Internal_Sym *,
   const char **, flagword *, asection **, bfd_vma *);
extern int _bfd_mips_elf_link_output_symbol_hook
  (struct bfd_link_info *, const char *, Elf_Internal_Sym *,
   asection *, struct elf_link_hash_entry *);
extern bool _bfd_mips_elf_create_dynamic_sections
  (bfd *, struct bfd_link_info *);
extern bool _bfd_mips_elf_check_relocs
  (bfd *, struct bfd_link_info *, asection *, const Elf_Internal_Rela *);
extern bool _bfd_mips_elf_adjust_dynamic_symbol
  (struct bfd_link_info *, struct elf_link_hash_entry *);
extern bool _bfd_mips_elf_always_size_sections
  (bfd *, struct bfd_link_info *);
extern bool _bfd_mips_elf_size_dynamic_sections
  (bfd *, struct bfd_link_info *);
extern int _bfd_mips_elf_relocate_section
  (bfd *, struct bfd_link_info *, bfd *, asection *, bfd_byte *,
   Elf_Internal_Rela *, Elf_Internal_Sym *, asection **);
extern bool _bfd_mips_elf_finish_dynamic_symbol
  (bfd *, struct bfd_link_info *, struct elf_link_hash_entry *,
   Elf_Internal_Sym *);
extern bool _bfd_mips_vxworks_finish_dynamic_symbol
  (bfd *, struct bfd_link_info *, struct elf_link_hash_entry *,
   Elf_Internal_Sym *);
extern bool _bfd_mips_elf_finish_dynamic_sections
  (bfd *, struct bfd_link_info *);
extern bool _bfd_mips_elf_sort_relocs_p
  (asection *);
extern void _bfd_mips_final_write_processing
  (bfd *);
extern bool _bfd_mips_elf_final_write_processing
  (bfd *);
extern int _bfd_mips_elf_additional_program_headers
  (bfd *, struct bfd_link_info *);
extern bool _bfd_mips_elf_modify_segment_map
  (bfd *, struct bfd_link_info *);
extern asection * _bfd_mips_elf_gc_mark_hook
  (asection *, struct bfd_link_info *, Elf_Internal_Rela *,
   struct elf_link_hash_entry *, Elf_Internal_Sym *);
extern void _bfd_mips_elf_copy_indirect_symbol
  (struct bfd_link_info *, struct elf_link_hash_entry *,
   struct elf_link_hash_entry *);
extern void _bfd_mips_elf_hide_symbol
  (struct bfd_link_info *, struct elf_link_hash_entry *, bool);
extern bool _bfd_mips_elf_ignore_discarded_relocs
  (asection *);
extern bool _bfd_mips_elf_is_target_special_symbol
  (bfd *abfd, asymbol *sym);
extern bool _bfd_mips_elf_find_nearest_line
  (bfd *, asymbol **, asection *, bfd_vma,
   const char **, const char **, unsigned int *, unsigned int *);
#define _bfd_mips_elf_find_nearest_line_with_alt \
  _bfd_nosymbols_find_nearest_line_with_alt
extern bool _bfd_mips_elf_find_inliner_info
  (bfd *, const char **, const char **, unsigned int *);
extern bool _bfd_mips_elf_set_section_contents
  (bfd *, asection *, const void *, file_ptr, bfd_size_type);
extern bfd_byte *_bfd_elf_mips_get_relocated_section_contents
  (bfd *, struct bfd_link_info *, struct bfd_link_order *,
   bfd_byte *, bool, asymbol **);
extern bool _bfd_mips_elf_relax_section
  (bfd *abfd, asection *sec, struct bfd_link_info *link_info,
   bool *again);
extern struct bfd_link_hash_table *_bfd_mips_elf_link_hash_table_create
  (bfd *);
extern struct bfd_link_hash_table *_bfd_mips_vxworks_link_hash_table_create
  (bfd *);
extern bool _bfd_mips_elf_final_link
  (bfd *, struct bfd_link_info *);
extern bool _bfd_mips_elf_merge_private_bfd_data
  (bfd *, struct bfd_link_info *);
extern bool _bfd_mips_elf_set_private_flags
  (bfd *, flagword);
extern const char * _bfd_mips_fp_abi_string
  (int);
extern bool _bfd_mips_elf_print_private_bfd_data
  (bfd *, void *);
extern bool _bfd_mips_elf_discard_info
  (bfd *, struct elf_reloc_cookie *, struct bfd_link_info *);
extern bool _bfd_mips_elf_write_section
  (bfd *, struct bfd_link_info *, asection *, bfd_byte *);

extern bool _bfd_mips_elf_read_ecoff_info
  (bfd *, asection *, struct ecoff_debug_info *);
extern void _bfd_mips_elf_reloc_unshuffle
  (bfd *, int, bool, bfd_byte *);
extern void _bfd_mips_elf_reloc_shuffle
  (bfd *, int, bool, bfd_byte *);
extern bool _bfd_mips_reloc_offset_in_range
  (bfd *, asection *, arelent *, enum reloc_check);
extern bfd_reloc_status_type _bfd_mips_elf_gprel16_with_gp
  (bfd *, asymbol *, arelent *, asection *, bool, void *, bfd_vma);
extern bfd_reloc_status_type _bfd_mips_elf32_gprel16_reloc
  (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);
extern bfd_reloc_status_type _bfd_mips_elf_hi16_reloc
  (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);
extern bfd_reloc_status_type _bfd_mips_elf_got16_reloc
  (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);
extern bfd_reloc_status_type _bfd_mips_elf_lo16_reloc
  (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);
extern bfd_reloc_status_type _bfd_mips_elf_generic_reloc
  (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);
extern unsigned long _bfd_elf_mips_mach
  (flagword);
extern bfd_vma _bfd_mips_elf_sign_extend
  (bfd_vma, int);
extern void _bfd_mips_elf_merge_symbol_attribute
  (struct elf_link_hash_entry *, unsigned int, bool, bool);
extern char *_bfd_mips_elf_get_target_dtag (bfd_vma);
extern bool _bfd_mips_elf_ignore_undef_symbol
  (struct elf_link_hash_entry *);
extern void _bfd_mips_elf_use_plts_and_copy_relocs
  (struct bfd_link_info *);
extern void _bfd_mips_elf_linker_flags
  (struct bfd_link_info *, bool, bool, bool);
extern void _bfd_mips_elf_compact_branches
  (struct bfd_link_info *, bool);
extern bool _bfd_mips_elf_init_stubs
  (struct bfd_link_info *,
   asection *(*) (const char *, asection *, asection *));
extern bfd_vma _bfd_mips_elf_plt_sym_val
  (bfd_vma, const asection *, const arelent *rel);
extern long _bfd_mips_elf_get_synthetic_symtab
  (bfd *, long, asymbol **, long, asymbol **, asymbol **);
extern bool _bfd_mips_elf_gc_mark_extra_sections
  (struct bfd_link_info *, elf_gc_mark_hook_fn);
extern bool _bfd_mips_init_file_header
  (bfd *abfd, struct bfd_link_info *link_info);

extern const struct bfd_elf_special_section _bfd_mips_elf_special_sections [];

extern bool _bfd_mips_elf_common_definition (Elf_Internal_Sym *);

extern int _bfd_mips_elf_compact_eh_encoding (struct bfd_link_info *);
extern int _bfd_mips_elf_cant_unwind_opcode (struct bfd_link_info *);

extern void _bfd_mips_elf_record_xhash_symbol
  (struct elf_link_hash_entry *h, bfd_vma xlat_loc);

/* MIPS ABI flags data access.  For the disassembler.  */
extern struct elf_internal_abiflags_v0 *bfd_mips_elf_get_abiflags (bfd *);

static inline bool
gprel16_reloc_p (unsigned int r_type)
{
  return (r_type == R_MIPS_GPREL16
	  || r_type == R_MIPS16_GPREL
	  || r_type == R_MICROMIPS_GPREL16
	  || r_type == R_MICROMIPS_GPREL7_S2);
}

static inline bool
literal_reloc_p (int r_type)
{
  return r_type == R_MIPS_LITERAL || r_type == R_MICROMIPS_LITERAL;
}

#define elf_backend_common_definition   _bfd_mips_elf_common_definition
#define elf_backend_name_local_section_symbols \
  _bfd_mips_elf_name_local_section_symbols
#define elf_backend_special_sections _bfd_mips_elf_special_sections
#define elf_backend_eh_frame_address_size _bfd_mips_elf_eh_frame_address_size
#define elf_backend_merge_symbol_attribute  _bfd_mips_elf_merge_symbol_attribute
#define elf_backend_ignore_undef_symbol _bfd_mips_elf_ignore_undef_symbol
#define elf_backend_init_file_header _bfd_mips_init_file_header
#define elf_backend_compact_eh_encoding _bfd_mips_elf_compact_eh_encoding
#define elf_backend_cant_unwind_opcode _bfd_mips_elf_cant_unwind_opcode
#define elf_backend_record_xhash_symbol _bfd_mips_elf_record_xhash_symbol
#define elf_backend_always_renumber_dynsyms true
