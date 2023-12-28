/* vms.h -- Header file for VMS (Alpha and Vax) support.
   Copyright (C) 1996-2023 Free Software Foundation, Inc.

   Main header file.

   Written by Klaus K"ampf (kkaempf@rmi.de)

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

#undef vms
#ifndef VMS_H
#define VMS_H

#include <time.h>

/* Size of a VMS block on disk.  */

#define VMS_BLOCK_SIZE  512

/* Miscellaneous definitions.  */

#define MAX_OUTREC_SIZE 4096
#define MIN_OUTREC_LUFT 64

/* File format.  */

enum file_format_enum
  {
    /* Not yet known.  */
    FF_UNKNOWN,

    /* Unix format.  Each record is preceeded by the record length,
       on 2 bytes.  */
    FF_FOREIGN,

    /* Native (=VMS) format.  The file only contains the content of the
       records.  This may also appear on Unix, depending on which tool
       was used to transfer files.  */
    FF_NATIVE
  };

/* VMS records input buffer.  */

struct vms_rec_rd
{
  /* Buffer and its size.  */
  unsigned char *buf;
  unsigned int buf_size;

  /* Current record and its size.  */
  unsigned char *rec;
  unsigned int rec_size;

  /* Input file format.  */
  enum file_format_enum file_format;
};

/* VMS records output buffer.  */

struct vms_rec_wr
{
  /* Output buffer.  */
  unsigned char *buf;

  /* Current length of the record.  */
  unsigned short int size;

  /* Sub-record start offset.  */
  unsigned short int subrec_offset;

  /* Some records must have a size that is a multiple of the alignment.
     Mustn't be 0.  */
  unsigned short int align;
};

struct evax_private_udata_struct
{
  asymbol *bsym;
  asymbol *enbsym;
  char *origname;
  int lkindex;
};

/* vms-misc.c.  */

#define VMS_DEBUG 0

#if VMS_DEBUG
extern void _bfd_vms_debug (int, char *, ...) ATTRIBUTE_PRINTF_2;
extern void _bfd_hexdump   (int, unsigned char *, int, int);

#define vms_debug _bfd_vms_debug
#define vms_debug2(X) _bfd_vms_debug X
#else
#define vms_debug2(X)
#endif

extern char * vms_get_module_name (const char *, bool);
extern unsigned char *get_vms_time_string (unsigned char *);
extern time_t vms_time_to_time_t (unsigned int hi, unsigned int lo);
extern time_t vms_rawtime_to_time_t (unsigned char *);
extern void vms_time_t_to_vms_time (time_t ut, unsigned int *hi, unsigned int *lo);
extern void vms_get_time (unsigned int *hi, unsigned int *lo);
extern void vms_raw_get_time (unsigned char *buf);

extern char * _bfd_vms_save_sized_string (bfd *, unsigned char *, size_t);
extern char * _bfd_vms_save_counted_string (bfd *, unsigned char *, size_t);
extern void   _bfd_vms_output_begin (struct vms_rec_wr *, int);
extern void   _bfd_vms_output_alignment (struct vms_rec_wr *, int);
extern void   _bfd_vms_output_begin_subrec (struct vms_rec_wr *, int);
extern void   _bfd_vms_output_end_subrec (struct vms_rec_wr *);
extern void   _bfd_vms_output_end (bfd *, struct vms_rec_wr *);
extern int    _bfd_vms_output_check (struct vms_rec_wr *, int);
extern void   _bfd_vms_output_byte (struct vms_rec_wr *, unsigned);
extern void   _bfd_vms_output_short (struct vms_rec_wr *, unsigned);
extern void   _bfd_vms_output_long (struct vms_rec_wr *, unsigned long);
extern void   _bfd_vms_output_quad (struct vms_rec_wr *, bfd_vma);
extern void   _bfd_vms_output_counted (struct vms_rec_wr *, const char *);
extern void   _bfd_vms_output_dump (struct vms_rec_wr *, const unsigned char *, int);
extern void   _bfd_vms_output_fill (struct vms_rec_wr *, int, int);
extern int    _bfd_vms_convert_to_var_unix_filename (const char *);

/* vms-alpha.c  */

extern void bfd_vms_set_section_flags (bfd *, asection *, flagword, flagword);

#endif /* VMS_H */
