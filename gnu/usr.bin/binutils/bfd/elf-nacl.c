/* Native Client support for ELF
   Copyright (C) 2012-2023 Free Software Foundation, Inc.

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
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"
#include "elf-bfd.h"
#include "elf-nacl.h"
#include "elf/common.h"
#include "elf/internal.h"

static bool
segment_executable (struct elf_segment_map *seg)
{
  if (seg->p_flags_valid)
    return (seg->p_flags & PF_X) != 0;
  else
    {
      /* The p_flags value has not been computed yet,
	 so we have to look through the sections.  */
      unsigned int i;
      for (i = 0; i < seg->count; ++i)
	if (seg->sections[i]->flags & SEC_CODE)
	  return true;
    }
  return false;
}

/* Determine if this segment is eligible to receive the file and program
   headers.  It must be read-only and non-executable.
   Its first section must start far enough past the page boundary to
   allow space for the headers.  */
static bool
segment_eligible_for_headers (struct elf_segment_map *seg,
			      bfd_vma minpagesize, bfd_vma sizeof_headers)
{
  unsigned int i;
  if (seg->count == 0 || seg->sections[0]->lma % minpagesize < sizeof_headers)
    return false;
  for (i = 0; i < seg->count; ++i)
    {
      if ((seg->sections[i]->flags & (SEC_CODE|SEC_READONLY)) != SEC_READONLY)
	return false;
    }
  return true;
}


/* We permute the segment_map to get BFD to do the file layout we want:
   The first non-executable PT_LOAD segment appears first in the file
   and contains the ELF file header and phdrs.  */
