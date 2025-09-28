/* BFD support for the Visium processor.

   Copyright (C) 2003-2023 Free Software Foundation, Inc.

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
   Foundation, Inc., 51 Franklin Street - Fifth Floor,
   Boston, MA 02110-1301, USA.  */

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"

const bfd_arch_info_type bfd_visium_arch =
{
  32,				/* Bits per word.  */
  32,				/* Bits per address.  */
  8,				/* Bits per byte.  */
  bfd_arch_visium,		/* Architecture number.  */
  bfd_mach_visium,		/* Machine number.  */
  "visium",			/* Architecture name.  */
  "visium",			/* Machine name.  */
  2,				/* Section align power.  */
  true,				/* The default ?  */
  bfd_default_compatible,	/* Architecture comparison fn.  */
  bfd_default_scan,		/* String to architecture convert fn.  */
  bfd_arch_default_fill,	/* Default fill.  */
  NULL,				/* Next in list.  */
  0 /* Maximum offset of a reloc from the start of an insn.  */
};
