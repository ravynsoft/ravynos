/* ELF program property support.
   Copyright (C) 2017-2023 Free Software Foundation, Inc.

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

/* GNU program property draft is at:

   https://github.com/hjl-tools/linux-abi/wiki/property-draft.pdf
 */

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"
#include "elf-bfd.h"

/* Get a property, allocate a new one if needed.  */

elf_property *
_bfd_elf_get_property (bfd *abfd, unsigned int type, unsigned int datasz)
{
  elf_property_list *p, **lastp;

  if (bfd_get_flavour (abfd) != bfd_target_elf_flavour)
    {
      /* Never should happen.  */
      abort ();
    }

  /* Keep the property list in order of type.  */
  lastp = &elf_properties (abfd);
  for (p = *lastp; p; p = p->next)
    {
      /* Reuse the existing entry.  */
      if (type == p->property.pr_type)
	{
	  if (datasz > p->property.pr_datasz)
	    {
	      /* This can happen when mixing 32-bit and 64-bit objects.  */
	      p->property.pr_datasz = datasz;
	    }
	  return &p->property;
	}
      else if (type < p->property.pr_type)
	break;
      lastp = &p->next;
    }
  p = (elf_property_list *) bfd_alloc (abfd, sizeof (*p));
  if (p == NULL)
    {
      _bfd_error_handler (_("%pB: out of memory in _bfd_elf_get_property"),
			  abfd);
      _exit (EXIT_FAILURE);
    }
  memset (p, 0, sizeof (*p));
  p->property.pr_type = type;
  p->property.pr_datasz = datasz;
  p->next = *lastp;
  *lastp = p;
  return &p->property;
}

/* Parse GNU properties.  */

