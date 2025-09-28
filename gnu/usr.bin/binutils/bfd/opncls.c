/* opncls.c -- open and close a BFD.
   Copyright (C) 1990-2023 Free Software Foundation, Inc.

   Written by Cygnus Support.

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
#include "objalloc.h"
#include "libbfd.h"
#include "libiberty.h"
#include "elf-bfd.h"

#ifndef S_IXUSR
#define S_IXUSR 0100	/* Execute by owner.  */
#endif
#ifndef S_IXGRP
#define S_IXGRP 0010	/* Execute by group.  */
#endif
#ifndef S_IXOTH
#define S_IXOTH 0001	/* Execute by others.  */
#endif

/*
SECTION
	Opening and closing BFDs

SUBSECTION
	Functions for opening and closing
*/

/* Counters used to initialize the bfd identifier.  */

static unsigned int bfd_id_counter = 0;
static unsigned int bfd_reserved_id_counter = 0;

/*
EXTERNAL
.{* Set to N to open the next N BFDs using an alternate id space.  *}
.extern unsigned int bfd_use_reserved_id;
.
*/
unsigned int bfd_use_reserved_id = 0;

/* fdopen is a loser -- we should use stdio exclusively.  Unfortunately
   if we do that we can't use fcntl.  */

/*
INTERNAL_FUNCTION
	_bfd_new_bfd

SYNOPSIS
	bfd *_bfd_new_bfd (void);

DESCRIPTION
	Return a new BFD.  All BFD's are allocated through this routine.
*/

bfd *
_bfd_new_bfd (void)
{
  bfd *nbfd;

  nbfd = (bfd *) bfd_zmalloc (sizeof (bfd));
  if (nbfd == NULL)
    return NULL;

  if (bfd_use_reserved_id)
    {
      nbfd->id = --bfd_reserved_id_counter;
      --bfd_use_reserved_id;
    }
  else
    nbfd->id = bfd_id_counter++;

  nbfd->memory = objalloc_create ();
  if (nbfd->memory == NULL)
    {
      bfd_set_error (bfd_error_no_memory);
      free (nbfd);
      return NULL;
    }

  nbfd->arch_info = &bfd_default_arch_struct;

  if (!bfd_hash_table_init_n (& nbfd->section_htab, bfd_section_hash_newfunc,
			      sizeof (struct section_hash_entry), 13))
    {
      objalloc_free ((struct objalloc *) nbfd->memory);
      free (nbfd);
      return NULL;
    }

  nbfd->archive_plugin_fd = -1;

  return nbfd;
}

static const struct bfd_iovec opncls_iovec;

/*
INTERNAL_FUNCTION
	_bfd_new_bfd_contained_in

SYNOPSIS
	bfd *_bfd_new_bfd_contained_in (bfd *);

DESCRIPTION
	Allocate a new BFD as a member of archive OBFD.
*/

bfd *
_bfd_new_bfd_contained_in (bfd *obfd)
{
  bfd *nbfd;

  /* Nested archives in bims are unsupported.  */
  if ((obfd->flags & BFD_IN_MEMORY) != 0)
    {
      bfd_set_error (bfd_error_malformed_archive);
      return NULL;
    }
  nbfd = _bfd_new_bfd ();
  if (nbfd == NULL)
    return NULL;
  nbfd->xvec = obfd->xvec;
  nbfd->iovec = obfd->iovec;
  if (obfd->iovec == &opncls_iovec)
    nbfd->iostream = obfd->iostream;
  nbfd->my_archive = obfd;
  nbfd->direction = read_direction;
  nbfd->target_defaulted = obfd->target_defaulted;
  nbfd->lto_output = obfd->lto_output;
  nbfd->no_export = obfd->no_export;
  return nbfd;
}

/* Delete a BFD.  */

static void
_bfd_delete_bfd (bfd *abfd)
{
  /* Give the target _bfd_free_cached_info a chance to free memory.  */
  if (abfd->memory && abfd->xvec)
    bfd_free_cached_info (abfd);

  /* The target _bfd_free_cached_info may not have done anything..  */
  if (abfd->memory)
    {
      bfd_hash_table_free (&abfd->section_htab);
      objalloc_free ((struct objalloc *) abfd->memory);
    }
  else
    free ((char *) bfd_get_filename (abfd));

  free (abfd->arelt_data);
  free (abfd);
}

/*
INTERNAL_FUNCTION
	_bfd_free_cached_info

SYNOPSIS
	bool _bfd_free_cached_info (bfd *);

DESCRIPTION
	Free objalloc memory.
*/

bool
_bfd_free_cached_info (bfd *abfd)
{
  if (abfd->memory)
    {
      const char *filename = bfd_get_filename (abfd);
      if (filename)
	{
	  /* We can't afford to lose the bfd filename when freeing
	     abfd->memory, because that would kill the cache.c scheme
	     of closing and reopening files in order to limit the
	     number of open files.  To reopen, you need the filename.
	     And indeed _bfd_compute_and_write_armap calls
	     _bfd_free_cached_info to free up space used by symbols
	     and by check_format_matches.  Which we want to continue
	     doing to handle very large archives.  Later the archive
	     elements are copied, which might require reopening files.
	     We also want to keep using objalloc memory for the
	     filename since that allows the name to be updated
	     without either leaking memory or implementing some sort
	     of reference counted string for copies of the filename.  */
	  size_t len = strlen (filename) + 1;
	  char *copy = bfd_malloc (len);
	  if (copy == NULL)
	    return false;
	  memcpy (copy, filename, len);
	  abfd->filename = copy;
	}
      bfd_hash_table_free (&abfd->section_htab);
      objalloc_free ((struct objalloc *) abfd->memory);

      abfd->sections = NULL;
      abfd->section_last = NULL;
      abfd->outsymbols = NULL;
      abfd->tdata.any = NULL;
      abfd->usrdata = NULL;
      abfd->memory = NULL;
    }

  return true;
}

/*
FUNCTION
	bfd_fopen

SYNOPSIS
	bfd *bfd_fopen (const char *filename, const char *target,
			const char *mode, int fd);

DESCRIPTION
	Open the file @var{filename} with the target @var{target}.
	Return a pointer to the created BFD.  If @var{fd} is not -1,
	then <<fdopen>> is used to open the file; otherwise, <<fopen>>
	is used.  @var{mode} is passed directly to <<fopen>> or
	<<fdopen>>.

	Calls <<bfd_find_target>>, so @var{target} is interpreted as by
	that function.

	The new BFD is marked as cacheable iff @var{fd} is -1.

	If <<NULL>> is returned then an error has occured.   Possible errors
	are <<bfd_error_no_memory>>, <<bfd_error_invalid_target>> or
	<<system_call>> error.

	On error, @var{fd} is always closed.

	A copy of the @var{filename} argument is stored in the newly created
	BFD.  It can be accessed via the bfd_get_filename() macro.
*/

