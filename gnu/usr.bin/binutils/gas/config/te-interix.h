/* Copyright (C) 2007-2023 Free Software Foundation, Inc.

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

#define TE_PE_DYN /* PE with dynamic linking (UNIX shared lib) support */
#define TE_PE
#define LEX_AT 1 /* can have @'s inside labels */
#define LEX_QM 3 /* can have ?'s in or begin labels */

/* The PE format supports long section names.  */
#define COFF_LONG_SECTION_NAMES

#define GLOBAL_OFFSET_TABLE_NAME "__GLOBAL_OFFSET_TABLE_"

/* Both architectures use these.  */
#ifndef LOCAL_LABELS_FB
#define LOCAL_LABELS_FB 1
#endif

#include "obj-format.h"