bool
_bfd_elf_parse_gnu_properties (bfd *abfd, Elf_Internal_Note *note)
{
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  unsigned int align_size = bed->s->elfclass == ELFCLASS64 ? 8 : 4;
  bfd_byte *ptr = (bfd_byte *) note->descdata;
  bfd_byte *ptr_end = ptr + note->descsz;

  if (note->descsz < 8 || (note->descsz % align_size) != 0)
    {
    bad_size:
      _bfd_error_handler
	(_("warning: %pB: corrupt GNU_PROPERTY_TYPE (%ld) size: %#lx"),
	 abfd, note->type, note->descsz);
      return false;
    }

  while (ptr != ptr_end)
    {
      unsigned int type;
      unsigned int datasz;
      elf_property *prop;

      if ((size_t) (ptr_end - ptr) < 8)
	goto bad_size;

      type = bfd_h_get_32 (abfd, ptr);
      datasz = bfd_h_get_32 (abfd, ptr + 4);
      ptr += 8;

      if (datasz > (size_t) (ptr_end - ptr))
	{
	  _bfd_error_handler
	    (_("warning: %pB: corrupt GNU_PROPERTY_TYPE (%ld) type (0x%x) datasz: 0x%x"),
	     abfd, note->type, type, datasz);
	  /* Clear all properties.  */
	  elf_properties (abfd) = NULL;
	  return false;
	}

      if (type >= GNU_PROPERTY_LOPROC)
	{
	  if (bed->elf_machine_code == EM_NONE)
	    {
	      /* Ignore processor-specific properties with generic ELF
		 target vector.  They should be handled by the matching
		 ELF target vector.  */
	      goto next;
	    }
	  else if (type < GNU_PROPERTY_LOUSER
		   && bed->parse_gnu_properties)
	    {
	      enum elf_property_kind kind
		= bed->parse_gnu_properties (abfd, type, ptr, datasz);
	      if (kind == property_corrupt)
		{
		  /* Clear all properties.  */
		  elf_properties (abfd) = NULL;
		  return false;
		}
	      else if (kind != property_ignored)
		goto next;
	    }
	}
      else
	{
	  switch (type)
	    {
	    case GNU_PROPERTY_STACK_SIZE:
	      if (datasz != align_size)
		{
		  _bfd_error_handler
		    (_("warning: %pB: corrupt stack size: 0x%x"),
		     abfd, datasz);
		  /* Clear all properties.  */
		  elf_properties (abfd) = NULL;
		  return false;
		}
	      prop = _bfd_elf_get_property (abfd, type, datasz);
	      if (datasz == 8)
		prop->u.number = bfd_h_get_64 (abfd, ptr);
	      else
		prop->u.number = bfd_h_get_32 (abfd, ptr);
	      prop->pr_kind = property_number;
	      goto next;

	    case GNU_PROPERTY_NO_COPY_ON_PROTECTED:
	      if (datasz != 0)
		{
		  _bfd_error_handler
		    (_("warning: %pB: corrupt no copy on protected size: 0x%x"),
		     abfd, datasz);
		  /* Clear all properties.  */
		  elf_properties (abfd) = NULL;
		  return false;
		}
	      prop = _bfd_elf_get_property (abfd, type, datasz);
	      elf_has_no_copy_on_protected (abfd) = true;
	      prop->pr_kind = property_number;
	      goto next;

	    default:
	      if ((type >= GNU_PROPERTY_UINT32_AND_LO
		   && type <= GNU_PROPERTY_UINT32_AND_HI)
		  || (type >= GNU_PROPERTY_UINT32_OR_LO
		      && type <= GNU_PROPERTY_UINT32_OR_HI))
		{
		  if (datasz != 4)
		    {
		      _bfd_error_handler
			(_("error: %pB: <corrupt property (0x%x) size: 0x%x>"),
			 abfd, type, datasz);
		      /* Clear all properties.  */
		      elf_properties (abfd) = NULL;
		      return false;
		    }
		  prop = _bfd_elf_get_property (abfd, type, datasz);
		  prop->u.number |= bfd_h_get_32 (abfd, ptr);
		  prop->pr_kind = property_number;
		  if (type == GNU_PROPERTY_1_NEEDED
		      && ((prop->u.number
			   & GNU_PROPERTY_1_NEEDED_INDIRECT_EXTERN_ACCESS)
			  != 0))
		    {
		      elf_has_indirect_extern_access (abfd) = true;
		      /* GNU_PROPERTY_NO_COPY_ON_PROTECTED is implied.  */
		      elf_has_no_copy_on_protected (abfd) = true;
		    }
		  goto next;
		}
	      break;
	    }
	}

      _bfd_error_handler
	(_("warning: %pB: unsupported GNU_PROPERTY_TYPE (%ld) type: 0x%x"),
	 abfd, note->type, type);

    next:
      ptr += (datasz + (align_size - 1)) & ~ (align_size - 1);
    }

  return true;
}

/* Merge GNU property BPROP with APROP.  If APROP isn't NULL, return TRUE
   if APROP is updated.  Otherwise, return TRUE if BPROP should be merged
   with ABFD.  */