bfd *
bfd_fopen (const char *filename, const char *target, const char *mode, int fd)
{
  bfd *nbfd;
  const bfd_target *target_vec;

  nbfd = _bfd_new_bfd ();
  if (nbfd == NULL)
    {
      if (fd != -1)
	close (fd);
      return NULL;
    }

  target_vec = bfd_find_target (target, nbfd);
  if (target_vec == NULL)
    {
      if (fd != -1)
	close (fd);
      _bfd_delete_bfd (nbfd);
      return NULL;
    }

#ifdef HAVE_FDOPEN
  if (fd != -1)
    nbfd->iostream = fdopen (fd, mode);
  else
#endif
    nbfd->iostream = _bfd_real_fopen (filename, mode);
  if (nbfd->iostream == NULL)
    {
      bfd_set_error (bfd_error_system_call);
      if (fd != -1)
	close (fd);
      _bfd_delete_bfd (nbfd);
      return NULL;
    }

  /* OK, put everything where it belongs.  */

  /* PR 11983: Do not cache the original filename, but
     rather make a copy - the original might go away.  */
  if (!bfd_set_filename (nbfd, filename))
    {
      fclose (nbfd->iostream);
      _bfd_delete_bfd (nbfd);
      return NULL;
    }

  /* Figure out whether the user is opening the file for reading,
     writing, or both, by looking at the MODE argument.  */
  if ((mode[0] == 'r' || mode[0] == 'w' || mode[0] == 'a')
      && mode[1] == '+')
    nbfd->direction = both_direction;
  else if (mode[0] == 'r')
    nbfd->direction = read_direction;
  else
    nbfd->direction = write_direction;

  if (!bfd_cache_init (nbfd))
    {
      fclose (nbfd->iostream);
      _bfd_delete_bfd (nbfd);
      return NULL;
    }
  nbfd->opened_once = true;

  /* If we opened the file by name, mark it cacheable; we can close it
     and reopen it later.  However, if a file descriptor was provided,
     then it may have been opened with special flags that make it
     unsafe to close and reopen the file.  */
  if (fd == -1)
    (void) bfd_set_cacheable (nbfd, true);

  return nbfd;
}

/*
FUNCTION
	bfd_openr

SYNOPSIS
	bfd *bfd_openr (const char *filename, const char *target);

DESCRIPTION
	Open the file @var{filename} (using <<fopen>>) with the target
	@var{target}.  Return a pointer to the created BFD.

	Calls <<bfd_find_target>>, so @var{target} is interpreted as by
	that function.

	If <<NULL>> is returned then an error has occured.   Possible errors
	are <<bfd_error_no_memory>>, <<bfd_error_invalid_target>> or
	<<system_call>> error.

	A copy of the @var{filename} argument is stored in the newly created
	BFD.  It can be accessed via the bfd_get_filename() macro.
*/

bfd *
bfd_openr (const char *filename, const char *target)
{
  return bfd_fopen (filename, target, FOPEN_RB, -1);
}

/* Don't try to `optimize' this function:

   o - We lock using stack space so that interrupting the locking
       won't cause a storage leak.
   o - We open the file stream last, since we don't want to have to
       close it if anything goes wrong.  Closing the stream means closing
       the file descriptor too, even though we didn't open it.  */
/*
FUNCTION
	bfd_fdopenr

SYNOPSIS
	bfd *bfd_fdopenr (const char *filename, const char *target, int fd);

DESCRIPTION
	<<bfd_fdopenr>> is to <<bfd_fopenr>> much like <<fdopen>> is to
	<<fopen>>.  It opens a BFD on a file already described by the
	@var{fd} supplied.

	When the file is later <<bfd_close>>d, the file descriptor will
	be closed.  If the caller desires that this file descriptor be
	cached by BFD (opened as needed, closed as needed to free
	descriptors for other opens), with the supplied @var{fd} used as
	an initial file descriptor (but subject to closure at any time),
	call bfd_set_cacheable(bfd, 1) on the returned BFD.  The default
	is to assume no caching; the file descriptor will remain open
	until <<bfd_close>>, and will not be affected by BFD operations
	on other files.

	Possible errors are <<bfd_error_no_memory>>,
	<<bfd_error_invalid_target>> and <<bfd_error_system_call>>.

	On error, @var{fd} is closed.

	A copy of the @var{filename} argument is stored in the newly created
	BFD.  It can be accessed via the bfd_get_filename() macro.
*/

bfd *
bfd_fdopenr (const char *filename, const char *target, int fd)
{
  const char *mode;
#if defined(HAVE_FCNTL) && defined(F_GETFL)
  int fdflags;
#endif

#if ! defined(HAVE_FCNTL) || ! defined(F_GETFL)
  mode = FOPEN_RUB; /* Assume full access.  */
#else
  fdflags = fcntl (fd, F_GETFL, NULL);
  if (fdflags == -1)
    {
      int save = errno;

      close (fd);
      errno = save;
      bfd_set_error (bfd_error_system_call);
      return NULL;
    }

  /* (O_ACCMODE) parens are to avoid Ultrix header file bug.  */
  switch (fdflags & (O_ACCMODE))
    {
    case O_RDONLY: mode = FOPEN_RB; break;
    case O_WRONLY: mode = FOPEN_RUB; break;
    case O_RDWR:   mode = FOPEN_RUB; break;
    default: abort ();
    }
#endif

  return bfd_fopen (filename, target, mode, fd);
}

/*
FUNCTION
	bfd_fdopenw

SYNOPSIS
	bfd *bfd_fdopenw (const char *filename, const char *target, int fd);

DESCRIPTION
	<<bfd_fdopenw>> is exactly like <<bfd_fdopenr>> with the exception that
	the resulting BFD is suitable for output.
*/

bfd *
bfd_fdopenw (const char *filename, const char *target, int fd)
{
  bfd *out = bfd_fdopenr (filename, target, fd);

  if (out != NULL)
    {
      if (!bfd_write_p (out))
	{
	  close (fd);
	  _bfd_delete_bfd (out);
	  out = NULL;
	  bfd_set_error (bfd_error_invalid_operation);
	}
      else
	out->direction = write_direction;
    }

  return out;
}

/*
FUNCTION
	bfd_openstreamr

SYNOPSIS
	bfd *bfd_openstreamr (const char * filename, const char * target,
			      void * stream);

DESCRIPTION
	Open a BFD for read access on an existing stdio stream.  When
	the BFD is passed to <<bfd_close>>, the stream will be closed.

	A copy of the @var{filename} argument is stored in the newly created
	BFD.  It can be accessed via the bfd_get_filename() macro.
*/

bfd *
bfd_openstreamr (const char *filename, const char *target, void *streamarg)
{
  FILE *stream = (FILE *) streamarg;
  bfd *nbfd;
  const bfd_target *target_vec;

  nbfd = _bfd_new_bfd ();
  if (nbfd == NULL)
    return NULL;

  target_vec = bfd_find_target (target, nbfd);
  if (target_vec == NULL)
    {
      _bfd_delete_bfd (nbfd);
      return NULL;
    }

  nbfd->iostream = stream;
  /* PR 11983: Do not cache the original filename, but
     rather make a copy - the original might go away.  */
  if (!bfd_set_filename (nbfd, filename))
    {
      _bfd_delete_bfd (nbfd);
      return NULL;
    }
  nbfd->direction = read_direction;

  if (! bfd_cache_init (nbfd))
    {
      _bfd_delete_bfd (nbfd);
      return NULL;
    }

  return nbfd;
}

