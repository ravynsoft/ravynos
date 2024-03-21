/* BFD support for LoongArch.
   Copyright (C) 2021-2023 Free Software Foundation, Inc.
   Contributed by Loongson Ltd.

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
   along with this program; see the file COPYING3.  If not,
   see <http://www.gnu.org/licenses/>.  */

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"

static const bfd_arch_info_type bfd_loongarch32_arch =
{
  32,				/* 32 bits in a word.  */
  32,				/* 64 bits in an address.  */
  8,				/* 8 bits in a byte.  */
  bfd_arch_loongarch,		/* Architecture.  */
  bfd_mach_loongarch32, 	/* Machine number - 0 for now.  */
  "loongarch32",		/* Architecture name.  */
  "Loongarch32",		/* Printable name.  */
  3,				/* Section align power.  */
  false,			/* This is the default architecture.  */
  bfd_default_compatible,	/* Architecture comparison function.  */
  bfd_default_scan,		/* String to architecture conversion.  */
  bfd_arch_default_fill,	/* Default fill.  */
  NULL, 			/* Next in list.  */
  0,
};

const bfd_arch_info_type bfd_loongarch_arch =
{
  32,				/* 32 bits in a word.  */
  64,				/* 64 bits in an address.  */
  8,				/* 8 bits in a byte.  */
  bfd_arch_loongarch,		/* Architecture.  */
  /* Machine number of LoongArch64 is larger
   * so that LoongArch64 is compatible to LoongArch32.  */
  bfd_mach_loongarch64,
  "loongarch64",		/* Architecture name.  */
  "Loongarch64",		/* Printable name.  */
  3,				/* Section align power.  */
  true, 			/* This is the default architecture.  */
  bfd_default_compatible,	/* Architecture comparison function.  */
  bfd_default_scan,		/* String to architecture conversion.  */
  bfd_arch_default_fill,	/* Default fill.  */
  &bfd_loongarch32_arch,	/* Next in list.  */
  0,
};
