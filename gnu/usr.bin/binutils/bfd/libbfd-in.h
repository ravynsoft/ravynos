/* libbfd.h -- Declarations used by bfd library *implementation*.
   (This include file is not for users of the library.)

   Copyright (C) 1990-2023 Free Software Foundation, Inc.

   Written by Cygnus Support.

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

#ifndef _LIBBFD_H
#define _LIBBFD_H 1

#ifndef ATTRIBUTE_HIDDEN
#if HAVE_HIDDEN
#define ATTRIBUTE_HIDDEN __attribute__ ((__visibility__ ("hidden")))
#else
#define ATTRIBUTE_HIDDEN
#endif
#endif

#include "hashtab.h"

#ifdef __cplusplus
extern "C" {
#endif

/* If you want to read and write large blocks, you might want to do it
   in quanta of this amount */
#define DEFAULT_BUFFERSIZE 8192

/* Set a tdata field.  Can't use the other macros for this, since they
   do casts, and casting to the left of assignment isn't portable.  */
#define set_tdata(bfd, v) ((bfd)->tdata.any = (v))

/* If BFD_IN_MEMORY is set for a BFD, then the iostream fields points
   to an instance of this structure.  */

struct bfd_in_memory
{
  /* Size of buffer.  */
  bfd_size_type size;
  /* Buffer holding contents of BFD.  */
  bfd_byte *buffer;
};

struct section_hash_entry
{
  struct bfd_hash_entry root;
  asection section;
};

/* Unique section id.  */
extern unsigned int _bfd_section_id ATTRIBUTE_HIDDEN;

/* tdata for an archive.  For an input archive, cache
   needs to be free()'d.  For an output archive, symdefs do.  */

struct artdata
{
  ufile_ptr first_file_filepos;
  /* Speed up searching the armap */
  htab_t cache;
  carsym *symdefs;		/* The symdef entries.  */
  symindex symdef_count;	/* How many there are.  */
  char *extended_names;		/* Clever intel extension.  */
  bfd_size_type extended_names_size; /* Size of extended names.  */
  /* When more compilers are standard C, this can be a time_t.  */
  long  armap_timestamp;	/* Timestamp value written into armap.
				   This is used for BSD archives to check
				   that the timestamp is recent enough
				   for the BSD linker to not complain,
				   just before we finish writing an
				   archive.  */
  file_ptr armap_datepos;	/* Position within archive to seek to
				   rewrite the date field.  */
  void *tdata;			/* Backend specific information.  */
};

#define bfd_ardata(bfd) ((bfd)->tdata.aout_ar_data)

/* Goes in bfd's arelt_data slot */
struct areltdata
{
  char * arch_header;		/* It's actually a string.  */
  bfd_size_type parsed_size;	/* Octets of filesize not including ar_hdr.  */
  bfd_size_type extra_size;	/* BSD4.4: extra bytes after the header.  */
  char *filename;		/* Null-terminated.  */
  file_ptr origin;		/* For element of a thin archive.  */
  void *parent_cache;		/* Where and how to find this member.  */
  file_ptr key;
};

#define arelt_size(bfd) (((struct areltdata *)((bfd)->arelt_data))->parsed_size)

extern void *bfd_malloc
  (bfd_size_type) ATTRIBUTE_HIDDEN;

static inline char *
bfd_strdup (const char *str)
{
  size_t len = strlen (str) + 1;
  char *buf = bfd_malloc (len);
  if (buf != NULL)
    memcpy (buf, str, len);
  return buf;
}

extern bfd * _bfd_create_empty_archive_element_shell
  (bfd *) ATTRIBUTE_HIDDEN;
extern bfd * _bfd_look_for_bfd_in_cache
  (bfd *, file_ptr) ATTRIBUTE_HIDDEN;
extern bool _bfd_add_bfd_to_archive_cache
  (bfd *, file_ptr, bfd *) ATTRIBUTE_HIDDEN;
extern bool _bfd_generic_mkarchive
  (bfd *) ATTRIBUTE_HIDDEN;
extern char *_bfd_append_relative_path
  (bfd *, char *) ATTRIBUTE_HIDDEN;
extern bfd_cleanup bfd_generic_archive_p
  (bfd *) ATTRIBUTE_HIDDEN;
extern bool bfd_slurp_armap
  (bfd *) ATTRIBUTE_HIDDEN;
#define bfd_slurp_bsd_armap bfd_slurp_armap
#define bfd_slurp_coff_armap bfd_slurp_armap
extern bool _bfd_archive_64_bit_slurp_armap
  (bfd *) ATTRIBUTE_HIDDEN;
extern bool _bfd_archive_64_bit_write_armap
  (bfd *, unsigned int, struct orl *, unsigned int, int) ATTRIBUTE_HIDDEN;
#define _bfd_archive_64_bit_slurp_extended_name_table \
  _bfd_slurp_extended_name_table
#define _bfd_archive_64_bit_construct_extended_name_table \
  _bfd_archive_coff_construct_extended_name_table
#define _bfd_archive_64_bit_truncate_arname \
  bfd_dont_truncate_arname
#define _bfd_archive_64_bit_read_ar_hdr \
  _bfd_generic_read_ar_hdr
#define _bfd_archive_64_bit_write_ar_hdr \
  _bfd_generic_write_ar_hdr
#define _bfd_archive_64_bit_openr_next_archived_file \
  bfd_generic_openr_next_archived_file
#define _bfd_archive_64_bit_get_elt_at_index \
  _bfd_generic_get_elt_at_index
#define _bfd_archive_64_bit_generic_stat_arch_elt \
  bfd_generic_stat_arch_elt
#define _bfd_archive_64_bit_update_armap_timestamp _bfd_bool_bfd_true

extern bool _bfd_slurp_extended_name_table
  (bfd *) ATTRIBUTE_HIDDEN;
