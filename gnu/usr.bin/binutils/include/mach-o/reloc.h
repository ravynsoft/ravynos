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

#ifndef _MACH_O_RELOC_H
#define _MACH_O_RELOC_H

/* Generic relocation types (used by i386).  */
#define BFD_MACH_O_GENERIC_RELOC_VANILLA 	0
#define BFD_MACH_O_GENERIC_RELOC_PAIR	 	1
#define BFD_MACH_O_GENERIC_RELOC_SECTDIFF	2
#define BFD_MACH_O_GENERIC_RELOC_PB_LA_PTR	3
#define BFD_MACH_O_GENERIC_RELOC_LOCAL_SECTDIFF	4
#define BFD_MACH_O_GENERIC_RELOC_TLV		5

#endif /* _MACH_O_RELOC_H */
