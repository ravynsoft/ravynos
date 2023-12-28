/* SPARC-specific support for 32-bit ELF
   Copyright (C) 1993-2023 Free Software Foundation, Inc.

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
#include "bfdlink.h"
#include "libbfd.h"
#include "elf-bfd.h"
#include "elf/sparc.h"
#include "opcode/sparc.h"
#include "elfxx-sparc.h"
#include "elf-vxworks.h"

/* Support for core dump NOTE sections.  */

static bool
elf32_sparc_grok_psinfo (bfd *abfd, Elf_Internal_Note *note)
{
  switch (note->descsz)
    {
    default:
      return false;

    case 260:			/* Solaris prpsinfo_t.  */
      elf_tdata (abfd)->core->program
	= _bfd_elfcore_strndup (abfd, note->descdata + 84, 16);
      elf_tdata (abfd)->core->command
	= _bfd_elfcore_strndup (abfd, note->descdata + 100, 80);
      break;

    case 336:			/* Solaris psinfo_t.  */
      elf_tdata (abfd)->core->program
	= _bfd_elfcore_strndup (abfd, note->descdata + 88, 16);
      elf_tdata (abfd)->core->command
	= _bfd_elfcore_strndup (abfd, note->descdata + 104, 80);
      break;
    }

  return true;
}

/* Functions for dealing with the e_flags field.

   We don't define set_private_flags or copy_private_bfd_data because
   the only currently defined values are based on the bfd mach number,
   so we use the latter instead and defer setting e_flags until the
   file is written out.  */

/* Merge backend specific data from an object file to the output
   object file when linking.  */

