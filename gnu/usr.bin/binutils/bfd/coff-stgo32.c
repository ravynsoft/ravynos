/* BFD back-end for Intel 386 COFF files (DJGPP variant with a stub).
   Copyright (C) 1997-2023 Free Software Foundation, Inc.
   Written by Robert Hoehne.

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

/* This file handles now also stubbed coff images. The stub is a small
   DOS executable program before the coff image to load it in memory
   and execute it. This is needed, because DOS cannot run coff files.

   The COFF image is loaded in memory without the stub attached, so
   all offsets are relative to the beginning of the image, not the
   actual file.  We handle this in bfd by setting bfd->origin to where
   the COFF image starts.  */

#define TARGET_SYM		i386_coff_go32stubbed_vec
#define TARGET_NAME		"coff-go32-exe"
#define TARGET_UNDERSCORE	'_'
#define COFF_GO32_EXE
#define COFF_LONG_SECTION_NAMES
#define COFF_SUPPORT_GNU_LINKONCE
#define COFF_LONG_FILENAMES

#define COFF_SECTION_ALIGNMENT_ENTRIES \
{ COFF_SECTION_NAME_EXACT_MATCH (".data"), \
  COFF_ALIGNMENT_FIELD_EMPTY, COFF_ALIGNMENT_FIELD_EMPTY, 4 }, \
{ COFF_SECTION_NAME_EXACT_MATCH (".text"), \
  COFF_ALIGNMENT_FIELD_EMPTY, COFF_ALIGNMENT_FIELD_EMPTY, 4 }, \
{ COFF_SECTION_NAME_PARTIAL_MATCH (".debug"), \
  COFF_ALIGNMENT_FIELD_EMPTY, COFF_ALIGNMENT_FIELD_EMPTY, 0 }, \
{ COFF_SECTION_NAME_PARTIAL_MATCH (".gnu.linkonce.wi"), \
  COFF_ALIGNMENT_FIELD_EMPTY, COFF_ALIGNMENT_FIELD_EMPTY, 0 }

/* Section contains extended relocations. */
#define IMAGE_SCN_LNK_NRELOC_OVFL (0x01000000)

#include "sysdep.h"
#include "bfd.h"
#include "coff/msdos.h"

static bfd_cleanup go32exe_check_format (bfd *);
static bool go32exe_write_object_contents (bfd *);
static bool go32exe_mkobject (bfd *);
static bool go32exe_copy_private_bfd_data (bfd *, bfd *);

/* Defined in coff-go32.c.  */
bool _bfd_go32_mkobject (bfd *);
void _bfd_go32_swap_scnhdr_in (bfd *, void *, void *);
unsigned int _bfd_go32_swap_scnhdr_out (bfd *, void *, void *);

#define COFF_CHECK_FORMAT go32exe_check_format
#define COFF_WRITE_CONTENTS go32exe_write_object_contents
#define coff_mkobject go32exe_mkobject
#define coff_bfd_copy_private_bfd_data go32exe_copy_private_bfd_data
#define coff_SWAP_scnhdr_in _bfd_go32_swap_scnhdr_in
#define coff_SWAP_scnhdr_out _bfd_go32_swap_scnhdr_out

#include "coff-i386.c"

/* This macro is used, because I cannot assume the endianness of the
   host system.  */
#define _H(index) (H_GET_16 (abfd, (header + index * 2)))

/* These bytes are a 2048-byte DOS executable, which loads the COFF
   image into memory and then runs it. It is called 'stub'.  */
#define GO32EXE_DEFAULT_STUB_SIZE 2048
static const unsigned char go32exe_default_stub[GO32EXE_DEFAULT_STUB_SIZE] =
{
#include "go32stub.h"
};

/* Temporary location for stub read from input file.  */
static char * go32exe_temp_stub = NULL;
static bfd_size_type go32exe_temp_stub_size = 0;

/* That's the function, which creates the stub. There are
   different cases from where the stub is taken.
   At first the environment variable $(GO32STUB) is checked and then
   $(STUB) if it was not set.
   If it exists and points to a valid stub the stub is taken from
   that file. This file can be also a whole executable file, because
   the stub is computed from the exe information at the start of that
   file.

   If there was any error, the standard stub (compiled in this file)
   is taken.

   Ideally this function should exec '$(TARGET)-stubify' to generate
   a stub, like gcc does.  */

