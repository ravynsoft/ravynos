/* BFD back-end for Intel 386 COFF files (DJGPP variant).
   Copyright (C) 1990-2023 Free Software Foundation, Inc.
   Written by DJ Delorie.

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

#define TARGET_SYM		i386_coff_go32_vec
#define TARGET_NAME		"coff-go32"
#define TARGET_UNDERSCORE	'_'
#define COFF_GO32
#define COFF_LONG_SECTION_NAMES
#define COFF_SUPPORT_GNU_LINKONCE
#define COFF_LONG_FILENAMES

#define COFF_SECTION_ALIGNMENT_ENTRIES \
{ COFF_SECTION_NAME_PARTIAL_MATCH (".data"), \
  COFF_ALIGNMENT_FIELD_EMPTY, COFF_ALIGNMENT_FIELD_EMPTY, 4 }, \
{ COFF_SECTION_NAME_PARTIAL_MATCH (".text"), \
  COFF_ALIGNMENT_FIELD_EMPTY, COFF_ALIGNMENT_FIELD_EMPTY, 4 }, \
{ COFF_SECTION_NAME_PARTIAL_MATCH (".const"), \
  COFF_ALIGNMENT_FIELD_EMPTY, COFF_ALIGNMENT_FIELD_EMPTY, 4 }, \
{ COFF_SECTION_NAME_PARTIAL_MATCH (".rodata"), \
  COFF_ALIGNMENT_FIELD_EMPTY, COFF_ALIGNMENT_FIELD_EMPTY, 4 }, \
{ COFF_SECTION_NAME_PARTIAL_MATCH (".bss"), \
  COFF_ALIGNMENT_FIELD_EMPTY, COFF_ALIGNMENT_FIELD_EMPTY, 4 }, \
{ COFF_SECTION_NAME_PARTIAL_MATCH (".gnu.linkonce.d"), \
  COFF_ALIGNMENT_FIELD_EMPTY, COFF_ALIGNMENT_FIELD_EMPTY, 4 }, \
{ COFF_SECTION_NAME_PARTIAL_MATCH (".gnu.linkonce.t"), \
  COFF_ALIGNMENT_FIELD_EMPTY, COFF_ALIGNMENT_FIELD_EMPTY, 4 }, \
{ COFF_SECTION_NAME_PARTIAL_MATCH (".gnu.linkonce.r"), \
  COFF_ALIGNMENT_FIELD_EMPTY, COFF_ALIGNMENT_FIELD_EMPTY, 4 }, \
{ COFF_SECTION_NAME_PARTIAL_MATCH (".gnu.linkonce.b"), \
  COFF_ALIGNMENT_FIELD_EMPTY, COFF_ALIGNMENT_FIELD_EMPTY, 4 }, \
{ COFF_SECTION_NAME_PARTIAL_MATCH (".debug"), \
  COFF_ALIGNMENT_FIELD_EMPTY, COFF_ALIGNMENT_FIELD_EMPTY, 0 }, \
{ COFF_SECTION_NAME_PARTIAL_MATCH (".gnu.linkonce.wi"), \
  COFF_ALIGNMENT_FIELD_EMPTY, COFF_ALIGNMENT_FIELD_EMPTY, 0 }

/* Section contains extended relocations. */
#define IMAGE_SCN_LNK_NRELOC_OVFL (0x01000000)

#include "sysdep.h"
#include "bfd.h"

/* The following functions are not static, because they are also
   used for coff-go32-exe (coff-stgo32.c).  */
bool _bfd_go32_mkobject (bfd *);
void _bfd_go32_swap_scnhdr_in (bfd *, void *, void *);
unsigned int _bfd_go32_swap_scnhdr_out (bfd *, void *, void *);

#define coff_mkobject _bfd_go32_mkobject
#define coff_SWAP_scnhdr_in _bfd_go32_swap_scnhdr_in
#define coff_SWAP_scnhdr_out _bfd_go32_swap_scnhdr_out

#include "coff-i386.c"

bool
_bfd_go32_mkobject (bfd * abfd)
{
  const bfd_size_type amt = sizeof (coff_data_type);

  abfd->tdata.coff_obj_data = bfd_zalloc (abfd, amt);
  if (abfd->tdata.coff_obj_data == NULL)
    return false;

  coff_data (abfd)->go32 = true;

  bfd_coff_long_section_names (abfd)
    = coff_backend_info (abfd)->_bfd_coff_long_section_names;

  return true;
}

