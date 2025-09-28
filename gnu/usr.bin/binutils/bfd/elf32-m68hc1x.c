/* Motorola 68HC11/HC12-specific support for 32-bit ELF
   Copyright (C) 1999-2023 Free Software Foundation, Inc.
   Contributed by Stephane Carrez (stcarrez@nerim.fr)

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
#include "elf32-m68hc1x.h"
#include "elf/m68hc11.h"
#include "opcode/m68hc11.h"
#include "libiberty.h"

#define m68hc12_stub_hash_lookup(table, string, create, copy) \
  ((struct elf32_m68hc11_stub_hash_entry *) \
   bfd_hash_lookup ((table), (string), (create), (copy)))

static struct elf32_m68hc11_stub_hash_entry* m68hc12_add_stub
  (const char *stub_name,
   asection *section,
   struct m68hc11_elf_link_hash_table *htab);

static struct bfd_hash_entry *stub_hash_newfunc
  (struct bfd_hash_entry *, struct bfd_hash_table *, const char *);

static void m68hc11_elf_set_symbol (bfd* abfd, struct bfd_link_info *info,
				    const char* name, bfd_vma value,
				    asection* sec);

static bool m68hc11_elf_export_one_stub
  (struct bfd_hash_entry *gen_entry, void *in_arg);

static void scan_sections_for_abi (bfd*, asection*, void *);

struct m68hc11_scan_param
{
   struct m68hc11_page_info* pinfo;
   bool use_memory_banks;
};


/* Destroy a 68HC11/68HC12 ELF linker hash table.  */

static void
m68hc11_elf_bfd_link_hash_table_free (bfd *obfd)
{
  struct m68hc11_elf_link_hash_table *ret
    = (struct m68hc11_elf_link_hash_table *) obfd->link.hash;

  bfd_hash_table_free (ret->stub_hash_table);
  free (ret->stub_hash_table);
  _bfd_elf_link_hash_table_free (obfd);
}

/* Create a 68HC11/68HC12 ELF linker hash table.  */

struct m68hc11_elf_link_hash_table*
m68hc11_elf_hash_table_create (bfd *abfd)
{
  struct m68hc11_elf_link_hash_table *ret;
  size_t amt = sizeof (struct m68hc11_elf_link_hash_table);

  ret = (struct m68hc11_elf_link_hash_table *) bfd_zmalloc (amt);
  if (ret == (struct m68hc11_elf_link_hash_table *) NULL)
    return NULL;

  if (!_bfd_elf_link_hash_table_init (&ret->root, abfd,
				      _bfd_elf_link_hash_newfunc,
				      sizeof (struct elf_link_hash_entry),
				      M68HC11_ELF_DATA))
    {
      free (ret);
      return NULL;
    }

  /* Init the stub hash table too.  */
  amt = sizeof (struct bfd_hash_table);
  ret->stub_hash_table = (struct bfd_hash_table*) bfd_malloc (amt);
  if (ret->stub_hash_table == NULL)
    {
      _bfd_elf_link_hash_table_free (abfd);
      return NULL;
    }
  if (!bfd_hash_table_init (ret->stub_hash_table, stub_hash_newfunc,
			    sizeof (struct elf32_m68hc11_stub_hash_entry)))
    {
      free (ret->stub_hash_table);
      _bfd_elf_link_hash_table_free (abfd);
      return NULL;
    }
  ret->root.root.hash_table_free = m68hc11_elf_bfd_link_hash_table_free;

  return ret;
}

/* Assorted hash table functions.  */

/* Initialize an entry in the stub hash table.  */

static struct bfd_hash_entry *
stub_hash_newfunc (struct bfd_hash_entry *entry, struct bfd_hash_table *table,
		   const char *string)
{
  /* Allocate the structure if it has not already been allocated by a
     subclass.  */
  if (entry == NULL)
    {
      entry = bfd_hash_allocate (table,
				 sizeof (struct elf32_m68hc11_stub_hash_entry));
      if (entry == NULL)
	return entry;
    }

  /* Call the allocation method of the superclass.  */
  entry = bfd_hash_newfunc (entry, table, string);
  if (entry != NULL)
    {
      struct elf32_m68hc11_stub_hash_entry *eh;

      /* Initialize the local fields.  */
      eh = (struct elf32_m68hc11_stub_hash_entry *) entry;
      eh->stub_sec = NULL;
      eh->stub_offset = 0;
      eh->target_value = 0;
      eh->target_section = NULL;
    }

  return entry;
}

/* Add a new stub entry to the stub hash.  Not all fields of the new
   stub entry are initialised.  */

static struct elf32_m68hc11_stub_hash_entry *
m68hc12_add_stub (const char *stub_name, asection *section,
		  struct m68hc11_elf_link_hash_table *htab)
{
  struct elf32_m68hc11_stub_hash_entry *stub_entry;

  /* Enter this entry into the linker stub hash table.  */
  stub_entry = m68hc12_stub_hash_lookup (htab->stub_hash_table, stub_name,
					 true, false);
  if (stub_entry == NULL)
    {
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: cannot create stub entry %s"),
			  section->owner, stub_name);
      return NULL;
    }

  if (htab->stub_section == 0)
    {
      htab->stub_section = (*htab->add_stub_section) (".tramp",
						      htab->tramp_section);
    }

  stub_entry->stub_sec = htab->stub_section;
  stub_entry->stub_offset = 0;
  return stub_entry;
}

/* Hook called by the linker routine which adds symbols from an object
   file.  We use it for identify far symbols and force a loading of
   the trampoline handler.  */

bool
elf32_m68hc11_add_symbol_hook (bfd *abfd, struct bfd_link_info *info,
			       Elf_Internal_Sym *sym,
			       const char **namep ATTRIBUTE_UNUSED,
			       flagword *flagsp ATTRIBUTE_UNUSED,
			       asection **secp ATTRIBUTE_UNUSED,
			       bfd_vma *valp ATTRIBUTE_UNUSED)
{
  if (sym->st_other & STO_M68HC12_FAR)
    {
      struct elf_link_hash_entry *h;

      h = (struct elf_link_hash_entry *)
	bfd_link_hash_lookup (info->hash, "__far_trampoline",
			      false, false, false);
      if (h == NULL)
	{
	  struct bfd_link_hash_entry* entry = NULL;

	  _bfd_generic_link_add_one_symbol (info, abfd,
					    "__far_trampoline",
					    BSF_GLOBAL,
					    bfd_und_section_ptr,
					    (bfd_vma) 0, (const char*) NULL,
					    false, false, &entry);
	}

    }
  return true;
}