/*
FUNCTION
	bfd_openr_iovec

SYNOPSIS
	bfd *bfd_openr_iovec (const char *filename, const char *target,
			      void *(*open_func) (struct bfd *nbfd,
						  void *open_closure),
			      void *open_closure,
			      file_ptr (*pread_func) (struct bfd *nbfd,
						      void *stream,
						      void *buf,
						      file_ptr nbytes,
						      file_ptr offset),
			      int (*close_func) (struct bfd *nbfd,
						 void *stream),
			      int (*stat_func) (struct bfd *abfd,
						void *stream,
						struct stat *sb));

DESCRIPTION
	Create and return a BFD backed by a read-only @var{stream}.
	The @var{stream} is created using @var{open_func}, accessed using
	@var{pread_func} and destroyed using @var{close_func}.

	Calls <<bfd_find_target>>, so @var{target} is interpreted as by
	that function.

	Calls @var{open_func} (which can call <<bfd_zalloc>> and
	<<bfd_get_filename>>) to obtain the read-only stream backing
	the BFD.  @var{open_func} either succeeds returning the
	non-<<NULL>> @var{stream}, or fails returning <<NULL>>
	(setting <<bfd_error>>).

	Calls @var{pread_func} to request @var{nbytes} of data from
	@var{stream} starting at @var{offset} (e.g., via a call to
	<<bfd_read>>).  @var{pread_func} either succeeds returning the
	number of bytes read (which can be less than @var{nbytes} when
	end-of-file), or fails returning -1 (setting <<bfd_error>>).

	Calls @var{close_func} when the BFD is later closed using
	<<bfd_close>>.  @var{close_func} either succeeds returning 0, or
	fails returning -1 (setting <<bfd_error>>).

	Calls @var{stat_func} to fill in a stat structure for bfd_stat,
	bfd_get_size, and bfd_get_mtime calls.  @var{stat_func} returns 0
	on success, or returns -1 on failure (setting <<bfd_error>>).

	If <<bfd_openr_iovec>> returns <<NULL>> then an error has
	occurred.  Possible errors are <<bfd_error_no_memory>>,
	<<bfd_error_invalid_target>> and <<bfd_error_system_call>>.

	A copy of the @var{filename} argument is stored in the newly created
	BFD.  It can be accessed via the bfd_get_filename() macro.
*/

struct opncls
{
  void *stream;
  file_ptr (*pread) (struct bfd *abfd, void *stream, void *buf,
		     file_ptr nbytes, file_ptr offset);
  int (*close) (struct bfd *abfd, void *stream);
  int (*stat) (struct bfd *abfd, void *stream, struct stat *sb);
  file_ptr where;
};

static file_ptr
opncls_btell (struct bfd *abfd)
{
  struct opncls *vec = (struct opncls *) abfd->iostream;
  return vec->where;
}

static int
opncls_bseek (struct bfd *abfd, file_ptr offset, int whence)
{
  struct opncls *vec = (struct opncls *) abfd->iostream;
  switch (whence)
    {
    case SEEK_SET: vec->where = offset; break;
    case SEEK_CUR: vec->where += offset; break;
    case SEEK_END: return -1;
    }
  return 0;
}

static file_ptr
opncls_bread (struct bfd *abfd, void *buf, file_ptr nbytes)
{
  struct opncls *vec = (struct opncls *) abfd->iostream;
  file_ptr nread = (vec->pread) (abfd, vec->stream, buf, nbytes, vec->where);

  if (nread < 0)
    return nread;
  vec->where += nread;
  return nread;
}

static file_ptr
opncls_bwrite (struct bfd *abfd ATTRIBUTE_UNUSED,
	      const void *where ATTRIBUTE_UNUSED,
	      file_ptr nbytes ATTRIBUTE_UNUSED)
{
  return -1;
}

static int
opncls_bclose (struct bfd *abfd)
{
  struct opncls *vec = (struct opncls *) abfd->iostream;
  /* Since the VEC's memory is bound to the bfd deleting the bfd will
     free it.  */
  int status = 0;

  if (vec->close != NULL)
    status = (vec->close) (abfd, vec->stream);
  abfd->iostream = NULL;
  return status;
}

static int
opncls_bflush (struct bfd *abfd ATTRIBUTE_UNUSED)
{
  return 0;
}

static int
opncls_bstat (struct bfd *abfd, struct stat *sb)
{
  struct opncls *vec = (struct opncls *) abfd->iostream;

  memset (sb, 0, sizeof (*sb));
  if (vec->stat == NULL)
    return 0;

  return (vec->stat) (abfd, vec->stream, sb);
}

static void *
opncls_bmmap (struct bfd *abfd ATTRIBUTE_UNUSED,
	      void *addr ATTRIBUTE_UNUSED,
	      bfd_size_type len ATTRIBUTE_UNUSED,
	      int prot ATTRIBUTE_UNUSED,
	      int flags ATTRIBUTE_UNUSED,
	      file_ptr offset ATTRIBUTE_UNUSED,
	      void **map_addr ATTRIBUTE_UNUSED,
	      bfd_size_type *map_len ATTRIBUTE_UNUSED)
{
  return (void *) -1;
}

static const struct bfd_iovec opncls_iovec =
{
  &opncls_bread, &opncls_bwrite, &opncls_btell, &opncls_bseek,
  &opncls_bclose, &opncls_bflush, &opncls_bstat, &opncls_bmmap
};

bfd *
bfd_openr_iovec (const char *filename, const char *target,
		 void *(*open_p) (struct bfd *, void *),
		 void *open_closure,
		 file_ptr (*pread_p) (struct bfd *, void *, void *,
				      file_ptr, file_ptr),
		 int (*close_p) (struct bfd *, void *),
		 int (*stat_p) (struct bfd *, void *, struct stat *))
{
  bfd *nbfd;
  const bfd_target *target_vec;
  struct opncls *vec;
  void *stream;

  nbfd = _bfd_new_bfd ();
  if (nbfd == NULL)
    return NULL;

  target_vec = bfd_find_target (target, nbfd);
  if (target_vec == NULL)
    {
      _bfd_delete_bfd (nbfd);
      return NULL;
    }

  /* PR 11983: Do not cache the original filename, but
     rather make a copy - the original might go away.  */
  if (!bfd_set_filename (nbfd, filename))
    {
      _bfd_delete_bfd (nbfd);
      return NULL;
    }
  nbfd->direction = read_direction;

  /* `open_p (...)' would get expanded by an the open(2) syscall macro.  */
  stream = (*open_p) (nbfd, open_closure);
  if (stream == NULL)
    {
      _bfd_delete_bfd (nbfd);
      return NULL;
    }

  vec = (struct opncls *) bfd_zalloc (nbfd, sizeof (struct opncls));
  vec->stream = stream;
  vec->pread = pread_p;
  vec->close = close_p;
  vec->stat = stat_p;

  nbfd->iovec = &opncls_iovec;
  nbfd->iostream = vec;

  return nbfd;
}

/* bfd_openw -- open for writing.
   Returns a pointer to a freshly-allocated BFD on success, or NULL.

   See comment by bfd_fdopenr before you try to modify this function.  */

/*
FUNCTION
	bfd_openw

SYNOPSIS
	bfd *bfd_openw (const char *filename, const char *target);

DESCRIPTION
	Create a BFD, associated with file @var{filename}, using the
	file format @var{target}, and return a pointer to it.

	Possible errors are <<bfd_error_system_call>>, <<bfd_error_no_memory>>,
	<<bfd_error_invalid_target>>.

	A copy of the @var{filename} argument is stored in the newly created
	BFD.  It can be accessed via the bfd_get_filename() macro.
*/

bfd *
bfd_openw (const char *filename, const char *target)
{
  bfd *nbfd;
  const bfd_target *target_vec;

  /* nbfd has to point to head of malloc'ed block so that bfd_close may
     reclaim it correctly.  */
  nbfd = _bfd_new_bfd ();
  if (nbfd == NULL)
    return NULL;

  target_vec = bfd_find_target (target, nbfd);
  if (target_vec == NULL)
    {
      _bfd_delete_bfd (nbfd);
      return NULL;
    }

  /* PR 11983: Do not cache the original filename, but
     rather make a copy - the original might go away.  */
  if (!bfd_set_filename (nbfd, filename))
    {
      _bfd_delete_bfd (nbfd);
      return NULL;
    }
  nbfd->direction = write_direction;

  if (bfd_open_file (nbfd) == NULL)
    {
      /* File not writeable, etc.  */
      bfd_set_error (bfd_error_system_call);
      _bfd_delete_bfd (nbfd);
      return NULL;
  }

  return nbfd;
}

