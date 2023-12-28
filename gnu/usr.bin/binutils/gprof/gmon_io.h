/* gmon_io.h

   Copyright (C) 2000-2023 Free Software Foundation, Inc.

   This file is part of GNU Binutils.

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

#ifndef gmon_io_h
#define gmon_io_h

#define INPUT_HISTOGRAM		(1 << 0)
#define INPUT_CALL_GRAPH	(1 << 1)
#define INPUT_BB_COUNTS		(1 << 2)

extern int gmon_input;		/* What input did we see?  */
extern int gmon_file_version;	/* File version are we dealing with.  */

extern int gmon_io_read_vma (FILE *ifp, bfd_vma *valp);
extern int gmon_io_read_32 (FILE *ifp, unsigned int *valp);
extern int gmon_io_read (FILE *ifp, char *buf, size_t n);
extern int gmon_io_write_vma (FILE *ifp, bfd_vma val);
extern int gmon_io_write_32 (FILE *ifp, unsigned int val);
extern int gmon_io_write_8 (FILE *ifp, unsigned int val);
extern int gmon_io_write (FILE *ifp, char *buf, size_t n);

extern void gmon_out_read   (const char *);
extern void gmon_out_write  (const char *);

#endif /* gmon_io_h */
