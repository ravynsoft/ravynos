/* Copyright (C) 2008-2023 Free Software Foundation, Inc.

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

#define TE_SOLARIS

#define LOCAL_LABELS_DOLLAR 1
#define LOCAL_LABELS_FB 1

#include "obj-format.h"

/* The Sun linker doesn't merge read-only and read-write sections into
   a single section so we must force all EH frame sections to use the
   same flags.  For SPARC and 32-bit i386 this is read-write, whilst
   for x86_64 this is read-only, matching GCC behavior.

   See the definition of EH_TABLES_CAN_BE_READ_ONLY in
   gcc/config/i386/sol2.h in the GCC sources and the thread starting at
   http://sourceware.org/ml/binutils/2010-01/msg00401.html.  */
#ifdef TC_SPARC
#define DWARF2_EH_FRAME_READ_ONLY SEC_NO_FLAGS
#else
#define DWARF2_EH_FRAME_READ_ONLY \
  (bfd_get_arch_size (stdoutput) == 64 ? SEC_READONLY : SEC_NO_FLAGS)
#endif