bool
nacl_modify_segment_map (bfd *abfd, struct bfd_link_info *info)
{
  const struct elf_backend_data *const bed = get_elf_backend_data (abfd);
  struct elf_segment_map **m = &elf_seg_map (abfd);
  struct elf_segment_map **first_load = NULL;
  struct elf_segment_map **headers = NULL;
  int sizeof_headers;

  if (info != NULL && info->user_phdrs)
    /* The linker script used PHDRS explicitly, so don't change what the
       user asked for.  */
    return true;

  if (info != NULL)
    /* We're doing linking, so evalute SIZEOF_HEADERS as in a linker script.  */
    sizeof_headers = bfd_sizeof_headers (abfd, info);
  else
    {
      /* We're not doing linking, so this is objcopy or suchlike.
	 We just need to collect the size of the existing headers.  */
      struct elf_segment_map *seg;
      sizeof_headers = bed->s->sizeof_ehdr;
      for (seg = *m; seg != NULL; seg = seg->next)
	sizeof_headers += bed->s->sizeof_phdr;
    }

  while (*m != NULL)
    {
      struct elf_segment_map *seg = *m;

      if (seg->p_type == PT_LOAD)
	{
	  bool executable = segment_executable (seg);

	  if (executable
	      && seg->count > 0
	      && seg->sections[0]->vma % bed->minpagesize == 0)
	    {
	      asection *lastsec = seg->sections[seg->count - 1];
	      bfd_vma end = lastsec->vma + lastsec->size;
	      if (end % bed->minpagesize != 0)
		{
		  /* This is an executable segment that starts on a page
		     boundary but does not end on a page boundary.  Fill
		     it out to a whole page with code fill (the tail of
		     the segment will not be within any section).  Thus
		     the entire code segment can be mapped from the file
		     as whole pages and that mapping will contain only
		     valid instructions.

		     To accomplish this, we must fake out the code in
		     assign_file_positions_for_load_sections (elf.c) so
		     that it advances past the rest of the final page,
		     rather than trying to put the next (unaligned, or
		     unallocated) section.  We do this by appending a
		     dummy section record to this element in the segment
		     map.  No such output section ever actually exists,
		     but this gets the layout logic to advance the file
		     positions past this partial page.  Since we are
		     lying to BFD like this, nothing will ever know to
		     write the section contents.  So we do that by hand
		     after the fact, in nacl_final_write_processing, below.  */

		  struct elf_segment_map *newseg;
		  asection *sec;
		  struct bfd_elf_section_data *secdata;

		  BFD_ASSERT (!seg->p_size_valid);

		  secdata = bfd_zalloc (abfd, sizeof *secdata);
		  if (secdata == NULL)
		    return false;

		  sec = bfd_zalloc (abfd, sizeof *sec);
		  if (sec == NULL)
		    return false;

		  /* Fill in only the fields that actually affect the logic
		     in assign_file_positions_for_load_sections.  */
		  sec->vma = end;
		  sec->lma = lastsec->lma + lastsec->size;
		  sec->size = bed->minpagesize - (end % bed->minpagesize);
		  sec->flags = (SEC_ALLOC | SEC_LOAD
				| SEC_READONLY | SEC_CODE | SEC_LINKER_CREATED);
		  sec->used_by_bfd = secdata;

		  secdata->this_hdr.sh_type = SHT_PROGBITS;
		  secdata->this_hdr.sh_flags = SHF_ALLOC | SHF_EXECINSTR;
		  secdata->this_hdr.sh_addr = sec->vma;
		  secdata->this_hdr.sh_size = sec->size;

		  newseg
		    = bfd_alloc (abfd, (sizeof (*newseg)
					+ seg->count * sizeof (asection *)));
		  if (newseg == NULL)
		    return false;
		  memcpy (newseg, seg, (sizeof (*newseg) - sizeof (asection *)
					+ seg->count * sizeof (asection *)));
		  newseg->sections[newseg->count++] = sec;
		  *m = seg = newseg;
		}
	    }

	  /* First, we're just finding the earliest PT_LOAD.
	     By the normal rules, this will be the lowest-addressed one.  */
	  if (first_load == NULL)
	    first_load = m;

	  /* Now that we've noted the first PT_LOAD, we're looking for
	     the first non-executable PT_LOAD with a nonempty p_filesz.  */
	  else if (headers == NULL
		   && segment_eligible_for_headers (seg, bed->minpagesize,
						    sizeof_headers))
	    headers = m;
	}
      m = &seg->next;
    }

  if (headers != NULL)
    {
      struct elf_segment_map **last_load = NULL;
      struct elf_segment_map *seg;

      m = first_load;
      while ((seg = *m) != NULL)
	{
	  if (seg->p_type == PT_LOAD)
	    {
	      /* Clear the flags on any previous segment that
		 included the file header and phdrs.  */
	      seg->includes_filehdr = 0;
	      seg->includes_phdrs = 0;
	      seg->no_sort_lma = 1;
	      /* Also strip out empty segments.  */
	      if (seg->count == 0)
		{
		  if (headers == &seg->next)
		    headers = m;
		  *m = seg->next;
		  continue;
		}
	      last_load = m;
	    }
	  m = &seg->next;
	}

      /* This segment will include those headers instead.  */
      seg = *headers;
      seg->includes_filehdr = 1;
      seg->includes_phdrs = 1;

      if (last_load != NULL && first_load != last_load && first_load != headers)
	{
	  /* Put the first PT_LOAD header last.  */
	  struct elf_segment_map *first = *first_load;
	  struct elf_segment_map *last = *last_load;
	  *first_load = first->next;
	  first->next = last->next;
	  last->next = first;
	}
    }

  return true;
}

/* After nacl_modify_segment_map has done its work, the file layout has
   been done as we wanted.  But the PT_LOAD phdrs are no longer in the
   proper order for the ELF rule that they must appear in ascending address
   order.  So find the two segments we swapped before, and swap them back.  */
