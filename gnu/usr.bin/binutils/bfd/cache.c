/* BFD library -- caching of file descriptors.

   Copyright (C) 1990-2023 Free Software Foundation, Inc.

   Hacked by Steve Chamberlain of Cygnus Support (steve@cygnus.com).

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

/*
SECTION
	File caching

	The file caching mechanism is embedded within BFD and allows
	the application to open as many BFDs as it wants without
	regard to the underlying operating system's file descriptor
	limit (often as low as 20 open files).  The module in
	<<cache.c>> maintains a least recently used list of
	<<bfd_cache_max_open>> files, and exports the name
	<<bfd_cache_lookup>>, which runs around and makes sure that
	the required BFD is open. If not, then it chooses a file to
	close, closes it and opens the one wanted, returning its file
	handle.

SUBSECTION
	Caching functions
*/

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"
#include "libiberty.h"

#ifdef HAVE_MMAP
#include <sys/mman.h>
#endif

/* In some cases we can optimize cache operation when reopening files.
   For instance, a flush is entirely unnecessary if the file is already
   closed, so a flush would use CACHE_NO_OPEN.  Similarly, a seek using
   SEEK_SET or SEEK_END need not first seek to the current position.
   For stat we ignore seek errors, just in case the file has changed
   while we weren't looking.  If it has, then it's possible that the
   file is shorter and we don't want a seek error to prevent us doing
   the stat.  */
enum cache_flag {
  CACHE_NORMAL = 0,
  CACHE_NO_OPEN = 1,
  CACHE_NO_SEEK = 2,
  CACHE_NO_SEEK_ERROR = 4
};

/* The maximum number of files which the cache will keep open at
   one time.  When needed call bfd_cache_max_open to initialize.  */

static int max_open_files = 0;

/* Set max_open_files, if not already set, to 12.5% of the allowed open
   file descriptors, but at least 10, and return the value.  */
static int
bfd_cache_max_open (void)
{
  if (max_open_files == 0)
    {
      int max;
#if defined(__sun) && !defined(__sparcv9) && !defined(__x86_64__)
      /* PR ld/19260: 32-bit Solaris has very inelegant handling of the 255
	 file descriptor limit.  The problem is that setrlimit(2) can raise
	 RLIMIT_NOFILE to a value that is not supported by libc, resulting
	 in "Too many open files" errors.  This can happen here even though
	 max_open_files is set to rlim.rlim_cur / 8.  For example, if
	 a parent process has set rlim.rlim_cur to 65536, then max_open_files
	 will be computed as 8192.

	 This check essentially reverts to the behavior from binutils 2.23.1
	 for 32-bit Solaris only.  (It is hoped that the 32-bit libc
	 limitation will be removed soon).  64-bit Solaris libc does not have
	 this limitation.  */
      max = 16;
#else
#ifdef HAVE_GETRLIMIT
      struct rlimit rlim;

      if (getrlimit (RLIMIT_NOFILE, &rlim) == 0
	  && rlim.rlim_cur != (rlim_t) RLIM_INFINITY)
	max = rlim.rlim_cur / 8;
      else
#endif
#ifdef _SC_OPEN_MAX
	max = sysconf (_SC_OPEN_MAX) / 8;
#else
	max = 10;
#endif
#endif /* not 32-bit Solaris */

      max_open_files = max < 10 ? 10 : max;
    }

  return max_open_files;
}

/* The number of BFD files we have open.  */

static int open_files;

/* Zero, or a pointer to the topmost BFD on the chain.  This is
   used by the <<bfd_cache_lookup>> macro in @file{libbfd.h} to
   determine when it can avoid a function call.  */

static bfd *bfd_last_cache = NULL;

/* Insert a BFD into the cache.  */

static void
insert (bfd *abfd)
{
  if (bfd_last_cache == NULL)
    {
      abfd->lru_next = abfd;
      abfd->lru_prev = abfd;
    }
  else
    {
      abfd->lru_next = bfd_last_cache;
      abfd->lru_prev = bfd_last_cache->lru_prev;
      abfd->lru_prev->lru_next = abfd;
      abfd->lru_next->lru_prev = abfd;
    }
  bfd_last_cache = abfd;
}

/* Remove a BFD from the cache.  */

static void
snip (bfd *abfd)
{
  abfd->lru_prev->lru_next = abfd->lru_next;
  abfd->lru_next->lru_prev = abfd->lru_prev;
  if (abfd == bfd_last_cache)
    {
      bfd_last_cache = abfd->lru_next;
      if (abfd == bfd_last_cache)
	bfd_last_cache = NULL;
    }
}

/* Close a BFD and remove it from the cache.  */

