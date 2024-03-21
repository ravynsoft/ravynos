/* Mach-O support for BFD.
   Copyright (C) 2011-2023 Free Software Foundation, Inc.

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

#ifndef _MACH_O_X86_64_H
#define _MACH_O_X86_64_H

/* X86-64 relocations.  */
#define BFD_MACH_O_X86_64_RELOC_UNSIGNED   0 /* Absolute addresses.  */
#define BFD_MACH_O_X86_64_RELOC_SIGNED     1 /* 32-bit disp.  */
#define BFD_MACH_O_X86_64_RELOC_BRANCH     2 /* 32-bit pcrel disp.  */
#define BFD_MACH_O_X86_64_RELOC_GOT_LOAD   3 /* Movq load of a GOT entry.  */
#define BFD_MACH_O_X86_64_RELOC_GOT        4 /* GOT reference.  */
#define BFD_MACH_O_X86_64_RELOC_SUBTRACTOR 5 /* Symbol difference.  */
#define BFD_MACH_O_X86_64_RELOC_SIGNED_1   6 /* 32-bit signed disp -1.  */
#define BFD_MACH_O_X86_64_RELOC_SIGNED_2   7 /* 32-bit signed disp -2.  */
#define BFD_MACH_O_X86_64_RELOC_SIGNED_4   8 /* 32-bit signed disp -4.  */
#define BFD_MACH_O_X86_64_RELOC_TLV	   9 /* Thread local variables.  */

#endif /* _MACH_O_X86_64_H */