static bool
elf_merge_gnu_properties (struct bfd_link_info *info, bfd *abfd, bfd *bbfd,
			  elf_property *aprop, elf_property *bprop)
{
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  unsigned int pr_type = aprop != NULL ? aprop->pr_type : bprop->pr_type;
  unsigned int number;
  bool updated;

  if (bed->merge_gnu_properties != NULL
      && pr_type >= GNU_PROPERTY_LOPROC
      && pr_type < GNU_PROPERTY_LOUSER)
    return bed->merge_gnu_properties (info, abfd, bbfd, aprop, bprop);

  switch (pr_type)
    {
    case GNU_PROPERTY_STACK_SIZE:
      if (aprop != NULL && bprop != NULL)
	{
	  if (bprop->u.number > aprop->u.number)
	    {
	      aprop->u.number = bprop->u.number;
	      return true;
	    }
	  break;
	}
      /* FALLTHROUGH */

    case GNU_PROPERTY_NO_COPY_ON_PROTECTED:
      /* Return TRUE if APROP is NULL to indicate that BPROP should
	 be added to ABFD.  */
      return aprop == NULL;

    default:
      updated = false;
      if (pr_type >= GNU_PROPERTY_UINT32_OR_LO
	  && pr_type <= GNU_PROPERTY_UINT32_OR_HI)
	{
	  if (aprop != NULL && bprop != NULL)
	    {
	      number = aprop->u.number;
	      aprop->u.number = number | bprop->u.number;
	      /* Remove the property if all bits are empty.  */
	      if (aprop->u.number == 0)
		{
		  aprop->pr_kind = property_remove;
		  updated = true;
		}
	      else
		updated = number != (unsigned int) aprop->u.number;
	    }
	  else
	    {
	      /* Only one of APROP and BPROP can be NULL.  */
	      if (aprop != NULL)
		{
		  if (aprop->u.number == 0)
		    {
		      /* Remove APROP if all bits are empty.  */
		      aprop->pr_kind = property_remove;
		      updated = true;
		    }
		}
	      else
		{
		  /* Return TRUE if APROP is NULL and all bits of BPROP
		     aren't empty to indicate that BPROP should be added
		     to ABFD.  */
		  updated = bprop->u.number != 0;
		}
	    }
	  return updated;
	}
      else if (pr_type >= GNU_PROPERTY_UINT32_AND_LO
	       && pr_type <= GNU_PROPERTY_UINT32_AND_HI)
	{
	  /* Only one of APROP and BPROP can be NULL:
	     1. APROP & BPROP when both APROP and BPROP aren't NULL.
	     2. If APROP is NULL, remove x86 feature.
	     3. Otherwise, do nothing.
	     */
	  if (aprop != NULL && bprop != NULL)
	    {
	      number = aprop->u.number;
	      aprop->u.number = number & bprop->u.number;
	      updated = number != (unsigned int) aprop->u.number;
	      /* Remove the property if all feature bits are cleared.  */
	      if (aprop->u.number == 0)
		aprop->pr_kind = property_remove;
	    }
	  else
	    {
	      /* There should be no AND properties since some input
	         doesn't have them.   */
	      if (aprop != NULL)
		{
		  aprop->pr_kind = property_remove;
		  updated = true;
		}
	    }
	  return updated;
	}

      /* Never should happen.  */
      abort ();
    }

  return false;
}

/* Return the property of TYPE on *LISTP and remove it from *LISTP if RM is
   true.  Return NULL if not found.  */

static elf_property *
elf_find_and_remove_property (elf_property_list **listp,
			      unsigned int type, bool rm)
{
  elf_property_list *list;

  for (list = *listp; list; list = list->next)
    {
      if (type == list->property.pr_type)
	{
	  /* Remove this property.  */
	  if (rm)
	    *listp = list->next;
	  return &list->property;
	}
      else if (type < list->property.pr_type)
	break;
      listp = &list->next;
    }

  return NULL;
}

/* Merge GNU property list *LISTP in ABFD with FIRST_PBFD.  */

