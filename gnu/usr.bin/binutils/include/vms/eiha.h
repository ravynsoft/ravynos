/* Alpha VMS external format of Extended Image Activation.

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

#ifndef _VMS_EIHA_H
#define _VMS_EIHA_H

struct vms_eiha
{
  /* Size of the struct.  */
  unsigned char size[4];

  unsigned char spare[4];

  /* First transfer address.  */
  unsigned char tfradr1[4];
  unsigned char tfradr1_h[4];

  /* Second.  */
  unsigned char tfradr2[4];
  unsigned char tfradr2_h[4];

  /* Third.  */
  unsigned char tfradr3[4];
  unsigned char tfradr3_h[4];

  /* Fourth (must be 0).  */
  unsigned char tfradr4[4];
  unsigned char tfradr4_h[4];

  /* Shared image initialization (only if EIHD__V_INISHR is set).  */
  unsigned char inishr[4];
  unsigned char inishr_h[4];
};

#endif /* _VMS_EIHA_H */
