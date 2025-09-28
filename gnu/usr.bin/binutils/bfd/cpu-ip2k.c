/* BFD support for the Scenix IP2xxx processor.
   Copyright (C) 2000-2023 Free Software Foundation, Inc.

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

#define N(NUMBER, PRINT, DEFAULT, NEXT)			\
  {							\
    32,         /* Bits in a word.  */			\
    16,         /* Bits in an address.  */		\
    8,	        /* Bits in a byte.  */			\
    bfd_arch_ip2k,					\
    NUMBER,						\
    "ip2k",						\
    PRINT,						\
    1,		/* Section alignment power.  */		\
    DEFAULT,						\
    bfd_default_compatible,				\
    bfd_default_scan,					\
    bfd_arch_default_fill,				\
    NEXT,						\
    0 /* Maximum offset of a reloc from the start of an insn.  */ \
  }

const bfd_arch_info_type bfd_ip2k_nonext_arch =
  N (bfd_mach_ip2022, "ip2022", false, NULL);

const bfd_arch_info_type bfd_ip2k_arch =
  N (bfd_mach_ip2022ext, "ip2022ext", true, & bfd_ip2k_nonext_arch);

