/* BFD support for the ia64 architecture.
   Copyright (C) 1998-2023 Free Software Foundation, Inc.
   Contributed by David Mosberger-Tang <davidm@hpl.hp.com>

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

#define N(BITS_ADDR, NUMBER, PRINT, DEFAULT, NEXT) \
  {							\
    64,        /* Bits in a word.  */			\
    BITS_ADDR, /* Bits in an address.  */		\
    8,	       /* Bits in a byte.  */			\
    bfd_arch_ia64,					\
    NUMBER,						\
    "ia64",						\
    PRINT,						\
    3,		/* Section alignment power.  */		\
    DEFAULT,						\
    bfd_default_compatible,				\
    bfd_default_scan,					\
    bfd_arch_default_fill,				\
    NEXT,						\
    0 /* Maximum offset of a reloc from the start of an insn.  */ \
  }

const bfd_arch_info_type bfd_ia64_elf32_arch =
  N (32, bfd_mach_ia64_elf32, "ia64-elf32", false, NULL);

const bfd_arch_info_type bfd_ia64_arch =
  N (64, bfd_mach_ia64_elf64, "ia64-elf64", true, &bfd_ia64_elf32_arch);

#include "cpu-ia64-opc.c"
