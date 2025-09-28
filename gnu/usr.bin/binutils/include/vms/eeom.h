/* Alpha VMS external format of Extended End Of Module.

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

#ifndef _VMS_EEOM_H
#define _VMS_EEOM_H

/* Completion flags.  */
#define EEOM__C_SUCCESS 0
#define EEOM__C_WARNING 1
#define EEOM__C_ERROR   2
#define EEOM__C_ABORT   3

struct vms_eeom
{
  /* Record type.  */
  unsigned char rectyp[2];

  /* Record size.  */
  unsigned char size[2];

  /* Number of conditional linkage pairs.  */
  unsigned char total_lps[4];

  /* Completion code.  */
  unsigned char comcod[2];


  /* Transfer address flags.  */
  unsigned char tfrflg;

  /* Pad for alignment.  */
  unsigned char temp;

  /* Psect of transfer address.  */
  unsigned char psindx[4];

  /* Transfer address.  */
  unsigned char tfradr[8];
};

#define EEOM__M_WKTFR (1 << 0)	/* Transfer address is weak.  */

#endif /* _VMS_EEOM_H */
