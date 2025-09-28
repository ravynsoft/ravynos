/* BFD support for the Imagination Technologies Meta processor.
   Copyright (C) 2013-2023 Free Software Foundation, Inc.
   Contributed by Imagination Technologies Ltd.

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

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"

const bfd_arch_info_type bfd_metag_arch =
{
  32,				/* Bits per word.  */
  32,				/* Bits per address.  */
  8,				/* Bits per byte.  */
  bfd_arch_metag,		/* Architecture.  */
  bfd_mach_metag,		/* Machine.  */
  "metag",			/* Architecture name.  */
  "metag",			/* Printable name.  */
  4,				/* Section align power.  */
  true,				/* The default ?  */
  bfd_default_compatible,	/* Architecture comparison fn.  */
  bfd_default_scan,		/* String to architecture convert fn.  */
  bfd_arch_default_fill,	/* Default fill.  */
  NULL,				/* Next in list.  */
  0 /* Maximum offset of a reloc from the start of an insn.  */
};
