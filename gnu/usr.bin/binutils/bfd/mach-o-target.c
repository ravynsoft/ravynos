/* Mach-O support for BFD.
   Copyright (C) 1999-2023 Free Software Foundation, Inc.

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

/* Define generic entry points here so that we don't need to duplicate the
   defines in every target.  But define once as this file may be included
   several times.  */
#ifndef MACH_O_TARGET_COMMON_DEFINED
#define MACH_O_TARGET_COMMON_DEFINED

#define bfd_mach_o_get_section_contents_in_window     _bfd_generic_get_section_contents_in_window
#define bfd_mach_o_bfd_print_private_bfd_data	      bfd_mach_o_bfd_print_private_bfd_data
#define bfd_mach_o_bfd_is_target_special_symbol	      _bfd_bool_bfd_asymbol_false
#define bfd_mach_o_bfd_is_local_label_name	      bfd_generic_is_local_label_name
#define bfd_mach_o_get_lineno			      _bfd_nosymbols_get_lineno
#define bfd_mach_o_find_inliner_info		      _bfd_nosymbols_find_inliner_info
#define bfd_mach_o_get_symbol_version_string	      _bfd_nosymbols_get_symbol_version_string
#define bfd_mach_o_bfd_make_debug_symbol	      _bfd_nosymbols_bfd_make_debug_symbol
#define bfd_mach_o_read_minisymbols		      _bfd_generic_read_minisymbols
#define bfd_mach_o_minisymbol_to_symbol		      _bfd_generic_minisymbol_to_symbol
#define bfd_mach_o_bfd_get_relocated_section_contents bfd_generic_get_relocated_section_contents
#define bfd_mach_o_bfd_relax_section		      bfd_generic_relax_section
#define bfd_mach_o_bfd_link_hash_table_create	      _bfd_generic_link_hash_table_create
#define bfd_mach_o_bfd_link_add_symbols		      _bfd_generic_link_add_symbols
#define bfd_mach_o_bfd_link_just_syms		      _bfd_generic_link_just_syms
#define bfd_mach_o_bfd_copy_link_hash_symbol_type \
  _bfd_generic_copy_link_hash_symbol_type
#define bfd_mach_o_bfd_final_link		      _bfd_generic_final_link
#define bfd_mach_o_bfd_link_split_section	      _bfd_generic_link_split_section
#define bfd_mach_o_bfd_link_check_relocs	      _bfd_generic_link_check_relocs
#define bfd_mach_o_bfd_merge_private_bfd_data	      _bfd_generic_bfd_merge_private_bfd_data
#define bfd_mach_o_bfd_set_private_flags	      bfd_mach_o_bfd_set_private_flags
#define bfd_mach_o_get_section_contents		      _bfd_generic_get_section_contents
#define bfd_mach_o_bfd_gc_sections		      bfd_generic_gc_sections
#define bfd_mach_o_bfd_lookup_section_flags	      bfd_generic_lookup_section_flags
#define bfd_mach_o_bfd_merge_sections		      bfd_generic_merge_sections
#define bfd_mach_o_bfd_is_group_section		      bfd_generic_is_group_section
#define bfd_mach_o_bfd_group_name		      bfd_generic_group_name
#define bfd_mach_o_bfd_discard_group		      bfd_generic_discard_group
#define bfd_mach_o_section_already_linked	      _bfd_generic_section_already_linked
#define bfd_mach_o_bfd_define_common_symbol	      bfd_generic_define_common_symbol
#define bfd_mach_o_bfd_link_hide_symbol		      _bfd_generic_link_hide_symbol
#define bfd_mach_o_bfd_define_start_stop	      bfd_generic_define_start_stop
#define bfd_mach_o_bfd_copy_private_bfd_data	      _bfd_generic_bfd_copy_private_bfd_data
#define bfd_mach_o_core_file_matches_executable_p     generic_core_file_matches_executable_p
#define bfd_mach_o_core_file_pid		      _bfd_nocore_core_file_pid
#define bfd_mach_o_set_reloc			      _bfd_generic_set_reloc

#define bfd_mach_o_get_dynamic_symtab_upper_bound     bfd_mach_o_get_symtab_upper_bound
#define bfd_mach_o_canonicalize_dynamic_symtab	      bfd_mach_o_canonicalize_symtab

/* For Mach-O special archives.  */
#define bfd_mach_o_read_ar_hdr			  _bfd_noarchive_read_ar_hdr
#define bfd_mach_o_write_ar_hdr			  _bfd_noarchive_write_ar_hdr
#define bfd_mach_o_slurp_armap			  _bfd_noarchive_slurp_armap
#define bfd_mach_o_slurp_extended_name_table	  _bfd_noarchive_slurp_extended_name_table
#define bfd_mach_o_construct_extended_name_table  _bfd_noarchive_construct_extended_name_table
#define bfd_mach_o_truncate_arname		  _bfd_noarchive_truncate_arname
#define bfd_mach_o_write_armap			  _bfd_noarchive_write_armap
#define bfd_mach_o_get_elt_at_index		  _bfd_noarchive_get_elt_at_index
#define bfd_mach_o_update_armap_timestamp	  _bfd_noarchive_update_armap_timestamp

#define TARGET_NAME_BACKEND XCONCAT2(TARGET_NAME,_backend)

#endif /* MACH_O_TARGET_COMMON_DEFINED */

