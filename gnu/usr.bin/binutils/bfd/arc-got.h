/* ARC-specific support for 32-bit ELF
   Copyright (C) 1994-2023 Free Software Foundation, Inc.
   Contributed by Cupertino Miranda (cmiranda@synopsys.com).

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

#ifndef ARC_GOT_H
#define ARC_GOT_H

#define TCB_SIZE (8)

#define	align_power(addr, align)	\
  (((addr) + ((bfd_vma) 1 << (align)) - 1) & (-((bfd_vma) 1 << (align))))

enum tls_type_e
{
  GOT_UNKNOWN = 0,
  GOT_NORMAL,
  GOT_TLS_GD,
  GOT_TLS_IE,
  GOT_TLS_LE
};

enum tls_got_entries
{
  TLS_GOT_NONE = 0,
  TLS_GOT_MOD,
  TLS_GOT_OFF,
  TLS_GOT_MOD_AND_OFF
};

struct got_entry
{
  struct got_entry *next;
  enum tls_type_e type;
  bfd_vma offset;
  bool processed;
  bool created_dyn_relocation;
  enum tls_got_entries existing_entries;
};

/* Return the local got list, if not defined, create an empty one.  */

static struct got_entry **
arc_get_local_got_ents (bfd * abfd)
{
  if (elf_local_got_ents (abfd) == NULL)
    {
      bfd_size_type amt = (elf_tdata (abfd)->symtab_hdr.sh_info
			   * sizeof (*elf_local_got_ents (abfd)));
      elf_local_got_ents (abfd) = bfd_zmalloc (amt);
      if (elf_local_got_ents (abfd) == NULL)
	{
	  _bfd_error_handler (_("%pB: cannot allocate memory for local "
				"GOT entries"), abfd);
	  bfd_set_error (bfd_error_bad_value);
	  return NULL;
	}
    }

  return elf_local_got_ents (abfd);
}

static struct got_entry *
got_entry_for_type (struct got_entry **list,
		    enum tls_type_e type)
{
  struct got_entry **p = list;

  while (*p != NULL)
    {
      if ((*p)->type == type)
	return *p;
      p = &((*p)->next);
    }
  return NULL;
}

static void
new_got_entry_to_list (struct got_entry **list,
		       enum tls_type_e type,
		       bfd_vma offset,
		       enum tls_got_entries existing_entries)
{
  /* Find list end.  Avoid having multiple entries of the same
     type.  */
  struct got_entry **p = list;
  struct got_entry *entry;

  while (*p != NULL)
    {
      if ((*p)->type == type)
	return;
      p = &((*p)->next);
    }

  entry = (struct got_entry *) xmalloc (sizeof (struct got_entry));

  entry->type = type;
  entry->offset = offset;
  entry->next = NULL;
  entry->processed = false;
  entry->created_dyn_relocation = false;
  entry->existing_entries = existing_entries;

  ARC_DEBUG ("New GOT got entry added to list: "
	     "type: %d, offset: %ld, existing_entries: %d\n",
	     type, (long) offset, existing_entries);

  /* Add the entry to the end of the list.  */
  *p = entry;
}

static enum tls_type_e
tls_type_for_reloc (reloc_howto_type *howto)
{
  enum tls_type_e ret = GOT_UNKNOWN;

  if (is_reloc_for_GOT (howto))
    return GOT_NORMAL;

  switch (howto->type)
    {
    case R_ARC_TLS_GD_GOT:
      ret = GOT_TLS_GD;
      break;
    case R_ARC_TLS_IE_GOT:
      ret = GOT_TLS_IE;
      break;
    case R_ARC_TLS_LE_32:
      ret = GOT_TLS_LE;
      break;
    default:
      ret = GOT_UNKNOWN;
      break;
    }

  return ret;
};

static struct got_entry **
get_got_entry_list_for_symbol (bfd *abfd,
			       unsigned long r_symndx,
			       struct elf_link_hash_entry *h)
{
  struct elf_arc_link_hash_entry *h1 =
    ((struct elf_arc_link_hash_entry *) h);
  if (h1 != NULL)
    {
      return &h1->got_ents;
    }
  else
    {
      return arc_get_local_got_ents (abfd) + r_symndx;
    }
}


