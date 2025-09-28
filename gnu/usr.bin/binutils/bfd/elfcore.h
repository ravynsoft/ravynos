/* ELF core file support for BFD.
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

char*
elf_core_file_failing_command (bfd *abfd)
{
  return elf_tdata (abfd)->core->command;
}

int
elf_core_file_failing_signal (bfd *abfd)
{
  return elf_tdata (abfd)->core->signal;
}

int
elf_core_file_pid (bfd *abfd)
{
  return elf_tdata (abfd)->core->pid;
}

bool
elf_core_file_matches_executable_p (bfd *core_bfd, bfd *exec_bfd)
{
  char* corename;

  /* xvecs must match if both are ELF files for the same target.  */

  if (core_bfd->xvec != exec_bfd->xvec)
    {
      bfd_set_error (bfd_error_system_call);
      return false;
    }

  /* If both BFDs have identical build-ids, then they match.  */
  if (core_bfd->build_id != NULL
      && exec_bfd->build_id != NULL
      && core_bfd->build_id->size == exec_bfd->build_id->size
      && memcmp (core_bfd->build_id->data, exec_bfd->build_id->data,
		 core_bfd->build_id->size) == 0)
    return true;

  /* See if the name in the corefile matches the executable name.  */
  corename = elf_tdata (core_bfd)->core->program;
  if (corename != NULL)
    {
      const char* execname = strrchr (bfd_get_filename (exec_bfd), '/');

      execname = execname ? execname + 1 : bfd_get_filename (exec_bfd);

      if (strcmp (execname, corename) != 0)
	return false;
    }

  return true;
}

/*  Core files are simply standard ELF formatted files that partition
    the file using the execution view of the file (program header table)
    rather than the linking view.  In fact, there is no section header
    table in a core file.

    The process status information (including the contents of the general
    register set) and the floating point register set are stored in a
    segment of type PT_NOTE.  We handcraft a couple of extra bfd sections
    that allow standard bfd access to the general registers (.reg) and the
    floating point registers (.reg2).  */

