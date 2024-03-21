/* Copyright (C) 2005-2023 Free Software Foundation, Inc.

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

#define	NO_CORE_COMMAND		/* No command name in core file */

#undef	ALIGN			/* They use it, we use it too */

/* Note that HOST_PAGE_SIZE -- the page size as far as executable files
   are concerned -- is not the same as NBPG, because of page clustering.  */
#define	HOST_PAGE_SIZE		1024
#define	HOST_MACHINE_ARCH	bfd_arch_vax

#define	HOST_TEXT_START_ADDR	0
#define	HOST_STACK_END_ADDR	(0x80000000 - (UPAGES * NBPG))
#undef	HOST_BIG_ENDIAN_P
