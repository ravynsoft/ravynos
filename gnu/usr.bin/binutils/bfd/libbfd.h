/* DO NOT EDIT!  -*- buffer-read-only: t -*-  This file is automatically
   generated from "libbfd-in.h", "libbfd.c", "bfd.c", "bfdio.c",
   "archive.c", "archures.c", "bfdwin.c", "cache.c", "hash.c", "linker.c",
   "opncls.c", "reloc.c", "section.c", "stabs.c" and "targets.c".
   Run "make headers" in your build bfd/ to regenerate.  */

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
/* Extracted from libbfd.c.  */
void *bfd_malloc (bfd_size_type /*size*/) ATTRIBUTE_HIDDEN;

void *bfd_realloc (void */*mem*/, bfd_size_type /*size*/) ATTRIBUTE_HIDDEN;

void *bfd_realloc_or_free (void */*mem*/, bfd_size_type /*size*/) ATTRIBUTE_HIDDEN;

void *bfd_zmalloc (bfd_size_type /*size*/) ATTRIBUTE_HIDDEN;

bool bfd_write_bigendian_4byte_int (bfd *, unsigned int) ATTRIBUTE_HIDDEN;

unsigned int bfd_log2 (bfd_vma x) ATTRIBUTE_HIDDEN;

/* Extracted from bfd.c.  */
/* A buffer that is freed on bfd_close.  */
extern char *_bfd_error_buf;

char *bfd_asprintf (const char *fmt, ...) ATTRIBUTE_HIDDEN;

bfd_error_handler_type _bfd_set_error_handler_caching (bfd *) ATTRIBUTE_HIDDEN;

const char *_bfd_get_error_program_name (void) ATTRIBUTE_HIDDEN;

/* Extracted from bfdio.c.  */
struct bfd_iovec
{
  /* To avoid problems with macros, a "b" rather than "f"
     prefix is prepended to each method name.  */
  /* Attempt to read/write NBYTES on ABFD's IOSTREAM storing/fetching
     bytes starting at PTR.  Return the number of bytes actually
     transfered (a read past end-of-file returns less than NBYTES),
     or -1 (setting <<bfd_error>>) if an error occurs.  */
  file_ptr (*bread) (struct bfd *abfd, void *ptr, file_ptr nbytes);
  file_ptr (*bwrite) (struct bfd *abfd, const void *ptr,
		      file_ptr nbytes);
  /* Return the current IOSTREAM file offset, or -1 (setting <<bfd_error>>
     if an error occurs.  */
  file_ptr (*btell) (struct bfd *abfd);
  /* For the following, on successful completion a value of 0 is returned.
     Otherwise, a value of -1 is returned (and <<bfd_error>> is set).  */
  int (*bseek) (struct bfd *abfd, file_ptr offset, int whence);
  int (*bclose) (struct bfd *abfd);
  int (*bflush) (struct bfd *abfd);
  int (*bstat) (struct bfd *abfd, struct stat *sb);
  /* Mmap a part of the files. ADDR, LEN, PROT, FLAGS and OFFSET are the usual
     mmap parameter, except that LEN and OFFSET do not need to be page
     aligned.  Returns (void *)-1 on failure, mmapped address on success.
     Also write in MAP_ADDR the address of the page aligned buffer and in
     MAP_LEN the size mapped (a page multiple).  Use unmap with MAP_ADDR and
     MAP_LEN to unmap.  */
  void *(*bmmap) (struct bfd *abfd, void *addr, bfd_size_type len,
		  int prot, int flags, file_ptr offset,
		  void **map_addr, bfd_size_type *map_len);
};
extern const struct bfd_iovec _bfd_memory_iovec;

/* Extracted from archive.c.  */
/* Used in generating armaps (archive tables of contents).  */
struct orl             /* Output ranlib.  */
{
  char **name;         /* Symbol name.  */
  union
  {
    file_ptr pos;
    bfd *abfd;
  } u;                 /* bfd* or file position.  */
  int namidx;          /* Index into string table.  */
};

/* Extracted from archures.c.  */
extern const bfd_arch_info_type bfd_default_arch_struct;

const bfd_arch_info_type *bfd_default_compatible
   (const bfd_arch_info_type *a, const bfd_arch_info_type *b) ATTRIBUTE_HIDDEN;

bool bfd_default_scan
   (const struct bfd_arch_info *info, const char *string) ATTRIBUTE_HIDDEN;

void *bfd_arch_default_fill (bfd_size_type count,
    bool is_bigendian,
    bool code) ATTRIBUTE_HIDDEN;

/* Extracted from bfdwin.c.  */
typedef struct _bfd_window_internal
{
  struct _bfd_window_internal *next;
  void *data;
  bfd_size_type size;
  int refcount : 31;           /* should be enough...  */
  unsigned mapped : 1;         /* 1 = mmap, 0 = malloc */
}
bfd_window_internal;

/* Extracted from cache.c.  */
bool bfd_cache_init (bfd *abfd) ATTRIBUTE_HIDDEN;

FILE* bfd_open_file (bfd *abfd) ATTRIBUTE_HIDDEN;

/* Extracted from hash.c.  */
struct bfd_strtab_hash *_bfd_stringtab_init (void) ATTRIBUTE_HIDDEN;

struct bfd_strtab_hash *_bfd_xcoff_stringtab_init
   (bool /*isxcoff64*/) ATTRIBUTE_HIDDEN;

void _bfd_stringtab_free (struct bfd_strtab_hash *) ATTRIBUTE_HIDDEN;

bfd_size_type _bfd_stringtab_add
   (struct bfd_strtab_hash *, const char *,
    bool /*hash*/, bool /*copy*/) ATTRIBUTE_HIDDEN;

bfd_size_type _bfd_stringtab_size (struct bfd_strtab_hash *) ATTRIBUTE_HIDDEN;

bool _bfd_stringtab_emit (bfd *, struct bfd_strtab_hash *) ATTRIBUTE_HIDDEN;

/* Extracted from linker.c.  */
bool _bfd_generic_verify_endian_match
   (bfd *ibfd, struct bfd_link_info *info) ATTRIBUTE_HIDDEN;

/* Extracted from opncls.c.  */
bfd *_bfd_new_bfd (void) ATTRIBUTE_HIDDEN;

bfd *_bfd_new_bfd_contained_in (bfd *) ATTRIBUTE_HIDDEN;

bool _bfd_free_cached_info (bfd *) ATTRIBUTE_HIDDEN;

/* Extracted from reloc.c.  */
#ifdef _BFD_MAKE_TABLE_bfd_reloc_code_real