void
_bfd_go32_swap_scnhdr_in (bfd * abfd, void * ext, void * in)
{
  SCNHDR *scnhdr_ext = (SCNHDR *) ext;
  struct internal_scnhdr *scnhdr_int = (struct internal_scnhdr *) in;

  memcpy (scnhdr_int->s_name, scnhdr_ext->s_name, sizeof (scnhdr_int->s_name));

  scnhdr_int->s_vaddr = GET_SCNHDR_VADDR (abfd, scnhdr_ext->s_vaddr);
  scnhdr_int->s_paddr = GET_SCNHDR_PADDR (abfd, scnhdr_ext->s_paddr);
  scnhdr_int->s_size = GET_SCNHDR_SIZE (abfd, scnhdr_ext->s_size);

  scnhdr_int->s_scnptr = GET_SCNHDR_SCNPTR (abfd, scnhdr_ext->s_scnptr);
  scnhdr_int->s_relptr = GET_SCNHDR_RELPTR (abfd, scnhdr_ext->s_relptr);
  scnhdr_int->s_lnnoptr = GET_SCNHDR_LNNOPTR (abfd, scnhdr_ext->s_lnnoptr);
  scnhdr_int->s_flags = GET_SCNHDR_FLAGS (abfd, scnhdr_ext->s_flags);
  scnhdr_int->s_nreloc = GET_SCNHDR_NRELOC (abfd, scnhdr_ext->s_nreloc);
  scnhdr_int->s_nlnno = GET_SCNHDR_NLNNO (abfd, scnhdr_ext->s_nlnno);

  /* DJGPP follows the same strategy as PE COFF.
     Iff the file is an executable then the higher 16 bits
     of the line number have been stored in the relocation
     counter field.  */
  if (abfd->flags & EXEC_P && (strcmp (scnhdr_ext->s_name, ".text") == 0))
    {
      scnhdr_int->s_nlnno
	= (GET_SCNHDR_NRELOC (abfd, scnhdr_ext->s_nreloc) << 16)
	  + GET_SCNHDR_NLNNO (abfd, scnhdr_ext->s_nlnno);
      scnhdr_int->s_nreloc = 0;
    }
}

unsigned int
_bfd_go32_swap_scnhdr_out (bfd * abfd, void * in, void * out)
{
  struct internal_scnhdr *scnhdr_int = (struct internal_scnhdr *) in;
  SCNHDR *scnhdr_ext = (SCNHDR *) out;
  unsigned int ret = bfd_coff_scnhsz (abfd);

  memcpy (scnhdr_ext->s_name, scnhdr_int->s_name, sizeof (scnhdr_int->s_name));

  PUT_SCNHDR_VADDR (abfd, scnhdr_int->s_vaddr, scnhdr_ext->s_vaddr);
  PUT_SCNHDR_PADDR (abfd, scnhdr_int->s_paddr, scnhdr_ext->s_paddr);
  PUT_SCNHDR_SIZE (abfd, scnhdr_int->s_size, scnhdr_ext->s_size);
  PUT_SCNHDR_SCNPTR (abfd, scnhdr_int->s_scnptr, scnhdr_ext->s_scnptr);
  PUT_SCNHDR_RELPTR (abfd, scnhdr_int->s_relptr, scnhdr_ext->s_relptr);
  PUT_SCNHDR_LNNOPTR (abfd, scnhdr_int->s_lnnoptr, scnhdr_ext->s_lnnoptr);
  PUT_SCNHDR_FLAGS (abfd, scnhdr_int->s_flags, scnhdr_ext->s_flags);

  if (abfd->flags & EXEC_P && (strcmp (scnhdr_int->s_name, ".text") == 0))
    {
      /* DJGPP follows the same strategy as PE COFF.
	 By inference from looking at MS output, the 32 bit field
	 which is the combination of the number_of_relocs and
	 number_of_linenos is used for the line number count in
	 executables.  A 16-bit field won't do for cc1.  The MS
	 document says that the number of relocs is zero for
	 executables, but the 17-th bit has been observed to be there.
	 Overflow is not an issue: a 4G-line program will overflow a
	 bunch of other fields long before this!  */
      PUT_SCNHDR_NLNNO (abfd, (scnhdr_int->s_nlnno & 0xffff),
			scnhdr_ext->s_nlnno);
      PUT_SCNHDR_NRELOC (abfd, (scnhdr_int->s_nlnno >> 16),
			 scnhdr_ext->s_nreloc);
    }
  else
    {
      /* DJGPP follows the same strategy as PE COFF.  */
      if (scnhdr_int->s_nlnno <= MAX_SCNHDR_NLNNO)
	PUT_SCNHDR_NLNNO (abfd, scnhdr_int->s_nlnno, scnhdr_ext->s_nlnno);
      else
	{
	  char buf[sizeof (scnhdr_int->s_name) + 1];

	  memcpy (buf, scnhdr_int->s_name, sizeof (scnhdr_int->s_name));
	  buf[sizeof (scnhdr_int->s_name)] = '\0';
	  _bfd_error_handler
	    /* xgettext:c-format */
	    (_("%pB: warning: %s: line number overflow: 0x%lx > 0xffff"),
	     abfd, buf, scnhdr_int->s_nlnno);
	  bfd_set_error (bfd_error_file_truncated);
	  PUT_SCNHDR_NLNNO (abfd, 0xffff, scnhdr_ext->s_nlnno);
	  ret = 0;
	}

      /* Although we could encode 0xffff relocs here, we do not, to be
	 consistent with other parts of bfd.  Also it lets us warn, as
	 we should never see 0xffff here w/o having the overflow flag
	 set.  */
      if (scnhdr_int->s_nreloc < MAX_SCNHDR_NRELOC)
	PUT_SCNHDR_NRELOC (abfd, scnhdr_int->s_nreloc, scnhdr_ext->s_nreloc);
      else
	{
	  /* DJGPP can deal with large #s of relocs, but not here.  */
	  PUT_SCNHDR_NRELOC (abfd, 0xffff, scnhdr_ext->s_nreloc);
	  scnhdr_int->s_flags |= IMAGE_SCN_LNK_NRELOC_OVFL;
	  PUT_SCNHDR_FLAGS (abfd, scnhdr_int->s_flags, scnhdr_ext->s_flags);
	}
    }

  return ret;
}