/*
FUNCTION
	bfd_elf_bfd_from_remote_memory

SYNOPSIS
	bfd *bfd_elf_bfd_from_remote_memory
	  (bfd *templ, bfd_vma ehdr_vma, bfd_size_type size, bfd_vma *loadbasep,
	   int (*target_read_memory)
	     (bfd_vma vma, bfd_byte *myaddr, bfd_size_type len));

DESCRIPTION
	Create a new BFD as if by bfd_openr.  Rather than opening a
	file, reconstruct an ELF file by reading the segments out of
	remote memory based on the ELF file header at EHDR_VMA and the
	ELF program headers it points to.  If non-zero, SIZE is the
	known extent of the object.  If not null, *LOADBASEP is filled
	in with the difference between the VMAs from which the
	segments were read, and the VMAs the file headers (and hence
	BFD's idea of each section's VMA) put them at.

	The function TARGET_READ_MEMORY is called to copy LEN bytes
	from the remote memory at target address VMA into the local
	buffer at MYADDR; it should return zero on success or an
	errno code on failure.  TEMPL must be a BFD for an ELF
	target with the word size and byte order found in the remote
	memory.
*/

bfd *
bfd_elf_bfd_from_remote_memory
  (bfd *templ,
   bfd_vma ehdr_vma,
   bfd_size_type size,
   bfd_vma *loadbasep,
   int (*target_read_memory) (bfd_vma, bfd_byte *, bfd_size_type))
{
  if (bfd_get_flavour (templ) != bfd_target_elf_flavour)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return NULL;
    }
  return (*get_elf_backend_data (templ)->elf_backend_bfd_from_remote_memory)
    (templ, ehdr_vma, size, loadbasep, target_read_memory);
}

static inline void
_maybe_make_executable (bfd * abfd)
{
  /* If the file was open for writing and is now executable,
     make it so.  */
  if (abfd->direction == write_direction
      && (abfd->flags & (EXEC_P | DYNAMIC)) != 0)
    {
      struct stat buf;

      if (stat (bfd_get_filename (abfd), &buf) == 0
	  /* Do not attempt to change non-regular files.  This is
	     here especially for configure scripts and kernel builds
	     which run tests with "ld [...] -o /dev/null".  */
	  && S_ISREG(buf.st_mode))
	{
	  unsigned int mask = umask (0);

	  umask (mask);
	  chmod (bfd_get_filename (abfd),
		 (0777
		  & (buf.st_mode | ((S_IXUSR | S_IXGRP | S_IXOTH) &~ mask))));
	}
    }
}

/*
FUNCTION
	bfd_close

SYNOPSIS
	bool bfd_close (bfd *abfd);

DESCRIPTION
	Close a BFD. If the BFD was open for writing, then pending
	operations are completed and the file written out and closed.
	If the created file is executable, then <<chmod>> is called
	to mark it as such.

	All memory attached to the BFD is released.

	The file descriptor associated with the BFD is closed (even
	if it was passed in to BFD by <<bfd_fdopenr>>).

	<<TRUE>> is returned if all is ok, otherwise <<FALSE>>.
*/

bool
bfd_close (bfd *abfd)
{
  bool ret = (!bfd_write_p (abfd)
	      || BFD_SEND_FMT (abfd, _bfd_write_contents, (abfd)));

  return bfd_close_all_done (abfd) && ret;
}

/*
FUNCTION
	bfd_close_all_done

SYNOPSIS
	bool bfd_close_all_done (bfd *);

DESCRIPTION
	Close a BFD.  Differs from <<bfd_close>> since it does not
	complete any pending operations.  This routine would be used
	if the application had just used BFD for swapping and didn't
	want to use any of the writing code.

	If the created file is executable, then <<chmod>> is called
	to mark it as such.

	All memory attached to the BFD is released.

	<<TRUE>> is returned if all is ok, otherwise <<FALSE>>.
*/

bool
bfd_close_all_done (bfd *abfd)
{
  bool ret = BFD_SEND (abfd, _close_and_cleanup, (abfd));

  if (ret && abfd->iovec != NULL)
    {
      ret = abfd->iovec->bclose (abfd) == 0;

      if (ret)
	_maybe_make_executable (abfd);
    }

  _bfd_delete_bfd (abfd);
  free (_bfd_error_buf);
  _bfd_error_buf = NULL;

  return ret;
}

/*
FUNCTION
	bfd_create

SYNOPSIS
	bfd *bfd_create (const char *filename, bfd *templ);

DESCRIPTION
	Create a new BFD in the manner of <<bfd_openw>>, but without
	opening a file. The new BFD takes the target from the target
	used by @var{templ}. The format is always set to <<bfd_object>>.

	A copy of the @var{filename} argument is stored in the newly created
	BFD.  It can be accessed via the bfd_get_filename() macro.
*/

bfd *
bfd_create (const char *filename, bfd *templ)
{
  bfd *nbfd;

  nbfd = _bfd_new_bfd ();
  if (nbfd == NULL)
    return NULL;
  /* PR 11983: Do not cache the original filename, but
     rather make a copy - the original might go away.  */
  if (!bfd_set_filename (nbfd, filename))
    {
      _bfd_delete_bfd (nbfd);
      return NULL;
    }
  if (templ)
    nbfd->xvec = templ->xvec;
  nbfd->direction = no_direction;
  bfd_set_format (nbfd, bfd_object);

  return nbfd;
}

/*
FUNCTION
	bfd_make_writable

SYNOPSIS
	bool bfd_make_writable (bfd *abfd);

DESCRIPTION
	Takes a BFD as created by <<bfd_create>> and converts it
	into one like as returned by <<bfd_openw>>.  It does this
	by converting the BFD to BFD_IN_MEMORY.  It's assumed that
	you will call <<bfd_make_readable>> on this bfd later.

	<<TRUE>> is returned if all is ok, otherwise <<FALSE>>.
*/

bool
bfd_make_writable (bfd *abfd)
{
  struct bfd_in_memory *bim;

  if (abfd->direction != no_direction)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return false;
    }

  bim = (struct bfd_in_memory *) bfd_malloc (sizeof (struct bfd_in_memory));
  if (bim == NULL)
    return false;	/* bfd_error already set.  */
  abfd->iostream = bim;
  /* bfd_bwrite will grow these as needed.  */
  bim->size = 0;
  bim->buffer = 0;

  abfd->flags |= BFD_IN_MEMORY;
  abfd->iovec = &_bfd_memory_iovec;
  abfd->origin = 0;
  abfd->direction = write_direction;
  abfd->where = 0;

  return true;
}

/*
FUNCTION
	bfd_make_readable

SYNOPSIS
	bool bfd_make_readable (bfd *abfd);

DESCRIPTION
	Takes a BFD as created by <<bfd_create>> and
	<<bfd_make_writable>> and converts it into one like as
	returned by <<bfd_openr>>.  It does this by writing the
	contents out to the memory buffer, then reversing the
	direction.

	<<TRUE>> is returned if all is ok, otherwise <<FALSE>>.  */