/* Merge non-visibility st_other attributes, STO_M68HC12_FAR and
   STO_M68HC12_INTERRUPT.  */

void
elf32_m68hc11_merge_symbol_attribute (struct elf_link_hash_entry *h,
				      unsigned int st_other,
				      bool definition,
				      bool dynamic ATTRIBUTE_UNUSED)
{
  if (definition)
    h->other = ((st_other & ~ELF_ST_VISIBILITY (-1))
		| ELF_ST_VISIBILITY (h->other));
}

/* External entry points for sizing and building linker stubs.  */

/* Set up various things so that we can make a list of input sections
   for each output section included in the link.  Returns -1 on error,
   0 when no stubs will be needed, and 1 on success.  */

int
elf32_m68hc11_setup_section_lists (bfd *output_bfd, struct bfd_link_info *info)
{
  bfd *input_bfd;
  unsigned int bfd_count;
  unsigned int top_id, top_index;
  asection *section;
  asection **input_list, **list;
  size_t amt;
  asection *text_section;
  struct m68hc11_elf_link_hash_table *htab;

  htab = m68hc11_elf_hash_table (info);
  if (htab == NULL)
    return -1;

  if (bfd_get_flavour (info->output_bfd) != bfd_target_elf_flavour)
    return 0;

  /* Count the number of input BFDs and find the top input section id.
     Also search for an existing ".tramp" section so that we know
     where generated trampolines must go.  Default to ".text" if we
     can't find it.  */
  htab->tramp_section = 0;
  text_section = 0;
  for (input_bfd = info->input_bfds, bfd_count = 0, top_id = 0;
       input_bfd != NULL;
       input_bfd = input_bfd->link.next)
    {
      bfd_count += 1;
      for (section = input_bfd->sections;
	   section != NULL;
	   section = section->next)
	{
	  const char *name = bfd_section_name (section);

	  if (!strcmp (name, ".tramp"))
	    htab->tramp_section = section;

	  if (!strcmp (name, ".text"))
	    text_section = section;

	  if (top_id < section->id)
	    top_id = section->id;
	}
    }
  htab->bfd_count = bfd_count;
  if (htab->tramp_section == 0)
    htab->tramp_section = text_section;

  /* We can't use output_bfd->section_count here to find the top output
     section index as some sections may have been removed, and
     strip_excluded_output_sections doesn't renumber the indices.  */
  for (section = output_bfd->sections, top_index = 0;
       section != NULL;
       section = section->next)
    {
      if (top_index < section->index)
	top_index = section->index;
    }

  htab->top_index = top_index;
  amt = sizeof (asection *) * (top_index + 1);
  input_list = (asection **) bfd_malloc (amt);
  htab->input_list = input_list;
  if (input_list == NULL)
    return -1;

  /* For sections we aren't interested in, mark their entries with a
     value we can check later.  */
  list = input_list + top_index;
  do
    *list = bfd_abs_section_ptr;
  while (list-- != input_list);

  for (section = output_bfd->sections;
       section != NULL;
       section = section->next)
    {
      if ((section->flags & SEC_CODE) != 0)
	input_list[section->index] = NULL;
    }

  return 1;
}

/* Determine and set the size of the stub section for a final link.

   The basic idea here is to examine all the relocations looking for
   PC-relative calls to a target that is unreachable with a "bl"
   instruction.  */

