/* BFD support for the NDS32 processor
   Copyright (C) 2012-2023 Free Software Foundation, Inc.
   Contributed by Andes Technology Corporation.

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
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"
#include "elf-bfd.h"

#define N(number, print, default, next)  \
    {32, 32, 8, bfd_arch_nds32, number, "nds32", print, 4, default, \
     bfd_default_compatible, bfd_default_scan, bfd_arch_default_fill, next, 0 }

#define NEXT		&arch_info_struct[0]
#define NDS32V2_NEXT	&arch_info_struct[1]
#define NDS32V3_NEXT	&arch_info_struct[2]
#define NDS32V3M_NEXT	&arch_info_struct[3]

static const bfd_arch_info_type arch_info_struct[] =
{
  N (bfd_mach_n1h, "n1h", false, NDS32V2_NEXT),
  N (bfd_mach_n1h_v2, "n1h_v2", false, NDS32V3_NEXT),
  N (bfd_mach_n1h_v3, "n1h_v3", false, NDS32V3M_NEXT),
  N (bfd_mach_n1h_v3m, "n1h_v3m", false, NULL),
};

const bfd_arch_info_type bfd_nds32_arch =
  N (bfd_mach_n1, "n1", true, NEXT);
