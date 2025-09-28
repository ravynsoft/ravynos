/* BFD support for the Lattice Mico32 architecture.
   Copyright (C) 2008-2023 Free Software Foundation, Inc.
   Contributed by Jon Beniston <jon@beniston.com>

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

const bfd_arch_info_type bfd_lm32_arch =
{
  32,			  /* Bits in word.  */
  32,			  /* Bits in address.  */
  8,			  /* Bits in byte.  */
  bfd_arch_lm32,	  /* Enum bfd_architecture.  */
  bfd_mach_lm32,	  /* Machine number.  */
  "lm32",		  /* Architecture name.  */
  "lm32",		  /* Printable name.  */
  4,			  /* Alignment.  */
  true,			  /* Is this the default machine for the target.  */
  bfd_default_compatible, /* Function callback to test if two files have compatible machines.  */
  bfd_default_scan,
  bfd_arch_default_fill,
  NULL,			  /* Next.  */
  0 			  /* Maximum offset of a reloc from the start of an insn.  */
};