bool
elf32_m68hc11_size_stubs (bfd *output_bfd, bfd *stub_bfd,
			  struct bfd_link_info *info,
			  asection * (*add_stub_section) (const char*, asection*))
{
  bfd *input_bfd;
  asection *section;
  Elf_Internal_Sym *local_syms, **all_local_syms;
  unsigned int bfd_indx, bfd_count;
  size_t amt;
  asection *stub_sec;
  struct m68hc11_elf_link_hash_table *htab = m68hc11_elf_hash_table (info);

  if (htab == NULL)
    return false;

  /* Stash our params away.  */
  htab->stub_bfd = stub_bfd;
  htab->add_stub_section = add_stub_section;

  /* Count the number of input BFDs and find the top input section id.  */
  for (input_bfd = info->input_bfds, bfd_count = 0;
       input_bfd != NULL;
       input_bfd = input_bfd->link.next)
    bfd_count += 1;

  /* We want to read in symbol extension records only once.  To do this
     we need to read in the local symbols in parallel and save them for
     later use; so hold pointers to the local symbols in an array.  */
  amt = sizeof (Elf_Internal_Sym *) * bfd_count;
  all_local_syms = (Elf_Internal_Sym **) bfd_zmalloc (amt);
  if (all_local_syms == NULL)
    return false;

  /* Walk over all the input BFDs, swapping in local symbols.  */
  for (input_bfd = info->input_bfds, bfd_indx = 0;
       input_bfd != NULL;
       input_bfd = input_bfd->link.next, bfd_indx++)
    {
      Elf_Internal_Shdr *symtab_hdr;

      /* We'll need the symbol table in a second.  */
      symtab_hdr = &elf_tdata (input_bfd)->symtab_hdr;
      if (symtab_hdr->sh_info == 0)
	continue;

      /* We need an array of the local symbols attached to the input bfd.  */
      local_syms = (Elf_Internal_Sym *) symtab_hdr->contents;
      if (local_syms == NULL)
	{
	  local_syms = bfd_elf_get_elf_syms (input_bfd, symtab_hdr,
					     symtab_hdr->sh_info, 0,
					     NULL, NULL, NULL);
	  /* Cache them for elf_link_input_bfd.  */
	  symtab_hdr->contents = (unsigned char *) local_syms;
	}
      if (local_syms == NULL)
	{
	  free (all_local_syms);
	  return false;
	}

      all_local_syms[bfd_indx] = local_syms;
    }

  for (input_bfd = info->input_bfds, bfd_indx = 0;
       input_bfd != NULL;
       input_bfd = input_bfd->link.next, bfd_indx++)
    {
      Elf_Internal_Shdr *symtab_hdr;
      struct elf_link_hash_entry ** sym_hashes;

      sym_hashes = elf_sym_hashes (input_bfd);

      /* We'll need the symbol table in a second.  */
      symtab_hdr = &elf_tdata (input_bfd)->symtab_hdr;
      if (symtab_hdr->sh_info == 0)
	continue;

      local_syms = all_local_syms[bfd_indx];

      /* Walk over each section attached to the input bfd.  */
      for (section = input_bfd->sections;
	   section != NULL;
	   section = section->next)
	{
	  Elf_Internal_Rela *internal_relocs, *irelaend, *irela;

	  /* If there aren't any relocs, then there's nothing more
	     to do.  */
	  if ((section->flags & SEC_RELOC) == 0
	      || section->reloc_count == 0)
	    continue;

	  /* If this section is a link-once section that will be
	     discarded, then don't create any stubs.  */
	  if (section->output_section == NULL
	      || section->output_section->owner != output_bfd)
	    continue;

	  /* Get the relocs.  */
	  internal_relocs
	    = _bfd_elf_link_read_relocs (input_bfd, section, NULL,
					 (Elf_Internal_Rela *) NULL,
					 info->keep_memory);
	  if (internal_relocs == NULL)
	    goto error_ret_free_local;

	  /* Now examine each relocation.  */
	  irela = internal_relocs;
	  irelaend = irela + section->reloc_count;
	  for (; irela < irelaend; irela++)
	    {
	      unsigned int r_type, r_indx;
	      struct elf32_m68hc11_stub_hash_entry *stub_entry;
	      asection *sym_sec;
	      bfd_vma sym_value;
	      struct elf_link_hash_entry *hash;
	      const char *stub_name;
	      Elf_Internal_Sym *sym;

	      r_type = ELF32_R_TYPE (irela->r_info);

	      /* Only look at 16-bit relocs.  */
	      if (r_type != (unsigned int) R_M68HC11_16)
		continue;

	      /* Now determine the call target, its name, value,
		 section.  */
	      r_indx = ELF32_R_SYM (irela->r_info);
	      if (r_indx < symtab_hdr->sh_info)
		{
		  /* It's a local symbol.  */
		  Elf_Internal_Shdr *hdr;
		  bool is_far;

		  sym = local_syms + r_indx;
		  is_far = (sym && (sym->st_other & STO_M68HC12_FAR));
		  if (!is_far)
		    continue;

		  if (sym->st_shndx >= elf_numsections (input_bfd))
		    sym_sec = NULL;
		  else
		    {
		      hdr = elf_elfsections (input_bfd)[sym->st_shndx];
		      sym_sec = hdr->bfd_section;
		    }
		  stub_name = (bfd_elf_string_from_elf_section
			       (input_bfd, symtab_hdr->sh_link,
				sym->st_name));
		  sym_value = sym->st_value;
		  hash = NULL;
		}
	      else
		{
		  /* It's an external symbol.  */
		  int e_indx;

		  e_indx = r_indx - symtab_hdr->sh_info;
		  hash = (struct elf_link_hash_entry *)
		    (sym_hashes[e_indx]);

		  while (hash->root.type == bfd_link_hash_indirect
			 || hash->root.type == bfd_link_hash_warning)
		    hash = ((struct elf_link_hash_entry *)
			    hash->root.u.i.link);

		  if (hash->root.type == bfd_link_hash_defined
		      || hash->root.type == bfd_link_hash_defweak
		      || hash->root.type == bfd_link_hash_new)
		    {
		      if (!(hash->other & STO_M68HC12_FAR))
			continue;
		    }
		  else if (hash->root.type == bfd_link_hash_undefweak)
		    {
		      continue;
		    }
		  else if (hash->root.type == bfd_link_hash_undefined)
		    {
		      continue;
		    }
		  else
		    {
		      bfd_set_error (bfd_error_bad_value);
		      goto error_ret_free_internal;
		    }
		  sym_sec = hash->root.u.def.section;
		  sym_value = hash->root.u.def.value;
		  stub_name = hash->root.root.string;
		}

	      if (!stub_name)
		goto error_ret_free_internal;

	      stub_entry = m68hc12_stub_hash_lookup
		(htab->stub_hash_table,
		 stub_name,
		 false, false);
	      if (stub_entry == NULL)
		{
		  if (add_stub_section == 0)
		    continue;

		  stub_entry = m68hc12_add_stub (stub_name, section, htab);
		  if (stub_entry == NULL)
		    {
		    error_ret_free_internal:
		      if (elf_section_data (section)->relocs == NULL)
			free (internal_relocs);
		      goto error_ret_free_local;
		    }
		}

	      stub_entry->target_value = sym_value;
	      stub_entry->target_section = sym_sec;
	    }

	  /* We're done with the internal relocs, free them.  */
	  if (elf_section_data (section)->relocs == NULL)
	    free (internal_relocs);
	}
    }

  if (add_stub_section)
    {
      /* OK, we've added some stubs.  Find out the new size of the
	 stub sections.  */
      for (stub_sec = htab->stub_bfd->sections;
	   stub_sec != NULL;
	   stub_sec = stub_sec->next)
	{
	  stub_sec->size = 0;
	}

      bfd_hash_traverse (htab->stub_hash_table, htab->size_one_stub, htab);
    }
  free (all_local_syms);
  return true;

 error_ret_free_local:
  free (all_local_syms);
  return false;
}

/* Export the trampoline addresses in the symbol table.  */
static bool
m68hc11_elf_export_one_stub (struct bfd_hash_entry *gen_entry, void *in_arg)
{
  struct bfd_link_info *info;
  struct m68hc11_elf_link_hash_table *htab;
  struct elf32_m68hc11_stub_hash_entry *stub_entry;
  char* name;
  bool result;

  info = (struct bfd_link_info *) in_arg;
  htab = m68hc11_elf_hash_table (info);
  if (htab == NULL)
    return false;

  /* Massage our args to the form they really have.  */
  stub_entry = (struct elf32_m68hc11_stub_hash_entry *) gen_entry;

  /* Generate the trampoline according to HC11 or HC12.  */
  result = (* htab->build_one_stub) (gen_entry, in_arg);

  /* Make a printable name that does not conflict with the real function.  */
  name = concat ("tramp.", stub_entry->root.string, NULL);

  /* Export the symbol for debugging/disassembling.  */
  m68hc11_elf_set_symbol (htab->stub_bfd, info, name,
			  stub_entry->stub_offset,
			  stub_entry->stub_sec);
  free (name);
  return result;
}

