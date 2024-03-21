/* Low-level I/O routines for BFDs.

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
#include <limits.h>
#include "bfd.h"
#include "libbfd.h"
#include "aout/ar.h"
#if defined (_WIN32)
#include <windows.h>
#include <locale.h>
#endif

#ifndef S_IXUSR
#define S_IXUSR 0100    /* Execute by owner.  */
#endif
#ifndef S_IXGRP
#define S_IXGRP 0010    /* Execute by group.  */
#endif
#ifndef S_IXOTH
#define S_IXOTH 0001    /* Execute by others.  */
#endif

#ifndef FD_CLOEXEC
#define FD_CLOEXEC 1
#endif

file_ptr
_bfd_real_ftell (FILE *file)
{
#if defined (HAVE_FTELLO64)
  return ftello64 (file);
#elif defined (HAVE_FTELLO)
  return ftello (file);
#else
  return ftell (file);
#endif
}

int
_bfd_real_fseek (FILE *file, file_ptr offset, int whence)
{
#if defined (HAVE_FSEEKO64)
  return fseeko64 (file, offset, whence);
#elif defined (HAVE_FSEEKO)
  return fseeko (file, offset, whence);
#else
  return fseek (file, offset, whence);
#endif
}

/* Mark FILE as close-on-exec.  Return FILE.  FILE may be NULL, in
   which case nothing is done.  */
static FILE *
close_on_exec (FILE *file)
{
#if defined (HAVE_FILENO) && defined (F_GETFD)
  if (file)
    {
      int fd = fileno (file);
      int old = fcntl (fd, F_GETFD, 0);
      if (old >= 0)
	fcntl (fd, F_SETFD, old | FD_CLOEXEC);
    }
#endif
  return file;
}

FILE *
_bfd_real_fopen (const char *filename, const char *modes)
{
#ifdef VMS
  char *vms_attr;

  /* On VMS, fopen allows file attributes as optional arguments.
     We need to use them but we'd better to use the common prototype.
     In fopen-vms.h, they are separated from the mode with a comma.
     Split here.  */
  vms_attr = strchr (modes, ',');
  if (vms_attr != NULL)
    {
      /* Attributes found.  Split.  */
      size_t modes_len = strlen (modes) + 1;
      char attrs[modes_len + 1];
      char *at[3];
      int i;

      memcpy (attrs, modes, modes_len);
      at[0] = attrs;
      for (i = 0; i < 2; i++)
	{
	  at[i + 1] = strchr (at[i], ',');
	  BFD_ASSERT (at[i + 1] != NULL);
	  *(at[i + 1]++) = 0; /* Replace ',' with a nul, and skip it.  */
	}
      return close_on_exec (fopen (filename, at[0], at[1], at[2]));
    }

#elif defined (_WIN32)
  /* PR 25713: Handle extra long path names possibly containing '..' and '.'.  */
   wchar_t **     lpFilePart = {NULL};
   const wchar_t  prefix[] = L"\\\\?\\";
   const size_t   partPathLen = strlen (filename) + 1;
#ifdef __MINGW32__
#if !HAVE_DECL____LC_CODEPAGE_FUNC
/* This prototype was added to locale.h in version 9.0 of MinGW-w64.  */
   _CRTIMP unsigned int __cdecl ___lc_codepage_func (void);
#endif
   const unsigned int cp = ___lc_codepage_func ();
#else
   const unsigned int cp = CP_UTF8;
#endif

   /* Converting the partial path from ascii to unicode.
      1) Get the length: Calling with lpWideCharStr set to null returns the length.
      2) Convert the string: Calling with cbMultiByte set to -1 includes the terminating null.  */
   size_t         partPathWSize = MultiByteToWideChar (cp, 0, filename, -1, NULL, 0);
   wchar_t *      partPath = calloc (partPathWSize, sizeof(wchar_t));
   size_t         ix;

   MultiByteToWideChar (cp, 0, filename, -1, partPath, partPathWSize);

   /* Convert any UNIX style path separators into the DOS i.e. backslash separator.  */
   for (ix = 0; ix < partPathLen; ix++)
     if (IS_UNIX_DIR_SEPARATOR(filename[ix]))
       partPath[ix] = '\\';

   /* Getting the full path from the provided partial path.
      1) Get the length.
      2) Resolve the path.  */
   long       fullPathWSize = GetFullPathNameW (partPath, 0, NULL, lpFilePart);
   wchar_t *  fullPath = calloc (fullPathWSize + sizeof(prefix) + 1, sizeof(wchar_t));

   wcscpy (fullPath, prefix);

   int        prefixLen = sizeof(prefix) / sizeof(wchar_t);

   /* Do not add a prefix to the null device.  */
   if (stricmp (filename, "nul") == 0)
    prefixLen = 1;

   wchar_t *  fullPathOffset = fullPath + prefixLen - 1;

   GetFullPathNameW (partPath, fullPathWSize, fullPathOffset, lpFilePart);
   free (partPath);

   /* It is non-standard for modes to exceed 16 characters.  */
   wchar_t    modesW[16];

   MultiByteToWideChar (cp, 0, modes, -1, modesW, sizeof(modesW));

   FILE *     file = _wfopen (fullPath, modesW);
   free (fullPath);

   return close_on_exec (file);

#elif defined (HAVE_FOPEN64)
  return close_on_exec (fopen64 (filename, modes));

#else
  return close_on_exec (fopen (filename, modes));
#endif
}