#ifndef TARGET_NAME
#error TARGET_NAME must be defined
#endif /* TARGET_NAME */

#ifndef TARGET_STRING
#error TARGET_STRING must be defined
#endif /* TARGET_STRING */

#ifndef TARGET_ARCHITECTURE
#error TARGET_ARCHITECTURE must be defined
#endif /* TARGET_ARCHITECTURE */

#ifndef TARGET_BIG_ENDIAN
#error TARGET_BIG_ENDIAN must be defined
#endif /* TARGET_BIG_ENDIAN */

#ifndef TARGET_ARCHIVE
#error TARGET_ARCHIVE must be defined
#endif /* TARGET_ARCHIVE */

#ifndef TARGET_PAGESIZE
#error TARGET_PAGESIZE must be defined
#endif

static const bfd_mach_o_backend_data TARGET_NAME_BACKEND =
{
  TARGET_ARCHITECTURE,
  TARGET_PAGESIZE,
  bfd_mach_o_canonicalize_one_reloc,
  bfd_mach_o_swap_reloc_out,
  bfd_mach_o_print_thread,
  bfd_mach_o_tgt_seg_table,
  bfd_mach_o_section_type_valid_for_tgt
};

const bfd_target TARGET_NAME =
{
  TARGET_STRING,		/* Name.  */
  bfd_target_mach_o_flavour,
#if TARGET_BIG_ENDIAN
  BFD_ENDIAN_BIG,		/* Target byte order.  */
  BFD_ENDIAN_BIG,		/* Target headers byte order.  */
#else
  BFD_ENDIAN_LITTLE,		/* Target byte order.  */
  BFD_ENDIAN_LITTLE,		/* Target headers byte order.  */
#endif
  (HAS_RELOC | EXEC_P |		/* Object flags.  */
   HAS_LINENO | HAS_DEBUG |
   HAS_SYMS | HAS_LOCALS | DYNAMIC | WP_TEXT | D_PAGED),
  (SEC_CODE | SEC_DATA | SEC_ROM | SEC_HAS_CONTENTS
   | SEC_ALLOC | SEC_LOAD | SEC_RELOC),	/* Section flags.  */
  '_',				/* symbol_leading_char.  */
  ' ',				/* ar_pad_char.  */
  16,				/* ar_max_namelen.  */
  TARGET_PRIORITY,	/* match priority.  */
  TARGET_KEEP_UNUSED_SECTION_SYMBOLS, /* keep unused section symbols.  */

#if TARGET_BIG_ENDIAN
  bfd_getb64, bfd_getb_signed_64, bfd_putb64,
  bfd_getb32, bfd_getb_signed_32, bfd_putb32,
  bfd_getb16, bfd_getb_signed_16, bfd_putb16,	/* Data.  */
  bfd_getb64, bfd_getb_signed_64, bfd_putb64,
  bfd_getb32, bfd_getb_signed_32, bfd_putb32,
  bfd_getb16, bfd_getb_signed_16, bfd_putb16,	/* Hdrs.  */
#else
  bfd_getl64, bfd_getl_signed_64, bfd_putl64,
  bfd_getl32, bfd_getl_signed_32, bfd_putl32,
  bfd_getl16, bfd_getl_signed_16, bfd_putl16,	/* data */
  bfd_getl64, bfd_getl_signed_64, bfd_putl64,
  bfd_getl32, bfd_getl_signed_32, bfd_putl32,
  bfd_getl16, bfd_getl_signed_16, bfd_putl16,	/* hdrs */
#endif /* TARGET_BIG_ENDIAN */

  {				/* bfd_check_format.  */
#if TARGET_ARCHIVE
    _bfd_dummy_target,
    _bfd_dummy_target,
    bfd_mach_o_archive_p,
    _bfd_dummy_target,
#else
    _bfd_dummy_target,
    bfd_mach_o_object_p,
    bfd_generic_archive_p,
    bfd_mach_o_core_p
#endif
  },
  {				/* bfd_set_format.  */
    _bfd_bool_bfd_false_error,
    bfd_mach_o_mkobject,
    _bfd_generic_mkarchive,
    bfd_mach_o_mkobject,
  },
  {				/* bfd_write_contents.  */
    _bfd_bool_bfd_false_error,
    bfd_mach_o_write_contents,
    _bfd_write_archive_contents,
    bfd_mach_o_write_contents,
  },

  BFD_JUMP_TABLE_GENERIC (bfd_mach_o),
  BFD_JUMP_TABLE_COPY (bfd_mach_o),
  BFD_JUMP_TABLE_CORE (bfd_mach_o),
#if TARGET_ARCHIVE
  BFD_JUMP_TABLE_ARCHIVE (bfd_mach_o),
#else
  BFD_JUMP_TABLE_ARCHIVE (_bfd_archive_bsd44),
#endif
  BFD_JUMP_TABLE_SYMBOLS (bfd_mach_o),
  BFD_JUMP_TABLE_RELOCS (bfd_mach_o),
  BFD_JUMP_TABLE_WRITE (bfd_mach_o),
  BFD_JUMP_TABLE_LINK (bfd_mach_o),
  BFD_JUMP_TABLE_DYNAMIC (bfd_mach_o),

  /* Alternative endian target.  */
  NULL,

  /* Back-end data.  */
  &TARGET_NAME_BACKEND
};