/* Export a symbol or set its value and section.  */
static void
m68hc11_elf_set_symbol (bfd *abfd, struct bfd_link_info *info,
			const char *name, bfd_vma value, asection *sec)
{
  struct elf_link_hash_entry *h;

  h = (struct elf_link_hash_entry *)
    bfd_link_hash_lookup (info->hash, name, false, false, false);
  if (h == NULL)
    {
      _bfd_generic_link_add_one_symbol (info, abfd,
					name,
					BSF_GLOBAL,
					sec,
					value,
					(const char*) NULL,
					true, false, NULL);
    }
  else
    {
      h->root.type = bfd_link_hash_defined;
      h->root.u.def.value = value;
      h->root.u.def.section = sec;
    }
}


/* Build all the stubs associated with the current output file.  The
   stubs are kept in a hash table attached to the main linker hash
   table.  This function is called via m68hc12elf_finish in the
   linker.  */

bool
elf32_m68hc11_build_stubs (bfd *abfd, struct bfd_link_info *info)
{
  asection *stub_sec;
  struct bfd_hash_table *table;
  struct m68hc11_elf_link_hash_table *htab;
  struct m68hc11_scan_param param;

  m68hc11_elf_get_bank_parameters (info);
  htab = m68hc11_elf_hash_table (info);
  if (htab == NULL)
    return false;

  for (stub_sec = htab->stub_bfd->sections;
       stub_sec != NULL;
       stub_sec = stub_sec->next)
    {
      bfd_size_type size;

      /* Allocate memory to hold the linker stubs.  */
      size = stub_sec->size;
      stub_sec->contents = (unsigned char *) bfd_zalloc (htab->stub_bfd, size);
      if (stub_sec->contents == NULL && size != 0)
	return false;
      stub_sec->size = 0;
    }

  /* Build the stubs as directed by the stub hash table.  */
  table = htab->stub_hash_table;
  bfd_hash_traverse (table, m68hc11_elf_export_one_stub, info);

  /* Scan the output sections to see if we use the memory banks.
     If so, export the symbols that define how the memory banks
     are mapped.  This is used by gdb and the simulator to obtain
     the information.  It can be used by programs to burn the eprom
     at the good addresses.  */
  param.use_memory_banks = false;
  param.pinfo = &htab->pinfo;
  bfd_map_over_sections (abfd, scan_sections_for_abi, &param);
  if (param.use_memory_banks)
    {
      m68hc11_elf_set_symbol (abfd, info, BFD_M68HC11_BANK_START_NAME,
			      htab->pinfo.bank_physical,
			      bfd_abs_section_ptr);
      m68hc11_elf_set_symbol (abfd, info, BFD_M68HC11_BANK_VIRTUAL_NAME,
			      htab->pinfo.bank_virtual,
			      bfd_abs_section_ptr);
      m68hc11_elf_set_symbol (abfd, info, BFD_M68HC11_BANK_SIZE_NAME,
			      htab->pinfo.bank_size,
			      bfd_abs_section_ptr);
    }

  return true;
}

void
m68hc11_elf_get_bank_parameters (struct bfd_link_info *info)
{
  unsigned i;
  struct m68hc11_page_info *pinfo;
  struct bfd_link_hash_entry *h;
  struct m68hc11_elf_link_hash_table *htab;

  htab = m68hc11_elf_hash_table (info);
  if (htab == NULL)
    return;

  pinfo = & htab->pinfo;
  if (pinfo->bank_param_initialized)
    return;

  pinfo->bank_virtual = M68HC12_BANK_VIRT;
  pinfo->bank_mask = M68HC12_BANK_MASK;
  pinfo->bank_physical = M68HC12_BANK_BASE;
  pinfo->bank_shift = M68HC12_BANK_SHIFT;
  pinfo->bank_size = 1 << M68HC12_BANK_SHIFT;

  h = bfd_link_hash_lookup (info->hash, BFD_M68HC11_BANK_START_NAME,
			    false, false, true);
  if (h != (struct bfd_link_hash_entry*) NULL
      && h->type == bfd_link_hash_defined)
    pinfo->bank_physical = (h->u.def.value
			    + h->u.def.section->output_section->vma
			    + h->u.def.section->output_offset);

  h = bfd_link_hash_lookup (info->hash, BFD_M68HC11_BANK_VIRTUAL_NAME,
			    false, false, true);
  if (h != (struct bfd_link_hash_entry*) NULL
      && h->type == bfd_link_hash_defined)
    pinfo->bank_virtual = (h->u.def.value
			   + h->u.def.section->output_section->vma
			   + h->u.def.section->output_offset);

  h = bfd_link_hash_lookup (info->hash, BFD_M68HC11_BANK_SIZE_NAME,
			    false, false, true);
  if (h != (struct bfd_link_hash_entry*) NULL
      && h->type == bfd_link_hash_defined)
    pinfo->bank_size = (h->u.def.value
			+ h->u.def.section->output_section->vma
			+ h->u.def.section->output_offset);

  pinfo->bank_shift = 0;
  for (i = pinfo->bank_size; i != 0; i >>= 1)
    pinfo->bank_shift++;
  pinfo->bank_shift--;
  pinfo->bank_mask = (1 << pinfo->bank_shift) - 1;
  pinfo->bank_physical_end = pinfo->bank_physical + pinfo->bank_size;
  pinfo->bank_param_initialized = 1;

  h = bfd_link_hash_lookup (info->hash, "__far_trampoline", false,
			    false, true);
  if (h != (struct bfd_link_hash_entry*) NULL
      && h->type == bfd_link_hash_defined)
    pinfo->trampoline_addr = (h->u.def.value
			      + h->u.def.section->output_section->vma
			      + h->u.def.section->output_offset);
}

/* Return 1 if the address is in banked memory.
   This can be applied to a virtual address and to a physical address.  */