extern bool _bfd_construct_extended_name_table
  (bfd *, bool, char **, bfd_size_type *) ATTRIBUTE_HIDDEN;
extern bool _bfd_write_archive_contents
  (bfd *) ATTRIBUTE_HIDDEN;
extern bool _bfd_compute_and_write_armap
  (bfd *, unsigned int) ATTRIBUTE_HIDDEN;
extern bfd *_bfd_get_elt_at_filepos
  (bfd *, file_ptr, struct bfd_link_info *) ATTRIBUTE_HIDDEN;
extern bfd *_bfd_generic_get_elt_at_index
  (bfd *, symindex) ATTRIBUTE_HIDDEN;

extern bool _bfd_bool_bfd_false
  (bfd *) ATTRIBUTE_HIDDEN;
extern bool _bfd_bool_bfd_asymbol_false
  (bfd *, asymbol *) ATTRIBUTE_HIDDEN;
extern bool _bfd_bool_bfd_false_error
  (bfd *) ATTRIBUTE_HIDDEN;
extern bool _bfd_bool_bfd_link_false_error
  (bfd *, struct bfd_link_info *) ATTRIBUTE_HIDDEN;
extern bool _bfd_bool_bfd_true
  (bfd *) ATTRIBUTE_HIDDEN;
extern bool _bfd_bool_bfd_link_true
  (bfd *, struct bfd_link_info *) ATTRIBUTE_HIDDEN;
extern bool _bfd_bool_bfd_bfd_true
  (bfd *, bfd *) ATTRIBUTE_HIDDEN;
extern bool _bfd_bool_bfd_uint_true
  (bfd *, unsigned int) ATTRIBUTE_HIDDEN;
extern bool _bfd_bool_bfd_asection_bfd_asection_true
  (bfd *, asection *, bfd *, asection *) ATTRIBUTE_HIDDEN;
extern bool _bfd_bool_bfd_asymbol_bfd_asymbol_true
  (bfd *, asymbol *, bfd *, asymbol *) ATTRIBUTE_HIDDEN;
extern bool _bfd_bool_bfd_ptr_true
  (bfd *, void *) ATTRIBUTE_HIDDEN;
extern void *_bfd_ptr_bfd_null_error
  (bfd *) ATTRIBUTE_HIDDEN;
extern int _bfd_int_bfd_0
  (bfd *) ATTRIBUTE_HIDDEN;
extern unsigned int _bfd_uint_bfd_0
  (bfd *) ATTRIBUTE_HIDDEN;
extern long _bfd_long_bfd_0
  (bfd *) ATTRIBUTE_HIDDEN;
extern long _bfd_long_bfd_n1_error
  (bfd *) ATTRIBUTE_HIDDEN;
extern void _bfd_void_bfd
  (bfd *) ATTRIBUTE_HIDDEN;
extern void _bfd_void_bfd_link
  (bfd *, struct bfd_link_info *) ATTRIBUTE_HIDDEN;
extern void _bfd_void_bfd_asection
  (bfd *, asection *) ATTRIBUTE_HIDDEN;

extern bfd_cleanup _bfd_dummy_target
  (bfd *) ATTRIBUTE_HIDDEN;
#define _bfd_no_cleanup _bfd_void_bfd

extern void bfd_dont_truncate_arname
  (bfd *, const char *, char *) ATTRIBUTE_HIDDEN;
extern void bfd_bsd_truncate_arname
  (bfd *, const char *, char *) ATTRIBUTE_HIDDEN;
extern void bfd_gnu_truncate_arname
  (bfd *, const char *, char *) ATTRIBUTE_HIDDEN;

extern bool _bfd_bsd_write_armap
  (bfd *, unsigned int, struct orl *, unsigned int, int) ATTRIBUTE_HIDDEN;

extern bool _bfd_coff_write_armap
  (bfd *, unsigned int, struct orl *, unsigned int, int) ATTRIBUTE_HIDDEN;

extern void *_bfd_generic_read_ar_hdr
  (bfd *) ATTRIBUTE_HIDDEN;
extern void _bfd_ar_spacepad
  (char *, size_t, const char *, long) ATTRIBUTE_HIDDEN;
extern bool _bfd_ar_sizepad
  (char *, size_t, bfd_size_type) ATTRIBUTE_HIDDEN;

extern void *_bfd_generic_read_ar_hdr_mag
  (bfd *, const char *) ATTRIBUTE_HIDDEN;

extern bool _bfd_generic_write_ar_hdr
  (bfd *, bfd *) ATTRIBUTE_HIDDEN;

extern bool _bfd_bsd44_write_ar_hdr
  (bfd *, bfd *) ATTRIBUTE_HIDDEN;

extern bfd * bfd_generic_openr_next_archived_file
  (bfd *, bfd *) ATTRIBUTE_HIDDEN;

extern int bfd_generic_stat_arch_elt
  (bfd *, struct stat *) ATTRIBUTE_HIDDEN;

#define _bfd_read_ar_hdr(abfd) \
	BFD_SEND (abfd, _bfd_read_ar_hdr_fn, (abfd))
#define _bfd_write_ar_hdr(archive, abfd)	 \
	BFD_SEND (abfd, _bfd_write_ar_hdr_fn, (archive, abfd))

/* Generic routines to use for BFD_JUMP_TABLE_GENERIC.  Use
   BFD_JUMP_TABLE_GENERIC (_bfd_generic).  */

#define _bfd_generic_close_and_cleanup _bfd_archive_close_and_cleanup
extern bool _bfd_archive_close_and_cleanup
  (bfd *) ATTRIBUTE_HIDDEN;
extern void _bfd_unlink_from_archive_parent (bfd *) ATTRIBUTE_HIDDEN;
#define _bfd_generic_bfd_free_cached_info _bfd_free_cached_info
extern bool _bfd_generic_new_section_hook
  (bfd *, asection *) ATTRIBUTE_HIDDEN;
