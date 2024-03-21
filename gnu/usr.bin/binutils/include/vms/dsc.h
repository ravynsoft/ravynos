/* Alpha VMS external format of Descriptors.

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

#ifndef _VMS_DSC_H
#define _VMS_DSC_H

/* Descriptors.  */
#define DSC__K_DTYPE_Z	  0 /* Unspecified.  */
#define DSC__K_DTYPE_V	  1 /* Bit.  */
#define DSC__K_DTYPE_BU   2 /* Byte logical.  */
#define DSC__K_DTYPE_WU   3 /* Word logical.  */
#define DSC__K_DTYPE_LU   4 /* Longword logical.  */
#define DSC__K_DTYPE_QU   5 /* Quadword logical.  */
#define DSC__K_DTYPE_B    6 /* Byte integer.  */
#define DSC__K_DTYPE_W    7 /* Word integer.  */
#define DSC__K_DTYPE_L    8 /* Longword integer.  */
#define DSC__K_DTYPE_Q    9 /* Quadword integer.  */
#define DSC__K_DTYPE_F   10 /* Single-precision floating.  */
#define DSC__K_DTYPE_D   11 /* Double-precision floating.  */
#define DSC__K_DTYPE_FC  12 /* Complex.  */
#define DSC__K_DTYPE_DC  13 /* Double-precision Complex.  */
#define DSC__K_DTYPE_T   14 /* ASCII text string.  */
#define DSC__K_DTYPE_NU  15 /* Numeric string, unsigned.  */
#define DSC__K_DTYPE_NL  16 /* Numeric string, left separate sign.  */
#define DSC__K_DTYPE_NLO 17 /* Numeric string, left overpunched sign.  */
#define DSC__K_DTYPE_NR  18 /* Numeric string, right separate sign.  */
#define DSC__K_DTYPE_NRO 19 /* Numeric string, right overpunched sign.  */
#define DSC__K_DTYPE_NZ  20 /* Numeric string, zoned sign.  */
#define DSC__K_DTYPE_P   21 /* Packed decimal string.  */
#define DSC__K_DTYPE_ZI  22 /* Sequence of instructions.  */
#define DSC__K_DTYPE_ZEM 23 /* Procedure entry mask.  */
#define DSC__K_DTYPE_DSC 24 /* Descriptor, used for arrays of dyn strings.  */
#define DSC__K_DTYPE_OU  25 /* Octaword logical.  */
#define DSC__K_DTYPE_O   26 /* Octaword integer.  */
#define DSC__K_DTYPE_G   27 /* Double precision G floating, 64 bit.  */
#define DSC__K_DTYPE_H   28 /* Quadruple precision floating, 128 bit.  */
#define DSC__K_DTYPE_GC  29 /* Double precision complex, G floating.  */
#define DSC__K_DTYPE_HC  30 /* Quadruple precision complex, H floating.  */
#define DSC__K_DTYPE_CIT 31 /* COBOL intermediate temporary.  */
#define DSC__K_DTYPE_BPV 32 /* Bound Procedure Value.  */
#define DSC__K_DTYPE_BLV 33 /* Bound Label Value.  */
#define DSC__K_DTYPE_VU  34 /* Bit Unaligned.  */
#define DSC__K_DTYPE_ADT 35 /* Absolute Date-Time.  */
#define DSC__K_DTYPE_VT  37 /* Varying Text.  */
#define DSC__K_DTYPE_T2  38 /* 16-bit char.  */
#define DSC__K_DTYPE_VT2 39 /* 16-bit varying char.  */

#define DSC__K_CLASS_S     1 /* Fixed-length scalar/string.  */
#define DSC__K_CLASS_D     2 /* Dynamic string.  */
#define DSC__K_CLASS_V     3 /* Reserved.  */
#define DSC__K_CLASS_A     4 /* Contiguous array.  */
#define DSC__K_CLASS_P     5 /* Procedure argument descriptor.  */
#define DSC__K_CLASS_PI    6 /* Procedure incarnation descriptor.  */
#define DSC__K_CLASS_J     7 /* Reserved.  */
#define DSC__K_CLASS_JI    8 /* Obsolete.  */
#define DSC__K_CLASS_SD    9 /* Decimal (scalar) string.  */
#define DSC__K_CLASS_NCA  10 /* Non-contiguous array.  */
#define DSC__K_CLASS_VS   11 /* Varying string.  */
#define DSC__K_CLASS_VSA  12 /* Varying string array.  */
#define DSC__K_CLASS_UBS  13 /* Unaligned bit string.  */
#define DSC__K_CLASS_UBA  14 /* Unaligned bit array.  */
#define DSC__K_CLASS_SB   15 /* String with bounds.  */
#define DSC__K_CLASS_UBSB 16 /* Unaligned bit string with bounds.  */

/* Common part.  */

struct vms_dsc
{
  unsigned char length[2];
  unsigned char dtype;
  unsigned char bclass;
  unsigned char pointer[4];
};

struct vms_dsc64
{
  unsigned char mbo[2];
  unsigned char dtype;
  unsigned char bclass;
  unsigned char mbmo[4];
  unsigned char length[8];
  unsigned char pointer[8];
};

struct vms_dsc_nca
{
  unsigned char length[2];
  unsigned char dtype;
  unsigned char bclass;
  unsigned char pointer[4];

  unsigned char scale;
  unsigned char digits;
  unsigned char aflags;
  unsigned char dimct;

  unsigned char arsize[4];
  unsigned char a0[4];
};

struct vms_dsc_ubs
{
  unsigned char length[2];
  unsigned char dtype;
  unsigned char bclass;
  unsigned char base[4];
  unsigned char pos[4];
};

#endif /* _VMS_DSC_H */
