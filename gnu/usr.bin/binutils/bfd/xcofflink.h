/* POWER/PowerPC XCOFF linker support.
   Copyright (C) 2019-2023 Free Software Foundation, Inc.

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

/* Used to pass info between ld and bfd.  */
struct bfd_xcoff_link_params
{
  /* Linker stub bfd.  */
  bfd *stub_bfd;

  /* Linker call-backs.  */
  asection * (*add_stub_section) (const char *, asection *);
  void (*layout_sections_again) (void);
};

extern bool bfd_xcoff_split_import_path
  (bfd *, const char *, const char **, const char **);
extern bool bfd_xcoff_set_archive_import_path
  (struct bfd_link_info *, bfd *, const char *);
extern bool bfd_xcoff_link_record_set
  (bfd *, struct bfd_link_info *, struct bfd_link_hash_entry *, bfd_size_type);
extern bool bfd_xcoff_import_symbol
  (bfd *, struct bfd_link_info *, struct bfd_link_hash_entry *, bfd_vma,
   const char *, const char *, const char *, unsigned int);
extern bool bfd_xcoff_export_symbol
  (bfd *, struct bfd_link_info *, struct bfd_link_hash_entry *);
extern bool bfd_xcoff_link_count_reloc
  (bfd *, struct bfd_link_info *, const char *);
extern bool bfd_xcoff_record_link_assignment
  (bfd *, struct bfd_link_info *, const char *);
extern bool bfd_xcoff_size_dynamic_sections
  (bfd *, struct bfd_link_info *, const char *, const char *,
   unsigned long, unsigned long, unsigned long, bool,
   int, bool, unsigned int, struct bfd_section **, bool);
extern bool bfd_xcoff_build_dynamic_sections
  (bfd *, struct bfd_link_info *);
extern bool bfd_xcoff_link_generate_rtinit
  (bfd *, const char *, const char *, bool);
extern bool bfd_xcoff_link_init
  (struct bfd_link_info *, struct bfd_xcoff_link_params *);
extern bool bfd_xcoff_size_stubs
  (struct bfd_link_info *info);
extern bool bfd_xcoff_build_stubs
  (struct bfd_link_info *info);