static enum tls_type_e
arc_got_entry_type_for_reloc (reloc_howto_type *howto)
{
  enum tls_type_e type = GOT_UNKNOWN;

  if (is_reloc_for_GOT (howto))
    return  GOT_NORMAL;

  if (is_reloc_for_TLS (howto))
    {
      switch (howto->type)
	{
	  case R_ARC_TLS_GD_GOT:
	    type = GOT_TLS_GD;
	    break;
	  case R_ARC_TLS_IE_GOT:
	    type = GOT_TLS_IE;
	    break;
	  default:
	    break;
	}
    }
  return type;
}

#define ADD_SYMBOL_REF_SEC_AND_RELOC(SECNAME, COND_FOR_RELOC, H)	\
  htab->s##SECNAME->size;						\
  {									\
    if (COND_FOR_RELOC)							\
      {									\
	htab->srel##SECNAME->size += sizeof (Elf32_External_Rela);	\
	  ARC_DEBUG ("arc_info: Added reloc space in "			\
		     #SECNAME " section at " __FILE__			\
		     ":%d for symbol %s\n",				\
		     __LINE__, name_for_global_symbol (H));		\
      }									\
    if (H)								\
      if (H->dynindx == -1 && !H->forced_local)				\
	if (! bfd_elf_link_record_dynamic_symbol (info, H))		\
	  return false;							\
     htab->s##SECNAME->size += 4;					\
   }									\

static bool
arc_fill_got_info_for_reloc (enum tls_type_e type,
			     struct got_entry **list,
			     struct bfd_link_info *  info,
			     struct elf_link_hash_entry *h)
{
  struct elf_link_hash_table *htab = elf_hash_table (info);

  if (got_entry_for_type (list, type) != NULL)
    return true;

  switch (type)
    {
      case GOT_NORMAL:
	{
	  bfd_vma offset
	    = ADD_SYMBOL_REF_SEC_AND_RELOC (got, bfd_link_pic (info)
						 || h != NULL, h);
	  new_got_entry_to_list (list, type, offset, TLS_GOT_NONE);
	}
	break;


      case GOT_TLS_GD:
	{
	  bfd_vma offset
	    = ADD_SYMBOL_REF_SEC_AND_RELOC (got, true, h);
	  bfd_vma ATTRIBUTE_UNUSED notneeded
	    = ADD_SYMBOL_REF_SEC_AND_RELOC (got, true, h);
	  new_got_entry_to_list (list, type, offset, TLS_GOT_MOD_AND_OFF);
	}
	break;
      case GOT_TLS_IE:
      case GOT_TLS_LE:
	{
	  bfd_vma offset
	    = ADD_SYMBOL_REF_SEC_AND_RELOC (got, true, h);
	  new_got_entry_to_list (list, type, offset, TLS_GOT_OFF);
	}
	break;

      default:
	return false;
	break;
    }
  return true;
}

struct arc_static_sym_data {
  bfd_vma sym_value;
  const char *symbol_name;
};

static struct arc_static_sym_data
get_static_sym_data (unsigned long  r_symndx,
		     Elf_Internal_Sym  *local_syms,
		     asection **local_sections,
		     struct elf_link_hash_entry *h,
		     struct arc_relocation_data *reloc_data)
{
  static const char local_name[] = "(local)";
  struct arc_static_sym_data ret = { 0, NULL };

  if (h != NULL)
    {
      BFD_ASSERT (h->root.type != bfd_link_hash_undefweak
		  && h->root.type != bfd_link_hash_undefined);
      /* TODO: This should not be here.  */
      reloc_data->sym_value = h->root.u.def.value;
      reloc_data->sym_section = h->root.u.def.section;

      ret.sym_value = h->root.u.def.value
	+ h->root.u.def.section->output_section->vma
	+ h->root.u.def.section->output_offset;

      ret.symbol_name = h->root.root.string;
    }
  else
  {
    Elf_Internal_Sym *sym = local_syms + r_symndx;
    asection *sec = local_sections[r_symndx];

    ret.sym_value = sym->st_value
      + sec->output_section->vma
      + sec->output_offset;

    ret.symbol_name = local_name;
  }
  return ret;
}

static bfd_vma
relocate_fix_got_relocs_for_got_info (struct got_entry **	   list_p,
				      enum tls_type_e		   type,
				      struct bfd_link_info *	   info,
				      bfd *			   output_bfd,
				      unsigned long		   r_symndx,
				      Elf_Internal_Sym *	   local_syms,
				      asection **		   local_sections,
				      struct elf_link_hash_entry * h,
				      struct arc_relocation_data * reloc_data)
{
  struct elf_link_hash_table *htab = elf_hash_table (info);
  struct got_entry *entry = NULL;