bool
bfd_make_readable (bfd *abfd)
{
  if (abfd->direction != write_direction || !(abfd->flags & BFD_IN_MEMORY))
    {
      bfd_set_error (bfd_error_invalid_operation);
      return false;
    }

  if (! BFD_SEND_FMT (abfd, _bfd_write_contents, (abfd)))
    return false;

  if (! BFD_SEND (abfd, _close_and_cleanup, (abfd)))
    return false;

  abfd->arch_info = &bfd_default_arch_struct;

  abfd->where = 0;
  abfd->format = bfd_unknown;
  abfd->my_archive = NULL;
  abfd->origin = 0;
  abfd->opened_once = false;
  abfd->output_has_begun = false;
  abfd->section_count = 0;
  abfd->usrdata = NULL;
  abfd->cacheable = false;
  abfd->flags |= BFD_IN_MEMORY;
  abfd->mtime_set = false;

  abfd->target_defaulted = true;
  abfd->direction = read_direction;
  abfd->sections = 0;
  abfd->symcount = 0;
  abfd->outsymbols = 0;
  abfd->tdata.any = 0;
  abfd->size = 0;

  bfd_section_list_clear (abfd);
  bfd_check_format (abfd, bfd_object);

  return true;
}

/*
   GNU Extension: separate debug-info files

   The idea here is that a special section called .gnu_debuglink might be
   embedded in a binary file, which indicates that some *other* file
   contains the real debugging information. This special section contains a
   filename and CRC32 checksum, which we read and resolve to another file,
   if it exists.

   This facilitates "optional" provision of debugging information, without
   having to provide two complete copies of every binary object (with and
   without debug symbols).  */

#define GNU_DEBUGLINK		".gnu_debuglink"
#define GNU_DEBUGALTLINK	".gnu_debugaltlink"

/*
FUNCTION
	bfd_calc_gnu_debuglink_crc32

SYNOPSIS
	uint32_t bfd_calc_gnu_debuglink_crc32
	  (uint32_t crc, const bfd_byte *buf, bfd_size_type len);

DESCRIPTION
	Computes a CRC value as used in the .gnu_debuglink section.
	Advances the previously computed @var{crc} value by computing
	and adding in the crc32 for @var{len} bytes of @var{buf}.

	Return the updated CRC32 value.
*/

uint32_t
bfd_calc_gnu_debuglink_crc32 (uint32_t crc,
			      const bfd_byte *buf,
			      bfd_size_type len)
{
  static const uint32_t crc32_table[256] =
    {
      0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419,
      0x706af48f, 0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4,
      0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07,
      0x90bf1d91, 0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
      0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7, 0x136c9856,
      0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
      0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4,
      0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
      0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3,
      0x45df5c75, 0xdcd60dcf, 0xabd13d59, 0x26d930ac, 0x51de003a,
      0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599,
      0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
      0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190,
      0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f,
      0x9fbfe4a5, 0xe8b8d433, 0x7807c9a2, 0x0f00f934, 0x9609a88e,
      0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
      0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed,
      0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
      0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3,
      0xfbd44c65, 0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
      0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a,
      0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5,
      0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa, 0xbe0b1010,
      0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
      0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17,
      0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6,
      0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615,
      0x73dc1683, 0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
      0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1, 0xf00f9344,
      0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
      0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a,
      0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
      0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1,
      0xa6bc5767, 0x3fb506dd, 0x48b2364b, 0xd80d2bda, 0xaf0a1b4c,
      0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef,
      0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
      0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe,
      0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31,
      0x2cd99e8b, 0x5bdeae1d, 0x9b64c2b0, 0xec63f226, 0x756aa39c,
      0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
      0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b,
      0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
      0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1,
      0x18b74777, 0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
      0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45, 0xa00ae278,
      0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7,
      0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc, 0x40df0b66,
      0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
      0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605,
      0xcdd70693, 0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8,
      0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b,
      0x2d02ef8d
    };
  const bfd_byte *end;

  crc = ~crc & 0xffffffff;
  for (end = buf + len; buf < end; ++ buf)
    crc = crc32_table[(crc ^ *buf) & 0xff] ^ (crc >> 8);
  return ~crc & 0xffffffff;
}


/* Extracts the filename and CRC32 value for any separate debug
   information file associated with @var{abfd}.

   The @var{crc32_out} parameter is an untyped pointer because
   this routine is used as a @code{get_func_type} function, but it
   is expected to be a uint32_t pointer.

   Returns the filename of the associated debug information file,
   or NULL if there is no such file.  If the filename was found
   then the contents of @var{crc32_out} are updated to hold the
   corresponding CRC32 value for the file.

   The returned filename is allocated with @code{malloc}; freeing
   it is the responsibility of the caller.  */

static char *
bfd_get_debug_link_info_1 (bfd *abfd, void *crc32_out)
{
  asection *sect;
  uint32_t *crc32 = crc32_out;
  bfd_byte *contents;
  unsigned int crc_offset;
  char *name;
  bfd_size_type size;

  BFD_ASSERT (abfd);
  BFD_ASSERT (crc32_out);

  sect = bfd_get_section_by_name (abfd, GNU_DEBUGLINK);

  if (sect == NULL || (sect->flags & SEC_HAS_CONTENTS) == 0)
    return NULL;

  size = bfd_section_size (sect);

  /* PR 22794: Make sure that the section has a reasonable size.  */
  if (size < 8)
    return NULL;

  if (!bfd_malloc_and_get_section (abfd, sect, &contents))
    return NULL;

  /* CRC value is stored after the filename, aligned up to 4 bytes.  */
  name = (char *) contents;
  /* PR 17597: Avoid reading off the end of the buffer.  */
  crc_offset = strnlen (name, size) + 1;
  crc_offset = (crc_offset + 3) & ~3;
  if (crc_offset + 4 > size)
    {
      free (name);
      return NULL;
    }

  *crc32 = bfd_get_32 (abfd, contents + crc_offset);
  return name;
}


/*
FUNCTION
	bfd_get_debug_link_info

SYNOPSIS
	char *bfd_get_debug_link_info (bfd *abfd, uint32_t *crc32_out);

DESCRIPTION
	Extracts the filename and CRC32 value for any separate debug
	information file associated with @var{abfd}.

	Returns the filename of the associated debug information file,
	or NULL if there is no such file.  If the filename was found
	then the contents of @var{crc32_out} are updated to hold the
	corresponding CRC32 value for the file.

	The returned filename is allocated with @code{malloc}; freeing
	it is the responsibility of the caller.
*/

char *
bfd_get_debug_link_info (bfd *abfd, uint32_t *crc32_out)
{
  return bfd_get_debug_link_info_1 (abfd, crc32_out);
}

/*
FUNCTION
	bfd_get_alt_debug_link_info

SYNOPSIS
	char *bfd_get_alt_debug_link_info (bfd * abfd,
					   bfd_size_type *buildid_len,
					   bfd_byte **buildid_out);

DESCRIPTION
	Fetch the filename and BuildID value for any alternate debuginfo
	associated with @var{abfd}.  Return NULL if no such info found,
	otherwise return filename and update @var{buildid_len} and
	@var{buildid_out}.  The returned filename and build_id are
	allocated with @code{malloc}; freeing them is the responsibility
	of the caller.
*/

