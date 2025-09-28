/* vms-misc.c -- BFD back-end for VMS/VAX (openVMS/VAX) and
   EVAX (openVMS/Alpha) files.
   Copyright (C) 1996-2023 Free Software Foundation, Inc.

   Miscellaneous functions.

   Written by Klaus K"ampf (kkaempf@rmi.de)

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

#if __STDC__
#include <stdarg.h>
#endif

#include "sysdep.h"
#include "bfd.h"
#include "bfdlink.h"
#include "libbfd.h"
#include "safe-ctype.h"

#ifdef VMS
#define __NEW_STARLET
#include <rms.h>
#include <unixlib.h>
#include <gen64def.h>
#include <starlet.h>
#define RME$C_SETRFM 0x00000001
#include <unistd.h>
#endif
#include <time.h>

#include "vms.h"
#include "vms/emh.h"

#if VMS_DEBUG
/* Debug functions.  */

/* Debug function for all vms extensions evaluates environment
   variable VMS_DEBUG for a numerical value on the first call all
   error levels below this value are printed:

   Levels:
   1	toplevel bfd calls (functions from the bfd vector)
   2	functions called by bfd calls
   ...
   9	almost everything

   Level is also indentation level. Indentation is performed
   if level > 0.  */

void
_bfd_vms_debug (int level, char *format, ...)
{
  static int min_level = -1;
  static FILE *output = NULL;
  char *eptr;
  va_list args;
  int abslvl = (level > 0) ? level : - level;

  if (min_level == -1)
    {
      if ((eptr = getenv ("VMS_DEBUG")) != NULL)
	{
	  min_level = atoi (eptr);
	  output = stderr;
	}
      else
	min_level = 0;
    }
  if (output == NULL)
    return;
  if (abslvl > min_level)
    return;

  while (--level > 0)
    fprintf (output, " ");
  va_start (args, format);
  vfprintf (output, format, args);
  fflush (output);
  va_end (args);
}

/* A debug function
   hex dump 'size' bytes starting at 'ptr'.  */

void
_bfd_hexdump (int level, unsigned char *ptr, int size, int offset)
{
  unsigned char *lptr = ptr;
  int count = 0;
  long start = offset;

  while (size-- > 0)
    {
      if ((count % 16) == 0)
	vms_debug (level, "%08lx:", start);
      vms_debug (-level, " %02x", *ptr++);
      count++;
      start++;
      if (size == 0)
	{
	  while ((count % 16) != 0)
	    {
	      vms_debug (-level, "   ");
	      count++;
	    }
	}
      if ((count % 16) == 0)
	{
	  vms_debug (-level, " ");
	  while (lptr < ptr)
	    {
	      vms_debug (-level, "%c", (*lptr < 32) ? '.' : *lptr);
	      lptr++;
	    }
	  vms_debug (-level, "\n");
	}
    }
  if ((count % 16) != 0)
    vms_debug (-level, "\n");
}
#endif


/* Copy sized string (string with fixed size) to new allocated area.
   Size is string size (size of record).  */

char *
_bfd_vms_save_sized_string (bfd *abfd, unsigned char *str, size_t size)
{
  char *newstr;

  if (size == (size_t) -1)
    {
      bfd_set_error (bfd_error_no_memory);
      return NULL;
    }
  newstr = bfd_alloc (abfd, size + 1);
  if (newstr == NULL)
    return NULL;
  memcpy (newstr, str, size);
  newstr[size] = 0;

  return newstr;
}

/* Copy counted string (string with size at first byte) to new allocated area.
   PTR points to size byte on entry.  */

char *
_bfd_vms_save_counted_string (bfd *abfd, unsigned char *ptr, size_t maxlen)
{
  unsigned int len;

  if (maxlen == 0)
    return NULL;
  len = *ptr++;
  if (len >  maxlen - 1)
    return NULL;
  return _bfd_vms_save_sized_string (abfd, ptr, len);
}

/* Object output routines.   */

/* Begin new record.
   Write 2 bytes rectype and 2 bytes record length.  */

void
_bfd_vms_output_begin (struct vms_rec_wr *recwr, int rectype)
{
  vms_debug2 ((6, "_bfd_vms_output_begin (type %d)\n", rectype));

  /* Record must have been closed.  */
  BFD_ASSERT (recwr->size == 0);

  _bfd_vms_output_short (recwr, (unsigned int) rectype);

  /* Placeholder for length.  */
  _bfd_vms_output_short (recwr, 0);
}

/* Begin new sub-record.
   Write 2 bytes rectype, and 2 bytes record length.  */

