/* Opening CTF files with BFD.
   Copyright (C) 2019-2023 Free Software Foundation, Inc.

   This file is part of libctf.

   libctf is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 3, or (at your option) any later
   version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not see
   <http://www.gnu.org/licenses/>.  */

#include <ctf-impl.h>
#include <stddef.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <elf.h>
#include <bfd.h>
#include "swap.h"
#include "ctf-endian.h"

#include "elf-bfd.h"

/* Free the BFD bits of a CTF file on ctf_arc_close().  */

static void
ctf_bfdclose (struct ctf_archive_internal *arci)
{
  if (arci->ctfi_abfd != NULL)
    if (!bfd_close_all_done (arci->ctfi_abfd))
      ctf_err_warn (NULL, 0, 0, _("cannot close BFD: %s"),
		    bfd_errmsg (bfd_get_error ()));
}

/* Open a CTF file given the specified BFD.  */

ctf_archive_t *
ctf_bfdopen (struct bfd *abfd, int *errp)
{
  ctf_archive_t *arc;
  asection *ctf_asect;
  bfd_byte *contents;
  ctf_sect_t ctfsect;

  libctf_init_debug();

  if ((ctf_asect = bfd_get_section_by_name (abfd, _CTF_SECTION)) == NULL)
    {
      return (ctf_set_open_errno (errp, ECTF_NOCTFDATA));
    }

  if (!bfd_malloc_and_get_section (abfd, ctf_asect, &contents))
    {
      ctf_err_warn (NULL, 0, 0, _("ctf_bfdopen(): cannot malloc "
				  "CTF section: %s"),
		    bfd_errmsg (bfd_get_error ()));
      return (ctf_set_open_errno (errp, ECTF_FMT));
    }

  ctfsect.cts_name = _CTF_SECTION;
  ctfsect.cts_entsize = 1;
  ctfsect.cts_size = bfd_section_size (ctf_asect);
  ctfsect.cts_data = contents;

  if ((arc = ctf_bfdopen_ctfsect (abfd, &ctfsect, errp)) != NULL)
    {
      /* This frees the cts_data later.  */
      arc->ctfi_data = (void *) ctfsect.cts_data;
      return arc;
    }

  free (contents);
  return NULL;				/* errno is set for us.  */
}

/* Open a CTF file given the specified BFD and CTF section (which may contain a
   CTF archive or a file).  */