char *
bfd_get_alt_debug_link_info (bfd * abfd, bfd_size_type *buildid_len,
			     bfd_byte **buildid_out)
{
  asection *sect;
  bfd_byte *contents;
  unsigned int buildid_offset;
  char *name;
  bfd_size_type size;

  BFD_ASSERT (abfd);
  BFD_ASSERT (buildid_len);
  BFD_ASSERT (buildid_out);

  sect = bfd_get_section_by_name (abfd, GNU_DEBUGALTLINK);

  if (sect == NULL || (sect->flags & SEC_HAS_CONTENTS) == 0)
    return NULL;

  size = bfd_section_size (sect);
  if (size < 8)
    return NULL;

  if (!bfd_malloc_and_get_section (abfd, sect, & contents))
    return NULL;

  /* BuildID value is stored after the filename.  */
  name = (char *) contents;
  buildid_offset = strnlen (name, size) + 1;
  if (buildid_offset >= bfd_section_size (sect))
    return NULL;

  *buildid_len = size - buildid_offset;
  *buildid_out = bfd_malloc (*buildid_len);
  memcpy (*buildid_out, contents + buildid_offset, *buildid_len);

  return name;
}

/* Checks to see if @var{name} is a file and if its contents match
   @var{crc32}, which is a pointer to a @code{uint32_t}
   containing a CRC32.

   The @var{crc32_p} parameter is an untyped pointer because this
   routine is used as a @code{check_func_type} function.  */

static bool
separate_debug_file_exists (const char *name, void *crc32_p)
{
  unsigned char buffer[8 * 1024];
  uint32_t file_crc = 0;
  FILE *f;
  bfd_size_type count;
  uint32_t crc;

  BFD_ASSERT (name);
  BFD_ASSERT (crc32_p);

  crc = *(uint32_t *) crc32_p;

  f = _bfd_real_fopen (name, FOPEN_RB);
  if (f == NULL)
    return false;

  while ((count = fread (buffer, 1, sizeof (buffer), f)) > 0)
    file_crc = bfd_calc_gnu_debuglink_crc32 (file_crc, buffer, count);

  fclose (f);

  return crc == file_crc;
}

/* Checks to see if @var{name} is a file.  */

static bool
separate_alt_debug_file_exists (const char *name, void *unused ATTRIBUTE_UNUSED)
{
  FILE *f;

  BFD_ASSERT (name);

  f = _bfd_real_fopen (name, FOPEN_RB);
  if (f == NULL)
    return false;

  fclose (f);

  return true;
}

/* Searches for a debug information file corresponding to @var{abfd}.

   The name of the separate debug info file is returned by the
   @var{get} function.  This function scans various fixed locations
   in the filesystem, including the file tree rooted at @var{dir}.
   If the @var{include_dirs} parameter is true then the directory
   components of @var{abfd}'s filename will be included in the
   searched locations.

   @var{data} is passed unmodified to the @var{get} and @var{check}
   functions.  It is generally used to implement build-id-like
   matching in the callback functions.

   Returns the filename of the first file to be found which
   receives a TRUE result from the @var{check} function.
   Returns NULL if no valid file could be found.  */

typedef char * (*get_func_type) (bfd *, void *);
typedef bool (*check_func_type) (const char *, void *);

static char *
find_separate_debug_file (bfd *abfd,
			  const char *debug_file_directory,
			  bool include_dirs,
			  get_func_type get_func,
			  check_func_type check_func,
			  void *func_data)
{
  char *base;
  char *dir;
  char *debugfile;
  char *canon_dir;
  size_t dirlen;
  size_t canon_dirlen;

  BFD_ASSERT (abfd);
  if (debug_file_directory == NULL)
    debug_file_directory = ".";

  /* BFD may have been opened from a stream.  */
  if (bfd_get_filename (abfd) == NULL)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return NULL;
    }

  base = get_func (abfd, func_data);

  if (base == NULL)
    return NULL;

  if (base[0] == '\0')
    {
      free (base);
      bfd_set_error (bfd_error_no_debug_section);
      return NULL;
    }

  if (include_dirs)
    {
      const char *fname = bfd_get_filename (abfd);
      for (dirlen = strlen (fname); dirlen > 0; dirlen--)
	if (IS_DIR_SEPARATOR (fname[dirlen - 1]))
	  break;

      dir = (char *) bfd_malloc (dirlen + 1);
      if (dir == NULL)
	{
	  free (base);
	  return NULL;
	}
      memcpy (dir, fname, dirlen);
      dir[dirlen] = '\0';
    }
  else
    {
      dir = (char *) bfd_malloc (1);
      * dir = 0;
      dirlen = 0;
    }

  /* Compute the canonical name of the bfd object with all symbolic links
     resolved, for use in the global debugfile directory.  */
  canon_dir = lrealpath (bfd_get_filename (abfd));
  for (canon_dirlen = strlen (canon_dir); canon_dirlen > 0; canon_dirlen--)
    if (IS_DIR_SEPARATOR (canon_dir[canon_dirlen - 1]))
      break;
  canon_dir[canon_dirlen] = '\0';

#ifndef EXTRA_DEBUG_ROOT1
#define EXTRA_DEBUG_ROOT1 "/usr/lib/debug"
#endif
#ifndef EXTRA_DEBUG_ROOT2
#define EXTRA_DEBUG_ROOT2 "/usr/lib/debug/usr"
#endif

  debugfile = (char *)
      bfd_malloc (strlen (debug_file_directory) + 1
		  + (canon_dirlen > dirlen ? canon_dirlen : dirlen)
		  + strlen (".debug/")
#ifdef EXTRA_DEBUG_ROOT1
		  + strlen (EXTRA_DEBUG_ROOT1)
#endif
#ifdef EXTRA_DEBUG_ROOT2
		  + strlen (EXTRA_DEBUG_ROOT2)
#endif
		  + strlen (base)
		  + 1);
  if (debugfile == NULL)
    goto found; /* Actually this returns NULL.  */

  /* First try in the same directory as the original file.

     FIXME: Strictly speaking if we are using the build-id method,
     (ie include_dirs == FALSE) then we should only check absolute
     paths, not relative ones like this one (and the next one).
     The check is left in however as this allows the binutils
     testsuite to exercise this feature without having to install
     a file into the root filesystem.  (See binutils/testsuite/
     binutils-all/objdump.exp for the test).  */
  sprintf (debugfile, "%s%s", dir, base);
  if (check_func (debugfile, func_data))
    goto found;

  /* Then try in a subdirectory called .debug.  */
  sprintf (debugfile, "%s.debug/%s", dir, base);
  if (check_func (debugfile, func_data))
    goto found;

#ifdef EXTRA_DEBUG_ROOT1
  /* Try the first extra debug file root.  */
  sprintf (debugfile, "%s%s%s", EXTRA_DEBUG_ROOT1,
	   include_dirs ? canon_dir : "/", base);
  if (check_func (debugfile, func_data))
    goto found;
#endif

#ifdef EXTRA_DEBUG_ROOT2
  /* Try the second extra debug file root.  */
  sprintf (debugfile, "%s%s%s", EXTRA_DEBUG_ROOT2,
	   include_dirs ? canon_dir : "/", base);
  if (check_func (debugfile, func_data))
    goto found;
#endif

  /* Then try in the global debugfile directory.  */
  strcpy (debugfile, debug_file_directory);
  dirlen = strlen (debug_file_directory) - 1;
  if (include_dirs)
    {
      if (dirlen > 0
	  && debug_file_directory[dirlen] != '/'
	  && canon_dir[0] != '/')
	strcat (debugfile, "/");
      strcat (debugfile, canon_dir);
    }
  else
    {
      if (dirlen > 0 && debug_file_directory[dirlen] != '/')
	strcat (debugfile, "/");
    }
  strcat (debugfile, base);

  if (check_func (debugfile, func_data))
    goto found;

  /* Failed to find the file.  */
  free (debugfile);
  debugfile = NULL;

 found:
  free (base);
  free (dir);
  free (canon_dir);
  return debugfile;
}