/*
INTERNAL_DEFINITION
	struct bfd_iovec

DESCRIPTION

	The <<struct bfd_iovec>> contains the internal file I/O class.
	Each <<BFD>> has an instance of this class and all file I/O is
	routed through it (it is assumed that the instance implements
	all methods listed below).

.struct bfd_iovec
.{
.  {* To avoid problems with macros, a "b" rather than "f"
.     prefix is prepended to each method name.  *}
.  {* Attempt to read/write NBYTES on ABFD's IOSTREAM storing/fetching
.     bytes starting at PTR.  Return the number of bytes actually
.     transfered (a read past end-of-file returns less than NBYTES),
.     or -1 (setting <<bfd_error>>) if an error occurs.  *}
.  file_ptr (*bread) (struct bfd *abfd, void *ptr, file_ptr nbytes);
.  file_ptr (*bwrite) (struct bfd *abfd, const void *ptr,
.		       file_ptr nbytes);
.  {* Return the current IOSTREAM file offset, or -1 (setting <<bfd_error>>
.     if an error occurs.  *}
.  file_ptr (*btell) (struct bfd *abfd);
.  {* For the following, on successful completion a value of 0 is returned.
.     Otherwise, a value of -1 is returned (and <<bfd_error>> is set).  *}
.  int (*bseek) (struct bfd *abfd, file_ptr offset, int whence);
.  int (*bclose) (struct bfd *abfd);
.  int (*bflush) (struct bfd *abfd);
.  int (*bstat) (struct bfd *abfd, struct stat *sb);
.  {* Mmap a part of the files. ADDR, LEN, PROT, FLAGS and OFFSET are the usual
.     mmap parameter, except that LEN and OFFSET do not need to be page
.     aligned.  Returns (void *)-1 on failure, mmapped address on success.
.     Also write in MAP_ADDR the address of the page aligned buffer and in
.     MAP_LEN the size mapped (a page multiple).  Use unmap with MAP_ADDR and
.     MAP_LEN to unmap.  *}
.  void *(*bmmap) (struct bfd *abfd, void *addr, bfd_size_type len,
.		   int prot, int flags, file_ptr offset,
.		   void **map_addr, bfd_size_type *map_len);
.};

.extern const struct bfd_iovec _bfd_memory_iovec;
.
*/


/*
FUNCTION
	bfd_bread

SYNOPSIS
	bfd_size_type bfd_bread (void *, bfd_size_type, bfd *);

DESCRIPTION
	Attempt to read SIZE bytes from ABFD's iostream to PTR.
	Return the amount read.
*/

bfd_size_type
bfd_bread (void *ptr, bfd_size_type size, bfd *abfd)
{
  file_ptr nread;
  bfd *element_bfd = abfd;
  ufile_ptr offset = 0;

  while (abfd->my_archive != NULL
	 && !bfd_is_thin_archive (abfd->my_archive))
    {
      offset += abfd->origin;
      abfd = abfd->my_archive;
    }
  offset += abfd->origin;

  /* If this is a non-thin archive element, don't read past the end of
     this element.  */
  if (element_bfd->arelt_data != NULL
      && element_bfd->my_archive != NULL
      && !bfd_is_thin_archive (element_bfd->my_archive))
    {
      bfd_size_type maxbytes = arelt_size (element_bfd);

      if (abfd->where < offset || abfd->where - offset >= maxbytes)
	{
	  bfd_set_error (bfd_error_invalid_operation);
	  return -1;
	}
      if (abfd->where - offset + size > maxbytes)
	size = maxbytes - (abfd->where - offset);
    }

  if (abfd->iovec == NULL)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return -1;
    }

  nread = abfd->iovec->bread (abfd, ptr, size);
  if (nread != -1)
    abfd->where += nread;

  return nread;
}