static const char *const bfd_reloc_code_real_names[] = { "@@uninitialized@@",

  "BFD_RELOC_64",
  "BFD_RELOC_32",
  "BFD_RELOC_26",
  "BFD_RELOC_24",
  "BFD_RELOC_16",
  "BFD_RELOC_14",
  "BFD_RELOC_8",
  "BFD_RELOC_64_PCREL",
  "BFD_RELOC_32_PCREL",
  "BFD_RELOC_24_PCREL",
  "BFD_RELOC_16_PCREL",
  "BFD_RELOC_12_PCREL",
  "BFD_RELOC_8_PCREL",
  "BFD_RELOC_32_SECREL",
  "BFD_RELOC_16_SECIDX",
  "BFD_RELOC_32_GOT_PCREL",
  "BFD_RELOC_16_GOT_PCREL",
  "BFD_RELOC_8_GOT_PCREL",
  "BFD_RELOC_32_GOTOFF",
  "BFD_RELOC_16_GOTOFF",
  "BFD_RELOC_LO16_GOTOFF",
  "BFD_RELOC_HI16_GOTOFF",
  "BFD_RELOC_HI16_S_GOTOFF",
  "BFD_RELOC_8_GOTOFF",
  "BFD_RELOC_64_PLT_PCREL",
  "BFD_RELOC_32_PLT_PCREL",
  "BFD_RELOC_24_PLT_PCREL",
  "BFD_RELOC_16_PLT_PCREL",
  "BFD_RELOC_8_PLT_PCREL",
  "BFD_RELOC_64_PLTOFF",
  "BFD_RELOC_32_PLTOFF",
  "BFD_RELOC_16_PLTOFF",
  "BFD_RELOC_LO16_PLTOFF",
  "BFD_RELOC_HI16_PLTOFF",
  "BFD_RELOC_HI16_S_PLTOFF",
  "BFD_RELOC_8_PLTOFF",
  "BFD_RELOC_SIZE32",
  "BFD_RELOC_SIZE64",
  "BFD_RELOC_68K_GLOB_DAT",
  "BFD_RELOC_68K_JMP_SLOT",
  "BFD_RELOC_68K_RELATIVE",
  "BFD_RELOC_68K_TLS_GD32",
  "BFD_RELOC_68K_TLS_GD16",
  "BFD_RELOC_68K_TLS_GD8",
  "BFD_RELOC_68K_TLS_LDM32",
  "BFD_RELOC_68K_TLS_LDM16",
  "BFD_RELOC_68K_TLS_LDM8",
  "BFD_RELOC_68K_TLS_LDO32",
  "BFD_RELOC_68K_TLS_LDO16",
  "BFD_RELOC_68K_TLS_LDO8",
  "BFD_RELOC_68K_TLS_IE32",
  "BFD_RELOC_68K_TLS_IE16",
  "BFD_RELOC_68K_TLS_IE8",
  "BFD_RELOC_68K_TLS_LE32",
  "BFD_RELOC_68K_TLS_LE16",
  "BFD_RELOC_68K_TLS_LE8",
  "BFD_RELOC_32_BASEREL",
  "BFD_RELOC_16_BASEREL",
  "BFD_RELOC_LO16_BASEREL",
  "BFD_RELOC_HI16_BASEREL",
  "BFD_RELOC_HI16_S_BASEREL",
  "BFD_RELOC_8_BASEREL",
  "BFD_RELOC_RVA",
  "BFD_RELOC_8_FFnn",
  "BFD_RELOC_32_PCREL_S2",
  "BFD_RELOC_16_PCREL_S2",
  "BFD_RELOC_23_PCREL_S2",
  "BFD_RELOC_HI22",
  "BFD_RELOC_LO10",
  "BFD_RELOC_GPREL16",
  "BFD_RELOC_GPREL32",
  "BFD_RELOC_NONE",
  "BFD_RELOC_SPARC_WDISP22",
  "BFD_RELOC_SPARC22",
  "BFD_RELOC_SPARC13",
  "BFD_RELOC_SPARC_GOT10",
  "BFD_RELOC_SPARC_GOT13",
  "BFD_RELOC_SPARC_GOT22",
  "BFD_RELOC_SPARC_PC10",
  "BFD_RELOC_SPARC_PC22",
  "BFD_RELOC_SPARC_WPLT30",
  "BFD_RELOC_SPARC_COPY",
  "BFD_RELOC_SPARC_GLOB_DAT",
  "BFD_RELOC_SPARC_JMP_SLOT",
  "BFD_RELOC_SPARC_RELATIVE",
  "BFD_RELOC_SPARC_UA16",
  "BFD_RELOC_SPARC_UA32",
  "BFD_RELOC_SPARC_UA64",
  "BFD_RELOC_SPARC_GOTDATA_HIX22",
  "BFD_RELOC_SPARC_GOTDATA_LOX10",
  "BFD_RELOC_SPARC_GOTDATA_OP_HIX22",
  "BFD_RELOC_SPARC_GOTDATA_OP_LOX10",
  "BFD_RELOC_SPARC_GOTDATA_OP",
  "BFD_RELOC_SPARC_JMP_IREL",
  "BFD_RELOC_SPARC_IRELATIVE",
  "BFD_RELOC_SPARC_BASE13",
  "BFD_RELOC_SPARC_BASE22",
  "BFD_RELOC_SPARC_10",
  "BFD_RELOC_SPARC_11",
  "BFD_RELOC_SPARC_OLO10",
  "BFD_RELOC_SPARC_HH22",
  "BFD_RELOC_SPARC_HM10",
  "BFD_RELOC_SPARC_LM22",
  "BFD_RELOC_SPARC_PC_HH22",
  "BFD_RELOC_SPARC_PC_HM10",
  "BFD_RELOC_SPARC_PC_LM22",
  "BFD_RELOC_SPARC_WDISP16",
  "BFD_RELOC_SPARC_WDISP19",
  "BFD_RELOC_SPARC_7",
  "BFD_RELOC_SPARC_6",
  "BFD_RELOC_SPARC_5",
  "BFD_RELOC_SPARC_PLT32",
  "BFD_RELOC_SPARC_PLT64",
  "BFD_RELOC_SPARC_HIX22",
  "BFD_RELOC_SPARC_LOX10",
  "BFD_RELOC_SPARC_H44",
  "BFD_RELOC_SPARC_M44",
  "BFD_RELOC_SPARC_L44",
  "BFD_RELOC_SPARC_REGISTER",
  "BFD_RELOC_SPARC_H34",
  "BFD_RELOC_SPARC_SIZE32",
  "BFD_RELOC_SPARC_SIZE64",
  "BFD_RELOC_SPARC_WDISP10",
  "BFD_RELOC_SPARC_REV32",
  "BFD_RELOC_SPARC_TLS_GD_HI22",
  "BFD_RELOC_SPARC_TLS_GD_LO10",
  "BFD_RELOC_SPARC_TLS_GD_ADD",
  "BFD_RELOC_SPARC_TLS_GD_CALL",
  "BFD_RELOC_SPARC_TLS_LDM_HI22",
  "BFD_RELOC_SPARC_TLS_LDM_LO10",
  "BFD_RELOC_SPARC_TLS_LDM_ADD",
  "BFD_RELOC_SPARC_TLS_LDM_CALL",
  "BFD_RELOC_SPARC_TLS_LDO_HIX22",
  "BFD_RELOC_SPARC_TLS_LDO_LOX10",
  "BFD_RELOC_SPARC_TLS_LDO_ADD",
  "BFD_RELOC_SPARC_TLS_IE_HI22",
  "BFD_RELOC_SPARC_TLS_IE_LO10",
  "BFD_RELOC_SPARC_TLS_IE_LD",
  "BFD_RELOC_SPARC_TLS_IE_LDX",
  "BFD_RELOC_SPARC_TLS_IE_ADD",
  "BFD_RELOC_SPARC_TLS_LE_HIX22",
  "BFD_RELOC_SPARC_TLS_LE_LOX10",
  "BFD_RELOC_SPARC_TLS_DTPMOD32",
  "BFD_RELOC_SPARC_TLS_DTPMOD64",
  "BFD_RELOC_SPARC_TLS_DTPOFF32",
  "BFD_RELOC_SPARC_TLS_DTPOFF64",
  "BFD_RELOC_SPARC_TLS_TPOFF32",
  "BFD_RELOC_SPARC_TLS_TPOFF64",
  "BFD_RELOC_SPU_IMM7",
  "BFD_RELOC_SPU_IMM8",
  "BFD_RELOC_SPU_IMM10",
  "BFD_RELOC_SPU_IMM10W",
  "BFD_RELOC_SPU_IMM16",
  "BFD_RELOC_SPU_IMM16W",
  "BFD_RELOC_SPU_IMM18",
  "BFD_RELOC_SPU_PCREL9a",
  "BFD_RELOC_SPU_PCREL9b",
  "BFD_RELOC_SPU_PCREL16",
  "BFD_RELOC_SPU_LO16",
  "BFD_RELOC_SPU_HI16",
  "BFD_RELOC_SPU_PPU32",
  "BFD_RELOC_SPU_PPU64",
  "BFD_RELOC_SPU_ADD_PIC",
  "BFD_RELOC_ALPHA_GPDISP_HI16",
  "BFD_RELOC_ALPHA_GPDISP_LO16",
  "BFD_RELOC_ALPHA_GPDISP",
  "BFD_RELOC_ALPHA_LITERAL",
  "BFD_RELOC_ALPHA_ELF_LITERAL",
  "BFD_RELOC_ALPHA_LITUSE",
  "BFD_RELOC_ALPHA_HINT",
  "BFD_RELOC_ALPHA_LINKAGE",
  "BFD_RELOC_ALPHA_CODEADDR",
  "BFD_RELOC_ALPHA_GPREL_HI16",
  "BFD_RELOC_ALPHA_GPREL_LO16",
  "BFD_RELOC_ALPHA_BRSGP",
  "BFD_RELOC_ALPHA_NOP",
  "BFD_RELOC_ALPHA_BSR",
  "BFD_RELOC_ALPHA_LDA",
  "BFD_RELOC_ALPHA_BOH",
  "BFD_RELOC_ALPHA_TLSGD",
  "BFD_RELOC_ALPHA_TLSLDM",
  "BFD_RELOC_ALPHA_DTPMOD64",
  "BFD_RELOC_ALPHA_GOTDTPREL16",
  "BFD_RELOC_ALPHA_DTPREL64",
  "BFD_RELOC_ALPHA_DTPREL_HI16",
  "BFD_RELOC_ALPHA_DTPREL_LO16",
  "BFD_RELOC_ALPHA_DTPREL16",
  "BFD_RELOC_ALPHA_GOTTPREL16",
  "BFD_RELOC_ALPHA_TPREL64",
  "BFD_RELOC_ALPHA_TPREL_HI16",
  "BFD_RELOC_ALPHA_TPREL_LO16",
  "BFD_RELOC_ALPHA_TPREL16",
  "BFD_RELOC_MIPS_JMP",
  "BFD_RELOC_MICROMIPS_JMP",
  "BFD_RELOC_MIPS16_JMP",
  "BFD_RELOC_MIPS16_GPREL",
  "BFD_RELOC_HI16",
  "BFD_RELOC_HI16_S",
  "BFD_RELOC_LO16",
  "BFD_RELOC_HI16_PCREL",
  "BFD_RELOC_HI16_S_PCREL",
  "BFD_RELOC_LO16_PCREL",
  "BFD_RELOC_MIPS16_GOT16",
  "BFD_RELOC_MIPS16_CALL16",
  "BFD_RELOC_MIPS16_HI16",
  "BFD_RELOC_MIPS16_HI16_S",
  "BFD_RELOC_MIPS16_LO16",
  "BFD_RELOC_MIPS16_TLS_GD",
  "BFD_RELOC_MIPS16_TLS_LDM",
  "BFD_RELOC_MIPS16_TLS_DTPREL_HI16",
  "BFD_RELOC_MIPS16_TLS_DTPREL_LO16",
  "BFD_RELOC_MIPS16_TLS_GOTTPREL",
  "BFD_RELOC_MIPS16_TLS_TPREL_HI16",
  "BFD_RELOC_MIPS16_TLS_TPREL_LO16",
  "BFD_RELOC_MIPS_LITERAL",
  "BFD_RELOC_MICROMIPS_LITERAL",
  "BFD_RELOC_MICROMIPS_7_PCREL_S1",
  "BFD_RELOC_MICROMIPS_10_PCREL_S1",
  "BFD_RELOC_MICROMIPS_16_PCREL_S1",
  "BFD_RELOC_MIPS16_16_PCREL_S1",
  "BFD_RELOC_MIPS_21_PCREL_S2",
  "BFD_RELOC_MIPS_26_PCREL_S2",
  "BFD_RELOC_MIPS_18_PCREL_S3",
  "BFD_RELOC_MIPS_19_PCREL_S2",
  "BFD_RELOC_MICROMIPS_GPREL16",
  "BFD_RELOC_MICROMIPS_HI16",
  "BFD_RELOC_MICROMIPS_HI16_S",
  "BFD_RELOC_MICROMIPS_LO16",
  "BFD_RELOC_MIPS_GOT16",
  "BFD_RELOC_MICROMIPS_GOT16",
  "BFD_RELOC_MIPS_CALL16",
  "BFD_RELOC_MICROMIPS_CALL16",
  "BFD_RELOC_MIPS_GOT_HI16",
  "BFD_RELOC_MICROMIPS_GOT_HI16",
  "BFD_RELOC_MIPS_GOT_LO16",
  "BFD_RELOC_MICROMIPS_GOT_LO16",
  "BFD_RELOC_MIPS_CALL_HI16",
  "BFD_RELOC_MICROMIPS_CALL_HI16",
  "BFD_RELOC_MIPS_CALL_LO16",
  "BFD_RELOC_MICROMIPS_CALL_LO16",
  "BFD_RELOC_MIPS_SUB",
  "BFD_RELOC_MICROMIPS_SUB",
  "BFD_RELOC_MIPS_GOT_PAGE",
  "BFD_RELOC_MICROMIPS_GOT_PAGE",
  "BFD_RELOC_MIPS_GOT_OFST",
  "BFD_RELOC_MICROMIPS_GOT_OFST",
  "BFD_RELOC_MIPS_GOT_DISP",
  "BFD_RELOC_MICROMIPS_GOT_DISP",
  "BFD_RELOC_MIPS_SHIFT5",
  "BFD_RELOC_MIPS_SHIFT6",
  "BFD_RELOC_MIPS_INSERT_A",
  "BFD_RELOC_MIPS_INSERT_B",
  "BFD_RELOC_MIPS_DELETE",
  "BFD_RELOC_MIPS_HIGHEST",
  "BFD_RELOC_MICROMIPS_HIGHEST",
  "BFD_RELOC_MIPS_HIGHER",
  "BFD_RELOC_MICROMIPS_HIGHER",
  "BFD_RELOC_MIPS_SCN_DISP",
  "BFD_RELOC_MICROMIPS_SCN_DISP",
  "BFD_RELOC_MIPS_16",
  "BFD_RELOC_MIPS_RELGOT",
  "BFD_RELOC_MIPS_JALR",
  "BFD_RELOC_MICROMIPS_JALR",
  "BFD_RELOC_MIPS_TLS_DTPMOD32",
  "BFD_RELOC_MIPS_TLS_DTPREL32",
  "BFD_RELOC_MIPS_TLS_DTPMOD64",
  "BFD_RELOC_MIPS_TLS_DTPREL64",
  "BFD_RELOC_MIPS_TLS_GD",
  "BFD_RELOC_MICROMIPS_TLS_GD",
  "BFD_RELOC_MIPS_TLS_LDM",
  "BFD_RELOC_MICROMIPS_TLS_LDM",
  "BFD_RELOC_MIPS_TLS_DTPREL_HI16",
  "BFD_RELOC_MICROMIPS_TLS_DTPREL_HI16",
  "BFD_RELOC_MIPS_TLS_DTPREL_LO16",
  "BFD_RELOC_MICROMIPS_TLS_DTPREL_LO16",
  "BFD_RELOC_MIPS_TLS_GOTTPREL",
  "BFD_RELOC_MICROMIPS_TLS_GOTTPREL",
  "BFD_RELOC_MIPS_TLS_TPREL32",
  "BFD_RELOC_MIPS_TLS_TPREL64",
  "BFD_RELOC_MIPS_TLS_TPREL_HI16",
  "BFD_RELOC_MICROMIPS_TLS_TPREL_HI16",
  "BFD_RELOC_MIPS_TLS_TPREL_LO16",
  "BFD_RELOC_MICROMIPS_TLS_TPREL_LO16",
  "BFD_RELOC_MIPS_EH",

  "BFD_RELOC_MIPS_COPY",
  "BFD_RELOC_MIPS_JUMP_SLOT",

  "BFD_RELOC_MOXIE_10_PCREL",

  "BFD_RELOC_FT32_10",
  "BFD_RELOC_FT32_20",
  "BFD_RELOC_FT32_17",
  "BFD_RELOC_FT32_18",
  "BFD_RELOC_FT32_RELAX",
  "BFD_RELOC_FT32_SC0",
  "BFD_RELOC_FT32_SC1",
  "BFD_RELOC_FT32_15",
  "BFD_RELOC_FT32_DIFF32",

  "BFD_RELOC_FRV_LABEL16",
  "BFD_RELOC_FRV_LABEL24",
  "BFD_RELOC_FRV_LO16",
  "BFD_RELOC_FRV_HI16",
  "BFD_RELOC_FRV_GPREL12",
  "BFD_RELOC_FRV_GPRELU12",
  "BFD_RELOC_FRV_GPREL32",
  "BFD_RELOC_FRV_GPRELHI",
  "BFD_RELOC_FRV_GPRELLO",
  "BFD_RELOC_FRV_GOT12",
  "BFD_RELOC_FRV_GOTHI",
  "BFD_RELOC_FRV_GOTLO",
  "BFD_RELOC_FRV_FUNCDESC",
  "BFD_RELOC_FRV_FUNCDESC_GOT12",
  "BFD_RELOC_FRV_FUNCDESC_GOTHI",
  "BFD_RELOC_FRV_FUNCDESC_GOTLO",
  "BFD_RELOC_FRV_FUNCDESC_VALUE",
  "BFD_RELOC_FRV_FUNCDESC_GOTOFF12",
  "BFD_RELOC_FRV_FUNCDESC_GOTOFFHI",
  "BFD_RELOC_FRV_FUNCDESC_GOTOFFLO",
  "BFD_RELOC_FRV_GOTOFF12",
  "BFD_RELOC_FRV_GOTOFFHI",
  "BFD_RELOC_FRV_GOTOFFLO",
  "BFD_RELOC_FRV_GETTLSOFF",
  "BFD_RELOC_FRV_TLSDESC_VALUE",
  "BFD_RELOC_FRV_GOTTLSDESC12",
  "BFD_RELOC_FRV_GOTTLSDESCHI",
  "BFD_RELOC_FRV_GOTTLSDESCLO",
  "BFD_RELOC_FRV_TLSMOFF12",
  "BFD_RELOC_FRV_TLSMOFFHI",
  "BFD_RELOC_FRV_TLSMOFFLO",
  "BFD_RELOC_FRV_GOTTLSOFF12",
  "BFD_RELOC_FRV_GOTTLSOFFHI",
  "BFD_RELOC_FRV_GOTTLSOFFLO",
  "BFD_RELOC_FRV_TLSOFF",
  "BFD_RELOC_FRV_TLSDESC_RELAX",
  "BFD_RELOC_FRV_GETTLSOFF_RELAX",
  "BFD_RELOC_FRV_TLSOFF_RELAX",
  "BFD_RELOC_FRV_TLSMOFF",

  "BFD_RELOC_MN10300_GOTOFF24",
  "BFD_RELOC_MN10300_GOT32",
  "BFD_RELOC_MN10300_GOT24",
  "BFD_RELOC_MN10300_GOT16",
  "BFD_RELOC_MN10300_COPY",
  "BFD_RELOC_MN10300_GLOB_DAT",
  "BFD_RELOC_MN10300_JMP_SLOT",
  "BFD_RELOC_MN10300_RELATIVE",
  "BFD_RELOC_MN10300_SYM_DIFF",
  "BFD_RELOC_MN10300_ALIGN",
  "BFD_RELOC_MN10300_TLS_GD",
  "BFD_RELOC_MN10300_TLS_LD",
  "BFD_RELOC_MN10300_TLS_LDO",
  "BFD_RELOC_MN10300_TLS_GOTIE",
  "BFD_RELOC_MN10300_TLS_IE",
  "BFD_RELOC_MN10300_TLS_LE",
  "BFD_RELOC_MN10300_TLS_DTPMOD",
  "BFD_RELOC_MN10300_TLS_DTPOFF",
  "BFD_RELOC_MN10300_TLS_TPOFF",
  "BFD_RELOC_MN10300_32_PCREL",
  "BFD_RELOC_MN10300_16_PCREL",

  "BFD_RELOC_386_GOT32",
  "BFD_RELOC_386_PLT32",
  "BFD_RELOC_386_COPY",
  "BFD_RELOC_386_GLOB_DAT",
  "BFD_RELOC_386_JUMP_SLOT",
  "BFD_RELOC_386_RELATIVE",
  "BFD_RELOC_386_GOTOFF",
  "BFD_RELOC_386_GOTPC",
  "BFD_RELOC_386_TLS_TPOFF",
  "BFD_RELOC_386_TLS_IE",
  "BFD_RELOC_386_TLS_GOTIE",
  "BFD_RELOC_386_TLS_LE",
  "BFD_RELOC_386_TLS_GD",
  "BFD_RELOC_386_TLS_LDM",
  "BFD_RELOC_386_TLS_LDO_32",
  "BFD_RELOC_386_TLS_IE_32",
  "BFD_RELOC_386_TLS_LE_32",
  "BFD_RELOC_386_TLS_DTPMOD32",
  "BFD_RELOC_386_TLS_DTPOFF32",
  "BFD_RELOC_386_TLS_TPOFF32",
  "BFD_RELOC_386_TLS_GOTDESC",
  "BFD_RELOC_386_TLS_DESC_CALL",
  "BFD_RELOC_386_TLS_DESC",
  "BFD_RELOC_386_IRELATIVE",
  "BFD_RELOC_386_GOT32X",
  "BFD_RELOC_X86_64_GOT32",
  "BFD_RELOC_X86_64_PLT32",
  "BFD_RELOC_X86_64_COPY",
  "BFD_RELOC_X86_64_GLOB_DAT",
  "BFD_RELOC_X86_64_JUMP_SLOT",
  "BFD_RELOC_X86_64_RELATIVE",
  "BFD_RELOC_X86_64_GOTPCREL",
  "BFD_RELOC_X86_64_32S",
  "BFD_RELOC_X86_64_DTPMOD64",
  "BFD_RELOC_X86_64_DTPOFF64",
  "BFD_RELOC_X86_64_TPOFF64",
  "BFD_RELOC_X86_64_TLSGD",
  "BFD_RELOC_X86_64_TLSLD",
  "BFD_RELOC_X86_64_DTPOFF32",
  "BFD_RELOC_X86_64_GOTTPOFF",
  "BFD_RELOC_X86_64_TPOFF32",
  "BFD_RELOC_X86_64_GOTOFF64",
  "BFD_RELOC_X86_64_GOTPC32",
  "BFD_RELOC_X86_64_GOT64",
  "BFD_RELOC_X86_64_GOTPCREL64",
  "BFD_RELOC_X86_64_GOTPC64",
  "BFD_RELOC_X86_64_GOTPLT64",
  "BFD_RELOC_X86_64_PLTOFF64",
  "BFD_RELOC_X86_64_GOTPC32_TLSDESC",
  "BFD_RELOC_X86_64_TLSDESC_CALL",
  "BFD_RELOC_X86_64_TLSDESC",
  "BFD_RELOC_X86_64_IRELATIVE",
  "BFD_RELOC_X86_64_PC32_BND",
  "BFD_RELOC_X86_64_PLT32_BND",
  "BFD_RELOC_X86_64_GOTPCRELX",
  "BFD_RELOC_X86_64_REX_GOTPCRELX",
  "BFD_RELOC_NS32K_IMM_8",
  "BFD_RELOC_NS32K_IMM_16",
  "BFD_RELOC_NS32K_IMM_32",
  "BFD_RELOC_NS32K_IMM_8_PCREL",
  "BFD_RELOC_NS32K_IMM_16_PCREL",
  "BFD_RELOC_NS32K_IMM_32_PCREL",
  "BFD_RELOC_NS32K_DISP_8",
  "BFD_RELOC_NS32K_DISP_16",
  "BFD_RELOC_NS32K_DISP_32",
  "BFD_RELOC_NS32K_DISP_8_PCREL",
  "BFD_RELOC_NS32K_DISP_16_PCREL",
  "BFD_RELOC_NS32K_DISP_32_PCREL",
  "BFD_RELOC_PDP11_DISP_8_PCREL",
  "BFD_RELOC_PDP11_DISP_6_PCREL",
  "BFD_RELOC_PJ_CODE_HI16",
  "BFD_RELOC_PJ_CODE_LO16",
  "BFD_RELOC_PJ_CODE_DIR16",
  "BFD_RELOC_PJ_CODE_DIR32",
  "BFD_RELOC_PJ_CODE_REL16",
  "BFD_RELOC_PJ_CODE_REL32",
  "BFD_RELOC_PPC_B26",
  "BFD_RELOC_PPC_BA26",
  "BFD_RELOC_PPC_TOC16",
  "BFD_RELOC_PPC_TOC16_LO",
  "BFD_RELOC_PPC_TOC16_HI",
  "BFD_RELOC_PPC_B16",
  "BFD_RELOC_PPC_B16_BRTAKEN",
  "BFD_RELOC_PPC_B16_BRNTAKEN",
  "BFD_RELOC_PPC_BA16",
  "BFD_RELOC_PPC_BA16_BRTAKEN",
  "BFD_RELOC_PPC_BA16_BRNTAKEN",
  "BFD_RELOC_PPC_COPY",
  "BFD_RELOC_PPC_GLOB_DAT",
  "BFD_RELOC_PPC_JMP_SLOT",
  "BFD_RELOC_PPC_RELATIVE",
  "BFD_RELOC_PPC_LOCAL24PC",
  "BFD_RELOC_PPC_EMB_NADDR32",
  "BFD_RELOC_PPC_EMB_NADDR16",
  "BFD_RELOC_PPC_EMB_NADDR16_LO",
  "BFD_RELOC_PPC_EMB_NADDR16_HI",
  "BFD_RELOC_PPC_EMB_NADDR16_HA",
  "BFD_RELOC_PPC_EMB_SDAI16",
  "BFD_RELOC_PPC_EMB_SDA2I16",
  "BFD_RELOC_PPC_EMB_SDA2REL",
  "BFD_RELOC_PPC_EMB_SDA21",
  "BFD_RELOC_PPC_EMB_MRKREF",
  "BFD_RELOC_PPC_EMB_RELSEC16",
  "BFD_RELOC_PPC_EMB_RELST_LO",
  "BFD_RELOC_PPC_EMB_RELST_HI",
  "BFD_RELOC_PPC_EMB_RELST_HA",
  "BFD_RELOC_PPC_EMB_BIT_FLD",
  "BFD_RELOC_PPC_EMB_RELSDA",
  "BFD_RELOC_PPC_VLE_REL8",
  "BFD_RELOC_PPC_VLE_REL15",
  "BFD_RELOC_PPC_VLE_REL24",
  "BFD_RELOC_PPC_VLE_LO16A",
  "BFD_RELOC_PPC_VLE_LO16D",
  "BFD_RELOC_PPC_VLE_HI16A",
  "BFD_RELOC_PPC_VLE_HI16D",
  "BFD_RELOC_PPC_VLE_HA16A",
  "BFD_RELOC_PPC_VLE_HA16D",
  "BFD_RELOC_PPC_VLE_SDA21",
  "BFD_RELOC_PPC_VLE_SDA21_LO",
  "BFD_RELOC_PPC_VLE_SDAREL_LO16A",
  "BFD_RELOC_PPC_VLE_SDAREL_LO16D",
  "BFD_RELOC_PPC_VLE_SDAREL_HI16A",
  "BFD_RELOC_PPC_VLE_SDAREL_HI16D",
  "BFD_RELOC_PPC_VLE_SDAREL_HA16A",
  "BFD_RELOC_PPC_VLE_SDAREL_HA16D",
  "BFD_RELOC_PPC_16DX_HA",
  "BFD_RELOC_PPC_REL16DX_HA",
  "BFD_RELOC_PPC_NEG",
  "BFD_RELOC_PPC64_HIGHER",
  "BFD_RELOC_PPC64_HIGHER_S",
  "BFD_RELOC_PPC64_HIGHEST",
  "BFD_RELOC_PPC64_HIGHEST_S",
  "BFD_RELOC_PPC64_TOC16_LO",
  "BFD_RELOC_PPC64_TOC16_HI",
  "BFD_RELOC_PPC64_TOC16_HA",
  "BFD_RELOC_PPC64_TOC",
  "BFD_RELOC_PPC64_PLTGOT16",
  "BFD_RELOC_PPC64_PLTGOT16_LO",
  "BFD_RELOC_PPC64_PLTGOT16_HI",
  "BFD_RELOC_PPC64_PLTGOT16_HA",
  "BFD_RELOC_PPC64_ADDR16_DS",
  "BFD_RELOC_PPC64_ADDR16_LO_DS",
  "BFD_RELOC_PPC64_GOT16_DS",
  "BFD_RELOC_PPC64_GOT16_LO_DS",
  "BFD_RELOC_PPC64_PLT16_LO_DS",
  "BFD_RELOC_PPC64_SECTOFF_DS",
  "BFD_RELOC_PPC64_SECTOFF_LO_DS",
  "BFD_RELOC_PPC64_TOC16_DS",
  "BFD_RELOC_PPC64_TOC16_LO_DS",
  "BFD_RELOC_PPC64_PLTGOT16_DS",
  "BFD_RELOC_PPC64_PLTGOT16_LO_DS",
  "BFD_RELOC_PPC64_ADDR16_HIGH",
  "BFD_RELOC_PPC64_ADDR16_HIGHA",
  "BFD_RELOC_PPC64_REL16_HIGH",
  "BFD_RELOC_PPC64_REL16_HIGHA",
  "BFD_RELOC_PPC64_REL16_HIGHER",
  "BFD_RELOC_PPC64_REL16_HIGHERA",
  "BFD_RELOC_PPC64_REL16_HIGHEST",
  "BFD_RELOC_PPC64_REL16_HIGHESTA",
  "BFD_RELOC_PPC64_ADDR64_LOCAL",
  "BFD_RELOC_PPC64_ENTRY",
  "BFD_RELOC_PPC64_REL24_NOTOC",
  "BFD_RELOC_PPC64_REL24_P9NOTOC",
  "BFD_RELOC_PPC64_D34",
  "BFD_RELOC_PPC64_D34_LO",
  "BFD_RELOC_PPC64_D34_HI30",
  "BFD_RELOC_PPC64_D34_HA30",
  "BFD_RELOC_PPC64_PCREL34",
  "BFD_RELOC_PPC64_GOT_PCREL34",
  "BFD_RELOC_PPC64_PLT_PCREL34",
  "BFD_RELOC_PPC64_ADDR16_HIGHER34",
  "BFD_RELOC_PPC64_ADDR16_HIGHERA34",
  "BFD_RELOC_PPC64_ADDR16_HIGHEST34",
  "BFD_RELOC_PPC64_ADDR16_HIGHESTA34",
  "BFD_RELOC_PPC64_REL16_HIGHER34",
  "BFD_RELOC_PPC64_REL16_HIGHERA34",
  "BFD_RELOC_PPC64_REL16_HIGHEST34",
  "BFD_RELOC_PPC64_REL16_HIGHESTA34",
  "BFD_RELOC_PPC64_D28",
  "BFD_RELOC_PPC64_PCREL28",
  "BFD_RELOC_PPC_TLS",
  "BFD_RELOC_PPC_TLSGD",
  "BFD_RELOC_PPC_TLSLD",
  "BFD_RELOC_PPC_TLSLE",
  "BFD_RELOC_PPC_TLSIE",
  "BFD_RELOC_PPC_TLSM",
  "BFD_RELOC_PPC_TLSML",
  "BFD_RELOC_PPC_DTPMOD",
  "BFD_RELOC_PPC_TPREL16",
  "BFD_RELOC_PPC_TPREL16_LO",
  "BFD_RELOC_PPC_TPREL16_HI",
  "BFD_RELOC_PPC_TPREL16_HA",
  "BFD_RELOC_PPC_TPREL",
  "BFD_RELOC_PPC_DTPREL16",
  "BFD_RELOC_PPC_DTPREL16_LO",
  "BFD_RELOC_PPC_DTPREL16_HI",
  "BFD_RELOC_PPC_DTPREL16_HA",
  "BFD_RELOC_PPC_DTPREL",
  "BFD_RELOC_PPC_GOT_TLSGD16",
  "BFD_RELOC_PPC_GOT_TLSGD16_LO",
  "BFD_RELOC_PPC_GOT_TLSGD16_HI",
  "BFD_RELOC_PPC_GOT_TLSGD16_HA",
  "BFD_RELOC_PPC_GOT_TLSLD16",
  "BFD_RELOC_PPC_GOT_TLSLD16_LO",
  "BFD_RELOC_PPC_GOT_TLSLD16_HI",
  "BFD_RELOC_PPC_GOT_TLSLD16_HA",
  "BFD_RELOC_PPC_GOT_TPREL16",
  "BFD_RELOC_PPC_GOT_TPREL16_LO",
  "BFD_RELOC_PPC_GOT_TPREL16_HI",
  "BFD_RELOC_PPC_GOT_TPREL16_HA",
  "BFD_RELOC_PPC_GOT_DTPREL16",
  "BFD_RELOC_PPC_GOT_DTPREL16_LO",
  "BFD_RELOC_PPC_GOT_DTPREL16_HI",
  "BFD_RELOC_PPC_GOT_DTPREL16_HA",
  "BFD_RELOC_PPC64_TLSGD",
  "BFD_RELOC_PPC64_TLSLD",
  "BFD_RELOC_PPC64_TLSLE",
  "BFD_RELOC_PPC64_TLSIE",
  "BFD_RELOC_PPC64_TLSM",
  "BFD_RELOC_PPC64_TLSML",
  "BFD_RELOC_PPC64_TPREL16_DS",
  "BFD_RELOC_PPC64_TPREL16_LO_DS",
  "BFD_RELOC_PPC64_TPREL16_HIGH",
  "BFD_RELOC_PPC64_TPREL16_HIGHA",
  "BFD_RELOC_PPC64_TPREL16_HIGHER",
  "BFD_RELOC_PPC64_TPREL16_HIGHERA",
  "BFD_RELOC_PPC64_TPREL16_HIGHEST",
  "BFD_RELOC_PPC64_TPREL16_HIGHESTA",
  "BFD_RELOC_PPC64_DTPREL16_DS",
  "BFD_RELOC_PPC64_DTPREL16_LO_DS",
  "BFD_RELOC_PPC64_DTPREL16_HIGH",
  "BFD_RELOC_PPC64_DTPREL16_HIGHA",
  "BFD_RELOC_PPC64_DTPREL16_HIGHER",
  "BFD_RELOC_PPC64_DTPREL16_HIGHERA",
  "BFD_RELOC_PPC64_DTPREL16_HIGHEST",
  "BFD_RELOC_PPC64_DTPREL16_HIGHESTA",
  "BFD_RELOC_PPC64_TPREL34",
  "BFD_RELOC_PPC64_DTPREL34",
  "BFD_RELOC_PPC64_GOT_TLSGD_PCREL34",
  "BFD_RELOC_PPC64_GOT_TLSLD_PCREL34",
  "BFD_RELOC_PPC64_GOT_TPREL_PCREL34",
  "BFD_RELOC_PPC64_GOT_DTPREL_PCREL34",
  "BFD_RELOC_PPC64_TLS_PCREL",
  "BFD_RELOC_I370_D12",
  "BFD_RELOC_CTOR",
  "BFD_RELOC_ARM_PCREL_BRANCH",
  "BFD_RELOC_ARM_PCREL_BLX",
  "BFD_RELOC_THUMB_PCREL_BLX",
  "BFD_RELOC_ARM_PCREL_CALL",
  "BFD_RELOC_ARM_PCREL_JUMP",
  "BFD_RELOC_THUMB_PCREL_BRANCH5",
  "BFD_RELOC_THUMB_PCREL_BFCSEL",
  "BFD_RELOC_ARM_THUMB_BF17",
  "BFD_RELOC_ARM_THUMB_BF13",
  "BFD_RELOC_ARM_THUMB_BF19",
  "BFD_RELOC_ARM_THUMB_LOOP12",
  "BFD_RELOC_THUMB_PCREL_BRANCH7",
  "BFD_RELOC_THUMB_PCREL_BRANCH9",
  "BFD_RELOC_THUMB_PCREL_BRANCH12",
  "BFD_RELOC_THUMB_PCREL_BRANCH20",
  "BFD_RELOC_THUMB_PCREL_BRANCH23",
  "BFD_RELOC_THUMB_PCREL_BRANCH25",
  "BFD_RELOC_ARM_OFFSET_IMM",
  "BFD_RELOC_ARM_THUMB_OFFSET",
  "BFD_RELOC_ARM_TARGET1",
  "BFD_RELOC_ARM_ROSEGREL32",
  "BFD_RELOC_ARM_SBREL32",
  "BFD_RELOC_ARM_TARGET2",
  "BFD_RELOC_ARM_PREL31",
  "BFD_RELOC_ARM_MOVW",
  "BFD_RELOC_ARM_MOVT",
  "BFD_RELOC_ARM_MOVW_PCREL",
  "BFD_RELOC_ARM_MOVT_PCREL",
  "BFD_RELOC_ARM_THUMB_MOVW",
  "BFD_RELOC_ARM_THUMB_MOVT",
  "BFD_RELOC_ARM_THUMB_MOVW_PCREL",
  "BFD_RELOC_ARM_THUMB_MOVT_PCREL",
  "BFD_RELOC_ARM_GOTFUNCDESC",
  "BFD_RELOC_ARM_GOTOFFFUNCDESC",
  "BFD_RELOC_ARM_FUNCDESC",
  "BFD_RELOC_ARM_FUNCDESC_VALUE",
  "BFD_RELOC_ARM_TLS_GD32_FDPIC",
  "BFD_RELOC_ARM_TLS_LDM32_FDPIC",
  "BFD_RELOC_ARM_TLS_IE32_FDPIC",
  "BFD_RELOC_ARM_JUMP_SLOT",
  "BFD_RELOC_ARM_GLOB_DAT",
  "BFD_RELOC_ARM_GOT32",
  "BFD_RELOC_ARM_PLT32",
  "BFD_RELOC_ARM_RELATIVE",
  "BFD_RELOC_ARM_GOTOFF",
  "BFD_RELOC_ARM_GOTPC",
  "BFD_RELOC_ARM_GOT_PREL",
  "BFD_RELOC_ARM_TLS_GD32",
  "BFD_RELOC_ARM_TLS_LDO32",
  "BFD_RELOC_ARM_TLS_LDM32",
  "BFD_RELOC_ARM_TLS_DTPOFF32",
  "BFD_RELOC_ARM_TLS_DTPMOD32",
  "BFD_RELOC_ARM_TLS_TPOFF32",
  "BFD_RELOC_ARM_TLS_IE32",
  "BFD_RELOC_ARM_TLS_LE32",
  "BFD_RELOC_ARM_TLS_GOTDESC",
  "BFD_RELOC_ARM_TLS_CALL",
  "BFD_RELOC_ARM_THM_TLS_CALL",
  "BFD_RELOC_ARM_TLS_DESCSEQ",
  "BFD_RELOC_ARM_THM_TLS_DESCSEQ",
  "BFD_RELOC_ARM_TLS_DESC",
  "BFD_RELOC_ARM_ALU_PC_G0_NC",
  "BFD_RELOC_ARM_ALU_PC_G0",
  "BFD_RELOC_ARM_ALU_PC_G1_NC",
  "BFD_RELOC_ARM_ALU_PC_G1",
  "BFD_RELOC_ARM_ALU_PC_G2",
  "BFD_RELOC_ARM_LDR_PC_G0",
  "BFD_RELOC_ARM_LDR_PC_G1",
  "BFD_RELOC_ARM_LDR_PC_G2",
  "BFD_RELOC_ARM_LDRS_PC_G0",
  "BFD_RELOC_ARM_LDRS_PC_G1",
  "BFD_RELOC_ARM_LDRS_PC_G2",
  "BFD_RELOC_ARM_LDC_PC_G0",
  "BFD_RELOC_ARM_LDC_PC_G1",
  "BFD_RELOC_ARM_LDC_PC_G2",
  "BFD_RELOC_ARM_ALU_SB_G0_NC",
  "BFD_RELOC_ARM_ALU_SB_G0",
  "BFD_RELOC_ARM_ALU_SB_G1_NC",
  "BFD_RELOC_ARM_ALU_SB_G1",
  "BFD_RELOC_ARM_ALU_SB_G2",
  "BFD_RELOC_ARM_LDR_SB_G0",
  "BFD_RELOC_ARM_LDR_SB_G1",
  "BFD_RELOC_ARM_LDR_SB_G2",
  "BFD_RELOC_ARM_LDRS_SB_G0",
  "BFD_RELOC_ARM_LDRS_SB_G1",
  "BFD_RELOC_ARM_LDRS_SB_G2",
  "BFD_RELOC_ARM_LDC_SB_G0",
  "BFD_RELOC_ARM_LDC_SB_G1",
  "BFD_RELOC_ARM_LDC_SB_G2",
  "BFD_RELOC_ARM_V4BX",
  "BFD_RELOC_ARM_IRELATIVE",
  "BFD_RELOC_ARM_THUMB_ALU_ABS_G0_NC",
  "BFD_RELOC_ARM_THUMB_ALU_ABS_G1_NC",
  "BFD_RELOC_ARM_THUMB_ALU_ABS_G2_NC",
  "BFD_RELOC_ARM_THUMB_ALU_ABS_G3_NC",
  "BFD_RELOC_ARM_IMMEDIATE",
  "BFD_RELOC_ARM_ADRL_IMMEDIATE",
  "BFD_RELOC_ARM_T32_IMMEDIATE",
  "BFD_RELOC_ARM_T32_ADD_IMM",
  "BFD_RELOC_ARM_T32_IMM12",
  "BFD_RELOC_ARM_T32_ADD_PC12",
  "BFD_RELOC_ARM_SHIFT_IMM",
  "BFD_RELOC_ARM_SMC",
  "BFD_RELOC_ARM_HVC",
  "BFD_RELOC_ARM_SWI",
  "BFD_RELOC_ARM_MULTI",
  "BFD_RELOC_ARM_CP_OFF_IMM",
  "BFD_RELOC_ARM_CP_OFF_IMM_S2",
  "BFD_RELOC_ARM_T32_CP_OFF_IMM",
  "BFD_RELOC_ARM_T32_CP_OFF_IMM_S2",
  "BFD_RELOC_ARM_T32_VLDR_VSTR_OFF_IMM",
  "BFD_RELOC_ARM_ADR_IMM",
  "BFD_RELOC_ARM_LDR_IMM",
  "BFD_RELOC_ARM_LITERAL",
  "BFD_RELOC_ARM_IN_POOL",
  "BFD_RELOC_ARM_OFFSET_IMM8",
  "BFD_RELOC_ARM_T32_OFFSET_U8",
  "BFD_RELOC_ARM_T32_OFFSET_IMM",
  "BFD_RELOC_ARM_HWLITERAL",
  "BFD_RELOC_ARM_THUMB_ADD",
  "BFD_RELOC_ARM_THUMB_IMM",
  "BFD_RELOC_ARM_THUMB_SHIFT",
  "BFD_RELOC_SH_PCDISP8BY2",
  "BFD_RELOC_SH_PCDISP12BY2",
  "BFD_RELOC_SH_IMM3",
  "BFD_RELOC_SH_IMM3U",
  "BFD_RELOC_SH_DISP12",
  "BFD_RELOC_SH_DISP12BY2",
  "BFD_RELOC_SH_DISP12BY4",
  "BFD_RELOC_SH_DISP12BY8",
  "BFD_RELOC_SH_DISP20",
  "BFD_RELOC_SH_DISP20BY8",
  "BFD_RELOC_SH_IMM4",
  "BFD_RELOC_SH_IMM4BY2",
  "BFD_RELOC_SH_IMM4BY4",
  "BFD_RELOC_SH_IMM8",
  "BFD_RELOC_SH_IMM8BY2",
  "BFD_RELOC_SH_IMM8BY4",
  "BFD_RELOC_SH_PCRELIMM8BY2",
  "BFD_RELOC_SH_PCRELIMM8BY4",
  "BFD_RELOC_SH_SWITCH16",
  "BFD_RELOC_SH_SWITCH32",
  "BFD_RELOC_SH_USES",
  "BFD_RELOC_SH_COUNT",
  "BFD_RELOC_SH_ALIGN",
  "BFD_RELOC_SH_CODE",
  "BFD_RELOC_SH_DATA",
  "BFD_RELOC_SH_LABEL",
  "BFD_RELOC_SH_LOOP_START",
  "BFD_RELOC_SH_LOOP_END",
  "BFD_RELOC_SH_COPY",
  "BFD_RELOC_SH_GLOB_DAT",
  "BFD_RELOC_SH_JMP_SLOT",
  "BFD_RELOC_SH_RELATIVE",
  "BFD_RELOC_SH_GOTPC",
  "BFD_RELOC_SH_GOT_LOW16",
  "BFD_RELOC_SH_GOT_MEDLOW16",
  "BFD_RELOC_SH_GOT_MEDHI16",
  "BFD_RELOC_SH_GOT_HI16",
  "BFD_RELOC_SH_GOTPLT_LOW16",
  "BFD_RELOC_SH_GOTPLT_MEDLOW16",
  "BFD_RELOC_SH_GOTPLT_MEDHI16",
  "BFD_RELOC_SH_GOTPLT_HI16",
  "BFD_RELOC_SH_PLT_LOW16",
  "BFD_RELOC_SH_PLT_MEDLOW16",
  "BFD_RELOC_SH_PLT_MEDHI16",
  "BFD_RELOC_SH_PLT_HI16",
  "BFD_RELOC_SH_GOTOFF_LOW16",
  "BFD_RELOC_SH_GOTOFF_MEDLOW16",
  "BFD_RELOC_SH_GOTOFF_MEDHI16",
  "BFD_RELOC_SH_GOTOFF_HI16",
  "BFD_RELOC_SH_GOTPC_LOW16",
  "BFD_RELOC_SH_GOTPC_MEDLOW16",
  "BFD_RELOC_SH_GOTPC_MEDHI16",
  "BFD_RELOC_SH_GOTPC_HI16",
  "BFD_RELOC_SH_COPY64",
  "BFD_RELOC_SH_GLOB_DAT64",
  "BFD_RELOC_SH_JMP_SLOT64",
  "BFD_RELOC_SH_RELATIVE64",
  "BFD_RELOC_SH_GOT10BY4",
  "BFD_RELOC_SH_GOT10BY8",
  "BFD_RELOC_SH_GOTPLT10BY4",
  "BFD_RELOC_SH_GOTPLT10BY8",
  "BFD_RELOC_SH_GOTPLT32",
  "BFD_RELOC_SH_SHMEDIA_CODE",
  "BFD_RELOC_SH_IMMU5",
  "BFD_RELOC_SH_IMMS6",
  "BFD_RELOC_SH_IMMS6BY32",
  "BFD_RELOC_SH_IMMU6",
  "BFD_RELOC_SH_IMMS10",
  "BFD_RELOC_SH_IMMS10BY2",
  "BFD_RELOC_SH_IMMS10BY4",
  "BFD_RELOC_SH_IMMS10BY8",
  "BFD_RELOC_SH_IMMS16",
  "BFD_RELOC_SH_IMMU16",
  "BFD_RELOC_SH_IMM_LOW16",
  "BFD_RELOC_SH_IMM_LOW16_PCREL",
  "BFD_RELOC_SH_IMM_MEDLOW16",
  "BFD_RELOC_SH_IMM_MEDLOW16_PCREL",
  "BFD_RELOC_SH_IMM_MEDHI16",
  "BFD_RELOC_SH_IMM_MEDHI16_PCREL",
  "BFD_RELOC_SH_IMM_HI16",
  "BFD_RELOC_SH_IMM_HI16_PCREL",
  "BFD_RELOC_SH_PT_16",
  "BFD_RELOC_SH_TLS_GD_32",
  "BFD_RELOC_SH_TLS_LD_32",
  "BFD_RELOC_SH_TLS_LDO_32",
  "BFD_RELOC_SH_TLS_IE_32",
  "BFD_RELOC_SH_TLS_LE_32",
  "BFD_RELOC_SH_TLS_DTPMOD32",
  "BFD_RELOC_SH_TLS_DTPOFF32",
  "BFD_RELOC_SH_TLS_TPOFF32",
  "BFD_RELOC_SH_GOT20",
  "BFD_RELOC_SH_GOTOFF20",
  "BFD_RELOC_SH_GOTFUNCDESC",
  "BFD_RELOC_SH_GOTFUNCDESC20",
  "BFD_RELOC_SH_GOTOFFFUNCDESC",
  "BFD_RELOC_SH_GOTOFFFUNCDESC20",
  "BFD_RELOC_SH_FUNCDESC",
  "BFD_RELOC_ARC_NONE",
  "BFD_RELOC_ARC_8",
  "BFD_RELOC_ARC_16",
  "BFD_RELOC_ARC_24",
  "BFD_RELOC_ARC_32",
  "BFD_RELOC_ARC_N8",
  "BFD_RELOC_ARC_N16",
  "BFD_RELOC_ARC_N24",
  "BFD_RELOC_ARC_N32",
  "BFD_RELOC_ARC_SDA",
  "BFD_RELOC_ARC_SECTOFF",
  "BFD_RELOC_ARC_S21H_PCREL",
  "BFD_RELOC_ARC_S21W_PCREL",
  "BFD_RELOC_ARC_S25H_PCREL",
  "BFD_RELOC_ARC_S25W_PCREL",
  "BFD_RELOC_ARC_SDA32",
  "BFD_RELOC_ARC_SDA_LDST",
  "BFD_RELOC_ARC_SDA_LDST1",
  "BFD_RELOC_ARC_SDA_LDST2",
  "BFD_RELOC_ARC_SDA16_LD",
  "BFD_RELOC_ARC_SDA16_LD1",
  "BFD_RELOC_ARC_SDA16_LD2",
  "BFD_RELOC_ARC_S13_PCREL",
  "BFD_RELOC_ARC_W",
  "BFD_RELOC_ARC_32_ME",
  "BFD_RELOC_ARC_32_ME_S",
  "BFD_RELOC_ARC_N32_ME",
  "BFD_RELOC_ARC_SECTOFF_ME",
  "BFD_RELOC_ARC_SDA32_ME",
  "BFD_RELOC_ARC_W_ME",
  "BFD_RELOC_AC_SECTOFF_U8",
  "BFD_RELOC_AC_SECTOFF_U8_1",
  "BFD_RELOC_AC_SECTOFF_U8_2",
  "BFD_RELOC_AC_SECTOFF_S9",
  "BFD_RELOC_AC_SECTOFF_S9_1",
  "BFD_RELOC_AC_SECTOFF_S9_2",
  "BFD_RELOC_ARC_SECTOFF_ME_1",
  "BFD_RELOC_ARC_SECTOFF_ME_2",
  "BFD_RELOC_ARC_SECTOFF_1",
  "BFD_RELOC_ARC_SECTOFF_2",
  "BFD_RELOC_ARC_SDA_12",
  "BFD_RELOC_ARC_SDA16_ST2",
  "BFD_RELOC_ARC_32_PCREL",
  "BFD_RELOC_ARC_PC32",
  "BFD_RELOC_ARC_GOT32",
  "BFD_RELOC_ARC_GOTPC32",
  "BFD_RELOC_ARC_PLT32",
  "BFD_RELOC_ARC_COPY",
  "BFD_RELOC_ARC_GLOB_DAT",
  "BFD_RELOC_ARC_JMP_SLOT",
  "BFD_RELOC_ARC_RELATIVE",
  "BFD_RELOC_ARC_GOTOFF",
  "BFD_RELOC_ARC_GOTPC",
  "BFD_RELOC_ARC_S21W_PCREL_PLT",
  "BFD_RELOC_ARC_S25H_PCREL_PLT",
  "BFD_RELOC_ARC_TLS_DTPMOD",
  "BFD_RELOC_ARC_TLS_TPOFF",
  "BFD_RELOC_ARC_TLS_GD_GOT",
  "BFD_RELOC_ARC_TLS_GD_LD",
  "BFD_RELOC_ARC_TLS_GD_CALL",
  "BFD_RELOC_ARC_TLS_IE_GOT",
  "BFD_RELOC_ARC_TLS_DTPOFF",
  "BFD_RELOC_ARC_TLS_DTPOFF_S9",
  "BFD_RELOC_ARC_TLS_LE_S9",
  "BFD_RELOC_ARC_TLS_LE_32",
  "BFD_RELOC_ARC_S25W_PCREL_PLT",
  "BFD_RELOC_ARC_S21H_PCREL_PLT",
  "BFD_RELOC_ARC_NPS_CMEM16",
  "BFD_RELOC_ARC_JLI_SECTOFF",
  "BFD_RELOC_BFIN_16_IMM",
  "BFD_RELOC_BFIN_16_HIGH",
  "BFD_RELOC_BFIN_4_PCREL",
  "BFD_RELOC_BFIN_5_PCREL",
  "BFD_RELOC_BFIN_16_LOW",
  "BFD_RELOC_BFIN_10_PCREL",
  "BFD_RELOC_BFIN_11_PCREL",
  "BFD_RELOC_BFIN_12_PCREL_JUMP",
  "BFD_RELOC_BFIN_12_PCREL_JUMP_S",
  "BFD_RELOC_BFIN_24_PCREL_CALL_X",
  "BFD_RELOC_BFIN_24_PCREL_JUMP_L",
  "BFD_RELOC_BFIN_GOT17M4",
  "BFD_RELOC_BFIN_GOTHI",
  "BFD_RELOC_BFIN_GOTLO",
  "BFD_RELOC_BFIN_FUNCDESC",
  "BFD_RELOC_BFIN_FUNCDESC_GOT17M4",
  "BFD_RELOC_BFIN_FUNCDESC_GOTHI",
  "BFD_RELOC_BFIN_FUNCDESC_GOTLO",
  "BFD_RELOC_BFIN_FUNCDESC_VALUE",
  "BFD_RELOC_BFIN_FUNCDESC_GOTOFF17M4",
  "BFD_RELOC_BFIN_FUNCDESC_GOTOFFHI",
  "BFD_RELOC_BFIN_FUNCDESC_GOTOFFLO",
  "BFD_RELOC_BFIN_GOTOFF17M4",
  "BFD_RELOC_BFIN_GOTOFFHI",
  "BFD_RELOC_BFIN_GOTOFFLO",
  "BFD_RELOC_BFIN_GOT",
  "BFD_RELOC_BFIN_PLTPC",
  "BFD_ARELOC_BFIN_PUSH",
  "BFD_ARELOC_BFIN_CONST",
  "BFD_ARELOC_BFIN_ADD",
  "BFD_ARELOC_BFIN_SUB",
  "BFD_ARELOC_BFIN_MULT",
  "BFD_ARELOC_BFIN_DIV",
  "BFD_ARELOC_BFIN_MOD",
  "BFD_ARELOC_BFIN_LSHIFT",
  "BFD_ARELOC_BFIN_RSHIFT",
  "BFD_ARELOC_BFIN_AND",
  "BFD_ARELOC_BFIN_OR",
  "BFD_ARELOC_BFIN_XOR",
  "BFD_ARELOC_BFIN_LAND",
  "BFD_ARELOC_BFIN_LOR",
  "BFD_ARELOC_BFIN_LEN",
  "BFD_ARELOC_BFIN_NEG",
  "BFD_ARELOC_BFIN_COMP",
  "BFD_ARELOC_BFIN_PAGE",
  "BFD_ARELOC_BFIN_HWPAGE",
  "BFD_ARELOC_BFIN_ADDR",
  "BFD_RELOC_D10V_10_PCREL_R",
  "BFD_RELOC_D10V_10_PCREL_L",
  "BFD_RELOC_D10V_18",
  "BFD_RELOC_D10V_18_PCREL",
  "BFD_RELOC_D30V_6",
  "BFD_RELOC_D30V_9_PCREL",
  "BFD_RELOC_D30V_9_PCREL_R",
  "BFD_RELOC_D30V_15",
  "BFD_RELOC_D30V_15_PCREL",
  "BFD_RELOC_D30V_15_PCREL_R",
  "BFD_RELOC_D30V_21",
  "BFD_RELOC_D30V_21_PCREL",
  "BFD_RELOC_D30V_21_PCREL_R",
  "BFD_RELOC_D30V_32",
  "BFD_RELOC_D30V_32_PCREL",
  "BFD_RELOC_DLX_HI16_S",
  "BFD_RELOC_DLX_LO16",
  "BFD_RELOC_DLX_JMP26",
  "BFD_RELOC_M32C_HI8",
  "BFD_RELOC_M32C_RL_JUMP",
  "BFD_RELOC_M32C_RL_1ADDR",
  "BFD_RELOC_M32C_RL_2ADDR",
  "BFD_RELOC_M32R_24",
  "BFD_RELOC_M32R_10_PCREL",
  "BFD_RELOC_M32R_18_PCREL",
  "BFD_RELOC_M32R_26_PCREL",
  "BFD_RELOC_M32R_HI16_ULO",
  "BFD_RELOC_M32R_HI16_SLO",
  "BFD_RELOC_M32R_LO16",
  "BFD_RELOC_M32R_SDA16",
  "BFD_RELOC_M32R_GOT24",
  "BFD_RELOC_M32R_26_PLTREL",
  "BFD_RELOC_M32R_COPY",
  "BFD_RELOC_M32R_GLOB_DAT",
  "BFD_RELOC_M32R_JMP_SLOT",
  "BFD_RELOC_M32R_RELATIVE",
  "BFD_RELOC_M32R_GOTOFF",
  "BFD_RELOC_M32R_GOTOFF_HI_ULO",
  "BFD_RELOC_M32R_GOTOFF_HI_SLO",
  "BFD_RELOC_M32R_GOTOFF_LO",
  "BFD_RELOC_M32R_GOTPC24",
  "BFD_RELOC_M32R_GOT16_HI_ULO",
  "BFD_RELOC_M32R_GOT16_HI_SLO",
  "BFD_RELOC_M32R_GOT16_LO",
  "BFD_RELOC_M32R_GOTPC_HI_ULO",
  "BFD_RELOC_M32R_GOTPC_HI_SLO",
  "BFD_RELOC_M32R_GOTPC_LO",
  "BFD_RELOC_NDS32_20",
  "BFD_RELOC_NDS32_9_PCREL",
  "BFD_RELOC_NDS32_WORD_9_PCREL",
  "BFD_RELOC_NDS32_15_PCREL",
  "BFD_RELOC_NDS32_17_PCREL",
  "BFD_RELOC_NDS32_25_PCREL",
  "BFD_RELOC_NDS32_HI20",
  "BFD_RELOC_NDS32_LO12S3",
  "BFD_RELOC_NDS32_LO12S2",
  "BFD_RELOC_NDS32_LO12S1",
  "BFD_RELOC_NDS32_LO12S0",
  "BFD_RELOC_NDS32_LO12S0_ORI",
  "BFD_RELOC_NDS32_SDA15S3",
  "BFD_RELOC_NDS32_SDA15S2",
  "BFD_RELOC_NDS32_SDA15S1",
  "BFD_RELOC_NDS32_SDA15S0",
  "BFD_RELOC_NDS32_SDA16S3",
  "BFD_RELOC_NDS32_SDA17S2",
  "BFD_RELOC_NDS32_SDA18S1",
  "BFD_RELOC_NDS32_SDA19S0",
  "BFD_RELOC_NDS32_GOT20",
  "BFD_RELOC_NDS32_9_PLTREL",
  "BFD_RELOC_NDS32_25_PLTREL",
  "BFD_RELOC_NDS32_COPY",
  "BFD_RELOC_NDS32_GLOB_DAT",
  "BFD_RELOC_NDS32_JMP_SLOT",
  "BFD_RELOC_NDS32_RELATIVE",
  "BFD_RELOC_NDS32_GOTOFF",
  "BFD_RELOC_NDS32_GOTOFF_HI20",
  "BFD_RELOC_NDS32_GOTOFF_LO12",
  "BFD_RELOC_NDS32_GOTPC20",
  "BFD_RELOC_NDS32_GOT_HI20",
  "BFD_RELOC_NDS32_GOT_LO12",
  "BFD_RELOC_NDS32_GOTPC_HI20",
  "BFD_RELOC_NDS32_GOTPC_LO12",
  "BFD_RELOC_NDS32_INSN16",
  "BFD_RELOC_NDS32_LABEL",
  "BFD_RELOC_NDS32_LONGCALL1",
  "BFD_RELOC_NDS32_LONGCALL2",
  "BFD_RELOC_NDS32_LONGCALL3",
  "BFD_RELOC_NDS32_LONGJUMP1",
  "BFD_RELOC_NDS32_LONGJUMP2",
  "BFD_RELOC_NDS32_LONGJUMP3",
  "BFD_RELOC_NDS32_LOADSTORE",
  "BFD_RELOC_NDS32_9_FIXED",
  "BFD_RELOC_NDS32_15_FIXED",
  "BFD_RELOC_NDS32_17_FIXED",
  "BFD_RELOC_NDS32_25_FIXED",
  "BFD_RELOC_NDS32_LONGCALL4",
  "BFD_RELOC_NDS32_LONGCALL5",
  "BFD_RELOC_NDS32_LONGCALL6",
  "BFD_RELOC_NDS32_LONGJUMP4",
  "BFD_RELOC_NDS32_LONGJUMP5",
  "BFD_RELOC_NDS32_LONGJUMP6",
  "BFD_RELOC_NDS32_LONGJUMP7",
  "BFD_RELOC_NDS32_PLTREL_HI20",
  "BFD_RELOC_NDS32_PLTREL_LO12",
  "BFD_RELOC_NDS32_PLT_GOTREL_HI20",
  "BFD_RELOC_NDS32_PLT_GOTREL_LO12",
  "BFD_RELOC_NDS32_SDA12S2_DP",
  "BFD_RELOC_NDS32_SDA12S2_SP",
  "BFD_RELOC_NDS32_LO12S2_DP",
  "BFD_RELOC_NDS32_LO12S2_SP",
  "BFD_RELOC_NDS32_DWARF2_OP1",
  "BFD_RELOC_NDS32_DWARF2_OP2",
  "BFD_RELOC_NDS32_DWARF2_LEB",
  "BFD_RELOC_NDS32_UPDATE_TA",
  "BFD_RELOC_NDS32_PLT_GOTREL_LO20",
  "BFD_RELOC_NDS32_PLT_GOTREL_LO15",
  "BFD_RELOC_NDS32_PLT_GOTREL_LO19",
  "BFD_RELOC_NDS32_GOT_LO15",
  "BFD_RELOC_NDS32_GOT_LO19",
  "BFD_RELOC_NDS32_GOTOFF_LO15",
  "BFD_RELOC_NDS32_GOTOFF_LO19",
  "BFD_RELOC_NDS32_GOT15S2",
  "BFD_RELOC_NDS32_GOT17S2",
  "BFD_RELOC_NDS32_5",
  "BFD_RELOC_NDS32_10_UPCREL",
  "BFD_RELOC_NDS32_SDA_FP7U2_RELA",
  "BFD_RELOC_NDS32_RELAX_ENTRY",
  "BFD_RELOC_NDS32_GOT_SUFF",
  "BFD_RELOC_NDS32_GOTOFF_SUFF",
  "BFD_RELOC_NDS32_PLT_GOT_SUFF",
  "BFD_RELOC_NDS32_MULCALL_SUFF",
  "BFD_RELOC_NDS32_PTR",
  "BFD_RELOC_NDS32_PTR_COUNT",
  "BFD_RELOC_NDS32_PTR_RESOLVED",
  "BFD_RELOC_NDS32_PLTBLOCK",
  "BFD_RELOC_NDS32_RELAX_REGION_BEGIN",
  "BFD_RELOC_NDS32_RELAX_REGION_END",
  "BFD_RELOC_NDS32_MINUEND",
  "BFD_RELOC_NDS32_SUBTRAHEND",
  "BFD_RELOC_NDS32_DIFF8",
  "BFD_RELOC_NDS32_DIFF16",
  "BFD_RELOC_NDS32_DIFF32",
  "BFD_RELOC_NDS32_DIFF_ULEB128",
  "BFD_RELOC_NDS32_EMPTY",
  "BFD_RELOC_NDS32_25_ABS",
  "BFD_RELOC_NDS32_DATA",
  "BFD_RELOC_NDS32_TRAN",
  "BFD_RELOC_NDS32_17IFC_PCREL",
  "BFD_RELOC_NDS32_10IFCU_PCREL",
  "BFD_RELOC_NDS32_TPOFF",
  "BFD_RELOC_NDS32_GOTTPOFF",
  "BFD_RELOC_NDS32_TLS_LE_HI20",
  "BFD_RELOC_NDS32_TLS_LE_LO12",
  "BFD_RELOC_NDS32_TLS_LE_20",
  "BFD_RELOC_NDS32_TLS_LE_15S0",
  "BFD_RELOC_NDS32_TLS_LE_15S1",
  "BFD_RELOC_NDS32_TLS_LE_15S2",
  "BFD_RELOC_NDS32_TLS_LE_ADD",
  "BFD_RELOC_NDS32_TLS_LE_LS",
  "BFD_RELOC_NDS32_TLS_IE_HI20",
  "BFD_RELOC_NDS32_TLS_IE_LO12",
  "BFD_RELOC_NDS32_TLS_IE_LO12S2",
  "BFD_RELOC_NDS32_TLS_IEGP_HI20",
  "BFD_RELOC_NDS32_TLS_IEGP_LO12",
  "BFD_RELOC_NDS32_TLS_IEGP_LO12S2",
  "BFD_RELOC_NDS32_TLS_IEGP_LW",
  "BFD_RELOC_NDS32_TLS_DESC",
  "BFD_RELOC_NDS32_TLS_DESC_HI20",
  "BFD_RELOC_NDS32_TLS_DESC_LO12",
  "BFD_RELOC_NDS32_TLS_DESC_20",
  "BFD_RELOC_NDS32_TLS_DESC_SDA17S2",
  "BFD_RELOC_NDS32_TLS_DESC_ADD",
  "BFD_RELOC_NDS32_TLS_DESC_FUNC",
  "BFD_RELOC_NDS32_TLS_DESC_CALL",
  "BFD_RELOC_NDS32_TLS_DESC_MEM",
  "BFD_RELOC_NDS32_REMOVE",
  "BFD_RELOC_NDS32_GROUP",
  "BFD_RELOC_NDS32_LSI",
  "BFD_RELOC_V850_9_PCREL",
  "BFD_RELOC_V850_22_PCREL",
  "BFD_RELOC_V850_SDA_16_16_OFFSET",
  "BFD_RELOC_V850_SDA_15_16_OFFSET",
  "BFD_RELOC_V850_ZDA_16_16_OFFSET",
  "BFD_RELOC_V850_ZDA_15_16_OFFSET",
  "BFD_RELOC_V850_TDA_6_8_OFFSET",
  "BFD_RELOC_V850_TDA_7_8_OFFSET",
  "BFD_RELOC_V850_TDA_7_7_OFFSET",
  "BFD_RELOC_V850_TDA_16_16_OFFSET",
  "BFD_RELOC_V850_TDA_4_5_OFFSET",
  "BFD_RELOC_V850_TDA_4_4_OFFSET",
  "BFD_RELOC_V850_SDA_16_16_SPLIT_OFFSET",
  "BFD_RELOC_V850_ZDA_16_16_SPLIT_OFFSET",
  "BFD_RELOC_V850_CALLT_6_7_OFFSET",
  "BFD_RELOC_V850_CALLT_16_16_OFFSET",
  "BFD_RELOC_V850_LONGCALL",
  "BFD_RELOC_V850_LONGJUMP",
  "BFD_RELOC_V850_ALIGN",
  "BFD_RELOC_V850_LO16_SPLIT_OFFSET",
  "BFD_RELOC_V850_16_PCREL",
  "BFD_RELOC_V850_17_PCREL",
  "BFD_RELOC_V850_23",
  "BFD_RELOC_V850_32_PCREL",
  "BFD_RELOC_V850_32_ABS",
  "BFD_RELOC_V850_16_SPLIT_OFFSET",
  "BFD_RELOC_V850_16_S1",
  "BFD_RELOC_V850_LO16_S1",
  "BFD_RELOC_V850_CALLT_15_16_OFFSET",
  "BFD_RELOC_V850_32_GOTPCREL",
  "BFD_RELOC_V850_16_GOT",
  "BFD_RELOC_V850_32_GOT",
  "BFD_RELOC_V850_22_PLT_PCREL",
  "BFD_RELOC_V850_32_PLT_PCREL",
  "BFD_RELOC_V850_COPY",
  "BFD_RELOC_V850_GLOB_DAT",
  "BFD_RELOC_V850_JMP_SLOT",
  "BFD_RELOC_V850_RELATIVE",
  "BFD_RELOC_V850_16_GOTOFF",
  "BFD_RELOC_V850_32_GOTOFF",
  "BFD_RELOC_V850_CODE",
  "BFD_RELOC_V850_DATA",
  "BFD_RELOC_TIC30_LDP",
  "BFD_RELOC_TIC54X_PARTLS7",
  "BFD_RELOC_TIC54X_PARTMS9",
  "BFD_RELOC_TIC54X_23",
  "BFD_RELOC_TIC54X_16_OF_23",
  "BFD_RELOC_TIC54X_MS7_OF_23",
  "BFD_RELOC_C6000_PCR_S21",
  "BFD_RELOC_C6000_PCR_S12",
  "BFD_RELOC_C6000_PCR_S10",
  "BFD_RELOC_C6000_PCR_S7",
  "BFD_RELOC_C6000_ABS_S16",
  "BFD_RELOC_C6000_ABS_L16",
  "BFD_RELOC_C6000_ABS_H16",
  "BFD_RELOC_C6000_SBR_U15_B",
  "BFD_RELOC_C6000_SBR_U15_H",
  "BFD_RELOC_C6000_SBR_U15_W",
  "BFD_RELOC_C6000_SBR_S16",
  "BFD_RELOC_C6000_SBR_L16_B",
  "BFD_RELOC_C6000_SBR_L16_H",
  "BFD_RELOC_C6000_SBR_L16_W",
  "BFD_RELOC_C6000_SBR_H16_B",
  "BFD_RELOC_C6000_SBR_H16_H",
  "BFD_RELOC_C6000_SBR_H16_W",
  "BFD_RELOC_C6000_SBR_GOT_U15_W",
  "BFD_RELOC_C6000_SBR_GOT_L16_W",
  "BFD_RELOC_C6000_SBR_GOT_H16_W",
  "BFD_RELOC_C6000_DSBT_INDEX",
  "BFD_RELOC_C6000_PREL31",
  "BFD_RELOC_C6000_COPY",
  "BFD_RELOC_C6000_JUMP_SLOT",
  "BFD_RELOC_C6000_EHTYPE",
  "BFD_RELOC_C6000_PCR_H16",
  "BFD_RELOC_C6000_PCR_L16",
  "BFD_RELOC_C6000_ALIGN",
  "BFD_RELOC_C6000_FPHEAD",
  "BFD_RELOC_C6000_NOCMP",
  "BFD_RELOC_FR30_48",
  "BFD_RELOC_FR30_20",
  "BFD_RELOC_FR30_6_IN_4",
  "BFD_RELOC_FR30_8_IN_8",
  "BFD_RELOC_FR30_9_IN_8",
  "BFD_RELOC_FR30_10_IN_8",
  "BFD_RELOC_FR30_9_PCREL",
  "BFD_RELOC_FR30_12_PCREL",
  "BFD_RELOC_MCORE_PCREL_IMM8BY4",
  "BFD_RELOC_MCORE_PCREL_IMM11BY2",
  "BFD_RELOC_MCORE_PCREL_IMM4BY2",
  "BFD_RELOC_MCORE_PCREL_32",
  "BFD_RELOC_MCORE_PCREL_JSR_IMM11BY2",
  "BFD_RELOC_MCORE_RVA",
  "BFD_RELOC_MEP_8",
  "BFD_RELOC_MEP_16",
  "BFD_RELOC_MEP_32",
  "BFD_RELOC_MEP_PCREL8A2",
  "BFD_RELOC_MEP_PCREL12A2",
  "BFD_RELOC_MEP_PCREL17A2",
  "BFD_RELOC_MEP_PCREL24A2",
  "BFD_RELOC_MEP_PCABS24A2",
  "BFD_RELOC_MEP_LOW16",
  "BFD_RELOC_MEP_HI16U",
  "BFD_RELOC_MEP_HI16S",
  "BFD_RELOC_MEP_GPREL",
  "BFD_RELOC_MEP_TPREL",
  "BFD_RELOC_MEP_TPREL7",
  "BFD_RELOC_MEP_TPREL7A2",
  "BFD_RELOC_MEP_TPREL7A4",
  "BFD_RELOC_MEP_UIMM24",
  "BFD_RELOC_MEP_ADDR24A4",
  "BFD_RELOC_MEP_GNU_VTINHERIT",
  "BFD_RELOC_MEP_GNU_VTENTRY",

  "BFD_RELOC_METAG_HIADDR16",
  "BFD_RELOC_METAG_LOADDR16",
  "BFD_RELOC_METAG_RELBRANCH",
  "BFD_RELOC_METAG_GETSETOFF",
  "BFD_RELOC_METAG_HIOG",
  "BFD_RELOC_METAG_LOOG",
  "BFD_RELOC_METAG_REL8",
  "BFD_RELOC_METAG_REL16",
  "BFD_RELOC_METAG_HI16_GOTOFF",
  "BFD_RELOC_METAG_LO16_GOTOFF",
  "BFD_RELOC_METAG_GETSET_GOTOFF",
  "BFD_RELOC_METAG_GETSET_GOT",
  "BFD_RELOC_METAG_HI16_GOTPC",
  "BFD_RELOC_METAG_LO16_GOTPC",
  "BFD_RELOC_METAG_HI16_PLT",
  "BFD_RELOC_METAG_LO16_PLT",
  "BFD_RELOC_METAG_RELBRANCH_PLT",
  "BFD_RELOC_METAG_GOTOFF",
  "BFD_RELOC_METAG_PLT",
  "BFD_RELOC_METAG_COPY",
  "BFD_RELOC_METAG_JMP_SLOT",
  "BFD_RELOC_METAG_RELATIVE",
  "BFD_RELOC_METAG_GLOB_DAT",
  "BFD_RELOC_METAG_TLS_GD",
  "BFD_RELOC_METAG_TLS_LDM",
  "BFD_RELOC_METAG_TLS_LDO_HI16",
  "BFD_RELOC_METAG_TLS_LDO_LO16",
  "BFD_RELOC_METAG_TLS_LDO",
  "BFD_RELOC_METAG_TLS_IE",
  "BFD_RELOC_METAG_TLS_IENONPIC",
  "BFD_RELOC_METAG_TLS_IENONPIC_HI16",
  "BFD_RELOC_METAG_TLS_IENONPIC_LO16",
  "BFD_RELOC_METAG_TLS_TPOFF",
  "BFD_RELOC_METAG_TLS_DTPMOD",
  "BFD_RELOC_METAG_TLS_DTPOFF",
  "BFD_RELOC_METAG_TLS_LE",
  "BFD_RELOC_METAG_TLS_LE_HI16",
  "BFD_RELOC_METAG_TLS_LE_LO16",
  "BFD_RELOC_MMIX_GETA",
  "BFD_RELOC_MMIX_GETA_1",
  "BFD_RELOC_MMIX_GETA_2",
  "BFD_RELOC_MMIX_GETA_3",
  "BFD_RELOC_MMIX_CBRANCH",
  "BFD_RELOC_MMIX_CBRANCH_J",
  "BFD_RELOC_MMIX_CBRANCH_1",
  "BFD_RELOC_MMIX_CBRANCH_2",
  "BFD_RELOC_MMIX_CBRANCH_3",
  "BFD_RELOC_MMIX_PUSHJ",
  "BFD_RELOC_MMIX_PUSHJ_1",
  "BFD_RELOC_MMIX_PUSHJ_2",
  "BFD_RELOC_MMIX_PUSHJ_3",
  "BFD_RELOC_MMIX_PUSHJ_STUBBABLE",
  "BFD_RELOC_MMIX_JMP",
  "BFD_RELOC_MMIX_JMP_1",
  "BFD_RELOC_MMIX_JMP_2",
  "BFD_RELOC_MMIX_JMP_3",
  "BFD_RELOC_MMIX_ADDR19",
  "BFD_RELOC_MMIX_ADDR27",
  "BFD_RELOC_MMIX_REG_OR_BYTE",
  "BFD_RELOC_MMIX_REG",
  "BFD_RELOC_MMIX_BASE_PLUS_OFFSET",
  "BFD_RELOC_MMIX_LOCAL",
  "BFD_RELOC_AVR_7_PCREL",
  "BFD_RELOC_AVR_13_PCREL",
  "BFD_RELOC_AVR_16_PM",
  "BFD_RELOC_AVR_LO8_LDI",
  "BFD_RELOC_AVR_HI8_LDI",
  "BFD_RELOC_AVR_HH8_LDI",
  "BFD_RELOC_AVR_MS8_LDI",
  "BFD_RELOC_AVR_LO8_LDI_NEG",
  "BFD_RELOC_AVR_HI8_LDI_NEG",
  "BFD_RELOC_AVR_HH8_LDI_NEG",
  "BFD_RELOC_AVR_MS8_LDI_NEG",
  "BFD_RELOC_AVR_LO8_LDI_PM",
  "BFD_RELOC_AVR_LO8_LDI_GS",
  "BFD_RELOC_AVR_HI8_LDI_PM",
  "BFD_RELOC_AVR_HI8_LDI_GS",
  "BFD_RELOC_AVR_HH8_LDI_PM",
  "BFD_RELOC_AVR_LO8_LDI_PM_NEG",
  "BFD_RELOC_AVR_HI8_LDI_PM_NEG",
  "BFD_RELOC_AVR_HH8_LDI_PM_NEG",
  "BFD_RELOC_AVR_CALL",
  "BFD_RELOC_AVR_LDI",
  "BFD_RELOC_AVR_6",
  "BFD_RELOC_AVR_6_ADIW",
  "BFD_RELOC_AVR_8_LO",
  "BFD_RELOC_AVR_8_HI",
  "BFD_RELOC_AVR_8_HLO",
  "BFD_RELOC_AVR_DIFF8",
  "BFD_RELOC_AVR_DIFF16",
  "BFD_RELOC_AVR_DIFF32",
  "BFD_RELOC_AVR_LDS_STS_16",
  "BFD_RELOC_AVR_PORT6",
  "BFD_RELOC_AVR_PORT5",
  "BFD_RELOC_RISCV_HI20",
  "BFD_RELOC_RISCV_PCREL_HI20",
  "BFD_RELOC_RISCV_PCREL_LO12_I",
  "BFD_RELOC_RISCV_PCREL_LO12_S",
  "BFD_RELOC_RISCV_LO12_I",
  "BFD_RELOC_RISCV_LO12_S",
  "BFD_RELOC_RISCV_GPREL12_I",
  "BFD_RELOC_RISCV_GPREL12_S",
  "BFD_RELOC_RISCV_TPREL_HI20",
  "BFD_RELOC_RISCV_TPREL_LO12_I",
  "BFD_RELOC_RISCV_TPREL_LO12_S",
  "BFD_RELOC_RISCV_TPREL_ADD",
  "BFD_RELOC_RISCV_CALL",
  "BFD_RELOC_RISCV_CALL_PLT",
  "BFD_RELOC_RISCV_ADD8",
  "BFD_RELOC_RISCV_ADD16",
  "BFD_RELOC_RISCV_ADD32",
  "BFD_RELOC_RISCV_ADD64",
  "BFD_RELOC_RISCV_SUB8",
  "BFD_RELOC_RISCV_SUB16",
  "BFD_RELOC_RISCV_SUB32",
  "BFD_RELOC_RISCV_SUB64",
  "BFD_RELOC_RISCV_GOT_HI20",
  "BFD_RELOC_RISCV_TLS_GOT_HI20",
  "BFD_RELOC_RISCV_TLS_GD_HI20",
  "BFD_RELOC_RISCV_JMP",
  "BFD_RELOC_RISCV_TLS_DTPMOD32",
  "BFD_RELOC_RISCV_TLS_DTPREL32",
  "BFD_RELOC_RISCV_TLS_DTPMOD64",
  "BFD_RELOC_RISCV_TLS_DTPREL64",
  "BFD_RELOC_RISCV_TLS_TPREL32",
  "BFD_RELOC_RISCV_TLS_TPREL64",
  "BFD_RELOC_RISCV_ALIGN",
  "BFD_RELOC_RISCV_RVC_BRANCH",
  "BFD_RELOC_RISCV_RVC_JUMP",
  "BFD_RELOC_RISCV_RVC_LUI",
  "BFD_RELOC_RISCV_GPREL_I",
  "BFD_RELOC_RISCV_GPREL_S",
  "BFD_RELOC_RISCV_TPREL_I",
  "BFD_RELOC_RISCV_TPREL_S",
  "BFD_RELOC_RISCV_RELAX",
  "BFD_RELOC_RISCV_CFA",
  "BFD_RELOC_RISCV_SUB6",
  "BFD_RELOC_RISCV_SET6",
  "BFD_RELOC_RISCV_SET8",
  "BFD_RELOC_RISCV_SET16",
  "BFD_RELOC_RISCV_SET32",
  "BFD_RELOC_RISCV_32_PCREL",
  "BFD_RELOC_RISCV_SET_ULEB128",
  "BFD_RELOC_RISCV_SUB_ULEB128",
  "BFD_RELOC_RL78_NEG8",
  "BFD_RELOC_RL78_NEG16",
  "BFD_RELOC_RL78_NEG24",
  "BFD_RELOC_RL78_NEG32",
  "BFD_RELOC_RL78_16_OP",
  "BFD_RELOC_RL78_24_OP",
  "BFD_RELOC_RL78_32_OP",
  "BFD_RELOC_RL78_8U",
  "BFD_RELOC_RL78_16U",
  "BFD_RELOC_RL78_24U",
  "BFD_RELOC_RL78_DIR3U_PCREL",
  "BFD_RELOC_RL78_DIFF",
  "BFD_RELOC_RL78_GPRELB",
  "BFD_RELOC_RL78_GPRELW",
  "BFD_RELOC_RL78_GPRELL",
  "BFD_RELOC_RL78_SYM",
  "BFD_RELOC_RL78_OP_SUBTRACT",
  "BFD_RELOC_RL78_OP_NEG",
  "BFD_RELOC_RL78_OP_AND",
  "BFD_RELOC_RL78_OP_SHRA",
  "BFD_RELOC_RL78_ABS8",
  "BFD_RELOC_RL78_ABS16",
  "BFD_RELOC_RL78_ABS16_REV",
  "BFD_RELOC_RL78_ABS32",
  "BFD_RELOC_RL78_ABS32_REV",
  "BFD_RELOC_RL78_ABS16U",
  "BFD_RELOC_RL78_ABS16UW",
  "BFD_RELOC_RL78_ABS16UL",
  "BFD_RELOC_RL78_RELAX",
  "BFD_RELOC_RL78_HI16",
  "BFD_RELOC_RL78_HI8",
  "BFD_RELOC_RL78_LO16",
  "BFD_RELOC_RL78_CODE",
  "BFD_RELOC_RL78_SADDR",
  "BFD_RELOC_RX_NEG8",
  "BFD_RELOC_RX_NEG16",
  "BFD_RELOC_RX_NEG24",
  "BFD_RELOC_RX_NEG32",
  "BFD_RELOC_RX_16_OP",
  "BFD_RELOC_RX_24_OP",
  "BFD_RELOC_RX_32_OP",
  "BFD_RELOC_RX_8U",
  "BFD_RELOC_RX_16U",
  "BFD_RELOC_RX_24U",
  "BFD_RELOC_RX_DIR3U_PCREL",
  "BFD_RELOC_RX_DIFF",
  "BFD_RELOC_RX_GPRELB",
  "BFD_RELOC_RX_GPRELW",
  "BFD_RELOC_RX_GPRELL",
  "BFD_RELOC_RX_SYM",
  "BFD_RELOC_RX_OP_SUBTRACT",
  "BFD_RELOC_RX_OP_NEG",
  "BFD_RELOC_RX_ABS8",
  "BFD_RELOC_RX_ABS16",
  "BFD_RELOC_RX_ABS16_REV",
  "BFD_RELOC_RX_ABS32",
  "BFD_RELOC_RX_ABS32_REV",
  "BFD_RELOC_RX_ABS16U",
  "BFD_RELOC_RX_ABS16UW",
  "BFD_RELOC_RX_ABS16UL",
  "BFD_RELOC_RX_RELAX",
  "BFD_RELOC_390_12",
  "BFD_RELOC_390_GOT12",
  "BFD_RELOC_390_PLT32",
  "BFD_RELOC_390_COPY",
  "BFD_RELOC_390_GLOB_DAT",
  "BFD_RELOC_390_JMP_SLOT",
  "BFD_RELOC_390_RELATIVE",
  "BFD_RELOC_390_GOTPC",
  "BFD_RELOC_390_GOT16",
  "BFD_RELOC_390_PC12DBL",
  "BFD_RELOC_390_PLT12DBL",
  "BFD_RELOC_390_PC16DBL",
  "BFD_RELOC_390_PLT16DBL",
  "BFD_RELOC_390_PC24DBL",
  "BFD_RELOC_390_PLT24DBL",
  "BFD_RELOC_390_PC32DBL",
  "BFD_RELOC_390_PLT32DBL",
  "BFD_RELOC_390_GOTPCDBL",
  "BFD_RELOC_390_GOT64",
  "BFD_RELOC_390_PLT64",
  "BFD_RELOC_390_GOTENT",
  "BFD_RELOC_390_GOTOFF64",
  "BFD_RELOC_390_GOTPLT12",
  "BFD_RELOC_390_GOTPLT16",
  "BFD_RELOC_390_GOTPLT32",
  "BFD_RELOC_390_GOTPLT64",
  "BFD_RELOC_390_GOTPLTENT",
  "BFD_RELOC_390_PLTOFF16",
  "BFD_RELOC_390_PLTOFF32",
  "BFD_RELOC_390_PLTOFF64",
  "BFD_RELOC_390_TLS_LOAD",
  "BFD_RELOC_390_TLS_GDCALL",
  "BFD_RELOC_390_TLS_LDCALL",
  "BFD_RELOC_390_TLS_GD32",
  "BFD_RELOC_390_TLS_GD64",
  "BFD_RELOC_390_TLS_GOTIE12",
  "BFD_RELOC_390_TLS_GOTIE32",
  "BFD_RELOC_390_TLS_GOTIE64",
  "BFD_RELOC_390_TLS_LDM32",
  "BFD_RELOC_390_TLS_LDM64",
  "BFD_RELOC_390_TLS_IE32",
  "BFD_RELOC_390_TLS_IE64",
  "BFD_RELOC_390_TLS_IEENT",
  "BFD_RELOC_390_TLS_LE32",
  "BFD_RELOC_390_TLS_LE64",
  "BFD_RELOC_390_TLS_LDO32",
  "BFD_RELOC_390_TLS_LDO64",
  "BFD_RELOC_390_TLS_DTPMOD",
  "BFD_RELOC_390_TLS_DTPOFF",
  "BFD_RELOC_390_TLS_TPOFF",
  "BFD_RELOC_390_20",
  "BFD_RELOC_390_GOT20",
  "BFD_RELOC_390_GOTPLT20",
  "BFD_RELOC_390_TLS_GOTIE20",
  "BFD_RELOC_390_IRELATIVE",
  "BFD_RELOC_SCORE_GPREL15",
  "BFD_RELOC_SCORE_DUMMY2",
  "BFD_RELOC_SCORE_JMP",
  "BFD_RELOC_SCORE_BRANCH",
  "BFD_RELOC_SCORE_IMM30",
  "BFD_RELOC_SCORE_IMM32",
  "BFD_RELOC_SCORE16_JMP",
  "BFD_RELOC_SCORE16_BRANCH",
  "BFD_RELOC_SCORE_BCMP",
  "BFD_RELOC_SCORE_GOT15",
  "BFD_RELOC_SCORE_GOT_LO16",
  "BFD_RELOC_SCORE_CALL15",
  "BFD_RELOC_SCORE_DUMMY_HI16",
  "BFD_RELOC_IP2K_FR9",
  "BFD_RELOC_IP2K_BANK",
  "BFD_RELOC_IP2K_ADDR16CJP",
  "BFD_RELOC_IP2K_PAGE3",
  "BFD_RELOC_IP2K_LO8DATA",
  "BFD_RELOC_IP2K_HI8DATA",
  "BFD_RELOC_IP2K_EX8DATA",
  "BFD_RELOC_IP2K_LO8INSN",
  "BFD_RELOC_IP2K_HI8INSN",
  "BFD_RELOC_IP2K_PC_SKIP",
  "BFD_RELOC_IP2K_TEXT",
  "BFD_RELOC_IP2K_FR_OFFSET",
  "BFD_RELOC_VPE4KMATH_DATA",
  "BFD_RELOC_VPE4KMATH_INSN",
  "BFD_RELOC_VTABLE_INHERIT",
  "BFD_RELOC_VTABLE_ENTRY",
  "BFD_RELOC_IA64_IMM14",
  "BFD_RELOC_IA64_IMM22",
  "BFD_RELOC_IA64_IMM64",
  "BFD_RELOC_IA64_DIR32MSB",
  "BFD_RELOC_IA64_DIR32LSB",
  "BFD_RELOC_IA64_DIR64MSB",
  "BFD_RELOC_IA64_DIR64LSB",
  "BFD_RELOC_IA64_GPREL22",
  "BFD_RELOC_IA64_GPREL64I",
  "BFD_RELOC_IA64_GPREL32MSB",
  "BFD_RELOC_IA64_GPREL32LSB",
  "BFD_RELOC_IA64_GPREL64MSB",
  "BFD_RELOC_IA64_GPREL64LSB",
  "BFD_RELOC_IA64_LTOFF22",
  "BFD_RELOC_IA64_LTOFF64I",
  "BFD_RELOC_IA64_PLTOFF22",
  "BFD_RELOC_IA64_PLTOFF64I",
  "BFD_RELOC_IA64_PLTOFF64MSB",
  "BFD_RELOC_IA64_PLTOFF64LSB",
  "BFD_RELOC_IA64_FPTR64I",
  "BFD_RELOC_IA64_FPTR32MSB",
  "BFD_RELOC_IA64_FPTR32LSB",
  "BFD_RELOC_IA64_FPTR64MSB",
  "BFD_RELOC_IA64_FPTR64LSB",
  "BFD_RELOC_IA64_PCREL21B",
  "BFD_RELOC_IA64_PCREL21BI",
  "BFD_RELOC_IA64_PCREL21M",
  "BFD_RELOC_IA64_PCREL21F",
  "BFD_RELOC_IA64_PCREL22",
  "BFD_RELOC_IA64_PCREL60B",
  "BFD_RELOC_IA64_PCREL64I",
  "BFD_RELOC_IA64_PCREL32MSB",
  "BFD_RELOC_IA64_PCREL32LSB",
  "BFD_RELOC_IA64_PCREL64MSB",
  "BFD_RELOC_IA64_PCREL64LSB",
  "BFD_RELOC_IA64_LTOFF_FPTR22",
  "BFD_RELOC_IA64_LTOFF_FPTR64I",
  "BFD_RELOC_IA64_LTOFF_FPTR32MSB",
  "BFD_RELOC_IA64_LTOFF_FPTR32LSB",
  "BFD_RELOC_IA64_LTOFF_FPTR64MSB",
  "BFD_RELOC_IA64_LTOFF_FPTR64LSB",
  "BFD_RELOC_IA64_SEGREL32MSB",
  "BFD_RELOC_IA64_SEGREL32LSB",
  "BFD_RELOC_IA64_SEGREL64MSB",
  "BFD_RELOC_IA64_SEGREL64LSB",
  "BFD_RELOC_IA64_SECREL32MSB",
  "BFD_RELOC_IA64_SECREL32LSB",
  "BFD_RELOC_IA64_SECREL64MSB",
  "BFD_RELOC_IA64_SECREL64LSB",
  "BFD_RELOC_IA64_REL32MSB",
  "BFD_RELOC_IA64_REL32LSB",
  "BFD_RELOC_IA64_REL64MSB",
  "BFD_RELOC_IA64_REL64LSB",
  "BFD_RELOC_IA64_LTV32MSB",
  "BFD_RELOC_IA64_LTV32LSB",
  "BFD_RELOC_IA64_LTV64MSB",
  "BFD_RELOC_IA64_LTV64LSB",
  "BFD_RELOC_IA64_IPLTMSB",
  "BFD_RELOC_IA64_IPLTLSB",
  "BFD_RELOC_IA64_COPY",
  "BFD_RELOC_IA64_LTOFF22X",
  "BFD_RELOC_IA64_LDXMOV",
  "BFD_RELOC_IA64_TPREL14",
  "BFD_RELOC_IA64_TPREL22",
  "BFD_RELOC_IA64_TPREL64I",
  "BFD_RELOC_IA64_TPREL64MSB",
  "BFD_RELOC_IA64_TPREL64LSB",
  "BFD_RELOC_IA64_LTOFF_TPREL22",
  "BFD_RELOC_IA64_DTPMOD64MSB",
  "BFD_RELOC_IA64_DTPMOD64LSB",
  "BFD_RELOC_IA64_LTOFF_DTPMOD22",
  "BFD_RELOC_IA64_DTPREL14",
  "BFD_RELOC_IA64_DTPREL22",
  "BFD_RELOC_IA64_DTPREL64I",
  "BFD_RELOC_IA64_DTPREL32MSB",
  "BFD_RELOC_IA64_DTPREL32LSB",
  "BFD_RELOC_IA64_DTPREL64MSB",
  "BFD_RELOC_IA64_DTPREL64LSB",
  "BFD_RELOC_IA64_LTOFF_DTPREL22",
  "BFD_RELOC_M68HC11_HI8",
  "BFD_RELOC_M68HC11_LO8",
  "BFD_RELOC_M68HC11_3B",
  "BFD_RELOC_M68HC11_RL_JUMP",
  "BFD_RELOC_M68HC11_RL_GROUP",
  "BFD_RELOC_M68HC11_LO16",
  "BFD_RELOC_M68HC11_PAGE",
  "BFD_RELOC_M68HC11_24",
  "BFD_RELOC_M68HC12_5B",
  "BFD_RELOC_XGATE_RL_JUMP",
  "BFD_RELOC_XGATE_RL_GROUP",
  "BFD_RELOC_XGATE_LO16",
  "BFD_RELOC_XGATE_GPAGE",
  "BFD_RELOC_XGATE_24",
  "BFD_RELOC_XGATE_PCREL_9",
  "BFD_RELOC_XGATE_PCREL_10",
  "BFD_RELOC_XGATE_IMM8_LO",
  "BFD_RELOC_XGATE_IMM8_HI",
  "BFD_RELOC_XGATE_IMM3",
  "BFD_RELOC_XGATE_IMM4",
  "BFD_RELOC_XGATE_IMM5",
  "BFD_RELOC_M68HC12_9B",
  "BFD_RELOC_M68HC12_16B",
  "BFD_RELOC_M68HC12_9_PCREL",
  "BFD_RELOC_M68HC12_10_PCREL",
  "BFD_RELOC_M68HC12_LO8XG",
  "BFD_RELOC_M68HC12_HI8XG",
  "BFD_RELOC_S12Z_15_PCREL",
  "BFD_RELOC_CR16_NUM8",
  "BFD_RELOC_CR16_NUM16",
  "BFD_RELOC_CR16_NUM32",
  "BFD_RELOC_CR16_NUM32a",
  "BFD_RELOC_CR16_REGREL0",
  "BFD_RELOC_CR16_REGREL4",
  "BFD_RELOC_CR16_REGREL4a",
  "BFD_RELOC_CR16_REGREL14",
  "BFD_RELOC_CR16_REGREL14a",
  "BFD_RELOC_CR16_REGREL16",
  "BFD_RELOC_CR16_REGREL20",
  "BFD_RELOC_CR16_REGREL20a",
  "BFD_RELOC_CR16_ABS20",
  "BFD_RELOC_CR16_ABS24",
  "BFD_RELOC_CR16_IMM4",
  "BFD_RELOC_CR16_IMM8",
  "BFD_RELOC_CR16_IMM16",
  "BFD_RELOC_CR16_IMM20",
  "BFD_RELOC_CR16_IMM24",
  "BFD_RELOC_CR16_IMM32",
  "BFD_RELOC_CR16_IMM32a",
  "BFD_RELOC_CR16_DISP4",
  "BFD_RELOC_CR16_DISP8",
  "BFD_RELOC_CR16_DISP16",
  "BFD_RELOC_CR16_DISP20",
  "BFD_RELOC_CR16_DISP24",
  "BFD_RELOC_CR16_DISP24a",
  "BFD_RELOC_CR16_SWITCH8",
  "BFD_RELOC_CR16_SWITCH16",
  "BFD_RELOC_CR16_SWITCH32",
  "BFD_RELOC_CR16_GOT_REGREL20",
  "BFD_RELOC_CR16_GOTC_REGREL20",
  "BFD_RELOC_CR16_GLOB_DAT",
  "BFD_RELOC_CRX_REL4",
  "BFD_RELOC_CRX_REL8",
  "BFD_RELOC_CRX_REL8_CMP",
  "BFD_RELOC_CRX_REL16",
  "BFD_RELOC_CRX_REL24",
  "BFD_RELOC_CRX_REL32",
  "BFD_RELOC_CRX_REGREL12",
  "BFD_RELOC_CRX_REGREL22",
  "BFD_RELOC_CRX_REGREL28",
  "BFD_RELOC_CRX_REGREL32",
  "BFD_RELOC_CRX_ABS16",
  "BFD_RELOC_CRX_ABS32",
  "BFD_RELOC_CRX_NUM8",
  "BFD_RELOC_CRX_NUM16",
  "BFD_RELOC_CRX_NUM32",
  "BFD_RELOC_CRX_IMM16",
  "BFD_RELOC_CRX_IMM32",
  "BFD_RELOC_CRX_SWITCH8",
  "BFD_RELOC_CRX_SWITCH16",
  "BFD_RELOC_CRX_SWITCH32",
  "BFD_RELOC_CRIS_BDISP8",
  "BFD_RELOC_CRIS_UNSIGNED_5",
  "BFD_RELOC_CRIS_SIGNED_6",
  "BFD_RELOC_CRIS_UNSIGNED_6",
  "BFD_RELOC_CRIS_SIGNED_8",
  "BFD_RELOC_CRIS_UNSIGNED_8",
  "BFD_RELOC_CRIS_SIGNED_16",
  "BFD_RELOC_CRIS_UNSIGNED_16",
  "BFD_RELOC_CRIS_LAPCQ_OFFSET",
  "BFD_RELOC_CRIS_UNSIGNED_4",
  "BFD_RELOC_CRIS_COPY",
  "BFD_RELOC_CRIS_GLOB_DAT",
  "BFD_RELOC_CRIS_JUMP_SLOT",
  "BFD_RELOC_CRIS_RELATIVE",
  "BFD_RELOC_CRIS_32_GOT",
  "BFD_RELOC_CRIS_16_GOT",
  "BFD_RELOC_CRIS_32_GOTPLT",
  "BFD_RELOC_CRIS_16_GOTPLT",
  "BFD_RELOC_CRIS_32_GOTREL",
  "BFD_RELOC_CRIS_32_PLT_GOTREL",
  "BFD_RELOC_CRIS_32_PLT_PCREL",
  "BFD_RELOC_CRIS_32_GOT_GD",
  "BFD_RELOC_CRIS_16_GOT_GD",
  "BFD_RELOC_CRIS_32_GD",
  "BFD_RELOC_CRIS_DTP",
  "BFD_RELOC_CRIS_32_DTPREL",
  "BFD_RELOC_CRIS_16_DTPREL",
  "BFD_RELOC_CRIS_32_GOT_TPREL",
  "BFD_RELOC_CRIS_16_GOT_TPREL",
  "BFD_RELOC_CRIS_32_TPREL",
  "BFD_RELOC_CRIS_16_TPREL",
  "BFD_RELOC_CRIS_DTPMOD",
  "BFD_RELOC_CRIS_32_IE",
  "BFD_RELOC_OR1K_REL_26",
  "BFD_RELOC_OR1K_SLO16",
  "BFD_RELOC_OR1K_PCREL_PG21",
  "BFD_RELOC_OR1K_LO13",
  "BFD_RELOC_OR1K_SLO13",
  "BFD_RELOC_OR1K_GOTPC_HI16",
  "BFD_RELOC_OR1K_GOTPC_LO16",
  "BFD_RELOC_OR1K_GOT_AHI16",
  "BFD_RELOC_OR1K_GOT16",
  "BFD_RELOC_OR1K_GOT_PG21",
  "BFD_RELOC_OR1K_GOT_LO13",
  "BFD_RELOC_OR1K_PLT26",
  "BFD_RELOC_OR1K_PLTA26",
  "BFD_RELOC_OR1K_GOTOFF_SLO16",
  "BFD_RELOC_OR1K_COPY",
  "BFD_RELOC_OR1K_GLOB_DAT",
  "BFD_RELOC_OR1K_JMP_SLOT",
  "BFD_RELOC_OR1K_RELATIVE",
  "BFD_RELOC_OR1K_TLS_GD_HI16",
  "BFD_RELOC_OR1K_TLS_GD_LO16",
  "BFD_RELOC_OR1K_TLS_GD_PG21",
  "BFD_RELOC_OR1K_TLS_GD_LO13",
  "BFD_RELOC_OR1K_TLS_LDM_HI16",
  "BFD_RELOC_OR1K_TLS_LDM_LO16",
  "BFD_RELOC_OR1K_TLS_LDM_PG21",
  "BFD_RELOC_OR1K_TLS_LDM_LO13",
  "BFD_RELOC_OR1K_TLS_LDO_HI16",
  "BFD_RELOC_OR1K_TLS_LDO_LO16",
  "BFD_RELOC_OR1K_TLS_IE_HI16",
  "BFD_RELOC_OR1K_TLS_IE_AHI16",
  "BFD_RELOC_OR1K_TLS_IE_LO16",
  "BFD_RELOC_OR1K_TLS_IE_PG21",
  "BFD_RELOC_OR1K_TLS_IE_LO13",
  "BFD_RELOC_OR1K_TLS_LE_HI16",
  "BFD_RELOC_OR1K_TLS_LE_AHI16",
  "BFD_RELOC_OR1K_TLS_LE_LO16",
  "BFD_RELOC_OR1K_TLS_LE_SLO16",
  "BFD_RELOC_OR1K_TLS_TPOFF",
  "BFD_RELOC_OR1K_TLS_DTPOFF",
  "BFD_RELOC_OR1K_TLS_DTPMOD",
  "BFD_RELOC_H8_DIR16A8",
  "BFD_RELOC_H8_DIR16R8",
  "BFD_RELOC_H8_DIR24A8",
  "BFD_RELOC_H8_DIR24R8",
  "BFD_RELOC_H8_DIR32A16",
  "BFD_RELOC_H8_DISP32A16",
  "BFD_RELOC_XSTORMY16_REL_12",
  "BFD_RELOC_XSTORMY16_12",
  "BFD_RELOC_XSTORMY16_24",
  "BFD_RELOC_XSTORMY16_FPTR16",
  "BFD_RELOC_RELC",

  "BFD_RELOC_VAX_GLOB_DAT",
  "BFD_RELOC_VAX_JMP_SLOT",
  "BFD_RELOC_VAX_RELATIVE",
  "BFD_RELOC_MT_PC16",
  "BFD_RELOC_MT_HI16",
  "BFD_RELOC_MT_LO16",
  "BFD_RELOC_MT_GNU_VTINHERIT",
  "BFD_RELOC_MT_GNU_VTENTRY",
  "BFD_RELOC_MT_PCINSN8",
  "BFD_RELOC_MSP430_10_PCREL",
  "BFD_RELOC_MSP430_16_PCREL",
  "BFD_RELOC_MSP430_16",
  "BFD_RELOC_MSP430_16_PCREL_BYTE",
  "BFD_RELOC_MSP430_16_BYTE",
  "BFD_RELOC_MSP430_2X_PCREL",
  "BFD_RELOC_MSP430_RL_PCREL",
  "BFD_RELOC_MSP430_ABS8",
  "BFD_RELOC_MSP430X_PCR20_EXT_SRC",
  "BFD_RELOC_MSP430X_PCR20_EXT_DST",
  "BFD_RELOC_MSP430X_PCR20_EXT_ODST",
  "BFD_RELOC_MSP430X_ABS20_EXT_SRC",
  "BFD_RELOC_MSP430X_ABS20_EXT_DST",
  "BFD_RELOC_MSP430X_ABS20_EXT_ODST",
  "BFD_RELOC_MSP430X_ABS20_ADR_SRC",
  "BFD_RELOC_MSP430X_ABS20_ADR_DST",
  "BFD_RELOC_MSP430X_PCR16",
  "BFD_RELOC_MSP430X_PCR20_CALL",
  "BFD_RELOC_MSP430X_ABS16",
  "BFD_RELOC_MSP430_ABS_HI16",
  "BFD_RELOC_MSP430_PREL31",
  "BFD_RELOC_MSP430_SYM_DIFF",
  "BFD_RELOC_MSP430_SET_ULEB128",
  "BFD_RELOC_MSP430_SUB_ULEB128",
  "BFD_RELOC_NIOS2_S16",
  "BFD_RELOC_NIOS2_U16",
  "BFD_RELOC_NIOS2_CALL26",
  "BFD_RELOC_NIOS2_IMM5",
  "BFD_RELOC_NIOS2_CACHE_OPX",
  "BFD_RELOC_NIOS2_IMM6",
  "BFD_RELOC_NIOS2_IMM8",
  "BFD_RELOC_NIOS2_HI16",
  "BFD_RELOC_NIOS2_LO16",
  "BFD_RELOC_NIOS2_HIADJ16",
  "BFD_RELOC_NIOS2_GPREL",
  "BFD_RELOC_NIOS2_UJMP",
  "BFD_RELOC_NIOS2_CJMP",
  "BFD_RELOC_NIOS2_CALLR",
  "BFD_RELOC_NIOS2_ALIGN",
  "BFD_RELOC_NIOS2_GOT16",
  "BFD_RELOC_NIOS2_CALL16",
  "BFD_RELOC_NIOS2_GOTOFF_LO",
  "BFD_RELOC_NIOS2_GOTOFF_HA",
  "BFD_RELOC_NIOS2_PCREL_LO",
  "BFD_RELOC_NIOS2_PCREL_HA",
  "BFD_RELOC_NIOS2_TLS_GD16",
  "BFD_RELOC_NIOS2_TLS_LDM16",
  "BFD_RELOC_NIOS2_TLS_LDO16",
  "BFD_RELOC_NIOS2_TLS_IE16",
  "BFD_RELOC_NIOS2_TLS_LE16",
  "BFD_RELOC_NIOS2_TLS_DTPMOD",
  "BFD_RELOC_NIOS2_TLS_DTPREL",
  "BFD_RELOC_NIOS2_TLS_TPREL",
  "BFD_RELOC_NIOS2_COPY",
  "BFD_RELOC_NIOS2_GLOB_DAT",
  "BFD_RELOC_NIOS2_JUMP_SLOT",
  "BFD_RELOC_NIOS2_RELATIVE",
  "BFD_RELOC_NIOS2_GOTOFF",
  "BFD_RELOC_NIOS2_CALL26_NOAT",
  "BFD_RELOC_NIOS2_GOT_LO",
  "BFD_RELOC_NIOS2_GOT_HA",
  "BFD_RELOC_NIOS2_CALL_LO",
  "BFD_RELOC_NIOS2_CALL_HA",
  "BFD_RELOC_NIOS2_R2_S12",
  "BFD_RELOC_NIOS2_R2_I10_1_PCREL",
  "BFD_RELOC_NIOS2_R2_T1I7_1_PCREL",
  "BFD_RELOC_NIOS2_R2_T1I7_2",
  "BFD_RELOC_NIOS2_R2_T2I4",
  "BFD_RELOC_NIOS2_R2_T2I4_1",
  "BFD_RELOC_NIOS2_R2_T2I4_2",
  "BFD_RELOC_NIOS2_R2_X1I7_2",
  "BFD_RELOC_NIOS2_R2_X2L5",
  "BFD_RELOC_NIOS2_R2_F1I5_2",
  "BFD_RELOC_NIOS2_R2_L5I4X1",
  "BFD_RELOC_NIOS2_R2_T1X1I6",
  "BFD_RELOC_NIOS2_R2_T1X1I6_2",
  "BFD_RELOC_PRU_U16",
  "BFD_RELOC_PRU_U16_PMEMIMM",
  "BFD_RELOC_PRU_LDI32",
  "BFD_RELOC_PRU_S10_PCREL",
  "BFD_RELOC_PRU_U8_PCREL",
  "BFD_RELOC_PRU_32_PMEM",
  "BFD_RELOC_PRU_16_PMEM",
  "BFD_RELOC_PRU_GNU_DIFF8",
  "BFD_RELOC_PRU_GNU_DIFF16",
  "BFD_RELOC_PRU_GNU_DIFF32",
  "BFD_RELOC_PRU_GNU_DIFF16_PMEM",
  "BFD_RELOC_PRU_GNU_DIFF32_PMEM",
  "BFD_RELOC_IQ2000_OFFSET_16",
  "BFD_RELOC_IQ2000_OFFSET_21",
  "BFD_RELOC_IQ2000_UHI16",
  "BFD_RELOC_XTENSA_RTLD",
  "BFD_RELOC_XTENSA_GLOB_DAT",
  "BFD_RELOC_XTENSA_JMP_SLOT",
  "BFD_RELOC_XTENSA_RELATIVE",
  "BFD_RELOC_XTENSA_PLT",
  "BFD_RELOC_XTENSA_DIFF8",
  "BFD_RELOC_XTENSA_DIFF16",
  "BFD_RELOC_XTENSA_DIFF32",
  "BFD_RELOC_XTENSA_SLOT0_OP",
  "BFD_RELOC_XTENSA_SLOT1_OP",
  "BFD_RELOC_XTENSA_SLOT2_OP",
  "BFD_RELOC_XTENSA_SLOT3_OP",
  "BFD_RELOC_XTENSA_SLOT4_OP",
  "BFD_RELOC_XTENSA_SLOT5_OP",
  "BFD_RELOC_XTENSA_SLOT6_OP",
  "BFD_RELOC_XTENSA_SLOT7_OP",
  "BFD_RELOC_XTENSA_SLOT8_OP",
  "BFD_RELOC_XTENSA_SLOT9_OP",
  "BFD_RELOC_XTENSA_SLOT10_OP",
  "BFD_RELOC_XTENSA_SLOT11_OP",
  "BFD_RELOC_XTENSA_SLOT12_OP",
  "BFD_RELOC_XTENSA_SLOT13_OP",
  "BFD_RELOC_XTENSA_SLOT14_OP",
  "BFD_RELOC_XTENSA_SLOT0_ALT",
  "BFD_RELOC_XTENSA_SLOT1_ALT",
  "BFD_RELOC_XTENSA_SLOT2_ALT",
  "BFD_RELOC_XTENSA_SLOT3_ALT",
  "BFD_RELOC_XTENSA_SLOT4_ALT",
  "BFD_RELOC_XTENSA_SLOT5_ALT",
  "BFD_RELOC_XTENSA_SLOT6_ALT",
  "BFD_RELOC_XTENSA_SLOT7_ALT",
  "BFD_RELOC_XTENSA_SLOT8_ALT",
  "BFD_RELOC_XTENSA_SLOT9_ALT",
  "BFD_RELOC_XTENSA_SLOT10_ALT",
  "BFD_RELOC_XTENSA_SLOT11_ALT",
  "BFD_RELOC_XTENSA_SLOT12_ALT",
  "BFD_RELOC_XTENSA_SLOT13_ALT",
  "BFD_RELOC_XTENSA_SLOT14_ALT",
  "BFD_RELOC_XTENSA_OP0",
  "BFD_RELOC_XTENSA_OP1",
  "BFD_RELOC_XTENSA_OP2",
  "BFD_RELOC_XTENSA_ASM_EXPAND",
  "BFD_RELOC_XTENSA_ASM_SIMPLIFY",
  "BFD_RELOC_XTENSA_TLSDESC_FN",
  "BFD_RELOC_XTENSA_TLSDESC_ARG",
  "BFD_RELOC_XTENSA_TLS_DTPOFF",
  "BFD_RELOC_XTENSA_TLS_TPOFF",
  "BFD_RELOC_XTENSA_TLS_FUNC",
  "BFD_RELOC_XTENSA_TLS_ARG",
  "BFD_RELOC_XTENSA_TLS_CALL",
  "BFD_RELOC_XTENSA_PDIFF8",
  "BFD_RELOC_XTENSA_PDIFF16",
  "BFD_RELOC_XTENSA_PDIFF32",
  "BFD_RELOC_XTENSA_NDIFF8",
  "BFD_RELOC_XTENSA_NDIFF16",
  "BFD_RELOC_XTENSA_NDIFF32",
  "BFD_RELOC_Z80_DISP8",
  "BFD_RELOC_Z80_BYTE0",
  "BFD_RELOC_Z80_BYTE1",
  "BFD_RELOC_Z80_BYTE2",
  "BFD_RELOC_Z80_BYTE3",
  "BFD_RELOC_Z80_WORD0",
  "BFD_RELOC_Z80_WORD1",
  "BFD_RELOC_Z80_16_BE",
  "BFD_RELOC_Z8K_DISP7",
  "BFD_RELOC_Z8K_CALLR",
  "BFD_RELOC_Z8K_IMM4L",
  "BFD_RELOC_LM32_CALL",
  "BFD_RELOC_LM32_BRANCH",
  "BFD_RELOC_LM32_16_GOT",
  "BFD_RELOC_LM32_GOTOFF_HI16",
  "BFD_RELOC_LM32_GOTOFF_LO16",
  "BFD_RELOC_LM32_COPY",
  "BFD_RELOC_LM32_GLOB_DAT",
  "BFD_RELOC_LM32_JMP_SLOT",
  "BFD_RELOC_LM32_RELATIVE",
  "BFD_RELOC_MACH_O_SECTDIFF",
  "BFD_RELOC_MACH_O_LOCAL_SECTDIFF",
  "BFD_RELOC_MACH_O_PAIR",
  "BFD_RELOC_MACH_O_SUBTRACTOR32",
  "BFD_RELOC_MACH_O_SUBTRACTOR64",
  "BFD_RELOC_MACH_O_X86_64_BRANCH32",
  "BFD_RELOC_MACH_O_X86_64_BRANCH8",
  "BFD_RELOC_MACH_O_X86_64_GOT",
  "BFD_RELOC_MACH_O_X86_64_GOT_LOAD",
  "BFD_RELOC_MACH_O_X86_64_PCREL32_1",
  "BFD_RELOC_MACH_O_X86_64_PCREL32_2",
  "BFD_RELOC_MACH_O_X86_64_PCREL32_4",
  "BFD_RELOC_MACH_O_X86_64_TLV",
  "BFD_RELOC_MACH_O_ARM64_ADDEND",
  "BFD_RELOC_MACH_O_ARM64_GOT_LOAD_PAGE21",
  "BFD_RELOC_MACH_O_ARM64_GOT_LOAD_PAGEOFF12",
  "BFD_RELOC_MACH_O_ARM64_POINTER_TO_GOT",
  "BFD_RELOC_MICROBLAZE_32_LO",
  "BFD_RELOC_MICROBLAZE_32_LO_PCREL",
  "BFD_RELOC_MICROBLAZE_32_ROSDA",
  "BFD_RELOC_MICROBLAZE_32_RWSDA",
  "BFD_RELOC_MICROBLAZE_32_SYM_OP_SYM",
  "BFD_RELOC_MICROBLAZE_64_NONE",
  "BFD_RELOC_MICROBLAZE_64_GOTPC",
  "BFD_RELOC_MICROBLAZE_64_GOT",
  "BFD_RELOC_MICROBLAZE_64_PLT",
  "BFD_RELOC_MICROBLAZE_64_GOTOFF",
  "BFD_RELOC_MICROBLAZE_32_GOTOFF",
  "BFD_RELOC_MICROBLAZE_COPY",
  "BFD_RELOC_MICROBLAZE_64_TLS",
  "BFD_RELOC_MICROBLAZE_64_TLSGD",
  "BFD_RELOC_MICROBLAZE_64_TLSLD",
  "BFD_RELOC_MICROBLAZE_32_TLSDTPMOD",
  "BFD_RELOC_MICROBLAZE_32_TLSDTPREL",
  "BFD_RELOC_MICROBLAZE_64_TLSDTPREL",
  "BFD_RELOC_MICROBLAZE_64_TLSGOTTPREL",
  "BFD_RELOC_MICROBLAZE_64_TLSTPREL",
  "BFD_RELOC_MICROBLAZE_64_TEXTPCREL",
  "BFD_RELOC_MICROBLAZE_64_TEXTREL",
  "BFD_RELOC_AARCH64_RELOC_START",
  "BFD_RELOC_AARCH64_NULL",
  "BFD_RELOC_AARCH64_NONE",
  "BFD_RELOC_AARCH64_64",
  "BFD_RELOC_AARCH64_32",
  "BFD_RELOC_AARCH64_16",
  "BFD_RELOC_AARCH64_64_PCREL",
  "BFD_RELOC_AARCH64_32_PCREL",
  "BFD_RELOC_AARCH64_16_PCREL",
  "BFD_RELOC_AARCH64_MOVW_G0",
  "BFD_RELOC_AARCH64_MOVW_G0_NC",
  "BFD_RELOC_AARCH64_MOVW_G1",
  "BFD_RELOC_AARCH64_MOVW_G1_NC",
  "BFD_RELOC_AARCH64_MOVW_G2",
  "BFD_RELOC_AARCH64_MOVW_G2_NC",
  "BFD_RELOC_AARCH64_MOVW_G3",
  "BFD_RELOC_AARCH64_MOVW_G0_S",
  "BFD_RELOC_AARCH64_MOVW_G1_S",
  "BFD_RELOC_AARCH64_MOVW_G2_S",
  "BFD_RELOC_AARCH64_MOVW_PREL_G0",
  "BFD_RELOC_AARCH64_MOVW_PREL_G0_NC",
  "BFD_RELOC_AARCH64_MOVW_PREL_G1",
  "BFD_RELOC_AARCH64_MOVW_PREL_G1_NC",
  "BFD_RELOC_AARCH64_MOVW_PREL_G2",
  "BFD_RELOC_AARCH64_MOVW_PREL_G2_NC",
  "BFD_RELOC_AARCH64_MOVW_PREL_G3",
  "BFD_RELOC_AARCH64_LD_LO19_PCREL",
  "BFD_RELOC_AARCH64_ADR_LO21_PCREL",
  "BFD_RELOC_AARCH64_ADR_HI21_PCREL",
  "BFD_RELOC_AARCH64_ADR_HI21_NC_PCREL",
  "BFD_RELOC_AARCH64_ADD_LO12",
  "BFD_RELOC_AARCH64_LDST8_LO12",
  "BFD_RELOC_AARCH64_TSTBR14",
  "BFD_RELOC_AARCH64_BRANCH19",
  "BFD_RELOC_AARCH64_JUMP26",
  "BFD_RELOC_AARCH64_CALL26",
  "BFD_RELOC_AARCH64_LDST16_LO12",
  "BFD_RELOC_AARCH64_LDST32_LO12",
  "BFD_RELOC_AARCH64_LDST64_LO12",
  "BFD_RELOC_AARCH64_LDST128_LO12",
  "BFD_RELOC_AARCH64_GOT_LD_PREL19",
  "BFD_RELOC_AARCH64_ADR_GOT_PAGE",
  "BFD_RELOC_AARCH64_LD64_GOT_LO12_NC",
  "BFD_RELOC_AARCH64_LD32_GOT_LO12_NC",
  "BFD_RELOC_AARCH64_MOVW_GOTOFF_G0_NC",
  "BFD_RELOC_AARCH64_MOVW_GOTOFF_G1",
  "BFD_RELOC_AARCH64_LD64_GOTOFF_LO15",
  "BFD_RELOC_AARCH64_LD32_GOTPAGE_LO14",
  "BFD_RELOC_AARCH64_LD64_GOTPAGE_LO15",
  "BFD_RELOC_AARCH64_TLSGD_ADR_PAGE21",
  "BFD_RELOC_AARCH64_TLSGD_ADR_PREL21",
  "BFD_RELOC_AARCH64_TLSGD_ADD_LO12_NC",
  "BFD_RELOC_AARCH64_TLSGD_MOVW_G0_NC",
  "BFD_RELOC_AARCH64_TLSGD_MOVW_G1",
  "BFD_RELOC_AARCH64_TLSIE_ADR_GOTTPREL_PAGE21",
  "BFD_RELOC_AARCH64_TLSIE_LD64_GOTTPREL_LO12_NC",
  "BFD_RELOC_AARCH64_TLSIE_LD32_GOTTPREL_LO12_NC",
  "BFD_RELOC_AARCH64_TLSIE_LD_GOTTPREL_PREL19",
  "BFD_RELOC_AARCH64_TLSIE_MOVW_GOTTPREL_G0_NC",
  "BFD_RELOC_AARCH64_TLSIE_MOVW_GOTTPREL_G1",
  "BFD_RELOC_AARCH64_TLSLD_ADD_DTPREL_HI12",
  "BFD_RELOC_AARCH64_TLSLD_ADD_DTPREL_LO12",
  "BFD_RELOC_AARCH64_TLSLD_ADD_DTPREL_LO12_NC",
  "BFD_RELOC_AARCH64_TLSLD_ADD_LO12_NC",
  "BFD_RELOC_AARCH64_TLSLD_ADR_PAGE21",
  "BFD_RELOC_AARCH64_TLSLD_ADR_PREL21",
  "BFD_RELOC_AARCH64_TLSLD_LDST16_DTPREL_LO12",
  "BFD_RELOC_AARCH64_TLSLD_LDST16_DTPREL_LO12_NC",
  "BFD_RELOC_AARCH64_TLSLD_LDST32_DTPREL_LO12",
  "BFD_RELOC_AARCH64_TLSLD_LDST32_DTPREL_LO12_NC",
  "BFD_RELOC_AARCH64_TLSLD_LDST64_DTPREL_LO12",
  "BFD_RELOC_AARCH64_TLSLD_LDST64_DTPREL_LO12_NC",
  "BFD_RELOC_AARCH64_TLSLD_LDST8_DTPREL_LO12",
  "BFD_RELOC_AARCH64_TLSLD_LDST8_DTPREL_LO12_NC",
  "BFD_RELOC_AARCH64_TLSLD_MOVW_DTPREL_G0",
  "BFD_RELOC_AARCH64_TLSLD_MOVW_DTPREL_G0_NC",
  "BFD_RELOC_AARCH64_TLSLD_MOVW_DTPREL_G1",
  "BFD_RELOC_AARCH64_TLSLD_MOVW_DTPREL_G1_NC",
  "BFD_RELOC_AARCH64_TLSLD_MOVW_DTPREL_G2",
  "BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G2",
  "BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G1",
  "BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G1_NC",
  "BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G0",
  "BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G0_NC",
  "BFD_RELOC_AARCH64_TLSLE_ADD_TPREL_HI12",
  "BFD_RELOC_AARCH64_TLSLE_ADD_TPREL_LO12",
  "BFD_RELOC_AARCH64_TLSLE_ADD_TPREL_LO12_NC",
  "BFD_RELOC_AARCH64_TLSLE_LDST16_TPREL_LO12",
  "BFD_RELOC_AARCH64_TLSLE_LDST16_TPREL_LO12_NC",
  "BFD_RELOC_AARCH64_TLSLE_LDST32_TPREL_LO12",
  "BFD_RELOC_AARCH64_TLSLE_LDST32_TPREL_LO12_NC",
  "BFD_RELOC_AARCH64_TLSLE_LDST64_TPREL_LO12",
  "BFD_RELOC_AARCH64_TLSLE_LDST64_TPREL_LO12_NC",
  "BFD_RELOC_AARCH64_TLSLE_LDST8_TPREL_LO12",
  "BFD_RELOC_AARCH64_TLSLE_LDST8_TPREL_LO12_NC",
  "BFD_RELOC_AARCH64_TLSDESC_LD_PREL19",
  "BFD_RELOC_AARCH64_TLSDESC_ADR_PREL21",
  "BFD_RELOC_AARCH64_TLSDESC_ADR_PAGE21",
  "BFD_RELOC_AARCH64_TLSDESC_LD64_LO12",
  "BFD_RELOC_AARCH64_TLSDESC_LD32_LO12_NC",
  "BFD_RELOC_AARCH64_TLSDESC_ADD_LO12",
  "BFD_RELOC_AARCH64_TLSDESC_OFF_G1",
  "BFD_RELOC_AARCH64_TLSDESC_OFF_G0_NC",
  "BFD_RELOC_AARCH64_TLSDESC_LDR",
  "BFD_RELOC_AARCH64_TLSDESC_ADD",
  "BFD_RELOC_AARCH64_TLSDESC_CALL",
  "BFD_RELOC_AARCH64_COPY",
  "BFD_RELOC_AARCH64_GLOB_DAT",
  "BFD_RELOC_AARCH64_JUMP_SLOT",
  "BFD_RELOC_AARCH64_RELATIVE",
  "BFD_RELOC_AARCH64_TLS_DTPMOD",
  "BFD_RELOC_AARCH64_TLS_DTPREL",
  "BFD_RELOC_AARCH64_TLS_TPREL",
  "BFD_RELOC_AARCH64_TLSDESC",
  "BFD_RELOC_AARCH64_IRELATIVE",
  "BFD_RELOC_AARCH64_RELOC_END",
  "BFD_RELOC_AARCH64_GAS_INTERNAL_FIXUP",
  "BFD_RELOC_AARCH64_LDST_LO12",
  "BFD_RELOC_AARCH64_TLSLD_LDST_DTPREL_LO12",
  "BFD_RELOC_AARCH64_TLSLD_LDST_DTPREL_LO12_NC",
  "BFD_RELOC_AARCH64_TLSLE_LDST_TPREL_LO12",
  "BFD_RELOC_AARCH64_TLSLE_LDST_TPREL_LO12_NC",
  "BFD_RELOC_AARCH64_LD_GOT_LO12_NC",
  "BFD_RELOC_AARCH64_TLSIE_LD_GOTTPREL_LO12_NC",
  "BFD_RELOC_AARCH64_TLSDESC_LD_LO12_NC",
  "BFD_RELOC_TILEPRO_COPY",
  "BFD_RELOC_TILEPRO_GLOB_DAT",
  "BFD_RELOC_TILEPRO_JMP_SLOT",
  "BFD_RELOC_TILEPRO_RELATIVE",
  "BFD_RELOC_TILEPRO_BROFF_X1",
  "BFD_RELOC_TILEPRO_JOFFLONG_X1",
  "BFD_RELOC_TILEPRO_JOFFLONG_X1_PLT",
  "BFD_RELOC_TILEPRO_IMM8_X0",
  "BFD_RELOC_TILEPRO_IMM8_Y0",
  "BFD_RELOC_TILEPRO_IMM8_X1",
  "BFD_RELOC_TILEPRO_IMM8_Y1",
  "BFD_RELOC_TILEPRO_DEST_IMM8_X1",
  "BFD_RELOC_TILEPRO_MT_IMM15_X1",
  "BFD_RELOC_TILEPRO_MF_IMM15_X1",
  "BFD_RELOC_TILEPRO_IMM16_X0",
  "BFD_RELOC_TILEPRO_IMM16_X1",
  "BFD_RELOC_TILEPRO_IMM16_X0_LO",
  "BFD_RELOC_TILEPRO_IMM16_X1_LO",
  "BFD_RELOC_TILEPRO_IMM16_X0_HI",
  "BFD_RELOC_TILEPRO_IMM16_X1_HI",
  "BFD_RELOC_TILEPRO_IMM16_X0_HA",
  "BFD_RELOC_TILEPRO_IMM16_X1_HA",
  "BFD_RELOC_TILEPRO_IMM16_X0_PCREL",
  "BFD_RELOC_TILEPRO_IMM16_X1_PCREL",
  "BFD_RELOC_TILEPRO_IMM16_X0_LO_PCREL",
  "BFD_RELOC_TILEPRO_IMM16_X1_LO_PCREL",
  "BFD_RELOC_TILEPRO_IMM16_X0_HI_PCREL",
  "BFD_RELOC_TILEPRO_IMM16_X1_HI_PCREL",
  "BFD_RELOC_TILEPRO_IMM16_X0_HA_PCREL",
  "BFD_RELOC_TILEPRO_IMM16_X1_HA_PCREL",
  "BFD_RELOC_TILEPRO_IMM16_X0_GOT",
  "BFD_RELOC_TILEPRO_IMM16_X1_GOT",
  "BFD_RELOC_TILEPRO_IMM16_X0_GOT_LO",
  "BFD_RELOC_TILEPRO_IMM16_X1_GOT_LO",
  "BFD_RELOC_TILEPRO_IMM16_X0_GOT_HI",
  "BFD_RELOC_TILEPRO_IMM16_X1_GOT_HI",
  "BFD_RELOC_TILEPRO_IMM16_X0_GOT_HA",
  "BFD_RELOC_TILEPRO_IMM16_X1_GOT_HA",
  "BFD_RELOC_TILEPRO_MMSTART_X0",
  "BFD_RELOC_TILEPRO_MMEND_X0",
  "BFD_RELOC_TILEPRO_MMSTART_X1",
  "BFD_RELOC_TILEPRO_MMEND_X1",
  "BFD_RELOC_TILEPRO_SHAMT_X0",
  "BFD_RELOC_TILEPRO_SHAMT_X1",
  "BFD_RELOC_TILEPRO_SHAMT_Y0",
  "BFD_RELOC_TILEPRO_SHAMT_Y1",
  "BFD_RELOC_TILEPRO_TLS_GD_CALL",
  "BFD_RELOC_TILEPRO_IMM8_X0_TLS_GD_ADD",
  "BFD_RELOC_TILEPRO_IMM8_X1_TLS_GD_ADD",
  "BFD_RELOC_TILEPRO_IMM8_Y0_TLS_GD_ADD",
  "BFD_RELOC_TILEPRO_IMM8_Y1_TLS_GD_ADD",
  "BFD_RELOC_TILEPRO_TLS_IE_LOAD",
  "BFD_RELOC_TILEPRO_IMM16_X0_TLS_GD",
  "BFD_RELOC_TILEPRO_IMM16_X1_TLS_GD",
  "BFD_RELOC_TILEPRO_IMM16_X0_TLS_GD_LO",
  "BFD_RELOC_TILEPRO_IMM16_X1_TLS_GD_LO",
  "BFD_RELOC_TILEPRO_IMM16_X0_TLS_GD_HI",
  "BFD_RELOC_TILEPRO_IMM16_X1_TLS_GD_HI",
  "BFD_RELOC_TILEPRO_IMM16_X0_TLS_GD_HA",
  "BFD_RELOC_TILEPRO_IMM16_X1_TLS_GD_HA",
  "BFD_RELOC_TILEPRO_IMM16_X0_TLS_IE",
  "BFD_RELOC_TILEPRO_IMM16_X1_TLS_IE",
  "BFD_RELOC_TILEPRO_IMM16_X0_TLS_IE_LO",
  "BFD_RELOC_TILEPRO_IMM16_X1_TLS_IE_LO",
  "BFD_RELOC_TILEPRO_IMM16_X0_TLS_IE_HI",
  "BFD_RELOC_TILEPRO_IMM16_X1_TLS_IE_HI",
  "BFD_RELOC_TILEPRO_IMM16_X0_TLS_IE_HA",
  "BFD_RELOC_TILEPRO_IMM16_X1_TLS_IE_HA",
  "BFD_RELOC_TILEPRO_TLS_DTPMOD32",
  "BFD_RELOC_TILEPRO_TLS_DTPOFF32",
  "BFD_RELOC_TILEPRO_TLS_TPOFF32",
  "BFD_RELOC_TILEPRO_IMM16_X0_TLS_LE",
  "BFD_RELOC_TILEPRO_IMM16_X1_TLS_LE",
  "BFD_RELOC_TILEPRO_IMM16_X0_TLS_LE_LO",
  "BFD_RELOC_TILEPRO_IMM16_X1_TLS_LE_LO",
  "BFD_RELOC_TILEPRO_IMM16_X0_TLS_LE_HI",
  "BFD_RELOC_TILEPRO_IMM16_X1_TLS_LE_HI",
  "BFD_RELOC_TILEPRO_IMM16_X0_TLS_LE_HA",
  "BFD_RELOC_TILEPRO_IMM16_X1_TLS_LE_HA",
  "BFD_RELOC_TILEGX_HW0",
  "BFD_RELOC_TILEGX_HW1",
  "BFD_RELOC_TILEGX_HW2",
  "BFD_RELOC_TILEGX_HW3",
  "BFD_RELOC_TILEGX_HW0_LAST",
  "BFD_RELOC_TILEGX_HW1_LAST",
  "BFD_RELOC_TILEGX_HW2_LAST",
  "BFD_RELOC_TILEGX_COPY",
  "BFD_RELOC_TILEGX_GLOB_DAT",
  "BFD_RELOC_TILEGX_JMP_SLOT",
  "BFD_RELOC_TILEGX_RELATIVE",
  "BFD_RELOC_TILEGX_BROFF_X1",
  "BFD_RELOC_TILEGX_JUMPOFF_X1",
  "BFD_RELOC_TILEGX_JUMPOFF_X1_PLT",
  "BFD_RELOC_TILEGX_IMM8_X0",
  "BFD_RELOC_TILEGX_IMM8_Y0",
  "BFD_RELOC_TILEGX_IMM8_X1",
  "BFD_RELOC_TILEGX_IMM8_Y1",
  "BFD_RELOC_TILEGX_DEST_IMM8_X1",
  "BFD_RELOC_TILEGX_MT_IMM14_X1",
  "BFD_RELOC_TILEGX_MF_IMM14_X1",
  "BFD_RELOC_TILEGX_MMSTART_X0",
  "BFD_RELOC_TILEGX_MMEND_X0",
  "BFD_RELOC_TILEGX_SHAMT_X0",
  "BFD_RELOC_TILEGX_SHAMT_X1",
  "BFD_RELOC_TILEGX_SHAMT_Y0",
  "BFD_RELOC_TILEGX_SHAMT_Y1",
  "BFD_RELOC_TILEGX_IMM16_X0_HW0",
  "BFD_RELOC_TILEGX_IMM16_X1_HW0",
  "BFD_RELOC_TILEGX_IMM16_X0_HW1",
  "BFD_RELOC_TILEGX_IMM16_X1_HW1",
  "BFD_RELOC_TILEGX_IMM16_X0_HW2",
  "BFD_RELOC_TILEGX_IMM16_X1_HW2",
  "BFD_RELOC_TILEGX_IMM16_X0_HW3",
  "BFD_RELOC_TILEGX_IMM16_X1_HW3",
  "BFD_RELOC_TILEGX_IMM16_X0_HW0_LAST",
  "BFD_RELOC_TILEGX_IMM16_X1_HW0_LAST",
  "BFD_RELOC_TILEGX_IMM16_X0_HW1_LAST",
  "BFD_RELOC_TILEGX_IMM16_X1_HW1_LAST",
  "BFD_RELOC_TILEGX_IMM16_X0_HW2_LAST",
  "BFD_RELOC_TILEGX_IMM16_X1_HW2_LAST",
  "BFD_RELOC_TILEGX_IMM16_X0_HW0_PCREL",
  "BFD_RELOC_TILEGX_IMM16_X1_HW0_PCREL",
  "BFD_RELOC_TILEGX_IMM16_X0_HW1_PCREL",
  "BFD_RELOC_TILEGX_IMM16_X1_HW1_PCREL",
  "BFD_RELOC_TILEGX_IMM16_X0_HW2_PCREL",
  "BFD_RELOC_TILEGX_IMM16_X1_HW2_PCREL",
  "BFD_RELOC_TILEGX_IMM16_X0_HW3_PCREL",
  "BFD_RELOC_TILEGX_IMM16_X1_HW3_PCREL",
  "BFD_RELOC_TILEGX_IMM16_X0_HW0_LAST_PCREL",
  "BFD_RELOC_TILEGX_IMM16_X1_HW0_LAST_PCREL",
  "BFD_RELOC_TILEGX_IMM16_X0_HW1_LAST_PCREL",
  "BFD_RELOC_TILEGX_IMM16_X1_HW1_LAST_PCREL",
  "BFD_RELOC_TILEGX_IMM16_X0_HW2_LAST_PCREL",
  "BFD_RELOC_TILEGX_IMM16_X1_HW2_LAST_PCREL",
  "BFD_RELOC_TILEGX_IMM16_X0_HW0_GOT",
  "BFD_RELOC_TILEGX_IMM16_X1_HW0_GOT",
  "BFD_RELOC_TILEGX_IMM16_X0_HW0_PLT_PCREL",
  "BFD_RELOC_TILEGX_IMM16_X1_HW0_PLT_PCREL",
  "BFD_RELOC_TILEGX_IMM16_X0_HW1_PLT_PCREL",
  "BFD_RELOC_TILEGX_IMM16_X1_HW1_PLT_PCREL",
  "BFD_RELOC_TILEGX_IMM16_X0_HW2_PLT_PCREL",
  "BFD_RELOC_TILEGX_IMM16_X1_HW2_PLT_PCREL",
  "BFD_RELOC_TILEGX_IMM16_X0_HW0_LAST_GOT",
  "BFD_RELOC_TILEGX_IMM16_X1_HW0_LAST_GOT",
  "BFD_RELOC_TILEGX_IMM16_X0_HW1_LAST_GOT",
  "BFD_RELOC_TILEGX_IMM16_X1_HW1_LAST_GOT",
  "BFD_RELOC_TILEGX_IMM16_X0_HW3_PLT_PCREL",
  "BFD_RELOC_TILEGX_IMM16_X1_HW3_PLT_PCREL",
  "BFD_RELOC_TILEGX_IMM16_X0_HW0_TLS_GD",
  "BFD_RELOC_TILEGX_IMM16_X1_HW0_TLS_GD",
  "BFD_RELOC_TILEGX_IMM16_X0_HW0_TLS_LE",
  "BFD_RELOC_TILEGX_IMM16_X1_HW0_TLS_LE",
  "BFD_RELOC_TILEGX_IMM16_X0_HW0_LAST_TLS_LE",
  "BFD_RELOC_TILEGX_IMM16_X1_HW0_LAST_TLS_LE",
  "BFD_RELOC_TILEGX_IMM16_X0_HW1_LAST_TLS_LE",
  "BFD_RELOC_TILEGX_IMM16_X1_HW1_LAST_TLS_LE",
  "BFD_RELOC_TILEGX_IMM16_X0_HW0_LAST_TLS_GD",
  "BFD_RELOC_TILEGX_IMM16_X1_HW0_LAST_TLS_GD",
  "BFD_RELOC_TILEGX_IMM16_X0_HW1_LAST_TLS_GD",
  "BFD_RELOC_TILEGX_IMM16_X1_HW1_LAST_TLS_GD",
  "BFD_RELOC_TILEGX_IMM16_X0_HW0_TLS_IE",
  "BFD_RELOC_TILEGX_IMM16_X1_HW0_TLS_IE",
  "BFD_RELOC_TILEGX_IMM16_X0_HW0_LAST_PLT_PCREL",
  "BFD_RELOC_TILEGX_IMM16_X1_HW0_LAST_PLT_PCREL",
  "BFD_RELOC_TILEGX_IMM16_X0_HW1_LAST_PLT_PCREL",
  "BFD_RELOC_TILEGX_IMM16_X1_HW1_LAST_PLT_PCREL",
  "BFD_RELOC_TILEGX_IMM16_X0_HW2_LAST_PLT_PCREL",
  "BFD_RELOC_TILEGX_IMM16_X1_HW2_LAST_PLT_PCREL",
  "BFD_RELOC_TILEGX_IMM16_X0_HW0_LAST_TLS_IE",
  "BFD_RELOC_TILEGX_IMM16_X1_HW0_LAST_TLS_IE",
  "BFD_RELOC_TILEGX_IMM16_X0_HW1_LAST_TLS_IE",
  "BFD_RELOC_TILEGX_IMM16_X1_HW1_LAST_TLS_IE",
  "BFD_RELOC_TILEGX_TLS_DTPMOD64",
  "BFD_RELOC_TILEGX_TLS_DTPOFF64",
  "BFD_RELOC_TILEGX_TLS_TPOFF64",
  "BFD_RELOC_TILEGX_TLS_DTPMOD32",
  "BFD_RELOC_TILEGX_TLS_DTPOFF32",
  "BFD_RELOC_TILEGX_TLS_TPOFF32",
  "BFD_RELOC_TILEGX_TLS_GD_CALL",
  "BFD_RELOC_TILEGX_IMM8_X0_TLS_GD_ADD",
  "BFD_RELOC_TILEGX_IMM8_X1_TLS_GD_ADD",
  "BFD_RELOC_TILEGX_IMM8_Y0_TLS_GD_ADD",
  "BFD_RELOC_TILEGX_IMM8_Y1_TLS_GD_ADD",
  "BFD_RELOC_TILEGX_TLS_IE_LOAD",
  "BFD_RELOC_TILEGX_IMM8_X0_TLS_ADD",
  "BFD_RELOC_TILEGX_IMM8_X1_TLS_ADD",
  "BFD_RELOC_TILEGX_IMM8_Y0_TLS_ADD",
  "BFD_RELOC_TILEGX_IMM8_Y1_TLS_ADD",
  "BFD_RELOC_BPF_64",
  "BFD_RELOC_BPF_DISP32",
  "BFD_RELOC_EPIPHANY_SIMM8",
  "BFD_RELOC_EPIPHANY_SIMM24",
  "BFD_RELOC_EPIPHANY_HIGH",
  "BFD_RELOC_EPIPHANY_LOW",
  "BFD_RELOC_EPIPHANY_SIMM11",
  "BFD_RELOC_EPIPHANY_IMM11",
  "BFD_RELOC_EPIPHANY_IMM8",
  "BFD_RELOC_VISIUM_HI16",
  "BFD_RELOC_VISIUM_LO16",
  "BFD_RELOC_VISIUM_IM16",
  "BFD_RELOC_VISIUM_REL16",
  "BFD_RELOC_VISIUM_HI16_PCREL",
  "BFD_RELOC_VISIUM_LO16_PCREL",
  "BFD_RELOC_VISIUM_IM16_PCREL",
  "BFD_RELOC_WASM32_LEB128",
  "BFD_RELOC_WASM32_LEB128_GOT",
  "BFD_RELOC_WASM32_LEB128_GOT_CODE",
  "BFD_RELOC_WASM32_LEB128_PLT",
  "BFD_RELOC_WASM32_PLT_INDEX",
  "BFD_RELOC_WASM32_ABS32_CODE",
  "BFD_RELOC_WASM32_COPY",
  "BFD_RELOC_WASM32_CODE_POINTER",
  "BFD_RELOC_WASM32_INDEX",
  "BFD_RELOC_WASM32_PLT_SIG",
  "BFD_RELOC_CKCORE_NONE",
  "BFD_RELOC_CKCORE_ADDR32",
  "BFD_RELOC_CKCORE_PCREL_IMM8BY4",
  "BFD_RELOC_CKCORE_PCREL_IMM11BY2",
  "BFD_RELOC_CKCORE_PCREL_IMM4BY2",
  "BFD_RELOC_CKCORE_PCREL32",
  "BFD_RELOC_CKCORE_PCREL_JSR_IMM11BY2",
  "BFD_RELOC_CKCORE_GNU_VTINHERIT",
  "BFD_RELOC_CKCORE_GNU_VTENTRY",
  "BFD_RELOC_CKCORE_RELATIVE",
  "BFD_RELOC_CKCORE_COPY",
  "BFD_RELOC_CKCORE_GLOB_DAT",
  "BFD_RELOC_CKCORE_JUMP_SLOT",
  "BFD_RELOC_CKCORE_GOTOFF",
  "BFD_RELOC_CKCORE_GOTPC",
  "BFD_RELOC_CKCORE_GOT32",
  "BFD_RELOC_CKCORE_PLT32",
  "BFD_RELOC_CKCORE_ADDRGOT",
  "BFD_RELOC_CKCORE_ADDRPLT",
  "BFD_RELOC_CKCORE_PCREL_IMM26BY2",
  "BFD_RELOC_CKCORE_PCREL_IMM16BY2",
  "BFD_RELOC_CKCORE_PCREL_IMM16BY4",
  "BFD_RELOC_CKCORE_PCREL_IMM10BY2",
  "BFD_RELOC_CKCORE_PCREL_IMM10BY4",
  "BFD_RELOC_CKCORE_ADDR_HI16",
  "BFD_RELOC_CKCORE_ADDR_LO16",
  "BFD_RELOC_CKCORE_GOTPC_HI16",
  "BFD_RELOC_CKCORE_GOTPC_LO16",
  "BFD_RELOC_CKCORE_GOTOFF_HI16",
  "BFD_RELOC_CKCORE_GOTOFF_LO16",
  "BFD_RELOC_CKCORE_GOT12",
  "BFD_RELOC_CKCORE_GOT_HI16",
  "BFD_RELOC_CKCORE_GOT_LO16",
  "BFD_RELOC_CKCORE_PLT12",
  "BFD_RELOC_CKCORE_PLT_HI16",
  "BFD_RELOC_CKCORE_PLT_LO16",
  "BFD_RELOC_CKCORE_ADDRGOT_HI16",
  "BFD_RELOC_CKCORE_ADDRGOT_LO16",
  "BFD_RELOC_CKCORE_ADDRPLT_HI16",
  "BFD_RELOC_CKCORE_ADDRPLT_LO16",
  "BFD_RELOC_CKCORE_PCREL_JSR_IMM26BY2",
  "BFD_RELOC_CKCORE_TOFFSET_LO16",
  "BFD_RELOC_CKCORE_DOFFSET_LO16",
  "BFD_RELOC_CKCORE_PCREL_IMM18BY2",
  "BFD_RELOC_CKCORE_DOFFSET_IMM18",
  "BFD_RELOC_CKCORE_DOFFSET_IMM18BY2",
  "BFD_RELOC_CKCORE_DOFFSET_IMM18BY4",
  "BFD_RELOC_CKCORE_GOTOFF_IMM18",
  "BFD_RELOC_CKCORE_GOT_IMM18BY4",
  "BFD_RELOC_CKCORE_PLT_IMM18BY4",
  "BFD_RELOC_CKCORE_PCREL_IMM7BY4",
  "BFD_RELOC_CKCORE_TLS_LE32",
  "BFD_RELOC_CKCORE_TLS_IE32",
  "BFD_RELOC_CKCORE_TLS_GD32",
  "BFD_RELOC_CKCORE_TLS_LDM32",
  "BFD_RELOC_CKCORE_TLS_LDO32",
  "BFD_RELOC_CKCORE_TLS_DTPMOD32",
  "BFD_RELOC_CKCORE_TLS_DTPOFF32",
  "BFD_RELOC_CKCORE_TLS_TPOFF32",
  "BFD_RELOC_CKCORE_PCREL_FLRW_IMM8BY4",
  "BFD_RELOC_CKCORE_NOJSRI",
  "BFD_RELOC_CKCORE_CALLGRAPH",
  "BFD_RELOC_CKCORE_IRELATIVE",
  "BFD_RELOC_CKCORE_PCREL_BLOOP_IMM4BY4",
  "BFD_RELOC_CKCORE_PCREL_BLOOP_IMM12BY4",
  "BFD_RELOC_S12Z_OPR",
  "BFD_RELOC_LARCH_TLS_DTPMOD32",
  "BFD_RELOC_LARCH_TLS_DTPREL32",
  "BFD_RELOC_LARCH_TLS_DTPMOD64",
  "BFD_RELOC_LARCH_TLS_DTPREL64",
  "BFD_RELOC_LARCH_TLS_TPREL32",
  "BFD_RELOC_LARCH_TLS_TPREL64",
  "BFD_RELOC_LARCH_MARK_LA",
  "BFD_RELOC_LARCH_MARK_PCREL",
  "BFD_RELOC_LARCH_SOP_PUSH_PCREL",
  "BFD_RELOC_LARCH_SOP_PUSH_ABSOLUTE",
  "BFD_RELOC_LARCH_SOP_PUSH_DUP",
  "BFD_RELOC_LARCH_SOP_PUSH_GPREL",
  "BFD_RELOC_LARCH_SOP_PUSH_TLS_TPREL",
  "BFD_RELOC_LARCH_SOP_PUSH_TLS_GOT",
  "BFD_RELOC_LARCH_SOP_PUSH_TLS_GD",
  "BFD_RELOC_LARCH_SOP_PUSH_PLT_PCREL",
  "BFD_RELOC_LARCH_SOP_ASSERT",
  "BFD_RELOC_LARCH_SOP_NOT",
  "BFD_RELOC_LARCH_SOP_SUB",
  "BFD_RELOC_LARCH_SOP_SL",
  "BFD_RELOC_LARCH_SOP_SR",
  "BFD_RELOC_LARCH_SOP_ADD",
  "BFD_RELOC_LARCH_SOP_AND",
  "BFD_RELOC_LARCH_SOP_IF_ELSE",
  "BFD_RELOC_LARCH_SOP_POP_32_S_10_5",
  "BFD_RELOC_LARCH_SOP_POP_32_U_10_12",
  "BFD_RELOC_LARCH_SOP_POP_32_S_10_12",
  "BFD_RELOC_LARCH_SOP_POP_32_S_10_16",
  "BFD_RELOC_LARCH_SOP_POP_32_S_10_16_S2",
  "BFD_RELOC_LARCH_SOP_POP_32_S_5_20",
  "BFD_RELOC_LARCH_SOP_POP_32_S_0_5_10_16_S2",
  "BFD_RELOC_LARCH_SOP_POP_32_S_0_10_10_16_S2",
  "BFD_RELOC_LARCH_SOP_POP_32_U",
  "BFD_RELOC_LARCH_ADD8",
  "BFD_RELOC_LARCH_ADD16",
  "BFD_RELOC_LARCH_ADD24",
  "BFD_RELOC_LARCH_ADD32",
  "BFD_RELOC_LARCH_ADD64",
  "BFD_RELOC_LARCH_SUB8",
  "BFD_RELOC_LARCH_SUB16",
  "BFD_RELOC_LARCH_SUB24",
  "BFD_RELOC_LARCH_SUB32",
  "BFD_RELOC_LARCH_SUB64",
  "BFD_RELOC_LARCH_B16",
  "BFD_RELOC_LARCH_B21",
  "BFD_RELOC_LARCH_B26",
  "BFD_RELOC_LARCH_ABS_HI20",
  "BFD_RELOC_LARCH_ABS_LO12",
  "BFD_RELOC_LARCH_ABS64_LO20",
  "BFD_RELOC_LARCH_ABS64_HI12",
  "BFD_RELOC_LARCH_PCALA_HI20",
  "BFD_RELOC_LARCH_PCALA_LO12",
  "BFD_RELOC_LARCH_PCALA64_LO20",
  "BFD_RELOC_LARCH_PCALA64_HI12",
  "BFD_RELOC_LARCH_GOT_PC_HI20",
  "BFD_RELOC_LARCH_GOT_PC_LO12",
  "BFD_RELOC_LARCH_GOT64_PC_LO20",
  "BFD_RELOC_LARCH_GOT64_PC_HI12",
  "BFD_RELOC_LARCH_GOT_HI20",
  "BFD_RELOC_LARCH_GOT_LO12",
  "BFD_RELOC_LARCH_GOT64_LO20",
  "BFD_RELOC_LARCH_GOT64_HI12",
  "BFD_RELOC_LARCH_TLS_LE_HI20",
  "BFD_RELOC_LARCH_TLS_LE_LO12",
  "BFD_RELOC_LARCH_TLS_LE64_LO20",
  "BFD_RELOC_LARCH_TLS_LE64_HI12",
  "BFD_RELOC_LARCH_TLS_IE_PC_HI20",
  "BFD_RELOC_LARCH_TLS_IE_PC_LO12",
  "BFD_RELOC_LARCH_TLS_IE64_PC_LO20",
  "BFD_RELOC_LARCH_TLS_IE64_PC_HI12",
  "BFD_RELOC_LARCH_TLS_IE_HI20",
  "BFD_RELOC_LARCH_TLS_IE_LO12",
  "BFD_RELOC_LARCH_TLS_IE64_LO20",
  "BFD_RELOC_LARCH_TLS_IE64_HI12",
  "BFD_RELOC_LARCH_TLS_LD_PC_HI20",
  "BFD_RELOC_LARCH_TLS_LD_HI20",
  "BFD_RELOC_LARCH_TLS_GD_PC_HI20",
  "BFD_RELOC_LARCH_TLS_GD_HI20",
  "BFD_RELOC_LARCH_32_PCREL",
  "BFD_RELOC_LARCH_RELAX",
  "BFD_RELOC_LARCH_DELETE",
  "BFD_RELOC_LARCH_ALIGN",
  "BFD_RELOC_LARCH_PCREL20_S2",
  "BFD_RELOC_LARCH_CFA",
  "BFD_RELOC_LARCH_ADD6",
  "BFD_RELOC_LARCH_SUB6",
  "BFD_RELOC_LARCH_ADD_ULEB128",
  "BFD_RELOC_LARCH_SUB_ULEB128",
  "BFD_RELOC_LARCH_64_PCREL",
 "@@overflow: BFD_RELOC_UNUSED@@",
};
#endif