extern bool _bfd_generic_get_section_contents
  (bfd *, asection *, void *, file_ptr, bfd_size_type) ATTRIBUTE_HIDDEN;
extern bool _bfd_generic_get_section_contents_in_window
  (bfd *, asection *, bfd_window *, file_ptr, bfd_size_type) ATTRIBUTE_HIDDEN;

/* Generic routines to use for BFD_JUMP_TABLE_COPY.  Use
   BFD_JUMP_TABLE_COPY (_bfd_generic).  */

#define _bfd_generic_bfd_copy_private_bfd_data _bfd_bool_bfd_bfd_true
#define _bfd_generic_bfd_merge_private_bfd_data \
  _bfd_bool_bfd_link_true
#define _bfd_generic_bfd_set_private_flags _bfd_bool_bfd_uint_true
#define _bfd_generic_bfd_copy_private_section_data \
  _bfd_bool_bfd_asection_bfd_asection_true
#define _bfd_generic_bfd_copy_private_symbol_data \
  _bfd_bool_bfd_asymbol_bfd_asymbol_true
#define _bfd_generic_bfd_copy_private_header_data _bfd_bool_bfd_bfd_true
#define _bfd_generic_bfd_print_private_bfd_data _bfd_bool_bfd_ptr_true

extern bool _bfd_generic_init_private_section_data
  (bfd *, asection *, bfd *, asection *, struct bfd_link_info *)
  ATTRIBUTE_HIDDEN;

/* Routines to use for BFD_JUMP_TABLE_CORE when there is no core file
   support.  Use BFD_JUMP_TABLE_CORE (_bfd_nocore).  */

extern char *_bfd_nocore_core_file_failing_command
  (bfd *) ATTRIBUTE_HIDDEN;
extern int _bfd_nocore_core_file_failing_signal
  (bfd *) ATTRIBUTE_HIDDEN;
extern bool _bfd_nocore_core_file_matches_executable_p
  (bfd *, bfd *) ATTRIBUTE_HIDDEN;
extern int _bfd_nocore_core_file_pid
  (bfd *) ATTRIBUTE_HIDDEN;

/* Routines to use for BFD_JUMP_TABLE_ARCHIVE when there is no archive
   file support.  Use BFD_JUMP_TABLE_ARCHIVE (_bfd_noarchive).  */

#define _bfd_noarchive_slurp_armap _bfd_bool_bfd_false_error
#define _bfd_noarchive_slurp_extended_name_table _bfd_bool_bfd_false_error
extern bool _bfd_noarchive_construct_extended_name_table
  (bfd *, char **, bfd_size_type *, const char **) ATTRIBUTE_HIDDEN;
extern void _bfd_noarchive_truncate_arname
  (bfd *, const char *, char *) ATTRIBUTE_HIDDEN;
extern bool _bfd_noarchive_write_armap
  (bfd *, unsigned int, struct orl *, unsigned int, int) ATTRIBUTE_HIDDEN;
#define _bfd_noarchive_read_ar_hdr _bfd_ptr_bfd_null_error
extern bool _bfd_noarchive_write_ar_hdr
  (bfd *, bfd *) ATTRIBUTE_HIDDEN;
extern bfd *
_bfd_noarchive_openr_next_archived_file
  (bfd *, bfd *) ATTRIBUTE_HIDDEN;
extern bfd * _bfd_noarchive_get_elt_at_index
  (bfd *, symindex) ATTRIBUTE_HIDDEN;
#define _bfd_noarchive_generic_stat_arch_elt bfd_generic_stat_arch_elt
#define _bfd_noarchive_update_armap_timestamp _bfd_bool_bfd_false_error

/* Routines to use for BFD_JUMP_TABLE_ARCHIVE to get BSD style
   archives.  Use BFD_JUMP_TABLE_ARCHIVE (_bfd_archive_bsd).  */

#define _bfd_archive_bsd_slurp_armap bfd_slurp_bsd_armap
#define _bfd_archive_bsd_slurp_extended_name_table \
  _bfd_slurp_extended_name_table
extern bool _bfd_archive_bsd_construct_extended_name_table
  (bfd *, char **, bfd_size_type *, const char **) ATTRIBUTE_HIDDEN;
#define _bfd_archive_bsd_truncate_arname bfd_bsd_truncate_arname
#define _bfd_archive_bsd_write_armap _bfd_bsd_write_armap
#define _bfd_archive_bsd_read_ar_hdr _bfd_generic_read_ar_hdr
#define _bfd_archive_bsd_write_ar_hdr _bfd_generic_write_ar_hdr
#define _bfd_archive_bsd_openr_next_archived_file \
  bfd_generic_openr_next_archived_file
#define _bfd_archive_bsd_get_elt_at_index _bfd_generic_get_elt_at_index
#define _bfd_archive_bsd_generic_stat_arch_elt \
  bfd_generic_stat_arch_elt
extern bool _bfd_archive_bsd_update_armap_timestamp
  (bfd *) ATTRIBUTE_HIDDEN;

/* Routines to use for BFD_JUMP_TABLE_ARCHIVE to get COFF style
   archives.  Use BFD_JUMP_TABLE_ARCHIVE (_bfd_archive_coff).  */

#define _bfd_archive_coff_slurp_armap bfd_slurp_coff_armap
#define _bfd_archive_coff_slurp_extended_name_table \
  _bfd_slurp_extended_name_table
extern bool _bfd_archive_coff_construct_extended_name_table
  (bfd *, char **, bfd_size_type *, const char **) ATTRIBUTE_HIDDEN;
