/* BFD back-end for MS-DOS executables.
   Copyright (C) 1990-2023 Free Software Foundation, Inc.
   Written by Bryan Ford of the University of Utah.

   Contributed by the Center for Software Science at the
   University of Utah (pa-gdb-bugs@cs.utah.edu).

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


#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"
#include "libaout.h"
#include "coff/msdos.h"

#define EXE_LOAD_HIGH	0x0000
#define EXE_LOAD_LOW	0xffff
#define EXE_PAGE_SIZE	512

static bool
msdos_mkobject (bfd *abfd)
{
  bfd_default_set_arch_mach (abfd, bfd_arch_i386, bfd_mach_i386_i8086);

  return aout_32_mkobject (abfd);
}

static bfd_cleanup
msdos_object_p (bfd *abfd)
{
  struct external_DOS_hdr hdr;
  bfd_byte buffer[2];
  asection *section;
  bfd_size_type size;

  if (bfd_seek (abfd, (file_ptr) 0, SEEK_SET) != 0
      || (size = bfd_bread (&hdr, sizeof (hdr), abfd)) + 1 < DOS_HDR_SIZE + 1)
    {
      if (bfd_get_error () != bfd_error_system_call)
	bfd_set_error (bfd_error_wrong_format);
      return NULL;
    }

  if (H_GET_16 (abfd, hdr.e_magic) != IMAGE_DOS_SIGNATURE)
    {
      bfd_set_error (bfd_error_wrong_format);
      return NULL;
    }

  /* Check that this isn't actually a PE, NE, or LE file. If it is, the
     e_lfanew field will be valid and point to a header beginning with one of
     the relevant signatures.  If not, e_lfanew might point to anything, so
     don't bail if we can't read there.  */
  if (size < offsetof (struct external_DOS_hdr, e_lfanew) + 4
      || H_GET_16 (abfd, hdr.e_cparhdr) < 4)
    ;
  else if (bfd_seek (abfd, H_GET_32 (abfd, hdr.e_lfanew), SEEK_SET) != 0
	   || bfd_bread (buffer, (bfd_size_type) 2, abfd) != 2)
    {
      if (bfd_get_error () == bfd_error_system_call)
	return NULL;
    }
  else
    {
      if (H_GET_16 (abfd, buffer) == IMAGE_NT_SIGNATURE
	  || H_GET_16 (abfd, buffer) == IMAGE_OS2_SIGNATURE
	  || H_GET_16 (abfd, buffer) == IMAGE_OS2_SIGNATURE_LE
	  || H_GET_16 (abfd, buffer) == IMAGE_OS2_SIGNATURE_LX)
	{
	  bfd_set_error (bfd_error_wrong_format);
	  return NULL;
	}
    }

  if (!msdos_mkobject (abfd))
    return NULL;

  abfd->flags = EXEC_P;
  abfd->start_address = H_GET_16 (abfd, hdr.e_ip);

  section = bfd_make_section (abfd, ".text");
  if (section == NULL)
    return NULL;

  section->flags = (SEC_ALLOC | SEC_LOAD | SEC_CODE | SEC_HAS_CONTENTS);
  section->filepos = H_GET_16 (abfd, hdr.e_cparhdr) * 16;
  size = (H_GET_16 (abfd, hdr.e_cp) - 1) * EXE_PAGE_SIZE - section->filepos;
  size += H_GET_16 (abfd, hdr.e_cblp);

  /* Check that the size is valid.  */
  if (bfd_seek (abfd, section->filepos + size, SEEK_SET) != 0)
    {
      if (bfd_get_error () != bfd_error_system_call)
	bfd_set_error (bfd_error_wrong_format);
      return NULL;
    }

  bfd_set_section_size (section, size);
  section->alignment_power = 4;

  return _bfd_no_cleanup;
}

static int
msdos_sizeof_headers (bfd *abfd ATTRIBUTE_UNUSED,
		      struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  return 0;
}