void
_bfd_vms_output_begin_subrec (struct vms_rec_wr *recwr, int rectype)
{
  vms_debug2 ((6, "_bfd_vms_output_begin_subrec (type %d)\n", rectype));

  /* Subrecord must have been closed.  */
  BFD_ASSERT (recwr->subrec_offset == 0);

  /* Save start of subrecord offset.  */
  recwr->subrec_offset = recwr->size;

  /* Subrecord type.  */
  _bfd_vms_output_short (recwr, (unsigned int) rectype);

  /* Placeholder for length.  */
  _bfd_vms_output_short (recwr, 0);
}

/* Set record/subrecord alignment.   */

void
_bfd_vms_output_alignment (struct vms_rec_wr *recwr, int alignto)
{
  vms_debug2 ((6, "_bfd_vms_output_alignment (%d)\n", alignto));
  recwr->align = alignto;
}

/* Align the size of the current record (whose length is LENGTH).
   Warning: this obviously changes the record (and the possible subrecord)
   length.  */

static void
_bfd_vms_output_align (struct vms_rec_wr *recwr, unsigned int length)
{
  unsigned int real_size = recwr->size;
  unsigned int aligncount;

  /* Pad with 0 if alignment is required.  */
  aligncount = (recwr->align - (length % recwr->align)) % recwr->align;
  vms_debug2 ((6, "align: adding %d bytes\n", aligncount));
  while (aligncount-- > 0)
    recwr->buf[real_size++] = 0;

  recwr->size = real_size;
}

/* Ends current sub-record.  Set length field.  */

void
_bfd_vms_output_end_subrec (struct vms_rec_wr *recwr)
{
  int real_size = recwr->size;
  int length;

  /* Subrecord must be open.  */
  BFD_ASSERT (recwr->subrec_offset != 0);

  length = real_size - recwr->subrec_offset;

  if (length == 0)
    return;

  _bfd_vms_output_align (recwr, length);

  /* Put length to buffer.  */
  bfd_putl16 ((bfd_vma) (recwr->size - recwr->subrec_offset),
	      recwr->buf + recwr->subrec_offset + 2);

  /* Close the subrecord.  */
  recwr->subrec_offset = 0;
}

/* Ends current record (and write it).  */

void
_bfd_vms_output_end (bfd *abfd, struct vms_rec_wr *recwr)
{
  vms_debug2 ((6, "_bfd_vms_output_end (size %u)\n", recwr->size));

  /* Subrecord must have been closed.  */
  BFD_ASSERT (recwr->subrec_offset == 0);

  if (recwr->size == 0)
    return;

  _bfd_vms_output_align (recwr, recwr->size);

  /* Write the length word.  */
  bfd_putl16 ((bfd_vma) recwr->size, recwr->buf + 2);

  /* File is open in undefined (UDF) format on VMS, but ultimately will be
     converted to variable length (VAR) format.  VAR format has a length
     word first which must be explicitly output in UDF format.  */
  /* So, first the length word.  */
  bfd_bwrite (recwr->buf + 2, 2, abfd);

  /* Align.  */
  if (recwr->size & 1)
    recwr->buf[recwr->size++] = 0;

  /* Then the record.  */
  bfd_bwrite (recwr->buf, (size_t) recwr->size, abfd);

  recwr->size = 0;
}

/* Check remaining buffer size.  Return what's left.  */

int
_bfd_vms_output_check (struct vms_rec_wr *recwr, int size)
{
  vms_debug2 ((6, "_bfd_vms_output_check (%d)\n", size));

  return (MAX_OUTREC_SIZE - (recwr->size + size + MIN_OUTREC_LUFT));
}

/* Output byte (8 bit) value.  */

void
_bfd_vms_output_byte (struct vms_rec_wr *recwr, unsigned int value)
{
  vms_debug2 ((6, "_bfd_vms_output_byte (%02x)\n", value));

  *(recwr->buf + recwr->size) = value;
  recwr->size += 1;
}

/* Output short (16 bit) value.  */

void
_bfd_vms_output_short (struct vms_rec_wr *recwr, unsigned int value)
{
  vms_debug2 ((6, "_bfd_vms_output_short (%04x)\n", value));

  bfd_putl16 ((bfd_vma) value & 0xffff, recwr->buf + recwr->size);
  recwr->size += 2;
}

/* Output long (32 bit) value.  */

void
_bfd_vms_output_long (struct vms_rec_wr *recwr, unsigned long value)
{
  vms_debug2 ((6, "_bfd_vms_output_long (%08lx)\n", value));

  bfd_putl32 ((bfd_vma) value, recwr->buf + recwr->size);
  recwr->size += 4;
}

/* Output quad (64 bit) value.  */

