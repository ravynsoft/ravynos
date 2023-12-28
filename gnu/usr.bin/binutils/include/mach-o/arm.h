/* Mach-O arm declarations for BFD.
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

#ifndef _MACH_O_ARM_H
#define _MACH_O_ARM_H

/* ARM relocations.  */
#define BFD_MACH_O_ARM_RELOC_VANILLA   0 /* Generic relocation.  */
#define BFD_MACH_O_ARM_RELOC_PAIR      1 /* Second entry in a pair.  */
#define BFD_MACH_O_ARM_RELOC_SECTDIFF  2 /* Subtract with a PAIR.  */
#define BFD_MACH_O_ARM_RELOC_LOCAL_SECTDIFF 3 /* Like above, but local ref.  */
#define BFD_MACH_O_ARM_RELOC_PB_LA_PTR 4 /* Prebound lazy pointer.  */
#define BFD_MACH_O_ARM_RELOC_BR24      5 /* 24bit branch.  */
#define BFD_MACH_O_THUMB_RELOC_BR22    6 /* 22bit branch.  */
#define BFD_MACH_O_THUMB_32BIT_BRANCH  7 /* Obselete.  */
#define BFD_MACH_O_ARM_RELOC_HALF      8
#define BFD_MACH_O_ARM_RELOC_HALF_SECTDIFF 9

#endif /* _MACH_O_ARM_H */