/*
FUNCTION
	bfd_bwrite

SYNOPSIS
	bfd_size_type bfd_bwrite (const void *, bfd_size_type, bfd *);

DESCRIPTION
	Attempt to write SIZE bytes to ABFD's iostream from PTR.
	Return the amount written.
*/

bfd_size_type
bfd_bwrite (const void *ptr, bfd_size_type size, bfd *abfd)
{
  file_ptr nwrote;

  while (abfd->my_archive != NULL
	 && !bfd_is_thin_archive (abfd->my_archive))
    abfd = abfd->my_archive;

  if (abfd->iovec == NULL)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return -1;
    }

  nwrote = abfd->iovec->bwrite (abfd, ptr, size);
  if (nwrote != -1)
    abfd->where += nwrote;
  if ((bfd_size_type) nwrote != size)
    {
#ifdef ENOSPC
      errno = ENOSPC;
#endif
      bfd_set_error (bfd_error_system_call);
    }
  return nwrote;
}

/*
FUNCTION
	bfd_tell

SYNOPSIS
	file_ptr bfd_tell (bfd *);

DESCRIPTION
	Return ABFD's iostream file position.
*/

file_ptr
bfd_tell (bfd *abfd)
{
  ufile_ptr offset = 0;
  file_ptr ptr;

  while (abfd->my_archive != NULL
	 && !bfd_is_thin_archive (abfd->my_archive))
    {
      offset += abfd->origin;
      abfd = abfd->my_archive;
    }
  offset += abfd->origin;

  if (abfd->iovec == NULL)
    return 0;

  ptr = abfd->iovec->btell (abfd);
  abfd->where = ptr;
  return ptr - offset;
}

/*
FUNCTION
	bfd_flush

SYNOPSIS
	int bfd_flush (bfd *);

DESCRIPTION
	Flush ABFD's iostream pending IO.
*/

int
bfd_flush (bfd *abfd)
{
  while (abfd->my_archive != NULL
	 && !bfd_is_thin_archive (abfd->my_archive))
    abfd = abfd->my_archive;

  if (abfd->iovec == NULL)
    return 0;

  return abfd->iovec->bflush (abfd);
}

/*
FUNCTION
	bfd_stat

SYNOPSIS
	int bfd_stat (bfd *, struct stat *);

DESCRIPTION
	Call fstat on ABFD's iostream.  Return 0 on success, and a
	negative value on failure.
*/

int
bfd_stat (bfd *abfd, struct stat *statbuf)
{
  int result;

  while (abfd->my_archive != NULL
	 && !bfd_is_thin_archive (abfd->my_archive))
    abfd = abfd->my_archive;

  if (abfd->iovec == NULL)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return -1;
    }

  result = abfd->iovec->bstat (abfd, statbuf);
  if (result < 0)
    bfd_set_error (bfd_error_system_call);
  return result;
}

/*
FUNCTION
	bfd_seek

SYNOPSIS
	int bfd_seek (bfd *, file_ptr, int);

DESCRIPTION
	Call fseek on ABFD's iostream.  Return 0 on success, and a
	negative value on failure.
*/

int
bfd_seek (bfd *abfd, file_ptr position, int direction)
{
  int result;
  ufile_ptr offset = 0;

  while (abfd->my_archive != NULL
	 && !bfd_is_thin_archive (abfd->my_archive))
    {
      offset += abfd->origin;
      abfd = abfd->my_archive;
    }
  offset += abfd->origin;

  if (abfd->iovec == NULL)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return -1;
    }

  /* For the time being, a BFD may not seek to it's end.  The problem
     is that we don't easily have a way to recognize the end of an
     element in an archive.  */
  BFD_ASSERT (direction == SEEK_SET || direction == SEEK_CUR);

  if (direction != SEEK_CUR)
    position += offset;

  result = abfd->iovec->bseek (abfd, position, direction);
  if (result != 0)
    {
      /* An EINVAL error probably means that the file offset was
	 absurd.  */
      if (errno == EINVAL)
	bfd_set_error (bfd_error_file_truncated);
      else
	bfd_set_error (bfd_error_system_call);
    }
  else
    {
      /* Adjust `where' field.  */
      if (direction == SEEK_CUR)
	abfd->where += position;
      else
	abfd->where = position;
    }

  return result;
}

/*
FUNCTION
	bfd_get_mtime

SYNOPSIS
	long bfd_get_mtime (bfd *abfd);

DESCRIPTION
	Return the file modification time (as read from the file system, or
	from the archive header for archive members).

*/