static void
elf_merge_gnu_property_list (struct bfd_link_info *info, bfd *first_pbfd,
			     bfd *abfd, elf_property_list **listp)
{
  elf_property_list *p, **lastp;
  elf_property *pr;
  bool number_p;
  bfd_vma number = 0;

  /* Merge each GNU property in FIRST_PBFD with the one on *LISTP.  */
  lastp = &elf_properties (first_pbfd);
  for (p = *lastp; p; p = p->next)
    if (p->property.pr_kind != property_remove)
      {
	if (p->property.pr_kind == property_number)
	  {
	    number_p = true;
	    number = p->property.u.number;
	  }
	else
	  number_p = false;
	pr = elf_find_and_remove_property (listp, p->property.pr_type,
					   true);
	/* Pass NULL to elf_merge_gnu_properties for the property which
	   isn't on *LISTP.  */
	elf_merge_gnu_properties (info, first_pbfd, abfd, &p->property, pr);
	if (p->property.pr_kind == property_remove)
	  {
	    if (info->has_map_file)
	      {
		if (number_p)
		  {
		    if (pr != NULL)
		      info->callbacks->minfo
			(_("Removed property %W to merge %pB (0x%v) "
			   "and %pB (0x%v)\n"),
			 (bfd_vma) p->property.pr_type, first_pbfd,
			 number, abfd, pr->u.number);
		    else
		      info->callbacks->minfo
			(_("Removed property %W to merge %pB (0x%v) "
			   "and %pB (not found)\n"),
			 (bfd_vma) p->property.pr_type, first_pbfd,
			 number, abfd);
		  }
		else
		  {
		    if (pr != NULL)
		      info->callbacks->minfo
			(_("Removed property %W to merge %pB and %pB\n"),
			 (bfd_vma) p->property.pr_type, first_pbfd, abfd);
		    else
		      info->callbacks->minfo
			(_("Removed property %W to merge %pB and %pB "
			   "(not found)\n"),
			 (bfd_vma) p->property.pr_type, first_pbfd, abfd);
		  }
	      }

	    /* Remove this property.  */
	    *lastp = p->next;
	    continue;
	  }
	else if (number_p)
	  {
	    if (pr != NULL)
	      {
		if (p->property.u.number != number
		    || p->property.u.number != pr->u.number)
		  info->callbacks->minfo
		    (_("Updated property %W (0x%v) to merge %pB (0x%v) "
		       "and %pB (0x%v)\n"),
		     (bfd_vma) p->property.pr_type, p->property.u.number,
		     first_pbfd, number, abfd, pr->u.number);
	      }
	    else
	      {
		if (p->property.u.number != number)
		  info->callbacks->minfo
		    (_("Updated property %W (%v) to merge %pB (0x%v) "
		       "and %pB (not found)\n"),
		     (bfd_vma) p->property.pr_type, p->property.u.number,
		     first_pbfd, number, abfd);
	      }
	  }
	lastp = &p->next;
      }

  /* Merge the remaining properties on *LISTP with FIRST_PBFD.  */
  for (p = *listp; p != NULL; p = p->next)
    {
      if (p->property.pr_kind == property_number)
	{
	  number_p = true;
	  number = p->property.u.number;
	}
      else
	number_p = false;

      if (elf_merge_gnu_properties (info, first_pbfd, abfd, NULL, &p->property))
	{
	  if (p->property.pr_type == GNU_PROPERTY_NO_COPY_ON_PROTECTED)
	    elf_has_no_copy_on_protected (first_pbfd) = true;

	  pr = _bfd_elf_get_property (first_pbfd, p->property.pr_type,
				      p->property.pr_datasz);
	  /* It must be a new property.  */
	  if (pr->pr_kind != property_unknown)
	    abort ();
	  /* Add a new property.  */
	  *pr = p->property;
	}
      else
	{
	  pr = elf_find_and_remove_property (&elf_properties (first_pbfd),
					     p->property.pr_type,
					     false);
	  if (pr == NULL)
	    {
	      if (number_p)
		info->callbacks->minfo
		  (_("Removed property %W to merge %pB (not found) and "
		     "%pB (0x%v)\n"),
		   (bfd_vma) p->property.pr_type, first_pbfd, abfd,
		   number);
	      else
		info->callbacks->minfo
		  (_("Removed property %W to merge %pB and %pB\n"),
		   (bfd_vma) p->property.pr_type, first_pbfd, abfd);
	    }
	  else if (pr->pr_kind != property_remove)
	    abort ();
	}
    }
}

/* Get GNU property section size.  */

static bfd_size_type
elf_get_gnu_property_section_size (elf_property_list *list,
				   unsigned int align_size)
{
  bfd_size_type size;
  unsigned int descsz;

  /* Compute the output section size.  */
  descsz = offsetof (Elf_External_Note, name[sizeof "GNU"]);
  descsz = (descsz + 3) & -(unsigned int) 4;
  size = descsz;
  for (; list != NULL; list = list->next)
    {
      unsigned int datasz;
      /* Check if this property should be skipped.  */
      if (list->property.pr_kind == property_remove)
	continue;
      /* There are 4 byte type + 4 byte datasz for each property.  */
      if (list->property.pr_type == GNU_PROPERTY_STACK_SIZE)
	datasz = align_size;
      else
	datasz = list->property.pr_datasz;
      size += 4 + 4 + datasz;
      /* Align each property.  */
      size = (size + (align_size - 1)) & ~(align_size - 1);
    }

  return size;
}

/* Write GNU properties.  */

