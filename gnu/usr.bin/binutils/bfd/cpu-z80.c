/* BFD library support routines for the Z80 architecture.
   Copyright (C) 2005-2023 Free Software Foundation, Inc.
   Contributed by Arnold Metselaar <arnold_m@operamail.com>

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

const bfd_arch_info_type bfd_z80_arch;

/* This routine is provided two arch_infos and
   returns whether they'd be compatible.  */

static const bfd_arch_info_type *
compatible (const bfd_arch_info_type *a, const bfd_arch_info_type *b)
{
  if (a->arch != b->arch || a->arch != bfd_arch_z80)
    return NULL;

  if (a->mach == b->mach)
    return a;
  switch (a->mach)
    {
    case bfd_mach_z80:
    case bfd_mach_z80full:
    case bfd_mach_z80strict:
      switch (b->mach)
	{
	case bfd_mach_z80:
	case bfd_mach_z80full:
	case bfd_mach_z80strict:
	  return & bfd_z80_arch;
	case bfd_mach_z180:
	case bfd_mach_ez80_z80:
	case bfd_mach_ez80_adl:
	case bfd_mach_z80n:
	case bfd_mach_r800:
	  return b;
	}
      break;
    case bfd_mach_z80n:
    case bfd_mach_r800:
      switch (b->mach)
	{
	case bfd_mach_z80:
	case bfd_mach_z80full:
	case bfd_mach_z80strict:
	  return a;
	}
      break;
    case bfd_mach_z180:
      switch (b->mach)
	{
	case bfd_mach_z80:
	case bfd_mach_z80full:
	case bfd_mach_z80strict:
	  return a;
	case bfd_mach_ez80_z80:
	case bfd_mach_ez80_adl:
	  return b;
	}
      break;
    case bfd_mach_ez80_z80:
    case bfd_mach_ez80_adl:
      switch (b->mach)
	{
	case bfd_mach_z80:
	case bfd_mach_z80full:
	case bfd_mach_z80strict:
	case bfd_mach_z180:
	case bfd_mach_ez80_z80:
	  return a;
	case bfd_mach_ez80_adl:
	  return b;
	}
      break;
    case bfd_mach_gbz80:
       return NULL;
    }

  return NULL;
}

#define N(name,print,bits,default,next)  \
 { 16, bits, 8, bfd_arch_z80, name, "z80", print, 0, default, \
   compatible, bfd_default_scan, bfd_arch_default_fill, next, 0 }

#define M(n) &arch_info_struct[n]

static const bfd_arch_info_type arch_info_struct[] =
{
  N (bfd_mach_z80,	 "z80",        16, true,  M(1)),
  N (bfd_mach_z80strict, "z80-strict", 16, false, M(2)),
  N (bfd_mach_z80full,	 "z80-full",   16, false, M(3)),
  N (bfd_mach_r800,	 "r800",       16, false, M(4)),
  N (bfd_mach_gbz80,	 "gbz80",      16, false, M(5)),
  N (bfd_mach_z180,	 "z180",       16, false, M(6)),
  N (bfd_mach_z80n,	 "z80n",       16, false, M(7)),
  N (bfd_mach_ez80_z80,	 "ez80-z80",   16, false, M(8)),
  N (bfd_mach_ez80_adl,	 "ez80-adl",   24, false, NULL)
};

const bfd_arch_info_type bfd_z80_arch =
  N (bfd_mach_z80,   "z80",   16, true,  M(1));