#define _bfd_archive_coff_truncate_arname bfd_dont_truncate_arname
#define _bfd_archive_coff_write_armap _bfd_coff_write_armap
#define _bfd_archive_coff_read_ar_hdr _bfd_generic_read_ar_hdr
#define _bfd_archive_coff_write_ar_hdr _bfd_generic_write_ar_hdr
#define _bfd_archive_coff_openr_next_archived_file \
  bfd_generic_openr_next_archived_file
#define _bfd_archive_coff_get_elt_at_index _bfd_generic_get_elt_at_index
#define _bfd_archive_coff_generic_stat_arch_elt \
  bfd_generic_stat_arch_elt
#define _bfd_archive_coff_update_armap_timestamp _bfd_bool_bfd_true

/* Routines to use for BFD_JUMP_TABLE_ARCHIVE to get BSD4.4 style
   archives.  Use BFD_JUMP_TABLE_ARCHIVE (_bfd_archive_bsd44).  */

#define _bfd_archive_bsd44_slurp_armap bfd_slurp_bsd_armap
#define _bfd_archive_bsd44_slurp_extended_name_table \
  _bfd_slurp_extended_name_table
extern bool _bfd_archive_bsd44_construct_extended_name_table
  (bfd *, char **, bfd_size_type *, const char **) ATTRIBUTE_HIDDEN;
#define _bfd_archive_bsd44_truncate_arname bfd_bsd_truncate_arname
#define _bfd_archive_bsd44_write_armap _bfd_bsd_write_armap
#define _bfd_archive_bsd44_read_ar_hdr _bfd_generic_read_ar_hdr
#define _bfd_archive_bsd44_write_ar_hdr _bfd_bsd44_write_ar_hdr
#define _bfd_archive_bsd44_openr_next_archived_file \
  bfd_generic_openr_next_archived_file
#define _bfd_archive_bsd44_get_elt_at_index _bfd_generic_get_elt_at_index
#define _bfd_archive_bsd44_generic_stat_arch_elt \
  bfd_generic_stat_arch_elt
#define _bfd_archive_bsd44_update_armap_timestamp \
  _bfd_archive_bsd_update_armap_timestamp

/* Routines to use for BFD_JUMP_TABLE_ARCHIVE to get VMS style
   archives.  Use BFD_JUMP_TABLE_ARCHIVE (_bfd_vms_lib).  Some of them
   are irrelevant.  */

extern bool _bfd_vms_lib_write_archive_contents
  (bfd *) ATTRIBUTE_HIDDEN;
#define _bfd_vms_lib_slurp_armap _bfd_noarchive_slurp_armap
#define _bfd_vms_lib_slurp_extended_name_table \
  _bfd_noarchive_slurp_extended_name_table
#define _bfd_vms_lib_construct_extended_name_table \
  _bfd_noarchive_construct_extended_name_table
#define _bfd_vms_lib_truncate_arname _bfd_noarchive_truncate_arname
#define _bfd_vms_lib_write_armap _bfd_noarchive_write_armap
#define _bfd_vms_lib_read_ar_hdr _bfd_noarchive_read_ar_hdr
#define _bfd_vms_lib_write_ar_hdr _bfd_noarchive_write_ar_hdr
extern bfd *_bfd_vms_lib_openr_next_archived_file
  (bfd *, bfd *) ATTRIBUTE_HIDDEN;
extern bfd *_bfd_vms_lib_get_elt_at_index
  (bfd *, symindex) ATTRIBUTE_HIDDEN;
extern int _bfd_vms_lib_generic_stat_arch_elt
  (bfd *, struct stat *) ATTRIBUTE_HIDDEN;
#define _bfd_vms_lib_update_armap_timestamp _bfd_bool_bfd_true

/* Extra routines for VMS style archives.  */

extern symindex _bfd_vms_lib_find_symbol
  (bfd *, const char *) ATTRIBUTE_HIDDEN;
extern bfd *_bfd_vms_lib_get_imagelib_file
  (bfd *) ATTRIBUTE_HIDDEN;
extern bfd_cleanup _bfd_vms_lib_alpha_archive_p
  (bfd *) ATTRIBUTE_HIDDEN;
extern bfd_cleanup _bfd_vms_lib_ia64_archive_p
  (bfd *) ATTRIBUTE_HIDDEN;
extern bool _bfd_vms_lib_alpha_mkarchive
  (bfd *) ATTRIBUTE_HIDDEN;
extern bool _bfd_vms_lib_ia64_mkarchive
  (bfd *) ATTRIBUTE_HIDDEN;

/* Routines to use for BFD_JUMP_TABLE_SYMBOLS where there is no symbol
   support.  Use BFD_JUMP_TABLE_SYMBOLS (_bfd_nosymbols).  */

#define _bfd_nosymbols_get_symtab_upper_bound _bfd_long_bfd_n1_error
extern long _bfd_nosymbols_canonicalize_symtab
  (bfd *, asymbol **) ATTRIBUTE_HIDDEN;
#define _bfd_nosymbols_make_empty_symbol _bfd_generic_make_empty_symbol
extern void _bfd_nosymbols_print_symbol
  (bfd *, void *, asymbol *, bfd_print_symbol_type) ATTRIBUTE_HIDDEN;
extern void _bfd_nosymbols_get_symbol_info
  (bfd *, asymbol *, symbol_info *) ATTRIBUTE_HIDDEN;
extern const char * _bfd_nosymbols_get_symbol_version_string
  (bfd *, asymbol *, bool, bool *) ATTRIBUTE_HIDDEN;
extern bool _bfd_nosymbols_bfd_is_local_label_name
  (bfd *, const char *) ATTRIBUTE_HIDDEN;
#define _bfd_nosymbols_bfd_is_target_special_symbol _bfd_bool_bfd_asymbol_false
extern alent *_bfd_nosymbols_get_lineno
  (bfd *, asymbol *) ATTRIBUTE_HIDDEN;
extern bool _bfd_nosymbols_find_nearest_line
  (bfd *, asymbol **, asection *, bfd_vma,
   const char **, const char **, unsigned int *, unsigned int *)
  ATTRIBUTE_HIDDEN;
