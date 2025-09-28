/* VxWorks ELF support for BFD.
   Copyright (C) 2007-2023 Free Software Foundation, Inc.

   Contributed by Nathan Sidwell <nathan@codesourcery.com>

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

#ifndef _ELF_VXWORKS_H
#define _ELF_VXWORKS_H

#define DT_VX_WRS_TLS_DATA_START 0x60000010
#define DT_VX_WRS_TLS_DATA_SIZE  0x60000011
#define DT_VX_WRS_TLS_DATA_ALIGN 0x60000015
#define DT_VX_WRS_TLS_VARS_START 0x60000012
#define DT_VX_WRS_TLS_VARS_SIZE  0x60000013
  
#endif /* _ELF_VXWORKS_H */