/*
FUNCTION
	bfd_follow_gnu_debuglink

SYNOPSIS
	char *bfd_follow_gnu_debuglink (bfd *abfd, const char *dir);

DESCRIPTION
	Takes a BFD and searches it for a .gnu_debuglink section.  If this
	section is found, it examines the section for the name and checksum
	of a '.debug' file containing auxiliary debugging information.  It
	then searches the filesystem for this .debug file in some standard
	locations, including the directory tree rooted at @var{dir}, and if
	found returns the full filename.

	If @var{dir} is NULL, the search will take place starting at
	the current directory.

	Returns <<NULL>> on any errors or failure to locate the .debug
	file, otherwise a pointer to a heap-allocated string
	containing the filename.  The caller is responsible for
	freeing this string.
*/

char *
bfd_follow_gnu_debuglink (bfd *abfd, const char *dir)
{
  uint32_t crc32;

  return find_separate_debug_file (abfd, dir, true,
				   bfd_get_debug_link_info_1,
				   separate_debug_file_exists, &crc32);
}

/* Helper for bfd_follow_gnu_debugaltlink.  It just returns the name
   of the separate debug file.  */

static char *
get_alt_debug_link_info_shim (bfd * abfd, void *unused ATTRIBUTE_UNUSED)
{
  bfd_size_type len;
  bfd_byte *buildid = NULL;
  char *result = bfd_get_alt_debug_link_info (abfd, &len, &buildid);

  free (buildid);

  return result;
}

/*
FUNCTION
	bfd_follow_gnu_debugaltlink

SYNOPSIS
	char *bfd_follow_gnu_debugaltlink (bfd *abfd, const char *dir);

DESCRIPTION
	Takes a BFD and searches it for a .gnu_debugaltlink section.  If this
	section is found, it examines the section for the name of a file
	containing auxiliary debugging information.  It then searches the
	filesystem for this file in a set of standard locations, including
	the directory tree rooted at @var{dir}, and if found returns the
	full filename.

	If @var{dir} is NULL, the search will take place starting at
	the current directory.

	Returns <<NULL>> on any errors or failure to locate the debug
	file, otherwise a pointer to a heap-allocated string
	containing the filename.  The caller is responsible for
	freeing this string.
*/

char *
bfd_follow_gnu_debugaltlink (bfd *abfd, const char *dir)
{
  return find_separate_debug_file (abfd, dir, true,
				   get_alt_debug_link_info_shim,
				   separate_alt_debug_file_exists,
				   NULL);
}

/*
FUNCTION
	bfd_create_gnu_debuglink_section

SYNOPSIS
	struct bfd_section *bfd_create_gnu_debuglink_section
	  (bfd *abfd, const char *filename);

DESCRIPTION
	Takes a @var{BFD} and adds a .gnu_debuglink section to it.  The
	section is sized to be big enough to contain a link to the specified
	@var{filename}.

	A pointer to the new section is returned if all is ok.  Otherwise
	<<NULL>> is returned and bfd_error is set.
*/

asection *
bfd_create_gnu_debuglink_section (bfd *abfd, const char *filename)
{
  asection *sect;
  bfd_size_type debuglink_size;
  flagword flags;

  if (abfd == NULL || filename == NULL)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return NULL;
    }

  /* Strip off any path components in filename.  */
  filename = lbasename (filename);

  sect = bfd_get_section_by_name (abfd, GNU_DEBUGLINK);
  if (sect)
    {
      /* Section already exists.  */
      bfd_set_error (bfd_error_invalid_operation);
      return NULL;
    }

  flags = SEC_HAS_CONTENTS | SEC_READONLY | SEC_DEBUGGING;
  sect = bfd_make_section_with_flags (abfd, GNU_DEBUGLINK, flags);
  if (sect == NULL)
    return NULL;

  /* Compute the size of the section.  Allow for the CRC after the filename,
     and padding so that it will start on a 4-byte boundary.  */
  debuglink_size = strlen (filename) + 1;
  debuglink_size += 3;
  debuglink_size &= ~3;
  debuglink_size += 4;

  if (!bfd_set_section_size (sect, debuglink_size))
    /* XXX Should we delete the section from the bfd ?  */
    return NULL;

  /* PR 21193: Ensure that the section has 4-byte alignment for the CRC.
     Note - despite the name of the function being called, we are
     setting an alignment power, not a byte alignment value.  */
  bfd_set_section_alignment (sect, 2);

  return sect;
}


/*
FUNCTION
	bfd_fill_in_gnu_debuglink_section

SYNOPSIS
	bool bfd_fill_in_gnu_debuglink_section
	  (bfd *abfd, struct bfd_section *sect, const char *filename);

DESCRIPTION
	Takes a @var{BFD} and containing a .gnu_debuglink section @var{SECT}
	and fills in the contents of the section to contain a link to the
	specified @var{filename}.  The filename should be absolute or
	relative to the current directory.

	<<TRUE>> is returned if all is ok.  Otherwise <<FALSE>> is returned
	and bfd_error is set.
*/

bool
bfd_fill_in_gnu_debuglink_section (bfd *abfd,
				   struct bfd_section *sect,
				   const char *filename)
{
  bfd_size_type debuglink_size;
  uint32_t crc32;
  char * contents;
  bfd_size_type crc_offset;
  FILE * handle;
  unsigned char buffer[8 * 1024];
  size_t count;
  size_t filelen;

  if (abfd == NULL || sect == NULL || filename == NULL)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return false;
    }

  /* Open the linked file so that we can compute a CRC.  */
  handle = _bfd_real_fopen (filename, FOPEN_RB);
  if (handle == NULL)
    {
      bfd_set_error (bfd_error_system_call);
      return false;
    }

  crc32 = 0;
  while ((count = fread (buffer, 1, sizeof buffer, handle)) > 0)
    crc32 = bfd_calc_gnu_debuglink_crc32 (crc32, buffer, count);
  fclose (handle);

  /* Strip off any path components in filename,
     now that we no longer need them.  */
  filename = lbasename (filename);

  filelen = strlen (filename);
  debuglink_size = filelen + 1;
  debuglink_size += 3;
  debuglink_size &= ~3;
  debuglink_size += 4;

  contents = (char *) bfd_malloc (debuglink_size);
  if (contents == NULL)
    {
      /* XXX Should we delete the section from the bfd ?  */
      return false;
    }

  crc_offset = debuglink_size - 4;
  memcpy (contents, filename, filelen);
  memset (contents + filelen, 0, crc_offset - filelen);

  bfd_put_32 (abfd, crc32, contents + crc_offset);

  if (! bfd_set_section_contents (abfd, sect, contents, 0, debuglink_size))
    {
      /* XXX Should we delete the section from the bfd ?  */
      free (contents);
      return false;
    }

  return true;
}

/* Finds the build-id associated with @var{abfd}.  If the build-id is
   extracted from the note section then a build-id structure is built
   for it, using memory allocated to @var{abfd}, and this is then
   attached to the @var{abfd}.

   Returns a pointer to the build-id structure if a build-id could be
   found.  If no build-id is found NULL is returned and error code is
   set.  */

