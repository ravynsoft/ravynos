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

#define TE_GO32

#define LOCAL_LABELS_DOLLAR 1
#define LOCAL_LABELS_FB 1

/* GAS should treat '.align value' as an alignment of 2**value.  */
#define USE_ALIGN_PTWO

#define COFF_LONG_SECTION_NAMES

/* These define interfaces.  */
#include "obj-format.h"
