/* ELF linking support for BFD.
   Copyright (C) 1995-2023 Free Software Foundation, Inc.

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
#if BFD_SUPPORTS_PLUGINS
#include "plugin-api.h"
#include "plugin.h"
#endif

#include <limits.h>
#ifndef CHAR_BIT
#define CHAR_BIT 8
#endif

/* This struct is used to pass information to routines called via
   elf_link_hash_traverse which must return failure.  */

struct elf_info_failed
{
  struct bfd_link_info *info;
  bool failed;
};

/* This structure is used to pass information to
   _bfd_elf_link_find_version_dependencies.  */

struct elf_find_verdep_info
{
  /* General link information.  */
  struct bfd_link_info *info;
  /* The number of dependencies.  */
  unsigned int vers;
  /* Whether we had a failure.  */
  bool failed;
};

static bool _bfd_elf_fix_symbol_flags
  (struct elf_link_hash_entry *, struct elf_info_failed *);

asection *
_bfd_elf_section_for_symbol (struct elf_reloc_cookie *cookie,
			     unsigned long r_symndx,
			     bool discard)
{
  if (r_symndx >= cookie->locsymcount
      || ELF_ST_BIND (cookie->locsyms[r_symndx].st_info) != STB_LOCAL)
    {
      struct elf_link_hash_entry *h;

      h = cookie->sym_hashes[r_symndx - cookie->extsymoff];

      while (h->root.type == bfd_link_hash_indirect
	     || h->root.type == bfd_link_hash_warning)
	h = (struct elf_link_hash_entry *) h->root.u.i.link;

      if ((h->root.type == bfd_link_hash_defined
	   || h->root.type == bfd_link_hash_defweak)
	   && discarded_section (h->root.u.def.section))
	return h->root.u.def.section;
      else
	return NULL;
    }
  else
    {
      /* It's not a relocation against a global symbol,
	 but it could be a relocation against a local
	 symbol for a discarded section.  */
      asection *isec;
      Elf_Internal_Sym *isym;

      /* Need to: get the symbol; get the section.  */
      isym = &cookie->locsyms[r_symndx];
      isec = bfd_section_from_elf_index (cookie->abfd, isym->st_shndx);
      if (isec != NULL
	  && discard ? discarded_section (isec) : 1)
	return isec;
     }
  return NULL;
}

/* Define a symbol in a dynamic linkage section.  */

struct elf_link_hash_entry *
_bfd_elf_define_linkage_sym (bfd *abfd,
			     struct bfd_link_info *info,
			     asection *sec,
			     const char *name)
{
  struct elf_link_hash_entry *h;
  struct bfd_link_hash_entry *bh;
  const struct elf_backend_data *bed;

  h = elf_link_hash_lookup (elf_hash_table (info), name, false, false, false);
  if (h != NULL)
    {
      /* Zap symbol defined in an as-needed lib that wasn't linked.
	 This is a symptom of a larger problem:  Absolute symbols
	 defined in shared libraries can't be overridden, because we
	 lose the link to the bfd which is via the symbol section.  */
      h->root.type = bfd_link_hash_new;
      bh = &h->root;
    }
  else
    bh = NULL;

  bed = get_elf_backend_data (abfd);
  if (!_bfd_generic_link_add_one_symbol (info, abfd, name, BSF_GLOBAL,
					 sec, 0, NULL, false, bed->collect,
					 &bh))
    return NULL;
  h = (struct elf_link_hash_entry *) bh;
  BFD_ASSERT (h != NULL);
  h->def_regular = 1;
  h->non_elf = 0;
  h->root.linker_def = 1;
  h->type = STT_OBJECT;
  if (ELF_ST_VISIBILITY (h->other) != STV_INTERNAL)
    h->other = (h->other & ~ELF_ST_VISIBILITY (-1)) | STV_HIDDEN;

  (*bed->elf_backend_hide_symbol) (info, h, true);
  return h;
}

bool
_bfd_elf_create_got_section (bfd *abfd, struct bfd_link_info *info)
{
  flagword flags;
  asection *s;
  struct elf_link_hash_entry *h;
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  struct elf_link_hash_table *htab = elf_hash_table (info);

  /* This function may be called more than once.  */
  if (htab->sgot != NULL)
    return true;

  flags = bed->dynamic_sec_flags;

  s = bfd_make_section_anyway_with_flags (abfd,
					  (bed->rela_plts_and_copies_p
					   ? ".rela.got" : ".rel.got"),
					  (bed->dynamic_sec_flags
					   | SEC_READONLY));
  if (s == NULL
      || !bfd_set_section_alignment (s, bed->s->log_file_align))
    return false;
  htab->srelgot = s;

  s = bfd_make_section_anyway_with_flags (abfd, ".got", flags);
  if (s == NULL
      || !bfd_set_section_alignment (s, bed->s->log_file_align))
    return false;
  htab->sgot = s;

  if (bed->want_got_plt)
    {
      s = bfd_make_section_anyway_with_flags (abfd, ".got.plt", flags);
      if (s == NULL
	  || !bfd_set_section_alignment (s, bed->s->log_file_align))
	return false;
      htab->sgotplt = s;
    }

  /* The first bit of the global offset table is the header.  */
  s->size += bed->got_header_size;

  if (bed->want_got_sym)
    {
      /* Define the symbol _GLOBAL_OFFSET_TABLE_ at the start of the .got
	 (or .got.plt) section.  We don't do this in the linker script
	 because we don't want to define the symbol if we are not creating
	 a global offset table.  */
      h = _bfd_elf_define_linkage_sym (abfd, info, s,
				       "_GLOBAL_OFFSET_TABLE_");
      elf_hash_table (info)->hgot = h;
      if (h == NULL)
	return false;
    }

  return true;
}

/* Create a strtab to hold the dynamic symbol names.  */
static bool
_bfd_elf_link_create_dynstrtab (bfd *abfd, struct bfd_link_info *info)
{
  struct elf_link_hash_table *hash_table;

  hash_table = elf_hash_table (info);
  if (hash_table->dynobj == NULL)
    {
      /* We may not set dynobj, an input file holding linker created
	 dynamic sections to abfd, which may be a dynamic object with
	 its own dynamic sections.  We need to find a normal input file
	 to hold linker created sections if possible.  */
      if ((abfd->flags & (DYNAMIC | BFD_PLUGIN)) != 0)
	{
	  bfd *ibfd;
	  asection *s;
	  for (ibfd = info->input_bfds; ibfd; ibfd = ibfd->link.next)
	    if ((ibfd->flags
		 & (DYNAMIC | BFD_LINKER_CREATED | BFD_PLUGIN)) == 0
		&& bfd_get_flavour (ibfd) == bfd_target_elf_flavour
		&& elf_object_id (ibfd) == elf_hash_table_id (hash_table)
		&& !((s = ibfd->sections) != NULL
		     && s->sec_info_type == SEC_INFO_TYPE_JUST_SYMS))
	      {
		abfd = ibfd;
		break;
	      }
	}
      hash_table->dynobj = abfd;
    }

  if (hash_table->dynstr == NULL)
    {
      hash_table->dynstr = _bfd_elf_strtab_init ();
      if (hash_table->dynstr == NULL)
	return false;
    }
  return true;
}

/* Create some sections which will be filled in with dynamic linking
   information.  ABFD is an input file which requires dynamic sections
   to be created.  The dynamic sections take up virtual memory space
   when the final executable is run, so we need to create them before
   addresses are assigned to the output sections.  We work out the
   actual contents and size of these sections later.  */

bool
_bfd_elf_link_create_dynamic_sections (bfd *abfd, struct bfd_link_info *info)
{
  flagword flags;
  asection *s;
  const struct elf_backend_data *bed;
  struct elf_link_hash_entry *h;

  if (! is_elf_hash_table (info->hash))
    return false;

  if (elf_hash_table (info)->dynamic_sections_created)
    return true;

  if (!_bfd_elf_link_create_dynstrtab (abfd, info))
    return false;

  abfd = elf_hash_table (info)->dynobj;
  bed = get_elf_backend_data (abfd);

  flags = bed->dynamic_sec_flags;

  /* A dynamically linked executable has a .interp section, but a
     shared library does not.  */
  if (bfd_link_executable (info) && !info->nointerp)
    {
      s = bfd_make_section_anyway_with_flags (abfd, ".interp",
					      flags | SEC_READONLY);
      if (s == NULL)
	return false;
    }

  /* Create sections to hold version informations.  These are removed
     if they are not needed.  */
  s = bfd_make_section_anyway_with_flags (abfd, ".gnu.version_d",
					  flags | SEC_READONLY);
  if (s == NULL
      || !bfd_set_section_alignment (s, bed->s->log_file_align))
    return false;

  s = bfd_make_section_anyway_with_flags (abfd, ".gnu.version",
					  flags | SEC_READONLY);
  if (s == NULL
      || !bfd_set_section_alignment (s, 1))
    return false;

  s = bfd_make_section_anyway_with_flags (abfd, ".gnu.version_r",
					  flags | SEC_READONLY);
  if (s == NULL
      || !bfd_set_section_alignment (s, bed->s->log_file_align))
    return false;

  s = bfd_make_section_anyway_with_flags (abfd, ".dynsym",
					  flags | SEC_READONLY);
  if (s == NULL
      || !bfd_set_section_alignment (s, bed->s->log_file_align))
    return false;
  elf_hash_table (info)->dynsym = s;

  s = bfd_make_section_anyway_with_flags (abfd, ".dynstr",
					  flags | SEC_READONLY);
  if (s == NULL)
    return false;

  s = bfd_make_section_anyway_with_flags (abfd, ".dynamic", flags);
  if (s == NULL
      || !bfd_set_section_alignment (s, bed->s->log_file_align))
    return false;

  /* The special symbol _DYNAMIC is always set to the start of the
     .dynamic section.  We could set _DYNAMIC in a linker script, but we
     only want to define it if we are, in fact, creating a .dynamic
     section.  We don't want to define it if there is no .dynamic
     section, since on some ELF platforms the start up code examines it
     to decide how to initialize the process.  */
  h = _bfd_elf_define_linkage_sym (abfd, info, s, "_DYNAMIC");
  elf_hash_table (info)->hdynamic = h;
  if (h == NULL)
    return false;

  if (info->emit_hash)
    {
      s = bfd_make_section_anyway_with_flags (abfd, ".hash",
					      flags | SEC_READONLY);
      if (s == NULL
	  || !bfd_set_section_alignment (s, bed->s->log_file_align))
	return false;
      elf_section_data (s)->this_hdr.sh_entsize = bed->s->sizeof_hash_entry;
    }

  if (info->emit_gnu_hash && bed->record_xhash_symbol == NULL)
    {
      s = bfd_make_section_anyway_with_flags (abfd, ".gnu.hash",
					      flags | SEC_READONLY);
      if (s == NULL
	  || !bfd_set_section_alignment (s, bed->s->log_file_align))
	return false;
      /* For 64-bit ELF, .gnu.hash is a non-uniform entity size section:
	 4 32-bit words followed by variable count of 64-bit words, then
	 variable count of 32-bit words.  */
      if (bed->s->arch_size == 64)
	elf_section_data (s)->this_hdr.sh_entsize = 0;
      else
	elf_section_data (s)->this_hdr.sh_entsize = 4;
    }

  if (info->enable_dt_relr)
    {
      s = bfd_make_section_anyway_with_flags (abfd, ".relr.dyn",
					      (bed->dynamic_sec_flags
					       | SEC_READONLY));
      if (s == NULL
	  || !bfd_set_section_alignment (s, bed->s->log_file_align))
	return false;
      elf_hash_table (info)->srelrdyn = s;
    }

  /* Let the backend create the rest of the sections.  This lets the
     backend set the right flags.  The backend will normally create
     the .got and .plt sections.  */
  if (bed->elf_backend_create_dynamic_sections == NULL
      || ! (*bed->elf_backend_create_dynamic_sections) (abfd, info))
    return false;

  elf_hash_table (info)->dynamic_sections_created = true;

  return true;
}

/* Create dynamic sections when linking against a dynamic object.  */

bool
_bfd_elf_create_dynamic_sections (bfd *abfd, struct bfd_link_info *info)
{
  flagword flags, pltflags;
  struct elf_link_hash_entry *h;
  asection *s;
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  struct elf_link_hash_table *htab = elf_hash_table (info);

  /* We need to create .plt, .rel[a].plt, .got, .got.plt, .dynbss, and
     .rel[a].bss sections.  */
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

  s = bfd_make_section_anyway_with_flags (abfd, ".plt", pltflags);
  if (s == NULL
      || !bfd_set_section_alignment (s, bed->plt_alignment))
    return false;
  htab->splt = s;

  /* Define the symbol _PROCEDURE_LINKAGE_TABLE_ at the start of the
     .plt section.  */
  if (bed->want_plt_sym)
    {
      h = _bfd_elf_define_linkage_sym (abfd, info, s,
				       "_PROCEDURE_LINKAGE_TABLE_");
      elf_hash_table (info)->hplt = h;
      if (h == NULL)
	return false;
    }

  s = bfd_make_section_anyway_with_flags (abfd,
					  (bed->rela_plts_and_copies_p
					   ? ".rela.plt" : ".rel.plt"),
					  flags | SEC_READONLY);
  if (s == NULL
      || !bfd_set_section_alignment (s, bed->s->log_file_align))
    return false;
  htab->srelplt = s;

  if (! _bfd_elf_create_got_section (abfd, info))
    return false;

  if (bed->want_dynbss)
    {
      /* The .dynbss section is a place to put symbols which are defined
	 by dynamic objects, are referenced by regular objects, and are
	 not functions.  We must allocate space for them in the process
	 image and use a R_*_COPY reloc to tell the dynamic linker to
	 initialize them at run time.  The linker script puts the .dynbss
	 section into the .bss section of the final image.  */
      s = bfd_make_section_anyway_with_flags (abfd, ".dynbss",
					      SEC_ALLOC | SEC_LINKER_CREATED);
      if (s == NULL)
	return false;
      htab->sdynbss = s;

      if (bed->want_dynrelro)
	{
	  /* Similarly, but for symbols that were originally in read-only
	     sections.  This section doesn't really need to have contents,
	     but make it like other .data.rel.ro sections.  */
	  s = bfd_make_section_anyway_with_flags (abfd, ".data.rel.ro",
						  flags);
	  if (s == NULL)
	    return false;
	  htab->sdynrelro = s;
	}

      /* The .rel[a].bss section holds copy relocs.  This section is not
	 normally needed.  We need to create it here, though, so that the
	 linker will map it to an output section.  We can't just create it
	 only if we need it, because we will not know whether we need it
	 until we have seen all the input files, and the first time the
	 main linker code calls BFD after examining all the input files
	 (size_dynamic_sections) the input sections have already been
	 mapped to the output sections.  If the section turns out not to
	 be needed, we can discard it later.  We will never need this
	 section when generating a shared object, since they do not use
	 copy relocs.  */
      if (bfd_link_executable (info))
	{
	  s = bfd_make_section_anyway_with_flags (abfd,
						  (bed->rela_plts_and_copies_p
						   ? ".rela.bss" : ".rel.bss"),
						  flags | SEC_READONLY);
	  if (s == NULL
	      || !bfd_set_section_alignment (s, bed->s->log_file_align))
	    return false;
	  htab->srelbss = s;

	  if (bed->want_dynrelro)
	    {
	      s = (bfd_make_section_anyway_with_flags
		   (abfd, (bed->rela_plts_and_copies_p
			   ? ".rela.data.rel.ro" : ".rel.data.rel.ro"),
		    flags | SEC_READONLY));
	      if (s == NULL
		  || !bfd_set_section_alignment (s, bed->s->log_file_align))
		return false;
	      htab->sreldynrelro = s;
	    }
	}
    }

  return true;
}

/* Record a new dynamic symbol.  We record the dynamic symbols as we
   read the input files, since we need to have a list of all of them
   before we can determine the final sizes of the output sections.
   Note that we may actually call this function even though we are not
   going to output any dynamic symbols; in some cases we know that a
   symbol should be in the dynamic symbol table, but only if there is
   one.  */

bool
bfd_elf_link_record_dynamic_symbol (struct bfd_link_info *info,
				    struct elf_link_hash_entry *h)
{
  if (h->dynindx == -1)
    {
      struct elf_strtab_hash *dynstr;
      char *p;
      const char *name;
      size_t indx;

      if (h->root.type == bfd_link_hash_defined
	  || h->root.type == bfd_link_hash_defweak)
	{
	  /* An IR symbol should not be made dynamic.  */
	  if (h->root.u.def.section != NULL
	      && h->root.u.def.section->owner != NULL
	      && (h->root.u.def.section->owner->flags & BFD_PLUGIN) != 0)
	    return true;
	}

      /* XXX: The ABI draft says the linker must turn hidden and
	 internal symbols into STB_LOCAL symbols when producing the
	 DSO. However, if ld.so honors st_other in the dynamic table,
	 this would not be necessary.  */
      switch (ELF_ST_VISIBILITY (h->other))
	{
	case STV_INTERNAL:
	case STV_HIDDEN:
	  if (h->root.type != bfd_link_hash_undefined
	      && h->root.type != bfd_link_hash_undefweak)
	    {
	      h->forced_local = 1;
	      if (!elf_hash_table (info)->is_relocatable_executable
		  || ((h->root.type == bfd_link_hash_defined
		       || h->root.type == bfd_link_hash_defweak)
		      && h->root.u.def.section->owner != NULL
		      && h->root.u.def.section->owner->no_export)
		  || (h->root.type == bfd_link_hash_common
		      && h->root.u.c.p->section->owner != NULL
		      && h->root.u.c.p->section->owner->no_export))
		return true;
	    }

	default:
	  break;
	}

      h->dynindx = elf_hash_table (info)->dynsymcount;
      ++elf_hash_table (info)->dynsymcount;

      dynstr = elf_hash_table (info)->dynstr;
      if (dynstr == NULL)
	{
	  /* Create a strtab to hold the dynamic symbol names.  */
	  elf_hash_table (info)->dynstr = dynstr = _bfd_elf_strtab_init ();
	  if (dynstr == NULL)
	    return false;
	}

      /* We don't put any version information in the dynamic string
	 table.  */
      name = h->root.root.string;
      p = strchr (name, ELF_VER_CHR);
      if (p != NULL)
	/* We know that the p points into writable memory.  In fact,
	   there are only a few symbols that have read-only names, being
	   those like _GLOBAL_OFFSET_TABLE_ that are created specially
	   by the backends.  Most symbols will have names pointing into
	   an ELF string table read from a file, or to objalloc memory.  */
	*p = 0;

      indx = _bfd_elf_strtab_add (dynstr, name, p != NULL);

      if (p != NULL)
	*p = ELF_VER_CHR;

      if (indx == (size_t) -1)
	return false;
      h->dynstr_index = indx;
    }

  return true;
}

/* Mark a symbol dynamic.  */

static void
bfd_elf_link_mark_dynamic_symbol (struct bfd_link_info *info,
				  struct elf_link_hash_entry *h,
				  Elf_Internal_Sym *sym)
{
  struct bfd_elf_dynamic_list *d = info->dynamic_list;

  /* It may be called more than once on the same H.  */
  if(h->dynamic || bfd_link_relocatable (info))
    return;

  if ((info->dynamic_data
       && (h->type == STT_OBJECT
	   || h->type == STT_COMMON
	   || (sym != NULL
	       && (ELF_ST_TYPE (sym->st_info) == STT_OBJECT
		   || ELF_ST_TYPE (sym->st_info) == STT_COMMON))))
      || (d != NULL
	  && h->non_elf
	  && (*d->match) (&d->head, NULL, h->root.root.string)))
    {
      h->dynamic = 1;
      /* NB: If a symbol is made dynamic by --dynamic-list, it has
	 non-IR reference.  */
      h->root.non_ir_ref_dynamic = 1;
    }
}

/* Record an assignment to a symbol made by a linker script.  We need
   this in case some dynamic object refers to this symbol.  */

bool
bfd_elf_record_link_assignment (bfd *output_bfd,
				struct bfd_link_info *info,
				const char *name,
				bool provide,
				bool hidden)
{
  struct elf_link_hash_entry *h, *hv;
  struct elf_link_hash_table *htab;
  const struct elf_backend_data *bed;

  if (!is_elf_hash_table (info->hash))
    return true;

  htab = elf_hash_table (info);
  h = elf_link_hash_lookup (htab, name, !provide, true, false);
  if (h == NULL)
    return provide;

  if (h->root.type == bfd_link_hash_warning)
    h = (struct elf_link_hash_entry *) h->root.u.i.link;

  if (h->versioned == unknown)
    {
      /* Set versioned if symbol version is unknown.  */
      char *version = strrchr (name, ELF_VER_CHR);
      if (version)
	{
	  if (version > name && version[-1] != ELF_VER_CHR)
	    h->versioned = versioned_hidden;
	  else
	    h->versioned = versioned;
	}
    }

  /* Symbols defined in a linker script but not referenced anywhere
     else will have non_elf set.  */
  if (h->non_elf)
    {
      bfd_elf_link_mark_dynamic_symbol (info, h, NULL);
      h->non_elf = 0;
    }

  switch (h->root.type)
    {
    case bfd_link_hash_defined:
    case bfd_link_hash_defweak:
    case bfd_link_hash_common:
      break;
    case bfd_link_hash_undefweak:
    case bfd_link_hash_undefined:
      /* Since we're defining the symbol, don't let it seem to have not
	 been defined.  record_dynamic_symbol and size_dynamic_sections
	 may depend on this.  */
      h->root.type = bfd_link_hash_new;
      if (h->root.u.undef.next != NULL || htab->root.undefs_tail == &h->root)
	bfd_link_repair_undef_list (&htab->root);
      break;
    case bfd_link_hash_new:
      break;
    case bfd_link_hash_indirect:
      /* We had a versioned symbol in a dynamic library.  We make the
	 the versioned symbol point to this one.  */
      bed = get_elf_backend_data (output_bfd);
      hv = h;
      while (hv->root.type == bfd_link_hash_indirect
	     || hv->root.type == bfd_link_hash_warning)
	hv = (struct elf_link_hash_entry *) hv->root.u.i.link;
      /* We don't need to update h->root.u since linker will set them
	 later.  */
      h->root.type = bfd_link_hash_undefined;
      hv->root.type = bfd_link_hash_indirect;
      hv->root.u.i.link = (struct bfd_link_hash_entry *) h;
      (*bed->elf_backend_copy_indirect_symbol) (info, h, hv);
      break;
    default:
      BFD_FAIL ();
      return false;
    }

  /* If this symbol is being provided by the linker script, and it is
     currently defined by a dynamic object, but not by a regular
     object, then mark it as undefined so that the generic linker will
     force the correct value.  */
  if (provide
      && h->def_dynamic
      && !h->def_regular)
    h->root.type = bfd_link_hash_undefined;

  /* If this symbol is currently defined by a dynamic object, but not
     by a regular object, then clear out any version information because
     the symbol will not be associated with the dynamic object any
     more.  */
  if (h->def_dynamic && !h->def_regular)
    h->verinfo.verdef = NULL;

  /* Make sure this symbol is not garbage collected.  */
  h->mark = 1;

  h->def_regular = 1;

  if (hidden)
    {
      bed = get_elf_backend_data (output_bfd);
      if (ELF_ST_VISIBILITY (h->other) != STV_INTERNAL)
	h->other = (h->other & ~ELF_ST_VISIBILITY (-1)) | STV_HIDDEN;
      (*bed->elf_backend_hide_symbol) (info, h, true);
    }

  /* STV_HIDDEN and STV_INTERNAL symbols must be STB_LOCAL in shared objects
     and executables.  */
  if (!bfd_link_relocatable (info)
      && h->dynindx != -1
      && (ELF_ST_VISIBILITY (h->other) == STV_HIDDEN
	  || ELF_ST_VISIBILITY (h->other) == STV_INTERNAL))
    h->forced_local = 1;

  if ((h->def_dynamic
       || h->ref_dynamic
       || bfd_link_dll (info)
       || elf_hash_table (info)->is_relocatable_executable)
      && !h->forced_local
      && h->dynindx == -1)
    {
      if (! bfd_elf_link_record_dynamic_symbol (info, h))
	return false;

      /* If this is a weak defined symbol, and we know a corresponding
	 real symbol from the same dynamic object, make sure the real
	 symbol is also made into a dynamic symbol.  */
      if (h->is_weakalias)
	{
	  struct elf_link_hash_entry *def = weakdef (h);

	  if (def->dynindx == -1
	      && !bfd_elf_link_record_dynamic_symbol (info, def))
	    return false;
	}
    }

  return true;
}

/* Record a new local dynamic symbol.  Returns 0 on failure, 1 on
   success, and 2 on a failure caused by attempting to record a symbol
   in a discarded section, eg. a discarded link-once section symbol.  */

int
bfd_elf_link_record_local_dynamic_symbol (struct bfd_link_info *info,
					  bfd *input_bfd,
					  long input_indx)
{
  size_t amt;
  struct elf_link_local_dynamic_entry *entry;
  struct elf_link_hash_table *eht;
  struct elf_strtab_hash *dynstr;
  size_t dynstr_index;
  char *name;
  Elf_External_Sym_Shndx eshndx;
  char esym[sizeof (Elf64_External_Sym)];

  if (! is_elf_hash_table (info->hash))
    return 0;

  /* See if the entry exists already.  */
  for (entry = elf_hash_table (info)->dynlocal; entry ; entry = entry->next)
    if (entry->input_bfd == input_bfd && entry->input_indx == input_indx)
      return 1;

  amt = sizeof (*entry);
  entry = (struct elf_link_local_dynamic_entry *) bfd_alloc (input_bfd, amt);
  if (entry == NULL)
    return 0;

  /* Go find the symbol, so that we can find it's name.  */
  if (!bfd_elf_get_elf_syms (input_bfd, &elf_tdata (input_bfd)->symtab_hdr,
			     1, input_indx, &entry->isym, esym, &eshndx))
    {
      bfd_release (input_bfd, entry);
      return 0;
    }

  if (entry->isym.st_shndx != SHN_UNDEF
      && entry->isym.st_shndx < SHN_LORESERVE)
    {
      asection *s;

      s = bfd_section_from_elf_index (input_bfd, entry->isym.st_shndx);
      if (s == NULL || bfd_is_abs_section (s->output_section))
	{
	  /* We can still bfd_release here as nothing has done another
	     bfd_alloc.  We can't do this later in this function.  */
	  bfd_release (input_bfd, entry);
	  return 2;
	}
    }

  name = (bfd_elf_string_from_elf_section
	  (input_bfd, elf_tdata (input_bfd)->symtab_hdr.sh_link,
	   entry->isym.st_name));

  dynstr = elf_hash_table (info)->dynstr;
  if (dynstr == NULL)
    {
      /* Create a strtab to hold the dynamic symbol names.  */
      elf_hash_table (info)->dynstr = dynstr = _bfd_elf_strtab_init ();
      if (dynstr == NULL)
	return 0;
    }

  dynstr_index = _bfd_elf_strtab_add (dynstr, name, false);
  if (dynstr_index == (size_t) -1)
    return 0;
  entry->isym.st_name = dynstr_index;

  eht = elf_hash_table (info);

  entry->next = eht->dynlocal;
  eht->dynlocal = entry;
  entry->input_bfd = input_bfd;
  entry->input_indx = input_indx;
  eht->dynsymcount++;

  /* Whatever binding the symbol had before, it's now local.  */
  entry->isym.st_info
    = ELF_ST_INFO (STB_LOCAL, ELF_ST_TYPE (entry->isym.st_info));

  /* The dynindx will be set at the end of size_dynamic_sections.  */

  return 1;
}

/* Return the dynindex of a local dynamic symbol.  */

long
_bfd_elf_link_lookup_local_dynindx (struct bfd_link_info *info,
				    bfd *input_bfd,
				    long input_indx)
{
  struct elf_link_local_dynamic_entry *e;

  for (e = elf_hash_table (info)->dynlocal; e ; e = e->next)
    if (e->input_bfd == input_bfd && e->input_indx == input_indx)
      return e->dynindx;
  return -1;
}

/* This function is used to renumber the dynamic symbols, if some of
   them are removed because they are marked as local.  This is called
   via elf_link_hash_traverse.  */

static bool
elf_link_renumber_hash_table_dynsyms (struct elf_link_hash_entry *h,
				      void *data)
{
  size_t *count = (size_t *) data;

  if (h->forced_local)
    return true;

  if (h->dynindx != -1)
    h->dynindx = ++(*count);

  return true;
}


/* Like elf_link_renumber_hash_table_dynsyms, but just number symbols with
   STB_LOCAL binding.  */

static bool
elf_link_renumber_local_hash_table_dynsyms (struct elf_link_hash_entry *h,
					    void *data)
{
  size_t *count = (size_t *) data;

  if (!h->forced_local)
    return true;

  if (h->dynindx != -1)
    h->dynindx = ++(*count);

  return true;
}

/* Return true if the dynamic symbol for a given section should be
   omitted when creating a shared library.  */
bool
_bfd_elf_omit_section_dynsym_default (bfd *output_bfd ATTRIBUTE_UNUSED,
				      struct bfd_link_info *info,
				      asection *p)
{
  struct elf_link_hash_table *htab;
  asection *ip;

  switch (elf_section_data (p)->this_hdr.sh_type)
    {
    case SHT_PROGBITS:
    case SHT_NOBITS:
      /* If sh_type is yet undecided, assume it could be
	 SHT_PROGBITS/SHT_NOBITS.  */
    case SHT_NULL:
      htab = elf_hash_table (info);
      if (htab->text_index_section != NULL)
	return p != htab->text_index_section && p != htab->data_index_section;

      return (htab->dynobj != NULL
	      && (ip = bfd_get_linker_section (htab->dynobj, p->name)) != NULL
	      && ip->output_section == p);

      /* There shouldn't be section relative relocations
	 against any other section.  */
    default:
      return true;
    }
}

bool
_bfd_elf_omit_section_dynsym_all
    (bfd *output_bfd ATTRIBUTE_UNUSED,
     struct bfd_link_info *info ATTRIBUTE_UNUSED,
     asection *p ATTRIBUTE_UNUSED)
{
  return true;
}

/* Assign dynsym indices.  In a shared library we generate a section
   symbol for each output section, which come first.  Next come symbols
   which have been forced to local binding.  Then all of the back-end
   allocated local dynamic syms, followed by the rest of the global
   symbols.  If SECTION_SYM_COUNT is NULL, section dynindx is not set.
   (This prevents the early call before elf_backend_init_index_section
   and strip_excluded_output_sections setting dynindx for sections
   that are stripped.)  */

static unsigned long
_bfd_elf_link_renumber_dynsyms (bfd *output_bfd,
				struct bfd_link_info *info,
				unsigned long *section_sym_count)
{
  unsigned long dynsymcount = 0;
  bool do_sec = section_sym_count != NULL;

  if (bfd_link_pic (info)
      || elf_hash_table (info)->is_relocatable_executable)
    {
      const struct elf_backend_data *bed = get_elf_backend_data (output_bfd);
      asection *p;
      for (p = output_bfd->sections; p ; p = p->next)
	if ((p->flags & SEC_EXCLUDE) == 0
	    && (p->flags & SEC_ALLOC) != 0
	    && elf_hash_table (info)->dynamic_relocs
	    && !(*bed->elf_backend_omit_section_dynsym) (output_bfd, info, p))
	  {
	    ++dynsymcount;
	    if (do_sec)
	      elf_section_data (p)->dynindx = dynsymcount;
	  }
	else if (do_sec)
	  elf_section_data (p)->dynindx = 0;
    }
  if (do_sec)
    *section_sym_count = dynsymcount;

  elf_link_hash_traverse (elf_hash_table (info),
			  elf_link_renumber_local_hash_table_dynsyms,
			  &dynsymcount);

  if (elf_hash_table (info)->dynlocal)
    {
      struct elf_link_local_dynamic_entry *p;
      for (p = elf_hash_table (info)->dynlocal; p ; p = p->next)
	p->dynindx = ++dynsymcount;
    }
  elf_hash_table (info)->local_dynsymcount = dynsymcount;

  elf_link_hash_traverse (elf_hash_table (info),
			  elf_link_renumber_hash_table_dynsyms,
			  &dynsymcount);

  /* There is an unused NULL entry at the head of the table which we
     must account for in our count even if the table is empty since it
     is intended for the mandatory DT_SYMTAB tag (.dynsym section) in
     .dynamic section.  */
  dynsymcount++;

  elf_hash_table (info)->dynsymcount = dynsymcount;
  return dynsymcount;
}

/* Merge st_other field.  */

static void
elf_merge_st_other (bfd *abfd, struct elf_link_hash_entry *h,
		    unsigned int st_other, asection *sec,
		    bool definition, bool dynamic)
{
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);

  /* If st_other has a processor-specific meaning, specific
     code might be needed here.  */
  if (bed->elf_backend_merge_symbol_attribute)
    (*bed->elf_backend_merge_symbol_attribute) (h, st_other, definition,
						dynamic);

  if (!dynamic)
    {
      unsigned symvis = ELF_ST_VISIBILITY (st_other);
      unsigned hvis = ELF_ST_VISIBILITY (h->other);

      /* Keep the most constraining visibility.  Leave the remainder
	 of the st_other field to elf_backend_merge_symbol_attribute.  */
      if (symvis - 1 < hvis - 1)
	h->other = symvis | (h->other & ~ELF_ST_VISIBILITY (-1));
    }
  else if (definition
	   && ELF_ST_VISIBILITY (st_other) != STV_DEFAULT
	   && (sec->flags & SEC_READONLY) == 0)
    h->protected_def = 1;
}

/* This function is called when we want to merge a new symbol with an
   existing symbol.  It handles the various cases which arise when we
   find a definition in a dynamic object, or when there is already a
   definition in a dynamic object.  The new symbol is described by
   NAME, SYM, PSEC, and PVALUE.  We set SYM_HASH to the hash table
   entry.  We set POLDBFD to the old symbol's BFD.  We set POLD_WEAK
   if the old symbol was weak.  We set POLD_ALIGNMENT to the alignment
   of an old common symbol.  We set OVERRIDE if the old symbol is
   overriding a new definition.  We set TYPE_CHANGE_OK if it is OK for
   the type to change.  We set SIZE_CHANGE_OK if it is OK for the size
   to change.  By OK to change, we mean that we shouldn't warn if the
   type or size does change.  */

static bool
_bfd_elf_merge_symbol (bfd *abfd,
		       struct bfd_link_info *info,
		       const char *name,
		       Elf_Internal_Sym *sym,
		       asection **psec,
		       bfd_vma *pvalue,
		       struct elf_link_hash_entry **sym_hash,
		       bfd **poldbfd,
		       bool *pold_weak,
		       unsigned int *pold_alignment,
		       bool *skip,
		       bfd **override,
		       bool *type_change_ok,
		       bool *size_change_ok,
		       bool *matched)
{
  asection *sec, *oldsec;
  struct elf_link_hash_entry *h;
  struct elf_link_hash_entry *hi;
  struct elf_link_hash_entry *flip;
  int bind;
  bfd *oldbfd;
  bool newdyn, olddyn, olddef, newdef, newdyncommon, olddyncommon;
  bool newweak, oldweak, newfunc, oldfunc;
  const struct elf_backend_data *bed;
  char *new_version;
  bool default_sym = *matched;
  struct elf_link_hash_table *htab;

  *skip = false;
  *override = NULL;

  sec = *psec;
  bind = ELF_ST_BIND (sym->st_info);

  if (! bfd_is_und_section (sec))
    h = elf_link_hash_lookup (elf_hash_table (info), name, true, false, false);
  else
    h = ((struct elf_link_hash_entry *)
	 bfd_wrapped_link_hash_lookup (abfd, info, name, true, false, false));
  if (h == NULL)
    return false;
  *sym_hash = h;

  bed = get_elf_backend_data (abfd);

  /* NEW_VERSION is the symbol version of the new symbol.  */
  if (h->versioned != unversioned)
    {
      /* Symbol version is unknown or versioned.  */
      new_version = strrchr (name, ELF_VER_CHR);
      if (new_version)
	{
	  if (h->versioned == unknown)
	    {
	      if (new_version > name && new_version[-1] != ELF_VER_CHR)
		h->versioned = versioned_hidden;
	      else
		h->versioned = versioned;
	    }
	  new_version += 1;
	  if (new_version[0] == '\0')
	    new_version = NULL;
	}
      else
	h->versioned = unversioned;
    }
  else
    new_version = NULL;

  /* For merging, we only care about real symbols.  But we need to make
     sure that indirect symbol dynamic flags are updated.  */
  hi = h;
  while (h->root.type == bfd_link_hash_indirect
	 || h->root.type == bfd_link_hash_warning)
    h = (struct elf_link_hash_entry *) h->root.u.i.link;

  if (!*matched)
    {
      if (hi == h || h->root.type == bfd_link_hash_new)
	*matched = true;
      else
	{
	  /* OLD_HIDDEN is true if the existing symbol is only visible
	     to the symbol with the same symbol version.  NEW_HIDDEN is
	     true if the new symbol is only visible to the symbol with
	     the same symbol version.  */
	  bool old_hidden = h->versioned == versioned_hidden;
	  bool new_hidden = hi->versioned == versioned_hidden;
	  if (!old_hidden && !new_hidden)
	    /* The new symbol matches the existing symbol if both
	       aren't hidden.  */
	    *matched = true;
	  else
	    {
	      /* OLD_VERSION is the symbol version of the existing
		 symbol. */
	      char *old_version;

	      if (h->versioned >= versioned)
		old_version = strrchr (h->root.root.string,
				       ELF_VER_CHR) + 1;
	      else
		 old_version = NULL;

	      /* The new symbol matches the existing symbol if they
		 have the same symbol version.  */
	      *matched = (old_version == new_version
			  || (old_version != NULL
			      && new_version != NULL
			      && strcmp (old_version, new_version) == 0));
	    }
	}
    }

  /* OLDBFD and OLDSEC are a BFD and an ASECTION associated with the
     existing symbol.  */

  oldbfd = NULL;
  oldsec = NULL;
  switch (h->root.type)
    {
    default:
      break;

    case bfd_link_hash_undefined:
    case bfd_link_hash_undefweak:
      oldbfd = h->root.u.undef.abfd;
      break;

    case bfd_link_hash_defined:
    case bfd_link_hash_defweak:
      oldbfd = h->root.u.def.section->owner;
      oldsec = h->root.u.def.section;
      break;

    case bfd_link_hash_common:
      oldbfd = h->root.u.c.p->section->owner;
      oldsec = h->root.u.c.p->section;
      if (pold_alignment)
	*pold_alignment = h->root.u.c.p->alignment_power;
      break;
    }
  if (poldbfd && *poldbfd == NULL)
    *poldbfd = oldbfd;

  /* Differentiate strong and weak symbols.  */
  newweak = bind == STB_WEAK;
  oldweak = (h->root.type == bfd_link_hash_defweak
	     || h->root.type == bfd_link_hash_undefweak);
  if (pold_weak)
    *pold_weak = oldweak;

  /* We have to check it for every instance since the first few may be
     references and not all compilers emit symbol type for undefined
     symbols.  */
  bfd_elf_link_mark_dynamic_symbol (info, h, sym);

  htab = elf_hash_table (info);

  /* NEWDYN and OLDDYN indicate whether the new or old symbol,
     respectively, is from a dynamic object.  */

  newdyn = (abfd->flags & DYNAMIC) != 0;

  /* ref_dynamic_nonweak and dynamic_def flags track actual undefined
     syms and defined syms in dynamic libraries respectively.
     ref_dynamic on the other hand can be set for a symbol defined in
     a dynamic library, and def_dynamic may not be set;  When the
     definition in a dynamic lib is overridden by a definition in the
     executable use of the symbol in the dynamic lib becomes a
     reference to the executable symbol.  */
  if (newdyn)
    {
      if (bfd_is_und_section (sec))
	{
	  if (bind != STB_WEAK)
	    {
	      h->ref_dynamic_nonweak = 1;
	      hi->ref_dynamic_nonweak = 1;
	    }
	}
      else
	{
	  /* Update the existing symbol only if they match. */
	  if (*matched)
	    h->dynamic_def = 1;
	  hi->dynamic_def = 1;
	}
    }

  /* If we just created the symbol, mark it as being an ELF symbol.
     Other than that, there is nothing to do--there is no merge issue
     with a newly defined symbol--so we just return.  */

  if (h->root.type == bfd_link_hash_new)
    {
      h->non_elf = 0;
      return true;
    }

  /* In cases involving weak versioned symbols, we may wind up trying
     to merge a symbol with itself.  Catch that here, to avoid the
     confusion that results if we try to override a symbol with
     itself.  The additional tests catch cases like
     _GLOBAL_OFFSET_TABLE_, which are regular symbols defined in a
     dynamic object, which we do want to handle here.  */
  if (abfd == oldbfd
      && (newweak || oldweak)
      && ((abfd->flags & DYNAMIC) == 0
	  || !h->def_regular))
    return true;

  olddyn = false;
  if (oldbfd != NULL)
    olddyn = (oldbfd->flags & DYNAMIC) != 0;
  else if (oldsec != NULL)
    {
      /* This handles the special SHN_MIPS_{TEXT,DATA} section
	 indices used by MIPS ELF.  */
      olddyn = (oldsec->symbol->flags & BSF_DYNAMIC) != 0;
    }

  /* Set non_ir_ref_dynamic only when not handling DT_NEEDED entries.  */
  if (!htab->handling_dt_needed
      && oldbfd != NULL
      && (oldbfd->flags & BFD_PLUGIN) != (abfd->flags & BFD_PLUGIN))
    {
      if (newdyn != olddyn)
	{
	  /* Handle a case where plugin_notice won't be called and thus
	     won't set the non_ir_ref flags on the first pass over
	     symbols.  */
	  h->root.non_ir_ref_dynamic = true;
	  hi->root.non_ir_ref_dynamic = true;
	}
      else if ((oldbfd->flags & BFD_PLUGIN) != 0
	       && hi->root.type == bfd_link_hash_indirect)
	{
	  /* Change indirect symbol from IR to undefined.  */
	  hi->root.type = bfd_link_hash_undefined;
	  hi->root.u.undef.abfd = oldbfd;
	}
    }

  /* NEWDEF and OLDDEF indicate whether the new or old symbol,
     respectively, appear to be a definition rather than reference.  */

  newdef = !bfd_is_und_section (sec) && !bfd_is_com_section (sec);

  olddef = (h->root.type != bfd_link_hash_undefined
	    && h->root.type != bfd_link_hash_undefweak
	    && h->root.type != bfd_link_hash_common);

  /* NEWFUNC and OLDFUNC indicate whether the new or old symbol,
     respectively, appear to be a function.  */

  newfunc = (ELF_ST_TYPE (sym->st_info) != STT_NOTYPE
	     && bed->is_function_type (ELF_ST_TYPE (sym->st_info)));

  oldfunc = (h->type != STT_NOTYPE
	     && bed->is_function_type (h->type));

  if (!(newfunc && oldfunc)
      && ELF_ST_TYPE (sym->st_info) != h->type
      && ELF_ST_TYPE (sym->st_info) != STT_NOTYPE
      && h->type != STT_NOTYPE
      && (newdef || bfd_is_com_section (sec))
      && (olddef || h->root.type == bfd_link_hash_common))
    {
      /* If creating a default indirect symbol ("foo" or "foo@") from
	 a dynamic versioned definition ("foo@@") skip doing so if
	 there is an existing regular definition with a different
	 type.  We don't want, for example, a "time" variable in the
	 executable overriding a "time" function in a shared library.  */
      if (newdyn
	  && !olddyn)
	{
	  *skip = true;
	  return true;
	}

      /* When adding a symbol from a regular object file after we have
	 created indirect symbols, undo the indirection and any
	 dynamic state.  */
      if (hi != h
	  && !newdyn
	  && olddyn)
	{
	  h = hi;
	  (*bed->elf_backend_hide_symbol) (info, h, true);
	  h->forced_local = 0;
	  h->ref_dynamic = 0;
	  h->def_dynamic = 0;
	  h->dynamic_def = 0;
	  if (h->root.u.undef.next || info->hash->undefs_tail == &h->root)
	    {
	      h->root.type = bfd_link_hash_undefined;
	      h->root.u.undef.abfd = abfd;
	    }
	  else
	    {
	      h->root.type = bfd_link_hash_new;
	      h->root.u.undef.abfd = NULL;
	    }
	  return true;
	}
    }

  /* Check TLS symbols.  We don't check undefined symbols introduced
     by "ld -u" which have no type (and oldbfd NULL), and we don't
     check symbols from plugins because they also have no type.  */
  if (oldbfd != NULL
      && (oldbfd->flags & BFD_PLUGIN) == 0
      && (abfd->flags & BFD_PLUGIN) == 0
      && ELF_ST_TYPE (sym->st_info) != h->type
      && (ELF_ST_TYPE (sym->st_info) == STT_TLS || h->type == STT_TLS))
    {
      bfd *ntbfd, *tbfd;
      bool ntdef, tdef;
      asection *ntsec, *tsec;

      if (h->type == STT_TLS)
	{
	  ntbfd = abfd;
	  ntsec = sec;
	  ntdef = newdef;
	  tbfd = oldbfd;
	  tsec = oldsec;
	  tdef = olddef;
	}
      else
	{
	  ntbfd = oldbfd;
	  ntsec = oldsec;
	  ntdef = olddef;
	  tbfd = abfd;
	  tsec = sec;
	  tdef = newdef;
	}

      if (tdef && ntdef)
	_bfd_error_handler
	  /* xgettext:c-format */
	  (_("%s: TLS definition in %pB section %pA "
	     "mismatches non-TLS definition in %pB section %pA"),
	   h->root.root.string, tbfd, tsec, ntbfd, ntsec);
      else if (!tdef && !ntdef)
	_bfd_error_handler
	  /* xgettext:c-format */
	  (_("%s: TLS reference in %pB "
	     "mismatches non-TLS reference in %pB"),
	   h->root.root.string, tbfd, ntbfd);
      else if (tdef)
	_bfd_error_handler
	  /* xgettext:c-format */
	  (_("%s: TLS definition in %pB section %pA "
	     "mismatches non-TLS reference in %pB"),
	   h->root.root.string, tbfd, tsec, ntbfd);
      else
	_bfd_error_handler
	  /* xgettext:c-format */
	  (_("%s: TLS reference in %pB "
	     "mismatches non-TLS definition in %pB section %pA"),
	   h->root.root.string, tbfd, ntbfd, ntsec);

      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  /* If the old symbol has non-default visibility, we ignore the new
     definition from a dynamic object.  */
  if (newdyn
      && ELF_ST_VISIBILITY (h->other) != STV_DEFAULT
      && !bfd_is_und_section (sec))
    {
      *skip = true;
      /* Make sure this symbol is dynamic.  */
      h->ref_dynamic = 1;
      hi->ref_dynamic = 1;
      /* A protected symbol has external availability. Make sure it is
	 recorded as dynamic.

	 FIXME: Should we check type and size for protected symbol?  */
      if (ELF_ST_VISIBILITY (h->other) == STV_PROTECTED)
	return bfd_elf_link_record_dynamic_symbol (info, h);
      else
	return true;
    }
  else if (!newdyn
	   && ELF_ST_VISIBILITY (sym->st_other) != STV_DEFAULT
	   && h->def_dynamic)
    {
      /* If the new symbol with non-default visibility comes from a
	 relocatable file and the old definition comes from a dynamic
	 object, we remove the old definition.  */
      if (hi->root.type == bfd_link_hash_indirect)
	{
	  /* Handle the case where the old dynamic definition is
	     default versioned.  We need to copy the symbol info from
	     the symbol with default version to the normal one if it
	     was referenced before.  */
	  if (h->ref_regular)
	    {
	      hi->root.type = h->root.type;
	      h->root.type = bfd_link_hash_indirect;
	      (*bed->elf_backend_copy_indirect_symbol) (info, hi, h);

	      h->root.u.i.link = (struct bfd_link_hash_entry *) hi;
	      if (ELF_ST_VISIBILITY (sym->st_other) != STV_PROTECTED)
		{
		  /* If the new symbol is hidden or internal, completely undo
		     any dynamic link state.  */
		  (*bed->elf_backend_hide_symbol) (info, h, true);
		  h->forced_local = 0;
		  h->ref_dynamic = 0;
		}
	      else
		h->ref_dynamic = 1;

	      h->def_dynamic = 0;
	      /* FIXME: Should we check type and size for protected symbol?  */
	      h->size = 0;
	      h->type = 0;

	      h = hi;
	    }
	  else
	    h = hi;
	}

      /* If the old symbol was undefined before, then it will still be
	 on the undefs list.  If the new symbol is undefined or
	 common, we can't make it bfd_link_hash_new here, because new
	 undefined or common symbols will be added to the undefs list
	 by _bfd_generic_link_add_one_symbol.  Symbols may not be
	 added twice to the undefs list.  Also, if the new symbol is
	 undefweak then we don't want to lose the strong undef.  */
      if (h->root.u.undef.next || info->hash->undefs_tail == &h->root)
	{
	  h->root.type = bfd_link_hash_undefined;
	  h->root.u.undef.abfd = abfd;
	}
      else
	{
	  h->root.type = bfd_link_hash_new;
	  h->root.u.undef.abfd = NULL;
	}

      if (ELF_ST_VISIBILITY (sym->st_other) != STV_PROTECTED)
	{
	  /* If the new symbol is hidden or internal, completely undo
	     any dynamic link state.  */
	  (*bed->elf_backend_hide_symbol) (info, h, true);
	  h->forced_local = 0;
	  h->ref_dynamic = 0;
	}
      else
	h->ref_dynamic = 1;
      h->def_dynamic = 0;
      /* FIXME: Should we check type and size for protected symbol?  */
      h->size = 0;
      h->type = 0;
      return true;
    }

  /* If a new weak symbol definition comes from a regular file and the
     old symbol comes from a dynamic library, we treat the new one as
     strong.  Similarly, an old weak symbol definition from a regular
     file is treated as strong when the new symbol comes from a dynamic
     library.  Further, an old weak symbol from a dynamic library is
     treated as strong if the new symbol is from a dynamic library.
     This reflects the way glibc's ld.so works.

     Also allow a weak symbol to override a linker script symbol
     defined by an early pass over the script.  This is done so the
     linker knows the symbol is defined in an object file, for the
     DEFINED script function.

     Do this before setting *type_change_ok or *size_change_ok so that
     we warn properly when dynamic library symbols are overridden.  */

  if (newdef && !newdyn && (olddyn || h->root.ldscript_def))
    newweak = false;
  if (olddef && newdyn)
    oldweak = false;

  /* Allow changes between different types of function symbol.  */
  if (newfunc && oldfunc)
    *type_change_ok = true;

  /* It's OK to change the type if either the existing symbol or the
     new symbol is weak.  A type change is also OK if the old symbol
     is undefined and the new symbol is defined.  */

  if (oldweak
      || newweak
      || (newdef
	  && h->root.type == bfd_link_hash_undefined))
    *type_change_ok = true;

  /* It's OK to change the size if either the existing symbol or the
     new symbol is weak, or if the old symbol is undefined.  */

  if (*type_change_ok
      || h->root.type == bfd_link_hash_undefined)
    *size_change_ok = true;

  /* NEWDYNCOMMON and OLDDYNCOMMON indicate whether the new or old
     symbol, respectively, appears to be a common symbol in a dynamic
     object.  If a symbol appears in an uninitialized section, and is
     not weak, and is not a function, then it may be a common symbol
     which was resolved when the dynamic object was created.  We want
     to treat such symbols specially, because they raise special
     considerations when setting the symbol size: if the symbol
     appears as a common symbol in a regular object, and the size in
     the regular object is larger, we must make sure that we use the
     larger size.  This problematic case can always be avoided in C,
     but it must be handled correctly when using Fortran shared
     libraries.

     Note that if NEWDYNCOMMON is set, NEWDEF will be set, and
     likewise for OLDDYNCOMMON and OLDDEF.

     Note that this test is just a heuristic, and that it is quite
     possible to have an uninitialized symbol in a shared object which
     is really a definition, rather than a common symbol.  This could
     lead to some minor confusion when the symbol really is a common
     symbol in some regular object.  However, I think it will be
     harmless.  */

  if (newdyn
      && newdef
      && !newweak
      && (sec->flags & SEC_ALLOC) != 0
      && (sec->flags & SEC_LOAD) == 0
      && sym->st_size > 0
      && !newfunc)
    newdyncommon = true;
  else
    newdyncommon = false;

  if (olddyn
      && olddef
      && h->root.type == bfd_link_hash_defined
      && h->def_dynamic
      && (h->root.u.def.section->flags & SEC_ALLOC) != 0
      && (h->root.u.def.section->flags & SEC_LOAD) == 0
      && h->size > 0
      && !oldfunc)
    olddyncommon = true;
  else
    olddyncommon = false;

  /* We now know everything about the old and new symbols.  We ask the
     backend to check if we can merge them.  */
  if (bed->merge_symbol != NULL)
    {
      if (!bed->merge_symbol (h, sym, psec, newdef, olddef, oldbfd, oldsec))
	return false;
      sec = *psec;
    }

  /* There are multiple definitions of a normal symbol.  Skip the
     default symbol as well as definition from an IR object.  */
  if (olddef && !olddyn && !oldweak && newdef && !newdyn && !newweak
      && !default_sym && h->def_regular
      && !(oldbfd != NULL
	   && (oldbfd->flags & BFD_PLUGIN) != 0
	   && (abfd->flags & BFD_PLUGIN) == 0))
    {
      /* Handle a multiple definition.  */
      (*info->callbacks->multiple_definition) (info, &h->root,
					       abfd, sec, *pvalue);
      *skip = true;
      return true;
    }

  /* If both the old and the new symbols look like common symbols in a
     dynamic object, set the size of the symbol to the larger of the
     two.  */

  if (olddyncommon
      && newdyncommon
      && sym->st_size != h->size)
    {
      /* Since we think we have two common symbols, issue a multiple
	 common warning if desired.  Note that we only warn if the
	 size is different.  If the size is the same, we simply let
	 the old symbol override the new one as normally happens with
	 symbols defined in dynamic objects.  */

      (*info->callbacks->multiple_common) (info, &h->root, abfd,
					   bfd_link_hash_common, sym->st_size);
      if (sym->st_size > h->size)
	h->size = sym->st_size;

      *size_change_ok = true;
    }

  /* If we are looking at a dynamic object, and we have found a
     definition, we need to see if the symbol was already defined by
     some other object.  If so, we want to use the existing
     definition, and we do not want to report a multiple symbol
     definition error; we do this by clobbering *PSEC to be
     bfd_und_section_ptr.

     We treat a common symbol as a definition if the symbol in the
     shared library is a function, since common symbols always
     represent variables; this can cause confusion in principle, but
     any such confusion would seem to indicate an erroneous program or
     shared library.  We also permit a common symbol in a regular
     object to override a weak symbol in a shared object.  */

  if (newdyn
      && newdef
      && (olddef
	  || (h->root.type == bfd_link_hash_common
	      && (newweak || newfunc))))
    {
      *override = abfd;
      newdef = false;
      newdyncommon = false;

      *psec = sec = bfd_und_section_ptr;
      *size_change_ok = true;

      /* If we get here when the old symbol is a common symbol, then
	 we are explicitly letting it override a weak symbol or
	 function in a dynamic object, and we don't want to warn about
	 a type change.  If the old symbol is a defined symbol, a type
	 change warning may still be appropriate.  */

      if (h->root.type == bfd_link_hash_common)
	*type_change_ok = true;
    }

  /* Handle the special case of an old common symbol merging with a
     new symbol which looks like a common symbol in a shared object.
     We change *PSEC and *PVALUE to make the new symbol look like a
     common symbol, and let _bfd_generic_link_add_one_symbol do the
     right thing.  */

  if (newdyncommon
      && h->root.type == bfd_link_hash_common)
    {
      *override = oldbfd;
      newdef = false;
      newdyncommon = false;
      *pvalue = sym->st_size;
      *psec = sec = bed->common_section (oldsec);
      *size_change_ok = true;
    }

  /* Skip weak definitions of symbols that are already defined.  */
  if (newdef && olddef && newweak)
    {
      /* Don't skip new non-IR weak syms.  */
      if (!(oldbfd != NULL
	    && (oldbfd->flags & BFD_PLUGIN) != 0
	    && (abfd->flags & BFD_PLUGIN) == 0))
	{
	  newdef = false;
	  *skip = true;
	}

      /* Merge st_other.  If the symbol already has a dynamic index,
	 but visibility says it should not be visible, turn it into a
	 local symbol.  */
      elf_merge_st_other (abfd, h, sym->st_other, sec, newdef, newdyn);
      if (h->dynindx != -1)
	switch (ELF_ST_VISIBILITY (h->other))
	  {
	  case STV_INTERNAL:
	  case STV_HIDDEN:
	    (*bed->elf_backend_hide_symbol) (info, h, true);
	    break;
	  }
    }

  /* If the old symbol is from a dynamic object, and the new symbol is
     a definition which is not from a dynamic object, then the new
     symbol overrides the old symbol.  Symbols from regular files
     always take precedence over symbols from dynamic objects, even if
     they are defined after the dynamic object in the link.

     As above, we again permit a common symbol in a regular object to
     override a definition in a shared object if the shared object
     symbol is a function or is weak.  */

  flip = NULL;
  if (!newdyn
      && (newdef
	  || (bfd_is_com_section (sec)
	      && (oldweak || oldfunc)))
      && olddyn
      && olddef
      && h->def_dynamic)
    {
      /* Change the hash table entry to undefined, and let
	 _bfd_generic_link_add_one_symbol do the right thing with the
	 new definition.  */

      h->root.type = bfd_link_hash_undefined;
      h->root.u.undef.abfd = h->root.u.def.section->owner;
      *size_change_ok = true;

      olddef = false;
      olddyncommon = false;

      /* We again permit a type change when a common symbol may be
	 overriding a function.  */

      if (bfd_is_com_section (sec))
	{
	  if (oldfunc)
	    {
	      /* If a common symbol overrides a function, make sure
		 that it isn't defined dynamically nor has type
		 function.  */
	      h->def_dynamic = 0;
	      h->type = STT_NOTYPE;
	    }
	  *type_change_ok = true;
	}

      if (hi->root.type == bfd_link_hash_indirect)
	flip = hi;
      else
	/* This union may have been set to be non-NULL when this symbol
	   was seen in a dynamic object.  We must force the union to be
	   NULL, so that it is correct for a regular symbol.  */
	h->verinfo.vertree = NULL;
    }

  /* Handle the special case of a new common symbol merging with an
     old symbol that looks like it might be a common symbol defined in
     a shared object.  Note that we have already handled the case in
     which a new common symbol should simply override the definition
     in the shared library.  */

  if (! newdyn
      && bfd_is_com_section (sec)
      && olddyncommon)
    {
      /* It would be best if we could set the hash table entry to a
	 common symbol, but we don't know what to use for the section
	 or the alignment.  */
      (*info->callbacks->multiple_common) (info, &h->root, abfd,
					   bfd_link_hash_common, sym->st_size);

      /* If the presumed common symbol in the dynamic object is
	 larger, pretend that the new symbol has its size.  */

      if (h->size > *pvalue)
	*pvalue = h->size;

      /* We need to remember the alignment required by the symbol
	 in the dynamic object.  */
      BFD_ASSERT (pold_alignment);
      *pold_alignment = h->root.u.def.section->alignment_power;

      olddef = false;
      olddyncommon = false;

      h->root.type = bfd_link_hash_undefined;
      h->root.u.undef.abfd = h->root.u.def.section->owner;

      *size_change_ok = true;
      *type_change_ok = true;

      if (hi->root.type == bfd_link_hash_indirect)
	flip = hi;
      else
	h->verinfo.vertree = NULL;
    }

  if (flip != NULL)
    {
      /* Handle the case where we had a versioned symbol in a dynamic
	 library and now find a definition in a normal object.  In this
	 case, we make the versioned symbol point to the normal one.  */
      flip->root.type = h->root.type;
      flip->root.u.undef.abfd = h->root.u.undef.abfd;
      h->root.type = bfd_link_hash_indirect;
      h->root.u.i.link = (struct bfd_link_hash_entry *) flip;
      (*bed->elf_backend_copy_indirect_symbol) (info, flip, h);
      if (h->def_dynamic)
	{
	  h->def_dynamic = 0;
	  flip->ref_dynamic = 1;
	}
    }

  return true;
}

/* This function is called to create an indirect symbol from the
   default for the symbol with the default version if needed. The
   symbol is described by H, NAME, SYM, SEC, and VALUE.  We
   set DYNSYM if the new indirect symbol is dynamic.  */

static bool
_bfd_elf_add_default_symbol (bfd *abfd,
			     struct bfd_link_info *info,
			     struct elf_link_hash_entry *h,
			     const char *name,
			     Elf_Internal_Sym *sym,
			     asection *sec,
			     bfd_vma value,
			     bfd **poldbfd,
			     bool *dynsym)
{
  bool type_change_ok;
  bool size_change_ok;
  bool skip;
  char *shortname;
  struct elf_link_hash_entry *hi;
  struct bfd_link_hash_entry *bh;
  const struct elf_backend_data *bed;
  bool collect;
  bool dynamic;
  bfd *override;
  char *p;
  size_t len, shortlen;
  asection *tmp_sec;
  bool matched;

  if (h->versioned == unversioned || h->versioned == versioned_hidden)
    return true;

  /* If this symbol has a version, and it is the default version, we
     create an indirect symbol from the default name to the fully
     decorated name.  This will cause external references which do not
     specify a version to be bound to this version of the symbol.  */
  p = strchr (name, ELF_VER_CHR);
  if (h->versioned == unknown)
    {
      if (p == NULL)
	{
	  h->versioned = unversioned;
	  return true;
	}
      else
	{
	  if (p[1] != ELF_VER_CHR)
	    {
	      h->versioned = versioned_hidden;
	      return true;
	    }
	  else
	    h->versioned = versioned;
	}
    }
  else
    {
      /* PR ld/19073: We may see an unversioned definition after the
	 default version.  */
      if (p == NULL)
	return true;
    }

  bed = get_elf_backend_data (abfd);
  collect = bed->collect;
  dynamic = (abfd->flags & DYNAMIC) != 0;

  shortlen = p - name;
  shortname = (char *) bfd_hash_allocate (&info->hash->table, shortlen + 1);
  if (shortname == NULL)
    return false;
  memcpy (shortname, name, shortlen);
  shortname[shortlen] = '\0';

  /* We are going to create a new symbol.  Merge it with any existing
     symbol with this name.  For the purposes of the merge, act as
     though we were defining the symbol we just defined, although we
     actually going to define an indirect symbol.  */
  type_change_ok = false;
  size_change_ok = false;
  matched = true;
  tmp_sec = sec;
  if (!_bfd_elf_merge_symbol (abfd, info, shortname, sym, &tmp_sec, &value,
			      &hi, poldbfd, NULL, NULL, &skip, &override,
			      &type_change_ok, &size_change_ok, &matched))
    return false;

  if (skip)
    goto nondefault;

  if (hi->def_regular || ELF_COMMON_DEF_P (hi))
    {
      /* If the undecorated symbol will have a version added by a
	 script different to H, then don't indirect to/from the
	 undecorated symbol.  This isn't ideal because we may not yet
	 have seen symbol versions, if given by a script on the
	 command line rather than via --version-script.  */
      if (hi->verinfo.vertree == NULL && info->version_info != NULL)
	{
	  bool hide;

	  hi->verinfo.vertree
	    = bfd_find_version_for_sym (info->version_info,
					hi->root.root.string, &hide);
	  if (hi->verinfo.vertree != NULL && hide)
	    {
	      (*bed->elf_backend_hide_symbol) (info, hi, true);
	      goto nondefault;
	    }
	}
      if (hi->verinfo.vertree != NULL
	  && strcmp (p + 1 + (p[1] == '@'), hi->verinfo.vertree->name) != 0)
	goto nondefault;
    }

  if (! override)
    {
      /* Add the default symbol if not performing a relocatable link.  */
      if (! bfd_link_relocatable (info))
	{
	  bh = &hi->root;
	  if (bh->type == bfd_link_hash_defined
	      && bh->u.def.section->owner != NULL
	      && (bh->u.def.section->owner->flags & BFD_PLUGIN) != 0)
	    {
	      /* Mark the previous definition from IR object as
		 undefined so that the generic linker will override
		 it.  */
	      bh->type = bfd_link_hash_undefined;
	      bh->u.undef.abfd = bh->u.def.section->owner;
	    }
	  if (! (_bfd_generic_link_add_one_symbol
		 (info, abfd, shortname, BSF_INDIRECT,
		  bfd_ind_section_ptr,
		  0, name, false, collect, &bh)))
	    return false;
	  hi = (struct elf_link_hash_entry *) bh;
	}
    }
  else
    {
      /* In this case the symbol named SHORTNAME is overriding the
	 indirect symbol we want to add.  We were planning on making
	 SHORTNAME an indirect symbol referring to NAME.  SHORTNAME
	 is the name without a version.  NAME is the fully versioned
	 name, and it is the default version.

	 Overriding means that we already saw a definition for the
	 symbol SHORTNAME in a regular object, and it is overriding
	 the symbol defined in the dynamic object.

	 When this happens, we actually want to change NAME, the
	 symbol we just added, to refer to SHORTNAME.  This will cause
	 references to NAME in the shared object to become references
	 to SHORTNAME in the regular object.  This is what we expect
	 when we override a function in a shared object: that the
	 references in the shared object will be mapped to the
	 definition in the regular object.  */

      while (hi->root.type == bfd_link_hash_indirect
	     || hi->root.type == bfd_link_hash_warning)
	hi = (struct elf_link_hash_entry *) hi->root.u.i.link;

      h->root.type = bfd_link_hash_indirect;
      h->root.u.i.link = (struct bfd_link_hash_entry *) hi;
      if (h->def_dynamic)
	{
	  h->def_dynamic = 0;
	  hi->ref_dynamic = 1;
	  if (hi->ref_regular
	      || hi->def_regular)
	    {
	      if (! bfd_elf_link_record_dynamic_symbol (info, hi))
		return false;
	    }
	}

      /* Now set HI to H, so that the following code will set the
	 other fields correctly.  */
      hi = h;
    }

  /* Check if HI is a warning symbol.  */
  if (hi->root.type == bfd_link_hash_warning)
    hi = (struct elf_link_hash_entry *) hi->root.u.i.link;

  /* If there is a duplicate definition somewhere, then HI may not
     point to an indirect symbol.  We will have reported an error to
     the user in that case.  */

  if (hi->root.type == bfd_link_hash_indirect)
    {
      struct elf_link_hash_entry *ht;

      ht = (struct elf_link_hash_entry *) hi->root.u.i.link;
      (*bed->elf_backend_copy_indirect_symbol) (info, ht, hi);

      /* If we first saw a reference to SHORTNAME with non-default
	 visibility, merge that visibility to the @@VER symbol.  */
      elf_merge_st_other (abfd, ht, hi->other, sec, true, dynamic);

      /* A reference to the SHORTNAME symbol from a dynamic library
	 will be satisfied by the versioned symbol at runtime.  In
	 effect, we have a reference to the versioned symbol.  */
      ht->ref_dynamic_nonweak |= hi->ref_dynamic_nonweak;
      hi->dynamic_def |= ht->dynamic_def;

      /* See if the new flags lead us to realize that the symbol must
	 be dynamic.  */
      if (! *dynsym)
	{
	  if (! dynamic)
	    {
	      if (! bfd_link_executable (info)
		  || hi->def_dynamic
		  || hi->ref_dynamic)
		*dynsym = true;
	    }
	  else
	    {
	      if (hi->ref_regular)
		*dynsym = true;
	    }
	}
    }

  /* We also need to define an indirection from the nondefault version
     of the symbol.  */

 nondefault:
  len = strlen (name);
  shortname = (char *) bfd_hash_allocate (&info->hash->table, len);
  if (shortname == NULL)
    return false;
  memcpy (shortname, name, shortlen);
  memcpy (shortname + shortlen, p + 1, len - shortlen);

  /* Once again, merge with any existing symbol.  */
  type_change_ok = false;
  size_change_ok = false;
  tmp_sec = sec;
  if (!_bfd_elf_merge_symbol (abfd, info, shortname, sym, &tmp_sec, &value,
			      &hi, poldbfd, NULL, NULL, &skip, &override,
			      &type_change_ok, &size_change_ok, &matched))
    return false;

  if (skip)
    {
      if (!dynamic
	  && h->root.type == bfd_link_hash_defweak
	  && hi->root.type == bfd_link_hash_defined)
	{
	  /* We are handling a weak sym@@ver and attempting to define
	     a weak sym@ver, but _bfd_elf_merge_symbol said to skip the
	     new weak sym@ver because there is already a strong sym@ver.
	     However, sym@ver and sym@@ver are really the same symbol.
	     The existing strong sym@ver ought to override sym@@ver.  */
	  h->root.type = bfd_link_hash_defined;
	  h->root.u.def.section = hi->root.u.def.section;
	  h->root.u.def.value = hi->root.u.def.value;
	  hi->root.type = bfd_link_hash_indirect;
	  hi->root.u.i.link = &h->root;
	}
      else
	return true;
    }
  else if (override)
    {
      /* Here SHORTNAME is a versioned name, so we don't expect to see
	 the type of override we do in the case above unless it is
	 overridden by a versioned definition.  */
      if (hi->root.type != bfd_link_hash_defined
	  && hi->root.type != bfd_link_hash_defweak)
	_bfd_error_handler
	  /* xgettext:c-format */
	  (_("%pB: unexpected redefinition of indirect versioned symbol `%s'"),
	   abfd, shortname);
      return true;
    }
  else
    {
      bh = &hi->root;
      if (! (_bfd_generic_link_add_one_symbol
	     (info, abfd, shortname, BSF_INDIRECT,
	      bfd_ind_section_ptr, 0, name, false, collect, &bh)))
	return false;
      hi = (struct elf_link_hash_entry *) bh;
    }

  /* If there is a duplicate definition somewhere, then HI may not
     point to an indirect symbol.  We will have reported an error
     to the user in that case.  */
  if (hi->root.type == bfd_link_hash_indirect)
    {
      (*bed->elf_backend_copy_indirect_symbol) (info, h, hi);
      h->ref_dynamic_nonweak |= hi->ref_dynamic_nonweak;
      hi->dynamic_def |= h->dynamic_def;

      /* If we first saw a reference to @VER symbol with
	 non-default visibility, merge that visibility to the
	 @@VER symbol.  */
      elf_merge_st_other (abfd, h, hi->other, sec, true, dynamic);

      /* See if the new flags lead us to realize that the symbol
	 must be dynamic.  */
      if (! *dynsym)
	{
	  if (! dynamic)
	    {
	      if (! bfd_link_executable (info)
		  || hi->ref_dynamic)
		*dynsym = true;
	    }
	  else
	    {
	      if (hi->ref_regular)
		*dynsym = true;
	    }
	}
    }

  return true;
}

/* This routine is used to export all defined symbols into the dynamic
   symbol table.  It is called via elf_link_hash_traverse.  */

static bool
_bfd_elf_export_symbol (struct elf_link_hash_entry *h, void *data)
{
  struct elf_info_failed *eif = (struct elf_info_failed *) data;

  /* Ignore indirect symbols.  These are added by the versioning code.  */
  if (h->root.type == bfd_link_hash_indirect)
    return true;

  /* Ignore this if we won't export it.  */
  if (!eif->info->export_dynamic && !h->dynamic)
    return true;

  if (h->dynindx == -1
      && (h->def_regular || h->ref_regular)
      && ! bfd_hide_sym_by_version (eif->info->version_info,
				    h->root.root.string))
    {
      if (! bfd_elf_link_record_dynamic_symbol (eif->info, h))
	{
	  eif->failed = true;
	  return false;
	}
    }

  return true;
}

/* Return true if GLIBC_ABI_DT_RELR is added to the list of version
   dependencies successfully.  GLIBC_ABI_DT_RELR will be put into the
   .gnu.version_r section.  */

static bool
elf_link_add_dt_relr_dependency (struct elf_find_verdep_info *rinfo)
{
  bfd *glibc_bfd = NULL;
  Elf_Internal_Verneed *t;
  Elf_Internal_Vernaux *a;
  size_t amt;
  const char *relr = "GLIBC_ABI_DT_RELR";

  /* See if we already know about GLIBC_PRIVATE_DT_RELR.  */
  for (t = elf_tdata (rinfo->info->output_bfd)->verref;
       t != NULL;
       t = t->vn_nextref)
    {
      const char *soname = bfd_elf_get_dt_soname (t->vn_bfd);
      /* Skip the shared library if it isn't libc.so.  */
      if (!soname || !startswith (soname, "libc.so."))
	continue;

      for (a = t->vn_auxptr; a != NULL; a = a->vna_nextptr)
	{
	  /* Return if GLIBC_PRIVATE_DT_RELR dependency has been
	     added.  */
	  if (a->vna_nodename == relr
	      || strcmp (a->vna_nodename, relr) == 0)
	    return true;

	  /* Check if libc.so provides GLIBC_2.XX version.  */
	  if (!glibc_bfd && startswith (a->vna_nodename, "GLIBC_2."))
	    glibc_bfd = t->vn_bfd;
	}

      break;
    }

  /* Skip if it isn't linked against glibc.  */
  if (glibc_bfd == NULL)
    return true;

  /* This is a new version.  Add it to tree we are building.  */
  if (t == NULL)
    {
      amt = sizeof *t;
      t = (Elf_Internal_Verneed *) bfd_zalloc (rinfo->info->output_bfd,
					       amt);
      if (t == NULL)
	{
	  rinfo->failed = true;
	  return false;
	}

      t->vn_bfd = glibc_bfd;
      t->vn_nextref = elf_tdata (rinfo->info->output_bfd)->verref;
      elf_tdata (rinfo->info->output_bfd)->verref = t;
    }

  amt = sizeof *a;
  a = (Elf_Internal_Vernaux *) bfd_zalloc (rinfo->info->output_bfd, amt);
  if (a == NULL)
    {
      rinfo->failed = true;
      return false;
    }

  a->vna_nodename = relr;
  a->vna_flags = 0;
  a->vna_nextptr = t->vn_auxptr;
  a->vna_other = rinfo->vers + 1;
  ++rinfo->vers;

  t->vn_auxptr = a;

  return true;
}

/* Look through the symbols which are defined in other shared
   libraries and referenced here.  Update the list of version
   dependencies.  This will be put into the .gnu.version_r section.
   This function is called via elf_link_hash_traverse.  */

static bool
_bfd_elf_link_find_version_dependencies (struct elf_link_hash_entry *h,
					 void *data)
{
  struct elf_find_verdep_info *rinfo = (struct elf_find_verdep_info *) data;
  Elf_Internal_Verneed *t;
  Elf_Internal_Vernaux *a;
  size_t amt;

  /* We only care about symbols defined in shared objects with version
     information.  */
  if (!h->def_dynamic
      || h->def_regular
      || h->dynindx == -1
      || h->verinfo.verdef == NULL
      || (elf_dyn_lib_class (h->verinfo.verdef->vd_bfd)
	  & (DYN_AS_NEEDED | DYN_DT_NEEDED | DYN_NO_NEEDED)))
    return true;

  /* See if we already know about this version.  */
  for (t = elf_tdata (rinfo->info->output_bfd)->verref;
       t != NULL;
       t = t->vn_nextref)
    {
      if (t->vn_bfd != h->verinfo.verdef->vd_bfd)
	continue;

      for (a = t->vn_auxptr; a != NULL; a = a->vna_nextptr)
	if (a->vna_nodename == h->verinfo.verdef->vd_nodename)
	  return true;

      break;
    }

  /* This is a new version.  Add it to tree we are building.  */

  if (t == NULL)
    {
      amt = sizeof *t;
      t = (Elf_Internal_Verneed *) bfd_zalloc (rinfo->info->output_bfd, amt);
      if (t == NULL)
	{
	  rinfo->failed = true;
	  return false;
	}

      t->vn_bfd = h->verinfo.verdef->vd_bfd;
      t->vn_nextref = elf_tdata (rinfo->info->output_bfd)->verref;
      elf_tdata (rinfo->info->output_bfd)->verref = t;
    }

  amt = sizeof *a;
  a = (Elf_Internal_Vernaux *) bfd_zalloc (rinfo->info->output_bfd, amt);
  if (a == NULL)
    {
      rinfo->failed = true;
      return false;
    }

  /* Note that we are copying a string pointer here, and testing it
     above.  If bfd_elf_string_from_elf_section is ever changed to
     discard the string data when low in memory, this will have to be
     fixed.  */
  a->vna_nodename = h->verinfo.verdef->vd_nodename;

  a->vna_flags = h->verinfo.verdef->vd_flags;
  a->vna_nextptr = t->vn_auxptr;

  h->verinfo.verdef->vd_exp_refno = rinfo->vers;
  ++rinfo->vers;

  a->vna_other = h->verinfo.verdef->vd_exp_refno + 1;

  t->vn_auxptr = a;

  return true;
}

/* Return TRUE and set *HIDE to TRUE if the versioned symbol is
   hidden.  Set *T_P to NULL if there is no match.  */

static bool
_bfd_elf_link_hide_versioned_symbol (struct bfd_link_info *info,
				     struct elf_link_hash_entry *h,
				     const char *version_p,
				     struct bfd_elf_version_tree **t_p,
				     bool *hide)
{
  struct bfd_elf_version_tree *t;

  /* Look for the version.  If we find it, it is no longer weak.  */
  for (t = info->version_info; t != NULL; t = t->next)
    {
      if (strcmp (t->name, version_p) == 0)
	{
	  size_t len;
	  char *alc;
	  struct bfd_elf_version_expr *d;

	  len = version_p - h->root.root.string;
	  alc = (char *) bfd_malloc (len);
	  if (alc == NULL)
	    return false;
	  memcpy (alc, h->root.root.string, len - 1);
	  alc[len - 1] = '\0';
	  if (alc[len - 2] == ELF_VER_CHR)
	    alc[len - 2] = '\0';

	  h->verinfo.vertree = t;
	  t->used = true;
	  d = NULL;

	  if (t->globals.list != NULL)
	    d = (*t->match) (&t->globals, NULL, alc);

	  /* See if there is anything to force this symbol to
	     local scope.  */
	  if (d == NULL && t->locals.list != NULL)
	    {
	      d = (*t->match) (&t->locals, NULL, alc);
	      if (d != NULL
		  && h->dynindx != -1
		  && ! info->export_dynamic)
		*hide = true;
	    }

	  free (alc);
	  break;
	}
    }

  *t_p = t;

  return true;
}

/* Return TRUE if the symbol H is hidden by version script.  */

bool
_bfd_elf_link_hide_sym_by_version (struct bfd_link_info *info,
				   struct elf_link_hash_entry *h)
{
  const char *p;
  bool hide = false;
  const struct elf_backend_data *bed
    = get_elf_backend_data (info->output_bfd);

  /* Version script only hides symbols defined in regular objects.  */
  if (!h->def_regular && !ELF_COMMON_DEF_P (h))
    return true;

  p = strchr (h->root.root.string, ELF_VER_CHR);
  if (p != NULL && h->verinfo.vertree == NULL)
    {
      struct bfd_elf_version_tree *t;

      ++p;
      if (*p == ELF_VER_CHR)
	++p;

      if (*p != '\0'
	  && _bfd_elf_link_hide_versioned_symbol (info, h, p, &t, &hide)
	  && hide)
	{
	  if (hide)
	    (*bed->elf_backend_hide_symbol) (info, h, true);
	  return true;
	}
    }

  /* If we don't have a version for this symbol, see if we can find
     something.  */
  if (h->verinfo.vertree == NULL && info->version_info != NULL)
    {
      h->verinfo.vertree
	= bfd_find_version_for_sym (info->version_info,
				    h->root.root.string, &hide);
      if (h->verinfo.vertree != NULL && hide)
	{
	  (*bed->elf_backend_hide_symbol) (info, h, true);
	  return true;
	}
    }

  return false;
}

/* Figure out appropriate versions for all the symbols.  We may not
   have the version number script until we have read all of the input
   files, so until that point we don't know which symbols should be
   local.  This function is called via elf_link_hash_traverse.  */

static bool
_bfd_elf_link_assign_sym_version (struct elf_link_hash_entry *h, void *data)
{
  struct elf_info_failed *sinfo;
  struct bfd_link_info *info;
  const struct elf_backend_data *bed;
  struct elf_info_failed eif;
  char *p;
  bool hide;

  sinfo = (struct elf_info_failed *) data;
  info = sinfo->info;

  /* Fix the symbol flags.  */
  eif.failed = false;
  eif.info = info;
  if (! _bfd_elf_fix_symbol_flags (h, &eif))
    {
      if (eif.failed)
	sinfo->failed = true;
      return false;
    }

  bed = get_elf_backend_data (info->output_bfd);

  /* We only need version numbers for symbols defined in regular
     objects.  */
  if (!h->def_regular && !ELF_COMMON_DEF_P (h))
    {
      /* Hide symbols defined in discarded input sections.  */
      if ((h->root.type == bfd_link_hash_defined
	   || h->root.type == bfd_link_hash_defweak)
	  && discarded_section (h->root.u.def.section))
	(*bed->elf_backend_hide_symbol) (info, h, true);
      return true;
    }

  hide = false;
  p = strchr (h->root.root.string, ELF_VER_CHR);
  if (p != NULL && h->verinfo.vertree == NULL)
    {
      struct bfd_elf_version_tree *t;

      ++p;
      if (*p == ELF_VER_CHR)
	++p;

      /* If there is no version string, we can just return out.  */
      if (*p == '\0')
	return true;

      if (!_bfd_elf_link_hide_versioned_symbol (info, h, p, &t, &hide))
	{
	  sinfo->failed = true;
	  return false;
	}

      if (hide)
	(*bed->elf_backend_hide_symbol) (info, h, true);

      /* If we are building an application, we need to create a
	 version node for this version.  */
      if (t == NULL && bfd_link_executable (info))
	{
	  struct bfd_elf_version_tree **pp;
	  int version_index;

	  /* If we aren't going to export this symbol, we don't need
	     to worry about it.  */
	  if (h->dynindx == -1)
	    return true;

	  t = (struct bfd_elf_version_tree *) bfd_zalloc (info->output_bfd,
							  sizeof *t);
	  if (t == NULL)
	    {
	      sinfo->failed = true;
	      return false;
	    }

	  t->name = p;
	  t->name_indx = (unsigned int) -1;
	  t->used = true;

	  version_index = 1;
	  /* Don't count anonymous version tag.  */
	  if (sinfo->info->version_info != NULL
	      && sinfo->info->version_info->vernum == 0)
	    version_index = 0;
	  for (pp = &sinfo->info->version_info;
	       *pp != NULL;
	       pp = &(*pp)->next)
	    ++version_index;
	  t->vernum = version_index;

	  *pp = t;

	  h->verinfo.vertree = t;
	}
      else if (t == NULL)
	{
	  /* We could not find the version for a symbol when
	     generating a shared archive.  Return an error.  */
	  _bfd_error_handler
	    /* xgettext:c-format */
	    (_("%pB: version node not found for symbol %s"),
	     info->output_bfd, h->root.root.string);
	  bfd_set_error (bfd_error_bad_value);
	  sinfo->failed = true;
	  return false;
	}
    }

  /* If we don't have a version for this symbol, see if we can find
     something.  */
  if (!hide
      && h->verinfo.vertree == NULL
      && sinfo->info->version_info != NULL)
    {
      h->verinfo.vertree
	= bfd_find_version_for_sym (sinfo->info->version_info,
				    h->root.root.string, &hide);
      if (h->verinfo.vertree != NULL && hide)
	(*bed->elf_backend_hide_symbol) (info, h, true);
    }

  return true;
}

/* Read and swap the relocs from the section indicated by SHDR.  This
   may be either a REL or a RELA section.  The relocations are
   translated into RELA relocations and stored in INTERNAL_RELOCS,
   which should have already been allocated to contain enough space.
   The EXTERNAL_RELOCS are a buffer where the external form of the
   relocations should be stored.

   Returns FALSE if something goes wrong.  */

static bool
elf_link_read_relocs_from_section (bfd *abfd,
				   asection *sec,
				   Elf_Internal_Shdr *shdr,
				   void *external_relocs,
				   Elf_Internal_Rela *internal_relocs)
{
  const struct elf_backend_data *bed;
  void (*swap_in) (bfd *, const bfd_byte *, Elf_Internal_Rela *);
  const bfd_byte *erela;
  const bfd_byte *erelaend;
  Elf_Internal_Rela *irela;
  Elf_Internal_Shdr *symtab_hdr;
  size_t nsyms;

  /* Position ourselves at the start of the section.  */
  if (bfd_seek (abfd, shdr->sh_offset, SEEK_SET) != 0)
    return false;

  /* Read the relocations.  */
  if (bfd_bread (external_relocs, shdr->sh_size, abfd) != shdr->sh_size)
    return false;

  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
  nsyms = NUM_SHDR_ENTRIES (symtab_hdr);

  bed = get_elf_backend_data (abfd);

  /* Convert the external relocations to the internal format.  */
  if (shdr->sh_entsize == bed->s->sizeof_rel)
    swap_in = bed->s->swap_reloc_in;
  else if (shdr->sh_entsize == bed->s->sizeof_rela)
    swap_in = bed->s->swap_reloca_in;
  else
    {
      bfd_set_error (bfd_error_wrong_format);
      return false;
    }

  erela = (const bfd_byte *) external_relocs;
  /* Setting erelaend like this and comparing with <= handles case of
     a fuzzed object with sh_size not a multiple of sh_entsize.  */
  erelaend = erela + shdr->sh_size - shdr->sh_entsize;
  irela = internal_relocs;
  while (erela <= erelaend)
    {
      bfd_vma r_symndx;

      (*swap_in) (abfd, erela, irela);
      r_symndx = ELF32_R_SYM (irela->r_info);
      if (bed->s->arch_size == 64)
	r_symndx >>= 24;
      if (nsyms > 0)
	{
	  if ((size_t) r_symndx >= nsyms)
	    {
	      _bfd_error_handler
		/* xgettext:c-format */
		(_("%pB: bad reloc symbol index (%#" PRIx64 " >= %#lx)"
		   " for offset %#" PRIx64 " in section `%pA'"),
		 abfd, (uint64_t) r_symndx, (unsigned long) nsyms,
		 (uint64_t) irela->r_offset, sec);
	      bfd_set_error (bfd_error_bad_value);
	      return false;
	    }
	}
      else if (r_symndx != STN_UNDEF)
	{
	  _bfd_error_handler
	    /* xgettext:c-format */
	    (_("%pB: non-zero symbol index (%#" PRIx64 ")"
	       " for offset %#" PRIx64 " in section `%pA'"
	       " when the object file has no symbol table"),
	     abfd, (uint64_t) r_symndx,
	     (uint64_t) irela->r_offset, sec);
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}
      irela += bed->s->int_rels_per_ext_rel;
      erela += shdr->sh_entsize;
    }

  return true;
}

/* Read and swap the relocs for a section O.  They may have been
   cached.  If the EXTERNAL_RELOCS and INTERNAL_RELOCS arguments are
   not NULL, they are used as buffers to read into.  They are known to
   be large enough.  If the INTERNAL_RELOCS relocs argument is NULL,
   the return value is allocated using either malloc or bfd_alloc,
   according to the KEEP_MEMORY argument.  If O has two relocation
   sections (both REL and RELA relocations), then the REL_HDR
   relocations will appear first in INTERNAL_RELOCS, followed by the
   RELA_HDR relocations.  If INFO isn't NULL and KEEP_MEMORY is true,
   update cache_size.  */

Elf_Internal_Rela *
_bfd_elf_link_info_read_relocs (bfd *abfd,
				struct bfd_link_info *info,
				asection *o,
				void *external_relocs,
				Elf_Internal_Rela *internal_relocs,
				bool keep_memory)
{
  void *alloc1 = NULL;
  Elf_Internal_Rela *alloc2 = NULL;
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  struct bfd_elf_section_data *esdo = elf_section_data (o);
  Elf_Internal_Rela *internal_rela_relocs;

  if (esdo->relocs != NULL)
    return esdo->relocs;

  if (o->reloc_count == 0)
    return NULL;

  if (internal_relocs == NULL)
    {
      bfd_size_type size;

      size = (bfd_size_type) o->reloc_count * sizeof (Elf_Internal_Rela);
      if (keep_memory)
	{
	  internal_relocs = alloc2 = (Elf_Internal_Rela *) bfd_alloc (abfd, size);
	  if (info)
	    info->cache_size += size;
	}
      else
	internal_relocs = alloc2 = (Elf_Internal_Rela *) bfd_malloc (size);
      if (internal_relocs == NULL)
	goto error_return;
    }

  if (external_relocs == NULL)
    {
      bfd_size_type size = 0;

      if (esdo->rel.hdr)
	size += esdo->rel.hdr->sh_size;
      if (esdo->rela.hdr)
	size += esdo->rela.hdr->sh_size;

      alloc1 = bfd_malloc (size);
      if (alloc1 == NULL)
	goto error_return;
      external_relocs = alloc1;
    }

  internal_rela_relocs = internal_relocs;
  if (esdo->rel.hdr)
    {
      if (!elf_link_read_relocs_from_section (abfd, o, esdo->rel.hdr,
					      external_relocs,
					      internal_relocs))
	goto error_return;
      external_relocs = (((bfd_byte *) external_relocs)
			 + esdo->rel.hdr->sh_size);
      internal_rela_relocs += (NUM_SHDR_ENTRIES (esdo->rel.hdr)
			       * bed->s->int_rels_per_ext_rel);
    }

  if (esdo->rela.hdr
      && (!elf_link_read_relocs_from_section (abfd, o, esdo->rela.hdr,
					      external_relocs,
					      internal_rela_relocs)))
    goto error_return;

  /* Cache the results for next time, if we can.  */
  if (keep_memory)
    esdo->relocs = internal_relocs;

  free (alloc1);

  /* Don't free alloc2, since if it was allocated we are passing it
     back (under the name of internal_relocs).  */

  return internal_relocs;

 error_return:
  free (alloc1);
  if (alloc2 != NULL)
    {
      if (keep_memory)
	bfd_release (abfd, alloc2);
      else
	free (alloc2);
    }
  return NULL;
}

/* This is similar to _bfd_elf_link_info_read_relocs, except for that
   NULL is passed to _bfd_elf_link_info_read_relocs for pointer to
   struct bfd_link_info.  */

Elf_Internal_Rela *
_bfd_elf_link_read_relocs (bfd *abfd,
			   asection *o,
			   void *external_relocs,
			   Elf_Internal_Rela *internal_relocs,
			   bool keep_memory)
{
  return _bfd_elf_link_info_read_relocs (abfd, NULL, o, external_relocs,
					 internal_relocs, keep_memory);

}

/* Compute the size of, and allocate space for, REL_HDR which is the
   section header for a section containing relocations for O.  */

static bool
_bfd_elf_link_size_reloc_section (bfd *abfd,
				  struct bfd_elf_section_reloc_data *reldata)
{
  Elf_Internal_Shdr *rel_hdr = reldata->hdr;

  /* That allows us to calculate the size of the section.  */
  rel_hdr->sh_size = rel_hdr->sh_entsize * reldata->count;

  /* The contents field must last into write_object_contents, so we
     allocate it with bfd_alloc rather than malloc.  Also since we
     cannot be sure that the contents will actually be filled in,
     we zero the allocated space.  */
  rel_hdr->contents = (unsigned char *) bfd_zalloc (abfd, rel_hdr->sh_size);
  if (rel_hdr->contents == NULL && rel_hdr->sh_size != 0)
    return false;

  if (reldata->hashes == NULL && reldata->count)
    {
      struct elf_link_hash_entry **p;

      p = ((struct elf_link_hash_entry **)
	   bfd_zmalloc (reldata->count * sizeof (*p)));
      if (p == NULL)
	return false;

      reldata->hashes = p;
    }

  return true;
}

/* Copy the relocations indicated by the INTERNAL_RELOCS (which
   originated from the section given by INPUT_REL_HDR) to the
   OUTPUT_BFD.  */

bool
_bfd_elf_link_output_relocs (bfd *output_bfd,
			     asection *input_section,
			     Elf_Internal_Shdr *input_rel_hdr,
			     Elf_Internal_Rela *internal_relocs,
			     struct elf_link_hash_entry **rel_hash
			       ATTRIBUTE_UNUSED)
{
  Elf_Internal_Rela *irela;
  Elf_Internal_Rela *irelaend;
  bfd_byte *erel;
  struct bfd_elf_section_reloc_data *output_reldata;
  asection *output_section;
  const struct elf_backend_data *bed;
  void (*swap_out) (bfd *, const Elf_Internal_Rela *, bfd_byte *);
  struct bfd_elf_section_data *esdo;

  output_section = input_section->output_section;

  bed = get_elf_backend_data (output_bfd);
  esdo = elf_section_data (output_section);
  if (esdo->rel.hdr && esdo->rel.hdr->sh_entsize == input_rel_hdr->sh_entsize)
    {
      output_reldata = &esdo->rel;
      swap_out = bed->s->swap_reloc_out;
    }
  else if (esdo->rela.hdr
	   && esdo->rela.hdr->sh_entsize == input_rel_hdr->sh_entsize)
    {
      output_reldata = &esdo->rela;
      swap_out = bed->s->swap_reloca_out;
    }
  else
    {
      _bfd_error_handler
	/* xgettext:c-format */
	(_("%pB: relocation size mismatch in %pB section %pA"),
	 output_bfd, input_section->owner, input_section);
      bfd_set_error (bfd_error_wrong_format);
      return false;
    }

  erel = output_reldata->hdr->contents;
  erel += output_reldata->count * input_rel_hdr->sh_entsize;
  irela = internal_relocs;
  irelaend = irela + (NUM_SHDR_ENTRIES (input_rel_hdr)
		      * bed->s->int_rels_per_ext_rel);
  while (irela < irelaend)
    {
      (*swap_out) (output_bfd, irela, erel);
      irela += bed->s->int_rels_per_ext_rel;
      erel += input_rel_hdr->sh_entsize;
    }

  /* Bump the counter, so that we know where to add the next set of
     relocations.  */
  output_reldata->count += NUM_SHDR_ENTRIES (input_rel_hdr);

  return true;
}

/* Make weak undefined symbols in PIE dynamic.  */

bool
_bfd_elf_link_hash_fixup_symbol (struct bfd_link_info *info,
				 struct elf_link_hash_entry *h)
{
  if (bfd_link_pie (info)
      && h->dynindx == -1
      && h->root.type == bfd_link_hash_undefweak)
    return bfd_elf_link_record_dynamic_symbol (info, h);

  return true;
}

/* Fix up the flags for a symbol.  This handles various cases which
   can only be fixed after all the input files are seen.  This is
   currently called by both adjust_dynamic_symbol and
   assign_sym_version, which is unnecessary but perhaps more robust in
   the face of future changes.  */

static bool
_bfd_elf_fix_symbol_flags (struct elf_link_hash_entry *h,
			   struct elf_info_failed *eif)
{
  const struct elf_backend_data *bed;

  /* If this symbol was mentioned in a non-ELF file, try to set
     DEF_REGULAR and REF_REGULAR correctly.  This is the only way to
     permit a non-ELF file to correctly refer to a symbol defined in
     an ELF dynamic object.  */
  if (h->non_elf)
    {
      while (h->root.type == bfd_link_hash_indirect)
	h = (struct elf_link_hash_entry *) h->root.u.i.link;

      if (h->root.type != bfd_link_hash_defined
	  && h->root.type != bfd_link_hash_defweak)
	{
	  h->ref_regular = 1;
	  h->ref_regular_nonweak = 1;
	}
      else
	{
	  if (h->root.u.def.section->owner != NULL
	      && (bfd_get_flavour (h->root.u.def.section->owner)
		  == bfd_target_elf_flavour))
	    {
	      h->ref_regular = 1;
	      h->ref_regular_nonweak = 1;
	    }
	  else
	    h->def_regular = 1;
	}

      if (h->dynindx == -1
	  && (h->def_dynamic
	      || h->ref_dynamic))
	{
	  if (! bfd_elf_link_record_dynamic_symbol (eif->info, h))
	    {
	      eif->failed = true;
	      return false;
	    }
	}
    }
  else
    {
      /* Unfortunately, NON_ELF is only correct if the symbol
	 was first seen in a non-ELF file.  Fortunately, if the symbol
	 was first seen in an ELF file, we're probably OK unless the
	 symbol was defined in a non-ELF file.  Catch that case here.
	 FIXME: We're still in trouble if the symbol was first seen in
	 a dynamic object, and then later in a non-ELF regular object.  */
      if ((h->root.type == bfd_link_hash_defined
	   || h->root.type == bfd_link_hash_defweak)
	  && !h->def_regular
	  && (h->root.u.def.section->owner != NULL
	      ? (bfd_get_flavour (h->root.u.def.section->owner)
		 != bfd_target_elf_flavour)
	      : (bfd_is_abs_section (h->root.u.def.section)
		 && !h->def_dynamic)))
	h->def_regular = 1;
    }

  /* Backend specific symbol fixup.  */
  bed = get_elf_backend_data (elf_hash_table (eif->info)->dynobj);
  if (bed->elf_backend_fixup_symbol
      && !(*bed->elf_backend_fixup_symbol) (eif->info, h))
    return false;

  /* If this is a final link, and the symbol was defined as a common
     symbol in a regular object file, and there was no definition in
     any dynamic object, then the linker will have allocated space for
     the symbol in a common section but the DEF_REGULAR
     flag will not have been set.  */
  if (h->root.type == bfd_link_hash_defined
      && !h->def_regular
      && h->ref_regular
      && !h->def_dynamic
      && (h->root.u.def.section->owner->flags & (DYNAMIC | BFD_PLUGIN)) == 0)
    h->def_regular = 1;

  /* Symbols defined in discarded sections shouldn't be dynamic.  */
  if (h->root.type == bfd_link_hash_undefined && h->indx == -3)
    (*bed->elf_backend_hide_symbol) (eif->info, h, true);

  /* If a weak undefined symbol has non-default visibility, we also
     hide it from the dynamic linker.  */
  else if (ELF_ST_VISIBILITY (h->other) != STV_DEFAULT
	   && h->root.type == bfd_link_hash_undefweak)
    (*bed->elf_backend_hide_symbol) (eif->info, h, true);

  /* A hidden versioned symbol in executable should be forced local if
     it is is locally defined, not referenced by shared library and not
     exported.  */
  else if (bfd_link_executable (eif->info)
	   && h->versioned == versioned_hidden
	   && !eif->info->export_dynamic
	   && !h->dynamic
	   && !h->ref_dynamic
	   && h->def_regular)
    (*bed->elf_backend_hide_symbol) (eif->info, h, true);

  /* If -Bsymbolic was used (which means to bind references to global
     symbols to the definition within the shared object), and this
     symbol was defined in a regular object, then it actually doesn't
     need a PLT entry.  Likewise, if the symbol has non-default
     visibility.  If the symbol has hidden or internal visibility, we
     will force it local.  */
  else if (h->needs_plt
	   && bfd_link_pic (eif->info)
	   && is_elf_hash_table (eif->info->hash)
	   && (SYMBOLIC_BIND (eif->info, h)
	       || ELF_ST_VISIBILITY (h->other) != STV_DEFAULT)
	   && h->def_regular)
    {
      bool force_local;

      force_local = (ELF_ST_VISIBILITY (h->other) == STV_INTERNAL
		     || ELF_ST_VISIBILITY (h->other) == STV_HIDDEN);
      (*bed->elf_backend_hide_symbol) (eif->info, h, force_local);
    }

  /* If this is a weak defined symbol in a dynamic object, and we know
     the real definition in the dynamic object, copy interesting flags
     over to the real definition.  */
  if (h->is_weakalias)
    {
      struct elf_link_hash_entry *def = weakdef (h);

      /* If the real definition is defined by a regular object file,
	 don't do anything special.  See the longer description in
	 _bfd_elf_adjust_dynamic_symbol, below.  If the def is not
	 bfd_link_hash_defined as it was when put on the alias list
	 then it must have originally been a versioned symbol (for
	 which a non-versioned indirect symbol is created) and later
	 a definition for the non-versioned symbol is found.  In that
	 case the indirection is flipped with the versioned symbol
	 becoming an indirect pointing at the non-versioned symbol.
	 Thus, not an alias any more.  */
      if (def->def_regular
	  || def->root.type != bfd_link_hash_defined)
	{
	  h = def;
	  while ((h = h->u.alias) != def)
	    h->is_weakalias = 0;
	}
      else
	{
	  while (h->root.type == bfd_link_hash_indirect)
	    h = (struct elf_link_hash_entry *) h->root.u.i.link;
	  BFD_ASSERT (h->root.type == bfd_link_hash_defined
		      || h->root.type == bfd_link_hash_defweak);
	  BFD_ASSERT (def->def_dynamic);
	  (*bed->elf_backend_copy_indirect_symbol) (eif->info, def, h);
	}
    }

  return true;
}

/* Make the backend pick a good value for a dynamic symbol.  This is
   called via elf_link_hash_traverse, and also calls itself
   recursively.  */

static bool
_bfd_elf_adjust_dynamic_symbol (struct elf_link_hash_entry *h, void *data)
{
  struct elf_info_failed *eif = (struct elf_info_failed *) data;
  struct elf_link_hash_table *htab;
  const struct elf_backend_data *bed;

  if (! is_elf_hash_table (eif->info->hash))
    return false;

  /* Ignore indirect symbols.  These are added by the versioning code.  */
  if (h->root.type == bfd_link_hash_indirect)
    return true;

  /* Fix the symbol flags.  */
  if (! _bfd_elf_fix_symbol_flags (h, eif))
    return false;

  htab = elf_hash_table (eif->info);
  bed = get_elf_backend_data (htab->dynobj);

  if (h->root.type == bfd_link_hash_undefweak)
    {
      if (eif->info->dynamic_undefined_weak == 0)
	(*bed->elf_backend_hide_symbol) (eif->info, h, true);
      else if (eif->info->dynamic_undefined_weak > 0
	       && h->ref_regular
	       && ELF_ST_VISIBILITY (h->other) == STV_DEFAULT
	       && !bfd_hide_sym_by_version (eif->info->version_info,
					    h->root.root.string))
	{
	  if (!bfd_elf_link_record_dynamic_symbol (eif->info, h))
	    {
	      eif->failed = true;
	      return false;
	    }
	}
    }

  /* If this symbol does not require a PLT entry, and it is not
     defined by a dynamic object, or is not referenced by a regular
     object, ignore it.  We do have to handle a weak defined symbol,
     even if no regular object refers to it, if we decided to add it
     to the dynamic symbol table.  FIXME: Do we normally need to worry
     about symbols which are defined by one dynamic object and
     referenced by another one?  */
  if (!h->needs_plt
      && h->type != STT_GNU_IFUNC
      && (h->def_regular
	  || !h->def_dynamic
	  || (!h->ref_regular
	      && (!h->is_weakalias || weakdef (h)->dynindx == -1))))
    {
      h->plt = elf_hash_table (eif->info)->init_plt_offset;
      return true;
    }

  /* If we've already adjusted this symbol, don't do it again.  This
     can happen via a recursive call.  */
  if (h->dynamic_adjusted)
    return true;

  /* Don't look at this symbol again.  Note that we must set this
     after checking the above conditions, because we may look at a
     symbol once, decide not to do anything, and then get called
     recursively later after REF_REGULAR is set below.  */
  h->dynamic_adjusted = 1;

  /* If this is a weak definition, and we know a real definition, and
     the real symbol is not itself defined by a regular object file,
     then get a good value for the real definition.  We handle the
     real symbol first, for the convenience of the backend routine.

     Note that there is a confusing case here.  If the real definition
     is defined by a regular object file, we don't get the real symbol
     from the dynamic object, but we do get the weak symbol.  If the
     processor backend uses a COPY reloc, then if some routine in the
     dynamic object changes the real symbol, we will not see that
     change in the corresponding weak symbol.  This is the way other
     ELF linkers work as well, and seems to be a result of the shared
     library model.

     I will clarify this issue.  Most SVR4 shared libraries define the
     variable _timezone and define timezone as a weak synonym.  The
     tzset call changes _timezone.  If you write
       extern int timezone;
       int _timezone = 5;
       int main () { tzset (); printf ("%d %d\n", timezone, _timezone); }
     you might expect that, since timezone is a synonym for _timezone,
     the same number will print both times.  However, if the processor
     backend uses a COPY reloc, then actually timezone will be copied
     into your process image, and, since you define _timezone
     yourself, _timezone will not.  Thus timezone and _timezone will
     wind up at different memory locations.  The tzset call will set
     _timezone, leaving timezone unchanged.  */

  if (h->is_weakalias)
    {
      struct elf_link_hash_entry *def = weakdef (h);

      /* If we get to this point, there is an implicit reference to
	 the alias by a regular object file via the weak symbol H.  */
      def->ref_regular = 1;

      /* Ensure that the backend adjust_dynamic_symbol function sees
	 the strong alias before H by recursively calling ourselves.  */
      if (!_bfd_elf_adjust_dynamic_symbol (def, eif))
	return false;
    }

  /* If a symbol has no type and no size and does not require a PLT
     entry, then we are probably about to do the wrong thing here: we
     are probably going to create a COPY reloc for an empty object.
     This case can arise when a shared object is built with assembly
     code, and the assembly code fails to set the symbol type.  */
  if (h->size == 0
      && h->type == STT_NOTYPE
      && !h->needs_plt)
    _bfd_error_handler
      (_("warning: type and size of dynamic symbol `%s' are not defined"),
       h->root.root.string);

  if (! (*bed->elf_backend_adjust_dynamic_symbol) (eif->info, h))
    {
      eif->failed = true;
      return false;
    }

  return true;
}

/* Adjust the dynamic symbol, H, for copy in the dynamic bss section,
   DYNBSS.  */

bool
_bfd_elf_adjust_dynamic_copy (struct bfd_link_info *info,
			      struct elf_link_hash_entry *h,
			      asection *dynbss)
{
  unsigned int power_of_two;
  bfd_vma mask;
  asection *sec = h->root.u.def.section;

  /* The section alignment of the definition is the maximum alignment
     requirement of symbols defined in the section.  Since we don't
     know the symbol alignment requirement, we start with the
     maximum alignment and check low bits of the symbol address
     for the minimum alignment.  */
  power_of_two = bfd_section_alignment (sec);
  mask = ((bfd_vma) 1 << power_of_two) - 1;
  while ((h->root.u.def.value & mask) != 0)
    {
       mask >>= 1;
       --power_of_two;
    }

  if (power_of_two > bfd_section_alignment (dynbss))
    {
      /* Adjust the section alignment if needed.  */
      if (!bfd_set_section_alignment (dynbss, power_of_two))
	return false;
    }

  /* We make sure that the symbol will be aligned properly.  */
  dynbss->size = BFD_ALIGN (dynbss->size, mask + 1);

  /* Define the symbol as being at this point in DYNBSS.  */
  h->root.u.def.section = dynbss;
  h->root.u.def.value = dynbss->size;

  /* Increment the size of DYNBSS to make room for the symbol.  */
  dynbss->size += h->size;

  /* No error if extern_protected_data is true.  */
  if (h->protected_def
      && (!info->extern_protected_data
	  || (info->extern_protected_data < 0
	      && !get_elf_backend_data (dynbss->owner)->extern_protected_data)))
    info->callbacks->einfo
      (_("%P: copy reloc against protected `%pT' is dangerous\n"),
       h->root.root.string);

  return true;
}

/* Adjust all external symbols pointing into SEC_MERGE sections
   to reflect the object merging within the sections.  */

static bool
_bfd_elf_link_sec_merge_syms (struct elf_link_hash_entry *h, void *data)
{
  asection *sec;

  if ((h->root.type == bfd_link_hash_defined
       || h->root.type == bfd_link_hash_defweak)
      && ((sec = h->root.u.def.section)->flags & SEC_MERGE)
      && sec->sec_info_type == SEC_INFO_TYPE_MERGE)
    {
      bfd *output_bfd = (bfd *) data;

      h->root.u.def.value =
	_bfd_merged_section_offset (output_bfd,
				    &h->root.u.def.section,
				    elf_section_data (sec)->sec_info,
				    h->root.u.def.value);
    }

  return true;
}

/* Returns false if the symbol referred to by H should be considered
   to resolve local to the current module, and true if it should be
   considered to bind dynamically.  */

bool
_bfd_elf_dynamic_symbol_p (struct elf_link_hash_entry *h,
			   struct bfd_link_info *info,
			   bool not_local_protected)
{
  bool binding_stays_local_p;
  const struct elf_backend_data *bed;
  struct elf_link_hash_table *hash_table;

  if (h == NULL)
    return false;

  while (h->root.type == bfd_link_hash_indirect
	 || h->root.type == bfd_link_hash_warning)
    h = (struct elf_link_hash_entry *) h->root.u.i.link;

  /* If it was forced local, then clearly it's not dynamic.  */
  if (h->dynindx == -1)
    return false;
  if (h->forced_local)
    return false;

  /* Identify the cases where name binding rules say that a
     visible symbol resolves locally.  */
  binding_stays_local_p = (bfd_link_executable (info)
			   || SYMBOLIC_BIND (info, h));

  switch (ELF_ST_VISIBILITY (h->other))
    {
    case STV_INTERNAL:
    case STV_HIDDEN:
      return false;

    case STV_PROTECTED:
      hash_table = elf_hash_table (info);
      if (!is_elf_hash_table (&hash_table->root))
	return false;

      bed = get_elf_backend_data (hash_table->dynobj);

      /* Proper resolution for function pointer equality may require
	 that these symbols perhaps be resolved dynamically, even though
	 we should be resolving them to the current module.  */
      if (!not_local_protected || !bed->is_function_type (h->type))
	binding_stays_local_p = true;
      break;

    default:
      break;
    }

  /* If it isn't defined locally, then clearly it's dynamic.  */
  if (!h->def_regular && !ELF_COMMON_DEF_P (h))
    return true;

  /* Otherwise, the symbol is dynamic if binding rules don't tell
     us that it remains local.  */
  return !binding_stays_local_p;
}

/* Return true if the symbol referred to by H should be considered
   to resolve local to the current module, and false otherwise.  Differs
   from (the inverse of) _bfd_elf_dynamic_symbol_p in the treatment of
   undefined symbols.  The two functions are virtually identical except
   for the place where dynindx == -1 is tested.  If that test is true,
   _bfd_elf_dynamic_symbol_p will say the symbol is local, while
   _bfd_elf_symbol_refs_local_p will say the symbol is local only for
   defined symbols.
   It might seem that _bfd_elf_dynamic_symbol_p could be rewritten as
   !_bfd_elf_symbol_refs_local_p, except that targets differ in their
   treatment of undefined weak symbols.  For those that do not make
   undefined weak symbols dynamic, both functions may return false.  */

bool
_bfd_elf_symbol_refs_local_p (struct elf_link_hash_entry *h,
			      struct bfd_link_info *info,
			      bool local_protected)
{
  const struct elf_backend_data *bed;
  struct elf_link_hash_table *hash_table;

  /* If it's a local sym, of course we resolve locally.  */
  if (h == NULL)
    return true;

  /* STV_HIDDEN or STV_INTERNAL ones must be local.  */
  if (ELF_ST_VISIBILITY (h->other) == STV_HIDDEN
      || ELF_ST_VISIBILITY (h->other) == STV_INTERNAL)
    return true;

  /* Forced local symbols resolve locally.  */
  if (h->forced_local)
    return true;

  /* Common symbols that become definitions don't get the DEF_REGULAR
     flag set, so test it first, and don't bail out.  */
  if (ELF_COMMON_DEF_P (h))
    /* Do nothing.  */;
  /* If we don't have a definition in a regular file, then we can't
     resolve locally.  The sym is either undefined or dynamic.  */
  else if (!h->def_regular)
    return false;

  /* Non-dynamic symbols resolve locally.  */
  if (h->dynindx == -1)
    return true;

  /* At this point, we know the symbol is defined and dynamic.  In an
     executable it must resolve locally, likewise when building symbolic
     shared libraries.  */
  if (bfd_link_executable (info) || SYMBOLIC_BIND (info, h))
    return true;

  /* Now deal with defined dynamic symbols in shared libraries.  Ones
     with default visibility might not resolve locally.  */
  if (ELF_ST_VISIBILITY (h->other) == STV_DEFAULT)
    return false;

  hash_table = elf_hash_table (info);
  if (!is_elf_hash_table (&hash_table->root))
    return true;

  /* STV_PROTECTED symbols with indirect external access are local. */
  if (info->indirect_extern_access > 0)
    return true;

  bed = get_elf_backend_data (hash_table->dynobj);

  /* If extern_protected_data is false, STV_PROTECTED non-function
     symbols are local.  */
  if ((!info->extern_protected_data
       || (info->extern_protected_data < 0
	   && !bed->extern_protected_data))
      && !bed->is_function_type (h->type))
    return true;

  /* Function pointer equality tests may require that STV_PROTECTED
     symbols be treated as dynamic symbols.  If the address of a
     function not defined in an executable is set to that function's
     plt entry in the executable, then the address of the function in
     a shared library must also be the plt entry in the executable.  */
  return local_protected;
}

/* Caches some TLS segment info, and ensures that the TLS segment vma is
   aligned.  Returns the first TLS output section.  */

struct bfd_section *
_bfd_elf_tls_setup (bfd *obfd, struct bfd_link_info *info)
{
  struct bfd_section *sec, *tls;
  unsigned int align = 0;

  for (sec = obfd->sections; sec != NULL; sec = sec->next)
    if ((sec->flags & SEC_THREAD_LOCAL) != 0)
      break;
  tls = sec;

  for (; sec != NULL && (sec->flags & SEC_THREAD_LOCAL) != 0; sec = sec->next)
    if (sec->alignment_power > align)
      align = sec->alignment_power;

  elf_hash_table (info)->tls_sec = tls;

  /* Ensure the alignment of the first section (usually .tdata) is the largest
     alignment, so that the tls segment starts aligned.  */
  if (tls != NULL)
    tls->alignment_power = align;

  return tls;
}

/* Return TRUE iff this is a non-common, definition of a non-function symbol.  */
static bool
is_global_data_symbol_definition (bfd *abfd ATTRIBUTE_UNUSED,
				  Elf_Internal_Sym *sym)
{
  const struct elf_backend_data *bed;

  /* Local symbols do not count, but target specific ones might.  */
  if (ELF_ST_BIND (sym->st_info) != STB_GLOBAL
      && ELF_ST_BIND (sym->st_info) < STB_LOOS)
    return false;

  bed = get_elf_backend_data (abfd);
  /* Function symbols do not count.  */
  if (bed->is_function_type (ELF_ST_TYPE (sym->st_info)))
    return false;

  /* If the section is undefined, then so is the symbol.  */
  if (sym->st_shndx == SHN_UNDEF)
    return false;

  /* If the symbol is defined in the common section, then
     it is a common definition and so does not count.  */
  if (bed->common_definition (sym))
    return false;

  /* If the symbol is in a target specific section then we
     must rely upon the backend to tell us what it is.  */
  if (sym->st_shndx >= SHN_LORESERVE && sym->st_shndx < SHN_ABS)
    /* FIXME - this function is not coded yet:

       return _bfd_is_global_symbol_definition (abfd, sym);

       Instead for now assume that the definition is not global,
       Even if this is wrong, at least the linker will behave
       in the same way that it used to do.  */
    return false;

  return true;
}

/* Search the symbol table of the archive element of the archive ABFD
   whose archive map contains a mention of SYMDEF, and determine if
   the symbol is defined in this element.  */
static bool
elf_link_is_defined_archive_symbol (bfd * abfd, carsym * symdef)
{
  Elf_Internal_Shdr * hdr;
  size_t symcount;
  size_t extsymcount;
  size_t extsymoff;
  Elf_Internal_Sym *isymbuf;
  Elf_Internal_Sym *isym;
  Elf_Internal_Sym *isymend;
  bool result;

  abfd = _bfd_get_elt_at_filepos (abfd, symdef->file_offset, NULL);
  if (abfd == NULL)
    return false;

  if (! bfd_check_format (abfd, bfd_object))
    return false;

  if (elf_use_dt_symtab_p (abfd))
    {
      bfd_set_error (bfd_error_wrong_format);
      return false;
    }

  /* Select the appropriate symbol table.  If we don't know if the
     object file is an IR object, give linker LTO plugin a chance to
     get the correct symbol table.  */
  if (abfd->plugin_format == bfd_plugin_yes
#if BFD_SUPPORTS_PLUGINS
      || (abfd->plugin_format == bfd_plugin_unknown
	  && bfd_link_plugin_object_p (abfd))
#endif
      )
    {
      /* Use the IR symbol table if the object has been claimed by
	 plugin.  */
      abfd = abfd->plugin_dummy_bfd;
      hdr = &elf_tdata (abfd)->symtab_hdr;
    }
  else if ((abfd->flags & DYNAMIC) == 0 || elf_dynsymtab (abfd) == 0)
    hdr = &elf_tdata (abfd)->symtab_hdr;
  else
    hdr = &elf_tdata (abfd)->dynsymtab_hdr;

  symcount = hdr->sh_size / get_elf_backend_data (abfd)->s->sizeof_sym;

  /* The sh_info field of the symtab header tells us where the
     external symbols start.  We don't care about the local symbols.  */
  if (elf_bad_symtab (abfd))
    {
      extsymcount = symcount;
      extsymoff = 0;
    }
  else
    {
      extsymcount = symcount - hdr->sh_info;
      extsymoff = hdr->sh_info;
    }

  if (extsymcount == 0)
    return false;

  /* Read in the symbol table.  */
  isymbuf = bfd_elf_get_elf_syms (abfd, hdr, extsymcount, extsymoff,
				  NULL, NULL, NULL);
  if (isymbuf == NULL)
    return false;

  /* Scan the symbol table looking for SYMDEF.  */
  result = false;
  for (isym = isymbuf, isymend = isymbuf + extsymcount; isym < isymend; isym++)
    {
      const char *name;

      name = bfd_elf_string_from_elf_section (abfd, hdr->sh_link,
					      isym->st_name);
      if (name == NULL)
	break;

      if (strcmp (name, symdef->name) == 0)
	{
	  result = is_global_data_symbol_definition (abfd, isym);
	  break;
	}
    }

  free (isymbuf);

  return result;
}

/* Add an entry to the .dynamic table.  */

bool
_bfd_elf_add_dynamic_entry (struct bfd_link_info *info,
			    bfd_vma tag,
			    bfd_vma val)
{
  struct elf_link_hash_table *hash_table;
  const struct elf_backend_data *bed;
  asection *s;
  bfd_size_type newsize;
  bfd_byte *newcontents;
  Elf_Internal_Dyn dyn;

  hash_table = elf_hash_table (info);
  if (! is_elf_hash_table (&hash_table->root))
    return false;

  if (tag == DT_RELA || tag == DT_REL)
    hash_table->dynamic_relocs = true;

  bed = get_elf_backend_data (hash_table->dynobj);
  s = bfd_get_linker_section (hash_table->dynobj, ".dynamic");
  BFD_ASSERT (s != NULL);

  newsize = s->size + bed->s->sizeof_dyn;
  newcontents = (bfd_byte *) bfd_realloc (s->contents, newsize);
  if (newcontents == NULL)
    return false;

  dyn.d_tag = tag;
  dyn.d_un.d_val = val;
  bed->s->swap_dyn_out (hash_table->dynobj, &dyn, newcontents + s->size);

  s->size = newsize;
  s->contents = newcontents;

  return true;
}

/* Strip zero-sized dynamic sections.  */

bool
_bfd_elf_strip_zero_sized_dynamic_sections (struct bfd_link_info *info)
{
  struct elf_link_hash_table *hash_table;
  const struct elf_backend_data *bed;
  asection *s, *sdynamic, **pp;
  asection *rela_dyn, *rel_dyn;
  Elf_Internal_Dyn dyn;
  bfd_byte *extdyn, *next;
  void (*swap_dyn_in) (bfd *, const void *, Elf_Internal_Dyn *);
  bool strip_zero_sized;
  bool strip_zero_sized_plt;

  if (bfd_link_relocatable (info))
    return true;

  hash_table = elf_hash_table (info);
  if (!is_elf_hash_table (&hash_table->root))
    return false;

  if (!hash_table->dynobj)
    return true;

  sdynamic= bfd_get_linker_section (hash_table->dynobj, ".dynamic");
  if (!sdynamic)
    return true;

  bed = get_elf_backend_data (hash_table->dynobj);
  swap_dyn_in = bed->s->swap_dyn_in;

  strip_zero_sized = false;
  strip_zero_sized_plt = false;

  /* Strip zero-sized dynamic sections.  */
  rela_dyn = bfd_get_section_by_name (info->output_bfd, ".rela.dyn");
  rel_dyn = bfd_get_section_by_name (info->output_bfd, ".rel.dyn");
  for (pp = &info->output_bfd->sections; (s = *pp) != NULL;)
    if (s->size == 0
	&& (s == rela_dyn
	    || s == rel_dyn
	    || s == hash_table->srelplt->output_section
	    || s == hash_table->splt->output_section))
      {
	*pp = s->next;
	info->output_bfd->section_count--;
	strip_zero_sized = true;
	if (s == rela_dyn)
	  s = rela_dyn;
	if (s == rel_dyn)
	  s = rel_dyn;
	else if (s == hash_table->splt->output_section)
	  {
	    s = hash_table->splt;
	    strip_zero_sized_plt = true;
	  }
	else
	  s = hash_table->srelplt;
	s->flags |= SEC_EXCLUDE;
	s->output_section = bfd_abs_section_ptr;
      }
    else
      pp = &s->next;

  if (strip_zero_sized_plt && sdynamic->size != 0)
    for (extdyn = sdynamic->contents;
	 extdyn < sdynamic->contents + sdynamic->size;
	 extdyn = next)
      {
	next = extdyn + bed->s->sizeof_dyn;
	swap_dyn_in (hash_table->dynobj, extdyn, &dyn);
	switch (dyn.d_tag)
	  {
	  default:
	    break;
	  case DT_JMPREL:
	  case DT_PLTRELSZ:
	  case DT_PLTREL:
	    /* Strip DT_PLTRELSZ, DT_JMPREL and DT_PLTREL entries if
	       the procedure linkage table (the .plt section) has been
	       removed.  */
	    memmove (extdyn, next,
		     sdynamic->size - (next - sdynamic->contents));
	    next = extdyn;
	  }
      }

  if (strip_zero_sized)
    {
      /* Regenerate program headers.  */
      elf_seg_map (info->output_bfd) = NULL;
      return _bfd_elf_map_sections_to_segments (info->output_bfd, info,
						NULL);
    }

  return true;
}

/* Add a DT_NEEDED entry for this dynamic object.  Returns -1 on error,
   1 if a DT_NEEDED tag already exists, and 0 on success.  */

int
bfd_elf_add_dt_needed_tag (bfd *abfd, struct bfd_link_info *info)
{
  struct elf_link_hash_table *hash_table;
  size_t strindex;
  const char *soname;

  if (!_bfd_elf_link_create_dynstrtab (abfd, info))
    return -1;

  hash_table = elf_hash_table (info);
  soname = elf_dt_name (abfd);
  strindex = _bfd_elf_strtab_add (hash_table->dynstr, soname, false);
  if (strindex == (size_t) -1)
    return -1;

  if (_bfd_elf_strtab_refcount (hash_table->dynstr, strindex) != 1)
    {
      asection *sdyn;
      const struct elf_backend_data *bed;
      bfd_byte *extdyn;

      bed = get_elf_backend_data (hash_table->dynobj);
      sdyn = bfd_get_linker_section (hash_table->dynobj, ".dynamic");
      if (sdyn != NULL && sdyn->size != 0)
	for (extdyn = sdyn->contents;
	     extdyn < sdyn->contents + sdyn->size;
	     extdyn += bed->s->sizeof_dyn)
	  {
	    Elf_Internal_Dyn dyn;

	    bed->s->swap_dyn_in (hash_table->dynobj, extdyn, &dyn);
	    if (dyn.d_tag == DT_NEEDED
		&& dyn.d_un.d_val == strindex)
	      {
		_bfd_elf_strtab_delref (hash_table->dynstr, strindex);
		return 1;
	      }
	  }
    }

  if (!_bfd_elf_link_create_dynamic_sections (hash_table->dynobj, info))
    return -1;

  if (!_bfd_elf_add_dynamic_entry (info, DT_NEEDED, strindex))
    return -1;

  return 0;
}

/* Return true if SONAME is on the needed list between NEEDED and STOP
   (or the end of list if STOP is NULL), and needed by a library that
   will be loaded.  */

static bool
on_needed_list (const char *soname,
		struct bfd_link_needed_list *needed,
		struct bfd_link_needed_list *stop)
{
  struct bfd_link_needed_list *look;
  for (look = needed; look != stop; look = look->next)
    if (strcmp (soname, look->name) == 0
	&& ((elf_dyn_lib_class (look->by) & DYN_AS_NEEDED) == 0
	    /* If needed by a library that itself is not directly
	       needed, recursively check whether that library is
	       indirectly needed.  Since we add DT_NEEDED entries to
	       the end of the list, library dependencies appear after
	       the library.  Therefore search prior to the current
	       LOOK, preventing possible infinite recursion.  */
	    || on_needed_list (elf_dt_name (look->by), needed, look)))
      return true;

  return false;
}

/* Sort symbol by value, section, size, and type.  */
static int
elf_sort_symbol (const void *arg1, const void *arg2)
{
  const struct elf_link_hash_entry *h1;
  const struct elf_link_hash_entry *h2;
  bfd_signed_vma vdiff;
  int sdiff;
  const char *n1;
  const char *n2;

  h1 = *(const struct elf_link_hash_entry **) arg1;
  h2 = *(const struct elf_link_hash_entry **) arg2;
  vdiff = h1->root.u.def.value - h2->root.u.def.value;
  if (vdiff != 0)
    return vdiff > 0 ? 1 : -1;

  sdiff = h1->root.u.def.section->id - h2->root.u.def.section->id;
  if (sdiff != 0)
    return sdiff;

  /* Sort so that sized symbols are selected over zero size symbols.  */
  vdiff = h1->size - h2->size;
  if (vdiff != 0)
    return vdiff > 0 ? 1 : -1;

  /* Sort so that STT_OBJECT is selected over STT_NOTYPE.  */
  if (h1->type != h2->type)
    return h1->type - h2->type;

  /* If symbols are properly sized and typed, and multiple strong
     aliases are not defined in a shared library by the user we
     shouldn't get here.  Unfortunately linker script symbols like
     __bss_start sometimes match a user symbol defined at the start of
     .bss without proper size and type.  We'd like to preference the
     user symbol over reserved system symbols.  Sort on leading
     underscores.  */
  n1 = h1->root.root.string;
  n2 = h2->root.root.string;
  while (*n1 == *n2)
    {
      if (*n1 == 0)
	break;
      ++n1;
      ++n2;
    }
  if (*n1 == '_')
    return -1;
  if (*n2 == '_')
    return 1;

  /* Final sort on name selects user symbols like '_u' over reserved
     system symbols like '_Z' and also will avoid qsort instability.  */
  return *n1 - *n2;
}

/* This function is used to adjust offsets into .dynstr for
   dynamic symbols.  This is called via elf_link_hash_traverse.  */

static bool
elf_adjust_dynstr_offsets (struct elf_link_hash_entry *h, void *data)
{
  struct elf_strtab_hash *dynstr = (struct elf_strtab_hash *) data;

  if (h->dynindx != -1)
    h->dynstr_index = _bfd_elf_strtab_offset (dynstr, h->dynstr_index);
  return true;
}

/* Assign string offsets in .dynstr, update all structures referencing
   them.  */

static bool
elf_finalize_dynstr (bfd *output_bfd, struct bfd_link_info *info)
{
  struct elf_link_hash_table *hash_table = elf_hash_table (info);
  struct elf_link_local_dynamic_entry *entry;
  struct elf_strtab_hash *dynstr = hash_table->dynstr;
  bfd *dynobj = hash_table->dynobj;
  asection *sdyn;
  bfd_size_type size;
  const struct elf_backend_data *bed;
  bfd_byte *extdyn;

  _bfd_elf_strtab_finalize (dynstr);
  size = _bfd_elf_strtab_size (dynstr);

  /* Allow the linker to examine the dynsymtab now it's fully populated.  */

  if (info->callbacks->examine_strtab)
    info->callbacks->examine_strtab (dynstr);

  bed = get_elf_backend_data (dynobj);
  sdyn = bfd_get_linker_section (dynobj, ".dynamic");
  BFD_ASSERT (sdyn != NULL);

  /* Update all .dynamic entries referencing .dynstr strings.  */
  for (extdyn = sdyn->contents;
       extdyn < PTR_ADD (sdyn->contents, sdyn->size);
       extdyn += bed->s->sizeof_dyn)
    {
      Elf_Internal_Dyn dyn;

      bed->s->swap_dyn_in (dynobj, extdyn, &dyn);
      switch (dyn.d_tag)
	{
	case DT_STRSZ:
	  dyn.d_un.d_val = size;
	  break;
	case DT_NEEDED:
	case DT_SONAME:
	case DT_RPATH:
	case DT_RUNPATH:
	case DT_FILTER:
	case DT_AUXILIARY:
	case DT_AUDIT:
	case DT_DEPAUDIT:
	  dyn.d_un.d_val = _bfd_elf_strtab_offset (dynstr, dyn.d_un.d_val);
	  break;
	default:
	  continue;
	}
      bed->s->swap_dyn_out (dynobj, &dyn, extdyn);
    }

  /* Now update local dynamic symbols.  */
  for (entry = hash_table->dynlocal; entry ; entry = entry->next)
    entry->isym.st_name = _bfd_elf_strtab_offset (dynstr,
						  entry->isym.st_name);

  /* And the rest of dynamic symbols.  */
  elf_link_hash_traverse (hash_table, elf_adjust_dynstr_offsets, dynstr);

  /* Adjust version definitions.  */
  if (elf_tdata (output_bfd)->cverdefs)
    {
      asection *s;
      bfd_byte *p;
      size_t i;
      Elf_Internal_Verdef def;
      Elf_Internal_Verdaux defaux;

      s = bfd_get_linker_section (dynobj, ".gnu.version_d");
      p = s->contents;
      do
	{
	  _bfd_elf_swap_verdef_in (output_bfd, (Elf_External_Verdef *) p,
				   &def);
	  p += sizeof (Elf_External_Verdef);
	  if (def.vd_aux != sizeof (Elf_External_Verdef))
	    continue;
	  for (i = 0; i < def.vd_cnt; ++i)
	    {
	      _bfd_elf_swap_verdaux_in (output_bfd,
					(Elf_External_Verdaux *) p, &defaux);
	      defaux.vda_name = _bfd_elf_strtab_offset (dynstr,
							defaux.vda_name);
	      _bfd_elf_swap_verdaux_out (output_bfd,
					 &defaux, (Elf_External_Verdaux *) p);
	      p += sizeof (Elf_External_Verdaux);
	    }
	}
      while (def.vd_next);
    }

  /* Adjust version references.  */
  if (elf_tdata (output_bfd)->verref)
    {
      asection *s;
      bfd_byte *p;
      size_t i;
      Elf_Internal_Verneed need;
      Elf_Internal_Vernaux needaux;

      s = bfd_get_linker_section (dynobj, ".gnu.version_r");
      p = s->contents;
      do
	{
	  _bfd_elf_swap_verneed_in (output_bfd, (Elf_External_Verneed *) p,
				    &need);
	  need.vn_file = _bfd_elf_strtab_offset (dynstr, need.vn_file);
	  _bfd_elf_swap_verneed_out (output_bfd, &need,
				     (Elf_External_Verneed *) p);
	  p += sizeof (Elf_External_Verneed);
	  for (i = 0; i < need.vn_cnt; ++i)
	    {
	      _bfd_elf_swap_vernaux_in (output_bfd,
					(Elf_External_Vernaux *) p, &needaux);
	      needaux.vna_name = _bfd_elf_strtab_offset (dynstr,
							 needaux.vna_name);
	      _bfd_elf_swap_vernaux_out (output_bfd,
					 &needaux,
					 (Elf_External_Vernaux *) p);
	      p += sizeof (Elf_External_Vernaux);
	    }
	}
      while (need.vn_next);
    }

  return true;
}

/* Return TRUE iff relocations for INPUT are compatible with OUTPUT.
   The default is to only match when the INPUT and OUTPUT are exactly
   the same target.  */

bool
_bfd_elf_default_relocs_compatible (const bfd_target *input,
				    const bfd_target *output)
{
  return input == output;
}

/* Return TRUE iff relocations for INPUT are compatible with OUTPUT.
   This version is used when different targets for the same architecture
   are virtually identical.  */

bool
_bfd_elf_relocs_compatible (const bfd_target *input,
			    const bfd_target *output)
{
  const struct elf_backend_data *obed, *ibed;

  if (input == output)
    return true;

  ibed = xvec_get_elf_backend_data (input);
  obed = xvec_get_elf_backend_data (output);

  if (ibed->arch != obed->arch)
    return false;

  /* If both backends are using this function, deem them compatible.  */
  return ibed->relocs_compatible == obed->relocs_compatible;
}

/* Make a special call to the linker "notice" function to tell it that
   we are about to handle an as-needed lib, or have finished
   processing the lib.  */

bool
_bfd_elf_notice_as_needed (bfd *ibfd,
			   struct bfd_link_info *info,
			   enum notice_asneeded_action act)
{
  return (*info->callbacks->notice) (info, NULL, NULL, ibfd, NULL, act, 0);
}

/* Call ACTION on each relocation in an ELF object file.  */

bool
_bfd_elf_link_iterate_on_relocs
  (bfd *abfd, struct bfd_link_info *info,
   bool (*action) (bfd *, struct bfd_link_info *, asection *,
		   const Elf_Internal_Rela *))
{
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  struct elf_link_hash_table *htab = elf_hash_table (info);

  /* If this object is the same format as the output object, and it is
     not a shared library, then let the backend look through the
     relocs.

     This is required to build global offset table entries and to
     arrange for dynamic relocs.  It is not required for the
     particular common case of linking non PIC code, even when linking
     against shared libraries, but unfortunately there is no way of
     knowing whether an object file has been compiled PIC or not.
     Looking through the relocs is not particularly time consuming.
     The problem is that we must either (1) keep the relocs in memory,
     which causes the linker to require additional runtime memory or
     (2) read the relocs twice from the input file, which wastes time.
     This would be a good case for using mmap.

     I have no idea how to handle linking PIC code into a file of a
     different format.  It probably can't be done.  */
  if ((abfd->flags & DYNAMIC) == 0
      && is_elf_hash_table (&htab->root)
      && elf_object_id (abfd) == elf_hash_table_id (htab)
      && (*bed->relocs_compatible) (abfd->xvec, info->output_bfd->xvec))
    {
      asection *o;

      for (o = abfd->sections; o != NULL; o = o->next)
	{
	  Elf_Internal_Rela *internal_relocs;
	  bool ok;

	  /* Don't check relocations in excluded sections.  Don't do
	     anything special with non-loaded, non-alloced sections.
	     In particular, any relocs in such sections should not
	     affect GOT and PLT reference counting (ie.  we don't
	     allow them to create GOT or PLT entries), there's no
	     possibility or desire to optimize TLS relocs, and
	     there's not much point in propagating relocs to shared
	     libs that the dynamic linker won't relocate.  */
	  if ((o->flags & SEC_ALLOC) == 0
	      || (o->flags & SEC_RELOC) == 0
	      || (o->flags & SEC_EXCLUDE) != 0
	      || o->reloc_count == 0
	      || ((info->strip == strip_all || info->strip == strip_debugger)
		  && (o->flags & SEC_DEBUGGING) != 0)
	      || bfd_is_abs_section (o->output_section))
	    continue;

	  internal_relocs = _bfd_elf_link_info_read_relocs (abfd, info,
							    o, NULL,
							    NULL,
							    _bfd_link_keep_memory (info));
	  if (internal_relocs == NULL)
	    return false;

	  ok = action (abfd, info, o, internal_relocs);

	  if (elf_section_data (o)->relocs != internal_relocs)
	    free (internal_relocs);

	  if (! ok)
	    return false;
	}
    }

  return true;
}

/* Check relocations in an ELF object file.  This is called after
   all input files have been opened.  */

bool
_bfd_elf_link_check_relocs (bfd *abfd, struct bfd_link_info *info)
{
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  if (bed->check_relocs != NULL)
    return _bfd_elf_link_iterate_on_relocs (abfd, info,
					    bed->check_relocs);
  return true;
}

/* Add symbols from an ELF object file to the linker hash table.  */

static bool
elf_link_add_object_symbols (bfd *abfd, struct bfd_link_info *info)
{
  Elf_Internal_Ehdr *ehdr;
  Elf_Internal_Shdr *hdr;
  size_t symcount;
  size_t extsymcount;
  size_t extsymoff;
  struct elf_link_hash_entry **sym_hash;
  bool dynamic;
  Elf_External_Versym *extversym = NULL;
  Elf_External_Versym *extversym_end = NULL;
  Elf_External_Versym *ever;
  struct elf_link_hash_entry *weaks;
  struct elf_link_hash_entry **nondeflt_vers = NULL;
  size_t nondeflt_vers_cnt = 0;
  Elf_Internal_Sym *isymbuf = NULL;
  Elf_Internal_Sym *isym;
  Elf_Internal_Sym *isymend;
  const struct elf_backend_data *bed;
  bool add_needed;
  struct elf_link_hash_table *htab;
  void *alloc_mark = NULL;
  struct bfd_hash_entry **old_table = NULL;
  unsigned int old_size = 0;
  unsigned int old_count = 0;
  void *old_tab = NULL;
  void *old_ent;
  struct bfd_link_hash_entry *old_undefs = NULL;
  struct bfd_link_hash_entry *old_undefs_tail = NULL;
  void *old_strtab = NULL;
  size_t tabsize = 0;
  asection *s;
  bool just_syms;

  htab = elf_hash_table (info);
  bed = get_elf_backend_data (abfd);

  if (elf_use_dt_symtab_p (abfd))
    {
      bfd_set_error (bfd_error_wrong_format);
      return false;
    }

  if ((abfd->flags & DYNAMIC) == 0)
    dynamic = false;
  else
    {
      dynamic = true;

      /* You can't use -r against a dynamic object.  Also, there's no
	 hope of using a dynamic object which does not exactly match
	 the format of the output file.  */
      if (bfd_link_relocatable (info)
	  || !is_elf_hash_table (&htab->root)
	  || info->output_bfd->xvec != abfd->xvec)
	{
	  if (bfd_link_relocatable (info))
	    bfd_set_error (bfd_error_invalid_operation);
	  else
	    bfd_set_error (bfd_error_wrong_format);
	  goto error_return;
	}
    }

  ehdr = elf_elfheader (abfd);
  if (info->warn_alternate_em
      && bed->elf_machine_code != ehdr->e_machine
      && ((bed->elf_machine_alt1 != 0
	   && ehdr->e_machine == bed->elf_machine_alt1)
	  || (bed->elf_machine_alt2 != 0
	      && ehdr->e_machine == bed->elf_machine_alt2)))
    _bfd_error_handler
      /* xgettext:c-format */
      (_("alternate ELF machine code found (%d) in %pB, expecting %d"),
       ehdr->e_machine, abfd, bed->elf_machine_code);

  /* As a GNU extension, any input sections which are named
     .gnu.warning.SYMBOL are treated as warning symbols for the given
     symbol.  This differs from .gnu.warning sections, which generate
     warnings when they are included in an output file.  */
  /* PR 12761: Also generate this warning when building shared libraries.  */
  for (s = abfd->sections; s != NULL; s = s->next)
    {
      const char *name;

      name = bfd_section_name (s);
      if (startswith (name, ".gnu.warning."))
	{
	  char *msg;
	  bfd_size_type sz;

	  name += sizeof ".gnu.warning." - 1;

	  /* If this is a shared object, then look up the symbol
	     in the hash table.  If it is there, and it is already
	     been defined, then we will not be using the entry
	     from this shared object, so we don't need to warn.
	     FIXME: If we see the definition in a regular object
	     later on, we will warn, but we shouldn't.  The only
	     fix is to keep track of what warnings we are supposed
	     to emit, and then handle them all at the end of the
	     link.  */
	  if (dynamic)
	    {
	      struct elf_link_hash_entry *h;

	      h = elf_link_hash_lookup (htab, name, false, false, true);

	      /* FIXME: What about bfd_link_hash_common?  */
	      if (h != NULL
		  && (h->root.type == bfd_link_hash_defined
		      || h->root.type == bfd_link_hash_defweak))
		continue;
	    }

	  sz = s->size;
	  msg = (char *) bfd_alloc (abfd, sz + 1);
	  if (msg == NULL)
	    goto error_return;

	  if (! bfd_get_section_contents (abfd, s, msg, 0, sz))
	    goto error_return;

	  msg[sz] = '\0';

	  if (! (_bfd_generic_link_add_one_symbol
		 (info, abfd, name, BSF_WARNING, s, 0, msg,
		  false, bed->collect, NULL)))
	    goto error_return;

	  if (bfd_link_executable (info))
	    {
	      /* Clobber the section size so that the warning does
		 not get copied into the output file.  */
	      s->size = 0;

	      /* Also set SEC_EXCLUDE, so that symbols defined in
		 the warning section don't get copied to the output.  */
	      s->flags |= SEC_EXCLUDE;
	    }
	}
    }

  just_syms = ((s = abfd->sections) != NULL
	       && s->sec_info_type == SEC_INFO_TYPE_JUST_SYMS);

  add_needed = true;
  if (! dynamic)
    {
      /* If we are creating a shared library, create all the dynamic
	 sections immediately.  We need to attach them to something,
	 so we attach them to this BFD, provided it is the right
	 format and is not from ld --just-symbols.  Always create the
	 dynamic sections for -E/--dynamic-list.  FIXME: If there
	 are no input BFD's of the same format as the output, we can't
	 make a shared library.  */
      if (!just_syms
	  && (bfd_link_pic (info)
	      || (!bfd_link_relocatable (info)
		  && info->nointerp
		  && (info->export_dynamic || info->dynamic)))
	  && is_elf_hash_table (&htab->root)
	  && info->output_bfd->xvec == abfd->xvec
	  && !htab->dynamic_sections_created)
	{
	  if (! _bfd_elf_link_create_dynamic_sections (abfd, info))
	    goto error_return;
	}
    }
  else if (!is_elf_hash_table (&htab->root))
    goto error_return;
  else
    {
      const char *soname = NULL;
      char *audit = NULL;
      struct bfd_link_needed_list *rpath = NULL, *runpath = NULL;
      const Elf_Internal_Phdr *phdr;
      struct elf_link_loaded_list *loaded_lib;

      /* ld --just-symbols and dynamic objects don't mix very well.
	 ld shouldn't allow it.  */
      if (just_syms)
	abort ();

      /* If this dynamic lib was specified on the command line with
	 --as-needed in effect, then we don't want to add a DT_NEEDED
	 tag unless the lib is actually used.  Similary for libs brought
	 in by another lib's DT_NEEDED.  When --no-add-needed is used
	 on a dynamic lib, we don't want to add a DT_NEEDED entry for
	 any dynamic library in DT_NEEDED tags in the dynamic lib at
	 all.  */
      add_needed = (elf_dyn_lib_class (abfd)
		    & (DYN_AS_NEEDED | DYN_DT_NEEDED
		       | DYN_NO_NEEDED)) == 0;

      s = bfd_get_section_by_name (abfd, ".dynamic");
      if (s != NULL && s->size != 0 && (s->flags & SEC_HAS_CONTENTS) != 0)
	{
	  bfd_byte *dynbuf;
	  bfd_byte *extdyn;
	  unsigned int elfsec;
	  unsigned long shlink;

	  if (!bfd_malloc_and_get_section (abfd, s, &dynbuf))
	    {
	    error_free_dyn:
	      free (dynbuf);
	      goto error_return;
	    }

	  elfsec = _bfd_elf_section_from_bfd_section (abfd, s);
	  if (elfsec == SHN_BAD)
	    goto error_free_dyn;
	  shlink = elf_elfsections (abfd)[elfsec]->sh_link;

	  for (extdyn = dynbuf;
	       (size_t) (dynbuf + s->size - extdyn) >= bed->s->sizeof_dyn;
	       extdyn += bed->s->sizeof_dyn)
	    {
	      Elf_Internal_Dyn dyn;

	      bed->s->swap_dyn_in (abfd, extdyn, &dyn);
	      if (dyn.d_tag == DT_SONAME)
		{
		  unsigned int tagv = dyn.d_un.d_val;
		  soname = bfd_elf_string_from_elf_section (abfd, shlink, tagv);
		  if (soname == NULL)
		    goto error_free_dyn;
		}
	      if (dyn.d_tag == DT_NEEDED)
		{
		  struct bfd_link_needed_list *n, **pn;
		  char *fnm, *anm;
		  unsigned int tagv = dyn.d_un.d_val;
		  size_t amt = sizeof (struct bfd_link_needed_list);

		  n = (struct bfd_link_needed_list *) bfd_alloc (abfd, amt);
		  fnm = bfd_elf_string_from_elf_section (abfd, shlink, tagv);
		  if (n == NULL || fnm == NULL)
		    goto error_free_dyn;
		  amt = strlen (fnm) + 1;
		  anm = (char *) bfd_alloc (abfd, amt);
		  if (anm == NULL)
		    goto error_free_dyn;
		  memcpy (anm, fnm, amt);
		  n->name = anm;
		  n->by = abfd;
		  n->next = NULL;
		  for (pn = &htab->needed; *pn != NULL; pn = &(*pn)->next)
		    ;
		  *pn = n;
		}
	      if (dyn.d_tag == DT_RUNPATH)
		{
		  struct bfd_link_needed_list *n, **pn;
		  char *fnm, *anm;
		  unsigned int tagv = dyn.d_un.d_val;
		  size_t amt = sizeof (struct bfd_link_needed_list);

		  n = (struct bfd_link_needed_list *) bfd_alloc (abfd, amt);
		  fnm = bfd_elf_string_from_elf_section (abfd, shlink, tagv);
		  if (n == NULL || fnm == NULL)
		    goto error_free_dyn;
		  amt = strlen (fnm) + 1;
		  anm = (char *) bfd_alloc (abfd, amt);
		  if (anm == NULL)
		    goto error_free_dyn;
		  memcpy (anm, fnm, amt);
		  n->name = anm;
		  n->by = abfd;
		  n->next = NULL;
		  for (pn = & runpath;
		       *pn != NULL;
		       pn = &(*pn)->next)
		    ;
		  *pn = n;
		}
	      /* Ignore DT_RPATH if we have seen DT_RUNPATH.  */
	      if (!runpath && dyn.d_tag == DT_RPATH)
		{
		  struct bfd_link_needed_list *n, **pn;
		  char *fnm, *anm;
		  unsigned int tagv = dyn.d_un.d_val;
		  size_t amt = sizeof (struct bfd_link_needed_list);

		  n = (struct bfd_link_needed_list *) bfd_alloc (abfd, amt);
		  fnm = bfd_elf_string_from_elf_section (abfd, shlink, tagv);
		  if (n == NULL || fnm == NULL)
		    goto error_free_dyn;
		  amt = strlen (fnm) + 1;
		  anm = (char *) bfd_alloc (abfd, amt);
		  if (anm == NULL)
		    goto error_free_dyn;
		  memcpy (anm, fnm, amt);
		  n->name = anm;
		  n->by = abfd;
		  n->next = NULL;
		  for (pn = & rpath;
		       *pn != NULL;
		       pn = &(*pn)->next)
		    ;
		  *pn = n;
		}
	      if (dyn.d_tag == DT_AUDIT)
		{
		  unsigned int tagv = dyn.d_un.d_val;
		  audit = bfd_elf_string_from_elf_section (abfd, shlink, tagv);
		}
	      if (dyn.d_tag == DT_FLAGS_1)
		elf_tdata (abfd)->is_pie = (dyn.d_un.d_val & DF_1_PIE) != 0;
	    }

	  free (dynbuf);
	}

      /* DT_RUNPATH overrides DT_RPATH.  Do _NOT_ bfd_release, as that
	 frees all more recently bfd_alloc'd blocks as well.  */
      if (runpath)
	rpath = runpath;

      if (rpath)
	{
	  struct bfd_link_needed_list **pn;
	  for (pn = &htab->runpath; *pn != NULL; pn = &(*pn)->next)
	    ;
	  *pn = rpath;
	}

      /* If we have a PT_GNU_RELRO program header, mark as read-only
	 all sections contained fully therein.  This makes relro
	 shared library sections appear as they will at run-time.  */
      phdr = elf_tdata (abfd)->phdr + elf_elfheader (abfd)->e_phnum;
      while (phdr-- > elf_tdata (abfd)->phdr)
	if (phdr->p_type == PT_GNU_RELRO)
	  {
	    for (s = abfd->sections; s != NULL; s = s->next)
	      {
		unsigned int opb = bfd_octets_per_byte (abfd, s);

		if ((s->flags & SEC_ALLOC) != 0
		    && s->vma * opb >= phdr->p_vaddr
		    && s->vma * opb + s->size <= phdr->p_vaddr + phdr->p_memsz)
		  s->flags |= SEC_READONLY;
	      }
	    break;
	  }

      /* We do not want to include any of the sections in a dynamic
	 object in the output file.  We hack by simply clobbering the
	 list of sections in the BFD.  This could be handled more
	 cleanly by, say, a new section flag; the existing
	 SEC_NEVER_LOAD flag is not the one we want, because that one
	 still implies that the section takes up space in the output
	 file.  */
      bfd_section_list_clear (abfd);

      /* Find the name to use in a DT_NEEDED entry that refers to this
	 object.  If the object has a DT_SONAME entry, we use it.
	 Otherwise, if the generic linker stuck something in
	 elf_dt_name, we use that.  Otherwise, we just use the file
	 name.  */
      if (soname == NULL || *soname == '\0')
	{
	  soname = elf_dt_name (abfd);
	  if (soname == NULL || *soname == '\0')
	    soname = bfd_get_filename (abfd);
	}

      /* Save the SONAME because sometimes the linker emulation code
	 will need to know it.  */
      elf_dt_name (abfd) = soname;

      /* If we have already included this dynamic object in the
	 link, just ignore it.  There is no reason to include a
	 particular dynamic object more than once.  */
      for (loaded_lib = htab->dyn_loaded;
	   loaded_lib != NULL;
	   loaded_lib = loaded_lib->next)
	{
	  if (strcmp (elf_dt_name (loaded_lib->abfd), soname) == 0)
	    return true;
	}

      /* Create dynamic sections for backends that require that be done
	 before setup_gnu_properties.  */
      if (add_needed
	  && !_bfd_elf_link_create_dynamic_sections (abfd, info))
	return false;

      /* Save the DT_AUDIT entry for the linker emulation code. */
      elf_dt_audit (abfd) = audit;
    }

  /* If this is a dynamic object, we always link against the .dynsym
     symbol table, not the .symtab symbol table.  The dynamic linker
     will only see the .dynsym symbol table, so there is no reason to
     look at .symtab for a dynamic object.  */

  if (! dynamic || elf_dynsymtab (abfd) == 0)
    hdr = &elf_tdata (abfd)->symtab_hdr;
  else
    hdr = &elf_tdata (abfd)->dynsymtab_hdr;

  symcount = hdr->sh_size / bed->s->sizeof_sym;

  /* The sh_info field of the symtab header tells us where the
     external symbols start.  We don't care about the local symbols at
     this point.  */
  if (elf_bad_symtab (abfd))
    {
      extsymcount = symcount;
      extsymoff = 0;
    }
  else
    {
      extsymcount = symcount - hdr->sh_info;
      extsymoff = hdr->sh_info;
    }

  sym_hash = elf_sym_hashes (abfd);
  if (extsymcount != 0)
    {
      isymbuf = bfd_elf_get_elf_syms (abfd, hdr, extsymcount, extsymoff,
				      NULL, NULL, NULL);
      if (isymbuf == NULL)
	goto error_return;

      if (sym_hash == NULL)
	{
	  /* We store a pointer to the hash table entry for each
	     external symbol.  */
	  size_t amt = extsymcount * sizeof (struct elf_link_hash_entry *);
	  sym_hash = (struct elf_link_hash_entry **) bfd_zalloc (abfd, amt);
	  if (sym_hash == NULL)
	    goto error_free_sym;
	  elf_sym_hashes (abfd) = sym_hash;
	}
    }

  if (dynamic)
    {
      /* Read in any version definitions.  */
      if (!_bfd_elf_slurp_version_tables (abfd,
					  info->default_imported_symver))
	goto error_free_sym;

      /* Read in the symbol versions, but don't bother to convert them
	 to internal format.  */
      if (elf_dynversym (abfd) != 0)
	{
	  Elf_Internal_Shdr *versymhdr = &elf_tdata (abfd)->dynversym_hdr;
	  bfd_size_type amt = versymhdr->sh_size;

	  if (bfd_seek (abfd, versymhdr->sh_offset, SEEK_SET) != 0)
	    goto error_free_sym;
	  extversym = (Elf_External_Versym *)
	    _bfd_malloc_and_read (abfd, amt, amt);
	  if (extversym == NULL)
	    goto error_free_sym;
	  extversym_end = extversym + amt / sizeof (*extversym);
	}
    }

  /* If we are loading an as-needed shared lib, save the symbol table
     state before we start adding symbols.  If the lib turns out
     to be unneeded, restore the state.  */
  if ((elf_dyn_lib_class (abfd) & DYN_AS_NEEDED) != 0)
    {
      unsigned int i;
      size_t entsize;

      for (entsize = 0, i = 0; i < htab->root.table.size; i++)
	{
	  struct bfd_hash_entry *p;
	  struct elf_link_hash_entry *h;

	  for (p = htab->root.table.table[i]; p != NULL; p = p->next)
	    {
	      h = (struct elf_link_hash_entry *) p;
	      entsize += htab->root.table.entsize;
	      if (h->root.type == bfd_link_hash_warning)
		{
		  entsize += htab->root.table.entsize;
		  h = (struct elf_link_hash_entry *) h->root.u.i.link;
		}
	      if (h->root.type == bfd_link_hash_common)
		entsize += sizeof (*h->root.u.c.p);
	    }
	}

      tabsize = htab->root.table.size * sizeof (struct bfd_hash_entry *);
      old_tab = bfd_malloc (tabsize + entsize);
      if (old_tab == NULL)
	goto error_free_vers;

      /* Remember the current objalloc pointer, so that all mem for
	 symbols added can later be reclaimed.  */
      alloc_mark = bfd_hash_allocate (&htab->root.table, 1);
      if (alloc_mark == NULL)
	goto error_free_vers;

      /* Make a special call to the linker "notice" function to
	 tell it that we are about to handle an as-needed lib.  */
      if (!(*bed->notice_as_needed) (abfd, info, notice_as_needed))
	goto error_free_vers;

      /* Clone the symbol table.  Remember some pointers into the
	 symbol table, and dynamic symbol count.  */
      old_ent = (char *) old_tab + tabsize;
      memcpy (old_tab, htab->root.table.table, tabsize);
      old_undefs = htab->root.undefs;
      old_undefs_tail = htab->root.undefs_tail;
      old_table = htab->root.table.table;
      old_size = htab->root.table.size;
      old_count = htab->root.table.count;
      old_strtab = NULL;
      if (htab->dynstr != NULL)
	{
	  old_strtab = _bfd_elf_strtab_save (htab->dynstr);
	  if (old_strtab == NULL)
	    goto error_free_vers;
	}

      for (i = 0; i < htab->root.table.size; i++)
	{
	  struct bfd_hash_entry *p;
	  struct elf_link_hash_entry *h;

	  for (p = htab->root.table.table[i]; p != NULL; p = p->next)
	    {
	      h = (struct elf_link_hash_entry *) p;
	      memcpy (old_ent, h, htab->root.table.entsize);
	      old_ent = (char *) old_ent + htab->root.table.entsize;
	      if (h->root.type == bfd_link_hash_warning)
		{
		  h = (struct elf_link_hash_entry *) h->root.u.i.link;
		  memcpy (old_ent, h, htab->root.table.entsize);
		  old_ent = (char *) old_ent + htab->root.table.entsize;
		}
	      if (h->root.type == bfd_link_hash_common)
		{
		  memcpy (old_ent, h->root.u.c.p, sizeof (*h->root.u.c.p));
		  old_ent = (char *) old_ent + sizeof (*h->root.u.c.p);
		}
	    }
	}
    }

  weaks = NULL;
  if (extversym == NULL)
    ever = NULL;
  else if (extversym + extsymoff < extversym_end)
    ever = extversym + extsymoff;
  else
    {
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: invalid version offset %lx (max %lx)"),
			  abfd, (long) extsymoff,
			  (long) (extversym_end - extversym) / sizeof (* extversym));
      bfd_set_error (bfd_error_bad_value);
      goto error_free_vers;
    }

  if (!bfd_link_relocatable (info)
      && abfd->lto_slim_object)
    {
      _bfd_error_handler
	(_("%pB: plugin needed to handle lto object"), abfd);
    }

  for (isym = isymbuf, isymend = PTR_ADD (isymbuf, extsymcount);
       isym < isymend;
       isym++, sym_hash++, ever = (ever != NULL ? ever + 1 : NULL))
    {
      int bind;
      bfd_vma value;
      asection *sec, *new_sec;
      flagword flags;
      const char *name;
      struct elf_link_hash_entry *h;
      struct elf_link_hash_entry *hi;
      bool definition;
      bool size_change_ok;
      bool type_change_ok;
      bool new_weak;
      bool old_weak;
      bfd *override;
      bool common;
      bool discarded;
      unsigned int old_alignment;
      unsigned int shindex;
      bfd *old_bfd;
      bool matched;

      override = NULL;

      flags = BSF_NO_FLAGS;
      sec = NULL;
      value = isym->st_value;
      common = bed->common_definition (isym);
      if (common && info->inhibit_common_definition)
	{
	  /* Treat common symbol as undefined for --no-define-common.  */
	  isym->st_shndx = SHN_UNDEF;
	  common = false;
	}
      discarded = false;

      bind = ELF_ST_BIND (isym->st_info);
      switch (bind)
	{
	case STB_LOCAL:
	  /* This should be impossible, since ELF requires that all
	     global symbols follow all local symbols, and that sh_info
	     point to the first global symbol.  Unfortunately, Irix 5
	     screws this up.  */
	  if (elf_bad_symtab (abfd))
	    continue;

	  /* If we aren't prepared to handle locals within the globals
	     then we'll likely segfault on a NULL symbol hash if the
	     symbol is ever referenced in relocations.  */
	  shindex = elf_elfheader (abfd)->e_shstrndx;
	  name = bfd_elf_string_from_elf_section (abfd, shindex, hdr->sh_name);
	  _bfd_error_handler (_("%pB: %s local symbol at index %lu"
				" (>= sh_info of %lu)"),
			      abfd, name, (long) (isym - isymbuf + extsymoff),
			      (long) extsymoff);

	  /* Dynamic object relocations are not processed by ld, so
	     ld won't run into the problem mentioned above.  */
	  if (dynamic)
	    continue;
	  bfd_set_error (bfd_error_bad_value);
	  goto error_free_vers;

	case STB_GLOBAL:
	  if (isym->st_shndx != SHN_UNDEF && !common)
	    flags = BSF_GLOBAL;
	  break;

	case STB_WEAK:
	  flags = BSF_WEAK;
	  break;

	case STB_GNU_UNIQUE:
	  flags = BSF_GNU_UNIQUE;
	  break;

	default:
	  /* Leave it up to the processor backend.  */
	  break;
	}

      if (isym->st_shndx == SHN_UNDEF)
	sec = bfd_und_section_ptr;
      else if (isym->st_shndx == SHN_ABS)
	sec = bfd_abs_section_ptr;
      else if (isym->st_shndx == SHN_COMMON)
	{
	  sec = bfd_com_section_ptr;
	  /* What ELF calls the size we call the value.  What ELF
	     calls the value we call the alignment.  */
	  value = isym->st_size;
	}
      else
	{
	  sec = bfd_section_from_elf_index (abfd, isym->st_shndx);
	  if (sec == NULL)
	    sec = bfd_abs_section_ptr;
	  else if (discarded_section (sec))
	    {
	      /* Symbols from discarded section are undefined.  We keep
		 its visibility.  */
	      sec = bfd_und_section_ptr;
	      discarded = true;
	      isym->st_shndx = SHN_UNDEF;
	    }
	  else if ((abfd->flags & (EXEC_P | DYNAMIC)) != 0)
	    value -= sec->vma;
	}

      name = bfd_elf_string_from_elf_section (abfd, hdr->sh_link,
					      isym->st_name);
      if (name == NULL)
	goto error_free_vers;

      if (isym->st_shndx == SHN_COMMON
	  && (abfd->flags & BFD_PLUGIN) != 0)
	{
	  asection *xc = bfd_get_section_by_name (abfd, "COMMON");

	  if (xc == NULL)
	    {
	      flagword sflags = (SEC_ALLOC | SEC_IS_COMMON | SEC_KEEP
				 | SEC_EXCLUDE);
	      xc = bfd_make_section_with_flags (abfd, "COMMON", sflags);
	      if (xc == NULL)
		goto error_free_vers;
	    }
	  sec = xc;
	}
      else if (isym->st_shndx == SHN_COMMON
	       && ELF_ST_TYPE (isym->st_info) == STT_TLS
	       && !bfd_link_relocatable (info))
	{
	  asection *tcomm = bfd_get_section_by_name (abfd, ".tcommon");

	  if (tcomm == NULL)
	    {
	      flagword sflags = (SEC_ALLOC | SEC_THREAD_LOCAL | SEC_IS_COMMON
				 | SEC_LINKER_CREATED);
	      tcomm = bfd_make_section_with_flags (abfd, ".tcommon", sflags);
	      if (tcomm == NULL)
		goto error_free_vers;
	    }
	  sec = tcomm;
	}
      else if (bed->elf_add_symbol_hook)
	{
	  if (! (*bed->elf_add_symbol_hook) (abfd, info, isym, &name, &flags,
					     &sec, &value))
	    goto error_free_vers;

	  /* The hook function sets the name to NULL if this symbol
	     should be skipped for some reason.  */
	  if (name == NULL)
	    continue;
	}

      /* Sanity check that all possibilities were handled.  */
      if (sec == NULL)
	abort ();

      /* Silently discard TLS symbols from --just-syms.  There's
	 no way to combine a static TLS block with a new TLS block
	 for this executable.  */
      if (ELF_ST_TYPE (isym->st_info) == STT_TLS
	  && sec->sec_info_type == SEC_INFO_TYPE_JUST_SYMS)
	continue;

      if (bfd_is_und_section (sec)
	  || bfd_is_com_section (sec))
	definition = false;
      else
	definition = true;

      size_change_ok = false;
      type_change_ok = bed->type_change_ok;
      old_weak = false;
      matched = false;
      old_alignment = 0;
      old_bfd = NULL;
      new_sec = sec;

      if (is_elf_hash_table (&htab->root))
	{
	  Elf_Internal_Versym iver;
	  unsigned int vernum = 0;
	  bool skip;

	  if (ever == NULL)
	    {
	      if (info->default_imported_symver)
		/* Use the default symbol version created earlier.  */
		iver.vs_vers = elf_tdata (abfd)->cverdefs;
	      else
		iver.vs_vers = 0;
	    }
	  else if (ever >= extversym_end)
	    {
	      /* xgettext:c-format */
	      _bfd_error_handler (_("%pB: not enough version information"),
				  abfd);
	      bfd_set_error (bfd_error_bad_value);
	      goto error_free_vers;
	    }
	  else
	    _bfd_elf_swap_versym_in (abfd, ever, &iver);

	  vernum = iver.vs_vers & VERSYM_VERSION;

	  /* If this is a hidden symbol, or if it is not version
	     1, we append the version name to the symbol name.
	     However, we do not modify a non-hidden absolute symbol
	     if it is not a function, because it might be the version
	     symbol itself.  FIXME: What if it isn't?  */
	  if ((iver.vs_vers & VERSYM_HIDDEN) != 0
	      || (vernum > 1
		  && (!bfd_is_abs_section (sec)
		      || bed->is_function_type (ELF_ST_TYPE (isym->st_info)))))
	    {
	      const char *verstr;
	      size_t namelen, verlen, newlen;
	      char *newname, *p;

	      if (isym->st_shndx != SHN_UNDEF)
		{
		  if (vernum > elf_tdata (abfd)->cverdefs)
		    verstr = NULL;
		  else if (vernum > 1)
		    verstr =
		      elf_tdata (abfd)->verdef[vernum - 1].vd_nodename;
		  else
		    verstr = "";

		  if (verstr == NULL)
		    {
		      _bfd_error_handler
			/* xgettext:c-format */
			(_("%pB: %s: invalid version %u (max %d)"),
			 abfd, name, vernum,
			 elf_tdata (abfd)->cverdefs);
		      bfd_set_error (bfd_error_bad_value);
		      goto error_free_vers;
		    }
		}
	      else
		{
		  /* We cannot simply test for the number of
		     entries in the VERNEED section since the
		     numbers for the needed versions do not start
		     at 0.  */
		  Elf_Internal_Verneed *t;

		  verstr = NULL;
		  for (t = elf_tdata (abfd)->verref;
		       t != NULL;
		       t = t->vn_nextref)
		    {
		      Elf_Internal_Vernaux *a;

		      for (a = t->vn_auxptr; a != NULL; a = a->vna_nextptr)
			{
			  if (a->vna_other == vernum)
			    {
			      verstr = a->vna_nodename;
			      break;
			    }
			}
		      if (a != NULL)
			break;
		    }
		  if (verstr == NULL)
		    {
		      _bfd_error_handler
			/* xgettext:c-format */
			(_("%pB: %s: invalid needed version %d"),
			 abfd, name, vernum);
		      bfd_set_error (bfd_error_bad_value);
		      goto error_free_vers;
		    }
		}

	      namelen = strlen (name);
	      verlen = strlen (verstr);
	      newlen = namelen + verlen + 2;
	      if ((iver.vs_vers & VERSYM_HIDDEN) == 0
		  && isym->st_shndx != SHN_UNDEF)
		++newlen;

	      newname = (char *) bfd_hash_allocate (&htab->root.table, newlen);
	      if (newname == NULL)
		goto error_free_vers;
	      memcpy (newname, name, namelen);
	      p = newname + namelen;
	      *p++ = ELF_VER_CHR;
	      /* If this is a defined non-hidden version symbol,
		 we add another @ to the name.  This indicates the
		 default version of the symbol.  */
	      if ((iver.vs_vers & VERSYM_HIDDEN) == 0
		  && isym->st_shndx != SHN_UNDEF)
		*p++ = ELF_VER_CHR;
	      memcpy (p, verstr, verlen + 1);

	      name = newname;
	    }

	  /* If this symbol has default visibility and the user has
	     requested we not re-export it, then mark it as hidden.  */
	  if (!bfd_is_und_section (sec)
	      && !dynamic
	      && abfd->no_export
	      && ELF_ST_VISIBILITY (isym->st_other) != STV_INTERNAL)
	    isym->st_other = (STV_HIDDEN
			      | (isym->st_other & ~ELF_ST_VISIBILITY (-1)));

	  if (!_bfd_elf_merge_symbol (abfd, info, name, isym, &sec, &value,
				      sym_hash, &old_bfd, &old_weak,
				      &old_alignment, &skip, &override,
				      &type_change_ok, &size_change_ok,
				      &matched))
	    goto error_free_vers;

	  if (skip)
	    continue;

	  /* Override a definition only if the new symbol matches the
	     existing one.  */
	  if (override && matched)
	    definition = false;

	  h = *sym_hash;
	  while (h->root.type == bfd_link_hash_indirect
		 || h->root.type == bfd_link_hash_warning)
	    h = (struct elf_link_hash_entry *) h->root.u.i.link;

	  if (h->versioned != unversioned
	      && elf_tdata (abfd)->verdef != NULL
	      && vernum > 1
	      && definition)
	    h->verinfo.verdef = &elf_tdata (abfd)->verdef[vernum - 1];
	}

      if (! (_bfd_generic_link_add_one_symbol
	     (info, override ? override : abfd, name, flags, sec, value,
	      NULL, false, bed->collect,
	      (struct bfd_link_hash_entry **) sym_hash)))
	goto error_free_vers;

      h = *sym_hash;
      /* We need to make sure that indirect symbol dynamic flags are
	 updated.  */
      hi = h;
      while (h->root.type == bfd_link_hash_indirect
	     || h->root.type == bfd_link_hash_warning)
	h = (struct elf_link_hash_entry *) h->root.u.i.link;

      *sym_hash = h;

      /* Setting the index to -3 tells elf_link_output_extsym that
	 this symbol is defined in a discarded section.  */
      if (discarded && is_elf_hash_table (&htab->root))
	h->indx = -3;

      new_weak = (flags & BSF_WEAK) != 0;
      if (dynamic
	  && definition
	  && new_weak
	  && !bed->is_function_type (ELF_ST_TYPE (isym->st_info))
	  && is_elf_hash_table (&htab->root)
	  && h->u.alias == NULL)
	{
	  /* Keep a list of all weak defined non function symbols from
	     a dynamic object, using the alias field.  Later in this
	     function we will set the alias field to the correct
	     value.  We only put non-function symbols from dynamic
	     objects on this list, because that happens to be the only
	     time we need to know the normal symbol corresponding to a
	     weak symbol, and the information is time consuming to
	     figure out.  If the alias field is not already NULL,
	     then this symbol was already defined by some previous
	     dynamic object, and we will be using that previous
	     definition anyhow.  */

	  h->u.alias = weaks;
	  weaks = h;
	}

      /* Set the alignment of a common symbol.  */
      if ((common || bfd_is_com_section (sec))
	  && h->root.type == bfd_link_hash_common)
	{
	  unsigned int align;

	  if (common)
	    align = bfd_log2 (isym->st_value);
	  else
	    {
	      /* The new symbol is a common symbol in a shared object.
		 We need to get the alignment from the section.  */
	      align = new_sec->alignment_power;
	    }
	  if (align > old_alignment)
	    h->root.u.c.p->alignment_power = align;
	  else
	    h->root.u.c.p->alignment_power = old_alignment;
	}

      if (is_elf_hash_table (&htab->root))
	{
	  /* Set a flag in the hash table entry indicating the type of
	     reference or definition we just found.  A dynamic symbol
	     is one which is referenced or defined by both a regular
	     object and a shared object.  */
	  bool dynsym = false;

	  /* Plugin symbols aren't normal.  Don't set def/ref flags.  */
	  if ((abfd->flags & BFD_PLUGIN) != 0)
	    {
	      /* Except for this flag to track nonweak references.  */
	      if (!definition
		  && bind != STB_WEAK)
		h->ref_ir_nonweak = 1;
	    }
	  else if (!dynamic)
	    {
	      if (! definition)
		{
		  h->ref_regular = 1;
		  if (bind != STB_WEAK)
		    h->ref_regular_nonweak = 1;
		}
	      else
		{
		  h->def_regular = 1;
		  if (h->def_dynamic)
		    {
		      h->def_dynamic = 0;
		      h->ref_dynamic = 1;
		    }
		}
	    }
	  else
	    {
	      if (! definition)
		{
		  h->ref_dynamic = 1;
		  hi->ref_dynamic = 1;
		}
	      else
		{
		  h->def_dynamic = 1;
		  hi->def_dynamic = 1;
		}
	    }

	  /* If an indirect symbol has been forced local, don't
	     make the real symbol dynamic.  */
	  if (h != hi && hi->forced_local)
	    ;
	  else if (!dynamic)
	    {
	      if (bfd_link_dll (info)
		  || h->def_dynamic
		  || h->ref_dynamic)
		dynsym = true;
	    }
	  else
	    {
	      if (h->def_regular
		  || h->ref_regular
		  || (h->is_weakalias
		      && weakdef (h)->dynindx != -1))
		dynsym = true;
	    }

	  /* Check to see if we need to add an indirect symbol for
	     the default name.  */
	  if ((definition
	       || (!override && h->root.type == bfd_link_hash_common))
	      && !(hi != h
		   && hi->versioned == versioned_hidden))
	    if (!_bfd_elf_add_default_symbol (abfd, info, h, name, isym,
					      sec, value, &old_bfd, &dynsym))
	      goto error_free_vers;

	  /* Check the alignment when a common symbol is involved. This
	     can change when a common symbol is overridden by a normal
	     definition or a common symbol is ignored due to the old
	     normal definition. We need to make sure the maximum
	     alignment is maintained.  */
	  if ((old_alignment || common)
	      && h->root.type != bfd_link_hash_common)
	    {
	      unsigned int common_align;
	      unsigned int normal_align;
	      unsigned int symbol_align;
	      bfd *normal_bfd;
	      bfd *common_bfd;

	      BFD_ASSERT (h->root.type == bfd_link_hash_defined
			  || h->root.type == bfd_link_hash_defweak);

	      symbol_align = ffs (h->root.u.def.value) - 1;
	      if (h->root.u.def.section->owner != NULL
		  && (h->root.u.def.section->owner->flags
		       & (DYNAMIC | BFD_PLUGIN)) == 0)
		{
		  normal_align = h->root.u.def.section->alignment_power;
		  if (normal_align > symbol_align)
		    normal_align = symbol_align;
		}
	      else
		normal_align = symbol_align;

	      if (old_alignment)
		{
		  common_align = old_alignment;
		  common_bfd = old_bfd;
		  normal_bfd = abfd;
		}
	      else
		{
		  common_align = bfd_log2 (isym->st_value);
		  common_bfd = abfd;
		  normal_bfd = old_bfd;
		}

	      if (normal_align < common_align)
		{
		  /* PR binutils/2735 */
		  if (normal_bfd == NULL)
		    _bfd_error_handler
		      /* xgettext:c-format */
		      (_("warning: alignment %u of common symbol `%s' in %pB is"
			 " greater than the alignment (%u) of its section %pA"),
		       1 << common_align, name, common_bfd,
		       1 << normal_align, h->root.u.def.section);
		  else
		    _bfd_error_handler
		      /* xgettext:c-format */
		      (_("warning: alignment %u of normal symbol `%s' in %pB"
			 " is smaller than %u used by the common definition in %pB"),
		       1 << normal_align, name, normal_bfd,
		       1 << common_align, common_bfd);

		  /* PR 30499: make sure that users understand that this warning is serious.  */
		  _bfd_error_handler
		    (_("warning: NOTE: alignment discrepancies can cause real problems.  Investigation is advised."));
		}
	    }

	  /* Remember the symbol size if it isn't undefined.  */
	  if (isym->st_size != 0
	      && isym->st_shndx != SHN_UNDEF
	      && (definition || h->size == 0))
	    {
	      if (h->size != 0
		  && h->size != isym->st_size
		  && ! size_change_ok)
		{
		  _bfd_error_handler
		    /* xgettext:c-format */
		    (_("warning: size of symbol `%s' changed"
		       " from %" PRIu64 " in %pB to %" PRIu64 " in %pB"),
		     name, (uint64_t) h->size, old_bfd,
		     (uint64_t) isym->st_size, abfd);

		  /* PR 30499: make sure that users understand that this warning is serious.  */
		  _bfd_error_handler
		    (_("warning: NOTE: size discrepancies can cause real problems.  Investigation is advised."));
		}

	      h->size = isym->st_size;
	    }

	  /* If this is a common symbol, then we always want H->SIZE
	     to be the size of the common symbol.  The code just above
	     won't fix the size if a common symbol becomes larger.  We
	     don't warn about a size change here, because that is
	     covered by --warn-common.  Allow changes between different
	     function types.  */
	  if (h->root.type == bfd_link_hash_common)
	    h->size = h->root.u.c.size;

	  if (ELF_ST_TYPE (isym->st_info) != STT_NOTYPE
	      && ((definition && !new_weak)
		  || (old_weak && h->root.type == bfd_link_hash_common)
		  || h->type == STT_NOTYPE))
	    {
	      unsigned int type = ELF_ST_TYPE (isym->st_info);

	      /* Turn an IFUNC symbol from a DSO into a normal FUNC
		 symbol.  */
	      if (type == STT_GNU_IFUNC
		  && (abfd->flags & DYNAMIC) != 0)
		type = STT_FUNC;

	      if (h->type != type)
		{
		  if (h->type != STT_NOTYPE && ! type_change_ok)
		    /* xgettext:c-format */
		    _bfd_error_handler
		      (_("warning: type of symbol `%s' changed"
			 " from %d to %d in %pB"),
		       name, h->type, type, abfd);

		  h->type = type;
		}
	    }

	  /* Merge st_other field.  */
	  elf_merge_st_other (abfd, h, isym->st_other, sec,
			      definition, dynamic);

	  /* We don't want to make debug symbol dynamic.  */
	  if (definition
	      && (sec->flags & SEC_DEBUGGING)
	      && !bfd_link_relocatable (info))
	    dynsym = false;

	  /* Nor should we make plugin symbols dynamic.  */
	  if ((abfd->flags & BFD_PLUGIN) != 0)
	    dynsym = false;

	  if (definition)
	    {
	      h->target_internal = isym->st_target_internal;
	      h->unique_global = (flags & BSF_GNU_UNIQUE) != 0;
	    }

	  /* Don't add indirect symbols for .symver x, x@FOO aliases
	     in IR.  Since all data or text symbols in IR have the
	     same type, value and section, we can't tell if a symbol
	     is an alias of another symbol by their types, values and
	     sections.  */
	  if (definition
	      && !dynamic
	      && (abfd->flags & BFD_PLUGIN) == 0)
	    {
	      char *p = strchr (name, ELF_VER_CHR);
	      if (p != NULL && p[1] != ELF_VER_CHR)
		{
		  /* Queue non-default versions so that .symver x, x@FOO
		     aliases can be checked.  */
		  if (!nondeflt_vers)
		    {
		      size_t amt = ((isymend - isym + 1)
				    * sizeof (struct elf_link_hash_entry *));
		      nondeflt_vers
			= (struct elf_link_hash_entry **) bfd_malloc (amt);
		      if (!nondeflt_vers)
			goto error_free_vers;
		    }
		  nondeflt_vers[nondeflt_vers_cnt++] = h;
		}
	    }

	  if (dynsym && h->dynindx == -1)
	    {
	      if (! bfd_elf_link_record_dynamic_symbol (info, h))
		goto error_free_vers;
	      if (h->is_weakalias
		  && weakdef (h)->dynindx == -1)
		{
		  if (!bfd_elf_link_record_dynamic_symbol (info, weakdef (h)))
		    goto error_free_vers;
		}
	    }
	  else if (h->dynindx != -1)
	    /* If the symbol already has a dynamic index, but
	       visibility says it should not be visible, turn it into
	       a local symbol.  */
	    switch (ELF_ST_VISIBILITY (h->other))
	      {
	      case STV_INTERNAL:
	      case STV_HIDDEN:
		(*bed->elf_backend_hide_symbol) (info, h, true);
		dynsym = false;
		break;
	      }

	  if (!add_needed
	      && matched
	      && definition
	      && h->root.type != bfd_link_hash_indirect
	      && ((dynsym
		   && h->ref_regular_nonweak)
		  || (old_bfd != NULL
		      && (old_bfd->flags & BFD_PLUGIN) != 0
		      && h->ref_ir_nonweak
		      && !info->lto_all_symbols_read)
		  || (h->ref_dynamic_nonweak
		      && (elf_dyn_lib_class (abfd) & DYN_AS_NEEDED) != 0
		      && !on_needed_list (elf_dt_name (abfd),
					  htab->needed, NULL))))
	    {
	      const char *soname = elf_dt_name (abfd);

	      info->callbacks->minfo ("%!", soname, old_bfd,
				      h->root.root.string);

	      /* A symbol from a library loaded via DT_NEEDED of some
		 other library is referenced by a regular object.
		 Add a DT_NEEDED entry for it.  Issue an error if
		 --no-add-needed is used and the reference was not
		 a weak one.  */
	      if (old_bfd != NULL
		  && (elf_dyn_lib_class (abfd) & DYN_NO_NEEDED) != 0)
		{
		  _bfd_error_handler
		    /* xgettext:c-format */
		    (_("%pB: undefined reference to symbol '%s'"),
		     old_bfd, name);
		  bfd_set_error (bfd_error_missing_dso);
		  goto error_free_vers;
		}

	      elf_dyn_lib_class (abfd) = (enum dynamic_lib_link_class)
		(elf_dyn_lib_class (abfd) & ~DYN_AS_NEEDED);

	      /* Create dynamic sections for backends that require
		 that be done before setup_gnu_properties.  */
	      if (!_bfd_elf_link_create_dynamic_sections (abfd, info))
		return false;
	      add_needed = true;
	    }
	}
    }

  if (info->lto_plugin_active
      && !bfd_link_relocatable (info)
      && (abfd->flags & BFD_PLUGIN) == 0
      && !just_syms
      && extsymcount)
    {
      int r_sym_shift;

      if (bed->s->arch_size == 32)
	r_sym_shift = 8;
      else
	r_sym_shift = 32;

      /* If linker plugin is enabled, set non_ir_ref_regular on symbols
	 referenced in regular objects so that linker plugin will get
	 the correct symbol resolution.  */

      sym_hash = elf_sym_hashes (abfd);
      for (s = abfd->sections; s != NULL; s = s->next)
	{
	  Elf_Internal_Rela *internal_relocs;
	  Elf_Internal_Rela *rel, *relend;

	  /* Don't check relocations in excluded sections.  */
	  if ((s->flags & SEC_RELOC) == 0
	      || s->reloc_count == 0
	      || (s->flags & SEC_EXCLUDE) != 0
	      || ((info->strip == strip_all
		   || info->strip == strip_debugger)
		  && (s->flags & SEC_DEBUGGING) != 0))
	    continue;

	  internal_relocs = _bfd_elf_link_info_read_relocs (abfd, info,
							    s, NULL,
							    NULL,
							    _bfd_link_keep_memory (info));
	  if (internal_relocs == NULL)
	    goto error_free_vers;

	  rel = internal_relocs;
	  relend = rel + s->reloc_count;
	  for ( ; rel < relend; rel++)
	    {
	      unsigned long r_symndx = rel->r_info >> r_sym_shift;
	      struct elf_link_hash_entry *h;

	      /* Skip local symbols.  */
	      if (r_symndx < extsymoff)
		continue;

	      h = sym_hash[r_symndx - extsymoff];
	      if (h != NULL)
		h->root.non_ir_ref_regular = 1;
	    }

	  if (elf_section_data (s)->relocs != internal_relocs)
	    free (internal_relocs);
	}
    }

  free (extversym);
  extversym = NULL;
  free (isymbuf);
  isymbuf = NULL;

  if ((elf_dyn_lib_class (abfd) & DYN_AS_NEEDED) != 0)
    {
      unsigned int i;

      /* Restore the symbol table.  */
      old_ent = (char *) old_tab + tabsize;
      memset (elf_sym_hashes (abfd), 0,
	      extsymcount * sizeof (struct elf_link_hash_entry *));
      htab->root.table.table = old_table;
      htab->root.table.size = old_size;
      htab->root.table.count = old_count;
      memcpy (htab->root.table.table, old_tab, tabsize);
      htab->root.undefs = old_undefs;
      htab->root.undefs_tail = old_undefs_tail;
      if (htab->dynstr != NULL)
	_bfd_elf_strtab_restore (htab->dynstr, old_strtab);
      free (old_strtab);
      old_strtab = NULL;
      for (i = 0; i < htab->root.table.size; i++)
	{
	  struct bfd_hash_entry *p;
	  struct elf_link_hash_entry *h;
	  unsigned int non_ir_ref_dynamic;

	  for (p = htab->root.table.table[i]; p != NULL; p = p->next)
	    {
	      /* Preserve non_ir_ref_dynamic so that this symbol
		 will be exported when the dynamic lib becomes needed
		 in the second pass.  */
	      h = (struct elf_link_hash_entry *) p;
	      if (h->root.type == bfd_link_hash_warning)
		h = (struct elf_link_hash_entry *) h->root.u.i.link;
	      non_ir_ref_dynamic = h->root.non_ir_ref_dynamic;

	      h = (struct elf_link_hash_entry *) p;
	      memcpy (h, old_ent, htab->root.table.entsize);
	      old_ent = (char *) old_ent + htab->root.table.entsize;
	      if (h->root.type == bfd_link_hash_warning)
		{
		  h = (struct elf_link_hash_entry *) h->root.u.i.link;
		  memcpy (h, old_ent, htab->root.table.entsize);
		  old_ent = (char *) old_ent + htab->root.table.entsize;
		}
	      if (h->root.type == bfd_link_hash_common)
		{
		  memcpy (h->root.u.c.p, old_ent, sizeof (*h->root.u.c.p));
		  old_ent = (char *) old_ent + sizeof (*h->root.u.c.p);
		}
	      h->root.non_ir_ref_dynamic = non_ir_ref_dynamic;
	    }
	}

      /* Make a special call to the linker "notice" function to
	 tell it that symbols added for crefs may need to be removed.  */
      if (!(*bed->notice_as_needed) (abfd, info, notice_not_needed))
	goto error_free_vers;

      free (old_tab);
      objalloc_free_block ((struct objalloc *) htab->root.table.memory,
			   alloc_mark);
      free (nondeflt_vers);
      return true;
    }

  if (old_tab != NULL)
    {
      if (!(*bed->notice_as_needed) (abfd, info, notice_needed))
	goto error_free_vers;
      free (old_tab);
      old_tab = NULL;
    }

  /* Now that all the symbols from this input file are created, if
     not performing a relocatable link, handle .symver foo, foo@BAR
     such that any relocs against foo become foo@BAR.  */
  if (!bfd_link_relocatable (info) && nondeflt_vers != NULL)
    {
      size_t cnt, symidx;

      for (cnt = 0; cnt < nondeflt_vers_cnt; ++cnt)
	{
	  struct elf_link_hash_entry *h = nondeflt_vers[cnt], *hi;
	  char *shortname, *p;
	  size_t amt;

	  p = strchr (h->root.root.string, ELF_VER_CHR);
	  if (p == NULL
	      || (h->root.type != bfd_link_hash_defined
		  && h->root.type != bfd_link_hash_defweak))
	    continue;

	  amt = p - h->root.root.string;
	  shortname = (char *) bfd_malloc (amt + 1);
	  if (!shortname)
	    goto error_free_vers;
	  memcpy (shortname, h->root.root.string, amt);
	  shortname[amt] = '\0';

	  hi = (struct elf_link_hash_entry *)
	       bfd_link_hash_lookup (&htab->root, shortname,
				     false, false, false);
	  if (hi != NULL
	      && hi->root.type == h->root.type
	      && hi->root.u.def.value == h->root.u.def.value
	      && hi->root.u.def.section == h->root.u.def.section)
	    {
	      (*bed->elf_backend_hide_symbol) (info, hi, true);
	      hi->root.type = bfd_link_hash_indirect;
	      hi->root.u.i.link = (struct bfd_link_hash_entry *) h;
	      (*bed->elf_backend_copy_indirect_symbol) (info, h, hi);
	      sym_hash = elf_sym_hashes (abfd);
	      if (sym_hash)
		for (symidx = 0; symidx < extsymcount; ++symidx)
		  if (sym_hash[symidx] == hi)
		    {
		      sym_hash[symidx] = h;
		      break;
		    }
	    }
	  free (shortname);
	}
      free (nondeflt_vers);
      nondeflt_vers = NULL;
    }

  /* Now set the alias field correctly for all the weak defined
     symbols we found.  The only way to do this is to search all the
     symbols.  Since we only need the information for non functions in
     dynamic objects, that's the only time we actually put anything on
     the list WEAKS.  We need this information so that if a regular
     object refers to a symbol defined weakly in a dynamic object, the
     real symbol in the dynamic object is also put in the dynamic
     symbols; we also must arrange for both symbols to point to the
     same memory location.  We could handle the general case of symbol
     aliasing, but a general symbol alias can only be generated in
     assembler code, handling it correctly would be very time
     consuming, and other ELF linkers don't handle general aliasing
     either.  */
  if (weaks != NULL)
    {
      struct elf_link_hash_entry **hpp;
      struct elf_link_hash_entry **hppend;
      struct elf_link_hash_entry **sorted_sym_hash;
      struct elf_link_hash_entry *h;
      size_t sym_count, amt;

      /* Since we have to search the whole symbol list for each weak
	 defined symbol, search time for N weak defined symbols will be
	 O(N^2). Binary search will cut it down to O(NlogN).  */
      amt = extsymcount * sizeof (*sorted_sym_hash);
      sorted_sym_hash = bfd_malloc (amt);
      if (sorted_sym_hash == NULL)
	goto error_return;
      sym_hash = sorted_sym_hash;
      hpp = elf_sym_hashes (abfd);
      hppend = hpp + extsymcount;
      sym_count = 0;
      for (; hpp < hppend; hpp++)
	{
	  h = *hpp;
	  if (h != NULL
	      && h->root.type == bfd_link_hash_defined
	      && !bed->is_function_type (h->type))
	    {
	      *sym_hash = h;
	      sym_hash++;
	      sym_count++;
	    }
	}

      qsort (sorted_sym_hash, sym_count, sizeof (*sorted_sym_hash),
	     elf_sort_symbol);

      while (weaks != NULL)
	{
	  struct elf_link_hash_entry *hlook;
	  asection *slook;
	  bfd_vma vlook;
	  size_t i, j, idx = 0;

	  hlook = weaks;
	  weaks = hlook->u.alias;
	  hlook->u.alias = NULL;

	  if (hlook->root.type != bfd_link_hash_defined
	      && hlook->root.type != bfd_link_hash_defweak)
	    continue;

	  slook = hlook->root.u.def.section;
	  vlook = hlook->root.u.def.value;

	  i = 0;
	  j = sym_count;
	  while (i != j)
	    {
	      bfd_signed_vma vdiff;
	      idx = (i + j) / 2;
	      h = sorted_sym_hash[idx];
	      vdiff = vlook - h->root.u.def.value;
	      if (vdiff < 0)
		j = idx;
	      else if (vdiff > 0)
		i = idx + 1;
	      else
		{
		  int sdiff = slook->id - h->root.u.def.section->id;
		  if (sdiff < 0)
		    j = idx;
		  else if (sdiff > 0)
		    i = idx + 1;
		  else
		    break;
		}
	    }

	  /* We didn't find a value/section match.  */
	  if (i == j)
	    continue;

	  /* With multiple aliases, or when the weak symbol is already
	     strongly defined, we have multiple matching symbols and
	     the binary search above may land on any of them.  Step
	     one past the matching symbol(s).  */
	  while (++idx != j)
	    {
	      h = sorted_sym_hash[idx];
	      if (h->root.u.def.section != slook
		  || h->root.u.def.value != vlook)
		break;
	    }

	  /* Now look back over the aliases.  Since we sorted by size
	     as well as value and section, we'll choose the one with
	     the largest size.  */
	  while (idx-- != i)
	    {
	      h = sorted_sym_hash[idx];

	      /* Stop if value or section doesn't match.  */
	      if (h->root.u.def.section != slook
		  || h->root.u.def.value != vlook)
		break;
	      else if (h != hlook)
		{
		  struct elf_link_hash_entry *t;

		  hlook->u.alias = h;
		  hlook->is_weakalias = 1;
		  t = h;
		  if (t->u.alias != NULL)
		    while (t->u.alias != h)
		      t = t->u.alias;
		  t->u.alias = hlook;

		  /* If the weak definition is in the list of dynamic
		     symbols, make sure the real definition is put
		     there as well.  */
		  if (hlook->dynindx != -1 && h->dynindx == -1)
		    {
		      if (! bfd_elf_link_record_dynamic_symbol (info, h))
			{
			err_free_sym_hash:
			  free (sorted_sym_hash);
			  goto error_return;
			}
		    }

		  /* If the real definition is in the list of dynamic
		     symbols, make sure the weak definition is put
		     there as well.  If we don't do this, then the
		     dynamic loader might not merge the entries for the
		     real definition and the weak definition.  */
		  if (h->dynindx != -1 && hlook->dynindx == -1)
		    {
		      if (! bfd_elf_link_record_dynamic_symbol (info, hlook))
			goto err_free_sym_hash;
		    }
		  break;
		}
	    }
	}

      free (sorted_sym_hash);
    }

  if (bed->check_directives
      && !(*bed->check_directives) (abfd, info))
    return false;

  /* If this is a non-traditional link, try to optimize the handling
     of the .stab/.stabstr sections.  */
  if (! dynamic
      && ! info->traditional_format
      && is_elf_hash_table (&htab->root)
      && (info->strip != strip_all && info->strip != strip_debugger))
    {
      asection *stabstr;

      stabstr = bfd_get_section_by_name (abfd, ".stabstr");
      if (stabstr != NULL)
	{
	  bfd_size_type string_offset = 0;
	  asection *stab;

	  for (stab = abfd->sections; stab; stab = stab->next)
	    if (startswith (stab->name, ".stab")
		&& (!stab->name[5] ||
		    (stab->name[5] == '.' && ISDIGIT (stab->name[6])))
		&& (stab->flags & SEC_MERGE) == 0
		&& !bfd_is_abs_section (stab->output_section))
	      {
		struct bfd_elf_section_data *secdata;

		secdata = elf_section_data (stab);
		if (! _bfd_link_section_stabs (abfd, &htab->stab_info, stab,
					       stabstr, &secdata->sec_info,
					       &string_offset))
		  goto error_return;
		if (secdata->sec_info)
		  stab->sec_info_type = SEC_INFO_TYPE_STABS;
	    }
	}
    }

  if (dynamic && add_needed)
    {
      /* Add this bfd to the loaded list.  */
      struct elf_link_loaded_list *n;

      n = (struct elf_link_loaded_list *) bfd_alloc (abfd, sizeof (*n));
      if (n == NULL)
	goto error_return;
      n->abfd = abfd;
      n->next = htab->dyn_loaded;
      htab->dyn_loaded = n;
    }
  if (dynamic && !add_needed
      && (elf_dyn_lib_class (abfd) & DYN_DT_NEEDED) != 0)
    elf_dyn_lib_class (abfd) |= DYN_NO_NEEDED;

  return true;

 error_free_vers:
  free (old_tab);
  free (old_strtab);
  free (nondeflt_vers);
  free (extversym);
 error_free_sym:
  free (isymbuf);
 error_return:
  return false;
}

/* Return the linker hash table entry of a symbol that might be
   satisfied by an archive symbol.  Return -1 on error.  */

struct bfd_link_hash_entry *
_bfd_elf_archive_symbol_lookup (bfd *abfd,
				struct bfd_link_info *info,
				const char *name)
{
  struct bfd_link_hash_entry *h;
  char *p, *copy;
  size_t len, first;

  h = bfd_link_hash_lookup (info->hash, name, false, false, true);
  if (h != NULL)
    return h;

  /* If this is a default version (the name contains @@), look up the
     symbol again with only one `@' as well as without the version.
     The effect is that references to the symbol with and without the
     version will be matched by the default symbol in the archive.  */

  p = strchr (name, ELF_VER_CHR);
  if (p == NULL || p[1] != ELF_VER_CHR)
    return h;

  /* First check with only one `@'.  */
  len = strlen (name);
  copy = (char *) bfd_alloc (abfd, len);
  if (copy == NULL)
    return (struct bfd_link_hash_entry *) -1;

  first = p - name + 1;
  memcpy (copy, name, first);
  memcpy (copy + first, name + first + 1, len - first);

  h = bfd_link_hash_lookup (info->hash, copy, false, false, true);
  if (h == NULL)
    {
      /* We also need to check references to the symbol without the
	 version.  */
      copy[first - 1] = '\0';
      h = bfd_link_hash_lookup (info->hash, copy, false, false, true);
    }

  bfd_release (abfd, copy);
  return h;
}

/* Add symbols from an ELF archive file to the linker hash table.  We
   don't use _bfd_generic_link_add_archive_symbols because we need to
   handle versioned symbols.

   Fortunately, ELF archive handling is simpler than that done by
   _bfd_generic_link_add_archive_symbols, which has to allow for a.out
   oddities.  In ELF, if we find a symbol in the archive map, and the
   symbol is currently undefined, we know that we must pull in that
   object file.

   Unfortunately, we do have to make multiple passes over the symbol
   table until nothing further is resolved.  */

static bool
elf_link_add_archive_symbols (bfd *abfd, struct bfd_link_info *info)
{
  symindex c;
  unsigned char *included = NULL;
  carsym *symdefs;
  bool loop;
  size_t amt;
  const struct elf_backend_data *bed;
  struct bfd_link_hash_entry * (*archive_symbol_lookup)
    (bfd *, struct bfd_link_info *, const char *);

  if (! bfd_has_map (abfd))
    {
      /* An empty archive is a special case.  */
      if (bfd_openr_next_archived_file (abfd, NULL) == NULL)
	return true;
      bfd_set_error (bfd_error_no_armap);
      return false;
    }

  /* Keep track of all symbols we know to be already defined, and all
     files we know to be already included.  This is to speed up the
     second and subsequent passes.  */
  c = bfd_ardata (abfd)->symdef_count;
  if (c == 0)
    return true;
  amt = c * sizeof (*included);
  included = (unsigned char *) bfd_zmalloc (amt);
  if (included == NULL)
    return false;

  symdefs = bfd_ardata (abfd)->symdefs;
  bed = get_elf_backend_data (abfd);
  archive_symbol_lookup = bed->elf_backend_archive_symbol_lookup;

  do
    {
      file_ptr last;
      symindex i;
      carsym *symdef;
      carsym *symdefend;

      loop = false;
      last = -1;

      symdef = symdefs;
      symdefend = symdef + c;
      for (i = 0; symdef < symdefend; symdef++, i++)
	{
	  struct bfd_link_hash_entry *h;
	  bfd *element;
	  struct bfd_link_hash_entry *undefs_tail;
	  symindex mark;

	  if (included[i])
	    continue;
	  if (symdef->file_offset == last)
	    {
	      included[i] = true;
	      continue;
	    }

	  h = archive_symbol_lookup (abfd, info, symdef->name);
	  if (h == (struct bfd_link_hash_entry *) -1)
	    goto error_return;

	  if (h == NULL)
	    continue;

	  if (h->type == bfd_link_hash_undefined)
	    {
	      /* If the archive element has already been loaded then one
		 of the symbols defined by that element might have been
		 made undefined due to being in a discarded section.  */
	      if (is_elf_hash_table (info->hash)
		  && ((struct elf_link_hash_entry *) h)->indx == -3)
		continue;
	    }
	  else if (h->type == bfd_link_hash_common)
	    {
	      /* We currently have a common symbol.  The archive map contains
		 a reference to this symbol, so we may want to include it.  We
		 only want to include it however, if this archive element
		 contains a definition of the symbol, not just another common
		 declaration of it.

		 Unfortunately some archivers (including GNU ar) will put
		 declarations of common symbols into their archive maps, as
		 well as real definitions, so we cannot just go by the archive
		 map alone.  Instead we must read in the element's symbol
		 table and check that to see what kind of symbol definition
		 this is.  */
	      if (! elf_link_is_defined_archive_symbol (abfd, symdef))
		continue;
	    }
	  else
	    {
	      if (h->type != bfd_link_hash_undefweak)
		/* Symbol must be defined.  Don't check it again.  */
		included[i] = true;
	      continue;
	    }

	  /* We need to include this archive member.  */
	  element = _bfd_get_elt_at_filepos (abfd, symdef->file_offset,
					     info);
	  if (element == NULL)
	    goto error_return;

	  if (! bfd_check_format (element, bfd_object))
	    goto error_return;

	  undefs_tail = info->hash->undefs_tail;

	  if (!(*info->callbacks
		->add_archive_element) (info, element, symdef->name, &element))
	    continue;
	  if (!bfd_link_add_symbols (element, info))
	    goto error_return;

	  /* If there are any new undefined symbols, we need to make
	     another pass through the archive in order to see whether
	     they can be defined.  FIXME: This isn't perfect, because
	     common symbols wind up on undefs_tail and because an
	     undefined symbol which is defined later on in this pass
	     does not require another pass.  This isn't a bug, but it
	     does make the code less efficient than it could be.  */
	  if (undefs_tail != info->hash->undefs_tail)
	    loop = true;

	  /* Look backward to mark all symbols from this object file
	     which we have already seen in this pass.  */
	  mark = i;
	  do
	    {
	      included[mark] = true;
	      if (mark == 0)
		break;
	      --mark;
	    }
	  while (symdefs[mark].file_offset == symdef->file_offset);

	  /* We mark subsequent symbols from this object file as we go
	     on through the loop.  */
	  last = symdef->file_offset;
	}
    }
  while (loop);

  free (included);
  return true;

 error_return:
  free (included);
  return false;
}

/* Given an ELF BFD, add symbols to the global hash table as
   appropriate.  */

bool
bfd_elf_link_add_symbols (bfd *abfd, struct bfd_link_info *info)
{
  switch (bfd_get_format (abfd))
    {
    case bfd_object:
      return elf_link_add_object_symbols (abfd, info);
    case bfd_archive:
      return elf_link_add_archive_symbols (abfd, info);
    default:
      bfd_set_error (bfd_error_wrong_format);
      return false;
    }
}

struct hash_codes_info
{
  unsigned long *hashcodes;
  bool error;
};

/* This function will be called though elf_link_hash_traverse to store
   all hash value of the exported symbols in an array.  */

static bool
elf_collect_hash_codes (struct elf_link_hash_entry *h, void *data)
{
  struct hash_codes_info *inf = (struct hash_codes_info *) data;
  const char *name;
  unsigned long ha;
  char *alc = NULL;

  /* Ignore indirect symbols.  These are added by the versioning code.  */
  if (h->dynindx == -1)
    return true;

  name = h->root.root.string;
  if (h->versioned >= versioned)
    {
      char *p = strchr (name, ELF_VER_CHR);
      if (p != NULL)
	{
	  alc = (char *) bfd_malloc (p - name + 1);
	  if (alc == NULL)
	    {
	      inf->error = true;
	      return false;
	    }
	  memcpy (alc, name, p - name);
	  alc[p - name] = '\0';
	  name = alc;
	}
    }

  /* Compute the hash value.  */
  ha = bfd_elf_hash (name);

  /* Store the found hash value in the array given as the argument.  */
  *(inf->hashcodes)++ = ha;

  /* And store it in the struct so that we can put it in the hash table
     later.  */
  h->u.elf_hash_value = ha;

  free (alc);
  return true;
}

struct collect_gnu_hash_codes
{
  bfd *output_bfd;
  const struct elf_backend_data *bed;
  unsigned long int nsyms;
  unsigned long int maskbits;
  unsigned long int *hashcodes;
  unsigned long int *hashval;
  unsigned long int *indx;
  unsigned long int *counts;
  bfd_vma *bitmask;
  bfd_byte *contents;
  bfd_size_type xlat;
  long int min_dynindx;
  unsigned long int bucketcount;
  unsigned long int symindx;
  long int local_indx;
  long int shift1, shift2;
  unsigned long int mask;
  bool error;
};

/* This function will be called though elf_link_hash_traverse to store
   all hash value of the exported symbols in an array.  */

static bool
elf_collect_gnu_hash_codes (struct elf_link_hash_entry *h, void *data)
{
  struct collect_gnu_hash_codes *s = (struct collect_gnu_hash_codes *) data;
  const char *name;
  unsigned long ha;
  char *alc = NULL;

  /* Ignore indirect symbols.  These are added by the versioning code.  */
  if (h->dynindx == -1)
    return true;

  /* Ignore also local symbols and undefined symbols.  */
  if (! (*s->bed->elf_hash_symbol) (h))
    return true;

  name = h->root.root.string;
  if (h->versioned >= versioned)
    {
      char *p = strchr (name, ELF_VER_CHR);
      if (p != NULL)
	{
	  alc = (char *) bfd_malloc (p - name + 1);
	  if (alc == NULL)
	    {
	      s->error = true;
	      return false;
	    }
	  memcpy (alc, name, p - name);
	  alc[p - name] = '\0';
	  name = alc;
	}
    }

  /* Compute the hash value.  */
  ha = bfd_elf_gnu_hash (name);

  /* Store the found hash value in the array for compute_bucket_count,
     and also for .dynsym reordering purposes.  */
  s->hashcodes[s->nsyms] = ha;
  s->hashval[h->dynindx] = ha;
  ++s->nsyms;
  if (s->min_dynindx < 0 || s->min_dynindx > h->dynindx)
    s->min_dynindx = h->dynindx;

  free (alc);
  return true;
}

/* This function will be called though elf_link_hash_traverse to do
   final dynamic symbol renumbering in case of .gnu.hash.
   If using .MIPS.xhash, invoke record_xhash_symbol to add symbol index
   to the translation table.  */

static bool
elf_gnu_hash_process_symidx (struct elf_link_hash_entry *h, void *data)
{
  struct collect_gnu_hash_codes *s = (struct collect_gnu_hash_codes *) data;
  unsigned long int bucket;
  unsigned long int val;

  /* Ignore indirect symbols.  */
  if (h->dynindx == -1)
    return true;

  /* Ignore also local symbols and undefined symbols.  */
  if (! (*s->bed->elf_hash_symbol) (h))
    {
      if (h->dynindx >= s->min_dynindx)
	{
	  if (s->bed->record_xhash_symbol != NULL)
	    {
	      (*s->bed->record_xhash_symbol) (h, 0);
	      s->local_indx++;
	    }
	  else
	    h->dynindx = s->local_indx++;
	}
      return true;
    }

  bucket = s->hashval[h->dynindx] % s->bucketcount;
  val = (s->hashval[h->dynindx] >> s->shift1)
	& ((s->maskbits >> s->shift1) - 1);
  s->bitmask[val] |= ((bfd_vma) 1) << (s->hashval[h->dynindx] & s->mask);
  s->bitmask[val]
    |= ((bfd_vma) 1) << ((s->hashval[h->dynindx] >> s->shift2) & s->mask);
  val = s->hashval[h->dynindx] & ~(unsigned long int) 1;
  if (s->counts[bucket] == 1)
    /* Last element terminates the chain.  */
    val |= 1;
  bfd_put_32 (s->output_bfd, val,
	      s->contents + (s->indx[bucket] - s->symindx) * 4);
  --s->counts[bucket];
  if (s->bed->record_xhash_symbol != NULL)
    {
      bfd_vma xlat_loc = s->xlat + (s->indx[bucket]++ - s->symindx) * 4;

      (*s->bed->record_xhash_symbol) (h, xlat_loc);
    }
  else
    h->dynindx = s->indx[bucket]++;
  return true;
}

/* Return TRUE if symbol should be hashed in the `.gnu.hash' section.  */

bool
_bfd_elf_hash_symbol (struct elf_link_hash_entry *h)
{
  return !(h->forced_local
	   || h->root.type == bfd_link_hash_undefined
	   || h->root.type == bfd_link_hash_undefweak
	   || ((h->root.type == bfd_link_hash_defined
		|| h->root.type == bfd_link_hash_defweak)
	       && h->root.u.def.section->output_section == NULL));
}

/* Array used to determine the number of hash table buckets to use
   based on the number of symbols there are.  If there are fewer than
   3 symbols we use 1 bucket, fewer than 17 symbols we use 3 buckets,
   fewer than 37 we use 17 buckets, and so forth.  We never use more
   than 32771 buckets.  */

static const size_t elf_buckets[] =
{
  1, 3, 17, 37, 67, 97, 131, 197, 263, 521, 1031, 2053, 4099, 8209,
  16411, 32771, 0
};

/* Compute bucket count for hashing table.  We do not use a static set
   of possible tables sizes anymore.  Instead we determine for all
   possible reasonable sizes of the table the outcome (i.e., the
   number of collisions etc) and choose the best solution.  The
   weighting functions are not too simple to allow the table to grow
   without bounds.  Instead one of the weighting factors is the size.
   Therefore the result is always a good payoff between few collisions
   (= short chain lengths) and table size.  */
static size_t
compute_bucket_count (struct bfd_link_info *info ATTRIBUTE_UNUSED,
		      unsigned long int *hashcodes ATTRIBUTE_UNUSED,
		      unsigned long int nsyms,
		      int gnu_hash)
{
  size_t best_size = 0;
  unsigned long int i;

  if (info->optimize)
    {
      size_t minsize;
      size_t maxsize;
      uint64_t best_chlen = ~((uint64_t) 0);
      bfd *dynobj = elf_hash_table (info)->dynobj;
      size_t dynsymcount = elf_hash_table (info)->dynsymcount;
      const struct elf_backend_data *bed = get_elf_backend_data (dynobj);
      unsigned long int *counts;
      bfd_size_type amt;
      unsigned int no_improvement_count = 0;

      /* Possible optimization parameters: if we have NSYMS symbols we say
	 that the hashing table must at least have NSYMS/4 and at most
	 2*NSYMS buckets.  */
      minsize = nsyms / 4;
      if (minsize == 0)
	minsize = 1;
      best_size = maxsize = nsyms * 2;
      if (gnu_hash)
	{
	  if (minsize < 2)
	    minsize = 2;
	  if ((best_size & 31) == 0)
	    ++best_size;
	}

      /* Create array where we count the collisions in.  We must use bfd_malloc
	 since the size could be large.  */
      amt = maxsize;
      amt *= sizeof (unsigned long int);
      counts = (unsigned long int *) bfd_malloc (amt);
      if (counts == NULL)
	return 0;

      /* Compute the "optimal" size for the hash table.  The criteria is a
	 minimal chain length.  The minor criteria is (of course) the size
	 of the table.  */
      for (i = minsize; i < maxsize; ++i)
	{
	  /* Walk through the array of hashcodes and count the collisions.  */
	  uint64_t max;
	  unsigned long int j;
	  unsigned long int fact;

	  if (gnu_hash && (i & 31) == 0)
	    continue;

	  memset (counts, '\0', i * sizeof (unsigned long int));

	  /* Determine how often each hash bucket is used.  */
	  for (j = 0; j < nsyms; ++j)
	    ++counts[hashcodes[j] % i];

	  /* For the weight function we need some information about the
	     pagesize on the target.  This is information need not be 100%
	     accurate.  Since this information is not available (so far) we
	     define it here to a reasonable default value.  If it is crucial
	     to have a better value some day simply define this value.  */
# ifndef BFD_TARGET_PAGESIZE
#  define BFD_TARGET_PAGESIZE	(4096)
# endif

	  /* We in any case need 2 + DYNSYMCOUNT entries for the size values
	     and the chains.  */
	  max = (2 + dynsymcount) * bed->s->sizeof_hash_entry;

# if 1
	  /* Variant 1: optimize for short chains.  We add the squares
	     of all the chain lengths (which favors many small chain
	     over a few long chains).  */
	  for (j = 0; j < i; ++j)
	    max += counts[j] * counts[j];

	  /* This adds penalties for the overall size of the table.  */
	  fact = i / (BFD_TARGET_PAGESIZE / bed->s->sizeof_hash_entry) + 1;
	  max *= fact * fact;
# else
	  /* Variant 2: Optimize a lot more for small table.  Here we
	     also add squares of the size but we also add penalties for
	     empty slots (the +1 term).  */
	  for (j = 0; j < i; ++j)
	    max += (1 + counts[j]) * (1 + counts[j]);

	  /* The overall size of the table is considered, but not as
	     strong as in variant 1, where it is squared.  */
	  fact = i / (BFD_TARGET_PAGESIZE / bed->s->sizeof_hash_entry) + 1;
	  max *= fact;
# endif

	  /* Compare with current best results.  */
	  if (max < best_chlen)
	    {
	      best_chlen = max;
	      best_size = i;
	      no_improvement_count = 0;
	    }
	  /* PR 11843: Avoid futile long searches for the best bucket size
	     when there are a large number of symbols.  */
	  else if (++no_improvement_count == 100)
	    break;
	}

      free (counts);
    }
  else
    {
      for (i = 0; elf_buckets[i] != 0; i++)
	{
	  best_size = elf_buckets[i];
	  if (nsyms < elf_buckets[i + 1])
	    break;
	}
      if (gnu_hash && best_size < 2)
	best_size = 2;
    }

  return best_size;
}

/* Size any SHT_GROUP section for ld -r.  */

bool
_bfd_elf_size_group_sections (struct bfd_link_info *info)
{
  bfd *ibfd;
  asection *s;

  for (ibfd = info->input_bfds; ibfd != NULL; ibfd = ibfd->link.next)
    if (bfd_get_flavour (ibfd) == bfd_target_elf_flavour
	&& (s = ibfd->sections) != NULL
	&& s->sec_info_type != SEC_INFO_TYPE_JUST_SYMS
	&& !_bfd_elf_fixup_group_sections (ibfd, bfd_abs_section_ptr))
      return false;
  return true;
}

/* Set a default stack segment size.  The value in INFO wins.  If it
   is unset, LEGACY_SYMBOL's value is used, and if that symbol is
   undefined it is initialized.  */

bool
bfd_elf_stack_segment_size (bfd *output_bfd,
			    struct bfd_link_info *info,
			    const char *legacy_symbol,
			    bfd_vma default_size)
{
  struct elf_link_hash_entry *h = NULL;

  /* Look for legacy symbol.  */
  if (legacy_symbol)
    h = elf_link_hash_lookup (elf_hash_table (info), legacy_symbol,
			      false, false, false);
  if (h && (h->root.type == bfd_link_hash_defined
	    || h->root.type == bfd_link_hash_defweak)
      && h->def_regular
      && (h->type == STT_NOTYPE || h->type == STT_OBJECT))
    {
      /* The symbol has no type if specified on the command line.  */
      h->type = STT_OBJECT;
      if (info->stacksize)
	/* xgettext:c-format */
	_bfd_error_handler (_("%pB: stack size specified and %s set"),
			    output_bfd, legacy_symbol);
      else if (h->root.u.def.section != bfd_abs_section_ptr)
	/* xgettext:c-format */
	_bfd_error_handler (_("%pB: %s not absolute"),
			    output_bfd, legacy_symbol);
      else
	info->stacksize = h->root.u.def.value;
    }

  if (!info->stacksize)
    /* If the user didn't set a size, or explicitly inhibit the
       size, set it now.  */
    info->stacksize = default_size;

  /* Provide the legacy symbol, if it is referenced.  */
  if (h && (h->root.type == bfd_link_hash_undefined
	    || h->root.type == bfd_link_hash_undefweak))
    {
      struct bfd_link_hash_entry *bh = NULL;

      if (!(_bfd_generic_link_add_one_symbol
	    (info, output_bfd, legacy_symbol,
	     BSF_GLOBAL, bfd_abs_section_ptr,
	     info->stacksize >= 0 ? info->stacksize : 0,
	     NULL, false, get_elf_backend_data (output_bfd)->collect, &bh)))
	return false;

      h = (struct elf_link_hash_entry *) bh;
      h->def_regular = 1;
      h->type = STT_OBJECT;
    }

  return true;
}

/* Sweep symbols in swept sections.  Called via elf_link_hash_traverse.  */

struct elf_gc_sweep_symbol_info
{
  struct bfd_link_info *info;
  void (*hide_symbol) (struct bfd_link_info *, struct elf_link_hash_entry *,
		       bool);
};

static bool
elf_gc_sweep_symbol (struct elf_link_hash_entry *h, void *data)
{
  if (!h->mark
      && (((h->root.type == bfd_link_hash_defined
	    || h->root.type == bfd_link_hash_defweak)
	   && !((h->def_regular || ELF_COMMON_DEF_P (h))
		&& h->root.u.def.section->gc_mark))
	  || h->root.type == bfd_link_hash_undefined
	  || h->root.type == bfd_link_hash_undefweak))
    {
      struct elf_gc_sweep_symbol_info *inf;

      inf = (struct elf_gc_sweep_symbol_info *) data;
      (*inf->hide_symbol) (inf->info, h, true);
      h->def_regular = 0;
      h->ref_regular = 0;
      h->ref_regular_nonweak = 0;
    }

  return true;
}

/* Set up the sizes and contents of the ELF dynamic sections.  This is
   called by the ELF linker emulation before_allocation routine.  We
   must set the sizes of the sections before the linker sets the
   addresses of the various sections.  */

bool
bfd_elf_size_dynamic_sections (bfd *output_bfd,
			       const char *soname,
			       const char *rpath,
			       const char *filter_shlib,
			       const char *audit,
			       const char *depaudit,
			       const char * const *auxiliary_filters,
			       struct bfd_link_info *info,
			       asection **sinterpptr)
{
  bfd *dynobj;
  const struct elf_backend_data *bed;

  *sinterpptr = NULL;

  if (!is_elf_hash_table (info->hash))
    return true;

  /* Any syms created from now on start with -1 in
     got.refcount/offset and plt.refcount/offset.  */
  elf_hash_table (info)->init_got_refcount
    = elf_hash_table (info)->init_got_offset;
  elf_hash_table (info)->init_plt_refcount
    = elf_hash_table (info)->init_plt_offset;

  bed = get_elf_backend_data (output_bfd);

  /* The backend may have to create some sections regardless of whether
     we're dynamic or not.  */
  if (bed->elf_backend_always_size_sections
      && ! (*bed->elf_backend_always_size_sections) (output_bfd, info))
    return false;

  dynobj = elf_hash_table (info)->dynobj;

  if (dynobj != NULL && elf_hash_table (info)->dynamic_sections_created)
    {
      struct bfd_elf_version_tree *verdefs;
      struct elf_info_failed asvinfo;
      struct bfd_elf_version_tree *t;
      struct bfd_elf_version_expr *d;
      asection *s;
      size_t soname_indx;

      /* If we are supposed to export all symbols into the dynamic symbol
	 table (this is not the normal case), then do so.  */
      if (info->export_dynamic
	  || (bfd_link_executable (info) && info->dynamic))
	{
	  struct elf_info_failed eif;

	  eif.info = info;
	  eif.failed = false;
	  elf_link_hash_traverse (elf_hash_table (info),
				  _bfd_elf_export_symbol,
				  &eif);
	  if (eif.failed)
	    return false;
	}

      if (soname != NULL)
	{
	  soname_indx = _bfd_elf_strtab_add (elf_hash_table (info)->dynstr,
					     soname, true);
	  if (soname_indx == (size_t) -1
	      || !_bfd_elf_add_dynamic_entry (info, DT_SONAME, soname_indx))
	    return false;
	}
      else
	soname_indx = (size_t) -1;

      /* Make all global versions with definition.  */
      for (t = info->version_info; t != NULL; t = t->next)
	for (d = t->globals.list; d != NULL; d = d->next)
	  if (!d->symver && d->literal)
	    {
	      const char *verstr, *name;
	      size_t namelen, verlen, newlen;
	      char *newname, *p, leading_char;
	      struct elf_link_hash_entry *newh;

	      leading_char = bfd_get_symbol_leading_char (output_bfd);
	      name = d->pattern;
	      namelen = strlen (name) + (leading_char != '\0');
	      verstr = t->name;
	      verlen = strlen (verstr);
	      newlen = namelen + verlen + 3;

	      newname = (char *) bfd_malloc (newlen);
	      if (newname == NULL)
		return false;
	      newname[0] = leading_char;
	      memcpy (newname + (leading_char != '\0'), name, namelen);

	      /* Check the hidden versioned definition.  */
	      p = newname + namelen;
	      *p++ = ELF_VER_CHR;
	      memcpy (p, verstr, verlen + 1);
	      newh = elf_link_hash_lookup (elf_hash_table (info),
					   newname, false, false,
					   false);
	      if (newh == NULL
		  || (newh->root.type != bfd_link_hash_defined
		      && newh->root.type != bfd_link_hash_defweak))
		{
		  /* Check the default versioned definition.  */
		  *p++ = ELF_VER_CHR;
		  memcpy (p, verstr, verlen + 1);
		  newh = elf_link_hash_lookup (elf_hash_table (info),
					       newname, false, false,
					       false);
		}
	      free (newname);

	      /* Mark this version if there is a definition and it is
		 not defined in a shared object.  */
	      if (newh != NULL
		  && !newh->def_dynamic
		  && (newh->root.type == bfd_link_hash_defined
		      || newh->root.type == bfd_link_hash_defweak))
		d->symver = 1;
	    }

      /* Attach all the symbols to their version information.  */
      asvinfo.info = info;
      asvinfo.failed = false;

      elf_link_hash_traverse (elf_hash_table (info),
			      _bfd_elf_link_assign_sym_version,
			      &asvinfo);
      if (asvinfo.failed)
	return false;

      if (!info->allow_undefined_version)
	{
	  /* Check if all global versions have a definition.  */
	  bool all_defined = true;
	  for (t = info->version_info; t != NULL; t = t->next)
	    for (d = t->globals.list; d != NULL; d = d->next)
	      if (d->literal && !d->symver && !d->script)
		{
		  _bfd_error_handler
		    (_("%s: undefined version: %s"),
		     d->pattern, t->name);
		  all_defined = false;
		}

	  if (!all_defined)
	    {
	      bfd_set_error (bfd_error_bad_value);
	      return false;
	    }
	}

      /* Set up the version definition section.  */
      s = bfd_get_linker_section (dynobj, ".gnu.version_d");
      BFD_ASSERT (s != NULL);

      /* We may have created additional version definitions if we are
	 just linking a regular application.  */
      verdefs = info->version_info;

      /* Skip anonymous version tag.  */
      if (verdefs != NULL && verdefs->vernum == 0)
	verdefs = verdefs->next;

      if (verdefs == NULL && !info->create_default_symver)
	s->flags |= SEC_EXCLUDE;
      else
	{
	  unsigned int cdefs;
	  bfd_size_type size;
	  bfd_byte *p;
	  Elf_Internal_Verdef def;
	  Elf_Internal_Verdaux defaux;
	  struct bfd_link_hash_entry *bh;
	  struct elf_link_hash_entry *h;
	  const char *name;

	  cdefs = 0;
	  size = 0;

	  /* Make space for the base version.  */
	  size += sizeof (Elf_External_Verdef);
	  size += sizeof (Elf_External_Verdaux);
	  ++cdefs;

	  /* Make space for the default version.  */
	  if (info->create_default_symver)
	    {
	      size += sizeof (Elf_External_Verdef);
	      ++cdefs;
	    }

	  for (t = verdefs; t != NULL; t = t->next)
	    {
	      struct bfd_elf_version_deps *n;

	      /* Don't emit base version twice.  */
	      if (t->vernum == 0)
		continue;

	      size += sizeof (Elf_External_Verdef);
	      size += sizeof (Elf_External_Verdaux);
	      ++cdefs;

	      for (n = t->deps; n != NULL; n = n->next)
		size += sizeof (Elf_External_Verdaux);
	    }

	  s->size = size;
	  s->contents = (unsigned char *) bfd_alloc (output_bfd, s->size);
	  if (s->contents == NULL && s->size != 0)
	    return false;

	  /* Fill in the version definition section.  */

	  p = s->contents;

	  def.vd_version = VER_DEF_CURRENT;
	  def.vd_flags = VER_FLG_BASE;
	  def.vd_ndx = 1;
	  def.vd_cnt = 1;
	  if (info->create_default_symver)
	    {
	      def.vd_aux = 2 * sizeof (Elf_External_Verdef);
	      def.vd_next = sizeof (Elf_External_Verdef);
	    }
	  else
	    {
	      def.vd_aux = sizeof (Elf_External_Verdef);
	      def.vd_next = (sizeof (Elf_External_Verdef)
			     + sizeof (Elf_External_Verdaux));
	    }

	  if (soname_indx != (size_t) -1)
	    {
	      _bfd_elf_strtab_addref (elf_hash_table (info)->dynstr,
				      soname_indx);
	      def.vd_hash = bfd_elf_hash (soname);
	      defaux.vda_name = soname_indx;
	      name = soname;
	    }
	  else
	    {
	      size_t indx;

	      name = lbasename (bfd_get_filename (output_bfd));
	      def.vd_hash = bfd_elf_hash (name);
	      indx = _bfd_elf_strtab_add (elf_hash_table (info)->dynstr,
					  name, false);
	      if (indx == (size_t) -1)
		return false;
	      defaux.vda_name = indx;
	    }
	  defaux.vda_next = 0;

	  _bfd_elf_swap_verdef_out (output_bfd, &def,
				    (Elf_External_Verdef *) p);
	  p += sizeof (Elf_External_Verdef);
	  if (info->create_default_symver)
	    {
	      /* Add a symbol representing this version.  */
	      bh = NULL;
	      if (! (_bfd_generic_link_add_one_symbol
		     (info, dynobj, name, BSF_GLOBAL, bfd_abs_section_ptr,
		      0, NULL, false,
		      get_elf_backend_data (dynobj)->collect, &bh)))
		return false;
	      h = (struct elf_link_hash_entry *) bh;
	      h->non_elf = 0;
	      h->def_regular = 1;
	      h->type = STT_OBJECT;
	      h->verinfo.vertree = NULL;

	      if (! bfd_elf_link_record_dynamic_symbol (info, h))
		return false;

	      /* Create a duplicate of the base version with the same
		 aux block, but different flags.  */
	      def.vd_flags = 0;
	      def.vd_ndx = 2;
	      def.vd_aux = sizeof (Elf_External_Verdef);
	      if (verdefs)
		def.vd_next = (sizeof (Elf_External_Verdef)
			       + sizeof (Elf_External_Verdaux));
	      else
		def.vd_next = 0;
	      _bfd_elf_swap_verdef_out (output_bfd, &def,
					(Elf_External_Verdef *) p);
	      p += sizeof (Elf_External_Verdef);
	    }
	  _bfd_elf_swap_verdaux_out (output_bfd, &defaux,
				     (Elf_External_Verdaux *) p);
	  p += sizeof (Elf_External_Verdaux);

	  for (t = verdefs; t != NULL; t = t->next)
	    {
	      unsigned int cdeps;
	      struct bfd_elf_version_deps *n;

	      /* Don't emit the base version twice.  */
	      if (t->vernum == 0)
		continue;

	      cdeps = 0;
	      for (n = t->deps; n != NULL; n = n->next)
		++cdeps;

	      /* Add a symbol representing this version.  */
	      bh = NULL;
	      if (! (_bfd_generic_link_add_one_symbol
		     (info, dynobj, t->name, BSF_GLOBAL, bfd_abs_section_ptr,
		      0, NULL, false,
		      get_elf_backend_data (dynobj)->collect, &bh)))
		return false;
	      h = (struct elf_link_hash_entry *) bh;
	      h->non_elf = 0;
	      h->def_regular = 1;
	      h->type = STT_OBJECT;
	      h->verinfo.vertree = t;

	      if (! bfd_elf_link_record_dynamic_symbol (info, h))
		return false;

	      def.vd_version = VER_DEF_CURRENT;
	      def.vd_flags = 0;
	      if (t->globals.list == NULL
		  && t->locals.list == NULL
		  && ! t->used)
		def.vd_flags |= VER_FLG_WEAK;
	      def.vd_ndx = t->vernum + (info->create_default_symver ? 2 : 1);
	      def.vd_cnt = cdeps + 1;
	      def.vd_hash = bfd_elf_hash (t->name);
	      def.vd_aux = sizeof (Elf_External_Verdef);
	      def.vd_next = 0;

	      /* If a basever node is next, it *must* be the last node in
		 the chain, otherwise Verdef construction breaks.  */
	      if (t->next != NULL && t->next->vernum == 0)
		BFD_ASSERT (t->next->next == NULL);

	      if (t->next != NULL && t->next->vernum != 0)
		def.vd_next = (sizeof (Elf_External_Verdef)
			       + (cdeps + 1) * sizeof (Elf_External_Verdaux));

	      _bfd_elf_swap_verdef_out (output_bfd, &def,
					(Elf_External_Verdef *) p);
	      p += sizeof (Elf_External_Verdef);

	      defaux.vda_name = h->dynstr_index;
	      _bfd_elf_strtab_addref (elf_hash_table (info)->dynstr,
				      h->dynstr_index);
	      defaux.vda_next = 0;
	      if (t->deps != NULL)
		defaux.vda_next = sizeof (Elf_External_Verdaux);
	      t->name_indx = defaux.vda_name;

	      _bfd_elf_swap_verdaux_out (output_bfd, &defaux,
					 (Elf_External_Verdaux *) p);
	      p += sizeof (Elf_External_Verdaux);

	      for (n = t->deps; n != NULL; n = n->next)
		{
		  if (n->version_needed == NULL)
		    {
		      /* This can happen if there was an error in the
			 version script.  */
		      defaux.vda_name = 0;
		    }
		  else
		    {
		      defaux.vda_name = n->version_needed->name_indx;
		      _bfd_elf_strtab_addref (elf_hash_table (info)->dynstr,
					      defaux.vda_name);
		    }
		  if (n->next == NULL)
		    defaux.vda_next = 0;
		  else
		    defaux.vda_next = sizeof (Elf_External_Verdaux);

		  _bfd_elf_swap_verdaux_out (output_bfd, &defaux,
					     (Elf_External_Verdaux *) p);
		  p += sizeof (Elf_External_Verdaux);
		}
	    }

	  elf_tdata (output_bfd)->cverdefs = cdefs;
	}
    }

  if (info->gc_sections && bed->can_gc_sections)
    {
      struct elf_gc_sweep_symbol_info sweep_info;

      /* Remove the symbols that were in the swept sections from the
	 dynamic symbol table.  */
      sweep_info.info = info;
      sweep_info.hide_symbol = bed->elf_backend_hide_symbol;
      elf_link_hash_traverse (elf_hash_table (info), elf_gc_sweep_symbol,
			      &sweep_info);
    }

  if (dynobj != NULL && elf_hash_table (info)->dynamic_sections_created)
    {
      asection *s;
      struct elf_find_verdep_info sinfo;

      /* Work out the size of the version reference section.  */

      s = bfd_get_linker_section (dynobj, ".gnu.version_r");
      BFD_ASSERT (s != NULL);

      sinfo.info = info;
      sinfo.vers = elf_tdata (output_bfd)->cverdefs;
      if (sinfo.vers == 0)
	sinfo.vers = 1;
      sinfo.failed = false;

      elf_link_hash_traverse (elf_hash_table (info),
			      _bfd_elf_link_find_version_dependencies,
			      &sinfo);
      if (sinfo.failed)
	return false;

      if (info->enable_dt_relr)
	{
	  elf_link_add_dt_relr_dependency (&sinfo);
	  if (sinfo.failed)
	    return false;
	}

      if (elf_tdata (output_bfd)->verref == NULL)
	s->flags |= SEC_EXCLUDE;
      else
	{
	  Elf_Internal_Verneed *vn;
	  unsigned int size;
	  unsigned int crefs;
	  bfd_byte *p;

	  /* Build the version dependency section.  */
	  size = 0;
	  crefs = 0;
	  for (vn = elf_tdata (output_bfd)->verref;
	       vn != NULL;
	       vn = vn->vn_nextref)
	    {
	      Elf_Internal_Vernaux *a;

	      size += sizeof (Elf_External_Verneed);
	      ++crefs;
	      for (a = vn->vn_auxptr; a != NULL; a = a->vna_nextptr)
		size += sizeof (Elf_External_Vernaux);
	    }

	  s->size = size;
	  s->contents = (unsigned char *) bfd_alloc (output_bfd, s->size);
	  if (s->contents == NULL)
	    return false;

	  p = s->contents;
	  for (vn = elf_tdata (output_bfd)->verref;
	       vn != NULL;
	       vn = vn->vn_nextref)
	    {
	      unsigned int caux;
	      Elf_Internal_Vernaux *a;
	      size_t indx;

	      caux = 0;
	      for (a = vn->vn_auxptr; a != NULL; a = a->vna_nextptr)
		++caux;

	      vn->vn_version = VER_NEED_CURRENT;
	      vn->vn_cnt = caux;
	      indx = _bfd_elf_strtab_add (elf_hash_table (info)->dynstr,
					  elf_dt_name (vn->vn_bfd) != NULL
					  ? elf_dt_name (vn->vn_bfd)
					  : lbasename (bfd_get_filename
						       (vn->vn_bfd)),
					  false);
	      if (indx == (size_t) -1)
		return false;
	      vn->vn_file = indx;
	      vn->vn_aux = sizeof (Elf_External_Verneed);
	      if (vn->vn_nextref == NULL)
		vn->vn_next = 0;
	      else
		vn->vn_next = (sizeof (Elf_External_Verneed)
			       + caux * sizeof (Elf_External_Vernaux));

	      _bfd_elf_swap_verneed_out (output_bfd, vn,
					 (Elf_External_Verneed *) p);
	      p += sizeof (Elf_External_Verneed);

	      for (a = vn->vn_auxptr; a != NULL; a = a->vna_nextptr)
		{
		  a->vna_hash = bfd_elf_hash (a->vna_nodename);
		  indx = _bfd_elf_strtab_add (elf_hash_table (info)->dynstr,
					      a->vna_nodename, false);
		  if (indx == (size_t) -1)
		    return false;
		  a->vna_name = indx;
		  if (a->vna_nextptr == NULL)
		    a->vna_next = 0;
		  else
		    a->vna_next = sizeof (Elf_External_Vernaux);

		  _bfd_elf_swap_vernaux_out (output_bfd, a,
					     (Elf_External_Vernaux *) p);
		  p += sizeof (Elf_External_Vernaux);
		}
	    }

	  elf_tdata (output_bfd)->cverrefs = crefs;
	}
    }

  if (bfd_link_relocatable (info)
      && !_bfd_elf_size_group_sections (info))
    return false;

  /* Determine any GNU_STACK segment requirements, after the backend
     has had a chance to set a default segment size.  */
  if (info->execstack)
    {
      /* If the user has explicitly requested warnings, then generate one even
	 though the choice is the result of another command line option.  */
      if (info->warn_execstack == 1)
	_bfd_error_handler
	  (_("\
warning: enabling an executable stack because of -z execstack command line option"));
      elf_stack_flags (output_bfd) = PF_R | PF_W | PF_X;
    }
  else if (info->noexecstack)
    elf_stack_flags (output_bfd) = PF_R | PF_W;
  else
    {
      bfd *inputobj;
      asection *notesec = NULL;
      bfd *noteobj = NULL;
      bfd *emptyobj = NULL;
      int exec = 0;

      for (inputobj = info->input_bfds;
	   inputobj;
	   inputobj = inputobj->link.next)
	{
	  asection *s;

	  if (inputobj->flags
	      & (DYNAMIC | EXEC_P | BFD_PLUGIN | BFD_LINKER_CREATED))
	    continue;
	  s = inputobj->sections;
	  if (s == NULL || s->sec_info_type == SEC_INFO_TYPE_JUST_SYMS)
	    continue;

	  s = bfd_get_section_by_name (inputobj, ".note.GNU-stack");
	  if (s)
	    {
	      notesec = s;
	      if (s->flags & SEC_CODE)
		{
		  noteobj = inputobj;
		  exec = PF_X;
		  /* There is no point in scanning the remaining bfds.  */
		  break;
		}
	    }
	  else if (bed->default_execstack && info->default_execstack)
	    {
	      exec = PF_X;
	      emptyobj = inputobj;
	    }
	}

      if (notesec || info->stacksize > 0)
	{
	  if (exec)
	    {
	      if (info->warn_execstack != 0)
		{
		  /* PR 29072: Because an executable stack is a serious
		     security risk, make sure that the user knows that it is
		     being enabled despite the fact that it was not requested
		     on the command line.  */
		  if (noteobj)
		    _bfd_error_handler (_("\
warning: %s: requires executable stack (because the .note.GNU-stack section is executable)"),
		       bfd_get_filename (noteobj));
		  else if (emptyobj)
		    {
		      _bfd_error_handler (_("\
warning: %s: missing .note.GNU-stack section implies executable stack"),
					  bfd_get_filename (emptyobj));
		      _bfd_error_handler (_("\
NOTE: This behaviour is deprecated and will be removed in a future version of the linker"));
		    }
		}
	    }
	  elf_stack_flags (output_bfd) = PF_R | PF_W | exec;
	}

      if (notesec && exec && bfd_link_relocatable (info)
	  && notesec->output_section != bfd_abs_section_ptr)
	notesec->output_section->flags |= SEC_CODE;
    }

  if (dynobj != NULL && elf_hash_table (info)->dynamic_sections_created)
    {
      struct elf_info_failed eif;
      struct elf_link_hash_entry *h;
      asection *dynstr;
      asection *s;

      *sinterpptr = bfd_get_linker_section (dynobj, ".interp");
      BFD_ASSERT (*sinterpptr != NULL || !bfd_link_executable (info) || info->nointerp);

      if (info->symbolic)
	{
	  if (!_bfd_elf_add_dynamic_entry (info, DT_SYMBOLIC, 0))
	    return false;
	  info->flags |= DF_SYMBOLIC;
	}

      if (rpath != NULL)
	{
	  size_t indx;
	  bfd_vma tag;

	  indx = _bfd_elf_strtab_add (elf_hash_table (info)->dynstr, rpath,
				      true);
	  if (indx == (size_t) -1)
	    return false;

	  tag = info->new_dtags ? DT_RUNPATH : DT_RPATH;
	  if (!_bfd_elf_add_dynamic_entry (info, tag, indx))
	    return false;
	}

      if (filter_shlib != NULL)
	{
	  size_t indx;

	  indx = _bfd_elf_strtab_add (elf_hash_table (info)->dynstr,
				      filter_shlib, true);
	  if (indx == (size_t) -1
	      || !_bfd_elf_add_dynamic_entry (info, DT_FILTER, indx))
	    return false;
	}

      if (auxiliary_filters != NULL)
	{
	  const char * const *p;

	  for (p = auxiliary_filters; *p != NULL; p++)
	    {
	      size_t indx;

	      indx = _bfd_elf_strtab_add (elf_hash_table (info)->dynstr,
					  *p, true);
	      if (indx == (size_t) -1
		  || !_bfd_elf_add_dynamic_entry (info, DT_AUXILIARY, indx))
		return false;
	    }
	}

      if (audit != NULL)
	{
	  size_t indx;

	  indx = _bfd_elf_strtab_add (elf_hash_table (info)->dynstr, audit,
				      true);
	  if (indx == (size_t) -1
	      || !_bfd_elf_add_dynamic_entry (info, DT_AUDIT, indx))
	    return false;
	}

      if (depaudit != NULL)
	{
	  size_t indx;

	  indx = _bfd_elf_strtab_add (elf_hash_table (info)->dynstr, depaudit,
				      true);
	  if (indx == (size_t) -1
	      || !_bfd_elf_add_dynamic_entry (info, DT_DEPAUDIT, indx))
	    return false;
	}

      eif.info = info;
      eif.failed = false;

      /* Find all symbols which were defined in a dynamic object and make
	 the backend pick a reasonable value for them.  */
      elf_link_hash_traverse (elf_hash_table (info),
			      _bfd_elf_adjust_dynamic_symbol,
			      &eif);
      if (eif.failed)
	return false;

      /* Add some entries to the .dynamic section.  We fill in some of the
	 values later, in bfd_elf_final_link, but we must add the entries
	 now so that we know the final size of the .dynamic section.  */

      /* If there are initialization and/or finalization functions to
	 call then add the corresponding DT_INIT/DT_FINI entries.  */
      h = (info->init_function
	   ? elf_link_hash_lookup (elf_hash_table (info),
				   info->init_function, false,
				   false, false)
	   : NULL);
      if (h != NULL
	  && (h->ref_regular
	      || h->def_regular))
	{
	  if (!_bfd_elf_add_dynamic_entry (info, DT_INIT, 0))
	    return false;
	}
      h = (info->fini_function
	   ? elf_link_hash_lookup (elf_hash_table (info),
				   info->fini_function, false,
				   false, false)
	   : NULL);
      if (h != NULL
	  && (h->ref_regular
	      || h->def_regular))
	{
	  if (!_bfd_elf_add_dynamic_entry (info, DT_FINI, 0))
	    return false;
	}

      s = bfd_get_section_by_name (output_bfd, ".preinit_array");
      if (s != NULL && s->linker_has_input)
	{
	  /* DT_PREINIT_ARRAY is not allowed in shared library.  */
	  if (! bfd_link_executable (info))
	    {
	      bfd *sub;
	      asection *o;

	      for (sub = info->input_bfds; sub != NULL; sub = sub->link.next)
		if (bfd_get_flavour (sub) == bfd_target_elf_flavour
		    && (o = sub->sections) != NULL
		    && o->sec_info_type != SEC_INFO_TYPE_JUST_SYMS)
		  for (o = sub->sections; o != NULL; o = o->next)
		    if (elf_section_data (o)->this_hdr.sh_type
			== SHT_PREINIT_ARRAY)
		      {
			_bfd_error_handler
			  (_("%pB: .preinit_array section is not allowed in DSO"),
			   sub);
			break;
		      }

	      bfd_set_error (bfd_error_nonrepresentable_section);
	      return false;
	    }

	  if (!_bfd_elf_add_dynamic_entry (info, DT_PREINIT_ARRAY, 0)
	      || !_bfd_elf_add_dynamic_entry (info, DT_PREINIT_ARRAYSZ, 0))
	    return false;
	}
      s = bfd_get_section_by_name (output_bfd, ".init_array");
      if (s != NULL && s->linker_has_input)
	{
	  if (!_bfd_elf_add_dynamic_entry (info, DT_INIT_ARRAY, 0)
	      || !_bfd_elf_add_dynamic_entry (info, DT_INIT_ARRAYSZ, 0))
	    return false;
	}
      s = bfd_get_section_by_name (output_bfd, ".fini_array");
      if (s != NULL && s->linker_has_input)
	{
	  if (!_bfd_elf_add_dynamic_entry (info, DT_FINI_ARRAY, 0)
	      || !_bfd_elf_add_dynamic_entry (info, DT_FINI_ARRAYSZ, 0))
	    return false;
	}

      dynstr = bfd_get_linker_section (dynobj, ".dynstr");
      /* If .dynstr is excluded from the link, we don't want any of
	 these tags.  Strictly, we should be checking each section
	 individually;  This quick check covers for the case where
	 someone does a /DISCARD/ : { *(*) }.  */
      if (dynstr != NULL && dynstr->output_section != bfd_abs_section_ptr)
	{
	  bfd_size_type strsize;

	  strsize = _bfd_elf_strtab_size (elf_hash_table (info)->dynstr);
	  if ((info->emit_hash
	       && !_bfd_elf_add_dynamic_entry (info, DT_HASH, 0))
	      || (info->emit_gnu_hash
		  && (bed->record_xhash_symbol == NULL
		      && !_bfd_elf_add_dynamic_entry (info, DT_GNU_HASH, 0)))
	      || !_bfd_elf_add_dynamic_entry (info, DT_STRTAB, 0)
	      || !_bfd_elf_add_dynamic_entry (info, DT_SYMTAB, 0)
	      || !_bfd_elf_add_dynamic_entry (info, DT_STRSZ, strsize)
	      || !_bfd_elf_add_dynamic_entry (info, DT_SYMENT,
					      bed->s->sizeof_sym)
	      || (info->gnu_flags_1
		  && !_bfd_elf_add_dynamic_entry (info, DT_GNU_FLAGS_1,
						  info->gnu_flags_1)))
	    return false;
	}
    }

  if (! _bfd_elf_maybe_strip_eh_frame_hdr (info))
    return false;

  /* The backend must work out the sizes of all the other dynamic
     sections.  */
  if (dynobj != NULL
      && bed->elf_backend_size_dynamic_sections != NULL
      && ! (*bed->elf_backend_size_dynamic_sections) (output_bfd, info))
    return false;

  if (dynobj != NULL && elf_hash_table (info)->dynamic_sections_created)
    {
      if (elf_tdata (output_bfd)->cverdefs)
	{
	  unsigned int crefs = elf_tdata (output_bfd)->cverdefs;

	  if (!_bfd_elf_add_dynamic_entry (info, DT_VERDEF, 0)
	      || !_bfd_elf_add_dynamic_entry (info, DT_VERDEFNUM, crefs))
	    return false;
	}

      if ((info->new_dtags && info->flags) || (info->flags & DF_STATIC_TLS))
	{
	  if (!_bfd_elf_add_dynamic_entry (info, DT_FLAGS, info->flags))
	    return false;
	}
      else if (info->flags & DF_BIND_NOW)
	{
	  if (!_bfd_elf_add_dynamic_entry (info, DT_BIND_NOW, 0))
	    return false;
	}

      if (info->flags_1)
	{
	  if (bfd_link_executable (info))
	    info->flags_1 &= ~ (DF_1_INITFIRST
				| DF_1_NODELETE
				| DF_1_NOOPEN);
	  if (!_bfd_elf_add_dynamic_entry (info, DT_FLAGS_1, info->flags_1))
	    return false;
	}

      if (elf_tdata (output_bfd)->cverrefs)
	{
	  unsigned int crefs = elf_tdata (output_bfd)->cverrefs;

	  if (!_bfd_elf_add_dynamic_entry (info, DT_VERNEED, 0)
	      || !_bfd_elf_add_dynamic_entry (info, DT_VERNEEDNUM, crefs))
	    return false;
	}

      if ((elf_tdata (output_bfd)->cverrefs == 0
	   && elf_tdata (output_bfd)->cverdefs == 0)
	  || _bfd_elf_link_renumber_dynsyms (output_bfd, info, NULL) <= 1)
	{
	  asection *s;

	  s = bfd_get_linker_section (dynobj, ".gnu.version");
	  s->flags |= SEC_EXCLUDE;
	}
    }
  return true;
}

/* Find the first non-excluded output section.  We'll use its
   section symbol for some emitted relocs.  */
void
_bfd_elf_init_1_index_section (bfd *output_bfd, struct bfd_link_info *info)
{
  asection *s;
  asection *found = NULL;

  for (s = output_bfd->sections; s != NULL; s = s->next)
    if ((s->flags & (SEC_EXCLUDE | SEC_ALLOC)) == SEC_ALLOC
	&& !_bfd_elf_omit_section_dynsym_default (output_bfd, info, s))
      {
	found = s;
	if ((s->flags & SEC_THREAD_LOCAL) == 0)
	  break;
      }
  elf_hash_table (info)->text_index_section = found;
}

/* Find two non-excluded output sections, one for code, one for data.
   We'll use their section symbols for some emitted relocs.  */
void
_bfd_elf_init_2_index_sections (bfd *output_bfd, struct bfd_link_info *info)
{
  asection *s;
  asection *found = NULL;

  /* Data first, since setting text_index_section changes
     _bfd_elf_omit_section_dynsym_default.  */
  for (s = output_bfd->sections; s != NULL; s = s->next)
    if ((s->flags & (SEC_EXCLUDE | SEC_ALLOC)) == SEC_ALLOC
	&& !(s->flags & SEC_READONLY)
	&& !_bfd_elf_omit_section_dynsym_default (output_bfd, info, s))
      {
	found = s;
	if ((s->flags & SEC_THREAD_LOCAL) == 0)
	  break;
      }
  elf_hash_table (info)->data_index_section = found;

  for (s = output_bfd->sections; s != NULL; s = s->next)
    if ((s->flags & (SEC_EXCLUDE | SEC_ALLOC)) == SEC_ALLOC
	&& (s->flags & SEC_READONLY)
	&& !_bfd_elf_omit_section_dynsym_default (output_bfd, info, s))
      {
	found = s;
	break;
      }
  elf_hash_table (info)->text_index_section = found;
}

#define GNU_HASH_SECTION_NAME(bed)			    \
  (bed)->record_xhash_symbol != NULL ? ".MIPS.xhash" : ".gnu.hash"

bool
bfd_elf_size_dynsym_hash_dynstr (bfd *output_bfd, struct bfd_link_info *info)
{
  const struct elf_backend_data *bed;
  unsigned long section_sym_count;
  bfd_size_type dynsymcount = 0;

  if (!is_elf_hash_table (info->hash))
    return true;

  bed = get_elf_backend_data (output_bfd);
  (*bed->elf_backend_init_index_section) (output_bfd, info);

  /* Assign dynsym indices.  In a shared library we generate a section
     symbol for each output section, which come first.  Next come all
     of the back-end allocated local dynamic syms, followed by the rest
     of the global symbols.

     This is usually not needed for static binaries, however backends
     can request to always do it, e.g. the MIPS backend uses dynamic
     symbol counts to lay out GOT, which will be produced in the
     presence of GOT relocations even in static binaries (holding fixed
     data in that case, to satisfy those relocations).  */

  if (elf_hash_table (info)->dynamic_sections_created
      || bed->always_renumber_dynsyms)
    dynsymcount = _bfd_elf_link_renumber_dynsyms (output_bfd, info,
						  &section_sym_count);

  if (elf_hash_table (info)->dynamic_sections_created)
    {
      bfd *dynobj;
      asection *s;
      unsigned int dtagcount;

      dynobj = elf_hash_table (info)->dynobj;

      /* Work out the size of the symbol version section.  */
      s = bfd_get_linker_section (dynobj, ".gnu.version");
      BFD_ASSERT (s != NULL);
      if ((s->flags & SEC_EXCLUDE) == 0)
	{
	  s->size = dynsymcount * sizeof (Elf_External_Versym);
	  s->contents = (unsigned char *) bfd_zalloc (output_bfd, s->size);
	  if (s->contents == NULL)
	    return false;

	  if (!_bfd_elf_add_dynamic_entry (info, DT_VERSYM, 0))
	    return false;
	}

      /* Set the size of the .dynsym and .hash sections.  We counted
	 the number of dynamic symbols in elf_link_add_object_symbols.
	 We will build the contents of .dynsym and .hash when we build
	 the final symbol table, because until then we do not know the
	 correct value to give the symbols.  We built the .dynstr
	 section as we went along in elf_link_add_object_symbols.  */
      s = elf_hash_table (info)->dynsym;
      BFD_ASSERT (s != NULL);
      s->size = dynsymcount * bed->s->sizeof_sym;

      s->contents = (unsigned char *) bfd_alloc (output_bfd, s->size);
      if (s->contents == NULL)
	return false;

      /* The first entry in .dynsym is a dummy symbol.  Clear all the
	 section syms, in case we don't output them all.  */
      ++section_sym_count;
      memset (s->contents, 0, section_sym_count * bed->s->sizeof_sym);

      elf_hash_table (info)->bucketcount = 0;

      /* Compute the size of the hashing table.  As a side effect this
	 computes the hash values for all the names we export.  */
      if (info->emit_hash)
	{
	  unsigned long int *hashcodes;
	  struct hash_codes_info hashinf;
	  bfd_size_type amt;
	  unsigned long int nsyms;
	  size_t bucketcount;
	  size_t hash_entry_size;

	  /* Compute the hash values for all exported symbols.  At the same
	     time store the values in an array so that we could use them for
	     optimizations.  */
	  amt = dynsymcount * sizeof (unsigned long int);
	  hashcodes = (unsigned long int *) bfd_malloc (amt);
	  if (hashcodes == NULL)
	    return false;
	  hashinf.hashcodes = hashcodes;
	  hashinf.error = false;

	  /* Put all hash values in HASHCODES.  */
	  elf_link_hash_traverse (elf_hash_table (info),
				  elf_collect_hash_codes, &hashinf);
	  if (hashinf.error)
	    {
	      free (hashcodes);
	      return false;
	    }

	  nsyms = hashinf.hashcodes - hashcodes;
	  bucketcount
	    = compute_bucket_count (info, hashcodes, nsyms, 0);
	  free (hashcodes);

	  if (bucketcount == 0 && nsyms > 0)
	    return false;

	  elf_hash_table (info)->bucketcount = bucketcount;

	  s = bfd_get_linker_section (dynobj, ".hash");
	  BFD_ASSERT (s != NULL);
	  hash_entry_size = elf_section_data (s)->this_hdr.sh_entsize;
	  s->size = ((2 + bucketcount + dynsymcount) * hash_entry_size);
	  s->contents = (unsigned char *) bfd_zalloc (output_bfd, s->size);
	  if (s->contents == NULL)
	    return false;

	  bfd_put (8 * hash_entry_size, output_bfd, bucketcount, s->contents);
	  bfd_put (8 * hash_entry_size, output_bfd, dynsymcount,
		   s->contents + hash_entry_size);
	}

      if (info->emit_gnu_hash)
	{
	  size_t i, cnt;
	  unsigned char *contents;
	  struct collect_gnu_hash_codes cinfo;
	  bfd_size_type amt;
	  size_t bucketcount;

	  memset (&cinfo, 0, sizeof (cinfo));

	  /* Compute the hash values for all exported symbols.  At the same
	     time store the values in an array so that we could use them for
	     optimizations.  */
	  amt = dynsymcount * 2 * sizeof (unsigned long int);
	  cinfo.hashcodes = (long unsigned int *) bfd_malloc (amt);
	  if (cinfo.hashcodes == NULL)
	    return false;

	  cinfo.hashval = cinfo.hashcodes + dynsymcount;
	  cinfo.min_dynindx = -1;
	  cinfo.output_bfd = output_bfd;
	  cinfo.bed = bed;

	  /* Put all hash values in HASHCODES.  */
	  elf_link_hash_traverse (elf_hash_table (info),
				  elf_collect_gnu_hash_codes, &cinfo);
	  if (cinfo.error)
	    {
	      free (cinfo.hashcodes);
	      return false;
	    }

	  bucketcount
	    = compute_bucket_count (info, cinfo.hashcodes, cinfo.nsyms, 1);

	  if (bucketcount == 0)
	    {
	      free (cinfo.hashcodes);
	      return false;
	    }

	  s = bfd_get_linker_section (dynobj, GNU_HASH_SECTION_NAME (bed));
	  BFD_ASSERT (s != NULL);

	  if (cinfo.nsyms == 0)
	    {
	      /* Empty .gnu.hash or .MIPS.xhash section is special.  */
	      BFD_ASSERT (cinfo.min_dynindx == -1);
	      free (cinfo.hashcodes);
	      s->size = 5 * 4 + bed->s->arch_size / 8;
	      contents = (unsigned char *) bfd_zalloc (output_bfd, s->size);
	      if (contents == NULL)
		return false;
	      s->contents = contents;
	      /* 1 empty bucket.  */
	      bfd_put_32 (output_bfd, 1, contents);
	      /* SYMIDX above the special symbol 0.  */
	      bfd_put_32 (output_bfd, 1, contents + 4);
	      /* Just one word for bitmask.  */
	      bfd_put_32 (output_bfd, 1, contents + 8);
	      /* Only hash fn bloom filter.  */
	      bfd_put_32 (output_bfd, 0, contents + 12);
	      /* No hashes are valid - empty bitmask.  */
	      bfd_put (bed->s->arch_size, output_bfd, 0, contents + 16);
	      /* No hashes in the only bucket.  */
	      bfd_put_32 (output_bfd, 0,
			  contents + 16 + bed->s->arch_size / 8);
	    }
	  else
	    {
	      unsigned long int maskwords, maskbitslog2, x;
	      BFD_ASSERT (cinfo.min_dynindx != -1);

	      x = cinfo.nsyms;
	      maskbitslog2 = 1;
	      while ((x >>= 1) != 0)
		++maskbitslog2;
	      if (maskbitslog2 < 3)
		maskbitslog2 = 5;
	      else if ((1 << (maskbitslog2 - 2)) & cinfo.nsyms)
		maskbitslog2 = maskbitslog2 + 3;
	      else
		maskbitslog2 = maskbitslog2 + 2;
	      if (bed->s->arch_size == 64)
		{
		  if (maskbitslog2 == 5)
		    maskbitslog2 = 6;
		  cinfo.shift1 = 6;
		}
	      else
		cinfo.shift1 = 5;
	      cinfo.mask = (1 << cinfo.shift1) - 1;
	      cinfo.shift2 = maskbitslog2;
	      cinfo.maskbits = 1 << maskbitslog2;
	      maskwords = 1 << (maskbitslog2 - cinfo.shift1);
	      amt = bucketcount * sizeof (unsigned long int) * 2;
	      amt += maskwords * sizeof (bfd_vma);
	      cinfo.bitmask = (bfd_vma *) bfd_malloc (amt);
	      if (cinfo.bitmask == NULL)
		{
		  free (cinfo.hashcodes);
		  return false;
		}

	      cinfo.counts = (long unsigned int *) (cinfo.bitmask + maskwords);
	      cinfo.indx = cinfo.counts + bucketcount;
	      cinfo.symindx = dynsymcount - cinfo.nsyms;
	      memset (cinfo.bitmask, 0, maskwords * sizeof (bfd_vma));

	      /* Determine how often each hash bucket is used.  */
	      memset (cinfo.counts, 0, bucketcount * sizeof (cinfo.counts[0]));
	      for (i = 0; i < cinfo.nsyms; ++i)
		++cinfo.counts[cinfo.hashcodes[i] % bucketcount];

	      for (i = 0, cnt = cinfo.symindx; i < bucketcount; ++i)
		if (cinfo.counts[i] != 0)
		  {
		    cinfo.indx[i] = cnt;
		    cnt += cinfo.counts[i];
		  }
	      BFD_ASSERT (cnt == dynsymcount);
	      cinfo.bucketcount = bucketcount;
	      cinfo.local_indx = cinfo.min_dynindx;

	      s->size = (4 + bucketcount + cinfo.nsyms) * 4;
	      s->size += cinfo.maskbits / 8;
	      if (bed->record_xhash_symbol != NULL)
		s->size += cinfo.nsyms * 4;
	      contents = (unsigned char *) bfd_zalloc (output_bfd, s->size);
	      if (contents == NULL)
		{
		  free (cinfo.bitmask);
		  free (cinfo.hashcodes);
		  return false;
		}

	      s->contents = contents;
	      bfd_put_32 (output_bfd, bucketcount, contents);
	      bfd_put_32 (output_bfd, cinfo.symindx, contents + 4);
	      bfd_put_32 (output_bfd, maskwords, contents + 8);
	      bfd_put_32 (output_bfd, cinfo.shift2, contents + 12);
	      contents += 16 + cinfo.maskbits / 8;

	      for (i = 0; i < bucketcount; ++i)
		{
		  if (cinfo.counts[i] == 0)
		    bfd_put_32 (output_bfd, 0, contents);
		  else
		    bfd_put_32 (output_bfd, cinfo.indx[i], contents);
		  contents += 4;
		}

	      cinfo.contents = contents;

	      cinfo.xlat = contents + cinfo.nsyms * 4 - s->contents;
	      /* Renumber dynamic symbols, if populating .gnu.hash section.
		 If using .MIPS.xhash, populate the translation table.  */
	      elf_link_hash_traverse (elf_hash_table (info),
				      elf_gnu_hash_process_symidx, &cinfo);

	      contents = s->contents + 16;
	      for (i = 0; i < maskwords; ++i)
		{
		  bfd_put (bed->s->arch_size, output_bfd, cinfo.bitmask[i],
			   contents);
		  contents += bed->s->arch_size / 8;
		}

	      free (cinfo.bitmask);
	      free (cinfo.hashcodes);
	    }
	}

      s = bfd_get_linker_section (dynobj, ".dynstr");
      BFD_ASSERT (s != NULL);

      elf_finalize_dynstr (output_bfd, info);

      s->size = _bfd_elf_strtab_size (elf_hash_table (info)->dynstr);

      for (dtagcount = 0; dtagcount <= info->spare_dynamic_tags; ++dtagcount)
	if (!_bfd_elf_add_dynamic_entry (info, DT_NULL, 0))
	  return false;
    }

  return true;
}

/* Make sure sec_info_type is cleared if sec_info is cleared too.  */

static void
merge_sections_remove_hook (bfd *abfd ATTRIBUTE_UNUSED,
			    asection *sec)
{
  BFD_ASSERT (sec->sec_info_type == SEC_INFO_TYPE_MERGE);
  sec->sec_info_type = SEC_INFO_TYPE_NONE;
}

/* Finish SHF_MERGE section merging.  */

bool
_bfd_elf_merge_sections (bfd *obfd, struct bfd_link_info *info)
{
  bfd *ibfd;
  asection *sec;

  if (!is_elf_hash_table (info->hash))
    return false;

  for (ibfd = info->input_bfds; ibfd != NULL; ibfd = ibfd->link.next)
    if ((ibfd->flags & DYNAMIC) == 0
	&& bfd_get_flavour (ibfd) == bfd_target_elf_flavour
	&& (elf_elfheader (ibfd)->e_ident[EI_CLASS]
	    == get_elf_backend_data (obfd)->s->elfclass))
      for (sec = ibfd->sections; sec != NULL; sec = sec->next)
	if ((sec->flags & SEC_MERGE) != 0
	    && !bfd_is_abs_section (sec->output_section))
	  {
	    struct bfd_elf_section_data *secdata;

	    secdata = elf_section_data (sec);
	    if (! _bfd_add_merge_section (obfd,
					  &elf_hash_table (info)->merge_info,
					  sec, &secdata->sec_info))
	      return false;
	    else if (secdata->sec_info)
	      sec->sec_info_type = SEC_INFO_TYPE_MERGE;
	  }

  if (elf_hash_table (info)->merge_info != NULL)
    _bfd_merge_sections (obfd, info, elf_hash_table (info)->merge_info,
			 merge_sections_remove_hook);
  return true;
}

/* Create an entry in an ELF linker hash table.  */

struct bfd_hash_entry *
_bfd_elf_link_hash_newfunc (struct bfd_hash_entry *entry,
			    struct bfd_hash_table *table,
			    const char *string)
{
  /* Allocate the structure if it has not already been allocated by a
     subclass.  */
  if (entry == NULL)
    {
      entry = (struct bfd_hash_entry *)
	bfd_hash_allocate (table, sizeof (struct elf_link_hash_entry));
      if (entry == NULL)
	return entry;
    }

  /* Call the allocation method of the superclass.  */
  entry = _bfd_link_hash_newfunc (entry, table, string);
  if (entry != NULL)
    {
      struct elf_link_hash_entry *ret = (struct elf_link_hash_entry *) entry;
      struct elf_link_hash_table *htab = (struct elf_link_hash_table *) table;

      /* Set local fields.  */
      ret->indx = -1;
      ret->dynindx = -1;
      ret->got = htab->init_got_refcount;
      ret->plt = htab->init_plt_refcount;
      memset (&ret->size, 0, (sizeof (struct elf_link_hash_entry)
			      - offsetof (struct elf_link_hash_entry, size)));
      /* Assume that we have been called by a non-ELF symbol reader.
	 This flag is then reset by the code which reads an ELF input
	 file.  This ensures that a symbol created by a non-ELF symbol
	 reader will have the flag set correctly.  */
      ret->non_elf = 1;
    }

  return entry;
}

/* Copy data from an indirect symbol to its direct symbol, hiding the
   old indirect symbol.  Also used for copying flags to a weakdef.  */

void
_bfd_elf_link_hash_copy_indirect (struct bfd_link_info *info,
				  struct elf_link_hash_entry *dir,
				  struct elf_link_hash_entry *ind)
{
  struct elf_link_hash_table *htab;

  if (ind->dyn_relocs != NULL)
    {
      if (dir->dyn_relocs != NULL)
	{
	  struct elf_dyn_relocs **pp;
	  struct elf_dyn_relocs *p;

	  /* Add reloc counts against the indirect sym to the direct sym
	     list.  Merge any entries against the same section.  */
	  for (pp = &ind->dyn_relocs; (p = *pp) != NULL; )
	    {
	      struct elf_dyn_relocs *q;

	      for (q = dir->dyn_relocs; q != NULL; q = q->next)
		if (q->sec == p->sec)
		  {
		    q->pc_count += p->pc_count;
		    q->count += p->count;
		    *pp = p->next;
		    break;
		  }
	      if (q == NULL)
		pp = &p->next;
	    }
	  *pp = dir->dyn_relocs;
	}

      dir->dyn_relocs = ind->dyn_relocs;
      ind->dyn_relocs = NULL;
    }

  /* Copy down any references that we may have already seen to the
     symbol which just became indirect.  */

  if (dir->versioned != versioned_hidden)
    dir->ref_dynamic |= ind->ref_dynamic;
  dir->ref_regular |= ind->ref_regular;
  dir->ref_regular_nonweak |= ind->ref_regular_nonweak;
  dir->non_got_ref |= ind->non_got_ref;
  dir->needs_plt |= ind->needs_plt;
  dir->pointer_equality_needed |= ind->pointer_equality_needed;

  if (ind->root.type != bfd_link_hash_indirect)
    return;

  /* Copy over the global and procedure linkage table refcount entries.
     These may have been already set up by a check_relocs routine.  */
  htab = elf_hash_table (info);
  if (ind->got.refcount > htab->init_got_refcount.refcount)
    {
      if (dir->got.refcount < 0)
	dir->got.refcount = 0;
      dir->got.refcount += ind->got.refcount;
      ind->got.refcount = htab->init_got_refcount.refcount;
    }

  if (ind->plt.refcount > htab->init_plt_refcount.refcount)
    {
      if (dir->plt.refcount < 0)
	dir->plt.refcount = 0;
      dir->plt.refcount += ind->plt.refcount;
      ind->plt.refcount = htab->init_plt_refcount.refcount;
    }

  if (ind->dynindx != -1)
    {
      if (dir->dynindx != -1)
	_bfd_elf_strtab_delref (htab->dynstr, dir->dynstr_index);
      dir->dynindx = ind->dynindx;
      dir->dynstr_index = ind->dynstr_index;
      ind->dynindx = -1;
      ind->dynstr_index = 0;
    }
}

void
_bfd_elf_link_hash_hide_symbol (struct bfd_link_info *info,
				struct elf_link_hash_entry *h,
				bool force_local)
{
  /* STT_GNU_IFUNC symbol must go through PLT.  */
  if (h->type != STT_GNU_IFUNC)
    {
      h->plt = elf_hash_table (info)->init_plt_offset;
      h->needs_plt = 0;
    }
  if (force_local)
    {
      h->forced_local = 1;
      if (h->dynindx != -1)
	{
	  _bfd_elf_strtab_delref (elf_hash_table (info)->dynstr,
				  h->dynstr_index);
	  h->dynindx = -1;
	  h->dynstr_index = 0;
	}
    }
}

/* Hide a symbol. */

void
_bfd_elf_link_hide_symbol (bfd *output_bfd,
			   struct bfd_link_info *info,
			   struct bfd_link_hash_entry *h)
{
  if (is_elf_hash_table (info->hash))
    {
      const struct elf_backend_data *bed
	= get_elf_backend_data (output_bfd);
      struct elf_link_hash_entry *eh
	= (struct elf_link_hash_entry *) h;
      bed->elf_backend_hide_symbol (info, eh, true);
      eh->def_dynamic = 0;
      eh->ref_dynamic = 0;
      eh->dynamic_def = 0;
    }
}

/* Initialize an ELF linker hash table.  *TABLE has been zeroed by our
   caller.  */

bool
_bfd_elf_link_hash_table_init
  (struct elf_link_hash_table *table,
   bfd *abfd,
   struct bfd_hash_entry *(*newfunc) (struct bfd_hash_entry *,
				      struct bfd_hash_table *,
				      const char *),
   unsigned int entsize,
   enum elf_target_id target_id)
{
  bool ret;
  int can_refcount = get_elf_backend_data (abfd)->can_refcount;

  table->init_got_refcount.refcount = can_refcount - 1;
  table->init_plt_refcount.refcount = can_refcount - 1;
  table->init_got_offset.offset = -(bfd_vma) 1;
  table->init_plt_offset.offset = -(bfd_vma) 1;
  /* The first dynamic symbol is a dummy.  */
  table->dynsymcount = 1;

  ret = _bfd_link_hash_table_init (&table->root, abfd, newfunc, entsize);

  table->root.type = bfd_link_elf_hash_table;
  table->hash_table_id = target_id;
  table->target_os = get_elf_backend_data (abfd)->target_os;

  return ret;
}

/* Create an ELF linker hash table.  */

struct bfd_link_hash_table *
_bfd_elf_link_hash_table_create (bfd *abfd)
{
  struct elf_link_hash_table *ret;
  size_t amt = sizeof (struct elf_link_hash_table);

  ret = (struct elf_link_hash_table *) bfd_zmalloc (amt);
  if (ret == NULL)
    return NULL;

  if (! _bfd_elf_link_hash_table_init (ret, abfd, _bfd_elf_link_hash_newfunc,
				       sizeof (struct elf_link_hash_entry),
				       GENERIC_ELF_DATA))
    {
      free (ret);
      return NULL;
    }
  ret->root.hash_table_free = _bfd_elf_link_hash_table_free;

  return &ret->root;
}

/* Destroy an ELF linker hash table.  */

void
_bfd_elf_link_hash_table_free (bfd *obfd)
{
  struct elf_link_hash_table *htab;

  htab = (struct elf_link_hash_table *) obfd->link.hash;
  if (htab->dynstr != NULL)
    _bfd_elf_strtab_free (htab->dynstr);
  _bfd_merge_sections_free (htab->merge_info);
  _bfd_generic_link_hash_table_free (obfd);
}

/* This is a hook for the ELF emulation code in the generic linker to
   tell the backend linker what file name to use for the DT_NEEDED
   entry for a dynamic object.  */

void
bfd_elf_set_dt_needed_name (bfd *abfd, const char *name)
{
  if (bfd_get_flavour (abfd) == bfd_target_elf_flavour
      && bfd_get_format (abfd) == bfd_object)
    elf_dt_name (abfd) = name;
}

int
bfd_elf_get_dyn_lib_class (bfd *abfd)
{
  int lib_class;
  if (bfd_get_flavour (abfd) == bfd_target_elf_flavour
      && bfd_get_format (abfd) == bfd_object)
    lib_class = elf_dyn_lib_class (abfd);
  else
    lib_class = 0;
  return lib_class;
}

void
bfd_elf_set_dyn_lib_class (bfd *abfd, enum dynamic_lib_link_class lib_class)
{
  if (bfd_get_flavour (abfd) == bfd_target_elf_flavour
      && bfd_get_format (abfd) == bfd_object)
    elf_dyn_lib_class (abfd) = lib_class;
}

/* Get the list of DT_NEEDED entries for a link.  This is a hook for
   the linker ELF emulation code.  */

struct bfd_link_needed_list *
bfd_elf_get_needed_list (bfd *abfd ATTRIBUTE_UNUSED,
			 struct bfd_link_info *info)
{
  if (! is_elf_hash_table (info->hash))
    return NULL;
  return elf_hash_table (info)->needed;
}

/* Get the list of DT_RPATH/DT_RUNPATH entries for a link.  This is a
   hook for the linker ELF emulation code.  */

struct bfd_link_needed_list *
bfd_elf_get_runpath_list (bfd *abfd ATTRIBUTE_UNUSED,
			  struct bfd_link_info *info)
{
  if (! is_elf_hash_table (info->hash))
    return NULL;
  return elf_hash_table (info)->runpath;
}

/* Get the name actually used for a dynamic object for a link.  This
   is the SONAME entry if there is one.  Otherwise, it is the string
   passed to bfd_elf_set_dt_needed_name, or it is the filename.  */

const char *
bfd_elf_get_dt_soname (bfd *abfd)
{
  if (bfd_get_flavour (abfd) == bfd_target_elf_flavour
      && bfd_get_format (abfd) == bfd_object)
    return elf_dt_name (abfd);
  return NULL;
}

/* Get the list of DT_NEEDED entries from a BFD.  This is a hook for
   the ELF linker emulation code.  */

bool
bfd_elf_get_bfd_needed_list (bfd *abfd,
			     struct bfd_link_needed_list **pneeded)
{
  asection *s;
  bfd_byte *dynbuf = NULL;
  unsigned int elfsec;
  unsigned long shlink;
  bfd_byte *extdyn, *extdynend;
  size_t extdynsize;
  void (*swap_dyn_in) (bfd *, const void *, Elf_Internal_Dyn *);

  *pneeded = NULL;

  if (bfd_get_flavour (abfd) != bfd_target_elf_flavour
      || bfd_get_format (abfd) != bfd_object)
    return true;

  s = bfd_get_section_by_name (abfd, ".dynamic");
  if (s == NULL || s->size == 0 || (s->flags & SEC_HAS_CONTENTS) == 0)
    return true;

  if (!bfd_malloc_and_get_section (abfd, s, &dynbuf))
    goto error_return;

  elfsec = _bfd_elf_section_from_bfd_section (abfd, s);
  if (elfsec == SHN_BAD)
    goto error_return;

  shlink = elf_elfsections (abfd)[elfsec]->sh_link;

  extdynsize = get_elf_backend_data (abfd)->s->sizeof_dyn;
  swap_dyn_in = get_elf_backend_data (abfd)->s->swap_dyn_in;

  for (extdyn = dynbuf, extdynend = dynbuf + s->size;
       (size_t) (extdynend - extdyn) >= extdynsize;
       extdyn += extdynsize)
    {
      Elf_Internal_Dyn dyn;

      (*swap_dyn_in) (abfd, extdyn, &dyn);

      if (dyn.d_tag == DT_NULL)
	break;

      if (dyn.d_tag == DT_NEEDED)
	{
	  const char *string;
	  struct bfd_link_needed_list *l;
	  unsigned int tagv = dyn.d_un.d_val;
	  size_t amt;

	  string = bfd_elf_string_from_elf_section (abfd, shlink, tagv);
	  if (string == NULL)
	    goto error_return;

	  amt = sizeof *l;
	  l = (struct bfd_link_needed_list *) bfd_alloc (abfd, amt);
	  if (l == NULL)
	    goto error_return;

	  l->by = abfd;
	  l->name = string;
	  l->next = *pneeded;
	  *pneeded = l;
	}
    }

  free (dynbuf);

  return true;

 error_return:
  free (dynbuf);
  return false;
}

struct elf_symbuf_symbol
{
  unsigned long st_name;	/* Symbol name, index in string tbl */
  unsigned char st_info;	/* Type and binding attributes */
  unsigned char st_other;	/* Visibilty, and target specific */
};

struct elf_symbuf_head
{
  struct elf_symbuf_symbol *ssym;
  size_t count;
  unsigned int st_shndx;
};

struct elf_symbol
{
  union
    {
      Elf_Internal_Sym *isym;
      struct elf_symbuf_symbol *ssym;
      void *p;
    } u;
  const char *name;
};

/* Sort references to symbols by ascending section number.  */

static int
elf_sort_elf_symbol (const void *arg1, const void *arg2)
{
  const Elf_Internal_Sym *s1 = *(const Elf_Internal_Sym **) arg1;
  const Elf_Internal_Sym *s2 = *(const Elf_Internal_Sym **) arg2;

  if (s1->st_shndx != s2->st_shndx)
    return s1->st_shndx > s2->st_shndx ? 1 : -1;
  /* Final sort by the address of the sym in the symbuf ensures
     a stable sort.  */
  if (s1 != s2)
    return s1 > s2 ? 1 : -1;
  return 0;
}

static int
elf_sym_name_compare (const void *arg1, const void *arg2)
{
  const struct elf_symbol *s1 = (const struct elf_symbol *) arg1;
  const struct elf_symbol *s2 = (const struct elf_symbol *) arg2;
  int ret = strcmp (s1->name, s2->name);
  if (ret != 0)
    return ret;
  if (s1->u.p != s2->u.p)
    return s1->u.p > s2->u.p ? 1 : -1;
  return 0;
}

static struct elf_symbuf_head *
elf_create_symbuf (size_t symcount, Elf_Internal_Sym *isymbuf)
{
  Elf_Internal_Sym **ind, **indbufend, **indbuf;
  struct elf_symbuf_symbol *ssym;
  struct elf_symbuf_head *ssymbuf, *ssymhead;
  size_t i, shndx_count, total_size, amt;

  amt = symcount * sizeof (*indbuf);
  indbuf = (Elf_Internal_Sym **) bfd_malloc (amt);
  if (indbuf == NULL)
    return NULL;

  for (ind = indbuf, i = 0; i < symcount; i++)
    if (isymbuf[i].st_shndx != SHN_UNDEF)
      *ind++ = &isymbuf[i];
  indbufend = ind;

  qsort (indbuf, indbufend - indbuf, sizeof (Elf_Internal_Sym *),
	 elf_sort_elf_symbol);

  shndx_count = 0;
  if (indbufend > indbuf)
    for (ind = indbuf, shndx_count++; ind < indbufend - 1; ind++)
      if (ind[0]->st_shndx != ind[1]->st_shndx)
	shndx_count++;

  total_size = ((shndx_count + 1) * sizeof (*ssymbuf)
		+ (indbufend - indbuf) * sizeof (*ssym));
  ssymbuf = (struct elf_symbuf_head *) bfd_malloc (total_size);
  if (ssymbuf == NULL)
    {
      free (indbuf);
      return NULL;
    }

  ssym = (struct elf_symbuf_symbol *) (ssymbuf + shndx_count + 1);
  ssymbuf->ssym = NULL;
  ssymbuf->count = shndx_count;
  ssymbuf->st_shndx = 0;
  for (ssymhead = ssymbuf, ind = indbuf; ind < indbufend; ssym++, ind++)
    {
      if (ind == indbuf || ssymhead->st_shndx != (*ind)->st_shndx)
	{
	  ssymhead++;
	  ssymhead->ssym = ssym;
	  ssymhead->count = 0;
	  ssymhead->st_shndx = (*ind)->st_shndx;
	}
      ssym->st_name = (*ind)->st_name;
      ssym->st_info = (*ind)->st_info;
      ssym->st_other = (*ind)->st_other;
      ssymhead->count++;
    }
  BFD_ASSERT ((size_t) (ssymhead - ssymbuf) == shndx_count
	      && (uintptr_t) ssym - (uintptr_t) ssymbuf == total_size);

  free (indbuf);
  return ssymbuf;
}

/* Check if 2 sections define the same set of local and global
   symbols.  */

static bool
bfd_elf_match_symbols_in_sections (asection *sec1, asection *sec2,
				   struct bfd_link_info *info)
{
  bfd *bfd1, *bfd2;
  const struct elf_backend_data *bed1, *bed2;
  Elf_Internal_Shdr *hdr1, *hdr2;
  size_t symcount1, symcount2;
  Elf_Internal_Sym *isymbuf1, *isymbuf2;
  struct elf_symbuf_head *ssymbuf1, *ssymbuf2;
  Elf_Internal_Sym *isym, *isymend;
  struct elf_symbol *symtable1 = NULL, *symtable2 = NULL;
  size_t count1, count2, sec_count1, sec_count2, i;
  unsigned int shndx1, shndx2;
  bool result;
  bool ignore_section_symbol_p;

  bfd1 = sec1->owner;
  bfd2 = sec2->owner;

  /* Both sections have to be in ELF.  */
  if (bfd_get_flavour (bfd1) != bfd_target_elf_flavour
      || bfd_get_flavour (bfd2) != bfd_target_elf_flavour)
    return false;

  if (elf_section_type (sec1) != elf_section_type (sec2))
    return false;

  shndx1 = _bfd_elf_section_from_bfd_section (bfd1, sec1);
  shndx2 = _bfd_elf_section_from_bfd_section (bfd2, sec2);
  if (shndx1 == SHN_BAD || shndx2 == SHN_BAD)
    return false;

  bed1 = get_elf_backend_data (bfd1);
  bed2 = get_elf_backend_data (bfd2);
  hdr1 = &elf_tdata (bfd1)->symtab_hdr;
  symcount1 = hdr1->sh_size / bed1->s->sizeof_sym;
  hdr2 = &elf_tdata (bfd2)->symtab_hdr;
  symcount2 = hdr2->sh_size / bed2->s->sizeof_sym;

  if (symcount1 == 0 || symcount2 == 0)
    return false;

  result = false;
  isymbuf1 = NULL;
  isymbuf2 = NULL;
  ssymbuf1 = (struct elf_symbuf_head *) elf_tdata (bfd1)->symbuf;
  ssymbuf2 = (struct elf_symbuf_head *) elf_tdata (bfd2)->symbuf;

  /* Ignore section symbols only when matching non-debugging sections
     or linkonce section with comdat section.  */
  ignore_section_symbol_p
    = ((sec1->flags & SEC_DEBUGGING) == 0
       || ((elf_section_flags (sec1) & SHF_GROUP)
	   != (elf_section_flags (sec2) & SHF_GROUP)));

  if (ssymbuf1 == NULL)
    {
      isymbuf1 = bfd_elf_get_elf_syms (bfd1, hdr1, symcount1, 0,
				       NULL, NULL, NULL);
      if (isymbuf1 == NULL)
	goto done;

      if (info != NULL && !info->reduce_memory_overheads)
	{
	  ssymbuf1 = elf_create_symbuf (symcount1, isymbuf1);
	  elf_tdata (bfd1)->symbuf = ssymbuf1;
	}
    }

  if (ssymbuf1 == NULL || ssymbuf2 == NULL)
    {
      isymbuf2 = bfd_elf_get_elf_syms (bfd2, hdr2, symcount2, 0,
				       NULL, NULL, NULL);
      if (isymbuf2 == NULL)
	goto done;

      if (ssymbuf1 != NULL && info != NULL && !info->reduce_memory_overheads)
	{
	  ssymbuf2 = elf_create_symbuf (symcount2, isymbuf2);
	  elf_tdata (bfd2)->symbuf = ssymbuf2;
	}
    }

  if (ssymbuf1 != NULL && ssymbuf2 != NULL)
    {
      /* Optimized faster version.  */
      size_t lo, hi, mid;
      struct elf_symbol *symp;
      struct elf_symbuf_symbol *ssym, *ssymend;

      lo = 0;
      hi = ssymbuf1->count;
      ssymbuf1++;
      count1 = 0;
      sec_count1 = 0;
      while (lo < hi)
	{
	  mid = (lo + hi) / 2;
	  if (shndx1 < ssymbuf1[mid].st_shndx)
	    hi = mid;
	  else if (shndx1 > ssymbuf1[mid].st_shndx)
	    lo = mid + 1;
	  else
	    {
	      count1 = ssymbuf1[mid].count;
	      ssymbuf1 += mid;
	      break;
	    }
	}
      if (ignore_section_symbol_p)
	{
	  for (i = 0; i < count1; i++)
	    if (ELF_ST_TYPE (ssymbuf1->ssym[i].st_info) == STT_SECTION)
	      sec_count1++;
	  count1 -= sec_count1;
	}

      lo = 0;
      hi = ssymbuf2->count;
      ssymbuf2++;
      count2 = 0;
      sec_count2 = 0;
      while (lo < hi)
	{
	  mid = (lo + hi) / 2;
	  if (shndx2 < ssymbuf2[mid].st_shndx)
	    hi = mid;
	  else if (shndx2 > ssymbuf2[mid].st_shndx)
	    lo = mid + 1;
	  else
	    {
	      count2 = ssymbuf2[mid].count;
	      ssymbuf2 += mid;
	      break;
	    }
	}
      if (ignore_section_symbol_p)
	{
	  for (i = 0; i < count2; i++)
	    if (ELF_ST_TYPE (ssymbuf2->ssym[i].st_info) == STT_SECTION)
	      sec_count2++;
	  count2 -= sec_count2;
	}

      if (count1 == 0 || count2 == 0 || count1 != count2)
	goto done;

      symtable1
	= (struct elf_symbol *) bfd_malloc (count1 * sizeof (*symtable1));
      symtable2
	= (struct elf_symbol *) bfd_malloc (count2 * sizeof (*symtable2));
      if (symtable1 == NULL || symtable2 == NULL)
	goto done;

      symp = symtable1;
      for (ssym = ssymbuf1->ssym, ssymend = ssym + count1 + sec_count1;
	   ssym < ssymend; ssym++)
	if (sec_count1 == 0
	    || ELF_ST_TYPE (ssym->st_info) != STT_SECTION)
	  {
	    symp->u.ssym = ssym;
	    symp->name = bfd_elf_string_from_elf_section (bfd1,
							  hdr1->sh_link,
							  ssym->st_name);
	    symp++;
	  }

      symp = symtable2;
      for (ssym = ssymbuf2->ssym, ssymend = ssym + count2 + sec_count2;
	   ssym < ssymend; ssym++)
	if (sec_count2 == 0
	    || ELF_ST_TYPE (ssym->st_info) != STT_SECTION)
	  {
	    symp->u.ssym = ssym;
	    symp->name = bfd_elf_string_from_elf_section (bfd2,
							  hdr2->sh_link,
							  ssym->st_name);
	    symp++;
	  }

      /* Sort symbol by name.  */
      qsort (symtable1, count1, sizeof (struct elf_symbol),
	     elf_sym_name_compare);
      qsort (symtable2, count1, sizeof (struct elf_symbol),
	     elf_sym_name_compare);

      for (i = 0; i < count1; i++)
	/* Two symbols must have the same binding, type and name.  */
	if (symtable1 [i].u.ssym->st_info != symtable2 [i].u.ssym->st_info
	    || symtable1 [i].u.ssym->st_other != symtable2 [i].u.ssym->st_other
	    || strcmp (symtable1 [i].name, symtable2 [i].name) != 0)
	  goto done;

      result = true;
      goto done;
    }

  symtable1 = (struct elf_symbol *)
      bfd_malloc (symcount1 * sizeof (struct elf_symbol));
  symtable2 = (struct elf_symbol *)
      bfd_malloc (symcount2 * sizeof (struct elf_symbol));
  if (symtable1 == NULL || symtable2 == NULL)
    goto done;

  /* Count definitions in the section.  */
  count1 = 0;
  for (isym = isymbuf1, isymend = isym + symcount1; isym < isymend; isym++)
    if (isym->st_shndx == shndx1
	&& (!ignore_section_symbol_p
	    || ELF_ST_TYPE (isym->st_info) != STT_SECTION))
      symtable1[count1++].u.isym = isym;

  count2 = 0;
  for (isym = isymbuf2, isymend = isym + symcount2; isym < isymend; isym++)
    if (isym->st_shndx == shndx2
	&& (!ignore_section_symbol_p
	    || ELF_ST_TYPE (isym->st_info) != STT_SECTION))
      symtable2[count2++].u.isym = isym;

  if (count1 == 0 || count2 == 0 || count1 != count2)
    goto done;

  for (i = 0; i < count1; i++)
    symtable1[i].name
      = bfd_elf_string_from_elf_section (bfd1, hdr1->sh_link,
					 symtable1[i].u.isym->st_name);

  for (i = 0; i < count2; i++)
    symtable2[i].name
      = bfd_elf_string_from_elf_section (bfd2, hdr2->sh_link,
					 symtable2[i].u.isym->st_name);

  /* Sort symbol by name.  */
  qsort (symtable1, count1, sizeof (struct elf_symbol),
	 elf_sym_name_compare);
  qsort (symtable2, count1, sizeof (struct elf_symbol),
	 elf_sym_name_compare);

  for (i = 0; i < count1; i++)
    /* Two symbols must have the same binding, type and name.  */
    if (symtable1 [i].u.isym->st_info != symtable2 [i].u.isym->st_info
	|| symtable1 [i].u.isym->st_other != symtable2 [i].u.isym->st_other
	|| strcmp (symtable1 [i].name, symtable2 [i].name) != 0)
      goto done;

  result = true;

 done:
  free (symtable1);
  free (symtable2);
  free (isymbuf1);
  free (isymbuf2);

  return result;
}

/* Return TRUE if 2 section types are compatible.  */

bool
_bfd_elf_match_sections_by_type (bfd *abfd, const asection *asec,
				 bfd *bbfd, const asection *bsec)
{
  if (asec == NULL
      || bsec == NULL
      || abfd->xvec->flavour != bfd_target_elf_flavour
      || bbfd->xvec->flavour != bfd_target_elf_flavour)
    return true;

  return elf_section_type (asec) == elf_section_type (bsec);
}

/* Final phase of ELF linker.  */

/* A structure we use to avoid passing large numbers of arguments.  */

struct elf_final_link_info
{
  /* General link information.  */
  struct bfd_link_info *info;
  /* Output BFD.  */
  bfd *output_bfd;
  /* Symbol string table.  */
  struct elf_strtab_hash *symstrtab;
  /* .hash section.  */
  asection *hash_sec;
  /* symbol version section (.gnu.version).  */
  asection *symver_sec;
  /* Buffer large enough to hold contents of any section.  */
  bfd_byte *contents;
  /* Buffer large enough to hold external relocs of any section.  */
  void *external_relocs;
  /* Buffer large enough to hold internal relocs of any section.  */
  Elf_Internal_Rela *internal_relocs;
  /* Buffer large enough to hold external local symbols of any input
     BFD.  */
  bfd_byte *external_syms;
  /* And a buffer for symbol section indices.  */
  Elf_External_Sym_Shndx *locsym_shndx;
  /* Buffer large enough to hold internal local symbols of any input
     BFD.  */
  Elf_Internal_Sym *internal_syms;
  /* Array large enough to hold a symbol index for each local symbol
     of any input BFD.  */
  long *indices;
  /* Array large enough to hold a section pointer for each local
     symbol of any input BFD.  */
  asection **sections;
  /* Buffer for SHT_SYMTAB_SHNDX section.  */
  Elf_External_Sym_Shndx *symshndxbuf;
  /* Number of STT_FILE syms seen.  */
  size_t filesym_count;
  /* Local symbol hash table.  */
  struct bfd_hash_table local_hash_table;
};

struct local_hash_entry
{
  /* Base hash table entry structure.  */
  struct bfd_hash_entry root;
  /* Size of the local symbol name.  */
  size_t size;
  /* Number of the duplicated local symbol names.  */
  long count;
};

/* Create an entry in the local symbol hash table.  */

static struct bfd_hash_entry *
local_hash_newfunc (struct bfd_hash_entry *entry,
		    struct bfd_hash_table *table,
		    const char *string)
{

  /* Allocate the structure if it has not already been allocated by a
     subclass.  */
  if (entry == NULL)
    {
      entry = bfd_hash_allocate (table,
				 sizeof (struct local_hash_entry));
      if (entry == NULL)
        return entry;
    }

  /* Call the allocation method of the superclass.  */
  entry = bfd_hash_newfunc (entry, table, string);
  if (entry != NULL)
    {
      ((struct local_hash_entry *) entry)->count = 0;
      ((struct local_hash_entry *) entry)->size = 0;
    }

  return entry;
}

/* This struct is used to pass information to elf_link_output_extsym.  */

struct elf_outext_info
{
  bool failed;
  bool localsyms;
  bool file_sym_done;
  struct elf_final_link_info *flinfo;
};


/* Support for evaluating a complex relocation.

   Complex relocations are generalized, self-describing relocations.  The
   implementation of them consists of two parts: complex symbols, and the
   relocations themselves.

   The relocations use a reserved elf-wide relocation type code (R_RELC
   external / BFD_RELOC_RELC internal) and an encoding of relocation field
   information (start bit, end bit, word width, etc) into the addend.  This
   information is extracted from CGEN-generated operand tables within gas.

   Complex symbols are mangled symbols (STT_RELC external / BSF_RELC
   internal) representing prefix-notation expressions, including but not
   limited to those sorts of expressions normally encoded as addends in the
   addend field.  The symbol mangling format is:

   <node> := <literal>
	  |  <unary-operator> ':' <node>
	  |  <binary-operator> ':' <node> ':' <node>
	  ;

   <literal> := 's' <digits=N> ':' <N character symbol name>
	     |  'S' <digits=N> ':' <N character section name>
	     |  '#' <hexdigits>
	     ;

   <binary-operator> := as in C
   <unary-operator> := as in C, plus "0-" for unambiguous negation.  */

static void
set_symbol_value (bfd *bfd_with_globals,
		  Elf_Internal_Sym *isymbuf,
		  size_t locsymcount,
		  size_t symidx,
		  bfd_vma val)
{
  struct elf_link_hash_entry **sym_hashes;
  struct elf_link_hash_entry *h;
  size_t extsymoff = locsymcount;

  if (symidx < locsymcount)
    {
      Elf_Internal_Sym *sym;

      sym = isymbuf + symidx;
      if (ELF_ST_BIND (sym->st_info) == STB_LOCAL)
	{
	  /* It is a local symbol: move it to the
	     "absolute" section and give it a value.  */
	  sym->st_shndx = SHN_ABS;
	  sym->st_value = val;
	  return;
	}
      BFD_ASSERT (elf_bad_symtab (bfd_with_globals));
      extsymoff = 0;
    }

  /* It is a global symbol: set its link type
     to "defined" and give it a value.  */

  sym_hashes = elf_sym_hashes (bfd_with_globals);
  h = sym_hashes [symidx - extsymoff];
  while (h->root.type == bfd_link_hash_indirect
	 || h->root.type == bfd_link_hash_warning)
    h = (struct elf_link_hash_entry *) h->root.u.i.link;
  h->root.type = bfd_link_hash_defined;
  h->root.u.def.value = val;
  h->root.u.def.section = bfd_abs_section_ptr;
}

static bool
resolve_symbol (const char *name,
		bfd *input_bfd,
		struct elf_final_link_info *flinfo,
		bfd_vma *result,
		Elf_Internal_Sym *isymbuf,
		size_t locsymcount)
{
  Elf_Internal_Sym *sym;
  struct bfd_link_hash_entry *global_entry;
  const char *candidate = NULL;
  Elf_Internal_Shdr *symtab_hdr;
  size_t i;

  symtab_hdr = & elf_tdata (input_bfd)->symtab_hdr;

  for (i = 0; i < locsymcount; ++ i)
    {
      sym = isymbuf + i;

      if (ELF_ST_BIND (sym->st_info) != STB_LOCAL)
	continue;

      candidate = bfd_elf_string_from_elf_section (input_bfd,
						   symtab_hdr->sh_link,
						   sym->st_name);
#ifdef DEBUG
      printf ("Comparing string: '%s' vs. '%s' = 0x%lx\n",
	      name, candidate, (unsigned long) sym->st_value);
#endif
      if (candidate && strcmp (candidate, name) == 0)
	{
	  asection *sec = flinfo->sections [i];

	  *result = _bfd_elf_rel_local_sym (input_bfd, sym, &sec, 0);
	  *result += sec->output_offset + sec->output_section->vma;
#ifdef DEBUG
	  printf ("Found symbol with value %8.8lx\n",
		  (unsigned long) *result);
#endif
	  return true;
	}
    }

  /* Hmm, haven't found it yet. perhaps it is a global.  */
  global_entry = bfd_link_hash_lookup (flinfo->info->hash, name,
				       false, false, true);
  if (!global_entry)
    return false;

  if (global_entry->type == bfd_link_hash_defined
      || global_entry->type == bfd_link_hash_defweak)
    {
      *result = (global_entry->u.def.value
		 + global_entry->u.def.section->output_section->vma
		 + global_entry->u.def.section->output_offset);
#ifdef DEBUG
      printf ("Found GLOBAL symbol '%s' with value %8.8lx\n",
	      global_entry->root.string, (unsigned long) *result);
#endif
      return true;
    }

  return false;
}

/* Looks up NAME in SECTIONS.  If found sets RESULT to NAME's address (in
   bytes) and returns TRUE, otherwise returns FALSE.  Accepts pseudo-section
   names like "foo.end" which is the end address of section "foo".  */

static bool
resolve_section (const char *name,
		 asection *sections,
		 bfd_vma *result,
		 bfd * abfd)
{
  asection *curr;
  unsigned int len;

  for (curr = sections; curr; curr = curr->next)
    if (strcmp (curr->name, name) == 0)
      {
	*result = curr->vma;
	return true;
      }

  /* Hmm. still haven't found it. try pseudo-section names.  */
  /* FIXME: This could be coded more efficiently...  */
  for (curr = sections; curr; curr = curr->next)
    {
      len = strlen (curr->name);
      if (len > strlen (name))
	continue;

      if (strncmp (curr->name, name, len) == 0)
	{
	  if (startswith (name + len, ".end"))
	    {
	      *result = (curr->vma
			 + curr->size / bfd_octets_per_byte (abfd, curr));
	      return true;
	    }

	  /* Insert more pseudo-section names here, if you like.  */
	}
    }

  return false;
}

static void
undefined_reference (const char *reftype, const char *name)
{
  /* xgettext:c-format */
  _bfd_error_handler (_("undefined %s reference in complex symbol: %s"),
		      reftype, name);
  bfd_set_error (bfd_error_bad_value);
}

static bool
eval_symbol (bfd_vma *result,
	     const char **symp,
	     bfd *input_bfd,
	     struct elf_final_link_info *flinfo,
	     bfd_vma dot,
	     Elf_Internal_Sym *isymbuf,
	     size_t locsymcount,
	     int signed_p)
{
  size_t len;
  size_t symlen;
  bfd_vma a;
  bfd_vma b;
  char symbuf[4096];
  const char *sym = *symp;
  const char *symend;
  bool symbol_is_section = false;

  len = strlen (sym);
  symend = sym + len;

  if (len < 1 || len > sizeof (symbuf))
    {
      bfd_set_error (bfd_error_invalid_operation);
      return false;
    }

  switch (* sym)
    {
    case '.':
      *result = dot;
      *symp = sym + 1;
      return true;

    case '#':
      ++sym;
      *result = strtoul (sym, (char **) symp, 16);
      return true;

    case 'S':
      symbol_is_section = true;
      /* Fall through.  */
    case 's':
      ++sym;
      symlen = strtol (sym, (char **) symp, 10);
      sym = *symp + 1; /* Skip the trailing ':'.  */

      if (symend < sym || symlen + 1 > sizeof (symbuf))
	{
	  bfd_set_error (bfd_error_invalid_operation);
	  return false;
	}

      memcpy (symbuf, sym, symlen);
      symbuf[symlen] = '\0';
      *symp = sym + symlen;

      /* Is it always possible, with complex symbols, that gas "mis-guessed"
	 the symbol as a section, or vice-versa. so we're pretty liberal in our
	 interpretation here; section means "try section first", not "must be a
	 section", and likewise with symbol.  */

      if (symbol_is_section)
	{
	  if (!resolve_section (symbuf, flinfo->output_bfd->sections, result, input_bfd)
	      && !resolve_symbol (symbuf, input_bfd, flinfo, result,
				  isymbuf, locsymcount))
	    {
	      undefined_reference ("section", symbuf);
	      return false;
	    }
	}
      else
	{
	  if (!resolve_symbol (symbuf, input_bfd, flinfo, result,
			       isymbuf, locsymcount)
	      && !resolve_section (symbuf, flinfo->output_bfd->sections,
				   result, input_bfd))
	    {
	      undefined_reference ("symbol", symbuf);
	      return false;
	    }
	}

      return true;

      /* All that remains are operators.  */

#define UNARY_OP(op)						\
  if (startswith (sym, #op))					\
    {								\
      sym += strlen (#op);					\
      if (*sym == ':')						\
	++sym;							\
      *symp = sym;						\
      if (!eval_symbol (&a, symp, input_bfd, flinfo, dot,	\
			isymbuf, locsymcount, signed_p))	\
	return false;						\
      if (signed_p)						\
	*result = op ((bfd_signed_vma) a);			\
      else							\
	*result = op a;						\
      return true;						\
    }

#define BINARY_OP_HEAD(op)					\
  if (startswith (sym, #op))					\
    {								\
      sym += strlen (#op);					\
      if (*sym == ':')						\
	++sym;							\
      *symp = sym;						\
      if (!eval_symbol (&a, symp, input_bfd, flinfo, dot,	\
			isymbuf, locsymcount, signed_p))	\
	return false;						\
      ++*symp;							\
      if (!eval_symbol (&b, symp, input_bfd, flinfo, dot,	\
			isymbuf, locsymcount, signed_p))	\
	return false;
#define BINARY_OP_TAIL(op)					\
      if (signed_p)						\
	*result = ((bfd_signed_vma) a) op ((bfd_signed_vma) b);	\
      else							\
	*result = a op b;					\
      return true;						\
    }
#define BINARY_OP(op) BINARY_OP_HEAD(op) BINARY_OP_TAIL(op)

    default:
      UNARY_OP  (0-);
      BINARY_OP_HEAD (<<);
      if (b >= sizeof (a) * CHAR_BIT)
	{
	  *result = 0;
	  return true;
	}
      signed_p = 0;
      BINARY_OP_TAIL (<<);
      BINARY_OP_HEAD (>>);
      if (b >= sizeof (a) * CHAR_BIT)
	{
	  *result = signed_p && (bfd_signed_vma) a < 0 ? -1 : 0;
	  return true;
	}
      BINARY_OP_TAIL (>>);
      BINARY_OP (==);
      BINARY_OP (!=);
      BINARY_OP (<=);
      BINARY_OP (>=);
      BINARY_OP (&&);
      BINARY_OP (||);
      UNARY_OP  (~);
      UNARY_OP  (!);
      BINARY_OP (*);
      BINARY_OP_HEAD (/);
      if (b == 0)
	{
	  _bfd_error_handler (_("division by zero"));
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}
      BINARY_OP_TAIL (/);
      BINARY_OP_HEAD (%);
      if (b == 0)
	{
	  _bfd_error_handler (_("division by zero"));
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}
      BINARY_OP_TAIL (%);
      BINARY_OP (^);
      BINARY_OP (|);
      BINARY_OP (&);
      BINARY_OP (+);
      BINARY_OP (-);
      BINARY_OP (<);
      BINARY_OP (>);
#undef UNARY_OP
#undef BINARY_OP
      _bfd_error_handler (_("unknown operator '%c' in complex symbol"), * sym);
      bfd_set_error (bfd_error_invalid_operation);
      return false;
    }
}

static void
put_value (bfd_vma size,
	   unsigned long chunksz,
	   bfd *input_bfd,
	   bfd_vma x,
	   bfd_byte *location)
{
  location += (size - chunksz);

  for (; size; size -= chunksz, location -= chunksz)
    {
      switch (chunksz)
	{
	case 1:
	  bfd_put_8 (input_bfd, x, location);
	  x >>= 8;
	  break;
	case 2:
	  bfd_put_16 (input_bfd, x, location);
	  x >>= 16;
	  break;
	case 4:
	  bfd_put_32 (input_bfd, x, location);
	  /* Computed this way because x >>= 32 is undefined if x is a 32-bit value.  */
	  x >>= 16;
	  x >>= 16;
	  break;
#ifdef BFD64
	case 8:
	  bfd_put_64 (input_bfd, x, location);
	  /* Computed this way because x >>= 64 is undefined if x is a 64-bit value.  */
	  x >>= 32;
	  x >>= 32;
	  break;
#endif
	default:
	  abort ();
	  break;
	}
    }
}

static bfd_vma
get_value (bfd_vma size,
	   unsigned long chunksz,
	   bfd *input_bfd,
	   bfd_byte *location)
{
  int shift;
  bfd_vma x = 0;

  /* Sanity checks.  */
  BFD_ASSERT (chunksz <= sizeof (x)
	      && size >= chunksz
	      && chunksz != 0
	      && (size % chunksz) == 0
	      && input_bfd != NULL
	      && location != NULL);

  if (chunksz == sizeof (x))
    {
      BFD_ASSERT (size == chunksz);

      /* Make sure that we do not perform an undefined shift operation.
	 We know that size == chunksz so there will only be one iteration
	 of the loop below.  */
      shift = 0;
    }
  else
    shift = 8 * chunksz;

  for (; size; size -= chunksz, location += chunksz)
    {
      switch (chunksz)
	{
	case 1:
	  x = (x << shift) | bfd_get_8 (input_bfd, location);
	  break;
	case 2:
	  x = (x << shift) | bfd_get_16 (input_bfd, location);
	  break;
	case 4:
	  x = (x << shift) | bfd_get_32 (input_bfd, location);
	  break;
#ifdef BFD64
	case 8:
	  x = (x << shift) | bfd_get_64 (input_bfd, location);
	  break;
#endif
	default:
	  abort ();
	}
    }
  return x;
}

static void
decode_complex_addend (unsigned long *start,   /* in bits */
		       unsigned long *oplen,   /* in bits */
		       unsigned long *len,     /* in bits */
		       unsigned long *wordsz,  /* in bytes */
		       unsigned long *chunksz, /* in bytes */
		       unsigned long *lsb0_p,
		       unsigned long *signed_p,
		       unsigned long *trunc_p,
		       unsigned long encoded)
{
  * start     =	 encoded	& 0x3F;
  * len	      = (encoded >>  6) & 0x3F;
  * oplen     = (encoded >> 12) & 0x3F;
  * wordsz    = (encoded >> 18) & 0xF;
  * chunksz   = (encoded >> 22) & 0xF;
  * lsb0_p    = (encoded >> 27) & 1;
  * signed_p  = (encoded >> 28) & 1;
  * trunc_p   = (encoded >> 29) & 1;
}

bfd_reloc_status_type
bfd_elf_perform_complex_relocation (bfd *input_bfd,
				    asection *input_section,
				    bfd_byte *contents,
				    Elf_Internal_Rela *rel,
				    bfd_vma relocation)
{
  bfd_vma shift, x, mask;
  unsigned long start, oplen, len, wordsz, chunksz, lsb0_p, signed_p, trunc_p;
  bfd_reloc_status_type r;
  bfd_size_type octets;

  /*  Perform this reloc, since it is complex.
      (this is not to say that it necessarily refers to a complex
      symbol; merely that it is a self-describing CGEN based reloc.
      i.e. the addend has the complete reloc information (bit start, end,
      word size, etc) encoded within it.).  */

  decode_complex_addend (&start, &oplen, &len, &wordsz,
			 &chunksz, &lsb0_p, &signed_p,
			 &trunc_p, rel->r_addend);

  mask = (((1L << (len - 1)) - 1) << 1) | 1;

  if (lsb0_p)
    shift = (start + 1) - len;
  else
    shift = (8 * wordsz) - (start + len);

  octets = rel->r_offset * bfd_octets_per_byte (input_bfd, input_section);
  x = get_value (wordsz, chunksz, input_bfd, contents + octets);

#ifdef DEBUG
  printf ("Doing complex reloc: "
	  "lsb0? %ld, signed? %ld, trunc? %ld, wordsz %ld, "
	  "chunksz %ld, start %ld, len %ld, oplen %ld\n"
	  "    dest: %8.8lx, mask: %8.8lx, reloc: %8.8lx\n",
	  lsb0_p, signed_p, trunc_p, wordsz, chunksz, start, len,
	  oplen, (unsigned long) x, (unsigned long) mask,
	  (unsigned long) relocation);
#endif

  r = bfd_reloc_ok;
  if (! trunc_p)
    /* Now do an overflow check.  */
    r = bfd_check_overflow ((signed_p
			     ? complain_overflow_signed
			     : complain_overflow_unsigned),
			    len, 0, (8 * wordsz),
			    relocation);

  /* Do the deed.  */
  x = (x & ~(mask << shift)) | ((relocation & mask) << shift);

#ifdef DEBUG
  printf ("           relocation: %8.8lx\n"
	  "         shifted mask: %8.8lx\n"
	  " shifted/masked reloc: %8.8lx\n"
	  "               result: %8.8lx\n",
	  (unsigned long) relocation, (unsigned long) (mask << shift),
	  (unsigned long) ((relocation & mask) << shift), (unsigned long) x);
#endif
  put_value (wordsz, chunksz, input_bfd, x, contents + octets);
  return r;
}

/* Functions to read r_offset from external (target order) reloc
   entry.  Faster than bfd_getl32 et al, because we let the compiler
   know the value is aligned.  */

static bfd_vma
ext32l_r_offset (const void *p)
{
  union aligned32
  {
    uint32_t v;
    unsigned char c[4];
  };
  const union aligned32 *a
    = (const union aligned32 *) &((const Elf32_External_Rel *) p)->r_offset;

  uint32_t aval = (  (uint32_t) a->c[0]
		   | (uint32_t) a->c[1] << 8
		   | (uint32_t) a->c[2] << 16
		   | (uint32_t) a->c[3] << 24);
  return aval;
}

static bfd_vma
ext32b_r_offset (const void *p)
{
  union aligned32
  {
    uint32_t v;
    unsigned char c[4];
  };
  const union aligned32 *a
    = (const union aligned32 *) &((const Elf32_External_Rel *) p)->r_offset;

  uint32_t aval = (  (uint32_t) a->c[0] << 24
		   | (uint32_t) a->c[1] << 16
		   | (uint32_t) a->c[2] << 8
		   | (uint32_t) a->c[3]);
  return aval;
}

static bfd_vma
ext64l_r_offset (const void *p)
{
  union aligned64
  {
    uint64_t v;
    unsigned char c[8];
  };
  const union aligned64 *a
    = (const union aligned64 *) &((const Elf64_External_Rel *) p)->r_offset;

  uint64_t aval = (  (uint64_t) a->c[0]
		   | (uint64_t) a->c[1] << 8
		   | (uint64_t) a->c[2] << 16
		   | (uint64_t) a->c[3] << 24
		   | (uint64_t) a->c[4] << 32
		   | (uint64_t) a->c[5] << 40
		   | (uint64_t) a->c[6] << 48
		   | (uint64_t) a->c[7] << 56);
  return aval;
}

static bfd_vma
ext64b_r_offset (const void *p)
{
  union aligned64
  {
    uint64_t v;
    unsigned char c[8];
  };
  const union aligned64 *a
    = (const union aligned64 *) &((const Elf64_External_Rel *) p)->r_offset;

  uint64_t aval = (  (uint64_t) a->c[0] << 56
		   | (uint64_t) a->c[1] << 48
		   | (uint64_t) a->c[2] << 40
		   | (uint64_t) a->c[3] << 32
		   | (uint64_t) a->c[4] << 24
		   | (uint64_t) a->c[5] << 16
		   | (uint64_t) a->c[6] << 8
		   | (uint64_t) a->c[7]);
  return aval;
}

/* When performing a relocatable link, the input relocations are
   preserved.  But, if they reference global symbols, the indices
   referenced must be updated.  Update all the relocations found in
   RELDATA.  */

static bool
elf_link_adjust_relocs (bfd *abfd,
			asection *sec,
			struct bfd_elf_section_reloc_data *reldata,
			bool sort,
			struct bfd_link_info *info)
{
  unsigned int i;
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  bfd_byte *erela;
  void (*swap_in) (bfd *, const bfd_byte *, Elf_Internal_Rela *);
  void (*swap_out) (bfd *, const Elf_Internal_Rela *, bfd_byte *);
  bfd_vma r_type_mask;
  int r_sym_shift;
  unsigned int count = reldata->count;
  struct elf_link_hash_entry **rel_hash = reldata->hashes;

  if (reldata->hdr->sh_entsize == bed->s->sizeof_rel)
    {
      swap_in = bed->s->swap_reloc_in;
      swap_out = bed->s->swap_reloc_out;
    }
  else if (reldata->hdr->sh_entsize == bed->s->sizeof_rela)
    {
      swap_in = bed->s->swap_reloca_in;
      swap_out = bed->s->swap_reloca_out;
    }
  else
    abort ();

  if (bed->s->int_rels_per_ext_rel > MAX_INT_RELS_PER_EXT_REL)
    abort ();

  if (bed->s->arch_size == 32)
    {
      r_type_mask = 0xff;
      r_sym_shift = 8;
    }
  else
    {
      r_type_mask = 0xffffffff;
      r_sym_shift = 32;
    }

  erela = reldata->hdr->contents;
  for (i = 0; i < count; i++, rel_hash++, erela += reldata->hdr->sh_entsize)
    {
      Elf_Internal_Rela irela[MAX_INT_RELS_PER_EXT_REL];
      unsigned int j;

      if (*rel_hash == NULL)
	continue;

      if ((*rel_hash)->indx == -2
	  && info->gc_sections
	  && ! info->gc_keep_exported)
	{
	  /* PR 21524: Let the user know if a symbol was removed by garbage collection.  */
	  _bfd_error_handler (_("%pB:%pA: error: relocation references symbol %s which was removed by garbage collection"),
			      abfd, sec,
			      (*rel_hash)->root.root.string);
	  _bfd_error_handler (_("%pB:%pA: error: try relinking with --gc-keep-exported enabled"),
			      abfd, sec);
	  bfd_set_error (bfd_error_invalid_operation);
	  return false;
	}
      BFD_ASSERT ((*rel_hash)->indx >= 0);

      (*swap_in) (abfd, erela, irela);
      for (j = 0; j < bed->s->int_rels_per_ext_rel; j++)
	irela[j].r_info = ((bfd_vma) (*rel_hash)->indx << r_sym_shift
			   | (irela[j].r_info & r_type_mask));
      (*swap_out) (abfd, irela, erela);
    }

  if (bed->elf_backend_update_relocs)
    (*bed->elf_backend_update_relocs) (sec, reldata);

  if (sort && count != 0)
    {
      bfd_vma (*ext_r_off) (const void *);
      bfd_vma r_off;
      size_t elt_size;
      bfd_byte *base, *end, *p, *loc;
      bfd_byte *buf = NULL;

      if (bed->s->arch_size == 32)
	{
	  if (abfd->xvec->header_byteorder == BFD_ENDIAN_LITTLE)
	    ext_r_off = ext32l_r_offset;
	  else if (abfd->xvec->header_byteorder == BFD_ENDIAN_BIG)
	    ext_r_off = ext32b_r_offset;
	  else
	    abort ();
	}
      else
	{
	  if (abfd->xvec->header_byteorder == BFD_ENDIAN_LITTLE)
	    ext_r_off = ext64l_r_offset;
	  else if (abfd->xvec->header_byteorder == BFD_ENDIAN_BIG)
	    ext_r_off = ext64b_r_offset;
	  else
	    abort ();
	}

      /*  Must use a stable sort here.  A modified insertion sort,
	  since the relocs are mostly sorted already.  */
      elt_size = reldata->hdr->sh_entsize;
      base = reldata->hdr->contents;
      end = base + count * elt_size;
      if (elt_size > sizeof (Elf64_External_Rela))
	abort ();

      /* Ensure the first element is lowest.  This acts as a sentinel,
	 speeding the main loop below.  */
      r_off = (*ext_r_off) (base);
      for (p = loc = base; (p += elt_size) < end; )
	{
	  bfd_vma r_off2 = (*ext_r_off) (p);
	  if (r_off > r_off2)
	    {
	      r_off = r_off2;
	      loc = p;
	    }
	}
      if (loc != base)
	{
	  /* Don't just swap *base and *loc as that changes the order
	     of the original base[0] and base[1] if they happen to
	     have the same r_offset.  */
	  bfd_byte onebuf[sizeof (Elf64_External_Rela)];
	  memcpy (onebuf, loc, elt_size);
	  memmove (base + elt_size, base, loc - base);
	  memcpy (base, onebuf, elt_size);
	}

      for (p = base + elt_size; (p += elt_size) < end; )
	{
	  /* base to p is sorted, *p is next to insert.  */
	  r_off = (*ext_r_off) (p);
	  /* Search the sorted region for location to insert.  */
	  loc = p - elt_size;
	  while (r_off < (*ext_r_off) (loc))
	    loc -= elt_size;
	  loc += elt_size;
	  if (loc != p)
	    {
	      /* Chances are there is a run of relocs to insert here,
		 from one of more input files.  Files are not always
		 linked in order due to the way elf_link_input_bfd is
		 called.  See pr17666.  */
	      size_t sortlen = p - loc;
	      bfd_vma r_off2 = (*ext_r_off) (loc);
	      size_t runlen = elt_size;
	      bfd_vma r_off_runend = r_off;
	      bfd_vma r_off_runend_next;
	      size_t buf_size = 96 * 1024;
	      while (p + runlen < end
		     && (sortlen <= buf_size
			 || runlen + elt_size <= buf_size)
		     /* run must not break the ordering of base..loc+1 */
		     && r_off2 > (r_off_runend_next = (*ext_r_off) (p + runlen))
		     /* run must be already sorted */
		     && r_off_runend_next >= r_off_runend)
		{
		  runlen += elt_size;
		  r_off_runend = r_off_runend_next;
		}
	      if (buf == NULL)
		{
		  buf = bfd_malloc (buf_size);
		  if (buf == NULL)
		    return false;
		}
	      if (runlen < sortlen)
		{
		  memcpy (buf, p, runlen);
		  memmove (loc + runlen, loc, sortlen);
		  memcpy (loc, buf, runlen);
		}
	      else
		{
		  memcpy (buf, loc, sortlen);
		  memmove (loc, p, runlen);
		  memcpy (loc + runlen, buf, sortlen);
		}
	      p += runlen - elt_size;
	    }
	}
      /* Hashes are no longer valid.  */
      free (reldata->hashes);
      reldata->hashes = NULL;
      free (buf);
    }
  return true;
}

struct elf_link_sort_rela
{
  union {
    bfd_vma offset;
    bfd_vma sym_mask;
  } u;
  enum elf_reloc_type_class type;
  /* We use this as an array of size int_rels_per_ext_rel.  */
  Elf_Internal_Rela rela[1];
};

/* qsort stability here and for cmp2 is only an issue if multiple
   dynamic relocations are emitted at the same address.  But targets
   that apply a series of dynamic relocations each operating on the
   result of the prior relocation can't use -z combreloc as
   implemented anyway.  Such schemes tend to be broken by sorting on
   symbol index.  That leaves dynamic NONE relocs as the only other
   case where ld might emit multiple relocs at the same address, and
   those are only emitted due to target bugs.  */

static int
elf_link_sort_cmp1 (const void *A, const void *B)
{
  const struct elf_link_sort_rela *a = (const struct elf_link_sort_rela *) A;
  const struct elf_link_sort_rela *b = (const struct elf_link_sort_rela *) B;
  int relativea, relativeb;

  relativea = a->type == reloc_class_relative;
  relativeb = b->type == reloc_class_relative;

  if (relativea < relativeb)
    return 1;
  if (relativea > relativeb)
    return -1;
  if ((a->rela->r_info & a->u.sym_mask) < (b->rela->r_info & b->u.sym_mask))
    return -1;
  if ((a->rela->r_info & a->u.sym_mask) > (b->rela->r_info & b->u.sym_mask))
    return 1;
  if (a->rela->r_offset < b->rela->r_offset)
    return -1;
  if (a->rela->r_offset > b->rela->r_offset)
    return 1;
  return 0;
}

static int
elf_link_sort_cmp2 (const void *A, const void *B)
{
  const struct elf_link_sort_rela *a = (const struct elf_link_sort_rela *) A;
  const struct elf_link_sort_rela *b = (const struct elf_link_sort_rela *) B;

  if (a->type < b->type)
    return -1;
  if (a->type > b->type)
    return 1;
  if (a->u.offset < b->u.offset)
    return -1;
  if (a->u.offset > b->u.offset)
    return 1;
  if (a->rela->r_offset < b->rela->r_offset)
    return -1;
  if (a->rela->r_offset > b->rela->r_offset)
    return 1;
  return 0;
}

static size_t
elf_link_sort_relocs (bfd *abfd, struct bfd_link_info *info, asection **psec)
{
  asection *dynamic_relocs;
  asection *rela_dyn;
  asection *rel_dyn;
  bfd_size_type count, size;
  size_t i, ret, sort_elt, ext_size;
  bfd_byte *sort, *s_non_relative, *p;
  struct elf_link_sort_rela *sq;
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  int i2e = bed->s->int_rels_per_ext_rel;
  unsigned int opb = bfd_octets_per_byte (abfd, NULL);
  void (*swap_in) (bfd *, const bfd_byte *, Elf_Internal_Rela *);
  void (*swap_out) (bfd *, const Elf_Internal_Rela *, bfd_byte *);
  struct bfd_link_order *lo;
  bfd_vma r_sym_mask;
  bool use_rela;

  /* Find a dynamic reloc section.  */
  rela_dyn = bfd_get_section_by_name (abfd, ".rela.dyn");
  rel_dyn  = bfd_get_section_by_name (abfd, ".rel.dyn");
  if (rela_dyn != NULL && rela_dyn->size > 0
      && rel_dyn != NULL && rel_dyn->size > 0)
    {
      bool use_rela_initialised = false;

      /* This is just here to stop gcc from complaining.
	 Its initialization checking code is not perfect.  */
      use_rela = true;

      /* Both sections are present.  Examine the sizes
	 of the indirect sections to help us choose.  */
      for (lo = rela_dyn->map_head.link_order; lo != NULL; lo = lo->next)
	if (lo->type == bfd_indirect_link_order)
	  {
	    asection *o = lo->u.indirect.section;

	    if ((o->size % bed->s->sizeof_rela) == 0)
	      {
		if ((o->size % bed->s->sizeof_rel) == 0)
		  /* Section size is divisible by both rel and rela sizes.
		     It is of no help to us.  */
		  ;
		else
		  {
		    /* Section size is only divisible by rela.  */
		    if (use_rela_initialised && !use_rela)
		      {
			_bfd_error_handler (_("%pB: unable to sort relocs - "
					      "they are in more than one size"),
					    abfd);
			bfd_set_error (bfd_error_invalid_operation);
			return 0;
		      }
		    else
		      {
			use_rela = true;
			use_rela_initialised = true;
		      }
		  }
	      }
	    else if ((o->size % bed->s->sizeof_rel) == 0)
	      {
		/* Section size is only divisible by rel.  */
		if (use_rela_initialised && use_rela)
		  {
		    _bfd_error_handler (_("%pB: unable to sort relocs - "
					  "they are in more than one size"),
					abfd);
		    bfd_set_error (bfd_error_invalid_operation);
		    return 0;
		  }
		else
		  {
		    use_rela = false;
		    use_rela_initialised = true;
		  }
	      }
	    else
	      {
		/* The section size is not divisible by either -
		   something is wrong.  */
		_bfd_error_handler (_("%pB: unable to sort relocs - "
				      "they are of an unknown size"), abfd);
		bfd_set_error (bfd_error_invalid_operation);
		return 0;
	      }
	  }

      for (lo = rel_dyn->map_head.link_order; lo != NULL; lo = lo->next)
	if (lo->type == bfd_indirect_link_order)
	  {
	    asection *o = lo->u.indirect.section;

	    if ((o->size % bed->s->sizeof_rela) == 0)
	      {
		if ((o->size % bed->s->sizeof_rel) == 0)
		  /* Section size is divisible by both rel and rela sizes.
		     It is of no help to us.  */
		  ;
		else
		  {
		    /* Section size is only divisible by rela.  */
		    if (use_rela_initialised && !use_rela)
		      {
			_bfd_error_handler (_("%pB: unable to sort relocs - "
					      "they are in more than one size"),
					    abfd);
			bfd_set_error (bfd_error_invalid_operation);
			return 0;
		      }
		    else
		      {
			use_rela = true;
			use_rela_initialised = true;
		      }
		  }
	      }
	    else if ((o->size % bed->s->sizeof_rel) == 0)
	      {
		/* Section size is only divisible by rel.  */
		if (use_rela_initialised && use_rela)
		  {
		    _bfd_error_handler (_("%pB: unable to sort relocs - "
					  "they are in more than one size"),
					abfd);
		    bfd_set_error (bfd_error_invalid_operation);
		    return 0;
		  }
		else
		  {
		    use_rela = false;
		    use_rela_initialised = true;
		  }
	      }
	    else
	      {
		/* The section size is not divisible by either -
		   something is wrong.  */
		_bfd_error_handler (_("%pB: unable to sort relocs - "
				      "they are of an unknown size"), abfd);
		bfd_set_error (bfd_error_invalid_operation);
		return 0;
	      }
	  }

      if (! use_rela_initialised)
	/* Make a guess.  */
	use_rela = true;
    }
  else if (rela_dyn != NULL && rela_dyn->size > 0)
    use_rela = true;
  else if (rel_dyn != NULL && rel_dyn->size > 0)
    use_rela = false;
  else
    return 0;

  if (use_rela)
    {
      dynamic_relocs = rela_dyn;
      ext_size = bed->s->sizeof_rela;
      swap_in = bed->s->swap_reloca_in;
      swap_out = bed->s->swap_reloca_out;
    }
  else
    {
      dynamic_relocs = rel_dyn;
      ext_size = bed->s->sizeof_rel;
      swap_in = bed->s->swap_reloc_in;
      swap_out = bed->s->swap_reloc_out;
    }

  size = 0;
  for (lo = dynamic_relocs->map_head.link_order; lo != NULL; lo = lo->next)
    if (lo->type == bfd_indirect_link_order)
      size += lo->u.indirect.section->size;

  if (size != dynamic_relocs->size)
    return 0;

  sort_elt = (sizeof (struct elf_link_sort_rela)
	      + (i2e - 1) * sizeof (Elf_Internal_Rela));

  count = dynamic_relocs->size / ext_size;
  if (count == 0)
    return 0;
  sort = (bfd_byte *) bfd_zmalloc (sort_elt * count);

  if (sort == NULL)
    {
      (*info->callbacks->warning)
	(info, _("not enough memory to sort relocations"), 0, abfd, 0, 0);
      return 0;
    }

  if (bed->s->arch_size == 32)
    r_sym_mask = ~(bfd_vma) 0xff;
  else
    r_sym_mask = ~(bfd_vma) 0xffffffff;

  for (lo = dynamic_relocs->map_head.link_order; lo != NULL; lo = lo->next)
    if (lo->type == bfd_indirect_link_order)
      {
	bfd_byte *erel, *erelend;
	asection *o = lo->u.indirect.section;

	if (o->contents == NULL && o->size != 0)
	  {
	    /* This is a reloc section that is being handled as a normal
	       section.  See bfd_section_from_shdr.  We can't combine
	       relocs in this case.  */
	    free (sort);
	    return 0;
	  }
	erel = o->contents;
	erelend = o->contents + o->size;
	p = sort + o->output_offset * opb / ext_size * sort_elt;

	while (erel < erelend)
	  {
	    struct elf_link_sort_rela *s = (struct elf_link_sort_rela *) p;

	    (*swap_in) (abfd, erel, s->rela);
	    s->type = (*bed->elf_backend_reloc_type_class) (info, o, s->rela);
	    s->u.sym_mask = r_sym_mask;
	    p += sort_elt;
	    erel += ext_size;
	  }
      }

  qsort (sort, count, sort_elt, elf_link_sort_cmp1);

  for (i = 0, p = sort; i < count; i++, p += sort_elt)
    {
      struct elf_link_sort_rela *s = (struct elf_link_sort_rela *) p;
      if (s->type != reloc_class_relative)
	break;
    }
  ret = i;
  s_non_relative = p;

  sq = (struct elf_link_sort_rela *) s_non_relative;
  for (; i < count; i++, p += sort_elt)
    {
      struct elf_link_sort_rela *sp = (struct elf_link_sort_rela *) p;
      if (((sp->rela->r_info ^ sq->rela->r_info) & r_sym_mask) != 0)
	sq = sp;
      sp->u.offset = sq->rela->r_offset;
    }

  qsort (s_non_relative, count - ret, sort_elt, elf_link_sort_cmp2);

  struct elf_link_hash_table *htab = elf_hash_table (info);
  if (htab->srelplt && htab->srelplt->output_section == dynamic_relocs)
    {
      /* We have plt relocs in .rela.dyn.  */
      sq = (struct elf_link_sort_rela *) sort;
      for (i = 0; i < count; i++)
	if (sq[count - i - 1].type != reloc_class_plt)
	  break;
      if (i != 0 && htab->srelplt->size == i * ext_size)
	{
	  struct bfd_link_order **plo;
	  /* Put srelplt link_order last.  This is so the output_offset
	     set in the next loop is correct for DT_JMPREL.  */
	  for (plo = &dynamic_relocs->map_head.link_order; *plo != NULL; )
	    if ((*plo)->type == bfd_indirect_link_order
		&& (*plo)->u.indirect.section == htab->srelplt)
	      {
		lo = *plo;
		*plo = lo->next;
	      }
	    else
	      plo = &(*plo)->next;
	  *plo = lo;
	  lo->next = NULL;
	  dynamic_relocs->map_tail.link_order = lo;
	}
    }

  p = sort;
  for (lo = dynamic_relocs->map_head.link_order; lo != NULL; lo = lo->next)
    if (lo->type == bfd_indirect_link_order)
      {
	bfd_byte *erel, *erelend;
	asection *o = lo->u.indirect.section;

	erel = o->contents;
	erelend = o->contents + o->size;
	o->output_offset = (p - sort) / sort_elt * ext_size / opb;
	while (erel < erelend)
	  {
	    struct elf_link_sort_rela *s = (struct elf_link_sort_rela *) p;
	    (*swap_out) (abfd, s->rela, erel);
	    p += sort_elt;
	    erel += ext_size;
	  }
      }

  free (sort);
  *psec = dynamic_relocs;
  return ret;
}

/* Add a symbol to the output symbol string table.  */

static int
elf_link_output_symstrtab (void *finf,
			   const char *name,
			   Elf_Internal_Sym *elfsym,
			   asection *input_sec,
			   struct elf_link_hash_entry *h)
{
  struct elf_final_link_info *flinfo = finf;
  int (*output_symbol_hook)
    (struct bfd_link_info *, const char *, Elf_Internal_Sym *, asection *,
     struct elf_link_hash_entry *);
  struct elf_link_hash_table *hash_table;
  const struct elf_backend_data *bed;
  bfd_size_type strtabsize;

  BFD_ASSERT (elf_onesymtab (flinfo->output_bfd));

  bed = get_elf_backend_data (flinfo->output_bfd);
  output_symbol_hook = bed->elf_backend_link_output_symbol_hook;
  if (output_symbol_hook != NULL)
    {
      int ret = (*output_symbol_hook) (flinfo->info, name, elfsym, input_sec, h);
      if (ret != 1)
	return ret;
    }

  if (ELF_ST_TYPE (elfsym->st_info) == STT_GNU_IFUNC)
    elf_tdata (flinfo->output_bfd)->has_gnu_osabi |= elf_gnu_osabi_ifunc;
  if (ELF_ST_BIND (elfsym->st_info) == STB_GNU_UNIQUE)
    elf_tdata (flinfo->output_bfd)->has_gnu_osabi |= elf_gnu_osabi_unique;

  if (name == NULL || *name == '\0')
    elfsym->st_name = (unsigned long) -1;
  else
    {
      /* Call _bfd_elf_strtab_offset after _bfd_elf_strtab_finalize
	 to get the final offset for st_name.  */
      char *versioned_name = (char *) name;
      if (h != NULL)
	{
	  if (h->versioned == versioned && h->def_dynamic)
	    {
	      /* Keep only one '@' for versioned symbols defined in
	         shared objects.  */
	      char *version = strrchr (name, ELF_VER_CHR);
	      char *base_end = strchr (name, ELF_VER_CHR);
	      if (version != base_end)
		{
		  size_t base_len;
		  size_t len = strlen (name);
		  versioned_name = bfd_alloc (flinfo->output_bfd, len);
		  if (versioned_name == NULL)
		    return 0;
		  base_len = base_end - name;
		  memcpy (versioned_name, name, base_len);
		  memcpy (versioned_name + base_len, version,
			  len - base_len);
		}
	    }
	}
      else if (flinfo->info->unique_symbol
	       && ELF_ST_BIND (elfsym->st_info) == STB_LOCAL)
	{
	  struct local_hash_entry *lh;
	  size_t count_len;
	  size_t base_len;
	  char buf[30];
	  switch (ELF_ST_TYPE (elfsym->st_info))
	    {
	    case STT_FILE:
	    case STT_SECTION:
	      break;
	    default:
	      lh = (struct local_hash_entry *) bfd_hash_lookup
		     (&flinfo->local_hash_table, name, true, false);
	      if (lh == NULL)
		return 0;
	      /* Always append ".COUNT" to local symbols to avoid
		 potential conflicts with local symbol "XXX.COUNT".  */
	      sprintf (buf, "%lx", lh->count);
	      base_len = lh->size;
	      if (!base_len)
		{
		  base_len = strlen (name);
		  lh->size = base_len;
		}
	      count_len = strlen (buf);
	      versioned_name = bfd_alloc (flinfo->output_bfd,
					  base_len + count_len + 2);
	      if (versioned_name == NULL)
		return 0;
	      memcpy (versioned_name, name, base_len);
	      versioned_name[base_len] = '.';
	      memcpy (versioned_name + base_len + 1, buf,
		      count_len + 1);
	      lh->count++;
	      break;
	    }
	}
      elfsym->st_name
	= (unsigned long) _bfd_elf_strtab_add (flinfo->symstrtab,
					       versioned_name, false);
      if (elfsym->st_name == (unsigned long) -1)
	return 0;
    }

  hash_table = elf_hash_table (flinfo->info);
  strtabsize = hash_table->strtabsize;
  if (strtabsize <= flinfo->output_bfd->symcount)
    {
      strtabsize += strtabsize;
      hash_table->strtabsize = strtabsize;
      strtabsize *= sizeof (*hash_table->strtab);
      hash_table->strtab
	= (struct elf_sym_strtab *) bfd_realloc (hash_table->strtab,
						 strtabsize);
      if (hash_table->strtab == NULL)
	return 0;
    }
  hash_table->strtab[flinfo->output_bfd->symcount].sym = *elfsym;
  hash_table->strtab[flinfo->output_bfd->symcount].dest_index
    = flinfo->output_bfd->symcount;
  flinfo->output_bfd->symcount += 1;

  return 1;
}

/* Swap symbols out to the symbol table and flush the output symbols to
   the file.  */

static bool
elf_link_swap_symbols_out (struct elf_final_link_info *flinfo)
{
  struct elf_link_hash_table *hash_table = elf_hash_table (flinfo->info);
  size_t amt;
  size_t i;
  const struct elf_backend_data *bed;
  bfd_byte *symbuf;
  Elf_Internal_Shdr *hdr;
  file_ptr pos;
  bool ret;

  if (flinfo->output_bfd->symcount == 0)
    return true;

  BFD_ASSERT (elf_onesymtab (flinfo->output_bfd));

  bed = get_elf_backend_data (flinfo->output_bfd);

  amt = bed->s->sizeof_sym * flinfo->output_bfd->symcount;
  symbuf = (bfd_byte *) bfd_malloc (amt);
  if (symbuf == NULL)
    return false;

  if (flinfo->symshndxbuf)
    {
      amt = sizeof (Elf_External_Sym_Shndx);
      amt *= bfd_get_symcount (flinfo->output_bfd);
      flinfo->symshndxbuf = (Elf_External_Sym_Shndx *) bfd_zmalloc (amt);
      if (flinfo->symshndxbuf == NULL)
	{
	  free (symbuf);
	  return false;
	}
    }

  /* Now swap out the symbols.  */
  for (i = 0; i < flinfo->output_bfd->symcount; i++)
    {
      struct elf_sym_strtab *elfsym = &hash_table->strtab[i];
      if (elfsym->sym.st_name == (unsigned long) -1)
	elfsym->sym.st_name = 0;
      else
	elfsym->sym.st_name
	  = (unsigned long) _bfd_elf_strtab_offset (flinfo->symstrtab,
						    elfsym->sym.st_name);

      /* Inform the linker of the addition of this symbol.  */

      if (flinfo->info->callbacks->ctf_new_symbol)
	flinfo->info->callbacks->ctf_new_symbol (elfsym->dest_index,
						 &elfsym->sym);

      bed->s->swap_symbol_out (flinfo->output_bfd, &elfsym->sym,
			       ((bfd_byte *) symbuf
				+ (elfsym->dest_index
				   * bed->s->sizeof_sym)),
			       NPTR_ADD (flinfo->symshndxbuf,
					 elfsym->dest_index));
    }

  hdr = &elf_tdata (flinfo->output_bfd)->symtab_hdr;
  pos = hdr->sh_offset + hdr->sh_size;
  amt = bed->s->sizeof_sym * flinfo->output_bfd->symcount;
  if (bfd_seek (flinfo->output_bfd, pos, SEEK_SET) == 0
      && bfd_bwrite (symbuf, amt, flinfo->output_bfd) == amt)
    {
      hdr->sh_size += amt;
      ret = true;
    }
  else
    ret = false;

  free (symbuf);

  free (hash_table->strtab);
  hash_table->strtab = NULL;

  return ret;
}

/* Return TRUE if the dynamic symbol SYM in ABFD is supported.  */

static bool
check_dynsym (bfd *abfd, Elf_Internal_Sym *sym)
{
  if (sym->st_shndx >= (SHN_LORESERVE & 0xffff)
      && sym->st_shndx < SHN_LORESERVE)
    {
      /* The gABI doesn't support dynamic symbols in output sections
	 beyond 64k.  */
      _bfd_error_handler
	/* xgettext:c-format */
	(_("%pB: too many sections: %d (>= %d)"),
	 abfd, bfd_count_sections (abfd), SHN_LORESERVE & 0xffff);
      bfd_set_error (bfd_error_nonrepresentable_section);
      return false;
    }
  return true;
}

/* For DSOs loaded in via a DT_NEEDED entry, emulate ld.so in
   allowing an unsatisfied unversioned symbol in the DSO to match a
   versioned symbol that would normally require an explicit version.
   We also handle the case that a DSO references a hidden symbol
   which may be satisfied by a versioned symbol in another DSO.  */

static bool
elf_link_check_versioned_symbol (struct bfd_link_info *info,
				 const struct elf_backend_data *bed,
				 struct elf_link_hash_entry *h)
{
  bfd *abfd;
  struct elf_link_loaded_list *loaded;

  if (!is_elf_hash_table (info->hash))
    return false;

  /* Check indirect symbol.  */
  while (h->root.type == bfd_link_hash_indirect)
    h = (struct elf_link_hash_entry *) h->root.u.i.link;

  switch (h->root.type)
    {
    default:
      abfd = NULL;
      break;

    case bfd_link_hash_undefined:
    case bfd_link_hash_undefweak:
      abfd = h->root.u.undef.abfd;
      if (abfd == NULL
	  || (abfd->flags & DYNAMIC) == 0
	  || (elf_dyn_lib_class (abfd) & DYN_DT_NEEDED) == 0)
	return false;
      break;

    case bfd_link_hash_defined:
    case bfd_link_hash_defweak:
      abfd = h->root.u.def.section->owner;
      break;

    case bfd_link_hash_common:
      abfd = h->root.u.c.p->section->owner;
      break;
    }
  BFD_ASSERT (abfd != NULL);

  for (loaded = elf_hash_table (info)->dyn_loaded;
       loaded != NULL;
       loaded = loaded->next)
    {
      bfd *input;
      Elf_Internal_Shdr *hdr;
      size_t symcount;
      size_t extsymcount;
      size_t extsymoff;
      Elf_Internal_Shdr *versymhdr;
      Elf_Internal_Sym *isym;
      Elf_Internal_Sym *isymend;
      Elf_Internal_Sym *isymbuf;
      Elf_External_Versym *ever;
      Elf_External_Versym *extversym;

      input = loaded->abfd;

      /* We check each DSO for a possible hidden versioned definition.  */
      if (input == abfd
	  || elf_dynversym (input) == 0)
	continue;

      hdr = &elf_tdata (input)->dynsymtab_hdr;

      symcount = hdr->sh_size / bed->s->sizeof_sym;
      if (elf_bad_symtab (input))
	{
	  extsymcount = symcount;
	  extsymoff = 0;
	}
      else
	{
	  extsymcount = symcount - hdr->sh_info;
	  extsymoff = hdr->sh_info;
	}

      if (extsymcount == 0)
	continue;

      isymbuf = bfd_elf_get_elf_syms (input, hdr, extsymcount, extsymoff,
				      NULL, NULL, NULL);
      if (isymbuf == NULL)
	return false;

      /* Read in any version definitions.  */
      versymhdr = &elf_tdata (input)->dynversym_hdr;
      if (bfd_seek (input, versymhdr->sh_offset, SEEK_SET) != 0
	  || (extversym = (Elf_External_Versym *)
	      _bfd_malloc_and_read (input, versymhdr->sh_size,
				    versymhdr->sh_size)) == NULL)
	{
	  free (isymbuf);
	  return false;
	}

      ever = extversym + extsymoff;
      isymend = isymbuf + extsymcount;
      for (isym = isymbuf; isym < isymend; isym++, ever++)
	{
	  const char *name;
	  Elf_Internal_Versym iver;
	  unsigned short version_index;

	  if (ELF_ST_BIND (isym->st_info) == STB_LOCAL
	      || isym->st_shndx == SHN_UNDEF)
	    continue;

	  name = bfd_elf_string_from_elf_section (input,
						  hdr->sh_link,
						  isym->st_name);
	  if (strcmp (name, h->root.root.string) != 0)
	    continue;

	  _bfd_elf_swap_versym_in (input, ever, &iver);

	  if ((iver.vs_vers & VERSYM_HIDDEN) == 0
	      && !(h->def_regular
		   && h->forced_local))
	    {
	      /* If we have a non-hidden versioned sym, then it should
		 have provided a definition for the undefined sym unless
		 it is defined in a non-shared object and forced local.
	       */
	      abort ();
	    }

	  version_index = iver.vs_vers & VERSYM_VERSION;
	  if (version_index == 1 || version_index == 2)
	    {
	      /* This is the base or first version.  We can use it.  */
	      free (extversym);
	      free (isymbuf);
	      return true;
	    }
	}

      free (extversym);
      free (isymbuf);
    }

  return false;
}

/* Convert ELF common symbol TYPE.  */

static int
elf_link_convert_common_type (struct bfd_link_info *info, int type)
{
  /* Commom symbol can only appear in relocatable link.  */
  if (!bfd_link_relocatable (info))
    abort ();
  switch (info->elf_stt_common)
    {
    case unchanged:
      break;
    case elf_stt_common:
      type = STT_COMMON;
      break;
    case no_elf_stt_common:
      type = STT_OBJECT;
      break;
    }
  return type;
}

/* Add an external symbol to the symbol table.  This is called from
   the hash table traversal routine.  When generating a shared object,
   we go through the symbol table twice.  The first time we output
   anything that might have been forced to local scope in a version
   script.  The second time we output the symbols that are still
   global symbols.  */

static bool
elf_link_output_extsym (struct bfd_hash_entry *bh, void *data)
{
  struct elf_link_hash_entry *h = (struct elf_link_hash_entry *) bh;
  struct elf_outext_info *eoinfo = (struct elf_outext_info *) data;
  struct elf_final_link_info *flinfo = eoinfo->flinfo;
  bool strip;
  Elf_Internal_Sym sym;
  asection *input_sec;
  const struct elf_backend_data *bed;
  long indx;
  int ret;
  unsigned int type;

  if (h->root.type == bfd_link_hash_warning)
    {
      h = (struct elf_link_hash_entry *) h->root.u.i.link;
      if (h->root.type == bfd_link_hash_new)
	return true;
    }

  /* Decide whether to output this symbol in this pass.  */
  if (eoinfo->localsyms)
    {
      if (!h->forced_local)
	return true;
    }
  else
    {
      if (h->forced_local)
	return true;
    }

  bed = get_elf_backend_data (flinfo->output_bfd);

  if (h->root.type == bfd_link_hash_undefined)
    {
      /* If we have an undefined symbol reference here then it must have
	 come from a shared library that is being linked in.  (Undefined
	 references in regular files have already been handled unless
	 they are in unreferenced sections which are removed by garbage
	 collection).  */
      bool ignore_undef = false;

      /* Some symbols may be special in that the fact that they're
	 undefined can be safely ignored - let backend determine that.  */
      if (bed->elf_backend_ignore_undef_symbol)
	ignore_undef = bed->elf_backend_ignore_undef_symbol (h);

      /* If we are reporting errors for this situation then do so now.  */
      if (!ignore_undef
	  && h->ref_dynamic_nonweak
	  && (!h->ref_regular || flinfo->info->gc_sections)
	  && !elf_link_check_versioned_symbol (flinfo->info, bed, h)
	  && flinfo->info->unresolved_syms_in_shared_libs != RM_IGNORE)
	{
	  flinfo->info->callbacks->undefined_symbol
	    (flinfo->info, h->root.root.string,
	     h->ref_regular ? NULL : h->root.u.undef.abfd, NULL, 0,
	     flinfo->info->unresolved_syms_in_shared_libs == RM_DIAGNOSE
	     && !flinfo->info->warn_unresolved_syms);
	}

      /* Strip a global symbol defined in a discarded section.  */
      if (h->indx == -3)
	return true;
    }

  /* We should also warn if a forced local symbol is referenced from
     shared libraries.  */
  if (bfd_link_executable (flinfo->info)
      && h->forced_local
      && h->ref_dynamic
      && h->def_regular
      && !h->dynamic_def
      && h->ref_dynamic_nonweak
      && !elf_link_check_versioned_symbol (flinfo->info, bed, h))
    {
      bfd *def_bfd;
      const char *msg;
      struct elf_link_hash_entry *hi = h;

      /* Check indirect symbol.  */
      while (hi->root.type == bfd_link_hash_indirect)
	hi = (struct elf_link_hash_entry *) hi->root.u.i.link;

      if (ELF_ST_VISIBILITY (h->other) == STV_INTERNAL)
	/* xgettext:c-format */
	msg = _("%pB: internal symbol `%s' in %pB is referenced by DSO");
      else if (ELF_ST_VISIBILITY (h->other) == STV_HIDDEN)
	/* xgettext:c-format */
	msg = _("%pB: hidden symbol `%s' in %pB is referenced by DSO");
      else
	/* xgettext:c-format */
	msg = _("%pB: local symbol `%s' in %pB is referenced by DSO");
      def_bfd = flinfo->output_bfd;
      if (hi->root.u.def.section != bfd_abs_section_ptr)
	def_bfd = hi->root.u.def.section->owner;
      _bfd_error_handler (msg, flinfo->output_bfd,
			  h->root.root.string, def_bfd);
      bfd_set_error (bfd_error_bad_value);
      eoinfo->failed = true;
      return false;
    }

  /* We don't want to output symbols that have never been mentioned by
     a regular file, or that we have been told to strip.  However, if
     h->indx is set to -2, the symbol is used by a reloc and we must
     output it.  */
  strip = false;
  if (h->indx == -2)
    ;
  else if ((h->def_dynamic
	    || h->ref_dynamic
	    || h->root.type == bfd_link_hash_new)
	   && !h->def_regular
	   && !h->ref_regular)
    strip = true;
  else if (flinfo->info->strip == strip_all)
    strip = true;
  else if (flinfo->info->strip == strip_some
	   && bfd_hash_lookup (flinfo->info->keep_hash,
			       h->root.root.string, false, false) == NULL)
    strip = true;
  else if ((h->root.type == bfd_link_hash_defined
	    || h->root.type == bfd_link_hash_defweak)
	   && ((flinfo->info->strip_discarded
		&& discarded_section (h->root.u.def.section))
	       || ((h->root.u.def.section->flags & SEC_LINKER_CREATED) == 0
		   && h->root.u.def.section->owner != NULL
		   && (h->root.u.def.section->owner->flags & BFD_PLUGIN) != 0)))
    strip = true;
  else if ((h->root.type == bfd_link_hash_undefined
	    || h->root.type == bfd_link_hash_undefweak)
	   && h->root.u.undef.abfd != NULL
	   && (h->root.u.undef.abfd->flags & BFD_PLUGIN) != 0)
    strip = true;

  type = h->type;

  /* If we're stripping it, and it's not a dynamic symbol, there's
     nothing else to do.   However, if it is a forced local symbol or
     an ifunc symbol we need to give the backend finish_dynamic_symbol
     function a chance to make it dynamic.  */
  if (strip
      && h->dynindx == -1
      && type != STT_GNU_IFUNC
      && !h->forced_local)
    return true;

  sym.st_value = 0;
  sym.st_size = h->size;
  sym.st_other = h->other;
  switch (h->root.type)
    {
    default:
    case bfd_link_hash_new:
    case bfd_link_hash_warning:
      abort ();
      return false;

    case bfd_link_hash_undefined:
    case bfd_link_hash_undefweak:
      input_sec = bfd_und_section_ptr;
      sym.st_shndx = SHN_UNDEF;
      break;

    case bfd_link_hash_defined:
    case bfd_link_hash_defweak:
      {
	input_sec = h->root.u.def.section;
	if (input_sec->output_section != NULL)
	  {
	    sym.st_shndx =
	      _bfd_elf_section_from_bfd_section (flinfo->output_bfd,
						 input_sec->output_section);
	    if (sym.st_shndx == SHN_BAD)
	      {
		_bfd_error_handler
		  /* xgettext:c-format */
		  (_("%pB: could not find output section %pA for input section %pA"),
		   flinfo->output_bfd, input_sec->output_section, input_sec);
		bfd_set_error (bfd_error_nonrepresentable_section);
		eoinfo->failed = true;
		return false;
	      }

	    /* ELF symbols in relocatable files are section relative,
	       but in nonrelocatable files they are virtual
	       addresses.  */
	    sym.st_value = h->root.u.def.value + input_sec->output_offset;
	    if (!bfd_link_relocatable (flinfo->info))
	      {
		sym.st_value += input_sec->output_section->vma;
		if (h->type == STT_TLS)
		  {
		    asection *tls_sec = elf_hash_table (flinfo->info)->tls_sec;
		    if (tls_sec != NULL)
		      sym.st_value -= tls_sec->vma;
		  }
	      }
	  }
	else
	  {
	    BFD_ASSERT (input_sec->owner == NULL
			|| (input_sec->owner->flags & DYNAMIC) != 0);
	    sym.st_shndx = SHN_UNDEF;
	    input_sec = bfd_und_section_ptr;
	  }
      }
      break;

    case bfd_link_hash_common:
      input_sec = h->root.u.c.p->section;
      sym.st_shndx = bed->common_section_index (input_sec);
      sym.st_value = 1 << h->root.u.c.p->alignment_power;
      break;

    case bfd_link_hash_indirect:
      /* These symbols are created by symbol versioning.  They point
	 to the decorated version of the name.  For example, if the
	 symbol foo@@GNU_1.2 is the default, which should be used when
	 foo is used with no version, then we add an indirect symbol
	 foo which points to foo@@GNU_1.2.  We ignore these symbols,
	 since the indirected symbol is already in the hash table.  */
      return true;
    }

  if (type == STT_COMMON || type == STT_OBJECT)
    switch (h->root.type)
      {
      case bfd_link_hash_common:
	type = elf_link_convert_common_type (flinfo->info, type);
	break;
      case bfd_link_hash_defined:
      case bfd_link_hash_defweak:
	if (bed->common_definition (&sym))
	  type = elf_link_convert_common_type (flinfo->info, type);
	else
	  type = STT_OBJECT;
	break;
      case bfd_link_hash_undefined:
      case bfd_link_hash_undefweak:
	break;
      default:
	abort ();
      }

  if (h->forced_local)
    {
      sym.st_info = ELF_ST_INFO (STB_LOCAL, type);
      /* Turn off visibility on local symbol.  */
      sym.st_other &= ~ELF_ST_VISIBILITY (-1);
    }
  /* Set STB_GNU_UNIQUE only if symbol is defined in regular object.  */
  else if (h->unique_global && h->def_regular)
    sym.st_info = ELF_ST_INFO (STB_GNU_UNIQUE, type);
  else if (h->root.type == bfd_link_hash_undefweak
	   || h->root.type == bfd_link_hash_defweak)
    sym.st_info = ELF_ST_INFO (STB_WEAK, type);
  else
    sym.st_info = ELF_ST_INFO (STB_GLOBAL, type);
  sym.st_target_internal = h->target_internal;

  /* Give the processor backend a chance to tweak the symbol value,
     and also to finish up anything that needs to be done for this
     symbol.  FIXME: Not calling elf_backend_finish_dynamic_symbol for
     forced local syms when non-shared is due to a historical quirk.
     STT_GNU_IFUNC symbol must go through PLT.  */
  if ((h->type == STT_GNU_IFUNC
       && h->def_regular
       && !bfd_link_relocatable (flinfo->info))
      || ((h->dynindx != -1
	   || h->forced_local)
	  && ((bfd_link_pic (flinfo->info)
	       && (ELF_ST_VISIBILITY (h->other) == STV_DEFAULT
		   || h->root.type != bfd_link_hash_undefweak))
	      || !h->forced_local)
	  && elf_hash_table (flinfo->info)->dynamic_sections_created))
    {
      if (! ((*bed->elf_backend_finish_dynamic_symbol)
	     (flinfo->output_bfd, flinfo->info, h, &sym)))
	{
	  eoinfo->failed = true;
	  return false;
	}
    }

  /* If we are marking the symbol as undefined, and there are no
     non-weak references to this symbol from a regular object, then
     mark the symbol as weak undefined; if there are non-weak
     references, mark the symbol as strong.  We can't do this earlier,
     because it might not be marked as undefined until the
     finish_dynamic_symbol routine gets through with it.  */
  if (sym.st_shndx == SHN_UNDEF
      && h->ref_regular
      && (ELF_ST_BIND (sym.st_info) == STB_GLOBAL
	  || ELF_ST_BIND (sym.st_info) == STB_WEAK))
    {
      int bindtype;
      type = ELF_ST_TYPE (sym.st_info);

      /* Turn an undefined IFUNC symbol into a normal FUNC symbol. */
      if (type == STT_GNU_IFUNC)
	type = STT_FUNC;

      if (h->ref_regular_nonweak)
	bindtype = STB_GLOBAL;
      else
	bindtype = STB_WEAK;
      sym.st_info = ELF_ST_INFO (bindtype, type);
    }

  /* If this is a symbol defined in a dynamic library, don't use the
     symbol size from the dynamic library.  Relinking an executable
     against a new library may introduce gratuitous changes in the
     executable's symbols if we keep the size.  */
  if (sym.st_shndx == SHN_UNDEF
      && !h->def_regular
      && h->def_dynamic)
    sym.st_size = 0;

  /* If a non-weak symbol with non-default visibility is not defined
     locally, it is a fatal error.  */
  if (!bfd_link_relocatable (flinfo->info)
      && ELF_ST_VISIBILITY (sym.st_other) != STV_DEFAULT
      && ELF_ST_BIND (sym.st_info) != STB_WEAK
      && h->root.type == bfd_link_hash_undefined
      && !h->def_regular)
    {
      const char *msg;

      if (ELF_ST_VISIBILITY (sym.st_other) == STV_PROTECTED)
	/* xgettext:c-format */
	msg = _("%pB: protected symbol `%s' isn't defined");
      else if (ELF_ST_VISIBILITY (sym.st_other) == STV_INTERNAL)
	/* xgettext:c-format */
	msg = _("%pB: internal symbol `%s' isn't defined");
      else
	/* xgettext:c-format */
	msg = _("%pB: hidden symbol `%s' isn't defined");
      _bfd_error_handler (msg, flinfo->output_bfd, h->root.root.string);
      bfd_set_error (bfd_error_bad_value);
      eoinfo->failed = true;
      return false;
    }

  /* If this symbol should be put in the .dynsym section, then put it
     there now.  We already know the symbol index.  We also fill in
     the entry in the .hash section.  */
  if (h->dynindx != -1
      && elf_hash_table (flinfo->info)->dynamic_sections_created
      && elf_hash_table (flinfo->info)->dynsym != NULL
      && !discarded_section (elf_hash_table (flinfo->info)->dynsym))
    {
      bfd_byte *esym;

      /* Since there is no version information in the dynamic string,
	 if there is no version info in symbol version section, we will
	 have a run-time problem if not linking executable, referenced
	 by shared library, or not bound locally.  */
      if (h->verinfo.verdef == NULL
	  && (!bfd_link_executable (flinfo->info)
	      || h->ref_dynamic
	      || !h->def_regular))
	{
	  char *p = strrchr (h->root.root.string, ELF_VER_CHR);

	  if (p && p [1] != '\0')
	    {
	      _bfd_error_handler
		/* xgettext:c-format */
		(_("%pB: no symbol version section for versioned symbol `%s'"),
		 flinfo->output_bfd, h->root.root.string);
	      eoinfo->failed = true;
	      return false;
	    }
	}

      sym.st_name = h->dynstr_index;
      esym = (elf_hash_table (flinfo->info)->dynsym->contents
	      + h->dynindx * bed->s->sizeof_sym);
      if (!check_dynsym (flinfo->output_bfd, &sym))
	{
	  eoinfo->failed = true;
	  return false;
	}

      /* Inform the linker of the addition of this symbol.  */

      if (flinfo->info->callbacks->ctf_new_dynsym)
	flinfo->info->callbacks->ctf_new_dynsym (h->dynindx, &sym);

      bed->s->swap_symbol_out (flinfo->output_bfd, &sym, esym, 0);

      if (flinfo->hash_sec != NULL)
	{
	  size_t hash_entry_size;
	  bfd_byte *bucketpos;
	  bfd_vma chain;
	  size_t bucketcount;
	  size_t bucket;

	  bucketcount = elf_hash_table (flinfo->info)->bucketcount;
	  bucket = h->u.elf_hash_value % bucketcount;

	  hash_entry_size
	    = elf_section_data (flinfo->hash_sec)->this_hdr.sh_entsize;
	  bucketpos = ((bfd_byte *) flinfo->hash_sec->contents
		       + (bucket + 2) * hash_entry_size);
	  chain = bfd_get (8 * hash_entry_size, flinfo->output_bfd, bucketpos);
	  bfd_put (8 * hash_entry_size, flinfo->output_bfd, h->dynindx,
		   bucketpos);
	  bfd_put (8 * hash_entry_size, flinfo->output_bfd, chain,
		   ((bfd_byte *) flinfo->hash_sec->contents
		    + (bucketcount + 2 + h->dynindx) * hash_entry_size));
	}

      if (flinfo->symver_sec != NULL && flinfo->symver_sec->contents != NULL)
	{
	  Elf_Internal_Versym iversym;
	  Elf_External_Versym *eversym;

	  if (!h->def_regular && !ELF_COMMON_DEF_P (h))
	    {
	      if (h->verinfo.verdef == NULL
		  || (elf_dyn_lib_class (h->verinfo.verdef->vd_bfd)
		      & (DYN_AS_NEEDED | DYN_DT_NEEDED | DYN_NO_NEEDED)))
		iversym.vs_vers = 1;
	      else
		iversym.vs_vers = h->verinfo.verdef->vd_exp_refno + 1;
	    }
	  else
	    {
	      if (h->verinfo.vertree == NULL)
		iversym.vs_vers = 1;
	      else
		iversym.vs_vers = h->verinfo.vertree->vernum + 1;
	      if (flinfo->info->create_default_symver)
		iversym.vs_vers++;
	    }

	  /* Turn on VERSYM_HIDDEN only if the hidden versioned symbol is
	     defined locally.  */
	  if (h->versioned == versioned_hidden && h->def_regular)
	    iversym.vs_vers |= VERSYM_HIDDEN;

	  eversym = (Elf_External_Versym *) flinfo->symver_sec->contents;
	  eversym += h->dynindx;
	  _bfd_elf_swap_versym_out (flinfo->output_bfd, &iversym, eversym);
	}
    }

  /* If the symbol is undefined, and we didn't output it to .dynsym,
     strip it from .symtab too.  Obviously we can't do this for
     relocatable output or when needed for --emit-relocs.  */
  else if (input_sec == bfd_und_section_ptr
	   && h->indx != -2
	   /* PR 22319 Do not strip global undefined symbols marked as being needed.  */
	   && (h->mark != 1 || ELF_ST_BIND (sym.st_info) != STB_GLOBAL)
	   && !bfd_link_relocatable (flinfo->info))
    return true;

  /* Also strip others that we couldn't earlier due to dynamic symbol
     processing.  */
  if (strip)
    return true;
  if ((input_sec->flags & SEC_EXCLUDE) != 0)
    return true;

  /* Output a FILE symbol so that following locals are not associated
     with the wrong input file.  We need one for forced local symbols
     if we've seen more than one FILE symbol or when we have exactly
     one FILE symbol but global symbols are present in a file other
     than the one with the FILE symbol.  We also need one if linker
     defined symbols are present.  In practice these conditions are
     always met, so just emit the FILE symbol unconditionally.  */
  if (eoinfo->localsyms
      && !eoinfo->file_sym_done
      && eoinfo->flinfo->filesym_count != 0)
    {
      Elf_Internal_Sym fsym;

      memset (&fsym, 0, sizeof (fsym));
      fsym.st_info = ELF_ST_INFO (STB_LOCAL, STT_FILE);
      fsym.st_shndx = SHN_ABS;
      if (!elf_link_output_symstrtab (eoinfo->flinfo, NULL, &fsym,
				      bfd_und_section_ptr, NULL))
	return false;

      eoinfo->file_sym_done = true;
    }

  indx = bfd_get_symcount (flinfo->output_bfd);
  ret = elf_link_output_symstrtab (flinfo, h->root.root.string, &sym,
				   input_sec, h);
  if (ret == 0)
    {
      eoinfo->failed = true;
      return false;
    }
  else if (ret == 1)
    h->indx = indx;
  else if (h->indx == -2)
    abort();

  return true;
}

/* Return TRUE if special handling is done for relocs in SEC against
   symbols defined in discarded sections.  */

static bool
elf_section_ignore_discarded_relocs (asection *sec)
{
  const struct elf_backend_data *bed;

  switch (sec->sec_info_type)
    {
    case SEC_INFO_TYPE_STABS:
    case SEC_INFO_TYPE_EH_FRAME:
    case SEC_INFO_TYPE_EH_FRAME_ENTRY:
    case SEC_INFO_TYPE_SFRAME:
      return true;
    default:
      break;
    }

  bed = get_elf_backend_data (sec->owner);
  if (bed->elf_backend_ignore_discarded_relocs != NULL
      && (*bed->elf_backend_ignore_discarded_relocs) (sec))
    return true;

  return false;
}

/* Return a mask saying how ld should treat relocations in SEC against
   symbols defined in discarded sections.  If this function returns
   COMPLAIN set, ld will issue a warning message.  If this function
   returns PRETEND set, and the discarded section was link-once and the
   same size as the kept link-once section, ld will pretend that the
   symbol was actually defined in the kept section.  Otherwise ld will
   zero the reloc (at least that is the intent, but some cooperation by
   the target dependent code is needed, particularly for REL targets).  */

unsigned int
_bfd_elf_default_action_discarded (asection *sec)
{
  const struct elf_backend_data *bed;
  bed = get_elf_backend_data (sec->owner);

  if (sec->flags & SEC_DEBUGGING)
    return PRETEND;

  if (strcmp (".eh_frame", sec->name) == 0)
    return 0;

  if (bed->elf_backend_can_make_multiple_eh_frame
      && strncmp (sec->name, ".eh_frame.", 10) == 0)
    return 0;

  if (strcmp (".sframe", sec->name) == 0)
    return 0;

  if (strcmp (".gcc_except_table", sec->name) == 0)
    return 0;

  return COMPLAIN | PRETEND;
}

/* Find a match between a section and a member of a section group.  */

static asection *
match_group_member (asection *sec, asection *group,
		    struct bfd_link_info *info)
{
  asection *first = elf_next_in_group (group);
  asection *s = first;

  while (s != NULL)
    {
      if (bfd_elf_match_symbols_in_sections (s, sec, info))
	return s;

      s = elf_next_in_group (s);
      if (s == first)
	break;
    }

  return NULL;
}

/* Check if the kept section of a discarded section SEC can be used
   to replace it.  Return the replacement if it is OK.  Otherwise return
   NULL.  */

asection *
_bfd_elf_check_kept_section (asection *sec, struct bfd_link_info *info)
{
  asection *kept;

  kept = sec->kept_section;
  if (kept != NULL)
    {
      if ((kept->flags & SEC_GROUP) != 0)
	kept = match_group_member (sec, kept, info);
      if (kept != NULL)
	{
	  if ((sec->rawsize != 0 ? sec->rawsize : sec->size)
	      != (kept->rawsize != 0 ? kept->rawsize : kept->size))
	    kept = NULL;
	  else
	    {
	      /* Get the real kept section.  */
	      asection *next;
	      for (next = kept->kept_section;
		   next != NULL;
		   next = next->kept_section)
		kept = next;
	    }
	}
      sec->kept_section = kept;
    }
  return kept;
}

/* Link an input file into the linker output file.  This function
   handles all the sections and relocations of the input file at once.
   This is so that we only have to read the local symbols once, and
   don't have to keep them in memory.  */

static bool
elf_link_input_bfd (struct elf_final_link_info *flinfo, bfd *input_bfd)
{
  int (*relocate_section)
    (bfd *, struct bfd_link_info *, bfd *, asection *, bfd_byte *,
     Elf_Internal_Rela *, Elf_Internal_Sym *, asection **);
  bfd *output_bfd;
  Elf_Internal_Shdr *symtab_hdr;
  size_t locsymcount;
  size_t extsymoff;
  Elf_Internal_Sym *isymbuf;
  Elf_Internal_Sym *isym;
  Elf_Internal_Sym *isymend;
  long *pindex;
  asection **ppsection;
  asection *o;
  const struct elf_backend_data *bed;
  struct elf_link_hash_entry **sym_hashes;
  bfd_size_type address_size;
  bfd_vma r_type_mask;
  int r_sym_shift;
  bool have_file_sym = false;

  output_bfd = flinfo->output_bfd;
  bed = get_elf_backend_data (output_bfd);
  relocate_section = bed->elf_backend_relocate_section;

  /* If this is a dynamic object, we don't want to do anything here:
     we don't want the local symbols, and we don't want the section
     contents.  */
  if ((input_bfd->flags & DYNAMIC) != 0)
    return true;

  symtab_hdr = &elf_tdata (input_bfd)->symtab_hdr;
  if (elf_bad_symtab (input_bfd))
    {
      locsymcount = symtab_hdr->sh_size / bed->s->sizeof_sym;
      extsymoff = 0;
    }
  else
    {
      locsymcount = symtab_hdr->sh_info;
      extsymoff = symtab_hdr->sh_info;
    }

  /* Enable GNU OSABI features in the output BFD that are used in the input
     BFD.  */
  if (bed->elf_osabi == ELFOSABI_NONE
      || bed->elf_osabi == ELFOSABI_GNU
      || bed->elf_osabi == ELFOSABI_FREEBSD)
    elf_tdata (output_bfd)->has_gnu_osabi
      |= (elf_tdata (input_bfd)->has_gnu_osabi
	  & (bfd_link_relocatable (flinfo->info)
	     ? -1 : ~elf_gnu_osabi_retain));

  /* Read the local symbols.  */
  isymbuf = (Elf_Internal_Sym *) symtab_hdr->contents;
  if (isymbuf == NULL && locsymcount != 0)
    {
      isymbuf = bfd_elf_get_elf_syms (input_bfd, symtab_hdr, locsymcount, 0,
				      flinfo->internal_syms,
				      flinfo->external_syms,
				      flinfo->locsym_shndx);
      if (isymbuf == NULL)
	return false;
    }

  /* Find local symbol sections and adjust values of symbols in
     SEC_MERGE sections.  Write out those local symbols we know are
     going into the output file.  */
  isymend = PTR_ADD (isymbuf, locsymcount);
  for (isym = isymbuf, pindex = flinfo->indices, ppsection = flinfo->sections;
       isym < isymend;
       isym++, pindex++, ppsection++)
    {
      asection *isec;
      const char *name;
      Elf_Internal_Sym osym;
      long indx;
      int ret;

      *pindex = -1;

      if (elf_bad_symtab (input_bfd))
	{
	  if (ELF_ST_BIND (isym->st_info) != STB_LOCAL)
	    {
	      *ppsection = NULL;
	      continue;
	    }
	}

      if (isym->st_shndx == SHN_UNDEF)
	isec = bfd_und_section_ptr;
      else if (isym->st_shndx == SHN_ABS)
	isec = bfd_abs_section_ptr;
      else if (isym->st_shndx == SHN_COMMON)
	isec = bfd_com_section_ptr;
      else
	{
	  isec = bfd_section_from_elf_index (input_bfd, isym->st_shndx);
	  if (isec == NULL)
	    {
	      /* Don't attempt to output symbols with st_shnx in the
		 reserved range other than SHN_ABS and SHN_COMMON.  */
	      isec = bfd_und_section_ptr;
	    }
	  else if (isec->sec_info_type == SEC_INFO_TYPE_MERGE
		   && ELF_ST_TYPE (isym->st_info) != STT_SECTION)
	    isym->st_value =
	      _bfd_merged_section_offset (output_bfd, &isec,
					  elf_section_data (isec)->sec_info,
					  isym->st_value);
	}

      *ppsection = isec;

      /* Don't output the first, undefined, symbol.  In fact, don't
	 output any undefined local symbol.  */
      if (isec == bfd_und_section_ptr)
	continue;

      if (ELF_ST_TYPE (isym->st_info) == STT_SECTION)
	{
	  /* We never output section symbols.  Instead, we use the
	     section symbol of the corresponding section in the output
	     file.  */
	  continue;
	}

      /* If we are stripping all symbols, we don't want to output this
	 one.  */
      if (flinfo->info->strip == strip_all)
	continue;

      /* If we are discarding all local symbols, we don't want to
	 output this one.  If we are generating a relocatable output
	 file, then some of the local symbols may be required by
	 relocs; we output them below as we discover that they are
	 needed.  */
      if (flinfo->info->discard == discard_all)
	continue;

      /* If this symbol is defined in a section which we are
	 discarding, we don't need to keep it.  */
      if (isym->st_shndx < SHN_LORESERVE
	  && (isec->output_section == NULL
	      || bfd_section_removed_from_list (output_bfd,
						isec->output_section)))
	continue;

      /* Get the name of the symbol.  */
      name = bfd_elf_string_from_elf_section (input_bfd, symtab_hdr->sh_link,
					      isym->st_name);
      if (name == NULL)
	return false;

      /* See if we are discarding symbols with this name.  */
      if ((flinfo->info->strip == strip_some
	   && (bfd_hash_lookup (flinfo->info->keep_hash, name, false, false)
	       == NULL))
	  || (((flinfo->info->discard == discard_sec_merge
		&& (isec->flags & SEC_MERGE)
		&& !bfd_link_relocatable (flinfo->info))
	       || flinfo->info->discard == discard_l)
	      && bfd_is_local_label_name (input_bfd, name)))
	continue;

      if (ELF_ST_TYPE (isym->st_info) == STT_FILE)
	{
	  if (input_bfd->lto_output)
	    /* -flto puts a temp file name here.  This means builds
	       are not reproducible.  Discard the symbol.  */
	    continue;
	  have_file_sym = true;
	  flinfo->filesym_count += 1;
	}
      if (!have_file_sym)
	{
	  /* In the absence of debug info, bfd_find_nearest_line uses
	     FILE symbols to determine the source file for local
	     function symbols.  Provide a FILE symbol here if input
	     files lack such, so that their symbols won't be
	     associated with a previous input file.  It's not the
	     source file, but the best we can do.  */
	  const char *filename;
	  have_file_sym = true;
	  flinfo->filesym_count += 1;
	  memset (&osym, 0, sizeof (osym));
	  osym.st_info = ELF_ST_INFO (STB_LOCAL, STT_FILE);
	  osym.st_shndx = SHN_ABS;
	  if (input_bfd->lto_output)
	    filename = NULL;
	  else
	    filename = lbasename (bfd_get_filename (input_bfd));
	  if (!elf_link_output_symstrtab (flinfo, filename, &osym,
					  bfd_abs_section_ptr, NULL))
	    return false;
	}

      osym = *isym;

      /* Adjust the section index for the output file.  */
      osym.st_shndx = _bfd_elf_section_from_bfd_section (output_bfd,
							 isec->output_section);
      if (osym.st_shndx == SHN_BAD)
	return false;

      /* ELF symbols in relocatable files are section relative, but
	 in executable files they are virtual addresses.  Note that
	 this code assumes that all ELF sections have an associated
	 BFD section with a reasonable value for output_offset; below
	 we assume that they also have a reasonable value for
	 output_section.  Any special sections must be set up to meet
	 these requirements.  */
      osym.st_value += isec->output_offset;
      if (!bfd_link_relocatable (flinfo->info))
	{
	  osym.st_value += isec->output_section->vma;
	  if (ELF_ST_TYPE (osym.st_info) == STT_TLS)
	    {
	      /* STT_TLS symbols are relative to PT_TLS segment base.  */
	      if (elf_hash_table (flinfo->info)->tls_sec != NULL)
		osym.st_value -= elf_hash_table (flinfo->info)->tls_sec->vma;
	      else
		osym.st_info = ELF_ST_INFO (ELF_ST_BIND (osym.st_info),
					    STT_NOTYPE);
	    }
	}

      indx = bfd_get_symcount (output_bfd);
      ret = elf_link_output_symstrtab (flinfo, name, &osym, isec, NULL);
      if (ret == 0)
	return false;
      else if (ret == 1)
	*pindex = indx;
    }

  if (bed->s->arch_size == 32)
    {
      r_type_mask = 0xff;
      r_sym_shift = 8;
      address_size = 4;
    }
  else
    {
      r_type_mask = 0xffffffff;
      r_sym_shift = 32;
      address_size = 8;
    }

  /* Relocate the contents of each section.  */
  sym_hashes = elf_sym_hashes (input_bfd);
  for (o = input_bfd->sections; o != NULL; o = o->next)
    {
      bfd_byte *contents;

      if (! o->linker_mark)
	{
	  /* This section was omitted from the link.  */
	  continue;
	}

      if (!flinfo->info->resolve_section_groups
	  && (o->flags & (SEC_LINKER_CREATED | SEC_GROUP)) == SEC_GROUP)
	{
	  /* Deal with the group signature symbol.  */
	  struct bfd_elf_section_data *sec_data = elf_section_data (o);
	  unsigned long symndx = sec_data->this_hdr.sh_info;
	  asection *osec = o->output_section;

	  BFD_ASSERT (bfd_link_relocatable (flinfo->info));
	  if (symndx >= locsymcount
	      || (elf_bad_symtab (input_bfd)
		  && flinfo->sections[symndx] == NULL))
	    {
	      struct elf_link_hash_entry *h = sym_hashes[symndx - extsymoff];
	      while (h->root.type == bfd_link_hash_indirect
		     || h->root.type == bfd_link_hash_warning)
		h = (struct elf_link_hash_entry *) h->root.u.i.link;
	      /* Arrange for symbol to be output.  */
	      h->indx = -2;
	      elf_section_data (osec)->this_hdr.sh_info = -2;
	    }
	  else if (ELF_ST_TYPE (isymbuf[symndx].st_info) == STT_SECTION)
	    {
	      /* We'll use the output section target_index.  */
	      asection *sec = flinfo->sections[symndx]->output_section;
	      elf_section_data (osec)->this_hdr.sh_info = sec->target_index;
	    }
	  else
	    {
	      if (flinfo->indices[symndx] == -1)
		{
		  /* Otherwise output the local symbol now.  */
		  Elf_Internal_Sym sym = isymbuf[symndx];
		  asection *sec = flinfo->sections[symndx]->output_section;
		  const char *name;
		  long indx;
		  int ret;

		  name = bfd_elf_string_from_elf_section (input_bfd,
							  symtab_hdr->sh_link,
							  sym.st_name);
		  if (name == NULL)
		    return false;

		  sym.st_shndx = _bfd_elf_section_from_bfd_section (output_bfd,
								    sec);
		  if (sym.st_shndx == SHN_BAD)
		    return false;

		  sym.st_value += o->output_offset;

		  indx = bfd_get_symcount (output_bfd);
		  ret = elf_link_output_symstrtab (flinfo, name, &sym, o,
						   NULL);
		  if (ret == 0)
		    return false;
		  else if (ret == 1)
		    flinfo->indices[symndx] = indx;
		  else
		    abort ();
		}
	      elf_section_data (osec)->this_hdr.sh_info
		= flinfo->indices[symndx];
	    }
	}

      if ((o->flags & SEC_HAS_CONTENTS) == 0
	  || (o->size == 0 && (o->flags & SEC_RELOC) == 0))
	continue;

      if ((o->flags & SEC_LINKER_CREATED) != 0)
	{
	  /* Section was created by _bfd_elf_link_create_dynamic_sections
	     or somesuch.  */
	  continue;
	}

      /* Get the contents of the section.  They have been cached by a
	 relaxation routine.  Note that o is a section in an input
	 file, so the contents field will not have been set by any of
	 the routines which work on output files.  */
      if (elf_section_data (o)->this_hdr.contents != NULL)
	{
	  contents = elf_section_data (o)->this_hdr.contents;
	  if (bed->caches_rawsize
	      && o->rawsize != 0
	      && o->rawsize < o->size)
	    {
	      memcpy (flinfo->contents, contents, o->rawsize);
	      contents = flinfo->contents;
	    }
	}
      else if (!(o->flags & SEC_RELOC)
	       && !bed->elf_backend_write_section
	       && o->sec_info_type == SEC_INFO_TYPE_MERGE)
	/* A MERGE section that has no relocations doesn't need the
	   contents anymore, they have been recorded earlier.  Except
	   if the backend has special provisions for writing sections.  */
	contents = NULL;
      else
	{
	  contents = flinfo->contents;
	  if (! bfd_get_full_section_contents (input_bfd, o, &contents))
	    return false;
	}

      if ((o->flags & SEC_RELOC) != 0)
	{
	  Elf_Internal_Rela *internal_relocs;
	  Elf_Internal_Rela *rel, *relend;
	  int action_discarded;
	  int ret;

	  /* Get the swapped relocs.  */
	  internal_relocs
	    = _bfd_elf_link_info_read_relocs (input_bfd, flinfo->info, o,
					      flinfo->external_relocs,
					      flinfo->internal_relocs,
					      false);
	  if (internal_relocs == NULL
	      && o->reloc_count > 0)
	    return false;

	  action_discarded = -1;
	  if (!elf_section_ignore_discarded_relocs (o))
	    action_discarded = (*bed->action_discarded) (o);

	  /* Run through the relocs evaluating complex reloc symbols and
	     looking for relocs against symbols from discarded sections
	     or section symbols from removed link-once sections.
	     Complain about relocs against discarded sections.  Zero
	     relocs against removed link-once sections.  */

	  rel = internal_relocs;
	  relend = rel + o->reloc_count;
	  for ( ; rel < relend; rel++)
	    {
	      unsigned long r_symndx = rel->r_info >> r_sym_shift;
	      unsigned int s_type;
	      asection **ps, *sec;
	      struct elf_link_hash_entry *h = NULL;
	      const char *sym_name;

	      if (r_symndx == STN_UNDEF)
		continue;

	      if (r_symndx >= locsymcount
		  || (elf_bad_symtab (input_bfd)
		      && flinfo->sections[r_symndx] == NULL))
		{
		  h = sym_hashes[r_symndx - extsymoff];

		  /* Badly formatted input files can contain relocs that
		     reference non-existant symbols.  Check here so that
		     we do not seg fault.  */
		  if (h == NULL)
		    {
		      _bfd_error_handler
			/* xgettext:c-format */
			(_("error: %pB contains a reloc (%#" PRIx64 ") for section %pA "
			   "that references a non-existent global symbol"),
			 input_bfd, (uint64_t) rel->r_info, o);
		      bfd_set_error (bfd_error_bad_value);
		      return false;
		    }

		  while (h->root.type == bfd_link_hash_indirect
			 || h->root.type == bfd_link_hash_warning)
		    h = (struct elf_link_hash_entry *) h->root.u.i.link;

		  s_type = h->type;

		  /* If a plugin symbol is referenced from a non-IR file,
		     mark the symbol as undefined.  Note that the
		     linker may attach linker created dynamic sections
		     to the plugin bfd.  Symbols defined in linker
		     created sections are not plugin symbols.  */
		  if ((h->root.non_ir_ref_regular
		       || h->root.non_ir_ref_dynamic)
		      && (h->root.type == bfd_link_hash_defined
			  || h->root.type == bfd_link_hash_defweak)
		      && (h->root.u.def.section->flags
			  & SEC_LINKER_CREATED) == 0
		      && h->root.u.def.section->owner != NULL
		      && (h->root.u.def.section->owner->flags
			  & BFD_PLUGIN) != 0)
		    {
		      h->root.type = bfd_link_hash_undefined;
		      h->root.u.undef.abfd = h->root.u.def.section->owner;
		    }

		  ps = NULL;
		  if (h->root.type == bfd_link_hash_defined
		      || h->root.type == bfd_link_hash_defweak)
		    ps = &h->root.u.def.section;

		  sym_name = h->root.root.string;
		}
	      else
		{
		  Elf_Internal_Sym *sym = isymbuf + r_symndx;

		  s_type = ELF_ST_TYPE (sym->st_info);
		  ps = &flinfo->sections[r_symndx];
		  sym_name = bfd_elf_sym_name (input_bfd, symtab_hdr,
					       sym, *ps);
		}

	      if ((s_type == STT_RELC || s_type == STT_SRELC)
		  && !bfd_link_relocatable (flinfo->info))
		{
		  bfd_vma val;
		  bfd_vma dot = (rel->r_offset
				 + o->output_offset + o->output_section->vma);
#ifdef DEBUG
		  printf ("Encountered a complex symbol!");
		  printf (" (input_bfd %s, section %s, reloc %ld\n",
			  bfd_get_filename (input_bfd), o->name,
			  (long) (rel - internal_relocs));
		  printf (" symbol: idx  %8.8lx, name %s\n",
			  r_symndx, sym_name);
		  printf (" reloc : info %8.8lx, addr %8.8lx\n",
			  (unsigned long) rel->r_info,
			  (unsigned long) rel->r_offset);
#endif
		  if (!eval_symbol (&val, &sym_name, input_bfd, flinfo, dot,
				    isymbuf, locsymcount, s_type == STT_SRELC))
		    return false;

		  /* Symbol evaluated OK.  Update to absolute value.  */
		  set_symbol_value (input_bfd, isymbuf, locsymcount,
				    r_symndx, val);
		  continue;
		}

	      if (action_discarded != -1 && ps != NULL)
		{
		  /* Complain if the definition comes from a
		     discarded section.  */
		  if ((sec = *ps) != NULL && discarded_section (sec))
		    {
		      BFD_ASSERT (r_symndx != STN_UNDEF);
		      if (action_discarded & COMPLAIN)
			(*flinfo->info->callbacks->einfo)
			  /* xgettext:c-format */
			  (_("%X`%s' referenced in section `%pA' of %pB: "
			     "defined in discarded section `%pA' of %pB\n"),
			   sym_name, o, input_bfd, sec, sec->owner);

		      /* Try to do the best we can to support buggy old
			 versions of gcc.  Pretend that the symbol is
			 really defined in the kept linkonce section.
			 FIXME: This is quite broken.  Modifying the
			 symbol here means we will be changing all later
			 uses of the symbol, not just in this section.  */
		      if (action_discarded & PRETEND)
			{
			  asection *kept;

			  kept = _bfd_elf_check_kept_section (sec,
							      flinfo->info);
			  if (kept != NULL)
			    {
			      *ps = kept;
			      continue;
			    }
			}
		    }
		}
	    }

	  /* Relocate the section by invoking a back end routine.

	     The back end routine is responsible for adjusting the
	     section contents as necessary, and (if using Rela relocs
	     and generating a relocatable output file) adjusting the
	     reloc addend as necessary.

	     The back end routine does not have to worry about setting
	     the reloc address or the reloc symbol index.

	     The back end routine is given a pointer to the swapped in
	     internal symbols, and can access the hash table entries
	     for the external symbols via elf_sym_hashes (input_bfd).

	     When generating relocatable output, the back end routine
	     must handle STB_LOCAL/STT_SECTION symbols specially.  The
	     output symbol is going to be a section symbol
	     corresponding to the output section, which will require
	     the addend to be adjusted.  */

	  ret = (*relocate_section) (output_bfd, flinfo->info,
				     input_bfd, o, contents,
				     internal_relocs,
				     isymbuf,
				     flinfo->sections);
	  if (!ret)
	    return false;

	  if (ret == 2
	      || bfd_link_relocatable (flinfo->info)
	      || flinfo->info->emitrelocations)
	    {
	      Elf_Internal_Rela *irela;
	      Elf_Internal_Rela *irelaend, *irelamid;
	      bfd_vma last_offset;
	      struct elf_link_hash_entry **rel_hash;
	      struct elf_link_hash_entry **rel_hash_list, **rela_hash_list;
	      Elf_Internal_Shdr *input_rel_hdr, *input_rela_hdr;
	      unsigned int next_erel;
	      bool rela_normal;
	      struct bfd_elf_section_data *esdi, *esdo;

	      esdi = elf_section_data (o);
	      esdo = elf_section_data (o->output_section);
	      rela_normal = false;

	      /* Adjust the reloc addresses and symbol indices.  */

	      irela = internal_relocs;
	      irelaend = irela + o->reloc_count;
	      rel_hash = PTR_ADD (esdo->rel.hashes, esdo->rel.count);
	      /* We start processing the REL relocs, if any.  When we reach
		 IRELAMID in the loop, we switch to the RELA relocs.  */
	      irelamid = irela;
	      if (esdi->rel.hdr != NULL)
		irelamid += (NUM_SHDR_ENTRIES (esdi->rel.hdr)
			     * bed->s->int_rels_per_ext_rel);
	      rel_hash_list = rel_hash;
	      rela_hash_list = NULL;
	      last_offset = o->output_offset;
	      if (!bfd_link_relocatable (flinfo->info))
		last_offset += o->output_section->vma;
	      for (next_erel = 0; irela < irelaend; irela++, next_erel++)
		{
		  unsigned long r_symndx;
		  asection *sec;
		  Elf_Internal_Sym sym;

		  if (next_erel == bed->s->int_rels_per_ext_rel)
		    {
		      rel_hash++;
		      next_erel = 0;
		    }

		  if (irela == irelamid)
		    {
		      rel_hash = PTR_ADD (esdo->rela.hashes, esdo->rela.count);
		      rela_hash_list = rel_hash;
		      rela_normal = bed->rela_normal;
		    }

		  irela->r_offset = _bfd_elf_section_offset (output_bfd,
							     flinfo->info, o,
							     irela->r_offset);
		  if (irela->r_offset >= (bfd_vma) -2)
		    {
		      /* This is a reloc for a deleted entry or somesuch.
			 Turn it into an R_*_NONE reloc, at the same
			 offset as the last reloc.  elf_eh_frame.c and
			 bfd_elf_discard_info rely on reloc offsets
			 being ordered.  */
		      irela->r_offset = last_offset;
		      irela->r_info = 0;
		      irela->r_addend = 0;
		      continue;
		    }

		  irela->r_offset += o->output_offset;

		  /* Relocs in an executable have to be virtual addresses.  */
		  if (!bfd_link_relocatable (flinfo->info))
		    irela->r_offset += o->output_section->vma;

		  last_offset = irela->r_offset;

		  r_symndx = irela->r_info >> r_sym_shift;
		  if (r_symndx == STN_UNDEF)
		    continue;

		  if (r_symndx >= locsymcount
		      || (elf_bad_symtab (input_bfd)
			  && flinfo->sections[r_symndx] == NULL))
		    {
		      struct elf_link_hash_entry *rh;
		      unsigned long indx;

		      /* This is a reloc against a global symbol.  We
			 have not yet output all the local symbols, so
			 we do not know the symbol index of any global
			 symbol.  We set the rel_hash entry for this
			 reloc to point to the global hash table entry
			 for this symbol.  The symbol index is then
			 set at the end of bfd_elf_final_link.  */
		      indx = r_symndx - extsymoff;
		      rh = elf_sym_hashes (input_bfd)[indx];
		      while (rh->root.type == bfd_link_hash_indirect
			     || rh->root.type == bfd_link_hash_warning)
			rh = (struct elf_link_hash_entry *) rh->root.u.i.link;

		      /* Setting the index to -2 tells
			 elf_link_output_extsym that this symbol is
			 used by a reloc.  */
		      BFD_ASSERT (rh->indx < 0);
		      rh->indx = -2;
		      *rel_hash = rh;

		      continue;
		    }

		  /* This is a reloc against a local symbol.  */

		  *rel_hash = NULL;
		  sym = isymbuf[r_symndx];
		  sec = flinfo->sections[r_symndx];
		  if (ELF_ST_TYPE (sym.st_info) == STT_SECTION)
		    {
		      /* I suppose the backend ought to fill in the
			 section of any STT_SECTION symbol against a
			 processor specific section.  */
		      r_symndx = STN_UNDEF;
		      if (bfd_is_abs_section (sec))
			;
		      else if (sec == NULL || sec->owner == NULL)
			{
			  bfd_set_error (bfd_error_bad_value);
			  return false;
			}
		      else
			{
			  asection *osec = sec->output_section;

			  /* If we have discarded a section, the output
			     section will be the absolute section.  In
			     case of discarded SEC_MERGE sections, use
			     the kept section.  relocate_section should
			     have already handled discarded linkonce
			     sections.  */
			  if (bfd_is_abs_section (osec)
			      && sec->kept_section != NULL
			      && sec->kept_section->output_section != NULL)
			    {
			      osec = sec->kept_section->output_section;
			      irela->r_addend -= osec->vma;
			    }

			  if (!bfd_is_abs_section (osec))
			    {
			      r_symndx = osec->target_index;
			      if (r_symndx == STN_UNDEF)
				{
				  irela->r_addend += osec->vma;
				  osec = _bfd_nearby_section (output_bfd, osec,
							      osec->vma);
				  irela->r_addend -= osec->vma;
				  r_symndx = osec->target_index;
				}
			    }
			}

		      /* Adjust the addend according to where the
			 section winds up in the output section.  */
		      if (rela_normal)
			irela->r_addend += sec->output_offset;
		    }
		  else
		    {
		      if (flinfo->indices[r_symndx] == -1)
			{
			  unsigned long shlink;
			  const char *name;
			  asection *osec;
			  long indx;

			  if (flinfo->info->strip == strip_all)
			    {
			      /* You can't do ld -r -s.  */
			      bfd_set_error (bfd_error_invalid_operation);
			      return false;
			    }

			  /* This symbol was skipped earlier, but
			     since it is needed by a reloc, we
			     must output it now.  */
			  shlink = symtab_hdr->sh_link;
			  name = (bfd_elf_string_from_elf_section
				  (input_bfd, shlink, sym.st_name));
			  if (name == NULL)
			    return false;

			  osec = sec->output_section;
			  sym.st_shndx =
			    _bfd_elf_section_from_bfd_section (output_bfd,
							       osec);
			  if (sym.st_shndx == SHN_BAD)
			    return false;

			  sym.st_value += sec->output_offset;
			  if (!bfd_link_relocatable (flinfo->info))
			    {
			      sym.st_value += osec->vma;
			      if (ELF_ST_TYPE (sym.st_info) == STT_TLS)
				{
				  struct elf_link_hash_table *htab
				    = elf_hash_table (flinfo->info);

				  /* STT_TLS symbols are relative to PT_TLS
				     segment base.  */
				  if (htab->tls_sec != NULL)
				    sym.st_value -= htab->tls_sec->vma;
				  else
				    sym.st_info
				      = ELF_ST_INFO (ELF_ST_BIND (sym.st_info),
						     STT_NOTYPE);
				}
			    }

			  indx = bfd_get_symcount (output_bfd);
			  ret = elf_link_output_symstrtab (flinfo, name,
							   &sym, sec,
							   NULL);
			  if (ret == 0)
			    return false;
			  else if (ret == 1)
			    flinfo->indices[r_symndx] = indx;
			  else
			    abort ();
			}

		      r_symndx = flinfo->indices[r_symndx];
		    }

		  irela->r_info = ((bfd_vma) r_symndx << r_sym_shift
				   | (irela->r_info & r_type_mask));
		}

	      /* Swap out the relocs.  */
	      input_rel_hdr = esdi->rel.hdr;
	      if (input_rel_hdr && input_rel_hdr->sh_size != 0)
		{
		  if (!bed->elf_backend_emit_relocs (output_bfd, o,
						     input_rel_hdr,
						     internal_relocs,
						     rel_hash_list))
		    return false;
		  internal_relocs += (NUM_SHDR_ENTRIES (input_rel_hdr)
				      * bed->s->int_rels_per_ext_rel);
		  rel_hash_list += NUM_SHDR_ENTRIES (input_rel_hdr);
		}

	      input_rela_hdr = esdi->rela.hdr;
	      if (input_rela_hdr && input_rela_hdr->sh_size != 0)
		{
		  if (!bed->elf_backend_emit_relocs (output_bfd, o,
						     input_rela_hdr,
						     internal_relocs,
						     rela_hash_list))
		    return false;
		}
	    }
	}

      /* Write out the modified section contents.  */
      if (bed->elf_backend_write_section
	  && (*bed->elf_backend_write_section) (output_bfd, flinfo->info, o,
						contents))
	{
	  /* Section written out.  */
	}
      else switch (o->sec_info_type)
	{
	case SEC_INFO_TYPE_STABS:
	  if (! (_bfd_write_section_stabs
		 (output_bfd,
		  &elf_hash_table (flinfo->info)->stab_info,
		  o, &elf_section_data (o)->sec_info, contents)))
	    return false;
	  break;
	case SEC_INFO_TYPE_MERGE:
	  if (! _bfd_write_merged_section (output_bfd, o,
					   elf_section_data (o)->sec_info))
	    return false;
	  break;
	case SEC_INFO_TYPE_EH_FRAME:
	  {
	    if (! _bfd_elf_write_section_eh_frame (output_bfd, flinfo->info,
						   o, contents))
	      return false;
	  }
	  break;
	case SEC_INFO_TYPE_EH_FRAME_ENTRY:
	  {
	    if (! _bfd_elf_write_section_eh_frame_entry (output_bfd,
							 flinfo->info,
							 o, contents))
	      return false;
	  }
	  break;
	case SEC_INFO_TYPE_SFRAME:
	    {
	      /* Merge .sframe sections into the ctf frame encoder
		 context of the output_bfd's section.  The final .sframe
		 output section will be written out later.  */
	      if (!_bfd_elf_merge_section_sframe (output_bfd, flinfo->info,
						  o, contents))
		return false;
	    }
	    break;
	default:
	  {
	    if (! (o->flags & SEC_EXCLUDE))
	      {
		file_ptr offset = (file_ptr) o->output_offset;
		bfd_size_type todo = o->size;

		offset *= bfd_octets_per_byte (output_bfd, o);

		if ((o->flags & SEC_ELF_REVERSE_COPY)
		    && o->size > address_size)
		  {
		    /* Reverse-copy input section to output.  */

		    if ((o->size & (address_size - 1)) != 0
			|| (o->reloc_count != 0
			    && (o->size * bed->s->int_rels_per_ext_rel
				!= o->reloc_count * address_size)))
		      {
			_bfd_error_handler
			  /* xgettext:c-format */
			  (_("error: %pB: size of section %pA is not "
			     "multiple of address size"),
			   input_bfd, o);
			bfd_set_error (bfd_error_bad_value);
			return false;
		      }

		    do
		      {
			todo -= address_size;
			if (! bfd_set_section_contents (output_bfd,
							o->output_section,
							contents + todo,
							offset,
							address_size))
			  return false;
			if (todo == 0)
			  break;
			offset += address_size;
		      }
		    while (1);
		  }
		else if (! bfd_set_section_contents (output_bfd,
						     o->output_section,
						     contents,
						     offset, todo))
		  return false;
	      }
	  }
	  break;
	}
    }

  return true;
}

/* Generate a reloc when linking an ELF file.  This is a reloc
   requested by the linker, and does not come from any input file.  This
   is used to build constructor and destructor tables when linking
   with -Ur.  */

static bool
elf_reloc_link_order (bfd *output_bfd,
		      struct bfd_link_info *info,
		      asection *output_section,
		      struct bfd_link_order *link_order)
{
  reloc_howto_type *howto;
  long indx;
  bfd_vma offset;
  bfd_vma addend;
  struct bfd_elf_section_reloc_data *reldata;
  struct elf_link_hash_entry **rel_hash_ptr;
  Elf_Internal_Shdr *rel_hdr;
  const struct elf_backend_data *bed = get_elf_backend_data (output_bfd);
  Elf_Internal_Rela irel[MAX_INT_RELS_PER_EXT_REL];
  bfd_byte *erel;
  unsigned int i;
  struct bfd_elf_section_data *esdo = elf_section_data (output_section);

  howto = bfd_reloc_type_lookup (output_bfd, link_order->u.reloc.p->reloc);
  if (howto == NULL)
    {
      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  addend = link_order->u.reloc.p->addend;

  if (esdo->rel.hdr)
    reldata = &esdo->rel;
  else if (esdo->rela.hdr)
    reldata = &esdo->rela;
  else
    {
      reldata = NULL;
      BFD_ASSERT (0);
    }

  /* Figure out the symbol index.  */
  rel_hash_ptr = reldata->hashes + reldata->count;
  if (link_order->type == bfd_section_reloc_link_order)
    {
      indx = link_order->u.reloc.p->u.section->target_index;
      BFD_ASSERT (indx != 0);
      *rel_hash_ptr = NULL;
    }
  else
    {
      struct elf_link_hash_entry *h;

      /* Treat a reloc against a defined symbol as though it were
	 actually against the section.  */
      h = ((struct elf_link_hash_entry *)
	   bfd_wrapped_link_hash_lookup (output_bfd, info,
					 link_order->u.reloc.p->u.name,
					 false, false, true));
      if (h != NULL
	  && (h->root.type == bfd_link_hash_defined
	      || h->root.type == bfd_link_hash_defweak))
	{
	  asection *section;

	  section = h->root.u.def.section;
	  indx = section->output_section->target_index;
	  *rel_hash_ptr = NULL;
	  /* It seems that we ought to add the symbol value to the
	     addend here, but in practice it has already been added
	     because it was passed to constructor_callback.  */
	  addend += section->output_section->vma + section->output_offset;
	}
      else if (h != NULL)
	{
	  /* Setting the index to -2 tells elf_link_output_extsym that
	     this symbol is used by a reloc.  */
	  h->indx = -2;
	  *rel_hash_ptr = h;
	  indx = 0;
	}
      else
	{
	  (*info->callbacks->unattached_reloc)
	    (info, link_order->u.reloc.p->u.name, NULL, NULL, 0);
	  indx = 0;
	}
    }

  /* If this is an inplace reloc, we must write the addend into the
     object file.  */
  if (howto->partial_inplace && addend != 0)
    {
      bfd_size_type size;
      bfd_reloc_status_type rstat;
      bfd_byte *buf;
      bool ok;
      const char *sym_name;
      bfd_size_type octets;

      size = (bfd_size_type) bfd_get_reloc_size (howto);
      buf = (bfd_byte *) bfd_zmalloc (size);
      if (buf == NULL && size != 0)
	return false;
      rstat = _bfd_relocate_contents (howto, output_bfd, addend, buf);
      switch (rstat)
	{
	case bfd_reloc_ok:
	  break;

	default:
	case bfd_reloc_outofrange:
	  abort ();

	case bfd_reloc_overflow:
	  if (link_order->type == bfd_section_reloc_link_order)
	    sym_name = bfd_section_name (link_order->u.reloc.p->u.section);
	  else
	    sym_name = link_order->u.reloc.p->u.name;
	  (*info->callbacks->reloc_overflow) (info, NULL, sym_name,
					      howto->name, addend, NULL, NULL,
					      (bfd_vma) 0);
	  break;
	}

      octets = link_order->offset * bfd_octets_per_byte (output_bfd,
							 output_section);
      ok = bfd_set_section_contents (output_bfd, output_section, buf,
				     octets, size);
      free (buf);
      if (! ok)
	return false;
    }

  /* The address of a reloc is relative to the section in a
     relocatable file, and is a virtual address in an executable
     file.  */
  offset = link_order->offset;
  if (! bfd_link_relocatable (info))
    offset += output_section->vma;

  for (i = 0; i < bed->s->int_rels_per_ext_rel; i++)
    {
      irel[i].r_offset = offset;
      irel[i].r_info = 0;
      irel[i].r_addend = 0;
    }
  if (bed->s->arch_size == 32)
    irel[0].r_info = ELF32_R_INFO (indx, howto->type);
  else
    irel[0].r_info = ELF64_R_INFO (indx, howto->type);

  rel_hdr = reldata->hdr;
  erel = rel_hdr->contents;
  if (rel_hdr->sh_type == SHT_REL)
    {
      erel += reldata->count * bed->s->sizeof_rel;
      (*bed->s->swap_reloc_out) (output_bfd, irel, erel);
    }
  else
    {
      irel[0].r_addend = addend;
      erel += reldata->count * bed->s->sizeof_rela;
      (*bed->s->swap_reloca_out) (output_bfd, irel, erel);
    }

  ++reldata->count;

  return true;
}

/* Generate an import library in INFO->implib_bfd from symbols in ABFD.
   Returns TRUE upon success, FALSE otherwise.  */

static bool
elf_output_implib (bfd *abfd, struct bfd_link_info *info)
{
  bool ret = false;
  bfd *implib_bfd;
  const struct elf_backend_data *bed;
  flagword flags;
  enum bfd_architecture arch;
  unsigned int mach;
  asymbol **sympp = NULL;
  long symsize;
  long symcount;
  long src_count;
  elf_symbol_type *osymbuf;
  size_t amt;

  implib_bfd = info->out_implib_bfd;
  bed = get_elf_backend_data (abfd);

  if (!bfd_set_format (implib_bfd, bfd_object))
    return false;

  /* Use flag from executable but make it a relocatable object.  */
  flags = bfd_get_file_flags (abfd);
  flags &= ~HAS_RELOC;
  if (!bfd_set_start_address (implib_bfd, 0)
      || !bfd_set_file_flags (implib_bfd, flags & ~EXEC_P))
    return false;

  /* Copy architecture of output file to import library file.  */
  arch = bfd_get_arch (abfd);
  mach = bfd_get_mach (abfd);
  if (!bfd_set_arch_mach (implib_bfd, arch, mach)
      && (abfd->target_defaulted
	  || bfd_get_arch (abfd) != bfd_get_arch (implib_bfd)))
    return false;

  /* Get symbol table size.  */
  symsize = bfd_get_symtab_upper_bound (abfd);
  if (symsize < 0)
    return false;

  /* Read in the symbol table.  */
  sympp = (asymbol **) bfd_malloc (symsize);
  if (sympp == NULL)
    return false;

  symcount = bfd_canonicalize_symtab (abfd, sympp);
  if (symcount < 0)
    goto free_sym_buf;

  /* Allow the BFD backend to copy any private header data it
     understands from the output BFD to the import library BFD.  */
  if (! bfd_copy_private_header_data (abfd, implib_bfd))
    goto free_sym_buf;

  /* Filter symbols to appear in the import library.  */
  if (bed->elf_backend_filter_implib_symbols)
    symcount = bed->elf_backend_filter_implib_symbols (abfd, info, sympp,
						       symcount);
  else
    symcount = _bfd_elf_filter_global_symbols (abfd, info, sympp, symcount);
  if (symcount == 0)
    {
      bfd_set_error (bfd_error_no_symbols);
      _bfd_error_handler (_("%pB: no symbol found for import library"),
			  implib_bfd);
      goto free_sym_buf;
    }


  /* Make symbols absolute.  */
  amt = symcount * sizeof (*osymbuf);
  osymbuf = (elf_symbol_type *) bfd_alloc (implib_bfd, amt);
  if (osymbuf == NULL)
    goto free_sym_buf;

  for (src_count = 0; src_count < symcount; src_count++)
    {
      memcpy (&osymbuf[src_count], (elf_symbol_type *) sympp[src_count],
	      sizeof (*osymbuf));
      osymbuf[src_count].symbol.section = bfd_abs_section_ptr;
      osymbuf[src_count].internal_elf_sym.st_shndx = SHN_ABS;
      osymbuf[src_count].symbol.value += sympp[src_count]->section->vma;
      osymbuf[src_count].internal_elf_sym.st_value =
	osymbuf[src_count].symbol.value;
      sympp[src_count] = &osymbuf[src_count].symbol;
    }

  bfd_set_symtab (implib_bfd, sympp, symcount);

  /* Allow the BFD backend to copy any private data it understands
     from the output BFD to the import library BFD.  This is done last
     to permit the routine to look at the filtered symbol table.  */
  if (! bfd_copy_private_bfd_data (abfd, implib_bfd))
    goto free_sym_buf;

  if (!bfd_close (implib_bfd))
    goto free_sym_buf;

  ret = true;

 free_sym_buf:
  free (sympp);
  return ret;
}

static void
elf_final_link_free (bfd *obfd, struct elf_final_link_info *flinfo)
{
  asection *o;

  if (flinfo->symstrtab != NULL)
    _bfd_elf_strtab_free (flinfo->symstrtab);
  free (flinfo->contents);
  free (flinfo->external_relocs);
  free (flinfo->internal_relocs);
  free (flinfo->external_syms);
  free (flinfo->locsym_shndx);
  free (flinfo->internal_syms);
  free (flinfo->indices);
  free (flinfo->sections);
  if (flinfo->symshndxbuf != (Elf_External_Sym_Shndx *) -1)
    free (flinfo->symshndxbuf);
  for (o = obfd->sections; o != NULL; o = o->next)
    {
      struct bfd_elf_section_data *esdo = elf_section_data (o);
      free (esdo->rel.hashes);
      free (esdo->rela.hashes);
    }
}

/* Do the final step of an ELF link.  */

bool
bfd_elf_final_link (bfd *abfd, struct bfd_link_info *info)
{
  bool dynamic;
  bool emit_relocs;
  bfd *dynobj;
  struct elf_final_link_info flinfo;
  asection *o;
  struct bfd_link_order *p;
  bfd *sub;
  bfd_size_type max_contents_size;
  bfd_size_type max_external_reloc_size;
  bfd_size_type max_internal_reloc_count;
  bfd_size_type max_sym_count;
  bfd_size_type max_sym_shndx_count;
  Elf_Internal_Sym elfsym;
  unsigned int i;
  Elf_Internal_Shdr *symtab_hdr;
  Elf_Internal_Shdr *symtab_shndx_hdr;
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  struct elf_outext_info eoinfo;
  bool merged;
  size_t relativecount;
  size_t relr_entsize;
  asection *reldyn = 0;
  bfd_size_type amt;
  asection *attr_section = NULL;
  bfd_vma attr_size = 0;
  const char *std_attrs_section;
  struct elf_link_hash_table *htab = elf_hash_table (info);
  bool sections_removed;
  bool ret;

  if (!is_elf_hash_table (&htab->root))
    return false;

  if (bfd_link_pic (info))
    abfd->flags |= DYNAMIC;

  dynamic = htab->dynamic_sections_created;
  dynobj = htab->dynobj;

  emit_relocs = (bfd_link_relocatable (info)
		 || info->emitrelocations);

  memset (&flinfo, 0, sizeof (flinfo));
  flinfo.info = info;
  flinfo.output_bfd = abfd;
  flinfo.symstrtab = _bfd_elf_strtab_init ();
  if (flinfo.symstrtab == NULL)
    return false;

  if (! dynamic)
    {
      flinfo.hash_sec = NULL;
      flinfo.symver_sec = NULL;
    }
  else
    {
      flinfo.hash_sec = bfd_get_linker_section (dynobj, ".hash");
      /* Note that dynsym_sec can be NULL (on VMS).  */
      flinfo.symver_sec = bfd_get_linker_section (dynobj, ".gnu.version");
      /* Note that it is OK if symver_sec is NULL.  */
    }

  if (info->unique_symbol
      && !bfd_hash_table_init (&flinfo.local_hash_table,
			       local_hash_newfunc,
			       sizeof (struct local_hash_entry)))
    return false;

  /* The object attributes have been merged.  Remove the input
     sections from the link, and set the contents of the output
     section.  */
  sections_removed = false;
  std_attrs_section = get_elf_backend_data (abfd)->obj_attrs_section;
  for (o = abfd->sections; o != NULL; o = o->next)
    {
      bool remove_section = false;

      if ((std_attrs_section && strcmp (o->name, std_attrs_section) == 0)
	  || strcmp (o->name, ".gnu.attributes") == 0)
	{
	  for (p = o->map_head.link_order; p != NULL; p = p->next)
	    {
	      asection *input_section;

	      if (p->type != bfd_indirect_link_order)
		continue;
	      input_section = p->u.indirect.section;
	      /* Hack: reset the SEC_HAS_CONTENTS flag so that
		 elf_link_input_bfd ignores this section.  */
	      input_section->flags &= ~SEC_HAS_CONTENTS;
	    }

	  attr_size = bfd_elf_obj_attr_size (abfd);
	  bfd_set_section_size (o, attr_size);
	  /* Skip this section later on.  */
	  o->map_head.link_order = NULL;
	  if (attr_size)
	    attr_section = o;
	  else
	    remove_section = true;
	}
      else if ((o->flags & SEC_GROUP) != 0 && o->size == 0)
	{
	  /* Remove empty group section from linker output.  */
	  remove_section = true;
	}
      if (remove_section)
	{
	  o->flags |= SEC_EXCLUDE;
	  bfd_section_list_remove (abfd, o);
	  abfd->section_count--;
	  sections_removed = true;
	}
    }
  if (sections_removed)
    _bfd_fix_excluded_sec_syms (abfd, info);

  /* Count up the number of relocations we will output for each output
     section, so that we know the sizes of the reloc sections.  We
     also figure out some maximum sizes.  */
  max_contents_size = 0;
  max_external_reloc_size = 0;
  max_internal_reloc_count = 0;
  max_sym_count = 0;
  max_sym_shndx_count = 0;
  merged = false;
  for (o = abfd->sections; o != NULL; o = o->next)
    {
      struct bfd_elf_section_data *esdo = elf_section_data (o);
      o->reloc_count = 0;

      for (p = o->map_head.link_order; p != NULL; p = p->next)
	{
	  unsigned int reloc_count = 0;
	  unsigned int additional_reloc_count = 0;
	  struct bfd_elf_section_data *esdi = NULL;

	  if (p->type == bfd_section_reloc_link_order
	      || p->type == bfd_symbol_reloc_link_order)
	    reloc_count = 1;
	  else if (p->type == bfd_indirect_link_order)
	    {
	      asection *sec;

	      sec = p->u.indirect.section;

	      /* Mark all sections which are to be included in the
		 link.  This will normally be every section.  We need
		 to do this so that we can identify any sections which
		 the linker has decided to not include.  */
	      sec->linker_mark = true;

	      if (sec->flags & SEC_MERGE)
		merged = true;

	      if (sec->rawsize > max_contents_size)
		max_contents_size = sec->rawsize;
	      if (sec->size > max_contents_size)
		max_contents_size = sec->size;

	      if (bfd_get_flavour (sec->owner) == bfd_target_elf_flavour
		  && (sec->owner->flags & DYNAMIC) == 0)
		{
		  size_t sym_count;

		  /* We are interested in just local symbols, not all
		     symbols.  */
		  if (elf_bad_symtab (sec->owner))
		    sym_count = (elf_tdata (sec->owner)->symtab_hdr.sh_size
				 / bed->s->sizeof_sym);
		  else
		    sym_count = elf_tdata (sec->owner)->symtab_hdr.sh_info;

		  if (sym_count > max_sym_count)
		    max_sym_count = sym_count;

		  if (sym_count > max_sym_shndx_count
		      && elf_symtab_shndx_list (sec->owner) != NULL)
		    max_sym_shndx_count = sym_count;

		  esdi = elf_section_data (sec);

		  if (esdi->this_hdr.sh_type == SHT_REL
		      || esdi->this_hdr.sh_type == SHT_RELA)
		    /* Some backends use reloc_count in relocation sections
		       to count particular types of relocs.  Of course,
		       reloc sections themselves can't have relocations.  */
		    ;
		  else if (emit_relocs)
		    {
		      reloc_count = sec->reloc_count;
		      if (bed->elf_backend_count_additional_relocs)
			{
			  int c;
			  c = (*bed->elf_backend_count_additional_relocs) (sec);
			  additional_reloc_count += c;
			}
		    }
		  else if (bed->elf_backend_count_relocs)
		    reloc_count = (*bed->elf_backend_count_relocs) (info, sec);

		  if ((sec->flags & SEC_RELOC) != 0)
		    {
		      size_t ext_size = 0;

		      if (esdi->rel.hdr != NULL)
			ext_size = esdi->rel.hdr->sh_size;
		      if (esdi->rela.hdr != NULL)
			ext_size += esdi->rela.hdr->sh_size;

		      if (ext_size > max_external_reloc_size)
			max_external_reloc_size = ext_size;
		      if (sec->reloc_count > max_internal_reloc_count)
			max_internal_reloc_count = sec->reloc_count;
		    }
		}
	    }

	  if (reloc_count == 0)
	    continue;

	  reloc_count += additional_reloc_count;
	  o->reloc_count += reloc_count;

	  if (p->type == bfd_indirect_link_order && emit_relocs)
	    {
	      if (esdi->rel.hdr)
		{
		  esdo->rel.count += NUM_SHDR_ENTRIES (esdi->rel.hdr);
		  esdo->rel.count += additional_reloc_count;
		}
	      if (esdi->rela.hdr)
		{
		  esdo->rela.count += NUM_SHDR_ENTRIES (esdi->rela.hdr);
		  esdo->rela.count += additional_reloc_count;
		}
	    }
	  else
	    {
	      if (o->use_rela_p)
		esdo->rela.count += reloc_count;
	      else
		esdo->rel.count += reloc_count;
	    }
	}

      if (o->reloc_count > 0)
	o->flags |= SEC_RELOC;
      else
	{
	  /* Explicitly clear the SEC_RELOC flag.  The linker tends to
	     set it (this is probably a bug) and if it is set
	     assign_section_numbers will create a reloc section.  */
	  o->flags &=~ SEC_RELOC;
	}

      /* If the SEC_ALLOC flag is not set, force the section VMA to
	 zero.  This is done in elf_fake_sections as well, but forcing
	 the VMA to 0 here will ensure that relocs against these
	 sections are handled correctly.  */
      if ((o->flags & SEC_ALLOC) == 0
	  && ! o->user_set_vma)
	o->vma = 0;
    }

  if (! bfd_link_relocatable (info) && merged)
    elf_link_hash_traverse (htab, _bfd_elf_link_sec_merge_syms, abfd);

  /* Figure out the file positions for everything but the symbol table
     and the relocs.  We set symcount to force assign_section_numbers
     to create a symbol table.  */
  abfd->symcount = info->strip != strip_all || emit_relocs;
  BFD_ASSERT (! abfd->output_has_begun);
  if (! _bfd_elf_compute_section_file_positions (abfd, info))
    goto error_return;

  /* Set sizes, and assign file positions for reloc sections.  */
  for (o = abfd->sections; o != NULL; o = o->next)
    {
      struct bfd_elf_section_data *esdo = elf_section_data (o);
      if ((o->flags & SEC_RELOC) != 0)
	{
	  if (esdo->rel.hdr
	      && !(_bfd_elf_link_size_reloc_section (abfd, &esdo->rel)))
	    goto error_return;

	  if (esdo->rela.hdr
	      && !(_bfd_elf_link_size_reloc_section (abfd, &esdo->rela)))
	    goto error_return;
	}

      /* _bfd_elf_compute_section_file_positions makes temporary use
	 of target_index.  Reset it.  */
      o->target_index = 0;

      /* Now, reset REL_COUNT and REL_COUNT2 so that we can use them
	 to count upwards while actually outputting the relocations.  */
      esdo->rel.count = 0;
      esdo->rela.count = 0;

      if ((esdo->this_hdr.sh_offset == (file_ptr) -1)
	  && !bfd_section_is_ctf (o))
	{
	  /* Cache the section contents so that they can be compressed
	     later.  Use bfd_malloc since it will be freed by
	     bfd_compress_section_contents.  */
	  unsigned char *contents = esdo->this_hdr.contents;
	  if (contents != NULL)
	    abort ();
	  contents
	    = (unsigned char *) bfd_malloc (esdo->this_hdr.sh_size);
	  if (contents == NULL)
	    goto error_return;
	  esdo->this_hdr.contents = contents;
	}
    }

  /* We have now assigned file positions for all the sections except .symtab,
     .strtab, and non-loaded reloc and compressed debugging sections.  We start
     the .symtab section at the current file position, and write directly to it.
     We build the .strtab section in memory.  */
  abfd->symcount = 0;
  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
  /* sh_name is set in prep_headers.  */
  symtab_hdr->sh_type = SHT_SYMTAB;
  /* sh_flags, sh_addr and sh_size all start off zero.  */
  symtab_hdr->sh_entsize = bed->s->sizeof_sym;
  /* sh_link is set in assign_section_numbers.  */
  /* sh_info is set below.  */
  /* sh_offset is set just below.  */
  symtab_hdr->sh_addralign = (bfd_vma) 1 << bed->s->log_file_align;

  if (max_sym_count < 20)
    max_sym_count = 20;
  htab->strtabsize = max_sym_count;
  amt = max_sym_count * sizeof (struct elf_sym_strtab);
  htab->strtab = (struct elf_sym_strtab *) bfd_malloc (amt);
  if (htab->strtab == NULL)
    goto error_return;
  /* The real buffer will be allocated in elf_link_swap_symbols_out.  */
  flinfo.symshndxbuf
    = (elf_numsections (abfd) > (SHN_LORESERVE & 0xFFFF)
       ? (Elf_External_Sym_Shndx *) -1 : NULL);

  if (info->strip != strip_all || emit_relocs)
    {
      file_ptr off = elf_next_file_pos (abfd);

      _bfd_elf_assign_file_position_for_section (symtab_hdr, off, true);

      /* Note that at this point elf_next_file_pos (abfd) is
	 incorrect.  We do not yet know the size of the .symtab section.
	 We correct next_file_pos below, after we do know the size.  */

      /* Start writing out the symbol table.  The first symbol is always a
	 dummy symbol.  */
      elfsym.st_value = 0;
      elfsym.st_size = 0;
      elfsym.st_info = 0;
      elfsym.st_other = 0;
      elfsym.st_shndx = SHN_UNDEF;
      elfsym.st_target_internal = 0;
      if (elf_link_output_symstrtab (&flinfo, NULL, &elfsym,
				     bfd_und_section_ptr, NULL) != 1)
	goto error_return;

      /* Output a symbol for each section if asked or they are used for
	 relocs.  These symbols usually have no names.  We store the
	 index of each one in the index field of the section, so that
	 we can find it again when outputting relocs.  */

      if (bfd_keep_unused_section_symbols (abfd) || emit_relocs)
	{
	  bool name_local_sections
	    = (bed->elf_backend_name_local_section_symbols
	       && bed->elf_backend_name_local_section_symbols (abfd));
	  const char *name = NULL;

	  elfsym.st_size = 0;
	  elfsym.st_info = ELF_ST_INFO (STB_LOCAL, STT_SECTION);
	  elfsym.st_other = 0;
	  elfsym.st_value = 0;
	  elfsym.st_target_internal = 0;
	  for (i = 1; i < elf_numsections (abfd); i++)
	    {
	      o = bfd_section_from_elf_index (abfd, i);
	      if (o != NULL)
		{
		  o->target_index = bfd_get_symcount (abfd);
		  elfsym.st_shndx = i;
		  if (!bfd_link_relocatable (info))
		    elfsym.st_value = o->vma;
		  if (name_local_sections)
		    name = o->name;
		  if (elf_link_output_symstrtab (&flinfo, name, &elfsym, o,
						 NULL) != 1)
		    goto error_return;
		}
	    }
	}
    }

  /* On some targets like Irix 5 the symbol split between local and global
     ones recorded in the sh_info field needs to be done between section
     and all other symbols.  */
  if (bed->elf_backend_elfsym_local_is_section
      && bed->elf_backend_elfsym_local_is_section (abfd))
    symtab_hdr->sh_info = bfd_get_symcount (abfd);

  /* Allocate some memory to hold information read in from the input
     files.  */
  if (max_contents_size != 0)
    {
      flinfo.contents = (bfd_byte *) bfd_malloc (max_contents_size);
      if (flinfo.contents == NULL)
	goto error_return;
    }

  if (max_external_reloc_size != 0)
    {
      flinfo.external_relocs = bfd_malloc (max_external_reloc_size);
      if (flinfo.external_relocs == NULL)
	goto error_return;
    }

  if (max_internal_reloc_count != 0)
    {
      amt = max_internal_reloc_count * sizeof (Elf_Internal_Rela);
      flinfo.internal_relocs = (Elf_Internal_Rela *) bfd_malloc (amt);
      if (flinfo.internal_relocs == NULL)
	goto error_return;
    }

  if (max_sym_count != 0)
    {
      amt = max_sym_count * bed->s->sizeof_sym;
      flinfo.external_syms = (bfd_byte *) bfd_malloc (amt);
      if (flinfo.external_syms == NULL)
	goto error_return;

      amt = max_sym_count * sizeof (Elf_Internal_Sym);
      flinfo.internal_syms = (Elf_Internal_Sym *) bfd_malloc (amt);
      if (flinfo.internal_syms == NULL)
	goto error_return;

      amt = max_sym_count * sizeof (long);
      flinfo.indices = (long int *) bfd_malloc (amt);
      if (flinfo.indices == NULL)
	goto error_return;

      amt = max_sym_count * sizeof (asection *);
      flinfo.sections = (asection **) bfd_malloc (amt);
      if (flinfo.sections == NULL)
	goto error_return;
    }

  if (max_sym_shndx_count != 0)
    {
      amt = max_sym_shndx_count * sizeof (Elf_External_Sym_Shndx);
      flinfo.locsym_shndx = (Elf_External_Sym_Shndx *) bfd_malloc (amt);
      if (flinfo.locsym_shndx == NULL)
	goto error_return;
    }

  if (htab->tls_sec)
    {
      bfd_vma base, end = 0;  /* Both bytes.  */
      asection *sec;

      for (sec = htab->tls_sec;
	   sec && (sec->flags & SEC_THREAD_LOCAL);
	   sec = sec->next)
	{
	  bfd_size_type size = sec->size;
	  unsigned int opb = bfd_octets_per_byte (abfd, sec);

	  if (size == 0
	      && (sec->flags & SEC_HAS_CONTENTS) == 0)
	    {
	      struct bfd_link_order *ord = sec->map_tail.link_order;

	      if (ord != NULL)
		size = ord->offset * opb + ord->size;
	    }
	  end = sec->vma + size / opb;
	}
      base = htab->tls_sec->vma;
      /* Only align end of TLS section if static TLS doesn't have special
	 alignment requirements.  */
      if (bed->static_tls_alignment == 1)
	end = align_power (end, htab->tls_sec->alignment_power);
      htab->tls_size = end - base;
    }

  if (!_bfd_elf_fixup_eh_frame_hdr (info))
    return false;

  /* Finish relative relocations here after regular symbol processing
     is finished if DT_RELR is enabled.  */
  if (info->enable_dt_relr
      && bed->finish_relative_relocs
      && !bed->finish_relative_relocs (info))
    info->callbacks->einfo
      (_("%F%P: %pB: failed to finish relative relocations\n"), abfd);

  /* Since ELF permits relocations to be against local symbols, we
     must have the local symbols available when we do the relocations.
     Since we would rather only read the local symbols once, and we
     would rather not keep them in memory, we handle all the
     relocations for a single input file at the same time.

     Unfortunately, there is no way to know the total number of local
     symbols until we have seen all of them, and the local symbol
     indices precede the global symbol indices.  This means that when
     we are generating relocatable output, and we see a reloc against
     a global symbol, we can not know the symbol index until we have
     finished examining all the local symbols to see which ones we are
     going to output.  To deal with this, we keep the relocations in
     memory, and don't output them until the end of the link.  This is
     an unfortunate waste of memory, but I don't see a good way around
     it.  Fortunately, it only happens when performing a relocatable
     link, which is not the common case.  FIXME: If keep_memory is set
     we could write the relocs out and then read them again; I don't
     know how bad the memory loss will be.  */

  for (sub = info->input_bfds; sub != NULL; sub = sub->link.next)
    sub->output_has_begun = false;
  for (o = abfd->sections; o != NULL; o = o->next)
    {
      for (p = o->map_head.link_order; p != NULL; p = p->next)
	{
	  if (p->type == bfd_indirect_link_order
	      && (bfd_get_flavour ((sub = p->u.indirect.section->owner))
		  == bfd_target_elf_flavour)
	      && elf_elfheader (sub)->e_ident[EI_CLASS] == bed->s->elfclass)
	    {
	      if (! sub->output_has_begun)
		{
		  if (! elf_link_input_bfd (&flinfo, sub))
		    goto error_return;
		  sub->output_has_begun = true;
		}
	    }
	  else if (p->type == bfd_section_reloc_link_order
		   || p->type == bfd_symbol_reloc_link_order)
	    {
	      if (! elf_reloc_link_order (abfd, info, o, p))
		goto error_return;
	    }
	  else
	    {
	      if (! _bfd_default_link_order (abfd, info, o, p))
		{
		  if (p->type == bfd_indirect_link_order
		      && (bfd_get_flavour (sub)
			  == bfd_target_elf_flavour)
		      && (elf_elfheader (sub)->e_ident[EI_CLASS]
			  != bed->s->elfclass))
		    {
		      const char *iclass, *oclass;

		      switch (bed->s->elfclass)
			{
			case ELFCLASS64: oclass = "ELFCLASS64"; break;
			case ELFCLASS32: oclass = "ELFCLASS32"; break;
			case ELFCLASSNONE: oclass = "ELFCLASSNONE"; break;
			default: abort ();
			}

		      switch (elf_elfheader (sub)->e_ident[EI_CLASS])
			{
			case ELFCLASS64: iclass = "ELFCLASS64"; break;
			case ELFCLASS32: iclass = "ELFCLASS32"; break;
			case ELFCLASSNONE: iclass = "ELFCLASSNONE"; break;
			default: abort ();
			}

		      bfd_set_error (bfd_error_wrong_format);
		      _bfd_error_handler
			/* xgettext:c-format */
			(_("%pB: file class %s incompatible with %s"),
			 sub, iclass, oclass);
		    }

		  goto error_return;
		}
	    }
	}
    }

  /* Free symbol buffer if needed.  */
  if (!info->reduce_memory_overheads)
    {
      for (sub = info->input_bfds; sub != NULL; sub = sub->link.next)
	if (bfd_get_flavour (sub) == bfd_target_elf_flavour)
	  {
	    free (elf_tdata (sub)->symbuf);
	    elf_tdata (sub)->symbuf = NULL;
	  }
    }

  ret = true;

  /* Output any global symbols that got converted to local in a
     version script or due to symbol visibility.  We do this in a
     separate step since ELF requires all local symbols to appear
     prior to any global symbols.  FIXME: We should only do this if
     some global symbols were, in fact, converted to become local.
     FIXME: Will this work correctly with the Irix 5 linker?  */
  eoinfo.failed = false;
  eoinfo.flinfo = &flinfo;
  eoinfo.localsyms = true;
  eoinfo.file_sym_done = false;
  bfd_hash_traverse (&info->hash->table, elf_link_output_extsym, &eoinfo);
  if (eoinfo.failed)
    {
      ret = false;
      goto return_local_hash_table;
    }

  /* If backend needs to output some local symbols not present in the hash
     table, do it now.  */
  if (bed->elf_backend_output_arch_local_syms)
    {
      if (! ((*bed->elf_backend_output_arch_local_syms)
	     (abfd, info, &flinfo, elf_link_output_symstrtab)))
	{
	  ret = false;
	  goto return_local_hash_table;
	}
    }

  /* That wrote out all the local symbols.  Finish up the symbol table
     with the global symbols. Even if we want to strip everything we
     can, we still need to deal with those global symbols that got
     converted to local in a version script.  */

  /* The sh_info field records the index of the first non local symbol.  */
  if (!symtab_hdr->sh_info)
    symtab_hdr->sh_info = bfd_get_symcount (abfd);

  if (dynamic
      && htab->dynsym != NULL
      && htab->dynsym->output_section != bfd_abs_section_ptr)
    {
      Elf_Internal_Sym sym;
      bfd_byte *dynsym = htab->dynsym->contents;

      o = htab->dynsym->output_section;
      elf_section_data (o)->this_hdr.sh_info = htab->local_dynsymcount + 1;

      /* Write out the section symbols for the output sections.  */
      if (bfd_link_pic (info)
	  || htab->is_relocatable_executable)
	{
	  asection *s;

	  sym.st_size = 0;
	  sym.st_name = 0;
	  sym.st_info = ELF_ST_INFO (STB_LOCAL, STT_SECTION);
	  sym.st_other = 0;
	  sym.st_target_internal = 0;

	  for (s = abfd->sections; s != NULL; s = s->next)
	    {
	      int indx;
	      bfd_byte *dest;
	      long dynindx;

	      dynindx = elf_section_data (s)->dynindx;
	      if (dynindx <= 0)
		continue;
	      indx = elf_section_data (s)->this_idx;
	      BFD_ASSERT (indx > 0);
	      sym.st_shndx = indx;
	      if (! check_dynsym (abfd, &sym))
		{
		  ret = false;
		  goto return_local_hash_table;
		}
	      sym.st_value = s->vma;
	      dest = dynsym + dynindx * bed->s->sizeof_sym;

	      /* Inform the linker of the addition of this symbol.  */

	      if (info->callbacks->ctf_new_dynsym)
		info->callbacks->ctf_new_dynsym (dynindx, &sym);

	      bed->s->swap_symbol_out (abfd, &sym, dest, 0);
	    }
	}

      /* Write out the local dynsyms.  */
      if (htab->dynlocal)
	{
	  struct elf_link_local_dynamic_entry *e;
	  for (e = htab->dynlocal; e ; e = e->next)
	    {
	      asection *s;
	      bfd_byte *dest;

	      /* Copy the internal symbol and turn off visibility.
		 Note that we saved a word of storage and overwrote
		 the original st_name with the dynstr_index.  */
	      sym = e->isym;
	      sym.st_other &= ~ELF_ST_VISIBILITY (-1);
	      sym.st_shndx = SHN_UNDEF;

	      s = bfd_section_from_elf_index (e->input_bfd,
					      e->isym.st_shndx);
	      if (s != NULL
		  && s->output_section != NULL
		  && elf_section_data (s->output_section) != NULL)
		{
		  sym.st_shndx =
		    elf_section_data (s->output_section)->this_idx;
		  if (! check_dynsym (abfd, &sym))
		    {
		      ret = false;
		      goto return_local_hash_table;
		    }
		  sym.st_value = (s->output_section->vma
				  + s->output_offset
				  + e->isym.st_value);
		}

	      /* Inform the linker of the addition of this symbol.  */

	      if (info->callbacks->ctf_new_dynsym)
		info->callbacks->ctf_new_dynsym (e->dynindx, &sym);

	      dest = dynsym + e->dynindx * bed->s->sizeof_sym;
	      bed->s->swap_symbol_out (abfd, &sym, dest, 0);
	    }
	}
    }

  /* We get the global symbols from the hash table.  */
  eoinfo.failed = false;
  eoinfo.localsyms = false;
  eoinfo.flinfo = &flinfo;
  bfd_hash_traverse (&info->hash->table, elf_link_output_extsym, &eoinfo);
  if (eoinfo.failed)
    {
      ret = false;
      goto return_local_hash_table;
    }

  /* If backend needs to output some symbols not present in the hash
     table, do it now.  */
  if (bed->elf_backend_output_arch_syms
      && (info->strip != strip_all || emit_relocs))
    {
      if (! ((*bed->elf_backend_output_arch_syms)
	     (abfd, info, &flinfo, elf_link_output_symstrtab)))
	{
	  ret = false;
	  goto return_local_hash_table;
	}
    }

  /* Finalize the .strtab section.  */
  _bfd_elf_strtab_finalize (flinfo.symstrtab);

  /* Swap out the .strtab section. */
  if (!elf_link_swap_symbols_out (&flinfo))
    {
      ret = false;
      goto return_local_hash_table;
    }

  /* Now we know the size of the symtab section.  */
  if (bfd_get_symcount (abfd) > 0)
    {
      /* Finish up and write out the symbol string table (.strtab)
	 section.  */
      Elf_Internal_Shdr *symstrtab_hdr = NULL;
      file_ptr off = symtab_hdr->sh_offset + symtab_hdr->sh_size;

      if (elf_symtab_shndx_list (abfd))
	{
	  symtab_shndx_hdr = & elf_symtab_shndx_list (abfd)->hdr;

	  if (symtab_shndx_hdr != NULL && symtab_shndx_hdr->sh_name != 0)
	    {
	      symtab_shndx_hdr->sh_type = SHT_SYMTAB_SHNDX;
	      symtab_shndx_hdr->sh_entsize = sizeof (Elf_External_Sym_Shndx);
	      symtab_shndx_hdr->sh_addralign = sizeof (Elf_External_Sym_Shndx);
	      amt = bfd_get_symcount (abfd) * sizeof (Elf_External_Sym_Shndx);
	      symtab_shndx_hdr->sh_size = amt;

	      off = _bfd_elf_assign_file_position_for_section (symtab_shndx_hdr,
							       off, true);

	      if (bfd_seek (abfd, symtab_shndx_hdr->sh_offset, SEEK_SET) != 0
		  || (bfd_bwrite (flinfo.symshndxbuf, amt, abfd) != amt))
		{
		  ret = false;
		  goto return_local_hash_table;
		}
	    }
	}

      symstrtab_hdr = &elf_tdata (abfd)->strtab_hdr;
      /* sh_name was set in prep_headers.  */
      symstrtab_hdr->sh_type = SHT_STRTAB;
      symstrtab_hdr->sh_flags = bed->elf_strtab_flags;
      symstrtab_hdr->sh_addr = 0;
      symstrtab_hdr->sh_size = _bfd_elf_strtab_size (flinfo.symstrtab);
      symstrtab_hdr->sh_entsize = 0;
      symstrtab_hdr->sh_link = 0;
      symstrtab_hdr->sh_info = 0;
      /* sh_offset is set just below.  */
      symstrtab_hdr->sh_addralign = 1;

      off = _bfd_elf_assign_file_position_for_section (symstrtab_hdr,
						       off, true);
      elf_next_file_pos (abfd) = off;

      if (bfd_seek (abfd, symstrtab_hdr->sh_offset, SEEK_SET) != 0
	  || ! _bfd_elf_strtab_emit (abfd, flinfo.symstrtab))
	{
	  ret = false;
	  goto return_local_hash_table;
	}
    }

  if (info->out_implib_bfd && !elf_output_implib (abfd, info))
    {
      _bfd_error_handler (_("%pB: failed to generate import library"),
			  info->out_implib_bfd);
      ret = false;
      goto return_local_hash_table;
    }

  /* Adjust the relocs to have the correct symbol indices.  */
  for (o = abfd->sections; o != NULL; o = o->next)
    {
      struct bfd_elf_section_data *esdo = elf_section_data (o);
      bool sort;

      if ((o->flags & SEC_RELOC) == 0)
	continue;

      sort = bed->sort_relocs_p == NULL || (*bed->sort_relocs_p) (o);
      if (esdo->rel.hdr != NULL
	  && !elf_link_adjust_relocs (abfd, o, &esdo->rel, sort, info))
	{
	  ret = false;
	  goto return_local_hash_table;
	}
      if (esdo->rela.hdr != NULL
	  && !elf_link_adjust_relocs (abfd, o, &esdo->rela, sort, info))
	{
	  ret = false;
	  goto return_local_hash_table;
	}

      /* Set the reloc_count field to 0 to prevent write_relocs from
	 trying to swap the relocs out itself.  */
      o->reloc_count = 0;
    }

  relativecount = 0;
  if (dynamic && info->combreloc && dynobj != NULL)
    relativecount = elf_link_sort_relocs (abfd, info, &reldyn);

  relr_entsize = 0;
  if (htab->srelrdyn != NULL
      && htab->srelrdyn->output_section != NULL
      && htab->srelrdyn->size != 0)
    {
      asection *s = htab->srelrdyn->output_section;
      relr_entsize = elf_section_data (s)->this_hdr.sh_entsize;
      if (relr_entsize == 0)
	{
	  relr_entsize = bed->s->arch_size / 8;
	  elf_section_data (s)->this_hdr.sh_entsize = relr_entsize;
	}
    }

  /* If we are linking against a dynamic object, or generating a
     shared library, finish up the dynamic linking information.  */
  if (dynamic)
    {
      bfd_byte *dyncon, *dynconend;

      /* Fix up .dynamic entries.  */
      o = bfd_get_linker_section (dynobj, ".dynamic");
      BFD_ASSERT (o != NULL);

      dyncon = o->contents;
      dynconend = PTR_ADD (o->contents, o->size);
      for (; dyncon < dynconend; dyncon += bed->s->sizeof_dyn)
	{
	  Elf_Internal_Dyn dyn;
	  const char *name;
	  unsigned int type;
	  bfd_size_type sh_size;
	  bfd_vma sh_addr;

	  bed->s->swap_dyn_in (dynobj, dyncon, &dyn);

	  switch (dyn.d_tag)
	    {
	    default:
	      continue;
	    case DT_NULL:
	      if (relativecount != 0)
		{
		  switch (elf_section_data (reldyn)->this_hdr.sh_type)
		    {
		    case SHT_REL: dyn.d_tag = DT_RELCOUNT; break;
		    case SHT_RELA: dyn.d_tag = DT_RELACOUNT; break;
		    }
		  if (dyn.d_tag != DT_NULL
		      && dynconend - dyncon >= bed->s->sizeof_dyn)
		    {
		      dyn.d_un.d_val = relativecount;
		      relativecount = 0;
		      break;
		    }
		  relativecount = 0;
		}
	      if (relr_entsize != 0)
		{
		  if (dynconend - dyncon >= 3 * bed->s->sizeof_dyn)
		    {
		      asection *s = htab->srelrdyn;
		      dyn.d_tag = DT_RELR;
		      dyn.d_un.d_ptr
			= s->output_section->vma + s->output_offset;
		      bed->s->swap_dyn_out (dynobj, &dyn, dyncon);
		      dyncon += bed->s->sizeof_dyn;

		      dyn.d_tag = DT_RELRSZ;
		      dyn.d_un.d_val = s->size;
		      bed->s->swap_dyn_out (dynobj, &dyn, dyncon);
		      dyncon += bed->s->sizeof_dyn;

		      dyn.d_tag = DT_RELRENT;
		      dyn.d_un.d_val = relr_entsize;
		      relr_entsize = 0;
		      break;
		    }
		  relr_entsize = 0;
		}
	      continue;

	    case DT_INIT:
	      name = info->init_function;
	      goto get_sym;
	    case DT_FINI:
	      name = info->fini_function;
	    get_sym:
	      {
		struct elf_link_hash_entry *h;

		h = elf_link_hash_lookup (htab, name, false, false, true);
		if (h != NULL
		    && (h->root.type == bfd_link_hash_defined
			|| h->root.type == bfd_link_hash_defweak))
		  {
		    dyn.d_un.d_ptr = h->root.u.def.value;
		    o = h->root.u.def.section;
		    if (o->output_section != NULL)
		      dyn.d_un.d_ptr += (o->output_section->vma
					 + o->output_offset);
		    else
		      {
			/* The symbol is imported from another shared
			   library and does not apply to this one.  */
			dyn.d_un.d_ptr = 0;
		      }
		    break;
		  }
	      }
	      continue;

	    case DT_PREINIT_ARRAYSZ:
	      name = ".preinit_array";
	      goto get_out_size;
	    case DT_INIT_ARRAYSZ:
	      name = ".init_array";
	      goto get_out_size;
	    case DT_FINI_ARRAYSZ:
	      name = ".fini_array";
	    get_out_size:
	      o = bfd_get_section_by_name (abfd, name);
	      if (o == NULL)
		{
		  _bfd_error_handler
		    (_("could not find section %s"), name);
		  goto error_return;
		}
	      if (o->size == 0)
		_bfd_error_handler
		  (_("warning: %s section has zero size"), name);
	      dyn.d_un.d_val = o->size;
	      break;

	    case DT_PREINIT_ARRAY:
	      name = ".preinit_array";
	      goto get_out_vma;
	    case DT_INIT_ARRAY:
	      name = ".init_array";
	      goto get_out_vma;
	    case DT_FINI_ARRAY:
	      name = ".fini_array";
	    get_out_vma:
	      o = bfd_get_section_by_name (abfd, name);
	      goto do_vma;

	    case DT_HASH:
	      name = ".hash";
	      goto get_vma;
	    case DT_GNU_HASH:
	      name = ".gnu.hash";
	      goto get_vma;
	    case DT_STRTAB:
	      name = ".dynstr";
	      goto get_vma;
	    case DT_SYMTAB:
	      name = ".dynsym";
	      goto get_vma;
	    case DT_VERDEF:
	      name = ".gnu.version_d";
	      goto get_vma;
	    case DT_VERNEED:
	      name = ".gnu.version_r";
	      goto get_vma;
	    case DT_VERSYM:
	      name = ".gnu.version";
	    get_vma:
	      o = bfd_get_linker_section (dynobj, name);
	    do_vma:
	      if (o == NULL || bfd_is_abs_section (o->output_section))
		{
		  _bfd_error_handler
		    (_("could not find section %s"), name);
		  goto error_return;
		}
	      if (elf_section_data (o->output_section)->this_hdr.sh_type == SHT_NOTE)
		{
		  _bfd_error_handler
		    (_("warning: section '%s' is being made into a note"), name);
		  bfd_set_error (bfd_error_nonrepresentable_section);
		  goto error_return;
		}
	      dyn.d_un.d_ptr = o->output_section->vma + o->output_offset;
	      break;

	    case DT_REL:
	    case DT_RELA:
	    case DT_RELSZ:
	    case DT_RELASZ:
	      if (dyn.d_tag == DT_REL || dyn.d_tag == DT_RELSZ)
		type = SHT_REL;
	      else
		type = SHT_RELA;
	      sh_size = 0;
	      sh_addr = 0;
	      for (i = 1; i < elf_numsections (abfd); i++)
		{
		  Elf_Internal_Shdr *hdr;

		  hdr = elf_elfsections (abfd)[i];
		  if (hdr->sh_type == type
		      && (hdr->sh_flags & SHF_ALLOC) != 0)
		    {
		      sh_size += hdr->sh_size;
		      if (sh_addr == 0
			  || sh_addr > hdr->sh_addr)
			sh_addr = hdr->sh_addr;
		    }
		}

	      if (bed->dtrel_excludes_plt && htab->srelplt != NULL)
		{
		  unsigned int opb = bfd_octets_per_byte (abfd, o);

		  /* Don't count procedure linkage table relocs in the
		     overall reloc count.  */
		  sh_size -= htab->srelplt->size;
		  if (sh_size == 0)
		    /* If the size is zero, make the address zero too.
		       This is to avoid a glibc bug.  If the backend
		       emits DT_RELA/DT_RELASZ even when DT_RELASZ is
		       zero, then we'll put DT_RELA at the end of
		       DT_JMPREL.  glibc will interpret the end of
		       DT_RELA matching the end of DT_JMPREL as the
		       case where DT_RELA includes DT_JMPREL, and for
		       LD_BIND_NOW will decide that processing DT_RELA
		       will process the PLT relocs too.  Net result:
		       No PLT relocs applied.  */
		    sh_addr = 0;

		  /* If .rela.plt is the first .rela section, exclude
		     it from DT_RELA.  */
		  else if (sh_addr == (htab->srelplt->output_section->vma
				       + htab->srelplt->output_offset) * opb)
		    sh_addr += htab->srelplt->size;
		}

	      if (dyn.d_tag == DT_RELSZ || dyn.d_tag == DT_RELASZ)
		dyn.d_un.d_val = sh_size;
	      else
		dyn.d_un.d_ptr = sh_addr;
	      break;
	    }
	  bed->s->swap_dyn_out (dynobj, &dyn, dyncon);
	}
    }

  /* If we have created any dynamic sections, then output them.  */
  if (dynobj != NULL)
    {
      if (! (*bed->elf_backend_finish_dynamic_sections) (abfd, info))
	goto error_return;

      /* Check for DT_TEXTREL (late, in case the backend removes it).  */
      if (bfd_link_textrel_check (info)
	  && (o = bfd_get_linker_section (dynobj, ".dynamic")) != NULL
	  && o->size != 0)
	{
	  bfd_byte *dyncon, *dynconend;

	  dyncon = o->contents;
	  dynconend = o->contents + o->size;
	  for (; dyncon < dynconend; dyncon += bed->s->sizeof_dyn)
	    {
	      Elf_Internal_Dyn dyn;

	      bed->s->swap_dyn_in (dynobj, dyncon, &dyn);

	      if (dyn.d_tag == DT_TEXTREL)
		{
		  if (info->textrel_check == textrel_check_error)
		    info->callbacks->einfo
		      (_("%P%X: read-only segment has dynamic relocations\n"));
		  else if (bfd_link_dll (info))
		    info->callbacks->einfo
		      (_("%P: warning: creating DT_TEXTREL in a shared object\n"));
		  else if (bfd_link_pde (info))
		    info->callbacks->einfo
		      (_("%P: warning: creating DT_TEXTREL in a PDE\n"));
		  else
		    info->callbacks->einfo
		      (_("%P: warning: creating DT_TEXTREL in a PIE\n"));
		  break;
		}
	    }
	}

      for (o = dynobj->sections; o != NULL; o = o->next)
	{
	  if ((o->flags & SEC_HAS_CONTENTS) == 0
	      || o->size == 0
	      || o->output_section == bfd_abs_section_ptr)
	    continue;
	  if ((o->flags & SEC_LINKER_CREATED) == 0)
	    {
	      /* At this point, we are only interested in sections
		 created by _bfd_elf_link_create_dynamic_sections.  */
	      continue;
	    }
	  if (htab->stab_info.stabstr == o)
	    continue;
	  if (htab->eh_info.hdr_sec == o)
	    continue;
	  if (strcmp (o->name, ".dynstr") != 0)
	    {
	      bfd_size_type octets = ((file_ptr) o->output_offset
				      * bfd_octets_per_byte (abfd, o));
	      if (!bfd_set_section_contents (abfd, o->output_section,
					     o->contents, octets, o->size))
		goto error_return;
	    }
	  else
	    {
	      /* The contents of the .dynstr section are actually in a
		 stringtab.  */
	      file_ptr off;

	      off = elf_section_data (o->output_section)->this_hdr.sh_offset;
	      if (bfd_seek (abfd, off, SEEK_SET) != 0
		  || !_bfd_elf_strtab_emit (abfd, htab->dynstr))
		goto error_return;
	    }
	}
    }

  if (!info->resolve_section_groups)
    {
      bool failed = false;

      BFD_ASSERT (bfd_link_relocatable (info));
      bfd_map_over_sections (abfd, bfd_elf_set_group_contents, &failed);
      if (failed)
	goto error_return;
    }

  /* If we have optimized stabs strings, output them.  */
  if (htab->stab_info.stabstr != NULL)
    {
      if (!_bfd_write_stab_strings (abfd, &htab->stab_info))
	goto error_return;
    }

  if (! _bfd_elf_write_section_eh_frame_hdr (abfd, info))
    goto error_return;

  if (! _bfd_elf_write_section_sframe (abfd, info))
    goto error_return;

  if (info->callbacks->emit_ctf)
      info->callbacks->emit_ctf ();

  elf_final_link_free (abfd, &flinfo);

  if (attr_section)
    {
      bfd_byte *contents = (bfd_byte *) bfd_malloc (attr_size);
      if (contents == NULL)
	{
	  /* Bail out and fail.  */
	  ret = false;
	  goto return_local_hash_table;
	}
      bfd_elf_set_obj_attr_contents (abfd, contents, attr_size);
      bfd_set_section_contents (abfd, attr_section, contents, 0, attr_size);
      free (contents);
    }

 return_local_hash_table:
  if (info->unique_symbol)
    bfd_hash_table_free (&flinfo.local_hash_table);
  return ret;

 error_return:
  elf_final_link_free (abfd, &flinfo);
  ret = false;
  goto return_local_hash_table;
}

/* Initialize COOKIE for input bfd ABFD.  */

static bool
init_reloc_cookie (struct elf_reloc_cookie *cookie,
		   struct bfd_link_info *info, bfd *abfd)
{
  Elf_Internal_Shdr *symtab_hdr;
  const struct elf_backend_data *bed;

  bed = get_elf_backend_data (abfd);
  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;

  cookie->abfd = abfd;
  cookie->sym_hashes = elf_sym_hashes (abfd);
  cookie->bad_symtab = elf_bad_symtab (abfd);
  if (cookie->bad_symtab)
    {
      cookie->locsymcount = symtab_hdr->sh_size / bed->s->sizeof_sym;
      cookie->extsymoff = 0;
    }
  else
    {
      cookie->locsymcount = symtab_hdr->sh_info;
      cookie->extsymoff = symtab_hdr->sh_info;
    }

  if (bed->s->arch_size == 32)
    cookie->r_sym_shift = 8;
  else
    cookie->r_sym_shift = 32;

  cookie->locsyms = (Elf_Internal_Sym *) symtab_hdr->contents;
  if (cookie->locsyms == NULL && cookie->locsymcount != 0)
    {
      cookie->locsyms = bfd_elf_get_elf_syms (abfd, symtab_hdr,
					      cookie->locsymcount, 0,
					      NULL, NULL, NULL);
      if (cookie->locsyms == NULL)
	{
	  info->callbacks->einfo (_("%P%X: can not read symbols: %E\n"));
	  return false;
	}
      if (_bfd_link_keep_memory (info) )
	{
	  symtab_hdr->contents = (bfd_byte *) cookie->locsyms;
	  info->cache_size += (cookie->locsymcount
			       * sizeof (Elf_External_Sym_Shndx));
	}
    }
  return true;
}

/* Free the memory allocated by init_reloc_cookie, if appropriate.  */

static void
fini_reloc_cookie (struct elf_reloc_cookie *cookie, bfd *abfd)
{
  Elf_Internal_Shdr *symtab_hdr;

  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
  if (symtab_hdr->contents != (unsigned char *) cookie->locsyms)
    free (cookie->locsyms);
}

/* Initialize the relocation information in COOKIE for input section SEC
   of input bfd ABFD.  */

static bool
init_reloc_cookie_rels (struct elf_reloc_cookie *cookie,
			struct bfd_link_info *info, bfd *abfd,
			asection *sec)
{
  if (sec->reloc_count == 0)
    {
      cookie->rels = NULL;
      cookie->relend = NULL;
    }
  else
    {
      cookie->rels = _bfd_elf_link_info_read_relocs (abfd, info, sec,
						     NULL, NULL,
						     _bfd_link_keep_memory (info));
      if (cookie->rels == NULL)
	return false;
      cookie->rel = cookie->rels;
      cookie->relend = cookie->rels + sec->reloc_count;
    }
  cookie->rel = cookie->rels;
  return true;
}

/* Free the memory allocated by init_reloc_cookie_rels,
   if appropriate.  */

static void
fini_reloc_cookie_rels (struct elf_reloc_cookie *cookie,
			asection *sec)
{
  if (elf_section_data (sec)->relocs != cookie->rels)
    free (cookie->rels);
}

/* Initialize the whole of COOKIE for input section SEC.  */

static bool
init_reloc_cookie_for_section (struct elf_reloc_cookie *cookie,
			       struct bfd_link_info *info,
			       asection *sec)
{
  if (!init_reloc_cookie (cookie, info, sec->owner))
    goto error1;
  if (!init_reloc_cookie_rels (cookie, info, sec->owner, sec))
    goto error2;
  return true;

 error2:
  fini_reloc_cookie (cookie, sec->owner);
 error1:
  return false;
}

/* Free the memory allocated by init_reloc_cookie_for_section,
   if appropriate.  */

static void
fini_reloc_cookie_for_section (struct elf_reloc_cookie *cookie,
			       asection *sec)
{
  fini_reloc_cookie_rels (cookie, sec);
  fini_reloc_cookie (cookie, sec->owner);
}

/* Garbage collect unused sections.  */

/* Default gc_mark_hook.  */

asection *
_bfd_elf_gc_mark_hook (asection *sec,
		       struct bfd_link_info *info ATTRIBUTE_UNUSED,
		       Elf_Internal_Rela *rel ATTRIBUTE_UNUSED,
		       struct elf_link_hash_entry *h,
		       Elf_Internal_Sym *sym)
{
  if (h != NULL)
    {
      switch (h->root.type)
	{
	case bfd_link_hash_defined:
	case bfd_link_hash_defweak:
	  return h->root.u.def.section;

	case bfd_link_hash_common:
	  return h->root.u.c.p->section;

	default:
	  break;
	}
    }
  else
    return bfd_section_from_elf_index (sec->owner, sym->st_shndx);

  return NULL;
}

/* Return the debug definition section.  */

static asection *
elf_gc_mark_debug_section (asection *sec ATTRIBUTE_UNUSED,
			   struct bfd_link_info *info ATTRIBUTE_UNUSED,
			   Elf_Internal_Rela *rel ATTRIBUTE_UNUSED,
			   struct elf_link_hash_entry *h,
			   Elf_Internal_Sym *sym)
{
  if (h != NULL)
    {
      /* Return the global debug definition section.  */
      if ((h->root.type == bfd_link_hash_defined
	   || h->root.type == bfd_link_hash_defweak)
	  && (h->root.u.def.section->flags & SEC_DEBUGGING) != 0)
	return h->root.u.def.section;
    }
  else
    {
      /* Return the local debug definition section.  */
      asection *isec = bfd_section_from_elf_index (sec->owner,
						   sym->st_shndx);
      if ((isec->flags & SEC_DEBUGGING) != 0)
	return isec;
    }

  return NULL;
}

/* COOKIE->rel describes a relocation against section SEC, which is
   a section we've decided to keep.  Return the section that contains
   the relocation symbol, or NULL if no section contains it.  */

asection *
_bfd_elf_gc_mark_rsec (struct bfd_link_info *info, asection *sec,
		       elf_gc_mark_hook_fn gc_mark_hook,
		       struct elf_reloc_cookie *cookie,
		       bool *start_stop)
{
  unsigned long r_symndx;
  struct elf_link_hash_entry *h, *hw;

  r_symndx = cookie->rel->r_info >> cookie->r_sym_shift;
  if (r_symndx == STN_UNDEF)
    return NULL;

  if (r_symndx >= cookie->locsymcount
      || ELF_ST_BIND (cookie->locsyms[r_symndx].st_info) != STB_LOCAL)
    {
      bool was_marked;

      h = cookie->sym_hashes[r_symndx - cookie->extsymoff];
      if (h == NULL)
	{
	  info->callbacks->einfo (_("%F%P: corrupt input: %pB\n"),
				  sec->owner);
	  return NULL;
	}
      while (h->root.type == bfd_link_hash_indirect
	     || h->root.type == bfd_link_hash_warning)
	h = (struct elf_link_hash_entry *) h->root.u.i.link;

      was_marked = h->mark;
      h->mark = 1;
      /* Keep all aliases of the symbol too.  If an object symbol
	 needs to be copied into .dynbss then all of its aliases
	 should be present as dynamic symbols, not just the one used
	 on the copy relocation.  */
      hw = h;
      while (hw->is_weakalias)
	{
	  hw = hw->u.alias;
	  hw->mark = 1;
	}

      if (!was_marked && h->start_stop && !h->root.ldscript_def)
	{
	  if (info->start_stop_gc)
	    return NULL;

	  /* To work around a glibc bug, mark XXX input sections
	     when there is a reference to __start_XXX or __stop_XXX
	     symbols.  */
	  else if (start_stop != NULL)
	    {
	      asection *s = h->u2.start_stop_section;
	      *start_stop = true;
	      return s;
	    }
	}

      return (*gc_mark_hook) (sec, info, cookie->rel, h, NULL);
    }

  return (*gc_mark_hook) (sec, info, cookie->rel, NULL,
			  &cookie->locsyms[r_symndx]);
}

/* COOKIE->rel describes a relocation against section SEC, which is
   a section we've decided to keep.  Mark the section that contains
   the relocation symbol.  */

bool
_bfd_elf_gc_mark_reloc (struct bfd_link_info *info,
			asection *sec,
			elf_gc_mark_hook_fn gc_mark_hook,
			struct elf_reloc_cookie *cookie)
{
  asection *rsec;
  bool start_stop = false;

  rsec = _bfd_elf_gc_mark_rsec (info, sec, gc_mark_hook, cookie, &start_stop);
  while (rsec != NULL)
    {
      if (!rsec->gc_mark)
	{
	  if (bfd_get_flavour (rsec->owner) != bfd_target_elf_flavour
	      || (rsec->owner->flags & DYNAMIC) != 0)
	    rsec->gc_mark = 1;
	  else if (!_bfd_elf_gc_mark (info, rsec, gc_mark_hook))
	    return false;
	}
      if (!start_stop)
	break;
      rsec = bfd_get_next_section_by_name (rsec->owner, rsec);
    }
  return true;
}

/* The mark phase of garbage collection.  For a given section, mark
   it and any sections in this section's group, and all the sections
   which define symbols to which it refers.  */

bool
_bfd_elf_gc_mark (struct bfd_link_info *info,
		  asection *sec,
		  elf_gc_mark_hook_fn gc_mark_hook)
{
  bool ret;
  asection *group_sec, *eh_frame;

  sec->gc_mark = 1;

  /* Mark all the sections in the group.  */
  group_sec = elf_section_data (sec)->next_in_group;
  if (group_sec && !group_sec->gc_mark)
    if (!_bfd_elf_gc_mark (info, group_sec, gc_mark_hook))
      return false;

  /* Look through the section relocs.  */
  ret = true;
  eh_frame = elf_eh_frame_section (sec->owner);
  if ((sec->flags & SEC_RELOC) != 0
      && sec->reloc_count > 0
      && sec != eh_frame)
    {
      struct elf_reloc_cookie cookie;

      if (!init_reloc_cookie_for_section (&cookie, info, sec))
	ret = false;
      else
	{
	  for (; cookie.rel < cookie.relend; cookie.rel++)
	    if (!_bfd_elf_gc_mark_reloc (info, sec, gc_mark_hook, &cookie))
	      {
		ret = false;
		break;
	      }
	  fini_reloc_cookie_for_section (&cookie, sec);
	}
    }

  if (ret && eh_frame && elf_fde_list (sec))
    {
      struct elf_reloc_cookie cookie;

      if (!init_reloc_cookie_for_section (&cookie, info, eh_frame))
	ret = false;
      else
	{
	  if (!_bfd_elf_gc_mark_fdes (info, sec, eh_frame,
				      gc_mark_hook, &cookie))
	    ret = false;
	  fini_reloc_cookie_for_section (&cookie, eh_frame);
	}
    }

  eh_frame = elf_section_eh_frame_entry (sec);
  if (ret && eh_frame && !eh_frame->gc_mark)
    if (!_bfd_elf_gc_mark (info, eh_frame, gc_mark_hook))
      ret = false;

  return ret;
}

/* Scan and mark sections in a special or debug section group.  */

static void
_bfd_elf_gc_mark_debug_special_section_group (asection *grp)
{
  /* Point to first section of section group.  */
  asection *ssec;
  /* Used to iterate the section group.  */
  asection *msec;

  bool is_special_grp = true;
  bool is_debug_grp = true;

  /* First scan to see if group contains any section other than debug
     and special section.  */
  ssec = msec = elf_next_in_group (grp);
  do
    {
      if ((msec->flags & SEC_DEBUGGING) == 0)
	is_debug_grp = false;

      if ((msec->flags & (SEC_ALLOC | SEC_LOAD | SEC_RELOC)) != 0)
	is_special_grp = false;

      msec = elf_next_in_group (msec);
    }
  while (msec != ssec);

  /* If this is a pure debug section group or pure special section group,
     keep all sections in this group.  */
  if (is_debug_grp || is_special_grp)
    {
      do
	{
	  msec->gc_mark = 1;
	  msec = elf_next_in_group (msec);
	}
      while (msec != ssec);
    }
}

/* Keep debug and special sections.  */

bool
_bfd_elf_gc_mark_extra_sections (struct bfd_link_info *info,
				 elf_gc_mark_hook_fn mark_hook)
{
  bfd *ibfd;

  for (ibfd = info->input_bfds; ibfd != NULL; ibfd = ibfd->link.next)
    {
      asection *isec;
      bool some_kept;
      bool debug_frag_seen;
      bool has_kept_debug_info;

      if (bfd_get_flavour (ibfd) != bfd_target_elf_flavour)
	continue;
      isec = ibfd->sections;
      if (isec == NULL || isec->sec_info_type == SEC_INFO_TYPE_JUST_SYMS)
	continue;

      /* Ensure all linker created sections are kept,
	 see if any other section is already marked,
	 and note if we have any fragmented debug sections.  */
      debug_frag_seen = some_kept = has_kept_debug_info = false;
      for (isec = ibfd->sections; isec != NULL; isec = isec->next)
	{
	  if ((isec->flags & SEC_LINKER_CREATED) != 0)
	    isec->gc_mark = 1;
	  else if (isec->gc_mark
		   && (isec->flags & SEC_ALLOC) != 0
		   && elf_section_type (isec) != SHT_NOTE)
	    some_kept = true;
	  else
	    {
	      /* Since all sections, except for backend specific ones,
		 have been garbage collected, call mark_hook on this
		 section if any of its linked-to sections is marked.  */
	      asection *linked_to_sec;
	      for (linked_to_sec = elf_linked_to_section (isec);
		   linked_to_sec != NULL && !linked_to_sec->linker_mark;
		   linked_to_sec = elf_linked_to_section (linked_to_sec))
		{
		  if (linked_to_sec->gc_mark)
		    {
		      if (!_bfd_elf_gc_mark (info, isec, mark_hook))
			return false;
		      break;
		    }
		  linked_to_sec->linker_mark = 1;
		}
	      for (linked_to_sec = elf_linked_to_section (isec);
		   linked_to_sec != NULL && linked_to_sec->linker_mark;
		   linked_to_sec = elf_linked_to_section (linked_to_sec))
		linked_to_sec->linker_mark = 0;
	    }

	  if (!debug_frag_seen
	      && (isec->flags & SEC_DEBUGGING)
	      && startswith (isec->name, ".debug_line."))
	    debug_frag_seen = true;
	  else if (strcmp (bfd_section_name (isec),
			   "__patchable_function_entries") == 0
		   && elf_linked_to_section (isec) == NULL)
	      info->callbacks->einfo (_("%F%P: %pB(%pA): error: "
					"need linked-to section "
					"for --gc-sections\n"),
				      isec->owner, isec);
	}

      /* If no non-note alloc section in this file will be kept, then
	 we can toss out the debug and special sections.  */
      if (!some_kept)
	continue;

      /* Keep debug and special sections like .comment when they are
	 not part of a group.  Also keep section groups that contain
	 just debug sections or special sections.  NB: Sections with
	 linked-to section has been handled above.  */
      for (isec = ibfd->sections; isec != NULL; isec = isec->next)
	{
	  if ((isec->flags & SEC_GROUP) != 0)
	    _bfd_elf_gc_mark_debug_special_section_group (isec);
	  else if (((isec->flags & SEC_DEBUGGING) != 0
		    || (isec->flags & (SEC_ALLOC | SEC_LOAD | SEC_RELOC)) == 0)
		   && elf_next_in_group (isec) == NULL
		   && elf_linked_to_section (isec) == NULL)
	    isec->gc_mark = 1;
	  if (isec->gc_mark && (isec->flags & SEC_DEBUGGING) != 0)
	    has_kept_debug_info = true;
	}

      /* Look for CODE sections which are going to be discarded,
	 and find and discard any fragmented debug sections which
	 are associated with that code section.  */
      if (debug_frag_seen)
	for (isec = ibfd->sections; isec != NULL; isec = isec->next)
	  if ((isec->flags & SEC_CODE) != 0
	      && isec->gc_mark == 0)
	    {
	      unsigned int ilen;
	      asection *dsec;

	      ilen = strlen (isec->name);

	      /* Association is determined by the name of the debug
		 section containing the name of the code section as
		 a suffix.  For example .debug_line.text.foo is a
		 debug section associated with .text.foo.  */
	      for (dsec = ibfd->sections; dsec != NULL; dsec = dsec->next)
		{
		  unsigned int dlen;

		  if (dsec->gc_mark == 0
		      || (dsec->flags & SEC_DEBUGGING) == 0)
		    continue;

		  dlen = strlen (dsec->name);

		  if (dlen > ilen
		      && strncmp (dsec->name + (dlen - ilen),
				  isec->name, ilen) == 0)
		    dsec->gc_mark = 0;
		}
	  }

      /* Mark debug sections referenced by kept debug sections.  */
      if (has_kept_debug_info)
	for (isec = ibfd->sections; isec != NULL; isec = isec->next)
	  if (isec->gc_mark
	      && (isec->flags & SEC_DEBUGGING) != 0)
	    if (!_bfd_elf_gc_mark (info, isec,
				   elf_gc_mark_debug_section))
	      return false;
    }
  return true;
}

static bool
elf_gc_sweep (bfd *abfd, struct bfd_link_info *info)
{
  bfd *sub;
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);

  for (sub = info->input_bfds; sub != NULL; sub = sub->link.next)
    {
      asection *o;

      if (bfd_get_flavour (sub) != bfd_target_elf_flavour
	  || elf_object_id (sub) != elf_hash_table_id (elf_hash_table (info))
	  || !(*bed->relocs_compatible) (sub->xvec, abfd->xvec))
	continue;
      o = sub->sections;
      if (o == NULL || o->sec_info_type == SEC_INFO_TYPE_JUST_SYMS)
	continue;

      for (o = sub->sections; o != NULL; o = o->next)
	{
	  /* When any section in a section group is kept, we keep all
	     sections in the section group.  If the first member of
	     the section group is excluded, we will also exclude the
	     group section.  */
	  if (o->flags & SEC_GROUP)
	    {
	      asection *first = elf_next_in_group (o);
	      o->gc_mark = first->gc_mark;
	    }

	  if (o->gc_mark)
	    continue;

	  /* Skip sweeping sections already excluded.  */
	  if (o->flags & SEC_EXCLUDE)
	    continue;

	  /* Since this is early in the link process, it is simple
	     to remove a section from the output.  */
	  o->flags |= SEC_EXCLUDE;

	  if (info->print_gc_sections && o->size != 0)
	    /* xgettext:c-format */
	    _bfd_error_handler (_("removing unused section '%pA' in file '%pB'"),
				o, sub);
	}
    }

  return true;
}

/* Propagate collected vtable information.  This is called through
   elf_link_hash_traverse.  */

static bool
elf_gc_propagate_vtable_entries_used (struct elf_link_hash_entry *h, void *okp)
{
  /* Those that are not vtables.  */
  if (h->start_stop
      || h->u2.vtable == NULL
      || h->u2.vtable->parent == NULL)
    return true;

  /* Those vtables that do not have parents, we cannot merge.  */
  if (h->u2.vtable->parent == (struct elf_link_hash_entry *) -1)
    return true;

  /* If we've already been done, exit.  */
  if (h->u2.vtable->used && h->u2.vtable->used[-1])
    return true;

  /* Make sure the parent's table is up to date.  */
  elf_gc_propagate_vtable_entries_used (h->u2.vtable->parent, okp);

  if (h->u2.vtable->used == NULL)
    {
      /* None of this table's entries were referenced.  Re-use the
	 parent's table.  */
      h->u2.vtable->used = h->u2.vtable->parent->u2.vtable->used;
      h->u2.vtable->size = h->u2.vtable->parent->u2.vtable->size;
    }
  else
    {
      size_t n;
      bool *cu, *pu;

      /* Or the parent's entries into ours.  */
      cu = h->u2.vtable->used;
      cu[-1] = true;
      pu = h->u2.vtable->parent->u2.vtable->used;
      if (pu != NULL)
	{
	  const struct elf_backend_data *bed;
	  unsigned int log_file_align;

	  bed = get_elf_backend_data (h->root.u.def.section->owner);
	  log_file_align = bed->s->log_file_align;
	  n = h->u2.vtable->parent->u2.vtable->size >> log_file_align;
	  while (n--)
	    {
	      if (*pu)
		*cu = true;
	      pu++;
	      cu++;
	    }
	}
    }

  return true;
}

struct link_info_ok
{
  struct bfd_link_info *info;
  bool ok;
};

static bool
elf_gc_smash_unused_vtentry_relocs (struct elf_link_hash_entry *h,
				    void *ptr)
{
  asection *sec;
  bfd_vma hstart, hend;
  Elf_Internal_Rela *relstart, *relend, *rel;
  const struct elf_backend_data *bed;
  unsigned int log_file_align;
  struct link_info_ok *info = (struct link_info_ok *) ptr;

  /* Take care of both those symbols that do not describe vtables as
     well as those that are not loaded.  */
  if (h->start_stop
      || h->u2.vtable == NULL
      || h->u2.vtable->parent == NULL)
    return true;

  BFD_ASSERT (h->root.type == bfd_link_hash_defined
	      || h->root.type == bfd_link_hash_defweak);

  sec = h->root.u.def.section;
  hstart = h->root.u.def.value;
  hend = hstart + h->size;

  relstart = _bfd_elf_link_info_read_relocs (sec->owner, info->info,
					     sec, NULL, NULL, true);
  if (!relstart)
    return info->ok = false;
  bed = get_elf_backend_data (sec->owner);
  log_file_align = bed->s->log_file_align;

  relend = relstart + sec->reloc_count;

  for (rel = relstart; rel < relend; ++rel)
    if (rel->r_offset >= hstart && rel->r_offset < hend)
      {
	/* If the entry is in use, do nothing.  */
	if (h->u2.vtable->used
	    && (rel->r_offset - hstart) < h->u2.vtable->size)
	  {
	    bfd_vma entry = (rel->r_offset - hstart) >> log_file_align;
	    if (h->u2.vtable->used[entry])
	      continue;
	  }
	/* Otherwise, kill it.  */
	rel->r_offset = rel->r_info = rel->r_addend = 0;
      }

  return true;
}

/* Mark sections containing dynamically referenced symbols.  When
   building shared libraries, we must assume that any visible symbol is
   referenced.  */

bool
bfd_elf_gc_mark_dynamic_ref_symbol (struct elf_link_hash_entry *h, void *inf)
{
  struct bfd_link_info *info = (struct bfd_link_info *) inf;
  struct bfd_elf_dynamic_list *d = info->dynamic_list;

  if ((h->root.type == bfd_link_hash_defined
       || h->root.type == bfd_link_hash_defweak)
      && (!h->start_stop
	  || h->root.ldscript_def
	  || !info->start_stop_gc)
      && ((h->ref_dynamic && !h->forced_local)
	  || ((h->def_regular || ELF_COMMON_DEF_P (h))
	      && ELF_ST_VISIBILITY (h->other) != STV_INTERNAL
	      && ELF_ST_VISIBILITY (h->other) != STV_HIDDEN
	      && (!bfd_link_executable (info)
		  || info->gc_keep_exported
		  || info->export_dynamic
		  || (h->dynamic
		      && d != NULL
		      && (*d->match) (&d->head, NULL, h->root.root.string)))
	      && (h->versioned >= versioned
		  || !bfd_hide_sym_by_version (info->version_info,
					       h->root.root.string)))))
    h->root.u.def.section->flags |= SEC_KEEP;

  return true;
}

/* Keep all sections containing symbols undefined on the command-line,
   and the section containing the entry symbol.  */

void
_bfd_elf_gc_keep (struct bfd_link_info *info)
{
  struct bfd_sym_chain *sym;

  for (sym = info->gc_sym_list; sym != NULL; sym = sym->next)
    {
      struct elf_link_hash_entry *h;

      h = elf_link_hash_lookup (elf_hash_table (info), sym->name,
				false, false, false);

      if (h != NULL
	  && (h->root.type == bfd_link_hash_defined
	      || h->root.type == bfd_link_hash_defweak)
	  && !bfd_is_const_section (h->root.u.def.section))
	h->root.u.def.section->flags |= SEC_KEEP;
    }
}

bool
bfd_elf_parse_eh_frame_entries (bfd *abfd ATTRIBUTE_UNUSED,
				struct bfd_link_info *info)
{
  bfd *ibfd = info->input_bfds;

  for (ibfd = info->input_bfds; ibfd != NULL; ibfd = ibfd->link.next)
    {
      asection *sec;
      struct elf_reloc_cookie cookie;

      if (bfd_get_flavour (ibfd) != bfd_target_elf_flavour)
	continue;
      sec = ibfd->sections;
      if (sec == NULL || sec->sec_info_type == SEC_INFO_TYPE_JUST_SYMS)
	continue;

      if (!init_reloc_cookie (&cookie, info, ibfd))
	return false;

      for (sec = ibfd->sections; sec; sec = sec->next)
	{
	  if (startswith (bfd_section_name (sec), ".eh_frame_entry")
	      && init_reloc_cookie_rels (&cookie, info, ibfd, sec))
	    {
	      _bfd_elf_parse_eh_frame_entry (info, sec, &cookie);
	      fini_reloc_cookie_rels (&cookie, sec);
	    }
	}
    }
  return true;
}

/* Do mark and sweep of unused sections.  */

bool
bfd_elf_gc_sections (bfd *abfd, struct bfd_link_info *info)
{
  bool ok = true;
  bfd *sub;
  elf_gc_mark_hook_fn gc_mark_hook;
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  struct elf_link_hash_table *htab;
  struct link_info_ok info_ok;

  if (!bed->can_gc_sections
      || !is_elf_hash_table (info->hash))
    {
      _bfd_error_handler(_("warning: gc-sections option ignored"));
      return true;
    }

  bed->gc_keep (info);
  htab = elf_hash_table (info);

  /* Try to parse each bfd's .eh_frame section.  Point elf_eh_frame_section
     at the .eh_frame section if we can mark the FDEs individually.  */
  for (sub = info->input_bfds;
       info->eh_frame_hdr_type != COMPACT_EH_HDR && sub != NULL;
       sub = sub->link.next)
    {
      asection *sec;
      struct elf_reloc_cookie cookie;

      sec = sub->sections;
      if (sec == NULL || sec->sec_info_type == SEC_INFO_TYPE_JUST_SYMS)
	continue;
      sec = bfd_get_section_by_name (sub, ".eh_frame");
      while (sec && init_reloc_cookie_for_section (&cookie, info, sec))
	{
	  _bfd_elf_parse_eh_frame (sub, info, sec, &cookie);
	  if (elf_section_data (sec)->sec_info
	      && (sec->flags & SEC_LINKER_CREATED) == 0)
	    elf_eh_frame_section (sub) = sec;
	  fini_reloc_cookie_for_section (&cookie, sec);
	  sec = bfd_get_next_section_by_name (NULL, sec);
	}
    }

  /* Apply transitive closure to the vtable entry usage info.  */
  elf_link_hash_traverse (htab, elf_gc_propagate_vtable_entries_used, &ok);
  if (!ok)
    return false;

  /* Kill the vtable relocations that were not used.  */
  info_ok.info = info;
  info_ok.ok = true;
  elf_link_hash_traverse (htab, elf_gc_smash_unused_vtentry_relocs, &info_ok);
  if (!info_ok.ok)
    return false;

  /* Mark dynamically referenced symbols.  */
  if (htab->dynamic_sections_created || info->gc_keep_exported)
    elf_link_hash_traverse (htab, bed->gc_mark_dynamic_ref, info);

  /* Grovel through relocs to find out who stays ...  */
  gc_mark_hook = bed->gc_mark_hook;
  for (sub = info->input_bfds; sub != NULL; sub = sub->link.next)
    {
      asection *o;

      if (bfd_get_flavour (sub) != bfd_target_elf_flavour
	  || elf_object_id (sub) != elf_hash_table_id (htab)
	  || !(*bed->relocs_compatible) (sub->xvec, abfd->xvec))
	continue;

      o = sub->sections;
      if (o == NULL || o->sec_info_type == SEC_INFO_TYPE_JUST_SYMS)
	continue;

      /* Start at sections marked with SEC_KEEP (ref _bfd_elf_gc_keep).
	 Also treat note sections as a root, if the section is not part
	 of a group.  We must keep all PREINIT_ARRAY, INIT_ARRAY as
	 well as FINI_ARRAY sections for ld -r.  */
      for (o = sub->sections; o != NULL; o = o->next)
	if (!o->gc_mark
	    && (o->flags & SEC_EXCLUDE) == 0
	    && ((o->flags & SEC_KEEP) != 0
		|| (bfd_link_relocatable (info)
		    && ((elf_section_data (o)->this_hdr.sh_type
			 == SHT_PREINIT_ARRAY)
			|| (elf_section_data (o)->this_hdr.sh_type
			    == SHT_INIT_ARRAY)
			|| (elf_section_data (o)->this_hdr.sh_type
			    == SHT_FINI_ARRAY)))
		|| (elf_section_data (o)->this_hdr.sh_type == SHT_NOTE
		    && elf_next_in_group (o) == NULL
		    && elf_linked_to_section (o) == NULL)
		|| ((elf_tdata (sub)->has_gnu_osabi & elf_gnu_osabi_retain)
		    && (elf_section_flags (o) & SHF_GNU_RETAIN))))
	  {
	    if (!_bfd_elf_gc_mark (info, o, gc_mark_hook))
	      return false;
	  }
    }

  /* Allow the backend to mark additional target specific sections.  */
  bed->gc_mark_extra_sections (info, gc_mark_hook);

  /* ... and mark SEC_EXCLUDE for those that go.  */
  return elf_gc_sweep (abfd, info);
}

/* Called from check_relocs to record the existence of a VTINHERIT reloc.  */

bool
bfd_elf_gc_record_vtinherit (bfd *abfd,
			     asection *sec,
			     struct elf_link_hash_entry *h,
			     bfd_vma offset)
{
  struct elf_link_hash_entry **sym_hashes, **sym_hashes_end;
  struct elf_link_hash_entry **search, *child;
  size_t extsymcount;
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);

  /* The sh_info field of the symtab header tells us where the
     external symbols start.  We don't care about the local symbols at
     this point.  */
  extsymcount = elf_tdata (abfd)->symtab_hdr.sh_size / bed->s->sizeof_sym;
  if (!elf_bad_symtab (abfd))
    extsymcount -= elf_tdata (abfd)->symtab_hdr.sh_info;

  sym_hashes = elf_sym_hashes (abfd);
  sym_hashes_end = PTR_ADD (sym_hashes, extsymcount);

  /* Hunt down the child symbol, which is in this section at the same
     offset as the relocation.  */
  for (search = sym_hashes; search != sym_hashes_end; ++search)
    {
      if ((child = *search) != NULL
	  && (child->root.type == bfd_link_hash_defined
	      || child->root.type == bfd_link_hash_defweak)
	  && child->root.u.def.section == sec
	  && child->root.u.def.value == offset)
	goto win;
    }

  /* xgettext:c-format */
  _bfd_error_handler (_("%pB: %pA+%#" PRIx64 ": no symbol found for INHERIT"),
		      abfd, sec, (uint64_t) offset);
  bfd_set_error (bfd_error_invalid_operation);
  return false;

 win:
  if (!child->u2.vtable)
    {
      child->u2.vtable = ((struct elf_link_virtual_table_entry *)
			  bfd_zalloc (abfd, sizeof (*child->u2.vtable)));
      if (!child->u2.vtable)
	return false;
    }
  if (!h)
    {
      /* This *should* only be the absolute section.  It could potentially
	 be that someone has defined a non-global vtable though, which
	 would be bad.  It isn't worth paging in the local symbols to be
	 sure though; that case should simply be handled by the assembler.  */

      child->u2.vtable->parent = (struct elf_link_hash_entry *) -1;
    }
  else
    child->u2.vtable->parent = h;

  return true;
}

/* Called from check_relocs to record the existence of a VTENTRY reloc.  */

bool
bfd_elf_gc_record_vtentry (bfd *abfd, asection *sec,
			   struct elf_link_hash_entry *h,
			   bfd_vma addend)
{
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  unsigned int log_file_align = bed->s->log_file_align;

  if (!h)
    {
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: section '%pA': corrupt VTENTRY entry"),
			  abfd, sec);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  if (!h->u2.vtable)
    {
      h->u2.vtable = ((struct elf_link_virtual_table_entry *)
		      bfd_zalloc (abfd, sizeof (*h->u2.vtable)));
      if (!h->u2.vtable)
	return false;
    }

  if (addend >= h->u2.vtable->size)
    {
      size_t size, bytes, file_align;
      bool *ptr = h->u2.vtable->used;

      /* While the symbol is undefined, we have to be prepared to handle
	 a zero size.  */
      file_align = 1 << log_file_align;
      if (h->root.type == bfd_link_hash_undefined)
	size = addend + file_align;
      else
	{
	  size = h->size;
	  if (addend >= size)
	    {
	      /* Oops!  We've got a reference past the defined end of
		 the table.  This is probably a bug -- shall we warn?  */
	      size = addend + file_align;
	    }
	}
      size = (size + file_align - 1) & -file_align;

      /* Allocate one extra entry for use as a "done" flag for the
	 consolidation pass.  */
      bytes = ((size >> log_file_align) + 1) * sizeof (bool);

      if (ptr)
	{
	  ptr = (bool *) bfd_realloc (ptr - 1, bytes);

	  if (ptr != NULL)
	    {
	      size_t oldbytes;

	      oldbytes = (((h->u2.vtable->size >> log_file_align) + 1)
			  * sizeof (bool));
	      memset (((char *) ptr) + oldbytes, 0, bytes - oldbytes);
	    }
	}
      else
	ptr = (bool *) bfd_zmalloc (bytes);

      if (ptr == NULL)
	return false;

      /* And arrange for that done flag to be at index -1.  */
      h->u2.vtable->used = ptr + 1;
      h->u2.vtable->size = size;
    }

  h->u2.vtable->used[addend >> log_file_align] = true;

  return true;
}

/* Map an ELF section header flag to its corresponding string.  */
typedef struct
{
  char *flag_name;
  flagword flag_value;
} elf_flags_to_name_table;

static const elf_flags_to_name_table elf_flags_to_names [] =
{
  { "SHF_WRITE", SHF_WRITE },
  { "SHF_ALLOC", SHF_ALLOC },
  { "SHF_EXECINSTR", SHF_EXECINSTR },
  { "SHF_MERGE", SHF_MERGE },
  { "SHF_STRINGS", SHF_STRINGS },
  { "SHF_INFO_LINK", SHF_INFO_LINK},
  { "SHF_LINK_ORDER", SHF_LINK_ORDER},
  { "SHF_OS_NONCONFORMING", SHF_OS_NONCONFORMING},
  { "SHF_GROUP", SHF_GROUP },
  { "SHF_TLS", SHF_TLS },
  { "SHF_MASKOS", SHF_MASKOS },
  { "SHF_EXCLUDE", SHF_EXCLUDE },
};

/* Returns TRUE if the section is to be included, otherwise FALSE.  */
bool
bfd_elf_lookup_section_flags (struct bfd_link_info *info,
			      struct flag_info *flaginfo,
			      asection *section)
{
  const bfd_vma sh_flags = elf_section_flags (section);

  if (!flaginfo->flags_initialized)
    {
      bfd *obfd = info->output_bfd;
      const struct elf_backend_data *bed = get_elf_backend_data (obfd);
      struct flag_info_list *tf = flaginfo->flag_list;
      int with_hex = 0;
      int without_hex = 0;

      for (tf = flaginfo->flag_list; tf != NULL; tf = tf->next)
	{
	  unsigned i;
	  flagword (*lookup) (char *);

	  lookup = bed->elf_backend_lookup_section_flags_hook;
	  if (lookup != NULL)
	    {
	      flagword hexval = (*lookup) ((char *) tf->name);

	      if (hexval != 0)
		{
		  if (tf->with == with_flags)
		    with_hex |= hexval;
		  else if (tf->with == without_flags)
		    without_hex |= hexval;
		  tf->valid = true;
		  continue;
		}
	    }
	  for (i = 0; i < ARRAY_SIZE (elf_flags_to_names); ++i)
	    {
	      if (strcmp (tf->name, elf_flags_to_names[i].flag_name) == 0)
		{
		  if (tf->with == with_flags)
		    with_hex |= elf_flags_to_names[i].flag_value;
		  else if (tf->with == without_flags)
		    without_hex |= elf_flags_to_names[i].flag_value;
		  tf->valid = true;
		  break;
		}
	    }
	  if (!tf->valid)
	    {
	      info->callbacks->einfo
		(_("unrecognized INPUT_SECTION_FLAG %s\n"), tf->name);
	      return false;
	    }
	}
      flaginfo->flags_initialized = true;
      flaginfo->only_with_flags |= with_hex;
      flaginfo->not_with_flags |= without_hex;
    }

  if ((flaginfo->only_with_flags & sh_flags) != flaginfo->only_with_flags)
    return false;

  if ((flaginfo->not_with_flags & sh_flags) != 0)
    return false;

  return true;
}

struct alloc_got_off_arg {
  bfd_vma gotoff;
  struct bfd_link_info *info;
};

/* We need a special top-level link routine to convert got reference counts
   to real got offsets.  */

static bool
elf_gc_allocate_got_offsets (struct elf_link_hash_entry *h, void *arg)
{
  struct alloc_got_off_arg *gofarg = (struct alloc_got_off_arg *) arg;
  bfd *obfd = gofarg->info->output_bfd;
  const struct elf_backend_data *bed = get_elf_backend_data (obfd);

  if (h->got.refcount > 0)
    {
      h->got.offset = gofarg->gotoff;
      gofarg->gotoff += bed->got_elt_size (obfd, gofarg->info, h, NULL, 0);
    }
  else
    h->got.offset = (bfd_vma) -1;

  return true;
}

/* And an accompanying bit to work out final got entry offsets once
   we're done.  Should be called from final_link.  */

bool
bfd_elf_gc_common_finalize_got_offsets (bfd *abfd,
					struct bfd_link_info *info)
{
  bfd *i;
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  bfd_vma gotoff;
  struct alloc_got_off_arg gofarg;

  BFD_ASSERT (abfd == info->output_bfd);

  if (! is_elf_hash_table (info->hash))
    return false;

  /* The GOT offset is relative to the .got section, but the GOT header is
     put into the .got.plt section, if the backend uses it.  */
  if (bed->want_got_plt)
    gotoff = 0;
  else
    gotoff = bed->got_header_size;

  /* Do the local .got entries first.  */
  for (i = info->input_bfds; i; i = i->link.next)
    {
      bfd_signed_vma *local_got;
      size_t j, locsymcount;
      Elf_Internal_Shdr *symtab_hdr;

      if (bfd_get_flavour (i) != bfd_target_elf_flavour)
	continue;

      local_got = elf_local_got_refcounts (i);
      if (!local_got)
	continue;

      symtab_hdr = &elf_tdata (i)->symtab_hdr;
      if (elf_bad_symtab (i))
	locsymcount = symtab_hdr->sh_size / bed->s->sizeof_sym;
      else
	locsymcount = symtab_hdr->sh_info;

      for (j = 0; j < locsymcount; ++j)
	{
	  if (local_got[j] > 0)
	    {
	      local_got[j] = gotoff;
	      gotoff += bed->got_elt_size (abfd, info, NULL, i, j);
	    }
	  else
	    local_got[j] = (bfd_vma) -1;
	}
    }

  /* Then the global .got entries.  .plt refcounts are handled by
     adjust_dynamic_symbol  */
  gofarg.gotoff = gotoff;
  gofarg.info = info;
  elf_link_hash_traverse (elf_hash_table (info),
			  elf_gc_allocate_got_offsets,
			  &gofarg);
  return true;
}

/* Many folk need no more in the way of final link than this, once
   got entry reference counting is enabled.  */

bool
bfd_elf_gc_common_final_link (bfd *abfd, struct bfd_link_info *info)
{
  if (!bfd_elf_gc_common_finalize_got_offsets (abfd, info))
    return false;

  /* Invoke the regular ELF backend linker to do all the work.  */
  return bfd_elf_final_link (abfd, info);
}

bool
bfd_elf_reloc_symbol_deleted_p (bfd_vma offset, void *cookie)
{
  struct elf_reloc_cookie *rcookie = (struct elf_reloc_cookie *) cookie;

  if (rcookie->bad_symtab)
    rcookie->rel = rcookie->rels;

  for (; rcookie->rel < rcookie->relend; rcookie->rel++)
    {
      unsigned long r_symndx;

      if (! rcookie->bad_symtab)
	if (rcookie->rel->r_offset > offset)
	  return false;
      if (rcookie->rel->r_offset != offset)
	continue;

      r_symndx = rcookie->rel->r_info >> rcookie->r_sym_shift;
      if (r_symndx == STN_UNDEF)
	return true;

      if (r_symndx >= rcookie->locsymcount
	  || ELF_ST_BIND (rcookie->locsyms[r_symndx].st_info) != STB_LOCAL)
	{
	  struct elf_link_hash_entry *h;

	  h = rcookie->sym_hashes[r_symndx - rcookie->extsymoff];

	  while (h->root.type == bfd_link_hash_indirect
		 || h->root.type == bfd_link_hash_warning)
	    h = (struct elf_link_hash_entry *) h->root.u.i.link;

	  if ((h->root.type == bfd_link_hash_defined
	       || h->root.type == bfd_link_hash_defweak)
	      && (h->root.u.def.section->owner != rcookie->abfd
		  || h->root.u.def.section->kept_section != NULL
		  || discarded_section (h->root.u.def.section)))
	    return true;
	}
      else
	{
	  /* It's not a relocation against a global symbol,
	     but it could be a relocation against a local
	     symbol for a discarded section.  */
	  asection *isec;
	  Elf_Internal_Sym *isym;

	  /* Need to: get the symbol; get the section.  */
	  isym = &rcookie->locsyms[r_symndx];
	  isec = bfd_section_from_elf_index (rcookie->abfd, isym->st_shndx);
	  if (isec != NULL
	      && (isec->kept_section != NULL
		  || discarded_section (isec)))
	    return true;
	}
      return false;
    }
  return false;
}

/* Discard unneeded references to discarded sections.
   Returns -1 on error, 1 if any section's size was changed, 0 if
   nothing changed.  This function assumes that the relocations are in
   sorted order, which is true for all known assemblers.  */

int
bfd_elf_discard_info (bfd *output_bfd, struct bfd_link_info *info)
{
  struct elf_reloc_cookie cookie;
  asection *o;
  bfd *abfd;
  int changed = 0;

  if (info->traditional_format
      || !is_elf_hash_table (info->hash))
    return 0;

  o = bfd_get_section_by_name (output_bfd, ".stab");
  if (o != NULL)
    {
      asection *i;

      for (i = o->map_head.s; i != NULL; i = i->map_head.s)
	{
	  if (i->size == 0
	      || i->reloc_count == 0
	      || i->sec_info_type != SEC_INFO_TYPE_STABS)
	    continue;

	  abfd = i->owner;
	  if (bfd_get_flavour (abfd) != bfd_target_elf_flavour)
	    continue;

	  if (!init_reloc_cookie_for_section (&cookie, info, i))
	    return -1;

	  if (_bfd_discard_section_stabs (abfd, i,
					  elf_section_data (i)->sec_info,
					  bfd_elf_reloc_symbol_deleted_p,
					  &cookie))
	    changed = 1;

	  fini_reloc_cookie_for_section (&cookie, i);
	}
    }

  o = NULL;
  if (info->eh_frame_hdr_type != COMPACT_EH_HDR)
    o = bfd_get_section_by_name (output_bfd, ".eh_frame");
  if (o != NULL)
    {
      asection *i;
      int eh_changed = 0;
      unsigned int eh_alignment;  /* Octets.  */

      for (i = o->map_head.s; i != NULL; i = i->map_head.s)
	{
	  if (i->size == 0)
	    continue;

	  abfd = i->owner;
	  if (bfd_get_flavour (abfd) != bfd_target_elf_flavour)
	    continue;

	  if (!init_reloc_cookie_for_section (&cookie, info, i))
	    return -1;

	  _bfd_elf_parse_eh_frame (abfd, info, i, &cookie);
	  if (_bfd_elf_discard_section_eh_frame (abfd, info, i,
						 bfd_elf_reloc_symbol_deleted_p,
						 &cookie))
	    {
	      eh_changed = 1;
	      if (i->size != i->rawsize)
		changed = 1;
	    }

	  fini_reloc_cookie_for_section (&cookie, i);
	}

      eh_alignment = ((1 << o->alignment_power)
		      * bfd_octets_per_byte (output_bfd, o));
      /* Skip over zero terminator, and prevent empty sections from
	 adding alignment padding at the end.  */
      for (i = o->map_tail.s; i != NULL; i = i->map_tail.s)
	if (i->size == 0)
	  i->flags |= SEC_EXCLUDE;
	else if (i->size > 4)
	  break;
      /* The last non-empty eh_frame section doesn't need padding.  */
      if (i != NULL)
	i = i->map_tail.s;
      /* Any prior sections must pad the last FDE out to the output
	 section alignment.  Otherwise we might have zero padding
	 between sections, which would be seen as a terminator.  */
      for (; i != NULL; i = i->map_tail.s)
	if (i->size == 4)
	  /* All but the last zero terminator should have been removed.  */
	  BFD_FAIL ();
	else
	  {
	    bfd_size_type size
	      = (i->size + eh_alignment - 1) & -eh_alignment;
	    if (i->size != size)
	      {
		i->size = size;
		changed = 1;
		eh_changed = 1;
	      }
	  }
      if (eh_changed)
	elf_link_hash_traverse (elf_hash_table (info),
				_bfd_elf_adjust_eh_frame_global_symbol, NULL);
    }

  o = bfd_get_section_by_name (output_bfd, ".sframe");
  if (o != NULL)
    {
      asection *i;

      for (i = o->map_head.s; i != NULL; i = i->map_head.s)
	{
	  if (i->size == 0)
	    continue;

	  abfd = i->owner;
	  if (bfd_get_flavour (abfd) != bfd_target_elf_flavour)
	    continue;

	  if (!init_reloc_cookie_for_section (&cookie, info, i))
	    return -1;

	  if (_bfd_elf_parse_sframe (abfd, info, i, &cookie))
	    {
	      if (_bfd_elf_discard_section_sframe (i,
						   bfd_elf_reloc_symbol_deleted_p,
						   &cookie))
		{
		  if (i->size != i->rawsize)
		    changed = 1;
		}
	    }
	  fini_reloc_cookie_for_section (&cookie, i);
	}
      /* Update the reference to the output .sframe section.  Used to
	 determine later if PT_GNU_SFRAME segment is to be generated.  */
      if (!_bfd_elf_set_section_sframe (output_bfd, info))
	return -1;
    }

  for (abfd = info->input_bfds; abfd != NULL; abfd = abfd->link.next)
    {
      const struct elf_backend_data *bed;
      asection *s;

      if (bfd_get_flavour (abfd) != bfd_target_elf_flavour)
	continue;
      s = abfd->sections;
      if (s == NULL || s->sec_info_type == SEC_INFO_TYPE_JUST_SYMS)
	continue;

      bed = get_elf_backend_data (abfd);

      if (bed->elf_backend_discard_info != NULL)
	{
	  if (!init_reloc_cookie (&cookie, info, abfd))
	    return -1;

	  if ((*bed->elf_backend_discard_info) (abfd, &cookie, info))
	    changed = 1;

	  fini_reloc_cookie (&cookie, abfd);
	}
    }

  if (info->eh_frame_hdr_type == COMPACT_EH_HDR)
    _bfd_elf_end_eh_frame_parsing (info);

  if (info->eh_frame_hdr_type
      && !bfd_link_relocatable (info)
      && _bfd_elf_discard_section_eh_frame_hdr (info))
    changed = 1;

  return changed;
}

bool
_bfd_elf_section_already_linked (bfd *abfd,
				 asection *sec,
				 struct bfd_link_info *info)
{
  flagword flags;
  const char *name, *key;
  struct bfd_section_already_linked *l;
  struct bfd_section_already_linked_hash_entry *already_linked_list;

  if (sec->output_section == bfd_abs_section_ptr)
    return false;

  flags = sec->flags;

  /* Return if it isn't a linkonce section.  A comdat group section
     also has SEC_LINK_ONCE set.  */
  if ((flags & SEC_LINK_ONCE) == 0)
    return false;

  /* Don't put group member sections on our list of already linked
     sections.  They are handled as a group via their group section.  */
  if (elf_sec_group (sec) != NULL)
    return false;

  /* For a SHT_GROUP section, use the group signature as the key.  */
  name = sec->name;
  if ((flags & SEC_GROUP) != 0
      && elf_next_in_group (sec) != NULL
      && elf_group_name (elf_next_in_group (sec)) != NULL)
    key = elf_group_name (elf_next_in_group (sec));
  else
    {
      /* Otherwise we should have a .gnu.linkonce.<type>.<key> section.  */
      if (startswith (name, ".gnu.linkonce.")
	  && (key = strchr (name + sizeof (".gnu.linkonce.") - 1, '.')) != NULL)
	key++;
      else
	/* Must be a user linkonce section that doesn't follow gcc's
	   naming convention.  In this case we won't be matching
	   single member groups.  */
	key = name;
    }

  already_linked_list = bfd_section_already_linked_table_lookup (key);

  for (l = already_linked_list->entry; l != NULL; l = l->next)
    {
      /* We may have 2 different types of sections on the list: group
	 sections with a signature of <key> (<key> is some string),
	 and linkonce sections named .gnu.linkonce.<type>.<key>.
	 Match like sections.  LTO plugin sections are an exception.
	 They are always named .gnu.linkonce.t.<key> and match either
	 type of section.  */
      if (((flags & SEC_GROUP) == (l->sec->flags & SEC_GROUP)
	   && ((flags & SEC_GROUP) != 0
	       || strcmp (name, l->sec->name) == 0))
	  || (l->sec->owner->flags & BFD_PLUGIN) != 0
	  || (sec->owner->flags & BFD_PLUGIN) != 0)
	{
	  /* The section has already been linked.  See if we should
	     issue a warning.  */
	  if (!_bfd_handle_already_linked (sec, l, info))
	    return false;

	  if (flags & SEC_GROUP)
	    {
	      asection *first = elf_next_in_group (sec);
	      asection *s = first;

	      while (s != NULL)
		{
		  s->output_section = bfd_abs_section_ptr;
		  /* Record which group discards it.  */
		  s->kept_section = l->sec;
		  s = elf_next_in_group (s);
		  /* These lists are circular.  */
		  if (s == first)
		    break;
		}
	    }

	  return true;
	}
    }

  /* A single member comdat group section may be discarded by a
     linkonce section and vice versa.  */
  if ((flags & SEC_GROUP) != 0)
    {
      asection *first = elf_next_in_group (sec);

      if (first != NULL && elf_next_in_group (first) == first)
	/* Check this single member group against linkonce sections.  */
	for (l = already_linked_list->entry; l != NULL; l = l->next)
	  if ((l->sec->flags & SEC_GROUP) == 0
	      && bfd_elf_match_symbols_in_sections (l->sec, first, info))
	    {
	      first->output_section = bfd_abs_section_ptr;
	      first->kept_section = l->sec;
	      sec->output_section = bfd_abs_section_ptr;
	      break;
	    }
    }
  else
    /* Check this linkonce section against single member groups.  */
    for (l = already_linked_list->entry; l != NULL; l = l->next)
      if (l->sec->flags & SEC_GROUP)
	{
	  asection *first = elf_next_in_group (l->sec);

	  if (first != NULL
	      && elf_next_in_group (first) == first
	      && bfd_elf_match_symbols_in_sections (first, sec, info))
	    {
	      sec->output_section = bfd_abs_section_ptr;
	      sec->kept_section = first;
	      break;
	    }
	}

  /* Do not complain on unresolved relocations in `.gnu.linkonce.r.F'
     referencing its discarded `.gnu.linkonce.t.F' counterpart - g++-3.4
     specific as g++-4.x is using COMDAT groups (without the `.gnu.linkonce'
     prefix) instead.  `.gnu.linkonce.r.*' were the `.rodata' part of its
     matching `.gnu.linkonce.t.*'.  If `.gnu.linkonce.r.F' is not discarded
     but its `.gnu.linkonce.t.F' is discarded means we chose one-only
     `.gnu.linkonce.t.F' section from a different bfd not requiring any
     `.gnu.linkonce.r.F'.  Thus `.gnu.linkonce.r.F' should be discarded.
     The reverse order cannot happen as there is never a bfd with only the
     `.gnu.linkonce.r.F' section.  The order of sections in a bfd does not
     matter as here were are looking only for cross-bfd sections.  */

  if ((flags & SEC_GROUP) == 0 && startswith (name, ".gnu.linkonce.r."))
    for (l = already_linked_list->entry; l != NULL; l = l->next)
      if ((l->sec->flags & SEC_GROUP) == 0
	  && startswith (l->sec->name, ".gnu.linkonce.t."))
	{
	  if (abfd != l->sec->owner)
	    sec->output_section = bfd_abs_section_ptr;
	  break;
	}

  /* This is the first section with this name.  Record it.  */
  if (!bfd_section_already_linked_table_insert (already_linked_list, sec))
    info->callbacks->einfo (_("%F%P: already_linked_table: %E\n"));
  return sec->output_section == bfd_abs_section_ptr;
}

bool
_bfd_elf_common_definition (Elf_Internal_Sym *sym)
{
  return sym->st_shndx == SHN_COMMON;
}

unsigned int
_bfd_elf_common_section_index (asection *sec ATTRIBUTE_UNUSED)
{
  return SHN_COMMON;
}

asection *
_bfd_elf_common_section (asection *sec ATTRIBUTE_UNUSED)
{
  return bfd_com_section_ptr;
}

bfd_vma
_bfd_elf_default_got_elt_size (bfd *abfd,
			       struct bfd_link_info *info ATTRIBUTE_UNUSED,
			       struct elf_link_hash_entry *h ATTRIBUTE_UNUSED,
			       bfd *ibfd ATTRIBUTE_UNUSED,
			       unsigned long symndx ATTRIBUTE_UNUSED)
{
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  return bed->s->arch_size / 8;
}

/* Routines to support the creation of dynamic relocs.  */

/* Returns the name of the dynamic reloc section associated with SEC.  */

static const char *
get_dynamic_reloc_section_name (bfd *       abfd,
				asection *  sec,
				bool is_rela)
{
  char *name;
  const char *old_name = bfd_section_name (sec);
  const char *prefix = is_rela ? ".rela" : ".rel";

  if (old_name == NULL)
    return NULL;

  name = bfd_alloc (abfd, strlen (prefix) + strlen (old_name) + 1);
  sprintf (name, "%s%s", prefix, old_name);

  return name;
}

/* Returns the dynamic reloc section associated with SEC.
   If necessary compute the name of the dynamic reloc section based
   on SEC's name (looked up in ABFD's string table) and the setting
   of IS_RELA.  */

asection *
_bfd_elf_get_dynamic_reloc_section (bfd *abfd,
				    asection *sec,
				    bool is_rela)
{
  asection *reloc_sec = elf_section_data (sec)->sreloc;

  if (reloc_sec == NULL)
    {
      const char *name = get_dynamic_reloc_section_name (abfd, sec, is_rela);

      if (name != NULL)
	{
	  reloc_sec = bfd_get_linker_section (abfd, name);

	  if (reloc_sec != NULL)
	    elf_section_data (sec)->sreloc = reloc_sec;
	}
    }

  return reloc_sec;
}

/* Returns the dynamic reloc section associated with SEC.  If the
   section does not exist it is created and attached to the DYNOBJ
   bfd and stored in the SRELOC field of SEC's elf_section_data
   structure.

   ALIGNMENT is the alignment for the newly created section and
   IS_RELA defines whether the name should be .rela.<SEC's name>
   or .rel.<SEC's name>.  The section name is looked up in the
   string table associated with ABFD.  */

asection *
_bfd_elf_make_dynamic_reloc_section (asection *sec,
				     bfd *dynobj,
				     unsigned int alignment,
				     bfd *abfd,
				     bool is_rela)
{
  asection * reloc_sec = elf_section_data (sec)->sreloc;

  if (reloc_sec == NULL)
    {
      const char * name = get_dynamic_reloc_section_name (abfd, sec, is_rela);

      if (name == NULL)
	return NULL;

      reloc_sec = bfd_get_linker_section (dynobj, name);

      if (reloc_sec == NULL)
	{
	  flagword flags = (SEC_HAS_CONTENTS | SEC_READONLY
			    | SEC_IN_MEMORY | SEC_LINKER_CREATED);
	  if ((sec->flags & SEC_ALLOC) != 0)
	    flags |= SEC_ALLOC | SEC_LOAD;

	  reloc_sec = bfd_make_section_anyway_with_flags (dynobj, name, flags);
	  if (reloc_sec != NULL)
	    {
	      /* _bfd_elf_get_sec_type_attr chooses a section type by
		 name.  Override as it may be wrong, eg. for a user
		 section named "auto" we'll get ".relauto" which is
		 seen to be a .rela section.  */
	      elf_section_type (reloc_sec) = is_rela ? SHT_RELA : SHT_REL;
	      if (!bfd_set_section_alignment (reloc_sec, alignment))
		reloc_sec = NULL;
	    }
	}

      elf_section_data (sec)->sreloc = reloc_sec;
    }

  return reloc_sec;
}

/* Copy the ELF symbol type and other attributes for a linker script
   assignment from HSRC to HDEST.  Generally this should be treated as
   if we found a strong non-dynamic definition for HDEST (except that
   ld ignores multiple definition errors).  */
void
_bfd_elf_copy_link_hash_symbol_type (bfd *abfd,
				     struct bfd_link_hash_entry *hdest,
				     struct bfd_link_hash_entry *hsrc)
{
  struct elf_link_hash_entry *ehdest = (struct elf_link_hash_entry *) hdest;
  struct elf_link_hash_entry *ehsrc = (struct elf_link_hash_entry *) hsrc;
  Elf_Internal_Sym isym;

  ehdest->type = ehsrc->type;
  ehdest->target_internal = ehsrc->target_internal;

  isym.st_other = ehsrc->other;
  elf_merge_st_other (abfd, ehdest, isym.st_other, NULL, true, false);
}

/* Append a RELA relocation REL to section S in BFD.  */

void
elf_append_rela (bfd *abfd, asection *s, Elf_Internal_Rela *rel)
{
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  bfd_byte *loc = s->contents + (s->reloc_count++ * bed->s->sizeof_rela);
  BFD_ASSERT (loc + bed->s->sizeof_rela <= s->contents + s->size);
  bed->s->swap_reloca_out (abfd, rel, loc);
}

/* Append a REL relocation REL to section S in BFD.  */

void
elf_append_rel (bfd *abfd, asection *s, Elf_Internal_Rela *rel)
{
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  bfd_byte *loc = s->contents + (s->reloc_count++ * bed->s->sizeof_rel);
  BFD_ASSERT (loc + bed->s->sizeof_rel <= s->contents + s->size);
  bed->s->swap_reloc_out (abfd, rel, loc);
}

/* Define __start, __stop, .startof. or .sizeof. symbol.  */

struct bfd_link_hash_entry *
bfd_elf_define_start_stop (struct bfd_link_info *info,
			   const char *symbol, asection *sec)
{
  struct elf_link_hash_entry *h;

  h = elf_link_hash_lookup (elf_hash_table (info), symbol,
			    false, false, true);
  /* NB: Common symbols will be turned into definition later.  */
  if (h != NULL
      && !h->root.ldscript_def
      && (h->root.type == bfd_link_hash_undefined
	  || h->root.type == bfd_link_hash_undefweak
	  || ((h->ref_regular || h->def_dynamic)
	      && !h->def_regular
	      && h->root.type != bfd_link_hash_common)))
    {
      bool was_dynamic = h->ref_dynamic || h->def_dynamic;
      h->verinfo.verdef = NULL;
      h->root.type = bfd_link_hash_defined;
      h->root.u.def.section = sec;
      h->root.u.def.value = 0;
      h->def_regular = 1;
      h->def_dynamic = 0;
      h->start_stop = 1;
      h->u2.start_stop_section = sec;
      if (symbol[0] == '.')
	{
	  /* .startof. and .sizeof. symbols are local.  */
	  const struct elf_backend_data *bed;
	  bed = get_elf_backend_data (info->output_bfd);
	  (*bed->elf_backend_hide_symbol) (info, h, true);
	}
      else
	{
	  if (ELF_ST_VISIBILITY (h->other) == STV_DEFAULT)
	    h->other = ((h->other & ~ELF_ST_VISIBILITY (-1))
			| info->start_stop_visibility);
	  if (was_dynamic)
	    bfd_elf_link_record_dynamic_symbol (info, h);
	}
      return &h->root;
    }
  return NULL;
}

/* Find dynamic relocs for H that apply to read-only sections.  */

asection *
_bfd_elf_readonly_dynrelocs (struct elf_link_hash_entry *h)
{
  struct elf_dyn_relocs *p;

  for (p = h->dyn_relocs; p != NULL; p = p->next)
    {
      asection *s = p->sec->output_section;

      if (s != NULL && (s->flags & SEC_READONLY) != 0)
	return p->sec;
    }
  return NULL;
}

/* Set DF_TEXTREL if we find any dynamic relocs that apply to
   read-only sections.  */

bool
_bfd_elf_maybe_set_textrel (struct elf_link_hash_entry *h, void *inf)
{
  asection *sec;

  if (h->root.type == bfd_link_hash_indirect)
    return true;

  sec = _bfd_elf_readonly_dynrelocs (h);
  if (sec != NULL)
    {
      struct bfd_link_info *info = (struct bfd_link_info *) inf;

      info->flags |= DF_TEXTREL;
      /* xgettext:c-format */
      info->callbacks->minfo (_("%pB: dynamic relocation against `%pT' "
				"in read-only section `%pA'\n"),
			      sec->owner, h->root.root.string, sec);

      if (bfd_link_textrel_check (info))
	/* xgettext:c-format */
	info->callbacks->einfo (_("%P: %pB: warning: relocation against `%s' "
				  "in read-only section `%pA'\n"),
				sec->owner, h->root.root.string, sec);

      /* Not an error, just cut short the traversal.  */
      return false;
    }
  return true;
}

/* Add dynamic tags.  */

bool
_bfd_elf_add_dynamic_tags (bfd *output_bfd, struct bfd_link_info *info,
			   bool need_dynamic_reloc)
{
  struct elf_link_hash_table *htab = elf_hash_table (info);

  if (htab->dynamic_sections_created)
    {
      /* Add some entries to the .dynamic section.  We fill in the
	 values later, in finish_dynamic_sections, but we must add
	 the entries now so that we get the correct size for the
	 .dynamic section.  The DT_DEBUG entry is filled in by the
	 dynamic linker and used by the debugger.  */
#define add_dynamic_entry(TAG, VAL) \
  _bfd_elf_add_dynamic_entry (info, TAG, VAL)

      const struct elf_backend_data *bed
	= get_elf_backend_data (output_bfd);

      if (bfd_link_executable (info))
	{
	  if (!add_dynamic_entry (DT_DEBUG, 0))
	    return false;
	}

      if (htab->dt_pltgot_required || htab->splt->size != 0)
	{
	  /* DT_PLTGOT is used by prelink even if there is no PLT
	     relocation.  */
	  if (!add_dynamic_entry (DT_PLTGOT, 0))
	    return false;
	}

      if (htab->dt_jmprel_required || htab->srelplt->size != 0)
	{
	  if (!add_dynamic_entry (DT_PLTRELSZ, 0)
	      || !add_dynamic_entry (DT_PLTREL,
				     (bed->rela_plts_and_copies_p
				      ? DT_RELA : DT_REL))
	      || !add_dynamic_entry (DT_JMPREL, 0))
	    return false;
	}

      if (htab->tlsdesc_plt
	  && (!add_dynamic_entry (DT_TLSDESC_PLT, 0)
	      || !add_dynamic_entry (DT_TLSDESC_GOT, 0)))
	return false;

      if (need_dynamic_reloc)
	{
	  if (bed->rela_plts_and_copies_p)
	    {
	      if (!add_dynamic_entry (DT_RELA, 0)
		  || !add_dynamic_entry (DT_RELASZ, 0)
		  || !add_dynamic_entry (DT_RELAENT,
					 bed->s->sizeof_rela))
		return false;
	    }
	  else
	    {
	      if (!add_dynamic_entry (DT_REL, 0)
		  || !add_dynamic_entry (DT_RELSZ, 0)
		  || !add_dynamic_entry (DT_RELENT,
					 bed->s->sizeof_rel))
		return false;
	    }

	  /* If any dynamic relocs apply to a read-only section,
	     then we need a DT_TEXTREL entry.  */
	  if ((info->flags & DF_TEXTREL) == 0)
	    elf_link_hash_traverse (htab, _bfd_elf_maybe_set_textrel,
				    info);

	  if ((info->flags & DF_TEXTREL) != 0)
	    {
	      if (htab->ifunc_resolvers)
		info->callbacks->einfo
		  (_("%P: warning: GNU indirect functions with DT_TEXTREL "
		     "may result in a segfault at runtime; recompile with %s\n"),
		   bfd_link_dll (info) ? "-fPIC" : "-fPIE");

	      if (!add_dynamic_entry (DT_TEXTREL, 0))
		return false;
	    }
	}
    }
#undef add_dynamic_entry

  return true;
}