extern bool _bfd_nosymbols_find_nearest_line_with_alt
  (bfd *, const char *, asymbol **, asection *, bfd_vma,
   const char **, const char **, unsigned int *, unsigned int *)
  ATTRIBUTE_HIDDEN;
extern bool _bfd_nosymbols_find_line
  (bfd *, asymbol **, asymbol *, const char **, unsigned int *)
  ATTRIBUTE_HIDDEN;
extern bool _bfd_nosymbols_find_inliner_info
  (bfd *, const char **, const char **, unsigned int *) ATTRIBUTE_HIDDEN;
extern asymbol *_bfd_nosymbols_bfd_make_debug_symbol
  (bfd *) ATTRIBUTE_HIDDEN;
extern long _bfd_nosymbols_read_minisymbols
  (bfd *, bool, void **, unsigned int *) ATTRIBUTE_HIDDEN;
extern asymbol *_bfd_nosymbols_minisymbol_to_symbol
  (bfd *, bool, const void *, asymbol *) ATTRIBUTE_HIDDEN;

/* Routines to use for BFD_JUMP_TABLE_RELOCS when there is no reloc
   support.  Use BFD_JUMP_TABLE_RELOCS (_bfd_norelocs).  */

extern long _bfd_norelocs_get_reloc_upper_bound
  (bfd *, asection *) ATTRIBUTE_HIDDEN;
extern long _bfd_norelocs_canonicalize_reloc
  (bfd *, asection *, arelent **, asymbol **) ATTRIBUTE_HIDDEN;
extern void _bfd_norelocs_set_reloc
  (bfd *, asection *, arelent **, unsigned int) ATTRIBUTE_HIDDEN;
extern reloc_howto_type *_bfd_norelocs_bfd_reloc_type_lookup
  (bfd *, bfd_reloc_code_real_type) ATTRIBUTE_HIDDEN;
extern reloc_howto_type *_bfd_norelocs_bfd_reloc_name_lookup
  (bfd *, const char *) ATTRIBUTE_HIDDEN;

/* Routines to use for BFD_JUMP_TABLE_WRITE for targets which may not
   be written.  Use BFD_JUMP_TABLE_WRITE (_bfd_nowrite).  */

extern bool _bfd_nowrite_set_arch_mach
  (bfd *, enum bfd_architecture, unsigned long) ATTRIBUTE_HIDDEN;
extern bool _bfd_nowrite_set_section_contents
  (bfd *, asection *, const void *, file_ptr, bfd_size_type) ATTRIBUTE_HIDDEN;

/* Generic routines to use for BFD_JUMP_TABLE_WRITE.  Use
   BFD_JUMP_TABLE_WRITE (_bfd_generic).  */

#define _bfd_generic_set_arch_mach bfd_default_set_arch_mach
extern bool _bfd_generic_set_section_contents
  (bfd *, asection *, const void *, file_ptr, bfd_size_type) ATTRIBUTE_HIDDEN;

/* Routines to use for BFD_JUMP_TABLE_LINK for targets which do not
   support linking.  Use BFD_JUMP_TABLE_LINK (_bfd_nolink).  */

extern int _bfd_nolink_sizeof_headers
  (bfd *, struct bfd_link_info *) ATTRIBUTE_HIDDEN;
extern bfd_byte *_bfd_nolink_bfd_get_relocated_section_contents
  (bfd *, struct bfd_link_info *, struct bfd_link_order *,
   bfd_byte *, bool, asymbol **) ATTRIBUTE_HIDDEN;
extern bool _bfd_nolink_bfd_relax_section
  (bfd *, asection *, struct bfd_link_info *, bool *) ATTRIBUTE_HIDDEN;
#define _bfd_nolink_bfd_gc_sections _bfd_bool_bfd_link_false_error
extern bool _bfd_nolink_bfd_lookup_section_flags
  (struct bfd_link_info *, struct flag_info *, asection *) ATTRIBUTE_HIDDEN;
#define _bfd_nolink_bfd_merge_sections _bfd_bool_bfd_link_false_error
extern bool _bfd_nolink_bfd_is_group_section
  (bfd *, const asection *) ATTRIBUTE_HIDDEN;
extern const char *_bfd_nolink_bfd_group_name
  (bfd *, const asection *) ATTRIBUTE_HIDDEN;
extern bool _bfd_nolink_bfd_discard_group
  (bfd *, asection *) ATTRIBUTE_HIDDEN;
extern struct bfd_link_hash_table *_bfd_nolink_bfd_link_hash_table_create
  (bfd *) ATTRIBUTE_HIDDEN;
#define _bfd_nolink_bfd_link_add_symbols _bfd_bool_bfd_link_false_error
extern void _bfd_nolink_bfd_link_just_syms
  (asection *, struct bfd_link_info *) ATTRIBUTE_HIDDEN;
extern void _bfd_nolink_bfd_copy_link_hash_symbol_type
  (bfd *, struct bfd_link_hash_entry *, struct bfd_link_hash_entry *)
  ATTRIBUTE_HIDDEN;
#define _bfd_nolink_bfd_final_link _bfd_bool_bfd_link_false_error
extern bool _bfd_nolink_bfd_link_split_section
  (bfd *, struct bfd_section *) ATTRIBUTE_HIDDEN;
extern bool _bfd_nolink_section_already_linked
  (bfd *, asection *, struct bfd_link_info *) ATTRIBUTE_HIDDEN;
extern bool _bfd_nolink_bfd_define_common_symbol
  (bfd *, struct bfd_link_info *, struct bfd_link_hash_entry *)
  ATTRIBUTE_HIDDEN;
#define _bfd_nolink_bfd_link_hide_symbol \
  _bfd_generic_link_hide_symbol