void
_bfd_vms_output_quad (struct vms_rec_wr *recwr, bfd_vma value)
{
  vms_debug2 ((6, "_bfd_vms_output_quad (%08lx)\n", (unsigned long)value));

  bfd_putl64 (value, recwr->buf + recwr->size);
  recwr->size += 8;
}

/* Output c-string as counted string.  */

void
_bfd_vms_output_counted (struct vms_rec_wr *recwr, const char *value)
{
  int len;

  vms_debug2 ((6, "_bfd_vms_output_counted (%s)\n", value));

  len = strlen (value);
  if (len == 0)
    {
      _bfd_error_handler (_("_bfd_vms_output_counted called with zero bytes"));
      return;
    }
  if (len > 255)
    {
      _bfd_error_handler (_("_bfd_vms_output_counted called with too many bytes"));
      return;
    }
  _bfd_vms_output_byte (recwr, (unsigned int) len & 0xff);
  _bfd_vms_output_dump (recwr, (const unsigned char *)value, len);
}

/* Output character area.  */

void
_bfd_vms_output_dump (struct vms_rec_wr *recwr, const unsigned char *data, int len)
{
  vms_debug2 ((6, "_bfd_vms_output_dump (%d)\n", len));

  if (len == 0)
    return;

  memcpy (recwr->buf + recwr->size, data, (size_t) len);
  recwr->size += len;
}

/* Output count bytes of value.  */

void
_bfd_vms_output_fill (struct vms_rec_wr *recwr, int value, int count)
{
  vms_debug2 ((6, "_bfd_vms_output_fill (val %02x times %d)\n", value, count));

  if (count == 0)
    return;
  memset (recwr->buf + recwr->size, value, (size_t) count);
  recwr->size += count;
}

#ifdef VMS
/* Convert the file to variable record length format. This is done
   using undocumented system call sys$modify().
   Pure VMS version.  */

static void
vms_convert_to_var (char * vms_filename)
{
  struct FAB fab = cc$rms_fab;

  fab.fab$l_fna = vms_filename;
  fab.fab$b_fns = strlen (vms_filename);
  fab.fab$b_fac = FAB$M_PUT;
  fab.fab$l_fop = FAB$M_ESC;
  fab.fab$l_ctx = RME$C_SETRFM;

  sys$open (&fab);

  fab.fab$b_rfm = FAB$C_VAR;

  sys$modify (&fab);
  sys$close (&fab);
}

static int
vms_convert_to_var_1 (char *filename, int type)
{
  if (type != DECC$K_FILE)
    return false;
  vms_convert_to_var (filename);
  return true;
}

/* Convert the file to variable record length format. This is done
   using undocumented system call sys$modify().
   Unix filename version.  */

int
_bfd_vms_convert_to_var_unix_filename (const char *unix_filename)
{
  if (decc$to_vms (unix_filename, &vms_convert_to_var_1, 0, 1) != 1)
    return false;
  return true;
}
#endif /* VMS */

/* Manufacture a VMS like time on a unix based system.
   stolen from obj-vms.c.  */

unsigned char *
get_vms_time_string (unsigned char *tbuf)
{
#ifndef VMS
  char *pnt;
  time_t timeb;

  time (& timeb);
  pnt = ctime (&timeb);
  pnt[3] = 0;
  pnt[7] = 0;
  pnt[10] = 0;
  pnt[16] = 0;
  pnt[24] = 0;
  sprintf ((char *) tbuf, "%2s-%3s-%s %s",
	   pnt + 8, pnt + 4, pnt + 20, pnt + 11);
#else
  struct
  {
    int Size;
    unsigned char *Ptr;
  } Descriptor;
  Descriptor.Size = 17;
  Descriptor.Ptr = tbuf;
  SYS$ASCTIM (0, &Descriptor, 0, 0);
#endif /* not VMS */

  vms_debug2 ((6, "vmstimestring:'%s'\n", tbuf));

  return tbuf;
}

/* Create module name from filename (ie, extract the basename and convert it
   in upper cases).  Works on both VMS and UNIX pathes.
   The result has to be free().  */

char *
vms_get_module_name (const char *filename, bool upcase)
{
  char *fname, *fptr;
  const char *fout;

  /* Strip VMS path.  */
  fout = strrchr (filename, ']');
  if (fout == NULL)
    fout = strchr (filename, ':');
  if (fout != NULL)
    fout++;
  else
    fout = filename;

  /* Strip UNIX path.  */
  fptr = strrchr (fout, '/');
  if (fptr != NULL)
    fout = fptr + 1;

  fname = strdup (fout);

  /* Strip suffix.  */
  fptr = strrchr (fname, '.');
  if (fptr != 0)
    *fptr = 0;

  /* Convert to upper case and truncate at 31 characters.
     (VMS object file format restricts module name length to 31).  */
  fptr = fname;
  for (fptr = fname; *fptr != 0; fptr++)
    {
      if (*fptr == ';' || (fptr - fname) >= 31)
	{
	  *fptr = 0;
	  break;
	}
      if (upcase)
	*fptr = TOUPPER (*fptr);
    }
  return fname;
}

