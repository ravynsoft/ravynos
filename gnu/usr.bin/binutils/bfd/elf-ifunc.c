/* ELF STT_GNU_IFUNC support.
   Copyright (C) 2009-2023 Free Software Foundation, Inc.

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
#define ARCH_SIZE 0
#include "elf-bfd.h"
#include "safe-ctype.h"
#include "libiberty.h"
#include "objalloc.h"

/* Create sections needed by STT_GNU_IFUNC symbol.  */

bool
_bfd_elf_create_ifunc_sections (bfd *abfd, struct bfd_link_info *info)
{
  flagword flags, pltflags;
  asection *s;
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  struct elf_link_hash_table *htab = elf_hash_table (info);

  if (htab->irelifunc != NULL || htab->iplt != NULL)
    return true;

  flags = bed->dynamic_sec_flags;
  pltflags = flags;
  if (bed->plt_not_loaded)
    /* We do not clear SEC_ALLOC here because we still want the OS to
       allocate space for the section; it's just that there's nothing
       to read in from the object file.  */
    pltflags &= ~ (SEC_CODE | SEC_LOAD | SEC_HAS_CONTENTS);
  else
    pltflags |= SEC_ALLOC | SEC_CODE | SEC_LOAD;
  if (bed->plt_readonly)
    pltflags |= SEC_READONLY;

  if (bfd_link_pic (info))
    {
      /* We need to create .rel[a].ifunc for PIC objects.  */
      const char *rel_sec = (bed->rela_plts_and_copies_p
			     ? ".rela.ifunc" : ".rel.ifunc");

      s = bfd_make_section_with_flags (abfd, rel_sec,
				       flags | SEC_READONLY);
      if (s == NULL
	  || !bfd_set_section_alignment (s, bed->s->log_file_align))
	return false;
      htab->irelifunc = s;
    }
  else
    {
      /* We need to create .iplt, .rel[a].iplt, .igot and .igot.plt
	 for static executables.   */
      s = bfd_make_section_with_flags (abfd, ".iplt", pltflags);
      if (s == NULL
	  || !bfd_set_section_alignment (s, bed->plt_alignment))
	return false;
      htab->iplt = s;

      s = bfd_make_section_with_flags (abfd,
				       (bed->rela_plts_and_copies_p
					? ".rela.iplt" : ".rel.iplt"),
				       flags | SEC_READONLY);
      if (s == NULL
	  || !bfd_set_section_alignment (s, bed->s->log_file_align))
	return false;
      htab->irelplt = s;

      /* We don't need the .igot section if we have the .igot.plt
	 section.  */
      if (bed->want_got_plt)
	s = bfd_make_section_with_flags (abfd, ".igot.plt", flags);
      else
	s = bfd_make_section_with_flags (abfd, ".igot", flags);
      if (s == NULL
	  || !bfd_set_section_alignment (s, bed->s->log_file_align))
	return false;
      htab->igotplt = s;
    }

  return true;
}

/* Allocate space in .plt, .got and associated reloc sections for
   dynamic relocs against a STT_GNU_IFUNC symbol definition.  */

