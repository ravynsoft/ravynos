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
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "zlib.h"
#include "util.h"
#include "DbeJarFile.h"
#include "Data_window.h"
#include "vec.h"

static uint32_t
get_u1 (unsigned char *b)
{
  return (uint32_t) ((b)[0]);
}

static uint32_t
get_u2 (unsigned char *b)
{
  return (get_u1 (b + 1) << 8) | get_u1 (b);
}

static uint32_t
get_u4 (unsigned char *b)
{
  return (get_u2 (b + 2) << 16) | get_u2 (b);
}

static uint64_t
get_u8 (unsigned char *b)
{
  return (((uint64_t) get_u4 (b + 4)) << 32) | get_u4 (b);
}

enum
{
  END_CENT_DIR_SIZE     = 22,
  LOC_FILE_HEADER_SIZE  = 30,
  CENT_FILE_HEADER_SIZE = 46,
  ZIP64_LOCATOR_SIZE    = 20,
  ZIP64_CENT_DIR_SIZE   = 56,
  ZIP_BUF_SIZE          = 65536
};

struct EndCentDir
{
  uint64_t count;
  uint64_t size;
  uint64_t offset;
};

class ZipEntry
{
public:

  ZipEntry ()
  {
    name = NULL;
    data_offset = 0;
  }

  ~ZipEntry ()
  {
    free (name);
  }

  int
  compare (ZipEntry *ze)
  {
    return dbe_strcmp (name, ze->name);
  }

  char *name;       // entry name
  int time;         // modification time
  int64_t size;     // size of uncompressed data
  int64_t csize;    // size of compressed data (zero if uncompressed)
  uint32_t compressionMethod;
  int64_t offset;   // offset of LOC header
  int64_t data_offset;
};

static int
cmp_names (const void *a, const void *b)
{
  ZipEntry *e1 = *((ZipEntry **) a);
  ZipEntry *e2 = *((ZipEntry **) b);
  return e1->compare (e2);
}

template<> void Vector<ZipEntry *>::dump (const char *msg)
{
  Dprintf (1, NTXT ("Vector<ZipEntry *> %s  [%lld]\n"), msg ? msg : NTXT (""), (long long) size ());
  for (long i = 0, sz = size (); i < sz; i++)
    {
      ZipEntry *ze = get (i);
      Dprintf (1, NTXT ("  %lld offset:%lld (0x%llx) size: %lld --> %lld %s\n"),
	       (long long) i, (long long) ze->offset, (long long) ze->offset,
	       (long long) ze->csize, (long long) ze->size, STR (ze->name));
    }
}

DbeJarFile::DbeJarFile (const char *jarName)
{
  name = strdup (jarName);
  fnames = NULL;
  dwin = new Data_window (name);
  get_entries ();
}

DbeJarFile::~DbeJarFile ()
{
  free (name);
  delete fnames;
}

