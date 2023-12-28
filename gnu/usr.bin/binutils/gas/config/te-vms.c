/* te-vms.c -- Utilities for VMS.
   Copyright (C) 2009-2023 Free Software Foundation, Inc.

   Written by Douglas B Rupp <rupp@gnat.com>

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
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.  */

#include "as.h"
#include "te-vms.h"

/* The purpose of the two alternate versions below is to have one that
   works for native VMS and one that works on an NFS mounted filesystem
   (Unix Server/VMS client).  The main issue being to generate the special
   VMS file timestamps for the debug info.  */

#ifdef VMS
#define __NEW_STARLET 1
#include <vms/starlet.h>
#include <vms/rms.h>
#include <vms/atrdef.h>
#include <vms/fibdef.h>
#include <vms/stsdef.h>
#include <vms/iodef.h>
#include <vms/fatdef.h>
#include <errno.h>
#include <vms/descrip.h>
#include <string.h>
#include <unixlib.h>

#define MAXPATH 256

/* Descrip.h doesn't have everything...  */
typedef struct fibdef * __fibdef_ptr32 __attribute__ (( mode (SI) ));

struct dsc$descriptor_fib
{
  unsigned int   fib$l_len;
  __fibdef_ptr32 fib$l_addr;
};

/* I/O Status Block.  */
struct IOSB
{
  unsigned short status, count;
  unsigned int devdep;
};

static char *tryfile;

/* Variable length string.  */
struct vstring
{
  short length;
  char string[NAM$C_MAXRSS+1];
};

static char filename_buff [MAXPATH];
static char vms_filespec [MAXPATH];

/* Callback function for filespec style conversion.  */

static int
translate_unix (char *name, int type ATTRIBUTE_UNUSED)
{
  strncpy (filename_buff, name, MAXPATH);
  filename_buff [MAXPATH - 1] = (char) 0;
  return 0;
}

/* Wrapper for DECC function that converts a Unix filespec
   to VMS style filespec.  */

static char *
to_vms_file_spec (char *filespec)
{
  strncpy (vms_filespec, "", MAXPATH);
  decc$to_vms (filespec, translate_unix, 1, 1);
  strncpy (vms_filespec, filename_buff, MAXPATH);

  vms_filespec [MAXPATH - 1] = (char) 0;

  return vms_filespec;
}

#else /* not VMS */

#define _BSD_SOURCE 1
#include <sys/stat.h>
#include <time.h>

#define VMS_EPOCH_OFFSET        35067168000000000LL
#define VMS_GRANULARITY_FACTOR  10000000

#endif /* VMS */

/* Return VMS file date, size, format, version given a name.  */