  if (list_p == NULL || type == GOT_UNKNOWN || type == GOT_TLS_LE)
    return 0;

  entry = got_entry_for_type (list_p, type);
  BFD_ASSERT (entry);

  if (h == NULL
      || h->forced_local == true
      || (! elf_hash_table (info)->dynamic_sections_created
	  || (bfd_link_pic (info)
	      && SYMBOL_REFERENCES_LOCAL (info, h))))
    {
      const char ATTRIBUTE_UNUSED *symbol_name;
      asection *tls_sec = elf_hash_table (info)->tls_sec;

      if (entry && !entry->processed)
	{
	  switch (entry->type)
	    {
	    case GOT_TLS_GD:
	      {
		BFD_ASSERT (tls_sec && tls_sec->output_section);
		bfd_vma sec_vma = tls_sec->output_section->vma;

		if (h == NULL || h->forced_local
		   || !elf_hash_table (info)->dynamic_sections_created)
		  {
		    struct arc_static_sym_data tmp =
		      get_static_sym_data (r_symndx, local_syms, local_sections,
					   h, reloc_data);

		    bfd_put_32 (output_bfd,
			    tmp.sym_value - sec_vma
			    + (elf_hash_table (info)->dynamic_sections_created
			       ? 0
			       : (align_power (0,
					       tls_sec->alignment_power))),
			    htab->sgot->contents + entry->offset
			    + (entry->existing_entries == TLS_GOT_MOD_AND_OFF
			       ? 4 : 0));

		    ARC_DEBUG ("arc_info: FIXED -> %s value = %#lx "
			  "@ %lx, for symbol %s\n",
			  (entry->type == GOT_TLS_GD ? "GOT_TLS_GD" :
			   "GOT_TLS_IE"),
			  (long) (sym_value - sec_vma),
			  (long) (htab->sgot->output_section->vma
			     + htab->sgot->output_offset
			     + entry->offset
			     + (entry->existing_entries == TLS_GOT_MOD_AND_OFF
				? 4 : 0)),
			  tmp.symbol_name);
		  }
	      }
	      break;

	    case GOT_TLS_IE:
	      {
		BFD_ASSERT (tls_sec && tls_sec->output_section);
		bfd_vma ATTRIBUTE_UNUSED sec_vma
		  = tls_sec->output_section->vma;

		struct arc_static_sym_data tmp =
		  get_static_sym_data (r_symndx, local_syms, local_sections,
				       h, reloc_data);

		bfd_put_32 (output_bfd,
			    tmp.sym_value - sec_vma
			    + (elf_hash_table (info)->dynamic_sections_created
			       ? 0
			       : (align_power (TCB_SIZE,
					       tls_sec->alignment_power))),
			    htab->sgot->contents + entry->offset
			    + (entry->existing_entries == TLS_GOT_MOD_AND_OFF
			       ? 4 : 0));

		ARC_DEBUG ("arc_info: FIXED -> %s value = %#lx "
			   "@ %p, for symbol %s\n",
			   (entry->type == GOT_TLS_GD ? "GOT_TLS_GD" :
			    "GOT_TLS_IE"),
			   (long) (sym_value - sec_vma),
			   (long) (htab->sgot->output_section->vma
			      + htab->sgot->output_offset
			      + entry->offset
			      + (entry->existing_entries == TLS_GOT_MOD_AND_OFF
				 ? 4 : 0)),
			   tmp.symbol_name);
	      }
	      break;

	    case GOT_NORMAL:
	      {
		bfd_vma sec_vma
		  = reloc_data->sym_section->output_section->vma
		  + reloc_data->sym_section->output_offset;

		if (h != NULL
		    && h->root.type == bfd_link_hash_undefweak)
		  ARC_DEBUG ("arc_info: PATCHED: NOT_PATCHED "
			     "@ %#08lx for sym %s in got offset %#lx "
			     "(is undefweak)\n",
			     (long) (htab->sgot->output_section->vma
				     + htab->sgot->output_offset
				     + entry->offset),
			     symbol_name,
			     (long) entry->offset);
		else
		  {
		    bfd_put_32 (output_bfd,
				reloc_data->sym_value + sec_vma,
				htab->sgot->contents + entry->offset);
		    ARC_DEBUG ("arc_info: PATCHED: %#08lx "
			       "@ %#08lx for sym %s in got offset %#lx\n",
			       (long) (reloc_data->sym_value + sec_vma),
			       (long) (htab->sgot->output_section->vma
				       + htab->sgot->output_offset
				       + entry->offset),
			       symbol_name,
			       (long) entry->offset);
		  }
	      }
	      break;
	    default:
	      BFD_ASSERT (0);
	      break;
	    }
	  entry->processed = true;
	}
    }