void
DbeJarFile::get_entries ()
{
  Dprintf (DUMP_JAR_FILE, NTXT ("\nArchive: %s\n"), STR (name));
  if (dwin->not_opened ())
    {
      append_msg (CMSG_ERROR, GTXT ("Cannot open file `%s'"), name);
      return;
    }
  struct EndCentDir endCentDir;
  if (get_EndCentDir (&endCentDir) == 0)
    return;

  if (endCentDir.count == 0)
    {
      append_msg (CMSG_WARN, GTXT ("No files in %s"), name);
      return;
    }
  unsigned char *b = (unsigned char *) dwin->bind (endCentDir.offset, endCentDir.size);
  if (b == NULL)
    {
      append_msg (CMSG_ERROR, GTXT ("%s: cannot read the central directory record"), name);
      return;
    }

  fnames = new Vector<ZipEntry*>(endCentDir.count);
  for (uint64_t i = 0, offset = endCentDir.offset, last = endCentDir.offset + endCentDir.size; i < endCentDir.count; i++)
    {
      if ((last - offset) < CENT_FILE_HEADER_SIZE)
	{
	  append_msg (CMSG_ERROR, GTXT ("%s: cannot read the central file header (%lld (from %lld), offset=0x%016llx last=0x%016llx"),
		      name, (long long) i, (long long) endCentDir.count, (long long) offset, (long long) last);
	  break;
	}
      b = (unsigned char *) dwin->bind (offset, CENT_FILE_HEADER_SIZE);
      //  Central file header
      //  Offset Bytes    Description
      //     0     4   central file header signature = 0x02014b50
      //     4     2   version made by
      //     6     2   version needed to extract
      //     8     2   general purpose bit flag
      //    10     2   compression method
      //    12     2   last mod file time
      //    14     2   last mod file date
      //    16     4   crc-32
      //    20     4   compressed size
      //    24     4   uncompressed size
      //    28     2   file name length
      //    30     2   extra field length
      //    32     2   file comment length
      //    34     2   disk number start
      //    36     2   internal file attributes
      //    38     4   external file attributes
      //    42     4   relative offset of local header
      //    46         file name (variable size)
      //               extra field (variable size)
      //               file comment (variable size)
      uint32_t signature = get_u4 (b);
      if (signature != 0x02014b50)
	{
	  append_msg (CMSG_ERROR, GTXT ("%s: wrong header signature (%lld (total %lld), offset=0x%016llx last=0x%016llx"),
		      name, (long long) i, (long long) endCentDir.count, (long long) offset, (long long) last);
	  break;
	}
      ZipEntry *ze = new ZipEntry ();
      fnames->append (ze);
      uint32_t name_len = get_u2 (b + 28);
      uint32_t extra_len = get_u2 (b + 30);
      uint32_t comment_len = get_u2 (b + 32);
      ze->compressionMethod = get_u2 (b + 10);
      ze->csize = get_u4 (b + 20);
      ze->size = get_u4 (b + 24);
      ze->offset = get_u4 (b + 42);
      char *nm = (char *) dwin->bind (offset + 46, name_len);
      if (nm)
	{
	  ze->name = (char *) malloc (name_len + 1);
	  strncpy (ze->name, nm, name_len);
	  ze->name[name_len] = 0;
	}
      offset += CENT_FILE_HEADER_SIZE + name_len + extra_len + comment_len;
    }
  fnames->sort (cmp_names);
  if (DUMP_JAR_FILE)
    fnames->dump (get_basename (name));
}

int
DbeJarFile::get_entry (const char *fname)
{
  if (fnames == NULL)
    return -1;
  ZipEntry zipEntry, *ze = &zipEntry;
  ze->name = (char *) fname;
  int ind = fnames->bisearch (0, -1, &ze, cmp_names);
  ze->name = NULL;
  return ind;
}

