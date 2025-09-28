/* Copyright (C) 2009-2023 Free Software Foundation, Inc.

   This file is part of GAS, the GNU Assembler.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 3,
   or (at your option) any later version.

   GAS is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
   the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GAS; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

#define TE_VMS
#include "obj-format.h"

extern uint64_t vms_dwarf2_file_time_name (const char *, const char *);
extern long vms_dwarf2_file_size_name (const char *, const char *);
extern char *vms_dwarf2_file_name (const char *, const char *);

/* VMS debugger expects a separator.  */
#define DWARF2_DIR_SHOULD_END_WITH_SEPARATOR 1

/* VMS debugger needs the file timestamp.  */
#define DWARF2_FILE_TIME_NAME(FILENAME,DIRNAME)                       \
  vms_dwarf2_file_time_name(FILENAME, DIRNAME)

/* VMS debugger needs the file size.  */
#define DWARF2_FILE_SIZE_NAME(FILENAME,DIRNAME)                       \
  vms_dwarf2_file_size_name(FILENAME, DIRNAME)

/* VMS debugger needs the filename with version appended.  */
/* Longest filename on VMS is 255 characters. Largest version is 32768.  */
#define DWARF2_FILE_NAME(FILENAME,DIRNAME)                            \
  vms_dwarf2_file_name(FILENAME, DIRNAME)
