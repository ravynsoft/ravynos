/* Alpha VMS external format of Extended Image Identification.

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

#ifndef _VMS_EIHI_H
#define _VMS_EIHI_H

#define EIHI__K_MAJORID 1
#define EIHI__K_MINORID 2

struct vms_eihi
{
  unsigned char majorid[4];
  unsigned char minorid[4];

  /* Time when this image was linked.  */
  unsigned char linktime[8];

  /* Image name.  */
  unsigned char imgnam[40];

  /* Image ident.  */
  unsigned char imgid[16];

  /* Linker ident.  */
  unsigned char linkid[16];

  /* Image build ident.  */
  unsigned char imgbid[16];
};

#endif /* _VMS_EIHI_H */
