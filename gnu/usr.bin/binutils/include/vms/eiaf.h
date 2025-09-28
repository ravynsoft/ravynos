/* Alpha VMS external format of Extended Image Activator Fixup section.

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

#ifndef _VMS_EIAF_H
#define _VMS_EIAF_H

struct vms_eiaf
{
  /* Version.  */
  unsigned char majorid[4];
  unsigned char minorid[4];

  /* Link for image activator use.  */
  unsigned char iaflink[8];

  /* Link for sharable image fixups.  */
  unsigned char fixuplnk[8];

  /* Size of EIAF fixed part.  */
  unsigned char size[4];

  /* Flags.  */
  unsigned char flags[4];

  /* Offsets to quadword and longword relocation fixup data.  */
  unsigned char qrelfixoff[4];
  unsigned char lrelfixoff[4];

  /* Offsets to quardword and longword .address fixup data.  */
  unsigned char qdotadroff[4];
  unsigned char ldotadroff[4];

  /* Offset to code address fixup data.  */
  unsigned char codeadroff[4];

  /* Offset to linkage part fixup data.  */
  unsigned char lpfixoff[4];

  /* Offset to isect change protection data.  */
  unsigned char chgprtoff[4];

  /* Offset to shareable image list.  */
  unsigned char shlstoff[4];

  /* Number of shareable images.  */
  unsigned char shrimgcnt[4];

  /* Number of extra shareable images allowed.  */
  unsigned char shlextra[4];

  /* Permanent shareable image context.  */
  unsigned char permctx[4];

  /* Base address of the image itself.  */
  unsigned char base_va[4];

  /* Offset to linkage pair with procedure signature fixups.  */
  unsigned char lppsbfixoff[4];
};

#endif /* _VMS_EIAF_H */