static void
go32exe_create_stub (bfd *abfd)
{
  /* Do it only once.  */
  if (coff_data (abfd)->stub == NULL)
    {
      char *stub;
      struct stat st;
      int f;
      unsigned char header[10];
      char magic[8];
      unsigned long coff_start;
      long exe_start;

      /* If we read a stub from an input file, use that one.  */
      if (go32exe_temp_stub != NULL)
	{
	  coff_data (abfd)->stub = bfd_alloc (abfd,
						  go32exe_temp_stub_size);
	  if (coff_data (abfd)->stub == NULL)
	    return;
	  memcpy (coff_data (abfd)->stub, go32exe_temp_stub,
		  go32exe_temp_stub_size);
	  coff_data (abfd)->stub_size = go32exe_temp_stub_size;
	  free (go32exe_temp_stub);
	  go32exe_temp_stub = NULL;
	  go32exe_temp_stub_size = 0;
	  return;
	}

      /* Check at first the environment variable $(GO32STUB).  */
      stub = getenv ("GO32STUB");
      /* Now check the environment variable $(STUB).  */
      if (stub == NULL)
	stub = getenv ("STUB");
      if (stub == NULL)
	goto stub_end;
      if (stat (stub, &st) != 0)
	goto stub_end;
#ifdef O_BINARY
      f = open (stub, O_RDONLY | O_BINARY);
#else
      f = open (stub, O_RDONLY);
#endif
      if (f < 0)
	goto stub_end;
      if (read (f, &header, sizeof (header)) < 0)
	{
	  close (f);
	  goto stub_end;
	}
      if (_H (0) != 0x5a4d)	/* It is not an exe file.  */
	{
	  close (f);
	  goto stub_end;
	}
      /* Compute the size of the stub (it is every thing up
	 to the beginning of the coff image).  */
      coff_start = (long) _H (2) * 512L;
      if (_H (1))
	coff_start += (long) _H (1) - 512L;

      exe_start = _H (4) * 16;
      if ((long) lseek (f, exe_start, SEEK_SET) != exe_start)
	{
	  close (f);
	  goto stub_end;
	}
      if (read (f, &magic, 8) != 8)
	{
	  close (f);
	  goto stub_end;
	}
      if (! startswith (magic, "go32stub"))
	{
	  close (f);
	  goto stub_end;
	}
      /* Now we found a correct stub (hopefully).  */
      coff_data (abfd)->stub = bfd_alloc (abfd, (bfd_size_type) coff_start);
      if (coff_data (abfd)->stub == NULL)
	{
	  close (f);
	  return;
	}
      lseek (f, 0L, SEEK_SET);
      if ((unsigned long) read (f, coff_data (abfd)->stub, coff_start)
	  != coff_start)
	{
	  bfd_release (abfd, coff_data (abfd)->stub);
	  coff_data (abfd)->stub = NULL;
	}
      else
	coff_data (abfd)->stub_size = coff_start;
      close (f);
    }
 stub_end:
  /* There was something wrong above, so use now the standard builtin
     stub.  */
  if (coff_data (abfd)->stub == NULL)
    {
      coff_data (abfd)->stub
	= bfd_alloc (abfd, (bfd_size_type) GO32EXE_DEFAULT_STUB_SIZE);
      if (coff_data (abfd)->stub == NULL)
	return;
      memcpy (coff_data (abfd)->stub, go32exe_default_stub,
	      GO32EXE_DEFAULT_STUB_SIZE);
      coff_data (abfd)->stub_size = GO32EXE_DEFAULT_STUB_SIZE;
    }
}

/* If ibfd was a stubbed coff image, copy the stub from that bfd
   to the new obfd.  */

static bool
go32exe_copy_private_bfd_data (bfd *ibfd, bfd *obfd)
{
  /* Check if both are the same targets.  */
  if (ibfd->xvec != obfd->xvec)
    return true;

  /* Make sure we have a source stub.  */
  BFD_ASSERT (coff_data (ibfd)->stub != NULL);

  /* Reallocate the output stub if necessary.  */
  if (coff_data (ibfd)->stub_size > coff_data (obfd)->stub_size)
    coff_data (obfd)->stub = bfd_alloc (obfd, coff_data (ibfd)->stub_size);
  if (coff_data (obfd)->stub == NULL)
    return false;

  /* Now copy the stub.  */
  memcpy (coff_data (obfd)->stub, coff_data (ibfd)->stub,
	  coff_data (ibfd)->stub_size);
  coff_data (obfd)->stub_size = coff_data (ibfd)->stub_size;
  obfd->origin = coff_data (obfd)->stub_size;

  return true;
}

/* Cleanup function, returned from check_format hook.  */

static void
go32exe_cleanup (bfd *abfd)
{
  abfd->origin = 0;
  coff_object_cleanup (abfd);

  free (go32exe_temp_stub);
  go32exe_temp_stub = NULL;
  go32exe_temp_stub_size = 0;
}