extern struct bfd_link_hash_entry *_bfd_nolink_bfd_define_start_stop
  (struct bfd_link_info *, const char *, asection *) ATTRIBUTE_HIDDEN;
#define _bfd_nolink_bfd_link_check_relocs \
  _bfd_generic_link_check_relocs

/* Routines to use for BFD_JUMP_TABLE_DYNAMIC for targets which do not
   have dynamic symbols or relocs.  Use BFD_JUMP_TABLE_DYNAMIC
   (_bfd_nodynamic).  */

#define _bfd_nodynamic_get_dynamic_symtab_upper_bound _bfd_long_bfd_n1_error
#define _bfd_nodynamic_canonicalize_dynamic_symtab \
  _bfd_nosymbols_canonicalize_symtab
extern long _bfd_nodynamic_get_synthetic_symtab
  (bfd *, long, asymbol **, long, asymbol **, asymbol **) ATTRIBUTE_HIDDEN;
#define _bfd_nodynamic_get_dynamic_reloc_upper_bound _bfd_long_bfd_n1_error
extern long _bfd_nodynamic_canonicalize_dynamic_reloc
  (bfd *, arelent **, asymbol **) ATTRIBUTE_HIDDEN;

/* Generic routine to determine of the given symbol is a local
   label.  */
extern bool bfd_generic_is_local_label_name
  (bfd *, const char *) ATTRIBUTE_HIDDEN;

/* Generic minisymbol routines.  */
extern long _bfd_generic_read_minisymbols
  (bfd *, bool, void **, unsigned int *) ATTRIBUTE_HIDDEN;
extern asymbol *_bfd_generic_minisymbol_to_symbol
  (bfd *, bool, const void *, asymbol *) ATTRIBUTE_HIDDEN;

/* Find the nearest line using .stab/.stabstr sections.  */
extern bool _bfd_stab_section_find_nearest_line
  (bfd *, asymbol **, asection *, bfd_vma, bool *,
   const char **, const char **, unsigned int *, void **) ATTRIBUTE_HIDDEN;

/* Find the nearest line using DWARF 1 debugging information.  */
extern bool _bfd_dwarf1_find_nearest_line
  (bfd *, asymbol **, asection *, bfd_vma,
   const char **, const char **, unsigned int *) ATTRIBUTE_HIDDEN;

/* Clean up the data used to handle DWARF 1 debugging information. */
extern void _bfd_dwarf1_cleanup_debug_info
  (bfd *, void **) ATTRIBUTE_HIDDEN;

struct dwarf_debug_section
{
  const char * uncompressed_name;
  const char * compressed_name;
};

/* Map of uncompressed DWARF debug section name to compressed one.  It
   is terminated by NULL uncompressed_name.  */

extern const struct dwarf_debug_section dwarf_debug_sections[] ATTRIBUTE_HIDDEN;

/* Find the nearest line using DWARF 2 debugging information.  */
extern int _bfd_dwarf2_find_nearest_line
  (bfd *, asymbol **, asymbol *, asection *, bfd_vma,
   const char **, const char **, unsigned int *, unsigned int *,
   const struct dwarf_debug_section *, void **) ATTRIBUTE_HIDDEN;

/* Find the nearest line using DWARF 2 debugging information, with
   the option of specifying a .gnu_debugaltlink file.  */
extern int _bfd_dwarf2_find_nearest_line_with_alt
  (bfd *, const char *, asymbol **, asymbol *, asection *, bfd_vma,
   const char **, const char **, unsigned int *, unsigned int *,
   const struct dwarf_debug_section *, void **) ATTRIBUTE_HIDDEN;

/* Find the bias between DWARF addresses and real addresses.  */
extern bfd_signed_vma _bfd_dwarf2_find_symbol_bias
  (asymbol **, void **) ATTRIBUTE_HIDDEN;

/* Find inliner info after calling bfd_find_nearest_line. */
extern bool _bfd_dwarf2_find_inliner_info
  (bfd *, const char **, const char **, unsigned int *, void **)
  ATTRIBUTE_HIDDEN;

/* Read DWARF 2 debugging information. */
extern bool _bfd_dwarf2_slurp_debug_info
  (bfd *, bfd *, const struct dwarf_debug_section *, asymbol **, void **,
   bool) ATTRIBUTE_HIDDEN;

/* Clean up the data used to handle DWARF 2 debugging information. */
extern void _bfd_dwarf2_cleanup_debug_info
  (bfd *, void **) ATTRIBUTE_HIDDEN;

extern void _bfd_stab_cleanup
  (bfd *, void **) ATTRIBUTE_HIDDEN;

/* Create a new section entry.  */
extern struct bfd_hash_entry *bfd_section_hash_newfunc
  (struct bfd_hash_entry *, struct bfd_hash_table *, const char *)
  ATTRIBUTE_HIDDEN;

/* A routine to create entries for a bfd_link_hash_table.  */
extern struct bfd_hash_entry *_bfd_link_hash_newfunc
  (struct bfd_hash_entry *entry, struct bfd_hash_table *table,
   const char *string) ATTRIBUTE_HIDDEN;

/* Initialize a bfd_link_hash_table.  */
extern bool _bfd_link_hash_table_init
  (struct bfd_link_hash_table *, bfd *,
   struct bfd_hash_entry *(*) (struct bfd_hash_entry *,
			       struct bfd_hash_table *,
			       const char *),
   unsigned int) ATTRIBUTE_HIDDEN;

/* Generic link hash table creation routine.  */
extern struct bfd_link_hash_table *_bfd_generic_link_hash_table_create
  (bfd *) ATTRIBUTE_HIDDEN;

/* Generic link hash table destruction routine.  */
extern void _bfd_generic_link_hash_table_free
  (bfd *) ATTRIBUTE_HIDDEN;