reloc_howto_type *bfd_default_reloc_type_lookup
   (bfd *abfd, bfd_reloc_code_real_type  code) ATTRIBUTE_HIDDEN;

bool bfd_generic_relax_section
   (bfd *abfd,
    asection *section,
    struct bfd_link_info *,
    bool *) ATTRIBUTE_HIDDEN;

bool bfd_generic_gc_sections
   (bfd *, struct bfd_link_info *) ATTRIBUTE_HIDDEN;

bool bfd_generic_lookup_section_flags
   (struct bfd_link_info *, struct flag_info *, asection *) ATTRIBUTE_HIDDEN;

bool bfd_generic_merge_sections
   (bfd *, struct bfd_link_info *) ATTRIBUTE_HIDDEN;

bfd_byte *bfd_generic_get_relocated_section_contents
   (bfd *abfd,
    struct bfd_link_info *link_info,
    struct bfd_link_order *link_order,
    bfd_byte *data,
    bool relocatable,
    asymbol **symbols) ATTRIBUTE_HIDDEN;

void _bfd_generic_set_reloc
   (bfd *abfd,
    sec_ptr section,
    arelent **relptr,
    unsigned int count) ATTRIBUTE_HIDDEN;

bool _bfd_unrecognized_reloc
   (bfd * abfd,
    sec_ptr section,
    unsigned int r_type) ATTRIBUTE_HIDDEN;

