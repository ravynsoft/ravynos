/* BFD support for the NEC V850 processor
   Copyright (C) 1996-2023 Free Software Foundation, Inc.

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
#include "safe-ctype.h"

#define N(number, print, default, next)  \
 { 32, 32, 8, bfd_arch_v850, number, "v850", print ":old-gcc-abi", 2, default, \
   bfd_default_compatible, bfd_default_scan, bfd_arch_default_fill, next, 0 }

static const bfd_arch_info_type arch_info_struct[6] =
{
  N (bfd_mach_v850e3v5, "v850e3v5", false, arch_info_struct + 1),
  N (bfd_mach_v850e3v5, "v850e2v4", false, arch_info_struct + 2),
  N (bfd_mach_v850e2v3, "v850e2v3", false, arch_info_struct + 3),
  N (bfd_mach_v850e2,	"v850e2",   false, arch_info_struct + 4),
  N (bfd_mach_v850e1,	"v850e1",   false, arch_info_struct + 5),
  N (bfd_mach_v850e,	"v850e",    false, NULL)
};

const bfd_arch_info_type bfd_v850_arch =
  N (bfd_mach_v850,     "v850",     true,  arch_info_struct + 0);