static void
elf_write_gnu_properties (struct bfd_link_info *info,
			  bfd *abfd, bfd_byte *contents,
			  elf_property_list *list, unsigned int size,
			  unsigned int align_size)
{
  unsigned int descsz;
  unsigned int datasz;
  Elf_External_Note *e_note;

  e_note = (Elf_External_Note *) contents;
  descsz = offsetof (Elf_External_Note, name[sizeof "GNU"]);
  descsz = (descsz + 3) & -(unsigned int) 4;
  bfd_h_put_32 (abfd, sizeof "GNU", &e_note->namesz);
  bfd_h_put_32 (abfd, size - descsz, &e_note->descsz);
  bfd_h_put_32 (abfd, NT_GNU_PROPERTY_TYPE_0, &e_note->type);
  memcpy (e_note->name, "GNU", sizeof "GNU");

  size = descsz;
  for (; list != NULL; list = list->next)
    {
      /* Check if this property should be skipped.  */
      if (list->property.pr_kind == property_remove)
	continue;
      /* There are 4 byte type + 4 byte datasz for each property.  */
      if (list->property.pr_type == GNU_PROPERTY_STACK_SIZE)
	datasz = align_size;
      else
	datasz = list->property.pr_datasz;
      bfd_h_put_32 (abfd, list->property.pr_type, contents + size);
      bfd_h_put_32 (abfd, datasz, contents + size + 4);
      size += 4 + 4;

      /* Write out property value.  */
      switch (list->property.pr_kind)
	{
	case property_number:
	  switch (datasz)
	    {
	    default:
	      /* Never should happen.  */
	      abort ();

	    case 0:
	      break;

	    case 4:
	      /* Save the pointer to GNU_PROPERTY_1_NEEDED so that it
		 can be updated later if needed.  */
	      if (info != NULL
		  && list->property.pr_type == GNU_PROPERTY_1_NEEDED)
		info->needed_1_p = contents + size;
	      bfd_h_put_32 (abfd, list->property.u.number,
			    contents + size);
	      break;

	    case 8:
	      bfd_h_put_64 (abfd, list->property.u.number,
			    contents + size);
	      break;
	    }
	  break;

	default:
	  /* Never should happen.  */
	  abort ();
	}
      size += datasz;

      /* Align each property.  */
      size = (size + (align_size - 1)) & ~ (align_size - 1);
    }
}

/* Set up GNU properties.  Return the first relocatable ELF input with
   GNU properties if found.  Otherwise, return NULL.  */