static bool
msdos_write_object_contents (bfd *abfd)
{
  static char hdr[EXE_PAGE_SIZE];
  file_ptr outfile_size = sizeof(hdr);
  bfd_vma high_vma = 0;
  asection *sec;

  /* Find the total size of the program on disk and in memory.  */
  for (sec = abfd->sections; sec != (asection *) NULL; sec = sec->next)
    {
      if (sec->size == 0)
	continue;
      if (bfd_section_flags (sec) & SEC_ALLOC)
	{
	  bfd_vma sec_vma = bfd_section_vma (sec) + sec->size;
	  if (sec_vma > high_vma)
	    high_vma = sec_vma;
	}
      if (bfd_section_flags (sec) & SEC_LOAD)
	{
	  file_ptr sec_end = (sizeof (hdr)
			      + bfd_section_vma (sec)
			      + sec->size);
	  if (sec_end > outfile_size)
	    outfile_size = sec_end;
	}
    }

  /* Make sure the program isn't too big.  */
  if (high_vma > (bfd_vma)0xffff)
    {
      bfd_set_error(bfd_error_file_too_big);
      return false;
    }

  /* Constants.  */
  H_PUT_16 (abfd, IMAGE_DOS_SIGNATURE, &hdr[0]);
  H_PUT_16 (abfd, EXE_PAGE_SIZE / 16, &hdr[8]);
  H_PUT_16 (abfd, EXE_LOAD_LOW, &hdr[12]);
  H_PUT_16 (abfd, 0x3e, &hdr[24]);
  H_PUT_16 (abfd, 0x0001, &hdr[28]); /* XXX??? */
  H_PUT_16 (abfd, 0x30fb, &hdr[30]); /* XXX??? */
  H_PUT_16 (abfd, 0x726a, &hdr[32]); /* XXX??? */

  /* Bytes in last page (0 = full page).  */
  H_PUT_16 (abfd, outfile_size & (EXE_PAGE_SIZE - 1), &hdr[2]);

  /* Number of pages.  */
  H_PUT_16 (abfd, (outfile_size + EXE_PAGE_SIZE - 1) / EXE_PAGE_SIZE, &hdr[4]);

  /* Set the initial stack pointer to the end of the bss.
     The program's crt0 code must relocate it to a real stack.  */
  H_PUT_16 (abfd, high_vma, &hdr[16]);

  if (bfd_seek (abfd, (file_ptr) 0, SEEK_SET) != 0
      || bfd_bwrite (hdr, (bfd_size_type) sizeof(hdr), abfd) != sizeof(hdr))
    return false;

  return true;
}

static bool
msdos_set_section_contents (bfd *abfd,
			    sec_ptr section,
			    const void *location,
			    file_ptr offset,
			    bfd_size_type count)
{

  if (count == 0)
    return true;

  section->filepos = EXE_PAGE_SIZE + bfd_section_vma (section);

  if (bfd_section_flags (section) & SEC_LOAD)
    {
      if (bfd_seek (abfd, section->filepos + offset, SEEK_SET) != 0
	  || bfd_bwrite (location, count, abfd) != count)
	return false;
    }

  return true;
}



#define msdos_make_empty_symbol aout_32_make_empty_symbol
#define msdos_bfd_reloc_type_lookup aout_32_reloc_type_lookup
#define msdos_bfd_reloc_name_lookup aout_32_reloc_name_lookup

#define	msdos_close_and_cleanup _bfd_generic_close_and_cleanup
#define msdos_bfd_free_cached_info _bfd_generic_bfd_free_cached_info
#define msdos_new_section_hook _bfd_generic_new_section_hook
#define msdos_get_section_contents _bfd_generic_get_section_contents
#define msdos_get_section_contents_in_window \
  _bfd_generic_get_section_contents_in_window
#define msdos_bfd_get_relocated_section_contents \
  bfd_generic_get_relocated_section_contents
#define msdos_bfd_relax_section bfd_generic_relax_section
#define msdos_bfd_gc_sections bfd_generic_gc_sections
#define msdos_bfd_lookup_section_flags bfd_generic_lookup_section_flags
#define msdos_bfd_merge_sections bfd_generic_merge_sections
#define msdos_bfd_is_group_section bfd_generic_is_group_section
#define msdos_bfd_group_name bfd_generic_group_name
#define msdos_bfd_discard_group bfd_generic_discard_group
#define msdos_section_already_linked \
  _bfd_generic_section_already_linked
#define msdos_bfd_define_common_symbol bfd_generic_define_common_symbol
#define msdos_bfd_link_hide_symbol _bfd_generic_link_hide_symbol
#define msdos_bfd_define_start_stop bfd_generic_define_start_stop
#define msdos_bfd_link_hash_table_create _bfd_generic_link_hash_table_create
#define msdos_bfd_link_add_symbols _bfd_generic_link_add_symbols
#define msdos_bfd_link_just_syms _bfd_generic_link_just_syms
#define msdos_bfd_copy_link_hash_symbol_type \
  _bfd_generic_copy_link_hash_symbol_type