long
bfd_get_mtime (bfd *abfd)
{
  struct stat buf;

  if (abfd->mtime_set)
    return abfd->mtime;

  if (bfd_stat (abfd, &buf) != 0)
    return 0;

  abfd->mtime = buf.st_mtime;		/* Save value in case anyone wants it */
  return buf.st_mtime;
}

/*
FUNCTION
	bfd_get_size

SYNOPSIS
	ufile_ptr bfd_get_size (bfd *abfd);

DESCRIPTION
	Return the file size (as read from file system) for the file
	associated with BFD @var{abfd}.

	The initial motivation for, and use of, this routine is not
	so we can get the exact size of the object the BFD applies to, since
	that might not be generally possible (archive members for example).
	It would be ideal if someone could eventually modify
	it so that such results were guaranteed.

	Instead, we want to ask questions like "is this NNN byte sized
	object I'm about to try read from file offset YYY reasonable?"
	As as example of where we might do this, some object formats
	use string tables for which the first <<sizeof (long)>> bytes of the
	table contain the size of the table itself, including the size bytes.
	If an application tries to read what it thinks is one of these
	string tables, without some way to validate the size, and for
	some reason the size is wrong (byte swapping error, wrong location
	for the string table, etc.), the only clue is likely to be a read
	error when it tries to read the table, or a "virtual memory
	exhausted" error when it tries to allocate 15 bazillon bytes
	of space for the 15 bazillon byte table it is about to read.
	This function at least allows us to answer the question, "is the
	size reasonable?".

	A return value of zero indicates the file size is unknown.
*/

ufile_ptr
bfd_get_size (bfd *abfd)
{
  /* A size of 0 means we haven't yet called bfd_stat.  A size of 1
     means we have a cached value of 0, ie. unknown.  */
  if (abfd->size <= 1 || bfd_write_p (abfd))
    {
      struct stat buf;

      if (abfd->size == 1 && !bfd_write_p (abfd))
	return 0;

      if (bfd_stat (abfd, &buf) != 0
	  || buf.st_size == 0
	  || buf.st_size - (ufile_ptr) buf.st_size != 0)
	{
	  abfd->size = 1;
	  return 0;
	}
      abfd->size = buf.st_size;
    }
  return abfd->size;
}

/*
FUNCTION
	bfd_get_file_size

SYNOPSIS
	ufile_ptr bfd_get_file_size (bfd *abfd);

DESCRIPTION
	Return the file size (as read from file system) for the file
	associated with BFD @var{abfd}.  It supports both normal files
	and archive elements.

*/

ufile_ptr
bfd_get_file_size (bfd *abfd)
{
  ufile_ptr file_size, archive_size = (ufile_ptr) -1;
  unsigned int compression_p2 = 0;

  if (abfd->my_archive != NULL
      && !bfd_is_thin_archive (abfd->my_archive))
    {
      struct areltdata *adata = (struct areltdata *) abfd->arelt_data;
      if (adata != NULL)
	{
	  archive_size = adata->parsed_size;
	  /* If the archive is compressed, assume an element won't
	     expand more than eight times file size.  */
	  if (adata->arch_header != NULL
	      && memcmp (((struct ar_hdr *) adata->arch_header)->ar_fmag,
			 "Z\012", 2) == 0)
	    compression_p2 = 3;
	  abfd = abfd->my_archive;
	}
    }

  file_size = bfd_get_size (abfd) << compression_p2;
  if (archive_size < file_size)
    return archive_size;
  return file_size;
}

/*
FUNCTION
	bfd_mmap

SYNOPSIS
	void *bfd_mmap (bfd *abfd, void *addr, bfd_size_type len,
			int prot, int flags, file_ptr offset,
			void **map_addr, bfd_size_type *map_len);

DESCRIPTION
	Return mmap()ed region of the file, if possible and implemented.
	LEN and OFFSET do not need to be page aligned.  The page aligned
	address and length are written to MAP_ADDR and MAP_LEN.

*/

void *
bfd_mmap (bfd *abfd, void *addr, bfd_size_type len,
	  int prot, int flags, file_ptr offset,
	  void **map_addr, bfd_size_type *map_len)
{
  while (abfd->my_archive != NULL
	 && !bfd_is_thin_archive (abfd->my_archive))
    {
      offset += abfd->origin;
      abfd = abfd->my_archive;
    }
  offset += abfd->origin;

  if (abfd->iovec == NULL)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return (void *) -1;
    }

  return abfd->iovec->bmmap (abfd, addr, len, prot, flags, offset,
			     map_addr, map_len);
}