ctf_archive_t *
ctf_bfdopen_ctfsect (struct bfd *abfd _libctf_unused_,
		     const ctf_sect_t *ctfsect, int *errp)
{
  ctf_archive_t *arci;
  ctf_sect_t *symsectp = NULL;
  ctf_sect_t *strsectp = NULL;
  const char *bfderrstr = NULL;
  char *strtab_alloc = NULL;
  int symsect_endianness = -1;

  libctf_init_debug();

#ifdef HAVE_BFD_ELF
  ctf_sect_t symsect, strsect;
  Elf_Internal_Shdr *symhdr;
  size_t symcount;
  Elf_Internal_Sym *isymbuf;
  bfd_byte *symtab = NULL;
  const char *symtab_name;
  const char *strtab = NULL;
  const char *strtab_name;
  size_t strsize;
  const ctf_preamble_t *preamble;

  if (ctfsect->cts_data == NULL)
    {
      bfderrstr = N_("CTF section is NULL");
      goto err;
    }
  preamble = ctf_arc_bufpreamble (ctfsect);

  if (preamble->ctp_flags & CTF_F_DYNSTR)
    {
      symhdr = &elf_tdata (abfd)->dynsymtab_hdr;
      strtab_name = ".dynstr";
      symtab_name = ".dynsym";
    }
  else
    {
      symhdr = &elf_tdata (abfd)->symtab_hdr;
      strtab_name = ".strtab";
      symtab_name = ".symtab";
    }

  /* TODO: handle SYMTAB_SHNDX.  */

  /* Get the symtab, and the strtab associated with it.  */
  if (elf_tdata (abfd) && symhdr && symhdr->sh_size && symhdr->sh_entsize)
    {
      symcount = symhdr->sh_size / symhdr->sh_entsize;
      if ((symtab = malloc (symhdr->sh_size)) == NULL)
	{
	  bfderrstr = N_("cannot malloc symbol table");
	  goto err;
	}

      isymbuf = bfd_elf_get_elf_syms (abfd, symhdr, symcount, 0,
				      NULL, symtab, NULL);
      free (isymbuf);
      if (isymbuf == NULL)
	{
	  bfderrstr = N_("cannot read symbol table");
	  goto err_free_sym;
	}

      if (elf_elfsections (abfd) != NULL
	  && symhdr->sh_link < elf_numsections (abfd))
	{
	  Elf_Internal_Shdr *strhdr = elf_elfsections (abfd)[symhdr->sh_link];

	  strsize = strhdr->sh_size;
	  if (strhdr->contents == NULL)
	    {
	      if ((strtab = bfd_elf_get_str_section (abfd, symhdr->sh_link)) == NULL)
		{
		  bfderrstr = N_("cannot read string table");
		  goto err_free_sym;
		}
	    }
	  else
	    strtab = (const char *) strhdr->contents;
	}
    }
  else		/* No symtab: just try getting .strtab or .dynstr by name.  */
    {
      bfd_byte *str_bcontents;
      asection *str_asect;

      if ((str_asect = bfd_get_section_by_name (abfd, strtab_name)) != NULL)
	{
	  if (bfd_malloc_and_get_section (abfd, str_asect, &str_bcontents))
	    {
	      strtab = (const char *) str_bcontents;
	      strtab_alloc = (char *) str_bcontents;
	      strsize = str_asect->size;
	    }
	}
    }

  if (strtab)
    {
      /* The names here are more or less arbitrary, but there is no point
	 thrashing around digging the name out of the shstrtab given that we don't
	 use it for anything but debugging.  */

      strsect.cts_data = strtab;
      strsect.cts_name = strtab_name;
      strsect.cts_size = strsize;
      strsectp = &strsect;
    }

  if (symtab)
    {
      assert (symhdr->sh_entsize == get_elf_backend_data (abfd)->s->sizeof_sym);
      symsect.cts_name = symtab_name;
      symsect.cts_entsize = symhdr->sh_entsize;
      symsect.cts_size = symhdr->sh_size;
      symsect.cts_data = symtab;
      symsectp = &symsect;
    }

  symsect_endianness = bfd_little_endian (abfd);
#endif

  arci = ctf_arc_bufopen (ctfsect, symsectp, strsectp, errp);
  if (arci)
    {
      /* Request freeing of the symsect and possibly the strsect.  */
      arci->ctfi_free_symsect = 1;
      if (strtab_alloc)
	arci->ctfi_free_strsect = 1;

      /* Get the endianness right.  */
      if (symsect_endianness > -1)
	ctf_arc_symsect_endianness (arci, symsect_endianness);
      return arci;
    }
#ifdef HAVE_BFD_ELF
 err_free_sym:
  free (symtab);
  free (strtab_alloc);
#endif
err: _libctf_unused_;
  if (bfderrstr)
    {
      ctf_err_warn (NULL, 0, 0, "ctf_bfdopen(): %s: %s", gettext (bfderrstr),
		   bfd_errmsg (bfd_get_error()));
      ctf_set_open_errno (errp, ECTF_FMT);
    }
  return NULL;
}

/* Open the specified file descriptor and return a pointer to a CTF archive that
   contains one or more CTF dicts.  The file can be an ELF file, a file
   containing raw CTF, or a CTF archive.  The caller is responsible for closing
   the file descriptor when it is no longer needed.  If this is an ELF file,
   TARGET, if non-NULL, should be the name of a suitable BFD target.  */

