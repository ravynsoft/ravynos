/* BFD support for the NEC V850 processor with the RH850 ABI.
   Copyright (C) 2012-2023 Free Software Foundation, Inc.

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

#define R(number, print, default, next)  \
 { 32, 32, 8, bfd_arch_v850_rh850, number, "v850", print, 2, default, \
   bfd_default_compatible, bfd_default_scan, bfd_arch_default_fill, next, 0 }

static const bfd_arch_info_type arch_info_struct[] =
{
  R (bfd_mach_v850e3v5, "v850e3v5",	false, & arch_info_struct[1]),
  R (bfd_mach_v850e3v5, "v850e2v4",	false, & arch_info_struct[2]),
  R (bfd_mach_v850e2v3, "v850e2v3",	false, & arch_info_struct[3]),
  R (bfd_mach_v850e2,	"v850e2",	false, & arch_info_struct[4]),
  R (bfd_mach_v850e1,	"v850e1",	false, & arch_info_struct[5]),
  R (bfd_mach_v850e,	"v850e",	false, & arch_info_struct[6]),
  R (bfd_mach_v850,	"v850-rh850",	false, NULL) /* For backwards compatibility.  */
};

const bfd_arch_info_type bfd_v850_rh850_arch =
  R (bfd_mach_v850,	"v850:rh850",	true,  & arch_info_struct[0]);