bfd *
_bfd_elf_link_setup_gnu_properties (struct bfd_link_info *info)
{
  bfd *abfd, *first_pbfd = NULL, *elf_bfd = NULL;
  elf_property_list *list;
  asection *sec;
  bool has_properties = false;
  const struct elf_backend_data *bed
    = get_elf_backend_data (info->output_bfd);
  unsigned int elfclass = bed->s->elfclass;
  int elf_machine_code = bed->elf_machine_code;
  elf_property *p;

  /* Find the first relocatable ELF input with GNU properties.  */
  for (abfd = info->input_bfds; abfd != NULL; abfd = abfd->link.next)
    if (bfd_get_flavour (abfd) == bfd_target_elf_flavour
	&& (abfd->flags & DYNAMIC) == 0
	&& (elf_machine_code
	    == get_elf_backend_data (abfd)->elf_machine_code)
	&& (elfclass == get_elf_backend_data (abfd)->s->elfclass))
      {
	/* Ignore GNU properties from ELF objects with different machine
	   code or class.  Also skip objects without a GNU_PROPERTY note
	   section.  */
	elf_bfd = abfd;

	if (elf_properties (abfd) != NULL)
	  {
	    has_properties = true;

	    if (bfd_get_section_by_name (abfd,
					 NOTE_GNU_PROPERTY_SECTION_NAME)
		!= NULL)
	      {
		/* Keep .note.gnu.property section in FIRST_PBFD.  */
		first_pbfd = abfd;
		break;
	      }
	  }
      }

  if (info->indirect_extern_access > 0 && elf_bfd != NULL)
    {
      /* Support -z indirect-extern-access.  */
      if (first_pbfd == NULL)
	{
	  sec = bfd_make_section_with_flags (elf_bfd,
					     NOTE_GNU_PROPERTY_SECTION_NAME,
					     (SEC_ALLOC
					      | SEC_LOAD
					      | SEC_IN_MEMORY
					      | SEC_READONLY
					      | SEC_HAS_CONTENTS
					      | SEC_DATA));
	  if (sec == NULL)
	    info->callbacks->einfo (_("%F%P: failed to create GNU property section\n"));

	  if (!bfd_set_section_alignment (sec,
					  elfclass == ELFCLASS64 ? 3 : 2))
	    info->callbacks->einfo (_("%F%pA: failed to align section\n"),
				    sec);

	  elf_section_type (sec) = SHT_NOTE;
	  first_pbfd = elf_bfd;
	  has_properties = true;
	}

      p = _bfd_elf_get_property (first_pbfd, GNU_PROPERTY_1_NEEDED, 4);
      if (p->pr_kind == property_unknown)
	{
	  /* Create GNU_PROPERTY_1_NEEDED.  */
	  p->u.number
	    = GNU_PROPERTY_1_NEEDED_INDIRECT_EXTERN_ACCESS;
	  p->pr_kind = property_number;
	}
      else
	p->u.number
	  |= GNU_PROPERTY_1_NEEDED_INDIRECT_EXTERN_ACCESS;
    }

  /* Do nothing if there is no .note.gnu.property section.  */
  if (!has_properties)
    return NULL;

  /* Merge .note.gnu.property sections.  */
  info->callbacks->minfo (_("\n"));
  info->callbacks->minfo (_("Merging program properties\n"));
  info->callbacks->minfo (_("\n"));

  for (abfd = info->input_bfds; abfd != NULL; abfd = abfd->link.next)
    if (abfd != first_pbfd
	&& (abfd->flags & (DYNAMIC | BFD_PLUGIN | BFD_LINKER_CREATED)) == 0)
      {
	elf_property_list *null_ptr = NULL;
	elf_property_list **listp = &null_ptr;

	/* Merge .note.gnu.property section in relocatable ELF input.  */
	if (bfd_get_flavour (abfd) == bfd_target_elf_flavour)
	  {
	    list = elf_properties (abfd);

	    /* Ignore GNU properties from ELF objects with different
	       machine code.  */
	    if (list != NULL
		&& (elf_machine_code
		    == get_elf_backend_data (abfd)->elf_machine_code))
	      listp = &elf_properties (abfd);
	  }
	else
	  list = NULL;

	/* Merge properties with FIRST_PBFD.  FIRST_PBFD can be NULL
	   when all properties are from ELF objects with different
	   machine code or class.  */
	if (first_pbfd != NULL)
	  elf_merge_gnu_property_list (info, first_pbfd, abfd, listp);

	if (list != NULL)
	  {
	    /* Discard the .note.gnu.property section in this bfd.  */
	    sec = bfd_get_section_by_name (abfd,
					   NOTE_GNU_PROPERTY_SECTION_NAME);
	    if (sec != NULL)
	      sec->output_section = bfd_abs_section_ptr;
	  }
      }

  /* Rewrite .note.gnu.property section so that GNU properties are
     always sorted by type even if input GNU properties aren't sorted.  */
  if (first_pbfd != NULL)
    {
      bfd_size_type size;
      bfd_byte *contents;
      unsigned int align_size = elfclass == ELFCLASS64 ? 8 : 4;

      sec = bfd_get_section_by_name (first_pbfd,
				     NOTE_GNU_PROPERTY_SECTION_NAME);
      BFD_ASSERT (sec != NULL);

      /* Update stack size in .note.gnu.property with -z stack-size=N
	 if N > 0.  */
      if (info->stacksize > 0)
	{
	  bfd_vma stacksize = info->stacksize;

	  p = _bfd_elf_get_property (first_pbfd, GNU_PROPERTY_STACK_SIZE,
				     align_size);
	  if (p->pr_kind == property_unknown)
	    {
	      /* Create GNU_PROPERTY_STACK_SIZE.  */
	      p->u.number = stacksize;
	      p->pr_kind = property_number;
	    }
	  else if (stacksize > p->u.number)
	    p->u.number = stacksize;
	}
      else if (elf_properties (first_pbfd) == NULL)
	{
	  /* Discard .note.gnu.property section if all properties have
	     been removed.  */
	  sec->output_section = bfd_abs_section_ptr;
	  return NULL;
	}

      /* Fix up GNU properties.  */
      if (bed->fixup_gnu_properties)
	bed->fixup_gnu_properties (info, &elf_properties (first_pbfd));

      if (elf_properties (first_pbfd) == NULL)
	{
	  /* Discard .note.gnu.property section if all properties have
	     been removed.  */
	  sec->output_section = bfd_abs_section_ptr;
	  return NULL;
	}

      /* Compute the section size.  */
      list = elf_properties (first_pbfd);
      size = elf_get_gnu_property_section_size (list, align_size);

      /* Update .note.gnu.property section now.  */
      sec->size = size;
      contents = (bfd_byte *) bfd_zalloc (first_pbfd, size);

      if (info->indirect_extern_access <= 0)
	{
	  /* Get GNU_PROPERTY_1_NEEDED properties.  */
	  p = elf_find_and_remove_property (&elf_properties (first_pbfd),
					    GNU_PROPERTY_1_NEEDED, false);
	  if (p != NULL)
	    {
	      if (info->indirect_extern_access < 0)
		{
		  /* Set indirect_extern_access to 1 to indicate that
		     it is turned on by input properties.  */
		  if ((p->u.number
		       & GNU_PROPERTY_1_NEEDED_INDIRECT_EXTERN_ACCESS)
		      != 0)
		    info->indirect_extern_access = 1;
		}
	      else
		/* Turn off indirect external access.  */
		p->u.number
		  &= ~GNU_PROPERTY_1_NEEDED_INDIRECT_EXTERN_ACCESS;
	    }
	}

      elf_write_gnu_properties (info, first_pbfd, contents, list, size,
				align_size);

      /* Cache the section contents for elf_link_input_bfd.  */
      elf_section_data (sec)->this_hdr.contents = contents;

      /* If GNU_PROPERTY_NO_COPY_ON_PROTECTED is set, protected data
	 symbol is defined in the shared object.  */
      if (elf_has_no_copy_on_protected (first_pbfd))
	info->extern_protected_data = false;

      if (info->indirect_extern_access > 0)
	{
	  /* For indirect external access, don't generate copy
	     relocations.  NB: Set to nocopyreloc to 2 to indicate
	     that it is implied by indirect_extern_access.  */
	  info->nocopyreloc = 2;
	  info->extern_protected_data = false;
	}
    }

  return first_pbfd;
}

