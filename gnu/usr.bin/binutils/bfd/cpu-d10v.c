/* BFD support for the D10V processor
   Copyright (C) 1996-2023 Free Software Foundation, Inc.
   Contributed by Martin Hunt (hunt@cygnus.com).

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
    16,         /* Bits in a word.  */			\
    18,         /* Bits in an address.  */		\
    8,	        /* Bits in a byte.  */			\
    bfd_arch_d10v,					\
    NUMBER,						\
    "d10v",						\
    PRINT,						\
    4,		/* Section alignment power.  */		\
    DEFAULT,						\
    bfd_default_compatible,				\
    bfd_default_scan,					\
    bfd_arch_default_fill,				\
    NEXT,						\
    0 /* Maximum offset of a reloc from the start of an insn.  */ \
  }

static const bfd_arch_info_type d10v_ts3_info =
  N (bfd_mach_d10v_ts3, "d10v:ts3", false, NULL);

static const bfd_arch_info_type d10v_ts2_info =
  N (bfd_mach_d10v_ts2, "d10v:ts2", false, & d10v_ts3_info);

const bfd_arch_info_type bfd_d10v_arch =
  N (bfd_mach_d10v, "d10v", true, & d10v_ts2_info);
