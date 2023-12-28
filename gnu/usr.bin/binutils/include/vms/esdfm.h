/* Alpha VMS external format of Extended Symbol Definition for version Mask.

   Copyright (C) 2010-2023 Free Software Foundation, Inc.
   Written by Tristan Gingold <gingold@adacore.com>, AdaCore.

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

#ifndef _VMS_ESDFM_H
#define _VMS_ESDFM_H

struct vms_esdfm
{
  /* Entry type.  */
  unsigned char gsdtyp[2];

  /* Length of the entry.  */
  unsigned char size[2];

  /* Data type.  */
  unsigned char datyp;

  /* Pad for alignment.  */
  unsigned char temp;

  unsigned char flags[2];

  unsigned char value[8];
  unsigned char psindx[4];
  unsigned char version_mask[4];
  unsigned char namlng;
  unsigned char name[31];
};

#endif /* _VMS_ESDFM_H */