/* Memory file I/O operations.  */

static file_ptr
memory_bread (bfd *abfd, void *ptr, file_ptr size)
{
  struct bfd_in_memory *bim;
  bfd_size_type get;

  bim = (struct bfd_in_memory *) abfd->iostream;
  get = size;
  if (abfd->where + get > bim->size)
    {
      if (bim->size < (bfd_size_type) abfd->where)
	get = 0;
      else
	get = bim->size - abfd->where;
      bfd_set_error (bfd_error_file_truncated);
    }
  memcpy (ptr, bim->buffer + abfd->where, (size_t) get);
  return get;
}

static file_ptr
memory_bwrite (bfd *abfd, const void *ptr, file_ptr size)
{
  struct bfd_in_memory *bim = (struct bfd_in_memory *) abfd->iostream;

  if (abfd->where + size > bim->size)
    {
      bfd_size_type newsize, oldsize;

      oldsize = (bim->size + 127) & ~(bfd_size_type) 127;
      bim->size = abfd->where + size;
      /* Round up to cut down on memory fragmentation */
      newsize = (bim->size + 127) & ~(bfd_size_type) 127;
      if (newsize > oldsize)
	{
	  bim->buffer = (bfd_byte *) bfd_realloc_or_free (bim->buffer, newsize);
	  if (bim->buffer == NULL)
	    {
	      bim->size = 0;
	      return 0;
	    }
	  if (newsize > bim->size)
	    memset (bim->buffer + bim->size, 0, newsize - bim->size);
	}
    }
  memcpy (bim->buffer + abfd->where, ptr, (size_t) size);
  return size;
}

static file_ptr
memory_btell (bfd *abfd)
{
  return abfd->where;
}

static int
memory_bseek (bfd *abfd, file_ptr position, int direction)
{
  file_ptr nwhere;
  struct bfd_in_memory *bim;

  bim = (struct bfd_in_memory *) abfd->iostream;

  if (direction == SEEK_SET)
    nwhere = position;
  else
    nwhere = abfd->where + position;

  if (nwhere < 0)
    {
      abfd->where = 0;
      errno = EINVAL;
      return -1;
    }

  if ((bfd_size_type)nwhere > bim->size)
    {
      if (abfd->direction == write_direction
	  || abfd->direction == both_direction)
	{
	  bfd_size_type newsize, oldsize;

	  oldsize = (bim->size + 127) & ~(bfd_size_type) 127;
	  bim->size = nwhere;
	  /* Round up to cut down on memory fragmentation */
	  newsize = (bim->size + 127) & ~(bfd_size_type) 127;
	  if (newsize > oldsize)
	    {
	      bim->buffer = (bfd_byte *) bfd_realloc_or_free (bim->buffer, newsize);
	      if (bim->buffer == NULL)
		{
		  errno = EINVAL;
		  bim->size = 0;
		  return -1;
		}
	      memset (bim->buffer + oldsize, 0, newsize - oldsize);
	    }
	}
      else
	{
	  abfd->where = bim->size;
	  errno = EINVAL;
	  bfd_set_error (bfd_error_file_truncated);
	  return -1;
	}
    }
  return 0;
}

static int
memory_bclose (struct bfd *abfd)
{
  struct bfd_in_memory *bim = (struct bfd_in_memory *) abfd->iostream;

  free (bim->buffer);
  free (bim);
  abfd->iostream = NULL;

  return 0;
}

static int
memory_bflush (bfd *abfd ATTRIBUTE_UNUSED)
{
  return 0;
}

static int
memory_bstat (bfd *abfd, struct stat *statbuf)
{
  struct bfd_in_memory *bim = (struct bfd_in_memory *) abfd->iostream;

  memset (statbuf, 0, sizeof (*statbuf));
  statbuf->st_size = bim->size;

  return 0;
}

static void *
memory_bmmap (bfd *abfd ATTRIBUTE_UNUSED, void *addr ATTRIBUTE_UNUSED,
	      bfd_size_type len ATTRIBUTE_UNUSED, int prot ATTRIBUTE_UNUSED,
	      int flags ATTRIBUTE_UNUSED, file_ptr offset ATTRIBUTE_UNUSED,
	      void **map_addr ATTRIBUTE_UNUSED,
	      bfd_size_type *map_len ATTRIBUTE_UNUSED)
{
  return (void *)-1;
}

const struct bfd_iovec _bfd_memory_iovec =
{
  &memory_bread, &memory_bwrite, &memory_btell, &memory_bseek,
  &memory_bclose, &memory_bflush, &memory_bstat, &memory_bmmap
};