/* Check that there is a GO32 stub and read it to go32exe_temp_stub.
   Then set abfd->origin so that the COFF image is read at the correct
   file offset.  */

static bfd_cleanup
go32exe_check_format (bfd *abfd)
{
  struct external_DOS_hdr filehdr_dos;
  uint16_t num_pages;
  uint16_t last_page_size;
  uint32_t header_end;
  bfd_size_type stubsize;

  /* This format can not appear in an archive.  */
  if (abfd->origin != 0)
    {
      bfd_set_error (bfd_error_wrong_format);
      return NULL;
    }

  bfd_set_error (bfd_error_system_call);

  /* Read in the stub file header, which is a DOS MZ executable.  */
  if (bfd_bread (&filehdr_dos, DOS_HDR_SIZE, abfd) != DOS_HDR_SIZE)
    goto fail;

  /* Make sure that this is an MZ executable.  */
  if (H_GET_16 (abfd, filehdr_dos.e_magic) != IMAGE_DOS_SIGNATURE)
    goto fail_format;

  /* Determine the size of the stub  */
  num_pages = H_GET_16 (abfd, filehdr_dos.e_cp);
  last_page_size = H_GET_16 (abfd, filehdr_dos.e_cblp);
  stubsize = num_pages * 512;
  if (last_page_size != 0)
    stubsize += last_page_size - 512;

  ufile_ptr filesize = bfd_get_file_size (abfd);
  if (filesize != 0 && stubsize > filesize)
    goto fail_format;

  /* Save now the stub to be used later.  Put the stub data to a temporary
     location first as tdata still does not exist.  It may not even
     be ever created if we are just checking the file format of ABFD.  */
  bfd_seek (abfd, 0, SEEK_SET);
  go32exe_temp_stub = bfd_malloc (stubsize);
  if (go32exe_temp_stub == NULL)
    goto fail;
  if (bfd_bread (go32exe_temp_stub, stubsize, abfd) != stubsize)
    goto fail;
  go32exe_temp_stub_size = stubsize;

  /* Confirm that this is a go32stub.  */
  header_end = H_GET_16 (abfd, filehdr_dos.e_cparhdr) * 16UL;
  if (go32exe_temp_stub_size < header_end
      || go32exe_temp_stub_size - header_end < sizeof "go32stub" - 1
      || !startswith (go32exe_temp_stub + header_end, "go32stub"))
    goto fail_format;

  /* Set origin to where the COFF header starts and seek there.  */
  abfd->origin = stubsize;
  if (bfd_seek (abfd, 0, SEEK_SET) != 0)
    goto fail;

  /* Call coff_object_p to read the COFF image.  If this fails then the file
     must be just a stub with no COFF data attached.  */
  bfd_cleanup cleanup = coff_object_p (abfd);
  if (cleanup == NULL)
    goto fail;
  BFD_ASSERT (cleanup == coff_object_cleanup);

  return go32exe_cleanup;

 fail_format:
  bfd_set_error (bfd_error_wrong_format);
 fail:
  go32exe_cleanup (abfd);
  return NULL;
}

/* Write the stub to the output file, then call coff_write_object_contents.  */

static bool
go32exe_write_object_contents (bfd *abfd)
{
  const bfd_size_type pos = bfd_tell (abfd);
  const bfd_size_type stubsize = coff_data (abfd)->stub_size;

  BFD_ASSERT (stubsize != 0);

  bfd_set_error (bfd_error_system_call);

  /* Write the stub.  */
  abfd->origin = 0;
  if (bfd_seek (abfd, 0, SEEK_SET) != 0)
    return false;
  if (bfd_bwrite (coff_data (abfd)->stub, stubsize, abfd) != stubsize)
    return false;

  /* Seek back to where we were.  */
  abfd->origin = stubsize;
  if (bfd_seek (abfd, pos, SEEK_SET) != 0)
    return false;

  return coff_write_object_contents (abfd);
}

/* mkobject hook.  Called directly through bfd_set_format or via
   coff_mkobject_hook etc from bfd_check_format.  */

static bool
go32exe_mkobject (bfd *abfd)
{
  /* Don't output to an archive.  */
  if (abfd->my_archive != NULL)
    return false;

  if (!_bfd_go32_mkobject (abfd))
    return false;

  go32exe_create_stub (abfd);
  if (coff_data (abfd)->stub == NULL)
    {
      bfd_release (abfd, coff_data (abfd));
      return false;
    }
  abfd->origin = coff_data (abfd)->stub_size;

  return true;
}
