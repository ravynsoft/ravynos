/* BFD library support routines for the NFP.
   Copyright (C) 2017-2023 Free Software Foundation, Inc.
   Contributed by Francois H. Theron <francois.theron@netronome.com>

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

static const bfd_arch_info_type *
bfd_nfp_compatible (const bfd_arch_info_type * a,
		    const bfd_arch_info_type * b)
{
  if (a->arch != b->arch)
    return NULL;

  if (a->mach != b->mach)
    return NULL;

  return a;
}

#define N(machine, print, default, next)			\
{								\
  32,								\
  64,								\
  8,								\
  bfd_arch_nfp,							\
  machine,							\
  "nfp",							\
  print,							\
  3,								\
  default,							\
  bfd_nfp_compatible,						\
  bfd_default_scan,						\
  bfd_arch_default_fill,					\
  next,								\
  0 /* Maximum offset of a reloc from the start of an insn.  */ \
}

static const bfd_arch_info_type arch_info_struct =
  N (bfd_mach_nfp3200, "NFP-32xx", false, NULL);

const bfd_arch_info_type bfd_nfp_arch =
  N (bfd_mach_nfp6000, "NFP-6xxx", true, &arch_info_struct);