bfd_cleanup
elf_core_file_p (bfd *abfd)
{
  Elf_External_Ehdr x_ehdr;	/* Elf file header, external form.  */
  Elf_Internal_Ehdr *i_ehdrp;	/* Elf file header, internal form.  */
  Elf_Internal_Phdr *i_phdrp;	/* Elf program header, internal form.  */
  unsigned int phindex;
  const struct elf_backend_data *ebd;
  bfd_size_type amt;
  ufile_ptr filesize;

  /* Read in the ELF header in external format.  */
  if (bfd_bread (&x_ehdr, sizeof (x_ehdr), abfd) != sizeof (x_ehdr))
    {
      if (bfd_get_error () != bfd_error_system_call)
	goto wrong;
      else
	goto fail;
    }

  /* Check the magic number.  */
  if (! elf_file_p (&x_ehdr))
    goto wrong;

  /* FIXME: Check EI_VERSION here !  */

  /* Check the address size ("class").  */
  if (x_ehdr.e_ident[EI_CLASS] != ELFCLASS)
    goto wrong;

  /* Check the byteorder.  */
  switch (x_ehdr.e_ident[EI_DATA])
    {
    case ELFDATA2MSB:		/* Big-endian.  */
      if (! bfd_big_endian (abfd))
	goto wrong;
      break;
    case ELFDATA2LSB:		/* Little-endian.  */
      if (! bfd_little_endian (abfd))
	goto wrong;
      break;
    default:
      goto wrong;
    }

  /* Give abfd an elf_obj_tdata.  */
  if (! (*abfd->xvec->_bfd_set_format[bfd_core]) (abfd))
    goto fail;

  /* Swap in the rest of the header, now that we have the byte order.  */
  i_ehdrp = elf_elfheader (abfd);
  elf_swap_ehdr_in (abfd, &x_ehdr, i_ehdrp);

#if DEBUG & 1
  elf_debug_file (i_ehdrp);
#endif

  ebd = get_elf_backend_data (abfd);

  /* Check that the ELF e_machine field matches what this particular
     BFD format expects.  */

  if (ebd->elf_machine_code != i_ehdrp->e_machine
      && (ebd->elf_machine_alt1 == 0
	  || i_ehdrp->e_machine != ebd->elf_machine_alt1)
      && (ebd->elf_machine_alt2 == 0
	  || i_ehdrp->e_machine != ebd->elf_machine_alt2)
      && ebd->elf_machine_code != EM_NONE)
    goto wrong;

  if (ebd->elf_machine_code != EM_NONE
      && i_ehdrp->e_ident[EI_OSABI] != ebd->elf_osabi
      && ebd->elf_osabi != ELFOSABI_NONE)
    goto wrong;

  /* If there is no program header, or the type is not a core file, then
     we are hosed.  */
  if (i_ehdrp->e_phoff == 0 || i_ehdrp->e_type != ET_CORE)
    goto wrong;

  /* Does BFD's idea of the phdr size match the size
     recorded in the file? */
  if (i_ehdrp->e_phentsize != sizeof (Elf_External_Phdr))
    goto wrong;

  /* If the program header count is PN_XNUM(0xffff), the actual
     count is in the first section header.  */
  if (i_ehdrp->e_shoff != 0 && i_ehdrp->e_phnum == PN_XNUM)
    {
      Elf_External_Shdr x_shdr;
      Elf_Internal_Shdr i_shdr;
      file_ptr where = (file_ptr) i_ehdrp->e_shoff;

      if (i_ehdrp->e_shoff < sizeof (x_ehdr))
	goto wrong;

      /* Seek to the section header table in the file.  */
      if (bfd_seek (abfd, where, SEEK_SET) != 0)
	goto fail;

      /* Read the first section header at index 0, and convert to internal
	 form.  */
      if (bfd_bread (&x_shdr, sizeof (x_shdr), abfd) != sizeof (x_shdr))
	goto fail;
      elf_swap_shdr_in (abfd, &x_shdr, &i_shdr);

      if (i_shdr.sh_info != 0)
	{
	  i_ehdrp->e_phnum = i_shdr.sh_info;
	  if (i_ehdrp->e_phnum != i_shdr.sh_info)
	    goto wrong;
	}
    }

  /* Sanity check that we can read all of the program headers.
     It ought to be good enough to just read the last one.  */
  if (i_ehdrp->e_phnum > 1)
    {
      Elf_External_Phdr x_phdr;
      Elf_Internal_Phdr i_phdr;
      file_ptr where;

      /* Check that we don't have a totally silly number of
	 program headers.  */
      if (i_ehdrp->e_phnum > (unsigned int) -1 / sizeof (x_phdr)
	  || i_ehdrp->e_phnum > (unsigned int) -1 / sizeof (i_phdr))
	goto wrong;

      where = (file_ptr)(i_ehdrp->e_phoff + (i_ehdrp->e_phnum - 1) * sizeof (x_phdr));
      if ((bfd_size_type) where <= i_ehdrp->e_phoff)
	goto wrong;

      if (bfd_seek (abfd, where, SEEK_SET) != 0)
	goto fail;
      if (bfd_bread (&x_phdr, sizeof (x_phdr), abfd) != sizeof (x_phdr))
	goto fail;
    }

  /* Move to the start of the program headers.  */
  if (bfd_seek (abfd, (file_ptr) i_ehdrp->e_phoff, SEEK_SET) != 0)
    goto wrong;

  /* Allocate space for the program headers.  */
  amt = sizeof (*i_phdrp) * i_ehdrp->e_phnum;
  i_phdrp = (Elf_Internal_Phdr *) bfd_alloc (abfd, amt);
  if (!i_phdrp)
    goto fail;

  elf_tdata (abfd)->phdr = i_phdrp;

  /* Read and convert to internal form.  */
  for (phindex = 0; phindex < i_ehdrp->e_phnum; ++phindex)
    {
      Elf_External_Phdr x_phdr;

      if (bfd_bread (&x_phdr, sizeof (x_phdr), abfd) != sizeof (x_phdr))
	goto fail;

      elf_swap_phdr_in (abfd, &x_phdr, i_phdrp + phindex);
    }

  /* Set the machine architecture.  Do this before processing the
     program headers since we need to know the architecture type
     when processing the notes of some systems' core files.  */
  if (! bfd_default_set_arch_mach (abfd, ebd->arch, 0)
      /* It's OK if this fails for the generic target.  */
      && ebd->elf_machine_code != EM_NONE)
    goto fail;

  /* Let the backend double check the format and override global
     information.  We do this before processing the program headers
     to allow the correct machine (as opposed to just the default
     machine) to be set, making it possible for grok_prstatus and
     grok_psinfo to rely on the mach setting.  */
  if (ebd->elf_backend_object_p != NULL
      && ! ebd->elf_backend_object_p (abfd))
    goto wrong;

  /* Process each program header.  */
  for (phindex = 0; phindex < i_ehdrp->e_phnum; ++phindex)
    if (! bfd_section_from_phdr (abfd, i_phdrp + phindex, (int) phindex))
      goto fail;

  /* Check for core truncation.  */
  filesize = bfd_get_file_size (abfd);
  if (filesize != 0)
    {
      for (phindex = 0; phindex < i_ehdrp->e_phnum; ++phindex)
	{
	  Elf_Internal_Phdr *p = i_phdrp + phindex;
	  if (p->p_filesz
	      && (p->p_offset >= filesize
		  || p->p_filesz > filesize - p->p_offset))
	    {
	      _bfd_error_handler (_("warning: %pB has a segment "
				    "extending past end of file"), abfd);
	      abfd->read_only = 1;
	      break;
	    }
      }
  }

  /* Save the entry point from the ELF header.  */
  abfd->start_address = i_ehdrp->e_entry;
  return _bfd_no_cleanup;

 wrong:
  bfd_set_error (bfd_error_wrong_format);
 fail:
  return NULL;
}

