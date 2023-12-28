/* Copyright (C) 2021-2023 Free Software Foundation, Inc.
   Contributed by Oracle.

   This file is part of GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "config.h"
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>     //  for close();

#include "util.h"
#include "Data_window.h"
#include "debug.h"

enum
{
  MINBUFSIZE    = 1 << 16,
  WIN_ALIGN     = 8
};

Data_window::Data_window (char *file_name)
{
  Dprintf (DEBUG_DATA_WINDOW, NTXT ("Data_window:%d %s\n"), (int) __LINE__, STR (file_name));
  page_size = sysconf (_SC_PAGESIZE);
  need_swap_endian = false;
  opened = false;
  fsize = 0;
  base = NULL;
  woffset = 0;
  wsize = 0;
  basesize = 0;
  fname = dbe_strdup (file_name);
  mmap_on_file = false;
  use_mmap = false;
#if DEBUG
  if (DBE_USE_MMAP)
    use_mmap = true;
#endif /* DEBUG */
  fd = open64 (fname, O_RDONLY);
  if (fd == -1)
    return;
  fsize = lseek (fd, 0, SEEK_END);
  if (fsize == 0)
    {
      close (fd);
      fd = -1;
      return;
    }
  opened = true;
  if (use_mmap)
    {
      if (fsize != -1)
	{
	  base = (void*) mmap (NULL, (size_t) fsize, PROT_READ, MAP_PRIVATE, fd, 0);
	  close (fd);
	  fd = -1;
	  if (base == MAP_FAILED)
	    {
	      base = NULL;
	      use_mmap = false;
	      return;
	    }
	  mmap_on_file = true;
	  wsize = fsize;
	}
    }
}

void *
Data_window::bind (int64_t file_offset, int64_t minSize)
{
  Span span;
  span.length = fsize - file_offset;
  span.offset = file_offset;
  return bind (&span, minSize);
}

void *
Data_window::bind (Span *span, int64_t minSize)
{
  // Do any necessary mapping to access the desired span of data
  // and return a pointer to the first byte.
  Dprintf (DEBUG_DATA_WINDOW, NTXT ("Data_window:bind:%d offset=%llx:%lld minSize=%lld \n"),
	   (int) __LINE__, (long long) span->offset, (long long) span->length, (long long) minSize);
  if (minSize == 0 || span->length < minSize)
    return NULL;

  if (span->offset < woffset || span->offset + minSize > woffset + wsize)
    {
      // Remap the window
      if (span->offset + minSize > fsize)
	return NULL;
      int myfd = fd;
      if (myfd == -1)
	{
	  if (fname)
	    myfd = open64 (fname, O_RDONLY, 0);
	  if (myfd == -1)
	    return NULL;
	}
      bool remap_failed = true;
      if (use_mmap)
	{
	  if (base)
	    {
	      munmap ((caddr_t) base, (size_t) wsize);
	      base = NULL;
	    }
	  woffset = span->offset & ~(page_size - 1);
	  wsize = page_size * ((MINBUFSIZE + page_size - 1) / page_size);
	  if (span->offset + minSize > woffset + wsize)
	    // Extend a window
	    wsize += page_size * ((span->offset + minSize -
				   woffset - wsize + page_size - 1) / page_size);
	  base = (void *) mmap (0, (size_t) wsize, PROT_READ, MAP_SHARED, fd, woffset);
	  if (base == MAP_FAILED)
	    {
	      base = NULL;
	      use_mmap = false;
	    }
	  remap_failed = (base == NULL);
	}
      if (remap_failed)
	{
	  remap_failed = false;
	  woffset = span->offset & ~(WIN_ALIGN - 1);
	  wsize = minSize + (span->offset % WIN_ALIGN);
	  if (wsize < MINBUFSIZE)
	    wsize = MINBUFSIZE;
	  if (wsize > fsize)
	    wsize = fsize;
	  if (basesize < wsize)
	    { // Need to realloc 'base'
	      free (base);
	      basesize = wsize;
	      base = (void *) malloc (basesize);
	      Dprintf (DEBUG_DATA_WINDOW,
		       NTXT ("Data_window:bind:%d realloc basesize=%llx woffset=%lld \n"),
		       (int) __LINE__, (long long) basesize, (long long) woffset);
	      if (base == NULL)
		{
		  basesize = 0;
		  remap_failed = true;
		}
	    }
	  if (wsize > fsize - woffset)
	    wsize = fsize - woffset;
	  off_t woff = (off_t) woffset;
	  if (base == NULL || woff != lseek (myfd, woff, SEEK_SET)
	      || wsize != read_from_file (myfd, base, wsize))
	    remap_failed = true;
	}
      if (fd == -1)
	close (myfd);
      if (remap_failed)
	{
	  woffset = 0;
	  wsize = 0;
	  return NULL;
	}
    }
  return (void *) ((char*) base + span->offset - woffset);
}

void *
Data_window::get_data (int64_t offset, int64_t size, void *datap)
{
  if (size <= 0)
    return NULL;
  void *buf = bind (offset, size);
  if (buf == NULL)
    return NULL;
  if (datap == NULL && !mmap_on_file)
    // Can be remmaped or reallocated. Need to make a copy
    datap = (void *) malloc (size);
  if (datap)
    {
      memcpy (datap, buf, (size_t) size);
      return datap;
    }
  return buf;
}

Data_window::~Data_window ()
{
  free (fname);
  if (fd != -1)
    close (fd);
  if (base)
    {
      if (use_mmap)
	munmap ((caddr_t) base, (size_t) wsize);
      else
	free (base);
    }
}

int64_t
Data_window::get_buf_size ()
{
  int64_t sz = MINBUFSIZE;
  if (sz < basesize)
    sz = basesize;
  if (sz > fsize)
    sz = fsize;
  return sz;
}

int64_t
Data_window::copy_to_file (int f, int64_t offset, int64_t size)
{
  long long bsz = get_buf_size ();
  for (long long n = 0; n < size;)
    {
      long long sz = (bsz <= (size - n)) ? bsz : (size - n);
      void *b = bind (offset + n, sz);
      if (b == NULL)
	return n;
      long long len = write (f, b, sz);
      if (len <= 0)
	return n;
      n += len;
    }
  return size;
}