long long
DbeJarFile::copy (char *toFileNname, int fromEntryNum)
{
  if (fromEntryNum < 0 || fromEntryNum >= VecSize (fnames))
    return -1;
  ZipEntry *ze = fnames->get (fromEntryNum);
  if (ze->data_offset == 0)
    {
      //  Local file header
      //  Offset Bytes    Description
      //     0     4   local file header signature = 0x04034b50
      //     4     2   version needed to extract
      //     6     2   general purpose bit flag
      //     8     2   compression method
      //    10     2   last mod file time
      //    12     2   last mod file date
      //    14     4   crc-32
      //    18     4   compressed size
      //    22     4   uncompressed size
      //    26     2   file name length
      //    28     2   extra field length
      //    30     2   file name (variable size)
      //               extra field (variable size)
      unsigned char *b = (unsigned char *) dwin->bind (ze->offset, LOC_FILE_HEADER_SIZE);
      if (b == NULL)
	{
	  append_msg (CMSG_ERROR,
		 GTXT ("%s: Cannot read a local file header (%s offset=0x%lld"),
		 name, STR (ze->name), (long long) ze->offset);
	  return -1;
	}
      uint32_t signature = get_u4 (b);
      if (signature != 0x04034b50)
	{
	  append_msg (CMSG_ERROR,
		      GTXT ("%s: wrong local header signature ('%s' offset=%lld (0x%llx)"),
		      name, STR (ze->name), (long long) ze->offset,
		      (long long) ze->offset);
	  return -1;
	}
      ze->data_offset = ze->offset + LOC_FILE_HEADER_SIZE + get_u2 (b + 26) + get_u2 (b + 28);
    }

  if (ze->compressionMethod == 0)
    {
      int fd = open (toFileNname, O_CREAT | O_WRONLY | O_LARGEFILE, 0644);
      if (fd == -1)
	{
	  append_msg (CMSG_ERROR, GTXT ("Cannot create file %s (%s)"), toFileNname, STR (strerror (errno)));
	  return -1;
	}
      long long len = dwin->copy_to_file (fd, ze->data_offset, ze->size);
      close (fd);
      if (len != ze->size)
	{
	  append_msg (CMSG_ERROR, GTXT ("%s: Cannot write %lld bytes (only %lld)"),
		      toFileNname, (long long) ze->size, (long long) len);
	  unlink (toFileNname);
	  return -1;
	}
      return len;
    }

  unsigned char *b = (unsigned char *) dwin->bind (ze->data_offset, ze->csize);
  if (b == NULL)
    {
      append_msg (CMSG_ERROR,
		  GTXT ("%s: Cannot extract file %s (offset=0x%lld csize=%lld)"),
		  name, STR (ze->name), (long long) ze->offset,
		  (long long) ze->csize);
      return -1;
    }
  z_stream strm;
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  strm.next_in = Z_NULL;
  strm.avail_in = 0;
  if (inflateInit2 (&strm, -MAX_WBITS) != Z_OK)
    {
      append_msg (CMSG_ERROR, GTXT ("%s: inflateInit2 failed (%s)"), STR (ze->name), STR (strm.msg));
      return -1;
    }
  strm.avail_in = ze->csize;
  strm.next_in = b;
  int retval = ze->size;
  unsigned char *buf = (unsigned char *) malloc (ze->size);
  for (;;)
    {
      strm.next_out = buf;
      strm.avail_out = ze->size;
      int ret = inflate (&strm, Z_SYNC_FLUSH);
      if ((ret == Z_NEED_DICT) || (ret == Z_DATA_ERROR) || (ret == Z_MEM_ERROR) || (ret == Z_STREAM_ERROR))
	{
	  append_msg (CMSG_ERROR, GTXT ("%s: inflate('%s') error %d (%s)"), name, STR (ze->name), ret, STR (strm.msg));
	  retval = -1;
	  break;
	}
      if (strm.avail_out != 0)
	break;
    }
  inflateEnd (&strm);
  if (retval != -1)
    {
      int fd = open (toFileNname, O_CREAT | O_WRONLY | O_LARGEFILE, 0644);
      if (fd == -1)
	{
	  append_msg (CMSG_ERROR, GTXT ("Cannot create file %s (%s)"), toFileNname, STR (strerror (errno)));
	  retval = -1;
	}
      else
	{
	  long long len = write (fd, buf, ze->size);
	  if (len != ze->size)
	    {
	      append_msg (CMSG_ERROR, GTXT ("%s: Cannot write %lld bytes (only %lld)"),
			  toFileNname, (long long) strm.avail_out, (long long) len);
	      retval = -1;
	    }
	  close (fd);
	}
    }
  free (buf);
  return retval;
}