static bool
bfd_cache_delete (bfd *abfd)
{
  bool ret;

  if (fclose ((FILE *) abfd->iostream) == 0)
    ret = true;
  else
    {
      ret = false;
      bfd_set_error (bfd_error_system_call);
    }

  snip (abfd);

  abfd->iostream = NULL;
  --open_files;
  abfd->flags |= BFD_CLOSED_BY_CACHE;

  return ret;
}

/* We need to open a new file, and the cache is full.  Find the least
   recently used cacheable BFD and close it.  */

static bool
close_one (void)
{
  register bfd *to_kill;

  if (bfd_last_cache == NULL)
    to_kill = NULL;
  else
    {
      for (to_kill = bfd_last_cache->lru_prev;
	   ! to_kill->cacheable;
	   to_kill = to_kill->lru_prev)
	{
	  if (to_kill == bfd_last_cache)
	    {
	      to_kill = NULL;
	      break;
	    }
	}
    }

  if (to_kill == NULL)
    {
      /* There are no open cacheable BFD's.  */
      return true;
    }

  to_kill->where = _bfd_real_ftell ((FILE *) to_kill->iostream);

  return bfd_cache_delete (to_kill);
}

/* Check to see if the required BFD is the same as the last one
   looked up. If so, then it can use the stream in the BFD with
   impunity, since it can't have changed since the last lookup;
   otherwise, it has to perform the complicated lookup function.  */

#define bfd_cache_lookup(x, flag) \
  ((x) == bfd_last_cache			\
   ? (FILE *) (bfd_last_cache->iostream)	\
   : bfd_cache_lookup_worker (x, flag))

/* Called when the macro <<bfd_cache_lookup>> fails to find a
   quick answer.  Find a file descriptor for @var{abfd}.  If
   necessary, it open it.  If there are already more than
   <<bfd_cache_max_open>> files open, it tries to close one first, to
   avoid running out of file descriptors.  It will return NULL
   if it is unable to (re)open the @var{abfd}.  */

static FILE *
bfd_cache_lookup_worker (bfd *abfd, enum cache_flag flag)
{
  if ((abfd->flags & BFD_IN_MEMORY) != 0)
    abort ();

  if (abfd->my_archive != NULL
      && !bfd_is_thin_archive (abfd->my_archive))
    abort ();

  if (abfd->iostream != NULL)
    {
      /* Move the file to the start of the cache.  */
      if (abfd != bfd_last_cache)
	{
	  snip (abfd);
	  insert (abfd);
	}
      return (FILE *) abfd->iostream;
    }

  if (flag & CACHE_NO_OPEN)
    return NULL;

  if (bfd_open_file (abfd) == NULL)
    ;
  else if (!(flag & CACHE_NO_SEEK)
	   && _bfd_real_fseek ((FILE *) abfd->iostream,
			       abfd->where, SEEK_SET) != 0
	   && !(flag & CACHE_NO_SEEK_ERROR))
    bfd_set_error (bfd_error_system_call);
  else
    return (FILE *) abfd->iostream;

  /* xgettext:c-format */
  _bfd_error_handler (_("reopening %pB: %s"),
		      abfd, bfd_errmsg (bfd_get_error ()));
  return NULL;
}

static file_ptr
cache_btell (struct bfd *abfd)
{
  FILE *f = bfd_cache_lookup (abfd, CACHE_NO_OPEN);
  if (f == NULL)
    return abfd->where;
  return _bfd_real_ftell (f);
}

static int
cache_bseek (struct bfd *abfd, file_ptr offset, int whence)
{
  FILE *f = bfd_cache_lookup (abfd, whence != SEEK_CUR ? CACHE_NO_SEEK : CACHE_NORMAL);
  if (f == NULL)
    return -1;
  return _bfd_real_fseek (f, offset, whence);
}

/* Note that archive entries don't have streams; they share their parent's.
   This allows someone to play with the iostream behind BFD's back.

   Also, note that the origin pointer points to the beginning of a file's
   contents (0 for non-archive elements).  For archive entries this is the
   first octet in the file, NOT the beginning of the archive header.  */

static file_ptr
cache_bread_1 (FILE *f, void *buf, file_ptr nbytes)
{
  file_ptr nread;

#if defined (__VAX) && defined (VMS)
  /* Apparently fread on Vax VMS does not keep the record length
     information.  */
  nread = read (fileno (f), buf, nbytes);
  /* Set bfd_error if we did not read as much data as we expected.  If
     the read failed due to an error set the bfd_error_system_call,
     else set bfd_error_file_truncated.  */
  if (nread == (file_ptr)-1)
    {
      bfd_set_error (bfd_error_system_call);
      return nread;
    }
#else
  nread = fread (buf, 1, nbytes, f);
  /* Set bfd_error if we did not read as much data as we expected.  If
     the read failed due to an error set the bfd_error_system_call,
     else set bfd_error_file_truncated.  */
  if (nread < nbytes && ferror (f))
    {
      bfd_set_error (bfd_error_system_call);
      return nread;
    }
#endif
  if (nread < nbytes)
    /* This may or may not be an error, but in case the calling code
       bails out because of it, set the right error code.  */
    bfd_set_error (bfd_error_file_truncated);
  return nread;
}