/* Generic add symbol routine.  */
extern bool _bfd_generic_link_add_symbols
  (bfd *, struct bfd_link_info *) ATTRIBUTE_HIDDEN;

/* Generic archive add symbol routine.  */
extern bool _bfd_generic_link_add_archive_symbols
  (bfd *, struct bfd_link_info *,
   bool (*) (bfd *, struct bfd_link_info *,
		    struct bfd_link_hash_entry *, const char *,
		    bool *)) ATTRIBUTE_HIDDEN;

/* Forward declaration to avoid prototype errors.  */
typedef struct bfd_link_hash_entry _bfd_link_hash_entry;

/* Generic routine to add a single symbol.  */
extern bool _bfd_generic_link_add_one_symbol
  (struct bfd_link_info *, bfd *, const char *name, flagword,
   asection *, bfd_vma, const char *, bool copy,
   bool constructor, struct bfd_link_hash_entry **) ATTRIBUTE_HIDDEN;

/* Generic routine to mark section as supplying symbols only.  */
extern void _bfd_generic_link_just_syms
  (asection *, struct bfd_link_info *) ATTRIBUTE_HIDDEN;

/* Generic routine that does nothing.  */
extern void _bfd_generic_copy_link_hash_symbol_type
  (bfd *, struct bfd_link_hash_entry *, struct bfd_link_hash_entry *)
  ATTRIBUTE_HIDDEN;

/* Generic link routine.  */
extern bool _bfd_generic_final_link
  (bfd *, struct bfd_link_info *) ATTRIBUTE_HIDDEN;

extern bool _bfd_generic_link_split_section
  (bfd *, struct bfd_section *) ATTRIBUTE_HIDDEN;

extern bool _bfd_generic_section_already_linked
  (bfd *, asection *, struct bfd_link_info *) ATTRIBUTE_HIDDEN;

/* Generic reloc_link_order processing routine.  */
extern bool _bfd_generic_reloc_link_order
  (bfd *, struct bfd_link_info *, asection *, struct bfd_link_order *)
  ATTRIBUTE_HIDDEN;

/* Default link order processing routine.  */
extern bool _bfd_default_link_order
  (bfd *, struct bfd_link_info *, asection *, struct bfd_link_order *)
  ATTRIBUTE_HIDDEN;

/* Count the number of reloc entries in a link order list.  */
extern unsigned int _bfd_count_link_order_relocs
  (struct bfd_link_order *) ATTRIBUTE_HIDDEN;

/* Final link relocation routine.  */
extern bfd_reloc_status_type _bfd_final_link_relocate
  (reloc_howto_type *, bfd *, asection *, bfd_byte *,
   bfd_vma, bfd_vma, bfd_vma) ATTRIBUTE_HIDDEN;

/* Relocate a particular location by a howto and a value.  */
extern bfd_reloc_status_type _bfd_relocate_contents
  (reloc_howto_type *, bfd *, bfd_vma, bfd_byte *) ATTRIBUTE_HIDDEN;

/* Clear a given location using a given howto.  */
extern bfd_reloc_status_type _bfd_clear_contents
  (reloc_howto_type *, bfd *, asection *, bfd_byte *, bfd_vma) ATTRIBUTE_HIDDEN;

/* Register a SEC_MERGE section as a candidate for merging.  */

extern bool _bfd_add_merge_section
  (bfd *, void **, asection *, void **) ATTRIBUTE_HIDDEN;

/* Attempt to merge SEC_MERGE sections.  */

extern bool _bfd_merge_sections
  (bfd *, struct bfd_link_info *, void *, void (*) (bfd *, asection *))
  ATTRIBUTE_HIDDEN;

/* Write out a merged section.  */

extern bool _bfd_write_merged_section
  (bfd *, asection *, void *) ATTRIBUTE_HIDDEN;

/* Find an offset within a modified SEC_MERGE section.  */

extern bfd_vma _bfd_merged_section_offset
  (bfd *, asection **, void *, bfd_vma) ATTRIBUTE_HIDDEN;

/* Tidy up when done.  */

extern void _bfd_merge_sections_free (void *) ATTRIBUTE_HIDDEN;

/* Macros to tell if bfds are read or write enabled.

   Note that bfds open for read may be scribbled into if the fd passed
   to bfd_fdopenr is actually open both for read and write
   simultaneously.  However an output bfd will never be open for
   read.  Therefore sometimes you want to check bfd_read_p or
   !bfd_read_p, and only sometimes bfd_write_p.
*/

#define	bfd_read_p(abfd) \
  ((abfd)->direction == read_direction || (abfd)->direction == both_direction)
#define	bfd_write_p(abfd) \
  ((abfd)->direction == write_direction || (abfd)->direction == both_direction)

extern void bfd_assert
  (const char*,int) ATTRIBUTE_HIDDEN;

#define BFD_ASSERT(x) \
  do { if (!(x)) bfd_assert(__FILE__,__LINE__); } while (0)

#define BFD_FAIL() \
  do { bfd_assert(__FILE__,__LINE__); } while (0)

extern void _bfd_abort
  (const char *, int, const char *) ATTRIBUTE_NORETURN ATTRIBUTE_HIDDEN;

/* if gcc >= 2.6, we can give a function name, too */
#if __GNUC__ < 2 || (__GNUC__ == 2 && __GNUC_MINOR__ < 6)
#define __PRETTY_FUNCTION__  ((char *) NULL)
#endif

#undef abort
#define abort() _bfd_abort (__FILE__, __LINE__, __PRETTY_FUNCTION__)

/* Manipulate a system FILE but using BFD's "file_ptr", rather than
   the system "off_t" or "off64_t", as the offset.  */
extern file_ptr _bfd_real_ftell
  (FILE *) ATTRIBUTE_HIDDEN;
extern int _bfd_real_fseek
  (FILE *, file_ptr, int) ATTRIBUTE_HIDDEN;
