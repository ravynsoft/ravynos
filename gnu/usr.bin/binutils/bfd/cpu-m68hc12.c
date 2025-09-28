/* BFD support for the Motorola 68HC12 processor
   Copyright (C) 1999-2023 Free Software Foundation, Inc.

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
    16,        /* Bits in a word.  */			\
    32,        /* Bits in an address.  */		\
    8,	       /* Bits in a byte.  */			\
    bfd_arch_m68hc12,					\
    NUMBER,						\
    "m68hc12",						\
    PRINT,						\
    4,		/* Section alignment power.  */		\
    DEFAULT,						\
    bfd_default_compatible,				\
    bfd_default_scan,					\
    bfd_arch_default_fill,				\
    NEXT,						\
    0 /* Maximum offset of a reloc from the start of an insn.  */ \
  }

const bfd_arch_info_type bfd_m68hc12s_arch =
  N (bfd_mach_m6812s, "m68hc12:HCS12", false, NULL);

const bfd_arch_info_type bfd_m68hc12_arch =
  N (bfd_mach_m6812_default, "m68hc12", true, &bfd_m68hc12s_arch);

