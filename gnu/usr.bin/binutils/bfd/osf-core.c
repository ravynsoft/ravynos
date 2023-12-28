/* BFD back-end for OSF/1 core files.
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


/* This file can only be compiled on systems which use OSF/1 style
   core files.  */

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"

#include <sys/user.h>
#ifdef OSF_CORE
#include <sys/core.h>
#endif

/* forward declarations */

#define osf_core_core_file_matches_executable_p generic_core_file_matches_executable_p
#define osf_core_core_file_pid _bfd_nocore_core_file_pid

/* These are stored in the bfd's tdata */

struct osf_core_struct
{
  int sig;
  char cmd[MAXCOMLEN + 1];
};

#define core_hdr(bfd) ((bfd)->tdata.osf_core_data)
#define core_signal(bfd)  (core_hdr(bfd)->sig)
#define core_command(bfd) (core_hdr(bfd)->cmd)

static asection *
make_bfd_asection (bfd *abfd,
		   const char *name,
		   flagword flags,
		   bfd_size_type size,
		   bfd_vma vma,
		   file_ptr filepos)
{
  asection *asect;

  asect = bfd_make_section_anyway_with_flags (abfd, name, flags);
  if (!asect)
    return NULL;

  asect->size = size;
  asect->vma = vma;
  asect->filepos = filepos;
  asect->alignment_power = 8;

  return asect;
}

static bfd_cleanup
osf_core_core_file_p (bfd *abfd)
{
  int val;
  int i;
  char *secname;
  struct core_filehdr core_header;
  size_t amt;

  amt = sizeof core_header;
  val = bfd_bread (& core_header, amt, abfd);
  if (val != sizeof core_header)
    return NULL;

  if (! startswith (core_header.magic, "Core"))
    return NULL;

  core_hdr (abfd) = (struct osf_core_struct *)
    bfd_zalloc (abfd, (bfd_size_type) sizeof (struct osf_core_struct));
  if (!core_hdr (abfd))
    return NULL;

  strncpy (core_command (abfd), core_header.name, MAXCOMLEN + 1);
  core_signal (abfd) = core_header.signo;

  for (i = 0; i < core_header.nscns; i++)
    {
      struct core_scnhdr core_scnhdr;
      flagword flags;

      amt = sizeof core_scnhdr;
      val = bfd_bread (& core_scnhdr, amt, abfd);
      if (val != sizeof core_scnhdr)
	break;

      /* Skip empty sections.  */
      if (core_scnhdr.size == 0 || core_scnhdr.scnptr == 0)
	continue;

      switch (core_scnhdr.scntype)
	{
	case SCNRGN:
	  secname = ".data";
	  flags = SEC_ALLOC + SEC_LOAD + SEC_HAS_CONTENTS;
	  break;
	case SCNSTACK:
	  secname = ".stack";
	  flags = SEC_ALLOC + SEC_LOAD + SEC_HAS_CONTENTS;
	  break;
	case SCNREGS:
	  secname = ".reg";
	  flags = SEC_HAS_CONTENTS;
	  break;
	default:
	  _bfd_error_handler (_("unhandled OSF/1 core file section type %d"),
			      core_scnhdr.scntype);
	  continue;
	}

      if (!make_bfd_asection (abfd, secname, flags,
			      (bfd_size_type) core_scnhdr.size,
			      (bfd_vma) core_scnhdr.vaddr,
			      (file_ptr) core_scnhdr.scnptr))
	goto fail;
    }

  /* OK, we believe you.  You're a core file (sure, sure).  */

  return _bfd_no_cleanup;

 fail:
  bfd_release (abfd, core_hdr (abfd));
  core_hdr (abfd) = NULL;
  bfd_section_list_clear (abfd);
  return NULL;
}

static char *
osf_core_core_file_failing_command (bfd *abfd)
{
  return core_command (abfd);
}

static int
osf_core_core_file_failing_signal (bfd *abfd)
{
  return core_signal (abfd);
}

/* If somebody calls any byte-swapping routines, shoot them.  */
static void
swap_abort (void)
{
  abort (); /* This way doesn't require any declaration for ANSI to fuck up */
}

#define	NO_GET ((bfd_vma (*) (const void *)) swap_abort)
#define	NO_PUT ((void (*) (bfd_vma, void *)) swap_abort)
#define	NO_GETS ((bfd_signed_vma (*) (const void *)) swap_abort)
#define	NO_GET64 ((uint64_t (*) (const void *)) swap_abort)
#define	NO_PUT64 ((void (*) (uint64_t, void *)) swap_abort)
#define	NO_GETS64 ((int64_t (*) (const void *)) swap_abort)

const bfd_target core_osf_vec =
  {
    "osf-core",
    bfd_target_unknown_flavour,
    BFD_ENDIAN_LITTLE,		/* target byte order */
    BFD_ENDIAN_LITTLE,		/* target headers byte order */
    (HAS_RELOC | EXEC_P |	/* object flags */
     HAS_LINENO | HAS_DEBUG |
     HAS_SYMS | HAS_LOCALS | WP_TEXT | D_PAGED),
    (SEC_HAS_CONTENTS | SEC_ALLOC | SEC_LOAD | SEC_RELOC), /* section flags */
    0,				/* symbol prefix */
    ' ',			/* ar_pad_char */
    16,				/* ar_max_namelen */
    0,				/* match priority.  */
    TARGET_KEEP_UNUSED_SECTION_SYMBOLS, /* keep unused section symbols.  */
    NO_GET64, NO_GETS64, NO_PUT64,	/* 64 bit data */
    NO_GET, NO_GETS, NO_PUT,		/* 32 bit data */
    NO_GET, NO_GETS, NO_PUT,		/* 16 bit data */
    NO_GET64, NO_GETS64, NO_PUT64,	/* 64 bit hdrs */
    NO_GET, NO_GETS, NO_PUT,		/* 32 bit hdrs */
    NO_GET, NO_GETS, NO_PUT,		/* 16 bit hdrs */

    {				/* bfd_check_format */
      _bfd_dummy_target,		/* unknown format */
      _bfd_dummy_target,		/* object file */
      _bfd_dummy_target,		/* archive */
      osf_core_core_file_p		/* a core file */
    },
    {				/* bfd_set_format */
      _bfd_bool_bfd_false_error,
      _bfd_bool_bfd_false_error,
      _bfd_bool_bfd_false_error,
      _bfd_bool_bfd_false_error
    },
    {				/* bfd_write_contents */
      _bfd_bool_bfd_false_error,
      _bfd_bool_bfd_false_error,
      _bfd_bool_bfd_false_error,
      _bfd_bool_bfd_false_error
    },

    BFD_JUMP_TABLE_GENERIC (_bfd_generic),
    BFD_JUMP_TABLE_COPY (_bfd_generic),
    BFD_JUMP_TABLE_CORE (osf_core),
    BFD_JUMP_TABLE_ARCHIVE (_bfd_noarchive),
    BFD_JUMP_TABLE_SYMBOLS (_bfd_nosymbols),
    BFD_JUMP_TABLE_RELOCS (_bfd_norelocs),
    BFD_JUMP_TABLE_WRITE (_bfd_generic),
    BFD_JUMP_TABLE_LINK (_bfd_nolink),
    BFD_JUMP_TABLE_DYNAMIC (_bfd_nodynamic),

    NULL,

    NULL			/* backend_data */
  };