#define msdos_bfd_final_link _bfd_generic_final_link
#define msdos_bfd_link_split_section _bfd_generic_link_split_section
#define msdos_set_arch_mach _bfd_generic_set_arch_mach
#define msdos_bfd_link_check_relocs _bfd_generic_link_check_relocs

#define msdos_get_symtab_upper_bound _bfd_nosymbols_get_symtab_upper_bound
#define msdos_canonicalize_symtab _bfd_nosymbols_canonicalize_symtab
#define msdos_print_symbol _bfd_nosymbols_print_symbol
#define msdos_get_symbol_info _bfd_nosymbols_get_symbol_info
#define msdos_get_symbol_version_string \
  _bfd_nosymbols_get_symbol_version_string
#define msdos_find_nearest_line _bfd_nosymbols_find_nearest_line
#define msdos_find_nearest_line_with_alt _bfd_nosymbols_find_nearest_line_with_alt
#define msdos_find_line _bfd_nosymbols_find_line
#define msdos_find_inliner_info _bfd_nosymbols_find_inliner_info
#define msdos_get_lineno _bfd_nosymbols_get_lineno
#define msdos_bfd_is_target_special_symbol _bfd_bool_bfd_asymbol_false
#define msdos_bfd_is_local_label_name _bfd_nosymbols_bfd_is_local_label_name
#define msdos_bfd_make_debug_symbol _bfd_nosymbols_bfd_make_debug_symbol
#define msdos_read_minisymbols _bfd_nosymbols_read_minisymbols
#define msdos_minisymbol_to_symbol _bfd_nosymbols_minisymbol_to_symbol

#define msdos_canonicalize_reloc _bfd_norelocs_canonicalize_reloc
#define msdos_set_reloc _bfd_norelocs_set_reloc
#define msdos_get_reloc_upper_bound _bfd_norelocs_get_reloc_upper_bound
#define msdos_32_bfd_link_split_section  _bfd_generic_link_split_section

const bfd_target i386_msdos_vec =
  {
    "msdos",			/* name */
    bfd_target_msdos_flavour,
    BFD_ENDIAN_LITTLE,		/* target byte order */
    BFD_ENDIAN_LITTLE,		/* target headers byte order */
    (EXEC_P),			/* object flags */
    (SEC_CODE | SEC_DATA | SEC_HAS_CONTENTS
     | SEC_ALLOC | SEC_LOAD),	/* section flags */
    0,				/* leading underscore */
    ' ',				/* ar_pad_char */
    16,				/* ar_max_namelen */
    0,				/* match priority.  */
    TARGET_KEEP_UNUSED_SECTION_SYMBOLS, /* keep unused section symbols.  */
    bfd_getl64, bfd_getl_signed_64, bfd_putl64,
    bfd_getl32, bfd_getl_signed_32, bfd_putl32,
    bfd_getl16, bfd_getl_signed_16, bfd_putl16,	/* data */
    bfd_getl64, bfd_getl_signed_64, bfd_putl64,
    bfd_getl32, bfd_getl_signed_32, bfd_putl32,
    bfd_getl16, bfd_getl_signed_16, bfd_putl16,	/* hdrs */

    {
      _bfd_dummy_target,
      msdos_object_p,		/* bfd_check_format */
      _bfd_dummy_target,
      _bfd_dummy_target,
    },
    {
      _bfd_bool_bfd_false_error,
      msdos_mkobject,
      _bfd_generic_mkarchive,
      _bfd_bool_bfd_false_error,
    },
    {				/* bfd_write_contents */
      _bfd_bool_bfd_false_error,
      msdos_write_object_contents,
      _bfd_write_archive_contents,
      _bfd_bool_bfd_false_error,
    },

    BFD_JUMP_TABLE_GENERIC (msdos),
    BFD_JUMP_TABLE_COPY (_bfd_generic),
    BFD_JUMP_TABLE_CORE (_bfd_nocore),
    BFD_JUMP_TABLE_ARCHIVE (_bfd_noarchive),
    BFD_JUMP_TABLE_SYMBOLS (msdos),
    BFD_JUMP_TABLE_RELOCS (msdos),
    BFD_JUMP_TABLE_WRITE (msdos),
    BFD_JUMP_TABLE_LINK (msdos),
    BFD_JUMP_TABLE_DYNAMIC (_bfd_nodynamic),

    NULL,

    NULL
  };