int
DbeJarFile::get_EndCentDir (struct EndCentDir *endCentDir)
{
  int64_t fsize = dwin->get_fsize ();
  int64_t sz = (fsize < ZIP_BUF_SIZE) ? fsize : ZIP_BUF_SIZE;

  // Find the end of central directory record:
  unsigned char *b = (unsigned char *) dwin->bind (fsize - sz, sz);
  if (b == NULL)
    {
      append_msg (CMSG_ERROR, GTXT ("%s: cannot find the central directory record (fsize=%lld)"),
		  name, (long long) fsize);
      return 0;
    }

  //  End of central directory record:
  //  Offset Bytes    Description
  //     0     4    end of central directory signature = 0x06054b50
  //     4     2    number of this disk
  //     6     2    disk where central directory starts
  //     8     2    number of central directory records on this disk
  //    10     2    total number of central directory records
  //    12     4    size of central directory(bytes)
  //    16     4    offset of start of central directory, relative to start of archive
  //    20     2    comment length(n)
  //    22     n    comment

  endCentDir->count = 0;
  endCentDir->size = 0;
  endCentDir->offset = 0;
  int64_t ecdrOffset = fsize;
  for (int64_t i = END_CENT_DIR_SIZE; i < sz; i++)
    {
      b = (unsigned char *) dwin->bind (fsize - i, END_CENT_DIR_SIZE);
      if (b == NULL)
	{
	  append_msg (CMSG_ERROR, GTXT ("%s: read failed (offset:0x%llx  bytes:%lld"),
		      name, (long long) (fsize - i), (long long) END_CENT_DIR_SIZE);
	  break;
	}
      uint32_t signature = get_u4 (b);
      if (signature == 0x06054b50)
	{
	  int64_t len_comment = get_u2 (b + 20);
	  if (i != (len_comment + END_CENT_DIR_SIZE))
	    continue;
	  ecdrOffset = fsize - i;
	  endCentDir->count = get_u2 (b + 10);
	  endCentDir->size = get_u4 (b + 12);
	  endCentDir->offset = get_u4 (b + 16);
	  Dprintf (DUMP_JAR_FILE,
		   "  Zip archive file size:              %10lld (0x%016llx)\n"
		   "  end-cent-dir record offset:         %10lld (0x%016llx)\n"
		   "  cent-dir offset:                    %10lld (0x%016llx)\n"
		   "  cent-dir size:                      %10lld (0x%016llx)\n"
		   "  cent-dir entries:                   %10lld\n",
		   (long long) fsize, (long long) fsize,
		   (long long) ecdrOffset, (long long) ecdrOffset,
		   (long long) endCentDir->offset, (long long) endCentDir->offset,
		   (long long) endCentDir->size, (long long) endCentDir->size,
		   (long long) endCentDir->count);
	  break;
	}
    }
  if (ecdrOffset == fsize)
    {
      append_msg (CMSG_ERROR,
		  GTXT ("%s: cannot find the central directory record"), name);
      return 0;
    }
  if (endCentDir->count == 0xffff || endCentDir->offset == 0xffffffff
      || endCentDir->size == 0xffffffff)
    {
      // Zip64 format:
      //      Zip64 end of central directory record
      //      Zip64 end of central directory locator  ( Can be absent )
      //      End of central directory record
      b = (unsigned char *) dwin->bind (ecdrOffset - ZIP64_LOCATOR_SIZE,
					ZIP64_LOCATOR_SIZE);
      if (b == NULL)
	{
	  append_msg (CMSG_ERROR,
	     GTXT ("%s: cannot find the Zip64 central directory record"), name);
	  return 0;
	}
      uint32_t signature = get_u4 (b);
      if (signature == 0x07064b50)
	{ // Get an offset from the Zip64 cent-dir locator
	  //  Zip64 end of central directory locator
	  //  Offset Bytes    Description
	  //     0     4    Zip64 end of central dir locator signature = 0x07064b50
	  //     4     4    number of the disk with the start of the zip64 end of central directory
	  //     8     8    relative offset of the Zip64 end of central directory record
	  //    12     4    total number of disks
	  Dprintf (DUMP_JAR_FILE, "    cent-dir locator offset           %10lld (0x%016llx)\n",
		   (long long) (ecdrOffset - ZIP64_LOCATOR_SIZE), (long long) (ecdrOffset - ZIP64_LOCATOR_SIZE));
	  ecdrOffset = get_u8 (b + 8);
	}
      else   // the Zip64 end of central directory locator is absent
	ecdrOffset -= ZIP64_CENT_DIR_SIZE;
      Dprintf (DUMP_JAR_FILE, NTXT ("  Zip64 end-cent-dir record offset:   %10lld (0x%016llx)\n"),
	       (long long) ecdrOffset, (long long) ecdrOffset);

      b = (unsigned char *) dwin->bind (ecdrOffset, ZIP64_CENT_DIR_SIZE);
      if (b == NULL)
	{
	  append_msg (CMSG_ERROR,
	     GTXT ("%s: cannot find the Zip64 central directory record"), name);
	  return 0;
	}
      //  Zip64 end of central directory record
      //  Offset Bytes    Description
      //     0     4    Zip64 end of central dir signature = 0x06064b50
      //     4     8    size of zip64 end of central directory record
      //    12     2    version made by
      //    14     2    version needed to extract
      //    16     4    number of this disk
      //    20     4    number of the disk with the start of the central directory
      //    24     8    total number of entries in the central directory on this disk
      //    32     8    total number of entries in the central directory
      //    40     8    size of the central directory
      //    48     8    offset of start of centraldirectory with respect to the starting disk number
      //    56          Zip64 extensible data sector (variable size)
      signature = get_u4 (b);
      if (signature != 0x06064b50)
	{
	  append_msg (CMSG_ERROR, GTXT ("%s: cannot find the Zip64 central directory record"), name);
	  return 0;
	}
      endCentDir->count = get_u8 (b + 32);
      endCentDir->size = get_u8 (b + 40);
      endCentDir->offset = get_u8 (b + 48);
      Dprintf (DUMP_JAR_FILE,
	       NTXT ("  cent-dir offset:                    %10lld (0x%016llx)\n"
		     "  cent-dir size:                      %10lld (0x%016llx)\n"
		     "  cent-dir entries:                   %10lld\n"),
	       (long long) endCentDir->offset, (long long) endCentDir->offset,
	       (long long) endCentDir->size, (long long) endCentDir->size,
	       (long long) endCentDir->count);
    }
  return 1;
}