/* Extracted from section.c.  */
bool _bfd_section_size_insane (bfd *abfd, asection *sec) ATTRIBUTE_HIDDEN;

/* Extracted from stabs.c.  */
bool _bfd_link_section_stabs
   (bfd *, struct stab_info *, asection *, asection *, void **,
    bfd_size_type *) ATTRIBUTE_HIDDEN;

bool _bfd_discard_section_stabs
   (bfd *, asection *, void *, bool (*) (bfd_vma, void *), void *) ATTRIBUTE_HIDDEN;

bool _bfd_write_section_stabs
   (bfd *, struct stab_info *, asection *, void **, bfd_byte *) ATTRIBUTE_HIDDEN;

bool _bfd_write_stab_strings (bfd *, struct stab_info *) ATTRIBUTE_HIDDEN;

bfd_vma _bfd_stab_section_offset (asection *, void *, bfd_vma) ATTRIBUTE_HIDDEN;

/* Extracted from targets.c.  */
/* Cached _bfd_check_format messages are put in this.  */
struct per_xvec_message
{
  struct per_xvec_message *next;
  char message[];
};

struct per_xvec_message **_bfd_per_xvec_warn (const bfd_target *, size_t) ATTRIBUTE_HIDDEN;

#ifdef __cplusplus
}
#endif
#endif