/* Attempt to find a build-id in a core file from the core file BFD.
   OFFSET is the file offset to a PT_LOAD segment that may contain
   the build-id note.  Returns TRUE upon success, FALSE otherwise.  */

bool
NAME(_bfd_elf, core_find_build_id)
  (bfd *abfd,
   bfd_vma offset)
{
  Elf_External_Ehdr x_ehdr;	/* Elf file header, external form.   */
  Elf_Internal_Ehdr i_ehdr;	/* Elf file header, internal form.   */
  Elf_Internal_Phdr *i_phdr;
  unsigned int i;
  size_t amt;

  /* Seek to the position of the segment at OFFSET.  */
  if (bfd_seek (abfd, offset, SEEK_SET) != 0)
    goto fail;

  /* Read in the ELF header in external format.  */
  if (bfd_bread (&x_ehdr, sizeof (x_ehdr), abfd) != sizeof (x_ehdr))
    {
      if (bfd_get_error () != bfd_error_system_call)
	goto wrong;
      else
	goto fail;
    }

  /* Now check to see if we have a valid ELF file, and one that BFD can
     make use of.  The magic number must match, the address size ('class')
     and byte-swapping must match our XVEC entry, and it must have a
     section header table (FIXME: See comments re sections at top of this
     file).  */
  if (! elf_file_p (&x_ehdr)
      || x_ehdr.e_ident[EI_VERSION] != EV_CURRENT
      || x_ehdr.e_ident[EI_CLASS] != ELFCLASS)
    goto wrong;

  /* Check that file's byte order matches xvec's.  */
  switch (x_ehdr.e_ident[EI_DATA])
    {
    case ELFDATA2MSB:		/* Big-endian.  */
      if (! bfd_header_big_endian (abfd))
	goto wrong;
      break;
    case ELFDATA2LSB:		/* Little-endian.  */
      if (! bfd_header_little_endian (abfd))
	goto wrong;
      break;
    case ELFDATANONE:		/* No data encoding specified.  */
    default:			/* Unknown data encoding specified . */
      goto wrong;
    }

  elf_swap_ehdr_in (abfd, &x_ehdr, &i_ehdr);
#if DEBUG
  elf_debug_file (&i_ehdr);
#endif

  if (i_ehdr.e_phentsize != sizeof (Elf_External_Phdr) || i_ehdr.e_phnum == 0)
    goto fail;

  /* Read in program headers.  */
  if (_bfd_mul_overflow (i_ehdr.e_phnum, sizeof (*i_phdr), &amt))
    {
      bfd_set_error (bfd_error_file_too_big);
      goto fail;
    }
  i_phdr = (Elf_Internal_Phdr *) bfd_alloc (abfd, amt);
  if (i_phdr == NULL)
    goto fail;

  if (bfd_seek (abfd, (file_ptr) (offset + i_ehdr.e_phoff), SEEK_SET) != 0)
    goto fail;

  /* Read in program headers and parse notes.  */
  for (i = 0; i < i_ehdr.e_phnum; ++i, ++i_phdr)
    {
      Elf_External_Phdr x_phdr;

      if (bfd_bread (&x_phdr, sizeof (x_phdr), abfd) != sizeof (x_phdr))
	goto fail;
      elf_swap_phdr_in (abfd, &x_phdr, i_phdr);

      if (i_phdr->p_type == PT_NOTE && i_phdr->p_filesz > 0)
	{
	  elf_read_notes (abfd, offset + i_phdr->p_offset,
			  i_phdr->p_filesz, i_phdr->p_align);

	  /* Make sure ABFD returns to processing the program headers.  */
	  if (bfd_seek (abfd, (file_ptr) (offset + i_ehdr.e_phoff
					  + (i + 1) * sizeof (x_phdr)),
			SEEK_SET) != 0)
	    goto fail;

	  if (abfd->build_id != NULL)
	    return true;
	}
    }

  /* Having gotten this far, we have a valid ELF section, but no
     build-id was found.  */
  goto fail;

 wrong:
  bfd_set_error (bfd_error_wrong_format);
 fail:
  return false;
}
