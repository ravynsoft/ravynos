/* Mach-O arm declarations for BFD.
   Copyright (C) 2015-2023 Free Software Foundation, Inc.

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

#ifndef _MACH_O_ARM64_H
#define _MACH_O_ARM64_H

/* ARM64 relocations.  */

/* Relocations for addresses in data.  */
#define BFD_MACH_O_ARM64_RELOC_UNSIGNED		0
#define BFD_MACH_O_ARM64_RELOC_SUBTRACTOR	1

/* Relocation for a call.  */
#define BFD_MACH_O_ARM64_RELOC_BRANCH26		2

/* Relocations for local data.  */
#define BFD_MACH_O_ARM64_RELOC_PAGE21		3
#define BFD_MACH_O_ARM64_RELOC_PAGEOFF12	4

/* Relocations for global data.  */
#define BFD_MACH_O_ARM64_RELOC_GOT_LOAD_PAGE21		5
#define BFD_MACH_O_ARM64_RELOC_GOT_LOAD_PAGEOFF12	6

/* Relocation for personality routine.  */
#define BFD_MACH_O_ARM64_RELOC_POINTER_TO_GOT		7

/* Relocations for thread local data.  */
#define BFD_MACH_O_ARM64_RELOC_TLVP_LOAD_PAGE21		8
#define BFD_MACH_O_ARM64_RELOC_TLVP_LOAD_PAGEOFF12	9

#define BFD_MACH_O_ARM64_RELOC_ADDEND			10

#endif /* _MACH_O_ARM64_H */