/* Convert GNU property size.  */

bfd_size_type
_bfd_elf_convert_gnu_property_size (bfd *ibfd, bfd *obfd)
{
  unsigned int align_size;
  const struct elf_backend_data *bed;
  elf_property_list *list = elf_properties (ibfd);

  bed = get_elf_backend_data (obfd);
  align_size = bed->s->elfclass == ELFCLASS64 ? 8 : 4;

  /* Get the output .note.gnu.property section size.  */
  return elf_get_gnu_property_section_size (list, align_size);
}

/* Convert GNU properties.  */

bool
_bfd_elf_convert_gnu_properties (bfd *ibfd, asection *isec,
				 bfd *obfd, bfd_byte **ptr,
				 bfd_size_type *ptr_size)
{
  unsigned int size;
  bfd_byte *contents;
  unsigned int align_shift;
  const struct elf_backend_data *bed;
  elf_property_list *list = elf_properties (ibfd);

  bed = get_elf_backend_data (obfd);
  align_shift = bed->s->elfclass == ELFCLASS64 ? 3 : 2;

  /* Get the output .note.gnu.property section size.  */
  size = bfd_section_size (isec->output_section);

  /* Update the output .note.gnu.property section alignment.  */
  bfd_set_section_alignment (isec->output_section, align_shift);

  if (size > bfd_section_size (isec))
    {
      contents = (bfd_byte *) bfd_malloc (size);
      if (contents == NULL)
	return false;
      free (*ptr);
      *ptr = contents;
    }
  else
    contents = *ptr;

  *ptr_size = size;

  /* Generate the output .note.gnu.property section.  */
  elf_write_gnu_properties (NULL, ibfd, contents, list, size,
			    1 << align_shift);

  return true;
}
