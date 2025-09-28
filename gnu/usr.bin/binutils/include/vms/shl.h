/* Alpha VMS external format of Shareable image List.

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

#ifndef _VMS_SHL_H
#define _VMS_SHL_H

struct vms_shl
{
  /* Base address of this shareable image.  */
  unsigned char baseva[4];

  /* Point in SHL shareable image to SHL in executable image.  */
  unsigned char shlptr[4];

  /* GSMATCH.  */
  unsigned char ident[4];

  /* Permanent shareable image context.  */
  unsigned char permctx[4];

  /* Size of this structure.  */
  unsigned char size;

  unsigned char fill_1[2];

  /* Flags.  */
  unsigned char flags;

  /* Address of the image control block (in memory).  */
  unsigned char icb[4];

  /* Image name.  ASCIC.  */
  unsigned char imgnam[40];
};

#endif /* _VMS_SHL_H */
