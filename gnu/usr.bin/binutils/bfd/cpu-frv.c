/* BFD support for the FRV processor.
   Copyright (C) 2002-2023 Free Software Foundation, Inc.

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

#define FRV_ARCH(MACHINE, NAME, DEFAULT, NEXT)				\
{									\
  32,				/* Bits in a word.  */			\
  32,				/* Bits in an address.  */		\
  8,				/* Bits in a byte.  */			\
  bfd_arch_frv,			/* Architecture number.  */		\
  MACHINE,			/* Machine number.  */			\
  "frv",			/* Architecture name.  */		\
  NAME,				/* Machine name.  */			\
  4,				/* Section alignment.  */		\
  DEFAULT,			/* Is this the default?  */		\
  bfd_default_compatible,	/* Architecture comparison fn.  */	\
  bfd_default_scan,		/* String to architecture convert fn. */\
  bfd_arch_default_fill,	/* Default fill.  */			\
  NEXT,				/* Next in list.  */			\
  0 /* Maximum offset of a reloc from the start of an insn.  */		\
}

static const bfd_arch_info_type arch_info_300
  = FRV_ARCH (bfd_mach_fr300,   "fr300",   false, (bfd_arch_info_type *)0);

static const bfd_arch_info_type arch_info_400
  = FRV_ARCH (bfd_mach_fr400, "fr400", false, &arch_info_300);

static const bfd_arch_info_type arch_info_450
  = FRV_ARCH (bfd_mach_fr450, "fr450", false, &arch_info_400);

static const bfd_arch_info_type arch_info_500
  = FRV_ARCH (bfd_mach_fr500, "fr500", false, &arch_info_450);

static const bfd_arch_info_type arch_info_550
  = FRV_ARCH (bfd_mach_fr550, "fr550", false, &arch_info_500);

static const bfd_arch_info_type arch_info_simple
  = FRV_ARCH (bfd_mach_frvsimple, "simple", false, &arch_info_550);

static const bfd_arch_info_type arch_info_tomcat
  = FRV_ARCH (bfd_mach_frvtomcat, "tomcat", false, &arch_info_simple);

const bfd_arch_info_type bfd_frv_arch
  = FRV_ARCH (bfd_mach_frv, "frv", true, &arch_info_tomcat);