int
m68hc11_addr_is_banked (struct m68hc11_page_info *pinfo, bfd_vma addr)
{
  if (addr >= pinfo->bank_virtual)
    return 1;

  if (addr >= pinfo->bank_physical && addr <= pinfo->bank_physical_end)
    return 1;

  return 0;
}

/* Return the physical address seen by the processor, taking
   into account banked memory.  */
bfd_vma
m68hc11_phys_addr (struct m68hc11_page_info *pinfo, bfd_vma addr)
{
  if (addr < pinfo->bank_virtual)
    return addr;

  /* Map the address to the memory bank.  */
  addr -= pinfo->bank_virtual;
  addr &= pinfo->bank_mask;
  addr += pinfo->bank_physical;
  return addr;
}

/* Return the page number corresponding to an address in banked memory.  */
bfd_vma
m68hc11_phys_page (struct m68hc11_page_info *pinfo, bfd_vma addr)
{
  if (addr < pinfo->bank_virtual)
    return 0;

  /* Map the address to the memory bank.  */
  addr -= pinfo->bank_virtual;
  addr >>= pinfo->bank_shift;
  addr &= 0x0ff;
  return addr;
}

/* This function is used for relocs which are only used for relaxing,
   which the linker should otherwise ignore.  */

bfd_reloc_status_type
m68hc11_elf_ignore_reloc (bfd *abfd ATTRIBUTE_UNUSED,
			  arelent *reloc_entry,
			  asymbol *symbol ATTRIBUTE_UNUSED,
			  void *data ATTRIBUTE_UNUSED,
			  asection *input_section,
			  bfd *output_bfd,
			  char **error_message ATTRIBUTE_UNUSED)
{
  if (output_bfd != NULL)
    reloc_entry->address += input_section->output_offset;
  return bfd_reloc_ok;
}

bfd_reloc_status_type
m68hc11_elf_special_reloc (bfd *abfd ATTRIBUTE_UNUSED,
			   arelent *reloc_entry,
			   asymbol *symbol,
			   void *data ATTRIBUTE_UNUSED,
			   asection *input_section,
			   bfd *output_bfd,
			   char **error_message ATTRIBUTE_UNUSED)
{
  if (output_bfd != (bfd *) NULL
      && (symbol->flags & BSF_SECTION_SYM) == 0
      && (! reloc_entry->howto->partial_inplace
	  || reloc_entry->addend == 0))
    {
      reloc_entry->address += input_section->output_offset;
      return bfd_reloc_ok;
    }

  if (output_bfd != NULL)
    return bfd_reloc_continue;

  if (reloc_entry->address > bfd_get_section_limit (abfd, input_section))
    return bfd_reloc_outofrange;

  abort();
}

/* Look through the relocs for a section during the first phase.
   Since we don't do .gots or .plts, we just need to consider the
   virtual table relocs for gc.  */