static bool
elf32_sparc_merge_private_bfd_data (bfd *ibfd, struct bfd_link_info *info)
{
  bfd *obfd = info->output_bfd;
  bool error;
  unsigned long ibfd_mach;
  /* FIXME: This should not be static.  */
  static unsigned long previous_ibfd_e_flags = (unsigned long) -1;

  if (bfd_get_flavour (ibfd) != bfd_target_elf_flavour
      || bfd_get_flavour (obfd) != bfd_target_elf_flavour)
    return true;

  error = false;

  ibfd_mach = bfd_get_mach (ibfd);
  if (bfd_mach_sparc_64bit_p (ibfd_mach))
    {
      error = true;
      _bfd_error_handler
	(_("%pB: compiled for a 64 bit system and target is 32 bit"), ibfd);
    }
  else if ((ibfd->flags & DYNAMIC) == 0)
    {
      if (bfd_get_mach (obfd) < ibfd_mach)
	bfd_set_arch_mach (obfd, bfd_arch_sparc, ibfd_mach);
    }

  if (((elf_elfheader (ibfd)->e_flags & EF_SPARC_LEDATA)
       != previous_ibfd_e_flags)
      && previous_ibfd_e_flags != (unsigned long) -1)
    {
      _bfd_error_handler
	(_("%pB: linking little endian files with big endian files"), ibfd);
      error = true;
    }
  previous_ibfd_e_flags = elf_elfheader (ibfd)->e_flags & EF_SPARC_LEDATA;

  if (error)
    {
      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  return _bfd_sparc_elf_merge_private_bfd_data (ibfd, info);
}

/* The final processing done just before writing out the object file.
   We need to set the e_machine field appropriately.  */

static void
sparc_final_write_processing (bfd *abfd)
{
  switch (bfd_get_mach (abfd))
    {
    case bfd_mach_sparc:
    case bfd_mach_sparc_sparclet:
    case bfd_mach_sparc_sparclite:
      break; /* nothing to do */
    case bfd_mach_sparc_v8plus:
      elf_elfheader (abfd)->e_machine = EM_SPARC32PLUS;
      elf_elfheader (abfd)->e_flags &=~ EF_SPARC_32PLUS_MASK;
      elf_elfheader (abfd)->e_flags |= EF_SPARC_32PLUS;
      break;
    case bfd_mach_sparc_v8plusa:
      elf_elfheader (abfd)->e_machine = EM_SPARC32PLUS;
      elf_elfheader (abfd)->e_flags &=~ EF_SPARC_32PLUS_MASK;
      elf_elfheader (abfd)->e_flags |= EF_SPARC_32PLUS | EF_SPARC_SUN_US1;
      break;
    case bfd_mach_sparc_v8plusb:
    case bfd_mach_sparc_v8plusc:
    case bfd_mach_sparc_v8plusd:
    case bfd_mach_sparc_v8pluse:
    case bfd_mach_sparc_v8plusv:
    case bfd_mach_sparc_v8plusm:
    case bfd_mach_sparc_v8plusm8:
      elf_elfheader (abfd)->e_machine = EM_SPARC32PLUS;
      elf_elfheader (abfd)->e_flags &=~ EF_SPARC_32PLUS_MASK;
      elf_elfheader (abfd)->e_flags |= EF_SPARC_32PLUS | EF_SPARC_SUN_US1
				       | EF_SPARC_SUN_US3;
      break;
    case bfd_mach_sparc_sparclite_le:
      elf_elfheader (abfd)->e_flags |= EF_SPARC_LEDATA;
      break;
    case 0: /* A non-sparc architecture - ignore.  */
      break;
    default:
      _bfd_error_handler
	(_("%pB: unhandled sparc machine value '%lu' detected during write processing"),
	 abfd, (long) bfd_get_mach (abfd));
      break;
    }
}

static bool
elf32_sparc_final_write_processing (bfd *abfd)
{
  sparc_final_write_processing (abfd);
  return _bfd_elf_final_write_processing (abfd);
}

/* Used to decide how to sort relocs in an optimal manner for the
   dynamic linker, before writing them out.  */

static enum elf_reloc_type_class
elf32_sparc_reloc_type_class (const struct bfd_link_info *info,
			      const asection *rel_sec ATTRIBUTE_UNUSED,
			      const Elf_Internal_Rela *rela)
{
  bfd *abfd = info->output_bfd;
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  struct _bfd_sparc_elf_link_hash_table *htab
    = _bfd_sparc_elf_hash_table (info);
  BFD_ASSERT (htab != NULL);

  if (htab->elf.dynsym != NULL
      && htab->elf.dynsym->contents != NULL)
    {
      /* Check relocation against STT_GNU_IFUNC symbol if there are
	 dynamic symbols.  */
      unsigned long r_symndx = htab->r_symndx (rela->r_info);
      if (r_symndx != STN_UNDEF)
	{
	  Elf_Internal_Sym sym;
	  if (!bed->s->swap_symbol_in (abfd,
				       (htab->elf.dynsym->contents
					+ r_symndx * bed->s->sizeof_sym),
				       0, &sym))
	    abort ();

	  if (ELF_ST_TYPE (sym.st_info) == STT_GNU_IFUNC)
	    return reloc_class_ifunc;
	}
    }

  switch ((int) ELF32_R_TYPE (rela->r_info))
    {
    case R_SPARC_IRELATIVE:
      return reloc_class_ifunc;
    case R_SPARC_RELATIVE:
      return reloc_class_relative;
    case R_SPARC_JMP_SLOT:
      return reloc_class_plt;
    case R_SPARC_COPY:
      return reloc_class_copy;
    default:
      return reloc_class_normal;
    }
}

#define TARGET_BIG_SYM	sparc_elf32_vec
#define TARGET_BIG_NAME	"elf32-sparc"
#define ELF_ARCH	bfd_arch_sparc
#define ELF_TARGET_ID	SPARC_ELF_DATA
#define ELF_MACHINE_CODE EM_SPARC
#define ELF_MACHINE_ALT1 EM_SPARC32PLUS
#define ELF_MAXPAGESIZE 0x10000
#define ELF_COMMONPAGESIZE 0x2000

#define bfd_elf32_bfd_merge_private_bfd_data \
					elf32_sparc_merge_private_bfd_data
#define elf_backend_final_write_processing \
					elf32_sparc_final_write_processing
#define elf_backend_grok_psinfo		elf32_sparc_grok_psinfo
#define elf_backend_reloc_type_class	elf32_sparc_reloc_type_class

#define elf_info_to_howto		_bfd_sparc_elf_info_to_howto
#define bfd_elf32_bfd_reloc_type_lookup	_bfd_sparc_elf_reloc_type_lookup
#define bfd_elf32_bfd_reloc_name_lookup \
  _bfd_sparc_elf_reloc_name_lookup
#define bfd_elf32_bfd_link_hash_table_create \
					_bfd_sparc_elf_link_hash_table_create
#define bfd_elf32_bfd_relax_section	_bfd_sparc_elf_relax_section
#define bfd_elf32_new_section_hook	_bfd_sparc_elf_new_section_hook
#define elf_backend_copy_indirect_symbol \
					_bfd_sparc_elf_copy_indirect_symbol
#define elf_backend_create_dynamic_sections \
					_bfd_sparc_elf_create_dynamic_sections
#define elf_backend_check_relocs	_bfd_sparc_elf_check_relocs
#define elf_backend_adjust_dynamic_symbol \
					_bfd_sparc_elf_adjust_dynamic_symbol
#define elf_backend_omit_section_dynsym	_bfd_sparc_elf_omit_section_dynsym
#define elf_backend_size_dynamic_sections \
					_bfd_sparc_elf_size_dynamic_sections
#define elf_backend_relocate_section	_bfd_sparc_elf_relocate_section
#define elf_backend_finish_dynamic_symbol \
					_bfd_sparc_elf_finish_dynamic_symbol
#define elf_backend_finish_dynamic_sections \
					_bfd_sparc_elf_finish_dynamic_sections
#define bfd_elf32_mkobject		_bfd_sparc_elf_mkobject
#define elf_backend_object_p		_bfd_sparc_elf_object_p
#define elf_backend_gc_mark_hook	_bfd_sparc_elf_gc_mark_hook
#define elf_backend_plt_sym_val		_bfd_sparc_elf_plt_sym_val
#define elf_backend_init_index_section	_bfd_elf_init_1_index_section
#define elf_backend_fixup_symbol	_bfd_sparc_elf_fixup_symbol

#define elf_backend_can_gc_sections 1
#define elf_backend_can_refcount 1
#define elf_backend_want_got_plt 0
#define elf_backend_plt_readonly 0
#define elf_backend_want_plt_sym 1
#define elf_backend_got_header_size 4
#define elf_backend_want_dynrelro 1
#define elf_backend_rela_normal 1

#define elf_backend_linux_prpsinfo32_ugid16	true

#include "elf32-target.h"

/* Solaris 2.  */

#undef	TARGET_BIG_SYM
#define	TARGET_BIG_SYM				sparc_elf32_sol2_vec
#undef	TARGET_BIG_NAME
#define	TARGET_BIG_NAME				"elf32-sparc-sol2"

#undef elf32_bed
#define elf32_bed				elf32_sparc_sol2_bed

/* The 32-bit static TLS arena size is rounded to the nearest 8-byte
   boundary.  */
#undef	elf_backend_static_tls_alignment
#define elf_backend_static_tls_alignment	8

#undef	elf_backend_strtab_flags
#define elf_backend_strtab_flags	SHF_STRINGS

static bool
elf32_sparc_copy_solaris_special_section_fields (const bfd *ibfd ATTRIBUTE_UNUSED,
						 bfd *obfd ATTRIBUTE_UNUSED,
						 const Elf_Internal_Shdr *isection ATTRIBUTE_UNUSED,
						 Elf_Internal_Shdr *osection ATTRIBUTE_UNUSED)
{
  /* PR 19938: FIXME: Need to add code for setting the sh_info
     and sh_link fields of Solaris specific section types.  */
  return false;
}

#undef  elf_backend_copy_special_section_fields
#define elf_backend_copy_special_section_fields elf32_sparc_copy_solaris_special_section_fields

#include "elf32-target.h"

/* A final_write_processing hook that does both the SPARC- and VxWorks-
   specific handling.  */

static bool
elf32_sparc_vxworks_final_write_processing (bfd *abfd)
{
  sparc_final_write_processing (abfd);
  return elf_vxworks_final_write_processing (abfd);
}

#undef  TARGET_BIG_SYM
#define TARGET_BIG_SYM	sparc_elf32_vxworks_vec
#undef  TARGET_BIG_NAME
#define TARGET_BIG_NAME	"elf32-sparc-vxworks"

#undef  ELF_MINPAGESIZE
#define ELF_MINPAGESIZE	0x1000

#undef	ELF_TARGET_OS
#define	ELF_TARGET_OS	is_vxworks

#undef  elf_backend_want_got_plt
#define elf_backend_want_got_plt		1
#undef  elf_backend_plt_readonly
#define elf_backend_plt_readonly		1
#undef  elf_backend_got_header_size
#define elf_backend_got_header_size		12
#undef  elf_backend_dtrel_excludes_plt
#define elf_backend_dtrel_excludes_plt		1
#undef  elf_backend_add_symbol_hook
#define elf_backend_add_symbol_hook \
  elf_vxworks_add_symbol_hook
#undef  elf_backend_link_output_symbol_hook
#define elf_backend_link_output_symbol_hook \
  elf_vxworks_link_output_symbol_hook
#undef  elf_backend_emit_relocs
#define elf_backend_emit_relocs \
  elf_vxworks_emit_relocs
#undef  elf_backend_final_write_processing
#define elf_backend_final_write_processing \
  elf32_sparc_vxworks_final_write_processing
#undef  elf_backend_static_tls_alignment
#undef  elf_backend_strtab_flags
#undef  elf_backend_copy_special_section_fields

#undef  elf32_bed
#define elf32_bed				sparc_elf_vxworks_bed

#include "elf32-target.h"