static int
vms_file_stats_name (const char *dirname,
                     const char *filename,
		     long long *cdt,
		     long *siz,
		     char *rfo,
		     int *ver)
{
  char * fullname;

#ifdef VMS
  struct FAB fab;
  struct NAM nam;

  unsigned long long create;
  FAT recattr;
  char ascnamebuff [256];

  ATRDEF atrlst[]
    = {
      { ATR$S_CREDATE,  ATR$C_CREDATE,  &create },
      { ATR$S_RECATTR,  ATR$C_RECATTR,  &recattr },
      { ATR$S_ASCNAME,  ATR$C_ASCNAME,  &ascnamebuff },
      { 0, 0, 0}
    };

  FIBDEF fib;
  struct dsc$descriptor_fib fibdsc = {sizeof (fib), (void *) &fib};

  struct IOSB iosb;

  long status;
  unsigned short chan;

  struct vstring file;
  struct dsc$descriptor_s filedsc
    = {NAM$C_MAXRSS, DSC$K_DTYPE_T, DSC$K_CLASS_S, (void *) file.string};
  struct vstring device;
  struct dsc$descriptor_s devicedsc
    = {NAM$C_MAXRSS, DSC$K_DTYPE_T, DSC$K_CLASS_S, (void *) device.string};
  struct vstring result;
  struct dsc$descriptor_s resultdsc
    = {NAM$C_MAXRSS, DSC$K_DTYPE_VT, DSC$K_CLASS_VS, (void *) result.string};

  if (strcmp (filename, "<internal>") == 0
      || strcmp (filename, "<built-in>") == 0)
    {
      if (cdt)
	*cdt = 0;

      if (siz)
	*siz = 0;

      if (rfo)
	*rfo = 0;

      if (ver)
        *ver = 0;

      return 0;
    }

  fullname = concat (dirname, filename, NULL);
  tryfile = to_vms_file_spec (fullname);

  /* Allocate and initialize a FAB and NAM structures.  */
  fab = cc$rms_fab;
  nam = cc$rms_nam;

  nam.nam$l_esa = file.string;
  nam.nam$b_ess = NAM$C_MAXRSS;
  nam.nam$l_rsa = result.string;
  nam.nam$b_rss = NAM$C_MAXRSS;
  fab.fab$l_fna = tryfile;
  fab.fab$b_fns = strlen (tryfile);
  fab.fab$l_nam = &nam;

  /* Validate filespec syntax and device existence.  */
  status = SYS$PARSE (&fab, 0, 0);
  if ((status & 1) != 1)
    {
      free (fullname);
      return 1;
    }

  file.string[nam.nam$b_esl] = 0;

  /* Find matching filespec.  */
  status = SYS$SEARCH (&fab, 0, 0);
  if ((status & 1) != 1)
    {
      free (fullname);
      return 1;
    }

  file.string[nam.nam$b_esl] = 0;
  result.string[result.length=nam.nam$b_rsl] = 0;

  /* Get the device name and assign an IO channel.  */
  strncpy (device.string, nam.nam$l_dev, nam.nam$b_dev);
  devicedsc.dsc$w_length  = nam.nam$b_dev;
  chan = 0;
  status = SYS$ASSIGN (&devicedsc, &chan, 0, 0, 0);
  if ((status & 1) != 1)
    {
      free (fullname);
      return 1;
    }

  /* Initialize the FIB and fill in the directory id field.  */
  memset (&fib, 0, sizeof (fib));
  fib.fib$w_did[0]  = nam.nam$w_did[0];
  fib.fib$w_did[1]  = nam.nam$w_did[1];
  fib.fib$w_did[2]  = nam.nam$w_did[2];
  fib.fib$l_acctl = 0;
  fib.fib$l_wcc = 0;
  strcpy (file.string, (strrchr (result.string, ']') + 1));
  filedsc.dsc$w_length = strlen (file.string);
  result.string[result.length = 0] = 0;

  /* Open and close the file to fill in the attributes.  */
  status
    = SYS$QIOW (0, chan, IO$_ACCESS|IO$M_ACCESS, &iosb, 0, 0,
		&fibdsc, &filedsc, &result.length, &resultdsc, &atrlst, 0);
  if ((status & 1) != 1)
    {
      free (fullname);
      return 1;
    }

  if ((iosb.status & 1) != 1)
    {
      free (fullname);
      return 1;
    }

  result.string[result.length] = 0;
  status = SYS$QIOW (0, chan, IO$_DEACCESS, &iosb, 0, 0, &fibdsc, 0, 0, 0,
		     &atrlst, 0);
  if ((status & 1) != 1)
    {
      free (fullname);
      return 1;
    }

  if ((iosb.status & 1) != 1)
    {
      free (fullname);
      return 1;
    }

  /* Deassign the channel and exit.  */
  status = SYS$DASSGN (chan);
  if ((status & 1) != 1)
    {
      free (fullname);
      return 1;
    }

  if (cdt) *cdt = create;
  if (siz) *siz = (512 * 65536 * recattr.fat$w_efblkh) +
                  (512 * (recattr.fat$w_efblkl - 1)) +
                  recattr.fat$w_ffbyte;
  if (rfo) *rfo = recattr.fat$v_rtype;
  if (ver) *ver = strtol (strrchr (ascnamebuff, ';') + 1, 0, 10);
#else /* not VMS */

  struct stat buff;
  struct tm *ts;
  long long gmtoff, secs, nsecs;

  fullname = concat (dirname, filename, NULL);

  if ((stat (fullname, &buff)) != 0)
    {
      free (fullname);
      return 1;
    }

  if (cdt)
    {
      ts = localtime (& buff.st_mtime);

#ifdef HAVE_TM_GMTOFF
	gmtoff = ts->tm_gmtoff;
#else
	{
	  extern long timezone;

	  if (ts->tm_isdst == 1)
	    gmtoff = - (timezone - 3600);
	  else
	    gmtoff = - timezone;
	}
#endif

#ifdef HAVE_ST_MTIM_TV_SEC
      secs = buff.st_mtim.tv_sec;
#else
      secs = buff.st_mtime;
#endif

#ifdef HAVE_ST_MTIM_TV_NSEC
      nsecs = buff.st_mtim.tv_nsec;
#else
      nsecs = 0;
#endif

      /* VMS timestamps are stored in local time to 100 nsec accuracy, but by
	 experiment I found timestamps truncated to (at least) microseconds
	 on an NFS mounted filesystem, hence the adjustment below. DBR. */
      *cdt = ((secs + gmtoff) * VMS_GRANULARITY_FACTOR)
	+ (nsecs / 1000 * 10) + VMS_EPOCH_OFFSET;
    }

  if (siz)
    *siz = buff.st_size;

  if (rfo)
    *rfo = 2; /* Stream LF format.  */

  /* Returning a file version of 0 is never correct for debug info, version 1
     will be correct if file editing is done only on the Unix side.  If editing
     is done on the VMS side, then its TBD.  */
  if (ver)
    *ver = 1;
#endif /* VMS */

  free (fullname);
  return 0;
}

uint64_t
vms_dwarf2_file_time_name (const char *filename, const char *dirname)
{
  long long cdt;

  if (vms_file_stats_name (dirname, filename, &cdt, 0, 0, 0) == 0)
    return cdt;
  else
    return 0;
}

long
vms_dwarf2_file_size_name (const char *filename, const char *dirname)
{
  long siz;

  if (vms_file_stats_name (dirname, filename, 0, &siz, 0, 0) == 0)
    return siz;
  else
    return 0;
}

/* VMS debugger needs the filename with version appended.  */
/* Longest filename on VMS is 255 characters. Largest version is 32768.  */
char *
vms_dwarf2_file_name (const char *filename, const char *dirname)
{
  int ver;
  static char buff [255 + 7];

  vms_file_stats_name (dirname, filename, 0, 0, 0, &ver);
  snprintf (buff, 255 + 7, "%s;%d", filename, ver);
  return buff;
}
