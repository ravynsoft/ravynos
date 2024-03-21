/* BFD support for the AMDGCN GPU architecture.

   Copyright (C) 2019-2023 Free Software Foundation, Inc.

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
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"

#define N(MACHINE, PRINTABLE_NAME, DEFAULT, NEXT)       \
  {                                                     \
    32, /* 32 bits in a word */                         \
    64, /* 64 bits in an address */                     \
    8,  /* 8 bits in a byte */                          \
    bfd_arch_amdgcn,                                    \
    MACHINE,                                            \
    "amdgcn",                                           \
    PRINTABLE_NAME,                                     \
    3, /* section align power */                        \
    DEFAULT,                                            \
    bfd_default_compatible,                             \
    bfd_default_scan,                                   \
    bfd_arch_default_fill,                              \
    NEXT,                                               \
    0                                                   \
  }

#define NN(index) (&arch_info_struct[index])

static const bfd_arch_info_type arch_info_struct[] =
{
  N (bfd_mach_amdgcn_gfx904, "amdgcn:gfx904", false, NN (1)),
  N (bfd_mach_amdgcn_gfx906, "amdgcn:gfx906", false, NN (2)),
  N (bfd_mach_amdgcn_gfx908, "amdgcn:gfx908", false, NN (3)),
  N (bfd_mach_amdgcn_gfx90a, "amdgcn:gfx90a", false, NN (4)),
  N (bfd_mach_amdgcn_gfx1010, "amdgcn:gfx1010", false, NN (5)),
  N (bfd_mach_amdgcn_gfx1011, "amdgcn:gfx1011", false, NN (6)),
  N (bfd_mach_amdgcn_gfx1012, "amdgcn:gfx1012", false, NN (7)),
  N (bfd_mach_amdgcn_gfx1030, "amdgcn:gfx1030", false, NN (8)),
  N (bfd_mach_amdgcn_gfx1031, "amdgcn:gfx1031", false, NN (9)),
  N (bfd_mach_amdgcn_gfx1032, "amdgcn:gfx1032", false, NULL)
};

const bfd_arch_info_type bfd_amdgcn_arch =
  N (bfd_mach_amdgcn_gfx900, "amdgcn:gfx900", true, NN (0));