static file_ptr
cache_bread (struct bfd *abfd, void *buf, file_ptr nbytes)
{
  file_ptr nread = 0;
  FILE *f;

  f = bfd_cache_lookup (abfd, CACHE_NORMAL);
  if (f == NULL)
    return -1;

  /* Some filesystems are unable to handle reads that are too large
     (for instance, NetApp shares with oplocks turned off).  To avoid
     hitting this limitation, we read the buffer in chunks of 8MB max.  */
  while (nread < nbytes)
    {
      const file_ptr max_chunk_size = 0x800000;
      file_ptr chunk_size = nbytes - nread;
      file_ptr chunk_nread;

      if (chunk_size > max_chunk_size)
	chunk_size = max_chunk_size;

      chunk_nread = cache_bread_1 (f, (char *) buf + nread, chunk_size);

      /* Update the nread count.

	 We just have to be careful of the case when cache_bread_1 returns
	 a negative count:  If this is our first read, then set nread to
	 that negative count in order to return that negative value to the
	 caller.  Otherwise, don't add it to our total count, or we would
	 end up returning a smaller number of bytes read than we actually
	 did.  */
      if (nread == 0 || chunk_nread > 0)
	nread += chunk_nread;

      if (chunk_nread < chunk_size)
	break;
    }

  return nread;
}

static file_ptr
cache_bwrite (struct bfd *abfd, const void *from, file_ptr nbytes)
{
  file_ptr nwrite;
  FILE *f = bfd_cache_lookup (abfd, CACHE_NORMAL);

  if (f == NULL)
    return 0;
  nwrite = fwrite (from, 1, nbytes, f);
  if (nwrite < nbytes && ferror (f))
    {
      bfd_set_error (bfd_error_system_call);
      return -1;
    }
  return nwrite;
}

static int
cache_bclose (struct bfd *abfd)
{
  return bfd_cache_close (abfd) - 1;
}

static int
cache_bflush (struct bfd *abfd)
{
  int sts;
  FILE *f = bfd_cache_lookup (abfd, CACHE_NO_OPEN);

  if (f == NULL)
    return 0;
  sts = fflush (f);
  if (sts < 0)
    bfd_set_error (bfd_error_system_call);
  return sts;
}

static int
cache_bstat (struct bfd *abfd, struct stat *sb)
{
  int sts;
  FILE *f = bfd_cache_lookup (abfd, CACHE_NO_SEEK_ERROR);

  if (f == NULL)
    return -1;
  sts = fstat (fileno (f), sb);
  if (sts < 0)
    bfd_set_error (bfd_error_system_call);
  return sts;
}

static void *
cache_bmmap (struct bfd *abfd ATTRIBUTE_UNUSED,
	     void *addr ATTRIBUTE_UNUSED,
	     bfd_size_type len ATTRIBUTE_UNUSED,
	     int prot ATTRIBUTE_UNUSED,
	     int flags ATTRIBUTE_UNUSED,
	     file_ptr offset ATTRIBUTE_UNUSED,
	     void **map_addr ATTRIBUTE_UNUSED,
	     bfd_size_type *map_len ATTRIBUTE_UNUSED)
{
  void *ret = (void *) -1;

  if ((abfd->flags & BFD_IN_MEMORY) != 0)
    abort ();
#ifdef HAVE_MMAP
  else
    {
      static uintptr_t pagesize_m1;
      FILE *f;
      file_ptr pg_offset;
      bfd_size_type pg_len;

      f = bfd_cache_lookup (abfd, CACHE_NO_SEEK_ERROR);
      if (f == NULL)
	return ret;

      if (pagesize_m1 == 0)
	pagesize_m1 = getpagesize () - 1;

      /* Align.  */
      pg_offset = offset & ~pagesize_m1;
      pg_len = (len + (offset - pg_offset) + pagesize_m1) & ~pagesize_m1;

      ret = mmap (addr, pg_len, prot, flags, fileno (f), pg_offset);
      if (ret == (void *) -1)
	bfd_set_error (bfd_error_system_call);
      else
	{
	  *map_addr = ret;
	  *map_len = pg_len;
	  ret = (char *) ret + (offset & pagesize_m1);
	}
    }
#endif

  return ret;
}