bool
_bfd_elf_allocate_ifunc_dyn_relocs (struct bfd_link_info *info,
				    struct elf_link_hash_entry *h,
				    struct elf_dyn_relocs **head,
				    unsigned int plt_entry_size,
				    unsigned int plt_header_size,
				    unsigned int got_entry_size,
				    bool avoid_plt)
{
  asection *plt, *gotplt, *relplt;
  struct elf_dyn_relocs *p;
  unsigned int sizeof_reloc;
  const struct elf_backend_data *bed;
  struct elf_link_hash_table *htab;
  /* If AVOID_PLT is TRUE, don't use PLT if possible.  */
  bool use_plt = !avoid_plt || h->plt.refcount > 0;
  bool need_dynreloc = !use_plt || bfd_link_pic (info);

  /* When a PIC object references a STT_GNU_IFUNC symbol defined
     in executable or it isn't referenced via PLT, the address of
     the resolved function may be used.  But in non-PIC executable,
     the address of its .plt slot may be used.  Pointer equality may
     not work correctly.  PIE or non-PLT reference should be used if
     pointer equality is required here.

     If STT_GNU_IFUNC symbol is defined in position-dependent executable,
     backend should change it to the normal function and set its address
     to its PLT entry which should be resolved by R_*_IRELATIVE at
     run-time.  All external references should be resolved to its PLT in
     executable.  */
  if (!need_dynreloc
      && !(bfd_link_pde (info) && h->def_regular)
      && (h->dynindx != -1
	  || info->export_dynamic)
      && h->pointer_equality_needed)
    {
      info->callbacks->einfo
	/* xgettext:c-format */
	(_("%F%P: dynamic STT_GNU_IFUNC symbol `%s' with pointer "
	   "equality in `%pB' can not be used when making an "
	   "executable; recompile with -fPIE and relink with -pie\n"),
	 h->root.root.string,
	 h->root.u.def.section->owner);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  htab = elf_hash_table (info);

  /* When the symbol is marked with regular reference, if PLT isn't used
     or we are building a PIC object, we must keep dynamic relocation
     if there is non-GOT reference and use PLT if there is PC-relative
     reference.  */
  if (need_dynreloc && h->ref_regular)
    {
      bool keep = false;
      for (p = *head; p != NULL; p = p->next)
	if (p->count)
	  {
	    h->non_got_ref = 1;
	    /* Need dynamic relocations for non-GOT reference.  */
	    keep = true;
	    if (p->pc_count)
	      {
		/* Must use PLT for PC-relative reference.  */
		use_plt = true;
		need_dynreloc = bfd_link_pic (info);
		break;
	      }
	  }
      if (keep)
	goto keep;
    }

  /* Support garbage collection against STT_GNU_IFUNC symbols.  */
  if (h->plt.refcount <= 0 && h->got.refcount <= 0)
    {
      h->got = htab->init_got_offset;
      h->plt = htab->init_plt_offset;
      *head = NULL;
      return true;
    }

  /* Return and discard space for dynamic relocations against it if
     it is never referenced.  */
  if (!h->ref_regular)
    {
      if (h->plt.refcount > 0
	  || h->got.refcount > 0)
	abort ();
      h->got = htab->init_got_offset;
      h->plt = htab->init_plt_offset;
      *head = NULL;
      return true;
    }

 keep:
  bed = get_elf_backend_data (info->output_bfd);
  if (bed->rela_plts_and_copies_p)
    sizeof_reloc = bed->s->sizeof_rela;
  else
    sizeof_reloc = bed->s->sizeof_rel;

  /* When building a static executable, use .iplt, .igot.plt and
     .rel[a].iplt sections for STT_GNU_IFUNC symbols.  */
  if (htab->splt != NULL)
    {
      plt = htab->splt;
      gotplt = htab->sgotplt;
      relplt = htab->srelplt;

      /* If this is the first .plt entry and PLT is used, make room for
	 the special first entry.  */
      if (plt->size == 0 && use_plt)
	plt->size += plt_header_size;
    }
  else
    {
      plt = htab->iplt;
      gotplt = htab->igotplt;
      relplt = htab->irelplt;
    }

  if (use_plt)
    {
      /* Don't update value of STT_GNU_IFUNC symbol to PLT.  We need
	 the original value for R_*_IRELATIVE.  */
      h->plt.offset = plt->size;

      /* Make room for this entry in the .plt/.iplt section.  */
      plt->size += plt_entry_size;

      /* We also need to make an entry in the .got.plt/.got.iplt section,
	 which will be placed in the .got section by the linker script.  */
      gotplt->size += got_entry_size;
    }

  /* We also need to make an entry in the .rel[a].plt/.rel[a].iplt
     section for GOTPLT relocation if PLT is used.  */
  if (use_plt)
    {
      relplt->size += sizeof_reloc;
      relplt->reloc_count++;
    }

  /* We need dynamic relocation for STT_GNU_IFUNC symbol only when
     there is a non-GOT reference in a PIC object or PLT isn't used.  */
  if (!need_dynreloc || !h->non_got_ref)
    *head = NULL;

  /* Finally, allocate space.  */
  p = *head;
  if (p != NULL)
    {
      bfd_size_type count = 0;
      do
	{
	  count += p->count;
	  p = p->next;
	}
      while (p != NULL);

      htab->ifunc_resolvers = count != 0;

      /* Dynamic relocations are stored in
	 1. .rel[a].ifunc section in PIC object.
	 2. .rel[a].got section in dynamic executable.
	 3. .rel[a].iplt section in static executable.  */
      if (bfd_link_pic (info))
	htab->irelifunc->size += count * sizeof_reloc;
      else if (htab->splt != NULL)
	htab->srelgot->size += count * sizeof_reloc;
      else
	{
	  relplt->size += count * sizeof_reloc;
	  relplt->reloc_count += count;
	}
    }

  /* For STT_GNU_IFUNC symbol, .got.plt has the real function address
     and .got has the PLT entry adddress.  We will load the GOT entry
     with the PLT entry in finish_dynamic_symbol if it is used.  For
     branch, it uses .got.plt.  For symbol value, if PLT is used,
     1. Use .got.plt in a PIC object if it is forced local or not
     dynamic.
     2. Use .got.plt in a non-PIC object if pointer equality isn't
     needed.
     3. Use .got.plt in PIE.
     4. Use .got.plt if .got isn't used.
     5. Otherwise use .got so that it can be shared among different
     objects at run-time.
     If PLT isn't used, always use .got for symbol value.
     We only need to relocate .got entry in PIC object or in dynamic
     executable without PLT.  */
  if (use_plt
      && (h->got.refcount <= 0
	  || (bfd_link_pic (info)
	      && (h->dynindx == -1
		  || h->forced_local))
	  || (!bfd_link_pic (info)
	      && !h->pointer_equality_needed)
	  || bfd_link_pie (info)
	  || htab->sgot == NULL))
    {
      /* Use .got.plt.  */
      h->got.offset = (bfd_vma) -1;
    }
  else
    {
      if (!use_plt)
	{
	  /* PLT isn't used.  */
	  h->plt.offset = (bfd_vma) -1;
	}
      if (h->got.refcount <= 0)
	{
	  /* GOT isn't need when there are only relocations for static
	     pointers.  */
	  h->got.offset = (bfd_vma) -1;
	}
      else
	{
	  h->got.offset = htab->sgot->size;
	  htab->sgot->size += got_entry_size;
	  /* Need to relocate the GOT entry in a PIC object or PLT isn't
	     used.  Otherwise, the GOT entry will be filled with the PLT
	     entry and dynamic GOT relocation isn't needed.  */
	  if (need_dynreloc)
	    {
	      /* For non-static executable, dynamic GOT relocation is in
		 .rel[a].got section, but for static executable, it is
		 in .rel[a].iplt section.  */
	      if (htab->splt != NULL)
		htab->srelgot->size += sizeof_reloc;
	      else
		{
		  relplt->size += sizeof_reloc;
		  relplt->reloc_count++;
		}
	    }
	}
    }

  return true;
}