extern FILE *_bfd_real_fopen
  (const char *, const char *) ATTRIBUTE_HIDDEN;

/* List of supported target vectors, and the default vector (if
   bfd_default_vector[0] is NULL, there is no default).  */
extern const bfd_target *const *const bfd_target_vector ATTRIBUTE_HIDDEN;
extern const bfd_target *bfd_default_vector[] ATTRIBUTE_HIDDEN;

/* List of associated target vectors.  */
extern const bfd_target *const *const bfd_associated_vector ATTRIBUTE_HIDDEN;

/* Functions shared by the ECOFF and MIPS ELF backends, which have no
   other common header files.  */

struct ecoff_debug_info;
struct ecoff_debug_swap;
struct ecoff_extr;
struct ecoff_find_line;

extern void _bfd_ecoff_free_ecoff_debug_info
  (struct ecoff_debug_info *debug);
extern bool _bfd_ecoff_locate_line
  (bfd *, asection *, bfd_vma, struct ecoff_debug_info * const,
   const struct ecoff_debug_swap * const, struct ecoff_find_line *,
   const char **, const char **, unsigned int *) ATTRIBUTE_HIDDEN;
extern bool _bfd_ecoff_get_accumulated_pdr
  (void *, bfd_byte *) ATTRIBUTE_HIDDEN;
extern bool _bfd_ecoff_get_accumulated_sym
  (void *, bfd_byte *) ATTRIBUTE_HIDDEN;
extern bool _bfd_ecoff_get_accumulated_ss
  (void *, bfd_byte *) ATTRIBUTE_HIDDEN;

extern bfd_vma _bfd_get_gp_value
  (bfd *) ATTRIBUTE_HIDDEN;
extern void _bfd_set_gp_value
  (bfd *, bfd_vma) ATTRIBUTE_HIDDEN;

/* Function shared by the COFF and ELF SH backends, which have no
   other common header files.  */

#ifndef _bfd_sh_align_load_span
extern bool _bfd_sh_align_load_span
  (bfd *, asection *, bfd_byte *,
   bool (*) (bfd *, asection *, void *, bfd_byte *, bfd_vma),
   void *, bfd_vma **, bfd_vma *, bfd_vma, bfd_vma, bool *) ATTRIBUTE_HIDDEN;
#endif

/* This is the shape of the elements inside the already_linked hash
   table. It maps a name onto a list of already_linked elements with
   the same name.  */

struct bfd_section_already_linked_hash_entry
{
  struct bfd_hash_entry root;
  struct bfd_section_already_linked *entry;
};

struct bfd_section_already_linked
{
  struct bfd_section_already_linked *next;
  asection *sec;
};

extern struct bfd_section_already_linked_hash_entry *
  bfd_section_already_linked_table_lookup (const char *) ATTRIBUTE_HIDDEN;
extern bool bfd_section_already_linked_table_insert
  (struct bfd_section_already_linked_hash_entry *, asection *)
  ATTRIBUTE_HIDDEN;
extern void bfd_section_already_linked_table_traverse
  (bool (*) (struct bfd_section_already_linked_hash_entry *,
		    void *), void *) ATTRIBUTE_HIDDEN;

extern bfd_vma _bfd_read_unsigned_leb128
  (bfd *, bfd_byte *, unsigned int *) ATTRIBUTE_HIDDEN;
extern bfd_signed_vma _bfd_read_signed_leb128
  (bfd *, bfd_byte *, unsigned int *) ATTRIBUTE_HIDDEN;
extern bfd_vma _bfd_safe_read_leb128
  (bfd *, bfd_byte **, bool, const bfd_byte * const) ATTRIBUTE_HIDDEN;
extern bfd_byte * _bfd_write_unsigned_leb128
  (bfd_byte *, bfd_byte *, bfd_vma) ATTRIBUTE_HIDDEN;

extern struct bfd_link_info *_bfd_get_link_info (bfd *);

extern bool _bfd_link_keep_memory (struct bfd_link_info *)
  ATTRIBUTE_HIDDEN;

#if GCC_VERSION >= 7000
#define _bfd_mul_overflow(a, b, res) __builtin_mul_overflow (a, b, res)
#else
/* Assumes unsigned values.  Careful!  Args evaluated multiple times.  */
#define _bfd_mul_overflow(a, b, res) \
  ((*res) = (a), (*res) *= (b), (b) != 0 && (*res) / (b) != (a))
#endif

#ifdef __GNUC__
#define _bfd_constant_p(v) __builtin_constant_p (v)
#else
#define _bfd_constant_p(v) 0
#endif

static inline void *
_bfd_alloc_and_read (bfd *abfd, bfd_size_type asize, bfd_size_type rsize)
{
  void *mem;
  if (!_bfd_constant_p (rsize))
    {
      ufile_ptr filesize = bfd_get_file_size (abfd);
      if (filesize != 0 && rsize > filesize)
	{
	  bfd_set_error (bfd_error_file_truncated);
	  return NULL;
	}
    }
  mem = bfd_alloc (abfd, asize);
  if (mem != NULL)
    {
      if (bfd_bread (mem, rsize, abfd) == rsize)
	return mem;
      bfd_release (abfd, mem);
    }
  return NULL;
}

static inline void *
_bfd_malloc_and_read (bfd *abfd, bfd_size_type asize, bfd_size_type rsize)
{
  void *mem;
  if (!_bfd_constant_p (rsize))
    {
      ufile_ptr filesize = bfd_get_file_size (abfd);
      if (filesize != 0 && rsize > filesize)
	{
	  bfd_set_error (bfd_error_file_truncated);
	  return NULL;
	}
    }
  mem = bfd_malloc (asize);
  if (mem != NULL)
    {
      if (bfd_bread (mem, rsize, abfd) == rsize)
	return mem;
      free (mem);
    }
  return NULL;
}