static const struct bfd_iovec cache_iovec =
{
  &cache_bread, &cache_bwrite, &cache_btell, &cache_bseek,
  &cache_bclose, &cache_bflush, &cache_bstat, &cache_bmmap
};

/*
INTERNAL_FUNCTION
	bfd_cache_init

SYNOPSIS
	bool bfd_cache_init (bfd *abfd);

DESCRIPTION
	Add a newly opened BFD to the cache.
*/

bool
bfd_cache_init (bfd *abfd)
{
  BFD_ASSERT (abfd->iostream != NULL);
  if (open_files >= bfd_cache_max_open ())
    {
      if (! close_one ())
	return false;
    }
  abfd->iovec = &cache_iovec;
  insert (abfd);
  abfd->flags &= ~BFD_CLOSED_BY_CACHE;
  ++open_files;
  return true;
}

/*
FUNCTION
	bfd_cache_close

SYNOPSIS
	bool bfd_cache_close (bfd *abfd);

DESCRIPTION
	Remove the BFD @var{abfd} from the cache. If the attached file is open,
	then close it too.

	<<FALSE>> is returned if closing the file fails, <<TRUE>> is
	returned if all is well.
*/

bool
bfd_cache_close (bfd *abfd)
{
  /* Don't remove this test.  bfd_reinit depends on it.  */
  if (abfd->iovec != &cache_iovec)
    return true;

  if (abfd->iostream == NULL)
    /* Previously closed.  */
    return true;

  return bfd_cache_delete (abfd);
}

/*
FUNCTION
	bfd_cache_close_all

SYNOPSIS
	bool bfd_cache_close_all (void);

DESCRIPTION
	Remove all BFDs from the cache. If the attached file is open,
	then close it too.

	<<FALSE>> is returned if closing one of the file fails, <<TRUE>> is
	returned if all is well.
*/

bool
bfd_cache_close_all (void)
{
  bool ret = true;

  while (bfd_last_cache != NULL)
    ret &= bfd_cache_close (bfd_last_cache);

  return ret;
}

/*
INTERNAL_FUNCTION
	bfd_open_file

SYNOPSIS
	FILE* bfd_open_file (bfd *abfd);

DESCRIPTION
	Call the OS to open a file for @var{abfd}.  Return the <<FILE *>>
	(possibly <<NULL>>) that results from this operation.  Set up the
	BFD so that future accesses know the file is open. If the <<FILE *>>
	returned is <<NULL>>, then it won't have been put in the
	cache, so it won't have to be removed from it.
*/

FILE *
bfd_open_file (bfd *abfd)
{
  abfd->cacheable = true;	/* Allow it to be closed later.  */

  if (open_files >= bfd_cache_max_open ())
    {
      if (! close_one ())
	return NULL;
    }

  switch (abfd->direction)
    {
    case read_direction:
    case no_direction:
      abfd->iostream = _bfd_real_fopen (bfd_get_filename (abfd), FOPEN_RB);
      break;
    case both_direction:
    case write_direction:
      if (abfd->opened_once)
	{
	  abfd->iostream = _bfd_real_fopen (bfd_get_filename (abfd),
					    FOPEN_RUB);
	  if (abfd->iostream == NULL)
	    abfd->iostream = _bfd_real_fopen (bfd_get_filename (abfd),
					      FOPEN_WUB);
	}
      else
	{
	  /* Create the file.

	     Some operating systems won't let us overwrite a running
	     binary.  For them, we want to unlink the file first.

	     However, gcc 2.95 will create temporary files using
	     O_EXCL and tight permissions to prevent other users from
	     substituting other .o files during the compilation.  gcc
	     will then tell the assembler to use the newly created
	     file as an output file.  If we unlink the file here, we
	     open a brief window when another user could still
	     substitute a file.

	     So we unlink the output file if and only if it has
	     non-zero size.  */
#ifndef __MSDOS__
	  /* Don't do this for MSDOS: it doesn't care about overwriting
	     a running binary, but if this file is already open by
	     another BFD, we will be in deep trouble if we delete an
	     open file.  In fact, objdump does just that if invoked with
	     the --info option.  */
	  struct stat s;

	  if (stat (bfd_get_filename (abfd), &s) == 0 && s.st_size != 0)
	    unlink_if_ordinary (bfd_get_filename (abfd));
#endif
	  abfd->iostream = _bfd_real_fopen (bfd_get_filename (abfd),
					    FOPEN_WUB);
	  abfd->opened_once = true;
	}
      break;
    }

  if (abfd->iostream == NULL)
    bfd_set_error (bfd_error_system_call);
  else
    {
      if (! bfd_cache_init (abfd))
	return NULL;
    }

  return (FILE *) abfd->iostream;
}