bool
elf32_m68hc11_check_relocs (bfd *abfd, struct bfd_link_info *info,
			    asection *sec, const Elf_Internal_Rela *relocs)
{
  Elf_Internal_Shdr *		symtab_hdr;
  struct elf_link_hash_entry ** sym_hashes;
  const Elf_Internal_Rela *	rel;
  const Elf_Internal_Rela *	rel_end;

  if (bfd_link_relocatable (info))
    return true;

  symtab_hdr = & elf_tdata (abfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (abfd);
  rel_end = relocs + sec->reloc_count;

  for (rel = relocs; rel < rel_end; rel++)
    {
      struct elf_link_hash_entry * h;
      unsigned long r_symndx;

      r_symndx = ELF32_R_SYM (rel->r_info);

      if (r_symndx < symtab_hdr->sh_info)
	h = NULL;
      else
	{
	  h = sym_hashes [r_symndx - symtab_hdr->sh_info];
	  while (h->root.type == bfd_link_hash_indirect
		 || h->root.type == bfd_link_hash_warning)
	    h = (struct elf_link_hash_entry *) h->root.u.i.link;
	}

      switch (ELF32_R_TYPE (rel->r_info))
	{
	/* This relocation describes the C++ object vtable hierarchy.
	   Reconstruct it for later use during GC.  */
	case R_M68HC11_GNU_VTINHERIT:
	  if (!bfd_elf_gc_record_vtinherit (abfd, sec, h, rel->r_offset))
	    return false;
	  break;

	/* This relocation describes which C++ vtable entries are actually
	   used.  Record for later use during GC.  */
	case R_M68HC11_GNU_VTENTRY:
	  if (!bfd_elf_gc_record_vtentry (abfd, sec, h, rel->r_addend))
	    return false;
	  break;
	}
    }

  return true;
}

static bool ATTRIBUTE_PRINTF (6, 7)
reloc_warning (struct bfd_link_info *info, const char *name, bfd *input_bfd,
	       asection *input_section, const Elf_Internal_Rela *rel,
	       const char *fmt, ...)
{
  va_list ap;
  char *buf;
  int ret;

  va_start (ap, fmt);
  ret = vasprintf (&buf, fmt, ap);
  va_end (ap);
  if (ret < 0)
    {
      bfd_set_error (bfd_error_no_memory);
      return false;
    }
  info->callbacks->warning (info, buf, name, input_bfd, input_section,
			    rel->r_offset);
  free (buf);
  return true;
}

/* Relocate a 68hc11/68hc12 ELF section.  */
int
elf32_m68hc11_relocate_section (bfd *output_bfd ATTRIBUTE_UNUSED,
				struct bfd_link_info *info,
				bfd *input_bfd, asection *input_section,
				bfd_byte *contents, Elf_Internal_Rela *relocs,
				Elf_Internal_Sym *local_syms,
				asection **local_sections)
{
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  Elf_Internal_Rela *rel, *relend;
  const char *name = NULL;
  struct m68hc11_page_info *pinfo;
  const struct elf_backend_data * const ebd = get_elf_backend_data (input_bfd);
  struct m68hc11_elf_link_hash_table *htab;
  unsigned long e_flags;

  symtab_hdr = &elf_tdata (input_bfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (input_bfd);
  e_flags = elf_elfheader (input_bfd)->e_flags;

  htab = m68hc11_elf_hash_table (info);
  if (htab == NULL)
    return false;

  /* Get memory bank parameters.  */
  m68hc11_elf_get_bank_parameters (info);

  pinfo = & htab->pinfo;
  rel = relocs;
  relend = relocs + input_section->reloc_count;

  for (; rel < relend; rel++)
    {
      int r_type;
      arelent arel;
      reloc_howto_type *howto;
      unsigned long r_symndx;
      Elf_Internal_Sym *sym;
      asection *sec;
      bfd_vma relocation = 0;
      bfd_reloc_status_type r = bfd_reloc_undefined;
      bfd_vma phys_page;
      bfd_vma phys_addr;
      bfd_vma insn_addr;
      bfd_vma insn_page;
      bool is_far = false;
      bool is_xgate_symbol = false;
      bool is_section_symbol = false;
      struct elf_link_hash_entry *h;
      bfd_vma val;
      const char *msg;

      r_symndx = ELF32_R_SYM (rel->r_info);
      r_type = ELF32_R_TYPE (rel->r_info);

      if (r_type == R_M68HC11_GNU_VTENTRY
	  || r_type == R_M68HC11_GNU_VTINHERIT)
	continue;

      if (! (*ebd->elf_info_to_howto_rel) (input_bfd, &arel, rel))
	continue;
      howto = arel.howto;

      h = NULL;
      sym = NULL;
      sec = NULL;
      if (r_symndx < symtab_hdr->sh_info)
	{
	  sym = local_syms + r_symndx;
	  sec = local_sections[r_symndx];
	  relocation = (sec->output_section->vma
			+ sec->output_offset
			+ sym->st_value);
	  is_far = (sym && (sym->st_other & STO_M68HC12_FAR));
	  is_xgate_symbol = (sym && (sym->st_target_internal));
	  is_section_symbol = ELF_ST_TYPE (sym->st_info) & STT_SECTION;
	}
      else
	{
	  bool unresolved_reloc, warned, ignored;

	  RELOC_FOR_GLOBAL_SYMBOL (info, input_bfd, input_section, rel,
				   r_symndx, symtab_hdr, sym_hashes,
				   h, sec, relocation, unresolved_reloc,
				   warned, ignored);

	  is_far = (h && (h->other & STO_M68HC12_FAR));
	  is_xgate_symbol = (h && (h->target_internal));
	}

      if (sec != NULL && discarded_section (sec))
	RELOC_AGAINST_DISCARDED_SECTION (info, input_bfd, input_section,
					 rel, 1, relend, howto, 0, contents);

      if (bfd_link_relocatable (info))
	{
	  /* This is a relocatable link.  We don't have to change
	     anything, unless the reloc is against a section symbol,
	     in which case we have to adjust according to where the
	     section symbol winds up in the output section.  */
	  if (sym != NULL && ELF_ST_TYPE (sym->st_info) == STT_SECTION)
	    rel->r_addend += sec->output_offset;
	  continue;
	}

      if (h != NULL)
	name = h->root.root.string;
      else
	{
	  name = (bfd_elf_string_from_elf_section
		  (input_bfd, symtab_hdr->sh_link, sym->st_name));
	  if (name == NULL || *name == '\0')
	    name = bfd_section_name (sec);
	}

      if (is_far && ELF32_R_TYPE (rel->r_info) == R_M68HC11_16)
	{
	  struct elf32_m68hc11_stub_hash_entry* stub;

	  stub = m68hc12_stub_hash_lookup (htab->stub_hash_table,
					   name, false, false);
	  if (stub)
	    {
	      relocation = stub->stub_offset
		+ stub->stub_sec->output_section->vma
		+ stub->stub_sec->output_offset;
	      is_far = false;
	    }
	}

      /* Do the memory bank mapping.  */
      phys_addr = m68hc11_phys_addr (pinfo, relocation + rel->r_addend);
      phys_page = m68hc11_phys_page (pinfo, relocation + rel->r_addend);
      switch (r_type)
	{
	case R_M68HC12_LO8XG:
	  /* This relocation is specific to XGATE IMM16 calls and will precede
	     a HI8. tc-m68hc11 only generates them in pairs.
	     Leave the relocation to the HI8XG step.  */
	  r = bfd_reloc_ok;
	  r_type = R_M68HC11_NONE;
	  break;

	case R_M68HC12_HI8XG:
	  /* This relocation is specific to XGATE IMM16 calls and must follow
	     a LO8XG. Does not actually check that it was a LO8XG.
	     Adjusts high and low bytes.  */
	  relocation = phys_addr;
	  if ((e_flags & E_M68HC11_XGATE_RAMOFFSET)
	      && (relocation >= 0x2000))
	    relocation += 0xc000; /* HARDCODED RAM offset for XGATE.  */

	  /* Fetch 16 bit value including low byte in previous insn.  */
	  val = (bfd_get_8 (input_bfd, (bfd_byte*) contents + rel->r_offset) << 8)
	    | bfd_get_8 (input_bfd, (bfd_byte*) contents + rel->r_offset - 2);

	  /* Add on value to preserve carry, then write zero to high byte.  */
	  relocation += val;

	  /* Write out top byte.  */
	  bfd_put_8 (input_bfd, (relocation >> 8) & 0xff,
		     (bfd_byte*) contents + rel->r_offset);

	  /* Write out low byte to previous instruction.  */
	  bfd_put_8 (input_bfd, relocation & 0xff,
		     (bfd_byte*) contents + rel->r_offset - 2);

	  /* Mark as relocation completed.  */
	  r = bfd_reloc_ok;
	  r_type = R_M68HC11_NONE;
	  break;

	/* The HI8 and LO8 relocs are generated by %hi(expr) %lo(expr)
	   assembler directives. %hi does not support carry.  */
	case R_M68HC11_HI8:
	case R_M68HC11_LO8:
	  relocation = phys_addr;
	  break;

	case R_M68HC11_24:
	  /* Reloc used by 68HC12 call instruction.  */
	  bfd_put_16 (input_bfd, phys_addr,
		      (bfd_byte*) contents + rel->r_offset);
	  bfd_put_8 (input_bfd, phys_page,
		     (bfd_byte*) contents + rel->r_offset + 2);
	  r = bfd_reloc_ok;
	  r_type = R_M68HC11_NONE;
	  break;

	case R_M68HC11_NONE:
	  r = bfd_reloc_ok;
	  break;

	case R_M68HC11_LO16:
	  /* Reloc generated by %addr(expr) gas to obtain the
	     address as mapped in the memory bank window.  */
	  relocation = phys_addr;
	  break;

	case R_M68HC11_PAGE:
	  /* Reloc generated by %page(expr) gas to obtain the
	     page number associated with the address.  */
	  relocation = phys_page;
	  break;

	case R_M68HC11_16:
	  if (is_far)
	    {
	      if (!reloc_warning (info, name, input_bfd, input_section, rel,
				  _("reference to the far symbol `%s' using a "
				    "wrong relocation may result in incorrect "
				    "execution"), name))
		return false;
	    }

	  /* Get virtual address of instruction having the relocation.  */
	  insn_addr = input_section->output_section->vma
	    + input_section->output_offset
	    + rel->r_offset;

	  insn_page = m68hc11_phys_page (pinfo, insn_addr);

	 /* If we are linking an S12 instruction against an XGATE symbol, we
	    need to change the offset of the symbol value so that it's correct
	    from the S12's perspective.  */
	  if (is_xgate_symbol)
	    {
	      /* The ram in the global space is mapped to 0x2000 in the 16-bit
		 address space for S12 and 0xE000 in the 16-bit address space
		 for XGATE.  */
	      if (relocation >= 0xE000)
		{
		  /* We offset the address by the difference
		     between these two mappings.  */
		  relocation -= 0xC000;
		  break;
		}
	      else
		{
		  if (!reloc_warning (info, name, input_bfd, input_section, rel,
				      _("XGATE address (%lx) is not within "
					"shared RAM(0xE000-0xFFFF), therefore "
					"you must manually offset the address, "
					"and possibly manage the page, in your "
					"code."), (long) phys_addr))
		    return false;
		  break;
		}
	    }

	  if (m68hc11_addr_is_banked (pinfo, relocation + rel->r_addend)
	      && m68hc11_addr_is_banked (pinfo, insn_addr)
	      && phys_page != insn_page
	      && !(e_flags & E_M68HC11_NO_BANK_WARNING))
	    {
	      if (!reloc_warning (info, name, input_bfd, input_section, rel,
				  _("banked address [%lx:%04lx] (%lx) is not "
				    "in the same bank as current banked "
				    "address [%lx:%04lx] (%lx)"),
				  (long) phys_page, (long) phys_addr,
				  (long) (relocation + rel->r_addend),
				  (long) insn_page,
				  (long) m68hc11_phys_addr (pinfo, insn_addr),
				  (long) insn_addr))
		return false;
	      break;
	    }

	  if (phys_page != 0 && insn_page == 0)
	    {
	      if (!reloc_warning (info, name, input_bfd, input_section, rel,
				  _("reference to a banked address [%lx:%04lx] "
				    "in the normal address space at %04lx"),
				  (long) phys_page, (long) phys_addr,
				  (long) insn_addr))
		return false;
	      relocation = phys_addr;
	      break;
	    }

	  /* If this is a banked address use the phys_addr so that
	     we stay in the banked window.  */
	  if (m68hc11_addr_is_banked (pinfo, relocation + rel->r_addend))
	    relocation = phys_addr;
	  break;
	}

      /* If we are linking an XGATE instruction against an S12 symbol, we
	 need to change the offset of the symbol value so that it's correct
	 from the XGATE's perspective.  */
      if (!strcmp (howto->name, "R_XGATE_IMM8_LO")
	  || !strcmp (howto->name, "R_XGATE_IMM8_HI"))
	{
	  /* We can only offset S12 addresses that lie within the non-paged
	     area of RAM.  */
	  if (!is_xgate_symbol && !is_section_symbol)
	    {
	      /* The ram in the global space is mapped to 0x2000 and stops at
		 0x4000 in the 16-bit address space for S12 and 0xE000 in the
		 16-bit address space for XGATE.  */
	      if (relocation >= 0x2000 && relocation < 0x4000)
		 /* We offset the address by the difference
		   between these two mappings.  */
		relocation += 0xC000;
	      else
		{
		  if (!reloc_warning (info, name, input_bfd, input_section, rel,
				      _("S12 address (%lx) is not within "
					"shared RAM(0x2000-0x4000), therefore "
					"you must manually offset the address "
					"in your code"), (long) phys_addr))
		    return false;
		  break;
		}
	    }
	}

      if (r_type != R_M68HC11_NONE)
	{
	  if ((r_type == R_M68HC12_PCREL_9) || (r_type == R_M68HC12_PCREL_10))
	    r = _bfd_final_link_relocate (howto, input_bfd, input_section,
				      contents, rel->r_offset,
				      relocation - 2, rel->r_addend);
	  else
	    r = _bfd_final_link_relocate (howto, input_bfd, input_section,
					  contents, rel->r_offset,
					  relocation, rel->r_addend);
	}

      if (r != bfd_reloc_ok)
	{
	  switch (r)
	    {
	    case bfd_reloc_overflow:
	      (*info->callbacks->reloc_overflow)
		(info, NULL, name, howto->name, (bfd_vma) 0,
		 input_bfd, input_section, rel->r_offset);
	      break;

	    case bfd_reloc_undefined:
	      (*info->callbacks->undefined_symbol)
		(info, name, input_bfd, input_section, rel->r_offset, true);
	      break;

	    case bfd_reloc_outofrange:
	      msg = _ ("internal error: out of range error");
	      goto common_error;

	    case bfd_reloc_notsupported:
	      msg = _ ("internal error: unsupported relocation error");
	      goto common_error;

	    case bfd_reloc_dangerous:
	      msg = _ ("internal error: dangerous error");
	      goto common_error;

	    default:
	      msg = _ ("internal error: unknown error");
	      /* fall through */

	    common_error:
	      (*info->callbacks->warning) (info, msg, name, input_bfd,
					   input_section, rel->r_offset);
	      break;
	    }
	}
    }

  return true;
}



/* Set and control ELF flags in ELF header.  */

bool
_bfd_m68hc11_elf_set_private_flags (bfd *abfd, flagword flags)
{
  BFD_ASSERT (!elf_flags_init (abfd)
	      || elf_elfheader (abfd)->e_flags == flags);

  elf_elfheader (abfd)->e_flags = flags;
  elf_flags_init (abfd) = true;
  return true;
}

/* Merge backend specific data from an object file to the output
   object file when linking.  */

bool
_bfd_m68hc11_elf_merge_private_bfd_data (bfd *ibfd, struct bfd_link_info *info)
{
  bfd *obfd = info->output_bfd;
  flagword old_flags;
  flagword new_flags;
  bool ok = true;

  /* Check if we have the same endianness */
  if (!_bfd_generic_verify_endian_match (ibfd, info))
    return false;

  if (bfd_get_flavour (ibfd) != bfd_target_elf_flavour
      || bfd_get_flavour (obfd) != bfd_target_elf_flavour)
    return true;

  new_flags = elf_elfheader (ibfd)->e_flags;
  elf_elfheader (obfd)->e_flags |= new_flags & EF_M68HC11_ABI;
  old_flags = elf_elfheader (obfd)->e_flags;

  if (! elf_flags_init (obfd))
    {
      elf_flags_init (obfd) = true;
      elf_elfheader (obfd)->e_flags = new_flags;
      elf_elfheader (obfd)->e_ident[EI_CLASS]
	= elf_elfheader (ibfd)->e_ident[EI_CLASS];

      if (bfd_get_arch (obfd) == bfd_get_arch (ibfd)
	  && bfd_get_arch_info (obfd)->the_default)
	{
	  if (! bfd_set_arch_mach (obfd, bfd_get_arch (ibfd),
				   bfd_get_mach (ibfd)))
	    return false;
	}

      return true;
    }

  /* Check ABI compatibility.  */
  if ((new_flags & E_M68HC11_I32) != (old_flags & E_M68HC11_I32))
    {
      _bfd_error_handler
	(_("%pB: linking files compiled for 16-bit integers (-mshort) "
	   "and others for 32-bit integers"), ibfd);
      ok = false;
    }
  if ((new_flags & E_M68HC11_F64) != (old_flags & E_M68HC11_F64))
    {
      _bfd_error_handler
	(_("%pB: linking files compiled for 32-bit double (-fshort-double) "
	   "and others for 64-bit double"), ibfd);
      ok = false;
    }

  /* Processor compatibility.  */
  if (!EF_M68HC11_CAN_MERGE_MACH (new_flags, old_flags))
    {
      _bfd_error_handler
	(_("%pB: linking files compiled for HCS12 with "
	   "others compiled for HC12"), ibfd);
      ok = false;
    }
  new_flags = ((new_flags & ~EF_M68HC11_MACH_MASK)
	       | (EF_M68HC11_MERGE_MACH (new_flags, old_flags)));

  elf_elfheader (obfd)->e_flags = new_flags;

  new_flags &= ~(EF_M68HC11_ABI | EF_M68HC11_MACH_MASK);
  old_flags &= ~(EF_M68HC11_ABI | EF_M68HC11_MACH_MASK);

  /* Warn about any other mismatches */
  if (new_flags != old_flags)
    {
      _bfd_error_handler
	/* xgettext:c-format */
	(_("%pB: uses different e_flags (%#x) fields than previous modules (%#x)"),
	 ibfd, new_flags, old_flags);
      ok = false;
    }

  if (! ok)
    {
      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  return true;
}

bool
_bfd_m68hc11_elf_print_private_bfd_data (bfd *abfd, void *ptr)
{
  FILE *file = (FILE *) ptr;

  BFD_ASSERT (abfd != NULL && ptr != NULL);

  /* Print normal ELF private data.  */
  _bfd_elf_print_private_bfd_data (abfd, ptr);

  /* xgettext:c-format */
  fprintf (file, _("private flags = %lx:"), elf_elfheader (abfd)->e_flags);

  if (elf_elfheader (abfd)->e_flags & E_M68HC11_I32)
    fprintf (file, _("[abi=32-bit int, "));
  else
    fprintf (file, _("[abi=16-bit int, "));

  if (elf_elfheader (abfd)->e_flags & E_M68HC11_F64)
    fprintf (file, _("64-bit double, "));
  else
    fprintf (file, _("32-bit double, "));

  if (strcmp (bfd_get_target (abfd), "elf32-m68hc11") == 0)
    fprintf (file, _("cpu=HC11]"));
  else if (elf_elfheader (abfd)->e_flags & EF_M68HCS12_MACH)
    fprintf (file, _("cpu=HCS12]"));
  else
    fprintf (file, _("cpu=HC12]"));

  if (elf_elfheader (abfd)->e_flags & E_M68HC12_BANKS)
    fprintf (file, _(" [memory=bank-model]"));
  else
    fprintf (file, _(" [memory=flat]"));

  if (elf_elfheader (abfd)->e_flags & E_M68HC11_XGATE_RAMOFFSET)
    fprintf (file, _(" [XGATE RAM offsetting]"));

  fputc ('\n', file);

  return true;
}

static void scan_sections_for_abi (bfd *abfd ATTRIBUTE_UNUSED,
				   asection *asect, void *arg)
{
  struct m68hc11_scan_param* p = (struct m68hc11_scan_param*) arg;

  if (asect->vma >= p->pinfo->bank_virtual)
    p->use_memory_banks = true;
}

/* Tweak the OSABI field of the elf header.  */

bool
elf32_m68hc11_init_file_header (bfd *abfd, struct bfd_link_info *link_info)
{
  struct m68hc11_scan_param param;
  struct m68hc11_elf_link_hash_table *htab;

  if (!_bfd_elf_init_file_header (abfd, link_info))
    return false;

  if (link_info == NULL)
    return true;

  htab = m68hc11_elf_hash_table (link_info);
  if (htab == NULL)
    return true;

  m68hc11_elf_get_bank_parameters (link_info);

  param.use_memory_banks = false;
  param.pinfo = & htab->pinfo;

  bfd_map_over_sections (abfd, scan_sections_for_abi, &param);

  if (param.use_memory_banks)
    {
      Elf_Internal_Ehdr * i_ehdrp;

      i_ehdrp = elf_elfheader (abfd);
      i_ehdrp->e_flags |= E_M68HC12_BANKS;
    }
  return true;
}
