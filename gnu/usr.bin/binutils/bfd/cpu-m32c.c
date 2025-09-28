/* BFD support for the M16C/M32C processors.
   Copyright (C) 2004-2023 Free Software Foundation, Inc.

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
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"

/* Like bfd_default_scan but if the string is just "m32c" then
   skip the m16c architecture.  */

static bool
m32c_scan (const bfd_arch_info_type * info, const char * string)
{
  if (strcmp (string, "m32c") == 0
      && info->mach == bfd_mach_m16c)
    return false;

  return bfd_default_scan (info, string);
}

#define N(number, print, align, default, next)			   \
{ 32, 32, 8, bfd_arch_m32c, number, "m32c", print, align, default, \
  bfd_default_compatible, m32c_scan, bfd_arch_default_fill, next, 0 }

static const bfd_arch_info_type arch_info_struct =
  N (bfd_mach_m32c, "m32c", 3, false, NULL);

const bfd_arch_info_type bfd_m32c_arch =
  N (bfd_mach_m16c, "m16c", 4, true, &arch_info_struct);

