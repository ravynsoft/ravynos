/* TILE-Gx-specific support for 64-bit ELF.
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

#ifndef _ELF64_TILEGX_H
#define _ELF64_TILEGX_H

/* This file contains sizes and offsets of Linux data structures.  */

#define TILEGX_PRSTATUS_SIZEOF		 632
#define TILEGX_PRSTATUS_OFFSET_PR_CURSIG  12
#define TILEGX_PRSTATUS_OFFSET_PR_PID	  32
#define TILEGX_PRSTATUS_OFFSET_PR_REG	 112

#define TILEGX_PRPSINFO_SIZEOF		 136
#define TILEGX_PRPSINFO_OFFSET_PR_FNAME	  40
#define TILEGX_PRPSINFO_OFFSET_PR_PSARGS  56
#define ELF_PR_PSARGS_SIZE		  80

#define TILEGX_GREGSET_T_SIZE		 512

#endif /* _ELF64_TILEGX_H */