ctf_archive_t *
ctf_fdopen (int fd, const char *filename, const char *target, int *errp)
{
  ctf_archive_t *arci;
  bfd *abfd;
  int nfd;

  struct stat st;
  ssize_t nbytes;

  ctf_preamble_t ctfhdr;
  uint64_t arc_magic;

  memset (&ctfhdr, 0, sizeof (ctfhdr));

  libctf_init_debug();

  if (fstat (fd, &st) == -1)
    return (ctf_set_open_errno (errp, errno));

  if ((nbytes = ctf_pread (fd, &ctfhdr, sizeof (ctfhdr), 0)) <= 0)
    return (ctf_set_open_errno (errp, nbytes < 0 ? errno : ECTF_FMT));

  /* If we have read enough bytes to form a CTF header and the magic string
     matches, in either endianness, attempt to interpret the file as raw
     CTF.  */

  if ((size_t) nbytes >= sizeof (ctf_preamble_t)
      && (ctfhdr.ctp_magic == CTF_MAGIC
	  || ctfhdr.ctp_magic == bswap_16 (CTF_MAGIC)))
    {
      ctf_dict_t *fp = NULL;
      void *data;

      if ((data = ctf_mmap (st.st_size, 0, fd)) == NULL)
	return (ctf_set_open_errno (errp, errno));

      if ((fp = ctf_simple_open (data, (size_t) st.st_size, NULL, 0, 0,
				 NULL, 0, errp)) == NULL)
	{
	  ctf_munmap (data, (size_t) st.st_size);
	  return NULL;			/* errno is set for us.  */
	}

      fp->ctf_data_mmapped = data;
      fp->ctf_data_mmapped_len = (size_t) st.st_size;

      return ctf_new_archive_internal (0, 1, NULL, fp, NULL, NULL, errp);
    }

  if ((nbytes = ctf_pread (fd, &arc_magic, sizeof (arc_magic), 0)) <= 0)
    return (ctf_set_open_errno (errp, nbytes < 0 ? errno : ECTF_FMT));

  if ((size_t) nbytes >= sizeof (uint64_t) && le64toh (arc_magic) == CTFA_MAGIC)
    {
      struct ctf_archive *arc;

      if ((arc = ctf_arc_open_internal (filename, errp)) == NULL)
	return NULL;			/* errno is set for us.  */

      return ctf_new_archive_internal (1, 1, arc, NULL, NULL, NULL, errp);
    }

  /* Attempt to open the file with BFD.  We must dup the fd first, since bfd
     takes ownership of the passed fd.  */

  if ((nfd = dup (fd)) < 0)
      return (ctf_set_open_errno (errp, errno));

  if ((abfd = bfd_fdopenr (filename, target, nfd)) == NULL)
    {
      ctf_err_warn (NULL, 0, 0, _("cannot open BFD from %s: %s"),
		    filename ? filename : _("(unknown file)"),
		    bfd_errmsg (bfd_get_error ()));
      return (ctf_set_open_errno (errp, ECTF_FMT));
    }
  bfd_set_cacheable (abfd, 1);

  if (!bfd_check_format (abfd, bfd_object))
    {
      ctf_err_warn (NULL, 0, 0, _("BFD format problem in %s: %s"),
		    filename ? filename : _("(unknown file)"),
		    bfd_errmsg (bfd_get_error ()));
      if (bfd_get_error() == bfd_error_file_ambiguously_recognized)
	return (ctf_set_open_errno (errp, ECTF_BFD_AMBIGUOUS));
      else
	return (ctf_set_open_errno (errp, ECTF_FMT));
    }

  if ((arci = ctf_bfdopen (abfd, errp)) == NULL)
    {
      if (!bfd_close_all_done (abfd))
	ctf_err_warn (NULL, 0, 0, _("cannot close BFD: %s"),
		      bfd_errmsg (bfd_get_error ()));
      return NULL;			/* errno is set for us.  */
    }
  arci->ctfi_bfd_close = ctf_bfdclose;
  arci->ctfi_abfd = abfd;

  return arci;
}

/* Open the specified file and return a pointer to a CTF dict.  The file
   can be either an ELF file or raw CTF file.  This is just a convenient
   wrapper around ctf_fdopen() for callers.  */

ctf_archive_t *
ctf_open (const char *filename, const char *target, int *errp)
{
  ctf_archive_t *arc;
  int fd;

  if ((fd = open (filename, O_RDONLY)) == -1)
    {
      if (errp != NULL)
	*errp = errno;
      return NULL;
    }

  arc = ctf_fdopen (fd, filename, target, errp);
  (void) close (fd);
  return arc;
}

/* Public entry point: open a CTF archive, or CTF file.  Returns the archive, or
   NULL and an error in *err.  Despite the fact that this uses CTF archives, it
   must be in this file to avoid dragging in BFD into non-BFD-using programs.  */
ctf_archive_t *
ctf_arc_open (const char *filename, int *errp)
{
  return ctf_open (filename, NULL, errp);
}