bool
nacl_modify_headers (bfd *abfd, struct bfd_link_info *info)
{
  if (info != NULL && info->user_phdrs)
    /* The linker script used PHDRS explicitly, so don't change what the
       user asked for.  */
    ;
  else
    {
      struct elf_segment_map **m = &elf_seg_map (abfd);
      Elf_Internal_Phdr *phdr = elf_tdata (abfd)->phdr;
      Elf_Internal_Phdr *p = phdr;

      /* Find the PT_LOAD that contains the headers (should be the first).  */
      while (*m != NULL)
	{
	  if ((*m)->p_type == PT_LOAD && (*m)->includes_filehdr)
	    break;

	  m = &(*m)->next;
	  ++p;
	}

      if (*m != NULL)
	{
	  struct elf_segment_map **first_load_seg = m;
	  Elf_Internal_Phdr *first_load_phdr = p;
	  struct elf_segment_map **next_load_seg = NULL;
	  Elf_Internal_Phdr *next_load_phdr = NULL;

	  /* Now move past that first one and find the PT_LOAD that should be
	     before it by address order.  */

	  m = &(*m)->next;
	  ++p;

	  while (*m != NULL)
	    {
	      if (p->p_type == PT_LOAD && p->p_vaddr < first_load_phdr->p_vaddr)
		{
		  next_load_seg = m;
		  next_load_phdr = p;
		  break;
		}

	      m = &(*m)->next;
	      ++p;
	    }

	  /* Swap their positions in the segment_map back to how they
	     used to be.  The phdrs have already been set up by now,
	     so we have to slide up the earlier ones to insert the one
	     that should be first.  */
	  if (next_load_seg != NULL)
	    {
	      Elf_Internal_Phdr move_phdr;
	      struct elf_segment_map *first_seg = *first_load_seg;
	      struct elf_segment_map *next_seg = *next_load_seg;
	      struct elf_segment_map *first_next = first_seg->next;
	      struct elf_segment_map *next_next = next_seg->next;

	      if (next_load_seg == &first_seg->next)
		{
		  *first_load_seg = next_seg;
		  next_seg->next = first_seg;
		  first_seg->next = next_next;
		}
	      else
		{
		  *first_load_seg = first_next;
		  *next_load_seg = next_next;

		  first_seg->next = *next_load_seg;
		  *next_load_seg = first_seg;

		  next_seg->next = *first_load_seg;
		  *first_load_seg = next_seg;
		}

	      move_phdr = *next_load_phdr;
	      memmove (first_load_phdr + 1, first_load_phdr,
		       (next_load_phdr - first_load_phdr) * sizeof move_phdr);
	      *first_load_phdr = move_phdr;
	    }
	}
    }

  return _bfd_elf_modify_headers (abfd, info);
}

bool
nacl_final_write_processing (bfd *abfd)
{
  struct elf_segment_map *seg;
  for (seg = elf_seg_map (abfd); seg != NULL; seg = seg->next)
    if (seg->p_type == PT_LOAD
	&& seg->count > 1
	&& seg->sections[seg->count - 1]->owner == NULL)
      {
	/* This is a fake section added in nacl_modify_segment_map, above.
	   It's not a real BFD section, so nothing wrote its contents.
	   Now write out its contents.  */

	asection *sec = seg->sections[seg->count - 1];
	char *fill;

	BFD_ASSERT (sec->flags & SEC_LINKER_CREATED);
	BFD_ASSERT (sec->flags & SEC_CODE);
	BFD_ASSERT (sec->size > 0);

	fill = abfd->arch_info->fill (sec->size, bfd_big_endian (abfd), true);

	if (fill == NULL
	    || bfd_seek (abfd, sec->filepos, SEEK_SET) != 0
	    || bfd_bwrite (fill, sec->size, abfd) != sec->size)
	  {
	    /* We don't have a proper way to report an error here.  So
	       instead fudge things so that elf_write_shdrs_and_ehdr will
	       fail.  */
	    elf_elfheader (abfd)->e_shoff = (file_ptr) -1;
	  }

	free (fill);
      }
  return _bfd_elf_final_write_processing (abfd);
}