  return entry->offset;
}

static void
create_got_dynrelocs_for_single_entry (struct got_entry *list,
				       bfd *output_bfd,
				       struct bfd_link_info *  info,
				       struct elf_link_hash_entry *h)
{
  if (list == NULL)
    return;

  bfd_vma got_offset = list->offset;

  if (list->type == GOT_NORMAL
      && !list->created_dyn_relocation)
    {
      if (bfd_link_pic (info)
	  && h != NULL
	      && (info->symbolic || h->dynindx == -1)
	      && h->def_regular)
	{
	  ADD_RELA (output_bfd, got, got_offset, 0, R_ARC_RELATIVE, 0);
	}
      /* Do not fully understand the side effects of this condition.
	 The relocation space might still being reserved.  Perhaps
	 I should clear its value.  */
      else if (h != NULL && h->dynindx != -1)
	{
	  ADD_RELA (output_bfd, got, got_offset, h->dynindx, R_ARC_GLOB_DAT, 0);
	}
      list->created_dyn_relocation = true;
    }
  else if (list->existing_entries != TLS_GOT_NONE
	   && !list->created_dyn_relocation)
    {
       /* TODO TLS: This is not called for local symbols.
	  In order to correctly implement TLS, this should also
	  be called for all local symbols with tls got entries.
	  Should be moved to relocate_section in order to make it
	  work for local symbols.  */
      struct elf_link_hash_table *htab = elf_hash_table (info);
      enum tls_got_entries e = list->existing_entries;

      BFD_ASSERT (list->type != GOT_TLS_GD
		  || list->existing_entries == TLS_GOT_MOD_AND_OFF);

      bfd_vma dynindx = (h == NULL || h->dynindx == -1) ? 0 : h->dynindx;

      if (e == TLS_GOT_MOD_AND_OFF || e == TLS_GOT_MOD)
	{
	      ADD_RELA (output_bfd, got, got_offset, dynindx,
			R_ARC_TLS_DTPMOD, 0);
	      ARC_DEBUG ("arc_info: TLS_DYNRELOC: type = %d, \
GOT_OFFSET = %#lx, GOT_VMA = %#lx, INDEX = %ld, ADDEND = 0x0\n",
			 list->type,
			 (long) got_offset,
			 (long) (htab->sgot->output_section->vma
				 + htab->sgot->output_offset + got_offset),
			 (long) dynindx);
	}

      if (e == TLS_GOT_MOD_AND_OFF || e == TLS_GOT_OFF)
	{
	  bfd_vma addend = 0;
	  if (list->type == GOT_TLS_IE)
	  {
	    addend = bfd_get_32 (output_bfd,
				 htab->sgot->contents + got_offset);
	  }

	  ADD_RELA (output_bfd, got,
		    got_offset + (e == TLS_GOT_MOD_AND_OFF ? 4 : 0),
		    dynindx,
		    (list->type == GOT_TLS_IE ? R_ARC_TLS_TPOFF
					      : R_ARC_TLS_DTPOFF),
		    addend);

	  ARC_DEBUG ("arc_info: TLS_DYNRELOC: type = %d, \
GOT_OFFSET = %#lx, GOT_VMA = %#lx, INDEX = %ld, ADDEND = %#lx\n",
		     list->type,
		     (long) got_offset,
		     (long) (htab->sgot->output_section->vma
			     + htab->sgot->output_offset + got_offset),
		     (long) dynindx, (long) addend);
	}
      list->created_dyn_relocation = true;
    }
}

static void
create_got_dynrelocs_for_got_info (struct got_entry **list_p,
				   bfd *output_bfd,
				   struct bfd_link_info *  info,
				   struct elf_link_hash_entry *h)
{
  if (list_p == NULL)
    return;

  struct got_entry *list = *list_p;
  /* Traverse the list of got entries for this symbol.  */
  while (list)
    {
      create_got_dynrelocs_for_single_entry (list, output_bfd, info, h);
      list = list->next;
    }
}

#undef ADD_SYMBOL_REF_SEC_AND_RELOC

#endif /* ARC_GOT_H */