static struct bfd_build_id *
get_build_id (bfd *abfd)
{
  struct bfd_build_id *build_id;
  Elf_Internal_Note inote;
  Elf_External_Note *enote;
  bfd_byte *contents;
  asection *sect;
  bfd_size_type size;

  BFD_ASSERT (abfd);

  if (abfd->build_id && abfd->build_id->size > 0)
    /* Save some time by using the already computed build-id.  */
    return (struct bfd_build_id *) abfd->build_id;

  sect = bfd_get_section_by_name (abfd, ".note.gnu.build-id");
  if (sect == NULL
      || (sect->flags & SEC_HAS_CONTENTS) == 0)
    {
      bfd_set_error (bfd_error_no_debug_section);
      return NULL;
    }

  size = bfd_section_size (sect);
  /* FIXME: Should we support smaller build-id notes ?  */
  if (size < 0x24)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return NULL;
    }

  if (!bfd_malloc_and_get_section (abfd, sect, & contents))
    return NULL;

  /* FIXME: Paranoia - allow for compressed build-id sections.
     Maybe we should complain if this size is different from
     the one obtained above...  */
  size = bfd_section_size (sect);
  if (size < sizeof (Elf_External_Note))
    {
      bfd_set_error (bfd_error_invalid_operation);
      free (contents);
      return NULL;
    }

  enote = (Elf_External_Note *) contents;
  inote.type = H_GET_32 (abfd, enote->type);
  inote.namesz = H_GET_32 (abfd, enote->namesz);
  inote.namedata = enote->name;
  inote.descsz = H_GET_32 (abfd, enote->descsz);
  inote.descdata = inote.namedata + BFD_ALIGN (inote.namesz, 4);
  /* FIXME: Should we check for extra notes in this section ?  */

  if (inote.descsz <= 0
      || inote.type != NT_GNU_BUILD_ID
      || inote.namesz != 4 /* sizeof "GNU"  */
      || !startswith (inote.namedata, "GNU")
      || inote.descsz > 0x7ffffffe
      || size < (12 + BFD_ALIGN (inote.namesz, 4) + inote.descsz))
    {
      free (contents);
      bfd_set_error (bfd_error_invalid_operation);
      return NULL;
    }

  build_id = bfd_alloc (abfd, sizeof (struct bfd_build_id) + inote.descsz);
  if (build_id == NULL)
    {
      free (contents);
      return NULL;
    }

  build_id->size = inote.descsz;
  memcpy (build_id->data, inote.descdata, inote.descsz);
  abfd->build_id = build_id;
  free (contents);

  return build_id;
}

/* Searches @var{abfd} for a build-id, and then constructs a pathname
   from it.  The path is computed as .build-id/NN/NN+NN.debug where
   NNNN+NN is the build-id value as a hexadecimal string.

   Returns the constructed filename or NULL upon error.  It is the
   caller's responsibility to free the memory used to hold the
   filename.  If a filename is returned then the @var{build_id_out_p}
   parameter (which points to a @code{struct bfd_build_id} pointer) is
   set to a pointer to the build_id structure.  */

static char *
get_build_id_name (bfd *abfd, void *build_id_out_p)
{
  struct bfd_build_id **build_id_out = build_id_out_p;
  struct bfd_build_id *build_id;
  char *name;
  char *n;
  bfd_size_type s;
  bfd_byte *d;

  if (abfd == NULL || bfd_get_filename (abfd) == NULL || build_id_out == NULL)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return NULL;
    }

  build_id = get_build_id (abfd);
  if (build_id == NULL)
    return NULL;

  /* Compute the debug pathname corresponding to the build-id.  */
  name = bfd_malloc (strlen (".build-id/") + build_id->size * 2 + 2 + strlen (".debug"));
  if (name == NULL)
    {
      bfd_set_error (bfd_error_no_memory);
      return NULL;
    }
  n = name;
  d = build_id->data;
  s = build_id->size;

  n += sprintf (n, ".build-id/");
  n += sprintf (n, "%02x", (unsigned) *d++); s--;
  n += sprintf (n, "/");
  while (s--)
    n += sprintf (n, "%02x", (unsigned) *d++);
  n += sprintf (n, ".debug");

  *build_id_out = build_id;
  return name;
}

/* Checks to see if @var{name} is a readable file and if its build-id
   matches @var{buildid}.

   Returns TRUE if the file exists, is readable, and contains a
   build-id which matches the build-id pointed at by @var{build_id_p}
   (which is really a @code{struct bfd_build_id **}).  */

static bool
check_build_id_file (const char *name, void *buildid_p)
{
  struct bfd_build_id *orig_build_id;
  struct bfd_build_id *build_id;
  bfd * file;
  bool result;

  BFD_ASSERT (name);
  BFD_ASSERT (buildid_p);

  file = bfd_openr (name, NULL);
  if (file == NULL)
    return false;

  /* If the file is an archive, process all of its elements.  */
  if (! bfd_check_format (file, bfd_object))
    {
      bfd_close (file);
      return false;
    }

  build_id = get_build_id (file);
  if (build_id == NULL)
    {
      bfd_close (file);
      return false;
    }

  orig_build_id = *(struct bfd_build_id **) buildid_p;

  result = build_id->size == orig_build_id->size
    && memcmp (build_id->data, orig_build_id->data, build_id->size) == 0;

  (void) bfd_close (file);

  return result;
}

/*
FUNCTION
	bfd_follow_build_id_debuglink

SYNOPSIS
	char *bfd_follow_build_id_debuglink (bfd *abfd, const char *dir);

DESCRIPTION
	Takes @var{abfd} and searches it for a .note.gnu.build-id section.
	If this section is found, it extracts the value of the NT_GNU_BUILD_ID
	note, which should be a hexadecimal value @var{NNNN+NN} (for
	32+ hex digits).  It then searches the filesystem for a file named
	@var{.build-id/NN/NN+NN.debug} in a set of standard locations,
	including the directory tree rooted at @var{dir}.  The filename
	of the first matching file to be found is returned.  A matching
	file should contain a .note.gnu.build-id section with the same
	@var{NNNN+NN} note as @var{abfd}, although this check is currently
	not implemented.

	If @var{dir} is NULL, the search will take place starting at
	the current directory.

	Returns <<NULL>> on any errors or failure to locate the debug
	file, otherwise a pointer to a heap-allocated string
	containing the filename.  The caller is responsible for
	freeing this string.
*/

char *
bfd_follow_build_id_debuglink (bfd *abfd, const char *dir)
{
  struct bfd_build_id *build_id;

  return find_separate_debug_file (abfd, dir, false,
				   get_build_id_name,
				   check_build_id_file, &build_id);
}

/*
FUNCTION
	bfd_set_filename

SYNOPSIS
	const char *bfd_set_filename (bfd *abfd, const char *filename);

DESCRIPTION
	Set the filename of @var{abfd}, copying the FILENAME parameter to
	bfd_alloc'd memory owned by @var{abfd}.  Returns a pointer the
	newly allocated name, or NULL if the allocation failed.
*/

const char *
bfd_set_filename (bfd *abfd, const char *filename)
{
  size_t len = strlen (filename) + 1;
  char *n = bfd_alloc (abfd, len);

  if (n == NULL)
    return NULL;

  if (abfd->filename != NULL)
    {
      /* PR 29389.  If we attempt to rename a file that has been closed due
	 to caching, then we will not be able to reopen it later on.  */
      if (abfd->iostream == NULL && (abfd->flags & BFD_CLOSED_BY_CACHE))
	{
	  bfd_set_error (bfd_error_invalid_operation);
	  return NULL;
	}

      /* Similarly if we attempt to close a renamed file because the
	 cache is now full, we will not be able to reopen it later on.  */
      if (abfd->iostream != NULL)
	abfd->cacheable = 0;
    }

  memcpy (n, filename, len);
  abfd->filename = n;

  return n;
}