/* Compared to usual UNIX time_t, VMS time has less limits:
   -  64 bit (63 bits in fact as the MSB must be 0)
   -  100ns granularity
   -  epoch is Nov 17, 1858.
   Here has the constants and the routines used to convert VMS from/to UNIX time.
   The conversion routines don't assume 64 bits arithmetic.

   Here we assume that the definition of time_t is the UNIX one, ie integer
   type, expressing seconds since the epoch.  */

/* UNIX time granularity for VMS, ie 1s / 100ns.  */
#define VMS_TIME_FACTOR 10000000

/* Number of seconds since VMS epoch of the UNIX epoch.  */
#define VMS_TIME_OFFSET 3506716800U

/* Convert a VMS time to a unix time.  */

time_t
vms_time_to_time_t (unsigned int hi, unsigned int lo)
{
  unsigned int tmp;
  unsigned int rlo;
  int i;
  time_t res;

  /* First convert to seconds.  */
  tmp = hi % VMS_TIME_FACTOR;
  hi = hi / VMS_TIME_FACTOR;
  rlo = 0;
  for (i = 0; i < 4; i++)
    {
      tmp = (tmp << 8) | (lo >> 24);
      lo <<= 8;

      rlo = (rlo << 8) | (tmp / VMS_TIME_FACTOR);
      tmp %= VMS_TIME_FACTOR;
    }
  lo = rlo;

  /* Return 0 in case of overflow.  */
  if (hi > 1
      || (hi == 1 && lo >= VMS_TIME_OFFSET))
    return 0;

  /* Return 0 in case of underflow.  */
  if (hi == 0 && lo < VMS_TIME_OFFSET)
    return 0;

  res = lo - VMS_TIME_OFFSET;
  if (res <= 0)
    return 0;
  return res;
}

/* Convert a time_t to a VMS time.  */

void
vms_time_t_to_vms_time (time_t ut, unsigned int *hi, unsigned int *lo)
{
  unsigned int val[4];
  unsigned int tmp[4];
  unsigned int carry;
  int i;

  /* Put into val.  */
  val[0] = ut & 0xffff;
  val[1] = (ut >> 16) & 0xffff;
  val[2] = sizeof (ut) > 4 ? (ut >> 32) & 0xffff : 0;
  val[3] = sizeof (ut) > 4 ? (ut >> 48) & 0xffff : 0;

  /* Add offset.  */
  tmp[0] = VMS_TIME_OFFSET & 0xffff;
  tmp[1] = VMS_TIME_OFFSET >> 16;
  tmp[2] = 0;
  tmp[3] = 0;
  carry = 0;
  for (i = 0; i < 4; i++)
    {
      carry += tmp[i] + val[i];
      val[i] = carry & 0xffff;
      carry = carry >> 16;
    }

  /* Multiply by factor, well first by 10000 and then by 1000.  */
  carry = 0;
  for (i = 0; i < 4; i++)
    {
      carry += val[i] * 10000;
      val[i] = carry & 0xffff;
      carry = carry >> 16;
    }
  carry = 0;
  for (i = 0; i < 4; i++)
    {
      carry += val[i] * 1000;
      val[i] = carry & 0xffff;
      carry = carry >> 16;
    }

  /* Write the result.  */
  *lo = val[0] | (val[1] << 16);
  *hi = val[2] | (val[3] << 16);
}

/* Convert a raw (stored in a buffer) VMS time to a unix time.  */

time_t
vms_rawtime_to_time_t (unsigned char *buf)
{
  unsigned int hi = bfd_getl32 (buf + 4);
  unsigned int lo = bfd_getl32 (buf + 0);

  return vms_time_to_time_t (hi, lo);
}

void
vms_get_time (unsigned int *hi, unsigned int *lo)
{
#ifdef VMS
  struct _generic_64 t;

  sys$gettim (&t);
  *lo = t.gen64$q_quadword;
  *hi = t.gen64$q_quadword >> 32;
#else
  time_t t;

  time (&t);
  vms_time_t_to_vms_time (t, hi, lo);
#endif
}

/* Get the current time into a raw buffer BUF.  */

void
vms_raw_get_time (unsigned char *buf)
{
  unsigned int hi, lo;

  vms_get_time (&hi, &lo);
  bfd_putl32 (lo, buf + 0);
  bfd_putl32 (hi, buf + 4);
}
